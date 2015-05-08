#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "ipc.h"
#include "serverdb.h"
#include "key.h"
#include "srvrlog.h"
#include "thutils.h"
#include "mem.h"
#include "tags.h"
#include "bits.h"
#include "rs.h"

#include "log.h"
#include "dbs.h"

int dbpoolid = -1, dbclassid = -1;
int ndbthreads = 1;	// If this is increased to be more than 1
					// check comment _STATIC_CMD_ below
int clipost = -1;

#define POSTCMD_IEDGE_REG	1
#define POSTCMD_GK_REG		2
#define POSTCMD_CR_ADD		4
#define POSTCMD_CR_EDIT		5


int
DbInit()
{
	int on = 1;
	unsigned char off = 0;

	// create a pool of threads to do database activity
	// these threads will try to update the
	// database instantly, opposed to a global thread

	// We dont want this to be a RT thread
	dbpoolid = ThreadPoolInit("db", ndbthreads, PTHREAD_SCOPE_PROCESS,  -1, 0);
	dbclassid = ThreadAddPoolClass("db", dbpoolid, 0, 0);
	ThreadPoolStart(dbpoolid);

	// Open a socket for the cli commands to be posted
	clipost = socket(AF_INET, SOCK_DGRAM, 0);
	if (setsockopt(clipost, SOL_SOCKET, SO_REUSEADDR,
		(void *)&on, sizeof(on)) < 0) {
		NETERROR(MDB, ("DbInit: setsockopt SO_REUSEADDR - %s\n", 
			strerror(errno)));
	}

	if (setsockopt(clipost, IPPROTO_IP, IP_MULTICAST_LOOP,
		  (void *)&off, sizeof(off)) < 0) {
		NETERROR(MDB, ("DbInit: setsockopt IP_MULTICAST_LOOP - %s\n", 
			strerror(errno)));
	}

	return 0;
}

void *
DbIedgeUpdateWorker(void *arg)
{
	char fn[] = "DbIedgeUpdateWorker():";
	NetoidSNKey *key = (NetoidSNKey *)arg;
	CacheTableInfo cacheInfoEntry;

	if (key == NULL)
	{
		NETERROR(MDB, ("%s key is NULL\n", fn));
		return NULL;
	}

	// Look up the entry in the cache
	if (CacheFind(regCache, key, &cacheInfoEntry, sizeof(cacheInfoEntry)) >= 0)
	{
		// Update the db
		UpdateNetoidDatabase(&cacheInfoEntry.data);
	}

	// free the key
	free(key);

	return NULL;
}

void *
DbIedgeDeleteWorker(void *arg)
{
	char fn[] = "DbIedgeDeleteWorker():";
	NetoidSNKey *key = (NetoidSNKey *)arg;

	if (key == NULL)
	{
		NETERROR(MDB, ("%s key is NULL\n", fn));
		return NULL;
	}

	// Delete from the db
	DeleteNetoidFromDatabase((NetoidInfoEntry *) key);

	// free the key
	free(key);

	return NULL;
}

void
DbScheduleIedgeUpdate(NetoidInfoEntry *netInfo)
{
	char fn[] = "DbScheduleIedgeUpdate():";
	NetoidSNKey *key;

	if (dbpoolid < 0)
	{
		return;
	}

	key = (NetoidSNKey *)malloc(sizeof(NetoidSNKey));
	memcpy(key, netInfo, sizeof(NetoidSNKey));

	if (ThreadDispatch(dbpoolid, dbclassid, DbIedgeUpdateWorker,
		key, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not update entry %s/%lu in the db\n",
			fn, netInfo->regid, netInfo->uport));
	}
}

void
DbScheduleIedgeDelete(NetoidInfoEntry *netInfo)
{
	char fn[] = "DbScheduleIedgeDelete():";
	NetoidSNKey *key;

	if (dbpoolid < 0)
	{
		return;
	}

	key = (NetoidSNKey *)malloc(sizeof(NetoidSNKey));
	memcpy(key, netInfo, sizeof(NetoidSNKey));

	if (ThreadDispatch(dbpoolid, dbclassid, DbIedgeDeleteWorker,
		key, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not delete entry %s/%lu in the db\n",
			fn, netInfo->regid, netInfo->uport));
	}
}

void *
DbRouteUpdateWorker(void *arg)
{
	char fn[] = "DbRouteUpdateWorker():";
	RouteKey *key = (RouteKey *)arg;
	CacheRouteEntry cacheRouteEntry;

	if (dbpoolid < 0)
	{
		return NULL;
	}

	if (key == NULL)
	{
		NETERROR(MDB, ("%s key is NULL\n", fn));
		return NULL;
	}

	// Look up the entry in the cache
	if (CacheFind(cpCache, key, &cacheRouteEntry, sizeof(cacheRouteEntry)) >= 0)
	{
		// Update the db
		if (DbUpdateEntry(CALLROUTE_DB_FILE, DB_eCallRoute, 
			(char*) &cacheRouteEntry.routeEntry,
			sizeof(RouteEntry), 
			(char*) &cacheRouteEntry.routeEntry, sizeof(RouteKey)))
		{
			NETERROR(MDB, ("%s DbUpdateEntry error\n", fn));
		}
	}

	// free the key
	free(key);

	return NULL;
}

void *
DbRouteDeleteWorker(void *arg)
{
	char fn[] = "DbRouteDeleteWorker():";
	RouteKey *key = (RouteKey *)arg;

	if (dbpoolid < 0)
	{
		return NULL;
	}

	if (key == NULL)
	{
		NETERROR(MDB, ("%s key is NULL\n", fn));
		return NULL;
	}

	// Delete from the db
	DeleteCRFromDatabase((VpnRouteEntry *) key); 

	free(key);

	return NULL;
}

void
DbScheduleRouteUpdate(RouteEntry *routeEntry)
{
	char fn[] = "DbScheduleRouteUpdate():";
	RouteKey *key;

	if (dbpoolid < 0)
	{
		return;
	}

	key = (RouteKey *)malloc(sizeof(RouteKey));
	memcpy(key, routeEntry, sizeof(RouteKey));

	if (ThreadDispatch(dbpoolid, dbclassid, DbRouteUpdateWorker,
		key, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not update entry %s in the db\n",
			fn, routeEntry->crname));
	}
}

void
DbScheduleRouteDelete(RouteEntry *routeEntry)
{
	char fn[] = "DbScheduleRouteIedgeDelete():";
	RouteKey *key;

	if (dbpoolid < 0)
	{
		return;
	}

	key = (RouteKey *)malloc(sizeof(RouteKey));
	memcpy(key, routeEntry, sizeof(RouteKey));

	if (ThreadDispatch(dbpoolid, dbclassid, DbRouteDeleteWorker,
		key, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not delete entry %s in the db\n",
			fn, routeEntry->crname));
	}
}

#ifdef _separate_cpb_entry_
void *
DbCPBUpdateWorker(void *arg)
{
	char fn[] = "DbCPBUpdateWorker():";
	CallPlanBindKey *key = (CallPlanBindKey *)arg;
	CacheCPBEntry cacheCPBEntry;

	if (key == NULL)
	{
		NETERROR(MDB, ("%s key is NULL\n", fn));
		return NULL;
	}

	// Look up the entry in the cache
	if (CacheFind(cpbCache, key, &cacheCPBEntry, 
			sizeof(CacheCPBEntry)) >= 0)
	{
		// Update the db
		if (DbUpdateEntry(CALLPLANBIND_DB_FILE, DB_eCallPlanBind, 
			(char*) &cacheCPBEntry.cpbEntry,
			sizeof(CallPlanBindEntry), 
			(char*) &cacheCPBEntry.cpbEntry, sizeof(CallPlanBindKey)))
		{
			NETERROR(MDB, ("%s DbUpdateEntry error\n", fn));
		}
	}

	// free the key
	free(key);

	return NULL;
}

void *
DbCPBDeleteWorker(void *arg)
{
	char fn[] = "DbCPBDeleteWorker():";
	CallPlanBindKey *key = (CallPlanBindKey *)arg;

	if (key == NULL)
	{
		NETERROR(MDB, ("%s key is NULL\n", fn));
		return NULL;
	}

	// Delete from the db
	DeleteCPBFromDatabase(key); 

	free(key);

	return NULL;
}

int
DbScheduleCPBUpdate(CallPlanBindEntry *cpbEntry)
{
	char fn[] = "DbScheduleCPBUpdate():";
	CallPlanBindKey *key;

	key = (CallPlanBindKey *)malloc(sizeof(CallPlanBindKey));
	memcpy(key, routeEntry, sizeof(CallPlanBindKey));

	if (ThreadDispatch(dbpoolid, dbclassid, DbCPBUpdateWorker,
		key, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not update entry %s/%s in the db\n",
			fn, cpbEntry->cpname, cpbEntry->crname));
	}
}

int
DbScheduleCPBDelete(CallPlanBindEntry *cpbEntry)
{
	char fn[] = "DbScheduleCPBDelete():";
	CallPlanBindKey *key;

	key = (CallPlanBindKey *)malloc(sizeof(CallPlanBindKey));
	memcpy(key, cpbEntry, sizeof(CallPlanBindKey));

	if (ThreadDispatch(dbpoolid, dbclassid, DbCPBDeleteWorker,
		key, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not delete entry %s in the db\n",
			fn, cpbEntry->cpname, cpbEntry->crname));
	}
}
#endif

// ROutines to allow posting of various cli commands

// cli iedge reg command
typedef struct
{
	NetoidSNKey key;
	char crname[CALLPLAN_ATTR_LEN];
	char tags[TAGH_LEN];
	int command;

} PostCmd;

#define CMDSTR	(cmdstr+len)
#define CMDMAX	(cmdlen-len)

int
GisPostCliFormatIedgeRegCmd(PostCmd *cmd, char *tags, 
	char *cmdstr, int cmdlen, int len)
{
	char fn[] = "GisPostCliFormatIedgeRegCmd():";
	CacheTableInfo cacheInfoEntry;
	NetoidInfoEntry *netInfo;
	struct in_addr in;
        char buf[INET_ADDRSTRLEN];
	// Look up the entry in the cache
	if (CacheFind(regCache, &cmd->key, &cacheInfoEntry, sizeof(cacheInfoEntry)) < 0)
	{
		NETDEBUG(MDB, NETLOG_DEBUG1,
			("%s Could not find iedge %s/%d\n",
			fn, cmd->key.regid, cmd->key.uport));

		goto _return;
	}

	netInfo = &cacheInfoEntry.data;

	len += snprintf(CMDSTR, CMDMAX, "iedge reg \"%s\" %d ",
		cmd->key.regid, cmd->key.uport);

	if (BITA_TEST(tags, TAG_IPADDRESS))
	{
		in.s_addr = htonl(netInfo->ipaddress.l);
		len += snprintf(CMDSTR, CMDMAX, "ip %s ", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
	}

	if (BITA_TEST(tags, TAG_H323SIGPT))
	{
		len += snprintf(CMDSTR, CMDMAX, "q931port %d ",
					netInfo->callsigport);
	}

	if (BITA_TEST(tags, TAG_GATEWAY))
	{
		len += snprintf(CMDSTR, CMDMAX, "gateway %s ",
				BIT_TEST(netInfo->cap, CAP_IGATEWAY)?"enable":"disable");
	}

	if (BITA_TEST(tags, TAG_SIP))
	{
		len += snprintf(CMDSTR, CMDMAX, "sip %s ",
				BIT_TEST(netInfo->cap, CAP_SIP)?"enable":"disable");
	}

	if (BITA_TEST(tags, TAG_H323))
	{
		len += snprintf(CMDSTR, CMDMAX, "h323 %s ",
				BIT_TEST(netInfo->cap, CAP_H323)?"enable":"disable");
	}

	if (BITA_TEST(tags, TAG_DND))
	{
		len += snprintf(CMDSTR, CMDMAX, "dnd %s ",
				(netInfo->stateFlags& CL_DND)?"enable":"disable");
	}

	// Use the same TAG, as we will bundle up all three
	// pieces of info using the same tag
	if (BITA_TEST(tags, TAG_NCALLS))
	{
		len += snprintf(CMDSTR, CMDMAX, "ncalls %d ",
					IedgeCalls(netInfo));
		len += snprintf(CMDSTR, CMDMAX, "nincalls %d ",
					IedgeInCalls(netInfo));
		len += snprintf(CMDSTR, CMDMAX, "noutcalls %d ",
					IedgeOutCalls(netInfo));
	}

	if (BITA_TEST(tags, TAG_CONTACT))
	{
		len += snprintf(CMDSTR, CMDMAX, "contact \"%s\" ",
					netInfo->contact);
	}

	if (BITA_TEST(tags, TAG_REGSTATUS))
	{
		len += snprintf(CMDSTR, CMDMAX, "reg %s ",
				(netInfo->stateFlags& CL_ACTIVE)?"active":"inactive");
	}

_return:
	return len;
}

int
GisPostCliFormatGkRegCmd(PostCmd *cmd, char *tags, 
	char *cmdstr, int cmdlen, int len)
{
	char fn[] = "GisPostCliFormatGkRegCmd():";
	CacheGkInfo *cacheGkInfo = NULL, cacheGkInfoEntry;

	// Look up the entry in the cache
	if (CacheFind(gkCache, &cmd->key, &cacheGkInfoEntry, 
			sizeof(cacheGkInfoEntry)) < 0)
	{
		NETDEBUG(MDB, NETLOG_DEBUG1,
			("%s Could not find gk %s/%d\n",
			fn, cmd->key.regid, cmd->key.uport));

		goto _return;
	}

	cacheGkInfo = &cacheGkInfoEntry;

	len += snprintf(CMDSTR, CMDMAX, "gk reg \"%s\" %d ",
		cmd->key.regid, cmd->key.uport);

	if (BITA_TEST(tags, TAG_EPID))
	{
		len += snprintf(CMDSTR, CMDMAX, "epid \"");
		len += hex2chr(CMDSTR, CMDMAX, cacheGkInfo->endpointIDString, 
					cacheGkInfo->endpointIDLen);
		len += snprintf(CMDSTR, CMDMAX, "\" ");
	}

	if (BITA_TEST(tags, TAG_GKREGTTL))
	{
		len += snprintf(CMDSTR, CMDMAX, "ttl %d ",
					cacheGkInfo->regttl);
	}

	if (BITA_TEST(tags, TAG_GKFLAGS))
	{
		len += snprintf(CMDSTR, CMDMAX, "flags %d ",
					cacheGkInfo->flags);
	}

	if (BITA_TEST(tags, TAG_REGSTATUS))
	{
		len += snprintf(CMDSTR, CMDMAX, "reg %s ",
				(cacheGkInfo->regState == GKREG_REGISTERED)?"active":"inactive");
	}

_return:
	return len;
}

int
GisPostCliFormatCRCmd(PostCmd *cmd, char *tags, 
	char *cmdstr, int cmdlen, int len)
{
	char fn[] = "GisPostCliFormatCRCmd():";
	CacheRouteEntry *route = NULL, cacheRouteEntry;

	// Look up the entry in the cache
	if (CacheFind(cpCache, cmd->crname, &cacheRouteEntry, 
			sizeof(CacheRouteEntry)) < 0)
	{
		NETDEBUG(MDB, NETLOG_DEBUG1,
			("%s Could not find route %s\n", fn, cmd->crname));

		goto _return;
	}

	route = &cacheRouteEntry;

	if (cmd->command == POSTCMD_CR_ADD)
	{
		len += snprintf(CMDSTR, CMDMAX, "cr add \"%s\" ",
			cmd->crname);
	}
	else if(cmd->command == POSTCMD_CR_EDIT)
	{
		len += snprintf(CMDSTR, CMDMAX, "cr edit \"%s\" ",
			cmd->crname);
	}

	if (!BITA_TEST(tags, TAG_CRMRU))
	{
		len += snprintf(CMDSTR, CMDMAX, "dest \"%s\" ",
			route->routeEntry.dest);
		len += snprintf(CMDSTR, CMDMAX, "destlen %d ",
			route->routeEntry.destlen);
		len += snprintf(CMDSTR, CMDMAX, "prefix \"%s\" ",
			route->routeEntry.prefix);
		len += snprintf(CMDSTR, CMDMAX, "src \"%s\" ",
			route->routeEntry.src);
		len += snprintf(CMDSTR, CMDMAX, "srclen %d ",
			route->routeEntry.srclen);
		len += snprintf(CMDSTR, CMDMAX, "srcprefix \"%s\" ",
			route->routeEntry.srcprefix);
		len += snprintf(CMDSTR, CMDMAX, "calltype \"%s\" ",
			RouteFlagsString(route->routeEntry.crflags & (CRF_CALLORIGIN|CRF_CALLDEST|CRF_TRANSIT)));
		len += snprintf(CMDSTR, CMDMAX, "type \"%s\" ",
			(route->routeEntry.crflags & CRF_REJECT)? "reject":"normal");
		len += snprintf(CMDSTR, CMDMAX, "dnisdefault \"%s\" ",
			(route->routeEntry.crflags & CRF_DNISDEFAULT)? "enable":"disable");
		len += snprintf(CMDSTR, CMDMAX, "template \"%s\" ",
			(route->routeEntry.crflags & CRF_TEMPLATE)? "enable":"disable");
		len += snprintf(CMDSTR, CMDMAX, "template \"%s\" ",
			(route->routeEntry.crflags & CRF_STICKY)? "enable":"disable");
		len += snprintf(CMDSTR, CMDMAX, "cpname \"%s\" ",
			route->routeEntry.cpname);
	}

_return:
	return len;
}

void *
GisPostCliCmdWorker(void *arg)
{
	char fn[] = "GisPostCliCmdWorker():";
	PostCmd *cmd = (PostCmd *)arg;
	char 	*tags;
	static char	cmdstr[512];	// _STATIC_CMD_ make this non-static, if
								// more than one db thread is used
	int		cmdlen = 512, initlen = sizeof(Cmd)+sizeof(RSPkt), len;
	Cmd 	*cmdp; 
	RSPkt	*pktp;
	struct sockaddr_in addr;
	time_t 	t = time(NULL);
	pid_t 	p = getpid();

	len = initlen;

	if (cmd == NULL)
	{
		NETERROR(MDB, ("%s cmd is NULL\n", fn));
		return NULL;
	}

	if (RSDConfig == 0)
	{
		NETDEBUG(MDB, NETLOG_DEBUG4,
			("%s No need to post command, RSD no configured\n",
			fn));

		goto _return;
	}

	tags = cmd->tags;

	if (tags == NULL)
	{
		NETERROR(MDB, ("%s tags is NULL\n", fn));

		goto _return;
	}

	switch (cmd->command)
	{
	case POSTCMD_IEDGE_REG:
		len = GisPostCliFormatIedgeRegCmd(cmd, tags, cmdstr, cmdlen, len);
		break;
	case POSTCMD_GK_REG:
		len = GisPostCliFormatGkRegCmd(cmd, tags, cmdstr, cmdlen, len);
		break;
	case POSTCMD_CR_ADD:
	case POSTCMD_CR_EDIT:
		len = GisPostCliFormatCRCmd(cmd, tags, cmdstr, cmdlen, len);
		break;
	default:
		NETERROR(MDB, ("%s undefined type %d\n", fn, cmd->command));
		break;
	}

	// Fill up the command structure
	cmdp = (Cmd *)(cmdstr + sizeof(RSPkt));
	cmdp->cmdtyp = htonl(CMD_CLI);
	cmdp->cmdlen = htonl(len - sizeof(RSPkt));
	cmdp->cmdseq = htonl(-1);
	cmdp->cmdrval = htonl(0);
	cmdp->cmdact =  htonl(0);
	cmdp->cmdtim =  htonl(t);
	cmdp->cmdpid =  htonl(p);

	// Fill up the Pkt Header
	pktp = (RSPkt *)cmdstr;
	pktp->type = htonl(PKT_REG | CLI_CMDS);
	pktp->datalen = cmdp->cmdlen; 		/* Only one cmd in this pkt */

	NETDEBUG(MDB, NETLOG_DEBUG1, ("%s pkttype = %x, pktlen = %x,\n"
		"cmdtyp = %x, cmdlen = %x, cmdseq = %x, cmdrval = %x, cmdact = %x\n"
		"cmdpid = %x, cmdtim = %x, Command %s\n", fn, ntohl(pktp->type), 
		ntohl(pktp->datalen), ntohl(cmdp->cmdtyp), ntohl(cmdp->cmdlen), 
		ntohl(cmdp->cmdseq), ntohl(cmdp->cmdrval), ntohl(cmdp->cmdact), 
		ntohl(cmdp->cmdpid), ntohl(cmdp->cmdtim), cmdstr+initlen));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(rs_mcast_addr);
	addr.sin_port = htons(atoi(rs_port));

	if (sendto(clipost, cmdstr, len, 0, (struct sockaddr *)&addr,
				sizeof(addr)) <  len)
	{
		NETERROR(MDB, ("%s Error sending to network\n", fn));
	}

_return:
	free(cmd);

	return NULL;
}

int
GisPostCliIedgeRegCmd(char *regid, int uport, char *tags)
{
	char fn[] = "GisPostCliIedgeRegCmd():";

	return GisPostCliCmd(POSTCMD_IEDGE_REG, regid, uport, tags);
}

int
GisPostCliGkRegCmd(char *regid, int uport, char *tags)
{
	char fn[] = "GisPostCliGkRegCmd():";

	return GisPostCliCmd(POSTCMD_GK_REG, regid, uport, tags);
}

int
GisPostCliCRAddCmd(char *crname, char *tags)
{
	char fn[] = "GisPostCliCRAddCmd():";

	GisPostCliCRCmd(POSTCMD_CR_ADD, crname, tags);

	return 0;
}

int
GisPostCliCmd(int command, char *regid, int uport, char *tags)
{
	char fn[] = "GisPostCliCmd():";
	PostCmd *cmd = (PostCmd *)malloc(sizeof(PostCmd));

	cmd->command = command;
	memcpy(cmd->key.regid, regid, REG_ID_LEN);
	cmd->key.uport = uport;
	memcpy(cmd->tags, tags, TAGH_LEN);

	// Now queue it.
	if (ThreadDispatch(dbpoolid, dbclassid, GisPostCliCmdWorker,
		cmd, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not post command for %s/%d in the db\n",
			fn, regid, uport));
	}

	return 0;
}

int
GisPostCliCRCmd(int command, char *crname, char *tags)
{
	char fn[] = "GisPostCliCRCmd():";
	PostCmd *cmd = (PostCmd *)malloc(sizeof(PostCmd));

	cmd->command = command;
	memcpy(cmd->crname, crname, CALLPLAN_ATTR_LEN);
	memcpy(cmd->tags, tags, TAGH_LEN);

	// Now queue it.
	if (ThreadDispatch(dbpoolid, dbclassid, GisPostCliCmdWorker,
		cmd, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not post command for %s in the db\n",
			fn, crname));
	}

	return 0;
}

