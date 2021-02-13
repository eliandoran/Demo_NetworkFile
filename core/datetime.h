#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <windows.h>
#include <stdio.h>

#define LOCALE LOCALE_USER_DEFAULT

int NetworkSend_FormatFileDate(DWORD lowDateTime, DWORD highDateTime, char* buf, int bufSize) {
    // Build the FILETIME from the low and high values.
    FILETIME fileTime;
    fileTime.dwLowDateTime = lowDateTime;
    fileTime.dwHighDateTime = highDateTime;

    // Convert the FILETIME to SYSTEMTIME.
    SYSTEMTIME systemTime;
    if (!FileTimeToSystemTime(&fileTime, &systemTime)) {        
        return 0; // Format failed.
    }

    // Format the date first.
    if (!(GetDateFormatA(LOCALE, 0, &systemTime, NULL, buf, bufSize))) {
        buf[0] = '\0';  // Force an empty string.
        return 0;
    }

    // Append a space and alter the buffer pointer to append the time to the right of the date.
    int dateLen = strlen(buf) + 1; // consider null character also.   
    buf[dateLen - 1] = ' '; // replace null with a space
    buf += dateLen;
    bufSize -= dateLen;

    // Format the time second.
    if (!(GetTimeFormatA(LOCALE, 0, &systemTime, NULL, buf, bufSize))) {
        buf[0] = '\0'; // Force an empty string.
        return 0;
    }

    return 1;
}

#endif