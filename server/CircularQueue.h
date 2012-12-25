/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/CircularQueue.h                     */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Tue 25 Dec 2012 03:28:46 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include "../common/dbProtocol.h"
#include "serversocket.h"

typedef struct
{
    ClientSockHandle hcsock;
    char *buf;
} QueueNode;

void InitQueue();

void EnQueue(QueueNode *pnode);

QueueNode *DeQueue();

/* return : 0-not full, 1-full */
int isQueueFull();

#endif
