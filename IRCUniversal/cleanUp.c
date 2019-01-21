//
//  cleanUp.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/20/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//

#include "cleanUp.h"

void clearMsgChain(LL *curMsg)
{
    if (curMsg == NULL) return;
    while (curMsg != NULL) {
        LL *nextNode = curMsg->nextStruct;
        Message *messageToFree = (Message*)curMsg->structRef;
        free(messageToFree->msg);
        free(messageToFree);
        curMsg = nextNode;
    }
    return;
}

void cleanRecvData(Ibd* recvData)
{
    free(recvData);
    return;
}
