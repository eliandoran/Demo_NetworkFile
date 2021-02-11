#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "log.h"
#include "socket.h"

#define NETWORKSEND_HOST "127.0.0.1"

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

    LOG("Connected to port %s:%s successfully.\n", NETWORKSEND_HOST, NETWORKSEND_PORT);

    LOG("Client has ended.\n");
    return EXIT_SUCCESS;
}