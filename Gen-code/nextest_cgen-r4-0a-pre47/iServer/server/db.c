/*
 * Db management functions
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <gdbm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "alerror.h"
#include "ipc.h"
#include "key.h"
#include "db.h"
#include "profile.h"
#include "srvrlog.h"
#include "ipcutils.h"
#include <malloc.h>
#include "nxosd.h"

void gdbm_fatal_fn(char *error)
{
	fprintf(stderr, error);
}

DB
DbOpen(char *dbName, char *lockName, DB_tDb *db)
{
	int dbID, lockID;
	GDBM_FILE gdbmf = (GDBM_FILE) NULL;
	int lockfd;
	int semid;
	int value = 10;

	dbID = DbID(dbName);

	if (dbID >= 0)
	{
		return DbOpenByID(dbName, dbID, db);
	}

	lockID = DbLockID(lockName);
	if (lockID >= 0)
	{
		return DbOpenByID(dbName, lockID, db);
	}

	/* THis looks like a totally new open */

	lockfd = open(lockName, O_RDWR|O_CREAT, S_IRWXU);
     
	if (lockfd < 0)
	{
		perror("open");
		ERROR(MDB, ("Could not open file %s\n", lockName));
		return 0;
	}

	close(lockfd);

	if (sm_get(ftok(lockName, 0), 1, 0, &semid) < 0)
	{
		NETERROR(MDB, ("sm_create failed\n"));
	}
	else
		sm_p(semid, 0, 0);

     gdbmf = gdbm_open(dbName, 0, db->read_write, 
															S_IRWXU|S_IROTH|GDBM_NOLOCK, gdbm_fatal_fn);

     if (gdbmf == NULL)
     {
	  ERROR(MDB, ("GDBM ERROR %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);

		sm_v(semid);
	  return 0;
     }

#if 1
					if (gdbm_setopt(gdbmf, GDBM_CACHESIZE, &value, sizeof(value)) < 0)
					{
	  ERROR(MDB, ("GDBM ERROR %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);
					}
#endif

     db->db = gdbmf;
	 db->dbid = 0;
     db->lockfd = semid;

     return ((DB)gdbmf);
}

/* Opens the database. Note that the caller has to specify
 * the lock file name, and the db file name correctly. The function will 
 * return a DB handle, which must be used to close the database.
 */
DB
DbOpenByID(char *dbName, int dbid, DB_tDb *db)
{
	GDBM_FILE gdbmf = (GDBM_FILE) NULL;
	int lockfd;
	int semid;
	char *lockName = NULL;
	int value = 10;
     
	if (smn_get(ftok(DBLOCKDIR, ISERVER_SEM_PROJ), DB_eMax, 1, 0, &semid) < 0)
	{
		NETERROR(MDB, ("sm_create failed\n"));
	}
	else
	{
		smn_p(semid, dbid, 0, 0);
		NETDEBUG(MDB, NETLOG_DEBUG4, ("pid %lu: %lu acquired locks for db %d\n", ULONG_FMT(getpid()), ULONG_FMT(pthread_self()), dbid));
	}

     gdbmf = gdbm_open(dbName, 4096, db->read_write, 
														S_IRWXU|S_IROTH|GDBM_NOLOCK, gdbm_fatal_fn);

     if (gdbmf == NULL)
     {
	  ERROR(MDB, ("GDBM ERROR %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);

		smn_v(semid, dbid);
		NETDEBUG(MDB, NETLOG_DEBUG4, ("pid %lu: %lu released locks for db %d on error\n", ULONG_FMT(getpid()), ULONG_FMT(pthread_self()), dbid));
	  return 0;
     }

#if 1
					if (gdbm_setopt(gdbmf, GDBM_CACHESIZE, &value, sizeof(value)) < 0)
					{
	  ERROR(MDB, ("GDBM ERROR %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);
					}
#endif

     db->db = gdbmf;
	 db->dbid = dbid;
     db->lockfd = semid;

     return ((DB)gdbmf);
}

DB
DbTestOpen(char *dbName, char *lockName, DB_tDb *db)
{
	int dbID, lockID;
	GDBM_FILE gdbmf = (GDBM_FILE) NULL;
	int lockfd;
	int semid;

	dbID = DbID(dbName);

	if (dbID >= 0)
	{
		return DbTestOpenByID(dbName, dbID, db);
	}

	lockID = DbLockID(lockName);
	if (lockID >= 0)
	{
		return DbTestOpenByID(dbName, lockID, db);
	}

	if (sm_get(ftok(lockName, 0), 1, 0, &semid) < 0)
	{
		NETERROR(MDB, ("sm_create failed\n"));
	}
	else
	{
		if (sm_p(semid, IPC_NOWAIT, 0) < 0)	
		{
			return ((DB)NULL);
		}
	}

     gdbmf = gdbm_open(dbName, 0, db->read_write, S_IRWXU|S_IROTH, gdbm_fatal_fn);

     if (gdbmf == NULL)
     {
	  ERROR(MDB, ("GDBM ERROR %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);

		sm_v(semid);
	  return 0;
     }

     db->db = gdbmf;
	 db->dbid = 0;
     db->lockfd = semid;

     return ((DB)gdbmf);
}

DB
DbTestOpenByID(char *dbName, int dbid, DB_tDb *db)
{
	GDBM_FILE gdbmf = (GDBM_FILE) NULL;
	int lockfd;
	int semid;
	char *lockName = NULL;
     
	if (smn_get(ftok(DBLOCKDIR, ISERVER_SEM_PROJ), DB_eMax, 1, 0, &semid) < 0)
	{
		NETERROR(MDB, ("sm_create failed\n"));
	}
	else
	{
		if (smn_p(semid, dbid, IPC_NOWAIT, 0) < 0)
		{
			if (errno == EAGAIN)
			{
				// Locks are busy right now
			}

			return ((DB)NULL);
		}

		NETDEBUG(MDB, NETLOG_DEBUG4, 
			("pid %lu: %lu acquired locks for db %d\n", 
			ULONG_FMT(getpid()), ULONG_FMT(pthread_self()), dbid));
	}

     gdbmf = gdbm_open(dbName, 0, db->read_write, S_IRWXU|S_IROTH, gdbm_fatal_fn);

     if (gdbmf == NULL)
     {
	  ERROR(MDB, ("GDBM ERROR %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);

		smn_v(semid, dbid);
		NETDEBUG(MDB, NETLOG_DEBUG4, ("pid %lu: %lu released locks for db %d on error\n", ULONG_FMT(getpid()), ULONG_FMT(pthread_self()), dbid));
	  return 0;
     }

     db->db = gdbmf;
	 db->dbid = dbid;
     db->lockfd = semid;

     return ((DB)gdbmf);
}

AlStatus
DbClose (DB_tDb *db)
{
//	gdbm_sync((GDBM_FILE)(db->db));
		if (db->db)
     gdbm_close((GDBM_FILE)(db->db));
    
     db->db = NULL;

     /* Now we are sure that we have sole access to the file. */
     /* We have rwx, others have r */
     /* If needed, the db is created here */

	smn_v(db->lockfd, db->dbid);
	NETDEBUG(MDB, NETLOG_DEBUG4, ("pid %lu: %lu released locks for db %d\n", ULONG_FMT(getpid()), ULONG_FMT(pthread_self()), db->dbid));

     return (AL_DBOK);
}

int
DbStore (DB db, datum key, datum data)
{
     if (gdbm_store((GDBM_FILE)db,
		    key, data, GDBM_INSERT) < 0)
     {
	  ERROR(MDB, ("gdbm_store error %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);
	  return (AL_DBERRSTORE);
     }
     
     return AL_DBOK;
}


datum
DbFind (DB db, datum key)
{
     return (gdbm_fetch((GDBM_FILE)db, key));
}

AlStatus
DbDelete (DB db, datum key)
{
     if (gdbm_delete((GDBM_FILE)db,  key) < 0)
     {
	  ERROR(MDB, ("gdbm_delete error %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);
	  return (AL_DBERRDELETE);
     }
     return AL_DBOK;
}

static datum 
DbFirstKey (DB db)
{
     return (gdbm_firstkey((GDBM_FILE)db));
}

static datum
DbNextKey (DB db, datum key)
{
     return (gdbm_nextkey((GDBM_FILE)db, key));
}

/********************************************************************************/

/* Now we have functions which hide gdbm from
 * the user 
 */

AlStatus
DbStoreEntry (DB db, char *dinfo, int dlen, char *skey, int skeylen)
{
     datum key, data;
     
     key.dptr = (char *)skey;
     key.dsize = skeylen;

     data.dptr = (char *)dinfo;
     data.dsize = dlen;

#if 1
     if (gdbm_store((GDBM_FILE)db,
		    key, data, GDBM_REPLACE) < 0)
     {
	  ERROR(MDB, ("gdbm_store error %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);
	  return (AL_DBERRSTORE);
     }
#endif

#if 0
     gdbm_delete((GDBM_FILE)db,  key);
     return DbInsertEntry(db, dinfo, dlen, skey, skeylen);     
#endif

     return AL_DBOK;
}

AlStatus
DbInsertEntry (DB db, char *dinfo, int dlen, char *skey, int skeylen)
{
     datum key, data;
     
     key.dptr = (char *)skey;
     key.dsize = skeylen;

     data.dptr = (char *)dinfo;
     data.dsize = dlen;

     if (gdbm_store((GDBM_FILE)db,
		    key, data, GDBM_INSERT) != 0)
     {
	  ERROR(MDB, ("gdbm_store error %d\n", gdbm_errno));
	  gdbm_strerror(gdbm_errno);
	  return (AL_DBERRSTORE);
     }

     return AL_DBOK;
}


char *
DbFindEntry (DB db, char *skey, int skeylen)
{
     datum key, data;

     key.dptr = (char *)skey;
     key.dsize = skeylen;
     
     data = gdbm_fetch((GDBM_FILE)db, key);
     return ((char *)data.dptr);
}

AlStatus
DbDeleteEntry (DB db, char *skey, int skeylen)
{
     datum key;
	 char *errstr;

     key.dptr = (char *)skey;
     key.dsize = skeylen;
     
     if (gdbm_delete((GDBM_FILE)db,  key) < 0)
     {
		  /* errstr = gdbm_strerror(gdbm_errno); */
		  NETDEBUG(MDB, NETLOG_DEBUG4, ("gdbm_delete error %d\n", gdbm_errno));
		  return (AL_DBERRDELETE);
     }
     return AL_DBOK;
}

char *
DbGetFirstEntry (DB db)
{
     datum key, data;

     key = gdbm_firstkey((GDBM_FILE)db);

     if (key.dptr)
     {
	 data = gdbm_fetch((GDBM_FILE)db, key);
	 // free the key
	 free(key.dptr);
	 return ((char *)data.dptr);
     }
     else
     {
	  return 0;
     }
}

char *
DbGetFirstKey (DB db)
{
     datum key, data;

     key = gdbm_firstkey((GDBM_FILE)db);

     return (key.dptr);
}

char *
DbGetNextEntry (DB db, char *skey, int skeylen)
{
     datum key, data;

     key.dptr = (char *)skey;
     key.dsize = skeylen;
     

     key = gdbm_nextkey((GDBM_FILE)db, key);

     if (key.dptr)
     {
	 data = gdbm_fetch((GDBM_FILE)db, key);
	 // free the key
	 free(key.dptr);
	 return ((char *)data.dptr);
     }
     else
     {
	  return 0;
     }
}

char *
DbGetNextKey (DB db, char *skey, int skeylen)
{
     datum key, data;

     key.dptr = (char *)skey;
     key.dsize = skeylen;

     key = gdbm_nextkey((GDBM_FILE)db, key);

     return (key.dptr);
}

int
DbDeleteLocks(void)
{
	int i, semid;

#if 0
	for (i = 0; i < DB_eMax; i++)
	{
		if (sm_get(ftok(DBLOCKNAME(dbHandles,i), 0), 1, 0, &semid) >= 0)
		{
			/* Delete the semaphore */
			sm_delete(semid);
		}
	}
#endif

	if (smn_get(ftok(DBLOCKDIR, ISERVER_SEM_PROJ), DB_eMax, 1, 0, &semid) >= 0)
	{	
		smn_delete(semid);
	}

	return(0);
}

int
DbCreateLocks(void)
{
	int i, semid;
	char fn[] = "DbCreateLocks():";

	if (smn_create(ftok(DBLOCKDIR, ISERVER_SEM_PROJ), DB_eMax, 1, 0, &semid) < 0)
	{
		NETERROR(MDB, ("%s Could not create database locks\n", fn));
		return(-1);
	}
	return(0);
}

int
DbID(char *dbname)
{
	int i;

	for (i = 0; i < DB_eMax; i++)
	{
		if (!strcmp(dbname, DBNAME(dbHandles,i)))
		{
			return i;
		}
	}

	return -1;
}

int
DbLockID(char *lockname)
{
	int i;

	for (i = 0; i < DB_eMax; i++)
	{
		if (!strcmp(lockname, DBLOCKNAME(dbHandles,i)))
		{
			return i;
		}
	}

	return -1;
}

int
DbUpdate()
{
	static char cmd[256];
	static int first = 0;

	if (++first == 1)
	{
		sprintf(cmd, "touch %s", DB_LASTUPDATE_FILE);
	}

	system(cmd);

	return 0;
}

char *
DbExtractNextKey (char *dbName, int dbid, char *skey, int skeylen, int *outkeylen)
{
	char fn[] = "DbExtractNextKey():";
	DB db;
	DB_tDb dbstruct = { 0 };
	datum key;

	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(dbName, dbid, &dbstruct)))
	{
		NETERROR(MDB, ("%s cannot open database %s(%d)\n", fn, dbName, dbid));
		return NULL;
	}

	key.dptr = skey;
	key.dsize = skeylen;
	key = DbNextKey (db, key);

	DbClose(&dbstruct);

	*outkeylen = key.dsize;

	return key.dptr;
}

char *
DbExtractFirstKey (char *dbName, int dbid, int *outkeylen)
{
	char fn[] = "DbExtractFirstKey():";
	DB db;
	DB_tDb dbstruct = { 0 };
	datum key;

	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(dbName, dbid, &dbstruct)))
	{
		NETERROR(MDB, ("%s cannot open database %s(%d)\n", fn, dbName, dbid));
		return NULL;
	}

	key = DbFirstKey (db);

	DbClose(&dbstruct);

	*outkeylen = key.dsize;

	return key.dptr;
}

char *
DbExtractNextEntry (char *dbName, int dbid, char *skey, int skeylen)
{
	char fn[] = "DbExtractNextEntry():";
	DB db;
	DB_tDb dbstruct = { 0 };
	char *entry;

	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(dbName, dbid, &dbstruct)))
	{
		NETERROR(MDB, ("%s cannot open database %s(%d)\n", fn, dbName, dbid));
		return NULL;
	}

	entry = DbGetNextEntry (db, skey, skeylen);

	DbClose(&dbstruct);

	return entry;
}

char *
DbExtractFirstEntry (char *dbName, int dbid)
{
	char fn[] = "DbExtractFirstEntry():";
	DB db;
	DB_tDb dbstruct = { 0 };
	char *entry;

	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(dbName, dbid, &dbstruct)))
	{
		NETERROR(MDB, ("%s cannot open database %s(%d)\n", fn, dbName, dbid));
		return NULL;
	}

	entry = DbGetFirstEntry (db);

	DbClose(&dbstruct);

	return entry;
}

char *
DbExtractEntry (char *dbName, int dbid, char *skey, int skeylen)
{
	char fn[] = "DbExtractEntry():";
	DB db;
	DB_tDb dbstruct = { 0 };
	char *entry;

	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(dbName, dbid, &dbstruct)))
	{
		NETERROR(MDB, ("%s cannot open database %s(%d)\n", fn, dbName, dbid));
		return NULL;
	}

	entry = DbFindEntry (db, skey, skeylen);

	DbClose(&dbstruct);

	return entry;
}

AlStatus
DbUpdateEntry (char *dbName, int dbid, char *dinfo, int dlen, char *skey, int skeylen)
{
	char fn[] = "DbUpdateEntry():";
	DB db;
	DB_tDb dbstruct = { 0 };
	AlStatus status;

	dbstruct.read_write = GDBM_WRCREAT;

	if (!(db = DbOpenByID(dbName, dbid, &dbstruct)))
	{
		NETERROR(MDB, ("%s cannot open database %s(%d)\n", fn, dbName, dbid));
	  	return (AL_DBERRSTORE);
	}

	status = DbStoreEntry (db, dinfo, dlen, skey, skeylen);

	DbClose(&dbstruct);

	return status;
}

AlStatus
DbRemoveEntry (char *dbName, int dbid, char *key, int keylen)
{
	char fn[] = "DbRemoveEntry():";
	DB db;
	DB_tDb dbstruct = { 0 };
	AlStatus status;
     
	dbstruct.read_write = GDBM_WRCREAT;

	if (!(db = DbOpenByID(dbName, dbid, &dbstruct)))
	{
		NETERROR(MDB, ("%s cannot open database %s(%d)\n", fn, dbName, dbid));
	  	return (AL_DBERRSTORE);
	}

	status = DbDeleteEntry (db, key, keylen);

	DbClose(&dbstruct);

	return status;
}

int
DbCount(DB db)
{
	int n = 0;
	datum key, oldkey;

	for (key = DbFirstKey (db);
			key.dptr;
			key = DbNextKey(db, oldkey), free(oldkey.dptr))
	{
		n ++;
		oldkey = key;
	}

	return n;
}

int
DbExtractCount(char *dbName, int dbid)
{
	char *key, *oldkey;
	int keylen;
	int n = 0;

	for (key = DbExtractFirstKey (dbName, dbid, &keylen);
			key;
			key = DbExtractNextKey(dbName, dbid, oldkey, keylen, &keylen), free(oldkey))
	{
		n ++;
		oldkey = key;
	}

	return n;
}

