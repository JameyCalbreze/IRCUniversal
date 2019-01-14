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
#include "communication_server.h"

// Now we need to execute the command
// Each command will be programmed here. It would be nice if I could load a configuration file that would allow me to save these
// and edit them from the terminal once the server/client connection has been established.
int executeCommand(struct message *command)
{
    // here we will check for commands through a set of if statements
    int validCommand = -1;
    // Should be expanded to notify which user attempted to use the command
    if(strncmp(command->msg,"/test\0",6) == 1) {
        fprintf(stdout,"Test command detected and confirmed\n");
        validCommand = 0;
    }
    
    return validCommand;
}

// Some method to check for commands. For now it will only be used for the / character
int checkForCommand(struct message *first)
{
    return first->msg[0] == '/';
}

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
    status = pthread_mutex_init(&self,NULL); checkMutexErr(status);
    status = pthread_cond_init(&wakeSelf,NULL); checkMutexErr(status);
    
    // The above will be ignored for now. Now we need to get the two child threads out the gate.
    // First we'll initialize the sending thread.
    obd *sendData = malloc(sizeof(obd));
    sendData->socketID = socketID;
    sendData->messages = NULL;
    sendData->editStatus = &self;
    sendData->connectionStatus = &connectionStatus;
    status = pthread_mutex_init(&sendData->queueSend,NULL); checkMutexErr(status);
    status = pthread_cond_init(&sendData->fireOff,NULL); checkMutexErr(status);
    
    // Second we'll make a receiving thread
    ibd *recvData = malloc(sizeof(ibd));
    recvData->socketID = socketID;
    recvData->queueMutex = &userData->addRmMsg;
    recvData->wakeClient = &wakeSelf;
    recvData->working = NULL;
    recvData->editStatus = &self;
    recvData->connectionStatus = &connectionStatus;
    
    // Create threads
    status = pthread_create(&sendData->tid,NULL,sendController,(void*)sendData); checkThreadError(status);
    status = pthread_create(&recvData->tid,NULL,recvController,(void*)recvData); checkThreadError(status);
    
    // This will be the command interpreter for the server
    // The send and receive threads will be able to change this status. Once that happens they will check back to the
    // shared variable. If the status is not CONNECTED then the child threads will exit on their own
    do {
        // Begin the sequence
        // First check for new messages
        pthread_mutex_lock(&userData->addRmMsg);
        if(userData->msgs != NULL) {
            pthread_mutex_lock(&sendData->queueSend);
            sendData->messages = userData->msgs;
            pthread_mutex_unlock(&sendData->queueSend);
            userData->msgs = NULL;
            pthread_cond_signal(&sendData->fireOff);
        }
        pthread_mutex_unlock(&userData->addRmMsg);
        
        // Second We'll check commands
        struct message* tempHold = NULL;
        pthread_mutex_lock(&userData->addRmCmd);
        if(userData->cmds != NULL) {
            tempHold = userData->cmds;
            userData->cmds = NULL;
        }
        pthread_mutex_unlock(&userData->addRmCmd);
        // If we've received a command we will now interpret it
        if(tempHold != NULL){
            int isCommand = checkForCommand(tempHold);
            if(isCommand) {
                // Place holder
                status = executeCommand(tempHold);
                checkCommandExecution(status,tempHold->msg);
            }
        }
        
        // This means that the receive thread will be the thread breaking out of the blocking behavior of the recv command
        pthread_cond_wait(&wakeSelf,&self);
    } while (connectionStatus == CONNECTED);
    
    // If we close the client first we won't have to worry about how the threads die as they should both be unblocked at a minimum
    close(socketID);
    
    // Clean up for the client
    pthread_cond_signal(&sendData->fireOff);
    status = pthread_join(sendData->tid,NULL); checkThreadError(status);
    status = pthread_join(recvData->tid,NULL); checkThreadError(status);
    cleanSendData(sendData);
    cleanRecvData(recvData);
    
    return NULL;
}

int server_main(const char* hostname,int port, int preferred)
{
    int status;
    
    // The first thing we need to do is to get the IP v6 address from the hostname
    struct addrinfo* determine;
    struct addrinfo hints;
    setHintsForAddress(&hints);
    // The largest port usable is only 5 digits in length. This is enough space
    char portStr[7];
    sprintf(portStr,"%d",port);
    status = getaddrinfo(hostname,portStr,&hints,&determine); checkGetAddrErr(status,__LINE__);
    
    // Socket to be used for client connection
    int socketID = socket(preferred,SOCK_STREAM,0); checkSocketErr(socketID);
    
    // Now that we've made the code IPvX we can just select the propper protocol using a for loop
    while (determine != NULL) {
        if (determine->ai_family == preferred && determine->ai_protocol == IPPROTO_TCP) break;
        determine = determine->ai_next;
    }
    checkFinAddr((int*)determine);
    
    status = bind(socketID,determine->ai_addr,determine->ai_addrlen); checkBindError(status);
    
    status = listen(socketID,10); checkListenError(status);
    
    // We need to initialize the main chatroom
    struct chatRoom* mainRoom = malloc(sizeof(struct chatRoom));
    mainRoom->mainRoom = mainRoom;
    mainRoom->next = NULL;
    mainRoom->numAdmins = 0;
    mainRoom->numClients = 0;
    mainRoom->clients = NULL;
    mainRoom->admins = NULL;
    mainRoom->permanent = PERM;
    mainRoom->roomName = "Main";
    
    // We're going to share this edit chatrooms mutex. If we're going to manage multiple chat rooms it would only make sense
    pthread_rwlock_t accessChatRooms;
    status = pthread_rwlock_init(&accessChatRooms, NULL); checkMutexErr(status);
    mainRoom->accessRooms = &accessChatRooms;
    
    // This mutex will be used to edit the master list of clients connected to the server
    pthread_mutex_t editOnline;
    status = pthread_mutex_init(&editOnline,NULL); checkMutexErr(status);
    
    status = pthread_mutex_init(&mainRoom->perm,NULL); checkMutexErr(status);
    status = pthread_mutex_init(&mainRoom->editAdmin,NULL); checkMutexErr(status);
    status = pthread_mutex_init(&mainRoom->editClients,NULL); checkMutexErr(status);
    
    // First let's make the infinite loop
    // in the future this 1 will be replaced with a boolean. A terminal side command will be able to take this down.
    while (1)
    {
        // Sockets will be closed by the client threads
        struct sockaddr_in client;
        socklen_t clientSize;
        int chatSocket = accept(socketID,(struct sockaddr*)&client,&clientSize); checkAcceptError(chatSocket);
        fprintf(stdout,"Client accepted on socket [%d]\n",chatSocket);

        // Now we're going to implement this in multiple threads
        CData* userThread = malloc(sizeof(CData));
        userThread->socketID = chatSocket;
        status = pthread_mutex_init(&userThread->addRmMsg,NULL); checkMutexErr(status);
        userThread->curRoom = mainRoom;
        userThread->msgs = NULL;
        userThread->usrName = NULL;
        pthread_attr_t clientAttributes;
        status = pthread_attr_init(&clientAttributes); checkThreadError(status);
        pthread_attr_setdetachstate(&clientAttributes, PTHREAD_CREATE_DETACHED);
        pthread_create(&userThread->tid, &clientAttributes, usrMngr, (void*)userThread);
        pthread_attr_destroy(&clientAttributes);
    }
    
    // for when the functionality to exit the server is implemented
    status = pthread_rwlock_destroy(&accessChatRooms); checkMutexErr(status);
    status = pthread_mutex_destroy(&mainRoom->editAdmin); checkMutexErr(status);
    status = pthread_mutex_destroy(&mainRoom->editClients); checkMutexErr(status);
    status = pthread_mutex_destroy(&mainRoom->perm); checkMutexErr(status);
    
    free(mainRoom);
    
    return 0;
}
