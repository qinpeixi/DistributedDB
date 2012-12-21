/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  dbProtocol.h                               */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  DistributedDB                              */
/*  Module Name            :  Protocol                                   */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */
/*  Created Time           :  Sat 15 Dec 2012 09:10:29 AM CST            */
/*  Description            :  Protocol apis.                             */
/*************************************************************************/

#ifndef DBPROTOCOL_H
#define DBPROTOCOL_H

/*
 * Data format
 * 0------------------------------------31
 * |   the total size of this packet    |
 * --------------------------------------
 * |           the command              |
 * --------------------------------------
 * |           the key                  |
 * --------------------------------------
 * |                                    |
 * /   appended content(
 */

#define MAXPACKETLEN 1024
#define APPENDSIZE (MAXPACKETLEN - sizeof(DBPacketHeader))
enum CMD {SET,   GET,   DEL,   OPEN,   CLOSE,   QUIT,  HELP,
          SET_R, GET_R, DEL_R, OPEN_R, CLOSE_R, QUIT_R, CMDFAIL};

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

#define debug(buf) printf("%s:%d %d %d %d %s\n", __FILE__, __LINE__,  \
        ((DBPacketHeader *)buf)->size, \
        ((DBPacketHeader *)buf)->cmd, ((DBPacketHeader *)buf)->key, \
        GetAppend((DBPacketHeader *)buf));

#endif
