//
//  sharedTypes.h
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/20/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//

#ifndef sharedTypes_h
#define sharedTypes_h

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Connection drop codes
#define CONNECTED 0
#define ERROR_DROP (-1)
#define CLEAN_DROP 1

// linked list structure
typedef struct linkedList {
    void *structRef;
    void *nextStruct;
} LL;

// What should the server have access to at the highest level. The client will have a pointer to this instance of the structure
typedef struct serverMasterReference {
    // Going to need a master structure of the send requests
    pthread_mutex_t addRmSendRequests;
    LL *allSendRequests;
    unsigned long long numRequests;
    
    // The master list of all chatrooms will now be stored in this structure
    pthread_mutex_t addRmChatRooms;
    LL *allChatRooms;
    unsigned long long numRooms;
    
    // There will also be a master list of all connected clients stored here as well
    pthread_mutex_t addRmClients;
    LL *allClients;
    unsigned long long numClients;
} SrvMast;

// We only need a mutex on the pointer as the message will be saved in the struct
// This will have to allocate memory
typedef struct message {
    int msgLen;
    char* msg;
} Message;

typedef struct chatRoom {
    // Boolean to state if the server should save the chatroom config
    pthread_mutex_t perm;
    int permanent;
    char *roomName;
    
    // Who is connected to the room
    pthread_mutex_t editClients;
    LL *clients;
    unsigned long long numClients;
    
    // For future developement
    pthread_mutex_t editAdmin;
    LL *admins;
    unsigned long long numAdmins;
    
    // Perhaps I can have this abstracted to allow for user defined user types at some point.
} ChatRoom;

typedef struct client {
    // reference to the serverMaster structure
    SrvMast *serverMaster;
    
    // Each client will run in it's own thread.
    pthread_t tid;
    char* usrName;
    
    // We need the socket the client is communicating through
    pthread_mutex_t preventCrossMessage;
    int socketID;
    
    // Which chatroom are we connected to?
    struct chatRoom* curRoom;
    
    // When a client sends a message it will have to add the message to the linked lists of messages
    // When the server wants to send the message to each of the clients it will need to remove messages from the pointer
    pthread_mutex_t addRmMsg;
    LL *msgs;
    
    pthread_mutex_t addRmCmd;
    LL *cmds;
} CData;

// As part of the send-req structure
// Simple linked list structure. I seem to really like making this for some reason.
typedef struct sendRequest {
    ChatRoom *roomForMessage;
    Message *messageToSend;
} SendRequest;

typedef struct sendControllerData {
    pthread_mutex_t addRmSendRequests;
    LL *sendRequests;
    unsigned long long numRequests;
} SendControllerData;

typedef struct RecvControllerData {
    pthread_t tid;
    pthread_mutex_t *queueMutex;
    int socketID;
    LL **cmdMsgQueue;
    // How we wake up our main client thread
    pthread_cond_t *wakeClient;
    pthread_mutex_t *editStatus;
    void **connectionStatus;
} RecvControllerData;

// Functions
void addNodeToLL(void *data,LL **nodeList);
void rmNodeFromLL(void *data, LL **nodeList);

#endif /* sharedTypes_h */
