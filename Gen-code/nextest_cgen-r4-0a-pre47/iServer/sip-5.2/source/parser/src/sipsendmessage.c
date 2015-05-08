/***********************************************************************
 ** FUNCTION:
 **            Send module
 *********************************************************************
 **
 ** FILENAME:
 ** sipsendmessage.c
 **
 ** DESCRIPTION:
 **		This file contains the logic for parsing SIP messages.
 ** 	It invokes a number of bison generated parsers. It also contains
 ** 	the logic for indicate invocation.
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 12/01/2000  Binu K S / Arjun RC 			 			Initial creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

#include "sipcommon.h"
#include "sipstruct.h"
#include "sipsendmessage.h"
#include "portlayer.h"
#include "sipstatistics.h"
#include "siptimer.h"
#include "siptrace.h"
#include "sipdecode.h"
#include "sipinit.h"
#include "sipparserclone.h"


/*****************************************************************************
 ** FUNCTION: sip_sendMessage
 *****************************************************************************
 **
 ** Forms SIP Text message from message structure and sends to network.
 **
 ****************************************************************************/

SipBool sip_sendMessage
#ifdef ANSI_PROTO
(
 SipMessage *s,
 SipOptions *options,
 SipTranspAddr *dAddr,
 SIP_S8bit dTranspType,
 SipEventContext *pContext,
 SipError *err
)
#else
(s, options, dAddr, dTranspType, pContext, err)
	SipMessage *s;
	SipOptions *options;
	SipTranspAddr *dAddr;
	SIP_S8bit dTranspType;
	SipEventContext *pContext;
	SipError *err;
#endif
{
	SIP_S8bit *out;
	SIP_U32bit dLength;

#ifndef SIP_TXN_LAYER	
	SIP_U32bit dRetransT1,count;
#endif	

#ifdef SIP_MIB
	SIP_S8bit* pTempMethod = SIP_NULL;
#ifndef SIP_TXN_LAYER	
	SipConfigStats *pConfig = SIP_NULL;
#endif
#endif

	if(s == SIP_NULL)
	{
		sip_error (SIP_Major, "SIP_ERROR: SIP Message is empty");
		INC_ERROR_NETWORK
			return SipFail;
	}

	/* Update statistics */
	INC_API_COUNT
		if(pContext != SIP_NULL)
		{
			pContext->dOptions.dOption = options->dOption;
		}
	if((options->dOption & SIP_OPT_DIRECTBUFFER) == SIP_OPT_DIRECTBUFFER)
	{
		if(pContext == SIP_NULL)
		{
			sip_error (SIP_Major, "SIP_ERROR: Event context is empty");
			INC_ERROR_NETWORK
				return SipFail;
		}
		if(pContext->pDirectBuffer == SIP_NULL)
		{
			sip_error (SIP_Major,"SIP_ERROR: Buffer in event context is empty");
			INC_ERROR_NETWORK
				return SipFail;
		}
		out = (SIP_S8bit*) pContext->pDirectBuffer;
		dLength = strlen(out);
	}
	else
	{
		out = (SIP_S8bit *) fast_memget(NETWORK_MEM_ID,SIP_MAX_MSG_SIZE,err);
		if(out == SIP_NULL)
			return SipFail;
		if(sip_formMessage( s, options, out, &dLength, err) == SipFail)
		{
			fast_memfree(NETWORK_MEM_ID,out,NULL);
			DEC_API_COUNT
				return SipFail;
		}
		/*Sipformmessage called internally,so do not increment User API count*/
		DEC_API_COUNT
	}
	if(pContext != SIP_NULL)
		dAddr->pData = pContext->pData;
	else
		dAddr->pData = SIP_NULL;

	if(sip_sendToNetwork(out, dLength, dAddr, dTranspType, err) == SipFail)
	{
		if((options->dOption & SIP_OPT_DIRECTBUFFER) != SIP_OPT_DIRECTBUFFER)
		{
			fast_memfree(NETWORK_MEM_ID,out,NULL);
		}
		sip_error (SIP_Major, "SIP_ERROR: Could not send to network ");
		INC_ERROR_NETWORK
		return SipFail;
	}
	/* Update response/request sent statistics */
	if(s->dType == SipMessageResponse)
	{
#ifdef SIP_MIB
		SIP_U16bit pTempCodeNum;
#endif		
		INC_API_RESP_SENT
#ifdef SIP_MIB
			pTempCodeNum = s->u.pResponse->pStatusLine->dCodeNum;
		if ( pTempCodeNum >= 100 && pTempCodeNum < 200)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_1XX)
		else if ( pTempCodeNum >= 200 && pTempCodeNum < 300)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_2XX)
		else if ( pTempCodeNum >= 300 && pTempCodeNum < 400)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_3XX)
		else if ( pTempCodeNum >= 400 && pTempCodeNum < 500)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_4XX)
		else if ( pTempCodeNum >= 500 && pTempCodeNum < 600)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_5XX)
		else if ( pTempCodeNum >= 600 && pTempCodeNum < 700)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_6XX)
		else if ( pTempCodeNum >= 700 && pTempCodeNum < 800)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_7XX)
		else if ( pTempCodeNum >= 800 && pTempCodeNum < 900)
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_8XX)
		else if ( pTempCodeNum >= 900 && pTempCodeNum < 1000)
		{
			INC_API_RESPCLASS_PARSED_OUT(SIP_STAT_RESPCLASS_9XX)
		}

		if ( pTempCodeNum < NUM_STAT_RESPONSECODE )
		{
			INC_API_RESPCODE_PARSED_OUT(pTempCodeNum)
				/*Also we need to send the MIB Related callback
				  for sending  response*/
#ifdef SIP_THREAD_SAFE
			fast_lock_synch(0,&glbLockStatisticsMutex,\
					FAST_MXLOCK_EXCLUSIVE ); 
#endif			

			/*Did the application register for receiving callbacks??*/
			if (glbSipParserResponseCodeParserStats[pTempCodeNum].\
					dGiveCallback ==SipSuccess)
			{	
				/*
				 * Throw the cbk indicating the sending of a response
				 * message of this type. Also add a reference to the
				 * actual message that caused this cbk to be invoked
				 */

				/*First increment the refcount of the SipMessage*/
				HSS_INCREF(s->dRefCount);

				sip_indicateResponseHandled(s, \
						SipCallbkForResponseSent, pTempCodeNum,\
						glbSipParserResponseCodeParserStats[pTempCodeNum]);
			}	
#ifdef SIP_THREAD_SAFE			
			fast_unlock_synch(0,&glbLockStatisticsMutex );
#endif			
		}
#endif
	}
	else if(s->dType == SipMessageRequest)
	{
		INC_API_REQ_SENT
#ifdef SIP_MIB
			pTempMethod = s->u.pRequest->pRequestLine->pMethod;
		if ( __sip_incrementStats(pTempMethod,SIP_STAT_API_REQUEST,\
					SIP_STAT_OUT) != SipSuccess )
		{
			INC_API_UNKNOWN_PARSED_OUT
		}
#endif
	}

#ifdef SIP_TRACE
	sip_trace(SIP_Brief,SIP_Outgoing,(SIP_S8bit *) "Message sent out.");
	sip_trace(SIP_Detailed,SIP_Outgoing,out);
#endif
	/* Retransmission timer logic */
#ifndef SIP_TXN_LAYER	
	if((s->dType == SipMessageRequest)&&((dTranspType&SIP_UDP)==SIP_UDP)\
			&&((dTranspType&SIP_NORETRANS)!=SIP_NORETRANS))
	{
		/* Message is a request sent by UDP and NO_RETRANS Option not used */
		if(strcmp(s->u.pRequest->pRequestLine->pMethod,"ACK")!=0)
		{
			/* All requests except ACK  are retransmitted */
			SipTimerBuffer *tbuf;
			SipTimerKey *pKey;

			/* Timer Buffer which stores the message Text and other stuff */
			if(sip_initSipTimerBuffer(&tbuf, err)==SipFail)
			{
				return SipFail;
			}
			/* Timer Key that'll identify this message in the timer list*/
			if(sip_initSipTimerKey(&pKey, err)!=SipSuccess)
				return SipFail;
			/* initial count */
			tbuf->dRetransCount = 0;
			tbuf->enableRetransCallback = SipFail;
#ifdef SIP_RETRANSCALLBACK
			if((options->dOption & SIP_OPT_RETRANSCALLBACK) \
					== SIP_OPT_RETRANSCALLBACK)
			{
				tbuf->enableRetransCallback = SipSuccess;
			}
#endif
			/* initial retransmission interval */
			if(((options->dOption & SIP_OPT_PERMSGRETRANS) == \
						SIP_OPT_PERMSGRETRANS) && (pContext != SIP_NULL))
			{
				/* User has specified retransmission intervals to be used for
				   this message. Use that for the initial interval and store
				   these values in the timer buffer for calculating
				   subsequent intervals. */
				tbuf->dRetransT2 = pContext->dRetransT2;
				tbuf->dDuration = pContext->dRetransT1;
				dRetransT1 = pContext->dRetransT1;
				if((options->dOption & SIP_OPT_PERMSGRETRANSCOUNT) ==\
						SIP_OPT_PERMSGRETRANSCOUNT)
				{
					tbuf->dMaxRetransCount = pContext->dMaxRetransCount;
					tbuf->dMaxInviteRetransCount = pContext->\
						dMaxInviteRetransCount;
				}
				else
				{
					tbuf->dMaxRetransCount = SIP_MAXU16BIT;
					tbuf->dMaxInviteRetransCount = SIP_MAXU16BIT;
				}
			}
			else
			{
				/* User wants to use default timer values.
				   Set initial duration to default value.
				   Set intervals in timer-buffer to 0 so that default values
				   are used for subsequent interval calculations. */
				dRetransT1 = SIP_T1;
				tbuf->dDuration = SIP_T1;
				tbuf->dRetransT2 = 0;
				tbuf->dMaxRetransCount = SIP_MAXU16BIT;
				tbuf->dMaxInviteRetransCount = SIP_MAXU16BIT;
#ifdef SIP_MIB
				/* Get the configuration for this method */
				if( SipSuccess == __sip_getConfigStatsForMethod\
						(pTempMethod,&pConfig,err))
				{
					/* check if the timer values are configured*/
					if(pConfig != SIP_NULL)
					{
						dRetransT1 = pConfig->dRetransT1;
						tbuf->dDuration = pConfig->dRetransT1;
						if(strcasecmp(pTempMethod,"INVITE") ==0)
							tbuf->dMaxInviteRetransCount = pConfig->\
								dMaxRetransCount;
						else
							tbuf->dMaxRetransCount = pConfig->\
								dMaxRetransCount;
					}
				}	
#endif				
			}
			/* Flag to be set when informational response is received
			   Retrans caps off for some reqeusts after informational Response*/
			tbuf->dInfoReceived = SipFail;
			/* Address and transport Type */
			tbuf->dAddr = *dAddr;
			tbuf->dTranspType = dTranspType;
			/* Message Text and Context provided by User */
			tbuf->pBuffer = out;
			tbuf->dBufferLength = dLength;
			tbuf->pContext = pContext;

			/* Timer pKey stuff - Callid, Method, Cseq */
			pKey->dMatchFlag = TIMER_NONE;
			pKey->dCallid = STRDUPNETWORK(s->pGeneralHdr->pCallidHdr->pValue);
			if(pKey->dCallid == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			pKey->pMethod = STRDUPNETWORK(s->u.pRequest->pRequestLine->pMethod);
			if(pKey->pMethod == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			pKey->dCseq = s->pGeneralHdr->pCseqHdr->dSeqNum;
			pKey->dMatchType = SipMessageRequest;
			pKey->pRackHdr = SIP_NULL;
			/* Call leg from and to addresses */
			if(sip_initSipFromHeader(&(pKey->pFromHdr),err)==SipFail)
				return SipFail;
			if(__sipParser_cloneSipFromHeader(pKey->pFromHdr,\
						s->pGeneralHdr->pFromHeader, err) == SipFail)
				return SipFail;
			if(sip_initSipToHeader(&(pKey->pToHdr),err)==SipFail)
				return SipFail;
			if(__sipParser_cloneSipToHeader(pKey->pToHdr,\
						s->pGeneralHdr->pToHdr,err) == SipFail)
				return SipFail;
			/* Add branch from first Via Header */
			pKey->pViaBranch = SIP_NULL;
			/* Check if Via header present */
			if(sip_listSizeOf(&(s->pGeneralHdr->slViaHdr), &count, err)\
					!=SipFail)
			{
				if(count!=0)
				{
					SipViaHeader *pViaHeader;

					/* Get first via header from the list */
					if(sip_listGetAt(&(s->pGeneralHdr->slViaHdr), 0, \
								(SIP_Pvoid *) &pViaHeader , err) != SipFail)
					{
						SIP_S8bit *pViaBranch;
						/* Get Branch parameter */
						if(sip_getBranchFromViaHdr(pViaHeader, &pViaBranch,\
									err) !=SipFail)
						{
							pKey->pViaBranch = STRDUPNETWORK(pViaBranch);
							if(pKey->pViaBranch == SIP_NULL)
							{
								*err = E_NO_MEM;
								return SipFail;
							}
						}
					}
				}
			}


			/* Start the timer now - implementation in User space
			   sip_processTimeOut(siptimer.c) will handle
			   further retransmissions */
			if(SipFail == fast_startTimer( dRetransT1, 0, sip_processTimeOut, \
						(SIP_Pvoid) tbuf, pKey, err))
			{
				sip_freeSipTimerBuffer(tbuf);
				sip_freeSipTimerKey(pKey);
				return SipFail;
			}
		}
		else
		{
			if(pContext != SIP_NULL)
				sip_freeEventContext(pContext);
			if((options->dOption & SIP_OPT_DIRECTBUFFER)\
					!= SIP_OPT_DIRECTBUFFER)
			{
				fast_memfree(NETWORK_MEM_ID,out,NULL);
			}
		}
	}
	else if((s->dType == SipMessageResponse) && ((dTranspType&SIP_NORETRANS)\
				!=SIP_NORETRANS) && (s->u.pResponse->pStatusLine->dCodeNum != \
					401) && (s->u.pResponse->pStatusLine->dCodeNum != 407))
	{
		/* Response - retransmitted over UDP and TCP */
		if(((strcmp(s->pGeneralHdr->pCseqHdr->pMethod,"INVITE")==0)\
					&&(s->u.pResponse->pStatusLine->dCodeNum >= 200))\
				||((s->u.pResponse->pResponseHdr->pRSeqHdr!=SIP_NULL)\
					&&(s->u.pResponse->pStatusLine->dCodeNum>100)\
					&&(s->u.pResponse->pStatusLine->dCodeNum<200)))
		{
			/* All responses to INVITE with CodeNum >=200 are retransmitted
			   Responses with CodeNum between 100 and 200 containing RSeq
			   Header are also retransmitted */
			SipTimerBuffer *tbuf;
			SipTimerKey *pKey;

			if(sip_initSipTimerBuffer(&tbuf, err)==SipFail)
				return SipFail;
			if(sip_initSipTimerKey(&pKey, err)!=SipSuccess)
				return SipFail;
			/* Buffer and Key formation */
			tbuf->dRetransCount = 0;
			tbuf->enableRetransCallback = SipFail;
#ifdef SIP_RETRANSCALLBACK
			if((options->dOption & SIP_OPT_RETRANSCALLBACK) \
					== SIP_OPT_RETRANSCALLBACK)
			{
				tbuf->enableRetransCallback = SipSuccess;
			}
#endif
			if(((options->dOption & SIP_OPT_PERMSGRETRANS) == \
						SIP_OPT_PERMSGRETRANS) && (pContext != SIP_NULL))
			{
				/* User has specified retransmission intervals to be used for
				   this message. Use that for the initial interval and store
				   these values in the timer buffer for calculating
				   subsequent intervals. */
				tbuf->dRetransT2 = pContext->dRetransT2;
				tbuf->dDuration = pContext->dRetransT1;
				dRetransT1 = pContext->dRetransT1;
				if((options->dOption & SIP_OPT_PERMSGRETRANSCOUNT) ==\
						SIP_OPT_PERMSGRETRANSCOUNT)
				{
					tbuf->dMaxRetransCount = pContext->dMaxRetransCount;
					tbuf->dMaxInviteRetransCount = pContext->\
						dMaxInviteRetransCount;
				}
				else
				{
					tbuf->dMaxRetransCount = SIP_MAXU16BIT;
					tbuf->dMaxInviteRetransCount = SIP_MAXU16BIT;
				}
			}
			else
			{
				/* User wants to use default timer values.
				   Set initial duration to default value.
				   Set intervals in timer-buffer to 0 so that default values
				   are used for subsequent interval calculations. */
				tbuf->dDuration = SIP_T1;
				tbuf->dRetransT2 = 0;
				dRetransT1 = SIP_T1;
				tbuf->dMaxRetransCount = SIP_MAXU16BIT;
				tbuf->dMaxInviteRetransCount = SIP_MAXU16BIT;

#ifdef SIP_MIB
				__sip_getConfigStatsForResponse(&pConfig);
				if(pConfig!= SIP_NULL)
				{
					dRetransT1 = pConfig->dRetransT1;
					tbuf->dDuration = pConfig->dRetransT1;
					tbuf->dMaxInviteRetransCount = pConfig->\
						dMaxRetransCount;
					tbuf->dMaxRetransCount = pConfig->\
						dMaxRetransCount;
				}
#endif
			}
			tbuf->dInfoReceived = SipFail;
			tbuf->dAddr = *dAddr;
			tbuf->dTranspType = dTranspType;
			tbuf->pBuffer = out;
			tbuf->dBufferLength = dLength;
			tbuf->pContext = pContext;

			pKey->dCallid = STRDUPNETWORK(s->pGeneralHdr->pCallidHdr->pValue);
			if(pKey->dCallid == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			pKey->pMethod = STRDUPNETWORK(s->pGeneralHdr->pCseqHdr->pMethod);
			if(pKey->pMethod == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			pKey->dCseq = s->pGeneralHdr->pCseqHdr->dSeqNum;
			pKey->dMatchType = SipMessageResponse;
			pKey->dCodeNum = s->u.pResponse->pStatusLine->dCodeNum;
			if(pKey->dCodeNum < 200)
				pKey->dRseq = s->u.pResponse->pResponseHdr->pRSeqHdr->dRespNum;
			pKey->pRackHdr = SIP_NULL;
			if(sip_initSipFromHeader(&(pKey->pFromHdr),err)==SipFail)
				return SipFail;
			if(__sipParser_cloneSipFromHeader(pKey->pFromHdr,\
						s->pGeneralHdr->pFromHeader,\
						err) == SipFail)
				return SipFail;
			if(sip_initSipToHeader(&(pKey->pToHdr),err)==SipFail)
				return SipFail;
			if(__sipParser_cloneSipToHeader(pKey->pToHdr,\
						s->pGeneralHdr->pToHdr,err)\
					== SipFail)
				return SipFail;
			/* Add branch from first Via Header */
			pKey->pViaBranch = SIP_NULL;
			/* Start the timer now - implementation in User space
			   sip_processTimeOut(siptimer.c) will handle
			   further retransmissions */
			if(SipFail == fast_startTimer( dRetransT1, 0, sip_processTimeOut, \
						(SIP_Pvoid) tbuf, pKey, err))
			{
				sip_freeSipTimerBuffer(tbuf);
				sip_freeSipTimerKey(pKey);
				return SipFail;
			}
		}
		else
		{
			if(pContext != SIP_NULL)
				sip_freeEventContext(pContext);
			if((options->dOption & SIP_OPT_DIRECTBUFFER) != \
						SIP_OPT_DIRECTBUFFER)
			{
				fast_memfree(NETWORK_MEM_ID,out,NULL);
			}
		}
	}
	else
	{
		if(pContext != SIP_NULL)
			sip_freeEventContext(pContext);
		if((options->dOption & SIP_OPT_DIRECTBUFFER) != SIP_OPT_DIRECTBUFFER)
		{
			fast_memfree(NETWORK_MEM_ID,out,NULL);
		}
	}
#endif	
	*err = E_NO_ERROR;
	return SipSuccess;
}
