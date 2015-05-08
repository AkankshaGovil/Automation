
/**************************************************************************
** FUNCTION:
**  This file contains the prototypes of the message formation APIs
**
***************************************************************************
**
** FILENAME:
**  sipformmessage.h
**
** DESCRIPTION
**
**
**  DATE           NAME                       REFERENCE
** 17Nov99  	 Arjun RC					 Initial
**
**
** Copyright 1999, Hughes Software Systems, Ltd.
***************************************************************************/
#ifndef __SIP_FORMMESSAGE_H_
#define __SIP_FORMMESSAGE_H_

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 
/*
 * temporary buffer size of 20 byte 
 */
#define SIP_TEMP_BUF_SIZE 20

/*****************************************************************
** Function: sip_formMessage
** Description: Forms a text message from a message structure.
** Parameters:
**		s(IN): The message structure that is to be converted to
**			text.
**		options(IN): The options to be used for forming the text
**			message. Multiple options can be ORed. The supported 
**			options are:
**			SIP_OPT_SINGLE: Each header is formed in a separate line.
**				The message will not contain comma separated headers.	
**				Cannot be used with SIP_OPT_COMMASEPARATED.
**			SIP_OPT_COMMASEPARATED: Multiple instances of the same
**				kind of header will appear on the same header line
**				separated by commas.
**				Cannot be used with SIP_OPT_SINGLE.
**			SIP_OPT_SHORTFORM: Single-letter short forms will be used
**				for headers	that have short names.
**				Cannot be used with SIP_OPT_SULLFORM.
**			SIP_OPT_FULLFORM: All hedernames will appear in full.
**				Cannot be used with SIP_OPT_SHORTFORM.
**			SIP_OPT_CLEN: A Content-Length header with the correct
**				length is inserted in the message being formed. If
**				the message structure has a Content-Length header,
**				the function corrects the length field depending on
**				size of the message body.
**			SIP_OPT_REORDERHOP: All hop to hop headers will be placed
**				above the end to end headers.
**			SIP_OPT_AUTHCANONICAL: All headers after the Authorization
**				header will be formed in the canonical form. This
**				will override options SIP_OPT_SHORTFORM and
**				SIP_OPT_COMMASEPARATED from the Authorization header
**				onwards.
**			SIP_OPT_NOSTARTLINE: allow messages without
**				request-line or status-line.
**			SIP_OPT_MAXBUFSIZE: This option allows the user to pass the
**				the maximum possible size of the message that is going to get
**				formed. If this option is used then the dLength parameter
**				contains the size of the preallocated buffer. The stack
**				will do boundary checks using this value while forming the
**				message. In the absence of this option the stack uses the
**				value SIP_MAX_MSG_SIZE for doing the boundary checks.
**				
**		out(OUT): A preallocated buffer that will contain the formed
**			text message.
**		dLength(IN/OUT): This will contain the length of the message formed.
**			(Also in case the SIP_OPT_MAXBUFSIZE option is used this parameter
**			will function as an input parameter and will inform the API of the
**			size of the message buffer that is passed in the parameter out)
**		err(OUT): The error code is returned in this of the function 
**			fails.
**
*******************************************************************/
extern SipBool sip_formMessage _ARGS_ ((SipMessage *s, \
	SipOptions *options, SIP_S8bit *out, \
		SIP_U32bit *dLength, SipError *err ));

/*****************************************************************
** Function: sip_formAddrSpec
** Description: Forms an address specification text from an AddrSpec
**		structure.
** Parameters:
**		out(OUT): A preallocated buffer that will contain the 
**			Addr-Spec string.
**		aspec(IN): The AddrSpec structure to be converted to string.
**		err(OUT): Contains the error code if the function fails.
**
*******************************************************************/
extern SipBool sip_formAddrSpec _ARGS_ ((SIP_S8bit* pEndBuff,SIP_S8bit 	**out, \
	SipAddrSpec 	*aspec, SipError 	*err));

/*****************************************************************
** Internal functions not used by the application directly
*******************************************************************/
extern SipBool sip_formEachHeader _ARGS_ ((SIP_S8bit* pEndBuff,SIP_U32bit cpiter,\
	SipMessage *s, \
		SIP_S8bit **out,SIP_U32bit siplistmap[],SipError *err));

extern SipBool sip_formSingleHeader _ARGS_ ((SIP_S8bit* pEndBuff,en_HeaderType hdrtype,\
	SIP_U32bit ndx,en_AdditionMode mode,\
		SipMessage *s,SIP_S8bit **out,SipError *err));

extern SipBool sip_formSingleGeneralHeader _ARGS_ ((SIP_S8bit* pEndBuff,en_HeaderType hdrtype,\
	SIP_U32bit ndx,en_AdditionMode mode,en_HeaderForm form,\
		SipGeneralHeader *g,SIP_S8bit **out,SipError *err));

extern SipBool sip_formSingleResponseHeader _ARGS_ ((SIP_S8bit* pEndBuff,en_HeaderType hdrtype,\
	SIP_U32bit ndx,en_AdditionMode mode,en_HeaderForm form,\
		SipRespHeader *g,SIP_S8bit **out,SipError *err));

extern SipBool sip_formSingleRequestHeader _ARGS_ ((SIP_S8bit* pEndBuff,en_HeaderType hdrtype,\
	SIP_U32bit ndx,en_AdditionMode mode,en_HeaderForm form,\
		SipReqHeader *g,SIP_S8bit **out,SipError *err));

extern SipBool sip_formMimeBody _ARGS_ ((SIP_S8bit* pEndBuff,SipList mbodyList, \
	SipContentTypeHeader *ctypehdr,SIP_S8bit **out, \
		SIP_U32bit *length,SipError *err ));


SipBool __sip_verifyTypeAny( en_HeaderType Type, SipError *err);

SipBool __sip_setHeaderCountInHeaderLine ( SipMessage *msg,\
          SIP_U32bit  line,  SIP_U32bit	  count,\
          SipError *err ); 

SipBool __sip_getHeaderCountFromHeaderLine ( SipMessage *msg,\
          SIP_U32bit line, SIP_U32bit	*count,\
	 	   SipError *err );

SipBool __sip_getHeaderFormFromHeaderLine( SipMessage   *msg,\
		  SIP_U32bit  line, en_HeaderForm	*dTextType,\
		  SipError *err );

SipBool __sip_setHeaderFormAtHeaderLine ( SipMessage  *msg,\
          SIP_U32bit    line,  en_HeaderForm   dTextType,\
          SipError  *err );
		  
SipBool __sip_getHeaderTypeAtHeaderLine ( SipMessage *msg, \
	  SIP_U32bit	line, en_HeaderType		*dType,\
	  SipError *err );

SipBool sip_formSipParamList (SIP_S8bit* pEndBuff , SIP_S8bit 	**out,\
	SipList 	*list, SIP_S8bit 	*separator,\
	SIP_U8bit	leadingsep, SipError 	*err);

SipBool sip_formDateStruct (SIP_S8bit* pEndBuff,SIP_S8bit **out, SipDateStruct *dt,\
	SipError *err);

SipBool sip_formSdpBody (SIP_S8bit* pEndBuff,SdpMessage *s, SIP_S8bit **out, \
	  SipError *err);
#ifdef SIP_MWI
SipBool sip_formMesgSummaryBody (SIP_S8bit* pEndBuff,MesgSummaryMessage *m,\
				SIP_S8bit **out,  SipError *err);
#endif

SipBool __checkHeaderTypeHop (en_HeaderType dType);

SipBool sip_formSingleRequest (SIP_S8bit* pEndBuff,en_HeaderType dType,\
	 SIP_U32bit ndx, en_AdditionMode mode, en_HeaderForm form,\
	 SipMessage *s, SIP_S8bit **out, SipError *err);
	 
SipBool sip_formSingleResponse (SIP_S8bit* pEndBuff,en_HeaderType dType,\
	 SIP_U32bit ndx, en_AdditionMode mode, en_HeaderForm form,\
	 SipMessage *s, SIP_S8bit **out, SipError *err);

SipBool __sip_getHeaderPositionFromIndex (SipMessage *msg,\
		en_HeaderType dType, SIP_U32bit list_index,\
	 SIP_U32bit *abs_line, SIP_U32bit *position, SipError *err);

SipBool sip_areThereParamsinAddrSpec (SipAddrSpec *aspec,\
	SipError	*err);

#ifndef SIP_MSGBUFFER_CHECK
SipBool __sip_check(char **pInput,char *pOutput);
#else
SipBool __sip_check(char *e,char **pInput,char *pOutput);
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
