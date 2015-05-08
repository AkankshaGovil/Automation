/***********************************************************************
 ** FUNCTION:
 **             Has Init Functions For all Structures

 *********************************************************************
 **
 ** FILENAME:
 ** telinit.h
 **
 ** DESCRIPTION:
 ** This file contains code to init all  tel-url structures
 **
 ** DATE        NAME                    REFERENCE              REASON
 ** ----        ----                    ---------              --------
 ** 4Jan01   	Rajasri 									   Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __TEL_INIT_H_
#define __TEL_INIT_H_

#include <stdlib.h>
#include "sipcommon.h"
#include "telstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern SipBool sip_initTelUrl _ARGS_((TelUrl **ppTel,SipError *pErr));

extern SipBool sip_initTelLocalNum _ARGS_ ((TelLocalNum **ppLocal,SipError \
	*pErr));

extern SipBool sip_initTelGlobalNum _ARGS_((TelGlobalNum **ppGlobal,SipError\
	 *pErr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif /*__TEL_INIT */
