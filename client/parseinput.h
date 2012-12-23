/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  parseinput.h                               */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  DistributedDB                              */
/*  Module Name            :  PareseInput                                */
/*  Language               :  C                                          */
/*  Target Environment     :  Any                                        */
/*  Created Time           :  Sun 16 Dec 2012 09:57:04 AM CST            */
/*  Description            :  Parse the input and call execution api     */
/*************************************************************************/

#ifndef PARSEINPUT_H
#define PARSEINPUT_H

#include <stdio.h>

#define MAX_INPUT_LEN 1024

/*
 * parse the input and then call dbapi to execute
 * input        : input
 * output       : none
 * input/output : none
 * return       : if input is quit/exit command, return 1
 *              : if input is incorrect command, return -1
 *              : if success, return 0
 */
int Parse(char *input);

#endif

