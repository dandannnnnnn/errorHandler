/* ----------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------*/
/* Main code for error handler */
// started compiler with: gcc -o main main.c -lpaho-mqtt3c and then ./main
//======================================================================================
//======================================================================================
/* Ik heb ChatGPT en Copilot gebruikt om mij te helpen met de segmentation fout eruit
  omdat ik het zelf na 4x te bekijken niet vond. */
/* -------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <MQTTClient.h> 
#include "settingErrHandler.h" //Make sure this file is added for the connection and correct variables

char error_field[][errorMSG_LEN] = {"", "", "", ""};
volatile MQTTClient_deliveryToken deliveredtoken;

struct tbl {
    char errorCode[errorCode_LEN + 1]; //added one more space for '\0'
    char errorText[errorMSG_LEN + 1];
    struct tbl *next; //links multiple struct tbls together ==> linked list
};

struct tbl *head = NULL; //declaring a pointer and not a value
struct tbl *current = NULL;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    if (context == NULL) {
        printf("Error context is NULL in delivered function\n");
        return;
    }
    printf("Message with token value %d delivery confirmed\n", dt);
    printf( "-----------------------------------------------\n" );    
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    if (message == NULL || message->payload == NULL) {
        printf("Received NULL message or payload\n");
        return 0;
    }

    char *error_in = message->payload;
    char  error_out[ errorOutput_LEN ] = "";
    
    // print incoming message
    printf( "msgarrvd: error_in: <%s>\n", error_in );   
    
    // format error out msg
    sprintf( error_out, "%s + Some additional text here", error_in );
    printf( "msgarrvd: error_out: <%s>\n", error_out );   

    //checking if client is valid
    if (context == NULL) {
        printf("Error");
        return 0;
    }

    // Create a new client to publish the error_out message
    MQTTClient client = (MQTTClient)context;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token2;

    pubmsg.payload = error_out;
    pubmsg.payloadlen = strlen( error_out );
    pubmsg.qos = QoS;
    pubmsg.retained = 0;

    //Checking if topic2 is correct
    printf("Publishing to topic %s with message: %s\n", topic2, error_out);

    //Publish the error_out message on MB_publishMQTT
    int rc = MQTTClient_publishMessage(client, topic2, &pubmsg, &token2);
    printf("Publishing to topic %s\n", topic2);
    
    // Validate that message has been successfully delivered
    rc = MQTTClient_waitForCompletion(client, token2, timeout);
    printf("Message with delivery token %d delivered, rc=%d\n", token2, rc);
    printf( "Msg out:\t<%s>\n", error_out );

    // Close the outgoing message queue
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

//insert_first function here
void insert_first(char *errorCode, char *errorText) { 
    struct tbl *lnklist = (struct tbl*)malloc(sizeof(struct tbl)); //reserves space for a new struct tbl

    if(!lnklist) { //Checking if memory allocation is correctly set
        printf("Memory allocation failed in insert_first function");
        exit(1); //exiting if it is incorrect
    }

    strncpy(lnklist->errorCode, errorCode, errorCode_LEN);
    lnklist->errorCode[errorCode_LEN] = '\0';
    strncpy(lnklist->errorText, errorText, errorMSG_LEN);
    lnklist->errorText[errorMSG_LEN] = '\0';

    lnklist -> next = head;
    head = lnklist;
}

void insert_next(struct tbl *list, char *errorCode, char *errorText) {
    if (!list) { //Checking if the pointer is correct
        printf("Received NULL pointer for list in insert_next function.\n");
        return;
    }

    struct tbl *lnklist = (struct tbl*)malloc(sizeof(struct tbl));

    if(!lnklist) { //Checking if memory allocation is correctly set
        printf("Memory allocation failed in insert_next function.\n");
        exit(1);
    }

    strncpy(lnklist->errorCode, errorCode, errorCode_LEN);
    lnklist->errorCode[errorCode_LEN] = '\0';
    strncpy(lnklist->errorText, errorText, errorMSG_LEN);
    lnklist->errorText[errorMSG_LEN] = '\0';

    lnklist -> next = list -> next;
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
    *list = NULL; //Update *list to NULL if no match is found
    return 0;
}

//printlist function here
void printList() {
    struct tbl *p = head;

    if(p == NULL) {
        printf("List is empty.\n");
        return;
    }
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
        return 1;
    }
}

//extractErrText function here
int extractErrText(char *line, char *errorText) {
    int i = errorCode_LEN + 1;
    int z = 0;

    if(!line || !errorText) {
        return 0;
    }

    while (line[i] != '\n') {
        errorText[z] = line[i];
        i++;
        z++;
        if ( i > errorMSG_LEN) {
            return 0;
        }
    }
     errorText[z] = '\0';
     return 1;
}

//readingFile function here
int readingFile(char *filename) {
    FILE *fp;
    int count = 0;
    char line[1024] = "";
    char errorCode[errorCode_LEN + 1] = "";
    char errorText[errorMSG_LEN + 1] = "";

    struct tbl *lnklist = (struct tbl*)malloc(sizeof(struct tbl));

    if(lnklist == NULL) {
        printf("Memory allocation failed!");
        exit(1);
    }

    head = lnklist;
    head -> next = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file.");
        return 0;
    }
    count = 0;
    while(fgets(line, sizeof(line), fp) != NULL) {
        line[sizeof(line) - 1] = '\0';
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

    if(line == NULL) { //Checking validation of the input file
        return 0;
    }

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

    if (error_field == NULL) {
        return;
    }

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

//connlost function here
void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

//main function here
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Using: %s <LANG>\n", argv[0]);
        return 1;
    }

    char *language = argv[1];
    char filename[50];

    //file EN, FRE or NL will open depending on the given argument
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

    MQTTClient_create(&client, IPaddress, CLIENT, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Define the correct call back functions when messages arrive
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

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

    return 0;
}

