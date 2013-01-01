/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  netdbapi.c                                 */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  DistributedDB                              */
/*  Module Name            :  Netdbapi                                   */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */
/*  Created Time           :  Sat 15 Dec 2012 08:51:58 AM CST            */
/*  Description            :  Implementation of Database.h.              */
/*                         :  Connet to server using socketwrapper.h.    */ 
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common/Database.h"
#include "../common/dbProtocol.h"
#include "clientsocket.h"

#define NODES_NUM 3 

typedef struct 
{
    char addr[ADDR_STR_LEN];
    int port;
    Socket sock;
} CloudNode;

int ReadCloudNodes(CloudNode *pnodes)
{
    CloudNode node = {LOCAL_ADDR, 5001, -1};
    int i;
    for (i=0; i<NODES_NUM; i++)
    {
        pnodes[i] = node;
        pnodes[i].port = node.port + i;
    }

    return 0;
}

DataBase OpenAllCloudNodes(char *buf)
{
    /* establish a connection with server */
    CloudNode *nodes = malloc(NODES_NUM * sizeof(CloudNode));
    ReadCloudNodes(nodes);
    int i;
    char recvBuf[MAXPACKETLEN];
    for (i=0; i<NODES_NUM; i++)
    {
        if (-1 == OpenRemoteService(&(nodes[i].sock), nodes[i].addr, nodes[i].port))
        {
            fprintf(stderr, "Open remote service failed.\n");
            return NULL;
        }
        /* send server the command */
        SendMsg(nodes[i].sock, buf);
        RecvMsg(nodes[i].sock, recvBuf); 

        DBPacketHeader *phd = GetHeader(recvBuf);
        if (phd->cmd == CMDFAIL)
        {
            fprintf(stderr, "Receive error:%s\n", GetAppend(phd));
            return NULL;
        }
    }

    return (DataBase)nodes;
}

int CloseAllCloudNodes(DataBase hdb, char *buf)
{
    CloudNode *nodes = (CloudNode *)hdb;
    int i;
    char recvBuf[MAXPACKETLEN];
    for (i=0; i<NODES_NUM; i++)
    {
        SendMsg(nodes[i].sock, buf);
        RecvMsg(nodes[i].sock, recvBuf);

        DBPacketHeader *phd = GetHeader(recvBuf);
        if (phd->cmd == CMDFAIL)
        {
            fprintf(stderr, "Close error:%s\n", GetAppend(phd));
            return -1;
        }

        CloseRemoteService(nodes[i].sock);
    }
    free(nodes);

    return 0;
}

/* Get socket by key according to cloud strategy */
Socket *GetSocket(DataBase hdb, dbKey key)
{
    CloudNode *nodes = (CloudNode *)hdb;
    int index = key % NODES_NUM;
    //printf("Send to %d:%d\n", index, nodes[index].port);
    return &(nodes[index].sock);
}

DataBase DBCreate(char *dbName)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    /* write data using protocol */
    hd.cmd = OPEN;
    WriteHeader(buf, &hd);
    Append(buf, dbName, strlen(dbName) + 1);
    debug(buf);
    
    return OpenAllCloudNodes(buf);
}

int DBDelete(DataBase hdb)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    hd.cmd = CLOSE;
    WriteHeader(buf, &hd);

    return CloseAllCloudNodes(hdb, buf);
}

int DBSetKeyValue(DataBase hdb, dbKey key, dbValue value)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];
    Socket *psock = GetSocket(hdb, key);

    hd.cmd = SET;
    hd.key = key;
    WriteHeader(buf, &hd);
    Append(buf, value, strlen(value) + 1);

    SendMsg(*psock, buf);
    RecvMsg(*psock, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Set error:%s\n", GetAppend(phd));
        return -1;
    }

    return 0;
}

dbValue DBGetKeyValue(DataBase hdb, dbKey key)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];
    Socket *psock = GetSocket(hdb, key);

    hd.cmd = GET;
    hd.key = key;
    WriteHeader(buf, &hd);

    SendMsg(*psock, buf);
    RecvMsg(*psock, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Get error:%s\n", GetAppend(phd));
        return NULL;
    }

    return (dbValue)GetAppend(phd);
}

int DBDelKeyValue(DataBase hdb, dbKey key)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];
    Socket *psock = GetSocket(hdb, key);

    hd.cmd = DEL;
    hd.key = key;
    WriteHeader(buf, &hd);

    SendMsg(*psock, buf);
    RecvMsg(*psock, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Delete error:%s\n", GetAppend(phd));
        return -1;
    }

    return 0;
}

