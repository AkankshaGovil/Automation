
/* THIS FILE IS INCLUDED INTERNALLY BY THE STACK
** THE USER SHHOULD NOT INCLUDE THIS DIRECTLY
*/

/******************************************************************************
 ** FUNCTION:
 **	 	This file has all internal function definitions of Instant Messaging
 **     and Presence Related Headers
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		imppinternal.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE		NAME				REFERENCE	REASON
 ** ----		----				--------	------
 ** 16/4/2001	Subhash Nayak U.	Original
 **
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/
  
#ifndef _IMPP_INTERNAL_H_
#define _IMPP_INTERNAL_H_

#include "sipstruct.h"
#include "imppstruct.h"
#include "imppfree.h"
#include "imppinit.h"
#include "portlayer.h"
#include "sipdecode.h"
#include "sipinternal.h"
#include "sipdecodeintrnl.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


SipBool __sip_impp_cloneSipEventHeader _ARGS_ ((SipEventHeader *dest, \
	SipEventHeader *source, SipError *err));

SipBool __sip_impp_cloneSipAllowEventsHeader _ARGS_ ((SipAllowEventsHeader \
	*dest, SipAllowEventsHeader *source, SipError *err));

SipBool __sip_impp_cloneSipAllowEventsHeaderList _ARGS_ ((SipList *pDest, \
	SipList *pSource, SipError *pErr));

SipBool __sip_impp_cloneSipSubscriptionStateHeader _ARGS_ ((SipSubscriptionStateHeader \
	*dest, SipSubscriptionStateHeader *source, SipError *err));

SipBool sip_formSipList (SIP_S8bit 	*out,\
	SipList 	*list, SIP_S8bit 	*separator,\
	SIP_U8bit	leadingsep, SipError 	*err);


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
