#ifndef RESPONSE_H_DEFINED
#define RESPONSE_H_DEFINED

#include "core.h"
#include "socket.h"

#define NETWORKSEND_RESPONSE_STATUS_OK 0
#define NETWORKSEND_RESPONSE_STATUS_ERROR 1

struct NetworkSend_Response {
    char status;
};

int NetworkSend_SendResponse(SOCKET socket, struct NetworkSend_Response *response) {
    char data[1];
    data[0] = response->status;
    return Socket_Send(socket, data, sizeof(data));
}

#endif