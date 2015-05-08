/*****************************************************************************
 ** FUNCTION:
 **			The HSS SIP Stack performance testing application.
 **
 *****************************************************************************
 **
 ** FILENAME:		sipclient.h
 **
 ** DESCRIPTION:	This file contains the prototypes of the functions
 **					implemented in the file sipclient.h
 **					This file also contains the structure definitions.
 **
 **
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			---------		------
 ** 10-Nov-01		Ramu K							Creation
 ** 30-Mar-02		R.Kamath 						Added prototypes for
 **													getting callid from buffer
 **
 *****************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 *****************************************************************************/
#ifndef __SIPCLIENT_H__
#define __SIPCLIENT_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


#include "sipstruct.h"
#ifdef SIP_TXN_LAYER
#include "siptxnstruct.h"
#endif
#include "portlayer.h"

#define STATS_DISPLAY_TIME	5 /* secs */
#define MAX_MSG_SIZE 10000

#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions */
typedef enum
{
	SipReady,
	SipInviteSent,
	SipInProgress,
	SipByeSent,
	SipTerminated
} SipCallState;

typedef struct
{
	SipCallState state;
	synch_id_t callLock;
	SIP_U32bit seqNum;
	SIP_S8bit callId[25];
#ifdef SIP_TXN_LAYER
	SIP_Pvoid pInvTxnKey;
	SIP_Pvoid pByeTxnKey;
	SIP_Pvoid pSInvTxnKey;
	SIP_Pvoid pSByeTxnKey;
#endif
} SipCall;

typedef struct
{
	SipTranspAddr client_dest_addr;
	SIP_S32bit nCalls;
	SIP_S32bit calls_per_burst; /* no of calls to be sent in a burst */
	SIP_S32bit delay_bet_bursts;
	SIP_S32bit nMaxCalls;
	SIP_S32bit calls_hold_time;
	SIP_S32bit call_timeout;
	synch_id_t client_start_mutex;
} SipClientParams;

typedef struct
{
	SIP_S32bit calls_attempted;
	SIP_S32bit successful_setups;
	SIP_S32bit teardowns_attempted;
	SIP_S32bit timed_out_calls;
	SIP_S32bit successful_teardowns;
	SIP_S32bit message_retransmissions;

	struct timeval end_time;
} SipTestStatistics;

typedef struct
{
	struct sockaddr_in  cli_addr;
	SIP_S8bit *message;
} SipEvent;

typedef struct
{
	SIP_S8bit contextStr[128];
	SIP_S8bit callId[32];
} SipCallContext;


typedef void (*fnSigType)(int);
typedef SIP_Pvoid (*fnPthreadType)(SIP_Pvoid);
/******************************************************************************
 ** FUNCTION: 		hss_getChar
 **
 ** DESCRIPTION: 	Function used to read the character.
 **
 ** PARAMETERS:		None
 **
 ******************************************************************************/
SIP_S32bit hss_getChar(void);
/******************************************************************************
 ** FUNCTION: 		sip_sleep
 **
 ** DESCRIPTION: 	Function sleeps for the specified duration
 **
 ** PARAMETERS:
 ** dNanosecs(IN):	Specifies the duration of sleep in microseconds
 ******************************************************************************/
void sip_sleep(long dNanosecs);
/******************************************************************************
 ** FUNCTION: 		SetupCalls
 **
 ** DESCRIPTION: 	This is function where the initial calls are Setup.
 **
 ** PARAMETERS:
 ** pClientParams(IN):	Specifies the Client Paramerters
 ******************************************************************************/
void SetupCalls(SipClientParams *pClientParams);

#ifndef SIP_TXN_LAYER
/******************************************************************************
 ** FUNCTION: 		timerThread
 **
 ** DESCRIPTION: 	This is function which keeps removing the timedOut
 **					elements from the timer Wheel
 ** PARAMETERS:
 ** pData(IN):		Currently this parameter is not being used
 ******************************************************************************/
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
SIP_Pvoid timerThread(SIP_Pvoid pData);
#endif

/******************************************************************************
 ** FUNCTION: 		client_message_handle
 **
 ** DESCRIPTION: 	This function handles all the messages received from the
 **					Network in the client mode.
 ** PARAMETERS:
 ** pMessage(IN)   :Message to be processed
 ** dThreadIndex(IN):Index of the thread that Invoked this function
 ******************************************************************************/
void client_message_handle(SipMessage *pMessage, SIP_U8bit dThreadIndex);

/******************************************************************************
 ** FUNCTION: 		server_message_handle
 **
 ** DESCRIPTION: 	This function handles all the messages received from the
 **					Network in the server mode.
 ** PARAMETERS:
 ** pMessage(IN)   :Message to be processed
 ** dThreadIndex(IN):Index of the thread that Invoked this function
 ** pClient_addr(IN):Client Address from where the message was received.
 ******************************************************************************/
void server_message_handle(SipMessage *pMessage, SipTranspAddr *pClient_addr,
	SIP_U8bit dThreadIndex);

/******************************************************************************
 ** FUNCTION: 		sip_sendRequest
 **
 ** DESCRIPTION: 	Function used to send requests.
 **
 ** PARAMETERS:
 ** pMethod(IN):		Request method to be sent
 ** pSendaddr(IN):	Address to which the Request is to be sent
 ** pCallID(IN):	CallId to be set in the message
 ** pErr(OUT):		Error variable for setting the error value in case
 **					an Error occurs
 ******************************************************************************/
SipBool sip_sendRequest(const SIP_S8bit *pMethod,SipTranspAddr *pSendaddr, \
	SIP_S8bit *pCallID, SipError *pErr);
/******************************************************************************
 ** FUNCTION: 		sendResponse
 **
 ** DESCRIPTION: 	Function used to send responses
 **
 ** PARAMETERS:
 ** pMessage(IN):	Message to be processed
 ** dCode(IN):		Response Code to be sent
 ** pReason(IN):		Reason field to be set in the response line
 ** pSendaddr(IN):	Address to which to send the Response
 ** pErr(OUT):		Error variable for setting the error value in case
 **					an Error occurs
 ******************************************************************************/
SipBool sendResponse(SipMessage *pMessage, SIP_S32bit dCode,const SIP_S8bit *pReason, \
	SipTranspAddr *pSendaddr,SipError *pErr);
#else

/******************************************************************************
 ** FUNCTION: 		client_message_handle
 **
 ** DESCRIPTION: 	This function handles all the messages received from the
 **					Network in the client mode.
 ** PARAMETERS:
 ** pMessage(IN)   :Message to be processed
 ** pTxnKey(IN):	Transaction Key of the Message
 ** dThreadIndex(IN):Index of the thread that Invoked this function
 ******************************************************************************/
void client_message_handle(SipMessage *pMessage,SipTxnKey* pTxnKey,\
	SIP_U8bit dThreadIndex);
/******************************************************************************
 ** FUNCTION: 		server_message_handle
 **
 ** DESCRIPTION: 	This function handles all the messages received from the
 **					Network in the server mode.
 ** PARAMETERS:
 ** pMessage(IN)   :Message to be processed
 ** pTxnKey(IN):	Transaction Key of the Message
 ** dThreadIndex(IN):Index of the thread that Invoked this function
 ** pClient_addr(IN):Client Address from where the message was received.
 ******************************************************************************/
void server_message_handle(SipMessage *pMessage, SipTranspAddr *pClient_addr,
	SipTxnKey* pTxnKey,SIP_U8bit dThreadIndex);
/******************************************************************************
 ** FUNCTION: 		sip_sendRequest
 **
 ** DESCRIPTION: 	Function used to send requests.
 **
 ** PARAMETERS:
 ** pMethod(IN):		Request method to be sent
 ** pSendaddr(IN):	Address to which the Request is to be sent
 ** pCallID(IN):	CallId to be set in the message
 ** ppTxnKey(OUT):	Transaction Key variable to be set
 ** pErr(OUT):		Error variable for setting the error value in case
 **					an Error occurs
 ******************************************************************************/
SipBool sip_sendRequest(const SIP_S8bit *pMethod,SipTranspAddr *pSendaddr, \
	SIP_S8bit *pCallID,SipTxnKey** ppTxnKey, SipError *pErr);
/******************************************************************************
 ** FUNCTION: 		sendResponse
 **
 ** DESCRIPTION: 	Function used to send responses
 **
 ** PARAMETERS:
 ** pMessage(IN):	Message to be processed
 ** dCode(IN):		Response Code to be sent
 ** pReason(IN):		Reason field to be set in the response line
 ** pSendaddr(IN):	Address to which to send the Response
 ** ppTxnKey(OUT):	Transaction Key variable to be set
 ** pErr(OUT):		Error variable for setting the error value in case
 **					an Error occurs
 ******************************************************************************/
SipBool sendResponse(SipMessage *pMessage, SIP_S32bit dCode,const SIP_S8bit *pRreason, \
	SipTranspAddr *pSendaddr,SipTxnKey** ppTxnKey,SipError *pErr);

#endif

/******************************************************************************
 ** FUNCTION: 		sip_getPortFromSentBy
 **
 ** DESCRIPTION: 	Function used to get the port number from the
 **					host string
 **
 ** PARAMETERS:
 ** pHost(IN):		Host String from which to extract the Port Number
 **
 ******************************************************************************/
SIP_S32bit sip_getPortFromSentBy(SIP_S8bit *pHost);
/******************************************************************************
 ** FUNCTION: 		sip_copyMandatoryHeaders
 **
 ** DESCRIPTION: 	Function to copy the From, To, Call-Id and CSeq headers
 **					from one message into the other.
 ** PARAMETERS:
 ** pOriginal(IN):	Original Message from which to copy the headers
 ** pResponse(IN):	Destination message to which to copy the headers
 **
 ******************************************************************************/
void sip_copyMandatoryHeaders(SipMessage *pOriginal, SipMessage *pResponse);
/******************************************************************************
 ** FUNCTION: 		sip_copyViaHeaders
 **
 ** DESCRIPTION: 	Function to copy the via header from the original message
 **					into the response.
 ** PARAMETERS:
 ** pOriginal(IN):	Original Message from which to copy the Via header
 ** pResponse(IN):	Destination message to which to copy the Via header
 **
 ******************************************************************************/
void sip_copyViaHeaders(SipMessage *pOriginal, SipMessage *pResponse);
/******************************************************************************
 ** FUNCTION: 		init_statistics
 **
 ** DESCRIPTION: 	Function used to initialize the statistics.
 **
 ** PARAMETERS:		None
 **
 ******************************************************************************/
void init_statistics(void);
/******************************************************************************
 ** FUNCTION: 		display_statistics
 **
 ** DESCRIPTION: 	Function used to display the statistics.
 **
 ** PARAMETERS:
 ** dInit_flag(IN):	Specifies whether to display the statistics or to reset the
 **					the start time.
 ******************************************************************************/
void display_statistics(SIP_S32bit dInit_flag);


/*****************************************************************************
 ** FUNCTION: 	 sip_getThreadIdFromCallId
 **
 ** DESCRIPTION: Function to return the thread id for the thread handling
 **				 a particular call id.
 **
 ** PARAMETERS:
 **
 **  dCallId(IN): The callId that needs to be mapped to a thread
 **	 pThreadId(OUT) : The threadId to which the callId has been mapped
 **
 *****************************************************************************/
SipBool sip_getThreadIdFromCallId(SIP_U32bit dCallId, SIP_S8bit *pThreadId);


/******************************************************************************
 ** FUNCTION: 	 sip_getCallIdFromBuffer
 **
 ** DESCRIPTION: Function to do a first level parse and get the call-id
 **				for the call
 **
 **	PARAMETERS:
 **
 **    pString(IN): The message received from which we have to determine
 **                 the CallId.
 **    pCallId(OUT):The CallId retrieved from th message received
 *****************************************************************************/
SipBool sip_getCallIdFromBuffer(SIP_S8bit *pString, SIP_U32bit *pCallId);


/* Extern definitions */
extern SIP_S32bit glbClient_mode;
extern SipClientParams dClient_params;
extern SipCall *glbpCallObjectArray;
extern SipTestStatistics *glbpTest_statistics;

extern SIP_U32bit glbThreadCount;

#if defined(SIP_WINDOWS) || defined(SIP_OSE)
void bzero(SIP_Pvoid s, SIP_U32bit n);
#endif


#ifdef __cplusplus
}
#endif

#endif
