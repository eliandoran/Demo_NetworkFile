#ifndef LS_H_DEFINED
#define LS_H_DEFINED

#include <stdio.h>
#include "../core.h"
#include "../socket.h"

void NetworkSend_ListFiles(char* path) {
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(path, &findData);

    if (findHandle == INVALID_HANDLE_VALUE) {
        // Unable to open handle for file search.
        return;
    }

    // Iterate through each of the files.
    do {
        printf("%s\n", findData.cFileName);
    } while (FindNextFileA(findHandle, &findData) != 0);
}

#endif