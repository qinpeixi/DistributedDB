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
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "serversocket.h"
#include "HandleRequest.h"

char szBuf[MAX_BUF_LEN] = "\0";
pthread_mutex_t mutex;

int main()
{
    Socket listensock;
    pthread_t thread_id;
    int epollid;
    struct epoll_event event;

    if (-1 == InitializeService(&listensock, LOCAL_ADDR))
        return -1;
    pthread_mutex_init(&mutex, NULL);

    epollid = epoll_create(1024);
    event.data.fd = listensock;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epollid, EPOLL_CTL_ADD, listensock, &event);

    while(1)
    {
        epoll_wait(epollid, &event, 1, -1);
        if (event.data.fd == listensock)
        {
            ClientSockHandle hcsock;
            hcsock = ServiceStart(listensock);
            if (hcsock.sock == -1)
                continue;
            event.data.fd = hcsock.sock;
            event.events = EPOLLIN;
            epoll_ctl(epollid, EPOLL_CTL_ADD, hcsock.sock, &event);
        }
        else
        {
            ClientSockHandle *phcsock;
            phcsock = RecvMsg(event.data.fd, szBuf);
            if (0 != pthread_create(&thread_id, NULL, (void*)HandleRequest, (void*)phcsock))
            {
                perror("create thread");
                ServiceStop(*phcsock);
            }
        }
    }

    pthread_mutex_destroy(&mutex);
    ShutdownService(listensock);

    return 0;
}
