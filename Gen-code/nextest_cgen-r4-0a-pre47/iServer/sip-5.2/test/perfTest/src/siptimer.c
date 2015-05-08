/*****************************************************************************
 ** FUNCTION:
 **			The HSS SIP Stack performance testing application.
 **
 *****************************************************************************
 **
 ** FILENAME:		siptimer.c
 **
 ** DESCRIPTION:	This file contains the implementation for the timer that
 **					is used by the stack to control the retransmissions.
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			---------		------
 ** 10-Nov-01		Ramu K							Creation
 **
 *****************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 *****************************************************************************/
/* Ensure Names are not Mangled by C++ Compilers */
#ifdef __cplusplus
extern "C" {
#endif
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
#include <pthread.h>
#endif



#include "sipcommon.h"
#include "sipstruct.h"
#include "sipdecode.h"
#include "sipsendmessage.h"
#include "sipfree.h"
#include "siptimer.h"
#include "sipclient.h"
#include "siphash.h"

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
#define NULL 0
#endif

TTimerWheel glbTimerWheel;
synch_id_t timer_list_lock;
extern synch_id_t timer_start_lock;
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)

extern thread_cond_t timer_start_cond_var;
#endif

extern SipTestStatistics *glbpTest_statistics;

/******************************************************************************
 ** FUNCTION: 		sip_timerWheelInitialize
 **
 ** DESCRIPTION: 	This function initializes the timerWheel and the HashTable
 **
 ******************************************************************************/
SipBool
sip_timerWheelInitialize(
	TTimerWheel *timerWheel,
	SIP_U32bit gran,
	SIP_U32bit maxTimeOut,
	SIP_U32bit hashtblSize)
{
	SIP_U32bit i;

	timerWheel->wheelSize = ( maxTimeOut / gran) + ((maxTimeOut%gran)?1:0);
	timerWheel->granularity = gran;
	timerWheel->currentIndex = 0;

	timerWheel->wheel = ( TTimerWheelElement* ) fast_memget (0, \
		sizeof(TTimerWheelElement) * timerWheel->wheelSize,NULL);
	if (timerWheel->wheel == NULL)
		return SipFail;

	/* intialize all wheel chains */
	for (i=0; i < (timerWheel->wheelSize); i++)
	{
		timerWheel->wheel[i].right = &timerWheel->wheel[i];
		timerWheel->wheel[i].left = &timerWheel->wheel[i];
	}

	for (i=0; i < (timerWheel->wheelSize); i++)
	{
		fast_init_synch(&timerWheel->TimerWheelMutex[i]);
	}


	/* Initialize the Hash Table */
	timerWheel->hashtblSize = hashtblSize;
	timerWheel->hashtbl = (TimerHashElement *) fast_memget(0, \
		sizeof(TimerHashElement) * hashtblSize,NULL);
	if (timerWheel->hashtbl == NULL)
	{
		fast_memfree (0,timerWheel->wheel,SIP_NULL);
		return SipFail;
	}

	/* initialize all the hash table chains */
	for (i=0; i<hashtblSize; ++i)
	{
		timerWheel->hashtbl[i].right = &timerWheel->hashtbl[i];
		timerWheel->hashtbl[i].left = &timerWheel->hashtbl[i];
	}

	for (i=0; i<hashtblSize;i++)
	{
		fast_init_synch(&timerWheel->HashMutex[i]);
	}

	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: 		sip_timerWheelAppend
 **
 ** DESCRIPTION: 	This function adds an element into the timerWheel
 **
 ******************************************************************************/
SipBool
sip_timerWheelAppend(
	TTimerWheel *timerWheel,
	TimerHashElement* hashElem )
{
	SIP_S32bit index=0;
	SIP_S32bit offset;
	TTimerWheelElement *temp, *lastElem;

	offset = (hashElem->duration / timerWheel->granularity)
		+ ((hashElem->duration % timerWheel->granularity)?1:0);
	index = ( timerWheel->currentIndex + offset) % timerWheel->wheelSize;

	temp = (TTimerWheelElement*) fast_memget(0,sizeof (TTimerWheelElement),\
		NULL );
	temp->hashElem = hashElem;
	if (temp == NULL)
		return SipFail;

	hashElem->wheelElementPointer = temp;
	hashElem->wheelListIndex = index;

	fast_lock_synch(0,&timerWheel->TimerWheelMutex[index],0);

	lastElem = timerWheel->wheel[index].left;
	temp->right = lastElem->right;
	temp->left = lastElem;
	lastElem->right->left = temp;
	lastElem->right = temp;

	fast_unlock_synch(0,&timerWheel->TimerWheelMutex[index]);
	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: 		sip_timerWheelAppend
 **
 ** DESCRIPTION: 	This function removes an element from the timerWheel
 **
 ******************************************************************************/
SipBool sip_timerWheelRemove(
	TTimerWheel *timerWheel,
	TTimerWheelElement* wheelElem )
{
	TTimerWheel* pTempWheel;
	pTempWheel = timerWheel;
	wheelElem->left->right = wheelElem->right;
	wheelElem->right->left = wheelElem->left;

	fast_memfree(0,wheelElem,SIP_NULL);
	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION:           sip_timerWheelHashAdd
 **
 ** DESCRIPTION:        This is the function to add an entry
 **                                     into the hash table.
 **
 ******************************************************************************/
SipBool sip_timerWheelHashAdd
        (TTimerWheel *timerWheel, TimerHashElement *elemHash)
{
	SIP_U32bit hashKey;
	SIP_U32bit bucketIndex;
	TimerHashElement *lastElem;

	hashKey = sip_elfHash(elemHash->key->dCallid);
	bucketIndex = hashKey % timerWheel->hashtblSize;
	fast_lock_synch(0,&timerWheel->HashMutex[bucketIndex],0);
	fast_lock_synch(0,&timerWheel->TimerWheelMutex[elemHash->wheelListIndex],0);

	elemHash->wheelElementPointer->HashIndex = bucketIndex;

	lastElem = timerWheel->hashtbl[bucketIndex].left;
	elemHash->right = lastElem->right;
	elemHash->left = lastElem;
	lastElem->right->left = elemHash;
	lastElem->right = elemHash;

	fast_unlock_synch(0,&timerWheel->TimerWheelMutex[elemHash->wheelListIndex]);
	fast_unlock_synch(0,&timerWheel->HashMutex[bucketIndex]);
	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION:           sip_timerWheelHashRemove
 **
 ** DESCRIPTION:        This is the function to remove an entry
 **                                     from the hash table.
 **
 ******************************************************************************/
SipBool sip_timerWheelHashRemove
        (TTimerWheel *timerWheel, TimerHashElement *elemHash)
{
	TTimerWheel* pTempWheel;
	pTempWheel = timerWheel;
	elemHash->left->right = elemHash->right;
	elemHash->right->left = elemHash->left;

	fast_memfree(0,elemHash,SIP_NULL);
	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION:           sip_timerWheelHashFetch
 **
 ** DESCRIPTION:        This is the function to Fetch an entry
 **                                     from the hash table.
 **
 ******************************************************************************/
TimerHashElement* sip_timerWheelHashFetch
        (TTimerWheel *timerWheel, SipTimerKey *key,SIP_U32bit* HIndex)
{
	SIP_U32bit hashKey;
	SIP_U32bit bucketIndex;
	TimerHashElement *pHead, *pCur;

	hashKey = sip_elfHash(key->dCallid);
	bucketIndex = hashKey % timerWheel->hashtblSize;
	*HIndex = bucketIndex;

	fast_lock_synch(0,&timerWheel->HashMutex[bucketIndex],0);
	pHead = &timerWheel->hashtbl[bucketIndex];

	pCur = pHead->right;
	while (pCur != pHead)
	{
		if (sip_stringKeyCompareFunction(pCur->key->dCallid, \
			key->dCallid) == 0)
		{
			/* Element found */
			return pCur;
		}

		pCur = pCur->right;
	}

	/* Element not found */
	fast_unlock_synch(0,&timerWheel->HashMutex[bucketIndex]);
	return NULL;
}


/******************************************************************************
 ** FUNCTION: 		fast_startTimer
 **
 ** DESCRIPTION: 	This is a Callback function used by the stack to
 **					Start the timer
 ******************************************************************************/
SipBool fast_startTimer
(SIP_U32bit duration, SIP_S8bit restart,sip_timeoutFuncPtr timeoutfunc,\
	SIP_Pvoid buffer, SipTimerKey *key, SipError *err)
{
	SIP_U32bit hashKey;
  	SIP_U32bit bucketIndex;
  	TimerHashElement *lastElem;
	TimerHashElement *elemHash;
  	SIP_S32bit index=0;
	SIP_S32bit offset;
	TTimerWheelElement *temp, *TlastElem;

	elemHash = (TimerHashElement *) fast_memget(0,sizeof(TimerHashElement),\
		NULL);
	if (elemHash == NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
	elemHash->duration = (SIP_S32bit)duration;
	elemHash->restart = restart;
	elemHash->timeoutfunc = timeoutfunc;
	elemHash->buffer = buffer;
	elemHash->key = key;

	hashKey = sip_elfHash(key->dCallid);
	bucketIndex = hashKey % glbTimerWheel.hashtblSize;

	offset = (duration / glbTimerWheel.granularity)
		+ ((duration % glbTimerWheel.granularity)?1:0);
	index = \
		( glbTimerWheel.currentIndex + offset) % glbTimerWheel.wheelSize;

	temp = (TTimerWheelElement*) fast_memget(0,sizeof (TTimerWheelElement),\
		NULL);
	if (temp == NULL)
	{
		fast_memfree(0,elemHash,SIP_NULL);
		return SipFail;
	}

	temp->hashElem = elemHash;
	temp->HashIndex = bucketIndex;

	elemHash->wheelElementPointer = temp;
	elemHash->wheelListIndex = index;

	fast_lock_synch(0,&glbTimerWheel.HashMutex[bucketIndex],0);
	fast_lock_synch(0,&glbTimerWheel.TimerWheelMutex[index],0);

 	lastElem = glbTimerWheel.hashtbl[bucketIndex].left;
	elemHash->right = lastElem->right;
	elemHash->left = lastElem;
	lastElem->right->left = elemHash;
	lastElem->right = elemHash;

	TlastElem = glbTimerWheel.wheel[index].left;
	temp->right = TlastElem->right;
	temp->left = TlastElem;
	TlastElem->right->left = temp;
	TlastElem->right = temp;

	fast_unlock_synch(0,&glbTimerWheel.TimerWheelMutex[index]);
	fast_unlock_synch(0,&glbTimerWheel.HashMutex[bucketIndex]);

	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: 		fast_stopTimer
 **
 ** DESCRIPTION: 	This is a Callback function used by the stack to
 **					Stop the timer
 ******************************************************************************/
SipBool fast_stopTimer
(SipTimerKey *inkey, SipTimerKey **outkey, SIP_Pvoid *buffer,  SipError *err)

{
	TimerHashElement *elemHash;
	SipError dummy;
	SIP_U32bit HashIndex;

	dummy = *err;

	elemHash = sip_timerWheelHashFetch (&glbTimerWheel, inkey,&HashIndex);

	if(elemHash != SIP_NULL)
	{
		SIP_U32bit inWheelIndex;

		fast_lock_synch(0, \
   			&glbTimerWheel.TimerWheelMutex[elemHash->wheelListIndex],0);

		inWheelIndex = elemHash->wheelListIndex;
		/* remove from the timer wheel */
		sip_timerWheelRemove(&glbTimerWheel,\
			elemHash->wheelElementPointer);
		/*remove from the hash */
		*outkey = elemHash->key;
		*buffer = elemHash->buffer;
		sip_timerWheelHashRemove(&glbTimerWheel, elemHash);

        fast_unlock_synch(0, \
        	&glbTimerWheel.TimerWheelMutex[inWheelIndex]);
        fast_unlock_synch(0,&glbTimerWheel.HashMutex[HashIndex]);
		return SipSuccess;
	}

	return SipFail;
}

/******************************************************************************
 ** FUNCTION: 		timerThread
 **
 ** DESCRIPTION: 	Function which removes the timerOut Elements from the
 **					timer Wheel
 ******************************************************************************/
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
SIP_Pvoid timerThread(SIP_Pvoid pData)
#endif
{
	struct timeval tv1, tv2;
	SIP_Pvoid pTemp;
	SIP_U32bit numBuckets = NUM_BUCKETS;
	SIP_U32bit granularity = GRANULARITY;
	SIP_U32bit maxTimeout = MAX_TIMEOUT;
	SIP_U32bit statsDisplayCounter;
	pTemp = pData;

	fast_lock_synch(0,&timer_start_lock,0);

	/* Initialize the timer list */
	/*Initing the Timer wheel*/
	fast_init_synch(&timer_list_lock);

	/*The Timer Wheel is inited here.*/
	sip_timerWheelInitialize(&glbTimerWheel,granularity,\
	maxTimeout,numBuckets);
	pTemp = NULL;

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	pthread_cond_signal(&timer_start_cond_var);
#endif

	fast_unlock_synch(0,&timer_start_lock);
	statsDisplayCounter = (STATS_DISPLAY_TIME * 1000)/granularity;

/* Now start timer loop */
	while(1)
	{
		SIP_U32bit elapsed_time;
		TTimerWheelElement *pTimerWheelElem=NULL, *pHead;
		TimerHashElement *hashElem;
		gettimeofday(&tv1, NULL);
		/* if any timers are due on current time */
		pHead = &glbTimerWheel.wheel[glbTimerWheel.currentIndex];
		while (1)
		{
			sip_timeoutFuncPtr tempfunc;
			SipTimerKey *tempkey;
			SIP_Pvoid tempbuffer;

			fast_lock_synch(0,\
				&glbTimerWheel.TimerWheelMutex[glbTimerWheel.currentIndex],0);
			pTimerWheelElem = pHead->right;

			if (pTimerWheelElem == pHead)
			{
				/* End of wheel chain */
				fast_unlock_synch(0,\
					&glbTimerWheel.TimerWheelMutex[glbTimerWheel.currentIndex]);
				break;
			}

			if ( fast_trylock_synch(0,\
				&glbTimerWheel.HashMutex[pTimerWheelElem->HashIndex],0) == SipSuccess )
			{
				SIP_U32bit inHashIndex;
				inHashIndex = pTimerWheelElem->HashIndex;
				hashElem =  pTimerWheelElem->hashElem;
				tempfunc = hashElem->timeoutfunc;
				tempkey = hashElem->key;
				tempbuffer = hashElem->buffer;
				sip_timerWheelRemove(&glbTimerWheel, pTimerWheelElem);

				/* Remove it from the hash*/
				sip_timerWheelHashRemove(&glbTimerWheel, hashElem);
				glbpTest_statistics[0].message_retransmissions++;

				fast_unlock_synch(0,&glbTimerWheel.HashMutex[inHashIndex]);
				fast_unlock_synch(0,\
					&glbTimerWheel.TimerWheelMutex[glbTimerWheel.currentIndex]);
				tempfunc(tempkey, tempbuffer);
			}
			else
			{
				fast_unlock_synch(0,\
					&glbTimerWheel.TimerWheelMutex[glbTimerWheel.currentIndex]);
			}
		}/* end of inner while loop */
		gettimeofday(&tv2, NULL);
		/* Get time elapsed */
		elapsed_time = (((tv2.tv_sec )*1000+(tv2.tv_usec/1000))\
			-((tv1.tv_sec )*1000+(tv1.tv_usec/1000)));
		if(elapsed_time < granularity)
			sip_sleep((granularity-elapsed_time)*1000);
		/* increment the currenttime pointer*/
		glbTimerWheel.currentIndex = (glbTimerWheel.currentIndex+1)\
			% glbTimerWheel.wheelSize;
		statsDisplayCounter--;
		if (statsDisplayCounter == 0)
		{
			display_statistics(0);
			statsDisplayCounter = (STATS_DISPLAY_TIME * 1000)/granularity;
		}
	}/* end of outer while loop*/
	return NULL;

}


/******************************************************************************
 ** FUNCTION: 		flush
 **
 ** DESCRIPTION: 	Function removes all the elements from the Timer Wheel
 **
 ******************************************************************************/
void flushTimerWheel(void)
{
	TimerHashElement *elemHash, *pCur, *pHead;
	SipTimerKey *tempkey;
	SIP_Pvoid tempbuffer;
	SIP_U32bit i;

	/* Flush the Timer List*/
	fast_lock_synch(0,&timer_list_lock,0);

	/*Flush the Hash*/
	for (i=0; i<glbTimerWheel.hashtblSize; ++i)
	{
		pHead = &glbTimerWheel.hashtbl[i];
		pCur = pHead->right;
		while (pCur != pHead)
		{
			elemHash = pCur;
			pCur = pCur->right;

			tempkey = elemHash->key;
			tempbuffer = elemHash->buffer;
			sip_timerWheelRemove(&glbTimerWheel, elemHash->wheelElementPointer);

			if(sip_timerWheelHashRemove(&glbTimerWheel, elemHash)
					!= SipFail)
			{
				sip_freeSipTimerKey(tempkey);
				sip_freeSipTimerBuffer((SipTimerBuffer *)tempbuffer);
			}
		}
	}
	fast_memfree(0,glbTimerWheel.hashtbl,SIP_NULL);
	fast_memfree(0,glbTimerWheel.wheel,SIP_NULL);
	fast_unlock_synch(0,&timer_list_lock);
}

#ifdef __cplusplus
}
#endif
