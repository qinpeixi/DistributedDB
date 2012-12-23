/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/serversocket.c                      */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :  Linux                                      */
/*  Created Time           :  Sun 23 Dec 2012 03:17:37 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include "serversocket.h"

int InitializeService(Socket *sockfd, char *addr)
{
    struct sockaddr_in sa;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd == -1)
    {
        perror("socket");
        return -1;
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = addr ? inet_addr(addr) : INADDR_ANY;
    memset(&sa.sin_zero, 0, 8);

    int res;
    
    res = bind(*sockfd, (struct sockaddr *)&sa, sizeof(sa));
    if (res != 0)
    {
        perror("bind");
        return -1;
    }

    res = listen(*sockfd, MAX_CONNECT_QUEUE);
    if (res != 0)
    {
        perror("listen");
        return -1;
    }

    return 0;
}

void ShutdownService(Socket sockfd)
{
    close(sockfd);
}
    
ClientSockHandle ServiceStart(Socket sockfd)
{
    struct sockaddr_in sa;
    ClientSockHandle hcsock;

    socklen_t len = sizeof(struct sockaddr);
    hcsock.sock = accept(sockfd, (struct sockaddr *)&sa, &len);
    if (hcsock.sock == -1)
    {
        perror("accept");
        return hcsock;
    }

    memcpy(hcsock.addr, inet_ntoa(sa.sin_addr), MAX_STRADDR_LEN);
    printf("Accept connnection from %s\n", hcsock.addr);
    return hcsock;
}

void ServiceStop(ClientSockHandle hcsock)
{
    printf("Close connnection from %s\n", hcsock.addr);
    close(hcsock.sock);
}

void SendMsg(Socket sockfd, char *buf)
{
    assert(sockfd != -1);
    int res = send(sockfd, buf, MAX_BUF_LEN, 0);
    if (res == -1) 
        perror("send");
}

void RecvMsg(Socket sockfd, char *buf)
{
    assert(sockfd != -1);
    int res = recv(sockfd, buf, MAX_BUF_LEN, 0);
    if (res == -1)
        perror("receive");
}

