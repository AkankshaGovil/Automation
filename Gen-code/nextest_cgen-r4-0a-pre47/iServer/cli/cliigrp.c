#include "cli.h"
#include "serverp.h"
#include "avl.h"
#include "ifs.h"
#include <sys/time.h>
#include <strings.h>
#include "nxosd.h"
#include "log.h"

int
HandleIgrpAdd(Command *comm, int argc, char **argv)
{
   	char fn[] = "HandleIgrpAdd():";
    DB_tDb  dbstruct;
    DB  db;
	IgrpInfo 	new = {0};
    IgrpKey     key = {0};
    int rc = xleOk;
     
   	if (argc < 1)
   	{
	  	HandleCommandUsage(comm, argc, argv);
	  	return -xleInsuffArgs;
   	}

	if (strlen(argv[0]) >= REALM_NAME_LEN)
	{
        CLIPRINTF((stdout, "Igrp name too long:[%s]\n", argv[0]));
		return -xleInvalArgs;
	}

	nx_strlcpy(key.igrpNameKey, argv[0], IGRP_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(IGRP_DB_FILE, DB_eIgrp, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", IGRP_DB_FILE));
        return -xleInvalArgs;
    }

    if (DbFindEntry(db, (char *)&key, sizeof(IgrpKey)))
    {
        CLIPRINTF((stdout, 
            "entry (%s) already exists\n", key.igrpNameKey));
        rc = -xleExists;
        goto _return;
    }

    nx_strlcpy(new.igrpName, argv[0], IGRP_NAME_LEN);
	new.dndTime = time(0);

    if (DbStoreEntry(db, (char *)&new, sizeof(IgrpInfo), (char *)&key, 
						sizeof(IgrpKey))<0)
    {
        CLIPRINTF((stdout, "database store error %d\n", errno));
        rc = -xleOpNoPerm;
        goto _return;
    }

	if (CacheAttach() > 0)
    {
        CacheHandleIgrp(&new, CLIOP_ADD);
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
HandleIgrpDelete(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleIgrpDelete():";
    DB_tDb  dbstruct;
    DB db;
    IgrpKey key = {0};
    IgrpInfo  *ptr = NULL;
    int rc = xleOk;

    if (argc < 1)
    {
        HandleCommandUsage(comm, argc, argv);
        return -xleInsuffArgs;
    }

    nx_strlcpy(key.igrpNameKey, argv[0], IGRP_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(IGRP_DB_FILE, DB_eIgrp, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", IGRP_DB_FILE));
        return -xleInvalArgs;
    }

    if (!(ptr = (IgrpInfo *)DbFindEntry(db, (char *)&key, sizeof(IgrpKey))))
    {
        CLIPRINTF((stdout,"entry (%s) does not exists %d\n", key.igrpNameKey, errno));
        rc = -xleNoEntry;
        goto _return;
    }

    if (DbDeleteEntry(db, (char *)&key, sizeof(IgrpKey)) < 0)
    {
        CLIPRINTF((stdout, "database delete error %d\n", errno));
        rc = -xleOpNoPerm;
        goto _return;
    }

    // Update the cache also
    if (CacheAttach() > 0)
    {
        CacheHandleIgrp(ptr, CLIOP_DELETE);
        CacheDetach();
    }

    /* Handle Orphaned netoid entries */
_return:
    DbClose(&dbstruct);
    free(ptr);
    return rc;
}

int
HandleIgrpList(Command *comm, int argc, char **argv)
{
    char fn[] = "HandleIgrpList():";
    DB_tDb 	dbstruct;
    DB db;
    IgrpInfo  *ptr, *oldptr;
    int rc = xleOk;
    int n=0;

    ptr = (IgrpInfo *)DbExtractFirstEntry(IGRP_DB_FILE, DB_eIgrp);
    while(ptr)
    {
        PrintIgrpEntry(stdout, ptr);
        CLIPRINTF((stdout, "\n"));
        n ++;
        oldptr = ptr;
        ptr = (IgrpInfo *)DbExtractNextEntry(IGRP_DB_FILE,
        DB_eIgrp, (char *)oldptr->igrpName, sizeof(IgrpKey));
        free(oldptr);
    }
    CLIPRINTF((stdout, "%d igrp entries\n", n));
    return rc;
}

int
HandleIgrpEdit(Command *comm, int argc, char **argv)
{
   	char fn[] = "HandleIgrpEdit()";
    DB_tDb  dbstruct;
    DB db;
    IgrpInfo    *ptr = NULL;
    IgrpKey     key = { 0 };
    int rc = xleOk;

   	if (argc < 1)
   	{
	  	HandleCommandUsage(comm, argc, argv);
	  	rc = -xleInsuffArgs;
		return rc;
   	}

    nx_strlcpy (key.igrpNameKey, argv[0], IGRP_NAME_LEN);

    dbstruct.read_write = GDBM_WRCREAT;
    if (!(db = DbOpenByID(IGRP_DB_FILE, DB_eIgrp, &dbstruct)))
    {
        CLIPRINTF((stdout, "Unable to open %s\n", IGRP_DB_FILE));
        rc = -xleInvalArgs;
		goto _return;
    }

    if (!(ptr = (IgrpInfo *)DbFindEntry(db, (char *)&key, sizeof(IgrpKey)) ))
    {
        CLIPRINTF((stdout, "entry (%s) does not exist\n", key.igrpNameKey));
        rc = -xleNoEntry;
        goto _return;
    }

    argc -= 1;
    argv += 1;

    if (argc <= 0)
    {
        goto _return;
    }

    GetAttrPairs(comm->name, &argc, &argv, ptr, CLI_GET_ATTR_IGRP);

    /* Store the entry */
    if (DbStoreEntry(db, (char *)ptr, sizeof(IgrpInfo), 
				(char *)ptr->igrpName, sizeof(IgrpKey)) < 0)
    {
        CLIPRINTF((stdout,"database store error %d\n", errno));
        rc = -xleOpNoPerm;
        goto _return;
    }

    if (CacheAttach() > 0)
    {
        CacheHandleIgrp(ptr, CLIOP_REPLACE);
        CacheDetach();
    }
_return:
    DbClose(&dbstruct);
    free(ptr);
    return rc;
}

int
HandleIgrpCache(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleIgrpCache():";
	CacheIgrpInfo *ptr = NULL;
	unsigned int n=0;

	if (CacheAttach() >= 0)
	{
		CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

		for (ptr = CacheGetFirst(igrpCache);
            ptr;
            ptr = CacheGetNext(igrpCache, ptr->igrp.igrpName))
		{
	  		PrintIgrpEntry(stdout, &ptr->igrp);
			n++;
		}
	
		CacheReleaseLocks(igrpCache);
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
HandleIgrpLkup(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleIgrpLkup():";
	CacheIgrpInfo 	*ptr;
    unsigned char	igrpName[IGRP_NAME_LEN];

    if (argc < 1)
    {
		HandleCommandUsage(comm, argc, argv);
		return -xleInsuffArgs;
    }

    nx_strlcpy(igrpName, argv[0], IGRP_NAME_LEN);

    if (CacheAttach() >= 0)
    {
        CLIPRINTF((stdout, "Iedge Group Cache...\n"));
     
		CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

		ptr = CacheGet(igrpCache, igrpName);
		if (ptr)
		{
			PrintIgrpEntry(stdout, &ptr->igrp);
        }
		else
		{
			CLIPRINTF((stdout, "No matching entries\n"));
		}
        CacheReleaseLocks(igrpCache);
        CacheDetach();
	}
    else
	{
		CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
        return -1;
	}

    return 0;
}


void
IgrpAddCalls(char *name, short incalls, short outcalls, short totalcalls)	
{
	CacheIgrpInfo *entry = NULL;
	IgrpInfo  *igrp = NULL;

	if (name[0] != '\0')
	{
		if (entry = CacheGet(igrpCache, name))
		{
			igrp = &entry->igrp;
		}
	}

	if (igrp == NULL)
	{	
		CLIPRINTF((stdout, "No Igrp entry exists for %s\n", name ));
		return;
	}

	IgrpInCalls(igrp) += incalls;
	IgrpOutCalls(igrp) += outcalls;
	IgrpCallsTotal(igrp) += totalcalls;

	if ((IgrpInCalls(igrp) >= IgrpXInCalls(igrp)) || 
		(IgrpOutCalls(igrp) >= IgrpXOutCalls(igrp)) ||
		(IgrpCallsTotal(igrp) >= IgrpXCallsTotal(igrp)) )
	{
		time(&igrp->dndTime);
	}

	return;
}

void
IgrpDeleteCalls(char *name, short incalls, short outcalls, short totalcalls)	
{
	CacheIgrpInfo *entry = NULL;
	IgrpInfo  *igrp = NULL;

	if (name[0] != '\0')
	{
		if (entry = CacheGet(igrpCache, name))
		{
			igrp = &entry->igrp;
		}
	}

	if (igrp == NULL)
	{	
		CLIPRINTF((stdout, "No Igrp entry exists for %s\n", name));
		return;
	}

	IgrpInCalls(igrp) -= incalls;
	IgrpOutCalls(igrp) -= outcalls;
	IgrpCallsTotal(igrp) -= totalcalls;

	return;
}
