/**************************************************************************
** FUNCTION:
**  This file contains the prototypes of the primary decode entry point
**
***************************************************************************
**
** FILENAME:
**  sipdecode.h
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

#ifndef __SIPDECODE_H_
#define __SIPDECODE_H_

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


/**********************************************************
** Function: sip_initStack 
** Description: 
**		Initialize the stack.
**		To be called initially before the stack is passed any 
**		messages.
** Parameters: 
**		None
**
************************************************************/
extern SipBool sip_initStack _ARGS_((void));

/**********************************************************
** Function: sip_releaseStack 
** Description: 
**		Release the stack. To be called when the stack 
**		is no longer required. This call releases memory 
**		allocated for the tokenizers.
** Parameters:
**		None
**
************************************************************/
extern void sip_releaseStack _ARGS_((void));

#ifdef SIP_SELECTIVE_PARSE
/**********************************************************
** Function: sip_willParseHeader 
** Description:
** 		Callback required in the application if the stack
**		is compiled with the SIP_SELECTIVE_PARSE option. May be
**		used by the application to ignore headers or to 
**		correct headers from buggy implementations.
** Parameters:
**		type(IN):	The enumeration giving the type of the header
**			that is about to be parsed
**		hdr(IN/OUT): The header that is about to be parsed.
**			This structure contains the name of the header
** 			and the content of the header body as a atring.
**			The content will be free of linefolds. The content 
**			may be modified in the callback to correct buggy
**			headers.
**	Return values:
**		0	-	The header is thrown away and parsing proceeds.
**		1 	-	The headers is to be parsed normally.
**		2	-	The header is to be treated like an unknown
**				header. It will not be parsed further but will
**				not be thrown away like in return 0.
**
************************************************************/
extern SIP_S8bit sip_willParseHeader _ARGS_((\
	en_HeaderType type, SipUnknownHeader *hdr));

/**********************************************************
** Function: sip_acceptIncorrectHeader 
** Description:	
**		A callback function to be implemented by the
**		application if the stack is compiled with the 
**		SIP_SELECTIVE_PARSE option enabled.
** Parameters:
**		type(IN): The type of the header that had the syntax
**			error.
**		hdr(IN): The structure containing the name and the 
**			text before parsing.
** Return Values:
**		SipSuccess:	Ignore tha error and treat the header
**			like and unknown header.
**		SipFail: Error should not be ignored. Message parsing 
**			will fail.
**
************************************************************/
extern SipBool sip_acceptIncorrectHeader _ARGS_((\
	en_HeaderType type, SipUnknownHeader *hdr));

/**********************************************************
** Function: sip_willParseSdpLine 
** Description:
**		A callback function to be implemented by the
**		application if the stack is compiled with the 
**		SIP_SELECTIVE_PARSE option enabled. Allows ignoring 
**		or correcting grammatically incorrect SDP lines 
**		before being sent to the parser.
** Parameters:
**		line(IN): String containing the SDP line that will
**			be parsed.
**		outline(OUT): String that is to be parsed instead of
**			line.
** Return Values:
**		0	-	Ignore the SDP line. (It is discarded.)
**		1	-	Parse the string the line variable if
**				*outline is SIP_NULL, else parse the string 
**				supplied in outline. (Note: *outline is SIP_NULL
**				if it is not set to anything in the callback.)
**
************************************************************/
extern SIP_S8bit sip_willParseSdpLine _ARGS_((\
	SIP_S8bit *line, SIP_S8bit **outline));

/**********************************************************
** Function: sip_allowIncorrectSdpLine
** Description:
**		A callback function to be implemented by the
**		application if the stack is compiled with the 
**		SIP_SELECTIVE_PARSE option enabled. Allows ignoring
**		grammatically incorrect SDP lines.
** Parameters:
**		line(IN):	The SDP line that generated a syntax error
**			while being parsed.
** Return Values:
**		SipSuccess: Throw the line away and ignore the error.
**		SipFail: Error not to be ignored. Parser returns a fail.
**
************************************************************/
extern SipBool sip_acceptIncorrectSdpLine _ARGS_((\
	SIP_S8bit *line));
#endif


/**********************************************************
** Function: sip_decodeMessage 
** Description:
**		This function decodes the plain text message and invokes
**		an appropriate callback.
** Parameters:
**		message(IN): The text message that is to be parsed.
**		opt(IN): Options to be used while parsing the text message
**			SIP_OPT_NOPARSEBODY - Instructs stack&nbsp; not to parse
**				the message body. In this case, stack treats the body
**				as an unknown body and does not parse it. Useful for
**					proxies which can save time by avoiding body parsing.
**			SIP_OPT_NOPARSESDP - Instructs stack not to parse SDP message
**				bodies. SDP bodies are trated as unknown bodies. Use this
**				option if you are not interested in SDP bodies but wish
**				to parse Multi-part MIME messages.
**			SIP_OPT_NOPARSEAPPSIP - Instructs stack not to parse
**				message/sipfrag mesage bodies. Content with type 
**				message/sipfrag type will be trated as unknown.
**			SIP_OPT_DBLNULLBUFFER - Use this option if you are willing to
**				allow the stack to modify the message buffer passed for 
**				decoding. When using this option, the last two bytes of 
**				the buffer must be set to '\0' and the length passed should
**				not include these double null characters.
**				This option results in fewer parser side memory allocations.
**			SIP_OPT_SELECTIVE - Use this option if you want to select the
**				type of headers that need to be parsed in this invocation 
**				of the function. The selection list to be used for deciding
**				on the header types is passed in the pList field of the 
**				SipEventContext parameter to this function. If this option 
**				is not used, the decode function uses the default header 
**				selection that has been set using the 
**				sip_stackSetDecodeHeaderTypes function.
**			SIP_OPT_BADMESSAGE - This option enables parsing of bad messages.
**				In this mode, the decode function ignores errors in headers 
**				and message bodies. The number of errors encountered are stored
**				in the SIP message. The error count can be extracted using 
**				the sip_getIncorrectHeadersCount and sip_getEntityErrorCount
**				funcitons. The bad headers are also stored in the SIP message.
**				These can be accessed using the sip_getBadHeaderCount and 
**				sip_getBadHeaderAtIndex functions. The decode function will
**				not continue if an error is encountered in the request-line 
**				or the status-line of the message. If the message does not 
**				contain the mandatory headers required for timer handling,
**				the incorrect header count will be incremented. With this 
**				option, the decode function will return with SipSuccess even 
**				if errors are encountered provided it can return a partially
**				parsed message structure.
**			SIP_OPT_NOTIMER - This instructs the decode function not to
**				call timer related callbacks after decoding the message. 
**				Checks for the presence of mandatory headers - From, To,
**				Call-ID and CSeq are not performed with this option. 
**				Use this option if you are not using the stack's retransmission
**				handling feature, or if the message being decoded is not going 
**				to result in any retransmissions being stopped.
**			SIP_OPT_PARTIAL - With this option, the decode function can parse
**				messages that contain only the a request line or status-line. 
**				This is useful for parsing message/sipfrag content-type which
**				may contain messages without headers. The SIP_OPT_NOTIMER 
**				option must be used along with this option to prevent message
**				rejection due to absence of mandatory headers (From, To,
**				Call-ID and CSeq).
**		messageLength(IN): The length of the message to be parsed.
**		nextmesg(OUT): This will contain any trailing text from
**			text passed after the parser parses one SIP message.
**			*nextmessage will be SIP_NULL if the text contained
**			one SIP message. 
**		context(IN): A SipEventContext structure that can be filled
**			by the application with any information. This structure
**			passed on to the indicate callbacks. The structure 
**			contains a field pData (void pointer) that may be used
**			to pass information.
**		err(OUT): The error code returned if the call fails.
**
**		Note: In the case the stack returns a E_INCOMPLETE error in pErr field, 
**		it also returns back the number of bytes more it was expecting in the 
**		message in the pContext->dRemainingLength field.  Note that if the 
**    	user passed EventContext as SIP_NULL, the stack cannot set the 
**		dRemainingBytes field and hence only returns E_INCOMPLETE. 
**
**	 	This case happens when the stack sees a content length which is greater 
**		that the body supplied. 
**		In cases that the stack returns E_MAYBE_INCOMPLETE, it does not set this 
**		field as it does not know how many more bytes to  expect. 
**		This typically happens if some header is incomplete. 
**
**	    The above error values are passed *only if* the stack is compiled 
**		with SIP_INCOMPLETE_MSG_CHECK option.
**
** NOTE: If the user invokes this in the SIP_NO_CALLBACK mode,
** sip_decodeMessage has an extra parameter of pointer to pointer to 
** decoded message. In this sceanrio, the stack returns the decoded message
** into this variable and does not invoke the sip_indicateXXXX methods.
**
************************************************************/

#ifndef SIP_NO_CALLBACK
extern SipBool sip_decodeMessage _ARGS_( (SIP_S8bit *message, \
	SipOptions *opt, SIP_U32bit messageLength, \
		SIP_S8bit **nextmesg, SipEventContext *context, SipError *err) );
#else
extern SipBool sip_decodeMessage _ARGS_( (SIP_S8bit *message, \
	SipMessage **ppDecodedMsg,\
	SipOptions *opt, SIP_U32bit messageLength, \
	SIP_S8bit **nextmesg, SipEventContext *context, SipError *err) );
#endif

/**********************************************************
** Function: sip_decodeMessageWithoutCallback
** Description:
**	This function is available only when the stack is not 
**	compiles without the SIP_NO_CALLBACK option. This function
**	decodes the buffer passed and returns the decoded SIP
**	message is a return parameter instead of invoking the
**	indicate callbacks. This function will however invoke
** 	other call backs related to selective parsing if the
**	stack has been compiled with the SIP_SELECTIVE_PARSE
**	option. This function will also invoke the timer callbacks
**	if required.
************************************************************/

#ifndef SIP_NO_CALLBACK
extern SipBool sip_decodeMessageWithoutCallback _ARGS_( (SIP_S8bit *message, \
	SipMessage **ppDecodedMsg,\
	SipOptions *opt, SIP_U32bit messageLength, \
	SIP_S8bit **nextmesg, SipEventContext *context, SipError *err) );
#endif

/* These are callbacks to be implemented by the user */

/**********************************************************
** Function: sip_indicateInvite 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an INVITE message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateInvite _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicatePublish
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an PUBLISH message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicatePublish _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateRegister 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an REGISTER message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateRegister _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateRefer
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an REFER message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateRefer _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateAck 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an ACK message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateAck _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateCancel 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an CANCEL message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateCancel _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateBye 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an BYE message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateBye _ARGS_((SipMessage *s,  \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateOptions 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an OPTIONS message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateOptions _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateInfo 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an INFO message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateInfo _ARGS_((SipMessage *s, \
	SipEventContext *context));

#ifdef SIP_DCS
/**********************************************************
** Function: sip_indicateComet
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		a COMET message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateComet _ARGS_((SipMessage *s, \
	SipEventContext *context));
#endif

/**********************************************************
** Function: sip_indicatePropose 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an PROPOSE message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicatePropose _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicatePrack 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an PRACK message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicatePrack _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateSubscribe
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an SUBSCRIBE message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateSubscribe _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateNotify
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an NOTIFY message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateNotify _ARGS_((SipMessage *s, \
	SipEventContext *context));


/**********************************************************
** Function: sip_indicateMessage
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an MESSAGE message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateMessage _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateUpdate
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an UPDATE message
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateUpdate _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateUnknownRequest 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		a request with an unknown method.
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateUnknownRequest _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateInformational 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		an informational response (1xx).
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateInformational _ARGS_((SipMessage *s, \
	SipEventContext *context));

/**********************************************************
** Function: sip_indicateFinalResponse 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the message decoded by sip_decodeMessage is
**		a final response (200 and above).
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		context(IN):
**			The event context structure that was passed to the 
**			sip_decodeMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateFinalResponse _ARGS_((SipMessage *s, \
	SipEventContext *context));

#ifdef SIP_MIB
/**********************************************************
** Function: sip_indicateResponseHandled
** Description:
**		Callback function to be implemented by the application.
**		Invoked if the application has registered to receive
**      callbacks whenever a response is sent or decoded.
** Parameters:
**		pMessage(IN): 
**			The structure containig the decoded message.
**	        This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		dEvent(IN):
**			Information so as to whether a response was recvd/sent.
**		dCode(IN):
**			The response code of the message that caused this cbk.
**		dParam(IN):
**			The entire stats for this response code that are maintained
**			by the MIB.
**
************************************************************/
extern void sip_indicateResponseHandled _ARGS_((SipMessage *pMessage, \
		en_SipMibCallbackType dEvent, SIP_S32bit dCode,SipStatParam dParam));
#endif

#ifndef SIP_TXN_LAYER
/**********************************************************
** Function: sip_indicateTimeout 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if a message that was sent through UDP with 
**		retransmissions did not receive a matching response
**		after the maximum retransmissions have been sent.
** Parameters:
**		context(IN):
**			The event context structure that was passed to the 
**			sip_sendMessage invocation that resulted in this
**			callback.
**
************************************************************/
extern void sip_indicateTimeOut _ARGS_ ((SipEventContext *context));
#endif

/**********************************************************
** Function: sip_decryptBuffer 
** Description:
**		Callback function to be implemented by the application.
**		Invoked if a message being decoded contains an Encryption 
**		header and a message body. 
** Parameters:
**		s(IN): The structure containig the decoded message.
**			This must be freed using sip_freeSipMessage call
**			when it is to be released.
**		encinbuffer(IN): The encrypted portion of the message that
**			has to be decrypted by the application. The application
**			must use the encryption algorithm and parameters in the
**			Encryption header in the message for the decryption.
**		clen(IN):
**			Then length of the buffer to be decrypted.
**		encoutbuffer(OUT):
**			This contains the decrypted buffer returned by the 
**			application to the stack
**		outlen(OUT):
**			The length of the decrypted buffer returned by the 
**			application to the stack.
**	Return Values:
**		SipSuccess:	The stack proceeds to parse the decrypted body.
**			It replaces headers in the original message with the 
**			corresponding headers in the decypted buffer and parses 
**			the message body if present in the decrypted buffer.
**		SipFail: The application returns SipFail if it cannot decrypt 
**			the buffer. The stack treats the message body like
**			an unknown type.
**
************************************************************/
extern SipBool sip_decryptBuffer _ARGS_ ((SipMessage *s, \
	SIP_S8bit *encinbuffer, SIP_U32bit clen, SIP_S8bit **encoutbuffer, SIP_U32bit *outlen));

/**********************************************************
** Function: sip_enableAllHeaders
** Description:
**		Enables parsing of all stack header types in the list
** Parameters:
**		pList(IN/OUT) - The list in which all headers are enabled
**
************************************************************/
extern void sip_enableAllHeaders _ARGS_((SipHdrTypeList *pList));

/**********************************************************
** Function: sip_disableAllHeaders
** Description:
**		Disable parsing of all stack header types in the list
**		Useful to initialize the list and then select the desired
**		headers with sip_enableHeader
** Parameters:
**		pList(IN/OUT) - The list in which all headers are disabled
**
************************************************************/
extern void sip_disableAllHeaders _ARGS_((SipHdrTypeList *pList));

/**********************************************************
** Function: sip_enableHeader
** Description:
**		Enables parsing for with the given type.
** Parameters:
**		pList(IN/OUT) - The list in which the header type is to be enabled
**		type(IN) - The header type to be enabled.
**
************************************************************/
extern void sip_enableHeader _ARGS_((SipHdrTypeList *pList, en_HeaderType type));

/**********************************************************
** Function: sip_disableHeader
** Description:
**		Disables parsing for with the given type.
** Parameters:
**		pList(IN/OUT) - The list in which the header type is to be disabled
**		type(IN) - The header type to be disabled.
**
************************************************************/
extern void sip_disableHeader _ARGS_((SipHdrTypeList *pList, en_HeaderType type));

/**********************************************************
** Function: sip_enableHeaders
** Description:
**		Enables parsing for multiple types.
** Parameters:
**		pList(IN/OUT) - The list in which the headers are enabled
**		type(IN) - An array containing types to be enabled.
**		size(IN) - The size of the type array
**
************************************************************/
extern void sip_enableHeaders _ARGS_((SipHdrTypeList *pList, en_HeaderType *type,\
	SIP_U16bit size));

/**********************************************************
** Function: sip_disableHeaders
** Description:
**		Disables parsing for multiple types.
** Parameters:
**		pList(IN/OUT) - The list in which the headers are disabled
**		type(IN) - An array containing types to be disabled.
**		size(IN) - The size of the type array
**
************************************************************/
extern void sip_disableHeaders _ARGS_((SipHdrTypeList *pList, en_HeaderType *type,\
	SIP_U16bit size));

/**********************************************************
** Function: sip_stackSetDecodeHeaderTypes
** Description:
**		Sets the stack's global parsing policy using the supplied list.
**		The header types in the given list are parsed by the stack. If
**		the decodeMessage function is invoked with the OPT_SELECTIVE option,
**		the header-types supplied to the function takes precedence over
*		the list set with this function.
** Parameters:
**		pList(IN/OUT) - The list containing the global/default selection
**			of headers for the stack.
************************************************************/
extern void sip_stackSetDecodeHeaderTypes _ARGS_((SipHdrTypeList *pList));

/* These are some internal functions */
extern SipBool __sip_clonesiptoHeader _ARGS_ ((SipToHeader *dest, SipToHeader *source, SipError *err));
extern SipBool __sip_clonesipfromheader _ARGS_ ((SipFromHeader *dest, SipFromHeader *source, SipError *err));
extern SipBool glbSipParserRemoveFromOrderTable _ARGS_( (SipMessage *s, en_HeaderType type, SipError *err) );
extern void glbSipParserRemoveExistingHeaders _ARGS_((SIP_Pvoid data, SIP_Pvoid pParserParams));

extern void sip_getTypeFromName_astart _ARGS_((SIP_S8bit *pName,\
		en_HeaderType *pType, SipError *pErr));
extern void sip_getTypeFromName_dstart _ARGS_((SIP_S8bit *pName,\
		en_HeaderType *pType, SipError *pErr));
extern void sip_getTypeFromName_mstart _ARGS_((SIP_S8bit *pName,\
		en_HeaderType *pType, SipError *pErr));
extern void sip_getTypeFromName_rstart _ARGS_((SIP_S8bit *pName,\
		en_HeaderType *pType, SipError *pErr));
extern void sip_getTypeFromName_sstart _ARGS_((SIP_S8bit *pName,\
		en_HeaderType *pType, SipError *pErr));

extern SipBool sip_checkMandatory _ARGS_( (SipMessage *s) );
extern SipBool glbSipParserMemStrStr _ARGS_( (SIP_S8bit *buffer, SIP_U32bit \
	buflen, SIP_S8bit *separator, SIP_U32bit *offset) );

extern SipBool glbSipParserParseBody _ARGS_( (SIP_U8bit level, SipOptions *opt,\
	SipContentTypeHeader *ctypehdr, SIP_S8bit *messagebody,SIP_U32bit clen, \
	SipList *msgbodylist, SipError *err) );


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
