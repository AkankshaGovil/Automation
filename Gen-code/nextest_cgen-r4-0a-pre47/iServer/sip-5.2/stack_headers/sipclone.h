/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes off all SIP Structure  
**      duplicating/cloning APIs.
**
*******************************************************************************
**
** FILENAME:
** 	sipclone.h
**
** DESCRIPTION:
**  	
**
** DATE    			  NAME           REFERENCE      REASON
** ----    			  ----           ---------      ------
** 13 Dec 99		S. Luthra
**					B. Borthakur
**					R. Preethy
**
** Copyrights 1999, Hughes Software Systems, Ltd.
*******************************************************************************/

#ifndef __SIP_CLONE_H_
#define __SIP_CLONE_H_

#include "sipstruct.h"
#include "sipcommon.h"
#include "sipcloneintrnl.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/*****************************************************************
** Function:sip_cloneSipStatusLine 
** Description:makes a deep copy of the source Status Line to the destination Status Line
** Parameters:
**			pDest(OUT)		- Destination Status Line
**			pSource(IN)		- Source Status Line
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipStatusLine _ARGS_ ((SipStatusLine *pDest, \
		SipStatusLine *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipReqLine 
** Description:makes a deep copy of the source Request Line to the destination Request Line
** Parameters:
**			pDest(OUT)		- Destination Request Line
**			pSource(IN)		- Source Request Line
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipReqLine _ARGS_ ((SipReqLine *pDest, \
		SipReqLine *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipAddrSpec 
** Description:makes a deep copy of the source AddrSpec to the destination AddrSpec
** Parameters:
**			pDest(OUT)		- Destination AddrSpec
**			pSource(IN)		- Source AddrSpec
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipAddrSpec _ARGS_ ((SipAddrSpec *pDest, \
		SipAddrSpec *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipContactParam 
** Description:makes a deep copy of the source Contact Parameter to the destination Contact Parameter
** Parameters:
**			pDest(OUT)		- Destination Contact Parameter Line
**			pSource(IN)		- Source Contact Parameter Line
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipContactParam _ARGS_ ((SipContactParam *pDest, \
		SipContactParam *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipExpiresStruct 
** Description:makes a deep copy of the source Expires struct to the destination Expires struct
** Parameters:
**			pDest(OUT)		- Destination Expires Struct Line
**			pSource(IN)		- Source Expires Struct Line
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipExpiresStruct _ARGS_ ((SipExpiresStruct *pDest, \
		SipExpiresStruct *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipStringList 
** Description:makes a deep copy of the source String LIst to the destination String List
** Parameters:
**			pDest(OUT)		- Destination String List
**			pSource(IN)		- Source String List
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipStringList _ARGS_ ((SipList *pDest, \
		SipList *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipGenericChallenge 
** Description:makes a deep copy of the source Generic Challenge to the destination Generic Challenge
** Parameters:
**			pDest(OUT)		- Destination Generic Challenge
**			pSource(IN)		- Source Generic Challenge
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipGenericChallenge _ARGS_ ((SipGenericChallenge *pDest, \
		SipGenericChallenge *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipGenericCredential 
** Description:makes a deep copy of the source GenericCredential to the destination GenericCredential
** Parameters:
**			pDest(OUT)		- Destination GenericCredential
**			pSource(IN)		- Source GenericCredential
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipGenericCredential _ARGS_ ((SipGenericCredential *pDest, \
		SipGenericCredential *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipUrl 
** Description:makes a deep copy of the source SipUrl to the destination SipUrl
** Parameters:
**			pDest(OUT)		- Destination SipUrl 
**			pSource(IN)		- Source SipUrl
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipUrl _ARGS_ ((SipUrl *pDest, \
		SipUrl *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipDateFormat 
** Description:makes a deep copy of the source SipDateFormat to the destination SipDateFormat 
** Parameters:
**			pDest(OUT)		- Destination SipDateFormat 
**			pSource(IN)		- Source SipDateFormat
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipDateFormat _ARGS_ ((SipDateFormat *pDest, \
		SipDateFormat *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipTimeFormat 
** Description:makes a deep copy of the source SipTimeFormat to the destination SipTimeFormat
** Parameters:
**			pDest(OUT)		- Destination SipTimeFormat 
**			pSource(IN)		- Source SipTimeFormat 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipTimeFormat _ARGS_ ((SipTimeFormat *pDest, \
		SipTimeFormat *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipDateStruct 
** Description:makes a deep copy of the source SipDateStruct to the destination SipDateStruct 
** Parameters:
**			pDest(OUT)		- Destination SipDateStruct 
**			pSource(IN)		- Source SipDateStruct 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipDateStruct _ARGS_ ((SipDateStruct *pDest, \
		SipDateStruct *pSource, SipError *pErr));

/*****************************************************************
** Function:sdp_cloneSdpTime 
** Description:makes a deep copy of the source SdpTime to the destination  SdpTime 
** Parameters:
**			pDest(OUT)		- Destination SdpTime 
**			pSource(IN)		- Source SdpTime 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sdp_cloneSdpTime _ARGS_ ((SdpTime *ppDest, \
		SdpTime *pSource, SipError *pErr));

/*****************************************************************
** Function:sdp_cloneSdpMedia 
** Description:makes a deep copy of the source SdpMedia to the destination SdpMedia 
** Parameters:
**			pDest(OUT)		- Destination SdpMedia 
**			pSource(IN)		- Source SdpMedia 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sdp_cloneSdpMedia _ARGS_ ((SdpMedia *ppDest, \
		SdpMedia *pSource, SipError *pErr));

/*****************************************************************
** Function:sdp_cloneSdpConnection 
** Description:makes a deep copy of the source SdpConnection to the destination  SdpConnection 
** Parameters:
**			pDest(OUT)		- Destination SdpConnection 
**			pSource(IN)		- Source SdpConnection 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sdp_cloneSdpConnection _ARGS_ ((SdpConnection *ppDest, \
		SdpConnection *pSource, SipError *pErr));

/*****************************************************************
** Function:sdp_cloneSdpAttr 
** Description:makes a deep copy of the source SdpAttributes to the destination SdpAttributes 
** Parameters:
**			pDest(OUT)		- Destination SdpAttributes 
**			pSource(IN)		- Source SdpAttributes 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sdp_cloneSdpAttr _ARGS_ ((SdpAttr *ppDest,\
		SdpAttr *pSource, SipError *pErr));

#define sip_cloneSdpMessage sdp_cloneSdpMessage
/*****************************************************************
** Function:sdp_cloneSdpMessage 
** Description:makes a deep copy of the source SdpMessage to the destination SdpMessage 
** Parameters:
**			pDest(OUT)		- Destination SdpMessage 
**			pSource(IN)		- Source SdpMessage 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sdp_cloneSdpMessage _ARGS_ ((SdpMessage *pDest, \
		SdpMessage *pSource, SipError *pErr));

/*****************************************************************
** Function:sdp_cloneSdpOrigin 
** Description:makes a deep copy of the source SdpOrigin to the destination SdpOrigin 
** Parameters:
**			pDest(OUT)		- Destination SdpOrigin 
**			pSource(IN)		- Source SdpOrigin 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sdp_cloneSdpOrigin _ARGS_ ((SdpOrigin *pDest, \
		SdpOrigin *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipUnknownMessage 
** Description:makes a deep copy of the source SipUnknownMessage to the destination SipUnknownMessage 
** Parameters:
**			pDest(OUT)		- Destination SipUnknownMessage 
**			pSource(IN)		- Source SipUnknownMessage 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipUnknownMessage _ARGS_ ((SipUnknownMessage *pDest, \
		SipUnknownMessage *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipMsgBody 
** Description:makes a deep copy of the source SipMessageBody to the destination SipMessageBody 
** Parameters:
**			pDest(OUT)		- Destination SipMessageBody 
**			pSource(IN)		- Source SipMessageBody 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipMsgBody _ARGS_ ((SipMsgBody *pDest, \
		SipMsgBody *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipParam 
** Description:makes a deep copy of the source SipParam to the destination SipParam 
** Parameters:
**			pDest(OUT)		- Destination SipParam 
**			pSource(IN)		- Source SipParam 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipParam _ARGS_ ((SipParam *pDest, \
		SipParam *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipHeader 
** Description:makes a deep copy of the source SipHeader to the destination SipHeader 
** Parameters:
**			pDest(OUT)		- Destination SipHeader 
**			pSource(IN)		- Source SipHeader 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipHeader _ARGS_ ((SipHeader *pDest, \
		SipHeader *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_cloneSipMessage 
** Description:makes a deep copy of the source SipMessage to the destination SipMessage 
** Parameters:
**			pDest(OUT)		- Destination SipMessage 
**			pSource(IN)		- Source SipMessage 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_cloneSipMessage _ARGS_ ((SipMessage *pDest, \
		SipMessage *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_bcpt_cloneSipMimeHeader 
** Description:makes a deep copy of the source SipMimeHeader to the destination SipMimeHeader 
** Parameters:
**			pDest(OUT)		- Destination SipMimeHeader 
**			pSource(IN)		- Source SipMimeHeader 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_bcpt_cloneSipMimeHeader _ARGS_((SipMimeHeader *pDest, \
		SipMimeHeader *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_bcpt_cloneMimeMessage 
** Description:makes a deep copy of the source SipMimeMessage to the destination SipMimeMessage 
** Parameters:
**			pDest(OUT)		- Destination SipMimeMessage 
**			pSource(IN)		- Source SipMimeMessage 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_bcpt_cloneMimeMessage _ARGS_((MimeMessage *pDest, \
		MimeMessage *pSource, SipError *pErr));
#ifdef SIP_MWI
/*****************************************************************
** Function:sip_mwi_cloneMesgSummaryMessage
** Description:makes a deep copy of the source MesgSummaryMessage to \
the destination MesgSummaryMessage
** Parameters:
**                      pDest(OUT)              - Destination MesgSummaryMessage
**                      pSource(IN)             - Source MesgSummaryMessage
**                      pErr(OUT)               - possible error value (see API
ref doc)
**
******************************************************************/
SipBool sip_mwi_cloneMesgSummaryMessage _ARGS_ ((MesgSummaryMessage *pDest, MesgSummaryMessage *pSource, SipError *pErr));

/*****************************************************************
** Function:sip_mwi_cloneSummaryLine
** Description:makes a deep copy of the source SummaryLine \
to the destination SummaryLine
** Parameters:
**                      pDest(OUT)              - Destination SummaryLine
**                      pSource(IN)             - Source SummaryLine
**                      pErr(OUT)               - possible error value (see API
ref doc)
**
******************************************************************/
SipBool sip_mwi_cloneSummaryLine _ARGS_ ((SummaryLine *pDest, SummaryLine *pSource, SipError *pErr));

#endif
/*****************************************************************
** Function:sip_cloneSipNameValuePair
** Description:makes a deep copy of the source NameValuePair to the destination NameValuePair 
** Parameters:
**                      pDest(OUT)              - Destination NameValuePair
**                      pSource(IN)             - Source NameValuePair
**                      pErr(OUT)               - possible error value (see API
ref doc)
**
******************************************************************/
SipBool sip_cloneSipNameValuePair _ARGS_ ((SipNameValuePair *pDest, SipNameValuePair *pSource, SipError *pErr));


/*****************************************************************
** Function:sip_bcpt_cloneIsupMessage 
** Description:makes a deep copy of the source IsupMessage to the destination IsupMessage 
** Parameters:
**			pDest(OUT)		- Destination IsupMessage 
**			pSource(IN)		- Source IsupMessage 
**			pErr(OUT)		- possible error value (see API ref doc)
**
******************************************************************/
extern SipBool sip_bcpt_cloneIsupMessage _ARGS_((IsupMessage *pDest, \
		IsupMessage *pSource, SipError *pErr));


extern SipBool __sip_cloneSipBadHeader _ARGS_ ((SipBadHeader *dest,\
	 SipBadHeader *source, SipError *err));
 
 
/******************************************************************
**
** FUNCTION:  __sip_cloneSipReplacesHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the ReplacesHeader structures "source" to "dest".
**
******************************************************************/
extern SipBool __sip_cloneSipReplacesHeader _ARGS_( (SipReplacesHeader *dest,\
				SipReplacesHeader *source, SipError *err));

/******************************************************************
**
** FUNCTION:  __sip_cloneSipMinExpiresHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the MinExpires Header structures "source" to "dest".
**
******************************************************************/
extern SipBool __sip_cloneSipMinExpiresHeader _ARGS_( (SipMinExpiresHeader \
				*dest, SipMinExpiresHeader *source, SipError *err));

/******************************************************************
**
** FUNCTION:  __sip_cloneSipReplyToHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the ReplyToHeader structures "source" to "dest".
**
******************************************************************/
extern SipBool __sip_cloneSipReplyToHeader _ARGS_( (SipReplyToHeader *dest,\
				SipReplyToHeader *source, SipError *err));



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
