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
#include <assert.h>
#include <pthread.h>
#include "CircularQueue.h"


QueueNode Q[MAX_QUEUE_LEN];
int head = 0;
int tail = 0;
pthread_mutex_t QUEUEMUTEX;

void InitQueue()
{
    int i;
    for (i=0; i<MAX_QUEUE_LEN; i++)
        Q[i].buf = (char *)malloc(MAX_BUF_LEN);
    pthread_mutex_init(&QUEUEMUTEX, NULL);
}

void EnQueue(QueueNode *pnode)
{
    pthread_mutex_lock(&QUEUEMUTEX);
    assert(isQueueFull() == 0);
    memcpy(&Q[tail], pnode, sizeof(QueueNode));
    tail = (tail+1) % MAX_QUEUE_LEN;
    pthread_mutex_unlock(&QUEUEMUTEX);
}

QueueNode *DeQueue()
{
    pthread_mutex_lock(&QUEUEMUTEX);
    assert(isQueueEmpty() == 0);
    QueueNode *pnode = &Q[head];
    head = (head+1) % MAX_QUEUE_LEN;
    pthread_mutex_unlock(&QUEUEMUTEX);
    
    return pnode;
}

int isQueueFull()
{
    return head == (tail+1) % MAX_QUEUE_LEN;
}

int isQueueEmpty()
{
    return head == tail;
}
