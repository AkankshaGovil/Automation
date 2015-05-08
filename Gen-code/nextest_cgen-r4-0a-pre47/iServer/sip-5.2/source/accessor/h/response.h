/************************************************************
** FUNCTION:
**	This file contains the prototypes of the response header
** accessor APIs.
**
*************************************************************
**
** FILENAME:
**	resp1csb.h
**
** DESCRIPTION
**
**  DATE           NAME             REFERENCE
**  ---			  ------			----------
** 22nov99	B.Borthakur	            Original
**
** Copyright 199, Hughes Software Systems, Ltd.
**************************************************************/

#ifndef _SIP_RESP_H
#define _SIP_RESP_H

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
** Function:	sip_getCodeFromWarningHdr
** Description: get code number field from Warning Header
** Parameters:
**				hdr(IN)		- SipHeader
**				code(OUT) 	- Code number to retrieve
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getCodeFromWarningHdr _ARGS_(( SipHeader *hdr, \
		SIP_U16bit *code, SipError *err));

/***********************************************************************
** Function: sip_setCodeInWarningHdr
** Description: set code number field in Warning Header
** Parameters:
**				hdr(IN/OUT)		- SipHeader
**				code(IN) 		- Code number to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setCodeInWarningHdr _ARGS_(( SipHeader *hdr, \
		SIP_U16bit code, SipError *err));

/***********************************************************************
** Function: sip_getAgentFromWarningHdr
** Description: get agent field from Warning Header
** Parameters:
**				hdr(IN)		- SipHeader
**				agent(OUT) 	- Agent to retrieve
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getAgentFromWarningHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **agent, SipError *err));

/***********************************************************************
** Function: sip_setAgentInWarningHdr
** Description: set agent field in Warning Header
** Parameters:
**				hdr(IN/OUT)		- SipHeader
**				agent(IN) 		- Agent to  set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setAgentInWarningHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *agent, SipError *err));

/***********************************************************************
** Function: sip_getTextFromWarningHdr
** Description: get text field from Warning Header
** Parameters:
**				hdr(IN)		- SipHeader
**				text(OUT) 	- text to retrieve
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getTextFromWarningHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **text, SipError *err));

/***********************************************************************
** Function: sip_setTextInWarningHdr
** Description: get agent field from Warning Header
** Parameters:
**				hdr(IN/OUT)	- SipHeader
**				text(IN) 	- text to set
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setTextInWarningHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *text, SipError *err));

/***********************************************************************
** Function: sip_getDeltaSecondsFromMinExpiresHdr
** Description: get delta seconds field from MinExpires Header
** Parameters:
**				hdr(IN)			- Sip MinExpires Header
**				dseconds(OUT) 	- seconds to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getDeltaSecondsFromMinExpiresHdr _ARGS_(( SipHeader *hdr, \
		SIP_U32bit *dseconds, SipError *err));

/***********************************************************************
** Function: sip_setDeltaSecondsInMinExpiresHdr
** Description: set delta seconds in MinExpires header
** Parameters:
**				hdr(IN/OUT)	- Sip MinExpires Header
**				dseconds(IN)	- delta seconds to set
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setDeltaSecondsInMinExpiresHdr _ARGS_(( SipHeader *hdr, \
		SIP_U32bit dseconds, SipError *err));


#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getDateFromRetryAfterHdr
** Description: get date field from RetryAfter Header
** Parameters:
**				hdr(IN)		- Sip RetryAfter Header
**				date(OUT) 	- date to retrieve
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getDateFromRetryAfterHdr _ARGS_((SipHeader *hdr, \
		SipDateStruct **date, SipError *err)) ;

#else
/***********************************************************************
** Function: sip_getDateFromRetryAfterHdr
** Description: get date field from RetryAfter Header
** Parameters:
**				hdr(IN)		- Sip RetryAfter Header
**				date(OUT) 	- date to retrieve
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getDateFromRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SipDateStruct *date, SipError *err));

#endif
/***********************************************************************
** Function: sip_setDateInRetryAfterHdr
** Description: set date in RetryAfter header
** Parameters:
**				hdr(IN/OUT)	- Sip RetryAfter Header
**				date(IN) 	- date to set
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setDateInRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SipDateStruct *date, SipError *err));

/***********************************************************************
** Function: sip_getDeltaSecondsFromRetryAfterHdr
** Description: get delta seconds field from RetryAfter Header
** Parameters:
**				hdr(IN)			- Sip RetryAfter Header
**				dseconds(OUT) 	- seconds to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getDeltaSecondsFromRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SIP_U32bit *dseconds, SipError *err));

/***********************************************************************
** Function: sip_setDeltaSecondsInRetryAfterHdr
** Description: set delta seconds in RetryAfter header
** Parameters:
**				hdr(IN/OUT)	- Sip RetryAfter Header
**				dseconds(IN)	- delta seconds to set
**				err(OUT)    - Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setDeltaSecondsInRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SIP_U32bit dseconds, SipError *err));

/***********************************************************************
** Function: sip_getCommentFromRetryAfterHdr
** Description: get Comment field from RetryAfter Header
** Parameters:
**				hdr(IN)			- Sip RetryAfter Header
**				comment(OUT) 	- Comment to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getCommentFromRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **comment, SipError *err));

/***********************************************************************
** Function: sip_setCommentInRetryAfterHdr
** Description: set Comment field in RetryAfter Header
** Parameters:
**				hdr(IN/OUT)		- Sip RetryAfter Header
**				comment(IN) 	- Comment to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setCommentInRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *comment, SipError *err));

/***********************************************************************
** Function: sip_getDurationFromRetryAfterHdr
** Description: get duration field from RetryAfter Header
** Parameters:
**				hdr(IN)			- Sip RetryAfter Header
**				duration(OUT) 	- duration to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
/* extern SipBool sip_getDurationFromRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SIP_U32bit *duration, SipError *err));
*/
/***********************************************************************
** Function: sip_setDurationInRetryAfterHdr
** Description: set duration field in RetryAfter Header
** Parameters:
**				hdr(IN/OUT)		- Sip RetryAfter Header
**				duration(IN) 	- duration to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
/* extern SipBool sip_setDurationInRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		SIP_U32bit duration, SipError *err));
*/
/***********************************************************************
** Function:  sip_getParamCountFromRetryAfterHdr
** Description: Retrieve the number of parameters in a RetryAfter header
** Parameters:
**				pHdr(IN/OUT)		- Sip RetryAfter Header
**				pCount(IN) 		- The number of parameters in RetryAfter header
**				pErr(OUT)    		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getParamCountFromRetryAfterHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromRetryAfterHdr
** Description: Retrieves the param at a specified index from a Retry
		After header
** Parameters:
**				pHdr(IN)		- Sip RetryAfter Header
**				pParam/ppParam(OUT)	_ The retrieved param
**				DIndex(IN) 		- The index of the param to be retrieved
**				pErr(OUT)    		- Possible error value (See API ref doc)
************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromRetryAfterHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromRetryAfterHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInRetryAfterHdr
** Description: Sets a param at a specified index in a Retry
		After header
** Parameters:
**				pHdr(IN/OUT)		- Sip RetryAfter Header
**				pParam(IN)		_ The param to be set
**				DIndex(IN) 		- The index at which the param to be set
**				pErr(OUT)    		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setParamAtIndexInRetryAfterHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInRetryAfterHdr
** Description: Inserts a param at a specified index in a Retry
		After header
** Parameters:
**				pHdr(IN/OUT)		- Sip RetryAfter Header
**				pParam(IN)		_ The param to be inserted
**				DIndex(IN) 		- The index at which the param to be inserted
**				pErr(OUT)    		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_insertParamAtIndexInRetryAfterHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInRetryAfterHdr
** Description: Deletes a param at a specified index in a Retry
		After header
** Parameters:
**				pHdr(IN/OUT)		- Sip RetryAfter Header
**				DIndex(IN) 		- The index at which the param to be deleted
**				pErr(OUT)    		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_deleteParamAtIndexInRetryAfterHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_getOptionFromUnsupportedHdr
** Description: get option field from Unsupported Header
** Parameters:
**				hdr(IN)			- Sip Unsupported Header
**				ppoption(OUT) 	- option to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getOptionFromUnsupportedHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit **ppOption, SipError *pErr));

/***********************************************************************
** Function: sip_setOptionInUnsupportedHdr
** Description: set option field in Unsupported Header
** Parameters:
**				hdr(IN/OUT)		- Sip Unsupported Header
**				poption(IN) 	- option to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setOptionInUnsupportedHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit *pOption, SipError *pErr));
/***********************************************************************
** Function: sip_getMethodFromAllowHdr
** Description: get Method field from Allow Header
** Parameters:
**				hdr(IN)			- Sip Allow Header
**				method(OUT) 	- method to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getMethodFromAllowHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **method, SipError *err));

/***********************************************************************
** Function: sip_setMethodInAllowHdr
** Description: set Method field in Allow Header
** Parameters:
**				hdr(IN/OUT)		- Sip Allow Header
**				method(IN) 		- method to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setMethodInAllowHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit  *method, SipError *err));



/***********************************************************************
** Function: sip_getValueFromServerHdr
** Description: get value field from Server Header
** Parameters:
**				hdr(IN)			- Sip Server Header
**				ppValue(OUT) 	- Value to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getValueFromServerHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit **ppValue, SipError *pErr));

/***********************************************************************
** Function: sip_setValueInServerHdr
** Description: set value field in Server Header
** Parameters:
**				hdr(IN/OUT)		- Sip Server Header
**				pValue(IN) 		- value to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setValueInServerHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit *pValue, SipError *pErr));


#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getChallengeFromWwwAuthenticateHdr
** Description: get Challenge field from WWWAuthenticate Header
** Parameters:
**				hdr(IN)			- Sip WWWAuthenticate Header
**				challenge(OUT) 	- Challenge to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getChallengeFromWwwAuthenticateHdr _ARGS_(( SipHeader *hdr, \
		SipGenericChallenge **challenge, SipError *err));

#else
/***********************************************************************
** Function: sip_getChallengeFromWwwAuthenticateHdr
** Description: get Challenge field from WWWAuthenticate Header
** Parameters:
**				hdr(IN)			- Sip WWWAuthenticate Header
**				challenge(OUT) 	- Challenge to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getChallengeFromWwwAuthenticateHdr _ARGS_(( SipHeader *hdr, \
		SipGenericChallenge *challenge, SipError *err));

#endif
/***********************************************************************
** Function: sip_setChallengeInWwwAuthenticateHdr
** Description: set Challenge field in WWWAuthenticate Header
** Parameters:
**				hdr(IN/OUT)		- Sip WWWAuthenticate Header
**				challenge(IN) 	- Challenge to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setChallengeInWwwAuthenticateHdr _ARGS_(( SipHeader *hdr, \
		SipGenericChallenge *challenge, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getChallengeFromProxyAuthenticateHdr
** Description: get Challenge field from ProxyAuthenticate Header
** Parameters:
**				hdr(IN)			- Sip ProxyAuthenticate Header
**				challenge(OUT) 	- Challenge to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getChallengeFromProxyAuthenticateHdr _ARGS_(( SipHeader *hdr, \
		SipGenericChallenge **challenge, SipError *err));

#else
/***********************************************************************
** Function: sip_getChallengeFromProxyAuthenticateHdr
** Description: get Challenge field from ProxyAuthenticate Header
** Parameters:
**				hdr(IN)			- Sip ProxyAuthenticate Header
**				challenge(OUT) 	- Challenge to retrieve
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getChallengeFromProxyAuthenticateHdr _ARGS_(( SipHeader *hdr, \
		SipGenericChallenge *challenge, SipError *err));

#endif
/***********************************************************************
** Function: sip_setChallengeInProxyAuthenticateHdr
** Description: set Challenge field in ProxyAuthenticate Header
** Parameters:
**				hdr(IN/OUT)		- Sip ProxyAuthenticate Header
**				challenge(IN) 	- Challenge to set
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setChallengeInProxyAuthenticateHdr _ARGS_(( SipHeader *hdr, \
		SipGenericChallenge *challenge, SipError *err));

/***********************************************************************
** Function: sip_getTypeFromRetryAfterHdr
** Description: get type of Retry After Header
** Parameters:
**				hdr(IN)			- SipHeader
**				type(OUT) 		- Retrieve type of RetryAfter header
**				err(OUT)    	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getTypeFromRetryAfterHdr _ARGS_(( SipHeader *hdr, \
		en_ExpiresType *type, SipError *err));

/***********************************************************************
** Function: sip_getUriFromErrorInfoHdr
** Description:gets the Uri field from the Sip ErrorInfo Header
** Parameters:
**			hdr(IN) 				- Sip ErrorInfo Header
**			ppUri(OUT)				- The Uri field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getUriFromErrorInfoHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **ppUri, SipError *pErr));


/***********************************************************************
** Function: sip_setUriInErrorInfoHdr
** Description:sets the Uri field in the Sip ErrorInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ErrorInfo Header
**			pUri(IN)				- The Uri field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setUriInErrorInfoHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *pUri, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromErrorInfoHdr
** Description:gets the number of Parameters from the Sip ErrorInfo Header
** Parameters:
**			hdr(IN) 			- Sip ErrorInfo Header
**			count(OUT)			- The Parameter count to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromErrorInfoHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromErrorInfoHdr
** Description:gets the Parameters at an index from the Sip ErrorInfo Header
** Parameters:
**			hdr(IN) 			- Sip ErrorInfo Header
**			pParam(OUT)			- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromErrorInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromErrorInfoHdr
** Description:gets the Parameter at an index from the Sip ErrorInfo Header
** Parameters:
**			hdr(IN) 			- Sip ErrorInfo Header
**			ppParam(OUT)		- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromErrorInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam **ppParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertParamAtIndexInErrorInfoHdr
** Description:inserts the Parameter at an index in the Sip ErrorInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ErrorInfo Header
**			pParam(IN)			- The Parameter to insert
**			index(IN)			- The index at which the Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInErrorInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteParamAtIndexInErrorInfoHdr
** Description:deletes the Parameter at an index in the Sip ErrorInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ErrorInfo Header
**			index(IN)			- The index at which the Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInErrorInfoHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setParamAtIndexInErrorInfoHdr
** Description:sets the Parameter at an index in the Sip ErrorInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ErrorInfo Header
**			pParam(IN)			- The Parameter to set
**			index(IN)			- The index at which the Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInErrorInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


extern SipBool sip_getNameValuePairCountFromAuthenticationInfoHdr _ARGS_( (\
	SipHeader *pAuthInfo, SIP_U32bit *dCount,SipError *pErr ));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_getNameValuePairAtIndexFromAuthenticationInfoHdr _ARGS_( (\
	SipHeader *pAuthInfo, SipNameValuePair **ppNameValue,\
	SIP_U32bit index, SipError *err ));
#else
extern SipBool sip_getNameValuePairAtIndexFromAuthenticationInfoHdr _ARGS_( (\
	SipHeader *pAuthInfo, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));
#endif

extern SipBool sip_setNameValuePairAtIndexInAuthenticationInfoHdr _ARGS_( (\
	SipHeader *pAuthInfo, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));

extern SipBool sip_insertNameValuePairAtIndexInAuthenticationInfoHdr _ARGS_( (\
	SipHeader *pAuthInfo, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));

extern SipBool sip_deleteNameValuePairAtIndexInAuthenticationInfoHdr _ARGS_( (\
	SipHeader *pAuthInfo, SIP_U32bit index, \
	SipError *err ));

#ifdef SIP_3GPP
/***********************************************************************
** Function: sip_getDispNameFromPAssociatedUriHdr 
** Description: gets the Display Name from the PAssociatedUri Header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			dsipName(OUT)		- The DisplayName retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromPAssociatedUriHdr	_ARGS_ ((SipHeader *pHdr,SIP_S8bit **ppDispname, SipError *pErr));

/***********************************************************************
** Function: sip_setDispNameInPAssociatedUriHdr 
** Description: gets the Display Name from the PAssociatedUri Header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			dsipName(IN)		- The DisplayName to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInPAssociatedUriHdr _ARGS_ ((SipHeader *pHdr,SIP_S8bit *pDispname, SipError *pErr));

/***********************************************************************
** Function: sip_setAddrSpecInPAssociatedUriHdr 
** Description: sets the AddrSpec in the PAssociatedUri Header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			pAddrSpec(IN)		- The AddrSpec to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInPAssociatedUriHdr _ARGS_ ((SipHeader *pHdr,SipAddrSpec *pAddrSpec, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamCountFromPAssociatedUriHdr
** Description: This function retrieves the number of params in a PAssociatedUri
**		header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			pCount(OUT)			- The number of parameters
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromPAssociatedUriHdr _ARGS_ ((SipHeader *pHdr,SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromPAssociatedUriHdr 
** Description: gets the AddrSpec from the PAssociatedUri Header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			ppAddrSpec(OUT)		- The AddrSpec to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromPAssociatedUriHdr	_ARGS_ ((SipHeader *pHdr,SipAddrSpec **ppAddrSpec, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromPAssociatedUriHdr
** Description: This function retrieves a param at a specified index in
		PAssociatedUri header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPAssociatedUriHdr _ARGS_((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromPAssociatedUriHdr 
** Description: gets the AddrSpec from the PAssociatedUri Header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			pAddrSpec(OUT)		- The AddrSpec to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromPAssociatedUriHdr	_ARGS_ ((SipHeader *pHdr,SipAddrSpec *pAddrSpec, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromPAssociatedUriHdr
** Description: This function retrieves a param at a specified index in
		PAssociatedUri header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPAssociatedUriHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInPAssociatedUriHdr
** Description: This function sets a param at a specified index in
		PAssociatedUri header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInPAssociatedUriHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInPAssociatedUriHdr
** Description: This function sets a param at a specified index in
		PAssociatedUri header
** Parameters:
**			pHdr(IN)			- Sip PAssociatedUri Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInPAssociatedUriHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInPAssociatedUriHdr
** Description: This function deletes a param at a specified index in
		PAssociatedUri header
** Parameters:
**			pHdr(IN/OUT)			- Sip PAssociatedUri Header
**			pParam(IN)			- The param to be deleted
**			dIndex(OUT)			_ The index at which the param is to be deleted
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPAssociatedUriHdr _ARGS_((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

#endif

#ifdef SIP_CONGEST

/***********************************************************************
** Function:  sip_getNamespaceFromAcceptRsrcPriorityHdr
** Description: This function gets the Namespace field from the 
** AcceptRsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip AcceptRsrcPriority header
**			ppNamespace(OUT)	- The Namespace to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getNamespaceFromAcceptRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit **ppNamespace,SipError *pErr));

/***********************************************************************
** Function:  sip_getPriorityFromAcceptRsrcPriorityHdr
** Description: This function gets the Priority field from the 
** AcceptRsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip AcceptRsrcPriority header
**			ppPriority(OUT)	- The Priority to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getPriorityFromAcceptRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit **ppPriority,SipError *pErr));

/***********************************************************************
** Function:  sip_setNamespaceInAcceptRsrcPriorityHdr
** Description: This function sets the Namespace field in the 
** AcceptRsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip AcceptRsrcPriority header
**			ppNamespace(IN)	- The Namespace to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setNamespaceInAcceptRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit *pNamespace, SipError *pErr));

/***********************************************************************
** Function:  sip_setPriorityNamespaceInAcceptRsrcPriorityHdr
** Description: This function sets the Priority field in the 
** AcceptRsrcPriority header
** 
** Parameters:
**			pHdr(IN/OUT)		- Sip AcceptRsrcPriority header
**			ppPriority(IN)	- The Priority to be set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setPriorityInAcceptRsrcPriorityHdr _ARGS_((SipHeader *pHdr,SIP_S8bit *pPriority, SipError *pErr));
#endif

#ifdef SIP_SECURITY
/***********************************************************************
** Function: sip_getMechanismNameFromSecurityServerHdr
** Description: gets the Mechanism Name from the Security-Server Header
** Parameters:
**                      pHdr(IN)                        - Sip Security-Server Header
**                      ppMechname(OUT)                 - The DisplayName retrieved
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getMechanismNameFromSecurityServerHdr _ARGS_((SipHeader *hdr,\
                SIP_S8bit **ppMechname, SipError *err));

/***********************************************************************
** Function: sip_setMechanismNameInSecurityServerHdr
** Description: sets the Display Name in the Security-Server Header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Server Header
**                      pMechname(IN)                   - The DisplayName to set
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setMechanismNameInSecurityServerHdr _ARGS_((SipHeader *hdr,\
                SIP_S8bit *pMechname, SipError *err));

/***********************************************************************
** Function:  sip_getParamCountFromSecurityServerHdr
** Description: This function retrieves the number of params in a Security-Server
**              header
** Parameters:
**                      pHdr(IN)                        - Sip Security-Server Header
**                      pCount(OUT)                     - The number of parameters
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromSecurityServerHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromSecurityServerHdr
** Description: This function retrieves a param at a specified index in
                Security-Server header
** Parameters:
**                      pHdr(IN)                        - Sip Security-Server Header
**                      pParam/ppParam(OUT)             - The retrieved param
**                      dIndex (IN)                     _ The index of the param to be retrieved
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromSecurityServerHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromSecurityServerHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex,SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInSecurityServerHdr
** Description: This function sets a param at a specified index in
                Security-Server header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Server Header
**                      pParam(IN)                      - The param to be set
**                      dIndex(OUT)                     _ The index at which the param is to be set
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInSecurityServerHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInSecurityServerHdr
** Description: This function inserts a param at a specified index in
                Security-Server header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Server Header
**                      pParam(IN)                      - The param to be inserted
**                      dIndex(OUT)                     _ The index at which the param is to be inserted
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInSecurityServerHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInSecurityServerHdr
** Description: This function deletes a param at a specified index in
                Security-Server header
** Parameters:
**                      pHdr(IN/OUT)                    - Sip Security-Server Header
**                      pParam(IN)                      - The param to be deleted
**                      dIndex(OUT)                     _ The index at which the param is to be deleted
**                      pErr(OUT)                       - Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInSecurityServerHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

#endif /* end of #ifdef SIP_SECURITY */


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
