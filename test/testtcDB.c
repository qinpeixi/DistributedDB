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
#include <assert.h>
#include "../common/Database.h"

int main()
{
    dbKey key;
    dbValue setvalue;
    dbValue getvalue;
    printf("\n>>>>> Test tcDB.c  <<<<<\n");
    DataBase hdb = DBCreate(".//test//abc.db");
    key = 100;
    setvalue = "hello world";

    int ret = DBSetKeyValue(hdb, key, setvalue);
    assert(ret == 0);
    getvalue = DBGetKeyValue(hdb, key);
    assert(getvalue != NULL);
    ret = DBDelKeyValue(hdb, key);
    assert(ret == 0);
    printf(">>>>> Test report of tcDB.c :  PASS\n\n");
    return 0;
}
