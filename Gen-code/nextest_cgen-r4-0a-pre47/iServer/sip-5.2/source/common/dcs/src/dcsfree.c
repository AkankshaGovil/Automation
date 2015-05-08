/***********************************************************************
 ** FUNCTION:
 **             Has Free Functions For all DCS Structures

 *********************************************************************
 **
 ** FILENAME:
 ** 		dcsfree.c
 **
 ** DESCRIPTION:
 ** 		This file contains the code to free all DCS structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 13Nov00   S.Luthra			Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


#include "sipstruct.h"
#include "dcsfree.h"
#include "siplist.h"
#include "portlayer.h"
#include "sipfree.h"
#include "siplist.h"


#define DCS_FREE(x) \
do \
{ \
	if ((x!=SIP_NULL)) fast_memfree(FREE_MEM_ID,x,SIP_NULL);\
} \
while(0)


void sip_dcs_freeSipDcsRemotePartyIdHeader 
#ifdef ANSI_PROTO
	(SipDcsRemotePartyIdHeader *pHdr)
#else
	(pHdr)
	SipDcsRemotePartyIdHeader *pHdr;
#endif
{
	SipError dErr;
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pDispName);
		sip_freeSipAddrSpec (pHdr->pAddrSpec);
		sip_listDeleteAll (&(pHdr->slParams), &dErr);
		/*
		sip_listDeleteAll (&(pHdr->slRpiAuths), &dErr);
		*/
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void sip_dcs_freeSipDcsRpidPrivacyHeader 
#ifdef ANSI_PROTO
	(SipDcsRpidPrivacyHeader *pHdr)
#else
	(pHdr)
	SipDcsRpidPrivacyHeader *pHdr;
#endif
{
	SipError dErr;
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		sip_listDeleteAll (&(pHdr->slParams), &dErr);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsTracePartyIdHeader 
#ifdef ANSI_PROTO
	(SipDcsTracePartyIdHeader *pHdr)
#else
	(pHdr)
	SipDcsTracePartyIdHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		sip_freeSipAddrSpec (pHdr->pAddrSpec);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsAnonymityHeader 
#ifdef ANSI_PROTO
	(SipDcsAnonymityHeader *pHdr)
#else
	(pHdr)
	SipDcsAnonymityHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pTag);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsMediaAuthorizationHeader 
#ifdef ANSI_PROTO
	(SipDcsMediaAuthorizationHeader *pHdr)
#else
	(pHdr)
	SipDcsMediaAuthorizationHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pAuth);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsGateHeader
#ifdef ANSI_PROTO
	(SipDcsGateHeader *pHdr)
#else
	(pHdr)
	SipDcsGateHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pHost);
		HSS_FREE(pHdr->pPort);
		HSS_FREE(pHdr->pId);
		HSS_FREE(pHdr->pKey);
		HSS_FREE(pHdr->pCipherSuite);
		HSS_FREE(pHdr->pStrength);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsStateHeader
#ifdef ANSI_PROTO
	(SipDcsStateHeader *pHdr)
#else
	(pHdr)
	SipDcsStateHeader *pHdr;
#endif
{
	SipError dErr;
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pHost);
		sip_listDeleteAll (&(pHdr->slParams), &dErr);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsOspsHeader
#ifdef ANSI_PROTO
	(SipDcsOspsHeader *pHdr)
#else
	(pHdr)
	SipDcsOspsHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pTag);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsBillingIdHeader
#ifdef ANSI_PROTO
	(SipDcsBillingIdHeader *pHdr)
#else
	(pHdr)
	SipDcsBillingIdHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pId);
		HSS_FREE(pHdr->pFEId);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}	
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void sip_dcs_freeSipDcsAcctEntry
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pEntry)
#else
	(pEntry)
	SipDcsAcctEntry *pEntry;
#endif
{
	if (pEntry == SIP_NULL)
		return;
	HSS_LOCKREF(pEntry->dRefCount);HSS_DECREF(pEntry->dRefCount);
	if(HSS_CHECKREF(pEntry->dRefCount))
	{
		HSS_FREE(pEntry->pChargeNum);
		HSS_FREE(pEntry->pCallingNum);
		HSS_FREE(pEntry->pCalledNum);
		HSS_FREE(pEntry->pRoutingNum);
		HSS_FREE(pEntry->pLocationRoutingNum);
		HSS_UNLOCKREF(pEntry->dRefCount);
		HSS_DELETEREF(pEntry->dRefCount);
		HSS_FREE(pEntry);
	}
	else
	{
		HSS_UNLOCKREF(pEntry->dRefCount);
	}
}


void sip_dcs_freeSipDcsBillingInfoHeader
#ifdef ANSI_PROTO
	(SipDcsBillingInfoHeader *pHdr)
#else
	(pHdr)
	SipDcsBillingInfoHeader *pHdr;
#endif
{
	SipError dErr;
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pHost);
		HSS_FREE(pHdr->pPort);
		sip_listDeleteAll(&(pHdr->slAcctEntry), &dErr);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void sip_dcs_freeSipDcsLaesHeader 
#ifdef ANSI_PROTO
	(SipDcsLaesHeader *pHdr)
#else
	(pHdr)
	SipDcsLaesHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pSignatureHost);
		HSS_FREE(pHdr->pSignaturePort);
		HSS_FREE(pHdr->pContentHost);
		HSS_FREE(pHdr->pContentPort);
		HSS_FREE(pHdr->pKey);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipDcsRedirectHeader 
#ifdef ANSI_PROTO
	(SipDcsRedirectHeader *pHdr)
#else
	(pHdr)
	SipDcsRedirectHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		sip_freeSipUrl (pHdr->pCalledId);
		sip_freeSipUrl (pHdr->pRedirector);
		pHdr->dNum = 0;	
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_dcs_freeSipSessionHeader
#ifdef ANSI_PROTO
	(SipSessionHeader *pHdr)
#else
	(pHdr)
	SipSessionHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{ 
		HSS_FREE(pHdr->pTag);	
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void __sip_dcs_freeSipDcsRemotePartyIdHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsRemotePartyIdHeader ((SipDcsRemotePartyIdHeader *)pHdr);
}

void __sip_dcs_freeSipDcsRpidPrivacyHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsRpidPrivacyHeader ((SipDcsRpidPrivacyHeader *)pHdr);
}


void __sip_dcs_freeSipDcsTracePartyIdHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsTracePartyIdHeader ((SipDcsTracePartyIdHeader*)pHdr);
}


void __sip_dcs_freeSipDcsAnonymityHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsAnonymityHeader ((SipDcsAnonymityHeader *)pHdr);
}


void __sip_dcs_freeSipDcsMediaAuthorizationHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsMediaAuthorizationHeader ((SipDcsMediaAuthorizationHeader *)pHdr);
}


void __sip_dcs_freeSipDcsGateHeader 
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsGateHeader ((SipDcsGateHeader *)pHdr);
}


void __sip_dcs_freeSipDcsStateHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsStateHeader ((SipDcsStateHeader *)pHdr);
}


void __sip_dcs_freeSipDcsOspsHeader 
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsOspsHeader ((SipDcsOspsHeader *)pHdr);
}


void __sip_dcs_freeSipDcsBillingIdHeader 
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsBillingIdHeader ((SipDcsBillingIdHeader *)pHdr);
}


void __sip_dcs_freeSipDcsLaesHeader 
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsLaesHeader ((SipDcsLaesHeader *)pHdr);
}


void __sip_dcs_freeSipDcsRedirectHeader 
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsRedirectHeader ((SipDcsRedirectHeader *)pHdr);
}


void __sip_dcs_freeSipSessionHeader 
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipSessionHeader ((SipSessionHeader *)pHdr);
}

void __sip_dcs_freeSipDcsBillingInfoHeader 
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_dcs_freeSipDcsBillingInfoHeader ((SipDcsBillingInfoHeader *)pHdr);
}

void __sip_dcs_freeSipDcsAcctEntry 
#ifdef ANSI_PROTO
	(SIP_Pvoid pEntry)
#else
	(pEntry)
	SIP_Pvoid pEntry;
#endif
{
	sip_dcs_freeSipDcsAcctEntry ((SipDcsAcctEntry *)pEntry);
}
