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
%token MEQ CR ALPHANUM TOKEN DIG COLON SLASH PROTO INFO CONNECT1 CONNECT2 NETTYPE ADDRTYPE  H323CPROTO DOLOR
%token CEQ IP HOST TTL BEQ BAND KEQ KEY PROMPT URITXT AEQ BYTESTR IPV6ADDR AAL1TRANS AAL2TRANS AAL5TRANS RTPTRANS DASH
%token ATMNETTYPE ATMADDRTYPE URICNOCOLON ZERO
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

mediafield:MEQ mediatype pProtocol pProtocolfmtlist
		{
#ifndef SIP_ATM
			*(((SipSdpParserParam *) pSdpParserParam)->pError) =\
			E_PARSER_ERROR;
			YYABORT;
#else
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pMediaValue = \
			sip_tokenBufferDup($2);
			((SipSdpParserParam*)pSdpParserParam)->pMedia->pVirtualCID = \
			sip_tokenBufferDup($3);
#endif
		}
		|MEQ mediatype dPort SLASH pPortNum pProtocol fmt
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
          |MEQ mediatype dPort pProtocol fmt
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
pProtocolfmtlist: pProtocolfmtlist pProtocolfmt
		 | pProtocolfmt;
pProtocolfmt:pATMProtocol fmt
		{
#ifndef SIP_ATM
			*(((SipSdpParserParam *) pSdpParserParam)->pError) =\
			E_PARSER_ERROR;
			YYABORT;
#else
			int index;		
	 		SipNameValuePair *vparam; 
			
			sip_initSipNameValuePair(&vparam,&*(((SipSdpParserParam *) pSdpParserParam)->pError));

	 		if ( vparam == SIP_NULL )
	 		{
				*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_NO_MEM;
				YYABORT;
	 		}

			index=0;
			while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				index++;
			$1.pToken = &($1.pToken[index]);
			$1.dLength -= index;
	
			vparam->pName = sip_tokenBufferDup($1);
			if(vparam->pName==SIP_NULL)
			{
				*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_NO_MEM;
				sip_freeSipNameValuePair(vparam);	
				YYABORT;
			}
			vparam->pValue = sip_tokenBufferDup($2);
			if(vparam->pValue==SIP_NULL)
			{
				*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_NO_MEM;
				free(vparam->pName);
				sip_freeSipNameValuePair(vparam);	
				
				YYABORT;
			}
		        sip_listAppend(&(((SipSdpParserParam*)pSdpParserParam)->\
				pMedia->slProtofmt),vparam,\
				(((SipSdpParserParam *) pSdpParserParam)->pError));
#endif
	
		};
pATMProtocol: AAL1TRANS 
		|AAL2TRANS
		|AAL5TRANS
		|RTPTRANS
		|H323CPROTO
		|PROTO
		|DASH;
dPort:DIG
	|ZERO;
pPortNum:DIG;
pProtocol:ALPHANUM
	|PROTO
 	|DASH
 	|DOLOR
	|RTPTRANS
	|H323CPROTO
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
   |DASH;
fmt1:ALPHANUM
    |DIG
    |ZERO
    |ATMNETTYPE;
infofield:INFO 
	{
		((SipSdpParserParam*)pSdpParserParam)->pMedia->pInformation =\
		sip_tokenBufferDup($1);
	};
connectionfield:CEQ ctypes;
ctypes:NETTYPE addrtype connectaddr
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
	}
	|atmnettype atmaddrtype atmconnectaddr
	{
		SdpConnection *yconnect;
#ifndef SIP_ATM
		*(((SipSdpParserParam *) pSdpParserParam)->pError) =\
		E_PARSER_ERROR;
		YYABORT;
#endif

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
	   |ip;
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
		
attrib: AEQ attname bytestring
	{
		SdpAttr *yattrib;

		if(sip_initSdpAttr(&yattrib,&*(((SipSdpParserParam *)\
			pSdpParserParam)->pError))==SipFail)
			YYABORT;
		yattrib->pName=sip_tokenBufferDup($2);
		yattrib->pValue=sip_tokenBufferDup($3);
		sip_listAppend(&(((SipSdpParserParam *)pSdpParserParam)->\
		pMedia->slAttr),yattrib,(((SipSdpParserParam *) \
		pSdpParserParam)->pError));
	}
	|AEQ attname
	{
		SdpAttr *yattrib;

		if(sip_initSdpAttr(&yattrib,&*(((SipSdpParserParam *) \
			pSdpParserParam)->pError))==SipFail)
			YYABORT;
		yattrib->pName=sip_tokenBufferDup($2);
		sip_listAppend(&(((SipSdpParserParam *)pSdpParserParam)->\
			pMedia->slAttr),yattrib,(((SipSdpParserParam *)\
			pSdpParserParam)->pError));
	};
bytestring:BYTESTR
	| BAND
	{
		$$.pToken = &($1.pToken[1]);
		$$.dLength = $1.dLength-1;
	};
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
	|ALPHANUM
	|IP
	|ADDRTYPE
	|NETTYPE
	|PROMPT;
atmaddrtype: ATMADDRTYPE
	   |atmnettype
	   |addrtype;
atmconnectaddr: uric
		|atmnettype;
uric: uric uricnocolon
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.\
			dLength,&*(((SipSdpParserParam *) pSdpParserParam)->\
			pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipSdpParserParam *) \
			 pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	}
	|uric COLON	
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.\
			dLength,&*(((SipSdpParserParam *) pSdpParserParam)->\
			pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipSdpParserParam *) \
			 pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	}
	|uricnocolon 
	|COLON; 
uricnocolon: URICNOCOLON
	|PROTO
	|digrule
	|ip
	|TOKEN
	|BYTESTR
	|ALPHANUM;
digrule:DIG
	|ZERO;
atmnettype:ATMNETTYPE
	|DOLOR
	|DASH;
