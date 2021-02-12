#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "log.h"
#include "socket.h"
#include "request.h"

#include "ls.h"

int NetworkSend_HandleClient(SOCKET clientSocket) {
    LOG("Client connected.\n");

    char buf[512];
    int result;

    struct NetworkSend_Request request;    
    result = NetworkSend_ReadRequest(clientSocket, &request);
    if (result < 0) return result;

    LOG("Got request from client w/ version #%d and command: %d.\n", request.version, request.commandId);
    
    switch (request.commandId) {
        case NETWORKSEND_REQUEST_COMMAND_LIST_FILES:
            NetworkSend_ListFiles(clientSocket, ".\\*");
            break;

        default:
            LOG_ERROR("Unrecognized client command: %d\n.", request.commandId);
            return -1;
    }
}

int main() {
    printf("NetworkSend Server\n\n");

    int result = 0;
    if (result = Socket_Initialize()) {
        LOG_ERROR("Unable to initialize socket.\n");
        Socket_Cleanup();
        return EXIT_FAILURE;
    }

    // Create the socket.
    SOCKET listenSocket = Socket_CreateServerSocket(NETWORKSEND_PORT);
    if (listenSocket == INVALID_SOCKET) {
        LOG_ERROR("Unable to create socket.\n");
        Socket_Cleanup();
        return EXIT_FAILURE;
    }

    // Listen on the socket.
    result = Socket_Listen(listenSocket);
    if (result == SOCKET_ERROR) {
        LOG_ERROR("Unable to listen for port %s.", NETWORKSEND_PORT);
        Socket_Cleanup();
        return EXIT_FAILURE;
    }

    // Accept a client socket.
    LOG("Server started on port %s.\n", NETWORKSEND_PORT);

    while (1) {
        SOCKET clientSocket = Socket_AcceptClient(listenSocket);
        if (clientSocket == INVALID_SOCKET) {
            LOG_ERROR("Unable to accept client socket.\n");
            Socket_Cleanup();
            return EXIT_FAILURE;                
        }

        // Handle the client connection.
        result = NetworkSend_HandleClient(clientSocket);
        if (result < 0) {
            LOG_ERROR("Connection error.\n");
            Socket_Cleanup();
            return EXIT_FAILURE;
        }

        // Disconnect the send half of the connection.
        Socket_ShutdownSend(clientSocket);

        LOG("---------------\n");
    }   

    LOG("Server has shut down.\n");
    Socket_Cleanup();
    return EXIT_SUCCESS;
}