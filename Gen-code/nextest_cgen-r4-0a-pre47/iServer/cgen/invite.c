#include <stdlib.h>
#include "bits.h"
#include "ipc.h"

#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"
#include "age.h"
#include "xmltags.h"

#include "gis.h"
#include <netdb.h>
#include <malloc.h>

pthread_t inv_thread;
extern SipOptions sip_options;
extern double probability; 
extern double aprobability; 
extern double bprobability; 

void 
SipIncomingRequest ( 
	SipMessage *s, 
	char *method,
	SipEventContext *context 
)
{
	char fn[] = "SipIncomingRequest():";
	SipError err;
	int rc = -1, reason = 400;

	DEBUG(MSIP, NETLOG_DEBUG3, 
		  ("%s Received an incoming request: %s\n", fn,method));

	/* We are just going to forward this one, no new request
	 * should be originated
	 */

	if (drand48() < probability)
	{
		goto _return;
	}

	if (!strcmp(method, "BYE"))
	{
		if (drand48() < bprobability)
		{
			goto _return;
		}
	}

	if (!strcmp(method, "ACK"))
	{
		if (drand48() < aprobability)
		{
			goto _return;
		}
	}

	if( SipTransIncomingMsg(s, method, context) < 0)
	{
		goto _error;
	}
		
	return;

 _return:
 _error:
	sip_freeSipMessage(s);
	SipFreeContext(context);

	return;
}

void 
sip_indicateInvite ( 
	SipMessage *s, 
	SipEventContext *context 
)
{
	char fn[] = "sip_indicateInvite():";
	
	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Request Received\n", fn));

	return SipIncomingRequest(s, "INVITE", context);
}

void 
SipIncomingResponse(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "SipIncomingResponse():";
	SipStatusLine *status_line ;
	SIP_U16bit code_num ;
	SIP_S8bit *callId = NULL;
	header_url *to = NULL, *from = NULL;
	SipError err;
	SIP_U32bit cseq;
	char *host = NULL;
	unsigned short port;
	int rcSentBy = 0;

	DEBUG(MSIP, NETLOG_DEBUG3, ("%s Received a response\n", fn));

	if (drand48() < probability)
	{
		goto _return;
	}

#if 0
	// Don't need to do this as we are a UA (stateful)
	// The TSM will tell us if we know about the transaction associated with
	// this response

	/* Check if we are at the top of the Via Header */
	if (SipCheckTopVia(s, &err, context) == 0)
	{
		/* We are not at the top of the Via Lost ...*/
		NETERROR(MSIP, ("%s We are not at the top of the Via list\n",
			fn));
		goto _error;
	} 
#endif
	
	/* Remove ourselves from the Via header */
	if (sip_deleteHeaderAtIndex(s, SipHdrTypeVia, 0, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not delete our via header\n", fn));
		goto _error;
	}

	/* This message may be meant for the UA itself */
	SipTransIncomingMsg(s, "RESPONSE", context);

	return;
	
_return:
_error:
	SipCheckFree(host);
	SipFreeContext(context);
	sip_freeSipMessage(s);

	return;
}
