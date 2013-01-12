/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/printslave.c                        */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 12 Jan 2013 11:00:48 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ServerCtrl.h"

void printslaves(SlaveList sl)
{
    int i;
    printf("The slaves are:\n");
    printf("version: %u\n", sl.version);
    for (i=0; i<sl.num; i++)
    {
        printf("ip: %s port: %d key: %d\n", 
                inet_ntoa(*(struct in_addr *)&(sl.nodes[i].ip)), 
                sl.nodes[i].port, sl.nodes[i].key);
    }
}
