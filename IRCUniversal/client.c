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
#include "communication.h"

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
    status = pthread_mutex_init(&sendData->queueSend, NULL); checkMutexErr(status);
    status = pthread_cond_init(&sendData->fireOff,NULL); checkMutexErr(status);
    status = pthread_create(&sendData->tid, NULL, sendController, (void*)sendData); checkThreadError(status);
    
    // Create the recv thread
    // Currently not formatted for the client to recveive commands from the server. Will be implemented at some point.
    ibd* recvData = malloc(sizeof(ibd));
    recvData->socketID = socketID;
    recvData->queueMutex = NULL;
    recvData->wakeClient = NULL;
    recvData->working = NULL;
    status = pthread_create(&recvData->tid,NULL,recvController,(void*)recvData); checkThreadError(status);
    
    // Clean up on exit
    status = pthread_kill(sendData->tid,SIGTERM); checkThreadError(status);
    pthread_join(sendData->tid,NULL);
    status = pthread_kill(recvData->tid,SIGTERM); checkThreadError(status);
    pthread_join(recvData->tid,NULL);
    cleanSendData(sendData);
    cleanRecvData(recvData);
    
    status = pthread_mutex_destroy(&waitMutex); checkMutexErr(status);
    status = pthread_cond_destroy(&wakeClient); checkMutexErr(status);
    return 0;
}
