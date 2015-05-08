#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#ifdef SUNOS
#include <sys/sockio.h>
#include <sys/filio.h>
#else
#include <linux/sockios.h>
#endif
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>

#include <signal.h>
#include <sys/wait.h>
#include <license.h>
#include "poll.h"

#include "spversion.h"

#include "generic.h"
#include "bits.h"
#include "ipc.h"
#include "serverdb.h"
#include "srvrlog.h"
#include "key.h"
#include "mem.h"
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "entry.h"
#include "pef.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"
#include "timer.h"
#include "fdsets.h"
#include "db.h"
#include "connapi.h"
#include "shm.h"
#include "shmapp.h"
#include "xmltags.h"
#include "sconfig.h"

#include "gis.h"
#include "lrq.h"
#include <malloc.h>
#include "uh323inc.h"

#include "gk.h"
#include "iwfsmproto.h"
#include "radclient.h"
#include "licenseIf.h"
#include "log.h"

// MUST remove both mappings: 
// Conf -> call
// call -> conf
int
GisDeleteCallFromConf(char *callID, char *confID)
{
	char fn[] = "GisDeleteCallFromConf():";
	ConfHandle *confHandle;
	CallHandle *callHandle;
	int i;
	char	cid[CALL_ID_LEN];
	char	cfid[CONF_ID_LEN];
	
	if (confCache == NULL)
	{
		// coming through generator code
		return 0;
	}

	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	confHandle = CacheGet(confCache, confID);

	if (confHandle)
	{
		if (confHandle->ncalls <= 0)
		{
			NETERROR(MARQ, ("%s Conf handle %s has no calls\n",
					fn, ConfID2String(confID, cfid)));
		}

		for(i =0;i<confHandle->ncalls;++i)
		{
			if (!memcmp(confHandle->callID[i], callID, CALL_ID_LEN))
			{
				NETDEBUG(MARQ, NETLOG_DEBUG4,
						("%s Deleting callID %s from conf handle %s\n",
						 fn, CallID2String(callID, cid), ConfID2String(confID, cfid)));
				/* match */
				memset(confHandle->callID[i], 0, CALL_ID_LEN);
				if ((confHandle->ncalls>1)&&(i!=(confHandle->ncalls-1)))
				{
					memcpy(confHandle->callID[i], 
						confHandle->callID[confHandle->ncalls-1], 
						CALL_ID_LEN);
					memset(confHandle->callID[confHandle->ncalls-1], 0, CALL_ID_LEN);
				}
				confHandle->ncalls --;
			}
		}


		if (confHandle->ncalls<= 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
					("%s Deleting conf handle %s\n",
					 fn, ConfID2String(confID, cfid)));
			if (CacheDelete(confCache, confID) != confHandle)
			{
				NETERROR(MARQ, ("%s Failed to delete conf handle %s\n",
							fn, ConfID2String(confID, cfid)));
			}
			GisFreeConfHandle(confHandle);
		}
	}
	
	CacheReleaseLocks(confCache);

	// Reset the other mapping also
	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	callHandle = CacheGet(callCache, callID);

	if (callHandle)
	{
		memset(callHandle->confID, 0, CALL_ID_LEN);
	}
	
	CacheReleaseLocks(callCache);

	return(0);
	
}

int
GisAddCallToConfGetConf(CallHandle *callHandle, ConfHandle **confhp)
{
	char fn[] = "GisAddCallToConf():";
	ConfHandle *confHandle;
	char confIdStr[CALL_ID_LEN];

	if (confhp) {
	  *confhp = NULL;
	}

	if (confCache == NULL)
	{
		// coming through generator code
		return 0;
	}

	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	confHandle = (ConfHandle *)CacheGet(confCache, callHandle->confID);

	if (confHandle == NULL)
	{
		if(nlm_getvport() <0 )
		{
			NETDEBUG(MLMGR, NETLOG_DEBUG4,
				("%s nlm_getvport failed - %s\n", 
				fn, (char*) ConfID2String(callHandle->confID, confIdStr)));
			CacheReleaseLocks(confCache);
			return -1;
		}
		/* Allocate a conf handle */
		confHandle = GisAllocConfHandle();

		/* Initialize the main parameter */
		ConfSetConfID(confHandle, callHandle->confID);

		/* Insert the call handle into the cache */
		if (CacheInsert(confCache, confHandle) < 0)
		{
			NETERROR(MARQ, 
				("%s Failed to add confHandle into cache\n", fn));
		}
		 
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Inserting new conf handle %s into the cache\n", 
			fn, (char*) CallID2String(callHandle->confID, confIdStr)));
	}

	if (confHandle->ncalls < MAX_CONF_CALLS)
	{
		int i;
		for(i = 0; i < confHandle->ncalls; i++)
		{
			if (!memcmp(confHandle->callID[i], callHandle->callID, CALL_ID_LEN))
			{
				NETERROR(MARQ, ("%s Conf already has this call id\n", fn));
				CacheReleaseLocks(confCache);
				return -1;
			}
		}
		/* Insert the call id into the call handle */
		memcpy(confHandle->callID[confHandle->ncalls], callHandle->callID, CALL_ID_LEN);
		confHandle->ncalls++;
		CacheReleaseLocks(confCache);
	}
	else
	{
		NETERROR(MARQ, ("%s Conf already has %d calls\n", fn, confHandle->ncalls));
		CacheReleaseLocks(confCache);
		return -1;
	}

	if (confhp) {
		*confhp = confHandle;
	}
	return 0;
}

/************************ CALL/CONF ********************************/

CallHandle *
GisAllocCallHandle(void)
{
	 CallHandle *c = (CallHandle *)(CMalloc(callCache))(sizeof(CallHandle));

	if ( c == NULL)
	{
		return NULL;
	}

	 memset(c, 0, sizeof(CallHandle));

	 setRadiusAccountingSessionId(c);

#if 0
	 time(&c->iTime);
	 time(&c->rTime);
#endif
	 c->evtList = listInit();
	 c->fcevtList = listInit();

	 return c;
}
/*	Free H323RTPSet if its there
*/
void
GisFreeCallHandle(void *ptr)
{
	CallHandle *p = ptr;
	void *timerdata = NULL;
	SCC_EventBlock 		*evtPtr = NULL;

	if(p->max_call_duration_tid)
	{
		if(timerDeleteFromList(&localConfig.timerPrivate, p->max_call_duration_tid, &timerdata))
		{
			if(timerdata)
			{
				free(timerdata);
			}
		}
		p->max_call_duration_tid = 0;
	}

	switch(p->handleType)
	{
	case SCC_eH323CallHandle:
		if (H323inChan(p)[2].dataTypeHandle > 0 )
		{
			cmMeiEnter(UH323Globals()->hApp);
			freeNodeTree(UH323Globals()->hApp, H323inChan(p)[2].dataTypeHandle, 0);
			cmMeiExit(UH323Globals()->hApp);
		}
		if (H323inChan(p)[1].dataTypeHandle > 0 )
		{
			cmMeiEnter(UH323Globals()->hApp);
			freeNodeTree(UH323Globals()->hApp, H323inChan(p)[1].dataTypeHandle, 0);
			cmMeiExit(UH323Globals()->hApp);
		}
		if (H323localSet(p)!= NULL)
		{
	 		(CFree(callCache))(H323localSet(p));
		}
		if (H323remoteSet(p)!= NULL)
		{
	 		(CFree(callCache))(H323remoteSet(p));
		}
		if(H323remoteTCSNodeId(p))
		{
			cmMeiEnter(UH323Globals()->peerhApp);
			freeNodeTree(UH323Globals()->peerhApp, H323remoteTCSNodeId(p), 0);
			cmMeiExit(UH323Globals()->peerhApp);
		}
		if(H323tokenNodeId(p)>0)
		{
			cmMeiEnter(UH323Globals()->peerhApp);
			freeNodeTree(UH323Globals()->peerhApp, H323tokenNodeId(p), 0);
			cmMeiExit(UH323Globals()->peerhApp);
		}
		break;
	case SCC_eSipCallHandle:
		SipFreeSipCallHandle(SipCallHandle(p), sipCallCache->free);
		break;
	default:
		break;
	}

	if (p->realmInfo) 
	{
		RealmInfoFree(p->realmInfo, callCache->malloc);
	}

	if(p->evtList)
	{
		SCC_EventBlock 		*evtPtr = NULL;
		while((evtPtr = listDeleteFirstItem(p->evtList)))
		{
			H323FreeEvData((H323EventData *)(evtPtr->data));
			free(evtPtr);
		}
		listDestroy(p->evtList);
	}

	while ((evtPtr = listDeleteFirstItem(p->fcevtList)))
	{
		if (evtPtr->callDetails.callType == CAP_SIP)
		{
			SipFreeAppCallHandle(evtPtr->data);
			free(evtPtr);
		}
		else
		{
			H323FreeEvent(evtPtr);
		}
	}

	if(p->fcevtList)
	{
		listDestroy(p->fcevtList);
	}

	if (p->conf_id) (CFree(callCache))(p->conf_id);
	if (p->incoming_conf_id) (CFree(callCache))(p->incoming_conf_id);
	if (p->custID) (CFree(callCache))(p->custID);
	if (p->tg) (CFree(callCache))(p->tg);
	if (p->destTg) (CFree(callCache))(p->destTg);
	if (p->dtgInfo) (CFree(callCache))(p->dtgInfo);
	if (p->srccrname) (CFree(callCache))(p->srccrname);
	if (p->destcrname) (CFree(callCache))(p->destcrname);
	if (p->transitcrname) (CFree(callCache))(p->transitcrname);
	if (p->transRouteNumber) (CFree(callCache))(p->transRouteNumber);
	if(p->ogprefix)  (CFree(callCache))(p->ogprefix);

	if (p->vpnName)
	{
		free(p->vpnName);
	}
	
	if (p->zone)
	{
		free(p->zone);
	}

	if (p->cpname)
	{
		free(p->cpname);
	}

	GwFreeRejectList(p->destRejectList, CFree(callCache));
	p->destRejectList = NULL;

	SipFreeRemotecontactList(p->remotecontact_list);
	p->remotecontact_list = NULL;

	CdrArgsFree(p->prevcdr);

	(CFree(callCache))(p);
}

ConfHandle *
GisAllocConfHandle(void)
{
	 ConfHandle *c = (ConfHandle *)(CMalloc(confCache))(sizeof(ConfHandle));

	 memset(c, 0, sizeof(ConfHandle));

#if 0
	 time(&c->iTime);
	 time(&c->rTime);
#endif
	 c->h323EvtList = listInit();

	 return c;
}

void
GisFreeEgressTokenNodeId(void *ptr)
{
	ConfHandle *confHandle = (ConfHandle *)ptr;

	if ((confHandle != NULL) && (confHandle->egressTokenNodeId > 0))
	{
		NETDEBUG(MH323, NETLOG_DEBUG4, 
				("GisFreeEgressTokenNodeId(): Freeing egress token %d\n", 
				confHandle->egressTokenNodeId));

		freeNodeTree(uh323Globals[0].peerhApp,
		confHandle->egressTokenNodeId, 0);
		confHandle->egressTokenNodeId = -1;
	}
}

void 
GisFreeSetupQ931NodeId(void *ptr)
{   
    ConfHandle *confHandle = (ConfHandle *)ptr;

    if ((confHandle != NULL) && (confHandle->setupQ931NodeId > 0))
    {
		NETDEBUG(MH323, NETLOG_DEBUG4, 
			("GisFreeSetupQ931NodeId(): Freeing Setup Q.931 node ID %d\n", 
			confHandle->setupQ931NodeId));

        freeNodeTree(uh323Globals[0].peerhApp,
            confHandle->setupQ931NodeId, 0);
        confHandle->setupQ931NodeId = -1;
    }
}

void
GisFreeConfHandle(void *ptr)
{
	ConfHandle *confHandle = (ConfHandle *)ptr;

	if (confHandle->h323EvtList)
	{
		FreeH323EvtQueue(&(confHandle->h323EvtList));
	}
		
	if (confHandle->sipEvt)
	{
		SipFreeAppCallHandle(confHandle->sipEvt->data);
		free(confHandle->sipEvt);
	}

	GisFreeSetupQ931NodeId(confHandle);

	GisFreeEgressTokenNodeId(confHandle);

	if (confHandle->mediaRouted)
		nlm_freeMRvport();  // free the media routing license for this call

	(CFree(confCache))(ptr);
}

CallRealmInfo *
RealmInfoDup (CallRealmInfo *ri, int mallocfn)
{
	char fn[] = "RealmInfoDup()";
	CallRealmInfo *realmInfo = NULL;

	if (ri == NULL)
	{
		return NULL;
	}
	realmInfo = (CallRealmInfo *)MMalloc (mallocfn, sizeof (CallRealmInfo));
	if (realmInfo == NULL) 
	{
		NETERROR (MSIP, ("%s malloc failed for realminfo", fn));
		return NULL;
	}

	memcpy(realmInfo, ri, sizeof(CallRealmInfo));

	if (ri->sipdomain)
	{
		realmInfo->sipdomain = MStrdup(mallocfn, ri->sipdomain);
	}

	return realmInfo;
}

void
RealmInfoFree(CallRealmInfo *ri, int freefn)
{
	if (!ri)
	{
		return;
	}

	if (ri->sipdomain)
	{
		MFree(freefn, ri->sipdomain);
	}

	MFree(freefn, ri);
}

