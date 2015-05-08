#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <time.h>
#include <libradius.h>
#include <conf.h>
#include <radpaths.h>
#include "cdr.h"
#include "radclient.h"
#include "srvrlog.h"
#include "licenseIf.h"
#include "lsconfig.h"
#include "radacct.h"
#include "net.h"


int radius_initialised = 0;

char nas_ipaddr[16];

static int do_output = 1;
static int totalapp = 0;
static int totalchal = 0;
static int totaldeny = 0;


extern int bridgeSipEventProcessorWorker(SCC_EventBlock *evtPtr);

int rad_getport(const char *name)
{
	struct servent *svp;

	svp = getservbyname(name, "udp");
	if(!svp)
	{
		return 0;
	}

	return ntohs(svp->s_port);
}


int rad_send_packet(RADIUS_PACKET *req, RADIUS_PACKET **rep, char *secret)
{
	char fn[] = "sendPacket";
	struct pollfd pollfd;
	int i;
	int ret;

	for(i = 0; i < rad_retries; i++)
	{
		rad_send(req, NULL, secret);

		/* And wait for reply, timing out as necessary */
		memset(&pollfd, 0, sizeof(struct pollfd));

		pollfd.fd = req->sockfd;
		pollfd.events = POLLIN;

_try_again:
		ret = poll(&pollfd, 1, rad_timeout * 1000);

		switch(ret)
		{
			case  0:
				NETDEBUG(MRADC, NETLOG_DEBUG4, ("%s Poll timeout\n", fn));
				continue;

			case -1:
				NETERROR(MRADC, ("%s Poll error (errno %d)\n", fn, errno));
				if(errno == EINTR || errno == EAGAIN)
				{
					goto _try_again;
				}
				continue;

			default:
				if(pollfd.revents & POLLIN == 0)
				{
					NETDEBUG(MRADC, NETLOG_DEBUG4, ("%s Poll no input\n", fn));
					continue;
				}
		}

		*rep = rad_recv(req->sockfd);
		if(*rep != NULL)
		{
			break;
		}
		else
		{	/* NULL: couldn't receive the packet */
			NETERROR(MRADC, ("%s Failed to receive packet \n", fn));
			return -1;
		}
	}

	/* No response or no data read (?) */
	if(i == rad_retries)
	{
		NETDEBUG(MRADC, NETLOG_DEBUG2, ("%s No response from server \n", fn));
		return -1;
	}

	if(rad_decode(*rep, req, secret) != 0)
	{
		NETERROR(MRADC, ("%s Failed to decode packet \n", fn));
		return -1;
	}

	#if 0
	/* libradius debug already prints out the value pairs for us */
	if(!librad_debug && do_output)
	{
		printf("Received response ID %d, code %d, length = %d\n",
				(*rep)->id, (*rep)->code, (*rep)->data_len);
		vp_printlist(stdout, (*rep)->vps);
	}
	NETDEBUG(MRADC, NETLOG_DEBUG4, ("Received response ID %d, code %d, length = %d\n",
				(*rep)->id, (*rep)->code, (*rep)->data_len));
	#endif

	if((*rep)->code == PW_AUTHENTICATION_ACK)
	{
		totalapp++;
	}
	else if((*rep)->code == PW_ACCESS_CHALLENGE)
	{
		totalchal++;
	}
	else
	{
		totaldeny++;
	}

	return 0;
}


int startRadiusClient()
{
	char fn[] = "initRadiusClient";
	const char *radius_dir = RADDBDIR;
	int i, herror;

	radius_initialised = 0;
	if(dict_init(radius_dir, RADIUS_DICTIONARY) < 0)
	{
		NETERROR(MRADC, ("%s Failed to initialize Radius Dictionary \n", fn));
		return 1;
	}

	if(initRadiusAuth())
	{
		NETERROR(MRADC, ("%s Failed to initialize Radius Auth Client \n", fn));
		return 1;
	}

	if(initRadiusAcct())
	{
		NETERROR(MRADC, ("%s Failed to initialize Radius Acct Client \n", fn));
		return 1;
	}

	if( strlen(mgmtInterfaceIp) )
	{
		strcpy(nas_ipaddr, mgmtInterfaceIp);
	}
	else if(strlen(sipdomain) != 0)
	{
		FormatIpAddress(ResolveDNS(sipdomain, &herror), nas_ipaddr);
	}
	else
	{
		FormatIpAddress(ntohl(iServerIP), nas_ipaddr);
	}

	radius_initialised = 1;

	return 0;
}


void stopRadiusClient()
{
	if(radius_initialised)
	{
		NETDEBUG(MRADC, NETLOG_DEBUG4, ("Stopping Radius Client \n"));

		stopRadiusAuth();

		stopRadiusAcct();
	}
}
