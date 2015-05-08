/*******************************************************************************
 **** FUNCTION:
 ****             Has Init Function declarations For all bcpt Structures

 ******************************************************************************
 ****
 **** FILENAME:
 **** sipbcptinit.h
 ****
 **** DESCRIPTION:
 **** This file contains code to init all structures
 ****
 **** DATE        NAME                    REFERENCE               REASON
 **** ----        ----                    ---------              --------
 **** 9/02/00   B. Borthakur       		                    Initial Creation
 ****
 ****
 ****     Copyright 1999, Hughes Software Systems, Ltd.
 *****************************************************************************/
#ifndef _SIP_BCPTINIT_H_
#define _SIP_BCPTINIT_H_

#include <stdlib.h>
#include "sipstruct.h"
#include "sipcommon.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern SipBool sip_bcpt_initIsupMessage _ARGS_((IsupMessage **m,SipError *err));
extern SipBool sip_bcpt_initQsigMessage _ARGS_((QsigMessage **m,SipError *err));
extern SipBool sip_bcpt_initSipMimeVersionHeader _ARGS_((SipMimeVersionHeader **h,SipError *err));
extern SipBool sip_bcpt_initMimeMessage _ARGS_((MimeMessage **m,SipError *err));
extern SipBool sip_bcpt_initSipMimeHeader _ARGS_((SipMimeHeader **h,SipError *err));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
