#ifdef USE_RPC

#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sockio.h>
#include <sys/filio.h>
#include <sys/wait.h>
#include <netdb.h>

#if SOLARIS_REL == 7
	#include <addrinfo.h>
#endif

#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sched.h>
#include <rpc/rpc.h>
#include <rpc/rpcent.h>
#include <libgen.h>
#include <memory.h>
#include <netconfig.h>
#include <sys/resource.h>

// iServer include files

#include "spversion.h"
#include "generic.h"
#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "serverdb.h"
#include "key.h"

#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "db.h"
#include "timer.h"
#include "connapi.h"
#include "sconfig.h"
#include "xmltags.h"
#include "gis.h"
#include "cli_rpc.h"

pthread_t gis_cacherpc_thread;
void * gis_cacherpc_init(void * args);
extern void cli_cache_handler_prog_1(struct svc_req *rqstp, register SVCXPRT *transp);
static SVCXPRT	*transp;

void
start_cacherpc()
{
	pthread_attr_t	thread_attr;
	int32_t			status;

	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr,
								 PTHREAD_CREATE_DETACHED );
	pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_SYSTEM );

	if ( (status = pthread_create(	&gis_cacherpc_thread,
									&thread_attr,
									gis_cacherpc_init,
									(void*) NULL) ) != 0 )
	{
		NETERROR(	MDEF,
					("start_cacherpc() : error failed to start "
					"gis_cacherpc_init() - status %d\n",
					status ));
		return;
	}

	return;
}



void *
gis_cacherpc_init(void * args)
{
	struct netconfig *	nconf = NULL;
	void *	handlep = (void*) NULL;

	(void) sigset(SIGPIPE, SIG_IGN);

	handlep = setnetconfig();

	if ( (nconf = getnetconfigent("tcp")) == NULL)
	{
		("gis_cacherpc_init(): cannot get transport info");
		return( (void*) NULL );
	}

	if ( ( transp = svc_tp_create(	cli_cache_handler_prog_1,
									CLI_CACHE_HANDLER_PROG,
									CLI_CACHE_HANDLER_VERS,
									nconf )) == (SVCXPRT *)	NULL )
	{
		NETERROR(MINIT, ("gis_cacherpc_init(): unable to create ( CLI_CACHE_HANDLER_PROG, CLI_CACHE_HANDLER_VERS )"));
		return( (void*) NULL );
	}

	freenetconfigent(nconf);

	if ( endnetconfig(handlep) != 0 )
	{
		NETERROR(MINIT, ("gis_cacherpc_init(): endnetconfig() call failed"));
		return( (void*) NULL );
	}

	if (!svc_reg( transp, CLI_CACHE_HANDLER_PROG, CLI_CACHE_HANDLER_VERS, cli_cache_handler_prog_1, 0))
	{
		NETERROR(MINIT, ("gis_cacherpc_init(): unable to register ( CLI_CACHE_HANDLER_PROG, CLI_CACHE_HANDLER_VERS )"));

		return( (void*) NULL );
	}

	svc_run();

	NETERROR(MINIT, ("daemon : svc_run returned"));

	return( (void*) NULL );
}

void
gis_cacherpc_shutdown( void )
{
	return;
}


int
CacheHandleIedge(NetoidInfoEntry *netInfo, int op)
{
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		// license_release(1);
		rc = DeleteNetoidFromCache(netInfo);
	}

	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		if (op == CLIOP_ADD)
		{
#if 0
			if (license_allocate(1))
			{
				CLIPRINTF((stdout,"could not obtain license\n"));
				return -xleNoLicense;
			}
#endif
		}

		rc = UpdateNetoidInCache(netInfo);
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

int
CacheHandleVpnG(VpnGroupEntry *vpnGroupEntry, int op)
{
	CacheVpnGEntry *cacheVpnGEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		cacheVpnGEntry = CacheDelete(vpnGCache, vpnGroupEntry);
	
		CacheReleaseLocks(vpnGCache);

		if (cacheVpnGEntry)
		{
			CFree(vpnGCache)(cacheVpnGEntry);
		}

		rc = 0;
	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

		cacheVpnGEntry = CacheGet(vpnGCache, vpnGroupEntry->vpnGroup);

		if (cacheVpnGEntry)
		{
			/* Compare the two */
			if (memcmp(&cacheVpnGEntry->vpnGroupEntry, 
				vpnGroupEntry,
				sizeof(VpnGroupEntry)))
			{
				/* the two are different */
				memcpy(&cacheVpnGEntry->vpnGroupEntry,
					vpnGroupEntry, sizeof(VpnGroupEntry));
				/* Do the update */
				update = 1;
			}
		}
		else
		{

			cacheVpnGEntry = (CacheVpnGEntry *)
					CacheInsert(vpnGCache, 
					CacheDupVpnGEntry(vpnGroupEntry));
			update = 1;
		}

		CacheReleaseLocks(vpnGCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

int
CacheHandleVpn(VpnEntry *vpnEntry, int op)
{
	CacheVpnEntry *cacheVpnEntry;
	int update = 0;
	int rc = -1;
	
	if (lsMem == NULL)
	{
		return 0;
	}

	if (strlen(vpnEntry->vpnId) == 0)
	{
		 return 0;
	}

	/* First Delete the entry, if its there */
	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		/* First Delete the entry, if its there */
		cacheVpnEntry = CacheDelete(vpnCache, vpnEntry);
	
		CacheReleaseLocks(vpnCache);

		if (cacheVpnEntry)
		{
			CFree(vpnCache)(cacheVpnEntry);
		}

		rc = 0;
	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		cacheVpnEntry = CacheGet(vpnCache, vpnEntry->vpnName);

		if (cacheVpnEntry)
		{
			/* Compare the two */
			if (memcmp(&cacheVpnEntry->vpnEntry, vpnEntry,
				sizeof(VpnEntry)))
			{
				/* the two are different */
				memcpy(&cacheVpnEntry->vpnEntry,
					vpnEntry, sizeof(VpnEntry));
				/* Do the update */
				update = 1;
			}
		}
		else
		{

			cacheVpnEntry = (CacheVpnEntry *)
					CacheInsert(vpnCache, 
						CacheDupVpnEntry(vpnEntry));
			update = 1;
		}

		CacheReleaseLocks(vpnCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleCR(VpnRouteEntry *routeEntry, int op)
{
	CacheVpnRouteEntry *cacheRouteEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		//cacheRouteEntry = CacheDelete(cpCache, routeEntry);
		DeleteRoute(routeEntry);
	
		CacheReleaseLocks(cpCache);

		rc = 0;
	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

		DeleteRoute(routeEntry);
		AddRoute(CacheDupVpnRouteEntry(routeEntry));

		CacheReleaseLocks(cpCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleCP(CallPlanEntry *cpEntry, int op)
{
	CacheEntry *ce = NULL;
	CacheCPEntry *cacheCPEntry = NULL;
	int rc = 0;

	if (lsMem == NULL)
	{
		return 0;
	}

	return rc;

}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleCPB(CallPlanBindEntry *cpbEntry, int op)
{
	CacheCPBEntry *cacheCPBEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		//cacheCPBEntry = CacheDelete(cpbCache, cpbEntry);
		DeleteCPB(cpbEntry);
	
		CacheReleaseLocks(cpbCache);

		rc = 0;

	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		DeleteCPB(cpbEntry);
		AddCPB(CacheDupCPBEntry(cpbEntry));

		CacheReleaseLocks(cpbCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}


/* return >=0, if no error, -1 if there is an error */
/* Note that because we have two caches and only one 
 * database, the entry my get inserted into the ls cache,
 * and its update list for redundant iserver, and fail the
 * vpns cache addition. If this happens, the entry will
 * fail the db addition, while on teh redundant iserver,
 * it will get added into the database. To prevent this,
 * we should return proper error codes from this
 * function, telling the caller what to do - future fix.
 */
int
UpdateNetoidInCache(NetoidInfoEntry *netInfo)
{
   	char fn[] = "UpdateNetoidInCache():";
   	CacheTableInfo *info, infoCopy;
   	NetoidSNKey key = {0};
   	int shmId;
   	int error = 1, found = 0;

   	strncpy(key.regid, netInfo->regid, REG_ID_LEN);
   	key.uport = netInfo->uport;

	shmId = CacheAttach();

   	if (shmId != -1)
   	{
		/* The iedge has to be added into each cache,
		 * and if there is a duplicate present for
		 * any iedge attribute, it is to be reported
		 */
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		info = GetDuplicateIedge(netInfo);

		if (info)
		{
#if 0
			CLIPRINTF((stdout, "duplicate iedge in cache:\n"));
			CLIPRINTF((stdout, "reg id %s, uport %d\n",
				info->data.regid, info->data.uport));
#endif

			error =  -xleExists;
			goto _return;
		}
	
		/* Otherwise, delete the iedge first and then add
		 * it back again.
		 */
	  	if (info = DeleteIedge(netInfo))
	  	{
			// update the db with this entry
			UpdateIedgeDynamicInfo(netInfo, &info->data);
			found = 1;

			SHM_Free(info);
	  	}

		info = CacheDupInfoEntry(netInfo);

		if (info)
		{
			ResetIedgeFields(info, 0);

			if (!found)
			{
				UpdateIedgeDynamicInfo(&info->data, NULL);
			}

			if (AddIedge(info) < 0)
			{
#if 0
				CLIPRINTF((stdout, 
					"iedge duplicate found. Cache is corrupted\n"));
#endif

				DeleteIedge(netInfo);

				SHM_Free(info);

				error = -xleOpNoPerm;
				goto _return;
			}

#if 0
			CLIPRINTF((stdout, "iedge added to cache\n"));
#endif
		}
		else
		{
#if 0
			CLIPRINTF((stdout, "unable to add iedge to cache\n"));
#endif
			error = -xleOpNoPerm;
			goto _return;
		}

_return:
		CacheReleaseLocks(regCache);

		CacheDetach();
	}

    return error;
}

int
DeleteNetoidFromCache(InfoEntry *netInfo)
{
    char fn[] = "DeleteNetoidFromCache():";
    CacheTableInfo *info, cacheInfoEntry;
    int shmId;

	shmId = CacheAttach();

    if (shmId != -1)
    {
		
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		if (info = DeleteIedge(netInfo))
	  	{
#if 0
	  	    CLIPRINTF((stdout, "Deleted entry from cache\n"));
#endif
	   	}
		else
		{
#if 0
	  	    CLIPRINTF((stdout, "No such entry in cache\n"));
#endif
		}

		CacheReleaseLocks(regCache);

#if 0
		if (info && (MemGetRwLock(&lsMem->updatemutex, 
			LOCK_WRITE, LOCK_BLOCK) == AL_OK))
		{
			CacheEntry *ce;
			
			info->state |= CACHE_NEEDS_DELETE;
			info->data.mTime = time(0);
			ce = (CacheEntry *)SHM_Malloc(sizeof(CacheEntry));
			if (ce)
			{
				ce->entry = info;
				ce->type = CACHE_INFO_ENTRY;
				ListInsert(lsMem->updateList.prev, ce);
			}
			else
			{
				CLIPRINTF((stdout, "SHM: Out of memory\n"));
			}
			MemReleaseRwLock(&lsMem->updatemutex);
		}
		else
#endif
		if (info)
		{
			CFree(regCache)(info);
		}
		
		CacheDetach();
	}

    return 1;
}

int
InheritIedgeGlobals(NetoidInfoEntry *netInfo)
{
   	int shmId;
	CacheTableInfo *cacheInfo = NULL;
	
	shmId = CacheAttach();

   	if (shmId != -1)
   	{
		/* The iedge has to be added into each cache,
		 * and if there is a duplicate present for
		 * any iedge attribute, it is to be reported
		 */
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		cacheInfo = CacheGet(regidCache, netInfo);

		if (cacheInfo != NULL)
		{
			netInfo->ipaddress.l = cacheInfo->data.ipaddress.l;
			BIT_COPY(netInfo->sflags, ISSET_IPADDRESS, 
				cacheInfo->data.sflags, ISSET_IPADDRESS);
			netInfo->stateFlags |= (cacheInfo->data.stateFlags & CL_STATIC);
			netInfo->callsigport = cacheInfo->data.callsigport;
			netInfo->rasip = cacheInfo->data.rasip;
			netInfo->rasport = cacheInfo->data.rasport;
		}

		CacheReleaseLocks(regCache);

		CacheDetach();
	}

	return 0;
}

cache_t
mapCacheType2Cache(int cacheType)
{
	cache_t		cache = 0;
	switch (cacheType)
	{
		case REG_CACHE:
				cache = regCache;
				break;
		case PHONE_CACHE:
				cache = phoneCache;
				break;
		case EMAIL_CACHE:
				cache = emailCache;
				break;
		case IP_CACHE:
				cache = ipCache;
				break;
		case GK_CACHE:
				cache = gkCache;
				break;
		case URI_CACHE:
				cache = uriCache;
				break;
		case H323ID_CACHE:
				cache = h323idCache;
				break;
		case SUBNET_CACHE:
				cache = subnetCache;
				break;
		case CRID_CACHE:
				cache = cridCache;
				break;
		case TG_CACHE:
				cache = tgCache;
				break;
		case GW_CACHE:
				cache = gwCache;
				break;

	}
	return (cache);
}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleTrigger(TriggerEntry *tgEntry, int op)
{
	CacheTriggerEntry *cacheTgEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		DeleteTrigger(tgEntry);
	
		CacheReleaseLocks(triggerCache);

		rc = 0;

	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);

		DeleteTrigger(tgEntry);
		AddTrigger(CacheDupTriggerEntry(tgEntry));

		CacheReleaseLocks(triggerCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

int
CacheHandleRealm(RealmEntry *rmEntry, int op)
{
	CacheRealmEntry *cachermEntry = NULL;
	CacheRealmEntry *newrmEntry = NULL;
	CacheRealmEntry *tmprmEntry = NULL;
	int		rc = -1;

    switch (op)
    {
        case CLIOP_DELETE:
                CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);
		/* Delete the entry, if its there */
                rc = DeleteRealm(&rmEntry->realmId);
                CacheReleaseLocks(realmCache);

                break;
	
#if 0				
        case CLIOP_ADD:
                CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

                DeleteRealm(&rmEntry->realmId);
				newrmEntry = CacheDupRealmEntry(rmEntry);
				/* init socketId to erroneous value */
				newrmEntry->socketId = -1; 
                rc = AddRealm(newrmEntry);

                CacheReleaseLocks(realmCache);

	            break;
#endif

        case CLIOP_ADD:
        case CLIOP_REPLACE:
                CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

				tmprmEntry = CacheGet(realmCache, &rmEntry->realmId);
				if (tmprmEntry == NULL)
				{
					tmprmEntry = CacheGet(rsaCache, &rmEntry->rsa);
				}

				newrmEntry = CacheDupRealmEntry(rmEntry);
				if (newrmEntry && tmprmEntry)
				{
					newrmEntry->socketId = tmprmEntry->socketId;
				}

                DeleteRealm(&rmEntry->realmId);
                rc = AddRealm(newrmEntry);

                CacheReleaseLocks(realmCache);

	            break;
	}

	return rc;
}

int
CacheHandleIgrp(IgrpInfo *igrpEntry, int op)
{
	CacheIgrpInfo  *cacheEntry = NULL;
	CacheIgrpInfo  *currCacheEntry = NULL;
	CacheIgrpInfo  *newCacheEntry = NULL;
	int     rc = 0;

    switch (op)
    {
        case CLIOP_DELETE:
                CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);
		/* Delete the entry, if its there */
                rc = DeleteIgrp(igrpEntry->igrpName);
                CacheReleaseLocks(igrpCache);

                break;
	
        case CLIOP_ADD:
                CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

                DeleteIgrp(igrpEntry->igrpName);
                rc = AddIgrp(CacheDupIgrpInfo(igrpEntry));

                CacheReleaseLocks(igrpCache);

	            break;

		case CLIOP_REPLACE:
			CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

			/* Get the existing Cache entry */
			currCacheEntry = CacheDelete(igrpCache, igrpEntry->igrpName);
			/* update calls stats from current entry */
			UpdateIgrpDynamicInfo(igrpEntry, &currCacheEntry->igrp);
			newCacheEntry = CacheDupIgrpInfo(igrpEntry);
			rc = AddIgrp(newCacheEntry);

			CacheReleaseLocks(igrpCache);

			CFree(igrpCache)(currCacheEntry);
			break;
	}

	return rc;
}


int
IgrpAddCalls(char *name, unsigned int incalls, unsigned int outcalls,
	   					 unsigned int totalcalls)	
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
		return (2);
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

	return 1;
}

int
IgrpDeleteCalls(char *name, unsigned int incalls, unsigned int outcalls,
	   					 unsigned int totalcalls)	
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
		/* CLIPRINTF((stdout, "No Igrp entry exists for %s\n", name)); */
		return (2);
	}

	IgrpInCalls(igrp) -= incalls;
	IgrpOutCalls(igrp) -= outcalls;
	IgrpCallsTotal(igrp) -= totalcalls;

	return 1;
}

int
CacheHandleVnet(Vnet *vnetEntry, int op)
{
	CacheVnetEntry  *cacheEntry = NULL;
	CacheVnetEntry  *currCacheEntry = NULL;
	CacheVnetEntry  *newCacheEntry = NULL;
	int     rc = 0;

    switch (op)
    {
        case CLIOP_DELETE:
                CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);
                rc = DeleteVnet(vnetEntry->vnetName);
                CacheReleaseLocks(vnetCache);
                break;
	
        case CLIOP_ADD:
                CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);
                DeleteVnet(vnetEntry->vnetName);
                rc = AddVnet(CacheDupVnetEntry(vnetEntry));
                CacheReleaseLocks(vnetCache);
                break;

        case CLIOP_REPLACE:
                CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);
                /* Get the existing Cache entry */
                currCacheEntry = CacheDelete(vnetCache, vnetEntry->vnetName);
                newCacheEntry = CacheDupVnetEntry(vnetEntry);
                rc = AddVnet(newCacheEntry);

                CacheReleaseLocks(vnetCache);

                CFree(vnetCache)(currCacheEntry);
                break;
	}

	return rc;
}

/*
 * Checks if the RSA is a unique IP address for all existing realms
 * and checks if the ipaddress is unique for all logical interfaces
 * on host
 */
int
CheckRSA(struct ifi_info *ifi_head, RealmEntry *newrm, RealmEntry *oldrm, char *errstr)
{
	struct ifi_info  *ifp = NULL, *ifi_head2 = NULL;
	unsigned char flag=0;;

	if ( CacheAttach() > 0)
	{
		CacheHandleRealm(oldrm, CLIOP_DELETE);
		/* Is this new rsa unique across realms */
		if (CacheHandleRealm(newrm, CLIOP_ADD)< 0)
		{
			sprintf(errstr, "RSA duplication:RSA [%s] on existing realm\n", 
													ULIPtostring(newrm->rsa));
			CacheHandleRealm(oldrm, CLIOP_ADD);
			CacheDetach();
			goto _return;
		}
		CacheDetach();
	}
	else
	{
		/* Can't verify the uniqueness of RSA */
		goto _return;
	}

	/* Is the new assigned IP unique across all logical interface
	 *  (not just realms)
	 */
	if (!ifi_head)
	{
		ifi_head2 = initIfs2(1);
		flag =1;
	}
	else
	{
		ifi_head2 = ifi_head;
	}

	if(ifp = matchIf(ifi_head2, htonl(newrm->rsa)))
	{
		if (strchr(ifp->ifi_name, ':'))
		{
			sprintf(errstr, "IP address duplication:RSA [%s] exists on interface [%s]\n", 
					ULIPtostring(newrm->rsa), ifp->ifi_name);

			goto _return;
		}
	}

	if (flag)
	{
		freeIfs(ifi_head2);
	}

	return 0;

_return:
	if (flag)
	{
		freeIfs(ifi_head2);
	}
	return -1;
}

int
CachePurge(char *cachename)
{
	char fn[] = "CachePurge():";
	int rc = xleOk;

	if (!strcmp(cachename, "iedge"))
	{
		// lock the cache
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		IedgeCacheDestroyData(lsMem);
		IedgeCacheInstantiate(lsMem);
	
		if (lsMem)
		{
			nlm_setconfigport(0);
			nlm_initConfigPort();
		}

		CacheReleaseLocks(regCache);
	}

	if (!strcmp(cachename, "vpn"))
	{
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		VpnCacheDestroyData(lsMem);
		VpnCacheInstantiate(lsMem);
	
		CacheReleaseLocks(vpnCache);
	}

	if (!strcmp(cachename, "vpng"))
	{
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);
		VpnGCacheDestroyData(lsMem);
		VpnGCacheInstantiate(lsMem);
	
		CacheReleaseLocks(vpnGCache);
	}

	if (!strcmp(cachename, "cp"))
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

		CPCacheDestroyData(lsMem);
		CPCacheInstantiate(lsMem);
	
		CacheReleaseLocks(cpCache);
	}

	if (!strcmp(cachename, "cpb"))
	{
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		CPBCacheDestroyData(lsMem);
		CPBCacheInstantiate(lsMem);
	
		CacheReleaseLocks(cpbCache);
	}

    if (!strcmp(cachename, "realm"))
    {
        CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

        RealmCacheDestroyData(lsMem);
        RealmCacheInstantiate(lsMem);

        CacheReleaseLocks(realmCache);
    }

    if (!strcmp(cachename, "igrp"))
    {
        CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

        IgrpCacheDestroyData(lsMem);
        IgrpCacheInstantiate(lsMem);

        CacheReleaseLocks(igrpCache);
    }

    if (!strcmp(cachename, "vnet"))
    {
        CacheGetLocks(vnet, LOCK_WRITE, LOCK_BLOCK);

        VnetCacheDestroyData(lsMem);
        VnetCacheInstantiate(lsMem);

        CacheReleaseLocks(vnetCache);
    }
	return rc;
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

			AssignIedgeSipUri(&info->data, vpnEntry);

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
#endif
