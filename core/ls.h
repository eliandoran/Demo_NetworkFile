#ifndef LS_H_DEFINED
#define LS_H_DEFINED

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

int NetworkSend_SendFileListing(SOCKET socket, struct NetworkSend_FileListing *fileData) {
    int nameLength = (fileData->nameLength + 1);    // incl. null terminator

    // Determine the total buffer size needed for sending the file listing data.
    int bufLength = 0;
    bufLength += sizeof(int);                               // nameLength field
    bufLength += sizeof(char) * nameLength;                 // name field
    bufLength += sizeof(fileData->lowDateTime);
    bufLength += sizeof(fileData->highDateTime);
    bufLength += sizeof(fileData->lowFileSize);
    bufLength += sizeof(fileData->highFileSize);

    // Allocate the buffer.
    char* data = (char*)malloc(bufLength);
    char* dataCursor = data;
    ZeroMemory(data, bufLength);

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

    int result = Socket_Send(socket, data, bufLength);
    free(data);
    return result;
}

int NetworkSend_ReadFileListing(SOCKET socket, struct NetworkSend_FileListing *fileData) {
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

            // Parse file name.
            fileData.name = findData.cFileName;
            fileData.nameLength = strlen(fileData.name);

            // Parse file modification date.
            FILETIME fileTime = findData.ftLastWriteTime;
            fileData.lowDateTime = fileTime.dwLowDateTime;
            fileData.highDateTime = fileTime.dwHighDateTime;

            // Parse file size.
            fileData.lowFileSize = findData.nFileSizeLow;
            fileData.highFileSize = findData.nFileSizeHigh;

            printf("%s %d %d\n", fileData.name, fileData.lowFileSize, fileData.highFileSize);
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