/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  testDB.c                                   */
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

enum CMD {SET, GET, DEL, OPEN, CLOSE, QUIT, HELP};

/*
 * Get tokens from the input 
 * input      : input
 * output     : cmd, key, value
 * in/out     : None
 * return     : if SUCCESS return SUCCESS
 *            : if FAILURE return FAILURE
 */
int GetTokens(char *input, enum CMD *cmd, dbKey *key, dbValue *value)
{
    char *strCmd;
    char *strKey;
    char *val;

    char *input2, *s;
    char delim[] = " ";
    char *p;
    int res;

    /* initial the output */
    *value = NULL;

    /* seperate input into parameters */ 
    input2 = strdup(input);
    s      = input2;
    strCmd = strsep(&s, delim);
    strKey = strsep(&s, delim);
    val    = strsep(&s, delim);

    /* check the command */ 
    if (strcmp(strCmd, "set") == 0)
        *cmd = SET;
    else if (strcmp(strCmd, "get") == 0)
        *cmd = GET;
    else if (strcmp(strCmd, "quit") == 0)
    {
        *cmd = QUIT;
        res = SUCCESS;
        goto END;
    }
    else
    {
        fprintf(stderr, "Invalid command.\n");
        res = FAILURE;
        goto END;
    }

    /* check the key */
    res = -1;
    for (p=strKey; p && *p != '\0'; p++)
    {
        if (!isdigit(*p))
        {
            res = -1;
            break;
        }
    }
    if (!strKey || res == -1)
    {
        fprintf(stderr, "Invalid key.\n");
        res = FAILURE;
        goto END;
    }
    else if (strKey)
    {
        *key = atoi(strKey);
    }

    /* check the value */
    if (val)
    {
        if (strlen(val) >= MAXVALUELEN)
        {
            fprintf(stderr, "Invalid value.\n");
            res = FAILURE;
            goto END;
        }
        else if (strlen(val) == 0)
            *value = strdup(s);
        else
            *value = strdup(val);
    }
    else
    {
        if (*cmd == SET)
        {
            fprintf(stderr, "Value needed.\n");
            res = FAILURE;
            goto END;
        }
    }

    res = SUCCESS;

END:
    free(input2);
    return res;
}

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
    enum CMD cmd;
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
			cmd = OPEN;
		else if (strcasecmp(strCmd, "CLOSE") == 0)
			cmd = CLOSE;
		else if (strcasecmp(strCmd, "QUIT") == 0)
			cmd = QUIT;
		else if (strcasecmp(strCmd, "SET") == 0)
			cmd = SET;
		else if (strcasecmp(strCmd, "GET") == 0)
			cmd = GET;
		else if (strcasecmp(strCmd, "DEL") == 0)
			cmd = DEL;
        else if (strcasecmp(strCmd, "HELP") == 0)
            cmd = HELP;
        /*res = GetTokens(input, &cmd, &key, &value);
        if (res == FAILURE)
        {
            printf("Invaild input!\n");
            continue;
        }
		*/

        switch (cmd)
        {
			case OPEN:
				strDBName = strsep(&input2, delim);
				hdb = DBCreate(strDBName);
				break;
            case QUIT: 
                quit = 1; 
                break;
            case SET: 
				key = atoi(strsep(&input2, delim));
				value = strsep(&input2, delim);
                res = DBSetKeyValue(hdb, key, value);
                break;
            case GET:
                key = atoi(strsep(&input2, delim));
                value = DBGetKeyValue(hdb, key);
                if (value)
                    printf("%d:%s\n", key, value);
                break;
			case DEL:
                key = atoi(strsep(&input2, delim));
                res = DBDelKeyValue(hdb, key);
                break;
			case CLOSE:
				DBDelete(hdb);
				strDBName = "";
				break;
            case HELP:
                ShowHelpPrompt();
                break;
        }

        //free (value);
    }

    //res = DBDelete(hdb);

    return 0;
}	
