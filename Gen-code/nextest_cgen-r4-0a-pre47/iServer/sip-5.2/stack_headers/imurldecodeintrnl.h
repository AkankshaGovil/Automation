/***********************************************************************
 ** FUNCTION:
 **	       This file contains the  global variables and function
 **				prototypes of IM-URL parser

 *********************************************************************
 **
 ** FILENAME:
 ** imurldecodeintrnl.h
 **
 ** DESCRIPTION:
 ** extern declarations of Im.y
 **
 ** DATE	NAME			REFERENCE	    REASON
 ** ----	----			---------	    --------
 **
 ** 15-1-2002	Manoj Chauhan			Initial creation	Support For Im Url
 **
 ** Copyright 2002, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __IMURLDECODEINTRNL_H_
#define __IMURLDECODEINTRNL_H_

#include "sipcommon.h"
#include "imppstruct.h"
#include "sipdecodeintrnl.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


#ifdef YYSTYPE
#undef YYSTYPE
#endif

#define YYSTYPE SipTokenBuffer


extern ImUrl	    * glbImUrl;

extern int glbSipParserImlex _ARGS_ ((YYSTYPE *));
extern int glbSipParserImlexreal _ARGS_ ((YYSTYPE *));
extern int glbSipParserImparse _ARGS_ ((void *pParserStruct));
extern void sip_lex_Im_free _ARGS_((void));
extern int sip_lex_Im_scan_buffer _ARGS_((SIP_S8bit *pBuffer, SIP_U32bit dLength));
extern void sip_lex_Im_release_buffer _ARGS_((void));
extern int glbSipParserImerror _ARGS_((const char *s));
extern void sip_lex_Im_reset_state(void);


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif /*__IMURLDECODEINTRNL_H */
