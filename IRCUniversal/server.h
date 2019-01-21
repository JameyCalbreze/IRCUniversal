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
#include "errorCheck.h"
#include "networkHelp.h"
#include "communication_server.h"
#include "cleanUp.h"
#include "sharedTypes.h"

#define PERM 1
#define TEMP 0

 /* server_h */

// Functions
int server_main(const char* hostname,int port,int preferred);
void* usrMngr(void* data);
int checkForCommand(void *pointerToMessage);

#endif
