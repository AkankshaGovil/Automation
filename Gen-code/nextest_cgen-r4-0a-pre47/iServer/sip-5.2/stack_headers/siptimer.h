
/**************************************************************************
** FUNCTION:
**  This file contains the prototypes of the Timer APIs
**
***************************************************************************
**
** FILENAME:
**  siptimer.h
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


#ifndef _SIPTIMER_H_
#define _SIPTIMER_H_

#include "sipcommon.h"
#include "sipstruct.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/*****************************************************************
** Function: fast_startTimer
** Description: Timer management callback to be implemented by the
**		application. 
** Parameters:
**		duration(IN): The duration in milliseconds after which the 
**			application must call timoutfunc with buffer and key
**			as parameters.
**		restart(IN): Currently not used. 0 to be passed.
**		timeoutfunc(IN): A function supplied by the stack. The
**			application must invoke this function when the timer
**			expires. The application must pass the buffer and key
**			passed to this function.
**		buffer(IN): The first parameter to be supplied to the timeout
**			function supplied by the stack.
**		key(IN): The second paramter to be supplied to the timeout
**			function provided by the stack.
**		err(OUT): Error code may be returned in this if the call fails.
**
*******************************************************************/
extern SipBool fast_startTimer _ARGS_((SIP_U32bit duration, \
	SIP_S8bit restart, sip_timeoutFuncPtr timoutfunc, \
		SIP_Pvoid buffer, SipTimerKey *key, SipError *err));

/*****************************************************************
** Function: fast_stoptTimer
** Description: Timer management callback to be implemented by the
**		application.
** Parameters:
**		inkey(IN): The key to be used to search the list of timers
**			maintained by the application. The application must use
**			the sip_compareTimerKeys function to search for a matching
**			entry. Please refer to the note in the sip_compareTimerKey
**			description.
**		outkey(OUT): Any matching key must be returned in outkey. The
**			stack releases memory used by outkey.
**		buffer(OUT): The buffer associated with the matching entry is
**			to be retured in this to the stack for releasing memory.
**		err(OUT): The error code is set into this in case of failure.
**
*******************************************************************/
extern SipBool fast_stopTimer _ARGS_((SipTimerKey *inkey, \
	SipTimerKey **outkey, SIP_Pvoid *buffer,  SipError *err));

/*****************************************************************
** Function: sip_compareTimerKeys
** Description: Function used for comparing Timer keys generated by
**		the stack with the keys stored by the application.
**		Note: the first two parameters are not interchangeable.
**		Comparing every key in the applications timer list using this
**		function may be a costly operation. The user may eliminate keys
**		by comparing the pCallid fields of the timer keys before invoking 
**		this function. Other fields in the timer keys may differ for 
**		matching keys and should not be used for elimination/hashing.
** Parameters:
**		paramkey(IN): This should be set to the key supplied by the 
**			stack in fast_stopTimer.
**		storedkey(IN): This should be set to the key in the list that 
**			was supplied by fast_startTimer. 
**		err(OUT):
** Return Value:
**		SipSuccess: The timer keys matched.
**		SipFail: The timer keys do not match.
**
*******************************************************************/
extern SipBool sip_compareTimerKeys _ARGS_((SipTimerKey *paramkey, \
	SipTimerKey *storedkey, SipError *err));

/*****************************************************************
** Function: sip_freeEventContext
** Description: Callback to be implemted by the application. This
**		callback is invoked by the stack when it needs to free
**		the context structure since the stack is not aware of the 
**		contents set by the application. The application must free
**		the pData element and the SipEventContext structure.
** Parameters:
**		pContext(IN): The structure to be released by the application.
**
*******************************************************************/
extern void sip_freeEventContext(SipEventContext *pContext);

/*****************************************************************
** The following functions are internal functions.
*******************************************************************/
extern SipBool sip_stopTimer _ARGS_((SipMessage *msg,\
	SipEventContext *pContext,SipError *err));

extern SipBool sip_updateTimer _ARGS_((SipMessage *msg, \
	SipBool infoflag, SipEventContext *pContext, SipError *err));

extern SipBool sip_processTimeOut _ARGS_((SipTimerKey *key, \
	SIP_Pvoid buf));

extern SipBool sip_getCharString _ARGS_((SIP_S8bit *inputstr, \
	SIP_S8bit **outstr, SipError *err));

extern SipBool sip_isEqualPort _ARGS_((SIP_U16bit *port1,\
	SIP_U16bit *port2));

extern SipBool sip_isEqualSipUrl _ARGS_((SipUrl *url1, \
	SipUrl *url2, SipError *err));

extern SipBool sip_isEqualReqUri _ARGS_((SIP_S8bit *uri1,\
	SIP_S8bit *uri2, SipError *err));

extern SipBool sip_getBranchFromViaHdr _ARGS_((SipViaHeader *pViaHeader, \
	SIP_S8bit **ppBranch, SipError *pErr));

SipBool sip_compareToHeaders _ARGS_(( SipToHeader	*hdr1,\
	  SipToHeader	*hdr2, SipError	*err ));
	  
SipBool sip_compareFromHeaders _ARGS_ (( SipFromHeader	*hdr1,\
	  SipFromHeader	*hdr2, SipError	*err ));



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif