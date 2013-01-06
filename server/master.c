/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/master.c                            */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sun 06 Jan 2013 03:24:12 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ServerCtrl.h"
#include "serversocket.h"
#include "../common/dbProtocol.h"

SlaveList slaves;

int FindLongestSection()
{
    int i;
    int maxlen = -1;
    int index = -1;
    for (i=0; i<slaves.num-1; i++)
    {
        if (maxlen < slaves.nodes[i+1].key - slaves.nodes[i].key)
        {
            maxlen = slaves.nodes[i+1].key - slaves.nodes[i].key;
            index = i;
        }
    }

    return index;
}

// add a new slave to slaves, return the index of the new slave
int AddToSlaveList(char *addr, int port, Socket sock)
{
    int i;
    int pos = FindLongestSection();

    for (i=slaves.num-1; i>pos; i--)
        slaves.nodes[i+1] = slaves.nodes[i];

    int newpos = pos + 1;
    slaves.nodes[newpos].ip = inet_addr(addr);
    slaves.nodes[newpos].port = port;
    slaves.nodes[newpos].sock = sock;
    slaves.nodes[newpos].key = (slaves.nodes[newpos+1].key + slaves.nodes[pos].key) / 2;
    slaves.version ++;
    slaves.num ++;

    return newpos;
}

void NotifyAll(int newpos)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = NEW_SLAVE;
    hd.key = newpos;
    WriteHeader(szBuf, &hd);
    Append(szBuf, (char *)&(slaves.nodes[newpos]), sizeof(SlaveNode));
    int i;
    ClientSockHandle hc;
    for (i=1; i<slaves.num-1; i++)
    {
        if (i == newpos)
            continue;
        hc.sock = slaves.nodes[i].sock;
        SendMsg(hc, szBuf);
    }
}

void HandleRequest(ClientSockHandle hcsock)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    char szReplyMsg[MAX_BUF_LEN] = "\0";
    DBPacketHeader *phd;
    DBPacketHeader hd;

    RecvMsg(hcsock.sock, szBuf);
    phd = (DBPacketHeader *)szBuf;
    debug(szBuf);

    switch (phd->cmd)
    {
        case GET_LIST:
            {
                hd.cmd = GET_LIST_R;
                WriteHeader(szReplyMsg, &hd);
                Append(szReplyMsg, (char *)&slaves, sizeof(slaves));
                break;
            }
        case ADD_SLAVE:
            {
                int newpos = AddToSlaveList(hcsock.addr, phd->key, hcsock.sock);
                printslaves(slaves);
                //NotifyAll(newpos);
                hd.cmd = ADD_SLAVE_R;
                hd.key = newpos;
                WriteHeader(szReplyMsg, &hd);
                Append(szReplyMsg, (char *)&slaves, sizeof(slaves));
                break;
            }
        case DEL_SLAVE:
            {
                break;
            }
        default:
            {
                break;
            }
    }

    SendMsg(hcsock, szReplyMsg);

    if (phd->cmd != ADD_SLAVE)
        ServiceStop(hcsock.sock);
}

int main(int argc, char *argv[])
{
    Socket listensock;
    ClientSockHandle hcsock;
    //pthread_t heart_thread_id;

    if (argc != 3)
    {
        printf("Address and port is needed.\n");
        exit(-1);
    }

    if (-1 == InitializeService(&listensock, argv[1], atoi(argv[2])))
        return -1;

    // guard elements, not corresbonding to any real slaves
    slaves.nodes[0].key = 0;
    slaves.nodes[0].sock = -1;
    slaves.nodes[1].key = 65536;
    slaves.nodes[1].sock = -1;
    slaves.num = 2;
    slaves.version = 0;

    while (1)
    {
        hcsock = ServiceStart(listensock);
        if (hcsock.sock != -1)
            HandleRequest(hcsock);
    }

    ShutdownService(listensock);

}
