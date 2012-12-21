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
#include <string.h>
#include "dbProtocol.h"
#include "socketwrapper.h"
#include "Database.h"

//void Execute(const char *buf, char *repbuf)
void HandleRequest(SocketHandler *sh)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    char szReplyMsg[MAX_BUF_LEN] = "hi\0";
    DataBase hdb = NULL;
    char *strAppend;

    printf("Accept connection from %s:%d\n", GetClientIP(sh), GetClientPort(sh));
    while (1)
    {
        DBPacketHeader *phd; 
        DBPacketHeader hd;

        RecvMsg(sh, szBuf);
        phd = (DBPacketHeader *)szBuf;
        debug(szBuf);

        switch (phd->cmd)
        {
            case OPEN:
                {
                    hdb = DBCreate(GetAppend(phd));
                    if (hdb == NULL)
                        hd.cmd = CMDFAIL;
                    else
                        hd.cmd = OPEN_R;
                    break;
                }
            case CLOSE:
                {
                    if (DBDelete(hdb) != 0)
                        hd.cmd = CMDFAIL;
                    else
                    {
                        printf("Finish connection from %s:%d\n", GetClientIP(sh), GetClientPort(sh));
                        return;
                    }
                    break;
                }
            case SET:
                {
                    if (0 != DBSetKeyValue(hdb, hd.key, GetAppend(phd)))
                        hd.cmd = CMDFAIL;
                    else
                        hd.cmd = SET_R;
                    break;
                }
            case GET:
                {
                    strAppend = DBGetKeyValue(hdb, hd.key);
                    if (strAppend == NULL)
                        hd.cmd = CMDFAIL;
                    else
                        hd.cmd = GET_R;
                    break;
                }
            case DEL:
                {
                    if (0 != DBDelKeyValue(hdb, hd.key))
                        hd.cmd = CMDFAIL;
                    else
                        hd.cmd = DEL_R;
                    break;
                }
            default:
                {
                    fprintf(stderr, "Unknown command.\n");
                }
        }
        
        WriteHeader(szReplyMsg, &hd);
        if (hd.cmd == CMDFAIL)
        {
            char *err = DBGetLastErrorMsg();
            Append(szReplyMsg, err, strlen(err) + 1);
        }
        else if (hd.cmd == GET_R)
            Append(szReplyMsg, strAppend, strlen(strAppend) + 1);

        SendMsg(sh, szReplyMsg, ((DBPacketHeader *)szReplyMsg)->size);
    }
}

int main()
{
    SocketHandler *sh;
    sh = CreateSocketHandler(IP_ADDR, PORT);
    InitializeService(sh);
    while(1)
    {
        ServiceStart(sh);
        HandleRequest(sh);
        ServiceStop(sh); 
    }
    ShutdownService(sh);
    return 0;
}
