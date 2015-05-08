#include "uh323.h"
#include "uh323cb.h"

#include "gis.h"
#include "codecs.h"

#include <h245.h>
#include <malloc.h>
#include "gk.h"
#include "uh323proto.h"

#define MSW_MAX_LEN_NONSTANDARD_DATA	256

/*(MAX_FASTSTART_CHANNELS+1)*MAX_LCN_S <= 64K(the number of possible LCNs)*/
#define MAX_FASTSTART_CHANNELS  	511
#define MAX_LCN_S               	128
#define MAX_CHANNELS_PER_CALL  		10

/* Maximum number of session ids that fast start can handle */
#define MAX_FS_SESSION_ID 10

struct nonStandardCodecTab{
	int 			codec;
	cmNonStandard	h221;
	char 			data[MSW_MAX_LEN_NONSTANDARD_DATA];
	int				dataLength;
}mswNonStandardTable[] = {
	{ CodecG723AR, { 181, 0, 18 }, "G7231ar", 7 },
	{ -1, {0,0,0}, "", 0 }
};

struct
{
     char channelName[25];
     int codecType;
}  Codecs[MAX_CODECS] = {
     { "g711Ulaw64k", CodecGPCMU },
     { "g711Alaw64k", CodecGPCMA },
     { "g7231", CodecG7231 },
     { "g728", CodecG728 },
     { "g729", CodecG729},
     { "g729AnnexA", CodecG729A },
     { "rtpTelephonyEvent", CodecRFC2833 },
	 { "t38fax", T38Fax },
     { "g729wAnnexB", CodecG729B },
     { "g729AnnexAwAnnexB", CodecG729AwB },
	 { "nonStandard", CodecNonStandard },
	 { "nonStandardIdentifier", CodecNonStandard },
};

char *
ChannelName(int codecType)
{
	int i;

     for (i=0; i< MAX_CODECS; i++)
     {
          if (codecType == Codecs[i].codecType)
          {
               return Codecs[i].channelName;
          }
     }

	return NULL;
}


int
ChannelCodec(char *name)
{
     int i;

     if (name == NULL)
     {
        return -1;
     }

     for (i=0; i< MAX_CODECS; i++)
     {
          if (!strcasecmp(name, Codecs[i].channelName))
          {
               return Codecs[i].codecType;
          }
     }

     return -1;
}



/*	Returns -1 on failure nodeId on success
* creates a new Tree. callers should do pvtDelete  on the root */
int
createDataHandle(const H323RTPSet *pRtpSet)
{
	static char		fn[] = "createDataHandle";
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	int 			nodeId,rtId;

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Entering codectype = %d param = %d\n", 
		fn,pRtpSet->codecType,pRtpSet->param));

	if((pRtpSet->codecType >=96) && (pRtpSet->codecType <=105))
	{
		// Suppress error messages for rfc2833 dtmf
		return -1;
	}
	rtId = getDataHandle(pRtpSet);
	if (rtId < 0)
	{
		NETERROR(MH323,
			("%s getDataHandle returns error\n", fn));
		return rtId;
	}
	cmMeiEnter(UH323Globals()->hApp);
	nodeId = pvtAddRoot(hVal, hSyn, 0,NULL);
	if (nodeId < 0)
	{
		NETERROR(MH323,
			("%s pvtAddRoot returns error\n", fn));
		goto _return;
	}
	pvtSetTree(hVal,nodeId,hVal,rtId);

_return:
	cmMeiExit(UH323Globals()->hApp);
	return nodeId;
}

/* returns a pvtTree - callers should not do pvtDelete */
int
getDataHandle(const H323RTPSet *pRtpSet)
{
	static char		fn[] = "getDataHandle";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			rtId;
	int 			j,i;
	int				param = pRtpSet->param;

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Entering codectype = %d param = %d flags = 0x%x\n", 
		fn,pRtpSet->codecType,pRtpSet->param,pRtpSet->flags));

	if ( MSW_IS_NONSTANDARD_CODEC(pRtpSet->codecType) )
	{
		for( i = 0; mswNonStandardTable[i].codec != -1; i++ )
		{
			if ( mswNonStandardTable[i].codec == pRtpSet->codecType )
			{
				int rtId;

				if ( i >= MAXPARAM )
				{
					NETERROR(MH323,
							("%s mswNonStandardTable got too many entries, %d\n", fn, i));
					return -1;
				}
				if ( mswNonStandard[i] )
				{
					rtId = mswNonStandard[i];
				}
				else
				{
					int nsRootId, h221Id;
					cmMeiEnter(UH323Globals()->hApp);
					rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
					if (rtId< 0)
					{
						NETERROR(MH323,
							("%s pvtAddRoot returns error\n", fn));
						goto _return;
					}
					
					if ( (nsRootId = pvtBuildByPath(hVal,rtId, "audioData.nonStandard", 0, NULL)) < 0)
					{
						NETERROR(MH323,
							("%s pvtBuildByPath returns error\n", fn));
						rtId = -1;
						goto _return;
					}
			        h221Id = pvtBuildByPath(hVal, nsRootId, "nonStandardIdentifier.h221NonStandard", 0, NULL);
			        pvtBuildByPath(hVal, nsRootId, "data", mswNonStandardTable[i].dataLength,mswNonStandardTable[i].data);
			        pvtBuildByPath(hVal, h221Id, "t35CountryCode", mswNonStandardTable[i].h221.t35CountryCode, NULL);
			        pvtBuildByPath(hVal, h221Id, "t35Extension", mswNonStandardTable[i].h221.t35Extension, NULL);
			        if (pvtBuildByPath(hVal, h221Id, "manufacturerCode", mswNonStandardTable[i].h221.manufacturerCode, NULL) < 0 )
					{
						NETERROR(MH323,
							("%s pvtBuildByPath returns error\n", fn));
						rtId = -1;
						goto _return;
					}
					mswNonStandard[i] = rtId;
					cmMeiExit(UH323Globals()->hApp);
				}
				return rtId;
			}
		}
		NETERROR(MH323,
			("%s Unknown non-std codec type %d\n", fn,pRtpSet->codecType));
		return -1;
	}

	if((param <= 0) ||  (param > MAXPARAM))
	{
		NETERROR(MH323,
			("%s Invalid param %d codec %d\n", fn,param,pRtpSet->codecType));
		return -1;

	}
	cmMeiEnter(UH323Globals()->hApp);

	switch(pRtpSet->codecType) 
	{
	case CodecG729A:
		if(g729a[param - 1])
		{
			rtId = g729a[param - 1];	
		}
		else 
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s G729A CREATING NEW NODEID param = %d\n", 
				fn,param));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal,rtId, 
					"audioData.g729AnnexA",
					 pRtpSet->param,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			g729a[param - 1] = rtId;
		}
		break;

		case CodecG729: 
		if(g729[param - 1])
		{
			rtId = g729[param - 1];	
		}
		else 
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s G729 CREATING NEW NODEID param = %d\n", 
				fn,param));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal, rtId,
					"audioData.g729",
				pRtpSet->param,
				NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			g729[param - 1] = rtId;
		}
		break;

	case CodecG7231: 
		// if no silence suppression
		if(pRtpSet->flags & RTP_FLAGS_SS)
		{
			if(g723SS[param - 1])
			{
				rtId = g723SS[param - 1];	
			}
			else 
			{
				NETDEBUG(MH323, NETLOG_DEBUG1, 
					("%s G723 CREATING NEW NODEID param = %d\n", 
					fn,param));
				rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
				if (rtId< 0)
				{
					NETERROR(MH323,
						("%s pvtAddRoot returns error\n", fn));
					goto _return;
				}

				if (pvtBuildByPath(hVal, rtId,
						"audioData.g7231.maxAl-sduAudioFrames",
						pRtpSet->param,
						NULL) < 0)
				{
					NETERROR(MH323,
						("%s pvtBuildByPath returns error\n", fn));
					rtId = -1;
					goto _return;
				}
				if (pvtBuildByPath(hVal, rtId,
						"audioData.g7231.silenceSuppression",
						1,
						NULL) < 0)
				{
					NETERROR(MH323,
						("%s pvtBuildByPath returns error\n", fn));
					rtId = -1;
					goto _return;
				}
				g723SS[param - 1]  = rtId;
			}
		}
		else {
			if(g723[param - 1])
			{
				rtId = g723[param - 1];	
			}
			else 
			{
				NETDEBUG(MH323, NETLOG_DEBUG1, 
					("%s G723 CREATING NEW NODEID param = %d\n", 
					fn,param));
				rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
				if (rtId< 0)
				{
					NETERROR(MH323,
						("%s pvtAddRoot returns error\n", fn));
					goto _return;
				}

				if (pvtBuildByPath(hVal, rtId,
						"audioData.g7231.maxAl-sduAudioFrames",
						pRtpSet->param,
						NULL) < 0)
				{
					NETERROR(MH323,
						("%s pvtBuildByPath returns error\n", fn));
					rtId = -1;
					goto _return;
				}
				if (pvtBuildByPath(hVal, rtId,
						"audioData.g7231.silenceSuppression",
						0,
						NULL) < 0)
				{
					NETERROR(MH323,
						("%s pvtBuildByPath returns error\n", fn));
					rtId = -1;
					goto _return;
				}
				g723[param - 1]  = rtId;
			}
		}
		break;

	case CodecGPCMA: 
		if(gcpma[param - 1])
		{
			rtId = gcpma[param - 1];	
		}
		else 
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s G711ALaw CREATING NEW NODEID param = %d\n", 
				fn,param));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal, rtId,
					"audioData.g711Alaw64k",
				pRtpSet->param,
				NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			gcpma[param - 1] = rtId;
		}
		break;

	case CodecGPCMU: 
		if(gcpmu[param - 1])
		{
			rtId = gcpmu[param - 1];	
		}
		else 
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s G711ULaw CREATING NEW NODEID param = %d\n", 
				fn,param));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal, rtId,
					"audioData.g711Ulaw64k",
					pRtpSet->param,
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			gcpmu[param - 1] = rtId;
		}
		break;
		case CodecG728: 
		if(g728[param - 1])
		{
			rtId = g728[param - 1];	
		}
		else 
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s G728 CREATING NEW NODEID param = %d\n", 
				fn,param));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal, rtId,
					"audioData.g728",
				pRtpSet->param,
				NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			g728[param - 1] = rtId;
		}
		break;
	case CodecG729B:
		if(g729b[param - 1])
		{
			rtId = g729b[param - 1];	
		}
		else 
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s G729B CREATING NEW NODEID param = %d\n", 
				fn,param));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal,rtId, 
					"audioData.g729wAnnexB",
					 pRtpSet->param,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			g729b[param - 1] = rtId;
		}
		break;
	case CodecG729AwB:
		if(g729awb[param - 1])
		{
			rtId = g729awb[param - 1];	
		}
		else 
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s G729AwB CREATING NEW NODEID param = %d\n", 
				fn,param));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal,rtId, 
					"audioData.g729AnnexAwAnnexB",
					 pRtpSet->param,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			g729awb[param - 1] = rtId;
		}
		break;
	default:
		NETERROR(MH323,
			("%s Unknown codec type %d\n",fn,pRtpSet->codecType));
		rtId = -1;
		break;
	}

_return:
	cmMeiExit(UH323Globals()->hApp);

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s dataHandle = %d\n", 
		fn,rtId));
	return rtId;
}

/*	Does not modify callHandle
*	Called for outgoing setup
*/
void
uh323FastStartSetupInit(const CallHandle *callHandle)
{
	static char 		fn[] = "uh323FastStart ";
	cmFastStartMessage 	fsMessage;
	HPVT hVal = 		cmGetValTree(UH323Globals()->hApp);
	int 				dataTypeHandle;
	int 				i = 0,j;
	H323RTPSet			*pRtpSet;
	int					dataHandle;
	HPST hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp, 
					"capData");
	int nodeId;
	int					rx,tx;
	cmFastStartChannel	*fsChannel;

	memset(&fsMessage,0,sizeof(fsMessage));

	/* One audio channels */
	fsMessage.partnerChannelsNum = 1;

	/* Just one possiblity for an audio channel */
	fsMessage.partnerChannels[0].type = cmCapAudio;

	/* Transmit */
	for(i = 0,j=0, pRtpSet = H323remoteSet(callHandle),
			fsChannel= fsMessage.partnerChannels[0].transmit.channels;
		j < H323nremoteset(callHandle); 
		++j,++pRtpSet)
	{
#ifndef _cisco
		if((dataHandle = createDataHandle(pRtpSet))<0)
		{
			NETDEBUG(MH323,NETLOG_DEBUG4, 
				("%s CreateDataHandle Failed\n",fn));
			continue;
		}
#endif /* _cisco */
		fsChannel[i].rtp.port = 0;
		fsChannel[i].rtcp.ip = 0;
		fsChannel[i].rtcp.port = pRtpSet->rtpport+1;
		fsChannel[i].rtcp.ip = htonl(pRtpSet->rtpaddr);
#ifndef _cisco
		fsChannel[i].dataTypeHandle = dataHandle;
		fsChannel[i].channelName = NULL;
#else
		fsChannel[i].dataTypeHandle = -1;
		fsChannel[i].channelName = ChannelName(pRtpSet->codecType);
#endif /* _cisco */
		i++;
	}

	tx = i;
	fsMessage.partnerChannels[0].transmit.altChannelNumber = i;

	/* Receive */
	for(i = 0, j = 0, pRtpSet = H323remoteSet(callHandle), 
			fsChannel= fsMessage.partnerChannels[0].receive.channels;
		j < H323nremoteset(callHandle); ++j,++pRtpSet)
	{
#ifndef _cisco 
		if((dataHandle = createDataHandle(pRtpSet))<0)
		{
			NETDEBUG(MH323,NETLOG_DEBUG4, 
				("%s CreateDataHandle Failed\n",fn));
			continue;
		}
#endif /* _cisco */
		fsChannel[i].rtp.ip = htonl(pRtpSet->rtpaddr);
		fsChannel[i].rtp.port = pRtpSet->rtpport;
		fsChannel[i].rtcp.ip = htonl(pRtpSet->rtpaddr);
		fsChannel[i].rtcp.port = pRtpSet->rtpport +1;
#ifndef _cisco
		fsChannel[i].dataTypeHandle = dataHandle;
		fsChannel[i].channelName = NULL;
#else 
		fsChannel[i].dataTypeHandle = -1;
		fsChannel[i].channelName = ChannelName(pRtpSet->codecType);
#endif /* _cisco */
		i++;
	}

	rx = i;
	if (callHandle->handle.h323CallHandle.nremoteset &&
	    callHandle->handle.h323CallHandle.remoteSet &&
	    callHandle->handle.h323CallHandle.remoteSet[0].direction == SendOnly)
	{
		/* Disable reverse channel in the case of transmit only 
		 * sip SendOnly = 1, h323 cmCapTransmit = 2 
		 */
		fsMessage.partnerChannels[0].receive.altChannelNumber = 0;
	}
	else
	{
		fsMessage.partnerChannels[0].receive.altChannelNumber = i;
	}

	if (cmFastStartOpenChannels(H323hsCall(callHandle),
					&fsMessage) < 0)
	{
		NETERROR(MH323, ("%s Failed to Fast Start\n",fn));
	}


#ifndef _cisco
	cmMeiEnter(UH323Globals()->hApp);
	for(i = 0, fsChannel= fsMessage.partnerChannels[0].receive.channels;
		i<rx; ++i)
	{
		pvtDelete(hVal,fsChannel[i].dataTypeHandle);
	}

	for(i = 0, fsChannel= fsMessage.partnerChannels[0].transmit.channels;
		i<tx; ++i)
	{
		pvtDelete(hVal,fsChannel[i].dataTypeHandle);
	}
	cmMeiExit(UH323Globals()->hApp);
#endif
}

// called from cmEvCallfastStartSetup
int
uh323ExtractFSAudioRtpSet(H323RTPSet *pRtpSet,int *pcount,cmFastStartChannel fsChannel[])
{
	static char		fn[] = "uh323ExtractFSAudioRtpSet";
	int i,j;
	int g7231r = 1,g729ar = -1,
	g729r = -1, g711Ulaw64r = -1, 
		g711Alaw64r = -1;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	INT32 g7231rParam =  g7231Frames, g7231rSS = 0, 
		 g729rParam =g729Frames, g729arParam = g729Frames, 
		 g711Ulaw64rParam = g711Ulaw64Duration, 
		 g711Alaw64rParam = g711Alaw64Duration;
	int tmpCodec, codecType;
	char ipstr[24];
	int 	nChannels = 0;

	for (i = j = 0; j < *pcount; j++)
	{
#if 1
		pRtpSet[i].dataTypeHandle = fsChannel[j].dataTypeHandle;
        pRtpSet[i].param = -1;
#endif
		codecType = ChannelCodec(fsChannel[j].channelName);

		if (codecType < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4, 
				("%s Codec %s at index %d will be ignored\n",fn,
				fsChannel[j].channelName?fsChannel[j].channelName:"NULL", j));
			continue;
		}
		pRtpSet[i].codecType = codecType;
		pRtpSet[i].index = fsChannel[j].index;
		if(fsChannel[j].rtp.ip)
		{
			pRtpSet[i].rtpaddr = ntohl(fsChannel[j].rtp.ip);
			pRtpSet[i].rtpport = fsChannel[j].rtp.port;
		}
#if 0
/* clarent */
		else if(fsChannel[j].rtcp.ip)
		{
			pRtpSet[i].rtpaddr = ntohl(fsChannel[j].rtcp.ip);
			pRtpSet[i].rtpport = fsChannel[j].rtcp.port - 1;
		}
#endif

#if 1
		cmMeiEnter(UH323Globals()->hApp);
		switch (codecType)
		{
		case CodecG729A:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729AnnexA\n",fn));
			g729ar = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g729AnnexA",
					 NULL,
					&g729arParam, NULL) < 0)
				{
                    g729arParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729ar\n"));
				}
                pRtpSet[i].param = g729arParam;
			}

			break;

		case CodecG729:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729\n",fn));
			g729r = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g729",
					 NULL,
					&g729rParam, NULL) < 0)
				{
                    g729rParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729r\n"));
				}
                pRtpSet[i].param = g729rParam;
			}

			break;

		case CodecG7231:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.7231\n",fn));

			g7231r = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				HPVT hVal = cmGetValTree(UH323Globals()->hApp);
				INT32 valueMaxAl = 0;

				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g7231.maxAl-sduAudioFrames",
					NULL,
					&g7231rParam, NULL) < 0)
				{
                    g7231rParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No rx sduAudioFrames found in fst\n"));
				}
				pRtpSet[i].param = g7231rParam;

				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g7231.silenceSuppression", 
					NULL,
					&g7231rSS, NULL) < 0)
				{
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No rx silence suppression value in fst\n"));
				}
				else {
					pRtpSet[i].flags |= g7231rSS; 
				}
			}

			break;

		case CodecGPCMU:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.711Ulaw64\n",fn));
			g711Ulaw64r = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g711Ulaw64k",
					 NULL,
					&g711Ulaw64rParam, NULL) < 0)
				{
                    g711Ulaw64rParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("%s No interval found for g711Ulaw64r\n",fn));
				}
                pRtpSet[i].param = g711Ulaw64rParam;
			}

			break;

		case CodecGPCMA:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.711Alaw64\n",fn));
			g711Alaw64r = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g711Alaw64k",
					 NULL,
					&g711Alaw64rParam, NULL) < 0)
				{
                    g711Alaw64rParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("%s No interval found for g711Alaw64r\n",fn));
				}
                pRtpSet[i].param = g711Alaw64rParam;
			}

			break;

		case CodecG729B:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729AnnexB\n",fn));
			g729ar = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g729wAnnexB",
					 NULL,
					&g729arParam, NULL) < 0)
				{
                    g729arParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729b\n"));
				}
                pRtpSet[i].param = g729arParam;
			}

			break;

		case CodecG729AwB:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729AnnexAwB\n",fn));
			g729ar = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g729AnnexAwAnnexB",
					 NULL,
					&g729arParam, NULL) < 0)
				{
                    g729arParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729awb\n"));
				}
                pRtpSet[i].param = g729arParam;
			}

			break;

		case CodecG728:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.728\n",fn));
			g729ar = j;

			if (fsChannel[j].dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, fsChannel[j].dataTypeHandle,
					"audioData.g728",
					 NULL,
					&g729arParam, NULL) < 0)
				{
                    g729arParam = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g728\n"));
				}
                pRtpSet[i].param = g729arParam;
			}

			break;
		case CodecNonStandard:
			/* save all nonStandard channles */
			if (fsChannel[j].dataTypeHandle == -1)
			{
				cmMeiExit(UH323Globals()->hApp);
				continue;
			}
			else
			{
				int nonsNodeId, nsCodec;

				HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
				pRtpSet[i].dataTypeHandle = -1;
				nonsNodeId = pvtGetNodeIdByPath( hVal, fsChannel[j].dataTypeHandle, "audioData.nonStandard");
				if ( nonsNodeId >= 0 && (nsCodec = findNonStandardCodec(hVal, nonsNodeId )) > 0 )
				{
					pRtpSet[i].codecType = nsCodec;
				}
				else
				{
					/* only support known nonStandard codecs for now */
					cmMeiExit(UH323Globals()->hApp);
					continue;
				}
			}
			break;

		default:
		NETERROR(MH323, 
			("%s Unknown codec %d %s in faststart setup\n",
			fn,codecType,fsChannel[j].channelName));
			break;
		}
		cmMeiExit(UH323Globals()->hApp);
		DEBUG(MH323, NETLOG_DEBUG1,
			("%s RTP = %s port = %d codec = %d param = %d\n",
			fn,FormatIpAddress(pRtpSet[i].rtpaddr, ipstr),pRtpSet[i].rtpport,codecType,pRtpSet[i].param));
#endif
		i++;
	}

	if( i == 0 ) 
	{
		NETERROR(MH323,
			("%s Received FS setup with %d non-standard codecs\n",fn,*pcount));
		*pcount = i;
		return -1;
	}

	*pcount = i;

	return 0;
}

// Given callHandle and rtpset
// Called for outgoing Alerting and Connect 
// G729 codec if passed in rtpSet may get modified for iwf call a G729 variant 
// returns 0 when nothing is done, 1 when fst was added and -1 if protocol error
int uh323SetFastStartParam(CallHandle *callHandle, RTPSet rtpSet[],int nrtpset)
{
	static char			fn[] = "uh323SetFastStartParam";
    cmTransportAddress 	rtp, rtcp;
	int					rc = 0, codecmatch;
	int					i,j;
	int					index = -1;
    CodecType           codecType = -1;
	int					arrindex = -1;
	int					param = 0;
	char				ipstr[24];

	if(H323localSet(callHandle)[0].index == -1)
	{
		return 0;
	}	

	//Match the codecs
	for(i = 0,codecmatch = 0; i <H323nremoteset(callHandle) && !codecmatch; ++i)
	{
		for(j = 0;j<nrtpset;++j)
		{
			if(H323remoteSet(callHandle)[i].codecType == rtpSet[j].codecType)
			{
#ifdef _checkparam
/* There is a problem scenario. for a H323 ->Sip call - on 200Ok we have to
*	ensure that the channel parameters are same as that we sent out at Invite
*  Currently these parameters are initialised to default in all events from sip
*/
				if((rtpSet[j].param == 0) ||  
					(rtpSet[j].param ==H323localSet(callHandle)[i].param))
#endif
				{
					index = H323remoteSet(callHandle)[i].index;
					codecType = H323remoteSet(callHandle)[i].codecType;
					param = H323remoteSet(callHandle)[i].param;
					arrindex = j;
					codecmatch = 1;
					break;
				}
			}
		}
	}

	if(!codecmatch)
	{
		/* G729, 729A,729B equivalence for iwf calls */
		for(j = 0;j<nrtpset;++j)
		{
			if(rtpSet[j].codecType == CodecG729 )
			{
				for(i = 0,codecmatch = 0; i <H323nremoteset(callHandle) && !codecmatch; ++i)
				{

					switch(H323remoteSet(callHandle)[i].codecType)
					{
						case CodecG729A: 
						case CodecG729B: 
						case CodecG729AwB:
							index = H323remoteSet(callHandle)[i].index;
							codecType = H323remoteSet(callHandle)[i].codecType;
							param = H323remoteSet(callHandle)[i].param;
							arrindex = j;
							codecmatch =1;
							/* update the rtpset also */
							rtpSet[j].codecType = H323remoteSet(callHandle)[i].codecType;
							break;
						default:
							break;
					}
				}
			}
		}
	}

	if(!codecmatch || (index < 0) || (arrindex < 0) || (codecType < 0))
	{
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			("%s Could not find a matching Rx codec.\n",fn));
		return 0;
	}
	
    rtp.ip = htonl(rtpSet[arrindex].rtpaddr);
    rtp.port = rtpSet[arrindex].rtpport;
    rtcp.ip = htonl(rtpSet[arrindex].rtpaddr);
    rtcp.port  = rtpSet[arrindex].rtpport +1;

	NETDEBUG(MH323, NETLOG_DEBUG1, 
			("%s Matched Rx Codec(%d param = %d).ip/port = %s/%d \n",fn,
			codecType,rtpSet[arrindex].param,FormatIpAddress(rtp.ip, ipstr),rtp.port));

	if(cmFastStartChannelsAckIndex(H323hsCall(callHandle),index,&rtcp,&rtp)>= 0)
	{
            DEBUG(MH323, NETLOG_DEBUG1, ("Tx Accepted.\n"));
	}
	else
	{
		NETERROR(MH323,
			("%s TX Ack failed\n",fn));
		rc = -1;
		goto _return;
	}

    for(i = 0,codecmatch = 0; i <H323nlocalset(callHandle) && !codecmatch; ++i)
    {
        if(H323localSet(callHandle)[i].codecType == codecType)
        {
#ifdef _checkparam
        	if((param ==0) || (H323localSet(callHandle)[i].param == param))
#endif
			{
				index = H323localSet(callHandle)[i].index;
				rtp.ip = htonl(H323localSet(callHandle)[i].rtpaddr);
				rtp.port = H323localSet(callHandle)[i].rtpport;
				codecmatch = 1;
				break;
			}
        }
    }

    if(!codecmatch)
    {
        NETDEBUG(MH323, NETLOG_DEBUG1,
            ("%s Could not find a matching codec.\n",fn));
        return 0;
    }

	NETDEBUG(MH323, NETLOG_DEBUG1,
			("%s Matched Tx Codec(%d param = %d).ip/port = %s/%d \n",fn,
			codecType,param,FormatIpAddress(rtp.ip, ipstr),rtp.port));


    if(cmFastStartChannelsAckIndex(H323hsCall(callHandle),index,&rtcp,&rtp)>= 0)
    {
        DEBUG(MH323, NETLOG_DEBUG1, ("Tx Accepted.\n"));
    }
    else
    {
        NETERROR(MH323,
            ("uh323ProcessIncomingFastStartAudio:: TX Ack failed\n"));
        rc = -1;
	goto _return;
    }

	/* We can initialize the codecs here */

	if (cmFastStartChannelsReady(H323hsCall(callHandle)) < 0)
	{
		NETERROR(MH323, ("cmFastStartChannelsReady failed\n"));
		rc = -1;
	}
	else
	{
		rc = 1;
	}
	
_return:
	/* we should not do AckIndex twice */
	H323localSet(callHandle)[0].index = -1;
	return rc;
}

/*	Does not modify callHandle
*	Called for outgoing setup
*/
int
uh323CapSend(HCALL hsCall,H323RTPSet *pRtpSet,int ncodecs,int nodeId,int ecaps1, int vendor)
{
	static char 		fn[] = "uh323CapSend";
	int 				dataTypeHandle;
	int 				i = 0,j;
	int					dataHandle;
	cmCapStruct			*cmCapStructPtr;
	cmCapStruct			**capSet = 0;
    cmCapStruct 		**capAltPtr = 0,**capAltDtmf = 0;
    cmCapStruct 		***capSim = 0;
    cmCapStruct 		***capDesc[] = {capSim, 0};
	int					nCapSet = 0;
	int					rv = 0;
	int					capNodeId,capRoot;
	HPVT 				hVal;

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Entering ncodecs = %d\n", fn,ncodecs));
	if( nodeId)
	{
		int rtId;
		/* delete t38Fax from TCS, if send TCS to Lucent TNT */
		if ( (ecaps1 & ECAPS1_DELTCST38) || (ecaps1 & ECAPS1_DELTCSRFC2833) )
		{
			char buf[256];
			int i, capNodeId, t38NodeId;
			HPVT hVal = cmGetValTree(UH323Globals()->peerhApp);

			cmMeiEnter(UH323Globals()->hApp);
			for( i=1; i<=256; i++ )
			{
				sprintf(buf, "capabilityTable.%d.capability", i);
				if ((capNodeId = pvtGetNodeIdByPath(hVal, nodeId, buf )) <= 0)
				{
					break;
				}
				if (ecaps1 & ECAPS1_DELTCST38)
				{
					if ((t38NodeId = pvtGetNodeIdByPath(hVal, capNodeId, "receiveAndTransmitDataApplicationCapability.application.t38fax" )) > 0)
					{
						pvtDelete(hVal, capNodeId);
						continue;
					}
					if ((t38NodeId = pvtGetNodeIdByPath(hVal, capNodeId, "receiveDataApplicationCapability.application.t38fax" )) > 0)
					{
						pvtDelete(hVal, capNodeId);
						continue;
					}
					if ((t38NodeId = pvtGetNodeIdByPath(hVal, capNodeId, "transmitDataApplicationCapability.application.t38fax" )) > 0)
					{
						pvtDelete(hVal, capNodeId);
						continue;
					}
				}

				if (ecaps1 & ECAPS1_DELTCSRFC2833)
				{
					if ( (t38NodeId = pvtGetNodeIdByPath(hVal, capNodeId, "receiveRTPAudioTelephonyEventCapability" )) > 0)
					{
						pvtDelete(hVal, capNodeId);
						continue;
					}
					if ( (t38NodeId = pvtGetNodeIdByPath(hVal, capNodeId, "receiveRTPAudioToneCapability" )) > 0)
					{
						pvtDelete(hVal, capNodeId);
						continue;
					}
				}
			}
			cmMeiExit(UH323Globals()->peerhApp);
		}

#if 1	// mti
		if (nh323Instances > 1)
		{
			HPVT hVal = cmGetValTree(UH323Globals()->hApp);
			HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
			int tmpnodeId;

			cmMeiEnter(UH323Globals()->hApp);
			cmMeiEnter(UH323Globals()->peerhApp);
			tmpnodeId = pvtAddRootByPath(hVal, 
							cmGetSynTreeByRootName(UH323Globals()->hApp, "h245"),
							"request.terminalCapabilitySet", 0, NULL);

			if(tmpnodeId<0)
			{
				NETERROR(MH323,("%s pvtAddRootByPath failed\n",fn));
			}
			else if(pvtSetTree(hVal, tmpnodeId, peerhVal, nodeId) <0)
			{
				pvtDelete(hVal,tmpnodeId);
				NETERROR(MH323,("%s pvtSetTree failed\n",fn));
			}
		
			//pvtDelete(peerhVal,nodeId);	- deleted OUT of this fn

			nodeId = tmpnodeId;

			cmMeiExit(UH323Globals()->hApp);
			cmMeiExit(UH323Globals()->peerhApp);
		}
#endif
		if(cmCallSendCapability(hsCall,nodeId)< 0) 
		{
			NETERROR(MH323, ("%s Could not forward TCS %p\n",fn,hsCall));
#if 1	// mti
			if (nh323Instances > 1)
			{
				freeNodeTree(UH323Globals()->hApp, nodeId, 0);
			}
#endif
			return -1;
		}
#if 1	// mti
		if (nh323Instances > 1)
		{
			freeNodeTree(UH323Globals()->hApp, nodeId, 0);
		}
#endif
	} 
	else 
	{
		int vtnodecount;
		int sendRfc2833 = 0;
		if(ncodecs<=0)
		{
			NETERROR(MH323,("%s ncodecs is %d \n",fn,ncodecs));
			return -1;
		}

		cmMeiEnter(UH323Globals()->hApp);

		hVal = cmGetValTree(UH323Globals()->hApp);
		vtnodecount = pvtCurSize(hVal);
		//NETDEBUG(MH323,NETLOG_DEBUG4, ("%s Before %d\n",fn,vtnodecount));
		/* ncodecs +fax + last NULL */
		capSet = (cmCapStruct **)calloc(ncodecs+4,sizeof(cmCapStructPtr));
		memset(capSet,0,(ncodecs+4)*sizeof(cmCapStructPtr));
		/* Transmit */
		for(nCapSet = 0, j=0; j < ncodecs; ++j,++pRtpSet)
		{
			if ( pRtpSet->codecType != CodecRFC2833 )
			{
				/* Assuming cmCapabilitySend/Build do not destroy the tree */
				if((dataHandle = getDataHandle(pRtpSet))<0)
				{
					NETDEBUG(MH323,NETLOG_DEBUG4, 
						("%s getDataHandle Failed\n",fn));
					continue;
				}
				dataHandle = pvtChild(hVal,dataHandle);
				capSet[nCapSet] = (cmCapStruct *)malloc(sizeof(cmCapStruct));
				memset(capSet[nCapSet],0,sizeof(cmCapStruct));
				capSet[nCapSet]->capabilityHandle = dataHandle;
				capSet[nCapSet]->direction = toH323Direction(pRtpSet->direction);
				capSet[nCapSet]->type = cmCapAudio;
			}
			else
			{
				if ( !(ecaps1 & ECAPS1_DELTCSRFC2833) )
				{
					sendRfc2833 = 1;
				}
				continue;
			}
			nCapSet++;
		}

		/* Add Capability for fax */
		dataHandle = -1;
		if (vendor == Vendor_eLucentTnt)
		{
			dataHandle = getLucentFaxDataHandle();
		}
		else if (!(ecaps1 & ECAPS1_DELTCST38))
		{
			dataHandle = getFaxDataHandle();
		}
		if(dataHandle > 0)
		{
			dataHandle = pvtChild(hVal,dataHandle);
			capSet[nCapSet] = (cmCapStruct *)malloc(sizeof(cmCapStruct));
			memset(capSet[nCapSet],0,sizeof(cmCapStruct));
			capSet[nCapSet]->capabilityHandle = dataHandle;
			capSet[nCapSet]->direction = cmCapReceiveAndTransmit;
			capSet[nCapSet]->type = cmCapData;
			nCapSet++;
		}		

		capSim = (cmCapStruct ***)calloc(3,sizeof(cmCapStructPtr));
		memset(capSim,0,3*sizeof(cmCapStructPtr));
		capAltPtr = (cmCapStruct **)calloc(nCapSet+1,sizeof(cmCapStructPtr));
		memset(capAltPtr,0,(nCapSet+1)*sizeof(cmCapStructPtr));
		for(j=0;j<nCapSet;++j)
		{
			capAltPtr[j] = capSet[j];
		}
		capSim[0] = capAltPtr;


		capDesc[0] = capSim;

		/* Add Capability for dtmf */
		if((dataHandle = getDtmfDataHandle())>0)
		{
			int n = 2;
			capSet[nCapSet] = (cmCapStruct *)malloc(sizeof(cmCapStruct));
			memset(capSet[nCapSet],0,sizeof(cmCapStruct));
			capSet[nCapSet]->capabilityHandle = dataHandle;
			capSet[nCapSet]->direction = cmCapReceiveAndTransmit;
			capSet[nCapSet]->type = cmCapUserInput;
			nCapSet++;
			if (sendRfc2833)
			{
				n++;
			}
			capAltDtmf = (cmCapStruct **)calloc((n+1),sizeof(cmCapStructPtr));
			memset(capAltDtmf,0,(n+1)*sizeof(cmCapStructPtr));

			capAltDtmf[0] = capSet[nCapSet-1];
			capSim[1] = capAltDtmf;
			/* now add userInputBasicString cap */
			/* !! becareful when merge with Ticket 4452 */
			if((dataHandle = getDtmfStringDataHandle())>0)
			{
				capSet[nCapSet] = (cmCapStruct *)malloc(sizeof(cmCapStruct));
				memset(capSet[nCapSet],0,sizeof(cmCapStruct));
				capSet[nCapSet]->capabilityHandle = dataHandle;
				capSet[nCapSet]->direction = cmCapReceive;
				capSet[nCapSet]->type = cmCapUserInput;
				nCapSet++;
				capAltDtmf[1] = capSet[nCapSet-1];
			}
			if (sendRfc2833)
			{
				if((dataHandle = getRfc2833DataHandle())>0)
				{
					capSet[nCapSet] = (cmCapStruct *)malloc(sizeof(cmCapStruct));
					memset(capSet[nCapSet],0,sizeof(cmCapStruct));
					capSet[nCapSet]->capabilityHandle = dataHandle;
					capSet[nCapSet]->direction = cmCapReceive;
					capSet[nCapSet]->type = cmCapAudioTelephonyEvent;
					nCapSet++;
					capAltDtmf[2] = capSet[nCapSet-1];
				}
			}
		}		

		/* End Add Capability for dtmf */

		if((capNodeId = cmCallCapabilitiesBuild(hsCall, capSet, capDesc)) <0)
		{
			NETERROR(MH323,("%s %p cmCallCapabilitiesBuild failed\n",fn,hsCall));
			rv = -1;
			goto _error;
		}

		if ((capRoot = uh323GetLocalCaps()) < 0)
		{
			NETERROR(MH323,
				("%s Could not obtain Local capabilities\n", fn));
			rv = -1;
			goto _error;
		}

		if((nodeId = pvtGetNodeIdByPath(hVal,capRoot,".multiplexCapability")) <0)
		{
			NETERROR(MH323,
				("%s Could not obtain capNodeId .multiplexCapability= %d \n", fn,nodeId));
		}

		if(pvtAddTree(hVal,capNodeId,hVal,nodeId) <0)
		{
			NETERROR(MH323,
				("%s Could not do addsubtree\n", fn));
			rv = -1;
			goto _error;
		}

		if( (cmCallSendCapability(hsCall,capNodeId)< 0) )
		{
			NETERROR(MH323,("%s %p cmCallCapabilitiesSend failed\n",
				fn,hsCall));
			rv = -1;
			goto _error;
		}

	//	pvtDelete(hVal,capNodeId);
		freeNodeTree(UH323Globals()->hApp, capNodeId, 0);

	_error:

		/* Free up all the memory */
		for(j=0;j<nCapSet;++j)
		{
			free(capSet[j]);
		}

		vtnodecount = pvtCurSize(hVal);
		cmMeiExit(UH323Globals()->hApp);

		if (capSim)
		{
			free(capSim);
		}

		if (capSet)
		{
			free(capSet);
		}

		if (capAltPtr)
		{
			free(capAltPtr);
		}

		if (capAltDtmf)
		{
			free(capAltDtmf);
		}

		if (rv < 0)
		{
			return rv;
		}
		//NETDEBUG(MH323,NETLOG_DEBUG4,("%s After = %d\n",fn,vtnodecount));
	}

	if (cmCallMasterSlaveDetermineExt(hsCall, 120,genMsdNumber()) < 0)
	{
		NETDEBUG(MH323,NETLOG_DEBUG4,
		("%s %p cmCallMSDExt failed.\n", 
			fn, hsCall));
	}

	return rv;		
}



int uh323NullCapSend2(HCALL hsCall)
{
	static char 	fn[] = "uh323NullCapSend2";
#ifndef _slowdown

	if( (cmCallSendCapability(hsCall,emptyTCSId)< 0) )
	{
		NETERROR(MH323,("%s %p cmCallCapabilitiesSend failed\n",
			fn,hsCall));
		return -1;
	}
#else
	int 			nodeId;
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int				strbuffer;
	HPVT			nullhVal;
	static int		capNodeId = 0;		

	
	if(!capNodeId)
	{
		/* Optimization */

		cmMeiEnter(UH323Globals()->hApp);

		capNodeId = pvtAddRoot(hVal,
					hSyn,
					0,
					NULL);

			if ((nodeId = uh323GetLocalCaps()) < 0)
			{
				NETERROR(MH323,
					("%s Could not obtain Local capabilities\n", fn));
				cmMeiExit(UH323Globals()->hApp);
				return -1;
			}


	/*
		nullhVal = pvtConstruct(strbuffer,pvtCurSize(hVal));
		pvtSetTree(nullhVal,capNodeId,hVal,nodeId);
	*/

		pvtSetTree(hVal,capNodeId,hVal,nodeId);

		if((nodeId = pvtGetNodeIdByPath(hVal,capNodeId,".multiplexCapability")) <0)
		{
			NETERROR(MH323,
				("%s Could not obtain capNodeId .multiplexCapability= %d \n", fn,nodeId));
		}
		else if(pvtDelete(hVal,nodeId) <0)
		{
			NETERROR(MH323,
			("%s Could not do addsubtree\n", fn));
			cmMeiExit(UH323Globals()->hApp);
			return -1;
		}
		cmMeiExit(UH323Globals()->hApp);
	
	}
	

	if( (cmCallSendCapability(hsCall,capNodeId)< 0) )
	{
		NETERROR(MH323,("%s %x cmCallCapabilitiesSend failed\n",
			fn,hsCall));
		return -1;
	}

#endif
	return 0;
}


int uh323NullCapSend(HCALL hsCall)
{
	static char 	fn[] = "uh323NullCapSend";
	int 			nodeId;

        if ((nodeId = uh323GetLocalCaps()) < 0)
        {
            NETERROR(MH323,
                ("%s Could not obtain Local capabilities\n", fn));
            return -1;
        }

        if (cmCallSendCapability(hsCall, nodeId) < 0)
        {
            NETERROR(MH323,
                ("%s Could not forward Local capabilities\n", fn));
            return -1;
        }
	return 0;
}

int toH323Direction(int direction)
{
	int rv;
	switch (direction)
	{	
		case SendRecv:
			rv = cmCapReceiveAndTransmit;
			break;
		case SendOnly:
			rv  = cmCapTransmit;
			break;
		case RecvOnly:
			rv  = cmCapReceive;
			break;
		default:
			return cmCapReceiveAndTransmit;
	}

	return rv;
}

/*	Returns -1 on failure nodeId on success
*	Check for Memory Leak when returning error
*/
int
createDataHandleTCS(const H323RTPSet *pRtpSet)
{
	static char		fn[] = "createDataHandleTCS";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			nodeId,rv;
	int 			j,i;

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Entering codectype = %d param = %d\n", 
		fn,pRtpSet->codecType,pRtpSet->param));
				

	if((pRtpSet->codecType >=96) && (pRtpSet->codecType <=105))
	{
		// Suppress error messages for rfc2833 dtmf
		return -1;
	}

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
		return -1;
	}
	rv = createCodecTree(pRtpSet,hVal,nodeId);
	if (rv < 0)
	{
		NETERROR(MH323,
			("%s pvtAddRoot returns error\n", fn));
		cmMeiExit(UH323Globals()->hApp);
		return -1;
	}
	rv = pvtChild(hVal,nodeId);
	cmMeiExit(UH323Globals()->hApp);

	return rv;
}

/* returns an audioData tree - this tree should not be freed */
int
createDataHandleOLC(const H323RTPSet *pRtpSet)
{
	static char		fn[] = "createDataHandleOLC";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			nodeId,rv;
	int 			j,i;

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Entering codectype = %d param = %d\n", 
		fn,pRtpSet->codecType,pRtpSet->param));
				
	if((pRtpSet->codecType >=96) && (pRtpSet->codecType <=105))
	{
		// Suppress error messages for rfc2833 dtmf
		return -1;
	}
	cmMeiEnter(UH323Globals()->hApp);

	nodeId = getDataHandle(pRtpSet);
	if (nodeId < 0)
	{
		NETERROR(MH323,
			("%s getDataHandle returns error\n", fn));
		cmMeiExit(UH323Globals()->hApp);
		return -1;
	}
	rv = pvtChild(hVal,nodeId);
	if (rv < 0)
	{
		NETERROR(MH323,
			("%s pvtChild returns error\n", fn));
		cmMeiExit(UH323Globals()->hApp);
		return -1;
	}
	rv = pvtChild(hVal,rv);
	cmMeiExit(UH323Globals()->hApp);
	return rv;
}


/* The Caller should do cmMeiEnter and Exit
*/
int createCodecTree(const H323RTPSet *pRtpSet,HPVT hVal,int nodeId)
{
	static char 	fn[] = "createCodecTree";
	int rv = 0;

	cmMeiEnter(UH323Globals()->hApp);

	switch(pRtpSet->codecType) 
		{
		case CodecG729A:
			if (pvtBuildByPath(hVal,nodeId, 
					"audioData.g729AnnexA",
					 pRtpSet->param,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}
			break;

		case CodecG729: 
			if (pvtBuildByPath(hVal, nodeId,
					"audioData.g729",
					pRtpSet->param,
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}
			break;

		case CodecG7231: 
			if (pvtBuildByPath(hVal, nodeId,
					"audioData.g7231.maxAl-sduAudioFrames",
					pRtpSet->param,
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}
			if (pvtBuildByPath(hVal, nodeId,
					"audioData.g7231.silenceSuppression",
					(pRtpSet->flags & RTP_FLAGS_SS),
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}
			break;
		case CodecGPCMA: 
			if (pvtBuildByPath(hVal, nodeId,
					"audioData.g711Alaw64k",
					pRtpSet->param,
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}
			break;

		case CodecGPCMU: 
			if (pvtBuildByPath(hVal, nodeId,
					"audioData.g711Ulaw64k",
					pRtpSet->param,
					NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath returns error\n", fn));
			}
			break;
		default:
			NETDEBUG(MH323,NETLOG_DEBUG4,
				("%s Unknown codec type %d\n",fn,pRtpSet->codecType));
			rv = -1;
			break;
		}

	cmMeiExit(UH323Globals()->hApp);

	return rv;
}

int
getCodecParam(int dataTypeHandle,int *pCodecType,int *pParam, int *pSS)
{
	static 	char	fn[] = "getCodecParam";
	int 			codecType;
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			param;
	int				nodeId;
	int				ss = 0;
	

	cmMeiEnter(UH323Globals()->hApp);
    if (pvtGet(hVal, dataTypeHandle, NULL, NULL, &param, NULL) < 0)
		goto _err;

    if ((dataTypeHandle = pvtParent(hVal, dataTypeHandle)) < 0)
		goto _err;

    if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle,
                                     "g7231.maxAl-sduAudioFrames")) >= 0)
    {
        // G.723

        codecType = CodecG7231;

        // codec Param is different in this case

        if (pvtGet(hVal, nodeId, NULL, NULL, &param, NULL) < 0)
			goto _err;

    }
    else if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle, "g729")) >= 0)
        codecType = CodecG729;
    else if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle, "g729AnnexA")) >= 0)
        codecType = CodecG729A;
	else if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle, "g729wAnnexB")) >= 0)
		codecType = CodecG729B;
	else if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle, "g729AnnexAwAnnexB")) >= 0)
		codecType = CodecG729AwB;
    else if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle, "g711Ulaw64k"))
>= 0)
        codecType = CodecGPCMU;
    else if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle, "g711Alaw64k")) >= 0)
        codecType = CodecGPCMA;
    else if ((nodeId = pvtGetNodeIdByPath(hVal, dataTypeHandle, "nonStandard")) >= 0)
	{
		codecType = CodecNonStandard;
	}
    else
	{
			goto _err;
	}

	switch(codecType)
	{
		case CodecG729A:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729AnnexA\n",fn));
			if (dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, dataTypeHandle,
					"g729AnnexA",
					 NULL,
					&param, NULL) < 0)
				{
                    param = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729ar\n"));
				}
			}

			break;

		case CodecG729:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729\n",fn));
			if (dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, dataTypeHandle,
					"g729",
					 NULL,
					&param, NULL) < 0)
				{
                    param = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729r\n"));
				}
			}

			break;

		case CodecG729B:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729\n",fn));
			if (dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, dataTypeHandle,
					"g729wAnnexB",
					 NULL,
					&param, NULL) < 0)
				{
                    param = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729AnnexB\n"));
				}
			}

			break;
		case CodecG729AwB:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.729\n",fn));
			if (dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, dataTypeHandle,
					"g729AnnexAwAnnexB",
					 NULL,
					&param, NULL) < 0)
				{
                    param = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No nframes found for g729AnnexAwAnnexB\n"));
				}
			}

			break;

		case CodecG7231:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.7231\n",fn));

			if (dataTypeHandle != -1)
			{
				HPVT hVal = cmGetValTree(UH323Globals()->hApp);
				INT32 valueMaxAl = 0;

				if (pvtGetByPath(hVal, dataTypeHandle,
					"g7231.maxAl-sduAudioFrames",
					NULL,
					&param, NULL) < 0)
				{
                    param = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No rx sduAudioFrames found in fst\n"));
				}

				if (pvtGetByPath(hVal, dataTypeHandle,
					"g7231.silenceSuppression", 
					NULL,
					&ss, NULL) < 0)
				{
					DEBUG(MH323, NETLOG_DEBUG1, 
					("No rx silence suppression param found in fst\n"));
				}
			}

			break;

		case CodecGPCMU:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.711Ulaw64\n",fn));

			if (dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, dataTypeHandle,
					"g711Ulaw64k",
					 NULL,
					&param, NULL) < 0)
				{
                    param = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("%s No interval found for g711Ulaw64r\n",fn));
				}
			}

			break;

		case CodecGPCMA:

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Incoming channel for RX G.711Alaw64\n",fn));

			if (dataTypeHandle != -1)
			{
				if (pvtGetByPath(hVal, dataTypeHandle,
					"g711Alaw64k",
					 NULL,
					&param, NULL) < 0)
				{
                    param = 0;
					DEBUG(MH323, NETLOG_DEBUG1, 
					("%s No interval found for g711Alaw64r\n",fn));
				}
			}

			break;
		case CodecNonStandard:
	        param = 0;
#if 1
	        codecType = findNonStandardCodec( hVal, nodeId );
			if ( codecType < 0 )
			{
				codecType = CodecNonStandard;
			}
#endif
		    break;

		default:
			cmMeiExit(UH323Globals()->hApp);
			return -1;
			break;
		}

	cmMeiExit(UH323Globals()->hApp);

	*pParam = param;
	*pCodecType = codecType;
	*pSS |= ss;
	return 0;
_err:
	cmMeiExit(UH323Globals()->hApp);
	return -1;

}

int getFaxDataHandle()
{
	static char		fn[] = "getFaxDataHandle";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			rtId;

	if(faxDataHandle)
		return faxDataHandle;

	cmMeiEnter(UH323Globals()->hApp);
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s T38 Fax Creating New NODEID\n", 
				fn));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProtocol.udp",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath t38FaxProtocol returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.fillBitRemoval",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath fillBitRemoval returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.transcodingJBIG",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath transcodingJBIG returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.transcodingMMR",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath trancodingMMR returns error\n", fn));
				rtId = -1;
				goto _return;
			}
// Use transferred TCF (SONUS doesn't work without this)
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.version",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath t38FaxMaxBuffer returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.t38FaxRateManagement.transferredTCF",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath transferredTCF returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.t38FaxUdpOptions.t38FaxMaxBuffer",
					 200,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath t38FaxMaxBuffer returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.t38FaxUdpOptions.t38FaxMaxDatagram",
					 72,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath t38FaxMaxDatagram returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.t38fax.t38FaxProfile.t38FaxUdpOptions.t38FaxUdpEC.t38UDPRedundancy",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath t38FaxUdpEC.t38UDPRedundancy returns error\n", fn));
				rtId = -1;
				goto _return;
			}
// END change to tranferred TCF 
			if (pvtBuildByPath(hVal,rtId, 
					"data.maxBitRate",
					 144,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath maxBitRate returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			faxDataHandle = rtId;
_return:
	cmMeiExit(UH323Globals()->hApp);
	return rtId;
}

int getDtmfDataHandle()
{
	static char		fn[] = "getDtmfDataHandle";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"h245");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			rtId,h245Conf;
	char			name[100];


	if(dtmfDataHandle)
		return dtmfDataHandle;

 	h245Conf = cmGetH245ConfigurationHandle(UH323Globals()->hApp);

	cmMeiEnter(UH323Globals()->hApp);
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s DTMF Creating New NODEID\n", 
				fn));
	rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
	if (rtId< 0)
	{
		NETERROR(MH323,
			("%s pvtAddRoot returns error\n", fn));
		goto _return;
	}

	if(pvtSetTree(hVal,rtId,hVal,h245Conf) < 0)
	{

		NETERROR(MH323,
			("%s pvtSetTree returns error\n", fn));
		goto _return;
	}


	if ((pvtBuildByPath(hVal,rtId,
	".capabilities.terminalCapabilitySet.capabilityTable.1.capability.receiveAndTransmitUserInputCapability.dtmf",
			 0,
			 NULL)) < 0)
	{
		NETERROR(MH323,
			("%s pvtBuildByPath returns error\n", fn));
	}

	if ((dtmfDataHandle = pvtGetNodeIdByPath(hVal,rtId,
	".capabilities.terminalCapabilitySet.capabilityTable.1.capability.receiveAndTransmitUserInputCapability")) < 0)
	{
		NETERROR(MH323,
			("%s pvtGetNodeIdByPath  returns error\n", fn));
		dtmfDataHandle = 0;
	}
_return:
	cmMeiExit(UH323Globals()->hApp);
	return dtmfDataHandle;
}

int getDtmfStringDataHandle()
{
	static char		fn[] = "getDtmfStringDataHandle";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"h245");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			rtId,h245Conf;
	char			name[100];


	if(dtmfStringDataHandle)
		return dtmfStringDataHandle;

 	h245Conf = cmGetH245ConfigurationHandle(UH323Globals()->hApp);

	cmMeiEnter(UH323Globals()->hApp);
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s DTMF string Creating New NODEID\n", 
				fn));
	rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
	if (rtId< 0)
	{
		NETERROR(MH323,
			("%s pvtAddRoot returns error\n", fn));
		goto _return;
	}

	if(pvtSetTree(hVal,rtId,hVal,h245Conf) < 0)
	{

		NETERROR(MH323,
			("%s pvtSetTree returns error\n", fn));
		goto _return;
	}


	if ((pvtBuildByPath(hVal,rtId,
	".capabilities.terminalCapabilitySet.capabilityTable.1.capability.receiveUserInputCapability.basicString",
			 0,
			 NULL)) < 0)
	{
		NETERROR(MH323,
			("%s pvtBuildByPath returns error\n", fn));
	}

	if ((dtmfStringDataHandle = pvtGetNodeIdByPath(hVal,rtId,
	".capabilities.terminalCapabilitySet.capabilityTable.1.capability.receiveUserInputCapability")) < 0)
	{
		NETERROR(MH323,
			("%s pvtGetNodeIdByPath  returns error\n", fn));
		dtmfStringDataHandle = 0;
	}
_return:
	cmMeiExit(UH323Globals()->hApp);
	return dtmfStringDataHandle;
}

int findNonStandardCodec( HPVT hVal, int nonsNodeId )
{
	cmNonStandardIdentifier nsId;
	int i;
	char data[MSW_MAX_LEN_NONSTANDARD_DATA];
	int dataLength = MSW_MAX_LEN_NONSTANDARD_DATA;
	if ( cmNonStandardParameterGet( hVal, nonsNodeId, &nsId, data, &dataLength) < 0 )
	{
		return -1;
	}
	for( i = 0; mswNonStandardTable[i].codec != -1; i++ )
	{
		if ( nsId.manufacturerCode == mswNonStandardTable[i].h221.manufacturerCode &&
			 nsId.t35Extension == mswNonStandardTable[i].h221.t35Extension &&
			 nsId.t35CountryCode == mswNonStandardTable[i].h221.t35CountryCode &&
			 memcmp( data, mswNonStandardTable[i].data, dataLength ) == 0 )
		{
			return mswNonStandardTable[i].codec;
		}
	}
	return -1;
}

int getLucentFaxDataHandle()
{
	static char		fn[] = "getLucentFaxDataHandle";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			rtId;

	if(faxLucentDataHandle)
		return faxLucentDataHandle;

	cmMeiEnter(UH323Globals()->hApp);
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s T38 Fax Creating New NODEID\n", 
				fn));
			rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
			if (rtId< 0)
			{
				NETERROR(MH323,
					("%s pvtAddRoot returns error\n", fn));
				goto _return;
			}

			if (pvtBuildByPath(hVal,rtId, 
					"data.application.nonStandard.nonStandardIdentifier.h221NonStandard.t35CountryCode",
					 181,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath t35CountryCode returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.nonStandard.nonStandardIdentifier.h221NonStandard.t35Extension",
					 0,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath t35 extension returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.nonStandard.nonStandardIdentifier.h221NonStandard.manufacturerCode",
					 20,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath manufacturerCode returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			if (pvtBuildByPath(hVal,rtId, 
					"data.application.nonStandard.data",
					 strlen("T38FaxUDP"),
					 "T38FaxUDP") < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath data T38FaxUDP returns error\n", fn));
				rtId = -1;
				goto _return;
			}

			if (pvtBuildByPath(hVal,rtId, 
					"data.maxBitRate",
					 144,
					 NULL) < 0)
			{
				NETERROR(MH323,
					("%s pvtBuildByPath maxBitRate returns error\n", fn));
				rtId = -1;
				goto _return;
			}
			faxLucentDataHandle = rtId;
_return:
	cmMeiExit(UH323Globals()->hApp);
	return rtId;
}

int getRfc2833DataHandle(void)
{
	static char		fn[] = "getRfc2833DataHandle";
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"h245");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int 			rtId,h245Conf;
	char			name[100] = "0-16";

	if(rfc2833DataHandle)
		return rfc2833DataHandle;

 	h245Conf = cmGetH245ConfigurationHandle(UH323Globals()->hApp);

	cmMeiEnter(UH323Globals()->hApp);
			NETDEBUG(MH323, NETLOG_DEBUG1, 
				("%s RFC2833 Creating New NODEID\n", 
				fn));
	rtId = pvtAddRoot(hVal, hSyn, 0,NULL);
	if (rtId< 0)
	{
		NETERROR(MH323,
			("%s pvtAddRoot returns error\n", fn));
		goto _return;
	}

	if(pvtSetTree(hVal,rtId,hVal,h245Conf) < 0)
	{

		NETERROR(MH323,
			("%s pvtSetTree returns error\n", fn));
		goto _return;
	}


	if ((pvtBuildByPath(hVal,rtId,
	".capabilities.terminalCapabilitySet.capabilityTable.1.capability.receiveRTPAudioTelephonyEventCapability.dynamicRTPPayloadType",
			 CodecRFC2833,
			 NULL)) < 0)
	{
		NETERROR(MH323,
			("%s pvtBuildByPath returns error\n", fn));
	}
	if ((pvtBuildByPath(hVal,rtId,
	".capabilities.terminalCapabilitySet.capabilityTable.1.capability.receiveRTPAudioTelephonyEventCapability.audioTelephoneEvent",
			 4,
			 name)) < 0)
	{
		NETERROR(MH323,
			("%s pvtBuildByPath returns error\n", fn));
	}

	if ((rfc2833DataHandle = pvtGetNodeIdByPath(hVal,rtId,
	".capabilities.terminalCapabilitySet.capabilityTable.1.capability.receiveRTPAudioTelephonyEventCapability")) < 0)
	{
		NETERROR(MH323,
			("%s pvtGetNodeIdByPath  returns error\n", fn));
		rfc2833DataHandle = 0;
	}
_return:
	cmMeiExit(UH323Globals()->hApp);
	return rfc2833DataHandle;
}

/******************************************************************************************
 * decodeFacilityFastStartMsg
 *
 * Purpose:  This function is called upon receipt of a Facility message.
 *           The function checks for any fast-start channels in the message,
 *           decodes the data and builds it into a structure that is passed in a CallBack
 *           to the application, such that the application may ack some of them.
 *
 * Input:    call       - call object instance
 *           message    - The node id to the setup message
 *
 * Reurned Value: Non-negative value on success
 *                Negative value on failure
 ****************************************************************************************/
int
decodeFacilityFastStartMsg(
        		IN  HCALL           			hsCall,
                IN  int             			message,
    			IN OUT cmFastStartMessage  		*fsMessage,
    			IN OUT INT16 					*lcnOut)
{
    int                 		nodeId;

	static char 				fn[] = "decodeFacilityFastStartMsg";
    BYTE 						lcnAllocationBuff[(MAX_FASTSTART_CHANNELS+1)/sizeof(char)];

    int 						decoded_length, buff_len, freeLoc=0;
    int 						index=0;
    int 						typeLoc[MAX_FS_CHANNELS+1];
    int 						encodedOctedStrings;
    cmAlternativeChannels 		*pAltChannel;
    HPVT 						hVal;

	HPST						synTreeH245= cmGetSynTreeByRootName(UH323Globals()->hApp, "fsOpenLcn");
    HAPP 						hApp=(HAPP)(UH323Globals()->hApp);
	BOOL 						string;
    INT32 						lcn = -1;

	hVal = cmGetValTree(UH323Globals()->hApp);

    encodedOctedStrings = pvtGetNodeIdByPath(hVal, message, "fastStart"); 


    if (!fsMessage) return RVERROR;
    if (encodedOctedStrings<0) return RVERROR;

    /* Initialized the table to be built */
    memset(fsMessage,0,sizeof(cmFastStartMessage));
    memset(typeLoc,0xff,sizeof(typeLoc));
    memset(lcnAllocationBuff,0,sizeof(lcnAllocationBuff));

	*lcnOut = -1;

    /* Go over the received encoded OLC message from the FACILITY,
       decode them for subsequent analysis */
    nodeId = pvtChild(hVal, encodedOctedStrings);
    while (nodeId > 0)
    {
		BYTE encodeBuffer[MAX_ENCODEDECODE_BUFFRSIZE];

        buff_len = pvtGetString(hVal, nodeId, MAX_ENCODEDECODE_BUFFRSIZE, (char*)encodeBuffer);
        if (buff_len > 0)
        {
            /* Decode an OpenLogicalChannel faststart message */
            int chanMsgId=pvtAddRoot(hVal, synTreeH245, 0, NULL);
            /*int chanMsgId=call->fastStartNodes[index]=pvtAddRoot(hVal, synTreeH245, 0, NULL);*/ //sohan

            if (cmEmDecode(hVal, chanMsgId, encodeBuffer, buff_len, &decoded_length)>=0)
            {

				NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s  Suggested faststart channel decoded:\n",fn));
                {
                    /* Make the logical channel's number as taken */
                    /*pvtGetChildValue(hVal, chanMsgId,__h245(forwardLogicalChannelNumber),&lcn,NULL);*/
					pvtGetByPath(hVal, chanMsgId, "forwardLogicalChannelNumber", NULL, &lcn, &string);
					NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s found LCN = %d\n",fn, lcn));
                    setBit(lcnAllocationBuff,lcn/MAX_LCN_S,1); //sohan
                }
            }
            else
            {
				NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s Decoding Problems!\n",fn));
                nodeId = pvtBrother(hVal, nodeId);
                continue;
            }

            {
                cmFastStartChannel fsChannel;
                cmCapDataType type;
                cmChannelDirection direction;
                int loc;

                /* We now have an offered channel */
                /* Go over the OLC message and analyse it */

                cmFastStartGet((HCALL)hsCall, chanMsgId, &type, &direction, &fsChannel);
				if ((type != cmCapEmpty) && (*lcnOut == -1)
							&& (lcn > 0))
 				{
						*lcnOut = lcn;
				}
				
                /* Fill the session Id into the channel table being prepared for the user */
                if (typeLoc[(int)type] == -1)
                {
                    loc=typeLoc[(int)type] = freeLoc++; /* todo: How is freeLoc used? */
                    fsMessage->partnerChannels[loc].type = type;
                    fsMessage->partnerChannelsNum++;
                }
                else
                    loc=typeLoc[(int)type];

                /* Set a pointer to the right part of the channel tables
                 according to the channels direction */
                if (direction==dirTransmit)
                                    pAltChannel    = &(fsMessage->partnerChannels[loc].transmit);
                else                pAltChannel    = &(fsMessage->partnerChannels[loc].receive);


                /* Now fill all the data from the OLC message to the table */
                if (pAltChannel->altChannelNumber < MAX_ALTERNATIVE_CHANNEL)
                {

                    /* Give the channel an index */
                    fsChannel.index = index;

                    pAltChannel->channels[pAltChannel->altChannelNumber]=fsChannel;

                    /* Advance the channels counter in the table */
                    pAltChannel->altChannelNumber++;

                }
            }
        }
        nodeId = pvtBrother(hVal, nodeId);
        index++;
        if (index == (MAX_CHANNELS_PER_CALL * 5))
        {
            /* we were offered more channels than we have room for - ignore the rest */
            break;
        }
    }
    
    /*If the number of fast start proposals was less then MAX_FASTSTART_CHANNELS then at least one range will be available */
    /*call->lcnOut=get1st0BitNumber(lcnAllocationBuff, 0, MAX_FASTSTART_CHANNELS)*MAX_LCN_S;*/
    return  TRUE;
}

int
encodeFacilityFastStartMsg(
                           	IN   HCALL       			hsCall,
    						IN OUT cmFastStartMessage  	*fsMessage,
    						IN INT16 					lcnIn)
{
	static char 				fn[] = "encodeFacilityFastStartMsg";
    int 						openMsgId,index,channelType,ret=1;
    HPVT 						hVal;
	int							message = RVERROR, nodeId = RVERROR, facilityNodeId = RVERROR, oldFaststartNodeId;	
	int 						newMessage= RVERROR;
    HAPP 						hApp=(HAPP)(UH323Globals()->hApp);
	
    if (!hsCall || fsMessage->partnerChannelsNum<=0) return RVERROR;
    if (!hApp) return RVERROR;
    hVal=cmGetValTree(hApp);

	if ((newMessage = pvtAddRoot(hVal, NULL, 0, NULL))<0)
	{
		NETERROR(MH323,
				("%s failed to add root node\n", fn));
    	return RVERROR;
	}

    /* Get the message to send */
    if ((message = callGetMessage(hsCall,cmQ931facility)) < 0)
	{
		NETERROR(MH323,
			("%s failed to get property nodeId\n", fn));
    	return RVERROR;
	}
	else
	{
		if ((facilityNodeId = pvtGetNodeIdByPath(hVal, message, 
			"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.facility")) < 0)
		{
			NETERROR(MH323,
				( "%s could not get facility node\n", fn));
    		return RVERROR;
		}
		else
		{
		    /* copy the message and add session data to the message */
			if ((oldFaststartNodeId = pvtGetNodeIdByPath(hVal, facilityNodeId, 
				"fastStart")) > 0)
			{
				/* remove this node id */
				pvtDelete(hVal, oldFaststartNodeId);
			}
        	pvtSetTree(hVal, newMessage,hVal, message);
			if ((facilityNodeId = pvtGetNodeIdByPath(hVal, newMessage, 
				"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.facility")) < 0)
			{
				NETERROR(MH323,
					("%s could not get facility node\n", fn));
    			return RVERROR;
			}			
			else
			{
				if ((nodeId = pvtBuildByPath(hVal, facilityNodeId,
    		       	"fastStart",0, NULL)) < 0) 
        		{
					NETERROR(MH323,
						("%s could not build fastStart node\n", fn));
	    			return RVERROR;
				}
			}
		}

         /* This is the main loop that goes over the offered channls in the given structure
         and build from it the sub-tree to be saved in the H245 machine and attached to the
         FACILITY message. The order is acccording to the channel type (Audio, Video etc.) */
        for (channelType = 0; channelType< fsMessage->partnerChannelsNum ;channelType++)
        {
            cmAlternativeChannels	*aChannel;
            /* We currently handle only audio and video channels in faststart */
			// ???? DataType == NULL
            if ( (fsMessage->partnerChannels[channelType].type < cmCapEmpty) || (fsMessage->partnerChannels[channelType].type > cmCapData) )
                continue;
            aChannel=&fsMessage->partnerChannels[channelType].receive;
            /* Go over the offered receive channels */
            for (index=0;index<aChannel->altChannelNumber;index++)
            {
                /* Build logicalChannel message */
                openMsgId = cmFastStartBuild(hsCall, fsMessage->partnerChannels[channelType].type, dirReceive, &aChannel->channels[index]);
				
				if (lcnIn > -1)
				{
					if (fsMessage->partnerChannels[channelType].type == cmCapEmpty)
 					{
			        	if (pvtBuildByPath(hVal, openMsgId, "forwardLogicalChannelNumber", lcnIn, NULL) < 0 )
						{
							NETERROR(MH323,
								("%s failed add forwardLogicalChannelNumber\n", fn));
						}
					}
					/* keep generated lcn */
					/*else
					{
			        	if (pvtBuildByPath(hVal, openMsgId, "forwardLogicalChannelNumber", lcnIn, NULL) < 0 )
						{
							NETERROR(MH323,
								("%s failed add forwardLogicalChannelNumber\n", fn));
						}
					}*/
				}
                /* The OLC is ready for the receive channel, encode it */
				addFacilityFastStart(nodeId, openMsgId);
            }
            aChannel=&fsMessage->partnerChannels[channelType].transmit;
            /* Now go over the offered transmit channels */
            for (index=0;index<aChannel->altChannelNumber;index++)
            {
                /* Build logicalChannel message */
                openMsgId = cmFastStartBuild(hsCall, fsMessage->partnerChannels[channelType].type, dirTransmit, &aChannel->channels[index]);

				if (lcnIn > -1)
				{
					if (fsMessage->partnerChannels[channelType].type == cmCapEmpty)
 					{
			        	if (pvtBuildByPath(hVal, openMsgId, "forwardLogicalChannelNumber", lcnIn, NULL) < 0 )
						{
							NETERROR(MH323,
								("%s failed add forwardLogicalChannelNumber\n", fn));
						}
					}
					/* keep generated lcn */
					/*else
					{
			        	if (pvtBuildByPath(hVal, openMsgId, "forwardLogicalChannelNumber", lcnIn, NULL) < 0 )
						{
							NETERROR(MH323,
								("%s failed add forwardLogicalChannelNumber\n", fn));
						}
					}*/
				}
                /* The OLC is ready for the receive channel, encode it */
				addFacilityFastStart(nodeId, openMsgId);
            }
        }
	}

    return newMessage;
}


int addFacilityFastStart(int nodeId, int openMsgId)
{
	static char 			fn[] = "addFacilityFastStart";
    HPVT 					hVal;
    int 					iBufLen;
    HAPP 					hApp=(HAPP)(UH323Globals()->hApp);
	BYTE 					encodeBuffer[MAX_ENCODEDECODE_BUFFRSIZE];

    if (!hApp) return RVERROR;

    hVal=cmGetValTree(hApp);

    if (! (cmEmEncode(hVal, openMsgId, encodeBuffer,MAX_ENCODEDECODE_BUFFRSIZE, &iBufLen) < 0) )
    {
        /* That's it, now we can add the encoded OLC message to the Facility message */
        pvtAdd(hVal,nodeId,-800,iBufLen,(char*)encodeBuffer,NULL);
	}
    else
	{
		NETERROR(MH323,
			("%s Encoding Problems!\n",fn));
	}

	return(0);
}

RVAPI
int RVCALLCONV sendFacilityFastStart(
						IN      HAPPCALL            		haCall,
                        IN   	HCALL       				hsCall,
    					IN 		cmFastStartMessage  		*fsMessage,
						IN		INT16						lcn)	
{
	static char 				fn[] = "sendFacilityFastStartCloseChannel";
	int							message = RVERROR;	
    HAPP 						hApp=(HAPP)(UH323Globals()->hApp);

    if (!hApp || !hsCall) return RVERROR;

	if (!fsMessage)
	{
			NETERROR(MH323,
				("%s Invalid Fast Start message\n",fn));
    		return RVERROR;
	}
	else
	{
		if ((message = encodeFacilityFastStartMsg(hsCall, fsMessage, lcn)) < 0)
		{
			NETERROR(MH323,
				("%s Failed to Encode facility Fast Start\n",fn));
    		return RVERROR;
		}
		else
		{
    		HPVT 					hVal;

    		hVal=cmGetValTree(hApp);
    		cmCallSetParam((HCALL)hsCall,cmParamFacilityReason,0,cmReasonTypeUndefinedReason,NULL);
		    cmCallFacility(hsCall, message);
		}
	}
    return 0;
}
