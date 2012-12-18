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

// cannot hold a DataBase instance, to be correct...

void SendAndRev(char *buf)
{
    /* connect with server */
    SocketHandler *sh;
    sh = CreateSocketHandler(IP_ADDR, PORT);
    OpenRemoteService(sh);
    SendMsg(sh, buf);
    memset(buf, 0, MAXPACKETLEN);
    RecvMsg(sh, buf); 
    CloseRemoteService(sh);
}

DataBase DBCreate(char *dbName)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    /* write data using protocol */
    hd.cmd = OPEN;
    WriteHeader(buf, &hd);
    Append(buf, dbName, strlen(dbName) + 1);

    SendAndRev(buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd != OPEN_R)
    {
        fprintf(stderr, "Receive error.\n");
        return NULL;
    }
    debug(buf);

    return (DataBase)GetAppend(phd);
}

int DBDelete(DataBase hdb)
{
    DBPacketHeader hd;
    char buf[MAXPACKETLEN];

    debug(buf);
    hd.cmd = CLOSE;
    WriteHeader(buf, &hd);
    Append(buf, (char *)&hdb, sizeof(hdb));

    SendAndRev(buf);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd != CLOSE_R)
    {
        fprintf(stderr, "Close error.\n");
        return -1;
    }

    return 0;
}

int DBSetKeyValue(DataBase hdb, dbKey key, dbValue value)
{
return 0;
}

dbValue DBGetKeyValue(DataBase hdb, dbKey key)
{
return 0;
}

int DBDelKeyValue(DataBase hdb, dbKey key)
{
return 0;
}
