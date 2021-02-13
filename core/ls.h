#ifndef LS_H_DEFINED
#define LS_H_DEFINED

#include <stdio.h>
#include "core.h"
#include "socket.h"
#include "response.h"

struct NetworkSend_FileListing {
    char* name;
};

int NetworkSend_SendFileListing(SOCKET socket, struct NetworkSend_FileListing *fileData) {
    int nameLength = strlen(fileData->name);
    int bufLength = nameLength + 1; // incl. null terminator.
    char *buf = (char*)malloc(sizeof(char) * bufLength);
    buf[nameLength + 1] = '\0';
    strncpy(buf, fileData->name, nameLength);

    int result = Socket_Send(socket, buf, bufLength);
    free(buf);
    return result;
}

int NetworkSend_ListFiles(SOCKET clientSocket, char* path) {
    struct NetworkSend_Response response;
    struct NetworkSend_FileListing fileData;
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(path, &findData);

    if (findHandle != INVALID_HANDLE_VALUE) {
        // Iterate through each of the files, to determine the body size.
        do {
            // Ignore directories.
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            printf("Got file: %s\n", findData.cFileName);

            fileData.name = findData.cFileName;
            printf("%s\n", fileData.name);
            NetworkSend_SendFileListing(clientSocket, &fileData);
        } while (FindNextFileA(findHandle, &findData) != 0);

        response.status = NETWORKSEND_RESPONSE_STATUS_OK;
    } else {
        // Unable to open handle for file search.
        response.status = NETWORKSEND_RESPONSE_STATUS_ERROR;
    }

    return NetworkSend_SendResponse(clientSocket, &response);
}

#endif