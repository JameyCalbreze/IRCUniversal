//
//  communication.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/5/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//
// This should be generalized to the point that both the server and the client can use these in their respective connections
#include "communication_server.h"
#include "server.h"
#include "errorCheck.h"

void* sendController(void* data)
{
    // Cast the pointer to the out bound data structure
    obd* sendData = (obd*)data;
    int status;
    
    pthread_mutex_lock(sendData->editStatus);
    int checkConnectionStatus = *sendData->connectionStatus;
    pthread_mutex_unlock(sendData->editStatus);
    
    while (checkConnectionStatus == CONNECTED) {
        // First thing we're going to do is wait for the client thread to wake us up
        pthread_cond_wait(&sendData->fireOff,&sendData->queueSend);
            // Let's take the messages out of the shared data and keep them on a temporary pointer
        while(sendData->messages != NULL) {
            struct message *curMsg = sendData->messages;
            int sent = 0;
            size_t leftToSend = curMsg->msgLen;
            while (leftToSend) {
                ssize_t checkSend = send(sendData->socketID,curMsg->msg + sent,leftToSend,0);
                status = checkSendError(checkSend, sendData->socketID);
                if(status) {
                    // This will only occur on a broken socket
                    // Has been reworked to keep the mutex locked while all messages are being sent. This way we won't have a miss fire
                    // where this sending thread is looking to send messages and not waiting on the broadcast
                    pthread_mutex_lock(sendData->editStatus);
                    *sendData->connectionStatus = ERROR_DROP;
                    pthread_mutex_unlock(sendData->editStatus);
                    pthread_exit((void*)ERROR_DROP);
                } else {
                    // Adjust the data position
                    sent += checkSend;
                    leftToSend -= checkSend;
                }
            }
            // If we've made it this far then the message has been successfully transmitted
            // therefore the message is no longer needed in memory;
            sendData->messages = curMsg->nextMsg;
            free(curMsg->msg);
            free(curMsg);
        }
        pthread_mutex_unlock(&sendData->queueSend);
        pthread_mutex_lock(sendData->editStatus);
        checkConnectionStatus = *sendData->connectionStatus;
        pthread_mutex_unlock(sendData->editStatus);
    }
    long long convertStatus = checkConnectionStatus;
    pthread_exit((void*)convertStatus);
}

void* recvController(void* data)
{
    ibd* recvData = (ibd*)data;
    // Now for the server side receive contoller
    pthread_mutex_lock(recvData->editStatus);
    int checkConnectionStatus = *recvData->connectionStatus;
    pthread_mutex_unlock(recvData->editStatus);
    
    while (checkConnectionStatus == CONNECTED) {
        // Allocate memory for the new message
        struct message *newMsg = malloc(sizeof(struct message));
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
        addToMsgChain(recvData->cmdMsgQueue,newMsg);
        pthread_mutex_unlock(recvData->queueMutex);
        
        pthread_mutex_lock(recvData->editStatus);
        checkConnectionStatus = *recvData->connectionStatus;
        pthread_mutex_unlock(recvData->editStatus);
    }
    long long convertStatus = checkConnectionStatus;
    pthread_exit((void*)convertStatus);
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

void addToMsgChain(struct message **chain, struct message *newMsg)
{
    if(*chain == NULL) {
        *chain = newMsg;
        return;
    }
    struct message *nextMsg = *chain;
    while (nextMsg->nextMsg != NULL) {
        nextMsg = nextMsg->nextMsg;
    }
    nextMsg->nextMsg = newMsg;
    return;
}
