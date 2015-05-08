#include "cli.h"
#include "serverp.h"
#include "avl.h"
#include "log.h"

static char s1[30];

int
HandleVpnAdd(Command *comm, int argc, char **argv)
{
   	char *vpnName, *vpnId, *vpnG = NULL;
   	unsigned long vpnExtLen;
	int rc = 0;
	VpnEntry vpnEntry;
	VpnGroupEntry vpnGroupEntry;
	int shmId;
	void *addr;
     
   	if (argc < 3)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

	vpnName = argv[0];
   	vpnId = argv[1];
     
   	vpnExtLen = atoi(argv[2]);

	memset(&vpnEntry, 0, sizeof(VpnEntry));
	memset(&vpnGroupEntry, 0, sizeof(VpnGroupEntry));

	strncpy(vpnEntry.vpnName, vpnName, VPNS_ATTR_LEN);
	strncpy(vpnEntry.vpnId, vpnId, VPN_LEN);
	vpnEntry.vpnExtLen = vpnExtLen;

	vpnEntry.mTime = time(0);

   	if (argc > 3)
   	{	
   		vpnG = argv[3];
		strncpy(vpnEntry.vpnGroup, vpnG, VPN_GROUP_LEN);
		strncpy(vpnGroupEntry.vpnGroup, vpnG, VPN_GROUP_LEN);
		vpnGroupEntry.mTime = time(0);
   	}

	shmId = CacheAttach();

	/* If the vpn already exists, stop */

   	if (StoreVpnInDb(comm, &vpnEntry, shmId) >= 0)
   	{
   		if (vpnG)
		{
			if (OpenDatabases((DefCommandData *)comm->data) < 0)
			{
				if (shmId > 0)
				{
					CacheDetach();
				}

				return -xleOpNoPerm;
			}
	
			StoreVpnG(GDBMF(comm->data, DB_eVpnG), &vpnGroupEntry);

   			CloseDatabases((DefCommandData *)comm->data);

			if (shmId > 0)
			{
				CacheHandleVpnG(&vpnGroupEntry, CLIOP_ADD);
			}
		}

		if (shmId > 0)
		{
			CacheHandleVpn(&vpnEntry, CLIOP_ADD);
		}
		
		rc = xleOk;
   	}
   	else
   	{
		rc = -xleExists;
   	}

	if (shmId > 0)
	{
		CacheDetach();
	}

   	return rc;
}     

int
HandleVpnDelete(Command *comm, int argc, char **argv)
{
     	char fn[] = "HandleVpnDelete():";
     	VpnKey vpnKey;
	int shmId;
	void *addr;

     	if (argc < 1)
     	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
     	}

     	memset(&vpnKey, 0, sizeof(VpnKey));
     	strcpy(vpnKey.vpnName, argv[0]);

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}
	
     	if (DbDeleteVpnEntry(GDBMF(comm->data, DB_eVpns), 
		&vpnKey, sizeof(VpnKey)) < 0)
     	{
	  	CLIPRINTF((stdout, "Entry Deletion Error\n"));
     	}
     	else
     	{
	  	CLIPRINTF((stdout, "Entry Deleted Successfully\n"));
     	}
     
	CloseDatabases((DefCommandData *)comm->data);

	if ((shmId = CacheAttach()) > 0)
	{
		CacheHandleVpn( (VpnEntry *)&vpnKey, CLIOP_DELETE);
		CacheDetach();
	}

     	return xleOk;
}

int
HandleVpnList(Command *comm, int argc, char **argv)
{
     	char *vpnName;
     	VpnEntry *vpnEntry,*lastVpnEntry;
	time_t tmptime;
     
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

     	for (vpnEntry = DbGetFirstVpnEntry(GDBMF(comm->data, DB_eVpns)); 
			vpnEntry != 0; 
	  	vpnEntry = DbGetNextVpnEntry(GDBMF(comm->data, DB_eVpns), 
			vpnName, sizeof(VpnKey)), 
	  	free(lastVpnEntry))
     	{
	  	tmptime = vpnEntry->mTime;
		PrintVpnEntry(stdout, vpnEntry);
	  	CLIPRINTF((stdout, "\tmod time %s\n", ctime(&tmptime)));

	  	vpnName = vpnEntry->vpnName;
	  	lastVpnEntry = vpnEntry;
     	}

	CloseDatabases((DefCommandData *)comm->data);
     	return xleOk;
}

int
HandleVpnCache(Command *comm, int argc, char **argv)
{
     	int shmId;
	VpnEntry *vpnEntry;
	CacheVpnEntry *cacheVpnEntry;
	time_t tmptime;
	
     	if (argc != 0)
     	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
     	}

     	/* Attach to shared memory */
     	shmId = CacheAttach();

     	if (shmId == -1)
     	{
	  	CLIPRINTF((stdout, "Unable to attach to VPNS cache\n"));
     	}
     	else
     	{
		CLIPRINTF((stdout, "VPN Cache...\n"));
	  
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		for (cacheVpnEntry = (CacheVpnEntry *)CacheGetFirst(vpnCache);
			cacheVpnEntry != NULL; 
			cacheVpnEntry = (CacheVpnEntry *)CacheGetNext(vpnCache, 
				&cacheVpnEntry->vpnEntry))
		{
			vpnEntry = &cacheVpnEntry->vpnEntry;

			PrintVpnEntry(stdout, vpnEntry);
	  		tmptime = vpnEntry->mTime;
	  		CLIPRINTF((stdout, "\tmod time %s\n", ctime(&tmptime)));
		}

		CacheFreeIterator(vpnCache);

		CacheReleaseLocks(vpnCache);

		CacheDetach();
     	}

     	return xleOk;
}

int
HandleVpnEdit(Command *comm, int argc, char **argv)
{
   	char *vpnName, *vpnG = NULL;
   	VpnEntry vpnEntry, *tmp;
	VpnGroupEntry vpnGroupEntry;
   	char oldVpnGroup[VPN_GROUP_LEN];
    char storeb[1024], oldprefix[PHONE_NUM_LEN] = { 0 },
	olddomain[SIPURL_LEN] = { 0 };
	int shmId;

   	if (argc < 1)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

   	vpnName = argv[0];
     
   	memset(&vpnEntry, 0, sizeof(VpnEntry));

   	strcpy(vpnEntry.vpnName, vpnName);

   	if (OpenDatabases((DefCommandData *)comm->data) < 0)
   	{
	  	return -xleOpNoPerm;
   	}

   	tmp = DbFindVpnEntry(GDBMF(comm->data, DB_eVpns), 
		(char *)&vpnEntry, sizeof(VpnKey));

	if (tmp == NULL)
	{
	 	CLIPRINTF((stdout, "no vpn entry %s\n", vpnName));

	  	CloseDatabases((DefCommandData *)comm->data);

	  	return -xleNoEntry;
   	}

    argc -= 1;
    argv += 1;

	strcpy(oldprefix, tmp->prefix);
	strcpy(olddomain, tmp->sipdomain);

	if (argc > 0)
	{
		GetAttrPairs(comm->name, &argc, &argv, tmp, CLI_GET_ATTR_VPN);
		goto _storedb;	
	}

	if (cliLibFlags == 0) {
		/* we should not be getting here... */
		printf("Invalid arguments passed from jserver\n");
		CloseDatabases((DefCommandData *)comm->data);
		return -xleInvalArgs;
	}

	CLIPRINTF((stdout, "Prefix [%s]: ", tmp->prefix));
	GetInput(stdin, storeb, PHONE_NUM_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->prefix, storeb);

	CLIPRINTF((stdout, "Sip Domain [%s]: ", tmp->sipdomain));
	GetInput(stdin, storeb, SIPURL_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->sipdomain, storeb);

	CLIPRINTF((stdout, "Call Plan [%s]: ", tmp->cpname));
	GetInput(stdin, storeb, CALLPLAN_ATTR_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->cpname, storeb);

	CLIPRINTF((stdout, "Location [%s]: ", tmp->vpnLoc));
	GetInput(stdin, storeb, VPNS_ATTR_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->vpnLoc, storeb);

	CLIPRINTF((stdout, "Contact [%s]: ", tmp->vpnContact));
	GetInput(stdin, storeb, VPNS_ATTR_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->vpnContact, storeb);

_storedb:
	strcpy(oldVpnGroup, tmp->vpnGroup);
	tmp->mTime = time(0);

	/* At this point, the name may be changed. We need
	 * to tell everybody in this Vpn group to re-register
	 * group has changed.
	 */
	if (DbStoreVpnEntry(GDBMF(comm->data, DB_eVpns), tmp, 
	  	tmp, sizeof(VpnKey)) < 0)
	{
	 	log(LOG_ERR, errno, "database store error \n");
	}

	CloseDatabases((DefCommandData *)comm->data);

	if ((shmId = CacheAttach()) > 0)
	{
		CacheHandleVpn(tmp, CLIOP_ADD);

	  	MemAgeIedgesInVpnGs(oldVpnGroup, tmp->vpnGroup);

		if (strcmp(oldprefix, tmp->prefix))
		{
			/* The prefix has been changed */
			HandleVpnPrefixChange(tmp);
		}

		if (strcmp(olddomain, tmp->sipdomain))
		{
			/* The prefix has been changed */
			HandleVpnSipDomainChange(tmp);
		}

		CacheDetach();
   	}
	else 
	{
		if (strcmp(oldprefix, tmp->prefix))
		{
			/* The prefix has been changed */
			HandleVpnPrefixChange(tmp);
		}

		if (strcmp(olddomain, tmp->sipdomain))
		{
			/* The prefix has been changed */
			HandleVpnSipDomainChange(tmp);
		}
	}

	free(tmp);

   	return xleOk;
}

int
HandleVpnGCache(Command *comm, int argc, char **argv)
{
     	int shmId;
	VpnGroupEntry *vpnGroupEntry;
	CacheVpnGEntry *cacheVpnGEntry;
	time_t tmptime;
	
     	if (argc != 0)
     	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
     	}

     	/* Attach to shared memory */
	shmId = CacheAttach();

     	if (shmId == -1)
     	{
	  	CLIPRINTF((stdout, "Unable to attach to VPNS cache\n"));
     	}
     	else
     	{
		CLIPRINTF((stdout, "VPNG Cache...\n"));
	  
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

		for (cacheVpnGEntry = (CacheVpnGEntry *)CacheGetFirst(vpnGCache);
			cacheVpnGEntry != NULL; 
			cacheVpnGEntry = (CacheVpnGEntry *)CacheGetNext(vpnGCache, 
				&cacheVpnGEntry->vpnGroupEntry))
		{
			vpnGroupEntry = &cacheVpnGEntry->vpnGroupEntry;
	  		CLIPRINTF((stdout, "Retrieved vpn entry \"%s\"\n", vpnGroupEntry->vpnGroup));

	  		tmptime = vpnGroupEntry->mTime;
	  		CLIPRINTF((stdout, "\tmod time %s\n", ctime(&tmptime)));
		}

		CacheFreeIterator(vpnGCache);

		CacheReleaseLocks(vpnGCache);

		/* detach from shared memory */
		CacheDetach();
     	}

     	return xleOk;
}

int
HandleVpnVpnG(Command *comm, int argc, char **argv)
{
   	char *vpnName, *vpnG = NULL;
   	VpnEntry vpnEntry, *tmp;
	VpnGroupEntry vpnGroupEntry;
   	char oldVpnGroup[VPN_GROUP_LEN];
	int shmId;
	void *addr;

   	if (argc < 2)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

   	vpnName = argv[0];
     
   	vpnG = argv[1];

   	memset(&vpnEntry, 0, sizeof(VpnEntry));

   	strcpy(vpnEntry.vpnName, vpnName);

   	if (OpenDatabases((DefCommandData *)comm->data) < 0)
   	{
	  	return -xleOpNoPerm;
   	}

   	tmp = DbFindVpnEntry(GDBMF(comm->data, DB_eVpns), 
		(char *)&vpnEntry, sizeof(VpnKey));

   	if (tmp)
   	{
	  	if (strcmp(vpnG, tmp->vpnGroup) == 0)
	  	{
		 	free(tmp);
		 	CloseDatabases((DefCommandData *)comm->data);
		 	return xleOk;
	  	}

	  	strcpy(oldVpnGroup, tmp->vpnGroup);
	  	strcpy(tmp->vpnGroup, vpnG);
	  	tmp->mTime = time(0);

	  	/* At this point, either the vpn is a new one, or its
	   	* group has changed.
	   	*/
	  	if (DbStoreVpnEntry(GDBMF(comm->data, DB_eVpns), tmp, 
		  	tmp, sizeof(VpnKey)) < 0)
	  	{
		 	log(LOG_ERR, errno, "database store error \n");
	  	}

		if (strlen(vpnG))
		{
	  		memset(&vpnGroupEntry, 0, sizeof(VpnGroupEntry));
	  		strncpy(vpnGroupEntry.vpnGroup, vpnG, VPN_GROUP_LEN);

	  		StoreVpnG(GDBMF(comm->data, DB_eVpnG), &vpnGroupEntry);
		}

	  	CloseDatabases((DefCommandData *)comm->data);

	  	if ((shmId = CacheAttach()) > 0)
	  	{
			CacheHandleVpn(tmp, CLIOP_ADD);

			if (strlen(vpnG))
			{
				CacheHandleVpnG(&vpnGroupEntry, CLIOP_ADD);
			}

	  		MemAgeIedgesInVpnGs(oldVpnGroup, tmp->vpnGroup);
			CacheDetach();
	  	}

	  	free(tmp);

	  	return xleOk;
   	}
   	else
   	{
	 	CLIPRINTF((stdout, "no vpn entry %s\n", vpnName));
	  	CloseDatabases((DefCommandData *)comm->data);

	  	return -xleNoEntry;
   	}

   	return xleOk;
}     

int
HandleVpnGAdd(Command *comm, int argc, char **argv)
{
     char *vpnG = NULL;
	int rc = xleOk;
	VpnGroupEntry vpnGroupEntry;
	int shmId;
	void *addr;
     
     log(LOG_DEBUG, 0, "Entering VpnG Add with argc=%d\n", argc);

     if (argc < 1)
     {
	  /* Here we prompt the user for the rest of the 
	   * information
	   */
	  HandleCommandUsage(comm, argc, argv);
	  return -xleInsuffArgs;
     }

     vpnG = argv[0];
     log(LOG_DEBUG, 0, "VpnG is %s\n", vpnG);
     
	memset(&vpnGroupEntry, 0, sizeof(VpnGroupEntry));
	strncpy(vpnGroupEntry.vpnGroup, vpnG, VPN_GROUP_LEN);
	vpnGroupEntry.mTime = time(0);

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}
	
     	rc = StoreVpnG(GDBMF(comm->data, DB_eVpnG), &vpnGroupEntry);
	
	CloseDatabases((DefCommandData *)comm->data);

	if (rc == 0)
	{
		rc = xleExists;
	}
	else if (rc < 0)
	{
		rc = xleIOError;
	}
	else
	{
		rc = xleOk;

	  	if ((shmId= CacheAttach()) > 0)
	  	{
			CacheHandleVpnG(&vpnGroupEntry, CLIOP_ADD);
			CacheDetach();
	  	}
	}

     	return xleOk;
}

int
HandleVpnGDelete(Command *comm, int argc, char **argv)
{
     	char fn[] = "HandleVpnGDelete():";
     	VpnGroupEntry vpnGroupEntry;
	int shmId;
	void *addr;

     	log(LOG_DEBUG, 0, "Entering Netoid Delete with argc=%d\n", argc);
                      
     	if (argc < 1)
     	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
     	}

     	memset(&vpnGroupEntry, 0, sizeof(VpnGroupEntry));
     	strcpy(vpnGroupEntry.vpnGroup, argv[0]);

     	log(LOG_DEBUG, 0, "VpnG is %s\n", argv[0]);
     
	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}
	
     	if (DbDeleteVpnGEntry(GDBMF(comm->data, DB_eVpnG), &vpnGroupEntry, sizeof(VpnGroupKey)) < 0)
     	{
	  	log(LOG_ERR, errno, "database delete error\n");

     	}
     	else
     	{
	  	CLIPRINTF((stdout, "Entry Deleted Successfully\n"));
     	}
     
	CloseDatabases((DefCommandData *)comm->data);

  	if ((shmId= CacheAttach()) > 0)
  	{
		CacheHandleVpnG(&vpnGroupEntry, CLIOP_DELETE);
		CacheDetach();
  	}

     	return xleOk;
}

int
HandleVpnGList(Command *comm, int argc, char **argv)
{
     char *vpnG;
     VpnGroupEntry *vpnGroupEntry,*lastVpnGroupEntry;
	time_t tmptime;
     
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
	
     for (vpnGroupEntry = DbGetFirstVpnGEntry(GDBMF(comm->data, DB_eVpnG)); 
			vpnGroupEntry != 0; 
	  vpnGroupEntry = DbGetNextVpnGEntry(GDBMF(comm->data, DB_eVpnG), 
			vpnG, sizeof(VpnGroupKey)), 
	  free(lastVpnGroupEntry))
     {
	  CLIPRINTF((stdout, "Retrieved vpn entry \"%s\"\n", vpnGroupEntry->vpnGroup));

	  tmptime = vpnGroupEntry->mTime;
	  CLIPRINTF((stdout, "\tmod time %s\n", ctime(&tmptime)));

	  vpnG = vpnGroupEntry->vpnGroup;
	  lastVpnGroupEntry = vpnGroupEntry;
     }

	CloseDatabases((DefCommandData *)comm->data);
     return xleOk;
}

int
HandleVpnGEdit(Command *comm, int argc, char **argv)
{
   	char *vpnG = NULL;
     	char storeb[VPNS_ATTR_LEN];
	VpnGroupEntry vpnGroupEntry, *tmp;
	int shmId;

   	if (argc < 1)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

   	vpnG = argv[0];
     
   	memset(&vpnGroupEntry, 0, sizeof(VpnGroupEntry));

   	strcpy(vpnGroupEntry.vpnGroup, vpnG);

   	if (OpenDatabases((DefCommandData *)comm->data) < 0)
   	{
	  	return -xleOpNoPerm;
   	}

   	tmp = DbFindVpnGEntry(GDBMF(comm->data, DB_eVpnG), 
		(char *)&vpnGroupEntry, sizeof(VpnGroupKey));

	if (tmp == NULL)
	{
	 	CLIPRINTF((stdout, "no vpn group entry %s\n", vpnG));

	  	CloseDatabases((DefCommandData *)comm->data);

	  	return -xleNoEntry;
   	}

     	argc -= 1;
     	argv += 1;

	if (argc > 0)
	{
		GetAttrPairs(comm->name, &argc, &argv, tmp, CLI_GET_ATTR_VPNG);
		goto _storedb;	
	}

	if (cliLibFlags == 0) {
		/* we should not be getting here... */
		printf("Invalid arguments passed from jserver\n");
		CloseDatabases((DefCommandData *)comm->data);
		return -xleInvalArgs;
	}

	CLIPRINTF((stdout, "Calling Plan [%s]: ", tmp->cpname));
	GetInput(stdin, storeb, CALLPLAN_ATTR_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->cpname, storeb);

_storedb:
	tmp->mTime = time(0);

	/* At this point, the name may be changed. We need
	 * to tell everybody in this Vpn group to re-register
	 * group has changed.
	 */
	if (DbStoreVpnEntry(GDBMF(comm->data, DB_eVpnG), tmp, 
	  	tmp, sizeof(VpnGroupKey)) < 0)
	{
	 	log(LOG_ERR, errno, "database store error \n");
	}

	CloseDatabases((DefCommandData *)comm->data);

	if ((shmId = CacheAttach()) > 0)
	{
		CacheHandleVpnG(tmp, CLIOP_ADD);

	  	MemAgeIedgesInVpnGs(NULL, vpnG);
		CacheDetach();
   	}

	free(tmp);

   	return xleOk;
}

