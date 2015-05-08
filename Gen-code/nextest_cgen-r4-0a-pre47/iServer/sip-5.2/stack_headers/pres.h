/******************************************************************************
 ** FUNCTION:
 **	 	This file has all API definitions of Instant Messaging and
 **     Presence Related Structures
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		pres.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE	NAME		REFERENCE	REASON
 ** ------------------------------------------------------
 ** 03-12-03    Jyoti     Release 5.2 SRDS    Initial Creation
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#ifndef __PRES_H_
#define __PRES_H_

#include "sipstruct.h"
#include "presstruct.h"
#include "portlayer.h"
#include "sipinternal.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_IM_URL_SIZE 2048
/***********************************************************************
** Function: sip_isPresUrl 
** Description: Checks if the Addrspec has a pres-url
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_isPresUrl _ARGS_ ((SipAddrSpec *pAddrSpec, \
		SipError *pErr));

/*********************************************************
** FUNCTION:sip_clonePresUrl
**
** DESCRIPTION:  This function makes a deep copy of the fileds from 
**	the PresUrl structures "from" to "to".
** Parameters:
**	to (OUT)		- PresUrl
**	from (IN)		- PresUrl which has to be cloned
**	pErr (OUT)		- Possible Error value (see API ref doc)
**
**********************************************************/

extern SipBool sip_clonePresUrl _ARGS_ ((PresUrl *pTo,\
	PresUrl *pFrom, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getPresUrlFromAddrSpec 
** Description: gets the PresUrl from the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		ppPresUrl (OUT)	- retrieved PresUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getPresUrlFromAddrSpec _ARGS_ ((SipAddrSpec *pAddrSpec,\
	PresUrl **ppPresUrl, SipError *pErr));
#else
/***********************************************************************
** Function: sip_getPresUrlFromAddrSpec 
** Description: gets the PresUrl from the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		pPresUrl (OUT)	- retrieved PresUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getPresUrlFromAddrSpec _ARGS_ ((SipAddrSpec *pAddrSpec,\
	PresUrl *pPresUrl, SipError *pErr));
#endif

/***********************************************************************
** Function: sip_setPresUrlInAddrSpec 
** Description: sets the PresUrl in the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec								
**		pPresUrl (OUT)	- retrieved PresUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setPresUrlInAddrSpec _ARGS_ ((SipAddrSpec *pAddrSpec,\
	PresUrl *pPresUrl, SipError *pErr));

/***********************************************************************
** Function: sip_getDispNameFromPresUrl 
** Description: gets the Disp Name from the PresUrl structure
** Parameters:
**		pUrl (IN)		- PresUrl
**		ppDispName (OUT)- retrieved Disp Name
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getDispNameFromPresUrl _ARGS_((PresUrl *pUrl, \
	SIP_S8bit **ppDispName, SipError *pErr));

/***********************************************************************
** Function: sip_setDispNameInPresUrl
** Description: sets the DispName in the PresUrl structure
** Parameters:
**		pUrl (IN/OUT)	- PresUrl
**		pDispName (OUT)	- Disp Name to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setDispNameInPresUrl _ARGS_ ((PresUrl *pUrl, \
	SIP_S8bit *pDispName, SipError *pErr));

/***********************************************************************
** Function: sip_getUserFromPresUrl 
** Description: gets the User Name from the PresUrl structure
** Parameters:
**		pUrl (IN)		- PresUrl
**		ppUser (OUT)	- retrieved User Name
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getUserFromPresUrl _ARGS_((PresUrl *pUrl, \
	SIP_S8bit **ppUser, SipError *pErr));

/***********************************************************************
** Function: sip_setUserInPresUrl
** Description: sets the User in the PresUrl structure
** Parameters:
**		pUrl (IN/OUT)	- PresUrl
**		pUser (OUT)		- User Name to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setUserInPresUrl _ARGS_ ((PresUrl *pUrl, \
	SIP_S8bit *pUser, SipError *pErr));

/***********************************************************************
** Function: sip_getHostFromPresUrl 
** Description: gets the Host Name from the PresUrl structure
** Parameters:
**		pUrl (IN)		- PresUrl
**		ppHost (OUT)	- retrieved Host
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getHostFromPresUrl _ARGS_((PresUrl *pUrl, \
	SIP_S8bit **ppHost, SipError *pErr));

/***********************************************************************
** Function: sip_setHostInPresUrl
** Description: sets the Host in the PresUrl structure
** Parameters:
**		pUrl (IN/OUT)	- PresUrl
**		pHost (OUT)		- Host to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setHostInPresUrl _ARGS_ ((PresUrl *pUrl, \
	SIP_S8bit *pHost, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromPresUrl
** Description: gets the number of parameters in PresUrl
** Parameters:
**		pPresUrl (IN)		- PresUrl 
**		pCount (OUT)	- number of parameters
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getParamCountFromPresUrl _ARGS_ ((PresUrl \
	*pPresUrl, SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE 
/*****************************************************************************
** Function: sip_getParamAtIndexFromPresUrl
** Description: gets the Param at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN)		- PresUrl
**	ppParam(OUT)	- retreived Parameter 
**	dIndex (IN)		- index at which param is to be retieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_getParamAtIndexFromPresUrl _ARGS_ ((PresUrl \
	*pPresUrl, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#else
/*****************************************************************************
** Function: sip_getParamAtIndexFromPresUrl
** Description: gets the Param at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN)		- PresUrl
**	pParam(OUT)		- retreived Parameter
**	dIndex (IN)		- index at which param is to be retieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_getParamAtIndexFromPresUrl _ARGS_ ((PresUrl \
	*pPresUrl, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/*****************************************************************************
** Function: sip_setParamAtIndexInPresUrl
** Description: sets the Param at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN/OUT)		- PresUrl	
**	pParam(IN)			- Param to be set
**	dIndex (IN)			- index at which param is set in PresUrl
**	pErr (OUT)			- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_setParamAtIndexInPresUrl _ARGS_ ((PresUrl \
	*pPresUrl, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/******************************************************************************
** Function: sip_insertParamAtIndexInPresUrl
** Description: inserts the Param at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN/OUT)	- PresUrl
**	pParam(IN)		- Param to be inserted
**	dIndex (IN)		- index at which param is inserted in PresUrl
**	pErr (OUT)		- Possible Error value (see API ref doc)
*******************************************************************************/
extern SipBool sip_insertParamAtIndexInPresUrl _ARGS_ ((PresUrl \
	*pPresUrl, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_deleteParamAtIndexInPresUrl
** Description: deletes the param at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN)		- PresUrl	
**	dIndex (IN)		- index at which param is deleted in PresUrl
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPresUrl _ARGS_ ((PresUrl \
	*pPresUrl, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_getRouteCountFromPresUrl
** Description: gets the route count from the PresUrl structure
** Parameters:
**	pPresUrl (IN)		- PresUrl	
**	pCount (OUT)	- number of PresUrl Routes 
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_getRouteCountFromPresUrl _ARGS_ ((\
	PresUrl *pPresUrl, SIP_U32bit *pCount, SipError *pErr));

/*****************************************************************************
** Function: sip_getRouteAtIndexFromPresUrl
** Description: gets the Route at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN)		- PresUrl								
**	ppRoute(OUT)	- retreived AreaSpecifier
**	dIndex (IN)		- index to get Route at
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
#ifdef SIP_BY_REFERENCE
extern SipBool sip_getRouteAtIndexFromPresUrl _ARGS_ ((\
	PresUrl *pPresUrl, SIP_S8bit **ppRoute, SIP_U32bit \
	dIndex, SipError *pErr));
#else
extern SipBool sip_getRouteAtIndexFromPresUrl _ARGS_ ((\
	PresUrl *pPresUrl, SIP_S8bit *ppRoute, SIP_U32bit \
	dIndex, SipError *pErr));
#endif
/******************************************************************************
** Function: sip_setRouteAtIndexInPresUrl
** Description: sets the Route at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN/OUT)	- PresUrl								
**	pRoute(IN)		- AreaSpecifier to set
**	dIndex (IN)		- index to set Route at
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
extern SipBool sip_setRouteAtIndexInPresUrl _ARGS_ ((\
	PresUrl *pPresUrl, SIP_S8bit *pRoute, SIP_U32bit\
	dIndex, SipError *pErr));

/*******************************************************************************
** Function: sip_insertRouteAtIndexInPresUrl
** Description: inserts the Route at the specified index in PresUrl
** Parameters:
**		pPresUrl (IN/OUT)	- PresUrl								
**		pRoute(IN)		- Route to insert
**		dIndex (IN)		- index to insert Route at
**		pErr (OUT)		- Possible Error value (see API ref doc)
*******************************************************************************/
extern SipBool sip_insertRouteAtIndexInPresUrl _ARGS_ ((\
	PresUrl *pPresUrl, SIP_S8bit *pRoute, SIP_U32bit \
	dIndex, SipError *pErr));

/*******************************************************************************
** Function: sip_deleteRouteAtIndexInPresUrl 
** Description: deletes the Route at the specified index in PresUrl
** Parameters:
**		pPresUrl (IN/OUT)	- PresUrl
**		dIndex (IN)		- index to delete Route at
**		pErr   (OUT)	- Possible Error value (see API ref doc)
******************************************************************************/
extern SipBool sip_deleteRouteAtIndexInPresUrl _ARGS_ ((\
	PresUrl *pPresUrl, SIP_U32bit dIndex, SipError *pErr));



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 


#endif/* #ifndef __PRES_H_ */

