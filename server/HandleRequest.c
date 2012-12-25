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

#include "../common/dbProtocol.h"
#include "../common/Database.h"
#include "serversocket.h"
#include "HandleRequest.h"

void HandleRequest(ClientSockHandle *phcsock)
{
    char szReplyMsg[MAX_BUF_LEN] = "hi\0";
    DataBase hdb = phcsock->app; // will be changed only when 'OPEN'
    char *strAppend = NULL;
    DBPacketHeader *precvhd; 
    DBPacketHeader sendhd;
    ClientSockHandle hcsock = *phcsock;
    free(phcsock);

    precvhd = (DBPacketHeader *)szBuf;
    debug(szBuf);

    switch (precvhd->cmd)
    {
        case OPEN:
            {
                pthread_mutex_lock(&mutex);
                hdb = DBCreate(GetAppend(precvhd));
                pthread_mutex_unlock(&mutex);
                hcsock.app = hdb;
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

    SendMsg(hcsock, szReplyMsg);

    if (sendhd.cmd == CLOSE_R)
        ServiceStop(hcsock); 
}
