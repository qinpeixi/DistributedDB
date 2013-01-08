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
#include "../common/Socket.h"
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
int AddToSlaveList(int ip, int port, int sock)
{
    int i;
    int pos = FindLongestSection();

    for (i=slaves.num-1; i>pos; i--)
        slaves.nodes[i+1] = slaves.nodes[i];

    int newpos = pos + 1;
    slaves.nodes[newpos].ip = ip;
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
    char szReplyMsg[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = NEW_SLAVE;
    hd.key = newpos;
    WriteHeader(szBuf, &hd);
    Append(szBuf, (char *)&(slaves.nodes[newpos]), sizeof(SlaveNode));
    int i;
    int sock;
    for (i=1; i<slaves.num-1; i++)
    {
        if (i == newpos)
            continue;
        sock = slaves.nodes[i].sock;
        SendMsg(sock, szBuf);
        RecvMsg(sock, szReplyMsg);
        DBPacketHeader *phd = (DBPacketHeader *)szReplyMsg;
        if (phd->cmd != NEW_SLAVE_R)
            fprintf(stderr, "Receive NEW_SLAVE_R error.\n");
    }
}

void HandleRequest(int sock, int ip)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    char szReplyMsg[MAX_BUF_LEN] = "\0";
    DBPacketHeader *phd;
    DBPacketHeader hd;

    RecvMsg(sock, szBuf);
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
                int newpos = AddToSlaveList(ip, phd->key, sock);
                printslaves(slaves);
                NotifyAll(newpos);
                hd.cmd = ADD_SLAVE_R;
                hd.key = newpos;
                WriteHeader(szReplyMsg, &hd);
                Append(szReplyMsg, (char *)&slaves, sizeof(slaves));
                break;
            }
        case DEL_SLAVE:
            {
                // DelFromSlaveList(phd->key);
                // printslaves(slaves);
                // NotifyAll(phd->key);(RM_SLAVE)
                // hd.cmd = DEL_SLAVE_R
                // WriteHeader(szReplyMsg, &hd);
                break;
            }
        default:
            {
                break;
            }
    }

    SendMsg(sock, szReplyMsg);

    if (phd->cmd != ADD_SLAVE)
        ServiceStop(sock);
}

int main(int argc, char *argv[])
{
    int listensock;
    int acsock;
    int ip;
    //pthread_t heart_thread_id;

    if (-1 == InitializeService(&listensock, NULL, 0))
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
        ServiceStart(listensock, &acsock, &ip);
        if (acsock != -1)
            HandleRequest(acsock, ip);
    }

    ShutdownService(listensock);

}
