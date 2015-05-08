#include "gis.h"
#include "iwfsm.h"
#include "sipcall.h"
#include "mealeysm.h"
#include "uh323inc.h"
#include "usipinc.h"

#include "log.h"
#include "nxosd.h"
#include <malloc.h>
#include <strings.h>

#include "iwfutils.h"
#include "bridge.h"
#include "iwfsmproto.h"
#include "gk.h"
#include "firewallcontrol.h"

int reInviteClc = 1;
int replacementFor = 0;

/*	Convert to SipData Format
	Find SipCallHandle 
	Send an event to it
*/

int iwfInitiateInvite(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateInvite";
	H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	header_url			*pCallingpn,*pCalledpn,*pRequri;
	char 				ipstr[24];
	char 				rsastr[24];
	SipAppCallHandle	*pSipData = NULL;
	char				h323CallID[CALL_ID_LEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char				evtStr[80];
	CallHandle			callHandleBlk = {0};
	CacheRealmEntry		*realmInfo = NULL;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	pCallingpn = (header_url *) malloc(sizeof(header_url));
	memset(pCallingpn, 0, sizeof(header_url));

 	if(strlen(pH323Data->callingpn))
 	{
 		pCallingpn->name = strdup(pH323Data->callingpn);
 	}
 	else 
	{
 		pCallingpn->name = strdup("Anonymous");
 	}

	if (pH323Data->displaylen)
	{
		if (pH323Data->display[0] != '"')
		{
			pCallingpn->display_name = malloc(pH323Data->displaylen + 4);
			pCallingpn->display_name[0] = '"';
			strncpy(&pCallingpn->display_name[1], pH323Data->display, pH323Data->displaylen); 
			pCallingpn->display_name[pH323Data->displaylen+1] = '"';
			pCallingpn->display_name[pH323Data->displaylen+2] = '\0';
		}
		else
		{
		   	pCallingpn->display_name = strdup(pH323Data->display);
		}
	}

	pCallingpn->host = 
		strdup(FormatIpAddress(pH323Data->srcsigip, ipstr));
	pCallingpn->port = SIP_PORT; 

	pCalledpn = (header_url *)malloc (sizeof(header_url));
	memset(pCalledpn, 0, sizeof(header_url));

	pCalledpn->name = strdup(pH323Data->calledpn);
	CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);
	realmInfo = CacheGet(realmCache, &pH323Data->requri->realmId);
	CacheReleaseLocks(realmCache);
	pCalledpn->host = strdup(FormatIpAddress(realmInfo->realm.rsa, rsastr));
	pCalledpn->port = pH323Data->destport; 

	pSipData->requri = pH323Data->requri;
	pH323Data->requri = NULL;
	pSipData->calledpn = pCalledpn;
	pSipData->callingpn = pCallingpn;

	pSipData->dipaddr = pH323Data->destip;
	pSipData->dipport = pH323Data->destport;
	if(pH323Data->nlocalset >0)
	{
		/* Allocate extra space for 2833/dtmf */
		int nset = pSipData->nlocalset = pH323Data->nlocalset;

		if(always2833)
		{
			nset = pSipData->nlocalset += 1;
		}

		pSipData->localSet = (RTPSet *)(calloc(pSipData->nlocalset, sizeof(RTPSet)));
		memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));

		if(always2833)
		{
			pSipData->localSet[nset-1].codecType = 101;
			pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
			pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
			pSipData->attr_count = 2;
			pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
			SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
			SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
		}
	
		 getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
	
		 // SONUS specific

         if (CacheFind(callCache, evtPtr->callID,&callHandleBlk,sizeof(CallHandle))< 0)
         {
         	NETERROR(MSCC,
            	("%s Unable to locate sipCallHandle.Dropping Call\n",fn));
            iwfSendRelComp(evtPtr->confID,h323CallID);
            goto _return;
         }

		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %d\n",
                fn, callHandleBlk.vendor));

		sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, callHandleBlk.vendor);
	}
	else {
		CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
		if ((confHandle = CacheGet(confCache,evtPtr->confID)))
		{
			confHandle->subState = media_sNotConnected;
		} 
		else {
			CacheReleaseLocks(confCache);
			NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
			// can't do anything
			SipFreeAppCallHandle(pSipData);
			goto _return;
		}
		CacheReleaseLocks(confCache);

		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

		// SONUS specific 

		if (CacheFind(callCache, evtPtr->callID,&callHandleBlk,sizeof(CallHandle))< 0)
		{
			NETERROR(MSCC,
				("%s Unable to locate sipCallHandle.Dropping Call\n",fn));
			SipFreeAppCallHandle(pSipData);
			iwfSendRelComp(evtPtr->confID,h323CallID);
			goto _return;
		}

		NETDEBUG(MIWF,NETLOG_DEBUG4,
			("%s Setup Non FS. vendor =%d\n", fn,callHandleBlk.vendor));

		if (!VendorSupportsFirstInviteNoSDP(callHandleBlk.vendor))
		{
				/* Allocate extra space for 2833/dtmf */
				int nset;
				nset = pSipData->nlocalset = 3;
				pSipData->localSet = (RTPSet *)(calloc(pSipData->nlocalset,sizeof(RTPSet)));
				memset(pSipData->localSet,0,nset*sizeof(RTPSet));
				pSipData->localSet[0].codecType = CodecGPCMU;
				pSipData->localSet[1].codecType = CodecGPCMA;
				pSipData->localSet[2].codecType = CodecG729;
				sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, callHandleBlk.vendor);
				NETDEBUG(MIWF,NETLOG_DEBUG4,
					("%s Setup Non FS. Destination is Sonus %s\n",
					fn,CallID2String(evtPtr->callID,callIDStr)));
		}
	}

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));
	pSipEvt->event = Sip_eBridgeInvite;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CALL_ID_LEN);

	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;

		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}
	
_return:
	H323FreeEvent(evtPtr);
	return 0;
}

/*	Forward RTPSet (if any) to Sip side  	
*
*/
int iwfAlertingTo1xx(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfAlertingTo1xx";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	// Send a 183 only if alerting has media, has PI and 
	// has progress descriptor with value 1 or 8
	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s : nlocalset = %d, pi_IE = %d, progress = %d\n",
		fn,pH323Data->nlocalset,pH323Data->pi_IE,pH323Data->progress));

	if((pH323Data->nlocalset > 0) && (pH323Data->pi_IE > 0) && 
		((pH323Data->progress == 1) || (pH323Data->progress == 8))) 
	{
		int nset = pSipData->nlocalset = pH323Data->nlocalset;

		if(always2833)
		{
			nset = pSipData->nlocalset += 1;
		}

		pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
		memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
		sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, Vendor_eGeneric);

		pSipData->responseCode = 183;
		if(always2833)
		{
			pSipData->localSet[nset-1].codecType = 101;
			pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
			pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
			pSipData->attr_count = 2;
			pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
			SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
			SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
		}
	}

	pSipEvt->event = Sip_eBridge1xx;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;
		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}

/*	Forward RTPSet (if any) to Sip side  	
*
*/
int iwfProgressTo1xx(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfAlertingTo1xx";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	if(pH323Data->nlocalset >0) 
	{
		int nset = pSipData->nlocalset = pH323Data->nlocalset;

		if(always2833)
		{
			nset = pSipData->nlocalset += 1;
		}

		pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
		memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
		sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, Vendor_eGeneric);

		pSipData->responseCode = 183;
		if(always2833)
		{
			pSipData->localSet[nset-1].codecType = 101;
			pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
			pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
			pSipData->attr_count = 2;
			pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
			SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
			SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
		}
	}

	pSipEvt->event = Sip_eBridge1xx;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;
		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}
/*	Forward only if there is RTPSet to Sip side  	
*
*/
int iwfProceedingTo1xx(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfProceedingTo1xx";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	int 				nset;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset <= 0) 
	{
		goto _return;
	}

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	nset = pSipData->nlocalset = pH323Data->nlocalset;

	if(always2833)
	{
		nset = pSipData->nlocalset += 1;
	}

	pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
	memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, Vendor_eGeneric);

	pSipData->responseCode = 183;
	if(always2833)
	{
		pSipData->localSet[nset-1].codecType = 101;
		pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
		pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
		pSipData->attr_count = 2;
		pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
		SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
		SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
	}

	pSipEvt->event = Sip_eBridge1xx;
	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;
		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}
	
_return:
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}

int iwfConnectTo200Ok(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfConnectTo200Ok";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	int					nset;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset <= 0) 
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Rtp not Set.Not sending 200 Ok\n",fn));

		CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
		if ((confHandle = CacheGet(confCache,evtPtr->confID)))
		{
			confHandle->subState = media_sNotConnected;
		} 
		else {
			NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
		}
		CacheReleaseLocks(confCache);
		goto _return;
	}

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	nset = pSipData->nlocalset = pH323Data->nlocalset;

	if(always2833)
	{
		nset = pSipData->nlocalset += 1;
	}

	pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
	memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, Vendor_eGeneric);

	if(always2833)
	{
		pSipData->localSet[nset-1].codecType = 101;
		pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
		pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
		pSipData->attr_count = 2;
		pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
		SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
		SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
	}

	pSipEvt->event = Sip_eBridge200;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;
		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}

_return:	
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
}

			
int iwfCallConnectedTCS(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfCallConnectedTCS";
    H323EventData       *pH323Data, *pNextEvData;
	SCC_EventBlock		*pNextEvt;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	SipCallHandle   	*sipCallHandle = NULL;
	CallHandle			*callHandle, *h323callHandle;
	int					i,j;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	int					subState;
	RTPSet				*localSet = NULL;
	int					codecMatch = 0;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset <= 0) 
	{
		NETERROR(MIWF,("%s Null TCS !! Not being handled\n",fn));
		goto _return;
	}

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if ((confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		subState = confHandle->subState;
	} 
	else {
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	CacheReleaseLocks(confCache);

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	if (!(callHandle = CacheGet(callCache, h323CallID)))
	{
		// Error
		NETERROR(MSCC,("%s Unable to locate h323CallHandle\n",fn));
		CacheReleaseLocks(callCache);
		goto _return;
	}

	h323callHandle = callHandle;

	// Previously check for media_sNotConnected
	if (callHandle->fastStartStatus == 0)
	{
		/* Find the matching codec */
		if (!(callHandle = CacheGet(callCache, evtPtr->callID)))
		{
			NETERROR(MSCC,("%s Unable to locate sipCallHandle\n",fn));
			CacheReleaseLocks(callCache);
			goto _return;
			/* drop call here */
		}
		sipCallHandle = SipCallHandle(callHandle);

		/* choose codec */
		for(i = 0; !codecMatch && i < sipCallHandle->localSet.localSet_len; ++i)
		{
			NETDEBUG(MIWF,NETLOG_DEBUG4,("%s codec = %d.\n",
				fn,sipCallHandle->localSet.localSet_val[i].codecType));

			for(j = 0; j<pH323Data->nlocalset;++j)
			{
				if(sipCallHandle->localSet.localSet_val[i].codecType == pH323Data->localSet[j].codecType)
				{
					localSet = (RTPSet *)malloc(sizeof(RTPSet));
					//*localSet = pH323Data->localSet[j];
					*localSet = sipCallHandle->localSet.localSet_val[i];

					if(CallFceTranslatedIpPort(h323callHandle))
					{
						localSet->rtpaddr = CallFceTranslatedIp(h323callHandle);
						localSet->rtpport = CallFceTranslatedPort(h323callHandle);
						NETDEBUG(MIWF,NETLOG_DEBUG4,
							("%s Mediarouting is enabled on Sip Side\n", fn));
					}

					codecMatch = 1;
					break;
				}
			}
		}
		
		CacheReleaseLocks(callCache);
		if(!localSet)
		{
			/* Should send OlcReject here */
			NETERROR(MSCC,("%s No Matching codec\n",fn));
			goto _return;
		}
		setDefaultRtpParams(localSet,1);
		/* Send OLC to H323 */
		pNextEvt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
		memset(pNextEvt,0,sizeof(SCC_EventBlock));

		pNextEvData = (H323EventData *) malloc(sizeof(H323EventData));
		memset(pNextEvData,0,sizeof(H323EventData));

		memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);
		memcpy(pNextEvt->callID,h323CallID,CALL_ID_LEN);
		memcpy(pNextEvData->callID,h323CallID,CALL_ID_LEN);
		pNextEvt->data = pNextEvData;
		pNextEvData->localSet = localSet;
		pNextEvData->nlocalset = 1;

		pNextEvt->event = SCC_eBridgeOLC;

		if(iwfSendH323Event(pNextEvt) !=0)
		{
			NETERROR(MIWF,
				("%s iwfSendH323Event Failed. Could not send OLC\n",fn));
		}
	}
	else
	{
		CacheReleaseLocks(callCache);
	}

_return:	
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
}

int iwfCallConnectedTxConnected(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfCallConnectedTxConnected";
    H323EventData       *pH323Data, *pNextEvData;
	SCC_EventBlock		*pNextEvt;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	SipCallHandle   	*sipCallHandle = NULL;
	CallHandle			*callHandle, *h323callHandle;
	int					i,nset;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	int					subState;
	int					codecMatch = 0;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if ((confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		subState = confHandle->subState;
	} 
	else {
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	CacheReleaseLocks(confCache);

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	if(subState == media_sNotConnected)
	{
		/* This case arises when we get 200 OK sdp for invite null sdp */
		/* if we have our capability send it */
		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
		if (!(h323callHandle = CacheGet(callCache, h323CallID)))
		{
			NETERROR(MSCC,("%s Unable to locate h323CallHandle\n",fn));
			CacheReleaseLocks(callCache);
			goto _return;
			/* drop call here */
		}
		if (!(callHandle = CacheGet(callCache, evtPtr->callID)))
		{
			NETERROR(MSCC,("%s Unable to locate sipCallHandle\n",fn));
			CacheReleaseLocks(callCache);
			goto _return;
			/* drop call here */
		}
		sipCallHandle = SipCallHandle(callHandle);
		nset = sipCallHandle->localSet.localSet_len;

		if(nset)
		{
			pNextEvt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
			memset(pNextEvt,0,sizeof(SCC_EventBlock));

			pNextEvData = (H323EventData *) malloc(sizeof(H323EventData));
			memset(pNextEvData,0,sizeof(H323EventData));

			memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);
			memcpy(pNextEvt->callID,h323CallID,CALL_ID_LEN);
			memcpy(pNextEvData->callID,h323CallID,CALL_ID_LEN);
			pNextEvt->data = pNextEvData;
			pNextEvt->event = SCC_eBridgeCapabilities;
			pNextEvData->localSet = (RTPSet *)calloc(nset,sizeof(RTPSet));
			pNextEvData->nlocalset = nset;
			memcpy(pNextEvData->localSet,sipCallHandle->localSet.localSet_val,nset*sizeof(RTPSet));

			if(CallFceTranslatedIpPort(h323callHandle))
			{
				for(i =0; i<nset;++i)
				{
					pNextEvData->localSet[i].rtpaddr = CallFceTranslatedIp(h323callHandle);
					pNextEvData->localSet[i].rtpport = CallFceTranslatedPort(h323callHandle);
					NETDEBUG(MIWF,NETLOG_DEBUG4,
						("%s Mediarouting is enabled on Sip Side\n", fn));
				}
			}

			CacheReleaseLocks(callCache);
			setDefaultRtpParams(pNextEvData->localSet,nset);
			NETDEBUG(MIWF,NETLOG_DEBUG4,
				("%s Sending Bridge Capability\n",fn));
			if(iwfSendH323Event(pNextEvt) !=0)
			{
				NETERROR(MIWF,
					("%s iwfSendH323Event Failed. Could not send OLC\n",fn));
			}
		}
	}

_return:	
	H323FreeEvent(evtPtr);
	return 0;
}

/*
 * Called when OLC ACK is received in Connected state.
 * If SIP invite had no SDP, and OLC was received from
 * Cisco with media port 4000, send 200Ok to SIP with
 * media port received in OLC ACK.
 */
int iwfChanConnectTo200Ok(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfChanConnectTo200Ok";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char                confIDStr[CONF_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	int					nset;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	
	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset <= 0) 
	{
		NETERROR(MIWF,("%s Rtp not Set. Dropping Call!\n",fn));
		CacheReleaseLocks(confCache);
		goto _dropCall;
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Changing mediaState from %d to %d \n",
		fn,confHandle->subState,media_sConnected));

	confHandle->subState = media_sConnected;

	/* If Invite had no SDP */
	if (confHandle->inviteNoSDP == 1)
	{
		/* If OLC was received from Cisco with media port 4000,
		   OLC ACK should contain actual media port */
		if (confHandle->olcFromCisco == 1)
		{
			/* Send media port received in OLC ACK to SIP in 200Ok */
			if (iwfSend200WithSDP(evtPtr->confID, evtPtr->callID, 
				pH323Data->localSet, pH323Data->nlocalset) < 0)
			{
				CacheReleaseLocks(confCache);
				goto _dropCall;
			}
		}
		else
		{
			confHandle->inviteNoSDP = 0;
			confHandle->olcFromCisco = 0;
		}
		CacheReleaseLocks(confCache);
		goto _return;
	}

	CacheReleaseLocks(confCache);

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	nset = pSipData->nlocalset = pH323Data->nlocalset;

	if(always2833)
	{
		nset = pSipData->nlocalset += 1;
	}

	pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
	memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, Vendor_eGeneric);

	if(always2833)
	{
		pSipData->localSet[nset-1].codecType = 101;
		pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
		pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
		pSipData->attr_count = 2;
		pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
		SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
		SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
	}

	pSipEvt->event = Sip_eBridge200;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
		goto _dropCall;
	}
	
_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;

_dropCall:
	/* Send a release complete to leg1 */
	evtPtr->event = SCC_eBridgeReleaseComp;
	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
	memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
	H323FreeEvData(evtPtr->data);
	evtPtr->data = 0;
	/* Do not try to free evtPtr - we are passing its ownership */
	if(iwfSendH323Event(evtPtr)!=0)
	{
		// Too Bad - can't do anything else
		NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
	}
	return 0;
}


int iwfChanConnectToAck(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfChanConnectToAck";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	header_url			*pRequri;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char                confIDStr[CONF_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	int					nset;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	
	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset <= 0) 
	{
		NETERROR(MIWF,("%s Rtp not Set. Dropping Call!\n",fn));
		CacheReleaseLocks(confCache);
		goto _dropCall;
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Changing mediaState from %d to %d \n",
		fn,confHandle->subState,media_sConnected));

	if (confHandle->subState == media_sConnected)
	{
		CacheReleaseLocks(confCache);
		goto _return;
	}

	confHandle->subState = media_sConnected;
	CacheReleaseLocks(confCache);

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	nset = pSipData->nlocalset = pH323Data->nlocalset;

	if(always2833)
	{
		nset = pSipData->nlocalset += 1;
	}

	pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
	memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, Vendor_eGeneric);

	if(always2833)
	{
		pSipData->localSet[nset-1].codecType = 101;
		pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
		pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
		pSipData->attr_count = 2;
		pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
		SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
		SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
	}

	pRequri = (header_url *)malloc (sizeof(header_url));
	memset(pRequri, 0, sizeof(header_url));

	pRequri->name = strdup(pH323Data->calledpn);
	pRequri->host = strdup(FormatIpAddress(pH323Data->destip, ipstr));
	pRequri->port = pH323Data->destport; 

	pSipData->requri = pRequri;

	pSipEvt->event = Sip_eBridgeAck;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;
		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}
	
_return:
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;

_dropCall:
	/* Send a release complete to leg1 */
	evtPtr->event = SCC_eBridgeReleaseComp;
	/* Do not try to free evtPtr - we are passing its ownership */
	if(iwfSendH323Event(evtPtr)!=0)
	{
		// Too Bad - can't do anything else
		NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
	}
	return 0;

}

/*	Called when we receive a release complete from H323 side
* 	when we are in WaitOnSip State	
*/
int iwfInitiateCancel(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateCancel";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	header_url			*pRequri;
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	memcpy(&pSipEvt->callDetails, &evtPtr->callDetails, sizeof(CallDetails));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	pRequri = (header_url *)malloc (sizeof(header_url));
	memset(pRequri, 0, sizeof(header_url));

	pRequri->name = strdup(pH323Data->calledpn);
	pRequri->host = strdup(FormatIpAddress(pH323Data->destip, ipstr));
	pRequri->port = pH323Data->destport; 

	pSipData->requri = pRequri;
	pSipEvt->event = Sip_eBridgeCancel;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		// h323 call has already been dropped 
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
	}
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}


/*	
*	Called when we receive a release complete from H323 side
*/
int iwfInitiateBye(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateBye";
    H323EventData       *pH323Data;
	char				sipCallID[CALL_ID_LEN];
	SCC_EventBlock		*pSipEvt;
	header_url			*pRequri;
	SipAppCallHandle	*pSipData;
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	memcpy(&pSipEvt->callDetails, &evtPtr->callDetails, sizeof(CallDetails));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	pRequri = (header_url *)malloc (sizeof(header_url));
	memset(pRequri, 0, sizeof(header_url));

	pRequri->name = strdup(pH323Data->calledpn);
	pRequri->host = strdup(FormatIpAddress(pH323Data->destip, ipstr));
	pRequri->port = pH323Data->destport; 

	pSipData->requri = pRequri;
	pSipEvt->event = Sip_eBridgeBye;
	pSipEvt->data = (void *)pSipData;

	memcpy(pSipEvt->confID,evtPtr->confID,CALL_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		// can't do anything else H323 Call has already been freed
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

	}
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}

// Called in FS200OK state we have received 200OK but not sent ack 
int iwfInitiateAckBye(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateAckBye";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	header_url			*pRequri;
	SipAppCallHandle	*pSipData;
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	RTPSet 				localSet = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;


	localSet.rtpport == 10000;

	if(iwfSendAck(evtPtr->confID,evtPtr->callID,&localSet,0) <0)
	{
		NETERROR(MIWF,("%s iwfSendAck Failed\n",fn));
	}

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	memcpy(&pSipEvt->callDetails, &evtPtr->callDetails, sizeof(CallDetails));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	pRequri = (header_url *)malloc (sizeof(header_url));
	memset(pRequri, 0, sizeof(header_url));

	pRequri->name = strdup(pH323Data->calledpn);
	pRequri->host = strdup(FormatIpAddress(pH323Data->destip, ipstr));
	pRequri->port = pH323Data->destport; 

	pSipData->requri = pRequri;
	pSipEvt->event = Sip_eBridgeBye;
	pSipEvt->data = (void *)pSipData;

	memcpy(pSipEvt->confID,evtPtr->confID,CALL_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		// can't do anything else H323 Call has already been freed
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

	}
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}

/* called when we receive release complete in waitOnH323 state */
int iwfInitiateFinalResponse(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateFinalResponse";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	header_url			*pRequri;
	SipAppCallHandle	*pSipData;
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	memcpy(&pSipEvt->callDetails, &evtPtr->callDetails, sizeof(CallDetails));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pRequri = (header_url *)malloc (sizeof(header_url));
	memset(pRequri, 0, sizeof(header_url));

	pRequri->name = strdup(pH323Data->calledpn);
	pRequri->host = strdup(FormatIpAddress(pH323Data->destip, ipstr));
	pRequri->port = pH323Data->destport; 

	pSipData->requri = pRequri;
	pSipEvt->callDetails.responseCode = pSipData->responseCode = 
			h3232SipRespCode(evtPtr->callDetails.cause, evtPtr->callDetails.callError);
	pSipEvt->event = Sip_eBridgeFinalResponse;
	pSipEvt->data = (void *)pSipData;

	memcpy(pSipEvt->confID,evtPtr->confID,CALL_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		// can't do anything else H323 Call has already been freed
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

	}
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}

/* Handle H323 OLC in state HeldBySip
*		Send BridgeChanConnect (OLCACk) 
*/
int
iwfOLC2ChanConnect(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfOLC2ChanConnect";
	SCC_EventBlock		*pSipEvt,*pH323Evt;
	char				h323CallID[CALL_ID_LEN] = {0};
	H323EventData		*pH323Data,*pData;
	char				str[EVENT_STRLEN];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	SipCallHandle   	*sipCallHandle = NULL;
	CallHandle			*callHandle, *h323callHandle;
	int 				i,codecMatch;
	RTPSet 				*localSet = NULL;
	char 				ipstr[24];

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pData = (H323EventData *)evtPtr->data;
	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	if (!(h323callHandle = CacheGet(callCache, h323CallID)))
	{
		NETERROR(MSCC,("%s Unable to locate h323CallHandle.Dropping Call\n",fn));
		iwfSendRelComp(evtPtr->confID,h323CallID);
		CacheReleaseLocks(callCache);
		goto _return;
	}

	if (!(callHandle = CacheGet(callCache, evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate sipCallHandle.Dropping Call\n",fn));
		iwfSendRelComp(evtPtr->confID,h323CallID);
		CacheReleaseLocks(callCache);
		goto _return;
	}

	sipCallHandle = SipCallHandle(callHandle);

	/* choose codec */
	codecMatch = 0;
	for(i = 0;i<sipCallHandle->localSet.localSet_len;++i)
	{
		if(pData->localSet[0].codecType == sipCallHandle->localSet.localSet_val[i].codecType)
		{
			localSet = (RTPSet *)malloc(sizeof(RTPSet));
			*localSet = sipCallHandle->localSet.localSet_val[i];
			localSet->param = pData->localSet[0].param;

			if(CallFceTranslatedIpPort(h323callHandle))
			{
				localSet->rtpaddr = CallFceTranslatedIp(h323callHandle);
				localSet->rtpport = CallFceTranslatedPort(h323callHandle);
				NETDEBUG(MIWF,NETLOG_DEBUG4,
					("%s Mediarouting is enabled on Sip Side\n", fn));
			}

			codecMatch = 1;
			localSet->mLineNo = pData->localSet[0].mLineNo;
			break;
		}
	}

	CacheReleaseLocks(callCache);
	
	if(!codecMatch)
	{
		/* Should send OlcReject here */
		NETERROR(MSCC,("%s No Matching codec to OLC - dropping call\n",fn));
		iwfSendRelComp(evtPtr->confID,h323CallID);
		goto _return;
	}

	NETDEBUG(MSCC,NETLOG_DEBUG4,("%s codec= %d param = %d ip = %s/%d\n",
		fn,localSet->codecType,localSet->param,
		FormatIpAddress(localSet->rtpaddr, ipstr),
		localSet->rtpport));

	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s changing state WaitOnOLCAck,sending OLC \n",fn));

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));
	pH323Data = (H323EventData *)malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));
	pH323Evt->event = SCC_eBridgeChanConnect;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Data->sid = pData->sid;
	pH323Data->localSet = localSet;
	pH323Data->nlocalset = 1;
	pH323Evt->data = pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed.\n",
			fn));
	}

_return:
	H323FreeEvent(evtPtr);
	return 0;
}

/*	
*
*/
int iwfH323LogEvent(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfLogH323Event";
    H323EventData       *pH323Data;
	char				str[EVENT_STRLEN];

	NETDEBUG(MIWF,NETLOG_DEBUG3,("%s Received event = %s\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,str)));

	pH323Data = (H323EventData *) evtPtr->data;
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}

int iwfH323Ignore(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfH323Ignore";
    H323EventData       *pH323Data;
	char				str[EVENT_STRLEN];

	NETDEBUG(MIWF,NETLOG_DEBUG3,("%s Received event = %s\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,str)));

	pH323Data = (H323EventData *) evtPtr->data;

	NETDEBUG(MIWF,NETLOG_DEBUG3,("%s Received event = %d\n",fn,evtPtr->event));
	
	H323FreeEvData(pH323Data);

	free(evtPtr);
	return 0;
}

int iwfReInviteBridgeTCS(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteBridgeTCS";
    H323EventData       *pH323Data,*pNextEvData;
	char				h323CallID[CALL_ID_LEN] = {0};
	CallHandle			callHandleBlk2 = {0};
	SipCallHandle   	*sipCallHandle = NULL;
	RTPSet				*localSet = NULL;
	SCC_EventBlock		*pNextEvt;
	int					i,j,codecMatch;

	pH323Data = (H323EventData *) evtPtr->data;

	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s received TCS .IGNORING \n", fn));
_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

int iwfReInviteTCSAck(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteTCSAck";
    H323EventData       *pH323Data;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	char                stateString[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				h323CallID[CALL_ID_LEN] = {0};
	int					event;
	CallHandle			callHandleBlk = {0},callHandleBlk2 = {0};
	SipCallHandle		*sipCallHandle;
	SCC_EventBlock		*pH323Evt;
	int					i,j,codecMatch;
	RTPSet				*localSet = NULL;
	char 				ipstr[24];
	int					oldstate;
	char				evtStr[80];


	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s received TCSAck.\n", fn));

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}

	oldstate = confHandle->state;
	/* Change State */
	switch(confHandle->subState)
	{
		case RI_sWONullTCSAck:
			confHandle->subState = RI_sWOTCSAck;
			CacheReleaseLocks(confCache);
			event = SCC_eBridgeCapabilities;
			NETDEBUG(MIWF,NETLOG_DEBUG4,
				("%s changing state WaitOnTCSAck,sending TCS\n",fn));
			break;

		case RI_sWOTCSAck:
			confHandle->subState = RI_sWOOLCAck;
			CacheReleaseLocks(confCache);
			if(reInviteClc || replacementFor)
			{
				event = SCC_eBridgeOLC;
				if (CacheFind(callCache, evtPtr->callID,&callHandleBlk,sizeof(CallHandle))< 0)
				{
					NETERROR(MSCC,("%s Unable to locate sipCallHandle\n",fn));
					goto _return;
					/* drop call here */
				}

				sipCallHandle = SipCallHandle(&callHandleBlk);
				if (CacheFind(callCache, h323CallID,&callHandleBlk2,sizeof(CallHandle))< 0)
				{
					NETERROR(MSCC,("%s Unable to locate H323CallHandle\n",fn));
					goto _return;
					/* drop call here */
				}

				/* choose codec */
				codecMatch = 0;
				for(i = 0;i < H323nlocalset(&callHandleBlk2) && !codecMatch; ++i)
				{
					for(j = 0;j<sipCallHandle->localSet.localSet_len;++j)
					{
						if(H323localSet(&callHandleBlk2)[i].codecType == sipCallHandle->localSet.localSet_val[j].codecType)
						{
							localSet = (RTPSet *)malloc(sizeof(RTPSet));
							*localSet = sipCallHandle->localSet.localSet_val[j];
							localSet->param = H323localSet(&callHandleBlk2)[i].param;

							if(CallFceTranslatedIpPort(&callHandleBlk2))
							{
								localSet->rtpaddr = CallFceTranslatedIp(&callHandleBlk2);
								localSet->rtpport = CallFceTranslatedPort(&callHandleBlk2);
								NETDEBUG(MIWF,NETLOG_DEBUG4,
									("%s Mediarouting is enabled on Sip Side\n", fn));
							}

							codecMatch = 1;
							break;
						}
					}
				}
				
				if(!codecMatch)
				{
					/* Should send OlcReject here */
					NETERROR(MSCC,("%s No Matching codec\n",fn));
					goto _return;
				}

				NETDEBUG(MSCC,NETLOG_DEBUG4,("%s codec= %d param = %d ip = %s/%d\n",
					fn,localSet->codecType,localSet->param,FormatIpAddress(localSet->rtpaddr, ipstr),
					localSet->rtpport));

				if(iwfSendBridgeOLC(evtPtr->confID,h323CallID,localSet,1) !=0)
				{
					NETERROR(MIWF,("%s iwfSendBridgeOLC Failed.\n",fn));
				}

				NETDEBUG(MIWF,NETLOG_DEBUG4,
					("%s changing state WaitOnOLCAck,sending OLC \n",fn));
			}
			goto _return;
			break;

		default:
			NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Ignoring. state %s/%d conf(%s)\n",
				fn,
				(char*) iwfState2Str(confHandle->state,stateString),
				confHandle->subState,
				(char*) ConfID2String(evtPtr->confID,confIDStr)));
			CacheReleaseLocks(confCache);
			goto _return;
			break;
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,evtStr),
		iwfState2Str(oldstate,stateString),
		iwfState2Str(confHandle->state,stateString2)));

	/* Send event to H323 */
	/* get sip CallHandle */
	if (CacheFind(callCache,evtPtr->callID,&callHandleBlk,sizeof(CallHandle))< 0)
	{
		NETERROR(MSCC,("%s Unable to locate SipCallHandle\n",fn));
		/* drop call here */
		goto _return;
	}

    sipCallHandle = SipCallHandle(&callHandleBlk);

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	copySipRtpset2H323Data(pH323Data, sipCallHandle->localSet.localSet_val, NULL, sipCallHandle->localSet.localSet_len);

	setDefaultRtpParams(pH323Data->localSet,pH323Data->nlocalset);
	pH323Evt->event = SCC_eBridgeCapabilities;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,
			("%s iwfSendH323Event Failed.Could not send new TCS\n",fn));
	}


_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

int iwfReInviteReqModeAck(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteReqModeAck";
    H323EventData       *pH323Data;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	char                stateString[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				h323CallID[CALL_ID_LEN] = {0};
	int					event;
	CallHandle			callHandleBlk = {0},callHandleBlk2 = {0};
	SipCallHandle		*sipCallHandle;
	SCC_EventBlock		*pH323Evt;
	RTPSet				*localSet;
	char 				ipstr[24];
	int					oldstate;
	char				evtStr[80];


	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s received ReqModeAck.\n", fn));

	pH323Data = (H323EventData *)evtPtr->data;
	
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}

	oldstate = confHandle->state;

	if (pH323Data->modeStatus != cmReqModeAccept)
	{
		NETERROR(MSCC,("%s H323 rejected reqmode (%s)\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		/* Send Final Response to ReInvite */
		confHandle->state = IWF_sConnected;
		confHandle->subState = 0;
		CacheReleaseLocks(confCache);
		iwfSendFinalResponse(evtPtr->confID,evtPtr->callID);
		goto _return;
	}

	/* Change State */
	confHandle->subState = RI_sWOOLCAck;
	CacheReleaseLocks(confCache);

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	if(iwfSendBridgeCLC(evtPtr->confID,h323CallID,confHandle->h323MediaType) !=0 )
	{
		NETERROR(MIWF,("%s iwfSendBridgeCLC Failed.\n",fn));
	}

	event = SCC_eBridgeOLC;
	if (CacheFind(callCache, evtPtr->callID,&callHandleBlk,sizeof(CallHandle))< 0)
	{
		NETERROR(MSCC,("%s Unable to locate sipCallHandle\n",fn));
		goto _return;
		/* drop call here */
	}

	sipCallHandle = SipCallHandle(&callHandleBlk);
	if (CacheFind(callCache, h323CallID,&callHandleBlk2,sizeof(CallHandle))< 0)
	{
		NETERROR(MSCC,("%s Unable to locate H323CallHandle\n",fn));
		goto _return;
		/* drop call here */
	}

	localSet = (RTPSet *)malloc(sizeof(RTPSet));
	*localSet = sipCallHandle->localSet.localSet_val[0];

	if(CallFceTranslatedIp(&callHandleBlk2))
	{
		localSet->rtpaddr = CallFceTranslatedIp(&callHandleBlk2);
		localSet->rtpport = CallFceTranslatedPort(&callHandleBlk2);
		NETDEBUG(MIWF,NETLOG_DEBUG4,
			("%s Mediarouting is enabled on Sip Side\n",
			fn));
	}

	setDefaultRtpParams(localSet,1);

	NETDEBUG(MSCC,NETLOG_DEBUG4,("%s codec= %d param = %d ip = %s/%d\n",
		fn,localSet->codecType,localSet->param,FormatIpAddress(localSet->rtpaddr, ipstr),
		localSet->rtpport));

	if(iwfSendBridgeOLC(evtPtr->confID,h323CallID,localSet,1) !=0)
	{
		NETERROR(MIWF,("%s iwfSendBridgeOLC Failed.\n",fn));
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4,
				("%s changing state WaitOnOLCAck,sending OLC \n",fn));

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,evtStr),
		iwfState2Str(oldstate,stateString),
		iwfState2Str(confHandle->state,stateString2)));

_return:
	H323FreeEvent(evtPtr);
	return 0;
}

/*
*	Send ReInvite to Sip if Media has changed 
* 	Else move to connected state
*/
int iwfReInviteOLCAck(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteOLCAck";
    H323EventData       *pH323Data;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	char                stateString[IWF_STATE_STR]={0},stateString2[IWF_STATE_STR] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	int					oldstate;
	char				evtStr[80];

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s received OLCAck.\n", fn));

	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	oldstate = confHandle->state;
	confHandle->state = IWF_sConnected;
	confHandle->subState = 0;

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s confID = %s\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,evtStr),
		iwfState2Str(oldstate,stateString),
		iwfState2Str(confHandle->state,stateString2),
		(char*) ConfID2String(evtPtr->confID,confIDStr)));


	if(pH323Data->nlocalset <= 0) 
	{
		// H323 will never send chan connect w/o localset
		NETERROR(MIWF,("%s Rtp not Set. Dropping Call!\n",fn));
		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}

	confHandle->h323MediaType = (pH323Data->localSet[0].codecType == T38Fax) ? 
							CONF_MEDIA_TYPE_DATA : CONF_MEDIA_TYPE_VOICE;

	CacheReleaseLocks(confCache);

	if(iwfSend200OkMedia(evtPtr->confID,evtPtr->callID,pH323Data->localSet,pH323Data->nlocalset)!=0)
	{
		NETERROR(MIWF,("%s iwfSend200OkMedia Failed\n",fn));
		/* Sip callHandle is probably gone. Send release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}

_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

/* 	Send OLCAck (ChanConnect) to H323. Also called in IWF_sConnected
*/
int iwfReInviteOLC(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteOLC";
    H323EventData       *pH323Data;
	char				h323CallID[CALL_ID_LEN] = {0};
	CallHandle			callHandleBlk2 = {0};
	SipCallHandle   	*sipCallHandle = NULL;
	RTPSet				*localSet = NULL;
	int					i;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	char                stateString[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	int					oldstate;
	char				evtStr[80];
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	CallHandle			*callHandle, *tmpcallHandle;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache, evtPtr->confID)))
	{
		NETERROR(MIWF,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		goto _return;
	}

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	/* Get H323 call handle */
    if (!(callHandle = CacheGet(callCache, h323CallID)))
 	{
   		NETERROR(MIWF,("%s Unable to locate H323 CallHandle\n",fn));
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
        goto _return;
   	}

	/* If Invite came in without SDP */
	if (confHandle->inviteNoSDP == 1)
	{
		if(pH323Data->nlocalset <= 0) 
		{
			NETERROR(MIWF,
				("%s Received OLC without RTP set...dropping calls\n",fn));
			iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
			iwfSendRelComp(evtPtr->confID, h323CallID);
			goto _return;
		}
		
		NETDEBUG(MIWF,NETLOG_DEBUG4,
			("%s Received OLC from vendor %s with rtp port %d\n", 
			fn, GetVendorDescription(callHandle->vendor), 
			pH323Data->localSet[0].rtpport));

		/* OLC came from Cisco CM with rtp port 4000 */
    	if ((callHandle->vendor == Vendor_eCisco) &&
        	(pH323Data->localSet[0].rtpport == 4000))
		{
			confHandle->olcFromCisco = 1;

			/* Send OLC to Cisco with rtp port 4000 */
			localSet = (RTPSet *)calloc(1, sizeof(RTPSet));
			*localSet = pH323Data->localSet[0];
			localSet->rtpport = 4000;
			if(iwfSendBridgeOLC(evtPtr->confID,h323CallID,localSet,1) != 0)
			{
				NETERROR(MIWF,("%s iwfSendBridgeOLC failed\n",fn));
				iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
				iwfSendRelComp(evtPtr->confID, h323CallID);
			}
			else
			{
				NETDEBUG(MIWF,NETLOG_DEBUG4,
					("%s Sent OLC to Cisco\n", fn));
			}
		}
		else
		{
			/* Send 200OK containing localset negotiated with H323 */

			/* Get SIP call handle */
			if (!(tmpcallHandle = CacheGet(callCache, evtPtr->callID)))
			{
				NETERROR(MIWF,("%s Unable to locate SipCallHandle\n",fn));
				iwfSendRelComp(evtPtr->confID, h323CallID);
			}
			else
			{
				iwfSend200WithSDP(evtPtr->confID, evtPtr->callID, 
			  		pH323Data->localSet, pH323Data->nlocalset);
			}
		}
		goto _return;
	}

	if(!reInviteClc && !replacementFor)
	{
		oldstate = confHandle->state;
		confHandle->state = IWF_sConnected;
		confHandle->subState = RI_sIdle;
		NETDEBUG(MIWF,NETLOG_DEBUG4, 
			("%s %s/%s--> %s confID = %s\n",
			fn,SCC_BridgeEventToStr(evtPtr->event,evtStr),
			iwfState2Str(oldstate,stateString),
			iwfState2Str(confHandle->state,stateString2),
			(char*) ConfID2String(evtPtr->confID,confIDStr)));
	}

	if (CacheFind(callCache, evtPtr->callID,&callHandleBlk2,sizeof(CallHandle))< 0)
	{
		NETERROR(MIWF,("%s Unable to locate SipCallHandle\n",fn));
		iwfSendRelComp(evtPtr->confID, h323CallID);
		goto _return;
		/* drop call here */
	}

    sipCallHandle = SipCallHandle(&callHandleBlk2);

	/* choose codec */
	for(i = 0;i < sipCallHandle->localSet.localSet_len; ++i)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s codec = %d.\n",
			fn,sipCallHandle->localSet.localSet_val[i].codecType));

		if(sipCallHandle->localSet.localSet_val[i].codecType == pH323Data->localSet[0].codecType)
		{
			localSet = (RTPSet *)malloc(sizeof(RTPSet));
			*localSet = sipCallHandle->localSet.localSet_val[i];

			if(CallFceTranslatedIpPort(callHandle))
			{
				localSet->rtpaddr = CallFceTranslatedIp(callHandle);
				localSet->rtpport = CallFceTranslatedPort(callHandle);
				NETDEBUG(MIWF,NETLOG_DEBUG4,
					("%s Mediarouting is enabled on Sip Side\n", fn));
			}

			break;
		}
	}
	
	if(!localSet)
	{
		/* Should send OlcReject here */
		NETERROR(MIWF,("%s No Matching codec\n",fn));
		goto _return;
	}
		
	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s codec= %d param = %d ip = %s/%d\n",
		fn,localSet->codecType,localSet->param,FormatIpAddress(localSet->rtpaddr, ipstr), localSet->rtpport));

	/* Send OLCAck to H323 */

	if(iwfSendBridgeOLCAck(evtPtr->confID,h323CallID,localSet,1) !=0)
	{
		NETERROR(MIWF,("%s iwfSendBridgeOLCAck Failed.\n",fn));
	}

	if(!reInviteClc && !replacementFor)
	{
		iwfSend200Ok(evtPtr->callID);
	}

_return:
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

/* Send 200Ok to Sip side
*  To be done when we receive ack for null tcs - so as to prevent getting another
*  reinvite b4 H323 has closed channels
*/
int iwfHeldBySipTCSAck(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfHeldBySipTCSAck";
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	char                stateString[IWF_STATE_STR];
	char                callIDStr[CALL_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}

	/* Change State */
	switch(confHandle->subState)
	{
		case RI_sWONullTCSAck:
			confHandle->subState = RI_sIdle;
			CacheReleaseLocks(confCache);
			if(siphold3264)
			{
				iwfSend200OkHold(evtPtr->callID);
			}
			else
			{
				iwfSend200Ok(evtPtr->callID);
			}
			NETDEBUG(MIWF,NETLOG_DEBUG4,
				("%s Received NullTCSAck. changing state RI_sIdle,"
				"sending 200Ok\n",fn));
			goto _return;
			break;

		default:
			NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Ignoring. state %s/%d conf(%s)\n",
				fn,
				(char*) iwfState2Str(confHandle->state,stateString),
				confHandle->subState,
				(char*) ConfID2String(evtPtr->confID,confIDStr)));
			CacheReleaseLocks(confCache);
			goto _return;
			break;
	}
			
_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

/* Execute and pending sip events */
int iwfControlConnected(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfControlConnected";
	SCC_EventBlock		*pSipEvt,*pH323Evt;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char                stateString[IWF_STATE_STR];
	char                callIDStr[CALL_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	pSipEvt=confHandle->sipEvt;
	confHandle->sipEvt = NULL;
	CacheReleaseLocks(confCache);


	if(pSipEvt)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,("%s Forwarding pending Sip Event\n",
			fn));
		if(iwfSipEventProcessor(pSipEvt)<0)
		{
				SipFreeAppCallHandle(pSipEvt->data);
				free(pSipEvt);
		}
	}	

_return:
	H323FreeEvent(evtPtr);
	return 0;
}

int iwfReqMode(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReqMode";
    H323EventData       *pH323Data;
	char				confIDStr[CONF_ID_STRLEN];
	char				h323CallID[CALL_ID_LEN] = {0};
	ConfHandle			*confHandle;
	int					sid;


	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s received RequestMode.\n", fn));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	sid = confHandle->h323MediaType;
	CacheReleaseLocks(confCache);

	pH323Data = (H323EventData *)evtPtr->data;
	
	if (pH323Data->modeStatus != cmReqModeRequest)
	{
		NETERROR(MSCC,("%s Error -  %s unexpected request Mode %d \n",
			fn,ConfID2String(evtPtr->confID,confIDStr),pH323Data->modeStatus));
		goto _return;
	}

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	if(iwfSendReqModeAck(evtPtr->confID,h323CallID) !=0 )
	{
		NETERROR(MIWF,("%s iwfSendReqModeAck Failed.\n",fn));
	}

	if(iwfSendBridgeCLC(evtPtr->confID,h323CallID,sid) !=0 )
	{
		NETERROR(MIWF,("%s iwfSendBridgeCLC Failed.\n",fn));
	}

_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

/* Called when OLC is received in ReqMode state.
 * Send reinvite to SIP. If OLC is from Cisco and has media
 * port 4000, do not include SDP in reinvite.
 */
int iwfReqModeOLC(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReqModeOLC";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	char				ipStr[20];
	SipAppCallHandle	*pSipData = NULL;
	char				h323CallID[CALL_ID_LEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char				evtStr[80];
    SDPAttr 			*attr = NULL;
	CallHandle			*callHandle;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset <=0)
	{
		NETERROR(MSCC,("%s Received OLC without RTPSet %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
		H323FreeEvData(evtPtr->data);
		free(evtPtr);
		return 0;
	}

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

    CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
    if (!(callHandle = CacheGet(callCache, h323CallID)))
    {
        NETERROR(MIWF,("%s Unable to locate H323 CallHandle %s\n",
				fn, CallID2String(h323CallID, callIDStr)));
		SipFreeAppCallHandle(pSipData);
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		CacheReleaseLocks(callCache);
        goto _return;
    }

	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s Received OLC from vendor %s with rtp port %d\n", 
		fn, GetVendorDescription(callHandle->vendor), 
		pH323Data->localSet[0].rtpport));


	/* Get conf handle */
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MIWF,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		CacheReleaseLocks(callCache);
		SipFreeAppCallHandle(pSipData);
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		iwfSendRelComp(evtPtr->confID, h323CallID);
		goto _return;
	}


    /* H323 side is Cisco Call Mgr and rtp port sent in OLC is 4000 */
    if ((callHandle->vendor == Vendor_eCisco) &&
        (pH323Data->localSet[0].rtpport == 4000))
    {
        /* Send a reinvite without SDP */
        pSipData->nlocalset = 0;
        pSipData->localSet = NULL;

		confHandle->olcFromCisco = 1;

		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Sending Invite without SDP\n", fn));
    }
	else
	{
		pSipData->nlocalset = pH323Data->nlocalset;
		if(pSipData->nlocalset >0)
		{
			pSipData->localSet = (RTPSet *)(calloc(pH323Data->nlocalset, sizeof(RTPSet)));
			memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
			sanitizeCodecs4sip(pSipData,&pSipData->nlocalset, callHandle->vendor);
		}

		if(pSipData->localSet[0].codecType == T38Fax)
		{
		  // This is fax call present only fax codec information
		  if( pSipData->attr_count > 0 )
		  {
		    int k;
		    for( k = 0; k < pSipData->attr_count; k++ )
		      {
		      
			if( (pSipData->attr)[k].name ) {
			  free( (pSipData->attr)[k].name );
			}
			if( (pSipData->attr)[k].value ) {
			  free( (pSipData->attr)[k].value );
			}
		      }
		    if(pSipData->attr) {
		      free (pSipData->attr);
		    }
		    pSipData->attr_count = 0;
		  }
			attr = (SDPAttr *) malloc(9*sizeof(SDPAttr));
			pSipData->attr_count = 9;
			pSipData->attr = attr;

			SipCreateMediaAttrib(&attr[0], 0, "T38FaxVersion", "0");
			SipCreateMediaAttrib(&attr[1], 0, "T38MaxBitRate", "14400");
			SipCreateMediaAttrib(&attr[2], 0, "T38FaxFillBitRemoval", "0");
			SipCreateMediaAttrib(&attr[3], 0, "T38FaxTranscodingMMR", "0");
			SipCreateMediaAttrib(&attr[4], 0, "T38FaxTranscodingJBIG", "0");
			SipCreateMediaAttrib(&attr[5], 0, "T38FaxRateManagement", "transferredTCF");
			SipCreateMediaAttrib(&attr[6], 0, "T38FaxMaxBuffer", "200");
			SipCreateMediaAttrib(&attr[7], 0, "T38FaxMaxDatagram", "72");
			SipCreateMediaAttrib(&attr[8], 0, "T38FaxUdpEC", "t38UDPRedundancy");
		}
		confHandle->h323MediaType = (pSipData->localSet[0].codecType == T38Fax)?
		  CONF_MEDIA_TYPE_DATA:CONF_MEDIA_TYPE_VOICE;

		confHandle->h323MediaType = (pSipData->localSet[0].codecType == T38Fax)?
											CONF_MEDIA_TYPE_DATA:CONF_MEDIA_TYPE_VOICE;
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Sending Invite with SDP\n", fn));
	}

	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	pSipData->maxForwards = sipmaxforwards;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));
	pSipEvt->event = Sip_eBridgeInvite;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);

	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;

		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}
	else
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Sent invite to SIP\n", fn));
	}

_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

/************************ Sip To H323 Action routines *************************/



/*	Convert to H323Data Format
	Send an event to h323 Processor 
*/
int iwfInitiateSetup(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateSetup";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;
	char 				*pCallingpn,*pCalledpn;
	char				ipStr[20];
	SipAppCallHandle	*pSipData;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char                confIDStr[CALL_ID_STRLEN] = {0};
	CallHandle			*callHandle;
	ConfHandle			*confHandle;
	H323RTPSet  		*h323RtpSet;
	char				sipCallID[CALL_ID_LEN] = {0};
        int                  index;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,ConfID2String(evtPtr->callID,callIDStr)));
		goto _dropCall;
	}

	pSipData = (SipAppCallHandle *)(evtPtr->data);
	setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	// Based on the privacy requested on the incoming call leg 
	// Callingpn has to be appropriately set on the h323 side.

	// Turn anonymous on when 
	// 1. Calling EP config indicates all calls from caller has to cid blocked
	// or 
	// 2. Caller has requested cid block on per call basis using *67 type of service
	// or 
	// 3. Caller has requested cid blocking on per call basis using SIP privacy headers

	if( pSipData->generate_cid == cid_block || 
	    ( pSipData->generate_cid == cid_noaction && 
	      pSipData->incomingPrivType != privacyTypeNone && pSipData->privLevel == privacyLevelId) )
	{
		if(pSipData->callingpn->name && strlen(pSipData->callingpn->name))
		{
			free(pSipData->callingpn->name);
		}
		pSipData->callingpn->name = strdup("anonymous");

		if(pSipData->callingpn->display_name && strlen(pSipData->callingpn->display_name))
		{
			free(pSipData->callingpn->display_name);
		}
		pSipData->callingpn->display_name = strdup("Anonymous");
	}

	if(pSipData->callingpn && 
	   pSipData->callingpn->name &&
	   ise164(SVal(pSipData->callingpn->name)))
	{
		nx_strlcpy(pH323Data->callingpn,SVal(pSipData->callingpn->name),PHONE_NUM_LEN);
	}
	nx_strlcpy(pH323Data->calledpn,sanitizeSipNumber(pSipData->requri->name),PHONE_NUM_LEN);
	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s called party number is %s\n",
		fn,pH323Data->calledpn));

	if (pSipData->callingpn->display_name &&
            pSipData->callingpn->display_name[0] != '\0')
	{
		pH323Data->display = strdup(pSipData->callingpn->display_name);
		pH323Data->displaylen = strlen(pSipData->callingpn->display_name);
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s calling party name is %s\n", fn,
					     pH323Data->display));
	}

	pH323Data->destip = pSipData->dipaddr;
	pH323Data->destport = pSipData->dipport;

	if(pSipData->nlocalset > 0) // && pSipData->localSet[0].rtpaddr)
	{
		if (pSipData->localSet[0].rtpaddr == 0)   /* rfc2543 on hold c=0 */
		{ 
			for (index = 0; index < pSipData->nlocalset; index++)
			{
				pSipData->localSet[index].direction = SendOnly;
			}
		} 
		copySipRtpset2H323Data(pH323Data, pSipData->localSet, pSipData,pSipData->nlocalset);
	}

	pH323Evt->event = SCC_eBridgeSetup;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));

		/* Send a Bye to Originator */

		evtPtr->event = Sip_eBridgeFinalResponse;

		/* Do not try to free evtPtr - we are passing its ownership */
		if(sipBridgeEventProcessor(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
		}
		CacheReleaseLocks(confCache);
		CacheReleaseLocks(callCache);
		return 0;
	}
	
	/* If SIP Invite has no SDP */
	if (pSipData->nlocalset <= 0)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Received Invite without SDP\n", fn));

        /* Make H323 stack send TCS */
		h323RtpSet = (H323RTPSet *) (CMalloc(callCache))(sizeof(H323RTPSet));
		memset(h323RtpSet, 0, sizeof(H323RTPSet));
        h323RtpSet->codecType = defaultCodec;
        switch(h323RtpSet->codecType)
        {
            case CodecG729A:
            case CodecG729:
                h323RtpSet->param = g729Frames;
                break;
            case CodecG7231:
                h323RtpSet->param = g7231Frames;
                break;
            case CodecGPCMA:
                h323RtpSet->param = g711Alaw64Duration;
                break;
            case CodecGPCMU:
                h323RtpSet->param = g711Ulaw64Duration;
                break;
	    default:
	        break; 
        }

		H323nremoteset(callHandle) = 1;
		H323remoteSet(callHandle) = h323RtpSet;

		confHandle->inviteNoSDP = 1;
	}

	goto _return;

_dropCall:
	getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);
	iwfSendFinalResponse(evtPtr->confID, sipCallID);

_return:
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

// Drop call only if media state is not connected
// Called only when we get a cancel
int iwfCancelInitiateReleaseComp(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfCancelInitiateReleaseComp";
	SCC_EventBlock		*pH323Evt;
	SipAppCallHandle	*pSipData;
	H323EventData		*pH323Data;
	ConfHandle			*confHandle;
	char 				str[EVENT_STRLEN];
	MLSM_StateMachineEntry *sm;
	char				stateString1[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				confIDStr[CONF_ID_LEN],callIDStr[CALL_ID_STRLEN] = {0};
	int					oldState;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}

	if(confHandle->subState ==  media_sConnected)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Ignoring event = %s.\n",
			fn, (char*) GetSipBridgeEvent(evtPtr->event)));
		CacheReleaseLocks(confCache);
		goto _return;
	}
	oldState = confHandle->state;
	confHandle->state = IWF_sError;

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s confID = %s\n",
		fn,sipEvent2Str(evtPtr->event,str),
		iwfState2Str(oldState,stateString1),
		iwfState2Str(confHandle->state,stateString2),
		(char*) ConfID2String(evtPtr->confID,confIDStr)));

	CacheReleaseLocks(confCache);

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Evt->event = SCC_eBridgeReleaseComp;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,evtPtr->callID,CALL_ID_LEN);

	memcpy(&pH323Evt->callDetails, &evtPtr->callDetails, 
		sizeof(CallDetails));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Evt->data = pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		/* Don't need to send any event to source */
		NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
	}
	
_return:
	SipFreeAppCallHandle(evtPtr->data);

	free(evtPtr);
	return 0;
}

/*	Send Release complete to H323 side
*	Doesn't send any event to the sip side
*/
int iwfInitiateReleaseComp(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateReleaseComp";
	SCC_EventBlock		*pH323Evt;
	SipAppCallHandle	*pSipData = NULL;
	H323EventData		*pH323Data = NULL;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	CallHandle			*callHandle = NULL;
	header_url			*new_contact = NULL;
	header_url_list		*remotecontact_list = NULL;
	char				str[256];

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));


	if(evtPtr->event == Sip_eBridgeFinalResponse && evtPtr->data)
	{
		pSipData = (SipAppCallHandle *)evtPtr->data;

		if(pSipData->msgHandle && pSipData->msgHandle->remotecontact_list)
		{
			remotecontact_list = pSipData->msgHandle->remotecontact_list;
			pSipData->msgHandle->remotecontact_list = NULL;
		}

		if(evtPtr->callDetails.flags & REDIRECT)
		{
			CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

			if((callHandle = CacheGet(callCache,evtPtr->callID)) < 0)
			{
				NETERROR(MIWF,("%s CallHandle Not found %s\n",
									fn,sccEventBlockToStr(evtPtr, str)));

				CacheReleaseLocks(callCache);

				goto _error;
			}

			new_contact = UrlDup(pSipData->msgHandle->remotecontact, MEM_LOCAL);

			if(!new_contact->name)
			{
				new_contact->name = strdup(callHandle->rfphonode.phone);
			}

			nx_strlcpy(callHandle->rfphonode.phone, new_contact->name,sizeof(callHandle->rfphonode.phone));
			sanitizeSipNumber(callHandle->rfphonode.phone);

			CacheReleaseLocks(callCache);
		}
	}

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Evt->event = SCC_eBridgeReleaseComp;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,evtPtr->callID,CALL_ID_LEN);

	memcpy(&pH323Evt->callDetails, &evtPtr->callDetails, 
		sizeof(CallDetails));

	// If final response code has arrived, it must be converted
	// into an ISDN cause !!
	if (evtPtr->callDetails.responseCode > 0)
	{
		pH323Evt->callDetails.cause = 
			iwfConvertSipCodeToCause(evtPtr->callDetails.responseCode);
	}

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Data->requri = new_contact;
	pH323Data->remotecontact_list = remotecontact_list;

	if(pSipData && pSipData->msgHandle)
	{
		if(pSipData->msgHandle->local && pSipData->msgHandle->local->display_name)
		{
			pH323Data->display    = strdup(pSipData->msgHandle->local->display_name);
			pH323Data->displaylen = strlen(pSipData->msgHandle->local->display_name);
		}
	}

	pH323Evt->data = pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		/* Don't need to send any event to source */
		NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
	}
	
_error:
	SipFreeAppCallHandle(evtPtr->data);

	free(evtPtr);
	return 0;
}

// Recieved Final Response in response to a reinvite 
int iwfReInviteFinalResponse(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteFinalResponse";
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				str[256];
	char				sipCallID[CALL_ID_LEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);
	iwfSendRelComp(evtPtr->confID,evtPtr->callID);
	iwfSendSipError(evtPtr->confID,sipCallID);

	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

//Recieved relcomp when we haven't answered a sip reinvite
int iwfReInviteRelComp(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteRelComp";
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				str[256];
	char				sipCallID[CALL_ID_LEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	iwfSendSipError(evtPtr->confID,evtPtr->callID);

	H323FreeEvent(evtPtr);
	return 0;
}

// Recieved unacceptable h323 event while handling reinvite from sip
// Not getting used yet
int iwfReInviteH323Error(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInviteH323Error";
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				str[256];
	char				h323CallID[CALL_ID_LEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
	iwfSendRelComp(evtPtr->confID,h323CallID);
	iwfSendSipError(evtPtr->confID,evtPtr->callID);

	H323FreeEvent(evtPtr);
	return 0;
}

/*	Convert to H323Data Format
	Send an event to h323 Processor 
*/
int iwfInitiateAlerting(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateAlerting";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;
	char 				*pCallingpn,*pCalledpn;
	char				ipStr[20];
	SipAppCallHandle	*pSipData;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	CallHandle			*callHandle;
	SipCallHandle		*sipCallHandle;
	char				sipCallID[CALL_ID_LEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);

	if (!(callHandle = CacheGet(callCache,sipCallID)))
	{
		NETERROR(MSCC,("%s Unable to locate SIP CallHandle %s\n",
			fn,CallID2String(sipCallID,callIDStr)));
		goto _return;
	}

	sipCallHandle = SipCallHandle(callHandle);

	pSipData = (SipAppCallHandle *)(evtPtr->data);

	pH323Evt = (SCC_EventBlock *)calloc(1, sizeof(SCC_EventBlock));
	pH323Data = (H323EventData *) calloc(1, sizeof(H323EventData));

	// If we had sent SDP in Invite and we receive 1xx with SDP,
    // send SDP in Alerting to trigger early H245.
	if(sipCallHandle->remoteSet.remoteSet_len > 0 && 
	   pSipData->nlocalset > 0 && pSipData->localSet[0].rtpaddr)
	{
		setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);
		copySipRtpset2H323Data(pH323Data, pSipData->localSet, pSipData, pSipData->nlocalset);
		// don't want to set PI for non fs h323 to sip 
		if(confHandle->subState == media_sConnected)
		{
			pH323Data->msgFlags |= MSGFLAG_PI;
		}
	}

	pH323Evt->event = SCC_eBridgeAlerting;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		/* Hopefully the error is temporary. Don't drop the call */
	}

_return:
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}



/*	Convert to H323Data Format
	Send an event to h323 Processor 
*/
int iwfInitiateConnect(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateConnect";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;
	char 				*pCallingpn,*pCalledpn;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				ipStr[20];
	SipAppCallHandle	*pSipData;
	ConfHandle			*confHandle;
	CallHandle			*callHandle;
	char				confIDStr[CONF_ID_STRLEN],evtStr[80];
	int					subState;
	int					oldState;
	char                stateString[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				sipCallID[CALL_ID_LEN] = {0};
	int					willAck = 0, willConnect = 0, haveLocks = 0, h323FastStart = 1;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	// We will send a Connect on the H.323 side only
	// if the Destination supports SDP in ACK, or we sent
	// an INVITE w/o SDP at start.
	// In other case, we will initiate an Invite NO SDP
	// and wait on a new offer from SIP
	// ACK will be sent, if are done (media is exchanged)
	// or we sent SDP in invite and dest won't take it in ACK

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);
	haveLocks = 1;

	if ((confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		if (callHandle = CacheGet(callCache, evtPtr->callID))
		{
			h323FastStart = callHandle->fastStart;	
		}

		getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);

		callHandle = CacheGet(callCache, sipCallID);

		if (!callHandle)
		{
			NETERROR(MSCC,
				("%s Unable to locate sipCallHandle.Dropping Call\n",fn));
			iwfSendRelComp(evtPtr->confID, evtPtr->callID);
			goto _return;
		}

		subState = confHandle->subState;
		if(subState == media_sNotConnected)
		{
			oldState = confHandle->state;

			if (!VendorSupportsFirstInviteNoSDP(callHandle->vendor))
			{
				// We had sent NULL SDP
				if (VendorSupportsNonStdAckSDP(callHandle->vendor))	
				{
					// Vendor supports non-std ack
					// This case is very unusual, and in future can be replaced
					// by new offer scenario

					willConnect = 1;

					/* Change state */
					confHandle->state = IWF_sNonFS200OK;
				}
				else
				{
					willAck = 1;

					/* Change state */
					confHandle->state = IWF_sWaitOnSipNewOffer;

					if (CallFceBundleId(callHandle) != 0)
					{
						FCECloseBundle(CallFceSession(callHandle), CallFceBundleId(callHandle));
						CallFceBundleId(callHandle) = 0;
					}
				}
			}
			else
			{
				// We had sent Invite NO SDP
				willAck = 0;
				willConnect = 1;

				/* Change state */
				confHandle->state = IWF_sNonFS200OK;
			}

			NETDEBUG(MIWF,NETLOG_DEBUG4,
				("%s %s/%s--> %s\n",
				fn,SCC_BridgeEventToStr(evtPtr->event,evtStr),
				iwfState2Str(oldState,stateString),
				iwfState2Str(confHandle->state,stateString2)));
		}
		else
		{
			willAck = 1;
			willConnect = 1;
		}
	} 
	else 
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	/* for early H245 connection */
	if (confHandle->earlyH245Connected)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Changing conf state from %d to %d \n",
				fn,confHandle->state,IWF_sConnected));
			confHandle->state = IWF_sConnected;
	}
	else
	{
		/* early H245 has failed and it was a slow start SETUP */
		if (h323FastStart == 0)
		{
			NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Changing conf state from %d to %d \n",
				fn,confHandle->state,IWF_sNonFS200OK));
			/* Fall back to NonFS200OK */
			confHandle->state = IWF_sNonFS200OK;
		}
	}

	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	haveLocks = 0;

	if (willAck)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,
			("%s Received %s generating ACK\n",
			fn,SCC_BridgeEventToStr(evtPtr->event,evtStr)));

		if(iwfSendAck(evtPtr->confID,sipCallID,NULL,0) <0)
		{
			NETERROR(MIWF,("%s iwfSendAck Failed\n",fn));
		}
	}


	if (willConnect)
	{
		pSipData = (SipAppCallHandle *)(evtPtr->data);

		pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
		memset(pH323Evt,0,sizeof(SCC_EventBlock));

		pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
		memset(pH323Data,0,sizeof(H323EventData));

		if(pSipData->nlocalset >0 && pSipData->localSet[0].rtpaddr)
		{
			setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);
			copySipRtpset2H323Data(pH323Data, pSipData->localSet, pSipData, pSipData->nlocalset);
		}

		pH323Evt->event = SCC_eBridgeConnect;

		memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
		memcpy(pH323Evt->callID,evtPtr->callID,CALL_ID_LEN);
		memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
		pH323Evt->data = (void *) pH323Data;

		if(iwfSendH323Event(pH323Evt) !=0)
		{
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));

			/* Send a Bye to Originator */

			evtPtr->event = Sip_eBridgeBye;

			/* Do not try to free evtPtr - we are passing its ownership */
			if(sipBridgeEventProcessor(evtPtr)!=0)
			{
				// Too Bad - can't do anything else
				NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
			}

			return 0;
		}
	}
	else
	{
		// We are going to do a re-invite
		// willReInvite = !willConnect

		// Before we re-invite we should close the hole we opened
		// as that is not going to be used

		pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
		memset(pSipData,0,sizeof(SipAppCallHandle));

		pSipData->maxForwards = sipmaxforwards;

		pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
		memset(pSipEvt,0,sizeof(SCC_EventBlock));

		pSipEvt->event = Sip_eBridgeInvite;

		memcpy(pSipEvt->confID, evtPtr->confID, CONF_ID_LEN);
		memcpy(pSipData->confID,evtPtr->confID, CONF_ID_LEN);

		memcpy(pSipEvt->callID, sipCallID, CALL_ID_LEN);
		memcpy(pSipData->callID, sipCallID, CALL_ID_LEN);

		pSipEvt->data = (void *) pSipData;

		if(sipBridgeEventProcessor(pSipEvt) !=0)
		{
			NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
			iwfSendRelComp(evtPtr->confID, evtPtr->callID);

			// Must free the original event
			goto _return;
		}
	}

_return:
	
	SipFreeAppCallHandle(evtPtr->data);

	free(evtPtr);

	if (haveLocks)
	{
		CacheReleaseLocks(confCache);
		CacheReleaseLocks(callCache);
	}

	return 0;
}

int iwfInitiateConnectOnNewOffer(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfInitiateConnectOnNewOffer";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;
	char 				*pCallingpn,*pCalledpn;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				ipStr[20];
	SipAppCallHandle	*pSipData;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN],evtStr[80];
	int					subState;
	int					oldState;
	char                stateString[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				sipCallID[CALL_ID_LEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	// Connect will be sent, ACK will not be.

	pSipData = (SipAppCallHandle *)(evtPtr->data);

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	if(pSipData->nlocalset >0 && pSipData->localSet[0].rtpaddr)
	{
		setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);
		copySipRtpset2H323Data(pH323Data, pSipData->localSet, pSipData, pSipData->nlocalset);
	}

	pH323Evt->event = SCC_eBridgeConnect;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));

		/* Send a Bye to Originator */

		evtPtr->event = Sip_eBridgeBye;

		/* Do not try to free evtPtr - we are passing its ownership */
		if(sipBridgeEventProcessor(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
		}
		return 0;
	}
	
	SipFreeAppCallHandle(evtPtr->data);

	free(evtPtr);
	return 0;
}


/*	
*	free the eventPtr 
*/
int iwfSipIgnore(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfSipIgnore";
	SipAppCallHandle	*pSipData;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Ignoring event = %d \n",fn,evtPtr->event));

	SipFreeAppCallHandle(evtPtr->data);

	free(evtPtr);
	return 0;
}

/*	
*	free the eventPtr 
*/
int iwfSipLogEvent(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfSipLogEvent";
	SipAppCallHandle	*pSipData;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Received event = %s \n",
		fn, (char*) GetSipBridgeEvent(evtPtr->event)));

	SipFreeAppCallHandle(evtPtr->data);

	free(evtPtr);
	return 0;
}

int iwfSipAck(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfSipAck";
	SipAppCallHandle	*pSipData;
    H323EventData       *pNextEvData;
	SCC_EventBlock		*pNextEvt;
	RTPSet				*localSet;
	ConfHandle			*confHandle;
    char                confIDStr[CONF_ID_STRLEN] = {0};
	char				sipCallID[CALL_ID_LEN] = {0};

	NETDEBUG(MIWF, NETLOG_DEBUG4,
		("Entering %s - event %d\n",fn,evtPtr->event));

	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MIWF,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	pSipData = (SipAppCallHandle *) evtPtr->data;

	/* If SIP Invite had no SDP, ACK should have one */
	if (confHandle->inviteNoSDP == 1)
	{
		if (pSipData->nlocalset <= 0)
		{
			NETERROR(MIWF,("%s Invite had no SDP, ACK has no SDP\n",fn));
			goto _dropCall;
		}

		setDefaultRtpParams(pSipData->localSet, pSipData->nlocalset);

		/* Send OLC Ack */
		localSet = (RTPSet *)calloc(1, sizeof(RTPSet));
		*localSet = pSipData->localSet[0];
		if(iwfSendBridgeOLCAck(evtPtr->confID,evtPtr->callID,localSet,1) != 0)
		{
			NETERROR(MIWF,("%s iwfSendBridgeOLC failed\n",fn));
			goto _dropCall;
		}

		/* If OLC was not received from Cisco, send OLC to H323 side */
		if (!(confHandle->olcFromCisco == 1))
		{
			localSet = (RTPSet *)calloc(1, sizeof(RTPSet));
			*localSet = pSipData->localSet[0];
			if(iwfSendBridgeOLC(evtPtr->confID,evtPtr->callID,localSet,1) != 0)
			{
				NETERROR(MIWF,("%s iwfSendBridgeOLC failed\n",fn));
				goto _dropCall;
			}
		}
		else
		{
			confHandle->olcFromCisco = 0;
			confHandle->inviteNoSDP = 0;
		}
	}

	goto _return;

_dropCall:
	iwfSendRelComp(evtPtr->confID, evtPtr->callID);
	getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);
	iwfSendFinalResponse(evtPtr->confID, sipCallID);

_return:
	CacheReleaseLocks(confCache);
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

/* 	Oops - we got a reInvite before H245 was done! 
*	Cache the Event
*/
int iwfCallConnectedReInvite(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfCallConnectedReInvite";
	ConfHandle			*confHandle;
	SCC_EventBlock 		*evt;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	H323EventData       *pH323Data;
	SCC_EventBlock		*pH323Evt;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		SipFreeAppCallHandle(evtPtr->data);
		free(evtPtr);
		return 0;
	}

	/* If we have a pending event - free it */
	if((evt = confHandle->sipEvt))
	{
		SipFreeAppCallHandle(evt->data);
		free(evt);
	}
	confHandle->sipEvt = evtPtr;
	CacheReleaseLocks(confCache);

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Evt->event = SCC_eBridgeControlConnected;

	memcpy(pH323Evt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed. Could not send new TCS\n",
			fn));
	}

	return 0;
}



/*	Handle 2 cases
*		Invite Hold
*		Invite SDP Change
*	Algorithm
*		Always send Null TCS
*		if hold send 200OK to Sip
*		if SDP Change Send BridgeCLC,BridgeTCS,BridgeOLC
*/
int iwfConnectedReInvite(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfConnectedReInvite";
	char 				*pCallingpn,*pCalledpn;
	char				ipStr[20];
	SipAppCallHandle	*pSipData;
	CallHandle			callHandleBlk = {0};
	CallHandle			*callHandle= NULL;
	int					hold = 0,modeChange= 0;
	char				sipCallID[CALL_ID_LEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char                stateString[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	int					i,j;
	RTPSet				*localSet;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	int					oldstate;
	char				evtStr[80];
	int					newMode;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pSipData = (SipAppCallHandle *)(evtPtr->data);

	if(pSipData->nlocalset == 0)
	{
		CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
		if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
		{
			NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
			CacheReleaseLocks(confCache);
			goto _return;
		}

		oldstate = confHandle->state;
		confHandle->state = IWF_sConnected;
		confHandle->subState = 0;
		NETDEBUG(MIWF,NETLOG_DEBUG4, 
			("%s %s/%s--> %s confID = %s\n",
			fn,sipEvent2Str(evtPtr->event,evtStr),
			iwfState2Str(oldstate,stateString),
			iwfState2Str(confHandle->state,stateString2),
			(char*) ConfID2String(evtPtr->confID,confIDStr)));

		CacheReleaseLocks(confCache);

		getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);
		iwfSendFinalResponse(evtPtr->confID,sipCallID);
		SipFreeAppCallHandle(evtPtr->data);
		free(evtPtr);
		return 0;
	}

	setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);

	/*
	   to preserve backward compatibility, interpret both IP address
	   set to 0.0.0.0 (rfc 2543) and a:sendonly (rfc 3264) as hold
	*/
	if(pSipData->nlocalset && ( (pSipData->localSet[0].direction == SendOnly) || (pSipData->localSet[0].rtpaddr == 0) ) )
	{
		hold = 1;
	}

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		goto _return;
	}

	oldstate = confHandle->state;

	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate SIP CallHandle %s\n",
			fn,CallID2String(evtPtr->callID, callIDStr)));
		goto _return;
	}

	/* Is the h.323 ep a Annex F SET */
	if(H323controlState(callHandle) == UH323_sControlIdle)
	{
		confHandle->state = IWF_sAnnexF;
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Handling Sip Event for AnnexF\n", fn));
		CacheReleaseLocks(confCache);

		if(iwfSipEventProcessor(evtPtr)<0)
		{
				SipFreeAppCallHandle(evtPtr->data);
				free(evtPtr);
		}
		return 0;
	}

	if(hold)
	{
		confHandle->state = IWF_sHeldBySip;
		confHandle->subState = RI_sWONullTCSAck;
	}
	else {
		confHandle->state = IWF_sReInvite;
		if ( ((confHandle->h323MediaType == CONF_MEDIA_TYPE_VOICE) && 
			  (pSipData->localSet[0].codecType == T38Fax))
				|| 
			(  (confHandle->h323MediaType == CONF_MEDIA_TYPE_DATA) && 
			   (pSipData->localSet[0].codecType != T38Fax) ) )
		{
			modeChange= 1;
		}
		else {
			confHandle->subState = RI_sWONullTCSAck;
		}
	}
	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s confID = %s\n",
		fn,sipEvent2Str(evtPtr->event,evtStr),
		iwfState2Str(oldstate,stateString),
		iwfState2Str(confHandle->state,stateString2),
		(char*) ConfID2String(evtPtr->confID,confIDStr)));


	CacheReleaseLocks(confCache);


	
	if(modeChange)
	{
		newMode = (confHandle->h323MediaType == CONF_MEDIA_TYPE_DATA) ? 
											CONF_MEDIA_TYPE_VOICE : CONF_MEDIA_TYPE_DATA;
		/* send RequestMode */
		if(iwfSendReqMode(evtPtr->confID,evtPtr->callID, newMode, pSipData) != 0)
		{
			NETERROR(MIWF,("%s iwfSendReqMode Failed\n",fn));
		}
		goto _return;
	}

	/* Send Null TCS */
	if(iwfSendCapabilities(evtPtr->confID,evtPtr->callID,NULL,0) != 0)
	{
		NETERROR(MIWF,("%s iwfSendCapabilities Failed\n",fn));
		goto _return;
	}

	if(reInviteClc)
	{
		/* if this doesn't work - we can send CLC after Nonnull TCSAck
		*  And OLC at CLCAck
		*/
		if(iwfSendBridgeCLC(evtPtr->confID,evtPtr->callID,confHandle->h323MediaType) !=0 )
		{
			NETERROR(MIWF,("%s iwfSendBridgeCLC Failed.\n",fn));
		}
	}

_return:
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

/* Handle Sip Reinvite in state 
*		Send Bridge CLC, BridgeTCS, BridgeOLC
*/

int iwfSipHeldReInvite(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfSipHeldReInvite";
	SipAppCallHandle	*pSipData;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	char				sipCallID[CALL_ID_LEN] = {0};
	char				stateString[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	CallHandle			callHandleBlk = {0};
	int					i,j;
	RTPSet				*localSet;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				evtStr[80];
	int					oldstate;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pSipData = (SipAppCallHandle *)(evtPtr->data);
	setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);

	/*
	   to preserve backward compatibility, interpret both IP address
	   set to 0.0.0.0 (rfc 2543) and a:sendonly (rfc 3264) as hold
	*/
	if(pSipData->nlocalset >0 && pSipData->localSet[0].rtpaddr != 0 && pSipData->localSet[0].direction != SendOnly)
	{
		CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
		if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
		{
			NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
			CacheReleaseLocks(confCache);
			goto _return;
		}
		oldstate = confHandle->state;
		confHandle->subState = RI_sWOTCSAck;
		CacheReleaseLocks(confCache);

		iwfSendCapabilities(evtPtr->confID,evtPtr->callID,pSipData,
						pSipData->nlocalset);
	}
	else {
			NETDEBUG(MIWF,NETLOG_DEBUG4,
				("%s received hold. Ignoring. Sending 200 Ok to src anyway\n",
				fn));

			getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);

			iwfSend200Ok(sipCallID);

			CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);

			/* Stay on the HeldBySip State */
			if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
			{
				NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
					fn,ConfID2String(evtPtr->confID,confIDStr)));
				CacheReleaseLocks(confCache);
				goto _return;
			}
			oldstate = confHandle->state;
			confHandle->state = IWF_sHeldBySip;
			NETDEBUG(MIWF,NETLOG_DEBUG4, 
				("%s %s/%s--> %s confID = %s\n",
				fn,sipEvent2Str(evtPtr->event,evtStr),
				iwfState2Str(oldstate,stateString),
				iwfState2Str(confHandle->state,stateString2),
				(char*) ConfID2String(evtPtr->confID,confIDStr)));
			CacheReleaseLocks(confCache);
			goto _return;
	}

_return:
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

int iwfReInvite200Ok(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReInvite200Ok";
	char				sipCallID[CALL_ID_LEN] = {0};
	char				stateString[IWF_STATE_STR];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				evtStr[80];

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s Received %s generating ACK\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,evtStr)));
	getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);
	if(iwfSendAck(evtPtr->confID,sipCallID,NULL,0) <0)
	{
		NETERROR(MIWF,("%s iwfSendAck Failed\n",fn));
	}

_return:
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

/* Called when 200Ok is received in ReqMode state.
*  Send OLC and OLC ACK to H323.
*/
int iwfReqMode200Ok(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfReqMode200Ok";
	SipAppCallHandle	*pSipData;
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN];
	char				sipCallID[CALL_ID_LEN] = {0};
	char				stateString[IWF_STATE_STR];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	RTPSet				*localSet;
	char				h323CallID[CALL_ID_LEN] = {0};
	CallHandle			*h323callHandle=NULL;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pSipData = (SipAppCallHandle *)(evtPtr->data);
	if(pSipData->nlocalset <=0)
	{
		NETERROR(MSCC,("%s Received 200 OK without RTPSet %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	getPeerCallID(evtPtr->confID,evtPtr->callID,sipCallID);

	setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);

	localSet = (RTPSet *)malloc(sizeof(RTPSet));
	*localSet = pSipData->localSet[0];

	if(iwfSendBridgeOLC(evtPtr->confID,evtPtr->callID,localSet,1) !=0)
	{
		NETERROR(MIWF,("%s iwfSendBridgeOLC Failed.\n",fn));
		goto _return;
	}

	localSet = (RTPSet *)malloc(sizeof(RTPSet));
	*localSet = pSipData->localSet[0];

	if(iwfSendBridgeOLCAck(evtPtr->confID,evtPtr->callID,localSet,1) !=0)
	{
		NETERROR(MIWF,("%s iwfSendBridgeOLCAck Failed.\n",fn));
		goto _return;
	}

	/* Get conf handle */
	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MIWF,("%s Unable to locate ConfHandle %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
		iwfSendRelComp(evtPtr->confID, evtPtr->callID);
		iwfSendFinalResponse(evtPtr->confID, sipCallID);
		goto _return_with_locks;
	}

	/* If OLC received was from Cisco CM with rtp port 4000, 
	   send SIP ACK after getting OLC ACK */
    if (!(confHandle->olcFromCisco == 1))
    {
		if(iwfSendAck(evtPtr->confID,sipCallID,NULL,0) <0)
		{
			NETERROR(MIWF,("%s iwfSendAck Failed\n",fn));
			goto _return_with_locks;
		}
	}

	if ((h323callHandle = CacheGet(callCache, evtPtr->callID)) == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate h323CallHandle\n",fn));
		goto _return_with_locks;
		/* drop call here */
	}

	if ((h323callHandle->vendor == Vendor_eNortel) &&
		(confHandle->mediaOnHold == MEDIA_RESUME_OLCACK_RX))
	{
		confHandle->state = IWF_sConnected;
		confHandle->mediaOnHold = 0;
	}
	else
	{
		confHandle->mediaOnHold = MEDIA_RESUME_200Ok_RX;
	}


_return_with_locks:
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
_return:
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}


void debugPrintH323EvData(H323EventData *ptr)
{
	if(ptr->callingpn)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("CPN=%s\t",ptr->callingpn));
	}
	if(ptr->calledpn)
	{
	
		NETDEBUG(MIWF,NETLOG_DEBUG4,("CDPN=%s dstip = %lx destport=%d\t",
			ptr->calledpn,ptr->destip,ptr->destport));
	}
}

void setDefaultRtpParams(RTPSet rtpSet[],int nset)
{
	int 	i;
	RTPSet	*pRtpSet;

	for(i = 0,pRtpSet = rtpSet; i<nset;++i,++pRtpSet)
	{
		if(pRtpSet->param !=0)
		{
			continue;
		}
		switch(rtpSet[i].codecType)
		{
			case CodecG729A:
			case CodecG729:
				pRtpSet->param = g729Frames;
				break;
			case CodecG7231:
				pRtpSet->param = g7231Frames;
				break;
			case CodecGPCMA:
				pRtpSet->param = g711Alaw64Duration;
				break;
			case CodecGPCMU:
				pRtpSet->param = g711Ulaw64Duration;
				break;
			case T38Fax:
				pRtpSet->mLineNo = 2;
				break;
			default:
				pRtpSet->param = 0;
				break;
		}
	}
}


void sanitizeCodecs4sip(SipAppCallHandle *pSipData, int *pnset, int vendor)
{
	char fn[] = "sanitizeCodecs4sip()";
	int 	i;
	RTPSet	*localRtp,*pRtpSet;
	RTPSet	*rtpSet;
	int		nset = *pnset;
	int		g729found = 0;
	int		g729annexbfound = 0;
	int		hasVoice = 0, hasFax = 0, faxPosn = -1;

	if(nset <=0)
		return;

	rtpSet = pSipData->localSet;
	localRtp = (RTPSet *)calloc (nset,sizeof(RTPSet));
	memcpy(localRtp,rtpSet,nset*sizeof(RTPSet));
	memset(rtpSet,0,nset*sizeof(RTPSet));
	*pnset =0;

	for(i = 0,pRtpSet = localRtp; i<nset;++i,pRtpSet++)
	{
		switch(pRtpSet->codecType)
		{
			case CodecG728:
			case CodecG7231:
			case CodecGPCMA:
			case CodecGPCMU:
				hasVoice = 1;

			// NTE RTP
			case 101:
				rtpSet[*pnset] = *pRtpSet;
				(*pnset)++;
				break;

			case CodecG729:
			case CodecG729A:
				hasVoice = 1;
				if(!g729found)
				{
					g729found = 1;
					pRtpSet->codecType = CodecG729;
					rtpSet[*pnset] = *pRtpSet;
					(*pnset)++;
				}
				break;

			case CodecG729B:
			case CodecG729AwB:
				hasVoice = 1;
				g729annexbfound = 1;
				if(!g729found)
				{
					g729found = 1;
					pRtpSet->codecType = CodecG729;
					rtpSet[*pnset] = *pRtpSet;
					(*pnset)++;
				}
				break;

			case T38Fax:
				hasFax++;
				faxPosn = *pnset;

				rtpSet[*pnset] = *pRtpSet;
				(*pnset)++;
				break;

			default:
				break;
		}
	}

	if (hasFax > 1)
	{
		NETERROR(MIWF, ("%s Found more than one fax codecs in\n", fn));
	}

	if (hasVoice && hasFax)
	{
		// filter the fax codec - swap it with the fax codec
		rtpSet[faxPosn] = rtpSet[*pnset -1];
		(*pnset)--;
	}

	if (g729found) 
	{
			if(!g729annexbfound)
			{
				/* annexb=no */
				AddG729AnnexbToAttr(pSipData, 0, vendor);
			}
			else
			{
				/* annexb=yes */
				AddG729AnnexbToAttr(pSipData, 1, vendor);
			}
	}

	free(localRtp);
	
}


void 
AddG729AnnexbToAttr(SipAppCallHandle *pSipData, int flag, int vendor)
{
	SDPAttr *curr_attr= NULL, *new_attr=NULL;
	int		curr_count = pSipData->attr_count;

	curr_attr = pSipData->attr;
	new_attr = (SDPAttr *) malloc((curr_count+2)*sizeof(SDPAttr));

	if (curr_attr)
	{
		memcpy(new_attr, curr_attr, curr_count*sizeof(SDPAttr));
		int k;
		for( k = 0; k < curr_count; k++ )
		{
		    
		  if( (curr_attr)[k].name ) 
                  {
		    free( (curr_attr)[k].name );
		  }
		  if( (curr_attr)[k].value ) 
		  {
		    free( (curr_attr)[k].value );
		  }
		}		
		free(curr_attr);
	}

	SipCreateMediaAttrib(&new_attr[curr_count], 0, "rtpmap", "18 G729/8000");

	// Sonus specific change - Sonus GSX does not understand annexb= notation.

	if (vendor == Vendor_eSonusGSX)
		SipCreateMediaAttrib(&new_attr[curr_count+1], 0, "fmtp",
                                                                        flag ? "18 annexb:yes" : "18 annexb:no");

	else
		SipCreateMediaAttrib(&new_attr[curr_count+1], 0, "fmtp", 
									flag ? "18 annexb=yes" : "18 annexb=no");

	pSipData->attr = new_attr;
	pSipData->attr_count += 2;

}

void copySipRtp2H323(RTPSet rtpSet[],int *pnset)
{
	int 	i;
	RTPSet	*localRtp,*pRtpSet;
	int		nset = *pnset;
	int		g729found = 0;

	if(nset <=0)
		return;


	localRtp = (RTPSet *)calloc (nset,sizeof(RTPSet));
	memcpy(localRtp,rtpSet,nset*sizeof(RTPSet));
	memset(rtpSet,0,nset*sizeof(RTPSet));
	*pnset =0;

	for(i = 0,pRtpSet = localRtp; i<nset;++i,pRtpSet++)
	{
		switch(pRtpSet->codecType)
		{
			case CodecG728:
			case CodecG7231:
			case CodecGPCMA:
			case CodecGPCMU:
			case T38Fax:
				rtpSet[*pnset] = *pRtpSet;
				(*pnset)++;
				break;
			case CodecG729:
			case CodecG729A:
			case CodecG729B:
			case CodecG729AwB:
				if(!g729found)
				{
					g729found = 1;
					pRtpSet->codecType = CodecG729;
					rtpSet[*pnset] = *pRtpSet;
					(*pnset)++;
				}
				break;
			default:
				break;
		}
	}
	
}


/* Utility routines - These are NOT action routines and DO NOT FREE evtPtr */

int
iwfSend200OkHold(char *sipCallID)
{
	static char		fn[]	= "iwfSend200OkHold";
	CallHandle		callHandleBlk = {0};
	SipCallHandle		*sipCallHandle = NULL;
	char			callIDStr[CALL_ID_STRLEN] = {0};
	RTPSet			localSet;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;

	if (CacheFind(callCache, sipCallID, &callHandleBlk, sizeof(CallHandle))< 0)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
					fn,CallID2String(sipCallID,callIDStr)));
		return -1;
	}

	sipCallHandle = SipCallHandle(&callHandleBlk);

	memset(&localSet, 0, sizeof(RTPSet));

	if(sipCallHandle->remoteSet.remoteSet_len > 0)
	{
		/* Copy only the negotiated codec */
		memcpy(&localSet,sipCallHandle->remoteSet.remoteSet_val,sizeof(RTPSet));
	}

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));
	
	pSipEvt->data = pSipData;	

	/* Send back 200 Ok */
	pSipEvt->event = Sip_eBridge200;

	memcpy(pSipData->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->confID,callHandleBlk.confID,CONF_ID_LEN);
	memcpy(pSipEvt->confID,callHandleBlk.confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,sipCallID,CALL_ID_LEN);

	pSipData->nlocalset = 1; 
	pSipData->localSet = (RTPSet *)(calloc(1, sizeof(RTPSet)));
	memcpy(pSipData->localSet,&localSet,1*sizeof(RTPSet));

	if(pSipData->localSet[0].rtpaddr != 0)
	{
		pSipData->localSet[0].direction = RecvOnly;
		pSipData->attr_count = 1;
		pSipData->attr = (SDPAttr *) malloc(sizeof(SDPAttr));
		SipCreateMediaAttrib(&pSipData->attr[0], 0, "recvonly", NULL);
	}

	if(sipBridgeEventProcessor(pSipEvt)!=0)
	{
		/* Too Bad - can't do anything else */
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
		return -1;
	}
	return 0;
}

/* 	Send 200Ok to Sip side based on existing H323 calldestCallI
*/

int
iwfSend200Ok(char *sipCallID)
{
	static char			fn[]	= "iwfSend200Ok";
	CallHandle			callHandleBlk = {0};
	SipCallHandle		*sipCallHandle = NULL;
	char				callIDStr[CALL_ID_STRLEN] = {0};
	RTPSet				localSet;

	if (CacheFind(callCache, sipCallID, &callHandleBlk, sizeof(CallHandle))< 0)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
					fn,CallID2String(sipCallID,callIDStr)));
		return -1;
	}

    sipCallHandle = SipCallHandle(&callHandleBlk);

	if(sipCallHandle->remoteSet.remoteSet_len > 0)
	{
		/* Copy only the negotiated codec */
		memcpy(&localSet,sipCallHandle->remoteSet.remoteSet_val,sizeof(RTPSet));
	}

	if(iwfSend200OkMedia(callHandleBlk.confID,sipCallID,&localSet,1)!=0)
	{
		/* Too Bad - can't do anything else */
		NETERROR(MIWF,("%s iwfSend200OkMedia Failed\n",fn));
		return -1;
	}
	return 0;
}

// This will fail only if sip event processor fails
int
iwfSend200OkMedia(char *confID,char *sipCallID, RTPSet *localSet, int nlocalset)
{
	static char			fn[]	= "iwfSend200OkMedia";
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	CallHandle			callHandleBlk = {0};
	SipCallHandle		*sipCallHandle;
	int					i;
	int					send2833 = 0;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));
	
	pSipEvt->data = pSipData;	

	/* Send back 200 Ok */
	pSipEvt->event = Sip_eBridge200;

    memcpy(pSipData->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->confID,confID,CONF_ID_LEN);
    memcpy(pSipEvt->confID,confID,CONF_ID_LEN);
    memcpy(pSipEvt->callID,sipCallID,CALL_ID_LEN);

	if(nlocalset > 0)
	{
		int nset;
		if(localSet->codecType != T38Fax)
		{
			if (CacheFind(callCache, sipCallID, &callHandleBlk, sizeof(CallHandle))> 0)
			{
    			sipCallHandle = SipCallHandle(&callHandleBlk);
				for(i=0;i<sipCallHandle->localSet.localSet_len;++i)
				{
					if(sipCallHandle->localSet.localSet_val[i].codecType == 101)
					{
						send2833 = 1;
						break;
					}
				}
			}
		}
		if(send2833 && always2833)
		{
			nset = pSipData->nlocalset = nlocalset + 1; 
			pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
			memcpy(pSipData->localSet,localSet,nlocalset*sizeof(RTPSet));

			pSipData->localSet[nset-1].codecType = 101;
			pSipData->localSet[nset-1].rtpaddr = pSipData->localSet[0].rtpaddr;
			pSipData->localSet[nset-1].rtpport = pSipData->localSet[0].rtpport;
			pSipData->attr_count = 2;
			pSipData->attr = (SDPAttr *) malloc(2*sizeof(SDPAttr));
			SipCreateMediaAttrib(&pSipData->attr[0], 0, "rtpmap", "101 telephone-event/8000");
			SipCreateMediaAttrib(&pSipData->attr[1], 0, "fmtp", "101 0-15");
		}
		else {
			nset = pSipData->nlocalset = nlocalset; 
			pSipData->localSet = (RTPSet *)(calloc(nset, sizeof(RTPSet)));
			memcpy(pSipData->localSet,localSet,nlocalset*sizeof(RTPSet));
		}
	}

	if(sipBridgeEventProcessor(pSipEvt)!=0)
	{
		/* Too Bad - can't do anything else */
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
		return -1;
	}
	return 0;
}

int
iwfSendCapabilities(char *confID,char *h323CallID, SipAppCallHandle *pSipData, int nset)
{
	static char			fn[]	= "iwfSendCapabilities";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;
	RTPSet 				*localSet;

	localSet = pSipData ? pSipData->localSet: NULL;
	setDefaultRtpParams(localSet,nset);

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));


	/* if the sdp is present and media address is non null */
	if(nset >0 && localSet[0].rtpaddr)
	{
		copySipRtpset2H323Data(pH323Data, localSet, pSipData, nset);
	}

	pH323Evt->event = SCC_eBridgeCapabilities;

	memcpy(pH323Evt->confID,confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed. Could not send new TCS\n",
			fn));
	}

	return 0;
}

/* pRtpSet is consumed by the call */
int
iwfSendBridgeOLC(char *confID,char *h323CallID,RTPSet *pRtpSet,int nset)
{
	static char			fn[]	= "iwfSendBridgeOLC";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Data->localSet = pRtpSet; 
	pH323Data->nlocalset = nset;
	pH323Evt->event = SCC_eBridgeOLC;
	if(pRtpSet->codecType == T38Fax)
	{
		pH323Data->sid = pRtpSet->mLineNo = 2;
	}

	memcpy(pH323Evt->confID,confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed.\n",
			fn));
	}

	return 0;
}

int
iwfSendBridgeCLC(char *confID, char *h323CallID, int sid)
{
	static char			fn[]	= "iwfSendBridgeCLC";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	memcpy(pH323Evt->confID,confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);

	pH323Data->sid = sid;
	pH323Evt->event = SCC_eBridgeCLC;
	pH323Evt->data = pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed.\n",
			fn));
	}

	return 0;
}

/* pRtpSet is consumed by the call */
int
iwfSendBridgeOLCAck(char *confID,char *h323CallID,RTPSet *pRtpSet,int nset)
{
	static char			fn[]	= "iwfSendBridgeOLCAck";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt,*pH323Evt;

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Data->localSet = pRtpSet; 
	pH323Data->nlocalset = nset;
	pH323Evt->event = SCC_eBridgeChanConnect;

	if(pRtpSet->codecType == T38Fax)
	{
		pH323Data->sid = pRtpSet->mLineNo = 2;
	}

	memcpy(pH323Evt->confID,confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed.\n",
			fn));
	}

	return 0;
}

int
iwfSendReqMode(char *confID,char *h323CallID, int h323MediaType, SipAppCallHandle *pSipData)
{
	static char			fn[]	= "iwfSendReqMode";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pH323Evt;
	char 				*voiceCodecStr= NULL;

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Evt->event = SCC_eBridgeRequestMode;
	switch(h323MediaType)
	{
		case CONF_MEDIA_TYPE_DATA:
			strcpy(pH323Data->modeName,"t38fax");
			break;

		case CONF_MEDIA_TYPE_VOICE:
			if (voiceCodecStr = SipDataCodec2StrForH323Stack(pSipData))
			{
				strcpy(pH323Data->modeName,voiceCodecStr);
				NETDEBUG(MIWF,NETLOG_DEBUG4,
				("%s sending SCC_eBridgeRequestMode for codec=%s\n",
						fn, voiceCodecStr));
			}
			else
			{
				NETERROR(MIWF,("%s Bad Value for CodecType %d\n",
					fn, pSipData->localSet[0].codecType));
				return(-1);
			}
			break;

		case CONF_MEDIA_TYPE_VIDEO:
			strcpy(pH323Data->modeName, "video");
			break;
	}

	pH323Data->modeStatus = cmReqModeRequest;

	memcpy(pH323Evt->confID,confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed.\n",
			fn));
		return(-1);
	}
	return 0;
}

int
iwfSendReqModeAck(char *confID,char *h323CallID)
{
	static char			fn[]	= "iwfSendReqModeAck";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pH323Evt;

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Evt->event = SCC_eBridgeRequestMode;
	strcpy(pH323Data->modeName,"t38fax");
	pH323Data->modeStatus = cmReqModeAccept;

	memcpy(pH323Evt->confID,confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed.\n",
			fn));
	}
	return 0;
}

int
iwfSendInfo200Ok(char *confID,char *sipCallID)
{
	static char			fn[]	= "iwfSendInfo200Ok";
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				callIDStr[CALL_ID_STRLEN] = {0};

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->responseCode = 200; 
	pSipEvt->event = Sip_eBridgeInfoFinalResponse;
	pSipEvt->data = (void *)pSipData;

	memcpy(pSipEvt->confID,confID,CALL_ID_LEN);
	memcpy(pSipEvt->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->confID,confID,CONF_ID_LEN);

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		// can't do anything 
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
	}
	
	return 0;
}

int
iwfSendFinalResponse(char *confID,char *sipCallID)
{
	static char			fn[]	= "iwfSendFinalResponse";
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				callIDStr[CALL_ID_STRLEN] = {0};

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->responseCode = 488; 
	pSipEvt->event = Sip_eBridgeFinalResponse;
	pSipEvt->data = (void *)pSipData;

	memcpy(pSipEvt->confID,confID,CALL_ID_LEN);
	memcpy(pSipEvt->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->confID,confID,CONF_ID_LEN);

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		// can't do anything 
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
	}
	
	return 0;
}

int
iwfSendSipError(char *confID,char *sipCallID)
{
	static char			fn[]	= "iwfSendSipError";
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	char				callIDStr[CALL_ID_STRLEN] = {0};

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	pSipEvt->event = Sip_eBridgeError;
	pSipEvt->data = (void *)pSipData;

	memcpy(pSipEvt->confID,confID,CALL_ID_LEN);
	memcpy(pSipEvt->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->confID,confID,CONF_ID_LEN);

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		// can't do anything 
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
	}
	
	return 0;
}

int
iwfSendRelComp(char *confID,char *h323CallID)
{
	static char			fn[]	= "iwfSendRelComp";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pH323Evt;

	pH323Evt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pH323Evt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Evt->event = SCC_eBridgeReleaseComp;

	memcpy(pH323Evt->confID,confID,CONF_ID_LEN);
	memcpy(pH323Evt->callID,h323CallID,CALL_ID_LEN);
	memcpy(pH323Data->callID,h323CallID,CALL_ID_LEN);
	pH323Evt->data = (void *) pH323Data;

	if(iwfSendH323Event(pH323Evt) !=0)
	{
		NETERROR(MIWF,("%s iwfSendH323Event Failed.\n",
			fn));
	}
	return 0;
}

int iwfH323HeldReInvite(SCC_EventBlock *evtPtr){return(0);}
int iwfBothHeldReInvite(SCC_EventBlock *evtPtr){return(0);}

int iwfSendAck(char *confID, char sipCallID[], RTPSet *localSet, int nlocalset)
{
	static char			fn[]	= "iwfSendAck";
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	header_url			*pRequri;
	char				h323CallID[CALL_ID_LEN] = {0};
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char                confIDStr[CONF_ID_STRLEN] = {0};
	ConfHandle			*confHandle;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(sipCallID,callIDStr)));


	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	if(nlocalset >0)
	{
		pSipData->nlocalset = nlocalset;

		pSipData->localSet = (RTPSet *)(calloc(nlocalset, sizeof(RTPSet)));
		memcpy(pSipData->localSet,localSet,nlocalset*sizeof(RTPSet));
	}
	pSipEvt->event = Sip_eBridgeAck;

	memcpy(pSipEvt->confID,confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->callID,sipCallID,CALL_ID_LEN);
	memcpy(pSipData->confID,confID,CONF_ID_LEN);
	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));
		return -1;
	}
	
	return 0;

}

int
iwfHandleH323Dtmf (SCC_EventBlock *evtPtr)
{
	char fn[] = "iwfHandleH323Dtmf ()";
	H323EventData *pH323Data;
	SipAppCallHandle  *pSipData;
	SCC_EventBlock *pSipEvt;
	header_url *pRequri;
	char ipstr[32];

	if (evtPtr == NULL) {
		NETERROR (MIWF, ("%s Null event pointer\n", fn));
		return -1;
	}
	pH323Data = (H323EventData *)evtPtr->data;

	if (evtPtr->subEvent != SCC_eDTMF) {
		NETDEBUG (MIWF, NETLOG_DEBUG4, ("%s: Not a dtmf event\n", fn));
		goto _return;
	}
	if ( pH323Data->dtmf->sig < 0 )
	{
		NETDEBUG (MIWF, NETLOG_DEBUG4, ("%s: ignored unsuportted userInput\n", fn));
		goto _return;
	}
	pSipEvt = (SCC_EventBlock *) malloc (sizeof (SCC_EventBlock));
	if (pSipEvt == NULL) {
		NETERROR (MIWF, ("%s Malloc Failure.\n", fn));
		goto _return;
	}
	memset (pSipEvt, 0, sizeof (SCC_EventBlock ));
	pSipData = (SipAppCallHandle *)malloc (sizeof (SipAppCallHandle));
	if (pSipData == NULL) {
		NETERROR (MIWF, ("%s Malloc failure.\n", fn));
		goto _return;
	}
	memset (pSipData, 0, sizeof (SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;
	pSipEvt->event = Sip_eBridgeInfo;
	pSipEvt->data = (void *) pSipData;
	memcpy (&pSipEvt->callDetails, &evtPtr->callDetails, sizeof (CallDetails));

/*	pRequri = (header_url *) malloc (sizeof (header_url));
	memset (pRequri, 0, sizeof (header_url));

	pRequri->name = strdup (pH323Data->calledpn);
	pRequri->host = strdup (FormatIpAddress (pH323Data->destip, ipstr));
	pRequri->port = pH323Data->destport;

	pSipData->requri = pRequri;
*/
	
	memcpy (pSipEvt->confID, evtPtr->confID, CONF_ID_LEN);
	memcpy (pSipEvt->callID, evtPtr->callID, CALL_ID_LEN);
	memcpy (pSipData->confID, evtPtr->confID, CONF_ID_LEN);
	memcpy (pSipData->callID, evtPtr->callID, CALL_ID_LEN);

	/* Copy dtmf parameters here. ... */
	pSipData->dtmf = (DTMFParams *)malloc (sizeof (DTMFParams));
	if (pSipData->dtmf == NULL) 
	{
		NETERROR(MIWF, ("%s Malloc failure.\n", fn));
		goto _return;
	}
	pSipData->dtmf->sig = pH323Data->dtmf->sig;
	pSipData->dtmf->duration = DTMF_DEFAULT_DURATION;
	pSipData->ndtmf = 1;

	/* Send to sip */
	if (sipBridgeEventProcessor (pSipEvt) != 0) 
	{
		NETERROR (MIWF, ("%s sipBridgeEventProcessor Failed.\n", fn));
	}
_return:	
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
}

static int 
iwfHandleSipDtmf (SCC_EventBlock *evtPtr)
{
	char fn[] = "iwfHandleSipDtmf()";
	SCC_EventBlock *pH323Evt;
	SipAppCallHandle *pSipData;
	H323EventData *pH323Data;
	char h323CallID[CALL_ID_LEN];

	if (evtPtr == NULL) 
	{
		NETERROR (MIWF, ("%s Null event !\n", fn));
		return -1;
	}
	pH323Evt = (SCC_EventBlock *) malloc (sizeof (SCC_EventBlock ));
	if (pH323Evt == NULL) {
		NETERROR (MIWF, ("%s malloc failure\n", fn));
		goto _return;
	}

	memset (pH323Evt, 0, sizeof (SCC_EventBlock));
	pH323Evt->event = SCC_eBridgeGenericMsg;
	pH323Evt->subEvent = SCC_eDTMF;
	pH323Data = (H323EventData *)malloc (sizeof (H323EventData));
	if (pH323Data == NULL) 
	{			
		NETERROR (MIWF, ("%s malloc failure.\n",fn));
		free (pH323Evt);
		goto _return;
	}
	memset (pH323Data, 0, sizeof (H323EventData));
	pSipData = (SipAppCallHandle *) evtPtr->data;
	pH323Data->dtmf = (DTMFParams *) malloc (sizeof (DTMFParams));
	pH323Data->dtmf->sig = pSipData->dtmf->sig;
	pH323Data->dtmf->duration = pSipData->dtmf->duration;
	pH323Evt->data = pH323Data;
	if (getPeerCallID (evtPtr->confID, evtPtr->callID, h323CallID) < 0) {
		NETERROR (MIWF, ("%s iwf failed to get peer call id\n",fn));
		free (pH323Data->dtmf);
		free (pH323Data);
		goto _return;
	}
	memcpy (pH323Evt->confID, evtPtr->confID, CONF_ID_LEN);
	memcpy (pH323Evt->callID, evtPtr->callID, CALL_ID_LEN);

	memcpy (pH323Data->callID, evtPtr->callID, CALL_ID_LEN);

	if (iwfSendH323Event (pH323Evt) != 0) {
		NETERROR (MIWF, ("%s iwfSendH323Event failed.\n", fn));
	}
_return:
	return 0;
}

int 
iwfHandleSipInfo (SCC_EventBlock *evtPtr)
{
	char fn[] = "iwfHandleSipInfo()";
	SCC_EventBlock *pH323Evt;
	SipAppCallHandle *pSipData;
	char sipCallID[CALL_ID_LEN] = {0};

	if (evtPtr == NULL) 
	{
		NETERROR (MIWF, ("%s Null event !\n", fn));
		return -1;
	}

	pSipData = (SipAppCallHandle *) evtPtr->data;
	if (pSipData->ndtmf > 0)
	{
		iwfHandleSipDtmf (evtPtr);
	}

	// May  need to handle ISUP, QSIG, or other here in the future
	// for now, make sure we return a 200 OK to bridge
    getPeerCallID(evtPtr->confID, evtPtr->callID, sipCallID);
	iwfSendInfo200Ok (evtPtr->confID, sipCallID);

	SipFreeAppCallHandle (evtPtr->data);
	free (evtPtr);
	return 0;
}

int iwfRcvBridgeTcs(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfRcvBridgeTcs";
    H323EventData       *pH323Data;
	char				h323CallID[CALL_ID_LEN] = {0};
	SipCallHandle		*sipCallHandle;
	int					i,j,codecMatch;
	RTPSet				*localSet;
	char 				ipstr[24];
	CallHandle			*h323callHandle, *callHandle;

	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s received TCS.\n", fn));

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if (h323callHandle = CacheGet(callCache, h323CallID))
	{
		if (h323callHandle->fastStart)
		{
			NETDEBUG(MIWF,NETLOG_DEBUG4, ("%s, BridgeTCS event is ingored for FS call.\n", fn));
			goto _return;
		}
	}

	if (!(callHandle = CacheGet(callCache, evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate sipCallHandle\n",fn));
		goto _return;
		/* drop call here */
	}

	sipCallHandle = SipCallHandle(callHandle);
	pH323Data = (H323EventData *) evtPtr->data;

	/* choose codec */
	codecMatch = 0;
	for(i = 0;i < pH323Data->nlocalset && !codecMatch; ++i)
	{
		for(j = 0;j<sipCallHandle->localSet.localSet_len;++j)
		{
			if(pH323Data->localSet[i].codecType == sipCallHandle->localSet.localSet_val[j].codecType)
			{
				localSet = (RTPSet *)malloc(sizeof(RTPSet));
				*localSet = sipCallHandle->localSet.localSet_val[j];
				localSet->param = pH323Data->localSet[i].param;
				if(CallFceTranslatedIpPort(h323callHandle))
				{
					localSet->rtpaddr = CallFceTranslatedIp(h323callHandle);
					localSet->rtpport = CallFceTranslatedPort(h323callHandle);
					NETDEBUG(MIWF,NETLOG_DEBUG4,
						("%s Mediarouting is enabled on Sip Side\n", fn));
				}
				codecMatch = 1;
				break;
			}
		}
	}
	
	if(!codecMatch)
	{
		/* Should send OlcReject here */
		NETERROR(MSCC,("%s No Matching codec\n",fn));
		goto _return;
	}

	NETDEBUG(MSCC,NETLOG_DEBUG4,("%s codec= %d param = %d ip = %s/%d\n",
		fn,localSet->codecType,localSet->param,FormatIpAddress(localSet->rtpaddr, ipstr),
		localSet->rtpport));

	if(iwfSendBridgeOLC(evtPtr->confID,h323CallID,localSet,1) !=0)
	{
		NETERROR(MIWF,("%s iwfSendBridgeOLC Failed.\n",fn));
	}

_return:

	CacheReleaseLocks(callCache);
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;

}

int iwfRcvBridgeOlc(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfRcvBridgeOlc";
    H323EventData       *pH323Data;
	char				h323CallID[CALL_ID_LEN] = {0};
	CallHandle			*h323callHandle, *callHandle;
	SipCallHandle   	*sipCallHandle = NULL;
	ConfHandle			*confHandle;
	RTPSet				*localSet = NULL;
	int					i;
	char 				ipstr[24];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				confIDStr[CONF_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));
	pH323Data = (H323EventData *) evtPtr->data;
	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if (!(h323callHandle = CacheGet(callCache, h323CallID)))
	{
		NETERROR(MSCC,("%s Unable to locate h323CallHandle\n",fn));
		CacheReleaseLocks(callCache);
		goto _return;
		/* drop call here */
	}

	if (!(callHandle = CacheGet(callCache, evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate SipCallHandle\n",fn));
		CacheReleaseLocks(callCache);
		goto _return;
		/* drop call here */
	}

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		CacheReleaseLocks(callCache);
		goto _return;
	}
	
	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s setting earlyH245Connected\n", fn));

	confHandle->earlyH245Connected = 1;
	CacheReleaseLocks(confCache);

    sipCallHandle = SipCallHandle(callHandle);

	/* choose codec */
	for(i = 0;i < sipCallHandle->localSet.localSet_len; ++i)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s codec = %d.\n",
			fn,sipCallHandle->localSet.localSet_val[i].codecType));

		if(sipCallHandle->localSet.localSet_val[i].codecType == pH323Data->localSet[0].codecType)
		{
			localSet = (RTPSet *)malloc(sizeof(RTPSet));
			*localSet = sipCallHandle->localSet.localSet_val[i];
			if(CallFceTranslatedIpPort(h323callHandle))
			{
				localSet->rtpaddr = CallFceTranslatedIp(h323callHandle);
				localSet->rtpport = CallFceTranslatedPort(h323callHandle);
				NETDEBUG(MIWF,NETLOG_DEBUG4,
					("%s Mediarouting is enabled on Sip Side\n", fn));
			}
			break;
		}
	}

	CacheReleaseLocks(callCache);
	
	if(!localSet)
	{
		/* Should send OlcReject here */
		NETERROR(MSCC,("%s No Matching codec\n",fn));
		goto _return;
	}
		
	NETDEBUG(MSCC,NETLOG_DEBUG4,("%s codec= %d param = %d ip = %s/%d\n",
		fn,localSet->codecType,localSet->param,FormatIpAddress(localSet->rtpaddr, ipstr),
		localSet->rtpport));

	/* Send OLCAck to H323 */
	if(iwfSendBridgeOLCAck(evtPtr->confID,h323CallID,localSet,1) !=0)
	{
		NETERROR(MIWF,("%s iwfSendBridgeOLCAck Failed.\n",fn));
	}

_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

int iwfConnectedNullTCS(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfConnectedNullTCS";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData = NULL;
	SipCallHandle   	*sipCallHandle = NULL;
	char				h323CallID[CALL_ID_LEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	CallHandle			callHandleBlk;
	int					i;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset != 0) 
	{
		NETERROR(MIWF,("%s Non Empty TCS !! Not being handled\n",fn));
		goto _return;
	}

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if ((confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		confHandle->state = IWF_sNullTCS;
		confHandle->subState = media_sNotConnected;
		confHandle->mediaOnHold = MEDIA_ON_HOLD_REINV_TX;
		getPeerCallIDFromConf(evtPtr->callID,confHandle,h323CallID);
	} 
	else 
	{
		CacheReleaseLocks(confCache);
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		// can't do anything
		SipFreeAppCallHandle(pSipData);
		goto _return;
	}
	CacheReleaseLocks(confCache);

	/* Null TCS received. Its not a good idea to do this
	 * state transition in the iwfsm 
	 */
	if (CacheFind(callCache, evtPtr->callID, &callHandleBlk,sizeof(CallHandle))< 0)
	{
		NETERROR(MSCC,
			("%s Unable to locate sipCallHandle.Dropping Call\n",fn));
		SipFreeAppCallHandle(pSipData);
		iwfSendRelComp(evtPtr->confID,h323CallID);
		goto _return;
	}

	sipCallHandle = SipCallHandle(&callHandleBlk);

	if(sipCallHandle->remoteSet.remoteSet_len > 0)
	{
		/* Copy only the negotiated codec */
		pSipData->nlocalset = sipCallHandle->remoteSet.remoteSet_len;
		pSipData->localSet = (RTPSet *)(calloc(sipCallHandle->remoteSet.remoteSet_len, sizeof(RTPSet)));
		memcpy(pSipData->localSet,sipCallHandle->remoteSet.remoteSet_val,
								sipCallHandle->remoteSet.remoteSet_len*sizeof(RTPSet));
	}

	for (i = 0; i < sipCallHandle->remoteSet.remoteSet_len; i++)
	{
		/*
		   it's decision time. since this invite is generated
		   by the msw, we have to decide whether to follow rfc
		   3264 or rfc 2543 to implement sip hold.
		   making this a configurable so that poor msw does not
		   have to toss a coin.
		*/
		if(siphold3264)
		{
			pSipData->localSet[i].direction = SendOnly;
		}
		/* otherwise set IP address to 0.0.0.0 */
		else
		{
			pSipData->localSet[i].rtpaddr = 0;
		}
	}

	/*
	   set a:sendonly
	*/
	if(siphold3264)
	{
		pSipData->attr_count = 1;
		pSipData->attr = (SDPAttr *) malloc(sizeof(SDPAttr));
		SipCreateMediaAttrib(&pSipData->attr[0], 0, "sendonly", NULL);
	}

	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, callHandleBlk.vendor);

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));
	pSipEvt->event = Sip_eBridgeInvite;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CALL_ID_LEN);

	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;

		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}

_return:	
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
}

int iwfNullTCS200Ok(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfNullTCS200Ok";
	SipAppCallHandle	*pSipData;
	ConfHandle			*confHandle;
	SCC_EventBlock 		*h323evtPtr = NULL;
	char				sipCallID[CALL_ID_LEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				confIDStr[CONF_ID_STRLEN] = {0};
	int					callonHold=0, count;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
    if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
    {
        NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
            fn,ConfID2String(evtPtr->confID,confIDStr)));
        goto _return;
    }
    getPeerCallIDFromConf(evtPtr->callID,confHandle,sipCallID);

	pSipData = (SipAppCallHandle *)(evtPtr->data);
	/* Cisco sip phone screws up on multiple Hold/Resumes and 
	* in one case sends out a 200Ok without SDP in it causing 
	* us to crash
	*/
	if (pSipData == NULL || pSipData->localSet == NULL)
	{
		iwfSendFinalResponse(evtPtr->confID, sipCallID);
		iwfSendRelComp(evtPtr->confID,evtPtr->callID);
		goto _return;
	}

	if(siphold3264)
	{
		if(pSipData->localSet[0].direction == RecvOnly)
		{       
			callonHold = 1;
		}       
		else    
		{       
			NETERROR(MSCC,("%s Received 200 OK with SDP attribute not set to recvonly in NULL TCS State %s\n", fn, ConfID2String(evtPtr->confID, confIDStr)));
		}       
	}
	else    
	{
		callonHold = 1; /* on 200Ok place the call on hold */
		if(pSipData->localSet[0].rtpaddr == 0)
		{       
			callonHold = 1;
		}       
		else    
		{       
			NETERROR(MSCC,("%s Received 200 OK with Connect address in NULL TCS State %s\n", fn,ConfID2String(evtPtr->confID, confIDStr)));
		}       
	}

	if(iwfSendAck(evtPtr->confID,sipCallID,NULL,0) <0)
	{
		NETERROR(MIWF,("%s iwfSendAck Failed\n",fn));
	}

	confHandle->mediaOnHold = MEDIA_ON_HOLD_200Ok_RX;
	count = 0;
	while (h323evtPtr = (SCC_EventBlock *)DeQueueH323Evt(confHandle->h323EvtList))
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Dequeuing msg %d %s\n", fn, count, CallID2String(evtPtr->callID,callIDStr)));
		/* Convert network event back to Bridge event*/
		h323evtPtr->event += SCC_eBridgeEventsMin;
		iwfH323EventProcessor(h323evtPtr);
		count ++;
	}

	if (callonHold)
	{
		if(iwfSendBridgeCLC(evtPtr->confID,evtPtr->callID, confHandle->h323MediaType) !=0 )
		{
			NETERROR(MIWF,("%s iwfSendBridgeCLC Failed.\n",fn));
		}
	}
	else
	{
		confHandle->state = IWF_sConnected;
	}

_return:
	CacheReleaseLocks(confCache);
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

int iwfNullTCSCapabilitiesRx(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= " iwfNullTCSCapabilitiesRx";
	ConfHandle			*confHandle;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				confIDStr[CONF_ID_STRLEN] = {0};
	SCC_EventBlock		*pNextEvt;
	char				h323CallID[CALL_ID_LEN] = {0};
	SipCallHandle   	*sipCallHandle = NULL;
	CallHandle			*callHandle, *h323callHandle;
    H323EventData       *pH323Data, *pNextEvData;
	int					i,j;
	RTPSet				*localSet = NULL;
	int					codecMatch = 0;
	SCC_EventBlock 		*h323evtPtr=NULL;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		H323FreeEvData(evtPtr->data);
		free(evtPtr);
		return 0;
	}

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	pH323Data = (H323EventData *)evtPtr->data;
	if (pH323Data->nlocalset)
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s TCS in IWF_sNullTCS mediaOnHold=%d\n",fn, confHandle->mediaOnHold));

		if (!(callHandle = CacheGet(callCache, h323CallID)))
		{
			// Error
			NETERROR(MSCC,("%s Unable to locate h323CallHandle\n",fn));
			goto _return;
		}

		if (confHandle->mediaOnHold != MEDIA_ON_HOLD_200Ok_RX)
		{
			if (callHandle->vendor == Vendor_eNortel) 
			{
				/* place this event in front of OLC */
				{
					List 		list= confHandle->h323EvtList;
					ListStruct	*olcptr =NULL;

					if (list)
					{
						olcptr = SearchListforMatch (confHandle->h323EvtList, NULL, matchOlcItem);

						if (olcptr == NULL)
						{
							/* no olc on list */
							QueueH323Evt(&(confHandle->h323EvtList), (void *)evtPtr);
							NETDEBUG(MIWF,NETLOG_DEBUG4,("%s TCS enqueued. No prior olc \n",fn));
						}
						else
						{
							listInsertItem(confHandle->h323EvtList,(void *)evtPtr, olcptr);
							NETDEBUG(MIWF,NETLOG_DEBUG4,("%s TCS enqueued with reshuffle\n",fn));
						}
					}
				}
			}
			else
			{
				/* non nortel */
				QueueH323Evt(&(confHandle->h323EvtList), (void *)evtPtr);
				NETDEBUG(MIWF,NETLOG_DEBUG4,("%s TCS enqueued\n",fn));
			}
		}
		else
		{
			NETDEBUG(MIWF,NETLOG_DEBUG4,("%s TCS processed. State changed to IWF_sReqMode %s\n",
				fn,CallID2String(evtPtr->callID,callIDStr)));

			/* conditions good for state transition */
			confHandle->state = IWF_sReqMode;
			if (callHandle->vendor == Vendor_eNortel) 
			{
				/* check for any pending OLCs */
				if (h323evtPtr = (SCC_EventBlock *)DeQueueH323Evt(confHandle->h323EvtList))
				{
					/* Convert network event back to Bridge event*/
					h323evtPtr->event += SCC_eBridgeEventsMin;
					iwfH323EventProcessor(h323evtPtr);
				}
			}
#if 1
			if (callHandle->vendor == Vendor_eNortel) 
			{
				h323callHandle = callHandle;
				/* Find the matching codec */
				if (!(callHandle = CacheGet(callCache, evtPtr->callID)))
				{
					NETERROR(MSCC,("%s Unable to locate sipCallHandle\n",fn));
						goto _return;
					/* drop call here */
				}
				sipCallHandle = SipCallHandle(callHandle);

				/* choose codec */
				for(i = 0; !codecMatch && i < sipCallHandle->localSet.localSet_len; ++i)
				{
					NETDEBUG(MIWF,NETLOG_DEBUG4,("%s codec = %d.\n",
						fn,sipCallHandle->localSet.localSet_val[i].codecType));

					for(j = 0; j<pH323Data->nlocalset;++j)
					{
						if(sipCallHandle->localSet.localSet_val[i].codecType == pH323Data->localSet[j].codecType)
						{
							localSet = (RTPSet *)malloc(sizeof(RTPSet));
							//*localSet = pH323Data->localSet[j];
								*localSet = sipCallHandle->localSet.localSet_val[i];

							if(CallFceTranslatedIpPort(h323callHandle))
							{
								localSet->rtpaddr = CallFceTranslatedIp(h323callHandle);
								localSet->rtpport = CallFceTranslatedPort(h323callHandle);
								NETDEBUG(MIWF,NETLOG_DEBUG4,
									("%s Mediarouting is enabled on Sip Side\n", fn));
							}

							codecMatch = 1;
							break;
						}
					}
				}

				if(!localSet)
				{
					/* Should send OlcReject here */
				NETERROR(MSCC,("%s No Matching codec\n",fn));
					goto _return;
				}
				setDefaultRtpParams(localSet,1);
				/* Send OLC to H323 */
				pNextEvt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
				memset(pNextEvt,0,sizeof(SCC_EventBlock));

				pNextEvData = (H323EventData *) malloc(sizeof(H323EventData));
				memset(pNextEvData,0,sizeof(H323EventData));

				memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);
				memcpy(pNextEvt->callID,h323CallID,CALL_ID_LEN);
				memcpy(pNextEvData->callID,h323CallID,CALL_ID_LEN);
				pNextEvt->data = pNextEvData;
				pNextEvData->localSet = localSet;
				pNextEvData->nlocalset = 1;

				pNextEvt->event = SCC_eBridgeOLC;

				if(iwfSendH323Event(pNextEvt) !=0)
				{
					NETERROR(MIWF,
						("%s iwfSendH323Event Failed. Could not send OLC\n",fn));
				}
			}
#endif

			H323FreeEvData(evtPtr->data);
			free(evtPtr);
		}
	}
	else
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Null TCS in IWF_sNullTCS mediaOnHold=%d. Freed\n",fn, confHandle->mediaOnHold));
		H323FreeEvent(evtPtr);
		free(evtPtr);
	}

_return:
	CacheReleaseLocks(callCache);
	CacheReleaseLocks(confCache);

	return 0;
}

int iwfNullTCSOLC(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfNullTCSOLC()";
	ConfHandle			*confHandle;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char				h323CallID[CALL_ID_LEN] = {0};
	CallHandle			*h323callHandle=NULL;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
	}

	CacheReleaseLocks(confCache);

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
	
	if ((h323callHandle = CacheGet(callCache, h323CallID)) == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate h323CallHandle\n",fn));
		goto _return;
	}

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s OLC with mediaOnHold=%d\n",fn, confHandle->mediaOnHold));

	/* Think about this one */
	if ((confHandle->mediaOnHold != MEDIA_ON_HOLD_200Ok_RX) ||
		(h323callHandle->vendor == Vendor_eNortel))
	{
		QueueH323Evt(&(confHandle->h323EvtList), (void *)evtPtr);
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Queued OLC\n",fn));
	}


_return:
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	/* don't free the evtPtr */
	return 0;
}

int iwfNullTCSReleaseComp(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfNullTCSReleaseComp()";
	ConfHandle			*confHandle;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char				h323CallID[CALL_ID_LEN] = {0};
	CallHandle			*h323callHandle=NULL;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s RelComp with mediaOnHold=%d\n",fn, confHandle->mediaOnHold));

	/* Nortel is sends a NullTCS followed by a Release when phone goes on-hook */
	if (confHandle->mediaOnHold != MEDIA_ON_HOLD_200Ok_RX) 
	{
		QueueH323Evt(&(confHandle->h323EvtList), (void *)evtPtr);
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Queued RelComp\n",fn));
	}
	else
	{
		CacheReleaseLocks(confCache);
		iwfInitiateBye(evtPtr);
		return 0;
	}


_return:
	CacheReleaseLocks(confCache);
	/* don't free the evtPtr */
	return 0;
}



int iwfRcvBridgeTcsAck(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfRcvBridgeTcsAck";
	ConfHandle			*confHandle;
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				confIDStr[CONF_ID_STRLEN] = {0};

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
	}
	else
	{
		confHandle->earlyH245Connected = 1;
	}
	CacheReleaseLocks(confCache);

_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

void
CheckG729AnnexbAttr(SipAppCallHandle *pSipData, int *support)
{
	int index;

	if (pSipData->attr_count == 0)
	{
		return;
	}

	for(index=0; index < pSipData->attr_count; index++)
	{
		if (!strcmp(pSipData->attr[index].name, "fmtp"))
		{
			if (pSipData->attr[index].value)
			{ 
				if(strcmp(pSipData->attr[index].value, "18 annexb=no")  == 0 ||
					strcmp(pSipData->attr[index].value, "18 annexb:no") == 0)
				{
					*support = 0;
					break;
				}
			}
		}
	}

	return;

}

void copySipRtpset2H323Data(H323EventData *pH323Data, RTPSet *localSet, SipAppCallHandle *pSipData, int nset)
{
	int i;
	RTPSet *p = NULL;
	int	annexbsupport=1; /* by default annexb is supported unless specified otherwise */
	int	addedcodecs;


	if ( nset < 1 )
	{
		return;
	}

	if (pSipData)
	{
		CheckG729AnnexbAttr(pSipData, &annexbsupport);
	}

	for(i=0;i <nset; i++)
	{
		if (localSet[i].codecType == CodecG729)
		{
			p = &localSet[i];
			break;
		}
	}
	pH323Data->nlocalset = nset;
	addedcodecs = annexbsupport? 3: 1;

	if (p)
	{
		pH323Data->nlocalset += addedcodecs;
	}

	pH323Data->localSet = (RTPSet *)(calloc(pH323Data->nlocalset, sizeof(RTPSet)));
	memcpy(pH323Data->localSet,localSet,nset*sizeof(RTPSet));
	if (p)
	{
		for ( i=0; i<addedcodecs; i++)
		{
			memcpy( &pH323Data->localSet[nset+i], p, sizeof(RTPSet) );
		}
		pH323Data->localSet[nset].codecType = CodecG729A;
		if (annexbsupport)
		{
			pH323Data->localSet[nset+1].codecType = CodecG729B;
			pH323Data->localSet[nset+2].codecType = CodecG729AwB;
		}
	}
}

char *
SipDataCodec2StrForH323Stack(SipAppCallHandle *pSipData)
{
	int	annexbsupport=1;
	int	codecType = pSipData->localSet[0].codecType;

	if (pSipData->nlocalset == 0)
	{
		return NULL;
	}

	if (codecType == CodecGPCMU)
	{
		return "g711Ulaw64k";
	}
	else if (codecType == CodecGPCMA)
	{
		return "g711Alaw64k";
	}
	else if (codecType == CodecG7231)
	{
		return "g7231";
	}
	else if (codecType == CodecG728)
	{
		return "g728";
	}
	else if (codecType == CodecG729)
	{
		CheckG729AnnexbAttr(pSipData, &annexbsupport);
		if (annexbsupport)
			{
				return "g729AnnexAwAnnexB";
			}
		else
			{
				return "g729AnnexA";
			}
	}
	else if (codecType == T38Fax)
	{
		return "t38fax";
	}
	else if (codecType == CodecG729A)
	{
		return "g729AnnexA";
	}
	else if (codecType == CodecG729B)
	{
		return "g729wAnnexB";
	}
	else if (codecType == CodecG729AwB)
	{
		return "g729AnnexAwAnnexB";
	}

	return NULL;
}

/* Called when OLC ACK is received in ReqMode state.
 * Send ACK with SDP to SIP if OLC came from Cisco 
 * Call Manager and had media port 4000.
 */
int iwfReqModeOLCAck(SCC_EventBlock *evtPtr)
{
    static char         fn[]    = "iwfReqModeOLCAck";
    H323EventData       *pH323Data;
    char                callIDStr[CALL_ID_STRLEN] = {0};
    char                confIDStr[CONF_ID_STRLEN] = {0};
    ConfHandle          *confHandle;
	char				h323CallID[CALL_ID_LEN] = {0};
	CallHandle			*h323callHandle=NULL;

    NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
        fn,CallID2String(evtPtr->callID,callIDStr)));

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MIWF,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	/* If OLC was received from Cisco with media port 4000, then
       OLC ACK will have the media port that Cisco actually intends
       to use. Send this media port to SIP side in ACK. */
    if (confHandle->olcFromCisco == 1)
    {
        pH323Data = (H323EventData *) (evtPtr->data);

        if(pH323Data->nlocalset <= 0)
        {
            NETERROR(MIWF,("%s Received OLC Ack without RTPSet %s\n",
                fn,ConfID2String(evtPtr->confID,confIDStr)));
            goto _dropCall;
        }

        if(iwfSendAck(evtPtr->confID, evtPtr->callID, pH323Data->localSet,
                      pH323Data->nlocalset) < 0)
        {
            NETERROR(MIWF,("%s iwfSendAck failed\n",fn));
            goto _dropCall;
        }

        confHandle->olcFromCisco = 0;
    }

	if (!(h323callHandle = CacheGet(callCache, h323CallID)))
	{
		NETERROR(MSCC,("%s Unable to locate h323CallHandle\n",fn));
		goto _return;
	}

	if ((h323callHandle->vendor == Vendor_eNortel) &&
		(confHandle->mediaOnHold != MEDIA_RESUME_200Ok_RX))
	{
		confHandle->mediaOnHold = MEDIA_RESUME_OLCACK_RX;
		confHandle->state = IWF_sReqMode;
		NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s IWF_sConnected --> IWF_sReqMode. Vendor Nortel. 200Ok not rx\n",fn));
	}
	else
	{
		confHandle->mediaOnHold = 0;
	}

	goto _return;

_dropCall:
	iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
	iwfSendRelComp(evtPtr->confID, h323CallID);

_return:
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
    H323FreeEvData(evtPtr->data);
    free(evtPtr);
    return 0;
}

/* Sends  200OK with SDP when Invite came in with no SDP */
int iwfSend200WithSDP(char *confID, char *sipCallID, RTPSet *localSet, int nlocalset)
{
    static char         fn[]    = "iwfSend200WithSDP";
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;

    NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering\n", fn));

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));
	
	pSipEvt->data = pSipData;	

	pSipData->nlocalset = nlocalset; 
	pSipData->localSet = (RTPSet *)(calloc(nlocalset, sizeof(RTPSet)));
   	memcpy(pSipData->localSet, localSet, nlocalset * sizeof(RTPSet));

	pSipData->maxForwards = sipmaxforwards;

	memcpy(pSipData->callID, sipCallID,CALL_ID_LEN);
	memcpy(pSipData->confID, confID,CONF_ID_LEN);
	memcpy(pSipEvt->confID, confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID, sipCallID,CALL_ID_LEN);

	pSipEvt->event = Sip_eBridge200;

	if(sipBridgeEventProcessor(pSipEvt) != 0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor failed\n",fn));
    	return -1;
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Sent 200Ok with SDP\n", fn));
	return 0;
}

/* For FS call from non-SET ep simply log the event
 * for SET endpoints switch over to Annex F state and send out
 * ReInvite (Hold SDP)
 */
int iwfCallConnectedCLC(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfCallConnectedCLC";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	SipAppCallHandle	*pSipData;
	SipCallHandle   	*sipCallHandle = NULL;
	char				h323CallID[CALL_ID_LEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	CallHandle			callHandleBlk;
	CallHandle			*callHandle;
	int					i;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pH323Data = (H323EventData *) evtPtr->data;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);

	if ((confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		getPeerCallIDFromConf(evtPtr->callID,confHandle,h323CallID);
	} 
	else 
	{
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		// can't do anything
		goto _return;
	}

	if (!(callHandle = CacheGet(callCache, h323CallID)))
	{
		NETERROR(MIWF,("%s Unable to locate H323 CallHandle\n",fn));
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		goto _return;
	}

	NETDEBUG(MIWF,NETLOG_DEBUG4, ("%s Received CLC from vendor %s \n", 
		fn, GetVendorDescription(callHandle->vendor) ));

	/* CLC from SET */
	if ((H323controlState(callHandle) == UH323_sControlIdle)
			&& (callHandle->vendor == Vendor_eAvaya))
	{
		confHandle->state = IWF_sAnnexF;
		confHandle->subState = AnnexF_sAwaiting_CLC_Ack;
	}
	else
	{
		CacheReleaseLocks(callCache);
		CacheReleaseLocks(confCache);
		iwfH323LogEvent(evtPtr);
		return 0;	
	}

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	pSipData->maxForwards = sipmaxforwards;


	if (CacheFind(callCache, evtPtr->callID, &callHandleBlk,sizeof(CallHandle))< 0)
	{
		NETERROR(MSCC,
			("%s Unable to locate sipCallHandle.Dropping Call\n",fn));
		iwfSendRelComp(evtPtr->confID,h323CallID);
		goto _return;
	}

	sipCallHandle = SipCallHandle(&callHandleBlk);

	if(sipCallHandle->remoteSet.remoteSet_len > 0)
	{
		/* Copy only the negotiated codec */
		pSipData->nlocalset = sipCallHandle->remoteSet.remoteSet_len;
		pSipData->localSet = (RTPSet *)(calloc(sipCallHandle->remoteSet.remoteSet_len, sizeof(RTPSet)));
		memcpy(pSipData->localSet,sipCallHandle->remoteSet.remoteSet_val,
								sipCallHandle->remoteSet.remoteSet_len*sizeof(RTPSet));
	}

	for (i = 0; i < sipCallHandle->remoteSet.remoteSet_len; i++)
	{
		/*
		   it's decision time. since this invite is generated
		   by the msw, we have to decide whether to follow rfc
		   3264 or rfc 2543 to implement sip hold.
		   making this a configurable so that poor msw does not
		   have to toss a coin.
		*/
		if(siphold3264)
		{
			pSipData->localSet[i].direction = SendOnly;
		}
		/* otherwise set IP address to 0.0.0.0 */
		else
		{
			pSipData->localSet[i].rtpaddr = 0;
		}
	}

	/*
	   set a:sendonly
	*/
	if(siphold3264)
	{
		pSipData->attr_count = 1;
		pSipData->attr = (SDPAttr *) malloc(sizeof(SDPAttr));
		pSipData->attr[0].name = strdup("sendonly");
		pSipData->attr[0].value = NULL;
		pSipData->attr[0].mLineNo = 0;
	}

	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, callHandle->vendor);

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));
	pSipEvt->event = Sip_eBridgeInvite;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CALL_ID_LEN);

	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;

		getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);
		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}

		CacheReleaseLocks(confCache);
		CacheReleaseLocks(callCache);
		return 0;
	}

_return:	
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
}


int iwfAnnexF200Ok(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfAnnexF200Ok";
	char				sipCallID[CALL_ID_LEN] = {0};
	char				stateString[IWF_STATE_STR];
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				evtStr[80];
	char				ipstr[24];
	char				confIDStr[CONF_ID_STRLEN] = {0};
	CallHandle 			callHandleBlk;
	ConfHandle			*confHandle;


	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	if (getPeerCallIDFromConf(evtPtr->callID, confHandle, sipCallID) < 0)
	{
		NETERROR(MSCC,("%s Unable to get SIP callID for confID %s\n", 
				fn, ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	if(iwfSendAck(evtPtr->confID,sipCallID,NULL,0) <0)
	{
		NETERROR(MIWF,("%s iwfSendAck Failed\n",fn));
		goto _return;
	}

	switch (confHandle->subState) 
	{
		case AnnexF_sAwaiting_OLC_Ack:
			{
				SipAppCallHandle	*pSipData;
				RTPSet				*localSet;
				SCC_EventBlock 		*h323evtPtr = (SCC_EventBlock *)NULL;

				confHandle->state  = IWF_sCallConnected;	

				pSipData = (SipAppCallHandle *) evtPtr->data;
				if (pSipData->nlocalset <= 0)
				{
					NETERROR(MIWF,("%s 200Ok had no SDP\n",fn));
					goto _return;
				}

				localSet = (RTPSet *)malloc(sizeof(RTPSet));
				*localSet = pSipData->localSet[0];

				NETDEBUG(MSCC,NETLOG_DEBUG4,("%s codec= %d param = %d ip = %s/%d\n",
					fn,localSet->codecType,localSet->param,FormatIpAddress(localSet->rtpaddr, ipstr),
					localSet->rtpport));
					if(iwfSendBridgeOLCAck(evtPtr->confID,evtPtr->callID,localSet,1) !=0)
				{
					NETERROR(MIWF,("%s iwfSendBridgeOLCAck Failed.\n",fn));
				}
				confHandle->subState = media_sConnected;
				/* check for any pending OLCs */
				if (h323evtPtr = (SCC_EventBlock *)DeQueueH323Evt(confHandle->h323EvtList))
				{
					/* Convert network event back to Bridge event*/
					h323evtPtr->event += SCC_eBridgeEventsMin;
					iwfH323EventProcessor(h323evtPtr);
				}
			}
			break;

		case AnnexF_sAwaiting_CLC_Ack:	
			{
				SCC_EventBlock 		*h323evtPtr = (SCC_EventBlock *)NULL;
				confHandle->subState = media_sNotConnected;
				/* check for any pending OLCs */
				if (h323evtPtr = (SCC_EventBlock *)DeQueueH323Evt(confHandle->h323EvtList))
				{
					/* Convert network event back to Bridge event*/
					h323evtPtr->event += SCC_eBridgeEventsMin;
					iwfH323EventProcessor(h323evtPtr);
				}
			} 
			break;
	}


_return:
	CacheReleaseLocks(callCache);
	CacheReleaseLocks(confCache);
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

int iwfAnnexFCLC(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfAnnexFCLC";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	char				ipStr[20];
	SipAppCallHandle	*pSipData;
	char				h323CallID[CALL_ID_LEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char				evtStr[80];
    SDPAttr 			*attr = NULL;
	CallHandle			*callHandle;
	SCC_EventBlock 		*h323evtPtr = (SCC_EventBlock *)NULL;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));
	/* Get conf handle */
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MIWF,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		iwfSendRelComp(evtPtr->confID, h323CallID);
		goto _return;
	}
	
	if ((confHandle->subState == AnnexF_sAwaiting_CLC_Ack) ||
		(confHandle->subState == AnnexF_sAwaiting_OLC_Ack))
	{
		SCC_EventBlock 			*evt = (SCC_EventBlock *)NULL;

		QueueH323Evt(&(confHandle->h323EvtList), (void *)evtPtr);
		NETDEBUG(MIWF,NETLOG_DEBUG4,
			("%s Received CLC When pending request. Queue and exit\n", fn));
		CacheReleaseLocks(confCache);
		return 0;
	}

	/* check for any pending OLC/CLC */
	if (h323evtPtr = (SCC_EventBlock *)DeQueueH323Evt(confHandle->h323EvtList))
	{
		/* Conevert network event back to Bridge event*/
		h323evtPtr->event += SCC_eBridgeEventsMin;
		iwfH323EventProcessor(h323evtPtr);
	}

	CacheReleaseLocks(confCache);
_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}


int iwfAnnexFOLC(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfAnnexFOLC";
    H323EventData       *pH323Data;
	SCC_EventBlock		*pSipEvt;
	char				ipStr[20];
	SipAppCallHandle	*pSipData;
	char				h323CallID[CALL_ID_LEN] = {0};
	char                callIDStr[CALL_ID_STRLEN] = {0};
	ConfHandle			*confHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	char				evtStr[80];
    SDPAttr 			*attr = NULL;
	CallHandle			*callHandle;

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	/* Get conf handle */
	CacheGetLocks(confCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MIWF,("%s Unable to locate ConfHandle %s\n",
			fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		iwfSendRelComp(evtPtr->confID, h323CallID);
		goto _return;
	}
	
	if ((confHandle->subState == AnnexF_sAwaiting_CLC_Ack) ||
		(confHandle->subState == AnnexF_sAwaiting_OLC_Ack))
	{
		SCC_EventBlock 			*evt = (SCC_EventBlock *)NULL;
		/* queue Event */
		/* If we have old pending event - free it */
		QueueH323Evt(&(confHandle->h323EvtList), (void *)evtPtr);

		NETDEBUG(MIWF,NETLOG_DEBUG4,
			("%s Received OLC When pending request. Queue and exit\n", fn));
		CacheReleaseLocks(confCache);
		return 0;
	}

	pH323Data = (H323EventData *) evtPtr->data;

	if(pH323Data->nlocalset <=0)
	{
		NETERROR(MSCC,("%s Received OLC without RTPSet %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
		CacheReleaseLocks(confCache);
		H323FreeEvData(evtPtr->data);
		free(evtPtr);
		return 0;
	}

	pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
	memset(pSipData,0,sizeof(SipAppCallHandle));

	getPeerCallID(evtPtr->confID,evtPtr->callID,h323CallID);

    CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
    if (!(callHandle = CacheGet(callCache, h323CallID)))
    {
        NETERROR(MIWF,("%s Unable to locate H323 CallHandle %s\n",
				fn, CallID2String(h323CallID, callIDStr)));
		iwfSendFinalResponse(evtPtr->confID, evtPtr->callID);
		CacheReleaseLocks(callCache);
		CacheReleaseLocks(confCache);
        goto _return;
    }

	NETDEBUG(MIWF,NETLOG_DEBUG4,
		("%s Received OLC from vendor %s with rtp port %d\n", 
		fn, GetVendorDescription(callHandle->vendor), 
		pH323Data->localSet[0].rtpport));



	confHandle->state = IWF_sAnnexF;
	confHandle->subState = AnnexF_sAwaiting_OLC_Ack;

	pSipData->nlocalset = pH323Data->nlocalset;
	if(pSipData->nlocalset >0)
	{
		pSipData->localSet = (RTPSet *)(calloc(pH323Data->nlocalset, sizeof(RTPSet)));
		memcpy(pSipData->localSet,pH323Data->localSet,pH323Data->nlocalset*sizeof(RTPSet));
		sanitizeCodecs4sip(pSipData,&pSipData->nlocalset, callHandle->vendor);
	}

	if(pSipData->localSet[0].codecType == T38Fax)
	{
		attr = (SDPAttr *) malloc(9*sizeof(SDPAttr));
		bzero(attr, 9*sizeof(SDPAttr));
		pSipData->attr_count = 9;
		pSipData->attr = attr;

		attr[0].name = strdup("T38FaxVersion");
		attr[0].value = strdup("0");
		attr[1].name = strdup("T38MaxBitRate");
		attr[1].value = strdup("14400");
		attr[2].name = strdup("T38FaxFillBitRemoval");
		attr[2].value = strdup("0");
		attr[3].name = strdup("T38FaxTranscodingMMR");
		attr[3].value = strdup("0");
		attr[4].name = strdup("T38FaxTranscodingJBIG");
		attr[4].value = strdup("0");
		attr[5].name = strdup("T38FaxRateManagement");
		attr[5].value = strdup("transferredTCF");
		attr[6].name = strdup("T38FaxMaxBuffer");
		attr[6].value = strdup("200");
		attr[7].name = strdup("T38FaxMaxDatagram");
		attr[7].value = strdup("72");
		attr[8].name = strdup("T38FaxUdpEC");
		attr[8].value = strdup("t38UDPRedundancy");
	}

	confHandle->h323MediaType = (pSipData->localSet[0].codecType == T38Fax)?
											CONF_MEDIA_TYPE_DATA:CONF_MEDIA_TYPE_VOICE;
	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Sending Invite with SDP\n", fn));

	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);
	pSipData->maxForwards = sipmaxforwards;

	pSipEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pSipEvt,0,sizeof(SCC_EventBlock));
	pSipEvt->event = Sip_eBridgeInvite;

	memcpy(pSipEvt->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(pSipEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);

	pSipEvt->data = (void *) pSipData;

	if(sipBridgeEventProcessor(pSipEvt) !=0)
	{
		NETERROR(MIWF,("%s sipBridgeEventProcessor Failed\n",fn));

		/* Send a release complete to leg1 */
		evtPtr->event = SCC_eBridgeReleaseComp;

		memcpy(evtPtr->callID,h323CallID,CALL_ID_LEN);
		H323FreeEvData(pH323Data);
		evtPtr->data = 0;
		/* Do not try to free evtPtr - we are passing its ownership */
		if(iwfSendH323Event(evtPtr)!=0)
		{
			// Too Bad - can't do anything else
			NETERROR(MIWF,("%s iwfSendH323Event Failed\n",fn));
		}
		return 0;
	}
	else
	{
		NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Sent invite to SIP\n", fn));
	}

_return:
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}



int iwfAnnexFReInvite(SCC_EventBlock *evtPtr)
{
	static char			fn[]	= "iwfAnnexFReInvite";
	char                callIDStr[CALL_ID_STRLEN] = {0};
	char				sipCallID[CALL_ID_LEN] = {0};
	ConfHandle			*confHandle;
	CallHandle			*callHandle;
	char				confIDStr[CONF_ID_STRLEN] = {0};
	RTPSet				*localSet = NULL;
	SipAppCallHandle	*pSipData = NULL;
	

	NETDEBUG(MIWF,NETLOG_DEBUG4,("%s Entering %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr)));

	pSipData = (SipAppCallHandle *)(evtPtr->data);

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
				fn,ConfID2String(evtPtr->confID,confIDStr)));
		goto _return;
	}

	if (getPeerCallIDFromConf(evtPtr->callID,confHandle,sipCallID) < 0)
	{
		NETERROR(MSCC,("%s Unable to get SIP callID for confID %s\n", 
					fn, ConfID2String(evtPtr->confID,confIDStr)));
		iwfSendRelComp(evtPtr->confID, evtPtr->callID);
		goto _return;
	}

	callHandle = CacheGet(callCache, evtPtr->callID);
	if (!callHandle)
	{
		NETERROR(MSCC,
			("%s Unable to locate sipCallHandle.Dropping Call\n",fn));
		iwfSendRelComp(evtPtr->confID, evtPtr->callID);
		goto _return;
	}


	/*
	 * to preserve backward compatibility, interpret both IP address
	 * set to 0.0.0.0 (rfc 2543) and a:sendonly (rfc 3264) as hold
	 */
	if(pSipData->nlocalset)
	{
		/*  call on hold */
		if ( (pSipData->localSet[0].direction == SendOnly) || (pSipData->localSet[0].rtpaddr == 0) )
		{
			if(iwfSendBridgeCLC(evtPtr->confID,evtPtr->callID, confHandle->h323MediaType) !=0 )
			{
				NETERROR(MIWF,("%s iwfSendBridgeCLC Failed.\n",fn));
				iwfSendRelComp(evtPtr->confID, evtPtr->callID);
				iwfSendFinalResponse(evtPtr->confID, sipCallID);
				goto _return;
			}

			/* Send 200 OK */
			iwfSend200OkHold(sipCallID);

		}
		/* media address change without call on hold */
		else if ( (pSipData->localSet[0].rtpaddr) && 
					(pSipData->localSet[0].codecType == 
						callHandle->handle.h323CallHandle.remoteSet[0].codecType) &&
				((pSipData->localSet[0].rtpaddr != 
						callHandle->handle.h323CallHandle.remoteSet[0].rtpaddr) ||
			     (pSipData->localSet[0].rtpaddr !=  
						callHandle->handle.h323CallHandle.remoteSet[0].rtpport)) )
		{
			if (H323remoteSet(callHandle)[0].rtpaddr)
			{
				if(iwfSendBridgeCLC(evtPtr->confID,evtPtr->callID, confHandle->h323MediaType) !=0 )
				{
					NETERROR(MIWF,("%s iwfSendBridgeCLC Failed.\n",fn));
					iwfSendRelComp(evtPtr->confID, evtPtr->callID);
					iwfSendFinalResponse(evtPtr->confID, sipCallID);
					goto _return;
				}
			}

			setDefaultRtpParams(pSipData->localSet,pSipData->nlocalset);

			localSet = (RTPSet *)malloc(sizeof(RTPSet));
			*localSet = pSipData->localSet[0];

			if(iwfSendBridgeOLC(evtPtr->confID,evtPtr->callID,localSet,1) !=0)
			{
				NETERROR(MIWF,("%s iwfSendBridgeOLC Failed.\n",fn));
			}

			/* Send 200 OK */
			iwfSend200Ok(sipCallID);
		}
		else if(pSipData->localSet[0].codecType == T38Fax)
		{
			/* send 501 not implemented */
			NETERROR(MIWF,("%s ReInvite with SDP codec = T.38 fax not implemented \n",fn));
		}

	}
	else
	{
		NETERROR(MIWF,("%s ReInvite with no SDP\n",fn));
	}
	
_return:
	CacheReleaseLocks(callCache);
	CacheReleaseLocks(confCache);
	SipFreeAppCallHandle(evtPtr->data);
	free(evtPtr);
	return 0;
}

int 
QueueH323Evt(List *list, void *item)
{
	int ret = -1;
	if (!(*list))
	 		*list = listInit();
	if (*list)
	{
#if 0
		/* not sure why are we dequeing these events */
		while (((*list)->head->nitems) > (IWF_H323QUEUE_SIZE -1))
		{
			SCC_EventBlock 			*evt = (SCC_EventBlock *)NULL;
			if (evt = (SCC_EventBlock *)DeQueueH323Evt(*list))
			{
				H323FreeEvData(evt->data);
				free(evt);
			}
		}
#endif
		ret = listAddItem(*list, item);
	}
	return ret;
}

void *
DeQueueH323Evt(List list)
{
	void 	*item = NULL;
	if (list && (list->head->nitems != 0))
	{
	 	item = listDeleteFirstItem(list);
	}
	return item;
}

void 
FreeH323EvtQueue(List *list)
{
	SCC_EventBlock 			*evt = (SCC_EventBlock *)NULL;
	if (*list)
	{
		while ((*list)->head->nitems && 
				(evt = (SCC_EventBlock *)DeQueueH323Evt(*list)) )
		{
			H323FreeEvData(evt->data);
			free(evt);
		}
		listDestroy(*list);
		*list = NULL;
	}
}

int 
matchOlcItem(const void *v1, const void *v2)
{
	SCC_EventBlock *h323evtPtr = (SCC_EventBlock *)v1;
	int			event;

	if (h323evtPtr == NULL)
	{
		return 0;
	}


	event = h323evtPtr->event + SCC_eBridgeEventsMin;
	if (event == SCC_eBridgeOLC)
	{
		return 1;	
	}

	return 0;
}

#if 0
void
iwfCheckPendingH323Events()
{
	if (h323evtPtr = (SCC_EventBlock *)DeQueueH323Evt(confHandle->h323EvtList))
	{
		/* Convert network event back to Bridge event*/
		h323evtPtr->event += SCC_eBridgeEventsMin;
		iwfH323EventProcessor(h323evtPtr);
	}
	return ;
}
#endif

