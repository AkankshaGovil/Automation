/******************************************************************************
 ** FUNCTION:
 **	 	This file has all API definitions of Instant Messaging and
 **     Presence Related Structures
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		impp.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE		NAME				REFERENCE	REASON
 ** ----		----				--------	------
 ** 16/2/2001	Subhash Nayak U.	Original
 **
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#ifndef _IMPP_H_
#define _IMPP_H_

#include "sipstruct.h"
#include "imppstruct.h"
#include "portlayer.h"
#include "sipinternal.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_IM_URL_SIZE 2048

/***********************************************************************
** Function: sip_impp_getEventTypeFromEventHdr 
** Description: get Event Type from Event Header
** Parameters:	
**				hdr(IN)		- Sip Event Header
**				type(OUT)	- Event Type retrieved 
**				err(OUT)  	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_getEventTypeFromEventHdr _ARGS_((SipHeader * hdr,  \
		SIP_S8bit ** type, SipError * err));

/***********************************************************************
** Function: sip_impp_setEventTypeInEventHdr 
** Description: set Event Type in Event Header
** Parameters:	
**				hdr(IN/OUT)	- Sip Event Header
**				type(IN)	- Event Type to set 
**				err(OUT)  	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_setEventTypeInEventHdr _ARGS_((SipHeader * hdr,  \
		SIP_S8bit * type, SipError * err));


/***********************************************************************
** Function: sip_impp_getParamCountFromEventHdr 
** Description: get count of parameters from Event header
** Parameters:
**				pHdr(IN)	- Sip Event Header
**				pCount(IN)		- Count of parameters retrieved 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_getParamCountFromEventHdr _ARGS_(( SipHeader *pHdr, \
		SIP_U32bit *pCount, SipError *pErr ));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_impp_getParamAtIndexFromEventHdr 
** Description: get parameter at index from Event header
** Parameters:
**				pHdr(IN)		- Sip Event Header
**				pParam(OUT)     - Parameter retrieved
**				index(IN)		- index at which parameter is to be retrieved 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_getParamAtIndexFromEventHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam **pParam, SIP_U32bit	index, SipError *pErr ));

#else
/***********************************************************************
** Function: sip_impp_getParamAtIndexFromEventHdr 
** Description: get parameter at index from Event header
** Parameters:
**				pHdr(IN)		- Sip Event Header
**				pParam(OUT)     - Parameter retrieved
**				index(IN)		- index at which parameter is to be retrieved 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_getParamAtIndexFromEventHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam *pParam, SIP_U32bit	index, SipError *pErr ));

#endif
/***********************************************************************
** Function: sip_impp_setParamAtIndexInEventHdr 
** Description: set parameter at index in Event header
** Parameters:
**				pHdr(IN/OUT)	- Sip Event Header
**				pParam(IN)      - Parameter to set
**				index(IN)		- index at which parameter is to be set 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_setParamAtIndexInEventHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam *pParam, SIP_U32bit index, SipError *pErr ));

/***********************************************************************
** Function: sip_impp_insertParamAtIndexInEventHdr 
** Description: insert parameter at index in Event header
** Parameters:
**				pHdr(IN/OUT)	- Sip Event Header
**				pParam(IN)      - Parameter to insert
**				index(IN)		- index at which parameter is to be inserted 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_insertParamAtIndexInEventHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam	*pParam, SIP_U32bit index, SipError *pErr ));

/***********************************************************************
** Function: sip_impp_deleteParamAtIndexInEventHdr 
** Description: delete parameter at index in Event header
** Parameters:
**				pHdr(IN/OUT)	- Sip Event Header
**				index(IN)		- index at which parameter is to be deleted 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_impp_deleteParamAtIndexInEventHdr _ARGS_( ( SipHeader *pHdr, \
		SIP_U32bit index, SipError *pErr ));


/***********************************************************************
** Function:  sip_impp_getEventTypeFromAllowEventsHdr
** Description: Gets the pEventType field from Allow Events pHeader.
** Parameters:
**			hdr(IN)		- Sip Allow Events Header
**			pEventType(OUT)- The pEventType field got
**			pErr(OUT)	- Possible Error value (see API ref doc)
**
**********************************************************************/
extern SipBool sip_impp_getEventTypeFromAllowEventsHdr _ARGS_((SipHeader *hdr,\
			SIP_S8bit **pEventType, SipError *err));

/***********************************************************************
** Function:  sip_impp_setEventTypeInAllowEventsHdr
** Description: Sets the pEventType field in Allow Events pHeader.
** Parameters:
**			hdr(IN/OUT)		- Sip Allow Events Header
**			pEventType(IN)		- The pEventType field  set
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
***********************************************************************/
extern SipBool sip_impp_setEventTypeInAllowEventsHdr _ARGS_((SipHeader *hdr,\
			SIP_S8bit *pEventType, SipError *err));



/***********************************************************************
** Function: sip_impp_getSubStateFromSubscriptionStateHdr
** Description:gets the State field from the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN) 			- Sip Subscription-Expires Header
**			ppSubState(OUT)		- The State field retrieved
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_getSubStateFromSubscriptionStateHdr _ARGS_ ((\
SipHeader *pHdr, SIP_S8bit **ppSubState, SipError *pErr));


/***********************************************************************
** Function: sip_impp_setSubStateInSubscriptionStateHdr
** Description:sets the State field in the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip Subscription-Expires Header
**			pSubState(IN)		- The State field to set
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_setSubStateInSubscriptionStateHdr _ARGS_ ((SipHeader \
		*pHdr, SIP_S8bit *pSubState, SipError *pErr));

/***********************************************************************
** Function: sip_impp_getParamCountFromSubscriptionStateHdr
** Description:gets the number of Parameters from the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN) 			- Sip Subscription-Expires Header
**			count(OUT)			- The Parameter count retrieved
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_getParamCountFromSubscriptionStateHdr _ARGS_ (( \
		SipHeader *pHdr, SIP_U32bit *dCount, SipError *pErr));

#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_impp_getParamAtIndexFromSubscriptionStateHdr
** Description:gets the Parameters at an index from the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN) 			- Sip Subscription-Expires Header
**			pParam(OUT)			- The Parameter retrieved
**			index(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_getParamAtIndexFromSubscriptionStateHdr _ARGS_ (( \
		SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function: sip_impp_getParamAtIndexFromSubscriptionStateHdr
** Description:gets the Parameter at an index from the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN) 			- Sip Subscription-Expires Header
**			pParam(OUT)			- The Parameter retrieved
**			index(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_getParamAtIndexFromSubscriptionStateHdr _ARGS_ ((\
		SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));

#endif

/***********************************************************************
** Function: sip_impp_insertParamAtIndexInSubscriptionStateHdr
** Description:inserts the Parameter at an index in the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip Subscription-Expires Header
**			pParam(IN)			- The Parameter to insert
**			index(IN)			- The index at which the Parameter is inserted
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_insertParamAtIndexInSubscriptionStateHdr _ARGS_ ((\
		SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_impp_deleteParamAtIndexInSubscriptionStateHdr
** Description:deletes the Parameter at an index in the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip Subscription-Expires Header
**			index(IN)			- The index at which the Parameter is deleted
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_deleteParamAtIndexInSubscriptionStateHdr _ARGS_ ((\
		SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_impp_setParamAtIndexInSubscriptionStateHdr
** Description:sets the Parameter at an index in the Sip Subscription-Expires Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip Subscription-Expires Header
**			pParam(IN)			- The Parameter to set
**			index(IN)			- The index at which the Parameter is set
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_impp_setParamAtIndexInSubscriptionStateHdr _ARGS_ ((\
		SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_isImUrl 
** Description: Checks if the Addrspec has a im-url
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_isImUrl _ARGS_ ((SipAddrSpec *pAddrSpec, \
		SipError *pErr));

/*********************************************************
** FUNCTION:sip_cloneImUrl
**
** DESCRIPTION:  This function makes a deep copy of the fileds from 
**	the ImUrl structures "from" to "to".
** Parameters:
**	to (OUT)		- ImUrl
**	from (IN)		- ImUrl which has to be cloned
**	pErr (OUT)		- Possible Error value (see API ref doc)
**
**********************************************************/

extern SipBool sip_cloneImUrl _ARGS_ ((ImUrl *pTo,\
	ImUrl *pFrom, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getImUrlFromAddrSpec 
** Description: gets the ImUrl from the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		ppImUrl (OUT)	- retrieved ImUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getImUrlFromAddrSpec _ARGS_ ((SipAddrSpec *pAddrSpec,\
	ImUrl **ppImUrl, SipError *pErr));
#else
/***********************************************************************
** Function: sip_getImUrlFromAddrSpec 
** Description: gets the ImUrl from the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		pImUrl (OUT)	- retrieved ImUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getImUrlFromAddrSpec _ARGS_ ((SipAddrSpec *pAddrSpec,\
	ImUrl *pImUrl, SipError *pErr));
#endif

/***********************************************************************
** Function: sip_setImUrlInAddrSpec 
** Description: sets the ImUrl in the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		pImUrl (OUT)	- retrieved ImUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setImUrlInAddrSpec _ARGS_ ((SipAddrSpec *pAddrSpec,\
	ImUrl *pImUrl, SipError *pErr));

/***********************************************************************
** Function: sip_getDispNameFromImUrl 
** Description: gets the Disp Name from the ImUrl structure
** Parameters:
**		pUrl (IN)		- ImUrl
**		ppDispName (OUT)- retrieved Disp Name
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getDispNameFromImUrl _ARGS_((ImUrl *pUrl, \
	SIP_S8bit **ppDispName, SipError *pErr));

/***********************************************************************
** Function: sip_setDispNameInImUrl
** Description: sets the DispName in the ImUrl structure
** Parameters:
**		pUrl (IN/OUT)	- ImUrl
**		pDispName (OUT)	- Disp Name to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setDispNameInImUrl _ARGS_ ((ImUrl *pUrl, \
	SIP_S8bit *pDispName, SipError *pErr));

/***********************************************************************
** Function: sip_getUserFromImUrl 
** Description: gets the User Name from the ImUrl structure
** Parameters:
**		pUrl (IN)		- ImUrl
**		ppUser (OUT)	- retrieved User Name
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUserFromImUrl _ARGS_((ImUrl *pUrl, \
	SIP_S8bit **ppUser, SipError *pErr));

/***********************************************************************
** Function: sip_setUserInImUrl
** Description: sets the User in the ImUrl structure
** Parameters:
**		pUrl (IN/OUT)	- ImUrl
**		pUser (OUT)		- User Name to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setUserInImUrl _ARGS_ ((ImUrl *pUrl, \
	SIP_S8bit *pUser, SipError *pErr));

/***********************************************************************
** Function: sip_getHostFromImUrl 
** Description: gets the Host Name from the ImUrl structure
** Parameters:
**		pUrl (IN)		- ImUrl
**		ppHost (OUT)	- retrieved Host
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getHostFromImUrl _ARGS_((ImUrl *pUrl, \
	SIP_S8bit **ppHost, SipError *pErr));

/***********************************************************************
** Function: sip_setHostInImUrl
** Description: sets the Host in the ImUrl structure
** Parameters:
**		pUrl (IN/OUT)	- ImUrl
**		pHost (OUT)		- Host to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setHostInImUrl _ARGS_ ((ImUrl *pUrl, \
	SIP_S8bit *pHost, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromImUrl
** Description: gets the number of parameters in ImUrl
** Parameters:
**		pImUrl (IN)		- ImUrl 
**		pCount (OUT)	- number of parameters
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getParamCountFromImUrl _ARGS_ ((ImUrl \
	*pImUrl, SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE 
/*****************************************************************************
** Function: sip_getParamAtIndexFromImUrl
** Description: gets the Param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN)		- ImUrl
**	ppParam(OUT)	- retreived Parameter 
**	dIndex (IN)		- index at which param is to be retieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_getParamAtIndexFromImUrl _ARGS_ ((ImUrl \
	*pImUrl, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#else
/*****************************************************************************
** Function: sip_getParamAtIndexFromImUrl
** Description: gets the Param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN)		- ImUrl
**	pParam(OUT)		- retreived Parameter
**	dIndex (IN)		- index at which param is to be retieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_getParamAtIndexFromImUrl _ARGS_ ((ImUrl \
	*pImUrl, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/*****************************************************************************
** Function: sip_setParamAtIndexInImUrl
** Description: sets the Param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN/OUT)		- ImUrl	
**	pParam(IN)			- Param to be set
**	dIndex (IN)			- index at which param is set in ImUrl
**	pErr (OUT)			- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_setParamAtIndexInImUrl _ARGS_ ((ImUrl \
	*pImUrl, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/******************************************************************************
** Function: sip_insertParamAtIndexInImUrl
** Description: inserts the Param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN/OUT)	- ImUrl
**	pParam(IN)		- Param to be inserted
**	dIndex (IN)		- index at which param is inserted in ImUrl
**	pErr (OUT)		- Possible Error value (see API ref doc)
*******************************************************************************/
extern SipBool sip_insertParamAtIndexInImUrl _ARGS_ ((ImUrl \
	*pImUrl, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_deleteParamAtIndexInImUrl
** Description: deletes the param at the specified index in ImUrl
** Parameters:
**	pImUrl (IN)		- ImUrl	
**	dIndex (IN)		- index at which param is deleted in ImUrl
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_deleteParamAtIndexInImUrl _ARGS_ ((ImUrl \
	*pImUrl, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_getRouteCountFromImUrl
** Description: gets the route count from the ImUrl structure
** Parameters:
**	pImUrl (IN)		- ImUrl	
**	pCount (OUT)	- number of ImUrl Routes 
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getRouteCountFromImUrl _ARGS_ ((\
	ImUrl *pImUrl, SIP_U32bit *pCount, SipError *pErr));

/*****************************************************************************
** Function: sip_getRouteAtIndexFromImUrl
** Description: gets the Route at the specified index in ImUrl
** Parameters:
**	pImUrl (IN)		- ImUrl								
**	ppRoute(OUT)	- retreived AreaSpecifier
**	dIndex (IN)		- index to get Route at
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_getRouteAtIndexFromImUrl _ARGS_ ((\
	ImUrl *pImUrl, SIP_S8bit **ppRoute, SIP_U32bit \
	dIndex, SipError *pErr));
/******************************************************************************
** Function: sip_setRouteAtIndexInImUrl
** Description: sets the Route at the specified index in ImUrl
** Parameters:
**	pImUrl (IN/OUT)	- ImUrl								
**	pRoute(IN)		- AreaSpecifier to set
**	dIndex (IN)		- index to set Route at
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setRouteAtIndexInImUrl _ARGS_ ((\
	ImUrl *pImUrl, SIP_S8bit *pRoute, SIP_U32bit\
	dIndex, SipError *pErr));

/*******************************************************************************
** Function: sip_insertRouteAtIndexInImUrl
** Description: inserts the Route at the specified index in ImUrl
** Parameters:
**		pImUrl (IN/OUT)	- ImUrl								
**		pRoute(IN)		- Route to insert
**		dIndex (IN)		- index to insert Route at
**		pErr (OUT)		- Possible Error value (see API ref doc)
*******************************************************************************/
extern SipBool sip_insertRouteAtIndexInImUrl _ARGS_ ((\
	ImUrl *pImUrl, SIP_S8bit *pRoute, SIP_U32bit \
	dIndex, SipError *pErr));

/*******************************************************************************
** Function: sip_deleteRouteAtIndexInImUrl 
** Description: deletes the Route at the specified index in ImUrl
** Parameters:
**		pImUrl (IN/OUT)	- ImUrl
**		dIndex (IN)		- index to delete Route at
**		pErr   (OUT)	- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_deleteRouteAtIndexInImUrl _ARGS_ ((\
	ImUrl *pImUrl, SIP_U32bit dIndex, SipError *pErr));



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 


#endif
