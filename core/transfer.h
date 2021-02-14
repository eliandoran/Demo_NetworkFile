#ifndef TRANSFER_H
#define TRANSFER_h

#include "core.h"
#include "socket.h"
#include "response.h"

#define NETWORKSEND_RESPONSE_TRANSFER_FILE_NOT_FOUND 1

int NetworkSend_TransferFile(SOCKET clientSocket, char* path) {
    struct NetworkSend_Response response;
    int result;
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    // Check for file opening errors.
    int lastError = GetLastError();
    if (lastError != 0) {
        LOG_ERROR("Unable to open file: %d\n", lastError);
        response.status = NETWORKSEND_RESPONSE_TRANSFER_FILE_NOT_FOUND;
        result = NetworkSend_SendResponse(clientSocket, &response);
        return result;
    }

    // Send a successful response.
    response.status = NETWORKSEND_RESPONSE_STATUS_OK;
    result = NetworkSend_SendResponse(clientSocket, &response);
    return result;
}

#endif