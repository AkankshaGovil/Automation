/******************************************************************************
** FUNCTION:
** 	This file contains the source dCodeNum of all First Level General
**      Header SIP stack APIs.
**
*******************************************************************************
**
** FILENAME:
** 	general.c
**
** DESCRIPTION:
**  	This pHeader file contains the source dCodeNum of all the First Level
**		general headers
**
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 20Dec99   	S.Luthra    	Original
**
** Copyrights 1999, Hughes Software Systems, Ltd.
**
******************************************************************************/
#include "sipfree.h"
#include "sipinit.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipinternal.h"
#include "general.h"
#include "string.h"
#include "siplist.h"
#include "sipvalidate.h"
#include "sipclone.h"


/**********************************************************************
**
** FUNCTION:  sip_getMediaRangeFromAcceptHdr
**
** DESCRIPTION: This function retrieves the medai range field from a SIP
**		Accept pHeader
**
**********************************************************************/

SipBool sip_getMediaRangeFromAcceptHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pMediaRange, SipError *err)
#else
	(hdr, pMediaRange, err)
	SipHeader *hdr;
	SIP_S8bit **pMediaRange;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_media_range;
	SIPDEBUGFN("Entering function sip_getMediaRangeFromAcceptHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pMediaRange == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAccept)
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

 	temp_media_range = ((SipAcceptHeader *)(hdr ->pHeader))->pMediaRange;
 	if (temp_media_range == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pMediaRange = (SIP_S8bit *) STRDUPACCESSOR (temp_media_range);
	if (*pMediaRange == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pMediaRange = temp_media_range;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getMediaRangeFromAcceptHdr");
 	return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_setMediaRangeInAcceptHdr
**
** DESCRIPTION: This function sets the slMedia range field from a SIP
**		Accept pHeader
**
**********************************************************************/

SipBool sip_setMediaRangeInAcceptHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pMediaRange, SipError *err)
#else
	(hdr, pMediaRange, err)
	SipHeader *hdr;
	SIP_S8bit *pMediaRange;
	SipError *err;
#endif
{
	SIP_S8bit *temp_media_range, *mrange;
	SIPDEBUGFN("Entering function sip_setMediaRangeInAcceptHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAccept)
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
        if( pMediaRange == SIP_NULL)
                temp_media_range = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_media_range = (SIP_S8bit *) STRDUPACCESSOR(pMediaRange);
		if (temp_media_range == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_media_range = pMediaRange;
#endif
	}

        mrange = ((SipAcceptHeader *)(hdr->pHeader))->pMediaRange;

	if ( mrange != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&mrange), err) == SipFail)
		{
			sip_freeString(temp_media_range);
			return SipFail;
		}
	}

	((SipAcceptHeader *)(hdr->pHeader))->pMediaRange = temp_media_range;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setMediaRangeInAcceptHdr");
        return SipSuccess;
}


/**********************************************************************
**
** FUNCTION:  sip_getAcceptRangeFromAcceptHdr
**
** DESCRIPTION: This function retrieves the accept-range field from a
**		SIP accept pHeader
**
**********************************************************************/

SipBool sip_getAcceptRangeFromAcceptHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pAcceptRange, SipError *err)
#else
	(hdr, pAcceptRange, err)
	SipHeader *hdr;
	SIP_S8bit ** pAcceptRange;
        SipError *err;
#endif
{

	SIP_S8bit *temp_accept_range;
	SIPDEBUGFN("Entering function sip_getAcceptRangeFromAcceptHdr");
	if (err == SIP_NULL)
 		return SipFail;
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pAcceptRange == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAccept)
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
 	temp_accept_range = ((SipAcceptHeader *) (hdr ->pHeader))->pAcceptRange;
 	if (temp_accept_range == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pAcceptRange = (SIP_S8bit *)STRDUPACCESSOR(temp_accept_range);
	if (*pAcceptRange == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pAcceptRange = temp_accept_range;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAcceptRangeFromAcceptHdr");
 	return SipSuccess;
}



/*****************************************************************
**
** FUNCTION:  sip_setAcceptRangeInAcceptHdr
**
** DESCRIPTION: This function sets the accept-range field in a SIP
**		Accept Header
**
***************************************************************/
SipBool sip_setAcceptRangeInAcceptHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pAcceptRange, SipError *err)
#else
	(hdr, pAcceptRange, err)
	SipHeader *hdr;
	SIP_S8bit *pAcceptRange;
	SipError *err;
#endif
{
	SIP_S8bit *temp_accept_range, *arange;
	SIPDEBUGFN("Entering function sip_setAcceptRangeInAcceptHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAccept)
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

        if( pAcceptRange == SIP_NULL)
                temp_accept_range = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_accept_range = (SIP_S8bit *) STRDUPACCESSOR(pAcceptRange);
		if (temp_accept_range == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_accept_range = pAcceptRange;
#endif
	}

	arange = ((SipAcceptHeader *)(hdr->pHeader))->pAcceptRange;

	if ( arange != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&arange), err) == SipFail)
		{
			sip_freeString(temp_accept_range);
			return SipFail;
		}
	}

	((SipAcceptHeader *)(hdr->pHeader))->pAcceptRange = temp_accept_range;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAcceptRangeInAcceptHdr");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getCodingsFromAcceptEncodingHdr
**
** DESCRIPTION: This function retrives the coding field from a SIP
**		Accept-Encoding pHeader
**
***************************************************************/
SipBool sip_getCodingsFromAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pCoding, SipError *err)
#else
	(hdr, pCoding, err)
	SipHeader *hdr;
	SIP_S8bit **pCoding;
	SipError *err;
#endif
{
	SIP_S8bit *temp_codings;
	SIPDEBUGFN("Entering function sip_getCodingsFromAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pCoding == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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
 	temp_codings = ((SipAcceptEncodingHeader *)(hdr->pHeader))->pCoding;
 	if (temp_codings == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pCoding = (SIP_S8bit *) STRDUPACCESSOR (temp_codings);
	if (*pCoding == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pCoding = temp_codings;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getCodingsFromAcceptEncodingHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setCodingsInAcceptEncodingHdr
**
** DESCRIPTION: This function sets the coding field in a SIP
**		accept pEncoding pHeader
**
***************************************************************/
SipBool sip_setCodingsInAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pCoding, SipError *err)
#else
	(hdr, pCoding, err)
	SipHeader *hdr;
	SIP_S8bit *pCoding;
 	SipError *err;
#endif
{
	SIP_S8bit *temp_codings, *cdngs;
	SIPDEBUGFN("Entering function sip_setCodingsInAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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
        if( pCoding == SIP_NULL)
                temp_codings = SIP_NULL;
        else
	{
#ifndef SIP_BY_REFERENCE
	        temp_codings = (SIP_S8bit *) STRDUPACCESSOR(pCoding);
		if (temp_codings == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_codings = pCoding;
#endif
	}

        cdngs = ((SipAcceptEncodingHeader *)(hdr->pHeader))->pCoding;
        if ( cdngs != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&cdngs), err) == SipFail)
		{
			sip_freeString(temp_codings);
			return SipFail;
		}
	}

	((SipAcceptEncodingHeader *)(hdr->pHeader))->pCoding = temp_codings;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setCodingsInAcceptEncodingHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getQValuesFromAcceptEncodingHdr
**
** DESCRIPTION: This function retrives the qvalue field from a SIP
**		Accept-Encoding pHeader
**
***************************************************************/
SipBool sip_getQValuesFromAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pQValue, SipError *err)
#else
	(hdr, pQValue, err)
	SipHeader *hdr;
	SIP_S8bit **pQValue;
	SipError *err;
#endif
{
	SIP_S8bit *temp_qvalues;
	SIPDEBUGFN("Entering function sip_getQValuesFromAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pQValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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
	temp_qvalues = ((SipAcceptEncodingHeader *)(hdr->pHeader))->pQValue;
 	if (temp_qvalues== SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pQValue = (SIP_S8bit *) STRDUPACCESSOR (temp_qvalues);
	if (*pQValue == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pQValue = temp_qvalues;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getQValuesFromAcceptEncodingHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setQValuesInAcceptEncodingHdr
**
** DESCRIPTION: This function sets the QValue field in a SIP
**		accept pQValue pHeader
**
***************************************************************/
SipBool sip_setQValuesInAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pQValue, SipError *err)
#else
	(hdr, pQValue, err)
	SipHeader *hdr;
	SIP_S8bit *pQValue;
 	SipError *err;
#endif
{
	SIP_S8bit *temp_qvalues, *cdngs;
	SIPDEBUGFN("Entering function sip_setQValuesInAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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

        if( pQValue == SIP_NULL)
                temp_qvalues = SIP_NULL;
        else
	{
#ifndef SIP_BY_REFERENCE
	        temp_qvalues = (SIP_S8bit *) STRDUPACCESSOR(pQValue);
		if (temp_qvalues == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_qvalues = pQValue;
#endif
	}

        cdngs = ((SipAcceptEncodingHeader *)(hdr->pHeader))->pQValue;
        if ( cdngs != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&cdngs), err) == SipFail)
		{
			sip_freeString(temp_qvalues);
			return SipFail;
		}
	}

	((SipAcceptEncodingHeader *)(hdr->pHeader))->pQValue = temp_qvalues;


	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setQValuesInAcceptEncodingHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAcceptParamCountFromAcceptEncodingHdr
**
** DESCRIPTION: This function retrrieves the number of via-pParam
**		from a SIP Via pHeader
**
***************************************************************/
SipBool sip_getAcceptParamCountFromAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN\
	("Entering function sip_getAcceptParamCountFromAcceptEncodingHdr");

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
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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

	if (sip_listSizeOf\
		( &(((SipAcceptEncodingHeader *)(hdr->pHeader))->slParam), count , err)\
		== SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_getAcceptParamCountFromAcceptEncodingHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAcceptParamAtIndexFromAcceptEncodingHdr
**
** DESCRIPTION: This function retrievse a accept-param at a specified
**		index from a SIP via pHeader
**
***************************************************************/
SipBool sip_getAcceptParamAtIndexFromAcceptEncodingHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipNameValuePair *accept_param, SIP_U32bit index, SipError  *err)
#else
	(SipHeader *hdr, SipNameValuePair **accept_param, SIP_U32bit index, SipError  *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair *accept_param;
	SIP_U32bit index;
	SipError  *err;
#else
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair **accept_param;
	SIP_U32bit index;
	SipError  *err;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipNameValuePair *temp_accept_param;
	SIPDEBUGFN\
		("Entering function sip_getAcceptParamAtIndexFromAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  accept_param == SIP_NULL)
#else
	if(  accept_param == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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
	if (sip_listGetAt\
		( &(((SipAcceptEncodingHeader *)(hdr->pHeader))->slParam), index,  \
		&element_from_list, err) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	temp_accept_param = (SipNameValuePair *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipNameValuePair(accept_param, temp_accept_param, err)\
			== SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_accept_param->dRefCount);
	*accept_param = temp_accept_param;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_getAcceptParamAtIndexFromAcceptEncodingHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertAcceptParamAtIndexInAcceptEncodingHdr
**
** DESCRIPTION: This function inserts a via-param at a specified
**		index in a SIP Via pHeader
**
***************************************************************/
SipBool sip_insertAcceptParamAtIndexInAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipNameValuePair *accept_param, SIP_U32bit index,\
	SipError *err)
#else
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair *accept_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipNameValuePair *temp_accept_param;
	SIPDEBUGFN\
		("Entering function sip_insertAcceptParamAtIndexInAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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
	if ( accept_param == SIP_NULL )
		temp_accept_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_accept_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair\
				(temp_accept_param, accept_param, err) == SipFail)
		{
			sip_freeSipNameValuePair (temp_accept_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(accept_param->dRefCount);
		temp_accept_param = accept_param;
#endif
	}

	if( sip_listInsertAt\
		( &(((SipAcceptEncodingHeader *)(hdr->pHeader))->slParam),  \
		index, (SIP_Pvoid)(temp_accept_param), err) == SipFail)
	{
		if (temp_accept_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (temp_accept_param);
#else
			HSS_LOCKEDDECREF(accept_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_insertAcceptParamAtIndexInAcceptEncodingHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAcceptParamAtIndexInAcceptEncodingHdr
**
** DESCRIPTION: This function sets a via-param at a specified index
**		in a SIP Via pHeader
**
***************************************************************/
SipBool sip_setAcceptParamAtIndexInAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipNameValuePair *accept_param, SIP_U32bit index, SipError *err)
#else
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair *accept_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipNameValuePair *temp_accept_param;
	SIPDEBUGFN\
		("Entering function sip_setAcceptParamAtIndexInAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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
	if ( accept_param == SIP_NULL )
		temp_accept_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_accept_param, err) == SipFail)

			return SipFail;
		if (__sip_cloneSipNameValuePair(temp_accept_param, accept_param, err)\
				== SipFail)
		{
			sip_freeSipNameValuePair(temp_accept_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(accept_param->dRefCount);
		temp_accept_param = accept_param;
#endif
	}

	if( sip_listSetAt\
		( &(((SipAcceptEncodingHeader *)(hdr->pHeader))->slParam), index,  \
		(SIP_Pvoid)(temp_accept_param), err) == SipFail)
	{
		if (temp_accept_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair(temp_accept_param);
#else
			HSS_LOCKEDDECREF(temp_accept_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_setAcceptParamAtIndexInAcceptEncodingHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteAcceptParamAtIndexInAcceptEncodingHdr
**
** DESCRIPTION: This function deletes a via-param at a specified
**		index in a SIP Via pHeader
**
***************************************************************/
SipBool sip_deleteAcceptParamAtIndexInAcceptEncodingHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN\
		("Entering function sip_deleteAcceptParamAtIndexInAcceptEncodingHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptEncoding)
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
	if( sip_listDeleteAt\
			( &(((SipAcceptEncodingHeader *)(hdr->pHeader))->slParam),\
			index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_deleteAcceptParamAtIndexInAcceptEncodingHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getLangRangeFromAcceptLangHdr
**
** DESCRIPTION: This function retrieves the langauge-range field
**		from a SIP Accept Language pHeader
**
***************************************************************/
SipBool sip_getLangRangeFromAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pLangRange, SipError *err)
#else
	(hdr, pLangRange, err)
	SipHeader *hdr;
	SIP_S8bit **pLangRange;
	SipError *err;
#endif
{
	SIP_S8bit *temp_lang_range;
	SIPDEBUGFN("Entering function sip_getLangRangeFromAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pLangRange == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
 	temp_lang_range = ((SipAcceptLangHeader *)(hdr->pHeader))->pLangRange;
 	if (temp_lang_range == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pLangRange = (SIP_S8bit *) STRDUPACCESSOR(temp_lang_range);
	if (*pLangRange == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pLangRange = temp_lang_range;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getLangRangeFromAcceptLangHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setLangRangeInAcceptLangHdr
**
** DESCRIPTION: This function sets the Language-range field in a
**		SIP Accept language pHeader
**
***************************************************************/
SipBool sip_setLangRangeInAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pLangRange, SipError *err)
#else
	(hdr, pLangRange, err)
	SipHeader *hdr;
	SIP_S8bit *pLangRange;
	SipError *err;
#endif
{
	SIP_S8bit *temp_lang_range, *lrange;
	SIPDEBUGFN("Entering function sip_setLangRangeInAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
        if( pLangRange == SIP_NULL)
                temp_lang_range = SIP_NULL;
        else
	{
#ifndef SIP_BY_REFERENCE
		temp_lang_range = (SIP_S8bit *) STRDUPACCESSOR(pLangRange);
		if (temp_lang_range == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_lang_range = pLangRange;
#endif
	}

        lrange = ((SipAcceptLangHeader *)(hdr->pHeader))->pLangRange;

        if ( lrange != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&lrange), err) == SipFail)
		{
			sip_freeString(temp_lang_range);
			return SipFail;
		}
	}
	((SipAcceptLangHeader *)(hdr->pHeader))->pLangRange = temp_lang_range;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setLangRangeInAcceptLangHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getQvalueFromAcceptLangHdr
**
** DESCRIPTION: This function retrieves the pQValue-pValue field
** from aSIP Accept Language pHeader
**
***************************************************************/
SipBool sip_getQvalueFromAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pQValue, SipError *err)
#else
	(hdr, pQValue, err)
	SipHeader *hdr;
	SIP_S8bit **pQValue;
	SipError *err;
#endif
{
	SIP_S8bit *temp_qvalue;
	SIPDEBUGFN("Entering function sip_getQvalueFromAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pQValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
 	temp_qvalue = ((SipAcceptLangHeader *) (hdr ->pHeader))->pQValue;
 	if (temp_qvalue == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pQValue = (SIP_S8bit *) STRDUPACCESSOR (temp_qvalue);
	if (*pQValue == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pQValue = temp_qvalue;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getQvalueFromAcceptLangHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setQvalueInAcceptLangHdr
**
** DESCRIPTION: This function sets the pQValue-pValue field in a SIP
**		Accept Language pHeader
**
***************************************************************/
SipBool sip_setQvalueInAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pQValue, SipError *err)
#else
	(hdr, pQValue, err)
	SipHeader *hdr;
	SIP_S8bit *pQValue;
	SipError *err;
#endif
{
	SIP_S8bit *temp_qvalue, *qval;
	SIPDEBUGFN("Entering function sip_setQvalueInAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
        if( pQValue == SIP_NULL)
                temp_qvalue = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_qvalue = (SIP_S8bit *)STRDUPACCESSOR(pQValue);
		if (temp_qvalue == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_qvalue = pQValue;
#endif
	}

        qval = ((SipAcceptLangHeader *)(hdr->pHeader))->pQValue;
        if ( qval != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&qval), err) == SipFail)
		{
			sip_freeString(temp_qvalue);
			return SipFail;
		}
	}

	((SipAcceptLangHeader *)(hdr->pHeader))->pQValue = temp_qvalue;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setQvalueInAcceptLangHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAcceptParamCountFromAcceptLangHdr
**
** DESCRIPTION: This function retrrieves the number of via-pParam
**		from a SIP Via pHeader
**
***************************************************************/
SipBool sip_getAcceptParamCountFromAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN\
	("Entering function sip_getAcceptParamCountFromAcceptLangHdr");

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
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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

	if (sip_listSizeOf\
		( &(((SipAcceptLangHeader *)(hdr->pHeader))->slParam), count , err)\
		== SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_getAcceptParamCountFromAcceptLangHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAcceptParamAtIndexFromAcceptLangHdr
**
** DESCRIPTION: This function retrievse a accept-param at a specified
**		index from a SIP via pHeader
**
***************************************************************/
SipBool sip_getAcceptParamAtIndexFromAcceptLangHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipNameValuePair *accept_param, SIP_U32bit index, SipError  *err)
#else
	(SipHeader *hdr, SipNameValuePair **accept_param, SIP_U32bit index, SipError  *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair *accept_param;
	SIP_U32bit index;
	SipError  *err;
#else
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair **accept_param;
	SIP_U32bit index;
	SipError  *err;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipNameValuePair *temp_accept_param;
	SIPDEBUGFN\
		("Entering function sip_getAcceptParamAtIndexFromAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  accept_param == SIP_NULL)
#else
	if(  accept_param == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
	if (sip_listGetAt\
		( &(((SipAcceptLangHeader *)(hdr->pHeader))->slParam), index,  \
		&element_from_list, err) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	temp_accept_param = (SipNameValuePair *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipNameValuePair(accept_param, temp_accept_param, err)\
			== SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_accept_param->dRefCount);
	*accept_param = temp_accept_param;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_getAcceptParamAtIndexFromAcceptLangHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertAcceptParamAtIndexInAcceptLangHdr
**
** DESCRIPTION: This function inserts a via-param at a specified
**		index in a SIP Via pHeader
**
***************************************************************/
SipBool sip_insertAcceptParamAtIndexInAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipNameValuePair *accept_param, SIP_U32bit index,\
	SipError *err)
#else
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair *accept_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipNameValuePair *temp_accept_param;
	SIPDEBUGFN\
		("Entering function sip_insertAcceptParamAtIndexInAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
	if ( accept_param == SIP_NULL )
		temp_accept_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_accept_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair\
				(temp_accept_param, accept_param, err) == SipFail)
		{
			sip_freeSipNameValuePair (temp_accept_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(accept_param->dRefCount);
		temp_accept_param = accept_param;
#endif
	}

	if( sip_listInsertAt\
		( &(((SipAcceptLangHeader *)(hdr->pHeader))->slParam),  \
		index, (SIP_Pvoid)(temp_accept_param), err) == SipFail)
	{
		if (temp_accept_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (temp_accept_param);
#else
			HSS_LOCKEDDECREF(accept_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_insertAcceptParamAtIndexInAcceptLangHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAcceptParamAtIndexInAcceptLangHdr
**
** DESCRIPTION: This function sets a via-param at a specified index
**		in a SIP Via pHeader
**
***************************************************************/
SipBool sip_setAcceptParamAtIndexInAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipNameValuePair *accept_param, SIP_U32bit index, SipError *err)
#else
	(hdr, accept_param, index, err)
	SipHeader *hdr;
	SipNameValuePair *accept_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipNameValuePair *temp_accept_param;
	SIPDEBUGFN\
		("Entering function sip_setAcceptParamAtIndexInAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
	if ( accept_param == SIP_NULL )
		temp_accept_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&temp_accept_param, err) == SipFail)

			return SipFail;
		if (__sip_cloneSipNameValuePair(temp_accept_param, accept_param, err)\
				== SipFail)
		{
			sip_freeSipNameValuePair(temp_accept_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(accept_param->dRefCount);
		temp_accept_param = accept_param;
#endif
	}

	if( sip_listSetAt\
		( &(((SipAcceptLangHeader *)(hdr->pHeader))->slParam), index,  \
		(SIP_Pvoid)(temp_accept_param), err) == SipFail)
	{
		if (temp_accept_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair(temp_accept_param);
#else
			HSS_LOCKEDDECREF(temp_accept_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_setAcceptParamAtIndexInAcceptLangHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteAcceptParamAtIndexInAcceptLangHdr
**
** DESCRIPTION: This function deletes a via-param at a specified
**		index in a SIP Via pHeader
**
***************************************************************/
SipBool sip_deleteAcceptParamAtIndexInAcceptLangHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN\
		("Entering function sip_deleteAcceptParamAtIndexInAcceptLangHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAcceptLanguage)
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
	if( sip_listDeleteAt\
			( &(((SipAcceptLangHeader *)(hdr->pHeader))->slParam),\
			index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN\
		("Exiting function sip_deleteAcceptParamAtIndexInAcceptLangHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getValueFromCallIdHdr
**
** DESCRIPTION: This function retrieves the pValue field from a SIP
**		Call-Ifd Header
**
***************************************************************/
SipBool sip_getValueFromCallIdHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pValue, SipError *err)
#else
	(hdr, pValue, err)
	SipHeader *hdr;
	SIP_S8bit **pValue;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering function sip_getValueFromCallIdHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCallId)
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
 	temp_value = ((SipCallIdHeader *) (hdr ->pHeader))->pValue;
 	if (temp_value == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pValue = (SIP_S8bit *) STRDUPACCESSOR(temp_value);
	if (*pValue == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pValue = temp_value;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getValueFromCallIdHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setValueInCallIdHdr
**
** DESCRIPTION: This function sets the pValue field in a SIP call-Id
**		pHeader
**
***************************************************************/
SipBool sip_setValueInCallIdHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pValue, SipError *err)
#else
	(hdr, pValue, err)
	SipHeader *hdr;
	SIP_S8bit *pValue;
	SipError *err;
#endif
{
	SIP_S8bit *temp_value, *val;
	SIPDEBUGFN("Entering function sip_setValueInCallIdHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCallId)
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
        if( pValue == SIP_NULL)
                temp_value = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_value = (SIP_S8bit *) STRDUPACCESSOR(pValue);
		if (temp_value == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_value = pValue;
#endif
	}

        val = ((SipCallIdHeader *)(hdr->pHeader))->pValue;
        if ( val != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&val), err) == SipFail)
		{
			sip_freeString(temp_value);
			return SipFail;
		}
	}

	((SipCallIdHeader *)(hdr->pHeader))->pValue = temp_value;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setValueInCallIdHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getDispNameFromContactHdr
**
** DESCRIPTION: This function retrieves the display pName from a SIP
**		Contact pHeader
**
***************************************************************/
SipBool sip_getDispNameFromContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit **pDispName;
	SipError *err;
#endif
{
	SIP_S8bit *temp_disp_name;
	SIPDEBUGFN("Entering function sip_getDispNameFromContactHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pDispName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
 	temp_disp_name = ((SipContactHeader *) (hdr ->pHeader))->pDispName;
 	if (temp_disp_name == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDispName = (SIP_S8bit *) STRDUPACCESSOR (temp_disp_name);
	if (*pDispName == SIP_NULL)
	{
		*err = E_NO_ERROR;
		return SipFail;
	}
#else
	*pDispName = temp_disp_name;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromContactHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInContactHdr
**
** DESCRIPTION: This function sets the display pName field in a SIP
**		Contact pHeader
**
***************************************************************/
SipBool sip_setDispNameInContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit *pDispName;
	SipError *err;
#endif
{
        SIP_S8bit *temp_disp_name, *dname;
	SIPDEBUGFN("Entering function sip_setDispNameInContactHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
        if( pDispName == SIP_NULL)
                temp_disp_name = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_disp_name = (SIP_S8bit *) STRDUPACCESSOR(pDispName);
		if (temp_disp_name == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_disp_name = pDispName;
#endif
	}

	dname = ((SipContactHeader *)(hdr->pHeader))->pDispName;
        if ( dname != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&dname), err) == SipFail)
		{
			sip_freeString(temp_disp_name);
			return SipFail;
		}
	}
	((SipContactHeader *)(hdr->pHeader))->pDispName = temp_disp_name;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInContactHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromContactHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP contact pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromContactHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(SipHeader *hdr, SipAddrSpec **ppAddrSpec, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#else
	(hdr, ppAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec **ppAddrSpec;
	SipError *err;
#endif
#endif
{
 	SipAddrSpec *pTempAddrSpec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromContactHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pAddrSpec == SIP_NULL)
#else
	if(  ppAddrSpec == SIP_NULL)
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
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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

	if (hdr->dType == SipHdrTypeContactWildCard)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

 	pTempAddrSpec = ((SipContactHeader *) (hdr ->pHeader))->pAddrSpec;
 	if (pTempAddrSpec == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipAddrSpec(pAddrSpec, pTempAddrSpec, err) == SipFail)
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
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromContactHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInContactHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		conatct pHeader
**
***************************************************************/
SipBool sip_setAddrSpecInContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
 	SipAddrSpec *temp_addr_spec;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInContactHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
 	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipContactHeader *)(hdr ->pHeader))->pAddrSpec);
 		((SipContactHeader *) (hdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSipAddrSpec(&temp_addr_spec, pAddrSpec->dType , err ) == SipFail)
			return SipFail;
 		if (__sip_cloneSipAddrSpec(temp_addr_spec, pAddrSpec, err) == SipFail)
		{
			sip_freeSipAddrSpec(temp_addr_spec);
			return SipFail;
       		}
		sip_freeSipAddrSpec(((SipContactHeader *)(hdr ->pHeader))->pAddrSpec);
;
		((SipContactHeader *)(hdr ->pHeader))->pAddrSpec = temp_addr_spec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipContactHeader *)(hdr ->pHeader))->pAddrSpec);
		((SipContactHeader *)(hdr->pHeader))->pAddrSpec = pAddrSpec;
#endif
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInContactHdr");
	return SipSuccess;
}



/************************************************************************
**
** FUNCTION:  sip_getContactParamsCountFromContactHdr
**
** DESCRIPTION: This function retrieves the number of contact pParam
**		from a SIP contact pHeader
**
*************************************************************************/
SipBool sip_getContactParamsCountFromContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getContactParamsCountFromContactHdr");
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
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
	if (sip_listSizeOf( &(((SipContactHeader *)(hdr->pHeader))->slContactParam) \
		, count , err ) == SipFail )
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getContactParamsCountFromContactHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getContactParamAtIndexFromContactHdr
**
** DESCRIPTION: This function gets a conatct param at a specified
**		index from a SIP Contact pHeader
**
***************************************************************/
SipBool sip_getContactParamAtIndexFromContactHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipContactParam *pContactParam, SIP_U32bit index, SipError *err)
#else
	(SipHeader *hdr, SipContactParam **ppContactParam, SIP_U32bit index, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pContactParam, index, err)
	SipHeader *hdr;
	SipContactParam *pContactParam;
	SIP_U32bit index;
	SipError *err;
#else
	(hdr, ppContactParam, index, err)
	SipHeader *hdr;
	SipContactParam **ppContactParam;
	SIP_U32bit index;
	SipError *err;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipContactParam *temp_contact_param;

	SIPDEBUGFN("Entering function sip_getContactParamAtIndexFromContactHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pContactParam == SIP_NULL)
#else
	if(  ppContactParam == SIP_NULL)
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
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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

        if ( sip_listGetAt( &(((SipContactHeader *)(hdr->pHeader))->slContactParam),  \
			index, &element_from_list, err) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	temp_contact_param = (SipContactParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipContactParam(pContactParam, temp_contact_param, err) == SipFail)
	{
		switch (pContactParam->dType)
		{
			case SipCParamQvalue:
								sip_freeString((pContactParam->u).pQValue);
							 	(pContactParam->u).pQValue = SIP_NULL;
								break;
			case SipCParamExpires:
								sip_freeSipExpiresStruct\
									((pContactParam->u).pExpire);
								(pContactParam->u).pExpire = SIP_NULL;
								break;
			case SipCParamExtension:
								sip_freeString\
									((pContactParam->u).pExtensionAttr);
								(pContactParam->u).pExtensionAttr = SIP_NULL;
								break;
			case SipCParamFeatureParam:
				       sip_freeSipParam(\
							 (pContactParam->u).pParam) ;
							 break ;
			case SipCParamAny:
								break;
			default				:
								*err = E_INV_TYPE;
							 	return SipFail;
		}
		pContactParam->dType = SipCParamAny;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(temp_contact_param->dRefCount);
	*ppContactParam = temp_contact_param;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getContactParamAtIndexFromContactHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertContactParamAtIndexInContactHdr
**
** DESCRIPTION: This function inserts a contact param at a spec.
**		index in a SIP Contact pHeader
**
***************************************************************/
SipBool sip_insertContactParamAtIndexInContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipContactParam *contact_param, SIP_U32bit index, SipError *err)
#else
	(hdr, contact_param, index, err)
	SipHeader *hdr;
	SipContactParam *contact_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipContactParam *temp_contact_param;
	SIPDEBUGFN("Entering function sip_insertContactParamAtIndexInContactHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
	if (contact_param == SIP_NULL)
		temp_contact_param = SIP_NULL;
	else
	{

		if (validateSipContactParamsType(contact_param->dType, err) == SipFail)
			return SipFail;
#ifndef SIP_BY_REFERENCE
		if (sip_initSipContactParam(&temp_contact_param, contact_param->dType, err) == SipFail)
			return SipFail;

		if (__sip_cloneSipContactParam(temp_contact_param, contact_param, err) == SipFail)
		{
			sip_freeSipContactParam(temp_contact_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(contact_param->dRefCount);
		temp_contact_param = contact_param;
#endif
	}

	if ( sip_listInsertAt( &(((SipContactHeader *)(hdr->pHeader))->slContactParam),  \
		index, (SIP_Pvoid)temp_contact_param, err) == SipFail)
	{
		if (temp_contact_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipContactParam(temp_contact_param);
#else
			HSS_LOCKEDDECREF(contact_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertContactParamAtIndexInContactHdr");
	return SipSuccess;

}

/*************************************************************************
**
** FUNCTION:  sip_setContactParamAtIndexInContactHdr
**
** DESCRIPTION: This function sets a contact param at a specified index
**		in a SIP contact pHeader
**
*************************************************************************/
SipBool sip_setContactParamAtIndexInContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipContactParam *contact_param, SIP_U32bit index, SipError *err)
#else
	(hdr, contact_param, index, err)
	SipHeader *hdr;
	SipContactParam *contact_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipContactParam *temp_contact_param;
	SIPDEBUGFN("Entering function ");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
	if (contact_param == SIP_NULL)
		temp_contact_param = SIP_NULL;
	else
	{
		if (validateSipContactParamsType(contact_param->dType, err) == SipFail)
			return SipFail;
#ifndef SIP_BY_REFERENCE
		if (sip_initSipContactParam(&temp_contact_param, contact_param->dType, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipContactParam(temp_contact_param, contact_param, err) == SipFail)
		{
			sip_freeSipContactParam(temp_contact_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(contact_param->dRefCount);
		temp_contact_param = contact_param;
#endif
	}

	if ( sip_listSetAt( &(((SipContactHeader *)(hdr->pHeader))->slContactParam), index,  \
		(SIP_Pvoid)temp_contact_param, err) == SipFail)
	{
		if (temp_contact_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipContactParam(temp_contact_param);
#else
			HSS_LOCKEDDECREF(contact_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function ");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteContactParamAtIndexInContactHdr
**
** DESCRIPTION: This function deletes a contact-param at a specified
**		index in the Contact pHeader
**
***************************************************************/
SipBool sip_deleteContactParamAtIndexInContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteContactParamAtIndexInContactHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
	if( sip_listDeleteAt( &(((SipContactHeader *)(hdr->pHeader))->slContactParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteContactParamAtIndexInContactHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getSeqNumFromCseqHdr
**
** DESCRIPTION: This function retrieves the Sequence number from a
**		SIP Cseq pHeader
**
***************************************************************/
SipBool sip_getSeqNumFromCseqHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *dSeqNum, SipError *err)
#else
	(hdr, dSeqNum, err)
	SipHeader *hdr;
	SIP_U32bit *dSeqNum;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getSeqNumFromCseqHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dSeqNum == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCseq)
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
	*dSeqNum = ( (SipCseqHeader *)(hdr->pHeader) )->dSeqNum;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSeqNumFromCseqHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSeqNumInCseqHdr
**
** DESCRIPTION: This function sets the sequence number field in a
**		SIP Cseq pHeader
**
***************************************************************/
SipBool sip_setSeqNumInCseqHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit dSeqNum, SipError *err)
#else
	(hdr, dSeqNum, err)
	SipHeader *hdr;
	SIP_U32bit dSeqNum;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_setSeqNumInCseqHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCseq)
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
	((SipCseqHeader *)(hdr->pHeader))->dSeqNum = dSeqNum;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSeqNumInCseqHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getMethodFromCseqHdr
**
** DESCRIPTION: This function retrieves the pMethod field from a SIP
**		Cseq pHeader
**
***************************************************************/
SipBool sip_getMethodFromCseqHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pMethod, SipError *err)
#else
	(hdr, pMethod, err)
	SipHeader *hdr;
	SIP_S8bit **pMethod;
	SipError *err;
#endif
{
	SIP_S8bit *temp_method;
	SIPDEBUGFN("Entering function sip_getMethodFromCseqHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pMethod == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCseq)
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

 	temp_method = ((SipCseqHeader *)(hdr->pHeader))->pMethod;
 	if (temp_method == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pMethod = (SIP_S8bit *) STRDUPACCESSOR (temp_method);
	if (*pMethod == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pMethod = temp_method;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getMethodFromCseqHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setMethodInCseqHdr
**
** DESCRIPTION: This function sets the pMethod field in a SIP Cseq
**		pHeader
**
***************************************************************/
SipBool sip_setMethodInCseqHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pMethod, SipError *err)
#else
	(hdr, pMethod, err)
	SipHeader *hdr;
	SIP_S8bit *pMethod;
	SipError *err;
#endif
{

	SIP_S8bit *temp_method, *mthd;
        SIPDEBUGFN("Entering function sip_setMethodInCseqHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCseq)
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
        if( pMethod == SIP_NULL)
                temp_method = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_method = (SIP_S8bit *) STRDUPACCESSOR(pMethod);
		if (temp_method == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_method = pMethod;
#endif
	}

        mthd = ((SipCseqHeader *)(hdr->pHeader))->pMethod;

	if ( mthd != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&mthd), err) == SipFail)
		{
			sip_freeString(temp_method);
			return SipFail;
		}
	}

	((SipCseqHeader *)(hdr->pHeader))->pMethod = temp_method;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setMethodInCseqHdr");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getDayOfWeekFromDateHdr
**
** DESCRIPTION: This function retrieves the dDay of the week field
**		from a SIP Date pHeader
**
***************************************************************/
SipBool sip_getDayOfWeekFromDateHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, en_DayOfWeek *dDay, SipError *err)
#else
	(hdr, dDay, err)
	SipHeader *hdr;
	en_DayOfWeek *dDay;
	SipError *err;
#endif
{
	en_DayOfWeek temp_day;
	SIPDEBUGFN("Entering function sip_getDayOfWeekFromDateHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dDay == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeDate)
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
	temp_day = ( (SipDateHeader *) (hdr->pHeader) )->dDow;
	if (temp_day == SipDayNone)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
	if (validateSipDayOfWeek(temp_day,err) == SipFail)
		return SipFail;

		*dDay = temp_day;
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDayOfWeekFromDateHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDayOfWeekInDateHdr
**
** DESCRIPTION: This function sets the dDay of week field in a SIP
**		Date pHeader
**
***************************************************************/
SipBool sip_setDayOfWeekInDateHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, en_DayOfWeek dDay, SipError *err)
#else
	(hdr, dDay, err)
	SipHeader *hdr;
	en_DayOfWeek dDay;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_setDayOfWeekInDateHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeDate)
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
	if (validateSipDayOfWeek(dDay, err) == SipFail)
		return SipFail;
	else
		((SipDateHeader *)(hdr->pHeader))->dDow = dDay;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDayOfWeekInDateHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getDateFormatFromDateHdr
**
** DESCRIPTION: This function retrieves the dDate from a SIP dDate
**		pHeader
**
***************************************************************/
SipBool sip_getDateFormatFromDateHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipDateFormat *dDate, SipError *err)
#else
	(SipHeader *hdr, SipDateFormat **pdDate, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, dDate, err)
	SipHeader *hdr;
	SipDateFormat *dDate;
	SipError *err;
#else
	(hdr, pdDate, err)
	SipHeader *hdr;
	SipDateFormat **pdDate;
	SipError *err;
#endif
#endif
{
	SipDateFormat *temp_date_format;
	SIPDEBUGFN("Entering function sip_getDateFormatFromDateHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  dDate == SIP_NULL)
#else
	if(  pdDate == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeDate)
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

 	temp_date_format = ((SipDateHeader *)(hdr->pHeader))->pDate;
	if (temp_date_format == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	if (__sip_cloneSipDateFormat(dDate, temp_date_format, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_date_format->dRefCount);
	*pdDate = temp_date_format;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDateFormatFromDateHdr");
 	return SipSuccess;


}

/*****************************************************************
**
** FUNCTION:  sip_setDateFormatInDateHdr
**
** DESCRIPTION: This function sets the dDate in the SIP Date pHeader
**
***************************************************************/
SipBool sip_setDateFormatInDateHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipDateFormat *dDate, SipError *err)
#else
	(hdr, dDate, err)
	SipHeader *hdr;
	SipDateFormat *dDate;
	SipError *err;
#endif
{
	SipDateFormat *date_format;
	SIPDEBUGFN("Entering function sip_setDateFormatInDateHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeDate)
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
	if(dDate == SIP_NULL)
	{
		sip_freeSipDateFormat(((SipDateHeader *)(hdr->pHeader))->pDate);
		((SipDateHeader *)(hdr->pHeader))->pDate = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipDateFormat(&date_format, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipDateFormat (date_format, dDate, err) == SipFail)
		{
			sip_freeSipDateFormat(date_format);
			return SipFail;
		}
		sip_freeSipDateFormat(((SipDateHeader *)(hdr ->pHeader))->pDate);
		((SipDateHeader *)(hdr ->pHeader))->pDate = date_format;
#else
 		date_format = ((SipDateHeader *)(hdr->pHeader))->pDate;
		if (date_format != SIP_NULL)
			sip_freeSipDateFormat(date_format);
		HSS_LOCKEDINCREF(dDate->dRefCount);
		((SipDateHeader *)(hdr->pHeader))->pDate = dDate;
#endif
	}
 	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDateFormatInDateHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getTimeFormatFromDateHdr
**
** DESCRIPTION: This function retreives the slTime from a SIP Date
**		pHeader
**
***************************************************************/
SipBool sip_getTimeFormatFromDateHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipTimeFormat *slTime, SipError *err)
#else
	(SipHeader *hdr, SipTimeFormat **pslTime, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, slTime, err)
	SipHeader *hdr;
	SipTimeFormat *slTime;
	SipError *err;
#else
	(hdr, pslTime, err)
	SipHeader *hdr;
	SipTimeFormat **pslTime;
	SipError *err;
#endif
#endif
{
	SipTimeFormat *temp_time_format;
	SIPDEBUGFN("Entering function sip_getTimeFormatFromDateHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  slTime == SIP_NULL)
#else
	if(  pslTime == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeDate)
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

 	temp_time_format = ((SipDateHeader *)(hdr->pHeader))->pTime;
	if (temp_time_format == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipTimeFormat(slTime, temp_time_format, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_time_format->dRefCount);
	*pslTime = temp_time_format;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTimeFormatFromDateHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setTimeFormatInDateHdr
**
** DESCRIPTION: This function sets the slTime in a SIP Date pHeader
**
***************************************************************/
SipBool sip_setTimeFormatInDateHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipTimeFormat *slTime, SipError *err)
#else
	(hdr, slTime, err)
	SipHeader *hdr;
	SipTimeFormat *slTime;
	SipError *err;
#endif
{
	SipTimeFormat *time_format;
	SIPDEBUGFN("Entering function sip_setTimeFormatInDateHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeDate)
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
 	time_format = ((SipDateHeader *)(hdr->pHeader))->pTime;
	if(slTime == SIP_NULL)
	{
		if(time_format != SIP_NULL)
			sip_freeSipTimeFormat(time_format);
		((SipDateHeader *)(hdr->pHeader))->pTime = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
		if (time_format != SIP_NULL)
			sip_freeSipTimeFormat(time_format);
		HSS_LOCKEDINCREF(slTime->dRefCount);
		((SipDateHeader *)(hdr->pHeader))->pTime = slTime;
#else
		if (__sip_cloneSipTimeFormat(time_format, slTime, err) == SipFail)
			return SipFail;
#endif
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTimeFormatInDateHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getSchemeFromEncryptionHdr
**
** DESCRIPTION: This function retreives the pScheme field from a
**		SIP Encryption pHeader
**
***************************************************************/
SipBool sip_getSchemeFromEncryptionHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pScheme, SipError *err)
#else
	(hdr, pScheme, err)
	SipHeader *hdr;
	SIP_S8bit **pScheme;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_scheme;
	SIPDEBUGFN("Entering function sip_getSchemeFromEncryptionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pScheme == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeEncryption)
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

 	temp_scheme = ((SipEncryptionHeader *) (hdr ->pHeader))->pScheme;
 	if (temp_scheme == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pScheme = (SIP_S8bit *) STRDUPACCESSOR (temp_scheme);
	if (*pScheme == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pScheme = temp_scheme;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSchemeFromEncryptionHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSchemeInEncryptionHdr
**
** DESCRIPTION: This function sets teh pScheme field in a SIP
**		Encryption pHeader
**
***************************************************************/
SipBool sip_setSchemeInEncryptionHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pScheme, SipError *err)
#else
	(hdr, pScheme, err)
	SipHeader *hdr;
	SIP_S8bit *pScheme;
	SipError *err;
#endif
{
        SIP_S8bit *temp_scheme, *schm;
	SIPDEBUGFN("Entering function sip_setSchemeInEncryptionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeEncryption)
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
        if( pScheme == SIP_NULL)
                temp_scheme = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_scheme = (SIP_S8bit *)STRDUPACCESSOR (pScheme);
		if (temp_scheme == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_scheme = pScheme;
#endif
	}

    schm = ((SipEncryptionHeader *)(hdr->pHeader))->pScheme;

	if ( schm != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&schm), err) == SipFail)
			return SipFail;
	}


	((SipEncryptionHeader *)(hdr->pHeader))->pScheme = temp_scheme;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSchemeInEncryptionHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromEncryptionHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Encryption pHeader
**
***************************************************************/
SipBool sip_getParamCountFromEncryptionHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromEncryptionHdr");
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
	if( hdr->dType != SipHdrTypeEncryption)
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
	if (sip_listSizeOf( &(((SipEncryptionHeader *)(hdr->pHeader))->slParam), count , err) == SipFail )
	{
		return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromEncryptionHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromEncryptionHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP Encryption pHeader
**
***************************************************************/
SipBool sip_getParamAtIndexFromEncryptionHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromEncryptionHdr");
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
	if( hdr->dType != SipHdrTypeEncryption)
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

	if (sip_listGetAt( &(((SipEncryptionHeader *)(hdr->pHeader))->slParam), index,  \
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromEncryptionHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInEncryptionHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp Encryption pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInEncryptionHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInEncryptionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeEncryption)
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

	if( sip_listInsertAt( &(((SipEncryptionHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInEncryptionHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInEncryptionHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP Encryption pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInEncryptionHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInEncryptionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeEncryption)
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
	if( sip_listDeleteAt( &(((SipEncryptionHeader *)(hdr->pHeader))->slParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInEncryptionHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInEncryptionHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP encryption pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInEncryptionHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInEncryptionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeEncryption)
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

	if( sip_listSetAt( &(((SipEncryptionHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_seParamAtIndexInEncryptionHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getDateStructFromExpiresHdr
**
** DESCRIPTION: This function retrieves the dDate from a SIP Expires
**		pHeader
**
***************************************************************/
SipBool sip_getDateStructFromExpiresHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipDateStruct *date_struct, SipError *err)
#else
	(SipHeader *hdr, SipDateStruct **date_struct, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, date_struct, err)
	SipHeader *hdr;
	SipDateStruct *date_struct;
	SipError *err;
#else
	(hdr, date_struct, err)
	SipHeader *hdr;
	SipDateStruct **date_struct;
	SipError *err;
#endif
#endif
{
	SipDateStruct *temp_date_struct;
	SIPDEBUGFN("Entering function sip_getDateStructFromExpiresHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  date_struct == SIP_NULL)
#else
	if(  date_struct == SIP_NULL)
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
	if(( hdr->dType != SipHdrTypeExpiresDate)&&(hdr->dType != SipHdrTypeExpiresSec))
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
	temp_date_struct = (((SipExpiresHeader *)(hdr->pHeader))->u).pDate;
	if (temp_date_struct == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipDateStruct(date_struct, temp_date_struct, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_date_struct->dRefCount);
	*date_struct = temp_date_struct;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDateStructFromExpiresHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDateStructInExpiresHdr
**
** DESCRIPTION: This function sets the dDate in a SIP Expires pHeader
**
***************************************************************/
SipBool sip_setDateStructInExpiresHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipDateStruct *date_struct, SipError *err)
#else
	(hdr, date_struct, err)
	SipHeader *hdr;
	SipDateStruct *date_struct;
	SipError *err;
#endif
{
	SipDateStruct *temp_date_struct;
	SIPDEBUGFN("Entering function sip_setDateStructInExpiresHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeExpiresDate)&&(hdr->dType != SipHdrTypeExpiresSec))
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
	temp_date_struct = (((SipExpiresHeader *)(hdr->pHeader))->u).pDate;
	if (date_struct==SIP_NULL)
	{
		if (temp_date_struct != SIP_NULL)
			sip_freeSipDateStruct(temp_date_struct);
		(((SipExpiresHeader *)(hdr->pHeader))->u).pDate=SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipDateStruct(&temp_date_struct, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipDateStruct(temp_date_struct, date_struct, err) == SipFail)
		{
			sip_freeSipDateStruct(temp_date_struct);
			return SipFail;
		}
		sip_freeSipDateStruct((((SipExpiresHeader *)(hdr->pHeader))->u).pDate);
		(((SipExpiresHeader *)(hdr->pHeader))->u).pDate = temp_date_struct;
#else
		if (temp_date_struct != SIP_NULL)
			sip_freeSipDateStruct(temp_date_struct);
		HSS_LOCKEDINCREF(date_struct->dRefCount);
		 (((SipExpiresHeader *)(hdr->pHeader))->u).pDate = date_struct;
#endif
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDateStructInExpiresHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getSecondsFromExpiresHdr
**
** DESCRIPTION: This function retrieves the seconds field from a SIP
**		Encryption pHeader
**
***************************************************************/
SipBool sip_getSecondsFromExpiresHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipHeader *hdr;
	SIP_U32bit *dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getSecondsFromExpiresHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dSec == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeExpiresDate)&&(hdr->dType != SipHdrTypeExpiresSec))
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
	*dSec = (((SipExpiresHeader *)(hdr->pHeader))->u).dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSecondsFromExpiresHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSecondsInExpiresHdr
**
** DESCRIPTION: This function sets the seconds field in a SIP Expires
**		pHeader
**
***************************************************************/
SipBool sip_setSecondsInExpiresHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipHeader *hdr;
	SIP_U32bit dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_setSecondsInExpiresHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeExpiresDate)&&(hdr->dType != SipHdrTypeExpiresSec))
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
	(((SipExpiresHeader *)(hdr->pHeader))->u).dSec = dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSecondsInExpiresHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getDateStructFromExpires
**
** DESCRIPTION: This function retrieves the Date from a SIP Expires
**
**
***************************************************************/
SipBool sip_getDateStructFromExpires
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipExpiresStruct *hdr, SipDateStruct *date_struct, SipError *err)
#else
	(SipExpiresStruct *hdr, SipDateStruct **date_struct, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, date_struct, err)
	SipExpiresStruct *hdr;
	SipDateStruct *date_struct;
	SipError *err;
#else
	(hdr, date_struct, err)
	SipExpiresStruct *hdr;
	SipDateStruct **date_struct;
	SipError *err;
#endif
#endif
{
	SipDateStruct *temp_date_struct;
	SIPDEBUGFN("Entering function sip_getDateStructFromExpires");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  date_struct == SIP_NULL)
#else
	if(  date_struct == SIP_NULL)
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
	if( hdr->dType != SipExpDate)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	temp_date_struct = hdr->u.pDate;
	if (temp_date_struct == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipDateStruct(date_struct, temp_date_struct, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_date_struct->dRefCount);
	*date_struct = temp_date_struct;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDateStructFromExpires");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDateStructInExpires
**
** DESCRIPTION: This function sets the dDate in a SIP Expires Struct
**
***************************************************************/
SipBool sip_setDateStructInExpires
#ifdef ANSI_PROTO
	(SipExpiresStruct *hdr, SipDateStruct *date_struct, SipError *err)
#else
	(hdr, date_struct, err)
	SipExpiresStruct *hdr;
	SipDateStruct *date_struct;
	SipError *err;
#endif
{
	SipDateStruct *temp_date_struct;
	SIPDEBUGFN("Entering function sip_setDateStructInExpires");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipExpDate)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif

	temp_date_struct = (hdr->u).pDate;

	if (temp_date_struct != SIP_NULL)
		sip_freeSipDateStruct(temp_date_struct);

	if (date_struct==SIP_NULL)
	{
		hdr->u.pDate = 	SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(date_struct->dRefCount);
		hdr->u.pDate = 	date_struct;
#else
		hdr->dType=SipExpDate;
		if(sip_initSipDateStruct(&((hdr->u).pDate), err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipDateStruct((hdr->u).pDate, date_struct, err) \
				== SipFail)
		{
			sip_freeSipDateStruct((hdr->u).pDate);
			return SipFail;
		}	
#endif
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDateStructInExpires");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getSecondsFromExpires
**
** DESCRIPTION: This function retrieves the seconds field from a SIP
**		Expires Struct
**
***************************************************************/
SipBool sip_getSecondsFromExpires
#ifdef ANSI_PROTO
	(SipExpiresStruct *hdr, SIP_U32bit *dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipExpiresStruct *hdr;
	SIP_U32bit *dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getSecondsFromExpires");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dSec == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipExpSeconds)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	*dSec = hdr->u.dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSecondsFromExpires");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSecondsInExpires
**
** DESCRIPTION: This function sets the seconds field in a SIP Expires
**		Struct
**
***************************************************************/
SipBool sip_setSecondsInExpires
#ifdef ANSI_PROTO
	(SipExpiresStruct *hdr, SIP_U32bit dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipExpiresStruct *hdr;
	SIP_U32bit dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_setSecondsInExpires");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipExpSeconds)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	hdr->u.dSec = dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSecondsInExpires");
	return SipSuccess;
}




/*****************************************************************
**
** FUNCTION:  sip_getDispNameFromFromHdr
**
** DESCRIPTION: This function retrieves the Didsplay pName field
**		from a SIP From Header
**
***************************************************************/
SipBool sip_getDispNameFromFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit **pDispName;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_disp_name;
	SIPDEBUGFN("Entering function sip_getDispNameFromFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pDispName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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

 	temp_disp_name = ((SipFromHeader *)(hdr ->pHeader))->pDispName;
 	if (temp_disp_name == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDispName = (SIP_S8bit *) STRDUPACCESSOR (temp_disp_name);
	if (*pDispName == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pDispName = temp_disp_name;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromFromHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInFromHdr
**
** DESCRIPTION: This function sets the display pName field in a
**		SIp from pHeader
**
***************************************************************/
SipBool sip_setDispNameInFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit *pDispName;
	SipError *err;
#endif
{
        SIP_S8bit *temp_disp_name, *dname;;
	SIPDEBUGFN("Entering function sip_setDispNameInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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
        if( pDispName == SIP_NULL)
                temp_disp_name = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_disp_name = (SIP_S8bit *)STRDUPACCESSOR(pDispName);
		if (temp_disp_name == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_disp_name = pDispName;
#endif
	}

        dname = ((SipFromHeader *)(hdr->pHeader))->pDispName;
        if (dname != SIP_NULL)
        {
        	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&dname), err) == SipFail)
		{
			sip_freeString(temp_disp_name);
        		return SipFail;
		}
        }

	((SipFromHeader *)(hdr->pHeader))->pDispName = temp_disp_name;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getTagCountFromFromHdr
**
** DESCRIPTION: This function gets the number of slTag inserted in a
**		SIP From pHeader
**
***************************************************************/
SipBool sip_getTagCountFromFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getTagCountFromFromHdr");
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
	if( hdr->dType != SipHdrTypeFrom)
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
	if (sip_listSizeOf( &(((SipFromHeader *)(hdr->pHeader))->slTag), count , err) == SipFail )
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTagCountFromFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getTagAtIndexFromFromHdr
**
** DESCRIPTION: This function retrieves a tag at a specified index
**		in a SIP From pHeader
**
***************************************************************/
SipBool sip_getTagAtIndexFromFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **tag, SIP_U32bit index, SipError *err)
#else
	(hdr, tag, index, err)
	SipHeader *hdr;
	SIP_S8bit **tag;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIP_Pvoid element_from_list;
	SIPDEBUGFN("Entering function sip_getTagAtIndexFromFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( tag == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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
	if (sip_listGetAt( &(((SipFromHeader *)(hdr->pHeader))->slTag), index,  \
		&element_from_list, err) == SipFail)
		return SipFail;


	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	*tag = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *)element_from_list);
	if (*tag == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*tag = (SIP_S8bit  *) element_from_list;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTagAtIndexFromFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertTagAtIndexInFromHdr
**
** DESCRIPTION: This function inserts a tag at a specified index
** in a SIP From pHeader
**
***************************************************************/
SipBool sip_insertTagAtIndexInFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *tag, SIP_U32bit index, SipError *err)
#else
	(hdr, tag, index, err)
	SipHeader *hdr;
	SIP_S8bit *tag;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIP_S8bit *element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif
	SIPDEBUGFN("Entering function sip_insertTagAtIndexInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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
	if (tag == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *)STRDUPACCESSOR(tag);
		if (element_in_list == SIP_NULL)
		{	*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = tag;
#endif
	}

	if( sip_listInsertAt( &(((SipFromHeader *)(hdr->pHeader))->slTag),  \
		index, (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (sip_memfree (0, (SIP_Pvoid *)(&element_in_list), &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertTagAtIndexInFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setTagAtIndexInFromHdr
**
** DESCRIPTION: This function sets a tag at a specified index in a
**		Sip Encryption pHeader
**
***************************************************************/
SipBool sip_setTagAtIndexInFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *tag, SIP_U32bit index, SipError *err)
#else
	(hdr, tag, index, err)
	SipHeader *hdr;
	SIP_S8bit *tag;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIP_S8bit *element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif
	SIPDEBUGFN("Entering function sip_setTagAtIndexInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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
	if (tag == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *) STRDUPACCESSOR(tag);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = tag;
#endif
	}

	if( sip_listSetAt( &(((SipFromHeader *)(hdr->pHeader))->slTag),  \
		index, (SIP_Pvoid)element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&element_in_list), &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTagAtIndexInFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteTagAtIndexInFromHdr
**
** DESCRIPTION: This function deletes a tag at a specified index in
**		a SIP From pHeader
**
***************************************************************/
SipBool sip_deleteTagAtIndexInFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteTagAtIndexInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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
	if( sip_listDeleteAt( &(((SipFromHeader *)(hdr->pHeader))->slTag), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteTagAtIndexInFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getExtensionParamCountFromFromHdr
**
** DESCRIPTION: This function gets the number of extension-pParam
**		from a SIP From pHeader
**
***************************************************************/
SipBool sip_getExtensionParamCountFromFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getExtensionParamCountFromFromHdr");
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
	if( hdr->dType != SipHdrTypeFrom)
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
	if (sip_listSizeOf( &(((SipFromHeader *)(hdr->pHeader))->slParam), count , err) == SipFail )
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getExtensionParamCountFromFromHdr");
	return SipSuccess;

}

/*****************************************************************
**
** FUNCTION:  sip_getExtensionParamAtIndexFromFromHdr
**
** DESCRIPTION: This function gets an extension-param at a specified'
**		index from a SIP From pHeader
**
***************************************************************/
SipBool sip_getExtensionParamAtIndexFromFromHdr
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
	SIPDEBUGFN("Entering function sip_getExtParamAtIndexFromFromHdr");
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
	if( hdr->dType != SipHdrTypeFrom)
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

	if (sip_listGetAt( &(((SipFromHeader *)(hdr->pHeader))->slParam), index,  \
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
	SIPDEBUGFN("Exiting function sip_getExtParamAtIndexFromFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertExtensionParamAtIndexInFromHdr
**
** DESCRIPTION: This function inserts an extension-param at a
**		specified index in a SIP From pHeader
**
***************************************************************/
SipBool sip_insertExtensionParamAtIndexInFromHdr
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
	SIPDEBUGFN("Entering function sip_insertExtensionParamAtIndexInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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

	if( sip_listInsertAt( &(((SipFromHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_insertExtensionParamAtIndexInFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setExtensionParamAtIndexInFromHdr
**
** DESCRIPTION: This function sets an extension-param at a specified
**		index in a SIP From pHeader
**
***************************************************************/
SipBool sip_setExtensionParamAtIndexInFromHdr
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
	SIPDEBUGFN("Entering function sip_setExtensionParamAtIndexInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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

	if( sip_listSetAt( &(((SipFromHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_setExtensionParamAtIndexInFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteExtensionParamAtIndexInFromHdr
**
** DESCRIPTION: This function deletes an extension-param at a
**		specified index in a SIP From pHeader
**
***************************************************************/
SipBool sip_deleteExtensionParamAtIndexInFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteExtensionParamAtIndexInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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
	if( sip_listDeleteAt( &(((SipFromHeader *)(hdr->pHeader))->slParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteExtensionParamAtIndexInFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromFromHdr
**
** DESCRIPTION: This function retrrieves the dAddr-spec field from a
**		SIP From pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromFromHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(SipHeader *hdr, SipAddrSpec **ppAddrSpec, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#else
	(hdr, ppAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec **ppAddrSpec;
	SipError *err;
#endif
#endif
{
	SipAddrSpec *temp_addr_spec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pAddrSpec == SIP_NULL)
#else
	if(  ppAddrSpec == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeFrom)
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

 	temp_addr_spec = ((SipFromHeader *)(hdr ->pHeader))->pAddrSpec;
 	if (temp_addr_spec == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipAddrSpec(pAddrSpec, temp_addr_spec, err) == SipFail)
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
	HSS_LOCKEDINCREF(temp_addr_spec->dRefCount);
	*ppAddrSpec = temp_addr_spec;
#endif
 	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromFromHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInFromHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		From pHeader
**
***************************************************************/
SipBool sip_setAddrSpecInFromHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *temp_addr_spec;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInFromHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeFrom)
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
 	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipFromHeader *)(hdr ->pHeader))->pAddrSpec);
 		((SipFromHeader *)(hdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipAddrSpec(&temp_addr_spec, pAddrSpec->dType, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(temp_addr_spec, pAddrSpec, err) == SipFail)
		{
			sip_freeSipAddrSpec(temp_addr_spec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipFromHeader *)(hdr ->pHeader))->pAddrSpec);
		((SipFromHeader *)(hdr ->pHeader))->pAddrSpec = temp_addr_spec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipFromHeader *)(hdr ->pHeader))->pAddrSpec);
		((SipFromHeader *)(hdr ->pHeader))->pAddrSpec = pAddrSpec;
#endif

 	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInFromHdr");
	return SipSuccess;

}

/*****************************************************************
**
** FUNCTION:  sip_getSentByFromViaHdr
**
** DESCRIPTION: This function retrieves the sent-by field from a
**		SIP Via pHeader
**
***************************************************************/
SipBool sip_getSentByFromViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pSentBy, SipError *err)
#else
	(hdr, pSentBy, err)
	SipHeader *hdr;
	SIP_S8bit **pSentBy;
	SipError *err;
#endif
{
	SIP_S8bit *temp_sent_by;
	SIPDEBUGFN("Entering function sip_getSentByFromViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pSentBy == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
 	temp_sent_by = ((SipViaHeader *) (hdr ->pHeader))->pSentBy;
 	if (temp_sent_by == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pSentBy = (SIP_S8bit *)STRDUPACCESSOR(temp_sent_by);
	if (*pSentBy == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pSentBy = temp_sent_by;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSentByFromViaHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSentByInViaHdr
**
** DESCRIPTION: This function sets the sent-by field in a SIP
**		Via pHeader
**
***************************************************************/
SipBool sip_setSentByInViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pSentBy, SipError *err)
#else
	(hdr, pSentBy, err)
	SipHeader *hdr;
	SIP_S8bit *pSentBy;
	SipError *err;
#endif
{
        SIP_S8bit *temp_sent_by, *sby;
	SIPDEBUGFN("Entering function sip_setSentByInViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
        sby = ((SipViaHeader *)(hdr->pHeader))->pSentBy;
        if (sby != SIP_NULL)
        {
        	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&sby), err) == SipFail)
			{
        		return SipFail;
			}
        }

        if( pSentBy == SIP_NULL)
                temp_sent_by = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_sent_by = (SIP_S8bit *)STRDUPACCESSOR(pSentBy);
		if (temp_sent_by == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
#else
		temp_sent_by = pSentBy;
#endif
	}


	((SipViaHeader *)(hdr->pHeader))->pSentBy = temp_sent_by;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSentByInViaHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getSentProtocolFromViaHdr
**
** DESCRIPTION: This function retrieves the sent-protocol field from
**		a SIP Via pHeader
**
***************************************************************/
SipBool sip_getSentProtocolFromViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pSentProtocol, SipError *err)
#else
	(hdr, pSentProtocol, err)
	SipHeader *hdr;
	SIP_S8bit **pSentProtocol;
	SipError *err;
#endif
{
	SIP_S8bit *temp_sent_protocol;
	SIPDEBUGFN("Entering function sip_getSentProtocolFromViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pSentProtocol == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
 	temp_sent_protocol = ((SipViaHeader *)(hdr ->pHeader))->pSentProtocol;
 	if (temp_sent_protocol == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pSentProtocol = (SIP_S8bit *) STRDUPACCESSOR (temp_sent_protocol);
	if (*pSentProtocol == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pSentProtocol = temp_sent_protocol;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSentProtocolFromViaHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSentProtocolInViaHdr
**
** DESCRIPTION: This function sets the sent-protocol field from a
**		SIP Via pHeader
**
***************************************************************/
SipBool sip_setSentProtocolInViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pSentProtocol, SipError *err)
#else
	(hdr, pSentProtocol, err)
	SipHeader *hdr;
	SIP_S8bit *pSentProtocol;
	SipError *err;
#endif
{
	SIP_S8bit *temp_sent_protocol, *sp;
	SIPDEBUGFN("Entering function sip_setSentProtocolInViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
        if( pSentProtocol == SIP_NULL)
                temp_sent_protocol = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
			temp_sent_protocol = (SIP_S8bit *) STRDUPACCESSOR(pSentProtocol);
			if (temp_sent_protocol == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
#else
			temp_sent_protocol = pSentProtocol;
#endif
		}

    sp = ((SipViaHeader *)(hdr->pHeader))->pSentProtocol;

	if ( sp != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&sp), err) == SipFail)
		{
			sip_freeString(temp_sent_protocol);
			return SipFail;
		}
	}

	((SipViaHeader *)(hdr->pHeader))->pSentProtocol = temp_sent_protocol;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSentProtocolInViaHdr");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getViaParamCountFromViaHdr
**
** DESCRIPTION: This function retrrieves the number of via-pParam
**		from a SIP Via pHeader
**
***************************************************************/
SipBool sip_getViaParamCountFromViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getViaParamCountFromViaHdr");
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
	if( hdr->dType != SipHdrTypeVia)
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

	if (sip_listSizeOf( &(((SipViaHeader *)(hdr->pHeader))->slParam), count , err) == SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getViaParamCountFromViaHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getViaParamAtIndexFromViaHdr
**
** DESCRIPTION: This function retrievse a via-param at a specified
**		index from a SIP via pHeader
**
***************************************************************/
SipBool sip_getViaParamAtIndexFromViaHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipParam *via_param, SIP_U32bit index, SipError  *err)
#else
	(SipHeader *hdr, SipParam **via_param, SIP_U32bit index, SipError  *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, via_param, index, err)
	SipHeader *hdr;
	SipParam *via_param;
	SIP_U32bit index;
	SipError  *err;
#else
	(hdr, via_param, index, err)
	SipHeader *hdr;
	SipParam **via_param;
	SIP_U32bit index;
	SipError  *err;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_via_param;
	SIPDEBUGFN("Entering function sip_getViaParamAtIndexFromViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  via_param == SIP_NULL)
#else
	if(  via_param == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeVia)
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
	if (sip_listGetAt( &(((SipViaHeader *)(hdr->pHeader))->slParam), index,  \
		&element_from_list, err) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	temp_via_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(via_param, temp_via_param, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_via_param->dRefCount);
	*via_param = temp_via_param;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getViaParamAtIndexFromViaHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertViaParamAtIndexInViaHdr
**
** DESCRIPTION: This function inserts a via-param at a specified
**		index in a SIP Via pHeader
**
***************************************************************/
SipBool sip_insertViaParamAtIndexInViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipParam *via_param, SIP_U32bit index, SipError *err)
#else
	(hdr, via_param, index, err)
	SipHeader *hdr;
	SipParam *via_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipParam *temp_via_param;
	SIPDEBUGFN("Entering function sip_insertViaParamAtIndexInViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
	if ( via_param == SIP_NULL )
		temp_via_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(via_param->dType, err) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_via_param, err) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_via_param, via_param, err) == SipFail)
		{
			sip_freeSipParam (temp_via_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(via_param->dRefCount);
		temp_via_param = via_param;
#endif
	}

	if( sip_listInsertAt( &(((SipViaHeader *)(hdr->pHeader))->slParam),  \
		index, (SIP_Pvoid)(temp_via_param), err) == SipFail)
	{
		if (temp_via_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_via_param);
#else
		HSS_LOCKEDDECREF(via_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertViaParamAtIndexInViaHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setViaParamAtIndexInViaHdr
**
** DESCRIPTION: This function sets a via-param at a specified index
**		in a SIP Via pHeader
**
***************************************************************/
SipBool sip_setViaParamAtIndexInViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipParam *via_param, SIP_U32bit index, SipError *err)
#else
	(hdr, via_param, index, err)
	SipHeader *hdr;
	SipParam *via_param;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipParam *temp_via_param;
	SIPDEBUGFN("Entering function sip_setViaParamAtIndexInViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
	if ( via_param == SIP_NULL )
		temp_via_param = SIP_NULL;
	else
	{
		/* if (validateSipViaParamType(via_param->dType, err) == SipFail)
			return SipFail; */
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_via_param, err) == SipFail)

			return SipFail;
		if (__sip_cloneSipParam(temp_via_param, via_param, err) == SipFail)
		{
			sip_freeSipParam(temp_via_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(via_param->dRefCount);
		temp_via_param = via_param;
#endif
	}

	if( sip_listSetAt( &(((SipViaHeader *)(hdr->pHeader))->slParam), index,  \
		(SIP_Pvoid)(temp_via_param), err) == SipFail)
	{
		if (temp_via_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam(temp_via_param);
#else
			HSS_LOCKEDDECREF(temp_via_param->dRefCount);
#endif
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setViaParamAtIndexInViaHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteViaParamAtIndexInViaHdr
**
** DESCRIPTION: This function deletes a via-param at a specified
**		index in a SIP Via pHeader
**
***************************************************************/
SipBool sip_deleteViaParamAtIndexInViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteViaParamAtIndexInViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
	if( sip_listDeleteAt( &(((SipViaHeader *)(hdr->pHeader))->slParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteViaParamAtIndexInViaHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getCommentFromViaHdr
**
** DESCRIPTION: This function retrieves the pComment field from a
**		SIP Via pHeader
**
***************************************************************/
SipBool sip_getCommentFromViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pComment, SipError *err)
#else
	(hdr, pComment, err)
	SipHeader *hdr;
	SIP_S8bit **pComment;
	SipError *err;
#endif
{
	SIP_S8bit *temp_comment;
	SIPDEBUGFN("Entering function sip_getCommentFromViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pComment == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
 	temp_comment = ((SipViaHeader *) (hdr ->pHeader))->pComment;
 	if (temp_comment == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pComment = (SIP_S8bit *) STRDUPACCESSOR (temp_comment);
	if (*pComment == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pComment = temp_comment;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getCommentFromViaHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setCommentInViaHdr
**
** DESCRIPTION: This function sets the pComment field in a SIP Via
**		pHeader
**
***************************************************************/
SipBool sip_setCommentInViaHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pComment, SipError *err)
#else
	(hdr, pComment, err)
	SipHeader *hdr;
	SIP_S8bit *pComment;
	SipError *err;
#endif
{
	SIP_S8bit *temp_comment, *cmnt;
	SIPDEBUGFN("Entering function sip_setCommentInViaHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeVia)
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
	if( pComment == SIP_NULL)
                temp_comment = SIP_NULL;
        else
                {
#ifndef SIP_BY_REFERENCE
		 	temp_comment = (SIP_S8bit *) STRDUPACCESSOR(pComment);
			if (temp_comment == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
#else
			temp_comment = pComment;
#endif
		}

 	cmnt = ((SipViaHeader *)(hdr->pHeader))->pComment;
        if ( cmnt != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&cmnt), err) == SipFail)
			return SipFail;
	}
	((SipViaHeader *)(hdr->pHeader))->pComment = temp_comment;

	SIPDEBUGFN("Exiting function sip_setCommentInViaHdr");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getDispNameFromRecordRouteHdr
**
** DESCRIPTION:This function retrieves the display-pName field from
**		a SIP Record Route pHeader
**
***************************************************************/
SipBool sip_getDispNameFromRecordRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit **pDispName;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_disp_name;
	SIPDEBUGFN("Entering function sip_getDispNameFromRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pDispName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeRecordRoute)
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
 	temp_disp_name = ((SipRecordRouteHeader *)(hdr ->pHeader))->pDispName;
 	if (temp_disp_name == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDispName = (SIP_S8bit *)STRDUPACCESSOR(temp_disp_name);
	if (*pDispName == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pDispName = temp_disp_name;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromRecordRouteHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInRecordRouteHdr
**
** DESCRIPTION: This function sets the display-pName field in a SIP
**		Record-Route pHeader
**
***************************************************************/
SipBool sip_setDispNameInRecordRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit *pDispName;
	SipError *err;
#endif
{
	SIP_S8bit *temp_disp_name, *dname;
	SIPDEBUGFN("Entering function sip_setDispNameInRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeRecordRoute)
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
        if( pDispName == SIP_NULL)
                temp_disp_name = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_disp_name = (SIP_S8bit *)STRDUPACCESSOR (pDispName);
		if (temp_disp_name == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_disp_name = pDispName;
#endif
	}

        dname = ((SipRecordRouteHeader *)(hdr->pHeader))->pDispName;
        if (dname != SIP_NULL)
        {
        	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&dname), err) == SipFail)
        		return SipFail;
        }

	((SipRecordRouteHeader *)(hdr->pHeader))->pDispName = temp_disp_name;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInRecordRouteHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromRecordRouteHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP Record route pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromRecordRouteHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(SipHeader *hdr, SipAddrSpec **ppAddrSpec, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#else
	(hdr, ppAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec **ppAddrSpec;
	SipError *err;
#endif
#endif
{
	SipAddrSpec *temp_addr_spec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pAddrSpec == SIP_NULL)
#else
	if(  ppAddrSpec == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeRecordRoute)
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

 	temp_addr_spec = ((SipRecordRouteHeader *) (hdr ->pHeader))->pAddrSpec;
 	if (temp_addr_spec == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	if (__sip_cloneSipAddrSpec(pAddrSpec, temp_addr_spec, err) == SipFail)
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
		pAddrSpec->dType = SipAddrAny;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(temp_addr_spec->dRefCount);
	*ppAddrSpec = temp_addr_spec;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromRecordRouteHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInRecordRouteHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		Record-Route pHeader
**
***************************************************************/
SipBool sip_setAddrSpecInRecordRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *temp_addr_spec;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeRecordRoute)
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
  	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipRecordRouteHeader *) (hdr ->pHeader))->pAddrSpec);
 		((SipRecordRouteHeader *) (hdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipAddrSpec(&temp_addr_spec, pAddrSpec->dType, err ) == SipFail)
			return SipFail;

		if (__sip_cloneSipAddrSpec(temp_addr_spec, pAddrSpec, err) == SipFail)
		{
			sip_freeSipAddrSpec(temp_addr_spec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipRecordRouteHeader *) (hdr ->pHeader))->pAddrSpec);
 		((SipRecordRouteHeader *) (hdr ->pHeader))->pAddrSpec = temp_addr_spec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipRecordRouteHeader *) (hdr ->pHeader))->pAddrSpec);
		((SipRecordRouteHeader *) (hdr ->pHeader))->pAddrSpec = pAddrSpec;
#endif
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInRecordRouteHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromRecordRouteHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		Record-Route pHeader
**
***************************************************************/

SipBool sip_getParamCountFromRecordRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromRecordRouteHdr");
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
	if( pHdr->dType != SipHdrTypeRecordRoute)
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
	if (sip_listSizeOf( &(((SipRecordRouteHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromRecordRouteHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromRecordRouteHdr
**
** DESCRIPTION: This function gets the param field at a given index
**				 in a SIP
**		Record-Route pHeader
**
***************************************************************/

SipBool sip_getParamAtIndexFromRecordRouteHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pParam == SIP_NULL)
#else
	if(  ppParam == SIP_NULL)
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
	if( pHdr->dType != SipHdrTypeRecordRoute)
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

	if (sip_listGetAt( &(((SipRecordRouteHeader *)(pHdr->pHeader))->slParams), dIndex,  \
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromRecordRouteHdr");
	return SipSuccess;
}



/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInRecordRouteHdr
**
** DESCRIPTION: This function sets a param at a specified index
**		in a Record-Route Header
**
***************************************************************/

SipBool sip_setParamAtIndexInRecordRouteHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRecordRoute)
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

	if( sip_listSetAt( &(((SipRecordRouteHeader *)(pHdr->pHeader))->slParams),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInRecordRouteHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInRecordRouteHdr
**
** DESCRIPTION: This function inserts a param at a specified index
**		in a Record-Route Header
**
***************************************************************/
SipBool sip_insertParamAtIndexInRecordRouteHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRecordRoute)
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

	if( sip_listInsertAt( &(((SipRecordRouteHeader *)(pHdr->pHeader))->slParams),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInRecordRouteHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInRecordRouteHdr
**
** DESCRIPTION: This function deletes a param at a specified index
**		in a Record-Route Header
**
***************************************************************/
SipBool sip_deleteParamAtIndexInRecordRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInRecordRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRecordRoute)
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
	if( sip_listDeleteAt( &(((SipRecordRouteHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInRecordRouteHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getTimeFromTimeStampHdr
**
** DESCRIPTION: This function gets the slTime from a SIp Timestamp
**		pHeader
**
***************************************************************/
SipBool sip_getTimeFromTimeStampHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **slTime, SipError *err)
#else
	(hdr, slTime, err)
	SipHeader *hdr;
	SIP_S8bit **slTime;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_time;
	SIPDEBUGFN("Entering function sip_getTimeFromTimeStampHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTimestamp)
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
 	temp_time = ((SipTimeStampHeader *) (hdr ->pHeader))->pTime;
 	if (temp_time == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*slTime = (SIP_S8bit *) STRDUPACCESSOR(temp_time);
	if (*slTime == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*slTime = temp_time;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTimeFromTimeStampHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setTimeInTimeStampHdr
**
** DESCRIPTION: This function sets the slTime in a SIP TimeStamp
**		pHeader
**
***************************************************************/
SipBool sip_setTimeInTimeStampHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *slTime, SipError *err)
#else
	(hdr, slTime, err)
	SipHeader *hdr;
	SIP_S8bit *slTime;
	SipError *err;
#endif
{
	SIP_S8bit *temp_time, *t;
	SIPDEBUGFN("Entering function sip_setTimeInTimeStampHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTimestamp)
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
        if( slTime == SIP_NULL)
                temp_time = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	       	temp_time = (SIP_S8bit *) STRDUPACCESSOR(slTime);
		if (temp_time == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_time = slTime;
#endif
	}

        t = ((SipTimeStampHeader *)(hdr->pHeader))->pTime;
        if (t != SIP_NULL)
        {
        	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&t), err) == SipFail)
		{
			sip_freeString(temp_time);
        		return SipFail;
		}
        }

	((SipTimeStampHeader *)(hdr->pHeader))->pTime = temp_time;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTimeInTimeStampHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getDelayFromTimeStampHdr
**
** DESCRIPTION: This functions retrieves the delaty field from a
** 		SIP Time Stamp pHeader
**
***************************************************************/
SipBool sip_getDelayFromTimeStampHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **delay, SipError *err)
#else
	(hdr, delay, err)
	SipHeader *hdr;
	SIP_S8bit **delay;
	SipError *err;
#endif
{
	SIP_S8bit *temp_delay;
	SIPDEBUGFN("Entering function sip_getDelayFromTimeStampHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( delay == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTimestamp)
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
 	temp_delay = ((SipTimeStampHeader *)(hdr ->pHeader))->delay;
 	if (temp_delay == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*delay = (SIP_S8bit *)STRDUPACCESSOR(temp_delay);
	if (*delay == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*delay = temp_delay;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDelayFromTimeStampHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDelayInTimeStampHdr
**
** DESCRIPTION: This function sets the delay field in a SIP Time-
		-stamp pHeader
**
***************************************************************/
SipBool sip_setDelayInTimeStampHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *delay, SipError *err)
#else
	(hdr, delay, err)
	SipHeader *hdr;
	SIP_S8bit *delay;
	SipError *err;
#endif
{

        SIP_S8bit *temp_delay, *d;
	SIPDEBUGFN("Entering function sip_setDelayInTimeStampHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTimestamp)
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
        if( delay == SIP_NULL)
                temp_delay = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_delay = (SIP_S8bit *)STRDUPACCESSOR(delay);
		if (temp_delay == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_delay = delay;
#endif
        }

        d = ((SipTimeStampHeader *)(hdr->pHeader))->delay;
        if ( d != SIP_NULL )
        {
        	if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&d), err) == SipFail)
		{
			sip_freeString(temp_delay);
        		return SipFail;
		}
        }
	((SipTimeStampHeader *)(hdr->pHeader))->delay = temp_delay;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDelayInTimeStampHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getDispNameFromToHdr
**
** DESCRIPTION: This function retrieves the display-pName field from
**		a SIP To Header
**
***************************************************************/
SipBool sip_getDispNameFromToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit **pDispName;
	SipError *err;
#endif
{
	SIP_S8bit *temp_disp_name;
	SIPDEBUGFN("Entering function sip_getDispNameFromToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pDispName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
 	temp_disp_name = ((SipToHeader *) (hdr ->pHeader))->pDispName;
 	if (temp_disp_name == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDispName = (SIP_S8bit *) STRDUPACCESSOR(temp_disp_name);
	if (*pDispName == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pDispName = temp_disp_name;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromToHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInToHdr
**
** DESCRIPTION: This functions sets the display-pName field in a SIP
**		To Header
**
***************************************************************/
SipBool sip_setDispNameInToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit *pDispName;
	SipError *err;
#endif
{
	SIP_S8bit *temp_disp_name, *dname;
	SIPDEBUGFN("Entering function sip_setDispNameInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
        if( pDispName == SIP_NULL)
                temp_disp_name = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_disp_name = (SIP_S8bit *) STRDUPACCESSOR (pDispName);
		if (temp_disp_name == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_disp_name = pDispName;
#endif
	}

        dname = ((SipToHeader *)(hdr->pHeader))->pDispName;
        if (dname != SIP_NULL)
        {
        	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&dname), err) == SipFail)
		{
			sip_freeString(temp_disp_name);
        		return SipFail;
		}
        }
	((SipToHeader *)(hdr->pHeader))->pDispName = temp_disp_name;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getTagCountFromToHdr
**
** DESCRIPTION: This function retrieves the number of slTag inserted
**		into a SIP To Header
**
***************************************************************/
SipBool sip_getTagCountFromToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getTagCountFromToHdr");
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
	if( hdr->dType != SipHdrTypeTo)
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
	if (sip_listSizeOf( &(((SipToHeader *)(hdr->pHeader))->slTag), count , err) == SipFail )
		return SipFail;

	*err =E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTagCountFromToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getTagAtIndexFromToHdr
**
** DESCRIPTION: This function retrieves a tag at aspecified index
**		in a SIP To pHeader
**
***************************************************************/
SipBool sip_getTagAtIndexFromToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **tag, SIP_U32bit index, SipError *err)
#else
	(hdr, tag, index, err)
	SipHeader *hdr;
	SIP_S8bit **tag;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIP_Pvoid element_from_list;
	SIPDEBUGFN("Entering function sip_getTagAtIndexFromToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( tag == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
	if (sip_listGetAt( &(((SipToHeader *)(hdr->pHeader))->slTag), index,  \
		&element_from_list, err) == SipFail)
		return SipFail;
	if (element_from_list == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	*tag = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *)element_from_list);
	if (*tag == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*tag = (SIP_S8bit *) element_from_list;
#endif

	*err =E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTagAtIndexFromToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertTagAtIndexInToHdr
**
** DESCRIPTION: This function inserts a tag at a specified index in
**		a SIP To pHeader
**
***************************************************************/
SipBool sip_insertTagAtIndexInToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *tag, SIP_U32bit index, SipError *err)
#else
	(hdr, tag, index, err)
	SipHeader *hdr;
	SIP_S8bit *tag;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIP_S8bit *element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif
	SIPDEBUGFN("Entering function sip_insertTagAtIndexInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
	if (tag == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		 element_in_list = (SIP_S8bit *)STRDUPACCESSOR(tag);
		 if (element_in_list == SIP_NULL)
		 {
			*err = E_NO_MEM;
			return SipFail;
		 }
#else
		element_in_list = tag;
#endif
	}


	if (sip_listInsertAt( &(((SipToHeader *)(hdr->pHeader))->slTag), index,  \
		(SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&element_in_list), &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertTagAtIndexInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setTagAtIndexInToHdr
**
** DESCRIPTION: This function sets a tag at a specified index in a
**		SIP To pHeader
**
***************************************************************/
SipBool sip_setTagAtIndexInToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *tag, SIP_U32bit index, SipError *err)
#else
	(hdr, tag, index, err)
	SipHeader *hdr;
	SIP_S8bit *tag;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIP_S8bit *element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif
	SIPDEBUGFN("Entering function sip_setTagAtIndexInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
	if (tag == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *) STRDUPACCESSOR(tag);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list =tag;
#endif

	}

	if (sip_listSetAt( &(((SipToHeader *)(hdr->pHeader))->slTag), index,  \
		(SIP_Pvoid)element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&element_in_list), &temp_err) == SipFail)
			return SipFail;
#endif

		return SipFail;
	}
	*err =E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTagAtIndexInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteTagAtIndexInToHdr
**
** DESCRIPTION: This function deletes a tag at a specified index
**		in a SIP To Header
**
***************************************************************/
SipBool sip_deleteTagAtIndexInToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteTagAtIndexInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
	if( sip_listDeleteAt( &(((SipToHeader *)(hdr->pHeader))->slTag), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteTagAtIndexInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getExtensionParamCountFromToHdr
**
** DESCRIPTION: This function retrieves number of extension-pParam
**		from a SIP To pHeader
**
***************************************************************/
SipBool sip_getExtensionParamCountFromToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getExtensionParamCountFromToHdr");
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
	if( hdr->dType != SipHdrTypeTo)
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
	if (sip_listSizeOf( &(((SipToHeader *)(hdr->pHeader))->slParam),  \
		count , err) == SipFail )
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getExtensionParamCountFromToHdr");
	return SipSuccess;

}

/*****************************************************************
**
** FUNCTION:  sip_getExtensionParamAtIndexFromToHdr
**
** DESCRIPTION: This function retrieves an extension-param at a
**		specified index from a SIP To pHeader
**
***************************************************************/
SipBool sip_getExtensionParamAtIndexFromToHdr
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
	SIPDEBUGFN("Entering function sip_getExtParamAtIndexFromToHdr");
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
	if( hdr->dType != SipHdrTypeTo)
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

	if (sip_listGetAt( &(((SipToHeader *)(hdr->pHeader))->slParam), index,  \
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
	SIPDEBUGFN("Exiting function sip_getExtParamAtIndexFromToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertExtensionParamAtIndexInToHdr
**
** DESCRIPTION: This function inserts an extension-param at a
**		specified index in a SIP To pHeader
**
***************************************************************/
SipBool sip_insertExtensionParamAtIndexInToHdr
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
	SIPDEBUGFN("Entering function sip_insertExtensionParamAtIndexInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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

	if( sip_listInsertAt( &(((SipToHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_insertExtensionParamAtIndexInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setExtensionParamAtIndexInToHdr
**
** DESCRIPTION: This functin sets an extension-param at a specified
**		index in a SIP To Header
**
***************************************************************/
SipBool sip_setExtensionParamAtIndexInToHdr
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
	SIPDEBUGFN("Entering function sip_setExtensionParamAtIndexInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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

	if( sip_listSetAt( &(((SipToHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_setExtensionParamAtIndexInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteExtensionParamAtIndexInToHdr
**
** DESCRIPTION: This function deletes an extension-param at a
**		specified index in a SIp To pHeader
**
***************************************************************/
SipBool sip_deleteExtensionParamAtIndexInToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteExtensionParamAtIndexInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
	if( sip_listDeleteAt( &(((SipToHeader *)(hdr->pHeader))->slParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteExtensionParamAtIndexInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromToHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP To pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromToHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(SipHeader *hdr, SipAddrSpec **ppAddrSpec, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#else
	(hdr, ppAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec **ppAddrSpec;
	SipError *err;
#endif
#endif
{
	SipAddrSpec *temp_addr_spec;

	SIPDEBUGFN("Entering function sip_getAddrSpecFromToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pAddrSpec == SIP_NULL)
#else
	if(  ppAddrSpec == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeTo)
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
 	temp_addr_spec = ((SipToHeader *) (hdr->pHeader))->pAddrSpec;
 	if (temp_addr_spec == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipAddrSpec(pAddrSpec, temp_addr_spec, err) == SipFail)
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
	HSS_LOCKEDINCREF(temp_addr_spec->dRefCount);
	*ppAddrSpec = temp_addr_spec;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromToHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInToHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		To pHeader
**
***************************************************************/
SipBool sip_setAddrSpecInToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *temp_addr_spec;
#endif
 	SIPDEBUGFN("Entering function sip_setAddrSpecInToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeTo)
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
  	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipToHeader *) (hdr ->pHeader))->pAddrSpec);
 		((SipToHeader *) (hdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipAddrSpec(&temp_addr_spec, pAddrSpec->dType, err ) == SipFail)
			return SipFail;

		if (__sip_cloneSipAddrSpec(temp_addr_spec, pAddrSpec, err) == SipFail)
		{
			sip_freeSipAddrSpec(temp_addr_spec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipToHeader *)(hdr ->pHeader))->pAddrSpec);
 		((SipToHeader *) (hdr ->pHeader))->pAddrSpec = temp_addr_spec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipToHeader *)(hdr ->pHeader))->pAddrSpec);
		((SipToHeader *)(hdr->pHeader))->pAddrSpec = pAddrSpec;
#endif
 	}


	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getNameFromUnknownHdr
**
** DESCRIPTION: This function retrieves the pName from an Unknown
**		SIP pHeader
**
***************************************************************/
SipBool sip_getNameFromUnknownHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pName, SipError *err)
#else
	(hdr, pName, err)
	SipHeader *hdr;
	SIP_S8bit **pName;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_name;
	SIPDEBUGFN("Entering function sip_getNameFromUnknownHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeUnknown)
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
 	temp_name = ((SipUnknownHeader *)(hdr ->pHeader))->pName;
 	if (temp_name == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pName = (SIP_S8bit *) STRDUPACCESSOR (temp_name);
	if (*pName == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pName = temp_name;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getNameFromUnknownHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setNameInUnknownHdr
**
** DESCRIPTION: This function sets the pName of an Unknown SIP pHeader
**
***************************************************************/
SipBool sip_setNameInUnknownHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pName, SipError *err)
#else
	(hdr, pName, err)
	SipHeader *hdr;
	SIP_S8bit *pName;
	SipError *err;
#endif
{
	SIP_S8bit *temp_name, *nme;
	SIPDEBUGFN("Entering function sip_setNameInUnknownHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeUnknown)
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
        if( pName == SIP_NULL)
                temp_name = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_name = (SIP_S8bit *) STRDUPACCESSOR(pName);
		if (temp_name == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_name = pName;
#endif
	}

        nme = ((SipUnknownHeader *)(hdr->pHeader))->pName;

	if ( nme != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&nme), err) == SipFail)
		{
			sip_freeString(temp_name);
			return SipFail;
		}
	}

	((SipUnknownHeader *)(hdr->pHeader))->pName = temp_name;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setNameInUnknownHdr");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getBodyFromUnknownHdr
**
** DESCRIPTION: This function retrieves the pBody from an Unknown SIP
**		pHeader
**
***************************************************************/
SipBool sip_getBodyFromUnknownHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pBody, SipError *err)
#else
	(hdr, pBody, err)
	SipHeader *hdr;
	SIP_S8bit **pBody;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_body;
	SIPDEBUGFN("Entering function sip_getBodyFromUnknownHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pBody == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeUnknown)
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
 	temp_body = ((SipUnknownHeader *)(hdr->pHeader))->pBody;
 	if (temp_body == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pBody = (SIP_S8bit *) STRDUPACCESSOR (temp_body);
	if (*pBody == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pBody = temp_body;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getBodyFromUnknownHdr");
 	return SipSuccess;
}

/************************************************************************
**
** FUNCTION:  sip_setBodyInUnknownHdr
**
** DESCRIPTION: This function sets the pBody in an Unknown SIP pHeader
**
************************************************************************/
SipBool sip_setBodyInUnknownHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pBody, SipError *err)
#else
	(hdr, pBody, err)
	SipHeader *hdr;
	SIP_S8bit *pBody;
	SipError *err;
#endif
{
	SIP_S8bit *temp_body, *bdy;
	SIPDEBUGFN("Entering function sip_setBodyInUnknownHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeUnknown)
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
        if(pBody == SIP_NULL)
                temp_body = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_body = (SIP_S8bit *) STRDUPACCESSOR(pBody);
		if (temp_body == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_body = pBody;
#endif
	}

        bdy = ((SipUnknownHeader *)(hdr->pHeader))->pBody;

	if (bdy != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&bdy), err) == SipFail)
		{
			sip_freeString(temp_body);
			return SipFail;
		}
	}

	((SipUnknownHeader *)(hdr->pHeader))->pBody = temp_body;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setBodyInUnknownHdr");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getBodyFromBadHdr
**
** DESCRIPTION: This function retrieves the body from an Unknown SIP
**		header
**
***************************************************************/
SipBool sip_getBodyFromBadHdr
#ifdef ANSI_PROTO
	(SipBadHeader *hdr, SIP_S8bit **pBody, SipError *err)
#else
	(hdr, pBody, err)
	SipBadHeader *hdr;
	SIP_S8bit **pBody;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_body;
	SIPDEBUGFN("Entering function sip_getBodyFromBadHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pBody == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	temp_body = hdr->pBody;
 	if (temp_body == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pBody = (SIP_S8bit *) STRDUPACCESSOR (temp_body);
	if (*pBody == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pBody = temp_body;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getBodyFromBadHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getNameFromBadHdr
**
** DESCRIPTION: This function retrieves the name from an Unknown
**		SIP header
**
***************************************************************/
SipBool sip_getNameFromBadHdr
#ifdef ANSI_PROTO
	(SipBadHeader *hdr, SIP_S8bit **pName, SipError *err)
#else
	(hdr, pName, err)
	SipBadHeader *hdr;
	SIP_S8bit **pName;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_name;
	SIPDEBUGFN("Entering function sip_getNameFromBadHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	temp_name = hdr ->pName;
 	if (temp_name == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pName = (SIP_S8bit *) STRDUPACCESSOR (temp_name);
	if (*pName == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pName = temp_name;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getNameFromBadHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getTypeFromBadHdr
**
** DESCRIPTION: This function retrieves the header type from the bad
**		header. This is the type corresponding to the name of the
**		header.
**
***************************************************************/
SipBool sip_getTypeFromBadHdr
#ifdef ANSI_PROTO
	(SipBadHeader *hdr, en_HeaderType *pType, SipError *err)
#else
	(hdr, pType, err)
	SipBadHeader *hdr;
	en_HeaderType *pType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getTypeFromBadHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pType == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	*pType = hdr ->dType;
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTypeFromBadHdr");
 	return SipSuccess;
}

/************************2nd Level APIs*****************************/

/*****************************************************************
**
** FUNCTION:  sip_getVersionFromStatusLine
**
** DESCRIPTION: This function retrieves the pVersion from a SIP
** 		status line
**
***************************************************************/
SipBool sip_getVersionFromStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, SIP_S8bit **pVersion, SipError *err)
#else
	(line, pVersion, err)
	SipStatusLine *line;
	SIP_S8bit **pVersion;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_version;
 	SIPDEBUGFN("Entering function sip_getVersionFromStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pVersion == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	temp_version = line->pVersion;
 	if (temp_version == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pVersion = (SIP_S8bit *) STRDUPACCESSOR (temp_version);
	if (*pVersion == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pVersion = temp_version;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getVersionFromStatusLine");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setVersionInStatusLine
**
** DESCRIPTION: This function sets the Version field in a SIP status
**		line
**
***************************************************************/
SipBool sip_setVersionInStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, SIP_S8bit *pVersion, SipError *err)
#else
	(line, pVersion, err)
	SipStatusLine *line;
	SIP_S8bit *pVersion;
	SipError *err;
#endif
{
	SIP_S8bit *temp_version, *ver;
 	SIPDEBUGFN("Entering function sip_setVersionInStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
        if( pVersion == SIP_NULL)
                temp_version = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_version = (SIP_S8bit *) STRDUPACCESSOR(pVersion);
		if (temp_version == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_version = pVersion;
#endif
	}

        ver = line->pVersion;

	if ( ver != SIP_NULL)
	{
		if (fast_memfree(ACCESSOR_MEM_ID, ver, err) == SipFail)
		{
			sip_freeString(temp_version);
			return SipFail;
		}
	}

	line->pVersion = temp_version;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setVersionInStatusLine");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getStatusCodeFromStatusLine
**
** DESCRIPTION: This function retrieves the status dCodeNum from a SIP
**		status line
**
***************************************************************/
SipBool sip_getStatusCodeFromStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, en_StatusCode *dCodeNum, SipError *err)
#else
	(line, dCodeNum, err)
	SipStatusLine *line;
	en_StatusCode *dCodeNum;
	SipError *err;
#endif
{
	en_StatusCode temp_code;
 	SIPDEBUGFN("Entering function sip_getStatusCodeFromStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dCodeNum == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	temp_code = (en_StatusCode ) line->dCodeNum;
	if (validateSipStatusCode (temp_code, err) == SipFail)
		return SipFail;
	else
		*dCodeNum = temp_code;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getStatusCodeFromStatusLine");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setStatusCodeInStatusLine
**
** DESCRIPTION: This function sets the status dCodeNum field in a SIP
**		status line
**
***************************************************************/
SipBool sip_setStatusCodeInStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, en_StatusCode dCodeNum, SipError *err)
#else
	(line, dCodeNum, err)
	SipStatusLine *line;
	en_StatusCode dCodeNum;
	SipError *err;
#endif
{
 	SIPDEBUGFN("Entering function sip_setStatusCodeInStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (validateSipStatusCode(dCodeNum, err) == SipFail)
		return SipFail;
	else
		line->dCodeNum = dCodeNum;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setStatusCodeInStatusLine");
	return SipSuccess;
}
/*****************************************************************
**
** FUNCTION:  sip_getStatusCodeNumFromStatusLine
**
** DESCRIPTION: This function retrieves the statsu dCodeNum number from
**		a SIP statsu line
**
***************************************************************/
SipBool sip_getStatusCodeNumFromStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, SIP_U16bit *dCodeNum, SipError *err)
#else
	(line, dCodeNum, err)
	SipStatusLine *line;
	SIP_U16bit *dCodeNum;
	SipError *err;
#endif
{
 	SIPDEBUGFN("Entering function sip_getStatusCodeNumFromStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dCodeNum == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	*dCodeNum = line->dCodeNum;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getStatusCodeNumFromStatusLine");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setStatusCodeNumInStatusLine
**
** DESCRIPTION: This function sets the status dCodeNum number in a SIP
**		statsu line
**
***************************************************************/
SipBool sip_setStatusCodeNumInStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, SIP_U16bit dCodeNum, SipError *err)
#else
	(line, dCodeNum, err)
	SipStatusLine *line;
	SIP_U16bit dCodeNum;
	SipError *err;
#endif
{
 	SIPDEBUGFN("Entering function sip_setStatusCodeNumInStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	line->dCodeNum = dCodeNum;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setStatusCodeNumInStatusLine");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getReasonFromStatusLine
**
** DESCRIPTION: This function retrieves the pReason from a SIP status
**		line
**
***************************************************************/
SipBool sip_getReasonFromStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, SIP_S8bit **pReason, SipError *err)
#else
	(line, pReason, err)
	SipStatusLine *line;
	SIP_S8bit **pReason;
	SipError *err;
#endif
{
	SIP_S8bit *temp_reason;
 	SIPDEBUGFN("Entering function sip_getReasonFromStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pReason == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	temp_reason = line->pReason;
 	if (temp_reason == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pReason = (SIP_S8bit *) STRDUPACCESSOR (temp_reason);
	if (*pReason == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pReason = temp_reason;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getReasonFromStatusLine");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setReasonInStatusLine
**
** DESCRIPTION: This  function sets the pReason field in a SIP status
**		line
**
***************************************************************/
SipBool sip_setReasonInStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *line, SIP_S8bit *pReason, SipError *err)
#else
	(line, pReason, err)
	SipStatusLine *line;
	SIP_S8bit *pReason;
	SipError *err;
#endif
{
	SIP_S8bit *temp_reason, *rsn;
 	SIPDEBUGFN("Entering function sip_setReasonInStatusLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
        if( pReason == SIP_NULL)
                temp_reason = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_reason = (SIP_S8bit *) STRDUPACCESSOR(pReason);
		if (temp_reason == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_reason = pReason;
#endif
	}

        rsn = line->pReason;
	if ( rsn != SIP_NULL)
	{
		if (fast_memfree(ACCESSOR_MEM_ID, rsn, err) == SipFail)
			return SipFail;
	}

	line->pReason = temp_reason;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setReasonInStatusLine");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getMethodFromReqLine
**
** DESCRIPTION: This function retrieves the pMethod filed from a
*		SIP Request line
**
***************************************************************/
SipBool sip_getMethodFromReqLine
#ifdef ANSI_PROTO
	(SipReqLine *line, SIP_S8bit **pMethod, SipError *err)
#else
	(line, pMethod, err)
	SipReqLine *line;
	SIP_S8bit **pMethod;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_method;
 	SIPDEBUGFN("Entering function sip_getMethodFromReqLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pMethod == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	temp_method = line->pMethod;
 	if (temp_method == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pMethod = (SIP_S8bit *) STRDUPACCESSOR (temp_method);
	if (*pMethod == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pMethod = temp_method;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getMethodFromReqLine");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setMethodInReqLine
**
** DESCRIPTION: This function sets the pMethod field in a SIP
**		Request line
**
***************************************************************/
SipBool sip_setMethodInReqLine
#ifdef ANSI_PROTO
	(SipReqLine *line, SIP_S8bit *pMethod, SipError *err)
#else
	(line, pMethod, err)
	SipReqLine *line;
	SIP_S8bit *pMethod;
	SipError *err;
#endif
{
	SIP_S8bit *temp_method, *mthd;
 	SIPDEBUGFN("Entering function sip_setMethodInReqLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
        if( pMethod == SIP_NULL)
                temp_method = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_method = (SIP_S8bit *) STRDUPACCESSOR(pMethod);
		if (temp_method == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_method = pMethod;
#endif
	}

        mthd = line->pMethod;

	if ( mthd != SIP_NULL)
	{
		if (fast_memfree(ACCESSOR_MEM_ID, mthd, err) == SipFail)
			return SipFail;
	}

	line->pMethod = temp_method;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setMethodInReqLine");
        return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromReqLine
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP Request line
**
***************************************************************/
SipBool sip_getAddrSpecFromReqLine
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipReqLine *line, SipAddrSpec *pRequestUri, SipError *err)
#else
	(SipReqLine *line, SipAddrSpec **ppRequestUri, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(line, pRequestUri, err)
	SipReqLine *line;
	SipAddrSpec *pRequestUri;
	SipError *err;
#else
	(line, ppRequestUri, err)
	SipReqLine *line;
	SipAddrSpec **ppRequestUri;
	SipError *err;
#endif
#endif
{
	SipAddrSpec *temp_addr_spec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromReqLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pRequestUri == SIP_NULL)
#else
	if(  ppRequestUri == SIP_NULL)
#endif
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

 	temp_addr_spec = line->pRequestUri;
 	if (temp_addr_spec == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipAddrSpec(pRequestUri, temp_addr_spec, err) == SipFail)
	{
		if(pRequestUri->dType==SipAddrReqUri)
		{
			sip_freeString((pRequestUri->u).pUri);
			(pRequestUri->u).pUri = SIP_NULL;
		}
		else if((pRequestUri->dType==SipAddrSipUri) \
				|| (pRequestUri->dType==SipAddrSipSUri))
		{
			sip_freeSipUrl((pRequestUri->u).pSipUrl);
			(pRequestUri->u).pSipUrl = SIP_NULL;
		}
		pRequestUri->dType=SipAddrAny;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(temp_addr_spec->dRefCount);
	*ppRequestUri = temp_addr_spec;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromReqLine");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInReqLine
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		Request line
**
***************************************************************/
SipBool sip_setAddrSpecInReqLine
#ifdef ANSI_PROTO
	(SipReqLine *line, SipAddrSpec *pRequestUri, SipError *err)
#else
	(line, pRequestUri, err)
	SipReqLine *line;
	SipAddrSpec *pRequestUri;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
 	SipAddrSpec *temp_addr_spec;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInReqLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	if (pRequestUri == SIP_NULL)
	{
		sip_freeSipAddrSpec(line->pRequestUri);
 		line->pRequestUri = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSipAddrSpec(&temp_addr_spec, pRequestUri->dType , err ) == SipFail)
			return SipFail;
 		if (__sip_cloneSipAddrSpec(temp_addr_spec, pRequestUri, err) == SipFail)
		{
			sip_freeSipAddrSpec(temp_addr_spec);
			return SipFail;
       		}
		sip_freeSipAddrSpec(line->pRequestUri);
		line->pRequestUri = temp_addr_spec;
#else
		HSS_LOCKEDINCREF(pRequestUri->dRefCount);
		sip_freeSipAddrSpec (line->pRequestUri);
		line->pRequestUri = pRequestUri;
#endif
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInReqLine");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getVersionFromReqLine
**
** DESCRIPTION: This function retrieves the pVersion field from a
**		SIP Request line
**
***************************************************************/
SipBool sip_getVersionFromReqLine
#ifdef ANSI_PROTO
	(SipReqLine *line, SIP_S8bit **pVersion, SipError *err)
#else
	(line, pVersion, err)
	SipReqLine *line;
	SIP_S8bit **pVersion;
	SipError *err;
#endif
{
	SIP_S8bit *temp_version;
 	SIPDEBUGFN("Entering function sip_getVersioFromReqLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pVersion == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	temp_version = line->pVersion;
 	if (temp_version == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pVersion = (SIP_S8bit *) STRDUPACCESSOR (temp_version);
	if (*pVersion == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pVersion = temp_version;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getVersioFromReqLine");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setVersionInReqLine
**
** DESCRIPTION: This function sets the pVersion field in a SIP
**		Request line
**
***************************************************************/
SipBool sip_setVersionInReqLine
#ifdef ANSI_PROTO
	(SipReqLine *line, SIP_S8bit *pVersion, SipError *err)
#else
	(line, pVersion, err)
	SipReqLine *line;
	SIP_S8bit *pVersion;
	SipError *err;
#endif
{
	SIP_S8bit *temp_version, *ver;
 	SIPDEBUGFN("Entering function sip_setVersionInReqLine");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( line == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
        if( pVersion == SIP_NULL)
                temp_version = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
		temp_version = (SIP_S8bit *) STRDUPACCESSOR(pVersion);
		if (temp_version == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_version = pVersion;
#endif
	}

        ver = line->pVersion;

	if ( ver != SIP_NULL)
	{
		if (fast_memfree(ACCESSOR_MEM_ID, ver, err) == SipFail)
			return SipFail;
	}

	line->pVersion = temp_version;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setVersionInReqLine");
        return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getTypeFromExpiresHeader
**
** DESCRIPTION: This function retrieves the dType of a SIP
**		Expires pHeader
**
**********************************************************/
SipBool sip_getTypeFromExpiresHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr,
	en_ExpiresType *dType,
	SipError *err)
#else
	(hdr, dType , err )
	  SipHeader *hdr;  /* Expires pHeader */
	  en_ExpiresType *dType;
	  SipError * err;
#endif
{
	SIPDEBUGFN( "Entering getTypeFromExpiresHeader");
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
	if(( hdr->dType != SipHdrTypeExpiresDate)&&(hdr->dType != SipHdrTypeExpiresSec)\
		&&(hdr->dType != SipHdrTypeExpiresAny))
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
	*dType = ((SipExpiresHeader *)(hdr->pHeader))->dType;
	SIPDEBUGFN( "Exiting getTypeFromExpiresHeader");
	*err=E_NO_ERROR;

	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getTypeFromExpires
**
** DESCRIPTION: This function retrieves the Type of a SIP
**		Expires Struct
**
**********************************************************/
SipBool sip_getTypeFromExpires
#ifdef ANSI_PROTO
	(SipExpiresStruct *hdr,
	en_ExpiresType *dType,
	SipError *err)
#else
	(hdr, dType , err )
	  SipExpiresStruct *hdr;  /* Expires pHeader */
	  en_ExpiresType *dType;
	  SipError * err;
#endif
{
	SIPDEBUGFN( "Entering getTypeFromExpires");
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
#endif
	*dType = hdr->dType;
	SIPDEBUGFN( "Exiting getTypeFromExpires");
	*err=E_NO_ERROR;

	return SipSuccess;
}
/*****************************************************************
**
** FUNCTION:  sip_getDayOfWeekFromDateStruct
**
** DESCRIPTION: This function retrieves the dDay of week from a SIP
**		Date structure
**
***************************************************************/
SipBool sip_getDayOfWeekFromDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct *dstruct, en_DayOfWeek *dDay, SipError *err)
#else
	(dstruct, dDay, err)
	SipDateStruct *dstruct;
	en_DayOfWeek *dDay;
	SipError *err;
#endif
{
	en_DayOfWeek temp_day;
	SIPDEBUGFN("Entering function sip_getDayOfWeekFromDateStruct");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dDay == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( dstruct == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_day = dstruct->dDow;
	if (temp_day == SipDayNone)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
	if (validateSipDayOfWeek(temp_day,err) == SipFail)
		return SipFail;
	*dDay = temp_day;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDayOfWeekFromDateStruct");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDayOfWeekInDateStruct
**
** DESCRIPTION: This function sets the dDay of week in a SIP Date
**		structure
**
***************************************************************/
SipBool sip_setDayOfWeekInDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct *dstruct, en_DayOfWeek dDay, SipError *err)
#else
	(dstruct, dDay, err)
	SipDateStruct *dstruct;
	en_DayOfWeek dDay;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_setDayOfWeekInDateStruct");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dstruct == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (validateSipDayOfWeek(dDay, err) == SipFail)
		return SipFail;
	else
		dstruct->dDow = dDay;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDayOfWeekInDateStruct");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getDateFormatFromDateStruct
**
** DESCRIPTION: This function retrieves the dDate from a SIP dDate
**		structure
**
***************************************************************/
SipBool sip_getDateFormatFromDateStruct
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipDateStruct *dstruct, SipDateFormat *dDate, SipError *err)
#else
	(SipDateStruct *dstruct, SipDateFormat **pdDate, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(dstruct, dDate, err)
	SipDateStruct *dstruct;
	SipDateFormat *dDate;
	SipError *err;
#else
	(dstruct, pdDate, err)
	SipDateStruct *dstruct;
	SipDateFormat **pdDate;
	SipError *err;
#endif
#endif
{
	SipDateFormat *temp_date_format;
	SIPDEBUGFN("Entering function sip_getDateFormatFromDateStruct");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  dDate == SIP_NULL)
#else
	if(  pdDate == SIP_NULL)
#endif
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( dstruct == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

 	temp_date_format = dstruct->pDate;
	if (temp_date_format == SIP_NULL)
	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}

#ifndef SIP_BY_REFERENCE
 	if (__sip_cloneSipDateFormat(dDate, temp_date_format, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_date_format->dRefCount);
	*pdDate = temp_date_format;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDateFormatFromDateStruct");
 	return SipSuccess;


}

/*****************************************************************
**
** FUNCTION:  sip_setDateFormatInDateStruct
**
** DESCRIPTION: This function sets the dDate in a SIP Date structure
**
***************************************************************/
SipBool sip_setDateFormatInDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct *dstruct, SipDateFormat *dDate, SipError *err)
#else
	(dstruct, dDate, err)
	SipDateStruct *dstruct;
	SipDateFormat *dDate;
	SipError *err;
#endif
{
	SipDateFormat *date_format;
	SIPDEBUGFN("Entering function sip_setDateFormatInDateStruct");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dstruct == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
 	date_format = dstruct->pDate;
	if (dDate==SIP_NULL)
	{
		if ( date_format != SIP_NULL)
 			sip_freeSipDateFormat(date_format);
		dstruct->pDate=SIP_NULL;
	}
	else
	{
		if ( validateSipDateFormat(dDate, err ) == SipFail)
			return SipFail;
#ifndef SIP_BY_REFERENCE
		if(sip_initSipDateFormat(&date_format, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipDateFormat (date_format, dDate, err) == SipFail)
		{
			sip_freeSipDateFormat(date_format);
			return SipFail;
		}
		sip_freeSipDateFormat(dstruct->pDate);
		dstruct->pDate=date_format;
#else
		if ( date_format != SIP_NULL)
 			sip_freeSipDateFormat(date_format);
		HSS_LOCKEDINCREF(dDate->dRefCount);
		dstruct->pDate = dDate;
#endif
	}
 	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDateFormatInDateStruct");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getTimeFormatFromDateStruct
**
** DESCRIPTION: This function retrieves the slTime from a SIP Date
**		structure
**
***************************************************************/
SipBool sip_getTimeFormatFromDateStruct
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipDateStruct *dstruct, SipTimeFormat *slTime, SipError *err)
#else
	(SipDateStruct *dstruct, SipTimeFormat **pslTime, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(dstruct, slTime, err)
	SipDateStruct *dstruct;
	SipTimeFormat *slTime;
	SipError *err;
#else
	(dstruct, pslTime, err)
	SipDateStruct *dstruct;
	SipTimeFormat **pslTime;
	SipError *err;
#endif
#endif
{
	SipTimeFormat *temp_time_format;
	SIPDEBUGFN("Entering function sip_getTimeFormatFromDateStruct");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  slTime == SIP_NULL)
#else
	if(  pslTime == SIP_NULL)
#endif
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( dstruct == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

 	temp_time_format = dstruct->pTime;
	if (temp_time_format == SIP_NULL)
	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipTimeFormat(slTime, temp_time_format, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_time_format->dRefCount);
	*pslTime = temp_time_format;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getTimeFormatFromDateStruct");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setTimeFormatInDateStruct
**
** DESCRIPTION: This function sets the slTime in a SIP Date structure
**
***************************************************************/
SipBool sip_setTimeFormatInDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct *dstruct, SipTimeFormat *slTime, SipError *err)
#else
	(dstruct, slTime, err)
	SipDateStruct *dstruct;
	SipTimeFormat *slTime;
	SipError *err;
#endif
{
	SipTimeFormat *time_format;
	SIPDEBUGFN("Entering function sip_setTimeFormatInDateStruct");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dstruct == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( validateSipTimeFormat(slTime, err ) == SipFail)
		return SipFail;

 	time_format = dstruct->pTime;
	if (time_format != SIP_NULL)
		sip_freeSipTimeFormat(time_format);
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(slTime->dRefCount);
		dstruct->pTime = slTime;
#else
	if (__sip_cloneSipTimeFormat(time_format, slTime, err) == SipFail)
		return SipFail;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setTimeFormatInDateStruct");
 	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_getTypeFromContactHdr
**
** DESCRIPTION: This function retrieves the dType of a SIP
**		Contact pHeader
**
**********************************************************/
SipBool sip_getTypeFromContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr,
	en_ContactType *dType,
	SipError *err)
#else
	(hdr, dType , err )
	  SipHeader *hdr;  /* ContactParam pHeader */
	  en_ContactType *dType;
	  SipError * err;
#endif
{
	SIPDEBUGFN( "Entering getTypeFromContactHeader");
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
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
	*dType = ((SipContactHeader *)(hdr->pHeader))->dType;
	SIPDEBUGFN( "Exiting getTypeFromContactHeader");
	*err=E_NO_ERROR;

	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setTypeInContactHdr
**
** DESCRIPTION: This function sets the dType of a SIP Contact
**		pHeader
**
**********************************************************/
SipBool sip_setTypeInContactHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr,
	en_ContactType dType,
	SipError *err)
#else
	(hdr, dType , err )
	  SipHeader *hdr;  /* ContactParam pHeader */
	  en_ContactType dType;
	  SipError * err;
#endif
{
	SIPDEBUGFN( "Entering setTypeInContactHeader");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(( hdr->dType != SipHdrTypeContactNormal)&&(hdr->dType != SipHdrTypeContactWildCard))
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
	if (validateSipContactType(dType, err) == SipFail)
			return SipFail;

	((SipContactHeader *)(hdr->pHeader))->dType=dType;
	SIPDEBUGFN( "Exiting setTypeInContactHeader");
	*err=E_NO_ERROR;

	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: sip_getOptionFromSupportedHdr
**
** DESCRIPTION: This function retrives the dOption field from the
** Supported pHeader.
**
*****************************************************************/

SipBool sip_getOptionFromSupportedHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit **ppOption, SipError *pErr)
#else
	( hdr, ppOption, pErr )
	  SipHeader *hdr;
	  SIP_S8bit **ppOption;
	  SipError *pErr;
#endif
{
	SIP_S8bit * pTempOption;

	SIPDEBUGFN ( "Entering getOptionFromSupportedHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppOption == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeSupported)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	pTempOption = ( (SipSupportedHeader *) (hdr->pHeader) )->pOption;

	if( pTempOption == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppOption = pTempOption;
#else
	/* duplicating the pValue to be returned */
	*ppOption = (SIP_S8bit *) STRDUPACCESSOR( pTempOption);
	if( *ppOption == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getOptionFromSupportedHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/****************************************************************
**
** FUNCTION: sip_setOptionInSupportedHdr
**
** DESCRIPTION: This function sets the dOption field in the Supported
** pHeader.
**
*****************************************************************/
SipBool sip_setOptionInSupportedHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit *pOption, SipError *pErr)
#else
	( hdr, pOption, pErr )
	  SipHeader *hdr;
	  SIP_S8bit *pOption;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_S8bit * pTempOption;
#endif
	SipSupportedHeader * pTempSuppHdr;

	SIPDEBUGFN ( "Entering setOptionInSupportedHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeSupported)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( hdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* freeing original pValue if it exists */
	pTempSuppHdr = ( SipSupportedHeader *) ( hdr->pHeader);
	if ( pTempSuppHdr->pOption != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pTempSuppHdr->pOption)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempSuppHdr->pOption = pOption;
#else
	if( pOption == SIP_NULL)
		pTempOption = SIP_NULL;
	else
	{
		pTempOption = (SIP_S8bit *) STRDUPACCESSOR ( pOption );
		if ( pTempOption == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempSuppHdr->pOption = pTempOption;
#endif

	SIPDEBUGFN ( "Exiting setOptionInSupportedHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:   sip_getMethodFromAllowHdr
**********************************************************************
**
** DESCRIPTION: Gets the pMethod field pValue from allow pHeader.
**
*********************************************************************/
SipBool sip_getMethodFromAllowHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit **pMethod, SipError *err)
#else
	( hdr, pMethod, err )
	  SipHeader 	*hdr;
	  SIP_S8bit 	**pMethod;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit 	dLength;
#endif
	SIP_S8bit 	*temp_method;

	SIPDEBUGFN ( "Entering getMethodFromAllowHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pMethod == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAllow)
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
	temp_method = ( (SipAllowHeader *) ( hdr->pHeader) )->pMethod;
	if ( temp_method == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*pMethod = temp_method;
#else
	dLength = strlen( temp_method);

	*pMethod = ( SIP_S8bit * ) fast_memget(ACCESSOR_MEM_ID, dLength+1, err);
	if ( *pMethod == SIP_NULL )
		return SipFail;

	strcpy( *pMethod, temp_method);
#endif

	SIPDEBUGFN ( "Exiting getMethodFromAllowHdr");
	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:   sip_setMethodInAllowHdr
**********************************************************************
**
** DESCRIPTION: Sets the pMethod field in allow pHeader.
**
*********************************************************************/
SipBool sip_setMethodInAllowHdr
#ifdef ANSI_PROTO
	( SipHeader *hdr, SIP_S8bit *pMethod, SipError *err)
#else
	( hdr, pMethod, err )
	  SipHeader 	*hdr;
	  SIP_S8bit 	*pMethod;
	  SipError 	*err;
#endif
{
	SipAllowHeader *temp_all_hdr;
	SIP_S8bit 	*temp_method;

	SIPDEBUGFN ( "Entering setMethodInAllowHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeAllow)
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
	if ( pMethod == SIP_NULL )
		temp_method = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		temp_method = pMethod;
#else
		temp_method = ( SIP_S8bit * ) fast_memget(ACCESSOR_MEM_ID, strlen(pMethod)+1, err);
		if ( temp_method == SIP_NULL )
			return SipFail;

		strcpy( temp_method, pMethod );
#endif
	}

	temp_all_hdr = ( SipAllowHeader *) (hdr->pHeader);

	if( temp_all_hdr->pMethod != SIP_NULL )
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(temp_all_hdr->pMethod)), err) == SipFail)
			return SipFail;

	temp_all_hdr->pMethod = temp_method;

	SIPDEBUGFN ( "Exiting setMethodInAllowHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: sip_getUriFromCallInfoHdr
**
** DESCRIPTION: This function retrives the Uri field from the
** CallInfo pHeader.
**
*****************************************************************/

SipBool sip_getUriFromCallInfoHdr
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

	SIPDEBUGFN ( "Entering getUriFromCallInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppUri == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeCallInfo)
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
	pUri = ( (SipCallInfoHeader *) (pHdr->pHeader) )->pUri;

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

	SIPDEBUGFN ( "Exiting getUriFromCallInfoHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/****************************************************************
**
** FUNCTION: sip_setUriInCallInfoHdr
**
** DESCRIPTION: This function sets the dOption field in the Supported
** pHeader.
**
*****************************************************************/
SipBool sip_setUriInCallInfoHdr
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
	SIP_S8bit * pTempCallInfo;
#endif
	SipCallInfoHeader * pTempCallInfoHdr;

	SIPDEBUGFN ( "Entering setUriInCallInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeCallInfo)
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
	/* freeing original pValue if it exists */
	pTempCallInfoHdr = ( SipCallInfoHeader *) ( pHdr->pHeader);
	if ( pTempCallInfoHdr->pUri != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pTempCallInfoHdr->pUri)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempCallInfoHdr->pUri = pUri;
#else
	if( pUri == SIP_NULL)
		pTempCallInfo = SIP_NULL;
	else
	{
		pTempCallInfo = (SIP_S8bit *) STRDUPACCESSOR ( pUri );
		if ( pTempCallInfo == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempCallInfoHdr->pUri = pTempCallInfo;
#endif

	SIPDEBUGFN ( "Exiting setUriInCallInfoHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromCallInfoHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Encryption pHeader
**
***************************************************************/
SipBool sip_getParamCountFromCallInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromCallInfoHdr");
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
	if( hdr->dType != SipHdrTypeCallInfo)
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
	if (sip_listSizeOf( &(((SipCallInfoHeader *)(hdr->pHeader))->slParam), count , err) == SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromCallInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromCallInfoHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP CallInfo Header
**
***************************************************************/
SipBool sip_getParamAtIndexFromCallInfoHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromCallInfoHdr");
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
	if( hdr->dType != SipHdrTypeCallInfo)
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

	if (sip_listGetAt( &(((SipCallInfoHeader *)(hdr->pHeader))->slParam), index,  \
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromCallInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInCallInfoHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp CallInfo pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInCallInfoHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInCallInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCallInfo)
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

	if( sip_listInsertAt( &(((SipCallInfoHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInCallInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInCallInfoHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP CallInfo pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInCallInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInCallInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCallInfo)
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
	if( sip_listDeleteAt( &(((SipCallInfoHeader *)(hdr->pHeader))->slParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInCallInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInCallInfoHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP CallInfo pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInCallInfoHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInCallInfoHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeCallInfo)
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

	if( sip_listSetAt( &(((SipCallInfoHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInCallInfoHdr");
	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: sip_getDispTypeFromContentDispositionHdr
**
** DESCRIPTION: This function retrives the display typei field from the
** ContentDisposition pHeader.
**
*****************************************************************/

SipBool sip_getDispTypeFromContentDispositionHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit **ppDispType, SipError *pErr)
#else
	( pHdr, ppDispType, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit **ppDispType;
	  SipError *pErr;
#endif
{
	SIP_S8bit * pDispType;

	SIPDEBUGFN ( "Entering getDispTypeFromContentDispositionHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppDispType == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentDisposition)
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
	pDispType = ( (SipContentDispositionHeader *) (pHdr->pHeader) )->pDispType;

	if( pDispType == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppDispType = pDispType;
#else
	/* duplicating the pValue to be returned */
	*ppDispType = (SIP_S8bit *) STRDUPACCESSOR( pDispType);
	if( *ppDispType == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getDispTypeFromContentDispositionHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/****************************************************************
**
** FUNCTION: sip_setDispTypeInContentDispositionHdr
**
** DESCRIPTION: This function sets the pDispType field in the ContentDisposition
** Header.
**
*****************************************************************/
SipBool sip_setDispTypeInContentDispositionHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit *pDispType, SipError *pErr)
#else
	( pHdr, pDispType, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit *pDispType;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_S8bit * pTempContentDisposition;
#endif
	SipContentDispositionHeader * pTempContentDispositionHdr;

	SIPDEBUGFN ( "Entering setDispTypeInContentDispositionHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentDisposition)
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
	/* freeing original pValue if it exists */
	pTempContentDispositionHdr = ( SipContentDispositionHeader *) ( pHdr->pHeader);
	if ( pTempContentDispositionHdr->pDispType != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pTempContentDispositionHdr->pDispType)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempContentDispositionHdr->pDispType = pDispType;
#else
	if( pDispType == SIP_NULL)
		pTempContentDisposition = SIP_NULL;
	else
	{
		pTempContentDisposition = (SIP_S8bit *) STRDUPACCESSOR ( pDispType );
		if ( pTempContentDisposition == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempContentDispositionHdr->pDispType = pTempContentDisposition;
#endif

	SIPDEBUGFN ( "Exiting setDispTypeInContentDispositionHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromContentDispositionHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Encryption pHeader
**
***************************************************************/
SipBool sip_getParamCountFromContentDispositionHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromContentDispositionHdr");
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
	if( hdr->dType != SipHdrTypeContentDisposition)
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
	if (sip_listSizeOf( &(((SipContentDispositionHeader *)(hdr->pHeader))->slParam), count , err) == SipFail )
	{
		return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromContentDispositionHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromContentDispositionHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP ContentDisposition Header
**
***************************************************************/
SipBool sip_getParamAtIndexFromContentDispositionHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromContentDispositionHdr");
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
	if( hdr->dType != SipHdrTypeContentDisposition)
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

	if (sip_listGetAt( &(((SipContentDispositionHeader *)(hdr->pHeader))->slParam), index,  \
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromContentDispositionHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInContentDispositionHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp ContentDisposition pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInContentDispositionHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInContentDispositionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentDisposition)
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

	if( sip_listInsertAt( &(((SipContentDispositionHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInContentDispositionHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInContentDispositionHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP ContentDisposition pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInContentDispositionHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInContentDispositionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentDisposition)
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
	if( sip_listDeleteAt( &(((SipContentDispositionHeader *)(hdr->pHeader))->slParam), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInContentDispositionHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInContentDispositionHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP ContentDisposition pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInContentDispositionHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInContentDispositionHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeContentDisposition)
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

	if( sip_listSetAt( &(((SipContentDispositionHeader *)(hdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInContentDispositionHdr");
	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: sip_getLangTagFromContentLanguageHdr
**
** DESCRIPTION: This function retrives the LangTag field from the
** ContentLanguage pHeader.
**
*****************************************************************/

SipBool sip_getLangTagFromContentLanguageHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit **ppLangTag, SipError *pErr)
#else
	( pHdr, ppLangTag, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit **ppLangTag;
	  SipError *pErr;
#endif
{
	SIP_S8bit * pLangTag;

	SIPDEBUGFN ( "Entering getLangTagFromContentLanguageHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppLangTag == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentLanguage)
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
	pLangTag = ( (SipContentLanguageHeader *) (pHdr->pHeader) )->pLangTag;

	if( pLangTag == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppLangTag = pLangTag;
#else
	/* duplicating the pValue to be returned */
	*ppLangTag = (SIP_S8bit *) STRDUPACCESSOR( pLangTag);
	if( *ppLangTag == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting getLangTagFromContentLanguageHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/****************************************************************
**
** FUNCTION: sip_setLangTagInContentLanguageHdr
**
** DESCRIPTION: This function sets the LangTag field in the ContentLanguage
** pHeader.
**
*****************************************************************/
SipBool sip_setLangTagInContentLanguageHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit *pLangTag, SipError *pErr)
#else
	( pHdr, pLangTag, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit *pLangTag;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_S8bit * pTempContentLang;
#endif
	SipContentLanguageHeader * pTempContentLangHdr;

	SIPDEBUGFN ( "Entering setLangTagInContentLanguageHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeContentLanguage)
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
	/* freeing original pValue if it exists */
	pTempContentLangHdr = ( SipContentLanguageHeader *) ( pHdr->pHeader);
	if ( pTempContentLangHdr->pLangTag != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pTempContentLangHdr->pLangTag)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempContentLangHdr->pLangTag = pLangTag;
#else
	if( pLangTag == SIP_NULL)
		pTempContentLang = SIP_NULL;
	else
	{
		pTempContentLang = (SIP_S8bit *) STRDUPACCESSOR ( pLangTag );
		if ( pTempContentLang == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempContentLangHdr->pLangTag = pTempContentLang;
#endif

	SIPDEBUGFN ( "Exiting setLangTagInContentLanguageHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:sip_getTokenFromRequireHdr
**
** DESCRIPTION: This function retrieves the pToken from a SIP
**		Require pHeader
**
**********************************************************/
SipBool sip_getTokenFromRequireHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppToken,
	SipError *pErr)
#else
	( pHdr, ppToken ,pErr)
	  SipHeader * pHdr;	/* Require pHeader */
	  SIP_S8bit **ppToken;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_token;
	SIPDEBUGFN( "Entering getTokenFromRequireHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppToken == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRequire)
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
	pTemp_token = ( (SipRequireHeader *) (pHdr->pHeader) )->pToken;


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
	SIPDEBUGFN( "Exiting getTokenFromRequireHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setTokenInRequireHdr
**
** DESCRIPTION: This function sets the toekn in a SIP Require
**		pHeader
**
**********************************************************/
SipBool sip_setTokenInRequireHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit *pToken,
	SipError *pErr)
#else
	( pHdr, pToken , pErr )
	  SipHeader * pHdr;	/* Require pHeader */
	  SIP_S8bit *pToken;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_token;
#endif
	SipRequireHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setTokenInRequireHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRequire)
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
	pTemp_hdr=(SipRequireHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pToken !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pToken),pErr) ==SipFail)
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	((SipRequireHeader *)(pHdr->pHeader))->pToken = pToken;
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
	SIPDEBUGFN( "Exiting setTokenInRequireHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:sip_getOrganizationFromOrganizationHdr
**
** DESCRIPTION: This function retrieves the organization field
**		from a SIP Organization pHeader
**
**********************************************************/
SipBool sip_getOrganizationFromOrganizationHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppOrganization,
	SipError *pErr)
#else
	( pHdr, ppOrganization , pErr )
	  SipHeader * pHdr;	/* Organization pHeader */
	  SIP_S8bit **ppOrganization;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_organization;
	SIPDEBUGFN( "Entering getOrganizationFromOrganizationHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppOrganization == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeOrganization)
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
	pTemp_organization = ( (SipOrganizationHeader *) (pHdr->pHeader) )->pOrganization;

	if( pTemp_organization == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppOrganization = pTemp_organization;

#else
	dLength = strlen(pTemp_organization );
	*ppOrganization = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppOrganization == SIP_NULL )
		return SipFail;

	strcpy( *ppOrganization, pTemp_organization );
#endif
	SIPDEBUGFN( "Exiting getOrganizationFromOrganizationHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_setOrganizationInOrganizationHdr
**
** DESCRIPTION: This function sets the Organization field in
**		a SIP Organization pHeader
**
**********************************************************/
SipBool sip_setOrganizationInOrganizationHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit *pOrganization,
	SipError *pErr)
#else
	( pHdr, pOrganization, pErr )
	 SipHeader *pHdr;
	 SIP_S8bit *pOrganization;	/* Organization pHeader */
	 SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_organization;
#endif
	SipOrganizationHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setOrganizationInOrganizationHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeOrganization)
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
	pTemp_hdr=(SipOrganizationHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pOrganization !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pOrganization),pErr) ==SipFail)
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	((SipOrganizationHeader *)(pHdr->pHeader))->pOrganization = pOrganization;
#else
	if( pOrganization == SIP_NULL)
		pTemp_organization = SIP_NULL;
	else
	{
		dLength = strlen( pOrganization );
		pTemp_organization = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_organization == SIP_NULL )
			return SipFail;

		strcpy( pTemp_organization, pOrganization );
	}
	pTemp_hdr->pOrganization = pTemp_organization;

#endif

	SIPDEBUGFN( "Exiting setOrganizationInOrganizationHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:sip_getAgentFromUserAgentHdr
**
** DESCRIPTION: This function retrieves the pAgent- field from
**		a SIP pUser-Agent pHeader
**
**********************************************************/
SipBool sip_getAgentFromUserAgentHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit **ppAgent,
	SipError *pErr)
#else
	( pHdr, ppAgent ,pErr)
	  SipHeader * pHdr;	/* UserAgent pHeader */
	  SIP_S8bit **ppAgent;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_agent;
	SIPDEBUGFN( "Entering getAgentFromUserAgentHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppAgent == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeUserAgent)
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
	pTemp_agent = ( (SipUserAgentHeader *) (pHdr->pHeader) )->pValue;


	if( pTemp_agent == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppAgent = pTemp_agent;
#else
	dLength = strlen(pTemp_agent );
	*ppAgent = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppAgent == SIP_NULL )
		return SipFail;

	strcpy( *ppAgent, pTemp_agent );
#endif
	SIPDEBUGFN( "Exiting getAgentFromUserAgentHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:sip_setAgentInUserAgentHdr
**
** DESCRIPTION: This function sets the pAgent field in a SIP
*8		User-Agent pHeader
**
**********************************************************/
SipBool sip_setAgentInUserAgentHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr,
	SIP_S8bit *pAgent,
	SipError *pErr)
#else
	( pHdr, pAgent , pErr )
	  SipHeader * pHdr;	/* UserAgent pHeader */
	  SIP_S8bit *pAgent;
	  SipError * pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_agent;
#endif
	SipUserAgentHeader *pTemp_hdr;
	SIPDEBUGFN( "Entering setAgentInUserAgentHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeUserAgent)
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
	pTemp_hdr=(SipUserAgentHeader *)(pHdr->pHeader);
	if(pTemp_hdr->pValue !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pTemp_hdr->pValue),pErr) ==SipFail)
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE

	((SipUserAgentHeader *)(pHdr->pHeader))->pValue = pAgent;
#else
	if( pAgent == SIP_NULL)
		pTemp_agent = SIP_NULL;
	else
	{
		dLength = strlen( pAgent );
		pTemp_agent = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_agent == SIP_NULL )
			return SipFail;

		strcpy( pTemp_agent, pAgent );
	}
	pTemp_hdr->pValue = pTemp_agent;
#endif
	SIPDEBUGFN( "Exiting setAgentInUserAgentHdr");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


#ifdef SIP_SESSIONTIMER
/*****************************************************************
**
** FUNCTION:  sip_getSecondsFromSessionExpiresHdr
**
** DESCRIPTION: This function retrieves the seconds field from a SIP
**		SessionExpires pHeader
**
***************************************************************/
SipBool sip_getSecondsFromSessionExpiresHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipHeader *hdr;
	SIP_U32bit *dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getSecondsFromSessionExpiresHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dSec == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeSessionExpires)
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
	*dSec = (((SipSessionExpiresHeader *)(hdr->pHeader)))->dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSecondsFromSessionExpiresHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSecondsInSessionExpiresHdr
**
** DESCRIPTION: This function sets the seconds field in a SIP SessionExpires
**		pHeader
**
***************************************************************/
SipBool sip_setSecondsInSessionExpiresHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipHeader *hdr;
	SIP_U32bit dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_setSecondsInSessionExpiresHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeSessionExpires)
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
	(((SipSessionExpiresHeader *)(hdr->pHeader)))->dSec = dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSecondsInSessionExpiresHdr");
	return SipSuccess;
}
/********************************************************************
** FUNCTION:sip_getRefresherFromSessionExpiresHdr
**
** DESCRIPTION: This function retrieves the Refresher from an
**		SessionExpires Header
**
**
**********************************************************************/
SipBool sip_getRefresherFromSessionExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *pSessionExpires, SIP_S8bit **pRefresher, SipError *pErr )
#else
	( pSessionExpires,pRefresher,pErr)
	  SipHeader *pSessionExpires;
	  SIP_S8bit **pRefresher;
	  SipError *pErr;
#endif
{
	SIP_U32bit dCount=0,dI=0;
	SipNameValuePair *pElement = SIP_NULL;
	
	SIPDEBUGFN("Entering sip_getRefresherFromSessionExpiresHdr \n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pSessionExpires == SIP_NULL) || ( pRefresher == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSessionExpires->dType != SipHdrTypeSessionExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pSessionExpires->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->slNameValue),\
		&dCount , pErr) == SipFail )
	{
		return SipFail;
	}
	
	if ( dCount == 0 )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	for(dI=0;dI<dCount;dI++)
	{
		if( sip_listGetAt(&(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->\
			slNameValue),dI,(SIP_Pvoid*)&pElement, pErr) == SipFail)
		return SipFail;

		if ( pElement != SIP_NULL )
		{
			if ( strcasecmp(pElement->pName,"Refresher") == 0 )
			{
#ifdef SIP_BY_REFERENCE
				*pRefresher = pElement->pValue;
#else
			 	*pRefresher = (SIP_S8bit *)STRDUPACCESSOR(pElement->pValue);
				if (*pRefresher == SIP_NULL)
				{
					*pErr = E_NO_MEM;
					return SipFail;
				}
#endif
				return SipSuccess;
			}
		}
	}
	
	SIPDEBUGFN("Exiting sip_getRefresherFromSessionExpiresHdr\n");

	*pErr = E_NO_EXIST;
	return SipFail;
}

/********************************************************************
** FUNCTION:sip_setRefresherInSessionExpiresHdr
**
** DESCRIPTION: This function sets the Refresher into a 
**		SessionExpires Header
**
**
**********************************************************************/
SipBool sip_setRefresherInSessionExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *pSessionExpires, SIP_S8bit *pRefresher, SipError *pErr )
#else
	( pSessionExpires,pRefresher,pErr)
	  SipHeader *pSessionExpires;
	  SIP_S8bit *pRefresher;
	  SipError *pErr;
#endif
{
	SIP_U32bit dCount=0,dI=0;
	SipNameValuePair *pElement = SIP_NULL;
	
	SIPDEBUGFN("Entering sip_setRefresherInSessionExpiresHdr \n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if ( pSessionExpires == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSessionExpires->dType != SipHdrTypeSessionExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pSessionExpires->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->slNameValue),\
		&dCount , pErr) == SipFail )
	{
		return SipFail;
	}
	
	for(dI=0;dI<dCount;dI++)
	{
		if( sip_listGetAt(&(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->\
			slNameValue),dI,(SIP_Pvoid*)&pElement, pErr) == SipFail)
			return SipFail;

		if ( pElement != SIP_NULL )
		{
			if ( strcasecmp(pElement->pName,"Refresher") == 0 )
			{
				if ( pRefresher == SIP_NULL )
					pElement->pValue = SIP_NULL;
				else
				{
#ifdef SIP_BY_REFERENCE
					pElement->pValue = pRefresher;
#else
			 		pElement->pValue = (SIP_S8bit *)STRDUPACCESSOR(pRefresher);
					if (pElement->pValue == SIP_NULL)
					{
						*pErr = E_NO_MEM;
						return SipFail;
					}
#endif
				}
				return SipSuccess;
			}
		}
	}
	if ( sip_initSipNameValuePair(&pElement,pErr) == SipFail )
		return SipFail;
	pElement->pName = (SIP_S8bit *)STRDUPACCESSOR("Refresher");

#ifdef SIP_BY_REFERENCE
	pElement->pValue = pRefresher;
#else
	pElement->pValue = (SIP_S8bit *)STRDUPACCESSOR(pRefresher);
	if (pElement->pValue == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		__sip_freeSipNameValuePair(pElement);	
		return SipFail;
	}
#endif
	
	if ( sip_listAppend(&(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->\
			slNameValue),(SIP_Pvoid)pElement,pErr) == SipFail )
	{
		__sip_freeSipNameValuePair(pElement);	
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_setRefresherInSessionExpiresHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_getNameValuePairCountFromSessionExpiresHdr
**
** DESCRIPTION: This function retrieves the number of NameValuePairs from an
**		SessionExpires Header
**
**
**********************************************************************/
SipBool sip_getNameValuePairCountFromSessionExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *pSessionExpires, SIP_U32bit *pCount, SipError *pErr )
#else
	( pSessionExpires,pCount,pErr)
	  SipHeader *pSessionExpires;
	  SIP_U32bit *pCount;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_getNameValuePairCountFromSessionExpiresHdr \n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pSessionExpires == SIP_NULL) || ( pCount == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSessionExpires->dType != SipHdrTypeSessionExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pSessionExpires->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->slNameValue),\
		pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_getNameValuePairCountFromSessionExpiresHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_getNameValuePairAtIndexFromSessionExpiresHdr
**
** DESCRIPTION: This function retrieves NameValuePair at specified index
**		from SessionExpires Header
**
*******************************************************************/

SipBool sip_getNameValuePairAtIndexFromSessionExpiresHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader *pSessionExpires, SipNameValuePair **ppNameValue,\
	 SIP_U32bit index, SipError *pErr )
#else
	( SipHeader *pSessionExpires, SipNameValuePair *pNameValue,\
	 SIP_U32bit index, SipError *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pSessionExpires,ppNameValue,index,pErr)
	  SipHeader *pSessionExpires;
	  SipNameValuePair **ppNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#else
	( pSessionExpires,pNameValue,index,pErr)
	  SipHeader *pSessionExpires;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list=SIP_NULL;
	SIPDEBUGFN("Entering sip_getNameValuePairAtIndexFromSessionExpiresHdr\n");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

#ifdef SIP_BY_REFERENCE
	if (( pSessionExpires == SIP_NULL) || ( ppNameValue == SIP_NULL ))
#else
	if (( pSessionExpires == SIP_NULL) || ( pNameValue == SIP_NULL ))
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSessionExpires->dType != SipHdrTypeSessionExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pSessionExpires->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->slNameValue), \
		index,&element_from_list, pErr) == SipFail)
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

	SIPDEBUGFN("Exiting sip_getNameValuePairAtIndexFromSessionExpiresHdr\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************
** FUNCTION: sip_setNameValuePairAtIndexInSessionExpiresHdr
**
** DESCRIPTION: This function sets NameValuePair at specified index
**		in SessionExpires Header
**
**
*******************************************************************/
SipBool sip_setNameValuePairAtIndexInSessionExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *pSessionExpires, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pSessionExpires,pNameValue,dIndex,pErr)
	  SipHeader *pSessionExpires;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_setNameValuePairAtIndexInSessionExpiresHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pSessionExpires == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSessionExpires->dType != SipHdrTypeSessionExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pSessionExpires->pHeader == SIP_NULL )
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

	if( sip_listSetAt( &(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->slNameValue), \
		dIndex,(SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_setNameValuePairAtIndexInSessionExpiresHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_insertNameValuePairAtIndexInSessionExpiresHdr
**
** DESCRIPTION: This function inserts NameValuePair at specified index
**		in SessionExpires Header
**
**
*******************************************************************/
SipBool sip_insertNameValuePairAtIndexInSessionExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *pSessionExpires, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pSessionExpires,pNameValue,dIndex,pErr)
	  SipHeader *pSessionExpires;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_insertNameValuePairAtIndexInSessionExpiresHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pSessionExpires == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSessionExpires->dType != SipHdrTypeSessionExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pSessionExpires->pHeader == SIP_NULL )
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

	if( sip_listInsertAt( &(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->\
		slNameValue), dIndex,(SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_insertNameValuePairAtIndexInSessionExpiresHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_deleteNameValuePairAtIndexInSessionExpiresHdr
**
** DESCRIPTION: This function deletes NameValuePair at specified index
**		in SessionExpires Header
**
**
**********************************************************************/

SipBool sip_deleteNameValuePairAtIndexInSessionExpiresHdr
#ifdef ANSI_PROTO
	( SipHeader *pSessionExpires, SIP_U32bit dIndex, SipError *pErr )
#else
	( pSessionExpires,dIndex,pErr)
	  SipHeader *pSessionExpires;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering sip_deleteNameValuePairAtIndexInSessionExpiresHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pSessionExpires == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSessionExpires->dType != SipHdrTypeSessionExpires )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pSessionExpires->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipSessionExpiresHeader*)(pSessionExpires->pHeader))->\
		slNameValue), dIndex, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting sip_deleteNameValuePairAtIndexInSessionExpiresHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getSecondsFromMinSEHdr
**
** DESCRIPTION: This function retrieves the seconds field from a SIP
**		MinSE Header
**
***************************************************************/
SipBool sip_getSecondsFromMinSEHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipHeader *hdr;
	SIP_U32bit *dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getSecondsFromMinSEHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( dSec == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeMinSE)
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
	*dSec = ((SipMinSEHeader *)(hdr->pHeader))->dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getSecondsFromMinSEHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setSecondsInMinSEHdr
**
** DESCRIPTION: This function sets the seconds field in a SIP MinSE
**		Header
**
***************************************************************/
SipBool sip_setSecondsInMinSEHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit dSec, SipError *err)
#else
	(hdr, dSec, err)
	SipHeader *hdr;
	SIP_U32bit dSec;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_setSecondsInMinSEHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeMinSE)
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
	((SipMinSEHeader *)(hdr->pHeader))->dSec = dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setSecondsInMinSEHdr");
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_getNameValuePairCountFromMinSEHdr
**
** DESCRIPTION: This function retrieves the number of NameValuePairs from an
**		MinSE Header
**
**
**********************************************************************/
SipBool sip_getNameValuePairCountFromMinSEHdr
#ifdef ANSI_PROTO
	( SipHeader *pMinSE, SIP_U32bit *pCount, SipError *pErr )
#else
	( pMinSE,pCount,pErr)
	  SipHeader *pMinSE;
	  SIP_U32bit *pCount;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_getNameValuePairCountFromMinSEHdr \n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pMinSE == SIP_NULL) || ( pCount == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pMinSE->dType != SipHdrTypeMinSE )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pMinSE->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(((SipMinSEHeader*)(pMinSE->pHeader))->slNameValue),\
		pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_getNameValuePairCountFromMinSEHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_getNameValuePairAtIndexFromMinSEHdr
**
** DESCRIPTION: This function retrieves NameValuePair at specified index
**		from MinSE Header
**
*******************************************************************/

SipBool sip_getNameValuePairAtIndexFromMinSEHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader *pMinSE, SipNameValuePair **ppNameValue,\
	 SIP_U32bit index, SipError *pErr )
#else
	( SipHeader *pMinSE, SipNameValuePair *pNameValue,\
	 SIP_U32bit index, SipError *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMinSE,ppNameValue,index,pErr)
	  SipHeader *pMinSE;
	  SipNameValuePair **ppNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#else
	( pMinSE,pNameValue,index,pErr)
	  SipHeader *pMinSE;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list=SIP_NULL;
	SIPDEBUGFN("Entering sip_getNameValuePairAtIndexFromMinSEHdr\n");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

#ifdef SIP_BY_REFERENCE
	if (( pMinSE == SIP_NULL) || ( ppNameValue == SIP_NULL ))
#else
	if (( pMinSE == SIP_NULL) || ( pNameValue == SIP_NULL ))
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pMinSE->dType != SipHdrTypeMinSE )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pMinSE->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(((SipMinSEHeader*)(pMinSE->pHeader))->slNameValue), \
		index,&element_from_list, pErr) == SipFail)
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

	SIPDEBUGFN("Exiting sip_getNameValuePairAtIndexFromMinSEHdr\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_setNameValuePairAtIndexInMinSEHdr
**
** DESCRIPTION: This function sets NameValuePair at specified index
**		in MinSE Header
**
**
*******************************************************************/
SipBool sip_setNameValuePairAtIndexInMinSEHdr
#ifdef ANSI_PROTO
	( SipHeader *pMinSE, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pMinSE,pNameValue,dIndex,pErr)
	  SipHeader *pMinSE;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_setNameValuePairAtIndexInMinSEHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pMinSE == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pMinSE->dType != SipHdrTypeMinSE )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pMinSE->pHeader == SIP_NULL )
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

	if( sip_listSetAt( &(((SipMinSEHeader*)(pMinSE->pHeader))->slNameValue), \
		dIndex,(SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_setNameValuePairAtIndexInMinSEHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_insertNameValuePairAtIndexInMinSEHdr
**
** DESCRIPTION: This function inserts NameValuePair at specified index
**		in MinSE Header
**
**
*******************************************************************/
SipBool sip_insertNameValuePairAtIndexInMinSEHdr
#ifdef ANSI_PROTO
	( SipHeader *pMinSE, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pMinSE,pNameValue,dIndex,pErr)
	  SipHeader *pMinSE;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_insertNameValuePairAtIndexInMinSEHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pMinSE == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pMinSE->dType != SipHdrTypeMinSE )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pMinSE->pHeader == SIP_NULL )
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

	if( sip_listInsertAt( &(((SipMinSEHeader*)(pMinSE->pHeader))->\
		slNameValue), dIndex,(SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_insertNameValuePairAtIndexInMinSEHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sip_deleteNameValuePairAtIndexInMinSEHdr
**
** DESCRIPTION: This function deletes NameValuePair at specified index
**		in MinSE Header
**
**
**********************************************************************/

SipBool sip_deleteNameValuePairAtIndexInMinSEHdr
#ifdef ANSI_PROTO
	( SipHeader *pMinSE, SIP_U32bit dIndex, SipError *pErr )
#else
	( pMinSE,dIndex,pErr)
	  SipHeader *pMinSE;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering sip_deleteNameValuePairAtIndexInMinSEHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMinSE == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pMinSE->dType != SipHdrTypeMinSE )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pMinSE->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipMinSEHeader*)(pMinSE->pHeader))->\
		slNameValue), dIndex, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting sip_deleteNameValuePairAtIndexInMinSEHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

#endif
/********************************************************************
** FUNCTION:sip_getNameFromNameValuePair
**
** DESCRIPTION: This Function gets the Name from NamevaluePair
**
**
**********************************************************************/
SipBool sip_getNameFromNameValuePair
#ifdef ANSI_PROTO
	( SipNameValuePair *pNameValue, SIP_S8bit **pName, SipError *err)
#else
	( pNameValue, pName, err )
	  SipNameValuePair *pNameValue;
	  SIP_S8bit **pName;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *temp_name;
	SIPDEBUGFN("Entering sip_getNameFromNameValuePair\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( pNameValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_name = pNameValue->pName;

	if( temp_name == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_name);
	*pName = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pName == SIP_NULL )
		return SipFail;

	strcpy( *pName , temp_name );
#else
	*pName = temp_name;
#endif

	SIPDEBUGFN("Exiting sip_getNameFromNameValuePair\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sip_setNameInNameValuePair
**
** DESCRIPTION: This Function sets the Name in NamevaluePair
**
**
**********************************************************************/

SipBool sip_setNameInNameValuePair
#ifdef ANSI_PROTO
	( SipNameValuePair *pNameValue, SIP_S8bit *pName, SipError *err)
#else
	( pNameValue, pName, err )
	  SipNameValuePair *pNameValue;
	  SIP_S8bit *pName;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *temp_name;
	SIPDEBUGFN("Entering sip_setNameInNameValuePair\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pNameValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pName == SIP_NULL)
		temp_name = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pName );
		temp_name = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_name== SIP_NULL )
			return SipFail;

		strcpy( temp_name, pName );
#else
		temp_name = pName;
#endif

	}

	if ( pNameValue->pName != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, pNameValue->pName, err) == SipFail)
			return SipFail;
	}
        pNameValue->pName = temp_name;
	SIPDEBUGFN("Exiting sip_setNameInNameValuePair\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sip_getValueFromNameValuePair
**
** DESCRIPTION: This Function gets the value from NamevaluePair
**
**
**********************************************************************/
SipBool sip_getValueFromNameValuePair
#ifdef ANSI_PROTO
	( SipNameValuePair *pNameValue, SIP_S8bit **pValue, SipError *err)
#else
	( pNameValue, pValue, err )
	  SipNameValuePair *pNameValue;
	  SIP_S8bit **pValue;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering sip_getValueFromNameValuePair\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( pNameValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_value = pNameValue->pValue;

	if( temp_value == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_value);
	*pValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pValue == SIP_NULL )
		return SipFail;

	strcpy( *pValue , temp_value );
#else
	*pValue = temp_value;
#endif

	SIPDEBUGFN("Exiting sip_getValueFromNameValuePair\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sip_setValueInNameValuePair
**
** DESCRIPTION: This Function sets the value in NamevaluePair
**
**
**********************************************************************/

SipBool sip_setValueInNameValuePair
#ifdef ANSI_PROTO
	( SipNameValuePair *pNameValue, SIP_S8bit *pValue, SipError *err)
#else
	( pNameValue, pValue, err )
	  SipNameValuePair *pNameValue;
	  SIP_S8bit *pValue;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *temp_value;
	SIPDEBUGFN("Entering sip_setValueInNameValuePair\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pNameValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pValue == SIP_NULL)
		temp_value = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pValue );
		temp_value = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_value== SIP_NULL )
			return SipFail;

		strcpy( temp_value, pValue );
#else
		temp_value = pValue;
#endif

	}

	if ( pNameValue->pValue != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, pNameValue->pValue, err) == SipFail)
			return SipFail;
	}
        pNameValue->pValue = temp_value;
	SIPDEBUGFN("Exiting sip_setValueInNameValuePair\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromReplyToHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP ReplyTo pHeader
**
**************************************************************/
SipBool sip_getParamCountFromReplyToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit *count, SipError *err)
#else
	(hdr, count, err)
	SipHeader *hdr;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromReplyToHdr");
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
	if( hdr->dType != SipHdrTypeReplyTo)
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
	if (sip_listSizeOf( &(((SipReplyToHeader *)(hdr->pHeader))->slParams), count , err) == SipFail )
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromReplyToHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromReplyToHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP ReplyTo pHeader
**
***************************************************************/
SipBool sip_getParamAtIndexFromReplyToHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromReplyToHdr");
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
	if( hdr->dType != SipHdrTypeReplyTo)
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

	if (sip_listGetAt( &(((SipReplyToHeader *)(hdr->pHeader))->slParams), index,  \
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromReplyToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInReplyToHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp ReplyTo pHeader
**
***************************************************************/
SipBool sip_insertParamAtIndexInReplyToHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInReplyToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplyTo)
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

	if( sip_listInsertAt( &(((SipReplyToHeader *)(hdr->pHeader))->slParams),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInReplyToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInReplyToHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP ReplyTo pHeader
**
***************************************************************/
SipBool sip_deleteParamAtIndexInReplyToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(hdr, index, err)
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInReplyToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplyTo)
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
	if( sip_listDeleteAt( &(((SipReplyToHeader *)(hdr->pHeader))->slParams), index, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInReplyToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInReplyToHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP ReplyTo pHeader
**
***************************************************************/
SipBool sip_setParamAtIndexInReplyToHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInReplyToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplyTo)
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

	if( sip_listSetAt( &(((SipReplyToHeader *)(hdr->pHeader))->slParams),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInReplyToHdr");
	return SipSuccess;
}



/*****************************************************************
**
** FUNCTION:  sip_getDispNameFromReplyToHdr
**
** DESCRIPTION:This function retrieves the display-pName field from
**		a SIP Reply-To pHeader
**
***************************************************************/
SipBool sip_getDispNameFromReplyToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit **pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit **pDispName;
	SipError *err;
#endif
{
 	SIP_S8bit *temp_disp_name;
	SIPDEBUGFN("Entering function sip_getDispNameFromReplyToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( pDispName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplyTo)
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
 	temp_disp_name = ((SipReplyToHeader *)(hdr ->pHeader))->pDispName;
 	if (temp_disp_name == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDispName = (SIP_S8bit *)STRDUPACCESSOR(temp_disp_name);
	if (*pDispName == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pDispName = temp_disp_name;
#endif
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromReplyToHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInReplyToHdr
**
** DESCRIPTION: This function sets the display-pName field in a SIP
**		Reply-To pHeader
**
***************************************************************/
SipBool sip_setDispNameInReplyToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SIP_S8bit *pDispName, SipError *err)
#else
	(hdr, pDispName, err)
	SipHeader *hdr;
	SIP_S8bit *pDispName;
	SipError *err;
#endif
{
	SIP_S8bit *temp_disp_name, *dname;
	SIPDEBUGFN("Entering function sip_setDispNameInReplyToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplyTo)
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
        if( pDispName == SIP_NULL)
                temp_disp_name = SIP_NULL;
        else
        {
#ifndef SIP_BY_REFERENCE
	        temp_disp_name = (SIP_S8bit *)STRDUPACCESSOR (pDispName);
		if (temp_disp_name == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		temp_disp_name = pDispName;
#endif
	}

        dname = ((SipReplyToHeader *)(hdr->pHeader))->pDispName;
        if (dname != SIP_NULL)
        {
        	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&dname), err) == SipFail)
        		return SipFail;
        }

	((SipReplyToHeader *)(hdr->pHeader))->pDispName = temp_disp_name;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInReplyToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromReplyToHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP Reply-To pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromReplyToHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(SipHeader *hdr, SipAddrSpec **ppAddrSpec, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#else
	(hdr, ppAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec **ppAddrSpec;
	SipError *err;
#endif
#endif
{
	SipAddrSpec *temp_addr_spec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromReplyToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pAddrSpec == SIP_NULL)
#else
	if(  ppAddrSpec == SIP_NULL)
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
	if( hdr->dType != SipHdrTypeReplyTo)
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

 	temp_addr_spec = ((SipReplyToHeader *) (hdr ->pHeader))->pAddrSpec;
 	if (temp_addr_spec == SIP_NULL)
 	{
 		*err = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	if (__sip_cloneSipAddrSpec(pAddrSpec, temp_addr_spec, err) == SipFail)
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
		pAddrSpec->dType = SipAddrAny;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(temp_addr_spec->dRefCount);
	*ppAddrSpec = temp_addr_spec;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromReplyToHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInReplyToHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		Reply-To pHeader
**
***************************************************************/
SipBool sip_setAddrSpecInReplyToHdr
#ifdef ANSI_PROTO
	(SipHeader *hdr, SipAddrSpec *pAddrSpec, SipError *err)
#else
	(hdr, pAddrSpec, err)
	SipHeader *hdr;
	SipAddrSpec *pAddrSpec;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAddrSpec *temp_addr_spec;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInReplyToHdr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL)
		return SipFail;
	if( hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if( hdr->dType != SipHdrTypeReplyTo)
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
  	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipReplyToHeader *) (hdr ->pHeader))->pAddrSpec);
 		((SipReplyToHeader *) (hdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipAddrSpec(&temp_addr_spec, pAddrSpec->dType, err ) == SipFail)
			return SipFail;

		if (__sip_cloneSipAddrSpec(temp_addr_spec, pAddrSpec, err) == SipFail)
		{
			sip_freeSipAddrSpec(temp_addr_spec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipReplyToHeader *) (hdr ->pHeader))->pAddrSpec);
 		((SipReplyToHeader *) (hdr ->pHeader))->pAddrSpec = temp_addr_spec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipReplyToHeader *) (hdr ->pHeader))->pAddrSpec);
		((SipReplyToHeader *) (hdr ->pHeader))->pAddrSpec = pAddrSpec;
#endif
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInReplyToHdr");
	return SipSuccess;
}


#ifdef SIP_PRIVACY
/*****************************************************************
**
** FUNCTION:  sip_getDisplayNameFromPAssertIdHdr
**
** DESCRIPTION: This function retrieves the Display Name field 
**         from a SIP P-Asserted-Id pHeader
**
***************************************************************/
SipBool sip_getDisplayNameFromPAssertIdHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **pDisplayName, SipError *pErr)
#else
	(pHdr, pDisplayName, pErr)
	SipHeader *pHdr;
	SIP_S8bit **pDisplayName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempDisplayName = NULL ;
	SIPDEBUGFN("Entering function sip_getDisplayNameFromPAssertIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pDisplayName == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePAssertId)
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
 	pTempDisplayName = ((SipPAssertIdHeader *) (pHdr ->pHeader))->pDispName;
 	if (pTempDisplayName == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDisplayName = (SIP_S8bit *)STRDUPACCESSOR(pTempDisplayName);
	if (*pDisplayName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*pDisplayName = pTempDisplayName ;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDisplayNameFromPAssertIdHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromPAssertIdHdr
**
** DESCRIPTION: This function retrieves the AddrSpec field from a
**		SIP PAssertId pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromPAssertIdHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#else
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#endif
#endif
{
 	SipAddrSpec *pTempAddrSpec=SIP_NULL ;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromPAssertIdHdr");
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
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePAssertId )
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

 	pTempAddrSpec = ((SipPAssertIdHeader *) (pHdr ->pHeader))->pAddrSpec;
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
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromPAssertIdHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_setDisplayNameInPAssertIdHdr
**
** DESCRIPTION: This function sets the display-name field in a SIP
**		PAssertedId pHeader
**
***************************************************************/
SipBool sip_setDisplayNameInPAssertIdHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pDisplayName, SipError *pErr)
#else
	(pHdr, pDisplayName, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pDisplayName;
	SipError *pErr;
#endif
{
        SIP_S8bit *pTempDisplayName = NULL , *pDName = NULL ;
	SIPDEBUGFN("Entering function sip_setDisplayNameInPAssertIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePAssertId)
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
    pDName = ((SipPAssertIdHeader *)(pHdr->pHeader))->pDispName;
    if (pDName != SIP_NULL)
    {
    	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pDName), pErr) \
			== SipFail)
		{
       		return SipFail;
		}
    }

	if( pDisplayName == SIP_NULL)
		pTempDisplayName = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
        pTempDisplayName = (SIP_S8bit *)STRDUPACCESSOR(pDisplayName);
	if (pTempDisplayName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempDisplayName = pDisplayName;
#endif
	}


	((SipPAssertIdHeader *)(pHdr->pHeader))->pDispName = pTempDisplayName;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDisplayNameInPAssertIdHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInPAssertIdHdr
**
** DESCRIPTION: This function sets the pAssertId header field 
**             in a SIP PAssertId Header.
**
***************************************************************/
SipBool sip_setAddrSpecInPAssertIdHdr
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
 	SipAddrSpec *pTempAddrSpec = NULL ;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInPAssertIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pHdr->dType != SipHdrTypePAssertId )
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
		sip_freeSipAddrSpec(((SipPAssertIdHeader *)(pHdr->pHeader))->pAddrSpec);
 		((SipPAssertIdHeader *) (pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType , pErr )\
				== SipFail)
			return SipFail;
 		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			return SipFail;
       		}
		sip_freeSipAddrSpec(((SipPAssertIdHeader *)(pHdr->pHeader))->pAddrSpec);
		((SipPAssertIdHeader *)(pHdr ->pHeader))->pAddrSpec = pTempAddrSpec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipPAssertIdHeader *)(pHdr->pHeader))->pAddrSpec);
		((SipPAssertIdHeader *)(pHdr->pHeader))->pAddrSpec = pAddrSpec;
#endif
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInPAssertIdHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getDisplayNameFromPPreferredIdHdr
**
** DESCRIPTION: This function retrieves the Display Name field 
**         from a SIP P-Asserted-Id pHeader
**
***************************************************************/
SipBool sip_getDisplayNameFromPPreferredIdHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **pDisplayName, SipError *pErr)
#else
	(pHdr, pDisplayName, pErr)
	SipHeader *pHdr;
	SIP_S8bit **pDisplayName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempDisplayName = NULL ;
	SIPDEBUGFN("Entering function sip_getDisplayNameFromPPreferredIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pDisplayName == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePPreferredId)
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
 	pTempDisplayName = ((SipPPreferredIdHeader *) (pHdr ->pHeader))->pDispName;
 	if (pTempDisplayName == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDisplayName = (SIP_S8bit *)STRDUPACCESSOR(pTempDisplayName);
	if (*pDisplayName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*pDisplayName = pTempDisplayName ;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDisplayNameFromPPreferredIdHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromPPreferredIdHdr
**
** DESCRIPTION: This function retrieves the AddrSpec field from a
**		SIP PPreferredIdHdr pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromPPreferredIdHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#else
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#endif
#endif
{
 	SipAddrSpec *pTempAddrSpec=SIP_NULL ;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromPPreferredIdHdr");
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
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePPreferredId )
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

 	pTempAddrSpec = ((SipPPreferredIdHeader *) (pHdr ->pHeader))->pAddrSpec;
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
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromPPreferredIdHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_setDisplayNameInPPreferredIdHdr
**
** DESCRIPTION: This function sets the display-name field in a SIP
**		PPreferredIdHdr pHeader
**
***************************************************************/
SipBool sip_setDisplayNameInPPreferredIdHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pDisplayName, SipError *pErr)
#else
	(pHdr, pDisplayName, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pDisplayName;
	SipError *pErr;
#endif
{
    SIP_S8bit *pTempDisplayName = NULL , *pDName = NULL ;
	SIPDEBUGFN("Entering function sip_setDisplayNameInPPreferredIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePPreferredId)
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
	pDName = ((SipPPreferredIdHeader *)(pHdr->pHeader))->pDispName;
    if (pDName != SIP_NULL)
    {
    	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pDName), pErr) \
					== SipFail)
		{
       		return SipFail;
		}
    }

    if( pDisplayName == SIP_NULL)
    	pTempDisplayName = SIP_NULL;
    else
    {
#ifndef SIP_BY_REFERENCE
	    pTempDisplayName = (SIP_S8bit *)STRDUPACCESSOR(pDisplayName);
		if (pTempDisplayName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempDisplayName = pDisplayName;
#endif
	}


	((SipPPreferredIdHeader *)(pHdr->pHeader))->pDispName = pTempDisplayName;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDisplayNameInPPreferredIdHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInPPreferredIdHdr
**
** DESCRIPTION: This function sets the pAssertId header field 
**             in a SIP PPreferredIdHdr Header.
**
***************************************************************/
SipBool sip_setAddrSpecInPPreferredIdHdr
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
 	SipAddrSpec *pTempAddrSpec = NULL ;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInPPreferredIdHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pHdr->dType != SipHdrTypePPreferredId )
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
		sip_freeSipAddrSpec(((SipPPreferredIdHeader *)(pHdr->pHeader))->pAddrSpec);
 		((SipPPreferredIdHeader *) (pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType , pErr )\
				== SipFail)
			return SipFail;
 		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			return SipFail;
       		}
		sip_freeSipAddrSpec(((SipPPreferredIdHeader *)(pHdr->pHeader))->pAddrSpec);
		((SipPPreferredIdHeader *)(pHdr ->pHeader))->pAddrSpec = pTempAddrSpec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipPPreferredIdHeader *)(pHdr->pHeader))->pAddrSpec);
		((SipPPreferredIdHeader *)(pHdr->pHeader))->pAddrSpec = pAddrSpec;
#endif
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInPPreferredIdHdr");
	return SipSuccess;
}
/********************************************************************
** FUNCTION:sip_getNameValuePairCountFromPrivacyHdr
**
** DESCRIPTION: This function retrieves the number of NameValuePairs from an
**		Privacy Header
**
**
**********************************************************************/
SipBool sip_getNameValuePairCountFromPrivacyHdr
#ifdef ANSI_PROTO
	( SipHeader *pPrivacy, SIP_U32bit *pCount, SipError *pErr )
#else
	( pPrivacy,pCount,pErr)
	  SipHeader *pPrivacy;
	  SIP_U32bit *pCount;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sip_getNameValuePairCountFromPrivacyHdr \n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if (( pPrivacy == SIP_NULL) || ( pCount == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pPrivacy->dType != SipHdrTypePrivacy )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pPrivacy->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(((SipPrivacyHeader*)(pPrivacy->pHeader))->slPrivacyValue),\
		pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_getNameValuePairCountFromPrivacyHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sip_getNameValuePairAtIndexFromPrivacyHdr
**
** DESCRIPTION: This function retrieves NameValuePair at specified index
**		from Privacy Header
**
*******************************************************************/

SipBool sip_getNameValuePairAtIndexFromPrivacyHdr
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SipHeader *pPrivacy, SipNameValuePair **ppNameValue,\
	 SIP_U32bit index, SipError *pErr )
#else
	( SipHeader *pPrivacy, SipNameValuePair *pNameValue,\
	 SIP_U32bit index, SipError *pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pPrivacy,ppNameValue,index,pErr)
	  SipHeader *pPrivacy;
	  SipNameValuePair **ppNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#else
	( pPrivacy,pNameValue,index,pErr)
	  SipHeader *pPrivacy;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit index;
	  SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list=SIP_NULL;
	SipNameValuePair *temp_privacy_param;
	SIPDEBUGFN("Entering sip_getNameValuePairAtIndexFromPrivacyHdr\n");

#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

#ifdef SIP_BY_REFERENCE
	if (( pPrivacy == SIP_NULL) || ( ppNameValue == SIP_NULL ))
#else
	if (( pPrivacy == SIP_NULL) || ( pNameValue == SIP_NULL ))
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pPrivacy->dType != SipHdrTypePrivacy )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pPrivacy->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(((SipPrivacyHeader*)(pPrivacy->pHeader))->slPrivacyValue), \
		index,&element_from_list, pErr) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
	temp_privacy_param = (SipNameValuePair *)element_from_list;

#ifndef SIP_BY_REFERENCE
	if( __sip_cloneSipNameValuePair(pNameValue, \
		temp_privacy_param ,pErr) == SipFail )
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(temp_privacy_param->dRefCount);
	*ppNameValue = temp_privacy_param;
#endif

	SIPDEBUGFN("Exiting sip_getNameValuePairAtIndexFromPrivacyHdr\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sip_insertNameValuePairAtIndexInPrivacyHdr
**
** DESCRIPTION: This function inserts NameValuePair at specified index
**		in Privacy Header
**
**
*******************************************************************/
SipBool sip_insertNameValuePairAtIndexInPrivacyHdr
#ifdef ANSI_PROTO
	( SipHeader *pPrivacy, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pPrivacy,pNameValue,dIndex,pErr)
	  SipHeader *pPrivacy;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_insertNameValuePairAtIndexInPrivacyHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pPrivacy == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pPrivacy->dType != SipHdrTypePrivacy )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pPrivacy->pHeader == SIP_NULL )
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

	if( sip_listInsertAt( &(((SipPrivacyHeader*)(pPrivacy->pHeader))->\
		slPrivacyValue), dIndex,(SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_insertNameValuePairAtIndexInPrivacyHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_setNameValuePairAtIndexInPrivacyHdr
**
** DESCRIPTION: This function sets a Value at a specified index in a
**		SIP Privacy pHeader
**
***************************************************************/
SipBool sip_setNameValuePairAtIndexInPrivacyHdr
#ifdef ANSI_PROTO
	( SipHeader *pPrivacy, SipNameValuePair *pNameValue,\
	 SIP_U32bit dIndex, SipError *pErr )
#else
	( pPrivacy,pNameValue,dIndex,pErr)
	  SipHeader *pPrivacy;
	  SipNameValuePair *pNameValue;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SipNameValuePair* element_in_list=SIP_NULL;

	SIPDEBUGFN("Entering sip_setNameValuePairAtIndexInPrivacyHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pPrivacy == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pPrivacy->dType != SipHdrTypePrivacy )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pPrivacy->pHeader == SIP_NULL )
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

	if( sip_listSetAt( &(((SipPrivacyHeader*)(pPrivacy->pHeader))->slPrivacyValue), \
		dIndex,(SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		__sip_freeSipNameValuePair(element_in_list);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting sip_setNameValuePairAtIndexInPrivacyHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
** FUNCTION:sip_deleteNameValuePairAtIndexInPrivacyHdr
**
** DESCRIPTION: This function deletes NameValuePair at specified index
**		in Privacy Header
**
**
**********************************************************************/

SipBool sip_deleteNameValuePairAtIndexInPrivacyHdr
#ifdef ANSI_PROTO
	( SipHeader *pPrivacy, SIP_U32bit dIndex, SipError *pErr )
#else
	( pPrivacy,dIndex,pErr)
	  SipHeader *pPrivacy;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering sip_deleteNameValuePairAtIndexInPrivacyHdr\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pPrivacy == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pPrivacy->dType != SipHdrTypePrivacy )
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if ( pPrivacy->pHeader == SIP_NULL )
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipPrivacyHeader*)(pPrivacy->pHeader))->\
		slPrivacyValue), dIndex, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting sip_deleteNameValuePairAtIndexInPrivacyHdr\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
#endif

#ifdef SIP_3GPP
/*****************************************************************
**
** FUNCTION:  sip_getDispNameFromPathHdr
**
** DESCRIPTION:This function retrieves the Display Name field from
**		a SIP Path pHeader
**
***************************************************************/
SipBool sip_getDispNameFromPathHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **pDispName, SipError *pErr)
#else
	(pHdr, pDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit **pDispName;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempDispName=SIP_NULL;
	SIPDEBUGFN("Entering function sip_getDispNameFromPathHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pDispName == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePath)
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
 	pTempDispName = ((SipPathHeader *)(pHdr ->pHeader))->pDispName;
 	if (pTempDispName == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDispName = (SIP_S8bit *)STRDUPACCESSOR(pTempDispName);
	if (*pDispName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*pDispName = pTempDispName;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromPathHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInPathHdr
**
** DESCRIPTION: This function sets the Display-Name field in a SIP
**		Path pHeader
**
***************************************************************/
SipBool sip_setDispNameInPathHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pDispName, SipError *pErr)
#else
	(pHdr, pDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pDispName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempDispName=SIP_NULL;
	SIP_S8bit *pName=SIP_NULL;
	SIPDEBUGFN("Entering function sip_setDispNameInPathHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePath)
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
		pTempDispName = (SIP_S8bit *)STRDUPACCESSOR (pDispName);
		if (pTempDispName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		pTempDispName = pDispName;
#endif
	}

    pName = ((SipPathHeader *)(pHdr->pHeader))->pDispName;
    if (pName != SIP_NULL)
    {
    	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pName), pErr) \
    		== SipFail)
    	return SipFail;
    }

	((SipPathHeader *)(pHdr->pHeader))->pDispName = pTempDispName;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInPathHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromPathHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP Path pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromPathHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#else
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#endif
#endif
{
	SipAddrSpec *temp_addr_spec;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromPathHdr");
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
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePath)
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

 	temp_addr_spec = ((SipPathHeader *) (pHdr ->pHeader))->pAddrSpec;
 	if (temp_addr_spec == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	if (__sip_cloneSipAddrSpec(pAddrSpec, temp_addr_spec, pErr) == SipFail)
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
		pAddrSpec->dType = SipAddrAny;
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(temp_addr_spec->dRefCount);
	*ppAddrSpec = temp_addr_spec;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromPathHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInPathHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		Path pHeader
**
***************************************************************/
SipBool sip_setAddrSpecInPathHdr
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
	SipAddrSpec *temp_addr_spec;
#endif
	SIPDEBUGFN("Entering function sip_setAddrSpecInPathHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePath)
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
		sip_freeSipAddrSpec(((SipPathHeader *) (pHdr ->pHeader))->pAddrSpec);
 		((SipPathHeader *) (pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipAddrSpec(&temp_addr_spec, pAddrSpec->dType, pErr ) \
			== SipFail)
			return SipFail;

		if (__sip_cloneSipAddrSpec(temp_addr_spec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(temp_addr_spec);
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipPathHeader *) (pHdr ->pHeader))->pAddrSpec);
 		((SipPathHeader *) (pHdr ->pHeader))->pAddrSpec = temp_addr_spec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipPathHeader *) (pHdr ->pHeader))->pAddrSpec);
		((SipPathHeader *) (pHdr ->pHeader))->pAddrSpec = pAddrSpec;
#endif
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInPathHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromPathHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		Path pHeader
**
***************************************************************/

SipBool sip_getParamCountFromPathHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromPathHdr");
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
	if( pHdr->dType != SipHdrTypePath)
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
	if (sip_listSizeOf( &(((SipPathHeader *)(pHdr->pHeader))->slParams), \
		pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromPathHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromPathHdr
**
** DESCRIPTION: This function gets the param field at a given index
**				 in a SIP
**		Path pHeader
**
***************************************************************/

SipBool sip_getParamAtIndexFromPathHdr
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
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPathHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pParam == SIP_NULL)
#else
	if(  ppParam == SIP_NULL)
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
	if( pHdr->dType != SipHdrTypePath)
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

	if (sip_listGetAt( &(((SipPathHeader *)(pHdr->pHeader))->slParams), \
		dIndex, &element_from_list, pErr) == SipFail)
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPathHdr");
	return SipSuccess;
}



/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInPathHdr
**
** DESCRIPTION: This function sets a param at a specified index
**		in a Path Header
**
***************************************************************/

SipBool sip_setParamAtIndexInPathHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPathHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePath)
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

	if( sip_listSetAt( &(((SipPathHeader *)(pHdr->pHeader))->slParams),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInPathHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInPathHdr
**
** DESCRIPTION: This function inserts a param at a specified index
**		in a Path Header
**
***************************************************************/
SipBool sip_insertParamAtIndexInPathHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPathHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePath)
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

	if( sip_listInsertAt( &(((SipPathHeader *)(pHdr->pHeader))->slParams), \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPathHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInPathHdr
**
** DESCRIPTION: This function deletes a param at a specified index
**		in a Path Header
**
***************************************************************/
SipBool sip_deleteParamAtIndexInPathHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPathHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePath)
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
	if( sip_listDeleteAt( &(((SipPathHeader *)(pHdr->pHeader))->slParams), \
		dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPathHdr");
	return SipSuccess;
}
/********************************************************************
 ** FUNCTION: sip_getAccessTypeFromPanInfoHdr
 **
 ** DESCRIPTION: This function retrieves the Access Type field from
 ** the P-Access-Network-Info pHeader.
 **
 ****************************************************************/
SipBool sip_getAccessTypeFromPanInfoHdr
#ifdef ANSI_PROTO
    (SipHeader *pHdr, SIP_S8bit **ppAccessType, SipError *pErr)
#else
	(pHdr,ppAccessType,pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppAccessType;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_pAccessType;
	SIPDEBUGFN("Entering function sip_getpAccessTypeHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppAccessType == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePanInfo)
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

	pTemp_pAccessType = ((SipPanInfoHeader *)(pHdr->pHeader))->pAccessType;
	if(pTemp_pAccessType == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	*ppAccessType = (SIP_S8bit *) STRDUPACCESSOR (pTemp_pAccessType);
	if (*ppAccessType == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppAccessType = pTemp_pAccessType;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting Function sip_getpAccessTypeFromPanInfoHdr");
	return SipSuccess;
}

/***********************************************************************
 **
 ** FUNCTION: sip_setAccessTypeInPanInfoHdr
 **
 ** DESCRIPTION: This function sets the Access type field in
 **              a SIP P-Access-Network-Info pHeader
 **
 *********************************************************************/
SipBool sip_setAccessTypeInPanInfoHdr
#ifdef ANSI_PROTO
    (SipHeader *pHdr, SIP_S8bit *pAccessType, SipError *pErr)
#else
	(pHdr, pAccessType, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pAccessType;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_pAccessType,*dAccess;
	SIPDEBUGFN("Entering Function sip_setpAccessTypeInPanInfoHdr");
#ifndef	SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{	
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if(pHdr->dType != SipHdrTypePanInfo)
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
	if( pAccessType == SIP_NULL)
		pTemp_pAccessType = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		pTemp_pAccessType = (SIP_S8bit *)STRDUPACCESSOR(pAccessType);
	if (pTemp_pAccessType == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	  pTemp_pAccessType = pAccessType;
#endif
	}

	   dAccess = ((SipPanInfoHeader *)(pHdr->pHeader))->pAccessType;
	   if (dAccess!=SIP_NULL)
		{
			if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)(&dAccess), pErr)==\
				SipFail)
			{
				sip_freeString(pTemp_pAccessType);
				return SipFail;
			}
		}
	((SipPanInfoHeader *)(pHdr->pHeader))->pAccessType = pTemp_pAccessType;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting Function sip_setpAccessTypeInPanInfoHdr");
	return SipSuccess;
}

/*****************************************************************
 **
 ** FUNCTION: sip_getParamCountFromPanInfoHdr
 **
 ** DESCRIPTION: This function retrieves the number of parameters
 **              from a SIP P-Access-Network-Info pHeader
 **
 *****************************************************************/
SipBool sip_getParamCountFromPanInfoHdr
#ifdef ANSI_PROTO
       (SipHeader *pHdr, SIP_U32bit *pCount,SipError *pErr)
#else
	   (pHdr,pCount,pErr)
	   SipHeader *pHdr;
	   SIP_U32bit *pCount;
	   SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering Function sip_getParamCountFromPanInfoHdr");
#ifndef SIP_NO_CHECK	
	 if( pErr == SIP_NULL)
		 return SipFail;
	 if( pHdr == SIP_NULL )
	 {
		 *pErr = E_INV_PARAM;
		 return SipFail;
	 }
	 if(pHdr->dType!=SipHdrTypePanInfo)
	 {
		 *pErr = E_INV_TYPE;
		 return SipFail;
	 }
	 if(pHdr->pHeader == SIP_NULL)
	 {
		 *pErr = E_INV_HEADER;
		 return SipFail;
	 }
	 if(pCount == SIP_NULL)
	 {
		 *pErr = E_INV_PARAM;
		 return SipFail;
	 }
#endif
	 
	 if(sip_listSizeOf(&(((SipPanInfoHeader *)(pHdr->pHeader))->slParams),\
		 pCount,pErr) == SipFail)
	 {
		 return SipFail;
	 }

     *pErr = E_NO_ERROR;
	 SIPDEBUGFN("Exitting Function sip_getParamCountFromPanInfoHdr");
	 return SipSuccess;
}

/***********************************************************************
 **
 ** FUNCTION: sip_getParamAtIndexFromPanInfoHdr
 **
 ** DESCRIPTION: This function gets the the param field at a given index
                 in a SIP P-Access-Network-Info pHeader
 **
 **********************************************************************/
 
SipBool sip_getParamAtIndexFromPanInfoHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
    (SipHeader *pHdr, SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipNameValuePair **ppParam, SIP_U32bit dIndex, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipNameValuePair *pTemp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPanInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pParam == SIP_NULL)
#else
	if( ppParam	== SIP_NULL)
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
	if((pHdr->dType != SipHdrTypePanInfo))

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

	if (sip_listGetAt( &(((SipPanInfoHeader *)(pHdr->pHeader))->slParams), \
		dIndex, &element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
   pTemp_param = (SipNameValuePair *)element_from_list;
#ifndef SIP_BY_REFERENCE
   if (__sip_cloneSipNameValuePair(pParam,pTemp_param,pErr) == SipFail)
	   return SipFail;
#else
	   HSS_LOCKEDINCREF(pTemp_param->dRefCount);
	   *ppParam = pTemp_param;
#endif

	   *pErr = E_NO_ERROR;
	   SIPDEBUGFN("Exitting function sip_getParamAtIndexFromPanInfoHdr");
	   return SipSuccess;
}	   

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInPanInfoHdr
**
** DESCRIPTION: This function sets a param at a specified index
**		in a P-Access-Network-Info Header
**
***************************************************************/

SipBool sip_setParamAtIndexInPanInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipNameValuePair *pTemp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPanInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePanInfo)
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
		pTemp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipNameValuePair (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipPanInfoHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setParamAtIndexInPanInfoHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInPanInfoHdr
**
** DESCRIPTION: This function inserts a param at a specified index
**		in a P-Access-Network-Info Header
**
***************************************************************/
SipBool sip_insertParamAtIndexInPanInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipNameValuePair *pTemp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPanInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePanInfo)
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
		pTemp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipNameValuePair (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipPanInfoHeader *)(pHdr->pHeader))->slParams), \
		dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_insertParamAtIndexInPanInfoHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInPanInfoHdr
**
** DESCRIPTION: This function deletes a param at a specified index
**		in a PanInfo Header
**
***************************************************************/
SipBool sip_deleteParamAtIndexInPanInfoHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPanInfoHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePanInfo)
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
	if( sip_listDeleteAt( &(((SipPanInfoHeader *)(pHdr->pHeader))->slParams), \
		dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_deleteParamAtIndexInPanInfoHdr");
	return SipSuccess;
}





/****************************************************************
 ** FUNCTION: sip_getAccessTypeFromPcVectorHdr
 **
 ** DESCRIPTION: This function retrieves the Access Type field from
 ** the P-Charging-Vector pHeader.
 **
 ****************************************************************/
SipBool sip_getAccessTypeFromPcVectorHdr
#ifdef ANSI_PROTO
    (SipHeader *pHdr, SIP_S8bit **ppAccessType, SipError *pErr)
#else
	(pHdr,ppAccessType,pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppAccessType;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_pAccessType;
	SIPDEBUGFN("Entering function sip_getAccessTypeFromPcVectorHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppAccessType == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePcVector)
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

	pTemp_pAccessType = ((SipPcVectorHeader *)(pHdr->pHeader))->pAccessType;
	if(pTemp_pAccessType == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	*ppAccessType = (SIP_S8bit *) STRDUPACCESSOR (pTemp_pAccessType);
	if (*ppAccessType == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppAccessType = pTemp_pAccessType;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting Function sip_getAccessTypeFromPcVectorHdr");
	return SipSuccess;
}

/***********************************************************************
 **
 ** FUNCTION: sip_setAccessTypeInPcVectorHdr
 **
 ** DESCRIPTION: This function sets the Access type field in
 **              a SIP P-Charging-Vector pHeader
 **
 *********************************************************************/
SipBool sip_setAccessTypeInPcVectorHdr
#ifdef ANSI_PROTO
    (SipHeader *pHdr, SIP_S8bit *pAccessType, SipError *pErr)
#else
	(pHdr, pAccessType, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pAccessType;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_pAccessType,*dAccess;
	SIPDEBUGFN("Entering Function sip_setAccessTypeInPcVectorHdr");
#ifndef	SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{	
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((pHdr->dType != SipHdrTypePcVector))
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
	if( pAccessType == SIP_NULL)
		pTemp_pAccessType = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		pTemp_pAccessType = (SIP_S8bit *)STRDUPACCESSOR(pAccessType);
	if (pTemp_pAccessType == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	  pTemp_pAccessType = pAccessType;
#endif
	}

	   dAccess = ((SipPcVectorHeader *)(pHdr->pHeader))->pAccessType;
	   if (dAccess!=SIP_NULL)
		{
			if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)(&dAccess), pErr)==\
				SipFail)
			{
				sip_freeString(pTemp_pAccessType);
				return SipFail;
			}
		}
	((SipPcVectorHeader *)(pHdr->pHeader))->pAccessType = pTemp_pAccessType;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting Function sip_setAccessTypeInPcVectorHdr");
	return SipSuccess;
}



/****************************************************************
 ** FUNCTION: sip_getAccessValueFromPcVectorHdr
 **
 ** DESCRIPTION: This function retrieves the Access Value field from
 ** the P-Charging-Vector pHeader.
 **
 ****************************************************************/
SipBool sip_getAccessValueFromPcVectorHdr
#ifdef ANSI_PROTO
    (SipHeader *pHdr, SIP_S8bit **ppAccessValue, SipError *pErr)
#else
	(pHdr,ppAccessValue,pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppAccessValue;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_pAccessValue;
	SIPDEBUGFN("Entering function sip_getAccessValueFromPcVectorHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppAccessValue == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePcVector)
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

	pTemp_pAccessValue = ((SipPcVectorHeader *)(pHdr->pHeader))->pAccessValue;
	if(pTemp_pAccessValue == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	*ppAccessValue = (SIP_S8bit *) STRDUPACCESSOR (pTemp_pAccessValue);
	if (*ppAccessValue == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppAccessValue = pTemp_pAccessValue;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting Function sip_getAccessValueFromPcVectorHdr");
	return SipSuccess;
}

/***********************************************************************
 **
 ** FUNCTION: sip_setAccessValueInPcVectorHdr
 **
 ** DESCRIPTION: This function sets the Access Value field in
 **              a SIP P-Charging-Vector pHeader
 **
 *********************************************************************/
SipBool sip_setAccessValueInPcVectorHdr
#ifdef ANSI_PROTO
    (SipHeader *pHdr, SIP_S8bit *pAccessValue, SipError *pErr)
#else
	(pHdr, pAccessValue, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pAccessValue;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp_pAccessValue,*dAccess;
	SIPDEBUGFN("Entering Function sip_setAccessValueInPcVectorHdr");
#ifndef	SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{	
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if((pHdr->dType != SipHdrTypePcVector))
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
	if( pAccessValue == SIP_NULL)
		pTemp_pAccessValue = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		pTemp_pAccessValue = (SIP_S8bit *)STRDUPACCESSOR(pAccessValue);
	if (pTemp_pAccessValue == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	  pTemp_pAccessValue = pAccessValue;
#endif
	}

	   dAccess = ((SipPcVectorHeader *)(pHdr->pHeader))->pAccessValue;
	   if (dAccess!=SIP_NULL)
		{
			if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)(&dAccess), pErr)==\
				SipFail)
			{
				sip_freeString(pTemp_pAccessValue);
				return SipFail;
			}
		}
	((SipPcVectorHeader *)(pHdr->pHeader))->pAccessValue = pTemp_pAccessValue;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting Function sip_setAccessValueInPcVectorHdr");
	return SipSuccess;
}




/*****************************************************************
 **
 ** FUNCTION: sip_getParamCountFromPcVectorHdr
 **
 ** DESCRIPTION: This function retrieves the number of parameters
 **              from a SIP P-Charging-Vector pHeader
 **
 *****************************************************************/
SipBool sip_getParamCountFromPcVectorHdr
#ifdef ANSI_PROTO
       (SipHeader *pHdr, SIP_U32bit *pCount,SipError *pErr)
#else
	   (pHdr,pCount,pErr)
	   SipHeader *pHdr;
	   SIP_U32bit *pCount;
	   SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering Function sip_getParamCountFromPcVectorHdr");
#ifndef SIP_NO_CHECK	
	 if( pErr == SIP_NULL)
		 return SipFail;
	 if( pHdr == SIP_NULL )
	 {
		 *pErr = E_INV_PARAM;
		 return SipFail;
	 }
	 if((pHdr->dType!=SipHdrTypePcVector))
	 {
		 *pErr = E_INV_TYPE;
		 return SipFail;
	 }
	 if(pCount == SIP_NULL)
	 {
		 *pErr = E_INV_PARAM;
		 return SipFail;
	 }
	 if((pHdr->pHeader==SIP_NULL))
	 {
		 *pErr = E_INV_HEADER;
		 return SipFail;
	 }

	 
#endif
	 if(sip_listSizeOf(&(((SipPcVectorHeader *)(pHdr->pHeader))->slParams)\
		 ,pCount,pErr) == SipFail)
	 {
		 return SipFail;
	 }
     *pErr = E_NO_ERROR;
	 SIPDEBUGFN("Exitting Function sip_getParamCountFromPcVectorHdr");
	 return SipSuccess;
}

/***********************************************************************
 **
 ** FUNCTION: sip_getParamAtIndexFromPcVectorHdr
 **
 ** DESCRIPTION: This function gets the param field at a given index
                 in a SIP P-Charging-Vector pHeader
 **
 **********************************************************************/
 
SipBool sip_getParamAtIndexFromPcVectorHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
    (SipHeader *pHdr, SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipNameValuePair **ppParam, SIP_U32bit dIndex, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pHdr, ppParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipNameValuePair *pTemp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPcVectorHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if( pParam == SIP_NULL)
#else
	if( ppParam	== SIP_NULL)
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
	if( pHdr->dType != SipHdrTypePcVector)
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

	if (sip_listGetAt( &(((SipPcVectorHeader *)(pHdr->pHeader))->slParams), \
		dIndex, &element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
   pTemp_param = (SipNameValuePair *)element_from_list;
#ifndef SIP_BY_REFERENCE
   if (__sip_cloneSipNameValuePair(pParam,pTemp_param,pErr) == SipFail)
	   return SipFail;
#else
	   HSS_LOCKEDINCREF(pTemp_param->dRefCount);
	   *ppParam = pTemp_param;
#endif

	   *pErr = E_NO_ERROR;
	   SIPDEBUGFN("Exitting function sip_getParamAtIndexFromPcVectorHdr");
	   return SipSuccess;
}	   

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInPcVectorHdr
**
** DESCRIPTION: This function sets a param at a specified index
**		in a P-Charging-Vector Header
**
***************************************************************/

SipBool sip_setParamAtIndexInPcVectorHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipNameValuePair *pTemp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPcVectorHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePcVector)
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
		pTemp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipNameValuePair (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipPcVectorHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setParamAtIndexInPcVectorHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInPcVectorHdr
**
** DESCRIPTION: This function inserts a param at a specified index
**		in a P-Charging-Vector Header
**
***************************************************************/
SipBool sip_insertParamAtIndexInPcVectorHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, pParam, dIndex, pErr)
	SipHeader *pHdr;
	SipNameValuePair *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipNameValuePair *pTemp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPcVectorHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePcVector)
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
		pTemp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipNameValuePair(&pTemp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipNameValuePair(pTemp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipNameValuePair (pTemp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTemp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipPcVectorHeader *)(pHdr->pHeader))->slParams), \
		dIndex, (SIP_Pvoid)(pTemp_param), pErr) == SipFail)
	{
		if (pTemp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipNameValuePair (pTemp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_insertParamAtIndexInPcVectorHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInPcVectorHdr
**
** DESCRIPTION: This function deletes a param at a specified index
**		in a PcVector Header
**
***************************************************************/
SipBool sip_deleteParamAtIndexInPcVectorHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPcVectorHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypePcVector)
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
	if( sip_listDeleteAt( &(((SipPcVectorHeader *)(pHdr->pHeader))->slParams), \
		dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_deleteParamAtIndexInPcVectorHdr");
	return SipSuccess;
}
#endif


/****************************************************************
**
** FUNCTION: sip_getDispTypeFromReasonHdr
**
** DESCRIPTION: This function retrives the display type field from 
** the Reason pHeader.
**
*****************************************************************/

SipBool sip_getDispTypeFromReasonHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit **ppDispType, SipError *pErr)
#else
	( pHdr, ppDispType, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit **ppDispType;
	  SipError *pErr;
#endif
{
	SIP_S8bit * pDispType;

	SIPDEBUGFN ( "Entering sip_getDispTypeFromReasonHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppDispType == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeReason)
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
	pDispType = ( (SipReasonHeader *) (pHdr->pHeader) )->pDispType;

	if( pDispType == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppDispType = pDispType;
#else
	/* duplicating the pValue to be returned */
	*ppDispType = (SIP_S8bit *) STRDUPACCESSOR( pDispType);
	if( *ppDispType == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	SIPDEBUGFN ( "Exiting sip_getDispTypeFromReasonHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/****************************************************************
**
** FUNCTION: sip_setDispTypeInReasonHdr
**
** DESCRIPTION: This function sets the pDispType field in the Reason
** Header.
**
*****************************************************************/
SipBool sip_setDispTypeInReasonHdr
#ifdef ANSI_PROTO
	( SipHeader *pHdr, SIP_S8bit *pDispType, SipError *pErr)
#else
	( pHdr, pDispType, pErr )
	  SipHeader *pHdr;
	  SIP_S8bit *pDispType;
	  SipError *pErr;
#endif
{

#ifndef SIP_BY_REFERENCE
	SIP_S8bit * pTempReason;
#endif
	SipReasonHeader * pTempReasonHdr;

	SIPDEBUGFN ( "Entering sip_setDispTypeInReasonHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeReason)
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
	/* freeing original pValue if it exists */
	pTempReasonHdr = ( SipReasonHeader *) ( pHdr->pHeader);
	if ( pTempReasonHdr->pDispType != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)\
			(&(pTempReasonHdr->pDispType)), pErr) == SipFail)
			return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	pTempReasonHdr->pDispType = pDispType;
#else
	if( pDispType == SIP_NULL)
		pTempReason = SIP_NULL;
	else
	{
		pTempReason = (SIP_S8bit *) STRDUPACCESSOR ( pDispType );
		if ( pTempReason == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}

	pTempReasonHdr->pDispType = pTempReason;
#endif

	SIPDEBUGFN ( "Exiting sip_setDispTypeInReasonHdr");

	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromReasonHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Reason Header
**
***************************************************************/
SipBool sip_getParamCountFromReasonHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromReasonHdr");
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
	if( pHdr->dType != SipHdrTypeReason)
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
	if (sip_listSizeOf( &(((SipReasonHeader *)(pHdr->pHeader))->slParam),\
		pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromReasonHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromReasonHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP Reason Header
**
***************************************************************/
SipBool sip_getParamAtIndexFromReasonHdr
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
	SIP_Pvoid pElementFromList;
	SipParam *pTempParam;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromReasonHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pParam == SIP_NULL)
#else
	if(  ppParam == SIP_NULL)
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
	if( pHdr->dType != SipHdrTypeReason)
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

	if (sip_listGetAt( &(((SipReasonHeader *)(pHdr->pHeader))->slParam), dIndex,\
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
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromReasonHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInReasonHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIP Reason pHeader
**
******************************************************************/
SipBool sip_insertParamAtIndexInReasonHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInReasonHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeReason)
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

	if( sip_listInsertAt( &(((SipReasonHeader *)(pHdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInReasonHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInReasonHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP Reason Header
**
***************************************************************/
SipBool sip_deleteParamAtIndexInReasonHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInReasonHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeReason)
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
	if( sip_listDeleteAt( &(((SipReasonHeader *)(pHdr->pHeader))->slParam), \
		dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInReasonHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInReasonHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP Reason Header
**
***************************************************************/
SipBool sip_setParamAtIndexInReasonHdr
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
	SIPDEBUGFN("Entering function sip_setParamAtIndexInReasonHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeReason)
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

	if( sip_listSetAt( &(((SipReasonHeader *)(pHdr->pHeader))->slParam),  \
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
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInReasonHdr");
	return SipSuccess;
}

#ifdef SIP_3GPP
/*************3GPP header P-Charging-Function-Addresses*****/
/*********************************************************
** FUNCTION: sip_getParamCountFromPcfAddrHdr
**
** DESCRIPTION: This function retrieves the number of parameters
**		from a SIP PcfAddr Header
**
**********************************************************/
SipBool sip_getParamCountFromPcfAddrHdr
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_getParamCountFromPcfAddrHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
    {
        SIPDEBUGFN("Exiting function sip_getParamCountFromPcfAddrHdr");
		return SipFail;
    }

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPcfAddrHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePcfAddr))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPcfAddrHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPcfAddrHdr");
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPcfAddrHdr");
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipPcfAddrHeader *)(pHdr->pHeader))->slParams), pCount , pErr) == SipFail )
	{
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPcfAddrHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromPcfAddrHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_getParamAtIndexFromPcfAddrHdr
**
** DESCRIPTION: This function retrieves a param at a specified
**		index in a PcfAddr header
**
**********************************************************/
SipBool sip_getParamAtIndexFromPcfAddrHdr
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
#else
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr)
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPcfAddrHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
    {
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
    }

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;

        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePcfAddr))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL) )
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipPcfAddrHeader *)(pHdr->pHeader))->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
    {
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
    }

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
    {
        SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
		return SipFail;
    }
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPcfAddrHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_setParamAtIndexInPcfAddrHdr
**
** DESCRIPTION: This function sets a param at a specified
**		index in a PcfAddr header
**
**********************************************************/
SipBool sip_setParamAtIndexInPcfAddrHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPcfAddrHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
    {
        SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
		return SipFail;
    }

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePcfAddr))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
        {
            SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
			return SipFail;
        }
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
	        SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipPcfAddrHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInPcfAddrHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_insertParamAtIndexInPcfAddrHdr
**
** DESCRIPTION: This function inserts a param at a specified
**		index in a PcfAddr header
**
**********************************************************/
SipBool sip_insertParamAtIndexInPcfAddrHdr
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPcfAddrHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPcfAddrHdr");
		return SipFail;
	}

	if ((pHdr ->dType != SipHdrTypePcfAddr))
	{
		*pErr = E_INV_TYPE;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPcfAddrHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPcfAddrHdr");
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
        {
            SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPcfAddrHdr");
			return SipFail;
        }
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
	        SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPcfAddrHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(((SipPcfAddrHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPcfAddrHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPcfAddrHdr");
	return SipSuccess;
}


/*********************************************************
** FUNCTION: sip_deleteParamAtIndexInPcfAddrHdr
**
** DESCRIPTION: This function deletes a param at a specified
**		index in a PcfAddr header
**
**********************************************************/
SipBool sip_deleteParamAtIndexInPcfAddrHdr
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPcfAddrHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pHdr == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPcfAddrHdr");
		return SipFail;
	}

	if ((pHdr->dType != SipHdrTypePcfAddr))
	{
		*pErr = E_INV_TYPE;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPcfAddrHdr");
		return SipFail;
	}
	if ((pHdr->pHeader == SIP_NULL))
	{
		*pErr = E_INV_HEADER;
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPcfAddrHdr");
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(((SipPcfAddrHeader *)(pHdr->pHeader))->slParams), dIndex, pErr) == SipFail)
    {
        SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPcfAddrHdr");
		return SipFail;
    }

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPcfAddrHdr");
	return SipSuccess;
}
/*
 * --------------- Service Route Accessor APIs-----------------
 */

/*************************************************************
**
** FUNCTION:  sip_getDispNameFromServiceRouteHdr
**
** DESCRIPTION:This function retrieves the Display Name field from
**		a SIP ServiceRoute pHeader
**
**************************************************************/

SipBool sip_getDispNameFromServiceRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **pDispName, SipError *pErr)
#else
	(pHdr, pDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit **pDispName;
	SipError *pErr;
#endif
{
 	SIP_S8bit *pTempDispName=SIP_NULL;
	SIPDEBUGFN("Entering function sip_getDispNameFromServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pDispName == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getDispNameFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getDispNameFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_getDispNameFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_getDispNameFromServiceRouteHdr");
		return SipFail;
	}
#endif
 	pTempDispName = ((SipServiceRouteHeader *)(pHdr ->pHeader))->pDispName;
 	if (pTempDispName == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_getDispNameFromServiceRouteHdr");
 		return SipFail;
 	}
#ifndef SIP_BY_REFERENCE
 	*pDispName = (SIP_S8bit *)STRDUPACCESSOR(pTempDispName);
	if (*pDispName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		SIPDEBUGFN("Exiting function sip_getDispNameFromServiceRouteHdr");
		return SipFail;
	}
#else
	*pDispName = pTempDispName;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getDispNameFromServiceRouteHdr");
 	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInServiceRouteHdr
**
** DESCRIPTION: This function sets the Display-Name field in a SIP
**		Service-Route pHeader
**
***************************************************************/
SipBool sip_setDispNameInServiceRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pDispName, SipError *pErr)
#else
	(pHdr, pDispName, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pDispName;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempDispName=SIP_NULL;
	SIP_S8bit *pName=SIP_NULL;
	SIPDEBUGFN("Entering function sip_setDispNameInServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
			*pErr = E_INV_PARAM;
			SIPDEBUGFN("Exiting function sip_setDispNameInServiceRouteHdr");
			return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
			*pErr = E_INV_TYPE;
			SIPDEBUGFN("Exiting function sip_setDispNameInServiceRouteHdr");
			return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_setDispNameInServiceRouteHdr");
		return SipFail;
	}
#endif
    if( pDispName == SIP_NULL)
    	pTempDispName = SIP_NULL;
    else
    {
#ifndef SIP_BY_REFERENCE
		pTempDispName = (SIP_S8bit *)STRDUPACCESSOR (pDispName);
		if (pTempDispName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			SIPDEBUGFN("Exiting function sip_setDispNameInServiceRouteHdr");
			return SipFail;
		}
#else
		pTempDispName = pDispName;
#endif
	}

    pName = ((SipServiceRouteHeader *)(pHdr->pHeader))->pDispName;
    if (pName != SIP_NULL)
    {
    	if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pName), pErr) \
    		== SipFail)
			{
					SIPDEBUGFN("Exiting function sip_setDispNameInServiceRouteHdr");
					return SipFail;
			}
    }

	((SipServiceRouteHeader *)(pHdr->pHeader))->pDispName = pTempDispName;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setDispNameInServiceRouteHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromServiceRouteHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP ServiceRoute pHeader
**
***************************************************************/
SipBool sip_getAddrSpecFromServiceRouteHdr
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)
#endif
#else
#ifndef SIP_BY_REFERENCE
	(pHdr, pAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#else
	(pHdr, ppAddrSpec, pErr)
	SipHeader *pHdr;
	SipAddrSpec **ppAddrSpec;
	SipError *pErr;
#endif
#endif
{
	SipAddrSpec *pTempAddrSpec=SIP_NULL;
	SIPDEBUGFN("Entering function sip_getAddrSpecFromServiceRouteHdr");
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
		SIPDEBUGFN("Exiting function sip_getAddrSpecFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getAddrSpecFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_getAddrSpecFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_getAddrSpecFromServiceRouteHdr");
		return SipFail;
	}
#endif

 	pTempAddrSpec = ((SipServiceRouteHeader *) (pHdr ->pHeader))->pAddrSpec;
 	if (pTempAddrSpec == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_getAddrSpecFromServiceRouteHdr");
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
		pAddrSpec->dType = SipAddrAny;
		SIPDEBUGFN("Exiting function sip_getAddrSpecFromServiceRouteHdr");
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(pTempAddrSpec->dRefCount);
	*ppAddrSpec = pTempAddrSpec;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getAddrSpecFromServiceRouteHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInServiceRouteHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		ServiceRoute Header
**
***************************************************************/
SipBool sip_setAddrSpecInServiceRouteHdr
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
	SIPDEBUGFN("Entering function sip_setAddrSpecInServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_setAddrSpecInServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_setAddrSpecInServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_setAddrSpecInServiceRouteHdr");
		return SipFail;
	}
#endif
  	if (pAddrSpec == SIP_NULL)
	{
		sip_freeSipAddrSpec(((SipServiceRouteHeader *) (pHdr ->pHeader))->pAddrSpec);
 		((SipServiceRouteHeader *) (pHdr ->pHeader))->pAddrSpec = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipAddrSpec(&pTempAddrSpec, pAddrSpec->dType, pErr ) \
			== SipFail)
		{
			SIPDEBUGFN("Exiting function sip_setAddrSpecInServiceRouteHdr");
			return SipFail;
		}

		if (__sip_cloneSipAddrSpec(pTempAddrSpec, pAddrSpec, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pTempAddrSpec);
			SIPDEBUGFN("Exiting function sip_setAddrSpecInServiceRouteHdr");
			return SipFail;
		}
		sip_freeSipAddrSpec(((SipServiceRouteHeader *)(pHdr ->pHeader))->pAddrSpec);
 		((SipServiceRouteHeader *)(pHdr ->pHeader))->pAddrSpec = pTempAddrSpec;
#else
		HSS_LOCKEDINCREF(pAddrSpec->dRefCount);
		sip_freeSipAddrSpec(((SipServiceRouteHeader *) (pHdr ->pHeader))->pAddrSpec);
		((SipServiceRouteHeader *)(pHdr ->pHeader))->pAddrSpec = pAddrSpec;
#endif
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setAddrSpecInServiceRouteHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromServiceRouteHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		ServiceRoute pHeader
**
***************************************************************/

SipBool sip_getParamCountFromServiceRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, pCount, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getParamCountFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getParamCountFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_getParamCountFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_getParamCountFromServiceRouteHdr");
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(((SipServiceRouteHeader *)(pHdr->pHeader))->slParams), \
		pCount , pErr) == SipFail )
	{
			SIPDEBUGFN("Exiting function sip_getParamCountFromServiceRouteHdr");
			return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromServiceRouteHdr");
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromServiceRouteHdr
**
** DESCRIPTION: This function gets the param field at a given index
**				 in a SIP Service Route pHeader
**
***************************************************************/

SipBool sip_getParamAtIndexFromServiceRouteHdr
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
	SIP_Pvoid pElementFromList=SIP_NULL;
	SipParam *pTempParam;

	SIPDEBUGFN("Entering function sip_getParamAtIndexFromServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if(  pParam == SIP_NULL)
#else
	if(  ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(((SipServiceRouteHeader *)(pHdr->pHeader))->slParams), \
		dIndex, &pElementFromList, pErr) == SipFail)
	{
			SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
			return SipFail;
	}

	if (pElementFromList == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
		return SipFail;
	}

	pTempParam = (SipParam *)pElementFromList;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, pTempParam, pErr) == SipFail)
	{
			SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
			return SipFail;
	}
#else
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppParam = pTempParam;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromServiceRouteHdr");
	return SipSuccess;
}



/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInServiceRouteHdr
**
** DESCRIPTION: This function sets a param at a specified index
**		in a ServiceRoute Header
**
***************************************************************/

SipBool sip_setParamAtIndexInServiceRouteHdr
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
	SipParam *pTempParam=SIP_NULL;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_setParamAtIndexInServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_setParamAtIndexInServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_setParamAtIndexInServiceRouteHdr");
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
		{
				SIPDEBUGFN("Exiting function sip_setParamAtIndexInServiceRouteHdr");
				return SipFail;
		}
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam);
			SIPDEBUGFN("Exiting function sip_setParamAtIndexInServiceRouteHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}

	if( sip_listSetAt( &(((SipServiceRouteHeader *)(pHdr->pHeader))->slParams),  \
		dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		SIPDEBUGFN("Exiting function sip_setParamAtIndexInServiceRouteHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInServiceRouteHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInServiceRouteHdr
**
** DESCRIPTION: This function inserts a param at a specified index
**		in a ServiceRoute Header
**
***************************************************************/
SipBool sip_insertParamAtIndexInServiceRouteHdr
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
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_insertParamAtIndexInServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting function sip_insertParamAtIndexInServiceRouteHdr");
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		SIPDEBUGFN("Exiting function sip_insertParamAtIndexInServiceRouteHdr");
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
		{
				SIPDEBUGFN("Exiting function sip_insertParamAtIndexInServiceRouteHdr");
				return SipFail;
		}
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (pTempParam);
			SIPDEBUGFN("Exiting function sip_insertParamAtIndexInServiceRouteHdr");
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}

	if( sip_listInsertAt(&(((SipServiceRouteHeader *)(pHdr->pHeader))->slParams)\
		,dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		SIPDEBUGFN("Exiting function sip_insertParamAtIndexInServiceRouteHdr");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInServiceRouteHdr");
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInPathHdr
**
** DESCRIPTION: This function deletes a param at a specified index
**		in a ServiceRoute Header
**
***************************************************************/
SipBool sip_deleteParamAtIndexInServiceRouteHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dIndex, pErr)
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInServiceRouteHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeServiceRoute)
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
	if( sip_listDeleteAt( &(((SipServiceRouteHeader *)(pHdr->pHeader))->slParams), \
		dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInServiceRouteHdr");
	return SipSuccess;
}

#endif
