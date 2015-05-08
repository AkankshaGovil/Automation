/***************************************************************************
 ** FUNCTION:
 **             Yacc file for congestion related(Resource-Priority)headers

 ***************************************************************************
 **
 ** FILENAME:
 ** Congest.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form Token
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 **
 **     Copyright 2003, Hughes Software Systems, Ltd.
 *********************************************************************/


%token RESOURCEPRIORITY ACCEPTRESOURCEPRIORITY   
%token ALPHANUM ALPHA DIGIT UALPHA LALPHA MARK
%token COMMA DOT DASH
%pure_parser
%{ 
#include "sipparserinc.h"
#include <stdlib.h>
#include "sipdecodeintrnl.h"
#include "sipstruct.h"

#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "siplist.h"
#include "rprinit.h"
#include "siptrace.h"

#define MAXTEMP (300)
#define MAX_HOPS (255)

static en_HeaderType glbSipCongestHdrType;
#ifdef SIP_CONGEST
static SipRsrcPriorityHeader* glbSipRsrcPriorityHdr = SIP_NULL;
#endif
#define YYPARSE_PARAM pHeaderParserParam
%}
%%
pHeader:resourcepriority
		|acceptresourcepriority
		|error
		{
			if(glbSipRsrcPriorityHdr != SIP_NULL)
				{
					sip_freeSipRsrcPriorityHeader(glbSipRsrcPriorityHdr);
					glbSipRsrcPriorityHdr=SIP_NULL;
				}
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					==E_NO_ERROR)
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
					=E_PARSER_ERROR;
            
		};

resourcepriority: 
    {
#ifdef SIP_CONGEST
		SipHeaderOrderInfo *order;
		SipBool 	dResult;
        if(((SipHeaderParserParam *) pHeaderParserParam)->\
		    		pSipMessage->dType!=SipMessageRequest)
    	{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				 = E_NO_ERROR;
            sip_error(SIP_Major,\
				 	"Header should be part of a Request message");
			YYABORT;
		}
        glbSipCongestHdrType=SipHdrTypeRsrcPriority;
		dResult = sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		
		if(dResult != SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeRsrcPriority;
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
    }RESOURCEPRIORITY resourcevalues;

acceptresourcepriority: 
    {
#ifdef SIP_CONGEST
		SipHeaderOrderInfo *order;
		SipBool 	dResult;
        if(((SipHeaderParserParam *) pHeaderParserParam)->\
	    		pSipMessage->dType!=SipMessageResponse)
    	{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				 = E_NO_ERROR;
            sip_error(SIP_Major,\
				 	"Header should be part of a Response message");
			YYABORT;
		}

		glbSipCongestHdrType=SipHdrTypeAcceptRsrcPriority;
		dResult = sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
		
		if(dResult != SipSuccess)
		{
			YYABORT;
		}
		order->dType = SipHdrTypeAcceptRsrcPriority;
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

    }ACCEPTRESOURCEPRIORITY resourcevalues;

resourcevalues: resourcevalues COMMA resourcevalue 
    {
#ifdef SIP_CONGEST
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
        
		if (glbSipCongestHdrType==SipHdrTypeRsrcPriority)
        {
            if(sip_listAppend(&((SipHeaderParserParam *) \
			    pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
                slRsrcPriorityHdr,(SIP_Pvoid)glbSipRsrcPriorityHdr,\
                ((SipHeaderParserParam *)pHeaderParserParam)->\
                                pError) != SipSuccess)
            {
                YYABORT;
            } 
        }else
        {
            if(sip_listAppend(&((SipHeaderParserParam *) \
			    pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
                slAcceptRsrcPriorityHdr,(SIP_Pvoid)glbSipRsrcPriorityHdr,\
                ((SipHeaderParserParam *)pHeaderParserParam)->\
                                pError) != SipSuccess)
            {
                YYABORT;
            } 

        }
        
        glbSipRsrcPriorityHdr=SIP_NULL;
		
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
#endif
    }
    | resourcevalue
    {
#ifdef SIP_CONGEST
		if (glbSipCongestHdrType==SipHdrTypeRsrcPriority)
        {
            if(sip_listAppend(&((SipHeaderParserParam *) \
			    pHeaderParserParam)->pSipMessage->u.pRequest->pRequestHdr->\
                slRsrcPriorityHdr,(SIP_Pvoid)glbSipRsrcPriorityHdr,\
                ((SipHeaderParserParam *)pHeaderParserParam)->\
                                pError) != SipSuccess)
            {
                YYABORT;
            }
        
        }else{
            if(sip_listAppend(&((SipHeaderParserParam *) \
			    pHeaderParserParam)->pSipMessage->u.pResponse->pResponseHdr->\
                slAcceptRsrcPriorityHdr,(SIP_Pvoid)glbSipRsrcPriorityHdr,\
                ((SipHeaderParserParam *)pHeaderParserParam)->\
                                pError) != SipSuccess)
            {
                YYABORT;
            } 
        }
        
        glbSipRsrcPriorityHdr=SIP_NULL;
#endif
    };
        
resourcevalue:
     {
         if (sip_initSipRsrcPriorityHeader(&glbSipRsrcPriorityHdr,\
              ((SipHeaderParserParam *)pHeaderParserParam)->\
              pError) != SipSuccess)
        {
            YYABORT;
        }
     }
     namespace DOT priority;

namespace:ALPHANUM
    {
#ifdef SIP_CONGEST
        glbSipRsrcPriorityHdr->pNamespace = sip_tokenBufferDup($1);
#endif
    
    }
    | DASH
    {
#ifdef SIP_CONGEST
        glbSipRsrcPriorityHdr->pNamespace = sip_tokenBufferDup($1);
#endif
    };
     
priority:ALPHANUM
    {
#ifdef SIP_CONGEST
        glbSipRsrcPriorityHdr->pPriority = sip_tokenBufferDup($1);
#endif
    
    }
    | DASH
    {
#ifdef SIP_CONGEST
        glbSipRsrcPriorityHdr->pPriority = sip_tokenBufferDup($1);
#endif
    };


