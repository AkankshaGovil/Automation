#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include    <sys/un.h>
#include    <unistd.h>
#include	<netinet/in.h>
#include	<netinet/in_systm.h>
#include	<netinet/ip.h>
#include	<netinet/ip_icmp.h>

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

#include "nxioctl.h"

#include "gis.h"
#include "timer.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include "include/tsmtimer.h"
#include "include/tsmq.h"
#include <malloc.h>
#include "common.h"

int
SipOperateIcmpDestUnreach( SipTrans *siptranptr)
{
	char fn[]="SipOperateIcmpDestUnreach()";
	SipTranSMEntry *siptransm=NULL;
	int (*CSMCallFn)(void *) = NULL;
	int lockflag = 0;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering.\n",fn));
	
	if (siptranptr == NULL)
	{
		NETERROR(MSIP, ("%s siptranptr is NULL\n", fn));
		return 0;
	}

	if (siptranptr->key.type != SIPTRAN_UAC)
	{
		NETERROR(MSIP, ("%s Transaction type is UAS\n", fn));
		return 0;
	}

	if (!strcmp(siptranptr->key.method, "INVITE"))
	{
		siptranptr->event = SipInvite7Sent;
		siptranptr->error = tsmError_NO_RESPONSE;

		if ( siptranptr->currState < 0 ||
			 siptranptr->currState > SipInviteSM_ClientMaxStates )
		{
			NETERROR(	MSIP, 
						( "%s : Error current client Invite state out of range - %d\n",
						fn,
						siptranptr->currState ));
		}

		if ( siptranptr->event < 0 ||
			 siptranptr->event > SipInviteSM_ClientMaxEvents )
		{
			NETERROR(	MSIP, 
						( "%s : Error current client Invite event out of range - %d\n",
						fn,
						siptranptr->currState ));
		}

		siptransm = 
			&SipInviteSM_Client[siptranptr->currState][siptranptr->event];
		siptranptr->StateName = (char*) &SipInviteSM_ClientStates[siptranptr->currState][0];
		siptranptr->EventName = (char*) &SipInviteSM_ClientEvents[siptranptr->event][0];
	}
	else
	{
		NETERROR(MSIP, ("%s Transaction not INVITE\n", fn));
		return 0;
	}

	if(siptransm == NULL)
	{
		/* do not send anything to tsm */
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need for tsm to act \n", fn));
	}
	else
	{
		if( SipTranSMProcessor(siptranptr,siptransm) < 0)
		{
			goto _error;
		}
	}

	if(siptranptr->done)
	{
		goto _return;
	}

	CSMCallFn=siptranptr->CSMCallFn;
	siptranptr->CSMCallFn = NULL;

	/* call CSM */
	if(CSMCallFn != NULL)
	{
		if( CSMCallFn(siptranptr) < 0)
		{
			NETERROR(MSIP, ("%s CSMCallFn error\n", fn));
			goto _error;
		}
	}

 _return:

	return 0;
 _error:

	return -1;

}

int
SipTsmIcmpUnreachable(char *icmp, int icmplen, void *arg)
{
	char fn[] = "SipTsmIcmpUnreachable():";
	SipTrans *siptranptr=NULL, *start = NULL, *tmp = NULL;
	struct ip			*hip;
	struct udphdr		*udp;
	int i, hlen1, hlen2, dip; 
	short dport;
	TsmQEntry *qentry = NULL;
	SipTransDestKey key;

	hip = (struct ip *) (icmp + 8);
	hlen2 = hip->ip_hl << 2;
	udp = (struct udphdr *) (((char *)icmp) + 8 + hlen2);

	key.srcip = ntohl(hip->ip_src.s_addr);
	key.destip = dip = ntohl(hip->ip_dst.s_addr);
	key.destport = dport = ntohs(udp->uh_dport);

	CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);

	start = CacheGet(transDestCache, &key);
	if (start == NULL)
	{
		NETDEBUG(MICMPD, NETLOG_DEBUG4,
			("%s No Tsm entry waiting for %x/%d\n",
			fn, dip, dport));
		goto _return;
	}
	
	siptranptr = start;

	do
	{
		tmp = siptranptr->next;

		if ((SipTranRequestSendhost(siptranptr) != dip) ||
			(SipTranRequestSendport(siptranptr) != dport))
		{
			NETERROR(MICMPD, ("%s Found %lu/%d among %d/%d list\n",
				fn,
				SipTranRequestSendhost(siptranptr),
				SipTranRequestSendport(siptranptr),
				dip, dport));
			break;
		}

		LockGetLock(&siptranptr->lock, LOCK_WRITE, LOCK_BLOCK);

		if (siptranptr->done)
		{
			goto _continue;
		}

		if (siptranptr->inuse)
		{
			// Queue the message and return
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Queueing Message\n", fn));
			qentry = TsmNewQEntry();
			qentry->s = NULL;
			qentry->timer = NULL;
			qentry->from=3;
			qentry->context=NULL;

			listAddItem(siptranptr->msgs, qentry);
			goto _continue;
		}
		else
		{
			siptranptr->inuse = 1;

			LockReleaseLock(&siptranptr->lock); 

			// we have the entry
			// We cannot release the locks here, as we have the
			// whole list of entries to operate on, so we will
			// carry it on asynchronously
			SipTransOperateMarshall(3, siptranptr, NULL, NULL, 0, NULL, NULL);
		
			goto _continue_no_locks;
		}

	_continue:
		LockReleaseLock(&siptranptr->lock); 

	_continue_no_locks:
		siptranptr = tmp;

	} while (siptranptr != start);

_return:
	CacheReleaseLocks(transCache);

	return 0;
}

void
SipTsmIcmpInit()
{
	struct sockaddr_in addr;
	int len = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(lSipPort);
	
	// getsockname(realmEntry->socketId, (struct sockaddr *)&addr, &len);
	
	IcmpdRegisterCallback(0, (struct sockaddr *)&addr, 0, SipTsmIcmpUnreachable, NULL);
}
