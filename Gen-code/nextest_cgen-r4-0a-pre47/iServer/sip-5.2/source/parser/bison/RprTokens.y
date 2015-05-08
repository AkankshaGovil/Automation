/***********************************************************************
 ** FUNCTION:
 **		Yacc file for rseq and rack headers

 *********************************************************************
 **
 ** FILENAME:
 ** RprTokens.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form rseq, rack
 **
 ** DATE	NAME			REFERENCE		REASON
 ** ----	----			---------	       --------
 ** 24/11/99  KSBinu, Arjun RC				 Initial Creation
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token TOKEN DIGITS RSEQ RACK
%token DEFAULT
%pure_parser
%{ 
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "rprinit.h"
#include "siplist.h"
#include "siptrace.h"

#define MAXTEMP (300)

#define YYPARSE_PARAM pHeaderParserParam
%}
%%

header: rseq
		|rack
		|error
		{
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				=E_PARSER_ERROR;

		};
rseq:	RSEQ digits
		{
			SIP_S8bit *temp;
			SipHeaderOrderInfo *order;
			SipBool dResult;
			if(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->dType != SipMessageResponse)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					= E_NO_ERROR;
				YYABORT;
			}
			if(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pResponse->pResponseHdr->pRSeqHdr \
				!= SIP_NULL)
			{
				sip_error (SIP_Minor,\
					"There can only be one RSeq Header");
				*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_PARSER_ERROR;
				YYABORT;
			}
			if(sip_rpr_initSipRSeqHeader(&(((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->\
				u.pResponse->pResponseHdr->pRSeqHdr), \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))==SipFail)
				YYABORT;
			temp = sip_tokenBufferDup($2);
			((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pResponse->pResponseHdr->pRSeqHdr->\
			dRespNum  = (SIP_U32bit) STRTOU32CAP(temp, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError));
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			/* The value of the rseq num should be as 0 <= rseq num < 2^32 */
			if ((*(((SipHeaderParserParam *) pHeaderParserParam)->pError) ==\
				E_INV_RANGE))
			{
				   sip_error (SIP_Minor,"Resp Number out of Range");
			       *(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				   E_PARSER_ERROR;
			       YYABORT;
			}

			dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
			if (dResult!=SipSuccess)
			{
				YYABORT;
			}
			order->dType = SipHdrTypeRSeq;
			order->dTextType = SipFormFull;
			order->dNum = 1;
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->slOrderInfo, \
				(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		};
rack:	rackcolon digits digits token
		{
			SIP_S8bit *temp;
			SipHeaderOrderInfo *order;
			SipBool dResult;
		
			if(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->pRackHdr!= SIP_NULL)
			{
				sip_error (SIP_Minor, "There can only be one Rack header");
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_PARSER_ERROR;
				YYABORT;
			}
			if(sip_rpr_initSipRAckHeader(&(((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
				pRackHdr),(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))==SipFail)
				YYABORT;
			temp = sip_tokenBufferDup($2);
			((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->pRackHdr->\
				dRespNum = (SIP_U32bit) STRTOU32CAP(temp, \
					(((SipHeaderParserParam *)pHeaderParserParam)->pError));
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			/* check reqd here for overflow. */
			if ((*(((SipHeaderParserParam *) pHeaderParserParam)->pError) ==\
				E_INV_RANGE))
			{
				   sip_error (SIP_Minor,"Rack Header Resp Number out of Range");
			       *(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				   E_PARSER_ERROR;
			       YYABORT;
			}

			temp = sip_tokenBufferDup($3);
			((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->pRackHdr->\
				dCseqNum = (SIP_U32bit) STRTOU32CAP(temp, \
					(((SipHeaderParserParam *)pHeaderParserParam)->pError));
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			/* check reqd here for overflow. */
			if ((*(((SipHeaderParserParam *) pHeaderParserParam)->pError) ==\
				E_INV_RANGE))
			{
				   sip_error (SIP_Minor,"Rack Header Cseq Number out of Range");
			       *(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				   E_PARSER_ERROR;
			       YYABORT;
			}
		
			temp = sip_tokenBufferDup($4);
			((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->pRackHdr->\
				pMethod = STRDUPBISON(temp);
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			if(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->pRackHdr->\
				pMethod == SIP_NULL)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
				YYABORT;
			}
			dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
			if (dResult!=SipSuccess)
			{
				YYABORT;
			}
			order->dType = SipHdrTypeRAck;
			order->dTextType = SipFormFull;
			order->dNum = 1;
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->slOrderInfo,(\
				SIP_Pvoid)order,(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		};
digits: DIGITS;
token:	DIGITS
		|TOKEN;
rackcolon:	RACK
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_ERROR;
			YYABORT;
		}
	};
