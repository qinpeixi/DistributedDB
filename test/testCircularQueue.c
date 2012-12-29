/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  testCircularQueue.c                        */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 29 Dec 2012 10:58:31 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <assert.h>
#include "../server/CircularQueue.h"

int main()
{
    QueueNode q;
    QueueNode *pq;
    int i;
    InitQueue();
    for (i=0; i<MAX_QUEUE_USELEN; i++)
    {
        q.hcsock.sock = i;
        EnQueue(&q);
    }
    assert(isQueueFull() != 0);

    for (i=0; i<MAX_QUEUE_USELEN/2; i++)
    {
        pq = DeQueue();
        assert(pq->hcsock.sock == i);
    }
    assert(isQueueFull() == 0);

    while (isQueueEmpty() == 0)
    {
        pq = DeQueue();
        assert(pq->hcsock.sock == i++);
    }
    assert(i == MAX_QUEUE_USELEN);

    printf("Test of CircularQueue.c       : PASS\n");

    return 0;
}
