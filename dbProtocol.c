/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  dbProtocol.c                               */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 15 Dec 2012 10:38:48 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "dbProtocol.h"

int WriteHeader(char *buf, DBPacketHeader *header)
{
   DBPacketHeader *hBuf = (DBPacketHeader *)buf;
   hBuf->size = sizeof(DBPacketHeader);
   hBuf->cmd  = header->cmd;
   hBuf->key  = header->key;

   return 0;
}

DBPacketHeader *GetHeader(char *buf)
{
    return (DBPacketHeader*)buf;
}

int Append(char *buf, char *data, int len)
{
    DBPacketHeader *hBuf = (DBPacketHeader *)buf;
    if (hBuf->size > sizeof(DBPacketHeader))
        return -1;
    if (len > APPENDSIZE)
        return -1;
    
    memcpy((void *)(buf + hBuf->size), (void *)data, len);
    hBuf->size += len;

    return 0;
}

char *GetAppend(DBPacketHeader *phd)
{
    return (char *)(phd + 1);
}
   
    
