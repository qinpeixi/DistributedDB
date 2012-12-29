/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  test/testMDB.c                             */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Tue 25 Dec 2012 09:19:25 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <assert.h>
#include "../server/MemoryDB.h"

typedef struct 
{
    int sock;
    char addr[50];
    void *append;
} ClientSocket;

int main()
{
    int sock = 1;
    ClientSocket cs;

    cs.sock = sock;
    MemDB mdb = MDBCreate();
    MDBSet(mdb, &sock, sizeof(int), &cs, sizeof(ClientSocket));

    ClientSocket *pcs;
    int vsize = -1;
    pcs = MDBGet(mdb, &sock, sizeof(int), &vsize);

    assert(pcs->sock = sock);
    printf("Test of MemoryDB.c            : PASS\n");

    return 0;
}
