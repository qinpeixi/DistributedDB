/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  client/dbclient.c                          */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Thu 10 Jan 2013 07:39:12 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "inputcmd.h"

extern char *master_addr;
extern int master_port;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Address and port is needed.\n");
        exit(-1);
    }
    master_addr = argv[1];
    master_port =  atoi(argv[2]);
    InputCmd();
    return 0;
}
