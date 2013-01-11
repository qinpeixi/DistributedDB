/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  testtcDB.c                                 */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sat 22 Dec 2012 08:12:14 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../common/Database.h"

int main()
{
    dbKey key;
    dbValue setvalue;
    dbValue getvalue;
    DataBase hdb = DBCreate(".//test//abc.db");
    key = 100;
    setvalue = "hello world";

    int ret = DBSetKeyValue(hdb, key, setvalue);
    assert(ret == 0);
    getvalue = DBGetKeyValue(hdb, key);
    assert(getvalue != NULL);
    ret = DBDelKeyValue(hdb, key);
    assert(ret == 0);

    int i;
    char value[10];

    DataBase db = DBCreate("a.db");
/*    for (i=0; i<100; i++)
    {
        sprintf(value, "%d", i); 
        DBSetKeyValue(db, i, value);
    }
    SplitByKey("a.db", 50, "a.db~", 100);
    for (i=0; i<50; i++)
    {
        char *v = DBGetKeyValue(db, i);
        dbKey k;
        sscanf(v, "%d", &k);
        assert(i == k);
        free (v);
    }
    DBDelete(db);
    db = DBCreate("a.db~");
    for (i=50; i<100; i++)
    {
        char *v = DBGetKeyValue(db, i);
        dbKey k;
        sscanf(v, "%d", &k);
        assert(i == k);
        free(v);
    }
    DBDelete(db);
    db = DBCreate("a.db");
*/    for (i=0; i<4000; i++)
    {
        sprintf(value, "%d", i);
        DBSetKeyValue(db, i, value);
    }
    printf("Test of tcDB.c                : PASS\n");
    return 0;
}
