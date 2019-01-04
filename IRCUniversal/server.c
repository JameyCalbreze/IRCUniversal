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

void* chatRoomMngr(void* data)
{
    // Cast our data pointer out so that we have access to the mutex we passed
    RMData* waitMutex = (RMData*)data;
    int status;
    
    // Now our initialization step should be complete be this point so we'll anounce that the main thread may continue;
    pthread_cond_broadcast(waitMutex->initCond);
    
    return NULL;
}

void* usrMngr(void* data)
{
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
    
    // The new section that will accept the user connections. This will initialize the two threads.
    // One for the chatroom manager and another for the user manager
    pthread_mutex_t chatRoomInit;
    pthread_cond_t chatRoomInitCond;
    pthread_rwlock_t accessChatRooms;
    status = pthread_mutex_init(&chatRoomInit,NULL);
    checkMutexErr(status);
    status = pthread_cond_init(&chatRoomInitCond,NULL);
    checkMutexErr(status);
    status = pthread_rwlock_init(&accessChatRooms, NULL);
    checkMutexErr(status);
    
    // Initialize the room manager data structure
    RMData* rmThreadData = malloc(sizeof(RMData));
    checkMemError((void*)rmThreadData);
    rmThreadData->init = &chatRoomInit;
    rmThreadData->initCond = &chatRoomInitCond;
    rmThreadData->accessRoomList = &accessChatRooms;
    status = pthread_create(&rmThreadData->tid,NULL,chatRoomMngr,(void*)rmThreadData);
    checkThreadError(status);
    
    // Now we have to wait for the chatroom manager to finish initializing we'll unlock the mutex immediately
    // The mutex is only required in order to prevent the user manager from adding users to a chatroom that doesn't exist;
    pthread_cond_wait(&chatRoomInitCond, &chatRoomInit);
    pthread_mutex_unlock(&chatRoomInit);
    
    // These mutexes will be destroyed once the chatRoomManager has finished executing as there is no longer any need for them.
    status = pthread_cond_destroy(&chatRoomInitCond);
    checkMutexErr(status);
    status = pthread_mutex_destroy(&chatRoomInit);
    checkMutexErr(status);
    status = pthread_rwlock_destroy(&accessChatRooms);
    checkMutexErr(status);
    
    // First let's make the infinite loop
//    while (1)
//    {
//        struct sockaddr_in client;
//        socklen_t clientSize;
//        int chatSocket = accept(socketID,(struct sockaddr*)&client,&clientSize);
//        checkAcceptError(chatSocket);
//        fprintf(stdout,"Client accepted on socket [%d]\n",chatSocket);
//
//        // Once we get to this point we need to start multi threading the program
//        // We'll hold off on the threads for now. We need to see if we can get a connection to work
//        // between the server and the client.
//        pid_t child = fork();
//        if (child == 0) {
//            close(socketID);
//            close(chatSocket);
//            return -1;
//        } else {
//            fprintf(stdout,"Child :%d created\n",child);
//            close(chatSocket);
//            status = 0;
//            pid_t deadChild;
//            do {
//                deadChild = waitpid(0,&status,WNOHANG);checkWaitError(deadChild);
//                if (deadChild > 0) {
//                    fprintf(stdout,"Reaped %d\n",deadChild);
//                }
//            } while(deadChild > 0);
//        }
//    }
    return 0;
}
