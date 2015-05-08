/************************************************************
** FUNCTION:
	This file implements the response pHeader accessor APIs.
**
*************************************************************
**
** FILENAME:
	resp1csb.c
**
** DESCRIPTION
**
**
**  DATE           NAME                    REFERENCE
** ------        ---------                -------------
**   22nov99	B.Borthakur	            Original
**
** Copyright 1999, Hughes Software Systems, Ltd.
*************************************************************/


#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "response.h"
#include "sipinternal.h"
#include "sipclone.h"

/*********************************************************************
** FUNCTION:   sip_getCodeFromWarningHdr
**********************************************************************
**
** DESCRIPTION: Gets the dCodeNum field pValue from warning pHeader.
**
*********************************************************************/

SipBool sip_getCodeFromWarningHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_U16bit *dCodeNum, SipError *err)
#else
	( hdr, dCodeNum, err )
	  SipHeader *hdr;
	  SIP_U16bit *dCodeNum;
	  SipError *err;
#endif
{
	SIPDEBUGFN ( "Entering getCoAdeFromWarningHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ((hdr == SIP_NULL)||(dCodeNum==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWarning)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

#endif

	*dCodeNum = ( (SipWarningHeader *) (hdr->pHeader) )->dCodeNum;

	SIPDEBUGFN ( "Exiting getCodeFromWarningHdr");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:   sip_setCodeInWarningHdr
**********************************************************************
**
** DESCRIPTION: Sets the dCodeNum field in warning pHeader.
**
*********************************************************************/
SipBool sip_setCodeInWarningHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_U16bit dCodeNum, SipError *err)
#else
	( hdr, dCodeNum, err )
	  SipHeader *hdr;
	  SIP_U16bit dCodeNum;
	  SipError *err;
#endif
{

	SIPDEBUGFN ( "Entering setCodeInWarningHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWarning)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

#endif

	( (SipWarningHeader *) (hdr->pHeader) )->dCodeNum = dCodeNum;

	SIPDEBUGFN ( "Exiting setCodeInWarningHdr");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:   sip_getAgentFromWarningHdr
**********************************************************************
**
** DESCRIPTION: Gets the pAgent field pValue from warning pHeader.
**
*********************************************************************/
SipBool sip_getAgentFromWarningHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit **pAgent, SipError *err)
#else
	( hdr, pAgent, err )
	  SipHeader *hdr;
	  SIP_S8bit **pAgent;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_agent;

	SIPDEBUGFN ( "Entering getAgentFromWarningHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ((hdr == SIP_NULL)||(pAgent==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWarning)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_agent = ( (SipWarningHeader *) (hdr->pHeader) )->pAgent;

	if( temp_agent == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
		*pAgent = temp_agent;
#else
	dLength = strlen(temp_agent);
	*pAgent = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pAgent == SIP_NULL )
		return SipFail;

	strcpy( *pAgent , temp_agent );
#endif

	SIPDEBUGFN ( "Exiting getAgentFromWarningHdr");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:   sip_setAgentInWarningHdr
**********************************************************************
**
** DESCRIPTION: Sets the pAgent field pValue in warning pHeader.
**
*********************************************************************/
SipBool sip_setAgentInWarningHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit *pAgent, SipError *err)
#else
	( hdr, pAgent, err )
	  SipHeader *hdr;
	  SIP_S8bit *pAgent;
	  SipError *err;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_agent;
	SipWarningHeader * temp_warn_hdr;

	SIPDEBUGFN ( "Entering setAgentInWarningHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWarning)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader==SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

#endif
	if( pAgent == SIP_NULL)
		temp_agent = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_agent = pAgent;
#else
		dLength = strlen( pAgent );
		temp_agent = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_agent== SIP_NULL )
			return SipFail;

		strcpy( temp_agent, pAgent );
#endif
	}

	temp_warn_hdr = ( SipWarningHeader *) ( hdr->pHeader);
	if ( temp_warn_hdr->pAgent != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(temp_warn_hdr->pAgent)), err) == SipFail)
			return SipFail;
	}


	temp_warn_hdr->pAgent = temp_agent;

	SIPDEBUGFN ( "Exiting setAgentInWarningHdr");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:   sip_getTextFromWarningHdr
**********************************************************************
**
** DESCRIPTION: Gets the pText field pValue from warning pHeader.
**
*********************************************************************/
SipBool sip_getTextFromWarningHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit **pText, SipError *err)
#else
	( hdr, pText, err )
	  SipHeader *hdr;
	  SIP_S8bit **pText;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_text;

	SIPDEBUGFN ( "Entering getTextFromWarningHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (( hdr == SIP_NULL)||(pText==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWarning)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_text = ( (SipWarningHeader *) (hdr->pHeader) )->pText;

	if( temp_text == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*pText = temp_text;
#else
	dLength = strlen(temp_text);
	*pText = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pText == SIP_NULL )
		return SipFail;

	strcpy( *pText , temp_text );
#endif
	SIPDEBUGFN ( "Exiting getTextFromWarningHdr");

	*err = E_NO_ERROR;
	return SipSuccess;


}


/*********************************************************************
** FUNCTION:   sip_setTextInWarningHdr
**********************************************************************
**
** DESCRIPTION: Sets the pText field in warning pHeader.
**
*********************************************************************/
SipBool sip_setTextInWarningHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit *pText, SipError *err)
#else
	( hdr, pText, err )
	  SipHeader *hdr;
	  SIP_S8bit *pText;
	  SipError *err;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_text;
	SipWarningHeader* temp_warn_hdr;

	SIPDEBUGFN ( "Entering setTextInWarningHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWarning)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

#endif
	if( pText == SIP_NULL)
		temp_text = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_text = pText;
#else
		dLength = strlen( pText );
		temp_text = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_text== SIP_NULL )
			return SipFail;

		strcpy( temp_text, pText );
#endif

	}

	temp_warn_hdr = ( SipWarningHeader *) ( hdr->pHeader);
	if ( temp_warn_hdr->pText != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)( &(temp_warn_hdr->pText)), err) == SipFail)
			return SipFail;
	}


	temp_warn_hdr->pText = temp_text;

	SIPDEBUGFN ( "Exiting setTextInWarningHdr");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:   sip_getDeltaSecondsFromMinExpiresHdr
**********************************************************************
**
** DESCRIPTION: Gets the seconds field from MinExpires pHeader.
**
*********************************************************************/
SipBool sip_getDeltaSecondsFromMinExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_U32bit *dseconds, SipError *err)
#else
	( hdr, dseconds, err )
	  SipHeader *hdr;
	  SIP_U32bit *dseconds;
	  SipError *err;
#endif
{

	SipMinExpiresHeader * temp_ret_aft_hdr;

	SIPDEBUGFN ( "Entering getDeltaSecondsFromMinExpiresHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ((hdr == SIP_NULL)||(dseconds == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if((hdr->dType != SipHdrTypeMinExpires))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_ret_aft_hdr = ( SipMinExpiresHeader*) ( hdr->pHeader );


	*dseconds = temp_ret_aft_hdr->dSec;

	SIPDEBUGFN ( "Exiting getDeltaSecondsFromMinExpiresHeader");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_setDeltaSecondsInMinExpiresHdr
**********************************************************************
**
** DESCRIPTION: Sets the seconds field pValue in MinExpires pHeader.
**
*********************************************************************/
SipBool sip_setDeltaSecondsInMinExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_U32bit dseconds, SipError *err)
#else
	( hdr, dseconds, err )
	  SipHeader *hdr;
	  SIP_U32bit dseconds;
	  SipError *err;
#endif
{
	SipMinExpiresHeader * temp_ret_aft_hdr;

	SIPDEBUGFN ( "Entering setDeltaSecondsInMinExpiresHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if( (hdr->dType != SipHdrTypeMinExpires))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_ret_aft_hdr = ( SipMinExpiresHeader*) ( hdr->pHeader );
	temp_ret_aft_hdr->dSec  = dseconds;

	SIPDEBUGFN ( "Exiting setDeltaSecondsInMinExpiresHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_getTypeFromRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: Gets the dType pValue from retry after pHeader
**
*********************************************************************/
SipBool sip_getTypeFromRetryAfterHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, en_ExpiresType *dType, SipError *err)
#else
	( hdr, dType, err )
	  SipHeader *hdr;
	  en_ExpiresType *dType;
	  SipError *err;
#endif
{
	SIPDEBUGFN ( "Entering getDateFromRetryAfterHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (( hdr == SIP_NULL)||(dType == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if( (hdr->dType != SipHdrTypeRetryAfterSec) &&   \
		(hdr->dType != SipHdrTypeRetryAfterDate))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	*dType = ( ( SipRetryAfterHeader*)(hdr->pHeader))->dType;

	SIPDEBUGFN ( "Exiting getDateFromRetryAfterHeader");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_getDateFromRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: Gets the dDate field pValue from retry after pHeader
**
*********************************************************************/
SipBool sip_getDateFromRetryAfterHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader *hdr, SipDateStruct **ppDate, SipError *err)
#else
	( SipHeader *hdr, SipDateStruct *pDate, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( hdr, ppDate, err )
	  SipHeader *hdr;
	  SipDateStruct **ppDate;
	  SipError *err;
#else
	( hdr, pDate, err )
	  SipHeader *hdr;
	  SipDateStruct *pDate;
	  SipError *err;
#endif
#endif
{
	SIPDEBUGFN ( "Entering getDateFromRetryAfterHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	if(ppDate == SIP_NULL)
#else
	if(pDate == SIP_NULL )
#endif
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if( (hdr->dType != SipHdrTypeRetryAfterSec) &&   \
		(hdr->dType != SipHdrTypeRetryAfterDate) )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif

	if( ( (SipRetryAfterHeader*) (hdr->pHeader) )->dType != SipExpDate )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if ((( (SipRetryAfterHeader*) (hdr->pHeader) )->u.pDate) == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF((( (SipRetryAfterHeader*) (hdr->pHeader) )->u.pDate)->dRefCount);
	*ppDate = ( (SipRetryAfterHeader*) (hdr->pHeader) )->u.pDate;
#else
	__sip_cloneSipDateStruct(pDate, ((SipRetryAfterHeader*)(hdr->pHeader))->u.pDate,err);
#endif
	SIPDEBUGFN ( "Exiting getDateFromRetryAfterHeader");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_setDateInRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: Sets the dDate field pValue in retry after pHeader.
**
*********************************************************************/
SipBool sip_setDateInRetryAfterHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SipDateStruct *dDate, SipError *err)
#else
	( hdr, dDate, err )
	  SipHeader *hdr;
	  SipDateStruct *dDate;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipDateStruct *temp_date_struct;
#endif
	SipRetryAfterHeader * temp_ret_aft_hdr;

	SIPDEBUGFN ( "Entering setDateInRetryAfterHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if( (hdr->dType != SipHdrTypeRetryAfterSec)&&   \
		(hdr->dType != SipHdrTypeRetryAfterDate))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

	if( ( (SipRetryAfterHeader*) (hdr->pHeader) )->dType != SipExpDate )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

#endif

	temp_ret_aft_hdr = (SipRetryAfterHeader*)(hdr->pHeader);
	if (dDate == SIP_NULL)
	{
		if ((temp_ret_aft_hdr->u.pDate) != SIP_NULL)
			sip_freeSipDateStruct(temp_ret_aft_hdr->u.pDate);
		temp_ret_aft_hdr->u.pDate=SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipDateStruct(&temp_date_struct, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipDateStruct(temp_date_struct, dDate, err) == SipFail)
		{
			sip_freeSipDateStruct(temp_date_struct);
			return SipFail;
		}
		sip_freeSipDateStruct((((SipRetryAfterHeader *)(hdr->pHeader))->u).pDate);
		(((SipRetryAfterHeader *)(hdr->pHeader))->u).pDate = temp_date_struct;
#else
		if ((temp_ret_aft_hdr->u.pDate) != SIP_NULL)
			sip_freeSipDateStruct(temp_ret_aft_hdr->u.pDate);
		HSS_LOCKEDINCREF(dDate->dRefCount);
		temp_ret_aft_hdr->u.pDate = dDate;
#endif
	}
	SIPDEBUGFN ( "Exiting setDateInRetryAfterHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_getDeltaSecondsFromRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: Gets the seconds field from retry after pHeader.
**
*********************************************************************/
SipBool sip_getDeltaSecondsFromRetryAfterHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_U32bit *dseconds, SipError *err)
#else
	( hdr, dseconds, err )
	  SipHeader *hdr;
	  SIP_U32bit *dseconds;
	  SipError *err;
#endif
{

	SipRetryAfterHeader * temp_ret_aft_hdr;

	SIPDEBUGFN ( "Entering getDeltaSecondsFromRetryAfterHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ((hdr == SIP_NULL)||(dseconds == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if((hdr->dType != SipHdrTypeRetryAfterSec ) &&  \
		(hdr->dType != SipHdrTypeRetryAfterDate ) )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_ret_aft_hdr = ( SipRetryAfterHeader*) ( hdr->pHeader );

	if( temp_ret_aft_hdr->dType != SipExpSeconds )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	*dseconds = temp_ret_aft_hdr->u.dSec;

	SIPDEBUGFN ( "Exiting getDeltaSecondsFromRetryAfterHeader");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_setDeltaSecondsInRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: Sets the seconds field pValue in retry after pHeader.
**
*********************************************************************/
SipBool sip_setDeltaSecondsInRetryAfterHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_U32bit dseconds, SipError *err)
#else
	( hdr, dseconds, err )
	  SipHeader *hdr;
	  SIP_U32bit dseconds;
	  SipError *err;
#endif
{
	SipRetryAfterHeader * temp_ret_aft_hdr;

	SIPDEBUGFN ( "Entering setDeltaSecondsInRetryAfterHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if( (hdr->dType != SipHdrTypeRetryAfterSec) &&  \
		(hdr->dType != SipHdrTypeRetryAfterDate) )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_ret_aft_hdr = ( SipRetryAfterHeader*) ( hdr->pHeader );
	if( temp_ret_aft_hdr->dType != SipExpSeconds )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	temp_ret_aft_hdr->u.dSec  = dseconds;

	SIPDEBUGFN ( "Exiting setDeltaSecondsInRetryAfterHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_getCommentFromRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: Gets the pComment field pValue from retry after pHeader.
**
*********************************************************************/
SipBool sip_getCommentFromRetryAfterHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit **pComment, SipError *err)
#else
	( hdr, pComment, err )
	  SipHeader *hdr;
	  SIP_S8bit **pComment;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_comment;

	SIPDEBUGFN ( "Entering getCommentFromWarningHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (( hdr == SIP_NULL)||(pComment == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( ( hdr->dType != SipHdrTypeRetryAfterSec ) &&( hdr->dType != SipHdrTypeRetryAfterDate )  )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	temp_comment = ( ( SipRetryAfterHeader *)(hdr->pHeader ) )->pComment;

	if( temp_comment == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*pComment = temp_comment;
#else
	dLength = strlen(temp_comment);
	*pComment = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pComment == SIP_NULL )
		return SipFail;

	strcpy( *pComment , temp_comment );
#endif

	SIPDEBUGFN ( "Exiting getCommentFromWarningHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_setCommentInRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: Sets the pComment field pValue in retry after pHeader.
**
*********************************************************************/
SipBool sip_setCommentInRetryAfterHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit *pComment, SipError *err)
#else
	( hdr, pComment, err )
	  SipHeader *hdr;
	  SIP_S8bit *pComment;
	  SipError *err;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_comment;
	SipRetryAfterHeader * temp_ret_aft_hdr;

	SIPDEBUGFN ( "Entering setCommentInRetryAfterHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (( hdr->dType != SipHdrTypeRetryAfterSec )&& ( hdr->dType != SipHdrTypeRetryAfterDate ))
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( pComment == SIP_NULL)
		temp_comment = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_comment = pComment;
#else
		dLength = strlen( pComment );
		temp_comment = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_comment == SIP_NULL )
			return SipFail;

		strcpy( temp_comment, pComment );
#endif
	}

	temp_ret_aft_hdr = ( SipRetryAfterHeader *) ( hdr->pHeader);
	if ( temp_ret_aft_hdr->pComment != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(temp_ret_aft_hdr->pComment)), err) == SipFail)
			return SipFail;
	}

	temp_ret_aft_hdr->pComment = temp_comment;

	SIPDEBUGFN ( "Exiting setCommentInRetryAfterHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_getParamCountFromRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: This function retrieves the number of params from a Retry
**		after header
**
*********************************************************************/
SipBool sip_getParamCountFromRetryAfterHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromRetryAfterHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pHdr == SIP_NULL )||(pCount == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( (pHdr->dType != SipHdrTypeRetryAfterDate)&&(pHdr->dType != SipHdrTypeRetryAfterSec))
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
	if (sip_listSizeOf( &(((SipRetryAfterHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromRetryAfterHdr");
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_getParamAtIndexFromRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: This function retrieves a param at a specified index
**		in a Retry-after header
**
*********************************************************************/
SipBool sip_getParamAtIndexFromRetryAfterHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromRetryAfterHdr");
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

	if ((pHdr->dType != SipHdrTypeRetryAfterDate)&&(pHdr->dType != SipHdrTypeRetryAfterSec))
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

	if (sip_listGetAt( &(((SipRetryAfterHeader *)(pHdr->pHeader))->slParams), dIndex,  \
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromRetryAfterHdr");
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_setParamAtIndexInRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: This function  sets a param at a specified index
**		in a Retry-after header
**
*********************************************************************/
SipBool sip_setParamAtIndexInRetryAfterHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInRetryAfterHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRetryAfterDate)&&(pHdr->dType != SipHdrTypeRetryAfterSec))
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

	if( sip_listSetAt( &(((SipRetryAfterHeader *)(pHdr->pHeader))->slParams),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInRetryAfterHdr");
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_insertParamAtIndexInRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: This function  inserts a param at a specified index
**		in a Retry-after header
**
*********************************************************************/
SipBool sip_insertParamAtIndexInRetryAfterHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInRetryAfterHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRetryAfterDate)&&(pHdr->dType != SipHdrTypeRetryAfterSec))
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

	if( sip_listInsertAt( &(((SipRetryAfterHeader *)(pHdr->pHeader))->slParams),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInRetryAfterHdr");
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_deleteParamAtIndexInRetryAfterHdr
**********************************************************************
**
** DESCRIPTION: This function  deletes a param at a specified index
**		in a Retry-after header
**
*********************************************************************/
SipBool sip_deleteParamAtIndexInRetryAfterHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInRetryAfterHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeRetryAfterDate)&&(pHdr->dType != SipHdrTypeRetryAfterSec))
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
	if( sip_listDeleteAt( &(((SipRetryAfterHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInRetryAfterHdr");
	return SipSuccess;
}



/*********************************************************************
** FUNCTION:   sip_getChallengeFromWwwAuthenticateHdr
**********************************************************************
**
** DESCRIPTION: Gets the dChallenge field from www-authenticate pHeader.
**
*********************************************************************/
SipBool sip_getChallengeFromWwwAuthenticateHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader *hdr, SipGenericChallenge **dChallenge, SipError *err)
#else
	( SipHeader *hdr, SipGenericChallenge *dChallenge, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE

	( hdr, dChallenge, err )
	  SipHeader 		*hdr;
	  SipGenericChallenge 	**dChallenge;
	  SipError 		*err;
#else
	( hdr, dChallenge, err )
	  SipHeader 		*hdr;
	  SipGenericChallenge 	*dChallenge;
	  SipError 		*err;
#endif
#endif
{
	SipGenericChallenge 	*pTempChallenge;

	SIPDEBUGFN ( "Entering getChallengeFromWwwAuthHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (( hdr == SIP_NULL)||(dChallenge==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWwwAuthenticate)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTempChallenge = ((SipWwwAuthenticateHeader*) (hdr->pHeader) )->pChallenge;
	if ( pTempChallenge == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempChallenge->dRefCount);
	*dChallenge = pTempChallenge;
#else
	if ( __sip_cloneSipChallenge(dChallenge, pTempChallenge,err) == SipFail )
	{
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getChallengeFromWwwAutHeader");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_setChallengeInWwwAuthenticateHdr
**********************************************************************
**
** DESCRIPTION: Sets the cahllenge field in www-authenticate pHeader.
**
*********************************************************************/
SipBool sip_setChallengeInWwwAuthenticateHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SipGenericChallenge *dChallenge, SipError *err)
#else
	( hdr, dChallenge, err )
	  SipHeader 		*hdr;
	  SipGenericChallenge 	*dChallenge;
	  SipError *err;
#endif
{
	SipWwwAuthenticateHeader *temp_auth_hdr;
	SipGenericChallenge 	*pTempChallenge;

	SIPDEBUGFN ( "Entering setChallengeInWwwAuthenticateHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeWwwAuthenticate)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif

	temp_auth_hdr = (SipWwwAuthenticateHeader*)(hdr->pHeader);
	pTempChallenge = temp_auth_hdr->pChallenge;
	if (dChallenge==SIP_NULL)
	{
		if (pTempChallenge != SIP_NULL)
			sip_freeSipGenericChallenge(pTempChallenge);
		temp_auth_hdr->pChallenge = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipGenericChallenge(&pTempChallenge, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipChallenge(pTempChallenge, dChallenge, err) == SipFail)
		{
			sip_freeSipGenericChallenge(pTempChallenge);
			return SipFail;
		}
		sip_freeSipGenericChallenge(((SipWwwAuthenticateHeader *)(hdr->pHeader))->pChallenge);
		((SipWwwAuthenticateHeader *)(hdr->pHeader))->pChallenge = pTempChallenge;
#else
		if (pTempChallenge != SIP_NULL)
			sip_freeSipGenericChallenge(pTempChallenge);
		HSS_LOCKEDINCREF(dChallenge->dRefCount);
		temp_auth_hdr->pChallenge = dChallenge;
#endif
	}
	SIPDEBUGFN ( "Exiting setChallengeInWwwAuthenticateHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_getChallengeFromProxyAuthenticateHdr
**********************************************************************
**
** DESCRIPTION: Gets the dChallenge field from proxy-authenticate pHeader.
**
*********************************************************************/
SipBool sip_getChallengeFromProxyAuthenticateHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader *hdr, SipGenericChallenge **dChallenge, SipError *err)
#else
	( SipHeader *hdr, SipGenericChallenge *dChallenge, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( hdr, dChallenge, err )
	  SipHeader 		*hdr;
	  SipGenericChallenge 	**dChallenge;
	  SipError 		*err;
#else
	( hdr, dChallenge, err )
	  SipHeader 		*hdr;
	  SipGenericChallenge 	*dChallenge;
	  SipError 		*err;
#endif
#endif
{
	SipGenericChallenge 	*pTempChallenge;

	SIPDEBUGFN ( "Entering getChallengeFromProxyAutHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (( hdr == SIP_NULL)||(dChallenge ==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(hdr->dType != SipHdrTypeProxyAuthenticate)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTempChallenge = ( (SipProxyAuthenticateHeader*) (hdr->pHeader) )->pChallenge;

	if ( pTempChallenge == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempChallenge->dRefCount);
	*dChallenge = pTempChallenge;
#else
	if ( __sip_cloneSipChallenge(dChallenge, pTempChallenge	,err) == SipFail)
	{
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getChallengeFromProxyAuthenticateHeader");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_setChallengeInProxyAuthenticateHdr
**********************************************************************
**
** DESCRIPTION: Sets the dChallenge field in proxy-authenticate pHeader.
**
*********************************************************************/
SipBool sip_setChallengeInProxyAuthenticateHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SipGenericChallenge *dChallenge, SipError *err)
#else
	( hdr, dChallenge, err )
	  SipHeader 		*hdr;
	  SipGenericChallenge 	*dChallenge;
	  SipError *err;
#endif
{

	SipProxyAuthenticateHeader *temp_auth_hdr;
	SipGenericChallenge 	*pTempChallenge;

	SIPDEBUGFN ( "Entering setChallengeInProxyAuthenticateHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeProxyAuthenticate)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

#endif
	temp_auth_hdr = (SipProxyAuthenticateHeader*)(hdr->pHeader);
	pTempChallenge = temp_auth_hdr->pChallenge;
	if (dChallenge==SIP_NULL)
	{
		if (pTempChallenge !=SIP_NULL)
			sip_freeSipGenericChallenge(pTempChallenge);
			temp_auth_hdr->pChallenge = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipGenericChallenge(&pTempChallenge, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipChallenge(pTempChallenge, dChallenge, err) == SipFail)
		{
			sip_freeSipGenericChallenge(pTempChallenge);
			return SipFail;
		}
		sip_freeSipGenericChallenge(((SipProxyAuthenticateHeader *)(hdr->pHeader))->pChallenge);
		((SipProxyAuthenticateHeader *)(hdr->pHeader))->pChallenge = pTempChallenge;
#else
		if (pTempChallenge != SIP_NULL)
			sip_freeSipGenericChallenge(pTempChallenge);
		HSS_LOCKEDINCREF(dChallenge->dRefCount);
		temp_auth_hdr->pChallenge = dChallenge;
#endif
	}
	SIPDEBUGFN ( "Exiting setChallengeInProxyAuthenticateHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_getValueFromServerHdr
**********************************************************************
**
** DESCRIPTION:  Gets the value field from the Server Header
**
*********************************************************************/
SipBool sip_getValueFromServerHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppValue, SipError *pErr)
#else
	(pHdr, ppValue, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppValue;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *temp_value;

	SIPDEBUGFN ( "Entering sip_getValueFromServerHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pHdr == SIP_NULL)||( ppValue == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pHdr->dType != SipHdrTypeServer)
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
	temp_value = ( (SipServerHeader *) (pHdr->pHeader) )->pValue;

	if( temp_value == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
		*ppValue = temp_value;
#else
	dLength = strlen(temp_value);
	*ppValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppValue == SIP_NULL )
		return SipFail;

	strcpy( *ppValue , temp_value );
#endif

	SIPDEBUGFN ( "Exiting sip_getValueFromServerHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_setValueInServerHdr
**********************************************************************
**
** DESCRIPTION:  Sets the value field in the Server Header
**
*********************************************************************/
SipBool sip_setValueInServerHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pValue, SipError *pErr)
#else
	(pHdr, pValue, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pValue;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_value;
	SipServerHeader * temp_server_hdr;

	SIPDEBUGFN ( "Entering sip_setValueInServerHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pHdr->dType != SipHdrTypeServer)
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
	if( pValue == SIP_NULL)
		temp_value = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_value = pValue;
#else
		dLength = strlen( pValue );
		temp_value = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_value == SIP_NULL )
			return SipFail;

		strcpy( temp_value, pValue );
#endif
	}

	temp_server_hdr = ( SipServerHeader *) ( pHdr->pHeader);
	if ( temp_server_hdr->pValue != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(temp_server_hdr->pValue)), pErr) == SipFail)
			return SipFail;
	}


	temp_server_hdr->pValue = temp_value;

	SIPDEBUGFN ( "Exiting sip_setValueInServerHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:   sip_getOptionFromUnsupportedHdr
**********************************************************************
**
** DESCRIPTION:  Gets the option field from  the Unsupported Header
**
*********************************************************************/
SipBool sip_getOptionFromUnsupportedHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppOption, SipError *pErr)
#else
	(pHdr, ppOption, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppOption;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *temp_option;

	SIPDEBUGFN ( "Entering sip_getOptionFromUnsupportedHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ((pHdr == SIP_NULL)||(ppOption == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pHdr->dType != SipHdrTypeUnsupported)
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
	temp_option = ( (SipUnsupportedHeader *) (pHdr->pHeader) )->pOption;

	if( temp_option == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
		*ppOption = temp_option;
#else
	dLength = strlen(temp_option);
	*ppOption = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppOption == SIP_NULL )
		return SipFail;

	strcpy( *ppOption , temp_option );
#endif

	SIPDEBUGFN ( "Exiting sip_getOptionFromUnsupportedHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:    sip_setOptionInUnsupportedHdr
**********************************************************************
**
** DESCRIPTION:  Sets the option field in the Unsupported Header
**
*********************************************************************/
SipBool sip_setOptionInUnsupportedHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pOption, SipError *pErr)
#else
	(pHdr, pOption, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pOption;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_option;
	SipUnsupportedHeader * temp_unsupported_hdr;

	SIPDEBUGFN ( "Entering sip_setOptionInUnsupportedHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pHdr->dType != SipHdrTypeUnsupported)
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
	if( pOption == SIP_NULL)
		temp_option = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_option = pOption;
#else
		dLength = strlen( pOption );
		temp_option = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_option == SIP_NULL )
			return SipFail;

		strcpy( temp_option, pOption );
#endif
	}

	temp_unsupported_hdr = ( SipUnsupportedHeader *) ( pHdr->pHeader);
	if ( temp_unsupported_hdr->pOption != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(temp_unsupported_hdr->pOption)), pErr) == SipFail)
			return SipFail;
	}


	temp_unsupported_hdr->pOption = temp_option;

	SIPDEBUGFN ( "Exiting sip_setOptionInUnsupportedHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: sip_getUriFromErrorInfoHdr
**
** DESCRIPTION: This function retrives the Uri field from the
** ErrorInfo pHeader.
**
*****************************************************************/

SipBool sip_getUriFromErrorInfoHdr
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

	SIPDEBUGFN ( "Entering getUriFromErrorInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ((pHdr == SIP_NULL)||(ppUri == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	/* checking for correctness of the pHeader dType */
	if (pHdr->dType != SipHdrTypeErrorInfo)
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
	pUri = ( (SipErrorInfoHeader *) (pHdr->pHeader) )->pUri;

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

	SIPDEBUGFN ( "Exiting getUriFromErrorInfoHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: sip_setUriInErrorInfoHdr
**
** DESCRIPTION: This function sets the dOption field in the Supported
** pHeader.
**
*****************************************************************/
SipBool sip_setUriInErrorInfoHdr
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
	SIP_S8bit * pTempErrorInfo;
#endif
	SipErrorInfoHeader * pTempErrorInfoHdr;

	SIPDEBUGFN ( "Entering setUriInErrorInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	/* checking for correctness of the pHeader dType */
	if (pHdr->dType != SipHdrTypeErrorInfo)
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
	/* freeing original pValue if it exists */
	pTempErrorInfoHdr = ( SipErrorInfoHeader *) ( pHdr->pHeader);
	if ( pTempErrorInfoHdr->pUri != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pTempErrorInfoHdr->pUri)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempErrorInfoHdr->pUri = pUri;
#else
	if( pUri == SIP_NULL)
		pTempErrorInfo = SIP_NULL;
	else
	{
		pTempErrorInfo = (SIP_S8bit *) STRDUPACCESSOR ( pUri );
		if ( pTempErrorInfo == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempErrorInfoHdr->pUri = pTempErrorInfo;
#endif

	SIPDEBUGFN ( "Exiting setUriInErrorInfoHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromErrorInfoHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Encryption pHeader
**
***************************************************************/
SipBool sip_getParamCountFromErrorInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromErrorInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ((hdr == SIP_NULL)||(count==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeErrorInfo)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipErrorInfoHeader *)(hdr->pHeader))->slParam), count , err) == SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromErrorInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromErrorInfoHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP ErrorInfo Header
**
***************************************************************/
SipBool sip_getParamAtIndexFromErrorInfoHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromErrorInfoHdr");
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

	if (hdr->dType != SipHdrTypeErrorInfo)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipErrorInfoHeader *)(hdr->pHeader))->slParam), index,  \
		&element_from_list, err) == SipFail)
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromErrorInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInErrorInfoHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp ErrorInfo pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInErrorInfoHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInErrorInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeErrorInfo)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
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

	if( sip_listInsertAt( &(((SipErrorInfoHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInErrorInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInErrorInfoHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP ErrorInfo pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInErrorInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInErrorInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr->dType != SipHdrTypeErrorInfo)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}

#endif
	if( sip_listDeleteAt( &(((SipErrorInfoHeader *)(hdr->pHeader))->slParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInErrorInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInErrorInfoHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP ErrorInfo pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInErrorInfoHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInErrorInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( hdr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (hdr ->dType != SipHdrTypeErrorInfo)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (hdr->pHeader == SIP_NULL)
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

	if( sip_listSetAt( &(((SipErrorInfoHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInErrorInfoHdr");
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_getNameValuePairCountFromAuthenticationInfoHdr
**
** DESCRIPTION: This function retrieves the number of NameValuePairs from an
**		AuthenticationInfo Header
**
**
**********************************************************************/

SipBool sip_getNameValuePairCountFromAuthenticationInfoHdr
#ifdef ANSI_PROTO
	( SipHeader *pAuthInfo, SIP_U32bit *pIndex, SipError *pErr )
#else
	( pAuthInfo,pIndex,pErr)
	  SipHeader *pAuthInfo;
	  SIP_U32bit *pIndex;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_getNameValuePairCountFromAuthenticationInfoHdr \n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pAuthInfo == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pAuthInfo->dType != SipHdrTypeAuthenticationInfo )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pAuthInfo->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(((SipAuthenticationInfoHeader*)(pAuthInfo->pHeader))->slNameValue), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_getNameValuePairCountFromAuthenticationInfoHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_getNameValuePairAtIndexFromAuthenticationInfoHdr
**
** DESCRIPTION: This function retrieves NameValuePair at specified index
**		from Authentication Info Header
**
*******************************************************************/

SipBool sip_getNameValuePairAtIndexFromAuthenticationInfoHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader *pAuthInfo, SipNameValuePair **ppNameValue,\
	 SIP_U32bit index, SipError *pErr )
#else
	( SipHeader *pAuthInfo, SipNameValuePair *pNameValue,\
	 SIP_U32bit index, SipError *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pAuthInfo,ppNameValue,index,pErr)
	  SipHeader *pAuthInfo;
	  SipNameValuePair **ppNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#else
	( pAuthInfo,pNameValue,index,pErr)
	  SipHeader *pAuthInfo;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list=SIP_NULL;
	SIPDEBUGFN("Entering sip_getNameValuePairAtIndexFromAuthenticationInfoHdr\n");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

#ifdef SIP_BY_REFERENCE
	if (( pAuthInfo == SIP_NULL) || ( ppNameValue == SIP_NULL ))
#else
	if (( pAuthInfo == SIP_NULL) || ( pNameValue == SIP_NULL ))
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pAuthInfo->dType != SipHdrTypeAuthenticationInfo )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pAuthInfo->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(((SipAuthenticationInfoHeader*)(pAuthInfo->pHeader))->slNameValue), index,\
		 &element_from_list, pErr) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	if( __sip_cloneSipNameValuePair(pNameValue, \
		( SipNameValuePair* )element_from_list ,pErr) == SipFail )
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF((( SipNameValuePair* ) element_from_list)->dRefCount);
	*ppNameValue = ( SipNameValuePair* ) element_from_list;
#endif

	SIPDEBUGFN("Exiting sip_getNameValuePairAtIndexFromAuthenticationInfoHdr\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_setNameValuePairAtIndexInAuthenticationInfoHdr
**
** DESCRIPTION: This function sets NameValuePair at specified index
**		in Authentication Info Header
**
**
*******************************************************************/
SipBool sip_setNameValuePairAtIndexInAuthenticationInfoHdr
#ifdef ANSI_PROTO
	( SipHeader *pAuthInfo, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pAuthInfo,pNameValue,dIndex,pErr)
	  SipHeader *pAuthInfo;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_setNameValuePairAtIndexInAuthenticationInfoHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pAuthInfo == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pAuthInfo->dType != SipHdrTypeAuthenticationInfo )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pAuthInfo->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (pNameValue == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		sip_initSipNameValuePair(&element_in_list,pErr);
		if( __sip_cloneSipNameValuePair( element_in_list, pNameValue \
			 ,pErr) == SipFail )
        	{
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

#else
		element_in_list = pNameValue;
		HSS_LOCKEDINCREF((( SipNameValuePair* ) element_in_list)->\
		dRefCount);
#endif
	}

	if( sip_listSetAt( &(((SipAuthenticationInfoHeader*)(pAuthInfo->pHeader))->slNameValue), dIndex,   \
		 (SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_setNameValuePairAtIndexInAuthenticationInfoHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_insertNameValuePairAtIndexInAuthenticationInfoHdr
**
** DESCRIPTION: This function inserts NameValuePair at specified index
**		in AuthenticationInfo Header
**
**
*******************************************************************/
SipBool sip_insertNameValuePairAtIndexInAuthenticationInfoHdr
#ifdef ANSI_PROTO
	( SipHeader *pAuthInfo, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pAuthInfo,pNameValue,dIndex,pErr)
	  SipHeader *pAuthInfo;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_insertNameValuePairAtIndexInAuthenticationInfoHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pAuthInfo == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pAuthInfo->dType != SipHdrTypeAuthenticationInfo )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pAuthInfo->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (pNameValue == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		sip_initSipNameValuePair(&element_in_list,pErr);
		if(__sip_cloneSipNameValuePair( element_in_list, pNameValue \
			 ,pErr) == SipFail )
        	{
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

#else
		element_in_list = pNameValue;
		HSS_LOCKEDINCREF((( SipNameValuePair* ) element_in_list)->\
		dRefCount);
#endif
	}

	if( sip_listInsertAt( &(((SipAuthenticationInfoHeader*)(pAuthInfo->pHeader))->slNameValue), dIndex,   \
		 (SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_insertNameValuePairAtIndexInAuthenticationInfoHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_deleteNameValuePairAtIndexInAuthenticationInfoHdr
**
** DESCRIPTION: This function deletes NameValuePair at specified index
**		in AuthenticationInfo Header
**
**
**********************************************************************/

SipBool sip_deleteNameValuePairAtIndexInAuthenticationInfoHdr
#ifdef ANSI_PROTO
	( SipHeader *pAuthInfo, SIP_U32bit dIndex, SipError *pErr )
#else
	( pAuthInfo,dIndex,pErr)
	  SipHeader *pAuthInfo;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering sip_deleteNameValuePairAtIndexInAuthenticationInfoHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pAuthInfo == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pAuthInfo->dType != SipHdrTypeAuthenticationInfo )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pAuthInfo->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipAuthenticationInfoHeader*)(pAuthInfo->pHeader))->slNameValue), dIndex, pErr) == \
	SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting sip_deleteNameValuePairAtIndexInAuthenticationInfoHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

#ifdef SIP_3GPP
/*********************Accessor functions for 3GPP headers(RFC 3455)*********/
/*********************************************************
** FUNCTION:sip_getDispNameFromPAssociatedUriHdr
**
** DESCRIPTION: This function retrieves the display-pName field
**		from a SIP PAssociatedUri pHeader
**
**********************************************************/
SipBool sip_getDispNameFromPAssociatedUriHdr
	(SipHeader *pHdr,
	SIP_S8bit **ppDispname,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_dispname;
	SIPDEBUGFN( "Entering getDispNameFromPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getDispNameFromPAssociatedUriHdr");
		return SipFail;
	}
	if ( ppDispname == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getDispNameFromPAssociatedUriHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting getDispNameFromPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting getDispNameFromPAssociatedUriHdr");
		return SipFail;
	}
#endif
	pTemp_dispname = ( (SipPAssociatedUriHeader *) (pHdr->pHeader) )->pDispName;


	if( pTemp_dispname == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting getDispNameFromPAssociatedUriHdr");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppDispname = pTemp_dispname;
#else

	dLength = strlen(pTemp_dispname );
	*ppDispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppDispname == SIP_NULL )
    {
		*pErr = E_NO_MEM;
	    SIPDEBUGFN( "Exiting getDispNameFromPAssociatedUriHdr");
		return SipFail;
    }

	strcpy( *ppDispname, pTemp_dispname );
#endif

	SIPDEBUGFN( "Exiting getDispNameFromPAssociatedUriHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setDispNameInPAssociatedUriHdr
**
** DESCRIPTION: This function sets the display-pName field in
**		a SIP PAssociatedUri pHeader
**
**********************************************************/
SipBool sip_setDispNameInPAssociatedUriHdr
(SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_dispname;
#endif
	SipPAssociatedUriHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setDispNameInPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting setDispNameInPAssociatedUriHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting setDispNameInPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
	
        *pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting setDispNameInPAssociatedUriHdr");
		return SipFail;
	}
#endif
	pTemp_hdr=(SipPAssociatedUriHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pDispName !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pDispName),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipPAssociatedUriHeader *)(pHdr->pHeader))->pDispName = pDispname;
#else
	if( pDispname == SIP_NULL)
		pTemp_dispname = SIP_NULL;
	else
	{
		dLength = strlen( pDispname );
		pTemp_dispname = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_dispname == SIP_NULL )
        {
		    *pErr = E_NO_MEM;
	        SIPDEBUGFN( "Exiting setDispNameInPAssociatedUriHdr");
    		return SipFail;
        }

		strcpy( pTemp_dispname, pDispname );
	}
	pTemp_hdr->pDispName = pTemp_dispname;
#endif
	SIPDEBUGFN( "Exiting setDispNameInPAssociatedUriHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getAddrSpecFromPAssociatedUriHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field
**		from a SIP PAssociatedUri pHeader
**
**********************************************************/
SipBool sip_getAddrSpecFromPAssociatedUriHdr
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
	SIPDEBUGFN( "Entering getAddrSpecFromPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;


	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPAssociatedUriHdr");
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	if ( ppAddrspec == SIP_NULL)
#else
	if ( pAddrspec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPAssociatedUriHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPAssociatedUriHdr");
		return SipFail;
	}
#endif
	pFrom=((SipPAssociatedUriHeader *)(pHdr->pHeader))->pAddrSpec;
	if (pFrom == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting getAddrSpecFromPAssociatedUriHdr");
		return SipFail;

	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pFrom->dRefCount);
	*ppAddrspec = pFrom;
#else
	if(__sip_cloneAddrSpec(pAddrspec,pFrom,pErr)==SipFail)
	{
		sip_freeSipAddrSpec(pAddrspec);
/*		if(pAddrspec->dType==SipAddrReqUri)
		{
			sip_freeString(pAddrspec->u.pUri);
		}
		else if((pAddrspec->dType==SipAddrSipUri) \
				|| (pAddrspec->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl(pAddrspec->u.pSipUrl);
		}
		pAddrspec->dType=SipAddrAny;
*/
	    SIPDEBUGFN( "Exiting getAddrSpecFromPAssociatedUriHdr");
		return SipFail;
	}
#endif
	SIPDEBUGFN( "Exiting getAddrSpecFromPAssociatedUriHdr");

	*pErr=E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setAddrSpecInPAssociatedUriHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a
**		SIP PAssociatedUri Header
**
**********************************************************/
SipBool sip_setAddrSpecInPAssociatedUriHdr
	(SipHeader *pHdr,
	SipAddrSpec *pAddrspec,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTo;
#endif
	SIPDEBUGFN( "Entering setAddrSpecInPAssociatedUriHdr ");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting setAddrSpecInPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting setAddrSpecInPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting setAddrSpecInPAssociatedUriHdr");
		return SipFail;
	}
#endif
	sip_freeSipAddrSpec(((SipPAssociatedUriHeader *)(pHdr->pHeader))->pAddrSpec );
	if (pAddrspec==SIP_NULL)
	{
		((SipPAssociatedUriHeader *)(pHdr->pHeader))->pAddrSpec=SIP_NULL; 		
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pAddrspec->dRefCount);
		((SipPAssociatedUriHeader *)(pHdr->pHeader))->pAddrSpec = pAddrspec;
#else
		if(sip_initSipAddrSpec(&pTo,SipAddrAny,pErr)==SipFail)
		{
	        SIPDEBUGFN( "Exiting setAddrSpecInPAssociatedUriHdr");
			return SipFail;
		}
		if(__sip_cloneAddrSpec(pTo,pAddrspec,pErr)==SipFail)
		{
			sip_freeSipAddrSpec(pTo);
	        SIPDEBUGFN( "Exiting setAddrSpecInPAssociatedUriHdr");
			return SipFail;
		}
		((SipPAssociatedUriHeader *)(pHdr->pHeader))->pAddrSpec = pTo;
#endif
	}	
	*pErr=E_NO_ERROR;
	SIPDEBUGFN( "Exiting setAddrSpecInPAssociatedUriHdr");
	return SipSuccess;
}
/*********************************************************
** FUNCTION: sip_getParamCountFromPAssociatedUriHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**		from a SIP PAssociatedUr Header
**
**********************************************************/
SipBool sip_getParamCountFromPAssociatedUriHdr
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_getParamCountFromPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPAssociatedUriHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPAssociatedUriHdr");
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPAssociatedUriHdr");
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipPAssociatedUriHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPAssociatedUriHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromPAssociatedUriHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamAtIndexFromPAssociatedUriHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**		index in a PAssociatedUri header
**
**********************************************************/
SipBool sip_getParamAtIndexFromPAssociatedUriHdr
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPAssociatedUriHdr");
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPAssociatedUriHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPAssociatedUriHdr");
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipPAssociatedUriHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPAssociatedUriHdr");
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPAssociatedUriHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInPAssociatedUriHdr
**
** DESCRIPTION: This function sets a param at a specified
**		index in a PAssociatedUri header
**
**********************************************************/
SipBool sip_setParamAtIndexInPAssociatedUriHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPAssociatedUriHdr");
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPAssociatedUriHdr");
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
	        SIPDEBUGFN("Exiting function sip_setParamAtIndexInPAssociatedUriHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipPAssociatedUriHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPAssociatedUriHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInPAssociatedUriHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_insertParamAtIndexInPAssociatedUriHdr
**
** DESCRIPTION: This function inserts a param at a specified
**		index in a PAssociatedUri header
**
**********************************************************/
SipBool sip_insertParamAtIndexInPAssociatedUriHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePAssociatedUri))
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

	if( sip_listInsertAt( &(((SipPAssociatedUriHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPAssociatedUriHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPAssociatedUriHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInPAssociatedUriHdr
**
** DESCRIPTION: This function deletes a param at a specified
**		index in a PAssociatedUri header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInPAssociatedUriHdr
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPAssociatedUriHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPAssociatedUriHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePAssociatedUri))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPAssociatedUriHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPAssociatedUriHdr");
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipPAssociatedUriHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
    {
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPAssociatedUriHdr");
		return SipFail;
    }

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPAssociatedUriHdr");
	return SipSuccess;
}

#endif /*#ifdef SIP_3GPP */

#ifdef SIP_CONGEST
/*****Accessor functions for CONGEST headers(Accept-Resource-Priority)****/
/*********************************************************
** FUNCTION:sip_getNamespaceFromAcceptRsrcPriorityHdr
**
** DESCRIPTION: This function retrieves the Namespace field
**		from a SIP AcceptRsrcPriority Header
**
**********************************************************/
SipBool sip_getNamespaceFromAcceptRsrcPriorityHdr 
	(SipHeader *pHdr,
	SIP_S8bit **ppNamespace,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_namespace;
	SIPDEBUGFN( "Entering  sip_getNamespaceFromAcceptRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromAcceptRsrcPriorityHdr");
		return SipFail;
	}
	if ( ppNamespace == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromAcceptRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeAcceptRsrcPriority))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromAcceptRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromAcceptRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_namespace = ( (SipAcceptRsrcPriorityHeader *) (pHdr->pHeader) )->pNamespace;


	if( pTemp_namespace == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromAcceptRsrcPriorityHdr");
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
	    SIPDEBUGFN( "Exiting  sip_getNamespaceFromAcceptRsrcPriorityHdr");
		return SipFail;
	}

	strcpy( *ppNamespace, pTemp_namespace );
#endif

	SIPDEBUGFN( "Exiting  sip_getNamespaceFromAcceptRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getPriorityFromAcceptRsrcPriorityHdr
**
** DESCRIPTION: This function retrieves the Priority field
**		from a SIP AcceptRsrcPriority Header
**
**********************************************************/
SipBool sip_getPriorityFromAcceptRsrcPriorityHdr 
	(SipHeader *pHdr,
	SIP_S8bit **ppPriority,
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_priority;
	SIPDEBUGFN( "Entering  sip_getPriorityFromAcceptRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting  sip_getPriorityFromAcceptRsrcPriorityHdr");
		return SipFail;
	}
	if ( ppPriority == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN( "Exiting  sip_getPriorityFromAcceptRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeAcceptRsrcPriority))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN( "Exiting  sip_getPriorityFromAcceptRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN( "Exiting  sip_getPriorityFromAcceptRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_priority = ( (SipAcceptRsrcPriorityHeader *) (pHdr->pHeader) )->pPriority;


	if( pTemp_priority == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
	    SIPDEBUGFN( "Exiting  sip_getPriorityFromAcceptRsrcPriorityHdr");
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
	    SIPDEBUGFN( "Exiting  sip_getPriorityFromAcceptRsrcPriorityHdr");
		return SipFail;
	}

	strcpy( *ppPriority, pTemp_priority);
#endif

	SIPDEBUGFN( "Exiting  sip_getPriorityFromAcceptRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setNamespaceInAcceptRsrcPriorityHdr
**
** DESCRIPTION: This function sets the Namespace field in
**		a SIP AcceptRsrcPriorityHdr Header
**
**********************************************************/
SipBool sip_setNamespaceInAcceptRsrcPriorityHdr
(SipHeader *pHdr,SIP_S8bit *pNamespace, SipError *pErr)

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_namespace;
#endif
	SipAcceptRsrcPriorityHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setNamespaceInAcceptRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN( "Exiting sip_setNamespaceInAcceptRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeAcceptRsrcPriority))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN( "Exiting sip_setNamespaceInAcceptRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN( "Exiting sip_setNamespaceInAcceptRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_hdr=(SipAcceptRsrcPriorityHeader*)(pHdr->pHeader);
	if(pTemp_hdr->pNamespace !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pNamespace),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipAcceptRsrcPriorityHeader *)(pHdr->pHeader))->pNamespace = pNamespace;
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
            SIPDEBUGFN( "Exiting sip_setNamespaceInAcceptRsrcPriorityHdr");
			return SipFail;
        }

		strcpy( pTemp_namespace, pNamespace);
	}
	pTemp_hdr->pNamespace= pTemp_namespace;
#endif
	SIPDEBUGFN( "Exiting sip_setNamespaceInAcceptRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setPriorityInAcceptRsrcPriorityHdr
**
** DESCRIPTION: This function sets the Priority field in
**		a SIP AcceptRsrcPriorityHdr Header
**
**********************************************************/
SipBool sip_setPriorityInAcceptRsrcPriorityHdr
(SipHeader *pHdr,SIP_S8bit *pPriority, SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_priority;
#endif
	SipAcceptRsrcPriorityHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setPriorityInAcceptRsrcPriorityHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN( "Exiting sip_setPriorityInAcceptRsrcPriorityHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypeAcceptRsrcPriority))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN( "Exiting sip_setPriorityInAcceptRsrcPriorityHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN( "Exiting sip_setPriorityInAcceptRsrcPriorityHdr");
		return SipFail;
	}
#endif
	pTemp_hdr=(SipAcceptRsrcPriorityHeader*)(pHdr->pHeader);
	if(pTemp_hdr->pPriority != SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pPriority),pErr) ==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	((SipAcceptRsrcPriorityHeader *)(pHdr->pHeader))->pPriority= pPriority;
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
            SIPDEBUGFN( "Exiting sip_setPriorityInAcceptRsrcPriorityHdr");
			return SipFail;
        }

		strcpy( pTemp_priority, pPriority);
	}
	pTemp_hdr->pPriority= pTemp_priority;
#endif
	SIPDEBUGFN( "Exiting sip_setPriorityInAcceptRsrcPriorityHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

#ifdef SIP_SECURITY
/*********************************************************
** FUNCTION:sip_getMechanismNameFromSecurityServerHdr
**
** DESCRIPTION: This function retrieves the mechanism-name field
**              from a SIP SecurityServer pHeader
**
**********************************************************/

SipBool sip_getMechanismNameFromSecurityServerHdr
        (SipHeader *pHdr,
        SIP_S8bit **ppMechname,
        SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
        SIP_U16bit dLength;
#endif
        SIP_S8bit * pTemp_Mechname;
        SIPDEBUGFN( "Entering getMechanismNameFromSecurityServerHdr");
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

        if ((pHdr->dType != SipHdrTypeSecurityServer))
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
        pTemp_Mechname = ( (SipSecurityServerHeader *) (pHdr->pHeader) )->pMechanismName;


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

        SIPDEBUGFN( "Exiting getMechanismNameFromSecurityServerHdr");
        *pErr = E_NO_ERROR;
        return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setMechanismNameInSecurityServerHdr
**
** DESCRIPTION: This function sets the mechanism-name field in
**              a SIP Security-Server pHeader
**
**********************************************************/
SipBool sip_setMechanismNameInSecurityServerHdr
	(SipHeader *pHdr,
	SIP_S8bit *pMechname, 
	SipError *pErr)
{
#ifndef SIP_BY_REFERENCE
        SIP_U32bit dLength;
        SIP_S8bit * pTemp_Mechname;
#endif
        SipSecurityServerHeader *pTemp_hdr;
        SIPDEBUGFN( "Entering setMechanismNameInSecurityServerHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityServer))
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
        pTemp_hdr=(SipSecurityServerHeader *)(pHdr->pHeader);
        if(pTemp_hdr->pMechanismName !=SIP_NULL)
        {
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pMechanismName),pErr) ==SipFail)
                	return SipFail;
        }
#ifdef SIP_BY_REFERENCE
        ((SipSecurityServerHeader *)(pHdr->pHeader))->pMechanismName = pMechname;
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
        SIPDEBUGFN( "Exiting setMechanismNameInSecurityServerHdr");
        *pErr = E_NO_ERROR;
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_getParamCountFromSecurityServerHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**              from a SIP SecurityServer pHeader
**
**********************************************************/
SipBool sip_getParamCountFromSecurityServerHdr
        (SipHeader *pHdr, 
	SIP_U32bit *pCount, 
	SipError *pErr)
{
        SIPDEBUGFN("Entering function sip_getParamCountFromSecurityServerHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityServer))
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

        if (sip_listSizeOf( &(((SipSecurityServerHeader *)(pHdr->pHeader))->slParams),pCount , pErr) == SipFail )
        {
                return SipFail;
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getParamCountFromSecurityServerHdr");
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_getParamAtIndexFromSecurityServerHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**              index in a Security-Server header
**
**********************************************************/
SipBool sip_getParamAtIndexFromSecurityServerHdr
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
        SIPDEBUGFN("Entering function sip_getParamAtIndexFromSecurityServerHdr");
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

        if ((pHdr->dType != SipHdrTypeSecurityServer))
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
      if (sip_listGetAt( &(((SipSecurityServerHeader *)(pHdr->pHeader))->slParams), dIndex,  
\
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
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromSecurityServerHdr");
        return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInSecurityServerHdr
**
** DESCRIPTION: This function sets a param at a specified
**              index in a Security-Server header
**
**********************************************************/
SipBool sip_setParamAtIndexInSecurityServerHdr
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_setParamAtIndexInSecurityServerHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr ->dType != SipHdrTypeSecurityServer))
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

        if( sip_listSetAt( &(((SipSecurityServerHeader *)(pHdr->pHeader))->slParams),  \
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
        SIPDEBUGFN("Exiting function sip_setParamAtIndexInSecurityServerHdr");
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_insertParamAtIndexInSecurityServerHdr
**
** DESCRIPTION: This function inserts a param at a specified
**              index in a Security-Server header
**
**********************************************************/
SipBool sip_insertParamAtIndexInSecurityServerHdr
        (SipHeader *pHdr, 
	SipParam *pParam, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SipParam *temp_param;
        SIPDEBUGFN("Entering function sip_insertParamAtIndexInSecurityServerHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr ->dType != SipHdrTypeSecurityServer))
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

        if( sip_listInsertAt( &(((SipSecurityServerHeader *)(pHdr->pHeader))->slParams),  \
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
        SIPDEBUGFN("Exiting function sip_insertParamAtIndexInSecurityServerHdr");
        return SipSuccess;
}

/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInSecurityServerHdr
**
** DESCRIPTION: This function deletes a param at a specified
**              index in a Security-Server header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInSecurityServerHdr
        (SipHeader *pHdr, 
	SIP_U32bit dIndex, 
	SipError *pErr)
{
        SIPDEBUGFN("Entering function sip_deleteParamAtIndexInSecurityServerHdr");
#ifndef SIP_NO_CHECK
        if( pErr == SIP_NULL )
                return SipFail;

        if ( pHdr == SIP_NULL )
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ((pHdr->dType != SipHdrTypeSecurityServer))
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
	if( sip_listDeleteAt( &(((SipSecurityServerHeader *)(pHdr->pHeader))->slParams),dIndex, pErr) == SipFail)
                return SipFail;

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInSecurityServerHdr");
        return SipSuccess;
}

#endif	/* end of #ifdef SIP_SECURITY	*/


