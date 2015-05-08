#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include "include/tsmq.h"
#include "ssip.h"
#include <malloc.h>
#include "siputils.h"
#include "thutils.h"
#include "sipbcptinit.h"
#include "bcpt.h"
#include "sipbcptinit.h"
#include "sipbcptfree.h"
#include "sipbcptinit.h"

SipEventContext *SipDupContext (SipEventContext *context);

#define SIP_MSG_BODY_TYPE_SDP     1
#define SIP_MSG_BODY_TYPE_ISUP    2
#define SIP_MSG_BODY_TYPE_QSIG    4
#define SIP_MSG_BODY_TYPE_DTMF    8


SipTrans *
SipAllocateTranHandle(void)
{
	char fn[] = "SipAllocateTranHandle():";
	SipTrans *siptranptr;

	siptranptr = (SipTrans *) MMalloc(transCache->malloc, sizeof(SipTrans));
	if (siptranptr == NULL)
	{
		NETERROR(MSIP,
			("%s No more memory\n", fn));
		return NULL;
	}

	bzero(siptranptr, sizeof(SipTrans));

	siptranptr->currState=IDLE_STATE;
	siptranptr->count_ReTx=0;

	siptranptr->msgs = listInit();
	LockInit(&siptranptr->lock, 0);

	return siptranptr;
}

/* return -1 on error */
int
SipTranKeyFromIncomingMsg( SipMessage *s,
                           SipTransKey *siptrankey,
                           int nbytes )
{
  	char fn[]="SipTranKeyFromIncomingMsg():";
	SipError err;
	en_SipMessageType dType;
	header_url *remote = NULL, *local = NULL;
	char *callid = NULL, *method = NULL;
	int cseqno;

	if (sip_getMessageType(s, &dType, &err) == SipFail)
	{
		NETERROR(MSIP, 
			("%s sip_getMessageType returned error\n", fn));
		goto _error;
	}

	if (dType == SipMessageRequest)
    {
        /* incoming request from network */
        /* Extract the from */
        if (SipExtractFromUri(s, &remote, &err) == SipFail)
        {
            NETERROR(MSIP, ("%s Could not extract from \n", fn));
            goto _error;
        }

        /* Extract the To */
        if (SipExtractToUri(s, &local, &err) == SipFail)
        {
            NETERROR(MSIP, ("%s Could not extract to \n", fn));
            goto _error;
        }
    }
    else if(dType == SipMessageResponse)
    {
        /* incoming response from network */
        /* Extract the from */
        if (SipExtractFromUri(s, &local, &err) == SipFail)
        {
            NETERROR(MSIP, ("%s Could not extract from \n", fn));
            goto _error;
        }

        /* Extract the To */
        if (SipExtractToUri(s, &remote, &err) == SipFail)
        {
            NETERROR(MSIP, ("%s Could not extract to \n", fn));
            goto _error;
        }
    }

	/* Extract the callid */
	if (SipGetCallID(s, &callid, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not extract callid\n", fn));
		goto _error;
	}

	/* Extract the Cseq */
	if (SipGetCSeq(s, &cseqno, &method, &err) == SipFail) 
	{
		NETERROR(MSIP, ("%s Could not extract cseq and method\n", fn));
		goto _error;
	}

	SipFormTranKey((dType == SipMessageRequest) ? SIPTRAN_UAS : SIPTRAN_UAC,
                    remote, local, callid, cseqno, method, siptrankey);

	return 0;
_error:
	SipCheckFree(local);
	SipCheckFree(remote);
	SipCheckFree(callid);
	SipCheckFree(method);
	return -1;
}

/* return -1 on error */
int
SipTranKeyFromOutgoingMsg( SipMessage *s,
                           SipTransKey *siptrankey,
                           int nbytes )
{
	char fn[]="SipTranKeyFromOutgoingMsg():";
	SipError err;
	en_SipMessageType dType;
	header_url *remote = NULL, *local = NULL;
	char *callid = NULL, *method = NULL;
	int cseqno;

	if (sip_getMessageType(s, &dType, &err) == SipFail)
	{
		NETERROR(MSIP, 
			("%s sip_getMessageType returned error\n", fn));
		goto _error;
	}

	if (dType == SipMessageRequest)
	{
		/* outgoing request to network */
		/* Extract the from */
		if (SipExtractFromUri(s, &local, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Could not extract from \n", fn));
			goto _error;
		}

		/* Extract the To */
		if (SipExtractToUri(s, &remote, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Could not extract to \n", fn));
			goto _error;
		}
	}
	else if(dType == SipMessageResponse)
	{
		/* outgoing response to network */
		/* Extract the from */
		if (SipExtractFromUri(s, &remote, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Could not extract from \n", fn));
			goto _error;
		}

		/* Extract the To */
		if (SipExtractToUri(s, &local, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Could not extract to \n", fn));
			goto _error;
		}
	}

	/* Extract the callid */
	if (SipGetCallID(s, &callid, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not extract callid\n", fn));
		goto _error;
	}

	/* Extract the Cseq */
	if (SipGetCSeq(s, &cseqno, &method, &err) == SipFail) 
	{
		NETERROR(MSIP, ("%s Could not extract cseq\n", fn));
		goto _error;
	}

	SipFormTranKey((dType == SipMessageRequest) ? SIPTRAN_UAC : SIPTRAN_UAS,
                    remote, local, callid, cseqno, method, siptrankey);

	return 0;
_error:
	return -1;
}

int
SipTransOperateIncomingMsg( SipTrans *siptranptr,
                            SipMessage *msg, 
                            int newentry,
                            char *method, 
                            SipEventContext *context)
{
	char fn[] = "SipTransOperateIncomingMsg():";
	SipTranSMEntry *siptransm=NULL;
	SIP_S8bit *host=NULL;
	SIP_U16bit hostport;	
	SipMessage *resp=NULL;
	SipError err;
	int lockflag=0;
	int (*CSMCallFn)(void *) = NULL;
	int threadIndex = getMyThreadIndex ();

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	TsmCheckState(siptranptr);

	if(newentry)
	{
#if 0
		/* if initial INVITE, send 100 immediately */
		if( strcmp(method,"INVITE") == 0)
		{
			if (SipFormatResponse(msg, context, &SipTranResponse(siptranptr), 
					      100, "Trying", &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 100\n", fn));
				goto _error;
			}
		}
		else if( strcmp(method,"BYE") == 0 || strcmp(method,"CANCEL") == 0 )
		{
#if ALWAYS_RESPOND_WITH_200_TO_BYECANCEL
			/* if bye or cancel, send 200 0k immediately */
			if (SipFormatResponse(msg, context, &SipTranResponse(siptranptr), 
					      200, "OK", &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 100\n", fn));
				goto _error;
			}
#endif
		}
		else if( strcmp(method,"REFER") == 0 )
		{
#if ALWAYS_RESPOND_WITH_202_TO_REFER
			/* if refer, send 202 Accepted immediately */
			if (SipFormatResponse(msg, context, &SipTranResponse(siptranptr), 
					      202, "Accepted", &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 100\n", fn));
				goto _error;
			}
#endif
		}
#endif
		/* Discard info for new entries */

	}

	if( (strcmp(method, "ACK") == 0) && siptranptr )
	{
		siptransm = 
			&SipInviteSM_Server[siptranptr->currState][SipAckFrNet]; 

		if(siptransm == NULL)
		{
			/* do not send anything to tsm */
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				("%s no need for tsm to act \n", fn));

			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

			goto _return;
		}

		TsmAssignInvServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInvServerEventName(siptranptr, SipAckFrNet);

		if(SipTranAck(siptranptr))
		{
			/* free existing msg and context */
			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;
		}
		else
		{
			/* received ACK from network, deliver to Invite_Server tsm */
			SipTranAck(siptranptr) = msg;
		
			if (SipTranAckContext(siptranptr))
			{
				NETERROR(MSIP, ("%s No Ack but context\n", fn));
				SipFreeContext(context); context = NULL;
			}

			SipTranAckContext(siptranptr) = context;
		}

		siptranptr->event = SipAckFrNet;

	}
	else if( strcmp(method, "INVITE") == 0)
	{
		siptransm = 
			&SipInviteSM_Server[siptranptr->currState][SipRequestFrNet]; 

		if(siptransm == NULL)
		{
			/* do not send anything to tsm */
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				("%s no need for tsm to act \n", fn));

			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

			goto _return;
		}

		TsmAssignInvServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInvServerEventName(siptranptr, SipRequestFrNet);

		if(SipTranRequest(siptranptr) && !newentry)
		{
			/* free existing msg and context */
			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;
			sipStats[threadIndex].invsr++;
		}
	 	else
	 	{

			/* received INVITE from network, 
		 	* deliver to Invite_Server tsm 
		 	*/

			SipTranRequest(siptranptr) = msg;
			SipTranRequestContext(siptranptr) = context;
			SipTranRequestMethod(siptranptr) = strdup(method);
	 	}

		siptranptr->event = SipRequestFrNet;

	}
	else if( strcmp(method, "BYE") == 0 || strcmp(method, "CANCEL") == 0 ||
		 strcmp(method, "REFER") == 0 || strcmp(method, "REGISTER") == 0 ||
	       strcmp(method,"NOTIFY") == 0 )
	{
		siptransm = 
			&SipByeCancelSM_Server[siptranptr->currState][SipRequestFrNet]; 

		if(siptransm == NULL)
		{
			/* do not send anything to tsm */
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				("%s no need for tsm to act \n", fn));

			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

			goto _return;
		}

		TsmAssignOthServerStateName(siptranptr, siptranptr->currState);
		TsmAssignOthServerEventName(siptranptr, SipRequestFrNet);

		if(SipTranRequest(siptranptr) && !newentry)
		{
			/* free existing msg and context */
			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

			if( strcmp(method, "BYE") == 0)
			{
				sipStats[threadIndex].byesr ++;
				NETDEBUG(MSIP, NETLOG_DEBUG4, 
					 ("Incrementing rexmit byes for index %d byes rexmit %d",
					  threadIndex, sipStats[threadIndex].byesr));
			}
			else if (strcmp(method, "CANCEL") == 0)
			{
				sipStats[threadIndex].csr++;
			}
		}
		else
		{

			/* received BYE or CANCEL from network, 
		 	* deliver to ByeCancel_Server tsm 
		 	*/
			SipTranRequest(siptranptr) = msg;
			SipTranRequestContext(siptranptr) = context;
			SipTranRequestMethod(siptranptr) = strdup(method);
		}

		siptranptr->event = SipRequestFrNet;
	}
	else if(strcmp(method, "INFO") == 0)
	{
		siptransm = &SipInfoSM_Server[siptranptr->currState][SipRequestFrNet]; 

		if(siptransm == NULL)
		{
			/* do not send anything to tsm */
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				("%s no need for tsm to act \n", fn));

			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

			goto _return;
		}

		TsmAssignInfoServerStateName(siptranptr, siptranptr->currState);
		TsmAssignInfoServerEventName(siptranptr, SipRequestFrNet);

		if(SipTranRequest(siptranptr) && !newentry)
		{
			/* free existing msg and context */
			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

		}
		else
		{
			/* received INFO from network, deliver to Info_Server tsm */
			SipTranRequest(siptranptr) = msg;
			SipTranRequestContext(siptranptr) = context;
			SipTranRequestMethod(siptranptr) = strdup(method);
		}

		siptranptr->event = SipRequestFrNet;
	}
	else if( (strcmp(method, "RESPONSE") == 0) && siptranptr)
	{
		if( SipGetStatusCode(msg, &SipTranNewresponseCode(siptranptr)) < 0)
		{
			NETERROR(MSIP, ("%s Fail to get status code\n",fn));

			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

			goto _error;
		}

		if (SipTranNewresponse(siptranptr))
		{
			// This is not an error, as the transaction is probably
			// in a state where the response which arrived prior
			// to this was not used, and neither will this be used

			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s New response already exists, error.\n", fn));

			sip_freeSipMessage(msg); msg = NULL;
			SipFreeContext(context); context = NULL;

			goto _error;
		}

		NETDEBUG(MSIP, NETLOG_DEBUG4, 
			("%s received %d response from net, CseqMethod=%s\n",
		       fn,
		       SipTranNewresponseCode(siptranptr),
		       SipTranKeyMethod(&(siptranptr->key))));

		if(strcmp(SipTranKeyMethod(&(siptranptr->key)), "INVITE") == 0)
		{
			/* received response for INVITE */
			if( SipTranNewresponseCode(siptranptr) < 200)
			{
				siptranptr->event = Sip1xxFrNet;
			}
			else
			{
				siptranptr->event = SipFinalFrNet;
			}

			siptransm = 
				&SipInviteSM_Client[siptranptr->currState][siptranptr->event];

			if(siptransm == NULL)
			{
				/* do not send anything to tsm */
				NETDEBUG(MSIP, NETLOG_DEBUG4, 
					("%s no need for tsm to act \n", fn));
	
				sip_freeSipMessage(msg); msg = NULL;
				SipFreeContext(context); context = NULL;

				goto _return;
			}

			if( SipTranNewresponseCode(siptranptr) >= 200)
			{
				// Send ACK for non 2xx final response only
				// when its not be sent yet. Stop doing automatic
				// generation when Inv has SDP.
				if( ( SipTranNewresponseCode(siptranptr) != 200 )
				    && SipTranAck(siptranptr) == NULL)
				{
					/* received non-200 final response or invite has sdp,
					 * send ACK right away */

					if( SipFormatAck(&SipTranAck(siptranptr),
							 SipTranRequest(siptranptr), msg)
					    < 0 )
					{
						NETERROR(MSIP, ("%s error generating ack\n",
								fn));

						sip_freeSipMessage(msg); msg = NULL;
						SipFreeContext(context); context = NULL;

						goto _error;
					}

					SipTranAckSendhost(siptranptr) = SipTranRequestSendhost(siptranptr);
					SipTranAckSendport(siptranptr) = SipTranRequestSendport(siptranptr);

					// Duplicate the context here
					SipTranAckContext(siptranptr) = SipDupContext(context);
				}
				else if (SipTranAck(siptranptr))
				{
					// An ack was already there
					sipStats[threadIndex].invcfr ++;
				}
			}

			TsmAssignInvClientStateName(siptranptr, siptranptr->currState);
			TsmAssignInvClientEventName(siptranptr, siptranptr->event);

			SipTranNewresponse(siptranptr) = msg;
			SipTranNewresponseContext(siptranptr) = context;

		}
		else if(strcmp(SipTranKeyMethod(&(siptranptr->key)), "BYE") == 0 ||
			strcmp(SipTranKeyMethod(&(siptranptr->key)), "CANCEL") == 0 || 
			strcmp(SipTranKeyMethod(&(siptranptr->key)), "REFER") == 0  ||
			strcmp(SipTranKeyMethod(&(siptranptr->key)), "REGISTER") == 0 ||
			strcmp(SipTranKeyMethod(&(siptranptr->key)), "NOTIFY") == 0 )
		{
			/* received response for Cancel or Bye */

			if( SipTranNewresponseCode(siptranptr) < 200)
			{
				siptranptr->event = SipByeCancel1xxFrNet;
			}
			else
			{
				siptranptr->event = SipByeCancelFinalFrNet;
			}

			siptransm = 
				&SipByeCancelSM_Client[siptranptr->currState][siptranptr->event];

			if(siptransm == NULL)
			{
				/* do not send anything to tsm */
				NETDEBUG(MSIP, NETLOG_DEBUG4, 
					("%s no need for tsm to act \n", fn));
	
				sip_freeSipMessage(msg); msg = NULL;
				SipFreeContext(context); context = NULL;

				goto _return;
			}

			TsmAssignOthClientStateName(siptranptr, siptranptr->currState);
			TsmAssignOthClientEventName(siptranptr, siptranptr->event);

			SipTranNewresponse(siptranptr) = msg;
			SipTranNewresponseContext(siptranptr) = context;
		} 
		else if(strcmp(SipTranKeyMethod(&(siptranptr->key)), "INFO") == 0)
		{
			/* received response for Info */

			if( SipTranNewresponseCode(siptranptr) < 200)
			{
				siptranptr->event = SipInfo1xxFrNet;
			}
			else
			{
				siptranptr->event = SipInfoFinalFrNet;
			}

			siptransm = &SipInfoSM_Client[siptranptr->currState][siptranptr->event];

			if(siptransm == NULL)
			{
				/* do not send anything to tsm */
				NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));
	
				sip_freeSipMessage(msg); msg = NULL;
				SipFreeContext(context); context = NULL;

				goto _return;
			}

			TsmAssignInfoClientStateName(siptranptr, siptranptr->currState);
			TsmAssignInfoClientEventName(siptranptr, siptranptr->event);

			SipTranNewresponse(siptranptr) = msg;
			SipTranNewresponseContext(siptranptr) = context;
		} 
	}
	
	if(siptransm == NULL)
	{
		/* do not send anything to tsm */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));

		sip_freeSipMessage(msg); msg = NULL;
		SipFreeContext(context); context = NULL;
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
			NETDEBUG(MSIP, NETLOG_DEBUG1, ("%s CSMCallFn error\n", fn));
			goto _error;
		}
	}

_return:
	SipCheckFree(host);
	return 0;
_error:
	SipCheckFree(host);
	return -1;
}

/* return -1 on error */
int
SipTransProcIncomingMsg( SipMessage *msg, 
                         char *method, 
                         SipEventContext *context,
                         CacheTableInfo *cacheInfo)
{
	char fn[] = "SipTransProcIncomingMsg():";
	SipTranSMEntry *siptransm=NULL;
	SipTransKey siptrankey;
	SipTrans siptran, *siptranptr=NULL;
	int newentry=0;
	SIP_S8bit *host=NULL;
	SIP_U16bit hostport;	
	SipMessage *resp=NULL;
	SipError err;
	int lockflag=0, lockentryflag=0;
	int (*CSMCallFn)(void *) = NULL;
	TsmQEntry *qentry = NULL;
	CallHandle *callHandle = NULL;
	int threadIndex = getMyThreadIndex ();
	SipEventContext *orig_context;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	bzero(&siptrankey, sizeof(siptrankey));

	/* for now,TSM can only handle INVITE, BYE, CANCEL, ACK 
	* requests and responses */

	if(msg->dType == SipMessageRequest)
	{
		if( strcmp(method,"INVITE") && strcmp(method,"BYE") && 
		    strcmp(method,"CANCEL") && strcmp(method,"ACK") &&
		    strcmp(method,"REGISTER") && strcmp (method, "INFO") &&
		    strcmp(method,"REFER")  && strcmp(method, "NOTIFY")
		  )
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s sending 501 to %s request\n",
						       fn, method));
			
			if (SipFormatResponse(msg, context, &resp, 501, "Not Implemented", 
					      &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 100\n", fn));
				goto _error;
			}
			/* host will be freed by thread */
			SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);

			context = NULL;

			goto _error;
		}
	}

	if( SipTranKeyFromIncomingMsg(msg, &siptrankey, sizeof(siptrankey)) < 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s failed to get transaction for incoming message.\n", fn));

		/* error */
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s trying to locate transaction in cache\n", fn));
	PrintSipTranKey(&siptrankey);

#if 0
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s listing all cache entries\n", fn));
	TsmLogCache(0,0,0);
#endif

#if 0
	CacheGetLocks(localConfig.timerPrivate.timerCache, LOCK_READ, LOCK_BLOCK);
#endif

	CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);
	lockflag = 1;

	if( (siptranptr=CacheGet(transCache, &siptrankey)) == NULL)
	{
		/* not found, start new transaction */
		if(strcmp(method,"INVITE") && 
		   strcmp(method,"BYE") && strcmp(method,"CANCEL") &&
		   strcmp(method,"REFER") && strcmp(method,"REGISTER") && 
		   strcmp (method, "INFO") && strcmp(method,"NOTIFY") )
		{
			/* only INVITE, BYE, and CANCEL can start new transaction */
			NETDEBUG(MSIP, NETLOG_DEBUG1,
				("%s %s from network, but no tranx exists\n", fn, method));
			goto _error;
		}

		newentry=1;

		siptranptr= (SipTrans *) SipAllocateTranHandle();

		if(siptranptr == NULL)
		{
			goto _error;
		}
		
		memcpy(&(siptranptr->key), &siptrankey, sizeof(SipTransKey));

		//SipTranRequest(siptranptr) = msg;
		//SipTranRequestContext(siptranptr) = context;
		//SipTranRequestMethod(siptranptr) = strdup(method);

#if 1
		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

		callHandle = CacheGet(sipCallCache, &siptrankey.callLeg);

		CacheReleaseLocks(callCache);

		/* Check for mismatch in call handle's realm id */
		/* if initial INVITE, send 100 immediately */
		if (strcmp(method,"INVITE") == 0)
		{
			if(callHandle || SipTranKeyLocal(&siptrankey)->tag == NULL)
			{
				sipStats[threadIndex].invs ++;

				if (SipFormatResponse(msg, context, &SipTranResponse(siptranptr), 
							  100, "Trying", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 100\n", fn));
					goto _error;
				}
				SipTranResponseContext(siptranptr) = SipDupContext(context);
				SipTranResponseCode(siptranptr) = 100;
			}
			else
			{
				sipStats[threadIndex].notrans ++;

				if (SipFormatResponse(msg, context, &resp, 
							  481, "Call/Transaction Does Not Exist ", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 481\n", fn));
					goto _error;
				}

				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);

				context = NULL;

				goto _error;
			}
		}
		else if( strcmp(method,"BYE") == 0)
		{
			if(callHandle)
			{
				sipStats[threadIndex].byes ++;
				NETDEBUG (MSIP, NETLOG_DEBUG4,
					  ("Incrementing byes on thread %d byes : %d",
					   threadIndex, sipStats[threadIndex].byes));

				/* if bye or cancel, send 200 0k immediately */
				if (SipFormatResponse(msg, context, &SipTranResponse(siptranptr), 
							  200, "OK", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 200\n", fn));
					goto _error;
				}

				SipTranResponseContext(siptranptr) = SipDupContext(context);
				SipTranResponseCode(siptranptr) = 200;
			}
			else
			{
				sipStats[threadIndex].notrans ++;

				if (SipFormatResponse(msg, context, &resp, 
							  481, "Call/Transaction Does Not Exist ", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 481\n", fn));
					goto _error;
				}

				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);
				context = NULL;

				goto _error;
			}
		}
		else if( strcmp(method,"CANCEL") == 0 )
		{
			if(callHandle)
			{
				sipStats[threadIndex].cs ++;

				/* if bye or cancel, send 200 0k immediately */
				if (SipFormatResponse(msg, context, &SipTranResponse(siptranptr),
							  200, "OK", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 200\n", fn));
					goto _error;
				}
				SipTranResponseContext(siptranptr) = SipDupContext(context);
				SipTranResponseCode(siptranptr) = 200;
			}
			else
			{
				sipStats[threadIndex].notrans ++;

				if (SipFormatResponse(msg, context, &resp, 
							  481, "Call/Transaction Does Not Exist ", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 481\n", fn));
					goto _error;
				}

				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);

				context = NULL;

				goto _error;
			}
		}
		else if( strcmp(method,"REFER") == 0 )
		{
			if(callHandle)
			{
#if 0
				// Prior to Call XFER support we always sent a 202 to a REFER
				// With call XFER REFER is relayed to the destination and we will relay
				// back whatever response destination sends us.
				/* if refer, send 202 Accepted immediately */
				/*
				  if (SipFormatResponse(msg, context, &SipTranResponse(siptranptr), 
							  202, "Accepted", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 100\n", fn));
					goto _error;
				}
				SipTranResponseContext(siptranptr) = SipDupContext(context);
				SipTranResponseCode(siptranptr) = 202;
				*/
#endif
			}
			else
			{
				sipStats[threadIndex].notrans ++;

				if (SipFormatResponse(msg, context, &resp,
							  481, "Call/Transaction Does Not Exist ", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 481\n", fn));
					goto _error;
				}

				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);

				context = NULL;

				goto _error;
			}
		} 
		else if( strcmp(method,"NOTIFY") == 0 )
		{
			// Send 200 OK immediately back.
			if(callHandle)
			{
				if( SipFormatResponse(msg,context,&SipTranResponse(siptranptr),
						      200,"OK",&host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 100\n", fn));
					goto _error;
				}
				SipTranResponseContext(siptranptr) = SipDupContext(context);
				SipTranResponseCode(siptranptr) = 200;
			}
			else
			{
				SipHandleIncomingNotifyMessage(msg,method,context);
				goto _error;
			
			}
		}
		else if (strcmp (method, "INFO") == 0) 
		{
			if (!callHandle) 
			{
				sipStats[threadIndex].notrans ++;

				if (SipFormatResponse(msg, context, &resp, 
							  481, "Call/Transaction Does Not Exist ", &host, &hostport, &err) < 0)
				{
					NETERROR(MSIP, ("%s Error in generating 481\n", fn));
					goto _error;
				}

				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);

				context = NULL;

				goto _error;
			}
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("Processing info.."));
		}

		if (strcmp (method, "INFO") != 0)	// No reply from here for INFO
		{
			// Temporarly assign context here so it can be used if DNS has failed
			orig_context = SipTranResponseContext(siptranptr);
			SipTranResponseContext(siptranptr) = context;

			// immediately send the response
			SipSendResp(siptranptr);

			// restore original context
			SipTranResponseContext(siptranptr) = orig_context;
		}
#endif

		/* insert into tranCache */
		if( CacheInsert(transCache,siptranptr) < 0)
		{
			goto _error;
		}
		
		LockGetLock(&siptranptr->lock, LOCK_WRITE, LOCK_BLOCK); lockentryflag = 1;
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s cache found.\n", fn));
		LockGetLock(&siptranptr->lock, LOCK_WRITE, LOCK_BLOCK); lockentryflag = 1;
	}

	if(siptranptr->done)
	{
		sip_freeSipMessage(msg); msg = NULL;
		SipFreeContext(context); context = NULL;

		goto _return;
	}

	if (siptranptr->inuse)
	{
		// Queue the message and return
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Queueing Message\n", fn));

		qentry = TsmNewQEntry();
		qentry->s = msg;
		qentry->from=0;
		qentry->method = strdup(method);
		qentry->context=context;
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
		SipTransOperateMarshall(0, siptranptr, msg, NULL, newentry,
			strdup(method), context);
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
	SipCheckFree(host);
	if(!newentry)
	{
		SipFreeTranKeyMember(&siptrankey);
	}
	return 0;
_error:
	TranFreeQueue(siptranptr);
	MFree(transCache->free, siptranptr);
	sip_freeSipMessage(msg); msg = NULL;
	SipFreeContext(context); context = NULL;
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
	SipCheckFree(host);
	SipFreeTranKeyMember(&siptrankey);
	return -1;
}

/* return -1 on error */
int
SipGetStatusCode( SipMessage *msg,
                  int *pCode )
{
	SipStatusLine *pStatusLine=NULL;
	SipError err;
	char fn[] = "SipGetStatusCode():";

	if( (sip_getStatusLineFromSipRespMsg(msg, &pStatusLine,&err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Fail to get status line from sip msg \n",fn));
		goto _error;
	}

	*pCode = (int) (pStatusLine->dCodeNum);
#if 0
	/* HSS stack does NOT support 487 */
	if( (sip_getStatusCodeFromStatusLine(pStatusLine,pCode,&err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Fail to get status code from status line \n",fn));
		sip_freeSipStatusLine(pStatusLine);
		goto _error;
	}
#endif
	sip_freeSipStatusLine(pStatusLine);
	return 0;

 _error:
	sip_freeSipStatusLine(pStatusLine);
	return -1;
}

/* return -1 on error */
int
SipGetMethod( SipMessage *msg,
              SIP_S8bit **pMethod )
{
	char fn[]="SipGetMethod():";
	SipReqLine *pReqLine = NULL;
	SipError err;

	if( (sip_getReqLine(msg, &pReqLine, &err)) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get reqline\n",fn)); 
		goto _error;
	}

	if( (sip_getMethodFromReqLine(pReqLine, pMethod, &err)) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get method from reqline\n",fn));
		goto _error;
	}

	sip_freeSipReqLine(pReqLine);

	return 0;
 _error:

	sip_freeSipReqLine(pReqLine);
	return -1;
}


/* ported from HSS siptest.c */
int
SipBuildIsupBody (SipMsgBody       **pptrIsupMsgBody,
                  SipAppMsgHandle  *ptrAppMsgHandle)
{
    char         fn[] = "SipBuildIsupBody()";
    SipMsgBody   *pSipMsgBody = NULL ;
	SIP_S8bit    *tmpIsup;
    IsupMessage  *pIsupMsg = NULL ;
    SipError     err = 0;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

    /* Initialize ISUP Message */
    if (sip_bcpt_initIsupMessage(&pIsupMsg,&err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to init isup message, hss error = %d\n", fn, err));
        return -1 ;
    }

    /* Set Body in Isup Message */
	tmpIsup = malloc(ptrAppMsgHandle->isup_msg_len);
	memcpy (tmpIsup, ptrAppMsgHandle->isup_msg, ptrAppMsgHandle->isup_msg_len);
    if (sip_bcpt_setBodyInIsupMessage(pIsupMsg, tmpIsup, ptrAppMsgHandle->isup_msg_len, &err) == SipFail)
    {
        NETERROR(MSIP, ("%s fail to set isup message, hss error = %d\n", fn, err));
        sip_bcpt_freeIsupMessage(pIsupMsg) ;
        return  -1 ;
    }

    /* Initialize Sip Message Body where SipIsupMsgBody will be put */
    if ( sip_initSipMsgBody(&pSipMsgBody,SipIsupBody, &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to init isup message body, hss error = %d\n", fn, err));
        sip_bcpt_freeIsupMessage(pIsupMsg) ;
        return -1 ;
    }

    /* Set Isup-Msg-Body in Sip-Message-Body */
    if ( sip_bcpt_setIsupInMsgBody(pSipMsgBody,pIsupMsg,&err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to set isup message in msg body, hss error = %d\n", fn, err));
        sip_bcpt_freeIsupMessage(pIsupMsg) ;
        sip_freeSipMsgBody(pSipMsgBody) ;
        return -1 ;
    }

    /* Free IsupMessage as it is only required inside the SipMsgBody */
    sip_bcpt_freeIsupMessage(pIsupMsg);
    *pptrIsupMsgBody = pSipMsgBody;
    return 0;

}  /* SipBuildIsupBody */


/* ported from HSS siptest.c */
int
SipBuildQsigBody (SipMsgBody       **pptrQsigMsgBody,
                  SipAppMsgHandle  *ptrAppMsgHandle)
{
    char         fn[] = "SipBuildQsigBody()";
    SipMsgBody  *pSipMsgBody = NULL ;
    QsigMessage *pQsigMsg = NULL ;
    SipError     err = 0;
	SIP_S8bit	*qsig_dup;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

    /* Initialize QSIG Message */
    if (sip_bcpt_initQsigMessage(&pQsigMsg,&err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to init Qsig message, hss error = %d\n", fn, err));
        return -1 ;
    }

    /* Set Body in Qsig Message */
	qsig_dup = malloc(ptrAppMsgHandle->qsig_msg_len);
	if (qsig_dup == NULL)
	{
		NETERROR(MSIP, ("%s MALLOC error, can't set Qsig message\n", fn));
        sip_bcpt_freeQsigMessage(pQsigMsg) ;
		return -1;
	}

	memcpy (qsig_dup, ptrAppMsgHandle->qsig_msg, ptrAppMsgHandle->qsig_msg_len);
    if (sip_bcpt_setBodyInQsigMessage(pQsigMsg, qsig_dup, ptrAppMsgHandle->qsig_msg_len, &err) == SipFail)
    {
        NETERROR(MSIP, ("%s fail to set Qsig message, hss error = %d\n", fn, err));
        sip_bcpt_freeQsigMessage(pQsigMsg) ;
        return  -1 ;
    }

    /* Initialize Sip Message Body where SipQsigMsgBody will be put */
    if ( sip_initSipMsgBody(&pSipMsgBody,SipQsigBody, &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to init qsig message body, hss error = %d\n", fn, err));
        sip_bcpt_freeQsigMessage(pQsigMsg) ;
        return -1 ;
    }

    /* Set Qsig-Msg-Body in Sip-Message-Body */
    if ( sip_bcpt_setQsigInMsgBody(pSipMsgBody,pQsigMsg,&err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to set qsig message in msg body, hss error = %d\n", fn, err));
        sip_bcpt_freeQsigMessage(pQsigMsg) ;
        sip_freeSipMsgBody(pSipMsgBody) ;
        return -1 ;
    }

    /* Free QsigMessage as it is only required inside the SipMsgBody */
    sip_bcpt_freeQsigMessage(pQsigMsg);
    *pptrQsigMsgBody = pSipMsgBody;
    return 0;

}  /* SipBuildQsigBody */


/* This routine build content type header
 * Memory will be allocated for the header when building sucessfully
 */
int
SipBuildContentTypeHeader (SipHeader  **pptrContentTypeHeader,
                           char       *ptrMediaType,
                           char       *ptrVersion,
                           char       *ptrBase)
{
    char       fn[] = "SipBuildContentTypeHeader()";
    SipHeader  *ptrContentTypeHeader = NULL;
    SipParam   *ptrVersionParam = NULL;
    SipParam   *ptrBaseParam = NULL;
    SipError   err = 0;
    int        rc = -1;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));


    /* Initialize Sip Header of type SipHdrTypeContentType */
    if ( sip_initSipHeader(&ptrContentTypeHeader, SipHdrTypeContentType, &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to init content type header, hss error = %d\n",fn, err));
        goto _return;
    }

    /* set content type */
    if (sip_setMediaTypeInContentTypeHdr(ptrContentTypeHeader, strdup(ptrMediaType), &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to set media type, hss error = %d\n",fn, err));
        goto _return;
    }

    if (ptrBase != NULL)
    {
        if ( sip_initSipParam(&ptrBaseParam, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to init base param, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_setNameInSipParam(ptrBaseParam,strdup("base"), &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to set base param, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_insertValueAtIndexInSipParam(ptrBaseParam, strdup(ptrBase), 0, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to set base value, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_insertParamAtIndexInContentTypeHdr(ptrContentTypeHeader, ptrBaseParam, 0, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to add base to content type, hss error = %d\n",fn, err));
            goto _return;
        }
    }

    if (ptrVersion != NULL)
    {
        if ( sip_initSipParam(&ptrVersionParam, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to init version param, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_setNameInSipParam(ptrVersionParam, strdup("version"), &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to set version param, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_insertValueAtIndexInSipParam(ptrVersionParam, strdup(ptrVersion), 0, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to set version value, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_insertParamAtIndexInContentTypeHdr(ptrContentTypeHeader, ptrVersionParam, 0, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to add verion content type, hss error = %d\n",fn, err));
            goto _return;
        }
    }

    *pptrContentTypeHeader = ptrContentTypeHeader;
    rc = 0;

_return:
    if (ptrVersionParam != NULL)
    {
        sip_freeSipParam(ptrVersionParam) ;
    }
    if (ptrBaseParam != NULL)
    {
        sip_freeSipParam(ptrBaseParam) ;
    }

    if (rc < 0)
    {
        if (ptrContentTypeHeader != NULL)
        {
			sip_freeSipHeader(ptrContentTypeHeader);
			free(ptrContentTypeHeader);
        }
    }

    return rc;

} /* SipBuildContentTypeHeader */


/* This routine build content disposition header
 * Memory will be allocated for the header when building sucessfully
 */
int
SipBuildContentDispositionHeader (SipHeader  **pptrContentDispositionHeader,
                                  char       *ptrDisposition,
                                  char       *ptrHandling)
{
    char       fn[] = "SipBuildContentDispositionHeader()";
    SipHeader  *ptrContentDispositionHeader = NULL;
    SipParam   *ptrHandlingParam = NULL;
    SipError   err = 0;
    int        rc = -1;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

    /* Initialize Sip Header of type SipHdrTypeContentDisposition */
    if ( sip_initSipHeader(&ptrContentDispositionHeader, SipHdrTypeContentDisposition, &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to init content disposition header, hss error = %d\n",fn, err));
        goto _return;
    }
    if (sip_setDispTypeInContentDispositionHdr(ptrContentDispositionHeader, strdup(ptrDisposition), &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to set content disposition header, hss error = %d\n",fn, err));
        goto _return;
    }

    if (ptrHandling != NULL)
    {
        if ( sip_initSipParam(&ptrHandlingParam, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to init content disposition param, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_setNameInSipParam(ptrHandlingParam, strdup("handling"), &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to set content disposition param, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_insertValueAtIndexInSipParam(ptrHandlingParam, strdup(ptrHandling), 0, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to set content disposition param value, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_insertParamAtIndexInContentDispositionHdr(ptrContentDispositionHeader, ptrHandlingParam, 0, &err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to add content disposition, hss error = %d\n",fn, err));
            goto _return;
        }
    }

    *pptrContentDispositionHeader = ptrContentDispositionHeader;
    rc = 0;

_return:
    if (ptrHandlingParam != NULL)
    {
        sip_freeSipParam(ptrHandlingParam) ;
    }

    if (rc < 0)
    {
        if (ptrContentDispositionHeader != NULL)
        {
            sip_freeSipHeader(ptrContentDispositionHeader);
            free(ptrContentDispositionHeader);
        }
    }

    return rc;

} /* SipBuildContentDispositionHeader */


/* This routine build single message body
 * the message body could be:
 *   DTMF
 *   SDP
 *   ISUP
 *   QSIG
 */
int SipFormSingleMsgBodyFromMsgHandle( SipMessage      *ptrSipMsg,
                                       SipAppMsgHandle *ptrAppMsgHandle)
{
    char        fn[]="SipFormSingleMsgBodyFromMsgHandle()";
    SipHeader   *ptrContentTypeHeader=NULL;
    SipHeader   *ptrContentDispositionHeader=NULL;
    SipMsgBody  *ptrMsgBody = NULL ;
    SipError    err;
    int         rc = -1;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

    /* sdp msg body*/
    if (ptrAppMsgHandle->localSet != NULL)
    {
        return (SipFormSDPFromMsgHandle(ptrSipMsg, ptrAppMsgHandle) );
    }

	/* dtmf message body */
	if (ptrAppMsgHandle->ndtmf != 0)
	{
        if (SipBuildContentTypeHeader (&ptrContentTypeHeader, "application/dtmf-relay", NULL, NULL) < 0)
        {
            NETERROR(MSIP, ("%s fail to build content type hdr for isup, hss error = %d\n", fn, err));
            goto _return;
        }
		if (rc=SipFormDtmfFromMsgHandle (&ptrMsgBody, ptrAppMsgHandle) < 0)
		{
			NETERROR (MSIP, ("%s failed to set dtmf in msg\n", fn));
		}
	}
	else if (ptrAppMsgHandle->isup_msg != NULL)
    {
        if (SipBuildContentTypeHeader (&ptrContentTypeHeader, "application/ISUP",
            ptrAppMsgHandle->isupTypeVersion, ptrAppMsgHandle->isupTypeBase) < 0)
        {
            NETERROR(MSIP, ("%s fail to build content type hdr for isup, hss error = %d\n", fn, err));
            goto _return;
        }
		/* build content disposition header */
		if (ptrAppMsgHandle->isupDisposition != NULL)
		{
			if (SipBuildContentDispositionHeader (&ptrContentDispositionHeader,
				ptrAppMsgHandle->isupDisposition, ptrAppMsgHandle->isupHandling) < 0)
			{
				NETERROR(MSIP, ("%s fail to build content disposition hdr, hss error = %d\n", fn, err));
				goto _return;
			}
		}
        if (SipBuildIsupBody(&ptrMsgBody, ptrAppMsgHandle) < 0)
        {
            NETERROR(MSIP, ("%s fail to build isup msg body\n", fn));
            goto _return;
        }
    }
    else if (ptrAppMsgHandle->qsig_msg != NULL)
    {
        if (SipBuildContentTypeHeader (&ptrContentTypeHeader, "application/QSIG",
            ptrAppMsgHandle->qsigTypeVersion, NULL) < 0)
        {
            NETERROR(MSIP, ("%s fail to init content type hdr for qsig, hss error = %d\n", fn, err));
            goto _return;
        }
		if (ptrAppMsgHandle->qsigDisposition != NULL)
		{
			if (SipBuildContentDispositionHeader (&ptrContentDispositionHeader,
				ptrAppMsgHandle->qsigDisposition, ptrAppMsgHandle->qsigHandling) < 0)
			{
				NETERROR(MSIP, ("%s fail to build content disposition hdr, hss error = %d\n", fn, err));
				goto _return;
			}
		}
        if (SipBuildQsigBody(&ptrMsgBody, ptrAppMsgHandle) < 0)
        {
            NETERROR(MSIP, ("%s fail to build msg body\n", fn));
            goto _return;
        }
    }

    if( sip_setHeader(ptrSipMsg, ptrContentTypeHeader, &err) == SipFail)
    {
        NETERROR(MSIP, ("%s fail to set content type hdr in msg, hss error = %d\n", fn, err));
        goto _return;
    }

    if( (ptrContentDispositionHeader != NULL)  &&
        (sip_insertHeaderAtIndex(ptrSipMsg, ptrContentDispositionHeader, 0, &err) == SipFail))
    {
        NETERROR(MSIP, ("%s fail to set content type hdr in msg, hss error = %d\n", fn, err));
        goto _return;
    }

    /* insert message body */
    if( sip_insertMsgBodyAtIndex(ptrSipMsg, ptrMsgBody, 0, &err) == SipFail)
    {
        NETERROR(MSIP, ("%s fail to set sdp in msg, hss error = %d \n", fn, err));
        goto _return;
    }

    /* done */
    rc = 0;

 _return:
    if (ptrContentTypeHeader != NULL)
    {
        sip_freeSipHeader(ptrContentTypeHeader);
        free(ptrContentTypeHeader);
    }
    if (ptrContentDispositionHeader != NULL)
    {
        sip_freeSipHeader(ptrContentDispositionHeader);
        free(ptrContentDispositionHeader);
    }
	if (ptrMsgBody)
		sip_freeSipMsgBody(ptrMsgBody);

    return rc;

} /* SipFormSingleMsgBodyFromMsgHandle */


/* This routine builds multipart mime header
 * for each message body
 */
int
SipFormBuildMimeHeaderFromMsgHandle (SipMessage       *ptrSipMsg,
                                     SipAppMsgHandle  *ptrAppMsgHandle,
                                     int              bodyType,
                                     SipMimeHeader    **pptrMimeHeader)
{
    char           fn[] = "SipFormBuildMimeHeaderFromMsgHandle()";
    SipMimeHeader  *ptrMimeHdr = NULL;
    SipHeader      *ptrContentTypeHeader = NULL ;
    SipHeader      *ptrContentDispositionHeader = NULL ;
    char           mediaType[64] = "";
    char           *ptrVersion = NULL;
    char           *ptrBase = NULL;
    char           *ptrDisposition = NULL;
    char           *ptrHandling = NULL;
    SipParam       *ptrVersionParam = NULL;
    SipParam       *ptrBaseParam = NULL;
    SipParam       *ptrHandlingParam = NULL;
    SipError       err = 0;
    int            rc = -1;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

    /* Set MediaType in content type header*/
    switch (bodyType)
    {
    case SIP_MSG_BODY_TYPE_SDP:
         strcpy (mediaType, "application/SDP");
         break;
    case SIP_MSG_BODY_TYPE_ISUP:
         strcpy (mediaType, "application/ISUP");
         ptrVersion = ptrAppMsgHandle->isupTypeVersion;
         ptrBase = ptrAppMsgHandle->isupTypeBase;
         ptrDisposition = ptrAppMsgHandle->isupDisposition;
         ptrHandling = ptrAppMsgHandle->isupHandling;
         break;
    case SIP_MSG_BODY_TYPE_QSIG:
         strcpy (mediaType, "application/QSIG");
         ptrVersion = ptrAppMsgHandle->qsigTypeVersion;
         //ptrBase = ptrAppMsgHandle->qsigTypeBase;
         ptrDisposition = ptrAppMsgHandle->qsigDisposition;
         ptrHandling = ptrAppMsgHandle->qsigHandling;
         break;
    case SIP_MSG_BODY_TYPE_DTMF:
         strcpy (mediaType, "application/dtmf-relay");
         break;
    default:
        NETERROR(MSIP, ("%s invalid media type = %d\n",fn, bodyType));
        goto _return;
    }

    /* Initialize Mime Header */
    if ( sip_bcpt_initSipMimeHeader(&ptrMimeHdr, &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to init mime header, hss error = %d\n",fn, err));
        goto _return;
    }

    /* build content type header */
    if (SipBuildContentTypeHeader (&ptrContentTypeHeader, mediaType, ptrVersion, ptrBase) < 0)
    {
        NETERROR(MSIP, ("%s fail to init content type header, hss error = %d\n",fn, err));
        goto _return;
    }


    if ( sip_bcpt_setContentTypeInMimeHdr(ptrMimeHdr, ptrContentTypeHeader, &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to content type in mime header, hss error = %d\n",fn, err));
        goto _return;
    }

    /* build content disposition header */
    if (ptrDisposition != NULL)
    {
        if ( SipBuildContentDispositionHeader(&ptrContentDispositionHeader, ptrDisposition, ptrHandling) < 0)
        {
            NETERROR(MSIP, ("%s fail to init content disposition header, hss error = %d\n",fn, err));
            goto _return;
        }

        if ( sip_bcpt_setContentDispositionInMimeHdr(ptrMimeHdr, ptrContentDispositionHeader,&err) == SipFail )
        {
            NETERROR(MSIP, ("%s fail to init content disposition header, hss error = %d\n",fn, err));
            goto _return;
        }
    }

    /* done */
    rc = 0;
    *pptrMimeHeader = ptrMimeHdr;

_return:
    if (ptrContentTypeHeader != NULL)
    {
        sip_freeSipHeader(ptrContentTypeHeader);
        free(ptrContentTypeHeader);
    }
    if (ptrContentDispositionHeader != NULL)
    {
        sip_freeSipHeader(ptrContentDispositionHeader);
        free(ptrContentDispositionHeader);
    }
    if (rc < 0)
    {
        if (ptrMimeHdr != NULL)
        {
            sip_bcpt_freeSipMimeHeader(ptrMimeHdr);
        }
    }

    return rc;

} /* SipFormBuildMimeHeaderFromMsgHandle */


/* This routine builds multipart mime body
 * for each type
 */
int
SipFormMultipartMsgBodyFromMsgHandle(SipMessage       *ptrSipMsg,
                                     SipAppMsgHandle  *ptrAppMsgHandle,
                                     int              bodyType)
{
    char            fn[]="SipFormMultipartMsgBodyFromMsgHandle()";
    SipMimeHeader   *ptrMimeHdr = NULL ;
    SipMsgBody      *ptrMsgBody = NULL;
    SipError        err = 0;
    int             rc = -1;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

    /* Form mime header */
    if (SipFormBuildMimeHeaderFromMsgHandle(ptrSipMsg, ptrAppMsgHandle, bodyType, &ptrMimeHdr) < 0)
    {
        NETERROR(MSIP, ("%s fail to build sdp Mime header\n",fn));
        goto _return;
    }

    /* Form body */
    switch (bodyType)
    {
    case SIP_MSG_BODY_TYPE_SDP:
        if (SipFormSDPBodyFromMsgHandle(ptrSipMsg, ptrAppMsgHandle, &ptrMsgBody) < 0)
        {
            NETERROR(MSIP, ("%s fail to form sdp body in msg\n", fn));
            goto _return;
        }
        break;
    case SIP_MSG_BODY_TYPE_ISUP:
        if (SipBuildIsupBody(&ptrMsgBody, ptrAppMsgHandle) < 0)
        {
            NETERROR(MSIP, ("%s fail to form isup body in msg\n", fn));
            goto _return;
        }
        break;
    case SIP_MSG_BODY_TYPE_QSIG:
        if (SipBuildQsigBody(&ptrMsgBody, ptrAppMsgHandle) < 0)
        {
            NETERROR(MSIP, ("%s fail to form qsig body in msg\n", fn));
            goto _return;
        }
        break;
    case SIP_MSG_BODY_TYPE_DTMF:
        if (SipFormDtmfFromMsgHandle(&ptrMsgBody, ptrAppMsgHandle) < 0)
        {
            NETERROR(MSIP, ("%s fail to form dtmf body in msg\n", fn));
            goto _return;
        }
        break;
    default:
        NETERROR(MSIP, ("%s fail to form unknown body in msg\n", fn));
        goto _return;
    }

    if (sip_bcpt_setMimeHeaderInMsgBody(ptrMsgBody, ptrMimeHdr, &err) == SipFail )
    {
        NETERROR(MSIP, ("%s fail to set mime head in sdp body in msg, hss error = %d\n", fn, err));
        goto _return;
    }

    if ( sip_listInsertAt(&(ptrSipMsg->slMessageBody), 0, (void*)ptrMsgBody, &err) == SipFail)
    {
        NETERROR(MSIP, ("%s fail to insert sdp body to msg, hss error = %d\n", fn, err));
    }

    /* done */
    rc = 0;

_return:
    if (ptrMimeHdr != NULL)
    {
        sip_bcpt_freeSipMimeHeader(ptrMimeHdr);
    }

    if (rc < 0)
    {
        if (ptrMsgBody != NULL)
        {
            sip_freeSipMsgBody(ptrMsgBody);
        }
    }

    return rc;

} /* SipFormMultipartMsgBodyFromMsgHandle */


/* This routine builds multipart mixed message bodies
 */
int SipFormMultipartMixedBodyFromMsgHandle( SipMessage      *ptrSipMsg,
                                            SipAppMsgHandle *ptrAppMsgHandle)
{
    char  fn[]="SipFormMultipartMixedBodyFromMsgHandle()";

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

    /* form qsig body */
    if (ptrAppMsgHandle->qsig_msg != NULL)
    {
        if (SipFormMultipartMsgBodyFromMsgHandle(ptrSipMsg, ptrAppMsgHandle, SIP_MSG_BODY_TYPE_QSIG) < 0)
        {
            NETERROR(MSIP, ("%s fail to build qsig msg body\n",fn));
            return -1;
        }
    }

    /* form isup body */
    if (ptrAppMsgHandle->isup_msg != NULL)
    {
        if (SipFormMultipartMsgBodyFromMsgHandle(ptrSipMsg, ptrAppMsgHandle, SIP_MSG_BODY_TYPE_ISUP) < 0)
        {
            NETERROR(MSIP, ("%s fail to build isup msg body\n",fn));
            return -1;
        }
    }

    /* form dtmf body */
	if (ptrAppMsgHandle->ndtmf != 0)
    {
        if (SipFormMultipartMsgBodyFromMsgHandle(ptrSipMsg, ptrAppMsgHandle, SIP_MSG_BODY_TYPE_DTMF) < 0)
        {
            NETERROR(MSIP, ("%s fail to build dtmf msg body\n",fn));
            return -1;
        }
    }

    /* form sdp body */
    if (ptrAppMsgHandle->localSet != NULL)
    {
        if (SipFormMultipartMsgBodyFromMsgHandle(ptrSipMsg, ptrAppMsgHandle, SIP_MSG_BODY_TYPE_SDP) < 0)
        {
            NETERROR(MSIP, ("%s fail to build sdp msg body\n",fn));
            return -1;
        }
    }


    return 0;

} /* SipFormMultipartMixedBodyFromMsgHandle */


/* sipmessage is allocated in this function
 * via headers for response are set OUTSIDE of this function
 */
int
SipFormMsgFromMsgHandle( SipMessage **s,
                         SipAppMsgHandle *AppMsgHandle )
{
    char fn[]="SipFormMsgFromMsgHandle()";
    SipError err;
    SipMessage *m;
    SipMsgBody *msgBody = NULL;
//	SipMsgBody *dtmfMsgBody = NULL;
	SipMsgHandle *msghandle=AppMsgHandle->msgHandle;
	SipHeader *contentTypeHeader = NULL;
	SipParam *boundaryParam = NULL ;
	struct timeval tp;
	char boundaryValue[256];
	en_SipMessageType dType;
	int statuscode;
	header_url *tmpptr = NULL;
	SIP_S8bit *tmphost = NULL, tmparray[1024], *viahost = NULL, branchtoken[64];
	unsigned short port;
	char rsadomain[24], *rsadomainp = "";
	int  msgBodyCount = 0;

	dType = msghandle->msgType;
	statuscode = msghandle->responseCode;
	if( sip_initSipMessage(s, dType, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s error init msg \n", fn));
		goto _error;
	}
	m = *s;

	if(dType == SipMessageRequest)
	{
		if(msghandle->remotecontact)
		{
			tmpptr = msghandle->remotecontact;
		}
		else if(msghandle->requri)
		{
			tmpptr = msghandle->requri;
		}
		if(tmpptr == NULL)
		{
			NETERROR(MSIP, ("%s fail to construct requri\n", fn));
			goto _error;
		}

		if( SipSetReqUri(m, tmpptr, msghandle->method, &err) < 0)
		{
			NETERROR(MSIP, ("%s error setting requri in msg \n",fn));
			goto _error;
		}
#if 0
		if(msghandle->nroutes > 0)
		{
			/* install route hdr and pop top route to requri */
			if( SipSetRoute(m, msghandle->nroutes, 
					msghandle->routes, msghandle->remotecontact) < 0)
			{
				NETERROR(MSIP, ("%s fail to set route\n", fn));
				goto _error;
			}

			if( SipPopRoute(m, &tmphost, &port, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to poproute\n", fn));
				SipCheckFree(tmphost);
				goto _error;
			}
			SipCheckFree(tmphost);
		}
#endif
#if 0
		/* for testing only */
		AppMsgHandle->hdrWwwauthenticate = strdup("realm=\"1234\", nonce=\"45678\"");
#endif
		/* cook up proxyauthorization hdr */
		if(AppMsgHandle->hdrProxyauthenticate &&
		   AppMsgHandle->hdrProxyauthorization == NULL)
		{
			if( SipMakeAuthorizationString(m, AppMsgHandle->hdrProxyauthenticate, SipHdrTypeProxyauthorization, &tmparray[0]) < 0)
			{
				NETERROR(MSIP, ("%s fail to make proxyauth string\n", fn));
				goto _error;
			}
			AppMsgHandle->hdrProxyauthorization = strdup(tmparray);
		}
		/* cook up authorization hdr */
		if(AppMsgHandle->hdrWwwauthenticate &&
		   AppMsgHandle->hdrAuthorization == NULL)
		{
			if( SipMakeAuthorizationString(m, AppMsgHandle->hdrWwwauthenticate, SipHdrTypeAuthorization, &tmparray[0]) < 0)
			{
				NETERROR(MSIP, ("%s fail to make proxyauth string\n", fn));
				goto _error;
			}
			AppMsgHandle->hdrAuthorization = strdup(tmparray);
		}

		/* form proxyauthorization hdr */
		if(AppMsgHandle->hdrProxyauthorization && strcasecmp(msghandle->method, "INVITE") == 0)
		{
			if( sip_insertHeaderFromStringAtIndex(m, SipHdrTypeProxyauthorization, AppMsgHandle->hdrProxyauthorization, 0, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set proxyauth hdr\n",fn));
				goto _error;
			}
		}
		/* form authorization hdr */
		if(AppMsgHandle->hdrAuthorization &&
			(strcasecmp(msghandle->method, "INVITE") == 0 || strcasecmp(msghandle->method, "REGISTER") == 0))
		{
			if( sip_insertHeaderFromStringAtIndex(m, SipHdrTypeAuthorization, AppMsgHandle->hdrAuthorization, 0, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set auth hdr\n",fn));
				goto _error;
			}
		}

		/* Insert session timer headers */
		if (strcasecmp (msghandle->method, "INVITE") == 0)
		{
			if (SipSetSupported (m, strdup("timer")) < 0)
			{
				NETERROR (MSIP, ("%s failed to set supported header\n", fn));
				goto _error;
			}
			if (AppMsgHandle->minSE != sipminSE)
			{
				if (SipSetMinSE (m, AppMsgHandle->minSE?AppMsgHandle->minSE:sipminSE)  < 0)
				{
					NETERROR (MSIP, ("%s failed to set minSE header\n", fn));
					goto _error;
				}
			}
			if  (SipSetSessionExpires (m, AppMsgHandle->sessionExpires, AppMsgHandle->refresher) < 0)
			{
				NETERROR (MSIP, ("%s failed to set sessionExpires header.\n", fn));
				goto _error;
			}
		}
		
		if (SipSetMaxForwards(m, AppMsgHandle->maxForwards) < 0)
		{
			NETERROR(MSIP, ("%s Unable to set Max-Forward\n", fn));
			goto _error;
		}
	}
	else if(dType == SipMessageResponse)
	{
		if( SipSetStatusLine(m, statuscode, (char *) SipGetReason(statuscode)) < 0)
		{
			NETERROR(MSIP, ("%s error setting status line in msg\n",fn));
			goto _error;
		}
		/* form Proxyauthenticate hdr */
		if(AppMsgHandle->hdrProxyauthenticate)
		{
			if( sip_insertHeaderFromStringAtIndex(m, SipHdrTypeProxyAuthenticate, AppMsgHandle->hdrProxyauthenticate, 0, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set proxyauthen hdr\n",fn));
				goto _error;
			}
		}
		/* form Wwwauthenticate hdr */
		if(AppMsgHandle->hdrWwwauthenticate)
		{
			if( sip_insertHeaderFromStringAtIndex(m, SipHdrTypeWwwAuthenticate, AppMsgHandle->hdrWwwauthenticate, 0, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set wwwauthen hdr\n",fn));
				goto _error;
			}
		}
		if (statuscode == 422)
		{
			if (SipSetMinSE (m, sipminSE) < 0)
			{
				NETERROR (MSIP, ("%s fail to set minSE ", fn));
				goto _error;
			}
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s setting minse to %d", fn, sipminSE));
		}
		else if (AppMsgHandle->timerSupported && statuscode == 200)
		{
			if (SipSetRequire (m, strdup("timer")) < 0)
			{
				NETERROR (MSIP, ("%s failed to set require header",fn));
				goto _error;
			}
			if (SipSetSessionExpires (m, AppMsgHandle->sessionExpires, AppMsgHandle->refresher) < 0)
			{
				NETERROR (MSIP, ("%s fail to set session expires.", fn));
				goto _error;
			}
		}
	}

	/* set To Header */
	if( SipSetToFromHdr(m, SipMsgTo(msghandle), 0) < 0 )
	{
		NETERROR(MSIP, ("%s fail to set to header \n",fn));
		goto _error;
	}
	
	/* set From Header */
	if( SipSetToFromHdr(m, SipMsgFrom(msghandle), 1) < 0 )
	{
		NETERROR(MSIP, ("%s fail to set From header \n",fn));
		goto _error;
	}
	// Sip Privcy Related changes -
	// Copy privacy headers if required to outbound side.

	if (strcasecmp (msghandle->method, "INVITE") == 0) {

                if(AppMsgHandle->generate_cid == cid_block )
                {
                        AppMsgHandle->privLevel = privacyLevelId;
                        SipGeneratePrivacyHdr(m,AppMsgHandle);   
                }
                else 
                {
                        PrivacyTranslateHdrs(m,AppMsgHandle);
                }                
        }

	 /*******************************************/
         /* Insert expires header. */
	 /* added for gens only. */
         if ((AppMsgHandle->expires >=0) &&
         		(strcasecmp (msghandle->method, "REGISTER") == 0))
         {
         	if (SipSetExpiresHdr(m, AppMsgHandle->expires) < 0)
         	{
         		NETERROR (MSIP, ("%s failed to set expires header\n", fn));
         		goto _error;
         	}
         }

        /*******************************************/


	if( sip_form_callid_inmsg(m, SipMsgCallID(msghandle), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set call id\n",fn));
		goto _error;
	}

	if( sip_form_cseqnum_inmsg(m, SipMsgCallCSeqNo(msghandle),
				   SipMsgCallCseqMethod(msghandle), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set cseq\n",fn));
		goto _error;
	}

	if(dType == SipMessageRequest)
	{
		if (AppMsgHandle->realmInfo->rsa)
		{
			FormatIpAddress(AppMsgHandle->realmInfo->rsa, rsadomain);
			rsadomainp = rsadomain;
		}
		else if (AppMsgHandle->realmInfo->sipdomain)
		{
			rsadomainp = AppMsgHandle->realmInfo->sipdomain;
		}
		else
		{
			NETERROR(MSIP, ("%s fail to set via, as rsa is 0 and sipdomain is not set\n",fn));
			goto _error;
		}

		SipHashViaBranch(m, branchtoken,&err);

		if( SipInsertRealmVia(m, SIPPROTO_UDP, branchtoken, rsadomainp, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set via\n",fn));
			goto _error;
		}
	}

	if(msghandle->localcontact)
	{
		if( sip_form_contacthdr_inmsg(m, msghandle->localcontact, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set call id\n",fn));
			goto _error;
		}
	}

	if(msghandle->replaces)
	{
		if(sip_insertHeaderFromStringAtIndex(m,SipHdrTypeReplaces,msghandle->replaces,0,&err) == SipFail)
		{
			NETERROR(MSIP,("%s:Unable to insert replaces hdr ",fn));
		}
	}

	if(dType == SipMessageRequest &&
	   strcmp(SipMsgCallCseqMethod(msghandle), "REFER") == 0)
	{
		if( SipSetReferToByHdr(m, msghandle->referto, msghandle->referby) < 0)
		{
			NETERROR(MSIP, ("%s fail to set referto/by in msg\n", fn));
			goto _error;
		}
	}

	// Set all Notify specific headers
	if(dType == SipMessageRequest &&
	   strcmp(SipMsgCallCseqMethod(msghandle), "NOTIFY") == 0)
	{
		
		if(AppMsgHandle->allow)
		{
			if( sip_insertHeaderFromStringAtIndex(m,SipHdrTypeAllow,AppMsgHandle->allow,
							      0,&err) == SipFail )
			{
				NETERROR(MSIP,("%s:Unable to insert Allow hdr ",fn));
			}
		}
		if(AppMsgHandle->event)
		{
			if( sip_insertHeaderFromStringAtIndex(m,SipHdrTypeEvent,AppMsgHandle->event,
							      0,&err) == SipFail )
			{
				NETERROR(MSIP,("%s:Unable to insert Event hdr ",fn));
			}	  
		}
		if(AppMsgHandle->supported)
		{
			if( sip_insertHeaderFromStringAtIndex(m,SipHdrTypeSupported,AppMsgHandle->supported,
							      0,&err) == SipFail )
			{
				NETERROR(MSIP,("%s:Unable to insert Supported hdr ",fn));
			}	  
		}
		if(AppMsgHandle->sub_state)
		{
			if(sip_insertHeaderFromStringAtIndex(m,SipHdrTypeSubscriptionState,AppMsgHandle->sub_state,
							     0,&err) == SipFail )
			{
				NETERROR(MSIP,("%s:Unable to insert Sub-State hdr ",fn));
			}	  
		}
		
		if(AppMsgHandle->sip_frag)
		{
			if( sip_insertHeaderFromStringAtIndex(m,SipHdrTypeContentType,AppMsgHandle->content_type,
							      0,&err) == SipFail )
			{
				NETERROR(MSIP,("%s:Unable to insert ContentHdr hdr ",fn));
			}
			SipInsertSipFrag(m,AppMsgHandle);			
		}
		
	}
	
	SetSipUnknownHeaders(m, AppMsgHandle);

	// add the xconnid
	if (msghandle->xConnId)
	{
		if (useXConnId &&
			(SetSipCustomHeader(m, strdup("X-Connection-Id"),
				strdup(msghandle->xConnId), &err) == SipFail))
		{
			NETERROR(MSIP, ("%s fail to set xconnid\n",fn));
		}
	}

	if(msghandle->alert_info)
	{
		if( sip_insertHeaderFromStringAtIndex(m,SipHdrTypeAlertInfo,msghandle->alert_info, 
						      0, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s failed to set alert-info hdr\n",fn));
			goto _error;
		}
	}


	if(strcmp(msghandle->method, "BYE") == 0 && msghandle->also)
	{
		if( sip_insertHeaderFromStringAtIndex(m, SipHdrTypeAlso, msghandle->also, 0, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set also header\n",fn));
		}
	}

    msgBodyCount = (AppMsgHandle->ndtmf ? 1 : 0) +
					(AppMsgHandle->nlocalset ? 1 : 0) +
                   (AppMsgHandle->isup_msg ? 1 : 0) +
                   (AppMsgHandle->qsig_msg ? 1 : 0);
	switch(msgBodyCount)
	{
	case 0:
		// no content to send
		break;
	case 1:
		if( SipFormSingleMsgBodyFromMsgHandle (m, AppMsgHandle) < 0)
		{
			NETERROR(MSIP, ("%s fail to set sdp in msg\n", fn));
			goto _error;
		}
		break;
	default:
		// need to use multipart mime
		if (sip_initSipHeader (&contentTypeHeader, SipHdrTypeContentType, &err) == SipFail)
		{
			NETERROR (MSIP, ("%s fail to init content type hdr.\n", fn));
			goto _error;
		}

		if (sip_setMediaTypeInContentTypeHdr (contentTypeHeader, strdup ("multipart/mixed"), &err) == SipFail)
		{
			NETERROR (MSIP, ("%s fail to set content type in hdr\n", fn));
			goto _error;
		}

		if (sip_initSipParam(&boundaryParam, &err) == SipFail)
		{
			goto _error;
		}

		if (sip_setNameInSipParam(boundaryParam, strdup("boundary"), &err) == SipFail)
		{
			goto _error;
		}

		gettimeofday (&tp, NULL);
		sprintf (boundaryValue, "%lu-%ld-%ld", iServerIP, tp.tv_sec, tp.tv_usec);
		if (sip_insertValueAtIndexInSipParam(boundaryParam, strdup(boundaryValue), 0, &err) == SipFail)
		{
			goto _error;
		}

		if (sip_insertParamAtIndexInContentTypeHdr(contentTypeHeader, boundaryParam, 0, &err) == SipFail)
		{
			goto _error;
		}
		sip_freeSipParam(boundaryParam);

		if (sip_setHeader(m, contentTypeHeader, &err) == SipFail)
		{
			goto _error;
		}

		if (SipFormMultipartMixedBodyFromMsgHandle(m, AppMsgHandle) < 0)
    	{
       		 NETERROR(MSIP, ("%s fail to set multipart mixed body in msg\n", fn));
    	}

		break;
	}

	if (contentTypeHeader)
	{
		sip_freeSipHeader(contentTypeHeader);
		free(contentTypeHeader);
	}
	return 0;

 _error:
	if (contentTypeHeader)
	{
		sip_freeSipHeader(contentTypeHeader);
		free(contentTypeHeader);
	}
	if (boundaryParam)
	{
		sip_freeSipParam(boundaryParam);
	}
	sip_freeSipMessage(*s);
	*s=NULL;
	return -1;
}

int SipFormCallRealmInfo (SipEventContext *context, 
			  SipAppMsgHandle *appMsgHandle)
{
	CallRealmInfo *ri;
	char fn[] = "SipFormCallRealmInfo()";

	if (context == NULL || context->pData == NULL) {
		NETERROR (MSIP, ("%s %s is NULL, Failed to build realm Info",
				 fn, context ? "Realm Info in context": "Context"));
		return -1;
	}

	ri = (CallRealmInfo *) context->pData;
	if ((appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (ri, MEM_LOCAL)) == NULL)
	{
		NETERROR (MSIP, ("%s RealmInfoDup Failed !", fn));
		return -1;
	}
	return 0;

}
int
SipFormMsgHandleFromMsg( SipMessage *m,
                         SipAppMsgHandle *appMsgHandle )
{
	char fn[]="SipFormMsgHandleFromMsg():";
	SipMsgHandle *msghandle=appMsgHandle->msgHandle;
	header_url *tmpptr = NULL;
	SipError err;
	en_SipMessageType dType;
	int count, i, j;
	SIP_S8bit *strptr=NULL;
	int                   hdrCount;
	
	char             cidblk[CID_BLK_UNBLK_LEN];
        char             cidunblk[CID_BLK_UNBLK_LEN];
	CacheRealmEntry* realmEntryPtr = NULL; 


	if(msghandle==NULL)
	{
		NETERROR(MSIP,("%s msghandle error\n",fn));
		goto _error;
	}

	if(appMsgHandle->realmInfo == NULL)
	{
		NETERROR(MSIP,("%s msghandle error\n",fn));
		goto _error;
	}

	if (sip_getMessageType(m, &dType, &err) == SipFail)
	{
		NETERROR(MSIP, 
			 ("%s sip_getMessageType returned error\n", fn));
		goto _error;
	}
	
	msghandle->msgType = dType;
	msghandle->origin = 0;		/* 1 = local, 0 = remote */

	if(dType == SipMessageRequest)
	{
		if( SipExtractReqUri(m, &tmpptr, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s SipExtractRequri faled\n",fn));
			goto _error;
		}

		if(tmpptr != NULL)
		{
			SipPhoneContextPlus(&tmpptr->name);
			
			// On casual look it appears as though we will have memory freeing problems since
			// we are advancing tmpptr->name in the code below. 
			// But in reality, we are ok because the way SipExtractReqUri function works and due to the
			// fact that we are UrlDuping right after that.			
			
                        if(tmpptr->name)
                        {                

                                appMsgHandle->generate_cid = cid_noaction;

				memset(cidblk, 0 , CID_BLK_UNBLK_LEN );
				memset(cidunblk, 0 , CID_BLK_UNBLK_LEN );
				
				CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
				realmEntryPtr = CacheGet(realmCache, &(appMsgHandle->realmInfo->realmId));				
				if (realmEntryPtr)
				{
					if(realmEntryPtr->realm.cidblk && strlen(realmEntryPtr->realm.cidblk))
					{
						strncpy(cidblk,realmEntryPtr->realm.cidblk,
							strlen(realmEntryPtr->realm.cidblk));
					}
					if(realmEntryPtr->realm.cidunblk && strlen(realmEntryPtr->realm.cidunblk))
					{
						strncpy(cidunblk,realmEntryPtr->realm.cidunblk,
							strlen(realmEntryPtr->realm.cidunblk));
					}        

				}
				CacheReleaseLocks(realmCache);
                              
                                if(strlen(cidblk) && strncmp(tmpptr->name,cidblk,strlen(cidblk))==0 )
                                {                                
                                        tmpptr->name += strlen(cidblk);
                                        appMsgHandle->generate_cid = cid_block;
                                } 
                                else if (strlen(cidunblk) && strncmp(tmpptr->name,cidunblk,strlen(cidunblk))==0 )
                                {
                                        tmpptr->name += strlen(cidunblk);
                                        appMsgHandle->generate_cid = cid_unblock;
                                }
                        }

			msghandle->requri = UrlDup(tmpptr, MEM_LOCAL);
			appMsgHandle->requri = UrlDup(tmpptr, MEM_LOCAL);
			free(tmpptr);
		}

		/* source host and port of request from network */
		if (SipGetSentByHost(m, NULL, 0, 0, &(msghandle->srchost), 
				     &(msghandle->srcport), &err) <= 0)
		{
			NETERROR(MSIP, ("%s fail to get sentby from top via\n", fn));
			goto _error;
		}

		/* extract proxyauthorization hdr */

		if( (SipExtractHdrAsString(m, SipHdrTypeProxyauthorization, 
					   &strptr, 0)) <= 0 )
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				 ("%s no Proxyauthorization hdr found\n", fn));
		}
		else
		{
			appMsgHandle->hdrProxyauthorization = strptr;
		}
		/* extract authorization hdr */
		if( (SipExtractHdrAsString(m, SipHdrTypeAuthorization, 
					   &strptr, 0)) <= 0 )
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				 ("%s no Authorization hdr found\n", fn));
		}
		else
		{
			appMsgHandle->hdrAuthorization = strptr;
		}

		if(SipGetMaxForwards(m, &appMsgHandle->maxForwards) < 0)
		{
			appMsgHandle->maxForwards = sipmaxforwards;
		}
		if (SipGetSessionTimerSupport (m, &appMsgHandle->timerSupported) < 0) 
		{
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s no timer supported header.\n", fn));
			appMsgHandle->timerSupported = 0;
		}
		if (SipGetMinSE (m, &appMsgHandle->minSE) < 0) 
		{
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s no minSE hdr found\n", fn));
			appMsgHandle->minSE = sipminSE;
		}
		if (SipGetSessionExpires(m, &appMsgHandle->sessionExpires, &appMsgHandle->refresher) < 0)
		{
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s no sessionExpires hdr found", fn));
			appMsgHandle->sessionExpires = sipsessionexpiry;
			appMsgHandle->refresher = SESSION_REFRESHER_NONE;
		}

		strptr = NULL;
		if( (SipExtractHdrAsString(m, SipHdrTypeReplaces,&strptr, 0)) )
		{
			msghandle->replaces = strptr;
		}

		// Privacy Related Enhancements - Ticket 5377
		appMsgHandle->incomingPrivType = privacyTypeNone;
                appMsgHandle->privTranslate    = privTranslateNone;
                appMsgHandle->privLevel        = privacyLevelNone;

                tmpptr = NULL;                

		if(SipIsPAssertedIDPresent(m,&hdrCount,&err) == SipSuccess) {
		  
		  appMsgHandle->incomingPrivType = privacyTypeRFC3325;
                  appMsgHandle->pAssertedID_Tel  = NULL;                  

                  if(SipStorePAssertedIDTel(m,&(strptr),hdrCount,&err) == SipSuccess)
                  {
                          appMsgHandle->pAssertedID_Tel = strptr;
                  }
                  
		  // RFC 3325 - Privacy Related Headers
		  appMsgHandle->pAssertedID_Sip = NULL;
		  if( SipExtractFromPAssertedID(m,&(tmpptr),
						hdrCount,&err) == SipFail ) {
		    
		    NETDEBUG(MSIP,NETLOG_DEBUG4,("No PAssertedID Header found"));
		    appMsgHandle->pAssertedID_Sip = NULL;
		  }

                  if(tmpptr != NULL)
                  {
                          appMsgHandle->pAssertedID_Sip = UrlDup(tmpptr, MEM_LOCAL);
                          free(tmpptr);
                  }
                  tmpptr = NULL;
                  
		  // Privacy Header and P-Assereted-ID header go togehter
		  hdrCount = 0;
		  appMsgHandle->privLevel  = privacyLevelNone;
		  if ( SipIsPrivacyPresent(m,&hdrCount,&err) == SipSuccess) {
	  
		    appMsgHandle->priv_value = NULL;
		    appMsgHandle->privLevel  = privacyLevelNone;
		    SipExtractFromPrivacy(m,&(appMsgHandle->priv_value),&err);
		    if(appMsgHandle->priv_value != NULL && (strcmp(appMsgHandle->priv_value, "id")==0)) {
		      appMsgHandle->privLevel = privacyLevelId;
		    }
		  }
		}
		else if(SipIsRemotePartyIdPresent(m,&hdrCount,&err) == SipSuccess) {
		  // Sip privacy Draft privacy headers	
		  appMsgHandle->incomingPrivType = privacyTypeDraft01;
		  appMsgHandle->rpid_hdr = NULL;

		  tmpptr = NULL;
		  if( SipExtractFromRemotePartyHdr(m,&(tmpptr),&err) == SipFail){
		    NETDEBUG(MSIP,NETLOG_DEBUG4,("Error extracting from RPID Hdr"));
		  }
		  if(tmpptr)
		  {
			  appMsgHandle->rpid_hdr = UrlDup(tmpptr,MEM_LOCAL);
			  free(tmpptr);
		  }

		  SipExtractPrivacyLevelFromRPID(m,&(appMsgHandle->priv_value),&err);
		  if(appMsgHandle->priv_value != NULL && (strcmp(appMsgHandle->priv_value, "full")==0)) {
			  appMsgHandle->privLevel = privacyLevelId;
		  }
		  strptr = NULL;
		  if( SipExtractHdrAsString(m,SipHdrTypeDcsRemotePartyId, &strptr, 0 )  )
		  {
			  appMsgHandle->rpid_url = (strptr);
			  NETDEBUG(MSIP,NETLOG_DEBUG4,("%s strptr %p appmsg %p",fn,strptr,appMsgHandle->rpid_url));
		  }

		  if(SipIsProxyRequirePresent(m,&hdrCount,&err) == SipSuccess) {
			  strptr = NULL;			  
			  if( SipExtractHdrAsString( m,SipHdrTypeProxyRequire,&strptr,&err)  )
			  {
				  appMsgHandle->proxy_req_hdr = (strptr);
			  }
		  }
		}
	}
	else
	{
		if( SipGetStatusCode(m,  &(msghandle->responseCode)) < 0)
		{
			NETERROR(MSIP, ("%s SipGetStatusCode failed\n",fn));
			goto _error;
		}
		if(msghandle->responseCode == 401 || msghandle->responseCode == 407)
		{
			/* extract Proxyauthenticate hdr */
			if( (SipExtractHdrAsString(m, SipHdrTypeProxyAuthenticate, 
						   &strptr, 0)) <= 0 )
			{
				NETDEBUG(MSIP, NETLOG_DEBUG4,
					 ("%s no Proxyauthenticate hdr found\n", fn));
			}
			else
			{
				appMsgHandle->hdrProxyauthenticate = strptr;
			}
			/* extract Wwwauthenticate hdr */
			if( (SipExtractHdrAsString(m, SipHdrTypeWwwAuthenticate, 
						   &strptr, 0)) <= 0 )
			{
				NETDEBUG(MSIP, NETLOG_DEBUG4,
					 ("%s no Wwwauthenticate hdr found\n", fn));
			}
			else
			{
				appMsgHandle->hdrWwwauthenticate = strptr;
			}
		}
		if (msghandle->responseCode == 200) 
		{
			/* Extract session timer headers */
			if (SipGetSessionTimerRequire (m, &appMsgHandle->timerSupported) < 0)
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s: Failed to get timer require header", fn));
				appMsgHandle->timerSupported = 0;
			}
			if (SipGetSessionExpires (m, &appMsgHandle->sessionExpires,
						  &appMsgHandle->refresher) < 0) 
			{
				if (appMsgHandle->timerSupported)
				{
					/* Session expires header is required if require timer headr is present */
					NETERROR (MSIP, ("%s: Failed to get session expires header", fn));
					goto _error;
				}
				else
				{
					NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no SessionExpires hdr found\n", fn));
				}
			}
			else
			{
				appMsgHandle->timerSupported = 1;
			}
		}
		if (msghandle->responseCode == 422)
		{
			if (SipGetMinSE  (m, &appMsgHandle->minSE) < 0) 
			{
				NETERROR (MSIP, ("%s failed to get minSE header", fn));
				goto _error;
			}
		}
	}


	GetSipUnknownHeaders(m, appMsgHandle);

	tmpptr = NULL;

	if( SipExtractFromUri(m, &tmpptr, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to extract from hdr\n",fn));
		goto _error;
	}

	if(tmpptr != NULL)
	{
		SetSipMsgFrom(msghandle, UrlDup(tmpptr, MEM_LOCAL));
		free(tmpptr);
	}

	tmpptr = NULL;

	if( SipExtractToUri(m, &tmpptr, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to extract To hdr\n",fn));
		goto _error;
	}

	if(tmpptr != NULL)
	{
		SetSipMsgTo(msghandle, UrlDup(tmpptr, MEM_LOCAL));
		free(tmpptr);
	}

	tmpptr = NULL;

	if( SipExtractContactList(m, &msghandle->remotecontact_list, 
		appMsgHandle->realmInfo->realmId, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to extract list of contact hdr\n",fn));
		goto _error;
	}

	msghandle->remotecontact = SipPopUrlFromContactList(&msghandle->remotecontact_list, MEM_LOCAL);

	if( SipGetCallID(m, &SipMsgCallID(msghandle), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to extract call id hdr\n",fn));
		goto _error;
	}

	if( SipGetCSeq(m, &SipMsgCallCSeqNo(msghandle), 
		       &SipMsgCallCseqMethod(msghandle), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to extract CSeq hdr\n",fn));
		goto _error;
	}

	count = 0;
	if (sip_getHeaderCount(m, SipHdrTypeRecordRoute, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get RR count\n",fn));
		goto _error;
	}
	msghandle->nroutes = count;
	if(count > 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s found %d RR Hdr\n", fn, count));
		msghandle->routes = (char **)malloc(count*sizeof(char *));
		for(i=0;i<count;i++)
		{
			if( (SipExtractHdrAsString(m, SipHdrTypeRecordRoute, 
						   &strptr, i)) <= 0 )
			{
				NETERROR(MSIP, ("%s fail to get RR hdr from index %d\n", 
						fn, i));
				goto _error;
			}

			/* for incoming request, the RR order should be kept
			 * for incoming response, the order should be reversed 
			 */
			if(dType == SipMessageRequest)
			{
				(msghandle->routes)[i] = strdup(strptr + strlen("Record-"));
			}
			else
			{
				(msghandle->routes)[count-1-i] = strdup(strptr + strlen("Record-"));
			}

			SipCheckFree(strptr);
		}
	}

	if(dType == SipMessageRequest && 
	   strcmp(SipMsgCallCseqMethod(msghandle), "REFER") == 0)
	{
		tmpptr = NULL;

		if( SipExtractReferToUri(m, &tmpptr, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to extract ReferTo hdr\n",fn));
			goto _error;
		}

		if(tmpptr != NULL)
		{
			msghandle->referto = UrlDup(tmpptr, MEM_LOCAL);
			free(tmpptr);
		}

		tmpptr = NULL;

		if( SipExtractReferByUri(m, &tmpptr, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to extract Referby hdr\n",fn));
			goto _error;
		}

		if(tmpptr != NULL)
		{
			msghandle->referby = UrlDup(tmpptr, MEM_LOCAL);
			free(tmpptr);
		}

		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s referto name=%s host=%s\n", fn,
					       msghandle->referto->name,
					       msghandle->referto->host));
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s referby name=%s host=%s\n", fn,
					       msghandle->referby->name,
					       msghandle->referby->host));
	}
	
	// Extract Notify related parameters
	if(dType == SipMessageRequest &&
	   strcmp(SipMsgCallCseqMethod(msghandle), "NOTIFY") == 0 )
	{

		strptr = NULL;
		if(SipExtractHdrAsString(m,SipHdrTypeAllow,&(strptr),0) )
		{
			appMsgHandle->allow = strptr;
		}
		strptr = NULL;
		if(SipExtractHdrAsString(m,SipHdrTypeEvent,&(strptr),0)  )
		{
			appMsgHandle->event = (strptr);
		}
		strptr = NULL;
		if(SipExtractHdrAsString(m,SipHdrTypeSubscriptionState,&(strptr),0)  )
		{
			appMsgHandle->sub_state = (strptr);
		}
		strptr = NULL;
		if(SipExtractHdrAsString(m,SipHdrTypeSupported,&(strptr), 0)  )
		{
			appMsgHandle->supported = (strptr);
		}
		strptr = NULL;
		if(SipExtractHdrAsString(m,SipHdrTypeContentType,&(strptr), 0)  )
		{
			appMsgHandle->content_type = (strptr);
		}
	}

	// check for xconnid header
	if (useXConnId &&
		(GetSipCustomHeader(m, "X-Connection-Id", 
			&msghandle->xConnId, &err) == SipFail))
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s no xconnid header present\n", fn));
	}

	strptr = NULL;	
	if( (SipExtractHdrAsString(m, SipHdrTypeAlertInfo,&strptr, 0)) <= 0 )
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s no alert-info hdr found\n", fn));
	}
	else
	{
		msghandle->alert_info = (strptr);
	}
		
	if(strcmp(SipMsgCallCseqMethod(msghandle), "BYE") == 0)
	{
		if(SipExtractHdrAsString(m, SipHdrTypeAlso, &strptr, 0) > 0)
		{
			msghandle->also = strptr;
		}
	}

	return 0;
 _error:
	return -1;
}

int
SipFreeTranKeyMember( SipTransKey *siptrankey )
{
	char fn[]="SipFreeTranKeyMember";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("entering %s\n", fn));

	if(siptrankey == NULL)
	{
		return 0;
	}

	SipCheckFree(SipTranKeyLocal(siptrankey));
	SipCheckFree(SipTranKeyRemote(siptrankey));
	SipCheckFree(SipTranKeyCallid(siptrankey));
	SipCheckFree(SipTranKeyMethod(siptrankey));
	
	return 0;
}

#define NOTNULL(x) ((x)?(x):"")

int
PrintSipTranKey( SipTransKey *keyptr )
{
	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("callid =%s Local={user=%s host=%s port=%d tag=%s} Remote={user=%s host=%s port=%d tag=%s} Cseq=%d %s type=%d\n", 
				       NOTNULL(SipTranKeyCallid(keyptr)), 
				       NOTNULL(SipTranKeyLocal(keyptr)->name), 
				       NOTNULL(SipTranKeyLocal(keyptr)->host), 
				       SipTranKeyLocal(keyptr)->port, 
				       NOTNULL(SipTranKeyLocal(keyptr)->tag),
				       NOTNULL(SipTranKeyRemote(keyptr)->name),
				       NOTNULL(SipTranKeyRemote(keyptr)->host),
				       SipTranKeyRemote(keyptr)->port,
				       NOTNULL(SipTranKeyRemote(keyptr)->tag),
				       SipTranKeyCseqno(keyptr),
				       NOTNULL(SipTranKeyMethod(keyptr)), 
					   SipTranKeyType(keyptr) ));
	return 0;
}

int
PrintfSipTranKey( SipTransKey *keyptr )
{
	printf("callid =%s Local={user=%s host=%s port=%d tag=%s} Remote={user=%s host=%s port=%d tag=%s} Cseq=%d %s type=%d\n", 
				       NOTNULL(SipTranKeyCallid(keyptr)), 
				       NOTNULL(SipTranKeyLocal(keyptr)->name), 
				       NOTNULL(SipTranKeyLocal(keyptr)->host), 
				       SipTranKeyLocal(keyptr)->port, 
				       NOTNULL(SipTranKeyLocal(keyptr)->tag),
				       NOTNULL(SipTranKeyRemote(keyptr)->name),
				       NOTNULL(SipTranKeyRemote(keyptr)->host),
				       SipTranKeyRemote(keyptr)->port,
				       NOTNULL(SipTranKeyRemote(keyptr)->tag),
				       SipTranKeyCseqno(keyptr),
				       NOTNULL(SipTranKeyMethod(keyptr)), 
					   SipTranKeyType(keyptr) );
	return 0;
}

int
TsmLogCache(int where, int module, int level)
{
	SipTrans *siptranptr=NULL;
	int invs = 0, invc = 0, byes = 0, byec = 0, cc = 0, cs = 0;

	CacheGetLocks(transCache, LOCK_READ, LOCK_BLOCK);
	
#if 1
	for (siptranptr = CacheGetFirst(transCache);
			siptranptr; 
			siptranptr = CacheGetNext(transCache, &siptranptr->key))
	{
		//PrintfSipTranKey(&(siptranptr->key));
		if ((!strcmp(siptranptr->key.method, "INVITE") &&
			siptranptr->key.type == SIPTRAN_UAC))
		{
			invc++;
		}
		else if ((!strcmp(siptranptr->key.method, "INVITE") &&
			siptranptr->key.type == SIPTRAN_UAS))
		{
			invs++;
		}
		else if ((!strcmp(siptranptr->key.method, "BYE") &&
			siptranptr->key.type == SIPTRAN_UAC))
		{
			byec++;
		}
		else if ((!strcmp(siptranptr->key.method, "BYE") &&
			siptranptr->key.type == SIPTRAN_UAS))
		{
			byes++;
		}
		else if ((!strcmp(siptranptr->key.method, "CANCEL") &&
			siptranptr->key.type == SIPTRAN_UAC))
		{
			cc++;
		}
		else if ((!strcmp(siptranptr->key.method, "CANCEL") &&
			siptranptr->key.type == SIPTRAN_UAS))
		{
			cs++;
		}
	}

	CacheFreeIterator(transCache);

#endif
	printf("TSM no of items=%d\n", transCache->nitems);
	printf("TSMdest no of items=%d\n", transDestCache->nitems);
	printf("TSM invs=%d, invc=%d, byes=%d, byec=%d, cs=%d, cc=%d\n",
		invs, invc, byes, byec, cs, cc);

	CacheReleaseLocks(transCache);
	return(0);
}

int
TranFreeQueue(SipTrans *siptranptr)
{
	TsmQEntry *qentry;

	if (siptranptr == NULL)
	{
		return 0;
	}

	while (qentry = listGetFirstItem(siptranptr->msgs))
	{
		listDeleteItem(siptranptr->msgs, qentry);
		if (qentry->s)
		{
			sip_freeSipMessage(qentry->s);
		}
		SipCheckFree(qentry->method);
		SipFreeContext(qentry->context);

		if (qentry->timer)
		{
			TsmFreeTsmKey(timerGetCbData(qentry->timer));
			timerFreeHandle(qentry->timer);
		}

		free(qentry);
	}

	listDestroy(siptranptr->msgs);

	return 0;
}

TsmQEntry *
TsmNewQEntry(void)
{
	TsmQEntry *qentry;
	
	qentry = malloc(sizeof(TsmQEntry));
	memset(qentry, 0, sizeof(TsmQEntry));

	return qentry;
}

SipTransKey *
TsmDupTsmKey(SipTransKey *siptrankey)
{
	char fn[] = "TsmDupTsmKey():";
	SipTransKey *dupkey;

	dupkey = (SipTransKey *)malloc(sizeof(SipTransKey));
	if (dupkey)
	{
		dupkey->callLeg.callid = strdup(siptrankey->callLeg.callid);
		dupkey->callLeg.local = UrlDup(siptrankey->callLeg.local, MEM_LOCAL);
		dupkey->callLeg.remote = UrlDup(siptrankey->callLeg.remote, MEM_LOCAL);
		dupkey->method = strdup(siptrankey->method);
		dupkey->type = siptrankey->type;
		dupkey->cseqno = siptrankey->cseqno;
	}
	else
	{
		NETERROR(MINIT, ("%s no memory\n", fn));
	}

	return dupkey;
}	

int
TsmFreeTsmKey(SipTransKey *siptrankey)
{
	if (siptrankey != NULL)
	{
		SipCheckFree(siptrankey->callLeg.callid);
		UrlFree(siptrankey->callLeg.local, MEM_LOCAL);
		UrlFree(siptrankey->callLeg.remote, MEM_LOCAL);
		SipCheckFree(siptrankey->method);
		memset(siptrankey, 0xa, sizeof(SipTransKey));
		SipCheckFree(siptrankey);
	}

	return 0;
}	

void
AddTSMDest(SipTrans *siptranptr)
{
	char fn[] = "AddTSMDest():";
	SipTrans *tmp;
	SipTransDestKey key;
	CallRealmInfo *realmInfo;

	if (siptranptr->prev || siptranptr->next)
	{
		NETERROR(MSIP, ("%s Transaction already part of dest cache\n",
			fn));
		return;
	}

	if (strcmp(siptranptr->key.method, "INVITE"))
	{
		// Start only for INVITES
		return;
	}

	if (!siptranptr->request.context)
	{
		NETERROR(MCACHE, ("%s context is null!\n", fn));
		return;
	}

	realmInfo = (CallRealmInfo *)siptranptr->request.context->pData;
	
	if (realmInfo == NULL)
	{
		NETERROR(MCACHE, ("%s realmInfo is null!\n", fn));
		return;
	}

	if (siptranptr->key.type == SIPTRAN_UAC)
	{

		key.srcip = realmInfo->rsa;
		key.destip = SipTranRequestSendhost(siptranptr);
		key.destport = SipTranRequestSendport(siptranptr);

		// If this insert is successful,
		ListgInitElem(siptranptr, sizeof(SipTransKey));
	
		if (tmp = CacheGet(transDestCache, &key))
		{
			ListgInsert(tmp, siptranptr, sizeof(SipTransKey));
		}
		else
		{
			CacheInsert(transDestCache, siptranptr);
		}
	}
}

void
DelTSMDest(SipTrans *siptranptr)
{
	char fn[] = "DelTSMDest():";
	SipTrans *tmp;
	SipTransDestKey key;
	CallRealmInfo *realmInfo;

	if (!siptranptr->prev || !siptranptr->next)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Transaction not part of dest cache\n",
			fn));
		return;
	}

	if (siptranptr->key.type != SIPTRAN_UAC)
	{
		return;
	}

	if (strcmp(siptranptr->key.method, "INVITE"))
	{
		// Start only for INVITES
		return;
	}

	if (!siptranptr->request.context)
	{
		NETERROR(MCACHE, ("%s context is null!\n", fn));
		return;
	}

	realmInfo = (CallRealmInfo *)siptranptr->request.context->pData;
	
	if (realmInfo == NULL)
	{
		NETERROR(MCACHE, ("%s realmInfo is null!\n", fn));
		return;
	}

	key.srcip = realmInfo->rsa;
	key.destip = SipTranRequestSendhost(siptranptr);
	key.destport = SipTranRequestSendport(siptranptr);

	if (tmp = CacheDelete(transDestCache, &key))
	{
		if (!(ListgIsSingle(tmp, sizeof(SipTransKey))))
		{
			tmp = (SipTrans *)Listg(siptranptr, sizeof(SipTransKey))->next;
			ListgDelete(siptranptr, sizeof(SipTransKey));
		
			ListgResetElem(siptranptr, sizeof(SipTransKey));
			CacheInsert(transDestCache, tmp);
		}
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Transaction not present in dest cache\n",
			fn));
	}
}

int
SipFormTranKey(int type, header_url *remote, header_url *local, 
				char *callid, int cseqno, char *method, SipTransKey *siptrankey)
{
	SipTranKeyType(siptrankey) = type;
	SipTranKeyRemote(siptrankey) = remote;
	SipTranKeyLocal(siptrankey) = local;
	SipTranKeyCallid(siptrankey) = callid;
	SipTranKeyCseqno(siptrankey) = cseqno;
	SipTranKeyMethod(siptrankey) = method;

	return 0;
}
