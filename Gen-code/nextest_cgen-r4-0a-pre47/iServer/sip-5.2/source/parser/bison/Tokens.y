/***********************************************************************
 ** FUNCTION:
 **             Yacc file for Token headers

 *********************************************************************
 **
 ** FILENAME:
 ** Tokens.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form Token
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token TOKEN COMMA SEMICOLON HANDLING 
%token COLON DIGITS ZEROORONE
%token HIDE PRIORITY
%token COLON SEMICOLON DIGITS QVALUE EQUALS QSTRING
%token ACCEPTENCODING CALLID CSEQ CONTENTENCODING CONTENTLENGTH
%token MAXFORWARDS PROXYREQUIRE REQUIRE ALLOW ALLOWEVENTS UNSUPPORTED REQUESTDISPOSITION
%token URICNOCOLON SUPPORTED DEFAULT IPV6ADDR 
%token PRIVACY
%pure_parser
%{ 
#include "sipparserinc.h"
#include <stdlib.h>
#include "sipdecodeintrnl.h"
#include "sipstruct.h"
#ifdef SIP_CCP
#include "ccpstruct.h"
#include "ccpinit.h"
#endif

#ifdef SIP_IMPP
#include "imppstruct.h"
#include "imppinit.h"
#endif

#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "siplist.h"
#include "rprinit.h"
#include "siptrace.h"

#define MAXTEMP (300)
#define MAX_HOPS (255)

static SipAcceptEncodingHeader*	glbSipParserAccenc=SIP_NULL;
static en_HeaderForm glbSipContentEncType;
static en_HeaderForm glbSipSupportedType;
static en_HeaderForm glbSipReqDispType;
static en_HeaderForm glbSipAllowEventsType;


/*
static SipContentEncodingHeader*	glbSipParserConenc=SIP_NULL;
static SipProxyRequireHeader *		glbSipParserProreq=SIP_NULL;
static SipRequireHeader*			glbSipParserReqhdr=SIP_NULL;
static SipAllowHeader *			glbSipParserAllhdr=SIP_NULL;

#ifdef SIP_CCP
SipRequestDispositionHeader* glbSipParserReqDisphdr=SIP_NULL;
#endif*/
#define YYPARSE_PARAM pHeaderParserParam
%}
%%
pHeader:acceptenc
		|cseq
		|contentenc
		|contentlength
		|maxforwards
		|proxyreq
		|require
		|allow
		|allowevents
		|reqdisp
		|unsupported
		|supported
		|hide
		|priority
		|privacy
		|error
		{
			if(glbSipParserAccenc!= SIP_NULL)
			{
				sip_freeSipAcceptEncodingHeader(glbSipParserAccenc);	
				glbSipParserAccenc= SIP_NULL;
			}

			{
				if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
					==E_NO_ERROR)
					*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=\
					E_PARSER_ERROR;
			}
		};

acceptenc:	ACCEPTENCODING contentcodings;
contentcodings:	contentcodings COMMA contentcoding
		{
			SipHeaderOrderInfo *order;
			SIP_U32bit ordersize;
			if(sip_listSizeOf(&((SipHeaderParserParam *) \
				pHeaderParserParam)->pSipMessage->slOrderInfo, \
				&ordersize,&*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError)) == SipFail)
				YYABORT;
			if(sip_listGetAt(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
				(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
			order->dNum++;
			if(sip_listAppend( \
				&(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->pGeneralHdr->slAcceptEncoding), \
				glbSipParserAccenc,&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
			glbSipParserAccenc = SIP_NULL;
		}
		|contentcoding
		{
			SipHeaderOrderInfo *order;
			SipBool dResult;
			dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
			if (dResult!=SipSuccess)
			{
				YYABORT;
			}

			order->dType = SipHdrTypeAcceptEncoding;
			order->dTextType = SipFormFull;
			order->dNum = 1;
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->slOrderInfo, \
				(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
			if(sip_listAppend( \
				&(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->pGeneralHdr->slAcceptEncoding), \
				glbSipParserAccenc,&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
			glbSipParserAccenc = SIP_NULL;
		}
		|contentcodings COMMA
		| 
		{
			SipHeaderOrderInfo *order;
			SipBool dResult;
			dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
			if (dResult!=SipSuccess)
			{
				YYABORT;
			}
			order->dType = SipHdrTypeAcceptEncoding;
			order->dTextType = SipFormFull;
			order->dNum = 1;
			if(sip_listAppend(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->slOrderInfo,\
				(SIP_Pvoid)order,  (((SipHeaderParserParam *) \
				pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
			if(sip_initSipAcceptEncodingHeader(&glbSipParserAccenc,\
				 (((SipHeaderParserParam *) pHeaderParserParam)->\
				 pError))!=SipSuccess)
				YYABORT;
			if(sip_listAppend(&(((SipHeaderParserParam *) \
				pHeaderParserParam)->pSipMessage->pGeneralHdr->slAcceptEncoding),\
				(SIP_Pvoid)glbSipParserAccenc, \
				 (((SipHeaderParserParam *) pHeaderParserParam)->\
				 pError))!=SipSuccess) 
				YYABORT;
			glbSipParserAccenc=SIP_NULL;
		};
contentcoding:token
	{
		if(sip_initSipAcceptEncodingHeader(&glbSipParserAccenc, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		glbSipParserAccenc->pCoding = sip_tokenBufferDup($1);
		if(glbSipParserAccenc->pCoding==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}
		| token SEMICOLON acceptencnamevaluepairs
	{
		glbSipParserAccenc->pCoding = sip_tokenBufferDup($1);
		if(glbSipParserAccenc->pCoding==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}
 	};
	
acceptencnamevaluepairs: acceptencnamevaluepair
	|	acceptencnamevaluepairs SEMICOLON acceptencnamevaluepair;

acceptencnamevaluepair:	token
	{
		SipNameValuePair *pNameValuePair=SIP_NULL;
		SIP_S8bit *pName=SIP_NULL;
		if (glbSipParserAccenc==SIP_NULL)	
		{	
			if(sip_initSipAcceptEncodingHeader(&glbSipParserAccenc, \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;
		}	

		/*First find out if the param is of the type qvalue or not*/
		pName=sip_tokenBufferDup($1);
		if(pName==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}
		
		if(sip_initSipNameValuePair(&pNameValuePair,\
			(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
			== SipFail)
		YYABORT;

		pNameValuePair->pName=pName;
			
		/*Now add this NameValuePair to the list of NameValuepairs*/
		if(sip_listAppend(&(glbSipParserAccenc->slParam), pNameValuePair,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
		YYABORT;

	}
	| token EQUALS allstring
	{
		SipNameValuePair *pNameValuePair=SIP_NULL;
		SIP_S8bit *pName=SIP_NULL;
		SIP_S8bit *pValue=SIP_NULL;
		SipBool dQVal=SipFail;
		if (glbSipParserAccenc==SIP_NULL)	
		{	
			if(sip_initSipAcceptEncodingHeader(&glbSipParserAccenc, \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;
		}	

		/*First find out if the param is of the type qvalue or not*/
		pName=sip_tokenBufferDup($1);
		if(pName==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}

		pValue=sip_tokenBufferDup($3);
		if (pValue==SIP_NULL)
		{
		        *(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_MEM;
		        YYABORT;
		}

		if ((!strcmp(pName,"q"))||(!strcmp(pName,"Q")))
		{
			/*It is a "q=xyz" type and needs to be treated as such
				we need to however check if we have already recvd
				a qvalue*/
			if ((glbSipParserAccenc->pQValue)==SIP_NULL)
			{	
				dQVal=SipSuccess;
				fast_memfree(BISON_MEM_ID,pName,SIP_NULL);
			}	
		}

		if (dQVal==SipSuccess)
		{
			glbSipParserAccenc->pQValue = pValue;
		}
		else
		{
			/*Add this in the list of SipNameValuePairs*/
			if(sip_initSipNameValuePair(&pNameValuePair,\
				(((SipHeaderParserParam *)pHeaderParserParam)->pError))\
				== SipFail)
			YYABORT;

			pNameValuePair->pName=pName;
			pNameValuePair->pValue=pValue;
			
			/*Now add this NameValuePair to the list of NameValuepairs*/
			if(sip_listAppend(&(glbSipParserAccenc->slParam), pNameValuePair,\
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError)) == SipFail)
			YYABORT;
		}	
	};

allstring: TOKEN
	|QVALUE
	|QSTRING
	|digits
	|IPV6ADDR;

digits: DIGITS
	|	ZEROORONE;
	
cseq:cseqcolon digits TOKEN
	{
		SIP_S8bit *temp;
		SipHeaderOrderInfo *order;
		SipGeneralHeader * genhdr;
		SipBool dResult;

		genhdr = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(sip_initSipCseqHeader(&genhdr->pCseqHdr,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		
		/* cap off if Value greater that 2^32-1 */
		temp = sip_tokenBufferDup($2);
		genhdr->pCseqHdr->dSeqNum = (SIP_U32bit)STRTOU32CAP(temp, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		/* The value of the Cseq num should be as 0 <= cseq num < 2^32 */
		if((*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			== E_INV_RANGE))
		{
		    sip_error (SIP_Minor,"Cseq Number out of Range");
		    *(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
				E_PARSER_ERROR;
			YYABORT;
		}
		genhdr->pCseqHdr->pMethod = sip_tokenBufferDup($3);
		if(genhdr->pCseqHdr->pMethod==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}

		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeCseq;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)order\
			,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
	};
contentenc:	CONTENTENCODING 
	{
		if($1.dChar=='f')
			glbSipContentEncType = SipFormFull;
		else 
			glbSipContentEncType = SipFormShort;
	}
	cencfields;
cencfields:	cencfields COMMA cencfield
	{
		SipHeaderOrderInfo *order;
		SipContentEncodingHeader*	glbSipParserConenc;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
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
		if(sip_initSipContentEncodingHeader(&glbSipParserConenc, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserConenc->pEncoding = sip_tokenBufferDup($3);
		if(glbSipParserConenc->pEncoding==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( \
			&(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->slContentEncodingHdr), \
			glbSipParserConenc,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|cencfield
	{
		SipContentEncodingHeader*	glbSipParserConenc;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeContentEncoding;
		order->dTextType = glbSipContentEncType;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		if(sip_initSipContentEncodingHeader(&glbSipParserConenc, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserConenc->pEncoding = sip_tokenBufferDup($1);
		if(glbSipParserConenc->pEncoding==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( \
			&(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->slContentEncodingHdr), \
			glbSipParserConenc,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|cencfields COMMA;
cencfield:token;

contentlength:	contentlengthcolon digits
	{
		SipHeaderOrderInfo *order;
		SIP_S8bit *temp;
		SIP_U32bit tempDig;
		SipBool dResult;
		if(sip_initSipContentLengthHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pContentLengthHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pContentLengthHdr==SIP_NULL)
			YYABORT;
		temp = sip_tokenBufferDup($2);
		tempDig = STRTOU32CAP(temp,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
							pError));
		if ( tempDig > 65535 )
		{

			((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->pGeneralHdr->pContentLengthHdr->dLength = 65535;
			fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}
		((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr->pContentLengthHdr->dLength = tempDig;
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);

		
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeContentLength;
		if($1.dChar=='f')
			order->dTextType = SipFormFull;
		else
			order->dTextType = SipFormShort;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
maxforwards:maxforwardscolon digits
	{
		SipHeaderOrderInfo *order;
		SIP_S8bit *temp;
		SipBool dResult;
        SIP_U32bit tempHops=0; 

		if(sip_initSipMaxForwardsHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pMaxForwardsHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		temp = sip_tokenBufferDup($2);
		tempHops = STRTOU32CAP(temp,\
	   			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError));
		if ( tempHops > MAX_HOPS)
			tempHops = MAX_HOPS ;

		((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->u.pRequest->pRequestHdr->pMaxForwardsHdr->\
		dHops =   tempHops ;

		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeMaxforwards;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
proxyreq:proxyrequirecolon preqfields;
preqfields:	preqfields COMMA preqfield
	{
		SipProxyRequireHeader *		glbSipParserProreq=SIP_NULL;
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
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
		if(sip_initSipProxyRequireHeader(&glbSipParserProreq, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		glbSipParserProreq->pToken = sip_tokenBufferDup($3);
		if(glbSipParserProreq->pToken==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
			slProxyRequireHdr),glbSipParserProreq,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
		|preqfield
	{
		SipProxyRequireHeader *		glbSipParserProreq=SIP_NULL;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeProxyRequire;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipProxyRequireHeader(&glbSipParserProreq, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		glbSipParserProreq->pToken = sip_tokenBufferDup($1);
		if(glbSipParserProreq->pToken==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
			slProxyRequireHdr),glbSipParserProreq,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|preqfields COMMA;
preqfield:token;

require:requirecolon reqfields;
reqfields:reqfields COMMA reqfield
	{
		SipHeaderOrderInfo *order;
		SipRequireHeader*			glbSipParserReqhdr=SIP_NULL;
		SIP_U32bit ordersize;

		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,&ordersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1,\
			(SIP_Pvoid *)&order,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_initSipRequireHeader(&glbSipParserReqhdr,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserReqhdr->pToken = sip_tokenBufferDup($3);
		if(glbSipParserReqhdr->pToken==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( \
			&(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->slRequireHdr), \
			glbSipParserReqhdr,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
		|reqfield
	{
		SipHeaderOrderInfo *order;
		SipRequireHeader*			glbSipParserReqhdr=SIP_NULL;
		SipBool dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeRequire;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipRequireHeader(&glbSipParserReqhdr,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserReqhdr->pToken = sip_tokenBufferDup($1);
		if(glbSipParserReqhdr->pToken==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( \
			&(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->slRequireHdr), \
			glbSipParserReqhdr,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|reqfields COMMA;
reqfield:token;

allow:		allowcolon allowfields;
allowfields:	allowfields COMMA allowfield
	{
		SipAllowHeader *			glbSipParserAllhdr=SIP_NULL;
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_initSipAllowHeader(&glbSipParserAllhdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		glbSipParserAllhdr->pMethod = sip_tokenBufferDup($3);
		if(glbSipParserAllhdr->pMethod==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAllowHdr), \
			glbSipParserAllhdr,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|allowfield
	{
		SipAllowHeader *			glbSipParserAllhdr=SIP_NULL;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAllow;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipAllowHeader(&glbSipParserAllhdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		glbSipParserAllhdr->pMethod = sip_tokenBufferDup($1);
		if(glbSipParserAllhdr->pMethod==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAllowHdr), \
			glbSipParserAllhdr,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|allowfields COMMA
	|
	{
		SipAllowHeader *			glbSipParserAllhdr=SIP_NULL;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAllow;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order,  (((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipAllowHeader(&glbSipParserAllhdr,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAllowHdr),\
			(SIP_Pvoid)glbSipParserAllhdr, \
			 (((SipHeaderParserParam *) pHeaderParserParam)->\
			 pError))!=SipSuccess) 
			YYABORT;
		glbSipParserAllhdr=SIP_NULL;
	};
allowfield:	token;

allowevents:		alloweventscolon alloweventsfields;
alloweventsfields:	alloweventsfields COMMA alloweventsfield
	{
#ifdef SIP_IMPP		
		SipAllowEventsHeader *pAllowEventshdr;
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			&ordersize,((SipHeaderParserParam *) \
			pHeaderParserParam)->pError) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,((SipHeaderParserParam *)\
			pHeaderParserParam)->pError) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_impp_initSipAllowEventsHeader(&pAllowEventshdr,\
			((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) !=SipSuccess)
			YYABORT;
		pAllowEventshdr->pEventType = sip_tokenBufferDup($3);
		if(pAllowEventshdr->pEventType==SIP_NULL)
		{
			*(((SipHeaderParserParam *)pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAllowEventsHdr), \
			pAllowEventshdr,((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;
#endif		
	}
	|alloweventsfield
	{
#ifdef SIP_IMPP		
		SipAllowEventsHeader *pAllowEventshdr;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAllowEvents;
		order->dTextType = glbSipAllowEventsType;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, ((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;
		if(sip_impp_initSipAllowEventsHeader(&pAllowEventshdr,\
			((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) !=SipSuccess)
			YYABORT;
		pAllowEventshdr->pEventType = sip_tokenBufferDup($1);
		if(pAllowEventshdr->pEventType==SIP_NULL)
		{
			*(((SipHeaderParserParam *)pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slAllowEventsHdr), \
			pAllowEventshdr,((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)!=SipSuccess)
			YYABORT;

#endif			
	}
	|alloweventsfields COMMA;
alloweventsfield:	token;
reqdisp: reqdispcolon
	{
		if($1.dChar=='f')
			glbSipReqDispType = SipFormFull;
		else 
			glbSipReqDispType = SipFormShort;
	}
	reqdispfields;

reqdispfields: reqdispfields COMMA reqdispfield
	{
#ifdef SIP_CCP
		SipRequestDispositionHeader* glbSipParserReqDisphdr=SIP_NULL;
		SIP_S8bit *reqdisp;
		SipHeaderOrderInfo *order;

		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,&ordersize, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			== SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1,  \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
 		reqdisp = sip_tokenBufferDup($3);
		if(sip_ccp_initSipRequestDispositionHeader(&glbSipParserReqDisphdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;

		if(reqdisp==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		glbSipParserReqDisphdr->pFeature=reqdisp;
		if(sip_listAppend( \
			&(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->slRequestDispositionHdr), \
			glbSipParserReqDisphdr,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
#endif
	}
	|reqdispfield
	{
#ifdef SIP_CCP
		SipRequestDispositionHeader* glbSipParserReqDisphdr=SIP_NULL;
		SIP_S8bit *reqdisp;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType = SipHdrTypeRequestDisposition;
		order->dTextType = glbSipReqDispType;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
 		reqdisp = sip_tokenBufferDup($1);
		if(sip_ccp_initSipRequestDispositionHeader(&glbSipParserReqDisphdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;

		if(reqdisp==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		glbSipParserReqDisphdr->pFeature=reqdisp;
		if(sip_listAppend( \
			&(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->slRequestDispositionHdr), \
			glbSipParserReqDisphdr,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
#endif
	}
	|reqdispfields COMMA;
reqdispfield: TOKEN;
unsupported:unsupcolon unsupfields;
unsupfields:unsupfields COMMA unsupfield
	{
 		SipUnsupportedHeader *pUnsup;
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *)\
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
		if(sip_initSipUnsupportedHeader(&pUnsup,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;
 		pUnsup->pOption = sip_tokenBufferDup($3);
		if(pUnsup->pOption==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->u.pResponse->\
			pResponseHdr->slUnsupportedHdr), pUnsup,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
	}
	|unsupfield
	{
 		SipUnsupportedHeader *pUnsup;
		SipHeaderOrderInfo *order;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeUnsupported;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipUnsupportedHeader(&pUnsup,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			==SipFail)
			YYABORT;
 		pUnsup->pOption = sip_tokenBufferDup($1);
		if(pUnsup->pOption==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->u.pResponse->\
			pResponseHdr->slUnsupportedHdr), pUnsup,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
	}
		|unsupfields COMMA;
unsupfield:	token;

supported: SUPPORTED 
	{
		if($1.dChar=='f')
			glbSipSupportedType = SipFormFull;
		else 
			glbSipSupportedType = SipFormShort;
	} options;
options:options COMMA option
	{
		SipHeaderOrderInfo *order;
		SipGeneralHeader *genhdr;
		SipSupportedHeader *suphdr;
		SIP_U32bit ordersize;

		if(sip_listSizeOf(&((SipHeaderParserParam *)\
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
		if(sip_initSipSupportedHeader(&suphdr,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		suphdr->pOption = sip_tokenBufferDup($3);
		if(suphdr->pOption == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		genhdr = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(sip_listAppend(&(genhdr->slSupportedHdr), \
			(SIP_Pvoid) suphdr,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))==SipFail)
			YYABORT;
	}
	|option 
	{
		SipHeaderOrderInfo *order;
		SipGeneralHeader *genhdr;
		SipSupportedHeader *suphdr;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeSupported;
		order->dTextType = glbSipSupportedType;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->slOrderInfo,\
			(SIP_Pvoid)order,&*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_initSipSupportedHeader(&suphdr,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))==SipFail)
			YYABORT;
		suphdr->pOption = sip_tokenBufferDup($1);
		if(suphdr->pOption == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		genhdr = ((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->pGeneralHdr;
		if(sip_listAppend(&(genhdr->slSupportedHdr), \
			(SIP_Pvoid) suphdr,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))==SipFail)
			YYABORT;
	}
	| 
	{
	 	SipHeaderOrderInfo *order;
	    SipGeneralHeader *genhdr;
	    SipSupportedHeader *suphdr;
		SipBool dResult;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
    	order->dType = SipHdrTypeSupported;
    	order->dTextType = glbSipSupportedType;
    	order->dNum = 1;
    	if(sip_listAppend(&((SipHeaderParserParam *) \
    		    pHeaderParserParam)->pSipMessage->slOrderInfo,\
    		    (SIP_Pvoid)order,&*(((SipHeaderParserParam *) \
    		    pHeaderParserParam)->pError))!=SipSuccess)
    		    YYABORT;
    	if(sip_initSipSupportedHeader(&suphdr,(((SipHeaderParserParam *)\
    		    pHeaderParserParam)->pError))==SipFail)
    		    YYABORT;
		suphdr->pOption = strdup("");
   	     if(suphdr->pOption == SIP_NULL)
   	     {
          	*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
			E_NO_MEM;
   	        YYABORT;
   	     }
   	     genhdr = ((SipHeaderParserParam *) pHeaderParserParam)->\
   	     pSipMessage->pGeneralHdr;
   	     if(sip_listAppend(&(genhdr->slSupportedHdr), \
   	             (SIP_Pvoid) suphdr,&*(((SipHeaderParserParam *)\
   	             pHeaderParserParam)->pError))==SipFail)
   	             YYABORT;
	};
option:token; 

token:		TOKEN	
			|QVALUE
			|digits;	
cseqcolon:	CSEQ
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pCseqHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one CSeq header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};
contentlengthcolon:	CONTENTLENGTH
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pContentLengthHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor,"There can only be one content-length header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};
maxforwardscolon:MAXFORWARDS
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pMaxForwardsHdr\
			!=SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one Max forwards header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};
proxyrequirecolon:	PROXYREQUIRE 
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_ERROR;
			YYABORT;
		}
	};
requirecolon:REQUIRE;
allowcolon:	ALLOW;
alloweventscolon:	ALLOWEVENTS
	{
		if($1.dChar == 'f')
			glbSipAllowEventsType = SipFormFull;
		else
			glbSipAllowEventsType = SipFormShort;
	};
unsupcolon:	UNSUPPORTED
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType!=SipMessageResponse)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
	};
hide:	hidecolon token
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		if(sip_initSipHideHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pHideHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
		u.pRequest->pRequestHdr->pHideHdr->pType =sip_tokenBufferDup($2);
		if (((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pHideHdr->pType == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeHide;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
hidecolon:	HIDE
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType!=SipMessageRequest)
		{
		 	*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pHideHdr!=SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one Hide header");
		 	*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_PARSER_ERROR;
			YYABORT;
		}
	};
priority:	prioritycolon prioritytoken 
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;
		if(sip_initSipPriorityHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pPriorityHdr, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->u.pRequest->pRequestHdr->pPriorityHdr->\
			pPriority = sip_tokenBufferDup($2); 
		if (((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pPriorityHdr->\
			pPriority == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError) = E_NO_MEM;
			YYABORT;
		}
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypePriority;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
prioritycolon:	PRIORITY
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_ERROR;
			YYABORT;
		}		
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->pPriorityHdr \
			!= SIP_NULL)
		{
			sip_error(SIP_Minor,"There can only be one Priority header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
	};
prioritytoken: TOKEN|DIGITS|ZEROORONE|QVALUE;
reqdispcolon: REQUESTDISPOSITION
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
			YYABORT;
		}
	};

privacy: PRIVACY
	{
#ifdef SIP_PRIVACY
		SipGeneralHeader	 *genhdr;
	
		genhdr = ((SipHeaderParserParam *)pHeaderParserParam)->\
					pSipMessage->pGeneralHdr;
		if ( genhdr->pPrivacyHdr != SIP_NULL )
		{
			sip_error(SIP_Minor,"There can only be one Privacy header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;	
		}
		if(sip_initSipPrivacyHeader(&genhdr->pPrivacyHdr,
			(((SipHeaderParserParam *)pHeaderParserParam)->pError)) 
			!= SipSuccess)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
#endif
	}
	privvalues;

privvalues: privvalue SEMICOLON privvalues
			| privvalue
	{
#ifdef SIP_PRIVACY
		SipHeaderOrderInfo *order;
		SipBool 	dResult;
		
		dResult = sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		
		if(dResult != SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypePrivacy;
		order->dTextType = SipFormFull;
		order->dNum = 1;
					
		if(sip_listAppend(&(((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->slOrderInfo),\
				(SIP_Pvoid)order,&*(((SipHeaderParserParam *)\
				 pHeaderParserParam)->pError))!=SipSuccess)
		{
			YYABORT;
		}
#endif
};

privtoken: token | QSTRING;

privvalue: privtoken
	{	
#ifdef SIP_PRIVACY
		SipNameValuePair *pNameValuePair = NULL;
		SIP_U32bit dIndex = 0;

	   	if(sip_initSipNameValuePair(&pNameValuePair, (((SipHeaderParserParam *)\
			 pHeaderParserParam)->pError)) == SipFail)
			 YYABORT;	

		while($1.pToken[dIndex] == ' ' || $1.pToken[dIndex] == '\t')
		{
			dIndex++;
		}
		$1.pToken = &($1.pToken[dIndex]);
		$1.dLength -= dIndex;
		pNameValuePair->pName = sip_tokenBufferDup($1);
					
		if(pNameValuePair->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *)pHeaderParserParam)\
				->pError)= E_NO_MEM;	
			YYABORT;
		}
		if(sip_listAppend(&(((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->pGeneralHdr\
				->pPrivacyHdr->slPrivacyValue),pNameValuePair,\
				&*(((SipHeaderParserParam *)pHeaderParserParam)\
				->pError))!=SipSuccess)
		{
			YYABORT;
		}
#endif
	};
