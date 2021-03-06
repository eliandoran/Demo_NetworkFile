#ifndef SOCKET_H_DEFINED
#define SOCKET_H_DEFINED

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "log.h"

#pragma comment(lib, "ws2_32.lib")

int Socket_Initialize() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void Socket_Cleanup() {
    LOG("Cleaning up socket.\n");
    WSACleanup();
}

SOCKET Socket_CreateServerSocket(char* port) {
    struct addrinfo hints, *result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server.
    int resultCode = getaddrinfo(NULL, port, &hints, &result);
    if (resultCode != 0) {
        return INVALID_SOCKET;
    }

    // Create a socket to listen for client connections.
    SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        return INVALID_SOCKET;
    }

    // Setup the TCP listening socket.
    resultCode = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (resultCode == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(listenSocket);
        return INVALID_SOCKET;
    }

    return listenSocket;
}

SOCKET Socket_CreateClientSocket(char* host, char* port) {
    struct addrinfo hints, *result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;    

    // Resolve the server address and port.
    int resultCode = getaddrinfo(host, port, &hints, &result);
    if (resultCode != 0) {
        return INVALID_SOCKET;
    }

    // Create a socket for connecting to the server.
    SOCKET connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connectSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        return INVALID_SOCKET;
    }    

    // Connect to server.
    resultCode = connect(connectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (resultCode == SOCKET_ERROR) {
        // Ideally, if connect fails, the next address returned by getaddrinfo should be used instead.
        closesocket(connectSocket);
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);
    return connectSocket;
}

int Socket_Listen(SOCKET listenSocket) {
    int resultCode = listen(listenSocket, SOMAXCONN);
    if (resultCode == SOCKET_ERROR) {
        closesocket(listenSocket);
    }

    return resultCode;
}

SOCKET Socket_AcceptClient(SOCKET listenSocket) {
    SOCKET result = accept(listenSocket, NULL, NULL);
    if (result == INVALID_SOCKET) {
        LOG_ERROR("AcceptClient: %d\n", WSAGetLastError());
        closesocket(listenSocket);
    }

    return result;
}

void Socket_LogBytes(const char* buffer, int bufferLen) {
    for (int i=0; i<bufferLen; i++) {
        printf("%d ", (int)buffer[i]);
    }

    printf("\n");
}

int Socket_Send(SOCKET socket, const void* buffer, int bufferLen) {    
    const char *curPointer = (char*) buffer;

    #ifdef LOG_SOCKET
    printf("[SEND] "); Socket_LogBytes(curPointer, bufferLen);
    #endif

    int remaining = bufferLen;

    while (remaining > 0) {
        int num = send(socket, curPointer, remaining, 0);
        if (num == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                // optional: use select() to check for timeout to fail the send.
                continue;
            }

            closesocket(socket);
            return -1;
        }

        curPointer += num;
        remaining -= num;
    }

    #ifdef LOG_SOCKET
    LOG("Sent %d bytes.\n", bufferLen);    
    #endif
    
    return bufferLen;
}

int Socket_SendString(SOCKET socket, const char* string) {
    return Socket_Send(socket, string, strlen(string) + 1);
}

int Socket_Receive(SOCKET socket, void* buffer, int bufferLen) {
    char *curPointer = (char*) buffer;
    int remaining = bufferLen;
    int received = 0;

    while (remaining > 0) {
        int num = recv(socket, curPointer, remaining, 0);
        if (num == SOCKET_ERROR) {
             if (WSAGetLastError() == WSAEWOULDBLOCK) {
                // optional: use select() to check for timeout to fail the send.
                continue;
            }

            closesocket(socket);
            return -1;
        } else if (num == 0) {
            LOG("Connection closing...\n");
            return received;
        }

        #ifdef LOG_SOCKET
        LOG("Received %d bytes.\n", num);
        printf("[RECV] "); Socket_LogBytes(curPointer, num);
        #endif

        received += num;
        remaining -= num;
    }

    return received;
}

int Socket_ShutdownSend(SOCKET socket) {
    int result = shutdown(socket, SD_SEND);
    if (result == SOCKET_ERROR) {
        closesocket(socket);
        return -1;
    }

    return 0;
}

#endif