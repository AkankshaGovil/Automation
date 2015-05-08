/***********************************************************************
 ** FUNCTION:
 **             Yacc file for attrib sdp line

 *********************************************************************
 **
 ** FILENAME:
 ** Attrib.y
 **
 ** DESCRIPTION:
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99    Ashok Roy                   	      Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

%token AEQ CR COLON BYTESTR BYTESTRNOCOLON
%token DEFAULT
%start line
%pure_parser
%{
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "sipstruct.h"
#include "siplist.h"
#include "siptrace.h"
#define MAXTEMP (300)

#define YYPARSE_PARAM pSdpParserParam 
%}

%%
line:line lineattrib
    |lineattrib
	|error
	{
		if(*(((SipSdpParserParam *) pSdpParserParam)->pError)==E_NO_ERROR)
			*(((SipSdpParserParam *) pSdpParserParam)->pError)=E_PARSER_ERROR;
	}
    ;
lineattrib:attributefield; 
attributefield: AEQ BYTESTRNOCOLON BYTESTR
		{
			SdpAttr *yattrib;

			if(sip_initSdpAttr(&yattrib,&*(((SipSdpParserParam *)\
				pSdpParserParam)->pError))==SipFail)
				YYABORT;
			yattrib->pName = sip_tokenBufferDup($2);
			yattrib->pValue= sip_tokenBufferDup($3);
			sip_listAppend(&(((SipSdpParserParam *)pSdpParserParam)->\
			pSdpMessage->slAttr),yattrib,\
				&*(((SipSdpParserParam *) pSdpParserParam)->pError));
		}
		|AEQ BYTESTRNOCOLON
		{
			SdpAttr *yattrib;

			if(sip_initSdpAttr(&yattrib,&*(((SipSdpParserParam *)\
				pSdpParserParam)->pError))==SipFail)
				YYABORT;
			yattrib->pName = sip_tokenBufferDup($2);
			sip_listAppend(&(((SipSdpParserParam *)(pSdpParserParam))->\
			pSdpMessage->slAttr),yattrib,(((SipSdpParserParam *)\
				pSdpParserParam)->pError));
		};
