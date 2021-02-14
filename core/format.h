#ifndef FORMAT_H
#define FORMAT_H

#include <windows.h>
#include <shlwapi.h>

#define LOCALE LOCALE_USER_DEFAULT

int NetworkSend_BuildSystemTime(DWORD lowDateTime, DWORD highDateTime, SYSTEMTIME *systemTime) {
    // Build the FILETIME from the low and high values.
    FILETIME fileTime;
    fileTime.dwLowDateTime = lowDateTime;
    fileTime.dwHighDateTime = highDateTime;

    // Convert the FILETIME to SYSTEMTIME.
    if (!FileTimeToSystemTime(&fileTime, systemTime)) {        
        return 0; // Format failed.
    }

    return 1;
}

int NetworkSend_FormatFileTime(DWORD lowDateTime, DWORD highDateTime, char* buf, int bufSize) {
    SYSTEMTIME systemTime;
    if (!NetworkSend_BuildSystemTime(lowDateTime, highDateTime, &systemTime)) {
        buf[0] = '\0';  // Force an empty string.
        return 0;  // Unable to build system time.
    }

    if (!GetDateFormatA(LOCALE, 0, &systemTime, NULL, buf, bufSize)) {
        buf[0] = '\0';  // Force an empty string.
        return 0;
    }
}

int NetworkSend_FormatFileDate(DWORD lowDateTime, DWORD highDateTime, char* buf, int bufSize) {
    SYSTEMTIME systemTime;
    if (!NetworkSend_BuildSystemTime(lowDateTime, highDateTime, &systemTime)) {
        buf[0] = '\0';  // Force an empty string.
        return 0;  // Unable to build system time.
    }

    if (!GetTimeFormatA(LOCALE, 0, &systemTime, NULL, buf, bufSize)) {
        buf[0] = '\0'; // Force an empty string.
        return 0;
    }

    return 1;
}

int NetworkSend_FormatFileSize(DWORD lowFileSize, DWORD highFileSize, char* buf, int bufSize) {
    if (!StrFormatByteSizeA(lowFileSize, buf, bufSize)) {
        buf[0] = '\0'; // Force an empty string.
        return 0;
    }

    return 1;
}

#endif