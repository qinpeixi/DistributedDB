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
#include "MemoryDB.h"

MemDB MDB;

int InitializeService(Socket *psockfd, char *addr, int port)
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
    sa.sin_addr.s_addr = addr ? inet_addr(addr) : INADDR_ANY;
    memset(&sa.sin_zero, 0, 8);

    int res;
    
    res = bind(*psockfd, (struct sockaddr *)&sa, sizeof(sa));
    if (res != 0)
    {
        perror("bind");
        return -1;
    }

    res = listen(*psockfd, MAX_CONNECT_QUEUE);
    if (res != 0)
    {
        perror("listen");
        return -1;
    }

    MDB = MDBCreate();

    return 0;
}

void ShutdownService(Socket sockfd)
{
    MDBClose(MDB);
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

    MDBSet(MDB, (void*)&hcsock.sock, sizeof(Socket), (void*)&hcsock, sizeof(ClientSockHandle));

    return hcsock;
}

void ServiceStop(Socket sockfd)
{
    ClientSockHandle *phcsock;
    int vsize = -1;

    phcsock = MDBGet(MDB, &sockfd, sizeof(Socket), &vsize); // free in HanldeRequest
    printf("Close connnection from %s\n", phcsock->addr);
    close(phcsock->sock);

    MDBDel(MDB, &(phcsock->sock), sizeof(Socket));
}

void SendMsg(ClientSockHandle hcsock, char *buf)
{
    assert(hcsock.sock != -1);
    int res = send(hcsock.sock, buf, MAX_BUF_LEN, 0);
    if (res == -1) 
        perror("send");

    MDBSet(MDB, &hcsock.sock, sizeof(Socket), &hcsock, sizeof(ClientSockHandle));
}

ClientSockHandle *RecvMsg(Socket sockfd, char *buf)
{
    ClientSockHandle *phcsock;

    assert(sockfd != -1);
    int res = recv(sockfd, buf, MAX_BUF_LEN, 0);
    if (res == -1)
        perror("receive");

    int vsize = -1;
    phcsock = MDBGet(MDB, &sockfd, sizeof(Socket), &vsize); // free in HanldeRequest

    return phcsock;
}

