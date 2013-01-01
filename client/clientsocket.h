/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  client/clientsocket.h                      */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Mon 24 Dec 2012 08:47:23 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "../common/Socket.h"

int OpenRemoteService(Socket *sockfd, char *addr, int port);

void CloseRemoteService(Socket sockfd);

void SendMsg(Socket sockfd, char *buf);

void RecvMsg(Socket sockfd, char *buf);
#endif


