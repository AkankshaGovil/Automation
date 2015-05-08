#include "gis.h"
#include <malloc.h>
#include "timer.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include "include/tsmtimer.h"
#include "include/tsmq.h"
#include <malloc.h>

int
SipTimerProcessor(struct Timer *timer)
{
	char fn[]="SipTimerProcessor()";
	char *name=timer->name;
	SipTransKey *siptrankey=timer->data;
	SipTrans siptran, *siptranptr=NULL;
	SipTranSMEntry *siptransm=NULL;
	int (*CSMCallFn)(void *) = NULL;
	int lockflag=0, lockentryflag=0;
	TsmQEntry *qentry;

	if(name == NULL)
	{
		NETERROR(MSIP, ("%s error: timer expired, but has no name\n", fn));
		goto _error;
	}

	NETDEBUG( MSIP,
              NETLOG_DEBUG4,
              ("%s entering: service timer %s\n",fn, name));

	if (siptrankey == NULL)
	{
		NETERROR(MSIP, ("%s Null siptrankey for timer %s\n", 
			fn, name));
		goto _error;
	}

	// no need to acquire locks here
	CacheGetLocks(localConfig.timerPrivate.timerCache, LOCK_WRITE, LOCK_BLOCK);

	timer->data = NULL;

	CacheReleaseLocks(localConfig.timerPrivate.timerCache);

#if 0
	CacheGetLocks(localConfig.timerPrivate.timerCache, LOCK_READ, LOCK_BLOCK);
#endif

	CacheGetLocks( transCache, LOCK_WRITE, LOCK_BLOCK );
	lockflag = 1;

	if( (siptranptr=CacheGet(transCache, siptrankey)) == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG1,
			("%s error: timer expired, but no transaction\n",fn));
		goto _error;
	}

	LockGetLock(&siptranptr->lock, LOCK_WRITE, LOCK_BLOCK); lockentryflag = 1;

	if (siptranptr->done)
	{
		timerFreeHandle(timer);

		goto _return;
	}

	if (siptranptr->inuse)
	{
		// Queue the message and return
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Queueing Message\n", fn));
		qentry = TsmNewQEntry();
		qentry->s = NULL;
		qentry->timer = timer;
		qentry->from=2;
		qentry->context=NULL;

		listAddItem(siptranptr->msgs, qentry);
		goto _return;
	}
	else
	{
		siptranptr->inuse = 1;

		CacheReleaseLocks(transCache); lockflag = 0;
#if 0
		CacheReleaseLocks(localConfig.timerPrivate.timerCache);
#endif
		// release entry locks
		if (lockentryflag)
		{
			LockReleaseLock(&siptranptr->lock); 
			lockentryflag = 0;
		}

		// we have the entry
		SipTransOperate(2, siptranptr, NULL, timer, 0, NULL, NULL);
	}

 _return:
	if (lockentryflag)
	{
		LockReleaseLock(&siptranptr->lock); 
		lockentryflag = 0;
	}

	if(lockflag)
	{
		lockflag = 0;
		CacheReleaseLocks(transCache);
#if 0
		CacheReleaseLocks(localConfig.timerPrivate.timerCache);
#endif
	}

	TsmFreeTsmKey(siptrankey);

	return 0;

 _error:
	if (lockentryflag)
	{
		LockReleaseLock(&siptranptr->lock); 
		lockentryflag = 0;
	}

	if(lockflag)
	{
		lockflag = 0;
		CacheReleaseLocks(transCache);
#if 0
		CacheReleaseLocks(localConfig.timerPrivate.timerCache);
#endif
	}

	TsmFreeTsmKey(siptrankey);
	timerFreeHandle(timer);

	return -1;

}

int
SipOperateTimer( SipTrans *siptranptr, 
                 struct Timer *timer )
{
	char fn[]="SipOperateTimer()";
	SipTranSMEntry *siptransm=NULL;
	int (*CSMCallFn)(void *) = NULL;
	int lockflag = 0;
	char *name = timer->name;

	if( name == NULL )
	{
		NETERROR(MSIP, ("%s error: timer expired, but has no name\n", fn));
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("%s entering: service timer %s\n",fn, name));

	/* null out timer id, timer has expired */
	SipTranTimer(siptranptr) = 0;

	if(strcmp(name, "INV_C_Re") == 0)
	{
		/* INVITE retransmission timer just expired */
		if(siptranptr->count_ReTx >= MAX_INV_RQT_RETRAN)
		{
			/* INVITE max retransmission reached */
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s %d invite sent\n",
				fn, MAX_INV_RQT_RETRAN));
			siptranptr->event = SipInvite7Sent;
			siptranptr->error = tsmError_NO_RESPONSE;
		}
		else
		{
			/* INVITE client retransmission timer expired */
			siptranptr->event = SipInviteClientReTxExpire;
		}

		siptransm = 
			&SipInviteSM_Client[siptranptr->currState][siptranptr->event];

		TsmAssignInvClientStateName(siptranptr, siptranptr->currState);
		TsmAssignInvClientEventName(siptranptr, siptranptr->event);

	}
	else if(strcmp(name, "INV_C_TT") == 0)
	{
		/* INVITE termination timer just expired, time to delete transaction */
		siptranptr->event = SipInviteClientTermExpire;

		siptransm = 
			&SipInviteSM_Client[siptranptr->currState][siptranptr->event];

		TsmAssignInvClientStateName(siptranptr, siptranptr->currState);
		TsmAssignInvClientEventName(siptranptr, siptranptr->event);

	}
	else if(strcmp(name, "INV_S_Re") == 0)
	{
		if (SipTranResponseCode(siptranptr) < 200)
		{
			// how can we be doing this ?
			NETERROR(MSIP, ("%s INV_S_Re Error for %d\n", fn, 
				SipTranResponseCode(siptranptr)));
			goto _error;
		}

		/* INVITE response timer expired, time to retransmit response */
		siptranptr->event = SipInviteServerReTxExpire;

		siptransm = 
			&SipInviteSM_Server[siptranptr->currState][siptranptr->event];

		TsmAssignInvServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInvServerEventName(siptranptr, siptranptr->event);

	}
	else if(strcmp(name, "INV_S_TT") == 0)
	{
		/* INVITE server termination timer expired, time to delete transaction */
		siptranptr->event = SipInviteServerTermExpire;
		siptranptr->error = tsmError_NO_RESPONSE;

		siptransm = 
			&SipInviteSM_Server[siptranptr->currState][siptranptr->event];

		TsmAssignInvServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInvServerEventName(siptranptr, siptranptr->event);

	}
	else if(strcmp(name, "OTH_C_Re") == 0)
	{
		/* ByeCancel client retrans timer expired, time to retransmit */
		if(siptranptr->count_ReTx >= MAX_OTHER_RQT_RETRAN)
		{
			/* ByeCancel max retransmission reached */
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s 11 ByeCancel sent\n",fn));
			siptranptr->event = SipByeCancel11Sent;
			siptranptr->error = tsmError_NO_RESPONSE;
		}
		else
		{
			/* ByeCancel client retransmission timer expired */
			siptranptr->event = SipByeCancelClientReTxExpire;
		}

		siptransm = 
			&SipByeCancelSM_Client[siptranptr->currState][siptranptr->event];

		TsmAssignOthClientStateName(siptranptr, siptranptr->currState);
		TsmAssignOthClientEventName(siptranptr, siptranptr->event);

	}
	else if(strcmp(name, "OTH_S_TT") == 0) 
	{
		/* ByeCancel server termination timer expired, time to delete transaction */
		siptranptr->event = SipByeCancelServerTermExpire;

		siptransm = 
			&SipByeCancelSM_Server[siptranptr->currState][siptranptr->event];

		TsmAssignOthServerStateName(siptranptr, siptranptr->currState);
		TsmAssignOthServerEventName(siptranptr, siptranptr->event);

	}
	else if (strcmp (name, "INFO_S_Re") == 0)
	{
		if (siptranptr->count_ReTx < MAX_INFO_RETRAN)
			siptranptr->event = SipInfoFromNet;
		else
			siptranptr->event = SipInfoServerTermExpire;
		siptransm = &SipInfoSM_Server[siptranptr->currState][siptranptr->event];

		TsmAssignInfoServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInfoServerEventName(siptranptr, siptranptr->event);
	}
	else if (strcmp (name, "INFO_C_Re") == 0)
	{
		if (siptranptr->count_ReTx >= MAX_INFO_RETRAN)
		{
			/* INFO max retransmission reached */
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s %d INFOs sent\n", fn, MAX_INFO_RETRAN));
			siptranptr->event = SipInfoAllSent;
			siptranptr->error = tsmError_NO_RESPONSE;
		}
		else
			siptranptr->event = SipInfoClientReTxExpire;

		siptransm = &SipInfoSM_Client[siptranptr->currState][siptranptr->event];

		TsmAssignInfoClientStateName (siptranptr, siptranptr->currState);
		TsmAssignInfoClientEventName (siptranptr, siptranptr->event);
	}
	else if(strcmp(name, "INFO_S_TT") == 0) 
	{
		/* INFO server termination timer expired, time to delete transaction */
		siptranptr->event = SipInfoServerTermExpire;

		siptransm = &SipInfoSM_Server[siptranptr->currState][siptranptr->event];

		TsmAssignInfoServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInfoServerEventName(siptranptr, siptranptr->event);

	}
	else if(strcmp(name, "INFO_C_TT") == 0) 
	{
		/* INFO server termination timer expired, time to delete transaction */
		siptranptr->event = SipInfoClientTermExpire;

		siptransm = &SipInfoSM_Client[siptranptr->currState][siptranptr->event];

		TsmAssignInfoServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInfoServerEventName(siptranptr, siptranptr->event);

	}
	else if(strcmp(name, "GLB_TT") == 0)
	{
		/* global termination timer expired, to delete transaction immediately */
		SipRemoveTSM(siptranptr);
		goto _return;
	}
	else
	{
		NETERROR(MSIP, ("%s timer expired, but no service routine\n", fn));
		goto _error;
	}

	if(siptransm == NULL)
	{
		/* do not send anything to tsm */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));
	}
	else
	{
		if( SipTranSMProcessor(siptranptr,siptransm) < 0)
		{
			goto _error;
		}
	}

	if(siptranptr->done)
	{
		goto _return;
	}

	CSMCallFn=siptranptr->CSMCallFn;
	siptranptr->CSMCallFn = NULL;

	/* call CSM */
	if(CSMCallFn != NULL)
	{
		if( CSMCallFn(siptranptr) < 0)
		{
			NETERROR(MSIP, ("%s CSMCallFn error\n", fn));
			goto _error;
		}
	}

 _return:

	timerFreeHandle(timer);
	return 0;

 _error:

	timerFreeHandle(timer);
	return -1;

}

/* return 0 if no error 
 * GlbTimer has precedence over all other timers, 
	* can NOT be deleted, can only expire
 */
int
SipGlbTimer(SipTrans *siptran)
{
	char fn[]="SipGlbTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval;
	char name[]="GLB_TT";

	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("%s entering, retrans invite final response\n",fn));

	/* deleting running timer, 
		* GLB_TT will be the only running timer once set up 
  */

	if (siptran == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s siptran is NULL...not starting GLB timer\n", fn));
		goto _error;
	}

	SipDeleteTimer(siptran);

	siptrankey = TsmDupTsmKey(&(siptran->key));

	bzero(&timerval, sizeof(struct itimerval));
	timerval.it_value.tv_sec = SIPTRAN_GLB_TERM_TIMER;

	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("%s start %s timer = %ld(s) and %ld(usec)\n",fn,name, 
				       timerval.it_value.tv_sec,timerval.it_value.tv_usec));

	timerid=timerAddToList(&localConfig.timerPrivate, 
										&timerval, 0, PSOS_TIMER_REL,
			       name, SipTimerProcessor, siptrankey);

	/* saves timer id to transaction */
	SipTranTimer(siptran) = timerid;

	return 0;

 _error:
	return -1;

}
