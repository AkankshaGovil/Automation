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
sip_indicateRefer ( 
	SipMessage *s, 
	SipEventContext *context 
)
{
	char fn[] = "sip_indicateRefer():";
	
	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Request Received\n", fn));

	return SipIncomingRequest(s, "REFER", context);
}
