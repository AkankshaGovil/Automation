#include "uh323.h"
#include "uh323cb.h"
#include "uh323proto.h"

#include "gis.h"
#include "gk.h"

void
NetLogH323printf(int type, char *fmt, ...);


int
GkInitChannelHandle(
		CallHandle *callHandle,
  		BOOL	origin,
		HCHAN  	hsChannel,
		int		sid
)
{
	char fn[] = "GkInitChannelHandle():";
	char CallIDStr[CALL_ID_LEN];



	/* We now know who originated the channel */
    if (origin == TRUE)			
    {
		DEBUG(MH323, NETLOG_DEBUG1, 
			("%s Outgoing channel notification handle %p hsCall = %p sid = %d\n", 
						fn,hsChannel,H323hsCall(callHandle),sid));
		H323outChan(callHandle)[sid].hsChan = hsChannel;
		H323outChan(callHandle)[sid].active = TRUE;
    }
    else
    {
		DEBUG(MH323, NETLOG_DEBUG1,
			("%s Incoming channel request handle %p hsCall = %p sid = %d\n",
						fn,hsChannel,H323hsCall(callHandle),sid));
		H323inChan(callHandle)[sid].hsChan = hsChannel;
		H323inChan(callHandle)[sid].active = TRUE;
	}

	return 0;
}

int
GkInitChannelAddress(
		CallHandle *callHandle,
  		BOOL	origin,
		unsigned long ip,
		unsigned short port,
		int sid
)
{
	char fn[] = "GkInitChannelAddress():";
	unsigned long newPort = port;
	unsigned long newRtcpPort = H323outChan(callHandle)[sid].rtcpport;

    if (origin == TRUE)			
    {
        if(callHandle->vendor == Vendor_eVocalTec)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Vendor = VocalTec. Ignoring \n",fn));
			return 0;
		}

        if(callHandle->vendor == Vendor_eClarent)
        {
			H323inChan(callHandle)[sid].rtcpip = ip;
			H323inChan(callHandle)[sid].rtcpport = port+1;
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Seting up RTCP address of reverse channel based on forward channel\n",
				fn));

			if(H323outChan(callHandle)[sid].ip && H323outChan(callHandle)[sid].port &&
				((port != H323outChan(callHandle)[sid].port) ||
				 (ip != H323outChan(callHandle)[sid].ip)) ) 
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s Media Address Changed. origin = %d\n",fn,origin));
				H323outChan(callHandle)[sid].mediaChange = 1;
			}
				
			H323outChan(callHandle)[sid].ip = ip;
			H323outChan(callHandle)[sid].port = port;
        }
        else {
            /* this is special casing for vocaltec */
			// Do not fix anything if we are allowing all rtp to go thru
			if (newRtcpPort != newPort +1)    /* we have problem, need to update port number */
			{
			   if (newPort % 2)    /* RTP port does not look good */
			    {
			        newPort = newRtcpPort-1;
			    }
			    else    /* RTP port looks good, rtcp needs to be updated */
			    {
			        newRtcpPort = newPort+1;
			    }
			}
        }

		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s last rtp= %lx/%d new = %lx/%d sid = %d\n",fn,
			H323outChan(callHandle)[sid].ip,H323outChan(callHandle)[sid].port,
			ip,port,sid));
		if(H323outChan(callHandle)[sid].ip && H323outChan(callHandle)[sid].port
			 && ((port != H323outChan(callHandle)[sid].port) ||
			 (ip != H323outChan(callHandle)[sid].ip)) ) 
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Media Address Changed. origin = %d\n",fn,origin));
			H323outChan(callHandle)[sid].mediaChange = 1;
		}
				
		H323outChan(callHandle)[sid].ip = ip;
		H323outChan(callHandle)[sid].port = newPort;
		H323outChan(callHandle)[sid].rtcpport = newRtcpPort;
    }
	else 
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s last rtp= %lx/%d new = %lx/%lu\n",
			fn,H323inChan(callHandle)[sid].ip,H323inChan(callHandle)[sid].port,
			ip,newPort));

		H323inChan(callHandle)[sid].ip = ip;
		H323inChan(callHandle)[sid].port = port;

		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s GkInitChannelAddress: Initializing address for origin = 0 sid = %d\n", fn,sid));
	}

	return 0;
}

int
GkInitChannelRTCPAddress(
		CallHandle *callHandle,
  		BOOL	origin,
		unsigned long ip,
		unsigned short port,
		int		sid
)
{
	char fn[] = "GkInitChannelRTCPAddress():";
	int rtpPort;

    if (origin == TRUE)			
    {
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s last rtcp= %lx/%d new = %lx/%d sid = %d\n",
			fn,H323outChan(callHandle)[sid].ip,H323outChan(callHandle)[sid].port,
			ip,port,sid));
#if 0
		rtpPort = port - 1;
		if(H323outChan(callHandle).rtcpip && H323outChan(callHandle)[sid].rtcpport &&
			((port!= H323outChan(callHandle)[sid].port)) ||
			 (H323outChan(callHandle)[sid].ip && (ip != H323outChan(callHandle).ip)) ) ) 
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Media Address Changed. Setting rtp address to rtcp - 1\n",fn));
			H323outChan(callHandle)[sid].mediaChange = 1;
		}
#endif

		H323outChan(callHandle)[sid].rtcpip = ip;
		H323outChan(callHandle)[sid].rtcpport = port;

		// Let the RTCP port drive the RTP port, if it is VocalTec.
        if(callHandle->vendor == Vendor_eVocalTec)
        {
			if(H323outChan(callHandle)[sid].ip && H323outChan(callHandle)[sid].port &&
				(((port-1) != H323outChan(callHandle)[sid].port) ||
				 (ip != H323outChan(callHandle)[sid].ip)) ) 
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s Media Address Changed. origin = %d\n",fn,origin));
				H323outChan(callHandle)[sid].mediaChange = 1;
			}
				
			H323outChan(callHandle)[sid].ip = ip;
			H323outChan(callHandle)[sid].port = port-1;
            NETDEBUG(MH323, NETLOG_DEBUG4,
                ("%s Setting RTP port based on RTCP port to %d\n",
                fn,port - 1));
        }
    }
	else 
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s last rtcp= %lx/%d new = %lx/%d sid = %d\n",
			fn,H323inChan(callHandle)[sid].rtcpip,H323inChan(callHandle)[sid].rtcpport,
			ip,port,sid));


		/* we do not allow clarent to change port */
		if ((H323inChan(callHandle)[sid].rtcpip == 0) || 
			(callHandle->vendor != Vendor_eClarent))	// not yet assigned
		{
			rtpPort = port - 1; /* this would be port +1 for clarent */
			/* check if its media change */
			if(H323inChan(callHandle)[sid].rtcpip && H323inChan(callHandle)[sid].rtcpport &&
				((H323outChan(callHandle)[sid].port && (rtpPort!= H323outChan(callHandle)[sid].port)) ||
				 (H323outChan(callHandle)[sid].ip && (ip != H323outChan(callHandle)[sid].ip)) ) ) 
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Media Address Changed. origin = False Setting rtp address to rtcp - 1\n",fn));
				/* This flag assumes events are serialized */
				H323inChan(callHandle)[sid].mediaChange = 1;
				H323outChan(callHandle)[sid].ip = ip;
				H323outChan(callHandle)[sid].port = rtpPort;
			}

			H323inChan(callHandle)[sid].rtcpip = ip;
			H323inChan(callHandle)[sid].rtcpport = port;
		}
	}

	return 0;
}

int
GkInitChannelDataType(
		CallHandle *callHandle,
  		BOOL	origin,
		int     dataTypeHandle,
		cmCapDataType       dataType,
		int		param,
		int		sid,
		int		flags
)
{
	char fn[] = "GkInitChannelDataType():";

	if(dataType == cmCapVideo) 
	{
		HPST hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp, 
						"capData");
		HPVT hVal = cmGetValTree(UH323Globals()->hApp);
		int nodeId,dupeId;
		cmMeiEnter(UH323Globals()->hApp);

		nodeId = pvtAddRoot(hVal,
				hSyn,
				0,
				NULL);
	
		if (nodeId < 0)
		{
			NETERROR(MH323,
				("%s pvtAddRoot returns error\n", fn));
			return 0;
		}

		if (pvtBuildByPath(hVal, nodeId,
				"videoData",
				0,
				NULL) < 0)
		{
			NETERROR(MH323,
				("%s pvtBuildByPath returns error\n", fn));
		}
		//pvtPrint 83 TPKT 72 PER 62 UDP 
		dupeId = pvtChild(hVal, nodeId);

		if (pvtAddTree(hVal,dupeId, 
			hVal, dataTypeHandle) < 0)
		{
			NETERROR(MH323,
				("%s pvtAddTree returns error\n", fn));
		}
		dataTypeHandle = dupeId;

		// Free previous dataTypeHandle if it exists
		if (origin == FALSE && H323inChan(callHandle)[sid].dataTypeHandle > 0)
		{
			freeNodeTree(UH323Globals()->hApp, 
				H323inChan(callHandle)[sid].dataTypeHandle, 0);
			H323inChan(callHandle)[sid].dataTypeHandle = -1;
		}

		cmMeiExit(UH323Globals()->hApp);

	}
	else if(dataType == cmCapData) 
	{
		HPST hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp, 
						"capData");
		HPVT hVal = cmGetValTree(UH323Globals()->hApp);
		HPST peerhSyn = cmGetSynTreeByRootName(UH323Globals()->peerhApp,
						 "capData");
		HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
		int nodeId,dupeId;
		cmMeiEnter(UH323Globals()->hApp);

		nodeId = pvtAddRoot(hVal,
				hSyn,
				0,
				NULL);
	
		if (nodeId < 0)
		{
			NETERROR(MH323,
				("%s pvtAddRoot returns error\n", fn));
			return 0;
		}

		if (pvtBuildByPath(hVal, nodeId,
				"data",
				0,
				NULL) < 0)
		{
			NETERROR(MH323,
				("%s pvtBuildByPath returns error\n", fn));
		}
		//pvtPrint 83 TPKT 72 PER 62 UDP 
		dupeId = pvtChild(hVal, nodeId);

		if (pvtAddTree(hVal,dupeId, 
			hVal, dataTypeHandle) < 0)
		{
			NETERROR(MH323,
				("%s pvtAddTree returns error\n", fn));
		}
		dataTypeHandle = dupeId;

		// Free previous dataTypeHandle if it exists
		if (origin == FALSE && H323inChan(callHandle)[sid].dataTypeHandle > 0)
		{
			freeNodeTree(UH323Globals()->hApp, 
				H323inChan(callHandle)[sid].dataTypeHandle, 0);
			H323inChan(callHandle)[sid].dataTypeHandle = -1;
		}

//		pvtPrintStd(hVal,dataTypeHandle,62);
		cmMeiExit(UH323Globals()->hApp);

	}

    if (origin == TRUE)			
    {
		H323outChan(callHandle)[sid].dataTypeHandle = dataTypeHandle;
		H323outChan(callHandle)[sid].dataType = dataType;
		H323outChan(callHandle)[sid].param = param;
		H323outChan(callHandle)[sid].flags = flags;
    }
    else
    {
		H323inChan(callHandle)[sid].dataTypeHandle = dataTypeHandle;
		H323inChan(callHandle)[sid].dataType = dataType;
		H323inChan(callHandle)[sid].param = param;
		H323inChan(callHandle)[sid].flags = flags;
	}

	return 0;
}

int
GkSetChannelAddress(
		CallHandle *callHandle,
		unsigned long ip,
		unsigned short port,
		int sid
)
{
	char fn[] = "GkSetChannelAddress():";
	char ipstr[24];

	NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s last rtp= %lx/%d new = %lx/%d sid = %d\n",
		fn,H323inChan(callHandle)[sid].ip,H323inChan(callHandle)[sid].port,
		ip,port,sid));

	if(H323inChan(callHandle)[sid].ip && H323inChan(callHandle)[sid].port &&
		((port != H323inChan(callHandle)[sid].port) ||
		 (ip != H323inChan(callHandle)[sid].ip)) ) 
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s Media Address Changed. origin False\n",fn));
		H323inChan(callHandle)[sid].mediaChange = 1;
	}

	H323inChan(callHandle)[sid].ip = ip;
	H323inChan(callHandle)[sid].port = port;

	NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s ip port %s/%d\n",fn,FormatIpAddress(ip, ipstr),port));

	if (H323inChan(callHandle)[sid].hsChan == NULL)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s Channel Not Initialized yet. sid = %d\n", fn,sid));
		return 0;
	}

	if (cmChannelSetRTCPAddress(H323inChan(callHandle)[sid].hsChan,
		htonl(ip), port+1) < 0)
	{
		// This invocation returns error sometimes(fax) - checkout
		NETDEBUG(MH323,NETLOG_DEBUG4, 
			("%s cmChannelSetRTCPAddress(%p) returned error. sid = %d\n",
			fn,H323inChan(callHandle)[sid].hsChan,sid));
	}

	if (cmChannelSetAddress(H323inChan(callHandle)[sid].hsChan,
		htonl(ip), port) < 0)
	{
		NETERROR(MH323, 
			("%s cmChannelSetAddress(%p) returned error. sid = %d\n",
			fn,H323inChan(callHandle)[sid].hsChan,sid));
	}

	return 0;
}

/* Open a channel to the peer, with the given data type */
int
GkOpenChannel(CallHandle *callHandle, ChanInfo *chanInfo,int sid)
{
	char fn[] = "GkOpenChannel():";
	UH323CallAppHandle *appHandle;
	HPST hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp, 
					"capData");
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
	int nodeId;
	char ipstr[24];

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Entering sid = %d\n", fn,sid));
		
	appHandle = uh323CallAllocAppHandle();
	memcpy(appHandle,H323appHandle(callHandle),sizeof(UH323CallAppHandle));

	if (cmChannelNew(H323hsCall(callHandle),
		(HAPPCHAN)appHandle,
		&H323outChan(callHandle)[sid].hsChan) < 0)
	{
		NETERROR(MH323, 
				("%s cmChannelNew returned error leg = %d doOlc = %d dialled No. %s\n", 
				fn,callHandle->leg,H323doOlc(callHandle),callHandle->dialledNumber));
		return -1;
	}

	H323outChan(callHandle)[sid].dataTypeHandle = chanInfo->dataTypeHandle;	
	H323outChan(callHandle)[sid].dataType = chanInfo->dataType;	

	H323outChan(callHandle)[sid].rtcpip = chanInfo->rtcpip;
	H323outChan(callHandle)[sid].rtcpport = chanInfo->rtcpport;
	H323outChan(callHandle)[sid].codec = chanInfo->codec;
	H323outChan(callHandle)[sid].param = chanInfo->param;
	H323outChan(callHandle)[sid].flags = chanInfo->flags;

	NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s rtcp ip port %s/%d\n",fn,FormatIpAddress(chanInfo->rtcpip, ipstr),chanInfo->rtcpport));

	if (H323outChan(callHandle)[sid].rtcpip &&
		H323outChan(callHandle)[sid].rtcpport)
	{
		if (cmChannelSetRTCPAddress(H323outChan(callHandle)[sid].hsChan,
			htonl(H323outChan(callHandle)[sid].rtcpip), 
			H323outChan(callHandle)[sid].rtcpport) < 0)
		{
			NETERROR(MH323, 
				("%s SetRTCPAddress returned error\n", fn));
		}
	}

#if 0
	if (cmChannelSetAddress(H323outChan(callHandle)[sid].hsChan,
		chanInfo->ip, chanInfo->port) < 0)
	{
		NETERROR(MH323, 
				("%s SetRTPAddress returned error\n", fn));
	}
#endif

	if (chanInfo->dataType == cmCapAudio)
	{
		cmMeiEnter(UH323Globals()->hApp);

		nodeId = pvtAddRoot(hVal,
				hSyn,
				0,
				NULL);
	
		if (nodeId < 0)
		{
			NETERROR(MH323,
				("%s pvtAddRoot returns error\n", fn));
			cmMeiExit(UH323Globals()->hApp);
			return 0;
		}

		if (pvtBuildByPath(hVal, nodeId,
				"audioData",
				0,
				NULL) < 0)
		{
			NETERROR(MH323,
				("%s pvtBuildByPath returns error\n", fn));
		}

		if (pvtAddTree(hVal, pvtChild(hVal, nodeId),
			hVal, chanInfo->dataTypeHandle) < 0)
		{
			NETERROR(MH323,
				("%s pvtAddTree returns error\n", fn));
		}


		if (cmChannelOpenDynamic(H323outChan(callHandle)[sid].hsChan,
			pvtChild(hVal, nodeId),
			NULL,
			NULL,
			FALSE) < 0)
		{
			NETERROR(MH323, 
					("%s OpenDynamic returned error\n", fn));
		}
		else
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s success\n", fn));
		}

		pvtDelete(hVal,nodeId);

		cmMeiExit(UH323Globals()->hApp);

	}
	else if (chanInfo->dataType == cmCapData)
	{
		if(chanInfo->dataTypeHandle == 0) /* if iwf scenario */
		{
			// use config.val
			if (cmChannelOpen(H323outChan(callHandle)[sid].hsChan,
				"t38fax", NULL, NULL, 144) < 0)
			{
				NETERROR(MH323, 
					("%s ChannelOpen returned error\n", fn));
			}
		}
		else
		{
			int rv;
			cmMeiEnter(UH323Globals()->hApp);
			nodeId = pvtAddRoot(hVal,
					hSyn,
					0,
					NULL);
		
			if (nodeId < 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				cmMeiExit(UH323Globals()->hApp);
				return 0;
			}

			if (pvtBuildByPath(hVal, nodeId,
					"data.application",
					0,
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}

			if (pvtSetTree(hVal, pvtChild(hVal,pvtChild(hVal,nodeId)),
				peerhVal, chanInfo->dataTypeHandle) < 0)
			{
				NETERROR(MH323,
					("%s pvtAddTree returns error\n", fn));
			}
			if (pvtBuildByPath(hVal, nodeId,
					"data.maxBitRate",
					144,
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}
				
			if ((rv = cmChannelOpenDynamic(H323outChan(callHandle)[sid].hsChan,
				pvtChild(hVal,nodeId),
				NULL,
				H323outChan(callHandle)[0].hsChan,
				FALSE) )< 0)
			{
				NETERROR(MH323, 
						("%s OpenDynamic returned error %d\n", fn,rv));
			}
			else
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s success\n", fn));
			}

			pvtDelete(hVal,nodeId);
			cmMeiExit(UH323Globals()->hApp);
		}
	}
	else if (chanInfo->dataType == cmCapVideo)
	{
		int rv;
		cmMeiEnter(UH323Globals()->hApp);
#if 0
		pvtPrintStd(hVal,chanInfo->dataTypeHandle,51);
		pvtPrintStd(hVal,chanInfo->dataTypeHandle,62);
		if (pvtAddTree(hVal, pvtChild(hVal, nodeId),
			hVal, chanInfo->dataTypeHandle) < 0)
		{
			NETERROR(MH323,
				("%s pvtAddTree returns error\n", fn));
		}

#endif
		nodeId = pvtAddRoot(hVal,
				hSyn,
				0,
				NULL);
	
		if (nodeId < 0)
		{
			NETERROR(MH323,
				("%s pvtAddRoot returns error\n", fn));
			cmMeiExit(UH323Globals()->hApp);
			return 0;
		}

		if (pvtBuildByPath(hVal, nodeId,
				"videoData",
				0,
				NULL) < 0)
		{
			NETERROR(MH323,
				("%s pvtBuildByPath returns error\n", fn));
		}

		if (pvtSetTree(hVal, pvtChild(hVal,nodeId),
			hVal, chanInfo->dataTypeHandle) < 0)
		{
			NETERROR(MH323,
				("%s pvtAddTree returns error\n", fn));
		}
			
		// dataType has to be child of videoData
		if ((rv = cmChannelOpenDynamic(H323outChan(callHandle)[sid].hsChan,
			pvtChild(hVal,nodeId),
			NULL,
		//	NULL,
			H323outChan(callHandle)[0].hsChan,
			FALSE) )< 0)
		{
			NETERROR(MH323, 
					("%s OpenDynamic returned error %d\n", fn,rv));
		}
		else
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s success\n", fn));
		}

		pvtDelete(hVal,nodeId);
		cmMeiExit(UH323Globals()->hApp);

	}
#if 0
	else if (chanInfo->dataType == cmCapData)
	{
		// use config.val
	}
	else if (chanInfo->dataType == cmCapNonStandard)
	{
		if (pvtBuildByPath(hVal, nodeId,
				"nonStandard",
				0,
				NULL) < 0)
		{
			NETERROR(MH323,
				("%s pvtBuildByPath returns error\n", fn));
		}
	}
#endif
	
	return 0;
}

int
GkCallExtractCallerPhone(
	HCALL   hsCall,
	char	*phone,
	int		phoneLen,
	unsigned char *cgpnType
)
{
	char fn[] = "GkCallExtractCallerPhone():";
	int 				addrlen;
	cmAlias				cmPhoneNum = {0};
	int 				index = 0;
	char				aliasstr[256];

	addrlen = sizeof(cmAlias);
	cmPhoneNum.length = 256;
	cmPhoneNum.string =  aliasstr;
	cmPhoneNum.type = cmAliasTypePartyNumber;
	memset(cmPhoneNum.string,0,256);

	if (cmCallGetParam(hsCall,
				cmParamCallingPartyNumber,
				0,
				&addrlen,
				(char *)&cmPhoneNum) >=0)
	{
		*cgpnType = (cmPhoneNum.pnType & 0x70) >> 4;
		goto _set_phone;	
	}
	else
	{
		*cgpnType = cmPartyNumberPublicUnknown;
	}

	addrlen = sizeof(cmAlias);
	cmPhoneNum.length = 256;
	cmPhoneNum.string =  aliasstr;
	memset(cmPhoneNum.string,0,256);

	while (cmCallGetParam(hsCall,
			cmParamSourceAddress, 
			index++, 
			&addrlen,
			(char *)&cmPhoneNum) >= 0)
	{
		if (cmPhoneNum.type != cmAliasTypeE164)
		{
			 goto _continue;
		}

		goto _set_phone;

	_continue:
		addrlen = sizeof(cmAlias);
		cmPhoneNum.length = 256;
		cmPhoneNum.string =  aliasstr;
		memset(cmPhoneNum.string,0,256);

	}

	NETDEBUG(MSCC, NETLOG_DEBUG4,
		("%s Unable to retrieve Calling Party Number\n",fn));

	return -1;

_set_phone:
	NETDEBUG(MH323, NETLOG_DEBUG4, 
		("%s Found Alias %s in Call\n", fn, cmPhoneNum.string));

	if(cmPhoneNum.length)
	{
		strncpy(phone ,cmPhoneNum.string, phoneLen-1);
		phone[phoneLen-1] = '\0';
	}

	return 1;
}

// return the RAS Reason as a return value
// RAS reason is not set inside this function,
// it is set inside the uh323cb.c file, where the
// RAS request is actually being rejected.
// In that case, the hsCall may be passed as NULL
// h225Reason and rasReason fields: valid values are 
// off by 1 (one more than actual value). A value of
// 0 means that they are uninitialized. Same for cause.
int
GkCallDropReason(HCALL   hsCall,
				int callerror,
				int h225Reason,
				int rasReason,
				int	cause)
{
	char fn[] = "GkCallDropReason():";
	int reason = -1, rReason = 0;

	// rasReason cannot be conveyed as we cannot relay ras reasons
	// from one side to another
	// h225 reason can be conveyed/converted

	GkMapCallErrorToH225nRasReason( callerror, &reason, &rReason );
	reason--;
	// h225Reason overrides any of the above reasons
	if (h225Reason > 0)
	{
		reason = h225Reason - 1;
	}

	if (hsCall && (reason >= 0))
	{
		if (cmCallSetParam(hsCall,
				 cmParamReleaseCompleteReason, 
				 0, 
				 reason,
				 NULL) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Could not set reason\n", fn));
		}
	}

	if(cause == 0)
	{
		cause = GkMapCallErrorToCauseCode(callerror)-1;
	}
	else
	{
		cause = cause - 1;
	}

	if(hsCall && (cause >= 0) && (cause < Cause_eMax))
	{
		if (cmCallSetParam(hsCall,
				 cmParamReleaseCompleteCause,
				 0,
				 cause,
				 NULL) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Could not set reason\n", fn));
		}
	}

	return rReason;
}

// In case there is no reason in call,
// which may be true if its just a simulated disconnect
// or the sender does nt put in some reason,
// deduce a reason
int
BillCallDropInferReason(HCALL   hsCall, CallHandle *callHandle, int stateMode, 
	cmReasonType reason, int cause)
{
	char fn[] = "BillCallDropInferReason():";
	int callerror = SCC_errorNone;

	if (callHandle->callError)
	{
		return callHandle->callError;	/* we have the callError, drop initiated by us */
	}
	
	callerror = BillCallDropReason(reason);

	// cause to be decremented before interpreted
	cause --;

	if (callerror == SCC_errorNone)
	{
		//if (callHandle->callConnectTime)
		//{
		//}

		switch (stateMode)
		{
		case cmCallStateModeDisconnectedLocal:
			callerror = SCC_errorLocalDisconnect;
			break;
		case cmCallStateModeDisconnectedIncompleteAddress:
			callerror = SCC_errorIncompleteAddress;
			break;
		case cmCallStateModeDisconnectedUnreachable:
			if ((cause >= 0) && (cause!= Cause_eNormalCallClearing) &&
				(timedef_iszero(&callHandle->callConnectTime)) &&
				(callHandle->callSource))
			{
				if((callerror = cause2SCCError(cause)) == SCC_errorNone)
				{
					callerror = SCC_errorDestRelComp;	
				}
			}
			else 
			{
				callerror = SCC_errorDisconnectUnreachable;
			}
			break;
		case cmCallStateModeDisconnectedBusy:
		case cmCallStateModeDisconnectedReject:
		// If we get release complete before connect on destiantion leg
			if ((cause >= 0)&& (cause!= Cause_eNormalCallClearing) &&
				(timedef_iszero(&callHandle->callConnectTime)) &&
				(callHandle->callSource))
			{
				if((callerror = cause2SCCError(cause)) == SCC_errorNone)
				{
					callerror = SCC_errorDestRelComp;	
				}
			}
			else 
			{
				callerror = SCC_errorBusy;
			}
			break;
		case cmCallStateModeDisconnectedUnknown:
			if ((callHandle->callSource == 0) &&
				(timedef_iszero(&callHandle->callConnectTime)))
			{
				// call was disconnected before answered
				callerror = SCC_errorAbandoned;
			}
			else
			{
				callerror = SCC_errorUndefinedReason;
			}
			break;
		case cmCallStateModeDisconnectedNormal:
			if ((callHandle->callSource == 0) &&
				(timedef_iszero(&callHandle->callConnectTime)))
			{
				// call was disconnected before answered
				callerror = SCC_errorAbandoned;
			}
			else
			{
				callerror = SCC_errorNone;
			}
			break;
		}
	}

	return callerror;
}

int cause2SCCError(int cause)
{
	int rc;

	switch(cause)
	{
		case Cause_eUnassignedNumber:
		case Cause_eNoRouteTransit:
		case Cause_eNoRouteDest:
		case Cause_eNonSelectedUser:
		case Cause_eDestinationOutOfOrder:
		case Cause_eNormalUnspecified:
			rc = SCC_errorDestNoRoute;
			break;
		case Cause_eUserBusy:
			rc = SCC_errorBusy;
			break;
		case Cause_eInvalidNumberFormat:
			rc = SCC_errorInvalidPhone;
			break;
		case Cause_eUserNotResponding:
		case Cause_eUserNoAnswer:
		case Cause_eSubscriberAbsent:
			rc = SCC_errorTemporarilyUnavailable;
			break;
		case Cause_eInvalidTransit:
		case Cause_eInterworking:
			rc = SCC_errorUndefinedReason;
			break;
		case Cause_eNoCircuitAvailable:
		case Cause_eTemporaryFailure:
		case Cause_eNoResource:	
		case Cause_eNetworkOutOfOrder:
		case Cause_eSwitchingEquipmentCongestion:
			rc = SCC_errorGatewayResourceUnavailable;
			break;
		case Cause_eCallRejected:
		case Cause_eIncomingClassBarredInCUG:
		case Cause_eBearerCapNotAuthorized:
			rc = SCC_errorDestBlockedUser;
			break;
		case Cause_eNumberChanged:
			rc = SCC_errorDestGone;
			break;
		default:
			// we don't know, must direct user to look at ISDN cause code.
			rc = SCC_errorNone;
			break;
	}
	return rc;
}

// COnvert reason in call to IWF reason
int
BillCallDropReason(cmReasonType reason)
{
	char fn[] = "BillCallDropReason():";
	int callerror;

	// Decrement reason !!
	reason --;

	switch (reason)
	{	
	case -1:
		// No reason code present
		callerror = SCC_errorNone;
		break;
    case cmReasonTypeUnreachableDestination:
    case cmReasonTypeUnreachableGatekeeper:
		callerror = SCC_errorDestinationUnreachable;
		break;
    case cmReasonTypeNoBandwidth:
		callerror = SCC_errorInadequateBandwidth;
		break;
    case cmReasonTypeDestinationRejection:
    case cmReasonTypeAdaptiveBusy:
    case cmReasonTypeInConf:
		callerror = SCC_errorBusy;
		break;
    case cmReasonTypeNoPermision:
    case cmReasonTypeSecurityDenied:
    case cmReasonTypeCallerNotregistered:
		callerror = SCC_errorDestBlockedUser;
		break;
    case cmReasonTypeBadFormatAddress:
		callerror = SCC_errorInvalidPhone;
		break;
    case cmReasonTypeInvalidRevision:
    case cmReasonTypeGatekeeperResource:
    case cmReasonTypeCallForwarded:
    case cmReasonTypeRouteCallToMC:
    case cmReasonTypeRouteCallToGatekeeper:
    case cmReasonTypeFacilityCallDeflection:
	case cmReasonTypeConferenceListChoice:
    case cmReasonTypeNewConnectionNeeded:
    case cmReasonTypeGatewayResource:
		callerror = SCC_errorGatewayResourceUnavailable;
		break;

    case cmReasonTypeCalledPartyNotRegistered:
		callerror = SCC_errorDestNoRoute;
		break;
    case cmReasonTypeUndefinedReason:
		callerror = SCC_errorUndefinedReason;
		break;
    case cmReasonTypeNoH245:
    case cmReasonTypeStartH245:
		callerror = SCC_errorH245Incomplete;
		break;
	default:
		callerror = SCC_errorUndefinedReason;
		break;
	}

	return callerror;
}

// The return value may be DIFFERENT from what
// is set in pevt
int
CallSetEvtFromH323StackState(int state, int origin, int *pevt)
{
	int evt = SCC_evtUnknown;

	if (origin)
	{
		switch (state)
		{
		case cmCallStateDialtone:
			evt = SCC_evt323SetupTx;
			break;
   	 	case cmCallStateProceeding:
			evt = SCC_evt323ProcRx;
			break;
    	case cmCallStateRingBack:
			evt = SCC_evt323AlertRx;
			break;
    	case cmCallStateConnected:
			evt = SCC_evt323ConnRx;
			break;
    	case cmCallStateOffering:
			evt = SCC_evtInvalid;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (state)
		{
		case cmCallStateDialtone:
			evt = SCC_evtInvalid;
			break;
   	 	case cmCallStateProceeding:
			evt = SCC_evtInvalid;
			break;
    	case cmCallStateRingBack:
			evt = SCC_evtInvalid;
			break;
    	case cmCallStateConnected:
			evt = SCC_evt323ConnTx;
			break;
    	case cmCallStateOffering:
			evt = SCC_evt323SetupRx;
			break;
		default:
			break;
		}
	}

	if (pevt &&
		((*pevt == SCC_evtNone) || (evt != SCC_evtUnknown)))
	{
		*pevt = evt;
	}

	return evt;
}

void GkMapCallErrorToH225nRasReason(
	int callerror,		/* input */
	int *h225Reason,	/* output */
	int *rasReason )	/* output */
{
	int reason = -1;
	short arjRejectReason = cmRASReasonRequestDenied;
	short lrjRejectReason = cmRASReasonRequestDenied;

	switch (callerror)
	{
	case SCC_errorNetwork:
	case SCC_errorDestinationUnreachable:
	case SCC_errorDisconnectUnreachable:
		reason = cmReasonTypeUnreachableDestination;
		arjRejectReason = cmRASReasonCalledPartyNotRegistered;
		lrjRejectReason = cmRASReasonNotRegistered;
		break;
	case SCC_errorBusy:
	case SCC_errorTemporarilyUnavailable:
		reason = cmReasonTypeDestinationRejection;
		arjRejectReason = cmRASResourceUnavailable;
		lrjRejectReason = cmRASReasonNotRegistered;
		break;
	case SCC_errorAbandoned:
		reason = -1;
		arjRejectReason = 0;
		lrjRejectReason = 0;
		break;
	case SCC_errorLocalDisconnect:
	case SCC_errorGeneral:
	case SCC_errorSipInternalError:
	case SCC_errorSipNotImplemented:
		reason = cmReasonTypeUndefinedReason;
		arjRejectReason = cmRASReasonUndefined;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	case SCC_errorBlockedUser:
	case SCC_errorDestBlockedUser:
	case SCC_errorSipAuthRequired:
	case SCC_errorSipProxyAuthRequired:
	case SCC_errorSipForbidden:
		reason = cmReasonTypeNoPermision;
		arjRejectReason = cmRASReasonInvalidPermission;
		lrjRejectReason = cmRASReasonInvalidPermission;
		break;
	case SCC_errorNoRoute:
	case SCC_errorDestNoRoute:
	case SCC_errorHairPin:
	case SCC_errorRejectRoute:
		reason = cmReasonTypeCalledPartyNotRegistered;
		arjRejectReason = cmRASReasonCalledPartyNotRegistered;
		lrjRejectReason = cmRASReasonNotRegistered;
		break;
	case SCC_errorInvalidPhone:
	case SCC_errorIncompleteAddress:
		reason = cmReasonTypeBadFormatAddress;
		arjRejectReason = cmRASIncompleteAddress;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	case SCC_errorResourceUnavailable:
	case SCC_errorNoVports:
	case SCC_errorH323MaxCalls:
		reason = cmReasonTypeGatekeeperResource;
		arjRejectReason = cmRASResourceUnavailable;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	case SCC_errorNoPorts:
	case SCC_errorSipServiceUnavailable:
	case SCC_errorGatewayResourceUnavailable:
	case SCC_errorDestTimeout:
		reason = cmReasonTypeGatewayResource;
		//arjRejectReason should be for v4 cmRASReasonExceedsCallCapacity
		arjRejectReason = cmRASResourceUnavailable;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	case SCC_errorUndefinedReason:
	case SCC_errorMswInvalidEpId:
	case SCC_errorMswRouteCallToGk:
	case SCC_errorMswCallerNotRegistered:
	case SCC_errorH323Internal:
	case SCC_errorH323Protocol:
	case SCC_errorNoCallHandle:
		reason = cmReasonTypeUndefinedReason;
		arjRejectReason = cmRASReasonUndefined;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	case SCC_errorInadequateBandwidth:
	case SCC_errorFCE:
	case SCC_errorFCECallSetup:
		reason = cmReasonTypeNoBandwidth;
		arjRejectReason = cmRASReasonUndefined;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	case SCC_errorH245Incomplete:
		reason = cmReasonTypeNoH245;
		arjRejectReason = cmRASReasonUndefined;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	case SCC_errorNone:
		break;
	default:
		reason = cmReasonTypeUndefinedReason;
		arjRejectReason = cmRASReasonUndefined;
		lrjRejectReason = cmRASReasonUndefined;
		break;
	}

	if ((maplrjreason == 1) && (lrjRejectReason == cmRASReasonUndefined)) 
	{
		lrjRejectReason = cmRASReasonRequestDenied;
	}

	if (h225Reason != NULL)
	{
		*h225Reason = reason + 1;
	}
	if (rasReason != NULL)
	{
		*rasReason = (lrjRejectReason<<16) | arjRejectReason;
	}
}


int 	/* return casue code */
GkMapCallErrorToCauseCode(
	int callerror )		/* input, MSW callError */
{
	char fn[]= "GkMapCallErrorToCauseCode():";
	int cause = getH323Code(callerror);
	return ( cause + 1 );
}

int 	/* return casue code + 1 */
GkMapCauseCode(
	int cause )		/* input, orignal cause code + 1*/
{
    int isdncode = cause - 1;	// actual cause code

    if (VALID_ISDNCODE(isdncode))
    {
        return (codemap[isdncode].isdncode + 1);
    }

    return cause;
}
