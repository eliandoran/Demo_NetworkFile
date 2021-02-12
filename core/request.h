#ifndef REQUEST_H_DEFINED
#define REQUEST_H_DEFINED

#include "core.h"
#include "socket.h"

struct NetworkSend_Request {
    short version;
    short commandId;
};

int NetworkSend_Send_Request(SOCKET socket, struct NetworkSend_Request request) {

}

#endif