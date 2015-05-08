#include <sys/types.h>
#include <time.h>
#include "gis.h"
#include "sipcallactions.h"
#include "firewallcontrol.h"
#include <malloc.h>
#include "log.h"
#include "ua.h"
#include "scm_call.h"
#include "callutils.h"
#include "bridge.h"


void scmCallHandleUpdate(CallHandle *callHandle)
{
	char fn[] = "scmCallHandleUpdate():";
	char callidString[CALL_ID_LEN], confidString[CONF_ID_STRLEN];
	ConfHandle *confHandle = NULL;
	CallHandle *oldCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG2, ("%s: Entering\n", fn));

	// Must initialise empty lists
	callHandle->evtList = listInit();
	callHandle->fcevtList = listInit();

	// Reset none existent timers
	callHandle->rolloverTimer = 0;
	callHandle->max_call_duration_tid = 0;
	callHandle->handle.sipCallHandle.timerC = 0;
	callHandle->handle.sipCallHandle.sessionTimer = 0;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	if((oldCallHandle = CacheDelete(callCache, callHandle->callID)))
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: Found existing CallHandle (%s) in Conf (%s)\n", fn,
												CallID2String(callHandle->callID, callidString),
												ConfID2String(callHandle->confID, confidString)));

		GisDeleteCallFromConf(oldCallHandle->callID, oldCallHandle->confID);
		CallDelete(callCache, oldCallHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(oldCallHandle)->callLeg);
	}

	if (CacheInsert(callCache, callHandle) != -1)
	{
		if (CacheInsert(sipCallCache, callHandle) != -1)
		{
			if(GisAddCallToConfGetConf(callHandle, &confHandle) != -1)
			{
				NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: Added CallHandle (%s) to Conf (%s)\n", fn,
													CallID2String(callHandle->callID, callidString),
													ConfID2String(callHandle->confID, confidString)));

				openPinholeOnBkup(callHandle, oldCallHandle, confHandle);

				SCMCALL_Replicate(callHandle);
			}
			else
			{
				NETERROR(MSIP,  ("%s: Failed to add CallHandle (%s) to Conf (%s)\n", fn,
													CallID2String(callHandle->callID, callidString),
													ConfID2String(callHandle->confID, confidString)));

				CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);
				CacheDelete(callCache, callHandle->callID);
				GisFreeCallHandle(callHandle);
			}
		}
		else
		{
			NETERROR(MSIP, ("%s: Could not insert CallHandle (%s) in sip call cache\n", fn,
													CallID2String(callHandle->callID, callidString)));

			CacheDelete(callCache, callHandle->callID);
			GisFreeCallHandle(callHandle);
		}
	}
	else
	{
		NETERROR(MSIP, ("%s: Could not insert CallHandle (%s) in call cache\n", fn,
										CallID2String(callHandle->callID, callidString)));

		GisFreeCallHandle(callHandle);
	}

	if(oldCallHandle)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: Deleting old CallHandle (%s)\n", fn,
										CallID2String(oldCallHandle->callID, callidString)));

		GisFreeCallHandle(oldCallHandle);
	}

	CacheReleaseLocks(callCache);

	NETDEBUG(MSIP, NETLOG_DEBUG2, ("%s: Exiting\n", fn));
}

void scmCallHandleDelete(char *callid)
{
	char fn[] = "scmCallHandleDelete():";
	CallHandle *callHandle = NULL;
	char callidString[CALL_ID_LEN];

	NETDEBUG(MSIP, NETLOG_DEBUG2, ("%s: Entering\n", fn));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	if((callHandle = CacheGet(callCache, callid)))
	{
		// Close holes if they had been opened on this handle
		if (CallFceBundleId(callHandle) != 0)
		{
			FCECloseBundle(CallFceSession(callHandle), CallFceBundleId(callHandle));
			CallFceBundleId(callHandle) = 0;
		}

		GisDeleteCallFromConf(callHandle->callID, callHandle->confID);
		CallDelete(callCache, callHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);

		SCMCALL_Delete(callHandle);

		GisFreeCallHandle(callHandle);
	}
	else
	{
		NETERROR(MSIP, ("%s: Could not find callid (%s) in call cache\n", fn,
										CallID2String(callid, callidString)));
	}

	CacheReleaseLocks(callCache);

	NETDEBUG(MSIP, NETLOG_DEBUG2, ("%s: Exiting\n", fn));
}

int openPinholeOnBkup(CallHandle *newCallh, CallHandle *oldCallh, ConfHandle *confh)
{
	static char *fn = "openPinholeOnBkup";
	int retval = FCE_GENERAL_FAILURE;
	int license_allocated = 0;
	char * callID = malloc(CALL_ID_LEN);

	memcpy(callID, newCallh->callID, CALL_ID_LEN); 

	if (oldCallh)
	{
		if (CmpFceMediaHole(&newCallh->fceHandle, &oldCallh->fceHandle) == 0)
		{
			NETDEBUG(MFCE, NETLOG_DEBUG2, 
					("%s: media identical in old and new CallHandles\n", fn));

			retval = FCE_SUCCESS;
			return retval;
		}
		else 
		{	 // change in media

			// Close previously opened holes (if any)
			if (CallFceBundleId(oldCallh) != 0)
			{
				FCECloseBundle(CallFceSession(oldCallh), CallFceBundleId(oldCallh));
				CallFceBundleId(oldCallh) = 0;
				// Entry from tipCache must be removed by CallDelete on this handle
				license_allocated = 1;
			}
	    }
	}

	// Adjust the bundleId 
	if (CallFceBundleId(newCallh) == 0)
	{
		NETDEBUG(MFCE, NETLOG_DEBUG2, ("%s: No MR on the callhandle\n", fn));
		retval = FCE_SUCCESS;
		return retval;
	}	

	FCEAdjustBundleId(CallFceBundleId(newCallh));	


	if ((!license_allocated) && (allocateFCELicense(confh) < 0))
	{
		retval = FCE_LICENSE_FAILURE;
		return retval;
	}		

	// license allocated, open a new hole

	CallFceSession(newCallh) = FCEGetSession();


	// TODO - have to store the dtmf flags also on the fceHandle so that they can be passed here
	retval = FCEOpenResourceAsync(CallFceSession(newCallh),
									CallFceBundleId(newCallh),
									0,
									FCE_ANY_IP,
									FCE_ANY_PORT,
									CallFceUntranslatedIp(newCallh),
									CallFceUntranslatedPort(newCallh),
									CallFceNatSrcIp(newCallh),
									CallFceNatSrcPort(newCallh),
									CallFceNatDstIp(newCallh),
									CallFceNatDstPort(newCallh),
									newCallh->realmInfo->mPoolId,
									CallFceNatSrcPool(newCallh),
									"rtp",
									0,
									0,
									0,
									MFCPCallbackOnBkup,
									callID);
                                    
	if (retval == MFCP_RET_OK)
	{
		retval = FCE_INPROGRESS;
	}
	else
	{
		NETERROR(MFCE, ("%s: unable to open hole for bundle id %d\n", fn, CallFceBundleId(newCallh)));

		retval = FCE_GENERAL_FAILURE;
	}

	if (retval == FCE_INPROGRESS)
	{
		// Insert this call Handle into the tipCache
		if (CacheInsert(tipCache, newCallh) < 0)
		{
			NETERROR(MBRIDGE, ("%s Unable to insert %lx/%d into tipCache\n", 
				fn, CallFceTranslatedIp(newCallh), CallFceTranslatedPort(newCallh)));
		}

		NETDEBUG(MFCE, NETLOG_DEBUG2, 
				("%s: queued up to open media hole for bundle id %d\n", fn, CallFceBundleId(newCallh)));
	}

	return retval;
}


void MFCPCallbackOnBkup(MFCP_Request *rPtr)
{
	static char *fn = "MFCPCallbackOnBkup";
	char ipstr1[32], ipstr2[32];
	int resId = 0;
	CallHandle *callHandle = NULL;
  	char *callID;
	char callidString[CALL_ID_LEN];

	if (rPtr)
	{
		// Is an MFCP response
		if (mfcp_get_res_status(rPtr) == MFCP_RSTATUS_OK)
		{
			NETDEBUG(MFCE,NETLOG_DEBUG2, ("%s: nat_dst = %s:%d, nat_src = %s:%d\n",fn,
											FormatIpAddress(mfcp_get_nat_dest_addr(rPtr), ipstr1),
											mfcp_get_nat_dest_port(rPtr),
											FormatIpAddress(mfcp_get_nat_src_addr(rPtr), ipstr2),
											mfcp_get_nat_src_port(rPtr)));

			
      		resId = mfcp_get_int(rPtr, MFCP_PARAM_RESOURCE_ID);
		}
		else
		{
			NETERROR(MFCE, ("%s: unable to open media hole for %d:%d->%d: %s\n", fn, \
								mfcp_get_int(rPtr, MFCP_PARAM_BUNDLE_ID), \
								mfcp_get_int(rPtr, MFCP_PARAM_SRC_POOL), \
								mfcp_get_int(rPtr, MFCP_PARAM_DEST_POOL), \
								mfcp_get_res_estring(rPtr)));
		}

  		if (callID = (char *)mfcp_get_res_appdata(rPtr)) {

			CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

			if((callHandle = CacheGet(callCache, callID))) {
		  		if (resId > 0) {
				  	CallFceResourceId(callHandle) = resId;	
				}
				else {
				  	CallFceResourceId(callHandle) = 0;	
				  	CallFceBundleId(callHandle) = 0;	
				}
			}
			else {
				NETERROR(MFCE, ("%s: Could not find callid (%s) in call cache\n", fn,
											CallID2String(callID, callidString)));
			}
	
			free(callID);

			CacheReleaseLocks(callCache);
		}

		// free the mfcp resources
		mfcp_req_free(rPtr);
	}

}

/*
 * Return 0 if equal else 1
 */
int
CmpFceMediaHole(FceMediaHoleHandle *mh1, FceMediaHoleHandle *mh2)
{
	if (mh1->bundleId != mh2->bundleId)
		return 1;

  	if (mh1->translatedIp != mh2->translatedIp)
		return 1;

 	 if (mh1->translatedPort != mh2->translatedPort)
		return 1;

	if (mh1->refCount != mh2->refCount)
		return 1;
	
	if (mh1->natSrcIp != mh2->natSrcIp)
		return 1;
	
	if (mh1->natSrcPort != mh2->natSrcPort)
		return 1;
	
	if (mh1->natSrcPool != mh2->natSrcPool)
		return 1;
	
	if (!memcmp(mh1->srcCallID, mh2->srcCallID, CALL_ID_LEN))
		return 1;
	
	if (mh1->untranslatedIp != mh2->untranslatedIp)
		return 1;
	
	if (mh1->untranslatedPort != mh2->untranslatedPort)
		return 1;
#if 0	
	if (mh1->resourceId != mh2->resourceId)
		return 1;
#endif	
	return 0;
}
