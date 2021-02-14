#ifndef TRANSFER_H
#define TRANSFER_h

#include "core.h"
#include "socket.h"
#include "response.h"

#define NETWORKSEND_RESPONSE_TRANSFER_FILE_NOT_FOUND 1
#define NETWORKSEND_RESPONSE_TRANSFER_IO_ERROR 2

#define NETWORKSEND_TRANSFER_BUFFER_SIZE 16

struct NetworkSend_TransferInfo {
    DWORD fileSizeLow;
    DWORD fileSizeHigh;
};

int NetworkSend_SendTransferInfo(SOCKET socket, struct NetworkSend_TransferInfo *transferInfo) {    
    // Determine the total buffer size needed for the transfer info.
    int bufLength = 0;
    bufLength += sizeof(transferInfo->fileSizeLow);
    bufLength += sizeof(transferInfo->fileSizeHigh);

    // Allocate the buffer.
    char* data = malloc(sizeof(char) * bufLength);
    char* dataCursor = data;
    ZeroMemory(data, bufLength);

    // Set the fileSizeLow field.
    memcpy(dataCursor, &transferInfo->fileSizeLow, sizeof(transferInfo->fileSizeHigh));
    dataCursor += sizeof(transferInfo->fileSizeHigh);

    // Set the fileSizeHigh field.
    memcpy(dataCursor, &transferInfo->fileSizeHigh, sizeof(transferInfo->fileSizeHigh));
    dataCursor += sizeof(transferInfo->fileSizeHigh);

    return Socket_Send(socket, data, bufLength);
}

int NetworkSend_ReadTransferInfo(SOCKET socket, struct NetworkSend_TransferInfo *transferInfo) {
    DWORD fields[2];
    int result = Socket_Receive(socket, &fields, sizeof(fields));
    if (result <= 0) return result;

    transferInfo->fileSizeLow = fields[0];
    transferInfo->fileSizeHigh = fields[1];
    return 1;
}

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

    // Obtain the file size.
    DWORD fileSizeLow, fileSizeHigh;
    fileSizeLow = GetFileSize(file, &fileSizeHigh);
    if (fileSizeLow == INVALID_FILE_SIZE) {
        LOG_ERROR("Unable to determine file size: %d\n", GetLastError());
        response.status = NETWORKSEND_RESPONSE_TRANSFER_IO_ERROR;
        result = NetworkSend_SendResponse(clientSocket, &response);
        return result;
    }

    // Send the transfer info.
    struct NetworkSend_TransferInfo transferInfo;
    transferInfo.fileSizeLow = fileSizeLow;
    transferInfo.fileSizeHigh = fileSizeHigh;
    result = NetworkSend_SendTransferInfo(clientSocket, &transferInfo);
    if (result < 0) return result;

    // Transfer the actual file.
    char buffer[NETWORKSEND_TRANSFER_BUFFER_SIZE];
    DWORD bytesRead = 0;

    while (1) {
        result = ReadFile(file, buffer, sizeof(buffer), &bytesRead, NULL);
        if (!result) {
            LOG_ERROR("Read error for \"%s\": %d\n", path, GetLastError());
            return result;
        }

        if (bytesRead > 0) {
            result = Socket_Send(clientSocket, buffer, bytesRead);
            if (result < 0) return result;
        } else {
            return 1;
        }
    }

    return result;
}

int NetworkFile_ReceiveFile(SOCKET socket, char* path) {
    char buffer[NETWORKSEND_TRANSFER_BUFFER_SIZE];
    int bytesRead;
    
    do {
        bytesRead = Socket_Receive(socket, buffer, sizeof(buffer));

        // Connection is shutting down.
        if (bytesRead == 0) {
            return 1;
        }
    } while (bytesRead > 0);
}

#endif