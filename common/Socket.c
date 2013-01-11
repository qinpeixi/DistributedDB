/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  common/Socket.c                            */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Mon 07 Jan 2013 08:05:13 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include "Socket.h"

int InitializeService(int *listen_sock, char *addr, int port)
{
    struct sockaddr_in sa;
    int opt = 1;
    socklen_t len = sizeof(opt);

    *listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(*listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, len);
    if (*listen_sock == -1)
    {
        perror("socket");
        return -1;
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = addr ? inet_addr(addr) : INADDR_ANY;
    memset(&sa.sin_zero, 0, 8);

    int res;

    res = bind(*listen_sock, (struct sockaddr *)&sa, sizeof(sa));
    if (res != 0)
    {
        perror("bind");
        return -1;
    }

    res = listen(*listen_sock, MAX_CONNECT_QUEUE);
    if (res != 0)
    {
        perror("listen");
        return -1;
    }
    struct sockaddr_in sin;
    len = sizeof(sin);
    getsockname(*listen_sock, (struct sockaddr *)&sin, &len);
    printf("%s:%d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    return 0;
}

void ShutdownService(int listen_sock)
{
    close(listen_sock);
}

int ServiceStart(int listen_sock, int *accept_sock, int *ip)
{
    struct sockaddr_in sa;
    socklen_t len = sizeof(struct sockaddr);
    *accept_sock = accept(listen_sock, (struct sockaddr *)&sa, &len);
    if (*accept_sock == -1)
    {
        perror("accept");
        return -1;
    }

    *ip = *(int*)&sa.sin_addr;
    //printf("accept from:%s\n", inet_ntoa(sa.sin_addr));
    return 0;
}

void ServiceStop(int sockfd)
{
    close(sockfd);
}

int OpenRemoteService(int *psockfd, char *addr, int port)
{
    struct sockaddr_in sa;

    *psockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*psockfd == -1)
    {
        perror("int");
        return -1;
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = addr ? inet_addr(addr) : inet_addr(LOCAL_ADDR);
    memset(&sa.sin_zero, 0, 8);

    int res;

    res = connect(*psockfd, (struct sockaddr *)&sa, sizeof(struct sockaddr));
    if (res == -1)
    {
        perror("connet");
        return -1;
    }

    return 0;
}

int OpenRemoteService2(int *psockfd, int ip, int port)
{
    return OpenRemoteService(psockfd, inet_ntoa(*(struct in_addr *)&ip), port);
}

void CloseRemoteService(int sockfd)
{
    close(sockfd);
}

void SendMsg(int sockfd, char *buf)
{
    assert(sockfd != -1);
    int res = send(sockfd, buf, MAX_BUF_LEN, 0);
    if (res == -1) 
        perror("send");
}

void RecvMsg(int sockfd, char *buf)
{
    assert(sockfd != -1);
    int res = recv(sockfd, buf, MAX_BUF_LEN, 0);
    if (res == -1)
        perror("receive");
}

int GetPort(int sockfd)
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(sockfd, (struct sockaddr *)&sin, &len);
    //printf("%s:%d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    return ntohs(sin.sin_port);
}
