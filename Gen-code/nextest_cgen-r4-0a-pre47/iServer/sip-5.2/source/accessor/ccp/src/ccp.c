 /******************************************************************************
 ** FUNCTION:
 **	 	This file has all API source dCodeNum of SIP Caller & Callee 
 **             preferences Related Structures
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccp.c
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE	NAME		REFERENCE	REASON
 ** ----	----		--------	------
 ** 8/2/2000	S.Luthra	Original
 **
 ** 26Jul00	S.Luthra	Added SIP_NO_CHECK support
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#include "ccp.h"
#include "sipclone.h"

#ifdef SIP_CCP_VERSION10

/**********************************************************************
**
** FUNCTION:  sip_ccp_getTypeFromAcceptContactParam
**
** DESCRIPTION: This function retrieves the typeof an accept-Conatct 
**		param i.e. whether its of dType ExtParam or Qvalue
**
**********************************************************************/
SipBool sip_ccp_getTypeFromAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pAcceptContact, en_AcceptContactType *pType, SipError *pErr)
#else
	(pAcceptContact, pType, pErr)
	SipAcceptContactParam *pAcceptContact;
	en_AcceptContactType *pType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_getTypeFromAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pAcceptContact == SIP_NULL)||(pType == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getTypeFromAcceptContactParam");
		return SipFail;
	}
#endif
	*pType = pAcceptContact->dType;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getTypeFromAcceptContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getTokenParamFromAcceptContactParam
**
** DESCRIPTION: This function retrives the from a SIP
**		Accept-Contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_getTokenParamFromAcceptContactParam
	(SipAcceptContactParam *pAcceptContact, SIP_S8bit **ppParam, SipError *pErr)
{
	SIP_S8bit *pTempName=SIP_NULL;
	SIPDEBUGFN("Entering function sip_ccp_getTokenParamFromAcceptContactParam ");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAcceptContact == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromAcceptContactHdr");
		return SipFail;
	}
	if (pAcceptContact->dType != SipAccContactTypeOther)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromAcceptContactHdr");
		return SipFail;
	}
	if ( ppParam == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromAcceptContactHdr");
		return SipFail;
	}
		
#endif
	pTempName = pAcceptContact->u.pToken;
	if (pTempName == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromAcceptContactHdr");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppParam = pTempName;	
#else
	*ppParam = (SIP_S8bit *)STRDUPACCESSOR(pTempName);
	if (*ppParam == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromAcceptContactHdr");
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_setTokenParamInAcceptContactHdr
**
** DESCRIPTION: This function sets the Display pName in filed in a SIP
**		Accept Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setTokenParamInAcceptContactParam
	(SipAcceptContactParam *pAcceptContact, SIP_S8bit *pName, SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempName=SIP_NULL;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setTokenParamInAcceptContactParam");    
#ifndef SIP_NO_CHECK
    if( pErr == SIP_NULL )
        return SipFail;
    if( pAcceptContact == SIP_NULL )
		{
				*pErr = E_INV_PARAM ;
				SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInAcceptContactParam");    
        return SipFail;
		}
#endif
	if ( pAcceptContact->u.pToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pAcceptContact->u.pToken), 
				pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInAcceptContactParam");    
				return SipFail;
		}
	}

#ifdef SIP_BY_REFERENCE
		pAcceptContact->u.pToken=pName ;
#else        
	if( pName == SIP_NULL)
         pTempName = SIP_NULL;
	else
	{
		pTempName = (SIP_S8bit *) STRDUPACCESSOR(pName);
		if (pTempName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInAcceptContactParam");    
			return SipFail;
		}
	}
	pAcceptContact->u.pToken=pTempName ;
#endif	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInAcceptContactParam");    
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getFeatureParamFromAcceptContactParam
**
** DESCRIPTION: This function retrives the feature-param field from
**		a SIP accept-contact param structure
**
**********************************************************************/
SipBool sip_ccp_getFeatureParamFromAcceptContactParam 
#ifdef SIP_BY_REFERENCE
	(SipAcceptContactParam *pAcceptContact, SipParam **ppParam, SipError *pErr)
#else
	(SipAcceptContactParam *pAcceptContact, SipParam *pParam, SipError *pErr)
#endif
{
	SipParam	*pTempParam=SIP_NULL;
	SIPDEBUGFN("Entering function sip_ccp_getFeatureParamFromAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ((pAcceptContact == SIP_NULL)||(ppParam == SIP_NULL))
#else
	if ((pAcceptContact == SIP_NULL)||(pParam == SIP_NULL))
#endif
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromAcceptContactParam");
		return SipFail;
	}
	if (pAcceptContact->dType != SipAccContactTypeFeature)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromAcceptContactParam");
		return SipFail;
	}
#endif
	if ((pAcceptContact->u).pParam == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromAcceptContactParam");
		return SipFail;
	}
	pTempParam = (pAcceptContact->u).pParam;
	
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppParam = pTempParam;
#else
	if (__sip_cloneSipParam(pParam, (pAcceptContact->u).pParam, pErr) == SipFail)
	{
		if (pParam->pName != SIP_NULL)
			sip_freeString(pParam->pName);
		sip_listDeleteAll(&(pParam->slValue), pErr);
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromAcceptContactParam");
		return SipFail;
	}
	
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromAcceptContactParam");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_setFeatureParamInAcceptContactParam
**
** DESCRIPTION: This function sets the feature-param field in a SIP
**		accept-contact param structure
**
**********************************************************************/
 SipBool sip_ccp_setFeatureParamInAcceptContactParam 
	(SipAcceptContactParam *pAcceptContact, SipParam *pParam, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_setFeatureParamInAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAcceptContact == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInAcceptContactParam");
		return SipFail;
	}
	if (pAcceptContact->dType != SipAccContactTypeFeature)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInAcceptContactParam");
		return SipFail;
	}
#endif
	if (pParam == SIP_NULL)
	{
		sip_freeSipParam((pAcceptContact->u).pParam);
		(pAcceptContact->u).pParam = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
		sip_freeSipParam((pAcceptContact->u).pParam);
		HSS_LOCKEDINCREF(pParam->dRefCount);
		(pAcceptContact->u).pParam = pParam;

#else
		if ((pAcceptContact->u).pParam == SIP_NULL)
		{
			if (sip_initSipParam(&((pAcceptContact->u).pParam), pErr) == SipFail)
			{
				SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInAcceptContactParam");
				return SipFail;
			}
		}
		if (__sip_cloneSipParam((pAcceptContact->u).pParam, pParam,pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInAcceptContactParam");
				return SipFail;
		}
#endif
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInAcceptContactParam");
	return SipSuccess;	
}



/**********************************************************************
**
** FUNCTION:  sip_ccp_getGenericParamFromAcceptContactParam
**
** DESCRIPTION: This function retrives the extension-param field from
**		a SIP accept-contact param structure
**
**********************************************************************/
SipBool sip_ccp_getGenericParamFromAcceptContactParam 
#ifdef SIP_BY_REFERENCE
	(SipAcceptContactParam *pAcceptContact, SipParam **ppParam, SipError *pErr)
#else
	(SipAcceptContactParam *pAcceptContact, SipParam *pParam, SipError *pErr)
#endif
{
	SipParam	*pTempParam=SIP_NULL;
	SIPDEBUGFN("Entering function sip_ccp_getGenericParamFromAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ((pAcceptContact == SIP_NULL)||(ppParam == SIP_NULL))
#else
	if ((pAcceptContact == SIP_NULL)||(pParam == SIP_NULL))
#endif
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromAcceptContactParam");
		return SipFail;
	}
	if (pAcceptContact->dType != SipAccContactTypeGeneric)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromAcceptContactParam");
		return SipFail;
	}
#endif
	if ((pAcceptContact->u).pParam == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromAcceptContactParam");
		return SipFail;
	}
	pTempParam = (pAcceptContact->u).pParam;
	
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppParam = pTempParam;
#else
	if (__sip_cloneSipParam(pParam, (pAcceptContact->u).pParam, pErr) == SipFail)
	{
		if (pParam->pName != SIP_NULL)
			sip_freeString(pParam->pName);
		sip_listDeleteAll(&(pParam->slValue), pErr);
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromAcceptContactParam");
		return SipFail;
	}
	
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromAcceptContactParam");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_setGenericParamInAcceptContactParam
**
** DESCRIPTION: This function sets the extension-param field in a SIP
**		accept-contact param structure
**
**********************************************************************/
 SipBool sip_ccp_setGenericParamInAcceptContactParam 
	(SipAcceptContactParam *pAcceptContact, SipParam *pParam, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_setGenericParamInAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAcceptContact == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInAcceptContactParam");
		return SipFail;
	}
	if (pAcceptContact->dType != SipAccContactTypeGeneric)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInAcceptContactParam");
		return SipFail;
	}
#endif
	if (pParam == SIP_NULL)
	{
		sip_freeSipParam((pAcceptContact->u).pParam);
		(pAcceptContact->u).pParam = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
		sip_freeSipParam((pAcceptContact->u).pParam);
		HSS_LOCKEDINCREF(pParam->dRefCount);
		(pAcceptContact->u).pParam = pParam;

#else
		if ((pAcceptContact->u).pParam == SIP_NULL)
		{
			if (sip_initSipParam(&((pAcceptContact->u).pParam), pErr) == SipFail)
			{
					SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInAcceptContactParam");
					return SipFail;
			}
		}
		if (__sip_cloneSipParam((pAcceptContact->u).pParam, pParam,pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInAcceptContactParam");
				return SipFail;
		}
#endif
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInAcceptContactParam");
	return SipSuccess;	
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getAcceptContactParamCountFromAcceptContactHdr
**
** DESCRIPTION: This function retrives the nember of accept-contact
**		pParam from a SIP Accept-Conatct pHeader
**
**********************************************************************/
SipBool sip_ccp_getAcceptContactParamCountFromAcceptContactHdr 
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	
	if (( pHdr == SIP_NULL )||(pCount==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
		return SipFail;
	}
	
#endif	
	if (sip_listSizeOf( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), pCount , pErr) == SipFail )
	{
		SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr
**
** DESCRIPTION: This function retrives an accept-contact param at a
**		specified index from a SIP accept Conatct pHeader
**
**********************************************************************/
SipBool sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr 
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAcceptContactParam **ppAcceptContactParam, SIP_U32bit index, SipError *pErr)
#else
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr)
#endif
{
	SIP_Pvoid pElementFromList=SIP_NULL;
	SipAcceptContactParam *pTempAcceptContactParam=SIP_NULL;
		
	SIPDEBUGFN("Entering function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");		
#ifndef SIP_NO_CHECK
	if(  pErr == SIP_NULL  )
		return SipFail;
	
	if ( pHdr == SIP_NULL )
	{
			*pErr = E_INV_PARAM;
			SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
			return SipFail;
	}
	
#ifdef SIP_BY_REFERENCE
	if (ppAcceptContactParam == SIP_NULL)
#else
	if (pAcceptContactParam == SIP_NULL)
#endif
	{
			*pErr = E_INV_PARAM;
			SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
			return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
			*pErr = E_INV_HEADER;
			SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
			return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
			*pErr = E_INV_TYPE;
			SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
			return SipFail;
	}
#endif
	
    if ( sip_listGetAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), index, &pElementFromList, pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
				return SipFail;
		}
	
	if (pElementFromList == SIP_NULL)
	{
			*pErr = E_NO_EXIST;
			SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
			return SipFail;
	}
	pTempAcceptContactParam = (SipAcceptContactParam *)pElementFromList;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempAcceptContactParam->dRefCount);
	*ppAcceptContactParam = pTempAcceptContactParam;
#else

	if (__sip_ccp_cloneSipAcceptContactParam(pAcceptContactParam, pTempAcceptContactParam, pErr) == SipFail)
	{
		switch (pAcceptContactParam->dType)
		{
			case	SipAccContactTypeGeneric	:
			case	SipAccContactTypeFeature	:
			        sip_freeSipParam((pAcceptContactParam->u).pParam);
							(pAcceptContactParam->u).pParam = SIP_NULL;
							break;

				
			case	SipAccContactTypeOther	:
			     fast_memfree(ACCESSOR_MEM_ID, \
					        (SIP_Pvoid)((pAcceptContactParam->u).pToken), pErr);
						(pAcceptContactParam->u).pToken = SIP_NULL;
						break;

			case	SipAccContactTypeAny	:break;

			default				:*pErr = E_INV_TYPE;
							 return SipFail;
		}
		pAcceptContactParam->dType = SipAccContactTypeAny;	
		SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
		return SipFail;
	}
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION: sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr 
**
** DESCRIPTION: This fuunction sets an accept-contact param at a
**		specified index in the Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, pAcceptContactParam, index, pErr)
	SipHeader *pHdr;
	SipAcceptContactParam *pAcceptContactParam;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SipAcceptContactParam *pTempAcceptContactParam;
	SIPDEBUGFN("Entering function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
#endif
	if ( pAcceptContactParam == SIP_NULL )
		pTempAcceptContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAcceptContactParam->dRefCount);
		pTempAcceptContactParam = pAcceptContactParam;	
#else

		if (sip_ccp_validateSipAcceptContactType(pAcceptContactParam->dType, pErr) == SipFail)
		{
			SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
			return SipFail;
		}
		if (sip_ccp_initSipAcceptContactParam(&pTempAcceptContactParam, pAcceptContactParam->dType, \
								pErr) == SipFail)
		{
			SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
			return SipFail;
		}
		if (__sip_ccp_cloneSipAcceptContactParam(pTempAcceptContactParam, pAcceptContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
			SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
			return SipFail;
		}
#endif
	}
			
	if( sip_listSetAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), index, (SIP_Pvoid)(pTempAcceptContactParam), pErr) == SipFail)
	{
		if (pTempAcceptContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pAcceptContactParam->dRefCount);
#else
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
#endif
		}
		SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr
**
** DESCRIPTION: This function inserts an accept-contact param at a
**		specified index in the Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr 
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr)
{
	SipAcceptContactParam *pTempAcceptContactParam;
	SIPDEBUGFN("Entering function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
#endif
	if ( pAcceptContactParam == SIP_NULL )
		pTempAcceptContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAcceptContactParam->dRefCount);
		pTempAcceptContactParam = pAcceptContactParam;	
#else
		if (sip_ccp_validateSipAcceptContactType(pAcceptContactParam->dType, pErr) == SipFail)
		{
			SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
			return SipFail;
		}
		if (sip_ccp_initSipAcceptContactParam(&pTempAcceptContactParam, pAcceptContactParam->dType\
								,pErr) == SipFail)
		{
			SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
			return SipFail;
		}
		if (__sip_ccp_cloneSipAcceptContactParam(pTempAcceptContactParam, pAcceptContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
			SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
			return SipFail;
		}
#endif
	}
			
	if( sip_listInsertAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), index, (SIP_Pvoid)(pTempAcceptContactParam), pErr) == SipFail)
	{
		if (pTempAcceptContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pAcceptContactParam->dRefCount);
#else
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
#endif
		}

		SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr
**
** DESCRIPTION: This function deletes an accept-contact param at a
**		specified index in the SIP Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, index, pErr)
	SipHeader *pHdr;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
		
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams),\
							index, pErr) == SipFail)
	{
			SIPDEBUGFN("Exiting function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
			return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
	return SipSuccess;
}



/**********************************************************************
**
** FUNCTION:  sip_ccp_getTypeFromRejectContactParam
**
** DESCRIPTION: This function retrieves the dType of a Reject-Contact
**		param structure i.e. Ext-param or Q-pValue
**
**********************************************************************/
SipBool sip_ccp_getTypeFromRejectContactParam 
	(SipRejectContactParam *pRejectContact, en_RejectContactType *pType, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_getTypeFromRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pRejectContact == SIP_NULL)||(pType == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getTypeFromRejectContactParam");
		return SipFail;
	}
#endif
	*pType = pRejectContact->dType;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getTypeFromRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getTokenParamFromRejectContactParam
**
** DESCRIPTION: This function retrieves the pTokenParam-pValue field from a SIP
**		Reject-Contact param structure
**
**********************************************************************/
SipBool sip_ccp_getTokenParamFromRejectContactParam 
	(SipRejectContactParam *pRejectContact, SIP_S8bit **ppToken, SipError *pErr)
{
 	SIP_S8bit *pTempValue;
	SIPDEBUGFN("Entering function sip_ccp_getTokenFromRejectContactParam");
#ifndef SIP_NO_CHECK
 	if (pErr == SIP_NULL)
 		return SipFail;
 		
 	if ((pRejectContact == SIP_NULL)||(ppToken == SIP_NULL))
 	{
 		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromRejectContactParam");
 	 	return SipFail;
 	}
 	
 	if ( pRejectContact->dType != SipRejContactTypeOther )
 	{
 	 	*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromRejectContactParam");
 	 	return SipFail;
 	}
#endif
 	pTempValue = (pRejectContact->u).pToken;
 	if (pTempValue == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromRejectContactParam");
 		return SipFail;
 	}
#ifdef SIP_BY_REFERENCE
	*ppToken = pTempValue;
#else 	
 	*ppToken = (SIP_S8bit *)STRDUPACCESSOR(pTempValue);
	if (*ppToken == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromRejectContactParam");
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getTokenParamFromRejectContactParam");
 	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getFeatureParamFromRejectContactParam
**
** DESCRIPTION: This function retrives the extension-parm field from a
**		SIP Reject contact param structrure
**
**********************************************************************/
SipBool sip_ccp_getFeatureParamFromRejectContactParam 
#ifdef SIP_BY_REFERENCE
	(SipRejectContactParam *pRejectContact, SipParam **ppParam, SipError *pErr)
#else
	(SipRejectContactParam *pRejectContact, SipParam *pParam, SipError *pErr)
#endif
{

	SIPDEBUGFN("Entering function sip_ccp_getFeatureParamFromRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ((pRejectContact == SIP_NULL)||(ppParam == SIP_NULL))
#else
	if ((pRejectContact == SIP_NULL)||(pParam == SIP_NULL))
#endif
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromRejectContactParam");
		return SipFail;
	}
	if (pRejectContact->dType != SipRejContactTypeFeature)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromRejectContactParam");
		return SipFail;
	}
#endif
	if ((pRejectContact->u).pParam == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromRejectContactParam");
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppParam = (pRejectContact->u).pParam;
	HSS_LOCKEDINCREF((*ppParam)->dRefCount);
#else
	if (__sip_cloneSipParam(pParam, (pRejectContact->u).pParam, pErr) == SipFail)
	{
		if (pParam->pName != SIP_NULL)
			sip_freeString(pParam->pName);
		sip_listDeleteAll(&(pParam->slValue), pErr);
		SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromRejectContactParam");
		return SipFail;
	}
	
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getFeatureParamFromRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getGenericParamFromRejectContactParam
**
** DESCRIPTION: This function retrives the extension-parm field from a
**		SIP Reject contact param structrure
**
**********************************************************************/
SipBool sip_ccp_getGenericParamFromRejectContactParam 
#ifdef SIP_BY_REFERENCE
	(SipRejectContactParam *pRejectContact, SipParam **ppParam, SipError *pErr)
#else
	(SipRejectContactParam *pRejectContact, SipParam *pParam, SipError *pErr)
#endif
{

	SIPDEBUGFN("Entering function sip_ccp_getGenericParamFromRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ((pRejectContact == SIP_NULL)||(ppParam == SIP_NULL))
#else
	if ((pRejectContact == SIP_NULL)||(pParam == SIP_NULL))
#endif
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromRejectContactParam");
		return SipFail;
	}
	if (pRejectContact->dType != SipRejContactTypeGeneric)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromRejectContactParam");
		return SipFail;
	}
#endif
	if ((pRejectContact->u).pParam == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromRejectContactParam");
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppParam = (pRejectContact->u).pParam;
	HSS_LOCKEDINCREF((*ppParam)->dRefCount);
#else
	if (__sip_cloneSipParam(pParam, (pRejectContact->u).pParam, pErr) == SipFail)
	{
		if (pParam->pName != SIP_NULL)
			sip_freeString(pParam->pName);
		sip_listDeleteAll(&(pParam->slValue), pErr);
		SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromRejectContactParam");
		return SipFail;
	}
	
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getGenericParamFromRejectContactParam");
	return SipSuccess;
}



/**********************************************************************
**
** FUNCTION:  sip_ccp_setTokenParamInRejectContactPara
**
** DESCRIPTION: This function sets the Q-pValue field in a SIP Reject
**		contact param structure
**
**********************************************************************/
SipBool sip_ccp_setTokenParamInRejectContactParam 
	(SipRejectContactParam *pRejectContact, SIP_S8bit *pToken, SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempValue;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setTokenParamInRejectContactParam");   
#ifndef SIP_NO_CHECK
    if( pErr == SIP_NULL )
        return SipFail;

    if ( pRejectContact == SIP_NULL)
    {
				*pErr = E_INV_PARAM;
				SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInRejectContactParam");
        return SipFail;
    }

    if ( pRejectContact->dType != SipRejContactTypeOther )
    {
        *pErr = E_INV_TYPE;
				SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInRejectContactParam");
        return SipFail;
    }
#endif     
	 if ( (pRejectContact->u).pToken != SIP_NULL)
	 {
	    if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&((pRejectContact->u).
					pToken), pErr) == SipFail)
					{
							SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInRejectContactParam");
							return SipFail;
					}
   	}
#ifdef SIP_BY_REFERENCE
	(pRejectContact->u).pToken = pToken;	
#else   
    if( pToken == SIP_NULL)
          pTempValue = SIP_NULL;
    else
    {
				pTempValue = (SIP_S8bit *) STRDUPACCESSOR(pToken);
				if (pTempValue == SIP_NULL)
				{
						*pErr = E_NO_MEM;
						SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInRejectContactParam");
						return SipFail;
				}
		}	
        
   
	(pRejectContact->u).pToken = pTempValue;
#endif	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setTokenParamInRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_setFeatureParamInRejectContactParam
**
** DESCRIPTION: This function sets the Feature-param field in a SIP
**		reject-contact param structure
**
**********************************************************************/
SipBool sip_ccp_setFeatureParamInRejectContactParam 
	(SipRejectContactParam *pRejectContact, SipParam *pParam, 
	SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_setFeatureParamInRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pRejectContact == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInRejectContactParam");
		return SipFail;
	}
	if (pRejectContact->dType != SipRejContactTypeFeature)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInRejectContactParam");
		return SipFail;
	}
#endif
	if (pParam == SIP_NULL)
	{
		sip_freeSipParam((pRejectContact->u).pParam);
		(pRejectContact->u).pParam = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pParam->dRefCount);
	sip_freeSipParam((pRejectContact->u).pParam);
	(pRejectContact->u).pParam = pParam;
	
#else
		if ((pRejectContact->u).pParam == SIP_NULL)
		{
			if (sip_initSipParam(&((pRejectContact->u).pParam), pErr) == SipFail)
			{
					SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInRejectContactParam");
					return SipFail;
			}
		}
		if (__sip_cloneSipParam((pRejectContact->u).pParam, pParam, pErr) 
						== SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInRejectContactParam");
				return SipFail;
		}
	
#endif
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setFeatureParamInRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_setGenericParamInRejectContactParam
**
** DESCRIPTION: This function sets the Feature-param field in a SIP
**		reject-contact param structure
**
**********************************************************************/
SipBool sip_ccp_setGenericParamInRejectContactParam 
	(SipRejectContactParam *pRejectContact, SipParam *pParam, 
	SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_setGenericParamInRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pRejectContact == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInRejectContactParam");
		return SipFail;
	}
	if (pRejectContact->dType != SipRejContactTypeGeneric)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInRejectContactParam");
		return SipFail;
	}
#endif
	if (pParam == SIP_NULL)
	{
		sip_freeSipParam((pRejectContact->u).pParam);
		(pRejectContact->u).pParam = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pParam->dRefCount);
	sip_freeSipParam((pRejectContact->u).pParam);
	(pRejectContact->u).pParam = pParam;
	
#else
		if ((pRejectContact->u).pParam == SIP_NULL)
		{
			if (sip_initSipParam(&((pRejectContact->u).pParam), pErr) == SipFail)
			{
					SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInRejectContactParam");
					return SipFail;
			}
		}
		if (__sip_cloneSipParam((pRejectContact->u).pParam, pParam, pErr) 
						== SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInRejectContactParam");
				return SipFail;
		}
	
#endif
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setGenericParamInRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getRejectContactParamCountFromRejectContactHdr
**
** DESCRIPTION: This function retrives the number of reject-contact
**		pParam inb a reject-contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_getRejectContactParamCountFromRejectContactHdr 
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_getRejectParamCountFromRejectContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	
	if ((pHdr == SIP_NULL)||(pCount==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamCountFromRejectContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamCountFromRejectContactHdr");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamCountFromRejectContactHdr");
		return SipFail;
	}
#endif	
	if (sip_listSizeOf( &(((SipRejectContactHeader *)(pHdr->pHeader))->
			slRejectContactParams), pCount , pErr) == SipFail )
	{
			SIPDEBUGFN("Exiting function sip_ccp_getRejectParamCountFromRejectContactHdr");
			return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getRejectParamCountFromRejectContactHdr");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr
**
** DESCRIPTION: This function retrivea a reject-contact param at a 
**		specified index in a SIP Reject-contact pHeader.
**
**********************************************************************/
SipBool sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr 
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipRejectContactParam **ppRejectContactParam, SIP_U32bit index, SipError *pErr)
#else
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr)
#endif
{
	SIP_Pvoid pElementFromList;
	SipRejectContactParam *pTempRejectContactParam;
		
	SIPDEBUGFN("Entering function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");		
#ifndef SIP_NO_CHECK
	if(  pErr == SIP_NULL  )
		return SipFail;
	
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
		return SipFail;
	}
	
#ifdef SIP_BY_REFERENCE
	if (ppRejectContactParam == SIP_NULL)
#else
	if (pRejectContactParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
		return SipFail;
	}
		
	if (pHdr->pHeader == SIP_NULL)
	{
			*pErr = E_INV_HEADER;
			SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
			return SipFail;
	}
	 
#endif
	 
    if ( sip_listGetAt( &(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), index, &pElementFromList, pErr) == SipFail)
	{
			SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
			return SipFail;
	}
	
	if (pElementFromList == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
		return SipFail;
	}
	pTempRejectContactParam = (SipRejectContactParam *)pElementFromList;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempRejectContactParam->dRefCount);
	*ppRejectContactParam = pTempRejectContactParam;	
#else
	if (__sip_ccp_cloneSipRejectContactParam(pRejectContactParam, pTempRejectContactParam, pErr) == SipFail)
	{
		switch (pRejectContactParam->dType)
		{
			case	SipRejContactTypeFeature	:
			case	SipRejContactTypeGeneric	:
					sip_freeSipParam((pRejectContactParam->u).pParam);
					(pRejectContactParam->u).pParam = SIP_NULL;
					break;

			case	SipRejContactTypeOther	:
					fast_memfree(ACCESSOR_MEM_ID, 
									(SIP_Pvoid)((pRejectContactParam->u).pToken), pErr);
					(pRejectContactParam->u).pToken = SIP_NULL;
					break;


			case	SipRejContactTypeAny	:
					break;

			default				:
						*pErr = E_INV_TYPE;
						return SipFail;
		}
		pRejectContactParam->dType = SipRejContactTypeAny;	
		SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_insertRejectContactParamAtIndexInRejectContactHd
**
** DESCRIPTION: This function inserts  a reject-contact param at a 
**		specified index in a SIP Reject contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_insertRejectContactParamAtIndexInRejectContactHdr 
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr)
{
	SipRejectContactParam *pTempRejectContactParam;
	SIPDEBUGFN("Entering function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}
#endif
	if ( pRejectContactParam == SIP_NULL )
		pTempRejectContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pRejectContactParam->dRefCount);
		pTempRejectContactParam = pRejectContactParam;
#else
		if (sip_ccp_validateSipRejectContactType(pRejectContactParam->dType, pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
				return SipFail;
		}
		if (sip_ccp_initSipRejectContactParam(&pTempRejectContactParam, pRejectContactParam->dType\
				, pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
				return SipFail;
		}
		if (__sip_ccp_cloneSipRejectContactParam(pTempRejectContactParam, pRejectContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
			SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
			return SipFail;
		}
#endif
	}
			
	if( sip_listInsertAt( &(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), index, (SIP_Pvoid)(pTempRejectContactParam), pErr) == SipFail)
	{
		if (pTempRejectContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pRejectContactParam->dRefCount);
#else
			sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
#endif
		}
		SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}

	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
	return SipSuccess;
}

	
/**********************************************************************
**
** FUNCTION:  sip_ccp_setRejectContactParamAtIndexInRejectContactHdr
**
** DESCRIPTION: This function sets a reject-contact param at a specified
** 		index in a SIP reject-contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_setRejectContactParamAtIndexInRejectContactHdr 
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr)
{
	SipRejectContactParam *pTempRejectContactParam;
	SIPDEBUGFN("Entering function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}
	
#endif
	if ( pRejectContactParam == SIP_NULL )
		pTempRejectContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pRejectContactParam->dRefCount);
		pTempRejectContactParam = pRejectContactParam;
#else
		if (sip_ccp_validateSipRejectContactType(pRejectContactParam->dType, pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
				return SipFail;
		}
		if (sip_ccp_initSipRejectContactParam(&pTempRejectContactParam, pRejectContactParam->dType\
								, pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
				return SipFail;
		}
		if (__sip_ccp_cloneSipRejectContactParam(pTempRejectContactParam, pRejectContactParam, pErr) == SipFail)
		{
				sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
				SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
				return SipFail;
		}
#endif
	}
			
	if( sip_listSetAt( &(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), index, (SIP_Pvoid)(pTempRejectContactParam), pErr) == SipFail)
	{
		if (pTempRejectContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pRejectContactParam->dRefCount);
#else
			sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
#endif
		}
		SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr
**
** DESCRIPTION: This function deletes a Reject-Contact param at a
**		specified index in the Reject-Contact Header structure
**
**********************************************************************/
SipBool sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr 
	(SipHeader *pHdr, SIP_U32bit index, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
		
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams)\
							, index, pErr) == SipFail)
	{
			SIPDEBUGFN("Exiting function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
			return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
	return SipSuccess;
}

#else
/**********************************************************************
**
** FUNCTION:  sip_ccp_getTypeFromAcceptContactParam
**
** DESCRIPTION: This function retrieves the typeof an accept-Conatct 
**		param i.e. whether its of dType ExtParam or Qvalue
**
**********************************************************************/
SipBool sip_ccp_getTypeFromAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pAcceptContact, en_AcceptContactType *pType, SipError *pErr)
#else
	(pAcceptContact, pType, pErr)
	SipAcceptContactParam *pAcceptContact;
	en_AcceptContactType *pType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_getTypeFromAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pAcceptContact == SIP_NULL)||(pType == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pType = pAcceptContact->dType;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getTypeFromAcceptContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION: sip_ccp_getQvalueFromAcceptContactParam 
**
** DESCRIPTION: This function retrieves the pQValue-pValue field from an Accept
**		contact param structure
**
**********************************************************************/
SipBool sip_ccp_getQvalueFromAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pAcceptContact, SIP_S8bit **ppQvalue, SipError *pErr)
#else
	(pAcceptContact, ppQvalue, pErr)
	SipAcceptContactParam *pAcceptContact;
	SIP_S8bit **ppQvalue;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempQvalue;
	SIPDEBUGFN("Entering function sip_ccp_getQvalueFromAcceptContactParam");
#ifndef SIP_NO_CHECK
 	if (pErr == SIP_NULL)
 		return SipFail;
 		
 	if ((pAcceptContact == SIP_NULL)||(ppQvalue == SIP_NULL))
 	{
 		*pErr = E_INV_PARAM;
 	 	return SipFail;
 	}
 	
 	if ( pAcceptContact->dType != SipAccContactTypeQvalue )
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
#endif
 	pTempQvalue = (pAcceptContact->u).pQvalue;
 	if (pTempQvalue == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifdef SIP_BY_REFERENCE
 	*ppQvalue = pTempQvalue;
#else
 	*ppQvalue = (SIP_S8bit *) STRDUPACCESSOR (pTempQvalue);
	if (*ppQvalue == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getQvalueFromAcceptContactParam");
 	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getExtParamFromAcceptContactParam
**
** DESCRIPTION: This function retrives the extension-param field from
**		a SIP accept-contact param structure
**
**********************************************************************/
SipBool sip_ccp_getExtParamFromAcceptContactParam 
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipAcceptContactParam *pAcceptContact, SipParam **ppExtParam, SipError *pErr)
#else
	(SipAcceptContactParam *pAcceptContact, SipParam *pExtParam, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pAcceptContact, ppExtParam, pErr)
	SipAcceptContactParam *pAcceptContact;
	SipParam **ppExtParam;
	SipError *pErr;
#else

	(pAcceptContact, pExtParam, pErr)
	SipAcceptContactParam *pAcceptContact;
	SipParam *pExtParam;
	SipError *pErr;
#endif
#endif
{
	SipParam	*pTempParam;
	SIPDEBUGFN("Entering function sip_ccp_getExtParamFromAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ((pAcceptContact == SIP_NULL)||(ppExtParam == SIP_NULL))
#else
	if ((pAcceptContact == SIP_NULL)||(pExtParam == SIP_NULL))
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pAcceptContact->dType != SipAccContactTypeExt)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ((pAcceptContact->u).pExtParam == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	pTempParam = (pAcceptContact->u).pExtParam;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppExtParam = pTempParam;
#else
	if (__sip_cloneSipParam(pExtParam, (pAcceptContact->u).pExtParam, pErr) == SipFail)
	{
		if (pExtParam->pName != SIP_NULL)
			sip_freeString(pExtParam->pName);
		sip_listDeleteAll(&(pExtParam->slValue), pErr);
		return SipFail;
	}
	
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getExtParamFromAcceptContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_setQvalueInAcceptContactParam
**
** DESCRIPTION: This function sets the pQValue-pValue field in a SIP accept-
**		contact param structure
**
**********************************************************************/
SipBool sip_ccp_setQvalueInAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pAcceptContact, SIP_S8bit *pQvalue, SipError *pErr)
#else
	(pAcceptContact, pQvalue, pErr)
	SipAcceptContactParam *pAcceptContact;
	SIP_S8bit *pQvalue;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
 	SIP_S8bit *pTempQvalue;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setQvalueInAcceptContactParam");        
#ifndef SIP_NO_CHECK
    if( pErr == SIP_NULL )
        return SipFail;

    if ( pAcceptContact == SIP_NULL)
    {
        *pErr = E_INV_PARAM;
        return SipFail;
    }

    if ( pAcceptContact->dType != SipAccContactTypeQvalue )
    {
        *pErr = E_INV_TYPE;
        return SipFail;
    }
#endif
	if ( (pAcceptContact->u).pQvalue != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&((pAcceptContact->u).pQvalue), pErr) == SipFail)
			return SipFail;
	}
        
#ifdef SIP_BY_REFERENCE
	(pAcceptContact->u).pQvalue = pQvalue;
#else
    if( pQvalue == SIP_NULL)
        pTempQvalue = SIP_NULL;
    else
    {
		pTempQvalue = (SIP_S8bit *) STRDUPACCESSOR(pQvalue);
		if (pTempQvalue == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}	
	(pAcceptContact->u).pQvalue = pTempQvalue;
#endif	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setQvalueInAcceptContactParam");
        return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_setExtParamInAcceptContactParam
**
** DESCRIPTION: This function sets the extension-param field in a SIP
**		accept-contact param structure
**
**********************************************************************/
 SipBool sip_ccp_setExtParamInAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pAcceptContact, SipParam *pExtParam, SipError *pErr)
#else
	(pAcceptContact, pExtParam, pErr)
	SipAcceptContactParam *pAcceptContact;
	SipParam *pExtParam;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_setExtParamInAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAcceptContact == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pAcceptContact->dType != SipAccContactTypeExt)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if (pExtParam == SIP_NULL)
	{
		sip_freeSipParam((pAcceptContact->u).pExtParam);
		(pAcceptContact->u).pExtParam = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
		sip_freeSipParam((pAcceptContact->u).pExtParam);
		HSS_LOCKEDINCREF(pExtParam->dRefCount);
		(pAcceptContact->u).pExtParam = pExtParam;

#else
		if ((pAcceptContact->u).pExtParam == SIP_NULL)
		{
			if (sip_initSipParam(&((pAcceptContact->u).pExtParam), pErr) == SipFail)
				return SipFail;
		}
		if (__sip_cloneSipParam((pAcceptContact->u).pExtParam, pExtParam,pErr) == SipFail)
			return SipFail;
#endif
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setExtParamInAcceptContactParam");
	return SipSuccess;	
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getTypeFromRejectContactParam
**
** DESCRIPTION: This function retrieves the dType of a Reject-Contact
**		param structure i.e. Ext-param or Q-pValue
**
**********************************************************************/
SipBool sip_ccp_getTypeFromRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam *pRejectContact, en_RejectContactType *pType, SipError *pErr)
#else
	(pRejectContact, pType, pErr)
	SipRejectContactParam *pRejectContact;
	en_RejectContactType *pType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_getTypeFromRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pRejectContact == SIP_NULL)||(pType == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pType = pRejectContact->dType;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getTypeFromRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getQvalueFromRejectContactParam
**
** DESCRIPTION: This function retrieves the pQValue-pValue field from a SIP
**		Reject-Contact param structure
**
**********************************************************************/
SipBool sip_ccp_getQvalueFromRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam *pRejectContact, SIP_S8bit **ppQvalue, SipError *pErr)
#else
	(pRejectContact, ppQvalue, pErr)
	SipRejectContactParam *pRejectContact;
	SIP_S8bit **ppQvalue;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempQvalue;
	SIPDEBUGFN("Entering function sip_ccp_getQvalueFromRejectContactParam");
#ifndef SIP_NO_CHECK
 	if (pErr == SIP_NULL)
 		return SipFail;
 		
 	if ((pRejectContact == SIP_NULL)||(ppQvalue == SIP_NULL))
 	{
 		*pErr = E_INV_PARAM;
 	 	return SipFail;
 	}
 	
 	if ( pRejectContact->dType != SipRejContactTypeQvalue )
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
#endif
 	pTempQvalue = (pRejectContact->u).pQvalue;
 	if (pTempQvalue == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifdef SIP_BY_REFERENCE
	*ppQvalue = pTempQvalue;
#else 	
 	*ppQvalue = (SIP_S8bit *)STRDUPACCESSOR(pTempQvalue);
	if (*ppQvalue == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getQvalueFromRejectContactParam");
 	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getExtParamFromRejectContactParam
**
** DESCRIPTION: This function retrives the extension-parm field from a
**		SIP Reject contact param structrure
**
**********************************************************************/
SipBool sip_ccp_getExtParamFromRejectContactParam 
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipRejectContactParam *pRejectContact, SipParam **ppExtParam, SipError *pErr)
#else
	(SipRejectContactParam *pRejectContact, SipParam *pExtParam, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pRejectContact, ppExtParam, pErr)
	SipRejectContactParam *pRejectContact;
	SipParam **ppExtParam;
	SipError *pErr;
#else
	(pRejectContact, pExtParam, pErr)
	SipRejectContactParam *pRejectContact;
	SipParam *pExtParam;
	SipError *pErr;
#endif
#endif
{

	SIPDEBUGFN("Entering function sip_ccp_getExtParamFromRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ((pRejectContact == SIP_NULL)||(ppExtParam == SIP_NULL))
#else
	if ((pRejectContact == SIP_NULL)||(pExtParam == SIP_NULL))
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pRejectContact->dType != SipRejContactTypeExt)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ((pRejectContact->u).pExtParam == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppExtParam = (pRejectContact->u).pExtParam;
	HSS_LOCKEDINCREF((*ppExtParam)->dRefCount);
#else
	if (__sip_cloneSipParam(pExtParam, (pRejectContact->u).pExtParam, pErr) == SipFail)
	{
		if (pExtParam->pName != SIP_NULL)
			sip_freeString(pExtParam->pName);
		sip_listDeleteAll(&(pExtParam->slValue), pErr);
		return SipFail;
	}
	
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getExtParamFromRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_setQvalueInRejectContactPara
**
** DESCRIPTION: This function sets the Q-pValue field in a SIP Reject
**		contact param structure
**
**********************************************************************/
SipBool sip_ccp_setQvalueInRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam *pRejectContact, SIP_S8bit *pQvalue, SipError *pErr)
#else
	(pRejectContact, pQvalue, pErr)
	SipRejectContactParam *pRejectContact;
	SIP_S8bit *pQvalue;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempQvalue;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setQvalueInRejectContactParam");        
#ifndef SIP_NO_CHECK
    if( pErr == SIP_NULL )
        return SipFail;

    if ( pRejectContact == SIP_NULL)
    {
		*pErr = E_INV_PARAM;
        return SipFail;
    }

    if ( pRejectContact->dType != SipRejContactTypeQvalue )
    {
        *pErr = E_INV_TYPE;
        return SipFail;
    }
#endif     
	 if ( (pRejectContact->u).pQvalue != SIP_NULL)
	 {
	    if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&((pRejectContact->u).pQvalue), pErr) == SipFail)
			return SipFail;
   	}
#ifdef SIP_BY_REFERENCE
	(pRejectContact->u).pQvalue = pQvalue;	
#else   
    if( pQvalue == SIP_NULL)
          pTempQvalue = SIP_NULL;
    else
    {
		pTempQvalue = (SIP_S8bit *) STRDUPACCESSOR(pQvalue);
		if (pTempQvalue == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	 }	
        
   
	(pRejectContact->u).pQvalue = pTempQvalue;
#endif	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setQvalueInRejectContactParam");
    return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_setExtParamInRejectContactParam
**
** DESCRIPTION: This function sets the Extension-param field in a SIP
**		reject-contact param structure
**
**********************************************************************/
SipBool sip_ccp_setExtParamInRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam *pRejectContact, SipParam *pExtParam, SipError *pErr)
#else
	(pRejectContact, pExtParam, pErr)
	SipRejectContactParam *pRejectContact;
	SipParam *pExtParam;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_setExtParamInRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pRejectContact == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pRejectContact->dType != SipRejContactTypeExt)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if (pExtParam == SIP_NULL)
	{
		sip_freeSipParam((pRejectContact->u).pExtParam);
		(pRejectContact->u).pExtParam = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pExtParam->dRefCount);
	sip_freeSipParam((pRejectContact->u).pExtParam);
	(pRejectContact->u).pExtParam = pExtParam;
	
#else
		if ((pRejectContact->u).pExtParam == SIP_NULL)
		{
			if (sip_initSipParam(&((pRejectContact->u).pExtParam), pErr) == SipFail)
				return SipFail;
		}
		if (__sip_cloneSipParam((pRejectContact->u).pExtParam, pExtParam, pErr) == SipFail)
			return SipFail;
	
#endif
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setExtParamInRejectContactParam");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getDispNameFromRejectContactHdr
**
** DESCRIPTION: This function retrives the dispaly pName from a SIP
**		Reject-Contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_getDispNameFromRejectContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppDispName, SipError *pErr)
#else
	(pHdr, ppDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppDispName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempDispName;
	SIPDEBUGFN("Entering function sip_ccp_getDispNameFromRejectContactHdr");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
		
	if ((pHdr == SIP_NULL)||(ppDispName == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
#endif
	pTempDispName = ((SipRejectContactHeader *)(pHdr->pHeader))->pDispName;
	if (pTempDispName == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppDispName = pTempDispName;	
#else
	*ppDispName = (SIP_S8bit *)STRDUPACCESSOR(pTempDispName);
	if (*ppDispName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getDispNameFromRejectContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_setDispNameInRejectContactHdr
**
** DESCRIPTION: This function sets the Display pName in filed in a SIP
**		Reject Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setDispNameInRejectContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pDispName, SipError *pErr)
#else
	(pHdr, pDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pDispName;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempDispName;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setDispNameInRejectContactHdr");        
#ifndef SIP_NO_CHECK
    if( pErr == SIP_NULL )
        return SipFail;

	if ( pHdr == SIP_NULL)
    {
		*pErr = E_INV_PARAM;
			return SipFail;
	}

	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
    }
	
    if (pHdr->dType != SipHdrTypeRejectContact)
    {
		*pErr = E_INV_TYPE;
		return SipFail;
    }
#endif
	if ( ((SipRejectContactHeader *)(pHdr->pHeader))->pDispName != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(((SipRejectContactHeader *)\
			(pHdr->pHeader))->pDispName), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	((SipRejectContactHeader *)(pHdr->pHeader))->pDispName = pDispName;
#else        
	if( pDispName == SIP_NULL)
         pTempDispName = SIP_NULL;
	else
	{
		pTempDispName = (SIP_S8bit *) STRDUPACCESSOR(pDispName);
		if (pTempDispName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	((SipRejectContactHeader *)(pHdr->pHeader))->pDispName = pTempDispName;
#endif	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setDispNameInRejectContactHdr");
        return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getAddrSpecFromRejectContactHdr
**
** DESCRIPTION: This function retrives the dAddr-spec field from a SIP
**		Reject-Contact Header
**
**********************************************************************/
SipBool sip_ccp_getAddrSpecFromRejectContactHdr 
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#else
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
#endif
{
	SipAddrSpec *pTempAddrSpec;
	SIPDEBUGFN("Entering function sip_ccp_getAddrSpecFromRejectContactHdr");
#ifndef SIP_NO_CHECK	
 	if (pErr == SIP_NULL)
 		return SipFail;
 		
 	if (pHdr == SIP_NULL)
 	{
 		*pErr = E_INV_PARAM;
 	 	return SipFail;
 	}

#ifdef SIP_BY_REFERENCE
	if (ppAddrSpec == SIP_NULL)
#else
	if (pAddrSpec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}


 	if (pHdr->dType != SipHdrTypeRejectContact)
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
	
 	if (pHdr->pHeader == SIP_NULL)
 	{
 	 	*pErr = E_INV_HEADER;
 	 	return SipFail;
 	}
#endif

 	pTempAddrSpec = ((SipRejectContactHeader *)(pHdr ->pHeader))->pAddrSpec;

 	if (pTempAddrSpec == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempAddrSpec->dRefCount);
	*ppAddrSpec = pTempAddrSpec;
#else
	if (__sip_cloneSipAddrSpec(pAddrSpec, pTempAddrSpec, pErr) == SipFail)
	{
		if(pAddrSpec->dType==SipAddrReqUri)
		{
			sip_freeString((pAddrSpec->u).pUri);
			(pAddrSpec->u).pUri = SIP_NULL;
		}
		else if((pAddrSpec->dType==SipAddrSipUri)\
				|| (pAddrSpec->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl((pAddrSpec->u).pSipUrl);
			(pAddrSpec->u).pSipUrl = SIP_NULL;
		}
		pAddrSpec->dType=SipAddrAny;	
		return SipFail;
	}
#endif
 	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getAddrSpecFromRejectContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION: sip_ccp_setAddrSpecInRejectContactHd 
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP Reject
**		contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_setAddrSpecInRejectContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTempAddrSpec;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setAddrSpecInRejectContactHdr"); 
#ifndef SIP_NO_CHECK
 	if (pErr == SIP_NULL)
 		return SipFail;
 		
 	if (pHdr == SIP_NULL)
 	{
 		*pErr = E_INV_PARAM;
 	 	return SipFail;
 	}
 	
 	if (pHdr->pHeader == SIP_NULL)
 	{
 	 	*pErr = E_INV_HEADER;
 	 	return SipFail;
 	}
	
 	if (pHdr->dType != SipHdrTypeRejectContact)
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
 	
#endif
 	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipRejectContactHeader *)(pHdr ->pHeader))->pAddrSpec);
 		((SipRejectContactHeader *)(pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipRejectContactHeader *)(pHdr ->pHeader))->pAddrSpec);
		((SipRejectContactHeader *)(pHdr ->pHeader))->pAddrSpec = pAddrSpec;
#else
		if(sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipRejectContactHeader *)(pHdr ->pHeader))->pAddrSpec);
		((SipRejectContactHeader *)(pHdr ->pHeader))->pAddrSpec = pTempAddrSpec;
#endif
 	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setAddrSpecInRejectContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_getRejectContactParamCountFromRejectContactHdr
**
** DESCRIPTION: This function retrives the number of reject-contact
**		pParam inb a reject-contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_getRejectContactParamCountFromRejectContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_getRejectParamCountFromRejectContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	
	if ((pHdr == SIP_NULL)||(pCount==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if (sip_listSizeOf( &(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getRejectParamCountFromRejectContactHdr");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr
**
** DESCRIPTION: This function retrivea a reject-contact param at a 
**		specified index in a SIP Reject-contact pHeader.
**
**********************************************************************/
SipBool sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr 
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipRejectContactParam **ppRejectContactParam, SIP_U32bit index, SipError *pErr)
#else
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppRejectContactParam, index, pErr)
	SipHeader *pHdr;
	SipRejectContactParam **ppRejectContactParam;
	SIP_U32bit index;
	SipError *pErr;
#else
	(pHdr, pRejectContactParam, index, pErr)
	SipHeader *pHdr;
	SipRejectContactParam *pRejectContactParam;
	SIP_U32bit index;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipRejectContactParam *pTempRejectContactParam;
		
	SIPDEBUGFN("Entering function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");		
#ifndef SIP_NO_CHECK
	if(  pErr == SIP_NULL  )
		return SipFail;
	
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
#ifdef SIP_BY_REFERENCE
	if (ppRejectContactParam == SIP_NULL)
#else
	if (pRejectContactParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
		
	if (pHdr->pHeader == SIP_NULL)
    {
         *pErr = E_INV_HEADER;
         return SipFail;
     }
	 
#endif
	 
    if ( sip_listGetAt( &(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), index, &element_from_list, pErr) == SipFail)
		return SipFail;
	
	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	pTempRejectContactParam = (SipRejectContactParam *)element_from_list;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempRejectContactParam->dRefCount);
	*ppRejectContactParam = pTempRejectContactParam;	
#else
	if (__sip_ccp_cloneSipRejectContactParam(pRejectContactParam, pTempRejectContactParam, pErr) == SipFail)
	{
		switch (pRejectContactParam->dType)
		{
			case	SipRejContactTypeExt	:sip_freeSipParam((pRejectContactParam->u).pExtParam);
							 (pRejectContactParam->u).pExtParam = SIP_NULL;
							break;

			case	SipRejContactTypeQvalue	:fast_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid)((pRejectContactParam->u).pQvalue), pErr);
							(pRejectContactParam->u).pQvalue = SIP_NULL;
							break;


			case	SipRejContactTypeAny	:break;

			default				:*pErr = E_INV_TYPE;
							 return SipFail;
		}
		pRejectContactParam->dType = SipRejContactTypeAny;	
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getRejectParamAtIndexFromRejectContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_insertRejectContactParamAtIndexInRejectContactHd
**
** DESCRIPTION: This function inserts  a reject-contact param at a 
**		specified index in a SIP Reject contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_insertRejectContactParamAtIndexInRejectContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, pRejectContactParam, index, pErr)
	SipHeader *pHdr;
	SipRejectContactParam *pRejectContactParam;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SipRejectContactParam *pTempRejectContactParam;
	SIPDEBUGFN("Entering function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( pRejectContactParam == SIP_NULL )
		pTempRejectContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pRejectContactParam->dRefCount);
		pTempRejectContactParam = pRejectContactParam;
#else
		if (sip_ccp_validateSipRejectContactType(pRejectContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (sip_ccp_initSipRejectContactParam(&pTempRejectContactParam, pRejectContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (__sip_ccp_cloneSipRejectContactParam(pTempRejectContactParam, pRejectContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
			return SipFail;
		}
#endif
	}
			
	if( sip_listInsertAt( &(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), index, (SIP_Pvoid)(pTempRejectContactParam), pErr) == SipFail)
	{
		if (pTempRejectContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pRejectContactParam->dRefCount);
#else
			sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
#endif
		}
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_insertRejectParamAtIndexInRejectContactHdr");
	return SipSuccess;
}

	
/**********************************************************************
**
** FUNCTION:  sip_ccp_setRejectContactParamAtIndexInRejectContactHdr
**
** DESCRIPTION: This function sets a reject-contact param at a specified
** 		index in a SIP reject-contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_setRejectContactParamAtIndexInRejectContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, pRejectContactParam, index, pErr)
	SipHeader *pHdr;
	SipRejectContactParam *pRejectContactParam;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SipRejectContactParam *pTempRejectContactParam;
	SIPDEBUGFN("Entering function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
#endif
	if ( pRejectContactParam == SIP_NULL )
		pTempRejectContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pRejectContactParam->dRefCount);
		pTempRejectContactParam = pRejectContactParam;
#else
		if (sip_ccp_validateSipRejectContactType(pRejectContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (sip_ccp_initSipRejectContactParam(&pTempRejectContactParam, pRejectContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (__sip_ccp_cloneSipRejectContactParam(pTempRejectContactParam, pRejectContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
			return SipFail;
		}
#endif
	}
			
	if( sip_listSetAt( &(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), index, (SIP_Pvoid)(pTempRejectContactParam), pErr) == SipFail)
	{
		if (pTempRejectContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pRejectContactParam->dRefCount);
#else
			sip_ccp_freeSipRejectContactParam (pTempRejectContactParam); 
#endif
		}
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setRejectParamAtIndexInRejectContactHdr");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr
**
** DESCRIPTION: This function deletes a Reject-Contact param at a
**		specified index in the Reject-Contact Header structure
**
**********************************************************************/
SipBool sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, index, pErr)
	SipHeader *pHdr;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
		
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRejectContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(((SipRejectContactHeader *)(pHdr->pHeader))->slRejectContactParams), index, pErr) == SipFail)
		return SipFail;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_deleteRejectParamAtIndexInRejectContactHeader");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getAddrSpecFromAcceptContactHdr
**
** DESCRIPTION: This function gets the dAddr-spec field from an accept
**		contact pHeader
**
**********************************************************************/
SipBool sip_ccp_getAddrSpecFromAcceptContactHdr 
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec; 
	SipError *pErr;
#else
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec; 
	SipError *pErr;
#endif
#endif

{
	SipAddrSpec *pTempAddrSpec;
	SIPDEBUGFN("Entering function sip_ccp_getAddrSpecFromAcceptContactHdr");
#ifndef SIP_NO_CHECK	
 	if (pErr == SIP_NULL)
 		return SipFail;

#ifdef SIP_BY_REFERENCE
	if (ppAddrSpec == SIP_NULL)
#else
	if (pAddrSpec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
 	if (pHdr == SIP_NULL)
 	{
 		*pErr = E_INV_PARAM;
 	 	return SipFail;
 	}
 	
 	if (pHdr->dType != SipHdrTypeAcceptContact)
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
	
 	if (pHdr->pHeader == SIP_NULL)
 	{
 	 	*pErr = E_INV_HEADER;
 	 	return SipFail;
 	}
#endif
 	
 	pTempAddrSpec = ((SipAcceptContactHeader *)(pHdr ->pHeader))->pAddrSpec;
 	if (pTempAddrSpec == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempAddrSpec->dRefCount);
	*ppAddrSpec = pTempAddrSpec;
	
#else
	if (__sip_cloneSipAddrSpec(pAddrSpec, pTempAddrSpec, pErr) == SipFail)
	{
		if(pAddrSpec->dType==SipAddrReqUri)
		{
			sip_freeString((pAddrSpec->u).pUri);
			(pAddrSpec->u).pUri = SIP_NULL;
		}
		else if((pAddrSpec->dType==SipAddrSipUri) \
				|| (pAddrSpec->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl((pAddrSpec->u).pSipUrl);
			(pAddrSpec->u).pSipUrl = SIP_NULL;
		}
		pAddrSpec->dType=SipAddrAny;	
		return SipFail;
	}
#endif
 	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getAddrSpecFromAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION: sip_ccp_setAddrSpecInAcceptContactHdr 
**
** DESCRIPTION: This function sets teh dAddr-spec field in a SIp accept
**		contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setAddrSpecInAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTempAddrSpec;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setAddrSpecInAcceptContactHdr"); 
#ifndef SIP_NO_CHECK
 	if (pErr == SIP_NULL)
 		return SipFail;
 		
 	if (pHdr == SIP_NULL)
 	{
 		*pErr = E_INV_PARAM;
 	 	return SipFail;
 	}
 	
 	if (pHdr->dType != SipHdrTypeAcceptContact)
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
	
 	if (pHdr->pHeader == SIP_NULL)
 	{
 	 	*pErr = E_INV_HEADER;
 	 	return SipFail;
 	}
#endif
 	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipAcceptContactHeader *)(pHdr ->pHeader))->pAddrSpec);
 		((SipAcceptContactHeader *)(pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipAcceptContactHeader *)(pHdr ->pHeader))->pAddrSpec);
 		((SipAcceptContactHeader *)(pHdr ->pHeader))->pAddrSpec = pAddrSpec;
#else
		if(sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipAcceptContactHeader *)(pHdr ->pHeader))->pAddrSpec);
		((SipAcceptContactHeader *)(pHdr ->pHeader))->pAddrSpec = pTempAddrSpec;
#endif
 	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setAddrSpecInAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_getDispNameFromAcceptContactHdr
**
** DESCRIPTION: This function retrieves the display pName field from a 
**		SIP accept-contact pHeader
**
**********************************************************************/
SipBool sip_ccp_getDispNameFromAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppDispName, SipError *pErr)
#else
	(pHdr, ppDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppDispName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempDispName;
	SIPDEBUGFN("Entering function sip_ccp_getDispNameFromAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
		
	if ((pHdr == SIP_NULL)||(ppDispName == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
#endif
	pTempDispName = ((SipAcceptContactHeader *)(pHdr->pHeader))->pDispName;
	if (pTempDispName == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE 
	*ppDispName = pTempDispName;
#else
	*ppDispName = (SIP_S8bit *)STRDUPACCESSOR(pTempDispName);
	if (*ppDispName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getDispNameFromAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION: sip_ccp_setDispNameInAcceptContactHdr 
**
** DESCRIPTION: This function sets the display pName field in a SIP
**		accept-contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setDispNameInAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pDispName, SipError *pErr)
#else
	(pHdr, pDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pDispName;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempDispName;
#endif
	SIPDEBUGFN("Entering function sip_ccp_setDispNameInAcceptContactHdr");
#ifndef SIP_NO_CHECK        
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL)
        {
			*pErr = E_INV_PARAM;
			return SipFail;
        }

        if (pHdr->dType != SipHdrTypeAcceptContact)
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
		
        if (pHdr->pHeader == SIP_NULL) 
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
	if (((SipAcceptContactHeader *)(pHdr->pHeader))->pDispName != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(((SipAcceptContactHeader *)\
			(pHdr->pHeader))->pDispName), pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipAcceptContactHeader *)(pHdr->pHeader))->pDispName = pDispName;
#else
    if( pDispName == SIP_NULL)
             pTempDispName = SIP_NULL;
    else
    {
		pTempDispName = (SIP_S8bit *) STRDUPACCESSOR(pDispName);
		if (pTempDispName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}	
   	((SipAcceptContactHeader *)(pHdr->pHeader))->pDispName = pTempDispName;

#endif	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setDispNameInAcceptContactHdr");
        return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_getAcceptContactParamCountFromAcceptContactHdr
**
** DESCRIPTION: This function retrives the nember of accept-contact
**		pParam from a SIP Accept-Conatct pHeader
**
**********************************************************************/
SipBool sip_ccp_getAcceptContactParamCountFromAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	
	if (( pHdr == SIP_NULL )||(pCount==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
#endif	
	if (sip_listSizeOf( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getAcceptContactParamCountFromAcceptContactHdr");
	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr
**
** DESCRIPTION: This function retrives an accept-contact param at a
**		specified index from a SIP accept Conatct pHeader
**
**********************************************************************/
SipBool sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr 
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAcceptContactParam **ppAcceptContactParam, SIP_U32bit index, SipError *pErr)
#else
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppAcceptContactParam, index, pErr)
	SipHeader *pHdr;
	SipAcceptContactParam **ppAcceptContactParam;
	SIP_U32bit index;
	SipError *pErr;
#else
	(pHdr, pAcceptContactParam, index, pErr)
	SipHeader *pHdr;
	SipAcceptContactParam *pAcceptContactParam;
	SIP_U32bit index;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipAcceptContactParam *pTempAcceptContactParam;
		
	SIPDEBUGFN("Entering function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");		
#ifndef SIP_NO_CHECK
	if(  pErr == SIP_NULL  )
		return SipFail;
	
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
#ifdef SIP_BY_REFERENCE
	if (ppAcceptContactParam == SIP_NULL)
#else
	if (pAcceptContactParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	
    if ( sip_listGetAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), index, &element_from_list, pErr) == SipFail)
		return SipFail;
	
	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	pTempAcceptContactParam = (SipAcceptContactParam *)element_from_list;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempAcceptContactParam->dRefCount);
	*ppAcceptContactParam = pTempAcceptContactParam;
#else

	if (__sip_ccp_cloneSipAcceptContactParam(pAcceptContactParam, pTempAcceptContactParam, pErr) == SipFail)
	{
		switch (pAcceptContactParam->dType)
		{
			case	SipAccContactTypeExt	:sip_freeSipParam((pAcceptContactParam->u).pExtParam);
							 (pAcceptContactParam->u).pExtParam = SIP_NULL;
							break;

			case	SipAccContactTypeQvalue	:fast_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid)((pAcceptContactParam->u).pQvalue), pErr);
							(pAcceptContactParam->u).pQvalue = SIP_NULL;
							break;


			case	SipAccContactTypeAny	:break;

			default				:*pErr = E_INV_TYPE;
							 return SipFail;
		}
		pAcceptContactParam->dType = SipAccContactTypeAny;	
		return SipFail;
	}
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION: sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr 
**
** DESCRIPTION: This fuunction sets an accept-contact param at a
**		specified index in the Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, pAcceptContactParam, index, pErr)
	SipHeader *pHdr;
	SipAcceptContactParam *pAcceptContactParam;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SipAcceptContactParam *pTempAcceptContactParam;
	SIPDEBUGFN("Entering function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
#endif
	if ( pAcceptContactParam == SIP_NULL )
		pTempAcceptContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAcceptContactParam->dRefCount);
		pTempAcceptContactParam = pAcceptContactParam;	
#else

		if (sip_ccp_validateSipAcceptContactType(pAcceptContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (sip_ccp_initSipAcceptContactParam(&pTempAcceptContactParam, pAcceptContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (__sip_ccp_cloneSipAcceptContactParam(pTempAcceptContactParam, pAcceptContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
			return SipFail;
		}
#endif
	}
			
	if( sip_listSetAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), index, (SIP_Pvoid)(pTempAcceptContactParam), pErr) == SipFail)
	{
		if (pTempAcceptContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pAcceptContactParam->dRefCount);
#else
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
#endif
		}
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr
**
** DESCRIPTION: This function inserts an accept-contact param at a
**		specified index in the Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, pAcceptContactParam, index, pErr)
	SipHeader *pHdr;
	SipAcceptContactParam *pAcceptContactParam;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SipAcceptContactParam *pTempAcceptContactParam;
	SIPDEBUGFN("Entering function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( pAcceptContactParam == SIP_NULL )
		pTempAcceptContactParam = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAcceptContactParam->dRefCount);
		pTempAcceptContactParam = pAcceptContactParam;	
#else
		if (sip_ccp_validateSipAcceptContactType(pAcceptContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (sip_ccp_initSipAcceptContactParam(&pTempAcceptContactParam, pAcceptContactParam->dType, pErr) == SipFail)
			return SipFail;
		if (__sip_ccp_cloneSipAcceptContactParam(pTempAcceptContactParam, pAcceptContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
			return SipFail;
		}
#endif
	}
			
	if( sip_listInsertAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), index, (SIP_Pvoid)(pTempAcceptContactParam), pErr) == SipFail)
	{
		if (pTempAcceptContactParam != SIP_NULL)
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pAcceptContactParam->dRefCount);
#else
			sip_ccp_freeSipAcceptContactParam (pTempAcceptContactParam); 
#endif
		}

		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr
**
** DESCRIPTION: This function deletes an accept-contact param at a
**		specified index in the SIP Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit index, SipError *pErr)
#else
	(pHdr, index, pErr)
	SipHeader *pHdr;
	SIP_U32bit index;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
		
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeAcceptContact)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipAcceptContactHeader *)(pHdr->pHeader))->slAcceptContactParams), index, pErr) == SipFail)
		return SipFail;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr");
	return SipSuccess;
}
#endif

/**********************************************************************
**
** FUNCTION:  sip_ccp_getFeatureFromReqDispHd
**
** DESCRIPTION: This functionm retrieves the feature field from a
**		SIP Request-Disposition pHeader structure
**
**********************************************************************/
SipBool sip_ccp_getFeatureFromReqDispHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppFeature, SipError *pErr )
#else
	(pHdr, ppFeature, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppFeature;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempFeature;
	SIPDEBUGFN("Entering function sip_ccp_getFeatureFromReqDispHdr");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
		
	if ((pHdr == SIP_NULL)||(ppFeature == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if (pHdr->dType != SipHdrTypeRequestDisposition)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTempFeature = ((SipRequestDispositionHeader *)(pHdr->pHeader))->pFeature;
	if (pTempFeature == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppFeature = pTempFeature;
#else
	*ppFeature = (SIP_S8bit *)STRDUP(pTempFeature);
	if (*ppFeature == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_getFeatureFromReqDispHdr");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_setFeatureInReqDispHdr
**
** DESCRIPTION: This function sets the feature field in the SIP
**		Request-Dispositioon pHeader
**
**********************************************************************/
SipBool sip_ccp_setFeatureInReqDispHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pFeature, SipError *pErr )
#else
	(pHdr, pFeature, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pFeature;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempFeature;
#endif
	SIP_S8bit *pFeat;
	SIPDEBUGFN("Entering function sip_ccp_setFeatureInReqDispHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pHdr->dType != SipHdrTypeRequestDisposition)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if (pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
    pFeat = ((SipRequestDispositionHeader *)(pHdr->pHeader))->pFeature;
	if ( pFeat != SIP_NULL)
	{
		if (sip_memfree(0, (SIP_Pvoid*)(&pFeat), pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipRequestDispositionHeader *)(pHdr->pHeader))->pFeature = pFeature;
#else    
    if( pFeature == SIP_NULL)
        pTempFeature = SIP_NULL;
    else
        {
		pTempFeature = (SIP_S8bit *) STRDUP(pFeature);
		if (pTempFeature == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}	
     
   ((SipRequestDispositionHeader *)(pHdr->pHeader))->pFeature = pTempFeature;
#endif   
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_setFeatureInReqDispHdr");
        return SipSuccess;
}
