/***********************************************************
** FUNCTION:
** This file implements the Message Waiting Msg Body  accessor APIs.
**
*************************************************************
**
** FILENAME:
	mesgsummary.c
**
** DESCRIPTION
**
**
**   DATE              NAME                      REFERENCE
** --------           ------                     -----------
** 9/1/02	     Sasidhar P V K	         Initial Version
**
** Copyright 2002, Hughes Software Systems, Ltd.
*************************************************************/
#ifdef SIP_MWI
#include "sipfree.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "mesgsummary.h"
#include "sipinternal.h"
#include "sipclone.h"
#include "sipcommon.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siplist.h"
#include "string.h"

/********************************************************************
** FUNCTION:sip_mwi_getMesgSummaryFromMsgBody
**
** DESCRIPTION: This Function retrieves Message Summary From Message Body
**
**********************************************************************/
SipBool sip_mwi_getMesgSummaryFromMsgBody
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SipMsgBody *pMsg, MesgSummaryMessage *pMwi, SipError *pErr)
#else
	( SipMsgBody *pMsg, MesgSummaryMessage **ppMwi, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	( pMsg, pMwi, pErr )
	  SipMsgBody *pMsg;
	  MesgSummaryMessage *pMwi;
	  SipError *pErr;
#else
	( pMsg, ppMwi, pErr )
	  SipMsgBody *pMsg;
	  MesgSummaryMessage **ppMwi;
	  SipError *pErr;
#endif
#endif
{
	SIPDEBUGFN("Entering sip_mwi_getMesgSummaryFromMsgBody\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	if ( ( pMsg == SIP_NULL ) || ( pMwi == SIP_NULL ) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#else
	if ( ( pMsg == SIP_NULL ) || ( ppMwi == SIP_NULL ) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pMsg->dType != SipMessageSummaryBody )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( pMsg->u.pSummaryMessage == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if(__sip_mwi_cloneMesgSummaryMessage(pMwi,pMsg->u.pSummaryMessage,pErr) == SipFail)
	{
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF((pMsg->u.pSummaryMessage)->dRefCount);
	*ppMwi = (pMsg->u).pSummaryMessage;
#endif

	SIPDEBUGFN("exiting sip_mwi_getMesgSummaryFromMsgBody ");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_mwi_setMesgSummaryInMsgBody
**
** DESCRIPTION:This function sets the Message Summary in Msg Body
**
**********************************************************************/

SipBool sip_mwi_setMesgSummaryInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *pMsg, MesgSummaryMessage *pMwi, SipError *pErr)
#else
	( pMsg, pMwi, pErr )
	  SipMsgBody *pMsg;
	  MesgSummaryMessage *pMwi;
	  SipError *pErr;
#endif
{
	MesgSummaryMessage	*pTempMwi=SIP_NULL;

	SIPDEBUGFN("Entering sip_mwi_setMesgSummaryInMsgBody\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		return SipFail;
	}

	if (pMsg == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if( pMsg->dType!=SipMessageSummaryBody )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	
	if (pMwi!=SIP_NULL)	
	{	
#ifndef SIP_BY_REFERENCE
		if ( sip_mwi_initMesgSummaryMessage(&pTempMwi, pErr) == SipFail)
			return SipFail;

		if ( __sip_mwi_cloneMesgSummaryMessage(pTempMwi, pMwi, pErr) == SipFail)
		{
			sip_mwi_freeMesgSummaryMessage( pTempMwi );
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pMwi->dRefCount);
		pTempMwi = pMwi;
#endif
	}	
	if ( pMsg->u.pSummaryMessage != SIP_NULL)
		sip_mwi_freeMesgSummaryMessage( pMsg->u.pSummaryMessage);

	pMsg->u.pSummaryMessage = pTempMwi;

	SIPDEBUGFN("Leaving sip_mwi_setMesgSummaryInMsgBody\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION: sip_mwi_getStatusFromMesgSummaryMessage
**
** DESCRIPTION: This functon retrieves the status field from the
**		Mesg Summary Body
**
**********************************************************************/
SipBool sip_mwi_getStatusFromMesgSummaryMessage
#ifdef ANSI_PROTO
	(MesgSummaryMessage *pMsgSummary, en_StatusType *pStatusType,
	 SipError *pErr)
#else
	( pMsgSummary, pStatusType, pErr )
	MesgSummaryMessage *pMsgSummary;
	en_StatusType* pStatusType;
	SipError *pErr;

#endif
{
	SIPDEBUGFN("Entering sip_mwi_getStatusFromMesgSummaryMessage\n");
#ifndef SIP_NO_CHECK

	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pMsgSummary == SIP_NULL) || ( pStatusType == SIP_NULL ))

	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pStatusType  =  pMsgSummary->dStatus;
	SIPDEBUGFN("Exiting sip_mwi_getStatusFromMesgSummaryMessage\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}
/********************************************************************
** FUNCTION: sip_mwi_setStatusInMesgSummaryMessage
**
** DESCRIPTION: This functon sets the status field in the
**		Mesg Summary Body
**
**********************************************************************/
SipBool sip_mwi_setStatusInMesgSummaryMessage
#ifdef ANSI_PROTO
	(MesgSummaryMessage *pMsgSummary, en_StatusType dStatusType,
	 SipError *pErr)
#else
	( pMsgSummary, dStatusType, pErr )
	MesgSummaryMessage *pMsgSummary;
	en_StatusType dStatusType;
	SipError *pErr;

#endif
{
	SIPDEBUGFN("Entering sip_mwi_setStatusInMesgSummaryMessage\n");
	if(( dStatusType != SipMsgWaitingYes ) && \
		( dStatusType != SipMsgWaitingNo ))
	  {
		*pErr = E_INV_TYPE;
		return SipFail;
	  }
#ifndef SIP_NO_CHECK

	if ( pErr == SIP_NULL )
		return SipFail;

	if ( pMsgSummary == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pMsgSummary->dStatus = dStatusType;
	SIPDEBUGFN("Exiting sip_mwi_setStatusInMesgSummaryMessage\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sip_mwi_getSummaryLineCountFromMesgSummaryMessage
**
** DESCRIPTION: This function retrieves the number of SummaryLines from an
**		Message Summary Body
**
**********************************************************************/

SipBool sip_mwi_getSummaryLineCountFromMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SIP_U32bit *pIndex, SipError *pErr )
#else
	( pMsgSummary,pIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SIP_U32bit *pIndex;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_mwi_getSummaryLineCountFromMesgSummaryMessage");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pMsgSummary == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(sip_listSizeOf(&(pMsgSummary->slSummaryLine), pIndex, pErr) ==\
	 SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exitting sip_mwi_getSummaryLineCountFromMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sip_mwi_getSummaryLineAtIndexFromMesgSummaryMessage
**
** DESCRIPTION: This function retrieves Summary Line at specified index
**		from Mesg Summary Message
**
*******************************************************************/
SipBool sip_mwi_getSummaryLineAtIndexFromMesgSummaryMessage
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( MesgSummaryMessage *pMsgSummary, SummaryLine **slSummaryLine,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( MesgSummaryMessage *pMsgSummary, SummaryLine *slSummaryLine,\
	 SIP_U32bit dIndex, SipError *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMsgSummary,slSummaryLine,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SummaryLine **slSummaryLine;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#else
	( pMsgSummary,slSummaryLine,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SummaryLine *slSummaryLine;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list=SIP_NULL;
	SIPDEBUGFN("Entering sip_mwi_getSummaryLineAtIndexFromMesgSummaryMessage\n");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pMsgSummary == SIP_NULL) || ( slSummaryLine == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pMsgSummary->slSummaryLine), dIndex,\
		 &element_from_list, pErr) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	if( __sip_mwi_cloneSummaryLine(slSummaryLine, \
		( SummaryLine* )element_from_list, pErr) == SipFail )
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF((( SummaryLine* ) element_from_list)->dRefCount);
	*slSummaryLine = ( SummaryLine* ) element_from_list;
#endif

	SIPDEBUGFN("Exitting sip_mwi_getSummaryLineAtIndexFromMesgSummaryMessage\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_mwi_setSummaryLineAtIndexInMesgSummaryMessage
**
** DESCRIPTION: This function sets Summary Line at specified index
**		In Mesg Summary Message
**
**
*******************************************************************/
SipBool sip_mwi_setSummaryLineAtIndexInMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SummaryLine *slSummaryLine,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pMsgSummary,slSummaryLine,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SummaryLine *slSummaryLine;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SummaryLine* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_mwi_setSummaryLineAtIndexInMesgSummaryMessage\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pMsgSummary == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slSummaryLine == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		sip_mwi_initSummaryLine(&element_in_list,pErr);
		if(__sip_mwi_cloneSummaryLine( element_in_list, slSummaryLine,\
		 pErr) == SipFail )
        	{
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

#else
		element_in_list = slSummaryLine;
		HSS_LOCKEDINCREF((( SummaryLine* ) element_in_list)->dRefCount);
#endif
	}

	if( sip_listSetAt( &(pMsgSummary->slSummaryLine), dIndex,\
	 (SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_mwi_freeSummaryLine(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_mwi_setSummaryLineAtIndexInMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_mwi_insertSummaryLineAtIndexInMesgSummaryMessage
**
** DESCRIPTION: This function Inserts Summary Line at specified index
**		In Mesg Summary Message
**
**
*******************************************************************/
SipBool sip_mwi_insertSummaryLineAtIndexInMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SummaryLine *slSummaryLine,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pMsgSummary,slSummaryLine,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SummaryLine *slSummaryLine;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SummaryLine* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_mwi_insertSummaryLineAtIndexInMesgSummaryMessage\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pMsgSummary == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slSummaryLine == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		sip_mwi_initSummaryLine(&element_in_list,pErr);
		if(__sip_mwi_cloneSummaryLine( element_in_list, slSummaryLine,\
		 pErr) == SipFail )
        	{
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

#else
		element_in_list = slSummaryLine;
		HSS_LOCKEDINCREF((( SummaryLine* ) element_in_list)->dRefCount);
#endif
	}

	if( sip_listInsertAt( &(pMsgSummary->slSummaryLine), dIndex,\
	 (SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_mwi_freeSummaryLine(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_mwi_insertSummaryLineAtIndexInMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_mwi_deleteSummaryLineAtIndexInMesgSummaryMessage
**
** DESCRIPTION: This function deletes Summary Line at specified index
**		In Mesg Summary Message
**
**
**********************************************************************/

SipBool sip_mwi_deleteSummaryLineAtIndexInMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SIP_U32bit dIndex, SipError *pErr )
#else
	( pMsgSummary,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering sip_mwi_deleteSummaryLineAtIndexInMesgSummaryMessage\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMsgSummary == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(pMsgSummary->slSummaryLine), dIndex, pErr) ==\
	 SipFail)
		return SipFail;

	SIPDEBUGFN("Exitting sip_mwi_deleteSummaryLineAtIndexInMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sip_mwi_getNameValuePairCountFromMesgSummaryMessage
**
** DESCRIPTION: This function retrieves the number of NameValuePairs from an
**		Message Summary Body
**
**
**********************************************************************/

SipBool sip_mwi_getNameValuePairCountFromMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SIP_U32bit *pIndex, SipError *pErr )
#else
	( pMsgSummary,pIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SIP_U32bit *pIndex;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_mwi_getNameValuePairCountFromMesgSummaryMessage \n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pMsgSummary == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pMsgSummary->slNameValue), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exitting sip_mwi_getNameValuePairCountFromMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sip_mwi_getNameValuePairAtIndexFromMesgSummaryMessage
**
** DESCRIPTION: This function retrieves NameValuePair at specified index
**		from Mesg Summary Message
**
*******************************************************************/

SipBool sip_mwi_getNameValuePairAtIndexFromMesgSummaryMessage
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( MesgSummaryMessage *pMsgSummary, SipNameValuePair **slNameValue,\
	 SIP_U32bit index, SipError *pErr )
#else
	( MesgSummaryMessage *pMsgSummary, SipNameValuePair *slNameValue,\
	 SIP_U32bit index, SipError *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMsgSummary,slNameValue,index,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SipNameValuePair **slNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#else
	( pMsgSummary,slNameValue,index,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SipNameValuePair *slNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list=SIP_NULL;
	SIPDEBUGFN("Entering sip_mwi_getNameValuePairAtIndexFromMesgSummaryMessage\n");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pMsgSummary == SIP_NULL) || ( slNameValue == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pMsgSummary->slNameValue), index,\
		 &element_from_list, pErr) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	if( __sip_cloneSipNameValuePair(slNameValue, \
		( SipNameValuePair* )element_from_list ,pErr) == SipFail )
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF((( SipNameValuePair* ) element_from_list)->dRefCount);
	*slNameValue = ( SipNameValuePair* ) element_from_list;
#endif

	SIPDEBUGFN("Exitting sip_mwi_getNameValuePairAtIndexFromMesgSummaryMessage\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_mwi_setNameValuePairAtIndexInMesgSummaryMessage
**
** DESCRIPTION: This function sets NameValuePair at specified index
**		in Mesg Summary Message
**
**
*******************************************************************/
SipBool sip_mwi_setNameValuePairAtIndexInMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SipNameValuePair *slNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pMsgSummary,slNameValue,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SipNameValuePair *slNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_mwi_setNameValuePairAtIndexInMesgSummaryMessage\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pMsgSummary == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slNameValue == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		sip_initSipNameValuePair(&element_in_list,pErr);
		if( __sip_cloneSipNameValuePair( element_in_list, slNameValue \
			 ,pErr) == SipFail )
        	{
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

#else
		element_in_list = slNameValue;
		HSS_LOCKEDINCREF((( SipNameValuePair* ) element_in_list)->\
		dRefCount);
#endif
	}

	if( sip_listSetAt( &(pMsgSummary->slNameValue), dIndex,   \
		 (SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_mwi_setNameValuePairAtIndexInMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_mwi_insertNameValuePairAtIndexInMesgSummaryMessage
**
** DESCRIPTION: This function inserts NameValuePair at specified index
**		in Mesg Summary Message
**
**
*******************************************************************/
SipBool sip_mwi_insertNameValuePairAtIndexInMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SipNameValuePair *slNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pMsgSummary,slNameValue,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SipNameValuePair *slNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_mwi_insertNameValuePairAtIndexInMesgSummaryMessage\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pMsgSummary == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slNameValue == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		sip_initSipNameValuePair(&element_in_list,pErr);
		if(__sip_cloneSipNameValuePair( element_in_list, slNameValue \
			 ,pErr) == SipFail )
        	{
                	*pErr = E_NO_MEM;
                	return SipFail;
        	}

#else
		element_in_list = slNameValue;
		HSS_LOCKEDINCREF((( SipNameValuePair* ) element_in_list)->\
		dRefCount);
#endif
	}

	if( sip_listInsertAt( &(pMsgSummary->slNameValue), dIndex,   \
		 (SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_mwi_insertNameValuePairAtIndexInMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_mwi_deleteNameValuePairAtIndexInMesgSummaryMessage
**
** DESCRIPTION: This function deletes NameValuePair at specified index
**		in Mesg Summary Message
**
**
**********************************************************************/

SipBool sip_mwi_deleteNameValuePairAtIndexInMesgSummaryMessage
#ifdef ANSI_PROTO
	( MesgSummaryMessage *pMsgSummary, SIP_U32bit dIndex, SipError *pErr )
#else
	( pMsgSummary,dIndex,pErr)
	  MesgSummaryMessage *pMsgSummary;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering sip_mwi_deleteNameValuePairAtIndexInMesgSummaryMessage\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMsgSummary == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(pMsgSummary->slNameValue), dIndex, pErr) == \
	SipFail)
		return SipFail;

	SIPDEBUGFN("Exitting sip_mwi_deleteNameValuePairAtIndexInMesgSummaryMessage\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sip_mwi_getMediaFromSummaryLine
**
** DESCRIPTION: This Function gets the media type from Summary Line
**
**
**********************************************************************/
SipBool sip_mwi_getMediaFromSummaryLine
#ifdef ANSI_PROTO
	( SummaryLine *pSummaryLine, SIP_S8bit **ppMedia, SipError *pErr)
#else
	( pSummaryLine, ppMedia, pErr )
	  SummaryLine *pSummaryLine;
	  SIP_S8bit **ppMedia;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength=0;
#endif
	SIP_S8bit *pTempMedia=SIP_NULL;
	SIPDEBUGFN("Entering sip_mwi_getMediaFromSummaryLine\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pSummaryLine == SIP_NULL)||(ppMedia==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTempMedia = pSummaryLine->pMedia;

	if( pTempMedia == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(pTempMedia);
	*ppMedia = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppMedia == SIP_NULL )
		return SipFail;

	strcpy( *ppMedia , pTempMedia );
#else
	*ppMedia = pTempMedia;
#endif

	SIPDEBUGFN("Exitting sip_mwi_getMediaFromSummaryLine\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sip_mwi_setMediaInSummaryLine
**
** DESCRIPTION: This Function sets the media type in Summary Line
**
**
**********************************************************************/

SipBool sip_mwi_setMediaInSummaryLine
#ifdef ANSI_PROTO
	( SummaryLine *pSummaryLine, SIP_S8bit *pMedia, SipError *pErr)
#else
	( pSummaryLine, pMedia, pErr )
	  SummaryLine *pSummaryLine;
	  SIP_S8bit *pMedia;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength=0;
#endif
	SIP_S8bit *pTempMedia=SIP_NULL;
	SIPDEBUGFN("Entering sip_mwi_setMediaInSummaryLine\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pSummaryLine == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pMedia == SIP_NULL)
		pTempMedia = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pMedia );
		pTempMedia = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
		dLength+1,pErr);
		if ( pTempMedia== SIP_NULL )
			return SipFail;

		strcpy( pTempMedia, pMedia );
#else
		pTempMedia = pMedia;
#endif

	}

	if ( pSummaryLine->pMedia != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, pSummaryLine->pMedia,\
		 pErr) == SipFail)
			return SipFail;
	}
        pSummaryLine->pMedia = pTempMedia;
	SIPDEBUGFN("Exiting sip_mwi_setMediaInSummaryLine\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION: sip_mwi_getMesgAccountUrlFromMesgSummaryMessage
**
** DESCRIPTION: This functon retrieves the message account url from the
**		Mesg Summary Body
**
**********************************************************************/
SipBool sip_mwi_getMesgAccountUrlFromMesgSummaryMessage
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(MesgSummaryMessage *pMsgSummary, SipAddrSpec **ppAddrSpec,
	 SipError *pErr)
#else	
	(MesgSummaryMessage *pMsgSummary, SipAddrSpec *pAddrSpec,
	 SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMsgSummary, ppAddrSpec, pErr )
	MesgSummaryMessage *pMsgSummary;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#else
	( pMsgSummary, pAddrSpec, pErr )
	MesgSummaryMessage *pMsgSummary;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
#endif
{
	SipAddrSpec *pTempAddrSpec=SIP_NULL;

	SIPDEBUGFN("Entering function sip_mwi_getMesgAccountUrlFromMesgSummaryMessage");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pAddrSpec == SIP_NULL)
#else
	if(  ppAddrSpec == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pMsgSummary == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
 	pTempAddrSpec = pMsgSummary->pAddrSpec ;
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
	SIPDEBUGFN("Exiting function sip_mwi_getMesgAccountUrlFromMesgSummaryMessage");
	return SipSuccess;

}

/********************************************************************
** FUNCTION: sip_mwi_setMesgAccountUrlInMesgSummaryMessage
**
** DESCRIPTION: This functon set the message account url in the
**		Mesg Summary Body
**
**********************************************************************/
SipBool sip_mwi_setMesgAccountUrlInMesgSummaryMessage
#ifdef ANSI_PROTO
	(MesgSummaryMessage *pMsgSummary, SipAddrSpec *pAddrSpec,
	 SipError *pErr)
#else
	( pMsgSummary, pAddrSpec, pErr )
	MesgSummaryMessage *pMsgSummary;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *pTempAddrSpec=SIP_NULL;
#endif
 	SIPDEBUGFN("Entering function sip_mwi_setMesgAccountUrlInMesgSummaryMessage");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;

	if( pMsgSummary == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
  if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(pMsgSummary->pAddrSpec);
		pMsgSummary->pAddrSpec=SIP_NULL ;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType, pErr ) == SipFail)
			return SipFail;

		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			return SipFail;
		}
		sip_freeSipAddrSpec(pMsgSummary->pAddrSpec);
 		pMsgSummary->pAddrSpec = pTempAddrSpec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(pMsgSummary->pAddrSpec);
		pMsgSummary->pAddrSpec = pAddrSpec;
#endif
 	}

	*pErr = E_NO_ERROR;
 	SIPDEBUGFN("Exiting function sip_mwi_setMesgAccountUrlInMesgSummaryMessage");
	return SipSuccess;
}


#endif
