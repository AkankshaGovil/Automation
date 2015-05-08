/******************************************************************
 ** FUNCTION:
 **             This file is the header for Trace functions
 ******************************************************************
 **
 ** FILENAME:
 ** sipcommon.h
 **
 ** DESCRIPTION:
 ** Trace/Error modules implemented here
 **
 ** DATE           NAME                  REFERENCE          REASON
 ** ----          ----                    --------          ------
 ** 19-11-99   Arjun Roychowdhury                          Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *******************************************************************/


#ifndef __SIP_TRACE_H_
#define __SIP_TRACE_H_

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#ifndef SIP_NO_FILESYS_DEP
#include "portlayer.h"


/*================================================================
PrintFunction can be defined to any function that needs to be
used for logging the traces. The default definition will result
in on-screen trace dumps.
==================================================================*/


#ifndef PrintFunction
#define PrintFunction printf
#endif
#else
/*================================================================
For systems without a filesystem:
Tracing is currently defined to an empty function. This may
be redefined to implement custom tracing mechanism.
==================================================================*/
#define PrintFunction sip_printfunc
#endif


extern SipTraceLevel curSipLevel;
extern SipTraceType  curSipType;
extern SipErrorLevel	curSipErrLevel;

extern SipBool  sip_setTraceLevel _ARGS_((SipTraceLevel  lev, SipError *err));
extern SipBool  sip_getTraceLevel _ARGS_((SipTraceLevel  *lev, SipError *err));
extern SipBool  sip_setTraceType _ARGS_((SipTraceType  type, SipError *err));
extern SipBool  sip_getTraceType _ARGS_((SipTraceType  *type, SipError *err));
extern SipBool  sip_trace _ARGS_((SipTraceLevel lev, SipTraceType type, SIP_S8bit *txt));


extern SipBool  sip_setErrorLevel _ARGS_((SipErrorLevel  lev, SipError *err));
extern SipBool  sip_getErrorLevel _ARGS_((SipErrorLevel  *lev, SipError *err));

#ifdef SIP_ERROR
#define sip_error(lev, str) \
do \
{ \
	if (curSipErrLevel & lev ) \
	{ \
		PrintFunction("SIP STACK ERROR:%s\n", str); \
	} \
} while (0)
#else
#define sip_error(lev, str)
#endif
	


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
