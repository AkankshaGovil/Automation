#include "gis.h"
#include "uh323inc.h"
#include "firewallcontrol.h"
#include "bridge.h"
#include "licenseIf.h"
#include "callutils.h"
#include "log.h"

#define IS_FW_SYMMETRIC ((strcasecmp(fceConfigFwName, "MFCP") == 0)? 0:1)

typedef enum {
	NOP = 0,
	OPEN = 1,
	MODIFY = 2,
	CLOSE = 3,
} fwop;

char *
fwop2str(fwop op) {
	switch(op) {
		case NOP:
		   return "no-op";	
		case OPEN:
		   return "open";	
		case MODIFY:
		   return "modify";	
		case CLOSE:
		   return "close";	
		default :
		   return "unknown";	
	}
}

/**
 * allocate a media routing license
 *
 * @return 0 if success and -1 if not
 */
int allocateFCELicense (ConfHandle *confHandle)
{

  /* if we haven't allocated a license yet, allocate one */
  if (!confHandle->mediaRouted)
  {
    if (nlm_getMRvport())
      return -1;  // unable to allocate a license
    confHandle->mediaRouted = 1;
  }

  return 0;
}

/**
 * this is the callback function that gets passed to all the MFCP calls
 *
 * when MFCP calls this back, we fill in the data in evtPtr and call the callback into
 * bridge
 */
void BridgeMFCPCallback (MFCP_Request *rPtr)
{
  SCC_EventBlock *evtPtr = (SCC_EventBlock *)mfcp_get_res_appdata(rPtr);
  SipAppCallHandle *pSipData;
  static char *fn="BridgeMFCPCallback", ipstr1[32], ipstr2[32];
  H323EventData *pH323Data;
  RTPSet *localSet;
  int count, nlocalset;

  if (rPtr) {
	// Is an MFCP response
    if (mfcp_get_res_status(rPtr) == MFCP_RSTATUS_OK) {
      // is the call successful?
      evtPtr->callDetails.fceDstIp = mfcp_get_nat_dest_addr(rPtr);
      evtPtr->callDetails.fceDstPort = mfcp_get_nat_dest_port(rPtr);
      evtPtr->callDetails.fceSrcIp = mfcp_get_nat_src_addr(rPtr);
      evtPtr->callDetails.fceSrcPort = mfcp_get_nat_src_port(rPtr);
      evtPtr->callDetails.fceResource = mfcp_get_int(rPtr, MFCP_PARAM_RESOURCE_ID);
      evtPtr->callDetails.fceStatus.rval = FCE_SUCCESS;

      NETDEBUG(MFCE,NETLOG_DEBUG2,
			    ("%s: nat_dst = %s:%d, nat_src = %s:%d\n",fn,
				 FormatIpAddress(evtPtr->callDetails.fceDstIp, ipstr1),
				 evtPtr->callDetails.fceDstPort,
				 FormatIpAddress(evtPtr->callDetails.fceSrcIp, ipstr2),
				 evtPtr->callDetails.fceSrcPort
				 ));
    }
    else {
      NETERROR(MFCE, 
			("%s: unable to open media hole for %d:%d->%d: %s\n", fn, \
			 mfcp_get_int(rPtr, MFCP_PARAM_BUNDLE_ID), \
			 mfcp_get_int(rPtr, MFCP_PARAM_SRC_POOL), \
			 mfcp_get_int(rPtr, MFCP_PARAM_DEST_POOL), \
			 mfcp_get_res_estring(rPtr)));

      evtPtr->callDetails.fceResource = 0;
	  evtPtr->callDetails.fceStatus.rval = FCE_FAILURE;
	  evtPtr->callDetails.fceStatus.err = FCE_GENERAL_FAILURE;
    }

    // free the mfcp resources
    mfcp_req_free(rPtr);
  }

  // call back into bridge
  evtPtr->evtProcessor(evtPtr);
}

/**
 *
 * @param evtPtr the SCC event pointer
 * @param eventType the event type string
 * @param callHandle1 the call handle of the call leg that received this event
 * @param callHandle2 the call handle of the call leg on the other side
 * @param confHandle the conference handle of the call
 *
 * @return FCE_SUCCESS/FCE_INPROGRESS if success or FCE_LICENSE_FAILURE/FCE_GENERAL_FAILURE on error
 */
FCEStatusStruct
openPinholes (SCC_EventBlock *evtPtr,
			 CallHandle *callHandle1,
			 CallHandle *callHandle2,
			 ConfHandle *confHandle)
{
  int count;
  int retval;
  static char *fn = "openPinholes";
  char *opstr;
  fwop op;
  char eventType[80];
  char ipstr1[32], ipstr2[32];
  char callIDStr1[CALL_ID_LEN], callIDStr2[CALL_ID_LEN], PeerSrcCallID[CALL_ID_LEN];
  SipAppCallHandle *pSipData;
  H323EventData *pH323Data;
  RTPSet *localSet;
  int nlocalset;
  int mediaChanged = 0, epChanged = 0, mediaRoute = 0, resource_exists = 0, nullsdp = 0;
  int dtmf_detect;
  int dtmf_detect_param = 0;
  CallHandle *srcCH, *dstCH;
  CallRealmInfo *sri, *dri; 
  unsigned int nat_src_ip, nat_dst_ip;
  unsigned short nat_src_port, nat_dst_port;
  FCEStatusStruct status;
  CallHandle *peersrcCH;

  // media flow is the opposite direction of the signal flow

  EventToStr(evtPtr->event, evtPtr->callDetails.callType, eventType);
  status.rval = FCE_FAILURE;
  status.err = FCE_GENERAL_FAILURE;

  dstCH = callHandle2;
  resource_exists = (CallFceResourceId(dstCH) != 0);

  switch (evtPtr->callDetails.callType)
  {
    case CAP_SIP:
      pSipData = (SipAppCallHandle *)evtPtr->data;
      localSet = pSipData->localSet;
      nlocalset = pSipData->nlocalset;
      mediaChanged = pSipData->mediaChanged;
      break;

    case CAP_H323:
      pH323Data = (H323EventData *)evtPtr->data;
      localSet = pH323Data->localSet;
      nlocalset = pH323Data->nlocalset;
      mediaChanged = pH323Data->mediaChanged;
      break;

    default:
      NETERROR(MFCE, ("%s: invalid call type received for %s: %d\n", 
			fn, eventType, evtPtr->callDetails.callType));
	  return (status);
  }

  /* For null sdp i.e. hold do not send it as a change in media */
  if (localSet[0].rtpaddr) {
  	CallFceUntranslatedIp(dstCH) = localSet[0].rtpaddr;
  	CallFceUntranslatedPort(dstCH) = localSet[0].rtpport;
  }
  else {
	nullsdp = 1;
  }

  /* Figure out the src and dst CallHandles */
  if (srcCH = GetMediaSrcFromTip((unsigned int *)&CallFceUntranslatedIp(dstCH)))
  {
	  epChanged = 1;
		NETDEBUG(MFCE, NETLOG_DEBUG1,
			("%s: Found related call %s with incoming call %s\n",
			fn, (char*) CallID2String(srcCH->callID,callIDStr1),
			(char*) CallID2String(callHandle1->callID,callIDStr2)));

		// close hole on callHandleRelated (leg1)
		// open new hole on callHandleRelated (leg1), treating callHandleRelated
		// and callHandle2 (leg4) as callHandle1 and callHandle2 respectively.
		// store this new resourceId on callHandleRelated (callHandl1e1)
		// The translated address will be stored on callHandle1, as usual
		// when the FC call back returns. The changed address/port values will
		// go towards callHandle2 through signaling
		if (srcCH != callHandle1) {
			evtPtr->callDetails.relatedCallID = malloc(CALL_ID_LEN);
			memcpy(evtPtr->callDetails.relatedCallID, srcCH->callID, CALL_ID_LEN);
			if (getPeerCallID(srcCH->confID, srcCH->callID, PeerSrcCallID) == 0) {
				if ( (peersrcCH = CacheGet(callCache, PeerSrcCallID)) == NULL) {
			    	NETERROR(MFCE, ("%s: Can not get untranslated address for call %s from call %s\n",
					  fn, CallID2String(callHandle1->callID, callIDStr1), CallID2String(srcCH->callID, PeerSrcCallID)));
				}
  				CallFceUntranslatedIp(dstCH) = CallFceUntranslatedIp(peersrcCH);
  				CallFceUntranslatedPort(dstCH) = CallFceUntranslatedPort(peersrcCH);
				NETDEBUG(MFCE, NETLOG_DEBUG1,
					("%s: related call has untranslated address %s:%d\n",
						fn, FormatIpAddress(CallFceUntranslatedIp(dstCH), ipstr1),
						CallFceUntranslatedPort(dstCH)));
			}
			else {
			    NETERROR(MFCE, ("%s: Can not find peer for call %s\n",
					  fn, CallID2String(srcCH->callID, PeerSrcCallID)));
			}

		}
  }
  else
  {
    srcCH = callHandle1;
  }

  sri = srcCH->realmInfo;
  dri = dstCH->realmInfo;
  dtmf_detect = ( (srcCH->handleType == SCC_eSipCallHandle) && (dstCH->handleType == SCC_eSipCallHandle) && \
                  Is2833Supported(srcCH->ecaps1) && Is2833NotSupported(dstCH->ecaps1) );

  mediaRoute = EPBasedMR(sri, dri, srcCH, dstCH); 

  /* figure out if the operation is going to be nop/open/modify/close hole */
  if (mediaRoute) {
	if (resource_exists) {
  	  if ((epChanged || mediaChanged) && (!nullsdp)) {
        op = MODIFY;
      }
	  else {
		op = NOP;
      }
    }
	else {
      op = OPEN;
    }
  }
  else {
    if (resource_exists) {
      op = CLOSE;
    }
    else {
      op = NOP;
    }
  }
  opstr = fwop2str(op);
  status.op = op;
    
  NETDEBUG(MFCE, NETLOG_DEBUG2, 
    ("%s: Will %s a media hole for callId %s (%lu:%s-->%lu:%s)\n", fn, opstr, \
      (char*) CallID2String(callHandle1->callID,callIDStr2), \
      sri->realmId, FormatIpAddress(sri->rsa, ipstr1), \
	  dri->realmId, FormatIpAddress(dri->rsa, ipstr2)));

  switch(op) {
    case NOP: 
      if (resource_exists) {
        evtPtr->callDetails.fceResource = CallFceResourceId(dstCH);
        NETDEBUG(MFCE, NETLOG_DEBUG2, 
	      ("%s: Using existing media hole for %s [%s:%d]\n", fn, eventType, \
           FormatIpAddress(CallFceTranslatedIp(callHandle2), ipstr1), \
           CallFceTranslatedPort(callHandle2)));
      }
      else {
        NETDEBUG(MFCE, NETLOG_DEBUG2, 
	      ("%s: Using no media hole for %s [%s:%d]\n", fn, eventType, \
           FormatIpAddress(CallFceUntranslatedIp(callHandle2), ipstr1), \
           CallFceUntranslatedPort(callHandle2)));
        evtPtr->callDetails.fceResource = 0;
        evtPtr->callDetails.fceSession = 0;
      }
      status.rval = FCE_SUCCESS;
      break;

    case CLOSE:
      FCECloseResource(CallFceSession(dstCH), CallFceBundleId(dstCH));
      evtPtr->callDetails.fceResource = 0;
      evtPtr->callDetails.fceSession = 0;
      status.rval = FCE_SUCCESS;
      break;

    case OPEN:
      /* a new hole need to be opened or an existing hole need to be reopened */

	if( !natEnabled() && (srcCH->ecaps1 & ECAPS1_NATDETECT) )
	{
        	NETERROR(MFCE, ("%s: NAT Traversal Feature not licensed\n", fn));
        	status.err = FCE_NATT_LICENSE_FAILURE;
		status.rval = FCE_FAILURE;
		break;
	}

      if (allocateFCELicense(confHandle) < 0) {
        status.err = FCE_LICENSE_FAILURE;
		break;
      }		

      // license allocated, open a new hole
      CallFceSession(dstCH) = FCEAllocateBundleId(&CallFceBundleId(dstCH));
      if (CallFceSession(dstCH) == NULL) {
        evtPtr->callDetails.fceSession = 0;
        NETERROR(MFCE, ("%s: unable to allocate bundle/session for %s\n", fn, eventType));
      }
      else
      {
		status.rval = FCE_INPROGRESS;
        evtPtr->callDetails.fceStatus = status;
        evtPtr->callDetails.fceSession = CallFceSession(dstCH);

		// This may be an asynchronous call. Hence, after calling the following function
		// shouldn't modify the evtPtr in this thread

	if(IS_FW_SYMMETRIC)
	{
        retval = FCEOpenResourceAsync(CallFceSession(dstCH),
                                      CallFceBundleId(dstCH),
                                      CallFceResourceId(srcCH),
                                      FCE_ANY_IP,
                                      FCE_ANY_PORT,
                                      CallFceUntranslatedIp(dstCH),
                                      CallFceUntranslatedPort(dstCH),
                                      CallFceNatDstIp(srcCH),
                                      CallFceNatDstPort(srcCH),
                                      CallFceNatDstIp(dstCH),
                                      CallFceNatDstPort(dstCH),
                                      dri->mPoolId,
                                      sri->mPoolId,
                                      "rtp",
                                      dtmf_detect,
                                      dtmf_detect_param,
                                      CallDstSym(srcCH),
                                      BridgeMFCPCallback,
                                      evtPtr);
	}
	else
	{
        retval = FCEOpenResourceAsync(CallFceSession(dstCH),
                                      CallFceBundleId(dstCH),
                                      CallFceResourceId(srcCH),
                                      FCE_ANY_IP,
                                      FCE_ANY_PORT,
                                      CallFceUntranslatedIp(dstCH),
                                      CallFceUntranslatedPort(dstCH),
                                      0,
                                      0,
                                      0,
                                      0,
                                      dri->mPoolId,
                                      sri->mPoolId,
                                      "rtp",
                                      dtmf_detect,
                                      dtmf_detect_param,
                                      CallDstSym(srcCH),
                                      BridgeMFCPCallback,
                                      evtPtr);
	}
                                        
        if (retval != MFCP_RET_OK) {
          NETERROR(MFCE, 
            ("%s: unable to %s hole for %s, mfcp request failed: %d\n", fn, opstr, eventType, retval));
		  status.rval = FCE_FAILURE;
          status.err = FCE_GENERAL_FAILURE;
        }
      }
	  break;

	case MODIFY: 

	if( !natEnabled() && (srcCH->ecaps1 & ECAPS1_NATDETECT) )
	{
        	NETERROR(MFCE, ("%s: NAT Traversal Feature not licensed\n", fn));
        	status.err = FCE_NATT_LICENSE_FAILURE;
		status.rval = FCE_FAILURE;
		break;
	}

	status.rval = FCE_INPROGRESS;
	evtPtr->callDetails.fceStatus = status;

	if(IS_FW_SYMMETRIC)
	{
      retval = FCEModifyResourceAsync(CallFceSession(dstCH),
                                      CallFceBundleId(dstCH),
                                      CallFceResourceId(dstCH),
                                      CallFceResourceId(srcCH),
                                      FCE_ANY_IP,
                                      FCE_ANY_PORT,
                                      CallFceUntranslatedIp(dstCH),
                                      CallFceUntranslatedPort(dstCH),
                                      CallFceNatDstIp(srcCH),
                                      CallFceNatDstPort(srcCH),
                                      CallFceNatDstIp(dstCH),
                                      CallFceNatDstPort(dstCH),
                                      dri->mPoolId,
                                      sri->mPoolId,
                                      "rtp",
                                      dtmf_detect,
                                      dtmf_detect_param,
                                      CallDstSym(srcCH),
                                      BridgeMFCPCallback,
                                      evtPtr);
	}
	else
	{
      retval = FCEModifyResourceAsync(CallFceSession(dstCH),
                                      CallFceBundleId(dstCH),
                                      CallFceResourceId(dstCH),
                                      CallFceResourceId(srcCH),
                                      FCE_ANY_IP,
                                      FCE_ANY_PORT,
                                      CallFceUntranslatedIp(dstCH),
                                      CallFceUntranslatedPort(dstCH),
                                      0,
                                      0,
                                      0,
                                      0,
                                      dri->mPoolId,
                                      sri->mPoolId,
                                      "rtp",
                                      dtmf_detect,
                                      dtmf_detect_param,
                                      CallDstSym(srcCH),
                                      BridgeMFCPCallback,
                                      evtPtr);
	}
      if (retval != MFCP_RET_OK) {
        NETERROR(MFCE, 
          ("%s: unable to %s hole for %s, mfcp request failed: %d\n", fn, opstr, eventType, retval));
		status.rval = FCE_FAILURE;
        status.err = FCE_GENERAL_FAILURE;
      }
	  break;

  }

  if (status.rval == FCE_INPROGRESS)
  {
	// This thread shouldn't use evtPtr
    NETDEBUG(MFCE, NETLOG_DEBUG2, 
      ("%s: queued up to %s media hole for %s (%lu:%s:%d-->%lu:%s:%d)\n", \
	   fn, opstr, eventType, \
	   dri->realmId, FormatIpAddress(dri->rsa, ipstr1), dri->mPoolId,\
	   sri->realmId, FormatIpAddress(sri->rsa, ipstr2), sri->mPoolId));
  }

  return (status);
}

/*
 * UpdateFCEInfo return 0 if success, -ve if failure
 * should check that callHandle1, callHandle2, confHandle are not null and fce has returned
 * FCE_SUCCESS or higher code before calling this function
 */
int
UpdateFCEInfo(
		SCC_EventBlock *evtPtr,
		CallHandle *callHandle1,
		CallHandle *callHandle2,
		ConfHandle *confHandle)
{
	char fn[80] = "UpdateFCEInfo:";
	char callIDStr1[CALL_ID_LEN], confIDStr[CONF_ID_LEN], callIDStr2[CALL_ID_LEN];
	CallHandle *srcCH, *dstCH = callHandle2, *oldsrcCH;
	SipAppCallHandle *pSipData;
	H323EventData *pH323Data;
	RTPSet *localSet;
	int nlocalset, count, error = 0;
	int nat_src_allocated = 0;
	int ip,port;

	// FCE functionality has been executed
	// We must jump to leg2 processing code directly
	NETDEBUG(MBRIDGE,NETLOG_DEBUG1,
		("%s: Return from FCE code, sucess scenario, resuming leg2 processing for %s\n",
		fn, (char*) CallID2String(evtPtr->callID,callIDStr1)));

	if (evtPtr->callDetails.relatedCallID == NULL) {
		srcCH = callHandle1;
	}
	else {
		srcCH = CacheGet(callCache, evtPtr->callDetails.relatedCallID);
		if (!srcCH) {
			NETERROR(MFCE, 
					("Fce returned request for a missing call leg CallID %s\n",
					(char*) CallID2String(evtPtr->callDetails.relatedCallID, callIDStr1)));
			error = 1;
		}
		free(evtPtr->callDetails.relatedCallID);
		evtPtr->callDetails.relatedCallID = NULL;
		if (error) {
		  return 0;
		}
	}

	if (evtPtr->callDetails.callType == CAP_SIP) {
		pSipData = (SipAppCallHandle *)evtPtr->data;
		localSet = pSipData->localSet;
		nlocalset = pSipData->nlocalset;
	}
	else {
		pH323Data = (H323EventData *)evtPtr->data;
		localSet = pH323Data->localSet;
		nlocalset = pH323Data->nlocalset;
	}

	/* Update Information in the src Handle and dst Handle */
	if (evtPtr->callDetails.fceStatus.rval == FCE_SUCCESS) {

		switch (evtPtr->callDetails.fceStatus.op) { 

			case NOP 	:
				break;

			case OPEN 	:
				if (CallFceResourceId(dstCH)) {
					// got a new resource id, when an old one already exists
						NETERROR(MBRIDGE, 
				  		("Illegal attempt to replace resource %d with resource %d\n", \
				   		CallFceResourceId(dstCH), evtPtr->callDetails.fceResource));
				}

   				CallFceResourceId(dstCH) = evtPtr->callDetails.fceResource;

				// the fce call allocated the dstCH resource
				CallFceRxPortUsed(dstCH);
   				CallFceNatDstIp(dstCH) = evtPtr->callDetails.fceDstIp;
   				CallFceNatDstPort(dstCH) = evtPtr->callDetails.fceDstPort;
   				CallFceNatSrcIp(dstCH) = evtPtr->callDetails.fceSrcIp;
   				CallFceNatSrcPort(dstCH) = evtPtr->callDetails.fceSrcPort;
				CallFceNatSrcPool(dstCH) = srcCH->realmInfo->mPoolId;
				memcpy(CallFceSrcCallID(dstCH), srcCH->callID, CALL_ID_LEN);

				// the fce call allocated the srcCH resource
				CallFceTxPortUsed(srcCH);
				if(IS_FW_SYMMETRIC)
				{
   					CallFceNatDstIp(srcCH) = evtPtr->callDetails.fceSrcIp;
   					CallFceNatDstPort(srcCH) = evtPtr->callDetails.fceSrcPort;
				}

				// Insert this call Handle into the tipCache
				if (CacheInsert(tipCache, dstCH) < 0) {
					NETERROR(MBRIDGE, ("%s Unable to insert %lx:%d into tipCache\n", 
						fn, CallFceTranslatedIp(dstCH), CallFceTranslatedPort(dstCH)));
				}

				break;

			case MODIFY :
				if (CallFceResourceId(dstCH) != evtPtr->callDetails.fceResource) {
					// got a new resource id, when an old one already exists
						NETERROR(MBRIDGE, 
				  		("%s attempt to replace resource %d with resource %d\n", \
				   		fn, CallFceResourceId(dstCH), evtPtr->callDetails.fceResource));
				}
   				CallFceResourceId(dstCH) = evtPtr->callDetails.fceResource;

				// Modify should not modify the translated address
				if ( ( CallFceNatDstIp(dstCH) || CallFceNatDstPort(dstCH) ) && \
					 ( ( CallFceNatDstIp(dstCH) != evtPtr->callDetails.fceDstIp) || \
					   ( CallFceNatDstPort(dstCH) != evtPtr->callDetails.fceDstPort) ) ) {
					NETERROR(MBRIDGE, 
				  		("%s resource %d changed nat dst\n", \
				   		fn, evtPtr->callDetails.fceResource));
				}

				if ( ( CallFceNatSrcIp(dstCH) || CallFceNatSrcPort(dstCH) ) && \
				     ( ( CallFceNatSrcIp(dstCH) != evtPtr->callDetails.fceSrcIp) || \
					   ( CallFceNatSrcPort(dstCH) != evtPtr->callDetails.fceSrcPort) ) ) {
					// The nat src port got changed
					oldsrcCH = CacheGet(callCache, CallFceSrcCallID(dstCH));
					if (!oldsrcCH) {
						NETERROR(MFCE, 
						("Unable to find old nat src leg CallID %s\n",
						(char*) CallID2String(CallFceSrcCallID(dstCH), callIDStr1)));
					}
					else {
				  		// free the currently allocated nat_src
						CallFceTxPortFreed(oldsrcCH);
						if (CallFceRefCount(oldsrcCH) == 0) {
							// clean the address stored
   							CallFceNatDstIp(oldsrcCH) = 0;
   							CallFceNatDstPort(oldsrcCH) = 0;
						}
						CallFceNatSrcIp(dstCH) = 0;
						CallFceNatSrcPort(dstCH) = 0;
						CallFceNatSrcPool(dstCH) = 0;
						memset(CallFceSrcCallID(dstCH), 0, CALL_ID_LEN);
						nat_src_allocated = 1;
					}
				}

				memcpy(CallFceSrcCallID(dstCH), srcCH->callID, CALL_ID_LEN);
   				CallFceNatSrcIp(dstCH) = evtPtr->callDetails.fceSrcIp;
   				CallFceNatSrcPort(dstCH) = evtPtr->callDetails.fceSrcPort;
				CallFceNatSrcPool(dstCH) = srcCH->realmInfo->mPoolId;
				if (nat_src_allocated) {
					CallFceTxPortUsed(srcCH);
					if (!(CallFceNatSrcIp(srcCH) || CallFceNatSrcPort(srcCH))) {
						if(IS_FW_SYMMETRIC)
						{
   							CallFceNatDstIp(srcCH) = evtPtr->callDetails.fceSrcIp;
   							CallFceNatDstPort(srcCH) = evtPtr->callDetails.fceSrcPort;
						}
					}
				}
				break;

			case CLOSE :

				if (CallFceResourceId(dstCH)) {
					// the fce call allocated the dstCH resource
					CallFceRxPortFreed(dstCH);
					if (CallFceRefCount(dstCH) == 0) {
						// clean the address stored
   						CallFceNatDstIp(dstCH) = 0;
   						CallFceNatDstPort(dstCH) = 0;
					}

					// the fce call allocated the srcCH resource
					CallFceTxPortFreed(srcCH);
					if (CallFceRefCount(srcCH) == 0) {
						// clean the address stored
   						CallFceNatDstIp(srcCH) = 0;
   						CallFceNatDstPort(srcCH) = 0;
					}
   					CallFceNatSrcIp(dstCH) = 0;
   					CallFceNatSrcPort(dstCH) = 0;
					CallFceNatSrcPool(dstCH) = 0;
					memset(CallFceSrcCallID(dstCH), 0, CALL_ID_LEN);

					CallFceResourceId(dstCH) = 0;
					CallFceBundleId(dstCH) = 0;
					CallFceSession(dstCH) = 0;
				}

				break;

			default		:
				NETERROR(MBRIDGE, ("fce returned with unknown operation.\n"));
				break;
		}

		// Mark fceResource as 0 to denote that the fce info returned has been processed
   		evtPtr->callDetails.fceResource = 0;
	}
	
	if (CallFceResourceId(dstCH)) {
	  	ip = CallFceTranslatedIp(dstCH);
	  	port = CallFceTranslatedPort(dstCH);
	}
	else {
	  	ip = CallFceUntranslatedIp(dstCH);
	  	port = CallFceUntranslatedPort(dstCH);
	}

	if (localSet[0].rtpaddr == 0)
	  ip = 0;

	for (count = 0; count < nlocalset; count++) {
		localSet[count].rtpaddr = ip;
		localSet[count].rtpport = port;
	}
			
	// Reset the dtmf transcoding flag
	srcCH->dtmf_detect = 0;

	return 0;
}
