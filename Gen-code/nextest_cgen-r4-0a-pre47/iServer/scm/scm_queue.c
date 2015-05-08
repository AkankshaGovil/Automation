#include "gis.h"
#include "lsprocess.h"
#include "scm.h"

SCM_StateQueue_t scmQueue;

void
SCM_QueueInit()
{
	// Initialize the queue
	scmQueue.queue = CacheCreate(0);
	scmQueue.queue->dt = CACHE_DT_CLIST;
	CacheSetName(scmQueue.queue, "SCM Queue");
	scmQueue.queue->cachecmp = -1;
	scmQueue.queue->cacheinscmp = -1;
	scmQueue.queue->lock = &scmQueue.scmLock;
	scmQueue.queue->listoffset = 0;
	scmQueue.queue->xitems = 0;

	CacheInstantiate(scmQueue.queue);
}

int
SCM_QueueReset()
{
	char fn[] = "SCM_QueueReset():";

	// Lock the queue
	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);

	scmQueue.scmPtr = NULL;

	lsMem->scm->pendingStates = scmQueue.queue->nitems;
	lsMem->scm->successStates = 0;

	CacheReleaseLocks(scmQueue.queue);

	NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s Re-setting the SCM queue\n", fn));

	return 0;
}

int
SCM_QueueEmpty()
{
	char fn[] = "SCM_QueueEmpty():";
	SCM_QueueItem *ptr;

	NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s %s Re-setting the SCM queue\n", fn, SCM_Mode()));

	// Lock the queue
	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);

	while (ptr = (SCM_QueueItem *)SCM_GetEntry())
	{
		// Delete this
		// This will also invoke the application callback
		SCM_DeleteEntry(ptr);
		free(ptr);
	}

	CacheReleaseLocks(scmQueue.queue);

	return 0;
}

//	int (*repfn)();	// replication function which must be called with the data - RPC fn
//	int (*appinfprefn)();	// application inform function which is called just before replication happens
//	int (*appinfpostfn)();	// application inform function which is called just after replication happens
// This fn is equivalent to enqueue
int
SCM_EnQueue(void *data, int (*repfn)(), int (*appinfprefn)(), int (*appinfpostfn)())
{
	char fn[] = "SCM_Queue():";
	SCM_QueueItem *ptr;

	// allocate internal entry
	ptr = malloc(sizeof(SCM_QueueItem));
	if (ptr == NULL)
	{
		NETERROR(MSCM, ("%s Out of memory\n", fn));
		return -1;
	}

	memset(ptr, 0, sizeof(SCM_QueueItem));

	ptr->data = data;
	ptr->repfn = repfn;
	ptr->appinfprefn = appinfprefn;
	ptr->appinfpostfn = appinfpostfn;

	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);

	if (CacheInsert(scmQueue.queue, ptr) < 0)
	{
		NETERROR(MSCM, ("%s failed to insert into queue %s\n", fn, scmQueue.queue->name));
	}
	else
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s %s Inserting new element in the SCM queue. Now %d items\n", 
			fn, SCM_Mode(), scmQueue.queue->nitems));
	}

	// inform application that we just inserted an element into this queue
	if (scmQueue.appinfqins)
	{
		scmQueue.appinfqins(ptr);
	}

	lsMem->scm->pendingStates ++;

	if (lsMem->scm->pendingStates == 1)
	{
		SCM_Signal();
	}

	CacheReleaseLocks(scmQueue.queue);

	return 0;
}

void *
SCM_GetEntry()
{
	char fn[] = "SCM_GetEntry():";
	SCM_QueueItem *nexptr;

	// Lock the queue
	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);
	
	if (scmQueue.scmPtr == NULL)
	{
		nexptr = CacheGetFirst(scmQueue.queue);
	}
	else
	{
		nexptr = CacheGetNext(scmQueue.queue, scmQueue.scmPtr);
	}
		
	if (nexptr == NULL)
	{
		// There is no new element
		NETDEBUG(MSCM, NETLOG_DEBUG4, ("%s no new elt to return from SCM queue\n", fn));
	}
	else
	{
		++nexptr->refCount;		// indicate the replicator thread is using entry
		scmQueue.scmPtr = nexptr;
		nexptr->passed = 1;

		lsMem->scm->pendingStates --;

		NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s returning new elt from SCM queue\n", fn));
	}
	
	CacheReleaseLocks(scmQueue.queue);

	return nexptr;
}

int
SCM_Push()
{
	char fn[] = "SCM_Push():";

	// Lock the queue
	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);
	
	if (scmQueue.scmPtr != NULL)
	{

		lsMem->scm->pendingStates ++;

		scmQueue.scmPtr->passed = 0;

		if (scmQueue.scmPtr = CacheGetPrev(scmQueue.queue, scmQueue.scmPtr))
		{
			NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s going back on SCM queue ptr=%p\n", 
				fn, scmQueue.scmPtr));
		}
	}
	
	CacheReleaseLocks(scmQueue.queue);

	return scmQueue.scmPtr?1:0;
}

int
SCM_DeleteEntry(SCM_QueueItem *ptr)
{
	char fn[] = "SCM_DeleteEntry():";

	// Normally this would be a cache delete, but we want to
	// know if the scm pointer is pointing to this entry or not
	CacheGetLocks(scmQueue.queue, LOCK_WRITE, LOCK_BLOCK);
	
	// Delete from Queue
	if (scmQueue.scmPtr == ptr)
	{
		scmQueue.scmPtr = CacheGetPrev(scmQueue.queue, ptr);

		CacheDelete(scmQueue.queue, ptr);

		NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s deleting elt under scm pointer in SCM queue\n", fn));
		NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s new scm pointer is %p\n", fn, scmQueue.scmPtr));
	}
	else
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s deleting elt from SCM queue\n", fn));
		CacheDelete(scmQueue.queue, ptr);
	}

	if (!SCM_HasProcessed(ptr))
	{	
		lsMem->scm->pendingStates --;
	}

	NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s Leaving %d items\n", fn, scmQueue.queue->nitems));

	// inform application that we just inserted an element into this queue
	// Delete from application cache
	if (scmQueue.appinfqdel)
	{
		scmQueue.appinfqdel(ptr->data);
	}

	CacheReleaseLocks(scmQueue.queue);

	free(ptr->data);
	
	return 0;
}
