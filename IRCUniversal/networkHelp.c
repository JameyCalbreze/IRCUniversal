//
//  networkHelp.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/27/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "networkHelp.h"

void setHintsForAddress(struct addrinfo* hints)
{
    hints->ai_family = PF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_protocol = IPPROTO_TCP;
    hints->ai_flags = 0;
    hints->ai_addrlen = 0;
    hints->ai_addr = NULL;
    hints->ai_canonname = NULL;
    hints->ai_next = NULL;
}
