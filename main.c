/* ----------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------*/
/* Main code for error handler */
/* -------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <MQTTClient.h> //remove '//' when using RPi
#include "settingErrHandler.h" //file with array definitions and length
#include "send_mqtt_msg.c" //file with MQTT functions

volatile MQTTClient_deliveryToken deliveredToken;

struct tbl {
    char errorCode[errorCode_LEN + 1]; //added one more space for '\0'
    char errorText[errorMSG_LEN + 1];
    struct tbl *next; //links multiple struct tbls together ==> linked list
};

struct tbl *head = NULL; //declaring a pointer and not a value
struct tbl *current = NULL;

//insert_first function here
void insert_first(char *errorCode, char *errorText) { 
    struct tbl *lnklist = (struct tbl*)malloc(sizeof(struct tbl)); //reserves space for a new struct tbl
    strcpy(lnklist -> errorCode, errorCode);
    strcpy(lnklist -> errorText, errorText);

    lnklist -> next = NULL;
    head = lnklist;
}

void insert_next(struct tbl *list, char *errorCode, char *errorText) {
    struct tbl *lnklist = (struct tbl*)malloc(sizeof(struct tbl));
    strcpy(lnklist -> errorCode, errorCode);
    strcpy(lnklist -> errorText, errorText);

    lnklist -> next = NULL;
    list -> next = lnklist;
}

//searchList function here
int searchList(struct tbl **list, char *errorCode) {
    struct tbl *temp = head;
    while(temp != NULL) {
        if (strcmp(temp -> errorCode, errorCode) == 0) {
            *list = temp;
            return 1;
        }
        temp = temp -> next;
    }
    return 0;
}

//printlist function here
void printList() {
    struct tbl *p = head;
    printf("\nError list:\n");
    printf("==================================\n");
    //start from the beginning
    while(p != NULL) {
        printf("%s\t%s\n", p -> errorCode, p -> errorText);
        p = p -> next;
    }
    printf("\nEnd of Error list\n");
    printf("==================================\n");
}

//searchErrorCode function here
void searchErrorCode(char *errorCode) {
    struct tbl *p = head;
    while(p != NULL) {
        if (strcmp(p -> errorCode, errorCode) == 0) {
            printf("%s\t%s\n", p -> errorCode, p -> errorText);
        }
        p = p -> next;
    }
    printf("Error code not found.");
}

//splitStrings function here
void splitStrings(FILE *fp) {
    char line[150];
    char *token, *rest;

    if (fgets(line, sizeof(line), fp)) {
        if (line[0] != '#') {
            printf("%s", line);
        }
    }

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] != '#') {
            char fields[2][50];
            int fieldCount = 0;

            rest = line;
            while ((token = strtok_r(rest, "\t", &rest)) != NULL && fieldCount < 7) {
                strcpy(fields[fieldCount], token);
                fieldCount++;
            }
            if (fieldCount == 2) {
                insert_first(fields[0], fields[1]);
            } else {
                printf("Error: %d fields instead of 2 fields!\n", fieldCount);
            }
        }
    }
    fclose(fp);
}

//delivered function: used when a message is succesfully delivered
void delivered(void* context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivered.\n", dt);
    printf( "-----------------------------------------------\n" );
    deliveredToken = dt;
}
//readInputMQTT function here
int readInputMQTT(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    char *errorInput = message ->payload;
    char errorOutput[errorOutput_LEN] = "";

    printf("Message arrived: <%s>\n", errorInput);
    
    sprintf(errorOutput, "%s", errorInput);
    printf("Message arrived: <%s>\n", errorOutput);

    //create new client to publish errorOutput message
    MQTTClient client = (MQTTClient)context;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = errorOutput;
    pubmsg.payloadlen = strlen(errorOutput);
    pubmsg.qos = QoS;
    pubmsg.retained = 0;

    //Publish errorOutput message on topic2
    MQTTClient_publishMessage(client, topic2, &pubmsg, &token);
    printf("Publishing to topic %s\n", topic2);

    int rc = MQTTClient_waitForCompletion(client, token, timeout );
    printf("Message with delivery token %d delivered, rc=%d\n", token, rc);
    printf( "Msg out:\t<%s>\n", errorOutput); 

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1;
}

void connectionLost(void *context, char *cause) {
    printf("Connection lost!\n");
    printf("    cause: %s\n", cause);
}

//dateTimestamp function here: DD/MM/YYYY HH:MM:SS
void dateTimestamp() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now); //structure already in the library 
    char currentDate[20];
    char day[3];
    char month[3];
    char year [5];
    char hour[3];
    char minute[3];
    char second[3];

    sprintf(day, "%02d", t->tm_mday);
    sprintf(month, "%02d", t->tm_mon+1);
    sprintf(year, "%04d", t-> tm_year + 1900);
    sprintf(hour, "%02d", t->tm_hour);
    sprintf(minute, "%02d", t->tm_min);
    sprintf(second, "%02d", t->tm_sec);

    sprintf(dateTimestamp, "%s/%s/%s %s:%s:%s", day, month, year, hour, minute, second);
}
//format_outgoingMSG function here
void format_outgoingMSG(char *line, char *errorOutput) {
    
}
//optionFileLanguage function here

//main function here
int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, IPaddress, CLIENT, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Define the correct call back functions when messages arrive
    MQTTClient_setCallbacks(client, client, connectionLost, readInputMQTT, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribing to topic %s for client %s using QoS%d\n\n", topic1, CLIENT, QoS);
    MQTTClient_subscribe(client, topic1, QoS);

    // Keep the program running to continue receiving and publishing messages
    for(;;) {
        ;
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}



