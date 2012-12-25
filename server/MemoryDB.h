/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/MemoryDB.h                          */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Tue 25 Dec 2012 09:15:22 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef MEMORYDB_H
#define MEMORYDB_H

typedef void *MemDB;

MemDB MDBCreate();

void MDBClose(MemDB mdb);

void MDBSet(MemDB mdb, void *kbuf, int ksize, void *vbuf, int vsize);

void MDBDel(MemDB mdb, void *kbuf, int ksize);

void *MDBGet(MemDB mdb, void *kbuf, int ksize, int *sp);

#endif
