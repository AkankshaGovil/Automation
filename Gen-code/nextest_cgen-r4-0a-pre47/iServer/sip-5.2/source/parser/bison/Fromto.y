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

%token FROM TO ROUTE RECORDROUTE COMMA COLON SEMICOLON RANGLE TAG PASSERTEDID
%token EQUALS PLAINSIPCOLON SIPCOLON SIPSCOLON DIGITS SEMPARAMTOKEN SEMPARAMVALUE
%token QSTRING QSTRING1 TOKEN PPREFERREDID PCVECTOR ICID PANINFO
%token URICNOCOLON QUESTION PATH
%token DISPNAME USERINFOAT SEMTOKEN IPV6ADDR REFERTO REFERBY REFTOKEN
%token MSGID CID SEPARATOR SERVICEROUTE
%token DEFAULT QUOTE
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
#ifdef SIP_3GPP
#define PARSER_ERROR_VAR ((SipHeaderParserParam *)pHeaderParserParam)->pError
#endif				  

static SipParam *		glbSipParserSipParam=SIP_NULL;
static SipRouteHeader *	glbSipRouteHeader=SIP_NULL;
static SipRecordRouteHeader *	glbSipRecordRouteHeader=SIP_NULL;
#ifdef SIP_3GPP
en_HeaderType glbHdrType = SipHdrTypeAny;
static SipPathHeader *	glbSipPathHeader=SIP_NULL;
static SipPanInfoHeader * glbSipParserPanInfo=SIP_NULL;
static SipPcVectorHeader * glbSipParserPcVector=SIP_NULL;
#endif
static SipAddrSpec *glbSipParserAddrSpec=SIP_NULL;
static SipFromHeader *glbSipParserFromHeader = SIP_NULL;
static SipToHeader *glbSipParserToHeader = SIP_NULL;
SipReferredByHeader *glbSipParserReferredByHeader = SIP_NULL; 
static SipReferToHeader *glbSipParserReferToHeader = SIP_NULL; 
extern int glbSipParserFromToAllowSipColon;
extern int glbSipAssertedIdNameAddrStarted;
extern int glbSipPPreferredIdNameAddrStarted;
#ifdef SIP_PRIVACY
static SipPAssertIdHeader * glbSipParserPAssertIdHeader = SIP_NULL ;
static en_HeaderForm	glbSipParserPAssertIdType = SipFormFull ;
static SipPPreferredIdHeader * glbSipParserPPreferredIdHeader = SIP_NULL ;
static en_HeaderForm	glbSipParserPPreferredIdType = SipFormFull ;
#endif
#define YYPARSE_PARAM pHeaderParserParam
%}
%%
header:from
		| route
		| recordroute
		| path
		| referto
		| PcVector
		| referby
        | p_asserted_id 
        | p_preferred_id 
		| PanInfo
		|error
		{
			if(glbSipParserAddrSpec != SIP_NULL)
			{
				sip_freeSipAddrSpec(glbSipParserAddrSpec);
				glbSipParserAddrSpec=SIP_NULL;
			}
			if(glbSipRouteHeader != SIP_NULL)
			{
				sip_freeSipRouteHeader(glbSipRouteHeader);
				glbSipRouteHeader = SIP_NULL;
			}
			if(glbSipRecordRouteHeader != SIP_NULL)
			{
				sip_freeSipRecordRouteHeader(glbSipRecordRouteHeader);
				glbSipRecordRouteHeader = SIP_NULL;
			}
#ifdef SIP_3GPP		
			if(glbSipPathHeader != SIP_NULL)
			{
				sip_freeSipPathHeader(glbSipPathHeader);
				glbSipPathHeader = SIP_NULL;
			}
           if(glbSipParserPanInfo!= SIP_NULL)
			{
				sip_freeSipPanInfoHeader(glbSipParserPanInfo);
				glbSipParserPanInfo = SIP_NULL;
			}

            if(glbSipParserPcVector!= SIP_NULL)
			{
				sip_freeSipPcVectorHeader(glbSipParserPcVector);
				glbSipParserPcVector = SIP_NULL;
			}
#endif
			if(glbSipParserSipParam != SIP_NULL)
			{
				sip_freeSipParam(glbSipParserSipParam);
				glbSipParserSipParam = SIP_NULL;
			}
			if(glbSipParserFromHeader != SIP_NULL)
			{
				sip_freeSipFromHeader(glbSipParserFromHeader);
				glbSipParserFromHeader = SIP_NULL;
			}
			if(glbSipParserToHeader != SIP_NULL)
			{
				sip_freeSipToHeader(glbSipParserToHeader);
				glbSipParserToHeader = SIP_NULL;
			}
			if(glbSipParserReferredByHeader != SIP_NULL)
			{
				sip_freeSipReferredByHeader(glbSipParserReferredByHeader);
				glbSipParserReferredByHeader = SIP_NULL;
			}
			if(glbSipParserReferToHeader != SIP_NULL)
			{
				sip_freeSipReferToHeader(glbSipParserReferToHeader);
				glbSipParserReferToHeader = SIP_NULL;
			}
#ifdef SIP_PRIVACY
			if(glbSipParserPAssertIdHeader != SIP_NULL)
			{
				sip_freeSipPAssertIdHeader(glbSipParserPAssertIdHeader);
				glbSipParserPAssertIdHeader = SIP_NULL;
			}
			if(glbSipParserPPreferredIdHeader != SIP_NULL)
			{
				sip_freeSipPPreferredIdHeader(glbSipParserPPreferredIdHeader);
				glbSipParserPPreferredIdHeader = SIP_NULL;
			}
#endif
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					=E_PARSER_ERROR;
		};
referby:REFERBY
		{
			if(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->dType!=SipMessageRequest)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					 = E_NO_ERROR;
					YYABORT;
			}
			if (((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->\
				pReferredByHdr != SIP_NULL)
			{
				sip_error(SIP_Minor,\
				 	"There can be only one Referred-By header");
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					 = E_PARSER_ERROR;
				YYABORT;
			}
			if(sip_initSipReferredByHeader(&glbSipParserReferredByHeader,\
				&*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;

		}referbyvalue
		{
			SipHeaderOrderInfo *order;
			SipBool	dResult;
			
			((SipHeaderParserParam *)pHeaderParserParam)->pSipMessage->\
				u.pRequest->pRequestHdr->pReferredByHdr = \
				glbSipParserReferredByHeader;
			glbSipParserReferredByHeader = SIP_NULL;

			dResult=sip_initSipHeaderOrderInfo(&order,\
				((SipHeaderParserParam *)pHeaderParserParam)->pError);
			if (dResult!=SipSuccess)
			{
				YYABORT;
			}
			
			order->dType = SipHdrTypeReferredBy;
			if($1.dChar=='f')
			order->dTextType = SipFormFull;
			else
			order->dTextType = SipFormShort;
			order->dNum = 1;
			if(sip_listAppend(&((SipHeaderParserParam *) \
				pHeaderParserParam)->pSipMessage->slOrderInfo, \
				(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		};
referbyvalue: referrerurl refparam 
			| referrerurl;

/* NEXTONE - add tag to refparam */
refparam: signatureparams
		| referencedurl
		| referencedurl signatureparams
		| signatureparams referencedurl
		| tag;


cidtoken: CID EQUALS msgid
	{
		if (glbSipParserReferredByHeader->pMsgId==SIP_NULL)
		{
			glbSipParserReferredByHeader->pMsgId = sip_tokenBufferDup($3);
		}
	  fast_memfree(BISON_MEM_ID,$3.pToken,SIP_NULL) ;
	}; 
	
referrerurl:addrspecnoparam
	{
		glbSipParserReferredByHeader->pAddrSpecReferrer = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	}
	| dispname addrspec RANGLE 
	{
		glbSipParserReferredByHeader->pDispName = sip_tokenBufferDup($1);
		glbSipParserReferredByHeader->pAddrSpecReferrer = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	};

signatureparams : signatureparams signatureparam
			| signatureparam;


signatureparam:extparam
	{
		if(sip_listAppend( &(glbSipParserReferredByHeader->slParams),\
			(SIP_Pvoid)glbSipParserSipParam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserSipParam=SIP_NULL;
	}| cidtoken; 

referencedurl:REFTOKEN addrspec RANGLE
	{
		glbSipParserReferredByHeader->pAddrSpecReferenced=glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	};


referto:REFERTO 
		{
			if(((SipHeaderParserParam *) pHeaderParserParam)-> \
				pSipMessage->dType!=SipMessageRequest)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)-> \
					pError) = E_NO_ERROR;
					YYABORT;
			}
			if (((SipHeaderParserParam *) pHeaderParserParam)-> \
				pSipMessage->u.pRequest->pRequestHdr->\
				pReferToHdr != SIP_NULL)
			{
				sip_error(SIP_Minor, "There can be only one refer to"\
					" header");
				*(((SipHeaderParserParam *) pHeaderParserParam)-> \
					pError) = E_PARSER_ERROR;
				YYABORT;
			}
			if(sip_initSipReferToHeader(&glbSipParserReferToHeader,\
				&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
					YYABORT;
		}
		refertofields
		{
			SipHeaderOrderInfo *order;
			SipBool	dResult;
	
			((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->\
				pReferToHdr= glbSipParserReferToHeader;
			glbSipParserReferToHeader = SIP_NULL;	

			dResult=sip_initSipHeaderOrderInfo(&order,\
				((SipHeaderParserParam *)pHeaderParserParam)->pError);
			if (dResult!=SipSuccess)
			{
				YYABORT;
			}				
			
			order->dType = SipHdrTypeReferTo;
			if($1.dChar=='f')
				order->dTextType = SipFormFull;
			else
				order->dTextType = SipFormShort;

			order->dNum = 1;
			if(sip_listAppend(&((SipHeaderParserParam *) \
				pHeaderParserParam)->pSipMessage->slOrderInfo, \
				(SIP_Pvoid)order, (((SipHeaderParserParam *) \
				 pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		};

refertofields : refernameaddr;
		
refernameaddr : addrspec
		{
			glbSipParserReferToHeader->pAddrSpec= glbSipParserAddrSpec;
			glbSipParserAddrSpec = SIP_NULL;
		}
   |nameaddr
   |nameaddr referparams;
nameaddr: dispname addrspec RANGLE
		{
			glbSipParserReferToHeader->pDispName =  sip_tokenBufferDup($1);
			if(glbSipParserReferToHeader->pDispName ==SIP_NULL)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =  \
				E_NO_MEM;
				YYABORT;
			}
			glbSipParserReferToHeader->pAddrSpec = glbSipParserAddrSpec;
			glbSipParserAddrSpec = SIP_NULL;
		};

referparams : referparams extparam
		{
			if(sip_listAppend( \
				&(glbSipParserReferToHeader->slParams), (SIP_Pvoid) \
				glbSipParserSipParam,(((SipHeaderParserParam *) \
				 pHeaderParserParam)->pError)) !=SipSuccess)
				YYABORT;
			glbSipParserSipParam=SIP_NULL;
		}
		| extparam
		{
			if(sip_listAppend( \
				&(glbSipParserReferToHeader->slParams), (SIP_Pvoid) \
				glbSipParserSipParam,(((SipHeaderParserParam *) \
				 pHeaderParserParam)->pError)) !=SipSuccess)
				YYABORT;
			glbSipParserSipParam=SIP_NULL;
		};

route:	ROUTE
		{
			if(((SipHeaderParserParam *) pHeaderParserParam)-> \
				pSipMessage->dType!=SipMessageRequest)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					 = E_NO_ERROR;
				YYABORT;
			}

		}  routes;

routes: routefields
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
	
		order->dType = SipHdrTypeRoute;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
			slRouteHdr, (SIP_Pvoid)glbSipRouteHeader, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
				YYABORT;
		glbSipRouteHeader = SIP_NULL;
	}
		| routes COMMA routefields
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo, \
			&ordersize,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
				YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
				YYABORT;
		order->dNum++;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
			slRouteHdr,(SIP_Pvoid)glbSipRouteHeader, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
				YYABORT;		
		glbSipRouteHeader = SIP_NULL;
	};
routefields: dispname addrspec RANGLE
	{
		/* SipRouteHeader *routehdr; */
		if(sip_initSipRouteHeader(&glbSipRouteHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		glbSipRouteHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipRouteHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
				= E_NO_MEM;
			YYABORT;
		}
		glbSipRouteHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	}
	| dispname addrspec RANGLE 
	{
		/* SipRouteHeader *routehdr; */
		if(sip_initSipRouteHeader(&glbSipRouteHeader, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
				YYABORT;
		glbSipRouteHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipRouteHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				 = E_NO_MEM;
			YYABORT;
		}
		glbSipRouteHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	} rparams;
rparams: rparam
		| rparams  rparam;
rparam: routesemtoken
	{
		SipParam *pParam;
		SIP_U32bit index;

		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				 = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipRouteHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
		| routesemtoken EQUALS allstring
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		SIP_U32bit index;
		
		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *)  \
				pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =  \
				E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,  \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;

		if(sip_listAppend(&(glbSipRouteHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}; 
recordroute:	RECORDROUTE recordroutes;
recordroutes: recordroutefields 
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}			
		
		order->dType = SipHdrTypeRecordRoute;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo, (SIP_Pvoid)order, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
 		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->pGeneralHdr-> slRecordRouteHdr,(SIP_Pvoid) \
			glbSipRecordRouteHeader, (((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		glbSipRecordRouteHeader = SIP_NULL;
	}
		| recordroutes COMMA recordroutefields
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo,&ordersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))  \
			== SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo, ordersize-1,(SIP_Pvoid *)&order, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) \
			 == SipFail)
			YYABORT;
		order->dNum++;
 		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->pGeneralHdr-> slRecordRouteHdr, \
			(SIP_Pvoid)glbSipRecordRouteHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		glbSipRecordRouteHeader = SIP_NULL;
	}
		|recordroutes COMMA;
recordroutefields: dispname addrspec RANGLE
	{
		/* SipRecordRouteHeader *routehdr;*/
		if(sip_initSipRecordRouteHeader(&glbSipRecordRouteHeader,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		glbSipRecordRouteHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipRecordRouteHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_NO_MEM;
			YYABORT;
		}
		glbSipRecordRouteHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	} rrparams
		| dispname addrspec RANGLE
	{
		/* SipRecordRouteHeader *routehdr;*/
		if(sip_initSipRecordRouteHeader(&glbSipRecordRouteHeader,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipRecordRouteHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipRecordRouteHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
				E_NO_MEM;
			YYABORT;
		}
		glbSipRecordRouteHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	};
rrparams: rrparam
		| rrparams  rrparam;
rrparam: routesemtoken
	{
		SipParam *pParam;
		SIP_U32bit index;

		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipRecordRouteHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
		| routesemtoken EQUALS allstring
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		SIP_U32bit index;
		 
		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
				E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) ==SipFail)
			YYABORT;
		if(sip_listAppend(&(glbSipRecordRouteHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	};
allstring: token
		| QSTRING
		|IPV6ADDR;
from:	fromcolon 
	{ 
		if(sip_initSipFromHeader( &glbSipParserFromHeader,\
			&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
		fromfields
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;

		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
			pGeneralHdr->pFromHeader = glbSipParserFromHeader;
		glbSipParserFromHeader = SIP_NULL;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}		
			
		order->dType = SipHdrTypeFrom;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else 
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo, (SIP_Pvoid)order, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
	};
fromfields:	fromnameaddr fromaddrparams
		|fromnameaddr; 
fromnameaddr:	addrspecnoparam
	{
		glbSipParserFromHeader->pAddrSpec =  glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	}
		|dispname addrspec RANGLE
	{
		glbSipParserFromHeader->pDispName =  sip_tokenBufferDup($1);
		if(glbSipParserFromHeader->pDispName ==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =  \
				E_NO_MEM;
			YYABORT;
		}
		glbSipParserFromHeader->pAddrSpec =  \
			glbSipParserAddrSpec;
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
routesemtoken: SEMTOKEN
	|TAG;
semtoken:SEMTOKEN;
semparamtokens:	SEMPARAMTOKEN 
			|SEMTOKEN 
			|TAG;
		
headers:	QUESTION 
			{
				glbSipParserFromToAllowSipColon=1;
			}
			uricdummy
			|
			{
				glbSipParserFromToAllowSipColon=0;
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
			glbSipParserFromToAllowSipColon=0;
		};	
		
fromaddrparams:	fromaddrparams tag
	{ 
		SIP_S8bit *tmp;
		tmp = sip_tokenBufferDup($2);
		if(tmp==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =  \
				E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserFromHeader->slTag),\
			tmp,(((SipHeaderParserParam *) \
			pHeaderParserParam)-> pError))!=SipSuccess)
			YYABORT;
	}
		| fromaddrparams extparam
	{
		if(sip_listAppend( \
			&(glbSipParserFromHeader->slParam), (SIP_Pvoid) \
			glbSipParserSipParam,(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))\
				!=SipSuccess)
				YYABORT;
				glbSipParserSipParam=SIP_NULL;
	}
		|extparam
	{ 
		if(sip_listAppend( \
			&(glbSipParserFromHeader->slParam), (SIP_Pvoid) \
			glbSipParserSipParam,(((SipHeaderParserParam *) \
			 pHeaderParserParam)->pError)) !=SipSuccess)
			YYABORT;
			glbSipParserSipParam=SIP_NULL;
	}
		|tag
	{ 
		SIP_S8bit *tmp;
		tmp = (SIP_S8bit*) sip_tokenBufferDup($1);
		if(tmp==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( &(glbSipParserFromHeader->slTag),\
			tmp,(((SipHeaderParserParam *) \
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
		|digits		
		|REFTOKEN;


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


msgid:	msgid msgidvalue
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
    fast_memfree(BISON_MEM_ID,$2.pToken,SIP_NULL) ;
	} 
		|msgidvalue
		{
				$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength \
								,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
								pError));
				if($$.pToken == SIP_NULL)
						YYABORT;
				strncpy($$.pToken, $1.pToken, $1.dLength);
				$$.dLength = $1.dLength;
    fast_memfree(BISON_MEM_ID,$1.pToken,SIP_NULL) ;
	  } 
		; 

msgidvalue: QUOTE USERINFOAT restvalue QUOTE
				{
						$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$2.dLength \
								+$3.dLength+3\
										,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
												pError));
						if($$.pToken == SIP_NULL)
								YYABORT;
						strncpy($$.pToken,"\"",1);
						strncpy(&($$.pToken[1]), $2.pToken, $2.dLength);
						strncpy(&($$.pToken[$2.dLength+1]), $3.pToken, $3.dLength);
						strncpy(&($$.pToken[$2.dLength+$3.dLength+1]),"\"",1);
						strncpy(&($$.pToken[$2.dLength+$3.dLength+2]),"\0",1);
						$$.dLength = $2.dLength + $3.dLength+2;
				}
         ;
restvalue: TOKEN	
		|IPV6ADDR
		|digits   
		;
		
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
fromcolon:FROM
	{
        if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pFromHeader!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one From Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
			E_PARSER_ERROR;
			YYABORT;
		}
	};

p_asserted_id : PASSERTEDID pAssertId_comma_list ;


pAssertId_comma_list : pAssertId_list 
					 | pAssertId_list SEPARATOR;
					 
pAssertId_list : pAssertId_value
    {
#ifdef SIP_PRIVACY
		/* One value of type PAssertId is added */
		SipHeaderOrderInfo *pOrder = SIP_NULL ;
		SipBool dResult = SipSuccess ;

		dResult=sip_initSipHeaderOrderInfo(&pOrder,\
	    ((SipHeaderParserParam *)pHeaderParserParam)->pError);

		if (dResult!=SipSuccess)
					YYABORT;

		pOrder->dType     = SipHdrTypePAssertId;
		pOrder->dTextType = glbSipParserPAssertIdType;
		pOrder->dNum      = 1;

		if(sip_listAppend(&((SipHeaderParserParam *) 
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slPAssertIdHdr,\
			glbSipParserPAssertIdHeader,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
							YYABORT;
		glbSipParserPAssertIdHeader = SIP_NULL;
					
		if(sip_listAppend(&((SipHeaderParserParam *)pHeaderParserParam)->\
			 pSipMessage->slOrderInfo, (SIP_Pvoid)pOrder, \
			 &*(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
			 !=SipSuccess)
						  YYABORT;
#endif
    }
		| pAssertId_list SEPARATOR pAssertId_value 
    {
#ifdef SIP_PRIVACY                     
        /* SipList is being made of type SipHeaderPAsssertId structure
           and thereby working on slPAssertIdHdr of General Header */
		SipHeaderOrderInfo * pOrder = SIP_NULL ;
        SIP_U32bit orderSize = 0 ;
                    
		if(sip_listAppend( &((SipHeaderParserParam *)
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slPAssertIdHdr,\
			glbSipParserPAssertIdHeader,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) !=SipSuccess)
					YYABORT;
		glbSipParserPAssertIdHeader = SIP_NULL;
				
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,&orderSize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
					YYABORT;

		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,orderSize-1, \
			(SIP_Pvoid *)&pOrder,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
					YYABORT;
					pOrder->dNum++;
#endif
		}
		; 

pAssertId_value  :addrspecnoparam
	{
#ifdef SIP_PRIVACY
		if ( sip_initSipPAssertIdHeader( &glbSipParserPAssertIdHeader, 
       		& * ( ( (SipHeaderParserParam*)pHeaderParserParam)->pError) ) 
	    	 != SipSuccess)
		               YYABORT ;
        /* Adding addspecnoparam to global structure of PAssertId */
		glbSipParserPAssertIdHeader->pAddrSpec = glbSipParserAddrSpec ;
		glbSipParserAddrSpec = SIP_NULL ;
#endif
	}
		| dispname
		{
			glbSipAssertedIdNameAddrStarted=1;
		}
		addrspec RANGLE
	{
#ifdef SIP_PRIVACY
		if ( sip_initSipPAssertIdHeader( &glbSipParserPAssertIdHeader, 
      		& * ( ( (SipHeaderParserParam*)pHeaderParserParam)->pError) ) 
      		 != SipSuccess)
		                      YYABORT ;
         glbSipParserPAssertIdHeader->pDispName = sip_tokenBufferDup($1) ;
			
        /* Adding addspecnoparam to global structure of PAssertId */
		if ( glbSipParserPAssertIdHeader->pDispName == SIP_NULL )
        {
            *(((SipHeaderParserParam *)pHeaderParserParam)->pError)=E_NO_MEM;
                             YYABORT ;
         }
         glbSipParserPAssertIdHeader->pAddrSpec = glbSipParserAddrSpec ;
	     glbSipParserAddrSpec = SIP_NULL ;
		 glbSipAssertedIdNameAddrStarted = 0;
#endif
	};
service_path_colon : SERVICEROUTE  { 
#ifdef SIP_3GPP
		glbHdrType = SipHdrTypeServiceRoute ; 
#endif
		}
             |  PATH  {
#ifdef SIP_3GPP
					 glbHdrType = SipHdrTypePath ; 
#endif
	 } ;

path:	service_path_colon paths;
paths: pathfields 
	{
#ifdef SIP_3GPP
		SipHeaderOrderInfo *pOrder;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&pOrder,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}			
		
		if ( glbHdrType == SipHdrTypePath )
		{
		pOrder->dType = SipHdrTypePath;
		pOrder->dTextType = SipFormFull;
		pOrder->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo, (SIP_Pvoid)pOrder, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
 		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->pGeneralHdr-> slPathHdr,(SIP_Pvoid) \
			glbSipPathHeader, (((SipHeaderParserParam *)  \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		}
		else if ( glbHdrType == SipHdrTypeServiceRoute )
    {
				pOrder->dType = SipHdrTypeServiceRoute ;
				pOrder->dTextType = SipFormFull;
				pOrder->dNum = 1;
				if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
										pSipMessage->slOrderInfo, (SIP_Pvoid)pOrder, \
										(((SipHeaderParserParam *) pHeaderParserParam)-> \
										 pError))!=SipSuccess)
						YYABORT;
				if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
										pSipMessage->pGeneralHdr->slServiceRouteHdr,(SIP_Pvoid) \
										glbSipPathHeader, (((SipHeaderParserParam *)\
										pHeaderParserParam)->pError)) !=SipSuccess)
						YYABORT;
		}
		else
		{
						YYABORT;
		}
		glbSipPathHeader = SIP_NULL;
#endif
	}
		| paths COMMA pathfields
	{
#ifdef SIP_3GPP
		SipHeaderOrderInfo *pOrder;
		SIP_U32bit dOrdersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo,&dOrdersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))  \
			== SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->slOrderInfo, dOrdersize-1,(SIP_Pvoid *)&pOrder, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) \
			 == SipFail)
			YYABORT;
		pOrder->dNum++;


		if ( glbHdrType == SipHdrTypePath )
		{
 		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
			pSipMessage->pGeneralHdr-> slPathHdr, \
			(SIP_Pvoid)glbSipPathHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		}
		else if (  glbHdrType == SipHdrTypeServiceRoute )
		{
				if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)-> \
										pSipMessage->pGeneralHdr-> slServiceRouteHdr, \
										(SIP_Pvoid)glbSipPathHeader,\
										(((SipHeaderParserParam *) pHeaderParserParam)-> \
										 pError))!=SipSuccess)
						YYABORT;
		}
		else
		{
						YYABORT;
		}
		glbSipPathHeader = SIP_NULL;
#endif
	}
		|paths COMMA;
pathfields: dispname addrspec RANGLE
	{

#ifdef SIP_3GPP
		if(sip_initSipPathHeader(&glbSipPathHeader,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
		glbSipPathHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipPathHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_NO_MEM;
			YYABORT;
		}
		glbSipPathHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
#endif
	} pathparams
		| dispname addrspec RANGLE
	{
#ifdef SIP_3GPP
		if(sip_initSipPathHeader(&glbSipPathHeader,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipPathHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipPathHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
				E_NO_MEM;
			YYABORT;
		}
		glbSipPathHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
#endif
	};
pathparams: pathparam
		| pathparams  pathparam;
pathparam: routesemtoken
	{
#ifdef SIP_3GPP
		SipParam *pParam;
		SIP_U32bit dIndex;

		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		dIndex=1;
		while(($1.pToken[dIndex]==' ')||($1.pToken[dIndex]=='\t'))
			dIndex++;
		$1.pToken = &($1.pToken[dIndex]);
		$1.dLength -= dIndex;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipPathHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
#endif
	}
		| routesemtoken EQUALS allstring
	{
#ifdef SIP_3GPP
		SipParam *pParam;
		SIP_S8bit *pTempval;
		SIP_U32bit dIndex;
		 
		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		dIndex=1;
		while(($1.pToken[dIndex]==' ')||($1.pToken[dIndex]=='\t'))
			dIndex++;
		$1.pToken = &($1.pToken[dIndex]);
		$1.dLength -= dIndex;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
				E_NO_MEM;
			YYABORT;
		}
		pTempval = sip_tokenBufferDup($3);
		if(pTempval == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), pTempval, \
			(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) ==SipFail)
			YYABORT;
		if(sip_listAppend(&(glbSipPathHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
#endif
	};

p_preferred_id : PPREFERREDID pPreferredId_comma_list ;
pPreferredId_comma_list : pPreferredId_list 
					 | pPreferredId_list SEPARATOR;

pPreferredId_list : pPreferredId_value
    {
#ifdef SIP_PRIVACY
		/* One value of type PPreferredId is added */
		SipHeaderOrderInfo *pOrder = SIP_NULL ;
		SipBool dResult = SipSuccess ;

		dResult=sip_initSipHeaderOrderInfo(&pOrder,\
	    ((SipHeaderParserParam *)pHeaderParserParam)->pError);

		if (dResult!=SipSuccess)
					YYABORT;

		pOrder->dType     = SipHdrTypePPreferredId;
		pOrder->dTextType = glbSipParserPPreferredIdType;
		pOrder->dNum      = 1;

		if(sip_listAppend(&((SipHeaderParserParam *) 
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slPPreferredIdHdr,\
			glbSipParserPPreferredIdHeader,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
							YYABORT;
		glbSipParserPPreferredIdHeader = SIP_NULL;
					
		if(sip_listAppend(&((SipHeaderParserParam *)pHeaderParserParam)->\
			 pSipMessage->slOrderInfo, (SIP_Pvoid)pOrder, \
			 &*(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
			 !=SipSuccess)
						  YYABORT;
#endif
    }
		| pPreferredId_list SEPARATOR pPreferredId_value 
    {
#ifdef SIP_PRIVACY                     
        /* SipList is being made of type SipHeaderPPreferredId structure
           and thereby working on slPPreferredIdHdr of General Header */
		SipHeaderOrderInfo * pOrder = SIP_NULL ;
        SIP_U32bit orderSize = 0 ;
                    
		if(sip_listAppend( &((SipHeaderParserParam *)
			pHeaderParserParam)->pSipMessage->pGeneralHdr->
			slPPreferredIdHdr,glbSipParserPPreferredIdHeader,
			&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) !=SipSuccess)
					YYABORT;
		glbSipParserPPreferredIdHeader = SIP_NULL;
				
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,&orderSize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
					YYABORT;

		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,orderSize-1, \
			(SIP_Pvoid *)&pOrder,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
					YYABORT;
					pOrder->dNum++;
#endif
		}
		; 

pPreferredId_value  :addrspecnoparam
	{
#ifdef SIP_PRIVACY
		if ( sip_initSipPPreferredIdHeader( &glbSipParserPPreferredIdHeader, 
       		& * ( ( (SipHeaderParserParam*)pHeaderParserParam)->pError) ) 
	    	 != SipSuccess)
		               YYABORT ;
        /* Adding addspecnoparam to global structure of PPreferredId */
		glbSipParserPPreferredIdHeader->pAddrSpec = glbSipParserAddrSpec ;
		glbSipParserAddrSpec = SIP_NULL ;
#endif
	}
		| dispname
		{
			glbSipPPreferredIdNameAddrStarted=1;
		}
		addrspec RANGLE
	{
#ifdef SIP_PRIVACY
		if ( sip_initSipPPreferredIdHeader( 
			&glbSipParserPPreferredIdHeader, & * ( ( 
			(SipHeaderParserParam*)pHeaderParserParam)->pError) ) 
      		 != SipSuccess)
		                      YYABORT ;
         glbSipParserPPreferredIdHeader->pDispName = sip_tokenBufferDup($1) ;
			
        /* Adding addspecnoparam to global structure of PPreferredId */
		if ( glbSipParserPPreferredIdHeader->pDispName == SIP_NULL )
        {
            *(((SipHeaderParserParam *)pHeaderParserParam)->pError)=E_NO_MEM;
                             YYABORT ;
         }
         glbSipParserPPreferredIdHeader->pAddrSpec = glbSipParserAddrSpec ;
	     glbSipParserAddrSpec = SIP_NULL ;
		 glbSipPPreferredIdNameAddrStarted = 0;
#endif
	};

PanInfo: paninfocolon 
    {
#ifdef SIP_3GPP
		if(sip_initSipPanInfoHeader( &glbSipParserPanInfo,\
			&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
#endif
	}		
        access_net_type
		{
#ifdef SIP_3GPP
		SipHeaderOrderInfo *order;
		SipBool dResult;

	    dResult=sip_initSipHeaderOrderInfo(&order,\
		    ((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if(dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType = SipHdrTypePanInfo;
		order->dTextType = SipFormFull;
		order->dNum=1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
		pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
        &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
		pError))!=SipSuccess)
		YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
	    pGeneralHdr->pPanInfoHdr = glbSipParserPanInfo;
		glbSipParserPanInfo = SIP_NULL;
#endif			
      };

paninfocolon: PANINFO
        {
#ifdef SIP_3GPP
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pPanInfoHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor,"There can only be one P-A-N-Info Hdr");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=\
			E_PARSER_ERROR;
			YYABORT;
		}	
#endif
		};

access_net_type: access_type access_info
               | access_type
			   ;
access_type: token12
         {
#ifdef SIP_3GPP
			 glbSipParserPanInfo->pAccessType=sip_tokenBufferDup($1);
			 if(glbSipParserPanInfo->pAccessType==SIP_NULL)
			 {
				 *(((SipHeaderParserParam *) pHeaderParserParam)->pError)=\
				     E_NO_MEM;
				YYABORT;
			}	
#endif
		 };

access_info: access_info access_info1
           | access_info1
		   ;

access_info1: SEMTOKEN EQUALS genvalue
        {
#ifdef SIP_3GPP
			SIP_S32bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
			while(($1.pToken[index] == ' ') || ($1.pToken[index] == '\t'))
				index++;
			$1.pToken = &($1.pToken[index]);
			$1.dLength -= index;
			pParam->pName = sip_tokenBufferDup($1);
			pParam->pValue = sip_tokenBufferDup($3);
			if (sip_listAppend(&(glbSipParserPanInfo->slParams),pParam,\
				       PARSER_ERROR_VAR) == SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
#endif			
	}			 
            | SEMPARAMTOKEN
	{
#ifdef SIP_3GPP
		    SIP_S32bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
            while(($1.pToken[index] == ' ') || ($1.pToken[index] == '\t'))
				index++;
			$1.pToken = &($1.pToken[index]);
			$1.dLength -= index;
            pParam->pName = sip_tokenBufferDup($1);
			if (sip_listAppend(&(glbSipParserPanInfo->slParams),pParam,\
				      PARSER_ERROR_VAR) == SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
#endif
	}
			| SEMTOKEN
	{
#ifdef SIP_3GPP
		    SIP_S32bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			$1.pToken++;
			$1.dLength--;
            while(($1.pToken[index] == ' ') || ($1.pToken[index] == '\t'))
				index++;
			$1.pToken = &($1.pToken[index]);
			$1.dLength -= index;
            pParam->pName = sip_tokenBufferDup($1);
			if (sip_listAppend(&(glbSipParserPanInfo->slParams),pParam,\
				      PARSER_ERROR_VAR) == SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
#endif
	};
		

PcVector: pcvectorcolon 
    { 
#ifdef SIP_3GPP		
		if(sip_initSipPcVectorHeader( &glbSipParserPcVector,\
		&*(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError))!=SipSuccess)
		YYABORT;
#endif			
	}
	chargeexpr
	{
#ifdef SIP_3GPP		
		SipHeaderOrderInfo *order;
		SipBool dResult;

	   dResult=sip_initSipHeaderOrderInfo(&order,\
	       ((SipHeaderParserParam *)pHeaderParserParam)->pError);
	   if(dResult!=SipSuccess)
	   {
			YYABORT;
	   }

	    order->dType = SipHdrTypePcVector;
	    order->dTextType = SipFormFull;
		order->dNum=1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
		pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)order,\
	   	&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
				pError))!=SipSuccess)
				YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage-> \
		pGeneralHdr->pPcVectorHdr = glbSipParserPcVector;
		glbSipParserPcVector = SIP_NULL;	
      
#endif				
	};

	
pcvectorcolon: PCVECTOR 
       {
#ifdef SIP_3GPP		   
        if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pPcVectorHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one PcVector Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
			E_PARSER_ERROR;
			YYABORT;
		}
		
#endif		
	};

chargeexpr: chargeexpr chargeexpr1 
		   | chargeexpr1
	       ;
chargeexpr1: token1 EQUALS gen_value
        {
#ifdef SIP_3GPP			
		SIP_S32bit index=0;
		/* static int i=0; */
		SipNameValuePair *pParam;
		
		if(glbSipParserPcVector->pAccessValue==SIP_NULL)
		{	
		while(($1.pToken[index] == ' ') || ($1.pToken[index] == '\t'))
				index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;

       glbSipParserPcVector->pAccessType=sip_tokenBufferDup($1);
   	   if(glbSipParserPcVector->pAccessType==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= \
			    E_NO_MEM;
				YYABORT;
		}	
       glbSipParserPcVector->pAccessValue=sip_tokenBufferDup($3);
	   if(glbSipParserPcVector->pAccessValue==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= \
			    E_NO_MEM;
    			YYABORT;
		}
   
	
        }
		 else
		{	 
		index=1;
		while(($1.pToken[index] == ' ') || ($1.pToken[index] == '\t'))
				index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;
	
		sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
		pParam->pName = sip_tokenBufferDup($1);
		pParam->pValue = sip_tokenBufferDup($3);

		if (sip_listAppend(&(glbSipParserPcVector->slParams),pParam,\
			      PARSER_ERROR_VAR) == SipFail)
		{
			*PARSER_ERROR_VAR = E_PARSER_ERROR;
			YYABORT;
		}
		}
#endif
}
         | gen_value1
	{
#ifdef SIP_3GPP
		    SIP_S32bit index=0;
			SipNameValuePair *pParam;
			sip_initSipNameValuePair(&pParam,PARSER_ERROR_VAR);
			index=1;
            while(($1.pToken[index] == ' ') || ($1.pToken[index] == '\t'))
				index++;
			$1.pToken = &($1.pToken[index]);
			$1.dLength -= index;
            pParam->pName = sip_tokenBufferDup($1);
			if (sip_listAppend(&(glbSipParserPcVector->slParams),pParam,\
				      PARSER_ERROR_VAR) == SipFail)
			{
				*PARSER_ERROR_VAR = E_PARSER_ERROR;
				YYABORT;
			}
#endif
	};
genvalue: token12
        | QSTRING1
		;
gen_value1:SEMTOKEN
          |SEMPARAMTOKEN
		  ;
gen_value: token12
         | QSTRING1
		 |IPV6ADDR
		 ;
token1: TOKEN
     | SEMTOKEN
	 ;
token12: TOKEN
       | digit12
	   ;
digit12: DIGITS
       ;
