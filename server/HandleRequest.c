/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/HandleRequest.c                     */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Tue 25 Dec 2012 11:03:22 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>

#include "HandleRequest.h"
#include "serversocket.h"
#include "CircularQueue.h"
#include "../common/dbProtocol.h"
#include "../common/Database.h"

#define THREADS_NUM 2

#define pdebug printf

sem_t MSG_SEM;
pthread_t thread_id[THREADS_NUM];

void HandleRequest(int id);

void InitThreads()
{
    int i;
    for (i=0; i<THREADS_NUM; i++)
    {
        if (0 != pthread_create(&thread_id[i], NULL, (void*)HandleRequest,(void*)i))
        {
            perror("create thread");
        }
        //printf("The no.%d thread's id is %u\n", i, (int)thread_id[i]);
    }

}

void KillThreads()
{
    int i;
    for (i=0; i<THREADS_NUM; i++)
        pthread_cancel(thread_id[i]);

    //pthread_mutex_destroy(&DBMUTEX);
}

void HandleRequest(int id)
{
    char szReplyMsg[MAX_BUF_LEN] = "\0";
    DataBase hdb; 
    char *strAppend = NULL;
    DBPacketHeader *precvhd; 
    DBPacketHeader sendhd;
    ClientSockHandle hcsock;
    QueueNode *pnode;

    while (1)
    {
        pdebug("Thread %u is waiting.\n", id);
        sem_wait(&MSG_SEM);
        pnode = DeQueue();
        hcsock = pnode->hcsock;
        hdb = pnode->hcsock.app; // will be changed only when 'OPEN'
        precvhd = (DBPacketHeader *)pnode->buf;
        pdebug("Thread %u received:\n", id);
        debug(precvhd);

        switch (precvhd->cmd)
        {
            case OPEN:
                {
                    hdb = DBCreate(GetAppend(precvhd));
                    hcsock.app = hdb;
                    if (hdb == NULL)
                        sendhd.cmd = CMDFAIL;
                    else
                        sendhd.cmd = OPEN_R;
                    break;
                }
            case CLOSE:
                {
                    int res = DBDelete(hdb);
                    if (res != 0)
                        sendhd.cmd = CMDFAIL;
                    else
                        sendhd.cmd = CLOSE_R;
                    break;
                }
            case SET:
                {
                    if (0 != DBSetKeyValue(hdb, precvhd->key, GetAppend(precvhd)))
                        sendhd.cmd = CMDFAIL;
                    else
                        sendhd.cmd = SET_R;
                    break;
                }
            case GET:
                {
                    strAppend = DBGetKeyValue(hdb, precvhd->key);
                    if (strAppend == NULL)
                        sendhd.cmd = CMDFAIL;
                    else
                        sendhd.cmd = GET_R;
                    break;
                }
            case DEL:
                {
                    if (0 != DBDelKeyValue(hdb, precvhd->key))
                        sendhd.cmd = CMDFAIL;
                    else
                        sendhd.cmd = DEL_R;
                    break;
                }
            default:
                {
                    fprintf(stderr, "Unknown command.\n");
                }
        }

        /* write send buffer */
        WriteHeader(szReplyMsg, &sendhd);
        if (sendhd.cmd == CMDFAIL)
        {
            char *err = DBGetLastErrorMsg();
            Append(szReplyMsg, err, strlen(err) + 1);
        }
        else if (sendhd.cmd == GET_R)
        {
            Append(szReplyMsg, strAppend, strlen(strAppend) + 1);
            free(strAppend);
        }

        DBSendMsg(hcsock, szReplyMsg);

        if (sendhd.cmd == CLOSE_R)
            DBServiceStop(hcsock.sock); 
    }
}
