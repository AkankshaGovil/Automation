/***********************************************************************
 ** FUNCTION:
 **             Yacc file for attrib Media line

 *********************************************************************
 **
 ** FILENAME:
 ** Media.y
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


%pure_parser
%{
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "sipstruct.h"
#include "siplist.h"
#include "sipinit.h"
#include "portlayer.h"
#include "siptrace.h"
#define MAXTEMP (300)

int glbMaddrType=0;

#define YYPARSE_PARAM pSdpParserParam 
%}
%token MEQ CR ALPHANUM TOKEN DIG COLON SLASH PROTO INFO CONNECT1 CONNECT2 NETTYPE ADDRTYPE
%token CEQ IP HOST TTL BEQ BAND KEQ KEY PROMPT URITXT AEQ BYTESTR IPV6ADDR
%token URICNOCOLON ZERO DASH OPRTXT INPADDR LDPADDR START
%start line
%%
line:slMedia;
slMedia:slMedia media1
     |
     ;
media1:mediafield 
      |infofield 
      |connectionfield 
      |pBandwidth 
		{
			SIP_S8bit *yyband;
			yyband = sip_tokenBufferDup($1);
    	 	sip_listAppend(&((SipSdpParserParam *) pSdpParserParam) \
							->pMedia->slBandwidth,yyband,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError));
		}
      |keyfield 
		{
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pKey=\
			sip_tokenBufferDup($1);
		}
	|attrib 
      |error
		{
				if(*(((SipSdpParserParam *) pSdpParserParam)->pError)==\
					E_NO_ERROR)
			*(((SipSdpParserParam *) pSdpParserParam)->pError)=E_PARSER_ERROR;

		};

mediafield:MEQ mediatype dPort SLASH pPortNum pProtocol fmt
		{
			SIP_S8bit *temp;
                       
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pMediaValue =\
			sip_tokenBufferDup($2);
			temp = sip_tokenBufferDup($3);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->dPort=\
			(SIP_U16bit)atoi(temp);
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pPortNum =\
			(SIP_U32bit *)fast_memget(BISON_MEM_ID,sizeof(SIP_U32bit),\
			(((SipSdpParserParam *) pSdpParserParam)->pError));
			if(((SipSdpParserParam*)pSdpParserParam)->pMedia->pPortNum \
				== SIP_NULL)
				YYABORT;
			temp = sip_tokenBufferDup($5);
			*(((SipSdpParserParam*)pSdpParserParam)->pMedia->pPortNum)\
			=(SIP_U32bit)atol(temp);
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pProtocol=\
			sip_tokenBufferDup($6);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pFormat=\
			sip_tokenBufferDup($7);
		}
     |MEQ mediatype dPort pProtocol  fmt
		{
			SIP_S8bit *temp;


			((SipSdpParserParam*)pSdpParserParam)->pMedia->pMediaValue = \
			sip_tokenBufferDup($2);
			temp = sip_tokenBufferDup($3);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->dPort=\
			(SIP_U16bit)atoi(temp);
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pProtocol=\
			sip_tokenBufferDup($4);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pFormat=\
			sip_tokenBufferDup($5);
		};
mediatype:ALPHANUM
	|DIG;
dPort:DIG
	|ZERO;
pPortNum:DIG;
pProtocol:ALPHANUM 
	|PROTO 
	|HOST
	|URICNOCOLON;
fmt:fmt fmt1
	{
			$$.pToken=(SIP_S8bit *) fast_memget\
					(0,$1.dLength+$2.dLength+1,&*(((SipSdpParserParam *)\
																				 pSdpParserParam)->pError));
			if($$.pToken == SIP_NULL)
			YYABORT;
			if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
									pGCList, (SIP_Pvoid) $$.pToken,\
									(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
					YYABORT;
			strncpy($$.pToken, $1.pToken, $1.dLength);
			$$.pToken[$1.dLength] = ' ';
			strncpy(&($$.pToken[$1.dLength+1]), $2.pToken, $2.dLength);
			$$.dLength = $1.dLength+$2.dLength+1;
	}
   |fmt1 
    ;
fmt1:ALPHANUM 
    |DIG   
    |ZERO  
    |DASH  
    ;
infofield:INFO 
	{
		((SipSdpParserParam*)pSdpParserParam)->pMedia->pInformation =\
		sip_tokenBufferDup($1);
	};
connectionfield:CEQ ctypes;
ctypes:NETTYPE connectaddrtype connectaddr
	{
		SdpConnection * yconnect;

		if(sip_initSdpConnection(&yconnect,&*(((SipSdpParserParam *)\
			pSdpParserParam)->pError))==SipFail)
			YYABORT;
		yconnect->pNetType=sip_tokenBufferDup($1);
		yconnect->pAddrType=sip_tokenBufferDup($2);
		yconnect->dAddr=sip_tokenBufferDup($3);
        sip_listAppend(&(((SipSdpParserParam*)pSdpParserParam)->\
		pMedia->slConnection),yconnect,\
		(((SipSdpParserParam *) pSdpParserParam)->pError));
	};
connectaddr:ip SLASH DIG slashint
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength+$3.dLength+$4.dLength,\
		(((SipSdpParserParam *) pSdpParserParam)->pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		strncpy(&($$.pToken[$1.dLength+$2.dLength]), $3.pToken, $3.dLength);
		strncpy(&($$.pToken[$1.dLength+$2.dLength+$3.dLength]), $4.pToken,\
			$4.dLength);
		$$.dLength = $1.dLength+$2.dLength+$3.dLength+$4.dLength;
	}
	   |ip SLASH DIG 
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength+$3.dLength,(((SipSdpParserParam *)\
		pSdpParserParam)->pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		strncpy(&($$.pToken[$1.dLength+$2.dLength]), $3.pToken, $3.dLength);
		$$.dLength = $1.dLength+$2.dLength+$3.dLength;
	}
	   |ip
		 |tn_addr
		{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength\
						,(((SipSdpParserParam *) pSdpParserParam)->pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.dLength = $1.dLength;
	} ;
slashint:SLASH DIG
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
			$2.dLength,(((SipSdpParserParam *) pSdpParserParam)->pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	};
tn_addr : INPADDR 
        | LDPADDR 
	| START
	;
pBandwidth:BEQ attname BAND
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$2.dLength+\
		$3.dLength,(((SipSdpParserParam *) pSdpParserParam)->pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $2.pToken, $2.dLength);
		strncpy(&($$.pToken[$2.dLength]), $3.pToken, $3.dLength);
		$$.dLength = $2.dLength+$3.dLength;
	}; 
keyfield:KEQ KEY
		{
			$$=$2;
		} 
	|KEQ PROMPT
		{
			$$=$2;
		} 
	|KEQ URITXT 
		{
			$$=$2;
		}
	|KEQ keyval
		{
			$$=$2;	
		}; 
		
keyval: token BYTESTR
		{
			$$.pToken=(SIP_S8bit *) fast_memget\
				(0,$1.dLength+$2.dLength+2,&*(((SipSdpParserParam *)\
				pSdpParserParam)->pError));
			if($$.pToken == SIP_NULL)
				YYABORT;
			if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
				pGCList, (SIP_Pvoid) $$.pToken,(((SipSdpParserParam *)\
				pSdpParserParam)->pError)) == SipFail)
				YYABORT;
			strncpy($$.pToken, $1.pToken, $1.dLength);
			$$.pToken[$1.dLength] = ':';
			strncpy(&($$.pToken[$1.dLength+1]), $2.pToken, $2.dLength);
			$$.dLength = $1.dLength+$2.dLength+1;
		}
		| token;
token:TOKEN
	|HOST
	|PROTO
	|ALPHANUM
	|IP
	|ADDRTYPE
	|NETTYPE
	|ZERO
	|DIG
	|IPV6ADDR;

attrib: AEQ attributes 
     ;
attributes:attributes attribute 
          | attribute 
          ;
attribute:attname bytestring
	{
		SdpAttr *yattrib;

		if(sip_initSdpAttr(&yattrib,&*(((SipSdpParserParam *)\
			pSdpParserParam)->pError))==SipFail)
			YYABORT;
		yattrib->pName=sip_tokenBufferDup($1);
		yattrib->pValue=sip_tokenBufferDup($2);
		sip_listAppend(&(((SipSdpParserParam *)pSdpParserParam)->\
		pMedia->slAttr),yattrib,(((SipSdpParserParam *) \
		pSdpParserParam)->pError));
	} 
	| attname
		{
		SdpAttr *yattrib;
		

		if(sip_initSdpAttr(&yattrib,&*(((SipSdpParserParam *) \
			pSdpParserParam)->pError))==SipFail)
			YYABORT;
		yattrib->pName=sip_tokenBufferDup($1);
		sip_listAppend(&(((SipSdpParserParam *)pSdpParserParam)->\
			pMedia->slAttr),yattrib,(((SipSdpParserParam *)\
			pSdpParserParam)->pError));
	};

bytestring:BYTESTR 
        | URITXT
        | OPRTXT
	| BAND
	{
		$$.pToken = &($1.pToken[1]);
		$$.dLength = $1.dLength-1;
	};
connectaddrtype : addrtype 
                | tn_addrtype 
		;
tn_addrtype : TOKEN  
            | HOST  
	    | ALPHANUM 
	    ;
addrtype: ADDRTYPE
	{
		if(strcasecmp($1.pToken,"IP4")==0)
			glbMaddrType=4;
		else
		{
			if(strcasecmp($1.pToken,"IP6")==0)
			{
				glbMaddrType=6;
			}
			else
			{
				glbMaddrType=0;
			}
		}

	};
ip: IP
	{
		if(glbMaddrType==6)
		{
			glbMaddrType=0;
			sip_error (SIP_Minor,"Expecting IP6 Addrerss");
			*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_PARSER_ERROR;
			YYABORT;
		}
	}
	|IPV6ADDR
	{
		if (glbMaddrType==4)
		{
			glbMaddrType=0;
			sip_error (SIP_Minor,"Expecting IP4 Addrerss");
			*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_PARSER_ERROR;
			YYABORT;
		}
	}
	|HOST;
attname: TOKEN
	|HOST
	|PROTO
        |URICNOCOLON
	|ALPHANUM
	|IP
	|ADDRTYPE
	|NETTYPE
        |DIG
	|BYTESTR
        |ZERO
	|PROMPT;
