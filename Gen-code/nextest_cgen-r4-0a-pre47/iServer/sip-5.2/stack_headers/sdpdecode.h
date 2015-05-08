/***********************************************************************
 ** FUNCTION:
 **		   
 *********************************************************************
 **
 ** FILENAME:
 ** sdpdecode.h
 **
 ** DESCRIPTION:
 ** 
 ** DATE	NAME			REFERENCE	    REASON
 ** ----	----			---------	    --------
 **
 ** Copyright 2002, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __SDPDECODE_H_
#define __SDPDECODE_H_

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 * sip_decodeSdpMessage : Decodes a string containing an SDP message.    *
 *                        The SDP permission and error callbacks will    *
 *                        be invoked if selective parsing is enabled.    *
 *                                                                       *
 * ppSdpMessage : pointer to a pointer to a SdpMessage structure         *
 *                Need not be allocated or initialized.                  *
 *                User must free the SdpMessage structure later          *
 * buffer       : Null terminated buffer containing the SDP message      *
 ************************************************************************/
extern SipBool sip_decodeSdpMessage(SdpMessage **ppSdpMessage, char *buffer,\
	SipError *err);

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif


#endif
