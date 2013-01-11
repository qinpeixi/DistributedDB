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
#include <assert.h>
#include "../common/Database.h"
#include "../common/dbProtocol.h"
#include "../common/Socket.h"
#include "../server/ServerCtrl.h"

#define NODES_NUM slaves->num

/*typedef struct 
{
    char addr[MAX_STRADDR_LEN];
    int port;
    int sock;
} CloudNode;*/
typedef SlaveList CloudNode;

int master_sock;
char *master_addr;
int master_port;
char DBName[1024];

DBPacketHeader* ExchangePacket(enum CMD cmd, int key, 
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
}

int ReadCloudNodes(SlaveList *slaves)
{
    printf("Read cloud nodes from %s:%d\n", master_addr, master_port);
    if (-1 == OpenRemoteService(&master_sock,  master_addr,  master_port))
        return -1;

    DBPacketHeader *phd = ExchangePacket(GET_LIST, 0, NULL, 0, master_sock);
    assert(phd->cmd == GET_LIST_R);
    *slaves = *(SlaveList *)GetAppend(phd);
    printslaves(*slaves);
    return 0;
}

DataBase OpenAllCloudNodes(char *buf)
{
    /* establish a connection with server */
    //CloudNode *slaves->nodes = malloc(NODES_NUM * sizeof(CloudNode));
    //CloudNode *slaves->nodes = slaves;
    //ReadCloudNodes(slaves->nodes);
    SlaveList *slaves = malloc(sizeof(SlaveList));
    ReadCloudNodes(slaves);
    int i;
    char recvBuf[MAXPACKETLEN];
    for (i=0; i<NODES_NUM; i++)
    {
        if (-1 == OpenRemoteService2(&(slaves->nodes[i].sock), slaves->nodes[i].ip, slaves->nodes[i].port))
        {
            fprintf(stderr, "Open remote service failed.\n");
            return NULL;
        }
        /* send server the command */
        SendMsg(slaves->nodes[i].sock, buf);
        RecvMsg(slaves->nodes[i].sock, recvBuf); 

        DBPacketHeader *phd = GetHeader(recvBuf);
        if (phd->cmd != OPEN_R)
        {
            fprintf(stderr, "Receive error:%s\n", GetAppend(phd));
            return NULL;
        }
    }

    return (DataBase)slaves;
}

int CloseAllCloudNodes(DataBase hdb, char *buf)
{
    CloudNode *slaves= (CloudNode *)hdb;
    int i;
    char recvBuf[MAXPACKETLEN];
    for (i=0; i<NODES_NUM; i++)
    {
        SendMsg(slaves->nodes[i].sock, buf);
        RecvMsg(slaves->nodes[i].sock, recvBuf);

        DBPacketHeader *phd = GetHeader(recvBuf);
        if (phd->cmd == CMDFAIL)
        {
            fprintf(stderr, "Close error:%s\n", GetAppend(phd));
            return -1;
        }

        CloseRemoteService(slaves->nodes[i].sock);
    }
    free(slaves);

    return 0;
}

/* Get socket by key according to cloud strategy */
int *GetSocket(DataBase hdb, dbKey key)
{
    CloudNode *slaves = (CloudNode *)hdb;
    int index = key % NODES_NUM;
    //printf("Send to %d:%d\n", index, slaves->nodes[index].port);
    return &(slaves->nodes[index].sock);
}

DataBase DBCreate(char *dbName)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    strcpy(DBName, dbName);
    /* write data using protocol */
    hd.cmd = OPEN;
    hd.version = 0;
    WriteHeader(buf, &hd);
    Append(buf, dbName, strlen(dbName) + 1);
    debug(buf);
    
    return OpenAllCloudNodes(buf);
}

int DBDelete(DataBase hdb)
{
    DBPacketHeader hd;
    CloudNode *slaves= (CloudNode *)hdb;
    char buf[MAXPACKETLEN];

    hd.cmd = CLOSE;
    hd.version = slaves->version;
    WriteHeader(buf, &hd);

    return CloseAllCloudNodes(hdb, buf);
}

int DBSetKeyValue(DataBase hdb, dbKey key, dbValue value)
{
    DBPacketHeader hd;
    CloudNode *slaves= (CloudNode *)hdb;
    char buf[MAXPACKETLEN];
    int *psock;
REDO:
    psock = GetSocket(hdb, key);

    hd.cmd = SET;
    hd.key = key;
    hd.version = slaves->version;
    WriteHeader(buf, &hd);
    Append(buf, value, strlen(value) + 1);

    SendMsg(*psock, buf);
    RecvMsg(*psock, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == UPDATE_VERSION)
    {
        DBDelete(hdb);
        *slaves = *(SlaveList *)GetAppend(phd);
        printslaves(*slaves);
        DBCreate(DBName);
        goto REDO;
    }
    else if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Set error:%s\n", GetAppend(phd));
        return -1;
    }

    return 0;
}

dbValue DBGetKeyValue(DataBase hdb, dbKey key)
{
    DBPacketHeader hd;
    CloudNode *slaves= (CloudNode *)hdb;
    char buf[MAXPACKETLEN];
    int *psock;
REDO:
    psock = GetSocket(hdb, key);

    hd.cmd = GET;
    hd.key = key;
    hd.version = slaves->version;
    WriteHeader(buf, &hd);

    SendMsg(*psock, buf);
    RecvMsg(*psock, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == UPDATE_VERSION)
    {
        DBDelete(hdb);
        *slaves = *(SlaveList *)GetAppend(phd);
        printslaves(*slaves);
        DBCreate(DBName);
        goto REDO;
    }
    else if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Get error:%s\n", GetAppend(phd));
        return NULL;
    }

    return (dbValue)GetAppend(phd);
}

int DBDelKeyValue(DataBase hdb, dbKey key)
{
    DBPacketHeader hd;
    CloudNode *slaves= (CloudNode *)hdb;
    char buf[MAXPACKETLEN];
    int *psock;
REDO:
    psock = GetSocket(hdb, key);

    hd.cmd = DEL;
    hd.key = key;
    hd.version = slaves->version;
    WriteHeader(buf, &hd);

    SendMsg(*psock, buf);
    RecvMsg(*psock, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == UPDATE_VERSION)
    {
        DBDelete(hdb);
        *slaves = *(SlaveList *)GetAppend(phd);
        printslaves(*slaves);
        DBCreate(DBName);
        goto REDO;
    }
    else if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Delete error:%s\n", GetAppend(phd));
        return -1;
    }

    return 0;
}

