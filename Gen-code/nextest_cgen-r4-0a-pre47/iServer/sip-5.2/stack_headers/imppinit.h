
/***********************************************************************
 ** FUNCTION:
 **             Has Init Function prototypes For all IMPP Structures

 *********************************************************************
 **
 ** FILENAME:
 ** 		imppinit.h
 **
 ** DESCRIPTION:
 ** 		This file contains code to init all IMPP structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 16Apr01   Subhash Nayak U.			Initial Creation
 **
 **
 **     Copyright 2001, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __IMPPINIT_H__
#define __IMPPINIT_H__

#include "sipstruct.h"
#include "sipcommon.h"
#include "imppstruct.h"
#include "imppfree.h"
#include "imppinit.h"
#include "sipinit.h"
#include "portlayer.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef SIP_IMPP

extern SipBool sip_impp_initSipEventHeader _ARGS_((SipEventHeader **e, \
	SipError *err));

extern SipBool sip_impp_initSipAllowEventsHeader _ARGS_((SipAllowEventsHeader **a,\
	SipError *err));

extern SipBool sip_impp_initSipSubscriptionStateHeader _ARGS_((SipSubscriptionStateHeader **s, SipError *err));

extern SipBool sip_initImUrl _ARGS_((ImUrl **ppIm,SipError *pErr));



#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
