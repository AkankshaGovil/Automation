
 /******************************************************************************
 ** FUNCTION:
 **	 	This file has all the internal functions of the Instant Messaging
 ** 	and Presence Related headers
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		imppinternal.c
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

#include "imppinternal.h"
#include "sipclone.h"
#include "sipvalidate.h"


/******************************************************************
**
** FUNCTION:  __sip_impp_cloneSipEventHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from 
**	the EventHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_impp_cloneSipEventHeader
#ifdef ANSI_PROTO
	(SipEventHeader *dest, SipEventHeader *source, SipError *err)
#else
	(dest, source, err)
	SipEventHeader *dest;
	SipEventHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIP_U32bit count;

	SIPDEBUGFN("Entering function __sip_impp_cloneSipEventHeader");
	if (dest->pEventType != SIP_NULL)
		sip_freeString(dest->pEventType);
	if(sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	if (source->pEventType == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pEventType)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	} 
	dest->pEventType = temp;	
	
	/* copying siplist of SipParmas */
	if (sip_listInit(& (dest->slParams ),(source->slParams).freefunc,err)\
		==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( source->slParams ), &count, err) == SipFail )
		return SipFail;

	if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_impp_cloneSipEventHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_impp_cloneSipAllowEventsHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from 
**	the AllowEventsHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_impp_cloneSipAllowEventsHeader
#ifdef ANSI_PROTO
	(SipAllowEventsHeader *dest, SipAllowEventsHeader *source, SipError *err)
#else
	(dest, source, err)
	SipAllowEventsHeader *dest;
	SipAllowEventsHeader *source;
	SipError *err;
#endif
{	
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_impp_cloneSipAllowEventsHeader");
	if (dest->pEventType != SIP_NULL)
		sip_freeString(dest->pEventType);

	if (source->pEventType == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pEventType);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pEventType = temp;
	
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_impp_cloneSipAllowEventsHeader");
	return SipSuccess;	
}

SipBool __sip_impp_cloneSipAllowEventsHeaderList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAllowEventsHeader *temp_allowevents, *clone_allowevents;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_impp_cloneSipAllowEventsHeaderList");
	/* copying siplist of SipAllowEventsHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_allowevents), \
			pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_allowevents == SIP_NULL )
			clone_allowevents = SIP_NULL;
		else
		{
			if(sip_impp_initSipAllowEventsHeader(&clone_allowevents, pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( clone_allowevents == SIP_NULL )
				return SipFail;

			if ( __sip_impp_cloneSipAllowEventsHeader(  clone_allowevents, \
				temp_allowevents, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_allowevents), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_allowevents, pErr) == SipFail )
		{
			if ( clone_allowevents != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_allowevents), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_impp_cloneSipAllowEventsHeaderList");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_impp_cloneSipSubscriptionStateHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the SubscriptionStateHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_impp_cloneSipSubscriptionStateHeader
#ifdef ANSI_PROTO
	(SipSubscriptionStateHeader *pDest, \
	 SipSubscriptionStateHeader *pSource, \
	 SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipSubscriptionStateHeader *pDest;
	SipSubscriptionStateHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_U32bit dCount=0;
	SIP_S8bit *temp;

	SIPDEBUGFN("Entering function \
	__sip_impp_cloneSipSubscriptionStateHeader");
	if (pDest->pSubState!= SIP_NULL)
		sip_freeString(pDest->pSubState);
	if(sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;

	if (pSource->pSubState == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if((temp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pSubState)) == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	} 
	pDest->pSubState = temp;


	/* copying siplist of SipParmas */
	if (sip_listInit(& (pDest->slParams ),(pSource->slParams).freefunc,pErr)\
		==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( pSource->slParams ), &dCount, pErr) == SipFail )
		return SipFail;

	if(__sip_cloneSipParamList(&(pDest->slParams),&(pSource->slParams),pErr)==\
		SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function \
	__sip_impp_cloneSipSubscriptionStateHeader");
	return SipSuccess;
}
