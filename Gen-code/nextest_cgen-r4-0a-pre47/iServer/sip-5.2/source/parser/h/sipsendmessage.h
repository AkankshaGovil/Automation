
/**************************************************************************
** FUNCTION:
**  This file contains the prototypes of the send message APIs
**
***************************************************************************
**
** FILENAME:
**  sipsendmessage.h
**
** DESCRIPTION
**
**
**  DATE           NAME                       REFERENCE
** 17Nov99  	KSBinu, Arjun RC			 Initial
**
**
** Copyright 1999, Hughes Software Systems, Ltd.
***************************************************************************/
#ifndef _SIP_SENDMESSAGE_H
#define _SIP_SENDMESSAGE_H

#include "sipcommon.h"
#include "sipstruct.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/*****************************************************************
** Function: sip_sendMessage
** Description: Sends a SIP message.
** Parameters:
**		s(IN): The message structure that is to be converted to
**			text and sent.
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
**			SIP_OPT_PERMSGRETRANS: Do not use the default retransmission 
**				interval values. Use the values given in the event
**				context - pContext->dRetransT1 and pContext->dRetransT2
**				for the initial retransmission interval and the cap-off 
**				time respsctively.
**			SIP_OPT_DIRECTBUFFER: This option enables the user to pass
**				on a preallocated buffer to this API that will get used
**				during the formation of the message prior to actually 
**              sending it. This option renders unnecessary an extra malloc
**				within the API, thus leading to possible optimisations.
**
**		addr(IN): A preallocated buffer that will contain the formed
**			text message.
**		transptype(IN): The transport to be used for transmitting the 
**			SIP text message. Possible values are
**			SIP_UDP: UDP retransmission mechanisms are used for the
**				message unless overridden with SIP_NORETRANS.
**				This option will be passed on to the sip_sendToNetwork
**				callback in the application.
**			SIP_TCP: TCP retransmission is used for responses to INVITE.
**				This option will be passed on to the sip_sendToNetwork
**				callback in the application.
**			SIP_NORETRANS: This option when ORed with SIP_UDP or SIP_TCP
**				disables the stack retransmission mechanism for the 
**				message.
**		context(IN): This application may pass any information in this
**			structure though its pData (void pointer) element. This
**			structure is passed to the application in the Time-out 
**			indicate function. This function is called in the event that
**			the message is sent by UDP with retransmissions and the 
**			maximum retransmissions are reached.
**		err(OUT): The error code is returned in this of the function 
**			fails.
**
*******************************************************************/
extern SipBool sip_sendMessage _ARGS_ (( SipMessage *s, \
	SipOptions *options, SipTranspAddr *addr, \
		SIP_S8bit transptype, SipEventContext *context, SipError *err));

/*****************************************************************
** Function: sip_sendToNetwork
** Description: A calback invoked on the application by the stack
**		when a message is to be sent to the network. A call to 
**		sip_sendMessage results in this callback if the message is
**		formed successfully.
** Parameters:
**		buffer(IN): The buffer containing the message that is to be
**			sent across the network.
**		buflen(IN): The length of the message to be sent.
**		addr(IN): The transport address to which the message is to be
**			sent. This will be the same address that is passed to 
**			sip_sendMessage.
**		transptype(IN): The transport type to be used. This is the same
**			parameter that is passed to the sip_sendMessage function.
**		context(IN): Context information that was passed to the 
**			sip_sendMessage call that resulted in this callback.
**		err(OUT): The error code in case of an error.
** Return Values:
**		The application return a SipFail if it cannot send the message
**		across. The stack returns the Value and the error code to 
**		the application in the sip_endMessage call. The retransmission
**		mechanism is not used if the initial send fails.
**		The application returns SipSucess if it succeeds in the network
**		send. The stack will start retransmission mechanisms if required.
**
*******************************************************************/
extern SipBool sip_sendToNetwork _ARGS_ (( SIP_S8bit *buffer, \
	SIP_U32bit buflen,SipTranspAddr *addr, SIP_S8bit transptype, \
		SipError *err));

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
**		out(OUT): A preallocated buffer that will contain the formed
**			text message.
**		length(OUT): This will contain the length of the message formed.
**		err(OUT): The error code is returned in this of the function 
**			fails.
**
*******************************************************************/

extern SipBool sip_formMessage _ARGS_ (( SipMessage *s, \
	SipOptions *options, SIP_S8bit *out , \
		SIP_U32bit *length, SipError *err));

#ifndef SIP_TXN_LAYER
/*******************************************************************
**
** Function: sip_indicateMessageRetransmission
** Description: This callback function is invoked on the application 
**   when the stack retransmits a message. This callback is invoked 
**   only if:
**     a) The stack is compiled with the SIP_RETRANSCALLBACK option.
**     b) The SIP_OPT_RETRANSCALLBACK option was passed to the 
**        sip_sendMessage function.
**   The application should not free or modify any of the pointer 
**   values passed in this callback.
**		
** Parameters:
**		pContext(IN): The EventContext passed to the sip_sendMessage
**			function by the application.
**		pKey(IN): The timer-key associated with the message. This
**			structure contains the From, To, Call-ID and CSeq headers
**			of the message being retransmitted.
**		pBuffer(IN): This contains the text form of the message being
**			retransmitted.
**		dBufferLength(IN): The length of the message in pBuffer.
**		pAddr(IN): Pointer to the address to which the message was
**			dispatched.
**		retransCount(IN): The count of the retransmission being 
**			indicated. This will be 1 for the first retransmission.
**		duration(IN): This gives the time elapsed since the message
**			was last retransmitted and this retransmission. The value
**			is in milli-seconds.
**
*******************************************************************/

extern void sip_indicateMessageRetransmission _ARGS_((SipEventContext\
	*pContext, SipTimerKey *pKey, SIP_S8bit *pBuffer,\
	SIP_U32bit dBufferLength, SipTranspAddr *pAddr, SIP_U8bit retransCount,\
	SIP_U32bit duration));
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
