#include "cm.h"
#include "uh323.h"
#include "uh323cb.h"
#include "serverp.h"
#include "arq.h"
#include "gk.h"

#include "ipstring.h"

/* PROTOTYPES of RAS CALLBACKS */
int cmEvRASRequest(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmRASTransaction	transaction,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall);

int cmEvRASConfirm(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas);

int cmEvRASReject(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason);
		
int cmEvRASTimeout(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas);

SCMRASEVENT cmRASEvent = { cmEvRASRequest, cmEvRASConfirm, cmEvRASReject, cmEvRASTimeout }; 

/* DEFINITIONS */
int cmEvRASConfirm(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	 char fn[] = "cmEvRASConfirm():";
	 cmRASTransaction transaction;

	 cmRASGetTransaction(hsRas, &transaction);

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s RAS Transaction %d Confirmed\n", fn, transaction));
	 
	 switch (transaction)
	 {
	 case cmRASGatekeeper:
		GkHandleGCF(haRas, hsRas);
	   break;
	 case cmRASRegistration:
		GkHandleRCF(haRas, hsRas);
	   break;
	 case cmRASAdmission:
		GkHandleACF(haRas, hsRas);
	   break;
	 case cmRASLocation:
	   GKHandleLCF(haRas, hsRas);
	   break;
	 case cmRASDisengage:
	   break;
	 case cmRASUnregistration:
	   break;
	 case cmRASResourceAvailability:
	   GkHandleRAIResponse(haRas, hsRas, "confirm");
	   break;
	 case cmRASBandwidth:
	   break;
	 case cmRASInfo: 
		GkHandleIRR(haRas, hsRas);
	   break;
	 case cmRASNonStandard:
	   break;
	 default:
	   break;
	 }

	 cmRASClose(hsRas);

	 return 1;
}

int cmEvRASReject(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	 char fn[] = "cmEvRASReject():";
	 cmRASTransaction transaction;
	 int rc = 1;

	 cmRASGetTransaction(hsRas, &transaction);

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s RAS Transaction %d Rejected\n", fn, transaction));
	 
	 switch (transaction)
	 {
	 case cmRASGatekeeper:
		rc = GkHandleGRJ(haRas, hsRas, reason);
	   break;
	 case cmRASRegistration:
		rc = GkHandleRRJ(haRas, hsRas, reason);
	   break;
	 case cmRASAdmission:
		GkHandleARJ(haRas, hsRas, reason);
	   break;
	 case cmRASLocation:
	   GKHandleLRJ(haRas, hsRas, reason);
	   break;
	 case cmRASDisengage:
	   break;
	 case cmRASUnregistration:
	   break;
	 case cmRASResourceAvailability:
	   GkHandleRAIResponse(haRas, hsRas, "reject");
	   break;
	 case cmRASBandwidth:
	   break;
	 case cmRASInfo: 
	   break;
	 case cmRASNonStandard:
	   break;
	 default:
	   break;
	 }

	 cmRASClose(hsRas);

	 return 1;
}

int cmEvRASTimeout(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	 char fn[] = "cmEvRASTimeout():";
	 cmRASTransaction transaction;

	 cmRASGetTransaction(hsRas, &transaction);

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s RAS Transaction %d Timed out\n", fn, transaction));
	 
	 switch (transaction)
	 {
	 case cmRASGatekeeper:
		GkHandleGRJ(haRas, hsRas, -1);
	   break;
	 case cmRASRegistration:
		GkHandleRRJ(haRas, hsRas, -1);
	   break;
	 case cmRASAdmission:
		GkHandleARQTimeout(haRas, hsRas);
	   break;
	 case cmRASLocation:
	   GKHandleLRQTimeout(haRas, hsRas);
	   break;
	 case cmRASDisengage:
	   break;
	 case cmRASUnregistration:
	   break;
	 case cmRASResourceAvailability:
	   GkHandleRAIResponse(haRas, hsRas, "timeout");
	   break;
	 case cmRASBandwidth:
	   break;
	 case cmRASInfo: 
	   break;
	 case cmRASNonStandard:
	   break;
	 default:
	   break;
	 }

	 cmRASClose(hsRas);

	 return 1;
}

// Based on the return value of fn called, send
// a Confirm or a Reject. If any special action needs
// to be taken, the called fn should return 0
int cmEvRASRequest(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmRASTransaction	transaction,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	 char fn[] = "cmEvRASRequest():";
	 int rc = -1, noprocess = 0;

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		   ("%s (%s) - trans %d hsRas %p, hsCall %p, haCall %p\n",
			fn, (char*) ULIPtostring(ntohl(srcAddress->ip)),
			transaction, hsRas, hsCall, haCall));

	 switch (transaction)
	 {
	 case cmRASGatekeeper:
	   rc = GkHandleGRQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASRegistration:
	   rc = GkHandleRRQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASAdmission:
	   rc = GkHandleARQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASLocation:
	   rc = GKHandleLRQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASDisengage:
	   rc = GkHandleDRQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASUnregistration:
	   rc = GkHandleURQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASResourceAvailability:
	   rc = GkHandleRAI(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASBandwidth:
	   rc = GkHandleBRQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	   break;
	 case cmRASInfo: 
	   noprocess = 1;
	   break;
	 case cmRASNonStandard:
	   noprocess = 1;
	   break;
	 default:
	   noprocess = 1;
	   break;
	 }

	if ((transaction != cmRASDisengage) &&
			(transaction != cmRASAdmission) && hsCall)
	 {
		UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;

		if (!appHandle || 
			(CacheFind(callCache, appHandle->callID, NULL, 0) < 0))
		{
			NETERROR(MH323, 
				("%s Unexpected Call Handle Created by the stack for trans %d\n",
				fn, transaction));
			if (hsCall) cmCallDrop(hsCall);
		}
	 }

	 if (noprocess)
	 {
	 	cmRASClose(hsRas);
		rc = 0;
	 }

	 if (rc < 0)
	 {
		  cmRASReject(hsRas, -rc);
//		if (hsCall) cmCallDrop(hsCall);
	 }
	 else if (rc > 0)
	 {
		  cmRASConfirm(hsRas);
	 }
	 else if (rc == 0)
	 {
		  return 0;
	 }
			
	 cmRASClose(hsRas);

	 return 1;
}
