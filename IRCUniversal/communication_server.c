//
//  communication.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/5/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//
// This should be generalized to the point that both the server and the client can use these in their respective connections
#include "communication_server.h"

void* sendController(void *data) {
    return NULL;
}

void* recvController(void *data)
{
    Ibd* recvData = (Ibd*)data;
    // Now for the server side receive contoller
    pthread_mutex_lock(recvData->editStatus);
    void *checkConnectionStatus = *recvData->connectionStatus;
    pthread_mutex_unlock(recvData->editStatus);
    
    while (checkConnectionStatus == CONNECTED) {
        // Allocate memory for the new message
        Message *newMsg = malloc(sizeof(Message));
        char *newStr = malloc(64);
        int len = 64, totalRecv = 0;
        ssize_t checkRecv = 0;
        char leading = 'A';
        while(checkRecv >= 0 || leading != '\n')
        {
            checkRecv = recv(recvData->socketID, newStr + totalRecv, len - totalRecv, 0);
            totalRecv += checkRecv;
            leading = newStr[totalRecv - 1];
            if (totalRecv == len) {
                newStr = realloc(newStr, 2*len); checkMemError(newStr);
                len *= 2;
            }
        }
        
        newMsg->msg = newStr;
        newMsg->msgLen = totalRecv;
        
        // Up to this point we have allocated the necessary memory and recorded the entire message from the tcp stream.
        // Now the message has to be placed in queue in the client thread.
        pthread_mutex_lock(recvData->queueMutex);
        addNodeToLL((void*)newMsg, recvData->cmdMsgQueue);
        pthread_mutex_unlock(recvData->queueMutex);
        
        pthread_mutex_lock(recvData->editStatus);
        checkConnectionStatus = *recvData->connectionStatus;
        pthread_mutex_unlock(recvData->editStatus);
    }
    pthread_exit(checkConnectionStatus);
}
