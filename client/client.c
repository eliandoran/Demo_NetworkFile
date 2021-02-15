#include <stdio.h>
#include <stdlib.h>

#define LOG_QUIET
//#define LOG_SOCKET

#include "core.h"
#include "ls.h"
#include "log.h"
#include "socket.h"
#include "request.h"
#include "response.h"
#include "format.h"
#include "transfer.h"
#include "progress.h"

#define NETWORKSEND_HOST "127.0.0.1"
#define NETWORKSEND_DATE_BUF 128
#define NETWORKSEND_TIME_BUF 64
#define NETWORKSEND_SIZE_BUF 100
#define NETWORKSEND_PROGRESS_BUF 80

char sizeBuf[NETWORKSEND_SIZE_BUF];
char progressBuf[NETWORKSEND_PROGRESS_BUF];
int lastProgressTextLen = 0;

void NetworkSend_DownloadFileCallback(
        unsigned long long bytesWritten,
        unsigned long long fileSize,
        float averageSpeed
) {
    NetworkSend_FormatTransferProgressBar(
            bytesWritten,
            fileSize,
            averageSpeed,
            progressBuf,
            sizeof(progressBuf));
    int curProgressTextLen = strlen(progressBuf);
    printf("\r%s", progressBuf);

    /**
     * Since the progress is outputted on the same line as the previous ones, if the current
     * progress has less characters than the previous one, there will be merged text. In order to
     * fix this, a number of spaces must be added.
     */
    int numSpaces = (lastProgressTextLen - curProgressTextLen);
    if (numSpaces > 0) {
        NetworkSend_FillChar(' ', numSpaces, progressBuf, sizeof(progressBuf));
        printf(progressBuf);
    }

    lastProgressTextLen = curProgressTextLen;
}

int NetworkSend_DownloadFile(SOCKET connectSocket, char* filePath) {
    int result;
    struct NetworkSend_Request request;
    request.commandId = NETWORKSEND_REQUEST_COMMAND_DOWNLOAD;
    request.version = NETWORKSEND_REQUEST_VERSION_1;
    request.argument = filePath;
    request.argumentSize = strlen(filePath);

    // Send the request.
    result = NetworkSend_SendRequest(connectSocket, &request);
    if (result < 0) return -1;

    // Parse the response.
    struct NetworkSend_Response response;
    result = NetworkSend_ReadResponse(connectSocket, &response);    
    if (result < 0) return -1;

    switch (response.status) {
        case NETWORKSEND_RESPONSE_STATUS_OK:
            LOG("Transfer of file \"%s\" starting.\n", filePath);
            break;

        case NETWORKSEND_RESPONSE_TRANSFER_FILE_NOT_FOUND:
            LOG_ERROR("File \"%s\" not found on remote server.\n", filePath);
            return result;

        default:
            LOG_ERROR("Unable to obtain file \"%s\" on remote server due to an unknown error: %d.\n", filePath, result);
            return result;
    }

    // Since the response was OK, read the file transfer info.
    struct NetworkSend_TransferInfo transferInfo;
    result = NetworkSend_ReadTransferInfo(connectSocket, &transferInfo);
    if (result < 0) return -1;

    // Display the file size of the remote file.
    result = NetworkSend_FormatFileSize(transferInfo.fileSizeLow, transferInfo.fileSizeHigh, sizeBuf, sizeof(sizeBuf));
    if (result) {
        LOG("Downloading file \"%s\" of size %s...\n", filePath, sizeBuf);
    }

    // Calculate the file size.
    unsigned long long fileSize = (transferInfo.fileSizeHigh * (MAXDWORD + 1)) + transferInfo.fileSizeLow;

    result = NetworkFile_ReceiveFile(connectSocket, filePath, fileSize, NetworkSend_DownloadFileCallback);
    if (result < 1) {
        LOG_ERROR("Unable to complete transfer: %d\n.", result);
        return -1;
    }

    LOG("Transfer completed successfully.\n");

    return 0;
}

int NetworkSend_RequestFiles(SOCKET connectSocket) {
    int result;
    struct NetworkSend_Request request;
    request.commandId = NETWORKSEND_REQUEST_COMMAND_LIST_FILES;
    request.version = NETWORKSEND_REQUEST_VERSION_1;
    request.argument = NULL;
    request.argumentSize = 0;

    // Send the request.
    result = NetworkSend_SendRequest(connectSocket, &request);
    if (result < 0) return -1;

    // Parse the response.
    struct NetworkSend_Response response;
    result = NetworkSend_ReadResponse(connectSocket, &response);
    if (result < 0 || response.status != NETWORKSEND_RESPONSE_STATUS_OK) {
        LOG_ERROR("Failed to list files on remote server: %d\n", response.status);
        return -1;
    }

    // Read the file entries.
    struct NetworkSend_FileListing fileData;
    char dateBuf[NETWORKSEND_DATE_BUF],
         timeBuf[NETWORKSEND_TIME_BUF];
    int numFiles = 0;

    while (1) {
        result = NetworkSend_ReadFileListing(connectSocket, &fileData);
        if (result < 0) {
            LOG_ERROR("Failed to read an entry in the file list: %d\n", result);
            return -1;
        }

        if (result == 0) {
            LOG("Listing stopped due to connection closing down.\n");
            printf("\n%d files.", numFiles);
            return 0;
        }
        
        NetworkSend_FormatFileDate(fileData.lowDateTime, fileData.highDateTime, dateBuf, NETWORKSEND_DATE_BUF);
        NetworkSend_FormatFileTime(fileData.lowDateTime, fileData.highDateTime, timeBuf, NETWORKSEND_TIME_BUF);
        NetworkSend_FormatFileSize(fileData.lowFileSize, fileData.highFileSize, sizeBuf, NETWORKSEND_SIZE_BUF);

        printf("%s %s %12s  %s\n", dateBuf, timeBuf, sizeBuf, fileData.name);
        numFiles++;
    }    
}

int NetworkSend_HandleListFiles(SOCKET connectSocket) {
    int result = NetworkSend_RequestFiles(connectSocket);
    if (result < 0) return result;
}

int NetworkSend_HandleRetrieveFile(SOCKET connectSocket, char* path) {
    int result = NetworkSend_TransferFile(connectSocket, path);
    if (result < 0) return result;
}

void NetworkSend_DisplayUsage(char *programName) {
    printf("Usage: %s [host] ls\n", programName);
}

int main(int argc, char** argv) {
    printf("NetworkSend Client\n\n");

    if (argc < 3 || argc > 4) {
        NetworkSend_DisplayUsage(argv[0]);
        return EXIT_FAILURE;
    }

    char* host = argv[1];
    char* commandText = argv[2];
    char* arg = NULL;
    int commandId = NetworkSend_GetCommandIdFromText(commandText);

    switch (commandId) {
        case NETWORKSEND_REQUEST_COMMAND_LIST_FILES:
            break;

        case NETWORKSEND_REQUEST_COMMAND_DOWNLOAD:
            if (argc != 4) {
                NetworkSend_DisplayUsage(argv[0]);
                return EXIT_FAILURE;
            }
            
            arg = argv[3];
            break;

        default:
            printf("Unknown command: %s.\n", commandText);
            return -1;
    }

    int result = 0;
    if (result = Socket_Initialize()) {
        LOG_ERROR("Unable to initialize socket.\n");
        return EXIT_FAILURE;
    }

    // Create the socket.
    SOCKET connectSocket = Socket_CreateClientSocket(NETWORKSEND_HOST, NETWORKSEND_PORT);
    if (connectSocket == INVALID_SOCKET) {
        LOG_ERROR("Unable to connect to server at host %s on port %s.\n", NETWORKSEND_HOST, NETWORKSEND_PORT);
        Socket_Cleanup();
        return EXIT_FAILURE;
    }

    // Handle the connection to server.    
    LOG("Connected to port %s:%s successfully.\n", NETWORKSEND_HOST, NETWORKSEND_PORT);

    switch (commandId) {
        case NETWORKSEND_REQUEST_COMMAND_LIST_FILES:
            result = NetworkSend_HandleListFiles(connectSocket);
            break;

        case NETWORKSEND_REQUEST_COMMAND_DOWNLOAD:
            result = NetworkSend_DownloadFile(connectSocket, arg);
            break;
    }

    if (result < 0) {
        LOG_ERROR("Connection error.\n");
        Socket_Cleanup();
        return EXIT_FAILURE;
    }


    LOG("Client has ended.\n");
    Socket_Cleanup();
    return EXIT_SUCCESS;
}