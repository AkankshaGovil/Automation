#ifndef _QUEDB_H_
#define _QUEDB_H_
#include <stddef.h>
#include <inttypes.h>
#include "list.h"


typedef struct _dbenv_
{
	char* databaseName; //Database where data is to be added
	char* pathName;	     //Path where data files are to be created
	uint32_t numRecords; //no. of records in a file
	uint32_t queueSize;  //No. of data queue nodes in memory
	uintptr_t dbHandle;   //Pointer of database structure in memory
	uint32_t initialFileSize;   //Size of initial file to be created. 
}DbEnvStruct, *DbEnv;

typedef struct _dbtxn_
{
	uint32_t offset; //file offset
	uintptr_t fileHandle;   //File pointer
	uintptr_t dbHandle;	//Pointer to database structure in memory
}DbTxnStruct, *DbTxn;

typedef struct _data_
{
	char* data; //Data
	uint32_t len; //Length of data
}DataStruct, *Data;

extern int DbInit(DbEnv env, int* error);
extern int DbSetDbParam(char* databaseName, char* pathName, uint32_t numRecords, uint32_t queueSize, DbEnv env, int* error);
extern int DbGet(DbEnv env, Data *data, DbTxn *t, int* error);
extern int DbPut(DbEnv env, Data data, int* error);
extern int DbCommit(DbTxn t, int* error);
extern int DbClose(DbEnv env, int* error);

#endif
