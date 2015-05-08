/***********************************************************************
 ** FUNCTION:
 **             Yacc file for  Message Waiting Msg Body
 *********************************************************************
 **
 ** FILENAME:
 ** MesgSummary.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form sdp
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 07/01/2002   Sasidhar P V K                          Initial Creation
 **
 **     Copyright 2002, Hughes Software Systems, Ltd.
 *********************************************************************/
%token MWAITING LEFTPARAN SLASH RIGHTPARAN DIGITS YES NO LANGLE RANGLE 
%token MEDIATYPE HNAME HVALUE CRLF DEFAULT TAG QSTRING QSTRING1 COLON
%token MACCOUNT SIPCOLON SIPSCOLON IPV6ADDR URICNOCOLON	USERINFOAT EQUALS	QUESTION 
%token SEMICOLON COMMA TOKEN SEMPARAMTOKEN SEMTOKEN SEMPARAMVALUE PLAINSIPCOLON

%pure_parser
%{
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include <stdlib.h>
#include <string.h>
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "siplist.h"

#define MIN_DIGITS_RANGE (48)
#define MAX_DIGITS_RANGE (57)
#define MIN_UALPHA_RANGE (65)
#define MAX_UALPHA_RANGE (90)
#define MIN_LALPHA_RANGE (97)
#define MAX_LALPHA_RANGE (122)

extern int glbSipParserMesgSummaryAllowSipColon;
#define YYPARSE_PARAM pMesgSummaryParserParam 
#define MESG_SUMMARY_ERROR (*(((SipMesgSummaryParserParam *)pMesgSummaryParserParam)->pError))
#define MESG_SUMMARY_MESSAGE (((SipMesgSummaryParserParam *)pMesgSummaryParserParam)->pMesgSummaryMessage)
#define MESG_SUMMARY_GC_LIST (((SipMesgSummaryParserParam *)pMesgSummaryParserParam)->pGCList)
extern SIP_U8bit neglectMedia; /*This is Used to Reject Media Token Once we get HNAME						  or H HVALUE */
static SipAddrSpec *glbSipParserAddrSpec=SIP_NULL;
%}

%%

line: statusline msgaccount summaryline mheader  
      | statusline summaryline mheader 
      | statusline msgaccount summaryline 
      | statusline msgaccount mheader 
      | statusline msgaccount 
      | statusline summaryline 
      | statusline mheader
      | statusline
      |error
      {
	 if( MESG_SUMMARY_ERROR == E_NO_ERROR ) MESG_SUMMARY_ERROR = E_PARSER_ERROR;
      };

msgaccount : MACCOUNT  account_uri CRLF;

account_uri : LANGLE addrspec RANGLE
    {

				MESG_SUMMARY_MESSAGE->pAddrSpec =  glbSipParserAddrSpec;
				glbSipParserAddrSpec = SIP_NULL;
	} 
    | addrspec
    {
				MESG_SUMMARY_MESSAGE->pAddrSpec =  glbSipParserAddrSpec;
				glbSipParserAddrSpec = SIP_NULL;
	};
 

addrspec:sipurl 
		|uricval
        ;
		
uricval:uric
   {
		 if ( sip_initSipAddrSpec(&glbSipParserAddrSpec,SipAddrReqUri,
				 &(MESG_SUMMARY_ERROR) ) != SipSuccess)
			 YYABORT;

         glbSipParserAddrSpec->u.pUri = sip_tokenBufferDup($1);
		 if(glbSipParserAddrSpec->u.pUri==SIP_NULL)
		 {
		 	 MESG_SUMMARY_ERROR = E_NO_MEM ;
			 YYABORT;
		 }
  };
     

sipurl:sipcolon  userinfoat hostport urlparams headers
		|sipcolon  hostport urlparams headers
		;
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
				MESG_SUMMARY_ERROR = E_NO_MEM ;
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
				  (MESG_SUMMARY_ERROR) = E_NO_MEM ;
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

sipcolon:SIPCOLON
	{

		if(sip_initSipAddrSpec(&glbSipParserAddrSpec,SipAddrSipUri, 
				&(MESG_SUMMARY_ERROR)) != SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , 
					&(MESG_SUMMARY_ERROR)) != SipSuccess)
				YYABORT;
	}
        |SIPSCOLON
    {
		if(sip_initSipAddrSpec(&glbSipParserAddrSpec,SipAddrSipSUri, \
				&(MESG_SUMMARY_ERROR)) != SipSuccess)
			YYABORT;
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
				&(MESG_SUMMARY_ERROR)) != SipSuccess)
				YYABORT;
	};
hostport:	host COLON port
	{
		SIP_S8bit *temp;

		glbSipParserAddrSpec->u.pSipUrl->pHost = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost==SIP_NULL)
		{
		  (MESG_SUMMARY_ERROR) = E_NO_MEM ;
			YYABORT;
		}
		glbSipParserAddrSpec->u.pSipUrl->dPort = (SIP_U16bit *)  \
			fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit),&(MESG_SUMMARY_ERROR) );\
		if(glbSipParserAddrSpec->u.pSipUrl->dPort==SIP_NULL)
			YYABORT;
		temp = sip_tokenBufferDup($3);
		*(glbSipParserAddrSpec->u.pSipUrl->dPort) = atoi(temp);
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);
	}
  | host
	{
		glbSipParserAddrSpec->u.pSipUrl->pHost = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost==SIP_NULL)
		{
		  (MESG_SUMMARY_ERROR) = E_NO_MEM ;
			YYABORT;
		}
	};

host:token
	 |ipv6;

port:DIGITS;

token:TOKEN
	|DIGITS;		

ipv6:IPV6ADDR;

urlparams: 	urlparams urlparam
		|;

urlparam:semparamtokens EQUALS newalltoken 
         {
		    SipParam *param; 
			int index;		
			SIP_S8bit *yyuparam;

			if(sip_initSipParam(& param,&(MESG_SUMMARY_ERROR)) != SipSuccess)
				YYABORT;
			/* Ignore initial ';' and whitespace before parameter name */
			index=1;
			while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				index++;
			param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
							$1.dLength-index+1, &(MESG_SUMMARY_ERROR)) ;
			if(param->pName==SIP_NULL)
				YYABORT;
			strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
			param->pName[$1.dLength-index] = '\0';

			yyuparam=sip_tokenBufferDup($3);	
			if(sip_listAppend(&param->slValue,(SIP_Pvoid)yyuparam,\
							&(MESG_SUMMARY_ERROR))!=SipSuccess)
    			YYABORT;

			if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
							(SIP_Pvoid)param,&(MESG_SUMMARY_ERROR))!=SipSuccess)
				YYABORT;
		 }
		|semparamtokens
	     {
			SipParam *param; 
			int index;		
 
			if(sip_initSipParam(& param,&(MESG_SUMMARY_ERROR))!=SipSuccess)
					YYABORT;
			/* Ignore initial ';' and whitespace before parameter name */
			index=1;
			while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				index++;
			param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
							$1.dLength-index+1, &(MESG_SUMMARY_ERROR));
			if(param->pName==SIP_NULL)
				YYABORT;
			strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
			param->pName[$1.dLength-index] = '\0';

			if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
							(SIP_Pvoid)param,&(MESG_SUMMARY_ERROR))!=SipSuccess)
				YYABORT;
    	 };
				
semtoken:SEMTOKEN;

semparamtokens:	SEMPARAMTOKEN 
			|SEMTOKEN 
			|TAG;

newalltoken:SEMPARAMVALUE
		| token
		| ipv6
		| QSTRING;
		
			
headers:QUESTION 
		{
			glbSipParserMesgSummaryAllowSipColon=1;
		}	
		uricdummy	
		|
        { 
			glbSipParserMesgSummaryAllowSipColon=0;
		};


uricdummy:	uric
    {
		glbSipParserAddrSpec->u.pSipUrl->pHeader = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHeader==SIP_NULL)
		{
			MESG_SUMMARY_ERROR = E_NO_MEM;
			YYABORT;
		}
	 	glbSipParserMesgSummaryAllowSipColon=0;
	};

uric:uric uricnocolon	
    |uric COLON	
 	|uricnocolon 
	|COLON; 

uricnocolon:TOKEN	
		|DIGITS			
		|URICNOCOLON	
		|USERINFOAT		
		|EQUALS 			
		|QUESTION
		|semtoken
		|SEMICOLON
		|IPV6ADDR
		|COMMA
		|SEMPARAMVALUE
		|SEMPARAMTOKEN
		|QSTRING1
		|PLAINSIPCOLON;
					
statusline: MWAITING status CRLF
	   {
		
		SIP_S8bit *token;
		token = sip_tokenBufferDup($2);
		if( ( *token == 'Y' ) || ( *token == 'y' ) )
		MESG_SUMMARY_MESSAGE->dStatus = SipMsgWaitingYes;
		else
		MESG_SUMMARY_MESSAGE->dStatus = SipMsgWaitingNo;
		fast_memfree(BISON_MEM_ID, token, SIP_NULL);
		/* Intialize the flag */
		neglectMedia = 0;
		
	   };
status: YES
	|NO;
summaryline:summaryline1 
	    | summaryline summaryline1;
summaryline1: MEDIATYPE DIGITS SLASH DIGITS CRLF
	    {
		SummaryLine *pSummaryLine;
		SIP_S8bit *token;

		sip_mwi_initSummaryLine(&pSummaryLine,&MESG_SUMMARY_ERROR);

		/* remove LWS and : at the end */
		token = sip_tokenBufferDup($1);
		/* Make the First letter as Upper Case */
		if( *token >= MIN_LALPHA_RANGE ) *token -= 32; 
		pSummaryLine->pMedia = token;
		while(( *token != ' ') && ( *token != ':' ))token++;
		*token = '\0';

		token = sip_tokenBufferDup($2);
		pSummaryLine->newMessages = (SIP_U32bit)STRTOU32CAP(token, SIP_NULL);
		fast_memfree(BISON_MEM_ID, token, SIP_NULL);
		token = sip_tokenBufferDup($4);
		pSummaryLine->oldMessages = (SIP_U32bit)STRTOU32CAP(token, SIP_NULL);
		fast_memfree(BISON_MEM_ID, token, SIP_NULL);
		pSummaryLine->newUrgentMessages = 0;
		pSummaryLine->oldUrgentMessages = 0;
		if(sip_listAppend(&(MESG_SUMMARY_MESSAGE->slSummaryLine), (SIP_Pvoid) pSummaryLine , \
			&MESG_SUMMARY_ERROR) == SipFail)
			YYABORT;	
		
		
	    }
	    |MEDIATYPE DIGITS SLASH DIGITS LEFTPARAN DIGITS SLASH DIGITS RIGHTPARAN CRLF
	    {
		SummaryLine *pSummaryLine;
		SIP_S8bit *token;

		sip_mwi_initSummaryLine(&pSummaryLine,&MESG_SUMMARY_ERROR);

		/* remove LWS and : at the end */
		token = sip_tokenBufferDup($1);
		/* Make the First letter as Upper Case */
		if( *token >= MIN_LALPHA_RANGE ) *token -= 32; 
		pSummaryLine->pMedia = token;
		while(( *token != ' ') && ( *token != ':' ))token++;
		*token = '\0';

		token = sip_tokenBufferDup($2);
		pSummaryLine->newMessages = (SIP_U32bit)STRTOU32CAP(token, SIP_NULL);
		fast_memfree(BISON_MEM_ID, token, SIP_NULL);
		token = sip_tokenBufferDup($4);
		pSummaryLine->oldMessages = (SIP_U32bit)STRTOU32CAP(token, SIP_NULL);
		fast_memfree(BISON_MEM_ID, token, SIP_NULL);
		token = sip_tokenBufferDup($6);
		pSummaryLine->newUrgentMessages = (SIP_U32bit)STRTOU32CAP(token, SIP_NULL);
		fast_memfree(BISON_MEM_ID, token, SIP_NULL);
		token = sip_tokenBufferDup($8);
		pSummaryLine->oldUrgentMessages = (SIP_U32bit)STRTOU32CAP(token, SIP_NULL);
		fast_memfree(BISON_MEM_ID, token, SIP_NULL);
		if(sip_listAppend(&(MESG_SUMMARY_MESSAGE->slSummaryLine), (SIP_Pvoid) pSummaryLine , \
			&MESG_SUMMARY_ERROR) == SipFail)
			YYABORT;	
		

	    };
mheader:mheader1 
        | mheader mheader1;
hname : TOKEN 
		{
			SIP_S8bit dChar;
			dChar = $1.pToken[0];
			if(!(((dChar>=MIN_DIGITS_RANGE  && dChar<=MAX_DIGITS_RANGE)\
				|| (dChar>=MIN_UALPHA_RANGE && dChar<=MAX_UALPHA_RANGE)\
				|| (dChar>=MIN_LALPHA_RANGE && dChar<=MAX_LALPHA_RANGE))\
				&& ($1.dLength >=2)))
			{
				MESG_SUMMARY_ERROR = E_PARSER_ERROR;
				YYABORT;
			}
		};
mheader1:  hname HVALUE 
	 {
		SipNameValuePair *pNameValue;
		sip_initSipNameValuePair(&pNameValue,&MESG_SUMMARY_ERROR);
		pNameValue->pName = sip_tokenBufferDup($1);
		/* remove ':'  and at the begining */
		$2.pToken++;
		$2.dLength--;
		/* if there is a space at the begining remove that */
		if(*($2.pToken) == ' ')
		{
			$2.pToken++;
			$2.dLength--;
		}
		pNameValue->pValue = sip_tokenBufferDup($2);
		if(sip_listAppend(&(MESG_SUMMARY_MESSAGE->slNameValue), (SIP_Pvoid) pNameValue , \
			&MESG_SUMMARY_ERROR) == SipFail)
		YYABORT;	
		/* From now on Treat Media Token as HNAME */
		neglectMedia = 1;
		
	 }| CRLF;
