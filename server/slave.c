/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/slave.c                             */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sun 06 Jan 2013 07:32:55 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ServerCtrl.h"
#include "../common/dbProtocol.h"
#include "../client/clientsocket.h"

SlaveList slaves;
int pos;  // This server's position in slaves.
Socket mastersock;
char *addr;
int port;

void RegisterAndLoadSlaves(Socket sockfd)
{
    DBPacketHeader hd;
    char szBuf[MAX_BUF_LEN] = "\0";
    hd.cmd = ADD_SLAVE;
    hd.key = port;
    WriteHeader(szBuf, &hd);
    SendMsg(sockfd, szBuf);
    RecvMsg(sockfd, szBuf);
    DBPacketHeader *phd = (DBPacketHeader *)szBuf;
    assert(phd->cmd == ADD_SLAVE_R);
    pos = phd->key;
    slaves = *(SlaveList *)GetAppend(phd);
    printslaves(slaves);
}

int main(int argc, char *argv[])
{
    //Socket sockfd;

    if (argc != 3)
    {
        printf("Address and port is needed.\n");
        exit(-1);
    }

    addr = argv[1];
    port = atoi(argv[2]);

    if (-1 == OpenRemoteService(&mastersock, addr, port))
        return -1;
    
    RegisterAndLoadSlaves(mastersock);

    return 0;
}
