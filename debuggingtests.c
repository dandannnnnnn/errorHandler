//========================================================================================================================
//========================================================================================================================
//
//                                  DEBUGGING TESTS: Testing functions apart here
//
//========================================================================================================================
//========================================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

int main() {
    dateTimestamp();
    return 0;
}