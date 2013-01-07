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
#include <pthread.h>
#include <assert.h>
#include "ServerCtrl.h"
#include "../common/dbProtocol.h"
#include "../common/Socket.h"

SlaveList slaves;
int pos;  // This server's position in slaves.
char *addr;
int port;

void AddNewSlave(int pos, SlaveNode sn)
{
    int i;
    printf("slaves: num:%d pos:%d\n", slaves.num, pos);
    for (i=slaves.num-1; i>=pos; i--)
        slaves.nodes[i+1] = slaves.nodes[i];
    slaves.nodes[pos] = sn;
    slaves.num ++;
    slaves.version ++;
}

void HandleCtrlRequest(int mastersock)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader *phd;
    DBPacketHeader hd;

    while (1)
    {
        RecvMsg(mastersock, szBuf);
        phd = (DBPacketHeader *)szBuf;

        switch (phd->cmd)
        {
            case NEW_SLAVE:
                {
                    AddNewSlave(phd->key, *(SlaveNode *)GetAppend(phd));
                    printslaves(slaves);
                    hd.cmd = NEW_SLAVE_R;
                    WriteHeader(szBuf, &hd);
                    SendMsg(mastersock, szBuf);
                    break;
                }
            case RM_SLAVE:
                {
                }
            default:
                {
                }
        }
    }
}

void RegisterAndLoadSlaves(int mastersock)
{
    DBPacketHeader hd;
    char szBuf[MAX_BUF_LEN] = "\0";
    hd.cmd = ADD_SLAVE;
    hd.key = port;
    WriteHeader(szBuf, &hd);
    SendMsg(mastersock, szBuf);
    RecvMsg(mastersock, szBuf);
    DBPacketHeader *phd = (DBPacketHeader *)szBuf;
    assert(phd->cmd == ADD_SLAVE_R);
    pos = phd->key;
    slaves = *(SlaveList *)GetAppend(phd);
    printslaves(slaves);
}

int main2(int argc, char *argv[])
{
    int mastersock;
    pthread_t ctrl_thread_id;

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

    pthread_create(&ctrl_thread_id, NULL, (void*)HandleCtrlRequest, (void*)mastersock);

    while (1)
        ;  // DB operating

    return 0;
}
