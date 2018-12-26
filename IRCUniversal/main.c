//
//  main.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "main.h"

int main(int argc, const char * argv[]) {
    // Mode 0 means that the program should run as a client.
    int mode = 0; int port = -1;
    const char* hostname;
    
    //===================================================//
    // Mode selection based on arguments
    // Can be provided in any order. This way retards don't
    // complain about their crashing executables
    int i;
    for(i = 1; i < argc; i++) {
        if (strncmp(argv[i],"--mode",6)){
            assert(argv[++i] != NULL);
            if( strncmp("client",argv[i],6) ) {
                mode = 0;
            } else if( strncmp("server",argv[i],6) ) {
                mode = 1;
            }
        } else if ( strncmp("--host",argv[i],6) ) {
            assert(argv[++i] != NULL);
            hostname = argv[i];
        } else if ( strncmp("--port",argv[i],6) ) {
            assert(argv[++i] != NULL);
            port = atoi(argv[i]);
        }
    }
}
