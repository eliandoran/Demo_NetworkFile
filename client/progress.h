#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "format.h"

#define SIZE_BUF 100

void NetworkSend_FormatTransferProgressBar(
    unsigned long long currentValue,
    unsigned long long totalValue,
    float averageSpeed,
    char* buf,
    int bufSize
) {
    char sizeBuf[SIZE_BUF];

    buf += sprintf(buf, "DOWNLOAD ");
    
    // Display current value, formatted in units.
    LARGE_INTEGER li;
    li.QuadPart = currentValue;
    NetworkSend_FormatFileSize(li.LowPart, li.HighPart, sizeBuf, sizeof(sizeBuf));
    buf += sprintf(buf, "%s / ", sizeBuf);

    // Display total value, formatted in units.
    li.QuadPart = totalValue;
    NetworkSend_FormatFileSize(li.LowPart, li.HighPart, sizeBuf, sizeof(sizeBuf));
    buf += sprintf(buf, "%s", sizeBuf);

    // Display percentage of transfer.
    float percentage = ((float)currentValue / totalValue) * 100;
    buf += sprintf(buf, " (%.2f%%)", percentage);

    // Display average speed.
    li.QuadPart = (int) averageSpeed;
    NetworkSend_FormatFileSize(li.LowPart, li.HighPart, sizeBuf, sizeof(sizeBuf));
    buf += sprintf(buf, " avg. %s/s", sizeBuf);
}

#endif