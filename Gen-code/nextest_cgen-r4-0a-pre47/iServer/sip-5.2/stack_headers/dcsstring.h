
/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes off all DCS SIP Structure  
**      accessing String APis.
**
*******************************************************************************
**
** FILENAME:
** 	dcsstring.h
**
** DESCRIPTION:
**  THIS FILE IS USED INTERNALLY BY THE STACK
**	IT SHOULD NOT BE INCLUDED DIRECTLY BY THE USER
**
** DATE    	 NAME           REFERENCE      REASON
** ----    	 ----           ---------      ------
** 03Dec00	S. Luthra			Creation
**					
** Copyrights 2000, Hughes Software Systems, Ltd.
*******************************************************************************/


#ifndef __DCSSTRING_H__
#define __DCSSTRING_H__

#include "sipstruct.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

SipBool sip_dcs_getDcsTypeFromString _ARGS_ ((SIP_S8bit *pHdrName, en_HeaderType *pType, SipError *pErr));

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
