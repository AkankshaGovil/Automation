#include "gis.h"
#include "net.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include <netdb.h>
#include <malloc.h>

int
SipTranSendThreadLaunch(
	SipMessage *s, 
	SipEventContext *context,
	int type, 
	char *host,
	unsigned short port, unsigned long *sendhost, unsigned short *sendport)
{
	char fn[] = "SipTranSendThreadLaunch():";
	long ipaddr;
	int herror;

	ipaddr = ResolveDNS(host, &herror);

	if (ipaddr == -1)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3, ("%s Bad ip address for %s\n", fn, host));

		if(context && context->pTranspAddr)
		{
			ipaddr = ntohl(inet_addr(context->pTranspAddr->dIpv4));
		}

		if(ipaddr == -1)
		{
			goto _error;
		}
	}
	
	if (port == 0)
	{
		port = 5060;
	}

	*sendport = (unsigned short) port;
	*sendhost = (unsigned long) ipaddr;

	NETDEBUG(MSIP, NETLOG_DEBUG3,
		("%s Sending message over to %lx/%d\n", fn, ipaddr, port));

	/* Forward the message */
	if (SipSendMessage(s, ipaddr, port, context, SIPPROTO_UDP) < 0)
	{
		NETERROR(MSIP, ("%s Could not forward Invite\n", fn));
		goto _error;
	}

_return:
	SipCheckFree(host);

	return 0;
_error:
	SipCheckFree(host);

	return -1;
}
