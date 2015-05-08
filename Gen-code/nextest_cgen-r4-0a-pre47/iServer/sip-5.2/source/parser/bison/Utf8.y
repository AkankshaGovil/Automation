/***********************************************************************
 ** FUNCTION:
 **             Yacc file for utf8 headers

 *********************************************************************
 **
 ** FILENAME:
 ** utf8.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form utf8
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token SUBJECT ORGANIZATION
%token TEXTUTF8NOCOLON COLON
%token DEFAULT
%pure_parser
%{ 
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siptrace.h"

#define MAXTEMP (300)

#define YYPARSE_PARAM pHeaderParserParam
%}
%%
header:subject
		|organization
		|error
		{
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError)==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError)=E_PARSER_ERROR;

		};
subject:	subjectcolon textutf8trim
	{
	 	SipSubjectHeader *subhdr;
		SipHeaderOrderInfo *order; /* new line addition -- suman*/
		SipBool dResult;
		if(sip_initSipSubjectHeader(&subhdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		subhdr->pSubject = sip_tokenBufferDup($2);
		if(subhdr->pSubject==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		 ((SipHeaderParserParam *) pHeaderParserParam)->\
		 pSipMessage->u.pRequest->pRequestHdr->pSubjectHdr = subhdr;
		/* new addition -- suman */
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType = SipHdrTypeSubject;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		/* end of new adition -- suman */
	}
	|subjectcolon
	{
		/* commented by suman */
		/* SipSubjectHeader *subhdr;

		if(sip_initSipSubjectHeader(&subhdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		subhdr->pSubject = SIP_NULL;
		((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->u.pRequest->pRequestHdr->pSubjectHdr = subhdr;*/
	};
organization:organizationcolon textutf8trim
	{
		SipOrganizationHeader *orghdr;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		if(sip_initSipOrganizationHeader(&orghdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		orghdr->pOrganization = sip_tokenBufferDup($2);
		if(orghdr->pOrganization==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr->pOrganizationHdr = orghdr;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeOrganization;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
		|organizationcolon
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,((SipHeaderParserParam *)\
			pHeaderParserParam)->pError);
		if (dResult==SipFail) YYABORT;
		
		order->dType = SipHdrTypeOrganization;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT; 
	};
textutf8trim: textutf8trim textutf8atom
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) \
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	}
	|textutf8atom;
textutf8atom: TEXTUTF8NOCOLON
		|COLON;
subjectcolon:	SUBJECT
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pSubjectHdr != SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one Subject Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}
		/* order = (SipHeaderOrderInfo *)fast_memget(BISON_MEM_ID,\
		sizeof(SipHeaderOrderInfo),(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError));
		if(order == SIP_NULL)
			YYABORT;
		order->dType = SipHdrTypeSubject;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT; */
	};
organizationcolon:ORGANIZATION
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pOrganizationHdr!=SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one Organization Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
	};
