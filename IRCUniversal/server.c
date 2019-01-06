//
//  server.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "server.h"
#include "errorCheck.h"
#include "networkHelp.h"
#include "communication.h"

// These threads are initialized as detached. There needs to be a way to join all of these threads
void* usrMngr(void* data)
{
    // this will be the main thread for the users
    CData* userData = (CData*)data;
    int socketID = userData->socketID,status;
    ssize_t checkSocket; // will be used to ensure that the socket between the user and the client is still valid.
    
    // We're going to use an int to mark if the client has been dropped from the server or not
    int connectionStatus = CONNECTED;
    
    // Initialize the mutex used to wake up this client thread
    pthread_mutex_t self;
    pthread_cond_t wakeSelf;
    status = pthread_mutex_init(&self,NULL);
    checkMutexErr(status);
    status = pthread_cond_init(&wakeSelf,NULL);
    checkMutexErr(status);
    
    // The above will be ignored for now. Now we need to get the two child threads out the gate.
    // First we'll initialize the sending thread.
    obd *sendData = malloc(sizeof(obd));
    sendData->socketID = socketID;
    sendData->messages = NULL;
    
    status = pthread_mutex_init(&sendData->queueSend,NULL);
    checkMutexErr(status);
    status = pthread_cond_init(&sendData->fireOff,NULL);
    checkMutexErr(status);
    
    status = pthread_create(&sendData->tid,NULL,sendController,(void*)sendData);
    checkThreadError(status);
    
    // Second we'll make a receiving thread
    ibd *recvData = malloc(sizeof(ibd));
    recvData->socketID = socketID;
    recvData->queueMutex = &userData->addRmMsg;
    recvData->wakeClient = &wakeSelf;
    
    status = pthread_create(&recvData->tid,NULL,recvController,(void*)recvData);
    checkThreadError(status);
    
    // Clean up for the client
    status = pthread_cancel(sendData->tid);
    checkThreadError(status);
    status = pthread_cancel(recvData->tid);
    checkThreadError(status);
    status = pthread_join(sendData->tid,NULL);
    checkThreadError(status);
    status = pthread_join(recvData->tid,NULL);
    checkThreadError(status);
    
    // test to see if we haven't dropped the client
    checkSocket = send(socketID,"/test\n",6,0);
    
    
    return NULL;
}

int server_main(const char* hostname,int port, int preferred)
{
    // We're going to fork and the fork will hold the infinite loop.
    // This way we can send a commands into the server.
    
    int status;
    
    // The first thing we need to do is to get the IP v6 address from the hostname
    struct addrinfo* determine;
    struct addrinfo hints;
    setHintsForAddress(&hints);
    // The largest port usable is only 5 digits in length. This is enough space
    char portStr[7];
    sprintf(portStr,"%d",port);
    status = getaddrinfo(hostname,portStr,&hints,&determine);
    checkGetAddrErr(status,__LINE__);
    
    // Socket to be used for client connection
    int socketID = socket(preferred,SOCK_STREAM,0);
    checkSocketErr(socketID);
    
    // Now that we've made the code IPvX we can just select the propper protocol using a for loop
    while (determine != NULL) {
        if (determine->ai_family == preferred && determine->ai_protocol == IPPROTO_TCP) break;
        determine = determine->ai_next;
    }
    checkFinAddr((int*)determine);
    
    status = bind(socketID,determine->ai_addr,determine->ai_addrlen);
    checkBindError(status);
    
    status = listen(socketID,10);
    checkListenError(status);
    
    // We need to initialize the main chatroom
    struct chatRoom* mainRoom = malloc(sizeof(struct chatRoom));
    mainRoom->mainRoom = mainRoom;
    mainRoom->next = mainRoom;
    mainRoom->numAdmins = 0;
    mainRoom->numClients = 0;
    mainRoom->clients = NULL;
    mainRoom->admins = NULL;
    mainRoom->permanent = PERM;
    mainRoom->roomName = "Main";
    
    // We're going to share this edit chatrooms mutex. If we're going to manage multiple chat rooms it would only make sense
    pthread_rwlock_t accessChatRooms;
    status = pthread_rwlock_init(&accessChatRooms, NULL);
    checkMutexErr(status);
    mainRoom->accessRooms = &accessChatRooms;
    
    // This mutex will be used to edit the master list of clients connected to the server
    pthread_mutex_t editOnline;
    status = pthread_mutex_init(&editOnline,NULL);
    checkMutexErr(status);
    
    status = pthread_mutex_init(&mainRoom->perm,NULL);
    checkMutexErr(status);
    status = pthread_mutex_init(&mainRoom->editAdmin,NULL);
    checkMutexErr(status);
    status = pthread_mutex_init(&mainRoom->editClients,NULL);
    checkMutexErr(status);
    
    // First let's make the infinite loop
    while (1)
    {
        // Sockets will be closed by the client threads
        struct sockaddr_in client;
        socklen_t clientSize;
        int chatSocket = accept(socketID,(struct sockaddr*)&client,&clientSize);
        checkAcceptError(chatSocket);
        fprintf(stdout,"Client accepted on socket [%d]\n",chatSocket);

        // Now we're going to implement this in multiple threads
        CData* userThread = malloc(sizeof(CData));
        userThread->socketID = chatSocket;
        status = pthread_mutex_init(&userThread->addRmMsg,NULL);
        checkMutexErr(status);
        userThread->curRoom = mainRoom;
        userThread->msgs = NULL;
        userThread->usrName = NULL;
        pthread_attr_t clientAttributes;
        status = pthread_attr_init(&clientAttributes);
        checkThreadError(status);
        pthread_attr_setdetachstate(&clientAttributes, PTHREAD_CREATE_DETACHED);
        pthread_create(&userThread->tid, &clientAttributes, usrMngr, (void*)userThread);
        pthread_attr_destroy(&clientAttributes);
    }
    
    // for when the functionality to exit the server is implemented
    status = pthread_rwlock_destroy(&accessChatRooms);
    checkMutexErr(status);
    status = pthread_mutex_destroy(&mainRoom->editAdmin);
    checkMutexErr(status);
    status = pthread_mutex_destroy(&mainRoom->editClients);
    checkMutexErr(status);
    status = pthread_mutex_destroy(&mainRoom->perm);
    checkMutexErr(status);
    
    free(mainRoom);
    
    return 0;
}
