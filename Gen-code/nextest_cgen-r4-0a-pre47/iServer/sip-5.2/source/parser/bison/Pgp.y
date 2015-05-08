
/***********************************************************************
 ** FUNCTION:
 **             Yacc file for  headers of form Pgp

 *********************************************************************
 **
 ** FILENAME:
 ** Pgp.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form pgp
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

%token WWWAUTHENTICATE PROXYAUTHENTICATE
%token AUTHORIZATION PROXYAUTHORIZATION BASIC
%token TOKEN EQUALS COMMA QSTRING BASE64STRING TOKENEQUALS COMMATOKENEQUALS
%token DEFAULT EQUALS

%pure_parser
%{ 
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include <ctype.h>
#include "portlayer.h"
#include "sipfree.h"
#include "sipstruct.h"
#include "sipinit.h"
#include "siplist.h"
#include "siptrace.h"

#define MAXTEMP (300)

static SipGenericChallenge *glbSipParserChallenge=SIP_NULL;
static SipGenericCredential*glbSipParserCredential=SIP_NULL;
#define YYPARSE_PARAM pHeaderParserParam
%}
%%
header:		wwwauthenticate
			|proxyauthenticate
			|authorization
			|proxyauthorization
			|error
			{
				if(glbSipParserCredential != SIP_NULL)
				{
					sip_freeSipGenericCredential(glbSipParserCredential);
					glbSipParserCredential= SIP_NULL;
				}
				if(glbSipParserChallenge != SIP_NULL)
				{
					sip_freeSipGenericChallenge(glbSipParserChallenge);
					glbSipParserChallenge = SIP_NULL;
				}
				if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					=E_PARSER_ERROR;
	
			};

wwwauthenticate:	WWWAUTHENTICATE wwwchallenges;
proxyauthenticate:	proxyauthenticatecolon proxychallenges;
authorization:		authcolon authfield;
authfield : credentials	
	{
		SipHeaderOrderInfo *order;
		SipAuthorizationHeader *auth;
		SipBool	dResult;
		
	 	if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType\
			== SipMessageRequest)
		{
			
			if(sip_initSipAuthorizationHeader(&auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;

			auth->pCredential = glbSipParserCredential;
			glbSipParserCredential = SIP_NULL;
			
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
				slAuthorizationHdr, (SIP_Pvoid)auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;
	
		}
		else
		{
			if(sip_initSipAuthorizationHeader(&auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;

			auth->pCredential = glbSipParserCredential;
			glbSipParserCredential = SIP_NULL;

			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
				slAuthorizationHdr, (SIP_Pvoid)auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;
		}

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
	
		order->dType = SipHdrTypeAuthorization;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	| authfield COMMA credentials
	{
		SipHeaderOrderInfo *order;
		SipAuthorizationHeader *auth;
		SIP_U32bit ordersize;
		
	 	if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType\
			== SipMessageRequest)
		{
			
			if(sip_initSipAuthorizationHeader(&auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;

			auth->pCredential = glbSipParserCredential;
			glbSipParserCredential = SIP_NULL;
			
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
				slAuthorizationHdr, (SIP_Pvoid)auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;
	
		}
		else
		{
			if(sip_initSipAuthorizationHeader(&auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;

			auth->pCredential = glbSipParserCredential;
			glbSipParserCredential = SIP_NULL;

			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
				slAuthorizationHdr, (SIP_Pvoid)auth, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
				==SipFail)
				YYABORT;
		}
		if(sip_listSizeOf(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,&ordersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
	}
	|authfield COMMA;
proxyauthorization:	proxyauthcolon proxyauthfield;
proxyauthfield:	credentials
	{
		SipProxyAuthorizationHeader *pProxyAuthHdr;
		SipHeaderOrderInfo *order;
		SipBool	dResult;

		if(sip_initSipProxyAuthorizationHeader(&pProxyAuthHdr, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;

		pProxyAuthHdr->pCredentials = glbSipParserCredential;
		glbSipParserCredential = SIP_NULL;

		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
			slProxyAuthorizationHdr, (SIP_Pvoid)pProxyAuthHdr, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;
			
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
	
		order->dType = SipHdrTypeProxyauthorization;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	| proxyauthfield COMMA credentials
	{
		SipProxyAuthorizationHeader *pProxyAuthHdr;
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;

		if(sip_initSipProxyAuthorizationHeader(&pProxyAuthHdr, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;

		pProxyAuthHdr->pCredentials = glbSipParserCredential;
		glbSipParserCredential = SIP_NULL;

		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
			slProxyAuthorizationHdr, (SIP_Pvoid)pProxyAuthHdr, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;
		if(sip_listSizeOf(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,&ordersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
	}
	|proxyauthfield COMMA;

credentials:	basiccredentials
	{
		if(sip_initSipGenericCredential(&glbSipParserCredential,SipCredBasic, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserCredential->u.pBasic = sip_tokenBufferDup($1);
		if(glbSipParserCredential->u.pBasic==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}
			|challenge
	{
		if(sip_initSipGenericCredential(&glbSipParserCredential,SipCredAuth, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserCredential->u.pChallenge = glbSipParserChallenge;
		glbSipParserChallenge =  SIP_NULL;
	};
basiccredentials:	BASIC base64string
	{
		$$ = $2;
	};
wwwchallenges:	wwwchallenges COMMA challenge
	{
		SipWwwAuthenticateHeader *wheader; 
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;

		if(sip_initSipWwwAuthenticateHeader(&wheader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		wheader -> pChallenge = glbSipParserChallenge;
		glbSipParserChallenge =  SIP_NULL;
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType == SipMessageRequest)
		{
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pRequest->\
				pRequestHdr->slWwwAuthenticateHdr, wheader,\
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
				YYABORT;
		}
		else
		{
			if(sip_listAppend( &((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pResponse->\
				pResponseHdr->slWwwAuthenticateHdr, \
				wheader,&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		}
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,&ordersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
	}
			|challenge
	{
		SipWwwAuthenticateHeader *wheader; 
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		if(sip_initSipWwwAuthenticateHeader(&wheader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		wheader -> pChallenge = glbSipParserChallenge;
		glbSipParserChallenge = SIP_NULL;
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType == SipMessageRequest)
		{
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pRequest->\
				pRequestHdr->slWwwAuthenticateHdr, \
				wheader,&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		}
		else
		{
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->u.pResponse->\
				pResponseHdr->slWwwAuthenticateHdr, \
				wheader,&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		}

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType = SipHdrTypeWwwAuthenticate;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
			|wwwchallenges COMMA;
proxychallenges:proxychallenges COMMA challenge
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		SipProxyAuthenticateHeader *wheader; 
		if(sip_initSipProxyAuthenticateHeader(&wheader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		wheader -> pChallenge = glbSipParserChallenge;
		glbSipParserChallenge =  SIP_NULL;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
			slProxyAuthenticateHdr, wheader,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_listSizeOf(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,&ordersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
	}
			|challenge
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		SipProxyAuthenticateHeader *wheader; 
		if(sip_initSipProxyAuthenticateHeader(&wheader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) !=SipSuccess)
			YYABORT;
		wheader -> pChallenge = glbSipParserChallenge;
		glbSipParserChallenge =  SIP_NULL;
		if(sip_listAppend( &((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr\
			->slProxyAuthenticateHdr, wheader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}			
		
		order->dType = SipHdrTypeProxyAuthenticate;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
			|proxychallenges COMMA;

challenge:	scheme authparams;
scheme:		token
	{
		if(sip_initSipGenericChallenge(&glbSipParserChallenge,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserChallenge -> pScheme = sip_tokenBufferDup($1);
		if(glbSipParserChallenge->pScheme==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listInit(&glbSipParserChallenge->slParam,\
			&__sip_freeSipParam,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) != SipSuccess)
			YYABORT;
	};
authparams:	authparams commaauthparam
			|authparam;
commaauthparam:	COMMATOKENEQUALS allstring
	{
		SipParam * authparam;
		SIP_S8bit *yyaparam;
		SIP_U32bit i;
		
		if(sip_initSipParam(&authparam,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		/* Remove comma and leading whitespace */
		$1.pToken = &($1.pToken[1]);
		i=0;
		while(isspace((int)$1.pToken[i])) i++;
		$1.pToken = &($1.pToken[i]);
		$1.dLength -= i+1;
		/* Remove equals and trailing whitespace */
		i = $1.dLength-2;
		while(isspace((int)$1.pToken[i])) i--;
		$1.dLength = i+1;

		authparam->pName = sip_tokenBufferDup($1);
		if(authparam->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		yyaparam=sip_tokenBufferDup($2);
		if(sip_listAppend(&authparam->slValue,yyaparam,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;	
		if(sip_listAppend(&glbSipParserChallenge->slParam,authparam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
allstring:qstring
	|token;
authparam: TOKENEQUALS allstring
	{
		SipParam * authparam;
		SIP_S8bit *yyaparam;
		SIP_U32bit i;
		
		if(sip_initSipParam(&authparam,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		/* Remove equals and trailing whitespace */
		i = $1.dLength-2;
		while(isspace((int)$1.pToken[i])) i--;
		$1.dLength = i+1;

		authparam->pName = sip_tokenBufferDup($1);
		if(authparam->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		yyaparam=sip_tokenBufferDup($2);
		if(sip_listAppend(&authparam->slValue,yyaparam,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;	
		if(sip_listAppend(&glbSipParserChallenge->slParam,authparam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
base64string:BASE64STRING; 
token:		TOKEN	
			|BASE64STRING 
			|BASIC;
qstring:		QSTRING;	
authcolon:		AUTHORIZATION;
proxyauthcolon:	PROXYAUTHORIZATION
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
	};
proxyauthenticatecolon:	PROXYAUTHENTICATE
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType != SipMessageResponse)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
	};
