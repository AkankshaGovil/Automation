
/***********************************************************************
 ** FUNCTION:
 **             Yacc file for Request Line

 *********************************************************************
 **
 ** FILENAME:
 ** Reqline.y
 **
 ** DESCRIPTION:
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token SIPSLASH TOKEN URICNOCOLON
%token COLON USERINFOAT AT AMPERSAND EQUALS SEMICOLON
%token DIGITS SIPFLOAT COMMA SEMPARAMTOKEN SEMPARAMVALUE
%token SIPCOLON SIPSCOLON SIPSLASHFLOAT
%token USERINFOAT SEMTOKEN IPV6ADDR
%token QSTRING QUESTION LINEEND DEFAULT QSTRING1
%token DEFAULT
%pure_parser
%{ 
#include "sipparserinc.h"
#include <stdlib.h>
#include "sipdecodeintrnl.h"
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"

#define MAXTEMP (300)

static SipAddrSpec *		glbSipParserAddrSpec=SIP_NULL;
#define YYPARSE_PARAM pHeaderParserParam
%}
%%
requestline:	method addrspec sipversion LINEEND
	{
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestLine->pVersion = \
			sip_tokenBufferDup($3);
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestLine->pVersion==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType = \
			SipMessageRequest;
	}
	|error
	{
		if(glbSipParserAddrSpec!= SIP_NULL)
		{
			sip_freeSipAddrSpec(glbSipParserAddrSpec);
			glbSipParserAddrSpec=SIP_NULL;
		}

		if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)==\
			E_NO_ERROR)
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=\
				E_PARSER_ERROR;
	};
method:token
	{
	
		if(sip_initSipReqLine(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->u.pRequest->\
			pRequestLine), &*(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestLine->pMethod=\
			sip_tokenBufferDup($1);
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestLine->pMethod==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_MEM;
			YYABORT;
		}
	};
sipversion:	SIPSLASHFLOAT;

token:	TOKEN 		
		|float;
addrspec:sipurl	
	{
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestLine->pRequestUri = 
			glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	}
		|uric
	{
		if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrReqUri, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))!= \
			SipSuccess)
			YYABORT;
		glbSipParserAddrSpec->u.pUri = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pUri==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			u.pRequest->pRequestLine->pRequestUri =  \
			glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	};
uric:uric uricnocolon
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID, \
			$1.dLength+$2.dLength,&*(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
		|uric COLON	
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+ \
			$2.dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == \
			SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
|uricnocolon
| COLON;
uricnocolon:TOKEN	
		|digits			
		|URICNOCOLON	
		|USERINFOAT		
		|AMPERSAND 	
		|AT	 	
		|EQUALS 	
		|SEMICOLON 	
		|QUESTION
		|SEMTOKEN
		|SEMPARAMTOKEN
		|QSTRING1
		|QSTRING
		|SIPFLOAT
		|IPV6ADDR
		|COMMA
		|SEMPARAMVALUE;
digits:		DIGITS;		
float:		DIGITS		
			|SIPFLOAT;		
sipurl:		sipcolon  userinfoat hostport urlparams headers
		|sipcolon  hostport urlparams headers;
userinfoat:	USERINFOAT
	{
		char *pUser = SIP_NULL;
		char *pPasswd = SIP_NULL;
		int i,j,k;

		/* copy to temp and remove terminating @ */
		/* Copy stuff from beginning till ":" or END to pUser */
		for(i=0;($1.pToken[i]!=':')&&($1.pToken[i]!='@');i++);
				pUser = (SIP_S8bit*)fast_memget(BISON_MEM_ID,i+1,SIP_NULL);
		if(pUser==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			 = E_NO_MEM;
			YYABORT;
		}
		for(k=0;k<i;k++)	
			pUser[k] = $1.pToken[k];
		pUser[i] = '\0';

		glbSipParserAddrSpec->u.pSipUrl->pUser = pUser;

		/* Copy remaining stuff to passwd */
		j=0;
		if($1.pToken[i]!='@')
		{
			int a;
			a=i;
			for(;$1.pToken[i+1]!='@';i++,j++);
			if(j>0)
			{
				pPasswd= (SIP_S8bit*)fast_memget(BISON_MEM_ID,j+1,SIP_NULL);
				if(pPasswd ==SIP_NULL)
				{
					*(((SipHeaderParserParam *)\
					 pHeaderParserParam)->pError) =  \
					E_NO_MEM;
					YYABORT;
				}
				for(k=0;k<j;k++,a++)
					pPasswd[k] = $1.pToken[a+1];
				pPasswd[j]='\0';
			}
		}
		if(j==0)
		{	
			pPasswd =SIP_NULL;
		}
		glbSipParserAddrSpec->u.pSipUrl->pPassword = pPasswd;
	};
sipcolon:SIPCOLON
	{
		if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrSipUri, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))!= \
			SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))!= \
			SipSuccess)
				YYABORT;
	}
	| SIPSCOLON
	{
		if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrSipSUri, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))!= \
			SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))!= \
			SipSuccess)
				YYABORT;
	};
hostport:	host COLON port
	{
		SIP_S8bit *temp;

		glbSipParserAddrSpec->u.pSipUrl->pHost = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		glbSipParserAddrSpec->u.pSipUrl->dPort = (SIP_U16bit *) 
			fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit), \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(glbSipParserAddrSpec->u.pSipUrl->dPort==SIP_NULL)
			YYABORT;
		temp = sip_tokenBufferDup($3);
		 *(glbSipParserAddrSpec->u.pSipUrl->dPort) = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	}
		|host
	{
		glbSipParserAddrSpec->u.pSipUrl->pHost = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
host:	token
	|ipv6;
ipv6:IPV6ADDR;
port:		digits;
urlparams: 	urlparams urlparam
		|;
urlparam: semparamtokens EQUALS newalltoken 
	{
		SipParam *param; 
		int index;		
		SIP_S8bit *yyuparam;

		if(sip_initSipParam(&param,&*(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID,  \
			$1.dLength-index+1, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		yyuparam=sip_tokenBufferDup($3);	

		if(sip_listAppend(&param->slValue,yyuparam, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
			(SIP_Pvoid)param,(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	| semparamtokens
	{
		SipParam *param; 
		int index;		

		if(sip_initSipParam(&param,(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
			$1.dLength-index+1, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
			(SIP_Pvoid)param,&*(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};

headers:	QUESTION uric
	{
		glbSipParserAddrSpec->u.pSipUrl->pHeader = sip_tokenBufferDup($2);
		if(glbSipParserAddrSpec->u.pSipUrl->pHeader==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}
		|;

semparamtokens:	SEMPARAMTOKEN 
			|SEMTOKEN; 
newalltoken:SEMPARAMVALUE
		| token
		| QSTRING
		| IPV6ADDR;
