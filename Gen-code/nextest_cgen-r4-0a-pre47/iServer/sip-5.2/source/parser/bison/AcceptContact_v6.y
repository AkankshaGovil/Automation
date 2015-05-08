/***********************************************************************
 ** FUNCTION:
 **             Yacc file for accept contact headers
 *********************************************************************
 **
 ** FILENAME:
 ** Acceptcontact.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

%token ACCEPTCONTACT SEMICOLON COLON RANGLE AMPERSAND
%token AT EQUALS SIPCOLON SIPSCOLON PLAINSIPCOLON DIGITS 
%token QSTRING COMMA STAR TOKEN
%token URICNOCOLON QUESTION SIPFLOAT 
%token QVAL DIGIT2 DIGIT4 DISPNAME USERINFOAT
%token SEMTOKEN SEMPARAMTOKEN IPV6ADDR
%token DEFAULT
%pure_parser
%{
#include "sipparserinc.h"
#include "siptrace.h"
#include <stdlib.h>
#include <ctype.h>
#include "sipstruct.h"
#include "ccpstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "ccpinit.h"
#include "ccpfree.h"
#include "sipdecodeintrnl.h"
#define MAXTEMP (300)

static SipAcceptContactHeader* glbSipParserAcceptContactHeader=SIP_NULL;
extern int glbSipParserAcceptContactAllowSipColon;
#define YYPARSE_PARAM pHeaderParserParam
%}

%%
/* This Yacc file parsers Accept Contact Header */

header:acceptcontact
	   |error
		{
			if(glbSipParserAcceptContactHeader != SIP_NULL)
			{
				sip_ccp_freeSipAcceptContactHeader(\
					glbSipParserAcceptContactHeader);	
						glbSipParserAcceptContactHeader=SIP_NULL;
			}
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					=E_PARSER_ERROR;	
		};
acceptcontact:acceptcontactcolon normalcontacts;

/* This rule parses comma separated accept contact headers */
normalcontacts:normalcontacts COMMA normalcontact
{
	SipHeaderOrderInfo *order;
	SIP_U32bit      ordersize;
	/* Update Order table entry */
	if (sip_listAppend( \
		&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
		u.pRequest->pRequestHdr->slAcceptContactHdr,(SIP_Pvoid)\
		glbSipParserAcceptContactHeader, (((SipHeaderParserParam *)\
		pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
	glbSipParserAcceptContactHeader = SIP_NULL;
	if (sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)\
		->pSipMessage->slOrderInfo, &ordersize,(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError)) == SipFail)
		YYABORT;
	if (sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->slOrderInfo, ordersize - 1,(SIP_Pvoid *) & order,\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
		YYABORT;
	order->dNum++;

}
|normalcontact
{
	SipHeaderOrderInfo *order;
	SipBool	dResult;
	
	/* create and update order table entry */
	if (sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->u.pRequest->pRequestHdr->slAcceptContactHdr,\
		(SIP_Pvoid) glbSipParserAcceptContactHeader, \
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
	glbSipParserAcceptContactHeader = SIP_NULL;
	dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
	if (dResult!=SipSuccess)
	{
		YYABORT;
	}

	order->dType = SipHdrTypeAcceptContact;
	if($1.dChar=='f')
           order->dTextType = SipFormFull;
        else
           order->dTextType = SipFormShort;
	order->dNum = 1;
	if (sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->slOrderInfo, (SIP_Pvoid) order,(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
};
/*
 * this rule specifies that accept contact has addrspec followed by pParam or
 * STAR followed by pParam
 */
normalcontact:nameaddr
| nameaddr contactparams
| star contactparams
| star;
star:STAR
{
	if (sip_ccp_initSipAcceptContactHeader(&glbSipParserAcceptContactHeader, \
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
	glbSipParserAcceptContactHeader->pDispName = SIP_NULL;
	if (sip_listInit(&glbSipParserAcceptContactHeader->slAcceptContactParams, \
		&__sip_ccp_freeSipAcceptContactParam, (((SipHeaderParserParam *)\
		pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
	if (sip_initSipAddrSpec(&glbSipParserAcceptContactHeader->pAddrSpec,\
		SipAddrReqUri, (((SipHeaderParserParam *) pHeaderParserParam)->\
		pError)) != SipSuccess)
		YYABORT;
	glbSipParserAcceptContactHeader->pAddrSpec->u.pUri = sip_tokenBufferDup($1);
	if (glbSipParserAcceptContactHeader->pAddrSpec->u.pUri == SIP_NULL)
	{
		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
		YYABORT;
	}
};

nameaddr:addrspec;
dispname:DISPNAME
{

		int cnt,cnt1;
		$$ = $1;
	
		if (sip_ccp_initSipAcceptContactHeader(&glbSipParserAcceptContactHeader, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
			YYABORT;

		cnt = $$.dLength-2;
		/* remove trailing spaces */
		for(;(cnt>=0)&&(isspace((int)$$.pToken[cnt]));cnt--);
		$$.dLength = cnt+1;
		/* Now remove leading spaces */
		for(cnt1=0;(cnt1<=cnt)&&(isspace((int)$$.pToken[cnt1]));cnt1++)
			{}
		$$.pToken = &($$.pToken[cnt1]);
		$$.dLength-=cnt1;

		glbSipParserAcceptContactHeader->pDispName = sip_tokenBufferDup($$);

		glbSipParserAcceptContactHeader->pAddrSpec = SIP_NULL;
		if (sip_listInit(&glbSipParserAcceptContactHeader->slAcceptContactParams, \
			 &__sip_ccp_freeSipAcceptContactParam, (((SipHeaderParserParam *)\
			 pHeaderParserParam)->pError)) != SipSuccess)
			YYABORT;
};

addrspec:sipurl
| uri
{
	if (glbSipParserAcceptContactHeader == SIP_NULL)
	{
		if (sip_ccp_initSipAcceptContactHeader(\
			&glbSipParserAcceptContactHeader,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) != SipSuccess)
			YYABORT;
		glbSipParserAcceptContactHeader->pDispName = SIP_NULL;
		if (sip_listInit(&glbSipParserAcceptContactHeader->\
			slAcceptContactParams,&__sip_ccp_freeSipAcceptContactParam, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!= SipSuccess)
			YYABORT;
	}
	if (sip_initSipAddrSpec(&glbSipParserAcceptContactHeader->pAddrSpec,\
		SipAddrReqUri, (((SipHeaderParserParam *) pHeaderParserParam)->\
		pError)) != SipSuccess)
		YYABORT;
	glbSipParserAcceptContactHeader->pAddrSpec->u.pUri = sip_tokenBufferDup($1);
	if (glbSipParserAcceptContactHeader->pAddrSpec->u.pUri == SIP_NULL)
	{
		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
		YYABORT;
	}
};
uri:dispname uric RANGLE
	{
		$$=$2;
	}
	| uricnoparam;
sipurl:dispname sipcolon userinfoat hostport urlparams headers RANGLE
| dispname sipcolon hostport urlparams headers RANGLE
| sipcolon userinfoat hostport
| sipcolon hostport;

headers:QUESTION
{
	glbSipParserAcceptContactAllowSipColon=1;
}	
uricdummy
|
{
	glbSipParserAcceptContactAllowSipColon=0;
};

uricdummy:uric
{
	glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pHeader = \
		sip_tokenBufferDup($1);
	if (glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pHeader\
		== SIP_NULL)
	{
		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
		YYABORT;
	}
	glbSipParserAcceptContactAllowSipColon=0;
};

userinfoat:USERINFOAT
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

		glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pUser = pUser;

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
		glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pPassword =\
		pPasswd;
};
sipcolon:SIPCOLON
	{
	if (glbSipParserAcceptContactHeader == SIP_NULL)
	{
		if(sip_ccp_initSipAcceptContactHeader(&glbSipParserAcceptContactHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!= SipSuccess)
			YYABORT;
		glbSipParserAcceptContactHeader->pDispName = SIP_NULL;
		if(sip_listInit(&glbSipParserAcceptContactHeader->\
			slAcceptContactParams, &__sip_ccp_freeSipAcceptContactParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 != SipSuccess)
			YYABORT;
	}
	if (sip_initSipAddrSpec(&glbSipParserAcceptContactHeader->pAddrSpec,\
		SipAddrSipUri, (((SipHeaderParserParam *) pHeaderParserParam)->\
		pError)) != SipSuccess)
		YYABORT;
	if (sip_initSipUrl(&glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl,\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
	}
	| SIPSCOLON
	{
	if (glbSipParserAcceptContactHeader == SIP_NULL)
	{
		if(sip_ccp_initSipAcceptContactHeader(&glbSipParserAcceptContactHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!= SipSuccess)
			YYABORT;
		glbSipParserAcceptContactHeader->pDispName = SIP_NULL;
		if(sip_listInit(&glbSipParserAcceptContactHeader->\
			slAcceptContactParams, &__sip_ccp_freeSipAcceptContactParam,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			 != SipSuccess)
			YYABORT;
	}
	if (sip_initSipAddrSpec(&glbSipParserAcceptContactHeader->pAddrSpec,\
		SipAddrSipSUri, (((SipHeaderParserParam *) pHeaderParserParam)->\
		pError)) != SipSuccess)
		YYABORT;
	if (sip_initSipUrl(&glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl,\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
	};


hostport:host COLON port
{
	SIP_S8bit *temp;
	glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pHost = \
		(SIP_S8bit *) sip_tokenBufferDup($1);
	if (glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pHost==SIP_NULL)
	{
		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
		YYABORT;
	}
	glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->dPort=(SIP_U16bit *)\
		fast_memget(BISON_MEM_ID, sizeof(SIP_U16bit),\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError));
	if (glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->dPort==SIP_NULL)
		YYABORT;
	temp = sip_tokenBufferDup($3);
	*(glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->dPort)=atoi(temp);
	fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
}
|host
{
	glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pHost = \
		sip_tokenBufferDup($1);
	if (glbSipParserAcceptContactHeader->pAddrSpec->u.pSipUrl->pHost==SIP_NULL)
	{
		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
		YYABORT;
	}
};
host:token
	|IPV6ADDR;
port:digits;
urlparams:urlparams urlparam
|;
urlparam:	semparamtokens EQUALS newalltoken
	{
		SipParam *param; 
		int index;		
		SIP_S8bit *yyuparam;

		if(sip_initSipParam(&param,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID,\
			$1.dLength-index+1,(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		yyuparam=sip_tokenBufferDup($3);	

		if(sip_listAppend(&param->slValue,yyuparam,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) !=SipSuccess)
			YYABORT;

		if(sip_listAppend(&(glbSipParserAcceptContactHeader->pAddrSpec->\
			u.pSipUrl->slParam),(SIP_Pvoid)param,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) !=SipSuccess)
			YYABORT;
	}
	| semparamtokens
	{
		SipParam *param; 
		int index;		

		if(sip_initSipParam(&param,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID,\
			$1.dLength-index+1,(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		if(sip_listAppend(&(glbSipParserAcceptContactHeader->pAddrSpec->\
			u.pSipUrl->slParam),(SIP_Pvoid)param,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) !=SipSuccess)
			YYABORT;
	};

	
/*
 * this rule parses accept contact Param which can be of Type
 * cp-param,ext-param or q-Param
 */
contactparams:contactparams contactparam
| contactparam;
contactparam:qparam
| extensionattr;
extensionattr:semparamtokens EQUALS alltoken
{
	SipAcceptContactParam *cparam;
	SipParam       *param;
	SIP_S8bit      *pValue, *temp, *curr, *pos;
	SIP_U32bit index;

	if (sip_ccp_initSipAcceptContactParam(&cparam, SipAccContactTypeExt, \
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
	if (sip_initSipParam(&param, (((SipHeaderParserParam *)\
		pHeaderParserParam)->pError)) == SipFail)
		YYABORT;

	index=1;
	while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
		index++;

	$1.pToken = &($1.pToken[index]);
	$1.dLength -= index;
	param->pName = sip_tokenBufferDup($1);

	pValue = (SIP_S8bit *) sip_tokenBufferDup($3);
	if (pValue == SIP_NULL)
	{
		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
		YYABORT;
	}

	/* separate the  comma separated fields in a quoted string */
	if (pValue[0] == '\"')
	{
		curr = pValue + 1;
		while ((curr != SIP_NULL) && (pos = strchr(curr, ',')) != SIP_NULL)
		{
			temp = (char *) fast_memget(BISON_MEM_ID, pos - curr + 3,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
			if (temp == SIP_NULL)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
				YYABORT;
			}
			strcpy(temp, "");
			strncat(temp, curr, pos - curr);
			if (sip_listAppend(&(param->slValue), temp, \
				(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError)) == SipFail)
				YYABORT;

			curr = pos + 1;
		}
		if(curr==(pValue+1))
		{
				temp = (char *) fast_memget(BISON_MEM_ID, strlen(curr) + 5, \
					(((SipHeaderParserParam *) pHeaderParserParam)->pError));
				if (temp == SIP_NULL)
				{
					*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					= E_NO_MEM;
					YYABORT;
				}
				strcpy(temp, "\"");
				strncat(temp, curr, strlen(curr) - 1);
				strcat(temp, "\"");
		}
		else
		{
				temp = (char *) fast_memget(BISON_MEM_ID, strlen(curr) + 3, \
					(((SipHeaderParserParam *) pHeaderParserParam)->pError));
				if (temp == SIP_NULL)
				{
					*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					= E_NO_MEM;
					YYABORT;
				}
				strcpy(temp, "");
				strncat(temp, curr, strlen(curr) - 1);
		}
		if (sip_listAppend(&(param->slValue), temp, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		fast_memfree(BISON_MEM_ID, pValue, SIP_NULL);
	} 
	else
	{
		if (sip_listAppend(&(param->slValue), pValue, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
	}
	cparam->u.pExtParam = param;

	if (sip_listAppend(&glbSipParserAcceptContactHeader->\
		slAcceptContactParams, (SIP_Pvoid) cparam, (((SipHeaderParserParam *)\
		pHeaderParserParam)->pError)) != SipSuccess)
		YYABORT;
};
qparam:QVAL EQUALS float
{
	SipAcceptContactParam *cparam;

	if (sip_ccp_initSipAcceptContactParam(&cparam, SipAccContactTypeQvalue, \
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
	    YYABORT;
    (cparam->u.pQvalue) = sip_tokenBufferDup($3);
	if (cparam->u.pQvalue == SIP_NULL)
	{
		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
		YYABORT;
	}
	if (sip_listAppend(&glbSipParserAcceptContactHeader->\
		slAcceptContactParams, (SIP_Pvoid) cparam, \
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
		                YYABORT;
};

token:TOKEN
| digits
| STAR;
uric:uric uricnocolon
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength,(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *)\
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
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength,(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *)\
			 pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
|uricnocolon
| COLON;
uricnoparam:uricnoparam uricnocolonnoparam
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength,(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *)\
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
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
		$2.dLength,(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
			pGCList, (SIP_Pvoid) $$.pToken, (((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
|uricnocolonnoparam
| COLON;
uricnocolon:TOKEN
				|float
                |URICNOCOLON
				|USERINFOAT
				|AMPERSAND
				|AT
				|EQUALS
				|SEMICOLON
				|QUESTION
				|SEMTOKEN
				|IPV6ADDR
				|PLAINSIPCOLON;
uricnocolonnoparam:TOKEN
				|float
                |URICNOCOLON
				|USERINFOAT
				|AMPERSAND
				|AT
				|EQUALS
				|QUESTION;
digits:DIGITS
| DIGIT2
| DIGIT4;
float:  digits
		|SIPFLOAT;
alltoken:token
		|QSTRING
		|IPV6ADDR;
newalltoken:newalltoken newrule
		{
			$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+\
			$2.dLength,(((SipHeaderParserParam *) pHeaderParserParam)->pError));
			if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->\
				pGCList, (SIP_Pvoid) $$.pToken,(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError)) == SipFail)
				YYABORT;
			if($$.pToken == SIP_NULL)
				YYABORT;
			strncpy($$.pToken, $1.pToken, $1.dLength);
			strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
			$$.dLength = $1.dLength+$2.dLength;
		}
		|newrule;
semparamtokens:SEMPARAMTOKEN 
			|SEMTOKEN;		
acceptcontactcolon:ACCEPTCONTACT
{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType\
			!= SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_ERROR;
			YYABORT;
		}

};

newrule: STAR
	|QSTRING
	|TOKEN
	|float
    |URICNOCOLON
	|USERINFOAT
	|AMPERSAND
	|AT
	|EQUALS
	|SEMICOLON
	|IPV6ADDR
	|COLON
	|PLAINSIPCOLON;
