/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/slave.h                             */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Thu 10 Jan 2013 08:12:51 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef SLAVE_H
#define SLAVE_H

extern int listen_sock;
extern int master_sock;

int InitialSlave(char *master_addr, int master_port);

void HandleSlaveRequest(int accept_sock, char *szBuf);

void HandleMasterRequest();

void ShutDownSlave(int a);

unsigned GetVersion();

void SendSlaveList(int sockfd);

#endif

