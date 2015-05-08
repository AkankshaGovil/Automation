#include "gis.h"
#include <malloc.h>

#include "sipreg.h"
#include "ua.h"
#include "callutils.h"

extern int *sipinPool;
extern int *sipinClass;
extern int nCallIdThreads;

/* UA Event processor code */
int
SipRegProcessEventWorker(SipEventHandle *evHandle)
{
	char fn[] = "SipRegProcessEventWorker():";
	SipRegSMEntry *smEntry = NULL;
	CallHandle *callHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	int state = SipReg_sIdle;

	appMsgHandle = SipEventAppHandle(evHandle);

	/* change state */
	SipRegChangeState(evHandle, appMsgHandle, &state);

	smEntry = &SipRegSM[state][evHandle->event];

	if (smEntry == NULL)
	{
		NETERROR(MSIP, ("%s No State Machine Entry found\n", fn));

		SipFreeEventHandle(evHandle);
		goto _release_locks;
	}

	/* Call the action routine */
	if (smEntry->action)
	{
		if ((smEntry->action)(evHandle) < 0)
		{
			/* Action routine failed */
			NETDEBUG(MSIP, NETLOG_DEBUG1, 
				("%s Action routine failed\n", fn));
			goto _error;
		}
	}
	else
	{
		// No action routine ?? Event must be freed
		SipFreeEventHandle(evHandle);
	}

	return 0;

_release_locks:

_error:
	NETDEBUG(MSIP, NETLOG_DEBUG1, ("%s Returning Error\n", fn));
	return -1;
}

int
SipRegProcessEvent(SipEventHandle *evHandle)
{
	char fn[] = "SipRegProcessEvent():";

	SipQueueEvent(evHandle, (void* (*) (void*))SipRegProcessEventWorker);

	return 0;
}

int
SipRegChangeState(SipEventHandle *evHandle, SipAppMsgHandle *appMsgHandle, int *prevState)
{
	char fn[] = "SipRegChangeState()";
	SipRegistration *sipreg;
	SipCallLegKey 	leg = { 0 };
	SipMsgHandle        *msgHandle = NULL;
	int rc = -1;

	if (!(leg.remote = appMsgHandle->calledpn))
	{
		msgHandle = appMsgHandle->msgHandle;
		if (msgHandle)
		{
			leg.remote = msgHandle->remote;
		}
	}

	CacheGetLocks(sipregCache,LOCK_READ,LOCK_BLOCK);

	if (leg.remote && (sipreg = CacheGet(sipregCache, &leg)))
	{
		*prevState = sipreg->state;
		rc = SipRegChangeExistingState(evHandle, sipreg);
	}
	else
	{
		sipreg = SipRegistrationNew(sipregCache->malloc);
		
		// Insert the new entry in the cache
		sipreg->sipch.callLeg.remote = UrlDup(appMsgHandle->calledpn, sipregCache->malloc);
		memcpy(sipreg->callID, appMsgHandle->callID, CALL_ID_LEN);
		*prevState = sipreg->state = SipReg_sIdle;
		rc = SipRegChangeExistingState(evHandle, sipreg);

		if (CacheInsert(sipregCache, sipreg) < 0)
		{
			NETERROR(MSIP, ("%s Error inserting new state machine instance into cache\n",
				fn));
			// free up everything
			SipRegistrationFree(sipreg, sipregCache->free);
		}

		rc = 0;
	}

	CacheReleaseLocks(sipregCache);

	return rc;
}

/* Locks must be acquired around this routine, 
 * as callHandle is unprotected otherwise
 */
int
SipRegChangeExistingState(SipEventHandle *evHandle, SipRegistration *sipreg)
{
	char fn[] = "SipRegChangeState():";
	SipRegSMEntry *smEntry = NULL;

	if (sipreg == NULL)
	{
		NETERROR(MSIP, ("%s sipreg is NULL\n", fn));
		return -1;
	}

	smEntry = &SipRegSM[sipreg->state][evHandle->event];

	if (smEntry == NULL)
	{
		NETERROR(MSIP, 
			("%s No State Machine Entry found\n", fn));

		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s State %s Event %s New State %s\n", fn, 
			GetSipRegState(sipreg->state),
			GetSipRegEvent(evHandle->type, evHandle->event),
			GetSipRegState(smEntry->nextState)));

	if (sipreg)
	{
		sipreg->state = smEntry->nextState;
	}

	return sipreg->state;

_error:
	return -1;
}

int
SipRegFindAppCallID(SipMsgHandle *msgHandle, char *callID)
{
	char fn[] = "SipRegFindAppCallID():";
	SipRegistration *sipreg;
	int rc = -1;
	SipCallLegKey 	leg = { 0 };

	leg.remote = msgHandle->remote;

	// There is no need to do a lookup here,
	// as we intend to keep the SIP call ID the same
	// as the app callID. This code looks like this to keep
	// this identical to the Call SM code. In future,
	// we may have a functional mapping from the sip call id
	// to our call id.
	CacheGetLocks(sipregCache,LOCK_READ,LOCK_BLOCK);

	sipreg = CacheGet(sipregCache, &leg);

	if (sipreg)
	{
		memcpy(callID, sipreg->callID, CALL_ID_LEN);
		rc = 1;
	}

	CacheReleaseLocks(sipregCache);

	return rc;
}
