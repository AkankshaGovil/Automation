/*
THIS FILE IS INCLUDED INTERNALLY BY THE STACK
THE USER SHOULD NOT DIRECTLY INCLUDE THIS FILE
*/

/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	sipbcptinternal.h
**
** DESCRIPTION:
**  	
**	THIS FILE IS USED INTERNALLY BY THE STACK AND SHOULD
** NOT BE USED DIRECTLY BY THE USER
**
** DATE    		  NAME      	    REFERENCE      REASON
** ----     	  ----          	---------      ------
** 10/02/00   	B. Borthakur	  	 Creation
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/
#ifndef _SIP_BCPTINTERNAL_H_
#define _SIP_BCPTINTERNAL_H_

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siplist.h"
#include "sipinternal.h"
#include "sipvalidate.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


SipBool sip_bcpt_cloneMimeMessage _ARGS_((MimeMessage *pDest, MimeMessage *pSource, SipError *pErr));

SipBool sip_bcpt_cloneIsupMessage _ARGS_((IsupMessage *pDest, IsupMessage *pSource, SipError *pErr));
SipBool sip_bcpt_cloneQsigMessage _ARGS_((QsigMessage *pDest, QsigMessage *pSource, SipError *pErr));

SipBool __sip_bcpt_cloneSipMimeVersionHeader _ARGS_((SipMimeVersionHeader *pDest, SipMimeVersionHeader *pSource, SipError *pErr));

SipBool sip_bcpt_cloneSipMimeHeader _ARGS_((SipMimeHeader *pDest, SipMimeHeader *pSource, SipError *pErr));

SipBool __sip_bcpt_cloneSipMimeHeader _ARGS_((SipMimeHeader *pDest, SipMimeHeader *pSource, SipError *pErr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
