/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  socketwrapper.h                            */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 15 Dec 2012 04:51:06 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include<stdio.h>
#include <arpa/inet.h>

#define PORT                5001
#define IP_ADDR             "127.0.0.1"
#define MAX_BUF_LEN         1024
#define MAX_CONNECT_QUEUE   5

typedef struct 
{
    int sockfd;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    int newfd;
} SocketHandler;

SocketHandler* CreateSocketHandler(char *addr, int port);

int InitializeService(SocketHandler *sh);

void ShutdownService(SocketHandler *sh);

int ServiceStart(SocketHandler *sh);

void ServiceStop(SocketHandler *sh);


int OpenRemoteService(SocketHandler *sh);

void CloseRemoteService(SocketHandler *sh);

void RecvMsg(SocketHandler *sh, char *buf);

void SendMsg(SocketHandler *sh, char *buf);

char *GetClientIP(SocketHandler *sh);

int GetClientPort(SocketHandler *sh);
