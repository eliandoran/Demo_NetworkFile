#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "log.h"
#include "socket.h"
#include "request.h"

#define NETWORKSEND_HOST "127.0.0.1"

int NetworkSend_RequestFiles(SOCKET connectSocket) {
    struct NetworkSend_Request request;
    request.commandId = NETWORKSEND_REQUEST_COMMAND_LIST_FILES;
    request.version = NETWORKSEND_REQUEST_VERSION_1;
    return NetworkSend_SendRequest(connectSocket, &request);
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