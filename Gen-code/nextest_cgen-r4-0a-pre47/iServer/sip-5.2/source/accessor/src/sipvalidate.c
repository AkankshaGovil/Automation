/******************************************************************************
** FUNCTION:
** 	This pHeader file contains the source dCodeNum of all SIP enumaeration
**      and structure validating APIs.
**
*******************************************************************************
**
** FILENAME:
** 	sipvalidate.c
**
** DESCRIPTION:
**
**
** DATE      NAME           REFERENCE      REASON
** ----      ----           ---------      ------
** 15Dec99   S.Luthra	    Creation
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/
#include "sipvalidate.h"
#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#ifdef SIP_DCS
#include "dcsintrnl.h"
#endif




/*********************************************************
** FUNCTION:validateHideType
**
** DESCRIPTION:  This fucntion returns SipSuccess
** if "htype" is one among the defined en_HideType's
** else it returns SipFail.
**
**********************************************************/
SipBool validateHideType
#ifdef ANSI_PROTO
	(en_HideType *htype,
	SipError *err)
#else
	( htype, err )
	   en_HideType *htype;
	  SipError * err;
#endif
{
	SIPDEBUGFN("Entering function validateHideType");
	switch(*htype)
	{
		case SipHideRoute:
		case SipHideHop :
				*err=E_NO_ERROR;
					SIPDEBUGFN("Exiting function validateHideType");
				return SipSuccess;
		default:
			*err = E_INV_PARAM;
			SIPDEBUGFN("Exiting function validateHideType");
			return SipFail;

	}
}

/***************************************************************
** FUNCTION: validateUrlParamType
**
** DESCRIPTION: This fucntion returns SipSuccess if "htype"
** is one among the defined en_UrlParamType's else it returns SipFail.
**
***************************************************************/
/*
SipBool validateUrlParamType
#ifdef ANSI_PROTO
	(en_UrlParamType *htype,
	SipError *err)
#else
	( htype, err )
	en_UrlParamType *htype;
	  SipError * err;
#endif
{
	switch(*htype)
	{
	case 	SipUrlParamTransport:
	case 	SipUrlParamUser:
	case 	SipUrlParamMethod:
	case 	SipUrlParamTtl:
	case 	SipUrlParamMaddr:
	case 	SipUrlParamOther:
	case 	SipUrlParamAny:
				*err=E_NO_ERROR;
				return SipSuccess;
		default:
			*err = E_INV_PARAM;
			return SipFail;

	}
}
*/

/*********************************************************
** FUNCTION:validateMonth
**
** DESCRIPTION:  This fucntion returns SipSuccess if
** "htype" is one among the defined en_Month types else it
** returns SipFail.
**
**********************************************************/
SipBool validateMonth
#ifdef ANSI_PROTO
	(en_Month *htype,
	SipError *err)
#else
	( htype, err )
	   en_Month *htype;
	  SipError * err;
#endif
{
	switch(*htype)
	{
	case SipMonthJan:
	case SipMonthFeb:
	case SipMonthMar:
	case SipMonthApr:
	case SipMonthMay:
	case SipMonthJun:
	case SipMonthJul:
	case SipMonthAug:
	case SipMonthSep:
	case SipMonthOct:
	case SipMonthNov:
	case SipMonthDec:
			*err=E_NO_ERROR;
			return SipSuccess;
		default:
			*err = E_INV_PARAM;
			return SipFail;

	}
}

/********************************************************************
**
** FUNCTION:  validateSipAddrSpecType
**
**DESCRIPTION: This function returns SipSuccess if "dType"
** is among the defined en_AddType's ; else it returns SipFail;
**
********************************************************************/

SipBool validateSipAddrSpecType
#ifdef ANSI_PROTO
	(en_AddrType dType, SipError *err)
#else
	(dType, err)
	en_AddrType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipAddrSpecType");
	switch (dType)
	{
		case	SipAddrSipUri	:
		case	SipAddrSipSUri	:
		case	SipAddrReqUri	:
		case	SipAddrAny	:break;
		default			:*err = E_INV_TYPE;
								SIPDEBUGFN("Exiting function validateSipAddrSpecType");
					      return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipAddrSpecType");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipTimeFormat
**
**DESCRIPTION:  This function  validates the pValue of "time_format".
** It returns SipSuccess if dHour is less than 24, minute is less
** than 59 and second is lesss than 59; else it returns SipFail;
**
********************************************************************/

SipBool validateSipTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *time_format, SipError *err)
#else
	(time_format, err)
	SipTimeFormat *time_format;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipTimeFormat");
	if (time_format->dHour >SIP_23_HRS)
	{
		*err = E_INV_PARAM;
	  SIPDEBUGFN("Exiting function validateSipTimeFormat");
		return SipFail;
	}
	if (time_format->dMin >SIP_59_SECS)
	{
		*err = E_INV_PARAM;
	  SIPDEBUGFN("Exiting function validateSipTimeFormat");
		return SipFail;
	}
	if (time_format->dSec >SIP_59_SECS)
	{
		*err = E_INV_PARAM;
	  SIPDEBUGFN("Exiting function validateSipTimeFormat");
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipTimeFormat");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipDateFormat
**
** DESCRIPTION:  This function validates the values of the
** "date_format" structure fileds.
**
********************************************************************/

SipBool validateSipDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *date_format, SipError *err)
#else
	(date_format, err)
	SipDateFormat *date_format;
	SipError *err;
#endif
{
	SIP_S8bit days_in_month[SIP_NO_MONTHS] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	SIPDEBUGFN("Entering function validateSipDateFormat");
	if (date_format->dYear > SIP_9999_YEAR)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( (date_format->dYear)%4 != 0)
		days_in_month[1] = 28;
	else if ( (date_format->dYear)%400 == 0)
		days_in_month[1] = 29;
	else if ( (date_format->dYear)%100 == 0)
		days_in_month[1] = 28;
	else
		days_in_month[1] = 29;

	switch(date_format->dMonth)
	{
		case	SipMonthJan	:
		case	SipMonthFeb	:
		case	SipMonthMar	:
		case 	SipMonthApr	:
		case	SipMonthMay	:
		case	SipMonthJun	:
		case	SipMonthJul	:
		case	SipMonthAug	:
		case	SipMonthSep	:
		case	SipMonthOct	:
		case	SipMonthNov	:
		case	SipMonthDec	:break;
		default			:*err = E_INV_PARAM;
					 return SipFail;

	}
	if (date_format->dDay > days_in_month[date_format->dMonth])
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipDateFormat");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipDateStruct
**
** DESCRIPTION:  This function validates the field values of 'date_struct"
** structure.
**
********************************************************************/

SipBool validateSipDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct *date_struct, SipError *err)
#else
	(date_struct, err)
	SipDateStruct *date_struct;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateDateStruct");
	switch (date_struct->dDow)
	{
		case	SipDaySun	:
		case	SipDayMon	:
		case	SipDayTue	:
		case	SipDayWed	:
		case	SipDayThu	:
		case	SipDayFri	:
		case	SipDaySat	:
		case	SipDayNone	:break;
		default			:*err = E_INV_PARAM;
					 return SipFail;

	}
	if ( validateSipDateFormat((date_struct->pDate), err) == SipFail )
		return SipFail;
	if ( validateSipTimeFormat((date_struct->pTime), err) == SipFail )
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateDateStruct");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipExpiresType
**
** DESCRIPTION:  This function validates the pValue of "dType"
**
********************************************************************/

SipBool validateSipExpiresType
#ifdef ANSI_PROTO
	(en_ExpiresType dType, SipError *err)
#else
	(dType, err)
	en_ExpiresType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipExpiresType");
	switch (dType)
	{
		case	SipExpDate		:
		case	SipExpSeconds	:
		case	SipExpAny		: break;
		default					:*err = E_INV_TYPE;
					 			return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipExpiresType");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipContactParamsType
**
** DESCRIPTION:  This fucntion returns SipSuccess if "dType"
** is one among the defined en_ContactParamsType's; else it returns SipFail.
**
********************************************************************/

SipBool validateSipContactParamsType
#ifdef ANSI_PROTO
	(en_ContactParamsType dType, SipError *err)
#else
	(dType, err)
	en_ContactParamsType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipContactParamsType");
	switch(dType)
	{
		case	SipCParamQvalue		:
		case	SipCParamExpires	:
		case	SipCParamExtension	:
		case	SipCParamFeatureParam   :
		case	SipCParamAny		:
						 break;
		default				:*err = E_INV_TYPE;
						 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipContactParamsType");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipUrlParamType
**
**DESCRIPTION:  This fucntion returns SipSuccess if "dType"
** is one among the defined en_UrlParamType's; else it returns SipFail.
**
********************************************************************/

/*
SipBool validateSipUrlParamType
#ifdef ANSI_PROTO
	(en_UrlParamType dType, SipError *err)
#else
	(dType, err)
	en_UrlParamType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipUrlParamType");
	switch (dType)
	{
		case	SipUrlParamTransport	:
		case	SipUrlParamUser		:
		case	SipUrlParamMethod	:
		case	SipUrlParamTtl		:
		case	SipUrlParamMaddr	:
		case	SipUrlParamOther	:
		case	SipUrlParamAny		:break;
		default				:*err = E_INV_TYPE;
						 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipUrlParamType");
	return SipSuccess;
}

*/

/********************************************************************
**
** FUNCTION:  validateSipTransportParam
**
**DESCRIPTION:  This fucntion returns SipSuccess if "param" is one
** among the defined en_TransportParam types; else it returns SipFail.
**
********************************************************************/

SipBool validateSipTransportParam
#ifdef ANSI_PROTO
	(en_TransportParam param, SipError *err)
#else
	(param, err)
	en_TransportParam param;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipTransportParam");
	switch(param)
	{
		case	SipTranspTcp	:
		case	SipTranspUdp	:break;
		default			:*err = E_INV_TYPE;
					 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipTransportParam");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipUserParam
**
**DESCRIPTION:  This fucntion returns SipSuccess if "param" is
**one among the defined en_UserParam types; else it returns SipFail.
**
********************************************************************/

SipBool validateSipUserParam
#ifdef ANSI_PROTO
	(en_UserParam param, SipError *err)
#else
	(param, err)
	en_UserParam param;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipUserParam");
	switch(param)
	{
		case	SipUserParamPhone	:
		case	SipUserParamIp		:break;
		default				:*err = E_INV_TYPE;
						 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipUserParam");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipViaParamType
**
** DESCRIPTION:  This fucntion returns SipSuccess if "dType"
**is one among the defined en_ViaParamType's; else it returns SipFail.
**
********************************************************************/

/*
SipBool validateSipViaParamType
#ifdef ANSI_PROTO
	(en_ViaParamType dType, SipError *err)
#else
	(dType, err)
	en_ViaParamType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipViaParamType");
	switch(dType)
	{
		case	SipViaParamHidden	:
		case	SipViaParamTtl		:
		case	SipViaParamReceived	:
		case	SipViaParamBranch	:
		case	SipViaParamMaddr	:
		case	SipViaParamExtension	:
		case	SipViaParamAny:		 break;
		default				:*err = E_INV_TYPE;
						 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipViaParamType");
	return SipSuccess;
}
*/

/********************************************************************
**
** FUNCTION:  validateSipContactType
**
** DESCRIPTION:   This fucntion returns SipSuccess if "dType" is
** one among the defined en_ContactType's; else it returns SipFail.
**
********************************************************************/

SipBool validateSipContactType
#ifdef ANSI_PROTO
	(en_ContactType dType, SipError *err)
#else
	(dType, err)
	en_ContactType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipContactType");
	switch (dType)
	{
		case 	SipContactWildCard	:
		case	SipContactNormal	:
		case	SipContactAny		: break;
		default				:*err = E_INV_TYPE;
						 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipContactType");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipDayOfWeek
**
** DESCRIPTION:   This fucntion returns SipSuccess if "dDay" is
** one among the defined en_DayOfWeek types; else it returns SipFail.
**
********************************************************************/

SipBool validateSipDayOfWeek
#ifdef ANSI_PROTO
	(en_DayOfWeek dDay, SipError *err)
#else
	(dDay, err)
	en_DayOfWeek dDay;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipDayOfweek");
	switch (dDay)
	{
		case 	SipDaySun	:
		case	SipDayMon	:
		case	SipDayTue	:
		case	SipDayWed	:
		case	SipDayThu	:
		case	SipDayFri	:
		case	SipDaySat	:
		case	SipDayNone	:break;
		default			:*err = E_INV_PARAM;
					 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipDayOfweek");
	return SipSuccess;

}


/********************************************************************
**
** FUNCTION:  validateSipStatusCode
**
** DESCRIPTION:  This fucntion returns SipSuccess if "dCodeNum" is
** one among the defined en_StatusCode types; else it returns SipFail.
**
********************************************************************/

SipBool validateSipStatusCode
#ifdef ANSI_PROTO
	(en_StatusCode dCodeNum, SipError *err)
#else
	(dCodeNum, err)
	en_StatusCode dCodeNum;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipStatusCode");
	switch(dCodeNum)
	{
		case	SipStatusTrying	 	:
		case	SipStatusRinging	:
		case	SipStatusForwarded	:
		case	SipStatusQueued	 	:
		case	SipStatusSessionProgress:
		case	SipStatusOk	 	:
		case	SipStatusMultiChoice 	:
		case	SipStatusMovedPerm 	:
		case	SipStatusMovedTemp 	:
		case	SipStatusSeeOther 	:
		case	SipStatusUseProxy 	:
		case	SipStatusAltService 	:
		case	SipStatusBadRequest 	:
		case	SipStatusUnauthorized 	:
		case	SipStatusPaymentReq 	:
		case	SipStatusForbidden 	:
		case	SipStatusNotFound 	:
		case	SipStatusMethNotAllowed	:
		case	SipStatusNotAcceptable 	:
		case	SipStatusProxyAuthReq 	:
		case	SipStatusRequestTimeout	:
		case	SipStatusConflict 	:
		case	SipStatusGone	 	:
		case	SipStatusLengthReq 	:
		case	SipStatusReqEntTooLarge	:
		case 	SipStatusReqUriTooLarge	:
		case	SipStatusUnsuppMediaType:
		case	SipStatusBadExtension 	:
		case	SipStatusTempNotAvail 	:
		case	SipStatusCallNotExist 	:
		case	SipStatusLoopDetected 	:
		case	SipStatusTooManyHops 	:
		case	SipStatusAddressIncomp 	:
		case	SipStatusAmbiguous 	:
		case	SipStatusBusyHere 	:
		case	SipStatusRequestTerminated	:
		case	SipStatusInternalSrvErr	:
		case	SipStatusNotImplemented	:
		case	SipStatusBadGateway	:
		case	SipStatusServiceUnavail	:
		case	SipStatusGwTimeout 	:
		case	SipStatusSipVerNotSupp 	:
		case	SipStatusBusyEveryWhere	:
		case	SipStatusDecline	:
		case	SipStatusDoesNotExist 	:
		case	SipStatusGlobNotAcceptable:
		case	SipUnknownStatus 	:break;
		default			 	:*err = E_INV_TYPE;
					 	 return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipStatusCode");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  validateSipHeaderType
**
** DESCRIPTION:  This fucntion returns SipSuccess if "dType" is one
** among the defined en_HeaderType's else it returns SipFail.
**
********************************************************************/

SipBool validateSipHeaderType
#ifdef ANSI_PROTO
	(en_HeaderType dType, SipError *err)
#else
	(dType, err)
	en_HeaderType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function validateSipHeaderType");
	switch(dType)
	{
		case	SipHdrTypeAccept		:
		case	SipHdrTypeAcceptEncoding	:
		case	SipHdrTypeAcceptLanguage	:
		case	SipHdrTypeCallId		:
		case	SipHdrTypeCseq			:
		case	SipHdrTypeDate			:
		case	SipHdrTypeEncryption		:
		case	SipHdrTypeExpiresDate		:
		case	SipHdrTypeExpiresSec		:
		case	SipHdrTypeExpiresAny		:
		case	SipHdrTypeFrom			:
		case	SipHdrTypeRecordRoute		:
#ifdef SIP_3GPP
		case	SipHdrTypeServiceRoute	:
		case	SipHdrTypePath		:
		case    SipHdrTypePanInfo   :
		case    SipHdrTypePcVector  :
#endif
		case	SipHdrTypeTimestamp		:
		case	SipHdrTypeTo			:
		case	SipHdrTypeVia			:
		case	SipHdrTypeContentEncoding	:
		case	SipHdrTypeContentLength		:
		case	SipHdrTypeContentType		:
		case	SipHdrTypeAuthorization		:
		case	SipHdrTypeContactNormal		:
		case	SipHdrTypeContactWildCard	:
		case	SipHdrTypeContactAny		:
		case	SipHdrTypeHide			:
		case	SipHdrTypeMaxforwards		:
		case	SipHdrTypeOrganization		:
		case	SipHdrTypePriority		:
		case	SipHdrTypeProxyauthorization		:
		case	SipHdrTypeProxyRequire		:
		case	SipHdrTypeRoute			:
		case	SipHdrTypeRequire		:
		case	SipHdrTypeResponseKey		:
		case	SipHdrTypeSubject		:
		case	SipHdrTypeUserAgent		:
		case	SipHdrTypeAllow			:
		case	SipHdrTypeProxyAuthenticate	:
		case	SipHdrTypeRetryAfterDate	:
		case	SipHdrTypeRetryAfterSec		:
		case	SipHdrTypeRetryAfterAny		:
		case	SipHdrTypeServer		:
		case	SipHdrTypeUnsupported		:
		case	SipHdrTypeWarning		:
		case	SipHdrTypeWwwAuthenticate	:
		case	SipHdrTypeAuthenticationInfo	:
		case	SipHdrTypeUnknown		:
		case	SipHdrTypeRSeq			: /* Retrans */
		case	SipHdrTypeRAck			: /* Retrans */
		case 	SipHdrTypeSupported		: /* RPR Retrans*/
		case 	SipHdrTypeMimeVersion		: /* bcpt ext */
		case	SipHdrTypeContentDisposition:
		case	SipHdrTypeReason			:
		case	SipHdrTypeCallInfo			:
		case	SipHdrTypeErrorInfo			:
		case	SipHdrTypeAlertInfo			:
		case	SipHdrTypeInReplyTo			:
		case	SipHdrTypeContentLanguage	:
		case 	SipHdrTypeReferTo			:
		case 	SipHdrTypeReferredBy		:
		case 	SipHdrTypeReplyTo		:
		case 	SipHdrTypeReplaces		:
		case 	SipHdrTypeMinExpires	:
#ifdef SIP_CCP
		case	SipHdrTypeAcceptContact		: /* CCP */
		case	SipHdrTypeRejectContact		: /* CCP */
		case	SipHdrTypeRequestDisposition: /* CCP */
#endif
#ifdef SIP_IMPP
		case	SipHdrTypeEvent				:
		case	SipHdrTypeAllowEvents		:
		case	SipHdrTypeSubscriptionState	:
#endif
#ifdef SIP_SESSIONTIMER
		case 	SipHdrTypeMinSE			:
		case	SipHdrTypeSessionExpires:
#endif
		case	SipHdrTypeAlso		:
#ifdef SIP_PRIVACY
		case 	SipHdrTypePrivacy	: /* privacy header type*/
		case    SipHdrTypePAssertId     :
		case    SipHdrTypePPreferredId     :
# endif /* ifdef SIP_PRIVACY */

#ifdef SIP_CONF
		case	SipHdrTypeJoin		: /* Join header */
#endif
#ifdef SIP_SECURITY
		case SipHdrTypeSecurityClient	: /* Security-Client */
		case SipHdrTypeSecurityServer   : /* Security-Server */
		case SipHdrTypeSecurityVerify   : /* Security-Verify */
#endif
#ifdef SIP_3GPP
		case	SipHdrTypePAssociatedUri	:
		case	SipHdrTypePCalledPartyId	:
		case	SipHdrTypePVisitedNetworkId	:
		case	SipHdrTypePcfAddr	:
#endif
#ifdef SIP_CONGEST
		case	SipHdrTypeRsrcPriority	:
		case	SipHdrTypeAcceptRsrcPriority	:
#endif
					break;

		default		:
#ifdef SIP_DCS
				if (sip_dcs_validateDcsHeaderType(dType) == SipTrue)
						break;
#endif
				*err = E_INV_TYPE;
				return SipFail;

	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting functoion validateSipHeaderType");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sip_validateHeaderString
**
** DESCRIPTION: This function returns SipSuccess if the
** string "str1" is having a correct pFormat for any of the headers.
**
********************************************************************/

SipBool sip_validateHeaderString
#ifdef ANSI_PROTO
	 ( SIP_S8bit *str1,
	   en_HeaderType dType,
	   SipError *err)
#else
	( str1, dType, err )
	  SIP_S8bit *str1;
	  en_HeaderType  dType;
	  SipError *err;
#endif
{
	SipBool	ret;
	SIP_S8bit *hdr_name, old;
	SIP_S8bit *temp_str, *str;
	SIP_U32bit dLength, i, j;
        SIP_U32bit string_length;

	dLength = strlen ( str1 );
	if ( dLength == 0 )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	/* traversing hte given string till ':' is encountered */
	for ( i = 0; i < dLength ; i++ )
		if ( str1[i] == ':' )
			break;

	/* No ':' is present in the string and therfore E_INV_TYPE */
	if ( i >= dLength )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	/* duplicating the input string so that modifications made on it is not reflected outside */
	str = (SIP_S8bit *)STRDUPACCESSOR(str1);
	if ( str == SIP_NULL)
	{
		*err  = E_NO_MEM;
		return SipFail;
	}

	old = str[i+1];
	str[i+1] = '\0';

	temp_str = ( SIP_S8bit *) STRDUPACCESSOR(str);
	if ( temp_str == SIP_NULL )
	{
		*err = E_NO_MEM;
		return SipFail;
	}

	str[i+1] = old;

	hdr_name = ( SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID,  i + 2, err );
	if ( hdr_name == SIP_NULL)
		return SipFail;
        
        string_length=strlen(temp_str);
	for(i=0,j=0;i<string_length;i++)
		if((temp_str[i]==' ')||(temp_str[i]=='\t')||(temp_str[i]=='\n')||(temp_str[i]=='\r'))
	{
	}
	else
	{
		hdr_name[j] = temp_str[i];
		j++;
	}
	hdr_name[j]='\0';

	*err = E_NO_ERROR;


	switch ( dType )
	{

	case SipHdrTypeAccept :
			if(strcasecmp("Accept:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;

	case SipHdrTypeErrorInfo :
			if(strcasecmp("Error-Info:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;

	case SipHdrTypeInReplyTo :
			if(strcasecmp("In-Reply-To:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeAlertInfo :
			if(strcasecmp("Alert-Info:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeAuthenticationInfo :
			if(strcasecmp("Authentication-Info:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeCallInfo :
			if(strcasecmp("Call-Info:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeContentLanguage :
			if(strcasecmp("Content-Language:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeAcceptEncoding :
			if(strcasecmp("Accept-Encoding:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypeAcceptLanguage :
				if(strcasecmp("Accept-Language:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;

	case SipHdrTypeCallId :
			if((strcasecmp("Call-id:",hdr_name)==0)||(strcasecmp("i:",hdr_name)==0))

				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypeCseq :
			if(strcasecmp("CSeq:",hdr_name)==0)

				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeDate :
			if(strcasecmp("Date:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeEncryption :
			if(strcasecmp("Encryption:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeExpiresDate :
	case SipHdrTypeExpiresSec :
	case SipHdrTypeExpiresAny :
			if(strcasecmp("Expires:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeFrom :
			if((strcasecmp("From:",hdr_name)==0)||(strcasecmp("f:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeReplyTo:
			if((strcasecmp("Reply-To:",hdr_name)==0))
                                ret = SipSuccess;
                        else
                        {
                                *err = E_INV_TYPE;
                                ret = SipFail;
                        }
                        break;
	case SipHdrTypeReplaces:
			if((strcasecmp("Replaces:",hdr_name)==0))
                                ret = SipSuccess;
                        else
                        {
                                *err = E_INV_TYPE;
                                ret = SipFail;
                        }
                        break;
	case SipHdrTypeMinExpires:
			if((strcasecmp("Min-Expires:",hdr_name)==0))
                                ret = SipSuccess;
                        else
                        {
                                *err = E_INV_TYPE;
                                ret = SipFail;
                        }
                        break;
	case SipHdrTypeAlso :
			if((strcasecmp("Also:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeReferTo :
			if((strcasecmp("Refer-To:",hdr_name)==0)||(strcasecmp("r:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeReferredBy :
			if((strcasecmp("Referred-By:",hdr_name)==0)||(strcasecmp("b:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeRecordRoute :
			if(strcasecmp("Record-Route:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
#ifdef SIP_3GPP
	case SipHdrTypeServiceRoute :
			if(strcasecmp("Service-Route:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypePath :
			if(strcasecmp("Path:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypePanInfo :
	        if(strcasecmp("P-Access-Network-Info",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypePcVector :
	        if(strcasecmp("P-Charging-Vector",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
#endif
	case SipHdrTypeTimestamp :
			if(strcasecmp("Timestamp:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeTo :
			if((strcasecmp("To:",hdr_name)==0)||(strcasecmp("t:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeVia :
			if((strcasecmp("Via:", hdr_name)==0)||(strcasecmp("v:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeContentEncoding :
			if((strcasecmp("Content-Encoding:",hdr_name)==0)||(strcasecmp("e:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	/* RPR */
	case SipHdrTypeSupported :
			if((strcasecmp("Supported:",hdr_name)==0)||\
			   (strcasecmp("k:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;

	case SipHdrTypeContentLength :
 			if((strcasecmp("Content-Length:", hdr_name)==0)||(strcasecmp("l:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeContentType :
			if((strcasecmp("Content-Type:",hdr_name)==0)||(strcasecmp("c:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	/* bcpt ext */
	case SipHdrTypeMimeVersion :
			if(strcasecmp("Mime-Version:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;

	case SipHdrTypeAuthorization:
			if(strcasecmp("Authorization:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeContactNormal :
	case SipHdrTypeContactWildCard :
	case SipHdrTypeContactAny :
			if((strcasecmp("Contact:",hdr_name)==0)||(strcasecmp("m:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeHide :

			if(strcasecmp("Hide:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypeMaxforwards :
			if(strcasecmp("Max-Forwards:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypeOrganization :
			if(strcasecmp("Organization:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypePriority :
			if(strcasecmp("Priority:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeProxyauthorization :
			if(strcasecmp("Proxy-Authorization:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeProxyRequire :
			if(strcasecmp("Proxy-require:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeRoute :
			 if(strcasecmp("Route:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeRequire :
			if(strcasecmp("Require:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeResponseKey :
			if(strcasecmp("Response-Key:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeSubject :
			if((strcasecmp("Subject:",hdr_name)==0)||(strcasecmp("s:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeUserAgent :
			if(strcasecmp("User-Agent:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	/* Retrans1 */
	case SipHdrTypeRSeq :
	if(strcasecmp("RSeq:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeRAck :
	if(strcasecmp("RAck:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	/* Retrans1 ends */
	case SipHdrTypeAllow :
			if(strcasecmp("Allow:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeProxyAuthenticate  :
			if(strcasecmp("Proxy-Authenticate:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeRetryAfterDate  :
	case SipHdrTypeRetryAfterSec  :
	case SipHdrTypeRetryAfterAny  :
			if(strcasecmp("Retry-After:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeServer :
			if(strcasecmp("Server:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeUnsupported :
			if(strcasecmp("Unsupported:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeWarning :
			if(strcasecmp("Warning:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeWwwAuthenticate :
			if(strcasecmp("WWW-Authenticate:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
#ifdef SIP_CCP			
	case SipHdrTypeAcceptContact:
			if((strcasecmp("Accept-Contact:",hdr_name)==0)||\
			   (strcasecmp("a:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break; /* CCP case */
	case SipHdrTypeRejectContact:
			if((strcasecmp("Reject-Contact:",hdr_name)==0)||\
			   (strcasecmp("j:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break; /* CCP case */
	case SipHdrTypeRequestDisposition:
			if((strcasecmp("Request-Disposition:",hdr_name)==0)||\
			   (strcasecmp("d:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break; /* CCP case */
#endif			
	case SipHdrTypeContentDisposition:
			if(strcasecmp("Content-Disposition:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break; 
	case SipHdrTypeReason:
			if(strcasecmp("Reason:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break; 

#ifdef SIP_IMPP
	case SipHdrTypeEvent :
			if((strcasecmp("Event:",hdr_name)==0)||\
			   (strcasecmp("o:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeAllowEvents :
			if((strcasecmp("Allow-Events:",hdr_name)==0)||\
			   (strcasecmp("u:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
	case SipHdrTypeSubscriptionState :
			if(strcasecmp("Subscription-State:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}

			break;
#endif

	case SipHdrTypeUnknown :
				ret = SipSuccess;
			break;

#ifdef SIP_SESSIONTIMER
	case SipHdrTypeMinSE :
			if(strcasecmp("Min-SE:", hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypeSessionExpires :
			if((strcasecmp("Session-Expires:",hdr_name)==0)|| \
			(strcasecmp("x:",hdr_name)==0))
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
#endif
#ifdef SIP_PRIVACY
	case SipHdrTypePrivacy :
		if(strcasecmp("Privacy:",hdr_name)==0)
			ret = SipSuccess;
		else
		{
			*err = E_INV_TYPE;
			ret = SipFail;
		}
		break;
       case SipHdrTypePAssertId :
			if(strcasecmp("P-Asserted-Identity:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
       case SipHdrTypePPreferredId :
			if(strcasecmp("P-Preferred-Identity:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
#endif /* # ifdef SIP_PRIVACY */
            
#ifdef SIP_CONF
	   case SipHdrTypeJoin:
			if((strcasecmp("Join:",hdr_name)==0))
               ret = SipSuccess;
            else
            {
                *err = E_INV_TYPE;
                ret = SipFail;
            }
            break;
#endif                         
#ifdef SIP_SECURITY
	case SipHdrTypeSecurityClient :
			if(strcasecmp("Security-Client:",hdr_name)==0)
			        ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypeSecurityServer :
			if(strcasecmp("Security-Server:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;
	case SipHdrTypeSecurityVerify :
			if(strcasecmp("Security-Verify:",hdr_name)==0)
				ret = SipSuccess;
			else
			{
				*err = E_INV_TYPE;
				ret = SipFail;
			}
			break;	
#endif
#ifdef SIP_CONGEST
	   case SipHdrTypeRsrcPriority:
			if((strcasecmp("Resource-Priority:",hdr_name)==0))
               ret = SipSuccess;
            else
            {
                *err = E_INV_TYPE;
                ret = SipFail;
            }
            break;
	   case SipHdrTypeAcceptRsrcPriority:
			if((strcasecmp("Accept-Resource-Priority:",hdr_name)==0))
               ret = SipSuccess;
            else
            {
                *err = E_INV_TYPE;
                ret = SipFail;
            }
            break;

#endif
            
#ifdef SIP_3GPP
       case SipHdrTypePAssociatedUri:
			if((strcasecmp("P-Associated-URI:",hdr_name)==0))
               ret = SipSuccess;
            else
            {
                *err = E_INV_TYPE;
                ret = SipFail;
            }
            break;
       case SipHdrTypePCalledPartyId:
			if((strcasecmp("P-Called-Party-ID:",hdr_name)==0))
               ret = SipSuccess;
            else
            {
                *err = E_INV_TYPE;
                ret = SipFail;
            }
            break;
       case SipHdrTypePVisitedNetworkId:
			if((strcasecmp("P-Visited-Network-ID:",hdr_name)==0))
               ret = SipSuccess;
            else
            {
                *err = E_INV_TYPE;
                ret = SipFail;
            }
            break;
       case SipHdrTypePcfAddr:
			if((strcasecmp("P-Charging-Function-Addresses:",hdr_name)==0))
               ret = SipSuccess;
            else
            {
                *err = E_INV_TYPE;
                ret = SipFail;
            }
            break;
#endif            

	default			:
#ifdef SIP_DCS
				if ((sip_dcs_validateDcsHeaderString(hdr_name, dType, err)) == SipSuccess)
				{
					ret = SipSuccess;
					break;
				}
#endif
				*err = E_INV_TYPE;
				ret = SipFail;
			break;

	}

	sip_freeString ( str );
	sip_freeString ( temp_str);
	sip_freeString ( hdr_name );
	return ret;

}

/********************************************************************
**
** FUNCTION:  sip_validateSipMsgBodyType
**
**DESCRIPTION:  This fucntion returns SipSuccess if "dType"
** is one among the defined en_MsgBodyType's else it returns SipFail.
**
********************************************************************/

SipBool sip_validateSipMsgBodyType
#ifdef ANSI_PROTO
	(en_SipMsgBodyType dType, SipError *pErr)
#else
	(dType, pErr)
	en_SipMsgBodyType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function validateSipMsgBodyType");
	switch (dType)
	{
		case 	SipSdpBody				:
		case	SipIsupBody				:
		case	SipMultipartMimeBody	:
#ifdef SIP_MWI
		case	SipMessageSummaryBody	:
#endif
		case	SipUnknownBody			:
		case	SipBodyAny				:
		case	SipAppSipBody			:
						break;
		default			:*pErr = E_INV_TYPE;
					SIPDEBUGFN("Exiting function validateSipMsgBodyType");
					 return SipFail;

	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function validateSipMsgBodyType");
	return SipSuccess;

}


