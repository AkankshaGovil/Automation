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
sip_indicateBye(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "sip_indicateBye():";
	return SipIncomingRequest(s, "BYE", context);
}

