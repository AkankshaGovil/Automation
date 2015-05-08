/******************************************************************************
 ** FUNCTION:
 **	 	This file has all the SIP Caller & Callee preferences Related 
**              APIS for freing structures

 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccpfree.h
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
#ifndef _CCPFREE_H_
#define _CCPFREE_H_

#include "sipfree.h"
#include "ccpstruct.h"
#include "portlayer.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#ifdef SIP_CCP_VERSION10
	extern void sip_ccp_freeSipRequestDispositionHeader _ARGS_ ((SipRequestDispositionHeader *pHdr));

extern void sip_ccp_freeSipRejectContactHeader _ARGS_ ((SipRejectContactHeader *pHdr));

/* extern void sip_ccp_freeSipExtParam _ARGS_ ((SipExtParam * pExtParam)); */

extern void sip_ccp_freeSipAcceptContactParam _ARGS_ ((SipAcceptContactParam * pAcceptConatctParam));

extern void sip_ccp_freeSipAcceptContactHeader _ARGS_ ((SipAcceptContactHeader *pHeader));

extern void sip_ccp_freeSipRejectContactParam _ARGS_ ((SipRejectContactParam * pRejectContactParam));

extern void __sip_ccp_freeSipRequestDispositionHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_ccp_freeSipRejectContactHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_ccp_freeSipAcceptContactHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_ccp_freeSipRejectContactParam _ARGS_ ((SIP_Pvoid pRejectContactParam));

extern void __sip_ccp_freeSipAcceptContactParam _ARGS_ ((SIP_Pvoid pAcceptContactParam));

extern void __sip_ccp_freeSipExtParam _ARGS_ ((SIP_Pvoid pExtParam));

#else
extern void sip_ccp_freeSipRequestDispositionHeader _ARGS_ ((SipRequestDispositionHeader *pHdr));

extern void sip_ccp_freeSipRejectContactHeader _ARGS_ ((SipRejectContactHeader *pHdr));

/* extern void sip_ccp_freeSipExtParam _ARGS_ ((SipExtParam * pExtParam)); */

extern void sip_ccp_freeSipAcceptContactParam _ARGS_ ((SipAcceptContactParam * pAcceptConatctParam));

extern void sip_ccp_freeSipAcceptContactHeader _ARGS_ ((SipAcceptContactHeader *pHeader));

extern void sip_ccp_freeSipRejectContactParam _ARGS_ ((SipRejectContactParam * pRejectContactParam));

extern void __sip_ccp_freeSipRequestDispositionHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_ccp_freeSipRejectContactHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_ccp_freeSipAcceptContactHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_ccp_freeSipRejectContactParam _ARGS_ ((SIP_Pvoid pRejectContactParam));

extern void __sip_ccp_freeSipAcceptContactParam _ARGS_ ((SIP_Pvoid pAcceptContactParam));

extern void __sip_ccp_freeSipExtParam _ARGS_ ((SIP_Pvoid pExtParam));

#endif /*ifdef SIP_CCP_VERSION10 */



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
