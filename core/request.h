#ifndef REQUEST_H_DEFINED
#define REQUEST_H_DEFINED

#include "core.h"
#include "socket.h"

#define NETWORKSEND_REQUEST_COMMAND_LIST_FILES 0
#define NETWORKSEND_REQUEST_COMMAND_DOWNLOAD 1

#define NETWORKSEND_REQUEST_VERSION_1 1

struct NetworkSend_Request {
    char version;
    char commandId;
    char argumentSize;
    char* argument;
};

int NetworkSend_SendRequest(SOCKET socket, struct NetworkSend_Request *request) {
    int bufLength = 0;
    bufLength += sizeof(request->version);
    bufLength += sizeof(request->commandId);
    bufLength += sizeof(request->argumentSize);
    if (request->argumentSize > 0) {
        bufLength += sizeof(char) * (request->argumentSize + 1);
    }

    char *data = (char*)malloc(bufLength);
    data[0] = request->version;
    data[1] = request->commandId;
    data[2] = request->argumentSize;
    if (request->argumentSize > 0) {
        strncpy(data + 3, request->argument, request->argumentSize);
        data[bufLength - 1] = '\0';
    }
    return Socket_Send(socket, data, bufLength);
}

int NetworkSend_ReadRequest(SOCKET socket, struct NetworkSend_Request *request) {
    char data[3];
    int num = Socket_Receive(socket, data, sizeof(data));
    if (num != sizeof(data)) return -1;

    request->version = data[0];
    request->commandId = data[1];
    request->argumentSize = data[2];

    // Read the arguments, if any.
    if (request->argumentSize > 0) {
        int argumentSize = request->argumentSize + 1;
        char* argBuf = (char*) malloc(sizeof(char) * argumentSize);
        num = Socket_Receive(socket, argBuf, argumentSize);
        if (num != argumentSize) return -1;
        request->argument = argBuf;
    } else {
        request->argument = NULL;
    }

    return num;
}

#endif