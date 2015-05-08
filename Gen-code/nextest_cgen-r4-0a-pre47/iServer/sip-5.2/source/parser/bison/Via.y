/***********************************************************************
 ** FUNCTION:
 **             Yacc file for Via headers

 *********************************************************************
 **
 ** FILENAME:
 ** Via.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form Via
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 24/11/99  KSBinu, Arjun RC                           Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


%token VIA 
%token COMMA SLASH COLON EQUALS
%token DIGITS TOKEN SEMTOKEN COMMENT QSTRING SIPFLOAT IPV6ADDR
%token DEFAULT
%token SECURITYCLIENT SECURITYSERVER SECURITYVERIFY
%pure_parser
%{ 
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include "sipstruct.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"

#define MAXTEMP (300)

static SipViaHeader *			glbSipParserViaHeader=SIP_NULL;
static en_HeaderForm			glbSipParserViaType;

#ifdef SIP_SECURITY
static SipSecurityClientHeader *   glbSipParserSecurityClientHeader=SIP_NULL;
static SipSecurityServerHeader *   glbSipParserSecurityServerHeader=SIP_NULL;
static SipSecurityVerifyHeader *   glbSipParserSecurityVerifyHeader=SIP_NULL;
static en_HeaderType		   glbHdrType = SipHdrTypeAny;
#endif

#define YYPARSE_PARAM pHeaderParserParam
%}
%%
header:via
	|securityclient
	|error
	{
	    if(glbSipParserViaHeader!= SIP_NULL)
		{
			sip_freeSipViaHeader(glbSipParserViaHeader);	
			glbSipParserViaHeader= SIP_NULL;
		}
#ifdef SIP_SECURITY
		if(glbSipParserSecurityClientHeader!= SIP_NULL)
		{
			sip_freeSipSecurityClientHeader(glbSipParserSecurityClientHeader);
			glbSipParserSecurityClientHeader= SIP_NULL;
		}
		if(glbSipParserSecurityVerifyHeader!= SIP_NULL)
                {
                        sip_freeSipSecurityVerifyHeader(glbSipParserSecurityVerifyHeader);
                        glbSipParserSecurityVerifyHeader= SIP_NULL;
                }
		if(glbSipParserSecurityServerHeader!= SIP_NULL)
		{
			sip_freeSipSecurityServerHeader(glbSipParserSecurityServerHeader);
			glbSipParserSecurityServerHeader= SIP_NULL;
		}
#endif
		{
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=\
					E_PARSER_ERROR;
		}

	};
via:VIA 
	{
		if($1.dChar=='f')
			glbSipParserViaType = SipFormFull;
		else 
			glbSipParserViaType = SipFormShort;
	} 
	viafields;	
viafields:viafields COMMA viafield
	{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;

  		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slViaHdr, \
			glbSipParserViaHeader,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		glbSipParserViaHeader = SIP_NULL;
		
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

	}
		|viafield
	{
		SipHeaderOrderInfo *order;
		SipBool dResult;

  		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->slViaHdr, \
			glbSipParserViaHeader,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		glbSipParserViaHeader = SIP_NULL;
		dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeVia;
		order->dTextType = glbSipParserViaType;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
		|viafields COMMA;
viafield:sentprotocol sentby viaparams
		|sentprotocol sentby viaparams COMMENT viaparams
	{
		glbSipParserViaHeader->pComment = sip_tokenBufferDup($4);
		if(glbSipParserViaHeader->pComment==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}

	};
sentprotocol:	token SLASH token SLASH token
	{
		if(sip_initSipViaHeader(&glbSipParserViaHeader,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserViaHeader->pSentProtocol = (SIP_S8bit *)\
			fast_memget(BISON_MEM_ID, $1.dLength+$3.dLength+\
			$5.dLength+3, SIP_NULL);

		if(glbSipParserViaHeader->pSentProtocol==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}

		strncpy(glbSipParserViaHeader->pSentProtocol, $1.pToken, $1.dLength);
		glbSipParserViaHeader->pSentProtocol[$1.dLength] = '/';
		strncpy(&(glbSipParserViaHeader->pSentProtocol[$1.dLength+1]),\
			$3.pToken, $3.dLength);
		glbSipParserViaHeader->pSentProtocol[$1.dLength+$3.dLength+1] = '/';
		strncpy(&(glbSipParserViaHeader->pSentProtocol)\
			[$1.dLength+$3.dLength+2], $5.pToken, $5.dLength);
		glbSipParserViaHeader->pSentProtocol\
			[$1.dLength+$3.dLength+$5.dLength+2] = '\0';

		glbSipParserViaHeader->pSentBy = SIP_NULL;
		glbSipParserViaHeader->pComment = SIP_NULL;
		if(sip_listInit( &glbSipParserViaHeader->slParam, \
			&__sip_freeSipParam,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
sentby:	host
	{
		glbSipParserViaHeader->pSentBy = sip_tokenBufferDup($1);
		if(glbSipParserViaHeader->pSentBy==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	}
		|host COLON digits
	{
		glbSipParserViaHeader->pSentBy = (SIP_S8bit *)\
			fast_memget(BISON_MEM_ID, $1.dLength+$3.dLength+2, SIP_NULL);
		if(glbSipParserViaHeader->pSentBy==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		strncpy(glbSipParserViaHeader->pSentBy, $1.pToken, $1.dLength);
		glbSipParserViaHeader->pSentBy[$1.dLength] = ':';
		strncpy(&(glbSipParserViaHeader->pSentBy[$1.dLength+1]),\
			$3.pToken, $3.dLength);
		glbSipParserViaHeader->pSentBy[$1.dLength+$3.dLength+1] = '\0';
	};
viaparams:	viaparams viaparam
		|;
viaparam:SEMTOKEN EQUALS alltoken
	{
		int index;		
	 	SipParam *vparam; 
		SIP_S8bit *yyvparam;

		if(sip_initSipParam(&vparam,  \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;

		vparam->pName = sip_tokenBufferDup($1);
		if(vparam->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		yyvparam = sip_tokenBufferDup($3);	
		if(sip_listAppend(&vparam->slValue,yyvparam,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;

		if(sip_listAppend(&glbSipParserViaHeader->slParam, \
			(SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	|SEMTOKEN
	{
		int index;		
	 	SipParam *vparam; 

		if(sip_initSipParam(& vparam,  \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;

		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;

		vparam->pName = sip_tokenBufferDup($1);

		if(vparam->pName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		
		if(sip_listAppend(&glbSipParserViaHeader->slParam, \
			(SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};	
digits:		DIGITS;		
alltoken:	token
		|QSTRING
		|IPV6ADDR;
token:		DIGITS		
			|TOKEN 		
			|SIPFLOAT;
host: token
		| IPV6ADDR;


securityclient:   SECURITYCLIENT 
		{
			/* Security-Client Request Header */
#ifdef SIP_SECURITY
			glbHdrType=SipHdrTypeSecurityClient;
        		if(((SipHeaderParserParam *) pHeaderParserParam)-> \
                		pSipMessage->dType!=SipMessageRequest)
        		{
                		*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= E_NO_ERROR;
                 		YYABORT;
        		}
#endif
		}securityclientfields
		| SECURITYVERIFY  
		{
			/* Security-Verify Request Header */
#ifdef SIP_SECURITY
			glbHdrType=SipHdrTypeSecurityVerify;
			if(((SipHeaderParserParam *) pHeaderParserParam)-> \
				pSipMessage->dType!=SipMessageRequest)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= E_NO_ERROR;
				YYABORT;
			}
#endif
		}securityclientfields
		|SECURITYSERVER 
		 {  
			/* Security-Server Response Header */
#ifdef SIP_SECURITY
			glbHdrType=SipHdrTypeSecurityServer;
        		if(((SipHeaderParserParam *) pHeaderParserParam)-> \
                		pSipMessage->dType!=SipMessageResponse)
        		{
                		*(((SipHeaderParserParam *) pHeaderParserParam)->pError)= E_NO_ERROR;
                                YYABORT;
        		}
#endif 
    		}securityclientfields 
		;
securityclientfields: securityclientfields COMMA securityclientfield
		{
#ifdef SIP_SECURITY
			SipHeaderOrderInfo *pOrder;
                	SIP_U32bit dOrdersize;
                
			if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)-> \
                        	pSipMessage->slOrderInfo, \
                        	&dOrdersize,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
                        	pError)) == SipFail)
                        	YYABORT;
                	if(sip_listGetAt(&((SipHeaderParserParam *) pHeaderParserParam)-> \
                        	pSipMessage->slOrderInfo,dOrdersize-1, \
                        	(SIP_Pvoid *)&pOrder,&*(((SipHeaderParserParam *) \
                        	pHeaderParserParam)->pError)) == SipFail)
                        	YYABORT;
                	pOrder->dNum++;
	
			if(glbHdrType==SipHdrTypeSecurityClient)
			{
				if(sip_listAppend(&((SipHeaderParserParam *) \
					pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
					slSecurityClientHdr,(SIP_Pvoid)glbSipParserSecurityClientHeader, \
					(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
					!=SipSuccess)
					YYABORT;
				glbSipParserSecurityClientHeader=SIP_NULL;	
			}
			if(glbHdrType==SipHdrTypeSecurityVerify)
			{
				if(sip_listAppend(&((SipHeaderParserParam *) \
					pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
					slSecurityVerifyHdr,(SIP_Pvoid)glbSipParserSecurityVerifyHeader, \
					(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
					!=SipSuccess)
					YYABORT;
				glbSipParserSecurityVerifyHeader=SIP_NULL;	
			}
			if(glbHdrType==SipHdrTypeSecurityServer)
			{
				if(sip_listAppend(&((SipHeaderParserParam *) \
					pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
					slSecurityServerHdr,(SIP_Pvoid)glbSipParserSecurityServerHeader, \
					(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
					!=SipSuccess)
					YYABORT;
				glbSipParserSecurityServerHeader=SIP_NULL;
			}
#endif
		}                    
		| securityclientfield
		{
#ifdef SIP_SECURITY
			SipHeaderOrderInfo *pOrder;
                	SipBool dResult;
                	dResult=sip_initSipHeaderOrderInfo(&pOrder,\
                        	((SipHeaderParserParam *)pHeaderParserParam)->pError);
                	if (dResult!=SipSuccess)
                        	YYABORT;

			if(glbHdrType==SipHdrTypeSecurityClient)
				pOrder->dType = SipHdrTypeSecurityClient;
			if(glbHdrType==SipHdrTypeSecurityVerify)
				pOrder->dType = SipHdrTypeSecurityVerify;
			if(glbHdrType==SipHdrTypeSecurityServer)
				pOrder->dType = SipHdrTypeSecurityServer;
                
			pOrder->dTextType = SipFormFull;

                	pOrder->dNum = 1;
		
			if(sip_listAppend(&((SipHeaderParserParam *) \
                        	pHeaderParserParam)->pSipMessage->slOrderInfo, \
                        	(SIP_Pvoid)pOrder, &*(((SipHeaderParserParam *) \
                        	pHeaderParserParam)->pError))!=SipSuccess)
   				YYABORT;
                
			if(glbHdrType==SipHdrTypeSecurityClient)
			{
     				if(sip_listAppend(&((SipHeaderParserParam *) \
					pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
					slSecurityClientHdr, (SIP_Pvoid)glbSipParserSecurityClientHeader, \
					(((SipHeaderParserParam *) pHeaderParserParam)-> \
					pError))!=SipSuccess)
					YYABORT;
				glbSipParserSecurityClientHeader = SIP_NULL;
			}
			if(glbHdrType==SipHdrTypeSecurityVerify)
			{
     				if(sip_listAppend(&((SipHeaderParserParam *) \
					pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
					slSecurityVerifyHdr, (SIP_Pvoid)glbSipParserSecurityVerifyHeader, \
					(((SipHeaderParserParam *) pHeaderParserParam)-> \
					pError))!=SipSuccess)
					YYABORT;
				glbSipParserSecurityVerifyHeader = SIP_NULL;
			}
			if(glbHdrType==SipHdrTypeSecurityServer)
			{
     				if(sip_listAppend(&((SipHeaderParserParam *) \
					pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
					slSecurityServerHdr, (SIP_Pvoid)glbSipParserSecurityServerHeader, \
					(((SipHeaderParserParam *) pHeaderParserParam)-> \
					pError))!=SipSuccess)
					YYABORT;
				glbSipParserSecurityServerHeader = SIP_NULL;
			}
#endif
		}
		;

securityclientfield: mechname mechparams
	             |mechname	
	            ;	
mechname: token
	{
#ifdef SIP_SECURITY
		if(glbHdrType==SipHdrTypeSecurityClient)
		{
			if(sip_initSipSecurityClientHeader(&glbSipParserSecurityClientHeader,\
				&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;

                	glbSipParserSecurityClientHeader->pMechanismName = sip_tokenBufferDup($1);
                	if(glbSipParserSecurityClientHeader->pMechanismName==SIP_NULL)
                	{
                       		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
				YYABORT;
                	}
                	if(sip_listInit( &glbSipParserSecurityClientHeader->slParams, \
                        	&__sip_freeSipParam,&*(((SipHeaderParserParam *) \
                        	pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		}
		if(glbHdrType==SipHdrTypeSecurityVerify)
		{
			if(sip_initSipSecurityVerifyHeader(&glbSipParserSecurityVerifyHeader,\
				&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;

                	glbSipParserSecurityVerifyHeader->pMechanismName = sip_tokenBufferDup($1);
                	if(glbSipParserSecurityVerifyHeader->pMechanismName==SIP_NULL)
                	{
                       		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
				YYABORT;
                	}
                	if(sip_listInit( &glbSipParserSecurityVerifyHeader->slParams, \
                        	&__sip_freeSipParam,&*(((SipHeaderParserParam *) \
                        	pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		}
		if(glbHdrType==SipHdrTypeSecurityServer)
		{
			if(sip_initSipSecurityServerHeader(&glbSipParserSecurityServerHeader,\
				&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;

                	glbSipParserSecurityServerHeader->pMechanismName = sip_tokenBufferDup($1);
                	if(glbSipParserSecurityServerHeader->pMechanismName==SIP_NULL)
                	{
                       		*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
				YYABORT;
                	}
                	if(sip_listInit( &glbSipParserSecurityServerHeader->slParams, \
                        	&__sip_freeSipParam,&*(((SipHeaderParserParam *) \
                        	pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
		}
#endif
	}
	;
mechparams:  mechparam 
	     |mechparams mechparam
	     ;

mechparam: SEMTOKEN EQUALS alltoken
	{
#ifdef SIP_SECURITY
		int index;
                SipParam *vparam;
                SIP_S8bit *yyvparam;
		
                if(sip_initSipParam(&vparam,  \
                        &*(((SipHeaderParserParam *) pHeaderParserParam)->\
                        pError))!=SipSuccess)
                        YYABORT;
                index=1;
                while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
                        index++;
                $1.pToken = &($1.pToken[index]);
                $1.dLength -= index;

                vparam->pName = sip_tokenBufferDup($1);
                if(vparam->pName==SIP_NULL)
                {
                        *(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
                        YYABORT;
                }
                yyvparam = sip_tokenBufferDup($3);
		if(sip_listAppend(&vparam->slValue,yyvparam,\
                        (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
                        !=SipSuccess)
                        YYABORT;

		if(glbHdrType==SipHdrTypeSecurityClient)
		{
	 		if(sip_listAppend(&glbSipParserSecurityClientHeader->slParams, \
                        	(SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
                        	pHeaderParserParam)->pError))!=SipSuccess)
                        	YYABORT;
		}
		if(glbHdrType==SipHdrTypeSecurityVerify)
		{
			if(sip_listAppend(&glbSipParserSecurityVerifyHeader->slParams, \
                        	(SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
                        	pHeaderParserParam)->pError))!=SipSuccess)
                        	YYABORT;
		}
		if(glbHdrType==SipHdrTypeSecurityServer)
		{
			if(sip_listAppend(&glbSipParserSecurityServerHeader->slParams, \
                        	(SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
                        	pHeaderParserParam)->pError))!=SipSuccess)
                        	YYABORT;
		}
#endif
	}
	|SEMTOKEN
        {
#ifdef SIP_SECURITY
                int index;
                SipParam *vparam;

                if(sip_initSipParam(& vparam,  \
                        &*(((SipHeaderParserParam *) pHeaderParserParam)->\
                        pError))!=SipSuccess)
                        YYABORT;

                index=1;
                while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
                        index++;
                $1.pToken = &($1.pToken[index]);
                $1.dLength -= index;

                vparam->pName = sip_tokenBufferDup($1);

                if(vparam->pName==SIP_NULL)
                {
                        *(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
                        YYABORT;
                }

                if(glbHdrType==SipHdrTypeSecurityClient)
                {
                        if(sip_listAppend(&glbSipParserSecurityClientHeader->slParams, \
                                (SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
                                pHeaderParserParam)->pError))!=SipSuccess)
                                YYABORT;
                }
                if(glbHdrType==SipHdrTypeSecurityVerify)
                {
                        if(sip_listAppend(&glbSipParserSecurityVerifyHeader->slParams, \
                                (SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
                                pHeaderParserParam)->pError))!=SipSuccess)
                                YYABORT;
                }
                if(glbHdrType==SipHdrTypeSecurityServer)
                {
                        if(sip_listAppend(&glbSipParserSecurityServerHeader->slParams, \
                                (SIP_Pvoid)vparam,&*(((SipHeaderParserParam *)\
                                pHeaderParserParam)->pError))!=SipSuccess)
                                YYABORT;
                }
#endif
        }
	;
