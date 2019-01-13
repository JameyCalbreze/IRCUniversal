//
//  error.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/26/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "errorCheck.h"

void checkGetAddrErr(int status, int line)
{
    if (status != 0)
    {
        fprintf(stderr,"GetAddrInfo failed with error code [%s]\n",strerror(errno));
        fprintf(stderr,"Line #: %d\n",line);
        exit(-1);
    }
    return;
}

void checkBindError(int status)
{
    if (status != 0)
    {
        fprintf(stderr,"Bind failed with error code [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkListenError(int status)
{
    if (status != 0)
    {
        fprintf(stderr,"Listen failed with error code [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkAcceptError(int status)
{
    if (status <= 0)
    {
        fprintf(stderr,"Accept failed with error code [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkWaitError(int pid) {
    if (pid < 0)
    {
        fprintf(stderr,"WaitPid failed with error [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkConnectErr(int status, int line)
{
    if(status != 0)
    {
        fprintf(stderr,"Connect failed with error [%s]\nOn line %d\n",strerror(errno),line);
        exit(-1);
    }
    return;
}

void checkFinAddr(int* determine)
{
    // Check final address config
    if(determine == NULL){
        fprintf(stderr,"No compatible address information determined\n");
        fprintf(stderr,"error code [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkSocketErr(int socket)
{
    if(socket <= 0) {
        fprintf(stderr,"Socket creation failed with error code [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkMutexErr(int status)
{
    if(status != 0)
    {
        fprintf(stderr,"Mutex creation failed with error [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkMemError(void* pointer)
{
    if(pointer == NULL)
    {
        fprintf(stderr,"Memory allocation failed [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

void checkThreadError(int status)
{
    if(status != 0)
    {
        fprintf(stderr,"Thread creation failed [%s]\n",strerror(errno));
        exit(-1);
    }
    return;
}

int checkSendError(ssize_t sent,int socketID)
{
    if(sent < 0)
    {
        fprintf(stderr,"Send over socket failed [%s]\n",strerror(errno));
        fprintf(stderr,"Connection over socket: %d will be dropped\n",socketID);
        return -1;
    }
    return 0;
}

void checkCommandExecution(int status,char* messageSupplied)
{
    if(status != 0)
    {
        fprintf(stderr,"Message [%s] does not contain a valid command\n",messageSupplied);
    }
    return;
}
