#include "gis.h"
#include "lsprocess.h"
#include "scm.h"
#include "scm_callint.h"
#include "scmrpc_api.h"
#include "log.h"


cache_t scmCallCache;

int
SCMCALL_Init()
{
	scmCallCache = CacheCreate(MEM_SHARED);
	scmCallCache->dt = CACHE_DT_AVL;
	CacheSetName(scmCallCache, "SCM callids");
	scmCallCache->cachecmp = SCMCallCmp;
	scmCallCache->cacheinscmp = SCMCallInsCmp;
	scmCallCache->lock = &scmQueue.scmLock;
	
	SCM_SetQInsAppCb(SCMCALL_HandleNewElement);
	SCM_SetQDelAppCb(SCMCALL_HandleDeleteData);

	CacheinscmpArray[SCMCallInsCmp] = CacheSCMCallInsCmp;
	CachecmpArray[SCMCallCmp] = CacheSCMCallCmp;

	// We want data to be local
	scmCallCache->malloc = MEM_LOCAL;
	scmCallCache->free = MEM_LOCAL;

	CacheInstantiate(scmCallCache);

	lsMem->scm->scmCallCache = scmCallCache;

	return 0;
}

int
SCMCALL_HandleNewElement(SCM_QueueItem *elt)
{
	char fn[] = "SCMCALL_HandleNewElement():";
	
	// We don't want to insert states for deletes
	// We only want to track updates
	// However, at this point, we will use the same
	// callback

	// A new element as been added into this list
	// index it according to our callids
	if (CacheInsert(scmCallCache, elt) < 0)	
	{
		NETERROR(MSCM, ("%s %s cache insert failed\n", fn, SCM_Mode()));
	}
	else
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2,
			("%s %s inserted call into rep cache, now %d items\n", 
			fn, SCM_Mode(), scmCallCache->nitems));
	}

	return 0;
}

int
SCMCALL_HandleDeleteData(void *data)
{
	char fn[] = "SCMCALL_HandleDeleteData():";
	// We don't want to insert states for deletes
	// We only want to track deletes

	
	if (CacheDelete(scmCallCache, data) < 0)	
	{
		NETERROR(MSCM, ("%s %s cache delete failed\n", fn, SCM_Mode()));
	}
	else
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2,
			("%s %s deleted call from rep cache, now %d items\n", 
			fn, SCM_Mode(), scmCallCache->nitems));
	}

	return 0;
}

void
SCMCALL_Replicate(CallHandle *callHandle)
{
	char fn[] = "SCMCALL_Replicate():";
	char *callid, callstring[CALL_ID_LEN];
	SCM_QueueItem *elt;

	if (doScm == 0 || !scmEnabled())
	{
		return;
	}
	
	// We have been given a call handle to replicate
	// We will verify that the call is in a stable state
	// If so, we will check to see if there is a duplicate
	// already in the replication list

	if (!SCMCALL_IsStable(callHandle))
	{
		NETERROR(MSCM, ("%s %s Call %s is not stable\n", fn, SCM_Mode(),
			CallID2String(callHandle->callID, callstring)));
		return;
	}

	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);

	// Check if the element exists in the scm queue
	if (elt = CacheGet(scmCallCache, callHandle->callID))
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2,
			("%s %s call state already exists in cache, deleting existing entry\n", fn, SCM_Mode()));

#ifdef _no_del_cb
		if (CacheDelete(scmCallCache, callHandle->callID) < 0)
		{
			NETERROR(MSCM, ("%s %s cache delete failed\n", fn, SCM_Mode()));
		}
#endif

		// Check if the replicator thread is using entry
		if(elt->refCount == 0)
		{
			// no; delete this element
			SCM_DeleteEntry(elt);

			// free it
			free(elt);
		}
		else
		{
			// yes; mark for deletion by replicator thread
			elt->delete = 1;
		}
	}
	else
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2,
			("%s %s inserting new call state into cache\n", fn, SCM_Mode()));
	}

	CacheReleaseLocks(scmQueue.queue);

	// put the new updated element
	callid = (char *)malloc(CALL_ID_LEN);
	
	if (callid == NULL)
	{
		NETERROR(MSCM, ("%s %s ran out of memory for call %s\n", fn, SCM_Mode(),
			CallID2String(callHandle->callID, callstring)));
		return;
	}

	NETDEBUG(MSCM, NETLOG_DEBUG2,
		("%s %s inserting call %s in callCache\n", 
		fn, SCM_Mode(), CallID2String(callHandle->callID, callstring)));
	
	memcpy(callid, callHandle->callID, CALL_ID_LEN);

	SCM_EnQueue(callid, SCMCALL_ReplicateWorker, NULL, NULL);
}

int
SCMCALL_Delete(CallHandle *callHandle)
{
	char fn[] = "SCMCALL_Delete():";
	char *callid, callstring[CALL_ID_LEN];
	SCM_QueueItem *elt;
	int needDelete = 0;

	if (doScm == 0 || !scmEnabled())
	{
		return 0;
	}
	
	NETDEBUG(MSCM, NETLOG_DEBUG2,
		("%s %s for call %s\n",	fn, SCM_Mode(), 
		CallID2String(callHandle->callID, callstring)));

	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);

	// Check if the element exists in the scm queue
	if (elt = CacheGet(scmCallCache, callHandle->callID))
	{

		//  logic is different depending on whether element has processed or not
		if (SCM_HasProcessed(elt))
		{
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s call state already replciated, delete needs replication too\n", 
				fn, SCM_Mode()));

			// We have to send in a delete
			needDelete = 1;
		}
		else
		{
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s call state not replicated yet, delete does not need replication\n", 
				fn, SCM_Mode()));
		}

#ifdef _no_del_cb
		// We won't use the callback
		if (CacheDelete(scmCallCache, callHandle->callID) < 0)
		{
			NETERROR(MSCM, ("%s %s cache delete failed\n", fn, SCM_Mode()));
		}
#endif

		// Check if the replicator thread is using entry
		if(elt->refCount == 0)
		{
			// no; delete this element
			SCM_DeleteEntry(elt);

			// free it
			free(elt);
		}
		else
		{
			// yes; mark for deletion by replicator thread
			elt->delete = 1;
		}
	}

	CacheReleaseLocks(scmQueue.queue);

	if (needDelete)
	{
		// put the new updated element
		callid = (char *)malloc(CALL_ID_LEN);
	
		if (callid == NULL)
		{
			NETERROR(MSCM, ("%s %s ran out of memory for call %s\n", fn, SCM_Mode(),
				CallID2String(callHandle->callID, callstring)));
			return -1;
		}
	
		memcpy(callid, callHandle->callID, CALL_ID_LEN);
		
		SCM_EnQueue(callid, SCMCALL_DeleteWorker, NULL, NULL);

		NETDEBUG(MSCM, NETLOG_DEBUG2,
			("%s %s queueing delete\n", fn, SCM_Mode()));
	}

	return 0;
}

// returns 1 if state has been replicated
int
SCMCALL_CheckState(char *callid)
{
	char fn[] = "SCMCALL_CheckState():";
	char callstring[CALL_ID_LEN];
	SCM_QueueItem *elt;
	int hasReplicated = 0;

	if (doScm == 0 || !scmEnabled())
	{
		return 0;
	}
	
	NETDEBUG(MSCM, NETLOG_DEBUG2,
		("%s %s for call %s\n",	fn, SCM_Mode(), 
		CallID2String(callid, callstring)));

	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);

	// Check if the element exists in the scm queue
	if (elt = CacheGet(scmCallCache, callid))
	{
		//  logic is different depending on whether element has processed or not
		if (SCM_HasProcessed(elt))
		{
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s call state already replicated\n", 
				fn, SCM_Mode()));

			// We have to send in a delete
			hasReplicated = 1;
		}
		else
		{
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s call state not replicated yet\n", 
				fn, SCM_Mode()));
		}
	}

	CacheReleaseLocks(scmQueue.queue);

	return hasReplicated;
}

int
SCMCALL_ReplicateWorker(void *data)
{
	char fn[] = "SCMCALL_ReplicateWorker():";
	char *callid = (char *)data, callstring[CALL_ID_LEN], *callbuff = NULL;
	int callbufflen = 0;
	CallHandle *callHandle;

	// Lookup the call cache
	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	callHandle = CacheGet(callCache, callid);

	if (callHandle == NULL)
	{
		NETERROR(MSCM, ("%s %s could not find call %s in callCache\n", 
			fn, SCM_Mode(), CallID2String(callid, callstring)));

		CacheReleaseLocks(callCache);

		// We need to get rid of this entry from the cache
		return SCM_ErrorBadEntry;
	}

	if (!SCMCALL_IsStable(callHandle))
	{
		NETERROR(MSCM, ("%s %s call %s is not stable; postponing until next update\n", fn, SCM_Mode(),
			CallID2String(callHandle->callID, callstring)));

		CacheReleaseLocks(callCache);

		// We need to get rid of this entry from the cache
		return SCM_ErrorBadEntry;
	}
	else
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2,
			("%s %s call state being replicated, calling RPC function\n", fn, SCM_Mode()));
	}
	
	// Looks good, we can call the rpc encoding function
	if ((callbufflen = scm_encodecall(callHandle, &callbuff)) <= 0)
	{
		NETERROR(MSCM, 
			("%s %s scm_encodecall returned error len = %d\n", fn, SCM_Mode(), callbufflen));

		// currently this is a replication error. We
		// must reset back
		CacheReleaseLocks(callCache);

		// If an update cannot be replicated, we need
		// to stop and reset our replication pointer
		// Once the peer appears again, we need to start
		// again
		return SCM_ErrorNoPeer;
	}

	CacheReleaseLocks(callCache);

	// now we can call the rpc function
	if (scm_rpcsend(callbuff, callbufflen) < 0)
	{
		NETERROR(MSCM, 
			("%s %s scm_rpcsend returned error len = %d\n", fn, SCM_Mode(), callbufflen));

		// currently this is a replication error. We
		// must reset back

		// If an update cannot be replicated, we need
		// to stop and reset our replication pointer
		// Once the peer appears again, we need to start
		// again
		return SCM_ErrorNoPeer;
	}

	return SCM_Ok;
}

int
SCMCALL_DeleteWorker(void *data)
{
	char fn[] = "SCMCALL_DeleteWorker():";
	char *callid = (char *)data, callstring[CALL_ID_LEN];

	NETDEBUG(MSCM, NETLOG_DEBUG2,
		("%s %s for call %s\n",	fn, SCM_Mode(), CallID2String(callid, callstring)));

	// Looks good, we can call the rpc encoding function
	if (scm_deletecall(callid) < 0)
	{
		NETERROR(MSCM, 
			("%s %s RPC function returned error\n", fn, SCM_Mode()));

		return SCM_ErrorNoPeer;
	}

	return SCM_ErrorDeleteEntry;
}

int
CacheSCMCallCmp(const void *v1, const void *v2, void *param)
{
	SCM_QueueItem *ptr = (SCM_QueueItem *)v2;

	return memcmp(v1, ptr->data, CALL_ID_LEN);
}

int
CacheSCMCallInsCmp(const void *v1, const void *v2, void *param)
{
	SCM_QueueItem *ptr1 = (SCM_QueueItem *)v1;
	SCM_QueueItem *ptr2 = (SCM_QueueItem *)v2;

	return memcmp(ptr1->data, ptr2->data, CALL_ID_LEN);
}

