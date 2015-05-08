
/***********************************************************************
 ** FUNCTION:
 **             Yacc file for status line  headers

 *********************************************************************
 **
 ** FILENAME:
 ** Statusline.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form statusline
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token SIPSLASH TEXTUTF8NOSLASHDIGITS SLASH 
%token DIGIT3 SIPFLOAT DIGIT LINEEND
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

#define MAXTEMP (300)

#define YYPARSE_PARAM pHeaderParserParam
%}
%%
statusline:	statuslineparam
			| statuslineparam LINEEND;

statuslineparam :sipversion statuscode reasonphrase
	{
		SIP_S8bit *temp;
		int numvalcodes = 45;
		int validcodes[] = {100, 180, 181, 182, 183,
		200, 300, 301, 302, 303, 305, 380, 400,
		401, 402, 403, 404, 405, 406, 407, 408,
		409, 410, 411, 413, 414, 415, 420, 480,
		481, 482, 483, 484, 485, 486, 487, 500, 501, 
		502, 503, 504, 505, 600, 603, 604, 606, 2000, 2000 };
		int i,statuscode;

		if(sip_initSipStatusLine(&((SipHeaderParserParam *)  \
			pHeaderParserParam)->pSipMessage->u.pResponse->\
			pStatusLine, (((SipHeaderParserParam *) pHeaderParserParam) \
				->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage \
			->u.pResponse->pStatusLine->pVersion =  \
			sip_tokenBufferDup($1);
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pResponse->pStatusLine->pVersion==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =  \
				E_NO_MEM;
			YYABORT;
		}
		
		if ($3.dLength)
		{	
			((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
				u.pResponse->pStatusLine->pReason =  \
				sip_tokenBufferDup($3);
			
			if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
				u.pResponse->pStatusLine->pReason==SIP_NULL)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					= E_NO_MEM;
				YYABORT;
			}
		}
		
		temp = sip_tokenBufferDup($2);
		statuscode = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		for(i=0;(i<=numvalcodes)&&(validcodes[i]!=statuscode);i++)
			{}
 		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pResponse->pStatusLine->dCodeNum = statuscode;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage \
			->u.pResponse->pStatusLine->code =  \
			(en_StatusCode) validcodes[i];
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType \
			 = SipMessageResponse;
	}
	|error
	{
		if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)== \
			E_NO_ERROR)
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= \
				E_PARSER_ERROR;
	
	};
sipversion:	SIPSLASH SIPFLOAT
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$2.dLength+1,(((SipHeaderParserParam *)  \
				pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam) \
			->pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))  \
				== SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	};
statuscode:	DIGIT3;

reasonphrase:textutf8
			|
			{
				$$.dLength=0;
			};	
			
textutf8:	textutf8 utf8char
	{
		$$.pToken=(SIP_S8bit *) fast_memget\
			(0,$1.dLength+$2.dLength+1,(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)\
			->pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				 == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	}
	|utf8char;

utf8char:TEXTUTF8NOSLASHDIGITS	
		|DIGIT		
		|SIPSLASH		
		|DIGIT3			
		|SIPFLOAT			
		|SLASH;		
