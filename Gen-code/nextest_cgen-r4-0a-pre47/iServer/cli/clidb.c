#include "cli.h"
#include "serverp.h"
#include "rs.h"
#include "cacheinit.h"
#include "licenseIf.h"
#include "export.h"
#include "nxosd.h"

typedef struct DbOpStat {
	int numproc;
	int numerr;
} DbOpStat;

typedef struct DbOpStat CliOpStat[DB_eMax];

extern char netdbfilename[256], vpnsdbfilename[256], vpngdbfilename[256],
			cpdbfilename[256], crdbfilename[256], cpbdbfilename[256],
			attrdbfilename[256], realmdbfilename[256], igrpdbfilename[256],
			vnetdbfilename[256];
extern DB_tDb netdbstruct, vpnsdbstruct, vpngdbstruct, cpdbstruct, crdbstruct,
			cpbdbstruct, attrdbstruct, realmdbstruct, igrpdbstruct,
			vnetdbstruct;
extern DB netdb, vpnsdb, vpngdb, cpdb, crdb, cpbdb, attrdb, realmdb, igrpdb, vnetdb;
int clicmdop = CLIOP_NONE, clifileop = CLIOP_NONE;

// also print the number of keys in each of the databases
int
HandleDbInfo(Command *comm, int argc, char **argv)
{
	struct stat buf;
	int n;

	stat(DBNAME(comm->data, DB_eNetoids), &buf);

	CLIPRINTF((stdout, "iedge DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eNetoids), buf.st_size));
	CLIPRINTF((stdout, 
		"iedge Lock File: %s\n", DBLOCKNAME(comm->data, DB_eNetoids)));

	n = DbCount(GDBMF(comm->data, DB_eNetoids));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eIgrp), &buf);
	CLIPRINTF((stdout, "Igrp DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eIgrp), buf.st_size));
	CLIPRINTF((stdout, 
		"Igrp Lock File: %s\n", DBLOCKNAME(comm->data, DB_eIgrp)));

	n = DbCount(GDBMF(comm->data, DB_eIgrp));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eVnet), &buf);
	CLIPRINTF((stdout, "Vnet DB File: %s %d bytes\n", 
		DBNAME(comm->data, DB_eVnet), buf.st_size));
	CLIPRINTF((stdout, 
		"Vnet Lock File: %s\n", DBLOCKNAME(comm->data, DB_eVnet)));

	n = DbCount(GDBMF(comm->data, DB_eVnet));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eVpns), &buf);
	CLIPRINTF((stdout, "Vpns DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eVpns), buf.st_size));
	CLIPRINTF((stdout, 
		"Vpns Lock File: %s\n", DBLOCKNAME(comm->data, DB_eVpns)));

	n = DbCount(GDBMF(comm->data, DB_eVpns));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eVpnG), &buf);
	CLIPRINTF((stdout, "Vpns DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eVpnG), buf.st_size));
	CLIPRINTF((stdout, 
		"Vpns Lock File: %s\n", DBLOCKNAME(comm->data, DB_eVpnG)));

	n = DbCount(GDBMF(comm->data, DB_eVpnG));
	CLIPRINTF((stdout, "%d entries\n", n));

#if 0
	stat(DBNAME(comm->data, DB_eFax), &buf);
	CLIPRINTF((stdout, "Fax DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eFax), buf.st_size));
	CLIPRINTF((stdout, 
		"Fax Lock File: %s\n", DBLOCKNAME(comm->data, DB_eFax)));
#endif

	stat(DBNAME(comm->data, DB_eCallPlan), &buf);
	CLIPRINTF((stdout, "Call Plan  DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eCallPlan), buf.st_size));
	CLIPRINTF((stdout, 
		"Call Plan Lock File: %s\n", DBLOCKNAME(comm->data, DB_eCallPlan)));

	n = DbCount(GDBMF(comm->data, DB_eCallPlan));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eCallRoute), &buf);
	CLIPRINTF((stdout, "Call Route  DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eCallRoute), buf.st_size));
	CLIPRINTF((stdout, 
		"Call Route Lock File: %s\n", DBLOCKNAME(comm->data, DB_eCallRoute)));

	n = DbCount(GDBMF(comm->data, DB_eCallRoute));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eCallPlanBind), &buf);
	CLIPRINTF((stdout, "Call Plan Bind DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eCallPlanBind), buf.st_size));
	CLIPRINTF((stdout, 
		"Call Plan Bind Lock File: %s\n", DBLOCKNAME(comm->data, DB_eCallPlanBind)));

	n = DbCount(GDBMF(comm->data, DB_eCallPlanBind));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eAttribs), &buf);
	CLIPRINTF((stdout, "Attribs DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eAttribs), buf.st_size));
	CLIPRINTF((stdout, 
		"Attribs Lock File: %s\n", DBLOCKNAME(comm->data, DB_eAttribs)));

	n = DbCount(GDBMF(comm->data, DB_eAttribs));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eTrigger), &buf);
	CLIPRINTF((stdout, "Trigger DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eTrigger), buf.st_size));
	CLIPRINTF((stdout, 
		"Trigger Lock File: %s\n", DBLOCKNAME(comm->data, DB_eTrigger)));

	n = DbCount(GDBMF(comm->data, DB_eTrigger));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eRealm), &buf);
	CLIPRINTF((stdout, "Realm DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eRealm), buf.st_size));
	CLIPRINTF((stdout, 
		"Realm Lock File: %s\n", DBLOCKNAME(comm->data, DB_eRealm)));

	n = DbCount(GDBMF(comm->data, DB_eRealm));
	CLIPRINTF((stdout, "%d entries\n", n));

	stat(DBNAME(comm->data, DB_eIgrp), &buf);
	CLIPRINTF((stdout, "Igrp DB File: %s %ld bytes\n", 
		DBNAME(comm->data, DB_eIgrp), buf.st_size));
	CLIPRINTF((stdout, 
		"Igrp Lock File: %s\n", DBLOCKNAME(comm->data, DB_eIgrp)));

	n = DbCount(GDBMF(comm->data, DB_eIgrp));
	CLIPRINTF((stdout, "%d entries\n", n));

	// exporting for debugging purposes
	// NOT EXPORTING VNET DB, it is created from Realms for now
	stat(DBNAME(comm->data, DB_eVnet), &buf);
	CLIPRINTF((stdout, "Vnet DB File: %s %d bytes\n", 
		DBNAME(comm->data, DB_eVnet), buf.st_size));
	CLIPRINTF((stdout, 
		"Vnet Lock File: %s\n", DBLOCKNAME(comm->data, DB_eVnet)));

	n = DbCount(GDBMF(comm->data, DB_eVnet));
	CLIPRINTF((stdout, "%d entries\n", n));

	return xleOk;
}

int
HandleDbInit(Command *comm, int argc, char **argv)
{
	/* Do nothing */
	return xleOk;
}

int
HandleDbOrg(Command *comm, int argc, char **argv)
{
	int k;

    for (k = 0; k < DB_eMax; k ++)
    {
		gdbm_reorganize(GDBMF(comm->data, k));
	}

	return xleOk;
}

int
SetupDbs(int local, char *prefix)
{
	extern char netdbfilename[256], vpnsdbfilename[256], 
			vpngdbfilename[256], cpdbfilename[256], crdbfilename[256], 
			cpbdbfilename[256], attrdbfilename[256], trgdbfilename[256], realmdbfilename[256], 
			igrpdbfilename[256], vnetdbfilename[256];
    extern DB_tDb netdbstruct, vpnsdbstruct, vpngdbstruct, cpdbstruct, 
			crdbstruct, cpbdbstruct, attrdbstruct, trgdbstruct, realmdbstruct, igrpdbstruct,
			vnetdbstruct;
    extern DB netdb, vpnsdb, vpngdb, cpdb, crdb, cpbdb, attrdb, trgdb, realmdb, igrpdb, vnetdb;

	memset(netdbfilename, 0, 256);
	memset(vpnsdbfilename, 0, 256);
	memset(vpngdbfilename, 0, 256);
	memset(cpdbfilename, 0, 256);
	memset(crdbfilename, 0, 256);
	memset(cpbdbfilename, 0, 256);
	memset(attrdbfilename, 0, 256);
	memset(trgdbfilename, 0, 256);
	memset(realmdbfilename, 0, 256);
	memset(igrpdbfilename, 0, 256);
	memset(vnetdbfilename, 0, 256);
	
	if (local && prefix) nx_strlcpy(netdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(vpnsdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(vpngdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(cpdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(crdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(cpbdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(attrdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(trgdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(realmdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(igrpdbfilename, prefix, 256);
	if (local && prefix) nx_strlcpy(vnetdbfilename, prefix, 256);

    nx_strlcat(netdbfilename, local?NETOIDS_DB_FNAME:NETOIDS_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", netdbfilename));

    nx_strlcat(vpnsdbfilename, local?VPNS_DB_FNAME:VPNS_DB_FILE, 256); 
    CLIPRINTF((stdout, "%s\n", vpnsdbfilename));

    nx_strlcat(vpngdbfilename, local?VPNG_DB_FNAME:VPNG_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", vpngdbfilename));

    nx_strlcat(cpdbfilename, local?CALLPLAN_DB_FNAME:CALLPLAN_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", cpdbfilename));

    nx_strlcat(crdbfilename, local?CALLROUTE_DB_FNAME:CALLROUTE_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", crdbfilename));

    nx_strlcat(cpbdbfilename, local?CALLPLANBIND_DB_FNAME:CALLPLANBIND_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", cpbdbfilename));

    nx_strlcat(attrdbfilename, local?ATTRIBS_DB_FNAME:ATTRIBS_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", attrdbfilename));

    nx_strlcat(trgdbfilename, local?TRIGGER_DB_FNAME:TRIGGER_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", trgdbfilename));

    nx_strlcat(realmdbfilename, local?REALM_DB_FNAME:REALM_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", realmdbfilename));

    nx_strlcat(igrpdbfilename, local?IGRP_DB_FNAME:IGRP_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", igrpdbfilename));

    nx_strlcat(vnetdbfilename, local?VNET_DB_FNAME:VNET_DB_FILE, 256);
    CLIPRINTF((stdout, "%s\n", vnetdbfilename));

	return 0;
}

int
HandleDbAdd(Command *comm, int argc, char **argv)
{
    DefCommandData *dbDefaults = (DefCommandData *)comm->data;
    int plen;
    char *dot;
	int revno = -1, argcOrig, rc = -1;
	char **argvOrig;
	char filepath[256];

    if (argc != 1)
    {
		/* Here we prompt the user for the rest of the 
	 	* information
	 	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -1;
    }

	argcOrig = argc;
	argvOrig = argv;

    plen = strlen(argv[0]);
    if (strlen(clifiledir) && (argv[0][0] != '/'))
    {
	// patch specified is not absolute and we
	// have a directory prefix defined
	snprintf(filepath, 256, "%s/%s", clifiledir, argv[0]);
    }
    else
    {
	snprintf(filepath, 256, "%s", argv[0]);
    }

	SetupDbs(0, 0);

    strcpy(realmdbfilename, REALM_DB_FILE);
    CLIPRINTF((stdout, "%s\n", realmdbfilename));

    strcpy(igrpdbfilename, IGRP_DB_FILE);
    CLIPRINTF((stdout, "%s\n", igrpdbfilename));

    clicmdop = CLIOP_ADD;
    DbResetStats();

	if (CacheAttach() < 0)
	{
		LsMemStructReset();
	}

    /* Now we can call the parser to parse the input file */
    rc = parse_input(filepath); 

    clicmdop = CLIOP_NONE;

	CacheDetach();

	rc = (rc > 0)? -xleIOError:(rc<0)? -xleInvalArgs:xleOk;

	if (rc == xleOk)
	{
		// Obtain the new revision number of the db
		revno = CliUpdateRev();

		// Post the cp command
		CliPostCpCommand(argv[0], revno, 0, 0);
	}

    return rc;
}

int
HandleDbReplace(Command *comm, int argc, char **argv)
{
    DefCommandData *dbDefaults = (DefCommandData *)comm->data;
    int plen;
    char *dot;
	int revno = -1, argcOrig, rc = -1;
	char **argvOrig;
	char filepath[256];

    if (argc != 1)
    {
	 /* Here we prompt the user for the rest of the 
	  * information
	  */
	 HandleCommandUsage(comm, argc, argv);
	 return -1;
    }

	argcOrig = argc;
	argvOrig = argv;

     plen = strlen(argv[0]);
    if (strlen(clifiledir) && (argv[0][0] != '/'))
    {
	// patch specified is not absolute and we
	// have a directory prefix defined
	snprintf(filepath, 256, "%s/%s", clifiledir, argv[0]);
    }
    else
    {
	snprintf(filepath, 256, "%s", argv[0]);
    }

	SetupDbs(0, 0);

     strcpy(realmdbfilename, REALM_DB_FILE);
     CLIPRINTF((stdout, "%s\n", realmdbfilename));

     strcpy(igrpdbfilename, IGRP_DB_FILE);
     CLIPRINTF((stdout, "%s\n", igrpdbfilename));

     clicmdop = CLIOP_REPLACE;
     DbResetStats();

	if (CacheAttach() < 0)
	{
		LsMemStructReset();
	}

     /* Now we can call the parser to parse the input file */
    rc = parse_input(filepath); 

    clicmdop = CLIOP_NONE;

	CacheDetach();

	rc = (rc > 0)? -xleIOError:(rc<0)? -xleInvalArgs:xleOk;

	if (rc == xleOk)
	{
		// Obtain the new revision number of the db
		revno = CliUpdateRev();

		// Post the cli command
		CliPostCpCommand(argv[0], revno, 0, 0);
	}

    return rc;
}

int
HandleDbDelete(Command *comm, int argc, char **argv)
{
     DefCommandData *dbDefaults = (DefCommandData *)comm->data;
     int plen;
     char *dot;
	int revno = -1, argcOrig, rc = -1;
	char **argvOrig;
	char filepath[256];

     if (argc != 1)
     {
	 /* Here we prompt the user for the rest of the 
	  * information
	  */
	 HandleCommandUsage(comm, argc, argv);
	 return -1;
     }

	argcOrig = argc;
	argvOrig = argv;

     plen = strlen(argv[0]);
    if (strlen(clifiledir) && (argv[0][0] != '/'))
    {
	// patch specified is not absolute and we
	// have a directory prefix defined
	snprintf(filepath, 256, "%s/%s", clifiledir, argv[0]);
    }
    else
    {
	snprintf(filepath, 256, "%s", argv[0]);
    }

	SetupDbs(0, 0);

     strcpy(realmdbfilename, REALM_DB_FILE);
     CLIPRINTF((stdout, "%s\n", realmdbfilename));

     strcpy(igrpdbfilename, IGRP_DB_FILE);
     CLIPRINTF((stdout, "%s\n", igrpdbfilename));

     clicmdop = CLIOP_DELETE;
     DbResetStats();

	if (CacheAttach() < 0)
	{
		LsMemStructReset();
	}

     /* Now we can call the parser to parse the input file */
    rc = parse_input(filepath); 

     clicmdop = CLIOP_NONE;

	CacheDetach();

	rc = (rc > 0)? -xleIOError:(rc<0)? -xleInvalArgs:xleOk;

	if (rc == xleOk)
	{
		// Obtain the new revision number of the db
		revno = CliUpdateRev();

		// Post the cli command
		CliPostCpCommand(argv[0], revno, 0, 0);
	}

     return rc;
}

int
HandleDbCreate(Command *comm, int argc, char **argv)
{
     extern char netdbfilename[256], vpnsdbfilename[256], vpngdbfilename[256],
			cpdbfilename[256], crdbfilename[256], cpbdbfilename[256],
			attrdbfilename[256], trgdbfilename[256], realmdbfilename[256],
			igrpdbfilename[256],vnetdbfilename[256];
     extern DB_tDb netdbstruct, vpnsdbstruct, vpngdbstruct, cpdbstruct, crdbstruct,
			cpbdbstruct, attrdbstruct, trgdbstruct, realmdbstruct, igrpdbstruct;
     extern DB netdb, vpnsdb, vpngdb, cpdb, crdb, cpbdb, attrdb, trgdb, realmdb, igrpdb, vnetdb;
     DefCommandData *dbDefaults = (DefCommandData *)comm->data;
     int plen, rc = -1;
     char *dot;

     if (argc != 1)
     {
	 	HandleCommandUsage(comm, argc, argv);
	 	return -1;
     }

	SetupDbs(1, argv[0]);

     /* Now open the gdbm files... */
     netdbstruct.read_write = GDBM_NEWDB;

     if (!(netdb = DbOpenByID(netdbfilename, DB_eNetoids, &netdbstruct)))
     {
  	  CLIPRINTF((stdout, "Unable to open %s\n", netdbfilename));

	   return -xleInvalArgs;
     }


     vpnsdbstruct.read_write = GDBM_NEWDB;
     if (!(vpnsdb = DbOpenByID(vpnsdbfilename, DB_eVpns, &vpnsdbstruct)))
     {
    	CLIPRINTF((stdout, "Unable to open %s\n", vpnsdbfilename));
    	return -xleInvalArgs;
     }

     vpngdbstruct.read_write = GDBM_NEWDB;
     if (!(vpngdb = DbOpenByID(vpngdbfilename, DB_eVpnG, &vpngdbstruct)))
     {
    	CLIPRINTF((stdout, "Unable to open %s\n", vpngdbfilename));
    	return -xleInvalArgs;
     }

     cpdbstruct.read_write = GDBM_NEWDB;
     if (!(cpdb = DbOpenByID(cpdbfilename, DB_eCallPlan, &cpdbstruct)))
     {

      CLIPRINTF((stdout, "Unable to open %s\n", cpdbfilename));
	   return -xleInvalArgs;
     }

     crdbstruct.read_write = GDBM_NEWDB;
     if (!(crdb = DbOpenByID(crdbfilename, DB_eCallRoute, &crdbstruct)))
     {
    	CLIPRINTF((stdout, "Unable to open %s\n", crdbfilename));
	   return -xleInvalArgs;
     }

     cpbdbstruct.read_write = GDBM_NEWDB;
     if (!(cpbdb = DbOpenByID(cpbdbfilename, DB_eCallPlanBind, &cpbdbstruct)))
     {
    	CLIPRINTF((stdout, "Unable to open %s\n", cpbdbfilename));
	   return -xleInvalArgs;
     }
     attrdbstruct.read_write = GDBM_NEWDB;
     if (!(attrdb = DbOpenByID(attrdbfilename, DB_eAttribs, &attrdbstruct)))
     {
        CLIPRINTF((stdout, "Unable to open %s\n", attrdbfilename));
	     return -xleInvalArgs;
     }

     trgdbstruct.read_write = GDBM_NEWDB;
     if (!(trgdb = DbOpenByID(trgdbfilename, DB_eTrigger, &trgdbstruct)))
     {
        CLIPRINTF((stdout, "Unable to open %s\n", trgdbfilename));
	     return -xleInvalArgs;
     }

     realmdbstruct.read_write = GDBM_NEWDB;
     if (!(realmdb = DbOpenByID(realmdbfilename, DB_eRealm, &realmdbstruct)))
     {
        CLIPRINTF((stdout, "Unable to open %s\n", realmdbfilename));
	     return -xleInvalArgs;
     }
	 
     igrpdbstruct.read_write = GDBM_NEWDB;
     if (!(igrpdb = DbOpenByID(igrpdbfilename, DB_eIgrp, &igrpdbstruct)))
     {
        CLIPRINTF((stdout, "Unable to open %s\n", igrpdbfilename));
	     return -xleInvalArgs;
     }

     vnetdbstruct.read_write = GDBM_NEWDB;
     if (!(vnetdb = DbOpenByID(vnetdbfilename, DB_eVnet, &vnetdbstruct)))
     {
        CLIPRINTF((stdout, "Unable to open %s\n", vnetdbfilename));
	     return -xleInvalArgs;
     }

     DbClose(&netdbstruct);
     DbClose(&vpnsdbstruct);
     DbClose(&vpngdbstruct);
     DbClose(&cpdbstruct);
     DbClose(&crdbstruct);
     DbClose(&cpbdbstruct);
     DbClose(&attrdbstruct);
     DbClose(&trgdbstruct);
     DbClose(&realmdbstruct);
     DbClose(&igrpdbstruct);
     DbClose(&vnetdbstruct);

     netdb = 0; 
     vpnsdb = 0; 
     vpngdb = 0; 
     cpdb = 0; 
     crdb = 0; 
     cpbdb = 0; 
     attrdb = 0;
     trgdb = 0;
     realmdb = 0;
     igrpdb = 0;
     vnetdb = 0;

     DbResetStats();

     clicmdop = CLIOP_CREAT;

     /* Now we can call the parser to parse the input file */
     rc = parse_input(argv[0]); 

     clicmdop = CLIOP_NONE;

	rc = (rc > 0)? -xleIOError:(rc<0)? -xleInvalArgs:xleOk;

    return rc;
}

int
HandleDbCopy(Command *comm, int argc, char **argv)
{
     extern char netdbfilename[256], vpnsdbfilename[256], vpngdbfilename[256],
			cpdbfilename[256], crdbfilename[256], cpbdbfilename[256],
			attrdbfilename[256], trgdbfilename[256], realmdbfilename[256],
			igrpdbfilename[256], vnetdbfilename[256];
    int plen;
	char command[256], filepath[256];
	int ret, dblocked = 0;
	DB_tDb dbstruct;
	int revno = -1, argcOrig, rc = -1;
	char **argvOrig;

	/* Copy the database to the standard location */
	/* Hmmm the standard location database has already
	* been opened. We will overwrite it.
	*/

	// If the MSW is running, we will execute copy.
	// See if we can attach to the cache
	if (CacheAttach() < 0)
	{
		LsMemStructReset();
	}
	else
	{
		// MSW is running
		return HandleDbSwitch(comm, argc, argv);
	}

    if (argc != 1)
    {
		HandleCommandUsage(comm, argc, argv);
		return -1;
    }

	argcOrig = argc;
	argvOrig = argv;

    if (strlen(clifiledir) && (argv[0][0] != '/'))
    {
		// patch specified is not absolute and we
		// have a directory prefix defined
		snprintf(filepath, 256, "%s/%s", clifiledir, argv[0]);
    }
    else
    {
		snprintf(filepath, 256, "%s", argv[0]);
    }

	/* Just copy the stuff.. */

	SetupDbs(1, filepath);
	
	// Proceed to open and lock the database
	if (OpenDatabases((DefCommandData *)comm->data) < 0) {
		return -xleUndefined;
	}

	dblocked = 1;

	/* Now system it... */
	sprintf(command, "cp %s %s", netdbfilename, NETOIDS_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	IedgeCachePopulate(lsMem, GDBMF(comm->data, DB_eNetoids), 
		CACHE_POPULATE_ALL|CACHE_SAVE_DYNAMIC|CACHE_PURGE_OLD);

	if (lsMem)
	{
		nlm_initConfigPort();
	}
	
	sprintf(command, "cp %s %s", vpnsdbfilename, VPNS_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

	VpnCacheDestroyData(lsMem);
	VpnCacheInstantiate(lsMem);

	CacheReleaseLocks(vpnCache);

	VpnPopulate(lsMem, GDBMF(comm->data, DB_eVpns));
	
	sprintf(command, "cp %s %s", vpngdbfilename, VPNG_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

	VpnGCacheDestroyData(lsMem);
	VpnGCacheInstantiate(lsMem);

	CacheReleaseLocks(vpnGCache);

	VpnGPopulate(lsMem, GDBMF(comm->data, DB_eVpnG));
	
	sprintf(command, "cp %s %s", cpdbfilename, CALLPLAN_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	sprintf(command, "cp %s %s", crdbfilename, CALLROUTE_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

	CPCacheDestroyData(lsMem);
	CPCacheInstantiate(lsMem);

	CacheReleaseLocks(cpCache);

	CPPopulate(lsMem, GDBMF(comm->data, DB_eCallRoute));
	
	sprintf(command, "cp %s %s", cpbdbfilename, CALLPLANBIND_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

	CPBCacheDestroyData(lsMem);
	CPBCacheInstantiate(lsMem);
	CPBPopulate(lsMem, GDBMF(comm->data, DB_eCallPlanBind));

	CacheReleaseLocks(cpbCache);

	sprintf(command, "cp %s %s", attrdbfilename, ATTRIBS_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	sprintf(command, "cp %s %s", realmdbfilename, REALM_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	sprintf(command, "cp %s %s", trgdbfilename, TRIGGER_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

	RealmCacheDestroyData(lsMem);
	RealmCacheInstantiate(lsMem);
	RealmPopulate(lsMem, GDBMF(comm->data, DB_eRealm));

	CacheReleaseLocks(realmCache);

	sprintf(command, "cp %s %s", igrpdbfilename, IGRP_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

	IgrpCacheDestroyData(lsMem);
	IgrpCacheInstantiate(lsMem);
	IgrpPopulate(lsMem, GDBMF(comm->data, DB_eIgrp));

	CacheReleaseLocks(igrpCache);

	sprintf(command, "cp %s %s", vnetdbfilename, VNET_DB_FILE);
	CLIPRINTF((stdout, "%s\n", command));
	ret = system(command);
	if (ret != 0)
	{
		CLIPRINTF((stdout, "%s failed...\n", command));
		goto _error;
	}

	CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);

	VnetCacheDestroyData(lsMem);
	VnetCacheInstantiate(lsMem);
	VnetPopulate(lsMem, GDBMF(comm->data, DB_eVnet));

	CacheReleaseLocks(vnetCache);

	// Done, close the database and release locks
	CloseDatabases((DefCommandData *)comm->data);
		
	dblocked = 0;

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(netdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(vpnsdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(vpngdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(cpdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(crdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(cpbdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	CliPostCpCommand(attrdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(realmdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	CliPostCpCommand(trgdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(igrpdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCpCommand(vnetdbfilename, revno, 0, 0);

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Even if the command produced an error,
	// send it. This is because we dont know how much
	// of the db was changed.
	CliPostCmdline(CMD_CLI, "db copy", argcOrig, argvOrig, revno, 0, 0);

	return 0;

_error:
	if (dblocked)
	{
		// Done, close the database and release locks
		CloseDatabases((DefCommandData *)comm->data);
	}
		
	dblocked = 0;
	return -1;
}

int
HandleDbSwitch(Command *comm, int argc, char **argv)
{
	char 			**argvOrig, fn[] = "HandleDbSwitch";
   	NetoidInfoEntry *netInfo, *newInfo;
	CacheTableInfo	*cacheInfo, *newcacheInfo, *dupInfo;
	CacheIgrpInfo	*cacheIgrpInfo = NULL;
	CacheVnetEntry	*cacheVnetEntry = NULL;
	CacheVpnEntry	*entry;	
	VpnEntry		*curvpn, *newvpn;
	VpnGroupEntry	*curvpng, *newvpng;
	RouteEntry		*curroute, *newroute;
	CallPlanBindEntry	*curcpb, *newcpb;
	TriggerEntry	*curtrg, *newtrg;
	char 			*p;
	char 			filepath[256];
	char 			tmpdir[256], cmd[256];
	char 			netdbfname[256], vpnsdbfname[256], vpngdbfname[256]; 
	char			cpdbfname[256], crdbfname[256], cpbdbfname[256];
	char 			attdbfname[256], name[256], tname[256], dbname[256];
	char			trgdbfname[256], realmdbfname[256], igrpdbfname[256];
	char			vnetdbfname[256];
	RealmEntry		*currm, *newrm;
	IgrpInfo		*currigrp, *newigrp;
	VnetEntry		*currvnet, *newvnet;
	CacheGkInfo 	*gkInfo, *gkInfoNew, gkInfoEntry;
	DB				newdb, tmpdb;
	DB_tDb			dbstruct;
	CliOpStat		CliSt;
   	void			*key, *okey;
	int				argcOrig, status, revno, all = 0, dblocked = 0;

    if (argc < 1) {
		HandleCommandUsage(comm, argc, argv);
		return -xleInsuffArgs;
    }

	argcOrig = argc;
	argvOrig = argv;

    if (strlen(clifiledir) && (argv[0][0] != '/')) {
		snprintf(filepath, 256, "%s/%s", clifiledir, argv[0]);
		return -xleIOError;
    }
    else {
		snprintf(filepath, 256, "%s", argv[0]);
    }
	
	if (p = strrchr(argv[0], '/')) {
		p++; 	/* point to the next chracter after / */
	}
	else {
		p = argv[0];
	}
	snprintf(dbname, 256, "%s", p);

	/* Create a temprorary directory for the database */
	if (snprintf(tmpdir, 256, "/tmp/%lu/", ULONG_FMT(getpid())) < 0) {
		return -xleUndefined;
	}	
	CLIPRINTF((stdout, "Created /tmp/%lu temp dir\n", ULONG_FMT(getpid())));

	snprintf(cmd, 256, "mkdir -p %s \n", tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	/* Copy databases to the temporary directory */
	snprintf(cmd, 256, "cp %s%s %s \n", filepath, NETOIDS_DB_FNAME, tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, VPNS_DB_FNAME, tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, VPNG_DB_FNAME, tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, CALLPLAN_DB_FNAME, tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, CALLROUTE_DB_FNAME, 
		tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, CALLPLANBIND_DB_FNAME,
		tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, ATTRIBS_DB_FNAME, 
		tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, TRIGGER_DB_FNAME, 
		tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, REALM_DB_FNAME,
		tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, IGRP_DB_FNAME,
		tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s \n", filepath, VNET_DB_FNAME,
		tmpdir);
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	/* Create a scratch database*/
	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, NETOIDS_DB_FNAME, 
		tmpdir, dbname, NETOIDS_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, VPNS_DB_FNAME, 
		tmpdir, dbname, VPNS_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, VPNG_DB_FNAME, 
		tmpdir, dbname, VPNG_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, CALLPLAN_DB_FNAME, 
		tmpdir, dbname, CALLPLAN_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, CALLROUTE_DB_FNAME, 
		tmpdir, dbname, CALLROUTE_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF((stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, CALLPLANBIND_DB_FNAME,
		tmpdir, dbname, CALLPLANBIND_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF(( stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, ATTRIBS_DB_FNAME,
		tmpdir, dbname, ATTRIBS_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF(( stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, TRIGGER_DB_FNAME,
		tmpdir, dbname, TRIGGER_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF(( stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, REALM_DB_FNAME,
		tmpdir, dbname, REALM_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF(( stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, IGRP_DB_FNAME,
		tmpdir, dbname, IGRP_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF(( stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	snprintf(cmd, 256, "cp %s%s %s%s%s%s \n", filepath, VNET_DB_FNAME,
		tmpdir, dbname, VNET_DB_FNAME, ".jnk");
	if ((status = system(cmd)) != 0) {
		CLIPRINTF(( stdout, "%s failed: returned %d\n", cmd, status));
		goto _error;
	}

	// See if we can attach to the cache 
	if  (CacheAttach() < 0) {
		LsMemStructReset();
	}

	// Proceed to open and lock the database
	if (OpenDatabases((DefCommandData *)comm->data) < 0) {
		return -xleUndefined;
	}

	dblocked = 1;

	// iedge - database 

	// 	open the new and tmp iedge database 
	snprintf(netdbfname, 256, "%s%s%s", tmpdir, dbname, NETOIDS_DB_FNAME);
	if ((newdb = OpenDBFile(netdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", netdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (NetoidSNKey *)DbGetFirstInfoKey(GDBMF(comm->data, 
						DB_eNetoids)), 
			CliSt[DB_eNetoids].numproc = 0, 
			CliSt[DB_eNetoids].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (NetoidSNKey *)DbGetNextInfoKey(GDBMF(comm->data,
					DB_eNetoids), (char *)key, sizeof(NetoidSNKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		netInfo = DbFindInfoEntry(GDBMF(comm->data, DB_eNetoids),
			(char *)key, sizeof(NetoidSNKey));

		// get the info entry from the new database 
		newInfo = DbFindInfoEntry(newdb, 
			(char *)key, sizeof(NetoidSNKey));

		// Proceed to Open and Lock The Cache
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	
		if (IsGatekeeper(netInfo))
		{
			if (gkInfo = CacheGet(gkCache, netInfo))
			{
				memcpy(&gkInfoEntry, gkInfo, sizeof(CacheGkInfo));
			}
		}
		else
		{
			gkInfo = NULL;
		}
	
		// delete entry from the cache 
		// Get the cache information in cacheInfo
		if (!(cacheInfo = DeleteIedge(netInfo))) {
//			CLIPRINTF((stdout, "No such entry in cache \n"));
		}

		// If info entry exists in the new database
		if (newInfo) {
			// Count entry in the stats
			CliSt[DB_eNetoids].numproc++;

			// delete this entry from the temporary database 
			if (DbDeleteInfoEntry(tmpdb,
				(char *)key, sizeof(NetoidSNKey)) < 0) {
				NETERROR(MCLI, ("database deletion error\n"));
			}

			// Fill in the dynamic info  
			if (cacheInfo) 
			{
				UpdateIedgeDynamicInfo(newInfo, &cacheInfo->data);
			}

			//	update new db with dynamic info for this entry
			if (DbStoreInfoEntry(newdb, newInfo, 
				(char *)key, sizeof(NetoidSNKey)) < 0) {
				NETERROR(MCLI, ("database store error\n"));
			}

			// add the entry to the cache
			if (!(newcacheInfo = CacheDupInfoEntry(newInfo))) {
//				CLIPRINTF((stdout, 
//					"CacheDupInfoEntry: Out of Cache Memory\n"));  
			}

			free(newInfo); 

			if (newcacheInfo)
			{
				ResetIedgeDbFields(newcacheInfo, 1);

				if (AddIedge(newcacheInfo) < 0)
				{
					NETERROR(MCACHE,
						("Duplicate Iedge Entry found\n"));
					CliSt[DB_eNetoids].numerr++;
				}
				else if (IsGatekeeper(newInfo) && gkInfo)
				{
					gkInfoNew = CacheGet(gkCache, newInfo);
					if (gkInfoNew)
					{
						memcpy(gkInfoNew, &gkInfoEntry, sizeof(CacheGkInfo));
					}
				}
			}

			all++;
		}

		CacheReleaseLocks(regCache);

		if (cacheInfo) CFree(regCache)(cacheInfo);
		if (netInfo) free(netInfo);
	}

	// for each entry in the tmp database
    for (	key = (NetoidSNKey *)DbGetFirstInfoKey(tmpdb); 
			key != 0;
	 		okey = key,
	 		key = (NetoidSNKey *)DbGetNextInfoKey(tmpdb,
					(char *)key, sizeof(NetoidSNKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		netInfo = DbFindInfoEntry(tmpdb,
			(char *)key, sizeof(NetoidSNKey));

		CliSt[DB_eNetoids].numproc++;

		// check if duplicate entry exists in Cache
		if 	(dupInfo = GetDuplicateIedge(netInfo)) {
			NETERROR(MCACHE,
				("%s Duplicate Iedge Entry %s/%ld with %s/%ld\n",         
				fn, dupInfo->data.regid, dupInfo->data.uport,           
				netInfo->regid, netInfo->uport));
				CliSt[DB_eNetoids].numerr++;
			goto net_lp;
		}

		// add the entry to the cache
		cacheInfo = CacheDupInfoEntry(netInfo);  

		if (cacheInfo)
		{
			ResetIedgeFields(cacheInfo, 1);
			UpdateIedgeDynamicInfo(&cacheInfo->data, NULL);
		}

		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		if (cacheInfo && (AddIedge(cacheInfo) < 0)) {
			NETERROR(MCACHE,                                            
				("%s Duplicate Iedge Entry found\n", fn));

			CacheReleaseLocks(regCache);
			CliSt[DB_eNetoids].numerr++;

			goto net_lp;	
		}

		CacheReleaseLocks(regCache);
		all++;

net_lp:	free(netInfo);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	if (lsMem)
	{
		nlm_setconfigport(all);
	}
	
	CLIPRINTF(( stdout, "Updated iedge cache\n"));

	// vpns - database

	// 		open the new and tmp iedge database 
	snprintf(vpnsdbfname, 256, "%s%s%s", tmpdir, dbname, VPNS_DB_FNAME);
	if ((newdb = OpenDBFile(vpnsdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", vpnsdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (VpnKey *)DbGetFirstVpnKey(GDBMF(comm->data, 
						DB_eVpns)), 
			CliSt[DB_eVpns].numproc = 0, 
			CliSt[DB_eVpns].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (VpnKey *)DbGetNextVpnKey(GDBMF(comm->data,
					DB_eVpns), (char *)key, sizeof(VpnKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		curvpn = DbFindVpnEntry(GDBMF(comm->data, DB_eVpns),
			(char *)key, sizeof(VpnKey));

		// get the info entry from the new database 
		newvpn = DbFindVpnEntry(newdb, (char *)key, sizeof(VpnKey));
		
		// Delete entry from Cache
		CacheHandleVpn(curvpn, CLIOP_DELETE);

		// If vpn entry exists in the new database
		if (newvpn) {
			CliSt[DB_eVpns].numproc++;
			// delete this entry from the temporary database 
			if (DbDeleteVpnEntry(tmpdb,
				(char *)key, sizeof(VpnKey)) < 0) {
				NETERROR(MCLI, ("database deletion error\n"));
			}

			// add the vpn entry to the cache
			// Do I need to dup newvpn   ???
			if (CacheHandleVpn(newvpn, CLIOP_ADD)) {
				CliSt[DB_eVpns].numerr++;
			}
			free(newvpn); 
		}

		free(curvpn);
	}

	// for each entry in the tmp database
    for (	key = (VpnKey *)DbGetFirstVpnKey(tmpdb); 
			key != 0;
	 		okey = key,
	 		key = (VpnKey *)DbGetNextVpnKey(tmpdb,
					(char *)key, sizeof(VpnKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		newvpn = DbFindVpnEntry(tmpdb, (char *)key, sizeof(VpnKey));

		CliSt[DB_eVpns].numproc++;

		// add the entry to the cache
		if (CacheHandleVpn(newvpn, CLIOP_ADD)) {
				CliSt[DB_eVpns].numerr++;
		}

		free(newvpn);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated vpns cache\n"));

	// vpng - database

	// 		open the new and tmp vpng database 
	snprintf(vpngdbfname, 256, "%s%s%s", tmpdir, dbname, VPNG_DB_FNAME);
	if ((newdb = OpenDBFile(vpngdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", vpngdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (VpnGroupKey *)DbGetFirstVpnGKey(GDBMF(comm->data, 
						DB_eVpnG)), 
			CliSt[DB_eVpnG].numproc = 0, 
			CliSt[DB_eVpnG].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (VpnGroupKey *)DbGetNextVpnGKey(GDBMF(comm->data,
					DB_eVpnG), (char *)key, sizeof(VpnGroupKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		curvpng = DbFindVpnGEntry(GDBMF(comm->data, DB_eVpnG),
			(char *)key, sizeof(VpnGroupKey));

		// get the info entry from the new database 
		newvpng = DbFindVpnGEntry(newdb, 
			(char *)key, sizeof(VpnGroupKey));
		
		// Delete entry from Cache
		CacheHandleVpnG(curvpng, CLIOP_DELETE);

		// If vpn entry exists in the new database
		if (newvpng) {
			CliSt[DB_eVpnG].numproc++;
			// delete this entry from the temporary database 
			if (DbDeleteVpnGEntry(tmpdb,
				(char *)key, sizeof(VpnGroupKey)) < 0) {
				NETERROR(MCLI, ("database deletion error\n"));
			}

			// add the vpn entry to the cache
			// Do I need to dup newvpng   ???
			if (CacheHandleVpnG(newvpng, CLIOP_ADD)) {
				CliSt[DB_eVpnG].numerr++;
			}
			free(newvpng); 
		}

		free(curvpng);
	}

	// for each entry in the tmp database
    for (	key = (VpnGroupKey *)DbGetFirstVpnGKey(tmpdb); 
			key != 0;
	 		okey = key,
	 		key = (VpnGroupKey *)DbGetNextVpnGKey(tmpdb,
					(char *)key, sizeof(VpnGroupKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		newvpng = DbFindVpnGEntry(tmpdb, 
			(char *)key, sizeof(VpnGroupKey));

		CliSt[DB_eVpnG].numproc++;

		// add the entry to the cache
		// Do I need to dup newvpng   ???
		if (CacheHandleVpnG(newvpng, CLIOP_ADD)) {
			CliSt[DB_eVpnG].numerr++;
		}

		free(newvpng);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated vpng cache\n"));

	// cp - database

	// It seems nothings goes from cp database into the cache
	// Hence, we don't have to do anything. Eventually, we will
	// copy over the cp database
	snprintf(cpdbfname, 256, "%s%s%s", tmpdir, dbname, CALLPLAN_DB_FNAME);

	CLIPRINTF(( stdout, "Updated cp cache\n"));

	// cr - database
	// 		open the new and tmp cp database 
	snprintf(crdbfname, 256, "%s%s%s", tmpdir, dbname, CALLROUTE_DB_FNAME);
	if ((newdb = OpenDBFile(crdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", crdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (RouteKey *)DbGetFirstRouteKey(GDBMF(comm->data, 
						DB_eCallRoute)), 
			CliSt[DB_eCallRoute].numproc = 0, 
			CliSt[DB_eCallRoute].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (RouteKey *)DbGetNextRouteKey(GDBMF(comm->data,
					DB_eCallRoute), (char *)key, sizeof(RouteKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		curroute = DbFindRouteEntry(GDBMF(comm->data, DB_eCallRoute),
			(char *)key, sizeof(RouteKey));

		// get the info entry from the new database 
		newroute = DbFindRouteEntry(newdb, 
			(char *)key, sizeof(RouteKey));
		
		// Delete entry from Cache
		CacheHandleCR(curroute, CLIOP_DELETE);

		// If route entry exists in the new database
		if (newroute) {
			CliSt[DB_eCallRoute].numproc++;

			// delete this entry from the temporary database 
			if (DbDeleteRouteEntry(tmpdb,
				(char *)key, sizeof(RouteKey)) < 0) {
				NETERROR(MCLI, ("database deletion error\n"));
			}

			// add the route entry to the cache
			// Do I need to dup newroute   ???
			if (CacheHandleCR(newroute, CLIOP_ADD)) {
				CliSt[DB_eCallRoute].numerr++;
			}
			free(newroute); 
		}

		free(curroute);
	}

	// for each entry in the tmp database
    for (	key = (RouteKey *)DbGetFirstRouteKey(tmpdb); 
			key != 0;
	 		okey = key,
	 		key = (RouteKey *)DbGetNextRouteKey(tmpdb,
					(char *)key, sizeof(RouteKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		newroute = DbFindRouteEntry(tmpdb, 
			(char *)key, sizeof(RouteKey));

		CliSt[DB_eCallRoute].numproc++;

		// add the entry to the cache
		// Do I need to dup newvpng   ???
		if (CacheHandleCR(newroute, CLIOP_ADD)) {
			CliSt[DB_eCallRoute].numerr++;
		}

		free(newroute);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated cr cache\n"));

	// cpbind - database

	// 		open the new and tmp cp database 
	snprintf(cpbdbfname, 256, "%s%s%s", tmpdir, dbname, CALLPLANBIND_DB_FNAME);
	if ((newdb = OpenDBFile(cpbdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", cpbdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (CallPlanBindKey *)DbGetFirstCPBKey(GDBMF(comm->data, 
						DB_eCallPlanBind)), 
			CliSt[DB_eCallPlanBind].numproc = 0, 
			CliSt[DB_eCallPlanBind].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (CallPlanBindKey *)DbGetNextCPBKey(GDBMF(comm->data,
					DB_eCallPlanBind), (char *)key, sizeof(CallPlanBindKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		curcpb = DbFindCPBEntry(GDBMF(comm->data, DB_eCallPlanBind),
			(char *)key, sizeof(CallPlanBindKey));

		// get the info entry from the new database 
		newcpb = DbFindCPBEntry(newdb, 
			(char *)key, sizeof(CallPlanBindKey));
		
		// Delete entry from Cache
		CacheHandleCPB(curcpb, CLIOP_DELETE);

		// If route entry exists in the new database
		if (newcpb) {
			CliSt[DB_eCallPlanBind].numproc++;

			// delete this entry from the temporary database 
			if (DbDeleteCPBEntry(tmpdb,
				(char *)key, sizeof(CallPlanBindKey)) < 0) {
				NETERROR(MCLI, ("database deletion error\n"));
			}

			// add the route entry to the cache
			// Do I need to dup newcpb   ???
			if (CacheHandleCPB(newcpb, CLIOP_ADD)) {
				CliSt[DB_eCallPlanBind].numerr++;
			}
			free(newcpb); 
		}

		free(curcpb);
	}

	// for each entry in the tmp database
    for (	key = (CallPlanBindKey *)DbGetFirstCPBKey(tmpdb); 
			key != 0;
	 		okey = key,
	 		key = (CallPlanBindKey *)DbGetNextCPBKey(tmpdb,
					(char *)key, sizeof(CallPlanBindKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		newcpb = DbFindCPBEntry(tmpdb, 
			(char *)key, sizeof(CallPlanBindKey));

		CliSt[DB_eCallPlanBind].numproc++;

		// add the entry to the cache
		// Do I need to dup newcpb   ???
		if (CacheHandleCPB(newcpb, CLIOP_ADD)) {
			CliSt[DB_eCallPlanBind].numerr++;
		}

		free(newcpb);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated cpbind cache\n"));

	// attribs - database
	// Nothing to be done for attribs database 
	snprintf(attdbfname, 256, "%s%s%s", tmpdir, dbname, ATTRIBS_DB_FNAME);

	CLIPRINTF(( stdout, "Updated attribs cache\n"));

	// trigger - database

	// 		open the new and tmp trigger database 
	snprintf(trgdbfname, 256, "%s%s%s", tmpdir, dbname, TRIGGER_DB_FNAME);
	if ((newdb = OpenDBFile(trgdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", trgdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (TriggerKey *)DbGetFirstKey(GDBMF(comm->data, 
						DB_eTrigger)), 
			CliSt[DB_eTrigger].numproc = 0, 
			CliSt[DB_eTrigger].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (TriggerKey *)DbGetNextKey(GDBMF(comm->data,
					DB_eTrigger), (char *)key, sizeof(TriggerKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		curtrg = (TriggerEntry *)DbFindEntry(GDBMF(comm->data, DB_eTrigger),
			(char *)key, sizeof(TriggerKey));

		// get the info entry from the new database 
		newtrg = (TriggerEntry *)DbFindEntry(newdb, 
			(char *)key, sizeof(TriggerKey));
		
		// Delete entry from Cache
		CacheHandleTrigger(curtrg, CLIOP_DELETE);

		// If route entry exists in the new database
		if (newtrg) {
			CliSt[DB_eTrigger].numproc++;

			// delete this entry from the temporary database 
			if (DbDeleteEntry(tmpdb,
				(char *)key, sizeof(TriggerKey)) < 0) {
				NETERROR(MCLI, ("database deletion error\n"));
			}

			// add the trigger entry to the cache
			if (CacheHandleTrigger(newtrg, CLIOP_ADD)) {
				CliSt[DB_eTrigger].numerr++;
			}
			free(newtrg); 
		}

		free(curtrg);
	}

	// for each entry in the tmp database
    for (	key = (TriggerKey *)DbGetFirstKey(tmpdb); 
			key != 0;
	 		okey = key,
	 		key = (TriggerKey *)DbGetNextKey(tmpdb,
					(char *)key, sizeof(TriggerKey)),
	    	free(okey)
		) {
		// get the info entry from the current database
		newtrg = (TriggerEntry *)DbFindEntry(tmpdb, 
			(char *)key, sizeof(TriggerKey));

		CliSt[DB_eTrigger].numproc++;

		// add the entry to the cache
		// Do I need to dup newcpb   ???
		if (CacheHandleTrigger(newtrg, CLIOP_ADD)) {
			CliSt[DB_eTrigger].numerr++;
		}

		free(newtrg);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated trigger cache\n"));

	// realm - database

	// open the new and tmp vpng database 
	snprintf(realmdbfname, 256, "%s%s%s", tmpdir, dbname, REALM_DB_FNAME);

	if ((newdb = OpenDBFile(realmdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", realmdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (RealmKey *)DbGetFirstKey(GDBMF(comm->data, DB_eRealm)), 
			CliSt[DB_eRealm].numproc = 0, CliSt[DB_eRealm].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (RealmKey *)DbGetNextKey(GDBMF(comm->data, DB_eRealm), (char *)key, sizeof(RealmKey)),
			free(okey)  ) {
			// get the info entry from the current database
			currm = (RealmEntry *)DbFindEntry(GDBMF(comm->data, DB_eRealm), (char *)key, sizeof(RealmKey));

			// get the info entry from the new database 
			newrm = (RealmEntry *)DbFindEntry(newdb, (char *)key, sizeof(RealmKey));
		
			// Delete entry from Cache
			/* with redundancy secondary looses socket-id */
			/* CLIOP_ADD copies over dynamic info (e.g. socket-id )
			and then deletes the realm entry */

			/* CacheHandleRealm(currm, CLIOP_DELETE); */

			// If vpn entry exists in the new database
			if (newrm) {
				CliSt[DB_eRealm].numproc++;
				// delete this entry from the temporary database 
				if (DbDeleteEntry(tmpdb, (char *)key, sizeof(RealmKey)) < 0) 
					{ NETERROR(MCLI, ("database deletion error\n")); }

				// add the vpn entry to the cache
				// Do I need to dup newrm ???
				if (CacheHandleRealm(newrm, CLIOP_ADD)) 
					{ CliSt[DB_eRealm].numerr++; }
				free(newrm); 
			}

		free(currm);
	}

	// for each entry in the tmp database
    for (	key = (RealmKey *)DbGetFirstKey(tmpdb); 
			key != 0;
	 		okey = key,
			key = (RealmKey *)DbGetNextKey(tmpdb, (char *)key, sizeof(RealmKey)),
	    	free(okey) ) {
			// get the info entry from the current database
			newrm = (RealmEntry *)DbFindEntry(tmpdb, (char *)key, sizeof(RealmKey));

			CliSt[DB_eRealm].numproc++;

			// add the entry to the cache
			if (CacheHandleRealm(newrm, CLIOP_ADD)) 
				{ CliSt[DB_eRealm].numerr++; }

			free(newrm);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated realm cache\n"));


	// igrp - database

	// open the new and tmp igrp database 
	snprintf(igrpdbfname, 256, "%s%s%s", tmpdir, dbname, IGRP_DB_FNAME);

	if ((newdb = OpenDBFile(igrpdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", igrpdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (IgrpKey *)DbGetFirstKey(GDBMF(comm->data, DB_eIgrp)), 
			CliSt[DB_eIgrp].numproc = 0, CliSt[DB_eIgrp].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (IgrpKey *)DbGetNextKey(GDBMF(comm->data, DB_eIgrp), (char *)key, sizeof(IgrpKey)),
			free(okey)  ) {
			// get the info entry from the current database
			currigrp = (IgrpInfo *)DbFindEntry(GDBMF(comm->data, DB_eIgrp), (char *)key, sizeof(IgrpKey));

			// get the info entry from the new database 
			newigrp = (IgrpInfo *)DbFindEntry(newdb, (char *)key, sizeof(IgrpKey));
		
			// Delete entry from Cache
			CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);
			cacheIgrpInfo = CacheDelete(igrpCache, currigrp->igrpName);
			CacheReleaseLocks(igrpCache);

			// If igrp entry exists in the new database
			if (newigrp) {

				CliSt[DB_eIgrp].numproc++;
				// delete this entry from the temporary database 
				if (DbDeleteEntry(tmpdb, (char *)key, sizeof(IgrpKey)) < 0) 
					{ NETERROR(MCLI, ("database deletion error\n")); }

				// Fill in the dynamic info  
				if (cacheIgrpInfo) 
				{
					UpdateIgrpDynamicInfo(newigrp, &cacheIgrpInfo->igrp);
				}

				//	update new db with dynamic info for this entry
				if (DbStoreInfoEntry(newdb, newigrp, (char *)key, sizeof(IgrpKey)) < 0) {
					NETERROR(MCLI, ("database store error\n"));
				}

				// add the igrp entry to the cache
				if (CacheHandleIgrp(newigrp, CLIOP_ADD)) 
					{ CliSt[DB_eIgrp].numerr++; }
				free(newigrp); 
			}

		CFree(igrpCache)(cacheIgrpInfo);
		free(currigrp);
	}

	// for each entry in the tmp database
    for (	key = (IgrpKey *)DbGetFirstKey(tmpdb); 
			key != 0;
	 		okey = key,
			key = (IgrpKey *)DbGetNextKey(tmpdb, (char *)key, sizeof(IgrpKey)),
	    	free(okey) ) {
			// get the info entry from the current database
			newigrp = (IgrpInfo *)DbFindEntry(tmpdb, (char *)key, sizeof(IgrpKey));

			if(CacheGet(igrpCache, newigrp->igrpName) == NULL)
			{
				CliSt[DB_eIgrp].numproc++;

				/* reset all the dynamic entries */
				UpdateIgrpDynamicInfo(newigrp, NULL);

				// add the entry to the cache
				if (CacheHandleIgrp(newigrp, CLIOP_ADD)) 
					{ CliSt[DB_eIgrp].numerr++; }
			}

			free(newigrp);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated igrp cache\n"));


	// vnet - database

	// open the new and tmp vnet database 
	snprintf(vnetdbfname, 256, "%s%s%s", tmpdir, dbname, VNET_DB_FNAME);

	if ((newdb = OpenDBFile(vnetdbfname)) == NULL) {
		goto _error;
	}

	snprintf(tname, 256, "%s%s", vnetdbfname, ".jnk");
	if ((tmpdb = OpenDBFile(tname)) == NULL) {
		goto _error;
	}

	// for each key in the current database 
    for (	key = (VnetKey *)DbGetFirstKey(GDBMF(comm->data, DB_eVnet)), 
			CliSt[DB_eVnet].numproc = 0, CliSt[DB_eVnet].numerr = 0; 
			key != 0;
	 		okey = key,
	 		key = (VnetKey *)DbGetNextKey(GDBMF(comm->data, DB_eVnet), (char *)key, sizeof(VnetKey)),
			free(okey)  ) {
			// get the info entry from the current database
			currvnet = (VnetEntry *)DbFindEntry(GDBMF(comm->data, DB_eVnet), (char *)key, sizeof(VnetKey));

			// get the info entry from the new database 
			newvnet = (VnetEntry *)DbFindEntry(newdb, (char *)key, sizeof(VnetKey));
		
			// Delete entry from Cache
			CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);
			cacheVnetEntry = CacheDelete(vnetCache, currvnet->vnetName);
			CacheReleaseLocks(vnetCache);

			// If vnet entry exists in the new database
			if (newvnet) {

				CliSt[DB_eVnet].numproc++;
				// delete this entry from the temporary database 
				if (DbDeleteEntry(tmpdb, (char *)key, sizeof(VnetKey)) < 0) 
					{ NETERROR(MCLI, ("database deletion error\n")); }

				// Fill in the dynamic info  
				if (cacheVnetEntry) 
				{
					UpdateVnetDynamicInfo(newvnet, &cacheVnetEntry->vnet);
				}

				//	update new db with dynamic info for this entry
				if (DbStoreInfoEntry(newdb, newvnet, (char *)key, sizeof(VnetKey)) < 0) {
					NETERROR(MCLI, ("database store error\n"));
				}

				// add the vnet entry to the cache
				if (CacheHandleVnet(newvnet, CLIOP_ADD)) 
					{ CliSt[DB_eVnet].numerr++; }
				free(newvnet); 
			}

		CFree(vnetCache)(cacheVnetEntry);
		free(currvnet);
	}

	// for each entry in the tmp database
    for (	key = (VnetKey *)DbGetFirstKey(tmpdb); 
			key != 0;
	 		okey = key,
			key = (VnetKey *)DbGetNextKey(tmpdb, (char *)key, sizeof(VnetKey)),
	    	free(okey) ) {
			// get the info entry from the current database
			newvnet = (VnetEntry *)DbFindEntry(tmpdb, (char *)key, sizeof(VnetKey));

			if(CacheGet(vnetCache, newvnet->vnetName) == NULL)
			{
				CliSt[DB_eVnet].numproc++;

				/* reset all the dynamic entries */
				UpdateVnetDynamicInfo(newvnet, NULL);

				// add the entry to the cache
				if (CacheHandleVnet(newvnet, CLIOP_ADD)) 
					{ CliSt[DB_eVnet].numerr++; }
			}

			free(newvnet);
	}

	CloseDBFile(tmpdb);
	CloseDBFile(newdb);

	CLIPRINTF(( stdout, "Updated vnet cache\n"));

	// Copy all databases on top of current databases 
	snprintf(cmd, 256, "cp %s %s", netdbfname, NETOIDS_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}
	
	snprintf(cmd, 256,  "cp %s %s", vpnsdbfname, VPNS_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}
	
	snprintf(cmd, 256, "cp %s %s", vpngdbfname, VPNG_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}
	
	snprintf(cmd, 256, "cp %s %s", cpdbfname, CALLPLAN_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}
	
	snprintf(cmd, 256, "cp %s %s", crdbfname, CALLROUTE_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}
	
	snprintf(cmd, 256, "cp %s %s", cpbdbfname, CALLPLANBIND_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}
	
	snprintf(cmd, 256, "cp %s %s", attdbfname, ATTRIBS_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}

	snprintf(cmd, 256, "cp %s %s", realmdbfname, REALM_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}

	snprintf(cmd, 256, "cp %s %s", trgdbfname, TRIGGER_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}

	snprintf(cmd, 256, "cp %s %s", igrpdbfname, IGRP_DB_FILE);
	CLIPRINTF((stdout, "%s\n", cmd));
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}

	snprintf(cmd, 256, "rm -rf %s", tmpdir);
	status = system(cmd);
	if (status != 0) {
		CLIPRINTF((stdout, "%s failed...\n", cmd));
	}

    CLIPRINTF((stdout, "Switching to %s database...\n", argv[0]));

	if (cli_debug) {
		CLIPRINTF((stdout, "iedge entries processed - %d, error - %d\n",
			CliSt[DB_eNetoids].numproc, CliSt[DB_eNetoids].numerr));
		CLIPRINTF((stdout, "vpns entries processed - %d, error - %d\n",
			CliSt[DB_eVpns].numproc, CliSt[DB_eVpns].numerr));
		CLIPRINTF((stdout, "vpn Group entries processed - %d, error - %d\n",
			CliSt[DB_eVpnG].numproc, CliSt[DB_eVpnG].numerr));
		CLIPRINTF((stdout, "Route entries processed - %d, error - %d\n",
			CliSt[DB_eCallRoute].numproc, CliSt[DB_eCallRoute].numerr));
		CLIPRINTF((stdout, 
			"Calling plan binding entries processed - %d, error - %d\n",
			CliSt[DB_eCallPlanBind].numproc, 
			CliSt[DB_eCallPlanBind].numerr));
		CLIPRINTF((stdout, "Realm entries processed - %d, error - %d\n",
			CliSt[DB_eRealm].numproc, CliSt[DB_eRealm].numerr));
		CLIPRINTF((stdout, "Trigger entries processed - %d, error - %d\n",
			CliSt[DB_eTrigger].numproc, CliSt[DB_eTrigger].numerr));
		CLIPRINTF((stdout, "Igrp entries processed - %d, error - %d\n",
			CliSt[DB_eIgrp].numproc, CliSt[DB_eIgrp].numerr));
	}

	// Done, close the database and release locks
	CloseDatabases((DefCommandData *)comm->data);
		
	dblocked = 0;

	// Post the copy commands and then post the switch
	// command itself
	revno = CliUpdateRev();
	CliPostCpCommand(netdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(vpnsdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(vpngdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(cpdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(crdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(cpbdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(attdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(realmdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(trgdbfname, revno, 0, 0);

	revno = CliUpdateRev();
	CliPostCpCommand(igrpdbfname, revno, 0, 0);
	// Switch command

	// Obtain the new revision number of the db
	revno = CliUpdateRev();

	// Post the cli command
	CliPostCmdline(CMD_CLI, "db switch", argcOrig, argvOrig, revno, 0, 0);

	return 0;

_error:
	// If some error occured remove the tmp directory
	snprintf(cmd, 256, "rm -rf %s", tmpdir);
	status = system(cmd);

	// if some error occurred, remove the tmp dir
	snprintf(cmd, 256, "rm -rf %s", tmpdir);
	system(cmd);

	if (dblocked)
	{
		// Done, close the database and release locks
		CloseDatabases((DefCommandData *)comm->data);
	}

	return -xleUndefined;
}

int
HandleDbExport(Command *comm, int argc, char **argv)
{
   	NetoidInfoEntry *netInfo;
   	NetoidSNKey *key, *okey;
	ClientAttribs *clAttribs;
   	VpnEntry *vpnEntry,*lastVpnEntry;
   	VpnGroupEntry *vpnGroupEntry,*lastVpnGroupEntry;
   	CallPlanKey *cpkey, *ocpkey;
   	CallPlanBindKey *cpbkey, *ocpbkey;
   	CallPlanEntry *callPlanEntry;
   	CallPlanBindEntry *callPlanBindEntry;
   	VpnRouteKey *rkey, *orkey;
   	VpnRouteEntry *routeEntry;
	TriggerKey *tgkey, *otgkey;
	TriggerEntry *triggerEntry;
    RealmKey  *rmKey, *ormKey;
    RealmEntry  *realmEntry;
    IgrpKey  	*grpKey, *ogrpKey;
    IgrpInfo	*igrp;
    VnetKey  *vnetKey, *ovnetKey;
    VnetEntry  *vnetEntry;
   	FILE *exportf;
   	time_t now;
   	char *vpnName, *vpnG;
	int exportAll = 1, exportIedge = 0, exportVpn = 0, exportVpng = 0, exportCp = 0;
	int exportCpb = 0, exportCr = 0, exportDcr = 0, exportTrigger = 0, exportIgrp = 0, exportRealm = 0;
	int exportVnet = 0;
	int i;

	// command syntax
	// cli db export [iedge|vpn|vpng|cp|cpb|cr|dcr|trigger|igrp|realm]+ <file>

   	if (argc == 0 || argc > 10)
   	{
	 	/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -1;
   	}

	if(argc > 1)
	{
		// export only the specified data
		exportAll = 0;
		for(i = 0; i < argc - 1; i++)
		{
			if(strcmp(argv[i], "iedge") == 0)
			{
				exportIedge = 1;
			}
			else if(strcmp(argv[i], "vpn") == 0)
			{
				exportVpn = 1;
			}
			else if(strcmp(argv[i], "vpng") == 0)
			{
				exportVpng = 1;
			}
			else if(strcmp(argv[i], "cp") == 0)
			{
				exportCp = 1;
			}
			else if(strcmp(argv[i], "cpb") == 0)
			{
				exportCpb = 1;
			}
			else if(strcmp(argv[i], "cr") == 0)
			{
				exportCr = 1;
			}
			else if(strcmp(argv[i], "dcr") == 0)
			{
				exportDcr = 1;
			}
			else if(strcmp(argv[i], "trigger") == 0)
			{
				exportTrigger = 1;
			}
			else if(strcmp(argv[i], "igrp") == 0)
			{
				exportIgrp =1;
			}
			else if(strcmp(argv[i], "realm") == 0)
			{
				exportRealm =1;
			}
			else if(strcmp(argv[i], "vnet") == 0)
			{
				exportVnet =1;
			}
		}
	}

   	/* Open the file */
   	if ((exportf = fopen(exportAll ? argv[0]: argv[argc - 1], "w")) == NULL)
   	{
		perror("fopen");

		/* let perror take care of the error */
		return 1;
   	}
      
	fprintf(exportf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
   	fprintf(exportf, "<!--\n");
   	fprintf(exportf, "#Nextone one format, (c) Nextone Communications 1998-2000\n");
   	fprintf(exportf, "#This file is generated\n");
   	time(&now);
   	fprintf(exportf, "#from \"%s\" on %s", exportAll ? argv[0] : argv[argc - 1], ctime(&now));
   	fprintf(exportf, "-->\n\n");
	fprintf(exportf, "<DB>\n");

	if(exportAll || exportIedge)
	{
		for (key = (NetoidSNKey *)DbGetFirstInfoKey(GDBMF(comm->data, 
						DB_eNetoids)); key != 0; 
				key = (NetoidSNKey *)DbGetNextInfoKey(GDBMF(comm->data, 
						DB_eNetoids),
					(char *)key, 
					sizeof(NetoidSNKey)),
				free(okey), free(netInfo))
		{
			netInfo = DbFindInfoEntry(GDBMF(comm->data, DB_eNetoids), 
					(char *)key, sizeof(NetoidSNKey));
			clAttribs = DbFindAttrEntry(GDBMF(comm->data, DB_eAttribs), 
					(char *)key, sizeof(NetoidSNKey));
			ExportInfoEntry(exportf, netInfo, clAttribs);
			if (clAttribs)
			{
				free(clAttribs);
			}
			okey = key;
		}
	}


	if(exportAll || exportVpn)
	{
		/* Export the vpns now in the file */
		for (vpnEntry = DbGetFirstVpnEntry(GDBMF(comm->data, DB_eVpns)); 
				vpnEntry != 0; 
				vpnEntry = DbGetNextVpnEntry(GDBMF(comm->data, DB_eVpns), 
					vpnName, sizeof(VpnKey)), 
				free(lastVpnEntry))
		{
			ExportVpnEntry(exportf, vpnEntry);

			vpnName = vpnEntry->vpnName;
			lastVpnEntry = vpnEntry;
		}
	}

	if(exportAll || exportVpng)
	{
		/* Export the vpng's now in the file */
		for (vpnGroupEntry = DbGetFirstVpnGEntry(GDBMF(comm->data, DB_eVpnG)); 
				vpnGroupEntry != 0; 
				vpnGroupEntry = DbGetNextVpnGEntry(GDBMF(comm->data, DB_eVpnG), 
					vpnG, sizeof(VpnGroupKey)), 
				free(lastVpnGroupEntry))
		{
			ExportVpnGEntry(exportf, vpnGroupEntry);

			vpnG = vpnGroupEntry->vpnGroup;
			lastVpnGroupEntry = vpnGroupEntry;
		}
	}

	if(exportAll || exportCp)
	{
		for (cpkey = (CallPlanKey *)DbGetFirstKey(GDBMF(comm->data, 
						DB_eCallPlan)); 
				cpkey != 0; 
				cpkey = (CallPlanKey *)DbGetNextKey(GDBMF(comm->data, 
						DB_eCallPlan),
					(char *)cpkey, 
					sizeof(CallPlanKey)),
				free(ocpkey), free(callPlanEntry))
		{
			callPlanEntry = (CallPlanEntry *)DbFindEntry(GDBMF(comm->data, DB_eCallPlan), 
					(char *)cpkey, sizeof(CallPlanKey));
			ExportCPEntry(exportf, callPlanEntry);
			ocpkey = cpkey;
		}
	}

	if(exportAll || exportCr)
	{
		for (rkey = (VpnRouteKey *)DbGetFirstKey(GDBMF(comm->data, 
						DB_eCallRoute)); 
				rkey != 0; 
				rkey = (VpnRouteKey *)DbGetNextKey(GDBMF(comm->data, 
						DB_eCallRoute),
					(char *)rkey, 
					sizeof(VpnRouteKey)),
				free(orkey), free(routeEntry))
		{
			routeEntry = (VpnRouteEntry *)DbFindEntry(GDBMF(comm->data, DB_eCallRoute), 
					(char *)rkey, sizeof(VpnRouteKey));
			if(exportAll || (routeEntry->trname[0] == '\0'))
			{
				ExportCREntry(exportf, routeEntry);
			}
			orkey = rkey;
		}
	}

	if(!exportAll && exportDcr)
	{
		for (rkey = (VpnRouteKey *)DbGetFirstKey(GDBMF(comm->data, 
						DB_eCallRoute)); 
				rkey != 0; 
				rkey = (VpnRouteKey *)DbGetNextKey(GDBMF(comm->data, 
						DB_eCallRoute),
					(char *)rkey, 
					sizeof(VpnRouteKey)),
				free(orkey), free(routeEntry))
		{
			routeEntry = (VpnRouteEntry *)DbFindEntry(GDBMF(comm->data, DB_eCallRoute), 
					(char *)rkey, sizeof(VpnRouteKey));
			if(routeEntry->trname[0] != '\0')
			{
				ExportCREntry(exportf, routeEntry);
			}
			orkey = rkey;
		}
	}

	if(exportAll || exportCpb)
	{
		for (cpbkey = (CallPlanBindKey *)DbGetFirstKey(GDBMF(comm->data, 
						DB_eCallPlanBind)); 
				cpbkey != 0; 
				cpbkey = (CallPlanBindKey *)DbGetNextKey(GDBMF(comm->data, 
						DB_eCallPlanBind),
					(char *)cpbkey, 
					sizeof(CallPlanBindKey)),
				free(ocpbkey), free(callPlanBindEntry))
		{
			callPlanBindEntry = (CallPlanBindEntry *)DbFindEntry(GDBMF(comm->data, DB_eCallPlanBind), 
					(char *)cpbkey, sizeof(CallPlanBindKey));
			ExportCPBEntry(exportf, callPlanBindEntry);
			ocpbkey = cpbkey;
		}
	}

	if(exportAll || exportTrigger)
	{
		for (tgkey = (TriggerKey *)DbGetFirstKey(GDBMF(comm->data, 
						DB_eTrigger)); 
				tgkey != 0; 
				tgkey = (TriggerKey *)DbGetNextKey(GDBMF(comm->data, 
						DB_eTrigger),
					(char *)tgkey, 
					sizeof(TriggerKey)),
				free(otgkey), free(triggerEntry))
		{
			triggerEntry = (TriggerEntry *)DbFindEntry(GDBMF(comm->data, DB_eTrigger), 
					(char *)tgkey, sizeof(TriggerKey));
			ExportTriggerEntry(exportf, triggerEntry);
			otgkey = tgkey;
		}
	}

	if(exportAll || exportRealm)
	{
		for (rmKey = (RealmKey *)DbGetFirstKey(GDBMF(comm->data, DB_eRealm)); 
			rmKey != 0; 
			rmKey = 
			(RealmKey *)DbGetNextKey(GDBMF(comm->data, DB_eRealm), (char *)rmKey, sizeof(RealmKey)),
				free(ormKey), free(realmEntry))
		{
			realmEntry = (RealmEntry *)DbFindEntry(GDBMF(comm->data, DB_eRealm), (char *)rmKey, sizeof(RealmKey));
			ExportRealmEntry(exportf, realmEntry);
			ormKey = rmKey;
		}
	}

	if(exportAll || exportIgrp)
	{
		for (grpKey = (IgrpKey *)DbGetFirstKey(GDBMF(comm->data, DB_eIgrp)); 
				grpKey != 0; 
				grpKey = 
				(IgrpKey *)DbGetNextKey(GDBMF(comm->data, DB_eIgrp), (char *)grpKey, sizeof(IgrpKey)),
				free(ogrpKey), free(igrp))
		{
			igrp = (IgrpInfo *)DbFindEntry(GDBMF(comm->data, DB_eIgrp), (char *)grpKey, sizeof(IgrpKey));
			ExportIgrpEntry(exportf, igrp);
			ogrpKey = grpKey;
		}
	}

	if(exportAll || exportVnet)
	{
		for (vnetKey = (VnetKey *)DbGetFirstKey(GDBMF(comm->data, DB_eVnet)); 
				vnetKey != 0; 
				vnetKey = 
				(VnetKey *)DbGetNextKey(GDBMF(comm->data, DB_eVnet), (char *)vnetKey, sizeof(VnetKey)),
				free(ovnetKey), free(vnetEntry))
		{
			vnetEntry = (VnetEntry *)DbFindEntry(GDBMF(comm->data, DB_eVnet), (char *)vnetKey, sizeof(VnetKey));
			ExportVnetEntry(exportf, vnetEntry);
			ovnetKey = vnetKey;
		}
	}

	fprintf(exportf, "</DB>\n");

    fclose(exportf);

    return 0;
}

int
HandleDbSave(Command *comm, int argc, char **argv)
{
	char cmd[256];
	char *dest;
    int k;

   	if (argc != 1)
   	{
	 	/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -1;
   	}

	dest = argv[0];

    for (k = 0; k < DB_eMax; k ++)
    {
		sprintf(cmd, "cp -p %s %s", DBNAME(comm->data, k), dest);
		system(cmd);
	}

    return 0;
}

int
HandleDbRepl(Command *comm, int argc, char **argv)
{
     	if (argc != 1)
     	{
	 	/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
		HandleCommandUsage(comm, argc, argv);
	 	return -1;
     	}
	return(0);
}

int
HandleDbStale(Command *comm, int argc, char **argv)
{
	DbUpdate();
	return(0);
}

int
HandleDbClean(Command *comm, int argc, char **argv)
{
	int all = 0, nocache = 0;
	char cmd[256];
    DB_tDb dbstruct;
    DB db;

    if (argc != 1)
    {
		  /* Here we prompt the user for the rest of the 
		   * information
		   */
		  HandleCommandUsage(comm, argc, argv);
		  return -1;
    }

	//all = !strcmp(argv[0], "all");

	if (all)
	{
		// Cleanup the cache also
		HandleCacheClean(comm, 0, 0);
	}
	else if (CacheAttach() < 0)
	{
		LsMemStructReset();
		nocache = 1;
	}

	all = !strcmp(argv[0], "all");

	if (all || !strcmp(argv[0], "iedge"))
	{
		if (!nocache)
		{
			// Purge the iedge cache
			CachePurge("iedge");
		}

     	dbstruct.read_write = GDBM_NEWDB;

     	if (db = DbOpenByID(DBNAME(comm->data, DB_eNetoids), 
			DB_eNetoids, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0; 
     	}

     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eAttribs), 
				DB_eAttribs, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "vpn"))
	{
		if (!nocache)
		{
			// Purge the iedge cache
			CachePurge("vpn");
		}

     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eVpns), 
				DB_eVpns, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "vpng"))
	{
		if (!nocache)
		{
			// Purge the iedge cache
			CachePurge("vpng");
		}

     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eVpnG), 
				DB_eVpnG, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "fax"))
	{
     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eFax), 
				DB_eFax, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "cp"))
	{
     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eCallPlan), 
				DB_eCallPlan, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}

		if (!nocache)
		{
			// Purge the iedge cache
			CachePurge("cp");
		}

     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eCallRoute), 
				DB_eCallRoute, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}

		if (!nocache)
		{
			// Purge the iedge cache
			CachePurge("cpb");
		}

     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eCallPlanBind), 
				DB_eCallPlanBind, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "trigger"))
	{
     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eTrigger), 
				DB_eTrigger, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "realm"))
	{
		if (!nocache)
		{
			CachePurge("realm");
		}
     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eRealm), DB_eRealm, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "igrp"))
	{
		if (!nocache)
		{
			CachePurge("igrp");
		}
     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eIgrp), DB_eIgrp, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all || !strcmp(argv[0], "vnet"))
	{
		if (!nocache)
		{
			CachePurge("vnet");
		}
     	dbstruct.read_write = GDBM_NEWDB;
     	if (db = DbOpenByID(DBNAME(comm->data, DB_eVnet), DB_eVnet, &dbstruct))
     	{
     		DbClose(&dbstruct);
     		db = 0;
     	}
	}

	if (all)
	{
		DbDeleteLocks();
	}

	if (!nocache)
	{
		CacheDetach();
	}

	return xleOk;
}

int
HandleDbRev( Command *comm, int argc, char **argv )
{
    int i = comm->subCommsLen/sizeof(Command), j = 0;
    int rc = 1;
    char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

    /* Now we look at the subcommands */
    if (argc < 1)
    {
       	j = i*!cli_ix;
	/* There is nothing more to execute, we are done ! */
	goto _return;
    }

    for (j=0; j<i; j++)
    {
        if (strcmp(comm->subComms[j].name, argv[0]) == 0)
	{
	       /* pass the open file to the subcommands */
	       comm->subComms[j].data = comm->data;
	       rc = comm->subComms[j].commFn(
			  &(comm->subComms[j]), --argc, ++argv);
		   break;
	}
    }

_return:

    /* Handle the case when the command was not found */
    if (i==j)
    {
       	if (cliLibFlags == 0)
       	   return -xleInvalArgs;
       	else
       	   PrintUsage(comm, argc, argv, comm->flags);
    }

    return rc;

_error:
    return -1;
}      

int
HandleDbRevShow( Command *comm, int argc, char **argv )
{
	int	dbrev;

	dbrev = ReadDBRevNum();
	fprintf(stdout, "Current database revision number is %d\n", dbrev);

	return xleOk;
}      

int
HandleDbRevIncr( Command *comm, int argc, char **argv )
{
	int	dbrev;
	char fn[] = "HandleDBRevIncr";

	dbrev = GetNextDBRevNum();

	if (dbrev == -1) {
		NETERROR(MCLI, ("%s: Failed\n", fn));
		fprintf(stdout, "%s: Failed\n", fn);
		return -xleUndefined;
	}
	else {
		fprintf(stdout, "Database revision number incremented to %d\n", dbrev);
	}

	return xleOk;
}      

int
HandleDbRevMod( Command *comm, int argc, char **argv )
{
	int	dbrev;
	int	newdbrev;
	int	olddbrev;
	char fn[] = "HandleDbRevMod";

    if (argc != 1)
    {
		/* Here we prompt the user for the rest of the 
	 	* information
	 	*/
	 	HandleCommandUsage(comm, argc, argv);
	 	return -xleInsuffArgs;
    }

	errno = 0;
	if (((newdbrev = atoi(argv[0])) == 0) && (errno != 0)) {
		NETERROR(MCLI, ("%s: argument  - %s not a valid integer\n", fn, argv[0]));	
		return -xleInvalArgs;
	}

	olddbrev = ReadDBRevNum();
	dbrev = ModDBRevNum(newdbrev);
	fprintf(stdout, "Database revision number modified from %d to %d\n", 
		olddbrev, dbrev);

	return xleOk;
}
