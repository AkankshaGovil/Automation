/*
 * Functions which involve both the db management
 * and shared memory
 */

#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "alerror.h"
#include "ipc.h"
#include "bits.h"
#include "mem.h"
#include "db.h"
#include "srvrlog.h"
#include "serverp.h"
#include "gw.h"
#include "clist.h"
#include "lsconfig.h"
#include <malloc.h>
#include "nxosd.h"
#include "cache.h"

/* Database definitions */
DB_tHandle dbHandles[DB_eMax] =
{ 
	{ NETOIDS_DB_FILE, 		NETOIDS_LOCK_FILE, 		{ 0, 0 } },
	{ VPNS_DB_FILE, 		VPNS_LOCK_FILE, 		{ 0, 0 } }, 
	{ VPNG_DB_FILE, 		VPNG_LOCK_FILE, 		{ 0, 0 } }, 
	{ FAX_DB_FILE, 			FAX_LOCK_FILE, 			{ 0, 0 } }, 
	{ CALLPLAN_DB_FILE,		CALLPLAN_LOCK_FILE,		{ 0, 0 } },
	{ CALLROUTE_DB_FILE,	CALLROUTE_LOCK_FILE,	{ 0, 0 } },
	{ CALLPLANBIND_DB_FILE,	CALLPLANBIND_LOCK_FILE,	{ 0, 0 } },
	{ ATTRIBS_DB_FILE,		ATTRIBS_LOCK_FILE,		{ 0, 0 } },
	{ TRIGGER_DB_FILE,		TRIGGER_LOCK_FILE,		{ 0, 0 } },
	{ REALM_DB_FILE,		REALM_LOCK_FILE,		{ 0, 0 } },
	{ IGRP_DB_FILE,			IGRP_LOCK_FILE,			{ 0, 0 } },
	{ VNET_DB_FILE,			VNET_LOCK_FILE,			{ 0, 0 } }
};

int
DeleteNetoidFromDatabase(NetoidInfoEntry *netInfo)
{
	 char fn[] = "DeleteNetoidFromDatabase()";
	 DB db;
	 DB_tDb dbstruct;

     	dbstruct.read_write = GDBM_WRCREAT;

     	if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
     	{ 
		  ERROR(MCACHE, 
			("%s Unable to open database %s\n", 
			fn, NETOIDS_DB_FILE));
		  return 0;
     	}

     	DbDeleteInfoEntry(db, (char *)netInfo, 
			sizeof(NetoidSNKey));
     
     	DbClose(&dbstruct);
	
	return 1;
}

int
DeleteVpnFromDatabase(VpnEntry *vpnEntry)
{
	 char fn[] = "DeleteVpnFromDatabase()";
     	DB db;
	DB_tDb dbstruct;

     	/* Delete the entry from the database first
      	* as there is some funky behavior 
      	* even if we use GDBM_REPLACE
      	*/
     	dbstruct.read_write = GDBM_WRCREAT;

     	if (!(db = DbOpenByID(VPNS_DB_FILE, DB_eVpns, &dbstruct)))
     	{ 
		  ERROR(MCACHE, 
			("%s Unable to open database %s\n", 
			fn, VPNS_DB_FILE));
		  return 0;
     	}

     	DbDeleteVpnEntry(db, (char *)vpnEntry, sizeof(VpnKey));

     	DbClose(&dbstruct);
	
	return 1;
}

int
DeleteVpnGFromDatabase(VpnGroupEntry *vpnGroupEntry)
{
	 char fn[] = "DeleteVpnGFromDatabase():";
     	DB db;
		DB_tDb dbstruct;

     	/* Delete the entry from the database first
      	* as there is some funky behavior 
      	* even if we use GDBM_REPLACE
      	*/
     	dbstruct.read_write = GDBM_WRCREAT;

     	if (!(db = DbOpenByID(VPNG_DB_FILE, DB_eVpnG, &dbstruct)))
     	{ 
		  ERROR(MCACHE, 
			("%s Unable to open database %s\n", 
			fn, VPNG_DB_FILE));
		  return 0;
     	}

     	DbDeleteVpnEntry(db, (char *)vpnGroupEntry, sizeof(VpnGroupKey));

     	DbClose(&dbstruct);
	
	return 1;
}

int
DeleteCPFromDatabase(CallPlanEntry *cpEntry)
{
	 char fn[] = "DeleteCPFromDatabase():";
	 DB db;
	 DB_tDb dbstruct;
	 
	 /* Delete the entry from the database first
	  * as there is some funky behavior 
	  * even if we use GDBM_REPLACE
	  */
	 dbstruct.read_write = GDBM_WRCREAT;
	 
	 if (!(db = DbOpenByID(CALLPLAN_DB_FILE, DB_eCallPlan, &dbstruct)))
	 { 
		  ERROR(MDB, 
				("%s Unable to open database %s\n", 
			fn, CALLPLAN_DB_FILE));
		  return 0;	
	 }

	 /* At this point we can delete the cp entry */
	 if (DbDeleteEntry(db,
					   (char *)cpEntry, sizeof(CallPlanKey)) < 0)
	 {
		  NETERROR(MDB, ("%s Error storing entry\n", fn));
	 }

	 DbClose(&dbstruct);
	
	 return 1;
}

int
DeleteCRFromDatabase(VpnRouteEntry *routeEntry)
{
	 char fn[] = "DeleteCRFromDatabase():";
	 DB db;
	 DB_tDb dbstruct;
	 
	 /* Delete the entry from the database first
	  * as there is some funky behavior 
	  * even if we use GDBM_REPLACE
	  */
	 dbstruct.read_write = GDBM_WRCREAT;
	 
	 if (!(db = DbOpenByID(CALLROUTE_DB_FILE, DB_eCallRoute, &dbstruct)))
	 { 
		  ERROR(MDB, 
				("%s Unable to open database %s\n", 
			fn, CALLROUTE_DB_FILE));
		  return 0;	
	 }

	 /* At this point we can delete the cp entry */
	 if (DbDeleteEntry(db,
					   (char *)routeEntry, sizeof(VpnRouteKey)) < 0)
	 {
		  NETERROR(MDB, ("%s Error storing entry\n", fn));
	 }

	 DbClose(&dbstruct);
	
	 return 1;
}

int
DeleteCPBFromDatabase(CallPlanBindEntry *cpbEntry)
{
	 char fn[] = "DeleteCPBFromDatabase():";
	 DB db;
	 DB_tDb dbstruct;
	 
	 /* Delete the entry from the database first
	  * as there is some funky behavior 
	  * even if we use GDBM_REPLACE
	  */
	 dbstruct.read_write = GDBM_WRCREAT;
	 
	 if (!(db = DbOpenByID(CALLPLANBIND_DB_FILE, DB_eCallPlanBind, &dbstruct)))
	 { 
		  ERROR(MDB, 
				("%s Unable to open database %s\n", 
			fn, CALLPLANBIND_DB_FILE));
		  return 0;	
	 }

	 /* At this point we can delete the cp entry */
	 if (DbDeleteEntry(db,
					   (char *)cpbEntry, sizeof(CallPlanBindKey)) < 0)
	 {
		  NETERROR(MDB, ("%s Error storing entry\n", fn));
	 }

	 DbClose(&dbstruct);
	
	 return 1;
}

int
UpdateNetoidDatabase(NetoidInfoEntry *netInfo)
{
	char fn[] = "UpdateNetoidDatabase():";

	if (DbUpdateEntry(NETOIDS_DB_FILE, DB_eNetoids, (char*) netInfo,
		sizeof(NetoidInfoEntry), (char*) netInfo, sizeof(NetoidSNKey)))	
	{
		NETERROR(MDB, ("%s DbUpdateEntry error\n", fn));
	}
	
	return 1;
}
	
int
UpdateNetoidAttrDatabase(NetoidSNKey *snkey, ClientAttribs *clAttribs)
{
	char fn[] = "UpdateNetoidAttrDatabase():";

	if (DbUpdateEntry(ATTRIBS_DB_FILE, DB_eAttribs, (char*) clAttribs,
		sizeof(ClientAttribs), (char*) snkey, sizeof(NetoidSNKey)))	
	{
		NETERROR(MDB, ("%s DbUpdateEntry error\n", fn));
	}
	
	return 1;
}
	
InfoEntry *
SearchNetoidDatabase (char *key, int keylen)
{
	 char fn[] = "SearchNetoidDatabase():";
	 DB db;
	DB_tDb dbstruct;
	NetoidInfoEntry *netInfo = 0;
	
     	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
	{
	     ERROR(MDB, ("%s Unable to open database %s\n", fn, NETOIDS_DB_FILE));
	     return 0;
	}
	
	netInfo = DbFindInfoEntry(db, key, keylen);

	if (!netInfo)
	{
	     ERROR(MDB, ("No entry of such kind found in db\n"));
	}

	DbClose(&dbstruct);

	return netInfo;
}

int
GetIedgeVpn(char *vpnName, VpnEntry *vpn)
{
	char fn[] = "GetIedgeVpnGroup():";
	VpnEntry vpnEntry;
	CacheVpnEntry *cacheVpnEntry;
		
	if (vpnName[0] == '\0')
	{
		return -1;
	}

	memset(vpn, 0, sizeof(VpnEntry));
	memset(&vpnEntry, 0, sizeof(VpnEntry));

	strcpy(vpnEntry.vpnName, vpnName);

	cacheVpnEntry = CacheGet(vpnCache, &vpnEntry);

	if (cacheVpnEntry)
	{
		memcpy(vpn, &cacheVpnEntry->vpnEntry, sizeof(VpnEntry));
		return 1;
	}
	else
	{
		return -1;
	}
}

int
FindIedgeVpn(char *vpnName, VpnEntry *vpn)
{
	char fn[] = "FindIedgeVpnGroup():";
	int rc;

	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

	rc = GetIedgeVpn(vpnName, vpn);

	CacheReleaseLocks(vpnCache);

	return rc;
}

int
BranchFwdPhone(InfoEntry *pentry, CacheTableInfo *cacheInfo, char *newphone)
{
	char fn[] = "BranchFwdPhone():";
	int rc = -1;

	if ((rc < 0) && strlen(pentry->nphone))
	{
		rc = ApplyNetworkPolicy(pentry->vpnName, pentry->cpname, pentry->phone, pentry->nphone, newphone,
								CRF_POSITIVE|CRF_REJECT|
								CRF_CALLORIGIN|CRF_VPNINT|CRF_VPNEXT,
								APPLY_DEST,
								NULL,
								NULL,
								NULL);
		if (rc == 0)
		{	
			strcpy(newphone, pentry->nphone);
		}

		rc = CacheFind(phoneCache, newphone,
						cacheInfo, sizeof(CacheTableInfo));
	}	

	return rc;
}

int
BranchProxyPhone(InfoEntry *pentry, CacheTableInfo *cacheInfo, char *newphone)
{
	char fn[] = "BranchProxyPhone():";
	int rc = -1;

#if 0
	if (strlen(pentry->xphone))
	{
		rc = CacheFind(phoneCache, pentry->xphone,
			cacheInfo, sizeof(CacheTableInfo));
	}	
#endif

	return rc;
}

InfoEntry *
FindIedgeVpnGroupInDatabase(char *key, int keylen, char *vpnGroup)
{
	char fn[] = "FindIedgeVpnGroupInDatabase():";
        DB db;
	DB_tDb dbstruct;
	NetoidInfoEntry *netInfo = 0;
	VpnEntry vpnEntry, *tmp;

	memset(vpnGroup, 0, VPN_GROUP_LEN);

     	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
	{
	     ERROR(MDB, ("%s Unable to open database %s\n", fn, NETOIDS_DB_FILE));
	     return 0;
	}
	
	netInfo = DbFindInfoEntry(db, key, keylen);

	DbClose(&dbstruct);
	
	if (!netInfo)
	{
	     	ERROR(MDB, ("No entry of such kind found in db\n"));
		return 0;
	}

	if (strlen(netInfo->vpnName) <= 0)
	{
		NETDEBUG(MDB, NETLOG_DEBUG4, ("%s No vpn id set\n", fn));
	     	return netInfo;
	}

	memset(&vpnEntry, 0, sizeof(VpnEntry));

	/* Now determine the vpn Id of this iedge */
	strcpy(vpnEntry.vpnName, netInfo->vpnName);

	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(VPNS_DB_FILE, DB_eVpns, &dbstruct)))
	{
	     ERROR(MDB, ("%s Unable to open database %s\n", fn, VPNS_DB_FILE));
	     return netInfo;
	}

	tmp = DbFindVpnEntry(db, (char *)&vpnEntry, sizeof(VpnKey));

	if (!tmp)
	{
	     	ERROR(MDB, ("No vpn entry of such kind found in db\n"));
		DbClose(&dbstruct);
	     	return netInfo;
	}

	DbClose(&dbstruct);

	/* Now copy the vpn group of this vpn */
	strncpy(vpnGroup, tmp->vpnGroup, VPN_GROUP_LEN);
	vpnGroup[VPN_GROUP_LEN-1] = '\0';

	DEBUG(MDB, NETLOG_DEBUG4,
		("%s iedge %s %lu vpn %s vpngrp %s\n",
			fn, netInfo->regid, netInfo->uport,
			tmp->vpnId, tmp->vpnGroup));

	/* We are not returning the vpn entry */
	free(tmp);

	return netInfo;
}

/* Return 1 if the vpnG was found, and
 * -1 if not. A null vpnG is a valid vpnG, and would
 * correspong to a return value of 1.
 */
int
DbFindVpnG(char *vpnId, char *vpnG)
{
     	VpnEntry vpnEntry, *tmp;
        DB db;
	DB_tDb dbstruct;
	int rc = -1;

	memset(vpnG, 0, VPN_GROUP_LEN);

	/* For a vpnId not specified or a null vpnId,
	 * the vpnGroup is a Null Vpn Group
	 */
	if (!vpnId || !strlen(vpnId))
	{
		return 1;
	}

     	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(VPNS_DB_FILE, DB_eVpns, &dbstruct)))
	{
	     ERROR(MDB, ("DbFindVpnG(): Unable to open database %s\n", VPNS_DB_FILE));
	     return -1;
	}

	memset(&vpnEntry, 0, sizeof(VpnEntry));

     	strcpy(vpnEntry.vpnId, vpnId);

     	tmp = DbFindVpnEntry(db, (char *)&vpnEntry, sizeof(VpnKey));

     	if (tmp)
	{
		/* Vpn Entry Exists */
     		strcpy(vpnG, tmp->vpnGroup);
		rc = 1;
	}
	
	DbClose(&dbstruct);

	return rc;
}

NetoidInfoEntry *
DbFindEmailAddress(char *email)
{
        DB db;
	DB_tDb dbstruct;
     	NetoidInfoEntry *netInfo = NULL;
     	NetoidSNKey *key, *okey;

     	dbstruct.read_write = GDBM_READER;

	if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
	{
	     ERROR(MDB, ("DbFindEmailAddress(): Unable to open database %s\n",
				NETOIDS_DB_FILE));
	     return NULL;
	}

     	for (key = (NetoidSNKey *)DbGetFirstInfoKey(db); key != 0; 
	  	key = (NetoidSNKey *)DbGetNextInfoKey(db, (char *)key, 
						sizeof(NetoidSNKey)),
	       					free(okey), free(netInfo), 
						netInfo = NULL)
     	{
	  	netInfo = DbFindInfoEntry(db, (char *)key, sizeof(NetoidSNKey));
		if (BIT_TEST(netInfo->sflags|netInfo->dflags, ISSET_EMAIL))
		{
			if (!strncmp(netInfo->email, email, EMAIL_LEN))
			{
				/* Found a match */
				free(key); /* free this b4 getting out */
				break;
			}
		}
	  	okey = key;
     	}

	DbClose(&dbstruct);

	return netInfo;
}

int
StoreVpnG(DB db, VpnGroupEntry *vpnGroupEntry)
{
     VpnGroupEntry *tmp;

     tmp = DbFindVpnGEntry(db, (char *)vpnGroupEntry, sizeof(VpnGroupKey));

     if (tmp)
     {
	free(tmp);
	return 0;
     }

     if (DbStoreVpnGEntry(db, vpnGroupEntry, 
			 vpnGroupEntry, sizeof(VpnGroupKey)) < 0)
     {
	  NETERROR(MDB, ("StoreVpnG():database store error \n"));
	  return -1;
     }

     return 1;
}

/* Will add a new vpn with an extension length, but with or w/o a vpnG.
 * Will allow the change of the vpnG of a an existing vpn entry, but not
 * its extension length.
 * If this function is used. The databases should be opened.
 * The return value should be checked. If positive,
 * the function AgeIedgesInVpnGs should be called after
 * closing the databases. See StoreVpnInDb for details.
 */
int
StoreVpn(DB db, VpnEntry *vpnEntry,
	char *oldVpnGroup, char *newVpnGroup)
{
     VpnEntry *tmp;

     if (strlen(vpnEntry->vpnName) == 0)
     {
	return -1;
     }

     memset(oldVpnGroup, 0, VPN_GROUP_LEN);

     tmp = DbFindVpnEntry(db, (char *)vpnEntry, sizeof(VpnKey));

     if (tmp)
     {
	 /* Vpn Exists... See if the extension lengths match,
	  * and the group is same.
	  */ 
	 if (tmp->vpnExtLen != vpnEntry->vpnExtLen)
	 {
		memcpy(vpnEntry, tmp, sizeof(VpnEntry));
		free(tmp);
		return -1;
	 }
	 else
	 {
		if (strlen(newVpnGroup) == 0)
		{
			/* Dont check the vpn groups */
			memcpy(vpnEntry, tmp, sizeof(VpnEntry));
			free(tmp);
			return 0;
		}

		/* Check to see if the group is the same */
		if (strcmp(vpnEntry->vpnGroup, tmp->vpnGroup) == 0)
		{
			memcpy(vpnEntry, tmp, sizeof(VpnEntry));
			free(tmp);
			return 0;
		}
	 }
     	strcpy(oldVpnGroup, tmp->vpnGroup);
     }

	strcpy(newVpnGroup, vpnEntry->vpnGroup);

     /* At this point, either the vpn is a new one, or its
      * group has changed.
      */
     if (DbStoreVpnEntry(db, vpnEntry, 
			 vpnEntry, sizeof(VpnKey)) < 0)
     {
	return -1;
     }

	if (tmp)
	{
     		memcpy(vpnEntry, tmp, sizeof(VpnEntry));
     		free(tmp);
	}

     return 1;
}

/* Age all iedges in vpn groups 1 or 2 */
int
MemAgeIedgesInVpnGs(char *vpng1, char *vpng2)
{
     	char fn[] = "MemAgeIedgesInVpnGs():";
	VpnEntry vpnEntry;
	CacheVpnEntry *cacheVpnEntry;	
     	NetoidInfoEntry *netInfo;
     	CacheTableInfo *info;
	int i;

	memset(&vpnEntry, 0, sizeof(VpnEntry));

	for (info = CacheGetFirst(regCache); info;
		info = CacheGetNext(regCache, &info->data))
	{
		netInfo = &info->data;
		if (!(netInfo->stateFlags & CL_ACTIVE))
		{
			continue;
		}

		memset(&vpnEntry, 0, sizeof(VpnEntry));
		
		strcpy(vpnEntry.vpnName, netInfo->vpnName);
						
		cacheVpnEntry = (CacheVpnEntry *)
			CacheGet(vpnCache, &vpnEntry);

		if ( (cacheVpnEntry && vpng1 && 
				!strcmp(vpng1, cacheVpnEntry->vpnEntry.vpnGroup)) ||
			(cacheVpnEntry && vpng2 && 
				!strcmp(vpng2, cacheVpnEntry->vpnEntry.vpnGroup)) )
		{
			/* Force this iedge to re-register */
			netInfo->stateFlags &= ~CL_REGISTERED;
		}
							
	} 

	CacheFreeIterator(regCache);
	return(0);
}

char *
GuessVpnPhone(InfoEntry *entry, char *inphone, char *outphone, 
		unsigned long *extn)
{
     	char fn[] = "GuessnVpnPhone():";
	int prefixLen;

     	/* Guess the phone based on Vpn Information
      	* contained in entry
      	*/
	memcpy(outphone, inphone, PHONE_NUM_LEN);
	*extn = entry->vpnExtLen;
     
     	if ((entry == NULL) || (inphone == NULL))
     	{
		return NULL;
     	}

     	if (strlen(inphone) == entry->vpnExtLen)
     	{
	  	DEBUG(MFIND, NETLOG_DEBUG4,
		("%s This is an internal Vpn Phone Number\n", fn));

		prefixLen = strlen(entry->vpnPhone) - entry->vpnExtLen;

	  	strncpy(outphone, entry->vpnPhone, prefixLen);
		outphone[prefixLen] = '\0';

	  	nx_strlcat(outphone, inphone, PHONE_NUM_LEN);
	  	DEBUG(MFIND, NETLOG_DEBUG4, ("%s Guessed Phone %s\n",
	      		fn, outphone));
	  	return outphone;
     	}
     	else
     	{
		*extn = strlen(outphone);
	  	return outphone;
     	}
}

/*
 * With DMR Longest Match is now in the context 
 * of <ipaddress, realmid) i.e. find the longest matching
 * subnet for the ip address inside a realm
 */
CacheTableInfo *
GetIedgeLongestMatch(RealmSubnet *rmsub)
{
	return GetSubnetLongestMatch(subnetCache, (Subnet *)rmsub);
}

void *
GetSubnetLongestMatch(cache_t cache, Subnet *sub)
{
	void *entry = NULL;
	int i = 0;

	sub->mask = 0xffffffff;

	while (sub->mask)
	{
		entry = CacheGet(cache, sub);
	
		if (entry)
		{
			return entry;
		}
		
		sub->mask &= ~(0x1<<i++);
	}

	return NULL;
}

CacheTableInfo *
GetIedge(InfoEntry *netInfo)
{
	CacheTableInfo *cacheInfo = NULL;
	RealmIP realmip;
	RealmSubnet rmsub;

	/* Use each of the attributes of the iedge to locate it */
	cacheInfo = CacheGet(regCache, netInfo);

	if ((cacheInfo == NULL) && BIT_TEST(netInfo->sflags, ISSET_PHONE))
	{
		cacheInfo = CacheGet(phoneCache, netInfo->phone);
	}

#ifdef DISABLE_VP_CACHE
	if ((cacheInfo == NULL) && BIT_TEST(netInfo->sflags, ISSET_VPNPHONE))
	{
		cacheInfo = CacheGet(vpnPhoneCache, netInfo->vpnPhone);
	}
#endif
	if ((cacheInfo == NULL) && strlen(netInfo->uri))
	{
		cacheInfo = CacheGet(uriCache, netInfo->uri);
	}

#ifdef _use_emails_
	if ((cacheInfo == NULL) && strlen(netInfo->email))
	{
		cacheInfo = CacheGet(emailCache, netInfo->email);
	}
#endif

        if ((cacheInfo == NULL) && strlen(netInfo->h323id))
	{
		cacheInfo = CacheGet(h323idCache, netInfo->h323id);
	}
        
        if ((cacheInfo == NULL) && strlen(netInfo->tg))
	{
		cacheInfo = CacheGet(tgCache, netInfo->tg);
	}
        
	if ((cacheInfo == NULL) && IsGateway(netInfo))
	{
		cacheInfo = CacheGet(gwCache, netInfo);
	}

	if ((cacheInfo == NULL) && BIT_TEST(netInfo->sflags, ISSET_IPADDRESS) &&
		 ((netInfo->stateFlags & CL_ACTIVE)||(netInfo->stateFlags & CL_STATIC)))
	{
		realmip.ipaddress = netInfo->ipaddress.l;
		realmip.realmId = netInfo->realmId;
		cacheInfo = CacheGet(ipCache, &realmip);
	}

	if ((cacheInfo == NULL) && (netInfo->subnetip || netInfo->subnetmask))
	{
		rmsub.subnetip = netInfo->ipaddress.l;
		rmsub.realmId = netInfo->realmId;
		rmsub.mask = netInfo->subnetmask;
		cacheInfo = CacheGet(subnetCache, &rmsub);
	}

	if ((cacheInfo == NULL) && BIT_TEST(netInfo->sflags, ISSET_REGID))
	{
		cacheInfo = CacheGet(regidCache, netInfo->regid);
	}

	return cacheInfo;
}

CacheTableInfo *
GetDuplicateIedge(InfoEntry *netInfo)
{
	CacheTableInfo *cacheInfo = NULL;
	RealmSubnet  rmsub;

	if ((cacheInfo == NULL) && BIT_TEST(netInfo->sflags, ISSET_PHONE) &&
		!IsGatekeeper(netInfo))
	{
		cacheInfo = CacheGet(phoneCache, netInfo->phone);
		if (cacheInfo && (memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)) ||
							strcmp(cacheInfo->data.phone, netInfo->phone)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}

	if ((cacheInfo == NULL) && BIT_TEST(netInfo->sflags, ISSET_VPNPHONE))
	{
		cacheInfo = CacheGet(vpnPhoneCache, netInfo->vpnPhone);
		if (cacheInfo && memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}

	if ((cacheInfo == NULL) && strlen(netInfo->uri))
	{
		cacheInfo = CacheGet(uriCache, netInfo->uri);
		if (cacheInfo && (memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)) ||
							strcmp(cacheInfo->data.uri, netInfo->uri)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}


#ifdef _use_emails_
	if ((cacheInfo == NULL) && strlen(netInfo->email) &&
		!IsGatekeeper(netInfo))
	{
		cacheInfo = CacheGet(emailCache, netInfo->email);
		if (cacheInfo && memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}
#endif

	if ((cacheInfo == NULL) && strlen(netInfo->h323id) && !IsGatekeeper(netInfo))
	{
		cacheInfo = CacheGet(h323idCache, netInfo->h323id);
		if (cacheInfo && (memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)) ||
					strcmp(cacheInfo->data.h323id, netInfo->h323id)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}

	if ((cacheInfo == NULL) && strlen(netInfo->tg))
	{
		cacheInfo = CacheGet(tgCache, netInfo->tg);
		if (cacheInfo && (memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)) ||
					strcmp(cacheInfo->data.tg, netInfo->tg)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}

	if ((cacheInfo == NULL) && BIT_TEST(netInfo->sflags, ISSET_IPADDRESS) &&
		 ((netInfo->stateFlags & CL_ACTIVE)||(netInfo->stateFlags & CL_STATIC)))
	{
		cacheInfo = GetDuplicateIedgeIpAddr(ipCache, netInfo);
		if (cacheInfo)
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}

    if ((cacheInfo == NULL) && (netInfo->subnetip || netInfo->subnetmask))
	{
		rmsub.subnetip = netInfo->subnetip;
		rmsub.mask = netInfo->subnetmask;
		rmsub.realmId = netInfo->realmId;
		cacheInfo = CacheGet(subnetCache, &rmsub);
		if (cacheInfo && memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}

	if ((cacheInfo == NULL) && IsGateway(netInfo))
	{
		cacheInfo = CacheGet(gwCache, netInfo);
		if (cacheInfo && memcmp(&cacheInfo->data, netInfo, sizeof(NetoidSNKey)))
		{
			return cacheInfo;
		}
		else
		{
			cacheInfo = NULL;
		}
	}

	return cacheInfo;
}

int
ExtractIedge(char *key, void *copy, size_t size)
{
	InfoEntry *tmp;

	tmp = SearchNetoidDatabase (key, sizeof(NetoidSNKey));
	if (tmp)
	{
		memcpy(copy, tmp, size);
		free(tmp);

		return 1;
	}

	return -1;
}

int
FindIedge(InfoEntry *netInfo, void *copy, size_t size)
{
	CacheTableInfo *cacheInfo = NULL;

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = GetIedge(netInfo);

	if (cacheInfo)
	{
		memcpy(copy, cacheInfo, size);
	}

	CacheReleaseLocks(regCache);

	if (cacheInfo)
	{
		return 1;
	}
	else
	{
		return -1;
	}	
}

/* Delete An Iedge from the caches.
 * Only iedges with the specified serial number will be deleted.
 * This function uses netInfo as an index (serial no index),
 * to delete the iedge.
 */
CacheTableInfo *
DeleteIedge(InfoEntry *netInfo)
{
	char fn[] = "DeleteIedge():";
	CacheTableInfo *cacheInfo = NULL, *tmpCacheInfo = NULL;
	RealmSubnet rmsub;
	CacheTableInfo *entryList, *tmp, *startEntryList;
	int found = 0;

	cacheInfo = CacheDelete(regCache, netInfo);

	if (cacheInfo == NULL)
	{
		return NULL;
	}

	netInfo = &cacheInfo->data;

	if (BIT_TEST(netInfo->sflags, ISSET_PHONE))
	{
		tmpCacheInfo = CacheGet(phoneCache, netInfo->phone);
		if (tmpCacheInfo == cacheInfo)
		{
			CacheDelete(phoneCache, netInfo->phone);
		}
	}

#ifdef DISABLE_VP_CACHE
	if (BIT_TEST(netInfo->sflags, ISSET_VPNPHONE))
	{
		tmpCacheInfo = CacheGet(vpnPhoneCache, netInfo->vpnPhone);
		if (tmpCacheInfo == cacheInfo)
		{
			CacheDelete(vpnPhoneCache, netInfo->vpnPhone);
		}
	}
#endif

	DeleteIedgeIpAddr(ipCache, netInfo);
	
	if (netInfo->subnetip || netInfo->subnetmask)
	{
		 rmsub.subnetip = netInfo->subnetip;
		 rmsub.mask = netInfo->subnetmask;
		 rmsub.realmId = netInfo->realmId;
		 tmpCacheInfo = CacheGet(subnetCache, &rmsub);
		 if (tmpCacheInfo == cacheInfo)
		 {
			  CacheDelete(subnetCache, &rmsub);
		 }
	}

	if (BIT_TEST(netInfo->sflags, ISSET_REGID))
	{
		DeleteIedgeRegid(regidCache, netInfo);
	}

	if (strlen(netInfo->uri))
	{
		 tmpCacheInfo = CacheGet(uriCache, netInfo->uri);
		 if (tmpCacheInfo == cacheInfo)
		 {
			  CacheDelete(uriCache, netInfo->uri);
		 }
	}

#ifdef _use_emails_
	if (strlen(netInfo->email))
	{
		 tmpCacheInfo = CacheGet(emailCache, netInfo->email);
		 if (tmpCacheInfo == cacheInfo)
		 {
			  CacheDelete(emailCache, netInfo->email);
		 }
	}
#endif


	if (strlen(netInfo->h323id))
	{
		 tmpCacheInfo = CacheGet(h323idCache, netInfo->h323id);
		 if (tmpCacheInfo == cacheInfo)
		 {
			  CacheDelete(h323idCache, netInfo->h323id);
		 }
	}

	if (strlen(netInfo->tg))
	{
		 tmpCacheInfo = CacheGet(tgCache, netInfo->tg);
		 if (tmpCacheInfo == cacheInfo)
		 {
			  CacheDelete(tgCache, netInfo->tg);
		 }
	}

	if (strlen(netInfo->dtg))
	{
		if (startEntryList = entryList = CacheDelete(dtgCache, netInfo->dtg))
		{
			do
			{
				if (entryList == cacheInfo) found = 1;
				entryList = (CacheTableInfo*) Listg(entryList, sizeof(ListEntry))->next;
			} while (entryList != startEntryList);

			if (found && !ListgIsSingle(entryList, sizeof(ListEntry)))
			{
				// The new head can be the next of info
				entryList = (CacheTableInfo*) Listg(cacheInfo, sizeof(ListEntry))->next;
		
				// Delete this element from the list
				ListgDelete(cacheInfo, sizeof(ListEntry));
		
				// insert the list back
				if (CacheProbeInsert(dtgCache, entryList, (void**)&tmp) < 0)
				{
					NETERROR(MSHM, ("%s CacheProbeInsert Error\n", fn));
				}
			}
		}
	}

	DeleteGw(netInfo);

	if (IsSGatekeeper(netInfo))
	{
		CacheGkInfo *gkInfo;

		gkInfo = CacheDelete(gkCache, netInfo);

		/* Must free this here, as this is hidden from top
		 * level API
		 */
		CFree(gkCache)(gkInfo);

		if (crids)
		{
			tmpCacheInfo = CacheGet(cridCache, &netInfo->crId);
			if (tmpCacheInfo == cacheInfo)
			{
				CacheDelete(cridCache, &netInfo->crId);
			}
		}
	}

	return cacheInfo;
}

int
AddIedgePortToIpCache(CacheTableInfo *ipCacheInfo, InfoEntry *entry)
{
         char fn[] = "AddIedgePortToIpCache():";

         if (entry->uport >= MAX_IEDGE_PORTS)
         {
                  NETERROR(MCACHE, ("%s Iedge ports %lu exceeds max ports\n",
                                                        fn, entry->uport));
                  return -1;
         }

         if (!BITA_TEST(ipCacheInfo->data.ports, entry->uport))
         {
                  /* Increment the noprts by 1 */
                  ipCacheInfo->data.nports ++;
                  BITA_SET(ipCacheInfo->data.ports, entry->uport);
                  return 1;
         }
         return 0;
}

int
AddIedgePortToRegidCache(CacheTableInfo *regCacheInfo, InfoEntry *entry)
{
	 char fn[] = "AddIedgePortToRegidCache():";

	 if (entry->uport >= MAX_IEDGE_PORTS)
	 {
		  NETERROR(MCACHE, ("%s Iedge ports %lu exceeds max ports\n",
							fn, entry->uport));
		  return -1;
	 }

	 if (!BITA_TEST(regCacheInfo->data.cfgports, entry->uport))
	 {
		  /* Increment the noprts by 1 */
		  regCacheInfo->data.ncfgports ++;
		  BITA_SET(regCacheInfo->data.cfgports, entry->uport);
		  return 1;
	 }

	 return 0;
}

int
DeleteIedgePortFromIpCache(CacheTableInfo *ipCacheInfo, InfoEntry *entry)
{
         char fn[] = "DeleteIedgePortFromIpCache():";

         if (entry->uport >= MAX_IEDGE_PORTS)
         {
                  NETERROR(MCACHE, ("%s Iedge ports %lu exceeds max ports\n",
                                                        fn, entry->uport));
                  return -1;
         }

         if (BITA_TEST(ipCacheInfo->data.ports, entry->uport))
         {
                  /* Increment the noprts by 1 */
                  ipCacheInfo->data.nports --;
                  BITA_RESET(ipCacheInfo->data.ports, entry->uport);
                  return 1;
         }

         return 0;
}

int
DeleteIedgePortFromRegidCache(CacheTableInfo *regCacheInfo, InfoEntry *entry)
{
	 char fn[] = "DeleteIedgePortFromRegidCache():";

	 if (entry->uport >= MAX_IEDGE_PORTS)
	 {
		  NETERROR(MCACHE, ("%s Iedge ports %lu exceeds max ports\n",
							fn, entry->uport));
		  return -1;
	 }

	 if (BITA_TEST(regCacheInfo->data.cfgports, entry->uport))
	 {
		  /* Increment the noprts by 1 */
		  regCacheInfo->data.ncfgports --;
		  BITA_RESET(regCacheInfo->data.cfgports, entry->uport);
		  return 1;
	 }
	 
	 return 0;
}

int
AddIedgeIpCacheEntry(CacheTableInfo *cacheInfo)
{
	 /* Does not exist, add this */
	 if (CacheInsert(ipCache, cacheInfo) < 0)
	 {
		  return -1;
	 }
	 else
	 {
		  memset(cacheInfo->data.ports, 0, PORT_ARRAY_LEN);

		  cacheInfo->data.nports = 1;
		  BITA_SET(cacheInfo->data.ports, cacheInfo->data.uport);

		  return 1;
	 }
}

int
AddIedgeRegidCacheEntry(CacheTableInfo *cacheInfo)
{
	 /* Does not exist, add this */
	 if (CacheInsert(regidCache, cacheInfo) < 0)
	 {
		  return -1;
	 }
	 else
	 {
		  memset(cacheInfo->data.cfgports, 0, PORT_ARRAY_LEN);

		  cacheInfo->data.ncfgports = 1;
		  BITA_SET(cacheInfo->data.cfgports, cacheInfo->data.uport);

		  return 1;
	 }
}

/* -1 means an error
 * 0 means no error, and no addition took place.
 * 1 means success
 */
int
AddIedgeIpAddr(cache_t ipCache, CacheTableInfo *cacheInfo)
{
	 char fn[] = "AddIedgeIpAddr():";
	 CacheTableInfo *ipCacheInfo;
	 NetoidInfoEntry *netInfo;
	 RealmIP  realmip;

	 /* Search for the <IP address,realm Id> in the cache.
	  * If not found, we add it and set the nports to 0.
	  * If found, and its reg-id matches with what we are
	  * trying to add, then we return success, and increase
	  * the noprts by 1.
	  * If reg-id doesnt match, then its a failure.
	  */
	 if (!BIT_TEST(cacheInfo->data.sflags, ISSET_IPADDRESS) ||
		 (cacheInfo->data.ipaddress.l == 0))
	 {
		  /* No ip address */
		  return 0;
	 }

	 if (cacheInfo->data.uport >= MAX_IEDGE_PORTS)
	 {
		  NETERROR(MCACHE, ("%s Iedge ports %lu exceeds max ports\n",
							fn, cacheInfo->data.uport));
		  return -1;
	 }

	 netInfo = &cacheInfo->data;
	 realmip.ipaddress = netInfo->ipaddress.l;
	 realmip.realmId = netInfo->realmId;

	 ipCacheInfo = CacheGet(ipCache, &realmip);
	 if (ipCacheInfo)
	 {
		  if (!memcmp(&ipCacheInfo->data, netInfo, REG_ID_LEN))
		  {
			   AddIedgePortToIpCache(ipCacheInfo, netInfo);

			   return 1;
		  }
		  else
		  {
			   /* reg-id does not match */
			   return -1;
		  }
	 }
	 else
	 {
		  /* Does not exist, add this */
		  return AddIedgeIpCacheEntry(cacheInfo);
	 }

	 return -1;
}

/* -1 means an error
 * 0 means no error, and no addition took place.
 * 1 means success
 */
int
AddIedgeRegid(cache_t regidCache, CacheTableInfo *cacheInfo)
{
	 char fn[] = "AddIedgeRegid():";
	 CacheTableInfo *regCacheInfo;
	 NetoidInfoEntry *netInfo;

	 /* Search for the IP address in the cache.
	  * If not found, we add it and set the nports to 0.
	  * If found, and its reg-id matches with what we are
	  * trying to add, then we return success, and increase
	  * the noprts by 1.
	  * If reg-id doesnt match, then its a failure.
	  */
	 if (!BIT_TEST(cacheInfo->data.sflags, ISSET_REGID) ||
		 (strlen(cacheInfo->data.regid) == 0))
	 {
		  /* No regid */
		  return 0;
	 }

	 if (cacheInfo->data.uport >= MAX_IEDGE_PORTS)
	 {
		  NETERROR(MCACHE, ("%s Iedge ports %lu exceeds max ports\n",
							fn, cacheInfo->data.uport));
		  return -1;
	 }

	 netInfo = &cacheInfo->data;

	 regCacheInfo = CacheGet(regidCache, netInfo->regid);

	 if (regCacheInfo)
	 {
		  AddIedgePortToRegidCache(regCacheInfo, netInfo);
		  return 1;
	 }
	 else
	 {
		  /* Does not exist, add this */
		  return AddIedgeRegidCacheEntry(cacheInfo);
	 }

	 return -1;
}

/* return -1 if delete failed,
 * 0 if it was ok, but nothing happened.
 * 1 if something did get deleted
 */
int
DeleteIedgeIpAddr(cache_t ipCache, NetoidInfoEntry *netInfo)
{
	 char fn[] = "DeleteIedgeIpAddr():";
	 CacheTableInfo *ipCacheInfo = NULL, *tmpCacheInfo;
	 PhoNode phonode;
	 int rc = 0, i;
	 RealmIP  realmip;

	 /* Find the ipaddress cache entry.
	  * If the entry matches the reg-id we have here,
	  * remove the port from the port map.
	  * If this port was the last one, delete the
	  * the ip cache entry
	  * Special case: When the iedge port being taken out is
	  * the one we are using as refernce in our ip cache,
	  * we need to change the reference.
	  */

	 realmip.ipaddress = netInfo->ipaddress.l;
	 realmip.realmId = netInfo->realmId;

	 ipCacheInfo = CacheGet(ipCache, &realmip);
	 
	 if (ipCacheInfo)
	 {
		  if (!memcmp(&ipCacheInfo->data, netInfo, REG_ID_LEN))
		  {
			   rc = DeleteIedgePortFromIpCache(ipCacheInfo, netInfo);

			   if (ipCacheInfo->data.nports <= 0)
			   {
					/* Delete this iedge */
					CacheDelete(ipCache, &realmip);
					memset(ipCacheInfo->data.ports, 0, PORT_ARRAY_LEN);
			   }
			   else if (ipCacheInfo->data.uport == netInfo->uport)
			   {
					/* Delete this iedge */
					CacheDelete(ipCache, &realmip);

					memcpy(&phonode, &ipCacheInfo->data,
						sizeof(NetoidSNKey));

					/* We need to change reference point */
					for (i=0; i<MAX_IEDGE_PORTS; i++)
					{
						if (!BITA_TEST(ipCacheInfo->data.ports, i))
						{
							continue;
						}
						phonode.uport = i;
						tmpCacheInfo = CacheGet(regCache, &phonode);
						if (tmpCacheInfo && (tmpCacheInfo->data.ipaddress.l == netInfo->ipaddress.l) && (tmpCacheInfo->data.realmId == netInfo->realmId) )
						{
							/* Add this in the ip cache */
							AddIedgeIpAddr(ipCache, tmpCacheInfo);
							memcpy(tmpCacheInfo->data.ports, 
								ipCacheInfo->data.ports, PORT_ARRAY_LEN);
							tmpCacheInfo->data.nports = 
								ipCacheInfo->data.nports;

							// reset the deleted entry now
							memset(ipCacheInfo->data.ports, 0, PORT_ARRAY_LEN);
							ipCacheInfo->data.nports = 0;

							break;
						}
					}
			   }
			   
			   return rc;
		  }
		  else
		  {
			   /* reg-id does not match */
			   return -1;
		  }
	 }
	 else
	 {
		  /* We didnt find anything */
		  return 0;
	 }
	
	return -1;
}

/* return -1 if delete failed,
 * 0 if it was ok, but nothing happened.
 * 1 if something did get deleted
 */
int
DeleteIedgeRegid(cache_t regidCache, NetoidInfoEntry *netInfo)
{
	 char fn[] = "DeleteIedgeRegid():";
	 CacheTableInfo *regCacheInfo = NULL, *tmpCacheInfo;
	 PhoNode phonode;
	 int rc = 0, i;

	 /* Find the ipaddress cache entry.
	  * If the entry matches the reg-id we have here,
	  * remove the port from the port map.
	  * If this port was the last one, delete the
	  * the ip cache entry
	  */

	if (BIT_TEST(netInfo->sflags, ISSET_REGID))
	{
		 regCacheInfo = CacheGet(regidCache, netInfo->regid);
		 
		 if (regCacheInfo)
		 {
				rc = DeleteIedgePortFromRegidCache(regCacheInfo, netInfo);

				if (regCacheInfo->data.ncfgports <= 0)
				{
					/* Delete this iedge */
					CacheDelete(regidCache, netInfo->regid);
		  			memset(regCacheInfo->data.cfgports, 0, PORT_ARRAY_LEN);
				}
				else if (regCacheInfo->data.uport == netInfo->uport)
				{
					/* Delete this iedge */
					CacheDelete(regidCache, netInfo->regid);

					memcpy(&phonode, &regCacheInfo->data,
							sizeof(NetoidSNKey));

					/* We need to change reference point */
					for (i=0; i<MAX_IEDGE_PORTS; i++)
					{
         				if (!BITA_TEST(regCacheInfo->data.cfgports, i))
						{
							continue;
						}
						phonode.uport = i;
						tmpCacheInfo = CacheGet(regCache, &phonode);
						if (tmpCacheInfo)
						{
							/* Add this in the ip cache */
							AddIedgeRegid(regidCache, tmpCacheInfo);
							memcpy(tmpCacheInfo->data.cfgports, 
								regCacheInfo->data.cfgports, PORT_ARRAY_LEN);
							tmpCacheInfo->data.ncfgports = 
								regCacheInfo->data.ncfgports;

				  			memset(regCacheInfo->data.cfgports, 0, PORT_ARRAY_LEN);
							regCacheInfo->data.ncfgports = 0;

							break;
						}
					}
				}
				   
				return rc;
		 }
		 else
		 {
			  /* We didnt find anything */
			  return 0;
		 }
	}
	else
	{
		 return 0;
	}
	
	return -1;
}

CacheTableInfo *
GetDuplicateIedgeIpAddr(cache_t ipCache, NetoidInfoEntry *netInfo)
{
	 char fn[] = "GetDuplicateIedgeIpAddr():";
	 CacheTableInfo *ipCacheInfo = NULL;
	 RealmIP  realmip;

	 /* Get the ip address from the cache, if its there,
	  * then we check the reg-id, if the reg-id's dont
	  * match, then we have a duplicate, otherwise, its
	  * not a duplicate
	  */
	 if (BIT_TEST(netInfo->sflags, ISSET_IPADDRESS))
	 {
		  realmip.ipaddress = netInfo->ipaddress.l;
		  realmip.realmId = netInfo->realmId;
		  ipCacheInfo = CacheGet(ipCache, &realmip);
		 
		  if (ipCacheInfo)
		  {
			   if (memcmp(&ipCacheInfo->data, netInfo, REG_ID_LEN))
			   {
					return ipCacheInfo;
			   }
			   else
			   {
					/* reg-id matches */
					return NULL;
			   }
		  }
		  else
		  {
			   return NULL;
		  }
	}
	else
	{
		 return NULL;
	}
	
	return NULL;
}

#if 0  /* nobody using this function . functions in entry.c and process.c 
        * are ignored
        */

CacheTableInfo *
CheckDuplicateIedgeIpAddr(cache_t ipCache, NetoidInfoEntry *netInfo, unsigned long ipaddr)
{
	 char fn[] = "GetDuplicateIedgeIpAddr():";
	 CacheTableInfo *ipCacheInfo = NULL;
	 RealmIP    realmip;

	 /* Get the ip address from the cache, if its there,
	  * then we check the reg-id, if the reg-id's dont
	  * match, then we have a duplicate, otherwise, its
	  * not a duplicate
	  */
	 if (BIT_TEST(netInfo->sflags, ISSET_IPADDRESS))
	 {
		  realmip.ipaddress = ipaddr;
		  realmip.realmId = netInfo->realmId;
		  ipCacheInfo = CacheGet(ipCache, &realmip);
		 
		  if (ipCacheInfo)
		  {
			   if (memcmp(&ipCacheInfo->data, netInfo, REG_ID_LEN))
			   {
					return ipCacheInfo;
			   }
			   else
			   {
					/* reg-id matches */
					return NULL;
			   }
		  }
		  else
		  {
			   return NULL;
		  }
	}
	else
	{
		 return NULL;
	}
	
	return NULL;
}
#endif

int
AddIedge(CacheTableInfo *cacheInfo)
{
	int result = 0, retval = 0;
	InfoEntry *info = &cacheInfo->data;
	CacheTableInfo *entryList, *tmp;

	if (BIT_TEST(info->sflags, ISSET_REGID))
	{
		result = AddIedgeRegid(regidCache, cacheInfo);
	}

	if (result < 0)
	{
		retval = result;
	}

	result = CacheInsert(regCache, cacheInfo);

	if (result < 0)
	{
		retval = result;
	}

	if (BIT_TEST(info->sflags, ISSET_PHONE))
	{
		if (!IsGatekeeper(info))
		{
			result = CacheInsert(phoneCache, cacheInfo);
		}
	}

	if (info->subnetip || info->subnetmask)
	{
		//if (IsSubnetEndpoint(info))
		{
			result = CacheInsert(subnetCache, cacheInfo);
		}
	}

	if (result < 0)
	{
		retval = result;
	}

	if (strlen(info->uri))
	{
		if (!IsGatekeeper(info))
		{
			result = CacheInsert(uriCache, cacheInfo);
		}
	}

	if (result < 0)
	{
		retval = result;
	}

#ifdef _use_emails_
	if (strlen(info->email))
	{
		if (!IsGatekeeper(info))
		{
			result = CacheInsert(emailCache, cacheInfo);
		}
	}
#endif

	if (result < 0)
	{
		retval = result;
	}
        
	if (strlen(info->h323id))
	{
		if (!IsGatekeeper(info))
		{
			result = CacheInsert(h323idCache, cacheInfo);
		}
	}

	if (strlen(info->tg))
	{
		if (!IsGatekeeper(info))
		{
			result = CacheInsert(tgCache, cacheInfo);
		}
	}

	if (strlen(info->dtg))
	{
		if (!IsGatekeeper(info))
		{
			ListgInitElem(cacheInfo, sizeof(ListEntry));
			entryList = NULL;
			CacheProbeInsert(dtgCache, cacheInfo, (void**)&entryList);
			if (entryList)
			{
				// entry was not inserted
				ListgInsert(entryList, cacheInfo, sizeof(ListEntry));
			}
		}
	}

	if (result < 0)
	{
		retval = result;
	}

	AddGw(cacheInfo);

	if (result < 0)
	{
		retval = result;
	}

	if (IsSGatekeeper(info))
	{
		CacheGkInfo *gkInfo;

		/* Must allocate something in the gkCache */
		gkInfo = CMalloc(gkCache)(sizeof(CacheGkInfo));	
		if (gkInfo)
		{
			memset(gkInfo, 0, sizeof(CacheGkInfo));
			memcpy(gkInfo, &cacheInfo->data, sizeof(NetoidSNKey));

			gkInfo->crId = info->crId;

			if (info->stateFlags & CL_ACTIVE) 
			{
				if (info->stateFlags & CL_REGISTERED) 
				{
					gkInfo->regState = GKREG_REGISTERED;
				}
				else
				{
					gkInfo->regState = GKREG_REGISTERING;
				}
			}
			else
			{
				gkInfo->regState = GKREG_TRYALT;
			}
			result = CacheInsert(gkCache, gkInfo);
		}

		// Add into the crid cache
		if (crids && (info->stateFlags & CL_ACTIVE))
		{
			CacheInsert(cridCache, cacheInfo);
		}
	}

	if (result < 0)
	{
		retval = result;
	}

	if (BIT_TEST(info->sflags, ISSET_IPADDRESS) &&
			((info->stateFlags & CL_ACTIVE) ||
			 (info->stateFlags & CL_STATIC)))
	{
		result = AddIedgeIpAddr(ipCache, cacheInfo);
	}

	if (result < 0)
	{
		retval = result;
	}

	return retval;
}

int
HandleVpnPrefixChange(VpnEntry *vpnEntry)
{
	char fn[] = "HandleVpnPrefixChange():";
	CacheTableInfo *info;
	int rc = 0;

	/* Walk all iedges in cache, change phones of those
	 * in this vpn, and write them to the database.
	 * The problem we face is that once we change the phone
	 * of a cache entry, 
	 */

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (info = CacheGetFirst(regCache); info;
		info = CacheGetNext(regCache, &info->data))
	{
		if (!strcmp(info->data.vpnName, vpnEntry->vpnName))
		{
			/* This iedge is in the concerned vpn
			 */
			if (BIT_TEST(info->data.sflags, ISSET_PHONE))
			{
				CacheDelete(phoneCache, info->data.phone);
			}

			AssignIedgePhone(&info->data, vpnEntry);

			if (BIT_TEST(info->data.sflags, ISSET_PHONE))
			{
				CacheInsert(phoneCache, info);
			}

			UpdateNetoidDatabase(&info->data);
		}
	}

	CacheFreeIterator(regCache);

_release_locks:
	CacheReleaseLocks(regCache);

	return(rc);
}

int
HandleVpnPrefixChangeDb(VpnEntry *vpnEntry)
{
	char fn[] = "HandleVpnPrefixChangeDb():";
	DB db;
	DB_tDb dbstruct;
	NetoidSNKey *key, *okey;
	int keylen = sizeof(NetoidSNKey);
	InfoEntry *entry;

	/* Walk all iedges in the database, change the phone
	 * numbers and store them
	 */

	dbstruct.read_write = GDBM_WRCREAT;
	if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
	{
		NETERROR(MCACHE, ("%s Unable to open database %s\n",
			fn, NETOIDS_DB_FILE));
		goto _error;
	}

	for (key = (NetoidSNKey *)DbGetFirstInfoKey(db); key != 0;
		key = (NetoidSNKey *)DbGetNextInfoKey(db, (char *)key, keylen),
		free(okey), free(entry))
	{
		entry = DbFindInfoEntry(db, (char *)key, keylen);

		if (entry == NULL)
		{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
		}

		okey = key;

		if (!strcmp(entry->vpnName, vpnEntry->vpnName))
		{
			if (!BIT_TEST(entry->sflags, ISSET_PHONE))
			{
				continue;
			}

			AssignIedgePhone(entry, vpnEntry);

			/* Update the database */
			if (DbStoreInfoEntry(db, entry,
				(char *)entry, sizeof(NetoidSNKey)) < 0)
			{
				NETERROR(MCACHE, ("%s Db Store Error\n", fn));
			}
		}
	}

_return:
	/* Close the database */
	DbClose(&dbstruct);

	return 0;
_error:
	
	return -1;
}

int
AssignIedgePhone(InfoEntry *entry, VpnEntry *vpnEntry)
{
	char fn[] = "AssignIedgePhone():";

	if (!BIT_TEST(entry->sflags, ISSET_VPNPHONE))
	{
		memset(entry->phone, 0, PHONE_NUM_LEN);
		BIT_RESET(entry->sflags, ISSET_PHONE);
		return 0;
	}

	strcpy(entry->phone, entry->vpnPhone);
	BIT_SET(entry->sflags, ISSET_PHONE);

	if ((strlen(entry->vpnName) == 0) ||
			(vpnEntry == NULL) ||
			(strlen(vpnEntry->vpnName) == 0))
	{
		return 0;
	}

	strcpy(entry->phone, vpnEntry->prefix);
	nx_strlcat(entry->phone, entry->vpnPhone, PHONE_NUM_LEN);
	
	/* Cannot have a null phone */
	if (strlen(entry->phone) == 0)
	{
		BIT_RESET(entry->sflags, ISSET_PHONE);
	}

	return 0;
}

int
HandleVpnSipDomainChange(VpnEntry *vpnEntry)
{
	char fn[] = "HandleVpnSipDomainChange():";
	CacheTableInfo *info;
	int rc = 0;

	/* Walk all iedges in cache, change phones of those
	 * in this vpn, and write them to the database.
	 * The problem we face is that once we change the phone
	 * of a cache entry, 
	 */

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (info = CacheGetFirst(regCache); info;
		info = CacheGetNext(regCache, &info->data))
	{
		if (!strcmp(info->data.vpnName, vpnEntry->vpnName))
		{
			/* This iedge is in the concerned vpn
			 */
			if (BIT_TEST(info->data.sflags, ISSET_PHONE))
			{
				CacheDelete(phoneCache, info->data.phone);
			}

			if (BIT_TEST(info->data.sflags, ISSET_PHONE))
			{
				CacheInsert(phoneCache, info);
			}

			UpdateNetoidDatabase(&info->data);
		}
	}

	CacheFreeIterator(regCache);

_release_locks:
	CacheReleaseLocks(regCache);

	return(rc);
}

int
HandleVpnSipDomainChangeDb(VpnEntry *vpnEntry)
{
	char fn[] = "HandleVpnSipDomainChange():";
	DB db;
	DB_tDb dbstruct;
	NetoidSNKey *key, *okey;
	int keylen = sizeof(NetoidSNKey);
	InfoEntry *entry;

	/* Walk all iedges in the database, change the phone
	 * numbers and store them
	 */

	dbstruct.read_write = GDBM_WRCREAT;
	if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
	{
		NETERROR(MCACHE, ("%s Unable to open database %s\n",
			fn, NETOIDS_DB_FILE));
		goto _error;
	}

	for (key = (NetoidSNKey *)DbGetFirstInfoKey(db); key != 0;
		key = (NetoidSNKey *)DbGetNextInfoKey(db, (char *)key, keylen),
		free(okey), free(entry))
	{
		entry = DbFindInfoEntry(db, (char *)key, keylen);

		if (entry == NULL)
		{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
		}

		okey = key;

		if (!strcmp(entry->vpnName, vpnEntry->vpnName))
		{
			if (!BIT_TEST(entry->sflags, ISSET_PHONE))
			{
				continue;
			}

			/* Update the database */
			if (DbStoreInfoEntry(db, entry,
				(char *)entry, sizeof(NetoidSNKey)) < 0)
			{
				NETERROR(MCACHE, ("%s Db Store Error\n", fn));
			}
		}
	}

_return:
	/* Close the database */
	DbClose(&dbstruct);

	return 0;
_error:
	
	return -1;
}

/* Given a vpn id and a vpn group
 * figure out the vpn name. This function
 * provides a lot of backward compatibility
 * between rel 1.3 and prior releases
 */

int
VpnId2Name(char *vpnId, char *vpng, VpnEntry *vpnEntry)
{
	CacheVpnEntry *cacheVpnEntry;
	int rc = -1;

	/* Go over the list of vpns in the cache */
	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheVpnEntry = (CacheVpnEntry *)CacheGetFirst(vpnCache);
		cacheVpnEntry != NULL;
		cacheVpnEntry = (CacheVpnEntry *)CacheGetNext(vpnCache,
		&cacheVpnEntry->vpnEntry))
	{
		if (!strcmp(vpnId, cacheVpnEntry->vpnEntry.vpnId) &&
			!strcmp(vpng, cacheVpnEntry->vpnEntry.vpnGroup))
		{
			memcpy(vpnEntry, &cacheVpnEntry->vpnEntry, 
				sizeof(VpnEntry));
			rc = 1;
			break;
		}
	}

	CacheFreeIterator(vpnCache);

	CacheReleaseLocks(vpnCache);

	return rc;
}

/* We should be attached to the cache */
int
PropagateIedgeIpAddr(char *regid, unsigned long ipaddr)
{
	char fn[] = "PropagateIedgeIpAddr():";
	CacheTableInfo *regCacheInfo = NULL, *cacheInfo = NULL;
	NetoidSNKey key = {0};
	int i;

	/* Find all iedges with regid
	 * and see if all ports can be assigned the ipaddr
	 * If there is some iedge port which is registered
	 * and active/static with a different ip addr, 
	 * we will return an error. However we may not
	 * undo any operations which we may already have done
	 */

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	regCacheInfo = CacheGet(regidCache, regid);
	
	if (regCacheInfo == NULL)
	{
		goto _error;
	}

	strncpy(key.regid, regid, REG_ID_LEN);

	for (i=0; i<MAX_IEDGE_PORTS; i++)
	{
		if (BITA_TEST(regCacheInfo->data.cfgports, i))
		{
			key.uport = i;
			cacheInfo = CacheGet(regCache, &key);

			if (cacheInfo)
			{
				/* check the ipaddress */
				if (BIT_TEST(cacheInfo->data.sflags, ISSET_IPADDRESS) &&
					(cacheInfo->data.stateFlags & (CL_ACTIVE|CL_STATIC))&&
					(cacheInfo->data.ipaddress.l != ipaddr))
				{
					/* ip address mismatch */	
					NETERROR(MREGISTER, 
						("%s ip addr mismatch on %s/%d\n", 
						fn, regid, i));
				}
				else
				{
					DeleteIedgeIpAddr(ipCache, &cacheInfo->data);

					/* We can store the ipaddress */
					BIT_SET(cacheInfo->data.sflags, ISSET_IPADDRESS);
					cacheInfo->data.ipaddress.l = ipaddr;
					
					AddIedgeIpAddr(ipCache, cacheInfo);

					/* Store in Db */
					UpdateNetoidDatabase(&cacheInfo->data);
				}
			}
		}
	} 

_error:
	CacheReleaseLocks(regCache);

	return 1;
}

int
AddRoute(CacheRouteEntry *entry)
{
	char fn[] = "AddRoute()";
	int rc = 1;
	CacheRouteEntry *entryList;
	CacheCPBEntry *cachecpbEntry = NULL;

	if (entry == NULL)
	{
		return -1;
	}

	if (CacheInsert(cpCache, entry) < 0)
	{
		return -1;
	}

	if (entry->routeEntry.crflags & CRF_TEMPLATE)
	{
		// We are done
		return 0;
	}

	// Now we have to insert the route into the list entries
	ListInitElem(entry);

	if (strlen(entry->routeEntry.dest) ||
		strlen(entry->routeEntry.prefix) ||
		entry->routeEntry.destlen ||
		entry->routeEntry.crflags & CRF_DNISDEFAULT)
	{
		if (entry->routeEntry.crflags & CRF_TRANSIT)
		{
			CacheProbeInsert(cptransitCache, entry, (void**)&entryList);
		}
		else if (entry->routeEntry.crflags & CRF_CALLORIGIN)
		{
			CacheProbeInsert(cporigDNISCache, entry, (void**)&entryList);
		}
		else
		{
			CacheProbeInsert(cpdestCache, entry, (void**)&entryList);
		}

		if (entryList)
		{
			// entry was not inserted
			ListInsert(entryList, entry);	
		}
	}

	// Now we have to insert the route into the list entries
	entryList = NULL;

	ListgInitElem(entry, sizeof(ListEntry));

	// Transit routes are only DNIS based
	if (!(entry->routeEntry.crflags & CRF_TRANSIT))
	{
		if (strlen(entry->routeEntry.src) ||
			strlen(entry->routeEntry.srcprefix))
		{
			if (entry->routeEntry.crflags & CRF_CALLORIGIN)
			{
				CacheProbeInsert(cporigANICache, entry, (void**)&entryList);
			}
			else
			{
				CacheProbeInsert(cpsrcCache, entry, (void**)&entryList);
			}
	
			if (entryList)
			{
				// entry was not inserted
				ListgInsert(entryList, entry, sizeof(ListEntry));
			}
		}
	}

	if (entry->routeEntry.cpname[0] != '\0')
	{
		// Create a CPB entry
		cachecpbEntry = (CacheCPBEntry *) 
							MMalloc (cpbCache->malloc, sizeof(CacheCPBEntry));

		memset(cachecpbEntry, 0, sizeof(CacheCPBEntry));

		nx_strlcpy(cachecpbEntry->cpbEntry.cpname, entry->routeEntry.cpname, CALLPLAN_ATTR_LEN);
		nx_strlcpy(cachecpbEntry->cpbEntry.crname, entry->routeEntry.crname, CALLPLAN_ATTR_LEN);

		cachecpbEntry->cpbEntry.sTime.tm_year = TM_ANY;
		cachecpbEntry->cpbEntry.sTime.tm_yday = TM_ANY;
		cachecpbEntry->cpbEntry.sTime.tm_wday = TM_ANY;
        cachecpbEntry->cpbEntry.sTime.tm_mon = TM_ANY;
        cachecpbEntry->cpbEntry.sTime.tm_mday = TM_ANY;
        cachecpbEntry->cpbEntry.sTime.tm_hour = TM_ANY;
        cachecpbEntry->cpbEntry.sTime.tm_min = TM_ANY;
        cachecpbEntry->cpbEntry.sTime.tm_sec = TM_ANY;

		cachecpbEntry->cpbEntry.fTime.tm_year = TM_ANY;
		cachecpbEntry->cpbEntry.fTime.tm_yday = TM_ANY;
		cachecpbEntry->cpbEntry.fTime.tm_wday = TM_ANY;
        cachecpbEntry->cpbEntry.fTime.tm_mon = TM_ANY;
        cachecpbEntry->cpbEntry.fTime.tm_mday = TM_ANY;
        cachecpbEntry->cpbEntry.fTime.tm_hour = TM_ANY;
        cachecpbEntry->cpbEntry.fTime.tm_min = TM_ANY;
        cachecpbEntry->cpbEntry.fTime.tm_sec = TM_ANY;

		cachecpbEntry->cpbEntry.mTime = time(0);
	
		if (AddCPB(cachecpbEntry) < 0)	
		{
			NETERROR(MFIND, ("%s Could not add binding %s/%s\n",
				fn, cachecpbEntry->cpbEntry.cpname,
				cachecpbEntry->cpbEntry.crname));
		}
	}

	if (entry->routeEntry.crflags & CRF_DYNAMICLRU)
	{
		CacheInsert(lruRoutesCache, entry);
	
		// Check to see if the number has exceeded max count
	}

	return rc;
}

int
DeleteRoute(RouteEntry *entry)
{
	char fn[] = "DeleteRoute():";
	CacheRouteEntry *routeEntry;
	CacheRouteEntry *entryList, *tmp;
	CallPlanBindEntry cpbEntry;
	int rc;

	if (entry == NULL)
	{
		return -1;
	}

	routeEntry = CacheDelete(cpCache, entry);

	if (routeEntry == NULL)
	{
		return -1;
	}

	if (routeEntry->routeEntry.crflags & CRF_TEMPLATE)
	{
		// We are done
		goto _return;
	}

	// We dont have access to the head of the list.
	// Thus we must delete the whole list,
	// see if we have to insert it again into the tree,
	// as the head is 'hidden' in the tree

	if (strlen(routeEntry->routeEntry.dest) ||
		strlen(routeEntry->routeEntry.prefix) ||
		routeEntry->routeEntry.destlen ||
		routeEntry->routeEntry.crflags & CRF_DNISDEFAULT)
	{
		if (routeEntry->routeEntry.crflags & CRF_TRANSIT)
		{
			entryList = 
				CacheDelete(cptransitCache, routeEntry->routeEntry.dest);
		}
		else if (routeEntry->routeEntry.crflags & CRF_CALLORIGIN)
		{
			entryList = 
				CacheDelete(cporigDNISCache, routeEntry->routeEntry.dest);
		}
		else
		{
			entryList = 
				CacheDelete(cpdestCache, routeEntry->routeEntry.dest);
		}
	
		if (entryList != NULL)
		{
			if (!(ListIsSingle(entryList)))
			{
				// The new head can be the next of info
				entryList = routeEntry->next;
	
				// Delete this element from the list
				ListDelete(routeEntry);
	
				// insert the list back
				if (routeEntry->routeEntry.crflags & CRF_TRANSIT)
				{
					rc = CacheProbeInsert(cptransitCache, entryList, (void**)&tmp);
				}
				else if (routeEntry->routeEntry.crflags & CRF_CALLORIGIN)
				{
					rc = CacheProbeInsert(cporigDNISCache, entryList, (void**)&tmp);
				}
				else
				{
					rc = CacheProbeInsert(cpdestCache, entryList, (void**)&tmp);
				}
	
				if (rc < 0)
				{
					NETERROR(MSHM, ("%s CacheProbeInsert Error\n", fn));
				}
			}
		}
		else
		{
			NETERROR(MSHM, ("%s entryList is NULL for %s\n",
				fn, routeEntry->routeEntry.dest));
		}
	}

	if (!(routeEntry->routeEntry.crflags & CRF_TRANSIT))
	{
		if (strlen(routeEntry->routeEntry.src) ||
			strlen(routeEntry->routeEntry.srcprefix))
		{
			if (routeEntry->routeEntry.crflags & CRF_CALLORIGIN)
			{
				entryList = CacheDelete(cporigANICache, routeEntry->routeEntry.src);
			}
			else
			{
				entryList = CacheDelete(cpsrcCache, routeEntry->routeEntry.src);
			}

			if (entryList != NULL)
			{
				if (!(ListgIsSingle(entryList, sizeof(ListEntry))))
				{
					// Make sure that the element which we are
					// using to mark the rest of the list does have a valid tail
					// and not pointing to itself
					if ( (CacheRouteEntry *) Listg(routeEntry, sizeof(ListEntry))->next == routeEntry)
					{
						NETERROR(MSHM, 
							("%s Element being deleted does not appear to be part of list\n", fn));
					}
					else
					{
		
						// The new head can be the next of info
						entryList = (CacheRouteEntry*) Listg(routeEntry, sizeof(ListEntry))->next;
		
						// Delete this element from the list
						ListgDelete(routeEntry, sizeof(ListEntry));
					}
		
					// insert the list back
					if (routeEntry->routeEntry.crflags & CRF_CALLORIGIN)
					{
						rc = CacheProbeInsert(cporigANICache, entryList, (void**)&tmp);
					}
					else
					{
						rc = CacheProbeInsert(cpsrcCache, entryList, (void**)&tmp);
					}
		
					if (rc < 0)
					{
						NETERROR(MSHM, ("%s CacheProbeInsert Error\n", fn));
					}
				}
			}
			else
			{
				NETDEBUG(MSHM, NETLOG_DEBUG4,
					("%s entryList is NULL for %s\n",
					fn, routeEntry->routeEntry.src));
			}
		}
	}

	if (routeEntry->routeEntry.crflags & CRF_DYNAMICLRU)
	{
		CacheDelete(lruRoutesCache, routeEntry);
	}

	if (routeEntry->routeEntry.cpname[0] != '\0')
	{
		memset(&cpbEntry, 0, sizeof(CallPlanBindEntry));
		nx_strlcpy(cpbEntry.cpname, routeEntry->routeEntry.cpname, CALLPLAN_ATTR_LEN);
		nx_strlcpy(cpbEntry.crname, routeEntry->routeEntry.crname, CALLPLAN_ATTR_LEN);

		DeleteCPB(&cpbEntry);
	}

_return:

	// Free up the route entry
	CFree(cpCache)(routeEntry);

	return(0);
}

int
AddCPB(CacheCPBEntry *entry)
{
	int rc = 1;
	CacheCPBEntry *entryList = NULL;

	if (entry == NULL)
	{
		return -1;
	}

	if (CacheInsert(cpbCache, entry) < 0)
	{
		return -1;
	}

	// Now we have to insert the route into the list entries
	ListInitElem(entry);

	CacheProbeInsert(cpbcrCache, entry, (void**)&entryList);

	if (entryList)
	{
		// entry was not inserted
		ListInsert(entryList, entry);	
	}

	// Now we have to insert the route into the list entries
	entryList = NULL;

	ListgInitElem(entry, sizeof(ListEntry));

	CacheProbeInsert(cpbcpCache, entry, (void**)&entryList);

	if (entryList)
	{
		// entry was not inserted
		ListgInsert(entryList, entry, sizeof(ListEntry));
	}

	return rc;
}

int
DeleteCPB(CallPlanBindEntry *entry)
{
	char fn[] = "DeleteCPB():";
	CacheCPBEntry *cpbEntry;
	CacheCPBEntry *entryList = NULL, *tmp;
	int rc;

	if (entry == NULL)
	{
		return -1;
	}

	cpbEntry = CacheDelete(cpbCache, entry);

	if (cpbEntry == NULL)
	{
		return -1;
	}

#if 0
	if (ListIsSingle(cpbEntry))
	{
		CacheDelete(cpbcrCache, entry->crname);
	}
	else
	{
		ListDelete(cpbEntry);
	}

	if (ListgIsSingle(cpbEntry, sizeof(ListEntry)))
	{
		CacheDelete(cpbcpCache, cpbEntry->cpbEntry.crname);
	}
	else
	{
		ListgDelete(cpbEntry, sizeof(ListEntry));
	}
#endif
	// We dont have access to the head of the list.
	// Thus we must delete the whole list,
	// see if we have to insert it again into the tree,
	// as the head is 'hidden' in the tree

	entryList = CacheDelete(cpbcrCache, cpbEntry->cpbEntry.crname);
	if (entryList != NULL)
	{
		if (!(ListIsSingle(entryList)))
		{
			// The new head can be the next of info
			entryList = (CacheCPBEntry*) List(cpbEntry)->next;

			// Delete this element from the list
			ListDelete(cpbEntry);

			// insert the list back
			rc = CacheProbeInsert(cpbcrCache, entryList, (void**)&tmp);

			if (rc < 0)
			{
				NETERROR(MSHM, ("%s CacheProbeInsert Error\n", fn));
			}
		}
	}
	else
	{
		NETERROR(MSHM, ("%s entryList is NULL for %s\n",
			fn, cpbEntry->cpbEntry.crname));
	}

	// We dont have access to the head of the list.
	// Thus we must delete the whole list,
	// see if we have to insert it again into the tree,
	// as the head is 'hidden' in the tree

	entryList = CacheDelete(cpbcpCache, cpbEntry->cpbEntry.cpname);
	if (entryList != NULL)
	{
		if (!(ListgIsSingle(entryList, sizeof(ListEntry))))
		{
			// The new head can be the next of info
			entryList = (CacheCPBEntry*) Listg(cpbEntry, sizeof(ListEntry))->next;

			// Delete this element from the list
			ListgDelete(cpbEntry, sizeof(ListEntry));

			// insert the list back
			rc = CacheProbeInsert(cpbcpCache, entryList, (void**)&tmp);

			if (rc < 0)
			{
				NETERROR(MSHM, ("%s CacheProbeInsert Error\n", fn));
			}
		}
	}
	else
	{
		NETERROR(MSHM, ("%s entryList is NULL for %s\n",
			fn, cpbEntry->cpbEntry.cpname));
	}

_return:

	// Free up the route entry
	CFree(cpCache)(cpbEntry);

	return(0);
}

int
AddTrigger(CacheTriggerEntry *entry)
{
	char fn[] = "AddTrigger():";
	int rc = 1;

	if (entry == NULL)
	{
		return -1;
	}

	if (!entry->trigger.srcvendor || !entry->trigger.dstvendor)
	{
		return -1;
	}

	if (CacheInsert(triggerCache, entry) < 0)
	{
		return -1;
	}

	return rc;
}

int
DeleteTrigger(TriggerEntry *tgEntry)
{
	char fn[] = "DeleteTrigger():";
	CacheCPBEntry *cacheTgEntry;
	CacheCPBEntry *entryList = NULL, *tmp;
	int rc;

	if (tgEntry == NULL)
	{
		return -1;
	}

	cacheTgEntry = CacheDelete(triggerCache, tgEntry);

	if (cacheTgEntry == NULL)
	{
		return -1;
	}

_return:

	// Free up the route entry
	CFree(triggerCache)(cacheTgEntry);

	return(0);
}

int
AddRealm(CacheRealmEntry *entry)
{
	char fn[] = "AddRealm():";
	int		rc=-1;

	if (entry == NULL)
	{
		return rc;
	}

	if (CacheInsert(realmCache, entry) < 0)
	{
		return rc;
	}

    if (entry->realm.rsa)
    {
        rc = CacheInsert(rsaCache, entry);
    }

	if (entry->realm.rsa && entry->realm.mask &&
		(entry->realm.addrType == ENDPOINT_PUBLIC_ADDRESS))
	{
		// Do not check return value here, as we can have multiple
		// realms on the same subnet. If the user has that, then we will
		// just have to pick the first one we find.
		// Will provisioning help here ?
        CacheInsert(rsapubnetsCache, entry);
	}

	if (entry->realm.flags & REALMF_DEFAULT)
	{
		if (*defaultRealm)
		{
			rc = -1;
		}
		else
		{
			*defaultRealm = entry;
		}
	}

	return rc;
}

int
AddVnet(CacheVnetEntry *entry)
{
	char fn[] = "AddVnet():";
	int		rc=-1;

	if (entry == NULL)
	{
		return rc;
	}

	if (CacheInsert(vnetCache, entry) < 0)
	{
		return rc;
	}

	return rc;
}

int
AddIgrp(CacheIgrpInfo *entry)
{
	char fn[] = "AddIgrp():";
	int		rc=-1;

	if (entry == NULL)
	{
		return rc;
	}

	if (CacheInsert(igrpCache, entry) < 0)
	{
		return rc;
	}

	return 0;
}

/* Delete RealmCache Entry corresponding to 
 * gdbm entry rmEntry
 */
int
DeleteRealm(unsigned long *rmId)
{
	char fn[] = "DeleteRealm():";
	CacheRealmEntry *cachermEntry, *tmpRealmEntry=NULL;
	RealmEntry  *rmEntry=NULL;

	cachermEntry = CacheDelete(realmCache, rmId);

	if (cachermEntry == NULL)
		return( -1 );

	rmEntry = &cachermEntry->realm;

    if (rmEntry->rsa)
    {
		tmpRealmEntry = CacheGet(rsaCache, &rmEntry->rsa);

		if ( tmpRealmEntry == cachermEntry )
		{
			CacheDelete(rsaCache, &rmEntry->rsa);
		}
    }

	if (rmEntry->rsa && rmEntry->mask &&
		(rmEntry->addrType == ENDPOINT_PUBLIC_ADDRESS))
	{
		tmpRealmEntry = CacheGet(rsapubnetsCache, &rmEntry->rsa);

		if ( tmpRealmEntry == cachermEntry )
		{
			CacheDelete(rsapubnetsCache, &rmEntry->rsa);
		}
	}

	if (cachermEntry == *defaultRealm)
	{
		*defaultRealm = NULL;
	}

	// Free up the realm entry found in realmCache
	CFree(realmCache)(cachermEntry);

	return(0);
}

int
DeleteIgrp(unsigned char *igrpname)
{
	char fn[] = "DeleteiIgrp():";
	CacheIgrpInfo  *igrpCachePtr;

	igrpCachePtr = CacheDelete(igrpCache, igrpname);

	if (igrpCachePtr == NULL)
	{
		return -1;
	}

	// Free up the realm entry found in realmCache
	CFree(igrpCache)(igrpCachePtr);
	return(0);
}

int
DeleteVnet(unsigned char *vnetname)
{
	char fn[] = "DeleteVnet():";
	CacheVnetEntry  *vnetCachePtr;

	vnetCachePtr = CacheDelete(vnetCache, vnetname);

	if (vnetCachePtr == NULL)
	{
		return -1;
	}

	// Free up the realm entry found in realmCache
	CFree(vnetCache)(vnetCachePtr);
	return(0);
}

int
AddGw(CacheTableInfo *entry)
{
	int rc = 1;
	CacheTableInfo *entryList;

	if (entry == NULL)
	{
		return -1;
	}

	if (IsGateway(&entry->data))
	{
		if (CacheInsert(gwCache, entry) < 0)
		{
			return -1;
		}
	}
	else
	{
		// This entry is not a gateway.
		return 0;
	}

	// Now we have to insert the route into the list entries
	ListInitElem(entry);

	CacheProbeInsert(gwcpCache, entry, (void**)&entryList);

	if (entryList)
	{
		// entry was not inserted
		ListInsert(entryList, entry);	
	}

	return rc;
}

// As the gw entry is actually part of several
// indexes here, we cant free it up here,
// or not return what we found
CacheTableInfo *
DeleteGw(InfoEntry *entry)
{
	char fn[] = "DeleteGw():";
	CacheTableInfo *info = NULL;
	CacheTableInfo *entryList, *tmp;
	int rc;

	if (entry == NULL)
	{
		return 0;
	}

	if (IsGateway(entry))
	{
		info = CacheDelete(gwCache, entry);
	}

	if (info == NULL)
	{
		return 0;
	}

	// We dont have access to the head of the list.
	// Thus we must delete the whole list,
	// see if we have to insert it again into the tree,
	// as the head is 'hidden' in the tree

	entryList = CacheDelete(gwcpCache, info->data.cpname);
	if (entryList == NULL)
	{
		NETERROR(MSHM, ("%s entryList is NULL for %s\n",
			fn, info->data.cpname));

		goto _return;
	}

	if (!(ListIsSingle(entryList)))
	{
		// The new head can be the next of info
		entryList = info->next;

		// Delete this element from the list
		ListDelete(info);

		// insert the list back
		rc = CacheProbeInsert(gwcpCache, entryList, (void**)&tmp);

		if (rc < 0)
		{
			NETERROR(MSHM, ("%s CacheProbeInsert Error\n", fn));
		}
	}

_return:
	return info;
}

int
CPBTestTime(CallPlanBindEntry *cpbEntry)
{
	if ((cpbEntry->sTime.tm_hour != TM_ANY) ||
		(cpbEntry->sTime.tm_min != TM_ANY) ||
		(cpbEntry->sTime.tm_sec != TM_ANY) ||
		(cpbEntry->sTime.tm_year != TM_ANY) ||
		(cpbEntry->sTime.tm_yday != TM_ANY) ||
		(cpbEntry->sTime.tm_wday != TM_ANY) ||
		(cpbEntry->sTime.tm_mon != TM_ANY) ||
		(cpbEntry->sTime.tm_mday != TM_ANY) ||
		(cpbEntry->fTime.tm_hour != TM_ANY) ||
		(cpbEntry->fTime.tm_min != TM_ANY) ||
		(cpbEntry->fTime.tm_sec != TM_ANY) ||
		(cpbEntry->fTime.tm_year != TM_ANY) ||
		(cpbEntry->fTime.tm_yday != TM_ANY) ||
		(cpbEntry->fTime.tm_wday != TM_ANY) ||
		(cpbEntry->fTime.tm_mon != TM_ANY) ||
		(cpbEntry->fTime.tm_mday != TM_ANY) )
	{
		cpbEntry->crflags |= CRF_TIME;
	}
	else
	{
		cpbEntry->crflags &= ~CRF_TIME;
	}
	return(0);
}

//
//   getSignaledInterfaces()
//
// Purpose:
//
// 		return the physical interface names associated with
// 		signaling.
//
// Returns 0 on success, -1 on failure
//
//		numEnt      		is the size of the callers array of
//							interface names
//		returned_entries	is an int pointer to return to the caller
//							which tells the caller how many physical 
//							interfaces we found
//		ifs					pointer to an array of strings in
//							which we store the physical interface
//							names
//
// Notes:
//
// pass an array of type char [numEnt][IFI_NAME]
//
// first argument numEnt specifies the number of entries for
// which space has been allocated
//
int
getSignaledInterfaces(	int		numEnt,
						int *	returned_entries,
						char 	(*ifs)[IFI_NAME] )
{
	char			fn[] = "getSignaledInterfaces():";
	DB				db;
	DB_tDb			dbstruct;
	RealmEntry *	rInfo = 0;
	RealmKey 	*	key, *okey;
	int 			i, numifs = 0, err = 0;
	char			ifname[IFI_NAME];
	
	dbstruct.read_write = GDBM_READER;

	*returned_entries = 0;

	if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
	{
	     ERROR(MDB, ("%s Unable to open database %s\n", fn, REALM_DB_FILE));
	     return( 0 );
	}
	
    for (	key = (RealmKey *)DbGetFirstRealmKey(db);
			key != 0;
			key = (RealmKey *)DbGetNextRealmKey(	db,
					(char *)key,
					sizeof(RealmKey)),
					free(okey),
					free(rInfo),
					rInfo = NULL )
    {
		if (numifs == numEnt)
		{
			// can't fit the result in the allocated space

			NETERROR(	MDB,
						("%s More than %d entries present\n",
						 fn,
						 numEnt));
			err = 1;
			goto _error;
		}

		// get the physical interface name

	  	rInfo = DbFindRealmEntry(db, (char *)key, sizeof(RealmKey));

		okey = key;

		if (rInfo == NULL)
		{
			NETERROR(	MDB,
						("%s Realm Entry for a key does not exist\n",
						fn));
			continue;
		}

		// Is physical interface name specified ?

		if ( rInfo->ifName[0] == '\0' )
			continue;	// No, skip this record

		// Yes, copy name into local ifname[]

		strcpy( ifname, rInfo->ifName );

		// Check if it already exists in the
		// interface table table we are building 
		// for the caller

		for ( i = 0; i < numifs; i++ )
		{
			if (strcmp(ifname, ifs[i]) == 0)
				break;		// interface already exists in the table
		}

		// Did we get through all the ones added so far ?

		if (i == numifs)
		{
			// Yes, It doesn't exist in the interface table
			// being build so add it

			strcpy( ifs[numifs++], ifname );
		}
	}

	*returned_entries = numifs;

_error:
	DbClose(&dbstruct);

	return err;
}
