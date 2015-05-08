/***********************************************************************
 ** FUNCTION:
 **             Yacc file for key type hdr

 *********************************************************************
 **
 ** FILENAME:
 ** Key.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form key
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC       Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token RESPKEY ENCRYPTIONCOLON
%token TOKEN QSTRING COMMA EQUALS
%token IPV6ADDR SIPFLOAT
%token DEFAULT CONTENTLANGUAGE CONTENTLANGUAGEVAL
%pure_parser
%{ 
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include "siptrace.h"
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siptrace.h"
#define MAXTEMP (300)

#define YYPARSE_PARAM pHeaderParserParam
static SipRespKeyHeader *glbSipParserRespKeyHeader = SIP_NULL;
%}
%%
pHeader:respkey
		|encryption
		|contentlanguage
		|error
		{
			if(glbSipParserRespKeyHeader != SIP_NULL)
			{
				sip_freeSipRespKeyHeader(glbSipParserRespKeyHeader);
				glbSipParserRespKeyHeader = SIP_NULL;
			}
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				=E_PARSER_ERROR;
		};

contentlanguage: contentlanguagecolon contentlanguagevals;
contentlanguagecolon: CONTENTLANGUAGE;
contentlanguagevals:	contentlangval
	{

		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		SipContentLanguageHeader* glbSipParserContentLangHeader=SIP_NULL;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}			
		
		order->dType = SipHdrTypeContentLanguage;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo, (SIP_Pvoid)order, \
			&*(((SipHeaderParserParam *) pHeaderParserParam) \
			->pError))!=SipSuccess)
			YYABORT;

		if(sip_initSipContentLanguageHeader(&glbSipParserContentLangHeader,\
			 &*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) \
			 == SipFail)
			YYABORT;
		glbSipParserContentLangHeader->pLangTag = sip_tokenBufferDup($1);
	
		if(glbSipParserContentLangHeader->pLangTag == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *) pHeaderParserParam)\
			->pSipMessage->pGeneralHdr->slContentLanguageHdr), \
			glbSipParserContentLangHeader, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
	}
	|	contentlanguagevals COMMA contentlangval
	{
		SipHeaderOrderInfo *order;
		SipContentLanguageHeader* glbSipParserContentLangHeader=SIP_NULL;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)\
			->pSipMessage->slOrderInfo, &ordersize,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)\
			->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *) \
			 pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;

		if(sip_initSipContentLanguageHeader(&glbSipParserContentLangHeader,\
			 &*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 == SipFail)
			YYABORT;
		glbSipParserContentLangHeader->pLangTag = sip_tokenBufferDup($3);

		if(glbSipParserContentLangHeader->pLangTag == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *) pHeaderParserParam)\
			->pSipMessage->pGeneralHdr ->slContentLanguageHdr),\
			 glbSipParserContentLangHeader, &*(((SipHeaderParserParam *)\
			 pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

	} 
	| contentlanguagevals COMMA;
contentlangval: CONTENTLANGUAGEVAL
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
respkey:respkeycolon respkeyscheme 
	{
		if(sip_initSipRespKeyHeader( &glbSipParserRespKeyHeader,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
				YYABORT;
		glbSipParserRespKeyHeader->pKeyScheme =  sip_tokenBufferDup($2);
		if(glbSipParserRespKeyHeader->pKeyScheme==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}
	keyparams
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}		
		
		order->dType = SipHdrTypeResponseKey;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)\
			->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			 pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			u.pRequest->pRequestHdr->pRespKeyHdr = glbSipParserRespKeyHeader;
		glbSipParserRespKeyHeader = SIP_NULL;
	};
respkeyscheme: TOKEN
			|  CONTENTLANGUAGEVAL
			|	SIPFLOAT;
keyparams:	keyparams COMMA keyparam
		|keyparam 
		|;
keyparam:	respkeyscheme EQUALS alltoken
	{
		SipParam *kparam;
		SIP_S8bit *yykparam;
		
		if(sip_initSipParam(&kparam,  \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;

		kparam->pName = sip_tokenBufferDup($1);
		if(kparam->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	 	yykparam = sip_tokenBufferDup($3);
		
		if(sip_listAppend(&kparam->slValue,yykparam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		
		if(sip_listAppend(&(glbSipParserRespKeyHeader->slParam),\
			(SIP_Pvoid)kparam, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) !=SipSuccess)
			YYABORT;
	}
	| respkeyscheme
	{
		SipParam *kparam;
		
		if(sip_initSipParam(&kparam,  \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;

		kparam->pName = sip_tokenBufferDup($1);
		if(kparam->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		
		if(sip_listAppend(&(glbSipParserRespKeyHeader->slParam),\
			(SIP_Pvoid)kparam,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
respkeycolon:RESPKEY
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)\
			->pSipMessage->dType != SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
				 E_NO_ERROR;
			YYABORT;
		}
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestHdr->pRespKeyHdr\
			!=SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one Response Key header ");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
			E_PARSER_ERROR;
			YYABORT;
		}
	};
genvalue: IPV6ADDR 
		|alltoken;

alltoken:	TOKEN
	|	QSTRING
	| SIPFLOAT
	| CONTENTLANGUAGEVAL;
encryption: encryptioncolon encscheme  
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		if(sip_initSipEncryptionHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pEncryptionHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)\
			->pError))!=SipSuccess)
				YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pEncryptionHdr->pScheme =  \
			sip_tokenBufferDup($2);
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage\
			->pGeneralHdr->pEncryptionHdr->pScheme\
			==SIP_NULL)
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
		
		order->dType = SipHdrTypeEncryption;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *) pHeaderParserParam) \
			->pError))!=SipSuccess)
			YYABORT;
	}
	encparams;
encryptioncolon: ENCRYPTIONCOLON
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pEncryptionHdr!=SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one Encryption header ");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_PARSER_ERROR;
			YYABORT;
		}
	};
encscheme:	TOKEN
		|   CONTENTLANGUAGEVAL
		|	SIPFLOAT;
encparams: encparam 
		|  encparam COMMA encparams
		|;
encparam:	encscheme EQUALS genvalue
		{
			SipParam *eparam;
			SIP_S8bit *yyeparam;
			
			if(sip_initSipParam(&eparam,&*(((SipHeaderParserParam *)\
				 pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
			eparam->pName = sip_tokenBufferDup($1);

			yyeparam = sip_tokenBufferDup($3);
		
			if(sip_listAppend(&eparam->slValue,yyeparam,\
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
				YYABORT;
			if(sip_listAppend(&(((SipHeaderParserParam *) pHeaderParserParam)\
				->pSipMessage->pGeneralHdr->\
				pEncryptionHdr->slParam),\
				(SIP_Pvoid)eparam,&*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		}
		|	encscheme
		{
			SipParam *eparam;
			
			if(sip_initSipParam(&eparam,&*(((SipHeaderParserParam *)\
				 pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
			eparam->pName = sip_tokenBufferDup($1);
			if(sip_listAppend(&(((SipHeaderParserParam *) \
				pHeaderParserParam)->pSipMessage->pGeneralHdr->\
				pEncryptionHdr->slParam),\
				(SIP_Pvoid)eparam,&*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;

		};
