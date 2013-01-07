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

int DBInitializeService(int *psockfd, char *addr, int port)
{
    InitializeService(psockfd, addr, port);
    MDB = MDBCreate();
    return 0;
}

void DBShutdownService(int sockfd)
{
    MDBClose(MDB);
    ShutdownService(sockfd);
}

ClientSockHandle DBServiceStart(int sockfd)
{
    ClientSockHandle hcsock;

    ServiceStart(sockfd, &hcsock.sock, &hcsock.ip);
    printf("Accept connnection from %s\n", inet_ntoa(*(struct in_addr *)&hcsock.ip));

    MDBSet(MDB, (void*)&hcsock.sock, sizeof(int), (void*)&hcsock, sizeof(ClientSockHandle));
    return hcsock;
}

void DBServiceStop(int sockfd)
{
    ClientSockHandle *phcsock;
    int vsize = -1;

    phcsock = MDBGet(MDB, &sockfd, sizeof(int), &vsize); // free in HanldeRequest

    ServiceStop(phcsock->sock);
    printf("Close connection from %s\n", inet_ntoa(*(struct in_addr *)&phcsock->ip));

    MDBDel(MDB, &(phcsock->sock), sizeof(int));
}

void DBSendMsg(ClientSockHandle hcsock, char *buf)
{
    assert(hcsock.sock != -1);
    int res = send(hcsock.sock, buf, MAX_BUF_LEN, 0);
    if (res == -1) 
        perror("send");

    MDBSet(MDB, &hcsock.sock, sizeof(int), &hcsock, sizeof(ClientSockHandle));
}

ClientSockHandle *DBRecvMsg(int sockfd, char *buf)
{
    ClientSockHandle *phcsock;

    assert(sockfd != -1);
    int res = recv(sockfd, buf, MAX_BUF_LEN, 0);
    if (res == -1)
        perror("receive");

    int vsize = -1;
    phcsock = MDBGet(MDB, &sockfd, sizeof(int), &vsize); // free in HanldeRequest

    return phcsock;
}

