/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  common/int.h                            */
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
//#define ADDR_STR_LEN        128
#define MAX_BUF_LEN         1024

#define MAX_CONNECT_QUEUE   5
#define MAX_STRADDR_LEN     50

int InitializeService(int *listen_sock, char *addr, int port);

void ShutdownService(int listen_sock);

int ServiceStart(int listen_sock, int *accept_sock, int *ip);

void ServiceStop(int sockfd);

int OpenRemoteService(int *sockfd, char *addr, int port);

void CloseRemoteService(int sockfd);

void SendMsg(int sockfd, char *buf);

void RecvMsg(int sockfd, char *buf);

#endif
