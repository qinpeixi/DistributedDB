/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  common/Socket.h                            */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Mon 24 Dec 2012 08:45:03 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef SOCKET_H
#define SOCKET_H

#define PORT                5001
#define LOCAL_ADDR          "127.0.0.1"
#define MAX_BUF_LEN         1024

typedef int Socket;

void SendMsg(Socket sockfd, char *buf);

void RecvMsg(Socket sockfd, char *buf);

#endif
