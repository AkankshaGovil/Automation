#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include    <sys/un.h>
#include    <unistd.h>
#include	<netinet/in.h>
#include	<netinet/in_systm.h>
#include	<netinet/ip.h>
#include <stdio.h>
/* Define symbol __FAVOR_BSD in case of linux 
for the udphdr structure
*/
#ifdef NETOID_LINUX
#define __FAVOR_BSD
#endif
#include	<netinet/udp.h>
#ifdef NETOID_LINUX
#undef __FAVOR_BSD
#endif
#include 	<errno.h>
#include        "nxioctl.h"
#include        "nxosdtypes.h"
#include	"srvrlog.h"
#include	"clist.h"
#include        "common.h"
#include 	<malloc.h>

#define ICMPBUFSIZE	4096
typedef struct icmpcbdata
{
	struct icmpcbdata *prev, *next;

	uchar_t ip_p;
	struct sockaddr_in src, dest;	

	int (*fn)(char *, int, void *);
	void *data;

} icmpcbdata;

// Needs lock protection
icmpcbdata *icmpcbs = NULL;

void *
IcmpdInit(void *arg)
{
	// Open a raw socket
	char fn[] = "IcmpdInit():";
	int fd;
	struct sockaddr_in from;
	int fromlen = sizeof(from), nbytes;
	char icmpbuf[ICMPBUFSIZE];

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	// Now just read from the network
	while ((nbytes = recvfrom(fd, icmpbuf, ICMPBUFSIZE, 0, 
			(struct sockaddr *)&from, &fromlen)) > 0)
	{
		// process the packet
		IcmpdProcessPacket(icmpbuf, nbytes, &from, fromlen);
	}

	// Should never be here
	NETERROR(MICMPD, ("%s terminating, errno = %d\n",
			fn, errno));
	
	return  NULL;
}

int
IcmpdProcessPacket(char *buf, int n, struct sockaddr_in *from, int fromlen)
{
	struct ip			*ip, *hip;
	struct icmp *icmp;
	struct udphdr		*udp;
	int i, hlen1, hlen2, icmplen, sport;
	char				srcstr[INET_ADDRSTRLEN], dststr[INET_ADDRSTRLEN];

	ip = (struct ip *) buf;
	hlen1 = ip->ip_hl << 2;     /* length of IP header */

	icmp = (struct icmp *) (buf + hlen1);	/* start of ICMP header */
	if ( (icmplen = n - hlen1) < 8)
	{
		NETERROR(MICMPD, ("icmplen (%d) < 8", icmplen));
		return -1;
	}

	NETDEBUG(MICMPD, NETLOG_DEBUG4,
		(" type = %d, code = %d\n", icmp->icmp_type, icmp->icmp_code));

	if (icmp->icmp_type == ICMP_UNREACH ||
		icmp->icmp_type == ICMP_TIMXCEED ||
		icmp->icmp_type == ICMP_SOURCEQUENCH) {

		if (icmplen < 8 + 20 + 8)
		{
			NETERROR(MICMPD, ("icmplen (%d) < 8 + 20 + 8", icmplen));
			return -1;
		}

		hip = (struct ip *) (buf + hlen1 + 8);
		hlen2 = hip->ip_hl << 2;

		NETDEBUG(MICMPD, NETLOG_DEBUG4,
				("\tsrcip = %s, dstip = %s, proto = %d\n",
			   inet_ntop(AF_INET, &hip->ip_src, srcstr, sizeof(srcstr)),
			   inet_ntop(AF_INET, &hip->ip_dst, dststr, sizeof(dststr)),
			   hip->ip_p));

		// udp support only
 		if (hip->ip_p == IPPROTO_UDP) 
		{
			IcmpdCheckCallback(icmp, icmplen);
		}
	}

	return(0);
}

int
IcmpdCheckCallback(struct icmp *icmp, int icmplen)
{
	icmpcbdata *cbdata;
	struct ip			*hip;
	struct udphdr		*udp;
	int i, hlen1, hlen2, sport;

	if (!(cbdata = icmpcbs))
	{
		return 0;
	}
		
	hip = (struct ip *) (((char *)icmp) + 8);
	hlen2 = hip->ip_hl << 2;
	udp = (struct udphdr *) (((char *)icmp) + 8 + hlen2);
	sport = udp->uh_sport;

	do
	{
		if (cbdata->src.sin_port == sport)
		{
			cbdata->fn((char *)icmp, icmplen, cbdata->data);
			return 1;
		}

		cbdata = cbdata->next;

	} while (cbdata != icmpcbs);

	return 0;
}

// register one or more:
// src - src ip/port
// dst - dst ip/port
// protocol is picked up from one of the socket addresses
int
IcmpdRegisterCallback(uchar_t ip_p, struct sockaddr *src, struct sockaddr *dest,
						int (*fn)(char *, int, void *), void *arg)
{
	icmpcbdata *cbdata;

	cbdata = (icmpcbdata *)malloc(sizeof(icmpcbdata));	
	memset(cbdata, 0, sizeof(icmpcbdata));
	ListInitElem(cbdata);

	cbdata->ip_p = ip_p;
	if (src) memcpy(&cbdata->src, src, sizeof(struct sockaddr));
	if (dest) memcpy(&cbdata->dest, dest, sizeof(struct sockaddr));
	cbdata->fn = fn;
	cbdata->data = arg;

	if (icmpcbs)
	{
		ListInsert(icmpcbs, cbdata);
	}
	else
	{
		icmpcbs = cbdata;
	}

	return 1;
}

#if 0
int
IcmpdUnregisterCallback(uchar_t ip_p, 
	struct sockaddr *src, struct sockaddr *dest,
	int (*fn)(char *, int, void *), void *arg)
{
	icmpcbdata *cbdata;

	if (!(cbdata = icmpcbs))
	{
		return 0;
	}
		
	hip = (struct ip *) (((char *)icmp) + 8);
	hlen2 = hip->ip_hl << 2;
	udp = (struct udphdr *) (((char *)icmp) + 8 + hlen2);
	sport = udp->uh_sport;

	do
	{
		if (cbdata->src.sin_port == sport)
		{
			cbdata->fn((char *)icmp, icmplen, cbdata->data);
			return 1;
		}

		cbdata = cbdata->next;

	} while (cbdata != icmpcbs);

	return 1;
}
#endif
