/***********************************************************************
 ** FUNCTION:
 **             Yacc file for IM-URL

 *********************************************************************
 **
 ** FILENAME:
 ** Im.y
 **
 ** DESCRIPTION:
 ** This defines yaac rule for parsing IM-URL
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 11/01/02   Manoj Chauhan									Initial
 **
 **     Copyright 2002, Hughes Software Systems, Ltd.
 *********************************************************************/


%token LANGLE RANGLE LSQUARE RSQUARE SPACE
%token AT COLON COMMA DOT QUESTION AMPERSAND EQUALS 
%token IMATOM DTEXT QPAIRTEXT URLC IMCOLON
%token QSTRING TOKEN PRESCOLON
%token DEFAULT
%pure_parser
%{ 
#include <stdlib.h>
#include <ctype.h>
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siptrace.h"
#include "imppfree.h"
#include "imppinit.h"
#include "imppstruct.h"	
#include "imurldecodeintrnl.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "presstruct.h"


static SipList      glbslParam;
static SipList	    glbslRoute;	/* siplist of SIP_S8bit*   */
static SipParam    *glbSipParam=SIP_NULL;
static SIP_S8bit   *pTemp=SIP_NULL;
static SIP_S8bit   *pTempDomain=SIP_NULL;

#define YYPARSE_PARAM pImParserParam
%}
%%
header: impresurl
		|error
		{
			SIP_U32bit dSize=0;
			SipError dErr;
			if(sip_listSizeOf(&glbslParam,&dSize,&dErr)==SipFail)
				YYABORT;
       		if(dSize!=0)
      		{
                sip_listDeleteAll(&glbslParam,&dErr);
                glbslParam.size=0;
                ((SipImParserParam*)pImParserParam)->pImUrl->slParams = glbslParam;

        	}
			if(sip_listSizeOf(&glbslRoute,&dSize,&dErr)==SipFail)
				YYABORT;
         	if(dSize !=0)
        	{
                if(sip_listDeleteAll(&glbslRoute,&dErr)==SipFail)
					YYABORT;
                glbslRoute.size=0;
                ((SipImParserParam*)pImParserParam)->pImUrl->slRoute = glbslRoute;

        	}
        	if(((SipImParserParam*)pImParserParam)->pImUrl!=SIP_NULL)
			{
#ifdef SIP_IMPP
				sip_freeImUrl(((SipImParserParam*)pImParserParam)->pImUrl);
#else
#ifdef SIP_PRES
				sip_freePresUrl((PresUrl *)((SipImParserParam*)pImParserParam)->pImUrl);
#endif
#endif
        		((SipImParserParam*)pImParserParam)->pImUrl=SIP_NULL;
			}

			if(glbSipParam != SIP_NULL)
			{
				sip_freeSipParam(glbSipParam);	
				glbSipParam= SIP_NULL;
			}
			
			if(*(((SipImParserParam*)pImParserParam)->pError)==E_NO_ERROR)
				*(((SipImParserParam*)pImParserParam)->pError)=\
					E_PARSER_ERROR;

		};
		
impresurl: imprescolon imurlparams
	{
		((SipImParserParam*)pImParserParam)->pImUrl->slRoute=glbslRoute;
		((SipImParserParam*)pImParserParam)->pImUrl->slParams=glbslParam;
	};
imurlparams: tofield
	| headerfield
	| tofield headerfield
	|;

tofield: addrspec
	| phrase routeaddr
	{
		((SipImParserParam*)pImParserParam)->pImUrl->pDispName = \
			sip_tokenBufferDup($1);
	};

phrase: word;

routeaddr: LANGLE routeaddrspec RANGLE;
routeaddrspec: addrspec
	{
	}
	| route COLON addrspec;

route: routedomain
	| route COMMA routedomain;
	
routedomain: AT domain
	{
		pTemp=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.dLength+1, \
				(((SipImParserParam*)pImParserParam)->pError));
		if(pTemp==SIP_NULL)
			YYABORT;
		strncpy(pTemp, $1.pToken,$1.dLength);
		strncpy(&(pTemp[$1.dLength]),$2.pToken,$2.dLength);
		pTemp[$1.dLength+$2.dLength]='\0';
		
		if(sip_listAppend(&glbslRoute,pTemp, \
			(((SipImParserParam*)pImParserParam)->pError)) != \
			SipSuccess)
			YYABORT;
	};
	
addrspec: localpart AT domain
{
		((SipImParserParam*)pImParserParam)->pImUrl->pUser = \
			sip_tokenBufferDup($1);
		((SipImParserParam*)pImParserParam)->pImUrl->pHost = \
			sip_tokenBufferDup($3);

};
localpart: localpartword 
	| localpart DOT localpartword
	{
		
		pTemp=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$3.dLength+1, \
				(((SipImParserParam*)pImParserParam)->pError));

		if(sip_listAppend(((SipImParserParam *) pImParserParam)->\
			pGCList, (SIP_Pvoid) pTemp,(((SipImParserParam *) \
			pImParserParam)->pError)) == SipFail)
			YYABORT;
		
		if(pTemp==SIP_NULL)
			YYABORT;
		strncpy(pTemp, $1.pToken,$1.dLength);
		pTemp[$1.dLength]='.';
		strncpy(&(pTemp[$1.dLength+1]),$3.pToken,$3.dLength);
		$$.pToken=pTemp;
		$$.dLength=$1.dLength+1+$3.dLength;
	};
	
	
localpartword: word;

word: imatom
	| QSTRING;
	
imatom: IMATOM;
	
domain: subdomain
	| domain DOT subdomain
	{
		pTemp=(SIP_S8bit*)fast_memget(BISON_MEM_ID,$1.dLength+$3.dLength+1, \
				(((SipImParserParam*)pImParserParam)->pError));

		if(sip_listAppend(((SipImParserParam *) pImParserParam)->\
			pGCList, (SIP_Pvoid) pTemp,(((SipImParserParam *) \
			pImParserParam)->pError)) == SipFail)
			YYABORT;

		if(pTemp==SIP_NULL)
			YYABORT;
		strncpy(pTemp, $1.pToken,$1.dLength);
		pTemp[$1.dLength]='.';
		strncpy(&(pTemp[$1.dLength+1]),$3.pToken,$3.dLength);
		$$.pToken=pTemp;
		$$.dLength=$1.dLength+1+$3.dLength;
	};
subdomain: domainref
	| domainliteral;
domainref: imatom;
domainliteral: LSQUARE literaltext RSQUARE
	{
		pTempDomain=(SIP_S8bit*)fast_memget(BISON_MEM_ID,$1.dLength+$2.dLength+\
		$3.dLength, (((SipImParserParam*)pImParserParam)->pError));

		if(sip_listAppend(((SipImParserParam *) pImParserParam)->\
			pGCList, (SIP_Pvoid) pTempDomain,(((SipImParserParam *) \
			pImParserParam)->pError)) == SipFail)
			YYABORT;

		if(pTempDomain==SIP_NULL)
			YYABORT;
		strncpy(pTempDomain, $1.pToken,$1.dLength);
		strncpy(&(pTempDomain[$1.dLength]),$2.pToken,$2.dLength);
		strncpy(&(pTempDomain[$1.dLength+$2.dLength]),$3.pToken,$3.dLength);
		$$.pToken=pTempDomain;
		$$.dLength=$1.dLength+$2.dLength+$3.dLength;
	};
literaltext: DTEXT
	| qpairtext
	|;
	
qpairtext: QPAIRTEXT;
	
headerfield: QUESTION headerparams
	{

	};
headerparams: headerparam
	| headerparams AMPERSAND headerparam;

headerparam: URLC EQUALS URLC
	{
		SipParam *pParam;
		
		if(sip_initSipParam(&pParam, (((SipImParserParam*)\
					pImParserParam)->pError))== SipFail)
					YYABORT;
		pParam->pName = sip_tokenBufferDup($1);

		if(sip_listAppend(&pParam->slValue,sip_tokenBufferDup($3),\
			(((SipImParserParam*)pImParserParam)->\
			pError))!=SipSuccess)
			YYABORT;

		if(sip_listAppend(&glbslParam, pParam,\
			(((SipImParserParam*)pImParserParam)->pError))== SipFail)
			YYABORT;

};

imprescolon: IMCOLON
	{
#ifdef SIP_IMPP
		if(sip_initImUrl(&((SipImParserParam*)pImParserParam)->\
			pImUrl, (((SipImParserParam*)pImParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		if(sip_listInit(&glbslRoute,&__sip_freeString \
		,(((SipImParserParam*)pImParserParam)->pError)) != SipSuccess)
			YYABORT;	
		if(sip_listInit(&glbslParam,&__sip_freeSipParam\
		, (((SipImParserParam*)pImParserParam)->pError)) != SipSuccess)
			YYABORT;
#endif
	}
	| PRESCOLON
	{
#ifdef SIP_PRES
		if(sip_initPresUrl((PresUrl **) &((SipImParserParam*)pImParserParam)->\
			pImUrl, (((SipImParserParam*)pImParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		if(sip_listInit(&glbslRoute,&__sip_freeString \
		,(((SipImParserParam*)pImParserParam)->pError)) != SipSuccess)
			YYABORT;	
		if(sip_listInit(&glbslParam,&__sip_freeSipParam\
		, (((SipImParserParam*)pImParserParam)->pError)) != SipSuccess)
			YYABORT;
#endif
	}
	;

