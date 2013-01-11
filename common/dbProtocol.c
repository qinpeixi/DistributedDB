/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  dbProtocol.c                               */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  DistributedDB                              */
/*  Module Name            :  Protocol                                   */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */
/*  Created Time           :  Sat 15 Dec 2012 10:38:48 AM CST            */
/*  Description            :  Implement the protocol.                    */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "dbProtocol.h"

void WriteHeader(char *buf, DBPacketHeader *header)
{
   DBPacketHeader *hBuf = (DBPacketHeader *)buf;
   hBuf->size = sizeof(DBPacketHeader);
   hBuf->cmd  = header->cmd;
   hBuf->key  = header->key;
   hBuf->version = header->version;
}

DBPacketHeader *GetHeader(char *buf)
{
    return (DBPacketHeader*)buf;
}

int Append(char *buf, const char *data, int len)
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
   
    
