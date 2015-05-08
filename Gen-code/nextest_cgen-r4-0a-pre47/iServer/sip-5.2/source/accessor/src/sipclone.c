/******************************************************************************
** FUNCTION:
**
**
*******************************************************************************
**
** FILENAME:
** 	sipclone.c
**
** DESCRIPTION:
**
**
** DATE      NAME          REFERENCE      REASON
** ----      ----          ---------      ------
** 13Dec99   S.Luthra	   Creation
**	         B.Borthakur
**	         Preethy
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipbcptinit.h"
#ifdef SIP_CCP
#include "ccpinit.h"
#include "ccpinternal.h"
#include "ccpfree.h"
#endif
#ifdef SIP_IMPP
#include "imppinit.h"
#include "imppinternal.h"
#include "imppfree.h"
#endif
#include "rprinit.h"
#include "sipfree.h"
#include "sipbcptfree.h"
#include "rprfree.h"
#include "siplist.h"
#include "sipinternal.h"
#include "rprinternal.h"
#include "sipbcptinternal.h"
#include "sipvalidate.h"
#include "sdp.h"
#include "sipdecode.h"
#include "sipdecodeintrnl.h"
#include "sipclone.h"
#ifdef SIP_DCS
#include "dcsintrnl.h"
#include "dcsfree.h"
#endif


/******************************************************************
**
** FUNCTION:  __sip_cloneSipParamList
**
** DESCRIPTION:  This function makes a deep copy of a
** SipList of SipParam from the "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipParamList
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

	SIPDEBUGFN("Entering function __sip_cloneSipParamList");
	/* copying siplist of SipParam */
	if ( sip_listDeleteAll(dest , err ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(source, &count, err) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(source,index, (SIP_Pvoid * ) (&temp_param), err) == SipFail )
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

			if ( __sip_cloneSipParam(  clone_param, temp_param,err) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_param),SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(dest, clone_param, err) == SipFail )
		{
			if ( clone_param != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_param), SIP_NULL);
			return SipFail;
		}
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipParamList");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipContentLanguageHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
** 	the ContentLanguageHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipContentLanguageHeader
#ifdef ANSI_PROTO
	(SipContentLanguageHeader *dest, SipContentLanguageHeader *source, SipError *err)
#else
	(dest, source, err)
	SipContentLanguageHeader *dest;
	SipContentLanguageHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipContentLanguageHeader");
	if (dest->pLangTag != SIP_NULL)
		sip_freeString(dest->pLangTag);

	if (source->pLangTag == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pLangTag);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pLangTag = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentLanguageHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipCallIdHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
** 	the CallId structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipCallIdHeader
#ifdef ANSI_PROTO
	(SipCallIdHeader *dest, SipCallIdHeader *source, SipError *err)
#else
	(dest, source, err)
	SipCallIdHeader *dest;
	SipCallIdHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipCallIdHeader");
	if (dest->pValue != SIP_NULL)
		sip_freeString(dest->pValue);

	if (source->pValue == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pValue);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pValue = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipCallIdHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipCseqHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the CSeq structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipCseqHeader
#ifdef ANSI_PROTO
	(SipCseqHeader *dest, SipCseqHeader *source, SipError *err)
#else
	(dest, source, err)
	SipCseqHeader *dest;
	SipCseqHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipCseqHeader");
	if (dest->pMethod != SIP_NULL)
		sip_freeString(dest->pMethod);

	dest->dSeqNum = source->dSeqNum;
	if (source->pMethod == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ( (temp = (SIP_S8bit *)STRDUPACCESSOR (source->pMethod)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pMethod = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipCseqHeader");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sip_cloneSipInReplyToHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
** 	the InReplyToHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipInReplyToHeader
#ifdef ANSI_PROTO
	(SipInReplyToHeader *dest, SipInReplyToHeader *source, SipError *err)
#else
	(dest, source, err)
	SipInReplyToHeader *dest;
	SipInReplyToHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipInReplyToHeader");
	if (dest->pCallId != SIP_NULL)
		sip_freeString(dest->pCallId);

	if (source->pCallId == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pCallId);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pCallId = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipInReplyToHeader");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipAcceptHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
** 	the AcceptContactHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipAcceptHeader
#ifdef ANSI_PROTO
	(SipAcceptHeader *dest, SipAcceptHeader *source, SipError *err)
#else
	(dest, source, err)
	SipAcceptHeader *dest;
	SipAcceptHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipAcceptHeader");

	if (dest->pMediaRange != SIP_NULL)
		sip_freeString(dest->pMediaRange);
	if (dest->pAcceptRange != SIP_NULL)
		sip_freeString(dest->pAcceptRange);

	if (source->pMediaRange == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pMediaRange)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pMediaRange = temp;

	if (source->pAcceptRange == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pAcceptRange)) == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pMediaRange, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pAcceptRange = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipAcceptEncodingHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the AcceptEncodingHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipAcceptEncodingHeader
#ifdef ANSI_PROTO
	(SipAcceptEncodingHeader *dest, SipAcceptEncodingHeader *source,\
	 SipError *err)
#else
	(source, dest, err)
	SipAcceptEncodingHeader *dest;
	SipAcceptEncodingHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIP_U32bit size,index;
	SipNameValuePair *pElement_from_src=SIP_NULL;
	SipNameValuePair *pElement_into_dest=SIP_NULL;

	SIPDEBUGFN("Entering function __sip_cloneSipAcceptEncodingHeader");

	if (dest->pCoding != SIP_NULL)
		sip_freeString(dest->pCoding);

	if (dest->pQValue != SIP_NULL)
		sip_freeString(dest->pQValue);

	if (source->pCoding == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pCoding)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pCoding = temp;

	if (source->pQValue == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pQValue)) == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pCoding, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pQValue = temp;

	if ( sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	if ( sip_listSizeOf(&(source->slParam), &size, err) == SipFail)
		return SipFail;

	for ( index = 0; index < size ; index++ )
	{
		if( sip_listGetAt\
			(&(source->slParam),index, (SIP_Pvoid * ) (&pElement_from_src),\
				err) == SipFail )
		{
			return SipFail;
		}
		if (pElement_from_src == SIP_NULL)
			pElement_into_dest = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pElement_into_dest, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pElement_into_dest,\
					pElement_from_src, err) == SipFail)
			{
				sip_freeSipNameValuePair(pElement_into_dest);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slParam),\
			(SIP_Pvoid *)pElement_into_dest, err) == SipFail)
		{
			if (pElement_into_dest != SIP_NULL)
				sip_freeSipNameValuePair (pElement_into_dest);
			return SipFail;
		}

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptEncodingHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipAcceptLangHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the AcceptLanguageHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipAcceptLangHeader
#ifdef ANSI_PROTO
	(SipAcceptLangHeader *dest, SipAcceptLangHeader *src, SipError *err)
#else
	(dest, src, err)
	SipAcceptLangHeader *dest;
	SipAcceptLangHeader *src;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIP_U32bit	index,size;
	SipNameValuePair *pElement_from_src=SIP_NULL;
	SipNameValuePair *pElement_into_dest=SIP_NULL;


	SIPDEBUGFN("Entering function __sip_cloneSipAcceptLangHeader");

	if (dest->pLangRange != SIP_NULL)
		sip_freeString(dest->pLangRange);
	if (dest->pQValue != SIP_NULL)
		sip_freeString(dest->pQValue);

	if (src->pLangRange == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(src->pLangRange))\
				== SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pLangRange = temp;
	if (src->pQValue == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(src->pQValue))\
				== SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pLangRange, err )\
					== SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pQValue = temp;

	if ( sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	if ( sip_listSizeOf(&(src->slParam), &size, err) == SipFail)
		return SipFail;

	for ( index = 0; index < size ; index++ )
	{
		if( sip_listGetAt\
			(&(src->slParam),index, (SIP_Pvoid * ) (&pElement_from_src),\
				err) == SipFail )
		{
			return SipFail;
		}
		if (pElement_from_src == SIP_NULL)
			pElement_into_dest = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pElement_into_dest, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pElement_into_dest,\
					pElement_from_src, err) == SipFail)
			{
				sip_freeSipNameValuePair(pElement_into_dest);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slParam),\
			(SIP_Pvoid *)pElement_into_dest, err) == SipFail)
		{
			if (pElement_into_dest != SIP_NULL)
				sip_freeSipNameValuePair (pElement_into_dest);
			return SipFail;
		}

	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptLangHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipTimeStampHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the TimeStampHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipTimeStampHeader
#ifdef ANSI_PROTO
	(SipTimeStampHeader *dest, SipTimeStampHeader *source, SipError *err)
#else
	(dest, source, err)
	SipTimeStampHeader *dest;
	SipTimeStampHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipTimeStampHeader");
	if (dest->pTime != SIP_NULL)
		sip_freeString(dest->pTime);
	if (dest->delay != SIP_NULL)
		sip_freeString(dest->delay);

	if (source->pTime == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pTime)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pTime = temp;
	if (source->delay == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->delay)) == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pTime, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->delay = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipTimeStampHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipContentLengthHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ContentLengthHeader structures "source" to "dest".
**
******************************************************************/
SipBool	__sip_cloneSipContentLengthHeader
#ifdef ANSI_PROTO
	(SipContentLengthHeader *dest, SipContentLengthHeader *source,\
	 SipError *err)
#else
	(dest, source, err)
	SipContentLengthHeader *dest;
	SipContentLengthHeader *source;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipContentLengthHeader");
	dest->dLength = source->dLength;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentLengthHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipContentTypeHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ContentTypeHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipContentTypeHeader
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
	SIP_U32bit count;

	SIPDEBUGFN("Entering function __sip_cloneSipContentTypeHeader");
	if (dest->pMediaType != SIP_NULL)
		sip_freeString(dest->pMediaType);
	if(sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	if (source->pMediaType == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pMediaType)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pMediaType = temp;

	/* copying siplist of SipParmas */
	if (sip_listInit(& (dest->slParams ),(source->slParams).freefunc,err)\
		==SipFail)
		return SipFail;
	if ( sip_listSizeOf(&( source->slParams ), &count, err) == SipFail )
		return SipFail;

	if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentTypeHeader");
	return SipSuccess;
}



/******************************************************************
**
** FUNCTION:  __sip_cloneSipContentEncodingHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ContentEncodingHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipContentEncodingHeader
#ifdef ANSI_PROTO
	(SipContentEncodingHeader *dest, SipContentEncodingHeader *source,\
	 SipError *err)
#else
	(dest, source, err)
	SipContentEncodingHeader *dest;
	SipContentEncodingHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipContentEncodingHeader");

	if (dest->pEncoding != SIP_NULL)
		sip_freeString(dest->pEncoding);

	if (source->pEncoding == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pEncoding)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pEncoding = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentEncodingHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipUnknownHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the UnknownHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipUnknownHeader
#ifdef ANSI_PROTO
	(SipUnknownHeader *dest, SipUnknownHeader *source, SipError *err)
#else
	(dest, source, err)
	SipUnknownHeader *dest;
	SipUnknownHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipUnknownHeader");

	if (dest->pName != SIP_NULL)
		sip_freeString(dest->pName);
	if (dest->pBody != SIP_NULL)
		sip_freeString(dest->pBody);

	if (source->pName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pName)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pName = temp;

	if (source->pBody == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pBody)) == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pName, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pBody = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipUnknownHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipDateHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the UnknownHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipDateHeader
#ifdef ANSI_PROTO
	(SipDateHeader *dest, SipDateHeader *src, SipError *err)
#else
	(dest, src, err)
	SipDateHeader *dest;
	SipDateHeader *src;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipDateHeader");
	if(dest->pDate != SIP_NULL)
		sip_freeSipDateFormat(dest->pDate);
	if(dest->pTime != SIP_NULL)
		sip_freeSipTimeFormat(dest->pTime);
	dest->dDow = src->dDow;
	if(sip_initSipDateFormat( &(dest->pDate), err) == SipFail)
		return SipFail;
	if(__sip_cloneSipDateFormat( dest->pDate, src->pDate ,err)==SipFail)
	{
		sip_freeSipDateFormat(dest->pDate);
		return SipFail;
	}
	if(sip_initSipTimeFormat( &(dest->pTime), err) == SipFail)
	{
		sip_freeSipDateFormat(dest->pDate);
		return SipFail;
	}
	if(__sip_cloneSipTimeFormat( dest->pTime, src->pTime ,err)==SipFail)
	{
		sip_freeSipTimeFormat(dest->pTime);
		sip_freeSipDateFormat(dest->pDate);
		return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipDateHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipEncryptionHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the EncryptionHeader structures "source" to "dest".
**
******************************************************************/
SipBool	__sip_cloneSipEncryptionHeader
#ifdef ANSI_PROTO
	(SipEncryptionHeader *dest, SipEncryptionHeader *source, SipError *err)
#else
	(dest, source, err)
	SipEncryptionHeader *dest;
	SipEncryptionHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipEncryptionHeader");
	if (dest->pScheme != SIP_NULL)
		sip_freeString(dest->pScheme);
	if (sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	/* duplicating pScheme */
	if (source->pScheme == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pScheme);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pScheme = temp;

	/* duplicating pParam */
	if (__sip_cloneSipParamList(&(dest->slParam), &(source->slParam), err)\
		 == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipEncryptionHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipExpiresHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ExpiresHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipExpiresHeader
#ifdef ANSI_PROTO
	(SipExpiresHeader *dest, SipExpiresHeader *source, SipError *err)
#else
	(dest, source, err)
	SipExpiresHeader *dest;
	SipExpiresHeader *source;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipExpiresHeader");
	if (validateSipExpiresType(source->dType, err) == SipFail)
		return SipFail;

	if(dest->dType == SipExpDate)
		if(dest->u.pDate != SIP_NULL)
			sip_freeSipDateStruct(dest->u.pDate);

	dest->dType = source->dType;
	switch (source->dType)
	{
		case SipExpDate:
					if ((source->u).pDate!=SIP_NULL)
					{
						if(sip_initSipDateStruct(&(dest->u.pDate),err)==SipFail)
							return SipFail;
						if (__sip_cloneSipDateStruct((dest->u).pDate,\
							(source->u).pDate, err) == SipFail)
						{
							sip_freeSipDateStruct(dest->u.pDate);
							return SipFail;
						}
					}
					break;
		case SipExpSeconds:
					(dest->u).dSec = (source->u).dSec;
					break;
		case SipExpAny:
					*err = E_INV_PARAM;
					return SipFail;
		default:
					*err = E_INV_TYPE;
					return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipExpiresHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipFromHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the FromHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipFromHeader
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
	SIPDEBUGFN("Entering function __sip_cloneSipFromHeader");
	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);
	if (dest->pAddrSpec != SIP_NULL)
		sip_freeSipAddrSpec(dest->pAddrSpec);
	if (sip_listDeleteAll(&(dest->slTag), err) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	/* Duplicating pDispName */
	if (source->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pDispName);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pDispName = temp;
	/* Duplicating pAddrSpec */
	if (validateSipAddrSpecType((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}
	/* Duplicating slTag */
	if (__sip_cloneSipStringList (&(dest->slTag), &(source->slTag), err)\
		 == SipFail)
		return SipFail;

	/* clone pExtParam */
	if (__sip_cloneSipParamList (&(dest->slParam), &(source->slParam),\
		 err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipFromHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipToHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ToHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipToHeader
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
	SIPDEBUGFN("Entering function __sip_cloneSipToHeader");

	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);
	sip_freeSipAddrSpec(dest->pAddrSpec);
	if (sip_listDeleteAll(&(dest->slTag), err) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;
	/* Duplicating pDispName */
	if (source->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pDispName);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pDispName = temp;
	/* Duplicating pAddrSpec */
	if (validateSipAddrSpecType((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}

	/* Duplicating slTag */
	if (__sip_cloneSipStringList (&(dest->slTag), &(source->slTag), err)\
		 == SipFail)
		return SipFail;

	/* clone pExtParam */
	if (__sip_cloneSipParamList (&(dest->slParam), &(source->slParam),\
		 err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipToHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipViaHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ViaHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipViaHeader
#ifdef ANSI_PROTO
	(SipViaHeader *dest, SipViaHeader *source, SipError *err)
#else
	(dest, source, err)
	SipViaHeader *dest;
	SipViaHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;

	SIPDEBUGFN("Entering function __sip_cloneSipViaHeader");
	if (dest->pSentProtocol != SIP_NULL)
		sip_freeString(dest->pSentProtocol);
	if (dest->pSentBy != SIP_NULL)
		sip_freeString(dest->pSentBy);
	if (dest->pComment != SIP_NULL)
		sip_freeString(dest->pComment);
	if (sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	/* duplicating sent protocol */
	if (source->pSentProtocol == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR (source->pSentProtocol);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pSentProtocol = temp;

	/* duplicating sent by */
	if (source->pSentBy == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR (source->pSentBy);
		if (temp == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pSentProtocol, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pSentBy = temp;

	/* duplicating pComment */
	if (source->pComment == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR (source->pComment);
		if (temp == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pSentProtocol, err ) == SipFail )
				return SipFail;
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pSentBy, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pComment = temp;
	if(__sip_cloneSipParamList(&(dest->slParam),&(source->slParam),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipViaHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipContactHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ViaHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipContactHeader
#ifdef ANSI_PROTO
	(SipContactHeader *dest, SipContactHeader *source, SipError *err)
#else
	(dest, source, err)
	SipContactHeader *dest;
	SipContactHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIP_U32bit count, index;
	SipContactParam *temp_contact_param, *clone_contact_param;
	SIP_Pvoid dummy;

	SIPDEBUGFN("Entering function __sip_cloneSipContactHeader");
	if (validateSipContactType(source->dType, err) == SipFail)
		return SipFail;

	if (source->dType == SipContactWildCard)
	{
		dest->dType = SipContactWildCard;
		dest->pDispName = SIP_NULL;
		dest->pAddrSpec = SIP_NULL;
		if (sip_listInit(&(dest->slContactParam), __sip_freeSipContactParam,\
			 err) == SipFail)
			return SipFail;
	}
	else
	{
		if (dest->pDispName != SIP_NULL)
			sip_freeString(dest->pDispName);
		if (dest->pAddrSpec != SIP_NULL)
			sip_freeSipAddrSpec(dest->pAddrSpec);
		if (sip_listDeleteAll(&(dest->slContactParam), err) == SipFail)
			return SipFail;
		/* copying the dType */
		dest->dType = source->dType;

		/* duplicating pDispName */
		if (source->pDispName == SIP_NULL)
			temp = SIP_NULL;
		else
		{
			temp = (SIP_S8bit *)STRDUPACCESSOR (source->pDispName);
			if (temp == SIP_NULL)
			{
				*err = E_NO_MEM;
	 			return SipFail;
			}
		}
		dest->pDispName = temp;


		/* duplicating pAddrSpec */
		if (validateSipAddrSpecType((source->pAddrSpec)->dType, err) == SipFail)
			return SipFail;
		if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
			 err) == SipFail)
			return SipFail;
		if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err) \
			== SipFail)
		{
			sip_freeSipAddrSpec(dest->pAddrSpec);
			return SipFail;
		}

		/* duplicating slContactParam */

		if (sip_listSizeOf (&(source->slContactParam), &count, err) == SipFail)
			return SipFail;

		for (index = 0; index < count; index ++)
		{
			if (sip_listGetAt(&(source->slContactParam), index, &dummy, err)\
				 == SipFail)
				return SipFail;
			temp_contact_param = (SipContactParam *)dummy;
			if (temp_contact_param == SIP_NULL)
				clone_contact_param = SIP_NULL;
			else
			{
				if (sip_initSipContactParam(&clone_contact_param,\
					 temp_contact_param->dType, err) == SipFail)
					return SipFail;
				if (__sip_cloneSipContactParam(clone_contact_param,\
					 temp_contact_param, err) == SipFail)
				{
					sip_freeSipContactParam(clone_contact_param);
		 	 		return SipFail;
				}
			}/* end of else */
			if (sip_listAppend(&(dest->slContactParam),\
				 (SIP_Pvoid)clone_contact_param, err) == SipFail)
			{
				sip_freeSipContactParam(clone_contact_param);
				return SipFail;
			}
		}
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContactHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipRecordRouteHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the RecordRouteHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipRecordRouteHeader
#ifdef ANSI_PROTO
	(SipRecordRouteHeader *dest, SipRecordRouteHeader *source, SipError *err)
#else
	(dest, source, err)
	SipRecordRouteHeader *dest;
	SipRecordRouteHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipRecordRouteHeader");

	/* cleaning up dest */

	sip_freeString(dest->pDispName);
	dest->pDispName = SIP_NULL;

	sip_freeSipAddrSpec(dest->pAddrSpec);
	dest->pAddrSpec = SIP_NULL;

	if(sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;
	/* cleaned up */
	/* clone pDispName */
	if (source->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pDispName);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pDispName = temp;

	/* clone pAddrSpec */
	if (validateSipAddrSpecType ((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}

	if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR ;
	SIPDEBUGFN("Exiting function __sip_cloneSipRecordRouteHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipStatusLine
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the StatusLine structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *dest, SipStatusLine *source, SipError *err)
#else
	(dest, source, err)
	SipStatusLine *dest;
	SipStatusLine *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipStatusLine");

	/* clearing destination parameter */
	if (dest->pVersion != SIP_NULL)
		sip_freeString(dest->pVersion);
	if (dest->pReason != SIP_NULL)
		sip_freeString(dest->pReason);

	/* now copying pVersion */
	if (source->pVersion == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pVersion);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pVersion = temp;

	/* Now copying pReason */
	if (source->pReason == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pReason);
		if (temp == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pVersion, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pReason = temp;

	/* now copying dCodeNum */
	dest->dCodeNum = source->dCodeNum;

	/* Now copying dCodeNum */
	dest->dCodeNum = source->dCodeNum;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipStatusLine");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sip_cloneSipReqLine
**
** DESCRIPTION:   This function makes a deep copy of the fileds from
**	the ReqLine structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipReqLine
#ifdef ANSI_PROTO
	(SipReqLine *dest, SipReqLine *source, SipError *err)
#else
	(dest, source, err)
	SipReqLine *dest;
	SipReqLine *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipReqLine");
	/* clear the destination request line */
	if (dest->pVersion != SIP_NULL)
		sip_freeString(dest->pVersion);
	if (dest->pMethod != SIP_NULL)
		sip_freeString(dest->pMethod);
	if (dest->pRequestUri != SIP_NULL)
		sip_freeSipAddrSpec(dest->pRequestUri);

	/* Now copying pVersion */
	if (source->pVersion == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pVersion);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pVersion = temp;

	/* Now copying pMethod */
	if (source->pMethod == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pMethod);
		if (temp == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pVersion, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pMethod = temp;

	/* Now copying pRequestUri */
	if (validateSipAddrSpecType((source->pRequestUri)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pRequestUri), (source->pRequestUri)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pRequestUri, source->pRequestUri, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pRequestUri);
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipReqLine");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sip_cloneSipAllowHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the AllowHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipAllowHeader
#ifdef ANSI_PROTO
	(SipAllowHeader *dest, SipAllowHeader *source, SipError *err)
#else
	(dest, source, err)
	SipAllowHeader *dest;
	SipAllowHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipAllowHeader");
	if (dest->pMethod != SIP_NULL)
		sip_freeString(dest->pMethod);

	if (source->pMethod == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pMethod);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pMethod = temp;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAllowHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipWarningHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the WarningHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipWarningHeader
#ifdef ANSI_PROTO
	(SipWarningHeader *dest, SipWarningHeader *source, SipError *err)
#else
	(dest, source, err)
	SipWarningHeader *dest;
	SipWarningHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipWarningHeader");

	if (dest->pText != SIP_NULL)
		sip_freeString(dest->pText);
	if (dest->pAgent != SIP_NULL)
		sip_freeString(dest->pAgent);


	if (source->pText == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pText);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pText = temp;

	if (source->pAgent == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pAgent);
		if (temp == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pText, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pAgent = temp;
	dest->dCodeNum = source->dCodeNum;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipWarningHeader");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sip_cloneSipRetryAfterHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the RetryAfterHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipRetryAfterHeader
#ifdef ANSI_PROTO
	(SipRetryAfterHeader *dest, SipRetryAfterHeader *source, SipError *err)
#else
	(dest, source, err)
	SipRetryAfterHeader *dest;
	SipRetryAfterHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	/* SIP_U32bit *tempdur;*/

	SIPDEBUGFN("Entering function __sip_cloneSipRetryAfterHeader");

	if(source->dType == SipExpAny)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	/* Cleaning up destination */
	if (dest->pComment != SIP_NULL)
		sip_freeString(dest->pComment);
	if(dest->dType == SipExpDate)
	{
		if(dest->u.pDate != SIP_NULL)
			sip_freeSipDateStruct(dest->u.pDate);
	}
	/* Cleaned up destination */

	dest->dType = source->dType;
	if (source->pComment == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pComment);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pComment = temp;

	/* if (source->pDuration == SIP_NULL)
		tempdur = SIP_NULL;
	else
	{
		tempdur = (SIP_U32bit *)fast_memget(ACCESSOR_MEM_ID,sizeof(SIP_U32bit),err);
		if (tempdur == SIP_NULL)
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pComment, err ) == SipFail )
				return SipFail;
			*err = E_NO_MEM;
			return SipFail;
		}
		*tempdur = *(source->pDuration);
	}
	dest->pDuration = tempdur;*/

	if(source->dType == SipExpDate)
	{
		if(source->u.pDate == SIP_NULL)
			dest->u.pDate = SIP_NULL;
		else
		{
			if(sip_initSipDateStruct(&(dest->u.pDate), err) == SipFail)
				return SipFail;
			if(__sip_cloneSipDateStruct(dest->u.pDate,source->u.pDate,err)\
				==SipFail)
			{
				sip_freeSipDateStruct(dest->u.pDate);
				return SipFail;
			}
		}
	}
	else if(source->dType == SipExpSeconds)
	{
		dest->u.dSec = source->u.dSec;
	}

	if(sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	/* copying siplist of SipParmas */

	if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRetryAfterHeader");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sip_cloneSipProxyAuthenticateHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the ProxyAuthenticateHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipProxyAuthenticateHeader
#ifdef ANSI_PROTO
	(SipProxyAuthenticateHeader *dest, SipProxyAuthenticateHeader *source,\
	 SipError *err)
#else
	(dest, source, err)
	SipProxyAuthenticateHeader *dest;
	SipProxyAuthenticateHeader *source;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipProxyAuthenticateHeader");

	if(dest->pChallenge != SIP_NULL)
		sip_freeSipGenericChallenge(dest->pChallenge);
	if(source->pChallenge == SIP_NULL)
		dest->pChallenge = SIP_NULL;
	else
	{
		if(sip_initSipGenericChallenge(&(dest->pChallenge),err)==SipFail)
			return SipFail;
		if(__sip_cloneSipChallenge((dest->pChallenge),(source->pChallenge),err)\
			== SipFail)
		{
			sip_freeSipGenericChallenge(dest->pChallenge);
			return SipFail;
		}
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipProxyAuthenticateHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipWwwAuthenticateHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the WwwAuthenticateHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipWwwAuthenticateHeader
#ifdef ANSI_PROTO
	(SipWwwAuthenticateHeader *dest, SipWwwAuthenticateHeader *source,\
	 SipError *err)
#else
	(dest, source, err)
	SipWwwAuthenticateHeader *dest;
	SipWwwAuthenticateHeader *source;
	SipError *err;
#endif
{

	SIPDEBUGFN("Entering function __sip_cloneSipWwwAuthenticateHeader");
	if(dest->pChallenge != SIP_NULL)
		sip_freeSipGenericChallenge(dest->pChallenge);
	if(source->pChallenge == SIP_NULL)
		dest->pChallenge = SIP_NULL;
	else
	{
		if(sip_initSipGenericChallenge(&(dest->pChallenge),err)==SipFail)
			return SipFail;
		if(__sip_cloneSipChallenge((dest->pChallenge),(source->pChallenge),err)\
			== SipFail)
		{
			sip_freeSipGenericChallenge(dest->pChallenge);
			return SipFail;
		}
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipWwwAuthenticateHeader");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:__sip_cloneSipAuthorizationHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the AuthorizationHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipAuthorizationHeader
# ifdef ANSI_PROTO
	(SipAuthorizationHeader *dest,
	SipAuthorizationHeader *source,
	SipError *err)
# else
	( dest,source, err )
	SipAuthorizationHeader *dest;
	SipAuthorizationHeader *source;
	SipError *err;
#endif
{
	SIPDEBUGFN( "Entering __sip_cloneSipAuthorizationHeader");

	if((dest==SIP_NULL)||(source==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	/* Cleaning up to */
	if(dest->pCredential != SIP_NULL)
		sip_freeSipGenericCredential(dest->pCredential);
	/* Cleaned up to */
	if(source->pCredential == SIP_NULL)
		dest->pCredential = SIP_NULL;
	else
	{
		if(sip_initSipGenericCredential(&(dest->pCredential),\
			source->pCredential->dType, err)==SipFail)
			return SipFail;
		if(__sip_cloneCredential(dest->pCredential, source->pCredential, err)\
			==SipFail)
		{
			sip_freeSipGenericCredential(dest->pCredential);
			return SipFail;
		}
	}
	SIPDEBUGFN( "Exiting __sip_cloneSipAuthorizationHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneSipHideHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the HideHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipHideHeader
# ifdef ANSI_PROTO
	(SipHideHeader *to,
	SipHideHeader *from,
	SipError *err)
# else
	( to, from, err )
	SipHideHeader *to;
	SipHideHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipHideHeader");

	if(to->pType !=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)&(to->pType),err)==SipFail)
			return SipFail;

	/* Duplicating pType */
	if (from->pType == SIP_NULL)
		to->pType = SIP_NULL;
	else
	{
		dLength = strlen(from->pType);
		to->pType = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, dLength+1, err);
		if (to->pType == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy(to->pType, from->pType);
	}

	SIPDEBUGFN( "Exiting __sip_cloneSipHideHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneSipMaxForwardsHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the MaxForwardsHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipMaxForwardsHeader
# ifdef ANSI_PROTO
	(SipMaxForwardsHeader *to,
	SipMaxForwardsHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipMaxForwardsHeader *to;
	SipMaxForwardsHeader *from;
	SipError *err;
#endif
{
	SIPDEBUGFN( "Entering __sip_cloneSipMaxForwardsHeader");
	to->dHops=from->dHops;
	SIPDEBUGFN( "Exiting __sip_cloneSipMaxForwardsHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneSipOrganizationHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the OrganizationHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipOrganizationHeader
# ifdef ANSI_PROTO
	(SipOrganizationHeader *to,
	SipOrganizationHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipOrganizationHeader *to;
	SipOrganizationHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipOrganizationHeader");
	if((from==SIP_NULL)||(to==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	/* Cleaning up to */
	if(to->pOrganization!=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)&(to->pOrganization),err)==SipFail)
			return SipFail;
	/* Cleaned up to */
	if(from->pOrganization== SIP_NULL)
		{
			to->pOrganization=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pOrganization );

			to->pOrganization=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pOrganization == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pOrganization, from->pOrganization );

		}
	SIPDEBUGFN( "Exiting __sip_cloneSipOrganizationHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneSipPriorityHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the PriorityHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipPriorityHeader
# ifdef ANSI_PROTO
	(SipPriorityHeader *to,
	SipPriorityHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipPriorityHeader *to;
	SipPriorityHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipPriorityHeader");

	if (to->pPriority != SIP_NULL)
		if ((sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)&(to->pPriority),err))==SipFail)
			return SipFail;

	/* Duplicating pPriority */
	if (from->pPriority == SIP_NULL)
		to->pPriority = SIP_NULL;
	else
	{
		dLength = strlen(from->pPriority);

		to->pPriority = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, dLength+1, err);
		if (to->pPriority == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy(to->pPriority, from->pPriority);
	}

	SIPDEBUGFN( "Exiting __sip_cloneSipPriorityHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneSipProxyAuthorizationHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ProxyAuthorizationHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipProxyAuthorizationHeader
#ifdef ANSI_PROTO
	(SipProxyAuthorizationHeader *dest, SipProxyAuthorizationHeader *source,\
	 SipError *err)
#else
	(dest, source, err)
	SipProxyAuthorizationHeader *dest;
	SipProxyAuthorizationHeader *source;
	SipError *err;
#endif
{
	SIPDEBUGFN( "Entering __sip_cloneSipProxyAuthorizationHeader");

	if((dest==SIP_NULL)||(source==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	/* Cleaning up to */
	if(dest->pCredentials != SIP_NULL)
		sip_freeSipGenericCredential(dest->pCredentials);
	/* Cleaned up to */
	if(source->pCredentials == SIP_NULL)
		dest->pCredentials = SIP_NULL;
	else
	{
		if(sip_initSipGenericCredential(&(dest->pCredentials),\
			source->pCredentials->dType, err)==SipFail)
			return SipFail;
		if(__sip_cloneCredential(dest->pCredentials, source->pCredentials, err)\
			==SipFail)
		{
			sip_freeSipGenericCredential(dest->pCredentials);
			return SipFail;
		}
	}
	SIPDEBUGFN( "Exiting __sip_cloneSipProxyAuthorizationHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:__sip_cloneSipProxyRequireHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the ProxyRequireHeader structures "source" to "dest".
**
**********************************************************/

SipBool __sip_cloneSipProxyRequireHeader
# ifdef ANSI_PROTO
	(SipProxyRequireHeader *to,
	SipProxyRequireHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipProxyRequireHeader *to;
	SipProxyRequireHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	if((from==SIP_NULL)||(to==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	/* Cleaning up to */
	if(to->pToken!=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)&(to->pToken),err)==SipFail)
			return SipFail;
	/* Cleaned up to */
	SIPDEBUGFN( "Entering __sip_cloneSipProxyRequireHeader");
	if(from->pToken== SIP_NULL)
		{
			to->pToken=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pToken );

			to->pToken=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pToken == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pToken, from->pToken );
		}
	SIPDEBUGFN( "Exiting __sip_cloneSipProxyRequireHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/* call-transfer */

/******************************************************************
**
** FUNCTION:  __sip_cloneSipReferToHeader
**
** DESCRIPTION:  This function makes a deep copy of the fields from
**	the ReferToHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipReferToHeader
#ifdef ANSI_PROTO
	(SipReferToHeader *dest, SipReferToHeader *source, SipError *err)
#else
	(dest, source, err)
	SipReferToHeader *dest;
	SipReferToHeader *source;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN("Entering function __sip_cloneSipReferToHeader");

	/* cleaning up dest */
	if (dest->pAddrSpec != SIP_NULL)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		dest->pAddrSpec = SIP_NULL;
	}
	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);

	if (sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;
	/* cleaned up */

	/* copy the source parameters */
	if(source->pDispName== SIP_NULL)
	{
		dest->pDispName=SIP_NULL;
	}
	else
	{
		dLength = strlen(source->pDispName );
		dest->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,err);
		if ( dest->pDispName == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( dest->pDispName, source->pDispName );
	}

	/* clone pAddrSpec */
	if (validateSipAddrSpecType ((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}

	/* Copying generic-params list */
	if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams), err)\
	==SipFail)
		return SipFail;

	*err = E_NO_ERROR ;
	SIPDEBUGFN("Exiting function __sip_cloneSipReferToHeader");
	return SipSuccess;
}

/* call-transfer */

/******************************************************************
**
** FUNCTION:  __sip_cloneSipReferredByHeader
**
** DESCRIPTION:  This function makes a deep copy of the fields from
**	the ReferredByHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipReferredByHeader
#ifdef ANSI_PROTO
	(SipReferredByHeader *pDest, SipReferredByHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipReferredByHeader *pDest;
	SipReferredByHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipReferredByHeader");
	/* clen-iup the destination structure */
	if (pDest->pDispName != SIP_NULL)
		sip_freeString(pDest->pDispName);
	if (pDest->pMsgId != SIP_NULL)
		sip_freeString(pDest->pMsgId);
	if (pDest->pAddrSpecReferrer != SIP_NULL)
	{
		sip_freeSipAddrSpec (pDest->pAddrSpecReferrer);
		pDest->pAddrSpecReferrer = SIP_NULL;
	}
	if (pDest->pAddrSpecReferenced != SIP_NULL)
	{
		sip_freeSipAddrSpec (pDest->pAddrSpecReferenced);
		pDest->pAddrSpecReferenced = SIP_NULL;
	}

	if (sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;
	/* copy the source parameters */

	if(pSource->pDispName== SIP_NULL)
	{
		pDest->pDispName=SIP_NULL;
	}
	else
	{
		dLength = strlen(pSource->pDispName );
		pDest->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,pErr);
		if ( pDest->pDispName == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( pDest->pDispName, pSource->pDispName );
	}

	/* Copy the msgId parameter i.e the cid= component*/
	if(pSource->pMsgId== SIP_NULL)
	{
		pDest->pMsgId=SIP_NULL;
	}
	else
	{
		dLength = strlen(pSource->pMsgId);
		pDest->pMsgId=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,pErr);
		if ( pDest->pMsgId == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( pDest->pMsgId, pSource->pMsgId );
	}
	
	/* copying Referrer */
	if (validateSipAddrSpecType ((pSource->pAddrSpecReferrer)->dType, pErr)\
	== SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(pDest->pAddrSpecReferrer), (pSource->\
	pAddrSpecReferrer)->dType, pErr) == SipFail)
		return SipFail;

	if (__sip_cloneSipAddrSpec(pDest->pAddrSpecReferrer, pSource->\
	pAddrSpecReferrer, pErr) == SipFail)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpecReferrer);
		return SipFail;
	}

	/* copying Referenced */
	if(pSource->pAddrSpecReferenced != SIP_NULL)
	{
		if (validateSipAddrSpecType ((pSource->pAddrSpecReferenced)->dType,\
		pErr) == SipFail)
			return SipFail;
		if (sip_initSipAddrSpec(&(pDest->pAddrSpecReferenced), (pSource->\
		pAddrSpecReferrer)->dType, pErr) == SipFail)
			return SipFail;

		if (__sip_cloneSipAddrSpec(pDest->pAddrSpecReferenced, pSource->\
		pAddrSpecReferenced, pErr) == SipFail)
		{
			sip_freeSipAddrSpec(pDest->pAddrSpecReferenced);
			return SipFail;
		}
	}


	/* Copying Scheme-params list */
	if(__sip_cloneSipParamList(&(pDest->slParams),&(pSource->slParams), pErr)\
	==SipFail)
		return SipFail;

	SIPDEBUGFN( "Entering __sip_cloneSipReferredByHeader");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:__sip_cloneSipRouteHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ProxyRouteHeader structures "source" to "dest".
**
**********************************************************/

SipBool __sip_cloneSipRouteHeader
# ifdef ANSI_PROTO
	(SipRouteHeader *to,
	SipRouteHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipRouteHeader *to;
	SipRouteHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipRouteHeader");
	/* clean up of to */

	if(to->pDispName != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pDispName)), err) == SipFail)
			return SipFail;
	}
	sip_freeSipAddrSpec(to->pAddrSpec);

	if(sip_listDeleteAll(&(to->slParams), err) == SipFail)
		return SipFail;
	/* clean up over */

	if(from->pDispName== SIP_NULL)
		{
			to->pDispName=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pDispName );

			to->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pDispName == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pDispName, from->pDispName );
		}

	if(sip_initSipAddrSpec(&to->pAddrSpec,from->pAddrSpec->dType,err)==SipFail)
	{
		return SipFail;
	}
	if(__sip_cloneSipAddrSpec(to->pAddrSpec,from->pAddrSpec,err)==SipFail)
	{
		sip_freeSipAddrSpec(to->pAddrSpec);
		return SipFail;
	}
	if(__sip_cloneSipParamList(&(to->slParams),&(from->slParams),err)==SipFail)
		return SipFail;

	SIPDEBUGFN( "Exiting __sip_cloneSipRouteHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneSipAlsoHeader
**
** DESCRIPTION:  This function makes a deep copy of the fields from
**	the Also Header structure "source" to "dest".
**
**********************************************************/

SipBool __sip_cloneSipAlsoHeader
# ifdef ANSI_PROTO
	(SipAlsoHeader *to,
	SipAlsoHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipAlsoHeader *to;
	SipAlsoHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipAlsoHeader");
	/* clean up of to */

	if(to->pDispName != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pDispName)), err) == SipFail)
			return SipFail;
	}
	sip_freeSipAddrSpec(to->pAddrSpec);
	/* clean up over */

	if(from->pDispName== SIP_NULL)
		{
			to->pDispName=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pDispName );

			to->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pDispName == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pDispName, from->pDispName );
		}

	if(sip_initSipAddrSpec(&to->pAddrSpec,from->pAddrSpec->dType,err)==SipFail)
	{
		return SipFail;
	}
	if(__sip_cloneSipAddrSpec(to->pAddrSpec,from->pAddrSpec,err)==SipFail)
	{
		sip_freeSipAddrSpec(to->pAddrSpec);
		return SipFail;
	}

	SIPDEBUGFN( "Exiting __sip_cloneSipAlsoHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneSipRequireHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the ProxyRequireHeader structures "source" to "dest".
**
**********************************************************/

SipBool __sip_cloneSipRequireHeader
# ifdef ANSI_PROTO
	(SipRequireHeader *to,
	SipRequireHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipRequireHeader *to;
	SipRequireHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;

	SIPDEBUGFN( "Entering __sip_cloneSipRequireHeader");
	/* clean up of to */

	if(to->pToken != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pToken)), err) == SipFail)
			return SipFail;
	}

	/* clean up over */

	if(from->pToken== SIP_NULL)
		{
			to->pToken=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pToken );

			to->pToken=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pToken == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pToken, from->pToken );
		}
	SIPDEBUGFN( "Exiting __sip_cloneSipRequireHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneSipRespKeyHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ProxyRespKeyHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipRespKeyHeader
# ifdef ANSI_PROTO
	(SipRespKeyHeader *to,
	SipRespKeyHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipRespKeyHeader *to;
	SipRespKeyHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipRespKeyHeader");

	/* clean up of from */

	if(sip_listDeleteAll(&(to->slParam),err)==SipFail)
		return SipFail;
	if(to->pKeyScheme != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pKeyScheme)), err) == SipFail)
			return SipFail;
	}

	/* clean up over */

	if(from->pKeyScheme== SIP_NULL)
		{
			to->pKeyScheme=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pKeyScheme );

			to->pKeyScheme=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pKeyScheme == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pKeyScheme, from->pKeyScheme );
		}
	if(__sip_cloneSipParamList(&(to->slParam),&(from->slParam),err)==SipFail)
		return SipFail;
	SIPDEBUGFN( "Exiting __sip_cloneSipRespKeyHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneSipSubjectHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the SubjectHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipSubjectHeader
# ifdef ANSI_PROTO
	(SipSubjectHeader *to,
	SipSubjectHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipSubjectHeader *to;
	SipSubjectHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipSubjectHeader");

	/* clean up of from */

	if(to->pSubject != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pSubject)), err) == SipFail)
			return SipFail;
	}

	/* clean up over */

	if(from->pSubject== SIP_NULL)
		{
			to->pSubject=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pSubject );

			to->pSubject=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pSubject == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pSubject, from->pSubject );
		}
	SIPDEBUGFN( "Exiting __sip_cloneSipSubjectHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneSipUserAgentHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the UserAgentHeader structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneSipUserAgentHeader
# ifdef ANSI_PROTO
	(SipUserAgentHeader *to,
	SipUserAgentHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipUserAgentHeader *to;
	SipUserAgentHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipUserAgentHeader");

	/* clean up of from */

	if(to->pValue != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pValue)), err) == SipFail)
			return SipFail;
	}

	/* clean up over */


	if(from->pValue== SIP_NULL)
		{
			to->pValue=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pValue );

			to->pValue=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pValue == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pValue, from->pValue );
		}
	SIPDEBUGFN( "Exiting __sip_cloneSipUserAgentHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipAddrSpec
**
** DESCRIPTION:   This function makes a deep copy of the fileds from
**	the AddrSpec structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipAddrSpec
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
	SIPDEBUGFN("Entering function __sip_cloneSipAddrSpec");

	if (validateSipAddrSpecType(source->dType, err) == SipFail)
		return SipFail;

	switch (dest->dType)
	{
		case SipAddrReqUri:
			if ((dest->u).pUri != SIP_NULL)
				sip_freeString((dest->u).pUri);
			break;
		case SipAddrSipUri:
		case SipAddrSipSUri:
			if ((dest->u).pSipUrl != SIP_NULL)
			 	sip_freeSipUrl((dest->u).pSipUrl);
			break;
		case SipAddrAny:
			break;
		default:
			*err = E_INV_TYPE;
			return SipFail;
	}

	dest->dType = source->dType;
	switch (source->dType)
	{
		case SipAddrReqUri:
			if ((source->u).pUri == SIP_NULL)
				temp = SIP_NULL;
			else
			{
				temp = (SIP_S8bit *)STRDUPACCESSOR((source->u).pUri);
				if (temp == SIP_NULL)
				{
					*err = E_NO_MEM;
					return SipFail;
				}
			 }
			 (dest->u).pUri = temp;
			 break;
		case SipAddrSipUri:
		case SipAddrSipSUri:
			if ((source->u).pSipUrl == SIP_NULL)
				(dest->u).pSipUrl = SIP_NULL;
			else
			{
				if (sip_initSipUrl(&((dest->u).pSipUrl), err) == SipFail)
					return SipFail;
				if (__sip_cloneSipUrl((dest->u).pSipUrl, (source->u).pSipUrl,\
					err) == SipFail)
				{
					sip_freeSipUrl((dest->u).pSipUrl);
					return SipFail;
				}
			}
			break;
		case SipAddrAny:
			*err = E_INV_PARAM;
			return SipFail;
		default:
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAddrSpec");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipViaParam
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
** the ViaParam structures "source" to "dest".
**
******************************************************************/
/*
SipBool __sip_cloneSipViaParam
#ifdef ANSI_PROTO
	(SipViaParam *dest, SipViaParam *source, SipError *err)
#else
	(dest, source, err)
	SipViaParam *dest;
	SipViaParam *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipViaParam");
	if (validateSipViaParamType(source->dType, err) == SipFail)
		return SipFail;
	 clear the destination parameter
	switch (dest->dType)
	{
		case SipViaParamHidden:
			sip_freeString((dest->u).pHidden);
			break;
		case SipViaParamTtl:
			break;
		case SipViaParamReceived:
			sip_freeString((dest->u).pViaReceived);
			break;
		case SipViaParamBranch:
			sip_freeString((dest->u).pViaBranch);
			break;
		case SipViaParamMaddr:
			sip_freeString((dest->u).pMaddr);
			break;
		case SipViaParamExtension:
			sip_freeString((dest->u).pExtParam);
			break;
		case SipViaParamAny:
			break;
		default:
			*err = E_INV_TYPE;
			return SipFail;
	}

	dest->dType = source->dType;

	switch (source->dType)
	{
		case SipViaParamHidden:
			if ((source->u).pHidden == SIP_NULL)
				temp = SIP_NULL;
			else
			{
				temp = (SIP_S8bit *)STRDUPACCESSOR((source->u).pHidden);
			 	if (temp == SIP_NULL)
			 	{
					*err = E_NO_MEM;
					return SipFail;
			 	}
			}
			(dest->u).pHidden = temp;
			break;
		case SipViaParamTtl:
			(dest->u).dTtl = (source->u).dTtl;
			break;
		case SipViaParamReceived:
			if ((source->u).pViaReceived == SIP_NULL)
				temp = SIP_NULL;
			else
			{
			 	temp = (SIP_S8bit *)STRDUPACCESSOR((source->u).pViaReceived);
			 	if (temp == SIP_NULL)
			 	{
					*err = E_NO_MEM;
					return SipFail;
			 	}
			 }
			 (dest->u).pViaReceived = temp;
			 break;
		case SipViaParamBranch:
			if ((source->u).pViaBranch == SIP_NULL)
				temp = SIP_NULL;
			else
			{
			 	temp  = (SIP_S8bit *)STRDUPACCESSOR((source->u).pViaBranch);
			 	if (temp == SIP_NULL)
			 	{
					*err = E_NO_MEM;
					return SipFail;
			 	}
			 }
			 (dest->u).pViaBranch = temp;
			 break;
		case SipViaParamMaddr:
			if ((source->u).pMaddr == SIP_NULL)
				temp = SIP_NULL;
			else
			{
				temp = (SIP_S8bit *)STRDUPACCESSOR((source->u).pMaddr);
			   	if (temp == SIP_NULL)
			   	{
				 	*err = E_NO_MEM;
					return SipFail;
				}
			}
			(dest->u).pMaddr = temp;
			break;
		case SipViaParamExtension:
			if ((source->u).pExtParam == SIP_NULL)
				temp = SIP_NULL;
			else
			{
				temp = (SIP_S8bit *)STRDUPACCESSOR((source->u).pExtParam);
			 	if (temp == SIP_NULL)
			 	{
					*err = E_NO_MEM;
					return SipFail;
			 	}
			}
			(dest->u).pExtParam = temp;
			break;
		case SipViaParamAny:
			*err = E_INV_PARAM;
			return SipFail;
		default:
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipViaParam");
	return SipSuccess;
}
*/

/******************************************************************
**
** FUNCTION:  __sip_cloneSipContactParam
**
** DESCRIPTION:  This function makes a deep copy of the fields from
**	the ContactParam structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipContactParam
#ifdef ANSI_PROTO
	(SipContactParam *dest, SipContactParam *source, SipError *err)
#else
	(dest, source, err)
	SipContactParam *dest;
	SipContactParam *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipContactParam");
	if (validateSipContactParamsType(source->dType, err) == SipFail)
		return SipFail;

	/* clear destination parameter */
	switch(dest->dType)
	{
		case SipCParamQvalue:
			if ((dest->u).pQValue != SIP_NULL)
				sip_freeString((dest->u).pQValue);
		 	break;
		case SipCParamExpires:
			if ((dest->u).pExpire != SIP_NULL)
				sip_freeSipExpiresStruct((dest->u).pExpire);
		 	break;
		case SipCParamExtension:
			if ((dest->u).pExtensionAttr != SIP_NULL)
				sip_freeString((dest->u).pExtensionAttr);
		 	break;
		case SipCParamFeatureParam:
			if ((dest->u).pParam!= SIP_NULL)
				sip_freeSipParam((dest->u).pParam);
		 	break;
		case SipCParamAny:
			break;
		default:
			*err = E_INV_TYPE;
			return SipFail;
	}
	dest->dType = source->dType;
	switch (source->dType)
	{
		case SipCParamQvalue:
			if ((source->u).pQValue == SIP_NULL)
				temp = SIP_NULL;
			else
			{
				temp = (SIP_S8bit *)STRDUPACCESSOR((source->u).pQValue);
				if (temp == SIP_NULL)
				{
					*err = E_NO_MEM;
					return SipFail;
				}
			}
			(dest->u).pQValue = temp;
			break;
		case SipCParamExpires:
			if (sip_initSipExpiresStruct(&((dest->u).pExpire), \
				((source->u).pExpire)->dType, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipExpiresStruct( (dest->u).pExpire, \
				(source->u).pExpire, err ) == SipFail)
			{
				sip_freeSipExpiresStruct((dest->u).pExpire);
				return SipFail;
			}
			break;
		case SipCParamExtension:
			if ((source->u).pExtensionAttr == SIP_NULL)
				temp = SIP_NULL;
			else
			{
				temp = (SIP_S8bit *)STRDUPACCESSOR((source->u).pExtensionAttr);
				if (temp == SIP_NULL)
				{
					*err = E_NO_MEM;
					return SipFail;
				}
			}
			(dest->u).pExtensionAttr = temp;
			break;
		case SipCParamFeatureParam:
		    {
		      if (sip_initSipParam(&((dest->u).pParam), err)\
                                            == SipFail)
				return SipFail;
		      if (__sip_cloneSipParam((dest->u).pParam,
       			       (source->u).pParam, err) == SipFail)
				return SipFail;
			break;
		    }
		case SipCParamAny:
			*err = E_INV_PARAM;
			break;
		default:
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContactParam");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipExpiresStruct
**
** DESCRIPTION:   This function makes a deep copy of the fileds from
**	the ExpiresStruct structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipExpiresStruct
#ifdef ANSI_PROTO
	(SipExpiresStruct *dest, SipExpiresStruct *source, SipError *err)
#else
	(dest, source, err)
	SipExpiresStruct *dest;
	SipExpiresStruct *source;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipExpiresStruct");
	if (validateSipExpiresType(source->dType, err) == SipFail)
		return SipFail;

	if(dest->dType == SipExpDate)
		if(dest->u.pDate != SIP_NULL)
			sip_freeSipDateStruct(dest->u.pDate);

	dest->dType = source->dType;
	switch (source->dType)
	{
		case SipExpDate:
					if ((source->u).pDate!=SIP_NULL)
					{	
						if(sip_initSipDateStruct(&(dest->u.pDate),err)==SipFail)
							return SipFail;
						if (__sip_cloneSipDateStruct((dest->u).pDate,\
							(source->u).pDate, err) == SipFail)
						{
							sip_freeSipDateStruct(dest->u.pDate);
							return SipFail;
						}
					}	
					break;
		case SipExpSeconds:
					(dest->u).dSec = (source->u).dSec;
					break;
		case SipExpAny:
					*err = E_INV_PARAM;
					return SipFail;
		default:
					*err = E_INV_TYPE;
					return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipExpiresStruct");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sip_cloneSipStringList
**
** DESCRIPTION:  This function makes a deep copy of a SipList of
** SIP_S8bit *  from the "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipStringList
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

	SIPDEBUGFN("Entering function __sip_cloneStringList");
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
			clone_string = (SIP_S8bit *)STRDUPACCESSOR(temp_string);
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
	SIPDEBUGFN("Exiting function __sip_cloneStringList");
	return SipSuccess;
}


/*********************************************************
** FUNCTION:__sip_cloneChallenge
**
** DESCRIPTION:  This function makes a deep copy of the fileds
** from the GenericChallenge structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneChallenge
# ifdef ANSI_PROTO
	(SipGenericChallenge *dest,
	SipGenericChallenge *src,
	SipError *err)
# else
	( dest, src, err )
	SipGenericChallenge *dest;
	SipGenericChallenge *src;
	SipError *err;
#endif
{
	SIP_U32bit dLength;

	SIPDEBUGFN( "Entering __sip_cloneChallenge");
	/* freeing the field s of the dest structure */
	if ( dest->pScheme != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(dest->pScheme), err) == SipFail )
			return SipFail;
	}

	if ( src->pScheme == SIP_NULL)
		dest->pScheme = src->pScheme;
	else
	{
		dLength = strlen(src->pScheme);
		dest->pScheme = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, dLength+1,err);
		if ( dest->pScheme == SIP_NULL )
			return SipFail;
		strcpy(dest->pScheme,src->pScheme);
	}

	/* copying siplist of auth pParam */
	if ( sip_listDeleteAll(&( dest->slParam ), err ) == SipFail )
		return SipFail;
	if(__sip_cloneSipParamList(&(dest->slParam),&(src->slParam),err)==SipFail)
		return SipFail;

	SIPDEBUGFN( "Exiting __sip_cloneChallenge");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneCredential
**
** DESCRIPTION:  This function makes a deep copy of the fileds
** from the GenericCredential structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneCredential
# ifdef ANSI_PROTO
	(SipGenericCredential *to,
	SipGenericCredential *from,
	SipError *err)
# else
	( to,from, err )
	SipGenericCredential *to;
	SipGenericCredential *from;
	SipError *err;
#endif
{
	SIP_U16bit dLength;

	SIPDEBUGFN( "Entering __sip_cloneCredential");

	if((from==SIP_NULL)||(to==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	/* Cleaning up to */
	if(to->dType == SipCredBasic)
	{
		if(to->u.pBasic!=SIP_NULL)
		{
			if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)&(to->u.pBasic),err)==SipFail)
				return SipFail;
		}
	}
	else if(to->dType == SipCredAuth)
	{
		if(to->u.pChallenge != SIP_NULL)
			sip_freeSipGenericChallenge(to->u.pChallenge);
	}
	/* Cleaned up to */
	to->dType=from->dType;
	if(from->dType==SipCredBasic)
	{
		if(from->u.pBasic== SIP_NULL)
			to->u.pBasic=SIP_NULL;
		else
		{
			dLength = strlen(from->u.pBasic );
			to->u.pBasic=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->u.pBasic == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->u.pBasic, from->u.pBasic );
		}
	}
	else if(from->dType==SipCredAuth)
		{
			if(from->u.pChallenge == SIP_NULL)
				to->u.pChallenge = SIP_NULL;
			else
			{
				if(sip_initSipGenericChallenge(&(to->u.pChallenge),\
					err)==SipFail)
					return SipFail;
				if(__sip_cloneChallenge(to->u.pChallenge,from->u.pChallenge,\
					err) ==SipFail)
				{
					sip_freeSipGenericChallenge(to->u.pChallenge);
					return SipFail;
				}
			}
		}
	else
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	SIPDEBUGFN( "Exiting __sip_cloneCredential");

	*err=E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneSipUrl
**
** DESCRIPTION:  This function makes a deep copy of the fileds
** from the Url structures "source" to "dest".
**
**********************************************************/

SipBool __sip_cloneSipUrl
# ifdef ANSI_PROTO
	(SipUrl *to,
	SipUrl *from,
	SipError *err)
# else
	( to,from, err )
	SipUrl *to;
	SipUrl *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIP_U8bit mem_flag = 0;

	SIPDEBUGFN( "Entering __sip_cloneSipUrl");
	do
	{

	if((from==SIP_NULL)||(to==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	/* Cleaning up to */
	if(to->pUser!=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(to->pUser),err)==SipFail)
			return SipFail;
	if(to->pPassword!=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(to->pPassword),err)==SipFail)
			return SipFail;
	if(to->pHost!=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(to->pHost),err)==SipFail)
			return SipFail;
	if(to->pHeader!=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(to->pHeader),err)==SipFail)
			return SipFail;
	if(to->dPort!=SIP_NULL)
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(to->dPort),err)==SipFail)
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

		to->pUser=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pUser == SIP_NULL )
		{
			*err = E_NO_MEM;
			mem_flag = 1;break;
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

		to->pPassword=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pPassword == SIP_NULL )
		{
			*err = E_NO_MEM;
			mem_flag = 1;break;
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

		to->pHost=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pHost == SIP_NULL )
		{
			*err = E_NO_MEM;
			mem_flag = 1;break;
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

		to->dPort=( SIP_U16bit * )fast_memget(ACCESSOR_MEM_ID,dLength,err);
		if ( to->dPort == SIP_NULL )
		{
			*err = E_NO_MEM;
			mem_flag = 1;break;
		}
		*to->dPort=*from->dPort;
	}

	/* copying siplist of Url pParam */
	if ( sip_listDeleteAll(&( to->slParam ), err ) == SipFail )
		return SipFail;
	if(__sip_cloneSipParamList(&(to->slParam), &(from->slParam), err)==SipFail)
		return SipFail;

	if(from->pHeader== SIP_NULL)
		to->pHeader=SIP_NULL;
	else
	{
		dLength = strlen(from->pHeader );

		to->pHeader=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pHeader == SIP_NULL )
		{
			*err = E_NO_MEM;
			mem_flag = 1;break;
		}
		strcpy( to->pHeader, from->pHeader );
	}

	}
	while(0);
	if( mem_flag )
	{
		sip_freeSipUrl(to);
        	return SipFail;
	}

	SIPDEBUGFN( "Exiting __sip_cloneSipUrl");

	*err=E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneAddrSpec
**
** DESCRIPTION:  This function makes a deep copy of the fileds
** from the AddrSpec structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneAddrSpec
# ifdef ANSI_PROTO
	(SipAddrSpec *to,
	SipAddrSpec *from,
	SipError *err)
# else
	( to,from, err )
	SipAddrSpec *to;
	SipAddrSpec *from;
	SipError *err;
#endif
{
	SIP_U16bit dLength;

	SIPDEBUGFN( "Entering __sip_cloneAddrSpec");

	if((from==SIP_NULL)||(to==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	/* Cleaning up to */
	if(to->dType==SipAddrReqUri)
	{
		if(to->u.pUri!=SIP_NULL)
		{
			if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)&(to->u.pUri),err)==SipFail)
				return SipFail;
		}
	}
	else if((to->dType==SipAddrSipUri) || (to->dType==SipAddrSipSUri))
	{
		if(to->u.pSipUrl!=SIP_NULL)
			sip_freeSipUrl(to->u.pSipUrl);
	}
	/* Cleaned up to */
	if(from->dType==SipAddrReqUri)
	{
		to->dType=from->dType;
		if(from->u.pUri== SIP_NULL)
			to->u.pUri=SIP_NULL;
		else
		{
			dLength = strlen(from->u.pUri );
			to->u.pUri=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->u.pUri == SIP_NULL )
			{
				*err = E_NO_MEM;
			return SipFail;
			}
			strcpy( to->u.pUri, from->u.pUri );
		}
	}
	else if((from->dType==SipAddrSipUri) || (from->dType==SipAddrSipSUri))
	{
		to->dType=from->dType;
		if(sip_initSipUrl(&(to->u.pSipUrl),err)==SipFail)
		{
			return SipFail;
		}
		if ( __sip_cloneSipUrl(to->u.pSipUrl,from->u.pSipUrl,err) == SipFail )
		{
			sip_freeSipUrl(to->u.pSipUrl);
			return SipFail;
		}
	}
	else
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	SIPDEBUGFN( "Exiting __sip_cloneAddrSpec");

	*err=E_NO_ERROR;
	return SipSuccess;

}
/*********************************************************
** FUNCTION:__sip_cloneDateFormat
**
** DESCRIPTION:   This function makes a deep copy of the fileds
** from the DateFormat structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneDateFormat
# ifdef ANSI_PROTO
	(SipDateFormat *dest, SipDateFormat *src,SipError *err)
# else
	(dest,src,err )
	  SipDateFormat *dest;
	  SipDateFormat *src;
	  SipError *err;
# endif
{
	SipError *dummy;
	dummy=err;
	SIPDEBUGFN( "Entering __sip_cloneDateFormat");
	dest->dDay = src->dDay;
	dest->dMonth = src->dMonth;
	dest->dYear = src->dYear;
	SIPDEBUGFN( "Exiting __sip_cloneDateFormat");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneTimeFormat
**
** DESCRIPTION:  This function makes a deep copy of the fileds
** from the TimeFormat structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneTimeFormat
# ifdef ANSI_PROTO
	(SipTimeFormat * dest, SipTimeFormat *src,SipError *err)
# else
	( dest,src,err )
	  SipTimeFormat *dest;
	  SipTimeFormat *src;
	  SipError *err;
# endif
{
	SipError *dummy;
	dummy=err;
	SIPDEBUGFN( "Entering __sip_cloneTimeFormat");
	dest->dHour = src->dHour;
	dest->dMin = src->dMin;
	dest->dSec = src->dSec;
	SIPDEBUGFN( "Exiting __sip_cloneTimeFormat");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneDateStruct
**
** DESCRIPTION: This function makes a deep copy of the fileds
** from the DateStruct structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneDateStruct
# ifdef ANSI_PROTO
	(SipDateStruct * dest, SipDateStruct *src,SipError *err)
# else
	( dest,src,err )
	  SipDateStruct *dest;
	  SipDateStruct *src;
	  SipError *err;
# endif
{
	SIPDEBUGFN( "Entering __sip_cloneDateStruct");
	dest->dDow = src->dDow;
	if (dest->pDate==SIP_NULL)
		sip_initSipDateFormat(&dest->pDate,err);
	if (dest->pTime==SIP_NULL)	
		sip_initSipTimeFormat(&dest->pTime,err);
	__sip_cloneDateFormat( dest->pDate,src->pDate, err );
	__sip_cloneTimeFormat( dest->pTime,src->pTime, err );
	SIPDEBUGFN( "Exiting __sip_cloneDateStruct");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneExpires
**
** DESCRIPTION:  This function makes a deep copy of the fileds
** from the Expires structures "source" to "dest".
**
**********************************************************/
SipBool __sip_cloneExpires
# ifdef ANSI_PROTO
	(SipExpiresStruct *to,
	SipExpiresStruct *from,
	SipError *err)
# else
	( to, from, err )
	SipExpiresStruct *to;
	SipExpiresStruct *from;
	SipError *err;
#endif
{
	SIPDEBUGFN( "Entering __sip_cloneExpires");

	if((from==SIP_NULL)||(to==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(from->dType== SipExpDate)
	{
		if (from->u.pDate!=SIP_NULL)
		{	
			if (sip_initSipDateStruct(&(to->u.pDate),err)==SipFail)
				return SipFail;
			to->dType=from->dType;
			__sip_cloneDateStruct(to->u.pDate,from->u.pDate,err);
		}	
	}
	else if(from->dType== SipExpSeconds)
	{
		to->dType=from->dType;
		to->u.dSec=from->u.dSec;
	}
	else
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	SIPDEBUGFN( "Exiting __sip_cloneExpires");

	*err=E_NO_ERROR;
	return SipSuccess;
}

/**************************************************************
** FUNCTION:__sip_cloneSipDateFormat
**
** DESCRIPTION: This function makes a deep copy of the fileds
** from the DateFormat structures "source" to "dest".
**
**************************************************************/

SipBool __sip_cloneSipDateFormat
# ifdef ANSI_PROTO
	(SipDateFormat * dest, SipDateFormat *src, SipError * err)
# else
	( dest,src , err)
	  SipDateFormat *dest;
	  SipDateFormat *src;
	  SipError	*err;
# endif
{
	SipError *dummy=err;
	dummy=err;
	SIPDEBUGFN ( "Entering __sip_cloneSipDateFormat");
	dest->dDay = src->dDay;
	dest->dMonth = src->dMonth;
	dest->dYear = src->dYear;
	SIPDEBUGFN ( "Exiting __sip_cloneSipDateFormat");
	return SipSuccess;
}

/**************************************************************
** FUNCTION:__sip_cloneSipTimeFormat
**
** DESCRIPTION: This function makes a deep copy of the fileds
** from the TimeFormat structures "source" to "dest".
**
**************************************************************/

SipBool __sip_cloneSipTimeFormat
# ifdef ANSI_PROTO
	(SipTimeFormat * dest, SipTimeFormat *src, SipError * err)
# else
	( dest,src ,err)
	  SipTimeFormat *dest;
	  SipTimeFormat *src;
	  SipError	*err;
# endif
{
	SipError *dummy=err;
	dummy=err;
	SIPDEBUGFN ( "Entering __sip_cloneSipTimeFormat");
	dest->dHour = src->dHour;
	dest->dMin = src->dMin;
	dest->dSec = src->dSec;
	SIPDEBUGFN ( "Exiting __sip_cloneSipTimeFormat");
	return SipSuccess;
}

/**************************************************************
** FUNCTION:__sip_cloneSipDateStruct
**
** DESCRIPTION: This function makes a deep copy of the fileds
** from the DateStruct structures "source" to "dest".
**
**************************************************************/

SipBool __sip_cloneSipDateStruct
# ifdef ANSI_PROTO
	(SipDateStruct * dest, SipDateStruct *src, SipError * err)
# else
	( dest,src , err)
	  SipDateStruct *dest;
	  SipDateStruct *src;
	  SipError 	*err;
# endif
{
	SIPDEBUGFN ( "Entering __sip_cloneSipDateStuct");
	if(dest->pDate != SIP_NULL)
		sip_freeSipDateFormat(dest->pDate);
	if(dest->pTime != SIP_NULL)
		sip_freeSipTimeFormat(dest->pTime);
	dest->dDow = src->dDow;
	if(sip_initSipDateFormat( &(dest->pDate), err) == SipFail)
		return SipFail;
	if(__sip_cloneSipDateFormat( dest->pDate, src->pDate ,err)==SipFail)
	{
		sip_freeSipDateFormat(dest->pDate);
		return SipFail;
	}
	if(sip_initSipTimeFormat( &(dest->pTime), err) == SipFail)
	{
		sip_freeSipDateFormat(dest->pDate);
		return SipFail;
	}
	if(__sip_cloneSipTimeFormat( dest->pTime, src->pTime ,err)==SipFail)
	{
		sip_freeSipTimeFormat(dest->pTime);
		sip_freeSipDateFormat(dest->pDate);
		return SipFail;
	}
	SIPDEBUGFN ( "Exiting __sip_cloneSipDateStruct");
	return SipSuccess;
}

/**************************************************************
** FUNCTION:__sip_cloneSipChallenge
**
** DESCRIPTION: This function makes a deep copy of the fileds
** from the Generaic Challenge structures "source" to "dest".
**
**************************************************************/

SipBool __sip_cloneSipChallenge
# ifdef ANSI_PROTO
	(SipGenericChallenge * dest, SipGenericChallenge *src,SipError * err)
# else
	( dest,src,err )
	  SipGenericChallenge 	*dest;
	  SipGenericChallenge 	*src;
	  SipError 		*err;
# endif
{
	SIP_U32bit dLength;

	SIPDEBUGFN ( "Entering __sip_cloneSipChallenge");

	/* freeing the fields of the dest structure */
	if ( dest->pScheme != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pScheme)), err) == SipFail )
			return SipFail;
	}

	if ( src->pScheme == SIP_NULL)
		dest->pScheme = src->pScheme;
	else
	{
		dLength = strlen(src->pScheme);
		dest->pScheme = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, dLength+1,err);
		if ( dest->pScheme == SIP_NULL )
			return SipFail;
		strcpy(dest->pScheme,src->pScheme);
	}

	/* freeing siplist of auth pParam */
	if ( sip_listDeleteAll(&( dest->slParam ), err ) == SipFail )
		return SipFail;
	if(__sip_cloneSipParamList(&(dest->slParam), &(src->slParam), err)==SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exiting __sip_cloneSipChallenge");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************************
** FUNCTION:__sip_cloneSdpTime
**
** DESCRIPTION: This function makes a deep copy of the fileds
** from the SdpTime structures "source" to "dest".
**
*******************************************************************************/
SipBool __sip_cloneSdpTime
# ifdef ANSI_PROTO
	( SdpTime 	*dest,
	  SdpTime 	*src,
	  SipError 		*err)
# else
	( dest, src, err)
	  SdpTime 	*dest;
	  SdpTime 	*src;
	  SipError 		*err;
# endif
{
	SIP_U32bit dLength,size,index;
	SIP_S8bit * element_in_list, * dup_element_in_list;
	SipError temp_err;
	SIP_U8bit mem_flag = 0;


	SIPDEBUGFN ( "Entering __sip_cloneTime");
	do
	{


	if ( dest->pStart != SIP_NULL )
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pStart)), err ) == SipFail )
			return SipFail;
	if ( src->pStart == SIP_NULL )
		dest->pStart = SIP_NULL;
	else
	{
		dLength = strlen(src->pStart);

		dest->pStart = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pStart == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pStart, src->pStart);
	}

	if ( dest->pStop != SIP_NULL )
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pStop)), err ) == SipFail )
			return SipFail;
	if ( src->pStop == SIP_NULL )
		dest->pStop = SIP_NULL;
	else
	{
		dLength = strlen(src->pStop);

		dest->pStop = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pStop == SIP_NULL )
			{mem_flag = 1;break;}

		strcpy(dest->pStop, src->pStop);
	}
	if ( dest->pZone != SIP_NULL )
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pZone)), err ) == SipFail )
			return SipFail;
	if ( src->pZone == SIP_NULL )
		dest->pZone = SIP_NULL;
	else
	{
		dLength = strlen(src->pZone);
		dest->pZone = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pZone == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pZone, src->pZone);
	}
	/* cloning slRepeat here */
	if( sip_listDeleteAll(&(dest->slRepeat), err) == SipFail)
		return SipFail;
	if( sip_listSizeOf(&(src->slRepeat),&size, err) == SipFail)
		return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		if ( sip_listGetAt(&(src->slRepeat), index,\
			(SIP_Pvoid *)(&element_in_list), err) == SipFail)
			return SipFail;
		if ( element_in_list == SIP_NULL)
			dup_element_in_list = SIP_NULL;
		else
		{
			dLength = strlen(element_in_list);
			dup_element_in_list = ( SIP_S8bit * ) fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if (dup_element_in_list == SIP_NULL )
				{mem_flag = 1;break;}
			strcpy(dup_element_in_list, element_in_list);
		}
		if( sip_listAppend( &(dest->slRepeat),(SIP_Pvoid)dup_element_in_list,\
			err) == SipFail)
		{
			if ( dup_element_in_list != SIP_NULL)
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&dup_element_in_list), &temp_err);
			return SipFail;
		}
	}
	if( mem_flag ) break;
	} while(0);
	if( mem_flag )
	{
		sip_freeSdpTime( dest );
        	return SipFail;
	}


	SIPDEBUGFN ( "Exiting __sip_cloneTime");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************************
** FUNCTION:__sip_cloneSdpMedia
**
** DESCRIPTION: This function copies an SDP slMedia structure into another
**
******************************************************************************/
SipBool __sip_cloneSdpMedia
# ifdef ANSI_PROTO
	( SdpMedia 	*dest,
	  SdpMedia 	*src,
	  SipError 	*err)
# else
	( dest, src, err)
	  SdpMedia 	*dest;
	  SdpMedia 	*src;
	  SipError 	*err;
# endif
{
	SIP_U32bit dLength,size,index;
	SdpConnection *element_from_src, *clone_connection;
	SdpAttr *element_from_src_attr, *clone_attr;

#ifdef SIP_ATM
	SipNameValuePair *pElement_from_src_fmt, *pClone_fmt;
#endif
	SIP_U8bit mem_flag = 0;

	SIPDEBUGFN ( "Entering __sip_cloneMedia");
	do
	{


	/* cloning pInformation */
	if ( dest->pInformation != SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)(&(dest->pInformation)),err) == SipFail)
			return SipFail;

	if ( src->pInformation == SIP_NULL )
		dest->pInformation = SIP_NULL;
	else
	{
		dLength = strlen(src->pInformation);
		dest->pInformation = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pInformation == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pInformation, src->pInformation);
	}

	/* cloning pBandwidth */
	if( sip_listDeleteAll(&(dest->slBandwidth),err) == SipFail)
		return SipFail;
	if  (__sip_cloneSipStringList (&(dest->slBandwidth), &(src->slBandwidth), err) == SipFail)
		return SipFail;

	/* cloning pKey */
	if ( dest->pKey != SIP_NULL)
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pKey)), err ) == SipFail)
			return SipFail;
	if ( src->pKey == SIP_NULL )
		dest->pKey = SIP_NULL;
	else
	{
		dLength = strlen(src->pKey);
		dest->pKey = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pKey == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pKey, src->pKey);
	}
	/* cloning pFormat */
	if ( dest->pFormat != SIP_NULL)
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pFormat)), err ) == SipFail)
			return SipFail;
	if ( src->pFormat == SIP_NULL )
		dest->pFormat = SIP_NULL;
	else
	{
		dLength = strlen(src->pFormat);
		dest->pFormat = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pFormat == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pFormat, src->pFormat);
	}
	/* cloning pProtocol */
	if ( dest->pProtocol != SIP_NULL)
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pProtocol)), err ) == SipFail)
			return SipFail;
	if ( src->pProtocol == SIP_NULL )
		dest->pProtocol = SIP_NULL;
	else
	{
		dLength = strlen(src->pProtocol);
		dest->pProtocol = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pProtocol == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pProtocol, src->pProtocol);
	}
	/* cloning pMediaValue */
	if ( dest->pMediaValue != SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid *)(&(dest->pMediaValue)),err) == SipFail)
			return SipFail;

	if ( src->pMediaValue == SIP_NULL )
		dest->pMediaValue = SIP_NULL;
	else
	{
		dLength = strlen(src->pMediaValue);

		dest->pMediaValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pMediaValue == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pMediaValue, src->pMediaValue);
	}
	/* cloning dPort */
	dest->dPort = src->dPort;
	/* cloning pPortNum */
	if ( dest->pPortNum != SIP_NULL)
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pPortNum)), err ) == SipFail)
			return SipFail;
	if ( src->pPortNum == SIP_NULL )
		dest->pPortNum = SIP_NULL;
	else
	{
		dest->pPortNum = ( SIP_U32bit * )fast_memget(ACCESSOR_MEM_ID,sizeof(SIP_U32bit), err);
		if ( dest->pPortNum == SIP_NULL )
			{mem_flag = 1;break;}
		*(dest->pPortNum) = *(src->pPortNum);
	}
	/* cloning slConnection */

	if ( sip_listDeleteAll(&(dest->slConnection), err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf(&(src->slConnection), &size, err) == SipFail)
		return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		if( sip_listGetAt(&(src->slConnection),index, (SIP_Pvoid * ) (&element_from_src), err) == SipFail )
		{
			return SipFail;
		}
		if (element_from_src == SIP_NULL)
			clone_connection = SIP_NULL;
		else
		{
			if ( sip_initSdpConnection(&clone_connection, err) == SipFail)
				return SipFail;
			if (__sip_cloneSdpConnection ( clone_connection, element_from_src, err) == SipFail)
			{
				sip_freeSdpConnection(clone_connection);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slConnection), (SIP_Pvoid *)clone_connection, err) == SipFail)
		{
			if (clone_connection != SIP_NULL)
				sip_freeSdpConnection (clone_connection);
			return SipFail;
		}

	}


	/* attribute */
	/* if ( sip_listDeleteAll(&(dest->slAttr), err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf(&(src->slAttr), &size, err) == SipFail)
		return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		if(sip_initSdpAttr(&element_from_src_attr,err) == SipFail)
			return SipFail;
		if ( sdp_getAttrAtIndexFromMedia( src, element_from_src_attr, index,\
			err ) == SipFail)
			return SipFail;
		if ( sip_listAppend( &(dest->slAttr), element_from_src_attr, err )\
			== SipFail)
			return SipFail;
	} */
	if ( sip_listDeleteAll(&(dest->slAttr), err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf(&(src->slAttr), &size, err) == SipFail)
		return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		if( sip_listGetAt(&(src->slAttr),index, (SIP_Pvoid * ) (&element_from_src_attr), err) == SipFail )
		{
			return SipFail;
		}
		if (element_from_src_attr == SIP_NULL)
			clone_attr = SIP_NULL;
		else
		{
			if ( sip_initSdpAttr (&clone_attr, err) == SipFail)
				return SipFail;
			if (__sip_cloneSdpAttr (clone_attr, element_from_src_attr, err) == SipFail)
			{
				sip_freeSdpAttr(clone_attr);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slAttr), (SIP_Pvoid *)clone_attr, err) == SipFail)
		{
			if (clone_attr != SIP_NULL)
				sip_freeSdpAttr (clone_attr);
			return SipFail;
		}

	}
	if( sip_listDeleteAll(&(dest->slIncorrectLines),err) == SipFail)
		return SipFail;
	if  (__sip_cloneSipStringList (&(dest->slIncorrectLines), \
		&(src->slIncorrectLines), err) == SipFail)
		return SipFail;

#ifdef SIP_ATM
	/* cloning pVirtualCID */
	if ( dest->pVirtualCID != SIP_NULL)
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pVirtualCID)), err ) == SipFail)
			return SipFail;
	if ( src->pVirtualCID == SIP_NULL )
		dest->pVirtualCID = SIP_NULL;
	else
	{
		dLength = strlen(src->pVirtualCID);
		dest->pVirtualCID = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pVirtualCID == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pVirtualCID, src->pVirtualCID);
	}
	/*Proto Fmt List */
	if ( sip_listDeleteAll(&(dest->slProtofmt), err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf(&(src->slProtofmt), &size, err) == SipFail)
		return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		if( sip_listGetAt(&(src->slProtofmt),index, (SIP_Pvoid * ) (&pElement_from_src_fmt), err) == SipFail )
		{
			return SipFail;
		}
		if (pElement_from_src_fmt == SIP_NULL)
			pClone_fmt = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pClone_fmt, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pClone_fmt, pElement_from_src_fmt, err) == SipFail)
			{
				sip_freeSipNameValuePair(pClone_fmt);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slProtofmt), (SIP_Pvoid *)pClone_fmt, err) == SipFail)
		{
			if (pClone_fmt != SIP_NULL)
				sip_freeSipNameValuePair (pClone_fmt);
			return SipFail;
		}

	}
#endif
	} while(0);
	if( mem_flag )
	{
		sip_freeSdpMedia (dest);
        	return SipFail;
	}


	SIPDEBUGFN ( "Exiting __sip_cloneMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************************
** FUNCTION:__sip_cloneSdpConnection
**
** DESCRIPTION: This function copies an SDP Connection structure into
**		another
**
******************************************************************************/
 SipBool __sip_cloneSdpConnection
# ifdef ANSI_PROTO
	( SdpConnection 	*dest,
	  SdpConnection 	*src,
	  SipError 		*err)
# else
	( dest, src, err)
	  SdpConnection 	*dest;
	  SdpConnection 	*src;
	  SipError 		*err;
# endif
{
	SIP_U32bit dLength;
	SIP_U8bit mem_flag = 0;


	SIPDEBUGFN ( "Entering __sip_cloneSdpConnection");
	do
	{

	/* freeing fileds of destination */
	if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pNetType)), err ) == SipFail )
		return SipFail;

	if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pAddrType)), err ) == SipFail )
		return SipFail;

	if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->dAddr)), err ) == SipFail )
		return SipFail;

	if ( src->pNetType == SIP_NULL )
		dest->pNetType = SIP_NULL;
	else
	{
		dLength = strlen(src->pNetType);
		dest->pNetType = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pNetType == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pNetType, src->pNetType);
	}

	if ( src->pAddrType == SIP_NULL )
		dest->pAddrType = SIP_NULL;
	else
	{
		dLength = strlen(src->pAddrType);

		dest->pAddrType = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pAddrType == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pAddrType, src->pAddrType);
	}

	if ( src->dAddr == SIP_NULL )
		dest->dAddr = SIP_NULL;
	else
	{
		dLength = strlen(src->dAddr);
		dest->dAddr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->dAddr == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->dAddr, src->dAddr);
	}
	} while(0);
	if( mem_flag )
	{
		sip_freeSdpConnection(dest);
        	return SipFail;
	}

	SIPDEBUGFN ( "Exiting __sip_cloneSdpConnection");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************************
** FUNCTION:__sip_cloneSdpAttr
**
** DESCRIPTION: This function copies an SDP attribute structure into
**		another SDP attribute structure
**
******************************************************************************/
SipBool __sip_cloneSdpAttr
# ifdef ANSI_PROTO
	( SdpAttr 	*dest,
	  SdpAttr 	*src,
	  SipError 	*err)
# else
	( dest, src, err)
	  SdpAttr 	*dest;
	  SdpAttr 	*src;
	  SipError 	*err;
# endif
{
	SIP_U32bit dLength;
	SIP_U8bit mem_flag = 0;

	SIPDEBUGFN ( "Entering __sip_cloneSdpAttr");
	do
	{

	/* freeing the fieds of destination */
	if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pName)), err ) == SipFail)
		return SipFail;

	if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(dest->pValue)), err ) == SipFail)
		return SipFail;

	if ( src->pName == SIP_NULL )
		dest->pName = SIP_NULL;
	else
	{
		dLength = strlen(src->pName);
		dest->pName = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pName == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pName, src->pName);
	}
	if ( src->pValue == SIP_NULL )
		dest->pValue = SIP_NULL;
	else
	{
		dLength = strlen(src->pValue);
		dest->pValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pValue == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pValue, src->pValue);
	}
	} while(0);
	if( mem_flag )
	{
		sip_freeSdpAttr(dest);
        	return SipFail;
	}


	SIPDEBUGFN ( "Exiting __sip_cloneSdpAttr");

	*err = E_NO_ERROR;
	return SipSuccess;
}
#ifdef SIP_MWI
/*****************************************************************************
** FUNCTION:__sip_mwi_cloneSummaryLine
**
** DESCRIPTION:
**
**
******************************************************************************/
SipBool __sip_mwi_cloneSummaryLine
# ifdef ANSI_PROTO
        (SummaryLine * dest, SummaryLine *src, SipError *err)
# else
        ( dest,src,err )
                SummaryLine *dest;
                SummaryLine *src;
                SipError *err;

# endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN("Entering __sip_mwi_cloneSummaryLine");

	/* Copying pMedia */
        if (dest->pMedia !=SIP_NULL)
                if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pMedia), err)==SipFail)
                        return SipFail;
        if ( src->pMedia == SIP_NULL )
                dest->pMedia = SIP_NULL;
        else
        {
                dLength = strlen(src->pMedia);
                dest->pMedia = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
                if ( dest->pMedia == SIP_NULL )
                {
                        return SipFail;
                }
                strcpy(dest->pMedia, src->pMedia);
        }

	/* copy other values */
	dest->newMessages = src->newMessages;
	dest->oldMessages = src->oldMessages;
	dest->newUrgentMessages = src->newUrgentMessages;
	dest->oldUrgentMessages = src->oldUrgentMessages;

	SIPDEBUGFN("Exiting __sip_mwi_cloneSummaryLine");
	*err = E_NO_ERROR;
	return SipSuccess;

}
#endif
/*****************************************************************************
** FUNCTION:__sip_cloneSipNameValuePair
**
** DESCRIPTION:
**
**
******************************************************************************/
SipBool __sip_cloneSipNameValuePair
# ifdef ANSI_PROTO
        (SipNameValuePair * dest, SipNameValuePair *src, SipError *err)
# else
        ( dest,src,err )
                SipNameValuePair *dest;
                SipNameValuePair *src;
                SipError *err;

# endif
{
        SIP_U32bit dLength;
	SIP_U8bit mem_flag = 0;
	SIPDEBUGFN("Entering __sip_cloneSipNameValuePair");
	do
	{

        /* Copying pName */
        if (dest->pName !=SIP_NULL)
                if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pName), err)==SipFail)
                        return SipFail;
        if ( src->pName == SIP_NULL )
                dest->pName = SIP_NULL;
        else
        {
                dLength = strlen(src->pName);
                dest->pName = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
                if ( dest->pName == SIP_NULL )
                {
			{mem_flag = 1;break;}
                }
                strcpy(dest->pName, src->pName);
        }

	/* Copying pValue */
        if (dest->pValue !=SIP_NULL)
                if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pValue), err)==SipFail)
                        return SipFail;
        if ( src->pValue == SIP_NULL )
                dest->pValue = SIP_NULL;
        else
        {
                dLength = strlen(src->pValue);
                dest->pValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
                if ( dest->pValue == SIP_NULL )
                {
			{mem_flag = 1;break;}
                }
                strcpy(dest->pValue, src->pValue);
        }
	} while(0);
	if( mem_flag )
	{
		sip_freeSipNameValuePair( dest );
        	return SipFail;
	}
	SIPDEBUGFN("Exiting __sip_cloneSipNameValuePair");
	*err = E_NO_ERROR;
	return SipSuccess;

}
#ifdef SIP_MWI
/*****************************************************************************
** FUNCTION:__sip_mwi_cloneMesgSummaryMessage
**
** DESCRIPTION:
**
**
******************************************************************************/
SipBool __sip_mwi_cloneMesgSummaryMessage
# ifdef ANSI_PROTO
        (MesgSummaryMessage * dest, MesgSummaryMessage *src, SipError *err)
# else
        ( dest,src,err )
                MesgSummaryMessage *dest;
                MesgSummaryMessage *src;
                SipError *err;

# endif
{
    SIP_U32bit size,index;
	SIP_Pvoid element_from_src;
	SummaryLine *clone_summaryline;
	SipNameValuePair *clone_namevalue;
	SIPDEBUGFN("Entering __sip_mwi_cloneMesgSummaryMessage");

	if((src == SIP_NULL)||(dest==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
    /* copy status */

	dest->dStatus = src->dStatus;

    /* cloning Message-Account field */
    if(src->pAddrSpec != SIP_NULL)
    {
        
   	if (dest->pAddrSpec != SIP_NULL)
    {
        sip_freeSipAddrSpec(dest->pAddrSpec);
    }

	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (src->pAddrSpec)->dType,\
		 err) == SipFail)
    {
		return SipFail;
    }

	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, src->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}
    }
    
    /* cloning slSummaryLine */

 	if ( sip_listDeleteAll(&(dest->slSummaryLine), err) == SipFail)
                return SipFail;
        if ( sip_listSizeOf(&(src->slSummaryLine), &size, err) == SipFail)
                return SipFail;
        for ( index = 0; index < size ; index++ )
        {
			element_from_src = SIP_NULL;
                if( sip_listGetAt(&(src->slSummaryLine),index, (SIP_Pvoid * )\
			 (&element_from_src), err) == SipFail )
                {
                        return SipFail;
                }
                if (element_from_src == SIP_NULL)
                        clone_summaryline = SIP_NULL;
                else
                {
                        if ( sip_mwi_initSummaryLine(&clone_summaryline, err) == SipFail)
                                return SipFail;
                        if ( __sip_mwi_cloneSummaryLine( clone_summaryline, \
				 (SummaryLine *)element_from_src, err) == SipFail)
                        {
                                sip_mwi_freeSummaryLine(clone_summaryline);
                                return SipFail;
                        }
                }
                if (sip_listAppend (&(dest->slSummaryLine), \
			 (SIP_Pvoid *)clone_summaryline, err) == SipFail)
                {
                        if (clone_summaryline != SIP_NULL)
                                sip_mwi_freeSummaryLine (clone_summaryline);
                        return SipFail;
                }

        }
/* cloning slNameValue */

	element_from_src = SIP_NULL;

	if ( sip_listDeleteAll(&(dest->slNameValue), err) == SipFail)
                return SipFail;
        if ( sip_listSizeOf(&(src->slNameValue), &size, err) == SipFail)
                return SipFail;
        for ( index = 0; index < size ; index++ )
        {
			element_from_src = SIP_NULL;
                if( sip_listGetAt(&(src->slNameValue),index, (SIP_Pvoid * )\
                         (&element_from_src), err) == SipFail )
                {
                        return SipFail;
                }
                if (element_from_src == SIP_NULL)
                        clone_namevalue = SIP_NULL;
                else
                {
                        if ( sip_initSipNameValuePair(&clone_namevalue, err) == SipFail)
                                return SipFail;
                        if ( __sip_cloneSipNameValuePair( clone_namevalue, \
                                 (SipNameValuePair *)element_from_src, err) == SipFail)
                        {
                                sip_freeSipNameValuePair(clone_namevalue);
                                return SipFail;
                        }
                }
                if (sip_listAppend (&(dest->slNameValue), \
                         (SIP_Pvoid *)clone_namevalue, err) == SipFail)
                {
                        if (clone_namevalue != SIP_NULL)
                                sip_freeSipNameValuePair(clone_namevalue);
                        return SipFail;
                }

        }
	SIPDEBUGFN("Exiting __sip_mwi_cloneMesgSummaryMessage");
	*err = E_NO_ERROR;
	return SipSuccess;

}
#endif
/*****************************************************************************
** FUNCTION:__sip_cloneSdpMessage
**
** DESCRIPTION: This function copies an SDP message structure into \\
**		another SDP slMedia structure
**
******************************************************************************/
SipBool __sip_cloneSdpMessage
# ifdef ANSI_PROTO
	(SdpMessage * dest, SdpMessage *src, SipError *err)
# else
	( dest,src,err )
		SdpMessage *dest;
		SdpMessage *src;
		SipError *err;

# endif
{
	SIP_U32bit i,dLength,count;
	SdpTime *timeout, *clone_timeout;
	SdpAttr *attrout, *clone_attrout;
	SdpMedia *mediaout, *clone_mediaout;
	SIP_U8bit mem_flag = 0;

	SIPDEBUGFN("Entering __sip_cloneSdpMessage\n");
	do
	{
	if((src == SIP_NULL)||(dest==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	/* Copying pVersion */
	if (dest->pVersion !=SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pVersion), err)==SipFail)
			return SipFail;
	if ( src->pVersion == SIP_NULL )
		dest->pVersion = SIP_NULL;
	else
	{
		dLength = strlen(src->pVersion);
		dest->pVersion = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pVersion == SIP_NULL )
		{
			{mem_flag = 1;break;}
		}
		strcpy(dest->pVersion, src->pVersion);
	}
	/* Copied pVersion */
	/* Copying pSession */
	if (dest->pSession !=SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pSession), err)==SipFail)
			return SipFail;
	if ( src->pSession == SIP_NULL )
		dest->pSession = SIP_NULL;
	else
	{
		dLength = strlen(src->pSession);
		dest->pSession = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pSession == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pSession, src->pSession);
	}
	/* Copied pSession */
	/* Copying pInformation */
	if (dest->pInformation !=SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pInformation), err)==SipFail)
			return SipFail;
	if ( src->pInformation == SIP_NULL )
		dest->pInformation = SIP_NULL;
	else
	{
		dLength = strlen(src->pInformation);
		dest->pInformation = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pInformation == SIP_NULL )
		{
			{mem_flag = 1;break;}
		}
		strcpy(dest->pInformation, src->pInformation);
	}
	/* Copied pInformation */
	/* Copying pUri */
	if (dest->pUri !=SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)&(dest->pUri), err)==SipFail)
			return SipFail;
	if ( src->pUri == SIP_NULL )
		dest->pUri = SIP_NULL;
	else
	{
		dLength = strlen(src->pUri);
		dest->pUri = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pUri == SIP_NULL )
		{
			{mem_flag = 1;break;}
		}
		strcpy(dest->pUri, src->pUri);
	}
	/* Copied pUri */
	/* Copying pKey */
	if (dest->pKey !=SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)&(dest->pKey), err)==SipFail)
			return SipFail;
	if ( src->pKey == SIP_NULL )
		dest->pKey = SIP_NULL;
	else
	{
		dLength = strlen(src->pKey);
		if ( dest->pKey != SIP_NULL )
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pKey, err ) == SipFail )
				return SipFail;
		}

		dest->pKey = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pKey == SIP_NULL )
			{mem_flag = 1;break;}
		strcpy(dest->pKey, src->pKey);
	}
	/* Copied pKey */

	/* Copy pOrigin */
	if (dest->pOrigin !=SIP_NULL)
		sip_freeSdpOrigin(dest->pOrigin);
	if(src->pOrigin==SIP_NULL)
		dest->pOrigin = SIP_NULL;
	else
	{
		if(sip_initSdpOrigin(&(dest->pOrigin),err)==SipFail)
			return SipFail;
		if(__sip_cloneSdpOrigin(dest->pOrigin,src->pOrigin,err)==SipFail)
		{
			sip_freeSdpOrigin(dest->pOrigin);
			return SipFail;
		}
	}

	/* Copy slConnection */
	if (dest->slConnection !=SIP_NULL)
		sip_freeSdpConnection(dest->slConnection);
	if(src->slConnection==SIP_NULL)
	{
		dest->slConnection = SIP_NULL;
	}
	else
	{
		if(sip_initSdpConnection(&(dest->slConnection),err)==SipFail)
			return SipFail;
		if(__sip_cloneSdpConnection(dest->slConnection,src->slConnection,err)\
			==SipFail)
			return SipFail;
	}
	/* Copy all siplists */
	/* Copy slEmail list */
	if( sip_listDeleteAll(&(dest->slEmail),err) == SipFail)
		return SipFail;
	if  (__sip_cloneSipStringList (&(dest->slEmail), &(src->slEmail), err) == SipFail)
		return SipFail;


	/* Copy slPhone list */

	if( sip_listDeleteAll(&(dest->slPhone),err) == SipFail)
		return SipFail;
	if  (__sip_cloneSipStringList (&(dest->slPhone), &(src->slPhone), err) \
		== SipFail)
		return SipFail;

	/* Copy BandWidth list */

	if( sip_listDeleteAll(&(dest->pBandwidth),err) == SipFail)
		return SipFail;
	if  (__sip_cloneSipStringList (&(dest->pBandwidth), &(src->pBandwidth), err) == SipFail)
		return SipFail;

	/* Copy Time list */

	if ( sip_listDeleteAll(&(dest->slTime), err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf(&(src->slTime), &count, err) == SipFail)
		return SipFail;
	for ( i = 0; i < count ; i++ )
	{
		if( sip_listGetAt(&(src->slTime),i, (SIP_Pvoid *) (&timeout), err) == SipFail )
		{
			return SipFail;
		}
		if (timeout == SIP_NULL)
			clone_timeout = SIP_NULL;
		else
		{
			if ( sip_initSdpTime (&clone_timeout, err) == SipFail)
				return SipFail;
			if (__sip_cloneSdpTime (clone_timeout, timeout, err) == SipFail)
			{
				sip_freeSdpTime (clone_timeout);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slTime), (SIP_Pvoid *)clone_timeout, err) == SipFail)
		{
			if (clone_timeout != SIP_NULL)
				sip_freeSdpTime (clone_timeout);
			return SipFail;
		}

	}
	/* Copy Attr list */
/* 	if ( sdp_getAttrCount( src, &count, err ) == SipFail)
		return SipFail;
	if( sip_listDeleteAll(&(dest->slAttr),err) == SipFail)
		return SipFail;
	for(i=0;i< count;i++)
	{
		if(sip_initSdpAttr(&attrout,err)==SipFail)
			return SipFail;
		if ( sdp_getAttrAtIndex( src, attrout, i, err ) == SipFail)
		{
			sip_freeSdpAttr(attrout);
			return SipFail;
		}
		if(sip_listAppend(&(dest->slAttr),attrout, err) == SipFail)
			return SipFail;
	} */
	if ( sip_listDeleteAll(&(dest->slAttr), err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf(&(src->slAttr), &count, err) == SipFail)
		return SipFail;
	for ( i = 0; i < count ; i++ )
	{
		if( sip_listGetAt(&(src->slAttr),i, (SIP_Pvoid *) (&attrout), err) == SipFail )
		{
			return SipFail;
		}
		if (attrout == SIP_NULL)
			clone_attrout = SIP_NULL;
		else
		{
			if ( sip_initSdpAttr (&clone_attrout, err) == SipFail)
				return SipFail;
			if (__sip_cloneSdpAttr (clone_attrout, attrout, err) == SipFail)
			{
				sip_freeSdpAttr (clone_attrout);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slAttr), (SIP_Pvoid *)clone_attrout, err) == SipFail)
		{
			if (clone_attrout != SIP_NULL)
				sip_freeSdpAttr (clone_attrout);
			return SipFail;
		}

	}
	/* Copy Media list */
/* 	if ( sdp_getMediaCount( src, &count, err ) == SipFail)
		return SipFail;
	if( sip_listDeleteAll(&(dest->slMedia),err) == SipFail)
		return SipFail;
	for(i=0;i< count;i++)
	{
		if(sip_initSdpMedia(&mediaout,err)==SipFail)
			return SipFail;
		if ( sdp_getMediaAtIndex( src, mediaout, i, err ) == SipFail)
		{
			sip_freeSdpMedia(mediaout);
			return SipFail;
		}
		if(sip_listAppend(&(dest->slMedia),mediaout, err) == SipFail)
			return SipFail;
	} */
	if ( sip_listDeleteAll(&(dest->slMedia), err) == SipFail)
		return SipFail;
	if ( sip_listSizeOf(&(src->slMedia), &count, err) == SipFail)
		return SipFail;
	for ( i = 0; i < count ; i++ )
	{
		if( sip_listGetAt(&(src->slMedia),i, (SIP_Pvoid *) (&mediaout), err) == SipFail )
		{
			return SipFail;
		}
		if (mediaout == SIP_NULL)
			clone_mediaout = SIP_NULL;
		else
		{
			if ( sip_initSdpMedia (&clone_mediaout, err) == SipFail)
				return SipFail;
			if (__sip_cloneSdpMedia (clone_mediaout, mediaout, err) == SipFail)
			{
				sip_freeSdpMedia (clone_mediaout);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slMedia), (SIP_Pvoid *)clone_mediaout, err) == SipFail)
		{
			if (clone_mediaout != SIP_NULL)
				sip_freeSdpMedia (clone_mediaout);
			return SipFail;
		}

	}
	if( sip_listDeleteAll(&(dest->slIncorrectLines),err) == SipFail)
		return SipFail;
	if  (__sip_cloneSipStringList (&(dest->slIncorrectLines), \
		&(src->slIncorrectLines), err) == SipFail)
		return SipFail;

	} while(0);
	if( mem_flag )
	{
		sip_freeSdpMessage(dest);
        	return SipFail;
	}

	SIPDEBUGFN("Exiting __sip_cloneSdpMessage. Succeeded.\n");
	return SipSuccess;
}

/*****************************************************************************
** FUNCTION:__sip_cloneSdpOrigin
**
** DESCRIPTION:This function copies the contents of an SDP Origin
**		structure into another SDP pOrigin structure
**
******************************************************************************/
SipBool __sip_cloneSdpOrigin
# ifdef ANSI_PROTO
	(SdpOrigin * dest, SdpOrigin *src, SipError *err)
# else
	( dest,src, err )
	  	SdpOrigin *dest;
		SdpOrigin *src;
		SipError *err;
# endif
{
	SIP_U32bit dLength;
	SIP_U8bit mem_flag = 0;

	SIPDEBUGFN("Entering __sip_cloneSdpOrigin\n");
	do
	{

	if((dest==SIP_NULL)||(src == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (dest->pUser !=SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)&(dest->pUser), err)==SipFail)
			return SipFail;
	if ( src->pUser == SIP_NULL )
		dest->pUser = SIP_NULL;
	else
	{
		dLength = strlen(src->pUser);
		if ( dest->pUser != SIP_NULL )
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pUser, err ) == SipFail )
			{
				SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
				return SipFail;
			}
		}

		dest->pUser = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pUser == SIP_NULL )
		{
			SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
			{mem_flag = 1;break;}
		}
		strcpy(dest->pUser, src->pUser);
	}

	if (dest->pSessionid != SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)&(dest->pSessionid), err)==SipFail)
			return SipFail;
	if ( src->pSessionid == SIP_NULL )
		dest->pSessionid = SIP_NULL;
	else
	{
		dLength = strlen(src->pSessionid);
		if ( dest->pSessionid != SIP_NULL )
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pSessionid, err ) == SipFail )
			{
				SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
				return SipFail;
			}
		}

		dest->pSessionid = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pSessionid == SIP_NULL )
		{
			SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
			{mem_flag = 1;break;}
		}
		strcpy(dest->pSessionid, src->pSessionid);
	}

	if (dest->pVersion !=SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pVersion), err)==SipFail)
			return SipFail;
	if ( src->pVersion == SIP_NULL )
		dest->pVersion = SIP_NULL;
	else
	{
		dLength = strlen(src->pVersion);
		if ( dest->pVersion != SIP_NULL )
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pVersion, err ) == SipFail )
			{
				SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
				return SipFail;
			}
		}

		dest->pVersion = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pVersion == SIP_NULL )
		{
			SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
			{mem_flag = 1;break;}
		}
		strcpy(dest->pVersion, src->pVersion);
	}

	if (dest->pNetType != SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pNetType), err)==SipFail)
			return SipFail;
	if ( src->pNetType == SIP_NULL )
		dest->pNetType = SIP_NULL;
	else
	{
		dLength = strlen(src->pNetType);
		if ( dest->pNetType != SIP_NULL )
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pNetType, err ) == SipFail )
			{
				SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
				return SipFail;
			}
		}
		dest->pNetType = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pNetType == SIP_NULL )
		{
			SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
			{mem_flag = 1;break;}
		}
		strcpy(dest->pNetType, src->pNetType);
	}
	if (dest->pAddrType != SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->pAddrType), err)==SipFail)
			return SipFail;
	if ( src->pAddrType == SIP_NULL )
		dest->pAddrType = SIP_NULL;
	else
	{
		dLength = strlen(src->pAddrType);
		if ( dest->pAddrType != SIP_NULL )
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->pAddrType, err ) == SipFail )
			{
				SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
				return SipFail;
			}
		}

		dest->pAddrType = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->pAddrType == SIP_NULL )
		{
			SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
			{mem_flag = 1;break;}
		}
		strcpy(dest->pAddrType, src->pAddrType);
	}

	if (dest->dAddr != SIP_NULL)
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *) &(dest->dAddr), err)==SipFail)
			return SipFail;
	if ( src->dAddr == SIP_NULL )
		dest->dAddr = SIP_NULL;
	else
	{
		dLength = strlen(src->dAddr);
		if ( dest->dAddr != SIP_NULL )
		{
			if ( fast_memfree(ACCESSOR_MEM_ID, dest->dAddr, err ) == SipFail )
			{
				SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
				return SipFail;
			}
		}
		dest->dAddr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength + 1, err);
		if ( dest->dAddr == SIP_NULL )
		{
			SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Failed.\n");
			{mem_flag = 1;break;}
		}
		strcpy(dest->dAddr, src->dAddr);
	}
	} while(0);
	if( mem_flag )
	{
		sip_freeSdpOrigin(dest);
        	return SipFail;
	}


	SIPDEBUGFN("Exiting __sip_cloneSdpOrigin. Succeeded.\n");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipUnknownMessage
**
** DESCRIPTION: This function copies the conmtents of a SIP Unknown
**		message structure into another SIP Unknown message
**		structure
**
******************************************************************/
SipBool __sip_cloneSipUnknownMessage
#ifdef ANSI_PROTO
(SipUnknownMessage *dest, SipUnknownMessage *src, SipError *err)
#else
	(dest, src, err)
	SipUnknownMessage *dest;
	SipUnknownMessage *src;
	SipError *err;
#endif
{
	/* No validation to be done, since function is internal */
	/* free up destination structure */
	if(dest->pBuffer != SIP_NULL)
		sip_freeString(dest->pBuffer);
	dest->dLength = src->dLength;
	dest->pBuffer = (SIP_S8bit *)\
		fast_memget(ACCESSOR_MEM_ID,src->dLength,err);
	if(dest->pBuffer == SIP_NULL)
		return SipFail;
	memcpy(dest->pBuffer,src->pBuffer,src->dLength);

	*err = E_NO_ERROR;
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipMsgBody
**
** DESCRIPTION: This function copies the contents of a SIP Message
**		pBody structure into another SIP Message pBody structure
**
******************************************************************/
SipBool __sip_cloneSipMsgBody
#ifdef ANSI_PROTO
(SipMsgBody *dest, SipMsgBody *src, SipError *err)
#else
	(dest, src, err)
	SipMsgBody *dest;
	SipMsgBody *src;
	SipError *err;
#endif
{
	/* No validation to be done, since function is internal */
	/* free up destination structure */
	if (dest->pMimeHeader != SIP_NULL)
		sip_bcpt_freeSipMimeHeader(dest->pMimeHeader);
	switch(dest->dType)
	{
		case SipSdpBody:
			sip_freeSdpMessage(dest->u.pSdpMessage);
			break;
		case SipUnknownBody:
			sip_freeSipUnknownMessage(dest->u.pUnknownMessage);
			break;
		case SipIsupBody:  /* bcpt ext */
			sip_bcpt_freeIsupMessage(dest->u.pIsupMessage);
			break;
		case SipQsigBody:  /* bcpt ext */
			sip_bcpt_freeQsigMessage(dest->u.pQsigMessage);
			break;
		case SipMultipartMimeBody:/* bcpt ext */
			sip_bcpt_freeMimeMessage(dest->u.pMimeMessage);
			break;
#ifdef SIP_MWI
		case SipMessageSummaryBody: /* SIP_MWI */
			sip_mwi_freeMesgSummaryMessage(dest->u.pSummaryMessage);
			break;
#endif
		case SipAppSipBody:
			sip_freeSipMessage(dest->u.pAppSipMessage);
			break;
		case SipBodyAny:
			break;
		default:
			*err = E_INV_PARAM;
			return SipFail;
	}
	/* set destination's dType to source dType */
	dest->dType = src->dType;
	/* copy src to destination */
	switch(src->dType)
	{
		case SipSdpBody:
			if(sip_initSdpMessage(&(dest->u.pSdpMessage),err)==SipFail)
				return SipFail;
			if(__sip_cloneSdpMessage(dest->u.pSdpMessage, src->u.pSdpMessage,\
				err)==SipFail)
			{
				sip_freeSdpMessage(dest->u.pSdpMessage);
				return SipFail;
			}
			break;
		case SipUnknownBody:
			if(sip_initSipUnknownMessage(&(dest->u.pUnknownMessage),err)\
				==SipFail)
				return SipFail;
			if(__sip_cloneSipUnknownMessage(dest->u.pUnknownMessage,\
				src->u.pUnknownMessage, err)==SipFail)
			{
				sip_freeSipUnknownMessage (dest->u.pUnknownMessage);
				return SipFail;
			}
			break;
		case SipIsupBody:
			if(sip_bcpt_initIsupMessage(&(dest->u.pIsupMessage),err)==SipFail)
				return SipFail;
			if(sip_bcpt_cloneIsupMessage(dest->u.pIsupMessage,\
				src->u.pIsupMessage, err)==SipFail)
			{
				return SipFail;
			}
			break;
		case SipQsigBody:
			if(sip_bcpt_initQsigMessage(&(dest->u.pQsigMessage),err)==SipFail)
				return SipFail;
			if(sip_bcpt_cloneQsigMessage(dest->u.pQsigMessage,\
				src->u.pQsigMessage, err)==SipFail)
			{
				sip_bcpt_freeQsigMessage(dest->u.pQsigMessage);
				return SipFail;
			}
			break;
		case SipMultipartMimeBody:
			if(sip_bcpt_initMimeMessage(&(dest->u.pMimeMessage),err)==SipFail)
				return SipFail;
			if(sip_bcpt_cloneMimeMessage(dest->u.pMimeMessage,\
				src->u.pMimeMessage, err)==SipFail)
			{
				return SipFail;
			}
			break;
#ifdef SIP_MWI
		case SipMessageSummaryBody: /* SIP_MWI */
			if(sip_mwi_initMesgSummaryMessage(&(dest->u.pSummaryMessage),err)==SipFail)
				return SipFail;
			if(__sip_mwi_cloneMesgSummaryMessage(dest->u.pSummaryMessage,\
				src->u.pSummaryMessage, err)==SipFail)
			{
				return SipFail;
			}
			break;
#endif
		case SipAppSipBody:
			if(sip_initSipMessage(&(dest->u.pAppSipMessage),\
				src->u.pAppSipMessage->dType,err)==SipFail)
				return SipFail;
			if(__sip_cloneSipMessage(dest->u.pAppSipMessage,\
				src->u.pAppSipMessage, err)==SipFail)
				return SipFail;
			break;
		default:
			*err = E_INV_PARAM;
			return SipFail;
	}
/* bcpt ext */
	if ( src->pMimeHeader == SIP_NULL)
		dest->pMimeHeader = SIP_NULL;
	else
	{
		if (sip_bcpt_initSipMimeHeader(&(dest->pMimeHeader), err)\
				== SipFail)
				return SipFail;
		if ( sip_bcpt_cloneSipMimeHeader(dest->pMimeHeader,src->pMimeHeader,\
			err) == SipFail)
		{
			sip_bcpt_freeSipMimeHeader (dest->pMimeHeader);
			return SipFail;
		}
	}/* end of else */
/* bcpt ext ends */

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:__sip_cloneSipParam
**
** DESCRIPTION: This function copies the contents of a SIP param
**		structure into another SIP param structure
**
**********************************************************/

SipBool __sip_cloneSipParam
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

	SIPDEBUGFN("Entering function __sip_cloneSipParam");
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
		if( sip_listGetAt(&( pSource->slValue),index, (SIP_Pvoid * )\
			(&pValue), pErr) == SipFail )
			return SipFail;

		if ( pValue == SIP_NULL )
			pDupValue = SIP_NULL;
		else
		{
			pDupValue = (SIP_S8bit *)STRDUPACCESSOR(pValue);
			if ( pDupValue == SIP_NULL)
			{
				*pErr = E_NO_MEM;
				return SipFail;
			}
		}

		if ( sip_listAppend(&( pDest->slValue ), pDupValue, pErr) == SipFail )
		{
			if ( pDupValue != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pDupValue), &tempErr);
			return SipFail;
		}
	}

	/* duplicating pName */
	/* freeing */
	if ( pDest->pName != SIP_NULL)
	{
		sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&pDest->pName), SIP_NULL);
		pDest->pName = SIP_NULL;
	}

	if (pSource->pName == SIP_NULL)
		pDest->pName = SIP_NULL;
	else
	{
		pDest->pName = (SIP_S8bit*)STRDUPACCESSOR(pSource->pName);
		if ( pDest->pName == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipParam");
	return SipSuccess;
}

/********************* Clone APIs Follow **************************/


SipBool sip_cloneSipStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *pDest, SipStatusLine *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipStatusLine *pDest;
	SipStatusLine *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipStatusLine");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/*	if (sip_initSipStatusLine (ppDest, pErr) == SipFail)
		return SipFail; */

	if  (__sip_cloneSipStatusLine (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipStatusLine");
	return SipSuccess;
}


SipBool sip_cloneSipReqLine
#ifdef ANSI_PROTO
	(SipReqLine *pDest, SipReqLine *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipReqLine *pDest;
	SipReqLine *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipReqLine");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipReqLine (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipReqLine (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipReqLine");
	return SipSuccess;
}


SipBool sip_cloneSipAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pDest, SipAddrSpec *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipAddrSpec *pDest;
	SipAddrSpec *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipAddrSpec");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipAddrSpec(ppDest, SipAddrAny, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipAddrSpec (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipAddrSpec");
	return SipSuccess;
}


SipBool sip_cloneSipContactParam
#ifdef ANSI_PROTO
	(SipContactParam *pDest, SipContactParam *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipContactParam *pDest;
	SipContactParam *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipContactParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipContactParam(pDest, SipCParamAny, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipContactParam (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipContactParam");
	return SipSuccess;
}


SipBool sip_cloneSipExpiresStruct
#ifdef ANSI_PROTO
	(SipExpiresStruct *pDest, SipExpiresStruct *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipExpiresStruct *pDest;
	SipExpiresStruct *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipExpiresStruct");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipExpiresStruct(ppDest, SipExpAny, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipExpiresStruct (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipExpiresStruct");
	return SipSuccess;
}


SipBool sip_cloneSipStringList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipStringList");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_listInit(*ppDest, __sip_freeString, pErr) == SipFail)
		return SipFail; */
	if (sip_listDeleteAll (pDest, pErr) == SipFail)
		return SipFail;
	if  (__sip_cloneSipStringList (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipStringList");
	return SipSuccess;
}

/*
SipBool sip_cloneSipAuthParam
#ifdef ANSI_PROTO
	(SipAuthParam **ppDest, SipAuthParam *pSource, SipError *pErr)
#else
	(ppDest, pSource, pErr)
	SipAuthParam	*ppDest;
	SipAuthParam *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneAuthParam");
	if (pErr == SIP_NULL)
		return SipFail;
	if ((ppDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (sip_initSipAuthParam(ppDest, pErr) == SipFail)
		return SipFail;
	if  (__sip_cloneSipAuthParam (*ppDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneAuthParam");
	return SipSuccess;
} */


SipBool sip_cloneSipGenericChallenge
#ifdef ANSI_PROTO
	(SipGenericChallenge *pDest, SipGenericChallenge *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipGenericChallenge *pDest;
	SipGenericChallenge *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipGenericChallenge");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipGenericChallenge(ppDest, pErr) == SipFail)
		return SipFail; */

	if  (__sip_cloneSipChallenge (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipGenericChallenge");
	return SipSuccess;
}


SipBool sip_cloneSipGenericCredential
#ifdef ANSI_PROTO
	(SipGenericCredential *pDest, SipGenericCredential *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipGenericCredential *pDest;
	SipGenericCredential *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipGenericCredential");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/*	if (sip_initSipGenericCredential(ppDest, SipCredAny, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneCredential (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipGenericCredential");
	return SipSuccess;
}


SipBool sip_cloneSipUrl
#ifdef ANSI_PROTO
	(SipUrl *pDest, SipUrl *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipUrl *pDest;
	SipUrl *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipUrl");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipUrl(pDest, pErr) == SipFail)
		return SipFail;*/
	if  (__sip_cloneSipUrl (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipUrl");
	return SipSuccess;
}


/* SipBool sip_cloneSipAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec **ppDest, SipAddrSpec *pSource, SipError *pErr)
#else
	(ppDest, pSource, pErr)
	SipAddrSpec **ppDest;
	SipAddrSpec *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipAddrSpec");
	if (pErr == SIP_NULL)
		return SipFail;
	if ((ppDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (sip_initSipAddrSpec(ppDest, SipAddrAny, pErr) == SipFail)
		return SipFail;
	if  (__sip_cloneSipAddrSpec (*ppDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipAddrSpec");
	return SipSuccess;
} */


SipBool sip_cloneSipDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *pDest, SipDateFormat *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDateFormat *pDest;
	SipDateFormat *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipDateFormat");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipDateFormat(ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipDateFormat (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipDateFormat");
	return SipSuccess;
}


SipBool sip_cloneSipTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *pDest, SipTimeFormat *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipTimeFormat *pDest;
	SipTimeFormat *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipTimeFormat");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipTimeFormat(ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipTimeFormat (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipTimeFormat");
	return SipSuccess;
}


SipBool sip_cloneSipDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct *pDest, SipDateStruct *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDateStruct *pDest;
	SipDateStruct *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipDateStruct");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipDateStruct (pDest, pErr) == SipFail)
		return SipFail;*/
	if  (__sip_cloneSipDateStruct (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipDateStruct");
	return SipSuccess;
}


/* SipBool sip_cloneSipExpiresStruct
#ifdef ANSI_PROTO
	(SipExpiresStruct **ppDest, SipExpiresStruct *pSource, SipError *pErr)
#else
	(ppDest, pSource, pErr)
	SipExpiresStruct **ppDest;
	SipExpiresStruct *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipExpiresStruct");
	if (pErr == SIP_NULL)
		return SipFail;
	if ((ppDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (sip_initSipExpiresStruct (ppDest, SipExpAny, pErr) == SipFail)
		return SipFail;
	if  (__sip_cloneSipExpiresStruct (*ppDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipExpiresStruct");
	return SipSuccess;
} */


SipBool sdp_cloneSdpTime
#ifdef ANSI_PROTO
	(SdpTime *pDest, SdpTime *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SdpTime *pDest;
	SdpTime *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sdp_cloneSdpTime");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSdpTime (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSdpTime (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sdp_cloneSdpTime");
	return SipSuccess;
}


SipBool sdp_cloneSdpMedia
#ifdef ANSI_PROTO
	(SdpMedia *pDest, SdpMedia *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SdpMedia *pDest;
	SdpMedia *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sdp_cloneSdpMedia");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSdpMedia (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSdpMedia (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sdp_cloneSdpMedia");
	return SipSuccess;
}


SipBool sdp_cloneSdpConnection
#ifdef ANSI_PROTO
	(SdpConnection *pDest, SdpConnection *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SdpConnection *pDest;
	SdpConnection *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sdp_cloneSdpConnection");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/*	if (sip_initSdpConnection (pDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSdpConnection (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sdp_cloneSdpConnection");
	return SipSuccess;
}


SipBool sdp_cloneSdpAttr
#ifdef ANSI_PROTO
	(SdpAttr *pDest,SdpAttr *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SdpAttr *pDest;
	SdpAttr *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sdp_clonesdp_cloneSdpAttr");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/*	if (sip_initSdpAttr (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSdpAttr (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sdp_clonesdp_cloneSdpAttr");
	return SipSuccess;
}


SipBool sdp_cloneSdpMessage
#ifdef ANSI_PROTO
	(SdpMessage *pDest, SdpMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SdpMessage *pDest;
	SdpMessage *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sdp_cloneSdpMessage");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSdpMessage (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSdpMessage (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sdp_cloneSdpMessage");
	return SipSuccess;
}

#ifdef SIP_MWI
SipBool sip_mwi_cloneMesgSummaryMessage
#ifdef ANSI_PROTO
        (MesgSummaryMessage *pDest, MesgSummaryMessage *pSource, SipError *pErr)
#else
        (pDest, pSource, pErr)
        MesgSummaryMessage *pDest;
        MesgSummaryMessage *pSource;
        SipError *pErr;
#endif
{
        SIPDEBUGFN("Entering function sip_mwi_cloneMesgSummaryMessage");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;
        if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif
        if  (__sip_mwi_cloneMesgSummaryMessage(pDest, pSource, pErr) == SipFail)
                return SipFail;

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_mwi_cloneMesgSummaryMessage");
        return SipSuccess;
}

SipBool sip_mwi_cloneSummaryLine
#ifdef ANSI_PROTO
        (SummaryLine *pDest, SummaryLine *pSource, SipError *pErr)
#else
        (pDest, pSource, pErr)
        SummaryLine *pDest;
        SummaryLine *pSource;
        SipError *pErr;
#endif
{
        SIPDEBUGFN("Entering function sip_mwi_cloneSummaryLine");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;
        if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif
        if  (__sip_mwi_cloneSummaryLine(pDest, pSource, pErr) == SipFail)
                return SipFail;

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_mwi_cloneSummaryLine");
        return SipSuccess;
}
#endif

SipBool sip_cloneSipNameValuePair
#ifdef ANSI_PROTO
        (SipNameValuePair *pDest, SipNameValuePair *pSource, SipError *pErr)
#else
        (pDest, pSource, pErr)
        SipNameValuePair *pDest;
        SipNameValuePair *pSource;
        SipError *pErr;
#endif
{
        SIPDEBUGFN("Entering function sip_cloneSipNameValuePair");
#ifndef SIP_NO_CHECK
        if (pErr == SIP_NULL)
                return SipFail;
        if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
        {
                *pErr = E_INV_PARAM;
                return SipFail;
        }
#endif
        if  (__sip_cloneSipNameValuePair(pDest, pSource, pErr) == SipFail)
                return SipFail;

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_cloneSipNameValuePair");
        return SipSuccess;
}

SipBool sdp_cloneSdpOrigin
#ifdef ANSI_PROTO
	(SdpOrigin *pDest, SdpOrigin *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SdpOrigin *pDest;
	SdpOrigin *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sdp_cloneSdpOrigin");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSdpOrigin (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSdpOrigin (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sdp_cloneSdpOrigin");
	return SipSuccess;
}


SipBool sip_cloneSipUnknownMessage
#ifdef ANSI_PROTO
	(SipUnknownMessage *pDest, SipUnknownMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipUnknownMessage *pDest;
	SipUnknownMessage *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipUnknownMessage");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/*	if (sip_initSipUnknownMessage (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipUnknownMessage (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipUnknownMessage");
	return SipSuccess;
}


SipBool sip_cloneSipMsgBody
#ifdef ANSI_PROTO
	(SipMsgBody *pDest, SipMsgBody *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipMsgBody *pDest;
	SipMsgBody *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipMsgBody");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipMsgBody (ppDest, SipBodyAny, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipMsgBody (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipMsgBody");
	return SipSuccess;
}


SipBool sip_cloneSipParam
#ifdef ANSI_PROTO
	(SipParam *pDest, SipParam *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipParam *pDest;
	SipParam *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipParam");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
/* 	if (sip_initSipParam (ppDest, pErr) == SipFail)
		return SipFail; */
	if  (__sip_cloneSipParam (pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipParam");
	return SipSuccess;
}


SipBool sip_cloneSipHeader
#ifdef ANSI_PROTO
	(SipHeader *pDest, SipHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipHeader *pDest;
	SipHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipHeader");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pSource->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* 	if (*ppDest != SIP_NULL)
		sip_freeSipHeader (*ppDest);
		if (sip_initSipHeader (ppDest, pSource->dType, pErr) == SipFail)
		return SipFail; */

	switch (pSource->dType)
	{
		case	SipHdrTypeAccept 		:if (__sip_cloneSipAcceptHeader( (SipAcceptHeader *)(pDest->pHeader), (SipAcceptHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeAcceptEncoding	:if (__sip_cloneSipAcceptEncodingHeader ((SipAcceptEncodingHeader *)(pDest->pHeader), (SipAcceptEncodingHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeAcceptLanguage	:if (__sip_cloneSipAcceptLangHeader( (SipAcceptLangHeader *)(pDest->pHeader), (SipAcceptLangHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeTimestamp 		:if (__sip_cloneSipTimeStampHeader((SipTimeStampHeader *)(pDest->pHeader), (SipTimeStampHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeContentLength		:if (__sip_cloneSipContentLengthHeader ((SipContentLengthHeader *)(pDest->pHeader), (SipContentLengthHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeContentType 		:if (__sip_cloneSipContentTypeHeader ((SipContentTypeHeader *)(pDest->pHeader), (SipContentTypeHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeContentEncoding	:if (__sip_cloneSipContentEncodingHeader ((SipContentEncodingHeader *)(pDest->pHeader), (SipContentEncodingHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeUnknown 		:if (__sip_cloneSipUnknownHeader((SipUnknownHeader *)(pDest->pHeader), (SipUnknownHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeDate	 		:if (__sip_cloneSipDateHeader((SipDateHeader *)(pDest->pHeader), (SipDateHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypeReplaces:
										 if(__sip_cloneSipReplacesHeader((SipReplacesHeader*)(pDest->pHeader),\
													 (SipReplacesHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypeMinExpires:
										 if(__sip_cloneSipMinExpiresHeader((SipMinExpiresHeader*)(pDest->pHeader),\
													 (SipMinExpiresHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeReplyTo:
										 if(__sip_cloneSipReplyToHeader((SipReplyToHeader*)(pDest->pHeader),\
													 (SipReplyToHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypeEncryption 		:if (__sip_cloneSipEncryptionHeader((SipEncryptionHeader *)(pDest->pHeader), (SipEncryptionHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;
		case	SipHdrTypeExpiresDate		:
		case	SipHdrTypeExpiresSec		:
		case	SipHdrTypeExpiresAny		:if (__sip_cloneSipExpiresHeader((SipExpiresHeader *)(pDest)->pHeader, (SipExpiresHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;
#ifdef SIP_IMPP
		case	SipHdrTypeSubscriptionState	:
											 if (__sip_impp_cloneSipSubscriptionStateHeader((\
															 SipSubscriptionStateHeader *)(pDest)->pHeader, \
														 (SipSubscriptionStateHeader *)(pSource->pHeader), pErr) == \
													 SipFail)
												 return SipFail;
											 break;
#endif

#ifdef SIP_SESSIONTIMER
		case	SipHdrTypeMinSE:
											 if ( __sip_cloneSipMinSEHeader((SipMinSEHeader *)(pDest->pHeader), (SipMinSEHeader *)(pSource->pHeader), pErr) == SipFail )
												 return SipFail;
											 break;

		case	SipHdrTypeSessionExpires		:if (__sip_cloneSipSessionExpiresHeader((SipSessionExpiresHeader *)(pDest)->pHeader, (SipSessionExpiresHeader *)(pSource->pHeader), pErr) == SipFail)
													 return SipFail;
												 break;
#endif



		case	SipHdrTypeFrom	 		:if (__sip_cloneSipFromHeader((SipFromHeader *)(pDest->pHeader), (SipFromHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeTo	 		:if (__sip_cloneSipToHeader( (SipToHeader *)(pDest->pHeader), (SipToHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeVia	 		:if (__sip_cloneSipViaHeader((SipViaHeader *)(pDest->pHeader), (SipViaHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case 	SipHdrTypeContactNormal		:
		case	SipHdrTypeContactWildCard	:
		case	SipHdrTypeContactAny	 	:if (__sip_cloneSipContactHeader((SipContactHeader *)(pDest->pHeader), (SipContactHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeRecordRoute	 	:if (__sip_cloneSipRecordRouteHeader( (SipRecordRouteHeader *)(pDest->pHeader), (SipRecordRouteHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;
#ifdef SIP_3GPP
		case	SipHdrTypePath	:
											 if (__sip_cloneSipPathHeader( \
														 (SipPathHeader *)(pDest->pHeader), \
														 (SipPathHeader *)(pSource->pHeader),\
														 pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeServiceRoute:
											 if (__sip_cloneSipServiceRouteHeader( \
														 (SipServiceRouteHeader *)(pDest->pHeader), \
														 (SipServiceRouteHeader *)(pSource->pHeader),\
														 pErr) == SipFail )
												 return SipFail;
											 break;


case	SipHdrTypePanInfo	 		:if (__sip_cloneSipPanInfoHeader(( \
SipPanInfoHeader *)(pDest->pHeader), (SipPanInfoHeader *)(pSource->pHeader),\
pErr)\
== SipFail)
											 return SipFail;
										 break;
case SipHdrTypePcVector :if (__sip_cloneSipPcVectorHeader(( SipPcVectorHeader*)\
(pDest->pHeader), (SipPcVectorHeader *)(pSource->pHeader),pErr)==\
SipFail)
											 return SipFail;
										 break;
       
											 
#endif
											 /* call-transfer */

		case	SipHdrTypeReferTo	 	:if (__sip_cloneSipReferToHeader( (SipReferToHeader *)(pDest->pHeader), (SipReferToHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;
										 /* call-transfer */

		case	SipHdrTypeReferredBy		: if (__sip_cloneSipReferredByHeader( (SipReferredByHeader *)(pDest->pHeader), (SipReferredByHeader *)(pSource->pHeader), pErr) == SipFail)
												  return SipFail;
											  break; /*  call-tansfer */

		case	SipHdrTypeAllow	 		:if (__sip_cloneSipAllowHeader( (SipAllowHeader *)(pDest->pHeader), (SipAllowHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeWarning	 	:if (__sip_cloneSipWarningHeader( (SipWarningHeader *)(pDest->pHeader), (SipWarningHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeRetryAfterDate	:
		case	SipHdrTypeRetryAfterSec		:
		case	SipHdrTypeRetryAfterAny		:if (__sip_cloneSipRetryAfterHeader( (SipRetryAfterHeader *)(pDest->pHeader), (SipRetryAfterHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeProxyAuthenticate	:if (__sip_cloneSipProxyAuthenticateHeader( (SipProxyAuthenticateHeader *)(pDest->pHeader), (SipProxyAuthenticateHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeWwwAuthenticate	:if (__sip_cloneSipWwwAuthenticateHeader( (SipWwwAuthenticateHeader *)(pDest->pHeader), (SipWwwAuthenticateHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeAuthenticationInfo:
											 if (__sip_cloneSipAuthenticationInfoHeader( (SipAuthenticationInfoHeader *)(pDest->pHeader), (SipAuthenticationInfoHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

											 /* 		case	SipHdrTypeAuthorizationAuth	:
														case	SipHdrTypeAuthorizationBasic	: */
		case	SipHdrTypeAuthorization 	:if (__sip_cloneSipAuthorizationHeader( (SipAuthorizationHeader *)(pDest->pHeader), (SipAuthorizationHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeHide	 		:if (__sip_cloneSipHideHeader( (SipHideHeader *)(pDest->pHeader), (SipHideHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeMaxforwards	 	:if (__sip_cloneSipMaxForwardsHeader( (SipMaxForwardsHeader *)(pDest->pHeader), (SipMaxForwardsHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeOrganization	 	:if (__sip_cloneSipOrganizationHeader( (SipOrganizationHeader *)(pDest->pHeader), (SipOrganizationHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypePriority	 	:if (__sip_cloneSipPriorityHeader( (SipPriorityHeader *)(pDest->pHeader), (SipPriorityHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

										 /*		case	SipHdrTypeProxyauthorizationAuth:
												case	SipHdrTypeProxyauthorizationBasic: */
		case	SipHdrTypeProxyauthorization:if (__sip_cloneSipProxyAuthorizationHeader( (SipProxyAuthorizationHeader *)(pDest->pHeader), (SipProxyAuthorizationHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeProxyRequire	 	:if (__sip_cloneSipProxyRequireHeader( (SipProxyRequireHeader *)(pDest->pHeader), (SipProxyRequireHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeRoute	 		:if (__sip_cloneSipRouteHeader( (SipRouteHeader *)(pDest->pHeader), (SipRouteHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeAlso			:if (__sip_cloneSipAlsoHeader( (SipAlsoHeader *)(pDest->pHeader), (SipAlsoHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeRequire	 	:if (__sip_cloneSipRequireHeader( (SipRequireHeader *)(pDest->pHeader), (SipRequireHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeResponseKey	 	:if (__sip_cloneSipRespKeyHeader( (SipRespKeyHeader *)(pDest->pHeader), (SipRespKeyHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeSubject	 	:if (__sip_cloneSipSubjectHeader( (SipSubjectHeader *)(pDest->pHeader), (SipSubjectHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeUserAgent	 	:if (__sip_cloneSipUserAgentHeader( (SipUserAgentHeader *)(pDest->pHeader), (SipUserAgentHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeCallId	 	:if (__sip_cloneSipCallIdHeader( (SipCallIdHeader *)(pDest->pHeader), (SipCallIdHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeServer 		:if (__sip_cloneSipServerHeader( (SipServerHeader *)(pDest->pHeader), (SipServerHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;

										 break;

		case	SipHdrTypeUnsupported 		:if (__sip_cloneSipUnsupportedHeader( (SipUnsupportedHeader *)(pDest->pHeader), (SipUnsupportedHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;

		case	SipHdrTypeCseq	 		:if (__sip_cloneSipCseqHeader( (SipCseqHeader *)(pDest->pHeader), (SipCseqHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypeInReplyTo	 		:if (__sip_cloneSipInReplyToHeader( (SipInReplyToHeader *)(pDest->pHeader), (SipInReplyToHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;
		case	SipHdrTypeContentLanguage	 		:if (__sip_cloneSipContentLanguageHeader( (SipContentLanguageHeader *)(pDest->pHeader), (SipContentLanguageHeader *)(pSource->pHeader), pErr) == SipFail)
														 return SipFail;
													 break;

		case	SipHdrTypeMimeVersion	 	:if (__sip_bcpt_cloneSipMimeVersionHeader( (SipMimeVersionHeader *)(pDest->pHeader), (SipMimeVersionHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break; /* BCP-T */


#ifdef SIP_CCP
		case	SipHdrTypeAcceptContact	 	:if (__sip_ccp_cloneSipAcceptContactHeader( (SipAcceptContactHeader *)(pDest->pHeader), (SipAcceptContactHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break; /* CCP */

		case	SipHdrTypeRejectContact		:if (__sip_ccp_cloneSipRejectContactHeader( (SipRejectContactHeader *)(pDest->pHeader), (SipRejectContactHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break; /* CCP */

		case	SipHdrTypeRequestDisposition		:if (__sip_ccp_cloneSipRequestDispositionHeader( (SipRequestDispositionHeader *)(pDest->pHeader), (SipRequestDispositionHeader *)(pSource->pHeader), pErr) == SipFail)
														 return SipFail;
													 break; /* CCP */
#endif
		case	SipHdrTypeRAck	 		:if (__sip_rpr_cloneSipRAckHeader( (SipRackHeader *)(pDest->pHeader), (SipRackHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break; /* RPR */

		case	SipHdrTypeRSeq	 		:if (__sip_rpr_cloneSipRSeqHeader( (SipRseqHeader *)(pDest->pHeader), (SipRseqHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break; /*RPR */

		case	SipHdrTypeSupported 		:if (__sip_cloneSipSupportedHeader( (SipSupportedHeader *)(pDest->pHeader), (SipSupportedHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;
		case	SipHdrTypeAlertInfo 		:if (__sip_cloneSipAlertInfoHeader( (SipAlertInfoHeader *)(pDest->pHeader), (SipAlertInfoHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;
		case	SipHdrTypeCallInfo 		:if (__sip_cloneSipCallInfoHeader( (SipCallInfoHeader *)(pDest->pHeader), (SipCallInfoHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypeErrorInfo 		:if (__sip_cloneSipErrorInfoHeader( (SipErrorInfoHeader *)(pDest->pHeader), (SipErrorInfoHeader *)(pSource->pHeader), pErr) == SipFail)
												 return SipFail;
											 break;
		case	SipHdrTypeContentDisposition 		:if (__sip_cloneSipContentDispositionHeader( (SipContentDispositionHeader *)(pDest->pHeader), (SipContentDispositionHeader *)(pSource->pHeader), pErr) == SipFail)
														 return SipFail;
													 break;
		case	SipHdrTypeReason	:
													 if (__sip_cloneSipReasonHeader( \
																 (SipReasonHeader *)(pDest->pHeader), \
																 (SipReasonHeader *)(pSource->pHeader), pErr) \
															 == SipFail)
														 return SipFail;
													 break;
#ifdef SIP_IMPP
		case	SipHdrTypeEvent 		:if (__sip_impp_cloneSipEventHeader( (SipEventHeader *)(pDest->pHeader), (SipEventHeader *)(pSource->pHeader), pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypeAllowEvents 		: if (__sip_impp_cloneSipAllowEventsHeader( (SipAllowEventsHeader *)(pDest->pHeader), (SipAllowEventsHeader *)(pSource->pHeader), pErr) == SipFail)
												  return SipFail;
											  break;
#endif

#ifdef SIP_PRIVACY
		case SipHdrTypePAssertId                : 
											  if (__sip_cloneSipPAssertIdHeader( (SipPAssertIdHeader *)\
														  (pDest->pHeader), (SipPAssertIdHeader *)\
														  (pSource->pHeader), pErr) == SipFail)
												  return SipFail;
											  break;
		case SipHdrTypePPreferredId                : 
											  if (__sip_cloneSipPPreferredIdHeader( (SipPPreferredIdHeader *)\
														  (pDest->pHeader), (SipPPreferredIdHeader *)\
														  (pSource->pHeader), pErr) == SipFail)
												  return SipFail;
											  break;
		case SipHdrTypePrivacy :
											  if(__sip_cloneSipPrivacyHeader( (SipPrivacyHeader *)\
														  (pDest->pHeader), (SipPrivacyHeader *)\
														  (pSource->pHeader),pErr) == SipFail)
												  return SipFail;
											  break;
#endif
#ifdef SIP_CONF                                              
		case	SipHdrTypeJoin:
										 if(__sip_cloneSipJoinHeader((SipJoinHeader*)(pDest->pHeader),\
													 (SipJoinHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;
#endif

#ifdef SIP_SECURITY
case    SipHdrTypeSecurityClient:      	
					if (__sip_cloneSipSecurityClientHeader( (SipSecurityClientHeader *)\
					(pDest->pHeader), (SipSecurityClientHeader *)(pSource->pHeader),\
					 pErr) == SipFail)
                                        	 return SipFail;
				break;
case    SipHdrTypeSecurityServer:      
					if (__sip_cloneSipSecurityServerHeader( (SipSecurityServerHeader *)\
                                        (pDest->pHeader), (SipSecurityServerHeader *)(pSource->pHeader),\
                                         pErr) == SipFail)
                                                 return SipFail;
                                break;

case    SipHdrTypeSecurityVerify:    
					if (__sip_cloneSipSecurityVerifyHeader( (SipSecurityVerifyHeader *)\
                                        (pDest->pHeader), (SipSecurityVerifyHeader *)(pSource->pHeader),\
                                         pErr) == SipFail)
                                                 return SipFail;
                                break;
#endif
#ifdef SIP_3GPP
		case	SipHdrTypePAssociatedUri:
										 if(__sip_cloneSipPAssociatedUriHeader((SipPAssociatedUriHeader*)(pDest->pHeader),\
													 (SipPAssociatedUriHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypePCalledPartyId:
										 if(__sip_cloneSipPCalledPartyIdHeader((SipPCalledPartyIdHeader*)(pDest->pHeader),\
													 (SipPCalledPartyIdHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;
		case	SipHdrTypePVisitedNetworkId:
										 if(__sip_cloneSipPVisitedNetworkIdHeader((SipPVisitedNetworkIdHeader*)(pDest->pHeader),\
													 (SipPVisitedNetworkIdHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypePcfAddr:
										 if(__sip_cloneSipPcfAddrHeader((SipPcfAddrHeader*)(pDest->pHeader),\
													 (SipPcfAddrHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;

#endif
#ifdef SIP_CONGEST
		case	SipHdrTypeRsrcPriority:
										 if(__sip_cloneSipRsrcPriorityHeader((SipRsrcPriorityHeader*)(pDest->pHeader),\
													 (SipRsrcPriorityHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;

		case	SipHdrTypeAcceptRsrcPriority:
										 if(__sip_cloneSipAcceptRsrcPriorityHeader((SipAcceptRsrcPriorityHeader*)(pDest->pHeader),\
													 (SipAcceptRsrcPriorityHeader*)(pSource->pHeader),pErr) == SipFail)
											 return SipFail;
										 break;

#endif


		case	SipHdrTypeAny:
											  *pErr = E_INV_TYPE;
											  return SipFail;

		default	:
#ifdef SIP_DCS
											  if (sip_dcs_cloneDcsHeaders(pDest, pSource, pErr) == SipSuccess)
												  break;
											  else
											  {
												  if (*pErr != E_INV_TYPE)
													  return SipFail;
											  }
#endif
											  *pErr = E_INV_TYPE;
											  return SipFail;
	}

	pDest->dType = pSource->dType;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipHeader");
	return SipSuccess;
}


SipBool sip_cloneSipMessage
#ifdef ANSI_PROTO
	(SipMessage *pDest, SipMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipMessage *pDest;
	SipMessage *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneSipMessage");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((pSource == SIP_NULL)||(pDest == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if (__sip_cloneSipMessage(pDest, pSource, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_cloneSipMessage");
	return SipSuccess;
}


SipBool __sip_cloneSipOrderInfoList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipHeaderOrderInfo *temp_order, *clone_order;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipOrderInfoList");
	/* copying siplist of SipHeaderOrderInfo */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_order), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_order == SIP_NULL )
			clone_order = SIP_NULL;
		else
		{
			if(sip_initSipHeaderOrderInfo(&clone_order,pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_order == SIP_NULL )
				return SipFail;

			if (__sip_cloneSipHeaderOrderInfo( clone_order, temp_order, pErr)\
				== SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_order),\
					SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_order, pErr) == SipFail )
		{
			if ( clone_order != SIP_NULL )
			{
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_order),\
					SIP_NULL);
			}
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipOrderInfoList");
	return SipSuccess;
}


SipBool __sip_cloneSipMessageBodyList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipMsgBody *temp_msgbody, *clone_msgbody;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipMessageBodyList");
	/* copying siplist of SipMsgBody */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_msgbody),\
			pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_msgbody == SIP_NULL )
			clone_msgbody = SIP_NULL;
		else
		{
			if(sip_initSipMsgBody(&clone_msgbody, SipBodyAny, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_msgbody == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipMsgBody(clone_msgbody, temp_msgbody, pErr)\
				== SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_msgbody),\
					SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_msgbody, pErr) == SipFail )
		{
			if ( clone_msgbody != SIP_NULL )
			{
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_msgbody),\
					SIP_NULL);
			}
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipMessageBodyList");
	return SipSuccess;
}


SipBool __sip_cloneSipContentLanguageHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipContentLanguageHeader *temp_contentlanguage, *clone_contentlanguage;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipContentLanguageHeaderList");
	/* copying siplist of SipContentLanguageHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_contentlanguage), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_contentlanguage == SIP_NULL )
			clone_contentlanguage = SIP_NULL;
		else
		{
			if(sip_initSipContentLanguageHeader(&clone_contentlanguage,pErr)\
				==SipFail)
			{
				return SipFail;
			}

			if ( clone_contentlanguage == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipContentLanguageHeader(clone_contentlanguage,\
				temp_contentlanguage, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID,\
					(SIP_Pvoid*)&(clone_contentlanguage), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_contentlanguage, pErr) == SipFail )
		{
			if ( clone_contentlanguage != SIP_NULL )
			{
				sip_memfree(ACCESSOR_MEM_ID,\
					(SIP_Pvoid*)&(clone_contentlanguage), SIP_NULL);
			}
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentLanguageHeaderList");
	return SipSuccess;
}



SipBool __sip_cloneSipInReplyToHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipInReplyToHeader *temp_inreplyto, *clone_inreplyto;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipInReplyToHeaderList");
	/* copying siplist of SipInReplyToHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_inreplyto), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_inreplyto == SIP_NULL )
			clone_inreplyto = SIP_NULL;
		else
		{
			if(sip_initSipInReplyToHeader(&clone_inreplyto,pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_inreplyto == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipInReplyToHeader(  clone_inreplyto,\
				temp_inreplyto, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_inreplyto),\
					SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_inreplyto, pErr) == SipFail )
		{
			if ( clone_inreplyto != SIP_NULL )
			{
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_inreplyto),\
					SIP_NULL);
			}
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipInReplyToHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipAcceptHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAcceptHeader *temp_accept, *clone_accept;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAcceptHeaderList");
	/* copying siplist of SipAcceptHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_accept), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_accept == SIP_NULL )
			clone_accept = SIP_NULL;
		else
		{
			if(sip_initSipAcceptHeader(&clone_accept,pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_accept == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAcceptHeader(  clone_accept, temp_accept, pErr)\
				== SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_accept),\
					SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_accept, pErr) == SipFail )
		{
			if ( clone_accept != SIP_NULL )
			{
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_accept),\
					SIP_NULL);
			}
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipAcceptEncodingHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAcceptEncodingHeader *temp_aencoding, *clone_aencoding;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAcceptEncodingHeaderList");
	/* copying siplist of SipAcceptEncodingHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_aencoding), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_aencoding == SIP_NULL )
			clone_aencoding = SIP_NULL;
		else
		{
			if(sip_initSipAcceptEncodingHeader(&clone_aencoding, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_aencoding == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAcceptEncodingHeader(  clone_aencoding, temp_aencoding, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_aencoding), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_aencoding, pErr) == SipFail )
		{
			if ( clone_aencoding != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_aencoding), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptEncodingHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipAcceptLangHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAcceptLangHeader *temp_alang, *clone_alang;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAcceptLangHeaderList");
	/* copying siplist of SipAcceptLangHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_alang), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_alang == SIP_NULL )
			clone_alang = SIP_NULL;
		else
		{
			if(sip_initSipAcceptLangHeader(&clone_alang, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_alang == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAcceptLangHeader(  clone_alang, temp_alang, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_alang), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_alang, pErr) == SipFail )
		{
			if ( clone_alang != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_alang), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptLangHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipContactHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipContactHeader *temp_contact, *clone_contact;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipContactHeaderList");
	/* copying siplist of SipContactHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_contact), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_contact == SIP_NULL )
			clone_contact = SIP_NULL;
		else
		{
			if(sip_initSipContactHeader(&clone_contact, SipContactAny, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_contact == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipContactHeader(  clone_contact, temp_contact, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_contact), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_contact, pErr) == SipFail )
		{
			if ( clone_contact != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_contact), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContactHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipRecordRouteHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipRecordRouteHeader *temp_recordroute, *clone_recordroute;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipRecordRouteHeaderList");
	/* copying siplist of SipRecordRouteHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_recordroute), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_recordroute == SIP_NULL )
			clone_recordroute = SIP_NULL;
		else
		{
			if(sip_initSipRecordRouteHeader(&clone_recordroute, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_recordroute == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipRecordRouteHeader(  clone_recordroute, temp_recordroute, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_recordroute), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_recordroute, pErr) == SipFail )
		{
			if ( clone_recordroute != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_recordroute), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRecordRouteHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipViaHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipViaHeader *temp_via, *clone_via;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipViaHeaderList");
	/* copying siplist of SipViaHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_via), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_via == SIP_NULL )
			clone_via = SIP_NULL;
		else
		{
			if(sip_initSipViaHeader(&clone_via, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_via == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipViaHeader(clone_via, temp_via, pErr) == SipFail)
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_via),SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_via, pErr) == SipFail )
		{
			if ( clone_via != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_via), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipViaHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipContentEncodingHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipContentEncodingHeader *temp_cencoding, *clone_cencoding;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipContentEncodingHeaderList");
	/* copying siplist of SipContentEncodingHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_cencoding), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_cencoding == SIP_NULL )
			clone_cencoding = SIP_NULL;
		else
		{
			if(sip_initSipContentEncodingHeader(&clone_cencoding, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_cencoding == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipContentEncodingHeader(  clone_cencoding, temp_cencoding, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_cencoding), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_cencoding, pErr) == SipFail )
		{
			if ( clone_cencoding != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_cencoding), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentEncodingHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipUnknownHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipUnknownHeader *temp_unknown, *clone_unknown;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipUnknownHeaderList");
	/* copying siplist of SipUnknownHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_unknown), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_unknown == SIP_NULL )
			clone_unknown = SIP_NULL;
		else
		{
			if(sip_initSipUnknownHeader(&clone_unknown, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_unknown == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipUnknownHeader(  clone_unknown, temp_unknown, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_unknown), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_unknown, pErr) == SipFail )
		{
			if ( clone_unknown != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_unknown), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipUnknownHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipBadHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipBadHeader *temp_unknown, *clone_unknown;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipBadHeaderList");
	/* copying siplist of SipUnknownHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_unknown), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_unknown == SIP_NULL )
			clone_unknown = SIP_NULL;
		else
		{
			if(sip_initSipBadHeader(&clone_unknown,SipHdrTypeAny, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_unknown == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipBadHeader(  clone_unknown, temp_unknown, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_unknown), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_unknown, pErr) == SipFail )
		{
			if ( clone_unknown != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_unknown), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipBadHeaderList");
	return SipSuccess;
}



SipBool __sip_cloneSipSupportedHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipSupportedHeader *temp_supported, *clone_supported;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipSupportedHeaderList");
	/* copying siplist of SipSupportedHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_supported), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_supported == SIP_NULL )
			clone_supported = SIP_NULL;
		else
		{
			if(sip_initSipSupportedHeader(&clone_supported, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_supported == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipSupportedHeader(  clone_supported, temp_supported, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_supported), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_supported, pErr) == SipFail )
		{
			if ( clone_supported != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_supported), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipSupportedHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipProxyRequireHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipProxyRequireHeader *temp_proxyrequire, *clone_proxyrequire;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipProxyRequireHeaderList");
	/* copying siplist of SipProxyRequireHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_proxyrequire), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_proxyrequire == SIP_NULL )
			clone_proxyrequire = SIP_NULL;
		else
		{
			if(sip_initSipProxyRequireHeader(&clone_proxyrequire, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_proxyrequire == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipProxyRequireHeader(  clone_proxyrequire, temp_proxyrequire, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_proxyrequire), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_proxyrequire, pErr) == SipFail )
		{
			if ( clone_proxyrequire != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_proxyrequire), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipProxyRequireHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipProxyAuthorizationHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipProxyAuthorizationHeader *temp_proxyauth, *clone_proxyauth;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipProxyAuthorizationHeaderList");
	/* copying siplist of SipProxyAuthorizationHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_proxyauth), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_proxyauth == SIP_NULL )
			clone_proxyauth = SIP_NULL;
		else
		{
			if(sip_initSipProxyAuthorizationHeader(&clone_proxyauth, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_proxyauth == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipProxyAuthorizationHeader(  clone_proxyauth, temp_proxyauth, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_proxyauth), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_proxyauth, pErr) == SipFail )
		{
			if ( clone_proxyauth != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_proxyauth), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipProxyAuthorizationHeaderList");
	return SipSuccess;
}

SipBool __sip_cloneSipRouteHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipRouteHeader *temp_route, *clone_route;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipRouteHeaderList");
	/* copying siplist of SipRouteHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_route), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_route == SIP_NULL )
			clone_route = SIP_NULL;
		else
		{
			if(sip_initSipRouteHeader(&clone_route, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_route == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipRouteHeader(  clone_route, temp_route, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_route), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_route, pErr) == SipFail )
		{
			if ( clone_route != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_route), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRouteHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipAlsoHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAlsoHeader *temp_also, *clone_also;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAlsoHeaderList");
	/* copying siplist of SipAlsoHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid *) (&temp_also), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_also == SIP_NULL )
			clone_also = SIP_NULL;
		else
		{
			if(sip_initSipAlsoHeader(&clone_also, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_also == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAlsoHeader(  clone_also, temp_also, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_also), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_also, pErr) == SipFail )
		{
			if ( clone_also != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_also), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAlsoHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipRequireHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipRequireHeader *temp_require, *clone_require;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipRequireHeaderList");
	/* copying siplist of SipRequireHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_require), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_require == SIP_NULL )
			clone_require = SIP_NULL;
		else
		{
			if(sip_initSipRequireHeader(&clone_require, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_require == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipRequireHeader(  clone_require, temp_require, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_require), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_require, pErr) == SipFail )
		{
			if ( clone_require != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_require), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRequireHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipWwwAuthenticateHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipWwwAuthenticateHeader *temp_wwwauthenticate, *clone_wwwauthenticate;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipWwwAuthenticateHeaderList");
	/* copying siplist of SipAcceptContactHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_wwwauthenticate), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_wwwauthenticate == SIP_NULL )
			clone_wwwauthenticate = SIP_NULL;
		else
		{
			if(sip_initSipWwwAuthenticateHeader(&clone_wwwauthenticate, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_wwwauthenticate == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipWwwAuthenticateHeader(  clone_wwwauthenticate, temp_wwwauthenticate, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_wwwauthenticate), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_wwwauthenticate, pErr) == SipFail )
		{
			if ( clone_wwwauthenticate != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_wwwauthenticate), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipWwwAuthenticateHeaderList");
	return SipSuccess;
}


#ifdef SIP_CCP
SipBool __sip_cloneSipAcceptContactHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAcceptContactHeader *temp_accept, *clone_accept;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAcceptContactHeaderList");
	/* copying siplist of SipAcceptContactHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_accept), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_accept == SIP_NULL )
			clone_accept = SIP_NULL;
		else
		{
			if(sip_ccp_initSipAcceptContactHeader(&clone_accept,pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_accept == SIP_NULL )
				return SipFail;

			if ( __sip_ccp_cloneSipAcceptContactHeader(  clone_accept, temp_accept, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_accept), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_accept, pErr) == SipFail )
		{
			if ( clone_accept != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_accept), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptContactHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipRejectContactHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipRejectContactHeader *temp_reject, *clone_reject;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipRejectContactHeaderList");
	/* copying siplist of SipRejectContactHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_reject), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_reject == SIP_NULL )
			clone_reject = SIP_NULL;
		else
		{
			if(sip_ccp_initSipRejectContactHeader(&clone_reject,pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_reject == SIP_NULL )
				return SipFail;

			if ( __sip_ccp_cloneSipRejectContactHeader(  clone_reject, temp_reject, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_reject), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_reject, pErr) == SipFail )
		{
			if ( clone_reject != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_reject), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRejectContactHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipRequestDispositionHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipRequestDispositionHeader *temp_reqdisp, *clone_reqdisp;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipRequestDispositionHeaderList");
	/* copying siplist of SipRequestDispositionHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_reqdisp), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_reqdisp == SIP_NULL )
			clone_reqdisp = SIP_NULL;
		else
		{
			if(sip_ccp_initSipRequestDispositionHeader(&clone_reqdisp, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_reqdisp == SIP_NULL )
				return SipFail;

			if ( __sip_ccp_cloneSipRequestDispositionHeader(  clone_reqdisp, temp_reqdisp, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_reqdisp), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_reqdisp, pErr) == SipFail )
		{
			if ( clone_reqdisp != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_reqdisp), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRequestDispositionHeaderList");
	return SipSuccess;
}
#endif /* of CCP */

SipBool __sip_cloneSipAllowHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAllowHeader *temp_allow, *clone_allow;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAllowHeaderList");
	/* copying siplist of SipAllowHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_allow), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_allow == SIP_NULL )
			clone_allow = SIP_NULL;
		else
		{
			if(sip_initSipAllowHeader(&clone_allow, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_allow == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAllowHeader(  clone_allow, temp_allow, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_allow), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_allow, pErr) == SipFail )
		{
			if ( clone_allow != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_allow), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAllowHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipProxyAuthenticateHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipProxyAuthenticateHeader *temp_proxyauthenticate, *clone_proxyauthenticate;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipProxyAuthenticateHeaderList");
	/* copying siplist of SipProxyAuthenticateHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_proxyauthenticate), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_proxyauthenticate == SIP_NULL )
			clone_proxyauthenticate = SIP_NULL;
		else
		{
			if(sip_initSipProxyAuthenticateHeader(&clone_proxyauthenticate, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_proxyauthenticate == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipProxyAuthenticateHeader(  clone_proxyauthenticate, temp_proxyauthenticate, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_proxyauthenticate), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_proxyauthenticate, pErr) == SipFail )
		{
			if ( clone_proxyauthenticate != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_proxyauthenticate), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipProxyAuthenticateHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipUnsupportedHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipUnsupportedHeader *temp_unsupported, *clone_unsupported;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipUnsupportedHeaderList");
	/* copying siplist of SipUnsupportedHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_unsupported), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_unsupported == SIP_NULL )
			clone_unsupported = SIP_NULL;
		else
		{
			if(sip_initSipUnsupportedHeader(&clone_unsupported, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_unsupported == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipUnsupportedHeader(  clone_unsupported, temp_unsupported, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_unsupported), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_unsupported, pErr) == SipFail )
		{
			if ( clone_unsupported != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_unsupported), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipUnsupportedHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipWarningHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipWarningHeader *temp_warning, *clone_warning;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipWarningHeaderList");
	/* copying siplist of SipWarningHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_warning), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_warning == SIP_NULL )
			clone_warning = SIP_NULL;
		else
		{
			if(sip_initSipWarningHeader(&clone_warning, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_warning == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipWarningHeader(  clone_warning, temp_warning, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_warning), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_warning, pErr) == SipFail )
		{
			if ( clone_warning != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_warning), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipWarningHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipAlertInfoHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAlertInfoHeader *temp_alertinfo, *clone_alertinfo;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAlertInfoHeaderList");
	/* copying siplist of SipAlertInfoHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_alertinfo), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_alertinfo == SIP_NULL )
			clone_alertinfo = SIP_NULL;
		else
		{
			if(sip_initSipAlertInfoHeader(&clone_alertinfo, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_alertinfo == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAlertInfoHeader(  clone_alertinfo, temp_alertinfo, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_alertinfo), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_alertinfo, pErr) == SipFail )
		{
			if ( clone_alertinfo != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_alertinfo), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAlertInfoHeaderList");
	return SipSuccess;
}


SipBool __sip_cloneSipErrorInfoHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipErrorInfoHeader *temp_errorinfo, *clone_errorinfo;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipErrorInfoHeaderList");
	/* copying siplist of SipErrorInfoHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_errorinfo), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_errorinfo == SIP_NULL )
			clone_errorinfo = SIP_NULL;
		else
		{
			if(sip_initSipErrorInfoHeader(&clone_errorinfo, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_errorinfo == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipErrorInfoHeader(  clone_errorinfo, temp_errorinfo, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_errorinfo), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_errorinfo, pErr) == SipFail )
		{
			if ( clone_errorinfo != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_errorinfo), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipErrorInfoHeaderList");
	return SipSuccess;
}

SipBool __sip_cloneSipCallInfoHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipCallInfoHeader *temp_callinfo, *clone_callinfo;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipCallInfoHeaderList");
	/* copying siplist of SipCallInfoHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_callinfo), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_callinfo == SIP_NULL )
			clone_callinfo = SIP_NULL;
		else
		{
			if(sip_initSipCallInfoHeader(&clone_callinfo, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_callinfo == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipCallInfoHeader(  clone_callinfo, temp_callinfo, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_callinfo), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_callinfo, pErr) == SipFail )
		{
			if ( clone_callinfo != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_callinfo), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipCallInfoHeaderList");
	return SipSuccess;
}

SipBool __sip_cloneSipContentDispositionHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipContentDispositionHeader *temp_contentdisposition, *clone_contentdisposition;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipContentDispositionHeaderList");
	/* copying siplist of SipContentDispositionHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_contentdisposition), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_contentdisposition == SIP_NULL )
			clone_contentdisposition = SIP_NULL;
		else
		{
			if(sip_initSipContentDispositionHeader(&clone_contentdisposition, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_contentdisposition == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipContentDispositionHeader(  clone_contentdisposition, temp_contentdisposition, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_contentdisposition), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_contentdisposition, pErr) == SipFail )
		{
			if ( clone_contentdisposition != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_contentdisposition), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentDispositionHeaderList");
	return SipSuccess;
}

SipBool __sip_cloneSipReasonHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipReasonHeader *pTempReason, *pCloneReason;
	SIP_U32bit dCount, dIndex;

	SIPDEBUGFN("Entering function __sip_cloneSipReasonHeaderList");
	/* copying siplist of SipReasonHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &dCount, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( dIndex = 0; dIndex < dCount ; dIndex++)
	{
		if( sip_listGetAt(pSource,dIndex, (SIP_Pvoid * ) (&pTempReason), \
			pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempReason == SIP_NULL )
			pCloneReason = SIP_NULL;
		else
		{
			if(sip_initSipReasonHeader(&pCloneReason, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( pCloneReason == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipReasonHeader(  pCloneReason, pTempReason, \
				pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCloneReason), \
					SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneReason, pErr) == SipFail )
		{
			if ( pCloneReason != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pCloneReason), \
					SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipReasonHeaderList");
	return SipSuccess;
}

#ifdef SIP_SECURITY
SipBool __sip_cloneSipSecurityClientHeaderList
        (SipList *pDest, 
	SipList *pSource, 
	SipError *pErr)
{
        SipSecurityClientHeader *temp_securityclient, *clone_securityclient;
        SIP_U32bit count, index;

        SIPDEBUGFN("Entering function __sip_cloneSipSecurityClientHeaderList");
        
	/* copying siplist of SipSecurityClientHeader */
        if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
                return SipFail;
        if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
        {
                return SipFail;
        }
        for ( index = 0; index < count ; index++)
        {
                if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_securityclient), \
							pErr) == SipFail )
                {
                        return SipFail;
                }
                if ( temp_securityclient == SIP_NULL )
                        clone_securityclient = SIP_NULL;
                else
                {
                        if(sip_initSipSecurityClientHeader(&clone_securityclient, pErr)==SipFail)
                        {
                                return SipFail;
                        }

                        if ( clone_securityclient == SIP_NULL )
                                return SipFail;
                	if ( __sip_cloneSipSecurityClientHeader(  clone_securityclient, \
				temp_securityclient, pErr) == SipFail )
                        {
                                sip_memfree(ACCESSOR_MEM_ID, \
					(SIP_Pvoid*)&(clone_securityclient), SIP_NULL);
                                return SipFail;
                        }
                }

                if ( sip_listAppend(pDest, clone_securityclient, pErr) == SipFail )
                {
                        if ( clone_securityclient != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_securityclient), SIP_NULL);
                        return SipFail;
                }
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function __sip_cloneSipSecurityClientHeaderList");
        return SipSuccess;
}

SipBool __sip_cloneSipSecurityVerifyHeaderList
        (SipList *pDest, 
	SipList *pSource, 
	SipError *pErr)
{
        SipSecurityVerifyHeader *temp_securityverify, *clone_securityverify;
        SIP_U32bit count, index;

        SIPDEBUGFN("Entering function __sip_cloneSipSecurityVerifyHeaderList");

        /* copying siplist of SipSecurityVerifyHeader */
        if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
                return SipFail;
        if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
        {
                return SipFail;
        }
	for ( index = 0; index < count ; index++)
        {
                if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_securityverify), \
                                                        pErr) == SipFail )
                {
                        return SipFail;
                }
                if ( temp_securityverify == SIP_NULL )
                        clone_securityverify = SIP_NULL;
                else
                {
                        if(sip_initSipSecurityVerifyHeader(&clone_securityverify, pErr)==SipFail)
                        {
                                return SipFail;
                        }

                        if ( clone_securityverify == SIP_NULL )
                                return SipFail;
                        if ( __sip_cloneSipSecurityVerifyHeader(  clone_securityverify, \
                                temp_securityverify, pErr) == SipFail )
                        {
                                sip_memfree(ACCESSOR_MEM_ID, \
                                        (SIP_Pvoid*)&(clone_securityverify), SIP_NULL);
                                return SipFail;
                        }
                }

                if ( sip_listAppend(pDest, clone_securityverify, pErr) == SipFail )
                {
                        if ( clone_securityverify != SIP_NULL )
                                sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_securityverify),SIP_NULL); 
                        return SipFail;
                }
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function __sip_cloneSipSecurityVerifyHeaderList");
        return SipSuccess;
}

SipBool __sip_cloneSipSecurityServerHeaderList
        (SipList *pDest, 
	SipList *pSource, 
	SipError *pErr)
{
        SipSecurityServerHeader *temp_securityserver, *clone_securityserver;
        SIP_U32bit count, index;

        SIPDEBUGFN("Entering function __sip_cloneSipSecurityServerHeaderList");

        /* copying siplist of SipSecurityServerHeader */
        if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
                return SipFail;
        if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
        {
                return SipFail;
        }
	for ( index = 0; index < count ; index++)
        {
                if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_securityserver), \
                                                        pErr) == SipFail )
                {
                        return SipFail;
                }
                if ( temp_securityserver == SIP_NULL )
                        clone_securityserver = SIP_NULL;
                else
                {
                        if(sip_initSipSecurityServerHeader(&clone_securityserver, pErr)==SipFail)
                        {
                                return SipFail;
                        }

                        if ( clone_securityserver == SIP_NULL )
                                return SipFail;
                        if ( __sip_cloneSipSecurityServerHeader(  clone_securityserver, \
                                temp_securityserver, pErr) == SipFail )
                        {
                                sip_memfree(ACCESSOR_MEM_ID, \
                                        (SIP_Pvoid*)&(clone_securityserver), SIP_NULL);
                                return SipFail;
                        }
                }

                if ( sip_listAppend(pDest, clone_securityserver, pErr) == SipFail )
                {
                        if ( clone_securityserver != SIP_NULL )
                                sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_securityserver),SIP_NULL); 
                        return SipFail;
                }
        }

        *pErr = E_NO_ERROR;
        SIPDEBUGFN("Exiting function __sip_cloneSipSecurityServerHeaderList");
        return SipSuccess;
}



#endif /*end of #ifdef SIP_SECURITY */



SipBool __sip_cloneSipGeneralHeader
#ifdef ANSI_PROTO
	(SipGeneralHeader *pDest, SipGeneralHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipGeneralHeader *pDest;
	SipGeneralHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipGeneralHeader");
	/* clearing up the destination parameter */
	if (sip_listDeleteAll(&(pDest->slAllowHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slAcceptHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slAcceptEncoding), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slAcceptLang), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slContactHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slRecordRouteHdr), pErr) == SipFail)
		return SipFail;
#ifdef SIP_3GPP
	if (sip_listDeleteAll(&(pDest->slServiceRouteHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slPathHdr), pErr) == SipFail)
		return SipFail;
	sip_freeSipPanInfoHeader (pDest->pPanInfoHdr);
	sip_freeSipPcVectorHeader (pDest->pPcVectorHdr);
	
#endif
	if (sip_listDeleteAll(&(pDest->slViaHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slContentEncodingHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slUnknownHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slBadHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slSupportedHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slCallInfoHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slContentDispositionHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slReasonHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slContentLanguageHdr), pErr) == SipFail)
		return SipFail;
 	if (sip_listDeleteAll(&(pDest->slRequireHdr), pErr) == SipFail)
		return SipFail;
#ifdef SIP_PRIVACY
 	if (sip_listDeleteAll(&(pDest->slPAssertIdHdr), pErr) == SipFail)
		return SipFail;
 	if (sip_listDeleteAll(&(pDest->slPPreferredIdHdr), pErr) == SipFail)
		return SipFail;
	sip_freeSipPrivacyHeader(pDest->pPrivacyHdr);
#endif

#ifdef SIP_IMPP
	if (sip_listDeleteAll(&(pDest->slAllowEventsHdr), pErr) == SipFail)
		return SipFail;
#endif
	sip_freeSipCallIdHeader (pDest->pCallidHdr);
	sip_freeSipCseqHeader (pDest->pCseqHdr);
	sip_freeSipDateHeader (pDest->pDateHdr);
	sip_freeSipEncryptionHeader (pDest->pEncryptionHdr);
	sip_freeSipReplyToHeader (pDest->pReplyToHdr);
	sip_freeSipExpiresHeader (pDest->pExpiresHdr);
#ifdef SIP_SESSIONTIMER
	sip_freeSipMinSEHeader (pDest->pMinSEHdr);
	sip_freeSipSessionExpiresHeader (pDest->pSessionExpiresHdr);
#endif
	sip_freeSipTimeStampHeader (pDest->pTimeStampHdr);
	sip_freeSipToHeader (pDest->pToHdr);
	sip_freeSipContentLengthHeader (pDest->pContentLengthHdr);
	sip_freeSipContentTypeHeader (pDest->pContentTypeHdr);
	sip_bcpt_freeSipMimeVersionHeader (pDest->pMimeVersionHdr);
	sip_freeSipOrganizationHeader (pDest->pOrganizationHdr);
	sip_freeSipUserAgentHeader (pDest->pUserAgentHdr);
	sip_freeSipRetryAfterHeader (pDest->pRetryAfterHdr);

#ifdef SIP_DCS
	if (sip_dcs_deleteAllDcsGeneralHeaders(pDest, pSource, pErr) == SipFail)
		return SipFail;
#endif
#ifdef SIP_3GPP
	sip_freeSipPcfAddrHeader(pDest->pPcfAddrHdr);
#endif
	/* copying source fields */
	if (__sip_cloneSipAllowHeaderList (&(pDest->slAllowHdr), &(pSource->slAllowHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipAcceptHeaderList (&(pDest->slAcceptHdr), &(pSource->slAcceptHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipAcceptEncodingHeaderList (&(pDest->slAcceptEncoding), &(pSource->slAcceptEncoding), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipAcceptLangHeaderList (&(pDest->slAcceptLang), &(pSource->slAcceptLang), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipContactHeaderList (&(pDest->slContactHdr), &(pSource->slContactHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRecordRouteHeaderList (&(pDest->slRecordRouteHdr), &(pSource->slRecordRouteHdr), pErr) == SipFail)
		return SipFail;
#ifdef SIP_3GPP
	if (__sip_cloneSipServiceRouteHeaderList (&(pDest->slServiceRouteHdr), &(pSource->slServiceRouteHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipPathHeaderList (&(pDest->slPathHdr), &(pSource->slPathHdr), pErr) == SipFail)
		return SipFail;
	if(pSource->pPanInfoHdr != SIP_NULL)
	{
	if (sip_initSipPanInfoHeader (&(pDest->pPanInfoHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipPanInfoHeader (pDest->pPanInfoHdr, pSource->pPanInfoHdr\
		, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pPcVectorHdr != SIP_NULL)
	{
	if (sip_initSipPcVectorHeader (&(pDest->pPcVectorHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipPcVectorHeader (pDest->pPcVectorHdr,\
		pSource->pPcVectorHdr,pErr) == SipFail)
		return SipFail;
	}
	
#endif
	if (__sip_cloneSipViaHeaderList (&(pDest->slViaHdr), &(pSource->slViaHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipContentEncodingHeaderList (&(pDest->slContentEncodingHdr), &(pSource->slContentEncodingHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipUnknownHeaderList (&(pDest->slUnknownHdr), &(pSource->slUnknownHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipBadHeaderList (&(pDest->slBadHdr), &(pSource->slBadHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipSupportedHeaderList (&(pDest->slSupportedHdr), &(pSource->slSupportedHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRequireHeaderList (&(pDest->slRequireHdr), &(pSource->slRequireHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipCallInfoHeaderList (&(pDest->slCallInfoHdr), &(pSource->slCallInfoHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipContentDispositionHeaderList (&(pDest->slContentDispositionHdr), &(pSource->slContentDispositionHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipReasonHeaderList (&(pDest->slReasonHdr), \
		&(pSource->slReasonHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipContentLanguageHeaderList (&(pDest->slContentLanguageHdr), &(pSource->slContentLanguageHdr), pErr) == SipFail)
		return SipFail;
#ifdef SIP_IMPP
	if (__sip_impp_cloneSipAllowEventsHeaderList (&(pDest->slAllowEventsHdr), &(pSource->slAllowEventsHdr), pErr) == SipFail)
		return SipFail;
#endif
#ifdef SIP_PRIVACY
	if (__sip_cloneSipPAssertIdHeaderList (&(pDest->slPAssertIdHdr), \
			&(pSource->slPAssertIdHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipPPreferredIdHeaderList (&(pDest->slPPreferredIdHdr), \
			&(pSource->slPPreferredIdHdr), pErr) == SipFail)
		return SipFail;
#endif

	if(pSource->pCallidHdr != SIP_NULL)
	{
	if (sip_initSipCallIdHeader (&(pDest->pCallidHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipCallIdHeader (pDest->pCallidHdr, pSource->pCallidHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pCseqHdr != SIP_NULL)
	{
	if (sip_initSipCseqHeader (&(pDest->pCseqHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipCseqHeader (pDest->pCseqHdr, pSource->pCseqHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pDateHdr != SIP_NULL)
	{
	if (sip_initSipDateHeader (&(pDest->pDateHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipDateHeader (pDest->pDateHdr, pSource->pDateHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pEncryptionHdr != SIP_NULL)
	{
	if (sip_initSipEncryptionHeader (&(pDest->pEncryptionHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipEncryptionHeader (pDest->pEncryptionHdr, pSource->pEncryptionHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pReplyToHdr != SIP_NULL)
	{
	if (sip_initSipReplyToHeader (&(pDest->pReplyToHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipReplyToHeader(pDest->pReplyToHdr, pSource->pReplyToHdr, pErr) == SipFail)
		return SipFail;
	}


	if(pSource->pExpiresHdr != SIP_NULL)
	{
	if (sip_initSipExpiresHeader (&(pDest->pExpiresHdr), SipExpAny, pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipExpiresHeader (pDest->pExpiresHdr, pSource->pExpiresHdr, pErr) == SipFail)
		return SipFail;
	}
#ifdef SIP_SESSIONTIMER
	if (pSource->pMinSEHdr != SIP_NULL)
	{
		if ( sip_initSipMinSEHeader(&(pDest->pMinSEHdr), pErr) == SipFail )
			return SipFail;
		if (__sip_cloneSipMinSEHeader(pDest->pMinSEHdr, pSource->pMinSEHdr,pErr) == SipFail)
			return SipFail;
	}

	if(pSource->pSessionExpiresHdr != SIP_NULL)
	{
	if (sip_initSipSessionExpiresHeader (&(pDest->pSessionExpiresHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipSessionExpiresHeader (pDest->pSessionExpiresHdr, pSource->pSessionExpiresHdr, pErr) == SipFail)
		return SipFail;
	}
#endif

	if(pSource->pFromHeader != SIP_NULL)
	{
	if (sip_initSipFromHeader (&(pDest->pFromHeader), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipFromHeader (pDest->pFromHeader, pSource->pFromHeader, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pTimeStampHdr != SIP_NULL)
	{
	if (sip_initSipTimeStampHeader (&(pDest->pTimeStampHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipTimeStampHeader (pDest->pTimeStampHdr, pSource->pTimeStampHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pToHdr != SIP_NULL)
	{
	if (sip_initSipToHeader (&(pDest->pToHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipToHeader (pDest->pToHdr, pSource->pToHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pContentLengthHdr != SIP_NULL)
	{
	if (sip_initSipContentLengthHeader (&(pDest->pContentLengthHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipContentLengthHeader (pDest->pContentLengthHdr, pSource->pContentLengthHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pContentTypeHdr != SIP_NULL)
	{
	if (sip_initSipContentTypeHeader (&(pDest->pContentTypeHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipContentTypeHeader (pDest->pContentTypeHdr, pSource->pContentTypeHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pMimeVersionHdr != SIP_NULL)
	{
	if (sip_bcpt_initSipMimeVersionHeader (&(pDest->pMimeVersionHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_bcpt_cloneSipMimeVersionHeader (pDest->pMimeVersionHdr, pSource->pMimeVersionHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pOrganizationHdr != SIP_NULL)
	{
	if (sip_initSipOrganizationHeader (&(pDest->pOrganizationHdr),  pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipOrganizationHeader (pDest->pOrganizationHdr, pSource->pOrganizationHdr, pErr) == SipFail)
		return SipFail;
	}
#ifdef SIP_PRIVACY
	if(pSource->pPrivacyHdr != SIP_NULL)
	{
		if(sip_initSipPrivacyHeader(&(pDest->pPrivacyHdr),pErr) == SipFail)
			return SipFail;
		if(__sip_cloneSipPrivacyHeader(pDest->pPrivacyHdr,pSource->pPrivacyHdr,\
									pErr) == SipFail)
			return SipFail;
	}
#endif

	if(pSource->pUserAgentHdr != SIP_NULL)
	{
	if (sip_initSipUserAgentHeader (&(pDest->pUserAgentHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipUserAgentHeader (pDest->pUserAgentHdr, pSource->pUserAgentHdr, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pRetryAfterHdr != SIP_NULL)
	{
	if (sip_initSipRetryAfterHeader (&(pDest->pRetryAfterHdr), SipExpAny, pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRetryAfterHeader (pDest->pRetryAfterHdr, pSource->pRetryAfterHdr, pErr) == SipFail)
		return SipFail;
	}




#ifdef SIP_DCS
	if (sip_dcs_cloneAllDcsGeneralHeaders (pDest, pSource, pErr) == SipFail)
		return SipFail;
#endif

#ifdef SIP_3GPP
    /* P-Charging-Function-Addresses */
	if(pSource->pPcfAddrHdr != SIP_NULL)
	{
    	if (sip_initSipPcfAddrHeader (&(pDest->pPcfAddrHdr), pErr) == SipFail)
	    	return SipFail;
    	if (__sip_cloneSipPcfAddrHeader(pDest->pPcfAddrHdr, pSource->pPcfAddrHdr, pErr) == SipFail)
	    	return SipFail;
	}
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Entering function __sip_cloneSipGeneralHeader");
	return SipSuccess;
}


SipBool __sip_cloneSipReqHeader
#ifdef ANSI_PROTO
	(SipReqHeader *pDest, SipReqHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipReqHeader *pDest;
	SipReqHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipReqHeader");
	/* Clearing destination parameter */
	if (sip_listDeleteAll(&(pDest->slProxyAuthorizationHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slProxyRequireHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slRouteHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slAlsoHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slWwwAuthenticateHdr), pErr) == SipFail)
		return SipFail;
#ifdef SIP_CCP
	if (sip_listDeleteAll(&(pDest->slAcceptContactHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slRejectContactHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slRequestDispositionHdr), pErr) == SipFail)
		return SipFail;
#endif
	if (sip_listDeleteAll(&(pDest->slAlertInfoHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slInReplyToHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slAuthorizationHdr), pErr) == SipFail)
		return SipFail;
	sip_freeSipReplacesHeader(pDest->pReplacesHdr);
	sip_freeSipHideHeader (pDest->pHideHdr);
#ifdef SIP_IMPP
	sip_impp_freeSipEventHeader (pDest->pEventHdr);
	if(pSource->pEventHdr != SIP_NULL)
	{
	if (sip_impp_initSipEventHeader (&(pDest->pEventHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_impp_cloneSipEventHeader (pDest->pEventHdr, pSource->pEventHdr, pErr) == SipFail)
		return SipFail;
	}
#endif
#ifdef SIP_IMPP
	sip_impp_freeSipSubscriptionStateHeader (pDest->pSubscriptionStateHdr);
#endif
	sip_freeSipMaxForwardsHeader (pDest->pMaxForwardsHdr);
	sip_freeSipPriorityHeader (pDest->pPriorityHdr);
	sip_freeSipRespKeyHeader (pDest->pRespKeyHdr);
	sip_freeSipSubjectHeader (pDest->pSubjectHdr);
	sip_rpr_freeSipRAckHeader (pDest->pRackHdr);
	sip_freeSipReferToHeader(pDest->pReferToHdr); /* call-transfer */
	sip_freeSipReferredByHeader (pDest->pReferredByHdr); /* call-tansfer */
#ifdef SIP_DCS
	if (sip_dcs_deleteAllDcsRequestHeaders(pDest, pSource, pErr) == SipFail)
		return SipFail;
#endif
    /* Join header */
#ifdef SIP_CONF
	sip_freeSipJoinHeader(pDest->pJoinHdr);
#endif

#ifdef SIP_SECURITY
	if (sip_listDeleteAll(&(pDest->slSecurityClientHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slSecurityVerifyHdr), pErr) == SipFail)
                return SipFail;

#endif
#ifdef SIP_3GPP
	sip_freeSipPCalledPartyIdHeader(pDest->pPCalledPartyIdHdr);
	if (sip_listDeleteAll(&(pDest->slPVisitedNetworkIdHdr), pErr) == SipFail)
		return SipFail;
#endif

	/* now copying the source fields */
	if (__sip_cloneSipProxyRequireHeaderList (&(pDest->slProxyRequireHdr), &(pSource->slProxyRequireHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRouteHeaderList (&(pDest->slRouteHdr), &(pSource->slRouteHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipWwwAuthenticateHeaderList (&(pDest->slWwwAuthenticateHdr), &(pSource->slWwwAuthenticateHdr), pErr) == SipFail)
		return SipFail;
#ifdef SIP_CCP
	if (__sip_cloneSipAcceptContactHeaderList (&(pDest->slAcceptContactHdr), &(pSource->slAcceptContactHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRejectContactHeaderList (&(pDest->slRejectContactHdr), &(pSource->slRejectContactHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRequestDispositionHeaderList (&(pDest->slRequestDispositionHdr), &(pSource->slRequestDispositionHdr), pErr) == SipFail)
		return SipFail;
#endif /* of ccp */
	if (__sip_cloneSipAlertInfoHeaderList (&(pDest->slAlertInfoHdr), &(pSource->slAlertInfoHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipInReplyToHeaderList (&(pDest->slInReplyToHdr), &(pSource->slInReplyToHdr), pErr) == SipFail)
		return SipFail;

	if (__sip_cloneSipAlsoHeaderList (&(pDest->slAlsoHdr), &(pSource->slAlsoHdr), pErr) == SipFail)
		return SipFail;

	if (__sip_cloneSipAuthorizationHeaderList (&(pDest->slAuthorizationHdr), &(pSource->slAuthorizationHdr), pErr) == SipFail)
		return SipFail;

	if(pSource->pHideHdr != SIP_NULL)
	{
	if (sip_initSipHideHeader (&(pDest->pHideHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipHideHeader (pDest->pHideHdr, pSource->pHideHdr, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pReplacesHdr != SIP_NULL)
	{
	if (sip_initSipReplacesHeader (&(pDest->pReplacesHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipReplacesHeader(pDest->pReplacesHdr, pSource->pReplacesHdr, pErr) == SipFail)
		return SipFail;
	}
#ifdef SIP_IMPP
	if(pSource->pSubscriptionStateHdr != SIP_NULL)
	{
		if (sip_impp_initSipSubscriptionStateHeader (& \
			(pDest->pSubscriptionStateHdr), pErr) == SipFail)
			return SipFail;
		if (__sip_impp_cloneSipSubscriptionStateHeader ( \
			pDest->pSubscriptionStateHdr,pSource->pSubscriptionStateHdr, \
			pErr) == SipFail)
			return SipFail;
	}
	else
	{
		pDest->pSubscriptionStateHdr=SIP_NULL;
	}
#endif
	if(pSource->pMaxForwardsHdr != SIP_NULL)
	{
	if (sip_initSipMaxForwardsHeader (&(pDest->pMaxForwardsHdr), pErr) == SipFail)
		return SipFail;
 	if (__sip_cloneSipMaxForwardsHeader (pDest->pMaxForwardsHdr, pSource->pMaxForwardsHdr, pErr) == SipFail)
		return SipFail;
	}


	if(pSource->pPriorityHdr != SIP_NULL)
	{
	if (sip_initSipPriorityHeader (&(pDest->pPriorityHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipPriorityHeader (pDest->pPriorityHdr, pSource->pPriorityHdr, pErr) == SipFail)
		return SipFail;
	}

	if (__sip_cloneSipProxyAuthorizationHeaderList (&(pDest->slProxyAuthorizationHdr), &(pSource->slProxyAuthorizationHdr), pErr) == SipFail)
		return SipFail;

	if(pSource->pRespKeyHdr != SIP_NULL)
	{
	if (sip_initSipRespKeyHeader (&(pDest->pRespKeyHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRespKeyHeader (pDest->pRespKeyHdr, pSource->pRespKeyHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pSubjectHdr != SIP_NULL)
	{
	if (sip_initSipSubjectHeader (&(pDest->pSubjectHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipSubjectHeader (pDest->pSubjectHdr, pSource->pSubjectHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pRackHdr != SIP_NULL)
	{
	if (sip_rpr_initSipRAckHeader (&(pDest->pRackHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_rpr_cloneSipRAckHeader (pDest->pRackHdr, pSource->pRackHdr, pErr) == SipFail)
		return SipFail;
	}

/* call-transfer */

	if(pSource->pReferToHdr != SIP_NULL)
	{
	if (sip_initSipReferToHeader (&(pDest->pReferToHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipReferToHeader (pDest->pReferToHdr, pSource->pReferToHdr, pErr) == SipFail)
		return SipFail;
	}
/* call-transfer */
	if(pSource->pReferredByHdr != SIP_NULL)
	{
	if (sip_initSipReferredByHeader (&(pDest->pReferredByHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipReferredByHeader (pDest->pReferredByHdr, pSource->pReferredByHdr, pErr) == SipFail)
		return SipFail;
	}
#ifdef SIP_DCS
	if (sip_dcs_cloneAllDcsRequestHeaders (pDest, pSource, pErr) == SipFail)
		return SipFail;
#endif
#ifdef SIP_CONF    
    /* Join header */
	if(pSource->pJoinHdr != SIP_NULL)
	{
	if (sip_initSipJoinHeader (&(pDest->pJoinHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipJoinHeader(pDest->pJoinHdr, pSource->pJoinHdr, pErr) == SipFail)
		return SipFail;
	}
#endif
	/*Security-Client */
#ifdef SIP_SECURITY
	if (__sip_cloneSipSecurityClientHeaderList (&(pDest->slSecurityClientHdr), &(pSource->slSecurityClientHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipSecurityVerifyHeaderList (&(pDest->slSecurityVerifyHdr), &(pSource->slSecurityVerifyHdr), pErr) == SipFail)
                return SipFail;

#endif
#ifdef SIP_3GPP
    /* P-Called-Party-Id */
	if(pSource->pPCalledPartyIdHdr != SIP_NULL)
	{
	if (sip_initSipPCalledPartyIdHeader (&(pDest->pPCalledPartyIdHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipPCalledPartyIdHeader(pDest->pPCalledPartyIdHdr, pSource->pPCalledPartyIdHdr, pErr) == SipFail)
		return SipFail;
	}

    /* P-Visited-Network-Id */
    if (__sip_cloneSipPVisitedNetworkIdHeaderList (&(pDest->slPVisitedNetworkIdHdr), &(pSource->slPVisitedNetworkIdHdr), pErr) == SipFail)
		return SipFail;

#endif

#ifdef SIP_CONGEST
    /* RsrcPriority */
    if (__sip_cloneSipRsrcPriorityHeaderList (&(pDest->slRsrcPriorityHdr), &(pSource->slRsrcPriorityHdr), pErr) == SipFail)
		return SipFail;

#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Entering function __sip_cloneSipReqHeader");
	return SipSuccess;
}


SipBool  __sip_cloneSipRespHeader
#ifdef ANSI_PROTO
	(SipRespHeader *pDest, SipRespHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRespHeader *pDest;
	SipRespHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipRespHeader");
	/* clear the destination parameter */
	if (sip_listDeleteAll(&(pDest->slProxyAuthenticateHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slUnsupportedHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slWarningHeader), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slWwwAuthenticateHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slAuthenticationInfoHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slErrorInfoHdr), pErr) == SipFail)
		return SipFail;
/* 	sip_freeSipRetryAfterHeader (pDest->pRetryAfterHdr);*/
	sip_freeSipServerHeader (pDest->pServerHdr);
	if (sip_listDeleteAll(&(pDest->slAuthorizationHdr), pErr) == SipFail)
		return SipFail;

	sip_rpr_freeSipRSeqHeader (pDest->pRSeqHdr);
	sip_freeSipMinExpiresHeader (pDest->pMinExpiresHdr);
#ifdef SIP_DCS
	if (sip_dcs_deleteAllDcsResponseHeaders(pDest, pSource, pErr) == SipFail)
		return SipFail;
#endif
#ifdef SIP_3GPP
	if (sip_listDeleteAll(&(pDest->slPAssociatedUriHdr), pErr) == SipFail)
		return SipFail;
#endif
#ifdef SIP_CONGEST
	if (sip_listDeleteAll(&(pDest->slAcceptRsrcPriorityHdr), pErr) == SipFail)
		return SipFail;
#endif
#ifdef SIP_SECURITY
	if (sip_listDeleteAll(&(pDest->slSecurityServerHdr), pErr) == SipFail)
                return SipFail;
#endif

	/* copy source parameter */
	if (__sip_cloneSipProxyAuthenticateHeaderList (&(pDest->slProxyAuthenticateHdr), &(pSource->slProxyAuthenticateHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipUnsupportedHeaderList (&(pDest->slUnsupportedHdr), &(pSource->slUnsupportedHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipWarningHeaderList (&(pDest->slWarningHeader), &(pSource->slWarningHeader), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipWwwAuthenticateHeaderList (&(pDest->slWwwAuthenticateHdr), &(pSource->slWwwAuthenticateHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipAuthenticationInfoHeaderList (&(pDest->slAuthenticationInfoHdr), &(pSource->slAuthenticationInfoHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipErrorInfoHeaderList (&(pDest->slErrorInfoHdr), &(pSource->slErrorInfoHdr), pErr) == SipFail)
		return SipFail;

 	if(pSource->pMinExpiresHdr != SIP_NULL)
	{
	if (sip_initSipMinExpiresHeader (&(pDest->pMinExpiresHdr),pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipMinExpiresHeader (pDest->pMinExpiresHdr, \
		pSource->pMinExpiresHdr, pErr) == SipFail)
		return SipFail;
	}

	if(pSource->pServerHdr != SIP_NULL)
	{
	if (sip_initSipServerHeader (&(pDest->pServerHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipServerHeader (pDest->pServerHdr, pSource->pServerHdr, pErr) == SipFail)
		return SipFail;
	}

	if (__sip_cloneSipAuthorizationHeaderList (&(pDest->slAuthorizationHdr), &(pSource->slAuthorizationHdr), pErr) == SipFail)
		return SipFail;

	if(pSource->pRSeqHdr != SIP_NULL)
	{
	if (sip_rpr_initSipRSeqHeader (&(pDest->pRSeqHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_rpr_cloneSipRSeqHeader (pDest->pRSeqHdr, pSource->pRSeqHdr, pErr) == SipFail)
		return SipFail;
	}
#ifdef SIP_DCS
	if (sip_dcs_cloneAllDcsResponseHeaders (pDest, pSource, pErr) == SipFail)
		return SipFail;
#endif
#ifdef SIP_3GPP
    /* P-Associated-URI*/
    if (__sip_cloneSipPAssociatedUriHeaderList (&(pDest->slPAssociatedUriHdr), &(pSource->slPAssociatedUriHdr), pErr) == SipFail)
		return SipFail;

#endif

#ifdef SIP_CONGEST
    /* AcceptRsrcPriority */
    if (__sip_cloneSipAcceptRsrcPriorityHeaderList (&(pDest->slAcceptRsrcPriorityHdr), &(pSource->slAcceptRsrcPriorityHdr), pErr) == SipFail)
		return SipFail;

#endif

#ifdef SIP_SECURITY
	/*Security-Server */	
	if (__sip_cloneSipSecurityServerHeaderList (&(pDest->slSecurityServerHdr), &(pSource->slSecurityServerHdr), pErr) == SipFail)
                return SipFail;
#endif
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRespHeader");
	return SipSuccess;
}


SipBool __sip_cloneSipReqMessage
#ifdef ANSI_PROTO
	(SipReqMessage *pDest, SipReqMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipReqMessage *pDest;
	SipReqMessage *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipReqMessage");
	/* clear destination parameter */
	if (pDest->pRequestLine != SIP_NULL)
		sip_freeSipReqLine(pDest->pRequestLine);
	if (pDest->pRequestHdr != SIP_NULL)
		sip_freeSipReqHeader (pDest->pRequestHdr);
	/* Now copying source */
	if (sip_initSipReqLine (&(pDest->pRequestLine), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipReqLine(pDest->pRequestLine, pSource->pRequestLine, pErr) == SipFail)
		return SipFail;
	if (sip_initSipReqHeader(&(pDest->pRequestHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipReqHeader(pDest->pRequestHdr, pSource->pRequestHdr, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipReqMessage");
	return SipSuccess;
}


SipBool __sip_cloneSipHeaderOrderInfo
#ifdef ANSI_PROTO
	(SipHeaderOrderInfo *pDest, SipHeaderOrderInfo *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipHeaderOrderInfo *pDest;
	SipHeaderOrderInfo *pSource;
	SipError *pErr;
#endif
{
 	SIPDEBUGFN("Entering function __sip_cloneSipHeaderOrderInfo");
	/* no pointers in dest so nothing to be cleared. copying source */
	pDest->dType = pSource->dType;
	pDest->dTextType = pSource->dTextType;
	pDest->dNum = pSource->dNum;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipHeaderOrderInfo");
	return SipSuccess;
}



SipBool __sip_cloneSipRespMessage
#ifdef ANSI_PROTO
	(SipRespMessage *pDest, SipRespMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRespMessage *pDest;
	SipRespMessage *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipRespMessage");
	/* clear destination parameter */
	if (pDest->pStatusLine != SIP_NULL)
		sip_freeSipStatusLine(pDest->pStatusLine);
	if (pDest->pResponseHdr != SIP_NULL)
		sip_freeSipRespHeader (pDest->pResponseHdr);
	/* Now copying source */
	if (sip_initSipStatusLine (&(pDest->pStatusLine), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipStatusLine(pDest->pStatusLine, pSource->pStatusLine, pErr) == SipFail)
		return SipFail;
	if (sip_initSipRespHeader(&(pDest->pResponseHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipRespHeader(pDest->pResponseHdr, pSource->pResponseHdr, pErr) == SipFail)
		return SipFail;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRespMessage");
	return SipSuccess;
}



SipBool __sip_cloneSipMessage
#ifdef ANSI_PROTO
	(SipMessage *pDest, SipMessage *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipMessage *pDest;
	SipMessage *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipMessage");
	/* clear destination Message */
	if (sip_listDeleteAll(&(pDest->slOrderInfo), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slMessageBody), pErr) == SipFail)
		return SipFail;
	sip_freeSipGeneralHeader (pDest->pGeneralHdr);
	switch (pDest->dType)
	{
		case SipMessageRequest:
			sip_freeSipReqMessage ((pDest->u).pRequest);
			break;
		case SipMessageResponse:
			sip_freeSipRespMessage ((pDest->u).pResponse);
			break;
		case SipMessageAny:
			break;
		default:
			*pErr = E_INV_PARAM;
			return SipFail;
	}
	/* copy Message Type */
	pDest->dType = pSource->dType;
	/* copy Order Info */
	if (__sip_cloneSipOrderInfoList(&(pDest->slOrderInfo),\
		&(pSource->slOrderInfo), pErr) == SipFail)
		return SipFail;
	/* copy Message Body list */
	if (__sip_cloneSipMessageBodyList(&(pDest->slMessageBody),\
		&(pSource->slMessageBody), pErr) == SipFail)
		return SipFail;
	/* copy General Headsers */
	if (sip_initSipGeneralHeader (&(pDest->pGeneralHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipGeneralHeader(pDest->pGeneralHdr, pSource->pGeneralHdr,\
		pErr) == SipFail)
		return SipFail;
	/* Copy REQUEST/RESPONSE headers */
	switch(pSource->dType)
	{
		case SipMessageRequest:
			if (sip_initSipReqMessage(&(pDest->u.pRequest), pErr) == SipFail)
				return SipFail;
			if (__sip_cloneSipReqMessage(pDest->u.pRequest,\
				pSource->u.pRequest, pErr) == SipFail)
			{
				sip_freeSipReqMessage (pDest->u.pRequest);
				return SipFail;
			}
			break;
		case SipMessageResponse:
			if (sip_initSipRespMessage(&(pDest->u.pResponse), pErr) == SipFail)
				return SipFail;
			if (__sip_cloneSipRespMessage (pDest->u.pResponse,\
				pSource->u.pResponse, pErr) == SipFail)
			{
				sip_freeSipRespMessage (pDest->u.pResponse);
				return SipFail;
			}
			break;
		case SipMessageAny:
			break;
		default:
			*pErr = E_INV_PARAM;
			return SipFail;
	}
	pDest->dIncorrectHdrsCount = pSource->dIncorrectHdrsCount;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipMessage");
	return SipSuccess;
}


SipBool __sip_cloneSipServerHeader
#ifdef ANSI_PROTO
	(SipServerHeader *pDest, SipServerHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipServerHeader *pDest;
	SipServerHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipServerHeader");

	if (pDest->pValue != SIP_NULL)
		sip_freeString(pDest->pValue);

	if (pSource->pValue == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pValue);
		if (temp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pValue = temp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipServerHeader");
	return SipSuccess;
}


SipBool __sip_cloneSipUnsupportedHeader
#ifdef ANSI_PROTO
	(SipUnsupportedHeader *pDest, SipUnsupportedHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipUnsupportedHeader *pDest;
	SipUnsupportedHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function __sip_cloneSipUnsupportedHeader");

	if (pDest->pOption != SIP_NULL)
		sip_freeString(pDest->pOption);

	if (pSource->pOption == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pOption);
		if (temp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pOption = temp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipUnsupportedHeader");
	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: __sip_cloneSipSupportedHeader
**
** DESCRIPTION: This function makes a deep copy of the pSource structure fields to pDest structure fileds.
**
*****************************************************************/
SipBool __sip_cloneSipSupportedHeader
#ifdef ANSI_PROTO
	(SipSupportedHeader *pDest, SipSupportedHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipSupportedHeader *pDest;
	SipSupportedHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering function __sip_cloneSipSupportedHeader");

	/* freeing up existing values */
	if (pDest->pOption != SIP_NULL)
	{
		sip_freeString(pDest->pOption);
		pDest->pOption = SIP_NULL;
	}

	/* copying */
	if (pSource->pOption == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pOption);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pOption = pTemp;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipSupportedHeader");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipCallInfoHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the CallInfoHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipCallInfoHeader
#ifdef ANSI_PROTO
	(SipCallInfoHeader *dest, SipCallInfoHeader *source, SipError *err)
#else
	(dest, source, err)
	SipCallInfoHeader *dest;
	SipCallInfoHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;

	SIPDEBUGFN("Entering function __sip_cloneSipCallInfoHeader");
	if (dest->pUri != SIP_NULL)
		sip_freeString(dest->pUri);
	if(sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	if (source->pUri == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pUri)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pUri = temp;

	/* copying siplist of SipParmas */

	if(__sip_cloneSipParamList(&(dest->slParam),&(source->slParam),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipCallInfoHeader");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipContentDispositionHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ContentDispositionHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipContentDispositionHeader
#ifdef ANSI_PROTO
	(SipContentDispositionHeader *dest, SipContentDispositionHeader *source, SipError *err)
#else
	(dest, source, err)
	SipContentDispositionHeader *dest;
	SipContentDispositionHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;

	SIPDEBUGFN("Entering function __sip_cloneSipContentDispositionHeader");
	if (dest->pDispType != SIP_NULL)
		sip_freeString(dest->pDispType);
	if(sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	if (source->pDispType == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pDispType)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pDispType = temp;

	/* copying siplist of SipParmas */

	if(__sip_cloneSipParamList(&(dest->slParam),&(source->slParam),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipContentDispositionHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipReasonHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ReasonHeader structures "pSource" to "pDest".
**
******************************************************************/
SipBool __sip_cloneSipReasonHeader
#ifdef ANSI_PROTO
	(SipReasonHeader *pDest, SipReasonHeader *pSource, \
		SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipReasonHeader *pDest;
	SipReasonHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;

	SIPDEBUGFN("Entering function __sip_cloneSipReasonHeader");
	if (pDest->pDispType != SIP_NULL)
		sip_freeString(pDest->pDispType);
	if(sip_listDeleteAll(&(pDest->slParam), pErr) == SipFail)
		return SipFail;

	if (pSource->pDispType == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		if ((pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pDispType)) \
			== SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pDispType = pTemp;

	/* copying siplist of SipParmas */

	if(__sip_cloneSipParamList(&(pDest->slParam),&(pSource->slParam),pErr)==\
		SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipReasonHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipAlertInfoHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the AlertInfoHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipAlertInfoHeader
#ifdef ANSI_PROTO
	(SipAlertInfoHeader *dest, SipAlertInfoHeader *source, SipError *err)
#else
	(dest, source, err)
	SipAlertInfoHeader *dest;
	SipAlertInfoHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;

	SIPDEBUGFN("Entering function __sip_cloneSipAlertInfoHeader");
	if (dest->pUri != SIP_NULL)
		sip_freeString(dest->pUri);
	if(sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	if (source->pUri == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pUri)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pUri = temp;

	/* copying siplist of SipParmas */

	if(__sip_cloneSipParamList(&(dest->slParam),&(source->slParam),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAlertInfoHeader");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipErrorInfoHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ErrorInfoHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipErrorInfoHeader
#ifdef ANSI_PROTO
	(SipErrorInfoHeader *dest, SipErrorInfoHeader *source, SipError *err)
#else
	(dest, source, err)
	SipErrorInfoHeader *dest;
	SipErrorInfoHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;

	SIPDEBUGFN("Entering function __sip_cloneSipErrorInfoHeader");
	if (dest->pUri != SIP_NULL)
		sip_freeString(dest->pUri);
	if(sip_listDeleteAll(&(dest->slParam), err) == SipFail)
		return SipFail;

	if (source->pUri == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		if ((temp = (SIP_S8bit *)STRDUPACCESSOR(source->pUri)) == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pUri = temp;

	/* copying siplist of SipParmas */

	if(__sip_cloneSipParamList(&(dest->slParam),&(source->slParam),err)==SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipErrorInfoHeader");
	return SipSuccess;
}


#ifdef SIP_SESSIONTIMER
/******************************************************************
**
** FUNCTION:  __sip_cloneSipMinSEHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the MinSEHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipMinSEHeader
#ifdef ANSI_PROTO
	(SipMinSEHeader *pDest, SipMinSEHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipMinSEHeader *pDest;
	SipMinSEHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_U32bit size=0,index=0;
	SipNameValuePair *clone_namevalue;
	SIP_Pvoid element_from_src;
	SIPDEBUGFN("Entering function __sip_cloneSipMinSEHeader");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;
	if ( pDest == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSource == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	pDest->dSec = pSource->dSec;
	
	if ( sip_listDeleteAll(&(pDest->slNameValue), pErr) == SipFail)
    	return SipFail;
    if ( sip_listSizeOf(&(pSource->slNameValue), &size, pErr) == SipFail)
    	return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		element_from_src = SIP_NULL;
		if( sip_listGetAt(&(pSource->slNameValue),index, (SIP_Pvoid * )\
			(&element_from_src), pErr) == SipFail )
        {
	         return SipFail;
        }
        if (element_from_src == SIP_NULL)
        	clone_namevalue = SIP_NULL;
        else
        {
        	if ( sip_initSipNameValuePair(&clone_namevalue, pErr) == SipFail)
        		return SipFail;
        	if ( __sip_cloneSipNameValuePair( clone_namevalue, \
        		(SipNameValuePair *)element_from_src, pErr) == SipFail)
        	{
        		sip_freeSipNameValuePair(clone_namevalue);
        		return SipFail;
        	}
        }
 		if (sip_listAppend (&(pDest->slNameValue), \
			(SIP_Pvoid *)clone_namevalue, pErr) == SipFail)
		{
			if (clone_namevalue != SIP_NULL)
				sip_freeSipNameValuePair(clone_namevalue);
                        return SipFail;
	    }

	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipMinSEHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipSessionExpiresHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the SessionExpiresHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipSessionExpiresHeader
#ifdef ANSI_PROTO
	(SipSessionExpiresHeader *dest, SipSessionExpiresHeader *source, SipError *err)
#else
	(dest, source, err)
	SipSessionExpiresHeader *dest;
	SipSessionExpiresHeader *source;
	SipError *err;
#endif
{
	SIP_U32bit size=0,index=0;
	SipNameValuePair *clone_namevalue;
	SIP_Pvoid element_from_src;
	SIPDEBUGFN("Entering function __sip_cloneSipSessionExpiresHeader");
	dest->dSec = source->dSec;

	if ( sip_listDeleteAll(&(dest->slNameValue), err) == SipFail)
    	return SipFail;
    if ( sip_listSizeOf(&(source->slNameValue), &size, err) == SipFail)
    	return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		element_from_src = SIP_NULL;
		if( sip_listGetAt(&(source->slNameValue),index, (SIP_Pvoid * )\
			(&element_from_src), err) == SipFail )
        {
	         return SipFail;
        }
        if (element_from_src == SIP_NULL)
        	clone_namevalue = SIP_NULL;
        else
        {
        	if ( sip_initSipNameValuePair(&clone_namevalue, err) == SipFail)
        		return SipFail;
        	if ( __sip_cloneSipNameValuePair( clone_namevalue, \
        		(SipNameValuePair *)element_from_src, err) == SipFail)
        	{
        		sip_freeSipNameValuePair(clone_namevalue);
        		return SipFail;
        	}
        }
 		if (sip_listAppend (&(dest->slNameValue), \
			(SIP_Pvoid *)clone_namevalue, err) == SipFail)
		{
			if (clone_namevalue != SIP_NULL)
				sip_freeSipNameValuePair(clone_namevalue);
                        return SipFail;
	    }

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipSessionExpiresHeader");
	return SipSuccess;
}
#endif

/******************************************************************
**
** FUNCTION:  __sip_cloneSipBadHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the FromHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipBadHeader
#ifdef ANSI_PROTO
	(SipBadHeader *dest, SipBadHeader *source, SipError *err)
#else
	(dest, source, err)
	SipBadHeader *dest;
	SipBadHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SIPDEBUGFN("Entering function ___sip_cloneSipBadHeader");
	if (dest->pName != SIP_NULL)
		sip_freeString(dest->pName);
	if (dest->pBody != SIP_NULL)
		sip_freeString(dest->pBody);

	/* Duplicating pDispName */
	if (source->pName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pName);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pName = temp;

	if (source->pBody == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pBody);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pBody = temp;

    dest->dType =source->dType;
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function ___sip_cloneSipBadHeader");
	return SipSuccess;
}

SipBool __sip_cloneSipAuthorizationHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAuthorizationHeader *temp_auth, *clone_auth;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAuthorizationHeaderList");
	/* copying siplist of SipAuthorizationHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_auth), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_auth == SIP_NULL )
			clone_auth = SIP_NULL;
		else
		{
			if(sip_initSipAuthorizationHeader(&clone_auth, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_auth == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAuthorizationHeader(  clone_auth, temp_auth, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_auth), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_auth, pErr) == SipFail )
		{
			if ( clone_auth != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_auth), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAuthorizationHeaderList");
	return SipSuccess;
}
/******************************************************************
**
** FUNCTION:  __sip_cloneSipAuthenticationInfoHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the AuthenticationInfoHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipAuthenticationInfoHeader
#ifdef ANSI_PROTO
	(SipAuthenticationInfoHeader *pDest, SipAuthenticationInfoHeader *pSource,\
	 SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipAuthenticationInfoHeader *pDest;
	SipAuthenticationInfoHeader *pSource;
	SipError *pErr;
#endif
{

	SIP_Pvoid element_from_src;
	SipNameValuePair *clone_namevalue;
	SIP_U32bit size=0,index=0;
	SIPDEBUGFN("Entering function __sip_cloneSipAuthenticationInfoHeader");

	element_from_src = SIP_NULL;

	if ( sip_listDeleteAll(&(pDest->slNameValue), pErr) == SipFail)
    	return SipFail;
    if ( sip_listSizeOf(&(pSource->slNameValue), &size, pErr) == SipFail)
    	return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		element_from_src = SIP_NULL;
		if( sip_listGetAt(&(pSource->slNameValue),index, (SIP_Pvoid * )\
			(&element_from_src), pErr) == SipFail )
        {
	         return SipFail;
        }
        if (element_from_src == SIP_NULL)
        	clone_namevalue = SIP_NULL;
        else
        {
        	if ( sip_initSipNameValuePair(&clone_namevalue, pErr) == SipFail)
        		return SipFail;
        	if ( __sip_cloneSipNameValuePair( clone_namevalue, \
        		(SipNameValuePair *)element_from_src, pErr) == SipFail)
        	{
        		sip_freeSipNameValuePair(clone_namevalue);
        		return SipFail;
        	}
        }
 		if (sip_listAppend (&(pDest->slNameValue), \
			(SIP_Pvoid *)clone_namevalue, pErr) == SipFail)
		{
			if (clone_namevalue != SIP_NULL)
				sip_freeSipNameValuePair(clone_namevalue);
                        return SipFail;
	    }

	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAuthenticationInfoHeader");
	return SipSuccess;
}

SipBool __sip_cloneSipAuthenticationInfoHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAuthenticationInfoHeader *pTemp_authenticate, *pClone_authenticate;
	SIP_U32bit dCount, dIndex;

	SIPDEBUGFN("Entering function __sip_cloneSipAuthenticationInfoHeaderList");
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &dCount, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( dIndex = 0; dIndex < dCount ; dIndex++)
	{
		if( sip_listGetAt(pSource,dIndex, (SIP_Pvoid * ) (&pTemp_authenticate), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTemp_authenticate == SIP_NULL )
			pClone_authenticate = SIP_NULL;
		else
		{
			if(sip_initSipAuthenticationInfoHeader(&pClone_authenticate, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( pClone_authenticate == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAuthenticationInfoHeader(  pClone_authenticate, pTemp_authenticate, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pClone_authenticate), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pClone_authenticate, pErr) == SipFail )
		{
			if ( pClone_authenticate != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pClone_authenticate), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAuthenticationInfoHeaderList");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipReplyToHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the ReplyToHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipReplyToHeader
#ifdef ANSI_PROTO
	(SipReplyToHeader *dest, SipReplyToHeader *source, SipError *err)
#else
	(dest, source, err)
	SipReplyToHeader *dest;
	SipReplyToHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SipNameValuePair* pElement_from_source=SIP_NULL;
	SipNameValuePair* pElement_into_dest=SIP_NULL;
	SIP_U32bit dIndex=0,dSize=0;

	SIPDEBUGFN("Entering function __sip_cloneSipReplyToHeader");
	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);
	if (dest->pAddrSpec != SIP_NULL)
		sip_freeSipAddrSpec(dest->pAddrSpec);
	if (sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	/* Duplicating pDispName */
	if (source->pDispName == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pDispName);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pDispName = temp;
	/* Duplicating pAddrSpec */
	if (validateSipAddrSpecType((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}

	/* clone generic param */
	if ( sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	if ( sip_listSizeOf(&(source->slParams), &dSize, err) == SipFail)
		return SipFail;

	for ( dIndex = 0; dIndex < dSize ; dIndex++ )
	{
		if( sip_listGetAt\
			(&(source->slParams),dIndex, (SIP_Pvoid * ) (&pElement_from_source),\
				err) == SipFail )
		{
			return SipFail;
		}
		if (pElement_from_source == SIP_NULL)
			pElement_into_dest = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pElement_into_dest, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pElement_into_dest,\
					pElement_from_source, err) == SipFail)
			{
				sip_freeSipNameValuePair(pElement_into_dest);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slParams),\
			(SIP_Pvoid *)pElement_into_dest, err) == SipFail)
		{
			if (pElement_into_dest != SIP_NULL)
				sip_freeSipNameValuePair (pElement_into_dest);
			return SipFail;
		}

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipReplyToHeader");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipReplacesHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the ReplacesHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipReplacesHeader
#ifdef ANSI_PROTO
	(SipReplacesHeader *dest, SipReplacesHeader *source, SipError *err)
#else
	(dest, source, err)
	SipReplacesHeader *dest;
	SipReplacesHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SipNameValuePair* pElement_from_source=SIP_NULL;
	SipNameValuePair* pElement_into_dest=SIP_NULL;
	SIP_U32bit dIndex=0,dSize=0;
	SIPDEBUGFN("Entering function __sip_cloneSipReplacesHeader");
	if (dest->pCallid != SIP_NULL)
		sip_freeString(dest->pCallid);
	if (dest->pFromTag != SIP_NULL)
		sip_freeString(dest->pFromTag);
	if (dest->pToTag != SIP_NULL)
		sip_freeString(dest->pToTag);
	if (sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	/* Duplicating pCallid */
	if (source->pCallid == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pCallid);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pCallid = temp;

	/* Duplicating pFromTag */
	if (source->pFromTag == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pFromTag);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pFromTag = temp;

	/* Duplicating pToTag */
	if (source->pToTag == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pToTag);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
	}
	dest->pToTag = temp;

	/* clone generic param */

	if ( sip_listDeleteAll(&(dest->slParams), err) == SipFail)
		return SipFail;

	if ( sip_listSizeOf(&(source->slParams), &dSize, err) == SipFail)
		return SipFail;

	for ( dIndex = 0; dIndex < dSize ; dIndex++ )
	{
		if( sip_listGetAt\
			(&(source->slParams),dIndex, (SIP_Pvoid * ) (&pElement_from_source),\
				err) == SipFail )
		{
			return SipFail;
		}
		if (pElement_from_source == SIP_NULL)
			pElement_into_dest = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pElement_into_dest, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pElement_into_dest,\
					pElement_from_source, err) == SipFail)
			{
				sip_freeSipNameValuePair(pElement_into_dest);
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slParams),\
			(SIP_Pvoid *)pElement_into_dest, err) == SipFail)
		{
			if (pElement_into_dest != SIP_NULL)
				sip_freeSipNameValuePair (pElement_into_dest);
			return SipFail;
		}

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipReplacesHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipMinExpiresHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the MinExpiresHeader structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipMinExpiresHeader
#ifdef ANSI_PROTO
	(SipMinExpiresHeader *dest, SipMinExpiresHeader *source, SipError *err)
#else
	(dest, source, err)
	SipMinExpiresHeader *dest;
	SipMinExpiresHeader *source;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function __sip_cloneSipMinExpiresHeader");

	/* Duplicating seconds*/
	dest->dSec = source->dSec;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipMinExpiresHeader");
	return SipSuccess;
}

#ifdef SIP_PRIVACY
/******************************************************************
**
** FUNCTION:  __sip_cloneSipPAssertIdHeader
**
** DESCRIPTION:  This function makes a deep copy of the fields from
**	the PAssertId Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipPAssertIdHeader
#ifdef ANSI_PROTO
	(SipPAssertIdHeader *dest, SipPAssertIdHeader *source, SipError *err)
#else
	(dest, source, err)
	SipPAssertIdHeader *dest;
	SipPAssertIdHeader *source;
	SipError *err;
#endif
{
	SIP_U32bit dLength = 0;
	SIPDEBUGFN("Entering function __sip_cloneSipPAssertIdHeader");

	/* cleaning up dest */
	if (dest->pAddrSpec != SIP_NULL)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		dest->pAddrSpec = SIP_NULL;
	}
	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);

	/* copy the source parameters */
	if(source->pDispName== SIP_NULL)
	{
		dest->pDispName=SIP_NULL;
	}
	else
	{
		dLength = strlen(source->pDispName );
		dest->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,err);
		if ( dest->pDispName == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( dest->pDispName, source->pDispName );
	}

	/* clone pAddrSpec */
	if (validateSipAddrSpecType ((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}

	*err = E_NO_ERROR ;
	SIPDEBUGFN("Exiting function __sip_cloneSipPAssertIdHeader");
	return SipSuccess;
}

SipBool __sip_cloneSipPAssertIdHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipPAssertIdHeader *pPAssertId =SIP_NULL , *pClonePAssertId = SIP_NULL ;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipPAssertIdHeaderList");
	/* copying siplist of SipPAssertIdHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&pPAssertId), pErr)\
			== SipFail )
		{
			return SipFail;
		}
		if ( pPAssertId == SIP_NULL )
			pClonePAssertId = SIP_NULL;
		else
		{
			if(sip_initSipPAssertIdHeader(&pClonePAssertId, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( pClonePAssertId == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipPAssertIdHeader(pClonePAssertId, pPAssertId, \
				    pErr) == SipFail)
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pClonePAssertId),SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pClonePAssertId, pErr) == SipFail )
		{
			if ( pClonePAssertId != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pClonePAssertId), \
					SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipPAssertIdHeaderList");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipPPreferredIdHeader
**
** DESCRIPTION:  This function makes a deep copy of the fields from
**	the PPreferredHeader Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipPPreferredIdHeader
#ifdef ANSI_PROTO
	(SipPPreferredIdHeader *dest, SipPPreferredIdHeader *source, SipError *err)
#else
	(dest, source, err)
	SipPPreferredIdHeader *dest;
	SipPPreferredIdHeader *source;
	SipError *err;
#endif
{
	SIP_U32bit dLength = 0;
	SIPDEBUGFN("Entering function __sip_cloneSipPPreferredIdHeader");

	/* cleaning up dest */
	if (dest->pAddrSpec != SIP_NULL)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		dest->pAddrSpec = SIP_NULL;
	}
	if (dest->pDispName != SIP_NULL)
		sip_freeString(dest->pDispName);

	/* copy the source parameters */
	if(source->pDispName== SIP_NULL)
	{
		dest->pDispName=SIP_NULL;
	}
	else
	{
		dLength = strlen(source->pDispName );
		dest->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,err);
		if ( dest->pDispName == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( dest->pDispName, source->pDispName );
	}

	/* clone pAddrSpec */
	if (validateSipAddrSpecType ((source->pAddrSpec)->dType, err) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(dest->pAddrSpec), (source->pAddrSpec)->dType,\
		 err) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(dest->pAddrSpec, source->pAddrSpec, err)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(dest->pAddrSpec);
		return SipFail;
	}

	*err = E_NO_ERROR ;
	SIPDEBUGFN("Exiting function __sip_cloneSipPPreferredIdHeader");
	return SipSuccess;
}

SipBool __sip_cloneSipPPreferredIdHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipPPreferredIdHeader *pPPreferred=SIP_NULL , *pClonePPreferred = SIP_NULL ;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipPPreferredIdHeaderList");
	/* copying siplist of SipPPreferredHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&pPPreferred), pErr)\
			== SipFail )
		{
			return SipFail;
		}
		if ( pPPreferred == SIP_NULL )
			pClonePPreferred = SIP_NULL;
		else
		{
			if(sip_initSipPPreferredIdHeader(&pClonePPreferred, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( pClonePPreferred == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipPPreferredIdHeader(pClonePPreferred, 
				pPPreferred,  pErr) == SipFail)
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pClonePPreferred),SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pClonePPreferred, pErr) == SipFail )
		{
			if ( pClonePPreferred != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pClonePPreferred), \
					SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipPPreferredIdHeaderList");
	return SipSuccess;
}
/***********************************************************************
**
** FUNCTION : __sip_cloneSipPrivacyHeader
**
** DESCRIPTION : This function makes a deep copy of the fields from
**				the Privacy header strutures "source" to "dest"
**
***********************************************************************/
SipBool __sip_cloneSipPrivacyHeader
#ifdef ANSI_PROTO
		(SipPrivacyHeader *pDest, SipPrivacyHeader *pSource, SipError *pErr)
#else
		(pDest,pSource,pErr)
		SipPrivacyHeader *pDest;
		SipPrivacyHeader *pSource;
		SipError *pErr;
#endif
{
	
	SIP_U32bit size=0,index=0;
	SipNameValuePair *clone_namevalue;
	SIP_Pvoid element_from_src;
	SIPDEBUGFN("Entering function __sip_cloneSipPrivacyHeader");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;
	if ( pDest == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( pSource == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	
	if ( sip_listDeleteAll(&(pDest->slPrivacyValue), pErr) == SipFail)
    	return SipFail;
    if ( sip_listSizeOf(&(pSource->slPrivacyValue), &size, pErr) == SipFail)
    	return SipFail;
	for ( index = 0; index < size ; index++ )
	{
		element_from_src = SIP_NULL;
		if( sip_listGetAt(&(pSource->slPrivacyValue),index, (SIP_Pvoid * )\
			(&element_from_src), pErr) == SipFail )
        {
	         return SipFail;
        }
        if (element_from_src == SIP_NULL)
        	clone_namevalue = SIP_NULL;
        else
        {
        	if ( sip_initSipNameValuePair(&clone_namevalue, pErr) == SipFail)
        		return SipFail;
        	if ( __sip_cloneSipNameValuePair( clone_namevalue, \
        		(SipNameValuePair *)element_from_src, pErr) == SipFail)
        	{
        		sip_freeSipNameValuePair(clone_namevalue);
        		return SipFail;
        	}
        }
 		if (sip_listAppend (&(pDest->slPrivacyValue), \
			(SIP_Pvoid *)clone_namevalue, pErr) == SipFail)
		{
			if (clone_namevalue != SIP_NULL)
				sip_freeSipNameValuePair(clone_namevalue);
                        return SipFail;
	    }

	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipPrivacyEHeader");
	return SipSuccess;
}
#endif /* #ifdef SIP_PRIVACY */

/******************************************************************
**
** FUNCTION:  __sip_cloneSipPathHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the PathHeader structures "source" to "dest".
**
******************************************************************/
#ifdef SIP_3GPP
SipBool __sip_cloneSipPathHeader
#ifdef ANSI_PROTO
	(SipPathHeader *pDest, SipPathHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipPathHeader *pDest;
	SipPathHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering function __sip_cloneSipPathHeader");

	/* cleaning up dest */

	sip_freeString(pDest->pDispName);
	pDest->pDispName = SIP_NULL;

	sip_freeSipAddrSpec(pDest->pAddrSpec);
	pDest->pAddrSpec = SIP_NULL;

	if(sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;
	/* cleaned up */
	/* clone pDispName */
	if (pSource->pDispName == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pDispName);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pDispName = pTemp;

	/* clone pAddrSpec */
	if (validateSipAddrSpecType ((pSource->pAddrSpec)->dType, pErr) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(pDest->pAddrSpec), (pSource->pAddrSpec)->dType,\
		 pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(pDest->pAddrSpec, pSource->pAddrSpec, pErr)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpec);
		return SipFail;
	}

	if(__sip_cloneSipParamList(&(pDest->slParams),&(pSource->slParams),pErr)==SipFail)
		return SipFail;

	*pErr = E_NO_ERROR ;
	SIPDEBUGFN("Exiting function __sip_cloneSipPathHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipPathHeaderList
**
** DESCRIPTION:  This function makes a copy of path headers from
**	the source PathHeader List to to "dest".
**
******************************************************************/
SipBool __sip_cloneSipPathHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipPathHeader *pTempPath, *pClonePath;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipPathHeaderList");
	/* copying siplist of SipPathHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&pTempPath), pErr) \
			== SipFail )
		{
			return SipFail;
		}
		if ( pTempPath == SIP_NULL )
			pClonePath = SIP_NULL;
		else
		{
			if(sip_initSipPathHeader(&pClonePath, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( pClonePath == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipPathHeader(  pClonePath, pTempPath, pErr) \
				== SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pClonePath), \
					SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pClonePath, pErr) == SipFail )
		{
			if ( pClonePath != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pClonePath), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipPathHeaderList");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipPanInfoHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the P-Access-Network-Info Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipPanInfoHeader
#ifdef ANSI_PROTO
	(SipPanInfoHeader *pDest, SipPanInfoHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipPanInfoHeader *pDest;
	SipPanInfoHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SipNameValuePair* pElement_from_source=SIP_NULL;
    SipNameValuePair* pElement_from_dest=SIP_NULL;
	SIP_U32bit dIndex=0,dSize=0;
	SIPDEBUGFN("Entering function __sip_cloneSipPanInfoHeader");
	if (pDest->pAccessType != SIP_NULL)
		sip_freeString(pDest->pAccessType);
	if (sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;

	/* duplicating pAccessType */
	if (pSource->pAccessType == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR (pSource->pAccessType);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pAccessType = pTemp;

	
	if ( sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;

	if ( sip_listSizeOf(&(pSource->slParams), &dSize, pErr) == SipFail)
		return SipFail;

	for ( dIndex = 0; dIndex < dSize ; dIndex++ )
	{
		if( sip_listGetAt\
			(&(pSource->slParams),dIndex, (SIP_Pvoid * ) (&pElement_from_source),\
				pErr) == SipFail )
		{
			return SipFail;
		}
		if (pElement_from_source == SIP_NULL)
			pElement_from_dest = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pElement_from_dest, pErr) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pElement_from_dest,\
					pElement_from_source, pErr) == SipFail)
			{
				sip_freeSipNameValuePair(pElement_from_dest);
				return SipFail;
			}
		}
		if (sip_listAppend (&(pDest->slParams),\
			(SIP_Pvoid *)pElement_from_dest, pErr) == SipFail)
		{
			if (pElement_from_dest != SIP_NULL)
				sip_freeSipNameValuePair (pElement_from_dest);
			return SipFail;
		}

	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_cloneSipPanInfoHeader");
	return SipSuccess;
	}



/******************************************************************
**
** FUNCTION:  __sip_cloneSipPcVectorHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the P-Charging-Vector Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipPcVectorHeader
#ifdef ANSI_PROTO
	(SipPcVectorHeader *pDest, SipPcVectorHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipPcVectorHeader *pDest;
	SipPcVectorHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SipNameValuePair* pElement_from_source=SIP_NULL;
    SipNameValuePair* pElement_from_dest=SIP_NULL;
	SIP_U32bit dSize=0,dIndex=0;
	SIPDEBUGFN("Entering function __sip_cloneSipPcVectorHeader");
	if (pDest->pAccessType != SIP_NULL)
		sip_freeString(pDest->pAccessType);
	if (pDest->pAccessValue != SIP_NULL)
		sip_freeString(pDest->pAccessValue);
	if (sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;

	/* duplicating pAccessType */
	if (pSource->pAccessType == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR (pSource->pAccessType);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pAccessType = pTemp;

	/* duplicating pAccessValue */
	if (pSource->pAccessValue == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR (pSource->pAccessValue);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pAccessValue = pTemp;
	if ( sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;

	if ( sip_listSizeOf(&(pSource->slParams), &dSize, pErr) == SipFail)
		return SipFail;

	for ( dIndex = 0; dIndex < dSize ; dIndex++ )
	{
		if( sip_listGetAt\
			(&(pSource->slParams),dIndex, (SIP_Pvoid * ) (&pElement_from_source),\
				pErr) == SipFail )
		{
			return SipFail;
		}
		if (pElement_from_source == SIP_NULL)
			pElement_from_dest = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pElement_from_dest, pErr) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pElement_from_dest,\
					pElement_from_source, pErr) == SipFail)
			{
				sip_freeSipNameValuePair(pElement_from_dest);
				return SipFail;
			}
		}
		if (sip_listAppend (&(pDest->slParams),\
			(SIP_Pvoid *)pElement_from_dest, pErr) == SipFail)
		{
			if (pElement_from_dest != SIP_NULL)
				sip_freeSipNameValuePair (pElement_from_dest);
			return SipFail;
		}

	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_cloneSipPcVectorHeader");
	return SipSuccess;
	}




#endif

#ifdef SIP_CONF
/******************************************************************
**
** FUNCTION:  __sip_cloneSipJoinHeader
**
** DESCRIPTION: This function makes a deep copy of the fields from
**	the JoinHeader structure "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipJoinHeader
#ifdef ANSI_PROTO
	(SipJoinHeader *dest, SipJoinHeader *source, SipError *err)
#else
	(dest, source, err)
	SipJoinHeader *dest;
	SipJoinHeader *source;
	SipError *err;
#endif
{
	SIP_S8bit *temp;
	SipNameValuePair* pElement_from_source=SIP_NULL;
	SipNameValuePair* pElement_into_dest=SIP_NULL;
	SIP_U32bit dIndex=0,dSize=0;
	SIPDEBUGFN("Entering function __sip_cloneSipJoinHeader");
	if (dest->pCallid != SIP_NULL)
		sip_freeString(dest->pCallid);
	if (dest->pFromTag != SIP_NULL)
		sip_freeString(dest->pFromTag);
	if (dest->pToTag != SIP_NULL)
		sip_freeString(dest->pToTag);
	if (sip_listDeleteAll(&(dest->slParams), err) == SipFail)
    {
	    SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
		return SipFail;
    }

	/* Duplicating pCallid */
	if (source->pCallid == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pCallid);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
	        SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
			return SipFail;
		}
	}
	dest->pCallid = temp;

	/* Duplicating pFromTag */
	if (source->pFromTag == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pFromTag);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
	        SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
			return SipFail;
		}
	}
	dest->pFromTag = temp;

	/* Duplicating pToTag */
	if (source->pToTag == SIP_NULL)
		temp = SIP_NULL;
	else
	{
		temp = (SIP_S8bit *)STRDUPACCESSOR(source->pToTag);
		if (temp == SIP_NULL)
		{
			*err = E_NO_MEM;
	        SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
			return SipFail;
		}
	}
	dest->pToTag = temp;

	/* clone generic param */

	if ( sip_listDeleteAll(&(dest->slParams), err) == SipFail)
    {
	    SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
		return SipFail;
    }

	if ( sip_listSizeOf(&(source->slParams), &dSize, err) == SipFail)
		return SipFail;

	for ( dIndex = 0; dIndex < dSize ; dIndex++ )
	{
		if( sip_listGetAt\
			(&(source->slParams),dIndex, (SIP_Pvoid * ) (&pElement_from_source),\
				err) == SipFail )
		{
	        SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
			return SipFail;
		}
		if (pElement_from_source == SIP_NULL)
			pElement_into_dest = SIP_NULL;
		else
		{
			if ( sip_initSipNameValuePair (&pElement_into_dest, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipNameValuePair (pElement_into_dest,\
					pElement_from_source, err) == SipFail)
			{
				sip_freeSipNameValuePair(pElement_into_dest);
	            SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
				return SipFail;
			}
		}
		if (sip_listAppend (&(dest->slParams),\
			(SIP_Pvoid *)pElement_into_dest, err) == SipFail)
		{
            if (pElement_into_dest != SIP_NULL)
				sip_freeSipNameValuePair (pElement_into_dest);
	       
            SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
			return SipFail;
		}

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_cloneSipJoinHeader");
	return SipSuccess;
}
#endif
#ifdef SIP_SECURITY
/******************************************************************
**
** FUNCTION:  __sip_cloneSipSecurityClientHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**      the Security-Client Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipSecurityClientHeader
        (SipSecurityClientHeader *dest, 
	SipSecurityClientHeader *source, 
	SipError *err)
{
        SIP_S8bit *temp;

        SIPDEBUGFN("Entering function __sip_cloneSipSecurityClientHeader");
        if (dest->pMechanismName != SIP_NULL)
                sip_freeString(dest->pMechanismName);
        
	if (sip_listDeleteAll(&(dest->slParams), err) == SipFail)
                return SipFail;

        /* duplicating mechanism name */
        if (source->pMechanismName == SIP_NULL)
                temp = SIP_NULL;
        else
        {
                temp = (SIP_S8bit *)STRDUPACCESSOR (source->pMechanismName);
                if (temp == SIP_NULL)
                {
                        *err = E_NO_MEM;
                        return SipFail;
                }
        }
	dest->pMechanismName = temp;

        if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams),err)==SipFail)
                return SipFail;

        *err = E_NO_ERROR;
        SIPDEBUGFN("Exiting function __sip_cloneSipSecurityClientHeader");
        return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipSecurityVerifyHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**      the Security-Verify Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipSecurityVerifyHeader
        (SipSecurityVerifyHeader *dest, 
	SipSecurityVerifyHeader *source, 
	SipError *err)
{
        SIP_S8bit *temp;

        SIPDEBUGFN("Entering function __sip_cloneSipSecurityVerifyHeader");
        if (dest->pMechanismName != SIP_NULL)
                sip_freeString(dest->pMechanismName);

        if (sip_listDeleteAll(&(dest->slParams), err) == SipFail)
                return SipFail;

        /* duplicating mechanism name */
        if (source->pMechanismName == SIP_NULL)
                temp = SIP_NULL;
        else
        {
                temp = (SIP_S8bit *)STRDUPACCESSOR (source->pMechanismName);
                if (temp == SIP_NULL)
                {
                        *err = E_NO_MEM;
                        return SipFail;
                }
        }
        dest->pMechanismName = temp;
	if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams),err)==SipFail)
                return SipFail;

        *err = E_NO_ERROR;
        SIPDEBUGFN("Exiting function __sip_cloneSipSecurityVerifyHeader");
        return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipSecurityServerHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**      the Security-Server Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipSecurityServerHeader
        (SipSecurityServerHeader *dest, 
	SipSecurityServerHeader *source, 
	SipError *err)
{
        SIP_S8bit *temp;

        SIPDEBUGFN("Entering function __sip_cloneSipSecurityServerHeader");
        if (dest->pMechanismName != SIP_NULL)
                sip_freeString(dest->pMechanismName);

        if (sip_listDeleteAll(&(dest->slParams), err) == SipFail)
                return SipFail;

        /* duplicating mechanism name */
        if (source->pMechanismName == SIP_NULL)
                temp = SIP_NULL;
        else
        {
                temp = (SIP_S8bit *)STRDUPACCESSOR (source->pMechanismName);
                if (temp == SIP_NULL)
                {
                        *err = E_NO_MEM;
                        return SipFail;
                }
        }
        dest->pMechanismName = temp;
	if(__sip_cloneSipParamList(&(dest->slParams),&(source->slParams),err)==SipFail)
                return SipFail;

        *err = E_NO_ERROR;
        SIPDEBUGFN("Exiting function __sip_cloneSipSecurityServerHeader");
        return SipSuccess;
}
#endif /*end of SIP_SECURITY */

#ifdef SIP_3GPP
/*********************************************************
** FUNCTION:__sip_cloneSipPAssociatedUriHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the PAssociatedUriHeader structures "source" to "dest".
**
*******************************************************/

SipBool __sip_cloneSipPAssociatedUriHeader
# ifdef ANSI_PROTO
	(SipPAssociatedUriHeader *to,
	SipPAssociatedUriHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipPAssociatedUriHeader *to;
	SipPAssociatedUriHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipPAssociatedUriHeader");
	/* clean up of to */

	if(to->pDispName != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pDispName)), err) == SipFail)
			return SipFail;
	}
	sip_freeSipAddrSpec(to->pAddrSpec);

	if(sip_listDeleteAll(&(to->slParams), err) == SipFail)
		return SipFail;
	/* clean up over */

	if(from->pDispName== SIP_NULL)
		{
			to->pDispName=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pDispName );

			to->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pDispName == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pDispName, from->pDispName );
		}

	if(sip_initSipAddrSpec(&to->pAddrSpec,from->pAddrSpec->dType,err)==SipFail)
	{
		return SipFail;
	}
	if(__sip_cloneSipAddrSpec(to->pAddrSpec,from->pAddrSpec,err)==SipFail)
	{
		sip_freeSipAddrSpec(to->pAddrSpec);
		return SipFail;
	}
	if(__sip_cloneSipParamList(&(to->slParams),&(from->slParams),err)==SipFail)
		return SipFail;

	SIPDEBUGFN( "Exiting __sip_cloneSipPAssociatedUriHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/***********************************************************************
** FUNCTION:__sip_cloneSipPCalledPartyIdHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the PCalledPartyIdHeader structures "source" to "dest".
**
************************************************************************/

SipBool __sip_cloneSipPCalledPartyIdHeader
# ifdef ANSI_PROTO
	(SipPCalledPartyIdHeader *to,
	SipPCalledPartyIdHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipPCalledPartyIdHeader *to;
	SipPCalledPartyIdHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipPCalledPartyIdHeader");
	/* clean up of to */

	if(to->pDispName != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pDispName)), err) == SipFail)
			return SipFail;
	}
	sip_freeSipAddrSpec(to->pAddrSpec);

	if(sip_listDeleteAll(&(to->slParams), err) == SipFail)
		return SipFail;
	/* clean up over */

	if(from->pDispName== SIP_NULL)
		{
			to->pDispName=SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pDispName );

			to->pDispName=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pDispName == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pDispName, from->pDispName );
		}

	if(sip_initSipAddrSpec(&to->pAddrSpec,from->pAddrSpec->dType,err)==SipFail)
	{
		return SipFail;
	}
	if(__sip_cloneSipAddrSpec(to->pAddrSpec,from->pAddrSpec,err)==SipFail)
	{
		sip_freeSipAddrSpec(to->pAddrSpec);
		return SipFail;
	}
	if(__sip_cloneSipParamList(&(to->slParams),&(from->slParams),err)==SipFail)
		return SipFail;

	SIPDEBUGFN( "Exiting __sip_cloneSipPCalledPartyIdHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:__sip_cloneSipPAssociatedUriHeaderList
**
** DESCRIPTION:  This function makes a deep copy of the whole list of 
**	the PAssociatedUriHeader structures "source" to "dest".
**
**********************************************************************/

SipBool __sip_cloneSipPAssociatedUriHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipPAssociatedUriHeader *temp_PAssociatedUri, *clone_PAssociatedUri;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipPAssociatedUriHeaderList");
	/* copying siplist of SipPAssociatedUriHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_PAssociatedUri), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_PAssociatedUri == SIP_NULL )
			clone_PAssociatedUri = SIP_NULL;
		else
		{
			if(sip_initSipPAssociatedUriHeader(&clone_PAssociatedUri, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_PAssociatedUri == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipPAssociatedUriHeader(  clone_PAssociatedUri, temp_PAssociatedUri, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_PAssociatedUri), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_PAssociatedUri, pErr) == SipFail )
		{
			if ( clone_PAssociatedUri != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_PAssociatedUri), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipPAssociatedUriHeaderList");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneSipPVisitedNetworkIdHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the PVisitedNetworkIdHeader structures "source" to "dest".
**
*******************************************************/

SipBool __sip_cloneSipPVisitedNetworkIdHeader
# ifdef ANSI_PROTO
	(SipPVisitedNetworkIdHeader *to,
	SipPVisitedNetworkIdHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipPVisitedNetworkIdHeader *to;
	SipPVisitedNetworkIdHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipPVisitedNetworkIdHeader");
	/* clean up of to */

	if(to->pVNetworkSpec != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pVNetworkSpec)), err) == SipFail)
			return SipFail;
	}

	if(sip_listDeleteAll(&(to->slParams), err) == SipFail)
		return SipFail;
	/* clean up over */

	if(from->pVNetworkSpec == SIP_NULL)
		{
			to->pVNetworkSpec = SIP_NULL;
		}
		else
		{
			dLength = strlen(from->pVNetworkSpec);

			to->pVNetworkSpec = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
			if ( to->pVNetworkSpec == SIP_NULL )
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			strcpy( to->pVNetworkSpec, from->pVNetworkSpec);
		}

	if(__sip_cloneSipParamList(&(to->slParams),&(from->slParams),err)==SipFail)
		return SipFail;

	SIPDEBUGFN( "Exiting __sip_cloneSipPVisitedNetworkIdHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:__sip_cloneSipPVisitedNetworkIdHeaderList
**
** DESCRIPTION:  This function makes a deep copy of the whole list of 
**	the PVisitedNetworkIdHeader structures "source" to "dest".
**
**********************************************************************/

SipBool __sip_cloneSipPVisitedNetworkIdHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipPVisitedNetworkIdHeader *temp_PVisitedNetworkId, *clone_PVisitedNetworkId;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipPVisitedNetworkIdHeaderList");
	/* copying siplist of SipPVisitedNetworkIdHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_PVisitedNetworkId), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_PVisitedNetworkId == SIP_NULL )
			clone_PVisitedNetworkId = SIP_NULL;
		else
		{
			if(sip_initSipPVisitedNetworkIdHeader(&clone_PVisitedNetworkId, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_PVisitedNetworkId == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipPVisitedNetworkIdHeader(  clone_PVisitedNetworkId, temp_PVisitedNetworkId, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_PVisitedNetworkId), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_PVisitedNetworkId, pErr) == SipFail )
		{
			if ( clone_PVisitedNetworkId != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_PVisitedNetworkId), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipPVisitedNetworkIdHeaderList");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:__sip_cloneSipPcfAddrHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the PcfAddrHeader structures "source" to "dest".
**
*******************************************************/

SipBool __sip_cloneSipPcfAddrHeader
# ifdef ANSI_PROTO
	(SipPcfAddrHeader *to,
	SipPcfAddrHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipPcfAddrHeader *to;
	SipPcfAddrHeader *from;
	SipError *err;
#endif
{
	SIPDEBUGFN( "Entering __sip_cloneSipPcfAddrHeader");
	/* clean up of to */

	if(sip_listDeleteAll(&(to->slParams), err) == SipFail)
		return SipFail;
	/* clean up over */

	if(__sip_cloneSipParamList(&(to->slParams),&(from->slParams),err)==SipFail)
		return SipFail;

	SIPDEBUGFN( "Exiting __sip_cloneSipPcfAddrHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

#endif

#ifdef SIP_CONGEST
/*********************************************************
** FUNCTION:__sip_cloneSipRsrcPriorityHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the RsrcPriorityHeader structures "source" to "dest".
**
*******************************************************/

SipBool __sip_cloneSipRsrcPriorityHeader
# ifdef ANSI_PROTO
	(SipRsrcPriorityHeader *to,
	SipRsrcPriorityHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipRsrcPriorityHeader *to;
	SipRsrcPriorityHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipRsrcPriorityHeader");
	/* clean up of to */

	if(to->pNamespace != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pNamespace)), err) == SipFail)
			return SipFail;
	}

    if(to->pPriority != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pPriority)), err) == SipFail)
			return SipFail;
	}

	/* clean up over */

	if(from->pNamespace== SIP_NULL)
	{
		to->pNamespace=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pNamespace );

		to->pNamespace=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pNamespace == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pNamespace, from->pNamespace );
	}

	if(from->pPriority== SIP_NULL)
	{
		to->pPriority=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pPriority );

		to->pPriority=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pPriority == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pPriority, from->pPriority );
	}


	SIPDEBUGFN( "Exiting __sip_cloneSipRsrcPriorityHeader");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************
** FUNCTION:__sip_cloneSipAcceptRsrcPriorityHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the AcceptRsrcPriorityHeader structures "source" to "dest".
**
*******************************************************/

SipBool __sip_cloneSipAcceptRsrcPriorityHeader
# ifdef ANSI_PROTO
	(SipAcceptRsrcPriorityHeader *to,
	SipAcceptRsrcPriorityHeader *from,
	SipError *err)
# else
	( to,from, err )
	SipAcceptRsrcPriorityHeader *to;
	SipAcceptRsrcPriorityHeader *from;
	SipError *err;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneSipAcceptRsrcPriorityHeader");
    	/* clean up of to */

	if(to->pNamespace != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pNamespace)), err) == SipFail)
			return SipFail;
	}

    if(to->pPriority != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pPriority)), err) == SipFail)
			return SipFail;
	}

	/* clean up over */

	if(from->pNamespace== SIP_NULL)
	{
		to->pNamespace=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pNamespace );

		to->pNamespace=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pNamespace == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pNamespace, from->pNamespace );
	}

	if(from->pPriority== SIP_NULL)
	{
		to->pPriority=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pPriority );

		to->pPriority=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( to->pPriority == SIP_NULL )
		{
			*err = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pPriority, from->pPriority );
	}
 
    SIPDEBUGFN( "Exiting __sip_cloneSipAcceptRsrcPriorityHeader");
	
	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:__sip_cloneSipRsrcPriorityHeaderList
**
** DESCRIPTION:  This function makes a deep copy of the whole list of 
**	the RsrcPriorityHeader structures "source" to "dest".
**
**********************************************************************/

SipBool __sip_cloneSipRsrcPriorityHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipRsrcPriorityHeader *temp_RsrcPriority, *clone_RsrcPriority;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipRsrcPriorityHeaderList");
	/* copying siplist of SipRsrcPriorityHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_RsrcPriority), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_RsrcPriority == SIP_NULL )
			clone_RsrcPriority = SIP_NULL;
		else
		{
			if(sip_initSipRsrcPriorityHeader(&clone_RsrcPriority, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_RsrcPriority == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipRsrcPriorityHeader(  clone_RsrcPriority, temp_RsrcPriority, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_RsrcPriority), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_RsrcPriority, pErr) == SipFail )
		{
			if ( clone_RsrcPriority != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_RsrcPriority), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipRsrcPriorityHeaderList");
	return SipSuccess;
}

/********************************************************************
** FUNCTION:__sip_cloneSipAcceptRsrcPriorityHeaderList
**
** DESCRIPTION:  This function makes a deep copy of the whole list of 
**	the AcceptRsrcPriorityHeader structures "source" to "dest".
**
**********************************************************************/

SipBool __sip_cloneSipAcceptRsrcPriorityHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipAcceptRsrcPriorityHeader *temp_AcceptRsrcPriority, *clone_AcceptRsrcPriority;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipAcceptRsrcPriorityHeaderList");
	/* copying siplist of SipAcceptRsrcPriorityHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&temp_AcceptRsrcPriority), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( temp_AcceptRsrcPriority == SIP_NULL )
			clone_AcceptRsrcPriority = SIP_NULL;
		else
		{
			
            if(sip_initSipAcceptRsrcPriorityHeader(&clone_AcceptRsrcPriority, pErr)==SipFail)
			{
				return SipFail;
			}

			if ( clone_AcceptRsrcPriority == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipAcceptRsrcPriorityHeader(  clone_AcceptRsrcPriority, temp_AcceptRsrcPriority, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(clone_AcceptRsrcPriority), SIP_NULL);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, clone_AcceptRsrcPriority, pErr) == SipFail )
		{
			if ( clone_AcceptRsrcPriority != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(clone_AcceptRsrcPriority), SIP_NULL);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipAcceptRsrcPriorityHeaderList");
	return SipSuccess;
}

#endif

#ifdef SIP_3GPP
/******************************************************************
**
** FUNCTION:  __sip_cloneSipServiceRouteHeader
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the ServiceRouteHeader structures "source" to "dest".
**
******************************************************************/
#ifdef SIP_3GPP
SipBool __sip_cloneSipServiceRouteHeader
#ifdef ANSI_PROTO
	(SipServiceRouteHeader *pDest, SipServiceRouteHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipServiceRouteHeader *pDest;
	SipServiceRouteHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp=SIP_NULL;
	SIPDEBUGFN("Entering function __sip_cloneSipServiceRouteHeader");

	/* cleaning up dest */

	sip_freeString(pDest->pDispName);
	pDest->pDispName = SIP_NULL;

	sip_freeSipAddrSpec(pDest->pAddrSpec);
	pDest->pAddrSpec = SIP_NULL;

	if(sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;
	/* cleaned up */
	/* clone pDispName */
	if (pSource->pDispName == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pDispName);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
	    SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeader");
			return SipFail;
		}
	}
	pDest->pDispName = pTemp;

	/* clone pAddrSpec */
	if (validateSipAddrSpecType ((pSource->pAddrSpec)->dType, pErr) == SipFail)
  {
	  SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeader");
		return SipFail;
  }
	if (sip_initSipAddrSpec(&(pDest->pAddrSpec), (pSource->pAddrSpec)->dType,\
		 pErr) == SipFail)
  { 
	  SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeader");
		return SipFail;
  }
	if (__sip_cloneSipAddrSpec(pDest->pAddrSpec, pSource->pAddrSpec, pErr)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpec);
	  SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeader");
		return SipFail;
	}

	if(__sip_cloneSipParamList(&(pDest->slParams),&(pSource->slParams),pErr)==SipFail)
  {
	  SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeader");
		return SipFail;
  }

	*pErr = E_NO_ERROR ;
	SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeader");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_cloneSipServiceRouteHeaderList
**
** DESCRIPTION:  This function makes a copy of path headers from
**	the source ServiceRouteHeader List to to "dest".
**
******************************************************************/
SipBool __sip_cloneSipServiceRouteHeaderList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipServiceRouteHeader *pTempService=SIP_NULL, *pCloneService=SIP_NULL;
	SIP_U32bit dCount, dIndex=0;

	SIPDEBUGFN("Entering function __sip_cloneSipServiceRouteHeaderList");
	/* copying siplist of SipServiceRouteHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
  {
	  SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
		return SipFail;
  }
	if ( sip_listSizeOf(pSource, &dCount, pErr) == SipFail )
	{
	  SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
		return SipFail;
	}
	for ( dIndex = 0; dIndex < dCount; dIndex++)
	{
		if( sip_listGetAt(pSource,dIndex, (SIP_Pvoid * ) (&pTempService), pErr)\
			== SipFail )
		{
	    SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
			return SipFail;
		}
		if ( pTempService == SIP_NULL )
			pCloneService = SIP_NULL;
		else
		{
			if(sip_initSipServiceRouteHeader(&pCloneService, pErr)==SipFail)
			{
	      SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
				return SipFail;
			}

			if ( pCloneService == SIP_NULL )
			{
	      SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
				return SipFail;
      }

			if ( __sip_cloneSipServiceRouteHeader(  pCloneService, pTempService,pErr)\
				== SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCloneService), \
					SIP_NULL);
	      SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneService, pErr) == SipFail )
		{
			if ( pCloneService != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pCloneService), SIP_NULL);
	    SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipServiceRouteHeaderList");
	return SipSuccess;
}
#endif /*3GPP */

#endif

