/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  client/clientsocket.c                      */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Mon 24 Dec 2012 08:58:09 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include "clientsocket.h"

int OpenRemoteService(Socket *psockfd, char *addr, int port)
{
    struct sockaddr_in sa;

    *psockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*psockfd == -1)
    {
        perror("socket");
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

void CloseRemoteService(Socket sockfd)
{
    close(sockfd);
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
