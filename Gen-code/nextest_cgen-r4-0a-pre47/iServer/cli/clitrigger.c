#include "cli.h"
#include "log.h"

int
HandleTriggerAdd(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleTriggerAdd():";
    DB_tDb dbstruct;
	DB db;
	TriggerEntry tgEntry = { 0 };
	TriggerKey tgKey = { 0 };
	int rc = xleOk;

	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	nx_strlcpy(tgKey.name, argv[0], TRIGGER_ATTR_LEN);

	dbstruct.read_write = GDBM_WRCREAT;
	if (!(db = DbOpenByID(TRIGGER_DB_FILE, DB_eTrigger, &dbstruct)))
	{
		CLIPRINTF((stdout, "Unable to open %s\n", TRIGGER_DB_FILE));
		return -xleInvalArgs;
	}

	if (DbFindEntry(db, (char *)&tgKey, sizeof(TriggerKey)))
	{
	 	CLIPRINTF((stdout, 
			"entry (%s) already exists\n", tgKey.name));

		rc = -xleExists;
		goto _return;
	}

	nx_strlcpy(tgEntry.name, argv[0], TRIGGER_ATTR_LEN);

	/* Store the entry */
	if (DbStoreEntry(db, (char *)&tgEntry, sizeof(TriggerEntry), 
			       (char *)&tgKey, sizeof(TriggerKey)) < 0)
	{
		CLIPRINTF((stdout,"database store error %d\n", errno));
		rc = -xleOpNoPerm;
		goto _return;
	}

	// Update the cache also
	if (CacheAttach() > 0)
	{
		CacheHandleTrigger(&tgEntry, CLIOP_ADD);
			
		CacheDetach();
	}

_return:
	DbClose(&dbstruct);

	return rc;
}

int
HandleTriggerDelete(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleTriggerDelete():";
    DB_tDb dbstruct;
	DB db;
	TriggerKey tgKey = { 0 };
	TriggerEntry *tgPtr;
	int rc = xleOk;

	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	nx_strlcpy(tgKey.name, argv[0], TRIGGER_ATTR_LEN);

	dbstruct.read_write = GDBM_WRCREAT;
	if (!(db = DbOpenByID(TRIGGER_DB_FILE, DB_eTrigger, &dbstruct)))
	{
		CLIPRINTF((stdout, "Unable to open %s\n", TRIGGER_DB_FILE));
		return -xleInvalArgs;
	}

	if (!(tgPtr = (TriggerEntry *)DbFindEntry(db, (char *)&tgKey, sizeof(TriggerKey))))
	{
	 	CLIPRINTF((stdout, 
			"entry (%s) does not exists\n", tgKey.name));

	  	rc = -xleNoEntry;
		DbClose(&dbstruct);
		goto _return;
	}

	/* Store the entry */
	if (DbDeleteEntry(db, (char *)&tgKey, sizeof(TriggerKey)) < 0)
	{
		CLIPRINTF((stdout,"database delete error %d\n", errno));
		rc = -xleOpNoPerm;
		DbClose(&dbstruct);
		goto _return;
	}

	DbClose(&dbstruct);

	// Update the cache also
	if (CacheAttach() > 0)
	{
		CacheHandleTrigger(tgPtr, CLIOP_DELETE);
			
		CacheDetach();
	}

_return:
	free(tgPtr);

	return rc;
}

int
HandleTriggerEdit(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleTriggerEdit():";
	DB_tDb dbstruct;
	DB db;
    	DB_tDb crdbstruct;
	DB crdb;
	TriggerEntry *tgPtr;
	TriggerKey tgKey = { 0 };
	int rc = xleOk;
	VpnRouteEntry *tmp;
        VpnRouteKey routeKey = { 0 };

	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	nx_strlcpy(tgKey.name, argv[0], TRIGGER_ATTR_LEN);

	dbstruct.read_write = GDBM_WRCREAT;
	if (!(db = DbOpenByID(TRIGGER_DB_FILE, DB_eTrigger, &dbstruct)))
	{
		CLIPRINTF((stdout, "Unable to open %s\n", TRIGGER_DB_FILE));
		return -xleInvalArgs;
	}

	if (!(tgPtr = (TriggerEntry *)DbFindEntry(db, (char *)&tgKey, sizeof(TriggerKey))))
	{
	 	CLIPRINTF((stdout, 
			"entry (%s) does not exists\n", tgKey.name));

	  	rc = -xleNoEntry;
		goto _return;
	}

	argc -= 1;
	argv += 1;

	if (argc <= 0)
	{
		goto _return;	
	}

	// Update the cache also
	if (CacheAttach() > 0)
	{
		CacheHandleTrigger(tgPtr, CLIOP_DELETE);
			
		CacheDetach();
	}

	GetAttrPairs(comm->name, &argc, &argv, tgPtr, CLI_GET_ATTR_TRIGGER);

	if(tgPtr->action == TRIGGER_ACTION_INSERTROUTE && strlen(tgPtr->actiondata) )
	{
		crdbstruct.read_write = GDBM_READER;
		if (!(crdb = DbOpenByID(CALLROUTE_DB_FILE, DB_eCallRoute, &crdbstruct)))
		{
			CLIPRINTF((stdout, "Unable to open %s\n", CALLROUTE_DB_FILE));
			rc = -xleInvalArgs;
			goto _return;
		}

        	strncpy(routeKey.crname, tgPtr->actiondata, CALLPLAN_ATTR_LEN-1);

		tmp = (VpnRouteEntry *)DbFindEntry(crdb, (char *)&routeKey, sizeof(VpnRouteKey));

		if(!tmp)
		{
			CLIPRINTF((stdout, "template route %s not found for trigger %s\n", tgPtr->actiondata, tgPtr->name));
			NETDEBUG(MCLI, NETLOG_DEBUG4, 
				("%s: template route %s not found for trigger %s\n", fn,
				tgPtr->actiondata, tgPtr->name));

			goto _template_route_error;
		}
		if( !(tmp->crflags & CRF_TEMPLATE) )
		{
			CLIPRINTF((stdout, "trigger %s can use only a template route\n", tgPtr->name));

			NETDEBUG(MCLI, NETLOG_DEBUG4, 
				("%s: trigger %s can use only a template route\n", fn, tgPtr->name));

			goto _template_route_error;
		}
		DbClose(&crdbstruct);
	}

	/* Store the entry */
	if (DbStoreEntry(db, (char *)tgPtr, sizeof(TriggerEntry), 
			       (char *)tgPtr, sizeof(TriggerKey)) < 0)
	{
		CLIPRINTF((stdout,"database store error %d\n", errno));
		rc = -xleOpNoPerm;
		goto _return;
	}

	// Update the cache also
	if (CacheAttach() > 0)
	{
		CacheHandleTrigger(tgPtr, CLIOP_ADD);
			
		CacheDetach();
	}

_return:
	DbClose(&dbstruct);
	free(tgPtr);

	return rc;

_template_route_error:
	rc = -xleInvalArgs;

	DbClose(&dbstruct);
	DbClose(&crdbstruct);
	free(tgPtr);

	return rc;
}

int
HandleTriggerList(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleTriggerList():";
    DB_tDb dbstruct;
	DB db;
	TriggerEntry *tgPtr, *otgPtr;
	int rc = xleOk;
	int n = 0;

	for (tgPtr = (TriggerEntry *)DbExtractFirstEntry(TRIGGER_DB_FILE, DB_eTrigger);
			tgPtr != NULL;
			tgPtr = (TriggerEntry *)DbExtractNextEntry(TRIGGER_DB_FILE, DB_eTrigger, 
		(char *)tgPtr, sizeof(TriggerKey)), free(otgPtr))
    {
		PrintTriggerEntry(stdout, tgPtr);
	  	CLIPRINTF((stdout, "\n"));
	  	n ++;
	  	otgPtr = tgPtr;
    }

	// CLIPRINTF((stdout, "%d triggers\n", n)); - not accurately tabulated
	return(0);
}

int
HandleTriggerCache(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleTriggerCache():";
	CacheTriggerEntry *cacheTriggerEntry;
	int n = 0;

	if (CacheAttach() < 0)
	{
		 CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
	}
	else
	{
		CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);
	
		for (cacheTriggerEntry = CacheGetFirst(triggerCache);
				cacheTriggerEntry;
				cacheTriggerEntry = CacheGetNext(triggerCache, &cacheTriggerEntry->trigger.event))
		{
	  		PrintTriggerEntry(stdout, &cacheTriggerEntry->trigger);
			fprintf(stdout, "#installed \t\t%d\n", cacheTriggerEntry->ntriggers);
	  		n ++;
		}

		CacheReleaseLocks(triggerCache);
		CacheDetach();
	}

	// CLIPRINTF((stdout, "%d triggers\n", n));  - not accurately tabulated
	return(0);
}

int
HandleTriggerPurge(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleTriggerPurge():";
    DB_tDb dbstruct;
	DB db;
	TriggerKey tgKey = { 0 };
	TriggerEntry *tgPtr;
	int rc = xleOk;

	if (argc < 1)
	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
	}

	// Update the cache also
	CacheAttach();

	CliDeleteTriggerRoutes(argv[0]);

	CacheDetach();

_return:
	return rc;
}

