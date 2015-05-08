/******************************************************************************
 ** FUNCTION:
 **	 	This file has all the SIP Instant Messaging and Presence Related 
 **     APIS for freeing structures
 ******************************************************************************
 **
 ** FILENAME:
 ** 		presstruct.h
 **
 ** DESCRIPTION:
 **	    This function describes all the presurl related structures.
 **
 ** DATE	NAME	       REFERENCE	   REASON
 ** -------------------------------------------------------------------------
 ** 03-12-03    Jyoti        Release 5.2 SRDS    Initial Creation
 **
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#ifndef __PRESSTRUCT_H_
#define __PRESSTRUCT_H_

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

# include "sipcommon.h"
# include "siplist.h"
# include "sipstruct.h"

typedef struct 
{
	SIP_S8bit*		pDispName; 	
	SipList			slRoute;   	
	SIP_S8bit* 		pUser;		
	SIP_S8bit*		pHost;   	
	SipList			slParams;   
	SIP_RefCount    dRefCount; 
} PresUrl;

extern void sip_freePresUrl _ARGS_((PresUrl *pUrl));
extern SipBool sip_initPresUrl _ARGS_((PresUrl **ppPres,SipError *pErr));

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
