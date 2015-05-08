/*
**  THIS FILE IS USED INTERNALLY BY THE STACK
**	IT SHOULD NOT BE INCLUDED DIRECTLY BY THE USER
*/

/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes off all SIP Structure  
**      duplicating/cloning APIs.
**
*******************************************************************************
**
** FILENAME:
** 	sipd.h
**
** DESCRIPTION:
**  THIS FILE IS USED INTERNALLY BY THE STACK
**	IT SHOULD NOT BE INCLUDED DIRECTLY BY THE USER
**
** DATE    			  NAME           REFERENCE      REASON
** ----    			  ----           ---------      ------
** 13 Dec 99		S. Luthra
**					B. Borthakur
**					R. Preethy
**
** Copyrights 1999, Hughes Software Systems, Ltd.
*******************************************************************************/

#ifndef __SIP_INTERNAL_H_
#define __SIP_INTERNAL_H_

#include "sipstruct.h"
#include "sipcommon.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


typedef enum
{
	SipFalse=0,
	SipTrue=1
} en_SipBoolean;

 en_SipBoolean isSipGeneralHeader _ARGS_ ((en_HeaderType type));
 en_SipBoolean isSipReqHeader _ARGS_ ((en_HeaderType type));
 en_SipBoolean isSipRespHeader _ARGS_ ((en_HeaderType type));
 SipBool getGeneralHdrCount _ARGS_ ((SipGeneralHeader *hdr, en_HeaderType type, SIP_U32bit *count, SipError *err));
 SipBool getRequestHdrCount _ARGS_ ((SipReqHeader *hdr, en_HeaderType type, SIP_U32bit *count, SipError *err));
 SipBool getResponseHdrCount _ARGS_ ((SipRespHeader *hdr, en_HeaderType type, SIP_U32bit *count, SipError *err));
#ifdef SIP_BY_REFERENCE
 SipBool getGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *gen_hdr, en_HeaderType type, 
 	SipHeader *hdr, SIP_U32bit index, SipError *err));
#else
 SipBool getGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *gen_hdr, \
 	en_HeaderType type, SipHeader *hdr, SIP_U32bit index, SipError *err));
#endif
#ifdef SIP_BY_REFERENCE
 SipBool getRequestHeaderAtIndex _ARGS_ ((SipReqHeader *req_hdr, en_HeaderType type, \
 	SipHeader *hdr, SIP_U32bit index, SipError *err));
#else
 SipBool getRequestHeaderAtIndex _ARGS_ ((SipReqHeader *req_hdr, \
 	en_HeaderType type, SipHeader *hdr, SIP_U32bit index, SipError *err));
#endif
#ifdef SIP_BY_REFERENCE
 SipBool getResponseHeaderAtIndex _ARGS_ ((SipRespHeader *resp_hdr, \
 	en_HeaderType type, SipHeader *hdr, SIP_U32bit index, SipError *err));
#else
 SipBool getResponseHeaderAtIndex _ARGS_ ((SipRespHeader *resp_hdr, \
 	en_HeaderType type, SipHeader *hdr, SIP_U32bit index, SipError *err));
#endif
 SipBool deleteGeneralHdrAtIndex _ARGS_ ((SipGeneralHeader *hdr, en_HeaderType type, SIP_U32bit index, SipError *err));
 SipBool deleteRequestHdrAtIndex _ARGS_ ((SipReqHeader *hdr, en_HeaderType type, SIP_U32bit index, SipError *err));
 SipBool deleteResponseHdrAtIndex _ARGS_ ((SipRespHeader *hdr, en_HeaderType type, SIP_U32bit index, SipError *err));
 SipBool deleteAllGeneralHdr _ARGS_ ((SipGeneralHeader *hdr, en_HeaderType type, SipError *err));
 SipBool deleteAllRequestHdr _ARGS_ ((SipReqHeader *hdr, en_HeaderType type, SipError *err));
 SipBool deleteAllResponseHdr _ARGS_ ((SipRespHeader *hdr, en_HeaderType type, SipError *err));
 SipBool setGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *gen_hdr, SipHeader *hdr, SIP_U32bit index, SipError *err));
 SipBool insertGeneralHeaderAtIndex _ARGS_ ((SipGeneralHeader *gen_hdr, SipHeader *hdr, SIP_U32bit index, SipError *err));
 SipBool setRequestHeaderAtIndex _ARGS_ ((SipReqHeader *req_hdr, SipHeader *hdr, SIP_U32bit index, SipError *err));
 SipBool insertRequestHeaderAtIndex _ARGS_ ((SipReqHeader *req_hdr, SipHeader *hdr, SIP_U32bit index, SipError *err));
 SipBool setResponseHeaderAtIndex _ARGS_ ((SipRespHeader *resp_hdr, SipHeader *hdr, SIP_U32bit index, SipError *err));
 SipBool insertResponseHeaderAtIndex _ARGS_ ((SipRespHeader *resp_hdr, SipHeader *hdr, SIP_U32bit index, SipError *err));
void sip_equateTypeInSipHeader _ARGS_((SipHeader *hdr));

SipBool __sip_deleteHeaderAtIndex _ARGS_((SipMessage *msg, en_HeaderType type, \
	SIP_U32bit index, SipError *err));

SipBool __sip_getHeaderTypeAtHeaderLine _ARGS_( ( SipMessage	*msg, SIP_U32bit	line, \
	en_HeaderType		*type, SipError              *err ));

SipBool __sip_getHeaderCountFromHeaderLine _ARGS_( ( SipMessage   *msg, \
	SIP_U32bit     line, SIP_U32bit		*count, SipError              *err ));

SipBool __sip_getHeaderFormFromHeaderLine _ARGS_( ( SipMessage   * msg, \
	SIP_U32bit     line, en_HeaderForm		*text_type, SipError              *err) );

SipBool __sip_setHeaderFormAtHeaderLine _ARGS_(( SipMessage     *msg,   SIP_U32bit line, \
	en_HeaderForm         text_type, SipError              *err ) ); 

SipBool sip_verifyTypeAny _ARGS_(( en_HeaderType         type, SipError              *err )  );

SipBool sip_changeTypeAny _ARGS_((   en_HeaderType         *type, SipError              *err )  );
SipBool sip_makeReqLine _ARGS_(( SipReqLine *pSipReqLine, SIP_S8bit *line, SipError *pErr));
SipBool sip_makeStatusLine _ARGS_((SipStatusLine *pSLine, SIP_S8bit *pStr, SipError *pErr));

SipBool sip_formSipList _ARGS_(
	(SIP_S8bit 	*pOut, SipList 	*pList, 
	SIP_S8bit 	*pSeparator, SIP_U8bit	dLeadingsep,SipError *pErr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
