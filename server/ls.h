#ifndef LS_H_DEFINED
#define LS_H_DEFINED

#include <stdio.h>
#include "core.h"
#include "socket.h"
#include "response.h"

int NetworkSend_ListFiles(SOCKET clientSocket, char* path) {
    struct NetworkSend_Response response;
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(path, &findData);
    int bodySize = 0;

    if (findHandle != INVALID_HANDLE_VALUE) {
        // Iterate through each of the files, to determine the body size.
        do {
            // Ignore directories.
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            printf("Got file: %s\n", findData.cFileName);
            bodySize += strlen(findData.cFileName + 1);
        } while (FindNextFileA(findHandle, &findData) != 0);

        printf("Body size: %d\n", bodySize);

        // Generate the response body by iterating again through

        response.status = NETWORKSEND_RESPONSE_STATUS_OK;
    } else {
        // Unable to open handle for file search.
        response.status = NETWORKSEND_RESPONSE_STATUS_ERROR;
    }

    return NetworkSend_SendResponse(clientSocket, &response);
}

#endif