/******************************************************************
** FUNCTION: Ths file has all the SDP headers exposed to the user
**
*******************************************************************
**
** FILENAME:
**	Sdp.c
**
** DESCRIPTION
**
**
**   DATE              NAME                      REFERENCE
**  ------			---------					------------
** 23/11/99            B.Borthakur, K. Deepali   Added Origin APIs
**
** Copyright 1999, Hughes Software Systems, Ltd.
*************************************************************/

#ifndef __SDP_H__
#define __SDP_H__

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
** Function: sdp_getVersion
** Description: get Version field from Sdp Message
** Parameters:
**		msg (IN) 		- Sdp Message
**		version(OUT)	- version retrieved
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getVersion _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **version, SipError *err));

/***********************************************************************
** Function: sdp_setVersion
** Description: set Version field in Sdp Message
** Parameters:
**		msg (IN/OUT) 	- Sdp Message
**		version(IN) 	- Version to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setVersion _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *version, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sdp_getOrigin
** Description: get Origin field from Sdp Message
** Parameters:
**		msg (IN) 	- Sdp Message
**		origin(OUT) - origin retrieved
**		err	(OUT)	- Possible error value (see API ref doc)
**
************************************************************************/
extern 	SipBool sdp_getOrigin _ARGS_(( SdpMessage *msg, \
		SdpOrigin **origin, SipError *err));

#else
/***********************************************************************
** Function: sdp_getOrigin
** Description: get Origin field from Sdp Message
** Parameters:
**		msg (IN) 	- Sdp Message
**		origin(OUT) - origin retrieved
**		err	(OUT)	- Possible error value (see API ref doc)
**
************************************************************************/
extern 	SipBool sdp_getOrigin _ARGS_(( SdpMessage *msg, \
		SdpOrigin *origin, SipError *err));

#endif
/***********************************************************************
** Function: sdp_setOrigin
** Description: set Origin field in Sdp Message
** Parameters:
**		msg (IN/OUT) 	- Sdp Message
**		origin(IN)		- Origin to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setOrigin _ARGS_(( SdpMessage *msg, \
		SdpOrigin *origin, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sdp_getConnection
** Description: get Connection field from Sdp Message
** Parameters:
**		msg (IN) 		- Sdp Message
**		connection(OUT)	- Retrieved connection field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getConnection _ARGS_(( SdpMessage *msg, \
		SdpConnection **connection, SipError *err));

#else
/***********************************************************************
** Function: sdp_getConnection
** Description: get Connection field from Sdp Message
** Parameters:
**		msg (IN) 		- Sdp Message
**		connection(OUT)	- Retrieved connection field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getConnection _ARGS_(( SdpMessage *msg, \
		SdpConnection *connection, SipError *err));

#endif
/***********************************************************************
** Function: sdp_setConnection
** Description: set connection field in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sp Message
**		connection(IN)	- connection to set
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setConnection _ARGS_(( SdpMessage *msg, \
		SdpConnection *connection, SipError *err));

/***********************************************************************
** Function: sdp_getUserFromOrigin
** Description: get user field from Origin line
** Parameters:
**		msg (IN)		- Sdp Origin line
**		user (OUT)		- retrieved user field
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getUserFromOrigin _ARGS_(( SdpOrigin * msg,	\
		SIP_S8bit ** user, SipError * err));


/***********************************************************************
** Function: sdp_setUserInOrigin
** Description: set user field in origin line
** Parameters:
**		msg (IN/OUT) -	Sdp Origin line
**		user (IN) 	 - 	User value to set
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setUserInOrigin _ARGS_(( SdpOrigin * msg,	\
		SIP_S8bit * user, SipError * err));

/***********************************************************************
** Function: sdp_getSessionIdFromOrigin
** Description: get session identifier from origin field
** Parameters:
**		origin (IN)		- Sdp Origin line
**		id	(OUT)		- Retrieved Session Id field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getSessionIdFromOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit ** id, SipError * err));

/***********************************************************************
** Function: sdp_setSessionIdInOrigin
** Description: set session identifier in origin field
** Parameters:
**		origin (IN/OUT)		- Sdp Origin line
**		id (IN)				- Session id to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setSessionIdInOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit * id, SipError * err));

/***********************************************************************
** Function: sdp_getVersionFromOrigin
** Description: get version field from Origin line
** Parameters:
**		origin (IN)		- Sdp Origin line
**		version (OUT)	- retrieved version field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getVersionFromOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit ** version, SipError * err));

/***********************************************************************
** Function: sdp_setVersionInOrigin
** Description: set version field in Origin line
** Parameters:
**		origin (IN/OUT)	- Sdp Origin line
**		version (IN)	- Version to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setVersionInOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit * version, SipError * err));

/***********************************************************************
** Function: sdp_getNetTypeFromOrigin
** Description: get Network Type field from Origin line
** Parameters:
**		origin (IN) 		- Sdp Origin line
**		ntype	(OUT)		- retrieved network type
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getNetTypeFromOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit ** ntype, SipError * err));

/***********************************************************************
** Function: sdp_setNetTypeInOrigin
** Description: set Network Type field in Origin line
** Parameters:
**		origin (IN/OUT)		- Sdp Origin line
**		ntype (IN)			- Network Type to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setNetTypeInOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit * ntype, SipError * err));

/***********************************************************************
** Function: sdp_getAddrFromOrigin
** Description:get Address field from Origin Line
** Parameters:
**		origin 	(IN) 		- Sdp Origin line
**		addr	(OUT)		- retrieved address field
**		err		(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getAddrFromOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit ** addr, SipError * err));

/***********************************************************************
** Function: sdp_setAddrInOrigin
** Description: Set address field in Origin Line
** Parameters:
**		origin (IN/OUT)		- Sdp Origin line
**		addr	(IN)		- Address field to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setAddrInOrigin _ARGS_(( SdpOrigin * origin,	\
		SIP_S8bit * addr, SipError * err));

/***********************************************************************
** Function: sdp_getAddrTypeFromOrigin
** Description: get address type field from origin line
** Parameters:
**		origin(IN)		- Sdp Origin line
**		addr_type(OUT)	- retrieved address type field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getAddrTypeFromOrigin _ARGS_(( SdpOrigin * origin, \
		SIP_S8bit ** addr_type, SipError * err));

/***********************************************************************
** Function: sdp_setAddrTypeInOrigin
** Description: set address type field in origin line
** Parameters:
**		origin (IN/OUT)	- Sdp Origin line
**		addr_type(IN)	- Address Type field to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setAddrTypeInOrigin _ARGS_(( SdpOrigin * origin, \
		SIP_S8bit * addr_type, SipError * err));

/***********************************************************************
** Function: sdp_getSession
** Description: get Session line from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		session(OUT)	- retrieved session line
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getSession _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **session, SipError *err));

/***********************************************************************
** Function: sdp_setSession
** Description: set Session line in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		session (IN)	- session value to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setSession _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *session, SipError *err));

/***********************************************************************
** Function: sdp_getUri
** Description: get uri line from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		uri (OUT)		- retrieved uri field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getUri _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **uri, SipError *err));

/***********************************************************************
** Function: sdp_setUri
** Description: set uri line in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		uri (IN)		- Uri to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setUri _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *uri, SipError *err));

/***********************************************************************
** Function: sdp_getInfo
** Description: get information line from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		info (OUT)		- retrieved uri line
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getInfo _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **info, SipError *err));

/***********************************************************************
** Function: sdp_setInfo
** Description: set information line in Sdp Message (i=)
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		info (IN)		- Information field to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setInfo _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *info, SipError *err));

/***********************************************************************
** Function: sdp_getKey
** Description: get key line from sdp Message (k=)
** Parameters:
**		msg (IN)		- Sdp Message
**		key (OUT)		- Retrieved key value
**		err	(OUT)		- Possible error value (see API ref doc)
**
************************************************************************/
extern 	SipBool sdp_getKey _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **key, SipError *err));

/***********************************************************************
** Function: sdp_setKey
** Description: set key line in Sdp Message (k=)
** Parameters:
**		msg (IN/OUT)		- Sdp Message
**		key (IN)			- key value to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setKey _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *key, SipError *err));

/***********************************************************************
** Function: sdp_getBandwidthCount
** Description: get number of bandwidth lines in Sdp Message (b=)
** Parameters:
**		msg (IN)		- Sdp Message
**		cnt (OUT)		- number of bandwidth lines
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getBandwidthCount _ARGS_(( SdpMessage *msg, \
		SIP_U32bit *cnt, SipError *err ));

/***********************************************************************
** Function: sdp_getBandwidthAtIndex
** Description: get bandwidth line at a specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		bandwidth(OUT)	- retrieved bandwidh field
**		cnt (IN)		- Index at which to retrieve
************************************************************************/
extern 	SipBool sdp_getBandwidthAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **bandwidth, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_setBandwidthAtIndex
** Description:	set bandwidth line at a specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)		- Sdp Message
**		bandwidth(IN)		- bandwidth value to set
** 		cnt (IN)			- Index at which to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setBandwidthAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *bandwidth, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_insertBandwidthAtIndex
** Description:	insert bandwidth line at a specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)		- Sdp Message
**		bandwidth(IN)		- bandwidth value to insert
** 		cnt (IN)			- Index at which to insert
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_insertBandwidthAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *bandwidth, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_deleteBandwidthAtIndex
** Description:	delete bandwidth line at a specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)		- Sdp Message
** 		cnt (IN)			- Index at which to delete
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_deleteBandwidthAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_getPhoneCount
** Description: get number of phone lines in Sdp Message
** Parameters:
**		msg (IN)			- Sdp Message
**		cnt (OUT)			- number of phone lines
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getPhoneCount _ARGS_(( SdpMessage *msg, \
		SIP_U32bit *cnt, SipError *err ));

/***********************************************************************
** Function: sdp_getPhoneAtIndex
** Description: get phone line at specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		phone (OUT)		- retrieved phone line
**		cnt (IN)		- specified index to retrieve from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getPhoneAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **phone, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_setPhoneAtIndex
** Description: set phone line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		phone (IN)		- phone line to set
**		cnt (IN)		- specified index to set into
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setPhoneAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *phone, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_insertPhoneAtIndex
** Description: insert phone line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		phone (IN)		- phone line to insert
**		cnt (IN)		- specified index to insert into
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_insertPhoneAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *phone, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_deletePhoneAtIndex
** Description: delete phone line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		cnt (IN)		- Index to delete phone line from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_deletePhoneAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_getEmailCount
** Description: get number of email lines in Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		cnt (OUT)		- number of email lines
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getEmailCount _ARGS_(( SdpMessage *msg, \
		SIP_U32bit *cnt, SipError *err ));

/***********************************************************************
** Function: sdp_getEmailAtIndex
** Description: get email line at specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		email (OUT)		- retrieved email line
**		cnt (IN)		- Index at which to retrieve
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getEmailAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **email, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_setEmailAtIndex
** Description: set email line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		email (IN)		- email line to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setEmailAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *email, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_insertEmailAtIndex
** Description: insert email line at specified index in Sdp Message
* Parameters:
**		msg (IN/OUT)	- Sdp Message
**		email (IN)		- email line to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_insertEmailAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit *email, SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_deleteEmailAtIndex
** Description: delete email line at specified index from Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		cnt (IN)		- specified index to delete from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_deleteEmailAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_getMediaCount
** Description: get number of media lines from Sdp Message
** Parameters:
**		msg(IN)			- Sdp Message
**		cnt (OUT)		- retrieved number of media lines
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getMediaCount _ARGS_(( SdpMessage	*msg, \
		SIP_U32bit	*cnt, SipError	*err  ));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sdp_getMediaAtIndex
** Description: get media line from Sdp Message at specified index
** Parameters:
**		msg (IN)	- Sdp Message
**		media (OUT)	- retrieved media line
**		cnt (IN)	- Index to retrieve Media line from
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getMediaAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpMedia 	**media, SIP_U32bit 	cnt, SipError 	*err ));

#else
/***********************************************************************
** Function: sdp_getMediaAtIndex
** Description: get media line from Sdp Message at specified index
** Parameters:
**		msg (IN)	- Sdp Message
**		media (OUT)	- retrieved media line
**		cnt (IN)	- Index to retrieve Media line from
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getMediaAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpMedia 	*media, SIP_U32bit 	cnt, SipError 	*err ));

#endif
/***********************************************************************
** Function: sdp_setMediaAtIndex
** Description: set media line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		media (IN)		- Media line to set
**		cnt (IN)		- Index to set at
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setMediaAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpMedia	*media, SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_insertMediaAtIndex
** Description: insert media at specified  index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		media (IN)		- Media line to insert
**		cnt (IN)		- Index to insert at
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_insertMediaAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpMedia	*media, SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_deleteMediaAtIndex
** Description: delete media at specified index from Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		cnt (IN)		- Index to delete at
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_deleteMediaAtIndex _ARGS_(( SdpMessage 	*msg, \
		SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_getTimeCount
** Description: get number of time lines in Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		cnt (OUT)		- retrieved number of times lines
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getTimeCount _ARGS_(( SdpMessage	*msg, \
		SIP_U32bit	*cnt, SipError	*err  ));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sdp_getTimeAtIndex
** Description: get time line at specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		time (OUT)		- retrieved time line
**		cnt (IN)		- Index to retrieve from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getTimeAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpTime 	**time, SIP_U32bit 	cnt, SipError 	*err ));

#else
/***********************************************************************
** Function: sdp_getTimeAtIndex
** Description: get time line at specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		time (OUT)		- retrieved time line
**		cnt (IN)		- Index to retrieve from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getTimeAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpTime 	*time, SIP_U32bit 	cnt, SipError 	*err ));

#endif
/***********************************************************************
** Function: sdp_setTimeAtIndex
** Description: set time line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		time (IN)		- time line to set
**		cnt (IN)		- Index to set at
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setTimeAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpTime	*time, SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_insertTimeAtIndex
** Description: insert time line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		time (IN)		- time line to insert
**		cnt (IN)		- Index to insert at
**		err	(OUT)		- Possible error value (see API ref doc)
**
************************************************************************/
extern 	SipBool sdp_insertTimeAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpTime	*time, SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_deleteTimeAtIndex
** Description: delete time line at specified index from Sdp Message
** Parameters:
**		msg (IN/OUT)	- Sdp Message
**		cnt (IN)		- Index to delete at
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_deleteTimeAtIndex _ARGS_(( SdpMessage 	*msg, \
		SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_getAttrCount
** Description: get number of attribute lines from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		cnt (OUT)		- retrieved number of attribute lines
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getAttrCount _ARGS_(( SdpMessage	*msg, \
		SIP_U32bit	*cnt, SipError	*err  ));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sdp_getAttrAtIndex
** Description: get attribute line at specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		attr(OUT)		- retrieved attribute line
**		cnt (IN)		- Index to retrieve from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getAttrAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpAttr 	**attr, SIP_U32bit 	cnt, SipError 	*err ));

#else
/***********************************************************************
** Function: sdp_getAttrAtIndex
** Description: get attribute line at specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		attr(OUT)		- retrieved attribute line
**		cnt (IN)		- Index to retrieve from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getAttrAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpAttr 	*attr, SIP_U32bit 	cnt, SipError 	*err ));

#endif
/***********************************************************************
** Function: sdp_setAttrAtIndex
** Description: set attribute line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT) 		- Sdp Message
**		attr (IN)			- attribute line to set
**		cnt (IN)			- index to set into
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setAttrAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpAttr	*attr, SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_insertAttrAtIndex
** Description: insert attribute line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT) 		- Sdp Message
**		attr (IN)			- attribute line to insert
**		cnt (IN)			- index to insert into
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_insertAttrAtIndex _ARGS_(( SdpMessage 	*msg, \
		SdpAttr	*attr, SIP_U32bit 	cnt, SipError 	*err ));

/***********************************************************************
** Function: sdp_deleteAttrAtIndex
** Description: delete attribute line at specified index in Sdp Message
** Parameters:
**		msg (IN/OUT) 		- Sdp Message
**		attr (IN)			- attribute line to delete
**		cnt (IN)			- index to delete from
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_deleteAttrAtIndex _ARGS_(( SdpMessage 	*msg, \
		SIP_U32bit 	cnt, SipError 	*err ));


/***********************************************************************
** Function: sdp_getNetTypeFromConnection
** Description: get network type field from connection line
** Parameters:
**	connection (IN)		- Sdp Connection line
**		ntype (OUT)		- retrieved network type field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getNetTypeFromConnection _ARGS_(( SdpConnection *connection, \
		SIP_S8bit **ntype,SipError *err ));

/***********************************************************************
** Function: sdp_setNetTypeInConnection
** Description: set network type field in connection line
** Parameters:
**		connection (IN/OUT)	- Sdp Connection line
**		ntype (IN)			- network type field to set
**		err	(OUT)			- Possible error value (see API ref doc)

************************************************************************/
extern SipBool sdp_setNetTypeInConnection _ARGS_(( SdpConnection*connection, \
		SIP_S8bit *ntype, SipError *err ));

/***********************************************************************
** Function: sdp_getAddrTypeFromConnection
** Description: get address type field from connection line
** Parameters:
**		connection (IN)		- Sdp connection line
**		atype (OUT)			- retrieved address type field
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getAddrTypeFromConnection _ARGS_(( SdpConnection *connection, \
		SIP_S8bit **atype, SipError *err ));

/***********************************************************************
** Function: sdp_setAddrTypeInConnection
** Description: set address type field in connection line
** Parameters:
**		connection (IN/OUT)	- Sdp Connection line
**		atype (IN)			- Addres type field to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setAddrTypeInConnection _ARGS_(( SdpConnection *connection,  \
		SIP_S8bit *atype, SipError *err ));

/***********************************************************************
** Function: sdp_getStartFromTime
** Description: retrieve start time from time line
** Parameters:
**		time (IN)		- Sdp Time
**		start (OUT)		- retrieved start time
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getStartFromTime _ARGS_(( SdpTime *time,SIP_S8bit **start, 	  \
		SipError *err ));

/***********************************************************************
** Function: sdp_setStartInTime
** Description: set start time in time  line
** Parameters:
**		time (IN/OUT)	- Sdp Time
**		start (IN)		- Start time to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setStartInTime _ARGS_(( SdpTime *time, SIP_S8bit *start, \
		SipError *err ));

/***********************************************************************
** Function: sdp_getStopFromTime
** Description: get stop time from time line
** Parameters:
**		time (IN)		- Sdp Time
**		stop(OUT)		- stop time retrieved
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getStopFromTime _ARGS_(( SdpTime *time,SIP_S8bit	**stop, \
		SipError *err ));

/***********************************************************************
** Function: sdp_setStopInTime
** Description: set stop time in time line
** Parameters:
**		time (IN/OUT)	- Sdp Time
**		stop (IN)		- stop time to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setStopInTime _ARGS_(( SdpTime *time, \
		SIP_S8bit *stop,  SipError *err ));

/***********************************************************************
** Function: sdp_getRepeatCountFromTime
** Description: get number of repeat lines in time line (r=)
** Parameters:
**		time (IN)		-  Sdp Time line
**		index (OUT)		- number of repeat lines
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getRepeatCountFromTime _ARGS_(( SdpTime *time, \
		SIP_U32bit *index, SipError	*err ));

/***********************************************************************
** Function: sdp_getRepeatAtIndexFromTime
** Description: get repeat line at specified index from Sdp Time line
** Parameters:
**		time (IN)		- Sdp Time
**		repeat (OUT)	- retrieved repeat line
**		index (IN)		- index to retrieve from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getRepeatAtIndexFromTime _ARGS_(( SdpTime *time, \
		SIP_S8bit **repeat,SIP_U32bit index, SipError *err ));

/***********************************************************************
** Function: sdp_setRepeatAtIndexInTime
** Description: set repeat line at specified index in Sdp Time line
** Parameters:
**		time (IN/OUT)	- Sdp Time
**		repeat (IN)		- repeat line to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setRepeatAtIndexInTime _ARGS_(( SdpTime *time,  \
		SIP_S8bit 	*repeat, SIP_U32bit index, SipError *err ));

/***********************************************************************
** Function: sdp_insertRepeatAtIndexInTime
** Description: insert repeat line at specified index in Sdp Time line
** Parameters:
**		time (IN/OUT)	- Sdp Time
**		repeat (IN)		- repeat line to insert
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_insertRepeatAtIndexInTime _ARGS_(( SdpTime *time, \
		SIP_S8bit *repeat, SIP_U32bit index, SipError *err ));

/***********************************************************************
** Function: sdp_deleteRepeatAtIndexInTime
** Description: delete repeat line at specified index in Sdp Time line
** Parameters:
**		time (IN/OUT)	- Sdp Time
**		index (IN)		- Index to delte repeat line from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_deleteRepeatAtIndexInTime _ARGS_(( SdpTime *time, \
		SIP_U32bit index,SipError *err ));

/***********************************************************************
** Function: sdp_getZoneFromTime
** Description: retrieve Zone field from Sdp Time line
** Parameters:
**	time (IN)		- Sdp Time line
**	zone (OUT)		- retrieved zone line
**	err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getZoneFromTime _ARGS_(( SdpTime *time, \
		SIP_S8bit **zone, SipError *err ));

/***********************************************************************
** Function: sdp_setZoneInTime
** Description: set Zone field in Sdp Time line
** Parameters:
**		time (IN)		- Sdp Time line
**		zone (IN)		- zone field to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setZoneInTime _ARGS_(( SdpTime *time, \
		SIP_S8bit *zone, SipError *err ));

/***********************************************************************
** Function: sdp_getAddrFromConnection
** Description: get address field from connection line
** Parameters:
**		connection (IN)		- Sdp Connection line
**		addr (OUT)			- retrieved address field
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getAddrFromConnection _ARGS_(( SdpConnection *connection,\
		SIP_S8bit **addr, SipError *err ));

/***********************************************************************
** Function: sdp_setAddrInConnection
** Description: set address field in connection line
** Parameters:
**		connection (IN/OUT)	- Sdp Connection line
**		addr (IN)			- address field to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setAddrInConnection	 _ARGS_(( SdpConnection	*connection, \
		SIP_S8bit *addr, SipError *err ));

/***********************************************************************
** Function: sdp_getNameFromAttr
** Description: get name field from attribute line
** Parameters:
**		attr (IN)		- Sdp Attribute line
**		name (OUT)		- retrieved name field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getNameFromAttr _ARGS_(( SdpAttr *attr, \
		SIP_S8bit **name, SipError *err ));

/***********************************************************************
** Function: sdp_setNameInAttr
** Description: set name field in attribute line
** Parameters:
**		attr (IN/OUT)		- Sdp attribute line
**		name (IN)			- name field to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setNameInAttr _ARGS_(( SdpAttr *attr, \
		SIP_S8bit *name, SipError *err ));

/***********************************************************************
** Function: sdp_getValueFromAttr
** Description: get value field from attribute line
** Parameters:
**		attr (IN)		- Sdp Attribute line
**		value (OUT)		- value field retrieved
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getValueFromAttr _ARGS_(( SdpAttr *attr, \
		SIP_S8bit **value, SipError *err ));

/***********************************************************************
** Function: sdp_setValueInAttr
** Description: set value field in attribute line
** Parameters:
**		attr (IN/OUT)	- Sdp Attribute line
**		value (IN)		- Sdp value field to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setValueInAttr _ARGS_(( SdpAttr *attr, \
		SIP_S8bit *value,  SipError *err ));

/***********************************************************************
** Function: sdp_getInfoFromMedia
** Description: get information line from Media line
** Parameters:
**		media (IN)	- Sdp Media line
**		info (OUT)	- retrieved information field
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getInfoFromMedia _ARGS_(( SdpMedia	*media, \
		SIP_S8bit 	**info,SipError	*err ));

/***********************************************************************
** Function: sdp_setInfoInMedia
** Description: set information line of media line
** Parameters:
**		media (IN/OUT)	- Sdp Media line
**		info (IN)	- retrieved information field
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setInfoInMedia _ARGS_(( SdpMedia *media,  \
		SIP_S8bit *info,  SipError *err ));

/***********************************************************************
** Function: sdp_getBandwidthCountFromMedia
** Description: get number of bandwidth lines in Sdp Medialine
** Parameters:
**  	pMedia (IN)		- Sdp Media
**	pBWidthCount (OUT)	- number of bandwidth lines
**		pErr	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getBandwidthCountFromMedia _ARGS_(( SdpMedia *pMedia, \
		SIP_U32bit *pBWidthCount, SipError *pErr ));

/***********************************************************************
** Function: sdp_getBandwidthAtIndexFromMedia
** Description: get bandwidth line at a specified index from Sdp Media line
** Parameters:
**		pMedia (IN)		- Sdp Media line
**		ppBwidth(OUT)	- retrieved bandwidh field
**		dIndex (IN)		- Index at which to retrieve
**		pErr	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getBandwidthAtIndexFromMedia _ARGS_(( SdpMedia *pMedia, \
		SIP_S8bit **ppBwidth, SIP_U32bit dIndex, SipError *pErr ));


/***********************************************************************
** Function: sdp_setBandwidthAtIndexInMedia
** Description:	set bandwidth line at a specified index in Sdp Media line
** Parameters:
**		pMedia (IN/OUT)		- Sdp Media line
**		pBWidth(IN) 		- bandwidth value to set
** 		dIndex (IN)			- Index at which to set
**		pErr	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_setBandwidthAtIndexInMedia _ARGS_(( SdpMedia *pMedia, \
		SIP_S8bit *pBWidth, SIP_U32bit dIndex, SipError *pErr ));


/***********************************************************************
** Function: sdp_insertBandwidthAtIndexInMedia
** Description:	insert bandwidth line at a specified index in Sdp Media line
** Parameters:
**		pMedia (IN/OUT)		- Sdp Media line
**		pBWidth(IN) 		- bandwidth value to insert
** 		dIndex (IN)			- Index at which to insert
**		pErr(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_insertBandwidthAtIndexInMedia _ARGS_(( SdpMedia *pMedia, \
		SIP_S8bit *pBWidth, SIP_U32bit dIndex, SipError *pErr ));


/***********************************************************************
** Function: sdp_deleteBandwidthAtIndexInMedia
** Description:	delete bandwidth line at a specified index in Sdp Media line
** Parameters:
**		pMedia (IN/OUT)		- Sdp Media line
** 		dIndex  (IN)			- Index at which to delete
**		pErr   (OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_deleteBandwidthAtIndexInMedia _ARGS_(( SdpMedia *pMedia, \
		SIP_U32bit dIndex, SipError *err ));


/***********************************************************************
** Function: sdp_getKeyFromMedia
** Description: get key line from Media line
** Parameters:
**		media (IN)		- Sdp Media line
**		key (OUT)		- retrieved key line
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getKeyFromMedia _ARGS_(( SdpMedia *media, \
		SIP_S8bit **key,  SipError *err ));

/***********************************************************************
** Function: sdp_setKeyInMedia
** Description: set key line in media line
** Parameters:
**		media (IN/OUT)		- Sdp Media line
**		key (IN)			- key line to set
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setKeyInMedia _ARGS_(( SdpMedia *media, \
		SIP_S8bit *key,  SipError *err ));

/***********************************************************************
** Function: sdp_getMvalueFromMedia
** Description: get media value from media line
** Parameters:
**		media (IN)		- Sdp Media line
**		mvalue (OUT)	- retrieved media value line
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getMvalueFromMedia _ARGS_(( SdpMedia *media,\
		SIP_S8bit **mvalue, SipError *err ));

/***********************************************************************
** Function: sdp_setMvalueInMedia
** Description: set media value in media line
** Parameters:
**		media (IN/OUT)		- Sdp Media line
**		mvalue (IN)			- media value line to set
**		err	(OUT)			- Possible error value (see API ref doc)

************************************************************************/
extern SipBool sdp_setMvalueInMedia _ARGS_(( SdpMedia *media,\
		SIP_S8bit *mvalue,  SipError *err ));

/***********************************************************************
** Function: sdp_getFormatFromMedia
** Description: get format field from Media line
** Parameters:
**		media (IN)		- Sdp Media line
**		format (OUT)	- retrieved Sdp Format field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getFormatFromMedia _ARGS_(( SdpMedia *media, \
		SIP_S8bit **format,SipError *err ));

/***********************************************************************
** Function: sdp_setFormatInMedia
** Description: set format field in Media line
** Parameters:
**		media (IN/OUT)	- Sdp Media line
**		format (IN)		- format field to set
**		err	(OUT)		- Possible error value (see API ref doc)
**
************************************************************************/
extern SipBool sdp_setFormatInMedia _ARGS_(( SdpMedia *media, \
		SIP_S8bit *format, SipError *err ));

/***********************************************************************
** Function: sdp_getProtoFromMedia
** Description: get protocol field from Media line
** Parameters:
**		media (IN)		- Sdp Media
**		proto (OUT)		- retrieved protocol field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getProtoFromMedia _ARGS_(( SdpMedia *media,\
		SIP_S8bit **proto, SipError *err ));

/***********************************************************************
** Function: sdp_setProtoInMedia
* Description: set protocol field in Media line
** Parameters:
**		media (IN/OUT)	- Sdp Media
**		proto (IN)		- protocol field to set
**		err	(OUT)		- Possible error value (see API ref doc)
**
************************************************************************/
extern SipBool sdp_setProtoInMedia _ARGS_(( SdpMedia *media, \
		SIP_S8bit *proto, SipError *err ));

/***********************************************************************
** Function: sdp_getPortFromMedia
** Description: get port field from Media line
** Parameters:
**		media (IN)		- Sdp Media
**		port (OUT)		- port field retrieved
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getPortFromMedia _ARGS_(( SdpMedia *media, \
		SIP_U16bit *port, SipError *err ));

/***********************************************************************
** Function: sdp_setPortInMedia
** Description: set port field in Media line
** Parameters:
**		media (IN/OUT)	- Sdp Media
**		port (IN)		- port field to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setPortInMedia _ARGS_(( SdpMedia *media, \
		SIP_U16bit port, SipError *err ));

/***********************************************************************
** Function: sdp_getNumportFromMedia
** Description: get number of ports field from media line
** Parameters:
**		media (IN)		- Sdp Media
**		numport (OUT)	- retrieved number of ports  field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getNumportFromMedia _ARGS_(( SdpMedia *media,\
		SIP_U32bit *numport, SipError 	*err));

/***********************************************************************
** Function: sdp_setNumportInMedia
** Description: set number of ports field in media line
** Parameters:
**		media (IN/OUT)	- Sdp Media
**		numport (IN)	- retrieved number of ports  field
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setNumportInMedia _ARGS_(( SdpMedia *media,\
		SIP_U32bit numport,SipError *err));

/***********************************************************************
** Function: sdp_getConnectionCountFromMedia
** Description: get number of connection lines in media line
** Parameters:
**		media (IN)		- Sdp Media
**		index (OUT)		- retrieved number of connection lines
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getConnectionCountFromMedia _ARGS_(( SdpMedia *media,\
		SIP_U32bit *index, SipError *err  ));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sdp_getConnectionAtIndexFromMedia
** Description: get connection line at specified index from Media line
** Parameters:
**		media (IN)		- Sdp Media
**		connection(OUT)	- retrieved connection line
**		index (IN)		- index to retrieve connection line from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getConnectionAtIndexFromMedia _ARGS_(( SdpMedia *media, \
		SdpConnection **connection, SIP_U32bit index,  SipError *err ));

#else
/***********************************************************************
** Function: sdp_getConnectionAtIndexFromMedia
** Description: get connection line at specified index from Media line
** Parameters:
**		media (IN)		- Sdp Media
**		connection(OUT)	- retrieved connection line
**		index (IN)		- index to retrieve connection line from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getConnectionAtIndexFromMedia _ARGS_(( SdpMedia *media, \
		SdpConnection *connection, SIP_U32bit index,  SipError *err ));

#endif
/***********************************************************************
** Function: sdp_setConnectionAtIndexInMedia
** Description: set connection line at specified index in Media line
** Parameters:
**		media (IN/OUT)	- Sdp Media
**		connection(IN)	- connection line to set
**		index (IN)		- index to set connection in
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setConnectionAtIndexInMedia _ARGS_(( SdpMedia *media,  \
		SdpConnection *connection,SIP_U32bit index, SipError *err ));

/***********************************************************************
** Function: sdp_insertConnectionAtIndexInMedia
** Description: insert connection line at specified index in Media line
** Parameters:
**		media (IN/OUT)	- Sdp Media
**		connection(IN)	- connection line to insert
**		index (IN)		- index to insert connection in
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_insertConnectionAtIndexInMedia _ARGS_(( SdpMedia *media,\
		SdpConnection *connection,SIP_U32bit index,SipError *err ));

/***********************************************************************
** Function: sdp_deleteConnectionAtIndexInMedia
** Description: delete connection at specified index in Media line
** Parameters:
**		media (IN/OUT)	- Sdp Media
**		index (IN)		- index to insert connection in
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_deleteConnectionAtIndexInMedia _ARGS_(( SdpMedia *media,\
		SIP_U32bit index, SipError *err ));

/***********************************************************************
** Function: sdp_getAttrCountFromMedia
** Description: get number of attribute lines in media line
** Parameters:
**		media (IN)			- Sdp Media
**		index (OUT)			- number of attribute lines in media
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getAttrCountFromMedia _ARGS_(( SdpMedia *media, \
		SIP_U32bit	*index, SipError *err  ));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sdp_getAttrAtIndexFromMedia
** Description: get attribute line at specifed index from Media line
** Parameters:
**		media (IN/OUT)		- Sdp Media
**		attr (OUT)			- retrieved attribute line
**		index (IN)			- index to retrieve from
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getAttrAtIndexFromMedia _ARGS_(( SdpMedia *media,SdpAttr **attr, \
		SIP_U32bit index,  SipError *err ));

#else
/***********************************************************************
** Function: sdp_getAttrAtIndexFromMedia
** Description: get attribute line at specifed index from Media line
** Parameters:
**		media (IN)		- Sdp Media
**		attr (OUT)		- retrieved attribute line
**		index (IN)		- index to retrieve from
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_getAttrAtIndexFromMedia _ARGS_(( SdpMedia *media,SdpAttr *attr, \
		SIP_U32bit index,  SipError *err ));

#endif
/***********************************************************************
** Function: sdp_setAttrAtIndexInMedia
** Description: set attribute line at specifed index in Media line
** Parameters:
**		media (IN/OUT)		- Sdp Media
**		attr (IN)			- attribute line to set
**		index (IN)			- index to set at
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_setAttrAtIndexInMedia _ARGS_(( SdpMedia  *media, SdpAttr	*attr, \
		SIP_U32bit index,SipError *err ));

/***********************************************************************
** Function: sdp_insertAttrAtIndexInMedia
** Description: insert attribute line at specifed index in Media line
** Parameters:
**		media (IN/OUT)		- Sdp Media
**		attr (IN)			- attribute line to insert at
**		index (IN)			- index to insert at
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_insertAttrAtIndexInMedia _ARGS_(( SdpMedia 	*media, SdpAttr	*attr, \
		SIP_U32bit index,SipError *err ));

/***********************************************************************
** Function: sdp_deleteAttrAtIndexInMedia
** Description:  delete attribute line at specifed index in Media line
** Parameters:
**		media (IN/OUT)		- Sdp Media
**		index (IN)			- index to  delete
**		err	(OUT)			- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sdp_deleteAttrAtIndexInMedia _ARGS_(( SdpMedia *media, \
		SIP_U32bit index, SipError *err ));


/***********************************************************************
** Function: sip_getLengthFromUnknownMessage
** Description: get length of unknow message body
** Parameters:
**		msg(IN)		- Unknown message
**		length(OUT)	- retrieved length
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_getLengthFromUnknownMessage _ARGS_( ( SipUnknownMessage *msg, \
		SIP_U32bit *length, SipError *err));

/***********************************************************************
** Function: sip_getBufferFromUnknownMessage
** Description: get unknown message body
** Parameters:
**		msg (IN)		- Unknown message
**		buffer (OUT)	- retrieved message buffer
************************************************************************/
extern SipBool sip_getBufferFromUnknownMessage _ARGS_(( SipUnknownMessage *msg, \
		SIP_S8bit **buffer, SipError *err));

/***********************************************************************
** Function: sip_setBufferInUnknownMessage
** Description: set unknown message body
** Parameters:
**		msg (IN/OUT)	- unknown message
**		buffer (IN)		- buffer to set
**		length (IN)		- length of buffer
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_setBufferInUnknownMessage _ARGS_(( SipUnknownMessage *msg, \
		SIP_S8bit *buffer, SIP_U32bit length,SipError *err));

/***********************************************************************
** Function: sip_getMsgBodyType
** Description: get type of message body that is present
** Parameters:
**		body (IN)		- sip message body
**		type (OUT)		- type of message body retrieved
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_getMsgBodyType _ARGS_(( SipMsgBody	*body, \
		en_SipMsgBodyType *type, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getUnknownFromMsgBody
** Description: get unknown message body element from sip message body
** Parameters:
**		msg (IN)		- sip message body
**		unknown(OUT)	- unknown message body retrieved
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUnknownFromMsgBody _ARGS_( ( SipMsgBody *msg, \
		SipUnknownMessage **unknown, SipError *err));

#else
/***********************************************************************
** Function: sip_getUnknownFromMsgBody
** Description: get unknown message body element from sip message body
** Parameters:
**		msg (IN)		- sip message body
**		unknown(OUT)	- unknown message body retrieved
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUnknownFromMsgBody _ARGS_( ( SipMsgBody *msg, \
		SipUnknownMessage *unknown, SipError *err));

#endif
/***********************************************************************
** Function: sip_setUnknownInMsgBody
** Description: set unknown message body  in sip message body
** Parameters:
**		msg (IN/OUT)- sip message body
**		unknown(IN)	- unknown message body  to set
**		err	(OUT)	- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_setUnknownInMsgBody _ARGS_( ( SipMsgBody *msg, \
		SipUnknownMessage *unknown, SipError *err));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getSdpFromMsgBody
** Description: get sdp body from sip message body
** Parameters:
**		msg (IN)	- sip message body
**		sdp (OUT)	- retrieved sdp message body
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_getSdpFromMsgBody _ARGS_(( SipMsgBody *msg, \
		SdpMessage **sdp, SipError *err));

#else
/***********************************************************************
** Function: sip_getSdpFromMsgBody
** Description: get sdp body from sip message body
** Parameters:
**		msg (IN)	- sip message body
**		sdp (OUT)	- retrieved sdp message body
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_getSdpFromMsgBody _ARGS_(( SipMsgBody *msg, \
		SdpMessage *sdp, SipError *err));

#endif
/***********************************************************************
** Function: sip_setSdpInMsgBody
** Description: set sdp body in sip message body
** Parameters:
**		msg (IN/OUT)	- sip message body
**		sdp (IN)		- sdp message body to set
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern SipBool sip_setSdpInMsgBody _ARGS_(( SipMsgBody *msg, \
		SdpMessage *sdp, SipError *err));


/***********************************************************************
** Function: sdp_getIncorrectLineAtIndex
** Description: get incorrect line at specified index from Sdp Message
** Parameters:
**		msg (IN)		- Sdp Message
**		line (OUT)		- retrieved incorrect line (string)
**		cnt (IN)		- Index at which to retrieve
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getIncorrectLineAtIndex _ARGS_(( SdpMessage *msg, \
		SIP_S8bit **line, SIP_U32bit cnt, SipError *err ));

/********************************************************************
** Function:sdp_deleteIncorrectLineAtIndex
** Description: This function deletes an incorrect SDP line (allowed by
**				the application during decode) at the specified index in
**				an SDP message structure
** Parameters:
**		msg (IN)		- Sdp Message
**		cnt (IN)		- Index at which to delete
**		err	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
SipBool sdp_deleteIncorrectLineAtIndex _ARGS_((SdpMessage *msg, \
			SIP_U32bit cnt, SipError *err ));

/***********************************************************************
** Function: sdp_getIncorrectLineAtIndexFromMedia
** Description: get incorrect line at specified index from Sdp Media
** Parameters:
**		media (IN)		- Sdp Media
**		line (OUT)		- retrieved incorrect line (string)
**		cnt (IN)		- Index at which to retrieve
**		err	(OUT)		- Possible error value (see API ref doc)
************************************************************************/
extern 	SipBool sdp_getIncorrectLineAtIndexFromMedia _ARGS_(( SdpMedia *media, \
		SIP_S8bit **line, SIP_U32bit cnt, SipError *err ));

/********************************************************************
** Function:sdp_deleteIncorrectLineAtIndexInMedia
** Description: This function deletes an incorrect SDP line (allowed by
**				the application during decode) at the specified index in
**				an SDP media structure
** Parameters:
**		media (IN)		- Sdp Media
**		cnt (IN)		- Index at which to delete
**		err	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
SipBool sdp_deleteIncorrectLineAtIndexInMedia _ARGS_((SdpMedia *media, \
			SIP_U32bit cnt, SipError *err ));

#ifdef SIP_ATM
/********************************************************************
** Function:sdp_getConnectionIDFromMedia
** Description: This function gets the ConnectionID from the Sdp Media
**
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		ppVirtualID (OUT)	- Connection ID, if existed.
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_getConnectionIDFromMedia _ARGS_(( SdpMedia *pMedia, \
			SIP_S8bit **ppVirtualID, SipError *pErr ));

/********************************************************************
** Function:sdp_setConnectionIDFromMedia
** Description: This function sets the ConnectionID from the Sdp Media
**
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		pVirtualID (IN)	- Connection ID to be set
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_setConnectionIDInMedia _ARGS_(( SdpMedia *pMedia, \
			SIP_S8bit *pVirtualID, SipError *pErr ));

/********************************************************************
** Function:sdp_getProtoFmtCountFromMedia
** Description: This function gets the count of Protocol-Format
**								from the Sdp Media
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		pCount (OUT)	- Count of Protocol-Format Pair
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_getProtoFmtCountFromMedia _ARGS_(( SdpMedia *pMedia, \
			SIP_U32bit *pCount, SipError *pErr ));

#ifdef SIP_BY_REFERENCE
/********************************************************************
** Function:sdp_getProtoFmtAtIndexFromMedia
** Description: This function gets the Protocol-Format at a specified
**					index from the Sdp Media
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		dIndex (IN)			- Index at which to get
**		ppProtofmt (OUT)    - Protocol-Format Pair
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_getProtoFmtAtIndexFromMedia _ARGS_(( SdpMedia *pMedia, \
			SipNameValuePair **ppProtofmt, SIP_U32bit dIndex, SipError *pErr ));
#else
/********************************************************************
** Function:sdp_getProtoFmtAtIndexFromMedia
** Description: This function gets the Protocol-Format at a specified
**					index from the Sdp Media
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		dIndex (IN)			- Index at which to get
**		pProtofmt (OUT)     - Protocol-Format Pair
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_getProtoFmtAtIndexFromMedia _ARGS_(( SdpMedia *pMedia, \
			SipNameValuePair *pProtofmt, SIP_U32bit dIndex, SipError *pErr ));
#endif

/********************************************************************
** Function:sdp_setProtoFmtAtIndexInMedia
** Description: This function sets the Protocol-Format at a specified
**					index in the Sdp Media
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		dIndex (IN)			- Index at which to set
**		pProtofmt (IN)     - Protocol-Format Pair
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_setProtoFmtAtIndexInMedia _ARGS_(( SdpMedia *pMedia, \
			SipNameValuePair *pProtoFmt, SIP_U32bit dIndex, SipError *pErr ));
/********************************************************************
** Function:sdp_insertProtoFmtAtIndexInMedia
** Description: This function inserts the Protocol-Format at a specified
**					index in the Sdp Media
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		dIndex (IN)			- Index at which to insert
**		pProtofmt (IN)     - Protocol-Format Pair
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_insertProtoFmtAtIndexInMedia _ARGS_(( SdpMedia *pMedia, \
			SipNameValuePair *pProtoFmt, SIP_U32bit dIndex, SipError *pErr ));
/********************************************************************
** Function:sdp_deleteProtoFmtAtIndexInMedia
** Description: This function inserts the Protocol-Format at a specified
**					index in the Sdp Media
**
** Parameters:
**		pMedia (IN)		- Sdp Media
**		dIndex (IN)			- Index at which to delete
**		pErr	(OUT)		- Possible error value (see API ref doc)
**********************************************************************/
extern 	SipBool sdp_deleteProtoFmtAtIndexInMedia _ARGS_(( SdpMedia *pMedia, \
			SIP_U32bit dIndex, SipError *pErr ));

#endif


/********************************************************************
** FUNCTION:sdp_getSdpType
**
** DESCRIPTION: This functon retrieves the Sdp Type from an SDP
**		message structure
**
**********************************************************************/
SipBool sip_getSdpType
     (SdpMessage *pMsg, SIP_S8bit **pSdpType, SipError *pErr) ;

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
