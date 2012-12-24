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
#include "../common/dbProtocol.h"
//#include "../common/socketwrapper.h"
#include "serversocket.h"
#include "../common/Database.h"

void HandleRequest(ClientSockHandle hcsock)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    char szReplyMsg[MAX_BUF_LEN] = "hi\0";
    DataBase hdb = NULL;
    char *strAppend = NULL;
    DBPacketHeader *phd; 
    DBPacketHeader hd;

    do
    {
        RecvMsg(hcsock.sock, szBuf);
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
                        hd.cmd = CLOSE_R;
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

        SendMsg(hcsock.sock, szReplyMsg);
    } while (hd.cmd != CLOSE_R);

    ServiceStop(hcsock); 
}

int main()
{
    Socket sockfd;
    ClientSockHandle hcsock;

    InitializeService(&sockfd, LOCAL_ADDR);
    while(1)
    {
        hcsock = ServiceStart(sockfd);
        if (hcsock.sock != -1)
            HandleRequest(hcsock);
    }
    ShutdownService(sockfd);

    return 0;
}
