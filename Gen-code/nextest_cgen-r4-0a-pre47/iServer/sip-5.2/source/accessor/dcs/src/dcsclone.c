/******************************************************************************
** FUNCTION:
** 	This header file contains the source code of all DCS SIP Structure  
**      duplicating/cloning APIs.
**
*******************************************************************************
**
** FILENAME:
** 	dcsclone.c
**
** DESCRIPTION:
**  	THIS FILE IS USED INTERNALLY BY THE STACK
**
** DATE    	 NAME           REFERENCE      REASON
** ----    	 ----           ---------      ------
** 16Nov00	S. Luthra			Creation
**					
** Copyrights 2000, Hughes Software Systems, Ltd.
*******************************************************************************/

#include "dcsclone.h"
#include "sipcommon.h"
#include "sipinit.h"
#include "sipfree.h"
#include "dcsinit.h"
#include "portlayer.h"
#include "sipclone.h"
#include "sipvalidate.h"


SipBool __sip_dcs_cloneSipDcsRemotePartyIdHeader 
#ifdef ANSI_PROTO
	(SipDcsRemotePartyIdHeader *pDest, SipDcsRemotePartyIdHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsRemotePartyIdHeader *pDest;
	SipDcsRemotePartyIdHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsRemotePartyIdHeader");
	if (pDest->pDispName != SIP_NULL)
	{
		sip_freeString(pDest->pDispName);
		pDest->pDispName = SIP_NULL;
	}  
	if (pDest->pAddrSpec != SIP_NULL)
	{
		sip_freeSipAddrSpec (pDest->pAddrSpec);
		pDest->pAddrSpec = SIP_NULL;
	} 
	/*
	if ( sip_listDeleteAll (&(pDest->slRpiAuths), pErr) == SipFail)
		return SipFail;
	*/
	if ( sip_listDeleteAll (&(pDest->slParams), pErr) == SipFail)
		return SipFail;
	/* Duplicating pDispName */
	if (pSource->pDispName == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pDispName);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pDispName = pTemp;
	/* Duplicating pAddrSpec */
	if (validateSipAddrSpecType((pSource->pAddrSpec)->dType, pErr) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(pDest->pAddrSpec), (pSource->pAddrSpec)->dType,\
		 pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(pDest->pAddrSpec, pSource->pAddrSpec, pErr)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpec);
		return SipFail;
	}
	/* duplicating SipParams list */
	if (__sip_cloneSipParamList (&(pDest->slParams), &(pSource->slParams), pErr) == SipFail)
		return SipFail;
	/* duplicating Rpi-Auth field */
/* 	if(pSource->pRpiAuth == SIP_NULL)
		pDest->pRpiAuth = SIP_NULL;
	else
	{
		if(sip_initSipGenericChallenge(&(pDest->pRpiAuth),pErr)==SipFail)
			return SipFail;
		if(__sip_cloneSipChallenge((pDest->pRpiAuth),(pSource->pRpiAuth),pErr) == SipFail)
		{
			sip_freeSipGenericChallenge(pDest->pRpiAuth);
			return SipFail;
		}
	}*/
	/*
	if (__sip_cloneSipGenericChallengeList (&(pDest->slRpiAuths), &(pSource->slRpiAuths), pErr) == SipFail)
		return SipFail;
	*/

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsRemotePartyIdHeader");
	return SipSuccess;
}

SipBool __sip_dcs_cloneSipDcsRpidPrivacyHeader 
#ifdef ANSI_PROTO
	(SipDcsRpidPrivacyHeader *pDest, SipDcsRpidPrivacyHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsRpidPrivacyHeader *pDest;
	SipDcsRpidPrivacyHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsRpidPrivacyHeader");
	if ( sip_listDeleteAll (&(pDest->slParams), pErr) == SipFail)
		return SipFail;
	
	/* duplicating SipParams list */
	if (__sip_cloneSipParamList (&(pDest->slParams),\
		&(pSource->slParams), pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsRpidPrivacyHeader");
	return SipSuccess;
}

SipBool __sip_dcs_cloneSipDcsTracePartyIdHeader 
#ifdef ANSI_PROTO
	(SipDcsTracePartyIdHeader *pDest, SipDcsTracePartyIdHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsTracePartyIdHeader *pDest;
	SipDcsTracePartyIdHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsTracePartyIdHeader"); 
	if (pDest->pAddrSpec != SIP_NULL)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpec);
		pDest->pAddrSpec = SIP_NULL;
	}
	/* Duplicating pAddrSpec */
	if (validateSipAddrSpecType((pSource->pAddrSpec)->dType, pErr) == SipFail)
		return SipFail;
	if (sip_initSipAddrSpec(&(pDest->pAddrSpec), (pSource->pAddrSpec)->dType,\
		 pErr) == SipFail)
		return SipFail;
	if (__sip_cloneSipAddrSpec(pDest->pAddrSpec, pSource->pAddrSpec, pErr)\
		 == SipFail)
	{
		sip_freeSipAddrSpec(pDest->pAddrSpec);
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsTracePartyIdHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsAnonymityHeader 
#ifdef ANSI_PROTO
	(SipDcsAnonymityHeader *pDest, SipDcsAnonymityHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsAnonymityHeader *pDest;
	SipDcsAnonymityHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering __sip_dcs_cloneSipDcsAnonymityHeader");
	if (pDest->pTag != SIP_NULL)
		sip_freeString (pDest->pTag);
	/* duplicating tag */
	if (pSource->pTag == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pTag);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pTag = pTemp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting __sip_dcs_cloneSipDcsAnonymityHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsMediaAuthorizationHeader 
#ifdef ANSI_PROTO
	(SipDcsMediaAuthorizationHeader *pDest, SipDcsMediaAuthorizationHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsMediaAuthorizationHeader *pDest;
	SipDcsMediaAuthorizationHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering __sip_dcs_cloneSipDcsMediaAuthorizationHeader");
	if (pDest->pAuth != SIP_NULL)
		sip_freeString (pDest->pAuth);
	/* duplicating Auth*/
	if (pSource->pAuth == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pAuth);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pAuth = pTemp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting __sip_dcs_cloneSipDcsMediaAuthorizationHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsGateHeader 
#ifdef ANSI_PROTO 
	(SipDcsGateHeader *pDest, SipDcsGateHeader *pSource, SipError *pErr)

#else
	(pDest, pSource, pErr)
	SipDcsGateHeader *pDest;
	SipDcsGateHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIP_U16bit *pPortVal;
	SipError dErr;
	SIPDEBUGFN("Entering __sip_dcs_cloneSipDcsGateHeader");
	/* clear destination parameters */
	if (pDest->pHost != SIP_NULL)
	{
		sip_freeString (pDest->pHost);
		pDest->pHost = SIP_NULL;
	} 
	if (pDest->pPort != SIP_NULL)
	{
		fast_memfree (ACCESSOR_MEM_ID, (SIP_Pvoid)(pDest->pPort), &dErr);
		pDest->pPort = SIP_NULL;
	}
	if (pDest->pId != SIP_NULL)
	{
		sip_freeString (pDest->pId);
		pDest->pId = SIP_NULL;
	}
	if (pDest->pKey != SIP_NULL)
	{
		sip_freeString (pDest->pKey);
		pDest->pKey = SIP_NULL;
	}
	if (pDest->pCipherSuite != SIP_NULL)
	{
		sip_freeString (pDest->pCipherSuite);
		pDest->pCipherSuite = SIP_NULL;
	}
	if (pDest->pStrength != SIP_NULL)
	{
		sip_freeString (pDest->pStrength);
		pDest->pStrength = SIP_NULL;
	}
	/* duplicating Host */
	if (pSource->pHost == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pHost);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pHost = pTemp;
	/* duplicating port */
	if (pSource->pPort == SIP_NULL)
		pDest->pPort = SIP_NULL;
	else
	{
		fast_memfree (ACCESSOR_MEM_ID, (SIP_Pvoid)(pDest->pPort), pErr);
		pPortVal = (SIP_U16bit *)fast_memget(ACCESSOR_MEM_ID, sizeof(SIP_U16bit *), pErr);
		if (pPortVal == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		*pPortVal = *(pSource->pPort);
		pDest->pPort = pPortVal;
	}
	/* duplicating Id */
		if (pSource->pId == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pId);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pId = pTemp;
	/* duplicating Key */
	if (pSource->pKey == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pKey);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pKey = pTemp;
	/* duplicating Cipher suite */
	if (pSource->pCipherSuite == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pCipherSuite);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pCipherSuite = pTemp;
	/* duplivating Strength */
	if (pSource->pStrength == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pStrength);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pStrength = pTemp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting __sip_dcs_cloneSipDcsGateHeader");
	return SipSuccess;	
}


SipBool __sip_dcs_cloneSipDcsStateHeader 
#ifdef ANSI_PROTO
	(SipDcsStateHeader *pDest, SipDcsStateHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsStateHeader *pDest;
	SipDcsStateHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsStateHeader");
	if (pDest->pHost != SIP_NULL)
	{
		sip_freeString(pDest->pHost);
		pDest->pHost = SIP_NULL;
	} 
	if (sip_listDeleteAll(&(pDest->slParams), pErr) == SipFail)
		return SipFail;
	/* duplicating pHost */
	if (pSource->pHost == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pHost);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pHost = pTemp;
	/* Clone SiParams List */
	if (__sip_cloneSipParamList (&(pDest->slParams), &(pSource->slParams), pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUG("Exiting function __sip_dcs_cloneSipDcsStateHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsOspsHeader 
#ifdef ANSI_PROTO
	(SipDcsOspsHeader *pDest, SipDcsOspsHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsOspsHeader *pDest;
	SipDcsOspsHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering __sip_dcs_cloneSipDcsOspsHeader");
	if (pDest->pTag != SIP_NULL)
		sip_freeString (pDest->pTag);
	/* duplicating tag */
	if (pSource->pTag == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pTag);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pTag = pTemp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting __sip_dcs_cloneSipDcsOspsHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsBillingIdHeader 
#ifdef ANSI_PROTO
	(SipDcsBillingIdHeader *pDest, SipDcsBillingIdHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsBillingIdHeader *pDest;
	SipDcsBillingIdHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering __sip_dcs_cloneSipDcsBillingIdHeader");
	if (pDest->pId != SIP_NULL)
		sip_freeString (pDest->pId);
	/* duplicating tag */
	if (pSource->pId == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pId);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pId = pTemp;

/* clone FEId */
	if (pDest->pFEId != SIP_NULL)
                sip_freeString (pDest->pFEId);
        if (pSource->pFEId == SIP_NULL)
                pTemp = SIP_NULL;
        else
        {
                pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pFEId);
                if (pTemp == SIP_NULL)
                {
                        *pErr = E_NO_MEM;
                        return SipFail;
                }
        }
        pDest->pFEId = pTemp;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting __sip_dcs_cloneSipDcsBillingIdHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsLaesHeader 
#ifdef ANSI_PROTO
	(SipDcsLaesHeader *pDest, SipDcsLaesHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsLaesHeader *pDest;
	SipDcsLaesHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIP_U16bit *pPortVal;
	SipError dErr;
	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsLaesHeader");
	if (pDest->pSignatureHost != SIP_NULL)
	{
		sip_freeString (pDest->pSignatureHost);
		pDest->pSignatureHost = SIP_NULL;
	}
	if (pDest->pContentHost != SIP_NULL)
	{
		sip_freeString (pDest->pContentHost);
		pDest->pContentHost = SIP_NULL;
	}
	if (pDest->pKey != SIP_NULL)
	{
		sip_freeString (pDest->pKey);
		pDest->pKey = SIP_NULL;
	}
	if (pDest->pSignaturePort != SIP_NULL)
	{
		fast_memfree (ACCESSOR_MEM_ID, (SIP_Pvoid)(pDest->pSignaturePort),  &dErr);
		pDest->pSignaturePort = SIP_NULL;
	}
	if (pDest->pContentPort != SIP_NULL)
	{
		fast_memfree (ACCESSOR_MEM_ID, (SIP_Pvoid)(pDest->pContentPort),  &dErr);
		pDest->pContentPort = SIP_NULL;
	}
	/* copying Signature host */
	if (pSource->pSignatureHost == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pSignatureHost);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pSignatureHost = pTemp;
	/* copying Content host */
	if (pSource->pContentHost == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pContentHost);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pContentHost = pTemp;
	/* copying the key field */
	if (pSource->pKey == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pKey);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pKey = pTemp;
	/* copying signature port */
	if (pSource->pSignaturePort == SIP_NULL)
		pDest->pSignaturePort = SIP_NULL;
	else
	{
		pPortVal = (SIP_U16bit *)fast_memget (ACCESSOR_MEM_ID, sizeof (SIP_U16bit), pErr);
		if (pPortVal == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		*pPortVal = *(pSource->pSignaturePort);
		pDest->pSignaturePort = pPortVal;
	}
	/* copying content port */
	if (pSource->pContentPort == SIP_NULL)
		pDest->pContentPort = SIP_NULL;
	else
	{
		pPortVal = (SIP_U16bit *)fast_memget (ACCESSOR_MEM_ID, sizeof (SIP_U16bit), pErr);
		if (pPortVal == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		*pPortVal = *(pSource->pContentPort);
		pDest->pContentPort = pPortVal;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsLaesHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsRedirectHeader 
#ifdef ANSI_PROTO
	(SipDcsRedirectHeader *pDest, SipDcsRedirectHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsRedirectHeader *pDest;
	SipDcsRedirectHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("__sip_dcs_cloneSipDcsRedirectHeader");
	if (pDest->pCalledId != SIP_NULL)
	{
		sip_freeSipUrl (pDest->pCalledId);
		pDest->pCalledId = SIP_NULL;
	}
	if (pDest->pRedirector != SIP_NULL)
	{
		sip_freeSipUrl (pDest->pRedirector);
		pDest->pRedirector = SIP_NULL;
	}
	/* duplicating calledId */
	if (pSource->pCalledId == SIP_NULL)
		pDest->pCalledId = SIP_NULL;
	else
	{
		if (sip_initSipUrl(&(pDest->pCalledId), pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipUrl(pDest->pCalledId, pSource->pCalledId, pErr) == SipFail)
		{
			sip_freeSipUrl(pDest->pCalledId);
			return SipFail;
		}
	}
	/* duplicating Redirector */
	if (pSource->pRedirector == SIP_NULL)
		pDest->pRedirector = SIP_NULL;
	else
	{
		if (sip_initSipUrl(&(pDest->pRedirector), pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipUrl(pDest->pRedirector, pSource->pRedirector, pErr) == SipFail)
		{
			sip_freeSipUrl(pDest->pRedirector);
			return SipFail;
		}
	}
	/* duplicating dNum */
	pDest->dNum = pSource->dNum;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("__sip_dcs_cloneSipDcsRedirectHeader");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipSessionHeader 
#ifdef ANSI_PROTO
	(SipSessionHeader *pDest, SipSessionHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipSessionHeader *pDest;
	SipSessionHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering __sip_dcs_cloneSipSessionHeader");
	if (pDest->pTag != SIP_NULL)
		sip_freeString (pDest->pTag);
	/* duplicating tag */
	if (pSource->pTag == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pTag);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pTag = pTemp;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting __sip_dcs_cloneSipSessionHeader");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipGenericChallengeList
**
** DESCRIPTION:  This function makes a deep copy of a 
** SipList of SipGenericChallenge from the "pSource" to "pDest".
**
******************************************************************/
SipBool __sip_cloneSipGenericChallengeList
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipGenericChallenge *pTempChallenge, *pCloneChallenge;
	SIP_U32bit dCount, dIndex;

	SIPDEBUGFN("Entering function __sip_cloneSipGenericChallengeList");
	/* copying siplist of SipGenericChallenge */
	if ( sip_listDeleteAll(pDest , pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &dCount, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( dIndex = 0; dIndex < dCount ; dIndex++)
	{
		if( sip_listGetAt(pSource,dIndex, (SIP_Pvoid * ) (&pTempChallenge), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempChallenge == SIP_NULL )
			pCloneChallenge = SIP_NULL;
		else
		{
			if(sip_initSipGenericChallenge(&pCloneChallenge,pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneChallenge == SIP_NULL )
				return SipFail;

			if ( __sip_cloneSipChallenge(  pCloneChallenge, pTempChallenge,pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCloneChallenge),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneChallenge, pErr) == SipFail )
		{
			if ( pCloneChallenge != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pCloneChallenge), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipGenericChallengeList");
	return SipSuccess;	
}


SipBool __sip_dcs_cloneSipDcsAnonymityHeaderList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipDcsAnonymityHeader *pTempAnonymity, *pCloneAnonymity;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsAnonymityHeaderList");
	/* copying siplist of SipDcsAnonymityHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&pTempAnonymity), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempAnonymity == SIP_NULL )
			pCloneAnonymity = SIP_NULL;
		else
		{
			if(sip_dcs_initSipDcsAnonymityHeader(&pCloneAnonymity, pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneAnonymity == SIP_NULL )
				return SipFail;

			if ( __sip_dcs_cloneSipDcsAnonymityHeader(  pCloneAnonymity, pTempAnonymity, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCloneAnonymity),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneAnonymity, pErr) == SipFail )
		{
			if ( pCloneAnonymity != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pCloneAnonymity), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsAnonymityHeaderList");
	return SipSuccess;
}

SipBool __sip_dcs_cloneSipDcsRemotePartyIdHeaderList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipDcsRemotePartyIdHeader *pTempRemotePartyId, *pCloneRemotePartyId;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsRemotePartyIdHeaderList");
	/* copying siplist of SipDcsRemotePartyIdHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) \
			(&pTempRemotePartyId), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempRemotePartyId == SIP_NULL )
			pCloneRemotePartyId = SIP_NULL;
		else
		{
			if(sip_dcs_initSipDcsRemotePartyIdHeader(&pCloneRemotePartyId,\
				pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneRemotePartyId == SIP_NULL )
				return SipFail;

			if ( __sip_dcs_cloneSipDcsRemotePartyIdHeader( \
				pCloneRemotePartyId, pTempRemotePartyId, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, \
				(SIP_Pvoid*)&(pCloneRemotePartyId),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneRemotePartyId, pErr) == SipFail )
		{
			if ( pCloneRemotePartyId != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID, \
				(SIP_Pvoid*)&(pCloneRemotePartyId), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsRemotePartyIdHeaderList");
	return SipSuccess;
}

SipBool __sip_dcs_cloneSipDcsRpidPrivacyHeaderList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipDcsRpidPrivacyHeader *pTempRpidPrivacy, *pCloneRpidPrivacy;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsRpidPrivacyHeaderList");
	/* copying siplist of SipDcsRpidPrivacyHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) \
			(&pTempRpidPrivacy), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempRpidPrivacy == SIP_NULL )
			pCloneRpidPrivacy = SIP_NULL;
		else
		{
			if(sip_dcs_initSipDcsRpidPrivacyHeader(&pCloneRpidPrivacy,\
				pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneRpidPrivacy == SIP_NULL )
				return SipFail;

			if ( __sip_dcs_cloneSipDcsRpidPrivacyHeader( \
				pCloneRpidPrivacy, pTempRpidPrivacy, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, \
				(SIP_Pvoid*)&(pCloneRpidPrivacy),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneRpidPrivacy, pErr) == SipFail )
		{
			if ( pCloneRpidPrivacy != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID, \
				(SIP_Pvoid*)&(pCloneRpidPrivacy), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsRpidPrivacyHeaderList");
	return SipSuccess;
}

SipBool __sip_dcs_cloneSipDcsMediaAuthorizationHeaderList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipDcsMediaAuthorizationHeader *pTempMedia, *pCloneMedia;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsMediaAuthorizationHeaderList");
	/* copying siplist of SipDcsMediaAuthorizationHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) \
			(&pTempMedia), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempMedia == SIP_NULL )
			pCloneMedia = SIP_NULL;
		else
		{
			if(sip_dcs_initSipDcsMediaAuthorizationHeader(&pCloneMedia,\
				pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneMedia == SIP_NULL )
				return SipFail;

			if ( __sip_dcs_cloneSipDcsMediaAuthorizationHeader( \
				pCloneMedia, pTempMedia, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, \
					(SIP_Pvoid*)&(pCloneMedia),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneMedia, pErr) == SipFail )
		{
			if ( pCloneMedia != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID, \
				(SIP_Pvoid*)&(pCloneMedia), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsRpidPrivacyHeaderList");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsStateHeaderList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipDcsStateHeader *pTempState, *pCloneState;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_dcs_cloneSipDcsStateHeaderList");
	/* copying siplist of SipDcsStateHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&pTempState), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempState == SIP_NULL )
			pCloneState = SIP_NULL;
		else
		{
			if(sip_dcs_initSipDcsStateHeader(&pCloneState, pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneState == SIP_NULL )
				return SipFail;

			if ( __sip_dcs_cloneSipDcsStateHeader(  pCloneState, pTempState, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCloneState),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneState, pErr) == SipFail )
		{
			if ( pCloneState != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pCloneState), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_dcs_cloneSipDcsStateHeaderList");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipSessionHeaderList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipSessionHeader *pTempSession, *pCloneSession;
	SIP_U32bit count, index;

	SIPDEBUGFN("Entering function __sip_cloneSipSessionHeaderList");
	/* copying siplist of SipSessionHeader */
	if ( sip_listDeleteAll(pDest, pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &count, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( index = 0; index < count ; index++)
	{
		if( sip_listGetAt(pSource,index, (SIP_Pvoid * ) (&pTempSession), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempSession == SIP_NULL )
			pCloneSession = SIP_NULL;
		else
		{
			if(sip_dcs_initSipSessionHeader(&pCloneSession, pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneSession == SIP_NULL )
				return SipFail;

			if ( __sip_dcs_cloneSipSessionHeader(  pCloneSession, pTempSession, pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCloneSession),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneSession, pErr) == SipFail )
		{
			if ( pCloneSession != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pCloneSession), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipSessionHeaderList");
	return SipSuccess;
}


SipBool sip_dcs_cloneSipDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pDest, SipDcsAcctEntry *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsAcctEntry *pDest;
	SipDcsAcctEntry *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUG("Entering function sip_dcs_cloneSipDcsAcctEntry");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
		return SipFail;
	if(pSource == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if(pDest == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if ((__sip_dcs_cloneSipDcsAcctEntry(pDest, pSource, pErr))==SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUG("Exiting function __sip_dcs_cloneSipDcsAcctEntry");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsAcctEntry 
#ifdef ANSI_PROTO
	(SipDcsAcctEntry *pDest, SipDcsAcctEntry *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsAcctEntry *pDest;
	SipDcsAcctEntry *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUG("Entering function __sip_dcs_cloneSipDcsAcctEntry");
	if (pDest->pChargeNum != SIP_NULL)
		sip_freeString(pDest->pChargeNum);
	if (pDest->pCalledNum != SIP_NULL)
		sip_freeString (pDest->pCalledNum);
	if (pDest->pCallingNum != SIP_NULL)
		sip_freeString (pDest->pCallingNum);
	if (pDest->pRoutingNum != SIP_NULL)
		sip_freeString (pDest->pRoutingNum);
	if (pDest->pLocationRoutingNum != SIP_NULL)
		sip_freeString (pDest->pLocationRoutingNum);
	/* Now copyinmg fields */
	if (pSource->pChargeNum == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *) STRDUPACCESSOR (pSource->pChargeNum);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pChargeNum = pTemp;
	if (pSource->pCalledNum == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *) STRDUPACCESSOR (pSource->pCalledNum);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pCalledNum = pTemp;
		if (pSource->pCallingNum == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *) STRDUPACCESSOR (pSource->pCallingNum);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pCallingNum = pTemp;
	if (pSource->pRoutingNum == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *) STRDUPACCESSOR (pSource->pRoutingNum);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pRoutingNum = pTemp;
		if (pSource->pLocationRoutingNum == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *) STRDUPACCESSOR (pSource->pLocationRoutingNum);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pLocationRoutingNum = pTemp;

	*pErr = E_NO_ERROR;
	SIPDEBUG("Exiting function __sip_dcs_cloneSipDcsAcctEntry");
	return SipSuccess;
}


SipBool __sip_dcs_cloneSipDcsBillingInfoHeader 
#ifdef ANSI_PROTO
	(SipDcsBillingInfoHeader *pDest, SipDcsBillingInfoHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipDcsBillingInfoHeader *pDest;
	SipDcsBillingInfoHeader *pSource;
	SipError *pErr;
#endif
{
	SipError dErr;
	SIP_U16bit *pTempPort;
	SIP_S8bit *pTemp;
	SIPDEBUG("Entering funcytion __sip_dcs_cloneSipDcsBillingInfoHeader");
	if (pDest->pHost != SIP_NULL)
		sip_freeString (pDest->pHost);
	if (pDest->pPort != SIP_NULL)
		fast_memfree (ACCESSOR_MEM_ID, (SIP_Pvoid)(pDest->pPort), &dErr);
	sip_listDeleteAll (&(pDest->slAcctEntry), &dErr);
	/* copying parameters */
		if (pSource->pHost == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *) STRDUPACCESSOR (pSource->pHost);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pHost = pTemp;
	if (pSource->pPort == SIP_NULL)
		pDest->pPort = SIP_NULL;
	else
	{
		pTempPort = (SIP_U16bit *)fast_memget (ACCESSOR_MEM_ID, sizeof (SIP_U16bit), pErr);
		if (pTempPort == SIP_NULL)
			return SipFail;
		*pTempPort = *(pSource->pPort);
		pDest->pPort = pTempPort;
	 }
	if (__sip_dcs_cloneSipDcsAcctEntryList(&(pDest->slAcctEntry), &(pSource->slAcctEntry), pErr) == SipFail)
		return SipFail;
	*pErr = E_NO_ERROR;
	SIPDEBUG("Exiting function __sip_dcs_cloneSipDcsBillingInfoHeader");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_cloneSipDcsAcctEntryList
**
** DESCRIPTION:  This function makes a deep copy of a 
** SipList of SipDcsAcctEntry from the "pSource" to "pDest".
**
******************************************************************/
SipBool __sip_dcs_cloneSipDcsAcctEntryList 
#ifdef ANSI_PROTO
	(SipList *pDest, SipList *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipList *pDest;
	SipList *pSource;
	SipError *pErr;
#endif
{
	SipDcsAcctEntry *pTempAcctEntry, *pCloneAcctEntry;
	SIP_U32bit dCount, dIndex;

	SIPDEBUGFN("Entering function __sip_cloneSipDcsAcctEntryList");
	/* copying siplist of SipDcsAcctEntry */
	if ( sip_listDeleteAll(pDest , pErr ) == SipFail )
		return SipFail;
	if ( sip_listSizeOf(pSource, &dCount, pErr) == SipFail )
	{
		return SipFail;
	}
	for ( dIndex = 0; dIndex < dCount ; dIndex++)
	{
		if( sip_listGetAt(pSource,dIndex, (SIP_Pvoid * ) (&pTempAcctEntry), pErr) == SipFail )
		{
			return SipFail;
		}
		if ( pTempAcctEntry == SIP_NULL )
			pCloneAcctEntry = SIP_NULL;
		else
		{
			if(sip_dcs_initSipDcsAcctEntry(&pCloneAcctEntry,pErr)==SipFail)
			{
				return SipFail;
			}
			
			if ( pCloneAcctEntry == SIP_NULL )
				return SipFail;

			if ( __sip_dcs_cloneSipDcsAcctEntry(  pCloneAcctEntry, pTempAcctEntry,pErr) == SipFail )
			{
				sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pCloneAcctEntry),pErr);
				return SipFail;
			}
		}

		if ( sip_listAppend(pDest, pCloneAcctEntry, pErr) == SipFail )
		{
			if ( pCloneAcctEntry != SIP_NULL )
				sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pCloneAcctEntry), pErr);
			return SipFail;
		}
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_cloneSipDcsAcctEntryList");
	return SipSuccess;
}


