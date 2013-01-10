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

int GetLength(int pos)
{
    int len = slaves.nodes[(pos+1)%slaves.num].key - slaves.nodes[pos].key;
    len = (len + MAX_KEY - 1) % MAX_KEY + 1;
    return len;
}

int FindLongestSection()
{
    int i;
    int maxlen = -1;
    int index = -1;
    for (i=0; i<slaves.num; i++)
    {
        int len = GetLength(i);
        if (maxlen < len)
        {
            maxlen = len;
            index = i;
        }
    }

    return index;
}

// add a new slave to slaves, return the index of the new slave
int AddToSlaveList(SlaveNode sn)
{
    int newpos;
    if (slaves.num == 0)
    {
        srand(time(0));
        sn.key = rand() % MAX_KEY;
        slaves.nodes[0] = sn;
        newpos = 0;
    }
    else
    {
        int i;
        int pos = FindLongestSection();

        for (i=slaves.num-1; i>pos; i--)
            slaves.nodes[i+1] = slaves.nodes[i];

        newpos = pos + 1;
        sn.key = (GetLength(pos)/2 + slaves.nodes[pos].key) % MAX_KEY;
        slaves.nodes[newpos] = sn;
    }

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
    for (i=0; i<slaves.num; i++)
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
    printf("HandleRequest.\n");

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
                SlaveNode sn = {ip, phd->key, sock};
                int newpos = AddToSlaveList(sn);
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

    if (-1 == InitializeService(&listensock, NULL, 5001))
        return -1;

    slaves.num = 0;
    slaves.version = 0;

    while (1)
    {
        ServiceStart(listensock, &acsock, &ip);
        if (acsock != -1)
            HandleRequest(acsock, ip);
    }

    ShutdownService(listensock);

}
