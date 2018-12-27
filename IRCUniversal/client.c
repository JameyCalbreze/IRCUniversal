//
//  client.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "client.h"
#include "errorCheck.h"

void setHintsForAddress6(struct addrinfo* hints)
{
    hints->ai_family = PF_INET6;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_protocol = IPPROTO_TCP;
    hints->ai_flags = 0;
    hints->ai_addrlen = 0;
    hints->ai_addr = NULL;
    hints->ai_canonname = NULL;
    hints->ai_next = NULL;
    return;
}

int client_main(const char* hostname, int port)
{
    // The first thing we need to do is to get the IP v6 address from the hostname
    struct addrinfo* determine;
    struct addrinfo hints;
    setHintsForAddress6(&hints);
    int status = getaddrinfo(hostname,NULL,&hints,&determine);
    checkGetAddrErr(status);
    
    // Create the socket
    int socketID = socket(PF_INET6,SOCK_STREAM,0);
    struct sockaddr_in6 srv;
    srv.sin6_family = AF_INET6;
    srv.sin6_port = htonl(port);
    // This should add the IPv6 information into this data structure.
    memcpy(&srv.sin6_addr,determine->ai_addr,determine->ai_addrlen);
    
    status = connect(socketID,(struct sockaddr*)&srv,sizeof(srv));
    checkConnectErr(status,__LINE__);
    
    //struct hostent *
    return 0;
}
