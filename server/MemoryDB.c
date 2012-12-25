/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/MemoryDB.c                          */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Tue 25 Dec 2012 08:54:44 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <tcutil.h>
#include <tchdb.h>
#include <stdlib.h>
#include "MemoryDB.h"


MemDB MDBCreate()
{
    return tcmdbnew();
}

void MDBClose(MemDB mdb)
{
    tcmdbdel(mdb);
}

void MDBSet(MemDB mdb, void *kbuf, int ksize, void *vbuf, int vsize)
{
    tcmdbput(mdb, kbuf, ksize, vbuf, vsize);
}

void MDBDel(MemDB mdb, void *kbuf, int ksize)
{
    tcmdbout(mdb, kbuf, ksize);
}

/* return value is allocated with 'malloc', should be free when no longer use */
void *MDBGet(MemDB mdb, void *kbuf, int ksize, int *sp)
{
    return tcmdbget(mdb, kbuf, ksize, sp);
}
