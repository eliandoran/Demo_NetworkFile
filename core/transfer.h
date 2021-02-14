#ifndef TRANSFER_H
#define TRANSFER_h

#include "core.h"
#include "socket.h"
#include "response.h"

int NetworkSend_TransferFile(SOCKET clientSocket, char* path) {
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    int lastError = GetLastError();
    if (lastError != 0) {
        LOG_ERROR("Unable to open file: %d\n", lastError);
        return -1;
    }

    return 0;
}

#endif