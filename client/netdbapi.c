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

DataBase DBCreate(char *dbName)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    /* write data using protocol */
    hd.cmd = OPEN;
    WriteHeader(buf, &hd);
    Append(buf, dbName, strlen(dbName) + 1);
    debug(buf);

    /* establish a connection with server */
    Socket *psock = malloc(sizeof(Socket));
    if (-1 == OpenRemoteService(psock, NULL))
    {
        fprintf(stderr, "Open remote service failed.\n");
        return NULL;
    }
    /* send server the command */
    SendMsg(*psock, buf);
    RecvMsg(*psock, buf); 

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Receive error:%s\n", GetAppend(phd));
        return NULL;
    }

    return (DataBase)psock;
}

int DBDelete(DataBase hdb)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];
    Socket *psock = hdb;

    hd.cmd = CLOSE;
    WriteHeader(buf, &hd);

    SendMsg(*psock, buf);
    RecvMsg(*psock, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Close error:%s\n", GetAppend(phd));
        return -1;
    }

    CloseRemoteService(*psock);
    free(psock);

    return 0;
}

int DBSetKeyValue(DataBase hdb, dbKey key, dbValue value)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];
    Socket *psock = hdb;

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
    Socket *psock = hdb;

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
    Socket *psock = hdb;

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

