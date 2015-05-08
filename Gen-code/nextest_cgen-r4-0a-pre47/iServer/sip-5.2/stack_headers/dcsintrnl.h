/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes off all DCS SIP Structure  
**      duplicating/cloning APIs.
**
*******************************************************************************
**
** FILENAME:
** 	dcsintrnl.h
**
** DESCRIPTION:
**  THIS FILE IS USED INTERNALLY BY THE STACK
**	IT SHOULD NOT BE INCLUDED DIRECTLY BY THE USER
**
** DATE    	 NAME           REFERENCE      REASON
** ----    	 ----           ---------      ------
** 16Nov00	S. Luthra			Creation
**					
** Copyrights 2000, Hughes Software Systems, Ltd.
*******************************************************************************/


#ifndef __DCSINTRNL_H__
#define __DCSINTRNL_H__

#include "sipstruct.h"
#include "sipinternal.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


SipBool sip_dcs_cloneDcsHeaders _ARGS_ ((SipHeader *pDest, SipHeader *pSource, SipError *pErr));

SipBool sip_dcs_deleteAllDcsGeneralHeaders _ARGS_ ((SipGeneralHeader *pDest, SipGeneralHeader *pSource, SipError *pErr));

SipBool sip_dcs_cloneAllDcsGeneralHeaders _ARGS_ ((SipGeneralHeader *pDest, SipGeneralHeader *pSource, SipError *pErr));

SipBool sip_dcs_deleteAllDcsRequestHeaders _ARGS_ ((SipReqHeader *pDest, SipReqHeader *pSource, SipError *pErr));

SipBool sip_dcs_cloneAllDcsRequestHeaders _ARGS_ ((SipReqHeader *pDest, SipReqHeader *pSource, SipError *pErr));

SipBool sip_dcs_deleteAllDcsResponseHeaders _ARGS_ ((SipRespHeader *pDest, SipRespHeader *pSource, SipError *pErr));

SipBool sip_dcs_cloneAllDcsResponseHeaders _ARGS_ ((SipRespHeader *pDest, SipRespHeader *pSource, SipError *pErr));

en_SipBoolean sip_dcs_isSipDcsGeneralHeader _ARGS_ ((en_HeaderType dType));

en_SipBoolean sip_dcs_isSipDcsRequestHeader _ARGS_ ((en_HeaderType dType));

en_SipBoolean sip_dcs_isSipDcsResponseHeader _ARGS_ ((en_HeaderType dType));

SipBool sip_dcs_deleteAllDcsGeneralHeaderByType _ARGS_ ((SipGeneralHeader *pHdr, en_HeaderType dType, SipError *pErr));

SipBool sip_dcs_deleteAllDcsRequestHeaderByType _ARGS_ ((SipReqHeader *pHdr, en_HeaderType dType, SipError *pErr));

SipBool sip_dcs_deleteAllDcsResponseHeaderByType _ARGS_ ((SipRespHeader *pHdr, en_HeaderType dType, SipError *pErr));

SipBool sip_dcs_getDcsGeneralHeaderCount _ARGS_ ((SipGeneralHeader *pHdr, en_HeaderType dType, SIP_U32bit *pCount, SipError *pErr));

SipBool sip_dcs_getDcsRequestHeaderCount _ARGS_ ((SipReqHeader *pHdr, en_HeaderType dType, SIP_U32bit *pCount, SipError *pErr));

SipBool sip_dcs_getDcsResponseHeaderCount _ARGS_ ((SipRespHeader *pHdr, en_HeaderType dType, SIP_U32bit *pCount, SipError *pErr));

SipBool sip_dcs_deleteDcsGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_deleteDcsRequestHeaderAtIndex _ARGS_ ((SipReqHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_deleteDcsResponseHeaderAtIndex _ARGS_ ((SipRespHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_getDcsGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *pGeneralHdr, SipHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_getDcsRequestHeaderAtIndex _ARGS_ ((SipReqHeader *pRequestHdr,  SipHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_getDcsResponseHeaderAtIndex _ARGS_ ((SipRespHeader *pResponseHdr,  SipHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_setDcsGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *pGeneralHdr, SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_setDcsRequestHeaderAtIndex _ARGS_ ((SipReqHeader *pRequestHdr, SipHeader *pHdr,   SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_setDcsResponseHeaderAtIndex _ARGS_ ((SipRespHeader *pResponseHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_insertDcsGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *pGeneralHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_insertDcsRequestHeaderAtIndex _ARGS_ ((SipReqHeader *pRequestHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

SipBool sip_dcs_insertDcsResponseHeaderAtIndex _ARGS_ ((SipRespHeader *pResponseHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

en_SipBoolean sip_dcs_validateDcsHeaderType _ARGS_ ((en_HeaderType dtype));

SipBool sip_dcs_validateDcsHeaderString _ARGS_ ((SIP_S8bit *pStr, en_HeaderType dType, SipError *pErr));



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
