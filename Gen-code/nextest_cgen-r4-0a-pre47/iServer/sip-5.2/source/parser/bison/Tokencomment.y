/***********************************************************************
 ** FUNCTION:
 **             Yacc file for Token comment  headers

 *********************************************************************
 **
 ** FILENAME:
 ** Tokencomment.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form token comment
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

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

%token SERVER USERAGENT
%token TOKEN COMMENT SLASH TOKENLWS
%token DEFAULT COMMENTLWS
%%
header:	server
		|useragent
		|error
		{
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				=E_PARSER_ERROR;

		};
server:	servercolon serverfields
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		if(sip_initSipServerHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pResponse->pResponseHdr->pServerHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->u.pResponse->pResponseHdr->pServerHdr->pValue=\
		sip_tokenBufferDup($2);
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pResponse->pResponseHdr->pServerHdr->\
			pValue==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeServer;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
useragent:	useragentcolon agentfields
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		if(sip_initSipUserAgentHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pUserAgentHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
		pGeneralHdr->pUserAgentHdr->pValue = sip_tokenBufferDup($2);
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pUserAgentHdr->pValue==SIP_NULL)
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
		order->dType = SipHdrTypeUserAgent;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
serverfields:serverfields field
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$2.dLength+2,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *)\
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		/* this was adding extra spaces
		$$.pToken[$1.dLength] = ' ';
		strncpy(&($$.pToken[$1.dLength+1]), $2.pToken, $2.dLength);
        */
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	}
		|field;	
agentfields:agentfields field
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$2.dLength+2,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *)\
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		/* this was adding extra spaces 
		$$.pToken[$1.dLength] = ' ';
		strncpy(&($$.pToken[$1.dLength+1]), $2.pToken, $2.dLength);
        */
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	}
		|field;	
field:	product 
		|comment; 
comment :  COMMENT 
        | COMMENTLWS;
product:	TOKEN SLASH TOKEN
	{
       
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$3.dLength+2,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength] = '/';
		strncpy(&($$.pToken[$1.dLength+1]), $3.pToken, $3.dLength);
		$$.dLength = $1.dLength+$3.dLength+1;
	}
    | TOKEN SLASH TOKENLWS
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$3.dLength+2,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength] = '/';
		strncpy(&($$.pToken[$1.dLength+1]), $3.pToken, $3.dLength);
		$$.dLength = $1.dLength+$3.dLength+1;
    }
			|TOKEN
			|TOKENLWS ;
servercolon:SERVER
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageResponse)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_ERROR;
			YYABORT;
		}
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			u.pResponse->pResponseHdr->pServerHdr != SIP_NULL)
		{
			sip_error (SIP_Minor,"There can only be one Server Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};
useragentcolon:	USERAGENT
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pUserAgentHdr != SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one User Agent Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
	};
