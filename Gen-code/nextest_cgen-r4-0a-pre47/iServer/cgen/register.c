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

/* As of Rel 1.3, we will support only one
 * contact address in the registration request.
 * Additive registrations may thus cause replacement of
 * the contact address. We will fill the action value
 * as "proxy" always. The callid in the registration
 * is not checked (thus no additions). The expires param
 * is used to do the timeouts. However there is no way
 * we do unregistrations when the timeout expires (The
 * CL_STATIC flag will be helful, if you want to make
 * all registrations static. An expires=0 will do the 
 * unregistration. The From header is not evaluated,
 * so third party registrations of the device are ok.
 * If the req-uri is not our own, we will forward
 * the registration. We will look for the To URL
 * in our cache. If its not found, the req-uri is
 * used to forward the registration.
 */
void 
sip_indicateRegister(
	SipMessage *s, 
	SipEventContext *context
)
{
	return;
}

/* Add the contact, se the params, send back the OK
 * with the contact params and the action.
 * OK might require a DNS
 */
int
SipRegisterContact(
	SipMessage *s, 
	SipEventContext **context,
	CacheTableInfo *cacheInfo, int rxedFormatNat, int generate_response
)
{
	return;
}
