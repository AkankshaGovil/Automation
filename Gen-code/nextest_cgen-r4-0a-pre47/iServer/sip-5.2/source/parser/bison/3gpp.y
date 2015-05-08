/***********************************************************************
 ** FUNCTION:
 **             Yacc file for 3GPP Headers

 *********************************************************************
 **
 ** FILENAME:
 ** 3gpp.y
 **
 ** DESCRIPTION:
 ** This has all the 3GPP headers defined in RFC 3455
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 12 Dec,03   S.Chatterjee                               Initial Creation
 **
 **     Copyright 2003, Hughes Software Systems, Ltd.
 *********************************************************************/

%token COMMA COLON SEMICOLON RANGLE PASSOCIATEDURI
%token EQUALS PLAINSIPCOLON SIPCOLON SIPSCOLON DIGITS SEMPARAMTOKEN SEMPARAMVALUE
%token QSTRING QSTRING1 TOKEN
%token URICNOCOLON QUESTION PATH
%token DISPNAME USERINFOAT SEMTOKEN IPV6ADDR 
%token DEFAULT TAG REFTOKEN
%token PCALLEDPARTYID PVISITEDNETWORKID PCFADDR
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

static SipParam  *glbSipParserSipParam=SIP_NULL;
static SipAddrSpec *glbSipParserAddrSpec=SIP_NULL;

static SipPAssociatedUriHeader *glbSipParserPAssociatedUriHeader = SIP_NULL;
static SipPCalledPartyIdHeader *glbSipParserPCalledPartyIdHeader = SIP_NULL;
static SipPVisitedNetworkIdHeader *glbSipParserPVisitedNetworkIdHeader = SIP_NULL;
static SipPcfAddrHeader *glbSipParserPcfAddrHeader = SIP_NULL;

extern int glbSipParser3gppAllowSipColon;

#define YYPARSE_PARAM pHeaderParserParam
%}
%%
header:passociateduri
       |pcalledpartyid 
       |pvisitednetworkid
       |pcfaddr 
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
			if(glbSipParserPAssociatedUriHeader != SIP_NULL)
			{
				sip_freeSipPAssociatedUriHeader(glbSipParserPAssociatedUriHeader);
				glbSipParserPAssociatedUriHeader = SIP_NULL;
			}
			if(glbSipParserPCalledPartyIdHeader != SIP_NULL)
			{
				sip_freeSipPCalledPartyIdHeader(glbSipParserPCalledPartyIdHeader);
				glbSipParserPCalledPartyIdHeader = SIP_NULL;
			}
			if(glbSipParserPVisitedNetworkIdHeader != SIP_NULL)
			{
				sip_freeSipPVisitedNetworkIdHeader(glbSipParserPVisitedNetworkIdHeader);
				glbSipParserPVisitedNetworkIdHeader = SIP_NULL;
			}
   			if(glbSipParserPcfAddrHeader != SIP_NULL)
			{
				sip_freeSipPcfAddrHeader(glbSipParserPcfAddrHeader);
				glbSipParserPcfAddrHeader = SIP_NULL;
			}
                        
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					=E_PARSER_ERROR;
		};

passociateduri : PASSOCIATEDURI
        {
			if(((SipHeaderParserParam *) pHeaderParserParam)-> \
				pSipMessage->dType!=SipMessageResponse)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					 = E_NO_ERROR;
				YYABORT;
			}

		} passociateduris;

passociateduris:passociatedurifields
        {
		    SipHeaderOrderInfo *order;
    		SipBool	dResult;
		
	    	dResult=sip_initSipHeaderOrderInfo(&order,\
		    	((SipHeaderParserParam *)pHeaderParserParam)->pError);
    		if (dResult!=SipSuccess)
	    	{
		    	YYABORT;
    		}
	
    		order->dType = SipHdrTypePAssociatedUri;
	    	order->dTextType = SipFormFull;
		    order->dNum = 1;
    		if(sip_listAppend(&((SipHeaderParserParam *) \
	    		pHeaderParserParam)->pSipMessage->slOrderInfo, \
		    	(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			    pHeaderParserParam)->pError))!=SipSuccess)
				    YYABORT;

            if(sip_listAppend(&((SipHeaderParserParam *) \
	    		pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
		    	slPAssociatedUriHdr, (SIP_Pvoid)glbSipParserPAssociatedUriHeader, \
			    (((SipHeaderParserParam *) pHeaderParserParam)-> \
    			pError))!=SipSuccess)
	    			YYABORT;
		    glbSipParserPAssociatedUriHeader = SIP_NULL;
    	}
            |passociateduris COMMA passociatedurifields
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
		    	pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
			    slPAssociatedUriHdr,(SIP_Pvoid)glbSipParserPAssociatedUriHeader, \
    			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
	    		!=SipSuccess)
		    		YYABORT;		
    		glbSipParserPAssociatedUriHeader = SIP_NULL;  
        };

passociatedurifields:dispname addrspec RANGLE
    {
		if(sip_initSipPAssociatedUriHeader(&glbSipParserPAssociatedUriHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		glbSipParserPAssociatedUriHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipParserPAssociatedUriHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
				= E_NO_MEM;
			YYABORT;
		}
		glbSipParserPAssociatedUriHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;
	}
      |dispname addrspec RANGLE
    {
        if(sip_initSipPAssociatedUriHeader(&glbSipParserPAssociatedUriHeader,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		glbSipParserPAssociatedUriHeader->pDispName = sip_tokenBufferDup($1);
		if(glbSipParserPAssociatedUriHeader->pDispName==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
				= E_NO_MEM;
			YYABORT;
		}
		glbSipParserPAssociatedUriHeader->pAddrSpec = glbSipParserAddrSpec;
		glbSipParserAddrSpec = SIP_NULL;

	}passociateduriparams;

passociateduriparams:passociateduriparams passociateduriparam
                    |passociateduriparam;

passociateduriparam: gppsemtoken
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
		if(sip_listAppend(&(glbSipParserPAssociatedUriHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
		| gppsemtoken EQUALS allstring
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

		if(sip_listAppend(&(glbSipParserPAssociatedUriHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	};

pcalledpartyid: PCALLEDPARTYID
        {
			if(((SipHeaderParserParam *) pHeaderParserParam)-> \
				pSipMessage->dType!=SipMessageRequest)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					 = E_NO_ERROR;
				YYABORT;
			}
            if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
		    	u.pRequest->pRequestHdr->pPCalledPartyIdHdr!=SIP_NULL)
    		{
	    		sip_error (SIP_Minor, "There can only be one P-Called-PartyId Header");
		    	*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
			    E_PARSER_ERROR;
    			YYABORT;
	    	}

		} pcalledpartyids;

pcalledpartyids : pcalledpartyidfields
        {
		    SipHeaderOrderInfo *order;
    		SipBool	dResult;
		
	    	dResult=sip_initSipHeaderOrderInfo(&order,\
		    	((SipHeaderParserParam *)pHeaderParserParam)->pError);
    		if (dResult!=SipSuccess)
	    	{
		    	YYABORT;
    		}
	
    		order->dType = SipHdrTypePCalledPartyId;
	    	order->dTextType = SipFormFull;
		    order->dNum = 1;
    		if(sip_listAppend(&((SipHeaderParserParam *) \
	    		pHeaderParserParam)->pSipMessage->slOrderInfo, \
		    	(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			    pHeaderParserParam)->pError))!=SipSuccess)
				    YYABORT;

            ((SipHeaderParserParam *)pHeaderParserParam)->pSipMessage->\
              u.pRequest->pRequestHdr->pPCalledPartyIdHdr =\
                glbSipParserPCalledPartyIdHeader;
  		    glbSipParserPCalledPartyIdHeader = SIP_NULL;
    	};

pcalledpartyidfields: addrspec
        {
		    if(sip_initSipPCalledPartyIdHeader(&glbSipParserPCalledPartyIdHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;
		    glbSipParserPCalledPartyIdHeader->pAddrSpec = glbSipParserAddrSpec;
    		glbSipParserAddrSpec = SIP_NULL;

        }
        | dispname addrspec RANGLE 
        {
		    if(sip_initSipPCalledPartyIdHeader(&glbSipParserPCalledPartyIdHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;
		    glbSipParserPCalledPartyIdHeader->pDispName = sip_tokenBufferDup($1);
    		if(glbSipParserPCalledPartyIdHeader->pDispName==SIP_NULL)
	    	{
		    	*(((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
			    	= E_NO_MEM;
    			YYABORT;
	    	}
		    glbSipParserPCalledPartyIdHeader->pAddrSpec = glbSipParserAddrSpec;
    		glbSipParserAddrSpec = SIP_NULL;
	    }
		| dispname addrspec RANGLE
        {
		    if(sip_initSipPCalledPartyIdHeader(&glbSipParserPCalledPartyIdHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;
		    glbSipParserPCalledPartyIdHeader->pDispName = sip_tokenBufferDup($1);
    		if(glbSipParserPCalledPartyIdHeader->pDispName==SIP_NULL)
	    	{
                *(((SipHeaderParserParam *) pHeaderParserParam)->pError)  \
			    	= E_NO_MEM;
    			YYABORT;
	    	}
		    glbSipParserPCalledPartyIdHeader->pAddrSpec = glbSipParserAddrSpec;
    		glbSipParserAddrSpec = SIP_NULL;

	    }pcalledpartyidparams;

pcalledpartyidparams: pcalledpartyidparam
| pcalledpartyidparams pcalledpartyidparam;

pcalledpartyidparam: gppsemtoken
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
		if(sip_listAppend(&(glbSipParserPCalledPartyIdHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
	| gppsemtoken EQUALS allstring
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

		if(sip_listAppend(&(glbSipParserPCalledPartyIdHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	};

pvisitednetworkid : PVISITEDNETWORKID 
        {
			if(((SipHeaderParserParam *) pHeaderParserParam)-> \
				pSipMessage->dType!=SipMessageRequest)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					 = E_NO_ERROR;
				YYABORT;
			}

		}pvisitednetworkids;

pvisitednetworkids: pvisitednetworkidfields
        {
		    SipHeaderOrderInfo *order;
    		SipBool	dResult;
		
	    	dResult=sip_initSipHeaderOrderInfo(&order,\
		    	((SipHeaderParserParam *)pHeaderParserParam)->pError);
    		if (dResult!=SipSuccess)
	    	{
		    	YYABORT;
    		}
	
    		order->dType = SipHdrTypePVisitedNetworkId;
	    	order->dTextType = SipFormFull;
		    order->dNum = 1;
    		if(sip_listAppend(&((SipHeaderParserParam *) \
	    		pHeaderParserParam)->pSipMessage->slOrderInfo, \
		    	(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			    pHeaderParserParam)->pError))!=SipSuccess)
				    YYABORT;

            if(sip_listAppend(&((SipHeaderParserParam *) \
	    		pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
		    	slPVisitedNetworkIdHdr, (SIP_Pvoid)glbSipParserPVisitedNetworkIdHeader, \
			    (((SipHeaderParserParam *) pHeaderParserParam)-> \
    			pError))!=SipSuccess)
	    			YYABORT;
		    glbSipParserPVisitedNetworkIdHeader = SIP_NULL;
    	}
		| pvisitednetworkids COMMA pvisitednetworkidfields
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
		    	slPVisitedNetworkIdHdr, (SIP_Pvoid)glbSipParserPVisitedNetworkIdHeader, \
			    (((SipHeaderParserParam *) pHeaderParserParam)-> \
    			pError))!=SipSuccess)
	    			YYABORT;
		    glbSipParserPVisitedNetworkIdHeader = SIP_NULL;
    	};

pvisitednetworkidfields: 
		token
        {
		    if(sip_initSipPVisitedNetworkIdHeader(&glbSipParserPVisitedNetworkIdHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;
            
		    glbSipParserPVisitedNetworkIdHeader->\
                pVNetworkSpec = sip_tokenBufferDup($1);
        }
		|QSTRING
        {
		    if(sip_initSipPVisitedNetworkIdHeader(&glbSipParserPVisitedNetworkIdHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;
              
		    glbSipParserPVisitedNetworkIdHeader->\
                pVNetworkSpec = sip_tokenBufferDup($1);
        }
        |token
        {
		    if(sip_initSipPVisitedNetworkIdHeader(&glbSipParserPVisitedNetworkIdHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;
            
		    glbSipParserPVisitedNetworkIdHeader->\
                pVNetworkSpec = sip_tokenBufferDup($1);
        
        }pvisitednetworkidparams       
	
        |QSTRING 
        {
		    if(sip_initSipPVisitedNetworkIdHeader(&glbSipParserPVisitedNetworkIdHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;
            
		    glbSipParserPVisitedNetworkIdHeader->\
                pVNetworkSpec = sip_tokenBufferDup($1);
       
        }pvisitednetworkidparams;

pvisitednetworkidparams: pvisitednetworkidparam
		| pvisitednetworkidparams pvisitednetworkidparam;

pvisitednetworkidparam: gppsemtoken
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
		if(sip_listAppend(&(glbSipParserPVisitedNetworkIdHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
		| gppsemtoken EQUALS allstring
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

		if(sip_listAppend(&(glbSipParserPVisitedNetworkIdHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	};		


pcfaddr: PCFADDR
    {
   		    SipHeaderOrderInfo *order;
    		SipBool	dResult;

           if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
		    	pGeneralHdr->pPcfAddrHdr!=SIP_NULL)
    		{
	    		sip_error (SIP_Minor, "There can only be one P-Charging-Function-Addresses Header");
		    	*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = \
			    E_PARSER_ERROR;
    			YYABORT;
	    	}

	    	dResult=sip_initSipHeaderOrderInfo(&order,\
		    	((SipHeaderParserParam *)pHeaderParserParam)->pError);
    		if (dResult!=SipSuccess)
	    	{
		    	YYABORT;
    		}
    		order->dType = SipHdrTypePcfAddr;
	    	order->dTextType = SipFormFull;
		    order->dNum = 1;

    		if(sip_listAppend(&((SipHeaderParserParam *) \
	    		pHeaderParserParam)->pSipMessage->slOrderInfo, \
		    	(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
			    pHeaderParserParam)->pError))!=SipSuccess)
				    YYABORT;
		    if(sip_initSipPcfAddrHeader(&glbSipParserPcfAddrHeader,\
			    (((SipHeaderParserParam *) pHeaderParserParam)->pError))\
    			!=SipSuccess)
	    		YYABORT;

    }pcfaddrparams
    {
            ((SipHeaderParserParam *)pHeaderParserParam)->pSipMessage->\
              pGeneralHdr->pPcfAddrHdr =\
                glbSipParserPcfAddrHeader;
  		    glbSipParserPcfAddrHeader = SIP_NULL;
    
    };

pcfaddrparams: pcfaddrparam
	   | pcfaddrparams pcfaddrparam;

pcfaddrparam: token
    {
		SipParam *pParam;
		SIP_U32bit index;

		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
        /*No semicolon is present, so start the index from 0 */
		index=0;
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
		if(sip_listAppend(&(glbSipParserPcfAddrHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
	| token EQUALS allstring
    {
		SipParam *pParam;
		SIP_S8bit *tempVal;
		SIP_U32bit index;
		
		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *)  \
				pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		index=0;
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

		if(sip_listAppend(&(glbSipParserPcfAddrHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
    |gppsemtoken
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
		if(sip_listAppend(&(glbSipParserPcfAddrHeader->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	}
	| gppsemtoken EQUALS allstring
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

		if(sip_listAppend(&(glbSipParserPcfAddrHeader->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError)) == SipFail)
			YYABORT;
	};

allstring: token
		| QSTRING
		|IPV6ADDR;

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
gppsemtoken: SEMTOKEN
	|TAG;
semparamtokens:	SEMPARAMTOKEN 
			|SEMTOKEN
	        |TAG;
		
headers:	QUESTION 
			{
				glbSipParser3gppAllowSipColon=1;
			}
			uricdummy
			|
			{
				glbSipParser3gppAllowSipColon=0;
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
			glbSipParser3gppAllowSipColon=0;
		};	
		
newalltoken:SEMPARAMVALUE
		| token
		| ipv6
		|QSTRING;
		
token:	TOKEN		
		|digits		
		|REFTOKEN;

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
		|SEMTOKEN
		|SEMICOLON
		|QSTRING1
		|IPV6ADDR
		|COMMA
		|SEMPARAMVALUE
		|SEMPARAMTOKEN
		|PLAINSIPCOLON;
		
digits:	DIGITS;


