/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  dbserver.c                                 */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  DistributedDB                              */
/*  Module Name            :  Server                                     */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */
/*  Created Time           :  Sat 15 Dec 2012 10:05:50 AM CST            */
/*  Description            :  Server of DistributedDB                    */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../common/dbProtocol.h"
#include "../common/Database.h"
#include "serversocket.h"

pthread_mutex_t mutex;

void HandleRequest(ClientSockHandle *phcsock)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    char szReplyMsg[MAX_BUF_LEN] = "hi\0";
    DataBase hdb = NULL;
    char *strAppend = NULL;
    DBPacketHeader *precvhd; 
    DBPacketHeader sendhd;
    ClientSockHandle hcsock = *phcsock;

    do
    {
        RecvMsg(hcsock.sock, szBuf);
        precvhd = (DBPacketHeader *)szBuf;
        debug(szBuf);

        switch (precvhd->cmd)
        {
            case OPEN:
                {
                    pthread_mutex_lock(&mutex);
                    hdb = DBCreate(GetAppend(precvhd));
                    pthread_mutex_unlock(&mutex);
                    if (hdb == NULL)
                        sendhd.cmd = CMDFAIL;
                    else
                        sendhd.cmd = OPEN_R;
                    break;
                }
            case CLOSE:
                {
                    pthread_mutex_lock(&mutex);
                    int res = DBDelete(hdb);
                    pthread_mutex_unlock(&mutex);
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

        SendMsg(hcsock.sock, szReplyMsg);
    } while (sendhd.cmd != CLOSE_R);

    ServiceStop(hcsock); 
}

int main()
{
    Socket sockfd;
    ClientSockHandle *phcsock;
    pthread_t thread_id;

    if (-1 == InitializeService(&sockfd, LOCAL_ADDR))
        return -1;
    pthread_mutex_init(&mutex, NULL);

    while(1)
    {
        phcsock = malloc(sizeof(ClientSockHandle));
        *phcsock = ServiceStart(sockfd);
        if (phcsock->sock == -1)
            continue;
        if (0 != pthread_create(&thread_id, NULL, (void*)HandleRequest, (void*)phcsock))
        {
            perror("create thread");
            ServiceStop(*phcsock);
        }
    }
    pthread_mutex_destroy(&mutex);
    ShutdownService(sockfd);

    return 0;
}
