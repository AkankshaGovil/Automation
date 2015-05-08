
 /******************************************************************************
 ** FUNCTION:
 **	 	This file has all accessor API source of Instant Messaging and
 **     Presence Related Structures
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		impp.c
 **
 ** DESCRIPTION:
 **
 **
 ** DATE		NAME				REFERENCE	REASON
 ** ----		----				--------	------
 ** 16/4/2001	Subhash Nayak U.	Original
 **
 **	Copyright 2001, Hughes Software Systems, Ltd.
 ******************************************************************************/

#include "impp.h"
#include "sipinit.h"
#include "sipfree.h"
#include "sipclone.h"
#include "sipstatistics.h"
#include "sipformmessage.h"
#include "sipdecodeintrnl.h"
#include "sipparserinc.h"
#include "imurldecodeintrnl.h"
#include "imppinternal.h"
#include "sipstruct.h"
#include "request.h"


/*****************************************************************
**
** FUNCTION:  sip_impp_getSubStateFromSubscriptionStateHdr
**
** DESCRIPTION: This function retrieves the state field from a SIP
**		Subscription Expires Header
**
***************************************************************/
SipBool sip_impp_getSubStateFromSubscriptionStateHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppSubState, SipError *pErr)
#else
	(pHdr, ppSubState, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppSubState;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *pTempSubState;
	SIPDEBUGFN("Entering function" \
	"sip_impp_getSubStateFromSubscriptionStateHdr");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pHdr == SIP_NULL) || (ppSubState  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->pHeader == SIP_NULL) )
 	{
 	 	*pErr = E_INV_HEADER;
 	 	return SipFail;
 	}

	if ( pHdr->dType != SipHdrTypeSubscriptionState)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	pTempSubState = ( (SipSubscriptionStateHeader*) (pHdr->pHeader) )\
					->pSubState;

	if( pTempSubState == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppSubState = pTempSubState;
#else
	dLength = strlen(pTempSubState);
	*ppSubState = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppSubState == SIP_NULL )
		return SipFail;

	strcpy( *ppSubState , pTempSubState);
#endif

	SIPDEBUGFN ( "Exiting sip_impp_getSubStateFromSubscriptionStateHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSubStateInSubscriptionStateHdr
**
** DESCRIPTION: This function sets the State field in a SIP SubscriptionState
**		pHeader
**
***************************************************************/
SipBool sip_impp_setSubStateInSubscriptionStateHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pSubState, SipError *pErr)
#else
	(pHdr, pSubState, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pSubState;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit 		dLength;
#endif
	SIP_S8bit 		*pTempSubState;
	SipSubscriptionStateHeader *pTempSubExpiresHdr;

	SIPDEBUGFN("Entering function"\
		"sip_setSubStateInSubscriptionStateHdr");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( pHdr->dType != SipHdrTypeSubscriptionState)
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
	if( pSubState == SIP_NULL)
		pTempSubState = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pTempSubState = pSubState;
#else
		dLength = strlen( pSubState);
		pTempSubState = ( SIP_S8bit * )fast_memget\
						(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTempSubState == SIP_NULL )
			return SipFail;
		strcpy(pTempSubState,pSubState);
#endif
	}

	pTempSubExpiresHdr = (SipSubscriptionStateHeader *) \
						(pHdr->pHeader);

	if ( pTempSubExpiresHdr->pSubState != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, \
				pTempSubExpiresHdr->pSubState, pErr)== SipFail )
			return SipFail;
	}

	pTempSubExpiresHdr->pSubState = pTempSubState;


	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function" \
		"sip_setSubStateInSubscriptionStateHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromSubscriptionStateHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP SubscriptionState pHeader
**
***************************************************************/
SipBool sip_impp_getParamCountFromSubscriptionStateHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *dCount, SipError *pErr)
#else
	(pHdr, dCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *dCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromSubscriptionStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pHdr->dType != SipHdrTypeSubscriptionState )
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
	if (pHdr->pHeader == SIP_NULL)
 	{
 	 	*pErr = E_INV_HEADER;
 	 	return SipFail;
 	}

	if (dCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipSubscriptionStateHeader *)(pHdr->pHeader))->\
		slParams),\
		dCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromSubscriptionStateHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromSubscriptionStateHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP SubscriptionState pHeader
**
***************************************************************/
SipBool sip_impp_getParamAtIndexFromSubscriptionStateHdr
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
	SipParam *pTemp_param=SIP_NULL;
	SIPDEBUGFN("Entering function \
	sip_getParamAtIndexFromSubscriptionStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pHdr->dType != SipHdrTypeSubscriptionState)
 	{
 	 	*pErr = E_INV_TYPE;
 	 	return SipFail;
 	}
	if (pHdr->pHeader == SIP_NULL)
 	{
 	 	*pErr = E_INV_HEADER;
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
#endif

	if (sip_listGetAt( &(((SipSubscriptionStateHeader *)(pHdr->pHeader))->\
		slParams), dIndex, &element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTemp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, pTemp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTemp_param->dRefCount);
	*ppParam = pTemp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function \
	sip_getParamAtIndexFromSubscriptionStateHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInSubscriptionStateHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp SubscriptionState pHeader
**
***************************************************************/
SipBool sip_impp_insertParamAtIndexInSubscriptionStateHdr
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
	SipParam *pTemp_param=SIP_NULL;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInSubscriptionStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pHdr->dType != SipHdrTypeSubscriptionState )
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
	if ( pParam == SIP_NULL )
		pTemp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipSubscriptionStateHeader *)(pHdr->pHeader))->\
		slParams),  dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInSubscriptionStateHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInSubscriptionStateHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP SubscriptionState pHeader
**
***************************************************************/
SipBool sip_impp_deleteParamAtIndexInSubscriptionStateHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function \
	sip_deleteParamAtIndexInSubscriptionStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pHdr->dType != SipHdrTypeSubscriptionState )
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
	if( sip_listDeleteAt( &(((SipSubscriptionStateHeader *)(pHdr->pHeader))->\
		slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function \
	sip_deleteParamAtIndexInSubscriptionStateHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInSubscriptionStateHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP encryption pHeader
**
***************************************************************/
SipBool sip_impp_setParamAtIndexInSubscriptionStateHdr
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
	SipParam *pTemp_param=SIP_NULL;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInSubscriptionStateHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pHdr->dType != SipHdrTypeSubscriptionState )
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
	if ( pParam == SIP_NULL )
		pTemp_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(pParam->dType, pErr) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipSubscriptionStateHeader *)(pHdr->pHeader))->\
		slParams), dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_seParamAtIndexInSubscriptionStateHdr");
	return SipSuccess;
}


/*********************************************************************
** FUNCTION: sip_impp_getEventTypeFromEventHdr

**********************************************************************

** DESCRIPTION:
	Gets the pEventType field from Event Header.

*********************************************************************/
SipBool sip_impp_getEventTypeFromEventHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,	/*Event pHeader */
	  SIP_S8bit ** dType,
	  SipError * err)
#else
	( hdr, dType, err )
	  SipHeader * hdr;      /*Event pHeader */
	  SIP_S8bit ** dType;
	  SipError * err;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_event_type;

	SIPDEBUGFN ( "Entering getEventTypeFromEventHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (( hdr == SIP_NULL) || (dType == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( hdr->dType != SipHdrTypeEvent )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_event_type = ( (SipEventHeader *) (hdr->pHeader) )->pEventType;

	if( temp_event_type == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*dType=temp_event_type;
#else
	dLength = strlen(temp_event_type );
	*dType = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *dType == SIP_NULL )
		return SipFail;

	strcpy( *dType, temp_event_type );
#endif

	SIPDEBUGFN ( "Exiting getEventTypeFromEventHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION: sip_impp_setEventTypeInEventHdr

**********************************************************************

** DESCRIPTION:
	Sets the pEventType field in Event Header.

*********************************************************************/
SipBool sip_impp_setEventTypeInEventHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,	/* Event pHeader */
	  SIP_S8bit * dType,
	  SipError * err)
#else
	( hdr, dType, err )
	  SipHeader * hdr; 	/* Event pHeader */
	  SIP_S8bit * dType;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit 		dLength;
#endif
	SIP_S8bit 		*temp_event_type;
	SipEventHeader 	*temp_event_hdr;

	SIPDEBUGFN ( "Entering setEventTypeInEventHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( hdr->dType != SipHdrTypeEvent )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( hdr->pHeader == SIP_NULL )
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( dType == SIP_NULL)
		temp_event_type = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_event_type=dType;
#else
		dLength = strlen( dType );
		temp_event_type = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_event_type == SIP_NULL )
			return SipFail;
		strcpy( temp_event_type, dType );
#endif
	}

	temp_event_hdr = (SipEventHeader *) (hdr->pHeader);

	if ( temp_event_hdr->pEventType != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_event_hdr->pEventType, err)== SipFail )
			return SipFail;
	}

	temp_event_hdr->pEventType = temp_event_type;

	SIPDEBUGFN ( "Exiting setEventTypeInEventHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:  sip_impp_getParamCountFromEventHdr
**********************************************************************
**
** DESCRIPTION:
**	Gets the number of SipParam structures present in the EventHdr.
**
*********************************************************************/

SipBool sip_impp_getParamCountFromEventHdr
#ifdef ANSI_PROTO
	( SipHeader	*pHdr,
	  SIP_U32bit	*pIndex,
	  SipError	*pErr  )
#else
	( pHdr,pIndex,pErr)
	  SipHeader 	*pHdr;
	  SIP_U32bit 	*pIndex;
	  SipError 	*pErr;
#endif
{
	SipEventHeader *pEventHdr;

	SIPDEBUGFN ( "Entering getParamCountFromEventHdr");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if ( (pHdr == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if(pHdr->dType != SipHdrTypeEvent)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if(pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pEventHdr = (SipEventHeader*) (pHdr->pHeader);

	if (sip_listSizeOf(&(pEventHdr->slParams), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting getParamCountFromEventHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_impp_getParamAtIndexFromEventHdr
**********************************************************************
**
** DESCRIPTION:
**	Gets the SipParam structure at a specified index
** ( starting from 0 ) from the EventHdr.
**
*********************************************************************/
SipBool sip_impp_getParamAtIndexFromEventHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader 	*pHdr, SipParam **pParam, SIP_U32bit dIndex, SipError *pErr )
#else
	( SipHeader 	*pHdr, SipParam *pParam, SIP_U32bit	dIndex, SipError *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pHdr,pParam,dIndex,pErr)
	  SipHeader 	*pHdr;
	  SipParam **pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#else
	( pHdr,pParam,dIndex,pErr)
	  SipHeader 	*pHdr;
	  SipParam *pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
#endif
{
	SIP_Pvoid 	pElementFromList;
	SipEventHeader * pEventHdr;

	SIPDEBUGFN ( "Entering getParamAtIndexFromEventHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pHdr == SIP_NULL) || (pParam == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( pHdr->dType != SipHdrTypeEvent)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pEventHdr = (SipEventHeader*)(pHdr->pHeader);

	if( sip_listGetAt(&(pEventHdr->slParams), dIndex, &pElementFromList, pErr) == SipFail)
		return SipFail;

	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(((SipParam *)(pElementFromList))->dRefCount);
	*pParam=(SipParam *)pElementFromList;
#else
	if ( __sip_cloneSipParam(pParam,(SipParam *)pElementFromList,pErr) == SipFail)
		return SipFail;
#endif

	SIPDEBUGFN ( "Exiting getParamAtIndexFromEventHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_impp_setParamAtIndexInEventHdr
**********************************************************************
**
** DESCRIPTION:
**	Sets the SipParam structure at a specified dIndex
** ( starting from 0 ) in the EventHdr.
**
*********************************************************************/
SipBool sip_impp_setParamAtIndexInEventHdr
#ifdef ANSI_PROTO
	( SipHeader 	*pHdr,
	  SipParam	*pParam,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pHdr,pParam,dIndex,pErr)
	  SipHeader 	*pHdr;
	  SipParam	*pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElementInList;

	SIPDEBUGFN ( "Entering setParamAtIndexInEventHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (( pHdr->dType != SipHdrTypeEvent))
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
		pElementInList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementInList=pParam;
#else
		if (sip_initSipParam(&pElementInList, pErr) == SipFail)
			return SipFail;
		if ( __sip_cloneSipParam(pElementInList, pParam,pErr) == SipFail)
		{
			sip_freeSipParam(pElementInList );
			return SipFail;
		}
#endif
	}

	if( sip_listSetAt(&(((SipEventHeader*)(pHdr->pHeader))->slParams),\
		 dIndex, (SIP_Pvoid) pElementInList, pErr) == SipFail)
	{
		if ( pElementInList != SIP_NULL )
			 sip_freeSipParam(pElementInList );
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pParam->dRefCount);
#endif
	SIPDEBUGFN ( "Exiting setParamAtIndexInEventHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_impp_insertParamAtIndexInEventHdr
**********************************************************************
**
** DESCRIPTION:
**	Inserts the SipParam structure at a specified dIndex
** ( starting from 0 ) in the EventHdr.
**
*********************************************************************/
SipBool sip_impp_insertParamAtIndexInEventHdr
#ifdef ANSI_PROTO
	( SipHeader 	*pHdr,
	  SipParam	*pParam,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pHdr,pParam,dIndex,pErr)
	  SipHeader 	*pHdr;
	  SipParam	*pParam;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElementInList;
	SipEventHeader *pEventHdr;

	SIPDEBUGFN ( "Entering InsertParamAtIndexInEventHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( pHdr->dType != SipHdrTypeEvent)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* copying the SipParam structure */
	if ( pParam == SIP_NULL )
		pElementInList = SIP_NULL;
	else
	{
		/* call init SipParam */
#ifdef SIP_BY_REFERENCE
		pElementInList=pParam;
#else
		if ( sip_initSipParam(&pElementInList, pErr) == SipFail)
			return SipFail;

		if ( __sip_cloneSipParam(pElementInList, pParam,pErr) == SipFail)
		{
			sip_freeSipParam(pElementInList);
			return SipFail;
		}
#endif
	}

	pEventHdr = (SipEventHeader *) (pHdr->pHeader);;
	if( sip_listInsertAt(&(pEventHdr->slParams), dIndex, pElementInList, pErr) == SipFail)
	{
		if ( pElementInList != SIP_NULL )
			sip_freeSipParam(pElementInList);
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pParam->dRefCount);
#endif

	SIPDEBUGFN ( "Exiting InsertParamAtIndexInEventHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_impp_deleteParamAtIndexInEventHdr
**********************************************************************
**
** DESCRIPTION:
**	Deletes a SipParam structure at a specified dIndex
** (starting from 0 ) in the EventHdr.
**
*********************************************************************/
SipBool sip_impp_deleteParamAtIndexInEventHdr
#ifdef ANSI_PROTO
	( SipHeader 	*pHdr,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pHdr,dIndex,pErr)
	  SipHeader 	*pHdr;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SipEventHeader * pEventHdr;

	SIPDEBUGFN ( "Entering deleteParamAtIndexInEventHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( pHdr->dType != SipHdrTypeEvent)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pHdr->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pEventHdr = (SipEventHeader *)(pHdr->pHeader);

	if( sip_listDeleteAt(&(pEventHdr->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exiting deleteParamAtIndexInEventHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_impp_getEventTypeFromAllowEventsHdr
**********************************************************************
**
** DESCRIPTION: Gets the pEventType field pValue from allow pHeader.
**
*********************************************************************/
SipBool sip_impp_getEventTypeFromAllowEventsHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit **pEventType, SipError *err)
#else
	( hdr, pEventType, err )
	  SipHeader 	*hdr;
	  SIP_S8bit 	**pEventType;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit 	dLength;
#endif
	SIP_S8bit 	*temp_eventtype;

	SIPDEBUGFN ( "Entering getEventTypeFromAllowEventsHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( (hdr->dType != SipHdrTypeAllowEvents))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( (hdr->pHeader == SIP_NULL) )
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

	if(pEventType == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_eventtype = ( (SipAllowEventsHeader *) ( hdr->pHeader) )->pEventType;
	if ( temp_eventtype == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*pEventType = temp_eventtype;
#else
	dLength = strlen( temp_eventtype);

	*pEventType = ( SIP_S8bit * ) fast_memget(ACCESSOR_MEM_ID, dLength+1, err);
	if ( *pEventType == SIP_NULL )
		return SipFail;

	strcpy( *pEventType, temp_eventtype);
#endif

	SIPDEBUGFN ( "Exiting getEventTypeFromAllowEventsHdr");
	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:   sip_impp_setEventTypeInAllowEventsHdr
**********************************************************************
**
** DESCRIPTION: Sets the pEventType field in allow pHeader.
**
*********************************************************************/
SipBool sip_impp_setEventTypeInAllowEventsHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit *pEventType, SipError *err)
#else
	( hdr, pEventType, err )
	  SipHeader 	*hdr;
	  SIP_S8bit 	*pEventType;
	  SipError 	*err;
#endif
{
	SipAllowEventsHeader *temp_allowevents_hdr;
	SIP_S8bit 	*temp_eventtype;

	SIPDEBUGFN ( "Entering setEventTypeInAllowEventsHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( (hdr->dType != SipHdrTypeAllowEvents))
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
	if ( pEventType == SIP_NULL )
		temp_eventtype = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_eventtype = pEventType;
#else
		temp_eventtype = ( SIP_S8bit * ) fast_memget(ACCESSOR_MEM_ID, strlen(pEventType)+1, err);
		if ( temp_eventtype == SIP_NULL )
			return SipFail;

		strcpy( temp_eventtype, pEventType );
#endif
	}

	temp_allowevents_hdr = ( SipAllowEventsHeader *) (hdr->pHeader);

	if( temp_allowevents_hdr->pEventType != SIP_NULL )
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(temp_allowevents_hdr->pEventType)), err) == SipFail)
			return SipFail;

	temp_allowevents_hdr->pEventType = temp_eventtype;

	SIPDEBUGFN ( "Exiting setEventTypeInAllowEventsHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}



/* IM-URL Related APIs */

/***********************************************************************
** Function: sip_isImUrl
** Description: Checks if the Addrspec has a im-url
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_isImUrl
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(pAddrSpec,pErr)
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
{
	en_AddrType pAddrtype;
	SIP_S8bit *pTempUri=SIP_NULL;
	SIP_U32bit i=0;

	SIPDEBUGFN("Entering sip_isImUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( sip_getAddrTypeFromAddrSpec(pAddrSpec,&pAddrtype,pErr) != \
		SipSuccess )
	{
		SIPDEBUGFN("Exiting sip_isImUrl\n");
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( pAddrtype == SipAddrReqUri )
	{

		pTempUri = pAddrSpec->u.pUri;

		i=0;
		if (( pTempUri[i] == 'I' || pTempUri[i] == 'i') &&
			( pTempUri[i+1] == 'M' || pTempUri[i+1] == 'm'))
		{
			for (i=2;((pTempUri[i] == ' ') || (pTempUri[i] == '\t'))&& \
					(pTempUri[i] != '\0');i++);

			if (pTempUri[i] == ':')
			{
				*pErr = E_NO_ERROR;
				SIPDEBUGFN("Exiting sip_isImUrl\n");
				return SipSuccess;
			}
		}
	}
	SIPDEBUGFN("Exiting sip_isImUrl\n");
	*pErr = E_NO_EXIST;
	return SipFail;
}

/***********************************************************************
** Function: sip_getImUrlFromAddrSpec
** Description: gets the ImUrl from the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec
**		pImUrl (OUT)	- retrieved ImUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
SipBool sip_getImUrlFromAddrSpec (
	SipAddrSpec *pAddrSpec, ImUrl **ppImUrl, SipError *pErr)
#else
SipBool sip_getImUrlFromAddrSpec (
	SipAddrSpec *pAddrSpec, ImUrl *pImUrl, SipError *pErr)
#endif
#else  /* ANSI_PROTO */
#ifdef SIP_BY_REFERENCE
SipBool sip_getImUrlFromAddrSpec (
	pAddrSpec,ppImUrl, pErr)
	SipAddrSpec *pAddrSpec;
	ImUrl **ppImUrl;
	SipError *pErr;
#else
SipBool sip_getImUrlFromAddrSpec (
	pAddrSpec, pImUrl, pErr)
	SipAddrSpec *pAddrSpec;
	ImUrl *pImUrl;
	SipError *pErr;
#endif
#endif /*ANSI_PROTO */
{
	SIP_S8bit *pParserBuffer=SIP_NULL;
	SIP_S32bit len=0;
	SipImParserParam *pImParserParam=SIP_NULL;

	INC_API_COUNT

	SIPDEBUGFN ("Entering sip_getImUrlFromAddrSpec\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pImUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#else
	if (ppImUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (pAddrSpec->u.pUri == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	if ( sip_isImUrl(pAddrSpec, pErr) == SipFail)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#endif
	len = strlen(pAddrSpec->u.pUri);
	pImParserParam = (SipImParserParam*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipImParserParam), pErr);
	if(pImParserParam==SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pImParserParam->pError = (SipError*)fast_memget( \
		ACCESSOR_MEM_ID,sizeof(SipError),pErr);
	if(pImParserParam->pError==SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pImParserParam->pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),pErr);
    if(pImParserParam->pGCList==SIP_NULL)
    {
         *pErr = E_NO_MEM;
          return SipFail;
    }
    sip_listInit((pImParserParam->pGCList),&sip_freeVoid,pErr);
    if(*pErr !=E_NO_ERROR)
	{
		fast_memfree(ACCESSOR_MEM_ID,pImParserParam->pGCList,SIP_NULL);
		*pErr = E_NO_MEM;
        return SipFail;
	}

	strcpy(pParserBuffer,pAddrSpec->u.pUri );
	pParserBuffer[len+1]='\0';
	*(pImParserParam->pError) = E_NO_ERROR;
	if (sip_lex_Im_scan_buffer(pParserBuffer, len+2) != 0)
	{
		INC_ERROR_MEM
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
		if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
        {
              return SipFail;
        }
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pGCList),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	sip_lex_Im_reset_state();
	pImParserParam->pImUrl = SIP_NULL;
	glbSipParserImparse((void *)pImParserParam);
	sip_lex_Im_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);

	if (*(pImParserParam->pError) != E_NO_ERROR)
	{
#ifdef SIP_STATISTICS
		INC_SECOND_ERROR_PROTOCOL
#endif

		*pErr = E_PARSER_ERROR;
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
		if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
        {
              return SipFail;
        }
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pGCList),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
		return SipFail;
	}
#ifdef SIP_STATISTICS
	INC_SECOND_API_REQ_PARSED
#endif

#ifdef SIP_BY_REFERENCE
	*ppImUrl = pImParserParam->pImUrl;
#else
	if (sip_cloneImUrl(pImUrl, pImParserParam->pImUrl, pErr) == SipFail)
	{
		sip_freeImUrl(pImParserParam->pImUrl);
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
		if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
        {
              return SipFail;
        }
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pGCList),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
		return SipFail;
	}
	sip_freeImUrl(pImParserParam->pImUrl);
#endif

	fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
	if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
    {
         return SipFail;
    }

	fast_memfree(ACCESSOR_MEM_ID,pImParserParam->pGCList,SIP_NULL);
	fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
	SIPDEBUGFN ("Exiting sip_getImUrlFromAddrSpec\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setImUrlInAddrSpec
** Description: sets the ImUrl in the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec
**		pImUrl (OUT)	- ImUrl to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setImUrlInAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrSpec, ImUrl *pImUrl, SipError *pErr)
#else
	(pAddrSpec, pImUrl, pErr)
	SipAddrSpec *pAddrSpec;
	ImUrl *pImUrl;
	SipError *pErr;
#endif
{
	SIP_S8bit *pOut=SIP_NULL;
	SIP_U32bit listSize=0;

	SIPDEBUGFN("Entering sip_setImUrlInAddrSpec\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pAddrSpec->dType != SipAddrReqUri )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( pAddrSpec->u.pUri != SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pAddrSpec->u.pUri),\
			pErr) ==SipFail)
			return SipFail;
	}

	if (pImUrl==SIP_NULL)
	{
		pAddrSpec->u.pUri=SIP_NULL;
		*pErr=E_NO_ERROR;
	}
	else
	{
		pOut=(SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID,MAX_IM_URL_SIZE, pErr);
		if (pOut == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy(pOut,"im:");

		if(pImUrl->pDispName != SIP_NULL)
		{
			STRCAT(pOut,pImUrl->pDispName);
			STRCAT(pOut,"%3c");
		}

		sip_formSipList(pOut, &(pImUrl->slRoute), (SIP_S8bit *) ",",1, pErr);

		if(pImUrl->pUser != SIP_NULL)
		{
			STRCAT(pOut,pImUrl->pUser);
			STRCAT(pOut,"@");
		}

		if(pImUrl->pHost != SIP_NULL)
		{
			STRCAT(pOut,pImUrl->pHost);
		}

		if(pImUrl->pDispName != SIP_NULL)
		{
			STRCAT(pOut,"%3e");
		}

		/* Now Form Header */
		sip_listSizeOf(&(pImUrl->slParams), &listSize, pErr);
		if(listSize!=0)
		{
			STRCAT(pOut,"?");
		}

		/* Called with leadingsep value of 2 to avoid putting leading space or
		   leading separator */
		sip_formSipParamList(0,&pOut, &(pImUrl->slParams), (SIP_S8bit *) "&",\
			2, pErr);

		pAddrSpec->u.pUri=pOut;
	}

	SIPDEBUGFN("Exiting sip_setImUrlInAddrSpec\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getDispNameFromImUrl
** Description: gets the Display Name field from the ImUrl structure
** Parameters:
**		pUrl (IN)		- ImUrl
**		ppDispName (OUT)- retrieved Display Name
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getDispNameFromImUrl
#ifdef ANSI_PROTO
	(ImUrl *pUrl,
	SIP_S8bit **ppDispName,
	SipError *pErr)
#else
	(pUrl,ppDispName,pErr)
	ImUrl *pUrl;
	SIP_S8bit **ppDispName;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_getDispNameFromImUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (( pUrl == SIP_NULL) || (ppDispName == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (pUrl->pDispName == SIP_NULL)
	{
		*pErr=E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
 	*ppDispName = (SIP_S8bit *) STRDUPACCESSOR (pUrl->pDispName);
	if (*ppDispName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppDispName = pUrl->pDispName;
#endif

	SIPDEBUGFN("Exiting sip_getDispNameFromImUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setDispNameInImUrl
** Description: sets the Display Name in the ImUrl structure
** Parameters:
**		pUrl (IN/OUT)	- ImUrl
**		pDispName (OUT)	- Display Name to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setDispNameInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pUrl,
	SIP_S8bit *pDispName,
	SipError *pErr)
#else
	(pUrl,pDispName,pErr)
	ImUrl *pUrl;
	SIP_S8bit *pDispName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_disp_name=SIP_NULL;
	SIP_S8bit *pName=SIP_NULL;

	SIPDEBUGFN("Entering sip_setDispNameInImUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( pDispName == SIP_NULL)
		pTemp_disp_name = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		pTemp_disp_name = (SIP_S8bit *)STRDUPACCESSOR(pDispName);
		if (pTemp_disp_name == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTemp_disp_name = pDispName;
#endif
	}

	pName = pUrl->pDispName;
	if (pName != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&pName), pErr) == \
			SipFail)
		{
#ifndef SIP_BY_REFERENCE
			sip_freeString(pTemp_disp_name);
#endif
			return SipFail;
		}
	}

	pUrl->pDispName = pTemp_disp_name;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_setDispNameInImUrl\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getUserFromImUrl
** Description: gets the User Name field from the ImUrl structure
** Parameters:
**		pUrl (IN)		- ImUrl
**		ppUser (OUT)	- retrieved User
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getUserFromImUrl
#ifdef ANSI_PROTO
	(ImUrl *pUrl,
	SIP_S8bit **ppUser,
	SipError *pErr)
#else
	(pUrl,ppUser,pErr)
	ImUrl *pUrl;
	SIP_S8bit **ppUser;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_getUserFromImUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (( pUrl == SIP_NULL) || (ppUser  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (pUrl->pUser == SIP_NULL)
	{
		*pErr=E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
 	*ppUser = (SIP_S8bit *) STRDUPACCESSOR (pUrl->pUser);
	if (*ppUser == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppUser = pUrl->pUser;
#endif

	SIPDEBUGFN("Exiting sip_getUserFromImUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setUserInImUrl
** Description: sets the User in the ImUrl structure
** Parameters:
**		pUrl (IN/OUT)	- ImUrl
**		pUser (OUT)		- User to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setUserInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pUrl,
	SIP_S8bit *pUser,
	SipError *pErr)
#else
	(pUrl,pUser,pErr)
	ImUrl *pUrl;
	SIP_S8bit *pUser;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_user_name=SIP_NULL;
	SIP_S8bit *pUsertemp=SIP_NULL;

	SIPDEBUGFN("Entering sip_setUserInImUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( pUser == SIP_NULL)
		pTemp_user_name = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		pTemp_user_name = (SIP_S8bit *)STRDUPACCESSOR(pUser);
		if (pTemp_user_name == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTemp_user_name = pUser;
#endif
	}

	pUsertemp = pUrl->pUser;
	if (pUsertemp != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&pUsertemp), pErr) == \
			SipFail)
		{
#ifndef SIP_BY_REFERENCE
			sip_freeString(pTemp_user_name);
#endif
			return SipFail;
		}
	}

	pUrl->pUser = pTemp_user_name;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_setUserInImUrl\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getHostFromImUrl
** Description: gets the Host Name field from the ImUrl structure
** Parameters:
**		pUrl (IN)		- ImUrl
**		ppHost (OUT)	- retrieved Host
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getHostFromImUrl
#ifdef ANSI_PROTO
	(ImUrl *pUrl,
	SIP_S8bit **ppHost,
	SipError *pErr)
#else
	(pUrl,ppHost,pErr)
	ImUrl *pUrl;
	SIP_S8bit **ppHost;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_getHostFromImUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (( pUrl == SIP_NULL) || (ppHost  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (pUrl->pHost == SIP_NULL)
	{
		*pErr=E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
 	*ppHost = (SIP_S8bit *) STRDUPACCESSOR (pUrl->pHost);
	if (*ppHost == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppHost = pUrl->pHost;
#endif

	SIPDEBUGFN("Exiting sip_getHostFromImUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setHostInImUrl
** Description: sets the Host in the ImUrl structure
** Parameters:
**		pUrl (IN/OUT)	- ImUrl
**		pHost (OUT)		- Host to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setHostInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pUrl,
	SIP_S8bit *pHost,
	SipError *pErr)
#else
	(pUrl,pHost,pErr)
	ImUrl *pUrl;
	SIP_S8bit *pHost;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_host_name=SIP_NULL;
	SIP_S8bit *pHosttemp=SIP_NULL;

	SIPDEBUGFN("Entering sip_setHostInImUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( pHost == SIP_NULL)
		pTemp_host_name = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		pTemp_host_name = (SIP_S8bit *)STRDUPACCESSOR(pHost);
		if (pTemp_host_name == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTemp_host_name = pHost;
#endif
	}

	pHosttemp = pUrl->pHost;
	if (pHosttemp != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&pHosttemp), pErr) == \
			SipFail)
		{
#ifndef SIP_BY_REFERENCE
			sip_freeString(pTemp_host_name);
#endif
			return SipFail;
		}
	}

	pUrl->pHost = pTemp_host_name;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_setHostInImUrl\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getParamCountFromImUrl
** Description: gets the number of parameters in ImUrl
** Parameters:
**		pImUrl (IN)		- ImUrl
**		pCount (OUT)	- number of parameters
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getParamCountFromImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl,SIP_U32bit *pCount, SipError *pErr)
#else
 	(pImUrl,pCount,pErr)
	ImUrl *pImUrl;
 	SIP_U32bit *pCount;
 	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pImUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(pImUrl)->slParams, pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromImUrl");
	return SipSuccess;
}

/*****************************************************************************
** Function: sip_getParamAtIndexFromImUrl
** Description: gets the Param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN)		- ImUrl
**	ppParam(OUT)	- retreived Parameter
**	dIndex (IN)		- index at which param is to be retieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/

SipBool sip_getParamAtIndexFromImUrl
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(ImUrl *pImUrl,SipParam **ppParam, SIP_U32bit dIndex,\
		SipError *pErr)
#else
	(ImUrl *pImUrl,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pImUrl,ppParam, dIndex, pErr)
	ImUrl *pImUrl;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pImUrl,pParam, dIndex, pErr)
	ImUrl *pImUrl;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *pTemp_param=SIP_NULL;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pImUrl == SIP_NULL )
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

#endif

	if (sip_listGetAt( &(pImUrl->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTemp_param = (SipParam *)element_from_list;

#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, pTemp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(pTemp_param->dRefCount);
	*ppParam = pTemp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromImUrl");
	return SipSuccess;
}

/*****************************************************************************
** Function: sip_setParamAtIndexInImUrl
** Description: sets the Param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN/OUT)		- ImUrl
**	pParam(IN)			- Param to be set
**	dIndex (IN)			- index at which param is set in ImUrl
**	pErr (OUT)			- Possible Error value (see API ref doc)
******************************************************************************/

SipBool sip_setParamAtIndexInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#else
	(pImUrl,pParam, dIndex, pErr)
	ImUrl *pImUrl;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTemp_param=SIP_NULL;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pImUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		pTemp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(pImUrl->slParams),  \
		dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInImUrl");
	return SipSuccess;
}
/******************************************************************************
** Function: sip_insertParamAtIndexInImUrl
** Description: inserts the Param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN/OUT)	- ImUrl
**	pParam(IN)		- Param to be inserted
**	dIndex (IN)		- index at which param is inserted in ImUrl
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/

SipBool sip_insertParamAtIndexInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl, SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#else
	(pImUrl, pParam, dIndex, pErr)
	ImUrl *pImUrl;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *pTemp_param=SIP_NULL;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pImUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if ( pParam == SIP_NULL )
		pTemp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(pImUrl->slParams),  \
		dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInImUrl");
	return SipSuccess;
}

/***********************************************************************
** Function: sip_deleteParamAtIndexInImUrl
** Description: deletes the param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN)		- ImUrl
**	dIndex (IN)		- index at which param is deleted in ImUrl
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_deleteParamAtIndexInImUrl
#ifdef ANSI_PROTO
(ImUrl *pImUrl, SIP_U32bit dIndex, SipError *pErr)
#else
(pImUrl, dIndex, pErr)
ImUrl *pImUrl;
SIP_U32bit dIndex;
SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pImUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if( sip_listDeleteAt( &(pImUrl->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInImUrl");
	return SipSuccess;
}


/***************************************************************
** Function: sip_getRouteCountFromImUrl
** Description: gets the Route count from the ImUrl Structure
** Parameters:
**	pImUrl (IN)		- ImUrl
**	pCount (OUT)	- number of ImUrl Routes
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getRouteCountFromImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl,
	SIP_U32bit *pCount,
	SipError *pErr)
#else
	(pImUrl, pCount, pErr)
	ImUrl *pImUrl;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getRouteCountFromImUrl");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if ( (pImUrl == SIP_NULL) || ( pCount == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pImUrl->slRoute), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting getRouteCountFromImUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_getRouteAtIndexFromImUrl
** Description: gets the Route field at the index from ImUrl
** Parameters:
**	pImUrl (IN)		- ImUrl
**	ppRoute(OUT)	- retreived Route
**	dIndex (IN)		- index at which Route is to be retrieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getRouteAtIndexFromImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl,
	 SIP_S8bit **ppRoute,
	 SIP_U32bit dIndex,
	 SipError 	*pErr )
#else
	(pImUrl,ppRoute,dIndex,pErr)
	 ImUrl *pImUrl;
	 SIP_S8bit **ppRoute;
     SIP_U32bit dIndex;
	 SipError *pErr;
#endif
{
	SIP_Pvoid pElementFromList;
#ifndef SIP_BY_REFERENCE
	SIP_U32bit size=0;
#endif

	SIPDEBUGFN ( "Entering sip_getRouteAtIndexFromImUrl ");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ( (pImUrl == SIP_NULL) || (ppRoute == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#else
	if ( (pImUrl == SIP_NULL) || (ppRoute == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
#endif
	if( sip_listGetAt(&(pImUrl->slRoute), dIndex, &pElementFromList, \
		pErr) == SipFail)
		return SipFail;

	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppRoute = (SIP_S8bit*)pElementFromList;
#else
	size = strlen( (SIP_S8bit * )pElementFromList);
	*ppRoute = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, size +1, \
		pErr);
	if(*ppRoute == SIP_NULL)
		return SipFail;

	strcpy(*ppRoute, (SIP_S8bit*)pElementFromList);
#endif
	SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromImUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setRouteAtIndexInImUrl
** Description: sets the Route field at the index in ImUrl
** Parameters:
**	pImUrl (IN/OUT)		- ImUrl
**	pRoute(IN)			- Route to be set
**	dIndex (IN)			- index at which Route is to be set
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool  sip_setRouteAtIndexInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl,
	 SIP_S8bit *pRoute,
	 SIP_U32bit dIndex,
	 SipError 	*pErr )
#else
	(pImUrl,pRoute,dIndex,pErr)
	 ImUrl *pImUrl;
	 SIP_S8bit *pRoute;
     SIP_U32bit dIndex;
	 SipError *pErr;
#endif
{

	SIP_S8bit * pElementFromList=SIP_NULL;

	SIPDEBUGFN ( "Entering sip_setRouteAtIndexInImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pImUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pRoute  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pRoute;
#else

		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pRoute) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pRoute );
#endif
	}

	if( sip_listSetAt(&(pImUrl->slRoute), dIndex, pElementFromList, \
		pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), \
				pErr);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting sip_setRouteAtIndexInImUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_insertRouteAtIndexInImUrl
** Description: inserts the Route field at the index in ImUrl
** Parameters:
**	pImUrl (IN/OUT)		- ImUrl
**	pRoute(IN)			- Route to be inserted
**	dIndex (IN)			- index at which Route is to be inserted
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_insertRouteAtIndexInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl,
	 SIP_S8bit *pRoute,
	 SIP_U32bit	dIndex,
	 SipError 	*pErr )
#else
	(pImUrl,pRoute,dIndex,pErr)
	 ImUrl *pImUrl;
	 SIP_S8bit *pRoute;
     SIP_U32bit dIndex;
	 SipError *pErr;
#endif
{

	SIP_S8bit * pElementFromList=SIP_NULL;

	SIPDEBUGFN ( "Entering sip_insertRouteAtIndexInImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pImUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the Value Headers structure/char*  */
	if (pRoute  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pRoute	;
#else
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pRoute) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pRoute );
#endif
	}

	if( sip_listInsertAt(&(pImUrl->slRoute), dIndex, pElementFromList,\
		pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), \
				pErr);
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting sip_insertRouteAtIndexInImUrl");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_deleteRouteAtIndexInImUrl
** Description: deletes the Route field at the dIndex in ImUrl
** Parameters:
**	pImUrl (IN/OUT)		- ImUrl
**	pRoute(IN)			- Route to be deleted
**	dIndex (IN)			- index at which Route is to be deleted
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_deleteRouteAtIndexInImUrl
#ifdef ANSI_PROTO
	(ImUrl *pImUrl,
	  SIP_U32bit dIndex,
	  SipError 	*pErr )
#else
	(pImUrl,dIndex,pErr)
  	ImUrl *pImUrl;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{

	SIPDEBUGFN ( "Entering sip_deleteRouteAtIndexInImUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pImUrl  == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pImUrl->slRoute), dIndex, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exiting sip_deleteRouteAtIndexInImUrl");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_cloneImUrl
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the  ImUrl structures "from" to "to".
** Parameters:
**	pTo (OUT)		- ImUrl
**	pFrom (IN)		- ImUrl which has to be cloned
**	pErr (OUT)		- Possible Error value (see API ref doc)
**
**********************************************************/

SipBool sip_cloneImUrl
#ifdef ANSI_PROTO
	(ImUrl *pTo,
	ImUrl *pFrom,
	SipError *pErr)
#else
	( pTo,pFrom, pErr )
	ImUrl *pTo;
	ImUrl *pFrom;
	SipError *pErr;
#endif
{
	SIP_U32bit dLength=0;

	SIPDEBUGFN("Entering function sip_cloneImUrl");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pTo == SIP_NULL)||(pFrom  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pTo->pDispName  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(pTo->pDispName)), pErr) \
		== SipFail)
		return SipFail;
	}
	if(pTo->pUser  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(pTo->pUser)), pErr) \
		== SipFail)
		return SipFail;
	}
	if(pTo->pHost  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(pTo->pHost)), pErr) \
		== SipFail)
		return SipFail;
	}
	if (sip_listDeleteAll(&(pTo->slParams), pErr) == SipFail)
		return SipFail;

	if (sip_listDeleteAll(&(pTo->slRoute), pErr) == SipFail)
		return SipFail;

	/* cleaning of to over */

	if(pFrom->pDispName== SIP_NULL)
	{
		pTo->pDispName=SIP_NULL;
	}
	else
	{
		dLength = strlen(pFrom->pDispName );

		pTo->pDispName=(SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTo->pDispName == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( pTo->pDispName, pFrom->pDispName );
	}
	if(pFrom->pUser== SIP_NULL)
	{
		pTo->pUser=SIP_NULL;
	}
	else
	{
		dLength = strlen(pFrom->pUser );

		pTo->pUser=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTo->pUser == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( pTo->pUser, pFrom->pUser );
	}
	if(pFrom->pHost== SIP_NULL)
	{
		pTo->pHost=SIP_NULL;
	}
	else
	{
		dLength = strlen(pFrom->pHost );

		pTo->pHost=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTo->pHost == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( pTo->pHost, pFrom->pHost);
	}
	if (__sip_cloneSipStringList(&(pTo->slRoute), \
		&(pFrom->slRoute), pErr) == SipFail)
		return SipFail;

	if (__sip_cloneSipParamList (&(pTo->slParams), &(pFrom->slParams), \
		pErr) == SipFail)
		return SipFail;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneImUrl");
	return SipSuccess;
}


