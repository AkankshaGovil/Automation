
/* THIS FILE IS INCLUDED INTERNALLY BY THE STACK
** THE USER SHHOULD NOT INCLUDE THIS DIRECTLY
*/

/******************************************************************************
 ** FUNCTION:
 **	 	This file has all internal function definnitions of SIP Caller 
 **             and Callee preferences draft
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccpinternal.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE	NAME		REFERENCE	REASON
 ** ----	----		--------	------
 ** 8/2/2000	S.Luthra	Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 ******************************************************************************/
  
#ifndef _CCP_INTERNAL_H_
#define _CCP_INTERNAL_H_

#include "sipstruct.h"
#include "ccpstruct.h"
#include "ccpfree.h"
#include "ccpinit.h"
#include "portlayer.h"
#include "sipdecode.h"
#include "sipinternal.h"
#include "sipdecodeintrnl.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern SipError glbSipParserErrorValue; 
extern SipMessage * glbSipParserSipMessage;
extern SipList glbSipParserGC;

SipBool __sip_ccp_cloneSipRequestDispositionHeader _ARGS_ ((\
		SipRequestDispositionHeader *pDest, SipRequestDispositionHeader *pSource, \
		SipError *pErr));

SipBool __sip_ccp_cloneSipAcceptContactHeader _ARGS_ ((SipAcceptContactHeader *pDest, \
		SipAcceptContactHeader *pSource, SipError *pErr ));

SipBool __sip_ccp_cloneSipRejectContactHeader _ARGS_ ((SipRejectContactHeader *pDest, \
		SipRejectContactHeader *pSource, SipError *pErr));

#define sip_ccp_cloneSipAcceptContactParam __sip_ccp_cloneSipAcceptContactParam
#define  sip_ccp_cloneSipRejectContactParam  __sip_ccp_cloneSipRejectContactParam 


extern SipBool __sip_ccp_cloneSipAcceptContactParam _ARGS_ ((SipAcceptContactParam *pDest, \
		SipAcceptContactParam *pSource, SipError *pErr));

extern SipBool __sip_ccp_cloneSipRejectContactParam _ARGS_ ((SipRejectContactParam *pdest, \
		SipRejectContactParam *pSource, SipError *pErr));

SipBool sip_ccp_validateSipAcceptContactType _ARGS_ ((en_AcceptContactType type, SipError *pErr));

SipBool sip_ccp_validateSipRejectContactType _ARGS_ ((en_RejectContactType type, SipError *pErr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
