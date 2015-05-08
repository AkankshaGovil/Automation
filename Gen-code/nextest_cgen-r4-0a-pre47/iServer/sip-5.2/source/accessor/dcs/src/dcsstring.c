/******************************************************************************
** FUNCTION:
** 	This file contains the source dCodeNum of all String DCS 
**      Header SIP stack APIs.
**
*******************************************************************************
**
** FILENAME:
** 	dcsstring.c
**
** DESCRIPTION:
**  	This Header file contains the source Code of all the DCS String APIs
**	
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 03Dec00   	S.Luthra    			Creation
**
** Copyrights 2000, Hughes Software Systems, Ltd.
**
******************************************************************************/
#include "dcsstring.h"
#include "portlayer.h"
#include "sipdecodeintrnl.h"
#include "sipparserinc.h"
#include "dcsinit.h"
#include "dcsfree.h"
#include "sipfree.h"
#include "dcsclone.h"



/*#ifdef SIP_THREAD_SAFE
extern synch_id_t glbSipParserMutex	;
#endif*/

SipBool sip_dcs_getDcsTypeFromString 
#ifdef ANSI_PROTO
	(SIP_S8bit *pHdrName, en_HeaderType  *pType, SipError *pErr)
#else
	(pHdrName, pType, pErr)
	SIP_S8bit *pHdrName;
	en_HeaderType  *pType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_getDcsTypeFromString");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdrName == SIP_NULL)||(pType == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	*pErr = E_INV_TYPE;
	if (strcasecmp("Remote-Party-ID:", pHdrName) == 0)
		*pType = SipHdrTypeDcsRemotePartyId;
	else if (strcasecmp("Dcs-Trace-Party-ID:", pHdrName) == 0)
		*pType = SipHdrTypeDcsTracePartyId;
	else if (strcasecmp("Anonymity:", pHdrName) == 0)
		*pType = SipHdrTypeDcsAnonymity;
	else if (strcasecmp("RPID-Privacy:", pHdrName) == 0)
		*pType = SipHdrTypeDcsRpidPrivacy;
	else if (strcasecmp("P-Media-Authorization:", pHdrName) == 0)
		*pType = SipHdrTypeDcsMediaAuthorization;
	else if (strcasecmp("Dcs-Gate:", pHdrName) == 0)
		*pType = SipHdrTypeDcsGate;
	else if (strcasecmp("State:", pHdrName) == 0)
		*pType = SipHdrTypeDcsState;
	else if (strcasecmp("Dcs-Osps:", pHdrName) == 0)
		*pType = SipHdrTypeDcsOsps;
	else if (strcasecmp("Dcs-Billing-ID:", pHdrName) == 0)
		*pType = SipHdrTypeDcsBillingId;
	else if (strcasecmp("Dcs-Billing-Info:", pHdrName) == 0)
		*pType = SipHdrTypeDcsBillingInfo;
	else if (strcasecmp("Dcs-LAES:", pHdrName) == 0)
		*pType = SipHdrTypeDcsLaes;
	else if (strcasecmp("Dcs-Redirect:", pHdrName) == 0)
		*pType = SipHdrTypeDcsRedirect;
	else if (strcasecmp("Session:", pHdrName) == 0)
		*pType = SipHdrTypeSession;
	else 
		return SipFail;	
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getDcsTypeFromString");
	return SipSuccess;
}

