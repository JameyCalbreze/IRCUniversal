//
//  server.h
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#ifndef server_h
#define server_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>

#endif /* server_h */

int server_main(const char* hostname,int port,int preferred);
void* usrMngr(void* data);
void* chatRoomMngr(void* data);

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

typedef struct roomManagerData {
    // The managers will have their own id's.
    pthread_t tid;
    
    // As the roomManager must be loaded before the user manager we're going to set a conditional
    // variable in here that will be activated only once.
    pthread_mutex_t *init;
    pthread_cond_t *initCond;
    
    // The reason these will be initialized outside the thread is so that the user manager may have access to these
    pthread_rwlock_t *accessRoomList;
    struct chatRoom* chatRooms;
} RMData;

struct userManager {
    // This will have it's own id
    pthread_t tid;
    
    // Array of connected users
    pthread_mutex_t editClientList;
    pthread_rwlock_t editClientRW;
    struct client* clients;
    
    
};
