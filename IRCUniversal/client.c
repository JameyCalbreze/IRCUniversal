//
//  client.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "client.h"
#include "errorCheck.h"
#include "networkHelp.h"

int client_main(const char* hostname, int port, int preferred)
{
    // The first thing we need to do is to get the IP v6 address from the hostname
    struct addrinfo* determine;
    struct addrinfo hints;
    setHintsForAddress(&hints);
    char portStr[7];
    sprintf(portStr,"%d",port);
    int status = getaddrinfo(hostname,portStr,&hints,&determine);
    checkGetAddrErr(status,__LINE__);
    
    // Create the socket
    int socketID = socket(preferred,SOCK_STREAM,0);
    
    // Now that we've made the code IPvX we can just select the propper protocol using a for loop
    while (determine != NULL) {
        if (determine->ai_family == preferred && determine->ai_protocol == IPPROTO_TCP) break;
        determine = determine->ai_next;
    }
    checkFinAddr((int*)determine);
    
    status = connect(socketID,determine->ai_addr,determine->ai_addrlen);
    checkConnectErr(status,__LINE__);
    
    //struct hostent *
    return 0;
}
