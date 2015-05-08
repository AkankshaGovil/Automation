/***********************************************************************
 ** FUNCTION:
 **             Yacc file for Tel Url

 *********************************************************************
 **
 ** FILENAME:
 ** Tel.y
 **
 ** DESCRIPTION:
 ** This has all the headers of form From/to
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 
 ** 5-1-2001   	Mahesh Govind		Initial creation			Support For Tel Url
 ** Copyright 2001, Hughes Software Systems, Ltd.
 *********************************************************************/
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
#include "telstruct.h"
#include "telinit.h"
#include "telfree.h"
#include "teldecodeintrnl.h"
/*
#include "sipclone.h"
*/
#define MAXTEMP	(300)

extern int glbtelabort;
extern int glbtelabort1;
static SipList      glbslParam;
static SipList	     glbslAreaSpecifier;/* siplist of SIP_S8bit*   */
static SipParam    *glbSipParam=SIP_NULL;

#define YYPARSE_PARAM pTelParserParam


%}

%token PHONETAG PHONEDIGIT PROVIDERTAG PAUSETOKEN PLUS TOKEN TOKENPLUS IPV6ADDR
%token DTMFDIGIT QSTRING ISDNTAG TELCOLON POSTDIALTAG EQUALS 
%token TOKENSEPARATOR DTMFTOKEN DTMFALPHA PHONENUMBER SEMICOLONGENERALTOKEN QUESTIONMARKGENERALTOKEN EQUALSGENERALTOKEN
%token DIGIT VISUALSEPARATOR EQUALSTAGGENERALTOKEN DEFAULT
%%
telheader:telcolon telephonesubscriber
		|error
		{
			SIP_U32bit size1=0,size2=0;
			SipError err1;
			sip_listSizeOf(&glbslParam,&size1,&err1);
       		sip_listSizeOf(&glbslAreaSpecifier,&size2,&err1);
       		if(size1!=0)
       		{
                sip_listDeleteAll(&glbslParam,&err1);
                glbslParam.size=0;
                if (((SipTelParserParam*)pTelParserParam)->pTelUrl->pGlobal)
                      ((SipTelParserParam*)pTelParserParam)->pTelUrl-> \
					  	pGlobal->slParams = glbslParam;
                if (((SipTelParserParam*)pTelParserParam)->pTelUrl->pLocal)
                    ((SipTelParserParam*)pTelParserParam)->pTelUrl-> \
						pLocal->slParams = glbslParam;

        	}
        	if(size2 !=0)
        	{
                sip_listDeleteAll(&glbslAreaSpecifier,&err1);
                glbslAreaSpecifier.size=0;
                if (((SipTelParserParam*)pTelParserParam)->pTelUrl->pGlobal)
                   ((SipTelParserParam*)pTelParserParam)->pTelUrl-> \
				   	pGlobal->slAreaSpecifier = glbslAreaSpecifier;
                if (((SipTelParserParam*)pTelParserParam)->pTelUrl->pLocal)
                    ((SipTelParserParam*)pTelParserParam)->pTelUrl-> \
					pLocal->slAreaSpecifier = glbslAreaSpecifier;

        	}
        	if(((SipTelParserParam*)pTelParserParam)->pTelUrl!=SIP_NULL)
			{
				sip_freeTelUrl (((SipTelParserParam*)pTelParserParam)-> \
					pTelUrl);
        		((SipTelParserParam*)pTelParserParam)->pTelUrl=SIP_NULL;
			}

			if(glbSipParam != SIP_NULL)
			{
				sip_freeSipParam(glbSipParam);	
				glbSipParam= SIP_NULL;
			}
			
			if(*(((SipTelParserParam*)pTelParserParam)->pError)==E_NO_ERROR)
				*(((SipTelParserParam*)pTelParserParam)->pError)=\
					E_PARSER_ERROR;
		};
				
						
telcolon: TELCOLON   					
		{
 		if(sip_initTelUrl(&((SipTelParserParam*)pTelParserParam)->\
			pTelUrl, (((SipTelParserParam*)pTelParserParam)->pError))\
			!=SipSuccess)
			YYABORT;
		if(sip_listInit(&glbslAreaSpecifier,&__sip_freeString \
		,(((SipTelParserParam*)pTelParserParam)->pError))!=SipSuccess)

						YYABORT;	
		if(sip_listInit(&glbslParam,&__sip_freeSipParam\
		, (((SipTelParserParam*)pTelParserParam)->pError)) 	!=SipSuccess)

					YYABORT;
		};
telephonesubscriber: phonenumber1 genparams localoptions1
		| phonenumber1 localoptions1
		| phonenumber1 
		| plus1 basephonenumber1 globalgenparams options
		{
			((SipTelParserParam*)pTelParserParam)->pTelUrl->pGlobal->\
			slAreaSpecifier=glbslAreaSpecifier;
			((SipTelParserParam*)pTelParserParam)->pTelUrl->pGlobal->\
			slParams=glbslParam;
		}
		| plus1 basephonenumber1 options
		{
			((SipTelParserParam*)pTelParserParam)->pTelUrl->pGlobal->\
			slAreaSpecifier=glbslAreaSpecifier;
			((SipTelParserParam*)pTelParserParam)->pTelUrl->pGlobal->\
			slParams=glbslParam;
		};
genparams: genparams isdbaddr1
		|genparams postdial1
		|isdbaddr1
		|postdial1;
globalgenparams: globalgenparams gisdnaddrs1
		|globalgenparams gpostdial1
		|gisdnaddrs1
		|gpostdial1;
plus1: PLUS
	    {
			if(sip_initTelGlobalNum(&((SipTelParserParam*)\
				pTelParserParam)->pTelUrl->pGlobal,\
				(((SipTelParserParam*)pTelParserParam)->pError))!=SipSuccess)
				YYABORT;
		}; 
gisdnaddrs1:isdnaddrs	
		{
			if($1.dLength!=0)
     			((SipTelParserParam*)pTelParserParam)->pTelUrl->\
				pGlobal->pIsdnSubAddr=sip_tokenBufferDup($1);
		};
gpostdial1:postdial
		{
			if($1.dLength!=0)
	    		((SipTelParserParam*)pTelParserParam)->pTelUrl->\
				pGlobal->pPostDial=sip_tokenBufferDup($1);
		};
phonenumber1:phonenumber
		{
			if(sip_initTelLocalNum(&((SipTelParserParam*)\
				pTelParserParam)->pTelUrl->pLocal, \
				(((SipTelParserParam*)pTelParserParam)->pError))\
				!=SipSuccess)
				YYABORT;
			((SipTelParserParam*)pTelParserParam)->pTelUrl->pLocal->\
			pLocalPhoneDigit=sip_tokenBufferDup($1);
		};
isdbaddr1:isdnaddrs 
		{
			if($1.dLength!=0)
	     		((SipTelParserParam*)pTelParserParam)->pTelUrl->\
				pLocal->pIsdnSubAddr=sip_tokenBufferDup($1);
		};
postdial1: 	postdial 
		 {
			if($1.dLength!=0)
	     		((SipTelParserParam*)pTelParserParam)->pTelUrl->\
				pLocal->pPostDial=sip_tokenBufferDup($1);
	  	};
localoptions1:PHONETAG equalsgeneraltoken options
		{ 

			((SipTelParserParam*)pTelParserParam)->pTelUrl->pLocal->\
			slAreaSpecifier = glbslAreaSpecifier;
			((SipTelParserParam*)pTelParserParam)->pTelUrl->pLocal->\
			slParams = glbslParam;
		}; 
basephonenumber1:phonedigit 
		{
			((SipTelParserParam*)pTelParserParam)->pTelUrl->pGlobal->\
			pBaseNo=sip_tokenBufferDup( $1 );
		};
isdnaddrs:ISDNTAG EQUALS phonedigit
		{
			$$.pToken=$3.pToken;
			$$.dLength=$3.dLength;   
		};
postdial:POSTDIALTAG EQUALS phonenumber
 		{
			 $$.pToken=unescapeCharacters($3.pToken);
		     $$.dLength=$3.dLength;   
		};
options:options option
			|;
		
option:futureextensions
      |serviceprovider
	  |PHONETAG equalsgeneraltoken;
equalsgeneraltoken:EQUALSTAGGENERALTOKEN
			{
				SIP_S8bit *temp;
				SIP_U32bit index;
				index=1;
				while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				index++;
				$1.pToken = &($1.pToken[index]);
				$1.dLength -= index;
				temp=sip_tokenBufferDup($1);

				/*calliing unescapeCharacters to convert to literals*/

				if(sip_listAppend(&glbslAreaSpecifier, \
				  (SIP_Pvoid)(unescapeCharacters(temp)), \
					(((SipTelParserParam*)pTelParserParam)->\
					pError))!=SipSuccess)
					YYABORT;
			};
serviceprovider:PROVIDERTAG EQUALS tokenplus
			{ 
				SipParam *pParam;
				SIP_U32bit index;

				if(sip_initSipParam(&pParam, (((SipTelParserParam*)\
					pTelParserParam)->pError))== SipFail)
					YYABORT;
				index=1;
				while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				index++;
				$1.pToken = &($1.pToken[index]);
				$1.dLength -= index;

				pParam->pName = sip_tokenBufferDup($1);
				if(pParam->pName == SIP_NULL)
				{
					*(((SipTelParserParam*)pTelParserParam)->pError) = E_NO_MEM;
					YYABORT;
				}
			  	if(sip_listAppend(&pParam->slValue,\
					(SIP_Pvoid)(unescapeCharacters(sip_tokenBufferDup ($3))),\
					(((SipTelParserParam*)pTelParserParam)->\
					pError))!=SipSuccess)
					 YYABORT;

				if(sip_listAppend(&glbslParam, pParam,\
					(((SipTelParserParam*)pTelParserParam)->pError))== SipFail)
					YYABORT;
		   };
futureextensions: SEMICOLONGENERALTOKEN
		{
			int index;
			glbtelabort=1;

			/* Ignore initial ';' and whitespace before parameter name */
			index=1;
			if(sip_initSipParam(&glbSipParam,(((SipTelParserParam*)pTelParserParam)->pError))\
				!=SipSuccess)
				YYABORT;

			while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
			glbSipParam->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
			$1.dLength-index+1,(((SipTelParserParam*)pTelParserParam)->pError));

			if(glbSipParam->pName==SIP_NULL)
				YYABORT;
			strncpy(glbSipParam->pName,unescapeCharacters(&($1.pToken[index])),\
			$1.dLength-index);
			glbSipParam->pName[$1.dLength-index] = '\0';
		}
		extopt
		{
			glbtelabort=0;
			if(sip_listAppend(&glbslParam,(SIP_Pvoid)glbSipParam, \
				(((SipTelParserParam*)pTelParserParam)->pError)) \
				!=SipSuccess)
				YYABORT;
			glbSipParam = SIP_NULL;
		};	

extopt:	EQUALSGENERALTOKEN
		{
			int index;
			SIP_S8bit* temp;
			index=1;
			while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
				index++;
			temp=(SIP_S8bit *) fast_memget(BISON_MEM_ID\
				, $1.dLength-index+1, \
				(((SipTelParserParam*)pTelParserParam)->pError));
			strncpy(temp,&($1.pToken[index]), $1.dLength-index);
			temp[$1.dLength-index] = '\0';

		 	if(sip_listAppend(&glbSipParam->slValue,(SIP_Pvoid)(\
				unescapeCharacters(temp)),(((SipTelParserParam*)\
				pTelParserParam)->pError))!=SipSuccess)
				YYABORT;
		 }
	| EQUALSGENERALTOKEN  QUESTIONMARKGENERALTOKEN 
		{
			int index,index1;
			SIP_S8bit* temp,*temp1,*buffer;
			int length,length1;
			index=1;
			index1=1;
			/* removes the Equal sign*/  
			while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
			temp=(SIP_S8bit *) fast_memget(BISON_MEM_ID,$1.dLength-index+1, \
				(((SipTelParserParam*)pTelParserParam)->pError));
			strncpy(temp,&($1.pToken[index]), $1.dLength-index);
			temp[$1.dLength-index] = '\0';
		
			temp1=(sip_tokenBufferDup($2));
			length=strlen(temp);
			length1=strlen(temp1);
			length=length+length1+1;
			buffer=(SIP_S8bit *) fast_memget(BISON_MEM_ID, length,\
				(((SipTelParserParam*)pTelParserParam)->pError));
		  	strcpy(buffer,temp);
			
			strcat(buffer,temp1);
			sip_memfree(BISON_MEM_ID,(SIP_Pvoid*)&temp,\
				(((SipTelParserParam*)pTelParserParam)->pError));
	
			sip_memfree(BISON_MEM_ID,(SIP_Pvoid*)&temp1,\
				(((SipTelParserParam*)pTelParserParam)->pError));	
			
			if(sip_listAppend(&glbSipParam->slValue, \
			(SIP_Pvoid)(unescapeCharacters(buffer)), \
			(((SipTelParserParam*)pTelParserParam)->pError))!=SipSuccess)
				YYABORT;
		}	 
   |EQUALS  
		{
			glbtelabort=1;
		}qstring
   |;
phonenumber:PHONENUMBER   
		|PHONEDIGIT
		|PAUSETOKEN
		|DTMFDIGIT
		|DTMFALPHA
		|DIGIT
		|TOKENSEPARATOR
		|VISUALSEPARATOR;
qstring:QSTRING
	{				
		SIP_S8bit *temp;
		int i=1,j=0;	
		int flg=0;
	
		/*conversion to literals*/
		temp=unescapeCharacters(sip_tokenBufferDup($1));
	

		j=strlen(temp)-1;/*length including ""*/
		
		/*search for " */
		while(i<(j))
		{
			if(temp[i]=='"')
			{
				if(temp[i-1]!='\\')
					{
						flg=1;
					}
			}
	      i++;
		}
		if(flg==1)
		{
			/* to set the errorvalue*/	
			 YYABORT; 
		}

		if(sip_listAppend(&glbSipParam->slValue,\
		 (SIP_Pvoid)(temp), \
		 (((SipTelParserParam*)pTelParserParam)->pError))!=SipSuccess)
			YYABORT;
	};
phonedigit:PHONEDIGIT
	|DIGIT
	|TOKENSEPARATOR
	|VISUALSEPARATOR;

tokenplus:TOKENPLUS
		|TOKEN
    	|TOKENSEPARATOR  
		|PLUS
		|DIGIT
		|DTMFTOKEN
		|PAUSETOKEN
		|DTMFALPHA
		|PHONEDIGIT
		|IPV6ADDR
		|PHONENUMBER;
