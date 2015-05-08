/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes of all DCS Header 
**      related APIs
**
*******************************************************************************
**
** FILENAME:
** 	dcs.h
**
** DESCRIPTION:
**  	
**
** DATE      NAME           REFERENCE      REASON
** ----      ----           ---------      ------
** 15Nov00   S.Luthra	    		   Creation
**
** Copyrights 2000, Hughes Software Systems, Ltd.
******************************************************************************/

#ifndef __DCS_H__
#define __DCS_H__

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern SipBool sip_dcs_getDispNameFromDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppDispName, SipError *pErr));

extern SipBool sip_dcs_setDispNameInDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pDispName, SipError *pErr));

#ifdef SIP_BY_REFERENCE 
extern SipBool sip_dcs_getAddrSpecFromDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipAddrSpec **ppAddrSpec, SipError *pErr));
#else
extern SipBool sip_dcs_getAddrSpecFromDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipAddrSpec *pAddrSpec, SipError *pErr));
#endif

extern SipBool sip_dcs_setAddrSpecInDcsRemotePartyIdHdr _ARGS_ (( SipHeader *pHdr,\
			SipAddrSpec *pAddrSpec, SipError *pErr));

extern SipBool sip_dcs_getParamCountFromDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE 
extern SipBool sip_dcs_getParamAtIndexFromDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_dcs_getParamAtIndexFromDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#endif

extern SipBool sip_dcs_setParamAtIndexInDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_insertParamAtIndexInDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_deleteParamAtIndexInDcsRemotePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U32bit dIndex, SipError *pErr));


extern SipBool sip_dcs_getParamCountFromDcsRpidPrivacyHdr _ARGS_ ((SipHeader \
	*pHdr, SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_dcs_getParamAtIndexFromDcsRpidPrivacyHdr _ARGS_ ((SipHeader \
	*pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_dcs_getParamAtIndexFromDcsRpidPrivacyHdr _ARGS_ ((SipHeader \
	*pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#endif

extern SipBool sip_dcs_setParamAtIndexInDcsRpidPrivacyHdr _ARGS_ ((SipHeader \
	*pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_insertParamAtIndexInDcsRpidPrivacyHdr _ARGS_ ((SipHeader\
	*pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_deleteParamAtIndexInDcsRpidPrivacyHdr _ARGS_ ((SipHeader\
	*pHdr,SIP_U32bit dIndex, SipError *pErr));

#ifdef SIP_BY_REFERENCE 
extern SipBool sip_dcs_getAddrSpecFromDcsTracePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipAddrSpec **ppAddrSpec, SipError *pErr));
#else
extern SipBool sip_dcs_getAddrSpecFromDcsTracePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipAddrSpec *pAddrSpec, SipError *pErr));
#endif

extern SipBool sip_dcs_setAddrSpecInDcsTracePartyIdHdr _ARGS_ ((SipHeader *pHdr,\
			SipAddrSpec *pAddrSpec,  SipError *pErr));

extern SipBool sip_dcs_getTagFromDcsAnonymityHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppTag, SipError *pErr));

extern SipBool sip_dcs_setTagInDcsAnonymityHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pTag, SipError *pErr));

extern SipBool sip_dcs_getAuthFromDcsMediaAuthorizationHdr _ARGS_ ((SipHeader* \
			pHdr, SIP_S8bit  **ppAuth, SipError *pErr));

extern SipBool sip_dcs_setAuthInDcsMediaAuthorizationHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pAuth, SipError *pErr));

extern SipBool sip_dcs_getHostFromDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit  **ppHost,  SipError *pErr));

extern SipBool sip_dcs_setHostInDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pHost, SipError *pErr));

extern SipBool sip_dcs_getPortFromDcsGateHdr  _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit *pPort, SipError *pErr));

extern SipBool sip_dcs_setPortInDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit  dPort, SipError *pErr));

extern SipBool sip_dcs_getIdFromDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit  **ppId,  SipError *pErr));

extern SipBool sip_dcs_setIdInDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pId, SipError *pErr));

extern SipBool sip_dcs_getKeyFromDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppKey, SipError *pErr));

extern SipBool sip_dcs_setKeyInDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pKey, SipError *pErr));

extern SipBool sip_dcs_getCipherSuiteFromDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppCipherSuite, SipError *pErr));

extern SipBool sip_dcs_setCipherSuiteInDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pCipherSuite, SipError *pErr));

extern SipBool sip_dcs_getStrengthFromDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppStrength, SipError *pErr));

extern SipBool sip_dcs_setStrengthInDcsGateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pStrength, SipError *pErr));

extern SipBool sip_dcs_getHostFromDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppHost, SipError *pErr));

extern SipBool sip_dcs_setHostInDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pHost, SipError *pErr));

extern SipBool sip_dcs_getParamCountFromDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE 
extern SipBool sip_dcs_getParamAtIndexFromDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_dcs_getParamAtIndexFromDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#endif

extern SipBool sip_dcs_setParamAtIndexInDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_insertParamAtIndexInDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_deleteParamAtIndexInDcsStateHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_getTagFromDcsOspsHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppTag, SipError *pErr));

extern SipBool sip_dcs_setTagInDcsOspsHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pTag, SipError *pErr));

extern SipBool sip_dcs_getFEIdFromDcsBillingIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppFEId, SipError *pErr));

extern SipBool sip_dcs_setFEIdInDcsBillingIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pFEId, SipError *pErr));

extern SipBool sip_dcs_getIdFromDcsBillingIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppId, SipError *pErr));

extern SipBool sip_dcs_setIdInDcsBillingIdHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pId, SipError *pErr));

extern SipBool sip_dcs_getSignatureHostFromDcsLaesHdr _ARGS_ ((SipHeader *pHdr, \
			SIP_S8bit **ppSignatureHost, SipError *pErr));

extern SipBool sip_dcs_setSignatureHostInDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pSignatureHost, SipError *pErr));

extern SipBool sip_dcs_getContentHostFromDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppContentHost, SipError *pErr));

extern SipBool sip_dcs_setContentHostInDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pContentHost, SipError *pErr));

extern SipBool sip_dcs_getSignaturePortFromDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit *pSignaturePort, SipError *pErr));

extern SipBool sip_dcs_setSignaturePortInDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit dSignaturePort, SipError *pErr));

extern SipBool sip_dcs_getContentPortFromDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit *pContentPort, SipError *pErr));

extern SipBool sip_dcs_setContentPortInDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit dContentPort, SipError *pErr));

extern SipBool sip_dcs_getKeyFromDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppKey, SipError *pErr));

extern SipBool sip_dcs_setKeyInDcsLaesHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pKey,  SipError *pErr));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_dcs_getCalledIdFromDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SipUrl **ppCalledId, SipError *pErr));
#else
extern SipBool sip_dcs_getCalledIdFromDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SipUrl *pCalledId, SipError *pErr));
#endif

extern SipBool sip_dcs_setCalledIdInDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SipUrl *pCalledId, SipError *pErr));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_dcs_getRedirectorFromDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SipUrl **ppRedirector, SipError *pErr));
#else
extern SipBool sip_dcs_getRedirectorFromDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SipUrl *pRedirector, SipError *pErr));
#endif

extern SipBool sip_dcs_setRedirectorInDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SipUrl *pRedirector, SipError *pErr));

extern SipBool sip_dcs_getNumFromDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U32bit *pNum, SipError *pErr));

extern SipBool sip_dcs_setNumInDcsRedirectHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U32bit dNum, SipError *pErr));

extern SipBool sip_dcs_getTagFromSessionHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppTag, SipError *pErr));

extern SipBool sip_dcs_setTagInSessionHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pTag, SipError *pErr));

extern SipBool sip_dcs_getHostFromDcsBillingInfoHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit **ppHost, SipError *pErr));

extern SipBool sip_dcs_setHostInDcsBillingInfoHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_S8bit *pHost, SipError *pErr));

extern SipBool sip_dcs_getPortFromDcsBillingInfoHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit *pPort, SipError *pErr));

extern SipBool sip_dcs_setPortInDcsBillingInfoHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U16bit dPort, SipError *pErr));

extern SipBool sip_dcs_getAcctEntryCountFromDcsBillingInfoHdr _ARGS_ ((SipHeader *pHdr,\
			SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE 
extern SipBool sip_dcs_getAcctEntryAtIndexFromDcsBillingInfoHdr _ARGS_ ((SipHeader* \
			pHdr, SipDcsAcctEntry **ppAcctEntry, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_dcs_getAcctEntryAtIndexFromDcsBillingInfoHdr _ARGS_ ((SipHeader*\
			pHdr, SipDcsAcctEntry *pAcctEntry, SIP_U32bit dIndex, SipError *pErr));
#endif

extern SipBool sip_dcs_setAcctEntryAtIndexInDcsBillingInfoHdr _ARGS_ ((SipHeader* \
			pHdr, SipDcsAcctEntry *pAcctEntry, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_insertAcctEntryAtIndexInDcsBillingInfoHdr _ARGS_ (\
	(SipHeader *pHdr, SipDcsAcctEntry *pAcctEntry, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_deleteAcctEntryAtIndexInDcsBillingInfoHdr _ARGS_ (\
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

extern SipBool sip_dcs_getChargeNumFromDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppChargeNum, SipError *pErr));

extern SipBool sip_dcs_setChargeNumInDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pChargeNum, SipError *pErr));

extern SipBool sip_dcs_getCallingNumFromDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppCallingNum, SipError *pErr));

extern SipBool sip_dcs_setCallingNumInDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pCallingNum, SipError *pErr));

extern SipBool sip_dcs_getCalledNumFromDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppCalledNum, SipError *pErr));

extern SipBool sip_dcs_setCalledNumInDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pCalledNum, SipError *pErr));

extern SipBool sip_dcs_getRoutingNumFromDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppRoutingNum, SipError *pErr));

extern SipBool sip_dcs_setRoutingNumInDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pRoutingNum, SipError *pErr));

extern SipBool sip_dcs_getLocationRoutingNumFromDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit **ppLocationRoutingNum, SipError *pErr));

extern SipBool sip_dcs_setLocationRoutingNumInDcsAcctEntry _ARGS_ (\
	(SipDcsAcctEntry *pAcctEntry, SIP_S8bit *pLocationRoutingNum, SipError *pErr));



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
