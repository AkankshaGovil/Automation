/***********************************************************************
 ** FUNCTION:
 **             Yacc file for From/To format Headers

 *********************************************************************
 **
 ** FILENAME:
 ** Fromto.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form From/to
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

%token FROM TO ROUTE RECORDROUTE COMMA COLON SEMICOLON RANGLE TAG
%token EQUALS PLAINSIPCOLON SIPCOLON SIPSCOLON DIGITS SEMPARAMTOKEN SEMPARAMVALUE
%token QSTRING QSTRING1 TOKEN 
%token URICNOCOLON QUESTION
%token DISPNAME USERINFOAT SEMTOKEN IPV6ADDR REFERTO REFERBY
%token DEFAULT
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
#include "sipfree.h"
#include "siptrace.h"

#define MAXTEMP (300)

static SipParam *		glbSipParserSipParam=SIP_NULL;
static SipAddrSpec *glbSipParserAddrSpec=SIP_NULL;
static SipToHeader *glbSipParserToHeader = SIP_NULL;
extern int glbSipParserToAllowSipColon;
#define YYPARSE_PARAM pHeaderParserParam
%}
%%
header:to
		|error
		{
			if(glbSipParserAddrSpec != SIP_NULL)
			{
				sip_freeSipAddrSpec(glbSipParserAddrSpec);
				glbSipParserAddrSpec=SIP_NULL;
			}
			if(glbSipParserSipParam != SIP_NULL)
			{
				sip_freeSipParam(glbSipParserSipParam);
				glbSipParserSipParam = SIP_NULL;
			}
			if(glbSipParserToHeader != SIP_NULL)
			{
				sip_freeSipToHeader(glbSipParserToHeader);
				glbSipParserToHeader = SIP_NULL;
			}
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					=E_PARSER_ERROR;
		};
to:		tocolon
	{ 
		if(sip_initSipToHeader( &glbSipParserToHeader, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	}
		tofields
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			pGeneralHdr->pToHdr = glbSipParserToHeader;
		glbSipParserToHeader = SIP_NULL;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeTo;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo, (SIP_Pvoid)order,  \
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
tofields:tonameaddr toaddrparams 
		|tonameaddr;
tonameaddr:	addrspecnoparam
	{
		glbSipParserToHeader->pAddrSpec =  glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	}
		|dispname addrspec RANGLE
	{
		glbSipParserToHeader->pDispName =  sip_tokenBufferDup($1);
		if( glbSipParserToHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_MEM;
			YYABORT;
		}
		glbSipParserToHeader->pAddrSpec =  glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	};
dispname:	DISPNAME
	{
		int cnt,cnt1;
		$$ = $1;

		cnt = $$.dLength-2;
		for(;(cnt>=0)&&(isspace((int)$$.pToken[cnt]));cnt--);
		$$.dLength = cnt+1;
		/* Now remove leading spaces */
		for(cnt1=0;(cnt1<=cnt)&&(isspace((int)$$.pToken[cnt1]));cnt1++)
			{}
		$$.pToken = &($$.pToken[cnt1]);
		$$.dLength-=cnt1;
	};
addrspec:sipurl
		|uricval;
addrspecnoparam:sipurlnoparam
		|uricvalnoparam;
uricvalnoparam:uricnoparam
	{
		if(sip_initSipAddrSpec(&glbSipParserAddrSpec,SipAddrReqUri, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserAddrSpec->u.pUri = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pUri==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
				= E_NO_MEM;
			YYABORT;
		}
	};
uricval:uric
	{
		if(sip_initSipAddrSpec(&glbSipParserAddrSpec,SipAddrReqUri, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserAddrSpec->u.pUri = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pUri==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
				= E_NO_MEM;
			YYABORT;
		}
	};
sipurlnoparam:	sipcolon  userinfoat hostport
		|sipcolon  hostport;
sipurl:sipcolon  userinfoat hostport urlparams headers
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
sipcolon:	SIPCOLON
	{
		if(sip_initSipAddrSpec(&glbSipParserAddrSpec,SipAddrSipUri, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
				YYABORT;
	}
	| SIPSCOLON
	{
		if(sip_initSipAddrSpec(&glbSipParserAddrSpec,SipAddrSipSUri, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
				YYABORT;
	};
hostport:	host COLON port
	{
		SIP_S8bit *temp;

		glbSipParserAddrSpec->u.pSipUrl->pHost = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =  \
				E_NO_MEM;
			YYABORT;
		}
		glbSipParserAddrSpec->u.pSipUrl->dPort = (SIP_U16bit *)  \
			fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit),\
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
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =  \
				E_NO_MEM;
			YYABORT;
		}
	};
host:		token
	|ipv6;
ipv6:IPV6ADDR;
port:		digits;
urlparams: 	urlparams urlparam
		|;
urlparam:semparamtokens EQUALS newalltoken 
	{
		SipParam *param; 
		int index;		
		SIP_S8bit *yyuparam;
 
		if(sip_initSipParam(& param,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
			$1.dLength-index+1, (((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		yyuparam=sip_tokenBufferDup($3);	
		if(sip_listAppend(&param->slValue,(SIP_Pvoid)yyuparam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
		YYABORT;

		if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
			(SIP_Pvoid)param,&*(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|semparamtokens
	{
		SipParam *param; 
		int index;		
 
		if(sip_initSipParam(& param,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
			$1.dLength-index+1, &*(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
			(SIP_Pvoid)param,&*(((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
semtoken:SEMTOKEN;
semparamtokens:	SEMPARAMTOKEN 
			|SEMTOKEN 
			|TAG;
		
headers:	QUESTION 
			{
				glbSipParserToAllowSipColon=1;
			}	
			uricdummy	
			|{ 
				glbSipParserToAllowSipColon=0;
			};

uricdummy:	uric
	{
		glbSipParserAddrSpec->u.pSipUrl->pHeader = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHeader==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_MEM;
			YYABORT;
		}
		glbSipParserToAllowSipColon=0;
	};
		
toaddrparams:	toaddrparams tag
	{ 
		SIP_S8bit *tmp;
		tmp = sip_tokenBufferDup($2);
		if(tmp==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( \
			&(glbSipParserToHeader->slTag),  tmp,(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
		|toaddrparams extparam
	{
		if(sip_listAppend( \
			&(glbSipParserToHeader->slParam), (SIP_Pvoid)glbSipParserSipParam, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;		
			glbSipParserSipParam=SIP_NULL;
		
	}
		|extparam
	{
		if(sip_listAppend( \
			&(glbSipParserToHeader->slParam), (SIP_Pvoid)glbSipParserSipParam, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
			glbSipParserSipParam=SIP_NULL;
	}
		|tag
	{ 
		SIP_S8bit *tmp;
		tmp = sip_tokenBufferDup($1);
		if(tmp==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
				E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( \
			&(glbSipParserToHeader->slTag), tmp,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
tag:TAG EQUALS token 	
	{ 
		$$ = $3; 
	};
extparam:semtoken
{
		int  index;		

		if(sip_initSipParam(&glbSipParserSipParam,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!= SipSuccess)
			YYABORT;
		/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		glbSipParserSipParam->pName = (SIP_S8bit *) fast_memget( \
			BISON_MEM_ID, $1.dLength-index+1, (((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError));
		if(glbSipParserSipParam->pName==SIP_NULL)
			YYABORT;
		strncpy(glbSipParserSipParam->pName,&($1.pToken[index]), \
			$1.dLength-index);
		glbSipParserSipParam->pName[$1.dLength-index] = '\0';
	}
		|semtoken EQUALS alltoken
	{
		SIP_S8bit *yyeparam;
		int  index;		

		if(sip_initSipParam(&glbSipParserSipParam,\
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) !=SipSuccess)
			YYABORT;

		/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		glbSipParserSipParam->pName = (SIP_S8bit *) fast_memget( \
			BISON_MEM_ID, $1.dLength-index+1, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError));
		if(glbSipParserSipParam->pName==SIP_NULL)
			YYABORT;
		strncpy(glbSipParserSipParam->pName,&($1.pToken[index]),  \
			$1.dLength-index);
		glbSipParserSipParam->pName[$1.dLength-index] = '\0';

		yyeparam=sip_tokenBufferDup($3);
		if(sip_listAppend(&glbSipParserSipParam->slValue,(SIP_Pvoid) \
			yyeparam,(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
alltoken:token
		|QSTRING
		|ipv6;
newalltoken:SEMPARAMVALUE
		| token
		| ipv6
		|QSTRING;
		
token:	TOKEN		
		|digits;		
uricnoparam:	uricnoparam uricnocolonnoparam
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2. \
			dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
		|uricnoparam COLON	
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2. \
			dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
		|uricnocolonnoparam 
		|COLON; 
uric:	uric uricnocolon	
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.\
			dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *) \
			 pHeaderParserParam)->pError)) == SipFail)
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
			dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
		|uricnocolon 
		|COLON; 
uricnocolon:TOKEN	
		|digits			
		|URICNOCOLON	
		|USERINFOAT		
		|EQUALS 			
		|QUESTION
		|semtoken
		|SEMICOLON
		|QSTRING1
		|IPV6ADDR
		|COMMA
		|SEMPARAMVALUE
		|SEMPARAMTOKEN
		|PLAINSIPCOLON;
uricnocolonnoparam:TOKEN	
		|digits			
		|URICNOCOLON	
		|USERINFOAT		
		|EQUALS 			
		|QUESTION
		|IPV6ADDR
		|COMMA;
digits:	DIGITS;
tocolon:TO
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pToHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one To Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_PARSER_ERROR;
			YYABORT;
		}
	};
