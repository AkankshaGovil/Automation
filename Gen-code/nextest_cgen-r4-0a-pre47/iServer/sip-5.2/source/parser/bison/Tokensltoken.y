/***********************************************************************
 ** FUNCTION:
 **             Yacc file for Token/Token headers

 *********************************************************************
 **
 ** FILENAME:
 ** Tokensltoken.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form Token/Token
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token TOKEN SLASH SEMICOLON EQUALS COMMA AT A ATT 
%token QPARAM QSTRING COLON WORD_LEX
%token ACCEPT ACCEPTLANG CONTENTTYPE WARNING EVENT
%token LRANGE LRANGE_INREP DIGIT3 SIPFLOAT
%token MIMEVERSION
%token DEFAULT 
%token ALERTINFO URI INREPLYTO CALLIDAT AUTHINFO
%token CALLINFO ERRORINFO CONTENTDISPOSITION IPV6ADDR REASON
%pure_parser
%{ 
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include <ctype.h>
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipbcptinit.h"
#include "sipfree.h"
#include "siptrace.h"

#ifdef SIP_IMPP
#include "imppinit.h"
#endif

#define MAXTEMP (300)

static SipAuthenticationInfoHeader *glbSipAuthenticationInfo = SIP_NULL;
static SipAcceptHeader*	glbSipParserAcceptHeader =SIP_NULL;
static SipAcceptLangHeader* glbSipParserAcceptLangHeader=SIP_NULL; 
static SipAlertInfoHeader *glbSipParserAlertInfoHeader=SIP_NULL;
static SipInReplyToHeader* glbSipParserInReplyToHeader=SIP_NULL;
static SipCallInfoHeader *glbSipParserCallInfoHeader=SIP_NULL;
static SipErrorInfoHeader *glbSipParserErrorInfoHeader=SIP_NULL;
static SipContentDispositionHeader *glbSipParserContentDispositionHeader\
			=SIP_NULL;
static SipReasonHeader *glbSipParserReasonHeader=SIP_NULL;
#ifdef SIP_IMPP
static SipEventHeader *glbSipParserEventHeader=SIP_NULL;
static en_HeaderForm glbSipSupportedType;
#endif
static SipContentTypeHeader *glbSipParserContentTypeHeader=SIP_NULL;
static SipWarningHeader *glbSipParserWarnHdr = SIP_NULL; 
#define YYPARSE_PARAM pHeaderParserParam
%}
%%
header:	accept
		|acceptlanguage
		|contenttype
		|warning
		|mimeversion
		|alertinfo
		|inreplyto
		|callinfo
		|errorinfo
		|authenticationinfo
        |contentdisp
		|event
		|reason
		|error
		{
			if(glbSipAuthenticationInfo != SIP_NULL)
			{
				SIP_U32bit dSize=0;
				SipError err;
				if (sip_listSizeOf(&(glbSipAuthenticationInfo->slNameValue),\
					&dSize,&err) != SipFail )
				{
					if ( dSize == 0 )
					{
						sip_freeSipAuthenticationInfoHeader( \
							glbSipAuthenticationInfo);
						glbSipAuthenticationInfo= SIP_NULL;
					}
				}
			}
			if(glbSipParserAcceptHeader!= SIP_NULL)
			{
				sip_freeSipAcceptHeader(glbSipParserAcceptHeader);	
				glbSipParserAcceptHeader= SIP_NULL;
			}
			if(glbSipParserAlertInfoHeader!= SIP_NULL)
			{
				sip_freeSipAlertInfoHeader(glbSipParserAlertInfoHeader);	
				glbSipParserAlertInfoHeader= SIP_NULL;
			}
			if(glbSipParserCallInfoHeader!= SIP_NULL)
			{
				sip_freeSipCallInfoHeader(glbSipParserCallInfoHeader);	
				glbSipParserCallInfoHeader= SIP_NULL;
			}
			if(glbSipParserErrorInfoHeader!= SIP_NULL)
			{
				sip_freeSipErrorInfoHeader(glbSipParserErrorInfoHeader);	
				glbSipParserErrorInfoHeader= SIP_NULL;
			}
			if(glbSipParserContentDispositionHeader!= SIP_NULL)
			{
				sip_freeSipContentDispositionHeader(\
					glbSipParserContentDispositionHeader);	
				glbSipParserContentDispositionHeader= SIP_NULL;
			}
			if(glbSipParserReasonHeader!= SIP_NULL)
			{
				sip_freeSipReasonHeader(\
					glbSipParserReasonHeader);	
				glbSipParserReasonHeader= SIP_NULL;
			}
#ifdef SIP_IMPP
			if(glbSipParserEventHeader!= SIP_NULL)
			{
				sip_impp_freeSipEventHeader(glbSipParserEventHeader);	
				glbSipParserEventHeader= SIP_NULL;
			}
#endif
			if(glbSipParserAcceptLangHeader != SIP_NULL)
			{
				sip_freeSipAcceptLangHeader(glbSipParserAcceptLangHeader);
				glbSipParserAcceptLangHeader = SIP_NULL;
			}
			if(glbSipParserContentTypeHeader != SIP_NULL)
			{
				sip_freeSipContentTypeHeader(glbSipParserContentTypeHeader);
				glbSipParserContentTypeHeader = SIP_NULL;
			}
			if(glbSipParserWarnHdr != SIP_NULL)
			{
				sip_freeSipWarningHeader(glbSipParserWarnHdr);
				glbSipParserWarnHdr = SIP_NULL;
			}
			if(glbSipParserInReplyToHeader != SIP_NULL)
			{
				sip_freeSipInReplyToHeader(glbSipParserInReplyToHeader);
				glbSipParserInReplyToHeader = SIP_NULL;
			}
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
						=E_PARSER_ERROR;

		};
errorinfo:	errorinfocolon errorinfofields;
errorinfocolon: ERRORINFO
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageResponse)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
	};
errorinfofields: errorinfofield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeErrorInfo;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)\
			order,  (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr\
			->slErrorInfoHdr), glbSipParserErrorInfoHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 == SipFail)
			YYABORT;
		glbSipParserErrorInfoHeader=SIP_NULL;
	}
	|	errorinfofields COMMA errorinfofield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr\
			->slErrorInfoHdr), glbSipParserErrorInfoHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 == SipFail)
			YYABORT;
		glbSipParserErrorInfoHeader=SIP_NULL;
	};
errorinfofield:	errorinfouri
	|	errorinfouri SEMICOLON errorinfogenparams;
errorinfouri:	URI	
	{
		if(sip_initSipErrorInfoHeader(&glbSipParserErrorInfoHeader,\
			  (((SipHeaderParserParam *) pHeaderParserParam)->\
			  pError)) == SipFail)
			YYABORT;
		glbSipParserErrorInfoHeader->pUri = sip_tokenBufferDup($1);
		if(glbSipParserErrorInfoHeader->pUri == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
errorinfogenparams: errorinfogenparam
	| errorinfogenparams SEMICOLON errorinfogenparam;
errorinfogenparam: token
	{
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserErrorInfoHeader->slParam), pParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 == SipFail)
			YYABORT;
	}
	| token EQUALS allstring
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;

		if(sip_listAppend(&(glbSipParserErrorInfoHeader->slParam), pParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 == SipFail)
			YYABORT;
	}
	| token EQUALS IPV6ADDR
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;

		if(sip_listAppend(&(glbSipParserErrorInfoHeader->slParam), pParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 == SipFail)
			YYABORT;
	};
inreplyto: inreplytocolon inreplytofields;
inreplytofields: callid
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeInReplyTo;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order,  (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr\
			->slInReplyToHdr), glbSipParserInReplyToHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		glbSipParserInReplyToHeader = SIP_NULL;
	}
	|inreplytofields COMMA callid
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;				
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr\
			->slInReplyToHdr), glbSipParserInReplyToHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		glbSipParserInReplyToHeader = SIP_NULL;
	}
	| inreplytofields COMMA;
callid: callidvalue  	
	{
		if(sip_initSipInReplyToHeader(&glbSipParserInReplyToHeader,\
			  (((SipHeaderParserParam *) pHeaderParserParam)->\
			  pError)) == SipFail)
			YYABORT;
    	glbSipParserInReplyToHeader->pCallId = sip_tokenBufferDup($1);
 		if(glbSipParserInReplyToHeader->pCallId == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}; 
callidvalue: CALLIDAT
	{
		SIP_U16bit i=0,j=0;
		while(i < strlen($1.pToken))
		{
			if ( $1.pToken[i] != ' ' && $1.pToken[i] != '\t' ) 
			{
				$1.pToken[j] = $1.pToken[i];
				j++;
			}
			i++;
		}
		$1.pToken[j] = '\0';
	}
	|WORD_LEX
	{
		SIP_U16bit i=0,j=0;
		while(i < strlen($1.pToken))
		{
			if ( $1.pToken[i] != ' ' && $1.pToken[i] != '\t' ) 
			{
				$1.pToken[j] = $1.pToken[i];
				j++;
			}
			i++;
		}
		$1.pToken[j] = '\0';
	}
		| token_inrep;
inreplytocolon:INREPLYTO 
	{
		/* Check if message is a request message */
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_ERROR;
			YYABORT;
		}
	};
callinfo: callinfocolon	
	|	callinfocolon callinfofields;
callinfocolon: CALLINFO;
callinfofields: callinfofield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeCallInfo;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)\
			order,  (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr\
			->slCallInfoHdr), glbSipParserCallInfoHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;	
		glbSipParserCallInfoHeader=SIP_NULL;
	}
	|	callinfofields COMMA callinfofield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr\
			->slCallInfoHdr), glbSipParserCallInfoHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;	
		glbSipParserCallInfoHeader=SIP_NULL;
	};
callinfofield:	callinfouri
	|	callinfouri SEMICOLON callinfogenparams;
callinfouri: CALLIDAT	
	{
		if(sip_initSipCallInfoHeader(&glbSipParserCallInfoHeader,\
			  (((SipHeaderParserParam *) pHeaderParserParam)->\
			  pError)) == SipFail)
			YYABORT;
		glbSipParserCallInfoHeader->pUri = sip_tokenBufferDup($1);
		if(glbSipParserCallInfoHeader->pUri == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}
	| uri 
	{
		if(sip_initSipCallInfoHeader(&glbSipParserCallInfoHeader,\
			  (((SipHeaderParserParam *) pHeaderParserParam)->\
			  pError)) == SipFail)
			YYABORT;
		glbSipParserCallInfoHeader->pUri = sip_tokenBufferDup($1);
		if(glbSipParserCallInfoHeader->pUri == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
uri: URI;
callinfogenparams: callinfogenparam
	| callinfogenparams SEMICOLON callinfogenparam;
callinfogenparam: token
	{
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserCallInfoHeader->slParam), pParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
	}
	| token EQUALS genvalue
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;

		if(sip_listAppend(&(glbSipParserCallInfoHeader->slParam), pParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
	};
contentdisp: contentdispcolon contentdispfields;
contentdispcolon:CONTENTDISPOSITION;
contentdispfields: contentdispfield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeContentDisposition;
		order->dTextType = SipFormFull;
		order->dNum = 1;
	
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)\
			order,  (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr\
			->slContentDispositionHdr), glbSipParserContentDispositionHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserContentDispositionHeader=SIP_NULL;
	}
	|contentdispfields COMMA contentdispfield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;

		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr\
			->slContentDispositionHdr), glbSipParserContentDispositionHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserContentDispositionHeader=SIP_NULL;
	};
contentdispfield: contentdispdisptype
	|	contentdispdisptype SEMICOLON contentdispparams;
contentdispdisptype:	token	
	{
		if(sip_initSipContentDispositionHeader(\
			&glbSipParserContentDispositionHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserContentDispositionHeader->pDispType=sip_tokenBufferDup($1);
		if(glbSipParserContentDispositionHeader->pDispType == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
contentdispparams: contentdispparam
	| contentdispparams SEMICOLON contentdispparam;
contentdispparam: token
	{
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserContentDispositionHeader->slParam),\
			pParam,  (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
	}
	| token EQUALS genvalue
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;

		if(sip_listAppend(&(glbSipParserContentDispositionHeader->slParam),\
			pParam,(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
	};
alertinfo: alertinfocolon 
		| alertinfocolon alertinfofields;
alertinfofields: alertinfofield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAlertInfo;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)\
			order,  (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr\
			->slAlertInfoHdr), glbSipParserAlertInfoHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserAlertInfoHeader=SIP_NULL;
	}
	| alertinfofields COMMA alertinfofield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;				
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr\
			->slAlertInfoHdr), glbSipParserAlertInfoHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserAlertInfoHeader=SIP_NULL;
	};
alertinfofield: alertinfouri
	| alertinfouri SEMICOLON alertinfogenparams;
alertinfouri: URI
	{
		if(sip_initSipAlertInfoHeader(&glbSipParserAlertInfoHeader,\
			  (((SipHeaderParserParam *) pHeaderParserParam)->\
			  pError)) == SipFail)
			YYABORT;
		glbSipParserAlertInfoHeader->pUri = sip_tokenBufferDup($1);
		if(glbSipParserAlertInfoHeader->pUri == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
alertinfocolon: ALERTINFO
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
	};
alertinfogenparams: alertinfogenparam
	| alertinfogenparams SEMICOLON alertinfogenparam;
alertinfogenparam: token
	{
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserAlertInfoHeader->slParam), pParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
			
	}
	| token EQUALS alltoken
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;
		if(sip_listAppend(&(glbSipParserAlertInfoHeader->slParam), pParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
	};
/*allstring: token
	|QSTRING*/
accept:	ACCEPT acceptfields;	
acceptfields:acceptfields COMMA acceptfield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAcceptHdr),\
			(SIP_Pvoid)glbSipParserAcceptHeader, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess) 
			YYABORT;
		glbSipParserAcceptHeader=SIP_NULL;
	}
	|acceptfield 
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAccept;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order,  (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAcceptHdr),\
			(SIP_Pvoid)glbSipParserAcceptHeader, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess) 
			YYABORT;
		glbSipParserAcceptHeader=SIP_NULL;
	}
	|acceptfields COMMA
	| 
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAccept;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order,  (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipAcceptHeader(&glbSipParserAcceptHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAcceptHdr),\
			(SIP_Pvoid)glbSipParserAcceptHeader, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess) 
			YYABORT;
		glbSipParserAcceptHeader=SIP_NULL;
	};
acceptfield:mediarange
	|mediarange acceptparams;
mediarange:	token SLASH token 
	{ 
		if(sip_initSipAcceptHeader(&glbSipParserAcceptHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
		glbSipParserAcceptHeader->pMediaRange = (SIP_S8bit *) \
			fast_memget(BISON_MEM_ID, $1.dLength+$3.dLength+2, SIP_NULL);
		if(glbSipParserAcceptHeader->pMediaRange==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(glbSipParserAcceptHeader->pMediaRange, $1.pToken, $1.dLength);
		glbSipParserAcceptHeader->pMediaRange[$1.dLength] = '/';
		strncpy(&(glbSipParserAcceptHeader->pMediaRange[$1.dLength+1]),\
			$3.pToken, $3.dLength);
		glbSipParserAcceptHeader->pMediaRange[$1.dLength+$3.dLength+1] = '\0';
		glbSipParserAcceptHeader->pAcceptRange = SIP_NULL;
	}
		|token SLASH token SEMICOLON parameters 
	{ 
		if(sip_initSipAcceptHeader(&glbSipParserAcceptHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
		glbSipParserAcceptHeader->pMediaRange = (SIP_S8bit *)\
			fast_memget(BISON_MEM_ID, $1.dLength+$3.dLength+\
			$5.dLength+3, SIP_NULL);
		if(glbSipParserAcceptHeader->pMediaRange==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}

		strncpy(glbSipParserAcceptHeader->pMediaRange, $1.pToken, $1.dLength);
		glbSipParserAcceptHeader->pMediaRange[$1.dLength] = '/';
		strncpy(&(glbSipParserAcceptHeader->pMediaRange[$1.dLength+1]),\
			$3.pToken, $3.dLength);
		glbSipParserAcceptHeader->pMediaRange[$1.dLength+$3.dLength+1] = ';';
		strncpy(&(glbSipParserAcceptHeader->\
			pMediaRange[$1.dLength+$3.dLength+2]), $5.pToken, $5.dLength);
		glbSipParserAcceptHeader->pMediaRange\
			[$1.dLength+$3.dLength+$5.dLength+2] = '\0';

		glbSipParserAcceptHeader->pAcceptRange = SIP_NULL;
	};
parameters:	parameters SEMICOLON parameter 
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$3.dLength+1, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *)\
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength] = ';';
		strncpy(&($$.pToken[$1.dLength+1]), $3.pToken, $3.dLength);
		$$.dLength = $1.dLength+$3.dLength+1;
	}
	|parameter; 
parameter:	token EQUALS alltoken
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$3.dLength+1, (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *) \
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength] = '=';
		strncpy(&($$.pToken[$1.dLength+1]), $3.pToken, $3.dLength);
		$$.dLength = $1.dLength+$3.dLength+1;
	}
	|token;
alltoken: token
	| QSTRING
	| IPV6ADDR;
acceptparams:qparam	
	{
		glbSipParserAcceptHeader->pAcceptRange = (SIP_S8bit *)\
			fast_memget(BISON_MEM_ID,$1.dLength+4,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(glbSipParserAcceptHeader->pAcceptRange==SIP_NULL)
			YYABORT;
		strncpy(glbSipParserAcceptHeader->pAcceptRange, ";q=",3);
		strncpy(&(glbSipParserAcceptHeader->pAcceptRange[3]),$1.pToken,\
			$1.dLength);
		glbSipParserAcceptHeader->pAcceptRange[$1.dLength+3]='\0';
	}
		|qparam SEMICOLON parameters 
	{
		glbSipParserAcceptHeader->pAcceptRange = (SIP_S8bit *)\
			fast_memget(BISON_MEM_ID, $1.dLength+$3.dLength+6, SIP_NULL);
		if(glbSipParserAcceptHeader->pAcceptRange==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(glbSipParserAcceptHeader->pAcceptRange, ";q=",3);
		strncpy(&(glbSipParserAcceptHeader->pAcceptRange[3]), $1.pToken,\
			$1.dLength);
		glbSipParserAcceptHeader->pAcceptRange[$1.dLength+3] = ';';
		strncpy(&(glbSipParserAcceptHeader->pAcceptRange[$1.dLength+4]),\
			$3.pToken, $3.dLength);
		glbSipParserAcceptHeader->pAcceptRange[$1.dLength+$3.dLength+4] = '\0';
	};
acceptlanguage:	ACCEPTLANG acptlangfields;
acptlangfields:	acptlangfields COMMA acptlangfield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)			
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAcceptLang),\
			(SIP_Pvoid)glbSipParserAcceptLangHeader, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess) 
			YYABORT;
		glbSipParserAcceptLangHeader = SIP_NULL;
	}
	|acptlangfield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAcceptLanguage;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order,  (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAcceptLang),\
			(SIP_Pvoid)glbSipParserAcceptLangHeader, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess) 
			YYABORT;
		glbSipParserAcceptLangHeader = SIP_NULL;
	}
	|acptlangfields COMMA
	|
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAcceptLanguage;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order,  (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipAcceptLangHeader(&glbSipParserAcceptLangHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAcceptLang),\
			(SIP_Pvoid)glbSipParserAcceptLangHeader, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess) 
			YYABORT;
		glbSipParserAcceptLangHeader=SIP_NULL;
	};
acptlangfield:	langrange 
	{
		if(sip_initSipAcceptLangHeader(&glbSipParserAcceptLangHeader, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
				YYABORT;
		glbSipParserAcceptLangHeader->pLangRange = sip_tokenBufferDup($1);
		if(glbSipParserAcceptLangHeader->pLangRange==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		glbSipParserAcceptLangHeader->pQValue = SIP_NULL;
	}
		|langrange acceptlangparams
	{
		glbSipParserAcceptLangHeader->pLangRange = sip_tokenBufferDup($1);
		if(glbSipParserAcceptLangHeader->pLangRange==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
acceptlangparams:	acceptlangparam
		|	acceptlangparams acceptlangparam;

acceptlangparam:qparam
	{
		SIP_S8bit	*pQValue=SIP_NULL;
		SIP_S8bit	*pName=SIP_NULL;
		SIP_S8bit	*pValue=SIP_NULL;
		
		SipBool	qValAlreadyPresent=SipFail;
		
		if (glbSipParserAcceptLangHeader==SIP_NULL)	
		{	
			if(sip_initSipAcceptLangHeader(&glbSipParserAcceptLangHeader, \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;
		}		
		
		pQValue=sip_tokenBufferDup($1);
		if (glbSipParserAcceptLangHeader->pQValue!=SIP_NULL)
		{	
			qValAlreadyPresent=SipSuccess;
		}	
		
		if (qValAlreadyPresent==SipFail)
		{	
			glbSipParserAcceptLangHeader->pQValue = pQValue;
		}
		else
		{
			SipNameValuePair *pNameValuePair=SIP_NULL;
			
			/*Since qvalue is already present in the hdr this needs
			to be added into the NameValuePair list*/
			pName=strdup("q");
			if (pName==SIP_NULL)
			{
				*(((SipHeaderParserParam *)pHeaderParserParam)->pError)=E_NO_MEM;
				YYABORT;
			}
			
			pValue=pQValue;
			/*Add this in the list of SipNameValuePairs*/
			if(sip_initSipNameValuePair(&pNameValuePair,\
				(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
				== SipFail) YYABORT;

			pNameValuePair->pName=pName;
			pNameValuePair->pValue=pValue;
			
			/*Now add this NameValuePair to the list of NameValuepairs*/
			if(sip_listAppend(&(glbSipParserAcceptLangHeader->slParam),\
				pNameValuePair,\
				(((SipHeaderParserParam *) pHeaderParserParam)->\
				 pError)) == SipFail)
				YYABORT;
		}
	}
	| SEMICOLON token
	{
		SipNameValuePair *pNameValuePair=SIP_NULL;
		SIP_S8bit *pName=SIP_NULL;
		if (glbSipParserAcceptLangHeader==SIP_NULL)	
		{	
			if(sip_initSipAcceptLangHeader(&glbSipParserAcceptLangHeader, \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;
		}	

		/*First find out if the param is of the type qvalue or not*/
		pName=sip_tokenBufferDup($2);
		if(pName==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}
		
		/*Add this in the list of SipNameValuePairs*/
		if(sip_initSipNameValuePair(&pNameValuePair,\
			(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
			== SipFail)
		YYABORT;

		pNameValuePair->pName=pName;
		pNameValuePair->pValue=SIP_NULL;
			
		/*Now add this NameValuePair to the list of NameValuepairs*/
		if(sip_listAppend(&(glbSipParserAcceptLangHeader->slParam),\
			pNameValuePair,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;

	}
	| SEMICOLON token EQUALS IPV6ADDR
	{
		SipNameValuePair *pNameValuePair=SIP_NULL;
		SIP_S8bit *pName=SIP_NULL;
		SIP_S8bit *pValue=SIP_NULL;
		if (glbSipParserAcceptLangHeader==SIP_NULL)	
		{	
			if(sip_initSipAcceptLangHeader(&glbSipParserAcceptLangHeader, \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;
		}	

		/*First find out if the param is of the type qvalue or not*/
		pName=sip_tokenBufferDup($2);
		if(pName==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}

		pValue=sip_tokenBufferDup($4);
		if (pValue==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}

		/*Add this in the list of SipNameValuePairs*/
		if(sip_initSipNameValuePair(&pNameValuePair,\
			(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
			== SipFail)
		YYABORT;

		pNameValuePair->pName=pName;
		pNameValuePair->pValue=pValue;
			
		/*Now add this NameValuePair to the list of NameValuepairs*/
		if(sip_listAppend(&(glbSipParserAcceptLangHeader->slParam),\
			pNameValuePair,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
	}	
	|	SEMICOLON token EQUALS allstring
	{
		SipNameValuePair *pNameValuePair=SIP_NULL;
		SIP_S8bit *pName=SIP_NULL;
		SIP_S8bit *pValue=SIP_NULL;
		SipBool dQVal=SipFail;
		if (glbSipParserAcceptLangHeader==SIP_NULL)	
		{	
			if(sip_initSipAcceptLangHeader(&glbSipParserAcceptLangHeader, \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;
		}	

		/*First find out if the param is of the type qvalue or not*/
		pName=sip_tokenBufferDup($2);
		if(pName==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}

		pValue=sip_tokenBufferDup($4);
		if (pValue==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}

		if ((!strcmp(pName,"q"))||(!strcmp(pName,"Q")))
		{
			/*It is a "q=xyz" type and needs to be treated as such
				we need to however check if we have already recvd
				a qvalue*/
			if ((glbSipParserAcceptLangHeader->pQValue)==SIP_NULL)
			{	
				dQVal=SipSuccess;
				fast_memfree(BISON_MEM_ID,pName,SIP_NULL);
			}	
		}

		if (dQVal==SipSuccess)
		{
			glbSipParserAcceptLangHeader->pQValue = pValue;
		}
		else
		{
			/*Add this in the list of SipNameValuePairs*/
			if(sip_initSipNameValuePair(&pNameValuePair,\
				(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
				== SipFail)
			YYABORT;

			pNameValuePair->pName=pName;
			pNameValuePair->pValue=pValue;
			
			/*Now add this NameValuePair to the list of NameValuepairs*/
			if(sip_listAppend(&(glbSipParserAcceptLangHeader->slParam),\
				pNameValuePair,\
				(((SipHeaderParserParam *) pHeaderParserParam)->\
				 pError)) == SipFail)
				YYABORT;
		}	
	};

genvalue : allstring
	 | IPV6ADDR; 
	
allstring: TOKEN
	|QSTRING
	|LRANGE
	|DIGIT3
	|SIPFLOAT;
		
langrange:	LRANGE
	{
		SIP_U16bit i=0,j=0;
		while(i < strlen($1.pToken))
		{
			if ( $1.pToken[i] != ' ' && $1.pToken[i] != '\t' ) 
			{
				$1.pToken[j] = $1.pToken[i];
				j++;
			}
			i++;
		}
		$1.pToken[j] = '\0';
	};
	
contenttype:contenttypecolon mediatype
{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType = SipHdrTypeContentType;
		if($1.dChar=='f')
		order->dTextType = SipFormFull;
		else
		order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
		(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pContentTypeHdr =\
			glbSipParserContentTypeHeader;
		glbSipParserContentTypeHeader = SIP_NULL;
};
event:eventcolon eventtype
{
#ifdef SIP_IMPP
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeEvent;
		order->dTextType = glbSipSupportedType;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->u.pRequest->pRequestHdr->pEventHdr = glbSipParserEventHeader;
		glbSipParserEventHeader=SIP_NULL;
#endif
};
mediatype:	type SLASH subtype ctypeparameters 
	{
		glbSipParserContentTypeHeader->pMediaType = (SIP_S8bit *) \
			fast_memget(BISON_MEM_ID, $1.dLength+$3.dLength+2, SIP_NULL);
		if(glbSipParserContentTypeHeader->pMediaType==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(glbSipParserContentTypeHeader->pMediaType,$1.pToken,$1.dLength);
		glbSipParserContentTypeHeader->pMediaType[$1.dLength] = '/';
		strncpy(&(glbSipParserContentTypeHeader->pMediaType[$1.dLength+1]),\
			$3.pToken,$3.dLength);
		glbSipParserContentTypeHeader->pMediaType[$1.dLength+$3.dLength+1]='\0';
	};
eventtype:	eventtypeinfo
	| eventtypeinfo SEMICOLON eventgenparams;
eventtypeinfo:	token	
	{
#ifdef SIP_IMPP
		glbSipParserEventHeader->pEventType = (SIP_S8bit *) \
			fast_memget(BISON_MEM_ID, $1.dLength+1, SIP_NULL);
		if(glbSipParserEventHeader->pEventType==SIP_NULL)
		{
			*((SipHeaderParserParam *)pHeaderParserParam)->pError = E_NO_MEM;
			YYABORT;
		}
		strncpy(glbSipParserEventHeader->pEventType,$1.pToken,$1.dLength);
		glbSipParserEventHeader->pEventType[$1.dLength] = '\0';
#endif		
	};

eventgenparams: eventgenparam
	| eventgenparams SEMICOLON eventgenparam;
eventgenparam: token
	{
#ifdef SIP_IMPP
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *)pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserEventHeader->slParams), pParam,\
			 (((SipHeaderParserParam *)pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
#endif
	}
	| token EQUALS genvalue
	{
#ifdef SIP_IMPP
		SipParam *pParam;
		SIP_S8bit *tempVal;
		
		if(sip_initSipParam(&pParam,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *)pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *)pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,  \
			(((SipHeaderParserParam *)pHeaderParserParam)->pError))	==SipFail)
			YYABORT;
		if(sip_listAppend(&(glbSipParserEventHeader->slParams), pParam,\
			 (((SipHeaderParserParam *)pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
#endif
	};
	
ctypeparameters:ctypeparameters SEMICOLON ctypeparameter 
	| ;
ctypeparameter:	token EQUALS qtoken
	{
		SipGeneralHeader *genhdr;
		SIP_S8bit *pValue;
		SipParam *param;
			
		genhdr = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;

		if(sip_initSipParam(&param, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		param->pName = sip_tokenBufferDup($1);
		if(param->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		pValue = sip_tokenBufferDup($3);
		if(pValue == SIP_NULL) 
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(param->slValue),pValue,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) ==SipFail)
			YYABORT;
		if(sip_listAppend (&(glbSipParserContentTypeHeader->slParams),\
			(SIP_Pvoid)param,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))==SipFail)
			YYABORT;
			
	};
qtoken:		QSTRING
			|token;
			
type:		token;		
subtype:	token;		
warning: 	warningcolon warningfields;
warningfields:	warningfields COMMA warningfield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize, (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pResponse->\
			pResponseHdr->slWarningHeader), \
			(SIP_Pvoid)glbSipParserWarnHdr, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess) 
			YYABORT;
		glbSipParserWarnHdr = SIP_NULL;
	}
	| warningfield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeWarning;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)\
			order,  (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pResponse->\
			pResponseHdr->slWarningHeader), \
			(SIP_Pvoid)glbSipParserWarnHdr, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess) 
			YYABORT;
		glbSipParserWarnHdr = SIP_NULL;
	}
		|warningfields COMMA;
warningfield: DIGIT3 agent QSTRING 
	{
		SIP_S8bit *temp;

		if(sip_initSipWarningHeader(&glbSipParserWarnHdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		temp = sip_tokenBufferDup($1);
		glbSipParserWarnHdr->dCodeNum = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		glbSipParserWarnHdr->pAgent = sip_tokenBufferDup($2);
		if(glbSipParserWarnHdr->pAgent==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		glbSipParserWarnHdr->pText = sip_tokenBufferDup($3);
		if(glbSipParserWarnHdr->pText==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
agent:  token
		|token COLON token 
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$3.dLength+1, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *) \
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength] = ':';
		strncpy(&($$.pToken[$1.dLength+1]), $3.pToken, $3.dLength);
		$$.dLength = $1.dLength+$3.dLength+1;
	}
	| IPV6ADDR
	| IPV6ADDR COLON token
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$3.dLength+1, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *) \
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength] = ':';
		strncpy(&($$.pToken[$1.dLength+1]), $3.pToken, $3.dLength);
		$$.dLength = $1.dLength+$3.dLength+1;
	};
mimeversion: MIMEVERSION SIPFLOAT
	{	
		SipHeaderOrderInfo *order;
		SipGeneralHeader *genhdr;
		SipMimeVersionHeader *mimeverhdr; 
		SipBool dResult;

		genhdr = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(genhdr->pMimeVersionHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one Mime Version header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
		if(sip_bcpt_initSipMimeVersionHeader(&mimeverhdr,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
		mimeverhdr->pVersion = sip_tokenBufferDup($2);
		if(mimeverhdr->pVersion==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		genhdr->pMimeVersionHdr = mimeverhdr;

		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeMimeVersion;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
	};
token_inrep:	TOKEN	
		| LRANGE
		| LRANGE_INREP
		|SIPFLOAT
		|DIGIT3;
token:	TOKEN	
		| LRANGE
		|SIPFLOAT
		|DIGIT3;
qparam: QPARAM
	{
		SIP_U16bit i,j;
	
		for(i=0;$1.pToken[i]!='=';i++)
		{}
		j=i+1;
		while(isspace((int)$1.pToken[j]))
			j++;
		$$.pToken = &($1.pToken[j]);
		$$.dLength = $1.dLength-j;
	};
contenttypecolon:CONTENTTYPE
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pContentTypeHdr != SIP_NULL)
		{
			sip_error (SIP_Minor,\
				"There can only be one Content Type header in a message");
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}

		if(sip_initSipContentTypeHeader(&glbSipParserContentTypeHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
	};
eventcolon:EVENT
	{
#ifdef SIP_IMPP		
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
                        pSipMessage->dType!=SipMessageRequest)
                {
                        *(((SipHeaderParserParam *) pHeaderParserParam)->\
	                        pError) = E_NO_ERROR;
                        YYABORT;
                }

		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pEventHdr != SIP_NULL)
		{
			sip_error (SIP_Minor,\
				"There can only be one Event header in a message");
			*(((SipHeaderParserParam *)pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}

		if(sip_impp_initSipEventHeader(&glbSipParserEventHeader,\
			 (((SipHeaderParserParam *)pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
		if($1.dChar=='f')
			glbSipSupportedType = SipFormFull;
		else
			glbSipSupportedType = SipFormShort;
#endif
	};
warningcolon : WARNING
{
	if(((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->dType != SipMessageResponse)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
};

authenticationinfo:authinfocolon authinfoparams
	{
		SipHeaderOrderInfo *pOrder;
		SipBool	dResult;
		dResult=sip_initSipHeaderOrderInfo(&pOrder,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}			
		
		pOrder->dType = SipHdrTypeAuthenticationInfo;
		pOrder->dTextType = SipFormFull;
		pOrder->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)pOrder,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
			slAuthenticationInfoHdr,glbSipAuthenticationInfo,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		glbSipAuthenticationInfo=SIP_NULL;	
	};
authinfocolon: AUTHINFO
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageResponse)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
		if ( sip_initSipAuthenticationInfoHeader(&glbSipAuthenticationInfo,(((SipHeaderParserParam *) pHeaderParserParam)->pError)) \
				== SipFail )
				YYABORT;
	};
authinfoparams:authinfoparams COMMA authinfoparam
	|authinfoparam
	|authinfoparams COMMA;
authinfoparam: token EQUALS alltoken1
	{
		SIP_U32bit I;
		SipNameValuePair *pTempValue;

		I = $1.dLength-1;
		while(isspace((int)$1.pToken[I])) I--;
		$1.dLength = I+1;

		if ( sip_initSipNameValuePair(&pTempValue,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
				YYABORT;
		
		pTempValue->pName = sip_tokenBufferDup($1);
		pTempValue->pValue = sip_tokenBufferDup($3);
		
		if(sip_listAppend(&(glbSipAuthenticationInfo->slNameValue),pTempValue,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

	};
alltoken1:TOKEN
	|LRANGE
	|QSTRING
	|DIGIT3
	|SIPFLOAT;
reason: reasoncolon reasonfields;
reasoncolon:REASON;
reasonfields: reasonfield
	{
		SipHeaderOrderInfo *pOrder;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&pOrder,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		pOrder->dType = SipHdrTypeReason;
		pOrder->dTextType = SipFormFull;
		pOrder->dNum = 1;
	
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)\
			pOrder,  (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr\
			->slReasonHdr), glbSipParserReasonHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserReasonHeader=SIP_NULL;
	}
	|reasonfields COMMA reasonfield
	{
		SipHeaderOrderInfo *pOrder;
		SIP_U32bit dOrdersize;

		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&dOrdersize, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,dOrdersize-1, \
			(SIP_Pvoid *)&pOrder, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pOrder->dNum++;
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr\
			->slReasonHdr), glbSipParserReasonHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserReasonHeader=SIP_NULL;
};
reasonfield: reasontype
	|	reasontype SEMICOLON reasonparams;
reasontype:	token	
	{
		if(sip_initSipReasonHeader(\
			&glbSipParserReasonHeader,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		glbSipParserReasonHeader->pDispType=sip_tokenBufferDup($1);
		if(glbSipParserReasonHeader->pDispType == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
reasonparams: reasonparam
	| reasonparams SEMICOLON reasonparam;
reasonparam: token
	{
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam,(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserReasonHeader->slParam),\
			pParam,  (((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
	}
	| token EQUALS genvalue
	{
		SipParam *pParam;
		SIP_S8bit *pTempval;
		
		if(sip_initSipParam(&pParam,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_MEM;
			YYABORT;
		}
		pTempval = sip_tokenBufferDup($3);
		if(pTempval == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), pTempval,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;

		if(sip_listAppend(&(glbSipParserReasonHeader->slParam),\
			pParam,(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
	};
