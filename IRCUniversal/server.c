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

// We only need a mutex on the pointer as the message will be saved in the struct
// This will have to allocate memory
struct message {
    int msgLen;
    char* msg;
    pthread_mutex_t next;
    struct message* nextMsg;
};

struct client {
    // Each client will run in it's own thread.
    pthread_t tid;
    char* usrName;
    
    // Which chatroom are we connected to?
    struct chatRoom* curRoom;
    
    // When a client sends a message it will have to add the message to the linked lists of messages
    // When the server wants to send the message to each of the clients it will need to remove messages from the pointer
    pthread_mutex_t addRmMsg;
    struct message* msgs;
};

struct chatRoom {
    int roomlen;
    // Boolean to state if the server should save the chatroom config
    pthread_mutex_t perm;
    int permanent;
    char *roomName;
    
    // The first room will ALWAYS be the main room.
    pthread_mutex_t addRmRooms;
    struct chatRoom* next;
    struct chatRoom* first;
    
    // Who is connected to the room
    pthread_mutex_t editUsr;
    struct client* users;
    
    // For future developement
    pthread_mutex_t editAdmin;
    struct client* admins;
};

struct roomManagerData
{
    // The managers will have their own id's.
    pthread_t tid;
    
    // As the roomManager must be loaded before the user manager we're going to set a conditional
    // variable in here that will be activated only once.
    pthread_mutex_t init;
    pthread_cond_t initCond;
    
    // What else should these managers keep track of?
    // The first chatRoom in this linked list will be the main room
    pthread_mutex_t editRoomList;
    pthread_rwlock_t editRoomRW;
    struct chatRoom* chatRooms;
    
    
} RMData;

struct userManager
{
    // This will have it's own id
    pthread_t tid;
    
    // Array of connected users
    pthread_mutex_t editClientList;
    pthread_rwlock_t editClientRW;
    struct client* clients;
    
    
};

void* chatRoomMngr(void* data)
{
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
    status = pthread_mutex_init(&chatRoomInitCond,NULL);
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
