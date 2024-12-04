/* ----------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------*/
/* Main code for error handler */
// started compiler with: gcc -o main main.c -lpaho-mqtt3c and then ./main
/* -------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <MQTTClient.h> 
#include "settingErrHandler.h" //file with array definitions and length
#include "send_mqtt_msg.c" //file with MQTT functions

char error_field[][errorMSG_LEN] = {"", "", "", ""};
volatile MQTTClient_deliveryToken deliveredtoken;

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

void delivered(void *context, MQTTClient_deliveryToken dt) {
    
    printf("Message with token value %d delivery confirmed\n", dt);
    printf( "-----------------------------------------------\n" );    
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *error_in = message->payload;
    char  error_out[ errorOutput_LEN ] = "";
    
    // print incoming message
    printf( "msgarrvd: error_in: <%s>\n", error_in );   
    
    // format error out msg
    sprintf( error_out, "%s + Some additional text here", error_in );
    printf( "msgarrvd: error_out: <%s>\n", error_out );   

    // Create a new client to publish the error_out message
    MQTTClient client = (MQTTClient)context;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token2;

    pubmsg.payload = error_out;
    pubmsg.payloadlen = strlen( error_out );
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    //Publish the error_out message on PUB TOPIC 
    MQTTClient_publishMessage(CLIENT, topic2, &pubmsg, &token2);
    printf("Publishing to topic %s\n", topic2);
    
    // Validate that message has been successfully delivered
    int rc = MQTTClient_waitForCompletion(CLIENT, token2, timeout);
    printf("Message with delivery token %d delivered, rc=%d\n", token2, rc);
    printf( "Msg out:\t<%s>\n", error_out );

    // Close the outgoing message queue
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

//connlost function here
void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
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

//extractErrCode function here
int extractErrCode(char *line, char *errorCode) {
    int i = 0;

    while ((line[i] != '\t') && (line[i] != '\n') ) {
        errorCode[i] = line[i];
        i++;
        if ( i > errorCode_LEN) {
            return 0;
        }
        errorCode[i] = '\0';
    }
}

//extractErrText function here
int extractErrText(char *line, char *errorText) {
    int i = errorCode_LEN + 1;
    int z = 0;

    while (line[i] != '\n') {
        errorText[z] = line[i];
        i++;
        z++;
        if ( i > errorMSG_LEN) {
            return 0;
        }
        errorText[z] = '\0';
    }
}

//readingFile function here
int readingFile(char *filename) {
    FILE *fp;
    int count = 0;
    char line[1024] = "";
    char errorCode[errorCode_LEN + 1] = "";
    char errorText[errorMSG_LEN + 1] = "";

    struct tbl *linklist = (struct tbl*)malloc(sizeof(struct tbl));
    head = linklist;
    head -> next = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file.");
        exit(1);
    }
    count = 0;
    while(fgets(line, sizeof(line), fp) != NULL) {
        if(line[0] != '#') {
            count++;
            if (extractErrCode(line, errorCode) == 0) {
                printf("Incorrect error code on line %d of file %s\n", count, filename);
                return 0;
            }
            if (extractErrText(line, errorText) == 0) {
                printf("Incorrect error text on line %d of file %s\n", count, filename);
                return 0;
            }
            if (count == 1) {
                insert_first(errorCode, errorText);
                current = head;
            } else {
                insert_next(current, errorCode, errorText);
                current = current -> next;
            }
        }
    }
    fclose(fp);
    return 1;
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

    strcpy(currentDate, day);
    strcat(currentDate, "/");
    strcat(currentDate, month);
    strcat(currentDate, "/");
    strcat(currentDate, year);
    strcat(currentDate, " ");
    strcat(currentDate, hour);
    strcat(currentDate, ":");
    strcat(currentDate, minute);
    strcat(currentDate, ":");
    strcat(currentDate, second);

    printf("%s\n", currentDate);
}

//parameterYES function here
int parameterYES(char *line) {
    int i = 0;

    while (line[i] != '\0') {
        if ((line[i] == '%') && (line[i+1] == 's')) {
            return 1;
        }
        i++;
    }
    return 0;
}

//defaultSettings function here
void defaultSettings() {
    if ((error_field[sevCode_field][0] < SEV1) || (error_field[sevCode_field][0] > SEV4)) {
        strcpy(error_field[sevCode_field], sevCode_DEFAULT);

        if (strlen(error_field[app_field]) > appLEN) { //appLen and app_field stand for the application length and amount of fields
            error_field[app_field][appLEN] = '\0';
        }
        if (error_field[app_field][0] == '\0') {
            strcpy(error_field[app_field], app_DEFAULT);
        }
        if (strlen(error_field[errorCode_field]) != errorCode_LEN) {
            strcpy(error_field[errorCode_field], errorCode_DEFAULT);
        }
        if (strlen(error_field[errorParam_field]) > errorParam_LEN) {
            error_field[errorParam_field][errorParam_LEN] = '\0';
        }
    }
 }


//main function here
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Using: %s <LANG>\n", argv[0]);
        return 1;
    }

    char *language = argv[1];
    char filename[50];

    if (strcmp(language, "EN") == 0) {
        strcpy(filename, "Error_msg_EN.txt");
    } else if (strcmp(language, "FRE") == 0) {
        strcpy(filename, "Error_msg_FRE.txt");
    } else if (strcmp(language, "NL") == 0) {
        strcpy(filename, "Error_msg_NL.txt");
    } else {
        printf("Unsupported language: %s. Default language will be used\n", language);
        strcpy(filename, "Error_msg_EN.txt");
    }

    printf("Running error messages, language: %s\n", language);
    if(!readingFile(filename)) {
        printf("Error: cannot read file %s\n", filename);
        return 1;
    }
    printList();

       // Open MQTT client for listening
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Define the correct call back functions when messages arrive
    MQTTClient_setCallbacks(client, client, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribing to topic %s for client %s using QoS%d\n\n", topic2, CLIENT, QoS);
    MQTTClient_subscribe(CLIENT, topic2, QoS);

    // Keep the program running to continue receiving and publishing messages
    for(;;) {
        ;
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
    
    return 0;
}

