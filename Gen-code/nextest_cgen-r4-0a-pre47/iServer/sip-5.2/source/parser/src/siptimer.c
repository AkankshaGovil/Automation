/***********************************************************************
 ** FUNCTION:
 **            Timer Logic Implementaion
 *********************************************************************
 **
 ** FILENAME:
 **		siptimer.c
 **
 ** DESCRIPTION:
 ** 	This file contains the timer logic implementation
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 12/01/2000  Binu K S  			 			Initial creation 
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/
#include "sipstruct.h"
#include "sipcommon.h"
#include "sipsendmessage.h"
#include "siptimer.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipstatistics.h"
#include "siptrace.h"
#include "sipdecode.h"
#include "rprinit.h"
#include "rprfree.h"
#include "sipparserclone.h"

#ifdef SIP_TEL
#include "telapi.h"
#include "telfree.h" 
#ifndef SIP_BY_REFERENCE 
#include "telinit.h"
#endif 
#endif



/*
   Retransmission Count
   Start at T1, keep resending till T2 is reached. 
   After that send in intervals of T2. Stop when count reaches 11
   if a provisional response is received, restransmit in steps of T2
 */

SipBool sip_getCharString
#ifdef ANSI_PROTO
( SIP_S8bit	*inputstr,
  SIP_S8bit	**outstr,
  SipError	*err )
#else
( inputstr, outstr, err)
	SIP_S8bit     *inputstr;
	SIP_S8bit     **outstr;
	SipError      *err;
#endif
{
	SIP_S32bit	val;
	SIP_S8bit	*outputstr, *tempstr, *temp;
	SIP_U32bit	dLength;

	if( inputstr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	dLength = strlen(inputstr);

	temp = (SIP_S8bit *)fast_memget(TIMER_MEM_ID, 5, err); 
	if(temp == SIP_NULL)
		return SipFail;
	tempstr = (SIP_S8bit *)fast_memget(TIMER_MEM_ID, dLength+1, err);
	if(tempstr == SIP_NULL)
	{
		fast_memfree(TIMER_MEM_ID, temp, err);
		return SipFail;
	}
	outputstr = tempstr;

	while (*inputstr != '\0')
	{
		if (*inputstr == '%')
		{
			strncpy (temp, ++inputstr, 2);
			*(temp+2) = '\0';
			val = (SIP_S32bit) strtol(temp, 0, SIP_BASE_16);
			*tempstr++ = val;
			inputstr++;
		}
		else
			*tempstr++ = *inputstr;
		inputstr++;
	}

	*tempstr = '\0';
	if( sip_memfree(TIMER_MEM_ID, (SIP_Pvoid *)(&temp), err) == SipFail)
		return SipFail;

	*outstr = outputstr;
	return SipSuccess;
} 


SipBool sip_isEqualPort
#ifdef ANSI_PROTO
( SIP_U16bit	*port1, 
  SIP_U16bit	*port2 )
#else
( port1, port2 )
	SIP_U16bit	*port1;
	SIP_U16bit    *port2;
#endif
{
	if( port1 == SIP_NULL && port2 == SIP_NULL )
		return SipSuccess;
	else
	{
		if( port1 == SIP_NULL && port2 != SIP_NULL )
		{
			if( *port2 == SIP_DEFAULT_PORT )
				return SipSuccess;
			else
				return SipFail;
		}

		if( port2 == SIP_NULL && port1 != SIP_NULL )
		{
			if( *port1 == SIP_DEFAULT_PORT )
				return SipSuccess;
			else
				return SipFail;
		}
	}

	if( *port1 == *port2 )
		return SipSuccess;
	else
		return SipFail;
}


SipBool sip_isEqualSipUrl
#ifdef ANSI_PROTO
( SipUrl	*url1,
  SipUrl	*url2,
  SipError      *err )
#else
( url1, url2, err )
	SipUrl        *url1;
	SipUrl        *url2;
	SipError      *err;
#endif
{
	SIP_S8bit	*temp1, *temp2;
	SIP_S32bit		result;

	if( url1 == SIP_NULL || url2 == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if((url1->pUser == SIP_NULL) && (url2->pUser != SIP_NULL))
		return SipFail;
	if((url2->pUser == SIP_NULL) && (url1->pUser != SIP_NULL))
		return SipFail;
	if((url1->pUser != SIP_NULL) && (url2->pUser != SIP_NULL))
	{
		if(sip_getCharString(url1->pUser, &temp1, err) == SipFail)
			return SipFail;
		if(sip_getCharString(url2->pUser, &temp2, err) == SipFail)
			return SipFail;
		result = strcasecmp(temp1, temp2);
		if( sip_memfree(TIMER_MEM_ID, (SIP_Pvoid *)(&temp1), err) == SipFail )
			return SipFail;
		if( sip_memfree(TIMER_MEM_ID, (SIP_Pvoid *)(&temp2), err) == SipFail )
			return SipFail;
		if( result !=0 )	
			return SipFail;	
	}

	if((url1->pPassword == SIP_NULL) && (url2->pPassword != SIP_NULL))
		return SipFail;
	if((url2->pPassword == SIP_NULL) && (url1->pPassword != SIP_NULL))
		return SipFail;
	if((url1->pPassword != SIP_NULL) && (url2->pPassword != SIP_NULL))
	{
		if(sip_getCharString(url1->pPassword, &temp1, err) == SipFail)
			return SipFail;
		if(sip_getCharString(url2->pPassword, &temp2, err) == SipFail)
			return SipFail;
		result = strcmp(temp1, temp2);
		if( sip_memfree(TIMER_MEM_ID, (SIP_Pvoid *)(&temp1), err) == SipFail )
			return SipFail;
		if( sip_memfree(TIMER_MEM_ID, (SIP_Pvoid *)(&temp2), err) == SipFail )
			return SipFail;
		if( result != 0 )
			return SipFail;
	}

	if( strcasecmp( url1->pHost, url2->pHost ) )
		return SipFail;

	if( !sip_isEqualPort( url1->dPort, url2->dPort) )
		return SipFail; 

	return SipSuccess;
}


SipBool sip_isEqualReqUri
#ifdef ANSI_PROTO
( SIP_S8bit	*uri1,
  SIP_S8bit	*uri2,
  SipError	*err )
#else
( uri1, uri2, err )
	SIP_S8bit*    uri1;
	SIP_S8bit*    uri2;
	SipError      *err;
#endif
{
	SIP_S8bit *temp1,*temp2;
	SIP_S32bit result;

	if( uri1 == SIP_NULL || uri2 == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(sip_getCharString(uri1, &temp1, err)==SipFail)
		return SipFail;
	if(sip_getCharString(uri2, &temp2, err)==SipFail)
		return SipFail;
	result = strcasecmp(temp1, temp2);
	if( sip_memfree(TIMER_MEM_ID, (SIP_Pvoid *)(&temp1), err) == SipFail )
		return SipFail;
	if( sip_memfree(TIMER_MEM_ID, (SIP_Pvoid *)(&temp2), err) == SipFail )
		return SipFail;

	if(result !=0 )
		return SipFail;
	else 
		return SipSuccess;
}

SipBool sip_compareToHeaders
#ifdef ANSI_PROTO
( SipToHeader	*hdr1,
  SipToHeader	*hdr2,
  SipError	*err )
#else
( hdr1, hdr2, err )
	SipToHeader *hdr1;
	SipToHeader *hdr2;
	SipError      *err;
#endif
{
	SIP_U32bit size1, size2;	
	SIP_S8bit *str1, *str2;
#ifdef SIP_TEL
	TelUrl* pTelUri1;
	TelUrl* pTelUri2;
#endif


	if( hdr1 == SIP_NULL || hdr2 == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if( ((hdr1->pAddrSpec->dType == SipAddrSipUri) &&
				(hdr2->pAddrSpec->dType == SipAddrSipUri )) ||
			((hdr1->pAddrSpec->dType == SipAddrSipSUri) &&
			 (hdr2->pAddrSpec->dType == SipAddrSipSUri )))
	{
		if(sip_isEqualSipUrl( (hdr1->pAddrSpec->u).pSipUrl,\
					(hdr2->pAddrSpec->u).pSipUrl,err )!=SipSuccess)
			return SipFail;
	}
#ifdef SIP_TEL
	else if(sip_isTelUrl(hdr1->pAddrSpec,err) && \
			(sip_isTelUrl(hdr1->pAddrSpec,err)))
	{
#ifdef SIP_BY_REFERENCE
		if(sip_getTelUrlFromAddrSpec(hdr1->pAddrSpec,&pTelUri1,err) == SipFail)
			return SipFail;
		if(sip_getTelUrlFromAddrSpec(hdr2->pAddrSpec,&pTelUri2,err) == SipFail)
			return SipFail;
#else
		sip_initTelUrl(&pTelUri1,err);
		sip_initTelUrl(&pTelUri2,err);
		if(sip_getTelUrlFromAddrSpec(hdr1->pAddrSpec,pTelUri1,err) == SipFail)
			return SipFail;
		if(sip_getTelUrlFromAddrSpec(hdr2->pAddrSpec,pTelUri2,err) == SipFail)
			return SipFail;
#endif
		if(	sip_compareTelUri(pTelUri1,pTelUri2,err) == SipFail)
		{
#ifndef SIP_BY_REFERENCE
			sip_freeTelUrl (pTelUri1);
			sip_freeTelUrl (pTelUri2);
#endif
			return SipFail;
		}
#ifndef SIP_BY_REFERENCE
		sip_freeTelUrl (pTelUri1);
		sip_freeTelUrl (pTelUri2);
#endif
	}
#endif
	else if( hdr1->pAddrSpec->dType == SipAddrReqUri &&
			hdr2->pAddrSpec->dType == SipAddrReqUri )
	{
		if(sip_isEqualReqUri( (hdr1->pAddrSpec->u).pUri,\
					(hdr2->pAddrSpec->u).pUri,err ) != SipSuccess)
			return SipFail;
	}
	else
		return SipFail;
	if(sip_listSizeOf(&(hdr1->slTag),&size1,err)!=SipSuccess) 
		return SipFail;
	if(sip_listSizeOf(&(hdr2->slTag),&size2,err)!=SipSuccess) 
		return SipFail;
	if((size1==0)&&(size2==0))
		return SipSuccess;
	if((size1==0)&&(size2!=0))
		return SipSuccess;
	if((size2==0)&&(size1!=0))
		return SipFail;
	if(sip_listGetAt(&(hdr1->slTag), 0, (SIP_Pvoid *)&str1, err) != SipSuccess)
		return SipFail;	
	if(sip_listGetAt(&(hdr2->slTag), 0, (SIP_Pvoid *)&str2, err) != SipSuccess)
		return SipFail;	
	if(strcmp(str1,str2)!=0)
		return SipFail;

	return SipSuccess;
}	



SipBool sip_compareFromHeaders
#ifdef ANSI_PROTO
( SipFromHeader	*hdr1,
  SipFromHeader	*hdr2,
  SipError	*err )
#else
( hdr1, hdr2, err )
	SipFromHeader *hdr1;
	SipFromHeader *hdr2;
	SipError      *err;
#endif
{
	SIP_U32bit size1, size2, index;	
	SIP_S8bit *str1, *str2;
#ifdef SIP_TEL
	TelUrl* pTelUri1;
	TelUrl* pTelUri2;
#endif

	if( hdr1 == SIP_NULL || hdr2 == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(((hdr1->pAddrSpec->dType == SipAddrSipUri) &&
				(hdr2->pAddrSpec->dType == SipAddrSipUri )) ||
			((hdr1->pAddrSpec->dType == SipAddrSipSUri) &&
			 (hdr2->pAddrSpec->dType == SipAddrSipSUri )))
	{
		if(sip_isEqualSipUrl( (hdr1->pAddrSpec->u).pSipUrl,\
					(hdr2->pAddrSpec->u).pSipUrl,err )!=SipSuccess)
			return SipFail;
	}
#ifdef SIP_TEL
	else if(sip_isTelUrl(hdr1->pAddrSpec,err) && \
			(sip_isTelUrl(hdr1->pAddrSpec,err)))
	{
#ifdef SIP_BY_REFERENCE
		if(sip_getTelUrlFromAddrSpec(hdr1->pAddrSpec,&pTelUri1,err) == SipFail)
			return SipFail;
		if(sip_getTelUrlFromAddrSpec(hdr2->pAddrSpec,&pTelUri2,err) == SipFail)
			return SipFail;
#else
		sip_initTelUrl(&pTelUri1,err);
		sip_initTelUrl(&pTelUri2,err);
		if(sip_getTelUrlFromAddrSpec(hdr1->pAddrSpec,pTelUri1,err) == SipFail)
			return SipFail;
		if(sip_getTelUrlFromAddrSpec(hdr2->pAddrSpec,pTelUri2,err) == SipFail)
			return SipFail;
#endif
		if(	sip_compareTelUri(pTelUri1,pTelUri2,err) == SipFail)
		{
#ifndef SIP_BY_REFERENCE
			sip_freeTelUrl (pTelUri1);
			sip_freeTelUrl (pTelUri2);
#endif
			return SipFail;
		}
#ifndef SIP_BY_REFERENCE
		sip_freeTelUrl (pTelUri1);
		sip_freeTelUrl (pTelUri2);
#endif
	}
#endif
	else if( hdr1->pAddrSpec->dType == SipAddrReqUri &&\
			hdr2->pAddrSpec->dType == SipAddrReqUri )
	{
		if(sip_isEqualReqUri( (hdr1->pAddrSpec->u).pUri,\
					(hdr2->pAddrSpec->u).pUri,err ) != SipSuccess)
			return SipFail;
	}
	else
		return SipFail;
	if(sip_listSizeOf(&(hdr1->slTag),&size1,err)!=SipSuccess) 
		return SipFail;
	if(sip_listSizeOf(&(hdr2->slTag),&size2,err)!=SipSuccess) 
		return SipFail;
	if(size1!=size2)
		return SipFail;
	index = 0;
	while(index < size1)
	{
		if(sip_listGetAt(&(hdr1->slTag), index, (SIP_Pvoid *)&str1, err)\
				!= SipSuccess)
			return SipFail;	
		if(sip_listGetAt(&(hdr2->slTag), index, (SIP_Pvoid *)&str2, err)\
				!= SipSuccess)
			return SipFail;	
		if(strcmp(str1,str2)!=0)
			return SipFail;
		index++;
	}
	return SipSuccess;
}	

SipBool sip_getBranchFromViaHdr
#ifdef ANSI_PROTO
(SipViaHeader *pViaHeader,\
 SIP_S8bit **ppBranch,\
 SipError *pErr)
#else
(pViaHeader,ppBranch,pErr)
	SipViaHeader *pViaHeader;
	SIP_S8bit **ppBranch;
	SipError *pErr;
#endif
{
	SIP_U32bit index, size;

	*pErr = E_NO_ERROR;

	if(sip_listSizeOf(&(pViaHeader->slParam), &size, pErr) == SipFail)
		return SipFail;
	index = 0;
	while (index < size)
	{
		SipParam *pParam;

		if(sip_listGetAt(&(pViaHeader->slParam), index, ((SIP_Pvoid *) &pParam)\
					, pErr) == SipFail)
			return SipFail;
		if(pParam->pName != SIP_NULL)
		{
			if(strcmp(pParam->pName,"branch")==0)
			{
				SIP_S8bit *pBranch;

				if(sip_listGetAt(&(pParam->slValue),0,((SIP_Pvoid *) &pBranch),\
							pErr) != SipFail)
				{
					*ppBranch = pBranch;
					return SipSuccess;
				}
			}
		}
		index++;
	}
	return SipFail;
}

SipBool sip_compareTimerKeys
#ifdef ANSI_PROTO
(SipTimerKey *paramkey, SipTimerKey *storedkey, SipError *err)
#else
(paramkey, storedkey, err)
	SipTimerKey *paramkey; 
	SipTimerKey *storedkey; 
	SipError *err;
#endif
{
	if(paramkey->dMatchFlag == TIMER_NONE)
		return SipFail;
	/* Check if any of the keys has to be matched independent of message type */
	if((paramkey->dMatchType!=SipMessageAny) && \
			(storedkey->dMatchType!=SipMessageAny))
	{
		/* Match keys only if message type matches exactly */
		if(paramkey->dMatchType != storedkey->dMatchType)
			return SipFail;	
	}
	/* Compare Call-Id first since it less expensive compared to From 
	   and To header comparisons. The expensive comparisons may be 
	   carried out only if this check succeeds  */
	if(strcmp(paramkey->dCallid,storedkey->dCallid)!=0)
		return SipFail;
	/* No compare From and To headers */
	if(sip_compareFromHeaders(paramkey->pFromHdr,storedkey->pFromHdr,err)\
			==SipFail)
		return SipFail;
	if(sip_compareToHeaders(storedkey->pToHdr,paramkey->pToHdr,err)==SipFail)
		return SipFail;
	/* Compare Via branch parameters */
	/* Do not perform Via branch checking for PRACK request matching */
	if(paramkey->dMatchFlag != TIMER_RPR)
	{
		/* Do not perform Via branch checking for responses sent */
		if((storedkey->dMatchType != SipMessageResponse))
		{
			if(((storedkey->pViaBranch!=SIP_NULL)\
						&&(paramkey->pViaBranch==SIP_NULL))\
					||((storedkey->pViaBranch==SIP_NULL)\
						&&(paramkey->pViaBranch!=SIP_NULL)))
				return SipFail;
			if((storedkey->pViaBranch != SIP_NULL)&&(paramkey->pViaBranch\
						!= SIP_NULL))
			{
				if(strcmp(storedkey->pViaBranch, paramkey->pViaBranch)!=0)
					return SipFail;
			}
		}
	}

	switch (paramkey->dMatchFlag)
	{
		case TIMER_TRANSACTION:
			/* Perform CSeq comparisons */
			if(storedkey->dMatchType == SipMessageResponse)
				if(storedkey->dCodeNum < 200)
					return SipFail;
			if(paramkey->dCseq==storedkey->dCseq)
			{
				/* For ACK, only CSeq number is to be matched */
				/* For other methods, the CSeq method must match exactly */
				if((strcmp(paramkey->pMethod,"ACK")==0)) 
					return SipSuccess;
				else if((strcmp(paramkey->pMethod,storedkey->pMethod)==0)) 
					return SipSuccess;
			}
			return SipFail;

		case TIMER_RPR:
			/* Matching for reliable response retransmissions */
			/* CSeq number must be greater that of message being acknowledged */
			if(paramkey->dCseq <= storedkey->dCseq)
				return SipFail;
			/* CSequence number in the RAck header must match the CSeq number
			   of the message being acknowledged */
			if(paramkey->pRackHdr->dCseqNum != storedkey->dCseq)
				return SipFail;
			/* Methods in the RAck header must match the CSeq method of the 
			   message being acknowledged */
			if(strcmp(paramkey->pRackHdr->pMethod,storedkey->pMethod)!=0)
				return SipFail;
			/* If all the conditions above are met, the message acknowledges
			   all responses with RSeq sequence less than or equal to the 
			   response number in the RACK header .
			   Changed to perform an exact match as required by 03 draft*/
			if(paramkey->pRackHdr->dRespNum != storedkey->dRseq)
				return SipFail;
			return SipSuccess;
	}
	return SipFail;
}

SipBool sip_stopTimer
#ifdef ANSI_PROTO
(SipMessage *msg, SipEventContext *pContext, SipError *err)
#else
(msg, pContext, err)
	SipMessage *msg; 
	SipEventContext *pContext;
	SipError *err;
#endif
{
	SipTimerKey tkey,*outkey=SIP_NULL;
	SipTimerBuffer *tbuf;
	SIP_Pvoid outbuf;
	SIP_U32bit count;

 	/* NEXTONE - return */
	return 0;

	switch(msg->dType)
	{
		case SipMessageResponse:
			{
				/* Decoded message is a response. 
				 * Will probably terminate matching request retransmissions */
				/* Form the timer key to be used for the cancellation */
				tkey.dCallid = \
							STRDUPTIMER(msg->pGeneralHdr->pCallidHdr->pValue);
				if(tkey.dCallid == SIP_NULL)
				{
					*err = E_NO_MEM;
					return SipFail;
				}
				tkey.pMethod = STRDUPTIMER(msg->pGeneralHdr->pCseqHdr->pMethod);
				if(tkey.pMethod == SIP_NULL)
				{
					*err = E_NO_MEM;
					return SipFail;
				}
				tkey.dCseq = msg->pGeneralHdr->pCseqHdr->dSeqNum;
				/* This key should match retransmission timers for 
				 * requests only */
				tkey.dMatchType = SipMessageRequest;
				/* Matching comparison should follow TRANSACTION 
				 * matching rules */
				tkey.dMatchFlag = TIMER_TRANSACTION;
				/* Clone the From and To headers into the timer key */
				if(sip_initSipFromHeader(&(tkey.pFromHdr),err)==SipFail)
					return SipFail;
				if(__sipParser_cloneSipFromHeader(tkey.pFromHdr,msg->\
							pGeneralHdr->pFromHeader,err) == SipFail)
					return SipFail;
				if(sip_initSipToHeader(&(tkey.pToHdr),err)==SipFail)
					return SipFail;
				if(__sipParser_cloneSipToHeader(tkey.pToHdr,msg->pGeneralHdr->\
							pToHdr,err) == SipFail)
					return SipFail;
				/* Add branch from first Via Header */
				tkey.pViaBranch = SIP_NULL;
				/* Check if Via header present */
				if(sip_listSizeOf(&(msg->pGeneralHdr->slViaHdr), &count, \
						err) != SipFail)
				{
					if(count!=0)
					{
						SipViaHeader *pViaHeader;

						/* Get first via header from the list */
						if(sip_listGetAt(&(msg->pGeneralHdr->slViaHdr), 0, \
									(SIP_Pvoid *) &pViaHeader , err) != SipFail)
						{
							SIP_S8bit *pViaBranch;
							/* Get Branch parameter */
							if(sip_getBranchFromViaHdr(pViaHeader, \
										&pViaBranch, err) !=SipFail)
							{
								tkey.pViaBranch = STRDUPTIMER(pViaBranch);
								if(tkey.pViaBranch == SIP_NULL)
								{
									*err = E_NO_MEM;
									return SipFail;
								}
							}
						}
					}
				}

				/* Ask application to stop timers that match the formed key */
				tkey.pContext = pContext;
				if(fast_stopTimer(&tkey,&outkey,&outbuf,err)==SipFail)
				{
					fast_memfree(TIMER_MEM_ID,tkey.dCallid,SIP_NULL);
					fast_memfree(TIMER_MEM_ID,tkey.pMethod,SIP_NULL);
					if(tkey.pViaBranch != SIP_NULL)
						fast_memfree(TIMER_MEM_ID, tkey.pViaBranch, SIP_NULL);
					sip_freeSipFromHeader(tkey.pFromHdr);
					sip_freeSipToHeader(tkey.pToHdr);
					return SipFail;
				}
				sip_freeSipFromHeader(tkey.pFromHdr);
				sip_freeSipToHeader(tkey.pToHdr);
				fast_memfree(TIMER_MEM_ID,tkey.dCallid,SIP_NULL);
				fast_memfree(TIMER_MEM_ID,tkey.pMethod,SIP_NULL);
				if(tkey.pViaBranch != SIP_NULL)
					fast_memfree(TIMER_MEM_ID, tkey.pViaBranch, SIP_NULL);
				break;
			}

		case SipMessageRequest:
			{
				/* Request message. May cause some response retransmissions to
				   cease */
				if(strcmp("ACK",msg->u.pRequest->pRequestLine->pMethod)==0)
				{
					/* ACK message, will end INVITE response retransmissions */
					/* Form timer key */
					tkey.dCallid=\
						STRDUPTIMER(msg->pGeneralHdr->pCallidHdr->pValue);
					if(tkey.dCallid == SIP_NULL)
					{
						*err = E_NO_MEM;
						return SipFail;
					}
					tkey.pMethod=\
						STRDUPTIMER(msg->pGeneralHdr->pCseqHdr->pMethod);
					if(tkey.pMethod == SIP_NULL)
					{
						*err = E_NO_MEM;
						return SipFail;
					}
					tkey.dCseq=msg->pGeneralHdr->pCseqHdr->dSeqNum;
					/* This key should match retransmission timers for 
					 * responses only */
					tkey.dMatchType = SipMessageResponse;
					/* Matching comparison should follow TRANSACTION matching 
					   rules */
					tkey.dMatchFlag = TIMER_TRANSACTION;
					/* clone from and to headers into the timer Key fields */
					if(sip_initSipFromHeader(&(tkey.pFromHdr),err)==SipFail)
						return SipFail;
					if(__sipParser_cloneSipFromHeader(tkey.pFromHdr,msg->\
								pGeneralHdr->pFromHeader,err) == SipFail)
						return SipFail;
					if(sip_initSipToHeader(&(tkey.pToHdr),err)==SipFail)
						return SipFail;
					if(__sipParser_cloneSipToHeader(tkey.pToHdr,msg->\
								pGeneralHdr->pToHdr,err) == SipFail)
						return SipFail;
					/* Add branch from first Via Header */
					tkey.pViaBranch = SIP_NULL;
					/* Check if Via header present */
					if(sip_listSizeOf(&(msg->pGeneralHdr->slViaHdr), &count, \
							err) !=SipFail)
					{
						if(count!=0)
						{
							SipViaHeader *pViaHeader;

							/* Get first via header from the list */
							if(sip_listGetAt(&(msg->pGeneralHdr->slViaHdr), 0, \
										(SIP_Pvoid *) &pViaHeader , err) \
										!= SipFail)
							{
								SIP_S8bit *pViaBranch;
								/* Get Branch parameter */
								if(sip_getBranchFromViaHdr(pViaHeader, \
											&pViaBranch, err) !=SipFail)
								{
									tkey.pViaBranch = STRDUPTIMER(pViaBranch);
									if(tkey.pViaBranch == SIP_NULL)
									{
										*err = E_NO_MEM;
										return SipFail;
									}
								}
							}
						}
					}

					/* Ask application to stop timers that match the 
					 * formed key */
					tkey.pContext = pContext;
					if(fast_stopTimer(&tkey,&outkey,&outbuf,err)==SipFail)
					{
						fast_memfree(TIMER_MEM_ID,tkey.dCallid,SIP_NULL);
						fast_memfree(TIMER_MEM_ID,tkey.pMethod,SIP_NULL);
						if(tkey.pViaBranch != SIP_NULL)
							fast_memfree(TIMER_MEM_ID,tkey.pViaBranch,SIP_NULL);
						sip_freeSipFromHeader(tkey.pFromHdr);
						sip_freeSipToHeader(tkey.pToHdr);
						return SipFail;
					}
					sip_freeSipFromHeader(tkey.pFromHdr);
					sip_freeSipToHeader(tkey.pToHdr);
					fast_memfree(TIMER_MEM_ID,tkey.dCallid,SIP_NULL);
					fast_memfree(TIMER_MEM_ID,tkey.pMethod,SIP_NULL);
					if(tkey.pViaBranch != SIP_NULL)
						fast_memfree(TIMER_MEM_ID,tkey.pViaBranch,SIP_NULL);
				}
				else if(strcmp("PRACK",msg->u.pRequest->pRequestLine->pMethod)\
								==0)
				{
					/* Message is a PRACK request used for acknowledgement of 
					   reliably sent provisional responses */
					/* Form timer key */
					tkey.dCallid=\
						STRDUPTIMER(msg->pGeneralHdr->pCallidHdr->pValue);
					if(tkey.dCallid == SIP_NULL)
					{
						*err = E_NO_MEM;
						return SipFail;
					}
					/* This key must be used for matching responses only */
					tkey.dMatchType = SipMessageResponse;
					tkey.dCseq = msg->pGeneralHdr->pCseqHdr->dSeqNum;
					if(sip_rpr_initSipRAckHeader(&(tkey.pRackHdr),err)==SipFail)
						return SipFail;
					tkey.pRackHdr->dRespNum = msg->u.pRequest->pRequestHdr->\
						pRackHdr->dRespNum;
					tkey.pRackHdr->dCseqNum = msg->u.pRequest->pRequestHdr->\
						pRackHdr->dCseqNum;
					tkey.pRackHdr->pMethod = (SIP_S8bit *)STRDUPTIMER\
						(msg->u.pRequest->pRequestHdr->pRackHdr->pMethod);
					if(tkey.pRackHdr->pMethod == SIP_NULL)
					{
						*err = E_NO_MEM;
						return SipFail;
					}
					/* Use different logic when comparing keys. Logic must take
					   into account the RSeq and RACK header matching */
					tkey.dMatchFlag = TIMER_RPR;
					/* clone from and to headers into the timer Key fields */
					if(sip_initSipFromHeader(&(tkey.pFromHdr),err)==SipFail)
						return SipFail;
					if(__sipParser_cloneSipFromHeader(tkey.pFromHdr,\
								msg->pGeneralHdr->pFromHeader,err) == SipFail)
						return SipFail;
					if(sip_initSipToHeader(&(tkey.pToHdr),err)==SipFail)
						return SipFail;
					if(__sipParser_cloneSipToHeader(tkey.pToHdr,\
								msg->pGeneralHdr->pToHdr,err) == SipFail)
						return SipFail;
					/* Add branch from first Via Header */
					tkey.pViaBranch = SIP_NULL;
					/* Check if Via header present */
					if(sip_listSizeOf(&(msg->pGeneralHdr->slViaHdr), &count, \
							err) !=SipFail)
					{
						if(count!=0)
						{
							SipViaHeader *pViaHeader;

							/* Get first via header from the list */
							if(sip_listGetAt(&(msg->pGeneralHdr->slViaHdr), 0, \
										(SIP_Pvoid *) &pViaHeader , err) != \
										SipFail)
							{
								SIP_S8bit *pViaBranch;
								/* Get Branch parameter */
								if(sip_getBranchFromViaHdr(pViaHeader, \
											&pViaBranch, err) !=SipFail)
								{
									tkey.pViaBranch = STRDUPTIMER(pViaBranch);
									if(tkey.pViaBranch == SIP_NULL)
									{
										*err = E_NO_MEM;
										return SipFail;
									}
								}
							}
						}
					}
					/* Ask application to stop matching retransmissions */	
					tkey.pContext = pContext;
					if(fast_stopTimer(&tkey,&outkey,&outbuf,err)==SipFail)
					{
						fast_memfree(TIMER_MEM_ID,tkey.dCallid,SIP_NULL);
						sip_freeSipFromHeader(tkey.pFromHdr);
						sip_freeSipToHeader(tkey.pToHdr);
						sip_rpr_freeSipRAckHeader(tkey.pRackHdr);
						if(tkey.pViaBranch != SIP_NULL)
							fast_memfree(TIMER_MEM_ID,tkey.pViaBranch,SIP_NULL);
						return SipFail;
					}
					fast_memfree(TIMER_MEM_ID,tkey.dCallid,SIP_NULL);
					sip_freeSipFromHeader(tkey.pFromHdr);
					sip_freeSipToHeader(tkey.pToHdr);
					sip_rpr_freeSipRAckHeader(tkey.pRackHdr);
					if(tkey.pViaBranch != SIP_NULL)
						fast_memfree(TIMER_MEM_ID,tkey.pViaBranch,SIP_NULL);
				}
				break;
			}	
		default:;

	}
	/* This section of code entered only if a fast_stopTimer returned 
	   successfully. Free all resources returned by the application on 
	   stopping the timer */
	tbuf = (SipTimerBuffer *) outbuf;
	sip_freeSipTimerKey(outkey);
	sip_freeSipTimerBuffer(tbuf);
	return SipSuccess;
}

/************************************************************************
 ** FUNCTION:sip_updateTimer
 **
 ** DESCRIPTION: This function is invoked by the stack to change retrans
 ** count of a message. 
 **
 *************************************************************************/
SipBool sip_updateTimer
#ifdef ANSI_PROTO
(SipMessage *msg, SipBool infoflag, SipEventContext *pContext, SipError *err)
#else
(msg, infoflag, pContext, err)
	SipMessage *msg; 
	SipBool infoflag; 
	SipEventContext *pContext;
	SipError *err;
#endif
{
	SipTimerKey *tkey, *outkey=SIP_NULL;
	SipTimerBuffer *tbuf;
	SIP_Pvoid outbuf;
	SIP_U16bit retrans;
	SipGeneralHeader *gheader;
	SIP_U32bit count;

 	/* NEXTONE - return */
	return 0;

	tkey = (SipTimerKey *) fast_memget(TIMER_MEM_ID, sizeof(SipTimerKey),\
			SIP_NULL);
	gheader = msg->pGeneralHdr;

	tkey->dCallid = STRDUPTIMER(gheader->pCallidHdr->pValue);
	if(tkey->dCallid == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
	tkey->pMethod = STRDUPTIMER(gheader->pCseqHdr->pMethod);
	if(tkey->pMethod == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
	tkey->dCseq = gheader->pCseqHdr->dSeqNum;
	tkey->dMatchType = SipMessageAny;
	tkey->dMatchFlag = TIMER_TRANSACTION;
	/* clone From/To headers into timer Key */
	if(sip_initSipFromHeader(&(tkey->pFromHdr),err)==SipFail)
		return SipFail;
	if(__sipParser_cloneSipFromHeader(tkey->pFromHdr,gheader->pFromHeader,err)\
			== SipFail)
		return SipFail;
	if(sip_initSipToHeader(&(tkey->pToHdr),err)==SipFail)
		return SipFail;
	if(__sipParser_cloneSipToHeader(tkey->pToHdr,gheader->pToHdr,err) == \
			SipFail)
		return SipFail;
	/* Add branch from first Via Header */
	tkey->pViaBranch = SIP_NULL;
	/* Check if Via header present */
	if(sip_listSizeOf(&(msg->pGeneralHdr->slViaHdr), &count, err)\
			!=SipFail)
	{
		if(count!=0)
		{
			SipViaHeader *pViaHeader;

			/* Get first via header from the list */
			if(sip_listGetAt(&(msg->pGeneralHdr->slViaHdr), 0, \
						(SIP_Pvoid *) &pViaHeader , err) != SipFail)
			{
				SIP_S8bit *pViaBranch;
				/* Get Branch parameter */
				if(sip_getBranchFromViaHdr(pViaHeader, &pViaBranch,\
							err) !=SipFail)
				{
					tkey->pViaBranch = STRDUPTIMER(pViaBranch);
					if(tkey->pViaBranch == SIP_NULL)
					{
						*err = E_NO_MEM;
						return SipFail;
					}
				}
			}
		}
	}

	tkey->pContext = pContext;
	if(fast_stopTimer(tkey,&outkey,&outbuf,err)==SipFail) 
	{
		/* There is no timer entry for this response */
		fast_memfree(TIMER_MEM_ID,tkey->dCallid,SIP_NULL);
		fast_memfree(TIMER_MEM_ID,tkey->pMethod,SIP_NULL);
		if(tkey->pViaBranch != SIP_NULL)
			fast_memfree(TIMER_MEM_ID,tkey->pViaBranch,SIP_NULL);
		sip_freeSipFromHeader(tkey->pFromHdr);
		sip_freeSipToHeader(tkey->pToHdr);
		fast_memfree(TIMER_MEM_ID,tkey,SIP_NULL);
		return SipFail;
	}

	tbuf = (SipTimerBuffer *) outbuf;
	/* outkey now contains the TimerKey and outbuf, the sip message */

	fast_memfree(TIMER_MEM_ID,tkey->dCallid,SIP_NULL);
	fast_memfree(TIMER_MEM_ID,tkey->pMethod,SIP_NULL);
	if(tkey->pViaBranch != SIP_NULL)
		fast_memfree(TIMER_MEM_ID,tkey->pViaBranch,SIP_NULL);
	sip_freeSipFromHeader(tkey->pFromHdr);
	sip_freeSipToHeader(tkey->pToHdr);
	fast_memfree(TIMER_MEM_ID,tkey,SIP_NULL);

	/* Now change Duration according to Information Flag */
	tbuf->dDuration = tbuf->dDuration * 2;
	if ((tbuf->dInfoReceived == SipSuccess) || (infoflag ==SipSuccess) \
			|| (tbuf->dDuration>=SIP_T2))
		tbuf->dDuration = SIP_T2; 

	/* INVITE needs to timeout after seven not 11 */
	if (!strncmp(outkey->pMethod,"INVITE",6)) 
		retrans = SIP_MAXINVRETRANS;
	else 
		retrans = SIP_MAXRETRANS;

	if (tbuf->dRetransCount < retrans)
	{
		/*tbuf->dRetransCount++;*/
		SipBool res;
		res = fast_startTimer( tbuf->dDuration , 0, sip_processTimeOut,\
				(SIP_Pvoid) tbuf, outkey, err);
		if(res == SipFail)
		{
			sip_freeSipTimerBuffer(tbuf);
			sip_freeSipTimerKey(outkey);
		}
	}
	else
	{
		/* Maximum Retransmissions have been reached, 
		   so delete space returned by fast_stoptimer */
		sip_freeSipTimerBuffer(tbuf);
		sip_freeSipTimerKey(outkey);
	}
	return SipSuccess;
}

/************************************************************************
 ** FUNCTION:sip_processTimeOut
 **
 ** DESCRIPTION: This function is invoked by the timer manager
 ** when a Time out occurs. It is passed the TimerKey and the TimerBuffer
 ** of the timeout Context
 **
 *************************************************************************/

	SipBool sip_processTimeOut
#ifdef ANSI_PROTO
(SipTimerKey *pKey, SIP_Pvoid pBuffer)
#else
(pKey, pBuffer)
	SipTimerKey *pKey; 
	SIP_Pvoid pBuffer;
#endif

{
	SipTimerBuffer *tbuf;
	SipError err;
	SIP_U16bit retrans;
	SIP_U32bit dRetransT2;
#ifdef SIP_TRACE
	SIP_S8bit tracebuf[SIP_TRACE_BUF_SIZE];
#endif
	SIP_U32bit olddur ;
#ifdef SIP_MIB
	SIP_U16bit pTempCodeNum;
	SIP_S8bit* pTempMethod;
#endif

	/* Retrieve previous retransmission interval and double it */
	tbuf = (SipTimerBuffer *) pBuffer;
	olddur = tbuf->dDuration;
	tbuf->dDuration = tbuf->dDuration * 2;

	/* If user has specified own T2 value, use that for the comparisons that
	   follow */
	if(tbuf->dRetransT2 != 0)
		dRetransT2 = tbuf->dRetransT2;
	else
		dRetransT2 = SIP_T2;

	/* If non-INVITE request, cap retransmission interval to T2 if new 
       retrans exceeds T2 */

	if (((tbuf->dInfoReceived == SipSuccess) || (tbuf->dDuration>=dRetransT2))\
			&& (pKey->dMatchType != SipMessageResponse) &&\
			(strcmp(pKey->pMethod, "INVITE")))
		tbuf->dDuration = dRetransT2; 

	/* Get the max retrans Value. Different for INVITE and responses */
	if(tbuf->dMaxInviteRetransCount == SIP_MAXU16BIT &&\
			tbuf->dMaxRetransCount == SIP_MAXU16BIT)
	{
		retrans = (strncmp((SIP_S8bit *)tbuf->pBuffer,"INVITE", 6)) ? \
			SIP_MAXRETRANS:SIP_MAXINVRETRANS;
		if(pKey->dMatchType == SipMessageResponse) 
			retrans = SIP_MAXINVRETRANS;
	}	
	else
	{
		retrans = (strncmp((SIP_S8bit *)tbuf->pBuffer,"INVITE", 6)) ? \
			tbuf->dMaxRetransCount:tbuf->dMaxInviteRetransCount;
		if(pKey->dMatchType == SipMessageResponse) 
			retrans = tbuf->dMaxInviteRetransCount;
	}

	if (tbuf->dRetransCount < retrans)
	{
		/* Retransmission count has not yet exceeded the limit */
#ifdef SIP_TRACE
		char msg[SIP_FIRST_20_BYTES];
#endif
		if(tbuf->pContext != SIP_NULL)
			tbuf->dAddr.pData = tbuf->pContext->pData;
		else
			tbuf->dAddr.pData = SIP_NULL;

		if (sip_sendToNetwork((SIP_S8bit *)(tbuf->pBuffer), \
					tbuf->dBufferLength, &tbuf->dAddr, \
					tbuf->dTranspType, &err) == SipFail)
		{
			sip_error (SIP_Major, "SIP_ERROR: Cannot send to network");
			INC_ERROR_NETWORK
		}

		/* Update response/request sent statistics */
		if(pKey->dMatchType == SipMessageResponse)
		{
			INC_API_RESP_SENT
#ifdef SIP_MIB
				pTempCodeNum = pKey->dCodeNum;
			if ( pTempCodeNum >= 100 && pTempCodeNum < 200)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_1XX)
			else if ( pTempCodeNum >= 200 && pTempCodeNum < 300)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_2XX)
			else if ( pTempCodeNum >= 300 && pTempCodeNum < 400)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_3XX)
			else if ( pTempCodeNum >= 400 && pTempCodeNum < 500)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_4XX)
			else if ( pTempCodeNum >= 500 && pTempCodeNum < 600)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_5XX)
			else if ( pTempCodeNum >= 600 && pTempCodeNum < 700)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_6XX)
			else if ( pTempCodeNum >= 700 && pTempCodeNum < 800)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_7XX)
			else if ( pTempCodeNum >= 800 && pTempCodeNum < 900)
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_8XX)
			else if ( pTempCodeNum >= 900 && pTempCodeNum < 1000)
			{
				INC_API_RESPCLASS_PARSED_RETRY(SIP_STAT_RESPCLASS_9XX)
			}

			if ( pTempCodeNum < NUM_STAT_RESPONSECODE )
			{
				INC_API_RESPCODE_PARSED_RETRY(pTempCodeNum)
			}

#endif
		}
		else if(pKey->dMatchType == SipMessageRequest)
		{
			INC_API_REQ_SENT
#ifdef SIP_MIB
				pTempMethod = pKey->pMethod;
			if ( __sip_incrementStats(pTempMethod,SIP_STAT_API_REQUEST,\
						SIP_STAT_RETRY) != SipSuccess )
			{
				INC_API_UNKNOWN_PARSED_RETRY
			}
#endif
		}

#ifdef SIP_TRACE
		strncpy (msg, (SIP_S8bit *)tbuf->pBuffer, 20);
		msg[20]='\0';
		sip_trace(SIP_Brief,SIP_Outgoing,(SIP_S8bit *) "Message sent out.");
		sip_trace(SIP_Detailed,SIP_Outgoing,(SIP_S8bit *)tbuf->pBuffer);
		HSS_SPRINTF (tracebuf,"Timer Value: %d\nMax transmissions: %d\nMsg"\
				"(1st 20 bytes) : \"%s\" \n",olddur, retrans+1,msg);
		sip_trace(SIP_Brief, SIP_Outgoing, tracebuf);
#endif
		tbuf->dRetransCount++;
#ifdef SIP_RETRANSCALLBACK
		if(tbuf->enableRetransCallback == SipSuccess)
		{
			sip_indicateMessageRetransmission\
				(tbuf->pContext, pKey, tbuf->pBuffer, tbuf->dBufferLength,
				 &(tbuf->dAddr), tbuf->dRetransCount, olddur);
		}
#endif
		if(SipFail==fast_startTimer( tbuf->dDuration , 0, sip_processTimeOut, \
					(SIP_Pvoid) tbuf, pKey, &err))
		{
			sip_freeSipTimerBuffer(tbuf);
			sip_freeSipTimerKey(pKey);
		}
	}
	else
	{
		/* Maximum Retransmissions have been reached, 
		   so free space returned by fast_stoptimer */
		SipEventContext *tempcontext;
		tempcontext = tbuf->pContext;
		tbuf->pContext = SIP_NULL;
		if(tempcontext != SIP_NULL)
		{
			if((tempcontext->dOptions.dOption & SIP_OPT_DIRECTBUFFER) == \
					SIP_OPT_DIRECTBUFFER)
			{
				tbuf->pBuffer= SIP_NULL;
			}
		}
		sip_freeSipTimerBuffer(tbuf);
		sip_freeSipTimerKey(pKey);
		sip_indicateTimeOut(tempcontext);
	}
	return SipSuccess;
}

