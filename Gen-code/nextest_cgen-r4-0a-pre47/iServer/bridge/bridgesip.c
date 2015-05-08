#include "bridge.h"
#include "log.h"
#include "firewallcontrol.h"
#include "callutils.h"
#include "radclient.h"
#include "callid.h"
#include "resutils.h"
#include "licenseIf.h"
#include "iwfsmproto.h"

/**
 * This method needs to finish whatever is left to do from bridgeSipEventProcessorWorker()
 * after it called openFirewallOnSipEvents()
 *
 * This is called from bridgefw.c in two ways:
 *    a. when it determines no holes need to be opened, it calls this right from the thread
 *       that calls openFirewallOnSipEvents()
 *    b. when holes need to be opened, it calls this from the callback method called by MFCP
 *       (so it happens in a different thread)
 */
void bridgeFceSipCallback (SCC_EventBlock *evtPtr)
{

  // if any error has happened...
  if (evtPtr->callDetails.fceStatus.rval == FCE_SUCCESS)
  {
    // hole opened successfully
	// resume leg2 processing in the worker
	bridgeSipEventProcessor(evtPtr);
  }
  else
  {
    // error processing
	// resume leg2 processing in the worker
	bridgeSipEventProcessor(evtPtr);
  }

}


/*  Handle Incoming Sip Events and determine other leg to pass it on to 
	Find callHandle
	Create and initialize if not there
	Change state 
	Execute the action routine
*/
int bridgeSipEventProcessorWorker(SCC_EventBlock *evtPtr)
{
	static char fn[] = "bridgeSipEventProcessorWorker";
	CallHandle 	*callHandle1 = NULL, *callHandle2 = NULL;
	CallHandle 	*callHandleRelated = NULL, *ch = NULL;
	SipCallHandle 	*sipCallHandle1 = NULL;
	ConfHandle	*confHandle = NULL;
	char 		str[EVENT_STRLEN], *evName;
	SipAppCallHandle	*pSipData;
	SCC_CallHandleType 	call2Type;
	SCC_EventBlock	*pAckSipEvt=NULL;
	SipAppCallHandle	*pAckSipData=NULL;
	char		srcCallID[CALL_ID_LEN] = {0};
	char		callIDStr1[CALL_ID_LEN],confIDStr[CONF_ID_LEN],
				callIDStr2[CALL_ID_LEN];
	header_url	*pRequri;
	int 		callError = SCC_errorNone;
	int			portAllocated = 0, handleAllocated = 0;
	InfoEntry	dinfo;
	FCEStatusStruct		status = { 0 };
	int 		herror, n, i, j, addcodec, serror;
	int			doLeg2 = 1;
	int			drealmId = -1, mydomain = 0;
	CodecType	codec2833[] = { CodecRFC2833 };
	char		*attrib2833[][2] = { { "rtpmap", "101 telephone-event/8000" }, { "fmtp",	"101 0-15"} };
	char		*attrib2833del[][2] = { { "rtpmap", "101 " }, { "fmtp",	"101 "} };
	int			numcodec2833 = 1, numattrib2833 = 2, numattrib2833del = 2;

	char rsadomain[24];
        
	CacheRealmEntry* realmEntryPtr = NULL; 
	unsigned long    mirrorproxy   = 0;
        CacheTableInfo*  info          = NULL;

	if(!evtPtr)
	{
		NETERROR(MBRIDGE,("%s Null EventPtr \n",fn));
		return -1;
	}

	evtPtr->callDetails.callType = CAP_SIP;
	pSipData = (SipAppCallHandle *) evtPtr->data;
	memcpy(srcCallID,evtPtr->callID,CALL_ID_LEN);

	evName = GetSipBridgeEvent(evtPtr->event);

	NETDEBUG(MBRIDGE, NETLOG_DEBUG4,("%s Event(%s)\n",
		fn, (char*) evName));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	bridgeFindCallLegsInfoForEvent(evtPtr, &callHandle1, &callHandle2, &confHandle);

	if (!callHandle1)
	{
		if (evtPtr->event == Sip_eBridgeInvite)
		{
			NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
				("%s callHandle1 already deleted for INVITE \n", fn));

			callError = SCC_errorNoCallHandle;
			goto _error;
		}
		else
		{
			NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
				("%s callHandle1 CallId %s does not exist for %s\n",
				fn, (char*) CallID2String(evtPtr->callID,callIDStr1), evName));
		}
	}
	else
	{
		sipCallHandle1 = SipCallHandle(callHandle1);
	}

	if (!confHandle)
	{
		NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
			("%s Failed to find ConfId %s Event %s callHandle 1 %p callHandle 2 %p\n",
			fn, (char*) CallID2String(evtPtr->confID,confIDStr), evName,
			callHandle1, callHandle2));

		callError = SCC_errorSipInternalError;
		goto _error;
	}

	
	if (pSipData->localSet && callHandle1 && IsFCEEnabled() == TRUE &&
  		FCE_STATUS_IS_NULL(evtPtr->callDetails.fceStatus) && 
		SipEventNeedsFCE(evtPtr->event))
	{

		// Check to see if any other fc events are still pending on this call
		if (callHandle1->flags & FL_CALL_FCPEND)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s FCE event added to list for event %s\n", fn, evName));

			evtPtr->evtProcessor = (int (*)(void))bridgeSipEventProcessorWorker;

			// Add this event into list and quit
			if (listAddItem(callHandle1->fcevtList, evtPtr) != 0)
			{
				NETERROR(MBRIDGE, ("%s Failed to queue Event\n", fn));

				CacheReleaseLocks(confCache);
				CacheReleaseLocks(callCache);

				return 0;
			}
			CacheReleaseLocks(confCache);
			CacheReleaseLocks(callCache);
			return -1;
		}

		// Check to see if any other fc events are still pending on the other leg
		if (callHandle2 && (callHandle2->flags & FL_CALL_FCPEND))
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s FCE event added to peer leg list for event %s\n", fn, evName));

			evtPtr->evtProcessor = (int (*)(void))bridgeSipEventProcessorWorker;

			// Add this event into list and quit
			if (listAddItem(callHandle2->fcevtList, evtPtr) != 0)
			{
				NETERROR(MBRIDGE, ("%s Failed to queue Event\n", fn));

				CacheReleaseLocks(confCache);
				CacheReleaseLocks(callCache);

				return 0;
			}
			CacheReleaseLocks(confCache);
			CacheReleaseLocks(callCache);
			return -1;
		}
	}

  	if (!(FCE_STATUS_IS_NULL(evtPtr->callDetails.fceStatus)))
	{
	  	status = evtPtr->callDetails.fceStatus;
		// Set up some old variables
		if(evtPtr->event == Sip_eBridgeInvite)
		{
			if (callHandle1 && (evtPtr->flags & SCCF_BRIDGE_PORTALLOCATED))
			{
				portAllocated = 1;
	
				// Do not pass this outside this module
				evtPtr->flags &= ~SCCF_BRIDGE_PORTALLOCATED;
			}

			if (callHandle2 && (evtPtr->flags & SCCF_BRIDGE_HANDLEALLOCATED))
			{
				handleAllocated = 1;
	
				// Do not pass this outside this module
				evtPtr->flags &= ~SCCF_BRIDGE_HANDLEALLOCATED;
			}
		}

  		if (!callHandle1 || !callHandle2 || !confHandle)
		{
			NETERROR(MBRIDGE, 
				("%s resuming leg2: openFirewallOnSipEvents invalid call state %p/%p/%p event = %d\n", 
				fn, callHandle1, confHandle, callHandle2, evtPtr->event));

			callError = SCC_errorFCECallSetup;
			goto _error;
		}

  		if ( (evtPtr->callDetails.fceStatus.rval == FCE_SUCCESS))
		{
			if (UpdateFCEInfo(evtPtr, callHandle1, callHandle2, confHandle) < 0)
				goto _error;

			if (callHandle1->mediaTurning && (evtPtr->event == Sip_eBridgeAck)) {
				// In case of media turning. Don't pass the sdp in ack we received on leg1 to leg2
				pSipData->nlocalset = 0;
				SipCheckFree(pSipData->localSet);
				pSipData->localSet = NULL;
				// Disable MediaTurning for future events - e.g. holding
				callHandle1->mediaTurning = 0;
			}

			goto _continue_leg2;
		}
		else
		{
			NETERROR(MBRIDGE, 
				("%s resuming leg2: openFirewallOnSipEvents failed! event = %d\n", fn,evtPtr->event));

			callError = SCC_errorFCECallSetup;
			goto _error;
		}
	}

	// First Invite on a call
	if ((evtPtr->event == Sip_eBridgeInvite) && sipCallHandle1 &&
		(sipCallHandle1->successfulInvites == 0))
	{
		
                FormatIpAddress(pSipData->realmInfo->rsa, rsadomain);

		
		CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);
		CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
		realmEntryPtr = CacheGet(realmCache, &(pSipData->realmInfo->realmId));
		if (realmEntryPtr)
		{
			// Look up the entry in the cache
			info = CacheGet(regCache, &(realmEntryPtr->realm.mp));
			if(info)
			{
				mirrorproxy = info->data.ipaddress.l;
			}                        
		}
		CacheReleaseLocks(realmCache);
		CacheReleaseLocks(regCache);					

		if( mirrorproxy != 0 && !SipMatchDomains(rsadomain,pSipData->requri->host))
                {
                        FormatIpAddress(mirrorproxy,rsadomain);                        
                        SipCheckFree(pSipData->requri->host);
                        pSipData->requri->host = strdup(rsadomain);                        

                        SipCheckFree(pSipData->calledpn->host);
                        pSipData->calledpn->host = strdup(rsadomain);                        

                        SipCheckFree(pSipData->callingpn->host);
                        pSipData->callingpn->host = strdup(rsadomain);                        

                }

        
                drealmId = RealmLocate(pSipData->requri->realmId>0?pSipData->requri->realmId:callHandle1->realmInfo->realmId, 
						pSipData->requri->host, 0, &mydomain);

		if (mydomain)
		{
			NETDEBUG(MBRIDGE,NETLOG_DEBUG2, ("%s mydomain TRUE\n", fn));
			// if we are not going to reuse the port, we should deallocate it
			if(callHandle1->flags & FL_FREE_DESTPORT)
			{
				GwPortFreeCall(&callHandle1->rfphonode, 1, 1);
				callHandle1->flags &= ~FL_FREE_DESTPORT;
			}

			// Free egress token node ID from conf handle
			GisFreeEgressTokenNodeId(confHandle);

			// Reset egress H323-ID in conf handle
			confHandle->egressH323Id[0] = '\0';

			if(bridgeSipResolveCache(callHandle1, pSipData,
				&callError, NULL, &dinfo)<=0)
			{
				NETDEBUG(MSCC,NETLOG_DEBUG2,
					("%s bridgeSipResolveCache Failed for = %s\n",
					fn, (char*) CallID2String(evtPtr->callID,
					callIDStr1)));
				goto _error;
			}

			portAllocated = 1;

			// prefer Sip over H323 
 
			if(BIT_TEST(callHandle1->rfphonode.cap, CAP_SIP))
			{
				call2Type = SCC_eSipCallHandle;
				// Do not fill in the dipaddr/port for sip case
			} 
			else if(BIT_TEST(callHandle1->rfphonode.cap, CAP_H323))
			{
				call2Type = SCC_eH323CallHandle;
				pSipData->dipaddr = callHandle1->rfphonode.ipaddress.l;
				pSipData->dipport= callHandle1->rfcallsigport;
			}
			else 
			{
				NETERROR(MBRIDGE,("%s UnSupported Remote Endpoint\n",fn));
				callError = SCC_errorDestinationUnreachable;
				goto _error;
			}
		}
		else
		{
			NETDEBUG(MBRIDGE,NETLOG_DEBUG2, ("%s mydomain FALSE\n", fn));
			// if we are not going to reuse the port, we should deallocate it
			if(callHandle1->flags & FL_FREE_DESTPORT)
			{
				GwPortFreeCall(&callHandle1->rfphonode, 1, 1);
				callHandle1->flags &= ~FL_FREE_DESTPORT;
			}

			GisFreeEgressTokenNodeId(confHandle);

			// Reset egress H323-ID in conf handle
			confHandle->egressH323Id[0] = '\0';

			if (!allowInternalCalling ||
					(bridgeSipResolveCache(callHandle1,pSipData,
						&callError, NULL,&dinfo) < 0))
			{
				// Look up the proxy entry in the database
				// This is the case for OBP, where we don't have the destination
				// information. We will have to find out the realm for the 
				// destination here. The proxy will be looked up by the URI we have


				if (drealmId < 0)
				{
					// In this case we will attempt to set the realm to that which is in the URI
					// If the URI was extracted from a contact, the TSM sets up the RealmId of
					// the realm on which the contact was received
					// If this realmId is 0, this may be because it has not been uninitialized
					// In any case, we will attempt to send the message out on the default realm
					
					drealmId = pSipData->requri->realmId;
				}

				if (drealmId < 0)
				{
					NETERROR(MSIP, ("%s Destination proxy realm unknown for %s\n",
						fn, pSipData->requri->host));

					callError = SCC_errorNoRoute;
					goto _error;
				}

				BillCallPrevCdr(callHandle1);

				memset(&dinfo, 0, sizeof(InfoEntry));
				pSipData->dipaddr = callHandle1->rfphonode.ipaddress.l = 
						dinfo.ipaddress.l = ResolveDNS(pSipData->requri->host, &herror);
				callHandle1->rfcallsigport = pSipData->requri->port;
				if (callHandle1->rfcallsigport == 0)
				{
					if ( pSipData->requri->type == HEADER_ADDRESS_H323 )
					{
						callHandle1->rfcallsigport = 1720;
					}
					else
					{
						callHandle1->rfcallsigport = 5060;
					}
				}

				pSipData->dipport = callHandle1->rfcallsigport;

				BIT_SET(callHandle1->rfphonode.sflags, ISSET_IPADDRESS);
				dinfo.realmId = drealmId;
			}

			portAllocated = 1;
			if ( pSipData->requri->type == HEADER_ADDRESS_H323 )
			{
				call2Type = SCC_eH323CallHandle;
				BIT_SET(callHandle1->rfphonode.cap, CAP_H323);
			}
			else
			{
				call2Type = SCC_eSipCallHandle;
				BIT_SET(callHandle1->rfphonode.cap, CAP_SIP);
			}
		}

		// Set Sm type in the  confhandle
		confHandle->confType = DetermineConfType(SCC_eSipCallHandle,call2Type);
		confHandle->state = IWF_sIdle;
		
		// This is to handle an ipphone was max calls set to -1
		if(GwPortAvailCall(&callHandle1->rfphonode,0,1) < 0 )
		{
			NETERROR(MBRIDGE,("%s PortAvailCall for destination failed ",fn));
			callError = SCC_errorGatewayResourceUnavailable;
			goto _error;
		}

		if (callHandle2 == NULL)
		{
			/* Now we need to create callhandle for leg2 */
			// Add Call Handle To Conf 
			if(!(callHandle2 = GisAllocCallHandle()))
			{
				NETERROR(MBRIDGE,("%s GisAllocCallHandle failed.\n",fn));
				callError = SCC_errorSipInternalError;
				goto _error;
			}
		}
		else
		{
			/* successinvites == 0 but callHandle2 exisits in 401/407 */
			goto continue_401_407_reInvite_processing;
		}

		handleAllocated = 1;

		if(h323Enabled())
		{
			// We are not sure this will be an iwf call or not
			generateOutgoingH323CallId(callHandle2->callID, 
				BIT_TEST(callHandle1->rfphonode.cap, CAP_ARQ));
		}
		else
		{
			generateCallId(callHandle2->callID);
		}

		memcpy(callHandle2->confID, confHandle->confID, CONF_ID_LEN);

		callHandle2->handleType = call2Type;
		callHandle2->vendor = dinfo.vendor;
		callHandle2->peerIp = dinfo.ipaddress.l;
		callHandle2->cap = dinfo.cap;
		callHandle2->ecaps1 = dinfo.ecaps1;

	 	// if  db entry has nat-ip and nat-port set fill them into the sip data
		if ( BIT_TEST(dinfo.sflags, ISSET_NATIP) )
		{
			callHandle2->natip = dinfo.natIp;
			if( BIT_TEST(dinfo.sflags, ISSET_NATPORT) )
			{
				callHandle2->natport = dinfo.natPort;
			}
		}

		// Get Realm Info for this dinfo.
		callHandle2->realmInfo = getRealmInfo (dinfo.realmId, callCache->malloc);

		if (callHandle2->realmInfo == NULL)
		{
			NETERROR (MBRIDGE, ("%s: Failed to get realm info for id %d.\n", fn, dinfo.realmId));
			callError = SCC_errorSipInternalError;
			goto _error;
		}
		
		if (mydomain || (mirrorproxy == 0))
		{
			if (pSipData->callingpn && pSipData->callingpn->host && !forwardSrcAddr)
			{
				free(pSipData->callingpn->host);
                FormatIpAddress(callHandle2->realmInfo->rsa, rsadomain);
				pSipData->callingpn->host = strdup(rsadomain);

				NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s: from changed to %s", fn, rsadomain));
			}
		}

		NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s: Retrieved realm info for realm %d", fn, dinfo.realmId));

		if(call2Type == SCC_eSipCallHandle)
		{
			// Don't need IWF
			callHandle2->bridgeEventProcessor = sipBridgeEventProcessor;
			callHandle2->state = Sip_sIdle;

			// to add the prefix to To:
			if(dinfo.ogprefix[0] != '\0')
			{
				callHandle2->ogprefix = CStrdup(callCache, dinfo.ogprefix);
			}
		} 
		else if(call2Type == SCC_eH323CallHandle) 
		{
			callHandle2->bridgeEventProcessor = SCC_H323Leg2BridgeEventProcessor;
			H323controlState(callHandle2) = UH323_sControlIdle;
			callHandle2->state = SCC_sIdle;
			memcpy(H323Callq931IE(callHandle2),dinfo.q931IE,Q931IE_LEN);
			memcpy(H323Callbcap(callHandle2),dinfo.bcap,BCAP_LEN);
			H323infotranscap(callHandle2) = dinfo.infoTransCap;
			NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s InforTransCap = %d\n",fn, dinfo.infoTransCap)); // for testing

			// We don't know SIP side's called partynumber type.
			// So we will use called party number type public unknown
			H323OrigCalledPartyNumType(callHandle2) = 
				getDestCalledPartyNumType(Q931CDPN_Unknown);

			if ( H323Callq931IE(callHandle2)[Q931IE_CDPN] == Q931CDPN_Pass )
			{
				/* if it is pass-through */

				H323DestCalledPartyNumType(callHandle1) = 
					H323DestCalledPartyNumType(callHandle2) = 
					H323OrigCalledPartyNumType(callHandle2);
			}
			else
			{
				H323DestCalledPartyNumType(callHandle1) = 
					H323DestCalledPartyNumType(callHandle2) =
					getDestCalledPartyNumType(H323Callq931IE(callHandle2)[Q931IE_CDPN]);
			}

			// We don't know SIP side's calling partynumber type.
			// So we will use calling party number type public unknown
			H323OrigCallingPartyNumType(callHandle2) = 
				getDestCallingPartyNumType(Q931CGPN_Unknown);

			if (H323Callq931IE(callHandle2)[Q931IE_CGPN] == Q931CGPN_Pass)
			{
				// Pass calling party number type from leg1 to leg2

				H323DestCallingPartyNumType(callHandle1) = 
					H323DestCallingPartyNumType(callHandle2) = 
					H323OrigCallingPartyNumType(callHandle2);
			}
			else
			{
				// Set the calling party number type on leg2 based on 
				// what is configured on the destination endpoint

				H323DestCallingPartyNumType(callHandle1) = 
					H323DestCallingPartyNumType(callHandle2) = 
					getDestCallingPartyNumType(H323Callq931IE(callHandle2)[Q931IE_CGPN]);
			}
		}

		// set the TAP flag for the dest call handle.
		// one of these calls should be fine, as we have the cache
		// access here
		if (strlen(tapcall))
		{
			callHandle2->flags |= (pSipData->flags & FL_CALL_TAP);
		}

		// Populate CallHandle2 structures
		memcpy(&callHandle2->phonode,&callHandle1->rfphonode,sizeof(PhoNode));
		memcpy(&callHandle2->rfphonode,&callHandle1->phonode,sizeof(PhoNode));

		memcpy(callHandle2->acct_session_id, callHandle1->acct_session_id, sizeof(callHandle1->acct_session_id));

		memcpy(callHandle2->inputNumber, callHandle1->inputNumber, PHONE_NUM_LEN);

		// Relay leg 1 id to leg 2. This will be the CID for H.323 calls and will
		// be used for the protocol CID also.
		callHandle2->incoming_conf_id = CStrdup(callCache, callHandle1->incoming_conf_id);
		if (dinfo.custID[0] != '\0')
		{
			callHandle2->custID = 
				CStrdup(callCache, dinfo.custID);
		}

		if (callHandle1->tg && (!(dinfo.ecaps1 & ECAPS1_NOTG)) )
		{
			callHandle2->tg = 
				CStrdup(callCache, callHandle1->tg);
		}

		if (callHandle1->destTg)
		{
			callHandle2->destTg = CStrdup(callCache, callHandle1->destTg);
		}
		else if (dinfo.dtg[0] != '\0')
		{
			callHandle2->destTg = CStrdup(callCache, dinfo.dtg);
			// to be used for CDR field 46 dest-trunk-group
			callHandle1->dtgInfo = CStrdup(callCache, dinfo.dtg);
		}

		if(CacheInsert(callCache, callHandle2) < 0)
		{
			NETERROR (MBRIDGE,("%s CacheInsert for callHandle2 failed.\n",fn));
			callError = SCC_errorSipInternalError;
			goto _error;
		}

		callHandle2->callSource=1;
		timedef_cur(&(callHandle2->callStartTime));
		// Privacy Related Changes
      
		// Ensure that this is the first invite on the call
		if ((evtPtr->event == Sip_eBridgeInvite) && sipCallHandle1 && 
		    (sipCallHandle1->successfulInvites == 0)) {
		  
		  //Determine the destination's privacy capability
		  pSipData->privTranslate = privTranslateNone;
		  if( (callHandle2->ecaps1 & ECAPS1_SIP_PRIVACY_RFC3325) == ECAPS1_SIP_PRIVACY_RFC3325 ) {    
                          pSipData->dest_priv_type = privacyTypeRFC3325;                          
                  }
                  else if( (callHandle2->ecaps1 & ECAPS1_SIP_PRIVACY_DRAFT01) == ECAPS1_SIP_PRIVACY_DRAFT01 ) {
                          pSipData->dest_priv_type = privacyTypeDraft01;                          
                  }
                  
		  switch(pSipData->incomingPrivType) {
		    
		  case privacyTypeNone:

		      NETDEBUG(MSIP,NETLOG_DEBUG4,("%s: Incoming privacy type None",fn));
		    // No Translation required Let it go through straight
		    break;
		    
		  case  privacyTypeRFC3325:
		      
		      NETDEBUG(MSIP,NETLOG_DEBUG4,("%s: Incoming privacy type RFC3325",fn));
		    // Determine if destination supports RFC 3325
		    if( (callHandle2->ecaps1 & ECAPS1_SIP_PRIVACY_RFC3325) == ECAPS1_SIP_PRIVACY_RFC3325 ) {
		      pSipData->privTranslate = privTranslateNone;
		    }
		    else if( (callHandle2->ecaps1 & ECAPS1_SIP_PRIVACY_DRAFT01) == ECAPS1_SIP_PRIVACY_DRAFT01 ) {
			    NETDEBUG(MSIP,NETLOG_DEBUG4,("%s: Translation from RFC3325 to Draft requested - ecaps1 flag 0x%x",fn,callHandle2->ecaps1));
			    pSipData->privTranslate = privTranslateRFC3325ToDraft01;
		    }
		    break;
		    
		  case  privacyTypeDraft01:
		    
		      NETDEBUG(MSIP,NETLOG_DEBUG4,("%s: Incoming privacy type Draft01",fn));

		    if( (callHandle2->ecaps1 & ECAPS1_SIP_PRIVACY_DRAFT01) == ECAPS1_SIP_PRIVACY_DRAFT01 ) {
		      pSipData->privTranslate = privTranslateNone;
		    }
		    else if( (callHandle2->ecaps1 & ECAPS1_SIP_PRIVACY_RFC3325) == ECAPS1_SIP_PRIVACY_RFC3325 ) {
			    // Since Draft to RFC3325 is not supported currently
			    // We will draft to rfc3325 case as if it is draft to draft/both case
			    NETDEBUG(MSIP,NETLOG_DEBUG4,("%s: Translation from  Draft to RFC3325 requested - ecap1 flag 0x%x",fn,callHandle2->ecaps1));
			    pSipData->privTranslate = privTranslateNone;
		    }
		    
		    break;
	  
		  default:
		    break;
		  }
		}
		// End of Privay Changes

		if( GisAddCallToConf(callHandle2) != 0 )
		{
			/* This should NEVER fail */
			NETERROR(MBRIDGE,("%s GisAddCallToConf 2 failed.\n",fn));
			callError = SCC_errorSipInternalError;
			goto _error;
		}

		if(billingType == BILLING_CISCOPREPAID)
		{
			/* Authourize user with Radius Server */
			SipCiscoRadiusAuthourize(evtPtr);

			CacheReleaseLocks(confCache);
			CacheReleaseLocks(callCache);
			return 0;
		}
	} 

	// At this point callHandle1/2, src/destCallID should have defined
	// values

continue_401_407_reInvite_processing:
	if (evtPtr->event == Sip_eBridgeInvite ||
		evtPtr->event == Sip_eBridge200 ||
		evtPtr->event == Sip_eBridgeAck)
	{
		if (callHandle1 == NULL)
		{
			NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
				("%s CallId %s not present for %d\n",
				fn, (char*) CallID2String(srcCallID, callIDStr1), evtPtr->event));
			callError = SCC_errorNoCallHandle;
			goto _error;
		}	

		if (evtPtr->event == Sip_eBridge200)
		{
			if (strlen(tapcall) && (callHandle1->flags & FL_CALL_TAP))
			{
				// Spawn off a tapcall
				CallTap(callHandle1, 1);
			}
		}
	}

	if (callHandle2 == NULL)
	{
		NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
			("%s Failed to find CallId %s\n",
		  	fn, (char*) CallID2String(evtPtr->callID, callIDStr2)));

		if ((evtPtr->event == Sip_eBridge200) &&
			(strcmp(SipMsgCallCseqMethod(pSipData->msgHandle),"INVITE") == 0)) 
		{

			NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
				("%s Generating Ack for INVITE transc %s\n",
				fn, (char*) CallID2String(evtPtr->callID, callIDStr2)));

			pAckSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
			memset(pAckSipEvt,0,sizeof(SCC_EventBlock));

			pAckSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
			memset(pAckSipData,0,sizeof(SipAppCallHandle));

			pAckSipData->maxForwards = sipmaxforwards;

			pAckSipEvt->event = Sip_eBridgeAck;

			memcpy(pAckSipEvt->confID,confHandle->confID,CONF_ID_LEN);
			memcpy(pAckSipEvt->callID,callHandle1->callID,CALL_ID_LEN);
			memcpy(pAckSipData->callID,callHandle1->callID,CALL_ID_LEN);
			memcpy(pAckSipData->confID,confHandle->confID,CONF_ID_LEN);
			pAckSipEvt->data = (void *) pAckSipData;

			if(sipBridgeEventProcessor(pAckSipEvt) !=0)
			{
				NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
			}
		
		}

		NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
			("%s Generating Final Response %s\n",
			fn, (char*) CallID2String(evtPtr->callID, callIDStr2)));

	   	callError = SCC_errorNoCallHandle;
	   	goto _error;
	}

	// Check if mediaTurning should activate
	if (pSipData->localSet == NULL) {
	    NETDEBUG(MFCE, NETLOG_DEBUG2, ("%s: pSipData->localSet is NULL for eventType %d\n", fn, evtPtr->event));        
	
		if ((evtPtr->event == Sip_eBridgeInvite) && 
			(callHandle2) && (callHandle2->handleType == SCC_eSipCallHandle)) { 
				// Create a fake sdp before opening holes 
				pSipData->localSet = (RTPSet *) calloc(1, sizeof(RTPSet));
				pSipData->nlocalset = 1;                                 
				pSipData->localSet[0].codecType = defaultCodec;
				// The following line sets the default param for the above codec
				setDefaultRtpParams(pSipData->localSet, pSipData->nlocalset);
				pSipData->localSet[0].rtpaddr = callHandle1->peerIp;
				// The first pair 14,15 unassigned ports according to IANA
				pSipData->localSet[0].rtpport = 14; 
				pSipData->localSet[0].mLineNo = 0;
				pSipData->localSet[0].direction = SendRecv;
				NETDEBUG(MFCE, NETLOG_DEBUG2, ("%s: setting pSipData->localSet as media turning enabled \n", fn));
				callHandle1->mediaTurning = 1;
		}
	}

	// Check here and make sure that the FCE is called
	// with a valid callHandle1. 
	if (pSipData->localSet && callHandle1 && IsFCEEnabled() == TRUE && SipEventNeedsFCE(evtPtr->event))
	{
		// Check if we need to turn transcoding on
  		if ( Is2833Supported(callHandle1->ecaps1) && Is2833NotSupported(callHandle2->ecaps1) &&
		  	 ((evtPtr->event == Sip_eBridgeInvite) || (evtPtr->event == Sip_eBridge200)) )
		{
			callHandle1->dtmf_detect = 1;
			// remember to remove it in the FCE callback

			// If SDP has 2833 then remove it
			RemoveCodecsAttribs(pSipData, codec2833, numcodec2833, attrib2833del, numattrib2833del, callHandle2->vendor);
		}

		// Check if we need to fill the SDP
  		if ( Is2833NotSupported(callHandle1->ecaps1) && Is2833Supported(callHandle2->ecaps1) &&
		  	 ((evtPtr->event == Sip_eBridgeInvite) || (evtPtr->event == Sip_eBridge200)) )
		{
	
			int addcodec = 1;	

			for (i = 0; i < pSipData->nlocalset; i++) {
				if (pSipData->localSet[i].codecType == CodecRFC2833) {
					NETDEBUG(MBRIDGE, NETLOG_DEBUG1, ("CallID %s has 2833 in CodecList\n",	
						(char*) CallID2String(evtPtr->callID, callIDStr1)));
					addcodec = 0;
				} 
			}
			
			if (addcodec) {
			  	AddCodecsAttribs(pSipData, codec2833, numcodec2833, attrib2833, numattrib2833, callHandle2->vendor);
			}
		}

		evtPtr->evtProcessor = (int (*)(void))bridgeFceSipCallback;

		if (portAllocated)
		{
			evtPtr->flags |= SCCF_BRIDGE_PORTALLOCATED;
		}

		if (handleAllocated)
		{
			evtPtr->flags |= SCCF_BRIDGE_HANDLEALLOCATED;
		}

		// Nothing is Pending in the fc event queue
		// We are going to queue up an event
		callHandle1->flags |= FL_CALL_FCPEND;

                // *************************************************************
                // Make sure you have a lock before calling reopenFirewall() or
                // openFirewall(). If multiple threads are calling these without
                // lock protection, there is a race condition underneath
                // *************************************************************

		status = openPinholes(evtPtr, callHandle1, callHandle2, confHandle);

		if (status.rval == FCE_FAILURE)
		{
			if (status.err == FCE_LICENSE_FAILURE)
			{
				callError = SCC_errorNoFCEVports;
			}
			else if(status.err == FCE_NATT_LICENSE_FAILURE)
			{
				callError = SCC_errorNoNATTLicense;
			}
			else
			{
				NETERROR(MBRIDGE, 
					("%s: openFirewallOnSipEvents failed! event = %d\n", fn,evtPtr->event));
				callError = SCC_errorFCECallSetup;
			}

			goto _error;
		}
		else
		{
  			if (status.rval == FCE_SUCCESS)
  			{
				// This thread can use evtPtr
				evtPtr->callDetails.fceStatus = status;
				UpdateFCEInfo(evtPtr, callHandle1, callHandle2, confHandle);
  			}

        	// FCE_SUCESS or FCE_INPROGRESS means things are done
			NETDEBUG(MBRIDGE,NETLOG_DEBUG2,
				("%s FCE call returned OK status = %d\n", fn, status.rval));
			
			if (status.rval == FCE_INPROGRESS) {
				// skip the rest of the code now
				CacheReleaseLocks(confCache);
				CacheReleaseLocks(callCache);

				return 0;
			}
		}
	}

_continue_leg2:

	if (sipCallHandle1 && (sipCallHandle1->uaflags & UAF_NETIDINVITE)) {
		// We are not supposed to forward this event to leg2
		NETDEBUG(MBRIDGE, NETLOG_DEBUG2, ("Skipping Leg2 processing of callid %s for event %s\n",\
			   	CallID2String(evtPtr->callID,callIDStr1), evName));
		sipCallHandle1->uaflags &= ~UAF_NETIDINVITE;
		doLeg2 = 0;
	}

	memcpy(pSipData->callID,callHandle2->callID,CALL_ID_LEN);
	memcpy(evtPtr->confID,confHandle->confID,CONF_ID_LEN);
	memcpy(evtPtr->callID,callHandle2->callID,CALL_ID_LEN);

	/* replace SipAppCallHandle->realmInfo with correct outgoing realm info */

	RealmInfoFree(pSipData->realmInfo, MEM_LOCAL);

	pSipData->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle2->realmInfo, MEM_LOCAL);

	if(confHandle->mediaRouted)
	{
		if(pSipData->pOriginIpAddress)
		{
			free(pSipData->pOriginIpAddress);
			pSipData->pOriginIpAddress = NULL;
		}
	}

	// Need to Fix this - Eliminate returning  ERROR
	switch(confHandle->confType)
	{
	case Conf_eSip2H323P2P :
		if(doLeg2 && (iwfSipEventProcessor(evtPtr) < 0) )
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG2,
				("%s iwfSipEventProcessor returned error\n",fn));
			callError = SCC_errorSipInternalError;
			goto _error;
		}

		break;

	case Conf_eSipP2P :
		// Delete callHandle from Conference if  dest is Sip
		// This gets rid of ConfHandle also 

		if(doLeg2 && (sipBridgeEventProcessor(evtPtr) < 0) )
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG2,
				("%s bridgeEventProcessor returned error\n",fn));
			callError = SCC_errorSipInternalError;
			goto _error;
		}

		break;

	default :
		NETERROR(MBRIDGE,
			("%s received invalid conf type (%d)\n",fn,confHandle->confType));

		SipFreeAppCallHandle(evtPtr->data);
		free(evtPtr);

		break;
	}

  	if (callHandle1 && (status.rval == FCE_SUCCESS))
	{
		// Do any remaining events queued up in the end
		BridgeCheckPendingFCEvents(callHandle1);
	}

	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);

	return 0;

_error:
	// Set the SIP response code based on the call error, if any
	if ((callError != SCC_errorNone) && (pSipData != NULL))
	{
		pSipData->responseCode = getSipCode(callError);
	}

   	if (evtPtr->callDetails.fceResource)
	{
		FCECloseResource(evtPtr->callDetails.fceSession, evtPtr->callDetails.fceResource);
	}

	// Send error back to both legs
	if(callHandle2)
	{
		if(handleAllocated)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG2,("%s Error !! Deleting callHandle2\n", fn));

			GisDeleteCallFromConf(callHandle2->callID,
				callHandle2->confID);

			CallDelete(callCache, callHandle2->callID);
			GisFreeCallHandle(callHandle2);
		}
		else
		{
			SCC_EventBlock		*pSipEvt;

			pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
			memset(pSipEvt,0,sizeof(SCC_EventBlock));

			// Send an error back on this leg
			memcpy(pSipEvt->callID, callHandle2->callID, CALL_ID_LEN);
			memcpy(pSipEvt->confID, callHandle2->confID, CONF_ID_LEN);

			// Set this so that it points to bridge
			pSipEvt->callDetails.callError = callError;
			pSipEvt->callDetails.lastEvent = SCC_evtResolve;

			pSipEvt->event = Sip_eBridgeError;
			pSipEvt->evtProcessor = NULL;

			switch(confHandle->confType)
			{
			case Conf_eSip2H323P2P :
				if(iwfSipEventProcessor(pSipEvt) < 0)
				{
					NETERROR(MBRIDGE, 
						("%s iwfSipEventProcessor returned error\n",fn));
				}

				break;

			case Conf_eSipP2P :
				if (sipBridgeEventProcessor(pSipEvt) < 0)
				{
					NETERROR(MBRIDGE, 
						("%s bridgeEventProcessor returned error\n",fn));
				}

				break;

			default :
				NETERROR(MBRIDGE,
					("%s invalid conf type (%d)\n", fn,confHandle->confType));
				free(pSipEvt);	
				break;
			}
		}
	}

	if (portAllocated)
	{
		// A port was allocated on the destination side,
		// but since an error has happened, it must
		// be freed

		GwPortFreeCall(&callHandle1->rfphonode, 1, 1);
		if(callHandle1->flags & FL_FREE_DESTPORT)
		{
			callHandle1->flags &= ~FL_FREE_DESTPORT;
		}
	}

	// Copy the old conf id and callid back
	if (callHandle1)
	{
		memcpy(pSipData->callID, callHandle1->callID, CALL_ID_LEN);

		// Free up any sdp which came in
		SipCheckFree(pSipData->localSet);
		pSipData->localSet = 0;
		pSipData->nlocalset = 0;

		// Free any incoming ISUP
		SipCheckFree (pSipData->isup_msg);
		pSipData->isup_msg = NULL;
		SipCheckFree (pSipData->isupTypeVersion);
		pSipData->isupTypeVersion = NULL;
		SipCheckFree (pSipData->isupTypeBase);
		pSipData->isupTypeBase = NULL;
		SipCheckFree (pSipData->isupDisposition);
		pSipData->isupDisposition = NULL;
		SipCheckFree (pSipData->isupHandling);
		pSipData->isupHandling = NULL;
		pSipData->isup_msg_len = 0;

		// Free any incoming QSIG
		SipCheckFree (pSipData->qsig_msg);
		pSipData->qsig_msg = NULL;
		SipCheckFree (pSipData->qsigTypeVersion);
		pSipData->qsigTypeVersion = NULL;
		SipCheckFree (pSipData->qsigDisposition);
		pSipData->qsigDisposition = NULL;
		SipCheckFree (pSipData->qsigHandling);
		pSipData->qsigHandling = NULL;
		pSipData->qsig_msg_len = 0;

		memcpy(evtPtr->callID, callHandle1->callID, CALL_ID_LEN);
		evtPtr->callDetails.callError = callError;
		evtPtr->callDetails.lastEvent = SCC_evtResolve;

		evtPtr->event = Sip_eBridgeError;
		evtPtr->evtProcessor = NULL;
		evtPtr->callDetails.flags = 0;

		if(sipBridgeEventProcessor(evtPtr) < 0 )
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG2,
				("%s bridgeEventProcessor returned error\n",fn));
		}

	}
	else
	{
		// Do this to prevent any memory leaks

		// We must free the event handle and whatever came in,
		// as there is no one to handle the error
		SipFreeAppCallHandle(evtPtr->data);
		free(evtPtr);
	}

	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	
	return -1;
}
