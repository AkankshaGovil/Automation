#include <stdlib.h>
#include <ctype.h>
#include "siplist.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "sipdecode.h"
#include "sipdecodeintrnl.h"
#include "siptrace.h"
#include "siptimer.h"
#include "sipstatistics.h"
#ifdef SIP_CCP
#include "ccpstruct.h"
#include "ccpfree.h"
#include "ccpinit.h"
#endif
#include "rprinit.h"
#include "rprfree.h"
#include "sipbcptinit.h"
#include "sipbcptfree.h"
#include "sipparserclone.h"



/*********************************************************
** FUNCTION: __sipParser_cloneSipParam
**
** DESCRIPTION: This function copies the contents of a SIP param
**		structure into another SIP param structure
**
**********************************************************/

SipBool __sipParser_cloneSipParam
#ifdef ANSI_PROTO
	(SipParam *pDest, SipParam *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipParam *pDest;
	SipParam *pSource;
	SipError *pErr;
#endif
{	
	SIP_U32bit index, count;
	SIP_S8bit *pValue, *pDupValue;
	SipError	tempErr;

	SIPDEBUGFN("Entering function sip_cloneSipParam");
	/* duplicating siplist of slParams */

	/* deleting siplist of slParams */
	if ( sip_listDeleteAll(&( pDest->slValue ), pErr ) == SipFail )
		return SipFail;

	/* copying */
	if ( sip_listInit(& (pDest->slValue),(pSource->slValue).freefunc,pErr)\
					==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( pSource->slValue), &count, pErr) == SipFail )
		return SipFail;

	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(&( pSource->slValue),index, \
			(SIP_Pvoid * ) (&pValue), pErr) == SipFail )
			return SipFail;

		if ( pValue == SIP_NULL )
			pDupValue = SIP_NULL;
		else
		{
			pDupValue = (SIP_S8bit *)STRDUPDECODE(pValue);
			if ( pDupValue == SIP_NULL)
			{
				*pErr = E_NO_MEM;
				return SipFail;
			}
		}

		if ( sip_listAppend(&( pDest->slValue ), pDupValue, pErr) == SipFail )
		{
			if ( pDupValue != SIP_NULL )
				sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(pDupValue), &tempErr);
			return SipFail;
		}
	}

	/* duplicating Name */

	/* freeing */
	if ( pDest->pName != SIP_NULL)
	{
		sip_memfree(DECODE_MEM_ID, (SIP_Pvoid *)(&pDest->pName), pErr);
		pDest->pName = SIP_NULL;
	}

	if (pSource->pName == SIP_NULL)
		pDest->pName = SIP_NULL;
	else
	{
		pDest->pName = (SIP_S8bit*)STRDUPDECODE(pSource->pName);
		if ( pDest->pName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneSipParam");
	return SipSuccess;	
}

/******************************************************************
**
** FUNCTION:  __sipParser_cloneSipContentTypeHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from the 
** ContentTypeHeader  structures "source" to "dest".
**
******************************************************************/
SipBool __sipParser_cloneSipContentTypeHeader
#ifdef ANSI_PROTO
	(SipContentTypeHeader *dest, SipContentTypeHeader *source, SipError *err)
#else
	(dest, source, err)
	SipContentTypeHeader *dest;
	SipContentTypeHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SipParam *pParam, *pDupParam;
	SIP_U32bit index, count;

	SIPDEBUGFN("Entering function sip_cloneSipContentTypeHeader");
	if (dest->pMediaType != SIP_NULL)
		sip_freeString(dest->pMediaType);
	if(sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	if (source->pMediaType == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPDECODE(source->pMediaType)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	} 
	dest->pMediaType = temp;	
	
	/* copying siplist of SipParmas */
	if ( sip_listInit(& (dest->slParams ),(source->slParams).freefunc,err)\
					==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( source->slParams ), &count, err) == SipFail )
		return SipFail;

	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(&( source->slParams ),index, \
					(SIP_Pvoid * ) (&pParam), err) == SipFail )
			return SipFail;

		if ( pParam == SIP_NULL )
			pDupParam = SIP_NULL;
		else
		{
			sip_initSipParam(&pDupParam,err);

			if ( pDupParam == SIP_NULL )
				return SipFail;

			if ( __sipParser_cloneSipParam( pDupParam, pParam, err) == SipFail )
			{
				sip_freeSipParam(pDupParam);
				return SipFail;
			}
		}

		if ( sip_listAppend(&( dest->slParams ), pDupParam, err) == SipFail )
		{
			if ( pDupParam != SIP_NULL )
				sip_freeSipParam(pDupParam);
			return SipFail;
		}
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneSipContentTypeHeader");
	return SipSuccess;
}

/*********************************************************
** FUNCTION: __sipParser_bcpt_cloneSipMimeHeader
**
** DESCRIPTION:  This function does a deep copy of all the fields 
** of Source structure to Dest structure.
**
**********************************************************/
SipBool __sipParser_bcpt_cloneSipMimeHeader
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

	SIPDEBUGFN("Entering function sip_bcpt_cloneSipMimeHeader");

	/* cleaning up the destination structure */

	/* content tranfer Encoding */
	if ( pDest->pContentTransEncoding != SIP_NULL )
		sip_memfree(DECODE_MEM_ID, (SIP_Pvoid *) (&pDest->pContentTransEncoding), pErr);
	/* content id */
	if ( pDest->pContentId != SIP_NULL )
		sip_memfree(DECODE_MEM_ID, (SIP_Pvoid *) (&pDest->pContentId), pErr);

	/* content description */
	if ( pDest->pContentDescription != SIP_NULL )
		sip_memfree(DECODE_MEM_ID, (SIP_Pvoid *) (&pDest->pContentDescription), pErr);

	/* content Type */
	if ( pDest->pContentType != SIP_NULL )
	{
		sip_freeSipContentTypeHeader(pDest->pContentType);
		pDest->pContentType = SIP_NULL;
	}

	/* Additional MimeHeaders */
	if ( sip_listDeleteAll( &(pDest->slAdditionalMimeHeaders), pErr) == SipFail)
		return SipFail;


	/* duplication begins here  */

	/* content transfer Encoding */
	if ( pSource->pContentTransEncoding == SIP_NULL )
		pDest->pContentTransEncoding = SIP_NULL;
	else
	{
		pDest->pContentTransEncoding =\
					 (SIP_S8bit *)STRDUPDECODE(pSource->pContentTransEncoding);
		if ( pDest->pContentTransEncoding == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}		
	}

	/* content id */
	if ( pSource->pContentId == SIP_NULL )
		pDest->pContentId = SIP_NULL;
	else
	{
		pDest->pContentId = (SIP_S8bit *)STRDUPDECODE(pSource->pContentId);
		if ( pDest->pContentId == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}		
	}

	/* content description */
	if ( pSource->pContentDescription == SIP_NULL )
		pDest->pContentDescription = SIP_NULL;
	else
	{
		pDest->pContentDescription = \
				(SIP_S8bit *)STRDUPDECODE(pSource->pContentDescription);
		if ( pDest->pContentDescription == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}		
	}

	/* content type */
	if ( pSource->pContentType == SIP_NULL )
		pDest->pContentType = SIP_NULL;
	else
	{
		if ( sip_initSipContentTypeHeader(&(pDest->pContentType), pErr)\
															==SipFail)
			return SipFail;

		if ( __sipParser_cloneSipContentTypeHeader(pDest->pContentType,\
								 pSource->pContentType, pErr) == SipFail)
		{
			sip_freeSipContentTypeHeader(pDest->pContentType);
			return SipFail;
		}
	}

	/* copying siplist of additional mime headers */
	if ( sip_listInit(& (pDest->slAdditionalMimeHeaders ),\
				(pSource->slAdditionalMimeHeaders).freefunc,pErr)==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( pSource->slAdditionalMimeHeaders ), \
											&count, pErr) == SipFail )
		return SipFail;

	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(&( pSource->slAdditionalMimeHeaders ),\
						index, (SIP_Pvoid * ) (&pMime), pErr) == SipFail )
			return SipFail;

		if ( pMime == SIP_NULL )
			pDupMime = SIP_NULL;
		else
		{
			pDupMime = (SIP_S8bit*)STRDUPDECODE(pMime);
			if ( pDupMime == SIP_NULL )
			{
				*pErr = E_NO_MEM;
				return SipFail;
			}
			
		}

		if ( sip_listAppend(&( pDest->slAdditionalMimeHeaders ), \
											pDupMime, pErr) == SipFail )
		{
			if ( pDupMime != SIP_NULL )
				sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(pDupMime), &tempErr);
			return SipFail;
		}
	}
	


	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_bcpt_cloneSipMimeHeader");
	return SipSuccess;	
}
/********************************************************************
**
** FUNCTION:  __validateSipAddrSpecType
**
** DESCRIPTION: This function returns SipSuccess if "Type" is 
** among the defined en_AddType's ; else it returns SipFail;
**
********************************************************************/

SipBool __validateSipAddrSpecType
#ifdef ANSI_PROTO
	(en_AddrType dType, SipError *err)
#else
	(dType, err)
	en_AddrType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipAddrSpecType");
	switch (dType)
	{
		case	SipAddrSipUri	:
		case	SipAddrSipSUri	:
		case	SipAddrReqUri	:
		case	SipAddrAny	:break;
		default			:*err = E_INV_TYPE;
					 return SipFail; 
	}
	
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function validateSipAddrSpecType");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_cloneSipUrl
**
** DESCRIPTION:  This function makes a deep copy of the 
** fileds from the Url structures "source" to "dest".
**
**********************************************************/

SipBool __sipParser_cloneSipUrl
#ifdef ANSI_PROTO
	(SipUrl *to,
	SipUrl *from,
	SipError *err)
#else
	( to,from, err )
	SipUrl *to;
	SipUrl *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;

	SIPDEBUGFN( "Entering sip_cloneSipUrl");
	
	if((from==SIP_NULL)||(to==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	/* Cleaning up to */
	if(to->pUser!=SIP_NULL)
		if(sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(to->pUser),err)==SipFail)
			return SipFail;
	if(to->pPassword!=SIP_NULL)
		if(sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(to->pPassword),err)==SipFail)
			return SipFail;
	if(to->pHost!=SIP_NULL)
		if(sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(to->pHost),err)==SipFail)
			return SipFail;
	if(to->pHeader!=SIP_NULL)
		if(sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(to->pHeader),err)==SipFail)
			return SipFail;
	if(to->dPort!=SIP_NULL)
		if(sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(to->dPort),err)==SipFail)
			return SipFail;
	if(sip_listDeleteAll(&(to->slParam),err)==SipFail)
		return SipFail;
	/* Cleaned up to */

	if(from->pUser== SIP_NULL)
	{
		to->pUser=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pUser );

		to->pUser=( SIP_S8bit * )fast_memget(DECODE_MEM_ID,dLength+1,err);
		if ( to->pUser == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pUser, from->pUser );
	}
	if(from->pPassword== SIP_NULL)
	{
		to->pPassword=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pPassword );

		to->pPassword=( SIP_S8bit * )fast_memget(DECODE_MEM_ID,dLength+1,err);
		if ( to->pPassword == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pPassword, from->pPassword );
	}
	if(from->pHost== SIP_NULL)
	{
		to->pHost=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pHost );

		to->pHost=( SIP_S8bit * )fast_memget(DECODE_MEM_ID,dLength+1,err);
		if ( to->pHost == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pHost, from->pHost );
	}
	if(from->dPort== SIP_NULL)
	{
		to->dPort=SIP_NULL;
	}
	else
	{
		dLength = sizeof(SIP_U16bit);

		to->dPort=( SIP_U16bit * )fast_memget(DECODE_MEM_ID,dLength,err);
		if ( to->dPort == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		*to->dPort=*from->dPort;
	}

	/* copying siplist of SipParam */
	if(__sipParser_cloneSipParamList(&(to->slParam),&(from->slParam),err)\
										==SipFail)
		return SipFail;
	
	if(from->pHeader== SIP_NULL)
	{
		to->pHeader=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pHeader );

		to->pHeader=( SIP_S8bit * )fast_memget(DECODE_MEM_ID,dLength+1,err);
		if ( to->pHeader == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pHeader, from->pHeader );
	}

	SIPDEBUGFN( "Exitting sip_cloneSipUrl");	
	
	*err=E_NO_ERROR;
	return SipSuccess;

}


/******************************************************************
**
** FUNCTION:  __sipParser_cloneSipAddrSpec
**
** DESCRIPTION:   This function makes a deep copy of the fileds 
** from the AddrSpec structures "source" to "dest".
**
******************************************************************/
SipBool __sipParser_cloneSipAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *dest, SipAddrSpec *source, SipError *err)
#else
	(dest, source, err)
	SipAddrSpec *dest;
	SipAddrSpec *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function sip_cloneSipAddrSpec");

	if (__validateSipAddrSpecType(source->dType, err) == SipFail)
		return SipFail;

	switch (dest->dType)
	{
		case	SipAddrReqUri	:if ((dest->u).pUri != SIP_NULL)
						sip_freeString((dest->u).pUri);
					 break;
		case	SipAddrSipUri	:
		case	SipAddrSipSUri  :
					if ((dest->u).pSipUrl != SIP_NULL)
					 	sip_freeSipUrl((dest->u).pSipUrl);
					 break;
		case	SipAddrAny	:break;
		default			:*err = E_INV_TYPE;
					 return SipFail;

	}

	dest->dType = source->dType;
	switch (source->dType)
	{
		case	SipAddrReqUri	:if ((source->u).pUri == SIP_NULL)
						temp = SIP_NULL;
					 else
					 {
						temp = (SIP_S8bit *)STRDUPDECODE((source->u).pUri);
						if (temp == SIP_NULL)
						{
							*err = E_NO_MEM;
							return SipFail;
						}
					 }
					 (dest->u).pUri = temp;
					 break;

		case	SipAddrSipUri	:
		case	SipAddrSipSUri	:
					if ((source->u).pSipUrl == SIP_NULL)
						(dest->u).pSipUrl = SIP_NULL;
					 else
					 {
						if (sip_initSipUrl(&((dest->u).pSipUrl), err) \
																== SipFail)
							return SipFail;
						if (__sipParser_cloneSipUrl((dest->u).pSipUrl,\
										 (source->u).pSipUrl, err) == SipFail)
						{
							sip_freeSipUrl((dest->u).pSipUrl);
							return SipFail;
						}
					 }
					 break;

		case	SipAddrAny	:*err = E_INV_PARAM;
					 return SipFail;

		default			:*err = E_INV_TYPE;
					 return SipFail;

	}
	
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneSipAddrSpec");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sipParser_cloneSipParamList
**
** DESCRIPTION:  This function makes a deep copy of a 
** SipList of SipParam from the "source" to "dest".
**
******************************************************************/
SipBool __sipParser_cloneSipParamList
#ifdef ANSI_PROTO
	(SipList *dest, SipList *source, SipError *err)
#else
	(dest, source, err)
	SipList *dest;
	SipList *source;
	SipError *err;
#endif
{
	SipParam *temp_param, *clone_param;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function sip_cloneSipParamList");
	/* copying siplist of SipParam */
	if ( sip_listDeleteAll(dest , err ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(source, &count, err) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(source,index, \
					(SIP_Pvoid * ) (&temp_param), err) == SipFail )
		{
			return SipFail;
		}
		if ( temp_param == SIP_NULL )
			clone_param = SIP_NULL;
		else
		{
			if(sip_initSipParam(&clone_param,err)==SipFail)
			{
				return SipFail;
			}
			
			if ( clone_param == SIP_NULL )
				return SipFail;

			if ( __sipParser_cloneSipParam(  clone_param, temp_param,err) \
																== SipFail )
			{
				sip_memfree(DECODE_MEM_ID, (SIP_Pvoid*)&(clone_param),err);
				return SipFail;
			}
		}

		if ( sip_listAppend(dest, clone_param, err) == SipFail )
		{
			if ( clone_param != SIP_NULL )
				sip_memfree(DECODE_MEM_ID,(SIP_Pvoid*)&(clone_param), err);
			return SipFail;
		}
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneSipParamList");
	return SipSuccess;	
}
	
/******************************************************************
**
** FUNCTION:  __sipParser_cloneSipStringList
**
** DESCRIPTION:  This function makes a deep copy of a 
** SipList of SIP_S8bit *  from the "source" to "dest".
**
******************************************************************/
SipBool __sipParser_cloneSipStringList
#ifdef ANSI_PROTO
	(SipList *dest, SipList *source, SipError *err)
#else
	(dest, source, err)
	SipList *dest;
	SipList *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp_string, *clone_string;
	SIP_Pvoid temp;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function sip_cloneStringList");
	if (sip_listDeleteAll(dest, err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf (source, &count, err) == SipFail )
			return SipFail;

	for (index = 0; index < count; index ++)
	{
		if (sip_listGetAt(source, index, &temp, err) == SipFail)
			return SipFail;
		temp_string = (SIP_S8bit *)temp;
		if (temp_string == SIP_NULL)
			clone_string = SIP_NULL;
		else
		{
			clone_string = (SIP_S8bit *)STRDUPDECODE(temp_string);
			if (clone_string == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}	
		}/* end of else */
		if (sip_listAppend(dest, (SIP_Pvoid)clone_string, err) == SipFail)
		{
			sip_freeString(clone_string);	
			return SipFail;
		}
	} /* end of for */

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneStringList");
	return SipSuccess;	
}

/******************************************************************
**
** FUNCTION:  __sipParser_cloneSipFromHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds 
** from the FromHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sipParser_cloneSipFromHeader
#ifdef ANSI_PROTO
	(SipFromHeader *dest, SipFromHeader *source, SipError *err)
#else
	(dest, source, err)
	SipFromHeader *dest;
	SipFromHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function sip_cloneSipFromHeader");
	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);
	if (dest->pAddrSpec != SIP_NULL)
		sip_freeSipAddrSpec(dest->pAddrSpec);
	if (sip_listDeleteAll(&(dest->slTag), err) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	/* Duplicating DispName */
	if (source->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPDECODE(source->pDispName);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pDispName = temp;
	/* Duplicating AddrSpec */
	if (__validateSipAddrSpecType((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), \
							(source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (__sipParser_cloneSipAddrSpec(dest->pAddrSpec, \
										source->pAddrSpec, err) == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}
	/* Duplicating Tag */
	if (__sipParser_cloneSipStringList (&(dest->slTag), \
										&(source->slTag), err) == SipFail)
		return SipFail;

	/* clone Param */
	if (__sipParser_cloneSipParamList (&(dest->slParam), \
										&(source->slParam), err) == SipFail)
		return SipFail;
	
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneSipFromHeader");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sipParser_cloneSipToHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds 
** from the ToHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sipParser_cloneSipToHeader
#ifdef ANSI_PROTO
	(SipToHeader *dest, SipToHeader *source, SipError *err)
#else
	(dest, source, err)
	SipToHeader *dest;
	SipToHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function sip_cloneSipToHeader");

	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);
	sip_freeSipAddrSpec(dest->pAddrSpec);
	if (sip_listDeleteAll(&(dest->slTag), err) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;
	/* Duplicating DispName */
	if (source->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPDECODE(source->pDispName);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pDispName = temp;
	/* Duplicating AddrSpec */
	if (__validateSipAddrSpecType((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), \
						(source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (__sipParser_cloneSipAddrSpec(dest->pAddrSpec,\
								 source->pAddrSpec, err) == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}

	/* Duplicating Tag */
	if (__sipParser_cloneSipStringList (&(dest->slTag), \
									&(source->slTag), err) == SipFail)
		return SipFail;

	/* clone Param */
	if (__sipParser_cloneSipParamList (&(dest->slParam),\
								 &(source->slParam), err) == SipFail)
		return SipFail;
	
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneSipToHeader");
	return SipSuccess;
}

