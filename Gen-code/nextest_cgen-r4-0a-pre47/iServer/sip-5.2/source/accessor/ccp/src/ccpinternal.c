 /******************************************************************************
 ** FUNCTION:
 **	 	This file has all the internal functions (clones/validate
 ** 		functions of the SIP Caller & callee Preferences  draft
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccpinternal.c
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

#include "ccpinternal.h"
#include "sipclone.h"
#include "sipparserinc.h"

#ifdef SIP_CCP_VERSION10

/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipAcceptContactHeader
**
** DESCRIPTION: This function duplicates the Accept Contact Header 
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipAcceptContactHeader 
#ifdef ANSI_PROTO
	(SipAcceptContactHeader *pDest, SipAcceptContactHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipAcceptContactHeader *pDest;
	SipAcceptContactHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_Pvoid pDummy=SIP_NULL;
	SIP_U32bit dCount=0, dIndex=0;
	SipAcceptContactParam *pCloneAcceptContactParam=SIP_NULL, 
	                       *pTempAcceptContactParam=SIP_NULL;

	SIPDEBUGFN("Entering function __sip_ccp_cloneSipAcceptContactHeader");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pErr = E_NO_ERROR;
	/* clear the destination Reject contact pHeader structure */
	sip_listDeleteAll(&(pDest->slAcceptContactParams), pErr);
	/* copy display pName */
	/* copying slRejectContactParams */
	if ( sip_listSizeOf (&(pSource->slAcceptContactParams), &dCount, \
							pErr) == SipFail )
	{
			SIPDEBUGFN("Exiting function __sip_ccp_cloneSipAcceptContactHeader");
			return SipFail;
	}
	for (dIndex = 0; dIndex < dCount; dIndex ++)
	{
		if (sip_listGetAt(&(pSource->slAcceptContactParams), dIndex, \
								&pDummy, pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function __sip_ccp_cloneSipAcceptContactHeader");
				return SipFail;
		}
		/* clone the accept-contact param */
		pTempAcceptContactParam = (SipAcceptContactParam *)pDummy;
		if (pTempAcceptContactParam == SIP_NULL)
			pCloneAcceptContactParam = SIP_NULL;
		else
		{
			if (sip_ccp_initSipAcceptContactParam(&pCloneAcceptContactParam,\
									pTempAcceptContactParam->dType, pErr) == SipFail)
			{
					SIPDEBUGFN("Exiting function __sip_ccp_cloneSipAcceptContactHeader");
					return SipFail;
			}
			if (__sip_ccp_cloneSipAcceptContactParam(pCloneAcceptContactParam, 
									pTempAcceptContactParam, pErr) == SipFail)	
			{
				sip_ccp_freeSipAcceptContactParam (pCloneAcceptContactParam);
				SIPDEBUGFN("Exiting function __sip_ccp_cloneSipAcceptContactHeader");
		 	 	return SipFail;
			}
		}/* end of else */
		/* add the accept-contact param to the destination structure */
		if (sip_listAppend(&(pDest->slAcceptContactParams), (SIP_Pvoid)pCloneAcceptContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipAcceptContactParam(pCloneAcceptContactParam);
			SIPDEBUGFN("Exiting function __sip_ccp_cloneSipAcceptContactHeader");
			return SipFail;
		}
	} /* end of for */
	SIPDEBUGFN("Exiting function __sip_ccp_cloneSipAcceptContactHeader");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipRejectContactHeader
**
** DESCRIPTION: This function duplicates the Sipreject Contact Header
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipRejectContactHeader 
#ifdef ANSI_PROTO
	(SipRejectContactHeader *pDest, SipRejectContactHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRejectContactHeader *pDest;
	SipRejectContactHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_Pvoid pDummy=SIP_NULL;
	SIP_U32bit dCount, dIndex;
	SipRejectContactParam *pCloneRejectContactParam=SIP_NULL, 
	                      *pTempRejectContactParam=SIP_NULL;

	SIPDEBUGFN("Entering function __sip_ccp_cloneSipRejectContactHeader");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactHeader");
		return SipFail;
	}
#endif
	/* clear the destination Reject contact pHeader structure */
	sip_listDeleteAll(&(pDest->slRejectContactParams), pErr);
	/* copy display pName */
	/* copying slRejectContactParams */
	if ( sip_listSizeOf (&(pSource->slRejectContactParams), &dCount, \
							pErr) == SipFail )
	{
			SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactHeader");
			return SipFail;
	}
	for (dIndex = 0; dIndex < dCount; dIndex ++)
	{
		if (sip_listGetAt(&(pSource->slRejectContactParams), dIndex,\
								&pDummy, pErr) == SipFail)
		{
				SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactHeader");
				return SipFail;
		}
		pTempRejectContactParam = (SipRejectContactParam *)pDummy;
		if (pTempRejectContactParam == SIP_NULL)
			pCloneRejectContactParam = SIP_NULL;
		else
		{
			if (sip_ccp_initSipRejectContactParam(&pCloneRejectContactParam,\
									pTempRejectContactParam->dType, pErr) == SipFail)
			{
					SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactHeader");
					return SipFail;
			}
			if (__sip_ccp_cloneSipRejectContactParam(pCloneRejectContactParam, 
									pTempRejectContactParam, pErr) == SipFail)	
			{
				sip_ccp_freeSipRejectContactParam (pCloneRejectContactParam);
				SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactHeader");
		 	 	return SipFail;
			}
		}/* end of else */
		if (sip_listAppend(&(pDest->slRejectContactParams), (SIP_Pvoid)pCloneRejectContactParam, pErr) == SipFail)
		{
			sip_ccp_freeSipRejectContactParam(pCloneRejectContactParam);
			SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactHeader");
			return SipFail;
		}
	} /* end of for */
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactHeader");
	return SipSuccess;
}

#else
/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipAcceptContactHeader
**
** DESCRIPTION: This function duplicates the Accept Contact Header 
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipAcceptContactHeader 
#ifdef ANSI_PROTO
	(SipAcceptContactHeader *pDest, SipAcceptContactHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipAcceptContactHeader *pDest;
	SipAcceptContactHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *temp;
	SIP_Pvoid dummy;
	SIP_U32bit index, count;
	SipAcceptContactParam *temp_accept_contact_param, *clone_accept_contact_param;
	SIPDEBUGFN("Entering function __sip_ccp_cloneSipAcceptContactHeader");
	/* clear destination accept-contact pHeader structure */
	if (pDest->pDispName != SIP_NULL)
	{
		sip_freeString(pDest->pDispName);
		pDest->pDispName = SIP_NULL;
	}
	if (pDest->pAddrSpec != SIP_NULL)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpec);
		pDest->pAddrSpec = SIP_NULL;
	}
	sip_listDeleteAll(&(pDest->slAcceptContactParams), pErr);
	/* copy display pName */
	if (pSource->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pDispName);
		if (temp == SIP_NULL)
		{
			*pErr = E_NO_ERROR;
			return SipFail;
		}
	}
	pDest->pDispName = temp;
	/* copying pAddrSpec */
	if (pSource->pAddrSpec == SIP_NULL)
		pDest->pAddrSpec = SIP_NULL;
	else
	{
		if (sip_initSipAddrSpec(&(pDest->pAddrSpec), SipAddrAny, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(pDest->pAddrSpec, pSource->pAddrSpec, pErr) == SipFail)
			return SipFail;
	}

	/* copying slAcceptContactParams */
	if ( sip_listSizeOf (&(pSource->slAcceptContactParams), &count, pErr) == SipFail )
			return SipFail;
	for (index = 0; index < count; index ++)
	{
		if (sip_listGetAt(&(pSource->slAcceptContactParams), index, &dummy, pErr) == SipFail)
			return SipFail;
		/* clone the accept-contact param */
		temp_accept_contact_param = (SipAcceptContactParam *)dummy;
		if (temp_accept_contact_param == SIP_NULL)
			clone_accept_contact_param = SIP_NULL;
		else
		{
			if (sip_ccp_initSipAcceptContactParam(&clone_accept_contact_param, temp_accept_contact_param->dType, pErr) == SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipAcceptContactParam(clone_accept_contact_param, temp_accept_contact_param, pErr) == SipFail)	{
				sip_ccp_freeSipAcceptContactParam(clone_accept_contact_param);
		 	 	return SipFail;
			}
		}/* end of else */
		if (sip_listAppend(&(pDest->slAcceptContactParams), (SIP_Pvoid)clone_accept_contact_param, pErr) == SipFail)
		{
			sip_ccp_freeSipAcceptContactParam(clone_accept_contact_param);
			return SipFail;
		}
	} /* end of for */
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_ccp_cloneSipAcceptContactHeader");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipRejectContactHeader
**
** DESCRIPTION: This function duplicates the Sipreject Contact Header
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipRejectContactHeader 
#ifdef ANSI_PROTO
	(SipRejectContactHeader *pDest, SipRejectContactHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRejectContactHeader *pDest;
	SipRejectContactHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *temp;
	SIP_Pvoid dummy;
	SIP_U32bit count, index;
	SipRejectContactParam *clone_reject_contact_param, *temp_reject_contact_param;
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function __sip_ccp_cloneSipRejectContactHeader");
	/* clear the destination Reject contact pHeader structure */
	if (pDest->pDispName != SIP_NULL)
	{
		sip_freeString(pDest->pDispName);
		pDest->pDispName = SIP_NULL;
	}
	if (pDest->pAddrSpec != SIP_NULL)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpec);
		pDest->pAddrSpec = SIP_NULL;
	}
	sip_listDeleteAll(&(pDest->slRejectContactParams), pErr);
	/* copy display pName */
	if (pSource->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pDispName);
		if (temp == SIP_NULL)
		{
			*pErr = E_NO_ERROR;
			return SipFail;
		}
	}
	pDest->pDispName = temp;
	/* copying pAddrSpec */
	if (pSource->pAddrSpec == SIP_NULL)
		pDest->pAddrSpec = SIP_NULL;
	else
	{
		if (sip_initSipAddrSpec(&(pDest->pAddrSpec), SipAddrAny, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(pDest->pAddrSpec, pSource->pAddrSpec, pErr) == SipFail)
			return SipFail;
	}
	
	/* copying slRejectContactParams */
	if ( sip_listSizeOf (&(pSource->slRejectContactParams), &count, pErr) == SipFail )
			return SipFail;
	for (index = 0; index < count; index ++)
	{
		if (sip_listGetAt(&(pSource->slRejectContactParams), index, &dummy, pErr) == SipFail)
			return SipFail;
		/* clone the accept-contact param */
		temp_reject_contact_param = (SipRejectContactParam *)dummy;
		if (temp_reject_contact_param == SIP_NULL)
			clone_reject_contact_param = SIP_NULL;
		else
		{
			if (sip_ccp_initSipRejectContactParam(&clone_reject_contact_param, temp_reject_contact_param->dType, pErr) == SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipRejectContactParam(clone_reject_contact_param, temp_reject_contact_param, pErr) == SipFail)	{
				sip_ccp_freeSipRejectContactParam (clone_reject_contact_param);
		 	 	return SipFail;
			}
		}/* end of else */
		/* add the accept-contact param to the destination structure */
		if (sip_listAppend(&(pDest->slRejectContactParams), (SIP_Pvoid)clone_reject_contact_param, pErr) == SipFail)
		{
			sip_ccp_freeSipRejectContactParam(clone_reject_contact_param);
			return SipFail;
		}
	} /* end of for */
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_ccp_cloneSipRejectContactHeader");
	return SipSuccess;
}

#endif

/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipRequestDispositionHeader
**
** DESCRIPTION: This function duplicates the SIP Request Disposition
**		pHeader structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipRequestDispositionHeader 
#ifdef ANSI_PROTO
	(SipRequestDispositionHeader *pDest, SipRequestDispositionHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRequestDispositionHeader *pDest;
	SipRequestDispositionHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *temp;
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function __sip_ccp_cloneSipRequestDispositionHeader");
	/* clear the destination structure */
	if (pDest->pFeature != SIP_NULL)
	{
		sip_freeString(pDest->pFeature);
		pDest->pFeature = SIP_NULL;
	}
	/* copy the feature */
	if (pSource->pFeature == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUP (pSource->pFeature);
		if (temp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pFeature = temp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_ccp_cloneSipRequestDispositionHeader");
	return SipSuccess;
}
#ifdef SIP_CCP_VERSION10
/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipAcceptContactParam
**
** DESCRIPTION: This fuunction duplicates the Sip Accept Contact param
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pDest, SipAcceptContactParam *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipAcceptContactParam *pDest;
	SipAcceptContactParam *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp=SIP_NULL;
	
	SIPDEBUGFN("Entering function __sip_ccp_cloneSipAcceptContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* clear the destination accept-contact param structure */
	switch (pDest->dType)
	{
		case 	SipAccContactTypeFeature	:
		case 	SipAccContactTypeGeneric	:
								sip_freeSipParam((pDest->u).pParam);
								(pDest->u).pParam = SIP_NULL;
								break;
		case 	SipAccContactTypeOther	:
								sip_freeString((pDest->u).pToken);
								break ;
		default				:
		            break;
	}
	/* copy dType */
	pDest->dType = pSource->dType;
	/* based on dType dupliacte the union */
	switch (pSource->dType)
	{
		case	SipAccContactTypeGeneric :
		case SipAccContactTypeFeature  :
		         if (sip_initSipParam(&((pDest->u).pParam), pErr) == SipFail)
								return SipFail;
						 if (__sip_cloneSipParam((pDest->u).pParam, 
								 (pSource->u).pParam, pErr) == SipFail)
							return SipFail;
						 break;

		case	SipAccContactTypeOther	:
		         if ((pSource->u).pToken == SIP_NULL)
							pTemp = SIP_NULL;
						 else
						 {
							pTemp = (SIP_S8bit *)STRDUPACCESSOR((pSource->u).pToken);
							if (pTemp == SIP_NULL)
							{
								*pErr = E_NO_MEM;
								return SipFail;
							}
						 }
						 (pDest->u).pToken = pTemp;
						 break;

		case	SipAccContactTypeAny	:
		         *pErr = E_INV_PARAM;
					   return SipFail;
		default				: 
		         *pErr = E_INV_TYPE;
						 return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_ccp_cloneSipAcceptContactParam");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipRejectContactParam
**
** DESCRIPTION: This function duplicates the SIP Reject Contact param
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam *pDest, SipRejectContactParam *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRejectContactParam *pDest;
	SipRejectContactParam *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp=SIP_NULL;
	
	SIPDEBUGFN("Entering function __sip_ccp_cloneSipRejectContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactParam");
		return SipFail;
	}
#endif
	/* clear destination reject-contact param */
	switch (pDest->dType)
	{
		case 	SipRejContactTypeFeature	:
		case	SipRejContactTypeGeneric	:
						if ((pDest->u).pParam != SIP_NULL)
						 	sip_freeSipParam((pDest->u).pParam);
						 (pDest->u).pParam = SIP_NULL;
						 break;

		case 	SipRejContactTypeOther	:
						if ((pDest->u).pToken != SIP_NULL)
						 	sip_freeString((pDest->u).pToken);
						 (pDest->u).pToken = SIP_NULL;
						 break;

		case 	SipRejContactTypeAny	:
						 (pDest->u).pToken = SIP_NULL;
						 (pDest->u).pParam = SIP_NULL;

		default				:
			              break ;
	}
	/* copy the dType */
	pDest->dType = pSource->dType;
	/* based on dType duplicate the union */
	switch (pSource->dType)
	{
		case  SipRejContactTypeFeature	:
		case  SipRejContactTypeGeneric  :
				if ((pSource->u).pParam != SIP_NULL)
				{
		         if (sip_initSipParam(&((pDest->u).pParam), pErr) == SipFail)
						 {
								 SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactParam");
								return SipFail;
						 }
				     if (__sip_cloneSipParam((pDest->u).pParam, 
								 (pSource->u).pParam, pErr) == SipFail)
				     {
								 SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactParam");
								 return SipFail;
				     }
				}
				break ;
		 
		case	SipRejContactTypeOther	:
				if ((pSource->u).pToken == SIP_NULL)
							pTemp = SIP_NULL;
				 else
				 {
						pTemp = (SIP_S8bit *)STRDUPACCESSOR((pSource->u).pToken);
						if (pTemp == SIP_NULL)
						{
							*pErr = E_NO_MEM;
							SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactParam");
							return SipFail;
						}
				 }
				 (pDest->u).pToken = pTemp;
				 break;

		default				: 
				*pErr = E_INV_TYPE;
				SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactParam");
			  return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_ccp_cloneSipRejectContactParam");
	return SipSuccess;
}

/**********************************************************************
** FUNCTION:  sip_ccp_validateSipAcceptContactType
**
** DESCRIPTION: This function validates the Accept Contact Param Type
**
**********************************************************************/
SipBool sip_ccp_validateSipAcceptContactType 
#ifdef ANSI_PROTO
	(en_AcceptContactType dType, SipError *pErr)
#else
	(dType, pErr)
	en_AcceptContactType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_validateSipAcceptContactType");
	switch(dType)
	{
		case	SipAccContactTypeFeature:
		case	SipAccContactTypeGeneric:
		case	SipAccContactTypeOther  :
		case	SipAccContactTypeAny	  : break;
		default				                : *pErr = E_INV_TYPE;
						                        return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_validateSipAcceptContactType");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_validateSipRejectContactType
**
** DESCRIPTION: This function validates the SIP Reject Contact param dType
**
**********************************************************************/
SipBool sip_ccp_validateSipRejectContactType 
#ifdef ANSI_PROTO
	(en_RejectContactType dType, SipError *pErr)	
#else
	(dType, pErr)
	en_RejectContactType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_validateSipRejectContactType");
	switch(dType)
	{
		case	SipRejContactTypeFeature:
		case	SipRejContactTypeGeneric:
		case	SipRejContactTypeOther  :
		case	SipRejContactTypeAny    : break;
		default				                : *pErr = E_INV_TYPE;
						                        return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_ccp_validateSipRejectContactType");
	return SipSuccess;
}

#else
/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipAcceptContactParam
**
** DESCRIPTION: This fuunction duplicates the Sip Accept Contact param
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pDest, SipAcceptContactParam *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipAcceptContactParam *pDest;
	SipAcceptContactParam *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *temp;
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function __sip_ccp_cloneSipAcceptContactParam");
	/* clear the destination accept-contact param structure */
	switch (pDest->dType)
	{
		case 	SipAccContactTypeExt	:sip_freeSipParam((pDest->u).pExtParam);
						 (pDest->u).pExtParam = SIP_NULL;
						 break;
		case	SipAccContactTypeQvalue	:if ((pDest->u).pQvalue != SIP_NULL)
						 	sip_freeString((pDest->u).pQvalue);
						 (pDest->u).pQvalue = SIP_NULL;
						 break;
		case	SipAccContactTypeAny	:break;
		default				:*pErr = E_INV_TYPE;
						 return SipFail;
	}
	/* copy dType */
	pDest->dType = pSource->dType;
	/* based on dType dupliacte the union */
	switch (pSource->dType)
	{
		case	SipAccContactTypeExt	:if (sip_initSipParam(&((pDest->u).pExtParam), pErr) == SipFail)
								return SipFail;
						 if (__sip_cloneSipParam((pDest->u).pExtParam, (pSource->u).pExtParam, pErr) == SipFail)
							return SipFail;
						 break;

		case	SipAccContactTypeQvalue	:if ((pSource->u).pQvalue == SIP_NULL)
							temp = SIP_NULL;
						 else
						 {
							temp = (SIP_S8bit *)STRDUPACCESSOR((pSource->u).pQvalue);
							if (temp == SIP_NULL)
							{
								*pErr = E_NO_MEM;
								return SipFail;
							}
						 }
						 (pDest->u).pQvalue = temp;
						 break;

		case	SipAccContactTypeAny	:*pErr = E_INV_PARAM;
						 return SipFail;

		default				: *pErr = E_INV_TYPE;
						 return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_ccp_cloneSipAcceptContactParam");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_cloneSipRejectContactParam
**
** DESCRIPTION: This function duplicates the SIP Reject Contact param
**		structure
**
**********************************************************************/
SipBool __sip_ccp_cloneSipRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam *pDest, SipRejectContactParam *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRejectContactParam *pDest;
	SipRejectContactParam *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *temp;
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function __sip_ccp_cloneSipRejectContactParam");
	/* clear destination reject-contact param */
	switch (pDest->dType)
	{
		case 	SipRejContactTypeExt	:sip_freeSipParam((pDest->u).pExtParam);
						(pDest->u).pExtParam = SIP_NULL;
						 break;
		
		case	SipRejContactTypeQvalue	:if ((pDest->u).pQvalue != SIP_NULL)
						 	sip_freeString((pDest->u).pQvalue);
						 (pDest->u).pQvalue = SIP_NULL;
						 break;

		case	SipRejContactTypeAny	:break;

		default				:*pErr = E_INV_TYPE;
						 return SipFail;
	}
	/* copy the dType */
	pDest->dType = pSource->dType;
	/* based on dType duplicate the union */
	switch (pSource->dType)
	{
		case	SipRejContactTypeExt	:if (sip_initSipParam(&((pDest->u).pExtParam), pErr) == SipFail)
								return SipFail;
						 if (__sip_cloneSipParam((pDest->u).pExtParam, (pSource->u).pExtParam, pErr) == SipFail)
							return SipFail;
						 break;
		case	SipRejContactTypeQvalue	:if ((pSource->u).pQvalue == SIP_NULL)
							temp = SIP_NULL;
						 else
						 {
							temp = (SIP_S8bit *)STRDUPACCESSOR((pSource->u).pQvalue);
							if (temp == SIP_NULL)
							{
								*pErr = E_NO_MEM;
								return SipFail;
							}
						 }
						 (pDest->u).pQvalue = temp;
						 break;

		case	SipRejContactTypeAny	:*pErr = E_INV_PARAM;
						 return SipFail;

		default				: *pErr = E_INV_TYPE;
						 return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_ccp_cloneSipRejectContactParam");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_validateSipAcceptContactType
**
** DESCRIPTION: This function validates the Accept Contact Param Type
**
**********************************************************************/
SipBool sip_ccp_validateSipAcceptContactType 
#ifdef ANSI_PROTO
	(en_AcceptContactType dType, SipError *pErr)
#else
	(dType, pErr)
	en_AcceptContactType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_validateSipAcceptContactType");
	switch(dType)
	{
		case	SipAccContactTypeExt	:
		case	SipAccContactTypeQvalue	:
		case	SipAccContactTypeAny	:break;
		default				: *pErr = E_INV_TYPE;
						 return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_validateSipAcceptContactType");
	return SipSuccess;
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_validateSipRejectContactType
**
** DESCRIPTION: This function validates the SIP Reject Contact param dType
**
**********************************************************************/
SipBool sip_ccp_validateSipRejectContactType 
#ifdef ANSI_PROTO
	(en_RejectContactType dType, SipError *pErr)	
#else
	(dType, pErr)
	en_RejectContactType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_ccp_validateSipRejectContactType");
	switch(dType)
	{
		case	SipRejContactTypeExt	:
		case	SipRejContactTypeQvalue	:
		case	SipRejContactTypeAny	:break;
		default				: *pErr = E_INV_TYPE;
						 return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_ccp_validateSipRejectContactType");
	return SipSuccess;
}
#endif


