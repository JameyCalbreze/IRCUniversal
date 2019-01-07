//
//  communication.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/5/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//
// This should be generalized to the point that both the server and the client can use these in their respective connections
#include "communication.h"
#include "server.h"
#include "errorCheck.h"

void* sendController(void* data)
{
    // Send the welcome message
    // This shouldn't occur until the send thread has been initialized
    //    char *tempMessage = "Please login with /login [user]\n";
    //    unsigned long lenMessage = strlen(tempMessage);
    //    ssize_t sent = 0;
    //    while (lenMessage - sent){
    //        sent += send(socketID, tempMessage, lenMessage - sent, 0);
    //        // If we get an error here we have to handle it as a bad connection can not be responsible for bringing the server down.
    //        status = checkSendError(sent,socketID);
    //        if(status){
    //            connectionStatus = ERROR_DROP;
    //        }
    //    }
    obd* sendData = (obd*)data;
    pthread_cond_wait(&sendData->fireOff, &sendData->queueSend);
    return NULL;
}

void* recvController(void* data)
{
    ibd* recvData = (ibd*)data;
    ssize_t check;
    char *buffer = "test\n";
    check = recv(recvData->socketID, buffer, (size_t)strlen(buffer), 0);
    return NULL;
}

void cleanSendData(obd* sendData)
{
    int status;
    status = pthread_mutex_destroy(&sendData->queueSend); checkMutexErr(status);
    status = pthread_cond_destroy(&sendData->fireOff); checkMutexErr(status);
    clearMsgChain(sendData->messages);
    free(sendData);
    return;
}

void cleanRecvData(ibd* recvData)
{
    clearMsgChain(recvData->working);
    free(recvData);
    return;
}

void clearMsgChain(struct message* curMsg)
{
    struct message* nxtMsg;
    while (curMsg != NULL) {
        free(curMsg->msg);
        nxtMsg = curMsg->nextMsg;
        free(curMsg);
        curMsg = nxtMsg;
    }
    return;
}