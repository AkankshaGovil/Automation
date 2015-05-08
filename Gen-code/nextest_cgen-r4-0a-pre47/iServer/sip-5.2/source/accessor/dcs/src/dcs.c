/******************************************************************************
** FUNCTION:
** 	This file contains the source dCodeNum of all DCS 
**      Header SIP stack APIs.
**
*******************************************************************************
**
** FILENAME:
** 	dcs.c
**
** DESCRIPTION:
**  	This Header file contains the source Code of all the DCS APIs
**		general headers
**
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 15Nov00   	S.Luthra    			Creation
**
** Copyrights 2000, Hughes Software Systems, Ltd.
**
******************************************************************************/


#include "dcs.h"
#include "sipcommon.h"
#include "siplist.h"
#include "sipfree.h"
#include "sipinit.h"
#include "portlayer.h"
#include "sipclone.h"
#include "dcsinit.h"
#include "dcsfree.h"
#include "dcsclone.h"


SipBool sip_dcs_getDispNameFromDcsRemotePartyIdHdr 
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
	SIPDEBUGFN("Entering function sip_getDispNameFromDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppDispName == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempDispName = ((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pDispName;
 	if (pTempDispName == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppDispName = (SIP_S8bit *) STRDUPACCESSOR (pTempDispName);
	if (*ppDispName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppDispName = pTempDispName;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromDcsRemotePartyIdHdr");
 	return SipSuccess;
}  


SipBool sip_dcs_setDispNameInDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pDispName, SipError *pErr)
#else
	(pHdr, pDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pDispName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempDispName, *pDispNameToken;
	SIPDEBUGFN("Entering function sip_setDispNameInDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pDispName == SIP_NULL)
                pTempDispName = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempDispName = (SIP_S8bit *) STRDUPACCESSOR(pDispName);
		if (pTempDispName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempDispName = pDispName;
#endif
	}	
        
        pDispNameToken = ((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->pDispName;
                
	if ( pDispNameToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pDispNameToken), pErr) == SipFail)
		{
			sip_freeString(pTempDispName);
			return SipFail;
		}
	}
	
	((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->pDispName = pTempDispName;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInDcsRemotePartyIdHdr");
        return SipSuccess;
}


#ifdef SIP_BY_REFERENCE 
SipBool sip_dcs_getAddrSpecFromDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#else
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getAddrSpecFromDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif 
#endif
{
	SipAddrSpec *pTempAddrSpec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pAddrSpec == SIP_NULL)
#else
	if( ppAddrSpec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
 	
 	pTempAddrSpec = ((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec;
 	if (pTempAddrSpec == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
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
#else
	HSS_LOCKEDINCREF(pTempAddrSpec->dRefCount);
	*ppAddrSpec = pTempAddrSpec;
#endif
 	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromDcsRemotePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_setAddrSpecInDcsRemotePartyIdHdr 
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
	SIPDEBUGFN("Entering function sip_setAddrSpecInDcsRemotePartyIdHdr"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif 	
 	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec);
 		((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE	
		if(sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec);
		((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec = pTempAddrSpec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec);
		((SipDcsRemotePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec = pAddrSpec;
#endif

 	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInDcsRemotePartyIdHdr");
	return SipSuccess;
}


/*
SipBool sip_dcs_getRpiAuthCountFromDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getRpiAuthCountFromDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slRpiAuths), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getRpiAuthCountFromDcsRemotePartyIdHdr");	
	return SipSuccess;
}


SipBool sip_dcs_getRpiAuthAtIndexFromDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipGenericChallenge **ppRpiAuth, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipGenericChallenge *pRpiAuth, SIP_U32bit dIndex, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppRpiAuth, dIndex, pErr)
	SipHeader *pHdr;
	SipGenericChallenge **ppRpiAuth;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pHdr, pRpiAuth, dIndex, pErr)
	SipHeader *pHdr;
	SipGenericChallenge *pRpiAuth;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid pElementFromList;
	SipGenericChallenge *pTempChallenge;
	SIPDEBUGFN("Entering function sip_getRpiAuthAtIndexFromDcsRemotePartyIdHdr");	
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  ppRpiAuth == SIP_NULL)
#else
	if(  pRpiAuth == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	
	if (sip_listGetAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slRpiAuths), dIndex,  \
		&pElementFromList, pErr) == SipFail)
		return SipFail;

	if (pElementFromList == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempChallenge = (SipGenericChallenge *)pElementFromList;
#ifndef SIP_BY_REFERENCE	
	if (__sip_cloneSipChallenge(pRpiAuth, pTempChallenge, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTempChallenge->dRefCount);
	*ppRpiAuth = pTempChallenge;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getRpiAuthAtIndexFromDcsRemotePartyIdHdr");
	return SipSuccess;
}



SipBool sip_dcs_setRpiAuthAtIndexInDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipGenericChallenge *pRpiAuth, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pRpiAuth, dIndex, pErr)
	SipHeader *pHdr;
	SipGenericChallenge *pRpiAuth;
	SIP_U32bit dIndex; 
	SipError *pErr;
#endif
{
	SipGenericChallenge *pTempChallenge;
	SIPDEBUGFN("Entering function sip_setRpiAuthAtIndexInDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pRpiAuth == SIP_NULL )
		pTempChallenge = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipGenericChallenge(&pTempChallenge, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipChallenge(pTempChallenge, pRpiAuth, pErr) == SipFail)
		{
			sip_freeSipGenericChallenge (pTempChallenge); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pRpiAuth->dRefCount);
		pTempChallenge = pRpiAuth;
#endif
	}
			
	if( sip_listSetAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slRpiAuths),  \
		dIndex, (SIP_Pvoid)(pTempChallenge), pErr) == SipFail)
	{
		if (pTempChallenge != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipGenericChallenge (pTempChallenge); 
#else
		HSS_LOCKEDDECREF(pRpiAuth->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setRpiAuthAtIndexInDcsRemotePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_insertRpiAuthAtIndexInDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipGenericChallenge *pRpiAuth, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pRpiAuth, dIndex, pErr)
	SipHeader *pHdr;
	SipGenericChallenge *pRpiAuth;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipGenericChallenge *pTempChallenge;
	SIPDEBUGFN("Entering function sip_insertRpiAuthAtIndexInDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pRpiAuth == SIP_NULL )
		pTempChallenge = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipGenericChallenge(&pTempChallenge, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipChallenge(pTempChallenge, pRpiAuth, pErr) == SipFail)
		{
			sip_freeSipGenericChallenge (pTempChallenge); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pRpiAuth->dRefCount);
		pTempChallenge = pRpiAuth;
#endif
	}
			
	if( sip_listInsertAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slRpiAuths),  \
		dIndex, (SIP_Pvoid)(pTempChallenge), pErr) == SipFail)
	{
		if (pTempChallenge != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipGenericChallenge (pTempChallenge); 
#else
		HSS_LOCKEDDECREF(pRpiAuth->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertRpiAuthAtIndexInDcsRemotePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_deleteRpiAuthAtIndexInDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteRpiAuthAtIndexInDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slRpiAuths), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteRpiAuthAtIndexInDcsRemotePartyIdHdr");	
	return SipSuccess;
}
*/

SipBool sip_dcs_getParamCountFromDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromDcsRemotePartyIdHdr");	
	return SipSuccess;
}


#ifdef SIP_BY_REFERENCE 
SipBool sip_dcs_getParamAtIndexFromDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getParamAtIndexFromDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid pElementFromList;
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromDcsRemotePartyIdHdr");	
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pParam == SIP_NULL)
#else
	if( ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	
	if (sip_listGetAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&pElementFromList, pErr) == SipFail)
		return SipFail;

	if (pElementFromList == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempParam = (SipParam *)pElementFromList;
#ifndef SIP_BY_REFERENCE	
	if (__sip_cloneSipParam(pParam, pTempParam, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppParam = pTempParam;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromDcsRemotePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_setParamAtIndexInDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTempParam, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}
			
	if( sip_listSetAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam); 
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInDcsRemotePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_insertParamAtIndexInDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTempParam, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}
			
	if( sip_listInsertAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam); 
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInDcsRemotePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_deleteParamAtIndexInDcsRemotePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInDcsRemotePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRemotePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipDcsRemotePartyIdHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInDcsRemotePartyIdHdr");	
	return SipSuccess;
}

SipBool sip_dcs_getParamCountFromDcsRpidPrivacyHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromDcsRpidPrivacyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRpidPrivacy)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipDcsRpidPrivacyHeader *)\
		(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromDcsRpidPrivacyHdr");	
	return SipSuccess;
}


#ifdef SIP_BY_REFERENCE 
SipBool sip_dcs_getParamAtIndexFromDcsRpidPrivacyHdr 
#ifdef ANSI_PROTO
   (SipHeader *pHdr,SipParam **ppParam,SIP_U32bit dIndex,SipError *pErr)
#else
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getParamAtIndexFromDcsRpidPrivacyHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr,SipParam *pParam,SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid pElementFromList;
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromDcsRpidPrivacyHdr");	
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pParam == SIP_NULL)
#else
	if( ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRpidPrivacy)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	
	if (sip_listGetAt( &(((SipDcsRpidPrivacyHeader *)\
		(pHdr->pHeader))->slParams), dIndex, &pElementFromList, pErr)\
		== SipFail)
		return SipFail;

	if (pElementFromList == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempParam = (SipParam *)pElementFromList;
#ifndef SIP_BY_REFERENCE	
	if (__sip_cloneSipParam(pParam, pTempParam, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppParam = pTempParam;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromDcsRpidPrivacyHdr");
	return SipSuccess;
}


SipBool sip_dcs_setParamAtIndexInDcsRpidPrivacyHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr,SipParam *pParam,SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInDcsRpidPrivacyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRpidPrivacy)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTempParam, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}
			
	if( sip_listSetAt( &(((SipDcsRpidPrivacyHeader *)\
		(pHdr->pHeader))->slParams), dIndex, (SIP_Pvoid)(pTempParam), \
		pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam); 
#else
			HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInDcsRpidPrivacyHdr");
	return SipSuccess;
}


SipBool sip_dcs_insertParamAtIndexInDcsRpidPrivacyHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr,SipParam *pParam,SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInDcsRpidPrivacyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRpidPrivacy)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTempParam, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}
			
	if( sip_listInsertAt( &(((SipDcsRpidPrivacyHeader *)\
		(pHdr->pHeader))->slParams), dIndex, (SIP_Pvoid)(pTempParam),\
		pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam); 
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInDcsRpidPrivacyHdr");
	return SipSuccess;
}


SipBool sip_dcs_deleteParamAtIndexInDcsRpidPrivacyHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInDcsRpidPrivacyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRpidPrivacy)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipDcsRpidPrivacyHeader *)\
		(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInDcsRpidPrivacyHdr");	
	return SipSuccess;
}

#ifdef SIP_BY_REFERENCE 
SipBool sip_dcs_getAddrSpecFromDcsTracePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#else
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getAddrSpecFromDcsTracePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
#endif
{
	SipAddrSpec *pTempAddrSpec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromDcsTracePartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pAddrSpec == SIP_NULL)
#else
	if( ppAddrSpec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsTracePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
 	
 	pTempAddrSpec = ((SipDcsTracePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec;
 	if (pTempAddrSpec == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
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
#else
	HSS_LOCKEDINCREF(pTempAddrSpec->dRefCount);
	*ppAddrSpec = pTempAddrSpec;
#endif
 	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromDcsTracePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_setAddrSpecInDcsTracePartyIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec,  SipError *pErr)
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
	SIPDEBUGFN("Entering function sip_setAddrSpecInDcsTracePartyIdHdr"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsTracePartyId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif 	
 	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipDcsTracePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec);
 		((SipDcsTracePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE	
		if(sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipDcsTracePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec);
		((SipDcsTracePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec = pTempAddrSpec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipDcsTracePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec);
		((SipDcsTracePartyIdHeader *)(pHdr ->pHeader))->pAddrSpec = pAddrSpec;
#endif

 	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInDcsTracePartyIdHdr");
	return SipSuccess;
}


SipBool sip_dcs_getTagFromDcsAnonymityHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppTag, SipError *pErr)
#else
	(pHdr, ppTag, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppTag;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempTag;
	SIPDEBUGFN("Entering function sip_getTagFromDcsAnonymityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppTag == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsAnonymity)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempTag = ((SipDcsAnonymityHeader *)(pHdr ->pHeader))->pTag;
 	if (pTempTag == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppTag = (SIP_S8bit *) STRDUPACCESSOR (pTempTag);
	if (*ppTag == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppTag = pTempTag;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTagFromDcsAnonymityHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setTagInDcsAnonymityHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pTag, SipError *pErr)
#else
	(pHdr, pTag, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pTag;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempTag, *pTagToken;
	SIPDEBUGFN("Entering function sip_setTagInDcsAnonymityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsAnonymity)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pTag == SIP_NULL)
                pTempTag = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempTag = (SIP_S8bit *) STRDUPACCESSOR(pTag);
		if (pTempTag == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempTag = pTag;
#endif
	}	
        
        pTagToken = ((SipDcsAnonymityHeader *)(pHdr->pHeader))->pTag;
                
	if ( pTagToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pTagToken), pErr) == SipFail)
		{
			sip_freeString(pTempTag);
			return SipFail;
		}
	}
	
	((SipDcsAnonymityHeader *)(pHdr->pHeader))->pTag = pTempTag;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTagInDcsAnonymityHdr");
        return SipSuccess;
}


SipBool sip_dcs_getAuthFromDcsMediaAuthorizationHdr 
#ifdef ANSI_PROTO
	(SipHeader  *pHdr, SIP_S8bit  **ppAuth, SipError *pErr)
#else
	(pHdr, ppAuth, pErr)
	SipHeader  *pHdr;
	SIP_S8bit  **ppAuth;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempAuth;
	SIPDEBUGFN("Entering function sip_getAuthFromDcsMediaAuthorizationHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppAuth == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsMediaAuthorization)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempAuth = ((SipDcsMediaAuthorizationHeader *)(pHdr ->pHeader))->pAuth;
 	if (pTempAuth == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppAuth = (SIP_S8bit *) STRDUPACCESSOR (pTempAuth);
	if (*ppAuth == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppAuth = pTempAuth;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAuthFromDcsMediaAuthorizationHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setAuthInDcsMediaAuthorizationHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pAuth, SipError *pErr)
#else
	(pHdr, pAuth, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pAuth;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempAuth, *pAuthorization;
	SIPDEBUGFN("Entering function sip_setAuthInDcsMediaAuthorizationHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsMediaAuthorization)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pAuth == SIP_NULL)
                pTempAuth = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempAuth = (SIP_S8bit *) STRDUPACCESSOR(pAuth);
		if (pTempAuth == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempAuth = pAuth;
#endif
	}	
        
        pAuthorization = ((SipDcsMediaAuthorizationHeader *)(pHdr->pHeader))->pAuth;
                
	if ( pAuthorization != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pAuthorization), pErr) == SipFail)
		{
			sip_freeString(pTempAuth);
			return SipFail;
		}
	}
	
	((SipDcsMediaAuthorizationHeader *)(pHdr->pHeader))->pAuth = pTempAuth;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAuthInDcsMediaAuthorizationHdr");
        return SipSuccess;
}


SipBool sip_dcs_getHostFromDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr,  SIP_S8bit  **ppHost,  SipError *pErr)
#else
	(pHdr, ppHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit  **ppHost;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempHost;
	SIPDEBUGFN("Entering function sip_getHostFromDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppHost == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempHost = ((SipDcsGateHeader *)(pHdr ->pHeader))->pHost;
 	if (pTempHost == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppHost = (SIP_S8bit *) STRDUPACCESSOR (pTempHost);
	if (*ppHost == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppHost = pTempHost;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getHostFromDcsGateHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setHostInDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pHost, SipError *pErr)
#else
	(pHdr, pHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempHost, *pHostToken;
	SIPDEBUGFN("Entering function sip_setHostInDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pHost == SIP_NULL)
                pTempHost = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempHost = (SIP_S8bit *) STRDUPACCESSOR(pHost);
		if (pTempHost == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempHost = pHost;
#endif
	}	
        
        pHostToken = ((SipDcsGateHeader *)(pHdr->pHeader))->pHost;
                
	if ( pHostToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pHostToken), pErr) == SipFail)
		{
			sip_freeString(pTempHost);
			return SipFail;
		}
	}
	
	((SipDcsGateHeader *)(pHdr->pHeader))->pHost = pTempHost;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setHostInDcsGateHdr");
        return SipSuccess;
}


SipBool sip_dcs_getPortFromDcsGateHdr  
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit *pPort, SipError *pErr)
#else
	(pHdr, pPort, pErr)
	SipHeader *pHdr;
	SIP_U16bit *pPort;
	SipError *pErr;
#endif
{
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pPort == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if(((SipDcsGateHeader *)(pHdr->pHeader) )->pPort==SIP_NULL)
	{
		 *pErr= E_NO_EXIST;
		 return SipFail;
	}

	*pPort = *(((SipDcsGateHeader *)(pHdr->pHeader) )->pPort); 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getPortFromDcsGateHdr");
	return SipSuccess;
}


SipBool sip_dcs_setPortInDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit  dPort, SipError *pErr)
#else
	(pHdr, dPort, pErr)
	SipHeader *pHdr;
	SIP_U16bit  dPort;
	SipError *pErr;
#endif
{
	SIP_U16bit *temp_port;
	SIPDEBUGFN("Entering function sip_setPortInDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
		temp_port = ( SIP_U16bit * )fast_memget(ACCESSOR_MEM_ID,sizeof(SIP_U16bit),pErr);
	*temp_port = dPort;
	if ( temp_port== SIP_NULL )
		return SipFail;
	if ((((SipDcsGateHeader *)(pHdr->pHeader))->pPort)  != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&((((SipDcsGateHeader *) \
			(pHdr->pHeader))->pPort)), pErr) == SipFail)
			return SipFail;
	}
	((SipDcsGateHeader *)(pHdr->pHeader))->pPort= temp_port; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setPortInDcsGateHdr");
	return SipSuccess;
}


SipBool sip_dcs_getIdFromDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr,  SIP_S8bit  **ppId,  SipError *pErr)
#else
	(pHdr, ppId, pErr)
	SipHeader *pHdr;
	SIP_S8bit  **ppId;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempId;
	SIPDEBUGFN("Entering function sip_getIdFromDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppId == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempId = ((SipDcsGateHeader *)(pHdr ->pHeader))->pId;
 	if (pTempId == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppId = (SIP_S8bit *) STRDUPACCESSOR (pTempId);
	if (*ppId == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppId = pTempId;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getIdFromDcsGateHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setIdInDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pId, SipError *pErr)
#else
	(pHdr, pId, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pId;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempId, *pIdToken;
	SIPDEBUGFN("Entering function sip_setIdInDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pId == SIP_NULL)
                pTempId = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempId = (SIP_S8bit *) STRDUPACCESSOR(pId);
		if (pTempId == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempId = pId;
#endif
	}	
        
        pIdToken = ((SipDcsGateHeader *)(pHdr->pHeader))->pId;
                
	if ( pIdToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pIdToken), pErr) == SipFail)
		{
			sip_freeString(pTempId);
			return SipFail;
		}
	}
	
	((SipDcsGateHeader *)(pHdr->pHeader))->pId = pTempId;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setIdInDcsGateHdr");
        return SipSuccess;
}


SipBool sip_dcs_getKeyFromDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppKey, SipError *pErr)
#else
	(pHdr, ppKey, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppKey;
	 SipError *pErr;
#endif
{
 	SIP_S8bit *pTempKey;
	SIPDEBUGFN("Entering function sip_getKeyFromDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppKey == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempKey = ((SipDcsGateHeader *)(pHdr ->pHeader))->pKey;
 	if (pTempKey == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppKey = (SIP_S8bit *) STRDUPACCESSOR (pTempKey);
	if (*ppKey == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppKey = pTempKey;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getKeyFromDcsGateHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setKeyInDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pKey, SipError *pErr)
#else
	(pHdr, pKey, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pKey;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempKey, *pKeyToken;
	SIPDEBUGFN("Entering function sip_setKeyInDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pKey == SIP_NULL)
                pTempKey = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempKey = (SIP_S8bit *) STRDUPACCESSOR(pKey);
		if (pTempKey == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempKey = pKey;
#endif
	}	
        
        pKeyToken = ((SipDcsGateHeader *)(pHdr->pHeader))->pKey;
                
	if ( pKeyToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pKeyToken), pErr) == SipFail)
		{
			sip_freeString(pTempKey);
			return SipFail;
		}
	}
	
	((SipDcsGateHeader *)(pHdr->pHeader))->pKey = pTempKey;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setKeyInDcsGateHdr");
        return SipSuccess;
}


SipBool sip_dcs_getCipherSuiteFromDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppCipherSuite, SipError *pErr)
#else
	(pHdr, ppCipherSuite, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppCipherSuite;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempCipherSuite;
	SIPDEBUGFN("Entering function sip_getCipherSuiteFromDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppCipherSuite == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempCipherSuite = ((SipDcsGateHeader *)(pHdr ->pHeader))->pCipherSuite;
 	if (pTempCipherSuite == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppCipherSuite = (SIP_S8bit *) STRDUPACCESSOR (pTempCipherSuite);
	if (*ppCipherSuite == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppCipherSuite = pTempCipherSuite;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getCipherSuiteFromDcsGateHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setCipherSuiteInDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pCipherSuite, SipError *pErr)
#else
	(pHdr, pCipherSuite, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pCipherSuite;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempCipherSuite, *pCipherSuiteToken;
	SIPDEBUGFN("Entering function sip_setCipherSuiteInDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pCipherSuite == SIP_NULL)
                pTempCipherSuite = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempCipherSuite = (SIP_S8bit *) STRDUPACCESSOR(pCipherSuite);
		if (pTempCipherSuite == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempCipherSuite = pCipherSuite;
#endif
	}	
        
        pCipherSuiteToken = ((SipDcsGateHeader *)(pHdr->pHeader))->pCipherSuite;
                
	if ( pCipherSuiteToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pCipherSuiteToken), pErr) == SipFail)
		{
			sip_freeString(pTempCipherSuite);
			return SipFail;
		}
	}
	
	((SipDcsGateHeader *)(pHdr->pHeader))->pCipherSuite = pTempCipherSuite;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setCipherSuiteInDcsGateHdr");
        return SipSuccess;
}


SipBool sip_dcs_getStrengthFromDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppStrength, SipError *pErr)
#else
	(pHdr, ppStrength, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppStrength;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempStrength;
	SIPDEBUGFN("Entering function sip_getStrengthFromDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppStrength == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempStrength = ((SipDcsGateHeader *)(pHdr ->pHeader))->pStrength;
 	if (pTempStrength == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppStrength = (SIP_S8bit *) STRDUPACCESSOR (pTempStrength);
	if (*ppStrength == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppStrength = pTempStrength;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getStrengthFromDcsGateHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setStrengthInDcsGateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pStrength, SipError *pErr)
#else
	(pHdr, pStrength, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pStrength;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempStrength, *pStrengthToken;
	SIPDEBUGFN("Entering function sip_setStrengthInDcsGateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsGate)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pStrength == SIP_NULL)
                pTempStrength = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempStrength = (SIP_S8bit *) STRDUPACCESSOR(pStrength);
		if (pTempStrength == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempStrength = pStrength;
#endif
	}	
        
        pStrengthToken = ((SipDcsGateHeader *)(pHdr->pHeader))->pStrength;
                
	if ( pStrengthToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pStrengthToken), pErr) == SipFail)
		{
			sip_freeString(pTempStrength);
			return SipFail;
		}
	}
	
	((SipDcsGateHeader *)(pHdr->pHeader))->pStrength = pTempStrength;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setStrengthInDcsGateHdr");
        return SipSuccess;
}


SipBool sip_dcs_getHostFromDcsStateHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppHost, SipError *pErr)
#else
	(pHdr, ppHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppHost;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_getHostFromDcsStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppHost == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
 	if (((SipDcsStateHeader *)(pHdr ->pHeader))->pHost == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppHost = (SIP_S8bit *) STRDUPACCESSOR (((SipDcsStateHeader *)(pHdr ->pHeader))->pHost);
	if (*ppHost == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppHost = ((SipDcsStateHeader *)(pHdr ->pHeader))->pHost;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getHostFromDcsBillingInfoHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setHostInDcsStateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pHost, SipError *pErr)
#else
	(pHdr, pHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempHost;
	SIPDEBUGFN("Entering function sip_dcs_setHostInDcsStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pHost == SIP_NULL)
                pTempHost = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempHost = (SIP_S8bit *) STRDUPACCESSOR(pHost);
		if (pTempHost == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempHost = pHost;
#endif
	}	
        
	if ( ((SipDcsStateHeader *)(pHdr->pHeader))->pHost != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&((SipDcsBillingInfoHeader *)(pHdr->pHeader))->pHost), pErr) == SipFail)
		{
			return SipFail;
		}
	}
	
	((SipDcsStateHeader *)(pHdr->pHeader))->pHost = pTempHost;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_setHostInDcsStateHdr");
        return SipSuccess;
}



SipBool sip_dcs_getParamCountFromDcsStateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromDcsStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipDcsStateHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromDcsStateHdr");	
	return SipSuccess;
}


#ifdef SIP_BY_REFERENCE 
SipBool sip_dcs_getParamAtIndexFromDcsStateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getParamAtIndexFromDcsStateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid pElementFromList;
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromDcsStateHdr");	
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pParam == SIP_NULL)
#else
	if( ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	
	if (sip_listGetAt( &(((SipDcsStateHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&pElementFromList, pErr) == SipFail)
		return SipFail;

	if (pElementFromList == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempParam = (SipParam *)pElementFromList;
#ifndef SIP_BY_REFERENCE	
	if (__sip_cloneSipParam(pParam, pTempParam, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppParam = pTempParam;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromDcsStateHdr");
	return SipSuccess;
}


SipBool sip_dcs_setParamAtIndexInDcsStateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInDcsStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTempParam, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}
			
	if( sip_listSetAt( &(((SipDcsStateHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam); 
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInDcsStateHdr");
	return SipSuccess;
}


SipBool sip_dcs_insertParamAtIndexInDcsStateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInDcsStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTempParam, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}
			
	if( sip_listInsertAt( &(((SipDcsStateHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam); 
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInDcsStateHdr");
	return SipSuccess;
}


SipBool sip_dcs_deleteParamAtIndexInDcsStateHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInDcsStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipDcsStateHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInDcsStateHdr");	
	return SipSuccess;
}


SipBool sip_dcs_getTagFromDcsOspsHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppTag, SipError *pErr)
#else
	(pHdr, ppTag, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppTag;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempTag;
	SIPDEBUGFN("Entering function sip_getTagFromDcsOspsHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppTag == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsOsps)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempTag = ((SipDcsOspsHeader *)(pHdr ->pHeader))->pTag;
 	if (pTempTag == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppTag = (SIP_S8bit *) STRDUPACCESSOR (pTempTag);
	if (*ppTag == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppTag = pTempTag;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTagFromDcsOspsHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setTagInDcsOspsHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pTag, SipError *pErr)
#else
	(pHdr, pTag, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pTag;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempTag, *pTagToken;
	SIPDEBUGFN("Entering function sip_setTagInDcsOspsHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsOsps)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pTag == SIP_NULL)
                pTempTag = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempTag = (SIP_S8bit *) STRDUPACCESSOR(pTag);
		if (pTempTag == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempTag = pTag;
#endif
	}	
        
        pTagToken = ((SipDcsOspsHeader *)(pHdr->pHeader))->pTag;
                
	if ( pTagToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pTagToken), pErr) == SipFail)
		{
			sip_freeString(pTempTag);
			return SipFail;
		}
	}
	
	((SipDcsOspsHeader *)(pHdr->pHeader))->pTag = pTempTag;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTagInDcsOspsHdr");
        return SipSuccess;
}

SipBool sip_dcs_getFEIdFromDcsBillingIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppFEId, SipError *pErr)
#else
	(pHdr, ppFEId, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppFEId;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempFEId;
	SIPDEBUGFN("Entering function sip_getFEIdFromDcsBillingIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppFEId == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempFEId = ((SipDcsBillingIdHeader *)(pHdr ->pHeader))->pFEId;
 	if (pTempFEId == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppFEId = (SIP_S8bit *) STRDUPACCESSOR (pTempFEId);
	if (*ppFEId == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppFEId = pTempFEId;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getFEIdFromDcsBillingIdHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setFEIdInDcsBillingIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pFEId, SipError *pErr)
#else
	(pHdr, pFEId, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pFEId;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempFEId, *pFEIdToken;
	SIPDEBUGFN("Entering function sip_setFEIdInDcsBillingIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pFEId == SIP_NULL)
                pTempFEId = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempFEId = (SIP_S8bit *) STRDUPACCESSOR(pFEId);
		if (pTempFEId == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempFEId = pFEId;
#endif
	}	
        
        pFEIdToken = ((SipDcsBillingIdHeader *)(pHdr->pHeader))->pFEId;
                
	if ( pFEIdToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pFEIdToken), pErr) == SipFail)
		{
			sip_freeString(pTempFEId);
			return SipFail;
		}
	}
	
	((SipDcsBillingIdHeader *)(pHdr->pHeader))->pFEId = pTempFEId;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setFEIdInDcsBillingIdHdr");
        return SipSuccess;
}

SipBool sip_dcs_getIdFromDcsBillingIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppId, SipError *pErr)
#else
	(pHdr, ppId, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppId;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempId;
	SIPDEBUGFN("Entering function sip_getIdFromDcsBillingIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppId == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempId = ((SipDcsBillingIdHeader *)(pHdr ->pHeader))->pId;
 	if (pTempId == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppId = (SIP_S8bit *) STRDUPACCESSOR (pTempId);
	if (*ppId == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppId = pTempId;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getIdFromDcsBillingIdHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setIdInDcsBillingIdHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pId, SipError *pErr)
#else
	(pHdr, pId, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pId;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempId, *pIdToken;
	SIPDEBUGFN("Entering function sip_setIdInDcsBillingIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingId)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pId == SIP_NULL)
                pTempId = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempId = (SIP_S8bit *) STRDUPACCESSOR(pId);
		if (pTempId == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempId = pId;
#endif
	}	
        
        pIdToken = ((SipDcsBillingIdHeader *)(pHdr->pHeader))->pId;
                
	if ( pIdToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pIdToken), pErr) == SipFail)
		{
			sip_freeString(pTempId);
			return SipFail;
		}
	}
	
	((SipDcsBillingIdHeader *)(pHdr->pHeader))->pId = pTempId;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setIdInDcsBillingIdHdr");
        return SipSuccess;
}


SipBool sip_dcs_getSignatureHostFromDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppSignatureHost, SipError *pErr)
#else
	(pHdr,ppSignatureHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppSignatureHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempSignatureHost;
	SIPDEBUGFN("Entering function sip_getSignatureHostFromDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppSignatureHost == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempSignatureHost = ((SipDcsLaesHeader *)(pHdr ->pHeader))->pSignatureHost;
 	if (pTempSignatureHost == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppSignatureHost = (SIP_S8bit *) STRDUPACCESSOR (pTempSignatureHost);
	if (*ppSignatureHost == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppSignatureHost = pTempSignatureHost;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSignatureHostFromDcsLaesHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setSignatureHostInDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pSignatureHost, SipError *pErr)
#else
	(pHdr, pSignatureHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pSignatureHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempSignatureHost, *pSignatureHostToken;
	SIPDEBUGFN("Entering function sip_setSignatureHostInDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pSignatureHost == SIP_NULL)
                pTempSignatureHost = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempSignatureHost = (SIP_S8bit *) STRDUPACCESSOR(pSignatureHost);
		if (pTempSignatureHost == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempSignatureHost = pSignatureHost;
#endif
	}	
        
        pSignatureHostToken = ((SipDcsLaesHeader *)(pHdr->pHeader))->pSignatureHost;
                
	if ( pSignatureHostToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pSignatureHostToken), pErr) == SipFail)
		{
			sip_freeString(pTempSignatureHost);
			return SipFail;
		}
	}
	
	((SipDcsLaesHeader *)(pHdr->pHeader))->pSignatureHost = pTempSignatureHost;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSignatureHostInDcsLaesHdr");
        return SipSuccess;
}


SipBool sip_dcs_getContentHostFromDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr,  SIP_S8bit **ppContentHost, SipError *pErr)
#else
	(pHdr, ppContentHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppContentHost;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempContentHost;
	SIPDEBUGFN("Entering function sip_getContentHostFromDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppContentHost == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempContentHost = ((SipDcsLaesHeader *)(pHdr ->pHeader))->pContentHost;
 	if (pTempContentHost == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppContentHost = (SIP_S8bit *) STRDUPACCESSOR (pTempContentHost);
	if (*ppContentHost == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppContentHost = pTempContentHost;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getContentHostFromDcsLaesHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setContentHostInDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pContentHost, SipError *pErr)
#else
	(pHdr, pContentHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pContentHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempContentHost, *pContentHostToken;
	SIPDEBUGFN("Entering function sip_setContentHostInDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pContentHost == SIP_NULL)
                pTempContentHost = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempContentHost = (SIP_S8bit *) STRDUPACCESSOR(pContentHost);
		if (pTempContentHost == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempContentHost = pContentHost;
#endif
	}	
        
        pContentHostToken = ((SipDcsLaesHeader *)(pHdr->pHeader))->pContentHost;
                
	if ( pContentHostToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pContentHostToken), pErr) == SipFail)
		{
			sip_freeString(pTempContentHost);
			return SipFail;
		}
	}
	
	((SipDcsLaesHeader *)(pHdr->pHeader))->pContentHost = pTempContentHost;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setContentHostInDcsLaesHdr");
        return SipSuccess;
}


SipBool sip_dcs_getSignaturePortFromDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit *pSignaturePort, SipError *pErr)
#else
	(pHdr, pSignaturePort, pErr)
	SipHeader *pHdr;
	SIP_U16bit *pSignaturePort;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getSignaturePortFromDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pSignaturePort == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if(((SipDcsLaesHeader*)(pHdr->pHeader) )->pSignaturePort==SIP_NULL)
    {
         *pErr= E_NO_EXIST;
         return SipFail;
    }

	*pSignaturePort = *(((SipDcsLaesHeader *)(pHdr->pHeader) )->pSignaturePort); 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSignaturePortFromDcsLaesHdr");
	return SipSuccess;
}


SipBool sip_dcs_setSignaturePortInDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit dSignaturePort, SipError *pErr)
#else
	(pHdr, dSignaturePort, pErr)
	SipHeader *pHdr;
	SIP_U16bit dSignaturePort;
	SipError *pErr;
#endif
{
	SIP_U16bit *temp_port;
	SIPDEBUGFN("Entering function sip_setSignaturePortInDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	temp_port = ( SIP_U16bit * )fast_memget(ACCESSOR_MEM_ID, \
		sizeof(SIP_U16bit),pErr);
	*temp_port = dSignaturePort;
	if ( temp_port== SIP_NULL )
		return SipFail;
	if ((((SipDcsLaesHeader*)(pHdr->pHeader))->pSignaturePort)  != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&((((SipDcsLaesHeader *) \
			(pHdr->pHeader))->pSignaturePort)), pErr) == SipFail)
			return SipFail;
	}
	((SipDcsLaesHeader*)(pHdr->pHeader))->pSignaturePort= temp_port; 

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSignaturePortInDcsLaesHdr");
	return SipSuccess;
}


SipBool sip_dcs_getContentPortFromDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit *pContentPort, SipError *pErr)
#else
	(pHdr, pContentPort, pErr)
	SipHeader *pHdr;
	SIP_U16bit *pContentPort;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getContentPortFromDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pContentPort == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if(((SipDcsLaesHeader*)(pHdr->pHeader) )->pContentPort==SIP_NULL)
    {
         *pErr= E_NO_EXIST;
         return SipFail;
    }

	*pContentPort = *(((SipDcsLaesHeader *)(pHdr->pHeader) )->pContentPort); 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getContentPortFromDcsLaesHdr");
	return SipSuccess;
}


SipBool sip_dcs_setContentPortInDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit dContentPort, SipError *pErr)
#else
	(pHdr, dContentPort, pErr)
	SipHeader *pHdr;
	SIP_U16bit dContentPort;
	SipError *pErr;
#endif
{
	SIP_U16bit *temp_port;
	SIPDEBUGFN("Entering function sip_setContentPortInDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	temp_port = ( SIP_U16bit * )fast_memget(ACCESSOR_MEM_ID, \
		sizeof(SIP_U16bit),pErr);
	*temp_port = dContentPort;
	if ( temp_port== SIP_NULL )
		return SipFail;
	if (((SipDcsLaesHeader *)(pHdr->pHeader))->pContentPort  != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&((( \
			SipDcsLaesHeader *)(pHdr->pHeader))->pContentPort), \
			pErr) == SipFail)
			return SipFail;
	}
	((SipDcsLaesHeader *)(pHdr->pHeader))->pContentPort= temp_port; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setContentPortInDcsLaesHdr");
	return SipSuccess;
}



SipBool sip_dcs_getKeyFromDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppKey, SipError *pErr)
#else
	(pHdr, ppKey, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppKey;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempKey;
	SIPDEBUGFN("Entering function sip_getKeyFromDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppKey == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempKey = ((SipDcsLaesHeader *)(pHdr ->pHeader))->pKey;
 	if (pTempKey == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppKey = (SIP_S8bit *) STRDUPACCESSOR (pTempKey);
	if (*ppKey == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppKey = pTempKey;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getKeyFromDcsLaesHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setKeyInDcsLaesHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pKey,  SipError *pErr)
#else
	(pHdr,pKey, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pKey;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempKey, *pKeyToken;
	SIPDEBUGFN("Entering function sip_setKeyInDcsLaesHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsLaes)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pKey == SIP_NULL)
                pTempKey = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempKey = (SIP_S8bit *) STRDUPACCESSOR(pKey);
		if (pTempKey == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempKey = pKey;
#endif
	}	
        
        pKeyToken = ((SipDcsLaesHeader *)(pHdr->pHeader))->pKey;
                
	if ( pKeyToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pKeyToken), pErr) == SipFail)
		{
			sip_freeString(pTempKey);
			return SipFail;
		}
	}
	
	((SipDcsLaesHeader *)(pHdr->pHeader))->pKey = pTempKey;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setKeyInDcsLaesHdr");
        return SipSuccess;
}


#ifdef SIP_BY_REFERENCE
SipBool sip_dcs_getCalledIdFromDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipUrl **ppCalledId, SipError *pErr)
#else
	(pHdr, ppCalledId, pErr)
	SipHeader *pHdr;
	SipUrl **ppCalledId;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getCalledIdFromDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipUrl *pCalledId, SipError *pErr)
#else
	(pHdr, pCalledId, pErr)
	SipHeader *pHdr;
	SipUrl *pCalledId;
	SipError *pErr;
#endif
#endif
{
	SipUrl *pTempUrl;
	SIPDEBUGFN("Entering function sip_getCalledIdFromDcsRedirectHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pCalledId == SIP_NULL)
#else
	if( ppCalledId == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRedirect)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
 	
 	pTempUrl = ((SipDcsRedirectHeader *)(pHdr ->pHeader))->pCalledId;
 	if (pTempUrl == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipUrl(pCalledId, pTempUrl, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTempUrl->dRefCount);
	*ppCalledId = pTempUrl;
#endif
 	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getCalledIdFromDcsRedirectHdr");
	return SipSuccess;
}


SipBool sip_dcs_setCalledIdInDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipUrl *pCalledId, SipError *pErr)
#else
	(pHdr, pCalledId, pErr)
	SipHeader *pHdr;
	SipUrl *pCalledId;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipUrl *pTempUrl;
#endif
	SIPDEBUGFN("Entering function sip_setCalledIdInDcsRedirectHdr"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRedirect)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif 	
 	if (pCalledId == SIP_NULL)
	{
		sip_freeSipUrl(((SipDcsRedirectHeader *)(pHdr ->pHeader))->pCalledId);
 		((SipDcsRedirectHeader *)(pHdr ->pHeader))->pCalledId = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE	
		if(sip_initSipUrl(&pTempUrl, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneSipUrl(pTempUrl, pCalledId, pErr) == SipFail)
		{
			sip_freeSipUrl(pTempUrl);
			return SipFail;
		}
		sip_freeSipUrl(((SipDcsRedirectHeader *)(pHdr ->pHeader))->pCalledId);
		((SipDcsRedirectHeader *)(pHdr ->pHeader))->pCalledId = pTempUrl;
#else
		HSS_LOCKEDINCREF(pCalledId->dRefCount);
		sip_freeSipUrl(((SipDcsRedirectHeader *)(pHdr ->pHeader))->pCalledId);
		((SipDcsRedirectHeader *)(pHdr ->pHeader))->pCalledId = pCalledId;
#endif
 	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setCalledIdInDcsRedirectHdr");
	return SipSuccess;
}
 

#ifdef SIP_BY_REFERENCE
SipBool sip_dcs_getRedirectorFromDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipUrl **ppRedirector, SipError *pErr)
#else
	(pHdr, ppRedirector, pErr)
	SipHeader *pHdr;
	SipUrl **ppRedirector;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getRedirectorFromDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipUrl *pRedirector, SipError *pErr)
#else
	(pHdr, pRedirector, pErr)
	SipHeader *pHdr;
	SipUrl *pRedirector;
	SipError *pErr;
#endif
#endif
{
	SipUrl *pTempUrl;
	SIPDEBUGFN("Entering function sip_getRedirectorFromDcsRedirectHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pRedirector == SIP_NULL)
#else
	if( ppRedirector == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRedirect)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
 	
 	pTempUrl = ((SipDcsRedirectHeader *)(pHdr ->pHeader))->pRedirector;
 	if (pTempUrl == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipUrl(pRedirector, pTempUrl, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTempUrl->dRefCount);
	*ppRedirector = pTempUrl;
#endif
 	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getRedirectorFromDcsRedirectHdr");
	return SipSuccess;
}


SipBool sip_dcs_setRedirectorInDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipUrl *pRedirector, SipError *pErr)
#else
	(pHdr, pRedirector, pErr)
	SipHeader *pHdr;
	SipUrl *pRedirector;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipUrl *pTempUrl;
#endif
	SIPDEBUGFN("Entering function sip_setRedirectorInDcsRedirectHdr"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRedirect)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif 	
 	if (pRedirector == SIP_NULL)
	{
		sip_freeSipUrl(((SipDcsRedirectHeader *)(pHdr ->pHeader))->pRedirector);
 		((SipDcsRedirectHeader *)(pHdr ->pHeader))->pRedirector = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE	
		if(sip_initSipUrl(&pTempUrl, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneSipUrl(pTempUrl, pRedirector, pErr) == SipFail)
		{
			sip_freeSipUrl(pTempUrl);
			return SipFail;
		}
		sip_freeSipUrl(((SipDcsRedirectHeader *)(pHdr ->pHeader))->pRedirector);
		((SipDcsRedirectHeader *)(pHdr ->pHeader))->pRedirector = pTempUrl;
#else
		HSS_LOCKEDINCREF(pRedirector->dRefCount);
		sip_freeSipUrl(((SipDcsRedirectHeader *)(pHdr ->pHeader))->pRedirector);
		((SipDcsRedirectHeader *)(pHdr ->pHeader))->pRedirector = pRedirector;
#endif
 	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setRedirectorInDcsRedirectHdr");
	return SipSuccess;
}


SipBool sip_dcs_getTagFromSessionHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppTag, SipError *pErr)
#else
	(pHdr, ppTag, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppTag;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempTag;
	SIPDEBUGFN("Entering function sip_getTagFromSessionHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppTag == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeSession)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempTag = ((SipSessionHeader *)(pHdr ->pHeader))->pTag;
 	if (pTempTag == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppTag = (SIP_S8bit *) STRDUPACCESSOR (pTempTag);
	if (*ppTag == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppTag = pTempTag;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTagFromSessionHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setTagInSessionHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pTag, SipError *pErr)
#else
	(pHdr, pTag, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pTag;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempTag, *pTagToken;
	SIPDEBUGFN("Entering function sip_setTagInSessionHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeSession)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pTag == SIP_NULL)
                pTempTag = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempTag = (SIP_S8bit *) STRDUPACCESSOR(pTag);
		if (pTempTag == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempTag = pTag;
#endif
	}	
        
        pTagToken = ((SipSessionHeader *)(pHdr->pHeader))->pTag;
                
	if ( pTagToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pTagToken), pErr) == SipFail)
		{
			sip_freeString(pTempTag);
			return SipFail;
		}
	}
	
	((SipSessionHeader *)(pHdr->pHeader))->pTag = pTempTag;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTagInSessionHdr");
        return SipSuccess;
}



SipBool sip_dcs_getHostFromDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppHost, SipError *pErr)
#else
	(pHdr, ppHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempHost;
	SIPDEBUGFN("Entering function sip_getHostFromDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppHost == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

 	pTempHost = ((SipDcsBillingInfoHeader *)(pHdr ->pHeader))->pHost;
 	if (pTempHost == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE	
 	*ppHost = (SIP_S8bit *) STRDUPACCESSOR (pTempHost);
	if (*ppHost == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppHost = pTempHost;
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getHostFromDcsBillingInfoHdr");
 	return SipSuccess;
}


SipBool sip_dcs_setHostInDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pHost, SipError *pErr)
#else
	(pHdr, pHost, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempHost, *pHostToken;
	SIPDEBUGFN("Entering function sip_setHostInDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pHost == SIP_NULL)
                pTempHost = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		pTempHost = (SIP_S8bit *) STRDUPACCESSOR(pHost);
		if (pTempHost == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempHost = pHost;
#endif
	}	
        
        pHostToken = ((SipDcsBillingInfoHeader *)(pHdr->pHeader))->pHost;
                
	if ( pHostToken != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pHostToken), pErr) == SipFail)
		{
			sip_freeString(pTempHost);
			return SipFail;
		}
	}
	
	((SipDcsBillingInfoHeader *)(pHdr->pHeader))->pHost = pTempHost;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setHostInDcsBillingInfoHdr");
        return SipSuccess;
}


SipBool sip_dcs_getPortFromDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit *pPort, SipError *pErr)
#else
	(pHdr, pPort, pErr)
	SipHeader *pHdr;
	SIP_U16bit *pPort;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_setPortInDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pPort == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if(((SipDcsBillingInfoHeader*)(pHdr->pHeader) )->pPort==SIP_NULL)
    {
         *pErr= E_NO_EXIST;
         return SipFail;
    }

	*pPort = *(((SipDcsBillingInfoHeader *)(pHdr->pHeader) )->pPort); 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getPortFromDcsBillingInfoHdr");
	return SipSuccess;
}


SipBool sip_dcs_setPortInDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U16bit dPort, SipError *pErr)
#else
	(pHdr, dPort, pErr)
	SipHeader *pHdr;
	SIP_U16bit dPort;
	SipError *pErr;
#endif
{
	SIP_U16bit *temp_port;
	SIPDEBUGFN("Entering function sip_setPortInDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	temp_port = ( SIP_U16bit * )fast_memget(ACCESSOR_MEM_ID, \
		sizeof(SIP_U16bit),pErr);
	*temp_port = dPort;
	if ( temp_port== SIP_NULL )
		return SipFail;
	if ((((SipDcsBillingInfoHeader*)(pHdr->pHeader))->pPort)  != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(((( \
			SipDcsBillingInfoHeader*) \
			(pHdr->pHeader))->pPort)), pErr) == SipFail)
			return SipFail;
	}
	((SipDcsBillingInfoHeader*)(pHdr->pHeader))->pPort= temp_port; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setPortInDcsBillingInfoHdr");
	return SipSuccess;
}


SipBool sip_dcs_getAcctEntryCountFromDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getAcctEntryCountFromDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipDcsBillingInfoHeader *)(pHdr->pHeader))->slAcctEntry), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAcctEntryCountFromDcsBillingInfoHdr");	
	return SipSuccess;
}


#ifdef SIP_BY_REFERENCE 
SipBool sip_dcs_getAcctEntryAtIndexFromDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipDcsAcctEntry **ppAcctEntry, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, ppAcctEntry, dIndex, pErr)
	SipHeader *pHdr;
	SipDcsAcctEntry **ppAcctEntry;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#else
SipBool sip_dcs_getAcctEntryAtIndexFromDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipDcsAcctEntry *pAcctEntry, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pAcctEntry, dIndex, pErr)
	SipHeader *pHdr;
	SipDcsAcctEntry *pAcctEntry;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid pElementFromList;
	SipDcsAcctEntry *pTempAcctEntry;
	SIPDEBUGFN("Entering function sip_getAcctEntryAtIndexFromDcsBillingInfoHdr");	
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pAcctEntry == SIP_NULL)
#else
	if( ppAcctEntry == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	
	if (sip_listGetAt( &(((SipDcsBillingInfoHeader *)(pHdr->pHeader))->slAcctEntry), dIndex,  \
		&pElementFromList, pErr) == SipFail)
		return SipFail;

	if (pElementFromList == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempAcctEntry = (SipDcsAcctEntry *)pElementFromList;
#ifndef SIP_BY_REFERENCE	
	if (__sip_dcs_cloneSipDcsAcctEntry(pAcctEntry, pTempAcctEntry, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTempAcctEntry->dRefCount);
	*ppAcctEntry = pTempAcctEntry;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAcctEntryAtIndexFromDcsBillingInfoHdr");
	return SipSuccess;
}


SipBool sip_dcs_setAcctEntryAtIndexInDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipDcsAcctEntry *pAcctEntry, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pAcctEntry, dIndex, pErr)
	SipHeader *pHdr;
	SipDcsAcctEntry *pAcctEntry;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipDcsAcctEntry *pTempAcctEntry;
	SIPDEBUGFN("Entering function sip_setAcctEntryAtIndexInDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pAcctEntry == SIP_NULL )
		pTempAcctEntry = SIP_NULL;
	else
	{
		/* if (validateSipViaAcctEntryType(pAcctEntry->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_dcs_initSipDcsAcctEntry(&pTempAcctEntry, pErr) == SipFail)
			return SipFail;
		if (__sip_dcs_cloneSipDcsAcctEntry(pTempAcctEntry, pAcctEntry, pErr) == SipFail)
		{
			sip_dcs_freeSipDcsAcctEntry (pTempAcctEntry); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pAcctEntry->dRefCount);
		pTempAcctEntry = pAcctEntry;
#endif
	}
			
	if( sip_listSetAt( &(((SipDcsBillingInfoHeader *)(pHdr->pHeader))->slAcctEntry),  \
		dIndex, (SIP_Pvoid)(pTempAcctEntry), pErr) == SipFail)
	{
		if (pTempAcctEntry != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_dcs_freeSipDcsAcctEntry (pTempAcctEntry); 
#else
		HSS_LOCKEDDECREF(pAcctEntry->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAcctEntryAtIndexInDcsBillingInfoHdr");
	return SipSuccess;
}


SipBool sip_dcs_insertAcctEntryAtIndexInDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipDcsAcctEntry *pAcctEntry, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pAcctEntry, dIndex, pErr)
	SipHeader *pHdr;
	SipDcsAcctEntry *pAcctEntry;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipDcsAcctEntry *pTempAcctEntry;
	SIPDEBUGFN("Entering function sip_insertAcctEntryAtIndexInDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	if ( pAcctEntry == SIP_NULL )
		pTempAcctEntry = SIP_NULL;
	else
	{
		/* if (validateSipViaAcctEntryType(pAcctEntry->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_dcs_initSipDcsAcctEntry(&pTempAcctEntry, pErr) == SipFail)
			return SipFail;
		if (__sip_dcs_cloneSipDcsAcctEntry(pTempAcctEntry, pAcctEntry, pErr) == SipFail)
		{
			sip_dcs_freeSipDcsAcctEntry (pTempAcctEntry); 
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pAcctEntry->dRefCount);
		pTempAcctEntry = pAcctEntry;
#endif
	}
			
	if( sip_listInsertAt( &(((SipDcsBillingInfoHeader *)(pHdr->pHeader))->slAcctEntry),  \
		dIndex, (SIP_Pvoid)(pTempAcctEntry), pErr) == SipFail)
	{
		if (pTempAcctEntry != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_dcs_freeSipDcsAcctEntry (pTempAcctEntry); 
#else
		HSS_LOCKEDDECREF(pAcctEntry->dRefCount);
#endif
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertAcctEntryAtIndexInDcsBillingInfoHdr");
	return SipSuccess;
}


SipBool sip_dcs_deleteAcctEntryAtIndexInDcsBillingInfoHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteAcctEntryAtIndexInDcsBillingInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsBillingInfo)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipDcsBillingInfoHeader *)(pHdr->pHeader))->slAcctEntry), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteAcctEntryAtIndexInDcsBillingInfoHdr");	
	return SipSuccess;
}



SipBool sip_dcs_getChargeNumFromDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppChargeNum, SipError *pErr)
#else
	(pAcctEntry, ppChargeNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit **ppChargeNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTempChargeNum;
	SIPDEBUGFN ("Entering function sip_dcs_getChargeNumFromDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppChargeNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	pTempChargeNum = ((SipDcsAcctEntry *) pAcctEntry)->pChargeNum; 
	
	if( pTempChargeNum == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE
	 *ppChargeNum = pTempChargeNum;
#else
	dLength = strlen(pTempChargeNum);
	*ppChargeNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppChargeNum == SIP_NULL )
		return SipFail;

	strcpy( *ppChargeNum , pTempChargeNum );
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_getChargeNumFromDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_setChargeNumInDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pChargeNum, SipError *pErr) 
#else
	(pAcctEntry, pChargeNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit *pChargeNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTempChargeNum;
#endif
	SIPDEBUGFN ("Entering function sip_dcs_setChargeNumInDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pAcctEntry->pChargeNum != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( pAcctEntry->pChargeNum), pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pAcctEntry->pChargeNum = pChargeNum;
#else
	if( pChargeNum == SIP_NULL)
		pTempChargeNum = SIP_NULL;
	else
	{
		dLength = strlen( pChargeNum );
		pTempChargeNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTempChargeNum== SIP_NULL )
			return SipFail;
		
		strcpy( pTempChargeNum, pChargeNum );

	}

	pAcctEntry->pChargeNum = pTempChargeNum; 
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_setChargeNumInDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_getCallingNumFromDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppCallingNum, SipError *pErr) 
#else
	(pAcctEntry, ppCallingNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit **ppCallingNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTempCallingNum;
	SIPDEBUGFN ("Entering function sip_dcs_getCallingNumFromDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppCallingNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	pTempCallingNum = ((SipDcsAcctEntry *) pAcctEntry)->pCallingNum; 
	
	if( pTempCallingNum == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE
	 *ppCallingNum = pTempCallingNum;
#else
	dLength = strlen(pTempCallingNum);
	*ppCallingNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppCallingNum == SIP_NULL )
		return SipFail;

	strcpy( *ppCallingNum , pTempCallingNum );
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_getCallingNumFromDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_setCallingNumInDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pCallingNum, SipError *pErr)
#else
	(pAcctEntry, pCallingNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit *pCallingNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTempCallingNum;
#endif
	SIPDEBUGFN ("Entering function sip_dcs_setCallingNumInDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pAcctEntry->pCallingNum != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( pAcctEntry->pCallingNum), pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pAcctEntry->pCallingNum = pCallingNum;
#else
	if( pCallingNum == SIP_NULL)
		pTempCallingNum = SIP_NULL;
	else
	{
		dLength = strlen( pCallingNum );
		pTempCallingNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTempCallingNum== SIP_NULL )
			return SipFail;
		
		strcpy( pTempCallingNum, pCallingNum );

	}

	pAcctEntry->pCallingNum = pTempCallingNum; 
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_setCallingNumInDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_getCalledNumFromDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppCalledNum, SipError *pErr)
#else
	(pAcctEntry, ppCalledNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit **ppCalledNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTempCalledNum;
	SIPDEBUGFN ("Entering function sip_dcs_getCalledNumFromDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppCalledNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	pTempCalledNum = ((SipDcsAcctEntry *) pAcctEntry)->pCalledNum; 
	
	if( pTempCalledNum == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE
	 *ppCalledNum = pTempCalledNum;
#else
	dLength = strlen(pTempCalledNum);
	*ppCalledNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppCalledNum == SIP_NULL )
		return SipFail;

	strcpy( *ppCalledNum , pTempCalledNum );
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_getCalledNumFromDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_setCalledNumInDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pCalledNum, SipError *pErr)
#else
	(pAcctEntry, pCalledNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit *pCalledNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTempCalledNum;
#endif
	SIPDEBUGFN ("Entering function sip_dcs_setCalledNumInDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pAcctEntry->pCalledNum != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( pAcctEntry->pCalledNum), pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pAcctEntry->pCalledNum = pCalledNum;
#else
	if( pCalledNum == SIP_NULL)
		pTempCalledNum = SIP_NULL;
	else
	{
		dLength = strlen( pCalledNum );
		pTempCalledNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTempCalledNum== SIP_NULL )
			return SipFail;
		
		strcpy( pTempCalledNum, pCalledNum );

	}

	pAcctEntry->pCalledNum = pTempCalledNum; 
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_setCalledNumInDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_getRoutingNumFromDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppRoutingNum, SipError *pErr)
#else
	(pAcctEntry, ppRoutingNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit **ppRoutingNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTempRoutingNum;
	SIPDEBUGFN ("Entering function sip_dcs_getRoutingNumFromDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppRoutingNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	pTempRoutingNum = ((SipDcsAcctEntry *) pAcctEntry)->pRoutingNum; 
	
	if( pTempRoutingNum == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE
	 *ppRoutingNum = pTempRoutingNum;
#else
	dLength = strlen(pTempRoutingNum);
	*ppRoutingNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppRoutingNum == SIP_NULL )
		return SipFail;

	strcpy( *ppRoutingNum , pTempRoutingNum );
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_getRoutingNumFromDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_setRoutingNumInDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pRoutingNum, SipError *pErr)
#else
	(pAcctEntry, pRoutingNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit *pRoutingNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTempRoutingNum;
#endif
	SIPDEBUGFN ("Entering function sip_dcs_setRoutingNumInDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pAcctEntry->pRoutingNum != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( pAcctEntry->pRoutingNum), pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pAcctEntry->pRoutingNum = pRoutingNum;
#else
	if( pRoutingNum == SIP_NULL)
		pTempRoutingNum = SIP_NULL;
	else
	{
		dLength = strlen( pRoutingNum );
		pTempRoutingNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTempRoutingNum== SIP_NULL )
			return SipFail;
		
		strcpy( pTempRoutingNum, pRoutingNum );

	}

	pAcctEntry->pRoutingNum = pTempRoutingNum; 
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_setRoutingNumInDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_getLocationRoutingNumFromDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppLocationRoutingNum, SipError *pErr)
#else
	(pAcctEntry, ppLocationRoutingNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit **ppLocationRoutingNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTempLocationRoutingNum;
	SIPDEBUGFN ("Entering function sip_dcs_getLocationRoutingNumFromDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppLocationRoutingNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	pTempLocationRoutingNum = ((SipDcsAcctEntry *) pAcctEntry)->pLocationRoutingNum; 
	
	if( pTempLocationRoutingNum == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE
	 *ppLocationRoutingNum = pTempLocationRoutingNum;
#else
	dLength = strlen(pTempLocationRoutingNum);
	*ppLocationRoutingNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppLocationRoutingNum == SIP_NULL )
		return SipFail;

	strcpy( *ppLocationRoutingNum , pTempLocationRoutingNum );
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_getLocationRoutingNumFromDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_setLocationRoutingNumInDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pLocationRoutingNum, SipError *pErr)
#else
	(pAcctEntry, pLocationRoutingNum, pErr)
	SipDcsAcctEntry *pAcctEntry;
	SIP_S8bit *pLocationRoutingNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTempLocationRoutingNum;
#endif
	SIPDEBUGFN ("Entering function sip_dcs_setLocationRoutingNumInDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pAcctEntry == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pAcctEntry->pLocationRoutingNum != SIP_NULL ) 
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( pAcctEntry->pLocationRoutingNum), pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pAcctEntry->pLocationRoutingNum = pLocationRoutingNum;
#else
	if( pLocationRoutingNum == SIP_NULL)
		pTempLocationRoutingNum = SIP_NULL;
	else
	{
		dLength = strlen( pLocationRoutingNum );
		pTempLocationRoutingNum = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTempLocationRoutingNum== SIP_NULL )
			return SipFail;
		
		strcpy( pTempLocationRoutingNum, pLocationRoutingNum );

	}

	pAcctEntry->pLocationRoutingNum = pTempLocationRoutingNum; 
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN ("Exiting function sip_dcs_setLocationRoutingNumInDcsAcctEntry");
	return SipSuccess;
}


SipBool sip_dcs_getNumFromDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pNum, SipError *pErr)
#else
	(pHdr, pNum, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pNum;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_getNumFromDcsRedirectHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRedirect)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	*pNum = ( (SipDcsRedirectHeader *)(pHdr->pHeader) )->dNum; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getNumFromDcsRedirectHdr");
	return SipSuccess;
}


SipBool sip_dcs_setNumInDcsRedirectHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dNum, SipError *pErr)
#else	
	(pHdr, dNum, pErr)
	SipHeader *pHdr;
	SIP_U32bit dNum;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_setNumInDcsRedirectHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeDcsRedirect)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	((SipDcsRedirectHeader *)(pHdr->pHeader))->dNum = dNum; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_setNumInDcsRedirectHdr");
	return SipSuccess;
}
