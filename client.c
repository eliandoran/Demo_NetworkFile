#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "log.h"
#include "socket.h"

#define NETWORKSEND_HOST "127.0.0.1"

int NetworkSend_HandleConnect(SOCKET connectSocket) {
    LOG("Connected to port %s:%s successfully.\n", NETWORKSEND_HOST, NETWORKSEND_PORT);

    char* message = "Hello world";
    int result;
    
    Socket_Send(connectSocket, message, strlen(message));
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
        return EXIT_FAILURE;
    }

    // Handle the connection to server.    
    result = NetworkSend_HandleConnect(connectSocket);
    if (result < 0) {
        LOG_ERROR("Connection error.\n");
        return EXIT_FAILURE;
    }

    LOG("Client has ended.\n");
    return EXIT_SUCCESS;
}