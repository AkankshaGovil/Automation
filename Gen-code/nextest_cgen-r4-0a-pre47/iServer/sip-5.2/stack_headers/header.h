/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes of all Generic SIP Header
**      Manipulation APIs.
**
*******************************************************************************
**
** FILENAME:
** 	generic1csb.h
**
** DESCRIPTION:
**  	This header file contains the protypes of all the Generic Header
**      manipulation APIs. This header file is to be included by the SU
**      application which wants to use the services of the SIP stack
**
** DATE      NAME           REFERENCE      REASON
** ----      ----           ---------      ------
** 19Dec99   S.Luthra	    Creation
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/

#ifndef _HEADER_H_
#define _HEADER_H_


#include "sipstruct.h"
#include "sipcommon.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


/***********************************************************************
** Function: sip_getHeaderCount 
** Description: Gets the number of headers of a given type in the message.
**		If the header is not present, the function returns SipSuccess with
**		count set to 0.
** Parameters:
**		msg(IN): The message from which the count is to be retrieved.
**		type(IN): The type of the header for which the count is to be
**			extracted. The type must be an Any type for headers like
**			Contact or Expires which have types.
**		count(OUT): The number of header present in the message. 
**		err(OUT): Error code in case the function fails.
**
************************************************************************/
extern SipBool sip_getHeaderCount _ARGS_ ((SipMessage *msg, \
		en_HeaderType type, SIP_U32bit *count, SipError *err));

/***********************************************************************
** Function: sip_getHeaderAtIndex 
** Description: Gets the header of the desired type at the given index.
** Parameters:
**	msg(IN): The message from which the header is to be retrieved.
**	type(IN): The type of the header to be retrieved. The type must
**		be an Any type for headers like Contact or Expires.
**	hdr(OUT): The retrieved header.
**	index(IN): The index of the header to be retrieved.
**	err(OUT): The error code is returned in this if the function fails.
**
************************************************************************/
#ifdef SIP_BY_REFERENCE
extern SipBool sip_getHeaderAtIndex _ARGS_ ((SipMessage *msg, \
		en_HeaderType type,  SipHeader *hdr, SIP_U32bit index, SipError *err));

#else
extern SipBool sip_getHeaderAtIndex _ARGS_ ((SipMessage *msg, \
		en_HeaderType type,  SipHeader *hdr, SIP_U32bit index, SipError *err));

#endif
/***********************************************************************
** Function: sip_insertHeaderAtIndex 
** Description: Insert a header in the message at the specified index.
** Parameters:
**	msg(IN/OUT): The message in which the header is to be inserted.
**	hdr(IN): The header to be inserted.
**	index(IN): The index at which the header is to be inserted.
**	err(OUT): The error code is returned in this if the function fails.
**
************************************************************************/
extern SipBool sip_insertHeaderAtIndex _ARGS_ ((SipMessage *msg, \
		SipHeader *hdr, SIP_U32bit index, SipError *err));

/***********************************************************************
** Function: sip_setHeaderAtIndex 
** Description: Set a header at the specified index in the message.
** Parameters:
**	msg(IN/OUT): The message in which the header is to be set.
**	hdr(IN): The header to be set.
**	index(IN): The index at which the header is to be set.
**	err(OUT): The error code is returned in this if the function fails.
**
************************************************************************/
extern SipBool sip_setHeaderAtIndex _ARGS_ ((SipMessage *msg, \
		SipHeader *hdr, SIP_U32bit index, SipError *err));

/***********************************************************************
** Function: sip_deleteHeaderAtIndex 
** Description: Deletes a header from the message at the specified index.
** Parameters:
**	msg(IN/OUT): The message from which the header is to be deleted.
**	type(IN): The type of the header to be deleted.
**	index(IN): The index of the header to be deleted.
**	err(OUT): The error code is returned in this if the function fails.
**
************************************************************************/
extern SipBool sip_deleteHeaderAtIndex _ARGS_ ((SipMessage *msg, \
		en_HeaderType type, SIP_U32bit index, SipError *err));

/***********************************************************************
** Function: sip_deleteAllHeaderType 
** Description: Deletes all headers of a particular type.
** Parameters:
**	msg(IN/OUT): The message from which the header is to be deleted.
**	type(IN): The type of the header to be deleted.
**	err(OUT): The error code is returned in this if the function fails.
**
************************************************************************/
extern SipBool sip_deleteAllHeaderType _ARGS_ ((SipMessage *msg, \
		en_HeaderType type, SipError *err));

/***********************************************************************
** Function: sip_getMessageType 
** Description: Retrieve the type of the message (Response/Request).
** Parameters:
**		msg(IN): The message for which the type is to be determined.
**		type(OUT): The type of the message. (SipMessageResponse or
**			SipMessageRequest)
**		err(OUT): The error code if the function fails.
**
************************************************************************/
extern SipBool sip_getMessageType _ARGS_ ((SipMessage *msg, \
		en_SipMessageType *type, SipError *err));

/***********************************************************************
** Function: sip_getStatusLineFromSipRespMsg 
**			 sip_getStatusLine
** Description: Retrieves the status line from the messge if it is 
**		a response message.
** Parameters:
**		msg(IN): The message from which the status line is to be retrieved.
**		line(OUT): The retrieved status line.
**		err(OUT): The error code if the function fails.
**
************************************************************************/
#define sip_getStatusLine sip_getStatusLineFromSipRespMsg
#ifdef SIP_BY_REFERENCE
extern SipBool sip_getStatusLineFromSipRespMsg _ARGS_ ((SipMessage *msg, \
		SipStatusLine **line, SipError *err));

#else
extern SipBool sip_getStatusLineFromSipRespMsg _ARGS_ ((SipMessage *msg, \
		SipStatusLine *line, SipError *err));

#endif
/***********************************************************************
** Function: sip_setStatusLineInSipRespMsg 
**			 sip_setStatusLine 
** Description: Set the status line in the message.
** Parameters:
**		msg(IN/OUT): The response message in which the status line is to
**			be set. 
**		line(IN): The status line to be set.
**		err(OUT): The error code if the function fails.
**
************************************************************************/
#define sip_setStatusLine sip_setStatusLineInSipRespMsg
extern SipBool sip_setStatusLineInSipRespMsg _ARGS_ ((SipMessage *msg, \
		SipStatusLine *line, SipError *err));

/***********************************************************************
** Function: sip_getReqLineFromSipReqMsg 
**			 sip_getReqLine
** Description: Get the request line from a request message.
** Parameters:
**		msg(IN): The message from which the request line is to be 
**			retrieved.
**		line(OUT): The retrieved request line.
**		err(OUT): The error code if the function fails.
**
************************************************************************/
#define sip_getReqLine sip_getReqLineFromSipReqMsg
#ifdef SIP_BY_REFERENCE
extern SipBool sip_getReqLineFromSipReqMsg _ARGS_ ((SipMessage *msg, \
		SipReqLine **line, SipError *err));

#else
extern SipBool sip_getReqLineFromSipReqMsg _ARGS_ ((SipMessage *msg, \
		SipReqLine *line, SipError *err));

#endif
/***********************************************************************
** Function: sip_setReqLineInSipReqMsg 
**			 sip_setReqLine
** Description: Sets the request line in a request message
** Parameters:
**		msg(IN/OUT): The message in which the request line is to be set.
**		line(IN): The request line to be set.
**		err(OUT): The error code if the function fails.
**
************************************************************************/
#define sip_setReqLine sip_setReqLineInSipReqMsg
extern SipBool sip_setReqLineInSipReqMsg _ARGS_ ((SipMessage *msg, \
		SipReqLine *line, SipError *err));

/***********************************************************************
** Function: sip_setHeader 
** Description: Same as setHeaderAtIndex with index 0.
************************************************************************/
extern SipBool sip_setHeader _ARGS_( (SipMessage *msg, \
		SipHeader *hdr, SipError *err));

/***********************************************************************
** Function: sip_getHeader 
** Description: Same as sip_getHeaderAtIndex with index 0.
************************************************************************/
#ifdef SIP_BY_REFERENCE
extern SipBool sip_getHeader _ARGS_((SipMessage *msg, en_HeaderType type,  \
		SipHeader *hdr, SipError *err));
#else
extern SipBool sip_getHeader _ARGS_((SipMessage *msg, en_HeaderType type,  \
		SipHeader *hdr, SipError *err));

#endif

/***********************************************************************
** Function: sip_setHeaderFromStringAtIndex 
** Description: Sets a header in a message from a string.
** Parameters:
**	msg(IN/OUT): The message in which the header is to be set.
**	type(IN): The type of the header to be set. Use Any types
**		for headers with types.
**	hdr(IN): The string containing the header to be set.
**	index(IN): The position at which the header is to be set.
**	err(OUT): The error code if the function fails.
**
************************************************************************/
extern SipBool sip_setHeaderFromStringAtIndex _ARGS_ ((SipMessage *msg, en_HeaderType type, \
		SIP_S8bit *hdr, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_insertHeaderFromStringAtIndex 
** Description: Inserts a header in a message from a string.
** Parameters:
**	msg(IN/OUT): The message in which the header is to be inserted.
**	type(IN): The type of the header to be inserted. Use Any types
**		for headers with types.
**	hdr(IN): The string containing the header to be inserted.
**	index(IN): The position at which the header is to be set.
**	err(OUT): The error code if the function fails.
**
************************************************************************/
extern SipBool sip_insertHeaderFromStringAtIndex _ARGS_ ((SipMessage *msg, \
		en_HeaderType type, SIP_S8bit *hdr, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getHeaderAsStringAtIndex 
** Description: Retrieves the header from the message in textual form.
** Parameters:
**		msg(IN): The message from which the header is to be extracted.
**		type(IN): The type of the header to be retrieved.
**		hdr(OUT): The string containing the textual form of the header.
**		index(IN): The index of the header to be retrieved.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_getHeaderAsStringAtIndex _ARGS_ ((SipMessage *msg, \
		en_HeaderType type,  SIP_S8bit **hdr, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getHeaderLineCount 
** Description: Gets the number of header lines in a message.
** Parameters:
**		msg(IN): The message from which the line count is to be retrieved.
**		count(OUT): Contains the number of header lines in the message.
**			This is different from the number of headers in a message.
**			A message may having comma separated headers has fewer header
**			lines than headers. The request/status line is not included
**			in this count.
**		err(OUT): The error code if the function fails.
**
************************************************************************/
extern SipBool sip_getHeaderLineCount _ARGS_ ((SipMessage *msg, \
		SIP_U32bit *count, SipError *err));


/***********************************************************************
** Function: sip_getHeaderPositionFromIndex 
** Description: Retrieves the exact location of a header in a message.
** Parameters:
**		msg(IN): The message in which the header position is to be 
**			determined.
**		type(IN): The type of the header for which the position is to be
**			determined.
**		list_index(IN): The index of the header.
**		abs_line(OUT): The line in the message where the header is present.
**		position(OUT): The index of the header within the header line.
**
************************************************************************/
extern SipBool sip_getHeaderPositionFromIndex _ARGS_ ((SipMessage *msg, \
		en_HeaderType type, SIP_U32bit list_index, SIP_U32bit *abs_line, \
				SIP_U32bit *position, SipError *err));


/***********************************************************************
** Function: sip_getHeaderIndexFromPosition 
** Description: Retrieves the index of a header at a position in the message.
** Parameters:
**		msg(IN): The message from which the index is to be retrieved.
**		abs_line(IN): The header line being probed.
**		position(IN): The position of the header in the header line.
**		index(OUT): The index of the header in the position.
**		err(OUT): Error code if the function fails.
**
************************************************************************/
extern SipBool sip_getHeaderIndexFromPosition _ARGS_ ((SipMessage *msg, \
		SIP_U32bit abs_line, SIP_U32bit position, SIP_U32bit *index, SipError *err));


/***********************************************************************
** Function: sip_getHeaderTypeAtHeaderLine 
** Description: Gets the type of the header at a given line in the message.
** Parameters:
**		msg(IN): The message from which the information is to be retrieved.
**		line(IN): The header line to be probed.
**		type(OUT): The type of the header found at the line.
**		err(OUT): The error code if the function fails.
**
************************************************************************/
extern SipBool sip_getHeaderTypeAtHeaderLine _ARGS_ ((SipMessage *msg, \
		SIP_U32bit line, en_HeaderType *type, SipError *err));


/***********************************************************************
** Function: sip_getHeaderCountFromHeaderLine 
** Description:	Retrieves the number of headers found in a header line
** Parameters:
**		msg(IN): The message from which the information is being retrieved.
**		abs_line(IN): The header line being probed.
**		count(OUT): The number of headers in the requested line.
**		err(OUT): Error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_getHeaderCountFromHeaderLine _ARGS_ ((SipMessage *msg, \
		SIP_U32bit abs_line, SIP_U32bit *count, SipError *err));


/***********************************************************************
** Function: sip_getHeaderFormFromHeaderLine 
** Description: Gets the form (short name or full name) of the header at
**		a given line of a message.
** Parameters:
**		msg(IN): The message from which the information is being retrieved.
**		abs_line(IN): The header line from which the form is to be retrieved.
**		type(OUT): The form of the header at the given line.
**		err(OUT): Error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_getHeaderFormFromHeaderLine _ARGS_ ((SipMessage *msg, \
		SIP_U32bit abs_line, en_HeaderForm *type, SipError *err ));


/***********************************************************************
** Function: sip_setHeaderFormAtHeaderLine 
** Description: Sets the form (short name or full name) of the header at
**		the given line.
** Parameters:
**		msg(IN/OUT): The message in which the header form is to be set.
**		abs_line(IN): The header line to be modified.
**		type(IN): The form to be set.
**		err(OUT): Error returned if the function fails.
**
************************************************************************/
extern SipBool sip_setHeaderFormAtHeaderLine _ARGS_ ((SipMessage *msg, \
		SIP_U32bit abs_line, en_HeaderForm type, SipError *err ));


/***********************************************************************
** Function: sip_insertHeaderAtPosition 
** Description: Inserts a header in the message at a given position.
** Parameters:
**		msg(IN/OUT): The message in which the header is to be inserted.
**		hdr(IN): The header to be inserted.
**		abs_line(IN): The header line at which the header is to be inserted.
**		position(IN): The position in the line where the header is to be
**			inserted.
**		mode(IN): Whether the insertion is to result in a new header line
**			or comma separated one.
**		err(OUT): The error code if the function fails.
**
************************************************************************/
extern SipBool sip_insertHeaderAtPosition _ARGS_ ((SipMessage *msg, \
		SipHeader *hdr,  SIP_U32bit abs_line, SIP_U32bit position, \
				en_AdditionMode mode, SipError *err));


/***********************************************************************
** Function: sip_insertHeaderFromStringAtPosition 
** Description: Inserts a header from a string at a given position.
** Parameters:
**		hdr(IN): The header to be inserted in text form.
**		Other parameters similar to sip_insertHeaderAtPosition.
**
************************************************************************/
extern SipBool sip_insertHeaderFromStringAtPosition _ARGS_ ((SipMessage *msg, \
		SIP_S8bit *hdr,en_HeaderType t,  SIP_U32bit abs_line, \
				SIP_U32bit position, en_AdditionMode, SipError *err));


/***********************************************************************
** Function: sip_deleteHeaderAtPosition 
** Description: Delete a header at given position in the message
** Parameters:
**		msg(IN/OUT): The message from which the header is to be deleted.
**		abs_line(IN): The header line from which the header is to be 
**			deleted.
**		position(IN): The position in the header line from which the 
**			header is to be deleted.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_deleteHeaderAtPosition _ARGS_ ((SipMessage *msg, \
		SIP_U32bit abs_line, SIP_U32bit position, SipError *err));


/***********************************************************************
** Function: sip_deleteHeaderLine 
** Description: Delete all headers in a header line from a message.
** Parameters:
**		msg(IN/OUT): The message from which the line is to be deleted.
**		abs_line(IN): The header line which is to be deleted.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_deleteHeaderLine _ARGS_ ((SipMessage *msg, \
		SIP_U32bit abs_line, SipError *err));


/***********************************************************************
** Function: sip_getMsgBodyAtIndex 
** Description: Gets the message body at the specified index from the message.
** Parameters:
**		s(IN): The message from which the message body is to be retrieved.
**		msgbody(OUT): The retrieved message body.
**		index(IN): The index from which the message body is to be retrieved.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
#ifdef SIP_BY_REFERENCE
extern SipBool sip_getMsgBodyAtIndex _ARGS_((SipMessage *s, \
		SipMsgBody **msgbody, SIP_U32bit index, SipError *err));
#else
extern SipBool sip_getMsgBodyAtIndex _ARGS_((SipMessage *s, \
		SipMsgBody *msgbody, SIP_U32bit index, SipError *err));
#endif

/***********************************************************************
** Function: sip_setMsgBodyAtIndex 
** Description: Sets the message body at the given index in the message.
** Parameters:
**		s(IN/OUT): The message in which the body is being set.
**		msgbody(IN): The message body being set.
**		index(IN): The index at which the body is to be set.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_setMsgBodyAtIndex _ARGS_((SipMessage *s, \
		SipMsgBody *msgbody, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_insertMsgBodyAtIndex 
** Description: Inserts the message body at the given index in the message.
** Parameters:
**		s(IN/OUT): The message in which the body is being inserted.
**		msgbody(IN): The message body being inserted.
**		index(IN): The index at which the body is to be inserted.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_insertMsgBodyAtIndex _ARGS_((SipMessage *s, \
		SipMsgBody *msgbody, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteMsgBodyAtIndex 
** Description: Deletes a message body at a give index.
** Parameters:
**		s(IN/OUT): The message from which the body is to be deleted.
**		index(IN): The index of the message body to be deleted.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_deleteMsgBodyAtIndex _ARGS_((SipMessage *s, \
		SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getMsgBodyTypeAtIndex 
** Description: Gets the type of the message body at the given index.
** Parameters:
**		s(IN): The message from which the type is to be determined.
**		type(OUT): The type of the message body 
**			SipSdpBody - The body contains an SDP message.
**			SipIsupBody - The body contains an ISUP message.
**			SipMultipartMimeBody - The body conatiain a Multipart-MIME body.
**			SipUnknownBody - An unknown type.
**		index(IN): The index at which the type is to be probed.
**
************************************************************************/
extern SipBool sip_getMsgBodyTypeAtIndex _ARGS_((SipMessage *s, \
		en_SipMsgBodyType *type, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getTypeFromMsgBody 
** Description: Gets the type of the body from a retrieved message body
** Parameters:
**		s(IN) - The message body being probed.
**		dType(OUT): The type of the message body
**			SipSdpBody - The body contains an SDP message.
**			SipIsupBody - The body contains an ISUP message.
**			SipMultipartMimeBody - The body conatiain a Multipart-MIME body.
**			SipUnknownBody - An unknown type.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_getTypeFromMsgBody _ARGS_((SipMsgBody *s, \
		en_SipMsgBodyType *dType, SipError *err));


/***********************************************************************
** Function: sip_getMsgBodyCount 
** Description: Gets the number of message body elements in a message
** Parameters:
**		s(IN): The message from which the count is to be retrieved.
**		count(OUT): The number of message body elements in the message.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
extern SipBool sip_getMsgBodyCount _ARGS_((SipMessage *s, \
		SIP_U32bit *count, SipError *err));

/***********************************************************************
** Function: sip_getIncorrectHeadersCount
** Description: This function retrieves the number of incorrect 
**				headers (accepted by the application at the time of
**				decode in the callback "sip_acceptIncorrectHeader")
**				from the SipMessage "s". The output is in the variable 
**				"count".
** Parameters:
**		s(IN): The message from which the count is to be retrieved.
**		count(OUT): The number of incorrect headers that were present in 
**					the incoming message.
**		err(OUT): The error code returned if the function fails.
**
************************************************************************/
SipBool sip_getIncorrectHeadersCount _ARGS_((SipMessage *s, \
					SIP_U32bit *count, SipError *err));

/********************************************************************
**
** FUNCTION:  sip_getEntityErrorCount
**
** DESCRIPTION: This function retrieves the number of incorrect 
**				errors encountered when parsing the entity body
**				of the SIP message. Partially parsed messages are
**				returned by the deocode with the SIP_OPT_BADMESSAGE
**				option.
** PARAMETERS:
**		s(IN): The message from which the count is to be ectracted
**		count(OUT): The number of entity body related errors in the message
**		err(OUT): The error code returned in case of failure.
**
*********************************************************************/
extern SipBool sip_getEntityErrorCount _ARGS_((SipMessage *s, SIP_U32bit *count,\
	SipError *err));

/********************************************************************
**
** FUNCTION:  sip_parseUnknownHeader
**
** DESCRIPTION: This function parses an unknown header using the grammar
**	of a known header type. If parsing is successful, it returns the 
**	a list of parsed headers.
** 
** PARAMETERS:
**	pHeader(IN) - An unknown header structure retrieved from a SIP message.
**	type(IN) - The type of the known header whose grammar is to be used 
**		for parsing the unknown header.
**	pParsedHeaderList(OUT) - An initialized list in which the parsed headers
**		will be returned. The result is returned in a list since the 
**		unknown header might contain multiple instances of the header
**		separated by commas. The list will contain SipHeader structures.
**		The type of the SipHeader structures will be the type using which
**		the uknown header has been parsed and will support all accessor
**		API for that header.
**	pError(OUT) - Error indication in case of failure.
**
*********************************************************************/
extern SipBool sip_parseUnknownHeader _ARGS_((SipHeader *pHeader,\
	en_HeaderType type, SipList *pParsedHeaderList, SipError *pError));


/********************************************************************
**
** FUNCTION:  sip_formUnknownHeader
**
** DESCRIPTION: This function forms an unknown header structure from a 
**	list of known headers. This is useful for inserting headers unrecognized
**	by the stack but complying to a grammar similar to that of a known header.
**	The headers are formed using structures of the known header type. After
**	this the header structures are converted to an unknown header structure
**	suitable for insertion in a message structure.
**
** PARAMETERS:
**	pKnownHeaderList(IN) - A list of SipHeader structures containing the
**		the headers that need to be converted to unknown header suitable
**		for insertion in the SIP message.
**	pName(IN) - The name to be used in the unknown header
**	pUnknownHeader(IN/OUT) - A SipHeader structure in which the unknown header
**		formed will be returned. This should be initialized to type
**		SipHdrTypeUnknown before the function is invoked.
**	pError(OUT) - Returns the error code if the function fails.
**
*********************************************************************/
extern SipBool sip_formUnknownHeader _ARGS_((SipList *pKnownHeaderList,\
	const SIP_S8bit *pName, SipHeader *pUnknownHeader, SipError *pError));

/********************************************************************
**
** FUNCTION:  sip_setSipMessageInMsgBody
**
** DESCRIPTION:  This function sets the SIP message element  in
**	the SipMsgBody structure. 
**
*********************************************************************/
extern SipBool sip_setSipMessageInMsgBody _ARGS_((SipMsgBody *msg,\
	SipMessage *sipmesg, SipError *err));


/********************************************************************
**
** FUNCTION:  sip_getSipMessageFromMsgBody
**
** DESCRIPTION:  This function returns the SIP message element from 
**	the SipMsgBody structure. This function should be used only
**	it the type of the message-body structure is SipAppSipBody
**
*********************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getSipMessageFromMsgBody _ARGS_(( SipMsgBody *msg,\
	SipMessage *sipmesg, SipError *err));
#else
extern SipBool sip_getSipMessageFromMsgBody _ARGS_(( SipMsgBody *msg,\
	SipMessage **sipmesg, SipError *err));
#endif

/********************************************************************
**
** FUNCTION:  sip_getBadHeaderCount
**
** DESCRIPTION: This function returns the number of bad headers in the 
**	SIP message. Messages with bad headers are returned when a syntactically
**	incorrect message is passed to sip_decodeMessage with the SIP_OPT_BADMESSAGE
**	option.
** PARAMETERS:
**	pMessage (IN) - The SIP message that is to be examined for the presence of
**		bad headers
**	pCount (OUT) - Returns the number of bad headers in the message.
**	pError (OUT) - Returns the error code in case of failure.
**
*********************************************************************/
extern SipBool sip_getBadHeaderCount _ARGS_((SipMessage *pMessage,\
	SIP_U32bit *pCount, SipError *pError));

/********************************************************************
**
** FUNCTION:  sip_getBadHeaderAtIndex
**
** DESCRIPTION: This function returns a bad header from a SIP message
**	Messages with bad headers are returned when a syntactically
**	incorrect message is passed to sip_decodeMessage with the 
**	SIP_OPT_BADMESSAGE option.
** PARAMETERS:
**	pMessage (IN) - The SIP message that is to be examined for the presence of
**		bad headers
**	ppBadHdr (OUT) - Returns the bad header from the message.
**	index (IN) - This index of the bad header to be returned.
**	pError (OUT) - Returns the error code in case of failure.
**
*********************************************************************/
extern SipBool sip_getBadHeaderAtIndex _ARGS_((SipMessage *pMessage,\
	SipBadHeader **ppBadHdr, SIP_U32bit index, SipError *pError));

/********************************************************************
**
** FUNCTION:  sip_parseSingleHeader
**
** DESCRIPTION: This function parses a header string to form a SipHeader
**		structure containing the parsed header. This function can only
**		be used to parse known headers.
** 
** PARAMETERS:
**	pParsedHeaderList(IN) - The string containing the header to be parsed.
**	dType(IN) - The type of of the header string that is to be parsed.
**		For types like Contact or Expires, the Any type should be used.
**		The function will return fail if this type does not match with 
**		the type of the header given in the string.
**	pHeader(OUT) - The header structure which will contain the parsed
**		header on return. This structure should be initialized using
**		the sip_initSipHeader function with header type SipHdrTypeAny.
**	pError(OUT) - Error indication in case of failure.
**
*********************************************************************/

extern SipBool sip_parseSingleHeader _ARGS_((SIP_S8bit *pHeaderStr,\
	en_HeaderType dType, SipHeader *pHeader, SipError *err));
/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
