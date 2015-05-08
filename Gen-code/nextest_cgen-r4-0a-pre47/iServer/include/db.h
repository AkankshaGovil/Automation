#ifndef _db_h_
#define _db_h_

#include <gdbm.h>
#include "ipckey.h"
#include "alerror.h"
#include "ipc.h"
#include "key.h"

typedef GDBM_FILE DB;

#define DBDATADIR			"/databases"

#define VPNS_DB_FNAME  			"vpns.gdbm" 
#define VPNS_LOCK_FNAME 		"vpns.lock" 
#define NETOIDS_DB_FNAME  		"iedge.gdbm" 
#define NETOIDS_LOCK_FNAME 		"iedge.lock" 
#define VPNG_DB_FNAME			"vpng.gdbm" 
#define VPNG_LOCK_FNAME			"vpng.lock" 
#define FAX_DB_FNAME			"fax.gdbm" 
#define FAX_LOCK_FNAME			"fax.lock" 
#define CALLPLAN_DB_FNAME		"cplan.gdbm" 
#define CALLPLAN_LOCK_FNAME		"callplan.lock"
#define CALLROUTE_DB_FNAME		"croute.gdbm"
#define CALLROUTE_LOCK_FNAME	"callroute.lock"
#define CALLPLANBIND_DB_FNAME	"cbind.gdbm"
#define CALLPLANBIND_LOCK_FNAME	"callbind.lock"
#define ATTRIBS_DB_FNAME  		"attrs.gdbm"
#define ATTRIBS_LOCK_FNAME 		"attrs.lock"
#define TRIGGER_DB_FNAME		"trigger.gdbm"
#define TRIGGER_LOCK_FNAME		"trigger.lock"
#define REALM_DB_FNAME          "realm.gdbm"
#define REALM_LOCK_FNAME        "realm.lock"
#define IGRP_DB_FNAME          	"igrp.gdbm"
#define IGRP_LOCK_FNAME        	"igrp.lock"
#define VNET_DB_FNAME          	"vnet.gdbm"
#define VNET_LOCK_FNAME        	"vnet.lock"

#define DB_LASTUPDATE_FNAME		"dbupdate"

#define VPNS_DB_FILE  			( DBDATADIR "/" VPNS_DB_FNAME )
#define VPNS_LOCK_FILE 			( DBLOCKDIR "/" VPNS_LOCK_FNAME )
#define NETOIDS_DB_FILE  		( DBDATADIR "/" NETOIDS_DB_FNAME )
#define NETOIDS_LOCK_FILE 		( DBLOCKDIR "/" NETOIDS_LOCK_FNAME )
#define VPNG_DB_FILE			( DBDATADIR "/" VPNG_DB_FNAME )
#define VPNG_LOCK_FILE			( DBLOCKDIR "/" VPNG_LOCK_FNAME )
#define FAX_DB_FILE				( DBDATADIR "/" FAX_DB_FNAME )
#define FAX_LOCK_FILE			( DBLOCKDIR "/" FAX_LOCK_FNAME )
#define CALLPLAN_DB_FILE		( DBDATADIR "/" CALLPLAN_DB_FNAME )
#define CALLPLAN_LOCK_FILE		( DBLOCKDIR "/" CALLPLAN_LOCK_FNAME )
#define CALLROUTE_DB_FILE		( DBDATADIR "/" CALLROUTE_DB_FNAME )
#define CALLROUTE_LOCK_FILE		( DBLOCKDIR "/" CALLROUTE_LOCK_FNAME )
#define CALLPLANBIND_DB_FILE	( DBDATADIR "/" CALLPLANBIND_DB_FNAME )
#define CALLPLANBIND_LOCK_FILE	( DBLOCKDIR "/" CALLPLANBIND_LOCK_FNAME )
#define ATTRIBS_DB_FILE  		( DBDATADIR "/" ATTRIBS_DB_FNAME )
#define ATTRIBS_LOCK_FILE 		( DBLOCKDIR "/" ATTRIBS_LOCK_FNAME )
#define TRIGGER_DB_FILE  		( DBDATADIR "/" TRIGGER_DB_FNAME )
#define TRIGGER_LOCK_FILE 		( DBLOCKDIR "/" TRIGGER_LOCK_FNAME )
#define REALM_DB_FILE           ( DBDATADIR "/" REALM_DB_FNAME )
#define REALM_LOCK_FILE         ( DBLOCKDIR "/" REALM_LOCK_FNAME )
#define IGRP_DB_FILE           	( DBDATADIR "/" IGRP_DB_FNAME )
#define IGRP_LOCK_FILE         	( DBLOCKDIR "/" IGRP_LOCK_FNAME )
#define VNET_DB_FILE           	( DBDATADIR "/" VNET_DB_FNAME )
#define VNET_LOCK_FILE         	( DBLOCKDIR "/" VNET_LOCK_FNAME )


#define DB_LASTUPDATE_FILE		( DBDATADIR "/" DB_LASTUPDATE_FNAME )

typedef enum
{
	DB_eNetoids = 0,
	DB_eVpns,
	DB_eVpnG,
	DB_eFax,
	DB_eCallPlan,
	DB_eCallRoute,
	DB_eCallPlanBind,
	DB_eAttribs,
	DB_eTrigger,
    DB_eRealm,
    DB_eIgrp,
    DB_eVnet,

	DB_eMax
} DB_tTypes;

typedef struct
{
	int dbid;	/* OUT */
	DB db;		/* OUT */
	int read_write;	/* IN */
	int mode;	/* IN */
	int lockfd;	/* OUT */
} DB_tDb;

typedef struct
{
     	char 	dbName[24];
     	char 	dbLockName[24];
     	DB_tDb	db;
} DefCommandData, DB_tHandle;

/* Update this structure in memdb.c, if you add a new database */
#define DBNAME(cmdData, type) 	\
				(((DefCommandData *)cmdData)[type].dbName)
#define DBLOCKNAME(cmdData, type) \
				(((DefCommandData *)cmdData)[type].dbLockName)
#define GDBMF(cmdData, type) 	\
				(((DefCommandData *)cmdData)[type].db.db)
#define DBSTRUCT(cmdData, type) (((DefCommandData *)cmdData)[type].db)

/* Macros for vpng entries... */
#define DbStoreVpnGEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(VpnEntry), (char *)skey, skeylen)

#define DbFindVpnGEntry(db, key, keylen) \
                 (VpnGroupEntry *)DbFindEntry(db, key, keylen)

#define DbDeleteVpnGEntry(db, key, keylen) \
                 DbDeleteEntry(db, (char *)key, keylen)

#define DbGetFirstVpnGEntry(db) \
                 (VpnGroupEntry *)DbGetFirstEntry(db)

#define DbGetNextVpnGEntry(db, key, keylen) \
                 (VpnGroupEntry *)DbGetNextEntry(db, (char *)key, keylen)

#define DbGetFirstVpnGKey(db) DbGetFirstKey(db)

#define DbGetNextVpnGKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)

/* Macros for vpn entries... */
#define DbStoreVpnEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(VpnEntry), (char *)skey, skeylen)

#define DbFindVpnEntry(db, key, keylen) \
                 (VpnEntry *)DbFindEntry(db, key, keylen)

#define DbDeleteVpnEntry(db, key, keylen) \
                 DbDeleteEntry(db, (char *)key, keylen)

#define DbGetFirstVpnEntry(db) \
                 (VpnEntry *)DbGetFirstEntry(db)

#define DbGetNextVpnEntry(db, key, keylen) \
                 (VpnEntry *)DbGetNextEntry(db, (char *)key, keylen)

#define DbGetFirstVpnKey(db) DbGetFirstKey(db)

#define DbGetNextVpnKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)

/* Macros for Route entries... */
#define DbStoreRouteEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(RouteEntry), (char *)skey, skeylen)

#define DbFindRouteEntry(db, key, keylen) \
                 (RouteEntry *)DbFindEntry(db, key, keylen)

#define DbDeleteRouteEntry(db, key, keylen) \
                 DbDeleteEntry(db, (char *)key, keylen)

#define DbGetFirstRouteEntry(db) \
                 (RouteEntry *)DbGetFirstEntry(db)

#define DbGetNextRouteEntry(db, key, keylen) \
                 (RouteEntry *)DbGetNextEntry(db, (char *)key, keylen)

#define DbGetFirstRouteKey(db) DbGetFirstKey(db)

#define DbGetNextRouteKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)

/* Macros for CPB entries... */
#define DbStoreCPBEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(CallPlanBindEntry), (char *)skey, skeylen)

#define DbFindCPBEntry(db, key, keylen) \
                 (CallPlanBindEntry *)DbFindEntry(db, key, keylen)

#define DbDeleteCPBEntry(db, key, keylen) \
                 DbDeleteEntry(db, (char *)key, keylen)

#define DbGetFirstCPBEntry(db) \
                 (CallPlanBindEntry *)DbGetFirstEntry(db)

#define DbGetNextCPBEntry(db, key, keylen) \
                 (CallPlanBindEntry *)DbGetNextEntry(db, (char *)key, keylen)

#define DbGetFirstCPBKey(db) DbGetFirstKey(db)

#define DbGetNextCPBKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)

/* Macros for Fax entries... */
#define DbStoreFaxEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(FaxEntry), (char *)skey, skeylen)

#define DbFindFaxEntry(db, key, keylen) \
                 (FaxEntry *)DbFindEntry(db, key, keylen)

#define DbDeleteFaxEntry(db, key, keylen) \
                 DbDeleteEntry(db, (char *)key, keylen)

#define DbGetFirstFaxEntry(db) \
                 (FaxEntry *)DbGetFirstEntry(db)

#define DbGetNextFaxEntry(db, key, keylen) \
                 (FaxEntry *)DbGetNextEntry(db, (char *)key, keylen)

#define DbGetFirstFaxKey(db) DbGetFirstKey(db)

#define DbGetNextFaxKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)


/* Macros for info entries... */
#define DbStoreInfoEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(InfoEntry), (char *)skey, skeylen)

#define DbInsertInfoEntry(db, info, skey, skeylen) \
                 DbInsertEntry(db, (char *)info, sizeof(InfoEntry), (char *)skey, skeylen)

#define DbFindInfoEntry(db, key, keylen) \
                 (InfoEntry *)DbFindEntry(db, key, keylen)

#define DbDeleteInfoEntry(db, key, keylen) \
                 DbDeleteEntry(db, key, keylen)

#define DbGetFirstInfoEntry(db) (InfoEntry *)DbGetFirstEntry(db)

#define DbGetFirstInfoKey(db) DbGetFirstKey(db)

#define DbGetNextInfoEntry(db, key, keylen) \
                 (InfoEntry *)DbGetNextEntry(db, key, keylen)

#define DbGetNextInfoKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)


/*Macros for igrp entries... */
#define DbFindIgrpEntry(db, key, keylen) \
                 (IgrpInfo *)DbFindEntry(db, key, keylen)

/*Macros for vnet entries... */
#define DbFindVnetEntry(db, key, keylen) \
                 (VnetEntry *)DbFindEntry(db, key, keylen)
	
/* Macros for realm entries... */
#define DbStoreRealmEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(RealmEntry), (char *)skey, skeylen)

#define DbInsertRealmEntry(db, info, skey, skeylen) \
                 DbInsertEntry(db, (char *)info, sizeof(RealmEntry), (char *)skey, skeylen)

#define DbFindRealmEntry(db, key, keylen) \
                 (RealmEntry *)DbFindEntry(db, key, keylen)

#define DbDeleteRealmEntry(db, key, keylen) \
                 DbDeleteEntry(db, key, keylen)

#define DbGetFirstRealmEntry(db) (RealmEntry *)DbGetFirstEntry(db)

#define DbGetFirstRealmKey(db) DbGetFirstKey(db)

#define DbGetNextRealmEntry(db, key, keylen) \
                 (RealmEntry *)DbGetNextEntry(db, key, keylen)

#define DbGetNextRealmKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)

/* Actual functions */
AlStatus
DbStoreEntry (DB db, char *entry, int len, char *skey, int skeylen);

AlStatus
DbInsertEntry (DB db, char *dinfo, int dlen, char *skey, int skeylen);

char *
DbFindEntry (DB db, char *key, int keylen);

AlStatus
DbDeleteEntry (DB db, char *key, int keylen);

char *
DbGetFirstEntry (DB db);

char *
DbGetFirstKey (DB db);

char *
DbGetNextEntry (DB db, char *key, int keylen);

char *
DbGetNextKey (DB db, char *skey, int skeylen);

DB
DbOpen(char *, char *, DB_tDb *db);

DB
DbOpenByID(char *, int dbid, DB_tDb *db);

DB
DbTestOpen(char *, char *, DB_tDb *db);

DB
DbTestOpenByID(char *, int dbid, DB_tDb *db);

DB
OpenDBFile(char *);

AlStatus
DbClose (DB_tDb *db);

int
DbStore (DB db, datum key, datum data);

datum
DbFind (DB db, datum key);

AlStatus
DbDelete (DB db, datum key);

/* Outdated stuff */
AlStatus
DbAliasStore(char *alias, char *phone);

char *
DbAliasFind(char *alias);

#ifdef SUNOS
#define LOCK_SH 1    /* Shared lock.  */
#define LOCK_EX 2    /* Exclusive lock.  */
#define LOCK_UN 8    /* Unlock.  */
#define LOCK_NB	4	/* Don't block when locking.  */

#endif

int
StoreVpn(DB db, VpnEntry *,
	char *oldVpnGroup, char *newVpnGroup);

int
StoreVpnG(DB db, VpnGroupEntry *);

/* Backward compatibility */
#define defCommandData	dbHandles
extern DB_tHandle dbHandles[];

/* Macros for attr entries... */
#define DbStoreAttrEntry(db, info, skey, skeylen) \
                 DbStoreEntry(db, (char *)info, sizeof(ClientAttribs), (char *)skey, skeylen)

#define DbFindAttrEntry(db, key, keylen) \
                 (ClientAttribs *)DbFindEntry(db, key, keylen)

#define DbDeleteAttrEntry(db, key, keylen) \
                 DbDeleteEntry(db, key, keylen)

#define DbGetFirstAttrEntry(db) (ClientAttribs *)DbGetFirstEntry(db)

#define DbGetFirstAttrKey(db) DbGetFirstKey(db)

#define DbGetNextAttrEntry(db, key, keylen) \
                 (ClientAttribs *)DbGetNextEntry(db, key, keylen)

#define DbGetNextAttrKey(db, skey, skeylen) \
                 DbGetNextKey (db, skey, skeylen)

char * DbExtractNextEntry (char *dbName, int dbid, char *skey, int skeylen);
char * DbExtractFirstEntry (char *dbName, int dbid);
char * DbExtractEntry (char *dbName, int dbid, char *skey, int skeylen);

AlStatus DbUpdateEntry (char *dbName, int dbid, char *dinfo, int dlen, char *skey, int skeylen);
AlStatus DbRemoveEntry (char *dbName, int dbid, char *key, int keylen);
int DbDeleteLocks (void);
int DbCreateLocks (void);
int DbID (char *dbname);
int DbLockID (char *lockname);
int DbUpdate (void);
int DbCount (DB db);

#endif /* _db_h_ */
