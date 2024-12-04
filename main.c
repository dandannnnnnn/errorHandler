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
int main(int argc, char ***argv[]) {

    //Asking user which language they want to use
    char filename[FNAME_LEN] = "./Error_msg_";
    int languageAnswer;
    char language[5];

    printf("Choose the language:\n");
    printf("1. English\n");
    printf("2. French\n");
    printf("3. Dutch\n");
    printf("Enter your choice (1-3): ");
    scanf("%d", &languageAnswer);

    switch (languageAnswer) {
        case 1:
            strcpy(language, "EN");
            break;
        case 2:
            strcpy(language, "FRE");
            break;
        case 3:
            strcpy(language, "NL");
            break;
        default:
            printf("Invalid choice. Default language will be used\n");
            strcpy(language, "EN");
            break;
    }
    snprintf(filename, FNAME_LEN, "./Error_msg_%s.txt", language); //complete file path
    if (readingFile(filename) == 0) {
        printf("Error reading file %s\n", filename);
        return 1;
    }
    printf("Language file loaded succesfully: %s\n", filename);
    printList();
    
    return 0;
}

