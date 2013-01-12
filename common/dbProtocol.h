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
enum CMD {
    // DB operations
    SET,   GET,   DEL,   OPEN,   CLOSE,   QUIT,  HELP,
    SET_R, GET_R, DEL_R, OPEN_R, CLOSE_R, QUIT_R, CMDFAIL,

    // Commands used for server controlling
    //
    // client to master
    GET_LIST,   GET_LIST_R, 
    // slave to master
    ADD_SLAVE,   DEL_SLAVE,   
    ADD_SLAVE_R, DEL_SLAVE_R, 
    HEARTBEAT,    
    // master to slave
    NEW_SLAVE,   RM_SLAVE,    UPDATE_BACKUP12,
    NEW_SLAVE_R, RM_SLAVE_R,  UPDATE_BACKUP12_R,
    // slave to slave
    CLIP_DATA,   BACKUP,    FILETRANS,
    CLIP_DATA_R, BACKUP_R,  FILETRANS_R,
    UPDATE_BACKUP2,
    UPDATE_BACKUP2_R,
    // slave to client
    UPDATE_VERSION
};

typedef struct 
{
    int size;
    enum CMD cmd;
    int key;
    int version;
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
int Append(char *buf, const char *data, int len);

/*
 * Get the appended content from buf
 * input	: hdb
 * output	: None
 * in/out	: None
 * return	: SUCCESS(0)/exit(-1)
 */
char *GetAppend(DBPacketHeader *phd);

#define Debu
#ifdef Debug
#define debug(buf) printf("%s:%d %d %d %d %s\n", __FILE__, __LINE__,  \
        ((DBPacketHeader *)buf)->size, \
        ((DBPacketHeader *)buf)->cmd, ((DBPacketHeader *)buf)->key, \
        GetAppend((DBPacketHeader *)buf));
#else
#define debug(buf)
#endif //debuf

#endif //DBPROTOCOL_H
