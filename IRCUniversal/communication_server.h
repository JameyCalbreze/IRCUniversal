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
#include "sharedTypes.h"
#include "server.h"
#include "errorCheck.h"
#include "cleanUp.h"

#endif /* communication_h */

void* sendController(void* data);
void* recvController(void* data);
