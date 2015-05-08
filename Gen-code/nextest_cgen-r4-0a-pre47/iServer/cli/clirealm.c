#include "cli.h"
#include "serverp.h"
#include "avl.h"
#include "ifs.h"
#include "qmsg.h"
#include "gisq.h"
#include <sys/time.h>
#include <strings.h>
#include "log.h"
#include "ipstring.h"

#define IsRealmOnLif(x)       (strchr(x->vipName, IF_DELIM_VLAN) || strchr(x->vipName, IF_DELIM_NORMAL))
#define IsIfLif(x)            (strchr(x, IF_DELIM_VLAN) || strchr(x, IF_DELIM_NORMAL))

int
HandleRealmAdd(Command *comm, int argc, char **argv)
{
   	char fn[] = "HandleRealmAdd():";
    DB_tDb  dbstruct;
    DB  db;
	RealmEntry realmEntry = {0};
    RealmKey   rmKey = { 0 };
    int rc = xleOk;
    unsigned short  flags;
     
   	if (argc < 1)
   	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

	if (strlen(argv[0]) >= REALM_NAME_LEN)
	{
        CLIPRINTF((stdout, "Realm name too long:[%s]\n", argv[0]));
		return -xleInvalArgs;
	}

	nx_strlcpy(rmKey.realmNameKey, argv[0], REALM_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
        return -xleInvalArgs;
    }

    if (DbFindEntry(db, (char *)&rmKey, sizeof(RealmKey)))
    {
        CLIPRINTF((stdout, 
            "entry (%s) already exists\n", rmKey.realmNameKey));
        rc = -xleExists;
        goto _return;
    }

    nx_strlcpy(realmEntry.realmName, argv[0], REALM_NAME_LEN);
 	realmEntry.realmId = GenRealmID(realmEntry.realmName);
	realmEntry.adminStatus = 1; /* default */
	
    // Explicitly set mirror proxy uport to -1 to avoid accidental mp feature invocationx
    realmEntry.mp.uport = -1;
    strcpy(realmEntry.vnetName, VNET_UNASSIGNED);

    if (DbStoreEntry(db, (char *)&realmEntry, sizeof(RealmEntry), 
                (char *)&rmKey, sizeof(RealmKey))<0)
    {
        CLIPRINTF((stdout, "database store error %d\n", errno));
        rc = -xleOpNoPerm;
        goto _return;
    }

	if (CacheAttach() > 0)
    {
        CacheHandleRealm(&realmEntry, CLIOP_ADD);
        CacheDetach();
    }

    /* Handle dynamically marking existing elements from default 
     * realm into this realm
     */
_return:
    DbClose(&dbstruct); 
   	return rc;
}

int
HandleRealmDelete(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleRealmDelete():";
    DB_tDb  dbstruct;
    DB db;
    RealmKey rmKey = { 0 };
    RealmEntry *realmEntryPtr = NULL;
    int rc = xleOk;
    VnetEntry vnetEntry = {"","",0,0,0};
    VnetEntry *vnetEntryPtr = NULL;
    unsigned long realmID = 0;
    unsigned long realmIP = 0;
    unsigned long realmMask = 0;
    int realmVlanid = VLANID_NONE;
    int doRealmSubnetProc = 0;
    char lifname[IFI_NAME] = "";

    if (argc < 1)
    {
        HandleCommandUsage(comm, argc, argv);
        return -xleInsuffArgs;
    }

    nx_strlcpy(rmKey.realmNameKey, argv[0], REALM_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
        return -xleInvalArgs;
    }

    if (!(realmEntryPtr = (RealmEntry *)DbFindEntry(db, (char *)&rmKey,
                    sizeof(RealmKey))))
    {
        CLIPRINTF((stdout,"entry (%s) does not exists %d\n", rmKey.realmNameKey, errno));
        rc = -xleNoEntry;
        goto _return;
    }

    if (strcmp(realmEntryPtr->vnetName, VNET_UNASSIGNED) &&
        (GetVnetInfo(realmEntryPtr->vnetName, &vnetEntry) == 0))
    {
        vnetEntryPtr = &vnetEntry;
    }

#ifdef NETOID_LINUX
    if (realmEntryPtr->vipName[0] && (IsPrimaryInterface(realmEntryPtr->vipName, realmEntryPtr->rsa) > 0))
    {
      realmID = realmEntryPtr->realmId;
      realmIP = realmEntryPtr->rsa;
      realmMask = realmEntryPtr->mask;
      realmVlanid = vnetEntryPtr ? vnetEntryPtr->vlanid : VLANID_NONE;
      strcpy(lifname, realmEntryPtr->vipName);
      doRealmSubnetProc = 1;
    }
#endif

    RealmCloseVip(realmEntryPtr, vnetEntryPtr);

    /* Send msg to GIS to stop listening on  5060 and 17xx on old RSA */
    RealmChangeVipSig(realmEntryPtr, 0);

    if (DbDeleteEntry(db, (char *)&rmKey, sizeof(RealmKey)) < 0)
    {
        CLIPRINTF((stdout, "database delete error %d\n", errno));
        rc = -xleOpNoPerm;
        goto _return;
    }

    // Update the cache also
    if (CacheAttach() > 0)
    {
        CacheHandleRealm(realmEntryPtr, CLIOP_DELETE);
        CacheDetach();
    }

    /* Handle Orphaned netoid entries */
_return:
    DbClose(&dbstruct);
    free(realmEntryPtr);

    if (doRealmSubnetProc)
    {
      RealmCloseAndOpenSubnetVips(realmID, lifname, realmIP, realmMask, realmVlanid);
    }

    return rc;
}

int
HandleRealmList(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleRealmList():";
    DB_tDb dbstruct;
    DB db;
    RealmEntry *rmPtr, *oldrmPtr;
    int rc = xleOk;
    int n=0;
    rmPtr = (RealmEntry *)DbExtractFirstEntry(REALM_DB_FILE, DB_eRealm);
    while(rmPtr)
    {
        PrintRealmEntry(stdout, rmPtr);
        CLIPRINTF((stdout, "\n"));
        n ++;
        oldrmPtr = rmPtr;
        rmPtr = (RealmEntry *)DbExtractNextEntry(REALM_DB_FILE,
        DB_eRealm, (char *)oldrmPtr->realmName, sizeof(RealmKey));
        free(oldrmPtr);
    }
    CLIPRINTF((stdout, "%d realm entries\n", n));
    return rc;
}

int
HandleRealmEdit(Command *comm, int argc, char **argv)
{
   	char fn[] = "HandleRealmEdit()";
    DB_tDb  dbstruct = { 0 };
    DB db = { 0 };
    RealmEntry  *realmEntryPtr = NULL;
    RealmEntry	oldrm = {0};
    RealmKey     rmKey = { 0 };
    unsigned long   oldRsa,newRsa;
    int rc = xleOk;
    struct ifi_info  *ifi_head, *ifp;
	unsigned char oldIfName[IFI_NAME];
	unsigned int	newOperStat ;
	CacheRealmEntry *ptr, *newptr;
	unsigned long realmID = 0;
        unsigned long realmIP = 0;
        unsigned long realmMask = 0;
        int realmVlanid = VLANID_NONE;
        int doRealmSubnetProc = 0;
        char lifname[IFI_NAME] = "";
	VnetEntry vnetEntry = {"","",0,0,0};
	VnetEntry *vnetEntryPtr = NULL;

   	if (argc < 1)
   	{
	  	RealmEditHelp(comm, argc, argv);
	  	rc = -xleInsuffArgs;
		return rc;
   	}

    // cli realm edit commands should not be excerciseable when 
    // gis is down.	
    if( CacheAttach() < 0 ) 
    {
	  
      CLIPRINTF((stdout, "iServer is not running - Realm edit operation disabled \n"));
      rc = -xleOpNoPerm;
      goto _return;
    }
    // gis is up and running - so let the normal realm edit continue
    CacheDetach();


    nx_strlcpy (rmKey.realmNameKey, argv[0], REALM_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
        rc = -xleInvalArgs;
		goto _return;
    }

    if (!(realmEntryPtr = (RealmEntry *)DbFindEntry(db, 
								 (char *)&rmKey, sizeof(RealmKey)) ))
    {
        CLIPRINTF((stdout, "entry (%s) does not exist\n", rmKey.realmNameKey));
        rc = -xleNoEntry;
        goto _return;
    }

    argc -= 1;
    argv += 1;

    if (argc <= 0)
    {
        goto _return;
    }

	memcpy(&oldrm, realmEntryPtr,sizeof(RealmEntry));

	CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
	ptr = CacheGet(realmCache, &realmEntryPtr->realmId);

	/* carry over the status of interface that was marked in GIS and 
	 * never stored in Db entry, only in Cache Entry 
	 */
	if(ptr)
	{
	  oldrm.operStatus = ptr->realm.operStatus;
	}
	CacheReleaseLocks(realmCache);

	GetAttrPairs(comm->name, &argc, &argv, realmEntryPtr, CLI_GET_ATTR_REALM);

#ifdef NETOID_LINUX
         // Get the VNET pointer
         if (strcmp(oldrm.vnetName, VNET_UNASSIGNED) &&
             (GetVnetInfo(oldrm.vnetName, &vnetEntry) == 0))
         {
             vnetEntryPtr = &vnetEntry;
         }

        if (vnetEntryPtr && oldrm.vipName[0] && (IsPrimaryInterface(oldrm.vipName, oldrm.rsa) > 0))
        {
          realmID = realmEntryPtr->realmId;
          realmIP = oldrm.rsa;
          realmMask = oldrm.mask;
          realmVlanid = vnetEntryPtr ? vnetEntryPtr->vlanid : VLANID_NONE;
          strcpy(lifname, oldrm.vipName);
          doRealmSubnetProc = 1;
        }
#endif

	RealmEditHandler(realmEntryPtr, &oldrm);

    if (CacheAttach() > 0)
    {
        CacheHandleRealm(realmEntryPtr, CLIOP_REPLACE);
        CacheDetach();
    }

	if ((realmEntryPtr->rsa == oldrm.rsa) &&
		(!strcmp(realmEntryPtr->vnetName, oldrm.vnetName)) &&
		(realmEntryPtr->mask == oldrm.mask))
	{
          doRealmSubnetProc = 0;
	}

	if ((realmEntryPtr->rsa != oldrm.rsa) ||
		(strcmp(realmEntryPtr->vnetName, oldrm.vnetName)) ||
		(realmEntryPtr->mask!= oldrm.mask) ||
		(realmEntryPtr->sigPoolId != oldrm.sigPoolId) ||
		(realmEntryPtr->medPoolId != oldrm.medPoolId) ||
		(realmEntryPtr->interRealm_mr!= oldrm.interRealm_mr) ||
		(realmEntryPtr->intraRealm_mr!= oldrm.intraRealm_mr)) 
	{
		/* start signalling only if rsa changed and adminStatus==1 */
		RealmChangeVipSig(realmEntryPtr, realmEntryPtr->adminStatus);
	}

	CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
	newptr = CacheGet(realmCache, &realmEntryPtr->realmId);
	newOperStat = newptr->realm.operStatus;
	CacheReleaseLocks(realmCache);

	/* Gis sets operStatus to 1 or does not touch it */
	/* bring Db and Cache in sync */
	realmEntryPtr->operStatus = newOperStat;

    /* Store the entry */
    if (DbStoreEntry(db, (char *)realmEntryPtr, sizeof(RealmEntry),
                (char *)realmEntryPtr->realmName, sizeof(RealmKey)) < 0)
    {
        CLIPRINTF((stdout,"database store error %d\n", errno));
        rc = -xleOpNoPerm;
        goto _return;
    }

_return:
    DbClose(&dbstruct);
	if (realmEntryPtr)
	{
    	free(realmEntryPtr);
	}

        if (doRealmSubnetProc)
        {
          RealmCloseAndOpenSubnetVips(realmID, lifname, realmIP, realmMask, realmVlanid);
        }

    return rc;
}

int
HandleRealmCache(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleRealmCache():";
	CacheRealmEntry *cacheRealmEntry;
	unsigned int n=0;

	if (CacheAttach() > 0)
	{
		CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);
	
		for (cacheRealmEntry = CacheGetFirst(realmCache);
            cacheRealmEntry;
            cacheRealmEntry = CacheGetNext(realmCache, 
                                &cacheRealmEntry->realm.realmId))
		{
	  		PrintRealmEntry(stdout, &cacheRealmEntry->realm);
//			CLIPRINTF((stdout, "socket id %d\n", cacheRealmEntry->socketId));
			n++;
		}

		CacheReleaseLocks(realmCache);
		CacheDetach();
	}
    else
	{
		 CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
         return -1;
	}

	CLIPRINTF((stdout, "\n%d Realm%s\n\n", n, n==1?"":"s"));

    return 0;
}

int
HandleRealmLkup(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleRealmLkup():";
	CacheRealmEntry *cacheRealmEntry;
    unsigned long rsaValue;
    unsigned int   n=0;

    if (argc < 1)
    {
		HandleCommandUsage(comm, argc, argv);
		return -xleInsuffArgs;
    }

    rsaValue = StringToIp(argv[0]);

    if (CacheAttach() > 0)
    {
        CLIPRINTF((stdout, "Realm Cache...\n"));
     
		CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

		for (cacheRealmEntry = CacheGetFirst(realmCache);
			 cacheRealmEntry;
			 cacheRealmEntry = CacheGetNext(realmCache, &cacheRealmEntry->realm.realmId))
		{
            if (rsaValue == cacheRealmEntry->realm.rsa)
            {
                n++;
                fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++++++++++\n");
                fprintf(stdout, "Realm Match %d:\n", n);
                fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++++++++++\n");

                PrintRealmEntry(stdout, &cacheRealmEntry->realm);
            }
        }
        CacheReleaseLocks(realmCache);
        CacheDetach();
	}
    else
	{
		CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
        return -1;
	}

    if (n) { CLIPRINTF((stdout, "Total Matches %d\n", n)); }
    else { CLIPRINTF((stdout, "No Matches found\n")); }

    return 0;
}

int
HandleRealmUp(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleRealmUp():";
    DB_tDb  dbstruct;
    DB db;
    RealmKey rmKey = { 0 };
    RealmEntry *realmEntryPtr = NULL;
    char all=0;
    int rc = xleOk;

    if (argc < 1)
    {
        HandleCommandUsage(comm, argc, argv);
        return -xleInsuffArgs;
    }

	if (CacheAttach() > 0)
	{ 
		CacheDetach(); 
	}
	else
	{ 
		return -1; 
	}

    if (strcasecmp(argv[0], "all"))
    {

        nx_strlcpy(rmKey.realmNameKey, argv[0], REALM_NAME_LEN);

        dbstruct.read_write = GDBM_WRCREAT;
        if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
        {
            CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
            return -xleInvalArgs;
        }

        if (!(realmEntryPtr = (RealmEntry *)DbFindEntry(db, (char *)&rmKey,
                        sizeof(RealmKey))))
        {
            CLIPRINTF((stdout,"entry (%s) does not exists %d\n", rmKey.realmNameKey, errno));
            rc = -xleNoEntry;
            goto _return;
        }

        RealmVipUpDown(realmEntryPtr, 1); /* UP = 1 */

        if (DbStoreEntry(db, (char *)realmEntryPtr, sizeof(RealmEntry),
                    (char *)realmEntryPtr->realmName, sizeof(RealmKey)) < 0)
        {
            CLIPRINTF((stdout,"database store error %d\n", errno));
            rc = -xleOpNoPerm;
            goto _return;
        }

        if (CacheAttach() > 0)
        {
            CacheHandleRealm(realmEntryPtr, CLIOP_REPLACE);
            CacheDetach();
        }

    }
    else
    {
        RealmAllVips(REALM_OPER_UP);
        return 0;
    }
_return:
    DbClose(&dbstruct);
    return 0;
}


int
HandleRealmDown(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleRealmDown ():";
    DB_tDb  dbstruct;
    DB db;
    RealmKey rmKey = { 0 };
    RealmEntry *realmEntryPtr = NULL;
    char all=0;
    int rc = xleOk;

    if (argc < 1)
    {
        HandleCommandUsage(comm, argc, argv);
        return -xleInsuffArgs;
    }

    if (strcasecmp(argv[0], "all"))
    {

        nx_strlcpy(rmKey.realmNameKey, argv[0], REALM_NAME_LEN);

        dbstruct.read_write = GDBM_WRCREAT;
        if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
        {
            CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
            return -xleInvalArgs;
        }

        if (!(realmEntryPtr = (RealmEntry *)DbFindEntry(db, (char *)&rmKey,
                        sizeof(RealmKey))))
        {
            CLIPRINTF((stdout,"entry (%s) does not exists %d\n", rmKey.realmNameKey, errno));
            rc = -xleNoEntry;
            goto _return;
        }

        RealmVipUpDown(realmEntryPtr, 0); /* DOWN = 0 */

        if (DbStoreEntry(db, (char *)realmEntryPtr, sizeof(RealmEntry),
                    (char *)realmEntryPtr->realmName, sizeof(RealmKey)) < 0)
        {
            CLIPRINTF((stdout,"database store error %d\n", errno));
            rc = -xleOpNoPerm;
            goto _return;
        }

        if (CacheAttach() > 0)
        {
            CacheHandleRealm(realmEntryPtr, CLIOP_REPLACE);
            CacheDetach();
        }

    }
    else
    {
        RealmAllVips(REALM_OPER_DOWN);
        return 0;
    }
_return:
    DbClose(&dbstruct);
    return 0;
}

int
HandleRealmOpen(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleRealmOpen():";
    DB_tDb  dbstruct;
    DB db;
    RealmKey rmKey = { 0 };
    RealmEntry *realmEntryPtr = NULL;
    char all=0;
    int rc = xleOk;
    VnetEntry vnetEntry = {"","",0,0,0};
    VnetEntry *vnetEntryPtr = NULL;


    if (argc < 1)
    {
        HandleCommandUsage(comm, argc, argv);
        return -xleInsuffArgs;
    }

    if (strcasecmp(argv[0], "all"))
    {

        nx_strlcpy(rmKey.realmNameKey, argv[0], REALM_NAME_LEN);

        dbstruct.read_write = GDBM_WRCREAT;
        if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
        {
            CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
            return -xleInvalArgs;
        }

        if (!(realmEntryPtr = (RealmEntry *)DbFindEntry(db, (char *)&rmKey,
                        sizeof(RealmKey))))
        {
            CLIPRINTF((stdout,"entry (%s) does not exists %d\n", rmKey.realmNameKey, errno));
            rc = -xleNoEntry;
            goto _return;
        }

        if (strcmp(realmEntryPtr->vnetName, VNET_UNASSIGNED) &&
            (GetVnetInfo(realmEntryPtr->vnetName, &vnetEntry) == 0))
        {
            vnetEntryPtr = &vnetEntry;
        }

        if (RealmOpenVip(realmEntryPtr, vnetEntryPtr)< 0)
        {
            goto _return;
        }

        /* Store the entry */
        if (DbStoreEntry(db, (char *)realmEntryPtr, sizeof(RealmEntry),
                    (char *)realmEntryPtr->realmName, sizeof(RealmKey)) < 0)
        {
            CLIPRINTF((stdout,"database store error %d\n", errno));
            rc = -xleOpNoPerm;
            goto _return;
        }

        if (CacheAttach() > 0)
        {
            CacheHandleRealm(realmEntryPtr, CLIOP_REPLACE);
            CacheDetach();
        }

    }
    else
    {
        RealmAllVips(REALM_OPER_ADD);
        return 0;
    }
_return:
    DbClose(&dbstruct);
    return 0;
}

int
HandleRealmClose(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleRealmClose():";
    DB_tDb  dbstruct;
    DB db;
    RealmKey rmKey = { 0 };
    RealmEntry *realmEntryPtr = NULL;
    char all=0;
    int rc = xleOk;
    struct ifi_info *ifi_head, *ifp;
    VnetEntry vnetEntry = {"","",0,0,0};
    VnetEntry *vnetEntryPtr = NULL;
    unsigned long realmID = 0;
    unsigned long realmIP = 0;
    unsigned long realmMask = 0;
    int realmVlanid = VLANID_NONE;
    int doRealmSubnetProc = 0;
    char lifname[IFI_NAME] = "";

    if (argc < 1)
    {
        HandleCommandUsage(comm, argc, argv);
        return -xleInsuffArgs;
    }

    if (strcasecmp(argv[0], "all"))
    {

        nx_strlcpy(rmKey.realmNameKey, argv[0], REALM_NAME_LEN);

        dbstruct.read_write = GDBM_WRCREAT;
        if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
        {
            CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
            return -xleInvalArgs;
        }

        if (!(realmEntryPtr = (RealmEntry *)DbFindEntry(db, (char *)&rmKey,
                        sizeof(RealmKey))))
        {
            CLIPRINTF((stdout,"entry (%s) does not exists %d\n", rmKey.realmNameKey, errno));
            rc = -xleNoEntry;
            goto _return;
        }

	if (strcmp(realmEntryPtr->vnetName, VNET_UNASSIGNED) &&
	    (GetVnetInfo(realmEntryPtr->vnetName, &vnetEntry) == 0))
	{
		vnetEntryPtr = &vnetEntry;
	}

#ifdef NETOID_LINUX
        if (realmEntryPtr->vipName[0] && (IsPrimaryInterface(realmEntryPtr->vipName, realmEntryPtr->rsa) > 0))
        {
          realmID = realmEntryPtr->realmId;
          realmIP = realmEntryPtr->rsa;
          realmMask = realmEntryPtr->mask;
          realmVlanid = vnetEntryPtr ? vnetEntryPtr->vlanid : VLANID_NONE;
          strcpy(lifname, realmEntryPtr->vipName);
          doRealmSubnetProc = 1;
        }
#endif

        RealmCloseVip(realmEntryPtr, vnetEntryPtr);

        /* Store the entry */
        if (DbStoreEntry(db, (char *)realmEntryPtr, sizeof(RealmEntry),
                    (char *)realmEntryPtr->realmName, sizeof(RealmKey)) < 0)
        {
            CLIPRINTF((stdout,"database store error %d\n", errno));
            rc = -xleOpNoPerm;
            goto _return;
        }

        if (CacheAttach() > 0)
        {
            CacheHandleRealm(realmEntryPtr, CLIOP_REPLACE);
            CacheDetach();
        }

    }
    else
    {
        RealmAllVips(REALM_OPER_REMOVE);
        return 0;
    }
_return:
    DbClose(&dbstruct);

    if (doRealmSubnetProc)
    {
      RealmCloseAndOpenSubnetVips(realmID, lifname, realmIP, realmMask, realmVlanid);
    }

    return 0;
}

/*
 * post_processing is 1 for the case of adding single interface
 */
int
RealmOpenVip(RealmEntry *rmEntry, VnetEntry *vnetEntry)
{
    unsigned long nm, rc = -1, flags;

    if (vnetEntry && vnetEntry->ifName[0] && rmEntry->rsa) 
	{
		rc = PlumbIf(vnetEntry->ifName, rmEntry->rsa, 
				rmEntry->mask, rmEntry->vipName, vnetEntry->vlanid, vnetEntry->rtgTblId);
		if (rc >= 0)
		{
			nm = GetIfIpMask(rmEntry->vipName);
			if (nm && rmEntry->mask && 
				(rmEntry->mask != nm))
			{
				CLIPRINTF((stdout, "Failed to set netmask for realm[%s] on interface[%s]\n", 
					rmEntry->realmName, rmEntry->vipName));
			}	
			else
			{
				rmEntry->mask = nm;
			}

			flags = GetIfFlags(rmEntry->vipName);
			rmEntry->operStatus = (flags & IFF_UP)? 1 : 0;
			if (vnetEntry->gateway)
			{
			  RealmEditVnetGateway(vnetEntry, 0);
			}
		}
    }
    return 0;
}


int
RealmCloseVip(RealmEntry *rmEntry, VnetEntry *vnetEntry)
{
	int 	rc = 0;

    if (vnetEntry && rmEntry->vipName[0] && IsRealmOnLif(rmEntry))
    {
        rc = UnplumbIf(rmEntry->vipName, rmEntry->rsa, vnetEntry->rtgTblId);
	if (rc >= 0)
        {
            memset(rmEntry->vipName, '\0', IFI_NAME);
            rmEntry->operStatus = 0;
        }
    }
	return rc;
}

int 
RealmVipUpDown(RealmEntry *rmEntry, unsigned short new_status)
{
    unsigned long curr_status, flags;

    /* Is if plumbed */
    if (rmEntry->vipName[0] && IsRealmOnLif(rmEntry))
    {
        flags = GetIfFlags(rmEntry->vipName);
        curr_status = (flags & IFF_UP) ?  1 : 0;
        if (curr_status != new_status)
        {
			StatusChgIf(rmEntry->vipName, new_status);
            flags = GetIfFlags(rmEntry->vipName);
            rmEntry->operStatus = (flags & IFF_UP) ?  1 : 0;
        }
        else
        {
            CLIPRINTF((stdout, "Realm %s (%s) already in state %s\n", 
                            rmEntry->realmName, rmEntry->vipName, 
                            curr_status? "UP" : "DOWN"));
        }
    }

    return 0;
}

void
GetParentIfName(char *lifn, char *pifn)    
{
    /* special case copy with a definite colon in the string */
    while (*lifn != IF_DELIM_VLAN && *lifn != IF_DELIM_NORMAL)
    {
        *pifn = *lifn;
        lifn++; pifn++;
    }
}

/*
 * RealmVipAddRemoveall
 *      Single function to addif/removeif all the VIP ifs
 * op : 0 (removeif all) / 1 (addif all)
 */
int
RealmAllVips(unsigned short op)
{
    RealmEntry  *rmPtr, *oldrmPtr;
    DB_tDb  dbstruct;
    DB  db;
    RealmEntry *out;
	RealmEntry    in={0};
	unsigned long	ifipaddr=0;
    struct ifi_info  *iflist1 = NULL, *iflist2 = NULL, *ifp = NULL;
    VnetEntry vnetEntry = {"","",0,0,0};
    VnetEntry *vnetEntryPtr = NULL;

	if ((op == REALM_OPER_ADD)  || (op == REALM_OPER_UP))
	{
		if (CacheAttach() > 0)
		{ 
			CacheDetach();
		}
	}

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
        return -xleInvalArgs;
    }

    for (rmPtr = (RealmEntry *)DbGetFirstEntry(db); 
        rmPtr;
        oldrmPtr = rmPtr, vnetEntryPtr = NULL,
        rmPtr = (RealmEntry *)DbGetNextEntry(db, 
											(char *)oldrmPtr->realmName, 
											sizeof(RealmKey)))
    {
        strcpy(vnetEntry.ifName, "");
        switch(op)
        {
            case REALM_OPER_ADD:
				if (strcmp(rmPtr->vnetName, VNET_UNASSIGNED) &&
		    	    	    (GetVnetInfo(rmPtr->vnetName, &vnetEntry) == 0))
				{
					vnetEntryPtr = &vnetEntry;
				}

				RealmOpenVip(rmPtr, vnetEntryPtr); 
				rmPtr->operStatus = GetIfFlags(rmPtr->vipName);
				rmPtr->mask = GetIfIpMask(rmPtr->vipName);
                break;

            case REALM_OPER_REMOVE:
				if (strcmp(rmPtr->vnetName, VNET_UNASSIGNED) &&
		    	    	    (GetVnetInfo(rmPtr->vnetName, &vnetEntry) == 0))
				{
					vnetEntryPtr = &vnetEntry;
				}

				RealmCloseVip(rmPtr, vnetEntryPtr);
				break;

            case REALM_OPER_UP:
            case REALM_OPER_DOWN:
				RealmVipUpDown(rmPtr, (op == REALM_OPER_UP ? 1 : 0));
                break;
        }

		DbStoreEntry(db, (char *)rmPtr, sizeof(RealmEntry), 
							rmPtr->realmName, sizeof(RealmKey));
		if (CacheAttach() > 0)
		{
			CacheHandleRealm(rmPtr, CLIOP_REPLACE);
			CacheDetach();
		}
    }

#if 0
  /* In a redundant system  ISPD is monitoring the health of
   * the VIP. deleting the VIP in the following code makes ISPD
   * exit
   */
    if (op == REALM_OPER_ADD)
    {
        iflist2 = initIfs2(1);
        for (ifp = iflist2; ifp; ifp=ifp->ifi_next) 
        {
			ifipaddr = 0; out=NULL;
            ifipaddr = ntohl(ifp->ifi_addr->sin_addr.s_addr);
            if ((out = (RealmEntry *)CacheGet(rsaCache, &ifipaddr)) 
																== NULL)
			{
				UnplumbIf(ifp->ifi_name, 0, 0);
            }
        }
		freeIfs(iflist2);
    }
#endif

// ALREADY CLOSED ALL THE VIPs ABOVE, WHY HERE????
	if (op == REALM_OPER_REMOVE)
	{
        iflist2 = initIfs2(1);
        for (ifp = iflist2; ifp; ifp=ifp->ifi_next) 
        {
			UnplumbIf(ifp->ifi_name, ifp->ifi_addr->sin_addr.s_addr, 0);
		}
		freeIfs(iflist2);
	}

    DbClose(&dbstruct);
    return 0;
}

void
RealmSetAdminStatus(unsigned long *rsa, unsigned int status)
{
	CacheRealmEntry *ptr;

	CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
	ptr = CacheGet(rsaCache, rsa);
	if (ptr)
	{
		ptr->realm.adminStatus =  status;
	}
	CacheReleaseLocks(realmCache);
	return ;
}


int
RealmChangeVipSig(RealmEntry *rmEntry, unsigned int status)
{
	char fn[] = "RealmChangeVipSig():";
	QDesc sqdesc, dqdesc;
	time_t now;
	char qgismsg[sizeof(QMsgHdr)+sizeof(unsigned long)];
	QMsgHdr *qmsghdr;
	unsigned long rsan;
	int rc = xleOk;

	if (QGet(&dqdesc, ISERVER_GIS_Q, GIS_SRVR_MSG_TYPE) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

	time(&now);

	if (QGet(&sqdesc, ISERVER_GIS_Q, now) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue from cli\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

	RealmSetAdminStatus(&rmEntry->rsa, status);

	rsan = htonl(rmEntry->rsa);
	memcpy(qgismsg+sizeof(QMsgHdr), &rsan, sizeof(unsigned long));

	if (QSendto(&dqdesc, &sqdesc, (QMsgHdr *)qgismsg, 
		GISQMSG_RSA, sizeof(QMsgHdr)+sizeof(unsigned long), 0) < 0)
	{
		CLIPRINTF((stdout, "%s could not send msg to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

	// Receive an acknowledgement that the operation was completed
	if (QRcvfrom2(&sqdesc, (QMsgHdr *)qgismsg, 
		sizeof(QMsgHdr)+sizeof(unsigned long), 0, 0) < 0)
	{
		CLIPRINTF((stdout, "%s could not recv ack from gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

	// Now check if the operation was successful
	qmsghdr = (QMsgHdr *)qgismsg;
	if (qmsghdr->mtype != GISQMSG_OK)
	{
		CLIPRINTF((stdout, "%s Gis unsuccessful in VIP operation\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

_return:
    return rc;
}

int
RealmDisableVipSig(RealmEntry *rmEntry)
{
	QDesc sqdesc, dqdesc;
#if 0
	QGisSrvrMsg  msg = {0};
    /* Q initialised ? */
    if (CliGisQ->Qid < 0)
    {
        return -1;
    }
	/* We might want to change the lenght. Why */
	CliQSendReq(ISERVER_GIS_Q, GIS_SRVR_MSG_TYPE, &msg, sizeof(QGisSrvrMsg));
#endif
    return 0;
}

long
realmNameToRealmId(char *rmName)
{
    char fn[] = "realmNameToRealmId:";
    DB_tDb  dbstruct;
    DB db;
    RealmKey rmKey = { 0 };
    RealmEntry *realmEntryPtr = NULL;
    unsigned long id=0;

	if (rmName)
	{
		if (!strcmp(rmName, REALM_ANY))
		{
			return REALM_ID_ANY;
		}
		
		if (!strcmp(rmName, REALM_UNASSIGNED))
		{
			return REALM_ID_UNASSIGNED;
		}
	}

	nx_strlcpy(rmKey.realmNameKey, rmName, REALM_NAME_LEN);

    dbstruct.read_write = GDBM_READER;
    if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
    {
    	CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
		return REALM_ID_INVALID;
    }

	if (!(realmEntryPtr = (RealmEntry *)DbFindEntry(db, (char *)&rmKey,
    	sizeof(RealmKey))))
	{
		DbClose(&dbstruct);
		return REALM_ID_INVALID;
    }

    id = realmEntryPtr->realmId;
    DbClose(&dbstruct);
    free(realmEntryPtr);

    return id;
}

/*
 * ParseCliGetRegidStr
 * Parses a string into ipaddress and realm name
 *
 * Notes :
 * Most normal strings of passed in should look like
 * "ip4:<IP Address>/<Realm Name>
 * "ip4:10.10.1.254/AAA"
 *  
 * Special cases
 * Case 1:
 * Seperator Character '/' does not mean the Realm Name
 * can't have '/'. Any thing left of first '/' will be 
 * interpreted as IP address string
 *  "ip4:10.10.1.254//home/export"
 *
 * Case 2:
 * For backward compatibility its not mandatory that 
 * there should be a '/' in the string.
 * "ip4:10.10.1.254"
 *
 * Case 3:
 * A string of type
 * "ip4:10.10.1.1/"
 * will result in default realm just like case 2
 *
 */
void
ParseCliGetRegidIPStr(char *s, unsigned long *ip, char *name)
{

    char *sep = strchr(s,'/');
    char *end = strchr(s, '\0');
    char ipstr[32] = {0};


    if (sep)
    {
        nx_strlcpy(ipstr, s, sep-s);
        ipstr[sep-s+1] = '\0';
        *ip = StringToIp(ipstr);

        if ((end - sep) < REALM_NAME_LEN)
        {
            nx_strlcpy(name, sep+1, end - sep);
        }
        else
        {
            CLIPRINTF((stdout, "Realm name too long for IP[%s]\n", ipstr));
        }
    }
    else
    {
        *ip = StringToIp(s);
    }

    return ;
}

// check if the realm is valid from system perspective
int
RealmEditHandler(RealmEntry *newrm, RealmEntry *oldrm)
{
	char  closed = 0;
	struct ifi_info *ifi_head = NULL; 
	struct ifi_info *ifi_head2 = NULL; 
	unsigned long opStatus=0, tmpopStatus=0;
        char errMsg[128] = "";
	VnetEntry vnetEntry = {"","",0,0,0};
        VnetEntry *vnetEntryPtr = NULL;

	if (strcmp(newrm->vnetName, VNET_UNASSIGNED) &&
	    (GetVnetInfo(newrm->vnetName, &vnetEntry) == 0))
	{
		vnetEntryPtr = &vnetEntry;
	}

	ifi_head = initIfs();
	ifi_head2 = initIfs2(1);

	tmpopStatus = oldrm->operStatus;

	if (strcmp(oldrm->vnetName, newrm->vnetName))
	{
		if (strcmp(newrm->vnetName, VNET_UNASSIGNED) &&
		    (GetVnetInfo(newrm->vnetName, &vnetEntry) == 0))
		{ 
			if (strcmp(oldrm->vnetName, VNET_UNASSIGNED))
			{ 
        			VnetEntry oldVnetEntry = {"", "", 0, 0, 0};
		    		GetVnetInfo(oldrm->vnetName, &oldVnetEntry);

				if (!tmpopStatus)
					tmpopStatus = oldrm->operStatus;
				if (!(RealmCloseVip(oldrm, &oldVnetEntry))) 
				{
					opStatus = tmpopStatus;
					closed =1;
				}
				freeIfs(ifi_head2);
				ifi_head2 = initIfs2(1);
			} else
			{
				closed = 1;
				opStatus = oldrm->operStatus;
			}
			strcpy(newrm->ifName, vnetEntry.ifName);
		}
		else
		{ 
			CLIPRINTF((stdout, "Bad VNET name [%s]\n", newrm->vnetName));
			nx_strlcpy (newrm->vnetName, oldrm->vnetName, VNET_NAME_LEN);
		}
	}

	if (oldrm->rsa != newrm->rsa)
	{
		/* Send msg to GIS to stop listening on  5060 and 17xx on old RSA */
		RealmChangeVipSig(oldrm, 0);

		if (CheckRSA(ifi_head2, newrm, oldrm) == 0)
		{ 
			if (!tmpopStatus)
				tmpopStatus = oldrm->operStatus;
			if(!(RealmCloseVip(oldrm, vnetEntryPtr)))
			{
				opStatus = tmpopStatus;
				closed =1; 
			}
			freeIfs(ifi_head2);
			ifi_head2 = initIfs2(1);
		}
		else
		{ 
			newrm->rsa = oldrm->rsa;
		}
	}

	if (oldrm->mask != newrm->mask)
	{
		if (checkMask(newrm->mask) == 0)
		{ 
			if (!tmpopStatus)
				tmpopStatus = oldrm->operStatus;
			/* RealmCloseVip will screw the opStatus so save it */
			if(!(RealmCloseVip(oldrm, vnetEntryPtr)))
			{
				opStatus = tmpopStatus;
				closed =1; 
			}
			freeIfs(ifi_head2);
			ifi_head2 = initIfs2(1);
		}
		else
		{ 
			CLIPRINTF((stdout, "wrong mask value [%s]\n", ULIPtostring(newrm->mask)));
			newrm->mask = oldrm->mask; 
		}
	}

	if (oldrm->adminStatus != newrm->adminStatus)
	{
		RealmChangeVipSig(newrm, newrm->adminStatus); 
	}

	if (oldrm->sigPoolId != newrm->sigPoolId)
	{
		RealmChangeVipSig(oldrm, 0);
	}

	if (oldrm->medPoolId != newrm->medPoolId)
	{
		RealmChangeVipSig(oldrm, 0);
	}

	if (oldrm->interRealm_mr != newrm->interRealm_mr)
	{
		RealmChangeVipSig(oldrm, 0);
	}

	if (oldrm->intraRealm_mr != newrm->intraRealm_mr)
	{
		RealmChangeVipSig(oldrm, 0);
	}

	if (closed) 
	{ 
		RealmOpenVip(newrm, vnetEntryPtr);
		/* only if the oldrm was operStatus=UP , we mark newrm with opStatus=UP */
		if (opStatus)
		{
			StatusChgIf(newrm->vipName, opStatus);
			newrm->operStatus = 1;
		}
	}

	free(ifi_head);
	free(ifi_head2);

	return 0;
}

/*
 * Checks if the RSA is a unique IP address for all existing realms
 * and checks if the ipaddress is unique for all logical interfaces
 * on host
 */
int
CheckRSA(struct ifi_info *ifi_head, RealmEntry *newrm, RealmEntry *oldrm)
{
	struct ifi_info  *ifp = NULL, *ifi_head2 = NULL;
	unsigned char flag=0;;

	if ( CacheAttach() > 0)
	{
		CacheHandleRealm(oldrm, CLIOP_DELETE);
		/* Is this new rsa unique across realms */
		if (CacheHandleRealm(newrm, CLIOP_ADD)< 0)
		{
			CLIPRINTF((stdout, "RSA duplication:RSA [%s] on existing realm\n", 
					ULIPtostring(newrm->rsa)));
			CacheHandleRealm(oldrm, CLIOP_ADD);
			CacheDetach();
			goto _return;
		}
		CacheDetach();
	}
	else
	{
		/* Can't verify the uniqueness of RSA */
		goto _return;
	}

	/* Is the new assigned IP unique across all logical interface
	 *  (not just realms)
	 */
	if (!ifi_head)
	{
		ifi_head2 = initIfs2(1);
		flag =1;
	}
	else
	{
		ifi_head2 = ifi_head;
	}

	if(ifp = matchIf(ifi_head2, htonl(newrm->rsa)))
	{
		if (IsIfLif(newrm->ifName))
		{
			CLIPRINTF((stdout, "IP address duplication:RSA [%s] exists on interface [%s]\n", 
					ULIPtostring(newrm->rsa), ifp->ifi_name));

		}
	}

	if (flag)
	{
		freeIfs(ifi_head2);
	}

	return 0;

_return:
	if (flag)
	{
		freeIfs(ifi_head2);
	}
	return -1;
}


unsigned long GenRealmID(char *realmName)
{
	char fn[] = "GenRealmID():";
	unsigned long value;
	int max=256;

	if (realmName)
	{
		if (!strcmp(realmName, REALM_ANY))
		{
			return REALM_ID_ANY;
		}
		
		if (!strcmp(realmName, REALM_UNASSIGNED))
		{
			return REALM_ID_UNASSIGNED;
		}
	}		

	// Make sure we discount duplicates too
	do
	{
		value = lrand48();

		if ((value != REALM_ID_UNASSIGNED) &&
			!CheckRealmIDDup(value) )
		{
			return value;
		}

	} while (--max > 0);

	// We are here because we could not do the generation
	NETERROR(MDB, 
		("%s Could not generate a valid value of realmId\n",
		fn));

	return 0xdeadbeef;
}

/* returns 1 if the given realm id already exists */
unsigned int
CheckRealmIDDup(unsigned long id)
{
	CacheRealmEntry  *ptr=NULL;

	CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
	ptr = CacheGet(realmCache, &id);
	CacheReleaseLocks(realmCache);
	
	return(ptr ? 1 : 0);
}

// This is called when a VNET is edited
// Update all the realms with the relevant info
// Close and reopen the VIPs
int
RealmCloseAndOpenVnetVips(VnetEntry *newVnetPtr, VnetEntry *oldVnetPtr)
{
    int rc = 0;
    RealmEntry *rmPtr, *oldrmPtr;
    DB_tDb dbstruct;
    DB db;
    unsigned long primaryRealmId = 0;

    if (!newVnetPtr || !oldVnetPtr)
    {
       return -1;
    }

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
        return -xleInvalArgs;
    }

    for (rmPtr = (RealmEntry *)DbGetFirstEntry(db); rmPtr;
         oldrmPtr = rmPtr,
         rmPtr = (RealmEntry *)DbGetNextEntry(db, (char *)oldrmPtr->realmName, sizeof(RealmKey)))
    {
         if (strcmp(rmPtr->vnetName, oldVnetPtr->vnetName))
         {
            continue;
         }

         RealmCloseVip(rmPtr, oldVnetPtr);

         strcpy(rmPtr->ifName, newVnetPtr->ifName);

         DbStoreEntry(db, (char *)rmPtr, sizeof(RealmEntry), rmPtr->realmName, sizeof(RealmKey));
         if (CacheAttach() > 0)
         {
             CacheHandleRealm(rmPtr, CLIOP_REPLACE);
             CacheDetach();
         }
    }

    for (rmPtr = (RealmEntry *)DbGetFirstEntry(db); rmPtr;
         oldrmPtr = rmPtr,
         rmPtr = (RealmEntry *)DbGetNextEntry(db, (char *)oldrmPtr->realmName, sizeof(RealmKey)))
    {
         if (strcmp(rmPtr->vnetName, oldVnetPtr->vnetName))
         {
            continue;
         }

         RealmOpenVip(rmPtr, newVnetPtr); 

         rmPtr->operStatus = GetIfFlags(rmPtr->vipName);
         rmPtr->mask = GetIfIpMask(rmPtr->vipName);

         DbStoreEntry(db, (char *)rmPtr, sizeof(RealmEntry), rmPtr->realmName, sizeof(RealmKey));
         if (CacheAttach() > 0)
         {
             CacheHandleRealm(rmPtr, CLIOP_REPLACE);
             CacheDetach();
         }
    }

    DbClose(&dbstruct);
    return rc;
}

int
RealmEditVnetGateway(VnetEntry *newVnetPtr, VnetEntry *oldVnetPtr)
{
    if (!newVnetPtr ||
        !IsVLANIdValid(newVnetPtr->vlanid) ||
        !IsRtgTblIdValid(newVnetPtr->rtgTblId))
    {
      return -1;
    }

    if (oldVnetPtr)
    {
      EditGatewayRoute(newVnetPtr->ifName, newVnetPtr->vlanid,
                       newVnetPtr->rtgTblId, newVnetPtr->gateway,
                       oldVnetPtr->gateway);
    } else
    {
      EditGatewayRoute(newVnetPtr->ifName, newVnetPtr->vlanid,
                       newVnetPtr->rtgTblId, newVnetPtr->gateway,
                       0);
    }

    return 0;
}

// Closes and reopens the 'secondary' VIPs on the same PHY
// Linux specific behavior ->
// IPs belonging to same subnet -> first is 'primary', rest are 'secondary'
// If you delete primary, secondary interfaces are deleted...sucks! 
int
RealmCloseAndOpenSubnetVips(unsigned long realmID, char *lifname, unsigned long ip,
                            unsigned long mask, int vlanid)
{
    int rc = 0;
    RealmEntry *rmPtr, *oldrmPtr;
    DB_tDb dbstruct;
    DB db;
    VnetEntry vnetEntry = {"","",0,0,0};
    VnetEntry *vnetEntryPtr = NULL;
    struct ifi_info *ifi_head2 = NULL; 

#ifndef NETOID_LINUX
    return 0;
#endif

    if (!realmID || !ip || !mask)
      return rc;

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", REALM_DB_FILE));
        return -xleInvalArgs;
    }

    for (rmPtr = (RealmEntry *)DbGetFirstEntry(db); rmPtr;
         oldrmPtr = rmPtr,
         rmPtr = (RealmEntry *)DbGetNextEntry(db, (char *)oldrmPtr->realmName, sizeof(RealmKey)))
    {
         if (!rmPtr->rsa ||
             !rmPtr->mask ||
             (realmID == rmPtr->realmId) ||       // For edited realm cases
             (ip == rmPtr->rsa) ||
             ((ip & mask) != (rmPtr->rsa & rmPtr->mask)))
         {
            continue;
         }
         // Get the VNET pointer
         if (strcmp(rmPtr->vnetName, VNET_UNASSIGNED) &&
             (GetVnetInfo(rmPtr->vnetName, &vnetEntry) == 0))
         {
             vnetEntryPtr = &vnetEntry;
         }

         // If this VIP is on a different PHY
         // we don't need to close this interface
         if (vnetEntryPtr && lifname &&
             ((vnetEntryPtr->vlanid != vlanid) ||
             strncmp(vnetEntryPtr->ifName, lifname, strlen(vnetEntryPtr->ifName))))
         {
            continue;
         }

         //printf("Closing..realmName=%s, vip=%s\n", rmPtr->realmName, rmPtr->vipName);

         // The LIF might have been delete already but we still need to delete rules etc
         RealmCloseVip(rmPtr, vnetEntryPtr);

         DbStoreEntry(db, (char *)rmPtr, sizeof(RealmEntry), rmPtr->realmName, sizeof(RealmKey));
         if (CacheAttach() > 0)
         {
             CacheHandleRealm(rmPtr, CLIOP_REPLACE);
             CacheDetach();
         }
    }

    // We couldn't have done in the same loop since the old VIP names and new VIP names may match
    // and we might close a just opened VIP
    for (rmPtr = (RealmEntry *)DbGetFirstEntry(db); rmPtr;
         oldrmPtr = rmPtr,
         rmPtr = (RealmEntry *)DbGetNextEntry(db, (char *)oldrmPtr->realmName, sizeof(RealmKey)))
    {
         if (!rmPtr->rsa ||
             !rmPtr->mask ||
             (realmID == rmPtr->realmId) ||       // For edited realm cases
             (ip == rmPtr->rsa) ||
             ((ip & mask) != (rmPtr->rsa & rmPtr->mask)))
         {
            continue;
         }

         // Get the VNET pointer
         if (strcmp(rmPtr->vnetName, VNET_UNASSIGNED) &&
             (GetVnetInfo(rmPtr->vnetName, &vnetEntry) == 0))
         {
             vnetEntryPtr = &vnetEntry;
         }

         // If this VIP is on a different PHY
         // we don't need to close this interface
         if (vnetEntryPtr && lifname &&
             ((vnetEntryPtr->vlanid != vlanid) ||
             strncmp(vnetEntryPtr->ifName, lifname, strlen(vnetEntryPtr->ifName))))
         {
            continue;
         }

         RealmOpenVip(rmPtr, vnetEntryPtr); 

         //printf("Opened..realmName=%s, vip=%s\n", rmPtr->realmName, rmPtr->vipName);

         rmPtr->operStatus = GetIfFlags(rmPtr->vipName);
         rmPtr->mask = GetIfIpMask(rmPtr->vipName);

         DbStoreEntry(db, (char *)rmPtr, sizeof(RealmEntry), rmPtr->realmName, sizeof(RealmKey));
         if (CacheAttach() > 0)
         {
             CacheHandleRealm(rmPtr, CLIOP_REPLACE);
             CacheDetach();
         }
    }

    DbClose(&dbstruct);
    return rc;
}
