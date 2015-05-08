#include "gis.h"
#include "sgen.h"
#include <malloc.h>
#include "nxosd.h"

extern unsigned long chanLocalAddr;
extern unsigned short chanLocalPort;
extern int finalRespCode;
extern int manualAccept;
extern int inviteType, reInvType;
extern Call*	Calls;
extern Call*	CallsOut;
extern int fax;
extern int blindXferMode, attXferMode, waitForNotify;
extern char xferTgtNum[256], replaceStr[1024];
extern char contact[256];
extern char *contactUser, *contactHost, **callIDIn;
extern int contactPort;
extern unsigned int tdSetup, tdConnect, tdIdle, tdMonitor;
extern int asrMode;
extern int automode;
extern int isupMode, qsigMode;
extern int nFailedInvites;
extern double iprobability;
extern int sipPrivRfcMode, sipPrivDftMode, sipPrivDualMode;

extern int SgenIdleTimerProcessor(struct Timer *timer);
extern int sgenInform(SipEventHandle *evb);

/* Process event from UA, and reverse it */
int
SipUASendToBridge(SipEventHandle *evb)
{
	char fn[] = "sgen SipUASendToBridge():";
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	ConfHandle		*confHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	char callID[CALL_ID_LEN], confID[CONF_ID_LEN];
	SCC_EventBlock *evPtr;
	int i = 0, forward = 1;
	SipEventHandle *evHandle = NULL;
	SDPAttr *attr = NULL;
	int callFinalResp = 0;
	int origin = 0, createNewCall = 0;
	unsigned long laddr = 0;
 	struct 	itimerval calltmr;
	void *timerdata;
	int hold = 0, hold3264 = 0;
	int resumeInvite = 0;
	int temp = 0;
	int ua_eventprocessed = 0;

	SipAppCallHandle        *pSipData;

	appMsgHandle = SipEventAppHandle(evb);

	RealmInfoFree(appMsgHandle->realmInfo, MEM_LOCAL);
	appMsgHandle->realmInfo = NULL;

	memcpy(callID, appMsgHandle->callID, CALL_ID_LEN);
	memcpy(confID, appMsgHandle->confID, CONF_ID_LEN);

	switch (evb->event)
	{
	case Sip_eNetworkInvite:
		
		if (asrMode)
                {
                        // Calculate the random probability for dropping the INVITE.
                        // If probability <= iprobability, process the INVITE,
                        // else drop it.
                        if (drand48() > iprobability)
                        {
                                finalRespCode = 486;
                                nFailedInvites++;
                        }
                }
		/* Added for Sip-T. */
		/* If we are talking ISUP and receive a QSIG message, send a 415. */
		/* If we are talking QSIG and receive ISUP, accept it. */
		if ((appMsgHandle->isup_msg) || (appMsgHandle->qsig_msg))
                {
                        // Check if SIP-T mode enabled.
                        if ((!isupMode && !qsigMode) || (isupMode && appMsgHandle->qsig_msg))
                        {
                                /* Not in any kind of Sip-T mode, send a 415.*/
                                finalRespCode = 415;
                        }
                }

		/* Added for Sip-Privacy. */
		/* If endpoint is sip-privacy enabled, verify the headers received.*/
		if ((appMsgHandle->pAssertedID_Sip) || (appMsgHandle->rpid_url))
		{
			// Received a Sip-Privacy header. Check if SIP-Priv enabled

			if ((sipPrivDualMode || sipPrivRfcMode) && appMsgHandle->pAssertedID_Sip)
				fprintf(stdout,"Received a RFC3325 format header.\n");

			else if((sipPrivDualMode || sipPrivDftMode) && appMsgHandle->rpid_url)
				fprintf(stdout,"Received a Draft format header.\n"); 

			else
				fprintf(stdout, "Sip_Privacy Disabled on endpoint.\n");

			// Received privacy headers in INVITE, remove them from 200OK.
			//Added for privacy
                        appMsgHandle->incomingPrivType = privacyTypeNone;
                        appMsgHandle->privTranslate    = privTranslateNone;
                        appMsgHandle->privLevel        = privacyLevelNone;

			
		}
		if (finalRespCode)
		{
			callFinalResp = finalRespCode;

			if ((finalRespCode < 400) && contactUser && contactHost)
			{
				// Insert a contact to be returned in the final response
				CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

				callHandle = CacheGet(callCache, callID);
				sipCallHandle = SipCallHandle(callHandle);

				MFree(callCache->free, sipCallHandle->localContact->name);
				MFree(callCache->free, sipCallHandle->localContact->host);
				sipCallHandle->localContact->name = 
					CStrdup(callCache, contactUser);
				sipCallHandle->localContact->host = 
					CStrdup(callCache, contactHost);
				sipCallHandle->localContact->port = contactPort;
				
				CacheReleaseLocks(callCache);
			}
		}
		else
		{
			CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

			callHandle = CacheGet(callCache, callID);
			sipCallHandle = SipCallHandle(callHandle);

			if (callHandle)
			{
				if (sipCallHandle->successfulInvites == 0)
				{
					i = SgenGetFreeCallNo(0);
				
					if (i >= 0)
					{
						// allocate this
						Calls[i].state = Sip_sConnectedAck;
						callHandle->callNo = i;
						callIDIn[i] = (char *) calloc(CALL_ID_LEN, sizeof(char));
						memcpy(callIDIn[i], callID, CALL_ID_LEN);

						// check if the invite has the replaces header
						if(appMsgHandle->msgHandle->replaces)
						{
							// this is a transfer invite
							// close the first call
							DisconnectCall(callIDIn[0],Sip_eBridgeBye);
						}
						/* Keeping a count of connected calls. Added for bug_7796 */
					}
					else
					{
						NETERROR(MSIP, ("%s No more calls can be allocated\n", fn));
						callFinalResp = 480;
					}

					MFree(callCache->free, sipCallHandle->localContact->host);
					sipCallHandle->localContact->host = 
						CStrdup(callCache, sipdomain);

					memcpy(callHandle->confID, &i, 4);
					memcpy(callHandle->confID+4, &origin, 4);
				}
				else
				{
					// Check type of reinvite
   					if(appMsgHandle->nlocalset)
					{ 
						if (appMsgHandle->localSet[0].rtpaddr == 0)
						{
       						hold = 1; 
						if (automode)
							fprintf(stdout, "Received hold reinvite for call %d\n", callHandle->callNo);
						}
		   				else if (appMsgHandle->localSet[0].direction == SendOnly)
						{
       						hold3264 = 1; 
						if (automode)
							fprintf(stdout, "Received RFC3264 hold reinvite for call %d\n", callHandle->callNo);
						}
						else
						{
						  	resumeInvite = 1;
						 if(automode)
							fprintf(stdout, "Received non-hold reinvite for call %d\n", callHandle->callNo);
						}
					}
					else
					{
						if (automode)
							fprintf(stdout, "Received reinvite without SDP for call %d\n", callHandle->callNo);
					}
					if (automode)
						fprintf(stdout, ">\n");
				}
			}
			else 
			{
				NETERROR(MSIP, ("%s Call handle is NULL\n", fn));
				callFinalResp = 480;
			}
			
			CacheReleaseLocks(callCache);
		}

		if(!callFinalResp)
		{
			if (!hold && !hold3264 && appMsgHandle->nlocalset)
			{
				Calls[callHandle->callNo].chOut[0].ip = appMsgHandle->localSet[0].rtpaddr;
				Calls[callHandle->callNo].chOut[0].port = appMsgHandle->localSet[0].rtpport;
				mgenInform(&Calls[callHandle->callNo], 1);
			}
			// If hold or resume type reinvites, do not send a 180
			if (!(hold || hold3264 || resumeInvite))
			{
				// If hold or resume type reinvites, do not send a 180
				evHandle = SipEventHandleDup(evb);
				evHandle->type = Sip_eBridgeEvent;
				evHandle->event = Sip_eBridge1xx;

				SipUAProcessEvent(evHandle);
			}
			if(hold || hold3264)
			{
				/* If hold is recived, stop sending media */
				mgenInform(&Calls[callHandle->callNo], 0);
			}

			if (!manualAccept)
			{
				evb->event = Sip_eBridge200;
				if (!hold && !hold3264 && appMsgHandle->nlocalset)
				{
					appMsgHandle->localSet[0].rtpaddr = chanLocalAddr;
					appMsgHandle->localSet[0].rtpport = Calls[callHandle->callNo].chIn[0].port;
				}

				if (hold3264)
				{
					if (appMsgHandle->attr == NULL)
					{
						appMsgHandle->attr = 
							(SDPAttr *) calloc(1, sizeof(SDPAttr));
						appMsgHandle->attr_count = 1;
					}
					SipCreateMediaAttrib(&appMsgHandle->attr[0], 0, "recvonly", NULL);
					appMsgHandle->localSet[0].direction == RecvOnly;	
				}
			}
		}
		else 
		{
			if (asrMode)
				finalRespCode = 0;
			evb->event = Sip_eBridgeFinalResponse;
			appMsgHandle->responseCode = callFinalResp;
			appMsgHandle->nlocalset=0; 		//do not send SDP with 3xx and above responses
		}

		break;
	case Sip_eNetwork1xx:
		forward = 0;
		break;
	case Sip_eNetwork200:
		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

		callHandle = CacheGet(callCache, callID);

		if (callHandle != NULL)
		{
			i = callHandle->callNo;

			CacheReleaseLocks(callCache);

			if (appMsgHandle->nlocalset) 
			{
				CallsOut[i].chOut[0].ip = appMsgHandle->localSet[0].rtpaddr;
				CallsOut[i].chOut[0].port = appMsgHandle->localSet[0].rtpport;
				mgenInform(&CallsOut[i], 1);
						

				appMsgHandle->maxForwards = sipmaxforwards;
				if ((inviteType & INVITE_NOSDP) | (reInvType == REINVITE_TYPE_NOSDP))
				{
					appMsgHandle->nlocalset = 2;
                			appMsgHandle->localSet = (RTPSet *) calloc(2, sizeof(RTPSet));

                			appMsgHandle->localSet[0].codecType = 0;
               				appMsgHandle->localSet[0].rtpaddr = chanLocalAddr;
                			appMsgHandle->localSet[0].rtpport = chanLocalPort;
                			appMsgHandle->localSet[0].mLineNo = 0;

                			appMsgHandle->localSet[1].codecType = 101;
                			appMsgHandle->localSet[1].rtpaddr = chanLocalAddr;
                			appMsgHandle->localSet[1].rtpport = chanLocalPort;
                			appMsgHandle->localSet[1].mLineNo = 0;
        			

 	       				appMsgHandle->attr_count = 2;
        				appMsgHandle->attr = (SDPAttr *) calloc(2, sizeof(SDPAttr));
        				appMsgHandle->attr[0].name = strdup("rtpmap");
        				appMsgHandle->attr[0].value = strdup("0 PCMU/8000");
        				appMsgHandle->attr[0].mLineNo = 0;
        				appMsgHandle->attr[1].name = strdup("rtpmap");
        				appMsgHandle->attr[1].value = strdup("101 telephone-event/8000");
        				appMsgHandle->attr[1].mLineNo = 0;


				}
				else
				{
					free(appMsgHandle->localSet);
					appMsgHandle->localSet = 0;
					appMsgHandle->nlocalset = 0;
				}
			}
#if 0
			evHandle = SipEventHandleDup(evb);
			evHandle->type = Sip_eBridgeEvent;
			evHandle->event = Sip_eBridgeAck;
			forward = 0;
#endif
			evb->type = Sip_eBridgeEvent;
			evb->event = Sip_eBridgeAck;
			if(SipUAProcessEvent(evb) == 0)
			{
				ua_eventprocessed = 1;
			}

			/* check if this is a 200OK to the transfer invite.
			 * if yes send a notify message to the transferer
			*/
			if(blindXferMode || attXferMode)
			{
				usleep(1000);
				SendNotifyMessage(appMsgHandle->responseCode);
			}
		}
		break;

	case Sip_eNetworkAck:

		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

		callHandle = CacheGet(callCache, callID);

		if (callHandle == NULL)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Call Handle already deleted?\n", fn));

			forward = 0;

			CacheReleaseLocks(callCache);

			break;
		}

		// Just received an ACK, see if we are doing fax
		if (fax && callHandle && !(callHandle->flags & FL_CALL_FAX))
		{
			sleep(2); 	//added for bug_7430 
			callHandle->flags |=  FL_CALL_FAX;

			evb->event = Sip_eBridgeInvite;
			evb->type = Sip_eBridgeEvent;

			if (appMsgHandle->nlocalset > 0)
			{
				free(appMsgHandle->localSet);
				appMsgHandle->localSet = 0;
				appMsgHandle->nlocalset = 0;
			}

			appMsgHandle->nlocalset = 1;
			appMsgHandle->localSet = (RTPSet *) calloc(1, sizeof(RTPSet));
			appMsgHandle->localSet[0].codecType = T38Fax;
			appMsgHandle->localSet[0].rtpaddr = chanLocalAddr;
			//chanLocalPort += 2;
			appMsgHandle->localSet[0].rtpport = chanLocalPort;

			if (appMsgHandle->attr_count > 0)
			{
				free(appMsgHandle->attr);
				appMsgHandle->attr = NULL;
				appMsgHandle->attr_count = 0;
			}

			attr = (SDPAttr *) calloc(9, sizeof(SDPAttr));
			appMsgHandle->attr_count = 9;
			appMsgHandle->attr = attr;

			SipCreateMediaAttrib(&attr[0], 0, "T38FaxVersion", "0");
			SipCreateMediaAttrib(&attr[1], 0, "T38MaxBitRate", "1400");
			SipCreateMediaAttrib(&attr[2], 0, "T38FaxFillBitRemoval", "0");
			SipCreateMediaAttrib(&attr[3], 0, "T38FaxTranscodingMMR", "0");
			SipCreateMediaAttrib(&attr[4], 0, "T38FaxTranscodingJBIG", "0");
			SipCreateMediaAttrib(&attr[5], 0, "T38FaxRateManagement", "transferredTCF");
			SipCreateMediaAttrib(&attr[6], 0, "T38FaxMaxBuffer", "200");
			SipCreateMediaAttrib(&attr[7], 0, "T38FaxMaxDatagram", "72");
			SipCreateMediaAttrib(&attr[8], 0, "T38FaxUdpEC", "t38UDPRedundancy");
		}
		else 
		{
			forward = 0;
		}
		
		CacheReleaseLocks(callCache);

		break;
	case Sip_eNetworkInfo:
        	if (appMsgHandle->ndtmf)
		{
			CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
			if ((callHandle = CacheGet(callCache, callID)) == NULL)
			{
				NETERROR(MSIP, ("%s Call handle is NULL\n", fn));
				CacheReleaseLocks(callCache);
				return -1;
			}
			fprintf(stdout, "Received signal DTMF for call %d : '%c', %dms\n",
					callHandle->callNo, appMsgHandle->dtmf->sig, 
					appMsgHandle->dtmf->duration);

			appMsgHandle->ndtmf = 0;
			free(appMsgHandle->dtmf);

			CacheReleaseLocks(callCache);
			fprintf(stdout, ">\n");
		}
		if ((appMsgHandle->isup_msg) || (appMsgHandle->qsig_msg))
                {

			CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
                        if ((callHandle = CacheGet(callCache, callID)) == NULL)
                        {
                                NETERROR(MSIP, ("%s Call handle is NULL\n", fn));
                                CacheReleaseLocks(callCache);
                                return -1;
                        }

			if (appMsgHandle->nlocalset > 0)
                        {
                                free(appMsgHandle->localSet);
                                appMsgHandle->localSet = 0;
                                appMsgHandle->nlocalset = 0;
                        }

			if (appMsgHandle->isup_msg) 
			{
                        	fprintf(stdout, "Received ISUP message for call %d.\n",
                                        callHandle->callNo);
				free(appMsgHandle->isup_msg);
				appMsgHandle->isup_msg = 0;
				appMsgHandle->isup_msg_len = 0;
			}
	
			if (appMsgHandle->qsig_msg)
			{
				fprintf(stdout, "Received QSIG message for call %d.\n",
                                        callHandle->callNo);
				free(appMsgHandle->qsig_msg);
				appMsgHandle->qsig_msg = 0;
				appMsgHandle->qsig_msg_len = 0;
			}

                        CacheReleaseLocks(callCache);
                        fprintf(stdout, ">\n");
		}


		// when an INFO message is received, send a 200 ok

		appMsgHandle->msgHandle->responseCode = 200;
		evb->event = Sip_eBridgeInfoFinalResponse;
		forward = 1;

		break;

	case Sip_eNetwork3xx:
		forward = 0;
		break;

	case Sip_eNetworkInfoFinalResponse:
		forward = 0;
		break;

	case Sip_eNetworkFinalResponse: 
		if (appMsgHandle->responseCode == 415)
			fprintf(stdout, "Received 415.\n");
		else if (blindXferMode || attXferMode)
		{
			SendNotifyMessage(appMsgHandle->responseCode);
		}
		fprintf(stdout, ">\n");
		appMsgHandle->responseCode = 0;
		break;

	case Sip_eNetworkRefer:
	case Sip_eBridgeRefer:

		// Refer message received, send a 202 back
		appMsgHandle->responseCode = 202;
		evHandle = SipEventHandleDup(evb);
		evHandle->type = Sip_eBridgeEvent;
		evHandle->event = Sip_eBridge202;
		forward = 0;
		SipUAProcessEvent(evHandle);

		// send an invite to transfer target
		if(appMsgHandle->msgHandle->referto->header)
		{
			attXferMode = 1;
			fprintf(stdout, "Received att call transfer.\n");
			fprintf(stdout, ">\n");
			strcpy(replaceStr, appMsgHandle->msgHandle->referto->header);
		}
		else
		{
			blindXferMode = 1;
			fprintf(stdout, "Received unatt call transfer.\n");
			fprintf(stdout, ">\n");
		}
		strcpy (xferTgtNum, appMsgHandle->msgHandle->referto->name);
		i = SgenGetFreeCallNo(1);
		SpawnOutgoingCall(i);
		break;

	case Sip_eNetwork202:
	case Sip_eBridge202:

		forward = 0;
		break;

	case Sip_eNetworkNotify:

		if (waitForNotify && (strcmp(appMsgHandle->sip_frag, "SIP/2.0 200 OK")))
		{
			SendReinvite(REINVITE_TYPE_RESUME);
		}
		evb->event = Sip_eBridgeNotifyResponse;
		forward = 1;
		break;

	case Sip_eBridgeNotify:
		fprintf(stdout, "Received a bridge notify message.\n");
		break;
	
	case Sip_eBridgeNotifyResponse:

		fprintf(stdout, "Received a notify response message.\n");
		break;

	case Sip_eNetworkBye:
	case Sip_eNetworkCancel:
	case Sip_eNetworkError:

		forward = 0;

		// We may need to re-initiate the call

		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	
		memcpy(&i, confID, 4);
		memcpy(&origin, confID+4, 4);

		if (i >= 0)
		{
			if (origin)
			{
				createNewCall = i;

				CallsOut[i].state = Sip_sIdle;
				mgenInform(&CallsOut[i], 0);
			}
			else
			{
				Calls[i].state = Sip_sIdle;
				mgenInform(&Calls[i], 0);
			}
		}
		else
		{
			NETERROR(MSIP, ("%s i=%d\n", fn, i));
		}

		CacheReleaseLocks(callCache);
		break;
	default:
		break;
	}
	/* Inform sgen about the message recieved from the network */
	sgenInform(evb);

	evb->type = Sip_eBridgeEvent;
	
	if(ua_eventprocessed == 1)
	{
		goto _return;
	}

	if (forward && (SipUAProcessEvent(evb) != 0))
	{
		NETERROR(MSIP, ("%s bridgeSipEventProcessor error\n", fn));

		// Free the call handle and return an error for now
		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

		callHandle = CacheGet(callCache, callID);

		if (callHandle == NULL)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Call Handle already deleted?\n", fn));
		
			goto _release_locks;
		}

		CacheDelete(callCache, callHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);

		/* Delete the call Handle */
		GisFreeCallHandle(callHandle);

	_release_locks:
		CacheReleaseLocks(callCache);

		return -1;
	}
	else if (forward == 0)
	{
		// free the event block
		SipFreeEventHandle(evb);

		// re-initiate the call if necessary
		if (tdIdle && createNewCall)
		{
			memset(&calltmr, 0, sizeof(struct itimerval));
			calltmr.it_value.tv_sec = tdIdle;
	
            timerAddToList(&localConfig.timerPrivate, &calltmr,
 				0, PSOS_TIMER_REL, "SgenTimer", 
			 	(TimerFn) SgenIdleTimerProcessor, (void *)createNewCall);
		}
		else if (tdConnect && createNewCall)
		{
			SpawnOutgoingCall(createNewCall);
		}
	}
			
_return:
	return 1;
}

int getPeerCallID(char * confID, char *callID,char *peerCallID)
{
}

int disconnectCallAtMaxCallDuration(tid timerid)
{
}
