
/***********************************************************************
 ** FUNCTION:
 **             Yacc file for contact type headers

 *********************************************************************
 **
 ** FILENAME:
 ** Contact.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 22/1/01    Ashok I roy             Added IPv6 token 	  as per Ipv6	
 **									   uricnoclon and         requirements
 **									   uricnocolonnoparam     of rfc 2732
 ** 22Jan01		Rajasri				Removed initialisation
 **									Code for Comment field
 ** 18/01/01  Mahesh G,Ashok I roy     escaped QSTRING       tel support
 **									
** 17/01/01  T.Seshashayi                              removed COMMENT parsing
 ** 27Oct00	S.Luthra				 added rule to account for optional day-of-week fiueld in rfc-1123 date format
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

%token CONTACT CALLID SEMICOLON COLON RANGLE AMPERSAND STAR ALSO
%token AT EQUALS SIPCOLON SIPSCOLON PLAINSIPCOLON DIGITS SEMPARAMTOKEN SEMPARAMVALUE
%token QSTRING QSTRING1 COMMA TOKEN FROMTAG TOTAG REPLACES REPLYTO
%token URICNOCOLON QUESTION SIPFLOAT
%token QVAL EXPIRES DIGIT2 DIGIT4 MONTH WEEKDAY GMT DISPNAME USERINFOAT SEMTOKEN
%token IPV6ADDR DOUBLEQUOTES
%token OPENBRACE CLOSEBRACE BACKSLASH HASH
%token DEFAULT JOIN NUMERIC_RELATION COMMA LANGLE RANGLE
%expect 1
%pure_parser
%{ 
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siptrace.h"

#define MAXTEMP (300)
#define YYPARSE_PARAM pHeaderParserParam
#define REPLACES_HEADER ((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->pReplacesHdr
#define REPLYTO_HEADER ((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->pGeneralHdr->pReplyToHdr
#define PARSER_ERROR_VAR ((SipHeaderParserParam *)pHeaderParserParam)->pError

#ifdef SIP_CONF                
/* Join Header */
#define JOIN_HEADER ((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->pJoinHdr
#endif

static SipContactHeader*		glbSipParserContactHeader=SIP_NULL;
static SipDateStruct 			*glbSipParserDateStruct=SIP_NULL;
static SIP_S8bit*				glbSipParserDispName=SIP_NULL;
static SipAlsoHeader*			glbSipParserAlsoHeader=SIP_NULL;
static SipAddrSpec*				glbSipParserAddrSpec=SIP_NULL;
extern int glbSipParserContactAllowSipColon;
static SIP_S8bit * glbName = SIP_NULL ; 
static SipList *pTagValList = SIP_NULL ;

%}
%%
header:		contact
			|also
			|callid
			|replaces
			|replyto
			|join
			|error
			{
				if(glbSipParserContactHeader != SIP_NULL)
				{
					sip_freeSipContactHeader(glbSipParserContactHeader);
					glbSipParserContactHeader=SIP_NULL;
				}
				if(glbSipParserAlsoHeader != SIP_NULL)
				{
					sip_freeSipAlsoHeader(glbSipParserAlsoHeader);
					glbSipParserAlsoHeader=SIP_NULL;
				}
				if(glbSipParserDateStruct != SIP_NULL)
				{
					sip_freeSipDateStruct(glbSipParserDateStruct);
					glbSipParserDateStruct=SIP_NULL;
				}
				if(glbSipParserDispName!= SIP_NULL)
				{
					fast_memfree(BISON_MEM_ID,glbSipParserDispName,SIP_NULL);
					glbSipParserDispName=SIP_NULL;
				}
				if(glbSipParserAddrSpec!=SIP_NULL)
				{
					sip_freeSipAddrSpec(glbSipParserAddrSpec);
					glbSipParserAddrSpec = SIP_NULL;
				}
				if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					==E_NO_ERROR)
					*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					=E_PARSER_ERROR;
			};

replyto: replytocolon replytonameaddr rplyparam
		|replytocolon replytonameaddr;
replytonameaddr: addrspec
	{
		SipHeaderOrderInfo *order;
		if(sip_initSipHeaderOrderInfo(&order,PARSER_ERROR_VAR) == SipFail)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		order->dType = SipHdrTypeReplyTo;
		order->dTextType = SipFormFull;
		order->dNum = 1;

		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
					pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
					PARSER_ERROR_VAR) !=SipSuccess)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		if(sip_initSipReplyToHeader(&REPLYTO_HEADER,PARSER_ERROR_VAR) == SipFail)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		if(glbSipParserDispName)
			REPLYTO_HEADER->pDispName = glbSipParserDispName;
		if(glbSipParserAddrSpec)
			REPLYTO_HEADER->pAddrSpec = glbSipParserAddrSpec;

		glbSipParserDispName = SIP_NULL;
		glbSipParserAddrSpec = SIP_NULL;
	};
rplyparam: rplyparam rplyvalue 
				| rplyvalue 
				{};
replytocolon:REPLYTO
		{
			
			/* if replyto header already exists */	
			if(REPLYTO_HEADER)
			{
				sip_error (SIP_Minor,\
						"There can only be one ReplyTo header in a message");
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}

		};
				
rplyvalue: SEMTOKEN 
		{
			SIP_S8bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
			while((*$1.pToken == ' ') || (*$1.pToken == '\t'))
				index++;
			$1.pToken += index;
			$1.dLength -= index;
			pParam->pName = sip_tokenBufferDup($1);
			if(sip_listAppend(&REPLYTO_HEADER->slParams,pParam,\
					PARSER_ERROR_VAR)	== SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
		}
		| SEMTOKEN  EQUALS newalltoken
		{
			SIP_S8bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
			while((*$1.pToken == ' ') || (*$1.pToken == '\t'))
				index++;
			$1.pToken += index;
			$1.dLength -= index;
			pParam->pName = sip_tokenBufferDup($1);
			pParam->pValue = sip_tokenBufferDup($3);

			if(sip_listAppend(&REPLYTO_HEADER->slParams,pParam,\
					PARSER_ERROR_VAR)	== SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
		};

replaces:  replacescolon replacescallid replaceparam1
	    |  replacescolon replacescallid	
		{};
replacescolon: REPLACES 
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
                       pSipMessage->dType!=SipMessageRequest)
       {
				*PARSER_ERROR_VAR = E_NO_ERROR;
                YYABORT;
       }
		/* if replaces header already exists */	
		if(REPLACES_HEADER)
		{
			sip_error (SIP_Minor,\
				"There can only be one Replaces header");
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}
			
	};

replacescallid: callidword
	{
		SipHeaderOrderInfo *order;
		SIP_S8bit* pTemp;

		if(sip_initSipHeaderOrderInfo(&order,PARSER_ERROR_VAR) == SipFail)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		order->dType = SipHdrTypeReplaces;
		order->dTextType = SipFormFull;
		order->dNum = 1;

		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
			PARSER_ERROR_VAR) !=SipSuccess)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		if(sip_initSipReplacesHeader(&REPLACES_HEADER,PARSER_ERROR_VAR)== SipFail)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		pTemp = sip_tokenBufferDup($1);
		if(pTemp == SIP_NULL)
		{
			*PARSER_ERROR_VAR = E_NO_MEM;
			YYABORT;
		}
		REPLACES_HEADER->pCallid = pTemp;
	};
replaceparam1:replaceparam1 replaceparam
		|replaceparam
		{};
replaceparam: fromtag 
			|totag 
			| genvalue
				{};
genvalue: SEMTOKEN
		{
			SIP_S8bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
			while((*$1.pToken == ' ') || (*$1.pToken == '\t'))
				index++;
			$1.pToken += index;
			$1.dLength -= index;
			pParam->pName = sip_tokenBufferDup($1);
			if(sip_listAppend(&REPLACES_HEADER->slParams,pParam,\
					PARSER_ERROR_VAR)	== SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
		}
		|SEMTOKEN  EQUALS newalltoken
		{
			SIP_S8bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
			while((*$1.pToken == ' ') || (*$1.pToken == '\t'))
				index++;
			$1.pToken += index;
			$1.dLength -= index;
			pParam->pName = sip_tokenBufferDup($1);
			pParam->pValue = sip_tokenBufferDup($3);

			if(sip_listAppend(&REPLACES_HEADER->slParams,pParam,\
					PARSER_ERROR_VAR)	== SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
		};
fromtag:  FROMTAG EQUALS newalltoken
		{
			SIP_S8bit* pTemp;
			/* if we already got an from-tag return parse error */
			if(REPLACES_HEADER->pFromTag)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
			
			pTemp = sip_tokenBufferDup($3);
			if(pTemp == SIP_NULL)
			{
				*PARSER_ERROR_VAR = E_NO_MEM;
				YYABORT;
			}
			REPLACES_HEADER->pFromTag = pTemp;
		};
totag:  TOTAG EQUALS newalltoken
		{
			SIP_S8bit* pTemp;
			/* if we already got an to-tag return parse error */
			if(REPLACES_HEADER->pToTag)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
			pTemp = sip_tokenBufferDup($3);
			if(pTemp == SIP_NULL)
			{
				*PARSER_ERROR_VAR = E_NO_MEM;
				YYABORT;
			}
			REPLACES_HEADER->pToTag = pTemp;
		};

callid:		callidcolon callidword
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeCallId;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
			&*(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		if(sip_initSipCallIdHeader( \
		    &((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->pGeneralHdr->pCallidHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->pGeneralHdr->pCallidHdr->pValue \
			= sip_tokenBufferDup($2);
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->pGeneralHdr->pCallidHdr->pValue == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}	
	};

also:ALSO
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= E_NO_ERROR;
			YYABORT;
		}
	} alsofields;


alsofields:	 alsofields COMMA alsofield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->slOrderInfo, \
		&ordersize,((SipHeaderParserParam *) pHeaderParserParam)->pError) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
		(SIP_Pvoid *)&order,((SipHeaderParserParam *) pHeaderParserParam)->pError) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
		slAlsoHdr,(SIP_Pvoid)glbSipParserAlsoHeader,((SipHeaderParserParam *) pHeaderParserParam)->pError)\
		!=SipSuccess)
			YYABORT;		
		glbSipParserAlsoHeader = SIP_NULL;
	}
	| alsofield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAlso;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->slOrderInfo, \
		(SIP_Pvoid)order, ((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
		slAlsoHdr,(SIP_Pvoid)glbSipParserAlsoHeader,((SipHeaderParserParam *) pHeaderParserParam)->pError)!=\
		SipSuccess)
			YYABORT;
		glbSipParserAlsoHeader = SIP_NULL;
	};

alsofield:	nameaddr
		{
			if(sip_initSipAlsoHeader(&glbSipParserAlsoHeader,((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				!=SipSuccess)
				YYABORT;
			glbSipParserAlsoHeader->pDispName = glbSipParserDispName;
			glbSipParserAlsoHeader->pAddrSpec = glbSipParserAddrSpec;
			glbSipParserAddrSpec = SIP_NULL;
			glbSipParserDispName= SIP_NULL;
		};

contact:CONTACT STAR
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		if(sip_initSipContactHeader(&glbSipParserContactHeader,SipContactWildCard, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		glbSipParserContactHeader->dType = SipContactWildCard;
		glbSipParserContactHeader->pDispName = SIP_NULL;
		glbSipParserContactHeader->pAddrSpec = SIP_NULL;
		if(sip_listInit(&glbSipParserContactHeader->slContactParam, \
			&__sip_freeSipContactHeader,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend( \
			&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->pGeneralHdr->slContactHdr, \
			(SIP_Pvoid)glbSipParserContactHeader,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) \
			!= SipSuccess)
			YYABORT;
		glbSipParserContactHeader = SIP_NULL;
	
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeContactAny;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)order\
			, &*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|CONTACT 
	{
		/* Empty rule required here. In the absence of this rule there will
		   be a reduce-reduce conflict. With this rule we get a predictable
		   shift-reduce conflict 
		*/
	}
	normalcontacts;

normalcontacts:	normalcontacts COMMA normalcontact
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;


		glbSipParserContactHeader->dType = SipContactNormal;
		if(sip_listAppend( \
			&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->pGeneralHdr->slContactHdr, \
			(SIP_Pvoid)glbSipParserContactHeader,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) \
			!= SipSuccess)
			YYABORT;
		if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->slOrderInfo,&ordersize,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1,\
		   (SIP_Pvoid *)&order,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if ( strcmp(glbSipParserContactHeader->pAddrSpec->u.pUri ,"*") == 0 ) 
		{
			sip_error (SIP_Minor, "There can't be a SipContactWildCard here.");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=E_PARSER_ERROR;
			glbSipParserContactHeader = SIP_NULL;
			YYABORT;
		}
		glbSipParserContactHeader = SIP_NULL;
	}
	|normalcontact
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeContactAny;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			slOrderInfo,(SIP_Pvoid)order, (((SipHeaderParserParam *) pHeaderParserParam)\
			->pError))!=SipSuccess)
			YYABORT;
		glbSipParserContactHeader->dType = SipContactNormal;
		if(sip_listAppend( \
			&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->pGeneralHdr->slContactHdr, \
			(SIP_Pvoid)glbSipParserContactHeader,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!= SipSuccess)
			YYABORT;
		if ( strcmp(glbSipParserContactHeader->pAddrSpec->u.pUri ,"*") == 0 ) 
		{
			sip_error (SIP_Minor, "There can't be a SipContactWildCard here.");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=E_PARSER_ERROR;
			glbSipParserContactHeader = SIP_NULL;
			YYABORT;
		}
		glbSipParserContactHeader = SIP_NULL;
	}
	| normalcontacts COMMA; 

normalcontact:	nameaddr
		{
			if(sip_initSipContactHeader(&glbSipParserContactHeader,\
				SipContactNormal, ((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
				YYABORT;
			glbSipParserContactHeader->pDispName = glbSipParserDispName;
			glbSipParserContactHeader->pAddrSpec= glbSipParserAddrSpec;
			glbSipParserDispName= SIP_NULL;
			glbSipParserAddrSpec = SIP_NULL;
			if(sip_listInit(&glbSipParserContactHeader->slContactParam, \
				&__sip_freeSipContactParam,((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
				!= SipSuccess)
				YYABORT;
		}
		|nameaddr
		{
			if(sip_initSipContactHeader(&glbSipParserContactHeader,\
				SipContactNormal, ((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
				YYABORT;
			glbSipParserContactHeader->pDispName = glbSipParserDispName;
			glbSipParserContactHeader->pAddrSpec= glbSipParserAddrSpec;
			glbSipParserDispName= SIP_NULL;
			glbSipParserAddrSpec = SIP_NULL;
			if(sip_listInit(&glbSipParserContactHeader->slContactParam, \
				&__sip_freeSipContactParam,((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
				!= SipSuccess)
				YYABORT;
		} contactparams;
nameaddr:	addrspec;
dispname:	DISPNAME
	{
		int cnt,cnt1;
		$$ = $1;

		cnt = $$.dLength-2;
		/* remove trailing spaces */
		for(;(cnt>=0)&&(isspace((int)$$.pToken[cnt]));cnt--);
		$$.dLength = cnt+1;
		/* Now remove leading spaces */
		for(cnt1=0;(cnt1<=cnt)&&(isspace((int)$$.pToken[cnt1]));cnt1++)
			{}
		$$.pToken = &($$.pToken[cnt1]);
		$$.dLength-=cnt1;
		glbSipParserDispName = sip_tokenBufferDup($$);
	}
	| LANGLE
	{
		glbSipParserDispName = (SIP_S8bit*)fast_memget(BISON_MEM_ID,2,SIP_NULL);
		strcpy(glbSipParserDispName," ") ;
	};
addrspec:sipurl
		|uri
	{
		if(sip_initSipAddrSpec(&glbSipParserAddrSpec, \
			SipAddrReqUri,((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)

			YYABORT;
		glbSipParserAddrSpec->u.pUri = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pUri == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
uri: 	dispname uric RANGLE
		{
			$$=$2;
		}
		|uricnoparam;
sipurl:dispname sipcolon userinfoat hostport urlparams headers RANGLE
		|dispname sipcolon hostport urlparams headers RANGLE
		|sipcolon userinfoat hostport
		|sipcolon hostport;
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
		if(sip_initSipAddrSpec(&glbSipParserAddrSpec, \
			SipAddrSipUri,((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl, \
			((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
				YYABORT;
		}
		| SIPSCOLON
		{
			if(sip_initSipAddrSpec(&glbSipParserAddrSpec, \
			SipAddrSipSUri,((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl, \
			((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
				YYABORT;

		};
hostport:	host COLON port
	{
		SIP_S8bit *temp;
		glbSipParserAddrSpec->u.pSipUrl->pHost =  \
			sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		glbSipParserAddrSpec->u.pSipUrl->dPort = (SIP_U16bit *)  \
			fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit),\
			((SipHeaderParserParam *) pHeaderParserParam)->pError);
		if(glbSipParserAddrSpec->u.pSipUrl->dPort == SIP_NULL)
			YYABORT;
		temp = sip_tokenBufferDup($3);
		*(glbSipParserAddrSpec->u.pSipUrl->dPort) = atoi(temp);
		fast_memfree(BISON_MEM_ID, temp, SIP_NULL);
	}
		|host
	{
		glbSipParserAddrSpec->u.pSipUrl->pHost =  \
			sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};
host:		token
	| IPV6ADDR;
port:		digits;
urlparams: 	urlparams urlparam
		|;
urlparam:	semparamtokens EQUALS newalltoken
	{
		
		SipParam *param; 
		int index;		
		SIP_S8bit *yyuparam;
 
		if(sip_initSipParam(& param,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, $1.dLength-index+1, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		yyuparam=sip_tokenBufferDup($3);	
		if(sip_listAppend(&param->slValue,(SIP_Pvoid)yyuparam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
		YYABORT;
		if(sip_listAppend(&(glbSipParserAddrSpec->u.pSipUrl\
			->slParam),(SIP_Pvoid)param,((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;
	}
	| semparamtokens
	{
		
		SipParam *param; 
		int index;		
 
		if(sip_initSipParam(& param,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, $1.dLength-index+1, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		if(sip_listAppend(&(glbSipParserAddrSpec->u.pSipUrl\
			->slParam),(SIP_Pvoid)param,((SipHeaderParserParam *) pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;
	};
	
headers:	QUESTION
	{
		glbSipParserContactAllowSipColon=1;
	}	
	uricdummy
	|
	{
		glbSipParserContactAllowSipColon=0;
	};

uricdummy:	uric
	{
		glbSipParserAddrSpec->u.pSipUrl->pHeader = 
			sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHeader == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		glbSipParserContactAllowSipColon=0;
	};	
	
contactparams:	contactparams contactparam
				|contactparam;
contactparam:	qparam
		|expiresparam
		|extensionattr;
extensionattr:	SEMTOKEN  {
				/* save the token into global Name */
	     SIP_U32bit	index=1;
			 while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
					 index++;
		
			 $1.pToken = &($1.pToken[index]);
			 $1.dLength -= index;
			 glbName = sip_tokenBufferDup($1);
			}	paramlist ;
paramlist :
	{
		SipContactParam *cparam;
		if(sip_initSipContactParam(& cparam,SipCParamExtension,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		/* Ignore initial ';' and whitespace before parameter name */
		cparam->u.pExtensionAttr = glbName;
		glbName = SIP_NULL ;	
		if(cparam->u.pExtensionAttr == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&glbSipParserContactHeader->slContactParam,\
		(SIP_Pvoid)cparam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
					YYABORT;
	}
  |EQUALS alltoken
	{
		SipContactParam *cparam;
		if(sip_initSipContactParam(& cparam,SipCParamExtension,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		cparam->u.pExtensionAttr = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
			strlen(glbName)+$2.dLength+2, SIP_NULL);
		if(cparam->u.pExtensionAttr == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(cparam->u.pExtensionAttr, glbName, strlen(glbName));
		cparam->u.pExtensionAttr[strlen(glbName)] = '=';
		strncpy(&(cparam->u.pExtensionAttr[strlen(glbName)])+1, $2.pToken, \
			$2.dLength);
		cparam->u.pExtensionAttr[strlen(glbName)+$2.dLength+1] = '\0';
		if(sip_listAppend(&glbSipParserContactHeader->slContactParam,\
								(SIP_Pvoid)cparam,&*(((SipHeaderParserParam *) 
								pHeaderParserParam)->pError))!=SipSuccess)
					YYABORT;
		sip_freeString(glbName) ;
	} 
	| EQUALS LANGLE {
		SipContactParam *pCParam=SIP_NULL;
		SipParam *pParam = SIP_NULL ;
		
		if(sip_initSipContactParam(&pCParam,SipCParamFeatureParam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)\
                        ->pError))!=SipSuccess)
			YYABORT;
		if (sip_initSipParam(&pParam, (((SipHeaderParserParam *)\
												pHeaderParserParam)->pError)) == SipFail)
				YYABORT;
		pParam->pName = glbName ;
		glbName = SIP_NULL ;	
		pTagValList = &(pParam->slValue) ;
		pCParam->u.pParam = pParam ;

		if(sip_listAppend(&glbSipParserContactHeader->slContactParam,\
								(SIP_Pvoid)pCParam,&*(((SipHeaderParserParam *) 
								pHeaderParserParam)->pError))!=SipSuccess)
		{
				YYABORT;
		}
	}tagvaluelist RANGLE
	;

tagvaluelist : tagvaluelist COMMA tagvalue
            | tagvalue
		        | QSTRING 
						 { 
								 SIP_S8bit *pValue = SIP_NULL ;
								 SIP_U32bit index=0 ;
								 /* Append the tag value list with $1*/
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;

		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;
								 pValue = sip_tokenBufferDup($1);
								 if (pValue == SIP_NULL)
								 {
										 *(((SipHeaderParserParam *) pHeaderParserParam)->pError) 
												 = E_NO_MEM;
										 YYABORT;
								 }
								 if (sip_listAppend((pTagValList), pValue, \
														 (((SipHeaderParserParam *) pHeaderParserParam)->\
															pError)) == SipFail)
										 YYABORT;

						 }
						 ;

tagvalue     : TOKEN
		{
	         SIP_S8bit *pValue = SIP_NULL ;
		       SIP_U32bit index=0 ;
		      /* Append the tag value list with $1*/
		       while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				        index++;

					 $1.pToken = &($1.pToken[index]);
					 $1.dLength -= index;
					 pValue = sip_tokenBufferDup($1);
					 if (pValue == SIP_NULL)
					 {
									 *(((SipHeaderParserParam *) pHeaderParserParam)->pError) 
													 = E_NO_MEM;
			YYABORT;
		}
					 if (sip_listAppend((pTagValList), pValue, \
													 (((SipHeaderParserParam *) pHeaderParserParam)->\
														pError)) == SipFail)
									 YYABORT;
				 }
         | numeric
				 ;

numeric : HASH numeric_relation digit
	    {
			  SIP_S8bit *pValue = SIP_NULL ;

				/* Append the tag value list with $1*/
				pValue = (SIP_S8bit*) FLEXMALLOC(strlen($2.pToken)+3) ;
				if (pValue == SIP_NULL)
				{
								*(((SipHeaderParserParam *) pHeaderParserParam)->pError) 
												= E_NO_MEM;
			YYABORT;
				}
				strcpy(pValue,"#") ;
				strcat(pValue,$2.pToken) ;
				/*strcat(pValue,$3.pToken) ;
				 * */
				if (sip_listAppend((pTagValList), pValue, \
						(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
					YYABORT;
			}
       ;
						 
numeric_relation : NUMERIC_RELATION 
                 | EQUALS
								 | digit COLON
						     ;
								 
digit : DIGITS
      | DIGIT2
			| DIGIT4
      | SIPFLOAT
			;

qparam:		QVAL EQUALS float
	{
		SipContactParam *cparam;
		if(sip_initSipContactParam(& cparam,SipCParamQvalue,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		cparam->u.pQValue = sip_tokenBufferDup($3);
		if(cparam->u.pQValue == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&glbSipParserContactHeader->slContactParam, \
			(SIP_Pvoid)cparam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess) 
			YYABORT;
	};
expiresparam: EXPIRES EQUALS DOUBLEQUOTES sipdate DOUBLEQUOTES
	{
		SipContactParam *cparam;
		if(sip_initSipContactParam(& cparam,SipCParamExpires, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipExpiresStruct(& cparam->u.pExpire,SipExpDate, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		cparam->u.pExpire->u.pDate = glbSipParserDateStruct;
		glbSipParserDateStruct=SIP_NULL;
		
		if(sip_listAppend(&glbSipParserContactHeader->slContactParam, \
			(SIP_Pvoid)cparam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

	}
		|EXPIRES EQUALS digits
	{
		SipContactParam *cparam;
		SIP_S8bit *temp;

		if(sip_initSipContactParam(& cparam,SipCParamExpires,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipExpiresStruct(& cparam->u.pExpire,SipExpSeconds, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		temp = sip_tokenBufferDup($3);
		/* Modified STRTOLCAP to STRTOU32CAP. */
		cparam->u.pExpire->u.dSec = (SIP_U32bit)STRTOU32CAP(temp, SIP_NULL);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		if(sip_listAppend(&glbSipParserContactHeader->slContactParam, \
			(SIP_Pvoid)cparam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
sipdate:	WEEKDAY COMMA date time GMT 
	{
		if(strncasecmp((const SIP_S8bit *) $1.pToken,"Sun",3)==0) 
		 	glbSipParserDateStruct->dDow = SipDaySun;
		else if(strncasecmp((const SIP_S8bit *)$1.pToken,"Mon",3)==0)
		 	glbSipParserDateStruct->dDow = SipDayMon;
		else if(strncasecmp((const SIP_S8bit *)$1.pToken,"Tue",3)==0)
		 	glbSipParserDateStruct->dDow = SipDayTue;
		else if(strncasecmp((const SIP_S8bit *)$1.pToken,"Wed",3)==0)
		 	glbSipParserDateStruct->dDow = SipDayWed;
		else if(strncasecmp((const SIP_S8bit *)$1.pToken,"Thu",3)==0)
		 	glbSipParserDateStruct->dDow = SipDayThu;
		else if(strncasecmp((const SIP_S8bit *)$1.pToken,"Fri",3)==0)
		 	glbSipParserDateStruct->dDow = SipDayFri;
		else if(strncasecmp((const SIP_S8bit *)$1.pToken,"Sat",3)==0)
 			glbSipParserDateStruct->dDow = SipDaySat;
	}
		| date time GMT
	{
			glbSipParserDateStruct->dDow = SipDayNone;
	};
date:		digits MONTH digits24
	{
		SIP_S8bit *temp;

		if(sip_initSipDateStruct(&glbSipParserDateStruct,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))== SipFail)
			YYABORT;
		if(sip_initSipDateFormat(&(glbSipParserDateStruct->pDate),\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;

		temp = sip_tokenBufferDup($1);
 		glbSipParserDateStruct->pDate->dDay = atoi(temp);

		if(atoi(temp)>SIP_MAX_MONTH_DAYS)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=E_PARSER_ERROR;
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			sip_freeSipDateStruct(glbSipParserDateStruct);
			glbSipParserDateStruct=SIP_NULL;
				YYABORT;
		}

		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);

		if(strncasecmp((const SIP_S8bit *)$2.pToken,"Jan",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthJan;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Feb",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthFeb;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Mar",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthMar;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Apr",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthApr;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"May",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthMay;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Jun",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthJun;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Jul",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthJul;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Aug",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthAug;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Sep",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthSep;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Oct",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthOct;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Nov",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthNov;
		else if(strncasecmp((const SIP_S8bit *)$2.pToken,"Dec",3)==0)
			glbSipParserDateStruct->pDate->dMonth = SipMonthDec;

		temp = sip_tokenBufferDup($3);
	 	glbSipParserDateStruct->pDate->dYear = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	};
digits24:DIGIT2
		|DIGIT4;
time: DIGIT2 COLON DIGIT2 COLON DIGIT2
	{
		SIP_S8bit *temp;
		if(sip_initSipTimeFormat(&(glbSipParserDateStruct->pTime),\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		temp = sip_tokenBufferDup($1);
		glbSipParserDateStruct->pTime->dHour = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		temp = sip_tokenBufferDup($3);
		glbSipParserDateStruct->pTime->dMin = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		temp = sip_tokenBufferDup($5);
		glbSipParserDateStruct->pTime->dSec = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	}
	| DIGIT2 COLON DIGIT2 
	{
		SIP_S8bit *temp;	

		if(sip_initSipTimeFormat(&(glbSipParserDateStruct->pTime),\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		temp = sip_tokenBufferDup($1);
		glbSipParserDateStruct->pTime->dHour = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		temp = sip_tokenBufferDup($3);
		glbSipParserDateStruct->pTime->dMin = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	};
alltoken:	token
		|QSTRING
		|IPV6ADDR
		|SEMPARAMVALUE;
	
token:	TOKEN
		|digits
		|MONTH
		|WEEKDAY
		|GMT
		|STAR
		|SIPFLOAT;
uricnoparam:	uricnoparam uricnocolonnoparam
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
		|uricnoparam COLON	
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
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
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.dLength,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->pGCList,\
			(SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
		|uric COLON	
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength+$2.dLength,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *) pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
		|uricnocolon 
		{
		}
		|COLON
		{
		};
callidword: callidtoken;
callidtoken: callidtoken callidparam
		{
			$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,\
				$1.dLength+$2.dLength, &*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError));
			if(sip_listAppend(((SipHeaderParserParam *)pHeaderParserParam)->\
				pGCList, (SIP_Pvoid) $$.pToken, &*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError)) == SipFail)
				YYABORT;
			if($$.pToken == SIP_NULL)
				YYABORT;
			strncpy($$.pToken, $1.pToken, $1.dLength);
			strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
			$$.dLength = $1.dLength+$2.dLength;
		}
		| callidparam;
callidparam: RANGLE
			|DISPNAME
			|OPENBRACE
			|CLOSEBRACE
			|DOUBLEQUOTES
			|BACKSLASH
			|TOKEN
			|float
			|URICNOCOLON
			|USERINFOAT
			|AMPERSAND
			|AT
			|EQUALS
			|LANGLE
			|SEMICOLON
			|QUESTION
			|STAR
			|QSTRING1
			|IPV6ADDR
			|COMMA
			|SEMPARAMVALUE
			|SEMPARAMTOKEN
			|PLAINSIPCOLON
			|COLON;

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
			|STAR
			|QSTRING1
			|IPV6ADDR
			|COMMA
			|SEMPARAMVALUE
			|SEMPARAMTOKEN
			|PLAINSIPCOLON;
uricnocolonnoparam:	TOKEN
			|float
			|URICNOCOLON
			|USERINFOAT	
			|AMPERSAND 	
			|AT	 
			|EQUALS 	
			|QUESTION
			|STAR
			|IPV6ADDR;
digits:	DIGITS
		|DIGIT2		
		|DIGIT4;		
float:	digits	
		|SIPFLOAT;
semparamtokens:SEMPARAMTOKEN 
			|SEMTOKEN
			|QVAL
			|EXPIRES;
newalltoken:SEMPARAMVALUE
		|token
		| IPV6ADDR
		|QSTRING;
		
callidcolon:	CALLID
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->pGeneralHdr->\
			pCallidHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one callid header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};

join:  joincolon joincallid joinparams
	    |  joincolon joincallid	;
		
joincolon: JOIN
	{
#ifdef SIP_CONF        
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
                       pSipMessage->dType!=SipMessageRequest)
       {
				*PARSER_ERROR_VAR = E_NO_ERROR;
                YYABORT;
       }
		/* if Join header already exists */	
		if(JOIN_HEADER)
		{
			sip_error (SIP_Minor,\
				"There can only be one JOIN header");
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}
#endif
	};

joincallid: callidword
	{
#ifdef SIP_CONF        
		SipHeaderOrderInfo *order;
		SIP_S8bit* pTemp;

		if(sip_initSipHeaderOrderInfo(&order,PARSER_ERROR_VAR) == SipFail)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		order->dType = SipHdrTypeJoin;
		order->dTextType = SipFormFull;
		order->dNum = 1;

		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
			PARSER_ERROR_VAR) !=SipSuccess)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		if(sip_initSipJoinHeader(&JOIN_HEADER,PARSER_ERROR_VAR)== SipFail)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}

		pTemp = sip_tokenBufferDup($1);
		if(pTemp == SIP_NULL)
		{
			*PARSER_ERROR_VAR = E_NO_MEM;
			YYABORT;
		}
		JOIN_HEADER->pCallid = pTemp;
#endif
	};

joinparams:joinparams joinparam
		|joinparam;

joinparam:  joinfromtag 
        	|jointotag 
			| joingenvalue;

joingenvalue: SEMTOKEN
		{
#ifdef SIP_CONF            
			SIP_S8bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
			while((*$1.pToken == ' ') || (*$1.pToken == '\t'))
				index++;
			$1.pToken += index;
			$1.dLength -= index;
			pParam->pName = sip_tokenBufferDup($1);
			if(sip_listAppend(&JOIN_HEADER->slParams,pParam,\
					PARSER_ERROR_VAR)	== SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
#endif
		}
		|SEMTOKEN  EQUALS newalltoken
		{
#ifdef SIP_CONF            
			SIP_S8bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
			while((*$1.pToken == ' ') || (*$1.pToken == '\t'))
				index++;
			$1.pToken += index;
			$1.dLength -= index;
			pParam->pName = sip_tokenBufferDup($1);
			pParam->pValue = sip_tokenBufferDup($3);

			if(sip_listAppend(&JOIN_HEADER->slParams,pParam,\
					PARSER_ERROR_VAR)	== SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
#endif
		};

joinfromtag:  FROMTAG EQUALS newalltoken
		{
#ifdef SIP_CONF            
			SIP_S8bit* pTemp;
			/* if we already got an from-tag return parse error */
			if(JOIN_HEADER->pFromTag)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
			
			pTemp = sip_tokenBufferDup($3);
			if(pTemp == SIP_NULL)
			{
				*PARSER_ERROR_VAR = E_NO_MEM;
				YYABORT;
			}
			JOIN_HEADER->pFromTag = pTemp;
#endif
		};

jointotag:  TOTAG EQUALS newalltoken
		{
#ifdef SIP_CONF            
			SIP_S8bit* pTemp;
			/* if we already got an to-tag return parse error */
			if(JOIN_HEADER->pToTag)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
			pTemp = sip_tokenBufferDup($3);
			if(pTemp == SIP_NULL)
			{
				*PARSER_ERROR_VAR = E_NO_MEM;
				YYABORT;
			}
			JOIN_HEADER->pToTag = pTemp;
#endif
		};
