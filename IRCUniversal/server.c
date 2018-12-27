//
//  server.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 12/25/18.
//  Copyright © 2018 Jamey Calabrese. All rights reserved.
//

#include "server.h"
#include "errorCheck.h"

struct clientMessages
{
    char* user;
    char* message;
} Cmsg;

struct clientThreads
{
    pthread_t tid;
    
} Cthr;

int server_main(const char* hostname,int port)
{
    // We're going to fork and the fork will hold the infinite loop.
    // This way we can send a commands into the server.
    
    int status;
    
    
    // Socket to be used for client connection
    int socketID = socket(PF_INET6,SOCK_STREAM,0);
    
    struct sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htonl(port);
    addr.sin6_addr = in6addr_any;
    
    status = bind(socketID,(struct sockaddr*)&addr,sizeof(addr));
    checkBindError(status);
    
    status = listen(socketID,10);
    checkListenError(status);
    
    // struct in6_addr host;
    
    // First let's make the infinite loop
    while (1)
    {
        struct sockaddr_in client;
        socklen_t clientSize;
        int chatSocket = accept(socketID,(struct sockaddr*)&client,&clientSize);
        checkAcceptError(chatSocket);
        fprintf(stdout,"Client accepted on scoket [%d]\n",chatSocket);
        
        // Once we get to this point we need to start multi threading the program
        // We'll hold off on the threads for now. We need to see if we can get a connection to work
        // between the server and the client.
        pid_t child = fork();
        if (child == 0) {
            close(socketID);
            close(chatSocket);
            return -1;
        } else {
            fprintf(stdout,"Child :%d created\n",child);
            close(chatSocket);
            status = 0;
            pid_t deadChild;
            do {
                deadChild = waitpid(0,&status,WNOHANG);checkWaitError(deadChild);
                if (deadChild > 0) {
                    fprintf(stdout,"Reaped %d\n",deadChild);
                }
            } while(deadChild > 0);
        }
    }
    return 0;
}
