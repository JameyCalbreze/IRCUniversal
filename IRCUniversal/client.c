//
//  client.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "client.h"
#include "errorCheck.h"
#include "networkHelp.h"
#include "communication_client.h"

struct message *readMessage(void)
{
    // Create the new message
    struct message *newMessage = malloc(sizeof(struct message)); checkMemError(newMessage);
    newMessage->nextMsg = NULL;
    // Base message size will be 64 bytes
    char *stringMsg = malloc(64); checkMemError(stringMsg);
    memset((void*)stringMsg,0,64);
    
    // Now we need to read from the standard input
    int lenString = 64, numChar = 0;
    char *pos = stringMsg;
    char cur = 'A';
    while (cur != '\n') {
        // Now we're going to read one character at a time until we hit a new line
        cur = fgetc(stdin);
        *pos = cur; pos++; numChar++;
        if (numChar == lenString) {
            stringMsg = realloc(stringMsg, 2*lenString); checkMemError(stringMsg);
            lenString *= 2;
            pos = (stringMsg + numChar);
        }
    }
    newMessage->msg = stringMsg;
    newMessage->msgLen = numChar;
    return newMessage;
}

// The parameter preferred reffers to the IP protocol used to make a connection.
int client_main(const char* hostname, int port, int preferred)
{
    // The first thing we need to do is to get the IP v6 address from the hostname
    struct addrinfo* determine;
    struct addrinfo hints;
    setHintsForAddress(&hints);
    char portStr[8];
    sprintf(portStr,"%d",port);
    int status = getaddrinfo(hostname,portStr,&hints,&determine); checkGetAddrErr(status,__LINE__);
    
    // Create the socket
    int socketID = socket(preferred,SOCK_STREAM,0);
    
    // Now that we've made the code IPvX we can just select the propper protocol using a for loop
    while (determine != NULL) {
        if (determine->ai_family == preferred && determine->ai_protocol == IPPROTO_TCP) break;
        determine = determine->ai_next;
    }
    checkFinAddr((int*)determine);
    
    status = connect(socketID,determine->ai_addr,determine->ai_addrlen); checkConnectErr(status,__LINE__);
    int connectionStatus = CONNECTED;
    
    // Create mutex and condition for the send thread to wake the client
    // Not actually sure if I need to use this in the implementation
    pthread_mutex_t waitMutex;
    pthread_cond_t wakeClient;
    status = pthread_mutex_init(&waitMutex,NULL); checkMutexErr(status);
    status = pthread_cond_init(&wakeClient,NULL); checkMutexErr(status);
    
    // Create the send thread
    obd* sendData = malloc(sizeof(obd));
    sendData->messages = NULL;
    sendData->socketID = socketID;
    sendData->connectionStatus = &connectionStatus;
    sendData->editStatus = &waitMutex;
    status = pthread_mutex_init(&sendData->queueSend, NULL); checkMutexErr(status);
    status = pthread_cond_init(&sendData->fireOff,NULL); checkMutexErr(status);
    
    // Create the recv thread
    // Currently not formatted for the client to recveive commands from the server. Will be implemented at some point.
    ibd* recvData = malloc(sizeof(ibd));
    recvData->socketID = socketID;
    recvData->queueMutex = NULL;
    recvData->wakeClient = NULL;
    recvData->working = NULL;
    recvData->connectionStatus = &connectionStatus;
    recvData->editStatus = &waitMutex;
    
    // Create threads
    status = pthread_create(&sendData->tid, NULL, sendController, (void*)sendData); checkThreadError(status);
    status = pthread_create(&recvData->tid,NULL,recvController,(void*)recvData); checkThreadError(status);
    
    // Once the threads have been created the client must enter the equence
    do {
        // Begin the client sequence
        // First step is to read from the stdin in for a string of any size.
        struct message *readFromConsole = readMessage();
        
        // Now we have the message from the console. We need to place the message in the send queue
        pthread_mutex_lock(&sendData->queueSend);
        struct message *seekLatest = sendData->messages;
        if (seekLatest == NULL) {sendData->messages = readFromConsole;}
        else {
            while (seekLatest->nextMsg != NULL) seekLatest = seekLatest->nextMsg;
            seekLatest->nextMsg = readFromConsole;
        }
        pthread_mutex_unlock(&sendData->queueSend);
    } while (connectionStatus == CONNECTED);
    
    int waitPeriod = 300;
    wait((void*)&waitPeriod);
    
    connectionStatus = CLEAN_DROP;
    // Clean up on exit
    pthread_cond_broadcast(&sendData->fireOff);
    pthread_join(sendData->tid,NULL);
    status = pthread_kill(recvData->tid,SIGTERM); checkThreadError(status);
    pthread_join(recvData->tid,NULL);
    cleanSendData(sendData);
    cleanRecvData(recvData);
    close(socketID);
    
    status = pthread_mutex_destroy(&waitMutex); checkMutexErr(status);
    status = pthread_cond_destroy(&wakeClient); checkMutexErr(status);
    return 0;
}
