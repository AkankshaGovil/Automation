/* 	==============================================================
	Filename: encodetime.c
	--------

	Details:
	-------
	This test stub can be used to calculate the encoding time of the stack.
	Encoding time is calculated in two ways :
	a) sip_formMessage is called NUM_ITERATIONS times and the average time
		to encode the message is computed.
	b) A sample SipMessage is formed using the SIP stack accessor API's and
		finally sip_formMessage is called. This is done NUM_ITERATIONS times
		and the average time to encode the message is computed.

	There is a call to
	i) clock in case of Solaris
	ii) clock_gettime in case of VxWorks
	iii) get_ticks() in case of OSE
	at the beginning of the loop and at the end which gives us the
	CPU time (or absolute time in case of VxWorks/OSE) taken for those
	many decodes.

	Dividing the time taken by the number of iterations gives the
	encode time for one message.

	Notes:
	-----
	1. The stack should be compiled in SIP_BY_REFERENCE mode to execute
		this program.

	2. For VxWorks please change the define for TARGET_IP at the
		beginning of the file.

	2. 	The number of iterations is configurable and can be changed
		by changing the define for NUM_ITERATIONS

	Usage:
	-----
		 encodetime <listening_port>

	==============================================================
				(c) Hughes Software Systems, 2001
	============================================================== */
#ifdef __cplusplus
extern "C"{
#endif

#define TARGET_IP "139.85.229.176"

#ifndef NUM_ITERATIONS
#define NUM_ITERATIONS  10000
#endif

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
#include <sys/types.h>
#include <sys/time.h>
#include <strings.h>
#endif


#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "portlayer.h"
#include "sipdecode.h"
#include "sipstruct.h"
#include "sipsendmessage.h"
#include "siplist.h"
#include "sipinit.h"
#include "sipfree.h"
#include "general.h"
#include "request.h"
#include "response.h"
#include "entity.h"
#include "ccp.h"
#include "rpr.h"
#include "header.h"
#include "sdp.h"
#include "sipstatistics.h"
#include "siptimer.h"
#include "siptrace.h"
#ifdef SIP_TXN_LAYER
#include "siptxnstruct.h"
#include "txndecode.h"
#include "siptxntimer.h"
#include "siptxntest.h"
#endif


#define MAXMESG 5000

int showMessage = 1;
int constructCLen;





void sip_freeEventContext(SipEventContext *context)
{
	(void)context;
}

SipBool sip_decryptBuffer(SipMessage *s, SIP_S8bit *encinbuffer, SIP_U32bit clen,SIP_S8bit **encoutbuffer, SIP_U32bit *outlen)
{
	(void) s;
	(void) encinbuffer;
	(void) clen;
	(void) encoutbuffer;
	(void) outlen;
	return SipFail;
}

#ifdef SIP_TXN_LAYER
/*Dummy Callbacks*/
SipBool fast_startTxnTimer
#ifdef ANSI_PROTO
    (
    SIP_U32bit dDuration,
    TimeoutFuncPtr funcptr,
    SIP_Pvoid pData,
    SIP_Pvoid* ppHandle,
    SipError *pErr
    )
#else
    (dDuration,funcptr,pData,ppHandle,pErr)
    SIP_U32bit dDuration;
    TimeoutFuncPtr funcptr;
    SIP_Pvoid pData;
    SIP_Pvoid* ppHandle;
    SipError *pErr;
#endif
{
	(void)dDuration;
	(void)funcptr;
	(void)pData;
	(void)ppHandle;
	(void)pErr;
	return SipSuccess;
}

SipBool fast_stopTxnTimer
#ifdef ANSI_PROTO
    ( SIP_Pvoid pHandle,SIP_Pvoid* ppBuff,SipError* pErr )
#else
    (pHandle,ppBuff,pErr)
    SIP_Pvoid pHandle;
    SIP_Pvoid* ppBuff;
    SipError* pErr;
#endif
{
	(void)pHandle;
	(void)ppBuff;
	(void)pErr;
	return SipSuccess;
}

SipBool sip_cbk_fetchTxn
#ifdef ANSI_PROTO
	(SIP_Pvoid* ppTxnBuffer,SIP_Pvoid pTxnKey,SIP_Pvoid pContext,\
		SIP_S8bit dOpt,SipError *pErr)
#else
	(ppTxnBuffer,pTxnKey,pContext,dOpt,pErr)
	SIP_Pvoid* ppTxnBuffer;
	SIP_Pvoid pTxnKey;
	SIP_Pvoid pContext;
	SIP_S8bit dOpt;
	SipError *pErr;
#endif
{
	(void)ppTxnBuffer;
	(void)pTxnKey;
	(void)pContext;
	(void)dOpt;
	(void)pErr;
	return SipSuccess;
}

SipBool sip_cbk_releaseTxn
#ifdef ANSI_PROTO
	(SIP_Pvoid pTxnKey, SIP_Pvoid* ppTxnKey,SIP_Pvoid *ppTxnBuffer, \
		SIP_Pvoid pContext,SIP_S8bit dOpt, SipError *pErr)
#else
	(pTxnKey, ppTxnKey,ppTxnBuffer,pContext,dOpt,pErr)
	SIP_Pvoid pTxnKey;
	SIP_Pvoid* ppTxnKey;
	SIP_Pvoid *ppTxnBuffer;
	SIP_Pvoid pContext;
	SIP_S8bit dOpt;
	SipError *pErr;
#endif
{
	(void)pTxnKey;
	(void)ppTxnKey;
	(void)ppTxnBuffer;
	(void)pContext;
	(void)dOpt;
	(void)pErr;
	return SipSuccess;
}

/*Dummy Callbacks*/
#endif

SipBool fast_startTimer( SIP_U32bit duration, SIP_S8bit restart,SipBool (*timeoutfunc)(SipTimerKey *key, SIP_Pvoid buf), SIP_Pvoid buffer, SipTimerKey *key, SipError *err)
{
	(void) duration;
	(void) restart;
	(void) buffer;
	(void) *key;
	(void) timeoutfunc;
	(void) *err;
	return SipSuccess;
}

/* Implementaion of the fast_stopTimer interface reqiuired by the stack
   Application developers may choose to implement this function in any
   manner while preserving the interface
*/
SipBool fast_stopTimer(SipTimerKey *inkey, SipTimerKey **outkey, SIP_Pvoid *buffer,  SipError *err)
{
	(void) inkey;
	(void) outkey;
	(void) buffer;
	(void) err;
	return SipSuccess;
}

SipBool sip_sendToNetwork
#ifdef ANSI_PROTO
( SIP_S8bit *buffer, SIP_U32bit buflen,SipTranspAddr *addr, SIP_S8bit transptype, SipError *err)
#else
	(buffer, buflen, addr, transptype, err)
	SIP_S8bit *buffer;
	SIP_U32bit buflen;
	SipTranspAddr *addr;
	SIP_S8bit transptype;
	SipError *err;
#endif
{
	(void) *buffer;
	(void) buflen;
	(void) *addr;
	(void) transptype;
	(void) *err;
	return SipSuccess;
}

#ifdef SIP_TXN_LAYER
#ifdef ANSI_PROTO
void sip_indicateTimeOut( SipEventContext *context,en_SipTimerType dTimer)
#else
void sip_indicateTimeOut(context,dTimer)
        SipEventContext *context;
        en_SipTimerType dTimer;
#endif
#else
#ifdef ANSI_PROTO
void sip_indicateTimeOut( SipEventContext *context)
#else
void sip_indicateTimeOut(context)
        SipEventContext *context;
#endif
#endif
{
	(void)context;
#ifdef SIP_TXN_LAYER
	(void)dTimer;
#endif
}


#ifdef SIP_SELECTIVE_PARSE
SIP_S8bit sip_willParseSdpLine (SIP_S8bit *in, SIP_S8bit **out)
{
	(void) in;
	(void) out;
	return 1;
}

SipBool sip_acceptIncorrectSdpLine (SIP_S8bit *in)
{
	(void) in;
	return SipFail ;
}


SIP_S8bit sip_willParseHeader (en_HeaderType type, SipUnknownHeader *hdr)
{
	(void) type;
	(void) hdr;
	return 1;
}

SipBool sip_acceptIncorrectHeader (en_HeaderType type, SipUnknownHeader *hdr)
{
	(void) type;
	(void) hdr;
	return SipFail;
}
#endif





/* Function prototype to avoid warning */
void do_simple_encode(SipMessage *pMsg);
SipBool do_full_encode(void);

void do_simple_encode(SipMessage *pMsg)
{
	SipBool r;
	SipError error;
	SIP_U32bit dLength;
	char *out;
	int i;
	SipOptions opt;
#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	clock_t pstime1, pstime;
#endif



	opt.dOption = 0;

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)

	pstime = clock();
#endif


	out = (SIP_S8bit *) fast_memget(0, SIP_MAX_MSG_SIZE, &error);
	for(i=0; i<NUM_ITERATIONS; i++)
	{
		strcpy(out, "");
		r = sip_formMessage(pMsg, &opt, out, &dLength, &error);
	}
	fast_memfree(0, out, NULL);
#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	pstime1 = clock();
#endif

	printf("\n=========== Results for simple message encoding =========\n\n");

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	printf("CPU clock ticks for %d messages - %d\n", NUM_ITERATIONS, pstime1-pstime);
	printf("CPU time for %d messages - %f sec\n", NUM_ITERATIONS, (float)(pstime1-pstime)/CLOCKS_PER_SEC);
#endif






	printf("\n=========== Results for simple message encoding =========\n");

	if (r==SipFail)
	{
		printf ("+++++++ sip_formMessage failed. Error %d++++++++++\n",error);
	}
}

SipBool do_full_encode(void)
{
	/* Method of request being formed */
	SIP_S8bit pMethod[] = "INVITE";
	SIP_S8bit pSipVersion[] = "SIP/2.0";

	/* Values to be used in the Req-URI header */
	SIP_S8bit pReqURIUser[] = "UserB";
	SIP_S8bit pReqURIHost[] = "www-db.research.bell-labs.com";

	/* Values to be used in the From header */
	SIP_S8bit pFromHeaderDispName[] = "BigGuy";
	SIP_S8bit pFromHeaderUser[] = "UserA";
	SIP_S8bit pFromHeaderHost[] = "here.com";

	/* Values to be used in the To header */
	SIP_S8bit pToHeaderDispName[] = "LittleGuy";
	SIP_S8bit pToHeaderUser[] = "UserB";
	SIP_S8bit pToHeaderHost[] = "there.com";

	/* Value to be used in the Call-Id header */
	SIP_S8bit pCallidHeaderValue[] = "12345601@here.com";

	/* Value to be used in the CSeq header */
	SIP_U32bit dCseq = 1;

	/* Value to be used in the Via header */
	SIP_S8bit pViaHeaderTransport[] = "SIP/2.0/UDP";
	SIP_S8bit pViaHeaderHost[] = "hr450f.eng.ascend.com:5060";

	/* Values to be used in Content-type header */
	SIP_S8bit pContentTypeHeaderValue[] = "application/sdp";

	/* Values to be used in Content-length header */
	SIP_U32bit dContentLengthHeaderValue = 147;

	/* Values to be used for the SDP body */
	SIP_U16bit dMediaPort = 49172;
	SIP_S8bit pMediaFormats[] = "0";
	SIP_S8bit pMediaMvalue[] = "audio";
	SIP_S8bit pMediaProto[] = "RTP/AVP";

	SIP_S8bit pSdpAttrName[] = "rtpmap";
	SIP_S8bit pSdpAttrValue[] = "0 PCMU/8000";

	SIP_S8bit pSdpSessionString[] = "Session SDP";

	SIP_S8bit pSdpStartTime[] = "0";
	SIP_S8bit pSdpStopTime[] = "0";

	SIP_S8bit pSdpVersion[] = "0";

	SIP_S8bit pSdpOriginDispName[] = "UserA";
	SIP_S8bit pSdpOriginVersion[] = "2890844526";
	SIP_S8bit pSdpOriginSessionId[] = "2890844526";
	SIP_S8bit pSdpOriginNetType[] = "IN";
	SIP_S8bit pSdpOriginAddrType[] = "IP4";
	SIP_S8bit pSdpOriginAddr[] = "here.com";

	SIP_S8bit pSdpConnectionNetType[] = "IN";
	SIP_S8bit pSdpConnectionAddrType[] = "IP4";
	SIP_S8bit pSdpConnectionAddr[] = "100.101.102.103";


	SipMessage *pMessage;
	SipError *pErr;
	SipReqLine *pReqLine;
	SipAddrSpec *pAddrspec;
	SipUrl *pSipUrl;
	SipHeader *pHeader;
	SipMsgBody *pMsgBody;
	SdpMessage *pSdpMessage;
	SdpTime *pTime;
	SdpMedia *pSdpMedia;
	SdpOrigin *pOrigin;
	SdpConnection *pConnection;
	SdpAttr *pAttr;

	int i;
	char *out;
	SipOptions opt;
	SIP_U32bit dLength;
	SipBool r;
#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	clock_t pstime1, pstime;
#endif


	opt.dOption = 0;
	pErr = (SipError *)fast_memget(0, sizeof(SipError), NULL);

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	pstime = clock();
#endif
	printf("FULL ENCODE");

	out = (SIP_S8bit *) fast_memget(0, SIP_MAX_MSG_SIZE, pErr);
	for (i = 0; i < NUM_ITERATIONS; i++)
	{
		strcpy(out, "");
		/* Initialize the request message */
		if (sip_initSipMessage(&pMessage, SipMessageRequest, pErr) == SipFail)
		{
			printf("sip_initSipMessage failed\n");
			return SipFail;
		}

		/* 	----------------------------
				Set the Request Line
			---------------------------- */
		if (sip_initSipReqLine(&pReqLine, pErr) == SipFail)
		{
			printf("sip_initSipReqLine failed\n");
			return SipFail;
		}
		if (SipFail == sip_initSipAddrSpec(&pAddrspec,SipAddrSipUri,pErr))
		{
			printf("sip_initSipAddrSpec for Request line failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		if (SipFail == sip_initSipUrl(&pSipUrl,pErr))
		{
			printf("sip_initSipUrl failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setMethodInReqLine(pReqLine, strdup(pMethod),  pErr);
		sip_setVersionInReqLine(pReqLine, strdup(pSipVersion), pErr);
		sip_setUserInUrl(pSipUrl, strdup(pReqURIUser), pErr);
		sip_setHostInUrl(pSipUrl, strdup(pReqURIHost), pErr);
		sip_setUrlInAddrSpec(pAddrspec,pSipUrl,pErr);
		sip_setAddrSpecInReqLine(pReqLine, pAddrspec, pErr);
		sip_setReqLine (pMessage, pReqLine, pErr);

		/* Free local variables */
		sip_freeSipUrl(pSipUrl);
		sip_freeSipAddrSpec(pAddrspec);
		sip_freeSipReqLine(pReqLine);

		/* 	----------------------------
				Set the From Header
			---------------------------- */
		if (SipFail == sip_initSipHeader(&pHeader,SipHdrTypeFrom,pErr))
		{
			printf("sip_initSipHeader failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		if (SipFail == sip_initSipAddrSpec(&pAddrspec,SipAddrSipUri,pErr))
		{
			printf("sip_initSipAddrSpec for From header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		if (SipFail == sip_initSipUrl(&pSipUrl,pErr))
		{
			printf("sip_initSipUrl for From header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setDispNameInToHdr(pHeader, strdup(pFromHeaderDispName), pErr);
		sip_setUserInUrl(pSipUrl, strdup(pFromHeaderUser), pErr);
		sip_setHostInUrl(pSipUrl, strdup(pFromHeaderHost), pErr);
		sip_setUrlInAddrSpec(pAddrspec,pSipUrl,pErr);
		sip_setAddrSpecInFromHdr(pHeader,pAddrspec,pErr);
		sip_setHeader(pMessage, pHeader, pErr);

		/* Free local variables */
		sip_freeSipUrl(pSipUrl);
		sip_freeSipAddrSpec(pAddrspec);
		sip_freeSipHeader(pHeader);
		free(pHeader);

		/* 	----------------------------
				Set the To Header
			---------------------------- */
		if (SipFail == sip_initSipHeader(&pHeader,SipHdrTypeTo,pErr))
		{
			printf("sip_initSipHeader for To header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		if (SipFail == sip_initSipAddrSpec(&pAddrspec,SipAddrSipUri,pErr))
		{
			printf("sip_initSipAddrSpec for To header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		if (SipFail == sip_initSipUrl(&pSipUrl,pErr))
		{
			printf("sip_initSipUrl for To header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setDispNameInToHdr(pHeader, strdup(pToHeaderDispName), pErr);
		sip_setUserInUrl(pSipUrl, strdup(pToHeaderUser), pErr);
		sip_setHostInUrl(pSipUrl, strdup(pToHeaderHost), pErr);
		sip_setUrlInAddrSpec(pAddrspec,pSipUrl,pErr);
		sip_setAddrSpecInToHdr(pHeader,pAddrspec,pErr);
		sip_setHeader(pMessage, pHeader, pErr);

		/* Free local variables */
		sip_freeSipUrl(pSipUrl);
		sip_freeSipAddrSpec(pAddrspec);
		sip_freeSipHeader(pHeader);
		free(pHeader);

		/* 	----------------------------
				Set the Call-id Header
			---------------------------- */
		if (SipFail == sip_initSipHeader(&pHeader,SipHdrTypeCallId,pErr))
		{
			printf("sip_initSipHeader for Call-Id header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setValueInCallIdHdr(pHeader, strdup(pCallidHeaderValue), pErr);
		sip_setHeader(pMessage, pHeader, pErr);
		sip_freeSipHeader(pHeader);
		free(pHeader);

		/* 	----------------------------
				Set the Cseq Header
			---------------------------- */
		if (SipFail == sip_initSipHeader(&pHeader,SipHdrTypeCseq,pErr))
		{
			printf("sip_initSipHeader for Cseq header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setMethodInCseqHdr(pHeader, strdup(pMethod), pErr);
		sip_setSeqNumInCseqHdr(pHeader, dCseq, pErr);
		sip_setHeader(pMessage, pHeader, pErr);
		sip_freeSipHeader(pHeader);
		free(pHeader);

		/* 	----------------------------
				Set the Via Header
			---------------------------- */
		if (SipFail == sip_initSipHeader(&pHeader, \
			SipHdrTypeVia,pErr))
		{
			printf("sip_initSipHeader for Via header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setSentProtocolInViaHdr(pHeader, strdup(pViaHeaderTransport), pErr);
		sip_setSentByInViaHdr(pHeader, strdup(pViaHeaderHost), pErr);
		sip_insertHeaderAtIndex(pMessage, pHeader, (SIP_U32bit)0, pErr);
		sip_freeSipHeader(pHeader);
		free(pHeader);

		/* 	---------------------------------
				Set the Content-type Header
			--------------------------------- */
		if (SipFail == sip_initSipHeader(&pHeader, \
			SipHdrTypeContentType,pErr))
		{
			printf("sip_initSipHeader for Content-type header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setMediaTypeInContentTypeHdr(pHeader, \
			strdup(pContentTypeHeaderValue), pErr);
		sip_setHeader(pMessage, pHeader, pErr);
		sip_freeSipHeader(pHeader);
		free(pHeader);

		/* 	---------------------------------
				Set the Content-length Header
			--------------------------------- */
		if (SipFail == sip_initSipHeader(&pHeader, \
			SipHdrTypeContentLength,pErr))
		{
			printf("sip_initSipHeader for Content-Length header failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sip_setLengthInContentLengthHdr(pHeader, dContentLengthHeaderValue, pErr);
		sip_setHeader(pMessage, pHeader, pErr);
		sip_freeSipHeader(pHeader);
		free(pHeader);


		/* 	--------------------------------------------
				All SIP headers have been set now.
				Proceed to create a new SDP Message.
			-------------------------------------------- */
		/* 	Initialize variables for message body and
			SDP Message */
		if (SipFail == sip_initSipMsgBody(&pMsgBody, \
			SipSdpBody, pErr))
		{
			printf("sip_initSipMsgBody failed\n");
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		if (SipFail == sip_initSdpMessage(&pSdpMessage, pErr))
		{
			printf("sip_initSdpMessage failed\n");
			sip_freeSipMsgBody(pMsgBody);
			sip_freeSipMessage(pMessage);
			return SipFail;
		}

		/* Set the v= line */
		sdp_setVersion(pSdpMessage, strdup(pSdpVersion), pErr);

		/* Set the o= line */
		if (sip_initSdpOrigin(&pOrigin, pErr) == SipFail)
		{
			printf("sip_initSdpOrigin failed\n");
			sip_freeSdpMessage(pSdpMessage);
			sip_freeSipMsgBody(pMsgBody);
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sdp_setUserInOrigin(pOrigin, strdup(pSdpOriginDispName), pErr);
		sdp_setSessionIdInOrigin(pOrigin, strdup(pSdpOriginSessionId), pErr);
		sdp_setVersionInOrigin(pOrigin, strdup(pSdpOriginVersion), pErr);
		sdp_setNetTypeInOrigin(pOrigin, strdup(pSdpOriginNetType), pErr);
		sdp_setAddrTypeInOrigin(pOrigin, strdup(pSdpOriginAddrType), pErr);
		sdp_setAddrInOrigin(pOrigin, strdup(pSdpOriginAddr), pErr);
		sdp_setOrigin(pSdpMessage, pOrigin, pErr);
		sip_freeSdpOrigin(pOrigin);

		/* Set the c= line */
		if ((sip_initSdpConnection(&pConnection, pErr))==SipFail)
		{
			printf("sip_initSdpConnection failed\n");
			sip_freeSdpMessage(pSdpMessage);
			sip_freeSipMsgBody(pMsgBody);
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sdp_setNetTypeInConnection(pConnection, \
			strdup(pSdpConnectionNetType), pErr);
		sdp_setAddrTypeInConnection(pConnection, \
			strdup(pSdpConnectionAddrType), pErr);
		sdp_setAddrInConnection(pConnection, \
			strdup(pSdpConnectionAddr), pErr);
		sdp_setConnection(pSdpMessage, pConnection, pErr);
		sip_freeSdpConnection(pConnection);

		/* set the s= line */
		sdp_setSession(pSdpMessage, strdup(pSdpSessionString), pErr);

		/* Setting the t= field in SDP */
		if (sip_initSdpTime(&pTime, pErr) == SipFail)
		{
			printf("sip_initSdpTime failed\n");
			sip_freeSdpMessage(pSdpMessage);
			sip_freeSipMsgBody(pMsgBody);
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sdp_setStartInTime(pTime, strdup(pSdpStartTime), pErr);
		sdp_setStopInTime(pTime, strdup(pSdpStopTime), pErr);
		sdp_insertTimeAtIndex(pSdpMessage, pTime, (SIP_U32bit)0, pErr);
		sip_freeSdpTime(pTime);

		/* Setting the m= line */
		if (sip_initSdpMedia(&pSdpMedia, pErr) == SipFail)
		{
			printf("sip_initSdpMedia failed\n");
			sip_freeSdpMessage(pSdpMessage);
			sip_freeSipMsgBody(pMsgBody);
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sdp_setMvalueInMedia( \
			pSdpMedia, strdup(pMediaMvalue), pErr);
		sdp_setProtoInMedia(pSdpMedia, strdup(pMediaProto), pErr);
		sdp_setPortInMedia(pSdpMedia, dMediaPort, pErr);
		sdp_setFormatInMedia(pSdpMedia, strdup(pMediaFormats), pErr);

		/* Setting the a= line */
		if (sip_initSdpAttr(&pAttr, pErr) == SipFail)
		{
			printf("sip_initSdpAttr failed\n");
			sip_freeSdpMessage(pSdpMessage);
			sip_freeSipMsgBody(pMsgBody);
			sip_freeSipMessage(pMessage);
			return SipFail;
		}
		sdp_setNameInAttr(pAttr, strdup(pSdpAttrName), pErr);
		sdp_setValueInAttr(pAttr, strdup(pSdpAttrValue), pErr);
		sdp_insertAttrAtIndexInMedia(pSdpMedia, pAttr, 0, pErr);

		sdp_insertMediaAtIndex(pSdpMessage, pSdpMedia, (SIP_U32bit)0, pErr);

		/* Freeing the local reference */
		sip_freeSdpAttr(pAttr);
		sip_freeSdpMedia(pSdpMedia);

		/* Set the filled SdpMessage into the message body now */
		sip_setSdpInMsgBody(pMsgBody, pSdpMessage, pErr);
		sip_insertMsgBodyAtIndex(pMessage, pMsgBody, (SIP_U32bit)0, pErr);

		/* Free local variables */
		sip_freeSdpMessage(pSdpMessage);
		sip_freeSipMsgBody(pMsgBody);

		/* The message has been formed into the SipMessage structure now.
			Call sip_formMessage to encode it into the buffer */
		r = sip_formMessage(pMessage, &opt, out, &dLength, pErr);

		/* Free the SipMessage structure */
		sip_freeSipMessage(pMessage);
	}
	fast_memfree(0, out, NULL);

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	pstime1 = clock();
#endif

	printf("\n=========== Results for message formation + encoding =========\n\n");

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	printf("CPU clock ticks for %d messages - %d\n", NUM_ITERATIONS, pstime1-pstime);
	printf("CPU time for %d messages - %f sec\n", NUM_ITERATIONS, (float)(pstime1-pstime)/CLOCKS_PER_SEC);
#endif


	printf("\n=========== Results for message formation + encoding =========\n\n");

	if (r==SipFail)
	{
		printf ("+++++++ sip_formMessage failed. Error %d++++++++++\n", pErr);
		return SipFail;
	}
	else
		return SipSuccess;
}

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

#ifndef SIP_NO_CALLBACK
void sip_indicateInvite(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateUpdate(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateRegister(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateCancel(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateOptions(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateBye(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}


void sip_indicateAck(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateInfo(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicatePropose(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicatePrack(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateUnknownRequest(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateInformational(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateFinalResponse(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateRefer(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateSubscribe(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateNotify(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}

void sip_indicateMessage(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}
#ifdef SIP_DCS
void sip_indicateComet(SipMessage *s, SipEventContext *context)
{
	(void)context;
	do_simple_encode(s);
	sip_freeSipMessage(s);
	if (do_full_encode() == SipFail)
		printf("do_full_encode failed\n");
}
#endif


#endif

#ifdef SIP_MIB
void sip_indicateResponseHandled(SipMessage *s,en_SipMibCallbackType dEvent,\
		SIP_S32bit	dCode,SipStatParam dParam)
{
	(void)s;
	(void)dEvent;
	(void)dCode;
	(void)dParam;
	sip_freeSipMessage(s);
}
#endif

/* End of callback implementations */


#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
int main(int argc, char * argv[])
#endif
{
	int sockfd;
	int	clilen,n;
	struct sockaddr_in	serv_addr, cli_addr;
	fd_set readfs;
	char *message;
	SIP_S8bit *nextmesg;
	SipError error;

	if(argc<2)
	{
		printf("Usage:\n");
		printf("%s my_port\n",argv[0]);
		exit(0);
	}

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("Server could not open dgram socket\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(atoi(argv[1]));


	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Server couldn't bind to local address.\n");
        close(sockfd);
		exit(0);
	}

	/* Stack and trace initialization */
	sip_initStack();

	printf("\nSip stack product id is:%s\n", (char *)(sip_getPart()) );

	if (sip_setErrorLevel(SIP_Major|SIP_Minor|SIP_Critical, &error)==SipFail)
	{
		printf ("########## Error Disabled at compile time #####\n");
	}


	if (sip_setTraceLevel(SIP_Brief,&error)== SipFail)
	{
		printf ("########## Trace Disabled at compile time #####\n");
	}
	sip_setTraceType(SIP_All,&error);



	for(;;)
	{


#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
	struct timeval tv1, tv2;
#endif



#ifdef SIP_NO_CALLBACK
		SipMessage *pMsg;
#endif

		/* get minimum timeout from the timer list */

		FD_ZERO(&readfs);
		FD_SET(sockfd,&readfs);
		FD_SET(0,&readfs);

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
		gettimeofday(&tv1, NULL);

		if (select(sockfd+1, &readfs, NULL, NULL, NULL) < 0)
		{
				printf("select returned < 0\n");
				fflush(stdout);
                close(sockfd);
				exit(0);
		}

#if defined(SIP_SOLARIS) || defined(SIP_LINUX)
		gettimeofday(&tv2, NULL);
#endif
#endif

		if(FD_ISSET(sockfd,&readfs))
		{
			/* Message on the receive port */
			message = (char *) malloc(sizeof(char)*MAXMESG);

			clilen = sizeof(cli_addr);
			n = recvfrom(sockfd, message, MAXMESG, 0,(struct sockaddr *) &cli_addr, &clilen);

			if (n < 0)
			{
				printf("Server : Error in receive.\n");
                close(sockfd);
				exit(0);
			}
			message[n]='\0';
			do
			{
				SipOptions opt;

				opt.dOption  = SIP_OPT_NOTIMER;

#ifndef SIP_NO_CALLBACK
				if (sip_decodeMessage((SIP_S8bit *)message,&opt, \
					strlen(message),&nextmesg,NULL, &error) == SipFail)
				{
					printf ("+++++++ BAD MESSAGE %d++++++++++\n",error);
				}
#else
				if (sip_decodeMessage((SIP_S8bit *)message,&pMsg,&opt, \
					strlen(message),&nextmesg,NULL, &error) == SipFail)
				{
					printf ("+++++++ BAD MESSAGE %d++++++++++\n",error);
				}
				else
				{
					do_simple_encode(pMsg);
					sip_freeSipMessage(pMsg);
					if (do_full_encode() == SipFail)
						printf("do_full_encode failed\n");
				}
#endif
				fast_memfree(0,message, SIP_NULL);
				message = nextmesg;

			} while (message != SIP_NULL);
		}
		else if (FD_ISSET(0,&readfs))
		{
			/* Key pressed */
			char c;
			c = getchar();

			if (c == 'q')
			{
				exit(0);
            }
		}
	}
}


#ifdef __cplusplus
}
#endif
