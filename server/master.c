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
#include <sys/epoll.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "ServerCtrl.h"
#include "../common/Socket.h"
#include "../common/dbProtocol.h"
#include "master.h"

#define UPDATE_BAKCUP_SECOND 5

SlaveList slaves;

extern DBPacketHeader* ExchangePacket(enum CMD cmd, int key, 
        const char *append_str, int len, int sockfd);
/*DBPacketHeader* ExchangePacket(enum CMD cmd, int key, 
        const char *append_str, int len, int sockfd)
{
    static char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = cmd;
    hd.key = key;
    WriteHeader(szBuf, &hd);
    Append(szBuf, append_str, len);
    SendMsg(sockfd, szBuf);
    RecvMsg(sockfd, szBuf);
    return (DBPacketHeader *)szBuf;
}*/

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

void DelFromSlaveList(int pos)
{
    int i;
    for (i=pos; i<slaves.num-1; i++)
        slaves.nodes[i] = slaves.nodes[i+1];
    slaves.num --;
    slaves.version ++;
}

void NotifyAll(int pos, enum CMD cmd, enum CMD rescmd)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    char szReplyMsg[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = cmd;
    hd.key = pos;
    WriteHeader(szBuf, &hd);
    Append(szBuf, (char *)&(slaves.nodes[pos]), sizeof(SlaveNode));
    int i;
    int sock;
    for (i=0; i<slaves.num; i++)
    {
        if (i == pos)
            continue;
        sock = slaves.nodes[i].sock;
        SendMsg(sock, szBuf);
        RecvMsg(sock, szReplyMsg);
        DBPacketHeader *phd = (DBPacketHeader *)szReplyMsg;
        if (phd->cmd != rescmd)
            fprintf(stderr, "Notify %d error.\n", i);
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
                printf("\nGET_LIST\n");
                hd.cmd = GET_LIST_R;
                WriteHeader(szReplyMsg, &hd);
                Append(szReplyMsg, (char *)&slaves, sizeof(slaves));
                break;
            }
        case ADD_SLAVE:
            {
                printf("\nADD_SLAVE\n");
                SlaveNode sn = {ip, phd->key, sock};
                int newpos = AddToSlaveList(sn);
                printslaves(slaves);
                NotifyAll(newpos, NEW_SLAVE, NEW_SLAVE_R);
                hd.cmd = ADD_SLAVE_R;
                hd.key = newpos;
                WriteHeader(szReplyMsg, &hd);
                Append(szReplyMsg, (char *)&slaves, sizeof(slaves));
                break;
            }
        case DEL_SLAVE:
            {
                printf("\nDEL_SLAVE\n");
                NotifyAll(phd->key, RM_SLAVE, RM_SLAVE_R);
                DelFromSlaveList(phd->key);
                printslaves(slaves);
                hd.cmd = DEL_SLAVE_R;
                WriteHeader(szReplyMsg, &hd);
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

void ShutDownMaster()
{
    printf("Shut down.\n");
    exit(0);
}

void UpdateBackup(int sig)
{
    static int pos = 0;
    pos = (pos + 1) % slaves.num;
    DBPacketHeader *phd = ExchangePacket(UPDATE_BACKUP12, 0, NULL, 0, 
            slaves.nodes[pos].sock);
    if (phd->cmd != UPDATE_BACKUP12_R)
        fprintf(stderr, "No.%d slave update backup error.\n", pos);
    alarm(UPDATE_BAKCUP_SECOND);
}

//int main(int argc, char *argv[])
int MasterProcess()
{
    int epollid;
    struct epoll_event event;
    int listen_sock;
    int acsock;
    int ip;
    //pthread_t heart_thread_id;

    if (-1 == InitializeService(&listen_sock, NULL, 5001))
        return -1;

    signal(SIGALRM, UpdateBackup);
    alarm(UPDATE_BAKCUP_SECOND);
    /*struct itimerval itv;
    struct itimerval olditv;
    itv.it_interval.tv_sec = UPDATE_BAKCUP_SECOND;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = UPDATE_BAKCUP_SECOND;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, &olditv);
    */

    slaves.num = 0;
    slaves.version = 0;

    epollid = epoll_create(1024);
    event.data.fd = listen_sock;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epollid, EPOLL_CTL_ADD, listen_sock, &event);

    while (1)
    {
        epoll_wait(epollid, &event, 1, -1);
        if (event.data.fd == listen_sock)
        {
            ServiceStart(listen_sock, &acsock, &ip);
            event.data.fd = acsock;
            event.events = EPOLLIN;
            epoll_ctl(epollid, EPOLL_CTL_ADD, acsock, &event);
            HandleRequest(acsock, ip);
        }
        else
        {
            if (errno == EINTR)
                printf("Alarm\n");
            else
            {
                printf("slave request.\n");
                HandleRequest(event.data.fd, ip);
            }
        }
    }

    ShutdownService(listen_sock);

}
