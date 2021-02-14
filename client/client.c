#include <stdio.h>
#include <stdlib.h>

//#define LOG_QUIET
#define LOG_SOCKET

#include "core.h"
#include "ls.h"
#include "log.h"
#include "socket.h"
#include "request.h"
#include "response.h"
#include "format.h"
#include "transfer.h"

#define NETWORKSEND_HOST "127.0.0.1"
#define NETWORKSEND_DATE_BUF 128
#define NETWORKSEND_TIME_BUF 64
#define NETWORKSEND_SIZE_BUF 100

int NetworkSend_DownloadFile(SOCKET connectSocket, char* filePath) {
    int result;
    struct NetworkSend_Request request;
    request.commandId = NETWORKSEND_REQUEST_COMMAND_DOWNLOAD;
    request.version = NETWORKSEND_REQUEST_VERSION_1;
    request.argument = filePath;
    request.argumentSize = strlen(filePath);

    // Send the request.
    result = NetworkSend_SendRequest(connectSocket, &request);
    if (result < 0) return -1;

    // Parse the response.
    struct NetworkSend_Response response;
    result = NetworkSend_ReadResponse(connectSocket, &response);    
    if (result < 0) return -1;

    switch (response.status) {
        case NETWORKSEND_RESPONSE_STATUS_OK:
            LOG("Transfer of file \"%s\" starting.\n", filePath);
            break;

        case NETWORKSEND_RESPONSE_TRANSFER_FILE_NOT_FOUND:
            LOG_ERROR("File \"%s\" not found on remote server.\n", filePath);
            return result;

        default:
            LOG_ERROR("Unable to obtain file \"%s\" on remote server due to an unknown error: %d.\n", filePath, result);
            return result;
    }

    return result;
}

int NetworkSend_RequestFiles(SOCKET connectSocket) {
    int result;
    struct NetworkSend_Request request;
    request.commandId = NETWORKSEND_REQUEST_COMMAND_LIST_FILES;
    request.version = NETWORKSEND_REQUEST_VERSION_1;
    request.argument = NULL;
    request.argumentSize = 0;

    // Send the request.
    result = NetworkSend_SendRequest(connectSocket, &request);
    if (result < 0) return -1;

    // Parse the response.
    struct NetworkSend_Response response;
    result = NetworkSend_ReadResponse(connectSocket, &response);
    if (result < 0 || response.status != NETWORKSEND_RESPONSE_STATUS_OK) {
        LOG_ERROR("Failed to list files on remote server: %d\n", response.status);
        return -1;
    }

    // Read the file entries.
    struct NetworkSend_FileListing fileData;
    char dateBuf[NETWORKSEND_DATE_BUF],
         timeBuf[NETWORKSEND_TIME_BUF],
         sizeBuf[NETWORKSEND_SIZE_BUF];
    int numFiles = 0;

    while (1) {
        result = NetworkSend_ReadFileListing(connectSocket, &fileData);
        if (result < 0) {
            LOG_ERROR("Failed to read an entry in the file list: %d\n", result);
            return -1;
        }

        if (result == 0) {
            LOG("Listing stopped due to connection closing down.\n");
            printf("\n%d files.", numFiles);
            return 0;
        }
        
        NetworkSend_FormatFileDate(fileData.lowDateTime, fileData.highDateTime, dateBuf, NETWORKSEND_DATE_BUF);
        NetworkSend_FormatFileTime(fileData.lowDateTime, fileData.highDateTime, timeBuf, NETWORKSEND_TIME_BUF);
        NetworkSend_FormatFileSize(fileData.lowFileSize, fileData.highFileSize, sizeBuf, NETWORKSEND_SIZE_BUF);

        printf("%s  %s\t %s\t %s\n", dateBuf, timeBuf, sizeBuf, fileData.name);
        numFiles++;
    }    
}

int NetworkSend_HandleConnect(SOCKET connectSocket) {
    LOG("Connected to port %s:%s successfully.\n", NETWORKSEND_HOST, NETWORKSEND_PORT);

    //int result = NetworkSend_RequestFiles(connectSocket);
    int result = NetworkSend_DownloadFile(connectSocket, "server.c");
    if (result < 0) return result;
}

int main() {
    printf("NetworkSend Client\n\n");

    int result = 0;
    if (result = Socket_Initialize()) {
        LOG_ERROR("Unable to initialize socket.\n");
        return EXIT_FAILURE;
    }

    // Create the socket.
    SOCKET connectSocket = Socket_CreateClientSocket(NETWORKSEND_HOST, NETWORKSEND_PORT);
    if (connectSocket == INVALID_SOCKET) {
        LOG_ERROR("Unable to connect to server at host %s on port %s.\n", NETWORKSEND_HOST, NETWORKSEND_PORT);
        Socket_Cleanup();
        return EXIT_FAILURE;
    }

    // Handle the connection to server.    
    result = NetworkSend_HandleConnect(connectSocket);
    if (result < 0) {
        LOG_ERROR("Connection error.\n");
        Socket_Cleanup();
        return EXIT_FAILURE;
    }

    LOG("Client has ended.\n");
    Socket_Cleanup();
    return EXIT_SUCCESS;
}