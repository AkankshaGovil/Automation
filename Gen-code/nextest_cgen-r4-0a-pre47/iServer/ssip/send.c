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
#include "net.h"

/* An Invite request requires to be multithreaded */
int
SipSendMsgToHost(
	SipMessage *s, 
	SipEventContext *context,
	int type, 
	char *host,
	unsigned short port)
{
	char fn[] = "SipSendMsgToHost():";
	long ipaddr;
	int herror;

	ipaddr = ResolveDNS(host, &herror);

	if (ipaddr == -1)
	{
		NETERROR(MSIP, ("%s Bad ip address after resolv for %s\n",
			fn, host));

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

	NETDEBUG(MSIP, NETLOG_DEBUG3,
		("%s Sending message over to %lx/%d\n", fn, ipaddr, port));

	/* Forward the message */
	if (SipSendMessage(s, ipaddr, port, context, SIPPROTO_UDP) < 0)
	{
		NETERROR(MSIP, ("%s Could not send msg\n", fn));
		goto _error;
	}

_return:
	SipCheckFree (host);
	sip_freeSipMessage(s);
	SipFreeContext(context);
	return 0;
_error:
	SipCheckFree (host);
	sip_freeSipMessage(s);
	SipFreeContext(context);
	return -1;
}

int
SipSendMessage(SipMessage *s, unsigned long ip, unsigned short port,
	void *data, int proto)
{
	char fn[] = "SipSendMessage():";
	SipTranspAddr transpadr;
	SipEventContext *context = (SipEventContext *)data;
	extern SipOptions sip_options;
	SipError siperr;
	CacheRealmEntry *realmEntry;
	CallRealmInfo *realmInfo;

	if (context == NULL) 
	{
		NETERROR (MSIP, ("%s No context !", fn));
		goto _error;
	}

	transpadr.dPort = port;
	FormatIpAddress(ip, transpadr.dIpv4);

	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);

	realmInfo = (CallRealmInfo *)context->pData;
	realmEntry = CacheGet (realmCache, (void *)&realmInfo->realmId);

	if (realmEntry == NULL) {
		NETERROR (MSIP, ("%s No realm entry found for realm %d",
				 fn, (int) realmInfo->realmId));
		CacheReleaseLocks (realmCache);
		goto _error;
	}

	transpadr.dSockFd = realmEntry->socketId;

	CacheReleaseLocks (realmCache);

	if ((sip_sendMessage(s, &sip_options,
		&transpadr, SIP_UDP|SIP_NORETRANS,
		NULL, &siperr)) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not send message to %s/%d\n",
			fn, transpadr.dIpv4, transpadr.dPort));
		goto _error;
	}

	return 0;
_error:
	return -1;
}
