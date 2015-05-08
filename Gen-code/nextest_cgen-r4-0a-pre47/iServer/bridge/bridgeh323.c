#include "gk.h"
#include "firewallcontrol.h"
#include "trigger.h"
#include "callutils.h"
#include "resutils.h"
#include "bridge.h"
#include "log.h"
#include "iwfsmproto.h"



/**
 * This method needs to finish whatever is left to do from bridgeH323EventProcessorWorker()
 * after it called openFirewallOnH323Events().
 *
 * This is called from bridgefw.c in two ways:
 *    a. when it determines no holes need to be opened, it calls this right from the thread
 *       that calls openFirewallOnH323Events()
 *    b. when holes need to be opened, it calls this from the callback method called by MFCP
 *       (so it happens in a different thread)
 */
void bridgeFceH323Callback (SCC_EventBlock *evtPtr)
{

  // if any error has happened...
  if (evtPtr->callDetails.fceStatus.rval == FCE_SUCCESS)
  {
    // hole opened successfully
	bridgeH323EventProcessor(evtPtr);
  }
  else
  {
    // error processing
	bridgeH323EventProcessor(evtPtr);
  }

}


/* 
*  Frees up allocated GwPort in case of error in Setup
*  Always Free up eventptr
*/
int bridgeH323EventProcessorWorker(SCC_EventBlock *evtPtr)
{
	static char fn[] = "bridgeH323EventProcessorWorker";
	CallHandle 			*callHandle1 = NULL,*callHandle2 = NULL;
	CallHandle 			*callHandleRelated = NULL, *ch = NULL;
	char 				str[EVENT_STRLEN];
	H323EventData   	*pH323Data;
	SCC_CallHandleType 	call2Type;
	char				srcCallID[CALL_ID_LEN] = {0},
						destCallID[CALL_ID_LEN] = {0};
	ConfHandle			*confHandle;
	char				callIDStr[CALL_ID_LEN],confIDStr[CONF_ID_LEN], callIDStr1[CALL_ID_LEN];
	char				ipStr[20], *evName;
	int					rv = -1;
	int 				callError = SCC_errorNone;
	int					portAllocated = 0, handleAllocated = 0;
	InfoEntry			dinfo;
	int					vendor = 0;
	FCEStatusStruct		status = { 0 };
	int					herror, serror;
	TriggerEntry		tgEntry;
	CacheTriggerEntry	*tgPtr;
	int			drealmId = -1, mydomain = 0;

	if(!evtPtr)
	{
		NETERROR(MBRIDGE,("%s Null EventPtr \n",fn));
		return -1;
	}

	evtPtr->callDetails.callType = CAP_H323;
	pH323Data = (H323EventData *) evtPtr->data;

	evName = GetH323Event(evtPtr->event);

	NETDEBUG(MBRIDGE, NETLOG_DEBUG4,("%s Event(%s)\n",
		fn, (char*) evName));

	if(!pH323Data)
	{
		NETERROR(MBRIDGE,("%s Null pH323Data \n",fn));
		H323FreeEvent(evtPtr);
		return -1;
	}

	NETDEBUG(MBRIDGE,NETLOG_DEBUG4,
		("%s Event(%s)\n", fn, evName));

	memcpy(srcCallID,evtPtr->callID,CALL_ID_LEN);

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	bridgeFindCallLegsInfoForEvent(evtPtr, &callHandle1, &callHandle2, &confHandle);

	if (callHandle1 && pH323Data->localSet && (IsFCEEnabled() == TRUE) &&
  		FCE_STATUS_IS_NULL(evtPtr->callDetails.fceStatus) &&
	   	H323EventNeedsFCE(evtPtr->event))
	{
	  	// check pending fce events on the call handle
		if (callHandle1->flags & FL_CALL_FCPEND)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s FCE event added to list for event %s\n", fn, evName));

			evtPtr->evtProcessor = (int (*)(void))bridgeH323EventProcessorWorker;

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

	  	// check pending fce events on the peer call handle
		if (callHandle2 && (callHandle2->flags & FL_CALL_FCPEND))
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s FCE event added to peer leg list for event %s\n", fn, evName));

			evtPtr->evtProcessor = (int (*)(void))bridgeH323EventProcessorWorker;

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
	
	if( !H323EventNeedsFCE(evtPtr->event)  && (callHandle1 != NULL)  &&
  		FCE_STATUS_IS_NULL(evtPtr->callDetails.fceStatus) &&
		(callHandle1->flags & FL_CALL_FCPEND) )	    
	{
		NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
			 ("%s FCE event added to same leg serializer list for event %s\n", fn, evName));
		
		evtPtr->evtProcessor = (int (*)(void))bridgeH323EventProcessorWorker;
		evtPtr->callDetails.fceStatus.rval = FCE_SUCCESS;
		
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

  	if (!(FCE_STATUS_IS_NULL(evtPtr->callDetails.fceStatus)))
	{
	  	status = evtPtr->callDetails.fceStatus;

		if (evtPtr->event == SCC_eBridgeSetup)
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
				("%s resuming leg2: openFirewallOnH323Events invalid call state %p/%p/%p event = %d\n", 
				fn, callHandle1, confHandle, callHandle2, evtPtr->event));

			callError = SCC_errorResourceUnavailable;
			goto _error;
		}

		memcpy(destCallID,callHandle2->callID,CALL_ID_LEN);

		if (callHandle1->mediaTurning && 
			(evtPtr->event == SCC_eBridgeChanConnect))
		{
			// Disable MediaTurning for future events - e.g. holding	
			callHandle1->mediaTurning = 0;
		}

		// set up some variables, for the context
  		if (evtPtr->callDetails.fceStatus.rval == FCE_SUCCESS)
		{
			if(H323EventNeedsFCE(evtPtr->event))
			{
				serror = UpdateFCEInfo(evtPtr, callHandle1, callHandle2, confHandle);
				if (serror < 0)
					goto _error;
			}

			goto _continue_leg2;
		}
		else
		{
			NETERROR(MBRIDGE, 
				("%s resuming leg2: openFirewallOnH323Events failed! event = %d\n", fn,evtPtr->event));

			callError = SCC_errorResourceUnavailable;
			goto _error;
		}
	}

	if(evtPtr->event == SCC_eBridgeSetup)
	{
		if(callHandle1 == NULL)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1, 
				("%s Failed to find callHandle %s.\n",
				fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1, 
				("%s Event = %s.\n",
				fn,SCC_EventToStr(evtPtr->event,str)));
			H323FreeEvent(evtPtr);
			goto _return;
		}

		if (pH323Data->requri == 0)
		{
			mydomain = 1;
		}
		else 
		{
			drealmId = RealmLocate(pH323Data->requri->realmId>0?pH323Data->requri->realmId:callHandle1->realmInfo->realmId, 
							pH323Data->requri->host, 0, &mydomain);
		}

		if (mydomain)
		{
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

			if(bridgeResolveH323Cache(callHandle1,pH323Data,1,1, &callError, NULL, &dinfo)<=0)
			{
				NETDEBUG(MSCC,NETLOG_DEBUG2,
					("%s bridgeResolveH323Cache Failed for = %s\n",
					fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
				goto _dropincoming;
			}
		}
		else
		{
			if (drealmId < 0)
			{
				// In this case we will attempt to set the realm to that which is in the URI
				// If the URI was extracted from a contact, the TSM sets up the RealmId of
				// the realm on which the contact was received
				// If this realmId is 0, this may be because it has not been uninitialized
				// In any case, we will attempt to send the message out on the default realm
				
				drealmId = pH323Data->requri->realmId;
			}

			if (drealmId < 0)
			{
				NETERROR(MSIP, ("%s Destination proxy realm unknown for %s\n",
					fn, pH323Data->requri->host));

				callError = SCC_errorNoRoute;
				goto _error;
			}

			BillCallPrevCdr(callHandle1);

			memset(&dinfo, 0, sizeof(InfoEntry));
			callHandle1->rfphonode.ipaddress.l = 
				dinfo.ipaddress.l = ResolveDNS(pH323Data->requri->host, &herror);
			BIT_SET(callHandle1->rfphonode.sflags, ISSET_IPADDRESS);
			dinfo.realmId = drealmId;

			if(pH323Data->requri->name && strstr(pH323Data->requri->name,";cic="))
			{
				vendor = Vendor_eSonusGSX;
			}		
		}

		// For all purposes, the port HAS BEEN allocated
		// on the gateway, and since the freeing up of the port is
		// associated to the call going to idle state
		// ANY error which happens until the handle is associated
		// with a call MUST free up the port right there.

		portAllocated = 1;

		if (callHandle1->flags & FL_CALL_CISCO_ANI)
		{
			strcpy(callHandle1->phonode.phone, callHandle1->inputANI);
			callHandle1->flags &= ~FL_CALL_CISCO_ANI;
		}

		strcpy(pH323Data->calledpn,callHandle1->rfphonode.phone);
		strcpy(pH323Data->callingpn,callHandle1->phonode.phone);

		// prefer Sip over H323 
 
		if(BIT_TEST(callHandle1->rfphonode.cap, CAP_H323))
		{
			call2Type = SCC_eH323CallHandle;
		}	
		else if(BIT_TEST(callHandle1->rfphonode.cap, CAP_SIP))
		{
			call2Type = SCC_eSipCallHandle;
		}
		else if(pH323Data->requri)
		{
			call2Type = SCC_eSipCallHandle;
		}
		else 
		{
			NETERROR(MBRIDGE,("%s UnSupported Remote Endpoint\n",fn));
			callError = SCC_errorDestinationUnreachable;
			goto _dropincoming;
		}

		if (!confHandle)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s Failed to find confHandle \n",fn));

			callError = SCC_errorAbandoned;
			goto _dropincoming;
		}


		// This is to handle an ipphne with max calls set to -1
		if(GwPortAvailCall(&callHandle1->rfphonode,0,1) < 0 )
		{
			NETERROR(MBRIDGE,("%s GwPortAvailCall for destination failed ",fn));
			callError = SCC_errorDestBlockedUser;
			goto _dropincoming;
		}			

		// Set Sm type in the  confhandle
		confHandle->confType = DetermineConfType(SCC_eH323CallHandle,call2Type);
		confHandle->state = IWF_sIdle;

        	NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
            		("%s Updating conf handle %s into the cache\n",
            		fn, (char*) CallID2String(confHandle->confID, confIDStr)));

		/* Now we need to create callhandle for leg2 */
		// Add Call Handle To Conf 
		if(!(callHandle2 = GisAllocCallHandle()))
		{
				NETERROR(MBRIDGE,("%s GisAllocCallHandle failed.\n",fn));
				callError = SCC_errorResourceUnavailable;
				goto _dropincoming;
		}

		handleAllocated = 1;

		// We are not sure this will be an iwf call or not
		generateOutgoingH323CallId(callHandle2->callID, 
			BIT_TEST(callHandle1->rfphonode.cap, CAP_ARQ));

		memcpy(callHandle2->confID, confHandle->confID, CONF_ID_LEN);

		if(call2Type == SCC_eSipCallHandle)
		{
			callHandle2->bridgeEventProcessor = sipBridgeEventProcessor;

			// fix the phone number
			SipStripUriParams(callHandle1->rfphonode.phone);

			// Sip end does not expect the state to be initialized
		} 
		else if(call2Type == SCC_eH323CallHandle) {
			// Don't need IWF
			callHandle2->bridgeEventProcessor = SCC_H323Leg2BridgeEventProcessor;
			H323controlState(callHandle2) = UH323_sControlIdle;
			callHandle2->state = SCC_sIdle;
			memcpy(H323Callq931IE(callHandle2),dinfo.q931IE,Q931IE_LEN);
			memcpy(H323Callbcap(callHandle2),dinfo.bcap,BCAP_LEN);
			H323infotranscap(callHandle2) = dinfo.infoTransCap;
			NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s InforTransCap = %d\n",fn, dinfo.infoTransCap)); // for testing

			H323OrigCalledPartyNumType(callHandle2) = 
				H323OrigCalledPartyNumType(callHandle1);

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

			// Set the orig calling party number type on leg2
			// based on what came in on leg1
			H323OrigCallingPartyNumType(callHandle2) = 
				H323OrigCallingPartyNumType(callHandle1);

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

		// Populate CallHandle2 structures
	    	memcpy(&callHandle2->phonode,&callHandle1->rfphonode,sizeof(PhoNode));
		memcpy(&callHandle2->rfphonode,&callHandle1->phonode,sizeof(PhoNode));
		callHandle2->handleType = call2Type;
		if(!vendor)
		{
			if (dinfo.vendor)
				callHandle2->vendor = dinfo.vendor;
		}
		else {
			callHandle2->vendor = vendor;
		}
		callHandle2->cap = dinfo.cap;
		callHandle2->ecaps1 = dinfo.ecaps1;
		callHandle2->peerIp = dinfo.ipaddress.l;
		callHandle2->realmInfo = getRealmInfo (dinfo.realmId, callCache->malloc);
		if (callHandle2->realmInfo == NULL)
		{
			NETERROR (MBRIDGE, ("%s: Failed to get realm info for id %d.\n", fn, dinfo.realmId));
			callError = SCC_errorResourceUnavailable;
			goto _dropincoming;
		}
		NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s: Retrieved realm info for realm %d:", fn, dinfo.realmId));

	 	// if  db entry has nat-ip and nat-port set fill them into the sip data
		if ( BIT_TEST(dinfo.sflags, ISSET_NATIP) )
		{
			callHandle2->natip = dinfo.natIp;
			if( BIT_TEST(dinfo.sflags, ISSET_NATPORT) )
			{
				callHandle2->natport = dinfo.natPort;
			}
		}

		if ( !(callHandle2->ecaps1 & ECAPS1_DELTCSRFC2833DFT) )
		{
			/* use system default */
			h323RemoveTcs2833 ? (callHandle2->ecaps1 |= ECAPS1_DELTCSRFC2833) : (callHandle2->ecaps1 &= ~ECAPS1_DELTCSRFC2833);
		}
		if ( !(callHandle2->ecaps1 & ECAPS1_DELTCST38DFT) )
		{
			/* use system default */
			h323RemoveTcsT38 ? (callHandle2->ecaps1 |= ECAPS1_DELTCST38) : (callHandle2->ecaps1 &= ~ECAPS1_DELTCST38);
		}

		// set the TAP flag for the dest call handle.
		// one of these calls should be fine, as we have the cache
		// access here
		callHandle2->flags |= (pH323Data->flags & FL_CALL_TAP);

		memcpy(callHandle2->acct_session_id, callHandle1->acct_session_id, sizeof(callHandle1->acct_session_id));

		callHandle2->incoming_conf_id = 
			CStrdup(callCache, callHandle1->incoming_conf_id);

		memcpy(callHandle2->inputNumber, callHandle1->inputNumber, PHONE_NUM_LEN);

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

		if (mydomain || !forwardSrcAddr)
		{
			
			pH323Data->srcsigip = callHandle2->realmInfo->rsa;
		}


		if(GisAddCallToConf(callHandle2)!=0)
		{
			NETERROR(MBRIDGE,("%s GisAddCallToConf 2 failed.\n",fn));
			goto _error;
		}

		if(CacheInsert(callCache, callHandle2) < 0)
		{
			NETERROR (MBRIDGE,("%s CacheInsert for H323 callHandle2 failed.\n",fn));
			callError = SCC_errorResourceUnavailable;
			goto _error;
		}

		callHandle2->callSource=1;
		timedef_cur(&(callHandle2->callStartTime));

	} 
	else { 
		if (!confHandle || !callHandle2)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s Failed to find confHandle \n",fn));

			callError = SCC_errorNoCallHandle;
			// Can't find peer leg if no conf handle
			goto _dropincoming;
		}
	}

	// CallHandle2 will be populated here in all cases
	memcpy(destCallID,callHandle2->callID,CALL_ID_LEN);

	if (callHandle1 && (evtPtr->event == SCC_eBridgeRequestMode) &&
			!strcmp(pH323Data->modeName, "t38fax"))
	{
		// Check the trigger cache
		tgEntry.event = TRIGGER_EVENT_H323REQMODEFAX;
		tgEntry.srcvendor = callHandle1->vendor;
		tgEntry.dstvendor = callHandle2->vendor;

		CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);
		tgPtr = CacheGet(triggerCache, &tgEntry);	
		if(!tgPtr)
		{
			// now look for a trigger based on call origination
			tgEntry.event = TRIGGER_EVENT_H323T38FAX;
			if(callHandle1->callSource)
			{
				tgEntry.srcvendor = callHandle2->vendor;
				tgEntry.dstvendor = callHandle1->vendor;
			}
			else
			{
				tgEntry.srcvendor = callHandle1->vendor;
				tgEntry.dstvendor = callHandle2->vendor;
			}
			tgPtr = CacheGet(triggerCache, &tgEntry);
		}


		if (tgPtr && tgPtr->trigger.srcvendor && tgPtr->trigger.dstvendor)
		{
			if(callHandle1->callSource)
			{
				TriggerInstall(tgPtr, callHandle1, callHandle2);
			}
			else
			{
				TriggerInstall(tgPtr, callHandle2, callHandle1);
			}
		}

		CacheReleaseLocks(triggerCache);
	}

	if (callHandle1 && (evtPtr->event == SCC_eBridgeConnect))
	{
		if (strlen(tapcall) && (callHandle1->flags & FL_CALL_TAP))
		{
			// Spawn off a tapcall
			CallTap(callHandle1, 1);
		}
	}

	if (pH323Data->localSet == NULL) {
		NETDEBUG(MFCE, NETLOG_DEBUG2, ("%s: pH323Data->localSet is NULL for eventType %d\n", fn, evtPtr->event));

		if ((evtPtr->event == SCC_eBridgeSetup) &&
			(callHandle2) && (callHandle2->handleType == SCC_eSipCallHandle)) {
			// Create a fake sdp before opening holes
			pH323Data->localSet = (RTPSet *) calloc(1, sizeof(RTPSet));
			pH323Data->nlocalset = 1;
			pH323Data->localSet[0].codecType = defaultCodec;
			// The following line sets the default param for the above codec
			setDefaultRtpParams(pH323Data->localSet, pH323Data->nlocalset);
			callHandle1->lastMediaIp = pH323Data->localSet[0].rtpaddr = callHandle1->peerIp;
			// The first unassigned pair 14,15 according to IANA
			callHandle1->lastMediaPort = pH323Data->localSet[0].rtpport = 14;
			pH323Data->localSet[0].mLineNo = 0;
			pH323Data->localSet[0].direction = SendRecv;
			NETDEBUG(MFCE, NETLOG_DEBUG2, ("%s: setting pH323Data->localSet as media turning enabled \n", fn));
			callHandle1->mediaTurning = 1;
		}
	}

	if (callHandle1 && pH323Data->localSet && (IsFCEEnabled() == TRUE) && H323EventNeedsFCE(evtPtr->event))
	{
		evtPtr->evtProcessor = (int (*)(void))bridgeFceH323Callback;

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

		if (status.rval ==  FCE_FAILURE)
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
					("%s openFirewallOnH323Events failed! event = %d\n", fn,evtPtr->event));
				if (evtPtr->event == SCC_eBridgeSetup)
				{
					callError = SCC_errorFCECallSetup;
				}
				else
				{
					callError = SCC_errorFCE;
				}
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
			NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
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

	// Store the new call id's, in the eventHandles
	memcpy(evtPtr->callID, destCallID, CALL_ID_LEN);

	// Add destination realm Info
	// That is already there inside the callHandle, when it reaches the second leg
	// Need to Fix this - Eliminate returning  ERROR
	switch(confHandle->confType)
	{
	case Conf_eH3232SipP2P :
			// delete callHandle from Conference 
			/* The callHandle gets in the iwfReleaseComp -if hangup is 
			* initiated from H323
			*/

			if (iwfH323EventProcessor(evtPtr) <0 )
			{
				NETDEBUG(MBRIDGE, NETLOG_DEBUG2,
					("%s iwfH323EventProcessor returned error\n",fn));
				goto _error;
			}

			break;

	case Conf_eH323P2P :
            if ((callHandle1 != NULL) && (callHandle1->callSource == 1) &&
                (evtPtr->event == SCC_eBridgeReleaseComp))
            {
				// delete callhandle1 from confhandle
                GisDeleteCallFromConf(srcCallID, evtPtr->confID);
            }

			// The bridge Event Processor will be set approprately
			if(callHandle2->bridgeEventProcessor(evtPtr) <0 )
			{
				NETDEBUG(MBRIDGE, NETLOG_DEBUG2,
					("%s bridgeEventProcessor returned error\n",fn));
				goto _error;
			}

			break;

	default :
			NETERROR(MBRIDGE,
				("%s received invalid conf type (%d)\n",fn,confHandle->confType));
			H323FreeEvent(evtPtr);
	}

	rv = 0;

  	if (callHandle1 && (status.rval == FCE_SUCCESS))
	{
		// Do any remaining events queued up in the end
		BridgeCheckPendingFCEvents(callHandle1);
	}

_return:
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	return rv;

_error:
   	if (evtPtr->callDetails.fceResource)
	{
		FCECloseResource(evtPtr->callDetails.fceSession, evtPtr->callDetails.fceResource);
	}

	//send ReleaseComp on callHandle2 
	if(callHandle2)
	{
		if(handleAllocated)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG2, ("%s Error !! Deleting callHandle2\n", fn));
			GisDeleteCallFromConf(callHandle2->callID,
				callHandle2->confID);
			CallDelete(callCache, callHandle2->callID);
			GisFreeCallHandle(callHandle2);
		}
		else 
		{
			H323EventData       *pH323Data;
			SCC_EventBlock      *pH323Evt;

			pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
			memset(pH323Evt,0,sizeof(SCC_EventBlock));

			pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
			memset(pH323Data,0,sizeof(H323EventData));

			pH323Evt->event = SCC_eBridgeReleaseComp;
			pH323Evt->callDetails.callError = callError;
			pH323Evt->callDetails.lastEvent = SCC_evtResolve;

			memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
			memcpy(pH323Evt->callID,destCallID,CALL_ID_LEN);
			memcpy(pH323Data->callID,destCallID,CALL_ID_LEN);
			pH323Evt->data = (void *) pH323Data;
			switch(confHandle->confType)
			{
			case Conf_eH3232SipP2P :
					// delete callHandle from Conference 
					/* The callHandle gets in the iwfReleaseComp -if hangup is 
					* initiated from H323
					*/
					if (iwfH323EventProcessor(pH323Evt) <0 )
					{
						NETDEBUG(MBRIDGE, NETLOG_DEBUG2,
							("%s iwfH323EventProcessor returned error\n",fn));
					}

					break;

			case Conf_eH323P2P :

					// The bridge Event Processor will be set approprately
					if(callHandle2->bridgeEventProcessor(pH323Evt) <0 )
					{
						NETDEBUG(MBRIDGE, NETLOG_DEBUG2,
							("%s bridgeEventProcessor returned error\n",fn));
					}

					break;

			default :
					H323FreeEvent(pH323Evt);
					NETERROR(MBRIDGE,
						("%s invalid conf type (%d)\n",fn,confHandle->confType));
			}
		}
	}
_dropincoming:
	if(portAllocated)
	{
		GwPortFreeCall(&callHandle1->rfphonode, 1, 1);
		if(callHandle1->flags & FL_FREE_DESTPORT)
		{
			callHandle1->flags &= ~FL_FREE_DESTPORT;
		}
	}

	//Send ReleaseComplete on callHandle1
	if(callHandle1)
	{
		H323EventData       *pH323Data;
		SCC_EventBlock      *pH323Evt;

		pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
		memset(pH323Evt,0,sizeof(SCC_EventBlock));

		pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
		memset(pH323Data,0,sizeof(H323EventData));

		pH323Evt->event = SCC_eBridgeReleaseComp;
		pH323Evt->callDetails.callError = callError;
		pH323Evt->callDetails.lastEvent = SCC_evtResolve;

		memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
		memcpy(pH323Evt->callID,srcCallID,CALL_ID_LEN);
		memcpy(pH323Data->callID,srcCallID,CALL_ID_LEN);
		pH323Evt->data = (void *) pH323Data;

		if(callHandle1->bridgeEventProcessor(pH323Evt) <0 )
		{
			NETDEBUG(MIWF, NETLOG_DEBUG2,
				("%s bridgeEventProcessor returned error\n",fn));
		}
	}
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	H323FreeEvent(evtPtr);
	return -1;
}
