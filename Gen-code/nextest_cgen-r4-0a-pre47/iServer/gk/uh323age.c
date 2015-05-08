#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"
#include "key.h"
#include "serverdb.h"
#include "age.h"
#include "gis.h"
#include "uh323.h"
#include "uh323cb.h"
#include "callcache.h"
#include "gk.h"
#include "stkutils.h"

#include "log.h"
#include "ls.h"
#include "ipstring.h"
#include "msg.h"

cmTransportAddress trAddr;

int
UH323AgeInit(void)
{
	char fn[] = "UH323AgeInit():";
	char  ipStr[80];
	int rc;

	/* If H.323 is enabled, initialize it */
	if (!H323Enabled())
	{
		NETDEBUG(MH323, NETLOG_DEBUG3, ("H.323 is disabled\n"));
		return -1;
	}

	NETDEBUG(MH323, NETLOG_DEBUG3, ("H.323 is enabled\n"));

	NETDEBUG(MH323, NETLOG_DEBUG3,
		("Initializing:: %s \n", cmGetVersion()));

	/* Initialize the stack instances */
	if (cmInitialize("./h323cfg-age.val", &UH323Globals()->hApp) < 0)
	{
		NETERROR(MH323, ("H.323 Initialization failed\n"));
		return -1;
	}

	UH323LoggingInit();

	return 0;
} 

int
UH323AgeInitCont(void)
{
	char fn[] = "UH323AgeInitCont():";
	char  ipStr[80];
	int rc;

	/* If H.323 is enabled, initialize it */
	if (!H323Enabled())
	{
		NETDEBUG(MH323, NETLOG_DEBUG3, ("H.323 is disabled\n"));
		return 0;
	}

	{
		NETDEBUG(MH323, NETLOG_DEBUG3, ("Stack Handle is %p\n", 
			UH323Globals()->hApp));

		/* Set our RAS event handler */
		cmRASSetEventHandler(UH323Globals()->hApp,
			&cmAgeRASEvent,
			sizeof(cmAgeRASEvent));

		/* Check our RAS address */
		rc = cmGetLocalRASAddress(
			UH323Globals()->hApp, 
			&trAddr);

		if (!rc)
		{
			NETDEBUG(MH323, NETLOG_DEBUG3,
				("cmGetLocalRASAddress: %s:%d\n\n", 
				liIpToString(trAddr.ip,ipStr), trAddr.port));
		}
	}

	/* We are ready to go into the main loop now */
	NETDEBUG(MH323, NETLOG_DEBUG3,	("H323 Initialization done\n"));

#if when_needed
	msSinkAdd("terminal");
	msAdd("UDPCHAN");
#if when_needen
	msAdd("CM");
	msAdd("CMAPI");
	msAdd("CMAPICB");
	msAdd("LI");
	msAdd("CHANNELS");
#endif
	msSetDebugLevel(3);
#endif
#ifndef NODEBUGLOG
	msSinkAdd("terminal");
#endif
#if 0
	msAdd("UDPCHAN");

	msAdd("CM");
	msAdd("CMAPI");
	msAdd("CMAPICB");

	msSetDebugLevel(4);
#endif
	return 0;
} 

int
GkSendIRQ(NetoidInfoEntry *entry)
{
	 char fn[] = "GkSendIRQ():";
	 cmTransportAddress
		  eptAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	 HRAS hsRas = NULL;

	 NETDEBUG(MRRQ, NETLOG_DEBUG1,
			  ("%s Sending IRQ for endpt %s Ip %s\n",
			   fn, entry->regid, 
			   (char*) ULIPtostring(entry->ipaddress.l)));

	 /* Fill in the RAS address */
	 eptAddr.ip = htonl(entry->rasip);
	 eptAddr.port = entry->rasport;

	 if (cmRASStartTransaction(UH323Globals()->hApp,
							   (HAPPRAS)NULL, /* Application Ras Handle */
							   &hsRas,
							   cmRASInfo,
							   &eptAddr,
							   NULL) < 0)
	 {
		  NETERROR(MRRQ, 
				   ("%s cmRASStartTransaction failed\n", fn));
		  return -1;
	 }	
 
	 /* Now start the transaction */
	 if (cmRASRequest(hsRas) < 0)
	 {
		  cmRASClose(hsRas);
		  NETERROR(MRRQ, ("%s cmRASRequest failed for registration\n", fn));
		  return -1;
	 }

	 NETDEBUG(MRRQ, NETLOG_DEBUG1,
		   ("%s Started the RAS Transaction successfully, hsRas = %p\n", 
			fn, hsRas));

	 return 0;
}

#ifdef _needed_
int
GkSendCallIRQ(CallHandle *callHandle)
{
	 char fn[] = "GkSendCallIRQ():";
	 cmTransportAddress
		  eptAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	 HRAS hsRas = NULL;
	 int nodeId, chNodeId;

	 NETDEBUG(MRRQ, NETLOG_DEBUG1,
			  ("%s Sending Call IRQs for call between endpt %s Ip %s\n and %s Ip %s",
			   fn, 
			   callHandle->phonode.regid, 
			   ULIPtostring(callHandle->phonode.ipaddress.l),
			   callHandle->rphonode.regid, 
			   ULIPtostring(callHandle->rphonode.ipaddress.l)
			   ));

	 /* Fill in the RAS address */
	 eptAddr.ip = htonl(callHandle->rasip);
	 eptAddr.port = callHandle->rasport;

	 if (cmRASStartTransaction(UH323Globals()->hApp,
							   (HAPPRAS)NULL, /* Application Ras Handle */
							   &hsRas,
							   cmRASInfo,
							   &eptAddr,
							   NULL) < 0)
	 {
		  NETERROR(MRRQ,
				   ("%s cmRASStartTransaction failed\n", fn));
		  return -1;
	 }	
 
	 nodeId = pdlGetProperty(hsRas);

	 /* printf("%s\n", cmGetProtocolMessageName(UH323Globals()->hApp, nodeId)); */

	 if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
							   nodeId, "request.infoRequest.callReferenceValue",
							   callHandle->inCRV, NULL) < 0)
	 {
		  NETERROR(MRRQ, 
				   ("%s CRV setup failed\n", fn));
	 }

	 if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
							   nodeId, "request.infoRequest.callIdentifier.guid",
							   16, &callHandle->callID[0]) < 0)
	 {
		  NETERROR(MRRQ, 
				   ("%s CallID setup failed\n", fn));
	 }

	 /* Now start the transaction */
	 if (cmRASRequest(hsRas) < 0)
	 {
		  cmRASClose(hsRas);
		  NETERROR(MRRQ, ("%s cmRASRequest failed for registration\n", fn));
		  return -1;
	 }

	 NETDEBUG(MRRQ, NETLOG_DEBUG1,
		   ("%s Started the RAS Transaction successfully, hsRas = 0x%x\n", 
			fn, hsRas));

	 return 0;
}
#endif

int
GkHandleIRR(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	 char fn[] = "GkHandleIRR():";
	 char endptIdStr[128] = { 0 };
	 BYTE endptId[128];
	 cmAlias alias;
	 int rc = -1;
	 PhoNode phonode = { 0 };
	 CacheTableInfo *cacheInfo = 0;
	 NetoidInfoEntry *netInfo;
	 INT32 tlen;
	 int nodeId, chNodeId;
	 HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	 char callID[CALL_ID_LEN];
	 RealmIP  realmip;

	 /* extract the endpoint info and refresh the client.
	  * See if there is any call information in the 
	  * IRR. If there is, refresh the call also.
	  */

	 /* Set up alias */
	 alias.string = &endptId[0];
	 alias.length = 128;
	 tlen = sizeof(cmAlias);

	 /* Extract the endpoint Id */
	 if (cmRASGetParam(hsRas, cmRASTrStageConfirm, 
					   cmRASParamEndpointID, 0, &tlen,
					   (char *)&alias) < 0)
	 {
		  NETERROR(MRRQ,
				   ("%s Could not get endpoint id\n", fn));
		  return rc;
	 }

	 /* Convert it back to a string */
	 utlBmp2Chr(&endptIdStr[0], alias.string, alias.length);

	 GisExtractRegIdInfo(endptIdStr, &phonode);
	 
	 NETDEBUG(MRRQ, NETLOG_DEBUG4,
			  ("%s Received from endpoint %s/%s\n",
			   fn, phonode.regid, (char*) ULIPtostring(phonode.ipaddress.l)));

	 rc = 1;

	 /* Now extract the endpoint and refresh it */

	 /* Lock the cache */
	 CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	 realmip.ipaddress = phonode.ipaddress.l;
	 realmip.realmId = phonode.realmId;
	 cacheInfo = (CacheTableInfo *)CacheGet(ipCache, &realmip);
	 if (cacheInfo == NULL)
	 {
		  goto _release_locks;
	 }

	 if (cacheInfo && 
		 (cacheInfo->data.stateFlags & CL_ACTIVE) &&
		 (cacheInfo->data.stateFlags & CL_REGISTERED))
	 {
		  netInfo = &cacheInfo->data;	
		  netInfo->rTime = (time(0)) +
				(rrqttl - RRQTTL_DEFAULT);

		  NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Entry refreshed at rTime %ld",
										 fn, (netInfo->rTime)));
	 }
	 else if (!(cacheInfo->data.stateFlags & CL_STATIC))
	 {
		  if (cacheInfo &&
			  (cacheInfo->data.stateFlags & CL_ACTIVE))
		  {
			   /* Update the refresh time */
			   netInfo = &cacheInfo->data;	

			   netInfo->rTime = (time(0)) +
					(rrqttl - RRQTTL_DEFAULT);
 
			   NETDEBUG(MRRQ, NETLOG_DEBUG4, 
						("%s Entry refreshed at rTime %ld",
						 fn, (netInfo->rTime)));
		  }

		  /* Not In cache... ah...
		 * Check the database. Maybe the netoid is aged.
		 * if database agrees with packet, bring him into
		 * cache. Else
		 * If we are the primary, we will send
		 * an error to the netoid 
		 */ 
		  NETDEBUG(MRRQ, NETLOG_DEBUG4,
				   ("%s No CacheInfo found or AGED\n", fn));
		
		  /* Send a URQ to the endpoint */
		  GkSendURQ(&cacheInfo->data);
	 }

	 nodeId = pdlGetProperty(hsRas);

	 nodeId = pvtGetNodeIdByPath(hVal, nodeId, 
								 "perCallInfo.1");
	 while (nodeId > 0)
	 {
		  NETDEBUG(MRRQ, NETLOG_DEBUG4,
				   ("%s Found Call Information in IRQ\n", fn));
			   
		  chNodeId = pvtGetNodeIdByPath(hVal, nodeId,
										"callIdentifier.guid");
		  pvtGetString(hVal, chNodeId, CALL_ID_LEN, &callID[0]);

		  PrintCallID(stdout, callID);
		  nodeId = pvtBrother(hVal, nodeId);
	 }
		  
		  
_release_locks:
	CacheReleaseLocks(regCache);

	return 0;
}
