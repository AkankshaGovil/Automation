
/***********************************************************************
 ** FUNCTION:
 **             Yacc file for date/time headers

 *********************************************************************
 **
 ** FILENAME:
 ** Datetime.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form date/time
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token DATEHDRNAME EXPIRES TIMESTAMP RETRYAFTER SESSIONEXPIRES 
%token MINSE SUBSCRIPTIONEXPIRES MINEXPIRES IPV6ADDR
%token DIGITS DIGIT2 DIGIT4 MONTH WEEKDAY SIPFLOAT
%token COMMA SEMICOLON EQUALS COLON
%token COMMENT GMT QSTRING TOKEN
%token DEFAULT
%pure_parser
%{ 
#include "sipparserinc.h"
#include "siptrace.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siptrace.h"

#ifdef SIP_IMPP
#include "imppinit.h"
#endif

#define MAXTEMP (300)


#define YYPARSE_PARAM pHeaderParserParam

static SipDateStruct *glbSipParserDateStruct=SIP_NULL;
static SipRetryAfterHeader *glbSipParserRetryAfterHdr=SIP_NULL;
#ifdef SIP_SESSIONTIMER
static SipSessionExpiresHeader *glbSipParserSessionExpiresHdr=SIP_NULL;
#endif
#ifdef SIP_IMPP
static SipSubscriptionStateHeader *glbSipParserSubscriptionStateHeader=SIP_NULL;
#endif
%}
%%
header:	dateheader
		|expires
		|minse
		|timestamp  
		|retryafter
		|sessionexpires
		|subscriptionexpires
		|minexpires
		|error
		{
			if(glbSipParserDateStruct != SIP_NULL)
			{
				sip_freeSipDateStruct(glbSipParserDateStruct);
				glbSipParserDateStruct = SIP_NULL;
			}
			if(glbSipParserRetryAfterHdr != SIP_NULL)
			{
				sip_freeSipRetryAfterHeader(glbSipParserRetryAfterHdr);
				glbSipParserRetryAfterHdr = SIP_NULL;
			}
#ifdef SIP_SESSIONTIMER
			if(glbSipParserSessionExpiresHdr != SIP_NULL)
			{
				sip_freeSipSessionExpiresHeader(glbSipParserSessionExpiresHdr);
				glbSipParserSessionExpiresHdr = SIP_NULL;
			}
#endif
#ifdef SIP_IMPP
			if(glbSipParserSubscriptionStateHeader!= SIP_NULL)
			{
				sip_impp_freeSipSubscriptionStateHeader(glbSipParserSubscriptionStateHeader);	
				glbSipParserSubscriptionStateHeader= SIP_NULL;
			}
#endif

			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					=E_PARSER_ERROR;
		};

dateheader:		datecolon sipdate
	{
		SipGeneralHeader *gheader;
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr;
		gheader->pDateHdr = (SipDateHeader *)glbSipParserDateStruct;
		glbSipParserDateStruct = SIP_NULL;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeDate;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)\
			->pSipMessage->slOrderInfo,(SIP_Pvoid)order\
			,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
	};
minexpires: minexpirescolon  digits
	{
		SipHeaderOrderInfo *order;
		SipRespHeader *pheader;
		SIP_S8bit *temp;
		SipBool dResult;

		pheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->u.pResponse->pResponseHdr;
		if(sip_initSipMinExpiresHeader(&(pheader->pMinExpiresHdr),\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		temp = sip_tokenBufferDup($2);
		pheader->pMinExpiresHdr->dSec = STRTOU32CAP(temp,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		fast_memfree(BISON_MEM_ID, temp, SIP_NULL);

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeMinExpires;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
	
expires:	expirescolon sipdate
	{
		SipHeaderOrderInfo *order;
		SipGeneralHeader *gheader;
		SipBool	dResult;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(sip_initSipExpiresHeader(& gheader->pExpiresHdr,SipExpDate, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		gheader->pExpiresHdr->u.pDate = glbSipParserDateStruct;
		glbSipParserDateStruct = SIP_NULL;
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
			
		order->dType = SipHdrTypeExpiresAny;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	}
		|expirescolon  digits
	{
		SipHeaderOrderInfo *order;
		SipGeneralHeader *gheader;
		SipBool	dResult;
		SIP_S8bit *temp;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(sip_initSipExpiresHeader(& gheader->pExpiresHdr,SipExpSeconds, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		temp = sip_tokenBufferDup($2);
		gheader->pExpiresHdr->u.dSec = STRTOU32CAP\
			(temp,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		fast_memfree(BISON_MEM_ID, temp, SIP_NULL);
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}	

		order->dType = SipHdrTypeExpiresAny;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
subscriptionexpires: subscriptionexpirescolon subscriptionparams
{
#ifdef SIP_IMPP
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeSubscriptionState;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		
		((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pSubscriptionStateHdr = \
			glbSipParserSubscriptionStateHeader;
		glbSipParserSubscriptionStateHeader=SIP_NULL;
#endif
};
subscriptionparams : substate 
					| substate SEMICOLON subexpgenparams;

substate : token 
	{
#ifdef SIP_IMPP
		glbSipParserSubscriptionStateHeader->pSubState \
				= sip_tokenBufferDup($1);
#endif
	};
subexpgenparams: subexpgenparam
		|subexpgenparams SEMICOLON subexpgenparam;

subexpgenparam: token
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
		if(sip_listAppend(&(glbSipParserSubscriptionStateHeader->slParams), \
			pParam, (((SipHeaderParserParam *)pHeaderParserParam)->pError)) == \
			SipFail)
			YYABORT;
		pParam=SIP_NULL;
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
		if(sip_listAppend(&(pParam->slValue), (void*)tempVal,  \
			(((SipHeaderParserParam *)pHeaderParserParam)->pError))	==SipFail)
			YYABORT;
		if(sip_listAppend(&(glbSipParserSubscriptionStateHeader->slParams),\
			(void*) pParam,(((SipHeaderParserParam *)pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
		pParam=SIP_NULL;
#endif
	};

minse:	minsecolondigits
	{
#ifdef SIP_SESSIONTIMER
		SipHeaderOrderInfo *order;
		SipBool	dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType = SipHdrTypeMinSE;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
#endif			
	}
	|minsecolondigits SEMICOLON minsegenparams
	{
#ifdef SIP_SESSIONTIMER
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType = SipHdrTypeMinSE;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
#endif			
	};
minsegenparams: minsegenparam
		|minsegenparams SEMICOLON minsegenparam;
minsegenparam: token
	{
#ifdef SIP_SESSIONTIMER
	 	SipNameValuePair *pNameValue; 
		SipGeneralHeader *gheader;
		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr;
	
		sip_initSipNameValuePair(&pNameValue,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));

	 	if ( pNameValue == SIP_NULL )
	 	{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			sip_freeSipMinSEHeader(gheader->pMinSEHdr);
			YYABORT;
	 	}
		pNameValue->pName = sip_tokenBufferDup($1);
		if(pNameValue->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			sip_freeSipNameValuePair(pNameValue);	
			sip_freeSipMinSEHeader(gheader->pMinSEHdr);
			YYABORT;
		}
	        sip_listAppend(&(gheader->pMinSEHdr->slNameValue),pNameValue,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
#endif			
	}
	| token EQUALS genvalue
	{
#ifdef SIP_SESSIONTIMER
	 	SipNameValuePair *pNameValue; 
		SipGeneralHeader *gheader;
		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr;
		
		sip_initSipNameValuePair(&pNameValue,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));

	 	if ( pNameValue == SIP_NULL )
	 	{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			sip_freeSipMinSEHeader(gheader->pMinSEHdr);
			YYABORT;
	 	}
		pNameValue->pName = sip_tokenBufferDup($1);
		if(pNameValue->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			sip_freeSipNameValuePair(pNameValue);	
			sip_freeSipMinSEHeader(gheader->pMinSEHdr);
			YYABORT;
		}
		pNameValue->pValue = sip_tokenBufferDup($3);
		if(pNameValue->pValue==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			fast_memfree(BISON_MEM_ID,pNameValue->pName,SIP_NULL);
			sip_freeSipNameValuePair(pNameValue);	
			sip_freeSipMinSEHeader(gheader->pMinSEHdr);
			YYABORT;
		}
	        sip_listAppend(&(gheader->pMinSEHdr->slNameValue),pNameValue,\
				(((SipHeaderParserParam *) pHeaderParserParam)->pError));
#endif			
	};
genvalue: allstring
 	| IPV6ADDR
	;

timestamp:	timestampcolon float
	{
		SipHeaderOrderInfo *order;
		SipGeneralHeader *gheader;
		SipBool	dResult;
		
		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(sip_initSipTimeStampHeader(&gheader->pTimeStampHdr,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		gheader->pTimeStampHdr->pTime = sip_tokenBufferDup($2);
		if(gheader->pTimeStampHdr->pTime==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_NO_MEM;
			YYABORT;
		}
		gheader->pTimeStampHdr->delay = SIP_NULL;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}	
		
		order->dType = SipHdrTypeTimestamp;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	}
		|timestampcolon float float
	{
		SipHeaderOrderInfo *order;
		SipGeneralHeader *gheader;
		SipBool	dResult;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(sip_initSipTimeStampHeader(&gheader->pTimeStampHdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		gheader->pTimeStampHdr->pTime = sip_tokenBufferDup($2);
		if(gheader->pTimeStampHdr->pTime==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		gheader->pTimeStampHdr->delay = sip_tokenBufferDup($3);
		if(gheader->pTimeStampHdr->delay == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
	
		order->dType = SipHdrTypeTimestamp;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo, (SIP_Pvoid)order, \
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
retryafter: retryafter1
{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeRetryAfterAny;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *)pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pRetryAfterHdr = glbSipParserRetryAfterHdr;
		glbSipParserRetryAfterHdr = SIP_NULL;
};
retryafter1:	retryaftercolon sipdate
	{
		if(sip_initSipRetryAfterHeader(&(glbSipParserRetryAfterHdr),\
			SipExpDate,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserRetryAfterHdr->dType = SipExpDate;
		glbSipParserRetryAfterHdr->u.pDate = glbSipParserDateStruct;
		glbSipParserDateStruct = SIP_NULL;
	} retryoptions	
	|retryaftercolon  digits 
	{
		SIP_S8bit *temp;
		
		if(sip_initSipRetryAfterHeader(&(glbSipParserRetryAfterHdr),\
			SipExpSeconds,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserRetryAfterHdr->dType = SipExpSeconds;
		temp = sip_tokenBufferDup($2);
		glbSipParserRetryAfterHdr->u.dSec = STRTOU32CAP(temp,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
							pError));
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	}retryoptions; 	
retryoptions: COMMENT
	{
		glbSipParserRetryAfterHdr->pComment = sip_tokenBufferDup($1);
		if(glbSipParserRetryAfterHdr-> pComment==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
					YYABORT;
		}
	}
		| COMMENT SEMICOLON retryparams
	{
		glbSipParserRetryAfterHdr->pComment =  sip_tokenBufferDup($1);
		if(glbSipParserRetryAfterHdr->pComment ==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}
		|  SEMICOLON retryparams
		| ;
retryparams: retryparam
     	| retryparams SEMICOLON retryparam
		 ;
retryparam: token
	{
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&( glbSipParserRetryAfterHdr-> slParams), pParam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;

		pParam=SIP_NULL;
	}
		| token EQUALS genvalue
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;

		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *)\
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
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) ==SipFail)
			YYABORT;
		if(sip_listAppend(&(glbSipParserRetryAfterHdr->slParams), pParam,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		pParam=SIP_NULL;
	};

allstring: token 
		| QSTRING;
token: TOKEN
	   |digits
	   |SIPFLOAT;
sessionexpires: sessionexpirescolon sessionexpires1
	{
#ifdef SIP_SESSIONTIMER
		SipHeaderOrderInfo *order;
		SipGeneralHeader *gheader;
		SipBool	dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
	
		order->dType = SipHdrTypeSessionExpires;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else 
			order->dTextType = SipFormShort;

		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT; 
		gheader =((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage\
			->pGeneralHdr;
		gheader->pSessionExpiresHdr = glbSipParserSessionExpiresHdr;
		glbSipParserSessionExpiresHdr = SIP_NULL;
#endif
	};
sessionexpires1: digits
	{
#ifdef SIP_SESSIONTIMER
		SIP_S8bit *temp;
		temp = sip_tokenBufferDup($1);
		glbSipParserSessionExpiresHdr->dSec = atoi(temp);
		fast_memfree(BISON_MEM_ID, temp, SIP_NULL);
#endif
	}
	| digits
	{
#ifdef SIP_SESSIONTIMER
		SIP_S8bit *temp;
		temp = sip_tokenBufferDup($1);
		glbSipParserSessionExpiresHdr->dSec = atoi(temp);
		fast_memfree(BISON_MEM_ID, temp, SIP_NULL);
#endif
	}SEMICOLON segenparams;

sipdate:	WEEKDAY COMMA date time GMT
	{
		if(strncasecmp((const SIP_S8bit *) $1.pToken,"Sun",3)==0) 
 			glbSipParserDateStruct->dDow = SipDaySun;
		else if(strncasecmp((const SIP_S8bit *) $1.pToken,"Mon",3)==0)
	 		glbSipParserDateStruct->dDow = SipDayMon;
		else if(strncasecmp((const SIP_S8bit *) $1.pToken,"Tue",3)==0)
	 		glbSipParserDateStruct->dDow = SipDayTue;
		else if(strncasecmp((const SIP_S8bit *) $1.pToken,"Wed",3)==0)
	 		glbSipParserDateStruct->dDow = SipDayWed;
		else if(strncasecmp((const SIP_S8bit *) $1.pToken,"Thu",3)==0)
 			glbSipParserDateStruct->dDow = SipDayThu;
		else if(strncasecmp((const SIP_S8bit *) $1.pToken,"Fri",3)==0)
	 		glbSipParserDateStruct->dDow = SipDayFri;
		else if(strncasecmp((const SIP_S8bit *) $1.pToken,"Sat",3)==0)
 			glbSipParserDateStruct->dDow = SipDaySat;
	}
		| date time GMT
	{
		glbSipParserDateStruct->dDow = SipDayNone;
	};
date:		digits MONTH digits24
	{
		SIP_S8bit *temp;

		if($1.dLength > 2)
		{	
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					=E_PARSER_ERROR;
			YYABORT;		
		}	
		
		if(sip_initSipDateStruct(&glbSipParserDateStruct, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		if(sip_initSipDateFormat(&(glbSipParserDateStruct->pDate),\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		temp = sip_tokenBufferDup($1);
		glbSipParserDateStruct->pDate->dDay = atoi(temp);

		
		if((atoi(temp)>SIP_MAX_MONTH_DAYS) || (atoi(temp) <= 0))
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			=E_PARSER_ERROR;
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct = SIP_NULL;
				YYABORT;
		}
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	
		if(strncasecmp((const SIP_S8bit *) $2.pToken,"Jan",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthJan;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Feb",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthFeb;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Mar",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthMar;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Apr",3)==0)
			  glbSipParserDateStruct->pDate->dMonth = SipMonthApr;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"May",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthMay;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Jun",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthJun;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Jul",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthJul;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Aug",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthAug;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Sep",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthSep;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Oct",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthOct;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Nov",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthNov;
		else if(strncasecmp((const SIP_S8bit *) $2.pToken,"Dec",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthDec;
		temp = sip_tokenBufferDup($3);
 		glbSipParserDateStruct->pDate->dYear = atoi(temp);
		if((atoi(temp) <= 0))
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			=E_PARSER_ERROR;
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct = SIP_NULL;
				YYABORT;
		}

		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	};
time:		DIGIT2 COLON DIGIT2 COLON DIGIT2
	{
		SIP_S8bit *temp;
	
		if(sip_initSipTimeFormat(&(glbSipParserDateStruct->pTime),\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		temp = sip_tokenBufferDup($1);
		glbSipParserDateStruct->pTime->dHour = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		if(glbSipParserDateStruct->pTime->dHour > SIP_23_HRS)
		{	
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_PARSER_ERROR;
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct = SIP_NULL;
			YYABORT;
		}
		temp = sip_tokenBufferDup($3);
		glbSipParserDateStruct->pTime->dMin = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		if(glbSipParserDateStruct->pTime->dMin > SIP_59_MIN)
		{	
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_PARSER_ERROR;
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct = SIP_NULL;
			YYABORT;
		}
		temp = sip_tokenBufferDup($5);
		glbSipParserDateStruct->pTime->dSec = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		if(glbSipParserDateStruct->pTime->dSec > SIP_59_SECS)
		{	
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_PARSER_ERROR;
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct = SIP_NULL;
			YYABORT;
		}
	}
		| DIGIT2 COLON DIGIT2
	{
		SIP_S8bit *temp;

		if (sip_initSipTimeFormat(&(glbSipParserDateStruct->pTime),\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		temp = sip_tokenBufferDup($1);
		glbSipParserDateStruct->pTime->dHour = atoi (temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		if(glbSipParserDateStruct->pTime->dHour > SIP_23_HRS)
		{	
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_PARSER_ERROR;
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct = SIP_NULL;
			YYABORT;
		}
		temp = sip_tokenBufferDup($3);
		glbSipParserDateStruct->pTime->dMin = atoi (temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		if(glbSipParserDateStruct->pTime->dMin > SIP_59_MIN)
		{	
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_PARSER_ERROR;
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct = SIP_NULL;
			YYABORT;
		}
		glbSipParserDateStruct->pTime->dSec = 0;
	};
digits24:DIGIT2
		|DIGIT4;
digits:	DIGIT2 	
		|DIGIT4	
		|DIGITS;	
float:	DIGIT2 	
		|DIGIT4	
		|DIGITS	
		|SIPFLOAT; 	
datecolon: DATEHDRNAME
	{
		SipGeneralHeader *gheader;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(gheader->pDateHdr !=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one Date Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
	};
retryaftercolon:RETRYAFTER
	{
			if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
				pGeneralHdr->pRetryAfterHdr!=SIP_NULL)
			{
				sip_error(SIP_Minor, "There can be only one retry after \
						 header");
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_PARSER_ERROR;
				YYABORT;
			}
	};
timestampcolon:		TIMESTAMP
	{
		SipGeneralHeader *gheader;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(gheader->pTimeStampHdr !=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one Time Stamp Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
	};
expirescolon:		EXPIRES
	{
		SipGeneralHeader *gheader;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(gheader->pExpiresHdr !=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one Expires Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_PARSER_ERROR;
			YYABORT;
		}
	};
minexpirescolon:	MINEXPIRES
	{
		SipRespHeader *pheader;

		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType!=\
			SipMessageResponse)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_ERROR;
			YYABORT;
		}

		pheader =((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pResponse->pResponseHdr;

		if(pheader->pMinExpiresHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor, \
			"There can only be one Min-Expires Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_PARSER_ERROR;
			YYABORT;
		}
	};
subscriptionexpirescolon:	SUBSCRIPTIONEXPIRES
	{
#ifdef SIP_IMPP
		SipReqHeader *sheader;

		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType!=\
			SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_ERROR;
			YYABORT;
		}

		sheader =((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestHdr;

		if(sheader->pSubscriptionStateHdr !=SIP_NULL)
		{
			sip_error (SIP_Minor, \
			"There can only be one Subscription-State Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_PARSER_ERROR;
			YYABORT;
		}

		if(sip_impp_initSipSubscriptionStateHeader( \
			&glbSipParserSubscriptionStateHeader, 
			(((SipHeaderParserParam *)pHeaderParserParam)->\
			 pError))!=SipSuccess)
			YYABORT;
#endif
	};
minsecolondigits: minsecolon  digits
	{
#ifdef SIP_SESSIONTIMER
		SipGeneralHeader *gheader;
		SIP_S8bit *temp;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr;
		if(sip_initSipMinSEHeader(& gheader->pMinSEHdr,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		temp = sip_tokenBufferDup($2);
		gheader->pMinSEHdr->dSec = STRTOU32CAP\
			(temp,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		fast_memfree(BISON_MEM_ID, temp, SIP_NULL);
		
#endif
	};
minsecolon:		MINSE
	{
#ifdef SIP_SESSIONTIMER
		SipGeneralHeader *gheader;

		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(gheader->pMinSEHdr !=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one Min-SE Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_PARSER_ERROR;
			YYABORT;
		}
#endif
	};

sessionexpirescolon:SESSIONEXPIRES
	{
#ifdef SIP_SESSIONTIMER
		SipGeneralHeader *gheader;
		gheader = ((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr;
		if(gheader->pSessionExpiresHdr !=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one Session Expires Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_PARSER_ERROR;
			YYABORT;
		}
		if(sip_initSipSessionExpiresHeader(&glbSipParserSessionExpiresHdr,\
			((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)!=SipSuccess)
			YYABORT;
#endif
	};
segenparams: segenparam
		|segenparams SEMICOLON segenparam;
segenparam: token
	{
#ifdef SIP_SESSIONTIMER
	 	SipNameValuePair *pNameValue; 
	
		sip_initSipNameValuePair(&pNameValue,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));

	 	if ( pNameValue == SIP_NULL )
	 	{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
	 	}
		pNameValue->pName = sip_tokenBufferDup($1);
		if(pNameValue->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			sip_freeSipNameValuePair(pNameValue);	
			YYABORT;
		}
	        sip_listAppend(&(glbSipParserSessionExpiresHdr->slNameValue),\
				pNameValue,(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError));
#endif			
	}
	| token EQUALS genvalue
	{
#ifdef SIP_SESSIONTIMER
	 	SipNameValuePair *pNameValue; 
		
		sip_initSipNameValuePair(&pNameValue,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));

	 	if ( pNameValue == SIP_NULL )
	 	{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
	 	}
		pNameValue->pName = sip_tokenBufferDup($1);
		if(pNameValue->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			sip_freeSipNameValuePair(pNameValue);	
			YYABORT;
		}
		pNameValue->pValue = sip_tokenBufferDup($3);
		if(pNameValue->pValue==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			fast_memfree(BISON_MEM_ID,pNameValue->pName,SIP_NULL);
			sip_freeSipNameValuePair(pNameValue);	
			YYABORT;
		}
	    sip_listAppend(&(glbSipParserSessionExpiresHdr->slNameValue),pNameValue,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
#endif			
	};
