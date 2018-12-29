//
//  error.h
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/26/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#ifndef error_h
#define error_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>

#endif /* error_h */

void checkGetAddrErr(int status, int line);
void checkBindError(int status);
void checkListenError(int status);
void checkAcceptError(int status);
void checkWaitError(int pid);
void checkConnectErr(int status, int line);
void checkFinAddr(int* determine);
void checkSocketErr(int socket);
