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
#include <stdlib.h>
#include <sys/epoll.h>
#include <semaphore.h>
#include "serversocket.h"
#include "CircularQueue.h"
#include "HandleRequest.h"

int main(int argc, char *argv[])
{
    int listen_sock;
    int epollid;
    struct epoll_event event;
    char szBuf[MAX_BUF_LEN] = "\0";

    if (argc != 3)
    {
        printf("Address and port is needed.\n");
        exit(-1);
    }

    if (-1 == DBInitializeService(&listen_sock, argv[1], atoi(argv[2])))
        return -1;
    InitQueue(); // message queue is shared by all threads
    sem_init(&MSG_SEM, 0, 0); // also shared by all threads
    InitThreads();

    epollid = epoll_create(1024);
    event.data.fd = listen_sock;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epollid, EPOLL_CTL_ADD, listen_sock, &event);

    while(1)
    {
        epoll_wait(epollid, &event, 1, -1);
        if (event.data.fd == listen_sock)
        {
            ClientSockHandle hcsock;
            hcsock = DBServiceStart(listen_sock);
            if (hcsock.sock == -1)
                continue;

            event.data.fd = hcsock.sock;
            event.events = EPOLLIN;
            epoll_ctl(epollid, EPOLL_CTL_ADD, hcsock.sock, &event);
        }
        else if ((event.events & EPOLLIN) && (event.events & EPOLLRDHUP))
        {
            DBServiceStop(event.data.fd);
        }
        else
        {
            ClientSockHandle *phcsock;
            phcsock = DBRecvMsg(event.data.fd, szBuf);

            QueueNode qn;
            qn.hcsock = *phcsock;
            qn.buf = szBuf;

            EnQueue(&qn);
            sem_post(&MSG_SEM);
            free(phcsock);
        }
    }

    sem_destroy(&MSG_SEM);
    KillThreads();
    DBShutdownService(listen_sock);

    return 0;
}
