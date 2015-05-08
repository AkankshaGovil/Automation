/***********************************************************************
 ** FUNCTION:
 **             Yacc file for  sdp body

 *********************************************************************
 **
 ** FILENAME:
 ** Sdp.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form sdp
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
#include <stdlib.h>
#include <string.h>
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "siplist.h"
#define MAXTEMP (300)

int glbAddrType=0;

#define YYPARSE_PARAM pSdpParserParam 
%}
%token CR AT SLASH VEQ DIG NETTYPE ADDRTYPE IP HOST OEQ SEQ IEQ UTF8 UEQ EEQ PEQ 
%token URICNOCOLON URICOLON URI
%token EMAIL EMAIL1 PHONE CEQ BEQ BAND TEQ ZERO START PROTO
%token REQ REPEAT DHMS ZONE DASHDIG KEQ KEY AEQ COLON BYTESTR CLEARSAFE 
%token ZEQ ALPHASAFE IPV6ADDR TOKEN BYTESTR
%token DEFAULT INPADDR LDPADDR
%start line
%%
line:protoversion 
    |originfield 
    |sessionnamefield 
    |infofield 
    |urifield 
    |emailfield 
    |phonefield 
    |repeatfield
    |connectionfield 
    |bandwidthfield 
    |timefield
    |keyfield 
	|zoneadjustments
	|error
	{
			if(*(((SipSdpParserParam *) pSdpParserParam)->pError)==E_NO_ERROR)
			*(((SipSdpParserParam *) pSdpParserParam)->pError)=E_PARSER_ERROR;

	};


protoversion:veq digits {
		 		((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
				pVersion = sip_tokenBufferDup($2);		
			};
veq:VEQ
		{
			if(((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
				pVersion!=SIP_NULL)
			{
				*(((SipSdpParserParam *) pSdpParserParam)->pError) =\
				E_PARSER_ERROR;
				YYABORT;
			}
		};
originfield:oeq userordig sessionandnet origaddr 
		{
			((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
			pOrigin->pUser = sip_tokenBufferDup($2);	
			((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
			pOrigin->dAddr = sip_tokenBufferDup($4);	

		};
sessionandnet: xsessionid xsessionver NETTYPE addrtype
	{
			SIP_S8bit* tempnet, *tempaddr,*tempid,*tempver;
			tempid = sip_tokenBufferDup($1);
			tempver = sip_tokenBufferDup($2);
			tempnet = sip_tokenBufferDup($3);
			tempaddr = sip_tokenBufferDup($4);
		
			sip_lex_Sdp_begin_uric();
			
			if(sip_initSdpOrigin(&(((SipSdpParserParam *)pSdpParserParam) \
				->pSdpMessage->pOrigin)\
				,&*(((SipSdpParserParam *) pSdpParserParam)->pError))==SipFail)
				YYABORT;
			((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
			pOrigin->pSessionid =sip_tokenBufferDup($1);
			((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
			pOrigin->pVersion = sip_tokenBufferDup($2);	
			((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
			pOrigin->pNetType = sip_tokenBufferDup($3);	
			((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
			pOrigin->pAddrType = sip_tokenBufferDup($4);
			
			free(tempid);
			free(tempver);
			free(tempaddr);
			free(tempnet);
	};
userordig: digrule
	|username;
oeq:OEQ
{
			if(((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
				pOrigin!=SIP_NULL)
			{
				*(((SipSdpParserParam *) pSdpParserParam)->pError) =\
				E_PARSER_ERROR;
				YYABORT;
			}
};
xsessionid: xdigrule;
xsessionver:xdigrule;
connectaddrtype : addrtype 
                | tn_addrtype 
							  ;
tn_addrtype : TOKEN
            | HOST
						;

addrtype: ADDRTYPE
	{
		if(strcasecmp($1.pToken,"IP4")==0)
			glbAddrType=SIP_ADDR_TYPE_IPV4;
		else
		{
			if(strcasecmp($1.pToken,"IP6")==0)
			{
				glbAddrType=SIP_ADDR_TYPE_IPV6;
			}
			else
			{
				glbAddrType=SIP_ADDR_TYPE_IP_DEF;
			}
		}
	};

origaddr: uric;
ip: IP
	{
		if(glbAddrType==SIP_ADDR_TYPE_IPV6)
		{
			glbAddrType=SIP_ADDR_TYPE_IP_DEF;
			sip_error (SIP_Minor,"Expecting IP6 Addrerss");
			*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_PARSER_ERROR;
			YYABORT;
		}
	}
	|IPV6ADDR
	{
		if (glbAddrType==SIP_ADDR_TYPE_IPV4)
		{
			glbAddrType=SIP_ADDR_TYPE_IP_DEF;
			sip_error (SIP_Minor,"Expecting IP4 Addrerss");
			*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};

sessionnamefield:seq 
	{
		if ( $1.dLength == 0 )
		{
			*(((SipSdpParserParam *) pSdpParserParam)->\
				pError) = E_PARSER_ERROR;
			YYABORT;
		}		
		((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
		pSession = sip_tokenBufferDup($1);
	};
seq: SEQ
{
			if(((SipSdpParserParam *)pSdpParserParam)->\
				pSdpMessage->pSession!=SIP_NULL)
			{
				*(((SipSdpParserParam *) pSdpParserParam)->\
				pError) = E_PARSER_ERROR;
				YYABORT;
			}
};
infofield:ieq 
	{
		((SipSdpParserParam *)pSdpParserParam)->pSdpMessage->\
		pInformation = sip_tokenBufferDup($1);
	};
ieq:IEQ
{
			if(((SipSdpParserParam *) pSdpParserParam) \
					->pSdpMessage->pInformation!=SIP_NULL)
			{
				*(((SipSdpParserParam *) pSdpParserParam)->pError)\
				= E_PARSER_ERROR;
				YYABORT;
			}
};
urifield:ueq URI
	{
		((SipSdpParserParam *) pSdpParserParam) \
			->pSdpMessage->pUri = sip_tokenBufferDup($2);
	};
ueq:UEQ
{
			if(((SipSdpParserParam *) pSdpParserParam) \
				->pSdpMessage->pUri!=SIP_NULL)
			{
				*(((SipSdpParserParam *) pSdpParserParam)->pError) \
				= E_PARSER_ERROR;
				YYABORT;
			}
};
emailfield:EEQ slEmail;
slEmail:EMAIL 
	{
		SIP_S8bit *yymail;
		yymail = sip_tokenBufferDup($1);
     	sip_listAppend(&((SipSdpParserParam *) pSdpParserParam) \
			->pSdpMessage->slEmail,yymail,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError));
	}
     |EMAIL1 
	{
		SIP_S8bit *yymail;
		yymail = sip_tokenBufferDup($1);
   		sip_listAppend(&((SipSdpParserParam *) pSdpParserParam) \
			->pSdpMessage->slEmail,yymail,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError));
	}
	|PROTO
	{
		SIP_S8bit *yymail;
		yymail = sip_tokenBufferDup($1);
   		sip_listAppend(&((SipSdpParserParam *) pSdpParserParam) \
			->pSdpMessage->slEmail,yymail,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError));
	};
phonefield:PEQ PHONE 
	{
		SIP_S8bit *yyphone;
		yyphone = sip_tokenBufferDup($2);
   		sip_listAppend(&((SipSdpParserParam *) pSdpParserParam) \
			->pSdpMessage->slPhone,yyphone,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError));
	};
connectionfield:ceq ctypes;
		
ceq:CEQ
	{
		if(((SipSdpParserParam *) pSdpParserParam)->pSdpMessage->\
			slConnection!=SIP_NULL)
		{
			*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_PARSER_ERROR;
			YYABORT;
		}

	};
ctypes:NETTYPE connectaddrtype connectaddr
	{
		if(sip_initSdpConnection(&(((SipSdpParserParam *) pSdpParserParam) \
			->pSdpMessage->slConnection),\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError))==SipFail)
			YYABORT;
		((SipSdpParserParam *) pSdpParserParam)->pSdpMessage->\
		slConnection->pNetType=sip_tokenBufferDup($1);
		((SipSdpParserParam *) pSdpParserParam)->pSdpMessage->\
		slConnection->pAddrType=sip_tokenBufferDup($2);
		((SipSdpParserParam *) pSdpParserParam)->pSdpMessage->\
		slConnection->dAddr=sip_tokenBufferDup($3);
	};
	
connectaddr:ip SLASH dig slashint
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
	   |ip SLASH dig 
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
	}
	|ip
	|HOST;
tn_addr : INPADDR 
        | LDPADDR 
	| START
	;
slashint:SLASH dig
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
bandwidthfield:BEQ BAND 
	{
		SIP_S8bit *yyband;
		yyband = sip_tokenBufferDup($2);
     	sip_listAppend(&((SipSdpParserParam *) pSdpParserParam) \
			->pSdpMessage->pBandwidth,yyband,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError));
	};
timefield:TEQ starttime stoptime 
		{
			SdpTime *ytime;
			sip_initSdpTime(&ytime, &*(((SipSdpParserParam *)\
			pSdpParserParam)->pError));
			ytime->pStart = sip_tokenBufferDup($2);
			ytime->pStop = sip_tokenBufferDup($3);
			sip_listAppend(&((SipSdpParserParam *) pSdpParserParam) \
				->pSdpMessage->slTime,ytime,\
				&*(((SipSdpParserParam *) pSdpParserParam)->pError));
		};
starttime:START
	 |ZERO;
stoptime:START
        |ZERO;
repeatfield:REQ typedtime typedtime typedtime1
	{
		SIP_U32bit siz;
		SdpTime *t;
		char *x;

		x=(SIP_S8bit *) fast_memget\
			(0,$2.dLength+$3.dLength+$4.dLength+3,(((SipSdpParserParam *)\
			pSdpParserParam)->pError));
		if(x == SIP_NULL)
			YYABORT;
		strncpy(x, $2.pToken, $2.dLength);
		x[$2.dLength] = ' ';
		strncpy(&(x[$2.dLength+1]), $3.pToken, $3.dLength);
		x[$2.dLength+$3.dLength+1] = ' ';
		strncpy(&(x[$2.dLength+$3.dLength+2]), $4.pToken, $4.dLength);
		x[$2.dLength+$3.dLength+$4.dLength+2]='\0';

		sip_listSizeOf( &((SipSdpParserParam *) pSdpParserParam)->\
		pSdpMessage->slTime, &siz,(((SipSdpParserParam *)\
		pSdpParserParam)->pError));
		sip_listGetAt(  &((SipSdpParserParam *)\
		pSdpParserParam)->pSdpMessage->slTime, siz-1,\
		(SIP_Pvoid *)&t, &*(((SipSdpParserParam *) pSdpParserParam)->pError));
		sip_listAppend( &(t->slRepeat), x, \
		(((SipSdpParserParam *) pSdpParserParam)->pError));
	};
typedtime:DHMS
	|digrule;
typedtime1:typedtime1 typedtime
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength+1,(((SipSdpParserParam *) pSdpParserParam)->pError));
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength]=' ';
		strncpy(&($$.pToken[$1.dLength+1]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength+1;
	}
  |typedtime;
zoneadjustments:ZEQ zone2 
	{	
		SIP_U32bit siz;
		char *x;
		SdpTime *t;

		x=sip_tokenBufferDup($2);
		sip_listSizeOf( &((SipSdpParserParam *) pSdpParserParam)->\
		pSdpMessage->slTime, &siz,(((SipSdpParserParam *)\
		pSdpParserParam)->pError));
		sip_listGetAt(  &((SipSdpParserParam *)\
		pSdpParserParam)->pSdpMessage->slTime, siz-1,\
		 (SIP_Pvoid*)&t, &*(((SipSdpParserParam *) pSdpParserParam)->pError));
		/* if already assigned free that memory */
		fast_memfree(BISON_MEM_ID, t->pZone, SIP_NULL);
		t->pZone = x;	
	};
zone2:zone2 zone1
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength+1,(((SipSdpParserParam *) pSdpParserParam)->pError));
		if($$.pToken == SIP_NULL)
			YYABORT;
		if(sip_listAppend(((SipSdpParserParam *) pSdpParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipSdpParserParam *) pSdpParserParam)->pError)) == SipFail)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		$$.pToken[$1.dLength] = ' ';
		strncpy(&($$.pToken[$1.dLength+1]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength+1;
	}
     | zone1;
zone1:START zonetype
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
		$$.pToken[$1.dLength] = ' ';
		strncpy(&($$.pToken[$1.dLength+1]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength+1;
	};
zonetype: DHMS
	|digrule
	|DASHDIG;
keyfield:keq keytype 
	{
		((SipSdpParserParam *) pSdpParserParam)->pSdpMessage->pKey =\
		sip_tokenBufferDup($2);
	};
keytype:KEY
       |uricolon URI
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
			strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
			$$.dLength = $1.dLength+$2.dLength;
		}
	   |CLEARSAFE
	   |token BYTESTR
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
	   |token;
token: TOKEN
	| HOST
	| START
	| ZERO
	| DIG
	| IP
	| PROTO
	| IPV6ADDR;

uricolon : URICOLON
		{
			sip_lex_Sdp_begin_uri_allow();
		};
keq:KEQ
	{
		if(((SipSdpParserParam *) pSdpParserParam)->pSdpMessage->pKey!=SIP_NULL)
		{
			*(((SipSdpParserParam *) pSdpParserParam)->pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};
digits:	digrule;		
xdigrule:digrule;
digrule:DIG
	|ZERO
	|START;
username:IP
	|ALPHASAFE;
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
	|HOST
	|digrule
	|PROTO
	|ip;
dig:	START
	|DIG;
