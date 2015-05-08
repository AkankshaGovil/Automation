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

%token ACCEPTCONTACT COLON RANGLE AMPERSAND
%token AT EQUALS DIGITS 
%token QSTRING COMMA STAR TOKEN HASH LANGLE RANGLE
%token URICNOCOLON QUESTION SIPFLOAT COMMA 
%token QVAL
%token SEMTOKEN SEMPARAMTOKEN IPV6ADDR NUMERIC_RELATION
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
en_AcceptContactType dType = SipAccContactTypeAny ;
static SIP_S8bit * pGlbName = SIP_NULL ;
static SipList *pTagValList = SIP_NULL ;
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
acceptcontact:acceptcontactcolon contacts
             ;

contacts : contacts COMMA contact
		{
			 SipHeaderOrderInfo *order=SIP_NULL;
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
    | contact
		{
				SipHeaderOrderInfo *order;
				SipBool	dResult;

				/* create and update order table entry */
				if (sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
						pSipMessage->u.pRequest->pRequestHdr->slAcceptContactHdr,\
						(SIP_Pvoid) glbSipParserAcceptContactHeader, \
						(((SipHeaderParserParam *) pHeaderParserParam)->pError)) 
								!= SipSuccess)
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
						pSipMessage->slOrderInfo, (SIP_Pvoid) order,
						(((SipHeaderParserParam *)\
						pHeaderParserParam)->pError)) != SipSuccess)
					YYABORT;
	 };
				
contact : STAR				 
          {
			if (sip_ccp_initSipAcceptContactHeader(&glbSipParserAcceptContactHeader, \
				(((SipHeaderParserParam *) pHeaderParserParam)->pError)) != SipSuccess)
				YYABORT;
		   } restparams 
		   ;
					
restparams : 
           |acrcvalues 
           ; 

acrcvalues:acrcvalues acparams
					| acparams
					;
        
acparams:SEMTOKEN { 
		/* save the token into global Name */
	     SIP_U32bit	index=1;
		 while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				 index++;

		 $1.pToken = &($1.pToken[index]);
		 $1.dLength -= index;
		 pGlbName = sip_tokenBufferDup($1);
		} paramlist
        ;  

paramlist : 
      { 
			  SipAcceptContactParam *cparam=SIP_NULL ;
			  dType = SipAccContactTypeOther ; 
			  if (sip_ccp_initSipAcceptContactParam(&cparam, dType, \
					((SipHeaderParserParam *) pHeaderParserParam)->pError) 
							!= SipSuccess)
			  {
				  YYABORT;
			  }
			  /* Put globalName as Name in pToken */
			  cparam->u.pToken = pGlbName;
			  pGlbName = SIP_NULL ;	
			  if (sip_listAppend(&glbSipParserAcceptContactHeader->\
									slAcceptContactParams, (SIP_Pvoid) cparam, 
									(((SipHeaderParserParam *)pHeaderParserParam)->pError)) 
												!= SipSuccess)
			  {
								YYABORT;
			  }
		  }
      | EQUALS alltoken 
	     { 
		  	SipAcceptContactParam *cparam=SIP_NULL ;
  			SipParam *pParam = SIP_NULL ;
		  	SIP_S8bit      *pValue, *temp, *curr, *pos;
  
  			dType = SipAccContactTypeGeneric ;
  			if (sip_ccp_initSipAcceptContactParam(&cparam, dType, \
  						((SipHeaderParserParam *) pHeaderParserParam)->pError) 
  							!= SipSuccess)
  			{
  				YYABORT;
  			}
        if (sip_initSipParam(&pParam, (((SipHeaderParserParam *)\
                            pHeaderParserParam)->pError)) == SipFail)
	  		   	YYABORT;
         pParam->pName = pGlbName ;
	       pGlbName = SIP_NULL ;	
		     pValue = (SIP_S8bit *) sip_tokenBufferDup($2) ;
			  if (pValue == SIP_NULL)
			  {
			  		*(((SipHeaderParserParam *) pHeaderParserParam)
			  						->pError) = E_NO_MEM;
			  		YYABORT;
			  }
			  /* separate the  comma separated fields in a quoted string */
			  if (pValue[0] == '\"')
		  	  {
			  	curr = pValue + 1;
			  	while ((curr != SIP_NULL) && (pos = strchr(curr, ',')) 
			  					!= SIP_NULL)
			  	{
			  		temp = (char *) fast_memget(BISON_MEM_ID, pos - curr + 3,\
			  			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
			  		if (temp == SIP_NULL)
			  		{
			  				*(((SipHeaderParserParam *) pHeaderParserParam)	
			  								->pError) = E_NO_MEM;
			  				YYABORT;
			  		}
			  		STRCPY(temp, "");
			  		strncat(temp, curr, pos - curr);
			  		if (sip_listAppend(&(pParam->slValue), temp, \
			  			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) 
			  						== SipFail)
			  		   YYABORT;
  
  				   curr = pos + 1;
  			   }
  			   if(curr==(pValue+1))
  			   {
  				   temp = (char *) fast_memget(BISON_MEM_ID, strlen(pValue)+1,\
  						(((SipHeaderParserParam *) pHeaderParserParam)->pError));
  				   if (temp == SIP_NULL)
  				   {
  						   *(((SipHeaderParserParam *) pHeaderParserParam)
  										   ->pError) = E_NO_MEM;
  						   YYABORT;
  				   }
						 STRCPY(temp,pValue) ;
  			   } 
  			   else
  			   {
  					   temp = (char *) fast_memget(BISON_MEM_ID, strlen(curr)+3, \
  								  (((SipHeaderParserParam *) pHeaderParserParam)
										->pError));
  					   if (temp == SIP_NULL)
  					   {
  							   *(((SipHeaderParserParam *) pHeaderParserParam)
  											   ->pError) = E_NO_MEM;
  							   YYABORT;
  					   }
  					   STRCPY(temp, "");
  					   strncat(temp, curr, strlen(curr) - 1);
  			   }
  			   if (sip_listAppend(&(pParam->slValue), temp,
  					 (((SipHeaderParserParam *)pHeaderParserParam)->pError))
  							   == SipFail)
  				   YYABORT;
  			   fast_memfree(BISON_MEM_ID, pValue, SIP_NULL);
  		    } 
  			  else
  			  {
  				   if (sip_listAppend(&(pParam->slValue), pValue, \
  					  (((SipHeaderParserParam *) pHeaderParserParam)->pError)) 
  						   == SipFail)
							   YYABORT;
  			  }
  			  cparam->u.pParam = pParam;
  			  if (sip_listAppend(&glbSipParserAcceptContactHeader->\
  									slAcceptContactParams, (SIP_Pvoid) cparam, 
  								(((SipHeaderParserParam *)pHeaderParserParam)->pError)) 
  												!= SipSuccess)
  			  {
  								YYABORT;
  			  }
  			  cparam = NULL ;
  		 }
       | EQUALS LANGLE 
	     { 
	  		SipAcceptContactParam *cparam=SIP_NULL ;
	  		SipParam *pParam = SIP_NULL ;
  
  			dType = SipAccContactTypeFeature ;
  			if (sip_ccp_initSipAcceptContactParam(&cparam, dType, \
  						((SipHeaderParserParam *) pHeaderParserParam)->pError) 
  							!= SipSuccess)
  			{
  				YYABORT;
  			}
  			if (sip_initSipParam(&pParam, (((SipHeaderParserParam *)\
  									pHeaderParserParam)->pError)) == SipFail)
  					YYABORT;
  			pParam->pName = pGlbName ;
  	    pGlbName = SIP_NULL ;	
  			pTagValList = &(pParam->slValue) ;
  			cparam->u.pParam = pParam ;
  			if (sip_listAppend(&glbSipParserAcceptContactHeader->\
  			     slAcceptContactParams, (SIP_Pvoid) cparam, 
  				(((SipHeaderParserParam *)pHeaderParserParam)->pError)) 
  								!= SipSuccess)
  			{
  								YYABORT;
  			}
      } tagvaluelist RANGLE 
	    ;
	
tagvaluelist:tagvaluelist COMMA tagvalue
            | tagvalue 
	          ;
					
tagvalue:TOKEN
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
			
numeric:HASH  numeric_relation digits
	    {
			  SIP_S8bit *pValue = SIP_NULL ;

				/* Append the tag value list with $1*/
				pValue = (SIP_S8bit *)fast_memget(BISON_MEM_ID,strlen($2.pToken)+3,
										(((SipHeaderParserParam *) pHeaderParserParam)->pError));
				if (pValue == SIP_NULL)
				{
								*(((SipHeaderParserParam *) pHeaderParserParam)->pError) 
												= E_NO_MEM;
								YYABORT;
				}
				STRCPY(pValue,"#") ;
				STRCAT(pValue,$2.pToken) ;
				/*STRCAT(pValue,$3.pToken) ;
				 * */
				if (sip_listAppend((pTagValList), pValue, \
						(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
					YYABORT;
			}
      ;
	
numeric_relation:NUMERIC_RELATION 
     | EQUALS  
     | digits COLON
     ;

digits:DIGITS
       |SIPFLOAT
	   ;

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

alltoken:TOKEN 
        | QSTRING
	| IPV6ADDR
	;
