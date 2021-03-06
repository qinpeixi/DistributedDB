/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  InputCmd.c                                 */
/*  Pricipal Author        :  Qinpxi                                     */
/*  Subsystem Name         :  DistributedDB                              */
/*  Module Name            :  InputCmd                                   */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */
/*  Created Time           :  Sun 16 Dec 2012 09:31:50 AM CST            */
/*  Description            :  Get input command of db operation          */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parseinput.h"
#include "inputcmd.h"

char *DBName;

int InputCmd()
{
    int res = 0;
    int len;
    char *input;
    input = (char *)malloc(MAX_INPUT_LEN);
    DBName = (char *)malloc(MAX_INPUT_LEN);

    while (res != 1)
    {
        printf("Nezha::%s>>", DBName);
        input = fgets(input, MAX_INPUT_LEN, stdin);
        len = strlen(input);
        if (len == 0 || input[0] == '\n')
            continue;
        else if (input[len-1] == '\n')
            input[len-1] = '\0';
        res = Parse(input);
    }

    return 0;
}

