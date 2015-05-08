/************************************************************
** FUNCTION:
**	This file contains the prototypes of the bcpt extention 
** accessor APIs.
**
*************************************************************
**
** FILENAME:
**	bcpt.h
**
** DESCRIPTION
**
**  DATE           NAME                    REFERENCE
**  -----         -------                -------------- 
** 10Feb00		B.Borthakur		            Original    
**
** Copyright 199, Hughes Software Systems, Ltd.
**************************************************************/

#ifndef _SIP_BCPT_H
#define _SIP_BCPT_H

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/******************************************************************************
** Function: sip_bcpt_getContentIdFromMimeHdr 
** Description: get Content Id field from Mime Header
** Parameters:	
**				pHdr(IN)	- Sip Mime Header
**				ppId(OUT) 	- Content Id to retrieve 
**				pErr(OUT)   - Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getContentIdFromMimeHdr _ARGS_( ( SipMimeHeader *pHdr, \
		SIP_S8bit **ppId, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_setContentIdInMimeHdr 
** Description: set Content Id field in Mime Header
** Parameters:
**				pHdr(IN/OUT)	- Sip Mime Header
**				ppId(IN) 		- Content Id to set
**				pErr(OUT)    	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setContentIdInMimeHdr _ARGS_( ( SipMimeHeader *pHdr, \
		SIP_S8bit *ppId, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_getContentDescFromMimeHdr 
** Description: get Content Description field from Mime Header
** Parameters:	
**				pHdr(IN)			- Sip Mime Header
**				ppDescription(OUT) 	- Content Description to retrieve 
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getContentDescFromMimeHdr _ARGS_( ( SipMimeHeader *pHdr, \
		SIP_S8bit **ppDescription, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_setContentDescInMimeHdr 
** Description: set Content Description field in Mime Header
** Parameters:
**				pHdr(IN/OUT)		- Sip Mime Header
**				pDescription(IN)	- Content Description to set
**				pErr(OUT)    		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setContentDescInMimeHdr _ARGS_( ( SipMimeHeader *pHdr, \
		SIP_S8bit *pDescription, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_getAdditionalMimeHdrCountFromMimeHdr 
** Description: get count of Additional mime headers Mime Header
** Parameters:	
**				pHdr(IN)		- Sip Mime Header
**				pCount(OUT) 	- Count of Additional Mime Headers
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getAdditionalMimeHdrCountFromMimeHdr _ARGS_( ( \
		SipMimeHeader	*pHdr, SIP_U32bit	*pCount, SipError	*pErr  ));

/******************************************************************************
** Function: sip_bcpt_getAdditionalMimeHdrAtIndexFromMimeHdr 
** Description: get Additional Mime Header at Index from MimeHeader
** Parameters:	
**				pHdr(IN)			- Sip Mime Header
**				ppAddMimeHdr(OUT)- Additional Mime Header Retrieved
**				index(IN)			- Index at which additional Mime header is retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getAdditionalMimeHdrAtIndexFromMimeHdr _ARGS_( ( \
		SipMimeHeader 	*pHdr, SIP_S8bit 	**ppAddMimeHdr, \
		SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_bcpt_setAdditionalMimeHdrAtIndexInMimeHdr 
** Description: set Additional Mime Header at Index in MimeHeader
** Parameters:	
**				pHdr(IN/OUT)		- Sip Mime Header
**				pAddMimeHdr(IN)		- Additional Mime Header to set
**				index(IN)			- Index at which additional Mime header is set
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setAdditionalMimeHdrAtIndexInMimeHdr _ARGS_( ( SipMimeHeader *pHdr, \
		SIP_S8bit 	*pAddMimeHdr, SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_bcpt_insertAdditionalMimeHdrAtIndexInMimeHdr 
** Description: insert Additional Mime Header at Index in MimeHeader
** Parameters:	
**				pHdr(IN/OUT)		- Sip Mime Header
**				pAddMimeHdr(IN)		- Additional Mime Header to insert
**				index(IN)			- Index at which additional Mime header is insert
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_insertAdditionalMimeHdrAtIndexInMimeHdr _ARGS_( ( \
		SipMimeHeader 	*pHdr, SIP_S8bit 	*pAddMimeHdr, SIP_U32bit 	index, SipError	*pErr ));

/******************************************************************************
** Function: sip_bcpt_deleteAdditionalMimeHdrAtIndexInMimeHdr 
** Description: delete Additional Mime Header at Index in MimeHeader
** Parameters:	
**				pHdr(IN)			- Sip Mime Header
**				index(IN)			- Index at which additional Mime header is to be deleted
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_deleteAdditionalMimeHdrAtIndexInMimeHdr _ARGS_( ( SipMimeHeader	*pHdr, \
		SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_bcpt_getMsgBodyCountFromMime 
** Description: get count of message bodies in Mime Message 
** Parameters:	
**				pMime(IN)		- Sip Mime Message
**				pCount(OUT) 	- Count of Message bodies in Mime Message
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMsgBodyCountFromMime _ARGS_( ( MimeMessage	*pMime, \
		SIP_U32bit	*pCount, SipError	*pErr  ));

#ifdef SIP_BY_REFERENCE
/******************************************************************************
** Function: sip_bcpt_getMsgBodyAtIndexFromMime 
** Description: get Message body at Index from Mime Message
** Parameters:
**				pMime(IN)		- Sip Mime Message
**				ppMsgB(OUT)		- Message Body retrieved
**				index(IN) 	    - Index at which Message body is retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMsgBodyAtIndexFromMime _ARGS_( ( MimeMessage 	*pMime, \
		SipMsgBody **ppMsgB, SIP_U32bit 	index,  SipError 	*pErr ));

#else
/******************************************************************************
** Function: sip_bcpt_getMsgBodyAtIndexFromMime 
** Description: get Message body at Index from Mime Message
** Parameters:
**				pMime(IN)		- Sip Mime Message
**				pMsgB(OUT)		- Message Body retrieved
**				index(IN) 	    - Index at which Message body is retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMsgBodyAtIndexFromMime _ARGS_( ( MimeMessage 	*pMime, \
		SipMsgBody *pMsgB, SIP_U32bit 	index,  SipError 	*pErr ));

#endif
/******************************************************************************
** Function: sip_bcpt_setMsgBodyAtIndexInMime 
** Description: set Message body at Index in Mime Message
** Parameters:
**				pMime(IN)		- Sip Mime Message
**				pMsgB(OUT)		- Message Body to set
**				index(IN) 	    - Index at which Message body is to be set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setMsgBodyAtIndexInMime _ARGS_( ( MimeMessage 	*pMime, \
			SipMsgBody	*pMsgB, SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_bcpt_insertMsgBodyAtIndexInMime 
** Description: insert Message body at Index in Mime Message
** Parameters:
**				pMime(IN)		- Sip Mime Message
**				pMsgB(OUT)		- Message Body to be inserted
**				index(IN) 	    - Index at which Message body is to be inserted
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_insertMsgBodyAtIndexInMime _ARGS_( ( MimeMessage 	*pMime, \
			SipMsgBody	*pMsgB, SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_bcpt_deleteMsgBodyAtIndexInMime 
** Description: delete Message body at Index in Mime Message
** Parameters:
**				pMime(IN)		- Sip Mime Message
**				index(IN) 	    - Index at which Message body is to be deleted
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_deleteMsgBodyAtIndexInMime _ARGS_( ( MimeMessage 	\
		*pMime, SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_bcpt_getMsgBodyTypeAtIndexFromMime 
** Description: get type of Message body at Index in Mime Message
** Parameters:
**				pMime(IN)		- Sip Mime Message
**				index(IN) 	    - Index at which Message body type is desired 
**				pType(IN)		- type of Message body
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMsgBodyTypeAtIndexFromMime _ARGS_( ( MimeMessage *pMime,\
		en_SipMsgBodyType *pType, SIP_U32bit index,  SipError 	*pErr ));

#ifdef SIP_BY_REFERENCE
/******************************************************************************
** Function: sip_bcpt_getIsupFromMsgBody 
** Description: get ISUP message from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				ppIsup(OUT)		- Isup Message to retrieve
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getIsupFromMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		IsupMessage **ppIsup, SipError *pErr));

#else
/******************************************************************************
** Function: sip_bcpt_getIsupFromMsgBody 
** Description: get ISUP message from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				pIsup(OUT)		- Isup Message to retrieve
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getIsupFromMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		IsupMessage *pIsup, SipError *pErr));

#endif
/******************************************************************************
** Function: sip_bcpt_setIsupInMsgBody 
** Description: set ISUP message in Sip Message Body
** Parameters:
**				pMsg(IN/OUT)		- Sip Message Body
**				pIsup(OUT)		- Isup Message to set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setIsupInMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		IsupMessage *pIsup, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/******************************************************************************
** Function: sip_bcpt_getQsigFromMsgBody 
** Description: get QSIG message from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				ppQsig(OUT)		- Qsig Message to retrieve
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getQsigFromMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		QsigMessage **ppQsig, SipError *pErr));

#else
/******************************************************************************
** Function: sip_bcpt_getQsigFromMsgBody 
** Description: get QSIG message from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				pQsig(OUT)		- Qsig Message to retrieve
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getQsigFromMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		QsigMessage *pQsig, SipError *pErr));

#endif
/******************************************************************************
** Function: sip_bcpt_setQsigInMsgBody 
** Description: set QSIG message in Sip Message Body
** Parameters:
**				pMsg(IN/OUT)	- Sip Message Body
**				pQsig(IN)		- Qsig Message to set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setQsigInMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		QsigMessage *pQsig, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_setMimeInMsgBody 
** Description: set Mime message in Sip Message Body
** Parameters:
**				pMsg(IN/OUT)		- Sip Message Body
**				pMime(OUT)		- Mime Message to set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setMimeInMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		MimeMessage *pMime, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/******************************************************************************
** Function: sip_bcpt_getMimeFromMsgBody 
** Description: get Mime message from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				ppMime(OUT)		- Mime Message retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMimeFromMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		MimeMessage **ppMime, SipError *pErr));

#else
/******************************************************************************
** Function: sip_bcpt_getMimeFromMsgBody 
** Description: get Mime message from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				pMime(OUT)		- Mime Message retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMimeFromMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		MimeMessage *pMime, SipError *pErr));

#endif
/******************************************************************************
** Function: sip_bcpt_getLengthFromIsupMessage 
** Description: get length from Isup Message
** Parameters:
**				pMsg(IN)		- Sip Isup Message 
**				dLength(OUT)	- length of Isup Message retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getLengthFromIsupMessage _ARGS_((IsupMessage *pMsg, \
		SIP_U32bit *dLength, SipError *err));

/******************************************************************************
** Function: sip_bcpt_getBodyFromIsupMessage 
** Description: get body from Isup Message
** Parameters:
**				pMsg(IN)		- Sip Isup Message 
**				ppBody(OUT)		- body of Isup Message retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getBodyFromIsupMessage _ARGS_( ( IsupMessage *pMsg, \
		SIP_S8bit **ppBody, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_setBodyInIsupMessage 
** Description: set body in Isup Message
** Parameters:
**				pMsg(IN/OUT)	- Sip Isup Message 
**				pBody(IN)		- body of Isup Message to set
**				length(IN)		- length of body to set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setBodyInIsupMessage _ARGS_( ( IsupMessage *pMsg, \
		SIP_S8bit *pBody, SIP_U32bit length,SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_getLengthFromQsigMessage 
** Description: get length from Qsig Message
** Parameters:
**				pMsg(IN)		- Sip Qsig Message 
**				dLength(OUT)	- length of Qsig Message retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getLengthFromQsigMessage _ARGS_((QsigMessage *pMsg, \
		SIP_U32bit *dLength, SipError *err));

/******************************************************************************
** Function: sip_bcpt_getBodyFromQsigMessage 
** Description: get body from Qsig Message
** Parameters:
**				pMsg(IN)		- Sip Qsig Message 
**				ppBody(OUT)		- body of Qsig Message retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getBodyFromQsigMessage _ARGS_( ( QsigMessage *pMsg, \
		SIP_S8bit **ppBody, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_setBodyInQsigMessage 
** Description: set body in Qsig Message
** Parameters:
**				pMsg(IN/OUT)	- Sip Qsig Message 
**				pBody(IN)		- body of Qsig Message to set
**				length(IN)		- length of body to set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setBodyInQsigMessage _ARGS_( ( QsigMessage *pMsg, \
		SIP_S8bit *pBody, SIP_U32bit length,SipError *pErr));

#ifdef SIP_BY_REFERENCE
/******************************************************************************
** Function: sip_bcpt_getMimeHeaderFromMsgBody 
** Description: get Mime Header from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				ppMime(IN)		- Mime Header to retrieve
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMimeHeaderFromMsgBody _ARGS_(( SipMsgBody *pMsg, \
		SipMimeHeader **ppMime, SipError *pErr));

#else
/******************************************************************************
** Function: sip_bcpt_getMimeHeaderFromMsgBody 
** Description: get Mime Header from Sip Message Body
** Parameters:
**				pMsg(IN)		- Sip Message Body
**				pMime(IN)		- Mime Header to retrieve
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getMimeHeaderFromMsgBody _ARGS_(( SipMsgBody *pMsg, \
		SipMimeHeader *pMime, SipError *pErr));

#endif
/******************************************************************************
** Function: sip_bcpt_setMimeHeaderInMsgBody 
** Description: set Mime Header in Sip Message Body
** Parameters:
**				pMsg(IN/OUT)	- Sip Message Body
**				pMime(IN)		- Mime Header to set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setMimeHeaderInMsgBody _ARGS_( ( SipMsgBody *pMsg, \
		SipMimeHeader *pMime, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_getVersionFromMimeVersionHdr  
** Description: get Version from Mime Version Header
** Parameters:
**				pHdr(IN)		- Mime Version Header
**				ppVersion(OUT)	- Version retrieved
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getVersionFromMimeVersionHdr _ARGS_( (SipHeader *pHdr,\
		SIP_S8bit **ppVersion, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_setVersionInMimeVersionHdr  
** Description: set Version in Mime Version Header
** Parameters:
**				pHdr(IN/OUT)	- Mime Version Header
**				pVersion(IN)	- Version to set
**				pErr(OUT)   	- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setVersionInMimeVersionHdr  _ARGS_( (SipHeader *pHdr, \
		SIP_S8bit *pVersion, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/******************************************************************************
** Function: sip_bcpt_getContentTypeFromMimeHdr 
** Description: get Content Type from Mime Header
** Parameters:
**				pMimeHdr(IN)		- Mime Header
**				ppContentType(OUT)	- Content Type retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getContentTypeFromMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SipHeader **ppContentType, SipError *pErr));

#else
/******************************************************************************
** Function: sip_bcpt_getContentTypeFromMimeHdr 
** Description: get Content Type from Mime Header
** Parameters:
**				pMimeHdr(IN)		- Mime Header
**				ppContentType(OUT)	- Content Type retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getContentTypeFromMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SipHeader *pContentType, SipError *pErr));

#endif
#ifdef SIP_BY_REFERENCE
/******************************************************************************
** Function: sip_bcpt_getContentDispositionFromMimeHdr 
** Description: get Content Type from Mime Header
** Parameters:
**				pMimeHdr(IN)		- Mime Header
**				ppContentDisposition(OUT)	- Content Type retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getContentDispositionFromMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SipHeader **ppContentDisposition, SipError *pErr));

#else
/******************************************************************************
** Function: sip_bcpt_getContentDispositionFromMimeHdr 
** Description: get Content Type from Mime Header
** Parameters:
**				pMimeHdr(IN)		- Mime Header
**				ppContentDisposition(OUT)	- Content Type retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getContentDispositionFromMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SipHeader *pContentDisposition, SipError *pErr));

#endif
/******************************************************************************
** Function: sip_bcpt_setContentDispositionInMimeHdr 
** Description: set Content Type in Mime Header
** Parameters:
**				pMimeHdr(IN/OUT)	- Mime Header
**				pContentDisposition(IN)	- Content Type to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setContentDispositionInMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SipHeader *pHdr, SipError *pErr));


/******************************************************************************
** Function: sip_bcpt_setContentTypeInMimeHdr 
** Description: set Content Type in Mime Header
** Parameters:
**				pMimeHdr(IN/OUT)	- Mime Header
**				pContentType(IN)	- Content Type to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setContentTypeInMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SipHeader *pHdr, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_getContentTransEncodingFromMimeHdr 
** Description: get Content Transfer Encoding from Mime Header
** Parameters:
**				pMimeHdr(IN)		- Mime Header
**				ppEncoding(OUT)		- Content Transfer Encoding retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_getContentTransEncodingFromMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SIP_S8bit **ppEncoding, SipError *pErr));

/******************************************************************************
** Function: sip_bcpt_setContentTransEncodingInMimeHdr 
** Description: set Content Transfer Encoding in Mime Header
** Parameters:
**				pMimeHdr(IN/OUT)	- Mime Header
**				pEncoding(IN)		- Content Transfer Encoding to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_bcpt_setContentTransEncodingInMimeHdr _ARGS_( (SipMimeHeader *pMimeHdr, \
		SIP_S8bit *pEncoding, SipError *pErr));


/******************************************************************************
** Function: sip_getNameFromSipParam 
** Description: get name from url parameter
** Parameters:
**				pParam(IN)			- Url parameter 
**				ppName(OUT)			- Name retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_getNameFromSipParam _ARGS_( ( SipParam *pParam,  \
		SIP_S8bit **ppName, SipError *pErr));

/******************************************************************************
** Function: sip_setNameInSipParam 
** Description: set name in url parameter
** Parameters:
**				pParam(IN/OUT)		- Url parameter 
**				pName(IN)			- Name to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_setNameInSipParam _ARGS_( ( SipParam *pParam, SIP_S8bit *pName, \
		SipError *pErr));

/******************************************************************************
** Function: sip_getValueCountFromSipParam 
** Description: get count of values of url parameter
** Parameters:
**				pParam(IN)			- Url parameter 
**				pCount(IN)			- Count of values 
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_getValueCountFromSipParam _ARGS_( ( SipParam	*pParam, \
		SIP_U32bit	*pCount, SipError	*pErr  ));

/******************************************************************************
** Function: sip_getValueAtIndexFromSipParam 
** Description: get value at Index from url parameter
** Parameters:
**				pParam(IN)			- Url parameter 
**				ppValue(IN)			- value of url parameter retrieve 
**				index(IN)			- index at which value of url param is to be retrieved 
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_getValueAtIndexFromSipParam _ARGS_( ( SipParam 	*pParam, \
		SIP_S8bit 	**ppValue, SIP_U32bit 	index,  SipError 	*pErr ));

/******************************************************************************
** Function: sip_setValueAtIndexInSipParam 
** Description: set value at Index in url parameter
** Parameters:
**				pParam(IN/OUT)		- Url parameter 
**				pValue(IN)			- value of url parameter to set 
**				index(IN)			- index at which value of url param is to be set 
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_setValueAtIndexInSipParam _ARGS_( ( SipParam 	*pParam, \
		SIP_S8bit 	*pValue, SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_insertValueAtIndexInSipParam 
** Description: insert value at Index in url parameter
** Parameters:
**				pParam(IN/OUT)		- Url parameter 
**				pValue(IN)			- value of url parameter to insert 
**				index(IN)			- index at which value of url param is to be inserted 
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_insertValueAtIndexInSipParam _ARGS_( ( SipParam 	*pParam, \
		SIP_S8bit 	*pValue, SIP_U32bit 	index, SipError 	*pErr ));

/******************************************************************************
** Function: sip_deleteValueAtIndexInSipParam 
** Description: delete value at Index in url parameter
** Parameters:
**				pParam(IN/OUT)		- Url parameter 
**				index(IN)			- index at which value of url param is to be deleted 
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************************/
extern SipBool sip_deleteValueAtIndexInSipParam _ARGS_( ( SipParam 	*pParam, \
		SIP_U32bit 	index, SipError 	*pErr ));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
