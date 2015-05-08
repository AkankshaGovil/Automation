#include <libgen.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "queDB.h"
#include "srvrlog.h"
#include "radacct.h"

#define NUM_DB 2
#define REC_SIZE 1024

#define PATH_LEN 200

extern char rad_dirname[];

#define DATABASE "radacct.db"


static DbEnv radacct_dbenv[NUM_DB];	/* Database environment. */



static int db_init_env(DbEnv *dbenv, const char home[])
{
	char dbPath[PATH_LEN];
	char *path;
	char *database;
	char *ptrptr;
	char *homeBuf;

	homeBuf = (char*) malloc(strlen(home) + 1);
	strcpy(homeBuf, home);

	path = strtok_r(homeBuf, "/", &ptrptr);
	database = strtok_r(NULL, "/", &ptrptr);
	sprintf(dbPath, "%s/%s/", rad_dirname, path);
	
	if(mkdir(dbPath, S_IRWXU) != 0){
		NETERROR(MRADC, ("radacct creating directory failed :%s\n", dbPath));
	}
	
	*dbenv = (DbEnv) malloc(sizeof(DbEnvStruct));
	(*dbenv)->databaseName = (char*) malloc(strlen(database) + 1);
	(*dbenv)->pathName = (char*) malloc(PATH_LEN + 1);
	strcpy((*dbenv)->databaseName, database);
	strcpy((*dbenv)->pathName, dbPath);
	(*dbenv)->numRecords = 20000;
	(*dbenv)->queueSize = 10000;
	(*dbenv)->initialFileSize = 20 *1024;

	return 0;
}


static int db_init(DbEnv dbenv)
{
	int ret;
	int error;


	/* Initialize the database. */
	if((ret = DbInit(dbenv, &error)) != 0)
	{
		return 1;
	}


	return 0;
}



int initRadacct_db(int *num_current, int *num_backlog)
{

	int i, ret;
	pthread_t tid;

	/* Initialize the current database environment. */
	if((ret = db_init_env(&radacct_dbenv[CURRENT], "RADACCT/current")) == 0)
	{
		if((ret = db_init(radacct_dbenv[CURRENT])) != 0)
		{
			return -1;
		}

	}

	/* Initialize the backlog database environment. */
	if((ret = db_init_env(&radacct_dbenv[BACKLOG], "RADACCT/backlog")) == 0)
	{
		if((ret = db_init(radacct_dbenv[BACKLOG])) != 0)
		{
			return -1;
		}

	}

	return 0;
}


void closeRadacct_db()
{
	int ret;
	int i;
	int error;
	
	for(i = 0; i < NUM_DB; ++i)
	{
		if((ret = DbClose(radacct_dbenv[i], &error)) != 0)
		{
			NETERROR(MRADC, ("radacct dp[%d] close failed: %d\n", i, ret));
		}

	}
}
	

int storeAccoutingInfo(int db, AccountingInfo *info)
{
	DbEnv dbenv = radacct_dbenv[db];
	char buf[REC_SIZE];
	int ret;
	int error;
	Data data;

	 marshalAccountingInfo(info, buf, sizeof(buf));
	
	data = (Data) malloc(sizeof(DataStruct));
	data->data = (char*) malloc(strlen(buf) + 1);
	data->len = strlen(buf);
	strcpy(data->data, buf);

	switch((ret = DbPut(dbenv, data, &error)))
	{
		case -1:
			NETERROR(MRADC, ("storeAccoutingInfo: error\n") );
			break;

		case 0:
			break;

	}	

	free(data->data);
	free(data);
	return ret;
}


AccountingInfo *getAccoutingInfo(int db)
{
	DbEnv dbenv = radacct_dbenv[db];
	DbTxn tid;
	Data data;
	AccountingInfo *info = NULL;
	char buf[REC_SIZE];
	int ret;
	int error;


	switch((ret = DbGet(dbenv, &data, &tid, &error)) != 0)
	{
		case -1:
			NETERROR(MRADC, ("getAccoutingInfo: error"));
			break;

		case 0:
			if((info = (AccountingInfo*)malloc(sizeof(AccountingInfo))))
			{
				memset(info, 0, sizeof(AccountingInfo));

				info->tid = (void*) tid;
				data->data[data->len] = '\0';

				unMarshalAccountingInfo(data->data, info);
			}
			break;

	}

	return info;
}


void removeAccoutingInfo(void* tid)
{
	int ret;
	int error;

	if(tid)
	{
		if((ret = DbCommit((DbTxn) tid, &error)) != 0)
		{
			NETERROR(MRADC, ("removeAccoutingInfo: commit failed\n"));
		}
	}
}
