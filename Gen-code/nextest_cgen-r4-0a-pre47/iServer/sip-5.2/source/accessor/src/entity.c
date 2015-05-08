/************************************************************
** FUNCTION:
	This file implements the entity pHeader accessor APIs.
**
*************************************************************
**
** FILENAME:
** entity.c
** DESCRIPTION
**
**
**   DATE              NAME                  REFERENCE
**  -----             -------               ------------
** 17Nov99	B.Borthakur            	Original   
**
** Copyright 1999, Hughes Software Systems, Ltd.
*************************************************************/

	

#include "sipcommon.h"
#include "sipstruct.h"
#include "sipinit.h"
#include "sipfree.h"
#include "portlayer.h"
#include "siplist.h"
#include "entity.h"
#include "sipinternal.h"
#include "sipclone.h"

/*********************************************************************
** FUNCTION: sip_getLengthFromContentLengthHdr

**********************************************************************

** DESCRIPTION:
	Gets the dLength field from Content-Length Header.

*********************************************************************/

SipBool sip_getLengthFromContentLengthHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,      /*  Content Length pHeader */
	  SIP_U32bit * dLength,
	  SipError * err)
#else
	( hdr, dLength, err )
	  SipHeader * hdr;     /* Content Length pHeader */
	  SIP_U32bit * dLength;
	  SipError * err;

#endif
{
	SIPDEBUGFN ( "Entering getLengthFromContentLengthHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dLength == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentLength)
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
	*dLength = ( (SipContentLengthHeader *) (hdr->pHeader) )->dLength; 
	
	SIPDEBUGFN ( "Exiting getLengthFromContentLengthHdr");
	
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION: sip_setLengthInContentLengthHdr

**********************************************************************

** DESCRIPTION:
	Sets the dLength field in Content-Length Header.

*********************************************************************/
SipBool sip_setLengthInContentLengthHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,    /* Content Length pHeader */
	  SIP_U32bit dLength,
	  SipError * err)
#else
	( hdr, dLength, err )
	  SipHeader * hdr;    /* Content Length pHeader */
	  SIP_U32bit dLength;
	  SipError * err;
#endif
{
	SIPDEBUGFN ( "Entering setLengthInContentLengthHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentLength)
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
	( (SipContentLengthHeader *) (hdr->pHeader) )->dLength = dLength;

	SIPDEBUGFN ( "Exiting setLengthInContentLengthHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}



/*********************************************************************
** FUNCTION: sip_getMediaTypeFromContentTypeHdr

**********************************************************************

** DESCRIPTION:
	Gets the slMedia dType field from Content-Type Header.

*********************************************************************/
SipBool sip_getMediaTypeFromContentTypeHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,	/*Content Type pHeader */
	  SIP_S8bit ** dType,
	  SipError * err)
#else
	( hdr, dType, err )
	  SipHeader * hdr;      /*Content Type pHeader */
	  SIP_S8bit ** dType;
	  SipError * err;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_media_type;

	SIPDEBUGFN ( "Entering getMediaTypeFromContentTypeHdr"); 
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dType == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentType)
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
	temp_media_type = ( (SipContentTypeHeader *) (hdr->pHeader) )->pMediaType; 
	
	if( temp_media_type == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}	
#ifdef SIP_BY_REFERENCE
	*dType=temp_media_type;
#else
	dLength = strlen(temp_media_type );
	*dType = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *dType == SIP_NULL )
		return SipFail;
	
	strcpy( *dType, temp_media_type );
#endif

	SIPDEBUGFN ( "Exiting getMediaTypeFromContentTypeHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION: sip_setMediaTypeInContentTypeHdr

**********************************************************************

** DESCRIPTION:
	Sets the slMedia dType field in Content-Type Header.

*********************************************************************/
SipBool sip_setMediaTypeInContentTypeHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,	/*Content Type pHeader */
	  SIP_S8bit * dType,
	  SipError * err)
#else
	( hdr, dType, err )	/*Content Type pHeader */
	  SipHeader * hdr;
	  SIP_S8bit * dType;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit 		dLength;
#endif
	SIP_S8bit 		*temp_media_type;
	SipContentTypeHeader 	*temp_cont_typ_hdr;

	SIPDEBUGFN ( "Entering setMediaTypeInContentTypeHdr"); 
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentType)
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
	if( dType == SIP_NULL)
		temp_media_type = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_media_type=dType;
#else
		dLength = strlen( dType );
		temp_media_type = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_media_type == SIP_NULL )
			return SipFail;
		strcpy( temp_media_type, dType );
#endif
	}
	
	temp_cont_typ_hdr = (SipContentTypeHeader *) (hdr->pHeader);

	if ( temp_cont_typ_hdr->pMediaType != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_cont_typ_hdr->pMediaType, err)== SipFail )
			return SipFail;
	}

	temp_cont_typ_hdr->pMediaType = temp_media_type; 

	SIPDEBUGFN ( "Exiting setMediaTypeInContentTypeHdr"); 

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_getEncodingFromContentEncodingHdr

**********************************************************************

** DESCRIPTION:
	Gets the pEncoding field from content pEncoding Header.

*********************************************************************/
SipBool sip_getEncodingFromContentEncodingHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,	/* Content pEncoding pHeader */
	  SIP_S8bit ** pEncoding,
	  SipError * err)
#else
	( hdr, pEncoding, err )
	  SipHeader * hdr;	/* Content pEncoding pHeader */
	  SIP_S8bit ** pEncoding;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_encoding;
	
	SIPDEBUGFN ( "Entering getEncodingFromContentEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pEncoding == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentEncoding)
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
	temp_encoding = ( (SipContentEncodingHeader *) (hdr->pHeader) )->pEncoding; 
	
	if( temp_encoding == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}	

#ifdef SIP_BY_REFERENCE
	*pEncoding=temp_encoding;
#else
	dLength = strlen(temp_encoding);
	*pEncoding = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pEncoding == SIP_NULL )
		return SipFail;

	strcpy( *pEncoding, temp_encoding );
#endif

	SIPDEBUGFN ( "Exiting getEncodingFromContentEncodingHdr");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION: sip_setEncodingInContentEncodingHdr

**********************************************************************

** DESCRIPTION:
	Sets the pEncoding field in content pEncoding Header.

*********************************************************************/
SipBool sip_setEncodingInContentEncodingHdr
#ifdef ANSI_PROTO
	( SipHeader * hdr,	/* Content pEncoding pHeader */
	  SIP_S8bit * pEncoding,
	  SipError * err)
#else
	( hdr, pEncoding, err )
	  SipHeader * hdr;	/* Content pEncoding pHeader */
	  SIP_S8bit * pEncoding;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit 			dLength;
#endif
	SIP_S8bit 			*temp_encoding;
	SipContentEncodingHeader	*temp_cont_enc_hdr;

	SIPDEBUGFN ( "Entering setEncodingFrom"); 
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentEncoding)
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
	if( pEncoding == SIP_NULL)
		temp_encoding = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_encoding=pEncoding;
#else
		dLength = strlen( pEncoding );
		temp_encoding = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_encoding == SIP_NULL )
			return SipFail;

		strcpy( temp_encoding, pEncoding );
#endif
	}
	
	temp_cont_enc_hdr = (SipContentEncodingHeader *) (hdr->pHeader);

	if ( temp_cont_enc_hdr->pEncoding != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_cont_enc_hdr->pEncoding, err)== SipFail )
			return SipFail;
	}

	temp_cont_enc_hdr->pEncoding = temp_encoding; 

	SIPDEBUGFN ( "Exiting setEncodingFrom"); 

	*err = E_NO_ERROR;
	return SipSuccess;
}




/*********************************************************************
** FUNCTION:  sip_getParamCountFromContentTypeHdr
**********************************************************************
**
** DESCRIPTION:
**	Gets the number of SipParam structures present in the ContentTypeHdr.
**
*********************************************************************/

SipBool sip_getParamCountFromContentTypeHdr
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
	SipContentTypeHeader *pContHdr;

	SIPDEBUGFN ( "Entering getParamCountFromContentTypeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pIndex == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentType)
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
	pContHdr = (SipContentTypeHeader*) (pHdr->pHeader);

	if (sip_listSizeOf(&(pContHdr->slParams), pIndex , pErr) == SipFail )
	{
		return SipFail;
	}
	
	SIPDEBUGFN ( "Exiting getParamCountFromContentTypeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;	
}

/*********************************************************************
** FUNCTION: sip_getParamAtIndexFromContentTypeHdr
**********************************************************************
**
** DESCRIPTION:
**	Gets the SipParam structure at a specified index 
** ( starting from 0 ) from the ContentTypeHdr.
**
*********************************************************************/
SipBool sip_getParamAtIndexFromContentTypeHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader 	*pHdr, SipParam **pParam, SIP_U32bit index, SipError *pErr )
#else
	( SipHeader 	*pHdr, SipParam *pParam, SIP_U32bit	index, SipError *pErr )
#endif	
#else 
#ifdef SIP_BY_REFERENCE
	( pHdr,pParam,index,pErr)
	  SipHeader 	*pHdr;
	  SipParam **pParam;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#else
	( pHdr,pParam,index,pErr)
	  SipHeader 	*pHdr;
	  SipParam *pParam;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
#endif
{
	SIP_Pvoid 	pElementFromList;
	SipContentTypeHeader * pContHdr;

	SIPDEBUGFN ( "Entering getParamAtIndexFromContentTypeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pParam == SIP_NULL)
#else
	if(  pParam == SIP_NULL)
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
	if( pHdr->dType != SipHdrTypeContentType)
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
	pContHdr = (SipContentTypeHeader*)(pHdr->pHeader);

	if( sip_listGetAt(&(pContHdr->slParams), index, &pElementFromList, pErr) == SipFail)
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

	SIPDEBUGFN ( "Exiting getParamAtIndexFromContentTypeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_setParamAtIndexInContentTypeHdr
**********************************************************************
**
** DESCRIPTION:
**	Sets the SipParam structure at a specified index 
** ( starting from 0 ) in the ContentTypeHdr.
**
*********************************************************************/
SipBool sip_setParamAtIndexInContentTypeHdr
#ifdef ANSI_PROTO
	( SipHeader 	*pHdr, 
	  SipParam	*pParam,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pHdr,pParam,index,pErr)
	  SipHeader 	*pHdr;
	  SipParam	*pParam;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElementInList;

	SIPDEBUGFN ( "Entering setParamAtIndexInContentTypeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentType)
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

	if( sip_listSetAt(&(((SipContentTypeHeader*)(pHdr->pHeader))->slParams),\
		 index, (SIP_Pvoid) pElementInList, pErr) == SipFail)
	{
		if ( pElementInList != SIP_NULL )
			 sip_freeSipParam(pElementInList ); 
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pParam->dRefCount);
#endif
	SIPDEBUGFN ( "Exiting setParamAtIndexInContentTypeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_insertParamAtIndexInContentTypeHdr
**********************************************************************
**
** DESCRIPTION:
**	Inserts the SipParam structure at a specified index 
** ( starting from 0 ) in the ContentTypeHdr. 
**
*********************************************************************/
SipBool sip_insertParamAtIndexInContentTypeHdr
#ifdef ANSI_PROTO
	( SipHeader 	*pHdr,
	  SipParam	*pParam,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pHdr,pParam,index,pErr)
	  SipHeader 	*pHdr;
	  SipParam	*pParam;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
	SipParam 	*pElementInList;
	SipContentTypeHeader *pContHdr;

	SIPDEBUGFN ( "Entering InsertParamAtIndexInContentTypeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentType)
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
	
	pContHdr = (SipContentTypeHeader *) (pHdr->pHeader);;
	if( sip_listInsertAt(&(pContHdr->slParams), index, pElementInList, pErr) == SipFail)
	{
		if ( pElementInList != SIP_NULL )
			sip_freeSipParam(pElementInList);
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pParam->dRefCount);
#endif

	SIPDEBUGFN ( "Exiting InsertParamAtIndexInContentTypeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION: sip_deleteParamAtIndexInContentTypeHdr
**********************************************************************
**
** DESCRIPTION:
**	Deletes a SipParam structure at a specified index 
** (starting from 0 ) in the ContentTypeHdr.
**
*********************************************************************/
SipBool sip_deleteParamAtIndexInContentTypeHdr
#ifdef ANSI_PROTO
	( SipHeader 	*pHdr,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else 
	( pHdr,index,pErr)
	  SipHeader 	*pHdr;
	  SIP_U32bit 	index;
	  SipError 	*pErr;
#endif
{
	SipContentTypeHeader * pContHdr;

	SIPDEBUGFN ( "Entering deleteParamAtIndexInContentTypeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentType)
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
	pContHdr = (SipContentTypeHeader *)(pHdr->pHeader);
	
	if( sip_listDeleteAt(&(pContHdr->slParams), index, pErr) == SipFail)
		return SipFail;
	
	SIPDEBUGFN ( "Exiting deleteParamAtIndexInContentTypeHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}



