#include "unp.h"
#include "serverp.h"
#include "srvrlog.h"
#include "dsa.h"
#include "db.h"

MemoryMap 	*map;
LsMemStruct	*lsMem;
extern char 	config_file[];
extern char 	pidfile[];

/* Dependecies of libcrypto */
unsigned char *
SHA1(unsigned char *d, unsigned long n, unsigned char *md)
{
	char fn[] = "SHA1";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

unsigned char *
MD5(unsigned char *d, unsigned long n, unsigned char *md)
{
	char fn[] = "MD5";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

void
DSA_free (DSA *r)
{
	char fn[] = "DSA_free";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

}

DSA *
DSA_new (void)
{
	char fn[] = "DSA_new";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

int
DSA_verify(int type,unsigned char *dgst,int dgst_len,                            
			unsigned char *sigbuf, int siglen, DSA *dsa)
{
	char fn[] = "DSA_verify";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(0);
}

BIGNUM *
BN_bin2bn(unsigned char *s, int len, BIGNUM *ret)
{
	char fn[] = "BN_bin2fn";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

/* Dependencies of libfceconfig.a */
#if 0
int     allowDestAll = 0;
int 	allowSrcAll = 0;
int     routeDebug = 0;
int 	max_segs = 6;
int     max_segsize = 0x100000;
int     realTimeEnable = 1;
int     routecall = 1; 
int     cacheTimeout;
int     sharecall = 1;
int     segaddrtype = 0;
int 	xthreads = 2;

char *
FormatIpAddress(unsigned int ipAddress, char s[])
{
	sprintf(s, "%u.%u.%u.%u",
		(ipAddress & 0xff000000) >> 24,
		(ipAddress & 0x00ff0000) >> 16,
		(ipAddress & 0x0000ff00) >> 8,
		(ipAddress & 0x000000ff));
	return s;
}

#endif
#if 0
/* Dependencies of libslocks.a */
int
spthread_init(void)
{
	char fn[] = "spthread_init";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

int
spthread_exit(void)
{
	char fn[] = "spthread_exit";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}
#endif 

#if 0
/* Dependencies of libsrvr.a */
char *
ULIPtostring(unsigned long ipaddress)
{
	char fn[] = "spthread_exit";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}
#endif

/* Dependencies of libutils.a */
#if 0
int
ERROR_PrintInfoEntry(int module, InfoEntry *infoentry)
{
	char fn[] = "ERROR_PrintInfoEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

int
DEBUG_PrintInfoEntry(int module, int level, InfoEntry *infoentry)
{
	char fn[] = "DEBUG_PrintInfoEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

int
DEBUG_PrintPhoNode(int module, int level, PhoNode *node)
{
	char fn[] = "DEBUG_PrintPhoNode";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}
#endif

#if 0
/* Dependencies of libdb.a */
AlStatus
DbStoreEntry (DB db, char *entry, int len, char *skey, int skeylen)
{
	char fn[] = "DbStoreEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}


AlStatus
DbInsertEntry (DB db, char *dinfo, int dlen, char *skey, int skeylen)
{
	char fn[] = "DbInsertEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char *
DbFindEntry (DB db, char *key, int keylen)
{
	char fn[] = "DbFindEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

AlStatus
DbDeleteEntry (DB db, char *key, int keylen)
{
	char fn[] = "DbDeleteEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char *
DbGetFirstEntry (DB db)
{
	char fn[] = "DbGetFirstEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char *
DbGetFirstKey (DB db)
{
	char fn[] = "DbGetFirstKey";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char *
DbGetNextEntry (DB db, char *key, int keylen)
{
	char fn[] = "DbGetNextEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char *
DbGetNextKey (DB db, char *skey, int skeylen)
{
	char fn[] = "DbGetNextKey";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

DB
DbOpen(char *a, char *b, DB_tDb *db)
{
	char fn[] = "DbOpen";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

DB
DbOpenByID(char *a, int dbid, DB_tDb *db)
{
	char fn[] = "DbOpenByID";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

DB
DbTestOpen(char *a, char *b, DB_tDb *db)
{
	char fn[] = "DbTestOPen";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

DB
DbTestOpenByID(char *a, int dbid, DB_tDb *db)
{
	char fn[] = "DBTestOpenByID";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

AlStatus
DbClose (DB_tDb *db)
{
	char fn[] = "DbClose";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

int
DbStore (DB db, datum key, datum data)
{
	char fn[] = "DbStore";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

datum
DbFind (DB db, datum key)
{
	char fn[] = "DBFind";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(key);
}

AlStatus
DbDelete (DB db, datum key)
{
	char fn[] = "DbDelete";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

AlStatus
DbAliasStore(char *alias, char *phone)
{
	char fn[] = "DbAliasStore";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char *
DbAliasFind(char *alias)
{
	char fn[] = "DbAliasFind";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char * DbExtractNextEntry (char *dbName, int dbid, char *skey, int skeylen)
{
	char fn[] = "DbExtractnextEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char * DbExtractFirstEntry (char *dbName, int dbid)
{
	char fn[] = "DbExtractFirstEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

char * DbExtractEntry (char *dbName, int dbid, char *skey, int skeylen)
{
	char fn[] = "DbExtractEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

AlStatus 
DbUpdateEntry (char *dbName, int dbid, char *dinfo, int dlen, char *skey, int skeylen)
{
	char fn[] = "DbUpdateEntry";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

int
DbCreateLocks(void)
{
	char fn[] = "DbCreateLocks";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}

int
DbDeleteLocks(void)
{
	char fn[] = "DbDeleteLocks";

	NETERROR(MRSD, ("Called function %s, which was not linked\n", fn));

	return(NULL);
}
#endif
