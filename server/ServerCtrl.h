/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/ServerCtrl.h                        */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sun 06 Jan 2013 03:32:24 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef SERVERCTRL_H
#define SERVERCTRL_H 

#include <stdio.h>

#define MAX_SLAVE_NUM 32
#define MAX_KEY       1000

typedef struct
{
    int ip;
    int port; // can not known yet
    int sock;
    int key; // The key that the slave uses.
} SlaveNode;

typedef struct
{
    unsigned version;
    int num; // The total number of slaves.
    SlaveNode nodes[MAX_SLAVE_NUM];
} SlaveList;

void printslaves(SlaveList sl)
{
    int i;
    printf("The slaves are:\n");
    printf("version: %u\n", sl.version);
    for (i=0; i<sl.num; i++)
    {
        printf("ip: %d port: %d key: %d\n", sl.nodes[i].ip, sl.nodes[i].port, 
                sl.nodes[i].key);
    }
}
#endif
