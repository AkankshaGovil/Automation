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
#include <malloc.h>

void 
sip_indicateOptions(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "sip_indicateOptions():";

	NETERROR(MSIP, ("%s Request Received\n", fn));

	return SipIncomingRequest(s, "OPTIONS", context);
}


void 
sip_indicateUnknownRequest(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "sip_indicateUnknownRequest():";

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Req Rcvd.!----\n", fn));

	return SipIncomingRequest(s, "UNKNOWN", context);
}

void 
sip_indicateInformational(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "sip_indicateInformational():";

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Response Rcvd.!----\n", fn));

	return SipIncomingResponse(s, context);
}

void 
sip_indicateFinalResponse(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "sip_indicateFinalResponse():";

	DEBUG(MSIP, NETLOG_DEBUG3, ("%s Received a final response\n", fn));

	return SipIncomingResponse(s, context);
}

void 
sip_indicateTimeOut( SipEventContext *context)
{
	char fn[] = "sip_indicateTimeOut():";

	SipFreeContext(context);

	return;
}

void 
sip_indicateInfo(SipMessage *s, SipEventContext *context)
{
	char fn[] = "sip_indicateInfo():";

	NETDEBUG(MSIP, NETLOG_DEBUG4,  ("%s Req Rcvd.!----\n", fn));

	return SipIncomingRequest(s, "INFO", context);
}

void 
sip_indicatePropose(SipMessage *message, SipEventContext *context)
{
	NETERROR(MSIP, ("sip_indicatePropose Req Rcvd.!----\n"));
	sip_freeSipMessage(message);
	SipFreeContext(context);
	return;
}

void 
sip_indicatePrack(SipMessage *message, SipEventContext *context)
{
	NETERROR(MSIP, ("sip_indicatePrack Req Rcvd.!----\n"));
	sip_freeSipMessage(message);
	SipFreeContext(context);
	return;
}

void
sip_indicateUpdate(SipMessage *message, SipEventContext *context)
{
	NETERROR(MSIP, ("sip_indicateUpdate Req Rcvd.!----\n"));
	sip_freeSipMessage(message);
	SipFreeContext(context);
	return;
}

void
sip_indicatePublish(SipMessage *message, SipEventContext *context)
{
	NETERROR(MSIP, ("sip_indicatePublish Req Rcvd.!----\n"));
	sip_freeSipMessage(message);
	SipFreeContext(context);
	return;
}

SipBool 
sip_decryptBuffer(SipMessage *s, SIP_S8bit *encinbuffer, SIP_U32bit clen,SIP_S8bit **encoutbuffer, SIP_U32bit *outlen)
{
	NETERROR(MSIP, ("Rcvd request to decrypt a buffer!!!. not supported!!!\n"));
	return SipFail ;
}


// Enabling SIP_DCS flag in the compiler also enables other methods
// we will not handle Comet message. We will log this event and free the message for now.

void
sip_indicateComet(SipMessage *message, SipEventContext *context)
{
	NETERROR(MSIP, ("Comet Req Rcvd.!----\n"));
	sip_freeSipMessage(message);
	SipFreeContext(context);
	return;
}

void 
sip_indicateSubscribe(SipMessage* message, SipEventContext* context)
{
	NETDEBUG(MSIP,NETLOG_DEBUG4,("Subscribe Message Rcvd !"));
	return;
}

void
sip_indicateNotify(SipMessage* message, SipEventContext* context)
{
	NETDEBUG(MSIP,NETLOG_DEBUG4,("Notify Message recvd"));
	return SipIncomingRequest(message, "NOTIFY", context);
}

void 
sip_indicateMessage(SipMessage* message, SipEventContext* context)
{
	NETDEBUG(MSIP,NETLOG_DEBUG4,("Message Message Rcvd !"));
	return;
}
