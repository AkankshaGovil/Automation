#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"

#include "cm.h"
#include "uh323.h"
#include "uh323cb.h"
#include "gk.h"

#include "ipstring.h"

/* PROTOTYPES of RAS CALLBACKS */
int cmEvAgeRASRequest(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmRASTransaction	transaction,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall);

int cmEvAgeRASConfirm(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas);

int cmEvAgeRASReject(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason);
		
int cmEvAgeRASTimeout(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas);

SCMRASEVENT cmAgeRASEvent = { cmEvAgeRASRequest, cmEvAgeRASConfirm, 
							  cmEvAgeRASReject, cmEvAgeRASTimeout }; 

/* DEFINITIONS */
int cmEvAgeRASConfirm(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	 char fn[] = "cmEvAgeRASConfirm():";
	 cmRASTransaction transaction;

	 cmRASGetTransaction(hsRas, &transaction);

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s RAS Transaction %d Confirmed\n", fn, transaction));
	 
	 switch (transaction)
	 {
	 case cmRASGatekeeper:
		break;
	 case cmRASRegistration:
	   	break;
	 case cmRASAdmission:
	   break;
	 case cmRASLocation:
	   break;
	 case cmRASDisengage:
	   break;
	 case cmRASUnregistration:
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

	 return 0;
}

int cmEvAgeRASReject(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	 char fn[] = "cmEvAgeRASReject():";
	 cmRASTransaction transaction;

	 cmRASGetTransaction(hsRas, &transaction);

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s RAS Transaction %d Rejected\n", fn, transaction));
	 
	 switch (transaction)
	 {
	 case cmRASGatekeeper:
	   break;
	 case cmRASRegistration:
	   	break;
	 case cmRASAdmission:
	   break;
	 case cmRASLocation:
	   break;
	 case cmRASDisengage:
	   break;
	 case cmRASUnregistration:
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

	 return 0;
}

int cmEvAgeRASTimeout(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	 char fn[] = "cmEvAgeRASTimeout():";
	 cmRASTransaction transaction;

	 cmRASGetTransaction(hsRas, &transaction);

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s RAS Transaction %d Timed out\n", fn, transaction));
	 
	 switch (transaction)
	 {
	 case cmRASGatekeeper:
	   break;
	 case cmRASRegistration:
	   	break;
	 case cmRASAdmission:
	   break;
	 case cmRASLocation:
	   break;
	 case cmRASDisengage:
	   break;
	 case cmRASUnregistration:
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

	 return 0;
}

int cmEvAgeRASRequest(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmRASTransaction	transaction,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	 char fn[] = "cmEvAgeRASRequest():";
	 int rc = -1;

	 NETDEBUG(MH323, NETLOG_DEBUG4,
		   ("%s (%s) - trans %d hsRas %p, hsCall %p, haCall %p\n",
			fn, (char*) ULIPtostring(ntohl(srcAddress->ip)),
			transaction, hsRas, hsCall, haCall));

	 switch (transaction)
	 {
	 case cmRASGatekeeper:
	   break;
	 case cmRASRegistration:
	   break;
	 case cmRASAdmission:
	   break;
	 case cmRASLocation:
	   break;
	 case cmRASDisengage:
	   break;
	 case cmRASUnregistration:
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

	 if (rc < 0)
	 {
		  cmRASReject(hsRas, 0);
	 }
	 else if (rc > 0)
	 {
		  cmRASConfirm(hsRas);
	 }
	 else	if (rc == 0)
	 {
		  return 0;
	 }
			
	 cmRASClose(hsRas);

	 return 0;
}

