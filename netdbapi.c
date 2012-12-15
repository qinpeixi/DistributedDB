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
/*  Description            :                                             */
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

    hd.cmd = OPEN;
    WriteHeader(buf, &hd);
    
    int len = strlen(dbName) + 1;
    Append(buf, dbName, len);
    debug(buf);

    SocketHandler *sh;
    sh = CreateSocketHandler(IP_ADDR, PORT);
    OpenRemoteService(sh);
    SendMsg(sh, buf);
    RecvMsg(sh, buf); 
    CloseRemoteService(sh);

    DBPacketHeader *phd = GetHeader(buf);
    if (phd->cmd != OPEN_R)
    {
        printf("Receive error.\n");
        return NULL;
    }

    return (DataBase)GetAppend(phd);
}

int DBDelete(DataBase hdb)
{
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
