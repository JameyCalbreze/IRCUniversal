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
    // Cast the pointer to the out bound data structure
    obd* sendData = (obd*)data;
    int status;
    
    while (1) {
        // First thing we're going to do is wait for the client thread to wake us up
        pthread_cond_wait(&sendData->fireOff,&sendData->queueSend);
        // Now we're going to need a way to check if the send thread needs to terminate.
        pthread_mutex_lock(sendData->editStatus);
        int checkConnectionStatus = *sendData->connectionStatus;
        pthread_mutex_unlock(sendData->editStatus);
        if(checkConnectionStatus == CONNECTED) {
            // Let's take the messages out of the shared data and keep them on a temporary pointer
            struct message* tempHold = sendData->messages;
            sendData->messages = NULL;
            pthread_mutex_unlock(&sendData->queueSend);
            while(tempHold != NULL) {
                struct message *curMsg = tempHold;
                int sent = 0;
                size_t leftToSend = curMsg->msgLen;
                while (leftToSend) {
                    ssize_t checkSend = send(sendData->socketID,curMsg->msg + sent,leftToSend,0);
                    status = checkSendError(checkSend, sendData->socketID);
                    if(status) {
                        // This will only occur on a broken socket
                        // Clear the messages we removed from the queue and then set the connection status in the main thread
                        // The conditions required to exit the while loops will be set
                        clearMsgChain(tempHold);
                        tempHold = NULL;
                        pthread_mutex_lock(sendData->editStatus);
                        *sendData->connectionStatus = ERROR_DROP;
                        pthread_mutex_unlock(sendData->editStatus);
                        leftToSend = 0;
                    } else {
                        // Adjust the data position
                        sent += checkSend;
                        leftToSend -= checkSend;
                    }
                }
            }
        } else {
            pthread_mutex_unlock(&sendData->queueSend);
            int *test = &checkConnectionStatus;
            pthread_exit((void*)test);
        }
        
    }
    
    //pthread_cond_wait(&sendData->fireOff, &sendData->queueSend);
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
