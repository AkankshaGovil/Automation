/******************************************************************************
** FUNCTION:
**  string APIs for sip headers (get/set headers as strings)	
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
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/


#ifndef __SIP_STRING_H_
#define __SIP_STRING_H_

#include "sipstruct.h"
#include "sipcommon.h"
#include "sipstringintrnl.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#define SIP_DEC_VAL_ASCII_lowercase_a            97
#define SIP_DEC_VAL_ASCII_lowercase_z           122
#define SIP_DEC_VAL_ASCII_uppercase_a            65
#define SIP_DEC_VAL_ASCII_uppercase_z            90
#define SIP_DEC_VAL_ASCII_zero                   48
#define SIP_DEC_VAL_ASCII_nine                    57

#define SIP_DEC_VAL_ASCII_asterik                 42
#define SIP_DEC_VAL_ASCII_plus                    43
#define SIP_DEC_VAL_ASCII_hyphen                  45
#define SIP_DEC_VAL_ASCII_period                  46
#define SIP_DEC_VAL_ASCII_percent                 37
#define SIP_DEC_VAL_ASCII_exclamation             33
#define SIP_DEC_VAL_ASCII_closingquote            39
#define SIP_DEC_VAL_ASCII_openingquote            96
#define SIP_DEC_VAL_ASCII_underscore              95
#define SIP_DEC_VAL_ASCII_tilde                  126
#define SIP_DEC_VAL_ASCII_numbersign              35
#define SIP_DEC_VAL_ASCII_openingsquarebracket    91
#define SIP_DEC_VAL_ASCII_closingsquarebracket    93
#define SIP_DEC_VAL_ASCII_unicode_192            192   
#define SIP_DEC_VAL_ASCII_unicode_248            248
#define SIP_DEC_VAL_ASCII_unicode_251            251  
#define SIP_DEC_VAL_ASCII_unicode_253            252   





/******************************************************************************
** Function: sip_getReqLineAsString
** Description:  gets request line as string from sip message
** Parameters:
**		pS (IN)			- Sip Message
**		ppReqLine (OUT)	- retrieved request line as string
**		pErr (OUT)		- possible error value (see api ref. doc)
********************************************************************************/
extern SipBool sip_getReqLineAsString _ARGS_((SipMessage *pS, SIP_S8bit **ppReqLine, SipError *pErr));

/******************************************************************************
** Function: sip_setReqLineAsString
** Description:  sets request line as string in sip message
** Parameters:
**		pSipMsg (IN/OUT)		- Sip Message
**		line (IN)				- retrieved request line as string
**		pErr (OUT)				- possible error value (see api ref. doc)
********************************************************************************/
extern SipBool sip_setReqLineFromString _ARGS_((SipMessage *pSipMsg, SIP_S8bit *line, SipError *pErr));



/******************************************************************************
** Function: sip_getStatusLineAsString
** Description:  gets status line as string from sip message
** Parameters:
**		pSipMsg (IN)		- Sip Message
**		outStatusLine (OUT)	- retrieved request line as string
**		pErr (OUT)			- possible error value (see api ref. doc)
********************************************************************************/
extern SipBool sip_getStatusLineAsString _ARGS_(( SipMessage *pSipMsg, SIP_S8bit **outStatusLine, SipError *pErr));



/******************************************************************************
** Function: sip_setStatusLineFromString
** Description:  sets status line as string from sip message
** Parameters:
**		pS (IN/OUT)			- Sip Message
**		pStr (IN)			- retrieved request line as string
**		pErr (OUT)			- possible error value (see api ref. doc)
********************************************************************************/

extern SipBool sip_setStatusLineFromString _ARGS_((SipMessage *pS, SIP_S8bit *pStr, SipError *pErr));

#ifdef SIP_BY_REFERENCE

/******************************************************************************
** Function: sip_setAddrSpecFromString
** Description:  sets address specification as string
** Parameters:
**		ppAddrSpec (IN/OUT)	-  Addr Spec to be filled
**		pStr (IN)			- retrieved request line as string
**		pErr (OUT)			- possible error value (see api ref. doc)
********************************************************************************/
extern SipBool sip_setAddrSpecFromString _ARGS_((SipAddrSpec **ppAddrSpec, SIP_S8bit *pStr, SipError *pErr));
#endif

/******************************************************************************
** Function: sip_convertIntoQuotedString
** Description: This API converts a string into a quoted string. Please note 
                that it returns a new memory area allocated. It does not modify
                the original string.
** Parameters:
** pOriginalString (IN)-  The string to be converted
** pErr (OUT)- possible error value (see api ref. doc)
** Return Values:
** SIP_U8bit *  - If successful returns a quoted string.
** SIP_NULL - If failed.
********************************************************************************/
extern SIP_U8bit* sip_convertIntoQuotedString _ARGS_((SIP_U8bit* pOriginalString, 
                                                   SipError *pErr));
extern SIP_U8bit * sip_formQuotedString(SIP_U8bit* pOriginalString, SipError *pError) ;


#define SIP_QUOTEQUOTENUL_SIZE 3

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
