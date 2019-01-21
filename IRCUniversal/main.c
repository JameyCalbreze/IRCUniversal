//
//  main.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "main.h"

int main(int argc, const char * argv[]) {
    int mode = NO_MODE, port = -1, preferred = PF_INET6;
    const char* hostname;
    
    if (!argc%2) {
        fprintf(stderr,"Usage --mode [client/server] ");
        fprintf(stderr,"--host [host address] ");
        fprintf(stderr,"--port [port num]\n");
        return -1;
    }
    
    //===================================================//
    // Mode selection based on arguments
    // Can be provided in any order. This way retards don't
    // complain about their crashing executables
    int i;
    for(i = 1; i < argc; i++) {
        if ( strncmp(argv[i],"--mode\0",7) == 0){
            assert(argv[++i] != NULL);
            if( strncmp("client\0",argv[i],7) == 0) {
                mode = CLIENT;
            } else if( strncmp("server\0",argv[i],7) == 0) {
                mode = SERVER;
            }
        } else if ( strncmp("--host\0",argv[i],7) == 0) {
            assert(argv[++i] != NULL);
            hostname = argv[i];
        } else if ( strncmp("--port\0",argv[i],7) == 0) {
            assert(argv[++i] != NULL);
            port = atoi(argv[i]);
        }
    }
    
    if (mode == CLIENT) {
        return client_main(hostname,port,preferred);
    }
    else if (mode == SERVER) {
        return server_main(hostname,port,preferred);
    }
    else {
        fprintf(stderr,"A valid mode was not supplied\n");
        return NO_MODE;
    }
}
