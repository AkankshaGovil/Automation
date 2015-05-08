/******************************************************************************
 ** FUNCTION:
 **	 	This file has all the SIP Instant Messaging and Presence Related 
 **     APIS for freeing structures
 ******************************************************************************
 **
 ** FILENAME:
 ** 		imppfree.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE			NAME				REFERENCE	REASON
 ** ----			----				--------	------
 ** 16/4/2001		Subhash Nayak U.	Original
 **
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#ifndef _IMPPFREE_H_
#define _IMPPFREE_H_

#include "sipfree.h"
#include "imppstruct.h"
#include "portlayer.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern void sip_impp_freeSipEventHeader _ARGS_((SipEventHeader *e));

extern void sip_impp_freeSipAllowEventsHeader _ARGS_((SipAllowEventsHeader *a));

extern void sip_impp_freeSipSubscriptionStateHeader _ARGS_((SipSubscriptionStateHeader *s));
extern void __sip_impp_freeSipEventHeader _ARGS_((SIP_Pvoid e));

extern void __sip_impp_freeSipAllowEventsHeader _ARGS_((SIP_Pvoid a));
extern void __sip_impp_freeSipSubscriptionStateHeader _ARGS_((SIP_Pvoid s));

extern void sip_freeImUrl _ARGS_((ImUrl *pUrl));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif

