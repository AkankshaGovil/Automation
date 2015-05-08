#include "gis.h"
#include <malloc.h>

#include "sipcallactions.h"
#include "dbs.h"
#include "ua.h"

int 
SipRegHandleBridgeRegister(SipEventHandle *evb)
{
	char fn[] = "SipRegHandleBridgeRegister():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipRegistration *sipreg;
	SipCallLegKey 	leg = { 0 };

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	leg.remote = appMsgHandle->calledpn;

	CacheGetLocks(sipregCache, LOCK_WRITE, LOCK_BLOCK);
	
	sipreg = CacheGet(sipregCache, &leg);

	if (!sipreg)
	{
		NETERROR(MSIP, ("%s no sipreg handle\n", fn));
		goto _release_locks;
	}

	SipBridgeInitSipCallHandle(evb, &sipreg->sipch);

	// For REGISTER, we must make the From and To to be the same
	// This code should be in uautils.c
	// UrlFree(sipreg->sipch.callLeg.local, sipregCache->malloc);
	// sipreg->sipch.callLeg.local = UrlDup(sipreg->sipch.callLeg.remote, sipregCache->malloc);

	// Create a msgHandle for the TSM and send the registration out

	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle2(&sipreg->sipch, 
						"REGISTER", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn));
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	CacheReleaseLocks(sipregCache);

	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(sipregCache);
	return 1;

_error:
	CacheReleaseLocks(sipregCache);
	return -1;
}

int 
SipRegHandleBridgeReRegister(SipEventHandle *evb)
{
	char fn[] = "SipRegHandleBridgeReRegister():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipRegistration *sipreg;
	SipCallLegKey 	leg = { 0 };

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	leg.remote = appMsgHandle->calledpn;

	CacheGetLocks(sipregCache, LOCK_WRITE, LOCK_BLOCK);
	
	sipreg = CacheGet(sipregCache, &leg);

	sipreg->sipch.localCSeqNo ++;

	if (!sipreg)
	{
		NETERROR(MSIP, ("%s no sipreg handle\n", fn));
		goto _release_locks;
	}

	// Create a msgHandle for the TSM and send the registration out

	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle2(&sipreg->sipch, 
						"REGISTER", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn));
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	CacheReleaseLocks(sipregCache);

	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(sipregCache);
	return 1;

_error:
	CacheReleaseLocks(sipregCache);
	return -1;
}

int 
SipRegHandleSMError (SipEventHandle *evb)
{
	NETERROR(MSIP, ("SIP Registration S/M error \n"));
	SipFreeEventHandle(evb);
	return(0);
}

int 
SipRegNoOp (SipEventHandle *evb)
{
	SipFreeEventHandle(evb);
	return(0);
}

int 
SipRegHandleNetworkConfirm (SipEventHandle *evb)
{
	char fn[] = "SipRegHandleNetworkConfirm():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipRegistration *sipreg;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;
	char uri[SIPURL_LEN];
	CacheTableInfo *info;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.remote = msgHandle->remote;

	CacheGetLocks(sipregCache, LOCK_WRITE, LOCK_BLOCK);
	
	sipreg = CacheGet(sipregCache, &leg);

	if (!sipreg)
	{
		NETERROR(MSIP, ("%s no sipreg handle\n", fn));
		CacheReleaseLocks(sipregCache);
		goto _release_locks;
	}

	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans2(evHandle, &sipreg->sipch, "REGISTER");

	// Create a msgHandle for the TSM and send the registration out

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}

	printf("Registration Confirmed.\n");		

	CacheReleaseLocks(sipregCache);
/*

	// Update the endpoint
	sprintf(uri, "%s@%s",
			SVal(msgHandle->remote->name),SVal(msgHandle->remote->host));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	info = CacheGet(uriCache, uri);
	
	if (info == NULL)
	{
		// We need to abandon this registration
		NETERROR(MSIP, ("%s no endpoint found\n", fn));
	}
	else
	{
		info->data.stateFlags |= (CL_ACTIVE|CL_REGISTERED);

		// Take this out of the S/M - Remove this when s/m timer
		// code is there

		info->data.stateFlags &= ~CL_UAREGSM;
		time(&info->data.rTime);
		DbScheduleIedgeUpdate(&info->data);
	}

	CacheReleaseLocks(regCache);

	// Start a timer for the next registration
*/

_release_locks:
	SipFreeEventHandle(evb);
	return 1;

_error:
	SipFreeEventHandle(evb);
	return -1;
}

int 
SipRegHandleNetwork3xx (SipEventHandle *evb)
{
	return(0);
}

int 
SipRegHandleNetworkFailure (SipEventHandle *evb)
{
	char fn[] = "SipRegHandleNetworkFailure():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipRegistration *sipreg;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;
	char uri[SIPURL_LEN];
	CacheTableInfo *info;
	char *auth = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.remote = msgHandle->remote;

	CacheGetLocks(sipregCache, LOCK_WRITE, LOCK_BLOCK);
			
	// Delete the sipreg entry from the cache
	sipreg = CacheDelete(sipregCache, &leg);

	if (!sipreg)
	{
		NETERROR(MSIP, ("%s no sipreg handle\n", fn));
		CacheReleaseLocks(sipregCache);
		goto _error;
	}

	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans2(evHandle, &sipreg->sipch, "REGISTER");

	sprintf(uri, "%s@%s", SVal(msgHandle->remote->name),SVal(msgHandle->remote->host));

	/* Registration failed. */
        printf("Registration Failed.\n");

	// Create a msgHandle for the TSM and send the registration out

	SipRegistrationFree(sipreg, sipregCache->free);

	CacheReleaseLocks(sipregCache);
/*

	// Update the endpoint

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	info = CacheGet(uriCache, uri);
			
	if (info == NULL)
	{
		// We need to abandon this registration
		NETERROR(MSIP, ("%s no endpoint found\n", fn));
		CacheReleaseLocks(regCache);
		goto _error;
	}
	else
	{
		if(appMsgHandle->hdrWwwauthenticate)
		{
			auth = createAuth(&info->data, msgHandle->method, appMsgHandle->hdrWwwauthenticate);
		}
		else
		{
			auth = createAuth(&info->data, msgHandle->method, appMsgHandle->hdrProxyauthenticate);
		}

		if(auth == NULL)
		{
			info->data.stateFlags &= ~CL_UAREGSM;
		}
	}

	CacheReleaseLocks(regCache);
*/

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
				
	if(auth && info->data.stateFlags & CL_UAREG)
	{
		SipRegStartRegSM(&info->data, auth);
	}

	SipFreeEventHandle(evb);
	return 1;

_error:
	if(auth)
	{
		free(auth);
	}
	SipFreeEventHandle(evb);
	return -1;
}

int 
SipRegHandleBridgeRegisterTimer (SipEventHandle *evb)
{
	return(0);
}

SipRegistration *
SipRegistrationNew(int mallocfn)
{
	SipRegistration *sipreg;

	sipreg = MMalloc(mallocfn, sizeof(SipRegistration));
	memset(sipreg, 0, sizeof(SipRegistration));

	return sipreg;
}

void
SipRegistrationFree(SipRegistration *sipreg, int freefn)
{
	if (sipreg)
	{
		SipFreeSipCallHandle(&sipreg->sipch, freefn);
		MFree(freefn, sipreg);
	}
}
