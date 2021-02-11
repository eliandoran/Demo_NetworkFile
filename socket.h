#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

int Socket_Initialize() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

SOCKET Socket_Create(char* port) {
    struct addrinfo hints, *result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server.
    int resultCode = getaddrinfo(NULL, port, &hints, &result);
    if (resultCode != 0) {
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Create a socket to listen for client connections.
    SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Setup the TCP listening socket.
    resultCode = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (resultCode == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return listenSocket;
}

int Socket_Listen(SOCKET socket) {
    int resultCode = listen(socket, SOMAXCONN);
    if (resultCode == SOCKET_ERROR) {
        closesocket(socket);
        WSACleanup();
    }

    return resultCode;
}