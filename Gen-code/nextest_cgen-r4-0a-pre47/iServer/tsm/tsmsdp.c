#include "gis.h"
#include "net.h"
#include <malloc.h>

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include <malloc.h>

#include "nxosd.h"
#include "siputils.h"
#include "bcpt.h"
#include "sdp.h"
#include "sipbcptinit.h"
#include "sipbcptfree.h"
// We either use this, or count the total no of
// entries needed by scanning the message once.
#define MAX_RTPSET_ENTRIES	64

int SipFormRTPParamFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle)
{
	char fn[]="SipFormRTPParamFromMsg():";
	int count, mcount, mcounti;
	SdpMessage *pSdp=NULL;
	SdpOrigin *pOrigin=NULL;
	SdpConnection *pConn=NULL;
	SdpMedia *pMedia=NULL;
	SdpAttr *pAttr=NULL;
	SIP_S8bit *pVersion=NULL, *tmp=NULL, *nextstr=NULL, *pOrigAddr = NULL;
	SIP_S8bit CodecList[256];
	char* ptrptr; 
	unsigned long rtpaddr=0;
	unsigned short rtpport=0;
	int i, n, codec[MAX_RTPSET_ENTRIES];
	RTPSet *rtpSet, rtpSetTemp[MAX_RTPSET_ENTRIES];
	int nrtpSet = 0, nsdpattr = 0;
	SDPAttr *sdpattr=NULL, sdpattrTemp[MAX_RTPSET_ENTRIES];
	CodecType direction=SendRecv;
	SipError err;
	int herror;
	SIP_S8bit *media;
	unsigned long originIp;
	SIP_S8bit *pValue;

	
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));
	
	if( sip_getSdpFromMsgBody(msgbody, &pSdp, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get sdp from msgbody\n", fn));
		goto _error;
	}

	/* extract version */
	if( sdp_getOrigin(pSdp, &pOrigin, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get origin from sdp body\n", fn));
		goto _error;
	}
	if( sdp_getVersionFromOrigin(pOrigin, &pVersion, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get version from origin\n", fn));
		goto _error;
	}
	appMsgHandle->sdpVersion = atoi(pVersion);

	if( sdp_getAddrFromOrigin(pOrigin, &pOrigAddr, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get address from origin\n", fn));
		goto _error;
	}

	appMsgHandle->pOriginIpAddress = strdup(pOrigAddr);

	/* extract connection */
	if( sdp_getConnection(pSdp, &pConn, &err) == SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no connection at session level\n", fn));
		pConn = NULL;
	}

	/* encryption key */
	if(sdp_getKey(pSdp, &tmp, &err) == SipSuccess)
	{
			SipCreateEncryptAttrib(&sdpattrTemp[nsdpattr], tmp);

			if (++nsdpattr == MAX_RTPSET_ENTRIES)
			{
				NETERROR(MSIP, ("%s Max attribs exceeded %d\n", fn, nsdpattr));
				goto _return;
			}
	}

	/* bandwith */
	count = 0;
	if( sdp_getBandwidthCount(pSdp, &count, &err) == SipSuccess)
	{
		if(count > 0)
		{
			for(i = 0; i < count; ++i)
			{
				if( sdp_getBandwidthAtIndex(pSdp, &tmp, i, &err) == SipSuccess)
				{
					SipCreateBandwidthAttrib(&sdpattrTemp[nsdpattr], tmp);

					if (++nsdpattr == MAX_RTPSET_ENTRIES)
					{
						NETERROR(MSIP, ("%s Max attribs exceeded %d\n", fn, nsdpattr));
						goto _return;
					}
				}
			}
		}
	}

	/* extract media */
	mcount=0;
	if( sdp_getMediaCount(pSdp, &mcount, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get media count\n", fn));
		goto _error;
	}
	if(mcount == 0)
	{
		NETERROR(MSIP, ("%s media count is 0\n", fn));
		goto _error;
	}

	for (mcounti = 0; mcounti < mcount; mcounti++)
	{
		if( sdp_getMediaAtIndex(pSdp, &pMedia, mcounti, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get media at index 0\n", fn));
			goto _error;
		}

		if( sdp_getMvalueFromMedia(pMedia, &media, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get media type at index 0\n", fn));
			goto _error;
		}

		/* media connection */
		if( sdp_getConnectionCountFromMedia(pMedia, &count, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get conn count from media\n", fn));
			goto _error;
		}

		if(count>0)
		{
			if( sdp_getConnectionAtIndexFromMedia(pMedia, &pConn, 0, &err) 
				== SipFail)
			{
				NETERROR(MSIP, ("%s fail to get conn from media\n", fn));
				goto _error;
			}
		}
		else if(pConn == NULL)
		{
			NETERROR(MSIP, ("%s no conncection at session and medial level\n",fn));
			goto _error;
		}

		if(pConn)
		{
		    if ((rtpaddr = ResolveDNS(pConn->dAddr, &herror)) == -1)
			{
				NETERROR(MSIP, ("%s inet_addr error %d\n",fn, errno));
				goto _error;
			}
		}

		/* media port */
		rtpport = pMedia->dPort;

		/* codec list */
		nx_strlcpy(CodecList, pMedia->pFormat, 256);
		nextstr=&CodecList[0];
		n=0;
		while( (n < MAX_RTPSET_ENTRIES) && 
			(tmp=strtok_r(nextstr, " ", &ptrptr)) )
		{
			codec[n++] = SipMediaFmtToCodec(tmp);
			nextstr = ptrptr;
		}

		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s found %d codecs:",fn,n));
		i=0;
		while(i<n)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s codec: %d", fn, codec[i]));
			i++;
		}
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("\n"));
		if(n == 0)
		{
			NETERROR(MSIP, ("%s no codec found\n", fn));
			goto _error;
		}
	
		rtpSet = (RTPSet *) &rtpSetTemp[nrtpSet];
		bzero(rtpSet, n*sizeof(RTPSet));
		for(i=0;i<n;i++)
		{
			rtpSet[i].mediaType = SipMediaType(media);
			rtpSet[i].codecType = codec[i];
			rtpSet[i].rtpaddr = rtpaddr;
			rtpSet[i].rtpport = rtpport;
			rtpSet[i].mLineNo = mcounti;
			if (++nrtpSet == MAX_RTPSET_ENTRIES)
			{
				NETERROR(MSIP, 
					("%s total rtp entries %d exceeded\n",
					fn, nrtpSet));
				goto _return;
			}
		}

		/* encryption key */
		if(sdp_getKeyFromMedia(pMedia, &tmp, &err) == SipSuccess)
		{
				SipCreateMediaEncryptAttrib(&sdpattrTemp[nsdpattr], mcounti, tmp);

				if (++nsdpattr == MAX_RTPSET_ENTRIES)
				{
					NETERROR(MSIP, ("%s Max attribs exceeded %d\n", fn, nsdpattr));
					goto _return;
				}
		}

		/* bandwith */
		count = 0;
		if( sdp_getBandwidthCountFromMedia(pMedia, &count, &err) == SipSuccess)
		{
			if(count > 0)
			{
				for(i = 0; i < count; ++i)
				{
					if( sdp_getBandwidthAtIndexFromMedia(pMedia, &tmp, i, &err) == SipSuccess)
					{
						SipCreateMediaBandwidthAttrib(&sdpattrTemp[nsdpattr], mcounti, tmp);

						if (++nsdpattr == MAX_RTPSET_ENTRIES)
						{
							NETERROR(MSIP, ("%s Max attribs exceeded %d\n", fn, nsdpattr));
							goto _return;
						}
					}
				}
			}
		}

		/* attributes */
		count = 0;
		if( sdp_getAttrCountFromMedia(pMedia, &count, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get attr count\n", fn));
			goto _error;
		}
		if(count == 0)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no media attr found\n",fn));
			continue;
		}

		sdpattr = (SDPAttr *) &sdpattrTemp[nsdpattr];

		for(i = 0; i < count;)
		{
			if( sdp_getAttrAtIndexFromMedia(pMedia, &pAttr, i, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to get attr at index=%d\n", fn, i));
				goto _error;
			}

			if(pAttr->pName)
			{
				pValue = pAttr->pValue;

				// Normalize pAttr with "annexb" to "annexb="
				if (pValue)
				{
					if (strstr(pValue, "annexb:no"))
						pValue = strdup("18 annexb=no");
					else 
					{
						if (strstr(pValue, "annexb:yes"))
							pValue = strdup("18 annexb=yes");
					}
				}
				SipCreateMediaAttrib(&sdpattr[i], mcounti, pAttr->pName, pValue);

				if ((pValue != NULL) && (pValue != pAttr->pValue))
					free(pValue);

				if( strcmp(pAttr->pName, "sendonly") == 0)
				{
					direction = SendOnly;
					
				}
				else if( strcmp(pAttr->pName, "recvonly") == 0)
				{
					direction = RecvOnly;
				}
			}
			else
			{
				NETERROR(MSIP, ("%s %d-th attr has NULL pName\n", fn, i));
				goto _error;
			}

			sip_freeSdpAttr(pAttr);
			pAttr = NULL;
			i++;
			if (++nsdpattr == MAX_RTPSET_ENTRIES)
			{
				NETERROR(MSIP, ("%s Max attribs exceeded %d\n",
					fn, nsdpattr));
				goto _return;
			}
		}

		// WHY is this global ?? - WILL not work
		if(direction != SendRecv)
		{
			for(i=0;i<n;i++)
			{
				rtpSet[i].direction = direction;
			}
		}
		sip_freeSdpMedia(pMedia); pMedia = NULL;
	}

	appMsgHandle->nlocalset = nrtpSet;
	if (nrtpSet)
	{
		appMsgHandle->localSet = (RTPSet *) malloc(nrtpSet*sizeof(RTPSet));
		memcpy(appMsgHandle->localSet, &rtpSetTemp[0], nrtpSet*sizeof(RTPSet));
	}

	appMsgHandle->attr_count = nsdpattr;
	if (nsdpattr)
	{
		appMsgHandle->attr = (SDPAttr *) malloc(nsdpattr*sizeof(SDPAttr));
		memcpy(appMsgHandle->attr, &sdpattrTemp[0], nsdpattr*sizeof(SDPAttr));
	}

 _return:
	sip_freeSdpMessage(pSdp);
	sip_freeSdpOrigin(pOrigin);
	sip_freeSdpConnection(pConn);
	sip_freeSdpMedia(pMedia);
	sip_freeSdpAttr(pAttr);
	return 0;

 _error:
	sip_freeSdpMessage(pSdp);
	sip_freeSdpOrigin(pOrigin);
	sip_freeSdpConnection(pConn);
	sip_freeSdpMedia(pMedia);
	sip_freeSdpAttr(pAttr);
	return -1;
}


int SipFormIsupFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle,
						SipHeader *pContentType, SipHeader *pContentDisp)
{
    char fn[]="SipFormIsupFromMsg():";
    IsupMessage *isup=NULL;
    SipError err;
	SipParam *pParam;
	SIP_S8bit *pName, *pValue;
	SIP_S8bit *disp;
	SIP_S8bit *isupBody;
	SIP_U32bit count=0;
	SIP_U32bit i;
	int ret = 0;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

	sip_getParamCountFromContentTypeHdr (pContentType, &count, &err);

	for (i=0; i<count; i++)
	{
		pParam=NULL;
		pValue = pName = NULL;

		sip_getParamAtIndexFromContentTypeHdr (pContentType, &pParam, i, &err);

		if ( sip_getNameFromSipParam(pParam,&pName,&err) == SipFail )
		{
		    NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Error in sip_getNameFromSipParam : %d \n", fn, err));
		}
		else
		{
			if ( sip_getValueAtIndexFromSipParam(pParam,&pValue,0,&err) == SipFail )
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("\nError in sip_getValueAtIndexFromSipParam : %d \n" ,err));
			}
			else
			{
				if (strcasecmp (pName, "version") == 0)
				{
					appMsgHandle->isupTypeVersion = strdup(pValue);	// Save value
				}
				else if (strcasecmp (pName, "base") == 0)
				{
					appMsgHandle->isupTypeBase = strdup(pValue);	// Save value
				}
			}
		}

		sip_freeSipParam(pParam) ;
	}

	if (pContentDisp)
	{
		sip_getDispTypeFromContentDispositionHdr(pContentDisp, &disp, &err);
		appMsgHandle->isupDisposition = strdup (disp);

		sip_getParamCountFromContentDispositionHdr (pContentDisp, &count, &err);
		for (i=0; i<count; i++)
		{
			SipParam *pParam=NULL;
			pValue = pName = NULL;

			sip_getParamAtIndexFromContentDispositionHdr (pContentDisp, &pParam, i, &err);

			if ( sip_getNameFromSipParam(pParam,&pName,&err) == SipFail )
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Error in sip_getNameFromSipParam : %d \n", fn, err));
			}
			else
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Name in SIP-Param : %s \n", fn, pName));
				if ( sip_getValueAtIndexFromSipParam(pParam,&pValue,0,&err) == SipFail )
				{
					NETDEBUG (MSIP, NETLOG_DEBUG4, ("\nError in sip_getValueAtIndexFromSipParam : %d \n" ,err));
				}
				else
				{
					NETDEBUG (MSIP, NETLOG_DEBUG4,("Value in SIP-Param : %s \n",pValue));
					if (strcasecmp (pName, "handling") == 0)
					{
						appMsgHandle->isupHandling = strdup(pValue);	// Save value
					}
				}
			}

			sip_freeSipParam(pParam);
		}
	}

    sip_bcpt_getIsupFromMsgBody(msgbody, &isup, &err);

    sip_bcpt_getLengthFromIsupMessage(isup, &appMsgHandle->isup_msg_len, &err);

    if(appMsgHandle->isup_msg = malloc(appMsgHandle->isup_msg_len))
    {
		sip_bcpt_getBodyFromIsupMessage(isup, &isupBody, &err);
		memcpy(appMsgHandle->isup_msg, isupBody, appMsgHandle->isup_msg_len);
    }
	else
	{
		NETERROR (MSIP, ("%s Failed to malloc %d bytes for isup message\n",fn,appMsgHandle->isup_msg_len));
	}

_done:
	if (isup)
		sip_bcpt_freeIsupMessage(isup);

    return ret;
}

int SipFormQsigFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle,
						SipHeader *pContentType, SipHeader *pContentDisp)
{
    char fn[]="SipFormQsigFromMsg():";
    QsigMessage *qsig=NULL;
    SipError err;
	SipParam *pParam;
	SIP_S8bit *pName, *pValue;
	SIP_S8bit *disp;
	SIP_S8bit *qsigBody;
	SIP_U32bit count=0;
	SIP_U32bit i;
	int ret = 0;

    NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

	sip_getParamCountFromContentTypeHdr (pContentType, &count, &err);
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Content Type parm count = %d", fn, count));
	for (i=0; i<count; i++)
	{
		pParam=NULL;
		pValue = pName = NULL;

		sip_getParamAtIndexFromContentTypeHdr (pContentType, &pParam, i, &err);

		if ( sip_getNameFromSipParam(pParam,&pName,&err) == SipFail )
		{
		    NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Error in sip_getNameFromSipParam : %d \n", fn, err));
		}
		else
		{
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Name in SIP-Param : %s \n", fn, pName));
			if ( sip_getValueAtIndexFromSipParam(pParam,&pValue,0,&err) == SipFail )
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("\nError in sip_getValueAtIndexFromSipParam : %d \n" ,err));
			}
			else
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4,("Value in SIP-Param : %s \n",pValue));
				if (strcasecmp (pName, "version") == 0)
				{
					appMsgHandle->qsigTypeVersion = strdup(pValue);	// Save value
				}
			}
		}

		sip_freeSipParam(pParam) ;
	}

	if(pContentDisp)
	{
		sip_getDispTypeFromContentDispositionHdr(pContentDisp, &disp, &err);
		appMsgHandle->qsigDisposition = strdup (disp);

		sip_getParamCountFromContentDispositionHdr (pContentDisp, &count, &err);
		for (i=0; i<count; i++)
		{
			SipParam *pParam=NULL;
			pValue = pName = NULL;

			sip_getParamAtIndexFromContentDispositionHdr (pContentDisp, &pParam, i, &err);

			if ( sip_getNameFromSipParam(pParam,&pName,&err) == SipFail )
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Error in sip_getNameFromSipParam : %d \n", fn, err));
			}
			else
			{
				NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Name in SIP-Param : %s \n", fn, pName));
				if ( sip_getValueAtIndexFromSipParam(pParam,&pValue,0,&err) == SipFail )
				{
					NETDEBUG (MSIP, NETLOG_DEBUG4, ("\nError in sip_getValueAtIndexFromSipParam : %d \n" ,err));
				}
				else
				{
					NETDEBUG (MSIP, NETLOG_DEBUG4,("Value in SIP-Param : %s \n",pValue));
					if (strcasecmp (pName, "handling") == 0)
					{
						appMsgHandle->qsigHandling = strdup(pValue);	// Save value
					}
				}
			}

			sip_freeSipParam(pParam);
		}
	}

    sip_bcpt_getQsigFromMsgBody(msgbody, &qsig, &err);

    sip_bcpt_getLengthFromQsigMessage(qsig, &appMsgHandle->qsig_msg_len, &err);

    if(appMsgHandle->qsig_msg = malloc(appMsgHandle->qsig_msg_len))
    {
		sip_bcpt_getBodyFromQsigMessage(qsig, &qsigBody, &err);
		memcpy(appMsgHandle->qsig_msg, qsigBody, appMsgHandle->qsig_msg_len);
    }
	else
	{
		NETERROR (MSIP, ("%s Failed to malloc %d bytes for qsig message\n",fn,appMsgHandle->qsig_msg_len));
	}

_done:
	if (qsig)
		sip_bcpt_freeQsigMessage(qsig);

    return ret;
}


static int 
split (char  *buf, char *out[], char delim)
{
	char *ptr, *nbuf= buf;
	int count = 0;

	if (buf == NULL) return 0;
	while (1) {
		ptr = strchr (nbuf, delim);
		if (ptr == NULL) {
			out[count++] = nbuf;
			return count;
		}
		out[count++] = nbuf;
		*ptr = 0;
		nbuf = ptr + 1;
	}
}
		

static int
parse_dtmf (char *pbuf, DTMFParams *dtmf)
{
	char *ptr;
	char *nameval[32], *list[32];
	char fn[] = "parse_dtmf()";
	int i, count;

	if (pbuf == NULL) return -1;
	if ((count = split (pbuf, list, '\n')) == 0) 
	{
		NETERROR (MSIP, ("%s: split returned 0 elements", fn));
		return -1;
	}
	for (i=0;i<count; i++) {
		if (split (list[i], nameval, '=') == 0) 
			continue;
		if (strcasecmp (nameval[0], "Signal") == 0)
			dtmf->sig = nameval[1][0];
		if (strcasecmp (nameval[0], "Duration") == 0)
			dtmf->duration = atoi (nameval[1]);
	}
	return 0;
}

int
SipFormDtmfParamFromMsg (SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle)
{
	char fn[] = "SipFormDtmfParamFromMsg ()";
	SipUnknownMessage *msg = NULL;
	SipError err;
	SipHeader *header = NULL;
	int count=0, length=0;
	char *pbuf;
	int sig, duration;

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering..\n", fn));

	if (sip_getUnknownFromMsgBody (msgbody, &msg, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to get msg from body", fn));
		goto _error;
	}
	if (sip_getBufferFromUnknownMessage (msg, &pbuf, &err) == SipFail) 
	{
		NETERROR (MSIP, ("%s failed to get buffer from msg", fn));
		goto _error;
	}
	if (sip_getLengthFromUnknownMessage (msg, &length, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to get length from msg", fn));
		goto _error;
	}
	appMsgHandle->dtmf = (DTMFParams *) malloc (sizeof (DTMFParams));
	bzero (appMsgHandle->dtmf, sizeof (DTMFParams));
	/* We only have one dtmf structure */
	appMsgHandle->ndtmf = 1;
	if (parse_dtmf (pbuf, appMsgHandle->dtmf) < 0) 
	{
		NETERROR (MSIP, ("%s failed to parse dtmf-relay content.", fn));
		goto _error;
	}
	
	sip_freeSipUnknownMessage (msg);
	return 0;
 _error:
	sip_freeSipUnknownMessage(msg);
	return -1;
}

int
SipFormDtmfFromMsgHandle (SipMsgBody **msgbody, SipAppMsgHandle *appMsgHandle)
{
	char fn[] = "SipFormDtmfFromMsgHandle()";
	SipError err;
	SipUnknownMessage *dtmfMsg;
	char dtmfmsg[1024], *ptr;
	int i;

	if (sip_initSipMsgBody (msgbody, SipUnknownBody, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s fail to init msg body\n", fn));
		goto _error;
	}

	if (sip_initSipUnknownMessage (&dtmfMsg, &err) == SipFail) 
	{
		NETERROR (MSIP, ("%s fail to init message body", fn));
		goto _error;
	}

	bzero (dtmfmsg, 1024);
	ptr = dtmfmsg;
	for (i = 0; i < appMsgHandle->ndtmf; i++)
	{
		ptr += sprintf (ptr, "Signal=%c\r\nDuration=%d\r\n", 
				appMsgHandle->dtmf[i].sig, appMsgHandle->dtmf[i].duration);
	}

	if (sip_setBufferInUnknownMessage (dtmfMsg, strdup (dtmfmsg), strlen(dtmfmsg), &err) == SipFail)
	{
		NETERROR (MSIP, ("%s fail to set buffer..", fn));
		goto _error;
	}

	if (sip_setUnknownInMsgBody (*msgbody, dtmfMsg, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set msg body", fn));
		goto _error;
	}

	sip_freeSipUnknownMessage (dtmfMsg);

	return 0;

 _error:
    sip_freeSipMsgBody(*msgbody);
    msgbody = NULL;
    return -1;
}

/* return 0 if no error
 * sipmessage is allocated outside of this function */
int SipFormSDPBodyFromMsgHandle(SipMessage *m, SipAppMsgHandle *appMsgHandle, SipMsgBody **ppMsgbody)
{
	char fn[]="SipFormSDPBodyFromMsgHandle()";
	RTPSet *rtpSet=NULL;
	SDPAttr *sdpattr=NULL;		/* our own structure */
	int count, acount = 0, rtpSetCount = 0;
	SipMsgBody *msgbody=NULL;
	SdpMessage *pSdp=NULL;
	SdpOrigin *pOrigin=NULL;
	SdpConnection *pConn=NULL;
	SdpTime *pTime=NULL;
	SdpMedia *pMedia=NULL;
	SdpAttr *pAttr=NULL;
	unsigned long version;
	SIP_S8bit *tmp=NULL;
	SIP_S8bit tmpstr[24];
	char ConnAddr[16];
	SIP_S8bit CodecList[64] = { 0 };
	unsigned long rtpaddr=0;
	unsigned short rtpport=0;
	int i, j, n, codec[16];
	SipError err;
	int mediaType = MediaAudio;
	int transportType = TransportRTPAVP;
	int curMLine = -1;
	int bwcount = 0, mbwcount;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

	n = appMsgHandle->nlocalset;
	count = appMsgHandle->attr_count;

	if( sip_initSipMsgBody(&msgbody, SipSdpBody, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to init msg body\n", fn));
		goto _error;
	}

	if( sip_initSdpMessage(&pSdp, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to init sdp message\n", fn));
		goto _error;
	}

	if( sip_initSdpOrigin(&pOrigin, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to init sdp origin\n", fn));
		goto _error;
	}

	/* form version */
	if( sdp_setVersion(pSdp, strdup("0"), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set version\n", fn));
		goto _error;
	}

	/* form session name */
	if( sdp_setSession(pSdp, strdup("sip call"), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set session name\n", fn));
		goto _error;
	}

	/* form origin */
	if( sdp_setUserInOrigin(pOrigin, strdup(sipservername), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set origin user name\n", fn));
		goto _error;
	}

	if( sdp_setSessionIdInOrigin(pOrigin, strdup("1234"), &err)== SipFail)
	{
		NETERROR(MSIP, ("%s fail to set origin session id\n", fn));
		goto _error;
	}

	version = appMsgHandle->sdpVersion;
	memset( tmpstr, (int) 0, 24 );
	sprintf( tmpstr, "%lu", version );

	if( sdp_setVersionInOrigin(pOrigin, strdup(tmpstr), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set origin version\n", fn));
		goto _error;
	}

	if( sdp_setNetTypeInOrigin(pOrigin, strdup("IN"), &err)== SipFail)
	{
		NETERROR(MSIP, ("%s fail to set origin net type\n", fn));
		goto _error;
	}

	if( sdp_setAddrTypeInOrigin(pOrigin, strdup("IP4"), &err)== SipFail)
	{
		NETERROR(MSIP, ("%s fail to set origin addr type\n", fn));
		goto _error;
	}

	if( sdp_setAddrInOrigin(pOrigin, strdup(SVal(appMsgHandle->pOriginIpAddress)), &err)== SipFail)
	{
		NETERROR(MSIP, ("%s fail to set origin addr\n", fn));
		goto _error;
	}

	if( sdp_setOrigin(pSdp, pOrigin, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set origin in sdp\n", fn));
		goto _error;
	}

	if( sip_initSdpTime(&pTime, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to init sdp time\n", fn));
		goto _error;
	}

	/* form time */
	if( sdp_setStartInTime(pTime, strdup("0"), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set start in time\n", fn));
		goto _error;
	}

	if( sdp_setStopInTime(pTime, strdup("0"), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set stop in time\n", fn));
		goto _error;
	}

	if( sdp_insertTimeAtIndex(pSdp, pTime, 0, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to insert time in sdp\n", fn));
		goto _error;
	}

	// Add the default C line first. We do not know
	// how many m lines there are in all, as all the rtpSet
	// contains is the different set of lines all in one
	// array, with same m line occupying multiple entries
	if( sip_initSdpConnection(&pConn, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to init sdp connection\n", fn));
		goto _error;
	}

	/* form default connection */
	if( sdp_setNetTypeInConnection(pConn, strdup("IN"), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set conn net type\n", fn));
		goto _error;
	} 

	if( sdp_setAddrTypeInConnection(pConn, strdup("IP4"), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set conn addr type\n", fn));
		goto _error;
	}

	// Use first one for now
	rtpSet = (RTPSet *) appMsgHandle->localSet;
	FormatIpAddress(rtpSet->rtpaddr, ConnAddr);

	tmp = &ConnAddr[0];
	if( sdp_setAddrInConnection(pConn, strdup(tmp), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set conn addr type\n", fn));
		goto _error;
	}

	if( sdp_setConnection(pSdp, pConn, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set conn in sdp\n", fn));
		goto _error;
	}

	// Free up the default value
	sip_freeSdpConnection(pConn);
	pConn = NULL;

	while (rtpSetCount < n)
	{
		mbwcount = 0;

		rtpSet = (RTPSet *) appMsgHandle->localSet + rtpSetCount;
		if(rtpSet == NULL)
		{
			NETERROR(MSIP, ("%s nlocalset=%d, but localset=NULL\n", fn, n));
			goto _error;
		}

		curMLine = rtpSet->mLineNo;

		FormatIpAddress(rtpSet->rtpaddr, ConnAddr);

		rtpport = (rtpSet->rtpport);
		rtpaddr = htonl(rtpSet->rtpaddr);
	
		/* form codec list as string */
		tmp = &CodecList[0];
		memset(tmp, 0, 64);

		//  If rtpCount < n, we WILL enter the loop,
		// and on exit, i must be in the new m line
		i=0;
		while (i<(n-rtpSetCount))
		{
			if (curMLine != rtpSet[i].mLineNo)
			{
				// This clearly WILL not happen for i=0
				if (i==0)
				{
					NETERROR(MSIP, ("%s Fatal error memory corruption\n",
						fn));
					i++;	// to get us out of this mess
				}
				break;
			}

			mediaType = rtpSet[i].mediaType;

			if ((int)rtpSet[i].codecType == T38Fax)
			{
				sprintf(tmpstr, "%st38", (i==0)?"":" ");
				strcat(tmp, tmpstr);
				mediaType = MediaImage;
				transportType = TransportUDPTL;
			}
			else if((int)rtpSet[i].codecType >= 0)
			{
				sprintf(tmpstr, "%s%d", (i==0)?"":" ", rtpSet[i].codecType);
				strcat(tmp, tmpstr);
			}

			i++;
		}

		// point to the new m  line
		rtpSetCount += i;

		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s codecs = %s\n", fn, CodecList));

		if( sip_initSdpMedia(&pMedia, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to init sdp media\n", fn));
			goto _error;
		}

		if (appMsgHandle->localSet[0].rtpaddr != rtpSet->rtpaddr)
		{
			// different from default
			// new c line needed
			if( sip_initSdpConnection(&pConn, &err) == SipFail)
			{
				NETERROR(MSIP,
				("%s fail to init sdp connection\n", fn));
				goto _error;
			}

			/* form connection */
			if( sdp_setNetTypeInConnection(pConn, strdup("IN"), &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set conn net type\n", fn));
				goto _error;
			}

			if( sdp_setAddrTypeInConnection(pConn, strdup("IP4"), &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set conn addr type\n", fn));
				goto _error;
			}

			tmp = &ConnAddr[0];
			if( sdp_setAddrInConnection(pConn, strdup(tmp), &err) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set conn addr type\n", fn));
				goto _error;
			}

			if( sdp_insertConnectionAtIndexInMedia(pMedia, pConn, 0, &err) 
				== SipFail)
			{
				NETERROR(MSIP, ("%s fail to set conn in sdp\n", fn));
				goto _error;
			}
		}

		/* form media */
		if( sdp_setMvalueInMedia(pMedia, strdup(SipMediaTypeAsStr(mediaType)),
			&err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set audio in media\n", fn));
			goto _error;
		}

		if( sdp_setPortInMedia(pMedia, rtpport, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set port in media\n", fn));
			goto _error;
		}

		if( sdp_setProtoInMedia(pMedia, strdup(SipTransportAsStr(transportType)),
			&err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set transport in media\n", fn));
			goto _error;
		}

		tmp = &CodecList[0];
		if( sdp_setFormatInMedia(pMedia, strdup(tmp), &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set fmt in media\n", fn));
			goto _error;
		}

		sdpattr = appMsgHandle->attr + acount;
		if(count == 0)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no attr\n", fn));
			goto _return;
		}
		else if(sdpattr == NULL)
		{
			NETERROR(MSIP, ("%s found %d attr, but sdpattr=NULL\n", fn, count));
			goto _error;
		}

		i=0; j=0;
		while ((acount < count) &&
				(sdpattr[i].mLineNo == curMLine))
		{
			switch(sdpattr[i].type)
			{
			case 'a':
				if( sip_initSdpAttr(&pAttr, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s fail to init attr\n", fn));
					goto _error;
				}
				if(sdpattr[i].name == NULL)
				{
					NETERROR(MSIP, ("%s attr name is NULL\n", fn));
					goto _error;
				}
				if( sdp_setNameInAttr(pAttr, strdup(sdpattr[i].name), &err) == SipFail)
				{
					NETERROR(MSIP, ("%s fail to set attr name\n", fn));
					goto _error;
				}

				if(sdpattr[i].value != NULL)
				{
					if( sdp_setValueInAttr(pAttr, strdup(sdpattr[i].value), &err) == SipFail)
					{
						NETERROR(MSIP, ("%s fail to set attr name\n", fn));
						goto _error;
					}
				}

				if( sdp_insertAttrAtIndexInMedia(pMedia, pAttr, j, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s fail to insert attr in media\n", fn));
					goto _error;
				}

				sip_freeSdpAttr(pAttr);
				pAttr = NULL;
				j++;
				break;

			case 'B':
				if( sdp_insertBandwidthAtIndex(pSdp, strdup(sdpattr[i].value), bwcount++, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s fail to insert bandwidth in media\n", fn));
					goto _error;
				}
				break;

			case 'b':
				if( sdp_insertBandwidthAtIndexInMedia(pMedia, strdup(sdpattr[i].value), mbwcount++, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s fail to insert bandwidth in media\n", fn));
					goto _error;
				}
				break;

			case 'K':
				if( sdp_setKey(pSdp, strdup(sdpattr[i].value), &err) == SipFail)
				{
					NETERROR(MSIP, ("%s fail to insert key\n", fn));
					goto _error;
				}
				break;

			case 'k':
				if( sdp_setKeyInMedia(pMedia, strdup(sdpattr[i].value), &err) == SipFail)
				{
					NETERROR(MSIP, ("%s fail to insert key in media\n", fn));
					goto _error;
				}
				break;
			}

			++i; acount ++;
		}

 	_return:
		if( sdp_insertMediaAtIndex(pSdp, pMedia, curMLine, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to insert media in sdp\n", fn));
			goto _error;
		}

		sip_freeSdpMedia(pMedia);
		pMedia = NULL;
		sip_freeSdpConnection(pConn);
		pConn = NULL;
	}

	/* set msg body */
	if( sip_setSdpInMsgBody(msgbody, pSdp, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set sdp in msg body\n", fn));
		goto _error;
	}

	sip_freeSdpMessage(pSdp);
	sip_freeSdpOrigin(pOrigin);
	sip_freeSdpConnection(pConn);	n = appMsgHandle->nlocalset;
	sip_freeSdpTime(pTime);
	sip_freeSdpMedia(pMedia);
	sip_freeSdpAttr(pAttr);

    *ppMsgbody = msgbody;

	return 0;
 _error:
	sip_freeSdpMessage(pSdp);
	sip_freeSdpOrigin(pOrigin);
	sip_freeSdpConnection(pConn);
	sip_freeSdpTime(pTime);
	sip_freeSdpMedia(pMedia);
	sip_freeSdpAttr(pAttr);

	return -1;
}


/* return 0 if no error
 * sipmessage is allocated outside of this function */
int SipFormSDPFromMsgHandle(SipMessage *m, SipAppMsgHandle *appMsgHandle)
{
	char        fn[]="SipFormSDPFromMsgHandle()";
	SipHeader   *header=NULL;
	SipMsgBody  *msgbody=NULL;
	SipError    err;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

	if (appMsgHandle->nlocalset == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s no codec found in appMsgHandle\n", fn));
		return 0;
	}

	/* init structures */
	if( sip_initSipHeader (&header, SipHdrTypeContentType , &err) == SipFail )
    {
		NETERROR(MSIP, ("%s fail to init content type hdr\n", fn));
		goto _error;
	}

    if(SipFormSDPBodyFromMsgHandle(m, appMsgHandle, &msgbody) < 0)
	{
		NETERROR(MSIP, ("%s fail to build sdp body\n", fn));
		goto _error;
	}


	if( sip_insertMsgBodyAtIndex(m, msgbody, 0, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set sdp in msg \n", fn));
		goto _error;
	}

	/* content type hdr */
	if( sip_setMediaTypeInContentTypeHdr(header, strdup("application/sdp"), &err)
	    ==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set media type in content hdr \n", fn));
		goto _error;
	}

	if( sip_setHeader(m, header, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set content type hdr in msg\n", fn));
		goto _error;
	}

	sip_freeSipHeader(header);
	sip_freeSipMsgBody(msgbody);
	free(header);

	return 0;
 _error:
	sip_freeSipHeader(header);
	sip_freeSipMsgBody(msgbody);
	free(header);

	return -1;
}

int
SipFormSipFragFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle)
{
	char*        fn     = "SipFormSipFragFromMsg()";
	SipMessage*  msg    = NULL;
	char*        pbuf   = NULL;
	SipError     err;   
	char*        strptr = NULL;
	int          j;

	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s entering",fn));
	
	if(sip_getSipMessageFromMsgBody(msgbody,&msg,&err) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to get msg from body", fn));
		goto _error;
	}
	
	if(sip_getStatusLineAsString(msg,&(pbuf),&err) == SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s error extracting sipfrag from msg",fn));
		goto _error;
	}
	else
	{
		// REMOVE trailing CRLF
		strptr = pbuf;
		for(j=0;j<strlen(strptr);j++)
		{
			if(strptr[j] == '\r' || strptr[j] == '\n')
			{
				strptr[j] = '\0';
				break;
			}
		}
		appMsgHandle->sip_frag = pbuf;
	}

	sip_freeSipMessage (msg);
	return 0;
 _error:
	sip_freeSipMessage (msg);
	return -1;
}

int
SipInsertSipFrag(SipMessage* m, SipAppMsgHandle* appMsgHandle)
{
	char*        	   fn        = "SipInsertSipFrag()";
	SipMsgBody*  	   msgbody   = NULL;
	SipUnknownMessage* msg       = NULL;
	char               msgptr[1024];
	SipError     	   err;

	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Entering %s", fn,appMsgHandle->sip_frag));

	if (sip_initSipUnknownMessage (&msg, &err) == SipFail) 
	{
		NETERROR (MSIP, ("%s fail to init message body", fn));
		goto _error;
	}
	bzero(msgptr,1024);
	sprintf(msgptr,"%s",appMsgHandle->sip_frag);

	if (sip_setBufferInUnknownMessage (msg, strdup (msgptr), strlen (msgptr), &err) == SipFail)
	{
		NETERROR (MSIP, ("%s fail to set buffer..", fn));
		goto _error;
	}
	
	if (sip_initSipMsgBody (&msgbody, SipUnknownBody, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s fail to init msg body\n", fn));
		goto _error;
	} 

	if (sip_setUnknownInMsgBody (msgbody, msg, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set msg body", fn));
		goto _error;
	}

	if(sip_insertMsgBodyAtIndex(m,msgbody,0,&err) == SipFail)
	{
		NETERROR(MSIP,("%s: Unable to insert msgbody in message ", fn));
		goto _error;
	}

	sip_freeSipUnknownMessage (msg);
	sip_freeSipMsgBody(msgbody);
	return 0;	
 _error:
	sip_freeSipUnknownMessage (msg);
	sip_freeSipMsgBody(msgbody);
	return -1;
}
