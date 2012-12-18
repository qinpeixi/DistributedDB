/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  netdbapi.c                                 */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 15 Dec 2012 08:51:58 AM CST            */
/*  Description            :  Implementation of Database.h.              */
/*                         :  Connet to server using socketwrapper.h.    */ 
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "Database.h"
#include "dbProtocol.h"
#include "socketwrapper.h"

DataBase DBCreate(char *dbName)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    /* write data using protocol */
    hd.cmd = OPEN;
    WriteHeader(buf, &hd);
    Append(buf, dbName, strlen(dbName) + 1);

    /* establish a connection with server */
    SocketHandler *sh;
    sh = CreateSocketHandler(IP_ADDR, PORT);
    OpenRemoteService(sh);
    SendMsg(sh, buf);
    RecvMsg(sh, buf); 

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Receive error:%s\n", GetAppend(phd));
        return NULL;
    }

    return (DataBase)sh;
}

int DBDelete(DataBase hdb)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    hd.cmd = CLOSE;
    WriteHeader(buf, &hd);

    SendMsg((SocketHandler *)hdb, buf);
    RecvMsg((SocketHandler *)hdb, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Close error:%s\n", GetAppend(phd));
        return -1;
    }

    CloseRemoteService((SocketHandler *)hdb);

    return 0;
}

int DBSetKeyValue(DataBase hdb, dbKey key, dbValue value)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    hd.cmd = SET;
    hd.key = key;
    WriteHeader(buf, &hd);
    Append(buf, value, strlen(value) + 1);

    SendMsg((SocketHandler *)hdb, buf);
    RecvMsg((SocketHandler *)hdb, buf);

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

    hd.cmd = GET;
    hd.key = key;
    WriteHeader(buf, &hd);

    SendMsg((SocketHandler *)hdb, buf);
    RecvMsg((SocketHandler *)hdb, buf);

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

    hd.cmd = DEL;
    hd.key = key;
    WriteHeader(buf, &hd);

    SendMsg((SocketHandler *)hdb, buf);
    RecvMsg((SocketHandler *)hdb, buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd == CMDFAIL)
    {
        fprintf(stderr, "Delete error:%s\n", GetAppend(phd));
        return -1;
    }

    return 0;
}

