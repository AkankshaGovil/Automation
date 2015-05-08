/******************************************************************************
 ** FUNCTION:
 **	 	This file has all the SIP Instant Messaging and Presence Related 
**              Structures

 ******************************************************************************
 **
 ** FILENAME:
 ** 		imppstruct.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE		NAME				REFERENCE	REASON
 ** ----		----				--------	------
 ** 16/4/2001	Subhash Nayak U.	Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#ifndef __IMPPSTRUCT_H__
#define __IMPPSTRUCT_H__

#include "sipstruct.h"
#include "siplist.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

typedef struct
{
	SIP_S8bit 	*pEventType;
	SipList		slParams;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipEventHeader;

typedef struct
{
	SIP_S8bit*		pEventType;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipAllowEventsHeader;

typedef struct
{
	SIP_S8bit 		*pSubState;
	SipList 		slParams;	/* sub-exp params */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipSubscriptionStateHeader;

typedef struct 
{
	SIP_S8bit*		pDispName; 	/* phrase go here */
	SipList			slRoute;   	/* route go here */
	SIP_S8bit* 		pUser;		/* local-part go here */
	SIP_S8bit*		pHost;   	/* domain go here */
	SipList			slParams;   /* header params go here*/
	SIP_RefCount    dRefCount; 
} ImUrl;

typedef struct
{
	SipList		*pGCList;
	SipError	*pError;
	ImUrl		*pImUrl;
} SipImParserParam;


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif

