 /******************************************************************************
 ** FUNCTION:
 **	 This file has the source dCodeNum of all the free functions in the
 **	 SIP caller & Callee Preferences draft	
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccpfree.c
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

#include "ccpfree.h"


/**********************************************************************
**
** FUNCTION: sip_ccp_freeSipRequestDispositionHeader 
**
** DESCRIPTION: This function frees a SIP RequestDisposition Header
**		structure
**
**********************************************************************/
void sip_ccp_freeSipRequestDispositionHeader 
#ifdef ANSI_PROTO
	(SipRequestDispositionHeader *pHdr)
#else
	(pHdr)
	SipRequestDispositionHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pFeature);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipRequestDispositionHeader
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipRequestDispositionHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_ccp_freeSipRequestDispositionHeader ((SipRequestDispositionHeader *)pHdr);
}



#ifdef SIP_CCP_VERSION10

/**********************************************************************
**
** FUNCTION:  sip_ccp_freeSipRejectContactHeader
**
** DESCRIPTION: This function frees a SIP Reject Contact pHeader 
**		structure
**
**********************************************************************/
void sip_ccp_freeSipRejectContactHeader
#ifdef ANSI_PROTO
	(SipRejectContactHeader *pHdr)
#else
	(pHdr)
	SipRejectContactHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		sip_listDeleteAll(&(pHdr->slRejectContactParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


/**********************************************************************
**
** FUNCTION: sip_ccp_freeSipAcceptContactParam 
**
** DESCRIPTION: This function frees a SIP Accept-Contact param 
**		structure
**
**********************************************************************/
void sip_ccp_freeSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pAcceptContactParam)
#else
	(pAcceptContactParam)
	SipAcceptContactParam *pAcceptContactParam;
#endif
{
	if (pAcceptContactParam == SIP_NULL) return;
	HSS_LOCKREF(pAcceptContactParam->dRefCount);HSS_DECREF(pAcceptContactParam->dRefCount);
	if(HSS_CHECKREF(pAcceptContactParam->dRefCount))
	{
		switch (pAcceptContactParam->dType)
		{
			case SipAccContactTypeFeature :
			case 	SipAccContactTypeGeneric :
				sip_freeSipParam((pAcceptContactParam->u).pParam);
			  break ;
			case  SipAccContactTypeOther :
				sip_freeString((pAcceptContactParam->u).pToken);
			  break ;
			case 	SipAccContactTypeAny	:
			  break;
		}
		HSS_UNLOCKREF(pAcceptContactParam->dRefCount);
		HSS_DELETEREF(pAcceptContactParam->dRefCount);
		HSS_FREE(pAcceptContactParam);
	}
	else
	{
		HSS_UNLOCKREF(pAcceptContactParam->dRefCount);
	}
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_freeSipAcceptContactHeader
**
** DESCRIPTION: This function frees the SIP accept Contact pHeader 
**		structure
**
**********************************************************************/
void sip_ccp_freeSipAcceptContactHeader 
#ifdef ANSI_PROTO
	(SipAcceptContactHeader *pHdr)
#else
	(pHdr)
	SipAcceptContactHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		sip_listDeleteAll(&(pHdr->slAcceptContactParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_freeSipRejectContactParam
**
** DESCRIPTION: This function frees a SIP Reject ContactParam structure
**
**********************************************************************/
void sip_ccp_freeSipRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam * pRejectContactParam)
#else
	(pRejectContactParam)
	SipRejectContactParam * pRejectContactParam;
#endif
{
	if (pRejectContactParam == SIP_NULL) return;
	HSS_LOCKREF(pRejectContactParam->dRefCount);HSS_DECREF(pRejectContactParam->dRefCount);
	if(HSS_CHECKREF(pRejectContactParam->dRefCount))
	{
		switch (pRejectContactParam->dType)
		{
			case SipRejContactTypeFeature :
			case 	SipRejContactTypeGeneric :
				sip_freeSipParam((pRejectContactParam->u).pParam);
			  break ;
			case  SipRejContactTypeOther :
				sip_freeString((pRejectContactParam->u).pToken);
			  break ;
			case 	SipRejContactTypeAny	:
			  break;
		}
		HSS_UNLOCKREF(pRejectContactParam->dRefCount);
		HSS_DELETEREF(pRejectContactParam->dRefCount);
		HSS_FREE(pRejectContactParam);
	}
	else
	{
		HSS_UNLOCKREF(pRejectContactParam->dRefCount);
	}
}
/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipRejectContactHeader
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipRejectContactHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_ccp_freeSipRejectContactHeader ((SipRejectContactHeader *)pHdr);
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipAcceptContactHeader
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipAcceptContactHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_ccp_freeSipAcceptContactHeader ((SipAcceptContactHeader *)pHdr);
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipRejectContactParam
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipRejectContactParam 
#ifdef ANSI_PROTO
	(SIP_Pvoid pRejectContactParam)
#else
	(pRejectContactParam)
	SIP_Pvoid pRejectContactParam;
#endif
{
	sip_ccp_freeSipRejectContactParam ((SipRejectContactParam *)pRejectContactParam);
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipAcceptContactParam
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SIP_Pvoid pAcceptContactParam)
#else
	(pAcceptContactParam)
	SIP_Pvoid pAcceptContactParam;
#endif
{
	sip_ccp_freeSipAcceptContactParam ((SipAcceptContactParam *)pAcceptContactParam);
}

#else
/**********************************************************************
**
** FUNCTION:  sip_ccp_freeSipRejectContactHeader
**
** DESCRIPTION: This function frees a SIP Reject Contact pHeader 
**		structure
**
**********************************************************************/
void sip_ccp_freeSipRejectContactHeader
#ifdef ANSI_PROTO
	(SipRejectContactHeader *pHdr)
#else
	(pHdr)
	SipRejectContactHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE (pHdr->pDispName);
		sip_freeSipAddrSpec(pHdr->pAddrSpec);
		sip_listDeleteAll(&(pHdr->slRejectContactParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


/**********************************************************************
**
** FUNCTION: sip_ccp_freeSipAcceptContactParam 
**
** DESCRIPTION: This function frees a SIP Accept-Contact param 
**		structure
**
**********************************************************************/
void sip_ccp_freeSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SipAcceptContactParam *pAcceptContactParam)
#else
	(pAcceptContactParam)
	SipAcceptContactParam *pAcceptContactParam;
#endif
{
	if (pAcceptContactParam == SIP_NULL) return;
	HSS_LOCKREF(pAcceptContactParam->dRefCount);HSS_DECREF(pAcceptContactParam->dRefCount);
	if(HSS_CHECKREF(pAcceptContactParam->dRefCount))
	{
		switch (pAcceptContactParam->dType)
		{
			case	SipAccContactTypeExt	:sip_freeSipParam((pAcceptContactParam->u).pExtParam);
							 break;
			case	SipAccContactTypeQvalue	:HSS_FREE((pAcceptContactParam->u).pQvalue);
			case 	SipAccContactTypeAny	:break;
		}
		HSS_UNLOCKREF(pAcceptContactParam->dRefCount);
		HSS_DELETEREF(pAcceptContactParam->dRefCount);
		HSS_FREE(pAcceptContactParam);
	}
	else
	{
		HSS_UNLOCKREF(pAcceptContactParam->dRefCount);
	}
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_freeSipAcceptContactHeader
**
** DESCRIPTION: This function frees the SIP accept Contact pHeader 
**		structure
**
**********************************************************************/
void sip_ccp_freeSipAcceptContactHeader 
#ifdef ANSI_PROTO
	(SipAcceptContactHeader *pHdr)
#else
	(pHdr)
	SipAcceptContactHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispName);
		sip_freeSipAddrSpec (pHdr->pAddrSpec);
		sip_listDeleteAll(&(pHdr->slAcceptContactParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

/**********************************************************************
**
** FUNCTION:  sip_ccp_freeSipRejectContactParam
**
** DESCRIPTION: This function frees a SIP Reject ContactParam structure
**
**********************************************************************/
void sip_ccp_freeSipRejectContactParam 
#ifdef ANSI_PROTO
	(SipRejectContactParam * pRejectContactParam)
#else
	(pRejectContactParam)
	SipRejectContactParam * pRejectContactParam;
#endif
{
	if (pRejectContactParam == SIP_NULL) return;
	HSS_LOCKREF(pRejectContactParam->dRefCount);HSS_DECREF(pRejectContactParam->dRefCount);
	if(HSS_CHECKREF(pRejectContactParam->dRefCount))
	{
		switch (pRejectContactParam->dType)
		{
			case	SipRejContactTypeExt	:sip_freeSipParam((pRejectContactParam->u).pExtParam);
							 break;
			case	SipRejContactTypeQvalue	:HSS_FREE((pRejectContactParam->u).pQvalue);
			case 	SipRejContactTypeAny	:break;
		}
		HSS_UNLOCKREF(pRejectContactParam->dRefCount);
		HSS_DELETEREF(pRejectContactParam->dRefCount);
		HSS_FREE(pRejectContactParam);
	}
	else
	{
		HSS_UNLOCKREF(pRejectContactParam->dRefCount);
	}
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipRejectContactHeader
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipRejectContactHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_ccp_freeSipRejectContactHeader ((SipRejectContactHeader *)pHdr);
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipAcceptContactHeader
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipAcceptContactHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_ccp_freeSipAcceptContactHeader ((SipAcceptContactHeader *)pHdr);
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipRejectContactParam
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipRejectContactParam 
#ifdef ANSI_PROTO
	(SIP_Pvoid pRejectContactParam)
#else
	(pRejectContactParam)
	SIP_Pvoid pRejectContactParam;
#endif
{
	sip_ccp_freeSipRejectContactParam ((SipRejectContactParam *)pRejectContactParam);
}

/**********************************************************************
**
** FUNCTION:  __sip_ccp_freeSipAcceptContactParam
**
** DESCRIPTION: Wrapper for siplist
**
**********************************************************************/
void __sip_ccp_freeSipAcceptContactParam 
#ifdef ANSI_PROTO
	(SIP_Pvoid pAcceptContactParam)
#else
	(pAcceptContactParam)
	SIP_Pvoid pAcceptContactParam;
#endif
{
	sip_ccp_freeSipAcceptContactParam ((SipAcceptContactParam *)pAcceptContactParam);
}
#endif
