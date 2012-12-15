/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  dbProtocol.h                               */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 15 Dec 2012 09:10:29 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef DBPROTOCOL_H
#define DBPROTOCOL_H
#include<stdio.h>

#define MAXPACKETLEN 1024
#define APPENDSIZE (MAXPACKETLEN - sizeof(DBPacketHeader))
enum CMD {SET,   GET,   DEL,   OPEN,   CLOSE,   QUIT,  HELP,
          SET_R, GET_R, DEL_R, OPEN_R, CLOSE_R, QUIT_R};

typedef struct 
{
    int size;
    enum CMD cmd;
    int key;
} DBPacketHeader;

int WriteHeader(char *buf, DBPacketHeader *header);
DBPacketHeader *GetHeader(char *buf);

int Append(char *buf, char *data, int len);
char *GetAppend(DBPacketHeader *phd);

#define debug(buf) printf("%s: %d %d %d %s\n", __FILE__, ((DBPacketHeader *)buf)->size, \
        ((DBPacketHeader *)buf)->cmd, ((DBPacketHeader *)buf)->key, \
        GetAppend((DBPacketHeader *)buf));

#endif
