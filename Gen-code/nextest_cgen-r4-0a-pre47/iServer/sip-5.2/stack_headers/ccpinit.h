/******************************************************************************
 ** FUNCTION:
 **	 	This file has all the SIP Caller & Callee preferences Related 
**              APIs for initilaization

 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccpinit.h
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

#ifndef _CCPINIT_H_
#define _CCPINIT_H_

#include <stdlib.h>
#include "sipstruct.h"
#include "sipcommon.h"
#include "ccpstruct.h"
#include "ccpfree.h"
#include "ccpinit.h"
#include "sipinit.h"
#include "portlayer.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#ifdef SIP_CCP_VERSION10
	extern SipBool sip_ccp_initSipRequestDispositionHeader _ARGS_((SipRequestDispositionHeader **ppHdr, SipError *pErr));

extern SipBool sip_ccp_initSipRejectContactHeader _ARGS_ ((SipRejectContactHeader **ppHdr,SipError *pErr));

/* extern SipBool sip_ccp_initSipExtParam _ARGS_ ((SipExtParam ** ppExtParam,SipError *pErr)); */

extern SipBool sip_ccp_initSipAcceptContactParam _ARGS_ ((SipAcceptContactParam ** ppAcceptContactParam, en_AcceptContactType type, SipError * pErr));

extern SipBool sip_ccp_initSipAcceptContactHeader _ARGS_ ((SipAcceptContactHeader **ppHeader, SipError *pErr));

extern SipBool sip_ccp_initSipRejectContactParam _ARGS_ ((SipRejectContactParam ** ppRejectContactParam, en_RejectContactType type, SipError * pErr));

#else
extern SipBool sip_ccp_initSipRequestDispositionHeader _ARGS_((SipRequestDispositionHeader **ppHdr, SipError *pErr));

extern SipBool sip_ccp_initSipRejectContactHeader _ARGS_ ((SipRejectContactHeader **ppHdr,SipError *pErr));

/* extern SipBool sip_ccp_initSipExtParam _ARGS_ ((SipExtParam ** ppExtParam,SipError *pErr)); */

extern SipBool sip_ccp_initSipAcceptContactParam _ARGS_ ((SipAcceptContactParam ** ppAcceptContactParam, en_AcceptContactType type, SipError * pErr));

extern SipBool sip_ccp_initSipAcceptContactHeader _ARGS_ ((SipAcceptContactHeader **ppHeader, SipError *pErr));

extern SipBool sip_ccp_initSipRejectContactParam _ARGS_ ((SipRejectContactParam ** ppRejectContactParam, en_RejectContactType type, SipError * pErr));

#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
