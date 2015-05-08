/***************************************************************
** FUNCTION:
**	This file implements the sdp accessor string APIs.
**
****************************************************************
**
** FILENAME:
** sdpstring.c
**
** DESCRIPTION
**
**
**   DATE           		  NAME                     REFERENCE
** --------        	  		 ------                   -----------
** 14 Jul 00	      Siddharth Toshniwal			 	 ---- 
**
** Copyright 1999, Hughes Software Systems, Ltd.
*****************************************************************/


#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "sdpstring.h"
#include "sipinternal.h"
#include "sipfree.h"
#include "siplist.h"
#include "string.h"
#include "sipdecode.h"
#include "sipdecodeintrnl.h"
#include "sipparserinc.h"
#include "sipformmessage.h" /* for __sip_check */

/*#ifdef SIP_THREAD_SAFE
extern synch_id_t glbSipParserMutex	;
#endif*/

#ifdef STRCAT
#undef STRCAT
#endif

#ifndef SIP_MSGBUFFER_CHECK
#define STRCAT(e,a,b) __sip_check(&a,(char*)b)
#else
#define STRCAT(e,a,b) \
do \
{ \
	if(__sip_check(e,&a,(char*)b) == SipFail)\
	{\
		*pErr = E_BUF_OVERFLOW;\
		fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);\
		return SipFail;\
	}\
}\
while(0)
#endif

/*
#define CRLF "\r\n"
*/
/********************************************************************
**
** FUNCTION:  sdp_getSdpConnectionAsString
**
** DESCRIPTION: This function returns the session level "c=" line 
**				of the SDP message (taken as input) as a string.
**
*********************************************************************/
SipBool sdp_getSdpConnectionAsString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit **ppStrConn, SipError *pErr)
#else
	( pSdpMsg,ppStrConn,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit **ppStrConn;
	SipError *pErr;
#endif
{
	SIP_S8bit *pOut;
	SIP_S8bit *pTempOut = SIP_NULL;
	SIPDEBUGFN("Entering function  sdp_getSdpConnectionAsString");
#ifndef SIP_NO_CHECK	
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (ppStrConn == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pSdpMsg->slConnection == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pOut = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, SIP_MAX_HDR_SIZE, pErr);
	if (pOut == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}	
	
	pTempOut = pOut;	

	STRCPY ((char *)pOut, "c=");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut, pSdpMsg->slConnection->pNetType);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut, pSdpMsg->slConnection->pAddrType);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut, pSdpMsg->slConnection->dAddr);
	
	*ppStrConn = (SIP_S8bit *) STRDUPACCESSOR (pOut);
	if (*ppStrConn == SIP_NULL)
	{
		fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
		*pErr = E_NO_MEM;
		return SipFail;
	}	

	fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function  sdp_getSdpConnectionAsString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_getSdpOriginAsString
**
** DESCRIPTION: This function returns the session level "o=" line 
**				of the SDP message (taken as input) as a string.
**
*********************************************************************/
SipBool sdp_getSdpOriginAsString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit **ppStrOrigin, SipError *pErr)
#else
	( pSdpMsg,ppStrOrigin,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit **ppStrOrigin;
	SipError *pErr;
#endif
{
	SIP_S8bit *pOut;
	SIP_S8bit *pTempOut = SIP_NULL;
	SIPDEBUGFN("Entering function sdp_getSdpOriginAsString");
#ifndef SIP_NO_CHECK	
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (ppStrOrigin == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
	if (pSdpMsg->pOrigin == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pOut = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, SIP_MAX_HDR_SIZE, pErr);
	if (pOut == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pTempOut = pOut;	

	STRCPY ((char *)pOut,"o=");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut,pSdpMsg->pOrigin->pUser);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut,pSdpMsg->pOrigin->pSessionid);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut,pSdpMsg->pOrigin->pVersion);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut,pSdpMsg->pOrigin->pNetType);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut,pSdpMsg->pOrigin->pAddrType);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut,pSdpMsg->pOrigin->dAddr);
	
	*ppStrOrigin = (SIP_S8bit *) STRDUPACCESSOR (pOut);
	if (*ppStrOrigin == SIP_NULL)
	{
		fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
		*pErr = E_NO_MEM;
		return SipFail;
	}

	fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_getSdpOriginAsString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_getSdpTimeAtIndexAsString
**
** DESCRIPTION: This function returns the "t=" line at a given index
**				of the SDP message (taken as input) as a string.
**
** NOTE: The "r=" line(s) associated with the "t=" line asked for (if any), 
**		 are not returned in the string which is filled here.
**
*********************************************************************/
SipBool sdp_getSdpTimeAtIndexAsString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit **ppStrTime, SIP_U32bit dIndex, \
 SipError *pErr)
#else
	( pSdpMsg,ppStrTime,dIndex,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit **ppStrTime;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIP_S8bit *pOut;
	SIP_S8bit *pTempOut = SIP_NULL;
	SdpTime *st;
	SIPDEBUGFN("Entering function sdp_getSdpTimeAtIndexAsString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (ppStrTime == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	pOut = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, SIP_MAX_HDR_SIZE, pErr);
	if (pOut == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pTempOut = pOut;

	if ((sip_listGetAt (&(pSdpMsg->slTime), dIndex, (SIP_Pvoid *)&st, pErr))==SipFail)
	{
		fast_memfree(ACCESSOR_MEM_ID, pOut, SIP_NULL);
		return SipFail;
	}

	STRCPY((char *)pOut,"t=");
	STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, st->pStart);
	STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, " ");
	STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, st->pStop);

	*ppStrTime = (SIP_S8bit *) STRDUPACCESSOR (pOut);
	if (*ppStrTime == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		fast_memfree(ACCESSOR_MEM_ID, pOut, SIP_NULL);
		return SipFail;
	}
	fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_getSdpTimeAtIndexAsString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_getSdpAttrAtIndexAsString
**
** DESCRIPTION: This function returns the SESSION LEVEL "a=" line 
**				of the SDP message (taken as input) at the given
**				index as a string.
**
** NOTE: There is a separate function to get MEDIA LEVEL "a=" line
**		 for the particular index of the "m=" line it corresponds to.
**
*********************************************************************/
SipBool sdp_getSdpAttrAtIndexAsString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit **ppStrAttr, SIP_U32bit dIndex, SipError *pErr)
#else
	( pSdpMsg,ppStrAttr,dIndex,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit **ppStrAttr;
	SIP_S8bit dIndex;
	SipError *pErr;
#endif
{
	SIP_S8bit *pOut;
	SIP_S8bit *pTempOut = SIP_NULL;
	SdpAttr *pSAttr;
	SIPDEBUGFN("Entering function dp_getSdpAttrAtIndexAsString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (ppStrAttr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	pOut = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, SIP_MAX_HDR_SIZE, pErr);
	if (pOut == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pTempOut = pOut;	
	if ((sip_listGetAt (&(pSdpMsg->slAttr), dIndex, \
						 (SIP_Pvoid *)&pSAttr, pErr))==SipFail)
	{
		return SipFail;
	}
	STRCPY(pOut,"a=");
	STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, pSAttr->pName);
	if (pSAttr->pValue != SIP_NULL) /*taking care of SdpAttributes 
									 *like a=recvonly */
	{		
		STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, ":");
		STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, pSAttr->pValue);
	}

	*ppStrAttr = (SIP_S8bit *) STRDUPACCESSOR (pOut);
	if (*ppStrAttr == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
		return SipFail;
	}

	fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function dp_getSdpAttrAtIndexAsString");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sdp_setSdpConnectionFromString
**
** DESCRIPTION: This function sets the SESSION LEVEL "c=" line of the 
**				SDP message (taken as input) from the string provided.
**
*********************************************************************/
SipBool sdp_setSdpConnectionFromString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit *pStrConn, SipError *pErr)
#else
	( pSdpMsg,pStrConn,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit *pStrConn;
	SipError *pErr;
#endif
{
	SdpConnection *pTempConn;
	SIPDEBUGFN("Entering function sdp_setSdpConnectionFromString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStrConn == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/*	GRABLOCK;*/
	if ((sdp_makeSdpConnection(&pTempConn, pStrConn, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/
	if(pSdpMsg->slConnection != SIP_NULL)
		sip_freeSdpConnection(pSdpMsg->slConnection);
	pSdpMsg->slConnection = pTempConn;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_setSdpConnectionFromString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_makeSdpConnection (internal function)
**
** DESCRIPTION: This function is used to parse a given string and fill  
**				in a SdpConnection Structure (taken as input)
**
*********************************************************************/
SipBool sdp_makeSdpConnection
#ifdef ANSI_PROTO
(SdpConnection **ppSdpConn, SIP_S8bit *pStr, SipError *pErr)
#else
	( ppSdpConn,pStr,pErr)
	SdpConnection **ppSdpConn;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SIP_U32bit len;
	SIP_S8bit *pParserBuffer;
	SipSdpParserParam dSdpParserParam;
	SIPDEBUGFN("Entering function sdp_makeSdpConnection");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (ppSdpConn == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pStr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	len = strlen(pStr);
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
		return SipFail;
	strcpy(pParserBuffer, pStr);
	pParserBuffer[len+1]='\0';

	if ((sip_initSdpMessage(&(dSdpParserParam.pSdpMessage), pErr))==SipFail)
	{
		*pErr = E_NO_MEM;	
		return SipFail;
	}	
	dSdpParserParam.pError= (SipError*) fast_memget(ACCESSOR_MEM_ID,  \
		sizeof(SipError), pErr);
	if(dSdpParserParam.pError==SIP_NULL)
	{
		*pErr=E_NO_MEM;
		return SipFail;
	}
	*(dSdpParserParam.pError) = E_NO_ERROR;
	dSdpParserParam.pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),pErr);
    if(dSdpParserParam.pGCList==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if(sip_listInit(dSdpParserParam.pGCList, sip_freeVoid, pErr)==SipFail)
	{
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if (sip_lex_Sdp_scan_buffer(pParserBuffer, len+2) != 0)
	{
		sip_listDeleteAll(dSdpParserParam.pGCList, pErr);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;

	}
	sip_lex_Sdp_reset_state();
	glbSipParserSdpparse((void *)&dSdpParserParam);	/* parse the given string */	
	sip_lex_Sdp_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);

	sip_listDeleteAll(dSdpParserParam.pGCList, pErr);

	if(*(dSdpParserParam.pError) != E_NO_ERROR)
	{
		sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = *(dSdpParserParam.pError);
		return SipFail;
	} 

	*ppSdpConn = dSdpParserParam.pSdpMessage->slConnection;
	HSS_LOCKEDINCREF(dSdpParserParam.pSdpMessage->slConnection->dRefCount);
	
	sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_makeSdpConnection");
	return SipSuccess;		
}

/********************************************************************
**
** FUNCTION:  sdp_makeSdpOrigin (internal function)
**
** DESCRIPTION: This function is used to parse a given string and fill  
**				in a SdpOrigin Structure (taken as input)
**
*********************************************************************/
SipBool sdp_makeSdpOrigin
#ifdef ANSI_PROTO
(SdpOrigin **ppSdpOrigin, SIP_S8bit *pStr, SipError *pErr)
#else
	( ppSdpOrigin, pStr,pErr)
	SdpOrigin **ppSdpOrigin;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SIP_U32bit len;
	SIP_S8bit *pParserBuffer;
	SipSdpParserParam dSdpParserParam;

#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (ppSdpOrigin == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pStr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif


	len = strlen(pStr);
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
		return SipFail;
	strcpy(pParserBuffer, pStr);
	pParserBuffer[len+1]='\0';
	/*GRABLOCK;*/
	dSdpParserParam.pError= (SipError*) fast_memget(ACCESSOR_MEM_ID,  \
		sizeof(SipError), pErr);
	if(dSdpParserParam.pError==SIP_NULL)
	{
		*pErr=E_NO_MEM;
		return SipFail;
	}

	*(dSdpParserParam.pError) = E_NO_ERROR;

	/* calling sip_initSdpMessage so that drefcount is initialized */
	if ((sip_initSdpMessage(&dSdpParserParam.pSdpMessage, pErr))==SipFail)
	{
		*pErr = E_NO_MEM;	
		/*FREELOCK;*/
		return SipFail;
	}
	
	dSdpParserParam.pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),pErr);
    if(dSdpParserParam.pGCList==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	if(sip_listInit(dSdpParserParam.pGCList, sip_freeVoid, pErr)==SipFail)
	{
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if (sip_lex_Sdp_scan_buffer(pParserBuffer, len+2) != 0)
	{
		sip_listDeleteAll(dSdpParserParam.pGCList, pErr);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	sip_lex_Sdp_reset_state();
	glbSipParserSdpparse((void *)&dSdpParserParam);	/* parse the given string */	
	sip_lex_Sdp_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
	sip_listDeleteAll(dSdpParserParam.pGCList, pErr);


	if(*(dSdpParserParam.pError) != E_NO_ERROR)
	{
		sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = *(dSdpParserParam.pError);
		/*FREELOCK;*/
		return SipFail;
	} 

	HSS_LOCKEDINCREF(dSdpParserParam.pSdpMessage->pOrigin->dRefCount);
	*ppSdpOrigin = dSdpParserParam.pSdpMessage->pOrigin;
	
	sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_setSdpOriginFromString");
	/*FREELOCK;*/
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sdp_setSdpOriginFromString
**
** DESCRIPTION: This function sets the "o=" line of the 
**				SDP message (taken as input) from the string provided.
**
*********************************************************************/
SipBool sdp_setSdpOriginFromString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit *pStr, SipError *pErr)
#else
	( pSdpMsg,pStr,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SdpOrigin *pTempOrigin;

	SIPDEBUGFN("Entering function sdp_setSdpOriginFromString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	/*GRABLOCK;*/
	if ((sdp_makeSdpOrigin(&pTempOrigin, pStr, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/

	if(pSdpMsg->pOrigin != SIP_NULL)
		sip_freeSdpOrigin(pSdpMsg->pOrigin);
	pSdpMsg->pOrigin = pTempOrigin;
	
	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/********************************************************************
**
** FUNCTION:  sdp_setSdpTimeAtIndexFromString
**
** DESCRIPTION: This function sets the "t=" line at a given index in the
**				SDP message (taken as input) from the string provided.
**
*********************************************************************/
SipBool sdp_setSdpTimeAtIndexFromString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit *pStr, SIP_U32bit dIndex, SipError *pErr)
#else
	( pSdpMsg,pStr,dIndex,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit *pStr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SdpTime *pTempTime;
	SIPDEBUGFN("Entering function sdp_setSdpTimeAtIndexFromString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/*GRABLOCK;*/
	if ((sdp_makeSdpTime(&pTempTime, pStr, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/
	
	if ((sip_listSetAt(&(pSdpMsg->slTime), dIndex, \
						(void *)pTempTime, pErr))==SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_setSdpTimeAtIndexFromString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_insertSdpTimeAtIndexFromString
**
** DESCRIPTION: This function inserts the "t=" line at a given index in 
**				the	SDP message (taken as input) from the string provided.
**
*********************************************************************/
SipBool sdp_insertSdpTimeAtIndexFromString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit *pStr, SIP_U32bit dIndex, SipError *pErr)
#else
	( pSdpMsg,pStr,dIndex,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit *pStr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SdpTime *pTempTime;
	SIPDEBUGFN("Entering function sdp_insertSdpTimeAtIndexFromString");
#ifndef SIP_NO_CHECK 
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/*GRABLOCK;*/
	if ((sdp_makeSdpTime(&pTempTime, pStr, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/
	if ((sip_listInsertAt(&(pSdpMsg->slTime), dIndex, \
						(void *)pTempTime, pErr))==SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_insertSdpTimeAtIndexFromString");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sdp_makeSdpTime (internal function)
**
** DESCRIPTION: This function is used to parse a given string and fill  
**				in a SdpTime Structure (taken as input)
**
*********************************************************************/
SipBool sdp_makeSdpTime
#ifdef ANSI_PROTO
(SdpTime **ppSdpTime, SIP_S8bit *pStr, SipError *pErr)
#else
	( ppSdpTime,pStr,pErr)
	SdpTime **ppSdpTime;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SdpTime *pTemp;
	SIP_U32bit len;
	SIP_S8bit *pParserBuffer;
	SipSdpParserParam dSdpParserParam;


	SIPDEBUGFN("Entering function sdp_makeSdpTime");
#ifndef SIP_NO_CHECK	
	if (pErr == SIP_NULL)
		return SipFail;
	if (ppSdpTime == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif

	len = strlen(pStr);
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
		return SipFail;
	strcpy(pParserBuffer, pStr);
	pParserBuffer[len+1]='\0';
	dSdpParserParam.pError= (SipError*) fast_memget(ACCESSOR_MEM_ID,  \
		sizeof(SipError), pErr);
	if(dSdpParserParam.pError==SIP_NULL)
	{
		*pErr=E_NO_MEM;
		return SipFail;
	}

	*(dSdpParserParam.pError) = E_NO_ERROR;
	/* calling sip_initSdpMessage so that drefcount is initialized */
	if ((sip_initSdpMessage(&dSdpParserParam.pSdpMessage, pErr))==SipFail)
	{
		*pErr = E_NO_MEM;	
		return SipFail;
	}
	dSdpParserParam.pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),pErr);
    if(dSdpParserParam.pGCList==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if(sip_listInit(dSdpParserParam.pGCList, sip_freeVoid, pErr)==SipFail)
	{
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if (sip_lex_Sdp_scan_buffer(pParserBuffer, len+2) != 0)
	{
		sip_listDeleteAll(dSdpParserParam.pGCList, pErr);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	sip_lex_Sdp_reset_state();
	glbSipParserSdpparse((void *)&dSdpParserParam);	/* parse the given string */	
	sip_lex_Sdp_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
	sip_listDeleteAll(dSdpParserParam.pGCList, pErr);


	if(*(dSdpParserParam.pError) != E_NO_ERROR)
	{
		sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = *(dSdpParserParam.pError);
		return SipFail;
	} 

	if ((sip_listGetAt(&(dSdpParserParam.pSdpMessage->slTime), (SIP_U32bit)0, \
					(SIP_Pvoid *)&pTemp, pErr)) == SipFail)
	{
		sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
		*pErr = *(dSdpParserParam.pError);
		return SipFail;
	}

	*ppSdpTime = pTemp;
	HSS_LOCKEDINCREF((*ppSdpTime)->dRefCount);

	sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_makeSdpTime");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_setSdpAttrAtIndexFromString
**
** DESCRIPTION: This function sets the SESSION LEVEL "a=" line 
**				of the SDP message (taken as input) at the given
**				index from the string.
**
** NOTE: There is a separate function to set MEDIA LEVEL "a=" line
**		 for the particular index of the "m=" line it corresponds to.
**
*********************************************************************/
SipBool sdp_setSdpAttrAtIndexFromString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit *pStrAttr, SIP_S8bit dIndex, SipError *pErr)
#else
	( pSdpMsg,pStrAttr,dIndex,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit *pStrAttr;
	SIP_S8bit dIndex;
	SipError *pErr;
#endif
{
	SdpAttr *pTempAttr;
	SIPDEBUGFN("Entering function sdp_setSdpAttrAtIndexFromString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStrAttr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif
	/*GRABLOCK;*/
	if ((sdp_makeSdpAttr(&pTempAttr, pStrAttr, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/
	
	if ((sip_listSetAt(&(pSdpMsg->slAttr), dIndex, \
						(void *)pTempAttr, pErr))==SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_setSdpAttrAtIndexFromString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_insertSdpAttrAtIndexFromString
**
** DESCRIPTION: This function inserts the SESSION LEVEL "a=" line 
**				of the SDP message (taken as input) at the given
**				index from the string.
**
** NOTE: There is a separate function to insert MEDIA LEVEL "a=" line
**		 for the particular index of the "m=" line it corresponds to.
**
*********************************************************************/
SipBool sdp_insertSdpAttrAtIndexFromString
#ifdef ANSI_PROTO
(SdpMessage *pSdpMsg, SIP_S8bit *pStrAttr, SIP_S8bit dIndex, SipError *pErr)
#else
	( pSdpMsg,pStrAttr,dIndex,pErr)
	SdpMessage *pSdpMsg;
	SIP_S8bit *pStrAttr;
	SIP_S8bit dIndex;
	SipError *pErr;
#endif
{
	SdpAttr *pTempAttr;
	SIPDEBUGFN("Entering function sdp_insertSdpAttrAtIndexFromString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMsg == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStrAttr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif
/*
	if (dIndex == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
*/
	/*GRABLOCK;*/
	if ((sdp_makeSdpAttr(&pTempAttr, pStrAttr, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/
	
	if ((sip_listInsertAt(&(pSdpMsg->slAttr), dIndex, \
						(void *)pTempAttr, pErr))==SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_insertSdpAttrAtIndexFromString");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sdp_makeSdpAttr (internal function)
**
** DESCRIPTION: This function is used to parse a given string and fill
**				in a SdpAttr Structure (taken as input)
**
*********************************************************************/
SipBool sdp_makeSdpAttr
#ifdef ANSI_PROTO
(SdpAttr **ppSdpAttr, SIP_S8bit *pStr, SipError *pErr)
#else
	( ppSdpAttr,pStr,pErr)
	SdpAttr **ppSdpAttr;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SdpAttr *pTemp;
	SIP_U32bit len;
	SIP_S8bit *pParserBuffer;
	SipSdpParserParam dSdpParserParam;

	SIPDEBUGFN ("Entering function sdp_makeSdpAttr");
#ifndef SIP_NO_CHECK	
	if (pErr == SIP_NULL)
		return SipFail;
	if (ppSdpAttr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif

	len = strlen(pStr);
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
		return SipFail;
	strcpy(pParserBuffer, pStr);
	pParserBuffer[len+1]='\0';

	/* calling sip_initSdpMessage so that drefcount is initialized */
	if ((sip_initSdpMessage(&dSdpParserParam.pSdpMessage, pErr))==SipFail)
	{
		*pErr = E_NO_MEM;	
		return SipFail;
	}
	dSdpParserParam.pError= (SipError*) fast_memget(ACCESSOR_MEM_ID,  \
		sizeof(SipError), pErr);
	if(dSdpParserParam.pError==SIP_NULL)
	{
		*pErr=E_NO_MEM;
		return SipFail;
	}
	*(dSdpParserParam.pError) = E_NO_ERROR;
	dSdpParserParam.pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),pErr);
    if(dSdpParserParam.pGCList==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if(sip_listInit(dSdpParserParam.pGCList, sip_freeVoid, pErr)==SipFail)
	{
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if (sip_lex_Attrib_scan_buffer(pParserBuffer, len+2) != 0)
	{
		sip_listDeleteAll(dSdpParserParam.pGCList, pErr);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	glbSipParserAttribparse((void *)&dSdpParserParam);	/* parse the given string */	
	sip_lex_Attrib_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
	sip_listDeleteAll(dSdpParserParam.pGCList, pErr);


	if(*(dSdpParserParam.pError) != E_NO_ERROR)
	{
		sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = *(dSdpParserParam.pError);
		return SipFail;
	} 
	if ((sip_listGetAt(&(dSdpParserParam.pSdpMessage->slAttr), (SIP_U32bit)0, \
					(SIP_Pvoid *)&pTemp, pErr)) == SipFail)
	{
		sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
		*pErr = *(dSdpParserParam.pError);
		return SipFail;
	}

	*ppSdpAttr = pTemp;
	HSS_LOCKEDINCREF((*ppSdpAttr)->dRefCount);

	sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
	*pErr = E_NO_ERROR;
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pError,SIP_NULL);
	fast_memfree(ACCESSOR_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
	SIPDEBUGFN ("Exiting function sdp_makeSdpAttr");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sdp_setConnectionFromStringAtIndexInMedia
**
** DESCRIPTION: This function 'sets' the MEDIA LEVEL "c=" line  
**				(at index: dIndex) from the string provided.
**
*********************************************************************/
SipBool sdp_setConnectionFromStringAtIndexInMedia
#ifdef ANSI_PROTO
(SdpMedia *pSdpMedia, SIP_S8bit *pStrConn, SIP_U32bit dIndex, SipError *pErr)
#else
	( pSdpMedia,pStrConn,dIndex,pErr)
	SdpMedia *pSdpMedia;
	SIP_S8bit *pStrConn;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SdpConnection *pTempConn;
	SIPDEBUGFN ("Entering function sdp_setConnectionFromStringAtIndexInMedia");	
#ifndef SIP_NO_CHECK 	
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMedia == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStrConn == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif	
/*
	if (dIndex == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}		
*/
	/*GRABLOCK;*/
	if ((sdp_makeSdpConnection(&pTempConn, pStrConn, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/

	if ((sip_listSetAt(&(pSdpMedia->slConnection), dIndex, \
						(void *)pTempConn, pErr))==SipFail)
		return SipFail;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_setConnectionFromStringAtIndexInMedia");
	return SipSuccess;	
}	


/********************************************************************
**
** FUNCTION:  sdp_insertConnectionFromStringAtIndexInMedia
**
** DESCRIPTION: This function 'inserts' the MEDIA LEVEL "c=" line  
**				(at index: dIndex) from the string provided.
**
*********************************************************************/
SipBool sdp_insertConnectionFromStringAtIndexInMedia
#ifdef ANSI_PROTO
(SdpMedia *pSdpMedia, SIP_S8bit *pStrConn, SIP_U32bit dIndex, SipError *pErr)
#else
	( pSdpMedia,pStrConn,dIndex,pErr)
	SdpMedia *pSdpMedia;
	SIP_S8bit *pStrConn;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SdpConnection *pTempConn;
	SIPDEBUGFN ("Entering function sdp_insertConnectionFromStringAtIndexInMedia");	
#ifndef SIP_NO_CHECK	
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMedia == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStrConn == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif		
/*
	if (dIndex == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}		
*/
	/*GRABLOCK;*/
	if ((sdp_makeSdpConnection(&pTempConn, pStrConn, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/

	if ((sip_listInsertAt(&(pSdpMedia->slConnection), dIndex, \
						(void *)pTempConn, pErr))==SipFail)
		return SipFail;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_insertConnectionFromStringAtIndexInMedia");
	return SipSuccess;	
}


/********************************************************************
**
** FUNCTION:  sdp_getConnectionAtIndexInMediaAsString
**
** DESCRIPTION: This function gets the MEDIA LEVEL "c=" line (at index
**				equal to dIndex) as a string
**
*********************************************************************/
SipBool sdp_getConnectionAtIndexInMediaAsString
#ifdef ANSI_PROTO
(SdpMedia *pSdpMedia, SIP_S8bit **ppStrConn, SIP_U32bit dIndex, SipError *pErr)
#else
	( pSdpMedia,ppStrConn,dIndex,pErr)
	SdpMedia *pSdpMedia;
	SIP_S8bit **ppStrConn;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIP_S8bit *pOut;
	SIP_S8bit *pTempOut = SIP_NULL;
	SdpConnection *pConnection;
	SIPDEBUGFN ("Entering function sdp_getConnectionAtIndexInMediaAsString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMedia == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (ppStrConn == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif	
/*
	if (dIndex == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
*/
	if ((sip_listGetAt (&(pSdpMedia->slConnection), dIndex, \
						 (SIP_Pvoid *)&pConnection, pErr))==SipFail)
	{
		return SipFail;
	}
	pOut = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, SIP_MAX_HDR_SIZE, pErr);
	if (pOut == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}	
	pTempOut = pOut;	

	STRCPY ((char *)pOut, "c=");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut, pConnection->pNetType);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut, pConnection->pAddrType);
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut," ");
	STRCAT ((pTempOut + SIP_MAX_HDR_SIZE),pOut, pConnection->dAddr);
	
	*ppStrConn = (SIP_S8bit *) STRDUPACCESSOR (pOut);
	if (*ppStrConn == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
		return SipFail;
	}	

	fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_getConnectionAtIndexInMediaAsString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_getAttrAtIndexInMediaAsString
**
** DESCRIPTION: This function gets the MEDIA LEVEL "a=" line (at index
**				equal to dIndex) as a string
**
*********************************************************************/
SipBool sdp_getAttrAtIndexInMediaAsString
#ifdef ANSI_PROTO
(SdpMedia *pSdpMedia, SIP_S8bit **ppStrAttr, SIP_U32bit dIndex, SipError *pErr)
#else
	( pSdpMedia,ppStrAttr,dIndex,pErr)
	SdpMedia *pSdpMedia;
	SIP_S8bit **ppStrAttr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIP_S8bit *pOut;
	SIP_S8bit *pTempOut = SIP_NULL;
	SdpAttr *pSAttr;
	SIPDEBUGFN ("Entering function sdp_getAttrAtIndexInMediaAsString");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMedia == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (ppStrAttr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif
/*
	if (dIndex == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
*/
	pOut = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, SIP_MAX_HDR_SIZE, pErr);
	if (pOut == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pTempOut = pOut;	
	if ((sip_listGetAt (&(pSdpMedia->slAttr), dIndex, \
						 (SIP_Pvoid *)&pSAttr, pErr))==SipFail)
	{
		return SipFail;
	}
	STRCPY(pOut,"a=");
	STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, pSAttr->pName);
	if (pSAttr ->pValue != SIP_NULL)/*taking care of SdpAttributes 
									 *like a=recvonly */
	{		
		STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, ":");
		STRCAT((pTempOut + SIP_MAX_HDR_SIZE),pOut, pSAttr->pValue);
	}

	*ppStrAttr = (SIP_S8bit *) STRDUPACCESSOR (pOut);
	if (*ppStrAttr == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
		return SipFail;
	}

	fast_memfree(ACCESSOR_MEM_ID, pOut, pErr);
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_getAttrAtIndexInMediaAsString");
	return SipSuccess;
}	


/********************************************************************
**
** FUNCTION:  sdp_setAttrFromStringAtIndexInMedia
**
** DESCRIPTION: This function 'sets' the MEDIA LEVEL "a=" line  
**				(at index: dIndex) from the string provided.
**
*********************************************************************/
SipBool sdp_setAttrFromStringAtIndexInMedia
#ifdef ANSI_PROTO
(SdpMedia *pSdpMedia, SIP_S8bit *pStrAttr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pSdpMedia,pStrAttr,dIndex,pErr)
	SdpMedia *pSdpMedia;
	SIP_S8bit *pStrAttr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SdpAttr *pTempAttr;
	SIPDEBUGFN ("Entering function sdp_setAttrFromStringAtIndexInMedia");	
#ifndef SIP_NO_CHECK	
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMedia == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStrAttr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}	
#endif	
	
	/*GRABLOCK;*/
	if ((sdp_makeSdpAttr(&pTempAttr, pStrAttr, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/

	if ((sip_listSetAt(&(pSdpMedia->slAttr), dIndex, \
						(void *)pTempAttr, pErr))==SipFail)
		return SipFail;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_setAttrFromStringAtIndexInMedia");
	return SipSuccess;	
}


/********************************************************************
**
** FUNCTION:  sdp_insertAttrFromStringAtIndexInMedia
**
** DESCRIPTION: This function 'inserts' the MEDIA LEVEL "a=" line  
**				(at index: dIndex) from the string provided.
**
*********************************************************************/
SipBool sdp_insertAttrFromStringAtIndexInMedia
#ifdef ANSI_PROTO
(SdpMedia *pSdpMedia, SIP_S8bit *pStrAttr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pSdpMedia,pStrAttr,dIndex,pErr)
	SdpMedia *pSdpMedia;
	SIP_S8bit *pStrAttr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SdpAttr *pTempAttr;
	SIPDEBUGFN ("Entering function sdp_insertAttrFromStringAtIndexInMedia");	
#ifndef SIP_NO_CHECK	
	if (pErr == SIP_NULL)
		return SipFail;
	if (pSdpMedia == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStrAttr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif		

	/*GRABLOCK;*/
	if ((sdp_makeSdpAttr(&pTempAttr, pStrAttr, pErr))==SipFail)
	{
		/*FREELOCK;*/
		return SipFail;
	}
	/*FREELOCK;*/
	if ((sip_listInsertAt(&(pSdpMedia->slAttr), dIndex, \
						(void *)pTempAttr, pErr))==SipFail)
		return SipFail;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sdp_insertAttrFromStringAtIndexInMedia");
	return SipSuccess;	
}

