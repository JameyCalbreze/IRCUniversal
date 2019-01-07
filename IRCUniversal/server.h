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

#define PERM 1
#define TEMP 0

// Connection drop codes
#define CONNECTED 0
#define ERROR_DROP (-1)
#define CLEAN_DROP 1

#endif /* server_h */

int server_main(const char* hostname,int port,int preferred);
void* usrMngr(void* data);

// We only need a mutex on the pointer as the message will be saved in the struct
// This will have to allocate memory
struct message {
    int msgLen;
    char* msg;
    struct message* nextMsg;
};

typedef struct client {
    // Each client will run in it's own thread.
    pthread_t tid;
    char* usrName;
    
    // We need the socket the client is communicating through
    int socketID;
    
    // Which chatroom are we connected to?
    struct chatRoom* curRoom;
    
    // When a client sends a message it will have to add the message to the linked lists of messages
    // When the server wants to send the message to each of the clients it will need to remove messages from the pointer
    pthread_mutex_t addRmMsg;
    struct message* msgs;
}CData;

struct chatRoom {
    // Boolean to state if the server should save the chatroom config
    pthread_mutex_t perm;
    int permanent;
    char *roomName;
    
    // The first room will ALWAYS be the main room.
    pthread_rwlock_t *accessRooms;
    struct chatRoom* next;
    struct chatRoom* mainRoom;
    
    // Who is connected to the room
    pthread_mutex_t editClients;
    struct client* clients;
    int numClients;
    
    // Who is connected to the server
    pthread_mutex_t *editOnline;
    struct client* online;
    int numOnline;
    
    // For future developement
    pthread_mutex_t editAdmin;
    struct client* admins;
    int numAdmins;
    
    // Perhaps I can have this abstracted to allow for user defined user types at some point.
};
