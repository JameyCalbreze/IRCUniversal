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
    // We'll create the read write lock for our room list
    int status;
    pthread_rwlock_t rwEditRooms;
    status = pthread_rwlock_init(&rwEditRooms,NULL);
    checkMutexErr(status);
    
    // This will be the clean up section
    status = pthread_rwlock_destroy(&rwEditRooms);
    checkMutexErr(status);
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
    status = pthread_mutex_init(&chatRoomInit,NULL);
    checkMutexErr(status);
    pthread_cond_t chatRoomInitCond;
    status = pthread_cond_init(&chatRoomInitCond,NULL);
    checkMutexErr(status);
    
    // Initialize the room manager data structure
    RMData* rmThreadData = malloc(sizeof(RMData));
    checkMemError((void*)rmThreadData);
    rmThreadData->init = &chatRoomInit;
    rmThreadData->initCond = &chatRoomInitCond;
    status = pthread_create(&rmThreadData->tid,NULL,chatRoomMngr,(void*)rmThreadData);
    checkThreadError(status);
    
    // This will be the end of the function where we have to clean up everything that we've done
    status = pthread_mutex_destroy(&chatRoomInit);
    checkMutexErr(status);
    status = pthread_cond_destroy(&chatRoomInitCond);
    checkMutexErr(status);
    
    // First let's make the infinite loop
    while (1)
    {
        struct sockaddr_in client;
        socklen_t clientSize;
        int chatSocket = accept(socketID,(struct sockaddr*)&client,&clientSize);
        checkAcceptError(chatSocket);
        fprintf(stdout,"Client accepted on socket [%d]\n",chatSocket);
        
        // Once we get to this point we need to start multi threading the program
        // We'll hold off on the threads for now. We need to see if we can get a connection to work
        // between the server and the client.
        pid_t child = fork();
        if (child == 0) {
            close(socketID);
            close(chatSocket);
            return -1;
        } else {
            fprintf(stdout,"Child :%d created\n",child);
            close(chatSocket);
            status = 0;
            pid_t deadChild;
            do {
                deadChild = waitpid(0,&status,WNOHANG);checkWaitError(deadChild);
                if (deadChild > 0) {
                    fprintf(stdout,"Reaped %d\n",deadChild);
                }
            } while(deadChild > 0);
        }
    }
    return 0;
}
