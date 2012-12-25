/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/CircularQueue.c                     */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Tue 25 Dec 2012 03:31:01 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CircularQueue.h"

#define MAX_QUEUE_LEN 1024

QueueNode Q[MAX_QUEUE_LEN];
int head = 0;
int tail = 0;

void InitQueue()
{
    int i;
    for (i=0; i<MAX_QUEUE_LEN; i++)
        Q[i].buf = (char *)malloc(MAX_BUF_LEN);
}

void EnQueue(QueueNode *pnode)
{
    memcpy(&Q[tail], pnode, sizeof(QueueNode));
    tail = (tail+1) % MAX_QUEUE_LEN;
}

QueueNode *DeQueue()
{
    QueueNode *pnode = &Q[head];
    head = (head+1) % MAX_QUEUE_LEN;
    
    return pnode;
}

int isQueueFull()
{
    return head == (tail+1) % MAX_QUEUE_LEN;
}
