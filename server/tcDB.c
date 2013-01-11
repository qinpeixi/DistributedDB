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
#include <pthread.h>
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

    /* !!!!!!!!!!!!!! */
    /* memory leakage */
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

int SplitByKey(char *srcfile, dbKey lower_bound, char *destfile, dbKey upper_bound)
{
    printf("split %s, %s\n", srcfile, destfile);
    OpenedDB *podb1 = DBCreate(srcfile);
    OpenedDB *podb2 = DBCreate(destfile);
    TCHDB *srchdb = podb1->hdb;
    TCHDB *desthdb = podb2->hdb;
    long sum = (long) tchdbrnum(srchdb);
    int i;
    if (!tchdbiterinit(srchdb))
    {
        int ecode = tchdbecode(srchdb);
        fprintf(stderr, "Iterator initialize error: %s\n", tchdberrmsg(ecode));
        SetLastErrorMsg(tchdberrmsg(ecode));
        return FAILURE;
    }

    int bl = upper_bound > lower_bound;
    for (i=0; i<sum; i++)
    {
        int ksize = -1;
        dbKey key = *(dbKey *)tchdbiternext(srchdb, &ksize);
        int valueLen = -1;
        if ((bl && lower_bound < key && key <= upper_bound)
                || (!bl && ((lower_bound < key) || (key <= upper_bound))))
        {
            dbValue value = (dbValue)tchdbget(srchdb, &key, sizeof(dbKey), &valueLen);
            tchdbput(desthdb, &key, sizeof(dbKey), value, strlen(value)+1);
            tchdbout(srchdb, &key, sizeof(dbKey));
            free(value);
        }
    }

    DBDelete(podb1);
    DBDelete(podb2);
    return SUCCESS;
}

int Merge2Files(char *srcfile, char *destfile)
{
    printf("merge %s, %s\n", srcfile, destfile);
    OpenedDB *podb1 = DBCreate(srcfile);
    OpenedDB *podb2 = DBCreate(destfile);
    TCHDB *srchdb = podb1->hdb;
    TCHDB *desthdb = podb2->hdb;
    long sum = (long) tchdbrnum(srchdb);
    int i;
    if (!tchdbiterinit(srchdb))
    {
        int ecode = tchdbecode(srchdb);
        fprintf(stderr, "Iterator initialize error: %s\n", tchdberrmsg(ecode));
        SetLastErrorMsg(tchdberrmsg(ecode));
        return FAILURE;
    }

    for (i=0; i<sum; i++)
    {
        int ksize = -1;
        dbKey key = *(dbKey *)tchdbiternext(srchdb, &ksize);
        int valueLen = -1;
        dbValue value = (dbValue)tchdbget(srchdb, &key, sizeof(dbKey), &valueLen);
        tchdbput(desthdb, &key, sizeof(dbKey), value, strlen(value)+1);
        tchdbout(srchdb, &key, sizeof(dbKey));
        free(value);
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
pthread_mutex_t DBMUTEX;

/* if the db specified by name is not open, return NULL */
OpenedDB *GetOpenedDB(char *name)
{
    if (mdb == NULL)
    {
        mdb = tcmdbnew();
        pthread_mutex_init(&DBMUTEX, NULL);
        return NULL;
    }

    OpenedDB *podb;
    int valuelen = -1;
    pthread_mutex_lock(&DBMUTEX);
    podb = (OpenedDB *)tcmdbget(mdb, name, strlen(name), &valuelen);
    if (podb != NULL)
    {
        podb->counter ++;
        tcmdbput(mdb, name, strlen(name), podb, sizeof(OpenedDB));
        debug("%s's counter is %d\n", podb->name, podb->counter);
    }
    pthread_mutex_unlock(&DBMUTEX);

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
    pthread_mutex_lock(&DBMUTEX);
    podb = (OpenedDB *)tcmdbget(mdb, podb->name, strlen(podb->name), &valuelen);
    podb->counter --;
    if (podb->counter == 0)
    {
        tcmdbout2(mdb, podb->name);
        pthread_mutex_unlock(&DBMUTEX);
        debug("%s's counter is %d\n", podb->name, podb->counter);
        free(podb->name);
        return 1;
    }
    else
    {
        tcmdbput(mdb, podb->name, strlen(podb->name), podb, sizeof(OpenedDB));
        pthread_mutex_unlock(&DBMUTEX);
        debug("%s's counter is %d\n", podb->name, podb->counter);
        return 0;
    }
}
