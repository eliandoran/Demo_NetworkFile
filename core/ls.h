#ifndef LS_H_DEFINED
#define LS_H_DEFINED

#define NETWORKSEND_FILE_BUF_COUNT 2
#define NETWORKSEND_FILE_MAX_NAME MAX_PATH

#include <stdio.h>
#include "core.h"
#include "socket.h"
#include "response.h"

struct NetworkSend_FileListing {    
    int nameLength;
    char* name;
    DWORD lowDateTime;
    DWORD highDateTime;
    DWORD lowFileSize;
    DWORD highFileSize;
};

int NetworkSend_SendMultipleFileListings(SOCKET socket, struct NetworkSend_FileListing *filesData, int numFiles) {
    // Determine the total buffer size needed for sending the file listing data.
    int bufLength = 0;
    for (int i=0; i<numFiles; i++) {
        struct NetworkSend_FileListing* fileData = &filesData[i];
        int nameLength = (fileData->nameLength + 1);    // incl. null terminator
        
        bufLength += sizeof(int);                               // nameLength field
        bufLength += sizeof(char) * nameLength;                 // name field
        bufLength += sizeof(fileData->lowDateTime);
        bufLength += sizeof(fileData->highDateTime);
        bufLength += sizeof(fileData->lowFileSize);
        bufLength += sizeof(fileData->highFileSize);
    }

    // Allocate the buffer.
    char* data = malloc(sizeof(char) * bufLength);    
    char* dataCursor = data;
    ZeroMemory(data, bufLength);

    for (int i=0; i<numFiles; i++) {
        struct NetworkSend_FileListing* fileData = &filesData[i];
        int nameLength = (fileData->nameLength + 1);    // incl. null terminator

        // Set the nameLength field.
        memcpy(dataCursor, &nameLength, sizeof(nameLength));
        dataCursor += sizeof(nameLength);

        // Set the name field.
        memcpy(dataCursor, fileData->name, nameLength);
        dataCursor[nameLength] = '\0';  // ensure null-terminated string
        dataCursor += (nameLength);

        // Set the lowDateTime field.
        memcpy(dataCursor, &fileData->lowDateTime, sizeof(fileData->lowDateTime));
        dataCursor += sizeof(fileData->lowDateTime);

        // Set the highDateTime field.
        memcpy(dataCursor, &fileData->highDateTime, sizeof(fileData->highDateTime));
        dataCursor += sizeof(fileData->highDateTime);

        // Set the lowFileSize field.
        memcpy(dataCursor, &fileData->lowFileSize, sizeof(fileData->lowFileSize));
        dataCursor += sizeof(fileData->lowFileSize);

        // Set the highFileSize field.
        memcpy(dataCursor, &fileData->highFileSize, sizeof(fileData->highFileSize));
        dataCursor += sizeof(fileData->highFileSize);
    }    

    int result = Socket_Send(socket, data, bufLength);
    free(data);
    return result;
}

int NetworkSend_ReadFileListing(SOCKET socket, struct NetworkSend_FileListing fileData[]) {
    // First, the file name length must be read, in order to know how many bytes to read in total.
    int nameLength;
    int result = Socket_Receive(socket, &nameLength, sizeof(nameLength));
    if (result <= 0) return result;
    fileData->nameLength = nameLength;

    // Read the file name.
    char* name = (char*)malloc(sizeof(char) * nameLength);
    result = Socket_Receive(socket, name, sizeof(char) * nameLength);
    if (result <= 0) return result;
    fileData->name = name;

    // Read the low/high date time & size fields.
    DWORD fields[4];
    result = Socket_Receive(socket, &fields, sizeof(fields));
    if (result <= 0) return result;    
    fileData->lowDateTime = fields[0];
    fileData->highDateTime = fields[1];
    fileData->lowFileSize = fields[2];
    fileData->highFileSize = fields[3];    

    return 1;
}

int NetworkSend_ListFiles(SOCKET clientSocket, char* path) {
    struct NetworkSend_Response response;
    struct NetworkSend_FileListing *filesData = (struct NetworkSend_FileListing*)malloc(sizeof(struct NetworkSend_FileListing) * NETWORKSEND_FILE_BUF_COUNT);
    int curBufIndex = 0;
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(path, &findData);

    // Initialize the name string for the file data.
    for (int i=0; i<NETWORKSEND_FILE_BUF_COUNT; i++) {
        filesData[i].name = (char*)malloc(sizeof(char) * NETWORKSEND_FILE_MAX_NAME);
    }

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

            // Parse file name.
            strncpy(filesData[curBufIndex].name, findData.cFileName, (NETWORKSEND_FILE_MAX_NAME - 1));
            filesData[curBufIndex].nameLength = strlen(filesData[curBufIndex].name);

            // Parse file modification date.
            FILETIME fileTime = findData.ftLastWriteTime;
            filesData[curBufIndex].lowDateTime = fileTime.dwLowDateTime;
            filesData[curBufIndex].highDateTime = fileTime.dwHighDateTime;

            // Parse file size.
            filesData[curBufIndex].lowFileSize = findData.nFileSizeLow;
            filesData[curBufIndex].highFileSize = findData.nFileSizeHigh;

            printf("%d %s %d %d\n", curBufIndex, filesData[curBufIndex].name, filesData[curBufIndex].lowFileSize, filesData[curBufIndex].highFileSize);

            curBufIndex++;
            if (curBufIndex >= NETWORKSEND_FILE_BUF_COUNT) {
                NetworkSend_SendMultipleFileListings(clientSocket, filesData, NETWORKSEND_FILE_BUF_COUNT);
                curBufIndex = 0;
            }
        } while (FindNextFileA(findHandle, &findData) != 0);        
    } else {
        // Unable to open handle for file search.
        response.status = NETWORKSEND_RESPONSE_STATUS_ERROR;
        int result = NetworkSend_SendResponse(clientSocket, &response);
        if (result < 0) return result;
    }    
}

#endif