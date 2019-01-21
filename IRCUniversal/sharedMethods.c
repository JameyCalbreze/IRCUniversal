//
//  sharedTypes.c
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/20/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//

#include "sharedMethods.h"

void addNodeToLL(void *data,LL **nodeList)
{
    LL *newNode = malloc(sizeof(LL)); checkMemError(newNode);
    newNode->nextStruct = NULL;
    newNode->structRef = data;
    LL *firstNode = *nodeList;
    if (firstNode == NULL) {
        *nodeList = newNode;
        return;
    }
    while(firstNode->nextStruct != NULL) firstNode = firstNode->nextStruct;
    firstNode->nextStruct = newNode;
    return;
}

void rmNodeFromLL(void *data, LL **nodeList)
{
    LL *firstNode = *nodeList;
    if (firstNode->structRef == data) {
        *nodeList = firstNode->nextStruct;
        free(firstNode);
        return;
    }
    LL *prevNode = NULL;
    while(firstNode->structRef != data) {
        prevNode = firstNode;
        firstNode = firstNode->nextStruct;
    }
    prevNode->nextStruct = firstNode->nextStruct;
    free(firstNode);
    return;
}
