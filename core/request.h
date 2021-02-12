#ifndef REQUEST_H_DEFINED
#define REQUEST_H_DEFINED

#include "core.h"
#include "socket.h"

#define NETWORKSEND_REQUEST_COMMAND_LIST_FILES 0

#define NETWORKSEND_REQUEST_VERSION_1 1

struct NetworkSend_Request {
    char version;
    char commandId;
};

int NetworkSend_SendRequest(SOCKET socket, struct NetworkSend_Request *request) {
    char data[2];
    data[0] = request->version;
    data[1] = request->commandId;
    return Socket_Send(socket, data, sizeof(data));
}

#endif