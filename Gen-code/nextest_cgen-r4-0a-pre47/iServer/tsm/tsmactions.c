#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include "include/tsmtimer.h"
#include <malloc.h>
#include "siputils.h"
#include "thutils.h"

// Local change for sgen.
char  SgenGwIpAddr[256];
int GenMode;
int callsdisconnected=0;

/* return 0 if no error */
int
SipSendResp(SipTrans *siptran)
{
	char *host = NULL;
	unsigned short port = 0;
	SipError err;
	char fn[]="SipSendResp():";
	SipTrans siptrancopy;

	DEBUG(MSIP, NETLOG_DEBUG4, ("Entering %s\n",fn));

	if(SipTranResponse(siptran) == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no response exists in tranx -- done\n",
					       fn));
		return 0;
	}

#if 0
	SipTranResponse(siptran)->dRefCount ++;
	memcpy(&siptrancopy, siptran, sizeof(SipTrans));
	siptran = &siptrancopy;
	CacheReleaseLocks(transCache);
#endif

	if(SipTranResponseSendhost(siptran))
	{
		/* no need to do DNS again, send message directly */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s resend resp. to host=%lx port=%d context = %p\n", fn,
					       SipTranResponseSendhost(siptran),
					       SipTranResponseSendport(siptran),
					       SipTranResponseContext(siptran)));
		if( SipSendMessage(SipTranResponse(siptran),
				   SipTranResponseSendhost(siptran),
				   SipTranResponseSendport(siptran),
				   SipTranResponseContext(siptran),
				   SIPPROTO_UDP) < 0)
		{			
			NETERROR(MSIP, ("%s Could not send response\n", fn));
			goto _error;
		}
	}
	else
	{
		/* first time to send response */
		if (SipGetSentByHost(SipTranResponse(siptran), SipTranResponseContext(siptran), 0, 1, &host, &port, &err) <= 0)
		{
			NETERROR(MSIP, ("%s fail to get sentby from top via\n", fn));
			goto _error;
		}

		if (host)
		{
#if 1
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				("%s resend resp. to host=%s port=%d context=%p\n", fn,
						host,
						port, SipTranResponseContext(siptran)));

			/* start new thread, do DNS, save result */
			/* host will be freed by thread */
			SipTranSendThreadLaunch(SipTranResponse(siptran),
					SipTranResponseContext(siptran),
					SIPMSG_RESPONSE, host, port, 
					&SipTranResponseSendhost(siptran), 
					&SipTranResponseSendport(siptran));

			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				("%s resp sent to. to host=%lx port=%d\n", fn,
				       SipTranResponseSendhost(siptran),
				       SipTranResponseSendport(siptran)));
#endif
		}
		else
		{
			NETERROR(MSIP, ("%s No host found, dumping msg\n", fn));
			SipDumpMessage(SipTranResponse(siptran));
		}
	}
	
#if 0
	CacheGetLocks(transCache, LOCK_READ, LOCK_BLOCK);

	if( (siptran=CacheGet(transCache, &siptrancopy)) == NULL)
	{
		NETERROR(MSIP, ("%s Cannot store host/port for retransmission\n", fn));
	// free the message
	sip_freeSipMessage(SipTranResponse(siptran));

		goto _error;
	}
	
	// free the message
	sip_freeSipMessage(SipTranResponse(siptran));
	// copy the host/port
	SipTranResponseSendhost(siptran) = SipTranResponseSendhost(&siptrancopy);
	SipTranResponseSendport(siptran) = SipTranResponseSendport(&siptrancopy);
#endif

	/* increment retransmission count for final response */
	if(SipTranResponseCode(siptran) >= 200)
	{
		siptran->count_ReTx++;
	}

	return 0;
_error:
	/* do this in case of error also */
	/* increment retransmission count for final response */
	if(SipTranResponseCode(siptran) >= 200)
	{
		siptran->count_ReTx++;
	}
	return 0;
}

/* return 0 if no error */
int
SipNotifyCSM2(SipTrans *siptran)
{
	char fn[]="SipNotifyCSM2():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: entering\n",fn));

	/* increment refcount once so that nobody can free it */
	//SipTranRequest(siptran)->dRefCount ++;
	siptran->CSMCallFn = (void *) (SipNotifyCSM);
	return 0;

}
/* return 0 if no error */
int
SipNotifyCSM(SipTrans *siptran)
{
	char fn[]="SipNotifyCSM():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: entering\n",fn));

	if( SipTranInformCSM(SipTranRequest(siptran), SipTranRequestContext(siptran)) < 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG1,
			("%s error informing CSM\n",fn));
		goto _error;
	}

	/* free message */
	//sip_freeSipMessage(SipTranRequest(siptran));
	return 0;

_error:
	//sip_freeSipMessage(SipTranRequest(siptran));
	return -1;
}

/* return 0 if no error */
int
SipNop(SipTrans *siptran)
{
	DEBUG(MSIP, NETLOG_DEBUG4, ("SipNop: Transaction dropped\n"));
	return 0;
}

/* return 0 if no error */
int
SipSetGenericTermTimer(SipTrans *siptran, char *name, struct itimerval *timerval)
{
	char fn[]="SipSetGenericTermTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;

	NETDEBUG( MSIP, NETLOG_DEBUG4, ("%s entering\n",fn));

	if(SipTranTimer(siptran))
	{
		name = timerGetName(&localConfig.timerPrivate, SipTranTimer(siptran));

		NETDEBUG( MSIP, NETLOG_DEBUG4,
                  ("%s timer (%s) already running, return\n", fn, name));

		free(name);
		return 0;
	}

	siptrankey = TsmDupTsmKey(&(siptran->key)); 
	if (siptrankey == NULL)
	{
		NETERROR (MSIP, ("%s Cannot set timer %s\n", fn, name));
		return (-1);
	}

	NETDEBUG( MSIP, NETLOG_DEBUG4, ("%s start %s timer = %ld(s) and %ld(usec)\n",
              fn, name, timerval->it_value.tv_sec, timerval->it_value.tv_usec ));

	timerid = timerAddToList( &localConfig.timerPrivate,
                              timerval,
                              0,
                              PSOS_TIMER_REL,
			                  name,
                              SipTimerProcessor,
                              siptrankey );

	/* saves timer id to transaction */
	SipTranTimer(siptran) = timerid;

	NETDEBUG( MSIP, NETLOG_DEBUG4, ("%s leaving, no error\n",fn));
	return 0;
}

/* return 0 if no error */
int
SipInviteServerTermTimer(SipTrans *siptran)
{
	char fn[]="SipInviteServerTermTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval = {0};

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));

	timerval.it_value.tv_sec = INVITE_SERVER_TERM_TIMER;

	/* Use common routine for Term Timer */
	return (SipSetGenericTermTimer(siptran, "INV_S_TT", &timerval));
}

/* return 0 if no error */
int
SipByeCancelServerTermTimer(SipTrans *siptran)
{
	char fn[]="SipByeCancelServerTermTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval = {0};

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));

	timerval.it_value.tv_sec = OTHER_SERVER_TERM_TIMER;

	/* Use common routine for Term Timer */
	return (SipSetGenericTermTimer(siptran, "OTH_S_TT", &timerval));
}

/* return 0 if no error */
int
SipInviteServerReTxTimer(SipTrans *siptran)
{
	char fn[]="SipInviteServerReTxTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval={0};
	int i;
	char *name;
	int threadIndex = getMyThreadIndex ();

	NETDEBUG( MSIP,
              NETLOG_DEBUG4,
              ("%s entering, retrans invite final response\n",
              fn));

	if(SipTranTimer(siptran))
	{
		name = timerGetName(&localConfig.timerPrivate, SipTranTimer(siptran));

		NETDEBUG( MSIP,
                  NETLOG_DEBUG4,
                  ("%s timer (%s) already running, return\n", 
                  fn, name));

		free(name);

		return 0;
	}

	siptrankey = TsmDupTsmKey(&(siptran->key));
	if (siptrankey == NULL)
	{
		NETERROR (MSIP, ("%s Cannot set timer %s\n", fn, name));
		return (-1);
	}

	if(siptran->count_ReTx >= MAX_INV_RSP_RETRAN)
	{
		/* have reached max response retransmission count,
         * start termination timer
		 * 15.5 s has passed during response retransmission
         */

		name="INV_S_TT";

		timerval.it_value.tv_sec = INVITE_SERVER_TERM_TIMER;
	}
	else
	{
		/* start timer = min(T2, T1^n) */
		name="INV_S_Re";
		timerval.it_value.tv_sec =  TIMER_T1/1000000;
		timerval.it_value.tv_usec = TIMER_T1%1000000;
		i=1;
		while(i++ < (siptran->count_ReTx) )
		{
			timerval.it_value.tv_sec *= 2;
			if(timerval.it_value.tv_sec > TIMER_T2/1000000)
			{
				timerval.it_value.tv_sec =  TIMER_T2/1000000;
				timerval.it_value.tv_usec = TIMER_T2%1000000;
				break;
			}
			timerval.it_value.tv_usec *= 2;
			timerval.it_value.tv_sec += timerval.it_value.tv_usec/1000000;
			timerval.it_value.tv_usec = timerval.it_value.tv_usec%1000000;
		}
	}

	if (siptran->count_ReTx > 1)
	{
		sipStats[threadIndex].invsfr ++;
	}

	NETDEBUG( MSIP,
              NETLOG_DEBUG4,
              ("%s start %s timer = %ld(s) and %ld(usec)\n",
              fn,
              name, 
              timerval.it_value.tv_sec,
              timerval.it_value.tv_usec));

	timerid = timerAddToList( &localConfig.timerPrivate,
                              &timerval,
                              0,
                              PSOS_TIMER_REL,
                              name,
                              SipTimerProcessor,
                              siptrankey );

	/* saves timer id to transaction */
	SipTranTimer(siptran) = timerid;

	return 0;
 _error:
	free(siptrankey);
	return -1;
}

int
SipInfoClientTermTimer (SipTrans *siptran)
{
	char fn[] = "SipInfoClientTermTimer ()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval = {0};

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));

	timerval.it_value.tv_sec = INFO_CLIENT_TERM_TIMER;

	/* Use common routine for Term Timer */
	return (SipSetGenericTermTimer(siptran, "INFO_C_TT", &timerval));
}

int
SipInfoServerTermTimer (SipTrans *siptran)
{
	char fn[] = "SipInfoServerTermTimer ()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval={0};

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));

	timerval.it_value.tv_sec = INFO_SERVER_TERM_TIMER;

	/* Use common routine for Term Timer */
	return (SipSetGenericTermTimer(siptran, "INFO_S_TT", &timerval));
}

#if 0	/* Not used as of May 2004 */
int
SipInfoErrResp (SipTrans *siptran)
{
	char fn[] = "SipInfoErrResp()";

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));
	return 0;
}
#endif

int
SipInfoNotifyError (SipTrans *siptran)
{
	char fn[] = "SipInfoNotifyError ()";

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));
	return SipErrorCSM2(siptran);
}

int
SipInfoClientRespCSM (SipTrans *siptran)
{
	char fn[] = "SipInfoClientRespCSM ()";

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));
	return SipRespCSM2(siptran);
}

int
SipInfoClientReTxTimer (SipTrans *siptran)
{
	char fn[] = "SipInfoClientReTxTimer()";
	SipTransKey *siptrankey = NULL;
	tid timerid;
	int i;
	struct itimerval timerval={0};
	char *name;

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));
	if (SipTranTimer (siptran))
	{
		name = timerGetName(&localConfig.timerPrivate, SipTranTimer(siptran));

		NETDEBUG( MSIP, NETLOG_DEBUG4, ("%s timer %s already running.\n",
						fn, name));
		free(name);
		return 0;
	}
	siptrankey = TsmDupTsmKey (&(siptran->key));
	if (siptrankey == NULL)
	{
		NETERROR (MSIP, ("%s Cannot set timer %s\n", fn, name));
		return (-1);
	}

	if (siptran->count_ReTx >= MAX_INFO_RETRAN)
	{
		/* have reached max retransmission count, make timer expire soon */
		timerval.it_value.tv_usec = 10;
	}
	else
	{
		timerval.it_value.tv_sec = TIMER_T1/1000000;
		timerval.it_value.tv_usec = TIMER_T1%1000000;
		i=1;
		while(i++ < (siptran->count_ReTx) )
		{
			timerval.it_value.tv_sec *= 2;
			if(timerval.it_value.tv_sec > TIMER_T2/1000000)
			{
				timerval.it_value.tv_sec =  TIMER_T2/1000000;
				timerval.it_value.tv_usec = TIMER_T2%1000000;
				break;
			}
			timerval.it_value.tv_usec *= 2;
			timerval.it_value.tv_sec += timerval.it_value.tv_usec/1000000;
			timerval.it_value.tv_usec = timerval.it_value.tv_usec%1000000;
		}
	}

	name = "INFO_C_Re";
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Starting timer %s :%ld sec. %ld usec\n",
					fn, name, timerval.it_value.tv_sec, 
					timerval.it_value.tv_usec));
	timerid = timerAddToList (&localConfig.timerPrivate, &timerval, 0,
				  PSOS_TIMER_REL, name, SipTimerProcessor, siptrankey);
	SipTranTimer(siptran) = timerid;
	return 0;
}


/* return 0 if no error */
int
SipDeleteTimer(SipTrans *siptran)
{
	char fn[]="SipDeleteTimer()";
	tid timerid=SipTranTimer(siptran);
	struct Timer *timer;
	void *timerdata = NULL;
	char *name;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n",fn));

	if(timerid == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no timer to delete\n",fn));
		return 0;
	}

	name = timerGetName(&localConfig.timerPrivate, timerid);

	if( strcmp(name, "GLB_TT") == 0 )
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s will not delete %s\n",fn,name));

		free(name);

		return 0;
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s deleting timer = %s\n",fn, name));

		free(name);
	}

	if (timerDeleteFromList(&localConfig.timerPrivate, timerid, &timerdata))
	{
		// timer was successfully deleted
		// now we can free the data
		if ( timerdata != NULL )
		{
			TsmFreeTsmKey(timerdata);
		}
	}

	SipTranTimer(siptran) = 0;

	return 0;
}

/* return 0 if no error */
int
SipAckCSM2(SipTrans *siptran)
{
	char fn[]="SipAckCSM2():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: entering\n",fn));

	/* increment refcount once so that nobody can free it */
	//SipTranAck(siptran)->dRefCount ++;
	siptran->CSMCallFn = (void *) (SipAckCSM);
	return 0;
}

/* return 0 if no error */
int
SipAckCSM(SipTrans *siptran)
{
	char fn[]="SipAckCSM():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: entering\n",fn));

	if( SipTranInformCSM(SipTranAck(siptran), SipTranAckContext(siptran)) < 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG1,
			("%s error informing CSM\n",fn));
		goto _error;
	}

	/* free message */
	//sip_freeSipMessage(SipTranAck(siptran));
	return 0;

_error:
	//sip_freeSipMessage(SipTranRequest(siptran));
	return -1;
}

/* return 0 if no error */
/* locks MUST be acquired prior to this function */
int
SipRemoveTSM(SipTrans *siptran)
{
	char fn[]="SipRemoveTSM():";
	SipTransKey *keyptr=&(siptran->key);
	SipTrans *siptranptr=NULL;

	DEBUG(MSIP, NETLOG_DEBUG4, 
	      ("Entering %s, will remove following transaction from cache\n",fn));
	PrintSipTranKey(keyptr);

#if 0
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s listing all cache entries before remove\n", fn));
	TsmLogCache(0,0,0);
#endif

#if 0
	CacheGetLocks(localConfig.timerPrivate.timerCache, LOCK_READ, LOCK_BLOCK);
#endif
	CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);

	SipDeleteTimer(siptran);

	/* remove cache entry */
	siptranptr = CacheDelete(transCache, siptran);

	DelTSMDest(siptranptr);

	sip_freeSipMessage(SipTranRequest(siptran));
	SipTranRequest(siptran) = NULL;
	SipFreeContext(SipTranRequestContext(siptran));
	SipTranRequestContext(siptran) = NULL;
	SipCheckFree(SipTranRequestMethod(siptran));
	SipTranRequestMethod(siptran) = NULL;

	sip_freeSipMessage(SipTranResponse(siptran));
	SipTranResponse(siptran) = NULL;
	SipFreeContext(SipTranResponseContext(siptran));
	SipTranResponseContext(siptran) = NULL;
	SipCheckFree(SipTranResponseMethod(siptran));
	SipTranResponseMethod(siptran) = NULL;

	sip_freeSipMessage(SipTranNewresponse(siptran));
	SipTranNewresponse(siptran) = NULL;
	SipFreeContext(SipTranNewresponseContext(siptran));
	SipTranNewresponseContext(siptran) = NULL;
	SipCheckFree(SipTranNewresponseMethod(siptran));
	SipTranNewresponseMethod(siptran) = NULL;

	sip_freeSipMessage(SipTranAck(siptran));
	SipTranAck(siptran) = NULL;
	SipFreeContext(SipTranAckContext(siptran));
	SipTranAckContext(siptran) = NULL;
	SipCheckFree(SipTranAckMethod(siptran));
	SipTranAckMethod(siptran) = NULL;


	SipCheckFree(SipTranKeyLocal(keyptr));
	SipTranKeyLocal(keyptr) = NULL;
	SipCheckFree(SipTranKeyRemote(keyptr));
	SipTranKeyRemote(keyptr) = NULL;
	SipCheckFree(SipTranKeyCallid(keyptr));
	SipTranKeyCallid(keyptr) = NULL;
	SipCheckFree(SipTranKeyMethod(keyptr));
	SipTranKeyMethod(keyptr) = NULL;

	siptran->done = 1;

	CacheReleaseLocks(transCache);
#if 0
	CacheReleaseLocks(localConfig.timerPrivate.timerCache);
#endif

	/* free memory just before releasing cache lock */
	/*  (CFree(transCache))(siptranptr); */

#if 0
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s listing all cache entries after remove\n", fn));
	TsmLogCache(0,0,0);
#endif

	return 0;
}

/* return 0 if no error */
int
SipSendRequest(SipTrans *siptran)
{
	header_url *req_uri = NULL;
	SipError err;
	SIP_S8bit *sendtohost = NULL;
	unsigned short sendtoport = 0;
	char fn[]="SipSendRequest():";
	SipTrans siptrancopy;
	int rc = 0;

	DEBUG(MSIP, NETLOG_DEBUG4, ("Entering %s\n",fn));

	if(SipTranRequest(siptran) == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no req to send, done\n", fn));
		return 0;	
	}

#if 0
	SipTranRequest(siptran)->dRefCount ++;
	memcpy(&siptrancopy, siptran, sizeof(SipTrans));
	siptran = &siptrancopy;
	CacheReleaseLocks(transCache);
#endif

	if(SipTranRequestSendhost(siptran))
	{
		/* no need to do DNS again, send message directly */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s resend resp. to host=%lx port=%d\n", fn,
					       SipTranRequestSendhost(siptran),
					       SipTranRequestSendport(siptran)));
		if( SipSendMessage(SipTranRequest(siptran),
				   SipTranRequestSendhost(siptran),
				   SipTranRequestSendport(siptran), SipTranRequestContext(siptran),SIPPROTO_UDP) < 0)
		{			
			NETERROR(MSIP, ("%s Could not send Request\n", fn));
			goto _error;
		}
	}
	else
	{
		/* first time to send Request */

		if(SipTranRequestContext(siptran)->pTranspAddr)
		{
			sendtohost = SipTranRequestContext(siptran)->pTranspAddr->dIpv4;
			sendtoport = SipTranRequestContext(siptran)->pTranspAddr->dPort;
		}
		else 
		{
			if ( SipExtractReqUri(SipTranRequest(siptran), &req_uri, &err) == SipFail)
			{
				NETERROR(MSIP, 
					 ("%s fail to extract ReqUri from request\n", fn)); 
				goto _error;
			}
#if 1
			/* start new thread, do DNS, save result */
			/* host will be freed by thread */
			if(req_uri->maddr)
			{
				sendtohost = req_uri->maddr;
			}
			else
			{
				// Local change for sgen.
				if (GenMode)
					sendtohost = strdup(SgenGwIpAddr);
				else
					 sendtohost = req_uri->host; 
			}
			sendtoport = req_uri->port;
		}

		/* no need to do DNS again, send message directly */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s resend resp. to host=%s port=%d\n", fn,
							sendtohost, sendtoport));
		if (SipTranSendThreadLaunch(SipTranRequest(siptran),
					SipTranRequestContext(siptran),
					SIPMSG_REQUEST, strdup(sendtohost), sendtoport,
					&SipTranRequestSendhost(siptran), 
					&SipTranRequestSendport(siptran)) < 0)
		{
			NETERROR(MSIP, ("%s Could not send Request\n", fn));
			goto _error;
		}

		CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);
		AddTSMDest(siptran);
		CacheReleaseLocks(transCache);

		/* no need to do DNS again, send message directly */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s resend resp. to host=%lx port=%d\n", fn,
					       SipTranRequestSendhost(siptran),
					       SipTranRequestSendport(siptran)));
#endif
	}
	
#if 0
	CacheGetLocks(transCache, LOCK_READ, LOCK_BLOCK);

	if( (siptran=CacheGet(transCache, &siptrancopy)) == NULL)
	{
	// free the message
	sip_freeSipMessage(SipTranRequest(siptran));

		NETERROR(MSIP, ("%s Cannot store host/port for retransmission\n", fn));
		goto _error;
	}
	
	// free the message
	sip_freeSipMessage(SipTranRequest(siptran));

	// copy the host/port
	SipTranRequestSendhost(siptran) = SipTranRequestSendhost(&siptrancopy);
	SipTranRequestSendport(siptran) = SipTranRequestSendport(&siptrancopy);
#endif

	/* increment retransmission count */
	siptran->count_ReTx++;

	SipCheckFree(req_uri);
	return 0;
_error:
	SipCheckFree(req_uri);

	// set up the CSM functions, there is no need to return error
	siptran->error = tsmError_NO_RESPONSE;
	siptran->CSMCallFn = (void *) (SipErrorCSM);

	return 0;
}

/* return 0 if no error */
int
SipRespCSM2(SipTrans *siptran)
{
	char fn[]="SipRespCSM2():";

	DEBUG(MSIP, NETLOG_DEBUG4, ("Entering %s\n",fn));
	siptran->CSMCallFn = (void *) (SipRespCSM);
	siptran->CSMCallParam = NULL;

	if( SipTranResponse(siptran) == NULL)
	{
		/* first response of any kind */
		SipTranResponse(siptran) = SipTranNewresponse(siptran);
		SipTranResponseContext(siptran) = SipTranNewresponseContext(siptran);
		SipTranResponseCode(siptran) = SipTranNewresponseCode(siptran);

		/* CSM does NOT want to know 100 Trying */
		if(SipTranResponseCode(siptran) > 100)
		{
			/* increment refcount once so that nobody can free it */
			//SipTranResponse(siptran)->dRefCount ++;
			siptran->CSMCallParam = SipTranResponse(siptran);
		}
	}
	else if( (SipTranResponseCode(siptran) < 200) &&
		 (SipTranResponseCode(siptran) < SipTranNewresponseCode(siptran)) )
	{
		/* new response */
		sip_freeSipMessage(SipTranResponse(siptran));
		SipFreeContext(SipTranResponseContext(siptran));

		SipTranResponse(siptran) = SipTranNewresponse(siptran);
		SipTranResponseContext(siptran) = SipTranNewresponseContext(siptran);
		SipTranResponseCode(siptran) = SipTranNewresponseCode(siptran);

		/* increment refcount once so that nobody can free it */
		//SipTranResponse(siptran)->dRefCount ++;
		siptran->CSMCallParam = SipTranResponse(siptran);
	}
	else
	{
		/* discard new response */
		sip_freeSipMessage(SipTranNewresponse(siptran));
		SipFreeContext(SipTranNewresponseContext(siptran));
	}

	SipTranNewresponse(siptran) = NULL;
	SipTranNewresponseContext(siptran) = NULL;
	SipTranNewresponseCode(siptran) = 0;

	return 0;
}

int
SipByeCancelRespCSM2(SipTrans *siptran)
{
	char fn[]="SipByeCancelRespCSM2():";

	DEBUG(MSIP, NETLOG_DEBUG4, ("Entering %s\n",fn));
	siptran->CSMCallFn = (void *) (SipRespCSM);
	siptran->CSMCallParam = NULL;

	if( SipTranResponse(siptran) == NULL)
	{
		/* first response of any kind */
		SipTranResponse(siptran) = SipTranNewresponse(siptran);
		SipTranResponseContext(siptran) = SipTranNewresponseContext(siptran);
		SipTranResponseCode(siptran) = SipTranNewresponseCode(siptran);

		// For REGISTER, we need to send back the response
		/* CSM does NOT want to know 100 Trying */
		if (!strcmp(SipTranRequestMethod(siptran), "REGISTER"))
		{
			if(SipTranResponseCode(siptran) > 100)
			{
				/* increment refcount once so that nobody can free it */
				//SipTranResponse(siptran)->dRefCount ++;
				siptran->CSMCallParam = SipTranResponse(siptran);
			}
		}
		else if(!strcmp(SipTranRequestMethod(siptran),"REFER"))
		{
			if(SipTranResponseCode(siptran) == 202 )
			{
				siptran->CSMCallParam = SipTranResponse(siptran);
			}
		}
	}
	else if( (SipTranResponseCode(siptran) < 200) &&
		 (SipTranResponseCode(siptran) < SipTranNewresponseCode(siptran)) )
	{
		/* new response */
		sip_freeSipMessage(SipTranResponse(siptran));
		SipFreeContext(SipTranResponseContext(siptran));

		SipTranResponse(siptran) = SipTranNewresponse(siptran);
		SipTranResponseContext(siptran) = SipTranNewresponseContext(siptran);
		SipTranResponseCode(siptran) = SipTranNewresponseCode(siptran);

		if (!strcmp(SipTranRequestMethod(siptran), "REGISTER"))
		{
			siptran->CSMCallParam = SipTranResponse(siptran);
		}
	}
	else
	{
		/* discard new response */
		sip_freeSipMessage(SipTranNewresponse(siptran));
		SipFreeContext(SipTranNewresponseContext(siptran));
	}

	SipTranNewresponse(siptran) = NULL;
	SipTranNewresponseContext(siptran) = NULL;
	SipTranNewresponseCode(siptran) = 0;

	if (strcmp(SipTranRequestMethod(siptran), "REGISTER") &&
	    strcmp(SipTranRequestMethod(siptran),"REFER")  
	   )
	{
// Transaction should not be deleted for 1xx messages
		if (SipTranResponseCode(siptran) >= 200)
		{
			/* added for bug_7796. incremeting disconnect count to indicate 200 ok is received for Bye sent */
			if(!(strcmp(SipTranRequestMethod(siptran),"BYE")) && (SipTranResponseCode(siptran) == 200))
				callsdisconnected++;
			/* ----- */
			SipRemoveTSM(siptran);
		}
	}

	return 0;
}

/* return 0 if no error */
int
SipRespCSM(SipTrans *siptran)
{
	char fn[]="SipRespCSM():";

	DEBUG(MSIP, NETLOG_DEBUG4, ("Entering %s\n",fn));

	if(siptran->CSMCallParam)
	{
		SipTranInformCSM(siptran->CSMCallParam, SipTranResponseContext(siptran));
	}

	/* free message */
	//sip_freeSipMessage(siptran->CSMCallParam);
	siptran->CSMCallParam = NULL;

	return 0;
}

/* return 0 if no error */
int
SipInviteClientReTxTimer(SipTrans *siptran)
{
	char fn[]="SipStartInviteReTxTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval={0};
	int i;
	int threadIndex = getMyThreadIndex ();
	char *name;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n",fn));

	if(SipTranTimer(siptran))
	{
		name = timerGetName(&localConfig.timerPrivate, SipTranTimer(siptran));

		NETDEBUG( MSIP,
                  NETLOG_DEBUG4,
                  ("%s timer(%s) already running, return\n", 
                  fn, name));

		free(name);

		return 0;
	}

	siptrankey = TsmDupTsmKey(&(siptran->key));
	if (siptrankey == NULL)
	{
		NETERROR (MSIP, ("%s Cannot set timer %s\n", fn, name));
		return (-1);
	}

	if(siptran->count_ReTx >= MAX_INV_RQT_RETRAN)
	{
		/* have reached max retransmission count, make timer expire soon */
		timerval.it_value.tv_usec = 10;
	}
	else
	{
		timerval.it_value.tv_sec =  TIMER_T1/1000000;
		timerval.it_value.tv_usec = TIMER_T1%1000000;
		i=1;

		while(i++ < (siptran->count_ReTx) )
		{
			timerval.it_value.tv_sec *= 2;
			timerval.it_value.tv_usec *= 2;
			timerval.it_value.tv_sec += timerval.it_value.tv_usec/1000000;
			timerval.it_value.tv_usec = timerval.it_value.tv_usec%1000000;
		}
	}

	if (siptran->count_ReTx > 1)
	{
		sipStats[threadIndex].invcr ++;
	}
	else
	{
		sipStats[threadIndex].invc ++;
	}

	NETDEBUG( MSIP,
              NETLOG_DEBUG4,
              ("%s start timer = %ld (s) and %ld (usec)\n",
              fn, 
              timerval.it_value.tv_sec,
              timerval.it_value.tv_usec));

	timerid = timerAddToList( &localConfig.timerPrivate,
                              &timerval,
                              0,
                              PSOS_TIMER_REL,
                              "INV_C_Re",
                              SipTimerProcessor,
                              siptrankey);

	/* saves timer id to transaction */
	SipTranTimer(siptran) = timerid;

	return 0;
}

/* return 0 if no error */
int
SipByeCancelClientReTxTimer(SipTrans *siptran)
{
	char fn[]="SipByeCancelClientReTxTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval={0};
	int i;
	int threadIndex = getMyThreadIndex();
	char *name;

	NETDEBUG( MSIP,
              NETLOG_DEBUG4,
              ("%s entering\n",
              fn));

	if(SipTranTimer(siptran))
	{
		name = timerGetName(&localConfig.timerPrivate, SipTranTimer(siptran));

		NETDEBUG( MSIP,
                  NETLOG_DEBUG4,
                  ("%s timer(%s) already running, return\n", 
                  fn, name));

		free(name);

		return 0;
	}

	siptrankey = TsmDupTsmKey(&(siptran->key));
	if (siptrankey == NULL)
	{
		NETERROR (MSIP, ("%s Cannot set timer %s\n", fn, name));
		return (-1);
	}

	if(siptran->count_ReTx >= MAX_OTHER_RQT_RETRAN)
	{
		/* have reached max retransmission count, make timer expire soon */
		timerval.it_value.tv_usec = 10;
	}
	else if(SipTranResponse(siptran) || SipTranNewresponse(siptran))
	{
		/* have received some response, retrans on T2 */
		timerval.it_value.tv_sec =  TIMER_T2/1000000;
		timerval.it_value.tv_usec = TIMER_T2%1000000;
	}
	else
	{
		timerval.it_value.tv_sec =  TIMER_T1/1000000;
		timerval.it_value.tv_usec = TIMER_T1%1000000;
		i=1;

		while(i++ < (siptran->count_ReTx) )
		{
			timerval.it_value.tv_sec *= 2;
			if(timerval.it_value.tv_sec > TIMER_T2/1000000)
			{
				timerval.it_value.tv_sec =  TIMER_T2/1000000;
				timerval.it_value.tv_usec = TIMER_T2%1000000;
				break;
			}
			timerval.it_value.tv_usec *= 2;
			timerval.it_value.tv_sec += timerval.it_value.tv_usec/1000000;
			timerval.it_value.tv_usec = timerval.it_value.tv_usec%1000000;
		}
	}

	if (!strcmp(siptran->key.method, "CANCEL"))
	{
		if (siptran->count_ReTx > 1)
		{
			sipStats[threadIndex].ccr++;
		}
		else
		{
			sipStats[threadIndex].cc++;
		}
	}
	else if (!strcmp(siptran->key.method, "BYE"))
	{
		if (siptran->count_ReTx > 1)
		{
			sipStats[threadIndex].byecr++;
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				 ("Incrementing bye client retrans on thread %d byes %d",
				  threadIndex, sipStats[threadIndex].byecr));
		}
		else
		{
			sipStats[threadIndex].byec++;
			NETDEBUG (MSIP, NETLOG_DEBUG4,
				  ("Incrementing bye client on thread %d, bye client %d",
				   threadIndex, sipStats[threadIndex].byec));
		}
	}

	NETDEBUG( MSIP,
              NETLOG_DEBUG4,
              ("%s start timer = %ld (s) and %ld (usec)\n",
              fn, 
              timerval.it_value.tv_sec,
              timerval.it_value.tv_usec));

	timerid = timerAddToList( &localConfig.timerPrivate,
                              &timerval,
                              0,
                              PSOS_TIMER_REL,
                              "OTH_C_Re",
                              SipTimerProcessor,
                              siptrankey);

	/* saves timer id to transaction */
	SipTranTimer(siptran) = timerid;

	return 0;
}

/* return 0 if no error */
int
SipInviteClientTermTimer(SipTrans *siptran)
{
	char fn[]="SipInviteClientTermTimer()";
	SipTransKey *siptrankey=NULL;
	tid timerid;
	struct itimerval timerval = {0};

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..", fn));

	timerval.it_value.tv_sec = INVITE_CLIENT_TERM_TIMER;

	/* Use common routine for Term Timer */
	return (SipSetGenericTermTimer(siptran, "INV_C_TT", &timerval));
}

/* return 0 if no error */
int
SipSendAck(SipTrans *siptran)
{
	header_url *req_uri = NULL;
	SipError err;
	char fn[]="SipSendAck():";
	SipTrans siptrancopy;
	SIP_S8bit *hostname = NULL;
	unsigned short port = 0;

	DEBUG(MSIP, NETLOG_DEBUG4, ("Entering %s\n",fn));

	if(SipTranAck(siptran) == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no ack to send, done\n", fn));
		return 0;	
	}

#if 0
	SipTranAck(siptran)->dRefCount ++;
	memcpy(&siptrancopy, siptran, sizeof(SipTrans));
	siptran = &siptrancopy;
	CacheReleaseLocks(transCache);
#endif

	if(SipTranAckSendhost(siptran))
	{
		/* no need to do DNS again, send message directly */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s resend resp. to host=%lx port=%d\n", fn,
					       SipTranAckSendhost(siptran),
					       SipTranAckSendport(siptran)));
		if( SipSendMessage(SipTranAck(siptran),
				   SipTranAckSendhost(siptran),
				   SipTranAckSendport(siptran), 
				   SipTranAckContext(siptran),
				   SIPPROTO_UDP) < 0)
		{			
			NETERROR(MSIP, ("%s Could not send Ack\n", fn));
			goto _error;
		}
	}
	else
	{
		/* first time to send Ack */

		if(SipTranAckContext(siptran)->pTranspAddr)
		{
			hostname = SipTranAckContext(siptran)->pTranspAddr->dIpv4;
			port = SipTranAckContext(siptran)->pTranspAddr->dPort;
		}
		else
		{
			if ( SipExtractReqUri(SipTranAck(siptran), &req_uri, &err) == SipFail)
			{
				NETERROR(MSIP, 
					 ("%s fail to extract ReqUri from request\n", fn)); 
				goto _error;
			}
#if 1
			/* start new thread, do DNS, save result */
			/* host will be freed by thread */
			if (req_uri->maddr)
			{
				hostname = req_uri->maddr;
			}
			else
			{
				hostname = req_uri->host;
			}
			port = req_uri->port;
		}

		SipTranSendThreadLaunch(SipTranAck(siptran),
					SipTranAckContext(siptran),
					SIPMSG_REQUEST, strdup(hostname),port,
					&SipTranAckSendhost(siptran), 
					&SipTranAckSendport(siptran));
#endif
	}
	
#if 0

	CacheGetLocks(transCache, LOCK_READ, LOCK_BLOCK);

	if( (siptran=CacheGet(transCache, &siptrancopy)) == NULL)
	{
		NETERROR(MSIP, ("%s Cannot store host/port for retransmission\n", fn));
	// free the message
	sip_freeSipMessage(SipTranAck(siptran));
		goto _error;
	}
	
	// free the message
	sip_freeSipMessage(SipTranAck(siptran));
	// copy the host/port
	SipTranAckSendhost(siptran) = SipTranAckSendhost(&siptrancopy);
	SipTranAckSendport(siptran) = SipTranAckSendport(&siptrancopy);
#endif

	SipCheckFree(req_uri);
	return 0;
_error:
	SipCheckFree(req_uri);
	return 0;
}

/* return 0 if no error */
int
SipErrorCSM2(SipTrans *siptran)
{
	char fn[]="SipErrorCSM2";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering, error=%d\n", fn, siptran->error));

	siptran->CSMCallFn = (void *) (SipErrorCSM);
	return 0;
}

static CallRealmInfo *
getRealmInfoFromTran (SipTrans *siptran)
{
	if (SipTranRequestContext(siptran)) {
		if (SipTranRequestContext(siptran)->pData) 
			return RealmInfoDup ((CallRealmInfo *)SipTranRequestContext(siptran)->pData, MEM_LOCAL);
		return NULL;
	}
	if (SipTranResponseContext(siptran)) {
		if (SipTranResponseContext(siptran)->pData)
			return RealmInfoDup ((CallRealmInfo *)SipTranResponseContext(siptran)->pData, MEM_LOCAL);
		return NULL;
	}
	if (SipTranAckContext(siptran)) {
		if (SipTranAckContext(siptran)->pData)
			return RealmInfoDup ((CallRealmInfo *)SipTranAckContext(siptran)->pData, MEM_LOCAL);
		return NULL;
	}
	return NULL;
}

/* return 0 if no error */
int
SipErrorCSM(SipTrans *siptran)
{
	char fn[]="SipErrorCSM";
	SipAppMsgHandle *appMsgHandle=NULL;
	SipMsgHandle *msghandle=NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering, error=%d\n", fn, siptran->error));

	if(siptran->error == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no error to inform, return now\n", fn));
		return 0;
	}

	appMsgHandle = (SipAppMsgHandle *) malloc(sizeof(SipAppMsgHandle));
	bzero(appMsgHandle, sizeof(SipAppMsgHandle));
	appMsgHandle->msgHandle = (SipMsgHandle *) malloc(sizeof(SipMsgHandle));
	bzero(appMsgHandle->msgHandle, sizeof(SipMsgHandle));
	msghandle=appMsgHandle->msgHandle;

	appMsgHandle->tsmError = siptran->error;

	appMsgHandle->incomingPrivType = privacyTypeNone;
	appMsgHandle->privTranslate    = privTranslateNone;
	appMsgHandle->privLevel        = privacyLevelNone;
	
	msghandle->callid = strdup(SipTranKeyCallid(&(siptran->key)));
	msghandle->cseqno = SipTranKeyCseqno(&(siptran->key));
	msghandle->method = strdup(SipTranKeyMethod(&(siptran->key)));
	msghandle->local = UrlDup(SipTranKeyLocal(&(siptran->key)), MEM_LOCAL);
	msghandle->remote = UrlDup(SipTranKeyRemote(&(siptran->key)), MEM_LOCAL);;

	appMsgHandle->realmInfo = getRealmInfoFromTran (siptran);
	
	if( SipTransRecvMsgHandle(appMsgHandle) < 0)
	{
		NETERROR(MSIP,("%s SipTransRecvMsgHandle failed\n",fn));
		goto _error;
	}

	return 0;
 _error:
	SipFreeAppMsgHandle(appMsgHandle);
	return -1;
}

/* return 0 if no error */
int
SipByeCancelErrorCSM2(SipTrans *siptran)
{
	char fn[]="SipByeCancelErrorCSM2";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering, error=%d\n", fn, siptran->error));

	// If this is a REGISTER, send up the error
	// o/w Free up the transaction
	if (!strcmp(siptran->key.method, "REGISTER"))
	{
		siptran->CSMCallFn = (void *) (SipErrorCSM);
	}
	else
	{
		return SipRemoveTSM(siptran);
	}

	return 0;
}

int
SipSendXferMsg(SipTrans* siptran)
{
	
	if( !strcmp(SipTranKeyMethod(&(siptran->key)),"NOTIFY" ) ||
	    !strcmp(SipTranKeyMethod(&(siptran->key)),"REFER") )
	{
		return SipSendResp(siptran);
	}
	else
	{
		return SipNop(siptran);
	}
}

int 
SipByeCancelRemoveTSM(SipTrans* siptran)
{
	char*        fn         = "SipByeCancelRemoveTSM()";
	SipTransKey* siptrankey = NULL;
	
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Entering",fn));

	// Remove REFER transactions	
	if( strcmp( SipTranKeyMethod(&(siptran->key)),"REFER") == 0)
	{
		return SipRemoveTSM(siptran);
	}
	else
	{
		return SipNop(siptran);
	}
}
