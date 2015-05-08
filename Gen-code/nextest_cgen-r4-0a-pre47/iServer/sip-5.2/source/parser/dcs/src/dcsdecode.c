/***********************************************************************
 ** FUNCTION:
 **    This file contains definitions of functions called from 
 **     sipdecode.c
 *********************************************************************
 **
 ** FILENAME:
 ** sipdecode.c
 **
 ** DESCRIPTION:
 **  THIS FILE IS USED INTERNALLY BY THE STACK
 **	  IT SHOULD NOT BE INCLUDED DIRECTLY BY THE USER
 **
 **	 DATE       	 NAME               REFERENCE               REASON
 **	 ----      		  ----              ---------              --------
 **  4Dec2000	  T.Seshashayi			Creation
 **
 **
 **     Copyright 2000, Hughes Software Systems, Ltd.
 *********************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif 

#include <stdlib.h>
#include <ctype.h>
#include "siplist.h"
#include "dcsdecode.h"
#include "sipdecode.h"
#include "portlayer.h"
#include "dcsfree.h"
#include "siptrace.h"
#include "siptimer.h"
#include "sipstatistics.h"
#include "sipparserclone.h"


SipBool sip_dcs_getTypeFromName_dstart 
#ifdef ANSI_PROTO
(SIP_S8bit *pName, en_HeaderType *pType,\
		SipError *pError)
#else
(pName,pType,pError)
SIP_S8bit *pName;
en_HeaderType *pType;
SipError *pError;
#endif
{
		SipError *dummy; 
		dummy = pError;

		if(strcasecmp("Dcs-Osps:",pName)==0)
		{
			*pType = SipHdrTypeDcsOsps;
			return SipSuccess;
		}
		else if(strcasecmp("Dcs-Trace-Party-Id:",pName)==0)
		{
			*pType = SipHdrTypeDcsTracePartyId;
			return SipSuccess;
		}
		else if(strcasecmp("Dcs-Redirect:",pName)==0)
		{
			*pType = SipHdrTypeDcsRedirect;
			return SipSuccess;
		}
		else  if(strcasecmp("Dcs-Laes:",pName)==0)
		{
			*pType = SipHdrTypeDcsLaes;
			return SipSuccess;
		}
		else  if(strcasecmp("Dcs-Gate:",pName)==0)
		{
			*pType = SipHdrTypeDcsGate;
			return SipSuccess;
		}
		else  if(strcasecmp("Dcs-Billing-Id:",pName)==0)
		{
			*pType = SipHdrTypeDcsBillingId;
			return SipSuccess;
		}
		else  if(strcasecmp("Dcs-Billing-Info:",pName)==0)
		{
			*pType = SipHdrTypeDcsBillingInfo;
			return SipSuccess;
		}
		return SipFail;
}

SipBool sip_dcs_getTypeFromName_astart 
#ifdef ANSI_PROTO
(SIP_S8bit *pName, en_HeaderType *pType,\
		SipError *pError)
#else
(pName,pType,pError)
SIP_S8bit *pName;
en_HeaderType *pType;
SipError *pError;
#endif
{
	SipError *dummy; 
	dummy = pError;
		
  if(strcasecmp("Anonymity:",pName)==0)
	{
		*pType = SipHdrTypeDcsAnonymity;
		return SipSuccess;
	}
	return SipFail;
}

SipBool sip_dcs_getTypeFromName_pstart 
#ifdef ANSI_PROTO
(SIP_S8bit *pName, en_HeaderType *pType,\
		SipError *pError)
#else
(pName,pType,pError)
SIP_S8bit *pName;
en_HeaderType *pType;
SipError *pError;
#endif
{
	SipError *dummy; 
	dummy = pError;
		
  if(strcasecmp("P-Media-Authorization:",pName)==0)
	{
		*pType = SipHdrTypeDcsMediaAuthorization;
		return SipSuccess;
	}
	return SipFail;
}


SipBool sip_dcs_getTypeFromName_rstart 
#ifdef ANSI_PROTO
(SIP_S8bit *pName, en_HeaderType *pType,\
		SipError *pError)
#else
(pName,pType,pError)
SIP_S8bit *pName;
en_HeaderType *pType;
SipError *pError;
#endif
{
	SipError *dummy; 
	dummy = pError;
		
	if(strcasecmp("Remote-Party-Id:",pName)==0)
	{
		*pType = SipHdrTypeDcsRemotePartyId;
		return SipSuccess;
	}
	else if(strcasecmp("RPID-Privacy:",pName)==0)
	{
		*pType = SipHdrTypeDcsRpidPrivacy;
		return SipSuccess;
	}

	return SipFail;
}

SipBool sip_dcs_getTypeFromName_sstart 
#ifdef ANSI_PROTO
(SIP_S8bit *pName, en_HeaderType *pType,\
		SipError *pError)
#else
(pName,pType,pError)
SIP_S8bit *pName;
en_HeaderType *pType;
SipError *pError;
#endif
{
	SipError *dummy; 
	dummy = pError;
		
  if(strcasecmp("Session:",pName)==0)
	{
		*pType = SipHdrTypeSession;
		return SipSuccess;
	}
	else  if(strcasecmp("State:",pName)==0)
	{
			*pType = SipHdrTypeDcsState;
			return SipSuccess;
	}
	return SipFail;
}

void sip_dcs_glbSipParserRemoveExistingHeaders
#ifdef ANSI_PROTO
 (SipUnknownHeader *yyhdr,\
	SipMessage *glbSipParserSipMessage,\
	SipError glbSipParserErrorValue)
#else
	(yyhdr,glbSipParserSipMessage,glbSipParserErrorValue)
	SipUnknownHeader *yyhdr;
	SipMessage *glbSipParserSipMessage;
	SipError glbSipParserErrorValue;
#endif
{
	if((strcasecmp("Dcs-Osps:",yyhdr->pName)==0))  
	{
		if(glbSipParserSipMessage->dType == SipMessageRequest)
		{
			if(glbSipParserSipMessage->u.pRequest->pRequestHdr->pDcsOspsHdr\
				!=SIP_NULL)
			{
				sip_dcs_freeSipDcsOspsHeader(glbSipParserSipMessage->u.pRequest->\
					pRequestHdr->pDcsOspsHdr);
				glbSipParserSipMessage->u.pRequest->pRequestHdr->pDcsOspsHdr \
					= SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsOsps,&glbSipParserErrorValue);
	}

	else if((strcasecmp("Dcs-Trace-Party-Id:",yyhdr->pName)==0))
	{		
		if(glbSipParserSipMessage->dType == SipMessageRequest)
		{
			if(glbSipParserSipMessage->u.pRequest->pRequestHdr->pDcsTracePartyIdHdr\
				!=SIP_NULL)
			{
				sip_dcs_freeSipDcsTracePartyIdHeader(glbSipParserSipMessage->u.pRequest->\
					pRequestHdr->pDcsTracePartyIdHdr);
				glbSipParserSipMessage->u.pRequest->pRequestHdr->pDcsTracePartyIdHdr \
					= SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsTracePartyId,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Dcs-Redirect:",yyhdr->pName)==0))
	{		
		if(glbSipParserSipMessage->dType == SipMessageRequest)
		{
			if(glbSipParserSipMessage->u.pRequest->pRequestHdr->pDcsRedirectHdr\
				!=SIP_NULL)
			{
				sip_dcs_freeSipDcsRedirectHeader(glbSipParserSipMessage->\
				u.pRequest-> pRequestHdr->pDcsRedirectHdr);

				glbSipParserSipMessage->u.pRequest->pRequestHdr->\
				pDcsRedirectHdr  = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsRedirect,&glbSipParserErrorValue);
	}
	else if((strcasecmp("P-Media-Authorization:",yyhdr->pName)==0))  
	{
		sip_listDeleteAll(&glbSipParserSipMessage->pGeneralHdr->\
			slDcsMediaAuthorizationHdr, &glbSipParserErrorValue);

		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsMediaAuthorization,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Dcs-Laes:",yyhdr->pName)==0))  
	{
		if(glbSipParserSipMessage->pGeneralHdr->pDcsLaesHdr!=\
			SIP_NULL)
		{
			sip_dcs_freeSipDcsLaesHeader(glbSipParserSipMessage->\
				pGeneralHdr->pDcsLaesHdr);
			glbSipParserSipMessage->pGeneralHdr->pDcsLaesHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsLaes,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Dcs-Gate:",yyhdr->pName)==0))  
	{
		if(glbSipParserSipMessage->pGeneralHdr->pDcsGateHdr!=\
			SIP_NULL)
		{
			sip_dcs_freeSipDcsGateHeader(glbSipParserSipMessage->\
				pGeneralHdr->pDcsGateHdr);
			glbSipParserSipMessage->pGeneralHdr->pDcsGateHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsGate,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Remote-Party-Id:",yyhdr->pName)==0))  
	{
		sip_listDeleteAll(&glbSipParserSipMessage->pGeneralHdr->\
		slDcsRemotePartyIdHdr, &glbSipParserErrorValue);

		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsRemotePartyId,&glbSipParserErrorValue);
	}
	else if((strcasecmp("RPID-Privacy:",yyhdr->pName)==0))  
	{
		sip_listDeleteAll(&glbSipParserSipMessage->pGeneralHdr->\
		slDcsRpidPrivacyHdr, &glbSipParserErrorValue);

		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsRpidPrivacy,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Anonymity:",yyhdr->pName)==0))  
	{
		sip_listDeleteAll(&glbSipParserSipMessage->pGeneralHdr->\
		slDcsAnonymityHdr, &glbSipParserErrorValue);

		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsAnonymity,&glbSipParserErrorValue);
	}
	else if((strcasecmp("State:",yyhdr->pName)==0))  
	{
		sip_listDeleteAll(&glbSipParserSipMessage->pGeneralHdr->\
		slDcsStateHdr, &glbSipParserErrorValue);

		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeDcsState,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Dcs-Billing-Id:",yyhdr->pName)==0))  
	{
		if(glbSipParserSipMessage->pGeneralHdr->pDcsBillingIdHdr!=\
		SIP_NULL)
		{
			sip_dcs_freeSipDcsBillingIdHeader(glbSipParserSipMessage->\
			pGeneralHdr->pDcsBillingIdHdr);
			glbSipParserSipMessage->pGeneralHdr->pDcsBillingIdHdr \
			= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
		SipHdrTypeDcsBillingId,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Dcs-Billing-Info:",yyhdr->pName)==0))  
	{
		if(glbSipParserSipMessage->pGeneralHdr->pDcsBillingInfoHdr!=\
		SIP_NULL)
		{
			sip_dcs_freeSipDcsBillingInfoHeader(glbSipParserSipMessage->\
			pGeneralHdr->pDcsBillingInfoHdr);
			glbSipParserSipMessage->pGeneralHdr->pDcsBillingInfoHdr \
			= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
		SipHdrTypeDcsBillingInfo,&glbSipParserErrorValue);
	}
	else if((strcasecmp("Session:",yyhdr->pName)==0))  
	{
		if(glbSipParserSipMessage->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&glbSipParserSipMessage->u.pResponse->\
				pResponseHdr->slSessionHdr,&glbSipParserErrorValue);
		}
		glbSipParserRemoveFromOrderTable(glbSipParserSipMessage,\
			SipHdrTypeSession,&glbSipParserErrorValue);
	}
}

#ifdef __cplusplus
}
#endif


