/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/HandleRequest.h                     */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Tue 25 Dec 2012 11:03:41 AM CST            */
/*  Description            :                                             */
/*************************************************************************/

#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUESE_H

#include <semaphore.h>

extern sem_t MSG_SEM;

void InitThreads();
void KillThreads();

#endif
