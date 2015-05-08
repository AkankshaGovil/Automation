/**************************************************************
 ** FUNCTION:
 **	 	This file has the global prototype definitions for the
 **		SIP First and Second Level Request APIs

 *************************************************************
 **
 ** FILENAME:
 ** req1csb.h
 **
 ** DESCRIPTION:
 **	
 **	 
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 16/11/99		      R.Preethy			--			Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 *****************************************************************/
#ifndef __SIP_REQUEST_H_
#define __SIP_REQUEST_H_

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


#define SIP_9999_YEAR	9999 /* year 9999 used for comparison in .c file */

/***********************************************************************
** Function: sip_getHideTypeFromHideHdr 
** Description: gets the type from the Sip Hide Header
** Parameters:
**			pHdr(IN)		- Sip Hide Header
**			pHtype(OUT)		- The header type retrieved
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
** NOTE: The type field in the Hide Header has been changed
** 		 to SIP_S8bit* This API exists only for conformance
**		 with the previous versions. Please use the other API (below)
**		 to get the type from the Hide Header.
************************************************************************/
extern SipBool sip_getHideTypeFromHideHdr _ARGS_((SipHeader *pHdr,\
		en_HideType *pHtype,SipError *pErr));

/***********************************************************************
** Function: sip_setHideTypeInHideHdr 
** Description: sets the type in the Sip Hide Header
** Parameters:
**			pHdr(IN/OUT)	- Sip Hide Header
**			pHtype(IN)		- The header type set
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
** NOTE: The type field in the Hide Header has been changed
** 		 to SIP_S8bit* This API exists only for conformance
**		 with the previous versions. Please use the other API (below)
**		 to set the type in the Hide Header.
************************************************************************/
extern SipBool sip_setHideTypeInHideHdr _ARGS_((SipHeader *hdr,\
		en_HideType htype,SipError *err));

/***********************************************************************
** Function:sip_getTypeFromHideHdr
** Description: This function retrieves the type field
**				from a SIP Hide pHeader
** Parameters:
**			pHdr(IN)		- Sip Hide Header
**			ppHtype(IN)		- The Hide Type retrieved
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
** NOTE: This is the new API added after the type in the 
** 		 hide header was converted to SIP_S8bit* 
************************************************************************/
extern SipBool sip_getTypeFromHideHdr _ARGS_((SipHeader *pHdr, \
		SIP_S8bit **ppHtype, SipError *pErr));

/***********************************************************************
** Function:sip_setTypeInHideHdr
** Description: This function sets the type field
**				in a SIP Hide pHeader
** Parameters:
**			pHdr(IN/OUT)	- Sip Hide Header
**			pHtype(IN)		- The Hide type to be set
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
** NOTE: This is the new API added after the type in the 
** 		 hide header was converted to SIP_S8bit* 
************************************************************************/
extern SipBool sip_setTypeInHideHdr _ARGS_((SipHeader *pHdr, \
		SIP_S8bit *pHtype, SipError *pErr));

/***********************************************************************
** Function: sip_getCredentialFromAuthorizationHdr 
** Description: gets the Credential type from the Sip Authorisation Header
** Parameters:
**			pHdr(IN)	- Sip Authorization Header
**			pHtype(OUT)		- The header type retrieved
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getCredentialFromAuthorizationHdr _ARGS_((SipHeader *hdr,\
		SipGenericCredential *cr, SipError *err));

/***********************************************************************
** Function: sip_setCredentialInAuthorizationHdr 
** Description: sets the Credential type in the Sip Authorisation Header
** Parameters:
**			pHdr(IN/OUT)	- Sip Authorization Header
**			pHtype(IN)		- The header type set
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setCredentialInAuthorizationHdr _ARGS_((SipHeader *hdr,\
		SipGenericCredential *cr, SipError *err));

/***********************************************************************
** Function: sip_getDispNameFromReferToHdr 
** Description: gets the Display Name from the ReferTo Header
** Parameters:
**			pHdr(IN)			- Sip ReferTo Header
**			dsipName(OUT)		- The DisplayName retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromReferToHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **dispname, SipError *err));

/***********************************************************************
** Function: sip_setDispNameInReferToHdr 
** Description: sets the Display Name in the ReferTo Header
** Parameters:
**			pHdr(IN/OUT)		- Sip ReferTo  Header
**			dsipName(IN)		- The DisplayName to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInReferToHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *dispname, SipError *err));



#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromReferToHdr 
** Description: gets the AddrSpec from the ReferTo Header
** Parameters:
**			pHdr(IN)			- Sip ReferTo Header
**			ppAddrSpec(OUT)		- The AddrSpec retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromReferToHdr _ARGS_((SipHeader *pHdr,\
		SipAddrSpec **ppAddrspec,SipError *pErr));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromReferToHdr 
** Description: gets the AddrSpec from the ReferTo Header
** Parameters:
**			pHdr(IN)			- Sip ReferTo Header
**			ppAddrSpec(OUT)		- The AddrSpec retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromReferToHdr _ARGS_((SipHeader *pHdr,\
		SipAddrSpec *pAddrspec, SipError *pErr));

#endif
/***********************************************************************
** Function: sip_setAddrSpecInReferToHdr 
** Description: sets the AddrSpec in the ReferTo Header
** Parameters:
**			pHdr(IN/OUT)		- Sip ReferTo Header
**			addrSpec(OUT)		- The AddrSpec to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInReferToHdr _ARGS_((SipHeader *pHdr,\
		SipAddrSpec *pAddrspec, SipError *pErr));



/*********************************************************
** FUNCTION:sip_getParamCountFromReferToHd
**
** DESCRIPTION: This function gets the paramater count in slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
extern SipBool sip_getParamCountFromReferToHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));


/*********************************************************
** FUNCTION:sip_getParamAtIndexFromReferToHdr
**
** DESCRIPTION: This function gets the paramater from slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromReferToHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromReferToHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif


/*********************************************************
** FUNCTION:sip_setParamAtIndexInReferToHdr
**
** DESCRIPTION: This function sets the paramater in slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
extern SipBool sip_setParamAtIndexInReferToHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));


/*********************************************************
** FUNCTION:sip_insertParamAtIndexInReferToHdr
**
** DESCRIPTION: This function inserts the paramater in slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
extern SipBool sip_insertParamAtIndexInReferToHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));


/*********************************************************
** FUNCTION:sip_deleteParamAtIndexInReferToHdr
**
** DESCRIPTION: This function  deletes the paramater from slParams
**		of the SIP ReferTo pHeader
**
**********************************************************/
extern SipBool sip_deleteParamAtIndexInReferToHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_getDispNameFromReferredByHdr 
** Description: gets the Display Name from the ReferredBy Header
** Parameters:
**			pHdr(IN)			- Sip ReferredBy Header
**			dsipName(OUT)		- The DisplayName retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromReferredByHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **dispname, SipError *err));

/***********************************************************************
** Function: sip_setDispNameInReferredByHdr 
** Description: sets the Display Name in the ReferredBy Header
** Parameters:
**			pHdr(IN/OUT)		- Sip ReferredBy  Header
**			dsipName(IN)		- The DisplayName to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInReferredByHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *dispname, SipError *err));


/***********************************************************************
** Function: sip_getMsgIdFromReferredByHdr 
** Description: gets the MsgId from the ReferredBy Header
** Parameters:
**			pHdr(IN)			- Sip ReferredBy Header
**			dsipName(OUT)		- The MsgId param value retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getMsgIdFromReferredByHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **ppMsgId, SipError *err));

/***********************************************************************
** Function: sip_setMsgIdInReferredByHdr 
** Description: sets the MsgId param value in the ReferredBy Header
** Parameters:
**			pHdr(IN/OUT)		- Sip ReferredBy  Header
**			dsipName(IN)		- The MsgId to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setMsgIdInReferredByHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *pMsgId, SipError *err));

/************************************************************************
** FUNCTION:sip_getReferrerFromReferredByHdr
**
** DESCRIPTION: This function gets the referrer url from 
**		a SIP ReferredBy pHeader
**
*************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getReferrerFromReferredByHdr _ARGS_ ((SipHeader *pHdr, SipAddrSpec *pAddrSpecReferrer, SipError *pErr));
#else
extern SipBool sip_getReferrerFromReferredByHdr _ARGS_ ((SipHeader *pHdr, SipAddrSpec **ppAddrSpecReferrer, SipError *pErr));
#endif

/***********************************************************************
** FUNCTION:sip_setReferrerInReferredByHdr
**
** DESCRIPTION: This function sets the referrer url in
**		a SIP ReferredBy pHeader
**
*************************************************************************/
extern SipBool sip_setReferrerInReferredByHdr _ARGS_ ((SipHeader *pHdr, SipAddrSpec *pAddrSpecReferrer, SipError *pErr));


/************************************************************************
** FUNCTION:sip_getReferencedrFromReferredByHdr
**
** DESCRIPTION: This function gets the referrer url from 
**		a SIP ReferredBy pHeader
**
*************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getReferencedFromReferredByHdr _ARGS_ ((SipHeader *pHdr, SipAddrSpec *pAddrSpecReferenced, SipError *pErr));
#else
extern SipBool sip_getReferencedFromReferredByHdr _ARGS_ ((SipHeader *pHdr, SipAddrSpec **ppAddrSpecReferenced, SipError *pErr));
#endif


/***********************************************************************
** FUNCTION:sip_setReferencedInReferredByHdr
**
** DESCRIPTION: This function sets the referrer url in
**		a SIP ReferredBy pHeader
**
*************************************************************************/
extern SipBool sip_setReferencedInReferredByHdr _ARGS_ ((SipHeader *pHdr, SipAddrSpec *pAddrSpecReferenced, SipError *pErr));


/*********************************************************
** FUNCTION:sip_getParamCountFromReferredByHd
**
** DESCRIPTION: This function gets the paramater count in slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
extern SipBool sip_getParamCountFromReferredByHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));


/*********************************************************
** FUNCTION:sip_getParamAtIndexFromReferredByHdr
**
** DESCRIPTION: This function gets the paramater from slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromReferredByHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromReferredByHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif


/*********************************************************
** FUNCTION:sip_setParamAtIndexInReferredByHdr
**
** DESCRIPTION: This function sets the paramater in slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
extern SipBool sip_setParamAtIndexInReferredByHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));


/*********************************************************
** FUNCTION:sip_insertParamAtIndexInReferredByHdr
**
** DESCRIPTION: This function inserts the paramater in slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
extern SipBool sip_insertParamAtIndexInReferredByHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));


/*********************************************************
** FUNCTION:sip_deleteParamAtIndexInReferredByHdr
**
** DESCRIPTION: This function  deletes the paramater from slParams
**		of the SIP ReferredBy pHeader
**
**********************************************************/
extern SipBool sip_deleteParamAtIndexInReferredByHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_getHopsFromMaxForwardsHdr 
** Description: gets the Hops from the Maximum Forwards Header
** Parameters:
**			pHdr(IN)		- Sip MaximumForwards Header
**			hops(OUT)		- The hops retrieved
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getHopsFromMaxForwardsHdr _ARGS_((SipHeader *hdr,\
		SIP_U32bit *hops, SipError *err));

/***********************************************************************
** Function: sip_setHopsInMaxForwardsHdr 
** Description: sets the Hops in the Maximum Forwards Header
** Parameters:
**			pHdr(IN/OUT)	- Sip MaximumForwards Header
**			hops(IN)		- The hops to set
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setHopsInMaxForwardsHdr _ARGS_((SipHeader *hdr,\
		SIP_U32bit hops, SipError *err));


/***********************************************************************
** Function: sip_getPriorityFromPriorityHdr 
** Description: gets the Priority from the Priority Header
** Parameters:
**			pHdr(IN)			- Sip Priority Header
**			en_Priority(OUT)	- The Priority retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
** NOTE:	The Priority field has been changed to SIP_S8bit*
**			This function exists only for conformance with
**			previous versions of the stack. Please use the
**			other function (below) for retrieving priority 
**			from the Priority Header.
************************************************************************/
extern SipBool sip_getPriorityFromPriorityHdr _ARGS_((SipHeader *hdr,\
		en_Priority *priority, SipError *err));

/***********************************************************************
** Function: sip_setPriorityInPriorityHdr 
** Description: sets the Priority in the Priority Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Priority Header
**			en_Priority(IN)		- The Priority to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
** NOTE:	The Priority field has been changed to SIP_S8bit*
**			This function exists only for conformance with
**			previous versions of the stack. Please use the
**			other function (below) for setting priority 
**			in the Priority Header.
************************************************************************/
extern SipBool sip_setPriorityInPriorityHdr _ARGS_((SipHeader *hdr,\
		en_Priority priority, SipError *err));

/***********************************************************************
** Function:sip_getPriorityStringFromPriorityHdr
** Description: This function retrieves the Priority from a
**				SIP Priority pHeader
** Parameters:
**			pHdr(IN)			- Sip Priority Header
**			ppPriority(OUT)		- The Priority retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
** NOTE: This is the new API added to get the priority 
**		 field from the Priority Header as a string.
************************************************************************/
extern SipBool sip_getPriorityStringFromPriorityHdr _ARGS_((SipHeader *pHdr, \
		SIP_S8bit **ppPriority, SipError *pErr));

/***********************************************************************
** Function:sip_setPriorityStringInPriorityHdr
** Description: This function retrieves the Priority from a
**				SIP Priority pHeader
** Parameters:
**			pHdr(IN/OUT)			- Sip Priority Header
**			pPriority(IN)		- The Priority to be set 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
** NOTE: This is the new API added to set the priority 
**		 field in the Priority Header from the string 
**		 input.
************************************************************************/
extern SipBool sip_setPriorityStringInPriorityHdr _ARGS_((SipHeader *pHdr, \
		SIP_S8bit *pPriority, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getCredentialsFromProxyAuthorizationHdr 
** Description: gets the Credential from the ProxyAuthorization Header
** Parameters:
**			pHdr(IN)			- Sip ProxyAuthorization Header
**			credentials(OUT)	- The Credentials retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getCredentialsFromProxyAuthorizationHdr _ARGS_((SipHeader *hdr,\
		SipGenericCredential **credentials, SipError *err));

#else
/***********************************************************************
** Function: sip_getCredentialsFromProxyAuthorizationHdr 
** Description: gets the Credential from the ProxyAuthorization Header
** Parameters:
**			pHdr(IN)			- Sip ProxyAuthorization Header
**			credentials(OUT)	- The Credentials retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getCredentialsFromProxyAuthorizationHdr _ARGS_((SipHeader *hdr,\
		SipGenericCredential *credentials, SipError *err));

#endif
/***********************************************************************
** Function: sip_setCredentialsInProxyAuthorizationHdr 
** Description: sets the Credential in the ProxyAuthorization Header
** Parameters:
**			pHdr(IN/OUT)		- Sip ProxyAuthorization Header
**			credentials(IN)		- The Credentials to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setCredentialsInProxyAuthorizationHdr _ARGS_((SipHeader *hdr,\
		SipGenericCredential *credentials, SipError *err));

/***********************************************************************
** Function: sip_getTokenFromProxyRequireHdr 
** Description: gets the Token from the ProxyRequired Header
** Parameters:
**			pHdr(IN)			- Sip ProxyRequired Header
**			credentials(OUT)	- The Token retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getTokenFromProxyRequireHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **token, SipError *err));

/***********************************************************************
** Function: sip_setTokenInProxyRequireHdr 
** Description: sets the Token in the ProxyRequired Header
** Parameters:
**			pHdr(IN/OUT)		- Sip ProxyRequired Header
**			Token(OUT)			- The Token to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setTokenInProxyRequireHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *token, SipError *err));

/***********************************************************************
** Function: sip_getDispNameFromRouteHdr 
** Description: gets the Display Name from the Route Header
** Parameters:
**			pHdr(IN)			- Sip Route Header
**			dsipName(OUT)		- The DisplayName retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromRouteHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **dispname, SipError *err));

/***********************************************************************
** Function: sip_setDispNameInRouteHdr 
** Description: sets the Display Name in the Route Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Route Header
**			dsipName(IN)		- The DisplayName to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInRouteHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *dispname, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromRouteHdr 
** Description: gets the AddrSpec from the Route Header
** Parameters:
**			pHdr(IN)			- Sip Route Header
**			ppAddrSpec(OUT)		- The AddrSpec retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromRouteHdr _ARGS_((SipHeader *pHdr,\
		SipAddrSpec **ppAddrspec,SipError *pErr));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromRouteHdr 
** Description: gets the AddrSpec from the Route Header
** Parameters:
**			pHdr(IN)			- Sip Route Header
**			ppAddrSpec(OUT)		- The AddrSpec retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromRouteHdr _ARGS_((SipHeader *hdr,\
		SipAddrSpec *addrspec, SipError *err));

#endif
/***********************************************************************
** Function: sip_setAddrSpecInRouteHdr 
** Description: sets the AddrSpec in the Route Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Route Header
**			addrSpec(OUT)		- The AddrSpec to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInRouteHdr _ARGS_((SipHeader *hdr,\
		SipAddrSpec *addrspec, SipError *err));

/***********************************************************************
** Function:  sip_getParamCountFromRouteHdr
** Description: This function retrieves the number of params in a Route
**		header
** Parameters:
**			pHdr(IN)			- Sip Route Header
**			pCount(OUT)			- The number of parameters
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromRouteHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromRouteHdr
** Description: This function retrieves a param at a specified index in
		Route header
** Parameters:
**			pHdr(IN)			- Sip Route Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInRouteHdr
** Description: This function sets a param at a specified index in
		Route header
** Parameters:
**			pHdr(IN/OUT)			- Sip Route Header
**			pParam(IN)			- The param to be set
**			dIndex(OUT)			_ The index at which the param is to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInRouteHdr
** Description: This function inserts a param at a specified index in
		Route header
** Parameters:
**			pHdr(IN/OUT)			- Sip Route Header
**			pParam(IN)			- The param to be inserted
**			dIndex(OUT)			_ The index at which the param is to be inserted
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInRouteHdr
** Description: This function deletes a param at a specified index in
		Route header
** Parameters:
**			pHdr(IN/OUT)			- Sip Route Header
**			pParam(IN)			- The param to be deleted
**			dIndex(OUT)			_ The index at which the param is to be deleted
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInRouteHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_getDispNameFromAlsoHdr 
** Description: gets the Display Name from the Also Header
** Parameters:
**			pHdr(IN)			- Sip Also Header
**			dsipName(OUT)		- The DisplayName retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromAlsoHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **dispname, SipError *err));

/***********************************************************************
** Function: sip_setDispNameInAlsoHdr 
** Description: sets the Display Name in the Also Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Also Header
**			dsipName(IN)		- The DisplayName to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInAlsoHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *dispname, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromAlsoHdr 
** Description: gets the AddrSpec from the Also Header
** Parameters:
**			pHdr(IN)			- Sip Also Header
**			ppAddrSpec(OUT)		- The AddrSpec retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromAlsoHdr _ARGS_((SipHeader *pHdr,\
		SipAddrSpec **ppAddrspec,SipError *pErr));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromRouteHdr 
** Description: gets the AddrSpec from the Route Header
** Parameters:
**			pHdr(IN)			- Sip Route Header
**			ppAddrSpec(OUT)		- The AddrSpec retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromAlsoHdr _ARGS_((SipHeader *hdr,\
		SipAddrSpec *addrspec, SipError *err));

#endif
/***********************************************************************
** Function: sip_setAddrSpecInAlsoHdr 
** Description: sets the AddrSpec in the Also Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Also Header
**			addrSpec(OUT)		- The AddrSpec to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInAlsoHdr _ARGS_((SipHeader *hdr,\
		SipAddrSpec *addrspec, SipError *err));


/***********************************************************************
** Function: sip_getKeySchemeFromRespKeyHdr 
** Description: gets the KeyScheme from the Response Key Header
** Parameters:
**			pHdr(IN)			- Sip Response Key Header
**			scheme(OUT)			- The key scheme retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getKeySchemeFromRespKeyHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **scheme, SipError *err));

/***********************************************************************
** Function: sip_setKeySchemeInRespKeyHdr 
** Description: sets the KeyScheme in the Response Key Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Response Key Header
**			scheme(OUT)			- The key scheme to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setKeySchemeInRespKeyHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *scheme, SipError *err));

/***********************************************************************
** Function: sip_getKeyParamCountFromRespHdr 
** Description: gets the number of Key Parameters from the Response Key Header
** Parameters:
**			pHdr(IN)			- Sip Response Key Header
**			count(OUT)			- The key Parameter count retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getKeyParamCountFromRespKeyHdr _ARGS_((SipHeader *pHdr, \
		SIP_U32bit *count, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getKeyParamAtIndexFromRespKeyHdr 
** Description: gets a Key Parameter at an index from the Response Key Header
** Parameters:
**			pHdr(IN)			- Sip Response Key Header
**			ppKeyParam(OUT)		- The key Parametr at an index  retrieved
**			index (IN)			- The index at which the Key Parameter is retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getKeyParamAtIndexFromRespKeyHdr _ARGS_((SipHeader *pHdr, \
		SipParam **ppKeyparam, SIP_U32bit index, SipError *pErr));

#else
/***********************************************************************
** Function: sip_getKeyParamAtIndexFromRespKeyHdr 
** Description: gets a Key Parameter at an index from the Response Key Header
** Parameters:
**			pHdr(IN)			- Sip Response Key Header
**			ppKeyParam(OUT)		- The key Parameter at an index  retrieved
**			index (IN)			- The index at which the Key Parameter is retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getKeyParamAtIndexFromRespKeyHdr _ARGS_((SipHeader *pHdr, \
		SipParam *pKeyparam, SIP_U32bit index, SipError *pErr));

#endif
/***********************************************************************
** Function: sip_insertKeyParamAtIndexInRespKeyHdr 
** Description: gets a Key Parameter at an index from the Response Key Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Response Key Header
**			ppKeyParam(OUT)		- The key Parameter at an index  retrieved
**			index (IN)			- The index at which the Key Parameter is retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertKeyParamAtIndexInRespKeyHdr _ARGS_((SipHeader *pHdr, \
		SipParam *pkeyparam, SIP_U32bit index, SipError *pErr));

/***********************************************************************
** Function: sip_setKeyParamAtIndexInRespKeyHdr 
** Description: sets a Key Parameter at an index in the Response Key Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Response Key Header
**			pKeyParam(OUT)		- The key Parameter to set
**			index (IN)			- The index at which the Key Parameter is set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setKeyParamAtIndexInRespKeyHdr _ARGS_((SipHeader *pHdr, \
		SipParam *pkeyparam, SIP_U32bit index, SipError *pErr));

/***********************************************************************
** Function: sip_deleteKeyParamAtIndexInRespKeyHdr 
** Description: deletes a Key Parameter at an index from the Response Key Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Response Key Header
**			pKeyParam(IN)		- The key Parameter at an index  to be deleted
**			index (IN)			- The index at which the Key Parameter is to be deleted
**			pErr(OUT)			- Possible Error value (see API ref doc)
** Description:
** Parameters:
**
************************************************************************/
extern SipBool sip_deleteKeyParamAtIndexInRespKeyHdr _ARGS_((SipHeader *pHdr, \
		SIP_U32bit index, SipError *pErr));

/***********************************************************************
** Function: sip_getSubjectFromSubjectHdr 
** Description: gets the subject from the Subject  Header
** Parameters:
**			pHdr(IN)			- Sip Subject Header
**			subject(OUT)		- The subject field  retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getSubjectFromSubjectHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **subject, SipError *err));

/***********************************************************************
** Function: sip_setSubjectInSubjectHdr 
** Description: sets the Subject in the Subject  Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Subject Header
**			subject(IN)			- The subject field  to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setSubjectInSubjectHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *subject, SipError *err));

/***********************************************************************
** Function: sip_setCredentialsInAuthorizationHdr 
** Description: sets the Credential in the Authorization  Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Authorization Header
**			credential(IN)		- The Credential  set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setCredentialsInAuthorizationHdr _ARGS_( (SipHeader *hdr, \
		SipGenericCredential *credentials, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getCredentialsFromAuthorizationHdr  
** Description: gets the Credential from the Authorization  Header
** Parameters:
**			pHdr(IN)			- Sip Authorization Header
**			credential(OUT)		- The Credential  to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getCredentialsFromAuthorizationHdr  _ARGS_( (SipHeader *hdr,	\
		SipGenericCredential **cr, SipError *err));

#else
/***********************************************************************
** Function: sip_getCredentialsFromAuthorizationHdr  
** Description: gets the Credential from the Authorization  Header
** Parameters:
**			pHdr(IN)			- Sip Authorization Header
**			credential(OUT)		- The Credential  to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getCredentialsFromAuthorizationHdr  _ARGS_( (SipHeader *hdr,	\
		SipGenericCredential *cr, SipError *err));

#endif


/*SECOND LEVEL APIS*/

/***************************************************************
** FUNCTION: sip_getFeatureParamFromContactParam
**
** DESCRIPTION: This function retrieves the FeatureParam
**		from a SIP contact-param
**
***************************************************************/
#ifdef SIP_BY_REFERENCE 
SipBool sip_getFeatureParamFromContactParam _ARGS_(
	(SipContactParam *pCp, SipParam **ppFeatureParam, SipError *pErr)) ;
#else
SipBool sip_getFeatureParamFromContactParam _ARGS_(
	(SipContactParam *pCp, SipParam *pFeatureParam, SipError *pErr)) ;
#endif

/***************************************************************
** FUNCTION: sip_setFeatureParamInContactParam
**
** DESCRIPTION: This function sets the feature-attribute in a
**		SIP contact param
**
***************************************************************/

SipBool sip_setFeatureParamInContactParam _ARGS_(
	(SipContactParam *pCp, SipParam *pParam, SipError *pErr)) ;

/***********************************************************************
** Function: sip_getTypeFromContactParam 
** Description: gets the Type from the Contact Parameter
** Parameters:
**			cp(IN)				- Sip Contact Parameter
**			type(OUT)			- The type  to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getTypeFromContactParam _ARGS_((SipContactParam *cp,\
		en_ContactParamsType *type, SipError *err));

/***********************************************************************
** Function: sip_getDayFromDateFormat 
** Description: gets the Day from the DateFormat  
** Parameters:
**			hdr(IN)				- Sip Date Format 
**			day(OUT)			- The day  to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getDayFromDateFormat _ARGS_((SipDateFormat *hdr,\
		SIP_U8bit *day, SipError *err));

/***********************************************************************
** Function: sip_setDayInDateFormat 
** Description: sets the Day in the DateFormat  
** Parameters:
**			hdr(IN/OUT)			- Sip Date Format
**			day(IN)				- The day  to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setDayInDateFormat _ARGS_((SipDateFormat *hdr,\
		SIP_U8bit day, SipError *err));

/***********************************************************************
** Function: sip_getMonthFromDateFormat 
** Description: gets the Month from the DateFormat
** Parameters:
**			hdr(IN)				- Sip Date Format 
**			month(OUT)			- The month  to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getMonthFromDateFormat _ARGS_((SipDateFormat *hdr,\
		en_Month *month, SipError *err));

/***********************************************************************
** Function: sip_setMonthInDateFormat 
** Description: sets the Month in the DateFormat  
** Parameters:
**			hdr(IN/OUT)			- Sip Date Format 
**			month(IN)			- The month  to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setMonthInDateFormat _ARGS_((SipDateFormat *hdr,\
		en_Month month, SipError *err));

/***********************************************************************
** Function: sip_getYearFromDateFormat 
** Description: gets the Year from the DateFormat 
** Parameters:
**			hdr(IN)				- Sip Date Format 
**			year(OUT)			- The year to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getYearFromDateFormat _ARGS_((SipDateFormat *hdr,\
		SIP_U16bit *year, SipError *err));

/***********************************************************************
** Function: sip_setYearInDateFormat 
** Description: sets the Year in the DateFormat  
** Parameters:
**			hdr(IN/OUT)			- Sip Date Format
**			year(IN)			- The year to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setYearInDateFormat _ARGS_((SipDateFormat *hdr,\
		SIP_U16bit year, SipError *err));

/***********************************************************************
** Function: sip_getHourFromTimeFormat 
** Description: gets the Hour from the TimeFormat
** Parameters:
**			hdr(IN)				- Sip Time Format
**			hour(OUT)			- The hour to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getHourFromTimeFormat _ARGS_((SipTimeFormat *hdr,\
		SIP_S8bit *hour, SipError *err));

/***********************************************************************
** Function: sip_setHourInTimeFormat 
** Description: sets the Hour in the TimeFormat  
** Parameters:
**			hdr(IN/OUT)			- Sip Time Format
**			hour(IN)			- The hour to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setHourInTimeFormat _ARGS_((SipTimeFormat *hdr,\
		SIP_S8bit hour, SipError *err));

/***********************************************************************
** Function: sip_getMinFromTimeFormat 
** Description: gets the Minutes from the TimeFormat
** Parameters:
**			hdr(IN)				- Sip Time Format 
**			min(OUT)			- The minutes to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getMinFromTimeFormat _ARGS_((SipTimeFormat *hdr,\
		SIP_S8bit *min, SipError *err));

/***********************************************************************
** Function: sip_setMinInTimeFormat 
** Description: sets the Minutes in the TimeFormat
** Parameters:
**			hdr(IN/OUT)			- Sip Time Format 
**			min(IN)				- The minutes to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setMinInTimeFormat _ARGS_((SipTimeFormat *hdr,\
		SIP_S8bit min, SipError *err));

/***********************************************************************
** Function: sip_getSecFromTimeFormat 
** Description: gets the Seconds from the TimeFormat
** Parameters:
**			hdr(IN)				- Sip Time Format
**			sec(OUT)			- The seconds to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getSecFromTimeFormat _ARGS_((SipTimeFormat *hdr,\
		SIP_S8bit *sec, SipError *err));

/***********************************************************************
** Function: sip_setSecInTimeFormat 
** Description: sets the Seconds in the TimeFormat
** Parameters:
**			hdr(IN/OUT)			- Sip Time Format 
**			sec(IN)				- The seconds to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setSecInTimeFormat _ARGS_((SipTimeFormat *hdr,\
		SIP_S8bit sec,SipError *err));

/***********************************************************************
** Function: sip_getQvalueFromContactParam 
** Description: gets the Q value from the Contact Parameter
** Parameters:
**			hdr(IN)				- Sip Contact Parameter 
**			qvalue(OUT)			- The QValue to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getQvalueFromContactParam _ARGS_((SipContactParam *hdr,\
		SIP_S8bit **qvalue, SipError *err));

/***********************************************************************
** Function: sip_setQvalueInContactParam 
** Description: sets the Q value in the Contact Parameter
** Parameters:
**			hdr(IN/OUT)			- Sip Contact Parameter
**			qvalue(IN)			- The QValue to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setQvalueInContactParam _ARGS_((SipContactParam *hdr,\
		SIP_S8bit *qvalue, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getExpiresFromContactParam 
** Description: gets the Expires from the Contact Parameter
** Parameters:
**			hdr(IN)				- Sip Contact Parameter
**			ppExpires(OUT)		- The Expires field to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getExpiresFromContactParam _ARGS_((SipContactParam *pHdr,\
		SipExpiresStruct **ppExpires,SipError *pErr));

#else
/***********************************************************************
** Function: sip_getExpiresFromContactParam 
** Description: gets the Expires from the Contact Parameter
** Parameters:
**			hdr(IN)				- Sip Contact Parameter
**			ppExpires(OUT)		- The Expires field to retrieve
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getExpiresFromContactParam _ARGS_((SipContactParam *hdr,\
		SipExpiresStruct *expires, SipError *err));

#endif
/***********************************************************************
** Function: sip_setExpiresInContactParam 
** Description: sets the Expires in the Contact Parameter
** Parameters:
**			hdr(IN/OUT)			- Sip Contact Parameter
**			pExpires(IN)		- The Expires field to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setExpiresInContactParam _ARGS_((SipContactParam *hdr,\
		SipExpiresStruct *expires, SipError *err));

/***********************************************************************
** Function: sip_getExtensionAttrFromContactParam 
** Description: gets the Extension attribute from the Contact Parameter
** Parameters:
**			hdr(IN)					- Sip Contact Parameter
**			extensionAttribute(OUT)	- The extensions attribute to retrieve
**			pErr(OUT)				- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getExtensionAttrFromContactParam _ARGS_((SipContactParam *hdr,\
		SIP_S8bit **extensionAttrribute, SipError *err));

/***********************************************************************
** Function: sip_setExtensionAttrInContactParam 
** Description: sets the ExtensionAttributes in the Contact Parameter
** Parameters:
**			hdr(IN/OUT)				- Sip Contact Parameter
**			extensionAttribute(IN)	- The extensions attribute to set
**			pErr(OUT)				- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setExtensionAttrInContactParam _ARGS_((SipContactParam *hdr,\
		SIP_S8bit *extensionAttrribute, SipError *err));

/***********************************************************************
** Function: sip_getCredentialTypeFromCredential 
** Description: get type of Credential
** Parameters:
**		cr (IN)		-  Generic Credential
**		type (OUT)	-  retrieved type of credential
**		pErr (OUT)	-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getCredentialTypeFromCredential _ARGS_((SipGenericCredential *cr,\
		en_CredentialType *type, SipError *err));

/***********************************************************************
** Function: sip_getBasicFromCredential 
** Description: get Basic token from credential
** Parameters:
**		cr (IN)		- Generic Credential
**		basic (OUT)	- retrieved basic token
**		pErr (OUT)	-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getBasicFromCredential _ARGS_((SipGenericCredential *cr,\
		SIP_S8bit **basic, SipError *err));

/***********************************************************************
** Function: sip_setBasicInCredential 
** Description: set basic token in credentials
** Parameters:
**		cr (IN/OUT) - Generic Credential
**		basic(IN) 	- basic token to set
**		pErr (OUT)	-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setBasicInCredential _ARGS_((SipGenericCredential *cr,\
		SIP_S8bit *basic, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getChallengeFromCredential 
** Description: get Challenge from Credential
** Parameters:
**		cr (IN)		- Generic Credential
**		ch (OUT)	- retrieved challenge
**		pErr (OUT)	-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getChallengeFromCredential _ARGS_((SipGenericCredential *cr,\
		SipGenericChallenge **ch, SipError *err));

#else
/***********************************************************************
** Function: sip_getChallengeFromCredential 
** Description: get Challenge from Credential
** Parameters:
**		cr (IN)		- Generic Credential
**		ch (OUT)	- retrieved challenge
**		pErr (OUT)	-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getChallengeFromCredential _ARGS_((SipGenericCredential *cr,\
		SipGenericChallenge *ch, SipError *err));

#endif
/***********************************************************************
** Function: sip_setChallengeInCredential 
** Description: set challenge in credential 
** Parameters:
**		cr (IN/OUT)		- Generic Credential
**		ch (IN)			- Challenge token to set
**		pErr (OUT)		-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setChallengeInCredential _ARGS_((SipGenericCredential *cr,\
		SipGenericChallenge *ch, SipError *err));

/***********************************************************************
** Function: sip_getAddrTypeFromAddrSpec 
** Description: get type of address from AddrSpec parameter
** Parameters:
**		addrspec (IN)	- Addr Spec parameter
**		addrtype (OUT)	- retrieved type 
**		pErr (OUT)		-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getAddrTypeFromAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		en_AddrType *addrtype, SipError *err));

/***********************************************************************
** Function: sip_getUriFromAddrSpec 
** Description: get Uri parameter from AddrSpec parameter
** Parameters:
**		addrspec (IN)	- Addr Spec parameter
**		uri (OUT)		- retrieved uri parameter
**		pErr (OUT)		-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUriFromAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SIP_S8bit **uri, SipError *err));

/***********************************************************************
** Function: sip_setUriInAddrSpec 
** Description: set Uri parameter in AddrSpec parameter
** Parameters:
**		addrspec (IN/OUT)	- Addr Spec parameter
**		uri (IN)			- uri to set
**		pErr (OUT)		-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setUriInAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SIP_S8bit *uri, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getUrlFromAddrSpec 
** Description: get Url parameter from Addr Spec
** Parameters: 
**		addrspec (IN)		- Addr Spec parameer
**		url (OUT)			- retrieved url
**		pErr (OUT)			-	Possible Error value (see API ref doc)
**
** (NOTE: "sip:" schemes are Url, non "sip:"schemes are Uri (as per grammar)
************************************************************************/
extern SipBool sip_getUrlFromAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SipUrl **url, SipError *err));

#else

/***********************************************************************
** Function: sip_getUrlFromAddrSpec 
** Description: get Url parameter from Addr Spec
** Parameters: 
**		addrspec (IN)		- Addr Spec parameter
**		url (OUT)			- Retrieved url
**		pErr (OUT)			-	Possible Error value (see API ref doc)
**
** (NOTE: "sip:" schemes are Url, non "sip:"schemes are Uri (as per grammar)
************************************************************************/

extern SipBool sip_getUrlFromAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SipUrl *url, SipError *err));

#endif

/***********************************************************************
** Function: sip_setUrlInAddrSpec 
** Description: set Url parameter in Addr Spec
** Parameters: 
**		addrspec (IN/OUT)	- Addr Spec parameter
**		url (OUT)			- url to set
**		pErr (OUT)			-	Possible Error value (see API ref doc)
**
** (NOTE: "sip:" schemes are Url, non "sip:"schemes are Uri (as per grammar)
************************************************************************/
extern SipBool sip_setUrlInAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SipUrl *url, SipError *err));


#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getSUrlFromAddrSpec 
** Description: get Sips Url parameter from Addr Spec
** Parameters: 
**		addrspec (IN)		- Addr Spec parameer
**		url (OUT)			- retrieved url
**		pErr (OUT)			-	Possible Error value (see API ref doc)
**
** (NOTE: "sip:" schemes are Url, non "sip:"schemes are Uri (as per grammar)
************************************************************************/
extern SipBool sip_getSUrlFromAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SipUrl **url, SipError *err));

#else

/***********************************************************************
** Function: sip_getSUrlFromAddrSpec 
** Description: get Url parameter from Addr Spec
** Parameters: 
**		addrspec (IN)		- Addr Spec parameter
**		url (OUT)			- Retrieved url
**		pErr (OUT)			-	Possible Error value (see API ref doc)
**
** (NOTE: "sips:" schemes are SUrl (as per grammar)
************************************************************************/

extern SipBool sip_getSUrlFromAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SipUrl *url, SipError *err));

#endif

/***********************************************************************
** Function: sip_setSUrlInAddrSpec 
** Description: set Sips Url parameter in Addr Spec
** Parameters: 
**		addrspec (IN/OUT)	- Addr Spec parameter
**		url (OUT)			- url to set
**		pErr (OUT)			-	Possible Error value (see API ref doc)
**
** (NOTE: "sips:" schemes are SUrl)
************************************************************************/
extern SipBool sip_setSUrlInAddrSpec _ARGS_((SipAddrSpec *addrspec,\
		SipUrl *url, SipError *err));


/* Third level APIs */

/***********************************************************************
** Function: sip_getSchemeFromChallenge 
** Description: get scheme token from Challenge
** Parameters:
**		ch (IN)		- Generic Challenge
**		sch (OUT)	- retrieved scheme
**		pErr (OUT)	-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getSchemeFromChallenge _ARGS_((SipGenericChallenge *ch,\
		SIP_S8bit **sch, SipError *error));

/***********************************************************************
** Function: sip_setSchemeInChallenge 
** Description: set scheme in Challenge
** Parameters:
**		ch (IN/OUT)		-  	Generic Challenge
**		sch (IN)		- 	Scheme to set
**		pErr (OUT)		-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setSchemeInChallenge _ARGS_((SipGenericChallenge *ch,\
		SIP_S8bit *sch, SipError *error));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAuthorizationParamAtIndexFromChallenge 
** Description: get authorization parameter at specified index from challenge
** Parameters:
**		pSch (IN)		- Generic Challenge
**		ppParam (OUT)	- retrieved authorization parameter	
**		dIndex (IN)		- index to retrieve from
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getAuthorizationParamAtIndexFromChallenge _ARGS_(( SipGenericChallenge *pSch,\
		SipParam **ppParam,SIP_U32bit dIndex, SipError 	*pErr));

#else

/***********************************************************************
** Function: sip_getAuthorizationParamAtIndexFromChallenge 
** Description: get authorization parameter at specified index from challenge
** Parameters:
**		pSch (IN)		- Generic Challenge
**		ppParam (OUT)	- retrieved authorization parameter	
**		dIndex (IN)		- index to retrieve from
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

extern SipBool sip_getAuthorizationParamAtIndexFromChallenge _ARGS_((SipGenericChallenge *sch,\
		SipParam *param,SIP_U32bit index,SipError *error));

#endif
/***********************************************************************
** Function: sip_insertAuthorizationParamAtIndexInChallenge 
** Description: insert authorization parameter at specified index in challenge
** Parameters:
**		sch (IN/OUT)	- Generic Challenge
**		param (IN)		- authorization parameter to insert
**		index (IN)		- index to insert at
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_insertAuthorizationParamAtIndexInChallenge _ARGS_((SipGenericChallenge *sch,\
		SipParam *param,SIP_U32bit index,SipError *error));

/***********************************************************************
** Function: sip_setAuthorizationParamAtIndexInChallenge 
** Description: set authorization parameter at specified index in challenge
** Parameters:
**		sch (IN/OUT)	- Generic Challenge
**		param (IN)		- authorization parameter to set
**		index (IN)		- index to set at
**		pErr (OUT)		- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setAuthorizationParamAtIndexInChallenge _ARGS_((SipGenericChallenge *sch,\
		SipParam *param,SIP_U32bit index,SipError *error));

/***********************************************************************
** Function: sip_getAuthorizationParamCountFromChallenge 
** Description: get number of authorization parameters in Challenge
** Parameters:
**		sch (IN)		- Generic Challenge
**		count (OUT)		- retrieved count of authorization parameters
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getAuthorizationParamCountFromChallenge _ARGS_((SipGenericChallenge *sch,\
		SIP_U32bit *count,SipError *error));

/***********************************************************************
** Function: sip_deleteAuthorizationParamAtIndexInChallenge 
** Description: delete Authorization parameter at specified index in challenge
** Parameters:
**		sch (IN/OUT)	- Generic Challenge
**		index (IN)		- index to delete from
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_deleteAuthorizationParamAtIndexInChallenge _ARGS_((SipGenericChallenge *sch,\
		SIP_U32bit index,SipError *error));

/***********************************************************************
** Function: sip_getUserFromUrl 
** Description: get user field from Url
** Parameters:
**		url (IN)	- Sip Url
**		user (OUT)	- retrieved user field
**		pErr (OUT)	-	Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUserFromUrl _ARGS_((SipUrl *url,SIP_S8bit **user, SipError *error));

/***********************************************************************
** Function: sip_setUserInUrl 
** Description: set user field from Url
** Parameters:
**		url (IN/OUT)	- Sip Url
**		user (IN)		- user field to set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setUserInUrl _ARGS_((SipUrl *url,SIP_S8bit *user, SipError *error));

/***********************************************************************
** Function: sip_getPasswordFromUrl 
** Description: get password field from Url
** Parameters:
**		url (IN)		- Sip Url
**		password (OUT)	- retrieved password field
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getPasswordFromUrl _ARGS_((SipUrl *url,SIP_S8bit **password, SipError *error));

/***********************************************************************
** Function: sip_setPasswordInUrl 
** Description: set password field in Url
** Parameters:
**		url (IN/OUT) 	- Sip Url
**		password (OUT)	- password to set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setPasswordInUrl _ARGS_((SipUrl *url,SIP_S8bit *password, SipError *error));

/***********************************************************************
** Function: sip_getHostFromUrl 
** Description: get host token from url
** Parameters:
**		url (IN)	- Sip Url
**		host (OUT)	- retrieved host
**		pErr (OUT)	- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getHostFromUrl _ARGS_((SipUrl *url,SIP_S8bit **host,SipError *error));

/***********************************************************************
** Function: sip_setHostInUrl 
** Description: set host token in url
** Parameters:
**		url (IN/OUT)	- Sip Url
**		host (IN)		- host to set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setHostInUrl _ARGS_((SipUrl *url,SIP_S8bit *host,SipError *error));

/***********************************************************************
** Function: sip_getHeaderFromUrl 
** Description: get header token from url
** Parameters:
**		url (IN)		- Sip Url
**		header (OUT)	- retrieved header parameter
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getHeaderFromUrl _ARGS_((SipUrl *url,SIP_S8bit **header,SipError *error));

/***********************************************************************
** Function: sip_setHeaderInUrl 
** Description: set header token in Url
** Parameters:
**		url (IN/OUT)	- Sip Url
**		header (IN)		- header to set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setHeaderInUrl _ARGS_((SipUrl *url,SIP_S8bit *header,SipError *error));

/***********************************************************************
** Function: sip_getPortFromUrl 
** Description: get port field from Url
** Parameters:
**		url (IN)	- Sip Url
**		port (OUT)	- retrieved port
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getPortFromUrl _ARGS_((SipUrl *url,SIP_U16bit *port,SipError *error));

/***********************************************************************
** Function: sip_setPortInUrl 
** Description: set port field in Url
** Parameters:
**		url (IN/OUT)	- Sip Url
**		port (IN)		- port to set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setPortInUrl _ARGS_((SipUrl *url,SIP_U16bit port,SipError *error));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getUrlParamAtIndexFromUrl 
** Description: get Url parameter from Url at specified index
** Parameters:
**		pUrl (IN)		- Sip Url
**		ppParam (OUT)	- retrieved url parameter
**		dIndex	(IN)	- index to retrieve from
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUrlParamAtIndexFromUrl _ARGS_((SipUrl *pUrl,\
		SipParam **ppParam,SIP_U32bit dIndex,SipError *pErr));

#else

/***********************************************************************
** Function: sip_getUrlParamAtIndexFromUrl 
** Description: get Url parameter from Url at specified index
** Parameters:
**		pUrl (IN)		- Sip Url
**		ppParam (OUT)	- retrieved url parameter
**		dIndex	(IN)	- index to retrieve from
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUrlParamAtIndexFromUrl _ARGS_((SipUrl *url,\
		SipParam *url_param,SIP_U32bit index,SipError *error));

#endif
/***********************************************************************
** Function: sip_insertUrlParamAtIndexInUrl 
** Description: insert Url parameter in Url at specified index
** Parameters:
**		url (IN/OUT)	- Sip Url
**		url_param (IN)	- url parameter to insert
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_insertUrlParamAtIndexInUrl _ARGS_((SipUrl *url,\
		SipParam *url_param,SIP_U32bit index,SipError *error));

/***********************************************************************
** Function: sip_setUrlParamAtIndexInUrl 
** Description: set Url parameter in Url at specified index
** Parameters:
**		url (IN/OUT)	- Sip Url
**		url_param (IN)	- url parameter to insert
**		pErr (OUT)		- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setUrlParamAtIndexInUrl _ARGS_((SipUrl *url,\
		SipParam *url_param,SIP_U32bit index,SipError *error));

/***********************************************************************
** Function: sip_getUrlParamCountFromUrl 
** Description: retrieve number of Url parameters in the Url
** Parameters:
**		url (IN) 	- Sip Url
**		count (OUT)	- retrieved count
**		pErr (OUT)	- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUrlParamCountFromUrl _ARGS_((SipUrl *url,\
		SIP_U32bit *count,SipError *error));

/***********************************************************************
** Function: sip_deleteUrlParamAtIndexInUrl 
** Description: delete Url parameter at specified index from Url
** Parameters:
**		url (IN/OUT)	- Sip Url
**		index (IN)		- index to delete at
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_deleteUrlParamAtIndexInUrl _ARGS_((SipUrl *url,\
		SIP_U32bit index,SipError *error));



/***********************************************************************
** Function: sip_getUriFromAlertInfoHdr 
** Description:gets the Uri field from the Sip AlertInfo Header
** Parameters:
**			hdr(IN) 				- Sip AlertInfo Header
**			ppUri(OUT)				- The Uri field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_getUriFromAlertInfoHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **ppUri, SipError *pErr));


/***********************************************************************
** Function: sip_setUriInAlertInfoHdr 
** Description:sets the Uri field in the Sip AlertInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip AlertInfo Header
**			pUri(IN)				- The Uri field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_setUriInAlertInfoHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *pUri, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromAlertInfoHdr 
** Description:gets the number of Parameters from the Sip AlertInfo Header
** Parameters:
**			hdr(IN) 			- Sip AlertInfo Header
**			count(OUT)			- The Parameter count to retrieve
**			err(OUT)			- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_getParamCountFromAlertInfoHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromAlertInfoHdr 
** Description:gets the Parameters at an index from the Sip AlertInfo Header
** Parameters:
**			hdr(IN) 			- Sip AlertInfo Header
**			pParam(OUT)			- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromAlertInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromAlertInfoHdr 
** Description:gets the Parameter at an index from the Sip AlertInfo Header
** Parameters:
**			hdr(IN) 			- Sip AlertInfo Header
**			ppParam(OUT)		- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromAlertInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam **ppParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertParamAtIndexInAlertInfoHdr 
** Description:inserts the Parameter at an index in the Sip AlertInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip AlertInfo Header
**			pParam(IN)			- The Parameter to insert
**			index(IN)			- The index at which the Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInAlertInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteParamAtIndexInAlertInfoHdr 
** Description:deletes the Parameter at an index in the Sip AlertInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip AlertInfo Header
**			index(IN)			- The index at which the Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInAlertInfoHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setParamAtIndexInAlertInfoHdr 
** Description:sets the Parameter at an index in the Sip AlertInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip AlertInfo Header
**			pParam(IN)			- The Parameter to set
**			index(IN)			- The index at which the Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_setParamAtIndexInAlertInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getCallIdFromInReplyToHdr 
** Description:gets the CallId field from the Sip InReplyTo Header
** Parameters:
**			hdr(IN) 				- Sip InReplyTo Header
**			ppCallId(OUT)				- The CallId field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_getCallIdFromInReplyToHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **ppCallId, SipError *pErr));


/***********************************************************************
** Function: sip_setCallIdInInReplyToHdr 
** Description:sets the CallId field in the Sip InReplyTo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip InReplyTo Header
**			pCallId(IN)				- The CallId field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)					
**
************************************************************************/
extern SipBool sip_setCallIdInInReplyToHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *pCallId, SipError *pErr));

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromReplacesHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP Replaces pHeader
**
***************************************************************/
extern SipBool sip_getParamCountFromReplacesHdr _ARGS_((SipHeader *hdr, \
			SIP_U32bit *count, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromReplacesHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP Replaces pHeader
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromReplacesHdr _ARGS_( (SipHeader *hdr,\
		SipNameValuePair *pParam, SIP_U32bit index, SipError *err));
#else
extern SipBool sip_getParamAtIndexFromReplacesHdr _ARGS_( (SipHeader *hdr,\
	 SipNameValuePair **ppParam, SIP_U32bit index, SipError *err));
#endif

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInReplacesHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp Replaces pHeader
**
***************************************************************/
extern SipBool sip_insertParamAtIndexInReplacesHdr _ARGS_( (SipHeader *hdr, \
		SipNameValuePair *pParam, SIP_U32bit index, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInReplacesHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP Replaces pHeader
**
***************************************************************/
extern SipBool sip_deleteParamAtIndexInReplacesHdr _ARGS_((SipHeader *hdr,\
			SIP_U32bit index, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInReplacesHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP Replaces pHeader
**
***************************************************************/
extern SipBool sip_setParamAtIndexInReplacesHdr _ARGS_( (SipHeader *hdr, \
			SipNameValuePair *pParam, SIP_U32bit index, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getFromTagFromReplacesHdr
**
** DESCRIPTION:This function retrieves the FromTag field from
**		a SIP Reply-To pHeader
**
***************************************************************/
extern SipBool sip_getFromTagFromReplacesHdr _ARGS_( (SipHeader *hdr, \
			SIP_S8bit **pFromTag, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setFromTagInReplacesHdr
**
** DESCRIPTION: This function sets the FromTag field in a SIP
**		Reply-To pHeader
**
***************************************************************/
extern SipBool sip_setFromTagInReplacesHdr _ARGS_( (SipHeader *hdr,\
			SIP_S8bit *pFromTag, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_getToTagFromReplacesHdr
**
** DESCRIPTION:This function retrieves the ToTag field from
**		a SIP Reply-To pHeader
**
***************************************************************/
extern SipBool sip_getToTagFromReplacesHdr _ARGS_( (SipHeader *hdr, \
			SIP_S8bit **pToTag, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setToTagInReplacesHdr
**
** DESCRIPTION: This function sets the ToTag field in a SIP
**		Reply-To pHeader
**
***************************************************************/
extern SipBool sip_setToTagInReplacesHdr _ARGS_( (SipHeader *hdr,\
			SIP_S8bit *pToTag, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getCallidFromReplacesHdr
**
** DESCRIPTION:This function retrieves the Callid field from
**		a SIP Reply-To pHeader
**
***************************************************************/
extern SipBool sip_getCallidFromReplacesHdr _ARGS_( (SipHeader *hdr, \
			SIP_S8bit **pCallid, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setCallidInReplacesHdr
**
** DESCRIPTION: This function sets the Callid field in a SIP
**		Reply-To pHeader
**
***************************************************************/
extern SipBool sip_setCallidInReplacesHdr _ARGS_( (SipHeader *hdr,\
			SIP_S8bit *pCallid, SipError *err));

#ifdef SIP_CONF
/*Prototypes for JOIN header */
/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromJoinHdr
**
** DESCRIPTION: This function retrieives the number of parameters
**		from a SIP Join pHeader
**
***************************************************************/
extern SipBool sip_getParamCountFromJoinHdr _ARGS_((SipHeader *hdr, \
			SIP_U32bit *count, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromJoinHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP Join Header
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromJoinHdr _ARGS_( (SipHeader *hdr,\
		SipNameValuePair *pParam, SIP_U32bit index, SipError *err));
#else
extern SipBool sip_getParamAtIndexFromJoinHdr _ARGS_( (SipHeader *hdr,\
	 SipNameValuePair **ppParam, SIP_U32bit index, SipError *err));
#endif

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInJoinHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp Join pHeader
**
***************************************************************/
extern SipBool sip_insertParamAtIndexInJoinHdr _ARGS_( (SipHeader *hdr, \
		SipNameValuePair *pParam, SIP_U32bit index, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInJoinHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP Join Header
**
***************************************************************/
extern SipBool sip_deleteParamAtIndexInJoinHdr _ARGS_((SipHeader *hdr,\
			SIP_U32bit index, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInJoinHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP Join Header
**
***************************************************************/
extern SipBool sip_setParamAtIndexInJoinHdr _ARGS_( (SipHeader *hdr, \
			SipNameValuePair *pParam, SIP_U32bit index, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getFromTagFromJoinHdr
**
** DESCRIPTION:This function retrieves the FromTag field from
**		a SIP Join pHeader
**
***************************************************************/
extern SipBool sip_getFromTagFromJoinHdr _ARGS_( (SipHeader *hdr, \
			SIP_S8bit **pFromTag, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setFromTagInJoinHdr
**
** DESCRIPTION: This function sets the FromTag field in a SIP
**		Join pHeader
**
***************************************************************/
extern SipBool sip_setFromTagInJoinHdr _ARGS_( (SipHeader *hdr,\
			SIP_S8bit *pFromTag, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_getToTagFromJoinHdr
**
** DESCRIPTION:This function retrieves the ToTag field from
**		a SIP Join pHeader
**
***************************************************************/
extern SipBool sip_getToTagFromJoinHdr _ARGS_( (SipHeader *hdr, \
			SIP_S8bit **pToTag, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setToTagInJoinHdr
**
** DESCRIPTION: This function sets the ToTag field in a SIP
**		Join pHeader
**
***************************************************************/
extern SipBool sip_setToTagInJoinHdr _ARGS_( (SipHeader *hdr,\
			SIP_S8bit *pToTag, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getCallidFromJoinHdr
**
** DESCRIPTION:This function retrieves the Callid field from
**		a SIP Join pHeader
**
***************************************************************/
extern SipBool sip_getCallidFromJoinHdr _ARGS_( (SipHeader *hdr, \
			SIP_S8bit **pCallid, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setCallidInJoinHdr
**
** DESCRIPTION: This function sets the Callid field in a SIP
**		Join pHeader
**
***************************************************************/
extern SipBool sip_setCallidInJoinHdr _ARGS_( (SipHeader *hdr,\
			SIP_S8bit *pCallid, SipError *err));

#endif/*#ifdef SIP_CONF */

#ifdef SIP_SECURITY
/***********************************************************************
** Function: sip_getMechanismNameFromSecurityClientHdr
** Description: gets the Mechanism Name from the Security-Client Header
** Parameters:
**                      pHdr(IN)                  	- Sip Security-Client Header
**                      ppMechname(OUT)           	- The DisplayName retrieved
**                      pErr(OUT)                 	- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getMechanismNameFromSecurityClientHdr _ARGS_((SipHeader *hdr,\
                SIP_S8bit **ppMechname, SipError *err));

/***********************************************************************
** Function: sip_setMechanismNameInSecurityClientHdr
** Description: sets the Display Name in the Security-Client Header
** Parameters:
**                      pHdr(IN/OUT)            	- Sip Security-Client Header
**                      pMechname(IN)           	- The DisplayName to set
**                      pErr(OUT)               	- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setMechanismNameInSecurityClientHdr _ARGS_((SipHeader *hdr,\
                SIP_S8bit *pMechname, SipError *err));

/***********************************************************************
** Function:  sip_getParamCountFromSecurityClientHdr
** Description: This function retrieves the number of params in a Security-Client
**              header
** Parameters:
**                      pHdr(IN)                	- Sip Security-Client Header
**                      pCount(OUT)             	- The number of parameters
**                      pErr(OUT)               	- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromSecurityClientHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromSecurityClientHdr
** Description: This function retrieves a param at a specified index in
                Security-Client header
** Parameters:
**                      pHdr(IN)                        - Sip Security-Client Header
**                      pParam/ppParam(OUT)             - The retrieved param
**                      dIndex (IN)                     _ The index of the param to be retrieved
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromSecurityClientHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr
));
#else
extern SipBool sip_getParamAtIndexFromSecurityClientHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInSecurityClientHdr
** Description: This function sets a param at a specified index in
                Security-Client header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Client Header
**                      pParam(IN)                      - The param to be set
**                      dIndex(OUT)                     _ The index at which the param is to be set
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInSecurityClientHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr))
;

/***********************************************************************
** Function:  sip_insertParamAtIndexInSecurityClientHdr
** Description: This function inserts a param at a specified index in
                Security-Client header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Client Header
**                      pParam(IN)                      - The param to be inserted
**                      dIndex(OUT)                     _ The index at which the param is to be inserted
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInSecurityClientHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInSecurityClientHdr
** Description: This function deletes a param at a specified index in
                Security-Client header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Client Header
**                      pParam(IN)                      - The param to be deleted
**                      dIndex(OUT)                     _ The index at which the param is to be deleted
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInSecurityClientHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: getMechanismNameFromSecurityVerifyHdr
** Description: gets the Mechanism Name from the Security-Verify Header
** Parameters:
**                      pHdr(IN)                        - Sip Security-Verify Header
**                      ppMechname(OUT)                 - The DisplayName retrieved
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getMechanismNameFromSecurityVerifyHdr _ARGS_((SipHeader *hdr,\
                SIP_S8bit **ppMechname, SipError *err));

/***********************************************************************
** Function: sip_setMechanismNameInSecurityVerifyHdr
** Description: sets the Display Name in the Security-Verify Header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Verify Header
**                      pMechname(IN)                   - The DisplayName to set
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setMechanismNameInSecurityVerifyHdr _ARGS_((SipHeader *hdr,\
                SIP_S8bit *pMechname, SipError *err));

/***********************************************************************
** Function:  sip_getParamCountFromSecurityVerifyHdr
** Description: This function retrieves the number of params in a Security-Verify
**              header
** Parameters:
**                      pHdr(IN)                        - Sip Security-Verify Header
**                      pCount(OUT)                     - The number of parameters
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromSecurityVerifyHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromSecurityVerifyHdr
** Description: This function retrieves a param at a specified index in
                Security-Verify header
** Parameters:
**                      pHdr(IN)                        - Sip Security-Verify Header
**                      pParam/ppParam(OUT)             - The retrieved param
**                      dIndex (IN)                     _ The index of the param to be retrieved
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromSecurityVerifyHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromSecurityVerifyHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex,SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInSecurityVerifyHdr
** Description: This function sets a param at a specified index in
                Security-Verify header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Verify Header
**                      pParam(IN)                      - The param to be set
**                      dIndex(OUT)                     _ The index at which the param is to be set
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInSecurityVerifyHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInSecurityVerifyHdr
** Description: This function inserts a param at a specified index in
                Security-Verify header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Verify Header
**                      pParam(IN)                      - The param to be inserted
**                      dIndex(OUT)                     _ The index at which the param is to be inserted
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInSecurityVerifyHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInSecurityVerifyHdr
** Description: This function deletes a param at a specified index in
                Security-Verify header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Verify Header
**                      pParam(IN)                      - The param to be deleted
**                      dIndex(OUT)                     _ The index at which the param is to be deleted
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInSecurityVerifyHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

#endif /* end of #ifdef SIP_SECURITY */


#ifdef SIP_3GPP
/* 3GPP related headers(RFC 3455) */
/***********************************************************************
** Function: sip_getDispNameFromPCalledPartyIdHdr 
** Description: gets the Display Name from the PCalledPartyId Header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			dsipName(OUT)		- The DisplayName retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromPCalledPartyIdHdr	_ARGS_ ((SipHeader *pHdr,SIP_S8bit **ppDispname, SipError *pErr));

/***********************************************************************
** Function: sip_setDispNameInPCalledPartyIdHdr 
** Description: gets the Display Name from the PCalledPartyId Header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			dsipName(IN)		- The DisplayName to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInPCalledPartyIdHdr _ARGS_ ((SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr));

/***********************************************************************
** Function: sip_setAddrSpecInPCalledPartyIdHdr 
** Description: sets the AddrSpec in the PCalledPartyId Header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			pAddrSpec(IN)		- The AddrSpec to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInPCalledPartyIdHdr _ARGS_ ((SipHeader *pHdr,SipAddrSpec *pAddrSpec, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamCountFromPCalledPartyIdHdr
** Description: This function retrieves the number of params in a PCalledPartyId
**		header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			pCount(OUT)			- The number of parameters
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromPCalledPartyIdHdr _ARGS_ ((SipHeader *pHdr,SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromPCalledPartyIdHdr 
** Description: gets the AddrSpec from the PCalledPartyId Header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			ppAddrSpec(OUT)		- The AddrSpec to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromPCalledPartyIdHdr	_ARGS_ ((SipHeader *pHdr,SipAddrSpec **ppAddrSpec, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromPCalledPartyIdHdr
** Description: This function retrieves a param at a specified index in
		PCalledPartyId header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPCalledPartyIdHdr _ARGS_((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromPCalledPartyIdHdr 
** Description: gets the AddrSpec from the PCalledPartyId Header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			ppAddrSpec(OUT)		- The AddrSpec to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromPCalledPartyIdHdr	_ARGS_ ((SipHeader *pHdr,SipAddrSpec *pAddrSpec, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromPCalledPartyIdHdr
** Description: This function retrieves a param at a specified index in
		PCalledPartyId header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPCalledPartyIdHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

#endif
/***********************************************************************
** Function:  sip_setParamAtIndexInPCalledPartyIdHdr
** Description: This function sets a param at a specified index in
		PCalledPartyId header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInPCalledPartyIdHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInPCalledPartyIdHdr
** Description: This function sets a param at a specified index in
		PCalledPartyId header
** Parameters:
**			pHdr(IN)			- Sip PCalledPartyId Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInPCalledPartyIdHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInPCalledPartyIdHdr
** Description: This function deletes a param at a specified index in
		PCalledPartyId header
** Parameters:
**			pHdr(IN/OUT)			- Sip PCalledPartyId Header
**			pParam(IN)			- The param to be deleted
**			dIndex(OUT)			_ The index at which the param is to be deleted
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPCalledPartyIdHdr _ARGS_((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

/* PVisitedNetworkId header */

/***********************************************************************
** Function: sip_getVNetworkSpecFromPVisitedNetworkIdHdr 
** Description: gets the VNetworkSpec from the PVisitedNetworkId Header
** Parameters:
**			pHdr(IN)			- Sip PVisitedNetworkId Header
**			ppVNetworkSpec(OUT)		- The VNetworkSpec to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getVNetworkSpecFromPVisitedNetworkIdHdr	_ARGS_ ((SipHeader *pHdr,SIP_S8bit **ppVNetworkSpec, SipError *pErr));

/***********************************************************************
** Function: sip_setVNetworkSpecInPVisitedNetworkIdHdr 
** Description: gets the VNetworkSpec from the PVisitedNetworkId Header
** Parameters:
**			pHdr(IN)			- Sip PVisitedNetworkId Header
**			ppVNetworkSpec(IN)		- The VNetworkSpec to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setVNetworkSpecInPVisitedNetworkIdHdr _ARGS_ ((SipHeader *pHdr,SIP_S8bit *pVNetworkSpec, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamCountFromPVisitedNetworkIdHdr
** Description: This function retrieves the number of params in a PVisitedNetworkId
**		header
** Parameters:
**			pHdr(IN)			- Sip PVisitedNetworkId Header
**			pCount(OUT)			- The number of parameters
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromPVisitedNetworkIdHdr _ARGS_ ((SipHeader *pHdr,SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function:  sip_getParamAtIndexFromPVisitedNetworkIdHdr
** Description: This function retrieves a param at a specified index in
		PVisitedNetworkId header
** Parameters:
**			pHdr(IN)			- Sip PVisitedNetworkId Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPVisitedNetworkIdHdr _ARGS_((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function:  sip_getParamAtIndexFromPVisitedNetworkIdHdr
** Description: This function retrieves a param at a specified index in
		PVisitedNetworkId header
** Parameters:
**			pHdr(IN)			- Sip PVisitedNetworkId Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPVisitedNetworkIdHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

#endif
/***********************************************************************
** Function:  sip_setParamAtIndexInPVisitedNetworkIdHdr
** Description: This function sets a param at a specified index in
		PVisitedNetworkId header
** Parameters:
**			pHdr(IN)			- Sip PVisitedNetworkId Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInPVisitedNetworkIdHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertsParamAtIndexInPVisitedNetworkIdHdr
** Description: This function inserts a param at a specified index in
		PVisitedNetworkId header
** Parameters:
**			pHdr(IN)			- Sip PVisitedNetworkId Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInPVisitedNetworkIdHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInPVisitedNetworkIdHdr
** Description: This function deletes a param at a specified index in
		PVisitedNetworkId header
** Parameters:
**			pHdr(IN/OUT)			- Sip PVisitedNetworkId Header
**			pParam(IN)			- The param to be deleted
**			dIndex(OUT)			_ The index at which the param is to be deleted
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPVisitedNetworkIdHdr _ARGS_((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

#endif

#ifdef SIP_CONGEST
/***********************************************************************
** Function:  sip_getNamespaceFromRsrcPriorityHdr
** Description: This function gets the Namespace field from the 
** RsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip RsrcPriority header
**			ppNamespace(OUT)	- The Namespace to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getNamespaceFromRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit **ppNamespace,SipError *pErr));

/***********************************************************************
** Function:  sip_getPriorityFromRsrcPriorityHdr
** Description: This function gets the Priority field from the 
** RsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip RsrcPriority header
**			ppPriority(OUT)	- The Priority to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getPriorityFromRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit **ppPriority,SipError *pErr));

/***********************************************************************
** Function:  sip_setNamespaceInRsrcPriorityHdr
** Description: This function sets the Namespace field in the 
** RsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip RsrcPriority header
**			ppNamespace(IN)	- The Namespace to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setNamespaceInRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit *pNamespace, SipError *pErr));

/***********************************************************************
** Function:  sip_setPriorityNamespaceInRsrcPriorityHdr
** Description: This function sets the Priority field in the 
** RsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip RsrcPriority header
**			ppPriority(IN)	- The Priority to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setPriorityInRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit *pPriority, SipError *pErr));
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
