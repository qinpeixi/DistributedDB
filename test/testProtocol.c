/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  testProtocol.c                             */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 22 Dec 2012 08:25:23 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../common/dbProtocol.h"

int main()
{
    DBPacketHeader hd;
    DBPacketHeader *phd;
    char buf[MAXPACKETLEN];
    char value[128] = "Hello World!";

    hd.cmd = SET;
    hd.key = 100;
    WriteHeader(buf, &hd);
    Append(buf, value, strlen(value)+1);
    phd = (DBPacketHeader *)buf;
    assert(phd->cmd == hd.cmd);
    assert(phd->key == hd.key);
    assert(strcmp(GetAppend(phd), value) == 0);
    printf("Test of dbProtocol.c          : PASS\n");
    return 0;
}

