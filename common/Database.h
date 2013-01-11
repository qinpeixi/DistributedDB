/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  DBAPI.h                                    */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :  Test Database                              */ 
/*  Module Name            :  Database                                   */ 
/*  Language               :  C                                          */  
/*  Target Environment     :  Any                                        */ 
/*  Created Time           :  2012年11月29日 星期四 21时29分01秒         */
/*  Description            :  Abstraction of database                    */ 
/*************************************************************************/

#ifndef _DATABASE_H_ 
#define _DATABASE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************/
/*     Abstract Interface of NoSQL DB API            */
/*****************************************************/
#define SUCCESS  0
#define FAILURE  (-1)

typedef void*          DataBase;
typedef unsigned int   dbKey;
typedef char*          dbValue;
extern const int MAXVALUELEN;

/*
 * Create an Database
 * input	: dbName 
 * output	: None
 * in/out	: None
 * return	: if SUCCESS return (DataBase *)hdb handler
 *          : if FAILURE exit(-1)
 */
DataBase  DBCreate(char *dbName);

/*
 * Delete the Database
 * input	: hdb
 * output	: None
 * in/out	: None
 * return	: SUCCESS(0)/exit(-1)
 */
int DBDelete(DataBase hdb);

/*
 * Set key/value
 * input	: hdb, key, value
 * output	: None
 * in/out	: None
 * return	: SUCCESS(0)/FAILURE(-1)
 */	
int DBSetKeyValue(DataBase hdb, dbKey key, dbValue value);

/*
 * get key/value
 * input	: hdb, key
 * output	: None
 * in/out	: None 
 * return	: if SUCCESS return (dbValue *)value
 *          : if FAILURE return NULL
 */
dbValue DBGetKeyValue(DataBase hdb, dbKey key);

/*
 * delete a key/value
 * input    : hdb, key
 * output   : None
 * in/out   : Node
 * return   : SUCCESS(0)/FAILURE(-1)
 */
int DBDelKeyValue(DataBase hdb, dbKey key);

int SplitByKey(char *srcfile, dbKey lower_bound, char *destfile, dbKey upper_bound);

int Merge2Files(char *srcfile, char *destfile);

/*
 * Get the last error message
 * input    : None
 * output   : None
 * in/out   : None
 * return   : The last error message
 */
char *DBGetLastErrorMsg();

#ifdef __cplusplus
}
#endif
#endif /* _DATABASE_H_ */

