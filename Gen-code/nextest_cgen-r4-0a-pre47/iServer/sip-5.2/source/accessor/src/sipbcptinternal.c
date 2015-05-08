/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	sipbcptinternal.c
**
** DESCRIPTION:
**  	
**
** DATE    		  NAME      	    REFERENCE      REASON
** ----     	  ----          	---------      ------
** 10/02/00   	B. Borthakur	  	 Creation
**
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "sipbcptinit.h"
#include "sipbcptfree.h"
#include "siplist.h"
#include "sipinternal.h"
#include "sipbcptinternal.h"
#include "sipvalidate.h"
#include "sipdecode.h"
#include "sipdecodeintrnl.h"
#include "sipclone.h"
#include "sipparserinc.h"


/********************************************************************
** FUNCTION:sip_bcpt_cloneMimeMessage
**
** DESCRIPTION: This function does a deep copy of all the fields 
** of pSource structure to pDest structure. 
**
*********************************************************************/

SipBool sip_bcpt_cloneMimeMessage
#ifdef ANSI_PROTO
	(MimeMessage *pDest, MimeMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	MimeMessage *pDest;
	MimeMessage *pSource;
	SipError *pErr;
#endif
{	
	SIP_U32bit index, count;
	SipMsgBody	*pMsgB, *pDupMsgB;

	SIPDEBUGFN("Entering function sip_bcpt_cloneMimeMessage");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
	{
		return SipFail;
	}
	if ( pDest == SIP_NULL || pSource == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying siplist of Msg Body */
	if ( sip_listDeleteAll(&( pDest->slRecmimeBody ), pErr ) == SipFail )
		return SipFail;
	if ( sip_listInit(& (pDest->slRecmimeBody ),\
				(pSource->slRecmimeBody).freefunc,pErr)==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( pSource->slRecmimeBody ), &count, pErr) == SipFail )
		return SipFail;

	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(&( pSource->slRecmimeBody ),index,\
								(SIP_Pvoid * ) (&pMsgB), pErr) == SipFail )
			return SipFail;

		if ( pMsgB == SIP_NULL )
			pDupMsgB = SIP_NULL;
		else
		{
			sip_initSipMsgBody(&pDupMsgB,pMsgB->dType,pErr);

			if ( pDupMsgB == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipMsgBody( pDupMsgB, pMsgB, pErr) == SipFail )
			{
				sip_freeSipMsgBody(pDupMsgB);
				return SipFail;
			}
		}

		if ( sip_listAppend(&( pDest->slRecmimeBody ), pDupMsgB, pErr)\
																== SipFail )
		{
			if ( pDupMsgB != SIP_NULL )
				sip_freeSipMsgBody(pDupMsgB);
			return SipFail;
		}
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_bcpt_cloneSipMimeMessage");
	return SipSuccess;	
}

/******************************************************************
** FUNCTION:sip_bcpt_cloneIsupMessage
**
** DESCRIPTION:  This function does a deep copy of all the fields 
** of pSource structure to pDest structure. 
**
********************************************************************/
SipBool sip_bcpt_cloneIsupMessage
#ifdef ANSI_PROTO
	(IsupMessage *pDest, IsupMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	IsupMessage *pDest;
	IsupMessage *pSource;
	SipError *pErr;
#endif
{	
	SIPDEBUGFN("Entering function sip_bcpt_cloneIsupMessage");
	
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
	{
		return SipFail;
	}
	if ( pDest == SIP_NULL || pSource == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	/* cleaning up the destination structure */
	if ( pDest->pBody != SIP_NULL )
		sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) (&pDest->pBody), pErr);

	/* duplicating */
	pDest->dLength = (pSource->dLength);
	if ( pSource->pBody == SIP_NULL )
		pDest->pBody = SIP_NULL;
	else
	{
		pDest->pBody = \
			(SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID,pSource->dLength,pErr);
		if(pDest->pBody == SIP_NULL)
			return SipFail;
		memcpy(pDest->pBody,pSource->pBody,pSource->dLength);	
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_bcpt_cloneIsupMessage");
	return SipSuccess;	
}

/*******************************************************************
** FUNCTION:sip_bcpt_cloneQsigMessage
**
** DESCRIPTION:  This function does a deep copy of all the fields 
** of pSource structure to pDest structure. 
**
********************************************************************/
SipBool sip_bcpt_cloneQsigMessage
#ifdef ANSI_PROTO
	(QsigMessage *pDest, QsigMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	QsigMessage *pDest;
	QsigMessage *pSource;
	SipError *pErr;
#endif
{	
	SIPDEBUGFN("Entering function sip_bcpt_cloneQsigMessage");
	
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
	{
		return SipFail;
	}
	if ( pDest == SIP_NULL || pSource == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	/* cleaning up the destination structure */
	if ( pDest->pBody != SIP_NULL )
		sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) (&pDest->pBody), pErr);

	/* duplicating */
	pDest->dLength = (pSource->dLength);
	if ( pSource->pBody == SIP_NULL )
		pDest->pBody = SIP_NULL;
	else
	{
		pDest->pBody = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, \
			pSource->dLength,pErr);
		if(pDest->pBody == SIP_NULL)
			return SipFail;
		strncpy(pDest->pBody,pSource->pBody,pSource->dLength);	
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_bcpt_cloneQsigMessage");
	return SipSuccess;	
}

/*****************************************************************
** FUNCTION:__sip_bcpt_cloneSipMimeVersionHeader
**
** DESCRIPTION:  This function does a deep copy of all the fields 
** of pSource structure to pDest structure.
**
******************************************************************/
SipBool __sip_bcpt_cloneSipMimeVersionHeader
#ifdef ANSI_PROTO
	(SipMimeVersionHeader *pDest, SipMimeVersionHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipMimeVersionHeader *pDest;
	SipMimeVersionHeader *pSource;
	SipError *pErr;
#endif
{	
	SIPDEBUGFN("Entering function __sip_bcpt_cloneSipMimeVersionHeader");
	
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	/* cleaning up the destination structure */
	if ( pDest->pVersion != SIP_NULL )
		sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) (&pDest->pVersion), pErr);

	/* duplicating */
	if ( pSource->pVersion == SIP_NULL )
		pDest->pVersion = SIP_NULL;
	else
	{
		pDest->pVersion = (SIP_S8bit *)STRDUPACCESSOR(pSource->pVersion);
		if ( pDest->pVersion == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}		
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_bcpt_cloneSipMimeVersionHeader");
	return SipSuccess;	
}
/******************************************************************
** FUNCTION:__sip_bcpt_cloneSipMimeHeader
**
** DESCRIPTION:  This function does a deep copy of all the fields 
** of pSource structure to pDest structure.
**
******************************************************************/
SipBool __sip_bcpt_cloneSipMimeHeader
#ifdef ANSI_PROTO
	(SipMimeHeader *pDest, SipMimeHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipMimeHeader *pDest;
	SipMimeHeader *pSource;
	SipError *pErr;
#endif
{	
	SIP_S8bit 	*pMime, *pDupMime;
	SIP_U32bit 	index, count;
	SipError 	tempErr;
	SIP_S8bit 	mem_flag=0;

	SIPDEBUGFN("Entering function __sip_bcpt_cloneSipMimeHeader");
	
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	
	do{

	/* cleaning up the destination structure */

	/* content tranfer pEncoding */
	if ( pDest->pContentTransEncoding != SIP_NULL )
		sip_memfree(ACCESSOR_MEM_ID,\
						(SIP_Pvoid *) (&pDest->pContentTransEncoding), pErr);
	/* content id */
	if ( pDest->pContentId != SIP_NULL )
		sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) (&pDest->pContentId), pErr);

	/* content description */
	if ( pDest->pContentDescription != SIP_NULL )
		sip_memfree(ACCESSOR_MEM_ID,\
							(SIP_Pvoid *) (&pDest->pContentDescription), pErr);

	/* content dType */
	if ( pDest->pContentType != SIP_NULL )
	{
		sip_freeSipContentTypeHeader(pDest->pContentType);
		pDest->pContentType = SIP_NULL;
	}
	/* content Disposition */
	if ( pDest->pContentType != SIP_NULL )
	{
		sip_freeSipContentDispositionHeader(pDest->pContentDisposition);
		pDest->pContentDisposition = SIP_NULL;
	}


	/* Additional MimeHeaders */
	if ( sip_listDeleteAll( &(pDest->slAdditionalMimeHeaders), pErr) == SipFail)
		return SipFail;


	/* duplication begins here  */

	/* contetn transfer pEncoding */
	if ( pSource->pContentTransEncoding == SIP_NULL )
		pDest->pContentTransEncoding = SIP_NULL;
	else
	{
		pDest->pContentTransEncoding = \
					(SIP_S8bit *)STRDUPACCESSOR(pSource->pContentTransEncoding);
		if ( pDest->pContentTransEncoding == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			{mem_flag=1;break;}
		}		
	}

	/* content id */
	if ( pSource->pContentId == SIP_NULL )
		pDest->pContentId = SIP_NULL;
	else
	{
		pDest->pContentId = (SIP_S8bit *)STRDUPACCESSOR(pSource->pContentId);
		if ( pDest->pContentId == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			{mem_flag=1;break;}
		}		
	}

	/* content description */
	if ( pSource->pContentDescription == SIP_NULL )
		pDest->pContentDescription = SIP_NULL;
	else
	{
		pDest->pContentDescription = \
					(SIP_S8bit *)STRDUPACCESSOR(pSource->pContentDescription);
		if ( pDest->pContentDescription == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			{mem_flag=1;break;}
		}		
	}

	/* sontent tyep */
	if ( pSource->pContentType == SIP_NULL )
		pDest->pContentType = SIP_NULL;
	else
	{
		if ( sip_initSipContentTypeHeader\
						(&(pDest->pContentType), pErr)==SipFail)
			return SipFail;

		if ( __sip_cloneSipContentTypeHeader\
				(pDest->pContentType, pSource->pContentType, pErr) == SipFail)
		{
			sip_freeSipContentTypeHeader(pDest->pContentType);
			return SipFail;
		}
	}
	/* Content Disposition */
        if ( pSource->pContentDisposition == SIP_NULL )
                pDest->pContentDisposition = SIP_NULL;
        else
        {
                if ( sip_initSipContentDispositionHeader\
						(&(pDest->pContentDisposition), pErr)==SipFail)
                        return SipFail;

                if ( __sip_cloneSipContentDispositionHeader\
					(pDest->pContentDisposition, pSource->pContentDisposition,\
						pErr) == SipFail)
                {
                        sip_freeSipContentDispositionHeader\
												(pDest->pContentDisposition);
                        return SipFail;
                }
        }


	/* copying siplist of additional mime headers */
	if ( sip_listInit(& (pDest->slAdditionalMimeHeaders ), \
		(pSource->slAdditionalMimeHeaders).freefunc,pErr)==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( pSource->slAdditionalMimeHeaders ), &count, pErr)\
			== SipFail )
		return SipFail;

	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(&( pSource->slAdditionalMimeHeaders ),index,  \
			(SIP_Pvoid * ) (&pMime), pErr) == SipFail )
			return SipFail;

		if ( pMime == SIP_NULL )
			pDupMime = SIP_NULL;
		else
		{
			pDupMime = (SIP_S8bit*)STRDUPACCESSOR(pMime);
			if ( pDupMime == SIP_NULL )
			{
				*pErr = E_NO_MEM;
				{mem_flag=1;break;}
			}
			
		}

		if ( sip_listAppend(&( pDest->slAdditionalMimeHeaders ),\
												pDupMime, pErr) == SipFail )
		{
			if ( pDupMime != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pDupMime), &tempErr);
			return SipFail;
		}
	}
	if( mem_flag ) break;
	}while(0);	
	
	if( mem_flag ) 
	{
		sip_bcpt_freeSipMimeHeader(pDest);
		return SipFail;
	}


	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_bcpt_cloneSipMimeHeader");
	return SipSuccess;	
}

/******************************************************************
** FUNCTION:sip_bcpt_cloneSipMimeHeader
**
** DESCRIPTION:  This function does a deep copy of all the fields 
** of pSource structure to pDest structure.
**
******************************************************************/
SipBool sip_bcpt_cloneSipMimeHeader
#ifdef ANSI_PROTO
	(SipMimeHeader *pDest, SipMimeHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipMimeHeader *pDest;
	SipMimeHeader *pSource;
	SipError *pErr;
#endif
{	
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
	{
		return SipFail;
	}
	if ( pDest == SIP_NULL || pSource == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(__sip_bcpt_cloneSipMimeHeader(pDest,pSource,pErr) == SipFail)
		return SipFail;
	return SipSuccess;	
}
