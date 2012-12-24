/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  DBAPI.c                                    */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  Database                                   */
/*  Module Name            :  Implementation                             */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */ 
/*  Created Time           :  2012年11月29日 星期四 21时07分27秒         */
/*  Description            :  Implement DBAPI.h using Tokyo Cabinet      */ 
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <tcutil.h>
#include <tchdb.h>
#include <limits.h>
#include <assert.h>
#include "../common/Database.h"

#define debug(...) ;

typedef struct
{
    char *name;
    TCHDB *hdb;
    int counter; // should be lock when multi-thread visite
} OpenedDB;

void SetLastErrorMsg(const char *);
OpenedDB *GetOpenedDB(char *name);
OpenedDB *AddOpenedDB(char *name, TCHDB *hdb);
int DelOpenedDB(OpenedDB *podb);

DataBase DBCreate(char *dbName)
{
    OpenedDB *podb;
    int ecode;

    podb = GetOpenedDB(dbName);

    if (podb == NULL)
    {
        TCHDB *hdb;

        /* create the database */
        hdb = tchdbnew();

        /* open the database */
        if(!tchdbopen(hdb, dbName, HDBOWRITER | HDBOCREAT))
        {
            ecode = tchdbecode(hdb);
            fprintf(stderr, "open db error: %s\n", tchdberrmsg(ecode));
            SetLastErrorMsg(tchdberrmsg(ecode));
            return NULL;
        }
        debug("open a db:%s\n", dbName);

        podb = AddOpenedDB(dbName, hdb);
    }

    return (DataBase)podb;
}

int DBDelete(DataBase db)
{
    OpenedDB *podb = (OpenedDB *)db;
    int ecode;

    if (DelOpenedDB(podb) == 0)
        return SUCCESS;

    /* close the database */
    if(!tchdbclose(podb->hdb))
    {
        ecode = tchdbecode(podb->hdb);
        fprintf(stderr, "close db error: %s\n", tchdberrmsg(ecode));
        SetLastErrorMsg(tchdberrmsg(ecode));
        return FAILURE;
    }
    debug("close a db:%s\n", podb->name);

    return SUCCESS;
}

int DBSetKeyValue(DataBase db, dbKey key, dbValue value)
{
    OpenedDB *podb = (OpenedDB *)db;
    int ecode;

    assert(podb->hdb != NULL);
    /* store key-value into the database */
    if (!tchdbput(podb->hdb, &key, sizeof(dbKey), value, strlen(value)+1)) 
    {
        ecode = tchdbecode(podb->hdb);
        fprintf(stderr, "put key-value error: %s\n", tchdberrmsg(ecode));
        SetLastErrorMsg(tchdberrmsg(ecode));

        return FAILURE;
    }
    debug("set %d:%s\n", key, value);

    return SUCCESS;
}

dbValue DBGetKeyValue(DataBase db, dbKey key)
{
    OpenedDB *podb = (OpenedDB *)db;
    int ecode;
    dbValue value;
    int valueLen = -1;

    /* get the value */
    value = (dbValue)tchdbget(podb->hdb, &key, sizeof(dbKey), &valueLen);
    if (value)
    {
        debug("get %d:%s\n", key, value);
        return value;
    }
    else
    {
        ecode = tchdbecode(podb->hdb);
        fprintf(stderr, "get value error: %s\n", tchdberrmsg(ecode));
        SetLastErrorMsg(tchdberrmsg(ecode));
        return NULL;
    }
}

int DBDelKeyValue(DataBase db, dbKey key)
{
    OpenedDB *podb = (OpenedDB *)db;
    int ecode;

    if (!tchdbout(podb->hdb, &key, sizeof(dbKey)))
    {
        ecode = tchdbecode(podb->hdb);
        fprintf(stderr, "delete key-value error: %s\n", tchdberrmsg(ecode));
        SetLastErrorMsg(tchdberrmsg(ecode));

        return FAILURE;
    }

    return SUCCESS;
}

static char lasterr[1024];
void SetLastErrorMsg(const char *msg)
{
    strncpy(lasterr, msg, 1024);
}
char *DBGetLastErrorMsg()
{
    return lasterr;
}

TCMDB *mdb = NULL;  // The db to save all of the opened db

/* if the db specified by name is not open, return NULL */
OpenedDB *GetOpenedDB(char *name)
{
    if (mdb == NULL)
    {
        mdb = tcmdbnew();
        return NULL;
    }

    OpenedDB *podb;
    int valuelen = -1;
    podb = (OpenedDB *)tcmdbget(mdb, name, strlen(name), &valuelen);
    if (podb != NULL)
    {
        podb->counter ++;
        tcmdbput(mdb, name, strlen(name), podb, sizeof(OpenedDB));
        debug("%s's counter is %d\n", podb->name, podb->counter);
    }

    return podb;
}

OpenedDB *AddOpenedDB(char *name, TCHDB *hdb)
{
    assert(mdb != NULL);
    OpenedDB *podb = malloc(sizeof(OpenedDB));
    podb->name = strdup(name);
    podb->hdb = hdb;
    podb->counter = 1;
    tcmdbput(mdb, name, strlen(name), podb, sizeof(OpenedDB));

    return podb;
}

/*
 * if db is still used by others, return 0
 * if no other user use this db, return 1
 */
int DelOpenedDB(OpenedDB *podb)
{
    int valuelen = -1;
    podb = (OpenedDB *)tcmdbget(mdb, podb->name, strlen(podb->name), &valuelen);
    podb->counter --;
    if (podb->counter == 0)
    {
        tcmdbout2(mdb, podb->name);
        debug("%s's counter is %d\n", podb->name, podb->counter);
        free(podb->name);
        return 1;
    }
    else
    {
        tcmdbput(mdb, podb->name, strlen(podb->name), podb, sizeof(OpenedDB));
        debug("%s's counter is %d\n", podb->name, podb->counter);
        return 0;
    }
}
