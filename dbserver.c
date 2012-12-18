/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  dbserver.c                                 */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 15 Dec 2012 10:05:50 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "dbProtocol.h"
#include "socketwrapper.h"
#include "Database.h"

DataBase hdb;

void Execute(const char *buf, char *repbuf)
{
    DBPacketHeader *phd = (DBPacketHeader *)buf;

    switch (phd->cmd)
    {
        case OPEN:
            {
                hdb = DBCreate(GetAppend(phd));
                DBPacketHeader hd;
                if (hdb == NULL)
                    hd.cmd = CMDFAIL;
                else
                    hd.cmd = OPEN_R;
                WriteHeader(repbuf, &hd);
                break;
            }
        case CLOSE:
            {
                DBPacketHeader hd;
                if (DBDelete(hdb) != 0)
                    hd.cmd = CMDFAIL;
                else
                    hd.cmd = CLOSE_R;
                WriteHeader(repbuf, &hd);
                break;
            }
        default:
            {
                fprintf(stderr, "Unknown command.\n");
            }
    }
}

int main()
{
    char szBuf[MAX_BUF_LEN] = "\0";
    char szReplyMsg[MAX_BUF_LEN] = "hi\0";
    SocketHandler *sh;
    sh = CreateSocketHandler(IP_ADDR, PORT);
    InitializeService(sh);
    while(1)
    {
        memset(szBuf, 0, MAX_BUF_LEN);
        memset(szReplyMsg, 0, MAX_BUF_LEN);
        ServiceStart(sh);
        RecvMsg(sh, szBuf); 
        Execute(szBuf, szReplyMsg);
        SendMsg(sh, szReplyMsg); 
        ServiceStop(sh); 
    }
    ShutdownService(sh);
    return 0;
}
