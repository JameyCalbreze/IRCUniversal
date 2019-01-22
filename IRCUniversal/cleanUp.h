//
//  cleanUp.h
//  IRCUniversal
//
//  Created by Jamey Calabrese on 1/20/19.
//  Copyright © 2019 Jamey Calabrese. All rights reserved.
//

#ifndef cleanUp_h
#define cleanUp_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sharedTypes.h"

void clearMsgChain(LL *curMsg);
void cleanRecvData(RecvControllerData* recvData);

#endif /* cleanUp_h */
