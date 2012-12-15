/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  dbclient.c                                 */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Wed 05 Dec 2012 03:21:17 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Database.h"

void ShowHelpPrompt()
{
    printf("open dbName     ----open a database named dbName.\n");
    printf("close           ----close the current database.\n");
    printf("quit            ----quit.\n");
    printf("set key value   ----insert a key/value tuple into the current database.\n");
    printf("get key         ----get the key/value tuple from the current database.\n");
    printf("del key         ----delete the key/value tuple from the current database.\n");
}

int main()
{
    DataBase hdb;
    dbKey key;
    dbValue value;
    char *strDBName = "";


    char delim[] = " ";
    char input[100];
    char *input2;
    char *strCmd;
    int res;
    int quit = 0;

    while (!quit)
    {
        printf("Nezha::%s>>", strDBName);
        memset(input, 0, sizeof(input));
        fgets(input, sizeof(input), stdin);
        input2 = strdup(input);
        input2[strlen(input2)-1] = '\0';

        strCmd = strsep(&input2, delim);
        if (strcasecmp(strCmd, "OPEN") == 0)
        {
            strDBName = strsep(&input2, delim);
            hdb = DBCreate(strDBName);
        }
        else if (strcasecmp(strCmd, "CLOSE") == 0)
        {
            DBDelete(hdb);
            strDBName = "";
        }
        else if (strcasecmp(strCmd, "QUIT") == 0)
        {
            quit = 1; 
        }
        else if (strcasecmp(strCmd, "SET") == 0)
        {
            key = atoi(strsep(&input2, delim));
            value = strsep(&input2, delim);
            res = DBSetKeyValue(hdb, key, value);
        }
        else if (strcasecmp(strCmd, "GET") == 0)
        {
            key = atoi(strsep(&input2, delim));
            value = DBGetKeyValue(hdb, key);
            if (value)
                printf("%d:%s\n", key, value);
        }
        else if (strcasecmp(strCmd, "DEL") == 0)
        {
            key = atoi(strsep(&input2, delim));
            res = DBDelKeyValue(hdb, key);
        }
        else if (strcasecmp(strCmd, "HELP") == 0)
        {
            ShowHelpPrompt();
        }
    }

    return 0;
}	
