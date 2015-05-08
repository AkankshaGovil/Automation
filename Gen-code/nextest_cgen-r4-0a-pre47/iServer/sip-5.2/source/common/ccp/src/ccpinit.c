 /******************************************************************************
 ** FUNCTION:
 **	 This file has all th source dCodeNum for init functions of SIP Caller &
 **	 Callee preferences draft
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccpinit.c
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


#include "ccpinit.h"


/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipRequestDispositionHeader
**
** DESCRIPTION: This function initializes a Sip Request Disposition
**		pHeader structure 
**
**********************************************************************/
SipBool sip_ccp_initSipRequestDispositionHeader 
#ifdef ANSI_PROTO
	(SipRequestDispositionHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipRequestDispositionHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipRequestDispositionHeader *)fast_memget(0, sizeof(SipRequestDispositionHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT((*ppHdr)->pFeature);
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

#ifdef SIP_CCP_VERSION10
/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipRejectContactHeader
**
** DESCRIPTION: This function initilaizes a SIP Reject Contact pHeader
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipRejectContactHeader 
#ifdef ANSI_PROTO
	(SipRejectContactHeader **ppHdr,SipError *pErr)
#else
	(ppHdr, pErr)
	SipRejectContactHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipRejectContactHeader *)fast_memget(0, sizeof(SipRejectContactHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	sip_listInit(&((*ppHdr)->slRejectContactParams), __sip_ccp_freeSipRejectContactParam, pErr) ;
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipAcceptContactParam
**
** DESCRIPTION: This function initilaizes a SIP Accept-Contact param
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam ** ppAcceptContactParam, en_AcceptContactType dType, SipError * pErr)
#else
	(ppAcceptContactParam, dType, pErr)
	SipAcceptContactParam ** ppAcceptContactParam;
	en_AcceptContactType dType;
	SipError * pErr;
#endif
{
	*ppAcceptContactParam = (SipAcceptContactParam *)fast_memget(0, sizeof(SipAcceptContactParam), pErr);
	if (*ppAcceptContactParam == SIP_NULL)
		return SipFail;
	(*ppAcceptContactParam)->dType = dType;
	switch (dType)
	{
		case   SipAccContactTypeFeature :
		case   SipAccContactTypeGeneric :
				INIT( (*ppAcceptContactParam)->u.pParam) ; 
				break ;
		case SipAccContactTypeOther:
		     INIT((*ppAcceptContactParam)->u.pToken) ;
				 break ;
		case	SipAccContactTypeAny	:break;
		default				:*pErr = E_INV_TYPE;
						 sip_ccp_freeSipAcceptContactParam(*ppAcceptContactParam);
						 return SipFail;	
	}
	HSS_INITREF((*ppAcceptContactParam)->dRefCount);
	

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipAcceptContactHeader
**
** DESCRIPTION: This function initilaizes a Sip Accept Contact Hedaer
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipAcceptContactHeader 
#ifdef ANSI_PROTO
	(SipAcceptContactHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipAcceptContactHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipAcceptContactHeader *)fast_memget(0, sizeof(SipAcceptContactHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	sip_listInit(&((*ppHdr)->slAcceptContactParams), __sip_ccp_freeSipAcceptContactParam, pErr);
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipRejectContactParam
**
** DESCRIPTION: This function initilaizes a SIP Reject Contact Param
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam ** ppRejectContactParam, en_RejectContactType dType, SipError * pErr)
#else
	(ppRejectContactParam, dType, pErr)
	SipRejectContactParam ** ppRejectContactParam;
	en_RejectContactType dType;
	SipError * pErr;
#endif
{
	*ppRejectContactParam = (SipRejectContactParam *)fast_memget(0, sizeof(SipRejectContactParam), pErr);
	if (*ppRejectContactParam == SIP_NULL)
		return SipFail;
	(*ppRejectContactParam)->dType = dType;
	switch (dType)
	{
		case   SipRejContactTypeFeature :
		case   SipRejContactTypeGeneric :
				INIT( (*ppRejectContactParam)->u.pParam) ; 
				break ;
		case SipRejContactTypeOther:
				INIT( (*ppRejectContactParam)->u.pParam) ; 
				 break ;
		case	SipRejContactTypeAny	:
				 INIT((*ppRejectContactParam)->u.pToken) ;
				 INIT((*ppRejectContactParam)->u.pToken) ;
				 break ;
		default				:*pErr = E_INV_TYPE;
						 sip_ccp_freeSipRejectContactParam(*ppRejectContactParam);
						 return SipFail;	
	}
	HSS_INITREF((*ppRejectContactParam)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#else
/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipRejectContactHeader
**
** DESCRIPTION: This function initilaizes a SIP Reject Contact pHeader
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipRejectContactHeader 
#ifdef ANSI_PROTO
	(SipRejectContactHeader **ppHdr,SipError *pErr)
#else
	(ppHdr, pErr)
	SipRejectContactHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipRejectContactHeader *)fast_memget(0, sizeof(SipRejectContactHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT((*ppHdr)->pDispName);
	INIT((*ppHdr)->pAddrSpec);
	sip_listInit(&((*ppHdr)->slRejectContactParams), __sip_ccp_freeSipRejectContactParam, pErr) ;
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipAcceptContactParam
**
** DESCRIPTION: This function initilaizes a SIP Accept-Contact param
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam ** ppAcceptContactParam, en_AcceptContactType dType, SipError * pErr)
#else
	(ppAcceptContactParam, dType, pErr)
	SipAcceptContactParam ** ppAcceptContactParam;
	en_AcceptContactType dType;
	SipError * pErr;
#endif
{
	*ppAcceptContactParam = (SipAcceptContactParam *)fast_memget(0, sizeof(SipAcceptContactParam), pErr);
	if (*ppAcceptContactParam == SIP_NULL)
		return SipFail;
	(*ppAcceptContactParam)->dType = dType;
	switch (dType)
	{
		case 	SipAccContactTypeExt	:INIT(((*ppAcceptContactParam)->u).pExtParam);
		case 	SipAccContactTypeQvalue	:INIT(((*ppAcceptContactParam)->u).pQvalue);
		case	SipAccContactTypeAny	:break;
		default				:*pErr = E_INV_TYPE;
						 sip_ccp_freeSipAcceptContactParam(*ppAcceptContactParam);
						 return SipFail;	
	}
	HSS_INITREF((*ppAcceptContactParam)->dRefCount);
	

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipAcceptContactHeader
**
** DESCRIPTION: This function initilaizes a Sip Accept Contact Hedaer
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipAcceptContactHeader 
#ifdef ANSI_PROTO
	(SipAcceptContactHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipAcceptContactHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipAcceptContactHeader *)fast_memget(0, sizeof(SipAcceptContactHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT((*ppHdr)->pDispName);
	INIT((*ppHdr)->pAddrSpec);
	sip_listInit(&((*ppHdr)->slAcceptContactParams), __sip_ccp_freeSipAcceptContactParam, pErr);
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_initSipRejectContactParam
**
** DESCRIPTION: This function initilaizes a SIP Reject Contact Param
**		structure
**
**********************************************************************/
SipBool sip_ccp_initSipRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam ** ppRejectContactParam, en_RejectContactType dType, SipError * pErr)
#else
	(ppRejectContactParam, dType, pErr)
	SipRejectContactParam ** ppRejectContactParam;
	en_RejectContactType dType;
	SipError * pErr;
#endif
{
	*ppRejectContactParam = (SipRejectContactParam *)fast_memget(0, sizeof(SipRejectContactParam), pErr);
	if (*ppRejectContactParam == SIP_NULL)
		return SipFail;
	(*ppRejectContactParam)->dType = dType;
	switch (dType)
	{
		case 	SipRejContactTypeExt	:INIT(((*ppRejectContactParam)->u).pExtParam);
		case 	SipRejContactTypeQvalue	:INIT(((*ppRejectContactParam)->u).pQvalue);
		case	SipRejContactTypeAny	:break;
		default				:*pErr = E_INV_TYPE;
						 sip_ccp_freeSipRejectContactParam(*ppRejectContactParam);
						 return SipFail;	
	}
	HSS_INITREF((*ppRejectContactParam)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
