#ifndef _cli_h_
#define _cli_h_

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include "spversion.h"

#include "alerror.h"
#include "bits.h"
#include "ipc.h"
#include "ipcerror.h"
#include "key.h"
#include "db.h"
#include "profile.h"
#include "mem.h"
#include "phone.h"
#include "srvrlog.h"
#include "gw.h"
#include "fileutils.h"
#ifdef USE_LDAP
#include <lber.h>
#include <ldap.h>
#endif
#include "rs.h"
#include "nxosd.h"
#include <stdlib.h>

#define NAME_LEN  24

extern char *progName;
extern int cli_debug;
extern int send2Rs;
extern int saveindb;
extern char clifiledir[80];

extern gdbm_error gdbm_errno;

#define COMMANDF_HIDDEN	0x0001
#define COMMANDF_USER	0x0002
#define COMMANDF_ALL	0x0003
#define COMMANDF_STATE	0x0004		// causes a state change in the db

extern short cliLibFlags;
extern int clicmdop, clifileop;

#define CLIOP_NONE	0
#define CLIOP_ADD	1
#define CLIOP_DELETE	2
#define CLIOP_REPLACE	3
#define CLIOP_CREAT	4		// Dont add in cache, just the DB

#define CLIOP_VERIFY	1

#define CLIOP_ERR	1
#define CLIOP_ERRDUP	2
#define CLIOP_ERRVF	2

#define CLI_GET_ATTR_VPN		1
#define CLI_GET_ATTR_ROUTE		2
#define CLI_GET_ATTR_VPNG 		3
#define CLI_GET_ATTR_CP			4
#define CLI_GET_ATTR_CPB		5
#define CLI_GET_ATTR_GK			6
#define CLI_GET_ATTR_TRIGGER	7
#define CLI_GET_ATTR_REALM		8
#define CLI_GET_ATTR_IGRP		9
#define CLI_GET_ATTR_VNET		10

/* used on clirealm.c */
#define REALM_OPER_ADD          1
#define REALM_OPER_REMOVE       2
#define REALM_OPER_UP           3
#define REALM_OPER_DOWN         4

extern int nroutes, nbindings, niedges, nplans, nvpns, nvpngs, nrealms, nigrps, ntriggers, nvnets;
extern int nerrors;
extern int cli_ix;

#define sys_execd       system
#define CLIPRINTF(x) do { if (cliLibFlags & (COMMANDF_USER|COMMANDF_HIDDEN)) fprintf x; } while (0)

typedef struct _Command
{
     char name[NAME_LEN];             /* Name of command */
     int ( *commFn)
     (struct _Command *, int, char **); /* Function for executing it */
     void ( *usageFn)                 
     (struct _Command *, int, char **); /* Function which prints usage */
     struct _Command *subComms;       /* pointer to subcommands */
     int subCommsLen;                 /* Number of these sub commands */
     void *data;                      /* Any command specific data */
     int dataLen;                     /* Length of data */
     
     int level;                       /* recursion level of command */
     short flags;		      /* See above COMMANDF stuff */
} Command;

#define CLI_BLOCK_SIGNALS	{\
     sigset_t o_signal_mask, n_signal_mask;\
     FillAllSignals(&n_signal_mask);\
     BlockAllSignals(&n_signal_mask, &o_signal_mask)

#define CLI_UNBLOCK_SIGNALS	UnblockAllSignals(&o_signal_mask, 0); }

extern int (*_lsAlarmFn)(time_t* valarms, time_t* mrvalarms);
extern int (*_CliRouteLogFn)(RouteNode *routeNode);

int
HandleNetoid(Command *, int , char **);

int
HandleFaxs(Command *, int , char **);

int
HandleGk(Command *, int , char **);

int
HandleTrigger(Command *, int , char **);

int
HandleRealm(Command *, int , char **);

int
HandleIgrp(Command *, int , char **);

int
HandleVnet(Command *, int , char **);

int
HandleCall(Command *, int , char **);

int
HandleGkReg(Command *, int , char **);

int
HandleNetoidAdd(Command *, int , char **);

int
HandleNetoidDelete(Command *, int , char **);

int
HandleNetoidFind(Command *, int , char **);

int
HandleNetoidLkup(Command *, int , char **);

int
HandleNetoidList(Command *, int , char **);

int
HandleNetoidCache(Command *, int , char **);

int
HandleNetoidEdit(Command *, int , char **);

int
HandleNetoidReg(Command *, int , char **);

int
HandleNetoidRoute(Command *, int , char **);

int
HandleNetoidHunt(Command *, int , char **);

int
HandleVpn(Command *, int , char **);

int
HandleVpnAdd(Command *, int , char **);

int
HandleVpnDelete(Command *, int , char **);

int
HandleVpnList(Command *, int , char **);

int
HandleVpnCache(Command *, int , char **);

int
HandleVpnEdit(Command *, int , char **);

int
HandleVpnVpnG(Command *, int , char **);

int
HandleVpnG(Command *, int , char **);

int
HandleVpnGAdd(Command *, int , char **);

int
HandleVpnGCache(Command *, int , char **);

int
HandleVpnGEdit(Command *, int , char **);

int
HandleVpnGDelete(Command *, int , char **);

int
HandleVpnGList(Command *, int , char **);

int
HandleNetoidPhones(Command *, int , char **);

int
HandleNetoidEmail(Command *, int , char **);

int
HandleNetoidZone(Command *, int , char **);

int
HandleNetoidVpns(Command *, int , char **);

int
HandleNetoidIp(Command *, int , char **);

int
HandleNetoidProxy(Command *, int , char **);

int
HandleDbExport(Command *, int , char **);

int
HandleDbSave(Command *, int , char **);

int
HandleDbCreate(Command *, int , char **);

int
HandleDbAdd(Command *, int , char **);

int
HandleDbReplace(Command *, int , char **);

int
HandleDbDelete(Command *, int , char **);

int
HandleDbCopy(Command *, int , char **);

int
HandleDbClean(Command *, int , char **);

int
HandleDbStale(Command *, int , char **);

int
HandleDb (Command *, int , char **);

int
HandleDbInit (Command *, int , char **);

int
HandleDbInfo (Command *, int , char **);

int
HandleDbOrg (Command *, int , char **);

int
HandleDbRepl (Command *, int , char **);

int
HandleDbHist (Command *, int , char **);

int
HandleDbRev (Command *, int , char **);

int
HandleDbRevShow (Command *, int , char **);

int
HandleDbRevIncr (Command *, int , char **);

int
HandleDbRevMod (Command *, int , char **);

int
HandleDbSwitch (Command *, int , char **);

int
HandleTest (Command *, int , char **);

int
HandleRedunds (Command *, int , char **);

int
Handlelstat (Command *, int , char **);

int
HandleSCM (Command *, int , char **);

int
HandlelsAlarm(Command *, int, char **);

int
HandleCP(Command *, int , char **);

int
HandleCR(Command *, int , char **);

int
HandleCPAdd(Command *, int , char **);

int
HandleCPDelete(Command *, int , char **);

int
HandleCPLkup(Command *, int , char **);

int
HandleCPEdit(Command *, int , char **);

int
HandleCPList(Command *, int , char **);

int
HandleCPCache(Command *, int , char **);

int
HandleCRAdd(Command *, int , char **);

int
HandleCRDelete(Command *, int , char **);

int
HandleCRList(Command *, int , char **);

int
HandleCRCache(Command *, int , char **);

int
HandleCRLkup(Command *, int , char **);

int
HandleCREdit(Command *, int , char **);

int
HandleTest (Command *, int , char **);

int
HandleClean (Command *, int , char **);

int
HandleShow (Command *, int , char **);

int
HandleFaxsAdd(Command *, int , char **);

int
HandleFaxsDelete(Command *, int , char **);

int
HandleFaxsLkup(Command *, int , char **);

int
HandleFaxsList(Command *, int , char **);

int
HandleCallCache(Command *, int , char **);

int
HandleCallLkup(Command *, int , char **);

int
HandleCallDelete(Command *, int , char **);

int
Handlestats( Command *, int , char **);

int
Handlelupdate( Command *, int , char **);

int
HandleCache( Command *, int , char **);

int
HandleIgnore( Command *, int , char **);

int
HandleHistDB( Command *, int , char **);

/*************** USAGE COMMANDS ********************/
void
CacheCreateUsage(Command *comm, int, char **);

void
CacheCleanUsage(Command *comm, int, char **);

void
NetoidAddUsage(Command *comm, int, char **);

void
NetoidDeleteUsage(Command *comm, int, char **);

void
NetoidFindUsage(Command *comm, int, char **);

void
NetoidListUsage(Command *comm, int, char **);

void
NetoidLkupUsage(Command *comm, int, char **);

void
NetoidCacheUsage(Command *comm, int, char **);

void
NetoidRouteUsage(Command *comm, int, char **);

void
NetoidHuntUsage(Command *comm, int, char **);

void
VpnAddUsage(Command *comm, int, char **);

void
VpnDeleteUsage(Command *comm, int, char **);

void
VpnCacheUsage(Command *comm, int, char **);

void
VpnEditUsage(Command *comm, int, char **);

void
VpnRenameUsage(Command *comm, int, char **);

void
VpnGRenameUsage(Command *comm, int, char **);

void
VpnGCacheUsage(Command *comm, int, char **);

void
VpnListUsage(Command *comm, int, char **);

void
VpnVpnGUsage(Command *comm, int, char **);

void
VpnGAddUsage(Command *comm, int, char **);

void
VpnGDeleteUsage(Command *comm, int, char **);

void
VpnGEditUsage(Command *comm, int, char **);

void
VpnGListUsage(Command *comm, int, char **);

void
NetoidVpnsUsage(Command *comm, int, char **);

void
NetoidPhonesUsage(Command *comm, int, char **);

void
NetoidEmailUsage(Command *comm, int, char **);

void
NetoidZoneUsage(Command *comm, int, char **);

void
NetoidIpUsage(Command *comm, int, char **);

void
NetoidProxyUsage(Command *comm, int, char **);

void
NetoidEditUsage(Command *comm, int, char **);

void
NetoidRegUsage(Command *comm, int, char **);

void
DbExportUsage(Command *comm, int, char **);

void
DbSaveUsage(Command *comm, int, char **);

void
DbCreateUsage(Command *comm, int, char **);

void
DbAddUsage(Command *comm, int, char **);

void
DbReplaceUsage(Command *comm, int, char **);

void
DbDeleteUsage(Command *comm, int, char **);

void
DbInitUsage(Command *comm, int, char **);

void
DbInfoUsage(Command *comm, int, char **);

void
DbCopyUsage(Command *comm, int, char **);

void
DbCleanUsage(Command *comm, int, char **);

void
DbStaleUsage(Command *comm, int, char **);

void
DbOrgUsage(Command *comm, int, char **);

void
DbReplUsage(Command *comm, int, char **);

void
DbHistUsage(Command *comm, int, char **);

void
DbRevUsage(Command *comm, int, char **);

void
DbRevShowUsage(Command *comm, int, char **);

void
DbRevIncrUsage(Command *comm, int, char **);

void
DbRevModUsage(Command *comm, int, char **);

void
DbSwitchUsage(Command *comm, int, char **);

void
FaxsAddUsage(Command *comm, int, char **);

void
FaxsDeleteUsage(Command *comm, int, char **);

void
FaxsListUsage(Command *comm, int, char **);

void
FaxsLkupUsage(Command *comm, int, char **);

void
CPAddUsage(Command *comm, int, char **);

void
CPDeleteUsage(Command *comm, int, char **);

void
CPListUsage(Command *comm, int, char **);

void
CPCacheUsage(Command *comm, int, char **);

void
CPEditUsage(Command *comm, int, char **);

void
CPLkupUsage(Command *comm, int, char **);

void
CRAddUsage(Command *comm, int, char **);

void
CRDeleteUsage(Command *comm, int, char **);

void
CRListUsage(Command *comm, int, char **);

void
CRCacheUsage(Command *comm, int, char **);

void
CRLkupUsage(Command *comm, int, char **);

void
CREditUsage(Command *comm, int, char **);

void
GkRegUsage(Command *comm, int, char **);

void
CallCacheUsage(Command *comm, int, char **);

void
CallLkupUsage(Command *comm, int, char **);

void
CallDeleteUsage(Command *comm, int, char **);

/* Aux fns */
int
DeleteNetoidFromCache(NetoidInfoEntry *netInfo);

void
PrintUsage(Command *comm, int argc, char **argv, short mflags);

int
StoreVpnInDb(Command *comm, VpnEntry *vpnEntry, int shmId);

int
UpdateNetoidInCache(NetoidInfoEntry *netInfo);

char *
GetInput(FILE *stream, char *buf, int xinlen);

void
NetoidStorePair(NetoidInfoEntry *netInfo, ClientAttribs *, char *f, char *v);

void
VpnStorePair(VpnEntry *vpnEntry, char *f, char *v);

int
RealmStorePair(RealmEntry *rmEntry, char *f, char *v);

int
IgrpStorePair(IgrpInfo *igrp, char *f, char *v);

int
GetAttrPairs(char *comm, int *argcp, char ***argvp, void *entry, int type);

int
VpnGetAttrPairs(char *comm, int *argcp, char ***argvp, NetoidInfoEntry *netInfo);

int
OpenDatabases(DefCommandData *dbDefaults);

int
CloseDatabases(DefCommandData *dbDefaults);

void
AgeIedgesInVpnGs(DefCommandData *dbs, char *vpng1, char *vpng2);

int
CliRouteLogFn(RouteNode *routeNode);

int
lsAlarmFn(time_t* valarms, time_t* mrvalarms);

int
HandleAlarmStatus( Command *comm, int argc, char **argv );

int
HandleAlarmClear( Command *comm, int argc, char **argv );

int
HandleCacheCreate(Command *, int , char **);

int
HandleCacheClean(Command *, int , char **);

int 
CliSendToRs(Cmd *cmd);

void 
CliPostCpCommand(char *path, int revno, int expectedrc, int action);

void
CliPostCmd(int cmdtype, char *cmdprefix, char *cmdrest, long revno, 
		int expectedrc, int action);

void
CliPostCmdline(int cmdtype,
		char *cmdprefix, int argc, char **argv, long revno, 
		int expectedrc, int action);

int
CliUpdateRev(void);

int
HandleRsd (Command*, int, char**);

int
HandleRsdClear (Command*, int, char**);

int
HandleRsdAdd (Command*, int, char**);

int
HandleRsdDelete (Command*, int, char**);

int
HandleRsdList (Command*, int, char**);

int
HandleRsdSync (Command*, int, char**);

int
HandleRsdStatus (Command*, int, char**);

void
RsdClearUsage (Command*, int, char**);

void
RsdAddUsage (Command*, int, char**);

void
RsdDeleteUsage (Command*, int, char**);

void
RsdListUsage (Command*, int, char**);

void
RsdSyncUsage (Command*, int, char**);

void
RsdStatusUsage (Command*, int, char**);

int
HandleTriggerAdd (Command*, int, char**);

int
HandleTriggerDelete (Command*, int, char**);

int
HandleTriggerList (Command*, int, char**);

int
HandleTriggerCache (Command*, int, char**);

int
HandleTriggerEdit (Command*, int, char**);

int
HandleTriggerPurge (Command*, int, char**);

void
TriggerAddUsage (Command*, int, char**);

void
TriggerDeleteUsage (Command*, int, char**);

void
TriggerListUsage (Command*, int, char**);

void
TriggerEditUsage (Command*, int, char**);

void
TriggerPurgeUsage (Command*, int, char**);

int 
ProcessCommand(int argc, char **argv);

int
RealmEditHandler(RealmEntry *new, RealmEntry *old);

int
RealmIfNameEdit(RealmEntry *newrm, RealmEntry *oldrm);

int
RealmRSAEdit(RealmEntry *newrm, RealmEntry *oldrm);

int
RealmMaskEdit(RealmEntry *newrm, RealmEntry *oldrm);

int
HandleRealmAdd (Command*, int, char**);

int
HandleRealmDelete (Command*, int, char**);

int
HandleRealmList (Command*, int, char**);

int
HandleRealmCache (Command*, int, char**);

int
HandleRealmLkup (Command*, int, char**);

int
HandleRealmEdit (Command*, int, char**);

int
HandleRealmUp (Command*, int, char**);

int
HandleRealmDown (Command*, int, char**);

int
HandleRealmOpen (Command*, int, char**);

int
HandleRealmClose (Command*, int, char**);

void
RealmAddUsage (Command*, int, char**);

void
RealmDeleteUsage (Command*, int, char**);

void
RealmListUsage (Command*, int, char**);

void
RealmEditUsage (Command*, int, char**);

void
RealmCacheUsage (Command*, int, char**);

void
RealmLkupUsage (Command*, int, char**);

void
RealmUpUsage (Command*, int, char**);

void
RealmDownUsage (Command*, int, char**);

void
RealmOpenUsage (Command*, int, char**);

void
RealmCloseUsage (Command*, int, char**);

int
_RealmOpenVip(RealmEntry *rmEntry, char post_proc);

int
RealmCloseVip(RealmEntry *rmEntry, VnetEntry *vnetEntryPtr);

int 
RealmVipUpDown(RealmEntry *rmEntry, unsigned short new_status);

void
GetParentIfName(char *lifn, char *pifn);

int
RealmAllVips(unsigned short op);

int
RealmEnableVipSig(RealmEntry *rmEntry);

int
RealmDisableVipSig(RealmEntry *rmEntry);

int
HandleIgrpAdd (Command*, int, char**);

int
HandleIgrpDelete (Command*, int, char**);

int
HandleIgrpList (Command*, int, char**);

int
HandleIgrpCache (Command*, int, char**);

int
HandleIgrpLkup (Command*, int, char**);

int
HandleIgrpEdit (Command*, int, char**);

void
IgrpAddUsage (Command*, int, char**);

void
IgrpDeleteUsage (Command*, int, char**);

void
IgrpListUsage (Command*, int, char**);

void
IgrpEditUsage (Command*, int, char**);

void
IgrpCacheUsage (Command*, int, char**);

void
IgrpLkupUsage (Command*, int, char**);

int
HandleVnetAdd (Command*, int, char**);

int
HandleVnetDelete (Command*, int, char**);

int
HandleVnetList (Command*, int, char**);

int
HandleVnetCache (Command*, int, char**);

int
HandleVnetLkup (Command*, int, char**);

int
HandleVnetEdit (Command*, int, char**);

int
VnetAddUsage (Command*, int, char**);

int
VnetDeleteUsage (Command*, int, char**);

int
VnetListUsage (Command*, int, char**);

int
VnetEditUsage (Command*, int, char**);

int
VnetCacheUsage (Command*, int, char**);

int
VnetLkupUsage (Command*, int, char**);

int
VnetEditHandler(VnetEntry *, VnetEntry *);

int
VnetRealmPresent(char *);

int
RealmCloseAndOpenVnetVips(VnetEntry *, VnetEntry *);

int
RealmCloseAndOpenSubnetVips(unsigned long, char *, unsigned long, unsigned long, int);

#include "clicmdhist.h"

int
CliDbQueueEntry(CacheEntry **centryListPtr, int type, void *entry, int sz);

int
CliDbQueueSave(CacheEntry *centryList);

unsigned long GenRealmID(char *);

unsigned int CheckRealmIDDup(unsigned long id);

long 
realmNameToRealmId(char *rmName);

void
ParseCliGetRegidIPStr(char *s, unsigned long *ip, char *name);

int
RealmEditVnetGateway(VnetEntry *newVnetPtr, VnetEntry *oldVnetPtr);

int
GetVnetInfo(char *name, VnetEntry *vnetEntryPtr);

extern int (*_lsAlarmFn)(time_t* valarms, time_t* mrvalarms);
extern int (*_CliRouteLogFn)(RouteNode *routeNode);

void callStats(void);

extern int CachePurge (char *cachename);
extern int HandleCPBEdit (Command *comm, int argc, char **argv);

extern void GkStorePair (void **addrs, char *f, char *v);
extern void IgrpAddCalls (char *name, short int incalls, short int outcalls, 
                          short int totalcalls);
extern void IgrpDeleteCalls (char *name, short int incalls, short int outcalls,                              short int totalcalls);

extern int RealmOpenVip (RealmEntry *rmEntry, VnetEntry *vnetEntryPtr);
extern int RealmChangeVipSig (RealmEntry *rmEntry, unsigned int status);
extern int CheckRSA (struct ifi_info *ifi_head, RealmEntry *newrm, 
                     RealmEntry *oldrm);

extern void HandleCommandUsage (Command *comm, int argc, char **argv);
extern void NetoidEditHelp (Command *comm, int argc, char **argv);
extern void CPBEditHelp (Command *comm, int argc, char **argv);
extern void CREditHelp (Command *comm, int argc, char **argv);
extern void RealmEditHelp (Command *comm, int argc, char **argv);

extern int parse_input (char *infile);

extern int GetNetoidAttrPairs (char *comm, int *argcp, char ***argvp, 
                               void *entry, ClientAttribs *clAttribs);
extern int CloseDBFile (DB dbf);
extern int CacheHandleVpnG (VpnGroupEntry *vpnGroupEntry, int op);
extern int CacheHandleIedge (NetoidInfoEntry *netInfo, int op);
extern int CacheHandleVpn (VpnEntry *vpnEntry, int op);
extern int CacheHandleCR (VpnRouteEntry *routeEntry, int op);
extern int CacheHandleCPB (CallPlanBindEntry *cpbEntry, int op);
extern int CacheHandleTrigger (TriggerEntry *tgEntry, int op);
extern int CacheHandleRealm (RealmEntry *rmEntry, int op);
extern int CacheHandleIgrp (IgrpInfo *igrpEntry, int op);
extern int CacheHandleVnet (VnetEntry *vnetEntry, int op);
extern int UpdateNetoidPorts (char *regid, 
                              int (*cbfn) (void *, void *, void *, void *), 
                              void *arg1, void *arg2, void *arg3);
extern int InheritIedgeGlobals (NetoidInfoEntry *netInfo);
extern int DbOperate (DB db, char *dinfo, int dlen, char *skey, int skeylen, 
                      int op, int verify);
extern void DbResetStats (void);
extern void DbPrintStats (void);

extern int CliGetRegid (char *key, char *regid);
extern int SaveCmdInHistDB (Cmd *cmdp);
extern int read_command (FILE *in, char *buffer, int buflen);
extern int CliDeleteTriggerRoutes (char *trname);
int ResetDatabaseParsing(void);
extern void CliSetupExtFns (void);
extern int CliMain (int argc, char **argv);
char * DbOperToStr(int op);
extern int IsSlave (unsigned int *ipaddr, unsigned int *port);
extern int CliSendToDBMaster (unsigned int ipaddr, unsigned int port, int argc,                               char **argv, char *msg);

#endif /* _cli_h_ */
