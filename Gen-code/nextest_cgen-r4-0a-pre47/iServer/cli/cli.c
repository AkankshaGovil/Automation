/*
 * cli.c:
 *
 * database management from command line
 */

#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>
#include "cli.h"
#include "shm.h"
#include "shmapp.h"
#include "serverp.h"
#include "licenseIf.h"
#include "log.h"
#include "cacheinit.h"
#include "common.h"
#include "callutils.h"

int (*_lsAlarmFn)(time_t* valarms, time_t* mrvalarms) = NULL;
int (*_CliRouteLogFn)(RouteNode *routeNode) = NULL;

/* Default cliLibFlags.
 * Use these to control debug output
 */
short cliLibFlags = 0;

char *progName;
int cli_debug = 0;
int send2Rs = 1;
int saveindb = 1;
int processCmdOnSlave = 1;
char clifiledir[80] = { 0 };
int cli_ix = 0;	// Interactive version

LsMemStruct *lsMem = 0;
MemoryMap *map = 0;
static char totalstr[] = "Total";
static char usedstr[] = "Used";
static char availstr[] = "Available";
static char epstr[] = "EndPoints";
static char callstr[] = "VPORTS";
static char warnstr[]  = "LICENSE USAGE IS ";
static char warnstr2[] = ". YOU MIGHT NEED TO OBTAIN MORE LICENSES SOON.\n";
static char expmsg[] = "License Expiry Date";
static char macmsg[] = "License Node locked to"; 
static char mrstr[] = "Media Routed"; 

Command netoidSubCommands[] = 
{
     { "add", HandleNetoidAdd, NetoidAddUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "vpns", HandleNetoidVpns, NetoidVpnsUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "phones", HandleNetoidPhones, NetoidPhonesUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "email", HandleNetoidEmail, NetoidEmailUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "zone", HandleNetoidZone, NetoidZoneUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "delete", HandleNetoidDelete, NetoidDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "find", HandleNetoidFind, NetoidFindUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "lkup", HandleNetoidLkup, NetoidLkupUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "list", HandleNetoidList, NetoidListUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "cache", HandleNetoidCache, NetoidCacheUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
     { "edit", HandleNetoidEdit, NetoidEditUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "route", HandleNetoidRoute, NetoidRouteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "reg", HandleNetoidReg, NetoidRegUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "hunt", HandleNetoidHunt, NetoidHuntUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
}; 

Command vpnSubCommands[] = 
{
     { "add", HandleVpnAdd, VpnAddUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "vpng", HandleVpnVpnG, VpnVpnGUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "delete", HandleVpnDelete, VpnDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "list", HandleVpnList, VpnListUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "cache", HandleVpnCache, VpnCacheUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
     { "edit", HandleVpnEdit, VpnEditUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
};

Command vpnGSubCommands[] = 
{
     { "add", HandleVpnGAdd, VpnGAddUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "delete", HandleVpnGDelete, VpnGDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "list", HandleVpnGList, VpnGListUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "cache", HandleVpnGCache, VpnGCacheUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
     { "edit", HandleVpnGEdit, VpnGEditUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
};

Command dbrevSubCommands[] = 
{
     { "show", HandleDbRevShow, DbRevShowUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 3, COMMANDF_USER },
     { "incr", HandleDbRevIncr, DbRevIncrUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 3, COMMANDF_USER },
     { "mod", HandleDbRevMod, DbRevModUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 3, COMMANDF_USER },
};

Command dbSubCommands[] = 
{
     { "init", HandleDbInit, DbInitUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "info", HandleDbInfo, DbInfoUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "org", HandleDbOrg, DbOrgUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "export", HandleDbExport, DbExportUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "save", HandleDbSave, DbSaveUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "create", HandleDbCreate, DbCreateUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "copy", HandleDbCopy, DbCopyUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "stale", HandleDbStale, DbStaleUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
     { "add", HandleDbAdd, DbAddUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "replace", HandleDbReplace, DbReplaceUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "switch", HandleDbSwitch, DbSwitchUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "delete", HandleDbDelete, DbDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "clean", HandleDbClean, DbCleanUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "hist", HandleDbHist, DbHistUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "rev", HandleDbRev, 0, dbrevSubCommands, sizeof(dbrevSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
};

Command faxsSubCommands[] = 
{
     { "add", HandleFaxsAdd, FaxsAddUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "delete", HandleFaxsDelete, FaxsDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "lkup", HandleFaxsLkup, FaxsLkupUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "list", HandleFaxsList, FaxsListUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
}; 

Command cpSubCommands[] = 
{
     { "add", HandleCPAdd, CPAddUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "delete", HandleCPDelete, CPDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "list", HandleCPList, CPListUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "edit", HandleCPEdit, CPEditUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "cache", HandleCPCache, CPCacheUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
}; 

Command crSubCommands[] = 
{
     { "add", HandleCRAdd, CRAddUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "delete", HandleCRDelete, CRDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "edit", HandleCREdit, CREditUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
     { "list", HandleCRList, CRListUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "lkup", HandleCRLkup, CRLkupUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
     { "cache", HandleCRCache, CRCacheUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
}; 

Command cacheSubCommands[] = 
{
     { "create", HandleCacheCreate, CacheCreateUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
     { "clean", HandleCacheClean, CacheCleanUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
};

Command callSubCommands[] = 
{
     { "cache", HandleCallCache, CallCacheUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "lkup", HandleCallLkup, CallLkupUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
     { "delete", HandleCallDelete, CallDeleteUsage, NULL, 0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
}; 

Command gkSubCommands[] = 
{
     { "reg", HandleGkReg, GkRegUsage, NULL, 
	   0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
};

Command alarmSubCommnds[] = 
{
     { "clear", HandleAlarmClear, 0, 0, 
	   0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER }
	   
};

Command rsdSubCommands[] =
{
  { "list", HandleRsdList, RsdListUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
  { "clear", HandleRsdClear, RsdClearUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
  { "add", HandleRsdAdd, RsdAddUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
  { "delete", HandleRsdDelete, RsdDeleteUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
};

Command triggerSubCommands[] =
{
  { "list", HandleTriggerList, TriggerListUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
  { "cache", HandleTriggerCache, TriggerListUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
  { "add", HandleTriggerAdd, TriggerAddUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "delete", HandleTriggerDelete, TriggerDeleteUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "edit", HandleTriggerEdit, TriggerEditUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "purge", HandleTriggerPurge, TriggerPurgeUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
};

Command realmSubCommands[] =
{
  { "list", HandleRealmList, RealmListUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
  { "add", HandleRealmAdd, RealmAddUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "delete", HandleRealmDelete, RealmDeleteUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "edit", HandleRealmEdit, RealmEditUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "cache", HandleRealmCache, RealmCacheUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
  { "lkup", HandleRealmLkup, RealmLkupUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN },
  { "up", HandleRealmUp, RealmUpUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE},
  { "down", HandleRealmDown, RealmDownUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE},
  { "open", HandleRealmOpen, RealmOpenUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN},
  { "close", HandleRealmClose, RealmCloseUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_HIDDEN}
};

Command igrpSubCommands[] =
{
  { "list", HandleIgrpList, IgrpListUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
  { "add", HandleIgrpAdd, IgrpAddUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "delete", HandleIgrpDelete, IgrpDeleteUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "edit", HandleIgrpEdit, IgrpEditUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "cache", HandleIgrpCache, IgrpCacheUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_HIDDEN },
  { "lkup", HandleIgrpLkup, IgrpLkupUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_HIDDEN }
};

Command vnetSubCommands[] =
{
  { "list", HandleVnetList, VnetListUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER },
  { "add", HandleVnetAdd, VnetAddUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "delete", HandleVnetDelete, VnetDeleteUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "edit", HandleVnetEdit, VnetEditUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_STATE },
  { "cache", HandleVnetCache, VnetCacheUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_HIDDEN },
  { "lkup", HandleVnetLkup, VnetLkupUsage, NULL,
    0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 2, COMMANDF_USER|COMMANDF_HIDDEN }
};

Command mainCommands[] = 
{
     { "iedge", HandleNetoid, 0, netoidSubCommands, 
       sizeof(netoidSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "vpn", HandleVpn, 0, vpnSubCommands, 
       sizeof(vpnSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "vpng", HandleVpnG, 0, vpnGSubCommands, 
       sizeof(vpnGSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "db", HandleDb, 0, dbSubCommands, 
	sizeof(dbSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "test", HandleTest, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_HIDDEN },
     { "lsalarm", HandlelsAlarm, 0, alarmSubCommnds,
       sizeof(alarmSubCommnds), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER
     },
     { "lstat", Handlelstat, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "show", HandleShow, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_HIDDEN },
     { "clean", HandleClean, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_HIDDEN },
     { "faxs", HandleFaxs, 0, faxsSubCommands, 
       sizeof(faxsSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_HIDDEN },
     { "cp", HandleCP, 0, cpSubCommands, 
	sizeof(cpSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "cr", HandleCR, 0, crSubCommands, 
	sizeof(crSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "gk", HandleGk, 0, gkSubCommands, 
	sizeof(gkSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_HIDDEN },
     { "stats", Handlestats, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "lupdate", Handlelupdate, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_HIDDEN },
     { "cache", HandleCache, 0, cacheSubCommands, 
	sizeof(cacheSubCommands), &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "ignore", HandleIgnore, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_HIDDEN },
     { "rsd", HandleRsd, NULL, rsdSubCommands,
       sizeof(rsdSubCommands),  &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "trigger", HandleTrigger, NULL, triggerSubCommands,
       sizeof(triggerSubCommands),  &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "realm", HandleRealm, NULL, realmSubCommands,
       sizeof(realmSubCommands),  &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "igrp", HandleIgrp, NULL, igrpSubCommands,
       sizeof(igrpSubCommands),  &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "call", HandleCall, NULL, callSubCommands,
       sizeof(callSubCommands),  &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "scm", HandleSCM, 0, 0, 
	0, &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER },
     { "vnet", HandleVnet, NULL, vnetSubCommands,
       sizeof(vnetSubCommands),  &defCommandData[0], DB_eMax*sizeof(DefCommandData), 1, COMMANDF_USER }
};

#define NUM_OF_COMMS  sizeof(mainCommands)/sizeof(Command)

/* A note about printing usage. Usage is normally
 * printed for only that part of teh command 
 * which was wrongly entered
 */
void
PrintCliUsage(char *progName, int argc, char **argv, short mflags)
{  
    int i = NUM_OF_COMMS;
    int j;
     
    mflags &= cliLibFlags;

	if (!(mflags & COMMANDF_USER))
    {
		return;
    }

    fprintf(stdout, "Usage: \n%s ", progName);

    for (j = 0; j < i; j++)
    {
	  	if (!(mainCommands[j].flags & mflags))
	  	{
			continue;
	  	}

		format(stdout, 1);

	  	if (mainCommands[j].usageFn) 
	  	{
	       	mainCommands[j].usageFn(&mainCommands[j], argc, argv);
	  	}
	  	else
	  	{
	       	PrintUsage(&mainCommands[j], argc, argv, mflags);
	  	}
    }
     
    fprintf(stdout, "\n");
}

void
PrintUsage(Command *comm, int argc, char **argv, short mflags)
{
     int i = comm->subCommsLen/sizeof(Command);
     int j;
     
     /* This has been called because no usage function
      * has been specified for this command.
      * By default, print its name, and then loop thru 
      * its subcommands and call the usage functions of each
      */

     mflags &= cliLibFlags;

     if (!(comm->flags & mflags))
     {
	return;
     }
     if (!comm->usageFn)
     {
# if 0
	  format(stdout, comm->level);
#endif
	  fprintf(stdout, "%s ", comm->name);
     }
     else 
     {
	  return (comm->usageFn(comm, argc, argv));
     }

     /* The first sub-command is printed on the same line,
      * and is a special case.
      * the rest have to be pre-fixed with level no of tabs.
      */
     if (comm->subComms)
     {
     	format(stdout, 1); 
     	PrintUsage(&(comm->subComms[0]), argc, argv, mflags);
     	fprintf(stdout, "\n");

     	for (j = 1; j < i; j++)
     	{
		  Command *subComm = &(comm->subComms[j]);
	
		  format(stdout, subComm->level);
		  
		  PrintUsage(subComm, --argc, ++argv, mflags);
		  fprintf(stdout, "\n");
     	}
     }
     else
     {
	fprintf(stdout, "\n");
     }
}
	  
int
CliMain( int argc, char **argv )
{
     sigset_t o_signal_mask, n_signal_mask;
	 int rc;
	int shmId;

     FillAllSignals(&n_signal_mask);
     sigdelset(&n_signal_mask, SIGINT);
     sigdelset(&n_signal_mask, SIGQUIT);
     sigdelset(&n_signal_mask, SIGABRT);

     //BlockAllSignals(&n_signal_mask, &o_signal_mask);
	if ((shmId  = CacheAttach()) == -1)
	{
		// No cache
		LsMemStructReset();
	}
	else
	{
		 InitCfgFromCfgParms(lsMem->cfgParms);
	}

     rc = ProcessCommand(argc, argv);

	if (shmId != -1)
	{
		CacheDetach();
	}

     //UnblockAllSignals(&o_signal_mask, 0);
     if (rc > 0)
     {
         rc = 0;
     }

     return rc;
}

int
HandleShow( Command *comm, int argc, char **argv )
{
	PrintCliUsage(progName, argc, argv, COMMANDF_ALL);
	return(1);
}

int 
ProcessCommand(int argc, char **argv)
{
    int i = NUM_OF_COMMS;
    int j, l, k;
	int rc = -xleInvalArgs;	
	char *argvtmp[64] = { 0 }, buffer[256];
	unsigned int ipAddr, port;
	int status;
	char msg[RS_MSGLEN];

_handle_command:

	rc = -xleInvalArgs;	

	if (argc < 1)
	{
		j = i*!cli_ix;
		processCmdOnSlave = 1;
		goto _return;
	}

	// Initialize the random num gen
	srand48((long)time(0));

    /* Open log */
    NetLogOpen(NULL, 0, NETLOG_ASYNC);

	/* if subcommand is to be processed only on the slave .. */
	if ((argc > 1) && (argv[1] != NULL))
	{
		for (j=0; j < NUM_OF_COMMS; j++)
		{
			if (strcmp(mainCommands[j].name, argv[0]) == 0)
	  		{
    			k = mainCommands[j].subCommsLen/sizeof(Command);
				for (l=0; l<k; l++)
				{
					if (strcmp(mainCommands[j].subComms[l].name, argv[1]) == 0)
					{
		   				if ((mainCommands[j].subComms[l].flags) & COMMANDF_STATE)
			 				processCmdOnSlave = 0;
		   				else
			 				processCmdOnSlave = 1;
					}
				}
			}
		}
	}
			
	/* Check if redundant configuration and if slave. */
	if (IsSlave (&ipAddr, &port) && (!processCmdOnSlave))
	{
		/* If Slave, send the cli command to Master. */
		rc = CliSendToDBMaster(ipAddr,port, argc, argv, msg);

		if (rc == xleOk)
			CLIPRINTF((stdout, "Cli Command Successful on DBMaster.\n"));
		else
			CLIPRINTF((stdout, "Cli Command Failed.\n"));

		CLIPRINTF((stdout, "%s\n", msg));

		processCmdOnSlave = 0;
		
	}
	else
	{
		
    	for (j = 0; j < NUM_OF_COMMS; j++)
    	{
			if (strcmp(mainCommands[j].name, argv[0]) == 0)
			{
				rc = ( mainCommands[j].commFn(&mainCommands[j],
						--argc, ++argv) );
				break;
			}
		}
	}
     
    NetLogClose();

_return:
		if ((i==j) && processCmdOnSlave)
    	{
			if (cliLibFlags == 0)
			   return -xleInvalArgs;
			else
	 			PrintCliUsage(progName, argc, argv, COMMANDF_USER);
   		 }

		if (CliHandleInteractive("cli>", buffer, 256, &argc, argvtmp, 64))
		{
			argv = argvtmp;
			goto _handle_command;
		}
    return rc;
}

int
HandleNetoid( Command *comm, int argc, char **argv )
{
	/* First try to obtain an exclusive lock on the db file */
    int i = comm->subCommsLen/sizeof(Command), j = 0;
	int rc = -xleInsuffArgs;
	int shmId;
	int revno = -1, argcOrig;
	char **argvOrig; 
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	rc = -xleInsuffArgs;
	revno = -1;

    /* Now we look at the subcommands */
    if (argc < 1)
    {
		j = i*!cli_ix;
		/* There is nothing more to execute, we are done ! */
		goto _return;
    }

#if 0
	if ((shmId  = CacheAttach()) == -1)
	{
		// No cache
		LsMemStructReset();
	}
	else
	{
		 InitCfgFromCfgParms(lsMem->cfgParms);
	}
#endif

	argcOrig = argc;
	argvOrig = argv;

    for (j=0; j<i; j++)
    {
		if (strcmp(comm->subComms[j].name, argv[0]) == 0)
	  	{
	   		/* pass the open file to the subcommands */
	   	   	comm->subComms[j].data = comm->data;

			if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

	   	    rc = comm->subComms[j].commFn(
				&(comm->subComms[j]), --argc, ++argv);
	
			break;
		 }
   	 }

#if 0
	if (shmId != -1)
	{
		CacheDetach();
	}
#endif

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "iedge", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
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

	if (CliHandleInteractive("cli iedge>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

	return rc;

_error:
    return -1;
}

int
HandleGk( Command *comm, int argc, char **argv )
{
	 char fn[] = "HandleGk():";
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	 rc = 1;

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

	if (CliHandleInteractive("cli gk>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;

_error:
     return -1;
     
}

int
HandleCall( Command *comm, int argc, char **argv )
{
	 char fn[] = "HandleGkCall():";
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	 rc = 1;

     /* Now we look at the subcommands */

     if (argc < 1)
     {
	  /* There is nothing more to execute, we are done ! */
		j = i*!cli_ix;
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

	if (CliHandleInteractive("cli call>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;

_error:
     return -1;
     
}

int
HandleFaxs( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;

     /* Now we look at the subcommands */

     if (argc < 1)
     {
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
     if (j == i)
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

// if a db command changes the db,
// its always a file input
int
HandleDb( Command *comm, int argc, char **argv )
{
    int rc = 1;	
    DefCommandData *dbDefaults = (DefCommandData *)comm->data;
    DB gdbmf;
    int i = comm->subCommsLen/sizeof(Command), j, k;
	int openDb = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

    dbDefaults = (DefCommandData *)comm->data;
	openDb = 1;
	revno = -1;

    /* First try to obtain an exclusive lock on the db file */
	if ( ( (argc > 0) && (
		(strcmp(argv[0], "create") == 0) ||
		(strcmp(argv[0], "copy") == 0) ||
		(strcmp(argv[0], "replace") == 0) ||
		(strcmp(argv[0], "delete") == 0) ||
		(strcmp(argv[0], "rev") == 0) ||
		(strcmp(argv[0], "hist") == 0) ||
		(strcmp(argv[0], "add") == 0) ||
		(strcmp(argv[0], "clean") == 0) ||
		(strcmp(argv[0], "switch") == 0) ) ) ||
		(argc == 0) )
    {
        openDb = 0;
    }

    for (k = 0; k < openDb*DB_eMax; k ++)
    {
		DBSTRUCT(dbDefaults,k).read_write = GDBM_WRCREAT;
     	gdbmf = DbOpenByID(DBNAME(dbDefaults, k), k, &DBSTRUCT(dbDefaults,k));

    	if (gdbmf == NULL)
     	{
		  CLIPRINTF((stdout, "Database Could not be opened, Quitting!"));
		  goto _error;
     	}

		GDBMF(dbDefaults, k) = gdbmf;
	  
    }

    /* Now we look at the subcommands */

    if (argc < 1)
    {
		/* There is nothing more to execute, we are done ! */
		j = i*!cli_ix;
	  	goto _return;
    }

	argcOrig = argc;
	argvOrig = argv;

    for (j=0; j<i; j++)
    {
	  	if (strcmp(comm->subComms[j].name, argv[0]) == 0)
	  	{
			/* pass the open file to the subcommands */
			comm->subComms[j].data = comm->data;
			rc = comm->subComms[j].commFn(
		    &(comm->subComms[j]), --argc, ++argv);

		   	if ((comm->subComms[j].flags & COMMANDF_STATE) && (rc == xleOk))
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

			break;
	  	}
    }

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "db", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

_return:

    for (k = 0; k < openDb*DB_eMax; k ++)
    {
     	DbClose(&DBSTRUCT(dbDefaults,k));
	  
    }

    /* Handle the case when the command was not found */
	if (i==j)
    {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
	  		PrintUsage(comm, argc, argv, comm->flags);
    }

	if (CliHandleInteractive("cli db>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

    return rc;

_error:
    return -1;
}

int
HandleVpn( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     DefCommandData *dbDefaults = (DefCommandData *)comm->data;
     char *fname = (char *)comm->data;
     DB gdbmf;
     int i = comm->subCommsLen/sizeof(Command), j = 0, k;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

     dbDefaults = (DefCommandData *)comm->data;
	 revno = -1;

    /* Now we look at the subcommands */

	argcOrig = argc;
	argvOrig = argv;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

	       	/* pass the open file to the subcommands */
	       	comm->subComms[j].data = comm->data;
	       	rc = comm->subComms[j].commFn(
			&(comm->subComms[j]), --argc, ++argv);
	       	break;
	  	}
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "vpn", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

   	/* Handle the case when the command was not found */
	if (i==j)
   	{
		if (cliLibFlags == 0)
			return -xleInvalArgs;
		else
		   	PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli vpn>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

    return rc;

_error:
     return -1;
     
}

int
HandleVpnG( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     DefCommandData *dbDefaults = (DefCommandData *)comm->data;
     char *fname = (char *)comm->data;
     DB gdbmf;
     int i = comm->subCommsLen/sizeof(Command), j = 0, k;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

     dbDefaults = (DefCommandData *)comm->data;
	 revno = -1;

	argcOrig = argc;
	argvOrig = argv;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

	       /* pass the open file to the subcommands */
	       comm->subComms[j].data = comm->data;
	       rc = comm->subComms[j].commFn(
			  &(comm->subComms[j]), --argc, ++argv);
	       break;
	  }
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "vpng", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

     /* Handle the case when the command was not found */
	if (i==j)
     {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
		   PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli vpng>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;

_error:
     return -1;
     
}

int
HandleCP( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	argcOrig = argc;
	argvOrig = argv;

     /* Now we look at the subcommands */
	revno = -1;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

			/* pass the open file to the subcommands */
			comm->subComms[j].data = comm->data;
			rc = comm->subComms[j].commFn(
			&(comm->subComms[j]), --argc, ++argv);
			break;
	  	}
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "cp", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

     /* Handle the case when the command was not found */
	if (i==j)
     {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
		   PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli cp>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;

_error:
     return -1;
}

int
HandleCR( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	argcOrig = argc;
	argvOrig = argv;

	 revno = -1;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

			/* pass the open file to the subcommands */
			comm->subComms[j].data = comm->data;
			rc = comm->subComms[j].commFn(
				&(comm->subComms[j]), --argc, ++argv);
			break;
		}
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "cr", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

     /* Handle the case when the command was not found */
	if (i==j)
     {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
		   PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli cr>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;
	
_error:
     return -1;
}

int
HandleClean( Command *comm, int argc, char **argv )
{
	MemoryMap *map, *mapl;
	void *alShmStart = (void *)ISERVER_SHM_STARTADDR;
	int totalSize = 0x100000;
	int nsegs, npages;
	int key;
	char cmd[256];

	if (argc < 1)
	{
		fprintf(stdout, "%s <cache | db <iedge|vpn|vpng|all> >\n", comm->name);
		return -1;
	}

	key = ISERVER_SHM_KEY;

	if (strcmp(argv[0], "cache") == 0)
	{
		HandleCacheClean (comm, argc, argv);
#if     0
		DbDeleteLocks();

		IpcEnd();

		/* Cleanup the cache */
		map = SHMAPP_UseMap(key, &alShmStart, totalSize, 
			&nsegs, &npages);
		if (map == NULL)
		{
			goto _continue1;
		}
		
		mapl = (MemoryMap *)malloc(map->len);
		memcpy(mapl, map, map->len);
		/*
		  mapl->segs = ( SegMap * )(mapl+1); */
		SHMAPP_ReleaseMap(mapl);
		free(mapl);
#endif
	}	
_continue1:

	if (!strcmp(argv[0], "db") && (argc>1))
	{
		int all = 1;/* !strcmp(argv[1], "all"); */

		CLIPRINTF((stdout, "command deprecated. use cli db, cli cache versions\n"));
		return -1;

		DbDeleteLocks();

		if (all || !strcmp(argv[1], "iedge"))
		{
			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eNetoids));
			
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eNetoids));
			system(cmd);

			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eAttribs));
			
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eAttribs));
			system(cmd);
		}
		if (all || !strcmp(argv[1], "vpn"))
		{
			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eVpns));
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eVpns));
			system(cmd);
		}
		if (all || !strcmp(argv[1], "vpng"))
		{
			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eVpnG));
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eVpnG));
			system(cmd);
		}
		if (all || !strcmp(argv[1], "fax"))
		{
			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eFax));
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eFax));
			system(cmd);
		}
		if (all || !strcmp(argv[1], "cp"))
		{
			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eCallPlan));
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eCallPlan));
			system(cmd);

			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eCallRoute));
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eCallRoute));
			system(cmd);

			sprintf(cmd, "rm -f %s", 
				DBNAME(comm->data, DB_eCallPlanBind));
			CLIPRINTF((stdout, "%s\n", cmd));
			system(cmd);
			sprintf(cmd, "rm -f %s",
				DBLOCKNAME(comm->data, DB_eCallPlanBind));
			system(cmd);
		}
	}

_continue2:
	return 1;
}

int
HandlePassiveTest( Command *comm, int argc, char **argv )
{
	if (CacheAttach() == -1)
	{
		CLIPRINTF((stdout, "No Cache\n"));
		return 0;
	}

	CLIPRINTF((stdout, "Last process to hold reg locks was %d\n", regCache->lock->pid));
	NETERROR(MCLI, ("Last process to hold reg locks was %d\n", regCache->lock->pid));
	CLIPRINTF((stdout, "regCache has %d items.\n", 
			regCache->nitems));
	NETERROR(MCLI, ("regCache has %d items.\n", 
			regCache->nitems));
	CLIPRINTF((stdout, "regidCache has %d items.\n", 
			regidCache->nitems));
	NETERROR(MCLI, ("regidCache has %d items.\n", 
			regidCache->nitems));
	CLIPRINTF((stdout, "phoneCache has %d items.\n", 
			phoneCache->nitems));
	NETERROR(MCLI, ("phoneCache has %d items.\n", 
			phoneCache->nitems));
	CLIPRINTF((stdout, "gwCache has %d items.\n", 
			gwCache->nitems));
	NETERROR(MCLI, ("gwCache has %d items.\n", 
			gwCache->nitems));
	CLIPRINTF((stdout, "ipCache has %d items.\n", 
			ipCache->nitems));
	NETERROR(MCLI, ("ipCache has %d items.\n", 
			ipCache->nitems));
	CLIPRINTF((stdout, "gkCache has %d items.\n", 
			gkCache->nitems));
	NETERROR(MCLI, ("gkCache has %d items.\n", 
			gkCache->nitems));
	CLIPRINTF((stdout, "h323idCache has %d items.\n", 
			h323idCache->nitems));
	NETERROR(MCLI, ("h323idCache has %d items.\n", 
			h323idCache->nitems));
	CLIPRINTF((stdout, "uriCache has %d items.\n", 
			uriCache->nitems));
	NETERROR(MCLI, ("uriCache has %d items.\n", 
			uriCache->nitems));
	CLIPRINTF((stdout, "gwcpCache has %d items.\n", 
			gwcpCache->nitems));
	NETERROR(MCLI, ("gwcpCache has %d items.\n", 
			gwcpCache->nitems));
	CLIPRINTF((stdout, "subnetCache has %d items.\n", 
			subnetCache->nitems));
	NETERROR(MCLI, ("subnetCache has %d items.\n", 
			subnetCache->nitems));
	CLIPRINTF((stdout, "cridCache has %d items.\n", 
			cridCache->nitems));
	NETERROR(MCLI, ("cridCache has %d items.\n", 
			cridCache->nitems));
	CLIPRINTF((stdout, "vpn cache has %d items\n", 
			vpnCache->nitems));
	NETERROR(MCLI, ("vpn cache has %d items\n", 
			vpnCache->nitems));
	CLIPRINTF((stdout, "Last process to hold vpn locks was %d\n", vpnCache->lock->pid));
	NETERROR(MCLI, ("Last process to hold vpn locks was %d\n", vpnCache->lock->pid));
	CLIPRINTF((stdout, "Last process to hold vpng locks was %d\n", vpnGCache->lock->pid));
	NETERROR(MCLI, ("Last process to hold vpng locks was %d\n", vpnGCache->lock->pid));
	CLIPRINTF((stdout, "vpng cache has %d items\n", vpnGCache->nitems));
	NETERROR(MCLI, ("vpng cache has %d items\n", vpnGCache->nitems));
	CLIPRINTF((stdout, "Last process to hold conf locks was %d\n", confCache->lock->pid));
	NETERROR(MCLI, ("Last process to hold conf locks was %d\n", confCache->lock->pid));
	CLIPRINTF((stdout, "conf cache has %d items\n", confCache->nitems));
	NETERROR(MCLI, ("conf cache has %d items\n", confCache->nitems));
	CLIPRINTF((stdout, "Last process to hold call locks was %d\n", callCache->lock->pid));
	NETERROR(MCLI, ("Last process to hold call locks was %d\n", callCache->lock->pid));
	CLIPRINTF((stdout, "call cache has %d items\n", callCache->nitems));
	NETERROR(MCLI, ("call cache has %d items\n", callCache->nitems));
	CLIPRINTF((stdout, "Last process to hold cp locks was %d\n", cpCache->lock->pid));
	NETERROR(MCLI, ("Last process to hold cp locks was %d\n", cpCache->lock->pid));
	CLIPRINTF((stdout, "cp cache has %d items\n", cpCache->nitems));
	NETERROR(MCLI, ("cp cache has %d items\n", cpCache->nitems));
	CLIPRINTF((stdout, "Last process to hold cpb locks was %d\n", cpbCache->lock->pid));
	NETERROR(MCLI, ("Last process to hold cpb locks was %d\n", cpbCache->lock->pid));
	CLIPRINTF((stdout, "cpb cache has %d items\n", cpbCache->nitems));
	NETERROR(MCLI, ("cpb cache has %d items\n", cpbCache->nitems));

	InitCfgFromCfgParms(lsMem->cfgParms);
	callStats();
	CacheDetach();

	return(0);
}

int
HandleTest( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     DefCommandData *dbDefaults = (DefCommandData *)comm->data;
     int i, k;
     int lockfd;
     int shmId;
	SipStats *sipStats;
	CfgParms *cfgParms;

	if ((argc > 0) && !strcmp(argv[0], "passive"))
	{
		return HandlePassiveTest(comm, argc, argv );
	}

   	fprintf(stdout, "testing database locks: ");

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	 else
	 {
		fprintf(stdout, "success.\n");
	 	CloseDatabases((DefCommandData *)comm->data);
	 }

	if ((shmId = CacheAttach()) == -1)
	{
		fprintf(stdout, "No Cache\n");
		goto _return;
	}

	PrintMap(map);

	if ((argc > 0) && !strcmp(argv[0], "blocks"))
	{
		PrintBlocks(map, 0);
	}
	callStats();
	fprintf(stdout, "Last process to hold locks was %d\n", regCache->lock->pid);
	fprintf(stdout, "testing regCache locks: ");

	/* Test the caches */
	if (CacheGetLocks(regCache, LOCK_WRITE, LOCK_TRY) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	else
	{
		fprintf(stdout, "success.\n");
		fprintf(stdout, "regCache has %d items.\n", 
			regCache->nitems);
		fprintf(stdout, "regidCache has %d items.\n", 
			regidCache->nitems);
		fprintf(stdout, "phoneCache has %d items.\n", 
			phoneCache->nitems);
		fprintf(stdout, "gwCache has %d items.\n", 
			gwCache->nitems);
		fprintf(stdout, "ipCache has %d items.\n", 
			ipCache->nitems);
		fprintf(stdout, "gkCache has %d items.\n", 
			gkCache->nitems);
		fprintf(stdout, "h323idCache has %d items.\n", 
			h323idCache->nitems);
		fprintf(stdout, "uriCache has %d items.\n", 
			uriCache->nitems);
		fprintf(stdout, "gwcpCache has %d items.\n", 
			gwcpCache->nitems);
		fprintf(stdout, "subnetCache has %d items.\n", 
			subnetCache->nitems);
		fprintf(stdout, "cridCache has %d items.\n", 
			cridCache->nitems);

		CacheReleaseLocks(regCache);
	}

	fprintf(stdout, "Last process to hold locks was %d\n", vpnCache->lock->pid);
	fprintf(stdout, "testing vpnCache locks: ");
	if (CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_TRY) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	else
	{
		fprintf(stdout, "success. Cache has %d items\n", 
			vpnCache->nitems);
		CacheReleaseLocks(vpnCache);
	}

	fprintf(stdout, "Last process to hold locks was %d\n", vpnGCache->lock->pid);
	fprintf(stdout, "testing vpnGCache locks: ");
	if (CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_TRY) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	else
	{
		fprintf(stdout, "success. Cache has %d items\n", vpnGCache->nitems);
		CacheReleaseLocks(vpnGCache);
	}
/*
	  
	  NOTE: confCache and callCache has been removed from shared memory.
	  Hence CacheXXX operation cannot be performed from another process 
	  such as cli. These checks needs to be implemented in the  request/response
	  paradigm. See cligkcalls.c for more details.

	fprintf(stdout, "Last process to hold locks was %d\n", confCache->lock->pid);
	fprintf(stdout, "testing confCache locks: ");
	if (CacheGetLocks(confCache, LOCK_WRITE, LOCK_TRY) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	else
	{
		fprintf(stdout, "success. Cache has %d items\n", confCache->nitems);
		CacheReleaseLocks(confCache);
	}

	fprintf(stdout, "Last process to hold locks was %d\n", callCache->lock->pid);
	fprintf(stdout, "testing callCache locks: ");
	if (CacheGetLocks(callCache, LOCK_WRITE, LOCK_TRY) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	else
	{
		fprintf(stdout, "success. Cache has %d items\n", callCache->nitems);
		CacheReleaseLocks(callCache);
	}
*/
	fprintf(stdout, "Last process to hold locks was %d\n", cpCache->lock->pid);
	fprintf(stdout, "testing cpCache locks: ");
	if (CacheGetLocks(cpCache, LOCK_WRITE, LOCK_TRY) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	else
	{
		fprintf(stdout, "success. Cache has %d items\n", cpCache->nitems);
		CacheReleaseLocks(cpCache);
	}

	fprintf(stdout, "Last process to hold locks was %d\n", cpbCache->lock->pid);
	fprintf(stdout, "testing cpbCache locks: ");
	if (CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_TRY) < 0)
	{
		fprintf(stdout, "error.\n");
	}
	else
	{
		fprintf(stdout, "success. Cache has %d items\n", cpbCache->nitems);
		CacheReleaseLocks(cpbCache);
	}

	// Cannot read off these locks or the TSM cache.
	// as it is stored in local memory
	//if (CacheGetLocks(transCache, LOCK_WRITE, LOCK_TRY) < 0)
	//{
	//	fprintf(stdout, "TSM Cache lock failed\n");
	//	fprintf(stdout, "Last process to hold locks was %d\n",
	//		transCache->pid);
	//}
	//else
	//{
	//	fprintf(stdout, "TSM cache has %d items\n", transCache->nitems);
	//	CacheReleaseLocks(transCache);
	//}

#if 0
	/* Test the updates cache also */
	if (MemGetRwLock(&lsMem->updatemutex, LOCK_WRITE, LOCK_TRY) != AL_OK)
	{
		fprintf(stdout, "update cache lock failed\n");
	}
	else
	{
		MemReleaseRwLock(&lsMem->updatemutex);
	}

	/* Test the updates cache also */
	if (MemGetRwLock(&lsMem->cpmutex, LOCK_WRITE, LOCK_TRY) != AL_OK)
	{
		fprintf(stdout, "cp cache lock failed\n");
	}
	else
	{
		MemReleaseRwLock(&lsMem->cpmutex);
	}

	/* Test the updates cache also */
	if (MemGetRwLock(&lsMem->cpbmutex, LOCK_WRITE, LOCK_TRY) != AL_OK)
	{
		fprintf(stdout, "cpb cache lock failed\n");
	}
	else
	{
		MemReleaseRwLock(&lsMem->cpbmutex);
	}

	/* Test the updates cache also */
	if (MemGetRwLock(&lsMem->callmutex, LOCK_WRITE, LOCK_TRY) != AL_OK)
	{
		fprintf(stdout, "call cache lock failed\n");
	}
	else
	{
		MemReleaseRwLock(&lsMem->callmutex);
	}

	/* Test the updates cache also */
	if (MemGetRwLock(&lsMem->confmutex, LOCK_WRITE, LOCK_TRY) != AL_OK)
	{
		fprintf(stdout, "conf cache lock failed\n");
	}
	else
	{
		MemReleaseRwLock(&lsMem->confmutex);
	}
#endif
	
	cfgParms = lsMem->cfgParms;
	if (cfgParms)
	{
		fprintf(stdout, "allowSrcAll = %d\n", cfgParms->allowSrcAll);
		fprintf(stdout, "allowDestAll = %d\n", cfgParms->allowDestAll);
		fprintf(stdout, "cacheTimeout = %d\n", cfgParms->cacheTimeout);
		fprintf(stdout, "RSDConfig = %d\n", cfgParms->RSDConfig);
		fprintf(stdout, "allowHairPin = %d\n", cfgParms->allowHairPin);
	}

	// Test out various block allocations
	printf("doing block test for shared memory blocks\n");
	SHM_MallocBlockTest();
	printf("block test done\n");

	CacheDetach();

    fprintf(stdout, "Test completed\n");

_return:

    return 1;

_error:
    return -1;
     
}

void callStats(void)
{
     /* First try to obtain an exclusive lock on the db file */
	int i, k;
	int lockfd;
	int shmId;
	AllocStats *a;
	SipStats *sipStats;
	SipStats sumStats;


	/* Print the licese information */
	for (i=0; i<nh323Instances; i++)
	{
		a = lsMem->allocStats[i];

		fprintf(stdout, "** instance %d (%s) **\n", i, h323InstanceName(i));

		// update the shared memory with h.323 alloc
		// protocol = 60 bytes each
		fprintf(stdout, "xprotocols=%d, nprotocols=%d\n",
			a->xprotocols, a->nprotocols);

		// procs = 1024 bytes
		fprintf(stdout, "xprocs=%d, nprocs=%d\n",
			a->xprocs, a->nprocs);

		// tpktchans = 4200 bytes
		fprintf(stdout, "xtpktchans=%d, ntpktchans=%d\n",
			a->xtpktchans, a->ntpktchans);

		// udpchans = 4200 bytes
		fprintf(stdout, "xudpchans=%d, nudpchans=%d\n",
			a->xudpchans, a->nudpchans);

		// channels = 40 bytes
		fprintf(stdout, "xchannels=%d, nchannels=%d\n",
			a->xchannels, a->nchannels);

		// messages = 20 bytes
		fprintf(stdout, "xmessages=%d, nmessages=%d\n",
			a->xmessages, a->nmessages);

		// channel descriptors 
		fprintf(stdout, "xchandescs=%d, nchandescs=%d\n",
			a->xchandescs, a->nchandescs);

		// channel descriptors 
		fprintf(stdout, "xtimers=%d, ntimers=%d\n",
			a->xtimers, a->ntimers);

		// channel descriptors 
		fprintf(stdout, "xevents=%d, nevents=%d\n",
			a->xevents, a->nevents);

		// vt node count
		fprintf(stdout, "hVal size =%d\n",a->vtnodecount );

		if (a->nCalls)
		fprintf(stdout, "nCalls =%d\n", *a->nCalls);
		if (a->arqRate)
		fprintf(stdout, "arqRate =%d/s\n", *a->arqRate);
		if (a->setupRate)
		fprintf(stdout, "setupRate =%d/s\n", *a->setupRate);
		if (a->selims)
		fprintf(stdout, "selims =%dms\n", *a->selims);
		if (a->bridgems)
		fprintf(stdout, "bridgems =%dms\n", *a->bridgems);
	}

	fprintf (stdout, "xthreads : %d\n", xthreads);
	// sipstats
	sipStats = lsMem->sipStats;
	memset ((char *)&sumStats, 0, sizeof (sumStats));
	for (i=0;i<xthreads;i++) {
		sumStats.invs    += sipStats[i].invs;
		sumStats.invsr   += sipStats[i].invsr;
		sumStats.invc    += sipStats[i].invc;
		sumStats.invcr   += sipStats[i].invcr;
		sumStats.invsfr  += sipStats[i].invsfr;
		sumStats.invcfr  += sipStats[i].invcfr;
		sumStats.byes    += sipStats[i].byes;
		sumStats.byesr   += sipStats[i].byesr;
		sumStats.byec    += sipStats[i].byec;
		sumStats.byecr   += sipStats[i].byecr;
		sumStats.cs      += sipStats[i].cs;
		sumStats.csr     += sipStats[i].csr;
		sumStats.cc      += sipStats[i].cc;
		sumStats.ccr     += sipStats[i].ccr;
		sumStats.notrans += sipStats[i].notrans;
		sumStats.ntrans  += sipStats[i].ntrans;
		sumStats.ptime   += sipStats[i].ptime;
	}
	if (sipStats)
	{
		fprintf(stdout, "invs=%d, invsr=%d, invc=%d, invcr=%d\n",
			sumStats.invs, sumStats.invsr, sumStats.invc,
			sumStats.invcr);
		fprintf(stdout, "invsfr=%d, invcfr=%d, byes=%d, byesr=%d\n",
			sumStats.invsfr, sumStats.invcfr, sumStats.byes,
			sumStats.byesr);
		fprintf(stdout, "byec=%d, byecr=%d, cs=%d, csr=%d\n",
			sumStats.byec, sumStats.byecr, sumStats.cs,
			sumStats.csr);
		fprintf(stdout, "no call/trans=%d\n",
			sumStats.notrans);
		fprintf(stdout, "cc=%d, ccr=%d, ntrans=%d, ptime=%lld\n",
			sumStats.cc, sumStats.ccr, sumStats.ntrans,
			sumStats.ptime);
	}

}

#if 0
int
HandleRedunds( Command *comm, int argc, char **argv )
{
     	int shmId;
	CacheEntry *ce, *nextce = 0;
	CacheTableInfo *info = 0;
	CacheVpnEntry *cacheVpnEntry;
	CacheVpnGEntry *cacheVpnGEntry;
	CacheVpnRouteEntry *cacheRouteEntry;

	if ((shmId = CacheAttach()) == -1)
	{
		fprintf(stdout, "No Cache\n");
		return -1;
	}

	/* Test the updates cache also */
	if (MemGetRwLock(&lsMem->updatemutex, LOCK_WRITE, LOCK_TRY) != AL_OK)
	{
		fprintf(stdout, "update cache lock failed\n");
	}
	else
	{
		/* Print the list */
	      	for (ce = updateList->next; ((ce != 0) && (ce != updateList)); 
			ce = ce->next)
		{
		  	if (ce->entry == NULL)
			{
				/* Dont process this one */
				printf("null entry\n");
				continue;
			}

			switch(ce->type)
			{
			case CACHE_INFO_ENTRY:
				info = (CacheTableInfo *)ce->entry;
				if (info->state & CACHE_NEEDS_DELETE)
				{
					printf("Found an iedge marked CACHE_NEEDS_DELETE");
				}
				if (info->state & CACHE_PUSH_ENTRY)
				{
					printf("Found an iedge marked CACHE_PUSH_ENTRY");
				}
				PrintInfoEntry(stdout, info);

				break;
			case CACHE_VPN_ENTRY:
				cacheVpnEntry = (CacheVpnEntry *)ce->entry;
				if (cacheVpnEntry->state & CACHE_NEEDS_DELETE)
				{
					printf("Found a vpn entry marked CACHE_NEEDS_DELETE");
				}
				if (cacheVpnEntry->state & CACHE_PUSH_ENTRY)
				{
					printf("Found a vpn entry marked CACHE_PUSH_ENTRY");
				}

				printf("vpn %s\n", cacheVpnEntry->vpnEntry.vpnId);
				break;
			case CACHE_VPNG_ENTRY:
				cacheVpnGEntry = (CacheVpnGEntry *)ce->entry;
				if (cacheVpnGEntry->state & CACHE_NEEDS_DELETE)
				{
					printf("Found a vpn group entry marked CACHE_NEEDS_DELETE");
				}
				if (cacheVpnGEntry->state & CACHE_PUSH_ENTRY)
				{
					printf("Found a vpn group entry marked CACHE_PUSH_ENTRY");
				}
				printf("vpn group %s\n", cacheVpnGEntry->vpnGroupEntry.vpnGroup);
				break;
			case CACHE_ROUTE_ENTRY:
				cacheRouteEntry = (CacheVpnRouteEntry *)ce->entry;
				PrintCREntry(cacheRouteEntry);
				break;
			default:
				break;
			}

		}

		MemReleaseRwLock(&lsMem->updatemutex);
	}

	CacheDetach();

	return 0;
}
#endif
int
HandlelsAlarm( Command *comm, int argc, char **argv )
{

    int i = comm->subCommsLen/sizeof(Command), j = 0, k;
    int rc = 1;

     /* Now we look at the subcommands */

     if (argc < 1)
     {
        rc = HandleAlarmStatus( comm, argc, argv );
     }else

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
//     if (j == i)
//     {
//		if (cliLibFlags == 0)
//		   return -xleInvalArgs;
//		else
//		   PrintUsage(comm, argc, argv, comm->flags);
//     }

     return rc;

_error:
     return -1;
     
 
}

int
HandleAlarmStatus( Command *comm, int argc, char **argv ){
   int shmId;
   time_t valarms[MAX_LS_ALARM];
   time_t mrvalarms[MAX_LS_ALARM];
   int rc = -1;

    if ((shmId = CacheAttach()) == -1)
    {
            fprintf(stdout, "No Cache\n");
            return -1;
    }

   if((LockGetLock(&(lsMem->alarmmutex), LOCK_READ, LOCK_TRY) ) == AL_OK){
     memcpy((void*)valarms,(void*)lsMem->lsVportAlarm,MAX_LS_ALARM*sizeof(time_t));
     memcpy((void*)mrvalarms,(void*)lsMem->lsMRVportAlarm,MAX_LS_ALARM*sizeof(time_t));
     LockReleaseLock(&(lsMem->alarmmutex));
	 if (_lsAlarmFn)
	 {
     	_lsAlarmFn(valarms, mrvalarms);
	 }
     rc = 0;
   }
    CacheDetach();
    return rc;
}

int
HandleAlarmClear( Command *comm, int argc, char **argv ){
   int shmId;
   int rc = -1;

    if ((shmId = CacheAttach()) == -1)
    {
            fprintf(stdout, "No Cache\n");
            return -1;
    }

   if((LockGetLock(&(lsMem->alarmmutex), LOCK_WRITE, LOCK_TRY) ) == AL_OK){
     memset(lsMem->lsVportAlarm,0,MAX_LS_ALARM*sizeof(time_t));
     memset(lsMem->lsMRVportAlarm,0,MAX_LS_ALARM*sizeof(time_t));
     LockReleaseLock(&(lsMem->alarmmutex));
     rc =0;
   }
    CacheDetach();
    return rc;
}

int
Handlelstat( Command *comm, int argc, char **argv )
{
	int vu,va,vt,mu,ma,mt;
	int shmId;
	char expiry_time[80] ;
	char *p;
	char flist[MAX_FEATURES*FEATURE_NAME_LEN] = {0};


	if ((shmId = CacheAttach()) == -1)
	{
		fprintf(stdout, "No Cache\n");
		return -1;
	}
	vt = lsMem->maxCalls;
	vu = nlm_getUsedvportNolock();
	va = vt-vu;
	mt = lsMem->maxMRCalls;
	mu = lsMem->nMRCalls;
	ma = mt-mu;
	
	if(lsMem->expiry_time == 0)
	{
		strcpy(expiry_time,"Never Expires ");
	}
	else 
		strcpy(expiry_time,ctime(&lsMem->expiry_time));

	fprintf(stdout,"%s\t\t%s\n",macmsg,lsMem->macstr);
	fprintf(stdout,"%s\t\t%s\n",expmsg,expiry_time);
	fprintf(stdout,"Licensed Features\t\t%s\n",nlm_getFeatureList(flist));
	fprintf(stdout, "\n%s %s\t\t\t%d\n%s %s\t\t%d\n%s %s\t\t\t%d\n",totalstr,
		callstr,vt,availstr,callstr,va,usedstr,callstr,vu);
	
	if(fceEnabled())
	{
		fprintf(stdout, "\n%s %s %s\t%d\n%s %s %s\t%d\n%s %s %s\t%d\n",totalstr,
			mrstr,callstr,mt,availstr,mrstr,callstr,ma,usedstr,mrstr,callstr,mu);
	}

	CacheDetach();

	return 1;
}

int h323PollQ;
int
Handlestats( Command *comm, int argc, char **argv )
{
	struct timeval 	tp;
	int				i,index;
	IntervalStats 	*isptr;
	int				shmId;

	if ((shmId = CacheAttach()) == -1)
	{
		fprintf(stdout, "No Cache\n");
		return -1;
	}
	gettimeofday(&tp,NULL);

	fprintf(stdout, "H323StackQ = %d\n",lsMem->callStats->h323PollQ);
	fprintf(stdout, "ThreadPoolQ = %d\n",lsMem->callStats->h323PoolQ);
	fprintf(stdout, "Active Calls = %d\n",nlm_getUsedvportNolock());

	index = (tp.tv_sec -9)%60;
	fprintf(stdout, "Stats for last 10 seconds\nSetups:\t\t");
	for(i = 0,isptr = lsMem->callStats->secStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%60].setup);
	}
	fprintf(stdout, "\nConnects:\t");
	for(i = 0,isptr = lsMem->callStats->secStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%60].connect);
	}
	fprintf(stdout, "\noutSetups:\t");
	for(i = 0,isptr = lsMem->callStats->secStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%60].outSetup);
	}

	index = ((tp.tv_sec)/60 -9)%60;
	fprintf(stdout, "\nStats for last 10 minutes\nSetups:\t\t");
	for(i = 0,isptr = lsMem->callStats->minStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%60].setup);
	}
	fprintf(stdout, "\nConnects:\t");
	for(i = 0,isptr = lsMem->callStats->minStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%60].connect);
	}
	fprintf(stdout, "\noutSetups:\t");
	for(i = 0,isptr = lsMem->callStats->minStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%60].outSetup);
	}

	index = (tp.tv_sec/3600 -9)%24;
	fprintf(stdout, "\nStats for last 10 hours\nSetups:\t\t");
	for(i = 0,isptr = lsMem->callStats->hourStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%24].setup);
	}
	fprintf(stdout, "\nConnect:\t");
	for(i = 0,isptr = lsMem->callStats->hourStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%24].connect);
	}
	fprintf(stdout, "\noutSetups:\t");
	for(i = 0,isptr = lsMem->callStats->hourStat; i<10;++i)
	{
		fprintf(stdout, "%d\t",isptr[(index+i)%24].outSetup);
	}


	fprintf(stdout, "\n");

	CacheDetach();
	return 1;
}

int
Handlelupdate( Command *comm, int argc, char **argv )
{
	int shmId;

	if ((shmId = CacheAttach()) == -1)
	{
		fprintf(stdout, "No Cache\n");
		return -1;
	}

	if (license_init() )
    {
        fprintf(stdout,"Error getting license. Stopping iServer.\n");
        system("/usr/local/nextone/bin/iserver all stop");
    }
	else {
        fprintf(stdout,"Successfully updated license.\n");
	}

	CacheDetach();
	return 1;
}

int
HandleCache( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	 rc = 1;

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

	if (CliHandleInteractive("cli cache>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;

_error:
     return -1;
}

int
HandleIgnore( Command *comm, int argc, char **argv )
{
	int revno;

	revno = CliUpdateRev();
	CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	return xleOk;
}

int
HandleDbHist( Command *comm, int argc, char **argv )
{
	HDB     *hdbp;

    hdbp = OpenCliHist();

    PrintCliHist(hdbp);

    CloseCliHist(hdbp);

    return(1);
}

int
HandleRsd (Command *comm, int argc, char **argv)
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
HandleTrigger( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	argcOrig = argc;
	argvOrig = argv;

	 revno = -1;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

			/* pass the open file to the subcommands */
			comm->subComms[j].data = comm->data;
			rc = comm->subComms[j].commFn(
				&(comm->subComms[j]), --argc, ++argv);
			break;
		}
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "trigger", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

     /* Handle the case when the command was not found */
	if (i==j)
     {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
		   PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli trigger>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;
	
_error:
     return -1;
}

int
HandleRealm( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	argcOrig = argc;
	argvOrig = argv;

	 revno = -1;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

			/* pass the open file to the subcommands */
			comm->subComms[j].data = comm->data;
			rc = comm->subComms[j].commFn(
				&(comm->subComms[j]), --argc, ++argv);
			break;
		}
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "realm", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

     /* Handle the case when the command was not found */
	if (i==j)
     {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
		   PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli realm>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;
	
_error:
     return -1;
}

int
HandleIgrp( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	argcOrig = argc;
	argvOrig = argv;

	 revno = -1;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

			/* pass the open file to the subcommands */
			comm->subComms[j].data = comm->data;
			rc = comm->subComms[j].commFn(
				&(comm->subComms[j]), --argc, ++argv);
			break;
		}
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "igrp", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

     /* Handle the case when the command was not found */
	if (i==j)
     {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
		   PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli igrp>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;
	
_error:
     return -1;
}

int
HandleVnet( Command *comm, int argc, char **argv )
{
     /* First try to obtain an exclusive lock on the db file */
     int i = comm->subCommsLen/sizeof(Command), j = 0;
	 int rc = 1;
	int revno = -1, argcOrig;
	char **argvOrig;
	char *argvtmp[64] = { 0 }, buffer[256];

_handle_command:

	argcOrig = argc;
	argvOrig = argv;

	 revno = -1;

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
		   	if (comm->subComms[j].flags & COMMANDF_STATE)
			{
				// Obtain the new revision number of the db
				revno = CliUpdateRev();
			}

			/* pass the open file to the subcommands */
			comm->subComms[j].data = comm->data;
			rc = comm->subComms[j].commFn(
				&(comm->subComms[j]), --argc, ++argv);
			break;
		}
     }

_return:

	// Post the cli command
	if (rc == xleOk)
	{
		// Even if the command produced an error,
		// send it. This is because we dont know how much
		// of the db was changed.
		CliPostCmdline(CMD_CLI, "vnet", argcOrig, argvOrig, revno, rc, 0);
	}
	else
	{
		CliPostCmd(CMD_CLI, "ignore", "", revno, 0, 0);
	}

     /* Handle the case when the command was not found */
	if (i==j)
     {
		if (cliLibFlags == 0)
		   return -xleInvalArgs;
		else
		   PrintUsage(comm, argc, argv, comm->flags);
     }

	if (CliHandleInteractive("cli vnet>", buffer, 256, &argc, argvtmp, 64))
	{
		argv = argvtmp;
		goto _handle_command;
	}

     return rc;
	
_error:
     return -1;
}

int
HandleSCM( Command *comm, int argc, char **argv )
{
	int i, shmid, *p_state = NULL;
	int l_tot, l_min, l_max, l_cur;

	shmid = shmget(ISERVER_STATE_SHM_KEY, sizeof(int), SHM_R);
	if( shmid > 0)
	{
		p_state = shmat(shmid, (void *)0, SHM_R);
		if( (int)p_state == -1)
		{
			p_state = NULL;
		}
	}
	if( p_state )
	{
		fprintf(stdout, "Signaling State\t\t\t%s\n", ( (*p_state) == 2 ? "active" : "standby") );
	}	
	else
	{
		fprintf(stdout, "Signaling State\t\t\tstandalone or uninitialized\n");
	}

	if (CacheAttach() < 0)
	{
		fprintf(stdout, "No Cache\n");
		return -1;
	}

	fprintf(stdout, "SCM Total States\t\t%d\n", lsMem->scm->scmCallCache?lsMem->scm->scmCallCache->nitems:0);
	fprintf(stdout, "SCM Pending States\t\t%d\n", lsMem->scm->pendingStates);
	fprintf(stdout, "SCM States successfully sent\t%d\n", lsMem->scm->successStates);
	fprintf(stdout, "SCM States failed\t\t%d\n", lsMem->scm->errorStates);

	if(lsMem->scm->nelems > 0)
	{
		l_min = l_max = l_tot = lsMem->scm->transportLatency[0];

		for (i=1; i < lsMem->scm->nelems; i++)
		{
			l_cur = lsMem->scm->transportLatency[i];

			l_min = (l_cur < l_min) ? l_cur : l_min;
			l_max = (l_cur > l_max) ? l_cur : l_max;

			l_tot += l_cur;
		}

		fprintf(stdout, "SCM processing latency (ms)\t\tmin=%3.2f max=%3.2f avg(%d)=%3.2f\n",
				l_min/1000.0, l_max/1000.0, i, l_tot/(i*1000.0));
	}

	CacheDetach();

	return 1;
}

