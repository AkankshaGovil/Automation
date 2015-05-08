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

void 
sip_indicateCancel(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "sip_indicateCancel():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Req Rcvd \n", fn));
	return SipIncomingRequest(s, "CANCEL", context);
}

