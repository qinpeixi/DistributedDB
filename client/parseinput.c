/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  parseinput.c                               */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  DistributedDB                              */
/*  Module Name            :  Parseinput                                 */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */
/*  Created Time           :  Sun 16 Dec 2012 09:38:40 AM CST            */
/*  Description            :  Parse user input then call dbapi           */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <assert.h>
#include "parseinput.h"
#include "../common/Database.h"

DataBase hdb = NULL;

int Check(const char *pat, const char *str);
int GetMatch(const char *pat, char *str, char *match);
void HelpPrompt();

int Parse(char *input)
{
    char *match = (char*)malloc(MAX_INPUT_LEN);
    char *s = strdup(input);
    dbKey key;
    dbValue value;
    int res = -1000; // no-meaning value

    if (0 == Check("^\\s*\\w{3,5}\\s+[0-9]+\\s+.+$", input))
    {// two parameters commands
        GetMatch("\\b\\w{3,5}\\b", s, match);
        if (0 == strcasecmp(match, "set"))
        {
            if (hdb == NULL)
                goto NODB;
            GetMatch("\\b[0-9]+\\b", s, match);
            key = atoi(match);
            if (0 != GetMatch("\\b.+", s, match))
            {// dbValue should not  started with &*#$^!@... 
                fprintf(stderr, "Invaild value.\n");
                res = -1;
                goto END;
            }
            value = match;
            res = DBSetKeyValue(hdb, key, value);
        }
        else
        {
            fprintf(stderr, "Wrong command with two parameters.\n");
            res = -1;
        }
    }
    else if (0 == Check("^\\s*\\w{3,5}\\s+\\w+(\\.\\w+)?\\s*$", input))
    {// one parameter commands
        GetMatch("\\b\\w{3,5}\\b", s, match);
        if (0 == strcasecmp(match, "open"))
        {
            extern char *DBName;
            if (hdb != NULL)
            {
                fprintf(stderr, "Please close current database first.\n");
                res = -1;
                goto END;
            }
            GetMatch("\\b\\w+\\.\\w+\\b", s, match);
            strncpy(DBName, match, MAX_INPUT_LEN);
            hdb = DBCreate(DBName);
            res = hdb ? 0 : -1;
        }
        else if (0 == strcasecmp(match, "get"))
        {
            if (hdb == NULL)
                goto NODB;
            if (0 != GetMatch("\\b[0-9]+\\b", s, match))
            {
                fprintf(stderr, "Key should be numbers.\n");
                res = -1;
                goto END;
            }
            key = atoi(match);
            value = DBGetKeyValue(hdb, key);
            if (value != NULL)
            {
                printf("%s\n", value);
                res = 0;
            }
            else
                res = -1;
        }
        else if (0 == strcasecmp(match, "del")
                || 0 == strcasecmp(match, "delete"))
        {
            if (hdb == NULL)
                goto NODB;
            if (0 != GetMatch("\\b[0-9]+\\b", s, match))
            {
                fprintf(stderr, "Key should be numbers.\n");
                res = -1;
                goto END;
            }
            key = atoi(match);
            res = DBDelKeyValue(hdb, key);
        }
        else
        {
            fprintf(stderr, "Wrong command with one paramaters.\n");
            res = -1;
        }
    }
    else if (0 == Check("^\\s*\\w{3,5}\\s*$", input))
    {// none parameter commands
        GetMatch("\\b\\w{3,5}\\b", s, match);
        if (0 == strcasecmp(match, "close"))
        {
            if (hdb == NULL)
                goto NODB;
            extern char *DBName;
            res = DBDelete(hdb);
            hdb = NULL;
            DBName[0] = '\0';
        }
        else if (0 == strcasecmp(match, "exit") 
                || 0 == strcasecmp(match, "quit"))
        {
            if (hdb != NULL)
            {
                fprintf(stderr, "Please close database first.\n");
                res = -1;
                goto END;
            }
            res = 1;
        }
        else if (0 == strcasecmp(match, "help"))
        {
            HelpPrompt();
            res = 0;
        }
        else
        {
            fprintf(stderr, "Wrong command with none parameter.\n");
            res = -1;
        }
    }
    else
    {
        fprintf(stderr, "Incorrect format.\n");
        res = -1;
    }

    goto END;

NODB:
    fprintf(stderr, "Please open a database first.\n");
    res = -1;

END:
    free(s);
    assert(res != -1000);
    return res;
}

int RegMatch(const char *pat, const char *str, int *start, int *end)
{
    int res;
    regex_t reg;
    regmatch_t pm[10];
    const size_t nmatch = 10;
    res = regcomp(&reg, pat, REG_EXTENDED | REG_ICASE);
    if (res != 0)
    {
        printf("Invalid pattern.\n");
        return -1;
    }
    res = regexec(&reg, str, nmatch, pm, 0);
    if (res == REG_NOMATCH)
        return -1;
    else if (res != 0)
    {
        printf("Execution error.\n");
        return -1;
    }
    else
    {
        *start = pm[0].rm_so;
        *end   = pm[0].rm_eo;
        return 0;
    }
}

int Check(const char *pat, const char *str)
{
    int s, e;
    return RegMatch(pat, str, &s, &e);
}

int GetMatch(const char *pat, char *str, char *match)
{
    int s, e;
    if (0 != RegMatch(pat, str, &s, &e))
        return -1;

    strncpy(match, str + s, e-s);
    match[e-s] = '\0';

    int len = strlen(str);
    memmove(str, str + e, len + 1 - e);

    return 0;
}

void HelpPrompt()
{
    printf("open dbName     ----open a database named dbName.\n");
    printf("close           ----close the current database.\n");
    printf("quit/exit       ----quit.\n");
    printf("set key value   ----insert a key/value tuple into the current database.\n");
    printf("get key         ----get the key/value tuple from the current database.\n");
    printf("del(ete) key    ----delete the key/value tuple from the current database.\n");
}
