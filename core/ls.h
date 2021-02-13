#ifndef LS_H_DEFINED
#define LS_H_DEFINED

#include <stdio.h>
#include "core.h"
#include "socket.h"
#include "response.h"

struct NetworkSend_FileListing {    
    int nameLength;
    char* name;
};

int NetworkSend_SendFileListing(SOCKET socket, struct NetworkSend_FileListing *fileData) {
    // Determine the total buffer size needed for sending the file listing data.
    int bufLength = 0;
    bufLength += sizeof(int);                               // nameLength field
    bufLength += sizeof(char) * (fileData->nameLength + 1); // name field w/ null terminator

    // Allocate the buffer.
    char* data = (char*)malloc(bufLength);
    char* dataCursor = data;
    ZeroMemory(data, bufLength);

    // Set the nameLength field.
    memcpy(dataCursor, &bufLength, sizeof(bufLength));
    dataCursor += sizeof(bufLength);

    // Set the name field.
    memcpy(dataCursor, fileData->name, fileData->nameLength);
    dataCursor[fileData->nameLength + 1] = '\0';
    dataCursor += (fileData->nameLength + 1);    

    int result = Socket_Send(socket, data, bufLength);
    free(data);
    return result;
}

int NetworkSend_ReadFileListing(SOCKET socket, struct NetworkSend_FileListing *fileData) {
    // First, the file name length must be read, in order to know how many bytes to read in total.
    int nameLength;
    int result = Socket_Receive(socket, &nameLength, sizeof(nameLength));
    if (result < 0) {
        return result;
    }

    LOG("Got file name of length %d\n", nameLength);
}

int NetworkSend_ListFiles(SOCKET clientSocket, char* path) {
    struct NetworkSend_Response response;
    struct NetworkSend_FileListing fileData;
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(path, &findData);

    if (findHandle != INVALID_HANDLE_VALUE) {
        // Send an OK response.
        response.status = NETWORKSEND_RESPONSE_STATUS_OK;
        int result = NetworkSend_SendResponse(clientSocket, &response);
        if (result < 0) return result;

        // Iterate through each of the files, to determine the body size.
        do {
            // Ignore directories.
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            printf("Got file: %s\n", findData.cFileName);

            fileData.name = findData.cFileName;
            fileData.nameLength = strlen(fileData.name);
            printf("%s\n", fileData.name);
            NetworkSend_SendFileListing(clientSocket, &fileData);
        } while (FindNextFileA(findHandle, &findData) != 0);        
    } else {
        // Unable to open handle for file search.
        response.status = NETWORKSEND_RESPONSE_STATUS_ERROR;
        int result = NetworkSend_SendResponse(clientSocket, &response);
        if (result < 0) return result;
    }    
}

#endif