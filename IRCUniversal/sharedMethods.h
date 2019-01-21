//
//  sharedMethods.h
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/20/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//

#ifndef sharedMethods_h
#define sharedMethods_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sharedTypes.h"
#include "errorCheck.h"

void addNodeToLL(void *data,LL **nodeList);
void rmNodeFromLL(void *data, LL **nodeList);

#endif /* sharedMethods_h */
