//
//  communication.h
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/5/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//

#ifndef communication_h
#define communication_h

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>


#endif /* communication_h */

void* sendController(void* data);
void* recvController(void* data);

// These threads will be autonomous workers
typedef struct outboundData {
    pthread_t tid;
    pthread_mutex_t queueSend;
    struct message* messages;
    pthread_cond_t fireOff;
    int socketID;
} obd;

typedef struct inboundData {
    pthread_t tid;
    // This is where we put the message
    pthread_mutex_t *queueMutex;
    int socketID;
    // How we wake up our main client thread
    pthread_cond_t *wakeClient;
} ibd;
