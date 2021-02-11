#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "socket.h"

void ListFiles(char* path) {
    WIN32_FIND_DATA findData;
    HANDLE findHandle = FindFirstFile(path, &findData);

    if (findHandle == INVALID_HANDLE_VALUE) {
        // Unable to open handle for file search.
        return;
    }

    // Iterate through each of the files.
    do {
        printf("%s\n", findData.cFileName);
    } while (FindNextFile(findHandle, &findData) != 0);
}

void NetworkSend_HandleClient(SOCKET clientSocket) {
    printf("Client connected.\n");
}

int main() {
    int result = 0;
    if (result = Socket_Initialize()) {
        printf("Unable to initialize socket.\n");
        return EXIT_FAILURE;
    }

    // Create the socket.
    SOCKET listenSocket = Socket_CreateServerSocket(NETWORKSEND_PORT);
    if (listenSocket == INVALID_SOCKET) {
        printf("Unable to create socket.\n");
        return EXIT_FAILURE;
    }

    // Listen on the socket.
    result = Socket_Listen(listenSocket);
    if (result == SOCKET_ERROR) {
        printf("Unable to listen for port %s.", NETWORKSEND_PORT);
        return EXIT_FAILURE;
    }

    // Accept a client socket.
    printf("Server started on port %s.\n", NETWORKSEND_PORT);

    SOCKET clientSocket = Socket_AcceptClient(listenSocket);
    if (clientSocket == INVALID_SOCKET) {
        printf("Unable to accept client socket.\n");
        return EXIT_FAILURE;                
    }
    NetworkSend_HandleClient(clientSocket);

    printf("Server has shut down.\n");
    return EXIT_SUCCESS;
}