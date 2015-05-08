#include "bits.h"
#include "ipc.h"

#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"

#include "uh323.h"
#include "uh323cb.h"

#include "gis.h"
#include "nxosd.h"
#include <malloc.h>

#include "gk.h"
#include "log.h"
#include "stkutils.h"
#include "ipstring.h"
#include "radclient.h"

//ASSUMPTION - Incoming setup will always be first call in a confHandle
// What that means is that we don't expect to receieve a setup if already have a conferencehandle 

int
GkAdmitCallFromSetup(HCALL hsCall, HAPPCALL haCall)
{
	 char fn[] = "GkAdmitCallFromSetup():";
	 INT32 tlen;
	 CallHandle *callHandle = NULL, *dupCallHandle = NULL;
	 int rc = -1;
	 ResolveHandle *rhandle = NULL;
	 cmAlias alias, number;
	 char string[256];
	 int len;
	 int addrlen = sizeof(cmTransportAddress);
	 CacheTableInfo scacheInfoEntry = { 0 }, *sgkInfo = NULL;
	 char *rPhone, guessPhone[PHONE_NUM_LEN] = { 0 };
	 ConfHandle *confHandle = NULL;
     cmTransportAddress addr = {0},remoteAddr = {0};
	 char h323id[H323ID_LEN] = { 0 };
	 int index = 0;
	 char ipstr[24], tg[PHONE_NUM_LEN] = { 0 };
	 HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	 int nodeId, groupNodeId, srcInfoNodeId, tokenNodeId;
	 int tglen;
	char CID[80] = { 0 };
	INT32 CIDLen;
	char confIDStr[CONF_ID_STRLEN];
	 BOOL isstring;
	 h221VendorInfo h221ns;
	 int vendor;
	 RealmEntry *realmEntry;
	unsigned int rsa;
	int dropCall = 0;
	int freePort = 0;
         
     UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
         
	 NETDEBUG(MH323, NETLOG_DEBUG1,
		   ("%s Entering, hsCall = %p, haCall = %p\n",
			fn, hsCall, haCall));

	/* Use the sourceAddress and destinationAddress information
	 * to set up the call
	 */

	callHandle = GisAllocCallHandle();

	callHandle->handleType = SCC_eH323CallHandle;
 	H323controlState(callHandle) = UH323_sControlIdle;
	callHandle->state = SCC_sIdle;
	timedef_cur(&callHandle->callStartTime);
	callHandle->callSource = 0;
	callHandle->lastEvent = SCC_evt323SetupRx;

	// Create the realm Info inside the call handle
	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);
	rsa = appHandle->localIP;
	realmEntry = CacheGet (rsaCache, &rsa);
	if (realmEntry == NULL) 
	{
		NETERROR (MSCC, ("%s Could not get realm entry for rsa %s",
			fn , ULIPtostring(appHandle->localIP)));
		CacheReleaseLocks (realmCache);

		callHandle->callError = SCC_errorGeneral;

		GkCallDropReasonNone(hsCall, callHandle->callError);

		BillCall(callHandle, CDR_CALLDROPPED);
		GisFreeCallHandle(callHandle);

		return -1;
	}

	callHandle->realmInfo =  (CallRealmInfo *) MMalloc (callCache->malloc, sizeof (CallRealmInfo));
	if (callHandle->realmInfo == NULL)
	{
		NETERROR (MSCC, ("%s malloc failed for realmInfo", fn));

		CacheReleaseLocks (realmCache);

		callHandle->callError = SCC_errorGeneral;

		GkCallDropReasonNone(hsCall, callHandle->callError);

		BillCall(callHandle, CDR_CALLDROPPED);
		GisFreeCallHandle(callHandle);
		return -1;
	}

	memset(callHandle->realmInfo, 0, sizeof (CallRealmInfo));
	callHandle->realmInfo->rsa = realmEntry->rsa;
	callHandle->realmInfo->realmId = realmEntry->realmId;
	callHandle->phonode.realmId = realmEntry->realmId;
	callHandle->realmInfo->sPoolId = realmEntry->sigPoolId;
	callHandle->realmInfo->mPoolId = realmEntry->medPoolId;
	callHandle->realmInfo->addrType = realmEntry->addrType;
	callHandle->realmInfo->interRealm_mr = realmEntry->interRealm_mr;
	callHandle->realmInfo->interRealm_mr = realmEntry->intraRealm_mr;

	CacheReleaseLocks (realmCache);

	
	setRadiusAccountingSessionId(callHandle);

	tlen = 80;
	/* get callID first */
	if (cmCallGetParam(hsCall, cmParamCallID, 0, &tlen, (char *)CID)<0)
	{
		NETERROR(MSCC, ("%s Could not get callID from Setup\n", fn));
		callHandle->callError = SCC_errorH323Protocol;
		dropCall = 1;
	}
	else
	{
		memcpy(callHandle->handle.h323CallHandle.guid,CID,GUID_LEN);
        if (CacheFind(guidCache, callHandle->handle.h323CallHandle.guid, NULL, 0) >= 0)
        {
            NETERROR(MH323, ("%s SETUP loop is detected!\n",fn));
            callHandle->callError = SCC_errorH323Protocol;
            dropCall = 1;
        }
	}
		
	if (cmCallGetParam(hsCall, cmParamCID, 0, &tlen, (char *)CID)<0)
	{
		NETERROR(MSCC, ("%s Could not get confID from Setup\n", fn));
		if (callHandle->callError == SCC_errorNone)
		{
			callHandle->callError = SCC_errorH323Protocol;
		}
		dropCall = 1;
	}
	else
	{
		ConfID2String2(CID, confIDStr);
		callHandle->incoming_conf_id = CStrdup(callCache, confIDStr);
		callHandle->conf_id = CStrdup(callCache, confIDStr);
	}

	callHandle->leg = SCC_CallLeg1;
	callHandle->networkEventProcessor = SCC_H323Leg1NetworkEventProcessor;
	callHandle->bridgeEventProcessor = SCC_H323Leg1BridgeEventProcessor;

	/* Create the call Handle */
	/* Initialize the main parameter */
	CallSetCallID(callHandle, appHandle->callID);
	memcpy(callHandle->confID, appHandle->confID, CONF_ID_LEN);
	H323hsCall(callHandle) = hsCall;

	tlen = sizeof(cmAlias);
	number.length = 256;
	number.string = string;

	while ((cmCallGetParam(hsCall, cmParamSourceAddress, index++, 
							  &tlen, (char *)&number)) >= 0)
	{
		if (number.type != cmAliasTypeH323ID)
		{
			goto _continue;
		}

		utlBmp2Chr(&h323id[0], number.string, number.length);
		strncpy(H323h323Id(callHandle),h323id,H323ID_LEN);
		H323h323Id(callHandle)[H323ID_LEN -1] = '\0';

	 _continue:
		  tlen = sizeof(cmAlias);
		  number.length = 256;
		  number.string = string;
	}

	if (GkCallExtractCallerPhone(H323hsCall(callHandle),
        	H323callingPartyNumber(callHandle),
        	PHONE_NUM_LEN, &H323OrigCallingPartyNumType(callHandle)) < 0)
   	{
   	 	NETDEBUG(MSCC, NETLOG_DEBUG4,
       	    ("%s Unable to retrieve Calling Party Number\n",fn));
   	}
	else
	{
		strcpy(callHandle->phonode.phone, H323callingPartyNumber(callHandle));
		BIT_SET(callHandle->phonode.sflags, ISSET_PHONE);
	}

	H323callingPartyNumberLen(callHandle) = strlen(H323callingPartyNumber(callHandle));
	strcpy(CallInputANI(callHandle), H323callingPartyNumber(callHandle));

	tlen = sizeof(cmAlias);
	number.length = 256;
	number.string = string;
	number.type = cmAliasTypePartyNumber;
	if (cmCallGetParam(hsCall, 
				   cmParamCalledPartyNumber,
				   0,
				   &tlen,
				   (char *)&number) < 0)
	{
        int destFound = 0;
		H323OrigCalledPartyNumType(callHandle) = cmPartyNumberPublicUnknown;
        tlen = sizeof(cmAlias);
        number.length = 256;
        number.string = string;
        index = 0;
        while ((cmCallGetParam(hsCall, cmParamDestinationAddress, index++,
                                  &tlen, (char *)&number)) >= 0)
        {
            if (number.type == cmAliasTypeE164)
            {
                destFound = 1;
                break;
            }
            else {
              tlen = sizeof(cmAlias);
              number.length = 256;
              number.string = string;
            }
        }
        if(!destFound)
        {
            NETERROR(MH323, ("%s cmParamCalledPartyNumber failed\n",fn));
			if (callHandle->callError == SCC_errorNone)
			{
            	callHandle->callError = SCC_errorInvalidPhone;
			}
			dropCall = 1;
        }
	}
	else
	{
		H323OrigCalledPartyNumType(callHandle) = (number.pnType & 0x70) >> 4;
	}

	NETDEBUG(MH323, NETLOG_DEBUG4,
	  ("%s Found destination phone = %s\n",fn,number.string));

	nx_strlcpy(H323dialledNumber(callHandle),number.string,PHONE_NUM_LEN);
	nx_strlcpy(callHandle->rfphonode.phone,number.string,PHONE_NUM_LEN);
	BIT_SET(callHandle->rfphonode.sflags, ISSET_PHONE);

	// The original input number is stored here
	strcpy(CallInputNumber(callHandle), callHandle->rfphonode.phone);

	if (cmCallGetParam(hsCall, 
					   cmParamRemoteIpAddress, 0, 
					   &tlen, (char *)&remoteAddr) < 0)
	{
		NETERROR(MQ931, ("%s No RemoteIp Address found\n", fn));
		remoteAddr.ip =  0;
	}

    if (cmCallGetParam(hsCall, 
					   cmParamSrcCallSignalAddress, 0, 
					   &tlen, (char *)&addr) < 0)
    {
		NETDEBUG(MH323,NETLOG_DEBUG3,("%s No Signalling Address found\n", fn));
		if(remoteAddr.ip)
		{
			addr = remoteAddr;
		}
		else {
			addr.ip = 0;
			NETERROR(MQ931, ("%s No Signalling Address and No Remote Address found\n", fn));
		}
    }

	callHandle->phonode.ipaddress.l = ntohl(remoteAddr.ip);
	BIT_SET(callHandle->phonode.sflags, ISSET_IPADDRESS);
	H323callsigport(callHandle) = remoteAddr.port;

	if (dropCall > 0)
	{
		goto _error;
	}

    if (cmCallGetParam(hsCall, 
                           cmParamFullSourceInfo, 0, 
                           &srcInfoNodeId, NULL) < 0)
    {
		NETDEBUG(MH323,NETLOG_DEBUG3,("%s No src info nodeid found\n", fn));
		srcInfoNodeId = -1;
    }

	cmMeiEnter(UH323Globals()->hApp);

	if ((nodeId = cmGetProperty((HPROTOCOL)hsCall)) >=0)
	{
		if ((groupNodeId = pvtGetNodeIdByPath(hVal,
			nodeId ,
			"setup.message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup.circuitInfo.sourceCircuitID.group.group")) > 0)
		{
			if ((tglen = pvtGetString(hVal, groupNodeId, PHONE_NUM_LEN-1, tg)) > 0)
			{
				tg[tglen] = '\0';
				callHandle->tg = CStrdup(callCache, tg);
			}
		}

		if ((groupNodeId = pvtGetNodeIdByPath(hVal,
			nodeId ,
			"setup.message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup.circuitInfo.destinationCircuitID.group.group")) > 0)
		{
			if ((tglen = pvtGetString(hVal, groupNodeId, PHONE_NUM_LEN-1, tg)) > 0)
			{
				tg[tglen] = '\0';
				callHandle->destTg = CStrdup(callCache, tg);
			}
		}
	}
	else 
	{
		NETERROR(MH323, ("%s : failed to get property nodeid\n", fn ));
	}


	if (srcInfoNodeId >= 0)
	{	
		if (pvtGetByPath(hVal, srcInfoNodeId,
			"vendor.vendor.t35CountryCode", NULL,
				(INT32 *)&h221ns.t35CountryCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s t35 country code not present\n",fn));
				h221ns.t35CountryCode = -1;
		}

		if (pvtGetByPath(hVal, srcInfoNodeId,
			"vendor.vendor.manufacturerCode", NULL,
				(INT32 *)&h221ns.manufacturerCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s mfr code not present\n",fn));
			h221ns.manufacturerCode = -1;
		}

		vendor = GetVendorFromH221Info(&h221ns);
		if (vendor > 0)
		{
			callHandle->vendor = vendor;
		}
	}

	// Extract the vendor info from srcinfo
	cmMeiExit(UH323Globals()->hApp);

	/* extract the tokens from Setup for use in ARQ later */
	if((H323tokenNodeId(callHandle) = getSetupTokenNodeId(hsCall))<0)
	{
		NETDEBUG(MARQ,NETLOG_DEBUG4,("%s : getSetupTokenNodeId failed",fn ));
	}

    rc = FillSourceCacheForCallerId(&callHandle->phonode, 
			h323id, callHandle->tg, 0, callHandle->rfphonode.phone, &scacheInfoEntry);

	// IF FillSource is removed from here to bridge, 
	// then FIX IT to look for remote ip also
	if((rc <0) && addr.ip && (remoteAddr.ip != addr.ip))
	{
		// most probably GK routed with src signalling diff from peerIP
		NETDEBUG(MSCC, NETLOG_DEBUG4,
	   		("%s Could not find source using remote ip. Trying sig Ip %s\n", 
			fn,FormatIpAddress(addr.ip, ipstr)));
		callHandle->phonode.ipaddress.l = ntohl(addr.ip);
		BIT_SET(callHandle->phonode.sflags, ISSET_IPADDRESS);
		rc = FillSourceCacheForCallerId(&callHandle->phonode, 
				h323id, callHandle->tg, 0, callHandle->rfphonode.phone, &scacheInfoEntry);
	}

   	if (rc <  0)
    {
		NETDEBUG(MSCC, NETLOG_DEBUG4,
	   		("%s Could not find source\n", fn));
		if (!SrcARQEnabled() && !allowSrcAll)
		{
			NETERROR(MH323, ("%s Source not found\n", fn));
			callHandle->callError = SCC_errorBlockedUser;
			goto _error;
		}			

		callHandle->vpnName = strdup("");
		callHandle->zone = strdup("");
		callHandle->cpname = strdup("");
	}
	else 
	{
			/* copy the stuff from scacheInforEntry */
			strcpy(callHandle->phonode.regid,scacheInfoEntry.data.regid);
			callHandle->phonode.uport = scacheInfoEntry.data.uport;
			if (scacheInfoEntry.data.vendor)
			{
				callHandle->vendor = scacheInfoEntry.data.vendor;
			}

			if (scacheInfoEntry.data.custID[0] != '\0')
			{
				callHandle->custID = CStrdup(callCache,
						scacheInfoEntry.data.custID);
			}
			
			if (scacheInfoEntry.data.h323id[0] != '\0')
			{
				strncpy(H323h323Id(callHandle),
						scacheInfoEntry.data.h323id ,H323ID_LEN);
				H323h323Id(callHandle)[H323ID_LEN -1] = '\0';
			}

			if (scacheInfoEntry.data.srcIngressTG[0] != '\0')
			{
				/* now replace tg with srcIngressTG */
				if ( callHandle->tg )
				{
					/* free it first, it might too short for strcpy */
					(CFree(callCache))(callHandle->tg);
				}
				callHandle->tg = CStrdup(callCache,
							scacheInfoEntry.data.srcIngressTG);
			}

			if (scacheInfoEntry.data.srcEgressTG[0] != '\0')
			{
				/* now replace tg with srcIngressTG */
				if ( callHandle->destTg )
				{
					/* free it first, it might too short for strcpy */
					(CFree(callCache))(callHandle->destTg);
				}
				callHandle->destTg = CStrdup(callCache,
							scacheInfoEntry.data.srcEgressTG);
			}

			if(GwPortAllocCall(&callHandle->phonode, 1, 0)<0)
			{
				NETERROR(MSCC,
				("%s GwPortAllocCall Failed source= %s/%lu called No.= %s\n",
				fn,callHandle->phonode.regid,callHandle->phonode.uport,
				H323dialledNumber(callHandle)));
				callHandle->callError = SCC_errorInadequateBandwidth;
				goto _error;
			}
			else
			{
				freePort = 1;
			}

		callHandle->maxHunts = scacheInfoEntry.data.maxHunts;

		if (callHandle->maxHunts == 0)
		{
			callHandle->maxHunts = maxHunts;
		}
		else if (callHandle->maxHunts > SYSTEM_MAX_HUNTS)
		{
			callHandle->maxHunts = SYSTEM_MAX_HUNTS;
		}

		callHandle->ecaps1 = scacheInfoEntry.data.ecaps1;
		callHandle->cap = scacheInfoEntry.data.cap;
		callHandle->vpnName = strdup(scacheInfoEntry.data.vpnName);
		callHandle->zone = strdup(scacheInfoEntry.data.zone);
		callHandle->cpname = strdup(scacheInfoEntry.data.cpname);
		if (BIT_TEST(scacheInfoEntry.data.sflags, ISSET_PHONE))
		{
			strcpy(callHandle->phonode.phone, scacheInfoEntry.data.phone);
			BIT_SET(callHandle->phonode.sflags, ISSET_PHONE);
		}

		if ( !(callHandle->ecaps1 & ECAPS1_DELTCSRFC2833DFT) )
		{
			/* use system default */
			h323RemoveTcs2833 ? (callHandle->ecaps1 |= ECAPS1_DELTCSRFC2833) : (callHandle->ecaps1 &= ~ECAPS1_DELTCSRFC2833);
		}
		if ( !(callHandle->ecaps1 & ECAPS1_DELTCST38DFT) )
		{
			/* use system default */
			h323RemoveTcsT38 ? (callHandle->ecaps1 |= ECAPS1_DELTCST38) : (callHandle->ecaps1 &= ~ECAPS1_DELTCST38);
		}

			// check to see if we should do an arq/drq
			if (scacheInfoEntry.data.crId != iservercrId)
			{
				H323arqcrid(callHandle) = 
					scacheInfoEntry.data.crId;
			}

			// Check to see if the call has been routed from an sgatekeeper
			// If so, is the sgatekeeper registered ?
			if (IsSGatekeeper(&scacheInfoEntry.data))
			{
				if (!(scacheInfoEntry.data.stateFlags & CL_ACTIVE))
				{
					NETERROR(MSCC,
						("%s Call coming from SGK %s/%lu, which we are not registered to!\n",
						fn, scacheInfoEntry.data.regid, scacheInfoEntry.data.uport));

					// Let this call go through
				}
			}

		if ((scacheInfoEntry.data.stateFlags & CL_STATIC) || 
			((scacheInfoEntry.data.stateFlags & CL_REGISTERED)
			  && ! IsSGatekeeper(&scacheInfoEntry.data)) ) 
		{
			/* Don't Exchange ARQ/ACF for registered/static eps 
			BIT_SET(H323flags(callHandle), H323_FLAG_AUTHARQ_SENT); */
		}
		else if (SrcARQEnabled())
		{
			/* Exhcange ARQ/ACF/ARJ with SGK for SETUP coming in from
			 * SGK, unprovisioned endpoints
			 */
			BIT_RESET(H323flags(callHandle), H323_FLAG_AUTHARQ_SENT);
		}
	}

	// Check if this instance has enough maxcalls resources
	// e.g. At minimum 100 max calls or 20% of configured maxcalls should
	// be left
	if (((nh323Instances == 1) && 
			(2*UH323Globals()->nCalls + h323maxCallsPadFixed > UH323Globals()->maxCalls) &&
			(2*UH323Globals()->nCalls*100 >= h323maxCallsPadVariable*UH323Globals()->maxCalls)) ||
		((nh323Instances > 1) && 
			(UH323Globals()->nCalls + h323maxCallsPadFixed > UH323Globals()->maxCalls) &&
			(UH323Globals()->nCalls*100 >= h323maxCallsPadVariable*UH323Globals()->maxCalls)))
	{
		NETERROR(MSCC, ("%s ncalls=%d, xcalls = %d, dropping incoming call\n",
			fn, UH323Globals()->nCalls, UH323Globals()->maxCalls));

		callHandle->callError = SCC_errorH323MaxCalls;
		goto _error;
	}

	if(!allowHairPin)
	{
		// Hairpin is NOT allowed, add src into reject list.
		// If src is not found, add the src ip addr
		GwAddPhoNodeToRejectList(&callHandle->phonode, NULL,
			&callHandle->destRejectList, CMalloc(callCache));
	}

	// After this the call MUST be in the CALL CACHE
	// as if the source port alloc fails, and the call goes
	// disconnected, we should be able to delee the port

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	CacheInsert(guidCache, callHandle);
	if (CacheInsert(callCache, callHandle) < 0)
	{
		// Really should never happen, error would mean duplicate in cache already
		NETERROR(MSCC,("%s CacheInsert for callHandle failed, continuing\n",fn));
	}

	/** Fill the source cache entry.... */
	H323rasip(callHandle) = scacheInfoEntry.data.rasip;
	H323rasport(callHandle) = scacheInfoEntry.data.rasport;

	/* Insert the call handle into the cache */
	NETDEBUG(MSCC, NETLOG_DEBUG4,
		   ("%s Inserting new call handle into the cache\n", fn));

	//If we have call Handle its from dest else its from source
	if((rc = GisAddCallToConf(callHandle))!= 0)
	{
		NETERROR(MSCC,("%s GisAddCallToConf Failed\n",fn));

		// mark the call as having been billed
		callHandle->callError = SCC_errorNoVports;
		GkCallDropReasonNone(hsCall, callHandle->callError);
		callHandle->cause = GkMapCallErrorToCauseCode( callHandle->callError );
		GkMapCallErrorToH225nRasReason( callHandle->callError, &(callHandle->h225Reason), NULL);
		BillCall(callHandle, CDR_CALLDROPPED);
        CacheDelete(guidCache, callHandle->handle.h323CallHandle.guid);
        CacheDelete(callCache, callHandle->callID);
		GwPortFreeCall(&callHandle->phonode, 1, 0);
		GisFreeCallHandle(callHandle);
		goto _return;
	}

	// if we dont resolve it, we set this to 1
	rc = 1;
	goto _return;

_error:
	GkCallDropReasonNone(hsCall, callHandle->callError);
	callHandle->cause = GkMapCallErrorToCauseCode( callHandle->callError );
	GkMapCallErrorToH225nRasReason( callHandle->callError, &(callHandle->h225Reason), NULL);
	BillCall(callHandle, CDR_CALLDROPPED);
	if (freePort > 0)
	{
		GwPortFreeCall(&callHandle->phonode, 1, 0);
	}
	GisFreeCallHandle(callHandle);
	return -1;

_return:
	CacheReleaseLocks(callCache);
	return rc;
}


#ifdef NOT_USED
/* reserves the allocated port if reservePort is nonZero 
*  resolve locally only if locally is nonzero 
*  If the phone is DND then resolve aphonode and overwrite callHandle->rfphonode
*  with alternate phone number
*/
int
GkResolvePhone2IP(CallHandle *callHandle, int reservePort,int resolveRemote)
{
	char 			fn[] = "GkResolvePhone2IP():";
	int				rc = -1;
	ResolveHandle 	*rhandle = NULL;
	CacheTableInfo 	scacheInfoEntry = { 0 };
	char  			guessPhone[PHONE_NUM_LEN] = { 0 };
	PhoNode			rfphonode = {0};

	NETDEBUG(MFIND, NETLOG_DEBUG1,("%s Entering\n",fn));

        //rc = FillSourceCacheForCallerId(&callHandle->phonode, H323h323Id(callHandle), callHandle->tg, &scacheInfoEntry);
        if (rc <  0)
        {
        	NETDEBUG(MSCC, NETLOG_DEBUG4,
              		 ("%s Could not find source\n", fn));
		if (!allowSrcAll)
		{
			rc = -1;
			goto _return;		
		}
        }

	/* Initialize the rhandle */
	rhandle 			= GisAllocRHandle();
	rhandle->phonodep 	= &callHandle->phonode;
	rhandle->rfphonodep = &callHandle->rfphonode;

	rhandle->reservePort = reservePort;
	rhandle->resolveRemote = resolveRemote;

	memcpy(&rhandle->scacheInfoEntry, &scacheInfoEntry, 
		sizeof(CacheTableInfo));

	/* Determine the vpn group of this entry */
	FindIedgeVpn(&rhandle->scacheInfoEntry.data.vpnName, &rhandle->sVpn);

	/* Zones are valid only for LUS */
	strncpy(rhandle->sZone, rhandle->scacheInfoEntry.data.zone, ZONE_LEN);
	rhandle->sZone[ZONE_LEN-1] = '\0';

	rhandle->checkZone = 1;
	rhandle->checkVpnGroup = 1;

	if (BIT_TEST(rhandle->rfphonodep->sflags, ISSET_PHONE))
	{
		GuessVpnPhone(&rhandle->scacheInfoEntry.data, 
						rhandle->rfphonodep->vpnPhone, 
						guessPhone, &rhandle->rfphonodep->vpnExtLen);

		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Guessed Vpn Phone %s\n", 
			fn,guessPhone));
		memcpy(rhandle->rfphonodep->vpnPhone, guessPhone, VPN_LEN-1); 
	}

    if (BIT_TEST(rhandle->rfphonodep->sflags, ISSET_PHONE))
    {
        GuessVpnPhone(&rhandle->scacheInfoEntry.data,
        rhandle->rfphonodep->vpnPhone,guessPhone,
        &rhandle->rfphonodep->vpnExtLen);

        NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Guessed Vpn Phone %s\n",fn,guessPhone));

        memcpy(rhandle->rfphonodep->vpnPhone, guessPhone, VPN_LEN-1);
    }

	// Save a copy of rfphonode 
	rfphonode = callHandle->rfphonode;

	/* Call the main function */
	ResolvePhone(rhandle);

	switch (rhandle->result)
	{
	 case CACHE_FOUND:
	   /* Fill up the response */
	   NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s CACHE_FOUND\n", fn));
	   rc = 1;
	   break;
	 case CACHE_NOTFOUND:
	   NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s CACHE_NOTFOUND\n", fn));
	   rc = -1;
	   goto _return;
	   break;
	 case CACHE_INPROG:
	   NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s CACHE_INPROG\n", fn));
	   /* Dont do anything here, The rhandle, and the client
		* handle should be preserved, until it times out
		*/
	   rc = 0;
	   return rc;
	   break;
	 default:
	   break;
	 }

	H323rfcallsigport(callHandle) = rhandle->rfcallsigport;

	if (H323rfcallsigport(callHandle) == 0)
	{
		NETERROR(MFIND, ("%s Call Signalling port was 0, setting it to 1720\n",
			fn));
		H323rfcallsigport(callHandle) = 1720;
	}

	if (BIT_TEST(rhandle->fCacheInfoEntry.data.sflags, ISSET_PHONE))
	{
		strcpy(H323dialledNumber(callHandle),
			rhandle->fCacheInfoEntry.data.phone);
	}
	else
	{
		NETERROR(MFIND, 
			("%s Phone Number not present in fCacheInfoEntry\n", fn));
	}

	GisFreeRHandle(rhandle);
 _reply:
 _return:
	return rc;
}

/* reserves the allocated port if reservePort is nonZero 
*  resolve locally only if locally is nonzero 
*/
int
GkResolve2Rollover(CallHandle *callHandle, int reservePort,int resolveRemote)
{
	char 			fn[] = "GkResolvePhone2IP():";
	int				rc = -1;
	ResolveHandle 	*rhandle = NULL;
	CacheTableInfo 	scacheInfoEntry = { 0 };
	char  			guessPhone[PHONE_NUM_LEN] = { 0 };
	PhoNode			rfphonode = {0};

	NETDEBUG(MFIND, NETLOG_DEBUG1,("%s Entering\n",fn));

	if(BIT_SET(callHandle->phonode.sflags, ISSET_IPADDRESS))
	{
        //rc = FillSourceCacheForCallerId(&callHandle->phonode, H323h323Id(callHandle), callHandle->tg, &scacheInfoEntry);
        if (rc <  0)
        {
            NETDEBUG(MSCC, NETLOG_DEBUG4,
                ("%s Could not find source\n", fn));
        }
	} 
	else {
		NETDEBUG(MFIND,NETLOG_DEBUG2,("%s No Signalling Address found\n", fn));
    }


	/* Initialize the rhandle */
	rhandle 			= GisAllocRHandle();
	rhandle->phonodep 	= &callHandle->phonode;
	rhandle->rfphonodep = &callHandle->rfphonode;

	rhandle->reservePort = reservePort;
	rhandle->resolveRemote = resolveRemote;

	memcpy(&rhandle->scacheInfoEntry, &scacheInfoEntry, 
		sizeof(CacheTableInfo));

	/* Determine the vpn group of this entry */
	FindIedgeVpn(&rhandle->scacheInfoEntry.data.vpnName, &rhandle->sVpn);

	/* Zones are valid only for LUS */
	strncpy(rhandle->sZone, rhandle->scacheInfoEntry.data.zone, ZONE_LEN);
	rhandle->sZone[ZONE_LEN-1] = '\0';

	rhandle->checkZone = 1;
	rhandle->checkVpnGroup = 1;

	if (BIT_TEST(rhandle->rfphonodep->sflags, ISSET_PHONE))
	{
		GuessVpnPhone(&rhandle->scacheInfoEntry.data, 
						rhandle->rfphonodep->vpnPhone, 
						guessPhone, &rhandle->rfphonodep->vpnExtLen);

		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Guessed Vpn Phone %s\n", 
			fn,guessPhone));
		memcpy(rhandle->rfphonodep->vpnPhone, guessPhone, VPN_LEN-1); 
	}

    if (BIT_TEST(rhandle->rfphonodep->sflags, ISSET_PHONE))
    {
        GuessVpnPhone(&rhandle->scacheInfoEntry.data,
        rhandle->rfphonodep->vpnPhone,guessPhone,
        &rhandle->rfphonodep->vpnExtLen);

        NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Guessed Vpn Phone %s\n",fn,guessPhone));

        memcpy(rhandle->rfphonodep->vpnPhone, guessPhone, VPN_LEN-1);
    }

	/* Set resolve remote to be 0 for alternate node.
	 * Dont know how to handle it yet.
	 */
	rhandle->primary = 1;
	rhandle->resolveRemote = 0;

	/* Call the main function */
	ResolvePhone(rhandle);

	switch (rhandle->result)
	{
	 case CACHE_FOUND:
	   /* Fill up the response */
	   NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s CACHE_FOUND\n", fn));
	   rc = 1;
	   break;
	 case CACHE_NOTFOUND:
	   NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s CACHE_NOTFOUND\n", fn));
	   rc = -1;
	   goto _return;
	   break;
	 case CACHE_INPROG:
	   NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s CACHE_INPROG\n", fn));
	   /* Dont do anything here, The rhandle, and the client
		* handle should be preserved, until it times out
		*/
	   rc = 0;
	   return rc;
	   break;
	 default:
	   break;
	 }

	H323rfcallsigport(callHandle) = rhandle->rfcallsigport;

	if (H323rfcallsigport(callHandle) == 0)
	{
		NETERROR(MFIND, ("%s Call Signalling port was 0, setting it to 1720\n",
			fn));
		H323rfcallsigport(callHandle) = 1720;
	}

	GisFreeRHandle(rhandle);
 _reply:
 _return:
	return rc;
}

/*
CallRolloverTimer(tid t)
{
  char *callID = (char *)tid.data;
  CallHandle  callHandleBlk;
  ConfHandle  confHandleBlk;
  


  	// Find CallHandle
	if (CacheFind(callCache, callID, &callHandleBlk,sizeof(CallHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle for call 0\n",fn));
		goto _error;
	}
  
	if (CacheFind(confCache, callHandleBlk.confID,&confHandleBlk,sizeof(ConfHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle\n",fn));
		goto _error;
	}

        
        // Drop the call..
        cmCallDrop((HCALL )H323hsCall(&callHandleBlk));
        
                //Delete the previous call handle...
                // Make a new call handle with the raphonode info..
        }
        

  free(tid.data);
}
*/

#endif /* NOT_USED */


