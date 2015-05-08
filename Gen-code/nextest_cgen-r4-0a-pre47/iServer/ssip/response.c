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
#include "ssip.h"
#include "bcpt.h"
#include "siputils.h"

int
SipFormatResponse(
	SipMessage *s,
	SipEventContext *context,	/* context of request */
	SipMessage **r,
	int reason,
	char *reasonStr,
	SIP_S8bit **hostname, 
	SIP_U16bit *port, 
	SipError *err
)
{
	char fn[] = "SipFormatResponse():";
	SipMessage *m = NULL;

	if (!(*r) && 
		(sip_initSipMessage(r, SipMessageResponse, err) == SipFail))
	{
		NETERROR(MSIP, ("%s Error in init msg\n", fn));
		goto _error;
	}
	
	m = *r;

	/* Copy headers from request to response */
	if (SipCopyHeaders(s, m, context) < 0)
	{
		NETERROR(MSIP, ("%s Error in copying headers\n", fn));
		goto _error;
	}

	/* Copy headers from request to response */
	if (SipSetStatusLine(m, reason, reasonStr) < 0)
	{
		NETERROR(MSIP, ("%s Error in copying status\n", fn));
		goto _error;
	}
		
	if (SipGetSentByHost(m, context, 0, 1, hostname, port, err) <= 0)
	{
		NETERROR(MSIP, ("response404: Could not extract remote addr%d\n", *err));
		goto _error;
	}

	return 1;

_error:
	sip_freeSipMessage(m);

	return -1;
}
