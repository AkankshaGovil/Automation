/***********************************************************************
 ** FUNCTION:
 **             Yacc file for mime headers
 ********************************************************************
 **
 ** FILENAME:
 ** Mime.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form Mime Headers
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 11/02/2000 Preethy.R                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token TOKEN SLASH SEMICOLON EQUALS COMMA DQUOTE 
%token QSTRING SIPTEXT LANGLE RANGLE ADDRSPEC CONTENTDISPOSITION
%token CONTENTTYPE CONTENTTRANSENCODING CONTENTDESCRIPTION CONTENTID
%token CRLF BODYSEPARATOR HEADERNAME HEADERBODYNOCOLON COLON
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

#define YYPARSE_PARAM pMimeParserParam
%}
%%
/* This Yacc file parsers Mime Headers - Content-Type,Content-Transfer-Encoding,
Content-Description, Content-Id */

mimebody:	headers BODYSEPARATOR
		| headers CRLF
		{
			YYABORT;
		}
		|error
		{
			if(*(((SipMimeParserParam *) pMimeParserParam)->pError)==E_NO_ERROR)
			*(((SipMimeParserParam *) pMimeParserParam)->pError)=E_PARSER_ERROR;
	
		};
headers: headers CRLF header
		|header;
header:contenttype
		|contentdisposition
		|contenttransencoding
		|contentdescription
		|contentid
		|unknownheader;

/* rule to parse mime content Type Header to Type/subtype and parameters*/
contentdisposition: contentdispcolon contentdispfield;

contentdispcolon:CONTENTDISPOSITION
	{
		SipMimeHeader *mimehdr;

		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		if(mimehdr->pContentDisposition!=SIP_NULL)
		{
			sip_error (SIP_Minor,\
			 "There can only be one content length header in one Mime section");
			*(((SipMimeParserParam *) pMimeParserParam)->pError) \
			= E_PARSER_ERROR;
			YYABORT;
		}
		if(sip_initSipContentDispositionHeader(&mimehdr->pContentDisposition,\
			(((SipMimeParserParam *) pMimeParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
	};
contentdispfield: contentdispdisptype
	|	contentdispdisptype SEMICOLON contentdispparams;
contentdispdisptype:	token	
	{
		SipMimeHeader *mimehdr;

		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		mimehdr->pContentDisposition->pDispType=sip_tokenBufferDup($1);
		if( mimehdr->pContentDisposition->pDispType == SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
contentdispparams: contentdispparam
	| contentdispparams SEMICOLON contentdispparam;
contentdispparam: token
	{
		SipParam *pParam;
		SipMimeHeader *mimehdr;

		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		
		if(sip_initSipParam(&pParam,  
			(((SipMimeParserParam *) pMimeParserParam)->pError))\
			 == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(mimehdr->pContentDisposition->slParam),\
			pParam,  (((SipMimeParserParam *) pMimeParserParam)->\
			pError)) == SipFail)
			YYABORT;
	}
	| token EQUALS value
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		SipMimeHeader *mimehdr;

		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		
		if(sip_initSipParam(&pParam,  (((SipMimeParserParam *)\
			pMimeParserParam)->pError)) == SipFail)
			YYABORT;
		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipMimeParserParam *) pMimeParserParam)->pError))==SipFail)
			YYABORT;

		if(sip_listAppend(&(mimehdr->pContentDisposition->slParam),\
			pParam,(((SipMimeParserParam *) pMimeParserParam)->\
			pError)) == SipFail)
			YYABORT;
	};

contenttype:contenttypecolon mediatype;
mediatype:	type SLASH subtype parameters 
	{
		SipMimeHeader *mimehdr;

		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		mimehdr->pContentType->pMediaType = (SIP_S8bit *) fast_memget(\
		BISON_MEM_ID, $1.dLength+$3.dLength+2, SIP_NULL); 
		if(mimehdr->pContentType->pMediaType==SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(mimehdr->pContentType->pMediaType, $1.pToken, $1.dLength);
		mimehdr->pContentType->pMediaType[$1.dLength] = '/';
		strncpy(&(mimehdr->pContentType->pMediaType[$1.dLength+1]), $3.pToken,\
			$3.dLength);
		mimehdr->pContentType->pMediaType[$1.dLength+$3.dLength+1] = '\0';
	}; 
type:	token;		
subtype:token;		
contenttypecolon: CONTENTTYPE
	{
		SipMimeHeader *mimehdr;

		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		if(mimehdr->pContentType!=SIP_NULL)
		{
			sip_error (SIP_Minor,\
			 "There can only be one content length header in one Mime section");
			*(((SipMimeParserParam *) pMimeParserParam)->pError) \
			= E_PARSER_ERROR;
			YYABORT;
		}
		if(sip_initSipContentTypeHeader(&mimehdr->pContentType,\
			&*(((SipMimeParserParam *) pMimeParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
	};
parameters: parameters SEMICOLON parameter
		| ;
/* separate parameters in content type Header and add to siplist */
parameter: token EQUALS value
	{
		SipMimeHeader *mimehdr;
		SipParam *param;
		SIP_S8bit *pValue;
		
		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;
		if(sip_initSipParam(&param,&*(((SipMimeParserParam *)\
			pMimeParserParam)->pError))==SipFail)
			YYABORT;
		param->pName = sip_tokenBufferDup($1);
		if(param->pName == SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		pValue = sip_tokenBufferDup($3);
		if(pValue == SIP_NULL) 
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(param->slValue),pValue,\
			(((SipMimeParserParam *) pMimeParserParam)->pError))== SipFail)
			YYABORT;
		if(sip_listAppend (&(mimehdr->pContentType->slParams), \
			(SIP_Pvoid)param, &*(((SipMimeParserParam *)\
			pMimeParserParam)->pError))==SipFail)
			YYABORT;
	};
value: token 
       |QSTRING;
token:TOKEN;	
/* rule for Content-Transfer Encoding */
contenttransencoding: contenttransencodingcolon token
	{
		SipMimeHeader *mimehdr;
		
		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		if(mimehdr->pContentTransEncoding!=SIP_NULL)
		{
			sip_error (SIP_Minor,\
 "There can only be one content transfer encoding Header in one Mime section");
			*(((SipMimeParserParam *) pMimeParserParam)->pError) \
			= E_PARSER_ERROR;
			YYABORT;
		}
		mimehdr->pContentTransEncoding = sip_tokenBufferDup($2);
		if(mimehdr->pContentTransEncoding==SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
/* Rule for Content-Description */
contentdescription:contentdescriptioncolon token
	{
		SipMimeHeader *mimehdr;
		
		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		if(mimehdr->pContentDescription!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one content description \
					Header in one Mime section");
			YYABORT;
		}
		mimehdr->pContentDescription = sip_tokenBufferDup($2);
		if(mimehdr->pContentDescription==SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
/* Rule for Content-Id: <addrspec> */
contentid: contentidcolon LANGLE addrspec RANGLE
	{
		SipMimeHeader *mimehdr;

		mimehdr=((SipMimeParserParam*)pMimeParserParam)->pMimeHeader;	
		if(mimehdr->pContentId!=SIP_NULL)
			YYABORT;
		mimehdr->pContentId = (SIP_S8bit *) fast_memget(BISON_MEM_ID,\
		$3.dLength+3,SIP_NULL);
		if(mimehdr->pContentId==SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(mimehdr->pContentId, "<", 1);
		strncpy(&(mimehdr->pContentId[1]), $3.pToken, $3.dLength);
		strncpy(&(mimehdr->pContentId[$3.dLength+1]), ">", 1);
		mimehdr->pContentId[$3.dLength+2] = '\0';
	};
addrspec: TOKEN; 
contenttransencodingcolon: CONTENTTRANSENCODING;
contentdescriptioncolon: CONTENTDESCRIPTION;
contentidcolon: CONTENTID;
unknownheader: HEADERNAME headerbody 
	{
		SIP_S8bit *tempstr;
		tempstr = (SIP_S8bit *) fast_memget(BISON_MEM_ID, $1.dLength+\
		$2.dLength+1,SIP_NULL); 
		if(tempstr==SIP_NULL)
		{
			*(((SipMimeParserParam *) pMimeParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(tempstr, $1.pToken, $1.dLength);
		strncpy(&(tempstr[$1.dLength]), $2.pToken, $2.dLength);
		tempstr[$1.dLength+$2.dLength] = '\0';

		if(sip_listAppend(&(((SipMimeParserParam*)pMimeParserParam)->\
			pMimeHeader->slAdditionalMimeHeaders),(SIP_Pvoid)tempstr, \
			(((SipMimeParserParam *) pMimeParserParam)->pError))==SipFail)
			YYABORT;
	};
headerbody:	headerbody headerbodyfields
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength,&*(((SipMimeParserParam *) pMimeParserParam)->pError));
		if(sip_listAppend(((SipMimeParserParam *) pMimeParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipMimeParserParam *) pMimeParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	}
	|headerbodyfields;
headerbodyfields: HEADERBODYNOCOLON
		|HEADERNAME
		|TOKEN
		|COLON;
