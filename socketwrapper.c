/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  socketwrapper.c                            */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 15 Dec 2012 02:43:54 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "socketwrapper.h"

SocketHandler* CreateSocketHandler(char *addr, int port)
{
    SocketHandler *sh;
    sh = malloc(sizeof(SocketHandler));
    sh->sockfd = socket(PF_INET, SOCK_STREAM, 0);
    sh->serveraddr.sin_family = AF_INET;
    sh->serveraddr.sin_port = htons(port);
    sh->serveraddr.sin_addr.s_addr = inet_addr(addr);
    memset(&(sh->serveraddr.sin_zero), 0, 8);
    sh->newfd = -1;

    return sh;
}

int InitializeService(SocketHandler *sh)
{
    assert(sh != NULL);
    int ret = bind(sh->sockfd,  \
            (struct sockaddr *)&(sh->serveraddr), \
            sizeof(struct sockaddr));
    if (ret == -1)
    {                                               
        fprintf(stderr,"Bind Error,%s:%d\n", __FILE__,__LINE__);
        close(sh->sockfd);                              
        return -1;                                  
    }
    listen(sh->sockfd, MAX_CONNECT_QUEUE);

    return 0;
}

void ShutdownService(SocketHandler *sh)
{
    close(sh->sockfd);
}

int ServiceStart(SocketHandler *sh)
{
    assert(sh != NULL);
    int len = sizeof(struct sockaddr);
    sh->newfd = accept(sh->sockfd, \
            (struct sockaddr *)&(sh->clientaddr), &len);
    if (sh->newfd == -1)
    {
        fprintf(stderr, "Accept error, %s:%d\n", __FILE__,__LINE__);
        return -1;
    }
    
    return 0;
}

void ServiceStop(SocketHandler *sh)
{
    close(sh->newfd);
}

int OpenRemoteService(SocketHandler *sh)
{
    assert(sh != NULL);
    int ret = connect(sh->sockfd,  \
            (struct sockaddr *)&(sh->serveraddr), \
            sizeof(struct sockaddr));
    if (ret == -1)
    {                                               
        fprintf(stderr,"Connect Error,%s:%d\n",__FILE__,__LINE__); 
        close(sh->sockfd);
        return -1;                                 
    }
    sh->newfd = sh->sockfd;

    return 0;
}

void CloseRemoteService(SocketHandler *sh)
{
    close(sh->sockfd);
}

void RecvMsg(SocketHandler *sh, char *buf)
{
    assert(sh && sh->newfd != -1);
    int ret = recv(sh->newfd, buf, MAX_BUF_LEN, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Recvive error,%s:%d\n", __FILE__, __LINE__);
    }
}

void SendMsg(SocketHandler *sh, char *buf)
{
    assert(sh && sh->newfd != -1);
    int ret = send(sh->newfd, buf, MAX_BUF_LEN, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Send error,%s:%d\n", __FILE__, __LINE__);
    }
}

char *GetClientIP(SocketHandler *sh)
{
    return (char*)inet_ntoa(sh->clientaddr.sin_addr);
}

int GetClientPort(SocketHandler *sh)
{
    return ntohs(sh->clientaddr.sin_port);
}
