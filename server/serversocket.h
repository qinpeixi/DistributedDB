/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/serversocket.h                      */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sun 23 Dec 2012 03:18:08 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef SERVERSOCKET_H 
#define SERVERSOCKET_H 

#include "../common/Socket.h"


typedef struct 
{
    int sock;
    //char addr[MAX_STRADDR_LEN];
    int ip;
    void *app; // to take sth with socket
} ClientSockHandle;

int DBInitializeService(int *sockfd, char *addr, int port);

void DBShutdownService(int sockfd);

ClientSockHandle DBServiceStart(int sockfd);

void DBServiceStop(int sockfd);

void DBSendMsg(ClientSockHandle hcsock, char *buf);

ClientSockHandle *DBRecvMsg(int sockfd, char *buf);
#endif
