#include <libgen.h>
#include <db.h>
#include <pthread.h>
#include <errno.h>
#include "srvrlog.h"
#include "radacct.h"
#include "nxosd.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define USE_TXN 1

#define NUM_DB 2
#define REC_SIZE 1024

extern char rad_dirname[];

#define DATABASE "radacct.db"

void *db_maint(void *arg);

static pthread_mutex_t radacctMutex = PTHREAD_MUTEX_INITIALIZER;

static DB_ENV *radacct_dbenv[NUM_DB];	/* Database environment. */
static DB *radacct_dbp[NUM_DB];			/* Database handles. */


static int db_init_env(DB_ENV **dbenv, const char *home)
{
	int ret;
	char abs_home[256];

	snprintf(abs_home, sizeof(abs_home), "%s/%s", rad_dirname, home);

	if((ret = nx_mkdirp(abs_home, 0755)) == -1)
	{
		if(errno != EEXIST)
		{
			NETERROR(MRADC, ("db_env_create: can not open directory path: %s\n", abs_home));
			return 1;
		}
	}

	if((ret = db_env_create(dbenv, 0)) != 0)
	{
		NETERROR(MRADC, ("db_env_create: %s\n", db_strerror(ret)));
		return 1;
	}

	(*dbenv)->set_errfile(*dbenv, stderr);
	(*dbenv)->set_errpfx(*dbenv, "gis");
	(void)(*dbenv)->set_cachesize(*dbenv, 0, 100 * 1024, 0);
	(void)(*dbenv)->set_lg_max(*dbenv, 200000);
	(void)(*dbenv)->set_tx_max(*dbenv, 300);
	(void)(*dbenv)->set_flags(*dbenv, DB_DIRECT_DB, 1);
	//(void)(*dbenv)->set_flags(*dbenv, DB_DIRECT_LOG, 1);
	//(void)(*dbenv)->set_flags(*dbenv, DB_TXN_NOSYNC, 1);

	#if USE_TXN
	if((ret = (*dbenv)->open(*dbenv, abs_home,
			DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG |
				DB_INIT_MPOOL | DB_INIT_TXN | DB_THREAD | DB_RECOVER, 0)) != 0)
	#else
	if((ret = (*dbenv)->open(*dbenv, abs_home,
			DB_CREATE | DB_INIT_LOCK | DB_INIT_MPOOL | DB_THREAD | DB_RECOVER, 0)) != 0)
	#endif
	{
		(*dbenv)->err(*dbenv, ret, NULL);
		(void)(*dbenv)->close(*dbenv, 0);
		return 1;
	}

	return 0;
}


static int db_init(DB_ENV *dbenv, DB **dbp)
{
	int ret;

	/* Initialize the database. */
	if((ret = db_create(dbp, dbenv, 0)) != 0)
	{
		dbenv->err(dbenv, ret, "db_create");
		(void)dbenv->close(dbenv, 0);
		return 1;
	}

	if((ret = (*dbp)->set_re_len(*dbp, REC_SIZE)) != 0)
	{
		(*dbp)->err(*dbp, ret, "set_re_source");
		goto err;
	}

	if((ret = (*dbp)->set_q_extentsize(*dbp, 1000)) != 0)
	{
		(*dbp)->err(*dbp, ret, "set_q_extentsize");
		goto err;
	}

	#if USE_TXN
	if((ret = (*dbp)->open(*dbp, NULL, DATABASE, NULL, DB_QUEUE,
					DB_AUTO_COMMIT | DB_CREATE | DB_THREAD, 0664)) != 0)
	#else
	if((ret = (*dbp)->open(*dbp, NULL, DATABASE, NULL, DB_QUEUE,
								DB_CREATE | DB_THREAD, 0664)) != 0)
	#endif
	{
		(*dbp)->err(*dbp, ret, "%s: open", DATABASE);
		goto err;
	}

	return 0;

err:
	(void)(*dbp)->close(*dbp, 0);
	(void)dbenv->close(dbenv, 0);

	return 1;
}



int initRadacct_db(int *num_current, int *num_backlog)
{

	int i, ret;
	pthread_t tid;
	DB *dbp;
	DB_QUEUE_STAT *stat;

	/* Initialize the current database environment. */
	if((ret = db_init_env(&radacct_dbenv[CURRENT], "RADACCT/current")) == 0)
	{
		if((ret = db_init(radacct_dbenv[CURRENT], &radacct_dbp[CURRENT])) != 0)
		{
			return -1;
		}

		dbp = radacct_dbp[CURRENT];

		if((ret = dbp->stat(dbp, &stat, 0)) == 0)
		{
			*num_current = stat->qs_ndata;

			free(stat);
		}
	}
	else
	{
		return -1;
	}

	/* Initialize the backlog database environment. */
	if((ret = db_init_env(&radacct_dbenv[BACKLOG], "RADACCT/backlog")) == 0)
	{
		if((ret = db_init(radacct_dbenv[BACKLOG], &radacct_dbp[BACKLOG])) != 0)
		{
			return -1;
		}

		dbp = radacct_dbp[BACKLOG];

		if((ret = dbp->stat(dbp, &stat, 0)) == 0)
		{
			*num_backlog = stat->qs_ndata;

			free(stat);
		}
	}
	else
	{
		return -1;
	}

	#if USE_TXN
	for(i = 0; i < NUM_DB; ++i)
	{
		if((ret = pthread_create(&tid, NULL, db_maint, radacct_dbenv[i])) != 0)
		{
			NETERROR(MRADC, ("failed to create db_maint [%d] thread: %s\n", i, strerror(ret)));
			return -1;
		}
	}
	#endif

	return 0;
}

int db_maint_delay = 1;

void *db_maint(void *arg)
{
	DB_ENV *dbenv = arg;
	int i, ret;
	char **begin, **list;

	for(;; sleep(db_maint_delay * 60))
	{
		if((ret = dbenv->txn_checkpoint(dbenv, 0, 0, DB_FORCE)) != 0)
		{
			NETERROR(MRADC, ("db_maint thread: checkpoint failed: %d\n", ret));
		}

		if((ret = dbenv->log_archive(dbenv, &list, DB_ARCH_ABS)) != 0)
		{
			NETERROR(MRADC, ("db_maint thread: log archive failed: %d\n", ret));
		}

		if(list != NULL)
		{
			for(begin = list; *list != NULL; ++list)
			{
				if((ret = remove(*list)) != 0)
				{
					NETERROR(MRADC, ("db_maint thread: remove %s failed: %d\n", *list, ret));
				}
			}

			free(begin);
		}
	}
}


void closeRadacct_db()
{
	int i, ret;

	for(i = 0; i < NUM_DB; ++i)
	{
		if((ret = radacct_dbp[i]->close(radacct_dbp[i], 0)) != 0)
		{
			NETERROR(MRADC, ("radacct dp[%d] close failed: %d\n", i, ret));
		}

		if((ret = radacct_dbenv[i]->close(radacct_dbenv[i], 0)) != 0)
		{
			NETERROR(MRADC, ("radacct env[%d] close failed: %d\n", i, ret));
		}
	}
}
	

int storeAccoutingInfo(int db, AccountingInfo *info)
{
	DB *dbp = radacct_dbp[db];
	DBT key, data;
	char buf[REC_SIZE];
	int ret;

	marshalAccountingInfo(info, buf, sizeof(buf));

	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.flags = DB_DBT_MALLOC;

	data.data = buf;
	data.size = data.ulen = sizeof(buf);
	data.flags = DB_DBT_USERMEM;

	#if USE_TXN
	switch((ret = dbp->put(dbp, NULL, &key, &data, DB_AUTO_COMMIT | DB_APPEND)))
	#else
	switch((ret = dbp->put(dbp, NULL, &key, &data, DB_APPEND)))
	#endif
	{
		case DB_LOCK_DEADLOCK:
			NETERROR(MRADC, ("storeAccoutingInfo: deadlock: %s\n", db_strerror(ret)));
			break;

		case 0:
			break;

		default:
			NETERROR(MRADC, ("storeAccoutingInfo: oops: %d\n", ret));
			break;
	}	

	if(key.data) free(key.data);

	return ret;
}


AccountingInfo *getAccoutingInfo(int db)
{
	DB_ENV *dbenv = radacct_dbenv[db];
	DB *dbp = radacct_dbp[db];
	DB_TXN *tid = NULL;
	DBT key, data;
	db_recno_t recno;
	AccountingInfo *info = NULL;
	char buf[REC_SIZE];
	int ret;

	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = &recno;
	key.size = key.ulen = sizeof(recno);
	key.flags = DB_DBT_USERMEM;

	data.data = buf;
	data.ulen = sizeof(buf);
	data.flags = DB_DBT_USERMEM;

	#if USE_TXN
	if((ret = dbenv->txn_begin(dbenv, NULL, &tid, 0)) != 0)
	{
		NETERROR(MRADC, ("getAccoutingInfo: transaction failed: %s\n", db_strerror(ret)));
		return 0;
	}

	switch((ret = dbp->get(dbp, tid, &key, &data, DB_CONSUME)))
	#else
	switch((ret = dbp->get(dbp, NULL, &key, &data, DB_CONSUME)))
	#endif
	{
		case DB_LOCK_DEADLOCK:
			NETERROR(MRADC, ("getAccoutingInfo: deadlock: %s\n", db_strerror(ret)));
			break;

		case 0:
			if((info = (AccountingInfo*)malloc(sizeof(AccountingInfo))))
			{
				memset(info, 0, sizeof(AccountingInfo));

				info->tid = tid;

				unMarshalAccountingInfo(buf, info);
			}
			break;

		default:
			NETERROR(MRADC, ("getAccoutingInfo: oops: %d\n", ret));
			break;
	}

	return info;
}


void removeAccoutingInfo(void *tid)
{
	int ret;

	if(tid)
	{
		if((ret = ((DB_TXN*)tid)->commit((DB_TXN*)tid, 0)) != 0)
		{
			NETERROR(MRADC, ("removeAccoutingInfo: commit failed: %s\n", db_strerror(ret)));
		}
	}
}
