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

#define ERRMSG_LEN 128

int
HandleVnetAdd(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleVnetAdd():";
    int rc = xleOk;
    char errMsg[ERRMSG_LEN] = "";
     
    if (argc < 1)
    {
      HandleCommandUsage(comm, argc, argv);
      return -xleInsuffArgs;
    }

    rc = AddVnetEntry(argv[0], errMsg);
    if (rc != xleOk)
    {
      CLIPRINTF((stdout, errMsg));
    }

    return rc;
}

int
HandleVnetDelete(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleVnetDelete():";
    int rc = xleOk;
    char errMsg[ERRMSG_LEN] = "";

    if (argc < 1)
    {
        HandleCommandUsage(comm, argc, argv);
        return -xleInsuffArgs;
    }

    rc = DeleteVnetEntry(argv[0], errMsg);
    if (rc != xleOk)
    {
      CLIPRINTF((stdout, errMsg));
    }

    return rc;
}

int
HandleVnetCache(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleVnetCache():";
    CacheVnetEntry *ptr = NULL;
    unsigned int n=0;

    if (CacheAttach() > 0)
    {
      CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);

      for (ptr = CacheGetFirst(vnetCache); ptr;
             ptr = CacheGetNext(vnetCache, ptr->vnet.vnetName))
      {
        PrintVnetEntry(stdout, &ptr->vnet);
        n++;
      }

      CacheReleaseLocks(vnetCache);
      CacheDetach();
    }
    else
    {
      CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
      return -1;
    }

    if (n)
    { 
      CLIPRINTF((stdout, "Total Matches %d\n", n)); 
    }
    else 
    { 
      CLIPRINTF((stdout, "No Matches found\n")); 
    }

    return 0;
}

int
HandleVnetEdit(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleVnetEdit()";
    DB_tDb  dbstruct = {0};
    DB db = {0};
    VnetEntry  *vnetEntryPtr = NULL;
    VnetEntry	oldvnet;
    VnetKey     vnetKey = {0};
    unsigned long oldGw,newGw;
    int rc = xleOk;
    CacheVnetEntry *ptr;
    char errMsg[ERRMSG_LEN] = "";

    if (argc < 1)
    {
      rc = -xleInsuffArgs;
      return rc;
    }

    // cli vnet edit commands should not be excerciseable when 
    // gis is down.	
    if( CacheAttach() < 0 ) 
    {
	  
      CLIPRINTF((stdout, "iServer is not running - Vnet edit operation disabled\n"));
      rc = -xleOpNoPerm;
      goto _return;
    }

    nx_strlcpy (vnetKey.vnetNameKey, argv[0], VNET_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(VNET_DB_FILE, DB_eVnet, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", VNET_DB_FILE));
        rc = -xleInvalArgs;
        goto _return;
    }

    if (!(vnetEntryPtr = (VnetEntry *)DbFindEntry(db, (char *)&vnetKey, sizeof(VnetKey)) ))
    {
        CLIPRINTF((stdout, "entry (%s) does not exist\n", vnetKey.vnetNameKey));
        rc = -xleNoEntry;
        goto _return;
    }

    argc -= 1;
    argv += 1;

    if (argc <= 0)
    {
        goto _return;
    }

    memcpy(&oldvnet, vnetEntryPtr, sizeof(VnetEntry));

    GetAttrPairs(comm->name, &argc, &argv, vnetEntryPtr, CLI_GET_ATTR_VNET);

    // Check ifname-vid uniqueness
    char dupVnetName[VNET_NAME_LEN] = "";
    if (CheckIfnameVlanidComboExists(vnetEntryPtr, dupVnetName))
    {
        CLIPRINTF((stdout,"Interface and VLAN combo exists for VNET=%s\n", dupVnetName));
        rc = -xleOpNoPerm;
        goto _return;
    }

    VnetEditHandler(vnetEntryPtr, &oldvnet);

    /* Store the entry */
    if (DbStoreEntry(db, (char *)vnetEntryPtr, sizeof(VnetEntry),
                (char *)vnetEntryPtr->vnetName, sizeof(VnetKey)) < 0)
    {
        CLIPRINTF((stdout,"database store error %d\n", errno));
        rc = -xleOpNoPerm;
        goto _return;
    }

    if (CacheAttach() > 0)
    {
        CacheHandleVnet(vnetEntryPtr, CLIOP_REPLACE);
        CacheDetach();
    }

_return:
    DbClose(&dbstruct);
    if (vnetEntryPtr)
    {
      free(vnetEntryPtr);
    }
    return rc;
}

int
HandleVnetList(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleVnetList():";
    DB_tDb dbstruct;
    DB db;
    VnetEntry  *ptr, *oldptr;
    int rc = xleOk;
    int n=0;

    ptr = (VnetEntry *)DbExtractFirstEntry(VNET_DB_FILE, DB_eVnet);
    while (ptr)
    {
        PrintVnetEntry(stdout, ptr);
        CLIPRINTF((stdout, "\n"));
        n++;
        oldptr = ptr;
        ptr = (VnetEntry *)DbExtractNextEntry(VNET_DB_FILE,
                DB_eVnet, (char *)oldptr->vnetName, sizeof(VnetKey));
        free(oldptr);
    }

    CLIPRINTF((stdout, "%d VNET entries\n", n));
    return rc;
}

int
HandleVnetLkup(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleVnetLkup():";
    CacheVnetEntry 	*ptr;
    unsigned char	vnetName[VNET_NAME_LEN];

    if (argc < 1)
    {
      HandleCommandUsage(comm, argc, argv);
      return -xleInsuffArgs;
    }

    nx_strlcpy(vnetName, argv[0], VNET_NAME_LEN);

    if (CacheAttach() > 0)
    {
        CLIPRINTF((stdout, "VNET Group Cache...\n"));

        CacheGetLocks(vnetCache, LOCK_READ, LOCK_BLOCK);

        ptr = CacheGet(vnetCache, vnetName);
        if (ptr)
        {
          PrintVnetEntry(stdout, &ptr->vnet);
        }
        else
        {
          CLIPRINTF((stdout, "No matching entries\n"));
        }
        CacheReleaseLocks(vnetCache);
        CacheDetach();
      }
      else
      {
        CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
        return -1;
      }

      return 0;
}

int
AddVnetEntry(char *name, char *errMsg)
{
    DB_tDb  dbstruct;
    DB db;
    VnetEntry vnetEntry = {0};
    VnetKey vnetKey = {0};
    unsigned short  flags;
    int rc = xleOk;
     
    strcpy(errMsg, "");

    if (strlen(name) >= VNET_NAME_LEN)
    {
      sprintf(errMsg, "VNET name too long:[%s]\n", name);
      return -xleInvalArgs;
    }

    nx_strlcpy(vnetKey.vnetNameKey, name, VNET_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(VNET_DB_FILE, DB_eVnet, &dbstruct)))
    {
        sprintf(errMsg, "Unable to open %s\n", VNET_DB_FILE);
        return -xleInvalArgs;
    }

    if (DbFindEntry(db, (char *)&vnetKey, sizeof(VnetKey)))
    {
        sprintf(errMsg, "entry (%s) already exists\n", vnetKey.vnetNameKey);
        DbClose(&dbstruct); 
        return -xleExists;
    }

    nx_strlcpy(vnetEntry.vnetName, name, VNET_NAME_LEN);

    vnetEntry.ifName[0] = '\0';
    vnetEntry.vlanid = VLANID_NONE;
    vnetEntry.rtgTblId = 0;
    vnetEntry.gateway = 0;

    if (DbStoreEntry(db, (char *)&vnetEntry, sizeof(VnetEntry), 
                (char *)&vnetKey, sizeof(VnetKey))<0)
    {
        sprintf(errMsg, "database store error %d\n", errno);
        DbClose(&dbstruct); 
        return -xleOpNoPerm;
    }

    if (CacheAttach() > 0)
    {
        CacheHandleVnet(&vnetEntry, CLIOP_ADD);
        CacheDetach();
    }

    DbClose(&dbstruct); 
    return rc;
}

int
DeleteVnetEntry(char *name, char *errMsg)
{
    DB_tDb dbstruct;
    DB db;
    VnetKey vnetKey = { 0 };
    VnetEntry *vnetEntryPtr = NULL;
    int rc = xleOk;

    // DO WE ALLOW DELETING A VNET ENTRY IF REALMS EXIST ON IT ??
    if (VnetRealmPresent(name) > 0)
    {
        sprintf(errMsg, "Realm(s) exist on this Vnet..cannot delete\n");
        return -xleExists;
    }

    nx_strlcpy(vnetKey.vnetNameKey, name, VNET_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(VNET_DB_FILE, DB_eVnet, &dbstruct)))
    {
        sprintf(errMsg, "Unable to open %s\n", VNET_DB_FILE);
        return -xleInvalArgs;
    }

    if (!(vnetEntryPtr = (VnetEntry *)DbFindEntry(db, (char *)&vnetKey,
                    sizeof(VnetKey))))
    {
        sprintf(errMsg,"Entry (%s) does not exist %d\n", vnetKey.vnetNameKey, errno);
        DbClose(&dbstruct);
        return -xleNoEntry;
    }

    if (DbDeleteEntry(db, (char *)&vnetKey, sizeof(VnetKey)) < 0)
    {
        sprintf(errMsg, "Database delete error %d\n", errno);
        DbClose(&dbstruct);
        free(vnetEntryPtr);
        return -xleOpNoPerm;
    }

    if (CacheAttach() > 0)
    {
        CacheHandleVnet(vnetEntryPtr, CLIOP_DELETE);
        CacheDetach();
    }

    DbClose(&dbstruct);
    free(vnetEntryPtr);
    return rc;
}

int
GetVnetEntryByName(char *name, VnetEntry *entry)
{
    char fn[] = "GetVnetEntryByName():";
    DB_tDb dbstruct;
    DB db;
    VnetKey vnetKey = { 0 };
    VnetEntry *vnetEntryPtr = NULL;

    nx_strlcpy(vnetKey.vnetNameKey, name, VNET_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(VNET_DB_FILE, DB_eVnet, &dbstruct)))
    {
        return 0;
    }

    if (!(vnetEntryPtr = (VnetEntry *)DbFindEntry(db, (char *)&vnetKey,
                    sizeof(VnetKey))))
    {
        DbClose(&dbstruct);
        return 0;
    }

    memcpy(entry, vnetEntryPtr, sizeof(VnetEntry));

    DbClose(&dbstruct);
    free(vnetEntryPtr);

    return 1;
}

// returns the number of realms that are part of this vnet
// maybe changed later to return just 1 or 0
int
VnetRealmPresent(char *name)
{
    char fn[] = "VnetRealmPresent():";
    DB_tDb dbstruct;
    DB db;
    RealmEntry *rmPtr, *oldrmPtr;
    int rc = xleOk;
    int n=0;

    rmPtr = (RealmEntry *)DbExtractFirstEntry(REALM_DB_FILE, DB_eRealm);
    while(rmPtr)
    {
        if (!strcmp(rmPtr->vnetName, name))
        {
          n++; // you may just return 'true' here
        }
        oldrmPtr = rmPtr;
        rmPtr = (RealmEntry *)DbExtractNextEntry(REALM_DB_FILE,
                 DB_eRealm, (char *)oldrmPtr->realmName, sizeof(RealmKey));
        free(oldrmPtr);
    }
    return n;
}

int
CheckIfnameVlanidComboExists(VnetEntry *vnetEntryPtr, char *dupVnetName)
{
	CacheVnetEntry *cacheVnetEntry;
	int exists = 0;

	if (CacheAttach() > 0)
	{
		CacheGetLocks(vnetCache, LOCK_READ, LOCK_BLOCK);

		for (cacheVnetEntry = CacheGetFirst(vnetCache); cacheVnetEntry;
            		cacheVnetEntry = CacheGetNext(vnetCache, &cacheVnetEntry->vnet.vnetName))
		{
			if (strcmp(cacheVnetEntry->vnet.vnetName, vnetEntryPtr->vnetName) &&
			    (strcmp(cacheVnetEntry->vnet.ifName, REALM_UNASSIGNED) &&
			    strcmp(vnetEntryPtr->ifName, REALM_UNASSIGNED) &&
			    !strcmp(vnetEntryPtr->ifName, cacheVnetEntry->vnet.ifName)) &&
			    (cacheVnetEntry->vnet.vlanid == vnetEntryPtr->vlanid))
			{
				exists = 1;
				strcpy(dupVnetName, cacheVnetEntry->vnet.vnetName);
				break;
			}
		}

		CacheReleaseLocks(vnetCache);
		CacheDetach();
	}

       	return exists;
}

unsigned short
FindUnusedRoutingTable()
{
	CacheVnetEntry *cacheVnetEntry;
	int count = 0;
	unsigned char ipTbls[RTG_TBL_MAX + 1];

	if (CacheAttach() > 0)
	{
		memset(ipTbls, 0, sizeof(ipTbls));
		CacheGetLocks(vnetCache, LOCK_READ, LOCK_BLOCK);
	
		for (cacheVnetEntry = CacheGetFirst(vnetCache); cacheVnetEntry;
            		cacheVnetEntry = CacheGetNext(vnetCache, &cacheVnetEntry->vnet.vnetName))
		{
			if (IsVLANIdValid(cacheVnetEntry->vnet.vlanid) &&
				IsRtgTblIdValid(cacheVnetEntry->vnet.rtgTblId))
			{
				ipTbls[cacheVnetEntry->vnet.rtgTblId] = 1;
			}
		}

		CacheReleaseLocks(vnetCache);
		CacheDetach();
	} else
	{
         	return 0;
	}

	for (count = RTG_TBL_MIN ; count <= RTG_TBL_MAX ; count++)
	{
		if (ipTbls[count] == 0)
			return count;   // this is the first free routing table
	}

       	return 0;
}

int
VnetEditHandler(VnetEntry *newVnet, VnetEntry *oldVnet)
{
	char  closed = 0;
	char  closeVnetVips = 0;
	char  invalidInfo = 0;
	struct ifi_info *ifi_head; 
	unsigned long opStatus=0, tmpopStatus=0;
        char errMsg[128] = "";

	ifi_head = initIfs();

	if (strcmp(oldVnet->ifName, newVnet->ifName))
	{
		if (checkIfName(ifi_head, newVnet->ifName) == 0)
		{ 
			closeVnetVips = 1;
		}
		else
		{ 
			CLIPRINTF((stdout, "Bad interface name [%s]\n", newVnet->ifName));
			nx_strlcpy (newVnet->ifName, oldVnet->ifName, IFI_NAME);
			invalidInfo = 1;
		}
	}

	if (oldVnet->vlanid != newVnet->vlanid)
	{
		if ((oldVnet->vlanid != VLANID_NONE) &&
		    (newVnet->vlanid == VLANID_NONE))
		{
			closeVnetVips = 1;
			newVnet->rtgTblId = 0;
		}
		else
		{
			if (!IsVLANIdValid(newVnet->vlanid)) // Invalid VLANID
			{
				CLIPRINTF((stdout, "VLAN Id is not valid. Use [0-4094, None]\n"));
				newVnet->vlanid = oldVnet->vlanid;
				invalidInfo = 1;
			}
			else
			{ 
				closeVnetVips = 1;
				newVnet->rtgTblId = FindUnusedRoutingTable();
				if (!IsRtgTblIdValid(newVnet->rtgTblId))
				{
					CLIPRINTF((stdout, "Failed to find Free routing table\n"));
        			}
			}
		}
	}

	if (!invalidInfo && closeVnetVips)
	{
		RealmCloseAndOpenVnetVips(newVnet, oldVnet);
	}

	if (!invalidInfo && (oldVnet->gateway != newVnet->gateway))
	{
		RealmEditVnetGateway(newVnet, oldVnet);
	}

	free(ifi_head);
	return 0;
}

int
EditVnetEntryByName(unsigned char *name, int attrName, void *value)
{
    DB_tDb  dbstruct = {0};
    DB db = {0};
    VnetEntry  *vnetEntryPtr = NULL;
    VnetKey     vnetKey = {0};
    int rc = xleOk;
    CacheVnetEntry *ptr;

    if (name == NULL || value == NULL)
    {
      return -xleInvalArgs;
    }

    nx_strlcpy (vnetKey.vnetNameKey, name, VNET_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(VNET_DB_FILE, DB_eVnet, &dbstruct)))
    {
      //CLIPRINTF((stdout, "Unable to open %s\n", VNET_DB_FILE));
      return -xleInvalArgs;
    }

    if (!(vnetEntryPtr = (VnetEntry *)DbFindEntry(db, (char *)&vnetKey, sizeof(VnetKey))))
    {
      //CLIPRINTF((stdout, "entry (%s) does not exist\n", vnetKey.vnetNameKey));
      DbClose(&dbstruct);
      return -xleNoEntry;
    }

    //CacheGetLocks(vnetCache, LOCK_READ, LOCK_BLOCK);
    //ptr = CacheGet(vnetCache, &vnetEntryPtr->vnetName);
    //CacheReleaseLocks(vnetCache);

    switch (attrName)
    {
      case VNET_VLANID:
        vnetEntryPtr->vlanid = *(unsigned short *)value;
        break;
      case VNET_RTGTBLID:
        vnetEntryPtr->rtgTblId = *(unsigned short *)value;
        break;
      case VNET_GATEWAY:
        vnetEntryPtr->gateway = *(unsigned long *)value;
        break;
      default:
        break;
    }

    /* Store the entry */
    if (DbStoreEntry(db, (char *)vnetEntryPtr, sizeof(VnetEntry),
                (char *)vnetEntryPtr->vnetName, sizeof(VnetKey)) < 0)
    {
      //CLIPRINTF((stdout,"database store error %d\n", errno));
      DbClose(&dbstruct);
      free(vnetEntryPtr);
      return -xleOpNoPerm;
    }

    if (CacheAttach() > 0)
    {
        CacheHandleVnet(vnetEntryPtr, CLIOP_REPLACE);
        CacheDetach();
    }

    DbClose(&dbstruct);
    free(vnetEntryPtr);

    return xleOk;
}

int GetVnetInfo(char *vnetName, VnetEntry *vnetPtr)
{
    DB_tDb  dbstruct = {0};
    DB db = {0};
    VnetEntry  *vnetEntryPtr = NULL;
    VnetKey     vnetKey = {0};
    int rc = xleOk;

    nx_strlcpy (vnetKey.vnetNameKey, vnetName, VNET_NAME_LEN);

    //dbstruct.read_write = GDBM_WRCREAT;
    dbstruct.read_write = GDBM_READER;
    if (!(db = DbOpenByID(VNET_DB_FILE, DB_eVnet, &dbstruct)))
    {
        rc = -xleInvalArgs;
        goto _return;
    }

    if (!(vnetEntryPtr = (VnetEntry *)DbFindEntry(db, (char *)&vnetKey, sizeof(VnetKey)) ))
    {
        rc = -xleNoEntry;
        goto _return;
    }

    memcpy(vnetPtr, vnetEntryPtr, sizeof(VnetEntry));

_return:
    DbClose(&dbstruct);
    if (vnetEntryPtr)
    {
      free(vnetEntryPtr);
    }
    return rc;
}
