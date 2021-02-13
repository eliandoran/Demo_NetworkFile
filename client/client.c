#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "ls.h"
#include "log.h"
#include "socket.h"
#include "request.h"
#include "response.h"

#define NETWORKSEND_HOST "127.0.0.1"

int NetworkSend_RequestFiles(SOCKET connectSocket) {
    int result;
    struct NetworkSend_Request request;
    request.commandId = NETWORKSEND_REQUEST_COMMAND_LIST_FILES;
    request.version = NETWORKSEND_REQUEST_VERSION_1;

    // Send the request.
    result = NetworkSend_SendRequest(connectSocket, &request);
    if (result < 0) return -1;

    // Parse the response.
    struct NetworkSend_Response response;
    result = NetworkSend_ReadResponse(connectSocket, &response);
    if (response.status != NETWORKSEND_RESPONSE_STATUS_OK) {
        LOG_ERROR("Failed to list files on remote server: %d\n", response.status);
        return -1;
    }

    // Read the file entries.
    struct NetworkSend_FileListing fileData;

    while (1) {
        result = NetworkSend_ReadFileListing(connectSocket, &fileData);
        if (result < 0) {
            LOG_ERROR("Failed to read an entry in the file list: %d\n", result);
            return -1;
        }

        if (result == 0) {
            LOG("Listing stopped due to connection closing down.\n");
            return 0;
        }

        printf("%s %d %d\n", fileData.name, fileData.lowDateTime, fileData.highDateTime);
    }
}

int NetworkSend_HandleConnect(SOCKET connectSocket) {
    LOG("Connected to port %s:%s successfully.\n", NETWORKSEND_HOST, NETWORKSEND_PORT);

    int result = NetworkSend_RequestFiles(connectSocket);
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