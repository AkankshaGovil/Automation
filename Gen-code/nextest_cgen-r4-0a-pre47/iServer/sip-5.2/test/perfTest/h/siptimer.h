/*****************************************************************************
 ** FUNCTION:
 **			The HSS SIP Stack performance testing application.
 **
 *****************************************************************************
 **
 ** FILENAME:		siptimer.h
 **
 ** DESCRIPTION:	This file contains the structure definitions and the
 **					prototypes for the functions implemented in siptimer.c
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			---------		------
 ** 10-Nov-01		Ramu K							Creation
 **
 *****************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 *****************************************************************************/

#ifndef __TIMERWHEEL_H__
#define __TIMERWHEEL_H__

/* Ensure Names are not Mangled by C++ Compilers */
#ifdef __cplusplus
extern "C" {
#endif

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"

#define NUM_ELEMENTS 10000
#define NUM_BUCKETS 1000
#define GRANULARITY 100
#define MAX_TIMEOUT 32000
struct TimerWheelElement;

/*The definition of the Timer Hash element*/
typedef struct TimerHashElement
{
	SIP_S32bit duration;
	SIP_S8bit restart;

	sip_timeoutFuncPtr timeoutfunc;

	SIP_Pvoid buffer;
	SipTimerKey *key;
	struct TimerWheelElement *wheelElementPointer;
	SIP_U32bit wheelListIndex;
	struct TimerHashElement *right;
	struct TimerHashElement *left;
} TimerHashElement;


/*Definition for an individual Timer Wheel element*/
typedef struct TimerWheelElement
{
	TimerHashElement* hashElem;
	struct TimerWheelElement* right;
	struct TimerWheelElement* left;
	SIP_U32bit HashIndex;
}TTimerWheelElement;

/* Timer wheel is a circular array of double linked lists made up of
** TTimerWheelElement nodes. TTimerWheel.wheel[index] always points to the
** tail of the corresponding list
*/
typedef struct TTimerWheel_struct
{
	TTimerWheelElement* wheel;
	SIP_U32bit granularity;  /* number of msecs per tick */
	SIP_U32bit currentIndex; /* index to the current entry in the wheel */
	SIP_U32bit wheelSize;	   /* Total number of entries in the wheel */

	TimerHashElement *hashtbl; /* Used to map to wheel entry using the
                                   ** timer Key */
	SIP_U32bit hashtblSize;  /* Total number buckets */

	synch_id_t TimerWheelMutex[(MAX_TIMEOUT/GRANULARITY)+1];
	synch_id_t HashMutex[NUM_BUCKETS+1];

} TTimerWheel;

/*The global Timer Wheel*/
extern TTimerWheel glbTimerWheel;

/******************************************************************************
 ** FUNCTION: 		sip_timerWheelInitialize
 **
 ** DESCRIPTION: 	This function initializes the timerWheel and the HashTable
 ** PARAMETERS:
 ** pTimeWheel(OUT):Timer Wheel To be initialized
 ** dGranuality(IN):Specifies the granuality for timer (in mSec)
 ** dMaxTimeout(IN):Specifies the Max timeOut (in mSec)
 ** dHashtblSixe(IN):Specifies the Hash Table Size
 **
 ******************************************************************************/
SipBool sip_timerWheelInitialize
(TTimerWheel *pTimerWheel,SIP_U32bit dGranuality, SIP_U32bit dMaxTimeout,\
	SIP_U32bit dHashtblSize);


/******************************************************************************
 ** FUNCTION: 		sip_timerWheelAppend
 **
 ** DESCRIPTION: 	This function adds an element into the timerWheel
 ** PARAMETERS:
 ** pTimerWheel(IN):Timer Wheel in which to append the Element
 ** pHashElem(IN):	Element to be appended
 ******************************************************************************/
SipBool sip_timerWheelAppend( TTimerWheel *pTimerWheel,\
	TimerHashElement* pHashElem );


/******************************************************************************
 ** FUNCTION: 		sip_timerWheelRemove
 **
 ** DESCRIPTION: 	This function removes an element from the timerWheel
 ** PARAMETERS:
 ** pTimerWheel(IN):Timer Wheel in which to remove the Element
 ** pWheelElem(IN):	Element to be removed
 ******************************************************************************/
SipBool sip_timerWheelRemove( TTimerWheel *pTimerWheel,\
	TTimerWheelElement* pWheelElem);


/******************************************************************************
 ** FUNCTION:		sip_timerWheelHashAdd
 **
 ** DESCRIPTION:	This is the function to add an entry
 **             	into the hash table.
 ** PARAMETERS:
 ** pTimerWheel(IN):Timer Wheel in which to append the Hash Element
 ** pElemHash(IN):	Element to be appended
 **
 ******************************************************************************/
SipBool sip_timerWheelHashAdd (TTimerWheel *pTimerWheel,\
	TimerHashElement *pElemHash);


/******************************************************************************
 ** FUNCTION:		sip_timerWheelHashRemove
 **
 ** DESCRIPTION:	This is the function to remove an entry
 **                 from the hash table.
 ** PARAMETERS:
 ** pTimerWheel(IN):Timer Wheel in which to append the Hash Element
 ** pElemHash(IN)  :Element to be appended
 **
 ******************************************************************************/
SipBool sip_timerWheelHashRemove (TTimerWheel *pTimerWheel,\
	TimerHashElement *pElemHash);


/******************************************************************************
 ** FUNCTION:		sip_timerWheelHashFetch
 **
 ** DESCRIPTION:	This is the function to Fetch an entry
 **             	from the hash table.
 ** pTimerWheel(IN):Timer Wheel from which to fetch the HashElement
 ** pKey(IN):		Key to be used for fetching the Hash Element
 ** pIndex(OUT):	Will be stored with the index value of the HashElement
 **					in the Hash Table
 ******************************************************************************/
TimerHashElement* sip_timerWheelHashFetch (TTimerWheel *pTimerWheel,\
	SipTimerKey *pKey,SIP_U32bit* pIndex);


/*****************************************************************
** Function: fast_startTimer
** Description: Timer management callback to be implemented by the
**              application.
** Parameters:
**              dDuration(IN): The duration in milliseconds after which the
**                      application must call timoutfunc with buffer and key
**                      as parameters.
**              dRestart(IN): Currently not used. 0 to be passed.
**              timeoutfunc(IN): A function supplied by the stack. The
**                      application must invoke this function when the timer
**                      expires. The application must pass the buffer and key
**                      passed to this function.
**              pBuffer(IN): The first parameter to be supplied to the timeout
**                      function supplied by the stack.
**              pKey(IN): The second paramter to be supplied to the timeout
**                      function provided by the stack.
**              pErr(OUT): Error code may be returned in this if the call fails.
**
*************************************************************************/
SipBool fast_startTimer (SIP_U32bit dDuration, \
        SIP_S8bit dRestart, sip_timeoutFuncPtr timoutfunc, \
                SIP_Pvoid pBuffer, SipTimerKey *pKey, SipError *pErr);


/*************************************************************************
** Function: fast_stoptTimer
** Description: Timer management callback to be implemented by the
**              application.
** Parameters:
**              pInkey(IN): The key to be used to search the list of timers
**              maintained by the application. The application must use
**              the sip_compareTimerKeys function to search for a matching
**              entry. Please refer to the note in the sip_compareTimerKey
**                      description.
**              ppOutkey(OUT): Any matching key must be returned in outkey. The
**                      stack releases memory used by outkey.
**              ppBuffer(OUT): The buffer associated with the matching entry is
**                      to be retured in this to the stack for releasing memory.
**              pErr(OUT): The error code is set into this in case of failure.
**
**************************************************************************/
SipBool fast_stopTimer (SipTimerKey *pInkey, \
        SipTimerKey **ppOutkey, SIP_Pvoid *ppBuffer,  SipError *pErr);


/*************************************************************************
 ** FUNCTION: 		timerThread
 **
 ** DESCRIPTION: 	Function which removes the timerOut Elements from the
 **					timer Wheel
 *************************************************************************/
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
SIP_Pvoid timerThread(SIP_Pvoid pData);
#endif


/*************************************************************************
 ** FUNCTION: 		flush/flushElems
 **
 ** DESCRIPTION: 	Function removes all the elements from the Timer Wheel
 **
 *************************************************************************/
void flushTimerWheel(void);

#ifdef __cplusplus
}
#endif

#endif
