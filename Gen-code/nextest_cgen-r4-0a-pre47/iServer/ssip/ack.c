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
sip_indicateAck(
	 SipMessage *s, 
	 SipEventContext *context
) 
{	
	return SipIncomingRequest(s, "ACK", context);
}
