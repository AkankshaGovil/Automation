#include "gis.h"
#include "lsprocess.h"
#include "scm.h"
#include "nxosd.h"
#include "scmrpc_api.h"
#include "ua.h"
#include "scm_call.h"

cvtuple 		scmcv;
cvtuple *scmcvp = NULL;
static int scmInitialized = 0;

// Replicator Main routine.
// Replicator and monitor code are combined to reduce possibility of 
// things getting out of sync
// main loop for replicator
void *
SCM_Main(void *arg)
{
	char fn[] = "SCM_Main():";
	unsigned long backupAddr;
	static int primStatus = SCM_ErrorNoPeer; // applies only if we are backup
	int lastState;

	if (doScm == 0)
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2, ("%s SCM Disabled\n", fn));
		return NULL;
	}

	if( !scmEnabled() )
	{
		NETDEBUG(MSCM, NETLOG_DEBUG2, 
			("%s SCM Feature not licensed\n", fn));
		return NULL;
	}

	scmcvp = &scmcv;

	CVInit(scmcvp);

	if (SCM_Init() < 0)
	{
		// There is no need to run scm
		return NULL;
	}

	SCM_QueueInit();

	SCM_ApplicationInit();

	NETDEBUG(MSCM, NETLOG_DEBUG1,
		("%s Configuration checks successful, starting to initialize\n", fn));

	lastState = SCM_IsBackup();

	scmInitialized = 1;

	while (1)
	{
		// If I am backup, then I should send a heartbeat to primary
		// and go back to loop
		if (lastState != SCM_IsBackup())
		{
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s: State Change detected %ds\n",
				fn, SCM_Mode(), SCM_HEARTBEAT_PERIOD));

			(lastState = SCM_IsBackup())? SCM_NowBackup(): SCM_NowPrimary();
		}

		if (SCM_IsBackup())
		{
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s: Sending Heartbeat to Primary, waiting for %ds\n", 
				fn, SCM_Mode(), SCM_HEARTBEAT_PERIOD));

			if (primStatus != SCM_Ok)
			{
				NETDEBUG(MSCM, NETLOG_DEBUG1, 
					("%s %s Connecting to peer\n", fn, SCM_Mode()));
	
				primStatus = SCM_PeerConnect();
			}

			if (SCM_HeartbeatTx() != 0)
			{
				NETDEBUG(MSCM, NETLOG_DEBUG2,
					("%s %s: Heartbeat failed to primary, initiating re-connect %ds\n", 
					fn, SCM_Mode(), SCM_HEARTBEAT_PERIOD));

				primStatus = SCM_ErrorNoPeer;
			}

			goto _continue;
		}
		
		// Here we must wait only conditionally

		// I am primary - do I have a backup ?
		if (SCM_BackupExists() == 0)
		{
			// We should not be doing anything at this point
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s: Backup has not joined in, waiting for %ds\n", 
				fn, SCM_Mode(), SCM_HEARTBEAT_PERIOD));

			goto _continue;
		}

		// We are primary and we have a backup
		// Did that backup recently change ?
		if (SCM_CheckBackup(&backupAddr) == 1)
		{
			// Backup just changed
			// reset the SCM pointer and wait for a dead period
			NETDEBUG(MSCM, NETLOG_DEBUG2,
				("%s %s: Backup just joined in, waiting for %ds\n", 
				fn, SCM_Mode(), SCM_HEARTBEAT_PERIOD));

			SCM_QueueReset();

			SCM_PeerConnect();

			// Fall through and check for replication data
		}

		// everything is fine !
		// At this point, we can now start the replication
		// The backup's address is obtained above
		NETDEBUG(MSCM, NETLOG_DEBUG4,
			("%s %s: Checking for updates to send to backup %ds\n", 
			fn, SCM_Mode(), SCM_HEARTBEAT_PERIOD));

		switch (SCM_ReplicatorDoNextReplication())
		{
		case SCM_Ok:
			lsMem->scm->successStates ++;

			// We need to keep checking for otgher entries
			continue;
			break;

		case SCM_ErrorNoEntry:
			// There is nothing to replicate
			// Backoff
			break;
			
		case SCM_ErrorNoPeer:

			// Go back and stay there
			SCM_Push();

			break;

		default:
			NETERROR(MSCM, ("%s Unknown error\n", fn));
			break;
		}

	_continue:

		SCM_Wait();
	}
}

int
SCM_ReplicatorDoNextReplication()
{
	char fn[] = "SCM_ReplicatorDoNextReplication():";

	static int latencyIndex = 0;
	SCM_QueueItem *ptr;
	int rc = SCM_Ok;
	hrtime_t t1, t2;

	ptr = (SCM_QueueItem *)SCM_GetEntry();
	
	if (ptr)
	{
		// call the application inform function
		if (ptr->appinfprefn)
		{
			rc = ptr->appinfprefn(ptr->data);
		}

		if (rc != 0)
		{
			// Check if call thread requested deletion
			if (ptr->delete)
			{
				SCM_DeleteEntry(ptr);
				free(ptr);
			}

			// success till now
			return SCM_Ok;
		}

		// Time this...
		// repfn must be there...
		t1 = nx_gethrtime();

		rc = ptr->repfn(ptr->data);

		t2 = nx_gethrtime();

		lsMem->scm->transportLatency[latencyIndex++] = (t2-t1)/1000;

		latencyIndex %= N_SCM_STAT_RECORDS;

		if (lsMem->scm->nelems < N_SCM_STAT_RECORDS)
			lsMem->scm->nelems++;

		// certain errors are internal to the scm library and
		// are returned by the application to signal things
		switch (rc)
		{
		case SCM_ErrorBadEntry:

			NETERROR(MSCM, ("%s Bad entry error\n", fn));

			// In this case, we need to delete the entry we just processed
			SCM_DeleteEntry(ptr);
			free(ptr);

			return SCM_Ok;

		case SCM_ErrorDeleteEntry:

			// In this case, we need to delete the entry we just processed
			SCM_DeleteEntry(ptr);
			free(ptr);

			return SCM_Ok;
		}

		if (ptr->appinfpostfn)
		{
			ptr->appinfpostfn(ptr->data);
		}

		// idenicate that replicator thread has finished with entry
		--ptr->refCount;

		// Check if call thread requested deletion
		if (ptr->delete)
		{
			SCM_DeleteEntry(ptr);
			free(ptr);
		}

		return rc;
	}
	else
	{
		return SCM_ErrorNoEntry;
	}
}

int
SCM_Signal()
{
	char fn[] = "SCM_Signal():";
	int status;

	if (scmInitialized == 0)
	{
		return 0;
	}

	if ((status = pthread_cond_signal(&(scmcvp->cond))) != 0)
	{
		NETERROR(MSCM, ("%s pthread_mutex_unlock error %d\n", fn, status));
	}
	
	return status;
}

int
SCM_Wait()
{
	char fn[] = "SCM_Wait():";
	struct timespec abstime;
	int status;

	if ((status = pthread_mutex_lock(&(scmcvp->mutex))) != 0)
	{
		NETERROR(MSCM, ("%s pthread_mutex_lock error  %d\n", 
			fn, status));
	}

//	Replacing the non portable call with the POSIX altaernate

//	abstime.tv_sec = SCM_DEADWAIT_PERIOD;
//	abstime.tv_nsec = 0;
//	status = pthread_cond_reltimedwait_np(&(scmcvp->cond), &(scmcvp->mutex), &abstime);

	clock_gettime( CLOCK_REALTIME, &abstime );
	incr_timespec( &abstime, SCM_DEADWAIT_PERIOD, 0 );
	status = pthread_cond_timedwait(&(scmcvp->cond), &(scmcvp->mutex), &abstime);


	if ((status != 0) && (status != ETIMEDOUT))
	{
		NETERROR(MSCM, ("%s pthread_cond_wait error %d\n", fn, status));
	}

	if ((status = pthread_mutex_unlock(&(scmcvp->mutex))) != 0)
	{
		 NETERROR(MSCM, 
			("%s pthread_mutex_unlock error %d\n", fn, status));
	}

	return status;
}

// These functions are not part of this code and will be implemented 
// inside the ua etc
void
SCM_CallUpdateRx(CallHandle *c)
{
	NETDEBUG(MSCM, NETLOG_DEBUG4, ("got a call update\n"));

	scmCallHandleUpdate(c);
}

void
SCM_CallDeleteRx(char *callid)
{
	NETDEBUG(MSCM, NETLOG_DEBUG4, ("got a call delete\n"));

	scmCallHandleDelete(callid);
}

int
SCM_ApplicationInit()
{
	// Initialize the RPC callbacks for call replication
	scm_initcb_updatecall(SCM_CallUpdateRx);
	scm_initcb_deletecall(SCM_CallDeleteRx);
	SCMCALL_Init();

	return 0;
}

