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

#define MAX_CONNECT_QUEUE   5
#define MAX_STRADDR_LEN     50

typedef struct 
{
    Socket sock;
    char addr[MAX_STRADDR_LEN];
} ClientSockHandle;

int InitializeService(Socket *sockfd, char *addr);

void ShutdownService(Socket sockfd);

ClientSockHandle ServiceStart(Socket sockfd);

void ServiceStop(ClientSockHandle hcsock);

#endif