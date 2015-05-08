/*******************************************************************************
 **** FUNCTION:
 ****             Has Init Functions For all bcpt Structures

 ******************************************************************************
 ****
 **** FILENAME:
 **** sipbcptinit.c
 ****
 **** DESCRIPTION:
 **** This file contains dCodeNum to init all structures
 ****
 **** DATE        NAME                    REFERENCE               REASON
 **** ----        ----                    ---------              --------
 **** 9/02/00   B. Borthakur       		                    Initial Creation
 ****
 ****
 ****     Copyright 1999, Hughes Software Systems, Ltd.
 *****************************************************************************/


#include <stdlib.h>
#include "sipstruct.h"
#include "sipfree.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipbcptfree.h"
#include "sipbcptinit.h"

/***********************************************************************************
**** FUNCTION:sip_bcpt_initIsupMessage
****
****
**** DESCRIPTION:
***********************************************************************************/

SipBool sip_bcpt_initIsupMessage
#ifdef ANSI_PROTO
	(IsupMessage **m,SipError *err)
#else
	(m,err)
	IsupMessage **m;
	SipError *err;	
#endif
{
	*m= ( IsupMessage * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(IsupMessage), err);
	if(*m==SIP_NULL)
		return SipFail;
	INIT((*m)->pBody);
	(*m)->dLength = 0;
	HSS_INITREF((*m)->dRefCount);
	return SipSuccess;
}

/***********************************************************************************
**** FUNCTION:sip_bcpt_initQsigMessage
****
****
**** DESCRIPTION:
***********************************************************************************/

SipBool sip_bcpt_initQsigMessage
#ifdef ANSI_PROTO
	(QsigMessage **ppM,SipError *pErr)
#else
	(ppM,pErr)
	QsigMessage **ppM;
	SipError *pErr;	
#endif
{
	*ppM= ( QsigMessage * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(QsigMessage), pErr);
	if(*ppM==SIP_NULL)
		return SipFail;
	INIT((*ppM)->pBody);
	(*ppM)->dLength = 0;
	HSS_INITREF((*ppM)->dRefCount);
	return SipSuccess;
}

/***********************************************************************************
**** FUNCTION:sip_bcpt_initSipMimeVersionHeader
****
****
**** DESCRIPTION:
***********************************************************************************/

SipBool sip_bcpt_initSipMimeVersionHeader
#ifdef ANSI_PROTO
	(SipMimeVersionHeader **h,SipError *err)
#else
	(h,err)
	SipMimeVersionHeader **h;
	SipError *err;	
#endif
{
	*h= ( SipMimeVersionHeader * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipMimeVersionHeader), err);
	if(*h==SIP_NULL)
		return SipFail;
	INIT((*h)->pVersion);
	HSS_INITREF((*h)->dRefCount);
	return SipSuccess;
}
/***********************************************************************************
**** FUNCTION:sip_bcpt_initMimeMessage
****
****
**** DESCRIPTION:
***********************************************************************************/

SipBool sip_bcpt_initMimeMessage
#ifdef ANSI_PROTO
	(MimeMessage **m,SipError *err)
#else
	(m,err)
	MimeMessage **m;
	SipError *err;	
#endif
{
	*m= ( MimeMessage * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(MimeMessage), err);
	if(*m==SIP_NULL)
		return SipFail;
	if( sip_listInit(&((*m)->slRecmimeBody), __sip_freeSipMsgBody,err) == SipFail)
		return SipFail;
	HSS_INITREF((*m)->dRefCount);
	return SipSuccess;
}
/***********************************************************************************
**** FUNCTION:sip_bcpt_initSipMimeHeader
****
****
**** DESCRIPTION:
***********************************************************************************/
SipBool sip_bcpt_initSipMimeHeader
#ifdef ANSI_PROTO
	(SipMimeHeader **h,SipError *err)
#else
	(h,err)
	SipMimeHeader **h;
	SipError *err;	
#endif
{
	*h= ( SipMimeHeader * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipMimeHeader), err);
	if(*h==SIP_NULL)
		return SipFail;
	INIT((*h)->pContentType);
	INIT((*h)->pContentDisposition);
	INIT((*h)->pContentTransEncoding);
	INIT((*h)->pContentId);
	INIT((*h)->pContentDescription);
	if( sip_listInit(&((*h)->slAdditionalMimeHeaders), __sip_freeString,err) == SipFail)
		return SipFail;
	HSS_INITREF((*h)->dRefCount);
	return SipSuccess;
}

