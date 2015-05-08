/******************************************************************************
** FUNCTION:
** 	This pHeader file contains the prototypes of all Generic SIP Header
**      Manipulation APIs.
**
*******************************************************************************
**
** FILENAME:
** 	generic1csb.c
**
** DESCRIPTION:
**  	
**
** DATE      NAME           REFERENCE      REASON
** ----      ----           ---------      ------
** 20Dec99   S.Luthra	    				Initial Creation
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/


#include "portlayer.h"
#include "sipstruct.h"
#include "sipcommon.h"
#include "sipfree.h"
#include "sipinit.h"
#include "siplist.h"
#include "sipinternal.h"
#include "header.h"
#include "sipvalidate.h"
#include "string.h"
#include "sipformmessage.h"
#include "sipstring.h"
#include "sipclone.h"
#include "sipdecodeintrnl.h"

/****************************************************************************************
**
** FUNCTION:  sip_getHeaderCount
**
** DESCRIPTION:  This function returns the number of headers of en_HeaderType "dType" 
** in the SipMessage "msg" in the variable "count". In case of headers in which "Any" 
** dType is possible , the dType passed in en_HeaderType must be "Any" 
** - otherwise E_INV_TYPE is returned.
**
****************************************************************************************/
SipBool sip_getHeaderCount
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SIP_U32bit *count, SipError *err)
#else
	(msg, dType, count, err)
	SipMessage *msg;
	en_HeaderType dType;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getHeaderCount");
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;

	if ( (msg == SIP_NULL)||(count == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	/* validating the dType of the pHeader passed */
	if (validateSipHeaderType (dType, err) == SipFail)
		return SipFail;

	switch (msg->dType)
	{
		case	SipMessageRequest:
					 if (isSipGeneralHeader(dType) == SipTrue)
					 {
						if (msg->pGeneralHdr == SIP_NULL)
						{
							*count = 0;
							return SipSuccess;
						}	
						else
						{
							if (getGeneralHdrCount(msg->pGeneralHdr, dType, count, err) == SipFail)
								return SipFail;
						}
					 }
					 else if (isSipReqHeader(dType) == SipTrue)
					 {
						if ((msg->u).pRequest == SIP_NULL)
						 {
							*count = 0;
							return SipSuccess;
						 }
						if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
						{
							*count = 0;
							return SipSuccess;
						}	
						else
						{
							if (getRequestHdrCount(((msg->u).pRequest)->pRequestHdr, dType, count, err) == SipFail)
								return SipFail;
						}
					 }
					 else
				  	 {
						*err = E_INV_TYPE;
						return SipFail;
					 }
					 break;
		case	SipMessageResponse:
					 if (isSipGeneralHeader(dType) == SipTrue)
					 {
						if (msg->pGeneralHdr == SIP_NULL)
						{
							*count = 0;
							return SipSuccess;
						}
						else
						{
							if (getGeneralHdrCount(msg->pGeneralHdr,  \
								dType, count, err) == SipFail)
								return SipFail;
						}
					 }
					 else if (isSipRespHeader(dType) == SipTrue)
					 {
						if ((msg->u).pResponse == SIP_NULL)
						 {
							*count = 0;
							return SipSuccess;
						 }
						if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
						{
							*count = 0;
							return SipSuccess;
						}
						else
						{
							if (getResponseHdrCount(((msg->u).pResponse)->pResponseHdr,  \
								dType, count, err) == SipFail)
								return SipFail;
						}
					 }
					 else
					  {
						*err = E_INV_TYPE;
						return SipFail;
					 }
					 break;

		default:
			*err = E_INV_TYPE;
			 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getHeaderCount");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getHeaderAtIndex
**
** DESCRIPTION: This function gets the pHeader at the index "index" 
** among the headers of en_HeaderType "dType" in the SipMessage "msg". 
** The pHeader is returned in the structure "hdr". If no headers are 
** present in "msg " of the specified dType SipFail is returned 
** with the error pValue set to E_NO_EXIST. In case of headers in 
** which "Any" dType is possible, the dType passed must be "Any" 
** - otherwise E_INV_TYPE is returned.
**
*********************************************************************/
SipBool sip_getHeaderAtIndex
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipMessage *msg, en_HeaderType dType,  SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(SipMessage *msg, en_HeaderType dType,  SipHeader *hdr, SIP_U32bit index, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(msg, dType, hdr, index, err)
	SipMessage *msg;
	en_HeaderType dType;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#else
	(msg, dType, hdr, index, err)
	SipMessage *msg;
	en_HeaderType dType;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
#endif
{
	SIPDEBUGFN("Entering function sip_getHeaderAt");	
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;

	if ( (msg == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	
#ifndef SIP_BY_REFERENCE
	if (hdr->dType != dType)
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
#endif
	
#ifdef SIP_BY_REFERENCE
	(hdr)->dType = dType;
#endif	
	
	switch (msg->dType)
	{
		case 	SipMessageRequest:
				if ((msg->u).pRequest == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
			 	}
			 	if (isSipGeneralHeader(dType) == SipTrue)
				{
					if (msg->pGeneralHdr == SIP_NULL)
					{
						*err = E_NO_EXIST;
						return SipFail;
					}
					if (getGeneralHeaderAtIndex( msg->pGeneralHdr,  \
						dType, hdr, index, err ) == SipFail)
						return SipFail;
						
				 }
				 else if (isSipReqHeader(dType) == SipTrue)
				 {
					if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
					{
						*err = E_NO_EXIST;
						return SipFail;
					}
					if (getRequestHeaderAtIndex( ((msg->u).pRequest)->pRequestHdr,  \
						dType, hdr, index, err) == SipFail)
						return SipFail;
				 }
			 	else
				{
					*err = E_INV_TYPE;
					return SipFail;
				 }
				 break;

		case	SipMessageResponse:
				if ((msg->u).pResponse == SIP_NULL)
				 {
					*err = E_NO_EXIST;
					return SipFail;
				 }
				 if (isSipGeneralHeader(dType) == SipTrue)
				 {
					if (msg->pGeneralHdr == SIP_NULL)
					{
						*err = E_NO_EXIST;
						return SipFail;
					}
					if (getGeneralHeaderAtIndex( msg->pGeneralHdr, dType,  \
						hdr, index, err ) == SipFail)
						return SipFail;
				 }
				 else if (isSipRespHeader(dType) == SipTrue)
				 {
					if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
					{
						*err = E_NO_EXIST;
						return SipFail;
					}
					if (getResponseHeaderAtIndex( ((msg->u).pResponse)->pResponseHdr,  \
						dType, hdr, index, err) == SipFail)
						return SipFail;
				 }
				 else
				 {
					*err = E_INV_TYPE;
					return SipFail;
				 }
				 break;

		default	:
				*err = E_INV_PARAM;
				 return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	sip_equateTypeInSipHeader(hdr);
#else
	sip_equateTypeInSipHeader(hdr);
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getHeaderAt");
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_insertHeaderAtIndex
**
** DESCRIPTION: This function inserts the pHeader "hdr" at the 
** index "index" among the headers of en_HeaderType "dType" in the 
** SipMessage "msg". If the pHeader dType of "hdr" is "Any" 
** E_INV_TYPE is returned. If no such pHeader dType exists in the "msg" 
** , a new line is inserted in the messages in the end; otherwise it is 
** appended at the end of an existing pHeader line of the given dType.
**
*********************************************************************/
SipBool sip_insertHeaderAtIndex
#ifdef ANSI_PROTO
	(SipMessage *msg, SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(msg, hdr, index, err)
	SipMessage *msg;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	en_SipBoolean x, y;
	SIP_U32bit line_number, temp_index;
	SipHeaderOrderInfo	*header_info_row;
	en_HeaderType	dType;
	
	SIPDEBUGFN("Entering function sip_insertHeaderAt");

	x = y = SipFalse;
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;
	if ((msg == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	
	switch (msg->dType)
	{
		case	SipMessageRequest:
				 if (isSipGeneralHeader(hdr->dType) == SipTrue)
				 {
					if (msg->pGeneralHdr == SIP_NULL)
					{
						if (sip_initSipGeneralHeader (&(msg->pGeneralHdr), err) == SipFail)
						{
							return SipFail;
						}
						y = SipTrue; 
					}
					if (insertGeneralHeaderAtIndex(msg->pGeneralHdr, hdr, index, err) == SipFail)
					{
						if (y == SipTrue)
							sip_freeSipGeneralHeader(msg->pGeneralHdr);
						return SipFail;
					}
				 }
				 else if (isSipReqHeader(hdr->dType) == SipTrue)
				 {
					if ((msg->u).pRequest == SIP_NULL)
					 {
						if (sip_initSipReqMessage (&((msg->u).pRequest), err) == SipFail)
							return SipFail;
						x = SipTrue;
					 }	

					if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
					{
						if (sip_initSipReqHeader (&(((msg->u).pRequest)->pRequestHdr), err) == SipFail)
						{
							if (x == SipTrue)
								sip_freeSipReqMessage((msg->u).pRequest);
							return SipFail;
						}
						y = SipTrue; 
					}
					if (insertRequestHeaderAtIndex(((msg->u).pRequest)->pRequestHdr, hdr,index, err) == SipFail)
					{
						if (x == SipTrue)
							sip_freeSipReqMessage((msg->u).pRequest);
						else if (y == SipTrue)
							sip_freeSipReqHeader(((msg->u).pRequest)->pRequestHdr);
						return SipFail;
					}
				 }
				 else
				 {
					*err = E_INV_TYPE;
					return SipFail;
				 }
				 break;

		case	SipMessageResponse:
				 if (isSipGeneralHeader(hdr->dType) == SipTrue)
				 {
					if (msg->pGeneralHdr == SIP_NULL)
					{
						if (sip_initSipGeneralHeader (&(msg->pGeneralHdr), err) == SipFail)
						{
							return SipFail;
						}
						y = SipTrue; 
					}
					if (insertGeneralHeaderAtIndex(msg->pGeneralHdr, hdr, index, err) == SipFail)
					{
						if (y == SipTrue)
							sip_freeSipGeneralHeader(msg->pGeneralHdr);
						return SipFail;
					}
				 }
				 else if (isSipRespHeader(hdr->dType) == SipTrue)
				 {
					if ((msg->u).pResponse == SIP_NULL)
				 	{
						if (sip_initSipRespMessage (&((msg->u).pResponse), err) == SipFail)
							return SipFail;
						x = SipTrue;
				 	}
					if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
					{
						if (sip_initSipRespHeader (&(((msg->u).pResponse)->pResponseHdr), err) == SipFail)
						{
							if (x == SipTrue)
								sip_freeSipRespMessage((msg->u).pResponse);
							return SipFail;
						}
						y = SipTrue; 
					}
					if (insertResponseHeaderAtIndex(((msg->u).pResponse)->pResponseHdr, hdr, index, err) == SipFail)
					{
						if (x == SipTrue)
							sip_freeSipRespMessage((msg->u).pResponse);
						else if (y == SipTrue)
							sip_freeSipRespHeader(((msg->u).pResponse)->pResponseHdr);
						return SipFail;
					}
				 }
				 else
				 {
					*err = E_INV_TYPE;
					return SipFail;
				 }
				 break;
		default	:
				*err = E_INV_PARAM;
				 return SipFail;
	} /* of switch */
	

 /* This is to check if second time insertion has happened
	* in single instance header then no need to update
	* order info */
  if ( *err == E_SECND_INSERTION_SINGLE_INST_HDR )
	{
			*err = E_NO_ERROR ;
			return SipSuccess ;
	}
	/* updating header_info_table */
	if ( index == 0 )
	 	temp_index = 0; 
	 else
	 	temp_index = index - 1;

	 dType = hdr->dType;
	 if(sip_changeTypeAny(&dType, err) == SipFail)
	 	return SipFail;

	if ( sip_getHeaderPositionFromIndex(msg, dType, temp_index,  \
		&line_number, SIP_NULL, err) == SipFail)
	{
		if( (index == 0) && ( *err == E_NO_EXIST ) ) /* means that pHeader does not exist in the message */
		{
			if ( sip_listSizeOf(&(msg->slOrderInfo), &line_number, err) == SipFail)   
				/* if that pHeader does not exist append at the end of message */
				return SipFail;

			if( sip_initSipHeaderOrderInfo( &header_info_row, err ) == SipFail)
				return SipFail;
		
			header_info_row->dType = dType;
			header_info_row->dTextType = SipFormFull;
			header_info_row->dNum = 1;

			if ( sip_listInsertAt( &(msg->slOrderInfo), line_number, \
					(SIP_Pvoid)(header_info_row), err ) == SipFail)
			{
				__sip_freeSipHeaderOrderInfo((SIP_Pvoid)header_info_row); 
				return SipFail;
			}
		}
		else
		{
			return SipFail;  /* error already set */
		}
	}
	else
	{
		if ( sip_listGetAt( &(msg->slOrderInfo), line_number,  \
			(SIP_Pvoid *)(&header_info_row), err) == SipFail )
			return SipFail;

		header_info_row->dNum++;

	}

	*err = E_NO_ERROR;	
	SIPDEBUGFN("Exiting function sip_insertHeaderAt"); 
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_setHeaderAtIndex
**
** DESCRIPTION:  This function sets the pHeader "hdr" at the 
** index "index" among the headers of en_HeaderType "dType" in the 
** SipMessage "msg". If the pHeader dType of "hdr" is "Any" E_INV_TYPE 
** is returned. If no such pHeader dType exists in the "msg" at the 
** given index ,E_INV_INDEX is returned. Note that this function returns 
** E_INV_INDEX for headers which are SipList if there are no headers of 
** en_HeaderType "dType" in the message already existing but for non 
** SipList Headers it successfully sets the pHeader at index 0 
** ( the only valid index for such headers ). 
**
*********************************************************************/
SipBool sip_setHeaderAtIndex
#ifdef ANSI_PROTO
	(SipMessage *msg, SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(msg, hdr, index, err)
	SipMessage *msg;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	en_SipBoolean x, y;
	SIP_U32bit	line_number;
	SipHeaderOrderInfo	*header_info_row;
	en_HeaderType dType;

	SIPDEBUGFN("Entering sip_setHeaderAtIndex Function");

	x = y = SipFalse;
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;
	if ((msg == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif

	switch (msg->dType)
	{
		case	SipMessageRequest:
				 if (isSipGeneralHeader(hdr->dType) == SipTrue)
				 {
					if (msg->pGeneralHdr == SIP_NULL)
					{
						if (sip_initSipGeneralHeader (&(msg->pGeneralHdr), err) == SipFail)
						{
							return SipFail;
						}
						y = SipTrue; 
					}
					if (setGeneralHeaderAtIndex(msg->pGeneralHdr, hdr, index, err) == SipFail)
					{
						if (y == SipTrue)
							sip_freeSipGeneralHeader(msg->pGeneralHdr);
						return SipFail;
					}
				 }
				 else if (isSipReqHeader(hdr->dType) == SipTrue)
				 {
					if ((msg->u).pRequest == SIP_NULL)
				 	{
						if (sip_initSipReqMessage (&((msg->u).pRequest), err) == SipFail)
							return SipFail;
						x = SipTrue;
				 	}
					if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
					{
						if (sip_initSipReqHeader (&(((msg->u).pRequest)->pRequestHdr), err) == SipFail)
						{
							if (x == SipTrue)
								sip_freeSipReqMessage((msg->u).pRequest);
							return SipFail;
						}
						y = SipTrue; 
					}
					if (setRequestHeaderAtIndex(((msg->u).pRequest)->pRequestHdr, hdr,index, err) == SipFail)
					{
						if (x == SipTrue)
							sip_freeSipReqMessage((msg->u).pRequest);
						else if (y == SipTrue)
							sip_freeSipReqHeader(((msg->u).pRequest)->pRequestHdr);
						return SipFail;
					}
				 }
				 else
				 {
					*err = E_INV_TYPE;
					return SipFail;
				 }
				 break;

		case	SipMessageResponse:
				 if (isSipGeneralHeader(hdr->dType) == SipTrue)
				 {
					if (msg->pGeneralHdr == SIP_NULL)
					{
						if (sip_initSipGeneralHeader (&(msg->pGeneralHdr), err) == SipFail)
						{
							return SipFail;
						}
						y = SipTrue; 
					}
					if (setGeneralHeaderAtIndex(msg->pGeneralHdr, hdr, index, err) == SipFail)
					{
						if (y == SipTrue)
							sip_freeSipGeneralHeader(msg->pGeneralHdr);
								return SipFail;
					}
				}
				else if (isSipRespHeader(hdr->dType) == SipTrue)
				{
					
					if ((msg->u).pResponse == SIP_NULL)
				 	{
						if (sip_initSipRespMessage (&((msg->u).pResponse), err) == SipFail)
							return SipFail;
						x = SipTrue;
				 	}
					if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
					{
						if (sip_initSipRespHeader (&(((msg->u).pResponse)->pResponseHdr), err) == SipFail)
						{
							if (x == SipTrue)
								sip_freeSipRespMessage((msg->u).pResponse);
							return SipFail;
						}
						y = SipTrue; 
					}
					if (setResponseHeaderAtIndex(((msg->u).pResponse)->pResponseHdr, hdr, index, err) == SipFail)
					{
						if (x == SipTrue)
							sip_freeSipRespMessage((msg->u).pResponse);
						else if (y == SipTrue)
							sip_freeSipRespHeader(((msg->u).pResponse)->pResponseHdr);
						return SipFail;
					}
				 }
				 else
				 {
					*err = E_INV_TYPE;
					return SipFail;
				 }
				 break;

		default:
				*err = E_INV_PARAM;
				 return SipFail;
	} /* of switch */

	/* updating header_info_table */

	dType = hdr->dType;
	if(sip_changeTypeAny(&dType, err) == SipFail)
		return SipFail;
	if ( sip_getHeaderPositionFromIndex(msg, dType, index, &line_number, SIP_NULL, err) == SipFail)
	{
		if( *err == E_NO_EXIST ) /* means that pHeader does not exist in the message but is newly inserted */
		{
			if ( sip_listSizeOf(&(msg->slOrderInfo), &line_number, err) == SipFail)   
			/* if that pHeader does not exist append at the end of message */
				return SipFail;

			if( sip_initSipHeaderOrderInfo( &header_info_row, err ) == SipFail)
				return SipFail;
	
			header_info_row->dType = dType;
			header_info_row->dTextType = SipFormFull;
			header_info_row->dNum = 1;

			if ( sip_listInsertAt( &(msg->slOrderInfo), line_number, (SIP_Pvoid)(header_info_row), err ) == SipFail)
				return SipFail;

		}
		else /* Error if something other than E_NO_EXIST */
			return SipFail;

	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setHeaderAt");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_deleteHeaderAtIndex
**
** DESCRIPTION:  This function  deletes the pHeader at 
** index "index" among the headers of en_HeaderType "dType" in 
** the SipMessage "msg". In case of headers in which "Any" dType is 
** possible, the dType passed must be "Any" - otherwise E_INV_TYPE 
** is returned.
**
*********************************************************************/
SipBool sip_deleteHeaderAtIndex
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SIP_U32bit index, SipError *err)
#else
	(msg, dType, index, err)
	SipMessage *msg;
	en_HeaderType dType;
	SIP_U32bit index;
	SipError *err;
#endif
{

	SIP_U32bit line_number;
	SipHeaderOrderInfo *header_info_row;
	SipMessage *s;

s = msg;

	SIPDEBUGFN("Entering function sip_deleteHeaderAt");
#ifndef SIP_NO_CHECK

	if (err == SIP_NULL)
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	if (validateSipHeaderType (dType, err) == SipFail)
		return SipFail;

	switch (msg->dType)
	{
		case	SipMessageRequest:
				 if (isSipGeneralHeader(dType) == SipTrue)
				 {
					if (msg->pGeneralHdr == SIP_NULL)
					{
						*err = E_NO_EXIST;
						return SipFail;
					}	
					else
					{
						if (deleteGeneralHdrAtIndex(msg->pGeneralHdr, dType, index, err) == SipFail)
							return SipFail;
					}
				 }
				 else if (isSipReqHeader(dType) == SipTrue)
				 {
					if ((msg->u).pRequest == SIP_NULL)
				 	{
						*err = E_NO_EXIST;
						return SipFail;
					 }
					if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
					{
						*err = E_NO_EXIST;
						return SipFail;
					}	
					else
					{
						if (deleteRequestHdrAtIndex(((msg->u).pRequest)->pRequestHdr, dType, index, err) == SipFail)
							return SipFail;
					}
				 }
				 else
			  	 {
					*err = E_INV_TYPE;
					return SipFail;
				 }
				 break;

		case	SipMessageResponse:
						 if (isSipGeneralHeader(dType) == SipTrue)
						 {
							if (msg->pGeneralHdr == SIP_NULL)
							{
								*err = E_NO_EXIST;
								return SipFail;
							}
							else
							{
								if (deleteGeneralHdrAtIndex(msg->pGeneralHdr,  \
									dType, index, err) == SipFail)
									return SipFail;
							}
						 }
						 else if (isSipRespHeader(dType) == SipTrue)
						 {
							if ((msg->u).pResponse == SIP_NULL)
							{
								*err = E_NO_EXIST;
								return SipFail;
						 	}
							if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
							{
								*err = E_NO_EXIST;
								return SipFail;
							}
							else
							{
								if (deleteResponseHdrAtIndex(((msg->u).pResponse)->pResponseHdr,  \
									dType, index, err) == SipFail)
									return SipFail;
							}
						 }
						 else
					  	 {
							*err = E_INV_TYPE;
							return SipFail;
						 }
						 break;
		default: 
				*err = E_INV_TYPE;
					 return SipFail;

	} /* of switch */
		

	/* updating header_info_table */
	if ( sip_getHeaderPositionFromIndex(msg, dType, index, &line_number, SIP_NULL, err) == SipFail)
		return SipFail;
	else
	{
		if ( sip_listGetAt( &(msg->slOrderInfo), line_number, (SIP_Pvoid *)(&header_info_row), err) == SipFail )
			return SipFail;

		(header_info_row->dNum)--;
	
		if ( header_info_row->dNum == 0 )  /* all headers in that row is deleted */
			if ( sip_listDeleteAt( &(msg->slOrderInfo), line_number, err) == SipFail)
				return SipFail;


	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteHeaderAt");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_deleteAllHeaderType
**
** DESCRIPTION: This function deletes all headers of en_HeaderType 
** "dType" from a SipMessage "msg".
**
*********************************************************************/
SipBool sip_deleteAllHeaderType
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SipError *err)
#else
	(msg,dType,err)
	SipMessage *msg;
	en_HeaderType dType;
	SipError *err;
#endif
{
	SIP_U32bit rep, count;
	SipHeaderOrderInfo	*temp_order;

	SIPDEBUGFN("Entering function sip_deleteAllHeaderType");

#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	if (validateSipHeaderType (dType, err) == SipFail)
		return SipFail;

	switch (msg->dType)
	{
		case	SipMessageRequest	:
						 if (isSipGeneralHeader(dType) == SipTrue)
						 {
							if (msg->pGeneralHdr == SIP_NULL)
							{
								*err = E_NO_EXIST;
								return SipFail;
							}	
							else
							{
								if (deleteAllGeneralHdr(msg->pGeneralHdr, dType, err) == SipFail)
									return SipFail;
							}
						 }
						 else if (isSipReqHeader(dType) == SipTrue)
						 {
							if ((msg->u).pRequest == SIP_NULL)
						 	{
								*err = E_NO_EXIST;
								return SipFail;
							 }
							if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
							{
								*err = E_NO_EXIST;
								return SipFail;
							}	
							else
							{
								if (deleteAllRequestHdr(((msg->u).pRequest)->pRequestHdr, dType, err) == SipFail)
									return SipFail;
							}
						 }
						 else
					  	 {
							*err = E_INV_TYPE;
							return SipFail;
						 }
						 break;

		case	SipMessageResponse	:
						 if (isSipGeneralHeader(dType) == SipTrue)
						 {
							if (msg->pGeneralHdr == SIP_NULL)
							{
								*err = E_NO_EXIST;
								return SipFail;
							}
							else
							{
								if (deleteAllGeneralHdr(msg->pGeneralHdr, dType, err) == SipFail)
									return SipFail;
							}
						 }
						 else if (isSipRespHeader(dType) == SipTrue)
						 {
							if ((msg->u).pResponse == SIP_NULL)
						 	{
								*err = E_NO_EXIST;
								return SipFail;
						 	}
							if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
							{
								*err = E_NO_EXIST;
								return SipFail;
							}
							else
							{
								if (deleteAllResponseHdr(((msg->u).pResponse)->pResponseHdr, dType, err) == SipFail)
									return SipFail;
							}
						 }
						 else
					  	 {
							*err = E_INV_TYPE;
							return SipFail;
						 }
						 break;

		default				: *err = E_INV_TYPE;
						 return SipFail;


	}

	/* updating orde table */
	if ( sip_getHeaderLineCount( msg, &count, err) == SipFail)
		return SipFail;

	for (rep = 0; rep < count; rep++)
	{
		if ( sip_listGetAt( &(msg->slOrderInfo), rep, (SIP_Pvoid*)(&temp_order), err) == SipFail)
			return SipFail;

		if ( temp_order->dType == dType)
		{
			if( sip_listDeleteAt(&(msg->slOrderInfo), rep,err) == SipFail)
				return SipFail;

			rep--;
			count--;
		}
	}


	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteAllHeaderType");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getMessageType
**
** DESCRIPTION: This function returns the dType ( request/respose ) of SipMessage "msg" n the variable ""dType"
**
*********************************************************************/
SipBool sip_getMessageType
#ifdef ANSI_PROTO
	(SipMessage *msg, en_SipMessageType *dType, SipError *err)
#else
	(msg, dType, err)
	SipMessage *msg;
	en_SipMessageType *dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getMessageType");
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;

	if ( (dType == SIP_NULL)||(msg == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	switch (msg->dType)
	{
		case	SipMessageRequest	:
		case	SipMessageResponse	: *dType = msg->dType;
						  break;
		default				: *err = E_INV_TYPE;
						  return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getMessageType");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getStatusLineFromSipRespMsg
**
** DESCRIPTION: This function returns the status line from a SipMessage of dType SipMessageResponse.
**
*********************************************************************/
SipBool sip_getStatusLineFromSipRespMsg
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipMessage *msg, SipStatusLine **line, SipError *err)
#else
	(SipMessage *msg, SipStatusLine *line, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(msg, line, err)
	SipMessage *msg;
	SipStatusLine **line;
	SipError *err;
#else
	(msg, line, err)
	SipMessage *msg;
	SipStatusLine *line;
	SipError *err;
#endif
#endif
{
	SipRespMessage *response_msg;
	SipStatusLine *temp_line;
	SIPDEBUGFN("Entering function sip_getstatusLineFromSipRespMsg");
#ifndef SIP_NO_CHECK

	if (err == SIP_NULL)
		return SipFail;

	if ( (msg == SIP_NULL)||(line == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (msg->dType != SipMessageResponse)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif

	response_msg = (msg->u).pResponse;
	if ((response_msg == SIP_NULL))
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
	temp_line = response_msg->pStatusLine;
	if (temp_line == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(temp_line->dRefCount);
	*line = temp_line;
#else
	if (__sip_cloneSipStatusLine(line, temp_line, err) == SipFail)
		return SipFail;
#endif
		
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getstatusLineFromSipRespMsg");
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_setStatusLineInSipRespMsg
**
** DESCRIPTION: This fucntions sets the status line in a SipMessage of dType SipMessageResponse.
**
*********************************************************************/
SipBool sip_setStatusLineInSipRespMsg
#ifdef ANSI_PROTO
	(SipMessage *msg, SipStatusLine *line, SipError *err)
#else
	(msg, line, err)
	SipMessage *msg;
	SipStatusLine *line;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipStatusLine *temp_status_line;
#endif
	
	SIPDEBUGFN("Entering function sip_setStatusLineInSipRespMsg");

#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;
	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (msg->dType != SipMessageResponse)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ((msg->u).pResponse == SIP_NULL)
	{
		if (sip_initSipRespMessage(&((msg->u).pResponse), err) == SipFail)
			return SipFail;
	}
	if (line == SIP_NULL)
	{
		if (((msg->u).pResponse)->pStatusLine != SIP_NULL)
			sip_freeSipStatusLine(((msg->u).pResponse)->pStatusLine);
		((msg->u).pResponse)->pStatusLine = SIP_NULL;	
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipStatusLine(&temp_status_line, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipStatusLine(temp_status_line, line, err) == SipFail)
		{
			sip_freeSipStatusLine(temp_status_line);
			return SipFail;
		}
		sip_freeSipStatusLine(((msg->u).pResponse)->pStatusLine);
		((msg->u).pResponse)->pStatusLine = temp_status_line;
#else
		if (((msg->u).pResponse)->pStatusLine != SIP_NULL)
			sip_freeSipStatusLine(((msg->u).pResponse)->pStatusLine);
		HSS_LOCKEDINCREF(line->dRefCount);
		((msg->u).pResponse)->pStatusLine = line;
#endif		
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setStatusLineInSipRespMsg");
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_getReqLineFromSipReqMsg
**
** DESCRIPTION: This function retrieves the request line from a SipMessage of dType SipMessagerequest.
**
*********************************************************************/
SipBool sip_getReqLineFromSipReqMsg
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipMessage *msg, SipReqLine **line, SipError *err)
#else
	(SipMessage *msg, SipReqLine *line, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(msg, line, err)
	SipMessage *msg;
	SipReqLine **line;
	SipError *err;
#else
	(msg, line, err)
	SipMessage *msg;
	SipReqLine *line;
	SipError *err;
#endif
#endif
{
	SipReqMessage *request_msg;
	SipReqLine *temp_line;
	SIPDEBUGFN("Entering function sip_getReqLineFromSipReqMsg");
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;

	if ( (msg == SIP_NULL)||(line == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (msg->dType != SipMessageRequest)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif

	request_msg = (msg->u).pRequest;
	if ((request_msg == SIP_NULL))
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
	temp_line = request_msg->pRequestLine;
	if (temp_line == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(temp_line->dRefCount);
	*line = temp_line;
#else
	if (__sip_cloneSipReqLine (line, temp_line, err) == SipFail)
		return SipFail;
#endif
		
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getReqLineFromSipReqMsg");
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_setReqLineInSipReqMsg
**
** DESCRIPTION: This function sets the Request Line in a SipMessage of dType SipMessageRequest.
**
*********************************************************************/
SipBool sip_setReqLineInSipReqMsg
#ifdef ANSI_PROTO
	(SipMessage *msg, SipReqLine *line, SipError *err)
#else
	(msg, line, err)
	SipMessage *msg;
	SipReqLine *line;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipReqLine *temp_req_line;
#endif

	SIPDEBUGFN("Entering function sip_setReqLineInSipReqMsg");

#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;
	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (msg->dType != SipMessageRequest)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	if (msg->u.pRequest == SIP_NULL)
	{
		if (sip_initSipReqMessage(&((msg->u).pRequest), err) == SipFail)
				return SipFail;
	}
	
	if (line == SIP_NULL)
	{
		if (((msg->u).pRequest)->pRequestLine != SIP_NULL)
			sip_freeSipReqLine(((msg->u).pRequest)->pRequestLine);
		((msg->u).pRequest)->pRequestLine = SIP_NULL;	
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSipReqLine(&temp_req_line, err ) == SipFail)
			return SipFail;
		if (__sip_cloneSipReqLine(temp_req_line, line, err) == SipFail)
		{
			sip_freeSipReqLine(temp_req_line);
			return SipFail;
		}
		sip_freeSipReqLine(((msg->u).pRequest)->pRequestLine);
		((msg->u).pRequest)->pRequestLine = temp_req_line;
#else
		if (((msg->u).pRequest)->pRequestLine != SIP_NULL)
			sip_freeSipReqLine(((msg->u).pRequest)->pRequestLine);
		HSS_LOCKEDINCREF(line->dRefCount);
		((msg->u).pRequest)->pRequestLine = line;
#endif
	}
			
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setReqLineInSipReqMsg");
	return SipSuccess;
}


/* General utilty Functions for this file */


/********************************************************************
**
** FUNCTION:  sip_getHeader
**
** DESCRIPTION:  This is a wrapper function for sip_getHeaderAtIndex at the index 0.
**
*********************************************************************/
SipBool sip_getHeader
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipMessage *msg, en_HeaderType dType,  SipHeader *hdr, SipError *err)
#else
	(SipMessage *msg, en_HeaderType dType,  SipHeader *hdr, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(msg, dType, hdr, err)
	SipMessage *msg;
	en_HeaderType dType;
	SipHeader *hdr;
	SipError *err;
#else
	(msg, dType, hdr, err)
	SipMessage *msg;
	en_HeaderType dType;
	SipHeader *hdr;
	SipError *err;
#endif
#endif
{
	if ( sip_getHeaderAtIndex(msg, dType, hdr,0, err) == SipFail)
		return SipFail;
	
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_setHeader
**
** DESCRIPTION:  This is a wrapper function for sip_setHeaderAtIndex at index 0.
**
*********************************************************************/
SipBool sip_setHeader
#ifdef ANSI_PROTO
	(SipMessage *msg, SipHeader *hdr, SipError *err)
#else
	(msg, hdr, err)
	SipMessage *msg;
	SipHeader *hdr;
	SipError *err;
#endif
{
	if ( sip_setHeaderAtIndex(msg, hdr, 0, err) == SipFail)
		return SipFail;

	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_setHeaderFromStringAtIndex
**
** DESCRIPTION: This function sets a Sipheader of en_HeaderType "dType" in exactly the same way as sip_setHeaderAtIndex; the input pHeader here is in the form of a string "hdr".
**
*********************************************************************/
SipBool sip_setHeaderFromStringAtIndex
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SIP_S8bit *hdr, SIP_U32bit index, SipError *err)
#else
	(msg, dType, hdr, index, err)
	SipMessage *msg;
	en_HeaderType dType;
	SIP_S8bit *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipHeaderParserParam	dParserParam;
	SipList					GCList;
	SIP_U32bit				count;
	SipError				tempError;
	SIP_S8bit				*temp;

#ifndef SIP_NO_CHECK
	if(err == SIP_NULL)
		return SipFail;

	if(msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	sip_changeTypeAny(&dType, err);

	if(sip_validateHeaderString(hdr, dType, err)!=SipSuccess)
		return SipFail;


	/* Check if header type is unknown. If so dont use the parser */
	if(dType == SipHdrTypeUnknown)
	{
#ifdef SIP_BY_REFERENCE
		SipHeader header;
#else
		SipHeader *pHeader;
#endif
		SipUnknownHeader *pUnknownHeader;

		if(sip_initSipUnknownHeader(&pUnknownHeader,err)==SipFail)
			return SipFail;
		if(sip_makeUnknownHeader(hdr, pUnknownHeader, err)==SipFail)
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
#ifdef SIP_BY_REFERENCE
		header.dType = SipHdrTypeUnknown;
		header.pHeader = pUnknownHeader;
		if(sip_setHeaderAtIndex(msg, &header, index, err)==SipFail)
#else
		if(sip_initSipHeader(&pHeader, SipHdrTypeAny, err)==SipFail)
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
		pHeader->dType = SipHdrTypeUnknown;
		pHeader->pHeader = pUnknownHeader;
		if(sip_setHeaderAtIndex(msg, pHeader, index, err)==SipFail)
#endif
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
#ifdef SIP_BY_REFERENCE
		sip_freeSipHeader(&header);
#else
		sip_freeSipHeader(pHeader);
#endif
		*err = E_NO_ERROR;
		return SipSuccess;
	}

	/* Initialize a parser strcture for passing to the bison parser */
	/* Get the dType of the header and initialize a message structure for it */
	if((isSipGeneralHeader(dType)==SipTrue)||(isSipReqHeader(dType)==SipTrue))
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageRequest,\
			err)==SipFail)
			return SipFail;
	}
	else
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageResponse,\
			err)==SipFail)
			return SipFail;
	}
	if(sip_listInit(&GCList, __sip_freeString, err)==SipFail)
		return SipFail;
	dParserParam.pGCList = &GCList;
	dParserParam.pError = err;
	*err = E_NO_ERROR;

	temp = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, strlen(hdr)+2, err);
	if(temp == SIP_NULL)
		return SipFail;
	strcpy(temp, hdr);
	temp[strlen(temp)+1]='\0';
	/* Invoke scan_buffer to set up lexical scanner */
	if(glbSipParserScanBuffer[dType](temp, strlen(hdr)+2)!=0)
	{
		*(dParserParam.pError) = E_NO_MEM;
		fast_memfree(DECODE_MEM_ID, temp, SIP_NULL);
		return SipFail;
	}
	/*  Call a reset if available */
	if (glbSipParserReset[dType] != SIP_NULL)
		glbSipParserReset[dType]();		
	/* Invoke the parser */
	glbSipParserParser[dType]((void *)&dParserParam);
	sip_listDeleteAll((dParserParam.pGCList), &tempError);
	/* Release lexer buffer */
	glbSipParserReleaseBuffer[dType]();
	
	fast_memfree(ACCESSOR_MEM_ID, temp, SIP_NULL);
	sip_listDeleteAll(&GCList,&tempError);
	/* Error during parsing */
	if(*err!=E_NO_ERROR)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(sip_getHeaderCount(dParserParam.pSipMessage, dType, &count,\
		err)!=SipSuccess)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	
	if(count==0)
	{	
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	
	{
#ifdef SIP_BY_REFERENCE
		SipHeader pParsedHeader;
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, dType,&pParsedHeader,\
			0, err)!=SipSuccess)
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_setHeaderAtIndex(msg, &pParsedHeader, index, err)\
			!=SipSuccess)
		{
			sip_freeSipHeader(&pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		sip_freeSipHeader(&pParsedHeader);
#else
		SipHeader *pParsedHeader;
		if(sip_initSipHeader(&pParsedHeader, dType, err)==SipFail)
			return SipFail;
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, dType,pParsedHeader,\
			0, err)!=SipSuccess)
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_setHeaderAtIndex(msg, pParsedHeader, index, err)\
			!=SipSuccess)
		{
			sip_freeSipHeader(pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		sip_freeSipHeader(pParsedHeader);
#endif
	}
	sip_freeSipMessage(dParserParam.pSipMessage);
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_insertHeaderFromStringAtIndex
**
** DESCRIPTION:  This function inserts a SipHeader of en_HeaderType "dType" in exactly the same way as sip_insertHeaderAtIndex; the input pHeader here is in the form of a string "hdr".
**
*********************************************************************/
SipBool sip_insertHeaderFromStringAtIndex
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SIP_S8bit *hdr, SIP_U32bit index, SipError *err)
#else
	(msg, dType, hdr, index, err)
	SipMessage *msg;
	en_HeaderType dType;
	SIP_S8bit *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipHeaderParserParam	dParserParam;
	SipList					GCList;
	SIP_U32bit				count;
	SipError				tempError;
	SIP_S8bit				*temp;

#ifndef SIP_NO_CHECK
	if(err == SIP_NULL)
		return SipFail;

	if(msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	sip_changeTypeAny(&dType, err);

	if(sip_validateHeaderString(hdr, dType, err)!=SipSuccess)
		return SipFail;
	

	/* Check if header type is unknown. If so dont use the parser */
	if(dType == SipHdrTypeUnknown)
	{
#ifdef SIP_BY_REFERENCE
		SipHeader header;
#else
		SipHeader *pHeader;
#endif
		SipUnknownHeader *pUnknownHeader;

		if(sip_initSipUnknownHeader(&pUnknownHeader,err)==SipFail)
			return SipFail;
		if(sip_makeUnknownHeader(hdr, pUnknownHeader, err)==SipFail)
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
#ifdef SIP_BY_REFERENCE
		header.dType = SipHdrTypeUnknown;
		header.pHeader = pUnknownHeader;
		if(sip_insertHeaderAtIndex(msg, &header, index, err)==SipFail)
#else
		if(sip_initSipHeader(&pHeader, SipHdrTypeAny, err)==SipFail)
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
		pHeader->dType = SipHdrTypeUnknown;
		pHeader->pHeader = pUnknownHeader;
		if(sip_insertHeaderAtIndex(msg, pHeader, index, err)==SipFail)
#endif
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
#ifdef SIP_BY_REFERENCE
		sip_freeSipHeader(&header);
#else
		sip_freeSipHeader(pHeader);
#endif
		*err = E_NO_ERROR;
		return SipSuccess;
	}

	/* Initialize a parser strcture for passing to the bison parser */
	/* Get the dType of the header and initialize a message structure for it */
	if((isSipGeneralHeader(dType)==SipTrue)||(isSipReqHeader(dType)==SipTrue))
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageRequest,\
			err)==SipFail)
			return SipFail;
	}
	else
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageResponse,\
			err)==SipFail)
			return SipFail;
	}
	if(sip_listInit(&GCList, __sip_freeString, err)==SipFail)
		return SipFail;
	dParserParam.pGCList = &GCList;
	dParserParam.pError = err;
	*err = E_NO_ERROR;

	temp = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, strlen(hdr)+2, err);
	if(temp == SIP_NULL)
		return SipFail;
	strcpy(temp, hdr);
	temp[strlen(temp)+1]='\0';
	/* Invoke scan_buffer to set up lexical scanner */
	if(glbSipParserScanBuffer[dType](temp, strlen(hdr)+2)!=0)
	{
		*(dParserParam.pError) = E_NO_MEM;
		fast_memfree(DECODE_MEM_ID, temp, SIP_NULL);
		return SipFail;
	}
	/*  Call a reset if available */
	if (glbSipParserReset[dType] != SIP_NULL)
		glbSipParserReset[dType]();		
	/* Invoke the parser */
	glbSipParserParser[dType]((void *)&dParserParam);
	sip_listDeleteAll((dParserParam.pGCList), &tempError);
	/* Release lexer buffer */
	glbSipParserReleaseBuffer[dType]();
	
	fast_memfree(ACCESSOR_MEM_ID, temp, SIP_NULL);
	sip_listDeleteAll(&GCList,&tempError);
	/* Error during parsing */
	if(*err!=E_NO_ERROR)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(sip_getHeaderCount(dParserParam.pSipMessage, dType, &count,\
		err)!=SipSuccess)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(count==0)
		return SipFail;
	{
#ifdef SIP_BY_REFERENCE
		SipHeader pParsedHeader;
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, dType,&pParsedHeader,\
			0, err)!=SipSuccess)
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_insertHeaderAtIndex(msg, &pParsedHeader, index, err)\
			!=SipSuccess)
		{
			sip_freeSipHeader(&pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		sip_freeSipHeader(&pParsedHeader);
#else
		SipHeader *pParsedHeader;
		if(sip_initSipHeader(&pParsedHeader, dType, err)==SipFail)
			return SipFail;
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, dType,pParsedHeader,\
			0, err)!=SipSuccess)
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_insertHeaderAtIndex(msg, pParsedHeader, index, err)\
			!=SipSuccess)
		{
			sip_freeSipHeader(pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		sip_freeSipHeader(pParsedHeader);
#endif
	}
	sip_freeSipMessage(dParserParam.pSipMessage);
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getHeaderAsStringAtIndex
**
** DESCRIPTION:  This function retrieves a SipHeader of en_HeaderType 
** "dType" in exactly the same way as sip_getHeaderAtIndex; 
** the output pHeader here is in the form of a string "hdr".

**
*********************************************************************/
SipBool sip_getHeaderAsStringAtIndex
#ifdef ANSI_PROTO
(SipMessage *s, en_HeaderType dType,  SIP_S8bit **hdr, SIP_U32bit index, SipError *err)
#else
	( s,dType, hdr,index, err)
	SipMessage *s;
	en_HeaderType dType;
	SIP_S8bit **hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SipError pTempErr;
	SIP_S8bit* pTemp;
	if(validateSipHeaderType(dType,err)==SipFail)
	{
		return SipFail;
	}
	*hdr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,SIP_MAX_HDR_SIZE,err);
	/* Maintain the reference to the allocated memory */
	pTemp = *hdr;
	if ( *hdr == SIP_NULL )
	{
		return SipFail;
	}
	if(sip_formSingleHeader((*hdr+SIP_MAX_HDR_SIZE),dType,index, SipModeNew, s, hdr, err)==SipFail)
	{
		fast_memfree(ACCESSOR_MEM_ID, *hdr, &pTempErr);
		*hdr=SIP_NULL;
		return SipFail;
	}
	/* Restore the pointer now, this was moved in the form API */
	pTemp[strlen(pTemp)-2] = '\0';
	*hdr = pTemp;
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sip_getHeaderLineCount
**
** DESCRIPTION: This function returns the number of distinct pHeader lines in the SipMessage "msg".
**
*********************************************************************/
SipBool sip_getHeaderLineCount 
#ifdef ANSI_PROTO
	(SipMessage *msg, SIP_U32bit *count, SipError *err)
#else
	(msg, count, err)
	SipMessage *msg;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering sip_getHeaderLineCount");
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;
	if ((msg == SIP_NULL)||(count==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	if (sip_listSizeOf (&(msg->slOrderInfo), count, err) == SipFail)
		return SipFail;
	
	*err = E_INV_PARAM;
	SIPDEBUGFN("Exiting sip_getHeaderLineCount");
	return SipSuccess;
}
	
/********************************************************************
**
** FUNCTION:  sip_getHeaderPositionFromIndex
**
** DESCRIPTION: This function retrieves the absolute line and the position in that line of an en_HeaderType "dType" given the linear index among all the headers of that dType.
**
*********************************************************************/
SipBool sip_getHeaderPositionFromIndex 
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SIP_U32bit list_index, SIP_U32bit *abs_line, SIP_U32bit *position, SipError *err)
#else
	(msg, dType, list_index, abs_line, position, err)
	SipMessage *msg;
	en_HeaderType dType;
	SIP_U32bit list_index;
	SIP_U32bit *abs_line;
	SIP_U32bit *position;
	SipError *err;
#endif
{
	SIP_Pvoid header_line;
	SipHeaderOrderInfo *hdr_info;
	SIP_U32bit temp_count, count, prev_count, iter;
	en_SipBoolean result = SipFalse;;
	SIPDEBUGFN("Entering sip_getHeaderPositionFromIndex");
	count=prev_count=0;

#ifndef SIP_NO_CHECK
	/* checking for the validity of input parameters. */
	if (err == SIP_NULL)
		return SipFail;
	if ( (msg==SIP_NULL)||((abs_line==SIP_NULL)&&(position==SIP_NULL)) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	if ( sip_verifyTypeAny(dType, err) == SipFail)
			return SipFail;
	
	if (sip_listSizeOf(&(msg->slOrderInfo), &temp_count, err) == SipFail)
		return SipFail;
	for (iter=0; iter<temp_count; iter++)
	{
		if (sip_listGetAt(&(msg->slOrderInfo), iter, &header_line, err) == SipFail)
			return SipFail;
		hdr_info = (SipHeaderOrderInfo *)header_line;
		if (hdr_info->dType == dType)
			count+=hdr_info->dNum;
		if (count>list_index)
		{
			if (abs_line != SIP_NULL)
				*abs_line = iter;
			if (position != SIP_NULL)
				*position = list_index-prev_count;
			result = SipTrue;
			break;
		}
		prev_count=count;
	}
	if (result == SipFalse)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_getHeaderPositionFromIndex");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getHeaderIndexFromPosition
**
** DESCRIPTION:  This function calculates the linear index of a pHeader of en_HeaderType "dType" given its absolute line and position within that line
**
*********************************************************************/
SipBool sip_getHeaderIndexFromPosition 
#ifdef ANSI_PROTO
	(SipMessage *msg, SIP_U32bit abs_line, SIP_U32bit position, SIP_U32bit *index, SipError *err)
#else
	(msg, abs_line, position, index, err)
	SipMessage *msg;
	SIP_U32bit abs_line;
	SIP_U32bit position;
	SIP_U32bit *index;
	SipError *err;
#endif
{
	SIP_Pvoid header_line;
	SipHeaderOrderInfo *hdr_info, *hdr_info1;
	en_HeaderType dType;
	SIP_U32bit count, iter;
	count=0;
	SIPDEBUGFN("Entering sip_getHeaderIndexFromPosition");
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;
	if ((msg==SIP_NULL)||(index==SIP_NULL))
	{
		*err=E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listGetAt(&(msg->slOrderInfo), abs_line, &header_line, err) == SipFail)
		return SipFail;
	hdr_info = (SipHeaderOrderInfo *)header_line;
	if (((hdr_info->dNum)-1) < position)
	{
		*err=E_INV_PARAM;
		return SipFail;
	}
	dType = hdr_info->dType;
	for (iter=0; iter<abs_line; iter++)
	{
		if (sip_listGetAt (&(msg->slOrderInfo), iter, &header_line, err) == SipFail)
			return SipFail;
		hdr_info1 = (SipHeaderOrderInfo *)header_line;
		if (hdr_info1->dType == dType)
			count += hdr_info1->dNum;
	}
	*index = count+position;
		
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_getHeaderIndexFromPosition");
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_getHeaderTypeAtHeaderLine
**
** DESCRIPTION: This function retrieves the dType of the headers in the line "line" from a SipMessage "msg"
**
*********************************************************************/
SipBool sip_getHeaderTypeAtHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage		*msg,
	  SIP_U32bit		line,
	  en_HeaderType		*dType,
	  SipError              *err )
#else
        ( msg, line, dType, err )
	  SipMessage            *msg;
	  SIP_U32bit            line;
	  en_HeaderType         *dType;
	  SipError              *err;
#endif
{
	SIP_Pvoid	temp;

	SIPDEBUGFN("Entering function sip_getHeaderTypeAtHeaderLine");

#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;

	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
		return SipFail;

	*dType = ((SipHeaderOrderInfo *)temp)->dType;
	*err = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getHeaderTypeAtHeaderLine");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getHeaderCountFromHeaderLine
**
** DESCRIPTION: This function retrieves the number of headers present at the "line" line in a SipMessage "msg".
**
*********************************************************************/
SipBool sip_getHeaderCountFromHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage            *msg,
          SIP_U32bit            line,
          SIP_U32bit		*count,
	  SipError              *err )
#else
        ( msg, line, count, err )
          SipMessage            *msg;
          SIP_U32bit            line;
          SIP_U32bit            *count;
          SipError              *err;
#endif
{
	SIP_Pvoid       temp;   

        SIPDEBUGFN("Entering function sip_getHeaderCountFromHeaderLine");

#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
                return SipFail;

        if (msg == SIP_NULL)
        {
                *err = E_INV_PARAM;
                return SipFail;
        }
#endif

	if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
                return SipFail;

	*count = ((SipHeaderOrderInfo *)temp)->dNum;
        *err = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getHeaderCountFromHeaderLine");
        return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getHeaderFormFromHeaderLine
**
** DESCRIPTION:  This function retrieves the form ( SHORT / FULL ) of the headers in the line "line" in a SipMessage "msg". 
**
*********************************************************************/
SipBool sip_getHeaderFormFromHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage            *msg,
          SIP_U32bit            line,
          en_HeaderForm		*dTextType,
          SipError              *err )
#else
        ( msg, line, dTextType, err )
          SipMessage            *msg;
          SIP_U32bit            line;
          en_HeaderForm		*dTextType;
          SipError              *err;
#endif
{
        SIP_Pvoid       temp;   

        SIPDEBUGFN("Entering function sip_getHeaderFormFromHeaderLine");

#ifndef SIP_NO_CHECK
        if (err == SIP_NULL)
                return SipFail;

        if (msg == SIP_NULL)
        {
                *err = E_INV_PARAM;
                return SipFail;
        }
#endif

        if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
                return SipFail;
	
	*dTextType = ((SipHeaderOrderInfo *)temp)->dTextType;
        *err = E_NO_ERROR;
        SIPDEBUGFN("Exiting function sip_getHeaderFormFromHeaderLine");
        return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_setHeaderFormAtHeaderLine
**
** DESCRIPTION:  This function sets the form ( SHORT / FULL ) of the headers in the line "line" in a SipMessage "msg". 
**
*********************************************************************/
SipBool sip_setHeaderFormAtHeaderLine
#ifdef ANSI_PROTO       
        ( SipMessage            *msg,   
          SIP_U32bit            line,   
          en_HeaderForm         dTextType,
          SipError              *err )  
#else   
        ( msg, line, dTextType, err )
          SipMessage            *msg;   
          SIP_U32bit            line;   
          en_HeaderForm         dTextType;
          SipError              *err;   
#endif  
{       
        SIP_Pvoid       temp;

        SIPDEBUGFN("Entering function sip_setHeaderFormAtHeaderLine");

#ifndef SIP_NO_CHECK
        if (err == SIP_NULL)
                return SipFail;

        if (msg == SIP_NULL) 
        {
                *err = E_INV_PARAM;
                return SipFail;
        }
#endif

	if (dTextType == SipFormFull || dTextType == SipFormShort)
	{
		if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
               		 return SipFail;

		((SipHeaderOrderInfo *)temp)->dTextType = dTextType;
       		*err = E_NO_ERROR;
       		SIPDEBUGFN("Exiting function sip_setHeaderFormAtHeaderLine");
       		return SipSuccess;
	}
	else
	{
                *err = E_INV_PARAM;
                return SipFail;
	}
}


/** Wrappers to be built around existing APIs/New functions TBM **/
/********************************************************************
**
** FUNCTION:  sip_insertHeaderAtPosition
**
** DESCRIPTION: This function is same as sip_insertHeaderAtIndex except that the the pHeader to be deleted is identified by the line in the message in which it is present and the position in that line.
**
*********************************************************************/
SipBool sip_insertHeaderAtPosition
#ifdef ANSI_PROTO
	(SipMessage *msg, SipHeader *hdr,  SIP_U32bit abs_line, SIP_U32bit position, en_AdditionMode mode, SipError *err)
#else
	(msg, hdr, abs_line, position, mode, err)
	SipMessage *msg;
	SipHeader *hdr;
	SIP_U32bit abs_line;
	SIP_U32bit position;
	en_AdditionMode mode;
	SipError *err;
#endif
{
	en_SipBoolean x, y;
	SipError temp_err;
	SIP_U32bit index, temp_count, iter=0;
	SipHeaderOrderInfo	*hdr_info;
	SIP_U8bit int_flag =0;
	SIP_Pvoid header_line;
	en_HeaderType	temp_type;

	SIPDEBUGFN("Entering function sip_insertHeaderAtPosition");

	x = y = SipFalse;
#ifndef SIP_NO_CHECK
	if (err == SIP_NULL)
		return SipFail;
	if ((msg == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (hdr->pHeader == SIP_NULL)
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* Generation of index */

	temp_type = hdr->dType;
	if ( sip_changeTypeAny( &temp_type, err ) == SipFail)
		return SipFail;

	if (sip_listSizeOf (&(msg->slOrderInfo), &temp_count, err) == SipFail)
		return SipFail;
	if ( abs_line > temp_count)
	{
		*err = E_INV_INDEX;
		return SipFail;
	}
	else if ( (abs_line-temp_count) == 0) /* new line at the end of message */
	{
		if (sip_getHeaderCount(msg, hdr->dType, &temp_count, err) == SipFail)
			return SipFail;
		index = temp_count;
		int_flag=1;
	}
	else /* in beteeen message */
	{
		if (sip_listGetAt (&(msg->slOrderInfo), abs_line, &header_line, err) == SipFail)
			return SipFail;
		hdr_info = (SipHeaderOrderInfo *)header_line;
		if (temp_type == hdr_info->dType) 
		{
			if ((position > hdr_info->dNum)) /* invalid position */
			{
				*err = E_INV_PARAM;
				return SipFail;
			}
			else if ((position-hdr_info->dNum) == 0) /* new pHeader to be appended */				       {
				if (sip_getHeaderIndexFromPosition(msg, abs_line, position-1, &temp_count, err) == SipFail)
					return SipFail;
				temp_count++;
				int_flag = 2;
			}
			else /* pHeader to be inserted in between line */
			{
				if (sip_getHeaderIndexFromPosition(msg, abs_line, position, &temp_count, err) == SipFail)
					return SipFail;
				int_flag = 3;
			}
			index = temp_count;
		}
		else /* not the same line */
		{
			/* a new line with only that pHeader is to be created */	
			if ( position != 0 )
			{
				*err = E_INV_INDEX; 
				return SipFail;
			}
	
			temp_count =0;
			for (iter=0; iter<abs_line; iter++)
			{
				if (sip_listGetAt(&(msg->slOrderInfo), iter, &header_line, err) == SipFail)
					return SipFail;
				hdr_info = (SipHeaderOrderInfo *)header_line;
				if (hdr_info->dType == temp_type)
					temp_count += hdr_info->dNum;
			}
			int_flag = 4;
			index = temp_count;
		}
	}
	/* end of index generation */
	switch (msg->dType)
	{
		case	SipMessageRequest	:
						 if (isSipGeneralHeader(hdr->dType) == SipTrue)
						 {
							if (msg->pGeneralHdr == SIP_NULL)
							{
								if (sip_initSipGeneralHeader (&(msg->pGeneralHdr), err) == SipFail)
								{
									return SipFail;
								}
								y = SipTrue; 
							}
							if (insertGeneralHeaderAtIndex(msg->pGeneralHdr, hdr, index, err) == SipFail)
							{
								if (y == SipTrue)
									sip_freeSipGeneralHeader(msg->pGeneralHdr);
								return SipFail;
							}
						 }
						 else if (isSipReqHeader(hdr->dType) == SipTrue)
						 {
							if ((msg->u).pRequest == SIP_NULL)
						 	{
								if (sip_initSipReqMessage (&((msg->u).pRequest), err) == SipFail)
									return SipFail;
								x = SipTrue;
							 }
							if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
							{
								if (sip_initSipReqHeader (&(((msg->u).pRequest)->pRequestHdr), err) == SipFail)
								{
									if (x == SipTrue)
										sip_freeSipReqMessage((msg->u).pRequest);
									return SipFail;
								}
								y = SipTrue; 
							}
							if (insertRequestHeaderAtIndex(((msg->u).pRequest)->pRequestHdr, hdr,index, err) == SipFail)
							{
								if (x == SipTrue)
									sip_freeSipReqMessage((msg->u).pRequest);
								else if (y == SipTrue)
									sip_freeSipReqHeader(((msg->u).pRequest)->pRequestHdr);
								return SipFail;
							}
						 }
						 else
						 {
							*err = E_INV_TYPE;
							return SipFail;
						 }
						 break;

		case	SipMessageResponse	:
						 if (isSipGeneralHeader(hdr->dType) == SipTrue)
						 {
							if (msg->pGeneralHdr == SIP_NULL)
							{
								if (sip_initSipGeneralHeader (&(msg->pGeneralHdr), err) == SipFail)
								{
									return SipFail;
								}
								y = SipTrue; 
							}
							if (insertGeneralHeaderAtIndex(msg->pGeneralHdr, hdr, index, err) == SipFail)
							{
								if (y == SipTrue)
									sip_freeSipGeneralHeader(msg->pGeneralHdr);
								return SipFail;
							}
						 }
						 else if (isSipRespHeader(hdr->dType) == SipTrue)
						 {
							if ((msg->u).pResponse == SIP_NULL)
						 	{
								if (sip_initSipRespMessage (&((msg->u).pResponse), err) == SipFail)
									return SipFail;
								x = SipTrue;
						 	}
							if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
							{
								if (sip_initSipRespHeader (&(((msg->u).pResponse)->pResponseHdr), err) == SipFail)
								{
									if (x == SipTrue)
										sip_freeSipRespMessage((msg->u).pResponse);
									return SipFail;
								}
								y = SipTrue; 
							}
							if (insertResponseHeaderAtIndex(((msg->u).pResponse)->pResponseHdr, hdr, index, err) == SipFail)
							{
								if (x == SipTrue)
									sip_freeSipRespMessage((msg->u).pResponse);
								else if (y == SipTrue)
									sip_freeSipRespHeader(((msg->u).pResponse)->pResponseHdr);
								return SipFail;
							}
						 }
						 else
						 {
							*err = E_INV_TYPE;
							return SipFail;
						 }
						 break;
		default				:*err = E_INV_PARAM;
						 return SipFail;
	}
	
	/* updating header_info_table */
	switch (int_flag)
	{

		case 	1:if (sip_initSipHeaderOrderInfo (&hdr_info, err) == SipFail)
			  {
				if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
					return SipFail;
				return SipFail;
			  }
			  hdr_info->dType = temp_type;
			  hdr_info->dNum = 1;
			  hdr_info->dTextType = SipFormFull;
			  if (sip_listInsertAt(&(msg->slOrderInfo), abs_line, (SIP_Pvoid)hdr_info, err) == SipFail)
			  {
				if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
					return SipFail;
				return SipFail;
			  }
			  break;

		case	2:if (sip_listGetAt (&(msg->slOrderInfo), abs_line, &header_line, err) == SipFail)
			  {
				if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
					return SipFail;
				return SipFail;
			  }
			  hdr_info = (SipHeaderOrderInfo *)header_line;
			  (hdr_info->dNum)++;
			  break;	
			   
		case	3:if ( (position==0)&&(mode==SipModeNew) )
			  {
				if (sip_initSipHeaderOrderInfo (&hdr_info, err) == SipFail)
			  	{
					if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
						return SipFail;
					return SipFail;
			 	}
			  	hdr_info->dType = temp_type;
			  	hdr_info->dNum = 1;
			  	hdr_info->dTextType = SipFormFull;
			  	if (sip_listInsertAt(&(msg->slOrderInfo), abs_line, (SIP_Pvoid)hdr_info, err) == SipFail)
			  	{
					if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
						return SipFail;
					return SipFail;
			  	}	
			  }
			  else
			  {
				if (sip_listGetAt (&(msg->slOrderInfo), abs_line, &header_line, err) == SipFail)
			  	{
					if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
						return SipFail;
					return SipFail;
			  	}
			  	hdr_info = (SipHeaderOrderInfo *)header_line;
			  	(hdr_info->dNum)++;
			  }
			  break;

		case	4:if (sip_initSipHeaderOrderInfo(&hdr_info, err) == SipFail)
			  {
				if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
					return SipFail;
				return SipFail;
			  }
			  hdr_info->dType = temp_type;
			  hdr_info->dNum = 1;
			  hdr_info->dTextType = SipFormFull;
			  if (sip_listInsertAt(&(msg->slOrderInfo), abs_line, (SIP_Pvoid)hdr_info, err) == SipFail)
			  {
				if (__sip_deleteHeaderAtIndex(msg, hdr->dType, index, &temp_err) == SipFail)
					return SipFail;
				return SipFail;
			  }
			  break;
	}
		
	*err = E_NO_ERROR;	
	SIPDEBUGFN("Exiting function sip_insertHeaderAtPosition"); 
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_insertHeaderFromStringAtPosition
**
** DESCRIPTION: This function is same as sip_insertHeaderFromStringAtIndex
**	except that the the pHeader to be deleted is identified by the line in
**	the message in which it is present and the position in that line.
**
*********************************************************************/
SipBool sip_insertHeaderFromStringAtPosition 
#ifdef ANSI_PROTO
	(SipMessage *msg, SIP_S8bit *hdr, en_HeaderType dType, SIP_U32bit abs_line, SIP_U32bit position, en_AdditionMode mode, SipError *err)
#else
	(msg, hdr, dType, abs_line, position, mode, err)
	SipMessage *msg;
	SIP_S8bit *hdr;
	en_HeaderType	dType;
	SIP_U32bit abs_line;
	SIP_U32bit position;
	en_AdditionMode mode;
	SipError *err;
#endif
{
	SipHeaderParserParam	dParserParam;
	SipList					GCList;
	SIP_U32bit				count;
	SipError				tempError;
	SIP_S8bit				*temp;

#ifndef SIP_NO_CHECK
	if(err == SIP_NULL)
		return SipFail;

	if(msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(hdr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if(sip_validateHeaderString(hdr, dType, err)!=SipSuccess)
		return SipFail;

	sip_changeTypeAny(&dType, err);

	/* Check if header type is unknown. If so dont use the parser */
	if(dType == SipHdrTypeUnknown)
	{
#ifdef SIP_BY_REFERENCE
		SipHeader header;
#else
		SipHeader *pHeader;
#endif
		SipUnknownHeader *pUnknownHeader;

		if(sip_initSipUnknownHeader(&pUnknownHeader,err)==SipFail)
			return SipFail;
		if(sip_makeUnknownHeader(hdr, pUnknownHeader, err)==SipFail)
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
#ifdef SIP_BY_REFERENCE
		header.dType = SipHdrTypeUnknown;
		header.pHeader = pUnknownHeader;
		if(sip_insertHeaderAtPosition(msg, &header, abs_line, position, mode,\
			err)==SipFail)
#else
		if(sip_initSipHeader(&pHeader, SipHdrTypeAny, err)==SipFail)
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
		pHeader->dType = SipHdrTypeUnknown;
		pHeader->pHeader = pUnknownHeader;
		if(sip_insertHeaderAtPosition(msg, pHeader, abs_line, position, mode, err)\
			==SipFail)
#endif
		{
			sip_freeSipUnknownHeader(pUnknownHeader);
			return SipFail;
		}
#ifdef SIP_BY_REFERENCE
		sip_freeSipHeader(&header);
#else
		sip_freeSipHeader(pHeader);
#endif
		*err = E_NO_ERROR;
		return SipSuccess;
	}

	/* Initialize a parser strcture for passing to the bison parser */
	/* Get the dType of the header and initialize a message structure for it */
	if((isSipGeneralHeader(dType)==SipTrue)||(isSipReqHeader(dType)==SipTrue))
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageRequest,\
			err)==SipFail)
			return SipFail;
	}
	else
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageResponse,\
			err)==SipFail)
			return SipFail;
	}
	if(sip_listInit(&GCList, __sip_freeString, err)==SipFail)
		return SipFail;
	dParserParam.pGCList = &GCList;
	dParserParam.pError = err;
	*err = E_NO_ERROR;

	temp = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, strlen(hdr)+2, err);
	if(temp == SIP_NULL)
		return SipFail;
	strcpy(temp, hdr);
	temp[strlen(temp)+1]='\0';
	/* Invoke scan_buffer to set up lexical scanner */
	if(glbSipParserScanBuffer[dType](temp, strlen(hdr)+2)!=0)
	{
		*(dParserParam.pError) = E_NO_MEM;
		fast_memfree(DECODE_MEM_ID, temp, SIP_NULL);
		return SipFail;
	}
	/*  Call a reset if available */
	if (glbSipParserReset[dType] != SIP_NULL)
		glbSipParserReset[dType]();		
	/* Invoke the parser */
	glbSipParserParser[dType]((void *)&dParserParam);
	sip_listDeleteAll((dParserParam.pGCList), &tempError);
	/* Release lexer buffer */
	glbSipParserReleaseBuffer[dType]();
	
	fast_memfree(ACCESSOR_MEM_ID, temp, SIP_NULL);
	sip_listDeleteAll(&GCList,&tempError);
	/* Error during parsing */
	if(*err!=E_NO_ERROR)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(sip_getHeaderCount(dParserParam.pSipMessage, dType, &count,\
		err)!=SipSuccess)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(count==0)
		return SipFail;
	{
#ifdef SIP_BY_REFERENCE
		SipHeader pParsedHeader;
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, dType,&pParsedHeader,\
			0, err)!=SipSuccess)
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_insertHeaderAtPosition(msg, &pParsedHeader, abs_line, position,\
			mode, err)\
			!=SipSuccess)
		{
			sip_freeSipHeader(&pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		sip_freeSipHeader(&pParsedHeader);
#else
		SipHeader *pParsedHeader;
		if(sip_initSipHeader(&pParsedHeader, dType, err)==SipFail)
			return SipFail;
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, dType,pParsedHeader,\
			0, err)!=SipSuccess)
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_insertHeaderAtPosition(msg, pParsedHeader, abs_line, position,\
			mode, err)\
			!=SipSuccess)
		{
			sip_freeSipHeader(pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		sip_freeSipHeader(pParsedHeader);
#endif
	}
	sip_freeSipMessage(dParserParam.pSipMessage);
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_deleteHeaderAtPosition
**
** DESCRIPTION: This function deletes the pHeader at "abs_line" line in the SipMessage "msg" and at "position" position within that line.
**
*********************************************************************/
SipBool sip_deleteHeaderAtPosition 
#ifdef ANSI_PROTO
	(SipMessage *msg, SIP_U32bit abs_line, SIP_U32bit position, SipError *err)
#else
	(msg, abs_line, position, err)
	SipMessage *msg;
	SIP_U32bit abs_line;
	SIP_U32bit position;
	SipError *err;
#endif
{
	SIP_U32bit index;
	SipHeaderOrderInfo	*temp_header_info;

	SIPDEBUGFN("Entering function sip_deleteHeaderAtPosition");

#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;
	
	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if ( sip_getHeaderIndexFromPosition(msg, abs_line, position, &index, err) == SipFail)
		return SipFail;

	if ( sip_listGetAt( &(msg->slOrderInfo), abs_line, (SIP_Pvoid*)(&temp_header_info), err) == SipFail)
		return SipFail;

	/* deleting from siplist in message */
	if ( __sip_deleteHeaderAtIndex(msg, temp_header_info->dType, index, err) == SipFail)
		return SipFail;
	
	temp_header_info->dNum--;

	if ( temp_header_info->dNum == 0 )
		if ( sip_listDeleteAt(&(msg->slOrderInfo), abs_line, err) == SipFail)
			return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_deleteHeaderAtPosition");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_deleteHeaderLine
**
** DESCRIPTION:  This function deletes an entire pHeader line at the absolute position "abs_line" from SipMessage "msg".
**
*********************************************************************/
SipBool sip_deleteHeaderLine 
#ifdef ANSI_PROTO
	(SipMessage *msg, SIP_U32bit abs_line, SipError *err)
#else
	(msg, abs_line, err)
	SipMessage *msg;
	SIP_U32bit abs_line;
	SipError *err;
#endif
{

	SIP_U32bit index, count;
	SipHeaderOrderInfo	*temp_header_info;

	SIPDEBUGFN("Entering function sip_deleteHeaderLine");

#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;
	
	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if ( sip_getHeaderIndexFromPosition(msg, abs_line, 0, &index, err) == SipFail)
		return SipFail;

	if ( sip_listGetAt( &(msg->slOrderInfo), abs_line, (SIP_Pvoid*)(&temp_header_info), err) == SipFail)
		return SipFail;

	/* deleting from siplist in message */
	
	for( count = 0; count < temp_header_info->dNum; count++)
		if ( __sip_deleteHeaderAtIndex(msg, temp_header_info->dType, index, err) == SipFail)
			return SipFail;
	
	if ( sip_listDeleteAt(&(msg->slOrderInfo), abs_line, err) == SipFail)
			return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteHeaderLine");
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_getMsgBodyAtIndex
**
** DESCRIPTION:  This function retrieves a MsgBody "msgbody" at the 
** 	index "index" from a SipMessage "s".
**
*********************************************************************/
SipBool sip_getMsgBodyAtIndex
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
(SipMessage *s, SipMsgBody **msgbody, SIP_U32bit index, SipError *err)
#else
(SipMessage *s, SipMsgBody *msgbody, SIP_U32bit index, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(s, msgbody, index, err)
	SipMessage *s; 
	SipMsgBody **msgbody; 
	SIP_U32bit index; 
	SipError *err;
#else
	(s, msgbody, index, err)
	SipMessage *s; 
	SipMsgBody *msgbody; 
	SIP_U32bit index; 
	SipError *err;
#endif
#endif
{
	SipMsgBody *body_from_list;

#ifndef SIP_NO_CHECK
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(msgbody==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* retrieve message pBody from list */
	if(sip_listGetAt(&(s->slMessageBody), index, (SIP_Pvoid *) &(body_from_list), err) == SipFail)
		return SipFail;
	/* clone message pBody into dest parameter */
#ifdef SIP_BY_REFERENCE
	*msgbody = body_from_list;
	HSS_LOCKEDINCREF((*msgbody)->dRefCount);
#else
	if(__sip_cloneSipMsgBody(msgbody,body_from_list,err)==SipFail)
		return SipFail;
#endif

	*err = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_setMsgBodyAtIndex
**
** DESCRIPTION: This function sets a MsgBody "msgbody" at the index "index" in a SipMessage "s".
**
*********************************************************************/
SipBool sip_setMsgBodyAtIndex
#ifdef ANSI_PROTO
(SipMessage *s, SipMsgBody *msgbody, SIP_U32bit index, SipError *err)
#else
	(s, msgbody, index, err)
	SipMessage *s; 
	SipMsgBody *msgbody; 
	SIP_U32bit index; 
	SipError *err;
#endif
{
	SipMsgBody *body_in_list;

#ifndef SIP_NO_CHECK
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(msgbody==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	
	/* Validate pBody dType */
	switch(msgbody->dType)
	{
		case SipSdpBody:
		case SipUnknownBody:
		case SipMultipartMimeBody:   /* bcpt ext */
		case SipIsupBody:		  	/* bcpt ext */
		case SipQsigBody:		  	/* bcpt ext */
#ifdef SIP_MWI
		case SipMessageSummaryBody:		/* mwi ext */
#endif
		case SipAppSipBody:
			break;
		default:
			*err = E_INV_PARAM;
			return SipFail;
	}
#endif

	/* clone pBody */
#ifdef SIP_BY_REFERENCE
	body_in_list=msgbody;
	HSS_LOCKEDINCREF(body_in_list->dRefCount);
#else
	if(sip_initSipMsgBody(&body_in_list, SipBodyAny, err)==SipFail)
		return SipFail;
	if(__sip_cloneSipMsgBody(body_in_list,msgbody,err)==SipFail)
	{
		sip_freeSipMsgBody(body_in_list);
		return SipFail;
	}
#endif
	/* set message pBody in list */
	if(sip_listSetAt(&(s->slMessageBody), index, (SIP_Pvoid)body_in_list, err) == SipFail)
	{
		sip_freeSipMsgBody(body_in_list);
		return SipFail;
	}

	*err = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_insertMsgBodyAtIndex
**
** DESCRIPTION:  This function inserts a MsgBody "msgbody" at the index "index" in a SipMessage "s".
**
*********************************************************************/
SipBool sip_insertMsgBodyAtIndex
#ifdef ANSI_PROTO
(SipMessage *s, SipMsgBody *msgbody, SIP_U32bit index, SipError *err)
#else
	(s, msgbody, index, err)
	SipMessage *s; 
	SipMsgBody *msgbody; 
	SIP_U32bit index; 
	SipError *err;
#endif
{
	SipMsgBody *body_in_list;

#ifndef SIP_NO_CHECK
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(msgbody==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	
	/* Validate pBody dType */
	switch(msgbody->dType)
	{
		case SipSdpBody:
		case SipUnknownBody:
		case SipIsupBody:                     /* bcpt ext */
		case SipQsigBody:                     /* bcpt ext */
		case SipMultipartMimeBody:			  /* bcpt ext */
#ifdef SIP_MWI
		case SipMessageSummaryBody:
#endif
		case SipAppSipBody:
			break;
		default:
			*err = E_INV_PARAM;
			return SipFail;
	}
#endif
	
	/* clone pBody */
#ifdef SIP_BY_REFERENCE
	body_in_list=msgbody;
	HSS_LOCKEDINCREF(body_in_list->dRefCount);
#else
	if(sip_initSipMsgBody(&body_in_list, SipBodyAny, err)==SipFail)
		return SipFail;
	if(__sip_cloneSipMsgBody(body_in_list,msgbody,err)==SipFail)
	{
		sip_freeSipMsgBody(body_in_list);
		return SipFail;
	}
#endif
	/* insert message pBody in list */
	if(sip_listInsertAt(&(s->slMessageBody), index, (SIP_Pvoid)body_in_list, err) == SipFail)
	{
		sip_freeSipMsgBody(body_in_list);
		return SipFail;
	}

	*err = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_deleteMsgBodyAtIndex
**
** DESCRIPTION:  This function deletes a MsgBody at index "index" in the SipMessage "s".
**
*********************************************************************/
SipBool sip_deleteMsgBodyAtIndex
#ifdef ANSI_PROTO
(SipMessage *s, SIP_U32bit index, SipError *err)
#else
	(s, index, err)
	SipMessage *s; 
	SIP_U32bit index; 
	SipError *err;
#endif
{

#ifndef SIP_NO_CHECK	
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if(s==SIP_NULL) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	/* delete message pBody from list */
	if(sip_listDeleteAt(&(s->slMessageBody), index, err) == SipFail)
	{
		return SipFail;
	}

	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getTypeFromMsgBody
**
** DESCRIPTION:  This function returns the dType of MsgBody 
** 				(Sdp / MultipartMime / Isup /Qsig / Unknown )
**				at the index "index" in a SipMessage "s".
**
*********************************************************************/
SipBool sip_getTypeFromMsgBody
#ifdef ANSI_PROTO
(SipMsgBody *s, en_SipMsgBodyType *dType, SipError *err)
#else
	(s, dType, err)
	SipMsgBody *s; 
	en_SipMsgBodyType *dType; 
	SipError *err;
#endif
{

#ifndef SIP_NO_CHECK	
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(dType==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copy message dType into dest parameter*/
	*dType = s->dType;

	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getSipMessageFromMsgBody
**
** DESCRIPTION:  This function returns the SIP message element from 
**	the SipMsgBody structure. This function should be used only
**	it the type of the message-body structure is SipAppSipBody
**
*********************************************************************/

SipBool sip_getSipMessageFromMsgBody
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SipMsgBody *msg, SipMessage *sipmesg, SipError *err)
#else
	( SipMsgBody *msg, SipMessage **sipmesg, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	( msg, sipmesg, err )
	  SipMsgBody *msg;
	  SipMessage *sipmesg;
	  SipError *err;
#else
	( msg, sipmesg, err )
	  SipMsgBody *msg;
	  SipMessage **sipmesg;
	  SipError *err;	
#endif
#endif
{
	SIPDEBUGFN("Entering getSipMessageFromMsgBody\n"); 
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
	{
		return SipFail;
	}

	if ( (msg == SIP_NULL) || (sipmesg == SIP_NULL ) )
	{
		*err = E_INV_PARAM; 
		return SipFail;
	}
	
	if ( msg->dType != SipAppSipBody)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif 
	if ( msg->u.pAppSipMessage == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if(__sip_cloneSipMessage(sipmesg,msg->u.pAppSipMessage,err) == SipFail)
	{
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF((msg->u.pAppSipMessage)->dRefCount);
	*sipmesg = (msg->u).pAppSipMessage;
#endif
		
	SIPDEBUGFN("Exiting getSipMessageFromMsgBody"); 
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_setSipMessageInMsgBody
**
** DESCRIPTION:  This function sets the SIP message element  in
**	the SipMsgBody structure. 
**
*********************************************************************/

SipBool sip_setSipMessageInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *msg, SipMessage *sipmesg, SipError *err)
#else
	( msg, sipmesg, err )
	  SipMsgBody *msg;
	  SipMessage *sipmesg;
	  SipError *err;
#endif
{
	SipMessage	*temp_sip;
	
	SIPDEBUGFN("Entering sip_setSipMessageInMsgBody\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
	{
		return SipFail;
	}

	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	
	if( msg->dType!=SipAppSipBody )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( msg->u.pAppSipMessage != SIP_NULL)
		sip_freeSipMessage( msg->u.pAppSipMessage);
	
	if (sipmesg==SIP_NULL)
	{
		temp_sip=SIP_NULL;
	}
	else
	{	
#ifndef SIP_BY_REFERENCE
		if ( sip_initSipMessage(&temp_sip, sipmesg->dType, err) == SipFail)
			return SipFail;

		if ( __sip_cloneSipMessage(temp_sip, sipmesg, err) == SipFail)
		{
			sip_freeSipMessage( temp_sip );
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(sipmesg->dRefCount);
		temp_sip = sipmesg;
#endif
	}	

	msg->u.pAppSipMessage = temp_sip;
	
	SIPDEBUGFN("Exiting sip_setSipMessageInMsgBody\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sip_getMsgBodyTypeAtIndex
**
** DESCRIPTION:  This function returns the dType of MsgBody 
**				( Sdp / MultipartMime / Isup /Qsig / Unknown )
**				at the index "index" in a SipMessage "s".
**
*********************************************************************/
SipBool sip_getMsgBodyTypeAtIndex
#ifdef ANSI_PROTO
(SipMessage *s, en_SipMsgBodyType *dType, SIP_U32bit index, SipError *err)
#else
	(s, dType, index, err)
	SipMessage *s; 
	en_SipMsgBodyType *dType; 
	SIP_U32bit index; 
	SipError *err;
#endif
{
	SipMsgBody *body_from_list;

	/* Validate parameters */
#ifndef SIP_NO_CHECK
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(dType==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	/* retrieve message pBody from list */
	if(sip_listGetAt(&(s->slMessageBody), index, (SIP_Pvoid *) \
		&(body_from_list), err) == SipFail)
		return SipFail;
	/* copy message dType into dest parameter*/
	*dType = body_from_list->dType;

	*err = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
**
** FUNCTION:  sip_getMsgBodyCount
**
** DESCRIPTION:  This function retrieves the number of MsgBody in a SipMessage "s". The output isin the variable "count".
**
*********************************************************************/
SipBool sip_getMsgBodyCount
#ifdef ANSI_PROTO
(SipMessage *s, SIP_U32bit *count, SipError *err)
#else
	(s, count, err)
	SipMessage *s; 
	SIP_U32bit *count; 
	SipError *err;
#endif
{
#ifndef SIP_NO_CHECK
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(count==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	/* Get count from pBody list */
	if(sip_listSizeOf(&(s->slMessageBody), count, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getIncorrectHeadersCount
**
** DESCRIPTION: This function retrieves the number of incorrect 
**				headers (accepted by the application at the time of
**				decode in the callback "sip_acceptIncorrectHeader")
**				from the SipMessage "s". The output is in the variable 
**				"count".
**
*********************************************************************/
SipBool sip_getIncorrectHeadersCount
#ifdef ANSI_PROTO
(SipMessage *s, SIP_U32bit *count, SipError *err)
#else
	(s, count, err)
	SipMessage *s; 
	SIP_U32bit *count; 
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getIncorrectHeadersCount");

#ifndef SIP_NO_CHECK
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(count==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	*count = s->dIncorrectHdrsCount;
		
	SIPDEBUGFN("Exiting function sip_getIncorrectHeadersCount");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getEntityErrorCount
**
** DESCRIPTION: This function retrieves the number of incorrect 
**				errors encountered when parsing the entity body
**				of the SIP message. Partially parsed messages are
**				returned by the deocode with the SIP_OPT_BADMESSAGE
**				option.
** PARAMETERS:
**		s(IN): The message from which the count is to be ectracted
**		count(OUT): The number of entity body related errors in the message
**		err(OUT): The error code returned in case of failure.
**
*********************************************************************/
SipBool sip_getEntityErrorCount
#ifdef ANSI_PROTO
(SipMessage *s, SIP_U32bit *count, SipError *err)
#else
	(s, count, err)
	SipMessage *s; 
	SIP_U32bit *count; 
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function sip_getEntityErrorCount");

#ifndef SIP_NO_CHECK
	/* Validate parameters */
	if(err==SIP_NULL) return SipFail;

	if((s==SIP_NULL)||(count==SIP_NULL)) 
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	*count = s->dEntityErrorCount;
		
	SIPDEBUGFN("Exiting function sip_getEntityErrorCount");
	return SipSuccess;
}

const char *glbSipParserHeaderNames[HEADERTYPENUM] =
{	/* This structure should be filled and should be in compliance with en_HeaderType in sipstruct.h */
	"Accept:", 	/* Tokensltoken */
	"Accept-Encoding:",	/* Tokens */
	"Accept-Language:",	/* Tokensltoken */
	"Call-ID:",			/* Contact */
	"CSeq:",				/* Tokens */
	"Date:",				/* Date */
	"Encryption:",		/* Key */
	"Expires:",		/* Date */
	"Expires:",		/* Date */
	"Expires:",		/* Date */
	"From:",				/* Fromto */
	"Record-Route:",		/* Fromto */
	"Timestamp:",		/* Date */
	"To:",				/* Fromto */
	"Via:",				/* Via */
	"Content-Encoding:",	/* Tokens */
	"Content-Length:",	/* Tokens */
	"Content-Type:",		/* Tokensltoken */
	"Authorization:",	/* Pgp */
	"Contact:",	/* Contact */
	"Contact:",	/* Contact */
	"Contact:",		/* Contact */
	"Hide:",				/* Fixed */
	"Max-Forwards:",		/* Tokens */
	"Organization:",		/* Utf8 */
	"Priority:",			/* Fixed */
	"Proxy-Authorization:", /* Pgp */
	"Proxy-Require:",		/* Tokens */
	"Route:",			/* Fromto */
	"Require:",			/* Tokens */
	"Response-Key:",		/* Key */
	"Subject:",			/* Utf8 */
	"User-Agent:",		/* Tokencomment */
	"Allow:",			/* Tokens */
	"Proxy-Authenticate:",/* Pgp */
	"Retry-After:",	/* Date */
	"Retry-After:",	/* Date */
	"Retry-After:",	/* Date */
	"Server:",			/* Tokencomment */
	"Unsupported:",		/* Tokens */
	"Warning:",			/* Tokensltoken */
	"WWW-Authenticate:",	/* Pgp */
	"Authentication-Info",
	"",			/* NULL */
	"MIME-Version:",  	/* Tokensltoken */
#ifdef SIP_CCP	
	"Accept-Contact:", 	/* AcceptContact */
	"Reject-Contact:", 	/* RejectContact */
	"Request-Disposition:", /* Tokens */
#endif	
	"RAck:", 			/* RprTokens */
	"RSeq:",	 			/* RprTokens */
	"Supported:",		/* Tokens */
	"Alert-Info:",		/* Tokensltoken */
	"In-Reply-To:",		/* Tokensltoken */
	"Call-Info:",			/* Tokensltoken */
	"Content-Language:",	/* Key */
	"Error-Info:",		/* Tokensltoken */
	"Content-Disposition:", /* Tokensltoken */
	"Refer-To:",	    	/* Fromto */
	"Also:",				/* Contact */
	"Referred-By:", 		/* Fromto */
	"Replaces:",		/* Contact */
	"Reply-To:",		/* Contact */
#ifdef SIP_IMPP	
	"Event:",			/* Tokensltoken */
	"Allow-Events:",			/* Tokens */
	"Subscription-State:",		/* Date */
#endif	
#ifdef SIP_DCS
	"Remote-Party-ID:",     /* Dcs */
	"RPID-Privacy:",     /* Dcs */
	"DCS-Trace-Party-ID:",          /* Dcs */
	"Anonymity:",           /* Dcs */
	"P-Media-Authorization:",         /* Dcs */
	"DCS-Gate:",    /* Dcs */
	"DCS-Redirect:",        /* Dcs */
	"State:",       /* Dcs */
	"DCS-Laes:",    /* Dcs */
	"Session:",     /* Dcs */
	"DCS-OSPS:",
	"DCS-Billing-ID:",      /* Dcs */
	"DCS-Billing-Info:",
#endif          /* Dcs */
#ifdef SIP_SESSIONTIMER
	"Min-SE:",
	"Session-Expires:",
#endif
#ifdef SIP_PRIVACY
	"P-Asserted-Identity" ,
	"P-Preferred-Identity" ,
	"Privacy:",
#endif
#ifdef SIP_3GPP
	"Path:",
	"P-Access-Network-Info:",
	"P-Charging-Vector:",
#endif
	"Reason:",
#ifdef SIP_CONGEST
	"Resource-Priority:",
	"Accept-Resource-Priority:",
#endif
#ifdef SIP_CONF    
    "Join:",
#endif
#ifdef SIP_3GPP
    "P-Associated-URI:",
    "P-Called-Party-ID:",
    "P-Visited-Network-ID:",
    "P-Charging-Function-Addresses:",
#endif
#ifdef SIP_SECURITY
	"Security-Client:", /* Via */
	"Security-Server:", /* Via */
	"Security-Verify:", /* Via */
#endif
#ifdef SIP_3GPP
		"Service-Route:",
#endif
	""				/* NULL */
};

/********************************************************************
**
** FUNCTION:  sip_parseUnknownHeader
**
** DESCRIPTION: This function parses an unknown header using the grammar
**	of a known header type. If parsing is successful, it returns the 
**	a list of parsed headers.
** 
** PARAMETERS:
**	pHeader(IN) - An unknown header structure retrieved from a SIP message.
**	type(IN) - The type of the known header whose grammar is to be used 
**		for parsing the unknown header.
**	pParsedHeaderList(OUT) - An initialized list in which the parsed headers
**		will be returned. The result is returned in a list since the 
**		unknown header might contain multiple instances of the header
**		separated by commas. The list will contain SipHeader structures.
**		The type of the SipHeader structures will be the type using which
**		the uknown header has been parsed and will support all accessor
**		API for that header.
**	pError(OUT) - Error indication in case of failure.
**
*********************************************************************/
SipBool sip_parseUnknownHeader
#ifdef ANSI_PROTO
(SipHeader *pHeader, en_HeaderType type,\
SipList *pParsedHeaderList, SipError *pError)
#else
(pHeader, type, pParsedHeaderList, pError)
SipHeader *pHeader;
en_HeaderType type;
SipList *pParsedHeaderList;
SipError *pError;
#endif
{
	SipParseEachHeaderParam dParseEachParam;
	SipHeaderParserParam	dParserParam;
	SipHdrTypeList			dTypeList;
	SipList					GCList;
	SIP_U32bit				i,count;
	SipError				tempError;
	SipOptions				dOpt;

	/* Get the name of the desired type and change the name field in the
	   unknown header to that name */
	const SIP_S8bit *pName = glbSipParserHeaderNames[type];
	SIP_S8bit *pTempName;
	/* Initialize a parser strcture for passing to the bison parser */
	/* Get the type of the header and initialize a message structure for it */
	if((isSipGeneralHeader(type)==SipTrue)||(isSipReqHeader(type)==SipTrue))
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageRequest,\
			pError)==SipFail)
			return SipFail;
	}
	else
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageResponse,\
			pError)==SipFail)
			return SipFail;
	}
	if(sip_listInit(&GCList, __sip_freeString, pError)==SipFail)
		return SipFail;
	dParserParam.pGCList = &GCList;
	dParserParam.pError = pError;
	*pError = E_NO_ERROR;
	dParseEachParam.pParserStruct = &dParserParam;
	dParseEachParam.pList = &dTypeList;
	dParseEachParam.ignoreErrors = 0;
	dParseEachParam.noSelectiveCallBacks = SipSuccess;
	dTypeList.enable[type]=SipSuccess;

	pTempName = ((SipUnknownHeader *)pHeader->pHeader)->pName;
	((SipUnknownHeader *)pHeader->pHeader)->pName = (SIP_S8bit*)pName;	
	/* Parse the unknown header's contents */
	dOpt.dOption = 0;
	
	glbSipParserParseEachHeader((SipUnknownHeader *)(pHeader->pHeader),\
		SIP_NULL,0, &dOpt, &dParseEachParam); 

	((SipUnknownHeader *)pHeader->pHeader)->pName =pTempName;
	sip_listDeleteAll(&GCList,&tempError);
	/* Error during parsing */
	if(*pError!=E_NO_ERROR)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(sip_getHeaderCount(dParserParam.pSipMessage, type, &count,\
		pError)!=SipSuccess)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	for(i=0;i<count;i++)
	{
		SipHeader *pParsedHeader;
#ifdef SIP_BY_REFERENCE
		if(sip_initSipHeader(&pParsedHeader, SipHdrTypeAny, pError)!=SipSuccess)
#else
		if(sip_initSipHeader(&pParsedHeader, type, pError)!=SipSuccess)
#endif
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, type, pParsedHeader,\
			i, pError)!=SipSuccess)
		{
			sip_freeSipHeader(pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
		if(sip_listAppend(pParsedHeaderList, pParsedHeader, pError)!=SipSuccess)
		{
			sip_freeSipHeader(pParsedHeader);
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
	}
	sip_freeSipMessage(dParserParam.pSipMessage);
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_parseSingleHeader
**
** DESCRIPTION: This function parses a header string to form a SipHeader
**		structure containing the parsed header. This function can only
**		be used to parse known headers.
** 
** PARAMETERS:
**	pParsedHeaderList(IN) - The string containing the header to be parsed.
**	dType(IN) - The type of of the header string that is to be parsed.
**		For types like Contact or Expires, the Any type should be used.
**		The function will return fail if this type does not match with 
**		the type of the header given in the string.
**	pHeader(OUT) - The header structure which will contain the parsed
**		header on return. This structure should be initialized using
**		the sip_initSipHeader function with header type SipHdrTypeAny.
**	pError(OUT) - Error indication in case of failure.
**
*********************************************************************/

SipBool sip_parseSingleHeader
#ifdef ANSI_PROTO
(SIP_S8bit *pHeaderStr, en_HeaderType dType, SipHeader *pHeader, SipError *err)
#else
(pHeaderStr, dType, pHeader, err)
SIP_S8bit *pHeaderStr;
en_HeaderType dType;
SipHeader *pHeader;
SipError *err;
#endif
{
	SipHeaderParserParam	dParserParam;
	SipList					GCList;
	SIP_U32bit				count;
	SipError				tempError;
	SIP_S8bit				*temp;

	if(err == SIP_NULL)
		return SipFail;

	if(sip_validateHeaderString(pHeaderStr, dType, err)!=SipSuccess)
		return SipFail;

	sip_changeTypeAny(&dType, err);

	/* Check if header type is unknown. If so dont use the parser */
	if(dType == SipHdrTypeUnknown)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	/* Initialize a parser strcture for passing to the bison parser */
	/* Get the dType of the header and initialize a message structure for it */
	if((isSipGeneralHeader(dType)==SipTrue)||(isSipReqHeader(dType)==SipTrue))
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageRequest,\
			err)==SipFail)
			return SipFail;
	}
	else
	{
		if(sip_initSipMessage(&(dParserParam.pSipMessage),SipMessageResponse,\
			err)==SipFail)
			return SipFail;
	}
	if(sip_listInit(&GCList, __sip_freeString, err)==SipFail)
		return SipFail;
	dParserParam.pGCList = &GCList;
	dParserParam.pError = err;
	*err = E_NO_ERROR;

	temp = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, strlen(pHeaderStr)+2, err);
	if(temp == SIP_NULL)
		return SipFail;
	strcpy(temp, pHeaderStr);
	temp[strlen(temp)+1]='\0';
	/* Invoke scan_buffer to set up lexical scanner */
	if(glbSipParserScanBuffer[dType](temp, strlen(pHeaderStr)+2)!=0)
	{
		*(dParserParam.pError) = E_NO_MEM;
		fast_memfree(DECODE_MEM_ID, temp, SIP_NULL);
		return SipFail;
	}
	/*  Call a reset if available */
	if (glbSipParserReset[dType] != SIP_NULL)
		glbSipParserReset[dType]();		
	/* Invoke the parser */
	glbSipParserParser[dType]((void *)&dParserParam);
	sip_listDeleteAll((dParserParam.pGCList), &tempError);
	/* Release lexer buffer */
	glbSipParserReleaseBuffer[dType]();
	
	fast_memfree(ACCESSOR_MEM_ID, temp, SIP_NULL);
	sip_listDeleteAll(&GCList,&tempError);
	/* Error during parsing */
	if(*err!=E_NO_ERROR)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(sip_getHeaderCount(dParserParam.pSipMessage, dType, &count,\
		err)!=SipSuccess)
	{
		sip_freeSipMessage(dParserParam.pSipMessage);
		return SipFail;
	}
	if(count==0)
		return SipFail;
	{
		if(sip_getHeaderAtIndex(dParserParam.pSipMessage, dType,pHeader,\
			0, err)!=SipSuccess)
		{
			sip_freeSipMessage(dParserParam.pSipMessage);
			return SipFail;
		}
	}
	sip_freeSipMessage(dParserParam.pSipMessage);
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_formUnknownHeader
**
** DESCRIPTION: This function forms an unknown header structure from a 
**	list of known headers. This is useful for inserting headers unrecognized
**	by the stack but complying to a grammar similar to that of a known header.
**	The headers are formed using structures of the known header type. After
**	this the header structures are converted to an unknown header structure
**	suitable for insertion in a message structure.
**
** PARAMETERS:
**	pKnownHeaderList(IN) - A list of SipHeader structures containing the
**		the headers that need to be converted to unknown header suitable
**		for insertion in the SIP message.
**	pName(IN) - The name to be used in the unknown header
**	pUnknownHeader(IN/OUT) - A SipHeader structure in which the unknown header
**		formed will be returned. This should be initialized to type
**		SipHdrTypeUnknown before the function is invoked.
**	pError(OUT) - Returns the error code if the function fails.
**
*********************************************************************/
SipBool sip_formUnknownHeader
#ifdef ANSI_PROTO
(SipList *pKnownHeaderList, const SIP_S8bit *pName, \
SipHeader *pUnknownHeader, SipError *pError)
#else
(pKnownHeaderList, pName, pUnknownHeader, pError)
SipList *pKnownHeaderList;
SIP_S8bit *pName;
SipHeader *pUnknownHeader;
SipError *pError;
#endif
{
	SIP_U32bit i,count;	
	char buffer[SIP_MAX_HDR_SIZE];
	en_HeaderType type;
	SipMessage *tempMessage;
	type = SipHdrTypeAny;
	
	/* Validate header list for types */
	sip_listSizeOf(pKnownHeaderList, &count, pError);
	for(i=0;i<count;i++)
	{
		SipHeader *pHeader;
		sip_listGetAt(pKnownHeaderList, i, (SIP_Pvoid*)&pHeader, pError);
		if(i==0)
			type = pHeader->dType;
		else if(type!=pHeader->dType)
			return SipFail;
	}

	if((isSipReqHeader(type)==SipTrue) || (isSipGeneralHeader(type)==SipTrue))
		sip_initSipMessage(&tempMessage, SipMessageRequest, pError);
	else
		sip_initSipMessage(&tempMessage, SipMessageResponse, pError);

	((SipUnknownHeader *)pUnknownHeader->pHeader)->pName = STRDUP(pName);
	if(((SipUnknownHeader *)pUnknownHeader->pHeader)->pName == SIP_NULL)
	{
		sip_freeSipMessage(tempMessage);
		*pError = E_NO_MEM;
		return SipFail;
	}
	buffer[0]='\0';
	for(i=0;i<count;i++)
	{
		SipHeader *pHeader;
		en_AdditionMode mode;
		sip_listGetAt(pKnownHeaderList, i, (SIP_Pvoid*)&pHeader, pError);
		sip_insertHeaderAtIndex(tempMessage, pHeader, i, pError);
		if(i==0)
			mode = SipModeNone;
		else
			mode = SipModeJoin;
		{
		SIP_S8bit* tempbuf;
		tempbuf = buffer;
		sip_formSingleHeader(buffer+SIP_MAX_HDR_SIZE,type, i, mode, tempMessage, &tempbuf,\
			pError);
		}
	}
	buffer[strlen(buffer)-2]='\0';
	((SipUnknownHeader *)pUnknownHeader->pHeader)->pBody = STRDUP(buffer);
	sip_freeSipMessage(tempMessage);
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_getBadHeaderCount
**
** DESCRIPTION: This function returns the number of bad headers in the 
**	SIP message. Messages with bad headers are returned when a syntactically
**	incorrect message is passed to sip_decodeMessage with the SIP_OPT_BADMESSAGE
**	option.
** PARAMETERS:
**	pMessage (IN) - The SIP message that is to be examined for the presence of
**		bad headers
**	pCount (OUT) - Returns the number of bad headers in the message.
**	pError (OUT) - Returns the error code in case of failure.
**
*********************************************************************/
SipBool sip_getBadHeaderCount
#ifdef ANSI_PROTO
(SipMessage *pMessage, SIP_U32bit *pCount, SipError *pError)
#else
(pMessage, pCount, pError)
SipMessage *pMessage;
SIP_U32bit *pCount;
SipError *pError;
#endif
{
#ifndef SIP_NO_CHECK
	if(pError == SIP_NULL)
		return SipFail;
	if(pMessage == SIP_NULL)
	{
		*pError = E_INV_PARAM;
		return SipFail;
	}
	if(pCount == SIP_NULL)
	{
		*pError = E_INV_PARAM;
		return SipFail;
	}
#endif
	return sip_listSizeOf(&(pMessage->pGeneralHdr->slBadHdr), pCount, pError);
}

/********************************************************************
**
** FUNCTION:  sip_getBadHeaderAtIndex
**
** DESCRIPTION: This function returns a bad header from a SIP message
**	Messages with bad headers are returned when a syntactically
**	incorrect message is passed to sip_decodeMessage with the 
**	SIP_OPT_BADMESSAGE option.
** PARAMETERS:
**	pMessage (IN) - The SIP message that is to be examined for the presence of
**		bad headers
**	ppBadHdr (OUT) - Returns the bad header from the message.
**	index (IN) - This index of the bad header to be returned.
**	pError (OUT) - Returns the error code in case of failure.
**
*********************************************************************/
SipBool sip_getBadHeaderAtIndex
#ifdef ANSI_PROTO
(SipMessage *pMessage, SipBadHeader **ppBadHdr, SIP_U32bit index, SipError *pError)
#else
(pMessage, ppBadHdr, index, pError)
SipMessage *pMessage;
SipBadHeader **ppBadHdr;
SIP_U32bit index;
SipError *pError;
#endif
{
#ifndef SIP_NO_CHECK
	if(pError == SIP_NULL)
		return SipFail;
	if(pMessage == SIP_NULL)
	{
		*pError = E_INV_PARAM;
		return SipFail;
	}
	if(ppBadHdr == SIP_NULL)
	{
		*pError = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(sip_listGetAt(&(pMessage->pGeneralHdr->slBadHdr), index, (SIP_Pvoid *)\
		ppBadHdr, pError)==SipFail)
		return SipFail;
	HSS_LOCKEDINCREF((*ppBadHdr)->dRefCount);
	*pError = E_NO_ERROR;
	return SipSuccess;
}

