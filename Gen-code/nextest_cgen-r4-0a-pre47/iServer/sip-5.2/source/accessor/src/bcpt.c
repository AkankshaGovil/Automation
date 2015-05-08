/************************************************************
** FUNCTION:
	This file implements the bcpt extention accesssor APIs.
**
*************************************************************
**
** FILENAME:
	bcpt.c
**
** DESCRIPTION
**	
**
**  DATE           NAME                    REFERENCE
** 10Feb00	    B.Borthakur	                Original    
**
** Copyright 1999, Hughes Software Systems, Ltd.
*************************************************************/


#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "sipbcptfree.h"
#include "sipbcptinit.h"
#include "bcpt.h"
#include "sipinternal.h"
#include "sipbcptinternal.h"
#include "sipclone.h"


/*********************************************************************
** FUNCTION:  sip_bcpt_getContentIdFromMimeHdr
**********************************************************************

** DESCRIPTION: Gets the ContentId field pValue from Mime pHeader.

*********************************************************************/
SipBool sip_bcpt_getContentIdFromMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader *pHdr, SIP_S8bit **ppId, SipError *pErr)
#else
	( pHdr, ppId, pErr )
	  SipMimeHeader *pHdr;
	  SIP_S8bit **ppId;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getContentIdFromMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pHdr == SIP_NULL) || (ppId == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	if( pHdr->pContentId == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE 
	*ppId = pHdr->pContentId;
#else
	*ppId = (SIP_S8bit *)STRDUPACCESSOR(pHdr->pContentId);
	if(*ppId == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getContentIdFromMimeHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sip_bcpt_setContentIdInMimeHdr
**********************************************************************

** DESCRIPTION: Sets the ContentId field pValue in Mime pHeader.

*********************************************************************/
SipBool sip_bcpt_setContentIdInMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader *pHdr, SIP_S8bit *pId, SipError *pErr)
#else
	( pHdr, pId, pErr )
	  SipMimeHeader *pHdr;
	  SIP_S8bit *pId;
	  SipError *pErr;
#endif
{
	SIP_S8bit * pTempId;

	SIPDEBUGFN ( "Entering setContentIdInMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if  ( pHdr == SIP_NULL)  
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pId == SIP_NULL)
		pTempId = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pTempId=pId;
#else
		pTempId = (SIP_S8bit *)STRDUPACCESSOR(pId);
		if(pTempId == SIP_NULL)
			return SipFail;
#endif
	
	}
	
	if ( pHdr->pContentId != SIP_NULL ) 
	{
		 if( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pHdr->pContentId)), pErr) == SipFail)
			return SipFail;
	}

	pHdr->pContentId = pTempId; 

	SIPDEBUGFN ( "Exiting setContentIdInMimeHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sip_bcpt_getContentDescFromMimeHdr
**********************************************************************

** DESCRIPTION: Gets the Content Description field pValue from Mime
** pHeader.

*********************************************************************/
SipBool sip_bcpt_getContentDescFromMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader *pHdr, SIP_S8bit **ppDescription, SipError *pErr)
#else
	( pHdr, ppDescription, pErr )
	  SipMimeHeader *pHdr;
	  SIP_S8bit **ppDescription;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getContentDescFromMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( ( pHdr == SIP_NULL) || (ppDescription == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	if( pHdr->pContentDescription == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	

#ifdef SIP_BY_REFERENCE
	*ppDescription = pHdr->pContentDescription;
#else
	*ppDescription = (SIP_S8bit *)STRDUPACCESSOR(pHdr->pContentDescription);
	if(*ppDescription == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	SIPDEBUGFN ( "Exiting getContentDescFromMimeHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sip_bcpt_setContentDescInMimeHdr
**********************************************************************

** DESCRIPTION: Sets the Content Description field pValue in Mime pHeader.

*********************************************************************/
SipBool sip_bcpt_setContentDescInMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader *pHdr, SIP_S8bit *pDescription, SipError *pErr)
#else
	( pHdr, pDescription, pErr )
	  SipMimeHeader *pHdr;
	  SIP_S8bit *pDescription;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempDescription;
#endif

	SIPDEBUGFN ( "Entering setContentDescInMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pHdr->pContentDescription != SIP_NULL ) 
	{
		 if( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pHdr->pContentDescription)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pHdr->pContentDescription = pDescription; 
#else
	if( pDescription == SIP_NULL)
		pTempDescription = SIP_NULL;
	else
	{
		pTempDescription = (SIP_S8bit *)STRDUPACCESSOR(pDescription);
		if(pTempDescription == SIP_NULL)
			return SipFail;
	
	}

	pHdr->pContentDescription = pTempDescription; 
#endif	

	SIPDEBUGFN ( "Exiting setContentDescInMimeHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION: sdp_getAdditionalMimeHdrCountFromMimeHdr  
**********************************************************************
**
** DESCRIPTION:  Gets the number of Additional Mime Headers nodes 
** present in Sip Mime Header structure.
**
*********************************************************************/
SipBool sip_bcpt_getAdditionalMimeHdrCountFromMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader	*pHdr,
	  SIP_U32bit	*pIndex,
	  SipError	*pErr  )
#else 
	( pHdr,pIndex,pErr)
	  SipMimeHeader 	*pHdr;
	  SIP_U32bit 	*pIndex;
	  SipError 	*pErr;
#endif
{

	SIPDEBUGFN ( "Entering GetAdditionalMimeHdrCountFromMimeHdr");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL ) 
		return SipFail;
	
	if ( (pHdr == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pHdr->slAdditionalMimeHeaders), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}
	
	SIPDEBUGFN ( "Exiting GetAdditionalMimeHdrCountFromMimeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;	
}

/*********************************************************************
** FUNCTION: sdp_getAdditionalMimeHdrAtIndexFromMimeHdr
**********************************************************************
**
** DESCRIPTION:	Gets the Additional Mime Headers pValue at a 
** specified index ( starting from 0 ) from pHdr strucutre.
**
*********************************************************************/
SipBool sip_bcpt_getAdditionalMimeHdrAtIndexFromMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader 	*pHdr,
	  SIP_S8bit 	**ppAddMimeHdr,
	  SIP_U32bit 	index, 
	  SipError 	*pErr )
#else 
	( pHdr,ppAddMimeHdr,index,pErr)
	  SipMimeHeader *pHdr;
	  SIP_S8bit **ppAddMimeHdr;
	  SIP_U32bit index;
	  SipError *pErr;
#endif
{
	SIP_Pvoid pElementFromList;
#ifndef SIP_BY_REFERENCE
	SIP_U32bit size;
#endif

	SIPDEBUGFN ( "Entering GetAdditionalMimeHdrAtIndexFromMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pHdr == SIP_NULL) || (ppAddMimeHdr == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pHdr->slAdditionalMimeHeaders), index, &pElementFromList, pErr) == SipFail)
		return SipFail;
	
	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;		
	}
#ifdef SIP_BY_REFERENCE
	*ppAddMimeHdr = (SIP_S8bit *) pElementFromList;
#else
	size = strlen( (SIP_S8bit * )pElementFromList);
	*ppAddMimeHdr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, size +1, pErr);
	if(*ppAddMimeHdr == SIP_NULL)
		return SipFail;

	strcpy(*ppAddMimeHdr, (SIP_S8bit*)pElementFromList);
#endif
	SIPDEBUGFN ( "Exiting GetAdditionalMimeHdrAtIndexFromMimeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION: sdp_setAdditionalMimeHdrAtIndexInMimeHdr  
**********************************************************************
**
** DESCRIPTION: Sets the Additional Mime Headers pValue at a 
** specified index ( starting from 0 )in Sip Mime Header strucutre.
**
*********************************************************************/
SipBool sip_bcpt_setAdditionalMimeHdrAtIndexInMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader 	*pHdr, 
	  SIP_S8bit 	*pAddMimeHdr,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pHdr,pAddMimeHdr,index,pErr)
	  SipMimeHeader 	*pHdr;
	  SIP_S8bit 	*pAddMimeHdr;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
		
	SIP_S8bit * pElementFromList;
#ifndef SIP_BY_REFERENCE
	SipError tempErr;		/* used in freeing memory after an pError has happened */
#endif

	SIPDEBUGFN ( "Entering SetAdditionalMimeHdrAtIndexInMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pHdr == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pAddMimeHdr == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pAddMimeHdr;
#else		
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,strlen(pAddMimeHdr) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;
	
		strcpy(pElementFromList, pAddMimeHdr);
#endif
	}

	if( sip_listSetAt(&(pHdr->slAdditionalMimeHeaders), index, pElementFromList, pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
#ifndef SIP_BY_REFERENCE
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), &tempErr);
#endif
		return SipFail;
	}
	
	SIPDEBUGFN ( "Exiting SetAdditionalMimeHdrAtIndexinMimeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sdp_insertAdditionalMimeHdrAtIndexInMimeHdr  
**********************************************************************
**
** DESCRIPTION: Inserts a Additional Mime Headers pValue at a 
** specified index ( starting from 0 )  in Sip Mime Header strucutre.
**
*********************************************************************/
SipBool sip_bcpt_insertAdditionalMimeHdrAtIndexInMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader 	*pHdr,
	  SIP_S8bit 	*pAddMimeHdr,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pHdr,pAddMimeHdr,index,pErr)
	  SipMimeHeader 	*pHdr;
	  SIP_S8bit 	*pAddMimeHdr;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
		
	SIP_S8bit * pElementFromList;
#ifndef SIP_BY_REFERENCE
	SipError tempErr;		/* used in freeing memory after an pError has happened */
#endif


	SIPDEBUGFN ( "Entering InsertAdditionalMimeHdrAtIndexInMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	/* copying the Additional Mime Headers structure/char*  */		
	if ( pAddMimeHdr == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pAddMimeHdr;
#else		
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,strlen(pAddMimeHdr) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;
	
		strcpy(pElementFromList, pAddMimeHdr);
#endif
	}

	if( sip_listInsertAt(&(pHdr->slAdditionalMimeHeaders), index, pElementFromList, pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
#ifndef SIP_BY_REFERENCE
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), &tempErr);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting InsertAdditionalMimeHdrAtIndexinMimeHdr");
	
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sdp_deleteAdditionalMimeHdrAtIndexInMimeHdr  
**********************************************************************
**
** DESCRIPTION: Deletes a Additional Mime Headers pValue at 
** a specified index ( starting from 0 )  in Sip Mime Header strucutre.
**
*********************************************************************/
SipBool sip_bcpt_deleteAdditionalMimeHdrAtIndexInMimeHdr
#ifdef ANSI_PROTO
	( SipMimeHeader 	*pHdr,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pHdr,index,pErr)
	  SipMimeHeader 	*pHdr;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{

	SIPDEBUGFN ( "Entering DeleteAdditionalMimeHdrAtIndexInMimeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
		
	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listDeleteAt(&(pHdr->slAdditionalMimeHeaders), index, pErr) == SipFail)
		return SipFail;
	
	SIPDEBUGFN ( "Exiting DeleteAdditionalMimeHdrAtIndexinMimeHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:   sip_bcpt_getMsgBodyCountFromMime
**********************************************************************
**
** DESCRIPTION: Gets the number of Messsage Body structures present 
** in the Mime Message.
**
*********************************************************************/

SipBool sip_bcpt_getMsgBodyCountFromMime
#ifdef ANSI_PROTO
	( MimeMessage	*pMime,
	  SIP_U32bit	*pIndex,
	  SipError	*pErr  )
#else 
	( pMime,pIndex,pErr)
	  MimeMessage 	*pMime;
	  SIP_U32bit 	*pIndex;
	  SipError 	*pErr;
#endif
{
	SIPDEBUGFN ( "Entering getMsgBodyCountFromMime");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL ) 
		return SipFail;
	
	if ( (pMime == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pMime->slRecmimeBody), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}
	
	SIPDEBUGFN ( "Exiting getMsgBodyCountFromMime");
	*pErr = E_NO_ERROR;
	return SipSuccess;	
}

/*********************************************************************
** FUNCTION: sip_bcpt_getMsgBodyAtIndexFromMime
**********************************************************************
**
** DESCRIPTION: Gets the Messsage Body structure at a specified 
** index ( starting from 0 ) from the Mime Message.
**
*********************************************************************/
SipBool sip_bcpt_getMsgBodyAtIndexFromMime
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( MimeMessage 	*pMime,
	  SipMsgBody **ppMsgB,
	  SIP_U32bit 	index, 
	  SipError 	*pErr )
#else 
	( MimeMessage 	*pMime,
	  SipMsgBody *pMsgB,
	  SIP_U32bit 	index, 
	  SipError 	*pErr )
#endif
#else 
#ifdef SIP_BY_REFERENCE
	( pMime,ppMsgB,index,pErr)
	  MimeMessage 	*pMime;
	  SipMsgBody **ppMsgB;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#else 
	( pMime,pMsgB,index,pErr)
	  MimeMessage 	*pMime;
	  SipMsgBody *pMsgB;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
#endif
{
	SIP_Pvoid 	pElementFromList;

	SIPDEBUGFN ( "Entering getMsgBodyAtIndexFromMime");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

#ifdef SIP_BY_REFERENCE
	if ( (pMime == SIP_NULL) || (ppMsgB == SIP_NULL) )
#else
	if ( (pMime == SIP_NULL) || (pMsgB == SIP_NULL) )
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pMime->slRecmimeBody), index, &pElementFromList, pErr) == SipFail)
		return SipFail;
	
	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppMsgB = (SipMsgBody *) pElementFromList;
#else
	if ( __sip_cloneSipMsgBody(pMsgB,(SipMsgBody *)pElementFromList,pErr) == SipFail)
		return SipFail;
#endif

	SIPDEBUGFN ( "Exiting getMsgBodyAtIndexFromMime");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_bcpt_setMsgBodyAtIndexInMime  
**********************************************************************
**
** DESCRIPTION: Sets the Messsage Body structure at a specified 
** index ( starting from 0 ) in the Mime Message.
**
*********************************************************************/
SipBool sip_bcpt_setMsgBodyAtIndexInMime
#ifdef ANSI_PROTO
	( MimeMessage 	*pMime, 
	  SipMsgBody	*pMsgB,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pMime,pMsgB,index,pErr)
	  MimeMessage 	*pMime;
	  SipMsgBody	*pMsgB;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
	SipMsgBody 	*pElementInList;

	SIPDEBUGFN ( "Entering setMsgBodyAtIndexInMime");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	
	if (pMime == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	/* validating whether the dType is one of the defined ones.*/
	if( sip_validateSipMsgBodyType(pMsgB->dType, pErr) == SipFail)
		return SipFail;

	if ( pMsgB == SIP_NULL )
		pElementInList = SIP_NULL;
	else
	{
		if ( pMsgB->dType == SipBodyAny )
		{
			*pErr = E_INV_TYPE;
			return SipFail;
		}

#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pMsgB->dRefCount);
		pElementInList = pMsgB;
#else
		if (sip_initSipMsgBody(&pElementInList,SipBodyAny, pErr) == SipFail)
			return SipFail;
		if ( __sip_cloneSipMsgBody(pElementInList, pMsgB,pErr) == SipFail)		
		{
			sip_freeSipMsgBody(pElementInList ); 
			return SipFail;
		}
#endif
	}

	if( sip_listSetAt(&(pMime->slRecmimeBody), index, (SIP_Pvoid) pElementInList, pErr) == SipFail)
	{
		if ( pElementInList != SIP_NULL )
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pMsgB->dRefCount);
#else
			 sip_freeSipMsgBody(pElementInList ); 
#endif
		return SipFail;
	}
	SIPDEBUGFN ( "Exiting setMsgBodyAtIndexInMime");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_bcpt_insertMsgBodyAtIndexInMime  
**********************************************************************
**
** DESCRIPTION: Inserts the Messsage Body structure at a specified 
** index ( starting from 0 ) in the Mime Message.
**
*********************************************************************/
SipBool sip_bcpt_insertMsgBodyAtIndexInMime
#ifdef ANSI_PROTO
	( MimeMessage 	*pMime,
	  SipMsgBody	*pMsgB,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pMime,pMsgB,index,pErr)
	  MimeMessage 	*pMime;
	  SipMsgBody	*pMsgB;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
	SipMsgBody 	*pElementInList;

	SIPDEBUGFN ( "Entering InsertMsgBodyAtIndexInMime");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMime == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* validating whether the dType is one of the defined ones.*/
	if ( sip_validateSipMsgBodyType(pMsgB->dType, pErr) == SipFail)
		return SipFail;
	/* copying the Messsage Body structure */		
	if ( pMsgB == SIP_NULL )
		pElementInList = SIP_NULL;
	else
	{
		if ( pMsgB->dType == SipBodyAny )
		{
			*pErr = E_INV_TYPE;
			return SipFail;
		}


#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pMsgB->dRefCount);
		pElementInList = pMsgB;
#else
		if (sip_initSipMsgBody(&pElementInList,SipBodyAny, pErr) == SipFail)
			return SipFail;
		if ( __sip_cloneSipMsgBody(pElementInList, pMsgB,pErr) == SipFail)		
		{
			sip_freeSipMsgBody(pElementInList ); 
			return SipFail;
		}
#endif

	}
	
	if( sip_listInsertAt(&(pMime->slRecmimeBody), index, pElementInList, pErr) == SipFail)
	{
		if ( pElementInList != SIP_NULL )
#ifdef SIP_BY_REFERENCE
			HSS_LOCKEDDECREF(pMsgB->dRefCount);
#else
			 sip_freeSipMsgBody(pElementInList ); 
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting InsertMsgBodyAtIndexInMime");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_bcpt_deleteMsgBodyAtIndexInMime  
**********************************************************************
**
** DESCRIPTION: Deletes a Messsage Body structure at a specified index 
** ( starting from 0 ) in the Mime Message.
**
*********************************************************************/
SipBool sip_bcpt_deleteMsgBodyAtIndexInMime
#ifdef ANSI_PROTO
	( MimeMessage 	*pMime,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pMime,index,pErr)
	  MimeMessage 	*pMime;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
	SIPDEBUGFN ( "Entering deleteMsgBodyAtIndexInMime");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
		
	if ( pMime == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pMime->slRecmimeBody), index, pErr) == SipFail)
		return SipFail;
	

	SIPDEBUGFN ( "Exiting deleteMsgBodyAtIndexInMime");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_bcpt_getMsgBodyTypeAtIndexFromMime  
**********************************************************************
**
** DESCRIPTION: Gets the Messsage Body structure dType 
** ( mime/sdp/unknown/isup at a specified index ( starting from 0 ) 
** from the Mime Message.
**
*********************************************************************/
SipBool sip_bcpt_getMsgBodyTypeAtIndexFromMime
#ifdef ANSI_PROTO
	( MimeMessage 		*pMime,
	  en_SipMsgBodyType *pType,
	  SIP_U32bit 		index, 
	  SipError 			*pErr )
#else 
	( pMime,pType,index,pErr)
	  MimeMessage 	*pMime;
	  en_SipMsgBodyType *pType;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
	SIP_Pvoid 	pElementFromList;

	SIPDEBUGFN ( "Entering getMsgBodyTypeAtIndexFromMime");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pMime == SIP_NULL) || (pType == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	if( sip_listGetAt(&(pMime->slRecmimeBody), index, &pElementFromList, pErr) == SipFail)
		return SipFail;
	
	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	*pType = ( (SipMsgBody*)(pElementFromList) )->dType;

	SIPDEBUGFN ( "Exiting getMsgBodyTypeAtIndexFromMime");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_bcpt_getIsupFromMsgBody
**
** DESCRIPTION: Retrieves the ISUP message pBody from SipMsgBody structure
**
**********************************************************************/
SipBool sip_bcpt_getIsupFromMsgBody
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipMsgBody *pMsg, IsupMessage **ppIsup, SipError *pErr)
#else
	( SipMsgBody *pMsg, IsupMessage *pIsup, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMsg, ppIsup, pErr )
	  SipMsgBody *pMsg;
	  IsupMessage **ppIsup;
	  SipError *pErr;
#else
	( pMsg, pIsup, pErr )
	  SipMsgBody *pMsg;
	  IsupMessage *pIsup;
	  SipError *pErr;
#endif
#endif
{
	IsupMessage *pTempIsup;
	SIPDEBUGFN("Entering getIsupFromMsgBody\n"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	if ( (pMsg == SIP_NULL) || (ppIsup == SIP_NULL ) )
	{
		*pErr = E_INV_PARAM; 
		return SipFail;
	}
#else
	if ( (pMsg == SIP_NULL) || (pIsup == SIP_NULL ) )
	{
		*pErr = E_INV_PARAM; 
		return SipFail;
	}
#endif	
	if ( pMsg->dType != SipIsupBody)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif 
	if (pMsg->u.pIsupMessage == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempIsup = pMsg->u.pIsupMessage;

#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pTempIsup->dRefCount);
	*ppIsup = pTempIsup;
#else
	if(sip_bcpt_cloneIsupMessage(pIsup,pMsg->u.pIsupMessage,pErr) == SipFail)
	{
		return SipFail;
	}
#endif
		
	SIPDEBUGFN("Exiting getIsupFromMsgBody\n"); 
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_bcpt_setIsupInMsgBody
**
** DESCRIPTION: Sets an ISUP message pBody in SipMsgBody structure.
**
**********************************************************************/

SipBool sip_bcpt_setIsupInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *pMsg, IsupMessage *pIsup, SipError *pErr)
#else
	( pMsg, pIsup, pErr )
	  SipMsgBody *pMsg;
	  IsupMessage *pIsup;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	IsupMessage	*pTempIsup;
#endif
	
	SIPDEBUGFN("Entering sip_bcpt_setIsupInBody\n");
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
	
	if( pMsg->dType!=SipIsupBody )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( pMsg->u.pIsupMessage != SIP_NULL)
		sip_bcpt_freeIsupMessage( pMsg->u.pIsupMessage);
	
	if (pIsup==SIP_NULL)
	{
		pMsg->u.pIsupMessage = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pIsup->dRefCount);
		pMsg->u.pIsupMessage = pIsup;
#else
		if ( sip_bcpt_initIsupMessage(&pTempIsup, pErr) == SipFail)
			return SipFail;

		if ( sip_bcpt_cloneIsupMessage(pTempIsup, pIsup, pErr) == SipFail)
		{
			sip_bcpt_freeIsupMessage( pTempIsup );
			return SipFail;
		}
		pMsg->u.pIsupMessage = pTempIsup;
#endif
	}	
	SIPDEBUGFN("Exiting sip_bcpt_setIsupBody\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_bcpt_getQsigFromMsgBody
**
** DESCRIPTION: Retrieves the QSIG message pBody from SipMsgBody structure
**
**********************************************************************/
SipBool sip_bcpt_getQsigFromMsgBody
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipMsgBody *pMsg, QsigMessage **ppQsig, SipError *pErr)
#else
	( SipMsgBody *pMsg, QsigMessage *pQsig, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMsg, ppQsig, pErr )
	  SipMsgBody *pMsg;
	  QsigMessage **ppQsig;
	  SipError *pErr;
#else
	( pMsg, pQsig, pErr )
	  SipMsgBody *pMsg;
	  QsigMessage *pQsig;
	  SipError *pErr;
#endif
#endif
{
	QsigMessage *pTempQsig=SIP_NULL;
	SIPDEBUGFN("Entering getQsigFromMsgBody\n"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		return SipFail;
	}

	#ifdef SIP_BY_REFERENCE
		if ( (pMsg == SIP_NULL) || (ppQsig == SIP_NULL ) )
		{
			*pErr = E_INV_PARAM; 
			return SipFail;
		}
	#else
		if ( (pMsg == SIP_NULL) || (pQsig == SIP_NULL ) )
		{
			*pErr = E_INV_PARAM; 
			return SipFail;
		}
	#endif	
	if ( pMsg->dType != SipQsigBody)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif 
	if (pMsg->u.pQsigMessage == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempQsig = pMsg->u.pQsigMessage;

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempQsig->dRefCount);
	*ppQsig = pTempQsig;
#else
	if(sip_bcpt_cloneQsigMessage(pQsig,pMsg->u.pQsigMessage,pErr) == SipFail)
	{
		return SipFail;
	}
#endif
		
	SIPDEBUGFN("Exiting getQsigFromMsgBody"); 
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_bcpt_setQsigInMsgBody
**
** DESCRIPTION: Sets an QSIG message pBody in SipMsgBody structure.
**
**********************************************************************/

SipBool sip_bcpt_setQsigInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *pMsg, QsigMessage *pQsig, SipError *pErr)
#else
	( pMsg, pQsig, pErr )
	  SipMsgBody *pMsg;
	  QsigMessage *pQsig;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	QsigMessage	*pTempQsig=SIP_NULL;
#endif
	
	SIPDEBUGFN("Entering sip_bcpt_setQsigInBody\n");
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
	
	if( pMsg->dType!=SipQsigBody )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( pMsg->u.pQsigMessage != SIP_NULL)
		sip_bcpt_freeQsigMessage( pMsg->u.pQsigMessage);
	
	if (pQsig==SIP_NULL)
	{
		pMsg->u.pQsigMessage = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pQsig->dRefCount);
		pMsg->u.pQsigMessage = pQsig;
#else
		if ( sip_bcpt_initQsigMessage(&pTempQsig, pErr) == SipFail)
			return SipFail;

		if ( sip_bcpt_cloneQsigMessage(pTempQsig, pQsig, pErr) == SipFail)
		{
			sip_bcpt_freeQsigMessage( pTempQsig );
			return SipFail;
		}
		pMsg->u.pQsigMessage = pTempQsig;
#endif
	}	
	SIPDEBUGFN("Exiting sip_bcpt_setQsigBody\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_bcpt_getMimeFromMsgBody
**
** DESCRIPTION: Retrieves the Mime message pBody from SipMsgBody structure
**
**********************************************************************/
SipBool sip_bcpt_getMimeFromMsgBody
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipMsgBody *pMsg, MimeMessage **ppMime, SipError *pErr)
#else
	( SipMsgBody *pMsg, MimeMessage *pMime, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMsg, ppMime, pErr )
	  SipMsgBody *pMsg;
	  MimeMessage **ppMime;
	  SipError *pErr;
#else
	( pMsg, pMime, pErr )
	  SipMsgBody *pMsg;
	  MimeMessage *pMime;
	  SipError *pErr;
#endif
#endif
{
	MimeMessage *pTempMime;
	SIPDEBUGFN("Entering sip_bcpt_getMimeFromMsgBody\n"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		return SipFail;
	}

	#ifdef SIP_BY_REFERENCE
		if ( (pMsg == SIP_NULL) || (ppMime == SIP_NULL ) )
		{
			*pErr = E_INV_PARAM; 
			return SipFail;
		}
	#else
		if ( (pMsg == SIP_NULL) || (pMime == SIP_NULL ) )
		{
			*pErr = E_INV_PARAM; 
			return SipFail;
		}
	#endif
	
	if ( pMsg->dType != SipMultipartMimeBody)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if (pMsg->u.pMimeMessage == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pTempMime = pMsg->u.pMimeMessage;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempMime->dRefCount);
	*ppMime = pTempMime;
#else
	if(sip_bcpt_cloneMimeMessage(pMime,pMsg->u.pMimeMessage,pErr) == SipFail)
	{
		return SipFail;
	}
#endif		

	SIPDEBUGFN("Exiting sip_bcpt_getMimeFromMsgBody"); 
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_bcpt_setMimeInMsgBody
**
** DESCRIPTION: Sets the mime message in the SipMsgBody structure. 
**
**********************************************************************/

SipBool sip_bcpt_setMimeInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *pMsg, MimeMessage *pMime, SipError *pErr)
#else
	( pMsg, pMime, pErr )
	  SipMsgBody *pMsg;
	  MimeMessage *pMime;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	MimeMessage	*pTempMime;
#endif
	
	SIPDEBUGFN("Entering sip_bcpt_setMimeInBody\n");
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
	
	if( pMsg->dType!=SipMultipartMimeBody )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( pMsg->u.pMimeMessage != SIP_NULL)
		sip_bcpt_freeMimeMessage( pMsg->u.pMimeMessage);

	if (pMime==SIP_NULL)
	{
		pMsg->u.pMimeMessage = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pMime->dRefCount);
		pMsg->u.pMimeMessage = pMime;
#else
		if ( sip_bcpt_initMimeMessage(&pTempMime, pErr) == SipFail)
			return SipFail;

		if ( sip_bcpt_cloneMimeMessage(pTempMime, pMime, pErr) == SipFail)
		{
			sip_bcpt_freeMimeMessage( pTempMime );
			return SipFail;
		}
		pMsg->u.pMimeMessage = pTempMime;
#endif
	}	

	SIPDEBUGFN("Exiting sip_bcpt_setMimeBody\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_bcpt_getLengthFromIsupMessage
**
** DESCRIPTION: This function retrieves the dLength of an Isup
**		message
**
******************************************************************/
SipBool sip_bcpt_getLengthFromIsupMessage
#ifdef ANSI_PROTO
	( IsupMessage *pMsg, SIP_U32bit *dLength, SipError *err)
#else
	( pMsg, dLength, err )
	  IsupMessage *pMsg;
	  SIP_U32bit *dLength;
	  SipError *err;
#endif
{
	SIPDEBUGFN ( "Entering getlengthFromIsupMessage");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pMsg == SIP_NULL) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(dLength == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif	
	*dLength = pMsg->dLength; 
	
	SIPDEBUGFN ( "Exiting getlengthFromIsupMessage");
	
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sip_bcpt_getBodyFromIsupMessage
**********************************************************************

** DESCRIPTION: Gets the Body field pValue from IsupMessage.

*********************************************************************/
SipBool sip_bcpt_getBodyFromIsupMessage
#ifdef ANSI_PROTO
	( IsupMessage *pMsg, SIP_S8bit **ppBody, SipError *pErr)
#else
	( pMsg, ppBody, pErr )
	  IsupMessage *pMsg;
	  SIP_S8bit **ppBody;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getBodyFromIsupMessage");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMsg == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if( pMsg->pBody == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#endif
#ifdef SIP_BY_REFERENCE
	*ppBody = pMsg->pBody;
#else
	*ppBody = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, pMsg->dLength+1,pErr);
	if ( *ppBody == SIP_NULL)
		return SipFail;

	memcpy(*ppBody, pMsg->pBody, pMsg->dLength);
	(*ppBody)[pMsg->dLength]='\0';
#endif 
	SIPDEBUGFN ( "Exiting getBodyFromIsupMessage");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION: sip_bcpt_setBodyInIsupMessage
**********************************************************************

** DESCRIPTION:
	Sets the Body field pValue in IsupMessage.

*********************************************************************/
SipBool sip_bcpt_setBodyInIsupMessage
#ifdef ANSI_PROTO
	( IsupMessage *pMsg, SIP_S8bit *pBody, SIP_U32bit dLength,SipError *pErr)
#else
	( pMsg, pBody, dLength, pErr )
	  IsupMessage *pMsg;
	  SIP_S8bit *pBody;
	  SIP_U32bit dLength;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempBody;
#endif

	SIPDEBUGFN ( "Entering setBodyInIsupMessage");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMsg == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( pMsg->pBody != SIP_NULL ) 
	{
		 if( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pMsg->pBody)), pErr) == SipFail)
			return SipFail;
	}
#endif
#ifdef SIP_BY_REFERENCE
	pMsg->pBody = pBody;
#else
	if( pBody == SIP_NULL)
		pTempBody = SIP_NULL;
	else
	{
		pTempBody = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, dLength, pErr);
		if (pTempBody == SIP_NULL)
			return SipFail;

		memcpy(pTempBody, pBody, dLength);
	}
	

	pMsg->pBody = pTempBody; 
#endif
	pMsg->dLength = dLength;	

	SIPDEBUGFN ( "Exiting setBodyInIsupMessage");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}
/*****************************************************************
**
** FUNCTION:  sip_bcpt_getLengthFromQsigMessage
**
** DESCRIPTION: This function retrieves the dLength of an Qsig
**		message
**
******************************************************************/
SipBool sip_bcpt_getLengthFromQsigMessage
#ifdef ANSI_PROTO
	( QsigMessage *pMsg, SIP_U32bit *dLength, SipError *err)
#else
	( pMsg, dLength, err )
	  QsigMessage *pMsg;
	  SIP_U32bit *dLength;
	  SipError *err;
#endif
{
	SIPDEBUGFN ( "Entering getlengthFromQsigMessage");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pMsg == SIP_NULL) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(dLength == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif	
	*dLength = pMsg->dLength; 
	
	SIPDEBUGFN ( "Exiting getlengthFromQsigMessage");
	
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sip_bcpt_getBodyFromQsigMessage
**********************************************************************

** DESCRIPTION: Gets the Body field pValue from QsigMessage.

*********************************************************************/
SipBool sip_bcpt_getBodyFromQsigMessage
#ifdef ANSI_PROTO
	( QsigMessage *pMsg, SIP_S8bit **ppBody, SipError *pErr)
#else
	( pMsg, ppBody, pErr )
	  QsigMessage *pMsg;
	  SIP_S8bit **ppBody;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getBodyFromQsigMessage");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMsg == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	
	if( pMsg->pBody == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#endif
#ifdef SIP_BY_REFERENCE
	*ppBody = pMsg->pBody;
#else
	*ppBody = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, pMsg->dLength,pErr);
	if ( *ppBody == SIP_NULL)
		return SipFail;

/* 	memcpy(*ppBody, pMsg->pBody, pMsg->dLength); */
	strncpy(*ppBody, pMsg->pBody, pMsg->dLength);
#endif 
	SIPDEBUGFN ( "Exiting getBodyFromQsigMessage");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION: sip_bcpt_setBodyInQsigMessage
**********************************************************************

** DESCRIPTION:
	Sets the Body field pValue in QsigMessage.

*********************************************************************/
SipBool sip_bcpt_setBodyInQsigMessage
#ifdef ANSI_PROTO
	( QsigMessage *pMsg, SIP_S8bit *pBody, SIP_U32bit dLength,SipError *pErr)
#else
	( pMsg, pBody, dLength, pErr )
	  QsigMessage *pMsg;
	  SIP_S8bit *pBody;
	  SIP_U32bit dLength;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempBody=SIP_NULL;
#endif

	SIPDEBUGFN ( "Entering setBodyInQsigMessage");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMsg == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pBody == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( pMsg->pBody != SIP_NULL ) 
	{
		 if( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pMsg->pBody)), \
			 pErr) == SipFail)
			return SipFail;
	}
#endif
#ifdef SIP_BY_REFERENCE
	pMsg->pBody = pBody;
#else
	if( pBody == SIP_NULL)
		pTempBody = SIP_NULL;
	else
	{
		pTempBody = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, dLength, pErr);
		if (pTempBody == SIP_NULL)
			return SipFail;

		strncpy(pTempBody, pBody, dLength);
	}
	

	pMsg->pBody = pTempBody; 
#endif
	pMsg->dLength = dLength;	

	SIPDEBUGFN ( "Exiting setBodyInQsigMessage");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sip_bcpt_getMimeHeaderFromMsgBody
**
** DESCRIPTION: Retrieves the mime pHeader from the SipMsgBody structure.
**
**********************************************************************/
SipBool sip_bcpt_getMimeHeaderFromMsgBody
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipMsgBody *pMsg, SipMimeHeader **ppMime, SipError *pErr)
#else
	( SipMsgBody *pMsg, SipMimeHeader *pMime, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMsg, ppMime, pErr )
	  SipMsgBody *pMsg;
	  SipMimeHeader **ppMime;
	  SipError *pErr;
#else
	( pMsg, pMime, pErr )
	  SipMsgBody *pMsg;
	  SipMimeHeader *pMime;
	  SipError *pErr;
#endif
#endif
{
	SipMimeHeader *pTempMime;

	SIPDEBUGFN("Entering getMimeHeaderFromMsgBody\n"); 
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	if ( (pMsg == SIP_NULL) || (ppMime == SIP_NULL ) )
	{
		*pErr = E_INV_PARAM; 
		return SipFail;
	}
#else
	if ( (pMsg == SIP_NULL) || (pMime == SIP_NULL ) )
	{
		*pErr = E_INV_PARAM; 
		return SipFail;
	}
#endif
#endif	
	if (pMsg->pMimeHeader == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	pTempMime = pMsg->pMimeHeader;
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pTempMime->dRefCount);
	*ppMime = pTempMime;
#else

	if(sip_bcpt_cloneSipMimeHeader(pMime,pMsg->pMimeHeader,pErr) == SipFail)
	{
		return SipFail;
	}
#endif		
	SIPDEBUGFN("Exiting getMimeHeaderFromMsgBody"); 
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_bcpt_setMimeHeaderInMsgBody
**
** DESCRIPTION:  Sets the mime pHeader in SipMsgBody structure.
**
**********************************************************************/

SipBool sip_bcpt_setMimeHeaderInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *pMsg, SipMimeHeader *pMime, SipError *pErr)
#else
	( pMsg, pMime, pErr )
	  SipMsgBody *pMsg;
	  SipMimeHeader *pMime;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipMimeHeader	*pTempMime;
#endif
	
	SIPDEBUGFN("Entering sip_bcpt_setMimeHeaderInBody\n");
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
#endif
	if ( pMsg->pMimeHeader != SIP_NULL)
		sip_bcpt_freeSipMimeHeader( pMsg->pMimeHeader);
		
	if (pMime==SIP_NULL)
	{
		pMsg->pMimeHeader = SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pMime->dRefCount);
		pMsg->pMimeHeader = pMime;	
#else
		if ( sip_bcpt_initSipMimeHeader(&pTempMime, pErr) == SipFail)
			return SipFail;

		if ( sip_bcpt_cloneSipMimeHeader(pTempMime, pMime, pErr) == SipFail)
		{
			sip_bcpt_freeSipMimeHeader( pTempMime );
			return SipFail;
		}
		pMsg->pMimeHeader = pTempMime;
#endif
	}	

	SIPDEBUGFN("Exiting sip_bcpt_setMimeHeaderinMsgBody\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sip_bcpt_getVersionFromMimeVersionHdr
**********************************************************************

** DESCRIPTION: Gets the Version field from Mime-Version Header.

*********************************************************************/
SipBool sip_bcpt_getVersionFromMimeVersionHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppVersion, SipError *pErr)
#else
        (pHdr, ppVersion, pErr)
        SipHeader *pHdr;
	SIP_S8bit **ppVersion;
	SipError *pErr;
#endif
{
        SIP_S8bit *pTempVersion;
	SIPDEBUGFN("Entering function sip_getVersionFromMimeVersionHdr");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
       		return SipFail;

        if (pHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if ( pHdr->pHeader == SIP_NULL)
        {
                *pErr = E_INV_HEADER;
                return SipFail;
        }
        if ( pHdr->dType != SipHdrTypeMimeVersion)
        {
                *pErr = E_INV_TYPE;
                return SipFail;
        }

	if (ppVersion == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
         return SipFail;
    }
#endif
	pTempVersion = ((SipMimeVersionHeader *)(pHdr->pHeader))->pVersion;
	if (pTempVersion == SIP_NULL)
    {
        *pErr = E_NO_EXIST;
        return SipFail;
     }
#ifdef SIP_BY_REFERENCE
	*ppVersion = pTempVersion;
#else
	*ppVersion = (SIP_S8bit *) STRDUPACCESSOR (pTempVersion);
     if (*ppVersion == SIP_NULL)
     {
         *pErr = E_NO_MEM;
         return SipFail;
     }
#endif
     *pErr = E_NO_ERROR;
     SIPDEBUGFN("Exiting function sip_getVersionFromMimeVersionHdr");
     return SipSuccess;
}


/*********************************************************************
** FUNCTION:  sip_bcpt_setVersionInMimeVersionHdr
**********************************************************************

** DESCRIPTION: Sets the Version field in Mime-Version Header.

*********************************************************************/
SipBool sip_bcpt_setVersionInMimeVersionHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pVersion, SipError *pErr)
#else
        (pHdr, pVersion, pErr)
        SipHeader *pHdr;
	SIP_S8bit *pVersion;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempVersion;
#endif
	SIP_S8bit *pVer;
	SIPDEBUGFN("Entering function sip_setVersionInMimeVersionHdr");
#ifndef SIP_NO_CHECK
    if (pErr == SIP_NULL)
          return SipFail;

    if (pHdr == SIP_NULL)
    {
         *pErr = E_INV_PARAM;
          return SipFail;
    }

    if ( pHdr->pHeader == SIP_NULL)
    {
        *pErr = E_INV_HEADER;
         return SipFail;
    }
    if ( pHdr->dType != SipHdrTypeMimeVersion)
    {
        *pErr = E_INV_TYPE;
         return SipFail;
    }
#endif

	pVer = ((SipMimeVersionHeader *)(pHdr->pHeader))->pVersion;

	if ( pVer != SIP_NULL)
    {
        if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pVer), pErr) == SipFail)
            return SipFail;
    }

#ifdef SIP_BY_REFERENCE
	((SipMimeVersionHeader *)(pHdr->pHeader))->pVersion = pVersion;
#else
	if (pVersion == SIP_NULL)
		pTempVersion = SIP_NULL;
	else
	{
		pTempVersion = (SIP_S8bit *) STRDUPACCESSOR(pVersion);
		if (pTempVersion == SIP_NULL)
        {
            *pErr = E_NO_MEM;
             return SipFail;
        }
    }

	((SipMimeVersionHeader *)(pHdr->pHeader))->pVersion = pTempVersion;
#endif

	*pErr  = E_NO_ERROR;
     SIPDEBUGFN("Exiting function sip_setVersionInMimeVersionHdr");
     return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_bcpt_getContentDispositionFromMimeHdr
**********************************************************************

** DESCRIPTION: Gets the Content Disposition field from Mime Header.

*********************************************************************/
SipBool sip_bcpt_getContentDispositionFromMimeHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
        (SipMimeHeader *pMimeHdr, SipHeader **ppContentDisposition, SipError *pErr)
#else
        (SipMimeHeader *pMimeHdr, SipHeader *pContentDisposition, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
        (pMimeHdr, ppContentDisposition, pErr)
        SipMimeHeader *pMimeHdr;
        SipHeader **ppContentDisposition;
        SipError *pErr;
#else
        (pMimeHdr, pContentDisposition, pErr)
        SipMimeHeader *pMimeHdr;
        SipHeader *pContentDisposition;
        SipError *pErr;
#endif
#endif
{
        SIPDEBUGFN("Entering function sip_getContentDispositionFromMimeHdr");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;

        if (pMimeHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
		#ifndef SIP_BY_REFERENCE
        if (pContentDisposition == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

		if ( pContentDisposition->dType != SipHdrTypeContentDisposition )
		{
			*pErr = E_INV_TYPE;
			return SipFail;
		}

		#else
        if (ppContentDisposition == SIP_NULL)
        {
       		 *pErr = E_INV_PARAM;
             return SipFail;
        }
		#endif
#endif
	if (pMimeHdr->pContentDisposition == SIP_NULL)
        {
                *pErr = E_NO_EXIST;
                return SipFail;
        }

#ifndef SIP_BY_REFERENCE

        if (__sip_cloneSipContentDispositionHeader((SipContentDispositionHeader*)(pContentDisposition->pHeader), \
			pMimeHdr->pContentDisposition, pErr) == SipFail)
		{
			return SipFail;
		}
#else
		(* ppContentDisposition)->dType = SipHdrTypeContentDisposition; 
		HSS_LOCKEDINCREF(pMimeHdr->pContentDisposition->dRefCount);
		((*ppContentDisposition)->pHeader) = (SIP_Pvoid ) pMimeHdr->pContentDisposition;
		
#endif
        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getContentDispositionFromMimeHdr");
        return SipSuccess;
}


/*********************************************************************
** FUNCTION: sip_bcpt_getContentTypeFromMimeHdr
**********************************************************************

** DESCRIPTION: Gets the Content Type field from Mime Header.

*********************************************************************/
SipBool sip_bcpt_getContentTypeFromMimeHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
        (SipMimeHeader *pMimeHdr, SipHeader **ppContentType, SipError *pErr)
#else
        (SipMimeHeader *pMimeHdr, SipHeader *pContentType, SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
        (pMimeHdr, ppContentType, pErr)
        SipMimeHeader *pMimeHdr;
        SipHeader **ppContentType;
        SipError *pErr;
#else
        (pMimeHdr, pContentType, pErr)
        SipMimeHeader *pMimeHdr;
        SipHeader *pContentType;
        SipError *pErr;
#endif
#endif
{
        SIPDEBUGFN("Entering function sip_getContentTypeFromMimeHdr");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;

        if (pMimeHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
		}
#ifndef SIP_BY_REFERENCE
        if (pContentType == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

		if ( pContentType->dType != SipHdrTypeContentType )
		{
			*pErr = E_INV_TYPE;
			return SipFail;
		}
#else
        if (ppContentType == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif
#endif
		
	if (pMimeHdr->pContentType == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE

        if (__sip_cloneSipContentTypeHeader((SipContentTypeHeader*)\
				(pContentType->pHeader),pMimeHdr->pContentType, pErr)\
				== SipFail)
		{
			return SipFail;
		}
#else
		(* ppContentType)->dType = SipHdrTypeContentType; 
		HSS_LOCKEDINCREF(pMimeHdr->pContentType->dRefCount);
		((*ppContentType)->pHeader) = (SIP_Pvoid ) pMimeHdr->pContentType;
#endif
        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getContentTypeFromMimeHdr");
        return SipSuccess;
}



/*********************************************************************
** FUNCTION: sip_bcpt_setContentTypeInMimeHdr

**********************************************************************

** DESCRIPTION: Sets the Content Type field in Mime Header.

*********************************************************************/
SipBool sip_bcpt_setContentTypeInMimeHdr
#ifdef ANSI_PROTO
        (SipMimeHeader *pMimeHdr, SipHeader *pHdr, SipError *pErr)
#else
        (pMimeHdr, pHdr, pErr)
        SipMimeHeader *pMimeHdr;
        SipHeader *pHdr;
        SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
		SipContentTypeHeader * pTempCType;
#endif
		SipContentTypeHeader *pContentType;
        SIPDEBUGFN("Entering function sip_setContentTypeInMimeHdr");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;

        if (pMimeHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
		
		if (pHdr!=SIP_NULL)
		{	
			if (pHdr->dType != SipHdrTypeContentType)
			{
				*pErr = E_INV_HEADER;
				return SipFail;
			}
		}	
#endif
	
		if (pMimeHdr->pContentType != SIP_NULL) 
			sip_freeSipContentTypeHeader(pMimeHdr->pContentType);
	
		if (pHdr==SIP_NULL)
		{
			pMimeHdr->pContentType = SIP_NULL;
		}
		else
		{	
			pContentType = (SipContentTypeHeader *) ( pHdr->pHeader);
			if (pContentType==SIP_NULL)
			{
				pMimeHdr->pContentType = SIP_NULL;
			}
			else
			{	
#ifdef SIP_BY_REFERENCE
				HSS_LOCKEDINCREF(pContentType->dRefCount);
				pMimeHdr->pContentType = pContentType;
#else
		        if (pContentType == SIP_NULL)
       	    	     pTempCType = SIP_NULL;
    		    else
       		 	{
					if ( sip_initSipContentTypeHeader(&pTempCType, pErr)\
									== SipFail)
						return SipFail;

					if (__sip_cloneSipContentTypeHeader(pTempCType,\
											pContentType, pErr) == SipFail)
					{
						sip_freeSipContentTypeHeader(pTempCType);
						return SipFail;
					}
       			 }
				pMimeHdr->pContentType = pTempCType;
#endif
			}
		}	
        *pErr  = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_setContentTypeInMimeHdr");
        return SipSuccess;
}
/*********************************************************************
** FUNCTION: sip_bcpt_setContentDispositionInMimeHdr

**********************************************************************

** DESCRIPTION: Sets the Content Disposition field in Mime Header.

*********************************************************************/
SipBool sip_bcpt_setContentDispositionInMimeHdr
#ifdef ANSI_PROTO
        (SipMimeHeader *pMimeHdr, SipHeader *pHdr, SipError *pErr)
#else
        (pMimeHdr, pHdr, pErr)
        SipMimeHeader *pMimeHdr;
        SipHeader *pHdr;
        SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
		SipContentDispositionHeader * pTempCType;
#endif
		SipContentDispositionHeader *pContentDisposition;
        SIPDEBUGFN("Entering function sip_setContentDispositionInMimeHdr");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;

        if (pMimeHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

		if( pHdr == SIP_NULL)
		{
			*pErr = E_INV_PARAM;
			return SipFail;
		}

		if (pHdr->dType != SipHdrTypeContentDisposition)
		{
			*pErr = E_INV_HEADER;
			return SipFail;
		}
#endif
	
		if (pMimeHdr->pContentDisposition != SIP_NULL) 
			sip_freeSipContentDispositionHeader(pMimeHdr->pContentDisposition);

		pContentDisposition = (SipContentDispositionHeader *) ( pHdr->pHeader);

#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pContentDisposition->dRefCount);
		pMimeHdr->pContentDisposition = pContentDisposition;
		
#else
        if (pContentDisposition == SIP_NULL)
                pTempCType = SIP_NULL;
        else
        {
			if ( sip_initSipContentDispositionHeader(&pTempCType, pErr) == SipFail)
				return SipFail;

			if (__sip_cloneSipContentDispositionHeader(pTempCType, pContentDisposition, pErr) == SipFail)
			{
				sip_freeSipContentDispositionHeader(pTempCType);
				return SipFail;
			}
        }


		pMimeHdr->pContentDisposition = pTempCType;
#endif
        *pErr  = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_setContentDispositionInMimeHdr");
        return SipSuccess;
}


/*********************************************************************
** FUNCTION: sip_bcpt_getContentTransEncodingFromMimeHdr  
**********************************************************************

** DESCRIPTION: Gets the Content-Transfer-Encoding field from Mime
** Header.

*********************************************************************/
SipBool sip_bcpt_getContentTransEncodingFromMimeHdr
#ifdef ANSI_PROTO
        (SipMimeHeader *pMimeHdr, SIP_S8bit **ppEncoding, SipError *pErr)
#else
        (pMimeHdr, ppEncoding, pErr)
        SipMimeHeader *pMimeHdr;
        SIP_S8bit **ppEncoding;
        SipError *pErr;
#endif
{
        SIP_S8bit *pTempEncoding;
        SIPDEBUGFN("Entering function sip_getContentTransEncodingFromMimeHdr");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;

        if (pMimeHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }

        if (ppEncoding == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif
        pTempEncoding = pMimeHdr->pContentTransEncoding;
        if (pTempEncoding == SIP_NULL)
        {
                *pErr = E_NO_EXIST;
                return SipFail;
        }
#ifdef SIP_BY_REFERENCE
		*ppEncoding = pTempEncoding;
#else
        *ppEncoding = (SIP_S8bit *) STRDUPACCESSOR (pTempEncoding);
        if (*ppEncoding == SIP_NULL)
        {
                *pErr = E_NO_MEM;
                return SipFail;
        }
#endif
        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getContentTransEncodingFromMimeHdr");
        return SipSuccess;
}



/*********************************************************************
** FUNCTION: sip_bcpt_setContentTransEncodingInMimeHdr  
**********************************************************************

** DESCRIPTION: Sets the Content-Transfer-Encoding field in Mime Header.

*********************************************************************/
SipBool sip_bcpt_setContentTransEncodingInMimeHdr
#ifdef ANSI_PROTO
        (SipMimeHeader *pMimeHdr, SIP_S8bit *pEncoding, SipError *pErr)
#else
        (pMimeHdr, pEncoding, pErr)
        SipMimeHeader *pMimeHdr;
        SIP_S8bit *pEncoding;
        SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
        SIP_S8bit *pTempEncoding;
#endif
        SIPDEBUGFN("Entering function sip_setContentTransEncodingInMimeHdr");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;

        if (pMimeHdr == SIP_NULL)
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif
        if ( pMimeHdr->pContentTransEncoding != SIP_NULL)
        {
                if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(pMimeHdr->pContentTransEncoding)), pErr) == SipFail)
                        return SipFail;
        }
#ifdef SIP_BY_REFERENCE
		pMimeHdr->pContentTransEncoding = pEncoding;
#else
        if (pEncoding == SIP_NULL)
                pTempEncoding = SIP_NULL;
        else
        {
                pTempEncoding = (SIP_S8bit *) STRDUPACCESSOR(pEncoding);
                if (pTempEncoding == SIP_NULL)
                {
                        *pErr = E_NO_MEM;
                        return SipFail;
                }
        }


        pMimeHdr->pContentTransEncoding = pTempEncoding;
#endif
        *pErr  = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_setContentTransEncodingInMimeHdr");
        return SipSuccess;
}

/*******************SipParams ******************************/


/*********************************************************************
** FUNCTION:  sip_getNameFromSipParam
**********************************************************************

** DESCRIPTION: Gets the Name field pValue from SipParam.

*********************************************************************/
SipBool sip_getNameFromSipParam
#ifdef ANSI_PROTO
	( SipParam *pParam, SIP_S8bit **ppName, SipError *pErr)
#else
	( pParam, ppName, pErr )
	  SipParam *pParam;
	  SIP_S8bit **ppName;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getNameFromSipParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( ( pParam == SIP_NULL) || (ppName == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif	
	if( pParam->pName == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE
	*ppName = pParam->pName;
#else
	*ppName = (SIP_S8bit *)STRDUPACCESSOR(pParam->pName);
	if(*ppName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	SIPDEBUGFN ( "Exiting getNameFromSipParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION: sip_setNameInSipParam
**********************************************************************

** DESCRIPTION: Sets the Name field pValue in SipParam.

*********************************************************************/
SipBool sip_setNameInSipParam
#ifdef ANSI_PROTO
	( SipParam *pParam, SIP_S8bit *pName, SipError *pErr)
#else
	( pParam, pName, pErr )
	  SipParam *pParam;
	  SIP_S8bit *pName;
	  SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempName;
#endif

	SIPDEBUGFN ( "Entering setNameInSipParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pParam == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pParam->pName != SIP_NULL ) 
	{
		 if( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pParam->pName)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pParam->pName = pName;
#else
	if( pName == SIP_NULL)
		pTempName = SIP_NULL;
	else
	{
		pTempName = (SIP_S8bit *)STRDUPACCESSOR(pName);
		if(pTempName == SIP_NULL)
			return SipFail;
	}
	

	pParam->pName = pTempName; 
#endif
	SIPDEBUGFN ( "Exiting setNameInSipParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}



/*********************************************************************
** FUNCTION:  sip_getValueCountFromSipParam
**********************************************************************
**
** DESCRIPTION:	Gets the number of Value nodes present in Sip 
** SipParam structure.
**
*********************************************************************/
SipBool sip_getValueCountFromSipParam
#ifdef ANSI_PROTO
	( SipParam	*pParam,
	  SIP_U32bit	*pIndex,
	  SipError	*pErr  )
#else 
	( pParam,pIndex,pErr)
	  SipParam 	*pParam;
	  SIP_U32bit 	*pIndex;
	  SipError 	*pErr;
#endif
{

	SIPDEBUGFN ( "Entering GeValueCountFromSipParam");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL ) 
		return SipFail;
	
	if ( (pParam == SIP_NULL) || ( pIndex == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pParam->slValue), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}
	
	SIPDEBUGFN ( "Exiting GeValueCountFromSipParam");
	*pErr = E_NO_ERROR;
	return SipSuccess;	
}

/*********************************************************************
** FUNCTION:  sip_getValueAtIndexFromSipParam
**********************************************************************
**
** DESCRIPTION:	Gets the Value at a specified index ( starting from 
** 0 ) from Param strucutre.
**
*********************************************************************/
SipBool sip_getValueAtIndexFromSipParam
#ifdef ANSI_PROTO
	( SipParam 	*pParam,
	  SIP_S8bit 	**ppValue,
	  SIP_U32bit 	index, 
	  SipError 	*pErr )
#else 
	( pParam,ppValue,index,pErr)
	  SipParam *pParam;
	  SIP_S8bit **ppValue;
	  SIP_U32bit index;
	  SipError *pErr;
#endif
{
	SIP_Pvoid pElementFromList;
#ifndef SIP_BY_REFERENCE
	SIP_U32bit size;
#endif

	SIPDEBUGFN ( "Entering GeValueAtIndexFromSipParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pParam == SIP_NULL) || (ppValue == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pParam->slValue), index, &pElementFromList, pErr) == SipFail)
		return SipFail;
	
	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;		
	}

#ifdef SIP_BY_REFERENCE
	*ppValue = (SIP_S8bit*)pElementFromList;
#else
	size = strlen( (SIP_S8bit * )pElementFromList);
	*ppValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, size +1, pErr);
	if(*ppValue == SIP_NULL)
		return SipFail;

	strcpy(*ppValue, (SIP_S8bit*)pElementFromList);
#endif
	SIPDEBUGFN ( "Exiting GeValueAtIndexFromSipParam");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:  sip_setValueAtIndexInSipParam
**********************************************************************
**
** DESCRIPTION:	Sets the Value pValue at a specified index 
** ( starting from 0 )in Sip SipParam strucutre.
**
*********************************************************************/
SipBool sip_setValueAtIndexInSipParam
#ifdef ANSI_PROTO
	( SipParam 	*pParam, 
	  SIP_S8bit 	*pValue,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pParam,pValue,index,pErr)
	  SipParam 	*pParam;
	  SIP_S8bit 	*pValue;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
		
	SIP_S8bit * pElementFromList;
	SipError tempErr;		/* used in freeing memory after an pError has happened */

	SIPDEBUGFN ( "Entering SeValueAtIndexInSipParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pParam == SIP_NULL) 
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pValue == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = 	pValue;
#else
		
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,strlen(pValue) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;
	
		strcpy(pElementFromList, pValue);
#endif
	}

	if( sip_listSetAt(&(pParam->slValue), index, pElementFromList, pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), &tempErr);
		return SipFail;
	}
	
	SIPDEBUGFN ( "Exiting SeValueAtIndexinSipParam");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sip_insertValueAtIndexInSipParam
**********************************************************************
**
** DESCRIPTION:	Inserts a Value at a specified index 
** ( starting from 0 )  in  SipParam strucutre.
**
*********************************************************************/
SipBool sip_insertValueAtIndexInSipParam
#ifdef ANSI_PROTO
	( SipParam 	*pParam,
	  SIP_S8bit 	*pValue,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pParam,pValue,index,pErr)
	  SipParam 	*pParam;
	  SIP_S8bit 	*pValue;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
		
	SIP_S8bit * pElementFromList;
	SipError tempErr;		/* used in freeing memory after an pError has happened */


	SIPDEBUGFN ( "Entering InserValueAtIndexInSipParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pParam == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the Value Headers structure/char*  */		
	if ( pValue == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = 	pValue;
#else
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,strlen(pValue) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList, pValue);
#endif
	}

	if( sip_listInsertAt(&(pParam->slValue), index, pElementFromList, pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), &tempErr);
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting InserValueAtIndexinSipParam");
	
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sip_deleteValueAtIndexInSipParam
**********************************************************************
**
** DESCRIPTION: Deletes a Value pValue at a specified index 
** ( starting from 0 )  in SipParam strucutre.
**
*********************************************************************/
SipBool sip_deleteValueAtIndexInSipParam
#ifdef ANSI_PROTO
	( SipParam 	*pParam,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pParam,index,pErr)
	  SipParam 	*pParam;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{

	SIPDEBUGFN ( "Entering DeleteiValueAtIndexInSipParam");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
		
	if ( pParam == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pParam->slValue), index, pErr) == SipFail)
		return SipFail;
	
	SIPDEBUGFN ( "Exiting DeleteValueAtIndexinSipParam");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
