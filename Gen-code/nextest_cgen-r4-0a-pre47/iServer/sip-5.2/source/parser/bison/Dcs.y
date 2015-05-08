 /******************************************************************** 
 ** FUNCTION:
 **             Yacc file for Dcs Headers

 *********************************************************************
 **
 ** FILENAME:
 ** Dcs.y
 **
 ** DESCRIPTION:
 ** This has all the headers of Dcs
 **
 ** DATE   		    NAME                    REFERENCE               REASON
 ** ----        	----                    ---------              --------
 ** 13-Nov-2000 	Seshashayi T.									Initial
 **					Mahesh Govind
 **					Siddharth
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/
%token DCSMEDIAAUTH DCSANONYMITY DCSRPIDPRIVACY DCSREMOTEPARTYID DCSBILLINGID DCSSTATE DCSBILLINGINFO
%token DCSTRACEPARTYID DCSGATE DCSLAES DCSSESSION DCSOSPS DCSREDIRECT 
%token HEXTOKEN SIPCOLON SIPSCOLON PLAINSIPCOLON QSTRING QSTRING1 COLON COMMA SEMICOLON
%token EQUALS QUESTION RANGLE LANGLE DIGITS USERINFOAT FWSLASH TOKENEQUALS
%token IPV6ADDR URICNOCOLON SEMTOKEN TOKEN MUNRESERVED
%token ALPHANUM SEMALPHANUM CRLF SEMPARAMTOKEN SEMPARAMVALUE
%token DEFAULT DISPNAME DISPLAYNAME
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
#include "dcsinit.h"
#include "dcsfree.h"
#include "siptrace.h"

#define MAXTEMP (300)
static SipAddrSpec *glbSipParserAddrSpec=SIP_NULL;
static SipParam*		glbSipParserDcsSipParam=SIP_NULL;
static SipDcsAnonymityHeader*	glbSipParserDcsAnonymity=SIP_NULL;
static SipSessionHeader*		glbSipParserSession=SIP_NULL;
static SipDcsStateHeader*		glbSipParserDcsState=SIP_NULL;
static SIP_S8bit *glbSipTempStr=SIP_NULL;
static SipDcsAcctEntry*		glbSipParserAcctEntry=SIP_NULL;
static SipDcsLaesHeader*	glbSipParserDcsLaes = SIP_NULL;
static SipDcsGateHeader*	glbSipParserDcsGate = SIP_NULL;
static SipDcsRedirectHeader* glbSipParserDcsRedirect = SIP_NULL;
static SipDcsRemotePartyIdHeader* glbSipParserDcsRemotePartyId = SIP_NULL;
static SipDcsBillingInfoHeader*   glbSipParserDcsBillingInfo = SIP_NULL;
static SipDcsRpidPrivacyHeader*		glbSipParserDcsRpidPrivacy=SIP_NULL;
static SipList					glbslPrivacyParam;

int SetSipSColon=0,SetHostFlag=0;
extern int lexabort;
extern int glbSipParserDcsAllowSipColon;
#define YYPARSE_PARAM pHeaderParserParam
%}

%%
header:dcsheader
		|error
		{
			SipError dErr;
			SIP_U32bit sizePrivacyParam=0;

			SetHostFlag=0;
			SetSipSColon=0;
			if(glbSipParserDcsBillingInfo != SIP_NULL)
			{
				sip_dcs_freeSipDcsBillingInfoHeader(glbSipParserDcsBillingInfo);
				glbSipParserDcsBillingInfo = SIP_NULL;
			}
			if(glbSipParserDcsRemotePartyId != SIP_NULL)
			{
				sip_dcs_freeSipDcsRemotePartyIdHeader\
					(glbSipParserDcsRemotePartyId);
				glbSipParserDcsRemotePartyId = SIP_NULL;	
			}
			if(glbSipParserDcsRedirect != SIP_NULL)
			{
				sip_dcs_freeSipDcsRedirectHeader(glbSipParserDcsRedirect);
				glbSipParserDcsRedirect = SIP_NULL;
			}
			if(glbSipParserDcsGate != SIP_NULL)
			{
				sip_dcs_freeSipDcsGateHeader(glbSipParserDcsGate);
				glbSipParserDcsGate = SIP_NULL;
			}
			if(glbSipParserDcsLaes != SIP_NULL)
			{
				sip_dcs_freeSipDcsLaesHeader(glbSipParserDcsLaes);
				glbSipParserDcsLaes = SIP_NULL;
			}
			if(glbSipParserAddrSpec != SIP_NULL)
			{
				sip_freeSipAddrSpec(glbSipParserAddrSpec);
				glbSipParserAddrSpec = SIP_NULL;
			}
			if(glbSipParserDcsSipParam != SIP_NULL)
			{
				sip_freeSipParam(glbSipParserDcsSipParam);
				glbSipParserDcsSipParam = SIP_NULL;
			}
			if(glbSipTempStr != SIP_NULL)
			{
				fast_memfree(BISON_MEM_ID,glbSipTempStr,SIP_NULL);
				glbSipTempStr = SIP_NULL;
			}
			/*
				if(glbSipParserSession != SIP_NULL)
				fast_memfree(BISON_MEM_ID,glbSipParserSession,SIP_NULL);
			*/	
			if(glbSipParserDcsState!= SIP_NULL)
			{
				sip_dcs_freeSipDcsStateHeader(glbSipParserDcsState);
				glbSipParserDcsState = SIP_NULL;
			} 
			if(glbSipParserAcctEntry!= SIP_NULL)
			{
				sip_dcs_freeSipDcsAcctEntry(glbSipParserAcctEntry);
				glbSipParserAcctEntry= SIP_NULL;
			} 
			if(glbSipParserDcsAnonymity!= SIP_NULL)
			{
				sip_dcs_freeSipDcsAnonymityHeader(glbSipParserDcsAnonymity);
				glbSipParserDcsAnonymity= SIP_NULL;
			} 
			if(glbSipParserDcsRpidPrivacy!= SIP_NULL)
			{
				sip_dcs_freeSipDcsRpidPrivacyHeader(glbSipParserDcsRpidPrivacy);
				glbSipParserDcsRpidPrivacy= SIP_NULL;
			} 
       		sip_listSizeOf(&glbslPrivacyParam,&sizePrivacyParam,&dErr);
        	if(sizePrivacyParam !=0)
        	{
                sip_listDeleteAll(&glbslPrivacyParam,&dErr);
                glbslPrivacyParam.size=0;
			}
			if(*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
				==E_NO_ERROR)
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)=\
				E_PARSER_ERROR;


		 };

dcsheader:dcsmediaauth
		|dcsanonymity
		|dcsrpidprivacy
		|dcsosps
		|dcslaes
		|dcssession
		|dcstracepartyid
		|dcsgate
		|dcsredirect
		|dcsstate
		|dcsremotepartyid
		|dcsbillingid
		|dcsbillinginfo;

dcsrpidprivacy: dcsrpidprivacycolon rpidprivacyparams;
dcsrpidprivacycolon: DCSRPIDPRIVACY
	{
		if(sip_listInit(&glbslPrivacyParam,&__sip_freeSipParam \
			,(((SipHeaderParserParam*)pHeaderParserParam)->pError))!=\
			SipSuccess)
			YYABORT;	
	};

rpidprivacyparams: rpidprivacyparam
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
			
		order->dType = SipHdrTypeDcsRpidPrivacy;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	| rpidprivacyparams COMMA rpidprivacyparam
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
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
		YYABORT;
		order->dNum++;
	}
	| rpidprivacyparams COMMA;
	
rpidprivacyparam: nosemprivacygenparams privacygenparams
	{
		
		if(sip_dcs_initSipDcsRpidPrivacyHeader( \
			&glbSipParserDcsRpidPrivacy, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
				
		glbSipParserDcsRpidPrivacy->slParams=glbslPrivacyParam;	
		
		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr-> \
			slDcsRpidPrivacyHdr), glbSipParserDcsRpidPrivacy,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;

		if(sip_listInit(&glbslPrivacyParam,&__sip_freeSipParam \
			,(((SipHeaderParserParam*)pHeaderParserParam)->pError))!=\
			SipSuccess)
			YYABORT;	
	
		glbSipParserDcsRpidPrivacy=SIP_NULL;

	};

nosemprivacygenparams: token
	{
		SipParam *pParam;
		SIP_U32bit index;

		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *)\
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
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&glbslPrivacyParam,\
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			== SipFail)
			YYABORT;
	}
	|TOKENEQUALS token
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		SIP_U32bit index;
		
		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		index=0;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;
		$1.dLength -= 1;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($2);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) ==SipFail)
			YYABORT;

		if(sip_listAppend(&glbslPrivacyParam,\
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			== SipFail)
			YYABORT;

	}
	|TOKENEQUALS QSTRING
	{
		SipParam *pParam;
		SIP_S8bit *tempVal;
		SIP_U32bit index;
		
		if(sip_initSipParam(&pParam, &*(((SipHeaderParserParam *) \
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;

		index=0;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		$1.pToken = &($1.pToken[index]);
		$1.dLength -= index;
		$1.dLength -= 1;

		pParam->pName = sip_tokenBufferDup($1);
		if(pParam->pName == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($2);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) ==SipFail)
			YYABORT;

		if(sip_listAppend(&glbslPrivacyParam,\
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			== SipFail)
			YYABORT;

	};

dcsbillinginfo:dcsbillinginfocolon dcsbillinginfofields
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}	
		
		order->dType = SipHdrTypeDcsBillingInfo;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->\
			pDcsBillingInfoHdr = glbSipParserDcsBillingInfo;
		glbSipParserDcsBillingInfo = SIP_NULL;
	};
dcsbillinginfocolon:DCSBILLINGINFO
		{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pDcsBillingInfoHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor,\
				"There can only be one DCS Billing Info Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
		if(sip_dcs_initSipDcsBillingInfoHeader(&glbSipParserDcsBillingInfo,\
			(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) !=SipSuccess)
			YYABORT;
		};
		
dcsbillinginfofields: hostport 
		{
		SIP_U16bit * temp;
		SIP_S8bit *tempstr;
		tempstr=(SIP_S8bit *)fast_memget(BISON_MEM_ID,\
			(strlen(glbSipParserAddrSpec->u.pSipUrl->pHost)+1),\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		strcpy(tempstr,glbSipParserAddrSpec->u.pSipUrl->pHost);

		glbSipParserDcsBillingInfo->pHost= tempstr;
		if(glbSipParserAddrSpec->u.pSipUrl->dPort !=SIP_NULL)
		{
			temp=(SIP_U16bit *)fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit *),\
				(((SipHeaderParserParam *) pHeaderParserParam)->pError));
			*(temp)= *(glbSipParserAddrSpec->u.pSipUrl->dPort);
			glbSipParserDcsBillingInfo->pPort= temp;
		}
		else
		{
			glbSipParserDcsBillingInfo->pPort= SIP_NULL;
		}
		sip_freeSipAddrSpec(glbSipParserAddrSpec);
		glbSipParserAddrSpec = SIP_NULL;
		} 
		dcsacctentrys
		| dcsacctentrys;
		
		

dcsacctentrys : dcsacctentry COMMA dcsacctentrys
		| dcsacctentry;

dcsacctentry : 	LANGLE dcschargenum RANGLE FWSLASH LANGLE dcscallingnum RANGLE FWSLASH LANGLE dcscallednum RANGLE FWSLASH LANGLE routingnum RANGLE FWSLASH LANGLE locroutingnum RANGLE
		{
		if(sip_dcs_initSipDcsAcctEntry(&glbSipParserAcctEntry,\
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;

		glbSipParserAcctEntry->pChargeNum = sip_tokenBufferDup($2);
		glbSipParserAcctEntry->pCallingNum = sip_tokenBufferDup($6);
		glbSipParserAcctEntry->pCalledNum = sip_tokenBufferDup($10);
		glbSipParserAcctEntry->pRoutingNum = sip_tokenBufferDup($14);
		glbSipParserAcctEntry->pLocationRoutingNum = sip_tokenBufferDup($18);

		if(sip_listAppend(&(glbSipParserDcsBillingInfo->slAcctEntry),\
			(SIP_Pvoid)glbSipParserAcctEntry ,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		glbSipParserAcctEntry =  SIP_NULL;
		}

		|LANGLE dcschargenum RANGLE FWSLASH LANGLE dcscallingnum RANGLE FWSLASH LANGLE dcscallednum RANGLE 
		{
		if(sip_dcs_initSipDcsAcctEntry(&glbSipParserAcctEntry,\
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;

		glbSipParserAcctEntry->pChargeNum = sip_tokenBufferDup($2);
		glbSipParserAcctEntry->pCallingNum = sip_tokenBufferDup($6);
		glbSipParserAcctEntry->pCalledNum = sip_tokenBufferDup($10);

		if(sip_listAppend(&(glbSipParserDcsBillingInfo->slAcctEntry),\
				(SIP_Pvoid)glbSipParserAcctEntry ,\
				(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
			YYABORT;
		glbSipParserAcctEntry =  SIP_NULL;
		};
		
dcschargenum: uric; 
dcscallingnum: uric; 
dcscallednum: uric; 
routingnum: uric; 
locroutingnum: uric; 
dcsbillingid:DCSBILLINGID hextoken FWSLASH hextoken 	
	{
   		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		if (((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pDcsBillingIdHdr != SIP_NULL)
		{
			sip_error(SIP_Minor, \
			"There can be only one Billing Id header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}
			
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}	
		
		order->dType=SipHdrTypeDcsBillingId;
		order->dNum=1;
		order->dTextType = SipFormFull;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		if(sip_dcs_initSipDcsBillingIdHeader( \
			&((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pDcsBillingIdHdr,(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pDcsBillingIdHdr->\
  			pId =  sip_tokenBufferDup($2);
		((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->pGeneralHdr->pDcsBillingIdHdr->\
  			pFEId =  sip_tokenBufferDup($4);
	};

/* dcs billing info */

dcsremotepartyid: dcsremotepartyidcolon dcsremotepartyidparams;

dcsremotepartyidparams: dcsremotepartyidfields
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
	
		order->dType = SipHdrTypeDcsRemotePartyId;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	| dcsremotepartyidparams COMMA dcsremotepartyidfields
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
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
		YYABORT;
		order->dNum++;
		
	}
	| dcsremotepartyidparams COMMA;
		
dcsremotepartyidcolon: DCSREMOTEPARTYID
	{
		if(sip_listInit(&glbslPrivacyParam,&__sip_freeSipParam \
			,(((SipHeaderParserParam*)pHeaderParserParam)->pError))!=\
			SipSuccess)
			YYABORT;	
	};
dcsremotepartyidfields: displayname addrspec RANGLE privacygenparams
	{
		if(sip_dcs_initSipDcsRemotePartyIdHeader( \
			&glbSipParserDcsRemotePartyId, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
				
		if ($1.pToken != SIP_NULL)
		{
			glbSipParserDcsRemotePartyId->pDispName = \
			sip_tokenBufferDup($1);
			if(glbSipParserDcsRemotePartyId->pDispName ==SIP_NULL)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_MEM;
				YYABORT;
			}
		}
		else
			glbSipParserDcsRemotePartyId->pDispName = SIP_NULL;

		glbSipParserDcsRemotePartyId->pAddrSpec = glbSipParserAddrSpec;
		
		glbSipParserDcsRemotePartyId->slParams=glbslPrivacyParam;	

		if(sip_listAppend(&(((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr-> \
			slDcsRemotePartyIdHdr), glbSipParserDcsRemotePartyId,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)-> \
			pError))!=SipSuccess)
			YYABORT;
			
		if(sip_listInit(&glbslPrivacyParam,&__sip_freeSipParam \
			,(((SipHeaderParserParam*)pHeaderParserParam)->pError))!=\
			SipSuccess)
			YYABORT;	

		glbSipParserAddrSpec = SIP_NULL;
		glbSipParserDcsRemotePartyId = SIP_NULL;	
	};

privacygenparams: privacygenparams privacygenparam 
			|;

privacygenparam: semtoken
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
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&glbslPrivacyParam,\
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			== SipFail)
			YYABORT;
		}
	
	|semtoken EQUALS QSTRING
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
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) ==SipFail)
			YYABORT;

		if(sip_listAppend(&glbslPrivacyParam,\
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			== SipFail)
			YYABORT;

	}
	|semtoken EQUALS token 
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
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) \
			= E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) ==SipFail)
			YYABORT;

		if(sip_listAppend(&glbslPrivacyParam,\
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			== SipFail)
			YYABORT;

	};

/* dcs state */
dcsstate: dcsstatecolon dcsstatefields;
dcsstatecolon :	DCSSTATE
		{
			SetHostFlag=1;
		};
dcsstatefields: dcsstatefield
		{
		SipHeaderOrderInfo *order;
		SipBool	dResult;

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeDcsState;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->\
			slDcsStateHdr, (SIP_Pvoid)glbSipParserDcsState,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		glbSipParserDcsState = SIP_NULL;
		}
		 | dcsstatefields COMMA dcsstatefield
		{
		SipHeaderOrderInfo *order;
		SIP_U32bit ordersize;
		if(sip_listSizeOf(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,&ordersize,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if(sip_listGetAt(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		order->dNum++;
		if(sip_listAppend(&((SipHeaderParserParam *) \
			pHeaderParserParam)->pSipMessage->pGeneralHdr->\
			slDcsStateHdr,(SIP_Pvoid)glbSipParserDcsState,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) !=SipSuccess)
			YYABORT;		
		glbSipParserDcsState = SIP_NULL;
		};

dcsstatefield: hostport
		{
		if(sip_dcs_initSipDcsStateHeader(&glbSipParserDcsState,\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError)) !=SipSuccess)
			YYABORT;
			
		glbSipTempStr =(SIP_S8bit *)fast_memget(BISON_MEM_ID,\
			(strlen(glbSipParserAddrSpec->u.pSipUrl->pHost)\
			+1) ,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		strcpy(glbSipTempStr,glbSipParserAddrSpec->u.pSipUrl->pHost);
		glbSipParserDcsState->pHost = glbSipTempStr;
		glbSipTempStr = SIP_NULL;
		SetHostFlag = 0;
		sip_freeSipAddrSpec(glbSipParserAddrSpec);
		glbSipParserAddrSpec = SIP_NULL;
		}  dcsstategenparams;

dcsstategenparams: dcsstategenparam
		| dcsstategenparams dcsstategenparam;

dcsstategenparam:semtoken
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
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(glbSipParserDcsState->slParams), \
			pParam,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		}
	
		|semtoken EQUALS alltoken
		{
		SipParam *pParam;
		SIP_S8bit *tempVal;
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
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		tempVal = sip_tokenBufferDup($3);
		if(tempVal == SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend(&(pParam->slValue), tempVal, \
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))==SipFail)
			YYABORT;

		if(sip_listAppend(&(glbSipParserDcsState->slParams), \
			pParam, &*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		};

/* dcs redirect */
dcsredirect:  dcsredirectcolon calledid redirector digits
	{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		SIP_S8bit *temp;

		temp = sip_tokenBufferDup($4);
		glbSipParserDcsRedirect->dNum = STRTOU32CAP(temp,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		fast_memfree(BISON_MEM_ID,temp,SIP_NULL);

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeDcsRedirect;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		
		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage ->\
			u.pRequest-> pRequestHdr->pDcsRedirectHdr = glbSipParserDcsRedirect;
		glbSipParserDcsRedirect = SIP_NULL;

	};
		
dcsredirectcolon : DCSREDIRECT
	{

		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
				E_NO_ERROR;
				YYABORT;
		}
		if (((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			u.pRequest->pRequestHdr->pDcsRedirectHdr != SIP_NULL)
		{
			sip_error(SIP_Minor, "There can be only one Dcs Redirect\
					 header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}


		if(sip_dcs_initSipDcsRedirectHeader(&glbSipParserDcsRedirect,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		SetSipSColon =1;			
		}; 

calledid: LANGLE sipurl RANGLE
		{
		glbSipParserDcsRedirect->pCalledId =\
		glbSipParserAddrSpec->u.pSipUrl;
		glbSipParserAddrSpec->u.pSipUrl = SIP_NULL;
		fast_memfree(BISON_MEM_ID,glbSipParserAddrSpec,SIP_NULL);
		glbSipParserAddrSpec = SIP_NULL;

		};
redirector:  LANGLE sipurl RANGLE
		{
			glbSipParserDcsRedirect->pRedirector\
			= glbSipParserAddrSpec->u.pSipUrl;
			glbSipParserAddrSpec->u.pSipUrl = SIP_NULL;
			fast_memfree(BISON_MEM_ID,glbSipParserAddrSpec,SIP_NULL);
			glbSipParserAddrSpec = SIP_NULL;
		};

/* dcs gate */
dcsgate : dcsgatecolon dcsgatefields
{
	SipHeaderOrderInfo *order;
	SipBool	dResult;

	dResult=sip_initSipHeaderOrderInfo(&order,\
		((SipHeaderParserParam *)pHeaderParserParam)->pError);
	if (dResult!=SipSuccess)
	{
		YYABORT;
	}
	
	order->dType = SipHdrTypeDcsGate;
	order->dTextType = SipFormFull;
	order->dNum = 1;

	if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)->\
		pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
		(((SipHeaderParserParam *) pHeaderParserParam)->\
		pError))!=SipSuccess)
		YYABORT;

	((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage ->pGeneralHdr->pDcsGateHdr=\
			glbSipParserDcsGate;
	glbSipParserDcsGate = SIP_NULL;
};

dcsgatecolon: DCSGATE
	{
			
		if (((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pDcsGateHdr != SIP_NULL)
		{
			sip_error(SIP_Minor, "There can be only one Dcs-Gate header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}

		if(sip_dcs_initSipDcsGateHeader(&glbSipParserDcsGate,\
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
	};
	

dcsgatefields: hostport slash dcsgategenparams 
	{
		SIP_U16bit * temp;
		SIP_S8bit *tempstr;
		tempstr=(SIP_S8bit *)fast_memget(BISON_MEM_ID,\
			(strlen(glbSipParserAddrSpec->u.pSipUrl->pHost)\
			+1) ,&*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		strcpy(tempstr,glbSipParserAddrSpec->u.pSipUrl->pHost);
	
		glbSipParserDcsGate->pHost= tempstr;
		if(glbSipParserAddrSpec->u.pSipUrl->dPort !=SIP_NULL)
		{
		temp=(SIP_U16bit *)fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit *),\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		*(temp)= *(glbSipParserAddrSpec->u.pSipUrl->dPort);
		glbSipParserDcsGate->pPort= temp;
		}
		else
		{
			glbSipParserDcsGate->pPort= SIP_NULL;
		}
		sip_freeSipAddrSpec(glbSipParserAddrSpec);
		glbSipParserAddrSpec = SIP_NULL;
	};
dcsgategenparams:gateid
		{
		glbSipParserDcsGate->pId=sip_tokenBufferDup($1);
		}
		|gateid dcsgategenparam
		{
			glbSipParserDcsGate->pId=sip_tokenBufferDup($1);
		};
gateid: alphanum;
dcsgategenparam: dcsgateoneparam 
				| dcsgatetwoparam
				| dcsgateoneparam dcsgatetwoparam;
				
dcsgateoneparam : gatekey gatecipher;

dcsgatetwoparam: token
		{
		glbSipParserDcsGate->pStrength=sip_tokenBufferDup($1);
		};

gatekey: SEMALPHANUM   	
	{
		int index;
		SIP_S8bit *pName;
	/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, $1.dLength-\
		index+1, &*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(pName==SIP_NULL)
			YYABORT;
		strncpy(pName,&($1.pToken[index]), $1.dLength-index);
		pName[$1.dLength-index] = '\0';
		glbSipParserDcsGate->pKey=pName;	
	};	
gatecipher:semtoken    	
	{
		int index;
		SIP_S8bit *pName;
	/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, $1.dLength-\
		index+1, &*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(pName==SIP_NULL)
			YYABORT;
		strncpy(pName,&($1.pToken[index]), $1.dLength-index);
		pName[$1.dLength-index] = '\0';
		glbSipParserDcsGate->pCipherSuite=pName;	
	};	

/* dcs trace party id */
dcstracepartyid:  dcstracepartyidcolon LANGLE 
		{
			if (((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->u.pRequest->pRequestHdr->\
				pDcsTracePartyIdHdr != SIP_NULL)
			{
				sip_error(SIP_Minor, "There can be only one Trace-Party-Id\
						 header");
				*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_PARSER_ERROR;
				YYABORT;
			}
			

	} addrspec RANGLE 
		{
			SipHeaderOrderInfo *order;
			SipBool	dResult;

			if(sip_dcs_initSipDcsTracePartyIdHeader\
			(&((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest-> pRequestHdr->\
			pDcsTracePartyIdHdr,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) !=SipSuccess)
				YYABORT;

			((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pRequest->pRequestHdr->\
			pDcsTracePartyIdHdr->pAddrSpec = glbSipParserAddrSpec;

			glbSipParserAddrSpec = SIP_NULL;
			dResult=sip_initSipHeaderOrderInfo(&order,\
				((SipHeaderParserParam *)pHeaderParserParam)->pError);
			if (dResult!=SipSuccess)
			{
				YYABORT;
			}
			
			order->dType = SipHdrTypeDcsTracePartyId;
			order->dTextType = SipFormFull;
			order->dNum = 1;

			if(sip_listAppend(&((SipHeaderParserParam *)pHeaderParserParam)-> \
				pSipMessage->slOrderInfo, (SIP_Pvoid)order, \
				((((SipHeaderParserParam *)pHeaderParserParam)-> \
				pError)))!=SipSuccess)
				YYABORT;

		};

/* Session */
dcssession:dcssessioncolon sessiontags;
dcssessioncolon:DCSSESSION   	
	{
			if(((SipHeaderParserParam *) pHeaderParserParam)->\
				pSipMessage->dType!=SipMessageResponse)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->\
					pError) = E_NO_ERROR;
					YYABORT;
			}
	};
sessiontags:sessiontags COMMA sessiontag 
			{
			SipHeaderOrderInfo *order;
			SIP_U32bit ordersize;
			if(sip_listSizeOf(&((SipHeaderParserParam *)\
				pHeaderParserParam)->pSipMessage->slOrderInfo, \
				&ordersize,&*(((SipHeaderParserParam *) \
				pHeaderParserParam)->pError)) == SipFail)
				YYABORT;
			if(sip_listGetAt(&((SipHeaderParserParam *) \
				pHeaderParserParam)->pSipMessage->slOrderInfo,ordersize-1, \
				(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
			order->dNum++;
			}
			|sessiontag
			{
				SipHeaderOrderInfo *order;
				SipBool	dResult;

				dResult=sip_initSipHeaderOrderInfo(&order,\
					((SipHeaderParserParam *)pHeaderParserParam)->pError);
				if (dResult!=SipSuccess)
				{
					YYABORT;
				}			
				
				order->dType = SipHdrTypeSession;
				order->dTextType = SipFormFull;
				order->dNum = 1;
				if(sip_listAppend(&((SipHeaderParserParam *)\
					pHeaderParserParam)->pSipMessage->slOrderInfo, \
					(SIP_Pvoid)order, &*(((SipHeaderParserParam *) \
					pHeaderParserParam)->pError))!=SipSuccess) YYABORT;
			};

sessiontag: token 
		{
		if(sip_dcs_initSipSessionHeader(&glbSipParserSession, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;
		glbSipParserSession->pTag = sip_tokenBufferDup($1);
		if(glbSipParserSession->pTag==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
		if(sip_listAppend( &((SipHeaderParserParam *) pHeaderParserParam)->\
			pSipMessage->u.pResponse->pResponseHdr->slSessionHdr,\
			glbSipParserSession,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		};
	


/*Starting of DCS LAES*/
dcslaes:dcslaescolon  laessigparam laeskey 
{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
		
		order->dType = SipHdrTypeDcsLaes;
		order->dTextType = SipFormFull;
		order->dNum = 1;

		if(sip_listAppend(&((SipHeaderParserParam *) pHeaderParserParam)\
			->pSipMessage->slOrderInfo,(SIP_Pvoid)order, \
			(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->pDcsLaesHdr =\
			glbSipParserDcsLaes;
		glbSipParserDcsLaes = SIP_NULL;
};

dcslaescolon :DCSLAES 	
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			pGeneralHdr->pDcsLaesHdr!=SIP_NULL)
		{
			sip_error (SIP_Minor, "There can only be one DCS \
			LAES Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}


		if(sip_dcs_initSipDcsLaesHeader(&glbSipParserDcsLaes,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
			YYABORT;

	};			

laessigparam: laessig
		| laessig COMMA laescontent;	 
laessig:hostport /*hostport comeback here ***/
	{
		SIP_U16bit * temp;
		SIP_S8bit *tempstr;
		tempstr=(SIP_S8bit *)fast_memget(BISON_MEM_ID,\
		(strlen(glbSipParserAddrSpec->u.pSipUrl->pHost)\
		+1) ,&*(((SipHeaderParserParam *) pHeaderParserParam)->\
					pError));
		strcpy(tempstr,glbSipParserAddrSpec->u.pSipUrl->pHost);
	
		glbSipParserDcsLaes->pSignatureHost= tempstr;

		if(glbSipParserAddrSpec->u.pSipUrl->dPort != SIP_NULL)
		{
		temp=(SIP_U16bit *)fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit *),\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		*(temp) = *(glbSipParserAddrSpec->u.pSipUrl->dPort);

		glbSipParserDcsLaes->pSignaturePort= temp;
		}
		else
		{
			glbSipParserDcsLaes->pSignaturePort= SIP_NULL;
		}
		sip_freeSipAddrSpec(glbSipParserAddrSpec);
		glbSipParserAddrSpec = SIP_NULL;
	};	
laescontent: hostport 
	{
		SIP_U16bit * temp;
		SIP_S8bit *tempstr;
		tempstr=(SIP_S8bit *)fast_memget(BISON_MEM_ID,\
		(strlen(glbSipParserAddrSpec->u.pSipUrl->pHost)	+1) ,\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		strcpy(tempstr,glbSipParserAddrSpec->u.pSipUrl->pHost);
		glbSipParserDcsLaes->pContentHost= tempstr;


		if(glbSipParserAddrSpec->u.pSipUrl->dPort != SIP_NULL)
		{
		temp=(SIP_U16bit *)fast_memget(BISON_MEM_ID,sizeof(SIP_U16bit *),\
		(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		*(temp) = *(glbSipParserAddrSpec->u.pSipUrl->dPort);
		glbSipParserDcsLaes->pContentPort= temp;
		}
		else
		{
			glbSipParserDcsLaes->pContentPort= SIP_NULL;
			
		}
		sip_freeSipAddrSpec(glbSipParserAddrSpec);
		glbSipParserAddrSpec = SIP_NULL;
	};	

laeskey:semtoken    	
	{
		int index;
		SIP_S8bit *pName;
	/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID, $1.dLength-\
		index+1, &*(((SipHeaderParserParam *) pHeaderParserParam)->pError));
		if(pName==SIP_NULL)
			YYABORT;
		strncpy(pName,&($1.pToken[index]), $1.dLength-index);
		pName[$1.dLength-index] = '\0';
		glbSipParserDcsLaes->pKey=pName;	
	};	


		
	
/* dcs osps  */
dcsosps: dcsospscolon token
		{
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		if (((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			u.pRequest->pRequestHdr->pDcsOspsHdr != SIP_NULL)
		{
			sip_error(SIP_Minor, \
				"There can be only one OSPS header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
			= E_PARSER_ERROR;
			YYABORT;
		}

		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}

		order->dType=SipHdrTypeDcsOsps;
		order->dNum=1;
		order->dTextType = SipFormFull;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
		(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		if(sip_dcs_initSipDcsOspsHeader( &((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->u.pRequest->\
		pRequestHdr->pDcsOspsHdr, &*(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
		u.pRequest-> pRequestHdr->pDcsOspsHdr->pTag =  sip_tokenBufferDup($2);

		};	 
 
/* dcs anonymity */
dcsanonymity:dcsanonymitycolon privacytags;
dcsanonymitycolon: DCSANONYMITY; 	
privacytags: privacytag COMMA privacytags  
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
				(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
				pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
			order->dNum++;
			}

			| privacytag

			{
				SipHeaderOrderInfo *order;
				SipBool	dResult;

				dResult=sip_initSipHeaderOrderInfo(&order,\
					((SipHeaderParserParam *)pHeaderParserParam)->pError);
				if (dResult!=SipSuccess)
				{
					YYABORT;
				}					
				order->dType = SipHdrTypeDcsAnonymity;
				order->dTextType = SipFormFull;
				order->dNum = 1;
				if(sip_listAppend(&((SipHeaderParserParam *)\
					pHeaderParserParam)->pSipMessage->slOrderInfo, \
					(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
					pHeaderParserParam)->pError))!=SipSuccess)
					YYABORT;

			};
privacytag: token
		{
			if(sip_dcs_initSipDcsAnonymityHeader( \
			&glbSipParserDcsAnonymity, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
			glbSipParserDcsAnonymity->pTag = sip_tokenBufferDup($1);
			if(glbSipParserDcsAnonymity->pTag==SIP_NULL)
			{
				*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
				YYABORT;
			}
			if(sip_listAppend(&(((SipHeaderParserParam *) pHeaderParserParam)\
				->pSipMessage->pGeneralHdr->slDcsAnonymityHdr),\
					glbSipParserDcsAnonymity,&*(((SipHeaderParserParam *)\
						pHeaderParserParam)->pError))!=SipSuccess)
				YYABORT;
			glbSipParserDcsAnonymity=SIP_NULL;
		};


/*  media auth */
dcsmediaauth: dcsmediacolon dcsmediaparams;
dcsmediacolon: DCSMEDIAAUTH;
dcsmediaparams: dcsmediaparam
	{
      	SipHeaderOrderInfo *order;
		SipBool	dResult;
		
		dResult=sip_initSipHeaderOrderInfo(&order,\
			((SipHeaderParserParam *)pHeaderParserParam)->pError);
		if (dResult!=SipSuccess)
		{
			YYABORT;
		}
			
		order->dType=SipHdrTypeDcsMediaAuthorization;
		order->dNum=1;
		order->dTextType = SipFormFull;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->slOrderInfo, \
			(SIP_Pvoid)order, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		
	}
	|dcsmediaparams COMMA dcsmediaparam
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
			(SIP_Pvoid *)&order,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError)) == SipFail)
		YYABORT;

		order->dNum++;
	}
	|dcsmediaparams COMMA;
dcsmediaparam: hextoken
	{
		SipDcsMediaAuthorizationHeader *pMediaAuth = SIP_NULL;
		if(sip_dcs_initSipDcsMediaAuthorizationHeader( \
			&pMediaAuth,
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
			YYABORT;
		if(sip_listAppend(&((SipHeaderParserParam *)\
			pHeaderParserParam)->pSipMessage->pGeneralHdr->\
			slDcsMediaAuthorizationHdr,\
			(SIP_Pvoid)pMediaAuth, &*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		pMediaAuth->pAuth =  sip_tokenBufferDup($1);
	};
/*general */

addrspec:sipurl
		|uricval;

sipurl :sipcolon  userinfoat hostport urlparams headers
		|sipcolon  hostport urlparams headers;
		
displayname: DISPNAME 
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
		}
		| alphanum1 LANGLE
		| TOKEN LANGLE
		| DISPLAYNAME LANGLE
		| LANGLE
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


hostport: host COLON port
		{
		SIP_S8bit *temp;

		if(SetHostFlag ==1)
		{
			sip_error(SIP_Minor, \
			"There can only be host in DCS-State header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
			E_PARSER_ERROR;
			SetHostFlag = 0;
				YYABORT;
		}

		if (glbSipParserAddrSpec == SIP_NULL)
			if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrSipUri, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))\
			!=SipSuccess)
				YYABORT;

		if (glbSipParserAddrSpec->u.pSipUrl == SIP_NULL)
			if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;

		glbSipParserAddrSpec->u.pSipUrl->pHost = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHost==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
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
			if (glbSipParserAddrSpec == SIP_NULL)
				if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrSipUri, \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
					YYABORT;

			if (glbSipParserAddrSpec->u.pSipUrl == SIP_NULL)
				if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
				&*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError))!=SipSuccess)
					YYABORT;
			glbSipParserAddrSpec->u.pSipUrl->pHost = sip_tokenBufferDup($1);
			if(glbSipParserAddrSpec->u.pSipUrl->pHost==SIP_NULL)
			{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
			}
		};
	
		
host:	token
		|ipv6;

ipv6:IPV6ADDR;

slash: FWSLASH;

		
alltoken: token
		| QSTRING;
newalltoken:recalltoken newalltoken
		{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,\
		$1.dLength+$2.dLength,&*(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *)\
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
		}
			|recalltoken
			|QSTRING;

			
recalltoken:SEMPARAMVALUE
			|TOKEN
			|ALPHANUM
			|DIGITS
			|HEXTOKEN
			|FWSLASH
			|IPV6ADDR;
digits : DIGITS;

sipcolon:	SIPCOLON
	{
		if (glbSipParserAddrSpec != SIP_NULL)
		{
			sip_freeSipAddrSpec(glbSipParserAddrSpec);
			glbSipParserAddrSpec = SIP_NULL;
		}
		if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrSipUri, \
		&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		if (glbSipParserAddrSpec->u.pSipUrl != SIP_NULL)
			sip_freeSipUrl(glbSipParserAddrSpec->u.pSipUrl);
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
		&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	}
	| SIPSCOLON
	{
		if(SetSipSColon==1)
		{
			sip_error(SIP_Minor, \
			"SIPS is not allowed in the Dcs-Redirect Header");
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) =\
			E_PARSER_ERROR;
			if(glbSipParserDcsRedirect != SIP_NULL)
			{
				sip_dcs_freeSipDcsRedirectHeader(glbSipParserDcsRedirect);
				glbSipParserDcsRedirect = SIP_NULL;
			}
			SetSipSColon= 0;
			YYABORT;
		}
		
		if (glbSipParserAddrSpec != SIP_NULL)
		{
			sip_freeSipAddrSpec(glbSipParserAddrSpec);
			glbSipParserAddrSpec = SIP_NULL;
		}
		if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrSipSUri, \
		&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;

		if (glbSipParserAddrSpec->u.pSipUrl != SIP_NULL)
			sip_freeSipUrl(glbSipParserAddrSpec->u.pSipUrl);
		if(sip_initSipUrl(&glbSipParserAddrSpec->u.pSipUrl , \
		&*(((SipHeaderParserParam *) pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};


port:	digits;

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
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID,\
		$1.dLength-index+1,(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		yyuparam=sip_tokenBufferDup($3);	
		if(sip_listAppend(&param->slValue,(SIP_Pvoid)yyuparam,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
		YYABORT;

		if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
			(SIP_Pvoid)param,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	
	}
	|semparamtokens
 	{
		SipParam *param; 
		int index;		
 
		if(sip_initSipParam(& param,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
		/* Ignore initial ';' and whitespace before parameter name */
		index=1;
		while(($1.pToken[index]==' ')||($1.pToken[index]=='\t'))
			index++;
		param->pName = (SIP_S8bit *) fast_memget(BISON_MEM_ID,\
		$1.dLength-index+1,(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError));
		if(param->pName==SIP_NULL)
			YYABORT;
		strncpy(param->pName,&($1.pToken[index]), $1.dLength-index);
		param->pName[$1.dLength-index] = '\0';

		if(sip_listAppend(&glbSipParserAddrSpec->u.pSipUrl->slParam, \
			(SIP_Pvoid)param,&*(((SipHeaderParserParam *)\
			pHeaderParserParam)->pError))!=SipSuccess)
			YYABORT;
	};


headers:	QUESTION 
	{
		glbSipParserDcsAllowSipColon=1;
	}
	uricdummy
	{
		glbSipParserDcsAllowSipColon=0;
	}
	|
	{
		glbSipParserDcsAllowSipColon=0;
	};

uricdummy:	uric
	{
		glbSipParserAddrSpec->u.pSipUrl->pHeader = sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pSipUrl->pHeader==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
	};	
uricval:uric
{
	 if(glbSipParserAddrSpec == SIP_NULL)
			if(sip_initSipAddrSpec(& glbSipParserAddrSpec,SipAddrReqUri, \
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError))!=SipSuccess)
				YYABORT;

		glbSipParserAddrSpec->u.pUri = (SIP_S8bit *) sip_tokenBufferDup($1);
		if(glbSipParserAddrSpec->u.pUri==SIP_NULL)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError) = E_NO_MEM;
			YYABORT;
		}
};
	


uric:	uric uricnocolon
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,\
		$1.dLength+$2.dLength,&*(((SipHeaderParserParam *)\
		pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *)\
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			&*(((SipHeaderParserParam *) pHeaderParserParam)->\
			pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
	|uric COLON	
	{
		$$.pToken=(SIP_S8bit *) fast_memget(BISON_MEM_ID,\
		$1.dLength+$2.dLength,&*(((SipHeaderParserParam *) \
		pHeaderParserParam)->pError));
		if(sip_listAppend(((SipHeaderParserParam *)\
			pHeaderParserParam)->pGCList, (SIP_Pvoid) $$.pToken,\
			(((SipHeaderParserParam *) pHeaderParserParam)->pError)) == SipFail)
			YYABORT;
		if($$.pToken == SIP_NULL)
			YYABORT;
		strncpy($$.pToken, $1.pToken, $1.dLength);
		strncpy(&($$.pToken[$1.dLength]), $2.pToken, $2.dLength);
		$$.dLength = $1.dLength+$2.dLength;
	} 
	|uricnocolon 
	|COLON;
		
uricnocolon:token
			|URICNOCOLON	
			|USERINFOAT		
			|EQUALS 			
			|QUESTION
			|semtoken
			|SEMICOLON
			|FWSLASH
			|COMMA
			|QSTRING1
			|MUNRESERVED	
			|IPV6ADDR	
			|TOKENEQUALS
			|SEMPARAMVALUE
			|PLAINSIPCOLON;

alphanum1:alphanum
         | alphanum1 alphanum 
				 ; 
alphanum: ALPHANUM
		| DIGITS
		| HEXTOKEN;

semtoken : SEMTOKEN
		| SEMALPHANUM;
semparamtokens : SEMTOKEN
			| SEMALPHANUM
			|SEMPARAMTOKEN;
		
token:  TOKEN
		|ALPHANUM
		|DIGITS
		|HEXTOKEN;
hextoken : HEXTOKEN
		| DIGITS;
dcstracepartyidcolon:  DCSTRACEPARTYID
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->\
				pError) = E_NO_ERROR;
				YYABORT;
		}
	};
dcsospscolon: DCSOSPS
	{
		if(((SipHeaderParserParam *) pHeaderParserParam)->pSipMessage->\
			dType!=SipMessageRequest)
		{
			*(((SipHeaderParserParam *) pHeaderParserParam)->pError)\
				= E_NO_ERROR;
			YYABORT;
		}
	};
