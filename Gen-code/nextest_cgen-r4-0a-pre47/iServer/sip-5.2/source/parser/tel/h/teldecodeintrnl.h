/***********************************************************************
 ** FUNCTION:
 **            This file contains the  global variables and function 
 **				prototypes of Tel parser
 *********************************************************************
 **
 ** FILENAME:
 ** teldecodeintrnl.h
 **
 ** DESCRIPTION:
 ** extern declarations of Tel.y
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 
 ** 5-1-2001   Mahesh Govind			Initial creation	Support For Tel Url
 ** 19-01-2001  T.Seshashayi            added funtions for escaping 
 **					and unescaping characters
 **
 ** Copyright 2001, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __TEL_DECODEINTRNL_H_
#define __TEL_DECODEINTRNL_H_

#include "sipcommon.h"
#include "telstruct.h"
#include "sipdecodeintrnl.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/*typedef struct 
{
	SIP_S8bit 	*pToken;
	SIP_U16bit 	dLength;
	SIP_S8bit 	dChar;
} SipTokenBuffer;*/


#ifdef YYSTYPE
#undef YYSTYPE
#endif

#define YYSTYPE SipTokenBuffer


extern TelUrl	    * glbTelUrl;

extern SIP_S8bit* escapeCharacters _ARGS_ ((SIP_S8bit* input,\
			SipError* pErr));
extern SIP_S8bit* unescapeCharacters _ARGS_ ((SIP_S8bit* url));
extern int glbSipParserTellex _ARGS_ ((YYSTYPE *));
extern int glbSipParserTellexreal _ARGS_ ((YYSTYPE *));
extern int glbSipParserTelparse _ARGS_ ((void *pParserStruct));
extern void sip_lex_Tel_free _ARGS_((void));
extern int sip_lex_Tel_scan_buffer _ARGS_((SIP_S8bit *pBuffer, SIP_U32bit dLength));
extern void sip_lex_Tel_release_buffer _ARGS_((void));
extern int glbSipParserTelerror _ARGS_((const char *s));
extern void sip_lex_Tel_reset_state(void);

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif /*__TEL_DECODEINTRNL_H */
