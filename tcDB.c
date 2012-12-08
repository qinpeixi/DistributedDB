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
#include <tcutil.h>
#include <tchdb.h>
#include <limits.h>
#include "Database.h"

const int MAXVALUELEN = 1024;

DataBase DBCreate(char *dbName)
{
    TCHDB *hdb;
    int ecode;

    /* create the database */
    hdb = tchdbnew();

    /* open the database */
    if(!tchdbopen(hdb, dbName, HDBOWRITER | HDBOCREAT))
    {
        ecode = tchdbecode(hdb);
        fprintf(stderr, "open error: %s\n", tchdberrmsg(ecode));
        return NULL;
    }

    return (DataBase)hdb;
}

int DBDelete(DataBase hdb)
{
    int ecode;

    /* close the database */
    if(!tchdbclose(hdb))
    {
        ecode = tchdbecode(hdb);
        fprintf(stderr, "close error: %s\n", tchdberrmsg(ecode));
        return FAILURE;
    }

    return SUCCESS;
}

int DBSetKeyValue(DataBase hdb, dbKey key, dbValue value)
{
    int ecode;

    /* store key-value into the database */
    if (!tchdbput(hdb, &key, sizeof(dbKey), value, sizeof(value)+1)) 
    {
        ecode = tchdbecode(hdb);
        fprintf(stderr, "put error: %s\n", tchdberrmsg(ecode));

        return FAILURE;
    }

    return SUCCESS;
}

dbValue DBGetKeyValue(DataBase hdb, dbKey key)
{
    int ecode;
    dbValue value;
    int valueLen = MAXVALUELEN;

    /* get the value */
    value = (dbValue)tchdbget(hdb, &key, sizeof(dbKey), &valueLen);
    if (value)
    {
        return value;
    }
    else
    {
        ecode = tchdbecode(hdb);
        fprintf(stderr, "get error: %s\n", tchdberrmsg(ecode));
        return NULL;
    }
}

int DBDelKeyValue(DataBase hdb, dbKey key)
{
    int ecode;

    if (!tchdbout(hdb, &key, sizeof(dbKey)))
    {
        ecode = tchdbecode(hdb);
        fprintf(stderr, "delete error: %s\n", tchdberrmsg(ecode));

        return FAILURE;
    }

    return SUCCESS;
}

