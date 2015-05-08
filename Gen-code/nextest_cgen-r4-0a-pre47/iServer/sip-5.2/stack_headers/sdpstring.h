/******************************************************************
** FUNCTION: Ths file has all the SDP string APIs exposed to the user
**
*******************************************************************
**
** FILENAME:
	sdpstring.c
**
** DESCRIPTION
**
**
**   DATE     	         NAME                      REFERENCE
**   ---				-----						--------
** 14/07/2000   	Siddharth Toshniwal		   		 Creation
**
** Copyright 2000, Hughes Software Systems, Ltd.
*************************************************************/

#ifndef __SDPSTRING_H__
#define __SDPSTRING_H__

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


/***********************************************************************
** Function: sdp_getSdpConnectionAsString 
** Description: get connection line as a string
** Parameters:
**		pSdpMsg (IN)		- Sdp Message
**		ppStrConn (OUT)		- retrieved connection field as string
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_getSdpConnectionAsString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit **ppStrConn, SipError *pErr));

/***********************************************************************
** Function: sdp_getSdpOriginAsString 
** Description: get Sdp Origin line as a string
** Parameters:
**		pSdpMsg (IN)		- Sdp Message
**		ppStrOrigin (OUT)	- retrieved origin field as string
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_getSdpOriginAsString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit **ppStrOrigin, SipError *pErr));

/***********************************************************************
** Function: sdp_getSdpTimeAtIndexAsString 
** Description: get Sdp Time line at specified index as a string
** Parameters:
**		pSdpMsg (IN)	- Sdp Message
**		ppStrTime(OUT)	- Sdp Time line retrieved as string
**		dIndex (IN)		- Index to retrieve Sdp Time from
**		pErr (OUT)		- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_getSdpTimeAtIndexAsString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit **ppStrTime, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_getSdpAttrAtIndexAsString 
** Description:  get attribute line at specified index as a string
** Parameters:
**		pSdpMsg (IN)	- Sdp Message
**		pStrAttr (OUT)	- Sdp Attribute line retrieved as string
**		dIndex (OUT)	- index to retrieve from
**		pErr (OUT)		- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_getSdpAttrAtIndexAsString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit **ppStrAttr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_setSdpConnectionFromString 
** Description: set connection line as a string
** Parameters:
**		pSdpMsg (IN/OUT)	- Sdp Message
**		pStrConn (IN)		- connection line to set
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_setSdpConnectionFromString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit *pStrConn, SipError *pErr));

/***********************************************************************
** Function: sdp_setSdpOriginFromString 
** Description: set Sdp Origin line as string
** Parameters:
**		pSdpMsg (IN/OUT)	- Sdp Message
**		pStr (IN)			- Sdp Origin to set
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_setSdpOriginFromString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit *pStr, SipError *pErr));

/***********************************************************************
** Function: sdp_setSdpTimeAtIndexFromString 
** Description: set time line as string
** Parameters:
**		pSdpMsg (IN/OUT)	- Sdp Message
**		pStr	(IN)		- time line to set
**		dIndex	(IN)		- index to set
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_setSdpTimeAtIndexFromString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit *pStr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_insertSdpTimeAtIndexFromString 
** Description: insert Sdp Time at specified index as a string
** Parameters:
**		pSdpMsg (IN/OUT)	- Sdp Message
**		pStr 	(IN)		-  time line to insert
**		dIndex (IN)			- index to insert into
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_insertSdpTimeAtIndexFromString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit *pStr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_setSdpAttrAtIndexFromString 
** Description: set Sdp attribute line as string at specified index
** Parameters:
**		pSdpMsg (IN/OUT) 	- Sdp Message
**		pStrAttr (IN)		- Sdp attribute line to set
**		dIndex (IN)			- Index to set into
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_setSdpAttrAtIndexFromString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit *pStrAttr, SIP_S8bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_insertSdpAttrAtIndexFromString 
** Description: insert Sdp Attribute line as string at specified index
** Parameters:
**		pSdpMsg (IN/OUT)		- Sdp Message
**		pStrAttr (IN)			- Attribute line to set
**		dIndex (IN)				- Index to insert into
**		pErr (OUT)				- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_insertSdpAttrAtIndexFromString _ARGS_((SdpMessage *pSdpMsg, \
		SIP_S8bit *pStrAttr, SIP_S8bit dIndex, SipError *pErr));


/********************************************************************************
** Function: sdp_setConnectionFromStringAtIndexInMedia 
** Description: set connection line as string at specified index in Media line
** Parameters:
**		pSdpMedia (IN/OUT) 		- Sdp Message
**		pStrConn (IN)			- Sdp connection line to set
**		dIndex (IN)				- index to set into
**		pErr (OUT)				- possible error value (see api ref. doc)
**********************************************************************************/
extern 	SipBool sdp_setConnectionFromStringAtIndexInMedia _ARGS_((SdpMedia *pSdpMedia, \
		SIP_S8bit *pStrConn, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_insertConnectionFromStringAtIndexInMedia 
** Description: insert connection line as string at specified index in Media line
** Parameters:
**		pSdpMedia (IN/OUT) 		- Sdp Message
**		pStrConn (IN)			- Sdp connection line to insert
**		dIndex (IN)				- index to insert into
**		pErr (OUT)				- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_insertConnectionFromStringAtIndexInMedia _ARGS_((SdpMedia *pSdpMedia, \
		SIP_S8bit *pStrConn, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_getConnectionAtIndexInMediaAsString 
** Description: get connection line as string at specified index from Media line
** Parameters:
**		pSdpMedia (IN)		- Sdp Message
**		ppStrConn (IN)		- retrieved connection line as string
**		dIndex	(IN)		- Index to set into
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_getConnectionAtIndexInMediaAsString _ARGS_((SdpMedia *pSdpMedia, \
		SIP_S8bit **ppStrConn, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_getAttrAtIndexInMediaAsString 
** Description: get attribute line as string at specified index from Media line
** Parameters:
**		pSdpMedia (IN)		- Sdp Message
**		ppStrAttr (OUT)		- retrieved attribute line as string
**		dIndex 	(IN)		- index to retrieve from
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_getAttrAtIndexInMediaAsString _ARGS_((SdpMedia *pSdpMedia, \
		SIP_S8bit **ppStrAttr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_setAttrFromStringAtIndexInMedia 
** Description: set attribute line as string at specified index in media line
** Parameters:
**		pSdpMedia (IN/OUT)	- Sdp Message
**		pStrAttr (IN)		- attribute line to set 
**		dIndex (IN)			- index to set into
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_setAttrFromStringAtIndexInMedia _ARGS_((SdpMedia *pSdpMedia, \
		SIP_S8bit *pStrAttr, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sdp_insertAttrFromStringAtIndexInMedia 
** Description: insert attribute line as string at a specified index in media line
** Parameters:
**		pSdpMedia (IN/OUT)		- Sdp Message
**		pStrAttr	(IN)		- Attribute line to insert
**		dIndex		(IN)		- Index to insert into
**		pErr (OUT)			- possible error value (see api ref. doc)
************************************************************************/
extern 	SipBool sdp_insertAttrFromStringAtIndexInMedia _ARGS_((SdpMedia *pSdpMedia, \
		SIP_S8bit *pStrAttr, SIP_U32bit dIndex, SipError *pErr));

/* some more internal functions - the user does not call these directly */


extern 	SipBool sdp_makeSdpConnection _ARGS_((SdpConnection **ppSdpConn, \
		SIP_S8bit *pStr, SipError *pErr));

extern 	SipBool sdp_makeSdpTime _ARGS_((SdpTime **ppSdpTime, \
		SIP_S8bit *pStr, SipError *pErr));

extern 	SipBool sdp_makeSdpAttr _ARGS_((SdpAttr **ppSdpAttr, \
		SIP_S8bit *pStr, SipError *pErr));

extern SipBool sdp_makeSdpOrigin _ARGS_((SdpOrigin **ppSdpOrigin, \
		SIP_S8bit *pStr, SipError *pErr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
