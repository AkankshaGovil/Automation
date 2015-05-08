#include "gis.h"
#include <malloc.h>

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include "include/tsmq.h"
#include "tsmtimer.h"
#include <malloc.h>

#include "nxosd.h"
#include "thutils.h"
#include "siputils.h"
#include "net.h"
#include "bcpt.h"
#include "sipbcptfree.h"

SipMessage *
SipTransGetMsg(SipTransKey *key, int msgType)
{
	char fn[] = "SipTransGetMsg():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));
	return(0);
}

int
SipTransOperateUnmarshall(
	SipTransOperateArg *arg
)
{
	char fn[] = "SipTransOperateUnmarshall():";

	int from;
	SipTrans *siptranptr; 
	SipMessage *msg;
	struct Timer *timer;
	int newentry;
	char *method; 
	SipEventContext *context;

	from = arg->from;
	siptranptr = arg->siptranptr;
	msg = arg->msg;
	timer = arg->timer;
	newentry = arg->newentry;
	method = arg->method;
	context = arg->context;

	SipTransOperate(from, siptranptr, msg, timer, newentry, method, context);

	free(arg);
	return(0);
}

int
SipTransOperateMarshall(
	int from,
	SipTrans *siptranptr, 
	SipMessage *msg,
	struct Timer *timer,
	int newentry,
	char *method, 
	SipEventContext *context
)
{
	char fn[] = "SipTransOperateMarshall():";
	SipTransOperateArg *arg;

	arg = (SipTransOperateArg *)malloc(sizeof(SipTransOperateArg));
	arg->from = from;
	arg->siptranptr = siptranptr;
	arg->msg = msg;
	arg->timer = timer;
	arg->newentry = newentry;
	arg->method = method;
	arg->context = context;

	if (ThreadDispatch(poolid, lpcid, (PFVP)SipTransOperateUnmarshall, arg, 1,
			PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		// Error in launching thread
		NETERROR(MSIP, 
			("%s Could not launch SipTransOperateUnmarshall\n",
			fn));
		free(arg);
	}

	return 0;
}

int
SipTransOperate(
	int from,
	SipTrans *siptranptr, 
	SipMessage *msg,
	struct Timer *timer,
	int newentry,
	char *method, 
	SipEventContext *context
)
{
	char fn[] = "SipTransOperate():";
	int rc;
	TsmQEntry *qentry = NULL;
	hrtime_t stime, ftime;
	int threadIndex = getMyThreadIndex ();

_start:

	if (from==0)
	{
		if((rc = SipTransOperateIncomingMsg(siptranptr, msg, newentry,
				method, context)) < 0)
		{
			SipRemoveTSM(siptranptr);
		}
	}
	else if (from == 1)
	{
		stime = nx_gethrtime();

		rc = SipTransOperateMsgHandle(siptranptr, msg, newentry);

		ftime = nx_gethrtime();
		sipStats[threadIndex].ntrans ++;
		sipStats[threadIndex].ptime = (ftime-stime);
	}
	else if (from==2)
	{
		rc = SipOperateTimer(siptranptr, timer);
	}
	else if (from == 3)
	{
		rc = SipOperateIcmpDestUnreach(siptranptr);
	}

	SipCheckFree(method);

	if(siptranptr->done)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entry is DONE\n", fn));

		// entry is marked for deletion
		// Free up the message list
		TranFreeQueue(siptranptr);
		MFree(transCache->free, siptranptr);
		return 0;
	}

	// we are finished with this message, now we must see
	// if there is any other msg on the queue
	// lock the entry	
	LockGetLock(&siptranptr->lock, LOCK_WRITE, LOCK_BLOCK);
	
	// check queue
	qentry = listGetFirstItem(siptranptr->msgs);
	if (!qentry)
	{
		// list is empty, we should exit
		siptranptr->inuse = 0;
	}
	else
	{
		listDeleteItem(siptranptr->msgs, qentry);
	}
	
	LockReleaseLock(&siptranptr->lock);
	
	if (qentry)
	{
		msg = qentry->s;
		from = qentry->from;
		context = qentry->context;
		method = qentry->method;
		timer = qentry->timer;
	
		newentry = 0;

		free(qentry);

		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Processing Queue\n", fn));

		goto _start;
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s No more msgs in Queue\n", fn));
	}

	return 0;
}

int
SipTransOperateMsgHandle(
	SipTrans *siptranptr, 
	SipMessage *msg,
	int newentry
)
{
	char fn[] = "SipTransOperateMsgHandle():";
	SipError err;
	en_SipMessageType dType;
	int statuscode;
	SIP_S8bit *method=NULL;
	SipTranSMEntry *siptransm=NULL;
	int lockflag=0, lockentryflag=0;
	int (*CSMCallFn)(void *)=NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if(siptranptr == NULL)
	{
		NETERROR(MSIP, ("%s could not allocate entry\n", fn));
		goto _error;
	}
			
	if (msg == NULL)
	{
		// special case when we have to launch the GlbTimer
		SipGlbTimer(siptranptr);
		goto _return;
	}

	if (sip_getMessageType(msg, &dType, &err) == SipFail)
	{
		NETERROR(MSIP, 
			("%s sip_getMessageType returned error\n", fn));

		sip_freeSipMessage(msg); msg = NULL;
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s %s from CSM\n", 
		fn, ( (dType==SipMessageResponse)?"response":"request" ) ));

	if( (dType == SipMessageRequest) && (SipGetMethod(msg, &method) < 0) )
	{
		NETERROR(MSIP, ("%s fail to get method from msg \n",fn));
		sip_freeSipMessage(msg); msg = NULL;
		goto _error;
	}
	else if( (dType == SipMessageResponse) && 
				(SipGetStatusCode(msg, &statuscode) < 0) )
	{
		NETERROR(MSIP, ("%s fail to get status code from msg \n",fn));
		sip_freeSipMessage(msg); msg = NULL;
		goto _error;
	}

	TsmCheckState(siptranptr);

	if (newentry) 
	{
		if( sip_getMsgBodyCount(msg, 
			&SipTranRequestSdp(siptranptr), &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get msg body count\n", fn));
			sip_freeSipMessage(msg); msg = NULL;
			goto _error;
		}

		if( strcmp(method, "INVITE") == 0)
		{
			siptranptr->event = SipRequestFrCSM;
			siptransm = 
				&SipInviteSM_Client[siptranptr->currState][siptranptr->event];

			TsmAssignInvClientStateName(siptranptr, siptranptr->currState);
			TsmAssignInvClientEventName(siptranptr, siptranptr->event);
		}
		else if( (strcmp(method, "BYE") == 0) || 
				 (strcmp(method, "CANCEL") == 0) || 
				 (strcmp(method, "REFER") == 0) ||
				 (strcmp(method, "REGISTER") == 0) ||
                                 (strcmp(method, "NOTIFY") == 0)  )
		{
			siptranptr->event = SipRequestFrCSM;

			siptransm = 
				&SipByeCancelSM_Client[siptranptr->currState][siptranptr->event];

			TsmAssignOthClientStateName(siptranptr, siptranptr->currState);
			TsmAssignOthClientEventName(siptranptr, siptranptr->event);
		} 
		else if(strcmp (method, "INFO") == 0)
		{
			siptranptr->event = SipInfoFromCSM;

			siptransm = 
				&SipInfoSM_Client[siptranptr->currState][siptranptr->event];

			TsmAssignInfoClientStateName(siptranptr, siptranptr->currState);
			TsmAssignInfoClientEventName(siptranptr, siptranptr->event);
		} 

		if(siptransm == NULL)
		{
			/* do not send anything to tsm */
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));

			sip_freeSipMessage(msg); msg = NULL;
			goto _return;
		}

		SipTranRequest(siptranptr) = msg;
		SipTranRequestMethod(siptranptr) = strdup(method);

	}
	else
	{
		if(dType == SipMessageRequest &&
		   (strcmp(method, "ACK") == 0) && 
			(SipTranAck(siptranptr)==NULL) )
		{

			siptransm = 
				&SipInviteSM_Client[siptranptr->currState][SipAckFrCSM];

			if(siptransm == NULL)
			{
				/* do not send anything to tsm */
				NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));
	
				sip_freeSipMessage(msg); msg = NULL;
				goto _return;
			}

			siptranptr->event = SipAckFrCSM;

			TsmAssignInvClientStateName(siptranptr, siptranptr->currState);
			TsmAssignInvClientEventName(siptranptr, siptranptr->event);

			/* first Ack ever */
			SipTranAck(siptranptr) = msg;
			SipTranAckMethod(siptranptr) = strdup(method);
		}
		else if(dType == SipMessageResponse)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				("%s %d response for %s\n", fn, statuscode,
			    SipTranKeyMethod(&(siptranptr->key))));

			if ((SipTranResponseCode(siptranptr) > 0) &&
				(statuscode < SipTranResponseCode(siptranptr)))
			{
				NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Unordered response %d %d\n",
					fn, SipTranResponseCode(siptranptr), statuscode));

				// We should not send this response
				sip_freeSipMessage(msg); msg = NULL;

				goto _return;
			}

			/* free existing response */
			if (SipTranResponse(siptranptr))
			{
				sip_freeSipMessage(SipTranResponse(siptranptr));
			}

			SipTranResponseSendhost(siptranptr) = 0;
			SipTranResponseSendport(siptranptr) = 0;

			/* need to insert via headers from request msg */
			SipCopyVia(SipTranRequest(siptranptr), msg);

			/* need to insert record-route headers from request msg */
			if(statuscode >= 200 && statuscode < 300) 
			{
				SipCopyRR(SipTranRequest(siptranptr), msg);
			}

			if( strcmp(SipTranKeyMethod(&(siptranptr->key)), "INVITE") == 0)
			{
				/* response for INVITE from CSM */
				if(statuscode < 200)
				{
					siptranptr->event = Sip1xxFrCSM;
				}
				else if(statuscode < 300)
				{
					siptranptr->event = Sip2xxFrCSM;
				}
				else
				{
					siptranptr->event = Sip4xxFrCSM;
				}

				siptransm = 
					&SipInviteSM_Server[siptranptr->currState][siptranptr->event];

				TsmAssignInvServerStateName(siptranptr, siptranptr->currState);
				TsmAssignInvServerEventName(siptranptr, siptranptr->event);
			}
			else if(strcmp(SipTranKeyMethod(&(siptranptr->key)), "BYE") == 0 ||
				strcmp(SipTranKeyMethod(&(siptranptr->key)), "CANCEL") == 0 ||
				strcmp(SipTranKeyMethod(&(siptranptr->key)), "REFER") == 0 ||
				strcmp(SipTranKeyMethod(&(siptranptr->key)), "REGISTER") == 0 ||
			        strcmp(SipTranKeyMethod(&(siptranptr->key)), "NOTIFY") == 0
			       )
			{
				/* response for BYE or CANCEL from CSM */
				if(statuscode < 200)
				{
					siptranptr->event = SipByeCancel1xxFrCSM;
				}
				else
				{
					siptranptr->event = SipByeCancelFinalFrCSM;
				}

				siptransm = 
					&SipByeCancelSM_Server[siptranptr->currState][siptranptr->event];

				TsmAssignOthServerStateName(siptranptr, siptranptr->currState);
				TsmAssignOthServerEventName(siptranptr, siptranptr->event);
			} 
			else if (strcmp (SipTranKeyMethod(&(siptranptr->key)), "INFO") == 0) 
			{
				if (statuscode < 200) 
				{
					siptranptr->event = SipInfo1xxFrCSM;
				} else 
				{
					siptranptr->event = SipInfoFinalFrCSM;
				}
				siptransm = &SipInfoSM_Server[siptranptr->currState][siptranptr->event];

				TsmAssignInfoServerStateName(siptranptr, siptranptr->currState);
				TsmAssignInfoServerEventName (siptranptr, siptranptr->event);
			}

			if(siptransm == NULL)
			{
				/* do not send anything to tsm */
				NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));
	
				sip_freeSipMessage(msg); msg = NULL;
				goto _return;
			}

			SipTranResponse(siptranptr) = msg;
			SipTranResponseCode(siptranptr) = statuscode; 

		} 
	}

	if(siptransm == NULL)
	{
		/* do not send anything to tsm */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));
		sip_freeSipMessage(msg); msg = NULL;
	}
	else
	{
		if( SipTranSMProcessor(siptranptr,siptransm) < 0)
		{
			NETERROR(MSIP, ("%s TSM Error \n", fn));
			goto _error;
		}
	}

	if(siptranptr->done)
	{
		// entry is marked for deletion
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
	return 0;
_error:
	return -1;
}

/* Call SM sends message to TSM */
int
SipTransSendMsgHandle(SipAppMsgHandle *AppMsgHandle)
{
	char fn[] = "SipTransSendMsgHandle():";
	SipMessage *msg;
	SipError err;
	en_SipMessageType dType;
	int statuscode;
	SIP_S8bit *method=NULL;
	SipTranSMEntry *siptransm=NULL;
	SipTransKey siptrankey;
	SipTrans siptran, *siptranptr=NULL;
	int newentry=0;
	int lockflag=0, lockentryflag=0;
	int (*CSMCallFn)(void *)=NULL;
	TsmQEntry *qentry = NULL;
	SipEventContext *context;
	CallRealmInfo *ri;
	int realmid;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	bzero(&siptrankey, sizeof(siptrankey));

	/* msg is allocated memory in called function */
	if( SipFormMsgFromMsgHandle(&msg, AppMsgHandle) < 0)
	{
		NETERROR(MSIP, ("%s SipFormMsgFromMsgHandle error\n",fn));
		goto _error;
	}
 
	if (sip_getMessageType(msg, &dType, &err) == SipFail)
	{
		NETERROR(MSIP, 
			("%s sip_getMessageType returned error\n", fn));
		sip_freeSipMessage(msg); msg = NULL;
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("%s %s from CSM\n", 
	    fn, ( (dType==SipMessageResponse)?"response":"request" ) ));

	if( (dType == SipMessageRequest) && 
			(SipGetMethod(msg, &method) < 0) )
	{
		NETERROR(MSIP, ("%s fail to get method from msg \n",fn));
		sip_freeSipMessage(msg); msg = NULL;
		goto _error;
	}
	else if( (dType == SipMessageResponse) && 
			(SipGetStatusCode(msg, &statuscode) < 0) )
	{
		NETERROR(MSIP, ("%s fail to get status code from msg \n",fn));
		sip_freeSipMessage(msg); msg = NULL;
		goto _error;
	}

	if( SipTranKeyFromOutgoingMsg(msg, &siptrankey, 
			sizeof(siptrankey)) < 0)
	{
		/* error */
		NETERROR(MSIP, ("%s failed to get key from msg \n",fn));
		sip_freeSipMessage(msg); msg = NULL;
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("%s trying to locate transaction in cache\n", fn));

	PrintSipTranKey(&siptrankey);

#if 0
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s listing all cache entries\n", fn));
	TsmLogCache(0,0,0);
#endif


	CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);
	lockflag = 1;

	if( (siptranptr=CacheGet(transCache, &siptrankey)) == NULL)
	{
		/* not found in cache */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s cache not found, new entry\n", fn));

		if(AppMsgHandle->csmError != 0)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s error(%d) from CSM, but not tranx exists\n", 
					fn, AppMsgHandle->csmError));

			sip_freeSipMessage(msg); msg = NULL;
			goto _return;
		}

		if(dType == SipMessageResponse)
		{
			/* error */
			NETERROR(MSIP, 
				("%s response from CSM, but no tranx exists\n", fn));

			sip_freeSipMessage(msg); msg = NULL;
			goto _error;
		}
		else if(dType == SipMessageRequest)
		{
			/* new request to go out, start new transaction */
			if(strcmp(method,"INVITE") && 
			   strcmp(method,"BYE") && strcmp(method,"CANCEL") &&
			   strcmp(method,"REFER") && strcmp(method,"REGISTER") && strcmp (method, "INFO") 
			  && strcmp(method,"NOTIFY") )
			{
				/* only INVITE, BYE, and CANCEL can start new transaction */
				NETERROR(MSIP,
					("%s %s from CSM, but no tranx exists\n", fn, method));

				sip_freeSipMessage(msg); msg = NULL;
				goto _error;
			}

			siptranptr= (SipTrans *) SipAllocateTranHandle();
			if(siptranptr == NULL)
			{
				NETERROR(MSIP, ("%s could not allocate entry\n", fn));
				sip_freeSipMessage(msg); msg = NULL;
				goto _error;
			}
			
			newentry=1;

			memcpy(&(siptranptr->key), &siptrankey, sizeof(SipTransKey));

			context = (SipEventContext  *)malloc (sizeof (SipEventContext));
			if (context == NULL) 
			{
				NETERROR (MSIP, ("%s could not allocate context ", fn));
				sip_freeSipMessage (msg); msg = NULL;
				goto _error;
			}
			memset (context, 0, sizeof (SipEventContext));
			context->pData = (void *)RealmInfoDup (AppMsgHandle->realmInfo, MEM_LOCAL);
			if (context->pData == NULL)
			{
				NETERROR (MSIP, ("%s malloc failure", fn));
				goto _error;
			}

			if(AppMsgHandle->natip && AppMsgHandle->natport)
			{
				context->pTranspAddr = (SipTranspAddr *) malloc (sizeof (SipTranspAddr));
				if (context->pTranspAddr == NULL)
				{
					NETERROR (MSIP, ("%s malloc failure", fn));
					goto _error;
				}
				memset(context->pTranspAddr, 0, sizeof (SipTranspAddr));
				FormatIpAddress(AppMsgHandle->natip, context->pTranspAddr->dIpv4);
				context->pTranspAddr->dPort = AppMsgHandle->natport;
			}

			SipTranRequestContext(siptranptr) = context;

			// NOW we are assigning the msg. It must be processed.
			// If an error happens, the msg should not be freed
			// without resetting these pointers.

			//SipTranRequest(siptranptr) = msg;
			//SipTranRequestMethod(siptranptr) = strdup(method);

			/* insert into tranCache */
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s inserting transaction in cache\n", fn));
			if( CacheInsert(transCache,siptranptr) < 0)
			{
				NETERROR(MSIP, ("%s cache insert failed\n", fn));
				goto _error;
			}

			LockGetLock(&siptranptr->lock, LOCK_WRITE, LOCK_BLOCK); lockentryflag = 1;
		}
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s cache found\n", fn));
		if (dType == SipMessageResponse) {
		    if (SipTranResponseContext(siptranptr) == NULL) {
			/* Create context if one does'nt exist */
			context = (SipEventContext *)malloc ( sizeof (SipEventContext));
			if (context == NULL) {
				NETERROR (MSIP, ("%s could not allocate context", fn));
				goto _error;
			}
			memset (context, 0, sizeof (SipEventContext));
			context->pData = (void *)RealmInfoDup (AppMsgHandle->realmInfo, MEM_LOCAL);
			context->pTranspAddr = (SipTranspAddr *) malloc (sizeof (SipTranspAddr));
			if (context->pData ==NULL || context->pTranspAddr == NULL) 
			{
				NETERROR (MSIP, ("%s malloc failure", fn));
				goto _error;
			}
			memset(context->pTranspAddr, 0, sizeof (SipTranspAddr));
			if (SipTranRequestContext(siptranptr)) {
				memcpy (context->pTranspAddr, SipTranRequestContext(siptranptr)->pTranspAddr,
					sizeof (SipTranspAddr));
			}
			else if(AppMsgHandle->natip && AppMsgHandle->natport)
			{
				FormatIpAddress(AppMsgHandle->natip, context->pTranspAddr->dIpv4);
				context->pTranspAddr->dPort = AppMsgHandle->natport;
			}
			SipTranResponseContext(siptranptr) = context;
		    } else {
			    context = SipTranResponseContext(siptranptr);
			    if (context->pData == NULL) {
				    context->pData = (void *) RealmInfoDup (AppMsgHandle->realmInfo, MEM_LOCAL);
			    }
		    }
		    if (context->pData == NULL) {
			    NETERROR (MSIP, ("%s: Failed to get realmInfo, appMsgHandle->relmInfo : %p",
					     fn, AppMsgHandle->realmInfo));
		    }
		}
		if (dType == SipMessageRequest &&
		    strcmp (method,"ACK") == 0) {
			/* Create ack context */
			context = (SipEventContext *) malloc (sizeof (SipEventContext));
			if (context == NULL) {
				NETERROR (MSIP, ("%s: could not allocate context", fn));
				goto _error;
			}
			memset (context, 0, sizeof (SipEventContext));
			context->pData = (void *) RealmInfoDup (AppMsgHandle->realmInfo, MEM_LOCAL);

			if (!context->pData)
			{
				NETERROR (MSIP, ("%s malloc failure.", fn));
				goto _error;
			}

			if(AppMsgHandle->natip && AppMsgHandle->natport)
			{
				context->pTranspAddr = (SipTranspAddr *) malloc (sizeof (SipTranspAddr));
				if (context->pTranspAddr == NULL)
				{
					NETERROR (MSIP, ("%s malloc failure", fn));
					goto _error;
				}
				memset(context->pTranspAddr, 0, sizeof (SipTranspAddr));
				FormatIpAddress(AppMsgHandle->natip, context->pTranspAddr->dIpv4);
				context->pTranspAddr->dPort = AppMsgHandle->natport;
			}

			SipTranAckContext (siptranptr) = context;
		}
		LockGetLock(&siptranptr->lock, LOCK_WRITE, LOCK_BLOCK); lockentryflag = 1;

		/* cache found */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s cache found \n", fn));

		if(AppMsgHandle->csmError == csmError_KILL_TRAN)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s install timer to kill tranx\n", fn));
			sip_freeSipMessage(msg); msg=NULL;

//			SipGlbTimer(siptranptr);
//			goto _return;
		}
	}

	if (siptranptr->done)
	{
		// we did not set it to done
		// free up the message and quit
		sip_freeSipMessage(msg);
		goto _return;
	}

	if (siptranptr->inuse)
	{
		// Queue the message and return
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Queueing Message\n", fn));

		qentry = TsmNewQEntry();
		qentry->s = msg;
		qentry->from=1;
		qentry->method = NULL;
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
		SipTransOperate(1, siptranptr, msg, NULL, newentry, NULL, NULL);
	}

_return:
	SipFreeAppMsgHandle(AppMsgHandle);
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
	if(!newentry)
	{
		SipFreeTranKeyMember(&siptrankey);
	}
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
	if(!newentry)
	{
		SipFreeTranKeyMember(&siptrankey);
	}
	return -1;
}

/* tsm informs csm */
/* return 0 if no error */
int
SipTranInformCSM(SipMessage *msg, SipEventContext *context)
{
	char fn[]="SipTranInformCSM():";
	SipAppMsgHandle *appMsgHandle=NULL;

	unsigned long srchostip = 0, ipaddrtrue = 0;
	int herror = 0;

	SipMessage *resp=NULL;
	SIP_S8bit *host=NULL;
	SIP_U16bit hostport;
	SipError err;
	int i, count = 0;
	SipMsgBody *msgbody = NULL;
	SipMimeHeader *mime = NULL;
	SipHeader *pContentType = NULL;
	SipHeader *pContentDisp = NULL;
	SIP_S8bit *type;
	SipEventContext *dupcontext;

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("Entering %s\n", fn));


	appMsgHandle = (SipAppMsgHandle *) malloc(sizeof(SipAppMsgHandle));
	bzero(appMsgHandle, sizeof(SipAppMsgHandle));
	appMsgHandle->msgHandle = (SipMsgHandle *) malloc(sizeof(SipMsgHandle));
	bzero(appMsgHandle->msgHandle, sizeof(SipMsgHandle));

	appMsgHandle->incomingPrivType = privacyTypeNone;
	appMsgHandle->privTranslate    = privTranslateNone;
	appMsgHandle->privLevel        = privacyLevelNone;

	if (SipFormCallRealmInfo (context, appMsgHandle) < 0) 
	{
		NETERROR (MSIP, ("%s failed to build realm info ", fn));
		goto _error;
	}

	if (SipFormMsgHandleFromMsg(msg, appMsgHandle) < 0)
	{
		NETERROR(MSIP,("%s msghandle forming error\n",fn));
		goto _error;
	}

	// check for NAT
	// initialize natip and natport fields in appMsgHandle
	if(appMsgHandle->msgHandle->srchost)
	{
		if((srchostip = ResolveDNS(appMsgHandle->msgHandle->srchost, &herror)) == -1)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s contact %s does not resolve to any ip\n", fn, appMsgHandle->msgHandle->srchost));
		}
		else
		{
			ipaddrtrue = ntohl(inet_addr(context->pTranspAddr->dIpv4));
	
			if (ipaddrtrue != srchostip)
			{
				appMsgHandle->natip = ipaddrtrue;
				appMsgHandle->natport = context->pTranspAddr->dPort;
			}
		}
	}

	if(sip_getMsgBodyCount(msg, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get msg body count\n", fn));
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Body count=%d\n", fn, count));
	for(i = 0; i < count; ++i)
	{
		if(sip_getMsgBodyAtIndex(msg, &msgbody, i, &err) != SipFail)
		{
			sip_initSipHeader (&pContentType, SipHdrTypeAny, &err);
			sip_initSipHeader (&pContentDisp, SipHdrTypeAny, &err);

			if(msgbody->pMimeHeader)
			{
				if(sip_bcpt_getMimeHeaderFromMsgBody(msgbody, &mime, &err) != SipFail)
				{
					if (sip_bcpt_getContentTypeFromMimeHdr(mime, &pContentType, &err) == SipFail)
					{
						NETERROR(MSIP, ("%s can't get ContentType from MimeHdr.\n", fn));
					}
					if (sip_bcpt_getContentDispositionFromMimeHdr(mime, &pContentDisp, &err) == SipFail)
					{
						NETDEBUG(MSIP, NETLOG_DEBUG2, ("%s can't get ContentDisp from MimeHdr.\n", fn));
						sip_freeSipHeader(pContentDisp);
						free(pContentDisp);
						pContentDisp = NULL;
					}
				}
			}
			else
			{
				if (sip_getHeader(msg, SipHdrTypeContentType, pContentType, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s can't get ContentType from message.\n", fn));
				}
				if (sip_getHeader(msg, SipHdrTypeContentDisposition, pContentDisp, &err) == SipFail)
				{
					NETDEBUG(MSIP, NETLOG_DEBUG2, ("%s can't get ContentDisp from message.\n", fn));
					sip_freeSipHeader(pContentDisp);
					free(pContentDisp);
					pContentDisp = NULL;
				}
			}

			sip_getMediaTypeFromContentTypeHdr(pContentType, &type, &err);
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Got mime type '%s'\n", fn, type));

			if(strcasecmp(type, "application/sdp") == 0)
			{
				if(SipFormRTPParamFromMsg(msgbody, appMsgHandle) < 0)
				{
					NETERROR(MSIP, ("%s fail to get rtp params from msg body at index %d\n", fn, i));
				}
			}
			else if(strcasecmp(type, "application/isup") == 0)
			{
				if(SipFormIsupFromMsg(msgbody, appMsgHandle, pContentType, pContentDisp) < 0)
				{
					NETERROR(MSIP, ("%s fail to get ISUP msg from msg body at index %d\n", fn, i));
				}
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s return from SipFormIsupFromMsg", fn));
			}
			else if(strcasecmp(type, "application/qsig") == 0)
			{
				if(SipFormQsigFromMsg(msgbody, appMsgHandle, pContentType, pContentDisp) < 0)
				{
					NETERROR(MSIP, ("%s fail to get QSIG msg from msg body at index %d\n", fn, i));
				}
			}
			else if(strcasecmp(type, "application/dtmf-relay") == 0)
			{
				if(SipFormDtmfParamFromMsg(msgbody, appMsgHandle) < 0)
				{
					NETERROR(MSIP, ("%s fail to get dtmf msg from msg body index %d\n", fn, i));
				}
			}
			else if(strcasecmp(type, "message/sipfrag") == 0)
			{
				if(SipFormSipFragFromMsg(msgbody,appMsgHandle) < 0 )
				{
					NETERROR(MSIP,("%s failed to get sip frag from msgbody", fn));
				}
			}
			else
			{
				NETERROR(MSIP,("%s unsupported media type %s\n",fn, type));

				if (SipFormatResponse(msg, context, &resp, 415, "Unsupported Media Type",
										  &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 415\n", fn));
					goto _error;
				}

				/* host will be freed by thread */
				dupcontext = SipDupContext(context);

				SipSendMsgToHost2(resp, dupcontext, SIPMSG_RESPONSE, host, hostport);

				goto _error;
			}

			sip_bcpt_freeSipMimeHeader(mime);
			mime = NULL;
			if (pContentType)
			{
				sip_freeSipHeader(pContentType);
				free(pContentType);
				pContentType = NULL;
			}
			if (pContentDisp)
			{
				sip_freeSipHeader(pContentDisp);
				free(pContentDisp);
				pContentDisp = NULL;
			}
			sip_freeSipMsgBody(msgbody);
			msgbody = NULL;
		}
		else
		{
			NETERROR(MSIP, ("%s fail to get msg body at index %d\n", fn, i));
		}
	}

	if( SipTransRecvMsgHandle(appMsgHandle) < 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG1,
			("%s SipTransRecvMsgHandle failed\n",fn));
		goto _error;
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("Leaving %s, no error\n", fn));

	return 0;
 _error:
	if (pContentType)
	{
		sip_freeSipHeader(pContentType);
		free(pContentType);
	}
	if (pContentDisp)
	{
		sip_freeSipHeader(pContentDisp);
		free(pContentDisp);
	}
	sip_freeSipMsgBody(msgbody);
	SipFreeAppMsgHandle(appMsgHandle);
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("Leaving %s, with error\n", fn));
	return -1;
}

/* Called when an incoming message is received
 * from network, meant for the TSM, and
 * transaction s/m corresponding to the message exists 
 */
int
SipTransIncomingMsg(
	SipMessage *msg, 
	char *method, 
	SipEventContext *context
)
{
	char fn[]="SipTransIncomingMsg():";

	if( SipTransProcIncomingMsg(msg,method,context,NULL) < 0)
	{
		return 0;
	}

	return 0;
}

/* Called when an incoming message is received
 * from network, meant for the TSM, and
 * no transaction s/m exists
 */
int
SipTransIncomingMsgForUADest(
	SipMessage *msg, 
	char *method, 
	SipEventContext *context,
	CacheTableInfo *cacheInfo
)
{
	char fn[] = "SipTransIncomingMsgForUADest():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if( SipTransProcIncomingMsg(msg,method,context,cacheInfo) < 0)
	{
		return 0;
	}

	return 0;
}

/* Gets the transaction corresponding to a message
 */ 
int
SipTransGetTransOrCallForIncomingMsg(SipMessage *msg)
{
	SipTrans siptran, *siptranptr=NULL;
	SipTransKey siptrankey;
	char fn[] = "SipTransGetTransOrCallForIncomingMsg():";
	int rc = 0;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	SipTranKeyFromIncomingMsg(msg, &siptrankey, sizeof(siptrankey));

	CacheGetLocks(transCache,LOCK_READ,LOCK_BLOCK);
	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	siptranptr = CacheGet(transCache, &siptrankey);

	if (siptranptr == NULL)
	{
		if (CacheGet(sipCallCache, &siptrankey.callLeg))
		{
			rc = 1;
		}
	}
	else
	{
		rc = 1;
	}
	
	CacheReleaseLocks(transCache);
	CacheReleaseLocks(callCache);
	
	SipFreeTranKeyMember(&siptrankey);

	return rc;
}

/* Gets the transaction corresponding to a message
 */ 
int
SipTransGetCallForIncomingMsg(SipMessage *msg)
{
	SipTrans siptran, *siptranptr=NULL;
	SipTransKey siptrankey;
	char fn[] = "SipTransGetCallForIncomingMsg():";
	int rc = 0;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	SipTranKeyFromIncomingMsg(msg, &siptrankey, sizeof(siptrankey));

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	if (CacheGet(sipCallCache, &siptrankey.callLeg))
	{
		rc = 1;
	}

	CacheReleaseLocks(callCache);
	
	SipFreeTranKeyMember(&siptrankey);

	return rc;
}
