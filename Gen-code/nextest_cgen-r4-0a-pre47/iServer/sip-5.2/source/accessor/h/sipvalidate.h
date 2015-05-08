/*
** THIS FILE IS USED INTERNALLY BY THE STACK
**  IT SHOULD NOT BE DIRECTLY INCLUDED BY THE APPLICATION	
*/

/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes off all SIP enumaeration  
**      and structure validating APIs.
**
*******************************************************************************
**
** FILENAME:
** 	sipvalidate.h
**
** DESCRIPTION:
**
** THIS FILE IS USED INTERNALLY BY THE STACK
**  IT SHOULD NOT BE DIRECTLY INCLUDED BY THE APPLICATION	
**
** DATE      NAME           REFERENCE      REASON
** ----      ----           ---------      ------
** 15Dec99   S.Luthra	    Creation
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/


#ifndef __SIP_VALIDATE_H_
#define __SIP_VALIDATE_H_

#include "sipstruct.h"
#include "portlayer.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#define SIP_9999_YEAR 9999 /* year 9999 used for comparison in .c file */



 SipBool validateSipAddrSpecType _ARGS_ ((en_AddrType type, SipError *err));

 SipBool validateSipExpiresType _ARGS_ ((en_ExpiresType type, SipError *err));

/* APIs No Longer Required. Must have a common validate API for these APIs */ 
/* SipBool validateSipUrlParamType _ARGS_ ((en_UrlParamType type, SipError *err));

 SipBool validateSipViaParamType _ARGS_ ((en_ViaParamType type, SipError *err));*/

 SipBool validateSipContactType _ARGS_ ((en_ContactType type, SipError *err));

 SipBool validateSipContactParamsType _ARGS_ ((en_ContactParamsType type, SipError *err));

 SipBool validateSipDayOfWeek _ARGS_ ((en_DayOfWeek day, SipError *err));

 SipBool validateTimeFormat _ARGS_ ((SipTimeFormat *time_format, SipError *err));

 SipBool validateDateFormat _ARGS_ ((SipDateFormat *date_format, SipError *err));

 SipBool validateDateStruct _ARGS_ ((SipDateStruct *date_struct, SipError *err));

 SipBool validateSipTransportParam _ARGS_ ((en_TransportParam param, SipError *err));

 SipBool validateSipUserParam _ARGS_ ((en_UserParam param, SipError *err));

 SipBool validateSipStatusCode _ARGS_ ((en_StatusCode code, SipError *err));

 SipBool validateSipHeaderType _ARGS_ ((en_HeaderType type, SipError *err));
 SipBool validateHideType _ARGS_((en_HideType *htype,SipError *err));
 SipBool validateMonth _ARGS_((en_Month *htype,SipError *err));
 SipBool validateSipDateFormat _ARGS_((SipDateFormat *date_format, SipError *err));
 SipBool validateSipTimeFormat _ARGS_((SipTimeFormat *time_format, SipError *err));
 SipBool sip_validateHeaderString(SIP_S8bit * str, en_HeaderType type, SipError *err);
SipBool sip_validateSipMsgBodyType _ARGS_((en_SipMsgBodyType type, SipError *pErr));

SipBool validateSipDateStruct _ARGS_((SipDateStruct *date_struct, SipError *err));



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
