/*************************************************************************
 ** FUNCTION:
 **	 	This file has the global prototype definitions for the Request APIs

 **************************************************************************
 **
 ** FILENAME:
 ** req1csb.c
 **
 ** DESCRIPTION:
 **
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 18/11/99	    R.Preethy			--			Original
 **
 **	Copyright 1999, Hughes Software Systems, Ltd.
 ***************************************************************************/

#include "sipstruct.h"
#include "sipcommon.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siplist.h"
#include "sipcommon.h"
#include "request.h"
#include "sipinternal.h"
#include "sipvalidate.h"
#include "sipclone.h"


/*********************************************************
** FUNCTION:sip_getCredentialsFromAuthorizationHdr
**
** DESCRIPTION: This function sets the Credentials in a SIP
**		Authorization pHeader
**
**********************************************************/
SipBool sip_getCredentialsFromAuthorizationHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr,	/*Authorization Header */
	SipGenericCredential **ppCr,
	SipError *pErr)
#else
	(SipHeader *pHdr,	/*Authorization Header */
	SipGenericCredential *pCr,
	SipError *pErr)
#endif
#else     /* ANSI_PROTO */
#ifdef SIP_BY_REFERENCE
	( pHdr, ppCr, pErr )
	SipHeader * pHdr;    /* Authorization Header */
	SipGenericCredential **ppCr;
	SipError * pErr;
#else
	( pHdr, pCr, pErr )
	SipHeader * pHdr;    /* Authorization Header */
	SipGenericCredential *pCr;
	SipError * pErr;
#endif
#endif
{
	SipGenericCredential *pFrom;
	SIPDEBUGFN( "Entering getCredentialsFromAuthorizationHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( ppCr == SIP_NULL)
#else
	if ( pCr == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeAuthorization) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pFrom=((SipAuthorizationHeader *)(pHdr->pHeader))->pCredential;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppCr = pFrom;
#else
	if(__sip_cloneCredential(pCr,pFrom,pErr)==SipFail)
	{
		return SipFail;
	}
#endif

	SIPDEBUGFN( "Exiting getCredentialsFromAuthorizationHdr ");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setCredentialsInAuthorizationHdr
**
** DESCRIPTION: This function sets teh dCredentials in a SIP
**		Authorization pHeader
**
**********************************************************/
SipBool sip_setCredentialsInAuthorizationHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SipGenericCredential *pCredentials,
	SipError *pErr)
#else
	( pHdr, pCredentials, pErr )
	  SipHeader * pHdr;
	  SipGenericCredential *pCredentials;
	  SipError * pErr;
#endif
{
	SipGenericCredential *pTo;
	SIPDEBUGFN( "Entering setCredentialsInAuthorizationHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeAuthorization) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTo=((SipAuthorizationHeader *)(pHdr->pHeader))->pCredential;
	if (pCredentials==SIP_NULL)
	{
		if (pTo != SIP_NULL)
			sip_freeSipGenericCredential(pTo);
		((SipAuthorizationHeader *)(pHdr->pHeader))->pCredential = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipGenericCredential(&pTo, pCredentials->dType, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneCredential(pTo, pCredentials, pErr) == SipFail)
		{
			sip_freeSipGenericCredential(pTo);
			return SipFail;
		}
		sip_freeSipGenericCredential(((SipAuthorizationHeader *)(pHdr->pHeader))->pCredential);
		((SipAuthorizationHeader *)(pHdr->pHeader))->pCredential = pTo;
#else		
		if (pTo != SIP_NULL)
			sip_freeSipGenericCredential(pTo);

		HSS_LOCKEDINCREF(pCredentials->dRefCount);
		((SipAuthorizationHeader *)(pHdr->pHeader))->pCredential = pCredentials;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setCredentialsInAuthorizationHdr");
	return SipSuccess;

}



/* call-transfer */
/*********************************************************
** FUNCTION:sip_getDispNameFromReferToHdr
**
** DESCRIPTION: This function retrieves the display-pName field
**		from a SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_getDispNameFromReferToHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppDispname,
	SipError *pErr)
#else
	( pHdr, ppDispname ,pErr)
	  SipHeader * pHdr;	/* ReferTo pHeader */
	  SIP_S8bit **ppDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_dispname;
	SIPDEBUGFN( "Entering getDispNameFromReferToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppDispname == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferTo) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_dispname = ( (SipReferToHeader *) (pHdr->pHeader) )->pDispName;


	if( pTemp_dispname == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppDispname = pTemp_dispname;
#else

	dLength = strlen(pTemp_dispname);
	*ppDispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppDispname == SIP_NULL )
		return SipFail;

	strcpy( *ppDispname, pTemp_dispname );
#endif

	SIPDEBUGFN( "Exiting getDispNameFromReferToHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setDispNameInReferToHdr
**
** DESCRIPTION: This function sets the display-pName field in
**		a SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_setDispNameInReferToHdr
#ifdef ANSI_PROTO
(SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr)

#else
	( pHdr, pDispname , pErr )
	  SipHeader *pHdr;	/*ReferTo Header */
	  SIP_S8bit *pDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_dispname;
#endif
	SipReferToHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setDispNameInReferToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferTo) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_hdr=(SipReferToHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pDispName !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pDispName),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipReferToHeader *)(pHdr->pHeader))->pDispName = pDispname;
#else
	if( pDispname == SIP_NULL)
		pTemp_dispname = SIP_NULL;
	else
	{
		dLength = strlen( pDispname );
		pTemp_dispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_dispname == SIP_NULL )
			return SipFail;

		strcpy( pTemp_dispname, pDispname );
	}
	pTemp_hdr->pDispName = pTemp_dispname;
#endif
	SIPDEBUGFN( "Exiting setDispNameInReferToHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getAddrSpecFromReferToHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field
**		from a SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_getAddrSpecFromReferToHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr,
	SipAddrSpec **ppAddrspec,
	SipError *pErr)
#else
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
#endif
#else    /* ANSI_PROTO */
#ifdef SIP_BY_REFERENCE
	( pHdr, ppAddrspec, pErr )
	  SipHeader * pHdr;    /* ReferTo Header */
	  SipAddrSpec **ppAddrspec;
	  SipError * pErr;
#else
	  ( pHdr, pAddrspec, pErr )
	  SipHeader * pHdr;    /* ReferTo Header */
	  SipAddrSpec *pAddrspec;
	  SipError * pErr;
#endif
#endif
{
	SipAddrSpec *pFrom;
	SIPDEBUGFN( "Entering getAddrSpecFromReferToHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( ppAddrspec == SIP_NULL)
#else
	if ( pAddrspec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferTo) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pFrom=((SipReferToHeader *)(pHdr->pHeader))->pAddrSpec;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;

	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppAddrspec = pFrom;
#else
	if(__sip_cloneAddrSpec(pAddrspec,pFrom,pErr)==SipFail)
	{
		if(pAddrspec->dType==SipAddrReqUri)
		{
			sip_freeString(pAddrspec->u.pUri);
		}
		else if((pAddrspec->dType==SipAddrSipUri) \
			|| (pAddrspec->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl(pAddrspec->u.pSipUrl);
		}
		pAddrspec->dType=SipAddrAny;
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getAddrSpecFromReferToHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setAddrSpecInReferToHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a
**		SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_setAddrSpecInReferToHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
#else
	( pHdr, pAddrspec, pErr )
	  SipHeader * pHdr;
	  SipAddrSpec *pAddrspec;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTo;
#endif
	SIPDEBUGFN( "Entering setAddrSpecInReferToHdr ");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if (pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( (pHdr->dType != SipHdrTypeReferTo) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	sip_freeSipAddrSpec(((SipReferToHeader *)(pHdr->pHeader))->pAddrSpec );
	if (pAddrspec==SIP_NULL)
	{
		((SipReferToHeader *)(pHdr->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrspec->dRefCount);
		((SipReferToHeader *)(pHdr->pHeader))->pAddrSpec = pAddrspec;
#else
		if(sip_initSipAddrSpec(&pTo,SipAddrAny,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneAddrSpec(pTo,pAddrspec,pErr)==SipFail)
		{
			sip_freeSipAddrSpec(pTo);
			return SipFail;
		}
		((SipReferToHeader *)(pHdr->pHeader))->pAddrSpec = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setAddrSpecInReferToHdr");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getParamCountFromReferToHdr
**
** DESCRIPTION: This function gets the paramater count in slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_getParamCountFromReferToHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromReferToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeReferTo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipReferToHeader *)(pHdr->pHeader))->slParams),\
		pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromReferToHdr");
	return SipSuccess;
}



/*********************************************************
** FUNCTION:sip_getParamAtIndexFromReferToHdr
**
** DESCRIPTION: This function gets the paramater from slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_getParamAtIndexFromReferToHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromReferToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeReferTo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipReferToHeader *)(pHdr->pHeader))->slParams),\
		dIndex,  &element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromReferToHdr");
	return SipSuccess;
}



/*********************************************************
** FUNCTION:sip_setParamAtIndexInReferToHdr
**
** DESCRIPTION: This function sets the paramater in slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_setParamAtIndexInReferToHdr
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
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInReferToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypeReferTo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipReferToHeader *)(pHdr->pHeader))->slParams),\
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInReferToHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_insertParamAtIndexInReferToHdr
**
** DESCRIPTION: This function inserts the paramater in slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_insertParamAtIndexInReferToHdr
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
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInReferToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypeReferTo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipReferToHeader *)(pHdr->pHeader))->slParams),\
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInReferToHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_deleteParamAtIndexInReferToHdr
**
** DESCRIPTION: This function  deletes the paramater from slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
SipBool sip_deleteParamAtIndexInReferToHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInReferToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeReferTo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipReferToHeader *)(pHdr->pHeader))->slParams),\
		dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInReferToHdr");
	return SipSuccess;
}
/* call-transfer */

/*********************************************************
** FUNCTION:sip_getDispNameFromReferredByHdr
**
** DESCRIPTION: This function retrieves the display-pName field
**		from a SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_getDispNameFromReferredByHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppDispname,
	SipError *pErr)
#else
	( pHdr, ppDispname ,pErr)
	  SipHeader * pHdr;	/* ReferredBy pHeader */
	  SIP_S8bit **ppDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_dispname;
	SIPDEBUGFN( "Entering getDispNameFromReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppDispname == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferredBy) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_dispname = ( (SipReferredByHeader *) (pHdr->pHeader) )->pDispName;


	if( pTemp_dispname == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppDispname = pTemp_dispname;
#else

	dLength = strlen(pTemp_dispname);
	*ppDispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppDispname == SIP_NULL )
		return SipFail;

	strcpy( *ppDispname, pTemp_dispname );
#endif

	SIPDEBUGFN( "Exiting getDispNameFromReferredByHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setDispNameInReferredByHdr
**
** DESCRIPTION: This function sets the display-pName field in
**		a SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_setDispNameInReferredByHdr
#ifdef ANSI_PROTO
(SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr)

#else
	( pHdr, pDispname , pErr )
	  SipHeader *pHdr;	/*ReferredBy Header */
	  SIP_S8bit *pDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_dispname;
#endif
	SipReferredByHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setDispNameInReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferredBy) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_hdr=(SipReferredByHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pDispName !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pDispName),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipReferredByHeader *)(pHdr->pHeader))->pDispName = pDispname;
#else
	if( pDispname == SIP_NULL)
		pTemp_dispname = SIP_NULL;
	else
	{
		dLength = strlen( pDispname );
		pTemp_dispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_dispname == SIP_NULL )
			return SipFail;

		strcpy( pTemp_dispname, pDispname );
	}
	pTemp_hdr->pDispName = pTemp_dispname;
#endif
	SIPDEBUGFN( "Exiting setDispNameInReferredByHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_getMsgIdFromReferredByHdr
**
** DESCRIPTION: This function retrieves the MsgId field
**		from a SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_getMsgIdFromReferredByHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppMsgId,
	SipError *pErr)
#else
	( pHdr, ppMsgId ,pErr)
	  SipHeader * pHdr;	/* ReferredBy pHeader */
	  SIP_S8bit **ppMsgId;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_MsgId;
	SIPDEBUGFN( "Entering getMsgIdFromReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppMsgId == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferredBy) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_MsgId = ( (SipReferredByHeader *) (pHdr->pHeader) )->pMsgId;


	if( pTemp_MsgId == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppMsgId = pTemp_MsgId;
#else

	dLength = strlen(pTemp_MsgId);
	*ppMsgId = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppMsgId == SIP_NULL )
		return SipFail;

	strcpy( *ppMsgId, pTemp_MsgId );
#endif

	SIPDEBUGFN( "Exiting getMsgIdFromReferredByHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setMsgIdInReferredByHdr
**
** DESCRIPTION: This function sets the msgId field in
**		a SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_setMsgIdInReferredByHdr
#ifdef ANSI_PROTO
(SipHeader *pHdr,SIP_S8bit *pMsgId, SipError *pErr)

#else
	( pHdr, pMsgId , pErr )
	  SipHeader *pHdr;	/*ReferredBy Header */
	  SIP_S8bit *pMsgId;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_MsgId;
#endif
	SipReferredByHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setMsgIdInReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferredBy) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_hdr=(SipReferredByHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pMsgId !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pMsgId),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipReferredByHeader *)(pHdr->pHeader))->pMsgId = pMsgId;
#else
	if( pMsgId == SIP_NULL)
		pTemp_MsgId = SIP_NULL;
	else
	{
		dLength = strlen( pMsgId );
		pTemp_MsgId = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_MsgId == SIP_NULL )
			return SipFail;

		strcpy( pTemp_MsgId, pMsgId );
	}
	pTemp_hdr->pMsgId = pTemp_MsgId;
#endif
	SIPDEBUGFN( "Exiting setMsgIdInReferredByHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_getReferrerFromReferredByHdr
**
** DESCRIPTION: This function gets the referrer url from
**		a SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_getReferrerFromReferredByHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec *pAddrSpecReferrer, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpecReferrer, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppAddrSpecReferrer, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpecReferrer;
	SipError *pErr;
#else
	(pHdr, pAddrSpecReferrer, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpecReferrer;
	SipError *pErr;
#endif
#endif
{
	SipAddrSpec *pFrom;
	SIPDEBUGFN( "Entering getReferrerFromReferredByHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( ppAddrSpecReferrer == SIP_NULL)
#else
	if ( pAddrSpecReferrer == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeReferredBy) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pFrom=((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferrer;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;

	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppAddrSpecReferrer = pFrom;
#else
	if(__sip_cloneAddrSpec(pAddrSpecReferrer,pFrom,pErr)==SipFail)
	{
		if(pAddrSpecReferrer->dType==SipAddrReqUri)
		{
			sip_freeString(pAddrSpecReferrer->u.pUri);
		}
		else if((pAddrSpecReferrer->dType==SipAddrSipUri) \
				|| (pAddrSpecReferrer->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl(pAddrSpecReferrer->u.pSipUrl);
		}
		pAddrSpecReferrer->dType=SipAddrAny;
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getReferrerFromReferredByHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setReferrerInReferredByHdr
**
** DESCRIPTION: This function sets the referrer url in
**		a SIP ReferredBy pHeader
**
**********************************************************/
extern SipBool sip_setReferrerInReferredByHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec *pAddrSpecReferrer, SipError *pErr)
#else
	(pHdr, pAddrSpecReferrer, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpecReferrer;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTo;
#endif
	SIPDEBUGFN( "Entering setReferrerInReferredByHdr ");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ((pHdr->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	sip_freeSipAddrSpec\
				(((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferrer );
	if (pAddrSpecReferrer==SIP_NULL)
	{
		((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferrer = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrSpecReferrer->dRefCount);
		((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferrer = \
															pAddrSpecReferrer;
#else
		if(sip_initSipAddrSpec(&pTo,SipAddrAny,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneAddrSpec(pTo,pAddrSpecReferrer,pErr)==SipFail)
		{
			sip_freeSipAddrSpec(pTo);
			return SipFail;
		}
		((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferrer = pTo;
#endif
	}	
	SIPDEBUGFN( "Exiting setReferrerInReferredByHdr");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getReferencedFromReferredByHdr
**
** DESCRIPTION: This function gets the referrer url from
**		a SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_getReferencedFromReferredByHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec *pAddrSpecReferenced, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpecReferenced, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppAddrSpecReferenced, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpecReferenced;
	SipError *pErr;
#else
	(pHdr, pAddrSpecReferenced, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpecReferenced;
	SipError *pErr;
#endif
#endif
{
	SipAddrSpec *pFrom;
	SIPDEBUGFN( "Entering getReferencedFromReferredByHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( ppAddrSpecReferenced == SIP_NULL)
#else
	if ( pAddrSpecReferenced == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pFrom=((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferenced;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;

	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppAddrSpecReferenced = pFrom;
#else
	if(__sip_cloneAddrSpec(pAddrSpecReferenced,pFrom,pErr)==SipFail)
	{
		if(pAddrSpecReferenced->dType==SipAddrReqUri)
		{
			sip_freeString(pAddrSpecReferenced->u.pUri);
		}
		else if((pAddrSpecReferenced->dType==SipAddrSipUri) \
				 || (pAddrSpecReferenced->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl(pAddrSpecReferenced->u.pSipUrl);
		}
		pAddrSpecReferenced->dType=SipAddrAny;
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getReferencedFromReferredByHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setReferencedInReferredByHdr
**
** DESCRIPTION: This function sets the referrer url in
**		a SIP ReferredBy pHeader
**
**********************************************************/
extern SipBool sip_setReferencedInReferredByHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipAddrSpec *pAddrSpecReferenced, SipError *pErr)
#else
	(pHdr, pAddrSpecReferenced, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpecReferenced;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTo;
#endif
	SIPDEBUGFN( "Entering setReferencedInReferredByHdr ");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ((pHdr->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	sip_freeSipAddrSpec\
			(((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferenced );
	if (pAddrSpecReferenced==SIP_NULL)
	{
		((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferenced =\
																	SIP_NULL;	
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrSpecReferenced->dRefCount);
		((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferenced = \
															pAddrSpecReferenced;
#else
		if(sip_initSipAddrSpec(&pTo,SipAddrAny,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneAddrSpec(pTo,pAddrSpecReferenced,pErr)==SipFail)
		{
			sip_freeSipAddrSpec(pTo);
			return SipFail;
		}
		((SipReferredByHeader *)(pHdr->pHeader))->pAddrSpecReferenced = pTo;
#endif
	}	
	SIPDEBUGFN( "Exiting setReferencedInReferredByHdr");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getParamCountFromReferredByHd
**
** DESCRIPTION: This function gets the paramater count in slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_getParamCountFromReferredByHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipReferredByHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromReferredByHdr");
	return SipSuccess;
}



/*********************************************************
** FUNCTION:sip_getParamAtIndexFromReferredByHdr
**
** DESCRIPTION: This function gets the paramater from slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_getParamAtIndexFromReferredByHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipReferredByHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromReferredByHdr");
	return SipSuccess;
}



/*********************************************************
** FUNCTION:sip_setParamAtIndexInReferredByHdr
**
** DESCRIPTION: This function sets the paramater in slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_setParamAtIndexInReferredByHdr
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
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipReferredByHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInReferredByHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_insertParamAtIndexInReferredByHdr
**
** DESCRIPTION: This function inserts the paramater in slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_insertParamAtIndexInReferredByHdr
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
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipReferredByHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInReferredByHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_deleteParamAtIndexInReferredByHdr
**
** DESCRIPTION: This function  deletes the paramater from slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
SipBool sip_deleteParamAtIndexInReferredByHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInReferredByHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeReferredBy))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipReferredByHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInReferredByHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_getHideTypeFromHideHdr
**
** DESCRIPTION: This function retrieves the hide dType field
**		from a SIP Hide pHeader
**
** NOTE: The type field in the Hide Header has been changed
** 		 to SIP_S8bit* This API exists only for conformance
**		 with the previous versions. Please use the other API
**		 to retrieve the type from the Hide Header.
**********************************************************/
SipBool sip_getHideTypeFromHideHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,	/*Hide Header */
	en_HideType *pHtype,
	SipError *pErr)
#else
	( pHdr, pHtype, pErr )
	  SipHeader * pHdr;    /* Hide Header */
	  en_HideType *pHtype;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getHideTypeFromHideHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pHtype == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeHide))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader==SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

#endif
	if ((strcasecmp(((SipHideHeader *)(pHdr->pHeader))->pType, "Hop")) == 0)
		*pHtype = SipHideHop;
	else if ((strcasecmp(((SipHideHeader *)(pHdr->pHeader))->pType, "Route")) == 0)
		*pHtype = SipHideRoute;
	else
		*pHtype = SipHideOther;

	SIPDEBUGFN( "Exiting getHideTypeFromHideHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setHideTypeInHideHdr
**
** DESCRIPTION: This function sets the hide dType in a SIP
**		Hide pHeader
**
** NOTE: The type field in the Hide Header has been changed
** 		 to SIP_S8bit* This API exists only for conformance
**		 with the previous versions. Please use the other API
**		 to set the type in the Hide Header.
**********************************************************/
SipBool sip_setHideTypeInHideHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,	/*Hide Header */
	en_HideType dHtype,
	SipError *pErr)
#else
	( pHdr, dHtype, pErr )
	  SipHeader * pHdr;    /* Hide Header */
	  en_HideType dHtype;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setHideTypeInHideHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeHide))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if (pHdr->pHeader== SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if(validateHideType(&dHtype,pErr)==SipFail)
		return SipFail;
	if (dHtype == SipHideHop)
	{
		((SipHideHeader *) (pHdr->pHeader))->pType = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, 4, pErr);
		if (((SipHideHeader *) (pHdr->pHeader))->pType == NULL)
			return SipFail;
		strcpy(((SipHideHeader *) (pHdr->pHeader))->pType, "Hop");
	}
	else if (dHtype == SipHideRoute)
	{
		((SipHideHeader *) (pHdr->pHeader))->pType = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, 6, pErr);
		if (((SipHideHeader *) (pHdr->pHeader))->pType == NULL)
			return SipFail;
		strcpy(((SipHideHeader *) (pHdr->pHeader))->pType, "Route");
	}
	SIPDEBUGFN( "Exiting setHideTypeInHideHdr");
	*pErr=E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:sip_getTypeFromHideHdr
**
** DESCRIPTION: This function retrieves the type field
**		from a SIP Hide pHeader
**
** NOTE: This is the new API added after the type in the
** 		 hide header was converted to SIP_S8bit*
**********************************************************/
SipBool sip_getTypeFromHideHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,	/*Hide Header */
	SIP_S8bit **ppHtype,
	SipError *pErr)
#else
	( pHdr, ppHtype, pErr )
	  SipHeader * pHdr;    /* Hide Header */
	  SIP_S8bit **ppHtype;
	  SipError * pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIPDEBUGFN( "Entering sip_getTypeFromHideHdr");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppHtype == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeHide))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if(((SipHideHeader *)(pHdr->pHeader))->pType == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppHtype = ((SipHideHeader *)(pHdr->pHeader))->pType;
#else
	dLength = strlen(((SipHideHeader *)(pHdr->pHeader))->pType);
	*ppHtype = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, dLength+1, pErr);
	if (*ppHtype == SIP_NULL)
		return SipFail;
	strcpy(*ppHtype, ((SipHideHeader *)(pHdr->pHeader))->pType);
#endif
	*pErr = E_NO_ERROR;

	SIPDEBUGFN( "Exiting sip_getTypeFromHideHdr");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setTypeInHideHdr
**
** DESCRIPTION: This function sets the type field
**		in a SIP Hide pHeader
**
** NOTE: This is the new API added after the type in the
** 		 hide header was converted to SIP_S8bit*
**********************************************************/
SipBool sip_setTypeInHideHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,	/*Hide Header */
	SIP_S8bit *pHtype,
	SipError *pErr)
#else
	( pHdr, pHtype, pErr )
	  SipHeader * pHdr;    /* Hide Header */
	  SIP_S8bit *pHtype;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif

	SIPDEBUGFN( "Entering sip_setTypeInHideHdr");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeHide))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if(((SipHideHeader *)(pHdr->pHeader))->pType != SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(((SipHideHeader *)(pHdr->pHeader))->pType),pErr) ==SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipHideHeader *)(pHdr->pHeader))->pType = pHtype;
#else
	if(pHtype != SIP_NULL)
	{
		dLength = strlen(pHtype);
		((SipHideHeader *)(pHdr->pHeader))->pType = (SIP_S8bit *)\
			fast_memget(ACCESSOR_MEM_ID, dLength+1, pErr);
		if (((SipHideHeader *)(pHdr->pHeader))->pType == SIP_NULL)
			return SipFail;
		strcpy(((SipHideHeader *)(pHdr->pHeader))->pType, pHtype);
	}
	else
		((SipHideHeader *)(pHdr->pHeader))->pType = SIP_NULL;

#endif
	*pErr = E_NO_ERROR;

	SIPDEBUGFN( "Exiting sip_setTypeInHideHdr");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getHopsFromMaxForwardsHdr
**
** DESCRIPTION: This function retrieves the number of Hops from
**		a SIP Max-Forwards pHeader
**
**********************************************************/
SipBool sip_getHopsFromMaxForwardsHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_U32bit *pHops,
	SipError *pErr)
#else
	( pHdr, pHops , pErr )
	  SipHeader * pHdr;
	  SIP_U32bit * pHops;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getHopsFromMaxForwardsHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pHops == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeMaxforwards))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

#endif
	*pHops = ( (SipMaxForwardsHeader *) (pHdr->pHeader) )->dHops;
	SIPDEBUGFN( "Exiting getHopsFromMaxForwardsHdr");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setHopsInMaxForwardsHdr
**
** DESCRIPTION: This function sets the number of dHops in a
**		SIP Max-Forwards pHeader
**
**********************************************************/
SipBool sip_setHopsInMaxForwardsHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_U32bit dHops,
	 SipError *pErr)
#else
	( pHdr, dHops, pErr )
	  SipHeader * pHdr;	/* Max Forwards pHeader */
	  SIP_U32bit  dHops;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setHopsInMaxForwardsHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeMaxforwards))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	( (SipMaxForwardsHeader *) (pHdr->pHeader) )->dHops = dHops;
	SIPDEBUGFN( "Exiting setHopsInMaxForwardsHdr");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_getPriorityFromPriorityHdr
**
** DESCRIPTION: This function retrieves the Priority from a
**		SIP Priority pHeader
**
** NOTE: The Priority field has been changed to SIP_S8bit*
**		 This function exists only for conformance with
**		 previous versions of the stack. Please use the
**		 other function (below) for retrieving priority
**		 from the Priority Header.
**********************************************************/
SipBool sip_getPriorityFromPriorityHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	en_Priority *pPriority,
	SipError *pErr)
#else
	( pHdr, pPriority,pErr)
	  SipHeader *pHdr;	/* Priority pHeader */
	  en_Priority *pPriority;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN( "Entering getPriorityFromPriorityHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pPriority == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePriority))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if ((strcasecmp(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "Urgent")) == 0)
		*pPriority = SipPriorityUrgent;
	else if ((strcasecmp(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "Emergency")) == 0)
		*pPriority = SipPriorityEmergency;
	else if ((strcasecmp(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "Normal")) == 0)
		*pPriority = SipPriorityNormal;
	else if ((strcasecmp(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "NonUrgent")) == 0)
		*pPriority = SipPriorityNonUrgent;
	else
		*pPriority = SipPriorityOther;
	SIPDEBUGFN( "Exiting getPriorityFromPriorityHdr");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setPriorityInPriorityHdr
**
** DESCRIPTION: This function sets thge priority field in a
**		SIP Priority pHeader
**
** NOTE: The Priority field has been changed to SIP_S8bit*
**		 This function exists only for conformance with
**		 previous versions of the stack. Please use the
**		 other function (below) for setting priority
**		 in the Priority Header.
**********************************************************/
SipBool sip_setPriorityInPriorityHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	en_Priority dPriority,
	SipError *pErr)
#else
	( pHdr, dPriority, pErr )
	  SipHeader * pHdr;	/* Priority pHeader */
	  en_Priority dPriority;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setPriorityInPriorityHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePriority))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* check if some Priority is already present */
	if (((SipPriorityHeader *) (pHdr->pHeader))->pPriority != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)((((SipPriorityHeader *)\
			(pHdr->pHeader))->pPriority)),pErr)==SipFail)
			return SipFail;
	}
	/* Fill in the appropriate string Priority */
	if (dPriority == SipPriorityUrgent)
	{
		((SipPriorityHeader *) (pHdr->pHeader))->pPriority = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, 7, pErr);
		if (((SipPriorityHeader *) (pHdr->pHeader))->pPriority == NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "Urgent");
	}
	else if (dPriority == SipPriorityEmergency)
	{
		((SipPriorityHeader *) (pHdr->pHeader))->pPriority = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, 10, pErr);
		if (((SipPriorityHeader *) (pHdr->pHeader))->pPriority == NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "Emergency");
	}
	else if (dPriority == SipPriorityNonUrgent)
	{
		((SipPriorityHeader *) (pHdr->pHeader))->pPriority = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, 10, pErr);
		if (((SipPriorityHeader *) (pHdr->pHeader))->pPriority == NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "NonUrgent");
	}
	else if (dPriority == SipPriorityNormal)
	{
		((SipPriorityHeader *) (pHdr->pHeader))->pPriority = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, 7, pErr);
		if (((SipPriorityHeader *) (pHdr->pHeader))->pPriority == NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy(((SipPriorityHeader *) (pHdr->pHeader))->pPriority, "Normal");
	}
	else
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}

	SIPDEBUGFN( "Exiting setPriorityInPriorityHdr");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getPriorityStringFromPriorityHdr
**
** DESCRIPTION: This function retrieves the Priority from a
**		SIP Priority pHeader
**
** NOTE: This is the new API added to get the priority
**		 field from the Priority Header as a string.
**********************************************************/
SipBool sip_getPriorityStringFromPriorityHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppPriority,
	SipError *pErr)
#else
	( pHdr, ppPriority,pErr)
	  SipHeader *pHdr;	/* Priority pHeader */
	  SIP_S8bit **ppPriority;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif

	SIPDEBUGFN( "Entering getPriorityStringFromPriorityHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppPriority == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePriority))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if(((SipPriorityHeader *)(pHdr->pHeader))->pPriority == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppPriority = ((SipPriorityHeader *)(pHdr->pHeader))->pPriority;
#else
	dLength = strlen(((SipPriorityHeader *)(pHdr->pHeader))->pPriority);
	*ppPriority = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, dLength+1, pErr);
	if (*ppPriority == SIP_NULL)
		return SipFail;
	strcpy(*ppPriority, ((SipPriorityHeader *)(pHdr->pHeader))->pPriority);
#endif
	*pErr = E_NO_ERROR;

	SIPDEBUGFN( "Exiting sip_getPriorityStringFromPriorityHdr");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setPriorityStringInPriorityHdr
**
** DESCRIPTION: This function retrieves the Priority from a
**		SIP Priority pHeader
**
** NOTE: This is the new API added to set the priority
**		 field in the Priority Header from the string
**		 input.
**********************************************************/
SipBool sip_setPriorityStringInPriorityHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit *pPriority,
	SipError *pErr)
#else
	( pHdr, pPriority,pErr)
	  SipHeader *pHdr;	/* Priority pHeader */
	  SIP_S8bit *pPriority;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif

	SIPDEBUGFN( "Entering setPriorityStringInPriorityHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePriority))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if(((SipPriorityHeader *)(pHdr->pHeader))->pPriority != SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(((SipPriorityHeader *)(pHdr->pHeader))->pPriority),pErr) ==SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	((SipPriorityHeader *)(pHdr->pHeader))->pPriority = pPriority;
#else
	if(pPriority != SIP_NULL)
	{
		dLength = strlen(pPriority);
		((SipPriorityHeader *)(pHdr->pHeader))->pPriority = \
			(SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, dLength+1, pErr);
		if (((SipPriorityHeader *)(pHdr->pHeader))->pPriority == SIP_NULL)
			return SipFail;
		strcpy(((SipPriorityHeader *)(pHdr->pHeader))->pPriority, pPriority);
	}
	else
		((SipPriorityHeader *)(pHdr->pHeader))->pPriority = SIP_NULL;
#endif
	*pErr = E_NO_ERROR;

	SIPDEBUGFN( "Exiting setPriorityStringInPriorityHdr");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getCredentialsFromProxyAuthorizationHdr
**
** DESCRIPTION: This function retrieves the dCredentials from
**		a SIP Proxy-Authorization pHeader
**
**********************************************************/
SipBool sip_getCredentialsFromProxyAuthorizationHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr,	/*ProxyAuthorization Header */
	SipGenericCredential **ppCr,
	SipError *pErr)
#else
	(SipHeader *pHdr,	/*ProxyAuthorization Header */
	SipGenericCredential *pCr,
	SipError *pErr)
#endif
#else   /* ANSI_PROTO  */
#ifdef SIP_BY_REFERENCE
	( pHdr, ppCr, pErr )
	SipHeader * pHdr;    /* ProxyAuthorization Header */
	SipGenericCredential **ppCr;
	SipError * pErr;
#else
	( pHdr, pCr, pErr )
	SipHeader * pHdr;    /* ProxyAuthorization Header */
	SipGenericCredential *pCr;
	SipError * pErr;
#endif
#endif
{

	SipGenericCredential *pFrom;
	SIPDEBUGFN( "Entering getCredentialsFromProxyAuthorizationHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( ppCr == SIP_NULL)
#else
	if ( pCr == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pHdr->dType != SipHdrTypeProxyauthorization  )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pFrom=((SipProxyAuthorizationHeader *)(pHdr->pHeader))->pCredentials;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppCr = pFrom;
#else
	if(__sip_cloneCredential(pCr,pFrom,pErr)==SipFail)
	{
		return SipFail;
	}
#endif


	SIPDEBUGFN( "Exiting getCredentialsFromProxyAuthorizationHdr ");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setCredentialsInProxyAuthorizationHdr
**
** DESCRIPTION: This function sets the credential in a Proxy-
**		-Authorization pHeader
**
**********************************************************/
SipBool sip_setCredentialsInProxyAuthorizationHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SipGenericCredential *pCredentials,
	SipError *pErr)
#else
	( pHdr, pCredentials, pErr )
	  SipHeader * pHdr;
	  SipGenericCredential *pCredentials;
	  SipError * pErr;
#endif
{
	SipGenericCredential *pTo;
	SIPDEBUGFN( "Entering setCredentialsInProxyAuthorizationHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pHdr->dType != SipHdrTypeProxyauthorization )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTo=((SipProxyAuthorizationHeader *)(pHdr->pHeader))->pCredentials;
	if (pCredentials == SIP_NULL)
	{
		if ( pTo != SIP_NULL)
			sip_freeSipGenericCredential(pTo);
		((SipProxyAuthorizationHeader *)(pHdr->pHeader))->pCredentials = \
																SIP_NULL;
	}
	else
	{	
#ifndef SIP_BY_REFERENCE
		if(sip_initSipGenericCredential(&pTo, pCredentials->dType, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneCredential(pTo, pCredentials, pErr) == SipFail)
		{
			sip_freeSipGenericCredential(pTo);
			return SipFail;
		}
		sip_freeSipGenericCredential(((SipProxyAuthorizationHeader *)(pHdr->pHeader))->pCredentials);
		((SipProxyAuthorizationHeader *)(pHdr->pHeader))->pCredentials = pTo;
#else
		if ( pTo != SIP_NULL)
			sip_freeSipGenericCredential(pTo);
		HSS_LOCKEDINCREF(pCredentials->dRefCount);
		((SipProxyAuthorizationHeader *)(pHdr->pHeader))->pCredentials = \
																pCredentials;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setCredentialsInProxyAuthorizationHdr");
	return SipSuccess;

}

/*********************************************************
** FUNCTION:sip_getTokenFromProxyRequireHdr
**
** DESCRIPTION: This function retrieves the Token from a SIP
**		Proxy-Require pHeader
**
**********************************************************/
SipBool sip_getTokenFromProxyRequireHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppToken,
	SipError *pErr)
#else
	( pHdr, ppToken ,pErr)
	  SipHeader * pHdr;	/* ProxyRequire pHeader */
	  SIP_S8bit **ppToken;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_token;
	SIPDEBUGFN( "Entering getTokenFromProxyRequireHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppToken == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeProxyRequire))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_token = ( (SipProxyRequireHeader *) (pHdr->pHeader) )->pToken;

	if( pTemp_token == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppToken = pTemp_token;
#else

	dLength = strlen(pTemp_token );
	*ppToken = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppToken == SIP_NULL )
		return SipFail;

	strcpy( *ppToken, pTemp_token );
#endif
	SIPDEBUGFN( "Exiting getTokenFromProxyRequireHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setTokenInProxyRequireHdr
**
** DESCRIPTION: This function sets the pToken in a SIP Proxy-
**		-Require pHeader
**
**********************************************************/
SipBool sip_setTokenInProxyRequireHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit *pToken,
	SipError *pErr)

#else
	( pHdr, pToken , pErr )
	  SipHeader * pHdr;	/* ProxyRequire pHeader */
	  SIP_S8bit *pToken;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_token;
#endif
	SipProxyRequireHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setTokenInProxyRequireHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeProxyRequire))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_hdr=(SipProxyRequireHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pToken !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pToken),pErr) ==SipFail)
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	((SipProxyRequireHeader *)(pHdr->pHeader))->pToken= pToken;
#else

	if( pToken == SIP_NULL)
		pTemp_token = SIP_NULL;
	else
	{
		dLength = strlen( pToken );
		pTemp_token = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_token == SIP_NULL )
			return SipFail;

		strcpy( pTemp_token, pToken );
	}

	pTemp_hdr->pToken = pTemp_token;
#endif

	SIPDEBUGFN( "Exiting setTokenInProxyRequireHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getDispNameFromRouteHdr
**
** DESCRIPTION: This function retrieves the display-pName field
**		from a SIP Route pHeader
**
**********************************************************/
SipBool sip_getDispNameFromRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppDispname,
	SipError *pErr)
#else
	( pHdr, ppDispname ,pErr)
	  SipHeader * pHdr;	/* Route pHeader */
	  SIP_S8bit **ppDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_dispname;
	SIPDEBUGFN( "Entering getDispNameFromRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppDispname == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_dispname = ( (SipRouteHeader *) (pHdr->pHeader) )->pDispName;


	if( pTemp_dispname == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppDispname = pTemp_dispname;
#else

	dLength = strlen(pTemp_dispname );
	*ppDispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppDispname == SIP_NULL )
		return SipFail;

	strcpy( *ppDispname, pTemp_dispname );
#endif

	SIPDEBUGFN( "Exiting getDispNameFromRouteHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setDispNameInRouteHdr
**
** DESCRIPTION: This function sets the display-pName field in
**		a SIP Route pHeader
**
**********************************************************/
SipBool sip_setDispNameInRouteHdr
#ifdef ANSI_PROTO
(SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr)

#else
	( pHdr, pDispname , pErr )
	  SipHeader *pHdr;	/*Route Header */
	  SIP_S8bit *pDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_dispname;
#endif
	SipRouteHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setDispNameInRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_hdr=(SipRouteHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pDispName !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pDispName),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipRouteHeader *)(pHdr->pHeader))->pDispName = pDispname;
#else
	if( pDispname == SIP_NULL)
		pTemp_dispname = SIP_NULL;
	else
	{
		dLength = strlen( pDispname );
		pTemp_dispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_dispname == SIP_NULL )
			return SipFail;

		strcpy( pTemp_dispname, pDispname );
	}
	pTemp_hdr->pDispName = pTemp_dispname;
#endif
	SIPDEBUGFN( "Exiting setDispNameInRouteHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getAddrSpecFromRouteHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field
**		from a SIP Route pHeader
**
**********************************************************/
SipBool sip_getAddrSpecFromRouteHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr,
	SipAddrSpec **ppAddrspec,
	SipError *pErr)
#else
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
#endif
#else    /* ANSI_PROTO */
#ifdef SIP_BY_REFERENCE
	( pHdr, ppAddrspec, pErr )
	  SipHeader * pHdr;    /* Route Header */
	  SipAddrSpec **ppAddrspec;
	  SipError * pErr;
#else
	  ( pHdr, pAddrspec, pErr )
	  SipHeader * pHdr;    /* Route Header */
	  SipAddrSpec *pAddrspec;
	  SipError * pErr;
#endif
#endif
{
	SipAddrSpec *pFrom;
	SIPDEBUGFN( "Entering getAddrSpecFromRouteHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	if ( ppAddrspec == SIP_NULL)
#else
	if ( pAddrspec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pFrom=((SipRouteHeader *)(pHdr->pHeader))->pAddrSpec;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;

	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppAddrspec = pFrom;
#else
	if(__sip_cloneAddrSpec(pAddrspec,pFrom,pErr)==SipFail)
	{
		if(pAddrspec->dType==SipAddrReqUri)
		{
			sip_freeString(pAddrspec->u.pUri);
		}
		else if((pAddrspec->dType==SipAddrSipUri) \
				|| (pAddrspec->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl(pAddrspec->u.pSipUrl);
		}
		pAddrspec->dType=SipAddrAny;
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getAddrSpecFromRouteHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setAddrSpecInRouteHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a
**		SIP route pHeader
**
**********************************************************/
SipBool sip_setAddrSpecInRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
#else
	( pHdr, pAddrspec, pErr )
	  SipHeader * pHdr;
	  SipAddrSpec *pAddrspec;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTo;
#endif
	SIPDEBUGFN( "Entering setAddrSpecInRouteHdr ");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ((pHdr->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	sip_freeSipAddrSpec(((SipRouteHeader *)(pHdr->pHeader))->pAddrSpec );
	if (pAddrspec==SIP_NULL)
	{
		((SipRouteHeader *)(pHdr->pHeader))->pAddrSpec=SIP_NULL; 		
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrspec->dRefCount);
		((SipRouteHeader *)(pHdr->pHeader))->pAddrSpec = pAddrspec;
#else
		if(sip_initSipAddrSpec(&pTo,SipAddrAny,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneAddrSpec(pTo,pAddrspec,pErr)==SipFail)
		{
			sip_freeSipAddrSpec(pTo);
			return SipFail;
		}
		((SipRouteHeader *)(pHdr->pHeader))->pAddrSpec = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setAddrSpecInRouteHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamCountFromRouteHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**		from a SIP route pHeader
**
**********************************************************/
SipBool sip_getParamCountFromRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipRouteHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromRouteHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamAtIndexFromRouteHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**		index in a Route header
**
**********************************************************/
SipBool sip_getParamAtIndexFromRouteHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipRouteHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromRouteHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInRouteHdr
**
** DESCRIPTION: This function sets a param at a specified
**		index in a Route header
**
**********************************************************/
SipBool sip_setParamAtIndexInRouteHdr
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
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipRouteHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInRouteHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_insertParamAtIndexInRouteHdr
**
** DESCRIPTION: This function inserts a param at a specified
**		index in a Route header
**
**********************************************************/
SipBool sip_insertParamAtIndexInRouteHdr
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
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipRouteHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInRouteHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInRouteHdr
**
** DESCRIPTION: This function deletes a param at a specified
**		index in a Route header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRoute))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipRouteHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInRouteHdr");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getDispNameFromAlsoHdr
**
** DESCRIPTION: This function retrieves the display-pName field
**		from a SIP Also Header
**
**********************************************************/
SipBool sip_getDispNameFromAlsoHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppDispname,
	SipError *pErr)
#else
	( pHdr, ppDispname ,pErr)
	  SipHeader * pHdr;	/* Also pHeader */
	  SIP_S8bit **ppDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_dispname;
	SIPDEBUGFN( "Entering getDispNameFromAlsoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppDispname == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeAlso))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_dispname = ( (SipAlsoHeader *) (pHdr->pHeader) )->pDispName;


	if( pTemp_dispname == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppDispname = pTemp_dispname;
#else

	dLength = strlen(pTemp_dispname );
	*ppDispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppDispname == SIP_NULL )
		return SipFail;

	strcpy( *ppDispname, pTemp_dispname );
#endif

	SIPDEBUGFN( "Exiting getDispNameFromAlsoHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setDispNameInAlsoHdr
**
** DESCRIPTION: This function sets the display-pName field in
**		a SIP Also Header
**
**********************************************************/
SipBool sip_setDispNameInAlsoHdr
#ifdef ANSI_PROTO
(SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr)

#else
	( pHdr, pDispname , pErr )
	  SipHeader *pHdr;	/*Also Header */
	  SIP_S8bit *pDispname;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_dispname;
#endif
	SipAlsoHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setDispNameInAlsoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeAlso))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_hdr=(SipAlsoHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pDispName !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pDispName),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipAlsoHeader *)(pHdr->pHeader))->pDispName = pDispname;
#else
	if( pDispname == SIP_NULL)
		pTemp_dispname = SIP_NULL;
	else
	{
		dLength = strlen( pDispname );
		pTemp_dispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_dispname == SIP_NULL )
			return SipFail;

		strcpy( pTemp_dispname, pDispname );
	}
	pTemp_hdr->pDispName = pTemp_dispname;
#endif
	SIPDEBUGFN( "Exiting setDispNameInAlsoHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getAddrSpecFromAlsoHdr
**
** DESCRIPTION: This function retrieves the pAddrSpec field
**		from a SIP Also Header
**
**********************************************************/
SipBool sip_getAddrSpecFromAlsoHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr,
	SipAddrSpec **ppAddrspec,
	SipError *pErr)
#else
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
#endif
#else    /* ANSI_PROTO */
#ifdef SIP_BY_REFERENCE
	( pHdr, ppAddrspec, pErr )
	  SipHeader * pHdr;    /* Also Header */
	  SipAddrSpec **ppAddrspec;
	  SipError * pErr;
#else
	  ( pHdr, pAddrspec, pErr )
	  SipHeader * pHdr;    /* Also Header */
	  SipAddrSpec *pAddrspec;
	  SipError * pErr;
#endif
#endif
{
	SipAddrSpec *pFrom;
	SIPDEBUGFN( "Entering getAddrSpecFromAlsoHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( ppAddrspec == SIP_NULL)
#else
	if ( pAddrspec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeAlso) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pFrom=((SipAlsoHeader *)(pHdr->pHeader))->pAddrSpec;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;

	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppAddrspec = pFrom;
#else
	if(__sip_cloneAddrSpec(pAddrspec,pFrom,pErr)==SipFail)
	{
		if(pAddrspec->dType==SipAddrReqUri)
		{
			sip_freeString(pAddrspec->u.pUri);
		}
		else if((pAddrspec->dType==SipAddrSipUri) \
				|| (pAddrspec->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl(pAddrspec->u.pSipUrl);
		}
		pAddrspec->dType=SipAddrAny;
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getAddrSpecFromAlsoHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setAddrSpecInAlsoHdr
**
** DESCRIPTION: This function sets the pAddrSpec field in a
**		SIP Also Header
**
**********************************************************/
SipBool sip_setAddrSpecInAlsoHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
#else
	( pHdr, pAddrspec, pErr )
	  SipHeader * pHdr;
	  SipAddrSpec *pAddrspec;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTo;
#endif
	SIPDEBUGFN( "Entering setAddrSpecInAlsoHdr ");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ((pHdr->dType != SipHdrTypeAlso))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	sip_freeSipAddrSpec(((SipAlsoHeader *)(pHdr->pHeader))->pAddrSpec );
	if (pAddrspec==SIP_NULL)
	{
		((SipAlsoHeader *)(pHdr->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrspec->dRefCount);
		((SipAlsoHeader *)(pHdr->pHeader))->pAddrSpec = pAddrspec;
#else
		if(sip_initSipAddrSpec(&pTo,SipAddrAny,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneAddrSpec(pTo,pAddrspec,pErr)==SipFail)
		{
			sip_freeSipAddrSpec(pTo);
			return SipFail;
		}
		((SipAlsoHeader *)(pHdr->pHeader))->pAddrSpec = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setAddrSpecInAlsoHdr");
	return SipSuccess;

}


/*********************************************************
** FUNCTION:sip_getKeySchemeFromRespKeyHdr
**
** DESCRIPTION: This function retrieves the pKey-pScheme field
**		from a SIP Resposne-Key pHeader
**
**********************************************************/
SipBool sip_getKeySchemeFromRespKeyHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppScheme,
	SipError *pErr)
#else
	( pHdr, ppScheme ,pErr)
	  SipHeader * pHdr;	/* RespKey pHeader */
	  SIP_S8bit **ppScheme;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_scheme;
	SIPDEBUGFN( "Entering getKeySchemeFromRespKeyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppScheme == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeResponseKey))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_scheme = ( (SipRespKeyHeader *) (pHdr->pHeader) )->pKeyScheme;


	if( pTemp_scheme == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppScheme = pTemp_scheme;
#else
	dLength = strlen(pTemp_scheme );
	*ppScheme = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppScheme == SIP_NULL )
		return SipFail;

	strcpy( *ppScheme, pTemp_scheme );
#endif
	SIPDEBUGFN( "Exiting getKeySchemeFromRespKeyHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setKeySchemeInRespKeyHdr
**
** DESCRIPTION: This function sets the pKey-pScheme in a SIP
**		Response-Key pHeader
**
**********************************************************/
SipBool sip_setKeySchemeInRespKeyHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit *pScheme,
	SipError *pErr)
#else
	( pHdr, pScheme , pErr )
	  SipHeader * pHdr;	/* RespKey pHeader */
	  SIP_S8bit *pScheme;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_scheme;
#endif
	SipRespKeyHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setKeySchemeInRespKeyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeResponseKey))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_hdr=(SipRespKeyHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pKeyScheme !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pKeyScheme),pErr) ==SipFail)
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	((SipRespKeyHeader *)(pHdr->pHeader))->pKeyScheme = pScheme;
#else
	if( pScheme == SIP_NULL)
		pTemp_scheme = SIP_NULL;
	else
	{
		dLength = strlen( pScheme );
		pTemp_scheme = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_scheme == SIP_NULL )
			return SipFail;

		strcpy( pTemp_scheme, pScheme );
	}
	pTemp_hdr->pKeyScheme = pTemp_scheme;
#endif
	SIPDEBUGFN( "Exiting setKeySchemeInRespKeyHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:sip_getKeyParamCountFromRespKeyHdr
**
** DESCRIPTION: This function retrieves the number of Key Params
**		from a RespKey Header
**
*********************************************************/
SipBool sip_getKeyParamCountFromRespKeyHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *count, SipError *pErr)
#else
	(pHdr, count, pErr)
	SipHeader *pHdr;
	SIP_U32bit *count;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getKeyParamCountFromRespKeyHdr");
#ifndef SIP_NO_CHECK
	if(  pErr == SIP_NULL  )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeResponseKey) )
	{
    	*pErr = E_INV_TYPE;
    	return SipFail;
    }
	if ((pHdr->pHeader == SIP_NULL))
	{
    	*pErr = E_INV_HEADER;
    	return SipFail;
    }

	if (count == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipRespKeyHeader *)(pHdr->pHeader))->slParam) \
		, count , pErr ) == SipFail )
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getKeyParamCountFromContactHdr");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getKeyParamAtIndexFromRespKeyHdr
**
** DESCRIPTION: This function retrieves the pKey-param field
**		from a SIP Resposne-Key pHeader
**
**********************************************************/
SipBool sip_getKeyParamAtIndexFromRespKeyHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam **ppKeyparam, SIP_U32bit index, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam *pKeyparam, SIP_U32bit index, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pHdr, ppKeyparam ,index, pErr)
	  SipHeader * pHdr;	/* RespKey pHeader */
	  SipParam **ppKeyparam;
	  SIP_U32bit index;
	  SipError * pErr;
#else
	( pHdr, pKeyparam ,index, pErr)
	  SipHeader * pHdr;	/* RespKey pHeader */
	  SipParam *pKeyparam;
	  SIP_U32bit index;
	  SipError * pErr;
#endif
#endif
{
	SipParam *KeyParam;

	SIPDEBUGFN( "Entering getKeyParamFromRespKeyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( ppKeyparam == SIP_NULL)
#else
	if ( pKeyparam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeResponseKey))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ((sip_listGetAt(&(( (SipRespKeyHeader *) (pHdr->pHeader))->slParam), index, (SIP_Pvoid *)&KeyParam, pErr)) == SipFail)
		return SipFail;
	if (KeyParam == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(KeyParam ->dRefCount)  ;
	*ppKeyparam = KeyParam;
#else
	if (__sip_cloneSipParam(pKeyparam, KeyParam , pErr) == SipFail)
	{
		sip_freeSipParam(pKeyparam);
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getKeyParamFromRespKeyHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
/*********************************************************
** FUNCTION:sip_insertKeyParamAtIndexInRespKeyHdr
**
** DESCRIPTION: This function inserts the Key-param field in a
**		SIP Response-Key Header
**
**********************************************************/

SipBool sip_insertKeyParamAtIndexInRespKeyHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipParam *pkeyparam, SIP_U32bit index, SipError *pErr)
#else
	( pHdr, pkeyparam , index, pErr )
	  SipHeader * pHdr;	/* RespKey pHeader */
	   SipParam *pkeyparam;
	   SIP_U32bit index;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipParam * pTemp_keyparams;
#endif
	SIPDEBUGFN( "Entering insertKeyParamInRespKeyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if ( (pHdr->dType != SipHdrTypeResponseKey) )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (pkeyparam==SIP_NULL)
	{
		if ((sip_listInsertAt(&(( (SipRespKeyHeader *)\
			(pHdr->pHeader))->slParam), index, pkeyparam, pErr)) == SipFail)
		return SipFail;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		if ((sip_listInsertAt(&(( (SipRespKeyHeader *)\
			(pHdr->pHeader))->slParam), index, pkeyparam, pErr)) == SipFail)
		return SipFail;
		HSS_LOCKEDINCREF(pkeyparam ->dRefCount)  ;
#else
		if (sip_initSipParam(&pTemp_keyparams, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTemp_keyparams, pkeyparam , pErr)\
																== SipFail)
		{
			sip_freeSipParam(pTemp_keyparams);
			return SipFail;
		}
		if ((sip_listInsertAt(&(( (SipRespKeyHeader *) (pHdr->pHeader))->\
					slParam), index, pTemp_keyparams, pErr)) == SipFail)
			return SipFail;
#endif
	}
	SIPDEBUGFN( "Exiting insertKeyParamInRespKeyHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setKeyParamAtIndexInRespKeyHdr
**
** DESCRIPTION: This function sets the Key-param field in a
**		SIP Response-Key Header
**
**********************************************************/
SipBool sip_setKeyParamAtIndexInRespKeyHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SipParam *pkeyparam,
	SIP_U32bit index,
	SipError *pErr)
#else
	( pHdr, pkeyparam , index, pErr )
	  SipHeader * pHdr;	/* RespKey pHeader */
	   SipParam *pkeyparam;
	   SIP_U32bit index;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipParam * pTemp_keyparams;
#endif
	SIPDEBUGFN( "Entering setKeyParamInRespKeyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeResponseKey))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	
	if (pkeyparam==SIP_NULL)
	{
		if ((sip_listSetAt(&(( (SipRespKeyHeader *)
			(pHdr->pHeader))->slParam), index, pkeyparam, pErr)) == SipFail)
			return SipFail;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		if ((sip_listSetAt(&(( (SipRespKeyHeader *)
			(pHdr->pHeader))->slParam), index, pkeyparam, pErr)) == SipFail)
			return SipFail;
		HSS_LOCKEDINCREF(pkeyparam->dRefCount);
#else
		if (sip_initSipParam(&pTemp_keyparams, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTemp_keyparams, pkeyparam , pErr) == SipFail)
		{
			sip_freeSipParam(pTemp_keyparams);
			return SipFail;
		}
		if ((sip_listSetAt(&(( (SipRespKeyHeader *) (pHdr->pHeader))->slParam),\
					index, pTemp_keyparams, pErr)) == SipFail)
			return SipFail;
#endif
	}		
	SIPDEBUGFN( "Exiting setKeyParamInRespKeyHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_deleteKeyParamInRespKeyHdr
**
** DESCRIPTION: This function deletes the Key-param field in a
**		SIP Response-Key Header
**
**********************************************************/
SipBool sip_deleteKeyParamAtIndexInRespKeyHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_U32bit index,
	SipError *pErr)
#else
	( pHdr, index, pErr )
	  SipHeader * pHdr;	/* RespKey pHeader */
	   SIP_U32bit index;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering deleteKeyParamInRespKeyHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeResponseKey))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipRespKeyHeader *)(pHdr->pHeader))->slParam), index, pErr) == SipFail)
			return SipFail;

	SIPDEBUGFN( "Exiting deleteKeyParamInRespKeyHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}



/*********************************************************
** FUNCTION:sip_getSubjectFromSubjectHdr
**
** DESCRIPTION: This function retrieves the subject field from
**		a SIP Subject pHeader
**
**********************************************************/
SipBool sip_getSubjectFromSubjectHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppSubject,
	SipError *pErr)
#else
	( pHdr, ppSubject ,pErr)
	  SipHeader * pHdr;	/* Subject pHeader */
	  SIP_S8bit **ppSubject;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_Subject;
	SIPDEBUGFN( "Entering getSubjectFromSubjectHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppSubject == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeSubject))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTemp_Subject = ( (SipSubjectHeader *) (pHdr->pHeader) )->pSubject;


	if( pTemp_Subject == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppSubject = pTemp_Subject;
#else
	dLength = strlen(pTemp_Subject );
	*ppSubject = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppSubject == SIP_NULL )
		return SipFail;

	strcpy( *ppSubject, pTemp_Subject );
#endif
	SIPDEBUGFN( "Exiting getSubjectFromSubjectHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setSubjectInSubjectHdr
**
** DESCRIPTION: This function sets the Subject field in a SIP
**		Subject pHeader
**
**********************************************************/
SipBool sip_setSubjectInSubjectHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit *pSubject,
	SipError *pErr)
#else
	( pHdr, pSubject , pErr )
	  SipHeader * pHdr;	/* Subject pHeader */
	  SIP_S8bit *pSubject;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_Subject;
#endif
	SipSubjectHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setSubjectInSubjectHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeSubject))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
  	pTemp_hdr=(SipSubjectHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pSubject !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pSubject),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipSubjectHeader *)(pHdr->pHeader))->pSubject = pSubject;
#else
	if( pSubject == SIP_NULL)
		pTemp_Subject = SIP_NULL;
	else
	{
		dLength = strlen( pSubject );
		pTemp_Subject = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_Subject == SIP_NULL )
			return SipFail;

		strcpy( pTemp_Subject, pSubject );
	}
	pTemp_hdr=(SipSubjectHeader *)(pHdr->pHeader);


	pTemp_hdr->pSubject = pTemp_Subject;
#endif
	SIPDEBUGFN( "Exiting setSubjectInSubjectHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/* =================== second Level APIs	 =============================*/

/*********************************************************
** FUNCTION:sip_getTypeFromContactParam
**
** DESCRIPTION: This function retrieves the dType of a SIP
**		contact-param
**
**********************************************************/
SipBool sip_getTypeFromContactParam
#ifdef ANSI_PROTO
	(SipContactParam *pCp,
	en_ContactParamsType *pType,
	SipError *pErr)
#else
	( pCp, pType , pErr )
	  SipContactParam *pCp;  /* ContactParam pHeader */
	  en_ContactParamsType *pType;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getTypeFromContactParam");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pCp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pType == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	*pType = pCp->dType;
	SIPDEBUGFN( "Exiting getTypeFromContactParam");
	*pErr=E_NO_ERROR;

	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getDayFromDateFormat
**
** DESCRIPTION: This function retrieves the dDay in a dMonth
**		from a SIP dDate pFormat
**
**********************************************************/
SipBool sip_getDayFromDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *pHdr,
	SIP_U8bit *pDay,
	SipError *pErr)
#else
	( pHdr, pDay , pErr )
	  SipDateFormat *pHdr;  /* DateFormat pHeader */
	  SIP_U8bit *pDay;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getDayFromDateFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL|| pDay == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pDay=pHdr->dDay;
	SIPDEBUGFN( "Exiting getDayFromDateFormat");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setDayInDateFormat
**
** DESCRIPTION: This function sets the dDay in dMonth in a
**		SIP dDate structure
**
**********************************************************/
SipBool sip_setDayInDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *pHdr,
	SIP_U8bit dDay,
	SipError *pErr)
#else
	( pHdr, dDay , pErr )
	  SipDateFormat *pHdr; /* DateFormat pHeader */
	  SIP_U8bit dDay;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setDayInDateFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((dDay<1)||(dDay>31))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pHdr->dDay=dDay;
	SIPDEBUGFN( "Exiting setDayInDateFormat");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getMonthFromDateFormat
**
** DESCRIPTION: This function retrieves the monthe field from
**		a SIP Date structure
**
**********************************************************/
SipBool sip_getMonthFromDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *pHdr,
	en_Month *pMonth,
	SipError *pErr)
#else
	( pHdr, pMonth , pErr )
	  SipDateFormat *pHdr;  /* DateFormat pHeader */
	  en_Month *pMonth;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getMonthFromDateFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL || pMonth == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}


#endif
	*pMonth = pHdr->dMonth;
	SIPDEBUGFN( "Exiting getMonthFromDateFormat");
	*pErr=E_NO_ERROR;

	return SipSuccess;
}
/***************************************************************
** FUNCTION: sip_setMonthInDateFormat
**
** DESCRIPTION: This function sets the dMonth in a SIP Date structure
**
***************************************************************/

SipBool sip_setMonthInDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *pHdr,
	en_Month dMonth,
	SipError *pErr)
#else
	( pHdr, dMonth , pErr )
	  SipDateFormat *pHdr; /* DateFormat pHeader */
	  en_Month dMonth;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setMonthInDateFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(validateMonth(&dMonth,pErr)==SipFail)
		return SipFail;

	pHdr->dMonth=dMonth;
	SIPDEBUGFN( "Exiting setMonthInDateFormat");
	*pErr=E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_getYearFromDateFormat
**
** DESCRIPTION: This function retrieves the dYear from a SIP dDate
**		structure
**
***************************************************************/

SipBool sip_getYearFromDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *pHdr,
	SIP_U16bit *pYear,
	SipError *pErr)
#else
	( pHdr, pYear , pErr )
	  SipDateFormat *pHdr;  /* DateFormat pHeader */
	  SIP_U16bit *pYear;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getYearFromDateFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pHdr == SIP_NULL|| pYear == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pYear=pHdr->dYear;
	SIPDEBUGFN( "Exiting getYearFromDateFormat");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setYearInDateFormat
**
** DESCRIPTION: This function sets the dYear field in a SIP Date
**		structure
**
***************************************************************/

SipBool sip_setYearInDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *pHdr,
	SIP_U16bit dYear,
	SipError *pErr)
#else
	( pHdr, dYear , pErr )
	  SipDateFormat *pHdr; /* DateFormat pHeader */
	  SIP_U16bit dYear;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setYearInDateFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if(dYear>SIP_9999_YEAR)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pHdr->dYear=dYear;
	SIPDEBUGFN( "Exiting setYearInDateFormat");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getHourFromTimeFormat
**
** DESCRIPTION: This function retrieves the hourt field from a
**		SIP Time structure
**
***************************************************************/

SipBool sip_getHourFromTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *pHdr,
	SIP_S8bit *pHour,
	SipError *pErr)
#else
	( pHdr, pHour , pErr )
	  SipTimeFormat *pHdr;  /* TimeFormat pHeader */
	  SIP_S8bit *pHour;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getHourFromTimeFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL|| pHour == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pHour=pHdr->dHour;
	SIPDEBUGFN( "Exiting getHourFromTimeFormat");
	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setHourInTimeFormat
**
** DESCRIPTION: This function sets the dHour field in a SIP Time
**		structure
**
***************************************************************/

SipBool sip_setHourInTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *pHdr,
	SIP_S8bit dHour,
	SipError *pErr)
#else
	( pHdr, dHour , pErr )
	  SipTimeFormat *pHdr; /* TimeFormat pHeader */
	  SIP_S8bit dHour;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setHourInTimeFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((SIP_U32bit)dHour>SIP_23_HRS)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pHdr->dHour=dHour;
	SIPDEBUGFN( "Exiting setHourInTimeFormat");
	*pErr=E_NO_ERROR;

	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getMinFromTimeFormat
**
** DESCRIPTION: This function retrieves the minutes field from a
**		SIP Time structure
**
***************************************************************/

SipBool sip_getMinFromTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *pHdr,
	SIP_S8bit *pMin,
	SipError *pErr)
#else
	( pHdr, pMin , pErr )
	  SipTimeFormat *pHdr;  /* TimeFormat pHeader */
	  SIP_S8bit *pMin;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getMinFromTimeFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL|| pMin == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pMin=pHdr->dMin;
	SIPDEBUGFN( "Exiting getMinFromTimeFormat");
	*pErr=E_NO_ERROR;

	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setMinInTimeFormat
**
** DESCRIPTION: This function sets the minutes field in a SIP
**		Time structure
**
***************************************************************/

SipBool sip_setMinInTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *pHdr,
	SIP_S8bit dMin,
	SipError *pErr)
#else
	( pHdr, dMin , pErr )
	  SipTimeFormat *pHdr; /* TimeFormat pHeader */
	  SIP_S8bit dMin;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setMinInTimeFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((SIP_U32bit)dMin>SIP_59_SECS)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pHdr->dMin=dMin;
	SIPDEBUGFN( "Exiting setMinInTimeFormat");
	*pErr=E_NO_ERROR;


	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getSecFromTimeFormat
**
** DESCRIPTION: This function retrieves the slTime from a SIP Time
*8		structure
**
***************************************************************/

SipBool sip_getSecFromTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *pHdr,
	SIP_S8bit *pSec,
	SipError *pErr)
#else
	( pHdr, pSec , pErr )
	  SipTimeFormat *pHdr;  /* TimeFormat pHeader */
	  SIP_S8bit *pSec;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getSecFromTimeFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL|| pSec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pSec=pHdr->dSec;
	SIPDEBUGFN( "Exiting getSecFromTimeFormat");
	*pErr=E_NO_ERROR;

	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setSecInTimeFormat
**
** DESCRIPTION: This function sets the seconds in a SIP Time structure
**
***************************************************************/

SipBool sip_setSecInTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *pHdr,
	SIP_S8bit dSec,
	SipError *pErr)
#else
	( pHdr, dSec , pErr )
	  SipTimeFormat *pHdr; /* TimeFormat pHeader */
	  SIP_S8bit dSec;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering setSecInTimeFormat");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((SIP_U32bit)dSec>SIP_59_SECS)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pHdr->dSec=dSec;
	SIPDEBUGFN( "Exiting setSecInTimeFormat");
	*pErr=E_NO_ERROR;


	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getQvalueFromContactParam
**
** DESCRIPTION: This function retrieves the pQValue-pValue field from a
**		SIP contact-param
**
***************************************************************/

SipBool sip_getQvalueFromContactParam
#ifdef ANSI_PROTO
	(SipContactParam *pCp,
	SIP_S8bit **ppQValue,
	SipError *pErr)
#else
	( pCp, ppQValue , pErr )
	  SipContactParam *pCp;  /* ContactParam pHeader */
	  SIP_S8bit **ppQValue;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_qvalue;
	SIPDEBUGFN( "Entering getQValueFromContactParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCp == SIP_NULL || ppQValue == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pCp->dType != SipCParamQvalue )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif

	pTemp_qvalue = pCp->u.pQValue;


	if( pTemp_qvalue == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppQValue = pTemp_qvalue;
#else
	dLength = strlen(pTemp_qvalue );
	*ppQValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppQValue == SIP_NULL )
		return SipFail;

	strcpy( *ppQValue, pTemp_qvalue );
#endif
	SIPDEBUGFN( "Exiting getQValueFromContactParam");
	*pErr = E_NO_ERROR;


	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_setQvalueInContactParam
**
** DESCRIPTION: This function sets the pQValue-pValue field in a SIP
**		contact-param
**
***************************************************************/

SipBool sip_setQvalueInContactParam
#ifdef ANSI_PROTO
	(SipContactParam *pCp,
	SIP_S8bit *pQValue,
	SipError *pErr)
#else
	( pCp, pQValue , pErr )
	  SipContactParam *pCp; /* ContactParam pHeader */
	  SIP_S8bit *pQValue;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_qvalue;
#endif
	SIPDEBUGFN( "Entering setQValueInContactParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pCp->dType != SipCParamQvalue )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if(pCp->u.pQValue !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCp->u.pQValue),pErr) ==SipFail)
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pCp->u.pQValue = pQValue;
#else
	if( pQValue == SIP_NULL)
		pTemp_qvalue = SIP_NULL;
	else
	{
		dLength = strlen( pQValue );
		pTemp_qvalue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_qvalue == SIP_NULL )
			return SipFail;

		strcpy( pTemp_qvalue, pQValue );
	}
	pCp->u.pQValue = pTemp_qvalue;
#endif

	SIPDEBUGFN( "Exiting setQValueInContactParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getExpiresFromContactParam
**
** DESCRIPTION: This function retrieves the expires field from a
**		SIP contact-param
**
***************************************************************/

SipBool sip_getExpiresFromContactParam
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipContactParam *pHdr,
	SipExpiresStruct **ppExpires,
	SipError *pErr)
#else
	(SipContactParam *pHdr,
	SipExpiresStruct *pExpires,
	SipError *pErr)
#endif
#else		/* ANSI_PROTO */
#ifdef SIP_BY_REFERENCE
	( pHdr, ppExpires, pErr )
	  SipContactParam *pHdr;
	  SipExpiresStruct **ppExpires;
	  SipError *pErr;
#else
	  ( pHdr, pExpires, pErr )
	  SipContactParam *pHdr;
	  SipExpiresStruct *pExpires;
	  SipError *pErr;
#endif
#endif
{
	SipExpiresStruct *pFrom;
	SIPDEBUGFN( "Entering getExpiresFromContactParam");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ( pHdr == SIP_NULL||ppExpires==SIP_NULL)
#else
	if ( pHdr == SIP_NULL||pExpires==SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipCParamExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pFrom=pHdr->u.pExpire;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppExpires = pFrom;
#else
	if(__sip_cloneExpires(pExpires,pFrom,pErr)==SipFail)
	{
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getExpiresFromContactParam ");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setExpiresInContactParam
**
** DESCRIPTION: This function sets the expires field in a SIP
**		contact-param
**
***************************************************************/

SipBool sip_setExpiresInContactParam
#ifdef ANSI_PROTO
	(SipContactParam *pHdr,
	SipExpiresStruct *pExpires,
	SipError *pErr)
#else
	( pHdr, pExpires, pErr )
	SipContactParam *pHdr;
	SipExpiresStruct *pExpires;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipExpiresStruct *pTo;
#endif
	SIPDEBUGFN( "Entering setExpiresInContactParam");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipCParamExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ((pHdr->u.pExpire) != SIP_NULL)
		sip_freeSipExpiresStruct(pHdr->u.pExpire);
	if (pExpires==SIP_NULL)
	{
		pHdr->u.pExpire = SIP_NULL;	
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pExpires->dRefCount);
		pHdr->u.pExpire = pExpires;
#else
		if(sip_initSipExpiresStruct(&pTo,SipExpAny,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneExpires(pTo,pExpires,pErr)==SipFail)
		{
			sip_freeSipExpiresStruct(pTo);
			return SipFail;
		}
		pHdr->u.pExpire = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setExpiresInContactParam");
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getExtensionAttrFromContactParam
**
** DESCRIPTION: This function retrieves the extension-attribute
**		from a SIP contact-param
**
***************************************************************/

SipBool sip_getExtensionAttrFromContactParam
#ifdef ANSI_PROTO
	(SipContactParam *pCp,
	SIP_S8bit **ppExtensionAttribute,
	SipError *pErr)
#else
	( pCp, ppExtensionAttribute , pErr )
	  SipContactParam *pCp;
	  SIP_S8bit **ppExtensionAttribute;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_extnAttr;
	SIPDEBUGFN( "Entering getExtensionAttrFromContactParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCp == SIP_NULL || ppExtensionAttribute == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pCp->dType != SipCParamExtension )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pTemp_extnAttr = pCp->u.pExtensionAttr;


	if( pTemp_extnAttr == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppExtensionAttribute = pTemp_extnAttr;
#else
	dLength = strlen(pTemp_extnAttr );
	*ppExtensionAttribute = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppExtensionAttribute == SIP_NULL )
		return SipFail;

	strcpy( *ppExtensionAttribute, pTemp_extnAttr );
#endif
	SIPDEBUGFN( "Exiting getExtensionAttrFromContactParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
/***************************************************************
** FUNCTION: sip_setExtensionAttrInContactParam
**
** DESCRIPTION: This function sets the extension-attribute in a
**		SIP contact param
**
***************************************************************/

SipBool sip_setExtensionAttrInContactParam
#ifdef ANSI_PROTO
	(SipContactParam *pCp,
	SIP_S8bit *pExtensionAttribute,
	SipError *pErr)
#else
	( pCp, pExtensionAttribute , pErr )
	  SipContactParam *pCp;
	  SIP_S8bit *pExtensionAttribute;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_extnAttr;
#endif
	SIPDEBUGFN( "Entering setExtensionAttributeInContactParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pCp->dType != SipCParamExtension )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif

	if(pCp->u.pExtensionAttr !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCp->u.pExtensionAttr),\
							pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pCp->u.pExtensionAttr = pExtensionAttribute;
#else
	if( pExtensionAttribute == SIP_NULL)
		pTemp_extnAttr = SIP_NULL;
	else
	{
		dLength = strlen( pExtensionAttribute );
		pTemp_extnAttr =\
					( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_extnAttr == SIP_NULL )
			return SipFail;

		strcpy( pTemp_extnAttr, pExtensionAttribute );
	}


	pCp->u.pExtensionAttr = pTemp_extnAttr;
#endif
	SIPDEBUGFN( "Exiting setExtensionAttributeInContactParam");
	*pErr = E_NO_ERROR;


	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_getFeatureParamFromContactParam
**
** DESCRIPTION: This function retrieves the FeatureParam
**		from a SIP contact-param
**
***************************************************************/

SipBool sip_getFeatureParamFromContactParam
#ifdef SIP_BY_REFERENCE 
	(SipContactParam *pCp,
	SipParam **ppFeatureParam,
	SipError *pErr)
#else
	(SipContactParam *pCp,
	SipParam *pFeatureParam,
	SipError *pErr)
#endif
{
	SipParam *pTemp = SIP_NULL ;
	
	SIPDEBUGFN( "Entering sip_getFeatureParamFromContactParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCp == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN( "Exiting sip_getFeatureParamFromContactParam");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
   if (ppFeatureParam == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN( "Exiting sip_getFeatureParamFromContactParam");
		return SipFail;
	}
#else
   if (pFeatureParam == SIP_NULL )
	 {
		*pErr = E_INV_PARAM;
		SIPDEBUGFN( "Exiting sip_getFeatureParamFromContactParam");
		return SipFail;
	}
#endif

	if( pCp->dType != SipCParamFeatureParam)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN( "Exiting sip_getFeatureParamFromContactParam");
		return SipFail;
	}
#endif
	pTemp = pCp->u.pParam ;


	if( pTemp == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN( "Exiting sip_getFeatureParamFromContactParam");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	 HSS_LOCKEDINCREF(pTemp->dRefCount);
	*ppFeatureParam = pTemp;
#else
	if ( __sip_cloneSipParam(pFeatureParam,(SipParam *)pTemp,pErr)\
																	== SipFail)
	{
		sip_freeSipParam(pFeatureParam);
		SIPDEBUGFN( "Exiting sip_getFeatureParamFromContactParam");
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting sip_getFeatureParamFromContactParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setFeatureParamInContactParam
**
** DESCRIPTION: This function sets the feature-attribute in a
**		SIP contact param
**
***************************************************************/

SipBool sip_setFeatureParamInContactParam
	(SipContactParam *pCp,
	SipParam *pParam,
	SipError *pErr)
{
	SIPDEBUGFN( "Entering sip_setFeatureParamInContactParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pCp->dType != SipCParamFeatureParam )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif

	if (pParam == SIP_NULL)
	{
		sip_freeSipParam((pCp->u).pParam);
		(pCp->u).pParam = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
		sip_freeSipParam((pCp->u).pParam);
		HSS_LOCKEDINCREF(pParam->dRefCount);
		(pCp->u).pParam = pParam;
#else
		if ((pCp->u).pParam == SIP_NULL)
		{
			if (sip_initSipParam(&((pCp->u).pParam), pErr) == SipFail)
			{
				return SipFail;
			}
		}
		if (__sip_cloneSipParam((pCp->u).pParam, pParam,pErr) == SipFail)
		{
				return SipFail;
		}
#endif
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN( "Exiting sip_setFeatureParamInContactParam");

	return SipSuccess;
}



/***************************************************************
** FUNCTION: sip_getTypeFromViaParam
**
** DESCRIPTION: This function retrieves the dType of a SIP via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_getTypeFromViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	en_ViaParamType *dType,
	SipError *pErr)
#else
	( pVp, dType , pErr )
	  SipViaParam *pVp;
	  en_ViaParamType *dType;
	  SipError * pErr;
#endif
{
	SIPDEBUGFN( "Entering getTypeFromViaParam");

	if( pErr == SIP_NULL )
		return SipFail;

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( dType == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	*dType = pVp->dType;

	SIPDEBUGFN( "Exiting getHiddenFromViaParam");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_getHiddenFromViaParam
**
** DESCRIPTION: This function retrieves the pHidden field from a
**		SIP via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED

SipBool sip_getHiddenFromViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit **pHidden,
	SipError *pErr)
#else
	( pVp, pHidden , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit **pHidden;
	  SipError * pErr;
#endif
{
	SIP_U16bit dLength;
	SIP_S8bit * temp_hidden;


	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering getHiddenFromViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamHidden )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_hidden = pVp->u.pHidden;


	if( temp_hidden == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	dLength = strlen(temp_hidden );
	*pHidden = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *pHidden == SIP_NULL )
		return SipFail;

	strcpy( *pHidden, temp_hidden );
	SIPDEBUGFN( "Exiting getHiddenFromViaParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_setHiddenInViaParam
**
** DESCRIPTION: This function sets a pHidden field in a SIP via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED

SipBool sip_setHiddenInViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit *pHidden,
	SipError *pErr)
#else
	( pVp, pHidden , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit *pHidden;
	  SipError * pErr;
#endif
{
	SIP_U32bit dLength;
	SIP_S8bit * temp_hidden;
	SipViaParam *temp_vp;
	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering setHiddenInViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamHidden )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHidden == SIP_NULL)
		temp_hidden = SIP_NULL;
	else
	{
		dLength = strlen( pHidden );
		temp_hidden = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_hidden == SIP_NULL )
			return SipFail;

		strcpy( temp_hidden, pHidden );
	}
	temp_vp=pVp;

	if(temp_vp->u.pHidden !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(temp_vp->u.pHidden),pErr) ==SipFail)
		return SipFail;
	}

	temp_vp->u.pHidden = temp_hidden;

	SIPDEBUGFN( "Exiting setHiddenInViaParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_getTtlFromViaParam
**
** DESCRIPTION: This function retrieves the Ttl field from a SIP
**		via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED

SipBool sip_getTtlFromViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_U8bit *dTtl,
	SipError *pErr)
#else
	( pVp, dTtl , pErr )
	  SipViaParam *pVp;
	  SIP_U8bit *dTtl;
	  SipError * pErr;
#endif
{

	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering getTtlFromViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamTtl  )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	*dTtl = pVp->u.dTtl;

	SIPDEBUGFN( "Exiting getTtlFromViaParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_setTtlInViaParam
**
** DESCRIPTION: This function sets the Ttl field in a SIP Via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED

SipBool sip_setTtlInViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_U8bit dTtl,
	SipError *pErr)
#else
	( pVp, dTtl , pErr )
	  SipViaParam *pVp;
	  SIP_U8bit dTtl;
	  SipError * pErr;
#endif
{
	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering setTtlInViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamTtl  )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}

	pVp->u.dTtl=dTtl;
	SIPDEBUGFN( "Exiting setTtlInViaParam");
	*pErr = E_NO_ERROR;


	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_getMaddrFromViaParam
**
** DESCRIPTION: This function retrieves the pMaddr field from a
**		SIP via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_getMaddrFromViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit **pMaddr,
	SipError *pErr)
#else
	( pVp, pMaddr , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit **pMaddr;
	  SipError * pErr;
#endif
{
	SIP_U16bit dLength;
	SIP_S8bit * temp_maddr;


	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering getMaddrFromViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamMaddr  )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_maddr = pVp->u.pMaddr;


	if( temp_maddr == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	dLength = strlen(temp_maddr );
	*pMaddr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *pMaddr == SIP_NULL )
		return SipFail;

	strcpy( *pMaddr, temp_maddr );
	SIPDEBUGFN( "Exiting getMaddrFromViaParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_setMaddrInViaParam
**
** DESCRIPTION: This function sets the pMaddr field in a SIP via-
**		-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_setMaddrInViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit *pMaddr,
	SipError *pErr)
#else
	( pVp, pMaddr , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit *pMaddr;
	  SipError * pErr;
#endif
{
	SIP_U32bit dLength;
	SIP_S8bit * temp_maddr;
	SipViaParam *temp_vp;
	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering setMaddrInViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamMaddr  )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}

	if( pMaddr == SIP_NULL)
		temp_maddr = SIP_NULL;
	else
	{
		dLength = strlen( pMaddr );
		temp_maddr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_maddr == SIP_NULL )
			return SipFail;

		strcpy( temp_maddr, pMaddr );
	}
	temp_vp=pVp;

	if(temp_vp->u.pMaddr !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(temp_vp->u.pMaddr),pErr) ==SipFail)
		return SipFail;
	}

	temp_vp->u.pMaddr = temp_maddr;
	SIPDEBUGFN( "Exiting setMaddrInViaParam");
	*pErr = E_NO_ERROR;


	return SipSuccess;
}
***************************************************************/


/***************************************************************
** FUNCTION: sip_getViaReceivedFromViaParam
**
** DESCRIPTION: This function retrieves the via-received field from
**		a SIP via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_getViaReceivedFromViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit **viaReceived,
	SipError *pErr)
#else
	( pVp, viaReceived , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit **viaReceived;
	  SipError * pErr;
#endif
{
	SIP_U16bit dLength;
	SIP_S8bit * temp_viaRecd;


	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering getViaReceivedFromViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamReceived )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_viaRecd = pVp->u.pViaReceived;


	if( temp_viaRecd == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	dLength = strlen(temp_viaRecd );
	*viaReceived = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *viaReceived == SIP_NULL )
		return SipFail;

	strcpy( *viaReceived, temp_viaRecd );
	SIPDEBUGFN( "Exiting getViaReceivedFromViaParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_setViaReceivedInViaParam
**
** DESCRIPTION: This function sets the via-received field in a SIP
**		via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_setViaReceivedInViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit *viaReceived,
	SipError *pErr)
#else
	( pVp, viaReceived , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit *viaReceived;
	  SipError * pErr;
#endif
{
	SIP_U32bit dLength;
	SIP_S8bit * temp_viaRecd;
	 SipViaParam *temp_vp;
	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering setViaReceivedInViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pVp->dType != SipViaParamReceived )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}


	if( viaReceived == SIP_NULL)
		temp_viaRecd = SIP_NULL;
	else
	{
		dLength = strlen( viaReceived );
		temp_viaRecd = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_viaRecd == SIP_NULL )
			return SipFail;

		strcpy( temp_viaRecd, viaReceived );
	}
	temp_vp=pVp;

	if(temp_vp->u.pViaReceived !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(temp_vp->u.pViaReceived),pErr) ==SipFail)
		return SipFail;
	}

	temp_vp->u.pViaReceived = temp_viaRecd;
	SIPDEBUGFN( "Exiting setViaReceivedInViaParam");
	*pErr = E_NO_ERROR;


	return SipSuccess;
}
***************************************************************/
/***************************************************************
** FUNCTION: sip_getViaBranchFromViaParam
**
** DESCRIPTION: This function retrieb=ves the via-branch field
**		from a SIP via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_getViaBranchFromViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit **viaBranch,
	SipError *pErr)
#else
	( pVp, viaBranch , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit **viaBranch;
	  SipError * pErr;
#endif
{
	SIP_U16bit dLength;
	SIP_S8bit * temp_viaBranch;


	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering getViaBranchFromViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamBranch )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_viaBranch = pVp->u.pViaBranch;


	if( temp_viaBranch == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	dLength = strlen(temp_viaBranch );
	*viaBranch = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *viaBranch == SIP_NULL )
		return SipFail;

	strcpy( *viaBranch, temp_viaBranch );
	SIPDEBUGFN( "Exiting getViaBranchFromViaParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_setViaBranchInViaParam
**
** DESCRIPTION: This function sets the via-branch field in a SIP
**		via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_setViaBranchInViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit *viaBranch,
	SipError *pErr)
#else
	( pVp, viaBranch , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit *viaBranch;
	  SipError * pErr;
#endif
{
	SIP_U32bit dLength;
	SIP_S8bit * temp_viaBranch;
	SipViaParam *temp_vp;
	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering setViaBranchInViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamBranch )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}

	if( viaBranch == SIP_NULL)
		temp_viaBranch = SIP_NULL;
	else
	{
		dLength = strlen( viaBranch );
		temp_viaBranch = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_viaBranch == SIP_NULL )
			return SipFail;

		strcpy( temp_viaBranch, viaBranch );
	}
	temp_vp=pVp;

	if(temp_vp->u.pViaBranch !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(temp_vp->u.pViaBranch),pErr) ==SipFail)
		return SipFail;
	}

	temp_vp->u.pViaBranch = temp_viaBranch;
	SIPDEBUGFN( "Exiting setViaBranchInViaParam");

	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_getExtParamFromViaParam
**
** DESCRIPTION: This function retrieves an extension-param from a
**		SIP via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED
SipBool sip_getExtParamFromViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit **ppExtParam,
	SipError *pErr)
#else
	( pVp, ppExtParam , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit **ppExtParam;
	  SipError * pErr;
#endif
{
	SIP_U16bit dLength;
	SIP_S8bit * pTemp_extParam;


	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering getExtParamFromViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pVp->dType != SipViaParamExtension )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTemp_extParam = pVp->u.pExtParam;


	if( pTemp_extParam == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	dLength = strlen(pTemp_extParam );
	*ppExtParam = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppExtParam == SIP_NULL )
		return SipFail;

	strcpy( *ppExtParam, pTemp_extParam );
	SIPDEBUGFN( "Exiting getExtParamFromViaParam");
	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_setExtParamInViaParam
**
** DESCRIPTION: This function sets an extension-parm in a SIP
**		via-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED

SipBool sip_setExtParamInViaParam
#ifdef ANSI_PROTO
	(SipViaParam *pVp,
	SIP_S8bit *pExtParam,
	SipError *pErr)
#else
	( pVp, pExtParam , pErr )
	  SipViaParam *pVp;
	  SIP_S8bit *pExtParam;
	  SipError * pErr;
#endif
{
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_extParam;
	SipViaParam *temp_vp;
	if( pErr == SIP_NULL )
		return SipFail;
	SIPDEBUGFN( "Entering setExtParamInViaParam");

	if ( pVp == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pVp->dType != SipViaParamExtension )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}


	if( pExtParam == SIP_NULL)
		pTemp_extParam=SIP_NULL;
	else
	{
		dLength = strlen( pExtParam );
		pTemp_extParam = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_extParam == SIP_NULL )
			return SipFail;

		strcpy( pTemp_extParam, pExtParam );
	}
	temp_vp=pVp;

	if(temp_vp->u.pExtParam !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(temp_vp->u.pExtParam),pErr) ==SipFail)
		return SipFail;
	}

	temp_vp->u.pExtParam = pTemp_extParam;
	SIPDEBUGFN( "Exiting setExtParamInViaParam");

	*pErr = E_NO_ERROR;

	return SipSuccess;
}
***************************************************************/

/***************************************************************
** FUNCTION: sip_getCredentialTypeFromCredential
**
** DESCRIPTION: This function retrieves the dType of a SIP credential
**
***************************************************************/

SipBool sip_getCredentialTypeFromCredential
#ifdef ANSI_PROTO
	(SipGenericCredential *pCr,
	en_CredentialType *pType,
	SipError *pErr)
#else
	( pCr, pType , pErr )
	  SipGenericCredential *pCr;
	  en_CredentialType *pType;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN( "Entering getCredentialTypeFromCredential");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pCr == SIP_NULL || pType == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif


	*pType = pCr->dType;
	SIPDEBUGFN( "Exiting getCredentialTypeFromCredential");

	*pErr=E_NO_ERROR;


	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getBasicFromCredential
**
** DESCRIPTION: This function retrieves the pBasic field from a SIP
**		Credential
**
***************************************************************/

SipBool sip_getBasicFromCredential
#ifdef ANSI_PROTO
	(SipGenericCredential *pCr,
	SIP_S8bit **ppBasic,
	SipError *pErr)
#else
	( pCr, ppBasic , pErr )
	SipGenericCredential *pCr;
	SIP_S8bit **ppBasic;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_basic;
	SIPDEBUGFN( "Entering getBasicFromCredential");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCr == SIP_NULL || ppBasic == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pCr->dType != SipCredBasic  )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pTemp_basic = pCr->u.pBasic;


	if( pTemp_basic == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppBasic = pTemp_basic;
#else
	dLength = strlen(pTemp_basic );
	*ppBasic = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppBasic == SIP_NULL )
		return SipFail;

	strcpy( *ppBasic, pTemp_basic );
#endif
	SIPDEBUGFN( "Exiting getBasicFromCredential");

	*pErr = E_NO_ERROR;

	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setBasicInCredential
**
** DESCRIPTION:This function sets the pBasic field in a SIP Credential
**
***************************************************************/

SipBool sip_setBasicInCredential
#ifdef ANSI_PROTO
	(SipGenericCredential *pCr,
	SIP_S8bit *pBasic,
	SipError *pErr)
#else
	( pCr, pBasic , pErr )
	SipGenericCredential *pCr;
	SIP_S8bit *pBasic;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_basic;
#endif

	SipGenericCredential *pTemp_cr;
	SIPDEBUGFN( "Entering setBasicInCredential");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pCr->dType != SipCredBasic  )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pTemp_cr=pCr;

	if(pTemp_cr->u.pBasic !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pTemp_cr->u.pBasic),pErr)\
					==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pTemp_cr->u.pBasic = pBasic;
#else
	if( pBasic == SIP_NULL)
		pTemp_basic = SIP_NULL;
	else
	{
		dLength = strlen( pBasic );
		pTemp_basic = \
				( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_basic == SIP_NULL )
			return SipFail;
		strcpy( pTemp_basic, pBasic );
	}

	pTemp_cr->u.pBasic = pTemp_basic;
#endif
	SIPDEBUGFN( "Exiting setBasicInCredential");

	*pErr = E_NO_ERROR;

	return SipSuccess;
}
/***************************************************************
** FUNCTION: sip_getChallengeFromCredential
**
** DESCRIPTION: This functionretrieves The SIP dChallenge from a
**		SIP Credential
**
***************************************************************/

SipBool sip_getChallengeFromCredential
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipGenericCredential *pCr,
	SipGenericChallenge **ppCh,
	SipError *pErr)
#else
	(SipGenericCredential *pCr,
	SipGenericChallenge *pCh,
	SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pCr, ppCh, pErr )
	  SipGenericCredential *pCr;
	  SipGenericChallenge **ppCh;
	  SipError *pErr;
#else
	( pCr, pCh, pErr )
	  SipGenericCredential *pCr;
	  SipGenericChallenge *pCh;
	  SipError *pErr;
#endif
#endif
{
	SipGenericChallenge *pFrom;
	SIPDEBUGFN( "Entering getChallengeFromCredential");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ( pCr == SIP_NULL||ppCh==SIP_NULL)
#else
	if ( pCr == SIP_NULL||pCh==SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pCr->dType != SipCredAuth )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pFrom=pCr->u.pChallenge;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppCh =	pFrom;
#else
	if(__sip_cloneChallenge(pCh,pFrom,pErr)==SipFail)
	{
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getChallengeFromCredential ");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setChallengeInCredential
**
** DESCRIPTION: this function sets a SIP dChallenge in a SIP Credential
**
***************************************************************/

SipBool sip_setChallengeInCredential
#ifdef ANSI_PROTO
	(SipGenericCredential *pCr,
	SipGenericChallenge *pCh,
	SipError *pErr)
#else
	( pCr, pCh, pErr )
	SipGenericCredential *pCr;
	SipGenericChallenge *pCh;
	SipError *pErr;
#endif
{
	SipGenericChallenge *pTo;
	SIPDEBUGFN( "Entering setChallengeInCredential");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pCr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pCr->dType != SipCredAuth )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pTo=pCr->u.pChallenge;
	if (pCh==SIP_NULL)
	{
		if (pTo != SIP_NULL)
			sip_freeSipGenericChallenge(pTo);
		pCr->u.pChallenge= SIP_NULL;
	}
	else
	{	
#ifndef SIP_BY_REFERENCE
		if(sip_initSipGenericChallenge(&pTo, pErr ) == SipFail)
			return SipFail;
		if (__sip_cloneSipChallenge(pTo, pCh, pErr) == SipFail)
		{
			sip_freeSipGenericChallenge(pTo);
			return SipFail;
		}
		sip_freeSipGenericChallenge(pCr->u.pChallenge);
		pCr->u.pChallenge = pTo;
#else
		if (pTo != SIP_NULL)
			sip_freeSipGenericChallenge(pTo);
		HSS_LOCKEDINCREF(pCh->dRefCount);
		pCr->u.pChallenge= pCh;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setChallengeInCredential");
	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_getAddrTypeFromAddrSpec
**
** DESCRIPTION: this function retrieves the dType of a SIP dAddr-spec
**
***************************************************************/

SipBool sip_getAddrTypeFromAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrspec,
	en_AddrType *pAddrtype,
	SipError *pErr)
#else
	( pAddrspec, pAddrtype , pErr )
	SipAddrSpec *pAddrspec;
	en_AddrType *pAddrtype;
	SipError *pErr;
#endif
{
	SIPDEBUGFN( "Entering getAddrTypeFromAddrSpec");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pAddrspec == SIP_NULL || pAddrtype == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif

	*pAddrtype = pAddrspec->dType;
	SIPDEBUGFN( "Exiting getAddrTypeFromAddrSpec");

	*pErr=E_NO_ERROR;


	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getUriFromAddrSpec
**
** DESCRIPTION: This function retrieves the pUri from a SIp dAddr-spec
**
***************************************************************/

SipBool sip_getUriFromAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrspec,
	SIP_S8bit **ppUri,
	SipError *pErr)
#else
	( pAddrspec, ppUri , pErr )
	SipAddrSpec *pAddrspec;
	SIP_S8bit **ppUri;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_uri;
	SIPDEBUGFN( "Entering getUriFromAddrSpec");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pAddrspec == SIP_NULL)||(ppUri==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pAddrspec->dType != SipAddrReqUri )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pTemp_uri = pAddrspec->u.pUri;


	if( pTemp_uri == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppUri = pTemp_uri;
#else
	dLength = strlen(pTemp_uri );
	*ppUri = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppUri == SIP_NULL )
		return SipFail;

	strcpy( *ppUri, pTemp_uri );
#endif
	SIPDEBUGFN( "Exiting getUriFromAddrSpec");

	*pErr = E_NO_ERROR;

	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setUriInAddrSpec
**
** DESCRIPTION: this function sets the pUri in a SIP dAddr-spec
**
***************************************************************/

SipBool sip_setUriInAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrspec,
	SIP_S8bit *pUri,
	SipError *pErr)
#else
	( pAddrspec, pUri , pErr )
	SipAddrSpec *pAddrspec;
	SIP_S8bit *pUri;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_uri;
#endif
	SIPDEBUGFN( "Entering _setUriInAddrSpec");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pAddrspec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pAddrspec->dType != SipAddrReqUri )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif

	if(pAddrspec->u.pUri !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pAddrspec->u.pUri),pErr)\
								==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pAddrspec->u.pUri = pUri;
#else
	if( pUri == SIP_NULL)
		pTemp_uri = SIP_NULL;
	else
	{
		dLength = strlen( pUri );
		pTemp_uri = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_uri == SIP_NULL )
			return SipFail;

		strcpy( pTemp_uri, pUri );
	}


	pAddrspec->u.pUri = pTemp_uri;
#endif
	SIPDEBUGFN( "Exiting setUriInAddrSpec");

	*pErr = E_NO_ERROR;

	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_getUrlFromAddrSpec
**
** DESCRIPTION: this function retrieves the url from a SIp dAddr-spec
**
***************************************************************/

SipBool sip_getUrlFromAddrSpec
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipAddrSpec *pAddrspec,
	SipUrl **pUrl,
	SipError *pErr)
#else
	(SipAddrSpec *pAddrspec,
	SipUrl *pUrl,
	SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pAddrspec, pUrl, pErr )
	  SipAddrSpec *pAddrspec;
	  SipUrl **pUrl;
	  SipError *pErr;
#else
	( pAddrspec, pUrl, pErr )
	  SipAddrSpec *pAddrspec;
	  SipUrl *pUrl;
	  SipError *pErr;
#endif
#endif
{
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif
	SipUrl *pFrom;
	SIPDEBUGFN( "Entering getUrlFromAddrSpec");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pAddrspec == SIP_NULL||pUrl==SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((pAddrspec->dType != SipAddrSipUri))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pFrom=pAddrspec->u.pSipUrl;
	if( pFrom == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
    *pUrl = pFrom;
#else
	if(__sip_cloneSipUrl(pUrl,pFrom,pErr)==SipFail)
	{
		sip_freeString(pUrl->pUser);
		sip_freeString(pUrl->pPassword);
		sip_freeString(pUrl->pHost);
		sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pUrl->dPort),&temp_err);
		sip_freeString(pUrl->pHeader);
		sip_listDeleteAll(&(pUrl->slParam),&temp_err);
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getUrlFromAddrSpec ");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setUrlInAddrSpec
**
** DESCRIPTION: This function sets the url field in a Sip dAddr-spec
**
***************************************************************/

SipBool sip_setUrlInAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrspec,
	SipUrl *pUrl,
	SipError *pErr)
#else
	( pAddrspec, pUrl, pErr )
	  SipAddrSpec *pAddrspec;
	  SipUrl *pUrl;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipUrl *pTo;
#endif
	SIPDEBUGFN( "Entering setUrlInAddrSpec");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pAddrspec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((pAddrspec->dType != SipAddrSipUri))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	sip_freeSipUrl(pAddrspec->u.pSipUrl);
	if (pUrl==SIP_NULL)
	{
		pAddrspec->u.pSipUrl = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pUrl->dRefCount);
		pAddrspec->u.pSipUrl = pUrl;
#else
		if(sip_initSipUrl(&pTo,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneSipUrl(pTo,pUrl,pErr)==SipFail)
		{
			sip_freeSipUrl(pTo);
			return SipFail;
		}
		pAddrspec->u.pSipUrl = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setUrlInAddrSpec");
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getSUrlFromAddrSpec
**
** DESCRIPTION: This function retrieves the sips url from a 
**				SIp dAddr-spec
**
***************************************************************/

SipBool sip_getSUrlFromAddrSpec
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipAddrSpec *pAddrspec,
	SipUrl **pUrl,
	SipError *pErr)
#else
	(SipAddrSpec *pAddrspec,
	SipUrl *pUrl,
	SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pAddrspec, pUrl, pErr )
	  SipAddrSpec *pAddrspec;
	  SipUrl **pUrl;
	  SipError *pErr;
#else
	( pAddrspec, pUrl, pErr )
	  SipAddrSpec *pAddrspec;
	  SipUrl *pUrl;
	  SipError *pErr;
#endif
#endif
{
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif
	SipUrl *pFrom;
	SIPDEBUGFN( "Entering getSUrlFromAddrSpec");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pAddrspec == SIP_NULL||pUrl==SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((pAddrspec->dType != SipAddrSipSUri))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pFrom=pAddrspec->u.pSipUrl;
	if( pFrom == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
    *pUrl = pFrom;
#else
	if(__sip_cloneSipUrl(pUrl,pFrom,pErr)==SipFail)
	{
		sip_freeString(pUrl->pUser);
		sip_freeString(pUrl->pPassword);
		sip_freeString(pUrl->pHost);
		sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pUrl->dPort),&temp_err);
		sip_freeString(pUrl->pHeader);
		sip_listDeleteAll(&(pUrl->slParam),&temp_err);
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getSUrlFromAddrSpec ");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_setSUrlInAddrSpec
**
** DESCRIPTION: This function sets the sips url field in a 
**				Sip dAddr-spec
**
***************************************************************/

SipBool sip_setSUrlInAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrspec,
	SipUrl *pUrl,
	SipError *pErr)
#else
	( pAddrspec, pUrl, pErr )
	  SipAddrSpec *pAddrspec;
	  SipUrl *pUrl;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipUrl *pTo;
#endif
	SIPDEBUGFN( "Entering setSUrlInAddrSpec");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if ( pAddrspec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((pAddrspec->dType != SipAddrSipSUri))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	sip_freeSipUrl(pAddrspec->u.pSipUrl);
	if (pUrl==SIP_NULL)
	{
		pAddrspec->u.pSipUrl = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pUrl->dRefCount);
		pAddrspec->u.pSipUrl = pUrl;
#else
		if(sip_initSipUrl(&pTo,pErr)==SipFail)
		{
			return SipFail;
		}
		if(__sip_cloneSipUrl(pTo,pUrl,pErr)==SipFail)
		{
			sip_freeSipUrl(pTo);
			return SipFail;
		}
		pAddrspec->u.pSipUrl = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setSUrlInAddrSpec");
	return SipSuccess;
}	

/*================= Third Level APIs ===========================*/

/***************************************************************
** FUNCTION: sip_getSchemeFromChallenge
**
** DESCRIPTION: This functionretrieves the pScheme from a SIP
**		Challenge
**
***************************************************************/

SipBool sip_getSchemeFromChallenge
#ifdef ANSI_PROTO
	( SipGenericChallenge *pCh, SIP_S8bit **ppSch, SipError *pErr)
#else
	( pCh, ppSch, pErr )
	  SipGenericChallenge *pCh;
	  SIP_S8bit **ppSch;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTemp_sch;
	SIPDEBUGFN( "Entering getSchemeFromChallenge");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCh == SIP_NULL || ppSch == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_sch = ((SipGenericChallenge *) pCh)->pScheme;

	if( pTemp_sch == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppSch = pTemp_sch;
#else
	dLength = strlen(pTemp_sch);
	*ppSch = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppSch == SIP_NULL )
		return SipFail;

	strcpy( *ppSch , pTemp_sch );
#endif
    SIPDEBUGFN( "Exiting getSchemeFromChallenge");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_setSchemeInChallenge
**
** DESCRIPTION: This function sets the Sceheme in a SIP Challenge
**
***************************************************************/

SipBool sip_setSchemeInChallenge
#ifdef ANSI_PROTO
	( SipGenericChallenge *pCh, SIP_S8bit *pSch, SipError *pErr)
#else
	( pCh, pSch, pErr )
	  SipGenericChallenge *pCh;
	  SIP_S8bit *pSch;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_sch;
#endif
	SipGenericChallenge * pTemp_ch_hdr;
	SIPDEBUGFN( "Entering setSchemeInChallenge");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pCh == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_ch_hdr= (SipGenericChallenge *)(pCh);

	if(pTemp_ch_hdr->pScheme !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_ch_hdr->pScheme),\
											pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipGenericChallenge *)(pCh))->pScheme = pSch;
#else
	if( pSch == SIP_NULL)
		pTemp_sch = SIP_NULL;
	else
	{
		dLength = strlen( pSch );
		pTemp_sch = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_sch== SIP_NULL )
			return SipFail;
		strcpy( pTemp_sch, pSch );
	}

	pTemp_ch_hdr->pScheme = pTemp_sch;
#endif
	SIPDEBUGFN( "Exiting setSchemeInChallenge");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_getUserFromUrl
**
** DESCRIPTION: this function retrieves the pUser from a SIP Url
**
***************************************************************/

SipBool sip_getUserFromUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit **ppUser, SipError *pErr)
#else
	( pUrl, ppUser, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit **ppUser;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTemp_user;
	SIPDEBUGFN( "Entering getUserFromUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL || ppUser == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_user = ((SipUrl *) pUrl)->pUser;

	if( pTemp_user == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppUser = pTemp_user;
#else
	dLength = strlen(pTemp_user);
	*ppUser = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppUser == SIP_NULL )
		return SipFail;

	strcpy( *ppUser , pTemp_user );
#endif
    SIPDEBUGFN( "Exiting getUserFromUrl");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_setUserInUrl
**
** DESCRIPTION: this function sets the pUser in a SIP Url
**
***************************************************************/

SipBool sip_setUserInUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit *pUser, SipError *pErr)
#else
	( pUrl, pUser, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit *pUser;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_user;
#endif
	SIPDEBUGFN( "Entering setUserInUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if(pUrl->pUser !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pUrl->pUser),pErr)\
																	==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pUrl->pUser = pUser;
#else
	if( pUser == SIP_NULL)
		pTemp_user = SIP_NULL;
	else
	{
		dLength = strlen( pUser );
		pTemp_user = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_user== SIP_NULL )
			return SipFail;

		strcpy( pTemp_user, pUser );

	}
	pUrl->pUser = pTemp_user;
#endif

	SIPDEBUGFN( "Exiting setUserInUrl");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/***************************************************************
** FUNCTION: sip_getPasswordFromUrl
**
** DESCRIPTION: This function retrieves the passworxd field from
**		a SIP Url
**
***************************************************************/

SipBool sip_getPasswordFromUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit **ppPassword, SipError *pErr)
#else
	( pUrl, ppPassword, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit **ppPassword;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTemp_password;
	SIPDEBUGFN( "Entering getPasswordFromUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL || ppPassword == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_password = ((SipUrl *) pUrl)->pPassword;

	if( pTemp_password == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppPassword = pTemp_password;
#else
	dLength = strlen(pTemp_password);
	*ppPassword = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppPassword == SIP_NULL )
		return SipFail;

	strcpy( *ppPassword , pTemp_password );
#endif
	SIPDEBUGFN( "Exiting getPasswordFromUrl");


	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/***************************************************************
** FUNCTION: sip_setPasswordInUrl
**
** DESCRIPTION: This function sets the pPassword field in a SIP Url
**
***************************************************************/

SipBool sip_setPasswordInUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit *pPassword, SipError *pErr)
#else
	( pUrl, pPassword, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit *pPassword;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_password;
#endif
	SIPDEBUGFN( "Entering setPasswordInUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pUrl->pPassword != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pUrl->pPassword),\
										pErr) == SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pUrl->pPassword = pPassword;
#else

	if( pPassword == SIP_NULL)
		pTemp_password = SIP_NULL;
	else
	{
		dLength = strlen( pPassword );
		pTemp_password = \
					( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_password== SIP_NULL )
			return SipFail;
		strcpy( pTemp_password, pPassword );
	}
	pUrl->pPassword = pTemp_password;
#endif
	SIPDEBUGFN( "Exiting setPasswordInUrl");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_getHostFromUrl
**
** DESCRIPTION: This function retrieves the pHost field from a SIP
**		Url
**
***************************************************************/

SipBool sip_getHostFromUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit **ppHost, SipError *pErr)
#else
	( pUrl, ppHost, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit **ppHost;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTemp_host;
	SIPDEBUGFN( "Entering getHostFromUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if ( pUrl == SIP_NULL || ppHost == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_host = ((SipUrl *) pUrl)->pHost;

	if( pTemp_host == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	 *ppHost = pTemp_host;
#else
	dLength = strlen(pTemp_host);
	*ppHost = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppHost == SIP_NULL )
		return SipFail;

	strcpy( *ppHost , pTemp_host );
#endif
	SIPDEBUGFN( "Exiting getHostFromUrl");


	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_setHostInUrl
**
** DESCRIPTION: this function sets the pHost field in a SIP Url
**
***************************************************************/

SipBool sip_setHostInUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit *pHost, SipError *pErr)
#else
	( pUrl, pHost, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit *pHost;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_host;
#endif
	SIPDEBUGFN( "Entering setHostInUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pUrl->pHost != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( pUrl->pHost), pErr)\
																	== SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pUrl->pHost = pHost;
#else
	if( pHost == SIP_NULL)
		pTemp_host = SIP_NULL;
	else
	{
		dLength = strlen( pHost );
		pTemp_host = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_host== SIP_NULL )
			return SipFail;

		strcpy( pTemp_host, pHost );

	}

	pUrl->pHost = pTemp_host;
#endif
	SIPDEBUGFN( "Exiting setHostInUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}
/***************************************************************
** FUNCTION: sip_getHeaderFromUrl
**
** DESCRIPTION: This function retrieves the pHeader field from a
**		SIP Url
**
***************************************************************/

SipBool sip_getHeaderFromUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit **ppHeader, SipError *pErr)
#else
	( pUrl, ppHeader, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit **ppHeader;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * pTemp_header;
	SIPDEBUGFN( "Entering getHostFromUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL || ppHeader == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_header = ((SipUrl *) pUrl)->pHeader;

	if( pTemp_header == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppHeader = pTemp_header;
#else
	dLength = strlen(pTemp_header);
	*ppHeader = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppHeader == SIP_NULL )
		return SipFail;

	strcpy( *ppHeader , pTemp_header );
#endif
	SIPDEBUGFN( "Exiting getHostFromUrl");


	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_setHeaderInUrl
**
** DESCRIPTION: This function sets the pHeader field in a SIP Url
**
***************************************************************/

SipBool sip_setHeaderInUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_S8bit *pHeader, SipError *pErr)
#else
	( pUrl, pHeader, pErr )
	  SipUrl *pUrl;
	  SIP_S8bit *pHeader;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_header;
#endif
	SIPDEBUGFN( "Entering setHostInUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pUrl->pHeader != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( pUrl->pHeader), pErr)\
																	== SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pUrl->pHeader = pHeader;
#else
	if( pHeader == SIP_NULL)
		pTemp_header = SIP_NULL;
	else
	{
		dLength = strlen( pHeader );
		pTemp_header = ( SIP_S8bit * )fast_memget\
											(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_header== SIP_NULL )
			return SipFail;

		strcpy( pTemp_header, pHeader );

	}
	pUrl->pHeader = pTemp_header;
#endif
	SIPDEBUGFN( "Exiting setHostInUrl");


	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_getPortFromUrl
**
** DESCRIPTION: This function retrieves the dPort field from a 
**              SIP URL
**
***************************************************************/

SipBool sip_getPortFromUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_U16bit *pPort, SipError *pErr)
#else
	( pUrl, pPort, pErr )
	  SipUrl *pUrl;
	  SIP_U16bit *pPort;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN( "Entering getPortFromUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;

		return SipFail;
	}

	if ( pPort == SIP_NULL )
	{
		*pErr= E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pUrl->dPort==SIP_NULL)
	{
		 *pErr= E_NO_EXIST;
		 return SipFail;
	}
	*pPort= *(pUrl->dPort);

	SIPDEBUGFN( "Exiting getPortFromUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_setPortInUrl
**
** DESCRIPTION: This function sets the dPort field in a SIP Url
**
***************************************************************/

SipBool sip_setPortInUrl
#ifdef ANSI_PROTO
	( SipUrl *pUrl, SIP_U16bit dPort, SipError *pErr)
#else
	( pUrl, dPort, pErr )
	  SipUrl *pUrl;
	  SIP_U16bit dPort;
	  SipError *pErr;
#endif
{

    SIP_U16bit *temp_port;
    SIPDEBUGFN( "Entering setPortInUrl");
#ifndef SIP_NO_CHECK
    if( pErr == SIP_NULL )
        return SipFail;


    if ( pUrl == SIP_NULL)
    {
        *pErr = E_INV_PARAM;
        return SipFail;
    }
#endif
    temp_port = (SIP_U16bit *)fast_memget(ACCESSOR_MEM_ID,
                                          sizeof(SIP_U16bit),
                                          pErr);
    if ( temp_port== SIP_NULL )
    {
        return SipFail;
    }

    *temp_port = dPort;


    if ( pUrl->dPort != SIP_NULL )
    {
        if (sip_memfree(ACCESSOR_MEM_ID, 
                        (SIP_Pvoid*)&(pUrl->dPort), 
                        pErr) == SipFail)
        {
            return SipFail;
        }
    }

    pUrl->dPort = temp_port;

	SIPDEBUGFN( "Exiting setPortInUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***************************************************************
** FUNCTION: sip_getUrlParamCountFromUrl
**
** DESCRIPTION: This function retrieves the number of url-pParam
**		from a SIP Url
**
***************************************************************/

SipBool sip_getUrlParamCountFromUrl
#ifdef ANSI_PROTO
	( SipUrl	*pUrl,
	  SIP_U32bit	*pIndex,
	  SipError	*pErr  )
#else
	( pUrl,pIndex,pErr)
	  SipUrl 	*pUrl;
	  SIP_U32bit 	*pIndex;
	  SipError 	*pErr;
#endif
{
	SIPDEBUGFN( "Entering getUrlParamCountFromUrl");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if ( (pUrl == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pUrl->slParam), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN( "Exiting getUrlParamCountFromUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getUrlParamAtIndexFromUrl
**
** DESCRIPTION: This function retrieves a Url-param at a specified
**		index from a Sip Url
**
***************************************************************/

SipBool sip_getUrlParamAtIndexFromUrl
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipUrl 	*pUrl,
	  SipParam **ppParam,
	  SIP_U32bit   dIndex,
	  SipError     *pErr )
#else
	( SipUrl 	*pUrl,
	  SipParam *pParam,
	  SIP_U32bit   dIndex,
	  SipError     *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pUrl,ppParam,dIndex,pErr)
	  SipUrl 	 *pUrl;
	  SipParam  **ppParam;
	  SIP_U32bit 	dIndex;
	  SipError       *pErr;
#else
	( pUrl,pParam,dIndex,pErr)
	  SipUrl 	 *pUrl;
	  SipParam  *pParam;
	  SIP_U32bit 	dIndex;
	  SipError       *pErr;
#endif
#endif
{
	SIP_Pvoid 	pElement_from_list;
	SipParam *  pUrl_param;

	SIPDEBUGFN( "Entering getUrlParamAtIndexFromUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( (pUrl == SIP_NULL) || (ppParam == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#else
	if ( (pUrl == SIP_NULL) || (pParam == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
#endif
	if( sip_listGetAt(&(pUrl->slParam), dIndex, &pElement_from_list, pErr)\
																	== SipFail)
		return SipFail;

	if ( pElement_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	pUrl_param = (SipParam * )pElement_from_list;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pUrl_param->dRefCount);
	*ppParam = pUrl_param;
#else
	if ( __sip_cloneSipParam(pParam,(SipParam *)pElement_from_list,pErr)\
																	== SipFail)
	{
		sip_freeSipParam(pParam);
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getUrlParamAtIndexFromUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
/***************************************************************
** FUNCTION: sip_setUrlParamAtIndexInUrl
**
** DESCRIPTION: This function sets aurl-param at a specified index
**		in a Sip Url
**
***************************************************************/

SipBool sip_setUrlParamAtIndexInUrl
#ifdef ANSI_PROTO
	( SipUrl 	*pUrl,
	  SipParam	*pParam,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pUrl,pParam,dIndex,pErr)
	  SipUrl 	*pUrl;
	  SipParam	*pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElement_in_list;

	SIPDEBUGFN( "Entering setUrlParamAtIndexInUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if ( pParam == SIP_NULL )
		pElement_in_list = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pElement_in_list = pParam;
#else
		sip_initSipParam(&pElement_in_list,pErr);

		if ( __sip_cloneSipParam(pElement_in_list, pParam,pErr) == SipFail)
		{
			 sip_freeSipParam(pElement_in_list);
			return SipFail;
		}
#endif
	}


	if( sip_listSetAt(&(pUrl->slParam), dIndex, (SIP_Pvoid) pElement_in_list,\
														pErr) == SipFail)
	{
		if ( pElement_in_list != SIP_NULL )
		{
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pParam->dRefCount);
#else
			 sip_freeSipParam(pElement_in_list);
#endif
		}
		return SipFail;
	}
	SIPDEBUGFN( "Exiting setUrlParamAtIndexInUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_insertUrlParamAtIndexInUrl
**
** DESCRIPTION: This function inserts a url-param at a specified
**		index in a Sip Url
**
***************************************************************/

SipBool sip_insertUrlParamAtIndexInUrl
#ifdef ANSI_PROTO
	( SipUrl 	*pUrl,
	  SipParam	*pParam,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pUrl,pParam,dIndex,pErr)
	  SipUrl 	*pUrl;
	  SipParam	*pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElement_in_list;

	SIPDEBUGFN( "Entering InsertUrlParamAtIndexInUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the param structure */
	if ( pParam == SIP_NULL )
		pElement_in_list = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pElement_in_list = pParam;
#else
		sip_initSipParam(&pElement_in_list, pErr);

		if ( __sip_cloneSipParam(pElement_in_list, pParam,pErr) == SipFail)
		{
			 sip_freeSipParam(pElement_in_list);
			return SipFail;
		}
#endif
	}

	if( sip_listInsertAt(&(pUrl->slParam), dIndex, pElement_in_list, pErr)\
																	== SipFail)
	{
		if ( pElement_in_list != SIP_NULL )
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pParam->dRefCount);
#else
			 sip_freeSipParam(pElement_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN( "Exiting InsertUrlParamAtIndexInUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_deleteUrlParamAtIndexInUrl
**
** DESCRIPTION: This function deletes a url-param at a specified
**		index in a Sip Url
**
***************************************************************/

SipBool sip_deleteUrlParamAtIndexInUrl
#ifdef ANSI_PROTO
	( SipUrl 	*pUrl,
	  SIP_U32bit dIndex,
	  SipError 	*pErr )
#else
	( pUrl,dIndex,pErr)
	  SipUrl 	*pUrl;
	  SIP_U32bit dIndex;
	  SipError 	*pErr;
#endif
{
	SIPDEBUGFN( "Entering deleteUrlParamAtIndexInUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pUrl->slParam), dIndex, pErr) == SipFail)
		return SipFail;


	SIPDEBUGFN( "Exiting deleteUrlParamAtIndexInUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}




/***************************************************************
** FUNCTION: sip_getAuthorizationParamCountFromChallenge
**
** DESCRIPTION: This function retrieves the number of
**		authorization-pParam from a SIP Challenge
**
***************************************************************/

SipBool sip_getAuthorizationParamCountFromChallenge
#ifdef ANSI_PROTO
	( SipGenericChallenge	*pSch,
	  SIP_U32bit	*pIndex,
	  SipError	*pErr  )
#else
	( pSch,pIndex,pErr)
	  SipGenericChallenge 	*pSch;
	  SIP_U32bit 	*pIndex;
	  SipError 	*pErr;
#endif
{
	SIPDEBUGFN( "Entering getAuthorizationParamCountFromChallenge");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if ( (pSch == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pSch->slParam), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN( "Exiting getAuthorizationParamCountFromChallenge");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************
** FUNCTION: sip_getAuthorizationParamAtIndexFromChallenge
**
** DESCRIPTION: This function retrieves an Authorization-param at
**		a specified index from a SIP Challenge
**
***************************************************************/

SipBool sip_getAuthorizationParamAtIndexFromChallenge
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipGenericChallenge 	*pSch,
	  SipParam **ppParam,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( SipGenericChallenge 	*pSch,
	  SipParam *pParam,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pSch,ppParam,dIndex,pErr)
	  SipGenericChallenge 	*pSch;
	  SipParam **ppParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#else
	( pSch,pParam,dIndex,pErr)
	  SipGenericChallenge 	*pSch;
	  SipParam *pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
#endif
{
	SIP_Pvoid 	pElement_from_list;

	SIPDEBUGFN( "Entering getAuthorizationParamAtIndexFromChallenge");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

#ifdef SIP_BY_REFERENCE
	if ( (pSch == SIP_NULL) || (ppParam == SIP_NULL) )
#else
	if ( (pSch == SIP_NULL) || (pParam == SIP_NULL) )
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif

	if( sip_listGetAt(&(pSch->slParam), dIndex, &pElement_from_list, pErr)\
																	== SipFail)
		return SipFail;

	if ( pElement_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(((SipParam *)pElement_from_list)->dRefCount);
	*ppParam = (SipParam *) pElement_from_list;

#else
	if ( __sip_cloneSipParam(pParam,(SipParam *)pElement_from_list,pErr)\
																	== SipFail)
	{
		sip_freeSipParam((SipParam *) pElement_from_list);
		return SipFail;
	}
#endif

	SIPDEBUGFN( "Exiting getAuthorizationParamAtIndexFromChallenge");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_setAuthorizationParamAtIndexInChallenge
**
** DESCRIPTION: This function sets an authorization-param at a
**		specified index in a SIP Challenge
**
***************************************************************/

SipBool sip_setAuthorizationParamAtIndexInChallenge
#ifdef ANSI_PROTO
	( SipGenericChallenge 	*pSch,
	  SipParam	*pParam,
	  SIP_U32bit dIndex,
	  SipError 	*pErr )
#else
	( pSch,pParam,dIndex,pErr)
	  SipGenericChallenge 	*pSch;
	  SipParam	*pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElement_in_list;

	SIPDEBUGFN( "Entering setAuthorizationParamAtIndexInChallenge");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pSch == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		pElement_in_list = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pElement_in_list = pParam;
#else
		sip_initSipParam(&pElement_in_list,pErr);

		if ( __sip_cloneSipParam(pElement_in_list, pParam,pErr) == SipFail)
		{
			sip_freeSipParam(pElement_in_list);
			return SipFail;
		}
#endif
	}

	if( sip_listSetAt(&(pSch->slParam), dIndex, (SIP_Pvoid) pElement_in_list, pErr) == SipFail)
	{
		if ( pElement_in_list != SIP_NULL )
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pParam->dRefCount);
#else
			sip_freeSipParam(pElement_in_list);
#endif
		return SipFail;
	}
	SIPDEBUGFN( "Exiting setAuthorizationParamAtIndexInChallenge");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_insertAuthorizationParamAtIndexInChallenge
**
** DESCRIPTION: This function inserts an authorization-param at a
**		specified index in a SIp Challenge
**
***************************************************************/

SipBool sip_insertAuthorizationParamAtIndexInChallenge
#ifdef ANSI_PROTO
	( SipGenericChallenge 	*pSch,
	  SipParam	*pParam,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pSch,pParam,dIndex,pErr)
	  SipGenericChallenge 	*pSch;
	  SipParam	*pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElement_in_list;

	SIPDEBUGFN( "Entering InsertAuthorizationParamAtIndexInChallenge");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pSch == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the param structure */
	if ( pParam == SIP_NULL )
		pElement_in_list = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pElement_in_list = pParam;
#else
		sip_initSipParam(&pElement_in_list,pErr);
		if ( __sip_cloneSipParam(pElement_in_list, pParam,pErr) == SipFail)
		{
			 sip_freeSipParam(pElement_in_list);
			return SipFail;
		}
#endif
	}

	if( sip_listInsertAt(&(pSch->slParam), dIndex, pElement_in_list, pErr)\
																	== SipFail)
	{
		if ( pElement_in_list != SIP_NULL )
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pParam->dRefCount);
#else
			sip_freeSipParam(pElement_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN( "Exiting InsertAuthorizationParamAtIndexInChallenge");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***************************************************************
** FUNCTION: sip_deleteAuthorizationParamAtIndexInChallenge
**
** DESCRIPTION: This function deletes an authorization-param at a
**		secified index in a SIP Challenge
**
***************************************************************/

SipBool sip_deleteAuthorizationParamAtIndexInChallenge
#ifdef ANSI_PROTO
	( SipGenericChallenge 	*pSch,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pSch,dIndex,pErr)
	  SipGenericChallenge 	*pSch;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SIPDEBUGFN( "Entering deleteAuthorizationParamAtIndexInChallenge");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pSch == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pSch->slParam), dIndex, pErr) == SipFail)
		return SipFail;


	SIPDEBUGFN( "Exiting deleteAuthorizationParamAtIndexInChallenge");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}



/****************************************************************
**
** FUNCTION: sip_getUriFromAlertInfoHdr
**
** DESCRIPTION: This function retrives the Uri field from the
** AlertInfo pHeader.
**
*****************************************************************/

SipBool sip_getUriFromAlertInfoHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit **ppUri, SipError *pErr)
#else
	( pHdr, ppUri, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit **ppUri;
	  SipError *pErr;
#endif
{
	SIP_S8bit * pUri;

	SIPDEBUGFN ( "Entering getUriFromAlertInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL || ppUri == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	/* checking for correctness of the pHeader dType */
	if ( (pHdr->dType != SipHdrTypeAlertInfo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pUri = ( (SipAlertInfoHeader *) (pHdr->pHeader) )->pUri;

	if( pUri == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppUri = pUri;
#else
	/* duplicating the pValue to be returned */
	*ppUri = (SIP_S8bit *) STRDUPACCESSOR( pUri);
	if( *ppUri == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getUriFromAlertInfoHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/****************************************************************
**
** FUNCTION: sip_setUriInAlertInfoHdr
**
** DESCRIPTION: This function sets the dOption field in the Supported
** pHeader.
**
*****************************************************************/
SipBool sip_setUriInAlertInfoHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit *pUri, SipError *pErr)
#else
	( pHdr, pUri, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit *pUri;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_S8bit * pTempAlertInfo;
#endif
	SipAlertInfoHeader * pTempAlertInfoHdr;

	SIPDEBUGFN ( "Entering setUriInAlertInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	/* checking for correctness of the pHeader dType */
	if ( (pHdr->dType != SipHdrTypeAlertInfo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* freeing original pValue if it exists */
	pTempAlertInfoHdr = ( SipAlertInfoHeader *) ( pHdr->pHeader);
	if ( pTempAlertInfoHdr->pUri != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)\
							(&(pTempAlertInfoHdr->pUri)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempAlertInfoHdr->pUri = pUri;
#else
	if( pUri == SIP_NULL)
		pTempAlertInfo = SIP_NULL;
	else
	{
		pTempAlertInfo = (SIP_S8bit *) STRDUPACCESSOR ( pUri );
		if ( pTempAlertInfo == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempAlertInfoHdr->pUri = pTempAlertInfo;
#endif

	SIPDEBUGFN ( "Exiting setUriInAlertInfoHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromAlertInfoHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Encryption pHeader
**
***************************************************************/
SipBool sip_getParamCountFromAlertInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromAlertInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( (hdr->dType != SipHdrTypeAlertInfo))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( (hdr->pHeader == SIP_NULL) )
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

	if (count == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipAlertInfoHeader *)(hdr->pHeader))->slParam),\
													count , err) == SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromAlertInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromAlertInfoHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP AlertInfo Header
**
***************************************************************/
SipBool sip_getParamAtIndexFromAlertInfoHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipParam *pParam, SIP_U32bit index, SipError *err)
#else
	(SipHeader *hdr, SipParam **ppParam, SIP_U32bit index, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pParam, index, err)
	SipHeader *hdr;
	SipParam *pParam;
	SIP_U32bit index;
	SipError *err;
#else
	(hdr, ppParam, index, err)
	SipHeader *hdr;
	SipParam **ppParam;
	SIP_U32bit index;
	SipError *err;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromAlertInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ((hdr->dType != SipHdrTypeAlertInfo))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( (hdr->pHeader == SIP_NULL) )
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipAlertInfoHeader *)(hdr->pHeader))->slParam),\
		index, &element_from_list, err) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromAlertInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInAlertInfoHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp AlertInfo pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInAlertInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipParam *pParam, SIP_U32bit index, SipError *err)
#else
	(hdr, pParam, index, err)
	SipHeader *hdr;
	SipParam *pParam;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInAlertInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( (hdr ->dType != SipHdrTypeAlertInfo))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( (hdr->pHeader == SIP_NULL) )
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, err) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipAlertInfoHeader *)(hdr->pHeader))->slParam),  \
		index, (SIP_Pvoid)(temp_param), err) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInAlertInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInAlertInfoHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP AlertInfo pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInAlertInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInAlertInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ((hdr->dType != SipHdrTypeAlertInfo))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ((hdr->pHeader == SIP_NULL))
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipAlertInfoHeader *)(hdr->pHeader))->slParam),\
														index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInAlertInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInAlertInfoHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP AlertInfo pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInAlertInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipParam *pParam, SIP_U32bit index, SipError *err)
#else
 	(hdr, pParam, index, err)
 	SipHeader *hdr;
 	SipParam *pParam;
 	SIP_U32bit index;
 	SipError *err;
#endif
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInAlertInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ((hdr ->dType != SipHdrTypeAlertInfo))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( (hdr->pHeader == SIP_NULL) )
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, err) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipAlertInfoHeader *)(hdr->pHeader))->slParam),  \
		index, (SIP_Pvoid)(temp_param), err) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInAlertInfoHdr");
	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: sip_getCallIdFromInReplyToHdr
**
** DESCRIPTION: This function retrives the CallId field from the
** InReplyTo pHeader.
**
*****************************************************************/

SipBool sip_getCallIdFromInReplyToHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit **ppCallId, SipError *pErr)
#else
	( pHdr, ppCallId, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit **ppCallId;
	  SipError *pErr;
#endif
{
	SIP_S8bit * pCallId;

	SIPDEBUGFN ( "Entering getCallIdFromInReplyToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL || ppCallId == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	/* checking for correctness of the pHeader dType */
	if ( (pHdr->dType != SipHdrTypeInReplyTo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pCallId = ( (SipInReplyToHeader *) (pHdr->pHeader) )->pCallId;

	if( pCallId == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppCallId = pCallId;
#else
	/* duplicating the pValue to be returned */
	*ppCallId = (SIP_S8bit *) STRDUPACCESSOR( pCallId);
	if( *ppCallId == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getCallIdFromInReplyToHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/****************************************************************
**
** FUNCTION: sip_setCallIdInInReplyToHdr
**
** DESCRIPTION: This function sets the dOption field in the Supported
** pHeader.
**
*****************************************************************/
SipBool sip_setCallIdInInReplyToHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit *pCallId, SipError *pErr)
#else
	( pHdr, pCallId, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit *pCallId;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_S8bit * pTempInReplyTo;
#endif
	SipInReplyToHeader * pTempInReplyToHdr;

	SIPDEBUGFN ( "Entering setCallIdInInReplyToHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	/* checking for correctness of the pHeader dType */
	if ( (pHdr->dType != SipHdrTypeInReplyTo))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( (pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* freeing original pCallId if it exists */
	pTempInReplyToHdr = ( SipInReplyToHeader *) ( pHdr->pHeader);
	if ( pTempInReplyToHdr->pCallId != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,\
			(SIP_Pvoid *)(&(pTempInReplyToHdr->pCallId)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempInReplyToHdr->pCallId = pCallId;
#else
	if( pCallId == SIP_NULL)
		pTempInReplyTo = SIP_NULL;
	else
	{
		pTempInReplyTo = (SIP_S8bit *) STRDUPACCESSOR ( pCallId );
		if ( pTempInReplyTo == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempInReplyToHdr->pCallId = pTempInReplyTo;
#endif

	SIPDEBUGFN ( "Exiting setCallIdInInReplyToHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/*============= Fourth  Level APIs =======================*/

/***************************************************************
** FUNCTION: sip_getUrlParamTypeFromUrlParam
**
** DESCRIPTION: This function retrieves the Url-param dType from a
**		SIP Url-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getUrlParamTypeFromUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,	/*UrlParam pHeader */
	  en_UrlParamType * dType,
	  SipError * pErr)
#else
	( pUrl_param, dType, pErr )
	  SipUrlParam * pUrl_param;
	  en_UrlParamType * dType;     /*UrlParam pHeader */
	  SipError * pErr;
#endif
{


	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering getUrlParamTypeFromUrlParam");

	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if(dType == SIP_NULL )
	{
		*pErr=E_INV_PARAM;
		return SipFail;
	 }


	*dType = ( (SipUrlParam *) pUrl_param )->dType;

	SIPDEBUGFN( "Exiting getUrlParamTypeFromUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_getTransportParamFromUrlParam
**
** DESCRIPTION: This function retrieves the transport-param field
**		from a SIp Url-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getTransportParamFromUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,	/*UrlParam pHeader */
	  en_TransportParam * dTranspParam,
	  SipError * pErr)
#else
	( pUrl_param, dTranspParam, pErr )
	  SipUrlParam * pUrl_param;
	  en_TransportParam * dTranspParam;     /*UrlParam pHeader */
	  SipError * pErr;
#endif
{

	if( pErr == SIP_NULL )
		return SipFail;

 	SIPDEBUGFN( "Entering getTransportParamFromUrlParam");

	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if(pUrl_param->dType!=SipUrlParamTransport)
	{
		*pErr=E_INV_HEADER;
		return SipFail;
	}

	if(dTranspParam == SIP_NULL )
	{
    		*pErr = E_INV_PARAM;
		return SipFail;
	}


	*dTranspParam = ( (SipUrlParam *) pUrl_param )->u.dTranspParam;

	SIPDEBUGFN( "Exiting getTransportParamFromUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setTransportParamInUrlParam
**
** DESCRIPTION: This function sets the transport-param field in a
**		SIP Url-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setTransportParamInUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,    /* UrlParam pHeader */
	  en_TransportParam dTranspParam,
	  SipError * pErr)
#else
	( pUrl_param, dTranspParam, pErr )
	  SipUrlParam * pUrl_param;    /* UrlParam pHeader */
	  en_TransportParam dTranspParam;
	  SipError * pErr;
#endif
{

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering setTransportParamInUrlParam");
	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}


	( (SipUrlParam *) pUrl_param )->u.dTranspParam = dTranspParam;

	pUrl_param->dType =SipUrlParamTransport;

	 SIPDEBUGFN( "Exiting setTransportParamInUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
/***************************************************************
** FUNCTION: sip_getMethodParamFromUrlParam
**
** DESCRIPTION: This function retrieves the pMethod-parm field from
**		a IP Url param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getMethodParamFromUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,
	  SIP_S8bit ** pMethod,
	  SipError * pErr)
#else
	( pUrl_param, pMethod, pErr )
	  SipUrlParam * pUrl_param;
	  SIP_S8bit **pMethod;
	  SipError * pErr;
#endif
{
	SIP_U32bit dLength;
	SIP_S8bit * temp_method;

 	SIPDEBUGFN( "Entering getMethodParamFromUrlParam");

	if( pErr == SIP_NULL )
		return SipFail;


	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if(pUrl_param->dType!=SipUrlParamMethod)
	{
		*pErr=E_INV_HEADER;
		return SipFail;
	}

	temp_method = pUrl_param->u.pMethod;

	if( temp_method == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	dLength = strlen(temp_method);
	*pMethod = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *pMethod == SIP_NULL )
		return SipFail;

	strcpy( *pMethod , temp_method );


/*	if(pMethod == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}


	*pMethod = ( (SipUrlParam *) pUrl_param )->u.pMethod;
*/
 SIPDEBUGFN( "Exiting getMethodParamFromUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setMethodParamInUrlParam
**
** DESCRIPTION: This function sets the pMethod-paramfield in a SIP
**		Url-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setMethodParamInUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,
	  SIP_S8bit *pMethod,
	  SipError * pErr)
#else
	( pUrl_param, pMethod, pErr )
	  SipUrlParam * pUrl_param;
	  SIP_S8bit * pMethod;
	  SipError * pErr;
#endif
{
	SIP_U32bit dLength;
	SIP_S8bit * temp_method;


 	SIPDEBUGFN( "Entering setMethodParamInUrlParam");

	if( pErr == SIP_NULL )
		return SipFail;


	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	if(pUrl_param->dType!=SipUrlParamMethod)
	{
		*pErr=E_INV_HEADER;
		return SipFail;
	}

	if( pMethod == SIP_NULL)
		temp_method = SIP_NULL;
	else
	{
		dLength = strlen( pMethod );
		temp_method = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_method== SIP_NULL )
			return SipFail;

		strcpy( temp_method, pMethod );
	}


	if ( pUrl_param->u.pMethod != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pUrl_param->u.pMethod), pErr) == SipFail)
			return SipFail;
	}


	pUrl_param->u.pMethod = temp_method;



	SIPDEBUGFN( "Exiting setMethodParamInUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
/***************************************************************
** FUNCTION: sip_getUserParamFromUrlParam
**
** DESCRIPTION: This function retrieves the pUser-param field from
**		a SIP Url-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getUserParamFromUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,	/*UrlParam pHeader */
	  en_UserParam * dUserParam,
	  SipError * pErr)
#else
	( pUrl_param, dUserParam, pErr )
	  SipUrlParam * pUrl_param;
	  en_UserParam * dUserParam;     /*UrlParam pHeader */
	  SipError * pErr;
#endif
{

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering getUserParamFromUrlParam");

	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if ( pUrl_param->dType != SipUrlParamUser)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if(dUserParam == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}


	*dUserParam = ( (SipUrlParam *) pUrl_param )->u.dUserParam;

	 SIPDEBUGFN( "Exiting getUserParamFromUrlParam");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setUserParamInUrlParam
**
** DESCRIPTION: This function sets the pUser-param field in a SIP
**		Url-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setUserParamInUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,    /* Url Param pHeader */
	  en_UserParam dUserParam,
	  SipError * pErr)
#else
	( pUrl_param, dUserParam, pErr )
	  SipUrlParam * pUrl_param;    /* Url Param pHeader */
	  en_UserParam dUserParam;
	  SipError * pErr;
#endif
{

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering setUserParamInUrlParam");

	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
	if ( pUrl_param->dType != SipUrlParamUser)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}


	( (SipUrlParam *) pUrl_param )->u.dUserParam = dUserParam;


	SIPDEBUGFN( "Exiting setUserParamInUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_getTtlParamFromUrlParam
**
** DESCRIPTION: This function retrieves The Ttl-param field from a
**		SIP Url param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getTtlParamFromUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,	/*UrlParam pHeader */
	  SIP_U8bit * dTtl,
	  SipError * pErr)
#else
	( pUrl_param, dTtl, pErr )
	  SipUrlParam * pUrl_param;
	  SIP_U8bit * dTtl;     /*UrlParam pHeader */
	  SipError * pErr;
#endif
{

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering getTtlParamFromUrlParam");

	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if ( pUrl_param->dType != SipUrlParamTtl)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}


	if(dTtl == SIP_NULL )
	{
    		*pErr = E_INV_PARAM;
		return SipFail;
	}


	*dTtl = ( (SipUrlParam *) pUrl_param )->u.dTtl;

 SIPDEBUGFN( "Exiting getTtlParamFromUrlParam");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setTtlParamInUrlParam
**
** DESCRIPTION: This function sets a Tttl-param field in a SIP Url
*8		param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setTtlParamInUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,    /* UrlParam pHeader */
	  SIP_U8bit dTtl,
	  SipError * pErr)
#else
	( pUrl_param, dTtl, pErr )
	  SipUrlParam * pUrl_param;    /* UrlParam pHeader */
	  SIP_U8bit dTtl;
	  SipError * pErr;
#endif
{

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering setTtlParamInUrlParam");
	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if ( pUrl_param->dType != SipUrlParamTtl)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}


	( (SipUrlParam *) pUrl_param )->u.dTtl = dTtl;


	SIPDEBUGFN( "Exiting setTtlParamInUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
/***************************************************************
** FUNCTION: sip_getMaddrParamFromUrlParam
**
** DESCRIPTION: This function retrieves the 'm-dAddr' field from a
**		SIP url param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getMaddrParamFromUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,	/*UrlParam pHeader */
	  SIP_S8bit ** pMaddr,
	  SipError * pErr)
#else
	( pUrl_param, pMaddr, pErr )
	  SipUrlParam * pUrl_param;      /*UrlParam pHeader */
	  SIP_S8bit ** pMaddr;
	  SipError * pErr;
#endif
{

	SIP_U32bit dLength;
	SIP_S8bit * temp_maddr_param;

	if( pErr == SIP_NULL )
		return SipFail;

SIPDEBUGFN( "Entering getMaddrParamFromUrlParam");

	if ( pUrl_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if ( pUrl_param->dType != SipUrlParamMaddr )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	temp_maddr_param = ( (SipUrlParam *) pUrl_param )->u.pMaddr;

	if( temp_maddr_param == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	dLength = strlen(temp_maddr_param );
	*pMaddr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *pMaddr == SIP_NULL )
		return SipFail;

	strcpy( *pMaddr, temp_maddr_param );

	SIPDEBUGFN( "Exiting getMaddrParamFromUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setMaddrParamInUrlParam
**
** DESCRIPTION: This function sets the m-dAddr field in a SIP Url
**		param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setMaddrParamInUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * pUrl_param,	/*UrlParam pHeader */
	  SIP_S8bit * pMaddr,
	  SipError * pErr)
#else
	( pUrl_param, pMaddr, pErr )	/*UrlParam pHeader */
	  SipUrlParam * pUrl_param;
	  SIP_S8bit * pMaddr;
	  SipError * pErr;
#endif

{
	SIP_U32bit dLength;
	SIP_S8bit * temp_maddr_param;
	SipUrlParam *temp_url_param_hdr;

	if( pErr == SIP_NULL )
		return SipFail;

	 SIPDEBUGFN( "Entering setMaddrParamInUrlParam");
	if ( url_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}


	if ( url_param->dType != SipUrlParamMaddr )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pMaddr == SIP_NULL)
		temp_maddr_param = SIP_NULL;
	else
	{
		dLength = strlen( pMaddr );
		temp_maddr_param = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_maddr_param == SIP_NULL )
			return SipFail;
		strcpy( temp_maddr_param, pMaddr );
	}

	temp_url_param_hdr = ( SipUrlParam *) ( url_param);

	if ( temp_url_param_hdr->u.pMaddr != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( temp_url_param_hdr->u.pMaddr), pErr) == SipFail)
			return SipFail;
	}


	temp_url_param_hdr->u.pMaddr = temp_maddr_param;


	SIPDEBUGFN( "Exitiing setMaddrParamInUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
/***************************************************************
** FUNCTION: sip_getOtherParamFromUrlParam
**
** DESCRIPTION: This function retieves an other-param from a SIP
**		Url param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getOtherParamFromUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * url_param,
	  SIP_S8bit ** other,
	  SipError * pErr)
#else
	( url_param, other, pErr )
	  SipUrlParam * url_param;
	  SIP_S8bit ** other;
	  SipError * pErr;
#endif
{

	SIP_U32bit dLength;
	SIP_S8bit * temp_other_param;

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering getOtherparamFromUrlParam");
	if ( url_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if ( url_param->dType != SipUrlParamOther )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	temp_other_param = ( (SipUrlParam *) url_param )->u.pOtherParam;

	if( temp_other_param == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	dLength = strlen(temp_other_param );
	*other = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *other == SIP_NULL )
		return SipFail;

	strcpy( *other, temp_other_param );


	SIPDEBUGFN( "Exiting getOtherparamFromUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setOtherParamInUrlParam
**
** DESCRIPTION: This function sets an 'other-param' in a SIP Url
**		Param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setOtherParamInUrlParam
#ifdef ANSI_PROTO
	( SipUrlParam * url_param,
	  SIP_S8bit * other,
	  SipError * pErr)
#else
	( url_param, other, pErr )
	  SipUrlParam * url_param;
	  SIP_S8bit * other;
	  SipError * pErr;
#endif

{
	SIP_U32bit dLength;
	SIP_S8bit * temp_other_param;
	SipUrlParam *temp_url_param_hdr;

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering setOtherparamInUrlParam");

	if ( url_param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	if ( url_param->dType != SipUrlParamOther )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}


	if( other == SIP_NULL)
		temp_other_param = SIP_NULL;
	else
	{
		dLength = strlen( other );
		temp_other_param = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_other_param == SIP_NULL )
			return SipFail;
		strcpy( temp_other_param, other);
	}

	temp_url_param_hdr = ( SipUrlParam *) ( url_param);

	if ( temp_url_param_hdr->u.pOtherParam != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( temp_url_param_hdr->u.pOtherParam), pErr) == SipFail)
			return SipFail;
	}


	temp_url_param_hdr->u.pOtherParam = temp_other_param;

	url_param->dType = SipUrlParamOther;

	SIPDEBUGFN( "Exitiing setOtherparamInUrlParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
/***************************************************************
** FUNCTION: sip_getTokenFromAuthorizationParam
**
** DESCRIPTION: This function retrieves the pToken from a SIP
**		Authorization parameter
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getTokenFromAuthorizationParam
#ifdef ANSI_PROTO
	( SipAuthParam * param,	     /* Authorizatioparam pHeader */
	  SIP_S8bit ** pToken,
	  SipError * pErr)
#else
	( param, pToken, pErr )
	  SipAuthParam * param;      /*AuthorizationParam pHeader */
	  SIP_S8bit ** pToken;
	  SipError * pErr;
#endif
{

	SIP_U32bit dLength;
	SIP_S8bit * pTemp_token_param;

	if( pErr == SIP_NULL )
		return SipFail;

 SIPDEBUGFN( "Entering getTokenFromAuthorizationParam");
	if ( param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}


	pTemp_token_param = ( (SipAuthParam *) param )->pToken;

	if( pTemp_token_param == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	dLength = strlen(pTemp_token_param );
	*pToken = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *pToken == SIP_NULL )
		return SipFail;

	strcpy( *pToken, pTemp_token_param );

	SIPDEBUGFN( "Exiting getTokenFromAuthorizationParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setTokenInAuthorizationParam
**
** DESCRIPTION: This function sets the pToken in a SIP authorization
**		param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setTokenInAuthorizationParam
#ifdef ANSI_PROTO
	( SipAuthParam * param,	/*AuthorizationParam pHeader */
	  SIP_S8bit * pToken,
	  SipError * pErr)
#else
	( param, pToken, pErr )	/*AuthorizationParam pHeader */
	  SipAuthParam * param;
	  SIP_S8bit * pToken;
	  SipError * pErr;
#endif

{
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_token_param;
	SipAuthParam *temp_auth_param_hdr;

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering setTokenInAuthorizationParam");
	if ( param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}


	if( pToken == SIP_NULL)
		pTemp_token_param = SIP_NULL;
	else
	{
		dLength = strlen( pToken );
		pTemp_token_param = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_token_param == SIP_NULL )
			return SipFail;
		strcpy( pTemp_token_param, pToken );
	}

	temp_auth_param_hdr = ( SipAuthParam *) ( param);
	if ( temp_auth_param_hdr->pToken != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( temp_auth_param_hdr->pToken), pErr) == SipFail)
			return SipFail;
	}


	temp_auth_param_hdr->pToken = pTemp_token_param;

	 SIPDEBUGFN( "Exitiing setTokenInAuthorizationParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
/***************************************************************
** FUNCTION: sip_getValueFromAuthorizationParam
**
** DESCRIPTION: This function retrieves the pValue field from a SIP
**		Authorization-param
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_getValueFromAuthorizationParam
#ifdef ANSI_PROTO
	( SipAuthParam * param,	     /* Authorizatioparam pHeader */
	  SIP_S8bit ** pValue,
	  SipError * pErr)
#else
	( param, pValue, pErr )
	  SipAuthParam * param;      /*AuthorizationParam pHeader */
	  SIP_S8bit ** pValue;
	  SipError * pErr;
#endif
{

	SIP_U32bit dLength;
	SIP_S8bit * temp_value_param;

	if( pErr == SIP_NULL )
		return SipFail;

SIPDEBUGFN( "Entering getValueFromAuthorizationParam");

	if ( param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}

	temp_value_param = ( (SipAuthParam *) param )->pValue;

	if( temp_value_param == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	dLength = strlen(temp_value_param );
	*pValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *pValue == SIP_NULL )
		return SipFail;

	strcpy( *pValue, temp_value_param );

 SIPDEBUGFN( "Exiting getValueFromAuthorizationParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/***************************************************************
** FUNCTION: sip_setValueInAuthorizationParam
**
** DESCRIPTION: This function sets the pValue field in a SIP
**		Authorization parameter
**
***************************************************************/
/**************************************************
 *** Masked off *** NOT USED *******/
#if 0
SipBool sip_setValueInAuthorizationParam
#ifdef ANSI_PROTO
	( SipAuthParam * param,	/*AuthorizationParam pHeader */
	  SIP_S8bit * pValue,
	  SipError * pErr)
#else
	( param, pValue, pErr )	/*AuthorizationParam pHeader */
	  SipAuthParam * param;
	  SIP_S8bit * pValue;
	  SipError * pErr;
#endif

{
	SIP_U32bit dLength;
	SIP_S8bit * temp_value_param;
	SipAuthParam *temp_auth_param_hdr;

	if( pErr == SIP_NULL )
		return SipFail;

	SIPDEBUGFN( "Entering setValueInAuthorizationParam");
	if ( param == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}


	if( pValue == SIP_NULL)
		temp_value_param = SIP_NULL;
	else
	{
		dLength = strlen( pValue );
		temp_value_param = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_value_param == SIP_NULL )
			return SipFail;
		strcpy( temp_value_param, pValue );
	}
	temp_auth_param_hdr = ( SipAuthParam *) ( param);
	if ( temp_auth_param_hdr->pValue != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&( temp_auth_param_hdr->pValue), pErr) == SipFail)
			return SipFail;
	}


	temp_auth_param_hdr->pValue = temp_value_param;


	SIPDEBUGFN( "Exitiing setValueInAuthorizationParam");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromReplacesHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Replaces pHeader
**
**************************************************************/
SipBool sip_getParamCountFromReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( count == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipReplacesHeader *)(hdr->pHeader))->slParams), count , err) == SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromReplacesHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromReplacesHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP Replaces pHeader
**
***************************************************************/
SipBool sip_getParamAtIndexFromReplacesHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipNameValuePair *pParam, SIP_U32bit index, SipError *err)
#else
	(SipHeader *hdr, SipNameValuePair **ppParam, SIP_U32bit index, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pParam, index, err)
	SipHeader *hdr;
	SipNameValuePair *pParam;
	SIP_U32bit index;
	SipError *err;
#else
	(hdr, ppParam, index, err)
	SipHeader *hdr;
	SipNameValuePair **ppParam;
	SIP_U32bit index;
	SipError *err;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipNameValuePair *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pParam == SIP_NULL)
#else
	if(  ppParam == SIP_NULL)
#endif
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipReplacesHeader *)(hdr->pHeader))->slParams), index,  \
		&element_from_list, err) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipNameValuePair *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipNameValuePair(pParam, temp_param, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromReplacesHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInReplacesHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp Replaces pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipNameValuePair *pParam, SIP_U32bit index, SipError *err)
#else
	(hdr, pParam, index, err)
	SipHeader *hdr;
	SipNameValuePair *pParam;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipNameValuePair *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, err) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(temp_param, pParam, err) == SipFail)
		{
			sip_freeSipNameValuePair (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipReplacesHeader *)(hdr->pHeader))->slParams),  \
		index, (SIP_Pvoid)(temp_param), err) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInReplacesHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInReplacesHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP Replaces pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipReplacesHeader *)(hdr->pHeader))->slParams), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInReplacesHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInReplacesHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP Replaces pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipNameValuePair *pParam, SIP_U32bit index, SipError *err)
#else
 	(hdr, pParam, index, err)
 	SipHeader *hdr;
 	SipNameValuePair *pParam;
 	SIP_U32bit index;
 	SipError *err;
#endif
{
	SipNameValuePair *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, err) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(temp_param, pParam, err) == SipFail)
		{
			sip_freeSipNameValuePair (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipReplacesHeader *)(hdr->pHeader))->slParams),  \
		index, (SIP_Pvoid)(temp_param), err) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInReplacesHdr");
	return SipSuccess;
}



/*****************************************************************
**
** FUNCTION:  sip_getFromTagFromReplacesHdr
**
** DESCRIPTION: This function retrieves the pFromTag field from a SIP
**		Replaces Header
**
***************************************************************/
SipBool sip_getFromTagFromReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pFromTag, SipError *err)
#else
	(hdr, pFromTag, err)
	SipHeader *hdr;
	SIP_S8bit **pFromTag;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering function sip_getFromTagFromReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pFromTag == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
 	temp_value = ((SipReplacesHeader *) (hdr ->pHeader))->pFromTag;
 	if (temp_value == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pFromTag = (SIP_S8bit *) STRDUPACCESSOR(temp_value);
	if (*pFromTag == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pFromTag = temp_value;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getFromTagFromReplacesHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setFromTagInReplacesHdr
**
** DESCRIPTION: This function sets the pFromTag field in a SIP replaces
**		pHeader
**
***************************************************************/
SipBool sip_setFromTagInReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pFromTag, SipError *err)
#else
	(hdr, pFromTag, err)
	SipHeader *hdr;
	SIP_S8bit *pFromTag;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value, *val;
	SIPDEBUGFN("Entering function sip_setFromTagInReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pFromTag == SIP_NULL)
                temp_value = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_value = (SIP_S8bit *) STRDUPACCESSOR(pFromTag);
		if (temp_value == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_value = pFromTag;
#endif
	}

        val = ((SipReplacesHeader *)(hdr->pHeader))->pFromTag;
        if ( val != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&val), err) == SipFail)
		{
			sip_freeString(temp_value);
			return SipFail;
		}
	}

	((SipReplacesHeader *)(hdr->pHeader))->pFromTag = temp_value;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setFromTagInReplacesHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getToTagFromReplacesHdr
**
** DESCRIPTION: This function retrieves the pToTag field from a SIP
**		Replaces Header
**
***************************************************************/
SipBool sip_getToTagFromReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pToTag, SipError *err)
#else
	(hdr, pToTag, err)
	SipHeader *hdr;
	SIP_S8bit **pToTag;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering function sip_getToTagFromReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pToTag == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
 	temp_value = ((SipReplacesHeader *) (hdr ->pHeader))->pToTag;
 	if (temp_value == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pToTag = (SIP_S8bit *) STRDUPACCESSOR(temp_value);
	if (*pToTag == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pToTag = temp_value;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getToTagFromReplacesHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setToTagInReplacesHdr
**
** DESCRIPTION: This function sets the pToTag field in a SIP replaces
**		pHeader
**
***************************************************************/
SipBool sip_setToTagInReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pToTag, SipError *err)
#else
	(hdr, pToTag, err)
	SipHeader *hdr;
	SIP_S8bit *pToTag;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value, *val;
	SIPDEBUGFN("Entering function sip_setToTagInReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pToTag == SIP_NULL)
                temp_value = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_value = (SIP_S8bit *) STRDUPACCESSOR(pToTag);
		if (temp_value == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_value = pToTag;
#endif
	}

        val = ((SipReplacesHeader *)(hdr->pHeader))->pToTag;
        if ( val != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&val), err) == SipFail)
		{
			sip_freeString(temp_value);
			return SipFail;
		}
	}

	((SipReplacesHeader *)(hdr->pHeader))->pToTag = temp_value;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setToTagInReplacesHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getCallidFromReplacesHdr
**
** DESCRIPTION: This function retrieves the pCallid field from a SIP
**		Replaces Header
**
***************************************************************/
SipBool sip_getCallidFromReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pCallid, SipError *err)
#else
	(hdr, pCallid, err)
	SipHeader *hdr;
	SIP_S8bit **pCallid;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering function sip_getCallidFromReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pCallid == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
 	temp_value = ((SipReplacesHeader *) (hdr ->pHeader))->pCallid;
 	if (temp_value == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pCallid = (SIP_S8bit *) STRDUPACCESSOR(temp_value);
	if (*pCallid == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pCallid = temp_value;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getCallidFromReplacesHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setCallidInReplacesHdr
**
** DESCRIPTION: This function sets the pCallid field in a SIP replaces
**		pHeader
**
***************************************************************/
SipBool sip_setCallidInReplacesHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pCallid, SipError *err)
#else
	(hdr, pCallid, err)
	SipHeader *hdr;
	SIP_S8bit *pCallid;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value, *val;
	SIPDEBUGFN("Entering function sip_setCallidInReplacesHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplaces)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pCallid == SIP_NULL)
                temp_value = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_value = (SIP_S8bit *) STRDUPACCESSOR(pCallid);
		if (temp_value == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_value = pCallid;
#endif
	}

        val = ((SipReplacesHeader *)(hdr->pHeader))->pCallid;
        if ( val != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&val), err) == SipFail)
		{
			sip_freeString(temp_value);
			return SipFail;
		}
	}

	((SipReplacesHeader *)(hdr->pHeader))->pCallid = temp_value;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setCallidInReplacesHdr");
	return SipSuccess;
}

#ifdef SIP_CONF
/***************** Accessor functions for JOIN header **************/

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromJoinHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Join pHeader
**
**************************************************************/
SipBool sip_getParamCountFromJoinHdr
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)

{
	SIPDEBUGFN("Entering function sip_getParamCountFromJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( count == SIP_NULL)
	{
		*err = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromJoinHdr");
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromJoinHdr");
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromJoinHdr");
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromJoinHdr");
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipJoinHeader *)(hdr->pHeader))->slParams), count , err) == SipFail )
	{
	    SIPDEBUGFN("Exiting function sip_getParamCountFromJoinHdr");
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromJoinHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromJoinHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP Join pHeader
**
***************************************************************/
SipBool sip_getParamAtIndexFromJoinHdr
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipNameValuePair *pParam, SIP_U32bit index, SipError *err)
#else
	(SipHeader *hdr, SipNameValuePair **ppParam, SIP_U32bit index, SipError *err)
#endif
{
	SIP_Pvoid element_from_list;
	SipNameValuePair *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pParam == SIP_NULL)
#else
	if(  ppParam == SIP_NULL)
#endif
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipJoinHeader *)(hdr->pHeader))->slParams), index,  \
		&element_from_list, err) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipNameValuePair *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipNameValuePair(pParam, temp_param, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getParamAtIndexFromJoinHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInJoinHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp Join pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInJoinHdr
	(SipHeader *hdr, SipNameValuePair *pParam, SIP_U32bit index, SipError *err)
{
	SipNameValuePair *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, err) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(temp_param, pParam, err) == SipFail)
		{
			sip_freeSipNameValuePair (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipJoinHeader *)(hdr->pHeader))->slParams),  \
		index, (SIP_Pvoid)(temp_param), err) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_insertParamAtIndexInJoinHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInJoinHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP Join pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInJoinHdr
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipJoinHeader *)(hdr->pHeader))->slParams), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_deleteParamAtIndexInJoinHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInJoinHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP Join pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInJoinHdr
	(SipHeader *hdr, SipNameValuePair *pParam, SIP_U32bit index, SipError *err)
{
	SipNameValuePair *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, err) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(temp_param, pParam, err) == SipFail)
		{
			sip_freeSipNameValuePair (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipJoinHeader *)(hdr->pHeader))->slParams),  \
		index, (SIP_Pvoid)(temp_param), err) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setParamAtIndexInJoinHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getFromTagFromJoinHdr
**
** DESCRIPTION: This function retrieves the pFromTag field from a SIP
**		Join Header
**
***************************************************************/
SipBool sip_getFromTagFromJoinHdr
	(SipHeader *hdr, SIP_S8bit **pFromTag, SipError *err)
{
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering function sip_getFromTagFromJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pFromTag == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
 	temp_value = ((SipJoinHeader *) (hdr ->pHeader))->pFromTag;
 	if (temp_value == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pFromTag = (SIP_S8bit *) STRDUPACCESSOR(temp_value);
	if (*pFromTag == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pFromTag = temp_value;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getFromTagFromJoinHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setFromTagInJoinHdr
**
** DESCRIPTION: This function sets the pFromTag field in a SIP Join
**		pHeader
**
***************************************************************/
SipBool sip_setFromTagInJoinHdr
	(SipHeader *hdr, SIP_S8bit *pFromTag, SipError *err)
{
	SIP_S8bit *temp_value, *val;
	SIPDEBUGFN("Entering function sip_setFromTagInJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pFromTag == SIP_NULL)
                temp_value = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_value = (SIP_S8bit *) STRDUPACCESSOR(pFromTag);
		if (temp_value == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_value = pFromTag;
#endif
	}

        val = ((SipJoinHeader *)(hdr->pHeader))->pFromTag;
        if ( val != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&val), err) == SipFail)
		{
			sip_freeString(temp_value);
			return SipFail;
		}
	}

	((SipJoinHeader *)(hdr->pHeader))->pFromTag = temp_value;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setFromTagInJoinHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getToTagFromJoinHdr
**
** DESCRIPTION: This function retrieves the pToTag field from a SIP
**		Join Header
**
***************************************************************/
SipBool sip_getToTagFromJoinHdr
	(SipHeader *hdr, SIP_S8bit **pToTag, SipError *err)

{
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering function sip_getToTagFromJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pToTag == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
 	temp_value = ((SipJoinHeader *) (hdr ->pHeader))->pToTag;
 	if (temp_value == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pToTag = (SIP_S8bit *) STRDUPACCESSOR(temp_value);
	if (*pToTag == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pToTag = temp_value;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getToTagFromJoinHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setToTagInJoinHdr
**
** DESCRIPTION: This function sets the pToTag field in a SIP Join
**		pHeader
**
***************************************************************/
SipBool sip_setToTagInJoinHdr
	(SipHeader *hdr, SIP_S8bit *pToTag, SipError *err)
{
	SIP_S8bit *temp_value, *val;
	SIPDEBUGFN("Entering function sip_setToTagInJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pToTag == SIP_NULL)
                temp_value = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_value = (SIP_S8bit *) STRDUPACCESSOR(pToTag);
		if (temp_value == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_value = pToTag;
#endif
	}

        val = ((SipJoinHeader *)(hdr->pHeader))->pToTag;
        if ( val != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&val), err) == SipFail)
		{
			sip_freeString(temp_value);
			return SipFail;
		}
	}

	((SipJoinHeader *)(hdr->pHeader))->pToTag = temp_value;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setToTagInJoinHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getCallidFromJoinHdr
**
** DESCRIPTION: This function retrieves the pCallid field from a SIP
**		Join Header
**
***************************************************************/
SipBool sip_getCallidFromJoinHdr
	(SipHeader *hdr, SIP_S8bit **pCallid, SipError *err)
{
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering function sip_getCallidFromJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pCallid == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
 	temp_value = ((SipJoinHeader *) (hdr ->pHeader))->pCallid;
 	if (temp_value == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pCallid = (SIP_S8bit *) STRDUPACCESSOR(temp_value);
	if (*pCallid == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pCallid = temp_value;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getCallidFromJoinHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setCallidInJoinHdr
**
** DESCRIPTION: This function sets the pCallid field in a SIP Join
**		pHeader
**
***************************************************************/
SipBool sip_setCallidInJoinHdr
	(SipHeader *hdr, SIP_S8bit *pCallid, SipError *err)
{
	SIP_S8bit *temp_value, *val;
	SIPDEBUGFN("Entering function sip_setCallidInJoinHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeJoin)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
        if( pCallid == SIP_NULL)
                temp_value = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_value = (SIP_S8bit *) STRDUPACCESSOR(pCallid);
		if (temp_value == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_value = pCallid;
#endif
	}

        val = ((SipJoinHeader *)(hdr->pHeader))->pCallid;
        if ( val != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&val), err) == SipFail)
		{
			sip_freeString(temp_value);
			return SipFail;
		}
	}

	((SipJoinHeader *)(hdr->pHeader))->pCallid = temp_value;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setCallidInJoinHdr");
	return SipSuccess;
}
#endif /* #ifdef SIP_CONF */


#ifdef SIP_SECURITY
/*********************************************************
** FUNCTION:sip_getMechanismNameFromSecurityClientHdr
**
** DESCRIPTION: This function retrieves the mechanism-name field
**              from a SIP SecurityClient pHeader
**
**********************************************************/

SipBool sip_getMechanismNameFromSecurityClientHdr
        (SipHeader *pHdr,
        SIP_S8bit **ppMechname,
        SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
        SIP_U16bit dLength;
#endif
        SIP_S8bit * pTemp_Mechname;
        SIPDEBUGFN( "Entering getMechanismNameFromSecurityClientHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
        if ( ppMechname == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityClient))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
        pTemp_Mechname = ( (SipSecurityClientHeader *) (pHdr->pHeader) )->pMechanismName;
        if( pTemp_Mechname == SIP_NULL )
        {
                *pErr = E_NO_EXIST;
                return SipFail;
        }
#ifdef SIP_BY_REFERENCE
        *ppMechname = pTemp_Mechname;
#else

        dLength = strlen(pTemp_Mechname );
        *ppMechname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
        if ( *ppMechname == SIP_NULL )
	{		
		*pErr = E_NO_MEM;
                return SipFail;
	}
        strcpy( *ppMechname, pTemp_Mechname );
#endif

        SIPDEBUGFN( "Exiting getMechanismNameFromSecurityClientHdr");
        *pErr = E_NO_ERROR;
        return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setMechanismNameInSecurityClientHdr
**
** DESCRIPTION: This function sets the mechanism-name field in
**              a SIP Security-Client pHeader
**
**********************************************************/
SipBool sip_setMechanismNameInSecurityClientHdr
	(SipHeader *pHdr,
	SIP_S8bit *pMechname, 
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
        SIP_U32bit dLength;
        SIP_S8bit * pTemp_Mechname;
#endif
        SipSecurityClientHeader *pTemp_hdr;
        SIPDEBUGFN( "Entering setMechanismNameInSecurityClientHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityClient))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }

        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
        pTemp_hdr=(SipSecurityClientHeader *)(pHdr->pHeader);
        if(pTemp_hdr->pMechanismName !=SIP_NULL)
        {
                
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pMechanismName),pErr) ==SipFail)
                return SipFail;
        }
#ifdef SIP_BY_REFERENCE
        ((SipSecurityClientHeader *)(pHdr->pHeader))->pMechanismName = pMechname;
#else
        if( pMechname == SIP_NULL)
                pTemp_Mechname = SIP_NULL;
        else
        {
                dLength = strlen( pMechname );
                pTemp_Mechname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
                if ( pTemp_Mechname == SIP_NULL )
		 {               
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

                strcpy( pTemp_Mechname, pMechname );
        }
        pTemp_hdr->pMechanismName = pTemp_Mechname;
#endif
        SIPDEBUGFN( "Exiting setMechanismNameInSecurityClientHdr");
        *pErr = E_NO_ERROR;
        return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamCountFromSecurityClientHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**              from a SIP SecurityClient pHeader
**
**********************************************************/
SipBool sip_getParamCountFromSecurityClientHdr
        (SipHeader *pHdr, 
	SIP_U32bit *pCount, 
	SipError *pErr)
{
        SIPDEBUGFN("Entering function sip_getParamCountFromSecurityClientHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityClient))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
	if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }

        if (pCount == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif
        if (sip_listSizeOf( &(((SipSecurityClientHeader *)(pHdr->pHeader))->slParams), 
pCount , pErr) == SipFail )
        {
                return SipFail;
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getParamCountFromSecurityClientHdr");
        return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamAtIndexFromSecurityClientHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**              index in a Security-Client header
**
**********************************************************/
SipBool sip_getParamAtIndexFromSecurityClientHdr
#ifndef SIP_BY_REFERENCE
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
#else
        (SipHeader *pHdr, 
	SipParam **ppParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
#endif
{
        SIP_Pvoid element_from_list;
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_getParamAtIndexFromSecurityClientHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#ifndef SIP_BY_REFERENCE
        if (pParam == SIP_NULL)
#else
        if (ppParam == SIP_NULL)
#endif
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityClient))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL) )
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
      if (sip_listGetAt( &(((SipSecurityClientHeader *)(pHdr->pHeader))->slParams), dIndex,  \
                &element_from_list, pErr) == SipFail)
                return SipFail;

        if (element_from_list == SIP_NULL)
        {
                *pErr = E_NO_EXIST;
                return SipFail;
        }

        temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
        if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
                return SipFail;
#else
        HSS_LOCKEDINCREF(temp_param->dRefCount);
        *ppParam = temp_param;
#endif

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromSecurityClientHdr");
        return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInSecurityClientHdr
**
** DESCRIPTION: This function sets a param at a specified
**              index in a Security-Client header
**
**********************************************************/
SipBool sip_setParamAtIndexInSecurityClientHdr
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_setParamAtIndexInSecurityClientHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr ->dType != SipHdrTypeSecurityClient))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
  if ( pParam == SIP_NULL )
                temp_param = SIP_NULL;
        else
        {
      
#ifndef SIP_BY_REFERENCE
                if (sip_initSipParam(&temp_param, pErr) == SipFail)
                        return SipFail;
                if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
                {
                        sip_freeSipParam (temp_param);
                        return SipFail;
                }
#else
                HSS_LOCKEDINCREF(pParam->dRefCount);
                temp_param = pParam;
#endif
        }

        if( sip_listSetAt( &(((SipSecurityClientHeader *)(pHdr->pHeader))->slParams),  \
                dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
        {
                if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
                        sip_freeSipParam (temp_param);
#else
                HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
                return SipFail;
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_setParamAtIndexInSecurityClientHdr");
        return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_insertParamAtIndexInSecurityClientHdr
**
** DESCRIPTION: This function inserts a param at a specified
**              index in a Security-Client header
**
**********************************************************/
SipBool sip_insertParamAtIndexInSecurityClientHdr
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_insertParamAtIndexInSecurityClientHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr ->dType != SipHdrTypeSecurityClient))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
  if ( pParam == SIP_NULL )
                temp_param = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
                if (sip_initSipParam(&temp_param, pErr) == SipFail)
                        return SipFail;
                if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
                {
                        sip_freeSipParam (temp_param);
                        return SipFail;
                }
#else
                HSS_LOCKEDINCREF(pParam->dRefCount);
                temp_param = pParam;
#endif
        }

        if( sip_listInsertAt( &(((SipSecurityClientHeader *)(pHdr->pHeader))->slParams),  \
                dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
        {
                if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
                        sip_freeSipParam (temp_param);
#else
                HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
                return SipFail;
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_insertParamAtIndexInSecurityClientHdr");
        return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInSecurityClientHdr
**
** DESCRIPTION: This function deletes a param at a specified
**              index in a Security-Client header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInSecurityClientHdr
        (SipHeader *pHdr, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SIPDEBUGFN("Entering function sip_deleteParamAtIndexInSecurityClientHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityClient))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
        if( sip_listDeleteAt( &(((SipSecurityClientHeader *)(pHdr->pHeader))->slParams), 
dIndex, pErr) == SipFail)
                return SipFail;

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInSecurityClientHdr");
        return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getMechanismNameFromSecurityVerifyHdr
**
** DESCRIPTION: This function retrieves the mechanism-name field
**              from a SIP SecurityVerify pHeader
**
**********************************************************/

SipBool sip_getMechanismNameFromSecurityVerifyHdr
        (SipHeader *pHdr,
        SIP_S8bit **ppMechname,
        SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
        SIP_U16bit dLength;
#endif
        SIP_S8bit * pTemp_Mechname;
        SIPDEBUGFN( "Entering getMechanismNameFromSecurityVerifyHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
        if ( ppMechname == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityVerify))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
	if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
        pTemp_Mechname = ( (SipSecurityVerifyHeader *) (pHdr->pHeader) )->pMechanismName;
        if( pTemp_Mechname == SIP_NULL )
        {
                *pErr = E_NO_EXIST;
                return SipFail;
        }
#ifdef SIP_BY_REFERENCE
        *ppMechname = pTemp_Mechname;
#else

        dLength = strlen(pTemp_Mechname );
        *ppMechname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
        if ( *ppMechname == SIP_NULL )
	{               
                *pErr = E_NO_MEM;
                return SipFail;
        }

        strcpy( *ppMechname, pTemp_Mechname );
#endif

        SIPDEBUGFN( "Exiting getMechanismNameFromSecurityVerifyHdr");
        *pErr = E_NO_ERROR;
        return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setMechanismNameInSecurityVerifyHdr
**
** DESCRIPTION: This function sets the mechanism-name field in
**              a SIP Security-Verify pHeader
**
**********************************************************/
SipBool sip_setMechanismNameInSecurityVerifyHdr
	(SipHeader *pHdr,
	SIP_S8bit *pMechname, 
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
        SIP_U32bit dLength;
        SIP_S8bit * pTemp_Mechname;
#endif
        SipSecurityVerifyHeader *pTemp_hdr;
        SIPDEBUGFN( "Entering setMechanismNameInSecurityVerifyHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityVerify))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }

        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
        pTemp_hdr=(SipSecurityVerifyHeader *)(pHdr->pHeader);
        if(pTemp_hdr->pMechanismName !=SIP_NULL)
        {
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pMechanismName),pErr) ==SipFail)
                return SipFail;
        }
#ifdef SIP_BY_REFERENCE
        ((SipSecurityVerifyHeader *)(pHdr->pHeader))->pMechanismName = pMechname;
#else
        if( pMechname == SIP_NULL)
                pTemp_Mechname = SIP_NULL;
        else
        {
                dLength = strlen( pMechname );
                pTemp_Mechname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
                if ( pTemp_Mechname == SIP_NULL )
		{               
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

                strcpy( pTemp_Mechname, pMechname );
        }
        pTemp_hdr->pMechanismName = pTemp_Mechname;
#endif
        SIPDEBUGFN( "Exiting setMechanismNameInSecurityVerifyHdr");
        *pErr = E_NO_ERROR;
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_getParamCountFromSecurityVerifyHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**              from a SIP SecurityVerify pHeader
**
**********************************************************/
SipBool sip_getParamCountFromSecurityVerifyHdr
        (SipHeader *pHdr, 
	SIP_U32bit *pCount, 
	SipError *pErr)
{
        SIPDEBUGFN("Entering function sip_getParamCountFromSecurityVerifyHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityVerify))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }

        if (pCount == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif

        if (sip_listSizeOf( &(((SipSecurityVerifyHeader *)(pHdr->pHeader))->slParams),\
		pCount , pErr) == SipFail )
        {
                return SipFail;
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getParamCountFromSecurityVerifyHdr");
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_getParamAtIndexFromSecurityVerifyHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**              index in a Security-Verify header
**
**********************************************************/
SipBool sip_getParamAtIndexFromSecurityVerifyHdr
#ifndef SIP_BY_REFERENCE
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
#else
        (SipHeader *pHdr, 
	SipParam **ppParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
#endif
{
        SIP_Pvoid element_from_list;
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_getParamAtIndexFromSecurityVerifyHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#ifndef SIP_BY_REFERENCE
        if (pParam == SIP_NULL)
#else
        if (ppParam == SIP_NULL)
#endif
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityVerify))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL) )
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
      if (sip_listGetAt( &(((SipSecurityVerifyHeader *)(pHdr->pHeader))->slParams), dIndex,\
		&element_from_list, pErr) == SipFail)
                return SipFail;

        if (element_from_list == SIP_NULL)
        {
                *pErr = E_NO_EXIST;
                return SipFail;
        }

        temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
        if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
                return SipFail;
#else
        HSS_LOCKEDINCREF(temp_param->dRefCount);
        *ppParam = temp_param;
#endif

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromSecurityVerifyHdr");
        return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInSecurityVerifyHdr
**
** DESCRIPTION: This function sets a param at a specified
**              index in a Security-Verify header
**
**********************************************************/
SipBool sip_setParamAtIndexInSecurityVerifyHdr
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_setParamAtIndexInSecurityVerifyHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr ->dType != SipHdrTypeSecurityVerify))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
	if ( pParam == SIP_NULL )
                temp_param = SIP_NULL;
        else
        {

#ifndef SIP_BY_REFERENCE
                if (sip_initSipParam(&temp_param, pErr) == SipFail)
                        return SipFail;
                if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
                {
                        sip_freeSipParam (temp_param);
                        return SipFail;
                }
#else
                HSS_LOCKEDINCREF(pParam->dRefCount);
                temp_param = pParam;
#endif
        }

        if( sip_listSetAt( &(((SipSecurityVerifyHeader *)(pHdr->pHeader))->slParams),  \
                dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
        {
                if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
                        sip_freeSipParam (temp_param);
#else
                HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
                return SipFail;
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_setParamAtIndexInSecurityVerifyHdr");
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_insertParamAtIndexInSecurityVerifyHdr
**
** DESCRIPTION: This function inserts a param at a specified
**              index in a Security-Verify header
**
**********************************************************/
SipBool sip_insertParamAtIndexInSecurityVerifyHdr
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_insertParamAtIndexInSecurityVerifyHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr ->dType != SipHdrTypeSecurityVerify))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
	if ( pParam == SIP_NULL )
                temp_param = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
                if (sip_initSipParam(&temp_param, pErr) == SipFail)
                        return SipFail;
                if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
                {
                        sip_freeSipParam (temp_param);
                        return SipFail;
                }
#else
                HSS_LOCKEDINCREF(pParam->dRefCount);
                temp_param = pParam;
#endif
        }

        if( sip_listInsertAt( &(((SipSecurityVerifyHeader *)(pHdr->pHeader))->slParams),  \
                dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
        {
                if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
                        sip_freeSipParam (temp_param);
#else
                HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
                return SipFail;
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_insertParamAtIndexInSecurityVerifyHdr");
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInSecurityVerifyHdr
**
** DESCRIPTION: This function deletes a param at a specified
**              index in a Security-Verify header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInSecurityVerifyHdr
        (SipHeader *pHdr, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SIPDEBUGFN("Entering function sip_deleteParamAtIndexInSecurityVerifyHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityVerify))
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }
        if ((pHdr->pHeader == SIP_NULL))
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
#endif
	if( sip_listDeleteAt( &(((SipSecurityVerifyHeader *)(pHdr->pHeader))->slParams),\
			dIndex, pErr) == SipFail)
                return SipFail;

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInSecurityVerifyHdr");
        return SipSuccess;
}

#endif /* end of  #ifdef SIP_SECURITY */


#ifdef SIP_3GPP
/*********************Accessor functions for 3GPP headers(RFC 3455)*********/
/*********************************************************
** FUNCTION:sip_getDispNameFromPCalledPartyIdHdr
**
** DESCRIPTION: This function retrieves the display-pName field
**		from a SIP PCalledPartyId pHeader
**
**********************************************************/
SipBool sip_getDispNameFromPCalledPartyIdHdr
	(SipHeader *pHdr,
	SIP_S8bit **ppDispname,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTempDispname;
	SIPDEBUGFN( "Entering getDispNameFromPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getDispNameFromPCalledPartyIdHdr");
		return SipFail;
	}
	if ( ppDispname == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getDispNameFromPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting getDispNameFromPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting getDispNameFromPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	pTempDispname = ( (SipPCalledPartyIdHeader *) (pHdr->pHeader) )->pDispName;


	if( pTempDispname == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting getDispNameFromPCalledPartyIdHdr");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppDispname = pTempDispname;
#else

	dLength = strlen(pTempDispname );
	*ppDispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppDispname == SIP_NULL )
    {
		*pErr = E_NO_MEM;
	    SIPDEBUGFN( "Exiting getDispNameFromPCalledPartyIdHdr");
		return SipFail;
    }

	strcpy( *ppDispname, pTempDispname );
#endif

	SIPDEBUGFN( "Exiting getDispNameFromPCalledPartyIdHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setDispNameInPCalledPartyIdHdr
**
** DESCRIPTION: This function sets the display-pName field in
**		a SIP PCalledPartyId pHeader
**
**********************************************************/
SipBool sip_setDispNameInPCalledPartyIdHdr
(SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr)

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTempDispname;
#endif
	SipPCalledPartyIdHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setDispNameInPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting setDispNameInPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting setDispNameInPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting setDispNameInPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	pTemp_hdr=(SipPCalledPartyIdHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pDispName !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pDispName),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipPCalledPartyIdHeader *)(pHdr->pHeader))->pDispName = pDispname;
#else
	if( pDispname == SIP_NULL)
		pTempDispname = SIP_NULL;
	else
	{
		dLength = strlen( pDispname );
		pTempDispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTempDispname == SIP_NULL )
        {
    		*pErr = E_NO_MEM;
	        SIPDEBUGFN( "Exiting setDispNameInPCalledPartyIdHdr");
	    	return SipFail;
        }

		strcpy( pTempDispname, pDispname );
	}
	pTemp_hdr->pDispName = pTempDispname;
#endif
	SIPDEBUGFN( "Exiting setDispNameInPCalledPartyIdHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getAddrSpecFromPCalledPartyIdHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field
**		from a SIP PCalledPartyId pHeader
**
**********************************************************/
SipBool sip_getAddrSpecFromPCalledPartyIdHdr
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr,
	SipAddrSpec **ppAddrspec,
	SipError *pErr)
#else
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
#endif
{
	SipAddrSpec *pFrom;
	SIPDEBUGFN( "Entering getAddrSpecFromPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPCalledPartyIdHdr");
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	if ( ppAddrspec == SIP_NULL)
#else
	if ( pAddrspec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	pFrom=((SipPCalledPartyIdHeader *)(pHdr->pHeader))->pAddrSpec;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPCalledPartyIdHdr");
		return SipFail;

	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppAddrspec = pFrom;
#else
	if(__sip_cloneAddrSpec(pAddrspec,pFrom,pErr)==SipFail)
	{
		if(pAddrspec->dType==SipAddrReqUri)
		{
			sip_freeString(pAddrspec->u.pUri);
		}
		else if((pAddrspec->dType==SipAddrSipUri) \
				|| (pAddrspec->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl(pAddrspec->u.pSipUrl);
		}
		pAddrspec->dType=SipAddrAny;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getAddrSpecFromPCalledPartyIdHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setAddrSpecInPCalledPartyIdHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a
**		SIP PCalledPartyId Header
**
**********************************************************/
SipBool sip_setAddrSpecInPCalledPartyIdHdr
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTo;
#endif
	SIPDEBUGFN( "Entering setAddrSpecInPCalledPartyIdHdr ");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting setAddrSpecInPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting setAddrSpecInPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting setAddrSpecInPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	sip_freeSipAddrSpec(((SipPCalledPartyIdHeader *)(pHdr->pHeader))->pAddrSpec );
	if (pAddrspec==SIP_NULL)
	{
		((SipPCalledPartyIdHeader *)(pHdr->pHeader))->pAddrSpec=SIP_NULL; 		
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrspec->dRefCount);
		((SipPCalledPartyIdHeader *)(pHdr->pHeader))->pAddrSpec = pAddrspec;
#else
		if(sip_initSipAddrSpec(&pTo,SipAddrAny,pErr)==SipFail)
		{
	        SIPDEBUGFN( "Exiting setAddrSpecInPCalledPartyIdHdr");
			return SipFail;
		}
		if(__sip_cloneAddrSpec(pTo,pAddrspec,pErr)==SipFail)
		{
			sip_freeSipAddrSpec(pTo);
	        SIPDEBUGFN( "Exiting setAddrSpecInPCalledPartyIdHdr");
			return SipFail;
		}
		((SipPCalledPartyIdHeader *)(pHdr->pHeader))->pAddrSpec = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setAddrSpecInPCalledPartyIdHdr");
	return SipSuccess;
}
/*********************************************************
** FUNCTION: sip_getParamCountFromPCalledPartyIdHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**		from a SIP PCalledPartyId Header
**
**********************************************************/
SipBool sip_getParamCountFromPCalledPartyIdHdr
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)

{
	SIPDEBUGFN("Entering function sip_getParamCountFromPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPCalledPartyIdHdr");
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipPCalledPartyIdHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPCalledPartyIdHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromPCalledPartyIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamAtIndexFromPCalledPartyIdHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**		index in a PCalledPartyId header
**
**********************************************************/
SipBool sip_getParamAtIndexFromPCalledPartyIdHdr
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPCalledPartyIdHdr");
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipPCalledPartyIdHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPCalledPartyIdHdr");
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
    {
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPCalledPartyIdHdr");
		return SipFail;
    }
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPCalledPartyIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInPCalledPartyIdHdr
**
** DESCRIPTION: This function sets a param at a specified
**		index in a PCalledPartyId header
**
**********************************************************/
SipBool sip_setParamAtIndexInPCalledPartyIdHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
	        SIPDEBUGFN("Exiting function sip_setParamAtIndexInPCalledPartyIdHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipPCalledPartyIdHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInPCalledPartyIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_insertParamAtIndexInPCalledPartyIdHdr
**
** DESCRIPTION: This function inserts a param at a specified
**		index in a PCalledPartyId header
**
**********************************************************/
SipBool sip_insertParamAtIndexInPCalledPartyIdHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
	        SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPCalledPartyIdHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipPCalledPartyIdHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPCalledPartyIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInPCalledPartyIdHdr
**
** DESCRIPTION: This function deletes a param at a specified
**		index in a PCalledPartyId header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInPCalledPartyIdHdr
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPCalledPartyIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePCalledPartyId))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipPCalledPartyIdHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
    {
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPCalledPartyIdHdr");
		return SipFail;
    }

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPCalledPartyIdHdr");
	return SipSuccess;
}

/*******PVisitedNetworkId header *************************/
/*********************************************************
** FUNCTION:sip_getVNetworkSpecFromPVisitedNetworkIdHdr
**
** DESCRIPTION: This function retrieves the pVNetworkSpec field
**		from a SIP PVisitedNetworkId Header
**
**********************************************************/
SipBool sip_getVNetworkSpecFromPVisitedNetworkIdHdr
	(SipHeader *pHdr,
	SIP_S8bit **ppVNetworkSpec,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_VNetworkSpec;
	SIPDEBUGFN( "Entering getVNetworkSpecFromPVisitedNetworkIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getVNetworkSpecFromPVisitedNetworkIdHdr");
		return SipFail;
	}
	if ( ppVNetworkSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getVNetworkSpecFromPVisitedNetworkIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePVisitedNetworkId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting getVNetworkSpecFromPVisitedNetworkIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting getVNetworkSpecFromPVisitedNetworkIdHdr");
		return SipFail;
	}
#endif
	pTemp_VNetworkSpec = ( (SipPVisitedNetworkIdHeader *) (pHdr->pHeader) )->pVNetworkSpec;


	if( pTemp_VNetworkSpec == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting getVNetworkSpecFromPVisitedNetworkIdHdr");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppVNetworkSpec = pTemp_VNetworkSpec;
#else

	dLength = strlen(pTemp_VNetworkSpec );
	*ppVNetworkSpec = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppVNetworkSpec == SIP_NULL )
    {
        *pErr = E_NO_MEM;
	    SIPDEBUGFN( "Exiting getVNetworkSpecFromPVisitedNetworkIdHdr");
		return SipFail;
    }

	strcpy( *ppVNetworkSpec, pTemp_VNetworkSpec );
#endif

	SIPDEBUGFN( "Exiting getVNetworkSpecFromPVisitedNetworkIdHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setVNetworkSpecInPVisitedNetworkIdHdr
**
** DESCRIPTION: This function sets the display-pName field in
**		a SIP PVisitedNetworkId pHeader
**
**********************************************************/
SipBool sip_setVNetworkSpecInPVisitedNetworkIdHdr
(SipHeader *pHdr,SIP_S8bit *pVNetworkSpec, SipError *pErr)

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_VNetworkSpec;
#endif
	SipPVisitedNetworkIdHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setVNetworkSpecInPVisitedNetworkIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting setVNetworkSpecInPVisitedNetworkIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePVisitedNetworkId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting setVNetworkSpecInPVisitedNetworkIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting setVNetworkSpecInPVisitedNetworkIdHdr");
		return SipFail;
	}
#endif
	pTemp_hdr=(SipPVisitedNetworkIdHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pVNetworkSpec !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pVNetworkSpec),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipPVisitedNetworkIdHeader *)(pHdr->pHeader))->pVNetworkSpec = pVNetworkSpec;
#else
	if( pVNetworkSpec == SIP_NULL)
		pTemp_VNetworkSpec = SIP_NULL;
	else
	{
		dLength = strlen( pVNetworkSpec );
		pTemp_VNetworkSpec = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_VNetworkSpec == SIP_NULL )
        {
	        *pErr = E_NO_MEM;
            SIPDEBUGFN( "Exiting setVNetworkSpecInPVisitedNetworkIdHdr");
			return SipFail;
        }

		strcpy( pTemp_VNetworkSpec, pVNetworkSpec );
	}
	pTemp_hdr->pVNetworkSpec = pTemp_VNetworkSpec;
#endif
	SIPDEBUGFN( "Exiting setVNetworkSpecInPVisitedNetworkIdHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_getParamCountFromPVisitedNetworkIdHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**		from a SIP PVisitedNetworkId Header
**
**********************************************************/
SipBool sip_getParamCountFromPVisitedNetworkIdHdr
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_getParamCountFromPVisitedNetworkIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPVisitedNetworkIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePVisitedNetworkId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPVisitedNetworkIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPVisitedNetworkIdHdr");
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPVisitedNetworkIdHdr");
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipPVisitedNetworkIdHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPVisitedNetworkIdHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromPVisitedNetworkIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamAtIndexFromPVisitedNetworkIdHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**		index in a PVisitedNetworkId header
**
**********************************************************/
SipBool sip_getParamAtIndexFromPVisitedNetworkIdHdr
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPVisitedNetworkIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePVisitedNetworkId))
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipPVisitedNetworkIdHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPVisitedNetworkIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInPVisitedNetworkIdHdr
**
** DESCRIPTION: This function sets a param at a specified
**		index in a PVisitedNetworkId header
**
**********************************************************/
SipBool sip_setParamAtIndexInPVisitedNetworkIdHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPVisitedNetworkIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePVisitedNetworkId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
	        SIPDEBUGFN("Exiting function sip_setParamAtIndexInPVisitedNetworkIdHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipPVisitedNetworkIdHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInPVisitedNetworkIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_insertParamAtIndexInPVisitedNetworkIdHdr
**
** DESCRIPTION: This function inserts a param at a specified
**		index in a PVisitedNetworkId header
**
**********************************************************/
SipBool sip_insertParamAtIndexInPVisitedNetworkIdHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPVisitedNetworkIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePVisitedNetworkId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
	        SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPVisitedNetworkIdHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipPVisitedNetworkIdHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPVisitedNetworkIdHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInPVisitedNetworkIdHdr
**
** DESCRIPTION: This function deletes a param at a specified
**		index in a PVisitedNetworkId header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInPVisitedNetworkIdHdr
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPVisitedNetworkIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePVisitedNetworkId))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPVisitedNetworkIdHdr");
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipPVisitedNetworkIdHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPVisitedNetworkIdHdr");
	return SipSuccess;
}

#endif /*#ifdef SIP_3GPP */

#ifdef SIP_CONGEST
/**********Accessor functions for CONGEST headers(Resource-Priority)*********/
/*********************************************************
** FUNCTION:sip_getNamespaceFromRsrcPriorityHdr
**
** DESCRIPTION: This function retrieves the Namespace field
**		from a SIP RsrcPriority Header
**
**********************************************************/
SipBool sip_getNamespaceFromRsrcPriorityHdr 
	(SipHeader *pHdr,
	SIP_S8bit **ppNamespace,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_namespace;
	SIPDEBUGFN( "Entering  sip_getNamespaceFromRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromRsrcPriorityHdr");
		return SipFail;
	}
	if ( ppNamespace == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRsrcPriority))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_namespace = ( (SipRsrcPriorityHeader *) (pHdr->pHeader) )->pNamespace;


	if( pTemp_namespace == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromRsrcPriorityHdr");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppNamespace = pTemp_namespace;
#else

	dLength = strlen(pTemp_namespace );
	*ppNamespace = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppNamespace == SIP_NULL )
    {
	    *pErr = E_NO_MEM;
        SIPDEBUGFN( "Exiting  sip_getNamespaceFromRsrcPriorityHdr");
		return SipFail;
    }

	strcpy( *ppNamespace, pTemp_namespace );
#endif

	SIPDEBUGFN( "Exiting sip_getNamespaceFromRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getPriorityFromRsrcPriorityHdr
**
** DESCRIPTION: This function retrieves the Priority field
**		from a SIP RsrcPriority Header
**
**********************************************************/
SipBool sip_getPriorityFromRsrcPriorityHdr 
	(SipHeader *pHdr,
	SIP_S8bit **ppPriority,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_priority;
	SIPDEBUGFN( "Entering  sip_getPriorityFromRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN( "Exiting  sip_getPriorityFromRsrcPriorityHdr");
		return SipFail;
	}
	if ( ppPriority == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN( "Exiting  sip_getPriorityFromRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRsrcPriority))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN( "Exiting  sip_getPriorityFromRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN( "Exiting  sip_getPriorityFromRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_priority = ( (SipRsrcPriorityHeader *) (pHdr->pHeader) )->pPriority;


	if( pTemp_priority == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
        SIPDEBUGFN( "Exiting  sip_getPriorityFromRsrcPriorityHdr");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppPriority = pTemp_priority;
#else

	dLength = strlen(pTemp_priority );
	*ppPriority = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppPriority == SIP_NULL )
    {
        *pErr = E_NO_MEM;
        SIPDEBUGFN( "Exiting  sip_getPriorityFromRsrcPriorityHdr");
		return SipFail;
    }

	strcpy( *ppPriority, pTemp_priority);
#endif

	SIPDEBUGFN( "Exiting sip_getPriorityFromRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setNamespaceInRsrcPriorityHdr
**
** DESCRIPTION: This function sets the Namespace field in
**		a SIP RsrcPriorityHdr Header
**
**********************************************************/
SipBool sip_setNamespaceInRsrcPriorityHdr
(SipHeader *pHdr,SIP_S8bit *pNamespace, SipError *pErr)

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_namespace;
#endif
	SipRsrcPriorityHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setNamespaceInRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN( "Exiting sip_setNamespaceInRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRsrcPriority))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN( "Exiting sip_setNamespaceInRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN( "Exiting sip_setNamespaceInRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_hdr=(SipRsrcPriorityHeader*)(pHdr->pHeader);
	if(pTemp_hdr->pNamespace !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pNamespace),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipRsrcPriorityHeader *)(pHdr->pHeader))->pNamespace = pNamespace;
#else
	if( pNamespace == SIP_NULL)
		pTemp_namespace= SIP_NULL;
	else
	{
		dLength = strlen( pNamespace);
		pTemp_namespace= ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_namespace== SIP_NULL )
        {
	        *pErr = E_NO_MEM;
            SIPDEBUGFN( "Exiting sip_setNamespaceInRsrcPriorityHdr");
			return SipFail;
        }

		strcpy( pTemp_namespace, pNamespace);
	}
	pTemp_hdr->pNamespace= pTemp_namespace;
#endif
	SIPDEBUGFN( "Exiting sip_setNamespaceInRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setPriorityInRsrcPriorityHdr
**
** DESCRIPTION: This function sets the Priority field in
**		a SIP RsrcPriorityHdr Header
**
**********************************************************/
SipBool sip_setPriorityInRsrcPriorityHdr
(SipHeader *pHdr,SIP_S8bit *pPriority, SipError *pErr)

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_priority;
#endif
	SipRsrcPriorityHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setPriorityInRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN( "Exiting sip_setPriorityInRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRsrcPriority))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN( "Exiting sip_setPriorityInRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN( "Exiting sip_setPriorityInRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_hdr=(SipRsrcPriorityHeader*)(pHdr->pHeader);
	if(pTemp_hdr->pPriority != SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pPriority),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipRsrcPriorityHeader *)(pHdr->pHeader))->pPriority= pPriority;
#else
	if( pPriority== SIP_NULL)
		pTemp_priority= SIP_NULL;
	else
	{
		dLength = strlen(pPriority);
		pTemp_priority= ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_priority == SIP_NULL )
        {
	        *pErr = E_NO_MEM;
            SIPDEBUGFN( "Exiting sip_setPriorityInRsrcPriorityHdr");
			return SipFail;
        }

		strcpy( pTemp_priority, pPriority);
	}
	pTemp_hdr->pPriority= pTemp_priority;
#endif
	SIPDEBUGFN( "Exiting sip_setPriorityInRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif
