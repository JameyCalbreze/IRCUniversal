//
//  server.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "server.h"

void addClientToChatRoom(ChatRoom *roomToAddTo, CData *client)
{
    addNodeToLL((void*)client, &roomToAddTo->clients);
    return;
}

void rmClientFromChatRoom(ChatRoom *roomToRemoveFrom, CData *client)
{
    rmNodeFromLL((void*)client,&roomToRemoveFrom->clients);
    return;
}

// Now we need to execute the command
// Each command will be programmed here. It would be nice if I could load a configuration file that would allow me to save these
// and edit them from the terminal once the server/client connection has been established.
int executeCommand(LL *commandNode)
{
    Message *command = (Message*)commandNode->structRef;
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
int checkForCommand(void *pointerToMessage)
{
    Message *first = (Message*)pointerToMessage;
    return first->msg[0] == '/';
}

// These threads are initialized as detached. There needs to be a way to join all of these threads
void* usrMngr(void* data)
{
    // this will be the main thread for the users
    CData* userData = (CData*)data;
    int socketID = userData->socketID; int status;
    SrvMast *serverMaster = userData->serverMaster;
    
    // The first thing the client must do is add themselves to the main online list
    pthread_mutex_lock(&serverMaster->addRmClients);
    addNodeToLL((void*)userData,&serverMaster->allClients);
    serverMaster->numClients++;
    pthread_mutex_unlock(&serverMaster->addRmClients);
    
    // Second we'll add ourselves to the currentRoom
    ChatRoom *curRoom = userData->curRoom;
    pthread_mutex_lock(&curRoom->editClients);
    addClientToChatRoom(curRoom,userData);
    curRoom->numClients++;
    pthread_mutex_unlock(&curRoom->editClients);
    
    // We're going to use an int to mark if the client has been dropped from the server or not
    void *connectionStatus = CONNECTED;
    
    // We need a place to store messages received be the recvData thread
    LL *msgQueue = NULL;
    
    // Initialize the mutex used to wake up this client thread
    pthread_mutex_t self;
    pthread_cond_t wakeSelf;
    status = pthread_mutex_init(&self,NULL); checkMutexErr(status);
    status = pthread_cond_init(&wakeSelf,NULL); checkMutexErr(status);
    
    // Second we'll make a receiving thread
    RecvControllerData *recvData = malloc(sizeof(RecvControllerData));
    recvData->socketID = socketID;
    recvData->queueMutex = &userData->addRmMsg;
    recvData->wakeClient = &wakeSelf;
    recvData->editStatus = &self;
    recvData->connectionStatus = &connectionStatus;
    recvData->cmdMsgQueue = &msgQueue;
    
    // Create thread
    status = pthread_create(&recvData->tid,NULL,recvController,(void*)recvData); checkThreadError(status);
    
    // This will be the command interpreter for the server
    // The send and receive threads will be able to change this status. Once that happens they will check back to the
    // shared variable. If the status is not CONNECTED then the child threads will exit on their own
    do {
        // Begin the sequence
        // Sort or distribute messages accross the current chatroom
        
        // Check for new messages
        pthread_mutex_lock(&userData->addRmMsg);
        if(userData->msgs != NULL) {
            // TODO
        }
        pthread_mutex_unlock(&userData->addRmMsg);
        
        // Second We'll check commands
        LL* commandHold = NULL;
        pthread_mutex_lock(&userData->addRmCmd);
        if(userData->cmds != NULL) {
            commandHold = userData->cmds;
            userData->cmds = NULL;
        }
        pthread_mutex_unlock(&userData->addRmCmd);
        // If we've received a command we will now interpret it
        while(commandHold != NULL){
            int isCommand = checkForCommand(commandHold);
            if(isCommand) {
                // Place holder
                status = executeCommand(commandHold); checkCommandExecution(status,(Message*)commandHold->structRef);
            }
            LL *tempNext = commandHold->nextStruct;
            commandHold->nextStruct = NULL;
            clearMsgChain(commandHold);
            commandHold = tempNext;
        }
        
        
        // This means that the receive thread will be the thread breaking out of the blocking behavior of the recv command
        pthread_cond_wait(&wakeSelf,&self);
    } while (connectionStatus == CONNECTED);
    
    // first we have to leave the chatroom once we are disconnecting
    pthread_mutex_lock(&curRoom->editClients);
    rmClientFromChatRoom(curRoom, userData);
    curRoom->numClients--;
    pthread_mutex_unlock(&curRoom->editClients);
    
    // Second we'll remove outselves from the online list
    pthread_mutex_lock(&serverMaster->addRmClients);
    rmNodeFromLL((void*)userData, &serverMaster->allClients);
    serverMaster->numClients--;
    pthread_mutex_unlock(&serverMaster->addRmClients);
    
    // If we close the client first we won't have to worry about how the threads die as they should both be unblocked at a minimum
    close(socketID);
    
    // Clean up for the client
    status = pthread_join(recvData->tid,NULL); checkThreadError(status);
    cleanRecvData(recvData);
    pthread_mutex_destroy(&userData->addRmCmd);
    pthread_mutex_destroy(&userData->addRmMsg);
    // Function is located in the cleanUp file
    clearMsgChain(userData->cmds);
    clearMsgChain(userData->msgs);
    free(userData);
    
    return connectionStatus;
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
    
    SrvMast *serverMaster = malloc(sizeof(SrvMast));
    serverMaster->allChatRooms = malloc(sizeof(LL)); checkMemError(serverMaster->allChatRooms);
    serverMaster->allChatRooms->nextStruct = NULL;
    serverMaster->allClients = NULL;
    serverMaster->allSendRequests = NULL;
    status = pthread_mutex_init(&serverMaster->addRmChatRooms,NULL); checkMutexErr(status);
    status = pthread_mutex_init(&serverMaster->addRmClients,NULL); checkMutexErr(status);
    status = pthread_mutex_init(&serverMaster->addRmSendRequests,NULL); checkMutexErr(status);
    
    // We need to initialize the main chatroom
    ChatRoom* mainRoom = malloc(sizeof(ChatRoom));
    mainRoom->admins = NULL;
    mainRoom->clients = NULL;
    mainRoom->numAdmins = 0;
    mainRoom->numClients = 0;
    mainRoom->permanent = PERM;
    status = pthread_mutex_init(&mainRoom->editAdmin,NULL); checkMutexErr(status);
    status = pthread_mutex_init(&mainRoom->editClients,NULL); checkMutexErr(status);
    status = pthread_mutex_init(&mainRoom->perm,NULL); checkMutexErr(status);
    
    serverMaster->allChatRooms->structRef = (void*)mainRoom;
    
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
        status = pthread_mutex_init(&userThread->preventCrossMessage,NULL); checkMutexErr(status);
        userThread->curRoom = mainRoom;
        userThread->msgs = NULL;
        userThread->usrName = NULL;
        pthread_attr_t clientAttributes;
        status = pthread_attr_init(&clientAttributes); checkThreadError(status);
        pthread_attr_setdetachstate(&clientAttributes, PTHREAD_CREATE_DETACHED);
        pthread_create(&userThread->tid, &clientAttributes, usrMngr, (void*)userThread);
        pthread_attr_destroy(&clientAttributes);
    }
    
    // At some point we'll need to free up all of the allocated memory for the server structure.
    
    return 0;
}
