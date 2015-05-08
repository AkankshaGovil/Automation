
/*
** THIS FILE IS USED INTERNALLY BY THE STACK
**	THE USER SHOULD NOT INCLUDE THIS DIRECTLY
*/

/******************************************************************************
** FUNCTION:
** 	 This file contains INTERNAL definitions of sip string apis
**
*******************************************************************************
**
** FILENAME:
** 	sipstring.c
**
** DESCRIPTION:
**  	
**
** DATE      NAME          REFERENCE      REASON
** ----      ----          ---------      ------
** 13Dec99   S.Luthra	   Creation
**	         B.Borthakur
**	         Preethy
** 
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/


#ifndef __SIP_STRING_INTRNL_H_
#define __SIP_STRING_INTRNL_H_

#include "sipstruct.h"
#include "sipcommon.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#ifdef SIP_BY_REFERENCE
 SipBool sip_makeToHeader _ARGS_( ( SIP_S8bit * str, SipToHeader **hdr, SipError *err));
#else
 SipBool sip_makeToHeader _ARGS_( ( SIP_S8bit * str, SipToHeader * hdr, SipError *err));
#endif

extern SipBool sip_makeUnknownHeader _ARGS_( ( SIP_S8bit * str, SipUnknownHeader * hdr, SipError *err));

extern SipBool sip_getTypeFromString _ARGS_(( SIP_S8bit * str, en_HeaderType *type, SipError *err));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
