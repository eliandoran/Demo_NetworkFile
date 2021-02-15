#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>

#define NETWORKSEND_REQUEST_COMMAND_LIST_FILES 0
#define NETWORKSEND_REQUEST_COMMAND_DOWNLOAD 1

#define NETWORKSEND_REQUEST_COMMAND_TEXT_BUFSIZE 10
#define NETWORKSEND_REQUEST_COMMAND_LIST_FILES_TEXT "LS"
#define NETWORKSEND_REQUEST_COMMAND_DOWNLOAD_TEXT "GET"

int NetworkSend_GetCommandTextFromId(char commandId, char* buf, int bufSize) {
    char* commandText;
    switch (commandId) {
        case NETWORKSEND_REQUEST_COMMAND_LIST_FILES:
            commandText=NETWORKSEND_REQUEST_COMMAND_LIST_FILES_TEXT;
            break;
        case NETWORKSEND_REQUEST_COMMAND_DOWNLOAD:
            commandText=NETWORKSEND_REQUEST_COMMAND_DOWNLOAD_TEXT;
            break;
        default:
            return -1;
    }

    int commandLength = strlen(commandText);
    if (commandLength >= bufSize) {
        return -2;
    }

    strcpy(buf, commandText);
    return commandLength;
}

int NetworkSend_GetCommandIdFromText(char* command) {
    if (!stricmp(command, NETWORKSEND_REQUEST_COMMAND_LIST_FILES_TEXT))
        return NETWORKSEND_REQUEST_COMMAND_LIST_FILES;
    
    if (!stricmp(command, NETWORKSEND_REQUEST_COMMAND_DOWNLOAD_TEXT))
        return NETWORKSEND_REQUEST_COMMAND_DOWNLOAD;

    return -1;
}

#endif