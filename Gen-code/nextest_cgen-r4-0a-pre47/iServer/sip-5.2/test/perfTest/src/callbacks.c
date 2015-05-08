/*****************************************************************************
 ** FUNCTION:
 **			The HSS SIP Stack performance testing application.
 **
 *****************************************************************************
 **
 ** FILENAME:		callbacks.c
 **
 ** DESCRIPTION:	This file contains the implementations for the
 **					callback methods that are expected by the stack.
 **					This also contains the implementation for the
 **					additional functions related to the Hash Implementation.
 **
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			---------		------
 ** 10-Nov-01		Ramu K							Creation
 **
 *****************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 *****************************************************************************/

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
#include <pthread.h>
#include <strings.h>
#include <sys/time.h>
#endif


#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <unistd.h>
#include <stdio.h>


#include <stdlib.h>
#include "sipcommon.h"
#include "sipstruct.h"
#include "sipdecode.h"
#include "sipsendmessage.h"
#include "sipfree.h"
#include "siptimer.h"
#include "sipclient.h"
#include "siphash.h"

#ifdef SIP_TXN_LAYER
#include "txndecode.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern SIP_U32bit dServer_fd;

/******************************************************************************
 ** FUNCTION: 		sip_freeEventContext
 **
 ** DESCRIPTION: 	This is the function frees the EventContext Structure
 **
 ******************************************************************************/
void sip_freeEventContext(SipEventContext *pContext)
{
	fast_memfree(0,pContext->pData,SIP_NULL);
	fast_memfree(0,pContext,SIP_NULL);
}

#ifdef SIP_SELECTIVE_PARSE
/******************************************************************************
 ** FUNCTION: 		sip_willParseSdpLine
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_SELECTIVE_PARSE option is enabled.
 **
 ******************************************************************************/
SIP_S8bit sip_willParseSdpLine (SIP_S8bit *pIn, SIP_S8bit **ppOut)
{
	(void)pIn;
	*ppOut = SIP_NULL;
	return 1;
}

/******************************************************************************
 ** FUNCTION: 		sip_acceptIncorrectSdpLine
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_SELECTIVE_PARSE option is enabled.
 **
 ******************************************************************************/
SipBool sip_acceptIncorrectSdpLine (SIP_S8bit *pIn)
{
	(void)pIn;
	return SipFail ;
}
/******************************************************************************
 ** FUNCTION: 		sip_willParseHeader
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_SELECTIVE_PARSE option is enabled.
 **
 ******************************************************************************/
SIP_S8bit sip_willParseHeader (en_HeaderType dType, SipUnknownHeader *pHdr)
{
	(void)dType;
	(void)pHdr;
	return 1;
}
/******************************************************************************
 ** FUNCTION: 		sip_acceptIncorrectHeader
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_SELECTIVE_PARSE option is enabled.
 **
 ******************************************************************************/
SipBool sip_acceptIncorrectHeader (en_HeaderType dType, SipUnknownHeader *pHdr)
{
	(void)dType;
	(void)pHdr;
	return SipFail;
}
#endif

#ifdef SIP_MIB
void sip_indicateResponseHandled(SipMessage *pM,en_SipMibCallbackType dEvent,\
		SIP_S32bit	dCode,SipStatParam	dParam)
{
	(void)pM;
	(void)dEvent;
	(void)dCode;
	(void)dParam;
}
#endif

#ifdef SIP_RETRANSCALLBACK
#ifdef  SIP_TXN_LAYER
/******************************************************************************
 ** FUNCTION: 		sip_indicateMessageRetransmission
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_RETRANSCALLBACK option is enabled.
 **
 ******************************************************************************/
void sip_indicateMessageRetransmission (SipEventContext *pContext,\
	SipTxnKey *pKey, SIP_S8bit *pBuffer,\
	SIP_U32bit dBufferLength, SipTranspAddr *pAddr, SIP_U8bit dRetransCount,\
	SIP_U32bit dDuration)
{
	(void) pContext;
	(void) pKey;
	(void) dBufferLength;
	(void) pAddr;
	(void ) dRetransCount;
	(void ) pBuffer;
	(void ) dDuration;
	SIPDEBUG("Inside sip_indicateMessageRetransmission.\n");
}
#else
/*******************************************************************************
 ** FUNCTION: 		sip_indicateMessageRetransmission
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_RETRANSCALLBACK option is enabled.
 **
 ******************************************************************************/
void sip_indicateMessageRetransmission (SipEventContext *pContext,\
	SipTimerKey *pKey, SIP_S8bit *pBuffer,\
	SIP_U32bit dBufferLength, SipTranspAddr *pAddr, SIP_U8bit dRetransCount,\
	SIP_U32bit dDuration)
{
	(void) pContext;
	(void) pKey;
	(void) dBufferLength;
	(void) pAddr;
	(void ) dRetransCount;
	(void ) pBuffer;
	(void ) dDuration;
	SIPDEBUG("Inside sip_indicateMessageRetransmission.\n");
}
#endif
#endif

/******************************************************************************
 ** FUNCTION: 		sip_decryptBuffer
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **
 **
 ******************************************************************************/
SipBool sip_decryptBuffer(SipMessage *s, SIP_S8bit *pEncinbuffer,\
	SIP_U32bit dClen,SIP_S8bit **ppEncoutbuffer, SIP_U32bit *pOutlen)
{
	const SIP_S8bit *pReplace_string = "content-type: application/sdp\r\n"
		"m: sip:hsssip \r\n\r\n"
		"v=1\no=mephoney 29739 7272939 IN IP4 139.85.229.128\n"
		"c=IN IP4 139.85.229.168\nm=audio 492170 RTP/AVP 0 15\n"
		"m=video 3227 RTP/AVP 33\na=rtpmap:31 LPR\n\n";
	SipMessage *pS_dummy;
	SIP_S8bit* pEncinbuffer_dummy;
	SIP_U32bit dClen_dummy;

	pS_dummy =s;
	pEncinbuffer_dummy = pEncinbuffer;
	dClen_dummy = dClen;

	*ppEncoutbuffer = (SIP_S8bit *) fast_memget(0,1000,NULL);
	*pOutlen = 193;

	SIPDEBUG("\n-----Decrypt Buffer Callback-----------------------------\n");
	SIPDEBUG("Inside sip_decryptBuffer demo implementation.\n");
	strcpy(*ppEncoutbuffer, pReplace_string);
	SIPDEBUG("-----Decrypt Buffer Callback Ends------------------------\n\n");
	return SipSuccess;
}

#ifndef SIP_TXN_LAYER
/******************************************************************************
 ** FUNCTION: 		sip_timerElementFree
 **
 ** DESCRIPTION: 	This function frees the timerElement
 **
 **
 ******************************************************************************/
void sip_timerElementFree(SIP_Pvoid pElement)
{
	TimerHashElement *pElem;

	pElem = (TimerHashElement *) pElement;
	sip_freeSipTimerKey(pElem->key);
	sip_freeSipTimerBuffer((SipTimerBuffer *)pElem->buffer);
}
#endif


/******************************************************************************
 ** FUNCTION: 		sip_sendToNetwork
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **
 **
 ******************************************************************************/
SipBool sip_sendToNetwork
(SIP_S8bit *pBuffer, SIP_U32bit dBuflen,SipTranspAddr *pAddr, \
	SIP_S8bit dTransptype, SipError *pErr)
{
	SIP_S32bit dLen;
	struct sockaddr_in  serv_addr;
	SIP_S8bit dDummy;
	SipError *pErr_dummy;

	dDummy = dTransptype;
	pErr_dummy = pErr;

	bzero((SIP_S8bit *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(pAddr->dIpv4);
	serv_addr.sin_port        = htons(pAddr->dPort);

	dLen=sendto(dServer_fd, pBuffer, dBuflen, 0, (struct sockaddr*) \
	&serv_addr, sizeof(serv_addr));

	if(dLen == -1 || (SIP_U32bit)dLen != dBuflen)
	{
		SIPDEBUG("Client : send to failed.\n");
		return SipFail;
	}
	return SipSuccess;
}

#ifndef SIP_TXN_LAYER
/******************************************************************************
 ** FUNCTION: 		sip_indicateTimeOut
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **
 **
 ******************************************************************************/
void sip_indicateTimeOut( SipEventContext *pContext)
{
	SipCallContext *pCall_context;
	SIP_U32bit dCallindex;
	SipError dErr;
	pCall_context = (SipCallContext *) pContext->pData;
	if (glbClient_mode == 1)
	{
		dCallindex = atoi(pCall_context->callId)-1;
		glbpTest_statistics[0].timed_out_calls++;
		fast_lock_synch(0,&glbpCallObjectArray[dCallindex].callLock,0);
		glbpCallObjectArray[dCallindex].state = SipInviteSent;
		sip_sendRequest("INVITE", &dClient_params.client_dest_addr,
			pCall_context->callId, &dErr);
		fast_unlock_synch(0,&glbpCallObjectArray[dCallindex].callLock);
	}
	sip_freeEventContext(pContext);
}
#endif

#ifndef SIP_TXN_LAYER
/******************************************************************************
 ** FUNCTION: 		sip_stringKeyCompareFunction
 **
 ** DESCRIPTION: 	Function used to compare the Keys
 **
 **
 ******************************************************************************/
SIP_U8bit sip_stringKeyCompareFunction \
					(void *pKey1, void *pKey2)
{
	return strcmp((SIP_S8bit*) pKey1, (SIP_S8bit*) pKey2);
}

/******************************************************************************
 ** FUNCTION: 		sip_hashElementFreeFunction
 **
 ** DESCRIPTION: 	Function used to free the hash Element
 **
 **
 ******************************************************************************/
void sip_hashElementFreeFunction(void *pElement)
{
	fast_memfree(0,pElement,SIP_NULL);
}

/******************************************************************************
 ** FUNCTION: 		sip_hashKeyFreeFunction
 **
 ** DESCRIPTION: 	Function used to free the key
 **
 **
 ******************************************************************************/
void sip_hashKeyFreeFunction (void *pKey)
{
	SIP_Pvoid pDummy;
	pDummy=pKey;
}
#endif

#ifdef __cplusplus
}
#endif
