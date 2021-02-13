#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <windows.h>
#include <stdio.h>

#define LOCALE LOCALE_USER_DEFAULT

char* NetworkSend_FormatDate(SYSTEMTIME *date) {
    // Determine the size needed to fit the date.
    int dateLength = GetDateFormatA(LOCALE, 0, date, NULL, NULL, 0);

    // Allocate enough memory for the date.
    char* dateStr = (char*) malloc(sizeof(char) * dateLength);

    // Obtain the formatted date.
    int result = GetDateFormatA(LOCALE, 0, date, NULL, dateStr, dateLength);
    return dateStr;
}

char* NetworkSend_FormatFileDate(FILETIME fileTime) {
    // Convert the FILETIME to SYSTEMTIME.
    SYSTEMTIME systemTime;
    if (!FileTimeToSystemTime(&fileTime, &systemTime)) {
        // Format failed.
        return NULL;
    }

    // Format the date.
    return NetworkSend_FormatDate(&systemTime);
}

#endif