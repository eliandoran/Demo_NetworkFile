#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "log.h"
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

int NetworkSend_HandleClient(SOCKET clientSocket) {
    LOG("Client connected.\n");
    char buf[512];
    int result;

    Socket_Receive(clientSocket, buf, 512);
    if (result < 0) return result;

    LOG("%s\n", buf);
}

int main() {
    printf("NetworkSend Server\n\n");

    int result = 0;
    if (result = Socket_Initialize()) {
        LOG_ERROR("Unable to initialize socket.\n");
        return EXIT_FAILURE;
    }

    // Create the socket.
    SOCKET listenSocket = Socket_CreateServerSocket(NETWORKSEND_PORT);
    if (listenSocket == INVALID_SOCKET) {
        LOG_ERROR("Unable to create socket.\n");
        return EXIT_FAILURE;
    }

    // Listen on the socket.
    result = Socket_Listen(listenSocket);
    if (result == SOCKET_ERROR) {
        LOG_ERROR("Unable to listen for port %s.", NETWORKSEND_PORT);
        return EXIT_FAILURE;
    }

    // Accept a client socket.
    LOG("Server started on port %s.\n", NETWORKSEND_PORT);

    SOCKET clientSocket = Socket_AcceptClient(listenSocket);
    if (clientSocket == INVALID_SOCKET) {
        LOG_ERROR("Unable to accept client socket.\n");
        return EXIT_FAILURE;                
    }

    // Handle the client connection.
    result = NetworkSend_HandleClient(clientSocket);
    if (result < 0) {
        LOG_ERROR("Connection error.\n");
        return EXIT_FAILURE;
    }

    LOG("Server has shut down.\n");
    return EXIT_SUCCESS;
}