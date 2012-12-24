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
 * /         appended content           /
 * /  (dbName /  dbValue / Error Msg)   /
 * |                                    |
 * --------------------------------------
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

/*
 * Write the header content into buf
 * input	: buf, header
 * output	: None
 * in/out	: None
 * return	: None
 */
void WriteHeader(char *buf, DBPacketHeader *header);

/*
 * Get the header from buf
 * input	: buf
 * output	: None
 * in/out	: None
 * return	: the header of the packet buf
 */
DBPacketHeader *GetHeader(char *buf);

/*
 * Write append content into buf
 * input	: buf, data, len
 * output	: None
 * in/out	: None
 * return	: SUCCESS(0)/exit(-1)
 */
int Append(char *buf, char *data, int len);

/*
 * Get the appended content from buf
 * input	: hdb
 * output	: None
 * in/out	: None
 * return	: SUCCESS(0)/exit(-1)
 */
char *GetAppend(DBPacketHeader *phd);

#ifdef debug
#define debug(buf) printf("%s:%d %d %d %d %s\n", __FILE__, __LINE__,  \
        ((DBPacketHeader *)buf)->size, \
        ((DBPacketHeader *)buf)->cmd, ((DBPacketHeader *)buf)->key, \
        GetAppend((DBPacketHeader *)buf));
#else
#define debug(buf)
#endif //debuf

#endif //DBPROTOCOL_H