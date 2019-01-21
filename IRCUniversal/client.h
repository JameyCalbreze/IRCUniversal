//
//  client.h
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#ifndef client_h
#define client_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "sharedTypes.h"
#include "errorCheck.h"
#include "networkHelp.h"
#include "communication_client.h"

#endif /* client_h */

int client_main(const char* hostname,int port,int preferred);
