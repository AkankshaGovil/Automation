#include "gis.h"
#include "net.h"
#include <malloc.h>
#include <strings.h>
#include "nxosd.h"
#include "cacheinit.h"
#include "callid.h"
#include "callutils.h"
#include "ua.h"
#include "log.h"

/* overflow allowed */
static unsigned long sipCalls = 0;
char zeroCallID[CALL_ID_LEN] = { 0 };


void
SipStripUriParams(char * uri)
{
	char *p;

	if(uri)
	{
		if(p = strchr(uri, ';'))
		{
			*p = '\0';
		}
	}
}

void
SipStripTgrpParamFromUriName(char * name)
{
	char *p;

	if(name)
	{
		if(p = strstr(name, ";tgrp="))
		{
			*p = '\0';	
			if (p = strchr(p + 1, ';'))
			{
				strcat(name, p);
			}
		}
	}
}

SipAppMsgHandle *
SipAllocAppMsgHandle(void)
{
	char fn[] = "SipAllocAppMsgHandle():";
	SipAppMsgHandle *appMsgHandle;

	appMsgHandle = (SipAppMsgHandle *)malloc(sizeof(SipAppMsgHandle)); 

	if (!appMsgHandle)
	{
		NETERROR (MSIP, ("%s malloc failed!", fn));
		return NULL;
	}

	memset(appMsgHandle, 0, sizeof(SipAppMsgHandle));
        
        appMsgHandle->incomingPrivType = privacyTypeNone;
	appMsgHandle->privTranslate    = privTranslateNone;
	appMsgHandle->privLevel        = privacyLevelNone;


	return appMsgHandle;
}

SipMsgHandle *
SipAllocMsgHandle(void)
{
	SipMsgHandle *handle;

	handle = (SipMsgHandle *)malloc(sizeof(SipMsgHandle));
	memset(handle, 0, sizeof(SipMsgHandle));

	return handle;
}

SipEventHandle *
SipAllocEventHandle(void)
{
	SipEventHandle *handle;

	handle = (SipEventHandle *)malloc(sizeof(SipEventHandle));
	memset(handle, 0, sizeof(SipEventHandle));

	return handle;
}

int
SipFindAppCallID(SipMsgHandle *msgHandle, char *callID, char *confID)
{
	char fn[] = "SipUAFindAppCallID():";
	SipCallLegKey	callLeg;
	CallHandle *callHandle;
	int rc = -1;

	callLeg.callid = msgHandle->callid;
	callLeg.local = msgHandle->local;
	callLeg.remote = msgHandle->remote;
	
	CacheGetLocks(sipCallCache,LOCK_READ,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &callLeg);

	if (callHandle)
	{
		memcpy(callID, callHandle->callID, CALL_ID_LEN);
		memcpy(confID, callHandle->confID, CONF_ID_LEN);
		rc = 1;
	}

	CacheReleaseLocks(sipCallCache);

	return rc;
}

/* Initialize  the call handle from the event handle */
int
SipNetworkInitCallHandle(SipEventHandle *evHandle, CallHandle *callHandle)
{
	SipCallHandle *sipCallHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	SIP_S8bit *domain;
	int i;
	int herror;
	char rsadomain[24];
	char s1[24];
	char *p;

	if ((callHandle == NULL) || (evHandle == NULL))
	{
		return -1;
	}

	sipCallHandle = SipCallHandle(callHandle);
	appMsgHandle = SipEventAppHandle(evHandle);
	msgHandle = SipEventMsgHandle(evHandle);

	if (sipCallHandle == NULL)
	{
		return -1;
	}

	if(!callHandle->realmInfo)
	{
		callHandle->realmInfo = (CallRealmInfo  *)RealmInfoDup (appMsgHandle->realmInfo, callCache->malloc);
	}

	FormatIpAddress(callHandle->realmInfo->rsa, rsadomain);

	memcpy(callHandle->callID, appMsgHandle->callID, CALL_ID_LEN);
		
	/* Initialize the Sip portion of the call handle */

	/* Init call leg */
	SipNetworkInitCallLeg(msgHandle, &sipCallHandle->callLeg);

	/* Initialize the local session version */
	sipCallHandle->lsdpVersion = appMsgHandle->sdpVersion;

	/* Initialize Cseq numbers */
	sipCallHandle->remoteCSeqNo = 
		msgHandle->cseqno;
	sipCallHandle->localCSeqNo = 1;

	/* Initialize SDP stuff */
	sipCallHandle->localSet.localSet_len = appMsgHandle->nlocalset;
	if (appMsgHandle->nlocalset)
	{
		sipCallHandle->localSet.localSet_val = (RTPSet *)MMalloc(sipCallCache->malloc, 
			appMsgHandle->nlocalset*sizeof(RTPSet));
		memcpy(sipCallHandle->localSet.localSet_val, appMsgHandle->localSet,
			appMsgHandle->nlocalset*sizeof(RTPSet));

		callHandle->lastMediaIp = sipCallHandle->localSet.localSet_val[0].rtpaddr;
	}

	sipCallHandle->localAttr.localAttr_len = appMsgHandle->attr_count;
	if (appMsgHandle->attr_count)
	{
		sipCallHandle->localAttr.localAttr_val = (SDPAttr *)MMalloc(sipCallCache->malloc, 
			appMsgHandle->attr_count*sizeof(SDPAttr));
		for (i=0;i<appMsgHandle->attr_count;i++)
		{
			SipCDupAttrib(sipCallCache, &sipCallHandle->localAttr.localAttr_val[i], &appMsgHandle->attr[i]);
		}
	}

	/* Initialize the Route Set */
	sipCallHandle->routes.routes_len = msgHandle->nroutes;
	if (msgHandle->nroutes)
	{
		sipCallHandle->routes.routes_val = 
					(char **)MMalloc(sipCallCache->malloc,
					msgHandle->nroutes*sizeof(char *));
	}

	for (i=0;i<msgHandle->nroutes;i++)
	{
		sipCallHandle->routes.routes_val[i] =
			CStrdup(sipCallCache, msgHandle->routes[i]);
	}

	if (msgHandle->remotecontact)
	{
		/* Initialize contact */
		sipCallHandle->remoteContact = UrlDup(msgHandle->remotecontact,
									sipCallCache->malloc);
	}
	else
	{
		sipCallHandle->remoteContact = UrlDup(msgHandle->remote,
									sipCallCache->malloc);
	}
	
	// Fill in the ip address of remote in the Call Handle,
	// so that its available for CDRs
	callHandle->peerIp = callHandle->phonode.ipaddress.l = ResolveDNS(sipCallHandle->remoteContact->host, &herror);
	callHandle->phonode.realmId = appMsgHandle->realmInfo->realmId;
	BIT_SET(callHandle->phonode.sflags, ISSET_IPADDRESS);

	sipCallHandle->inrequri = UrlDup(msgHandle->requri,
									sipCallCache->malloc);

	/* Local contact will be the called pn@our domain */
	/* ensure that domain is our domain (NB do not split this block of code) */
	{
		domain = msgHandle->requri->host;
		msgHandle->requri->host = rsadomain;
		sipCallHandle->localContact = UrlDup(msgHandle->requri,
									sipCallCache->malloc);
		/* restore original domain */
		msgHandle->requri->host = domain;
	}
	SipStripUriParams(sipCallHandle->localContact->name);

	// this makes sure that correct contact is passed to leg2
	if(appMsgHandle->localContact)
	{
		UrlFree(appMsgHandle->localContact, sipCallCache->malloc);
	}
	appMsgHandle->localContact = UrlDup(sipCallHandle->localContact, sipCallCache->malloc);

	if((callHandle->tg = SipExtractParmVal(msgHandle->remote, "otg", NULL)))
	{
		callHandle->tg = CStrdup(callCache, callHandle->tg);
	}
	else if((callHandle->tg = SipExtractParmVal(msgHandle->remotecontact, "tgrp", NULL)))
	{
		// Remove quotes
		if((p = strchr(callHandle->tg, '"')) == callHandle->tg)
		{
			callHandle->tg = CStrdup(callCache, callHandle->tg + 1);
			if((p = strrchr(callHandle->tg, '"')))
			{
				*p = '\0';
			}
		}
		else
		{
			callHandle->tg = CStrdup(callCache, callHandle->tg);
		}
	}

	callHandle->destTg = SipExtractParmVal(msgHandle->requri, "dtg", NULL);
	if (callHandle->destTg)
	{
		callHandle->destTg = CStrdup(callCache, callHandle->destTg);
	}
	else
	{
		if(msgHandle->requri->name && (p = strstr(msgHandle->requri->name, ";tgrp=")))
		{
			callHandle->destTg = CStrdup(callCache, p + strlen(";tgrp="));
			*p = '\0';
			SipStripUriParams(sipCallHandle->inrequri->name);
		}
	}
	
	sipCallHandle->srchost = CStrdup(sipCallCache, msgHandle->srchost); 

	if (appMsgHandle->callingpn && appMsgHandle->callingpn->name)
	{
		nx_strlcpy(callHandle->phonode.phone, 
			SVal(appMsgHandle->callingpn->name),
			PHONE_NUM_LEN);
		BIT_SET(callHandle->phonode.sflags, ISSET_PHONE);
	}

	return 0;
}

int
SipNetworkInitCallLeg(SipMsgHandle *msgHandle, SipCallLegKey *leg)
{
	if ((leg == NULL) || (msgHandle == NULL))
	{
		return -1;
	}

	leg->callid = CStrdup(sipCallCache, msgHandle->callid);
	leg->local = UrlDup(msgHandle->local,
						sipCallCache->malloc);
	leg->remote = UrlDup(msgHandle->remote,
						sipCallCache->malloc);

	return 0;
}

CallHandle *
SipNetworkCreateCallHandle(SipEventHandle *evb)
{
	char fn[] = "SipNetworkCreateCallHandle():";
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle *callHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;

	appMsgHandle = SipEventAppHandle(evb);

	if (appMsgHandle == NULL)
	{
		NETERROR(MSIP, ("%s Null appMsg handle\n", fn));
		goto _error;
	}

	if (memcmp(appMsgHandle->callID, zeroCallID, CALL_ID_LEN) == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s callid not initialized, generating callid!\n", fn));

		generateCallId(appMsgHandle->callID);
	}

	msgHandle = SipEventMsgHandle(evb);

	/* There should be a from/to */
	if (SipValidateFrom(SipMsgFrom(msgHandle)) < 0)
	{
		NETERROR(MSIP, ("%s SipValidateFrom failed\n", fn));
		goto _error;
	}

	if (SipValidateTo(SipMsgTo(msgHandle)) < 0)
	{
		NETERROR(MSIP, ("%s SipValidateTo failed\n", fn));
		goto _error;
	}

	/* We have a valid appMsg Handle, now we need a valid
	 * call Handle
	 */

	callHandle = CacheGet(callCache, appMsgHandle->callID);
	if (callHandle == NULL)
	{
		// Allocate it
		callHandle = SipInitAppCallHandleForEvent(evb);
		if (callHandle == NULL)
		{
			NETERROR(MSIP, ("%s SipInitAppCallHandleForEvent failed\n", fn));
			goto _error;
		}
	}

	/* Init call handle from event handle */
	SipNetworkInitCallHandle(evb, callHandle);

	/* Init phone numbers CDR */
	if (strlen(SVal(msgHandle->remote->name)))
	{
		strncpy(callHandle->phonode.phone, msgHandle->remote->name, PHONE_NUM_LEN);
		callHandle->phonode.phone[PHONE_NUM_LEN -1] = '\0';
		BIT_SET(callHandle->phonode.sflags, ISSET_PHONE);
		strcpy(CallInputANI(callHandle), callHandle->phonode.phone);
	}

	if (strlen(SVal(msgHandle->requri->name)))
	{
		strncpy(callHandle->rfphonode.phone, msgHandle->requri->name, PHONE_NUM_LEN);
		callHandle->rfphonode.phone[PHONE_NUM_LEN -1] = '\0';
		SipStripUriParams(callHandle->rfphonode.phone);
		BIT_SET(callHandle->rfphonode.sflags, ISSET_PHONE);

		// The original input number is stored here
		strcpy(CallInputNumber(callHandle), callHandle->rfphonode.phone);

		strcpy(H323dialledNumber(callHandle), callHandle->rfphonode.phone);
	}

	callHandle->conf_id = CStrdup(callCache, msgHandle->callid);
	callHandle->incoming_conf_id = CStrdup(callCache, msgHandle->callid);

	callHandle->handle.sipCallHandle.maxForwards = appMsgHandle->maxForwards;

        /* Copy the privacy headers into Call Handle */

        if(appMsgHandle->incomingPrivType != privacyTypeNone)
        {
                appMsgHandle->original_from_hdr = UrlDup(SipMsgFrom(msgHandle), MEM_LOCAL);	  
        }
        

        if( appMsgHandle->pAssertedID_Sip )
        {
                callHandle->handle.sipCallHandle.pAssertedID_Sip = 
                        UrlDup( appMsgHandle->pAssertedID_Sip,sipCallCache->malloc);
                
        }
        if( appMsgHandle->pAssertedID_Tel )
        {
                callHandle->handle.sipCallHandle.pAssertedID_Tel =
                        strdup( appMsgHandle->pAssertedID_Tel );
        }
        if( appMsgHandle->original_from_hdr )
        {
                callHandle->handle.sipCallHandle.original_from_hdr = 
                        UrlDup( appMsgHandle->original_from_hdr,sipCallCache->malloc);
                
        }
        if( appMsgHandle->rpid_hdr )
        {
                callHandle->handle.sipCallHandle.rpid_hdr = 
                        UrlDup( appMsgHandle->rpid_hdr,sipCallCache->malloc);
                
        }
        if( appMsgHandle->rpid_url )
        {
                callHandle->handle.sipCallHandle.rpid_url =
                        CStrdup(sipCallCache,appMsgHandle->rpid_url);
        }
        if( appMsgHandle->priv_value )
        {
                callHandle->handle.sipCallHandle.priv_value =
                        CStrdup(sipCallCache, appMsgHandle->priv_value );
        }
        if( appMsgHandle->proxy_req_hdr )
        {
            callHandle->handle.sipCallHandle.proxy_req_hdr =
                    CStrdup(sipCallCache, appMsgHandle->proxy_req_hdr);
        }    

        callHandle->handle.sipCallHandle.incomingPrivType = appMsgHandle->incomingPrivType; 
        callHandle->handle.sipCallHandle.privLevel        = appMsgHandle->privLevel       ;


	/* Insert into sipcallcache also */
	if (CacheInsert(sipCallCache, callHandle) < 0)
	{
		NETERROR(MSIP, ("%s Could not insert callid in sip call cache\n", fn));
		goto _error;
	}


	return callHandle;

_error:

	if (callHandle)
	{
		CallDelete(callCache, callHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);
		CFree(callCache)(callHandle);
	}

	return NULL;
}

/* Initialize  the call handle from the event handle */
int
SipNetworkUpdateCallData(SipEventHandle *evHandle, CallHandle *callHandle)
{
	char fn[] = "SipNetworkUpdateCallData():";
	SipCallHandle *sipCallHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	int i;
	int herror;
	int	rc;

	if ((callHandle == NULL) || (evHandle == NULL))
	{
		return -1;
	}

	sipCallHandle = SipCallHandle(callHandle);
	appMsgHandle = SipEventAppHandle(evHandle);
	msgHandle = SipEventMsgHandle(evHandle);

	if (sipCallHandle == NULL)
	{
		return -1;
	}

	/* Initialize Cseq numbers */
	sipCallHandle->remoteCSeqNo = msgHandle->cseqno;

	rc = AssignLocalSDPToCallHandle(evHandle, callHandle);
	// See if the SDP has changed
	if (sipCallHandle->successfulInvites > 0)
	{
		appMsgHandle->mediaChanged =  rc;
	}

	if (appMsgHandle->mediaChanged)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s media change detected\n", fn));
	}

	// UPDATE the route set if needed
	// Route set can be updated only the first time
	if ((sipCallHandle->routes.routes_val == NULL) &&
		(sipCallHandle->successfulInvites == 0) &&
			(evHandle->event == Sip_eNetwork200))
	{
		// allocate
		sipCallHandle->routes.routes_len = msgHandle->nroutes;
		if (sipCallHandle->routes.routes_len)
		{
			sipCallHandle->routes.routes_val = 
					(char **)MMalloc(sipCallCache->malloc,
					msgHandle->nroutes*sizeof(char *));
		}
		for (i=0;i<msgHandle->nroutes;i++)
		{
			sipCallHandle->routes.routes_val[i] =
				CStrdup(sipCallCache, msgHandle->routes[i]);
		}
	}

	// See if contact address has changed
	// and if the final response before refreshing contact
	if (((!sipCallHandle->remoteContact && msgHandle->remotecontact) ||
		SipCompareUrls(sipCallHandle->remoteContact, 
			msgHandle->remotecontact)) &&
		evHandle->event == Sip_eNetwork200)
	{
		if (SipInitializeContact(callHandle, msgHandle) < 0)
		{
			NETERROR(MSIP, ("%s SipInitializeContact failed\n", fn));
		}

		// Fill in the ip address of remote in the Call Handle,
		// so that its available for CDRs
		callHandle->peerIp = callHandle->phonode.ipaddress.l = 
			ResolveDNS(sipCallHandle->remoteContact->host, &herror);
		BIT_SET(callHandle->phonode.sflags, ISSET_IPADDRESS);
	}

	/* Update session timer parameters  */
	callHandle->timerSupported = appMsgHandle->timerSupported;
	if (callHandle->timerSupported) 
	{
		callHandle->minSE = appMsgHandle->minSE;
		callHandle->sessionExpires = appMsgHandle->sessionExpires;
		callHandle->refresher = appMsgHandle->refresher;
	}
	
	return 0;
}

CallHandle *
SipNetworkUpdateCallHandle(SipEventHandle *evb)
{
	char fn[] = "SipNetworkUpdateCallHandle():";
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle *callHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;

	appMsgHandle = SipEventAppHandle(evb);

	if (appMsgHandle == NULL)
	{
		NETERROR(MSIP, ("%s Null appMsg handle\n", fn));
		goto _error;
	}

	msgHandle = SipEventMsgHandle(evb);

	/* There should be a from/to */
	if (SipValidateFrom(SipMsgFrom(msgHandle)) < 0)
	{
		NETERROR(MSIP, ("%s SipValidateFrom failed\n", fn));
		goto _error;
	}

	if (SipValidateTo(SipMsgTo(msgHandle)) < 0)
	{
		NETERROR(MSIP, ("%s SipValidateTo failed\n", fn));
		goto _error;
	}

	/* We have a valid appMsg Handle, now we need a valid
	 * call Handle
	 */

	callHandle = CacheGet(callCache, appMsgHandle->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSIP, ("%s No call handle exists\n", fn));
		goto _error;
	}

	/* Init call handle from event handle */
	SipNetworkUpdateCallData(evb, callHandle);

	callHandle->handle.sipCallHandle.maxForwards = appMsgHandle->maxForwards;

	return callHandle;

_error:

	return NULL;
}

/* Initialize the call handle and the sip portion of it */
int
SipBridgeInitCallHandle(SipEventHandle *evHandle, CallHandle *callHandle)
{
	char fn[] ="SipBridgeInitCallHandle()";
	SipCallHandle *sipCallHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	char rsadomain[24], *rsadomainp = "";
	int i;
	char s1[24];
	char *p, tgrp[256];
	char tmp[PHONE_NUM_LEN];

	if ((callHandle == NULL) || (evHandle == NULL))
	{
		return -1;
	}

	if (callHandle->realmInfo->rsa)
	{
		FormatIpAddress(callHandle->realmInfo->rsa, rsadomain);
		rsadomainp = rsadomain;
	}
	else if (callHandle->realmInfo->sipdomain)
	{
		rsadomainp = callHandle->realmInfo->sipdomain;
	}
	else
	{
		NETERROR(MSIP, ("%s no sip domain determined\n", fn));
		return -1;
	}

	sipCallHandle = SipCallHandle(callHandle);
	appMsgHandle = SipEventAppHandle(evHandle);

	if (sipCallHandle == NULL)
	{
		return -1;
	}

	memcpy(callHandle->callID, appMsgHandle->callID, CALL_ID_LEN);

	/* change the host of contact to us */
	if (appMsgHandle->localContact && appMsgHandle->localContact->host)
	{
		MFree(sipCallCache->free, appMsgHandle->localContact->host);
		appMsgHandle->localContact->host = strdup(rsadomainp);
	}

	/* If vendor is Sonus for destination endpoint and if attribute is "annexb", change values to "annexb:"*/
	if (callHandle->vendor == Vendor_eSonusGSX)
	{
		if (appMsgHandle->attr_count)
		{
			for (i=0;i<appMsgHandle->attr_count;i++)
			{	
				if(appMsgHandle->attr[i].value && strstr(appMsgHandle->attr[i].value, "annexb=no"))
				{
					SipCheckFree(appMsgHandle->attr[i].value);
					appMsgHandle->attr[i].value = strdup("18 annexb:no");

				}
				else
				{
					if(appMsgHandle->attr[i].value && strstr(appMsgHandle->attr[i].value, "annexb=yes"))
					{
						SipCheckFree(appMsgHandle->attr[i].value);
						appMsgHandle->attr[i].value = strdup("18 annexb:yes");
					}
				}	
			}
		}
	}

	SipBridgeInitSipCallHandle(evHandle, sipCallHandle);

	if(callHandle->ogprefix)
	{
		if(sipCallHandle->callLeg.remote->name)
		{
			nx_strlcpy(tmp, callHandle->ogprefix, PHONE_NUM_LEN);
			nx_strlcat(tmp, sipCallHandle->callLeg.remote->name, PHONE_NUM_LEN);
			MFree(sipCallCache->free, sipCallHandle->callLeg.remote->name);
			sipCallHandle->callLeg.remote->name = CStrdup(sipCallCache, tmp);
		}

		if(sipCallHandle->callLeg.remote->display_name)
		{
			nx_strlcpy(tmp, callHandle->ogprefix, PHONE_NUM_LEN);
			nx_strlcat(tmp, sipCallHandle->callLeg.remote->display_name, PHONE_NUM_LEN);
			MFree(sipCallCache->free, sipCallHandle->callLeg.remote->display_name);
			sipCallHandle->callLeg.remote->display_name = CStrdup(sipCallCache, tmp);
		}
	}

	// check if any from parameters need to be added
	if (callHandle->tg)
	{
		if(callHandle->vendor == Vendor_eSonusGSX)
		{
			SipAddParm(sipCallHandle->callLeg.local, "otg", callHandle->tg, sipCallCache->malloc);
		}
		else
		{
			snprintf(tgrp, sizeof(tgrp), "\"%s\"", callHandle->tg);
			SipAddParm(sipCallHandle->localContact, "tgrp", tgrp, sipCallCache->malloc);
		}
	}

	// check to see if any requri parms need to be added here
	if (callHandle->destTg && callHandle->ecaps1 & ECAPS1_SETDESTTG)
	{
		if(callHandle->vendor == Vendor_eSonusGSX)
		{
			SipAddParm(sipCallHandle->requri, "dtg", callHandle->destTg, sipCallCache->malloc);
		}
		else
		{
			snprintf(tgrp, sizeof(tgrp), "%s;tgrp=%s", sipCallHandle->requri->name, callHandle->destTg);
			MFree(sipCallCache->free, sipCallHandle->requri->name);
			sipCallHandle->requri->name = CStrdup(sipCallCache, tgrp);
		}
	}

	return 0;
}

/* Initialize the call handle and the sip portion of it */
int
SipBridgeInitSipCallHandle(SipEventHandle *evHandle, SipCallHandle *sipCallHandle)
{
	SipAppMsgHandle *appMsgHandle = NULL;
	int i;

	appMsgHandle = SipEventAppHandle(evHandle);

	/* Init call leg */
	SipBridgeInitCallLeg(SipEventAppHandle(evHandle), 
		&sipCallHandle->callLeg);

	/* Initialize the remote session version */
	sipCallHandle->rsdpVersion = appMsgHandle->sdpVersion;

	/* Initialize Cseq numbers */
	sipCallHandle->remoteCSeqNo = -1;
	sipCallHandle->localCSeqNo = 1;

	/* Initialize SDP stuff */
	sipCallHandle->remoteSet.remoteSet_len = appMsgHandle->nlocalset;
	if (appMsgHandle->nlocalset)
	{
		sipCallHandle->remoteSet.remoteSet_val = (RTPSet *)MMalloc(sipCallCache->malloc,
			appMsgHandle->nlocalset*sizeof(RTPSet));
		memcpy(sipCallHandle->remoteSet.remoteSet_val, appMsgHandle->localSet,
			appMsgHandle->nlocalset*sizeof(RTPSet));
	}

	if (appMsgHandle->attr_count)
	{
		sipCallHandle->remoteAttr.remoteAttr_len = appMsgHandle->attr_count;
		sipCallHandle->remoteAttr.remoteAttr_val = (SDPAttr *)MMalloc(sipCallCache->malloc, 
			appMsgHandle->attr_count*sizeof(SDPAttr));
		for (i=0;i<appMsgHandle->attr_count;i++)
		{
			SipCDupAttrib(sipCallCache, &sipCallHandle->remoteAttr.remoteAttr_val[i], &appMsgHandle->attr[i]);
		}
	}

	// if we are not doing media routing and there is a valid address
	// in the SDP, use that
	if(appMsgHandle->pOriginIpAddress)
	{
		SipCheckFree2(sipCallHandle->pOriginIpAddress, sipCallCache->free);
		sipCallHandle->pOriginIpAddress = CStrdup(sipCallCache, appMsgHandle->pOriginIpAddress);
	}
	// we already have a valid address, use that
	else if(sipCallHandle->pOriginIpAddress)
	{
		appMsgHandle->pOriginIpAddress = strdup(sipCallHandle->pOriginIpAddress);
	}
	// we are doing media routing, so just copy the address from the
	// SDP c line
	else if(appMsgHandle->nlocalset && appMsgHandle->localSet[0].rtpaddr)
	{
		char OriginIpAddress[16];
		FormatIpAddress(appMsgHandle->localSet[0].rtpaddr, OriginIpAddress);
		sipCallHandle->pOriginIpAddress = CStrdup(sipCallCache, OriginIpAddress);
		appMsgHandle->pOriginIpAddress = strdup(OriginIpAddress);
	}

	/* Initialize routes */
	sipCallHandle->routes.routes_len = 0;

	/* Initialize contact - keep it same as calling pn */
	if (appMsgHandle->localContact)
	{
		sipCallHandle->localContact = UrlDup(appMsgHandle->localContact, 
									sipCallCache->malloc);
	}
	else
	{
		sipCallHandle->localContact = UrlDup(appMsgHandle->callingpn, 
									sipCallCache->malloc);
	}

	// make sure that the contact name is correct
	if(sipCallHandle->localContact->name)
	{
		MFree(sipCallCache->free, sipCallHandle->localContact->name);
		sipCallHandle->localContact->name = CStrdup(sipCallCache, appMsgHandle->callingpn->name);
	}

	sipCallHandle->localContact->port = lSipPort;

	/* Copy the request uri */
	sipCallHandle->requri = UrlDup(appMsgHandle->requri,
								sipCallCache->malloc);

	// Strip the tgrp parameter from requri name of the 
	// incoming Invite.
	// It will be inserted again into the requri name
	// of the outgoing Invite, if "Set Dest. Trunk Group" 
	// is enabled.
	SipStripTgrpParamFromUriName(sipCallHandle->requri->name);

	return 0;
}

int
SipBridgeInitCallLeg(SipAppMsgHandle *appMsgHandle, 
	SipCallLegKey *leg)
{
	char *p;

	if ((leg == NULL) || (appMsgHandle == NULL))
	{
		return -1;
	}

	/* generate a new sip call id */
	leg->callid = SipGenerateCallID(NULL, CMalloc(sipCallCache));


	// Privacy related changes
        if(appMsgHandle->generate_cid == cid_unblock)
        {
                leg->local = UrlDup(appMsgHandle->callingpn,
                                    sipCallCache->malloc);               
                appMsgHandle->privLevel = privacyLevelNone;
        }
        else if(appMsgHandle->generate_cid == cid_block)
        {
                if(leg->local) {	    
                        UrlFree(leg->local,MEM_LOCAL);
                }
                leg->local = (header_url *) MMalloc(sipCallCache->malloc, sizeof(header_url));
                memset(leg->local, 0, sizeof(header_url));

                leg->local->display_name = CStrdup(sipCallCache,"Anonymous");
                leg->local->name         = CStrdup(sipCallCache,"anonymous");   
                if(appMsgHandle->dest_priv_type == privacyTypeRFC3325)
                {
                        leg->local->host = CStrdup(sipCallCache,"anonymous.invalid");
                } 
                else if (appMsgHandle->dest_priv_type == privacyTypeDraft01 || 
			 // To handle the case when destination has no privacy configured
			appMsgHandle->dest_priv_type == privacyTypeNone)
                {
                        leg->local->host = CStrdup(sipCallCache,"localhost");
                }      
        }
        else {
	if(appMsgHandle->incomingPrivType == privacyTypeNone ) {
	  leg->local = UrlDup(appMsgHandle->callingpn, 
			      sipCallCache->malloc);
	}
	else {

	  if(leg->local) {	    
	    UrlFree(leg->local,MEM_LOCAL);
	  }
	  leg->local = (header_url *) MMalloc(sipCallCache->malloc, sizeof(header_url));
	  memset(leg->local, 0, sizeof(header_url));

	  if(appMsgHandle->privTranslate == privTranslateRFC3325ToDraft01) {
	    if( appMsgHandle->privLevel == privacyLevelId) {

	      leg->local->display_name = CStrdup(sipCallCache,"Anonymous");
	      leg->local->name         = CStrdup(sipCallCache,"anonymous");
	      leg->local->host         = CStrdup(sipCallCache,"localhost");
	    }
	    else if(appMsgHandle->privLevel == privacyLevelNone) {

	      if(appMsgHandle->callingpn->display_name) {
		leg->local->display_name = CStrdup(sipCallCache,appMsgHandle->callingpn->display_name);
	      }
	      if(appMsgHandle->callingpn->name) {
		leg->local->name = CStrdup(sipCallCache,appMsgHandle->callingpn->name);
	      }
	      if(appMsgHandle->callingpn->host) {
		leg->local->host = CStrdup(sipCallCache,appMsgHandle->callingpn->host);
	      }
	    }
	  }
	  else if(appMsgHandle->privTranslate == privTranslateNone) {
#if 0
	  	leg->local = UrlDup(appMsgHandle->callingpn, 
			      sipCallCache->malloc);
#endif
		// No translation required. Copy the incoming from header to outgoing side

		if(appMsgHandle->original_from_hdr->display_name) {
			leg->local->display_name = CStrdup(sipCallCache,appMsgHandle->original_from_hdr->display_name);
		}
		if(appMsgHandle->original_from_hdr->name) 
		{
			// If any ANI plan modifications were applied, ANI might have changed as a result.
			if( strcasecmp(appMsgHandle->original_from_hdr->name,"anonymous") != 0 ) 
			{
				leg->local->name = CStrdup(sipCallCache,appMsgHandle->callingpn->name);
			}
			else
			{           
				leg->local->name = CStrdup(sipCallCache,appMsgHandle->original_from_hdr->name);
			}
		}
		if(appMsgHandle->original_from_hdr->host) {

			if( strcasecmp(appMsgHandle->original_from_hdr->host, "localhost") != 0 ||
			  strcasecmp(appMsgHandle->original_from_hdr->host, "anonymous.invalid") != 0 )
			{
			      leg->local->host = CStrdup(sipCallCache,appMsgHandle->callingpn->host);
			}
			else 
			{           
			      leg->local->host = CStrdup(sipCallCache,appMsgHandle->original_from_hdr->host);
			}
		}
	}
	  else if(appMsgHandle->privTranslate == privTranslateDraft01ToRFC3325) {
		  
		  // We should never enter this section.
		  // Currently we do not support draft to rfc conversion.
		  // Howerver if we do encounter such a case draft to rfc conversion cases will be treated
		  // as draft to draft case.

		  NETDEBUG(MSIP,NETLOG_DEBUG4,("SipBridgeInitCallLeg(): draft01 to rfc 3325 conversion indicated feature "));
		  // to avoid memory leak 
		  UrlFree(leg->local,MEM_LOCAL);
		  leg->local = NULL;
		  
		  // leg->local cannot be null 
		  leg->local = UrlDup(appMsgHandle->callingpn,sipCallCache->malloc);
	  }
	}
        }

	if(leg->remote)
	{
	    UrlFree(leg->remote, MEM_LOCAL);
	}

	leg->remote = UrlDup(appMsgHandle->calledpn,
					sipCallCache->malloc);

	// Set up display name for remote
	if ((leg->remote->display_name == NULL) && leg->remote->name)
	{
		leg->remote->display_name = CStrdup(sipCallCache, leg->remote->name);

		// remove any extraneous info
		if((p = strchr(leg->remote->display_name, ';')) != NULL)
		{
			*p = '\0';
		}
	}

	return 0;
}

/* allocate a sip call id and return it */
char *
SipGenerateCallID(char *insipdomain, void *(*mallocfn)(size_t))
{
	char *callid = (char *)mallocfn(256);
	struct timeval tv;

	gettimeofday(&tv, NULL);

	/* When we generate the callid, we just use what we
	 * have in the callHandle
	 */
	sprintf(callid, "%lu-%lu-%lu", ++sipCalls, 
			(tv.tv_sec + 2208988800u), tv.tv_usec);
	strcat(callid, "@");
	strcat(callid, insipdomain?insipdomain:sipdomain);

	return callid;
}

/* Must be lock protected */
CallHandle *
SipBridgeCreateCallHandle(SipEventHandle *evb)
{
	char fn[] = "SipBridgeCreateCallHandle():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle *callHandle = NULL;
	int i, j, k;

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	if (appMsgHandle == NULL)
	{
		NETERROR(MSIP, ("%s Null appMsg handle\n", fn));
		goto _error;
	}

	if (memcmp(appMsgHandle->callID, zeroCallID, CALL_ID_LEN) == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s no callid found, generating callid\n", fn));

		generateCallId(appMsgHandle->callID);
	}

	/* There should be a called pn */
	if (SipValidateFrom(appMsgHandle->callingpn) < 0)
	{
		NETERROR(MSIP, ("%s SipValidateFrom failed\n", fn));
		goto _error;
	}

	if (SipValidateTo(appMsgHandle->calledpn) < 0)
	{
		NETERROR(MSIP, ("%s SipValidateFrom failed\n", fn));
		goto _error;
	}

	/* We have a valid appMsg Handle, now we need a valid
	 * call Handle
	 */

	callHandle = CacheGet(callCache, appMsgHandle->callID);
	if (callHandle == NULL)
	{
		// Allocate it
		callHandle = SipInitAppCallHandleForEvent(evb);
		if (callHandle == NULL)
		{
			NETERROR(MSIP, ("%s SipInitAppCallHandleForEvent failed\n", fn));
			goto _error;
		}
	}

	if(callHandle->ecaps1 & ECAPS1_DELTCSRFC2833)
	{
		for(i = 0; i < appMsgHandle->nlocalset; i++)
		{
			if((int)appMsgHandle->localSet[i].codecType == 101)
			{
				if(--appMsgHandle->nlocalset > 0)
				{
					for(k = i; k < appMsgHandle->nlocalset; k++)
					{
						appMsgHandle->localSet[k] = appMsgHandle->localSet[k+1];
					}
				}
				else
				{
					SipCheckFree(appMsgHandle->localSet);
					appMsgHandle->localSet = NULL;
				}

				--i;

				for(j = 0; j < appMsgHandle->attr_count; j++)
				{
					if(appMsgHandle->attr[j].value && 
							strstr(appMsgHandle->attr[j].value, "101") == appMsgHandle->attr[j].value)
					{
						SipCheckFree(appMsgHandle->attr[j].name);
						SipCheckFree(appMsgHandle->attr[j].value);
						if(--appMsgHandle->attr_count > 0)
						{
							for(k = j; k < appMsgHandle->attr_count; k++)
							{
								memcpy(&appMsgHandle->attr[k], &appMsgHandle->attr[k+1], sizeof(SDPAttr));
							}
						}
						else
						{
							SipCheckFree(appMsgHandle->attr);
							appMsgHandle->attr = NULL;
						}

						--j;
					}
				}
			}
		}
	}

	/* Init call handle from event handle */
	SipBridgeInitCallHandle(evb, callHandle);

	/* Init phone numbers CDR */
	if (strlen(SVal(appMsgHandle->callingpn->name)))
	{
		strncpy(callHandle->rfphonode.phone, appMsgHandle->callingpn->name, PHONE_NUM_LEN);
		callHandle->rfphonode.phone[PHONE_NUM_LEN -1] = '\0';
		BIT_SET(callHandle->rfphonode.sflags, ISSET_PHONE);
	}

	if (strlen(SVal(appMsgHandle->calledpn->name)))
	{
		nx_strlcpy(callHandle->phonode.phone, appMsgHandle->calledpn->name, PHONE_NUM_LEN);
		BIT_SET(callHandle->phonode.sflags, ISSET_PHONE);
		strcpy(H323dialledNumber(callHandle), callHandle->phonode.phone);
	}
	callHandle->minSE = sipminSE;
	callHandle->sessionExpires = sipsessionexpiry;
	callHandle->refresher = SESSION_REFRESHER_UAC;

	/* Insert into sipcallcache also */
	if (CacheInsert(sipCallCache, callHandle) < 0)
	{
		NETERROR(MSIP, ("%s Could not insert callid in sip call cache\n", fn));
		goto _error;
	}

	/* We need to generate a tag if we havent already done so */
	SipCreateTags(callHandle); 

	return callHandle;

_error:

	if (callHandle)
	{
		CallDelete(callCache, callHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);
		CFree(callCache)(callHandle);
	}

	return NULL;
}

/* If event handle does not contain any SDP,
 * it is not considered unequal from whatever is there in
 * the call handle
 */
int
AssignLocalSDPToCallHandle(SipEventHandle *evHandle, CallHandle *callHandle)
{
	SipCallHandle *sipCallHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	int mediaFlags = 0, i, j, codecMatch;

	sipCallHandle = SipCallHandle(callHandle);
	appMsgHandle = SipEventAppHandle(evHandle);

	if (appMsgHandle->nlocalset == 0)
	{
		return mediaFlags;
	}

	if (sipCallHandle->localSet.localSet_len == 0)
	{
		// New SDP has arrived or number of codecs have increased
		mediaFlags |= MEDIA_ALL;
		goto _return;
	}

	if (appMsgHandle->sdpVersion != sipCallHandle->lsdpVersion)
	{
		mediaFlags |= MEDIA_VERSION;
		sipCallHandle->lsdpVersion = appMsgHandle->sdpVersion;
	}

	if (appMsgHandle->nlocalset == sipCallHandle->localSet.localSet_len)
	{
		if (memcmp(sipCallHandle->localSet.localSet_val, appMsgHandle->localSet,
				appMsgHandle->nlocalset*sizeof(RTPSet)) == 0)
		{
			// They are equal
			return mediaFlags;
		}
	}

	// At this point we have determined that the codecs are equal 
	// or lesser in number than the previous set of codecs.
	// Now we need to see if the Reinvite SDP has the same codecs 
	// or at least a subset of the codecs that were in the Invite.
	// If a matching codec is found, we should check if its transport
	// parameters have changed.

	for (i=0;i<appMsgHandle->nlocalset; i++)
	{
		for (j=0, codecMatch = 0; 
			(codecMatch == 0) && (j<sipCallHandle->localSet.localSet_len); 
			j++)
		{
			if ((sipCallHandle->localSet.localSet_val[j].codecType ==
				appMsgHandle->localSet[i].codecType) &&
				(sipCallHandle->localSet.localSet_val[j].param ==
				appMsgHandle->localSet[i].param))
			{
				// Codec is present
				codecMatch = 1;
				if ((sipCallHandle->localSet.localSet_val[j].rtpaddr !=
					appMsgHandle->localSet[i].rtpaddr) ||
					(sipCallHandle->localSet.localSet_val[j].rtpport !=
					appMsgHandle->localSet[i].rtpport) ||
					(sipCallHandle->localSet.localSet_val[j].direction !=
					appMsgHandle->localSet[i].direction))
				{
					// Transport parameters for codec have changed
					mediaFlags |= MEDIA_TRANSPORT;
					break;
				}
			}
		}

		// Codec is not present
		if (codecMatch == 0)
		{
			mediaFlags |= MEDIA_CODEC;
			break;
		}
	}

	// If codecs are a subset of existing codecs and transport
	// parameters also match, do not update localset
	if (!(mediaFlags & MEDIA_CODEC) &&
		!(mediaFlags & MEDIA_TRANSPORT))
	{
		return mediaFlags;
	}

_return:

	// Assign
	if (sipCallHandle->localSet.localSet_val)
	{
		MFree(sipCallCache->free, sipCallHandle->localSet.localSet_val);
		sipCallHandle->localSet.localSet_val = NULL;
		sipCallHandle->localSet.localSet_len = 0;
	}

	if (appMsgHandle->nlocalset)
	{
		sipCallHandle->localSet.localSet_val = 
			(RTPSet *)MMalloc(sipCallCache->malloc,
							appMsgHandle->nlocalset*sizeof(RTPSet));
		sipCallHandle->localSet.localSet_len = appMsgHandle->nlocalset;
		memcpy(sipCallHandle->localSet.localSet_val, appMsgHandle->localSet,
				appMsgHandle->nlocalset*sizeof(RTPSet));

		callHandle->lastMediaIp = sipCallHandle->localSet.localSet_val[0].rtpaddr;
	}

	if (sipCallHandle->localAttr.localAttr_len)
	{
		for (i=0;i<sipCallHandle->localAttr.localAttr_len;i++)
		{
			MFree(sipCallCache->free, sipCallHandle->localAttr.localAttr_val[i].name);
			MFree(sipCallCache->free, sipCallHandle->localAttr.localAttr_val[i].value);
		}
		MFree(sipCallCache->free, sipCallHandle->localAttr.localAttr_val);
		sipCallHandle->localAttr.localAttr_val = NULL;
		sipCallHandle->localAttr.localAttr_len = 0;
	}

	if (appMsgHandle->attr_count)
	{
		sipCallHandle->localAttr.localAttr_len = appMsgHandle->attr_count;
		sipCallHandle->localAttr.localAttr_val = (SDPAttr *)MMalloc(sipCallCache->malloc, 
			appMsgHandle->attr_count*sizeof(SDPAttr));
		for (i=0;i<appMsgHandle->attr_count;i++)
		{
			SipCDupAttrib(sipCallCache, &sipCallHandle->localAttr.localAttr_val[i], &appMsgHandle->attr[i]);
		}
	}

	return mediaFlags;
}

int
AssignRemoteSDPToCallHandle(SipEventHandle *evHandle, CallHandle *callHandle)
{
	SipCallHandle *sipCallHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	int mediaFlags = 0, i, j, codecMatch;

	sipCallHandle = SipCallHandle(callHandle);
	appMsgHandle = SipEventAppHandle(evHandle);

	if (appMsgHandle->nlocalset == 0)
	{
		return mediaFlags;
	}

	if (appMsgHandle->nlocalset > sipCallHandle->remoteSet.remoteSet_len)
	{
		mediaFlags |= MEDIA_ALL;
		
		goto _return;
	}

#if 0
	if (appMsgHandle->nlocalset == sipCallHandle->remoteSet.remoteSet_len)
	{
		if (memcmp(sipCallHandle->remoteSet.remoteSet_val, appMsgHandle->localSet,
				appMsgHandle->nlocalset*sizeof(RTPSet)) == 0)
		{
			// They are equal
			return mediaFlags;
		}
	}
#endif
	// At this point we have determined that the codecs are equal 
	// or lesser in number than the previous set of codecs.
	// Now we need to see if the Reinvite SDP has the same codecs 
	// or at least a subset of the codecs that were in the Invite.
	// If a matching codec is found, we should check if its transport
	// parameters have changed.

	for (i=0;i<appMsgHandle->nlocalset; i++)
	{
		for (j=0, codecMatch = 0; 
			(codecMatch == 0) && (j<sipCallHandle->remoteSet.remoteSet_len); 
			j++)
		{
			if ((sipCallHandle->remoteSet.remoteSet_val[j].codecType ==
				appMsgHandle->localSet[i].codecType) &&
				(sipCallHandle->remoteSet.remoteSet_val[j].param ==
				appMsgHandle->localSet[i].param))
			{
				// Codec is present
				codecMatch = 1;
				if ((sipCallHandle->remoteSet.remoteSet_val[j].rtpaddr !=
					appMsgHandle->localSet[i].rtpaddr) ||
					(sipCallHandle->remoteSet.remoteSet_val[j].rtpport !=
					appMsgHandle->localSet[i].rtpport) ||
					(sipCallHandle->remoteSet.remoteSet_val[j].direction !=
					appMsgHandle->localSet[i].direction))
				{
					// Transport parameters for codec have changed
					mediaFlags |= MEDIA_TRANSPORT;
					break;
				}
			}
		}

		// Codec is not present
		if (codecMatch == 0)
		{
			mediaFlags |= MEDIA_CODEC;
			break;
		}
	}

	// If codecs are a subset of existing codecs and transport
	// parameters also match, do not update remoteset
	if (!(mediaFlags & MEDIA_CODEC) &&
		!(mediaFlags & MEDIA_TRANSPORT))
	{
		appMsgHandle->sdpVersion = sipCallHandle->rsdpVersion;
		return mediaFlags;
	}

_return:
	// atleast one of codec or transport param has changed
	// check if version change was detected
	if (appMsgHandle->sdpVersion != sipCallHandle->rsdpVersion)
	{
		mediaFlags |= MEDIA_VERSION;
		sipCallHandle->rsdpVersion = appMsgHandle->sdpVersion;
	}


	// Assign
	if (sipCallHandle->remoteSet.remoteSet_val)
	{
		MFree(sipCallCache->free, sipCallHandle->remoteSet.remoteSet_val);
		sipCallHandle->remoteSet.remoteSet_val = NULL;
		sipCallHandle->remoteSet.remoteSet_len = 0;
	}

	if (appMsgHandle->nlocalset)
	{
		sipCallHandle->remoteSet.remoteSet_val = 
			(RTPSet *)MMalloc(sipCallCache->malloc,
							appMsgHandle->nlocalset*sizeof(RTPSet));
		sipCallHandle->remoteSet.remoteSet_len = appMsgHandle->nlocalset;
		memcpy(sipCallHandle->remoteSet.remoteSet_val, appMsgHandle->localSet,
				appMsgHandle->nlocalset*sizeof(RTPSet));
	}

	if (sipCallHandle->remoteAttr.remoteAttr_len)
	{
		for (i=0;i<sipCallHandle->remoteAttr.remoteAttr_len;i++)
		{
			MFree(sipCallCache->free, sipCallHandle->remoteAttr.remoteAttr_val[i].name);
			MFree(sipCallCache->free, sipCallHandle->remoteAttr.remoteAttr_val[i].value);
		}
		MFree(sipCallCache->free, sipCallHandle->remoteAttr.remoteAttr_val);
		sipCallHandle->remoteAttr.remoteAttr_val = NULL;
		sipCallHandle->remoteAttr.remoteAttr_len = 0;
	}

	if (appMsgHandle->attr_count)
	{
		sipCallHandle->remoteAttr.remoteAttr_len = appMsgHandle->attr_count;
		sipCallHandle->remoteAttr.remoteAttr_val = (SDPAttr *)MMalloc(sipCallCache->malloc, 
			appMsgHandle->attr_count*sizeof(SDPAttr));
		for (i=0;i<appMsgHandle->attr_count;i++)
		{
			SipCDupAttrib(sipCallCache, &sipCallHandle->remoteAttr.remoteAttr_val[i], &appMsgHandle->attr[i]);
		}
	}

	// if we are not doing media routing and there is a valid address
	// in the SDP, use that
	if(appMsgHandle->pOriginIpAddress)
	{
		SipCheckFree2(sipCallHandle->pOriginIpAddress, sipCallCache->free);
		sipCallHandle->pOriginIpAddress = CStrdup(sipCallCache, appMsgHandle->pOriginIpAddress);
	}
	// we already have a valid address, use that
	else if(sipCallHandle->pOriginIpAddress)
	{
		appMsgHandle->pOriginIpAddress = strdup(sipCallHandle->pOriginIpAddress);
	}
	// we are doing media routing, so just copy the address from the
	// SDP c line
	else if(appMsgHandle->nlocalset && appMsgHandle->localSet[0].rtpaddr)
	{
		char OriginIpAddress[16];
		FormatIpAddress(appMsgHandle->localSet[0].rtpaddr, OriginIpAddress);
		sipCallHandle->pOriginIpAddress = CStrdup(sipCallCache, OriginIpAddress);
		appMsgHandle->pOriginIpAddress = strdup(OriginIpAddress);
	}

	return mediaFlags;
}

int
SipBridgeUpdateCallData(SipEventHandle *evHandle, CallHandle *callHandle)
{
	SipCallHandle *sipCallHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	int i =0;

	if ((callHandle == NULL) || (evHandle == NULL))
	{
		return -1;
	}

	sipCallHandle = SipCallHandle(callHandle);
	appMsgHandle = SipEventAppHandle(evHandle);

	if (sipCallHandle == NULL)
	{
		return -1;
	}

	if (!(sipCallHandle->uaflags & UAF_LOCALINVITE))
	{
		// See if the SDP has changed
		appMsgHandle->mediaChanged = 
			AssignRemoteSDPToCallHandle(evHandle, callHandle);
	}
	
	/*If vendor is Sonus for dest. endpoint and if attribute is "annexb", change values to "annexb:"*/
    if (callHandle->vendor == Vendor_eSonusGSX)
    {
		if (appMsgHandle->attr_count)
		{
    		for (i=0;i<appMsgHandle->attr_count;i++)
        	{
        		if(appMsgHandle->attr[i].value && strstr(appMsgHandle->attr[i].value, "annexb=no"))
				{	
					SipCheckFree(appMsgHandle->attr[i].value);
                	appMsgHandle->attr[i].value  = strdup("18 annexb:no");
				}
            	else
            	{
            		if(appMsgHandle->attr[i].value && strstr(appMsgHandle->attr[i].value, "annexb=yes"))
					{
						SipCheckFree(appMsgHandle->attr[i].value);
                    	appMsgHandle->attr[i].value = strdup("18 annexb:yes");
					}
            	}
         	}
		}
	}

	if(appMsgHandle->pOriginIpAddress == NULL && sipCallHandle->pOriginIpAddress)
	{
		appMsgHandle->pOriginIpAddress = strdup(sipCallHandle->pOriginIpAddress);
	}

	return 0;
}

/* Must be lock protected */
CallHandle *
SipBridgeUpdateCallHandle(SipEventHandle *evb)
{
	char fn[] = "SipBridgeUpdateCallHandle():";
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle *callHandle = NULL;

	appMsgHandle = SipEventAppHandle(evb);

	if (appMsgHandle == NULL)
	{
		NETERROR(MSIP, ("%s Null appMsg handle\n", fn));
		goto _error;
	}

	/* We have a valid appMsg Handle, now we need a valid
	 * call Handle
	 */

	callHandle = CacheGet(callCache, appMsgHandle->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSIP, ("%s No call handle exists\n", fn));
		goto _error;
	}

	/* Init call handle from event handle */
	SipBridgeUpdateCallData(evb, callHandle);

	return callHandle;

_error:

	return NULL;
}

/* Caller should have already figured out that we
 * need an app call handle
 */
CallHandle *
SipInitAppCallHandleForEvent(SipEventHandle *evb)
{
	char fn[] = "SipInitAppCallHandleForEvent():";
	CallHandle *callHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;

	appMsgHandle = SipEventAppHandle(evb);

	callHandle = GisAllocCallHandle();	
	if (callHandle == NULL)
	{
		NETERROR(MSIP, ("%s Could not insert callid in cache\n", fn));
		goto _error;
	}

	/* Initialize the callid's involved in both the
	 * handle, event handle, and set the handle to be a sip handle
	 */
	callHandle->handleType = SCC_eSipCallHandle;

	generateCallId(appMsgHandle->callID);
	memcpy(callHandle->callID, appMsgHandle->callID, CALL_ID_LEN);

	generateConfId(appMsgHandle->confID);
	memcpy(callHandle->confID, appMsgHandle->confID, CONF_ID_LEN);

	/* Copy realm info */
	callHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (appMsgHandle->realmInfo, callCache->malloc);
	if (callHandle->realmInfo == NULL) 
	{
		NETERROR (MSIP, ("%s Could not duplicate realm info.",fn));
		goto _error;
	}

	if (CacheInsert(callCache, callHandle) < 0)
	{
		NETERROR(MSIP, ("%s Could not insert callid in call cache\n", fn));
		goto _error;
	}

_error:
	return callHandle;
}

SipMsgHandle *
SipBridgeCreateMsgHandle2(SipCallHandle *sipCallHandle, 
							char *method, int msgType, int respCode)
{
	SipMsgHandle *msgHandle = NULL;
	int i;

	msgHandle = SipAllocMsgHandle();
	
	if (msgHandle == NULL)
	{
		goto _error;
	}

	msgHandle->origin = 1;

	msgHandle->callid = strdup(sipCallHandle->callLeg.callid);
	msgHandle->local = UrlDup(sipCallHandle->callLeg.local, MEM_LOCAL);
	msgHandle->remote = UrlDup(sipCallHandle->callLeg.remote, MEM_LOCAL);

	msgHandle->requri = UrlDup(sipCallHandle->requri, MEM_LOCAL);
	msgHandle->localcontact = UrlDup(sipCallHandle->localContact, MEM_LOCAL);
	msgHandle->remotecontact = UrlDup(sipCallHandle->remoteContact, MEM_LOCAL);

	if (method)
	{
		msgHandle->method = strdup(method);
	}

	msgHandle->msgType = msgType;
	msgHandle->responseCode = respCode;

	if ((msgType == SIPMSG_REQUEST) || (msgType == SIPMSG_REQUESTL))
	{
		msgHandle->cseqno = sipCallHandle->localCSeqNo;
	}
	else
	{
		msgHandle->cseqno = sipCallHandle->remoteCSeqNo;
	}

	/* Initialize the routes */
	/* Initialize the Route Set */
	msgHandle->nroutes = sipCallHandle->routes.routes_len;
	if (msgHandle->nroutes)
	{
		msgHandle->routes = (char **)
				malloc(sipCallHandle->routes.routes_len*sizeof(char *));
	}

	for (i=0;i<sipCallHandle->routes.routes_len;i++)
	{
		msgHandle->routes[i] =
			strdup(sipCallHandle->routes.routes_val[i]);
	}

	return msgHandle;

_error:
	if (msgHandle)
	{
		free (msgHandle);
	}

	return NULL;
}

/* Last two args are optional */
SipMsgHandle *
SipBridgeCreateMsgHandle(CallHandle *callHandle, 
							char *method, int msgType, int respCode)
{
	SipMsgHandle *msgHandle = NULL;
	SipCallHandle *sipCallHandle = NULL;
	int i;

	sipCallHandle = SipCallHandle(callHandle);
	msgHandle = SipBridgeCreateMsgHandle2(sipCallHandle, method, msgType, respCode);

	if (msgHandle == NULL)
	{
		goto _error;
	}

	if (useXConnId)
	{
		msgHandle->xConnId = SipXConnIdFromCallID(callHandle->callID);
	}

	return msgHandle;

_error:
	if (msgHandle)
	{
		free (msgHandle);
	}

	return NULL;
}

SipAppMsgHandle *
SipCreateAppMsgHandleForCall(CallHandle *callHandle, header_url *newrequri)
{
	SipAppMsgHandle *appMsgHandle;
	SipCallHandle *sipCallHandle;
	int i;

	sipCallHandle = SipCallHandle(callHandle);
	
	if (sipCallHandle == NULL)
	{
		return NULL;
	}

	appMsgHandle = SipAllocAppMsgHandle();
	
	if (appMsgHandle == NULL)
	{
		return NULL;
	}

	appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	if (callHandle->callSource == 1)
	{
		// We must fill up the localset. The localset of the appHandle
		// will come from the call handle's remote set
		appMsgHandle->nlocalset = sipCallHandle->remoteSet.remoteSet_len;
		if (sipCallHandle->remoteSet.remoteSet_len)
		{
			appMsgHandle->localSet = 
				malloc(sipCallHandle->remoteSet.remoteSet_len*sizeof(RTPSet));

			memcpy(appMsgHandle->localSet, sipCallHandle->remoteSet.remoteSet_val,
				appMsgHandle->nlocalset*sizeof(RTPSet));
		}
	
		appMsgHandle->attr_count = sipCallHandle->remoteAttr.remoteAttr_len;
		if (sipCallHandle->remoteAttr.remoteAttr_len)
		{
			appMsgHandle->attr = 
				malloc(sipCallHandle->remoteAttr.remoteAttr_len*sizeof(SDPAttr));

			for (i=0;i<sipCallHandle->remoteAttr.remoteAttr_len;i++)
			{
				SipDupAttrib(&appMsgHandle->attr[i], &sipCallHandle->remoteAttr.remoteAttr_val[i]);
			}
		}
	
		appMsgHandle->requri = UrlDup(sipCallHandle->requri, MEM_LOCAL);
	}
	else
	{
		appMsgHandle->nlocalset = sipCallHandle->localSet.localSet_len;
		if (sipCallHandle->localSet.localSet_len)
		{
			appMsgHandle->localSet = 
				malloc(sipCallHandle->localSet.localSet_len*sizeof(RTPSet));

			memcpy(appMsgHandle->localSet, sipCallHandle->localSet.localSet_val,
				appMsgHandle->nlocalset*sizeof(RTPSet));
		}
	
		appMsgHandle->attr_count = sipCallHandle->localAttr.localAttr_len;
		if (sipCallHandle->localAttr.localAttr_len)
		{
			appMsgHandle->attr = 
				malloc(sipCallHandle->localAttr.localAttr_len*sizeof(SDPAttr));

			for (i=0;i<sipCallHandle->localAttr.localAttr_len;i++)
			{
				SipDupAttrib(&appMsgHandle->attr[i], &sipCallHandle->localAttr.localAttr_val[i]);
			}
		}
	
		appMsgHandle->callingpn = UrlDup(sipCallHandle->callLeg.remote, 
									MEM_LOCAL);

		appMsgHandle->calledpn = UrlDup(newrequri?newrequri:sipCallHandle->inrequri, MEM_LOCAL);
		SipStripUriParams(appMsgHandle->calledpn->name);
		appMsgHandle->requri = UrlDup(newrequri?newrequri:sipCallHandle->inrequri, MEM_LOCAL);

		memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
		memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);
	}

	if(sipCallHandle->pOriginIpAddress)
	{
		appMsgHandle->pOriginIpAddress = strdup(sipCallHandle->pOriginIpAddress);
	}

	return appMsgHandle;
}

// If this routine returns an error,
// send back an error to the UA
int
SipUASendToTSMWorker(SipEventHandle *evb)
{
	char fn[] = "SipUASendToTSMWorker():";
	SipAppMsgHandle *appMsgHandle = NULL, *newappMsgHandle = NULL;
	SipEventHandle *evHandle = NULL;
	CallHandle 		*callHandle = NULL;
	int rc = 1;

	appMsgHandle = SipEventAppHandle(evb);

	if (SipTransSendMsgHandle(appMsgHandle) != 0)
	{
		NETERROR(MSIP, 
			("%s SipTransSendMsgHandle returned error\n", fn));

		// Free up the app msg handle
		evHandle = SipEventHandleDup(evb);
		SipFreeAppMsgHandle(appMsgHandle);
		appMsgHandle = SipEventAppHandle(evHandle);

		// set up the error
		appMsgHandle->tsmError = tsmError_UNDEFINED;

		evHandle->type = Sip_eNetworkEvent;
		evHandle->event = Sip_eNetworkError;

		rc = SipUAProcessEvent(evHandle);

		// must report error
		rc = -1;
	}

	// Free the rest of the event handle
    SipEventAppHandle(evb) = NULL;
    SipFreeEventHandle(evb);

	// success
	return rc;
}

int
SipUASendToTSM(SipEventHandle *evb)
{
	char fn[] = "SipUASendToTSM():";

	if ( ThreadDispatch( poolid,
                         lpcid,
                         (PFVP)SipUASendToTSMWorker,
                         evb,
                         0,
                         PTHREAD_SCOPE_PROCESS,
                         SCHED_RR,
                         50) != 0)
	{
		NETERROR(MSIP, ("%s Could not launch worker\n", fn));
	}
	return(0);
}

#if 0
int
SipUASendToTSM(SipEventHandle *evb)
{
	char fn[] = "SipUASendToTSM():";

	if (ThreadLaunch2(SipUASendToTSMWorker, evb, 0,
			PTHREAD_SCOPE_PROCESS, SCHED_RR, 50, PTHREAD_DETACHED_STATE, NULL) != 0)
	{
		NETERROR(MSIP, ("%s Error in launching Worker\n", fn));
	}

	return 1;
}
#endif

int
SipFreeAppCallHandle(SipAppMsgHandle *appMsgHandle)
{
	return SipFreeAppMsgHandle(appMsgHandle);
}

int
SipFreeEventHandle(SipEventHandle *evb)
{
	SipAppMsgHandle *appMsgHandle;

	if (evb == NULL)
	{
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	SipFreeAppMsgHandle(appMsgHandle);

	free(evb);

	return 1;
}

int
SipFreeAppMsgHandle(SipAppMsgHandle *appMsgHandle)
{
	int i;

	if (appMsgHandle == NULL)
	{
		return 0;
	}

	/* Free the complete app msg handle */
	UrlFree(appMsgHandle->calledpn, MEM_LOCAL);
	UrlFree(appMsgHandle->callingpn, MEM_LOCAL);
	UrlFree(appMsgHandle->requri, MEM_LOCAL);
	UrlFree(appMsgHandle->localContact, MEM_LOCAL);
	
	SipFreeSipMsgHandle(appMsgHandle->msgHandle);
	
	SipCheckFree(appMsgHandle->localSet);
	SipCheckFree(appMsgHandle->pOriginIpAddress);

	for(i=0;i<appMsgHandle->attr_count;i++)
	{
		SipCheckFree( (appMsgHandle->attr)[i].name );
		SipCheckFree( (appMsgHandle->attr)[i].value );
	}
	SipCheckFree(appMsgHandle->attr);
	SipCheckFree (appMsgHandle->dtmf);

	SipCheckFree (appMsgHandle->isup_msg);
	SipCheckFree (appMsgHandle->isupTypeVersion);
	SipCheckFree (appMsgHandle->isupTypeBase);
	SipCheckFree (appMsgHandle->isupDisposition);
	SipCheckFree (appMsgHandle->isupHandling);

	SipCheckFree (appMsgHandle->qsig_msg);
	SipCheckFree (appMsgHandle->qsigTypeVersion);
	SipCheckFree (appMsgHandle->qsigDisposition);
	SipCheckFree (appMsgHandle->qsigHandling);

	/* free auth hdrs */
	SipCheckFree(appMsgHandle->hdrAuthorization);
	SipCheckFree(appMsgHandle->hdrProxyauthorization);
	SipCheckFree(appMsgHandle->hdrProxyauthenticate);
	SipCheckFree(appMsgHandle->hdrWwwauthenticate);

	SipCheckFree(appMsgHandle->responseMsgStr);
	RealmInfoFree(appMsgHandle->realmInfo, MEM_LOCAL);


	// Privacy related headers
	UrlFree(appMsgHandle->pAssertedID_Sip,MEM_LOCAL);
	UrlFree(appMsgHandle->original_from_hdr,MEM_LOCAL);
	UrlFree(appMsgHandle->rpid_hdr,MEM_LOCAL);

	SipCheckFree(appMsgHandle->rpid_url);
	SipCheckFree(appMsgHandle->priv_value);
	SipCheckFree(appMsgHandle->proxy_req_hdr);
        SipCheckFree(appMsgHandle->pAssertedID_Tel);

	// REFER - Call Transfer
	SipCheckFree(appMsgHandle->sip_frag);
	SipCheckFree(appMsgHandle->event);
	SipCheckFree(appMsgHandle->allow);
	SipCheckFree(appMsgHandle->sub_state);
	SipCheckFree(appMsgHandle->supported);
	SipCheckFree(appMsgHandle->content_type);

	for(i=0; i<appMsgHandle->nunkhdr;i++)
	{
		SipCheckFree( (appMsgHandle->unkhdrs)[i].name );
		SipCheckFree( (appMsgHandle->unkhdrs)[i].val );
	}
	SipCheckFree(appMsgHandle->unkhdrs);

	free(appMsgHandle);
	return(0);
}

// Does not free the sipcallhandle itself
int
SipFreeSipCallHandle(SipCallHandle *sipCallHandle, int freefn)
{
	int i;

	SipFreeSipCallKey(&sipCallHandle->callLeg, freefn);
	UrlFree(sipCallHandle->localContact, freefn);
	UrlFree(sipCallHandle->remoteContact, freefn);
	UrlFree(sipCallHandle->requri, freefn);
	UrlFree(sipCallHandle->inrequri, freefn);

	// Free more stuff over here
	SipCheckFree2(sipCallHandle->remoteSet.remoteSet_val, freefn);
	for (i=0;i<sipCallHandle->remoteAttr.remoteAttr_len;i++)
	{
		SipCheckFree2(sipCallHandle->remoteAttr.remoteAttr_val[i].name, freefn);
		SipCheckFree2(sipCallHandle->remoteAttr.remoteAttr_val[i].value, freefn);
	}
	SipCheckFree2(sipCallHandle->remoteAttr.remoteAttr_val, freefn);
	SipCheckFree2(sipCallHandle->localSet.localSet_val, freefn);
	for (i=0;i<sipCallHandle->localAttr.localAttr_len;i++)
	{
		SipCheckFree2(sipCallHandle->localAttr.localAttr_val[i].name, freefn);
		SipCheckFree2(sipCallHandle->localAttr.localAttr_val[i].value, freefn);
	}
	SipCheckFree2(sipCallHandle->localAttr.localAttr_val, freefn);
	SipCheckFree2(sipCallHandle->localAddSet.localAddSet_val, freefn);

	SipCheckFree2(sipCallHandle->pOriginIpAddress, freefn);

	// Free the routes
	for (i=0;i<sipCallHandle->routes.routes_len;i++)
	{
		SipCheckFree2(sipCallHandle->routes.routes_val[i], freefn);
	}

	if (sipCallHandle->routes.routes_val)
	{
		SipCheckFree2(sipCallHandle->routes.routes_val, freefn);
	}

	SipCheckFree2(sipCallHandle->srchost, freefn);


        // Free up privacy related stuff here
        
        UrlFree(sipCallHandle->pAssertedID_Sip, freefn);
        UrlFree(sipCallHandle->original_from_hdr, freefn);
        UrlFree(sipCallHandle->rpid_hdr, freefn);
        
        SipCheckFree2(sipCallHandle->rpid_url,freefn);
        SipCheckFree2(sipCallHandle->priv_value,freefn);
        SipCheckFree2(sipCallHandle->proxy_req_hdr,freefn);
        SipCheckFree2(sipCallHandle->pAssertedID_Tel,freefn);        

	return(0);
}

// Does not free the call key itself 
int
SipFreeSipCallKey(SipCallLegKey *callLeg, int freefn)
{
	MFree(freefn, callLeg->callid);
	UrlFree(callLeg->local, freefn);
	UrlFree(callLeg->remote, freefn);
	return(0);
}

int
SipFreeSipMsgHandle(SipMsgHandle *msgHandle)
{
	int i;
	header_url_list *elem, *list;

	if (msgHandle == NULL)
	{
		return 0;
	}

	SipCheckFree(msgHandle->callid);
	UrlFree(msgHandle->local, MEM_LOCAL);
	UrlFree(msgHandle->remote, MEM_LOCAL);
	UrlFree(msgHandle->requri, MEM_LOCAL);
	UrlFree(msgHandle->localcontact, MEM_LOCAL);
	UrlFree(msgHandle->remotecontact, MEM_LOCAL);
	if(msgHandle->remotecontact_list)
	{
		list = msgHandle->remotecontact_list;
		do
		{
			elem = list;
			list = elem->next;
			UrlFree(elem->url, MEM_LOCAL);
			SipCheckFree(elem);
		}
		while(list != msgHandle->remotecontact_list);
	}
	UrlFree(msgHandle->referto, MEM_LOCAL);
	UrlFree(msgHandle->referby, MEM_LOCAL);

	SipCheckFree(msgHandle->srchost);

	for(i=0;i<msgHandle->nroutes;i++)
	{
		SipCheckFree((msgHandle->routes)[i]);
	}
	SipCheckFree(msgHandle->routes);

	SipCheckFree(msgHandle->method);
	SipCheckFree(msgHandle->responseMsgStr);
	
	// Free the routes also

	SipCheckFree(msgHandle->xConnId);

	SipCheckFree(msgHandle->also);
	SipCheckFree(msgHandle->replaces);
	SipCheckFree(msgHandle->alert_info);

	SipCheckFree(msgHandle);
	return 1;
}

// Validates the handle from both the Bridge and Network side
// Not used anu more, as was only in debugging phase
int
SipValidateSipEventHandle(SipEventHandle *evHandle)
{
	char fn[] = "SipValidateSipEventHandle():";
	SipAppMsgHandle *appMsgHandle;
	SipMsgHandle *msgHandle;

	return 1;

	appMsgHandle = evHandle->handle;
	msgHandle = appMsgHandle->msgHandle;

	if (evHandle->type == Sip_eBridgeEvent)
	{
		if (appMsgHandle->msgHandle)
		{
			NETERROR(MSIP, ("%s Stale msgHandle present, ignoring.\n", fn));
			free(appMsgHandle->msgHandle);

			appMsgHandle->msgHandle = NULL;
		}

		/* If this is a request event, we will need the request URI */
		if ((evHandle->event != Sip_eNetwork1xx) &&
			(evHandle->event != Sip_eNetwork200) &&
			(evHandle->event != Sip_eNetwork3xx) &&
			(evHandle->event != Sip_eNetworkFinalResponse)&&
			(evHandle->event != Sip_eNetworkAck))
		{
			// We must have a request uri in this case
			if (appMsgHandle->requri == NULL)
			{
				NETERROR(MSIP, ("%s No Request URI present for request\n", fn))
				return -1;
			}
		}
	}

	if (evHandle->event == Sip_eBridgeInvite)
	{
		if (evHandle->type == Sip_eBridgeEvent)
		{
			// must have a called pn and a calling pn
			if ((appMsgHandle->callingpn == NULL) ||
				(appMsgHandle->calledpn == NULL))
			{
				NETERROR(MSIP, 
					("%s No called party number or calling party no\n", fn));
				return -1;
			}
		}
		else
		{
			if ((msgHandle->callid == NULL) ||
				(msgHandle->local == NULL) ||
				(msgHandle->remote == NULL))
			{
				NETERROR(MSIP,
					("%s Malformed call leg key\n", fn));
				return -1;
			}
		}
	}

	return 1;
}

int
SipInitializeContact(CallHandle *callHandle, SipMsgHandle *msgHandle)
{
	char fn[] = "SipInitializeContact():";
	SipCallHandle *sipCallHandle;

	/* FIll in the tag in the call handle if one is not
	 * already there
	 */
	if (!msgHandle->remotecontact) 
	{
		return 0;
	}

	sipCallHandle = SipCallHandle(callHandle);

	if (sipCallHandle->remoteContact)
	{
		/* Free it */
		UrlFree(sipCallHandle->remoteContact, sipCallCache->free);
	}

	sipCallHandle->remoteContact = UrlDup(msgHandle->remotecontact,
										sipCallCache->malloc);

	return 1;
}

/* Must have locks before we call this fn */
int
SipInitializeTags(CallHandle *callHandle, SipMsgHandle *msgHandle)
{
	char fn[] = "SipInitializeTags():";
	SipCallHandle *sipCallHandle;

	/* FIll in the tag in the call handle if one is not
	 * already there
	 */
	if (!msgHandle->remote || !msgHandle->remote->tag)
	{
		return 0;
	}

	sipCallHandle = SipCallHandle(callHandle);

	if (sipCallHandle->callLeg.remote->tag)
	{
		if (strcmp(sipCallHandle->callLeg.remote->tag,
				msgHandle->remote->tag))
		{
			NETERROR(MSIP, ("%s Tag mismatch have %s got %s in remote\n",
				fn, sipCallHandle->callLeg.remote->tag,
				msgHandle->remote->tag));

			return -1;
		}
	}
	else
	{
		/* Delete the old reference from the sip cache */
		CacheDelete(sipCallCache, &sipCallHandle->callLeg);

		/* Initialize the tag to the one in the message */
		sipCallHandle->callLeg.remote->tag = 
			CStrdup(sipCallCache, msgHandle->remote->tag);
		
		/* Re-insert it into the cache */
		CacheInsert(sipCallCache, callHandle);		

		return 1;
	}

	return 1;
}

int
SipCreateTags(CallHandle *callHandle)
{
	char fn[] = "SipCreateTags():";
	SipCallHandle *sipCallHandle;
	char tag[256];
	struct timeval tv;

	sipCallHandle = SipCallHandle(callHandle);
	if (sipCallHandle->callLeg.local->tag)
	{
		/* tag already exists */
		return 1;
	}
	else
	{
		/* Delete the old reference from the sip cache */
		CacheDelete(sipCallCache, &sipCallHandle->callLeg);

		/* Generate the tag */
		gettimeofday(&tv, NULL);

		sprintf(tag, "%lu-%lu", (tv.tv_sec + 2208988800u), tv.tv_usec);	
		sipCallHandle->callLeg.local->tag = CStrdup(sipCallCache, tag);
		
		/* Re-insert it into the cache */
		CacheInsert(sipCallCache, callHandle);		

		return 1;
	}

	return 1;
}

int
SipPrintState(CallHandle * callHandle)
{
	SipCallHandle *sipCallHandle = NULL;
	char str1[CALL_ID_LEN], str2[CALL_ID_LEN];

	sipCallHandle = SipCallHandle(callHandle);

	NETDEBUG(MSIP, NETLOG_DEBUG1,
		("cid=%s cfid=%s leg %d %s/l=%s@%s:%d/r=%s@%s:%d", 
		(char*) CallID2String(callHandle->callID, str1),
		(char*) CallID2String(callHandle->confID, str2),
		callHandle->leg,
		(char*) SVal(sipCallHandle->callLeg.callid),
		sipCallHandle->callLeg.local ? (char*) SVal(sipCallHandle->callLeg.local->name) : "0",
		sipCallHandle->callLeg.local ? (char*) SVal(sipCallHandle->callLeg.local->host) : "0",
		sipCallHandle->callLeg.local ? sipCallHandle->callLeg.local->port : 0,
		sipCallHandle->callLeg.remote ? (char*) SVal(sipCallHandle->callLeg.remote->name) : "0",
		sipCallHandle->callLeg.remote ? (char*) SVal(sipCallHandle->callLeg.remote->host) : "0",
		sipCallHandle->callLeg.remote ? sipCallHandle->callLeg.remote->port : 0));
	return(0);
}

SipEventHandle *
SipEventHandleDup(SipEventHandle *evb)
{
	SipEventHandle *evHandle;
	SipAppMsgHandle *appMsgHandle, *inAppMsgHandle;

	if (evb == NULL)
	{
		return NULL;
	}

	inAppMsgHandle = SipEventAppHandle(evb);

	evHandle = SipAllocEventHandle();
	memcpy(evHandle, evb, sizeof(SipEventHandle));

	appMsgHandle = SipAllocAppMsgHandle();
	evHandle->handle = appMsgHandle;

	appMsgHandle->maxForwards = inAppMsgHandle->maxForwards;
	memcpy(appMsgHandle->callID, inAppMsgHandle->callID, CALL_ID_LEN);
	memcpy(appMsgHandle->confID, inAppMsgHandle->confID, CONF_ID_LEN);
	appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (inAppMsgHandle->realmInfo, MEM_LOCAL);

	return evHandle;
}

char *
SipXConnIdFromCallID(char *callID)
{
	char *xconnid = (char *)malloc(64);
	unsigned long ipaddr;
	pthread_t threadid;
	hrtime_t hrtime;

	memcpy((char *)&ipaddr, callID, 4);
	memcpy((char *)&threadid, callID+4, 4);
	memcpy((char *)&hrtime, callID+8, 8);

	sprintf(xconnid, "%lu-%lu-%llu", ipaddr, ULONG_FMT(threadid), hrtime);

	return xconnid;
}

// These arrays SHOULD be SORTED.
int huntRC[] = {
		404,
		480,
        };

int nohuntRC[] = {
		401,
		403,
		407,
		481,
		486,
		501,
        };

int huntRCMax = sizeof(huntRC)/sizeof(int);
int nohuntRCMax = sizeof(nohuntRC)/sizeof(int);
int sipHuntDefault = 1;

int sipHuntError(int respCode)
{
    if (VALID_SIPCODE(respCode))
    {
        return codemap[CODEMAP_SIPINDEX(respCode)].hunt;
    }
    
    return 0;
}

// return value -1 => no src is found
// input: uri (usually From), Via host (string), ipaddress
// usually the signaling ip (contact of remote ep)
// output: srcCacheInfo
int
SipFindSrc(
	header_url *from_uri,
	char *srchost,
	unsigned long ip,
	char *tg,
	char *cic,
	char *dnis,
	CallRealmInfo *realmInfo,
	CacheTableInfo *srcCacheInfo
)
{
	char 		fn[] = "SipFindSrc():";
	int 		rcSrc = -1;
	PhoNode 	phonode = { 0 };
	char 		*fromuri = NULL;
	int		fromurilen;
	int herror;
	char rsadomain[24];

	if (!from_uri || !from_uri->host)
	{
		NETERROR(MSIP, ("%s invalid uri %p or host\n",
			fn, from_uri));

		return rcSrc;
	}

	phonode.realmId = realmInfo->realmId;
	FormatIpAddress(realmInfo->rsa, rsadomain);
	if (!SipMatchDomains(rsadomain, from_uri->host))
	{
		// domain matches our domain
		// The EP may be registered or static
		// phone number may be provisioned

		if (from_uri->name)
		{
			nx_strlcpy(phonode.phone, SVal(from_uri->name),
				PHONE_NUM_LEN);
			BIT_SET(phonode.sflags, ISSET_PHONE);
		}

		phonode.ipaddress.l = ip;
		BIT_SET(phonode.sflags, ISSET_IPADDRESS);

		rcSrc = FillSourceCacheForCallerId(&phonode, "", tg, "", dnis, srcCacheInfo);

		// if contact failed, check the via
		if (rcSrc < 0)
		{
			phonode.ipaddress.l = ResolveDNS(srchost, &herror);
			rcSrc = FillSourceCacheForCallerId(&phonode, "", tg, "", dnis, srcCacheInfo);
		}
	}
	else
	{
		// domain is not ours. Either the Via or the
		// contact may have been provisioned. We may be acting
		// as the OBP also.
		// phone number does not belong directly in MSW domain,
		// so we will skip that step

		if (from_uri->name)
		{
			nx_strlcpy(phonode.phone, SVal(from_uri->name),
				PHONE_NUM_LEN);
			BIT_SET(phonode.sflags, ISSET_PHONE);
		}

		phonode.ipaddress.l = ResolveDNS(srchost, &herror);
		BIT_SET(phonode.sflags, ISSET_IPADDRESS);

		rcSrc = FillSourceCacheForCallerId(&phonode, "", tg, "", dnis, srcCacheInfo);

		if (rcSrc < 0)
		{
			// if via did not work, use the contact
			phonode.ipaddress.l = ip;
			rcSrc = FillSourceCacheForCallerId(&phonode, "", tg, "", dnis,
					srcCacheInfo);
		}
	}

	if (rcSrc < 0)
	{
		// Look up the from url in the uri cache

		/* Set the URLs */
		fromurilen = strlen(SVal(from_uri->name)) + 
				strlen(from_uri->host)+2;
		if(!(fromuri = (char *)calloc(fromurilen,sizeof(char))))
		{
			NETERROR(MSIP,
			("%s failed to allocate %d bytes for fromuri\n", 
			fn, fromurilen));

			return -1;
		}
	
		sprintf(fromuri, "%s@%s",
			SVal(from_uri->name),SVal(from_uri->host));

		/* Look up the uri cache */
		rcSrc = CacheFind(uriCache, fromuri, srcCacheInfo, 
				sizeof(CacheTableInfo));
	}

	if(rcSrc < 0)
	{
		/* Look up the phone cache */
		if(from_uri->name)
		{
			rcSrc = CacheFind(phoneCache, SVal(from_uri->name),
				srcCacheInfo, sizeof(CacheTableInfo));
		}
	}

	SipCheckFree(fromuri);

	return rcSrc;
}
