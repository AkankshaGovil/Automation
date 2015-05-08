#include <unistd.h>
#include <limits.h>
#include "ipc.h"
#include "cli.h"
#include "serverp.h"
#include "gw.h"
#include "license.h"
#include "strings.h"
#include "string.h"
#ifdef ALLOW_ISERVER_SIP
#include "sipaddr.h"
#endif
#include "handles.h"
#include "lsconfig.h"
#include <malloc.h>
#include "nxosd.h"
#include "log.h"
#include "cacheinit.h"
#include "ipstring.h"

static char s1[30];

typedef enum
{
    ST_REGID_UPORT = 0,
    ST_REGID_ANI,
    ST_REGID_DNIS,
    ST_CID,
    ST_REALM_IP
} eSourceType;

int
UpdateNetoidState(void *entry, void *andState, void *orState, void *ipp)
{
	CacheTableInfo *info = (CacheTableInfo *)entry;
	unsigned long state = (unsigned long)-1;
	unsigned long ipaddr = (unsigned long)-1;

	if (info == NULL) return 0;

	if (IsSGatekeeper(&info->data))
	{
		// For sgatekeepers, we are not propagating any state
		// now
		return 0;
	}

	if (andState)
	{
		state = *(long *)andState;
		info->data.stateFlags &= state;
	}

	if (orState)
	{
		state = *(long *)orState;
		info->data.stateFlags |= state;
	}

	if (ipp)
	{
		ipaddr = *(long *)ipp;
		if (info->data.ipaddress.l != ipaddr)
		{
			DeleteIedgeIpAddr(ipCache, &info->data);

			info->data.ipaddress.l = ipaddr;
			BIT_SET(info->data.sflags, ISSET_IPADDRESS);

			AddIedgeIpAddr(ipCache, info);
		}
	}

	return 0;
}

int
HandleNetoidAdd(Command *comm, int argc, char **argv)
{
   	char *serNo, *ports, *temps, *ptrptr, *tok;
   	unsigned long port = 0, portH = 0;
   	char *vpnName;
   	VpnEntry vpnEntry;
   	NetoidInfoEntry *netInfo, entry, *tmp;
	int shmId;
	void *addr;
	int rc = 0;
	long portsState = ~CL_REGISTERED;
   	if (argc < 2)
   	{
	 	HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
   	}

   	/* Registration No */
   	serNo = argv[0];
     
	ports = temps = strdup(argv[1]);
	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
   		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

   	argc -= 2;
   	argv += 2;

   	netInfo = &entry;

   	while (port <= portH)
   	{
     	InitNetoidInfoEntry(netInfo);

		// Get a random crid here
		netInfo->crId = lrand48();

   	 	strncpy(netInfo->regid, serNo, REG_ID_LEN);
   	 	netInfo->uport = port;

	 	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	 	{
			return -xleOpNoPerm;
	 	}

   	 	if (tmp = DbFindInfoEntry(GDBMF(comm->data, DB_eNetoids), 
				(char *)netInfo, sizeof(NetoidSNKey)))
	 	{
	 		CLIPRINTF((stdout, 
			"entry (%s, %lu) already exists\n", serNo, port));
			argc -=1;
	     		argv +=1;

			free(tmp);

	 		port ++;
	 		CloseDatabases((DefCommandData *)comm->data);
			return -xleExists;
	 	}

	 	CloseDatabases((DefCommandData *)comm->data);

   	 	BIT_SET(netInfo->sflags, ISSET_REGID);
   	 	BIT_SET(netInfo->sflags, ISSET_UPORT);
		netInfo->ecaps1 |= ECAPS1_NOCONNH245;

	 	netInfo->iTime = netInfo->mTime = netInfo->rTime = time(0);

#if 0
	 	if (license_allocate(1))
	 	{
		 	CLIPRINTF((stdout,"could not obtain license\n"));
		 	return -xleNoLicense;
	 	}
#endif

		/* Check to see if there are other ports of this iedge */
		InheritIedgeGlobals(netInfo);

	 	/* Update the iedge in cache */
	 	if (UpdateNetoidInCache(netInfo) < 0)
	 	{
			return -xleExists;
	 	}

		UpdateNetoidPorts(serNo, UpdateNetoidState, &portsState, NULL, NULL);

		UpdateNetoidDatabase(netInfo);

	 	port ++;
    }

    return xleOk;
}

int
HandleNetoidLkup(Command *comm, int argc, char **argv)
{
	CacheTableInfo *cacheInfo;
	CacheTableEntry *cacheHandle;
	char fn[] = "HandleNetoidLkup():";
	unsigned long pkey;
	RealmIP     realmip;
	RealmSubnet realmsub;
	char *sphone = 0, *url = 0, *tg = 0;
	int shmId, i;
	char *regid, *port;
	char *realmNamei, lkupip=0;
	PhoNode phonode = { 0 };
	int release_lock = 0;

	/* Lookup in the LUS's cache and VPNS's cache */

	if (argc < 1)
	{
		 /* Here we prompt the user for the rest of the 
		  * information
		  */
		 HandleCommandUsage(comm, argc, argv);
		 return -xleInsuffArgs;
	}

	tg = url = sphone = argv[0];

	if ((shmId  = CacheAttach()) == -1)
	{
		 CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
	}
	else if (argc == 1)
	{
		 InitCfgFromCfgParms(lsMem->cfgParms);

		 realmip.ipaddress = StringToIp(sphone);
		 realmip.realmId = 0;
		 lkupip = (realmip.ipaddress? 1 : 0);

		 CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		 release_lock = 1;
		
		 cacheInfo = CacheGet(phoneCache, sphone);

		 if (cacheInfo)
		 {
			  CLIPRINTF((stdout, "Phone Match:\n"));
			  PrintInfoEntry(stdout, &cacheInfo->data);
		 }

		 cacheInfo = CacheGet(vpnPhoneCache, sphone);

		 if (cacheInfo)
		 {
			  CLIPRINTF((stdout, "Vpn Phone Match:\n"));
			  PrintInfoEntry(stdout, &cacheInfo->data);
		 }

		 /* do a reg-id lookup */
		 memset(&phonode, 0, sizeof(PhoNode));
		 strncpy(phonode.regid, sphone, REG_ID_LEN);
		 cacheInfo = CacheGet(regidCache, &phonode);
		 if (cacheInfo)
		 {
				CLIPRINTF((stdout, "Reg Id Match:\n"));
				PrintInfoEntry(stdout, &cacheInfo->data);

					/* Print some information relevent only to the
					* regid cache entry */
					fprintf(stdout, "\tNo of ports configured = %d \n\t[",
							cacheInfo->data.ncfgports);
				
					for (i=0; i<MAX_IEDGE_PORTS; i++)
					{	
						if (BITA_TEST(cacheInfo->data.cfgports, i))
						{
							 fprintf(stdout, " %d", i);
						}
					}
					fprintf(stdout, " ]\n\n");
		 } 

		 /* do a url lookup */
		 cacheInfo = CacheGet(uriCache, url);

		 if (cacheInfo)
		 {
			  CLIPRINTF((stdout, "Url Match:\n"));
			  PrintInfoEntry(stdout, &cacheInfo->data);
		 }

		// h323id cache
		 cacheInfo = CacheGet(h323idCache, sphone);

		 if (cacheInfo)
		 {
			  CLIPRINTF((stdout, "H323 ID Match:\n"));
			  PrintInfoEntry(stdout, &cacheInfo->data);
		 }

		 /* do a tg lookup */
		 cacheInfo = CacheGet(tgCache, tg);

		 if (cacheInfo)
		 {
			  CLIPRINTF((stdout, "TG Match:\n"));
			  PrintInfoEntry(stdout, &cacheInfo->data);
		 }

	}
	else if (argc == 2)
	{
		 InitCfgFromCfgParms(lsMem->cfgParms);

		 CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		 release_lock = 1;

		 /* Do a reg-id uport lkup also, in the end */
		 strncpy(phonode.regid, argv[0], REG_ID_LEN);
		 phonode.uport = atoi(argv[1]);
		 cacheInfo = CacheGet(regCache, &phonode);
		 if (cacheInfo)
		 {
			CLIPRINTF((stdout, "Reg Id/Uport Match:\n"));
			PrintInfoEntry(stdout, &cacheInfo->data);
		 } 

		/* Look up this ip address in our cache */
		realmip.ipaddress = StringToIp(argv[0]);
		realmip.realmId = realmNameToRealmId(argv[1]);
		if (realmip.ipaddress && realmip.realmId)
		{
			lkupip = 1;
		}
	}

	if (lkupip)
	{
		  cacheInfo = CacheGet(ipCache, &realmip);
		  if (cacheInfo)
		  {
			CLIPRINTF((stdout, "IP/Realm Match:\n"));
			PrintInfoEntry(stdout, &cacheInfo->data);

			/* Print some information relevent only to the
			 * ip cache entry 
			 */
			fprintf(stdout, "\tNo of ports registered = %d \n\t[",
											cacheInfo->data.nports);
			
			for (i=0; i<MAX_IEDGE_PORTS; i++)
			{	
				if (BITA_TEST(cacheInfo->data.ports, i))
				{
					fprintf(stdout, " %d", i);
				}
			}
			fprintf(stdout, " ]\n\n");
		} 

		// Also lookup in subnets
		realmsub.subnetip = realmip.ipaddress;
		realmsub.realmId = realmip.realmId;
		cacheInfo = GetIedgeLongestMatch(&realmsub);
		if (cacheInfo)
		{
			CLIPRINTF((stdout, "Longest Match:\n"));
			PrintInfoEntry(stdout, &cacheInfo->data);
		}	
	}

	if(release_lock)
	{
		CacheReleaseLocks(regCache);
	}
	CacheDetach();
	return xleOk;
}

int
HandleNetoidFind(Command *comm, int argc, char **argv)
{
     char *serNo;
     unsigned long port;
     NetoidInfoEntry *netInfo;
     NetoidSNKey key;
     
     log(LOG_DEBUG, 0, "Entering Netoid Find with argc=%d\n", argc);

     if (argc < 2)
     {
	 /* Here we prompt the user for the rest of the 
	  * information
	  */
	 HandleCommandUsage(comm, argc, argv);
	 return -xleInsuffArgs;
     }

     /* Registration No */
     serNo = argv[0];
     log(LOG_DEBUG, 0, "Ser No is %s\n", serNo);
     
     port = atoi(argv[1]);

     strncpy(key.regid, serNo, REG_ID_LEN);
     key.uport = port;

     if (OpenDatabases((DefCommandData *)comm->data) < 0)
     {
	return -xleOpNoPerm;
     }

     netInfo = DbFindInfoEntry(GDBMF(comm->data, DB_eNetoids), 
				(char *)&key, sizeof(key));
     if (!netInfo)
     {
	 CLIPRINTF((stdout, "No entry (%s, %lu) found\n", serNo, port));
     }
     else
     {
	 PrintDbInfoEntry(stdout, netInfo);
	 CLIPRINTF((stdout, "\n\n"));
	 free(netInfo);
     }

     CloseDatabases((DefCommandData *)comm->data);

     return xleOk;
}

// Delete can only be done using actual regid for now
// ip address/port combo extn to be followed w/ later
int
HandleNetoidDelete(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleNetoidDelete():";
   	char *serNo, *ports, *temps,*ptrptr, *tok;
    unsigned long port = 0, portH = 0;
    NetoidInfoEntry *netInfo;
	ClientAttribs *clAttribs = 0;
    CacheTableInfo *info;
    NetoidSNKey key;
    int shmId;
	long portsState = ~CL_REGISTERED;
	char *netdbname, *attrdbname;

    log(LOG_DEBUG, 0, "Entering Netoid Delete with argc=%d\n", argc);
                      
    if (argc < 2)
    {
	 	HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
    }

	netdbname = DBNAME((DefCommandData *)comm->data, DB_eNetoids);
	attrdbname = DBNAME((DefCommandData *)comm->data, DB_eAttribs);

    /* Registration No */
    serNo = argv[0];
    log(LOG_DEBUG, 0, "Ser No is %s\n", serNo);
     
	ports = temps = strdup(argv[1]);
   	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

    strncpy(key.regid, serNo, REG_ID_LEN);

    while (port <= portH)
    {
    	key.uport = port;

     	netInfo = (NetoidInfoEntry *) DbExtractEntry(netdbname, 
				DB_eNetoids, (char *)&key, sizeof(NetoidSNKey));

		if (netInfo == NULL)
	 	{
	 		CLIPRINTF((stdout, 
			"entry (%s, %lu) does not exist\n", serNo, port));
			argc -=2;
	       	argv +=2;

	 		port ++;
			return -xleInvalArgs;
	 	}

		UpdateNetoidPorts(serNo, UpdateNetoidState, &portsState, NULL, NULL);

     	clAttribs = (ClientAttribs *) DbExtractEntry(attrdbname, 
				DB_eAttribs, (char *)&key, sizeof(NetoidSNKey));

     	if (DbRemoveEntry(netdbname, DB_eNetoids,
				(char *)&key, sizeof(key)) < 0)
     	{
		 	log(LOG_ERR, errno, "database delete error\n");
     	}
     	else
     	{
	 		CLIPRINTF((stdout, "Entry Deleted Successfully\n"));
     	}
     
     	if (DbRemoveEntry(attrdbname, DB_eAttribs,
				(char *)&key, sizeof(key)) < 0)
     	{
		 	log(LOG_ERR, errno, "database delete error\n");
     	}

     	/* We must delete this guy from the cache as well
      	* This should be done after we have deleted from the db,
      	* since there might be a chance that someone will lookup around
      	* this time, and maybe add the guy back to the cache
      	*/

     	DeleteNetoidFromCache(netInfo);

     	free(netInfo);
		if (clAttribs)
		{
			free(clAttribs);
		}

		port ++;
     }

     return xleOk;
}

int
HandleNetoidList(Command *comm, int argc, char **argv)
{
     long serNo;
     NetoidInfoEntry *netInfo;
	 ClientAttribs *clAttribs;
     NetoidSNKey *key, *okey;
     int n = 0;
	char ttypname[2*_POSIX_PATH_MAX];

	if (ttyname_r(1, ttypname, 2*_POSIX_PATH_MAX) != ENOTTY)
	{
		CLIPRINTF((stdout, "Please redirect output to a file\n"));
		return 0;
	}

     log(LOG_DEBUG, 0, "Entering Netoid List with argc=%d\n", argc);

     if (argc != 0)
     {
	 /* Here we prompt the user for the rest of the 
	  * information
	  */
	 HandleCommandUsage(comm, argc, argv);
	 return -xleInsuffArgs;
     }

     if (OpenDatabases((DefCommandData *)comm->data) < 0)
     {
	return -xleOpNoPerm;
     }

     for (key = (NetoidSNKey *)DbGetFirstInfoKey(GDBMF(comm->data, 
						DB_eNetoids)); key != 0; 
	 key = (NetoidSNKey *)DbGetNextInfoKey(GDBMF(comm->data, 
						DB_eNetoids),
						(char *)key, 
						sizeof(NetoidSNKey)),
	      					free(okey))
     {
	 netInfo = DbFindInfoEntry(GDBMF(comm->data, DB_eNetoids), 
					(char *)key, sizeof(NetoidSNKey));
	 clAttribs = DbFindAttrEntry(GDBMF(comm->data, DB_eAttribs), 
					(char *)key, sizeof(NetoidSNKey));
	 PrintInfoEntry(stdout, netInfo);
	 PrintDbAttrEntry(stdout, clAttribs);
	 free(netInfo);
	 if (clAttribs)
	 {
		free(clAttribs);
	 }
	 CLIPRINTF((stdout, "\n\n"));
	 okey = key;
	 n ++;
     }

     CloseDatabases((DefCommandData *)comm->data);

	CLIPRINTF((stdout, "%d Endpoints\n\n", n));

     return xleOk;
}

int
HandleNetoidGkCache(Command *comm, int argc, char **argv)
{
   	char fn[] = "HandleNetoidGkCache():";
   	long serNo;
   	CacheGkInfo *info, *next;
   	int i;
   	int shmId;
   	int n = 0, m = 0, nactive = 0, nstatic = 0;
	cache_t wCache = 0;

   	if (argc > 1)
   	{
  		/* Here we prompt the user for the rest of the 
   		* information
   		*/
  		HandleCommandUsage(comm, argc, argv);
  		return -xleInsuffArgs;
   	}

   	if ((shmId  = CacheAttach())== -1)
   	{
 		CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
   	}
   	else
   	{
		InitCfgFromCfgParms(lsMem->cfgParms);

		wCache = gkCache;

		CacheGetLocks(wCache, LOCK_WRITE, LOCK_BLOCK);
		
		info = CacheGetFirst(wCache);

		while (info)
		{
			n ++;
			fprintf(stdout, "\tGatekeeper %d\n", n);
			PrintGkInfoEntry(stdout, info);
			fprintf(stdout, "\n");

			info = CacheGetNext(wCache, info->regid);
	   }
	
		/* Release the iterator */
		CacheFreeIterator(wCache);
	 
		CacheReleaseLocks(wCache);
		CacheDetach();
	}
	 
  	CLIPRINTF((stdout, 
		"\nGIS %d Endpoints/Ports\n\n", n));

   	return xleOk;
}

int
HandleNetoidCache(Command *comm, int argc, char **argv)
{
   	char fn[] = "HandleNetoidCache():";
   	NetoidInfoEntry *netInfo;
   	CacheTableInfo *info, *next;
	cache_t wCache = 0;
	RealmIP realmip;
   	int i, shmId, n = 0, m = 0, nactive = 0, nstatic = 0;
   	long serNo;

   	log(LOG_DEBUG, 0, "Entering Netoid Cache with argc=%d\n", argc);

   	if (argc > 1)
   	{
  		/* Here we prompt the user for the rest of the 
   		* information
   		*/
  		HandleCommandUsage(comm, argc, argv);
  		return -xleInsuffArgs;
   	}

   	if ((shmId  = CacheAttach())== -1)
   	{
 		CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
   	}
   	else
   	{
		InitCfgFromCfgParms(lsMem->cfgParms);

 		CLIPRINTF((stdout, "GIS Cache...\n"));

		wCache = regCache;

		if (argc > 0)
		{
			if (!strcmp(argv[0], "phones"))
			{
				 wCache = phoneCache;
			}
			else if (!strcmp(argv[0], "vpnphones"))
			{
				 wCache = vpnPhoneCache;
			}
			else if (!strcmp(argv[0], "ip"))
			{
				 wCache = ipCache;
			}
			else if (!strcmp(argv[0], "gw"))
			{
				 wCache = gwCache;
			}
			else if (!strcmp(argv[0], "subnet"))
			{
				 wCache = subnetCache;
			}
			else if (!strcmp(argv[0], "uri"))
			{
				 wCache = uriCache;
			}
			else if (!strcmp(argv[0], "reg"))
			{
				 wCache = regidCache;
			}
			else if (!strcmp(argv[0], "email"))
			{
				 wCache = emailCache;			        
			}
			else if (!strcmp(argv[0], "h323id"))
			{
				 wCache = h323idCache;			        
			}
			else if (!strcmp(argv[0], "crid"))
			{
				 wCache = cridCache;			        
			}
			else if (!strcmp(argv[0], "tg"))
			{
				 wCache = tgCache;			        
			}
			else if (!strcmp(argv[0], "gk"))
			{
				CacheDetach();

				return HandleNetoidGkCache(comm, argc, argv);
			}
		}
     
		CacheGetLocks(wCache, LOCK_WRITE, LOCK_BLOCK);
		
		info = CacheGetFirst(wCache);

		while (info)
		{
			n ++;
			fprintf(stdout, "\tIedge %d\n", n);
			PrintInfoEntry(stdout, &info->data);
			if (info->data.stateFlags & CL_ACTIVE)
			{
				nactive ++;
			}

			if (info->data.stateFlags & CL_STATIC)
			{
				nstatic ++;
			}

			if (wCache == ipCache)
			{
				/* Print some information relevent only to the
				 * ip cache entry */
				fprintf(stdout, "\tNo of ports registered = %d \n\t[",
						info->data.nports);
				
				for (i=0; i<MAX_IEDGE_PORTS; i++)
				{	
					 if (BITA_TEST(info->data.ports, i))
					 {
						  fprintf(stdout, " %d", i);
					 }
				}
				fprintf(stdout, " ]\n\n");
			}
			else if (wCache == regidCache)
			{
					/* Print some information relevent only to the
					* regid cache entry */
					fprintf(stdout, "\tNo of ports configured = %d \n\t[",
							info->data.ncfgports);
				
					for (i=0; i<MAX_IEDGE_PORTS; i++)
					{	
						if (BITA_TEST(info->data.cfgports, i))
						{
							 fprintf(stdout, " %d", i);
						}
					}
					fprintf(stdout, " ]\n\n");
			}

			if ((wCache == regCache) || (wCache == gwCache))
			{
				info = CacheGetNext(wCache, &info->data);
			}
			else if (wCache == phoneCache)
			{
				info = CacheGetNext(wCache, info->data.phone);
			}
			else if (wCache == vpnPhoneCache)
			{
				info = CacheGetNext(wCache, info->data.vpnPhone);
			}
			else if (wCache == ipCache)
			{
				realmip.ipaddress = info->data.ipaddress.l;
				realmip.realmId = info->data.realmId;
				info = CacheGetNext(wCache, &realmip);
			}
			else if (wCache == subnetCache)
			{
				info = CacheGetNext(wCache, &info->data.subnetip);
			}
			else if (wCache == uriCache)
			{
				info = CacheGetNext(wCache, info->data.uri);
			}
			else if (wCache == regidCache)
			{
				info = CacheGetNext(wCache, info->data.regid);
			}
			else if (wCache == emailCache)
			{
				info = CacheGetNext(wCache, info->data.email);
			}
            else if (wCache == h323idCache)
			{
				info = CacheGetNext(wCache, info->data.h323id);
			}
            else if (wCache == cridCache)
			{
				info = CacheGetNext(wCache, &info->data.crId);
			}
            else if (wCache == tgCache)
			{
				info = CacheGetNext(wCache, &info->data.tg);
			}
	   }
	
		/* Release the iterator */
		CacheFreeIterator(wCache);
	 
		CacheReleaseLocks(wCache);
		CacheDetach();
	}
	 
  	CLIPRINTF((stdout, 
		"GIS %d Endpoints/Ports[%d active %d static]\n\n", 
		n, nactive, nstatic));

   	return xleOk;
}

int
HandleNetoidEdit(Command *comm, int argc, char **argv)
{
   	char *serNo, *ports, *temps,*ptrptr, *tok;
    unsigned long port = 0, portH = 0;
    char *vpnId;
    NetoidInfoEntry *netInfo = 0;
	CacheTableInfo cacheInfoEntry;
	ClientAttribs *clAttribs = 0;
    NetoidSNKey key = { 0 };
	VpnEntry vpnKey = { 0 }, *vpnEntry = NULL;
	char storeb[1024], oldrealmname[REALM_NAME_LEN];
	long portsState = ~CL_REGISTERED, portsStateOr = 0;
	int rc = xleOk, obtainedRegid = 0;
	char	ch;

    if (argc <= 2)
    {
		/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
		
	 	NetoidEditHelp(comm, argc, argv);
	 	//HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
    }

    /* Registration No */
    serNo = argv[0];
     
	ports = temps = strdup(argv[1]);
   	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

    argc -= 2;
    argv += 2;

    strncpy(key.regid, serNo, REG_ID_LEN);

    while (port <= portH)
    {
	 	key.uport = port;

		CacheAttach();

		if (!obtainedRegid)
		{
			CliGetRegid(serNo, key.regid);
			obtainedRegid = 1;
		}

		// Look up the entry in the cache
		if (CacheFind(regCache, &key, &cacheInfoEntry, 
				sizeof(cacheInfoEntry)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}
		else if (ExtractIedge ((char *)&key, &cacheInfoEntry.data, 
										sizeof(cacheInfoEntry.data)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}

		CacheDetach();

	 	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	 	{
			return -xleOpNoPerm;
	 	}

	 	clAttribs = DbFindAttrEntry(GDBMF(comm->data, DB_eAttribs),
			(char *)&key, sizeof(key));

	 	if (netInfo)
	 	{
			strcpy(vpnKey.vpnName, netInfo->vpnName);
	 	}

	 	if (strlen(vpnKey.vpnName))
	 	{
	 		vpnEntry = DbFindVpnEntry(GDBMF(comm->data, DB_eVpns),
						(char *)&vpnKey, sizeof(VpnKey));
	 	}

	 	CloseDatabases((DefCommandData *)comm->data);

	 	if (netInfo == 0)
	 	{
			CLIPRINTF((stdout, "iedge Not found in Cache/Database\n"));
			return -xleNoEntry;
	 	}

	 	if (clAttribs == 0)
	 	{
			/* Attributes not found */
			clAttribs = (ClientAttribs *)malloc(sizeof(ClientAttribs));
			memset(clAttribs, 0, sizeof(ClientAttribs));
	 	}

		nx_strlcpy(oldrealmname, netInfo->realmName, REALM_NAME_LEN);

	 	if (argc > 0)
	 	{
			GetNetoidAttrPairs(comm->name, &argc, &argv, 
				netInfo, clAttribs);
			goto _storedb;	
	 	}

	 	if (cliLibFlags == 0) 
		{
			/* we should not be getting here... */
			NETERROR(MCLI, ("Invalid arguments passed from jserver\n"));
			rc = -xleInvalArgs;
			goto _error;
	 	}

		CLIPRINTF((stdout, "Map ISDN Code Code [%s]: ", 
			(netInfo->ecaps1&ECAPS1_MAPISDNCC)?"enable":"disable"));
		GetInput(stdin, storeb, 10);
		if (strlen(storeb) > 0) 
		{
			if(!strcmp(storeb, "enable"))
			{
				netInfo->ecaps1 |= ECAPS1_MAPISDNCC;
			}
			else
			{
				netInfo->ecaps1 &= ~ECAPS1_MAPISDNCC;
			}
		}

	_storedb:
		// Flags must be adjusted, if ip address was deleted
		if (!BIT_TEST(netInfo->sflags, ISSET_IPADDRESS))
		{
			netInfo->stateFlags &= ~CL_ACTIVE;
			portsState &= ~CL_ACTIVE;
		}

		netInfo->stateFlags &= ~CL_REGISTERED;
		netInfo->mTime = time(0);
	
		if (strcmp(oldrealmname, netInfo->realmName))
		{
			netInfo->realmId = realmNameToRealmId(netInfo->realmName);
		}

		if (UpdateNetoidInCache(netInfo) < 0)
		{
			rc = -xleExists;
			goto _error;
		}
	
		if (IsSGatekeeper(netInfo))
		{
			// Sgatekeepers no longer allowed to be marked
			// static
			netInfo->stateFlags &= ~CL_STATIC;
		}

		/* reflect the static bit */
		if (netInfo->stateFlags & CL_STATIC)
		{
			portsStateOr |= CL_STATIC;
		}
		else
		{
			portsState &= ~CL_STATIC;
		}

		if (netInfo->stateFlags & CL_STATIC)
		{
			UpdateNetoidPorts(serNo, UpdateNetoidState, &portsState, 
				&portsStateOr, 
				netInfo->ipaddress.l?&netInfo->ipaddress.l:NULL);
		}
		else
		{
			UpdateNetoidPorts(serNo, UpdateNetoidState, &portsState, 
				&portsStateOr, NULL);
		}


		UpdateNetoidDatabase(netInfo);
	
		UpdateNetoidAttrDatabase((NetoidSNKey*)netInfo, clAttribs);
	
	_continue:	 
	_error:
		port ++;
	
	    netInfo = NULL;
		free(clAttribs); clAttribs = NULL;
	
		if (rc < 0)
		{
			return rc;
		}
     }
  
     if (argc > 0)
     {
		/* All arguments not exhausted... */
		CLIPRINTF((stdout, "%s: Error: Insufficient Arguments \n", comm->name));
		return -xleInvalArgs;
     }

	return xleOk;
}     

int
HandleNetoidPhones(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleNetoidPhones():";
   	char *serNo, *ports, *temps,*ptrptr, *tok;
    unsigned long port = 0, portH = 0;
    NetoidSNKey key;
    NetoidInfoEntry *netInfo = NULL;
	CacheTableInfo cacheInfoEntry;
	VpnEntry vpnKey = { 0 }, *vpnEntry = NULL;
	int rc = xleOk, obtainedRegid = 0;
	long portsState = ~CL_REGISTERED;
     
    if (argc < 2)
    {
	 	/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
    }

     /* Registration No */
     serNo = argv[0];
     
	ports = temps = strdup(argv[1]);
   	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

     argc -= 2;
     argv += 2;

	 strncpy(key.regid, serNo, REG_ID_LEN);

     while (port <= portH)
     {
	 	key.uport = port;
	 
		CacheAttach();

		if (!obtainedRegid)
		{
			CliGetRegid(serNo, key.regid);
			obtainedRegid = 1;
		}

		// Look up the entry in the cache
		if (CacheFind(regCache, &key, &cacheInfoEntry, sizeof(cacheInfoEntry)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}
		else if (ExtractIedge ((char *)&key, &cacheInfoEntry.data, 
										sizeof(cacheInfoEntry.data)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}

		CacheDetach();

	 if (OpenDatabases((DefCommandData *)comm->data) < 0)
	 {
		return -xleOpNoPerm;
	 }

	 if (netInfo)
	 {
		strcpy(vpnKey.vpnName, netInfo->vpnName);
	 }

	 if (strlen(vpnKey.vpnName))
	 {
	 	vpnEntry = DbFindVpnEntry(GDBMF(comm->data, DB_eVpns),
						(char *)&vpnKey, sizeof(VpnKey));
	 }

	 CloseDatabases((DefCommandData *)comm->data);

     if (!netInfo)
     {
		/* vpn entry exists only if netInfo existed!! */
	 	return -xleNoEntry;
     }

	 if (argc -- > 0)
	 {
	    strcpy(netInfo->vpnPhone, argv[0]);

		if (strlen(netInfo->vpnPhone))
		{
	      		BIT_SET(netInfo->sflags, ISSET_VPNPHONE);
				netInfo->vpnExtLen = strlen(netInfo->vpnPhone);
		}
		else
		{
	      		BIT_RESET(netInfo->sflags, ISSET_VPNPHONE);
				netInfo->vpnExtLen = 0;
		}

		AssignIedgePhone(netInfo, vpnEntry);

	    argv ++;
	 }

	 netInfo->stateFlags &= ~CL_REGISTERED;
	 netInfo->mTime = time(0);

	 /* If this netoid is in the cache, delete it */
	 if (UpdateNetoidInCache(netInfo) < 0)
	 {
		rc = -xleExists;
		goto _error;
	 }

	 UpdateNetoidPorts(serNo, UpdateNetoidState, &portsState, NULL, NULL);

	 UpdateNetoidDatabase(netInfo);

	 netInfo = NULL;
	 if (vpnEntry)
	 {
		free(vpnEntry);
		vpnEntry = NULL;
	 }

	 port ++;
     }

     return xleOk;

_error:
	if (netInfo)
	{
		free(netInfo);
	}
		
	if (vpnEntry)
	{
		free(vpnEntry);
	}

	return rc;
}     

int
HandleNetoidEmail(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleNetoidEmail():";
   	char *serNo, *ports, *temps,*ptrptr, *tok;
	unsigned long port = 0, portH = 0;
	NetoidSNKey key;
	NetoidInfoEntry *netInfo = NULL;
	CacheTableInfo cacheInfoEntry;
	int rc = xleOk, obtainedRegid = 0;
	if (argc < 2)
    {
	 	/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
    }


    /* Registration No */
    serNo = argv[0];
     
	ports = temps = strdup(argv[1]);
   	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

    argc -= 2;
    argv += 2;

	strncpy(key.regid, serNo, REG_ID_LEN);

    while (port <= portH)
    {
	 	key.uport = port;
	 
		CacheAttach();

		if (!obtainedRegid)
		{
			CliGetRegid(serNo, key.regid);
			obtainedRegid = 1;
		}

		// Look up the entry in the cache
		if (CacheFind(regCache, &key, &cacheInfoEntry, sizeof(cacheInfoEntry)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}
		else if (ExtractIedge ((char *)&key, &cacheInfoEntry.data, 
										sizeof(cacheInfoEntry.data)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}

		CacheDetach();

        if (!netInfo)
        {
     	    CLIPRINTF((stdout, "%s: Unable to find the iedge %s\n", 
					fn, serNo));
	      	return -xleNoEntry;
        }

	 	if (argc -- > 0)
	 	{
	      	/* This port has a specified phone num */
	      
	      	strncpy(netInfo->email, argv[0], EMAIL_LEN);
			if (strlen(netInfo->email))
			{
	      		BIT_SET(netInfo->sflags, ISSET_EMAIL);
			}
			else
			{
	      		BIT_RESET(netInfo->sflags, ISSET_EMAIL);
			}

	      	argv ++;
	 	}

	 	netInfo->stateFlags &= ~CL_REGISTERED;
	 	netInfo->mTime = time(0);

	 	/* If this netoid is in the cache, delete it */
	 	if (UpdateNetoidInCache(netInfo) < 0)
		{
			rc = -xleExists;
			goto _error;
		}

	 	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	 	{
			return -xleOpNoPerm;
	 	}

	 	/* Read the Vpn information here */
	 	if (DbStoreInfoEntry(GDBMF(comm->data, DB_eNetoids), netInfo, 
			      (char *)netInfo, sizeof(NetoidSNKey)) < 0)
	 	{
	      NETERROR(MCLI, ("database store error\n"));
	 	}

	 	CloseDatabases((DefCommandData *)comm->data);

_error:
	 	netInfo = NULL;
	 	port ++;
	 	if (rc < 0)
	 	{
			return rc;
	 	}
     }

     return rc;
}

int
HandleNetoidZone(Command *comm, int argc, char **argv)
{
     char fn[] = "HandleNetoidZone():";
   	char *serNo, *ports, *temps,*ptrptr, *tok;
     unsigned long port = 0, portH = 0;
     NetoidSNKey key;
     NetoidInfoEntry *netInfo = NULL;
	CacheTableInfo cacheInfoEntry;
	int rc = xleOk, obtainedRegid = 0;
     log(LOG_DEBUG, 0, "Entering Netoid Zone with argc=%d\n", argc);

     if (argc < 2)
     {
	 /* Here we prompt the user for the rest of the 
	  * information
	  */
	 HandleCommandUsage(comm, argc, argv);
	 return -xleInsuffArgs;
     }


     /* Registration No */
     serNo = argv[0];
     log(LOG_DEBUG, 0, "Ser No is %s\n", serNo);
     
	ports = temps = strdup(argv[1]);
   	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

     log(LOG_DEBUG, 0, "Ports is (%d..%d)\n", port, portH);

     argc -= 2;
     argv += 2;

	 strncpy(key.regid, serNo, REG_ID_LEN);

     while (port <= portH)
     {
	 	key.uport = port;
	 
		CacheAttach();

		if (!obtainedRegid)
		{
			CliGetRegid(serNo, key.regid);
			obtainedRegid = 1;
		}

		// Look up the entry in the cache
		if (CacheFind(regCache, &key, &cacheInfoEntry, sizeof(cacheInfoEntry)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}
		else if (ExtractIedge ((char *)&key, &cacheInfoEntry.data, 
										sizeof(cacheInfoEntry.data)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}

		CacheDetach();

          if (!netInfo)
          {
     	      log(LOG_DEBUG, 0, "%s: Unable to find the iedge %s\n", 
				fn, serNo);
	      return -xleNoEntry;
          }

	 if (argc -- > 0)
	 {
	      /* This port has a specified zone num */
	      log(LOG_DEBUG, 0, "Zone is %s\n", argv[0]);
	      
	      strncpy(netInfo->zone, argv[0], ZONE_LEN);
	      argv ++;
	 }

	 netInfo->mTime = time(0);

	 if (OpenDatabases((DefCommandData *)comm->data) < 0)
	 {
		return -xleOpNoPerm;
	 }

	 /* Read the Vpn information here */
	 if (DbStoreInfoEntry(GDBMF(comm->data, DB_eNetoids), netInfo, 
			      (char *)netInfo, sizeof(NetoidSNKey)) < 0)
	 {
	      log(LOG_ERR, errno, "database store error \n");
	 }

	 CloseDatabases((DefCommandData *)comm->data);

	 /* If this netoid is in the cache, delete it */
	 UpdateNetoidInCache(netInfo);

	 port ++;
     }

     return xleOk;
}

int
HandleNetoidVpns(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleNetoidVpns():";
   	char *serNo, *ports, *temps,*ptrptr, *tok;
    unsigned long port = 0, portH = 0;
    char *vpnName;
    NetoidSNKey key;
    NetoidInfoEntry *netInfo = NULL;
	CacheTableInfo cacheInfoEntry;
	VpnEntry vpnKey = { 0 }, *vpnEntry = NULL;
	int shmId;
	void *addr;
	long portsState = ~CL_REGISTERED;
	int rc = xleOk, obtainedRegid = 0;
    if (argc < 2)
    {
	 	/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
    }

    /* Registration No */
    serNo = argv[0];

	ports = temps = strdup(argv[1]);
   	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

    argc -= 2;
    argv += 2;

 	strncpy(key.regid, serNo, REG_ID_LEN);

    while ((port <= portH) && (argc > 0))
    {
	 	key.uport = port;

		CacheAttach();

		if (!obtainedRegid)
		{
			CliGetRegid(serNo, key.regid);
			obtainedRegid = 1;
		}

		// Look up the entry in the cache
		if (CacheFind(regCache, &key, &cacheInfoEntry, sizeof(cacheInfoEntry)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}
		else if (ExtractIedge ((char *)&key, &cacheInfoEntry.data, 
										sizeof(cacheInfoEntry.data)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}

		CacheDetach();

	 	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	 	{
			return -xleOpNoPerm;
	 	}

	 	/* This port has a specified vpn */
	 	vpnName = argv[0];

	 	if (strlen(vpnName))
	 	{
			strcpy(vpnKey.vpnName, vpnName);
	 		vpnEntry = DbFindVpnEntry(GDBMF(comm->data, DB_eVpns),
						(char *)&vpnKey, sizeof(VpnKey));
	 	}

	 	CloseDatabases((DefCommandData *)comm->data);

        if (!netInfo)
        {
     	   	CLIPRINTF((stdout, "%s: Unable to find the iedge %s\n", 
				fn, serNo));
	      	rc = -xleNoEntry;
			goto _error;
        }

		strcpy(netInfo->vpnName, vpnName);

		AssignIedgePhone(netInfo, vpnEntry);

		netInfo->stateFlags &= ~CL_REGISTERED;
		netInfo->mTime = time(0);

        if (UpdateNetoidInCache(netInfo) < 0)
		{
			rc = -xleExists;
			goto _error;
		}

	 	UpdateNetoidPorts(serNo, UpdateNetoidState, &portsState, NULL, NULL);

		UpdateNetoidDatabase(netInfo);

		netInfo = NULL;

	 	if (vpnEntry)
	 	{
			free(vpnEntry);
			vpnEntry = NULL;
	 	}

		port ++;
	}

     return xleOk;

_error:
	if (netInfo)
	{
		free(netInfo);
	}
		
	if (vpnEntry)
	{
		free(vpnEntry);
	}
	return(rc);
}

// Command takes regid/uport, ip4: or phone# as source
int
ComputeNetoidRoute(Command *comm, int argc, char **argv, int hunt)
{
	char fn[] = "ComputeNetoidRoute():";
	char *phone = 0, *url = 0, *tg = 0, *cic = 0, *dtg = NULL;
	char *regid = NULL, *sphone = NULL;
	int uport = 0, shmId;
	PhoNode phonode = { 0 }, fphonode = { 0 };
	PhoNode phonodeTmp = { 0 }, fphonodeTmp = { 0 };
	CacheTableInfo srcCacheInfoEntry, *srcCacheInfo;
	char *rPhone, guessPhone[PHONE_NUM_LEN] = { 0 };
	InfoEntry *entry = 0x0, fentry, aentry;
	int rc = -xleInvalArgs, nhunt = 0, xhunt = 0;
	int checkZone = 0, checkVpnGroup = 0;
	PhoNode *phonodep = &phonode;
	PhoNode *fphonodep = &fphonode;
	ResolveHandle *rhandle,*rhandle2;
	RouteNode routeNode = { 0 };
	ListEntry *rejectList = NULL;	
	char crname[CALLPLAN_ATTR_LEN];
	NetoidSNKey key = {0};
	CacheTableInfo *info = NULL;
	eSourceType sourceType = ST_REGID_UPORT;
	RealmIP realmip;

	/* Lookup in the LUS's cache and VPNS's cache */

	if (argc < 2)
	{
		rc = -xleInsuffArgs;
		goto _error;
	}

	if (strncasecmp(argv[1], "ani:", 4) == 0)
	{
		sourceType = ST_REGID_ANI;
	}
	else if (strncasecmp(argv[1], "dnis:", 5) == 0)
        {
                sourceType = ST_REGID_DNIS;
        }
	else if (strncasecmp(argv[0], "cid:", 4) == 0)
	{
		sourceType = ST_CID;
	}
	else if (strncasecmp(argv[0], "realm:", 6) == 0)
	{
		sourceType = ST_REALM_IP;
	}

	switch (argc)
	{
	case 2:
		if (sourceType == ST_REGID_ANI)
		{
			regid = argv[0];
			sphone = argv[1] + 4;
		}
		else if (sourceType == ST_REGID_DNIS)
		{
			regid = argv[0];
			phone = argv[1] + 5;
		}
		else if (sourceType == ST_CID)
		{
			sphone = argv[0] + 4;
			phone = argv[1];
		}
		else
		{
			goto _error;
		}
		break;
	case 3:
		if (sourceType == ST_REGID_UPORT)
		{
			regid = argv[0];
			uport = atoi(argv[1]);
			phone = argv[2];
		}
		else if (sourceType == ST_REALM_IP)
		{
			realmip.realmId = realmNameToRealmId(argv[0] + 6);
			realmip.ipaddress = ntohl(inet_addr(argv[1]));
			phone = argv[2];
		}
		else if (sourceType == ST_CID)
		{
			sphone = argv[0] + 4;
			phone = argv[1];
			dtg = argv[2];
		}
		else
		{
			goto _error;
		}
		break;
	case 4:
		if (sourceType == ST_REGID_UPORT)
		{
			regid = argv[0];
			uport = atoi(argv[1]);
		}
		else if (sourceType == ST_REALM_IP)
		{
			realmip.realmId = realmNameToRealmId(argv[0] + 6);
			realmip.ipaddress = ntohl(inet_addr(argv[1]));
		}
                else
                {
                        goto _error;
                }

		phone = argv[2];
		dtg = argv[3];
                break;
#if 0
/* comment it out since it is not supported */
	case 5:
		phonodep->ipaddress.l = ntohl(inet_addr(argv[0]));
		BIT_SET(phonodep->sflags, ISSET_IPADDRESS);
		tg = argv[1];
		cic = argv[2];
		sphone = argv[3];
		phone = argv[4];
		break;
#endif
	default:
		goto _error;

	}

	checkVpnGroup = 1;
	checkZone = 1;

#ifdef TURN_SYSLOG_LOGGING_ON
	/* Set the debug modules to appropriate levels */
	NetLogInit();
	NetLogOpen(NULL, 0, NETLOG_TERMINAL);
	NETLOG_SETLEVEL(MFIND, NETLOG_DEBUG4);
#endif

	if ((shmId  = CacheAttach()) == -1)
	{
		 CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
		 rc = -xleNoAccess;
	}
	else
	{

		InitCfgFromCfgParms(lsMem->cfgParms);

		/* Call the gis call routing api */	
		srcCacheInfo = &srcCacheInfoEntry;

		switch (sourceType)
		{
		case ST_REGID_UPORT:
			strncpy(phonodep->regid, regid, REG_ID_LEN);
			phonodep->uport = uport;

			BIT_SET(phonodep->sflags, ISSET_REGID);
			BIT_SET(phonodep->sflags, ISSET_UPORT);
			break;
		case ST_REGID_ANI:
		case ST_REGID_DNIS:
			strncpy(key.regid, regid, REG_ID_LEN);
 			CacheGetLocks(regidCache, LOCK_READ, LOCK_BLOCK);
			info = CacheGet(regidCache, &key);
			CacheReleaseLocks(regidCache);		
			if (info == NULL)
			{
				CLIPRINTF((stdout, "regid not found\n"));
				goto _return;
			}

			phonodep->ipaddress.l = info->data.ipaddress.l;
			phonodep->realmId = info->data.realmId;
			BIT_SET(phonodep->sflags, ISSET_IPADDRESS);

			if (sourceType == ST_REGID_ANI)
			{
				strncpy(phonodep->phone, sphone, PHONE_NUM_LEN);
				BIT_SET(phonodep->sflags, ISSET_PHONE);
			}
			break;
		case ST_REALM_IP:
			CacheGetLocks(ipCache, LOCK_READ, LOCK_BLOCK);
			info = CacheGet(ipCache, &realmip);
			CacheReleaseLocks(ipCache);
			if (info == NULL)
			{
				CLIPRINTF((stdout, "source not found\n"));
				goto _return;
			}
			phonodep->ipaddress.l = info->data.ipaddress.l;
			phonodep->realmId = info->data.realmId;
			BIT_SET(phonodep->sflags, ISSET_IPADDRESS);
			break;
		case ST_CID:
			if (CacheFind(phoneCache, sphone,
				srcCacheInfo, sizeof(CacheTableInfo)) < 0)
                        {
				CLIPRINTF((stdout, "caller not found\n"));
				rc = -xleInvalArgs;
				goto _return;
                        }
			phonodep->ipaddress.l = srcCacheInfo->data.ipaddress.l;
			phonodep->realmId = srcCacheInfo->data.realmId;
			BIT_SET(phonodep->sflags, ISSET_IPADDRESS);

			strncpy(phonodep->phone, sphone, PHONE_NUM_LEN);
			BIT_SET(phonodep->sflags, ISSET_PHONE);
		}

		if (phone)
		{
			strncpy(fphonodep->phone, phone, PHONE_NUM_LEN);
			BIT_SET(fphonodep->sflags, ISSET_PHONE);
		}

		/* Initialize the rhandle */
		rhandle 			= GisAllocRHandle();
		rhandle->phonodep 	= phonodep;
		rhandle->rfphonodep = fphonodep;
		rhandle->dtg		= dtg;
		rhandle->crname		= crname;

		memset(&rhandle->sVpn, 0, sizeof(VpnEntry));
		memset(rhandle->sZone, 0, ZONE_LEN);
	
		/* Get the originator's entry, straight from database. It may
		* be faster to look him up from the database (not true when there
		* are lesser netoids
		*/

		if (FillSourceCacheForCallerId(phonodep, "", tg, cic, phone, srcCacheInfo) < 0)
		{
			if (!allowSrcAll)
			{
				CLIPRINTF((stdout,
					"%s No Src Entry Found\n", fn));
				rc = -xleNoEntry;
				goto _return;
			}
		}
		else
		{
			CLIPRINTF((stdout, "\tFound SRC %s/%lu\n",
				srcCacheInfo->data.regid, srcCacheInfo->data.uport));
			
			SetPhonodeFromDb(phonodep, &srcCacheInfo->data);

			// copy back whatever the user specified!
			if (sourceType == ST_CID)
			{
				strncpy(phonodep->phone, sphone, PHONE_NUM_LEN);
				BIT_SET(phonodep->sflags, ISSET_PHONE);
			}
			else
			{
				nx_strlcpy(phonodep->regid, srcCacheInfo->data.regid, REG_ID_LEN);
				phonodep->uport = srcCacheInfo->data.uport;

				BIT_SET(phonodep->sflags, ISSET_REGID);
				BIT_SET(phonodep->sflags, ISSET_UPORT);
			}
		}

		if (hunt)
		{
			xhunt = srcCacheInfo->data.maxHunts;
			if (xhunt == 0)
			{
				xhunt = maxHunts;
			}
			else if (xhunt > SYSTEM_MAX_HUNTS)
			{
				xhunt = SYSTEM_MAX_HUNTS;
			}
		}

		if(!allowHairPin)
		{
			// Hairpin is NOT allowed, add src into reject list.
			// If src is not found, add the src ip addr
			// Add it to the resolve handle reject list
			// and remember to free it up later
			GwAddPhoNodeToRejectList(phonodep, NULL,
				&rhandle->destRejectList, malloc);
		}

		rc = xleOk;

		entry = &srcCacheInfo->data;
		FindIedgeVpn(entry->vpnName, &rhandle->sVpn);
		nx_strlcpy(rhandle->sZone, entry->zone, ZONE_LEN);
		rhandle->scpname = entry->cpname;

		memcpy(&routeNode.xphonode, rhandle->rfphonodep, 
			sizeof(PhoNode));

		if (!hunt)
		{
			CLIPRINTF((stdout, "\t----------------------\n"));
		}

		/* Set policy */
		rhandle->checkZone = 1;
		rhandle->checkVpnGroup = 1;
		rhandle->reservePort = 0;
		rhandle->phoneChange = 1;

		rhandle->primary = 0;
		if (srcCacheInfo->data.srcEgressTG[0] != '\0')
		{
			rhandle->dtg = srcCacheInfo->data.srcEgressTG;
		}
		
		memcpy(&phonodeTmp, rhandle->phonodep, sizeof(PhoNode));
		memcpy(&fphonodeTmp, rhandle->rfphonodep, sizeof(PhoNode));

	_resolve:
		memcpy(rhandle->phonodep, &phonodeTmp, sizeof(PhoNode));
		memcpy(rhandle->rfphonodep, &fphonodeTmp, sizeof(PhoNode));
		memset(crname, 0, CALLPLAN_ATTR_LEN);

		ResolvePhoneLocally(rhandle, hunt?0:_CliRouteLogFn);

		switch (rhandle->result)
		{
		case CACHE_FOUND:
		case CACHE_INPROG:
			memcpy(&routeNode.yphonode, rhandle->rfphonodep, sizeof(PhoNode));
			routeNode.branch = rhandle->primary;
			routeNode.crname = rhandle->crname;

			if (hunt)
			{
				CLIPRINTF((stdout, "\t------- result %d --------\n", nhunt+1));
			}
			else
			{
				CLIPRINTF((stdout, "\t------- final result --------\n"));
			}

			if (regid)
				CLIPRINTF((stdout, "\tSRC %s/%lu DEST %s\n",
					phonodep->regid, phonodep->uport, fphonodep->phone));
			else
				CLIPRINTF((stdout, "\tSRC %s DEST %s\n",
					phonodep->phone, fphonodep->phone));

			_CliRouteLogFn(&routeNode);

			// make sure we don't get into some infinite loop
			if (hunt && ++nhunt < xhunt)
			{
				GwAddPhoNodeToRejectList(rhandle->rfphonodep, rhandle->crname,
					&rhandle->destRejectList, malloc);
				goto _resolve;
			}

			break;
		case CACHE_NOTFOUND:
			routeNode.branch = rhandle->primary;
			routeNode.rejectReason = nextoneNoEntry;
			if (!nhunt)
			{
				CLIPRINTF((stdout, "\t-------no result--------\n"));
				if (regid)
					CLIPRINTF((stdout, "\tSRC %s/%lu DEST %s\n",
						phonodep->regid, phonodep->uport, fphonodep->phone));
				else
					CLIPRINTF((stdout, "\tSRC %s DEST %s\n",
						phonodep->phone, fphonodep->phone));

				_CliRouteLogFn(&routeNode);
			}
			goto _finish;

			break;
		default:
			break;
		}

	_finish:
		CLIPRINTF((stdout, "\t-------end--------\n"));

		if (rhandle->destRejectList)
		{
			GwFreeRejectList(rhandle->destRejectList, free);
		}

		GisFreeRHandle(rhandle);
	_return:
		CacheDetach();
	}

	return rc;

_error:
	/* Here we prompt the user for the rest of the
	 * information
	 */
	HandleCommandUsage(comm, argc, argv);

	return rc;
}

// Command takes regid/uport, ip4: or phone# as source
int
HandleNetoidRoute(Command *comm, int argc, char **argv)
{
	return ComputeNetoidRoute(comm, argc, argv, 0);
}

int
HandleNetoidHunt(Command *comm, int argc, char **argv)
{
	return ComputeNetoidRoute(comm, argc, argv, 1);
}

int
HandleNetoidReg(Command *comm, int argc, char **argv)
{
   	char *serNo, *ports, *temps,*ptrptr, *tok;
    unsigned long port = 0, portH = 0;
    NetoidInfoEntry *netInfo = 0;
	CacheTableInfo cacheInfoEntry;
    NetoidSNKey key;
    char storeb[1024];
	long portsState = ~0, portsStateOr = 0;
	int rc = xleOk, regn, obtainedRegid = 0;
    if (argc < 2)
    {
		/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
    }

    /* Registration No */
    serNo = argv[0];
    log(LOG_DEBUG, 0, "Ser No is %s\n", serNo);
     
	ports = temps = strdup(argv[1]);
   	tok = strtok_r(temps, "-", &ptrptr);
   	if (tok)
   	{
		port = atoi(tok);
		tok = strtok_r(NULL, "-", &ptrptr);
		if(tok){
	 	portH = atoi(tok);
	}
   		else{
	 	portH = port;
   	}
	}
	free(ports);

    argc -= 2;
    argv += 2;

    strncpy(key.regid, serNo, REG_ID_LEN);

    while (port <= portH)
    {
		regn = 0;
	 	key.uport = port;

		CacheAttach();

		if (!obtainedRegid)
		{
			CliGetRegid(serNo, key.regid);
			obtainedRegid = 1;
		}

		// Look up the entry in the cache
		if (CacheFind(regCache, &key, &cacheInfoEntry, sizeof(cacheInfoEntry)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}
		else if (ExtractIedge ((char *)&key, &cacheInfoEntry.data, 
										sizeof(cacheInfoEntry.data)) > 0)
		{
			netInfo = &cacheInfoEntry.data;
		}

		CacheDetach();

		// At this point of the entry is not found, its an error
		// if its found, then we are going to be agnostic to where
		// it came from
	 	if (netInfo == NULL)
	 	{
			CLIPRINTF((stdout, "iedge Not found in Cache/Database\n"));
			rc = -xleNoEntry;
			goto _error;
	 	}

	 	if (argc > 0)
	 	{
			GetNetoidAttrPairs(comm->name, &argc, &argv, 
				netInfo, NULL);
			goto _storedb;	
	 	}

	 	if (cliLibFlags == 0) 
		{
			/* we should not be getting here... */
			NETERROR(MCLI, ("Invalid arguments passed from jserver\n"));
			rc = -xleInvalArgs;
			goto _error;
	 	}

	 	CLIPRINTF((stdout, "ser # %s port %lu:\n", serNo, port));

		// Either the reg ip or the contact should
		// be sufficient to activate the registration

	 	CLIPRINTF((stdout, "Registration IP [%s?none]: ", 
			FormatIpAddress(netInfo->ipaddress.l, storeb)));
	 	GetInput(stdin, storeb, CLIENT_ATTR_LEN);
	 	if (strlen(storeb) > 0) 
	 	{
			if (!strcmp(storeb, "none"))
	 		{
				netInfo->stateFlags &= ~(CL_ACTIVE|CL_REGISTERED);
	 		}
	 		else /* User entered an ip address */
			{
				netInfo->ipaddress.l = inet_addr(storeb);
				netInfo->ipaddress.l = ntohl(netInfo->ipaddress.l);

				regn = 1;

				netInfo->stateFlags |= CL_ACTIVE;
				netInfo->stateFlags |= CL_REGISTERED;

				BIT_SET(netInfo->sflags, ISSET_IPADDRESS);
				netInfo->rasip = netInfo->ipaddress.l;
			}
			time(&netInfo->rTime);	
	 	}
	 
		CLIPRINTF((stdout, "SIP Enable? [%s]: ", 
			(BIT_TEST(netInfo->cap, CAP_SIP)?"true":"false")));
	
		GetInput(stdin, storeb, 25);
		if (strlen(storeb) > 0) 
		{
			if (!strcmp(storeb, "true"))
			{
				BIT_SET(netInfo->cap, CAP_SIP);
			}
			else
			{
				BIT_RESET(netInfo->cap, CAP_SIP);
			}
		}
	
		CLIPRINTF((stdout, "H323 Enable? [%s]: ", 
			(BIT_TEST(netInfo->cap, CAP_H323)?"true":"false")));
	
		GetInput(stdin, storeb, 25);
		if (strlen(storeb) > 0) 
		{
			if (!strcmp(storeb, "true"))
			{
				BIT_SET(netInfo->cap, CAP_H323);
			}
			else
			{
				BIT_RESET(netInfo->cap, CAP_H323);
			}
		}
	
		if (BIT_TEST(netInfo->cap, CAP_H323))
		{
		 	CLIPRINTF((stdout, "Q.931 port [%d]: ", netInfo->callsigport));
		 	GetInput(stdin, storeb, CLIENT_ATTR_LEN);
		 	if (strlen(storeb) > 0) 
		 	{
				netInfo->callsigport = atoi(storeb);
		 	}
		}
	
		if (BIT_TEST(netInfo->cap, CAP_SIP))
		{
		 	CLIPRINTF((stdout, "Contact [%s]: ", netInfo->contact));
		 	GetInput(stdin, storeb, SIPURL_LEN);
		 	if (strlen(storeb) > 0) 
		 	{
				  if (!strcmp(storeb, "none"))
				  {
						memset(netInfo->contact, 0, SIPURL_LEN);
						if (!regn)
						{
							netInfo->stateFlags &= ~(CL_ACTIVE|CL_REGISTERED);
						}
				  }
				  else
				  {
		 				strcpy(netInfo->contact, storeb);
						netInfo->stateFlags |= CL_ACTIVE;
						netInfo->stateFlags |= CL_REGISTERED;
						time(&netInfo->rTime);	
				  }
		 	}
		}
	
		if (netInfo->stateFlags&CL_DND)
		{
		 	CLIPRINTF((stdout, "DND [enable]: "));
		}
		else
		{
		 	CLIPRINTF((stdout, "DND [disable]: "));
		}
	
		GetInput(stdin, storeb, 25);
		if (strlen(storeb) > 0) 
		{
			  if (!strcmp(storeb, "enable"))
			  {
				netInfo->stateFlags |= CL_DND;
			  }
			  else
			  {
				netInfo->stateFlags &= ~CL_DND;
			  }
		}
	
		if (BIT_TEST(netInfo->cap, CAP_IGATEWAY))
		{
		 	CLIPRINTF((stdout, "Gateway [enable]: "));
		}
		else
		{
		 	CLIPRINTF((stdout, "Gateway [disable]: "));
		}
	
		GetInput(stdin, storeb, 25);
		if (strlen(storeb) > 0) 
		{
			  if (!strcmp(storeb, "enable"))
			  {
				BIT_SET(netInfo->cap, CAP_IGATEWAY);
			  }
			  else
			  {
				BIT_RESET(netInfo->cap, CAP_IGATEWAY);
			  }
		}
	
		CLIPRINTF((stdout, "Current Calls [%d]: ", IedgeCalls(netInfo)));
		GetInput(stdin, storeb, 10);
		if (strlen(storeb) > 0) 
		{
			IedgeCalls(netInfo) = atoi(storeb);
		}
	
	_storedb:
	
		if (UpdateNetoidInCache(netInfo) < 0)
		{
			rc = -xleExists;
			goto _error;
		}
	
		UpdateNetoidPorts(serNo, UpdateNetoidState, &portsState, 
			&portsStateOr, netInfo->ipaddress.l?&netInfo->ipaddress.l:NULL);

		UpdateNetoidDatabase(netInfo);
	
	_continue:	 
	_error:
		port ++;
	
	    netInfo = NULL;
	
		if (rc < 0)
		{
			return rc;
		}

    }
  
	if (argc > 0)
    {
		/* All arguments not exhausted... */
		CLIPRINTF((stdout, "%s: Error: Insufficient Arguments \n", comm->name));
		return -xleInvalArgs;
    }

	return xleOk;
}     

