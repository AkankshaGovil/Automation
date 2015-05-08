#include <unistd.h>
#include <limits.h>
#include "cli.h"
#include "serverp.h"

#include "resutils.h"
#include "log.h"

/* Handle Calling Plan commands */

int
HandleCPAdd(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCPAdd():";
   	char *name = NULL, *crname = NULL;
	CallPlanKey callPlanKey = { 0 };
	CallPlanEntry *callPlanEntry;
	CallPlanBindKey cpbKey = { 0 };
	CallPlanBindEntry *cpbEntry;
	int modified = 0;
	int shmId;
	int rc = xleOk;
     
	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	name = argv[0];
     
	if (strlen(name) <= 0)
	{
		/* name has to be non-null */
		return -xleInsuffArgs;
	}

	argc -= 1;
	argv += 1;

	strncpy(callPlanKey.cpname, name, CALLPLAN_ATTR_LEN-1);

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}

	if (callPlanEntry = (CallPlanEntry *)DbFindEntry(GDBMF(comm->data, DB_eCallPlan), 
			(char *)&callPlanKey, sizeof(CallPlanKey)))
	{
		if (argc == 0)
		{
	 		CLIPRINTF((stdout, 
				"entry (%s) already exists\n", callPlanKey.cpname));

			rc = -xleExists;
			goto _return;
		}
  	}
	else
	{
		/* Add the entry */
		callPlanEntry = (CallPlanEntry *)malloc(sizeof(CallPlanEntry));

		memset(callPlanEntry, 0, sizeof(CallPlanEntry));
		strncpy(callPlanEntry->cpname, name, CALLPLAN_ATTR_LEN-1);
		callPlanEntry->mTime = time(0);

		/* Store the entry */
	  	if (DbStoreEntry(GDBMF(comm->data, DB_eCallPlan), 
				(char *)callPlanEntry, sizeof(CallPlanEntry), 
			       (char *)&callPlanKey, sizeof(CallPlanKey)) < 0)
	  	{
	       		CLIPRINTF((stdout,"database store error %d\n", errno));
			rc = -xleOpNoPerm;
			goto _return;
	  	}
		
#if 0
		/* This entry does not need to go in cache,
		 * However, we still need to send it to our redundant peer
		 */
		if ((shmId = CacheAttach()) > 0)
		{
			/* Do our stuff here */
			CacheHandleCP(callPlanEntry, 1);
			CacheDetach();
		}
#endif
		
		modified = 1;

		free(callPlanEntry);
	}

	/* Now we are sure that the callplan entry is there, check for
	 * any routes the user wants to add
	 */
	/* Now create the entry */

  	if (argc > 0)
	{
		 /* This port has a specified vpn */
		 crname = argv[0];
	       
		 strncpy(cpbKey.crname, crname, CALLPLAN_ATTR_LEN-1);
		 strncpy(cpbKey.cpname, name, CALLPLAN_ATTR_LEN-1);

		 if (cpbEntry = (CallPlanBindEntry *)
				DbFindEntry(GDBMF(comm->data, DB_eCallPlanBind), 
				(char *)&cpbKey, sizeof(CallPlanBindKey)))
		 {
	 		CLIPRINTF((stdout, 
				"entry (%s/%s) already exists\n", 
					name, crname));
			rc = -xleExists;
			goto _return;
  		}
		else
		{
			/* Add the entry */
			cpbEntry = (CallPlanBindEntry *)
					malloc(sizeof(CallPlanBindEntry));

			memset(cpbEntry, 0, sizeof(CallPlanBindEntry));
			strncpy(cpbEntry->crname, crname, CALLPLAN_ATTR_LEN-1);
			strncpy(cpbEntry->cpname, name, CALLPLAN_ATTR_LEN-1);

			cpbEntry->sTime.tm_year = TM_ANY;
			cpbEntry->sTime.tm_yday = TM_ANY;
			cpbEntry->sTime.tm_wday = TM_ANY;
            cpbEntry->sTime.tm_mon = TM_ANY;
            cpbEntry->sTime.tm_mday = TM_ANY;
            cpbEntry->sTime.tm_hour = TM_ANY;
            cpbEntry->sTime.tm_min = TM_ANY;
            cpbEntry->sTime.tm_sec = TM_ANY;

			cpbEntry->fTime.tm_year = TM_ANY;
			cpbEntry->fTime.tm_yday = TM_ANY;
			cpbEntry->fTime.tm_wday = TM_ANY;
            cpbEntry->fTime.tm_mon = TM_ANY;
            cpbEntry->fTime.tm_mday = TM_ANY;
            cpbEntry->fTime.tm_hour = TM_ANY;
            cpbEntry->fTime.tm_min = TM_ANY;
            cpbEntry->fTime.tm_sec = TM_ANY;

			cpbEntry->mTime = time(0);

			/* Store the entry */
	 	 	if (DbStoreEntry(GDBMF(comm->data, DB_eCallPlanBind), 
				(char *)cpbEntry, sizeof(CallPlanBindEntry), 
			       (char *)&cpbKey, sizeof(CallPlanBindKey)) < 0)
	  		{
	       		CLIPRINTF((stdout,"database store error %d\n", errno));
				rc = -xleOpNoPerm;
				goto _return;
	  		}

			modified = 1;
		}

  		CloseDatabases((DefCommandData *)comm->data);

		if ((shmId = CacheAttach()) > 0)
		{
			/* Do our stuff here */
			CacheHandleCPB(cpbEntry, CLIOP_ADD);
			
			if (modified)
			{
				/* Inform our gateways about this change */
				GwUpdateCP(name);
			}

			CacheDetach();
		}

		free(cpbEntry);
     	return rc;
	} 

_return:
  	CloseDatabases((DefCommandData *)comm->data);

	return rc;
}     

int
HandleCRAdd(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCRAdd():";
   	char *name = NULL;
	VpnRouteKey routeKey = { 0 };
	VpnRouteEntry *routeEntry;
	int modified = 0;
	int shmId;
	int rc = xleOk;
     
	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	name = argv[0];
     
	if (strlen(name) <= 0)
	{
		/* name has to be non-null */
		return -xleInsuffArgs;
	}

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}

	strncpy(routeKey.crname, name, CALLPLAN_ATTR_LEN-1);

	argc -= 1;
	argv += 1;

	if (routeEntry = (VpnRouteEntry *)
		DbFindEntry(GDBMF(comm->data, DB_eCallRoute), 
		(char *)&routeKey, sizeof(VpnRouteKey)))
	{
		if (argc <= 0)
		{
			CLIPRINTF((stdout, 
				"entry (%s) already exists\n", name));
			rc = -xleExists;
			goto _return;
		}

		// else we may still edit...
  	}
	else
	{
		/* Add the entry */
		routeEntry = (VpnRouteEntry *)
					malloc(sizeof(VpnRouteEntry));

		memset(routeEntry, 0, sizeof(VpnRouteEntry));
		strncpy(routeEntry->crname, name, CALLPLAN_ATTR_LEN-1);
		routeEntry->mTime = time(0);
	}

	if (argc > 0)
	{
		// simulate the effect of the cr edit command
		GetAttrPairs(comm->name, &argc, &argv, routeEntry, 2);
	}

	/* Store the entry */
 	if (DbStoreEntry(GDBMF(comm->data, DB_eCallRoute), 
			(char *)routeEntry, sizeof(VpnRouteEntry), 
	        (char *)&routeKey, sizeof(VpnRouteKey)) < 0)
  	{
       	CLIPRINTF((stdout,"database store error %d\n", errno));
		rc = -xleOpNoPerm;
		goto _return;
  	}

	modified = 1;

  	CloseDatabases((DefCommandData *)comm->data);

	if ((shmId = CacheAttach()) > 0)
	{
		/* Do our stuff here */
		CacheHandleCR(routeEntry, CLIOP_ADD);
			
		if (modified)
		{
			/* Inform our gateways about this change */
			GwUpdateCR(name);
		}

		CacheDetach();
	}

	free(routeEntry);
    return rc;

_return:
  	CloseDatabases((DefCommandData *)comm->data);

	return rc;
}     

int
HandleCPDelete(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCPDelete():";
   	char *name = NULL, *crname = NULL;
	CallPlanKey callPlanKey = { 0 };
	CallPlanBindKey cpbKey = { 0 };
	CallPlanBindKey *rkey, *orkey;
	int shmId;
	int modified = 0;
	int rc = xleOk;

	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	name = argv[0];
     
	strncpy(callPlanKey.cpname, name, CALLPLAN_ATTR_LEN-1);

	if (argc > 1)
	{
		crname = argv[1];
	}

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}

	/* Handle the case of only deleting the cp entry from database */
	if (crname == NULL)
	{
		/* This is a cp delete. We must verify that
		 * the cp is empty.
		 * If not empty, just return.
		 */
		 for (rkey = (CallPlanBindKey *)DbGetFirstKey(GDBMF(comm->data, 
						DB_eCallPlanBind)); 
			rkey != 0; 
	  		rkey = (CallPlanBindKey *)DbGetNextKey(GDBMF(comm->data, 
					DB_eCallPlanBind),
					(char *)rkey, 
					sizeof(CallPlanBindKey)),
	       				free(orkey))
		 {
	
			 if (!strcmp(rkey->cpname, name)) 
			 {
				rc = -xleExists;
				CLIPRINTF((stdout, "Entry Not Empty\n"));
				goto _return;
			}
	  		orkey = rkey;
   		}
		
		/* At this point we can delete the cp entry */
		if (DbDeleteEntry(GDBMF(comm->data, DB_eCallPlan),
			(char *)&callPlanKey, sizeof(CallPlanKey)) < 0)
		{
	  		rc = -xleNoEntry;
		}
		else
		{
			CLIPRINTF((stdout, "Entry Deleted Successfully\n"));
		}
		goto _return;
	}

	/* Now handle the case of deleting the call route entry from
	 * the database as well as the cache.
	 */

	strncpy(cpbKey.crname, crname, CALLPLAN_ATTR_LEN-1);
	strncpy(cpbKey.cpname, name, CALLPLAN_ATTR_LEN-1);

	/* At this point we can delete the cp entry */
	if (DbDeleteEntry(GDBMF(comm->data, DB_eCallPlanBind),
		(char *)&cpbKey, sizeof(CallPlanBindKey)) < 0)
	{
	  	rc = -xleNoEntry;
	}
	else
	{
		CLIPRINTF((stdout, "Entry Deleted Successfully\n"));

  		CloseDatabases((DefCommandData *)comm->data);

		modified = 1;
		if ((shmId = CacheAttach()) > 0)
		{
			/* Do our stuff here */
			CacheHandleCPB((CallPlanBindEntry*)&cpbKey, CLIOP_DELETE);

			if (modified)
			{
				/* Inform our gateways about this change */
				GwUpdateCP(name);
			}

			CacheDetach();
		}

		return rc;
	}

_return:
    CloseDatabases((DefCommandData *)comm->data);
    return rc;
}

int
HandleCPEdit(Command *comm, int argc, char **argv)
{
	char *name = NULL;
	CallPlanKey callPlanKey = { 0 };
	CallPlanEntry *tmp;
	char storeb[256];
	int shmId;
	int modified = 0;
	int rc = xleOk;
     
	if (argc <= 1)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	//HandleCommandUsage(comm, argc, argv);
	 	CPBEditHelp(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

	if (!(argc % 2))
	{
		/* This is a command to edit the binding */
		return HandleCPBEdit(comm, argc, argv);
	}

	name = argv[0];

	strncpy(callPlanKey.cpname, name, CALLPLAN_ATTR_LEN-1);

   	if (OpenDatabases((DefCommandData *)comm->data) < 0)
   	{
	  	return -xleOpNoPerm;
   	}

    tmp = (CallPlanEntry *) DbFindEntry(GDBMF(comm->data, DB_eCallPlan), 
			(char *)&callPlanKey, sizeof(CallPlanKey));

	if (tmp == NULL)
	{
	 	CLIPRINTF((stdout, "no call plan entry %s\n", name));

	  	CloseDatabases((DefCommandData *)comm->data);

	  	return -xleNoEntry;
   	}

	argc -= 1;
	argv += 1;

	if (argc > 0)
	{
		GetAttrPairs(comm->name, &argc, &argv, tmp, CLI_GET_ATTR_CP);
		goto _storedb;	
	}

	if (cliLibFlags == 0) {
		/* we should not be getting here... */
		printf("Invalid arguments passed from jserver\n");
		CloseDatabases((DefCommandData *)comm->data);
		return -xleInvalArgs;
	}

	CLIPRINTF((stdout, "call plan %s\n", name));

	CLIPRINTF((stdout, "Base CP [%s]: ", tmp->pcpname));
	GetInput(stdin, storeb, VPN_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->pcpname, storeb);

_storedb:
	tmp->mTime = time(0);

	/* At this point, the name may be changed. We need
	 * to tell everybody in this Vpn group to re-register
	 * group has changed.
	 */

	if (DbStoreEntry(GDBMF(comm->data, DB_eCallPlan), 
		(char *)tmp, sizeof(CallPlanEntry), 
	       (char *)&callPlanKey, sizeof(CallPlanKey)) < 0)
	{
		CLIPRINTF((stdout,"database store error %d\n", errno));
		rc = -xleOpNoPerm;
	}

	CloseDatabases((DefCommandData *)comm->data);

	modified = 1;

	if ((shmId = CacheAttach()) > 0)
	{

		if (modified)
		{
			/* Inform our gateways about this change */
			GwUpdateCP(name);
		}

		CacheDetach();
	}

	free(tmp);

   	return xleOk;
}

int
HandleCPBEdit(Command *comm, int argc, char **argv)
{
	char *name = NULL, *crname = NULL;
	CallPlanBindKey cpbKey = { 0 };
	CallPlanBindEntry *cpbEntry;
	char storeb[256], storec;
	int shmId;
	int modified = 0;
	int rc = xleOk;
     
	if (argc <= 2)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	 	CPBEditHelp(comm, argc, argv);
	  	//HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

	name = argv[0];
	crname = argv[1];

	strncpy(cpbKey.cpname, name, CALLPLAN_ATTR_LEN-1);
	strncpy(cpbKey.crname, crname, CALLPLAN_ATTR_LEN-1);

   	if (OpenDatabases((DefCommandData *)comm->data) < 0)
   	{
	  	return -xleOpNoPerm;
   	}

    cpbEntry = (CallPlanBindEntry *) 
			DbFindEntry(GDBMF(comm->data, DB_eCallPlanBind), 
			(char *)&cpbKey, sizeof(CallPlanBindKey));

	if (cpbEntry == NULL)
	{
	 	CLIPRINTF((stdout, "no call plan binding entry %s/%s\n", 
			name, crname));

	  	CloseDatabases((DefCommandData *)comm->data);

	  	return -xleNoEntry;
   	}

	argc -= 2;
	argv += 2;

	if (argc > 0)
	{
		GetAttrPairs(comm->name, &argc, &argv, cpbEntry, CLI_GET_ATTR_CPB);
		goto _storedb;	
	}

	if (cliLibFlags == 0) {
		/* we should not be getting here... */
		printf("Invalid arguments passed from jserver\n");
		CloseDatabases((DefCommandData *)comm->data);
		return -xleInvalArgs;
	}

	CLIPRINTF((stdout, "call plan %s/%s\n", name, crname));

	CLIPRINTF((stdout, "Priority [%d]: ", cpbEntry->priority));
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0) cpbEntry->priority = atoi(storeb);

	if (cpbEntry->crflags & (CRF_FORWARD|CRF_ROLLOVER))
	{
		CLIPRINTF((stdout, "Type [%s]: ", 
			(cpbEntry->crflags&CRF_FORWARD)?"forward":"rollover"));
	}
	else
	{
		CLIPRINTF((stdout, "Fwd Type [none]: "));
	}
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0) 
	{
		if (!strcmp(storeb, "forward"))
		{
			cpbEntry->crflags &= ~(CRF_FORWARD|CRF_ROLLOVER);
			cpbEntry->crflags |= CRF_FORWARD;
		}
		else if (!strcmp(storeb, "rollover"))
		{
			cpbEntry->crflags &= ~(CRF_FORWARD|CRF_ROLLOVER);
			cpbEntry->crflags |= CRF_ROLLOVER;
		}
		else
		{
			cpbEntry->crflags &= ~(CRF_FORWARD|CRF_ROLLOVER);
		}
	}

	CLIPRINTF((stdout, "StartTime: HH/MM/SS[%d/%d/%d]: ",
		cpbEntry->sTime.tm_hour, cpbEntry->sTime.tm_min,
		cpbEntry->sTime.tm_sec));
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0)
	{
		sscanf(storeb, "%d%c%d%c%d", 
				&cpbEntry->sTime.tm_hour,
				&storec,
				&cpbEntry->sTime.tm_min,
				&storec,
				&cpbEntry->sTime.tm_sec);
	}

	CLIPRINTF((stdout, "StartTime: YYYY/DOY/DOW/MM/DOM[%d/%d/%d/%d/%d]: ",
		(cpbEntry->sTime.tm_year>0)?1900+cpbEntry->sTime.tm_year:cpbEntry->sTime.tm_year, 
		cpbEntry->sTime.tm_yday,
		cpbEntry->sTime.tm_wday, cpbEntry->sTime.tm_mon, 
		cpbEntry->sTime.tm_mday));
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0)
	{
		sscanf(storeb, "%d%c%d%c%d%c%d%c%d", 
				&cpbEntry->sTime.tm_year, 
				&storec,
				&cpbEntry->sTime.tm_yday,
				&storec,
				&cpbEntry->sTime.tm_wday, 
				&storec,
				&cpbEntry->sTime.tm_mon, 
				&storec,
				&cpbEntry->sTime.tm_mday);
		if (cpbEntry->sTime.tm_year != -1)
		{
			cpbEntry->sTime.tm_year -= 1900;
		}
	}

	CLIPRINTF((stdout, "FinishTime: HH/MM/SS[%d/%d/%d]: ",
		cpbEntry->fTime.tm_hour, cpbEntry->fTime.tm_min,
		cpbEntry->fTime.tm_sec));
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0)
	{
		sscanf(storeb, "%d%c%d%c%d", 
				&cpbEntry->fTime.tm_hour,
				&storec,
				&cpbEntry->fTime.tm_min,
				&storec,
				&cpbEntry->fTime.tm_sec);
	}

	CLIPRINTF((stdout, "FinishTime: YYYY/DOY/DOW/MM/DOM[%d/%d/%d/%d/%d]: ",
		(cpbEntry->fTime.tm_year>0)?1900+cpbEntry->fTime.tm_year:cpbEntry->fTime.tm_year, 
		cpbEntry->fTime.tm_yday,
		cpbEntry->fTime.tm_wday, cpbEntry->fTime.tm_mon, 
		cpbEntry->fTime.tm_mday));
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0)
	{
		sscanf(storeb, "%d%c%d%c%d%c%d%c%d", 
				&cpbEntry->fTime.tm_year, 
				&storec,
				&cpbEntry->fTime.tm_yday,
				&storec,
				&cpbEntry->fTime.tm_wday, 
				&storec,
				&cpbEntry->fTime.tm_mon, 
				&storec,
				&cpbEntry->fTime.tm_mday);
		if (cpbEntry->fTime.tm_year != -1)
		{
			cpbEntry->fTime.tm_year -= 1900;
		}
	}

_storedb:
	cpbEntry->mTime = time(0);

	CPBTestTime(cpbEntry);

	/* At this point, the name may be changed. We need
	 * to tell everybody in this Vpn group to re-register
	 * group has changed.
	 */

	if (DbStoreEntry(GDBMF(comm->data, DB_eCallPlanBind), 
		(char *)cpbEntry, sizeof(CallPlanBindEntry), 
	       (char *)&cpbKey, sizeof(CallPlanBindKey)) < 0)
	{
		CLIPRINTF((stdout,"database store error %d\n", errno));
		rc = -xleOpNoPerm;
	}

	CloseDatabases((DefCommandData *)comm->data);

	modified = 1;

	if ((shmId = CacheAttach()) > 0)
	{
		/* Do our stuff here */
		CacheHandleCPB(cpbEntry, CLIOP_ADD);

		if (modified)
		{
			/* Inform our gateways about this change */
			GwUpdateCP(name);
		}

		CacheDetach();
	}

	free(cpbEntry);

   	return xleOk;
}

int
HandleCRDelete(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCRDelete():";
     	char *name = NULL;
	VpnRouteKey routeKey = { 0 };
	VpnRouteKey *rkey, *orkey;
	int shmId;
	int modified = 0;
	int rc = xleOk;

	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	name = argv[0];
     
	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}

	strncpy(routeKey.crname, name, CALLPLAN_ATTR_LEN-1);

	/* At this point we can delete the cp entry */
	if (DbDeleteEntry(GDBMF(comm->data, DB_eCallRoute),
		(char *)&routeKey, sizeof(VpnRouteKey)) < 0)
	{
	  	rc = -xleNoEntry;
	}
	else
	{
		CLIPRINTF((stdout, "Entry Deleted Successfully\n"));
		modified = 1;

  		CloseDatabases((DefCommandData *)comm->data);

		if ((shmId = CacheAttach()) > 0)
		{
			/* Do our stuff here */
			CacheHandleCR((VpnRouteEntry *) &routeKey, CLIOP_DELETE);

			if (modified)
			{
				/* Inform our gateways about this change */
				GwUpdateCR(name);
			}

			CacheDetach();
		}

		return rc;
	}

_return:
    CloseDatabases((DefCommandData *)comm->data);
    return rc;
}

int
HandleCPCache(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCPCache():";
	CacheCPBEntry *cacheCPBEntry;
	char ttypname[2*_POSIX_PATH_MAX];
	int n = 0;

	if (CacheAttach() < 0)
	{
		 CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
	}
	else
	{
		if (ttyname_r(1, ttypname, 2*_POSIX_PATH_MAX) != ENOTTY)
		{
			CLIPRINTF((stdout, "Please redirect output to a file\n"));
			return 0;
		}

		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);
	
		for (cacheCPBEntry = CacheGetFirst(cpbCache);
				cacheCPBEntry;
				cacheCPBEntry = CacheGetNext(cpbCache, &cacheCPBEntry->cpbEntry))
		{
			PrintCPBEntry(stdout, &cacheCPBEntry->cpbEntry);
			n++;
		}

		CacheReleaseLocks(cpbCache);

		CacheDetach();
	}

	fprintf(stdout, "#entries %d\n", n);
	return(0);
}

int
HandleCPList(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCPList():";
	CallPlanEntry *callPlanEntry, *ocallPlanEntry;
	CallPlanBindEntry *cpbEntry, *ocpbEntry;
	int n = 0;
	char *cpdbName, *cpbdbName;
	char ttypname[2*_POSIX_PATH_MAX];
	int printBindings = 0;

	if(argc && !strcmp(argv[0], "bindings"))
	{
		printBindings = 1;
	}

	if (ttyname_r(1, ttypname, 2*_POSIX_PATH_MAX) != ENOTTY)
	{
		CLIPRINTF((stdout, "Please redirect output to a file\n"));
		return 0;
	}

	/* ANY CHANGE IN THIS FUNCTION WILL NEED CORRESPONDING CHANGE IN
	   BRIDGESERVERIMPL.C */

	cpdbName = DBNAME((DefCommandData *)comm->data, DB_eCallPlan);
	cpbdbName = DBNAME((DefCommandData *)comm->data, DB_eCallPlanBind);

	if(printBindings)
	{
		for (cpbEntry = (CallPlanBindEntry *)DbExtractFirstEntry(cpbdbName, DB_eCallPlanBind);
				cpbEntry != NULL;
				cpbEntry = (CallPlanBindEntry *)DbExtractNextEntry(cpbdbName, DB_eCallPlanBind, 
					(char *)cpbEntry, sizeof(CallPlanBindKey)), 
				free(ocpbEntry))
		{
			PrintCPBEntry(stdout, cpbEntry);

			CLIPRINTF((stdout, "\n"));
			n ++;
			ocpbEntry = cpbEntry;
		}

		CLIPRINTF((stdout, "%d route bindings\n", n));
	}
	else
	{
		for (callPlanEntry = (CallPlanEntry *)DbExtractFirstEntry(cpdbName, DB_eCallPlan);
				callPlanEntry != NULL;
				callPlanEntry = (CallPlanEntry *)DbExtractNextEntry(cpdbName, DB_eCallPlan,
					callPlanEntry->cpname, sizeof(CallPlanKey)),
				free(ocallPlanEntry))
		{
			PrintCPEntry(stdout, callPlanEntry);

			CLIPRINTF((stdout, "\n"));
			ocallPlanEntry = callPlanEntry;
			n ++;
		}
		CLIPRINTF((stdout, "%d plans\n", n));
	}


	return xleOk;
}

int
HandleCRCache(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCRCache():";
	CacheRouteEntry *cacheRouteEntry;
	char ttypname[2*_POSIX_PATH_MAX];
	int n = 0;

	if (CacheAttach() < 0)
	{
		 CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
	}
	else
	{
		if (ttyname_r(1, ttypname, 2*_POSIX_PATH_MAX) != ENOTTY)
		{
			CLIPRINTF((stdout, "Please redirect output to a file\n"));
			return 0;
		}

		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	
		for (cacheRouteEntry = CacheGetFirst(cpCache);
				cacheRouteEntry;
				cacheRouteEntry = CacheGetNext(cpCache, &cacheRouteEntry->routeEntry))
		{
	  		PrintCREntry(stdout, &cacheRouteEntry->routeEntry);
			n++;
		}

		CacheReleaseLocks(cpCache);
		CacheDetach();
	}

	fprintf(stdout, "#entries %d\n", n);
	return(0);
}

int
HandleCRList(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCRList():";
	VpnRouteEntry *routeEntry, *orouteEntry;
	int n = 0, m = 0;
	char *dbName;
	char ttypname[2*_POSIX_PATH_MAX];

	if (ttyname_r(1, ttypname, 2*_POSIX_PATH_MAX) != ENOTTY)
	{
		CLIPRINTF((stdout, "Please redirect output to a file\n"));
		return 0;
	}

	/* ANY CHANGE IN THIS FUNCTION WILL NEED CORRESPONDING CHANGE IN
	   BRIDGESERVERIMPL.C */

	dbName = DBNAME((DefCommandData *)comm->data, DB_eCallRoute);

	for (routeEntry = (VpnRouteEntry *)DbExtractFirstEntry(dbName, DB_eCallRoute);
		routeEntry != NULL;
		routeEntry = (VpnRouteEntry *)DbExtractNextEntry(dbName, DB_eCallRoute, 
											routeEntry->crname, sizeof(VpnRouteKey)),
											free(orouteEntry))
	{
	  	PrintCREntry(stdout, routeEntry);
		
	  	CLIPRINTF((stdout, "\n"));
	  	m ++;
	  	orouteEntry = routeEntry;
     }

	CLIPRINTF((stdout, "%d routes\n", m));

	return xleOk;
}

int
HandleCREdit(Command *comm, int argc, char **argv)
{
	 char *name = NULL, *prefix = NULL, *dest = NULL;
	 CallPlanKey callPlanKey = { 0 };
	 CallPlanEntry *callPlanEntry;
	 VpnRouteKey routeKey = { 0 };
	 VpnRouteEntry *tmp;
	 char storeb[256];
	 int shmId;
	 int modified = 0;
	 int rc = xleOk;
     
   	if (argc <= 1)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	 	CREditHelp(comm, argc, argv);
	  	//HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

	name = argv[0];

	strncpy(routeKey.crname, name, CALLPLAN_ATTR_LEN-1);

   	if (OpenDatabases((DefCommandData *)comm->data) < 0)
   	{
	  	return -xleOpNoPerm;
   	}

     	tmp = (VpnRouteEntry *) DbFindEntry(GDBMF(comm->data, DB_eCallRoute), 
			(char *)&routeKey, sizeof(VpnRouteKey));

	if (tmp == NULL)
	{
	 	CLIPRINTF((stdout, "no call route entry %s\n", name));

	  	CloseDatabases((DefCommandData *)comm->data);

	  	return -xleNoEntry;
   	}

	argc -= 1;
	argv += 1;

	if (argc > 0)
	{
		GetAttrPairs(comm->name, &argc, &argv, tmp, CLI_GET_ATTR_ROUTE);
		goto _storedb;	
	}

	if (cliLibFlags == 0) {
		/* we should not be getting here... */
		printf("Invalid arguments passed from jserver\n");
		CloseDatabases((DefCommandData *)comm->data);
		return -xleInvalArgs;
	}

	CLIPRINTF((stdout, "call route %s\n", name));

	CLIPRINTF((stdout, "Dest [%s]: ", tmp->dest));
	GetInput(stdin, storeb, VPN_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->dest, storeb);

	CLIPRINTF((stdout, "Destlen [%d]: ", tmp->destlen));
	GetInput(stdin, storeb, VPN_LEN);
	if (strlen(storeb) > 0) tmp->destlen = atoi(storeb);

	CLIPRINTF((stdout, "Prefix [%s]: ", tmp->prefix));
	GetInput(stdin, storeb, VPN_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->prefix, storeb);

	if (!strlen(tmp->dest) && !strlen(tmp->prefix) &&
		!tmp->destlen)
	{
		CLIPRINTF((stdout, "Keep DNIS as default? [%s]: ", 
			tmp->crflags&CRF_DNISDEFAULT?"enable":"disable"));

		GetInput(stdin, storeb, 25);
		if (strlen(storeb) > 0) 
		{
			  if (!strcmp(storeb, "enable"))
			  {
				tmp->crflags |= CRF_DNISDEFAULT;
			  }
			  else
			  {
				tmp->crflags &= ~CRF_DNISDEFAULT;
			  }
		}
	
	}

	CLIPRINTF((stdout, "Src [%s]: ", tmp->src));
	GetInput(stdin, storeb, VPN_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->src, storeb);

	CLIPRINTF((stdout, "Srclen [%d]: ", tmp->srclen));
	GetInput(stdin, storeb, VPN_LEN);
	if (strlen(storeb) > 0) tmp->srclen = atoi(storeb);

	CLIPRINTF((stdout, "Src Prefix [%s]: ", tmp->srcprefix));
	GetInput(stdin, storeb, VPN_LEN);
	if (strlen(storeb) > 0) strcpy(tmp->srcprefix, storeb);

	CLIPRINTF((stdout, "Call Type[%s]: ", 
		RouteFlagsString(tmp->crflags&(CRF_CALLORIGIN|CRF_CALLDEST))));
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0) 
	{
		tmp->crflags = RouteFlagsValue(tmp->crflags, storeb);
	}

_storedb:
	tmp->mTime = time(0);

	/* At this point, the name may be changed. We need
	 * to tell everybody in this Vpn group to re-register
	 * group has changed.
	 */

	if (DbStoreEntry(GDBMF(comm->data, DB_eCallRoute), 
		(char *)tmp, sizeof(VpnRouteEntry), 
	       (char *)&routeKey, sizeof(VpnRouteKey)) < 0)
	{
		CLIPRINTF((stdout,"database store error %d\n", errno));
		rc = -xleOpNoPerm;
	}

	CloseDatabases((DefCommandData *)comm->data);

	modified = 1;

	if ((shmId = CacheAttach()) > 0)
	{
		/* Do our stuff here */
		CacheHandleCR(tmp, CLIOP_ADD);

		if (modified)
		{
			/* Inform our gateways about this change */
			GwUpdateCR(name);
		}

		CacheDetach();
	}

	free(tmp);

   	return xleOk;
}

void *
PrintGwList(void *data)
{
	CacheTableInfo *entry = (CacheTableInfo *)data;
	CacheTableInfo *start;

	if (entry == NULL)
	{
		return 0;
	}

	start = entry;

	do
	{
	  	PrintInfoEntry(stdout, &entry->data);
		entry = entry->next;

	} while (entry != start);

	return 0;
}

void *
PrintCPBList(void *data)
{
	CacheCPBEntry *entry = (CacheCPBEntry *)data;
	CacheCPBEntry *start;

	if (entry == NULL)
	{
		return 0;
	}

	start = entry;

	do
	{
	  	PrintCPBEntry(stdout, &entry->cpbEntry);

		// Now we should lookup the Gws in this plan
		PrintGwList(CacheGet(gwcpCache, entry->cpbEntry.cpname));

		entry = entry->crnext;

	} while (entry != start);

	return 0;
}

void *
PrintCRList(void *data)
{
	CacheRouteEntry *entry = (CacheRouteEntry *)data;
	CacheRouteEntry *start;

	if (entry == NULL)
	{
		return 0;
	}

	start = entry;

	do
	{
	  	PrintCREntry(stdout, &entry->routeEntry);

 		CLIPRINTF((stdout, "\n"));
		
		// Now we should lookup the Bindings and print them out too
		PrintCPBList(CacheGet(cpbcrCache, entry->routeEntry.crname));

		entry = entry->next;

	} while (entry != start);

	return 0;
}

int
HandleCRLkup(Command *comm, int argc, char **argv)
{
	 char *name = NULL, *dest = NULL, *prefix;
	 CacheRouteEntry *cacheRouteEntry = NULL;
	 VpnRouteKey routeKey = { 0 };
	 int shmId;
	 int rc = xleOk, i;
     
   	if (argc < 1)
   	{
	  	/* Here we prompt the user for the rest of the 
	   	* information
	   	*/
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

	name = argv[0];
	dest = argv[0];

	strncpy(routeKey.crname, name, CALLPLAN_ATTR_LEN-1);

	if ((shmId  = CacheAttach()) == -1)
	{
		 CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
	}
	else
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	
		cacheRouteEntry = CacheGet(cpCache, &routeKey);

		if (cacheRouteEntry)
		{
	 		CLIPRINTF((stdout, "route entry by name: %s\n", name));
	  		PrintCREntry(stdout, &cacheRouteEntry->routeEntry);
		}

		// We dont want to change the dest
		prefix = strdup(dest);

		for (i=strlen(dest); i > 0; i--)
		{
			prefix[i] = '\0';

			CLIPRINTF((stdout, "Matches for length=%d\n", i));

			// Also do a phone based lookup, matching all routes
			cacheRouteEntry = CacheGet(cpdestCache, prefix);

			PrintCRList(cacheRouteEntry);
		}

		CLIPRINTF((stdout, "Matches for length=0\n"));

		// null matches
		cacheRouteEntry = CacheGet(cpdestCache, "%NULL%");
		PrintCRList(cacheRouteEntry);

		CacheReleaseLocks(cpCache);
		CacheDetach();
	}

   	return xleOk;
}
