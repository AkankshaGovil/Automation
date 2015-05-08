#include <sys/time.h>
#include "bits.h"
#include "ipc.h"

#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"

#include "gis.h"
#include "ifs.h"

#include "ssip.h"
#include <malloc.h>
#include "tsm.h"
#include "packets.h"

void * SipProcessIncomingPacket(void *threadArg);
int lSipPort = 5060;

int
SipDispatchIncomingPacket(
	int csock,
	char *pkt,
	int nbytes,
	SipEventContext *context
);

/*
 * SIP initialization
 */

int 	_sip_controlfd;	/* Used for UDP Connections */
int		_sip_sigfd;	/* Used for TCP Connections */

/* Global for SipOptions. This way we initialize only once! */
SipOptions sip_options;
SipOptions sip_contentless_options;

static int initialSipTraceLevel = 0;

int
_dummysipSetTraceLevel(int level)
{
	initialSipTraceLevel = level;
	return(0);
}

int
_localSipSetTraceLevel(int level)
{
	SipError siperror;
	int err;

	switch (level)
	{
	case 0:
	 	err = sip_setTraceLevel(SIP_None,&siperror);
		break;
	case 1:
	 	err = sip_setTraceLevel(SIP_Brief,&siperror);
		break;
	default:
	 	err = sip_setTraceLevel(SIP_Detailed,&siperror);
		break;
	}

	if (err == SipFail)
	{
		NETERROR(MSIP, ("Could not set debug level!!\n"));
	}

	return 0;
}

int
SSIPInit ()
{
	char fn[] = "SSIPInit():";
	int	retval = 0;
	SipError siperror;

	if (sipAdminStatus == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s SIP Disabled\n", fn));
		return 0;
	}

    NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Initializing Session Initiation Protocol v2\n", 
		 fn));

	sip_options.dOption = SIP_OPT_FULLFORM | SIP_OPT_CLEN | SIP_OPT_SINGLE;

	_sipSetTraceLevel = _localSipSetTraceLevel;

	/* First Initialize the SIP Stack */
	if ((sip_initStack()) == SipFail)
	{
		  NETERROR(MSIP, 
			   ("%s Sip Stack Initialization Failed!\n", fn));
		  return -1; ;
	}

	_localSipSetTraceLevel(initialSipTraceLevel);

	SIP_MAXRETRANS = 0;
	SIP_MAXINVRETRANS = 0;

	SipStackTimers = listInit();

	if (sip_setErrorLevel(SIP_Major|SIP_Minor|SIP_Critical, 
			&siperror)==SipFail)
	{
		  NETERROR(MSIP, 
			   ("%s ########## Error Disabled at compile time #####\n", fn));
	}

#ifdef _default_logging_
	if (sip_setTraceLevel(SIP_Detailed,&siperror)== SipFail)
	{
		  NETERROR(MSIP, 
			   ("%s ########## Trace Disabled at compile time #####\n", fn));
	}
#endif

	 if (sip_setTraceType(SIP_All,&siperror)== SipFail)
	 {
		  NETERROR(MSIP, 
		 	 ("%s ########## Trace Disabled at compile time #####\n", fn));
	 }

	 return retval;
}

int
SSIPInitCont (int sipport)
{
	char fn[] = "SSIPInitCont():";
	int	retval = 0;
	SipError siperror;

	lSipPort = sipport;

	if (SipInitNet (sipport) < 0)
	{
		  /* No point in proceeding */
		  return (-1);
	}


#ifdef _use_tcp_for_sip_
	 /* Add the signalling accept port also */
	 retval = NetFdsAdd(&lsnetfds, SipGetSigFd(), FD_READ, 
					(NetFn) sip_SigConnReceive, 
					(NetFn) 0, NULL, NULL);
#endif


	SipTsmIcmpInit();

	return retval;
}

void 
SipFreeRealmInfo (void *data)
{
	free (data);
}

static int
SipEnableRealm (CacheRealmEntry *realmEntry, int sipport)
{
	char msg[256];
        char buf[INET_ADDRSTRLEN];
	struct sockaddr_in wakeaddr;
	char fn[] = "SipEnableRealm()";
	struct in_addr in;
	CallRealmInfo *realmInfo;

        CacheTableInfo* info;

	if (realmEntry == NULL) return -1;

	in.s_addr = htonl (realmEntry->realm.rsa);
	realmInfo = (CallRealmInfo *) malloc (sizeof (CallRealmInfo));
	memset(realmInfo, 0, sizeof(CallRealmInfo));
	if (realmInfo == NULL) 
	{
		NETERROR (MSIP, ("%s: Malloc failure !", fn));
		return -1;
	}
	realmEntry->socketId = socket (AF_INET, SOCK_DGRAM, 0);
	if (realmEntry->socketId < 0) {
		sprintf (msg, "Socket failed for realm :%lu\n", realmEntry->realm.realmId);
		PERROR (msg);
		return -1;
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s: Enabling realm %s with rsa %s sockfd %d",
					fn, realmEntry->realm.realmName,
					inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN), realmEntry->socketId));
	realmInfo->realmId = realmEntry->realm.realmId;
	realmInfo->rsa = realmEntry->realm.rsa;
	realmInfo->sPoolId = realmEntry->realm.sigPoolId;
	realmInfo->mPoolId = realmEntry->realm.medPoolId;
	realmInfo->addrType = realmEntry->realm.addrType;
	realmInfo->interRealm_mr = realmEntry->realm.interRealm_mr;
	realmInfo->intraRealm_mr = realmEntry->realm.intraRealm_mr;

	if (realmEntry->realm.rsa == 0)
	{
		// Check to see if sipdomain is set
		if (sipdomain[0] != '\0')
		{
			realmInfo->sipdomain = strdup(sipdomain);
		}
	}

	GisSetUdpSockOpts (realmEntry->socketId);
	bzero ((char *)&wakeaddr, sizeof (struct sockaddr_in));
	wakeaddr.sin_family = AF_INET;
	wakeaddr.sin_addr.s_addr = htonl (realmEntry->realm.rsa);
	wakeaddr.sin_port = htons (sipport);
	if (bind (realmEntry->socketId, (struct sockaddr  *)&wakeaddr,
		  sizeof (wakeaddr)) < 0) 
	{
		sprintf (msg, "Bind failed for realm %lu", realmEntry->realm.realmId);
		PERROR (msg);
		return -1;
	}
	if (NetFdsAdd (&lsnetfds, realmEntry->socketId, FD_READ,
		      (NetFn)SipControlPacketReceive,
		      (NetFn) 0, (void *) realmInfo, 
		       SipFreeRealmInfo) < 0) 
	{
		NETERROR (MSIP, ("Failed to add to NetFds"));
		return -1;
	}

	return 0;
}

int
SipDisableRealm (CacheRealmEntry *realmEntry)
{
	char fn[] = "SipDisableRealm()";

	if (realmEntry ==  NULL)
		return -1;
	if (NetFdsRemove (&lsnetfds, realmEntry->socketId, FD_READ) < 0) 
	{
		NETERROR (MSIP, ("%s: Failed to remove from netfds",fn));
		return -1;
	}
	close (realmEntry->socketId);
	realmEntry->socketId = -1;
	return 0;
}

// If a Realm is admin disabled, but it exists in our listener
// set, take it out.
// If a realm does not exist in our listener set add it

int
SipRealmReconfig(u_long rsa)
{
	char fn[] = "SipRealmReconfig()";
	CacheRealmEntry *realmEntry;

	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);
	realmEntry = CacheGet(rsaCache, &rsa);
	if (realmEntry == NULL)
	{
		NETERROR (MSIP, ("%s: No realm entry found for rsa 0x%lx", fn, rsa));
		goto _return;
	}

	if(realmEntry->socketId < 0)
	{
		/* new realm added */
		if (realmEntry->realm.adminStatus == 1)
		{
			 if (SipEnableRealm (realmEntry, SIP_PORT) < 0) 
			 {
				 NETERROR (MSIP, ("%s: Failed to init SIP on realm %s", fn, realmEntry->realm.realmName));
			 }
		}
	}
	else
	{
		if (realmEntry->realm.adminStatus == 0)
		{
			if (SipDisableRealm(realmEntry) < 0)
			{
				 NETERROR (MSIP, ("%s: Failed to disable SIP on realm %s", fn, realmEntry->realm.realmName));
			}
		}
		else // flip the realm status for config changes
		{
			if (SipDisableRealm(realmEntry) < 0)
			{
				 NETERROR (MSIP, ("%s: Failed to disable SIP on realm %s", fn, realmEntry->realm.realmName));
			}
			if (SipEnableRealm (realmEntry, SIP_PORT) < 0) 
			{
				 NETERROR (MSIP, ("%s: Failed to init SIP on realm %s", fn, realmEntry->realm.realmName));
			}
		}
	}
_return:
	CacheReleaseLocks(realmCache);
	return 0;
}

/*
 * Creates and inits the control port  sockets.
 */
int 
SipInitNet (int sipport)
{
     int	retval = 0;
     struct sockaddr_in wakeaddr, sigaddr;
     int i =0;
     char msg[256];
     CacheRealmEntry *realmEntry;
     char fn[] = "SipInitNet()";

     /************************************************/
     /* Create and register a control socket for UDP */
     /************************************************/

     /* Walk the realm database and create sockets one per
      * realm 
      */
     CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);

     for (realmEntry = (CacheRealmEntry *)CacheGetFirst (realmCache);
	  realmEntry; 
	  realmEntry = (CacheRealmEntry *)CacheGetNext(realmCache,
						       &realmEntry->realm.realmId))
     {
		 if (realmEntry->realm.adminStatus ==1 )
		 {
			 if (SipEnableRealm (realmEntry, sipport) < 0) 
			 {
				 NETERROR (MSIP, ("%s: Failed to initialize realm %lu", fn, realmEntry->realm.realmId));
			 }
		 }
     }
     CacheReleaseLocks (realmCache);
	 
     return 0;
}


int
SipControlPacketReceive (int csock, FD_MODE rw, void *data)
{
	char fn[] = 		"SipControlPacketReceive():";
        char buf[INET_ADDRSTRLEN];
    static int			nbytes;
    char *				pkt = NULL;
	char * 				nextmsg = NULL;
    static int			remotelen;
	SIP_S8bit *			nextmesg = NULL;
	SipEventContext 	*context = NULL;
	SipOptions 			decopt;
	SipError 			siperror;
    static struct sockaddr_in 	remoteaddr;
    CallRealmInfo *realmInfo;

	decopt.dOption = 0;

    bzero (&remoteaddr, sizeof(struct sockaddr_in));
    remotelen = sizeof(struct sockaddr_in);

    DEBUG(MSIP, NETLOG_DEBUG4, 
		   ("%s Received a control packet on realm %lu\n", fn, ((CallRealmInfo*)data)->realmId));

	pkt = (char *)malloc(SIPMSG_LEN);

	if (pkt == NULL)
	{
		  NETERROR(MSIP, ("%s Malloc for control pkt. Failed!\n", fn));
		  return -1;
	}

	memset (pkt, 0, SIPMSG_LEN);

#if 1
	context = (SipEventContext *) malloc (sizeof(SipEventContext));
	if (context == NULL)
	{
		  NETERROR(MSIP, ("%s Malloc for SipEventContext failed!\n", fn));
		  SipCheckFree(pkt);
		  return -1;
	}

	memset(context, 0, sizeof(SipEventContext));
	context->pTranspAddr = (SipTranspAddr *)malloc(sizeof(SipTranspAddr));

	if (context->pTranspAddr == NULL)
	{
		  NETERROR(MSIP, ("Malloc for SipTranspAddr pkt. Failed!\n"));
		  SipCheckFree(context);
		  SipCheckFree(pkt);
		  return -1;
	}

	memset(context->pTranspAddr, 0, sizeof(SipTranspAddr));
	context->pTranspAddr->dSockFd = csock;
	context->pData = (SIP_Pvoid)RealmInfoDup(data, MEM_LOCAL);
#endif

	/* UDP Connection */
	nbytes = recvfrom(csock, pkt, SIPMSG_LEN, 0, 
					(struct sockaddr *)&remoteaddr, &remotelen);

#if 1
	context->pTranspAddr->dPort = ntohs(remoteaddr.sin_port);
	strcpy(context->pTranspAddr->dIpv4, inet_ntop( AF_INET, &remoteaddr.sin_addr, buf, INET_ADDRSTRLEN));

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s SIP Packet Received from %s:%d, %d bytes\n",
		fn,
		context->pTranspAddr->dIpv4, 
		context->pTranspAddr->dPort,
		nbytes));
#endif

	if (nbytes <= 0)
	{
		  PERROR("recv: ");
		  SipCheckFree (context->pData);
		  SipCheckFree(context->pTranspAddr);
		  SipCheckFree(context);
		  SipCheckFree(pkt);
		  return -1;
	}
	else if (*pkt == '\0')
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Empty SIP Packet Received from %s%d\n",
			fn, context->pTranspAddr->dIpv4, context->pTranspAddr->dPort));
		SipCheckFree (context->pData);
		SipCheckFree(context->pTranspAddr);
		SipCheckFree(context);
		SipCheckFree(pkt);
		return -1;
	}

	/* At this point we can start our thread.
	 * All resources allocated here will be freed by the
	 * the thread.
	 */
	SipDispatchIncomingPacket(csock, pkt, nbytes, context);
	
    return 0;
}

/*
 * function sip_sendToNetwork
 * Called by stack.
 */
SipBool 
sip_sendToNetwork(
	 SIP_S8bit *buffer, 
	 SIP_U32bit buflen,
	 SipTranspAddr *transpaddr, 
	 SIP_S8bit transptype,
	 SipError *err
) 
{
	char fn[] = "sip_sendToNetwork():";
	int sendto_num ;
	int tolen;
	struct sockaddr_in serv_addr;


	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s to %s:%d, %s, %d bytes, fd=%d\n", 
		fn,
		transpaddr->dIpv4,
		transpaddr->dPort,
		(transptype == SIP_TCP)?"TCP":"UDP",
		buflen,
		transpaddr->dSockFd));

	if (transptype == SIP_TCP) 
	{
		if ((write(transpaddr->dSockFd, buffer, buflen)) < 0)
		{
			perror("Sip_send_tcp failed:");
			return SipFail;
		}
	}
	else
	{   
  		memset((void*)&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(transpaddr -> dIpv4);
		serv_addr.sin_port = htons(transpaddr -> dPort);
		tolen = sizeof (serv_addr);

		sendto_num = sendto(transpaddr->dSockFd, buffer, buflen, 0, (struct sockaddr*) &serv_addr, tolen);
		if (sendto_num < buflen || sendto_num == -1) 
	    {
			NETERROR(MSIP, ("%s sendto failed for %p/%d r=%d, e=%d\n", 
				fn, transpaddr->dIpv4, transpaddr->dPort, 					    sendto_num, errno));
			return SipFail ;
    	}

	}

	return (SipSuccess);
}

#if 0
int
SipGetControlFd (void)
{
	 return (_sip_controlfd);
}
#endif

int
SipDispatchIncomingPacket(
	int csock,
	char *pkt,
	int nbytes,
	SipEventContext *context
)
{
	char fn[] = "SipDispatchIncomingPacket():";
	ReceiveArgs *arg= (ReceiveArgs *) malloc(sizeof(ReceiveArgs));
	pthread_attr_t  detached;
	
	/* pack the args */
	arg->csock = csock;
	arg->pkt = pkt;
	arg->nbytes = nbytes;
	arg->context = context;

	if (ThreadDispatch(poolid, hpcid, SipProcessIncomingPacket, arg, 1,
			PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		// Error in launching thread
		NETERROR(MSIP, 
			("%s Could not launch SipProcessIncomingPacket\n",
			fn));
	}

	return 0;
}

void *
SipProcessIncomingPacket(void *thread_arg)
{
	char fn[] = "SipProcessIncomingPacket():";
	ReceiveArgs *arg= (ReceiveArgs *)thread_arg;
	int csock = arg->csock;
	char *pkt = arg->pkt;
	int nbytes = arg->nbytes;
	SipEventContext *context = arg->context;
	SipEventContext *dup_context;
	SipError siperror;
	char *nextmsg = NULL;
	SipOptions 			decopt;

	if (pkt == NULL)
	{
		NETERROR(MSIP,
			("%s Null Packet present in args - malloc error\n", fn));
		goto _error;
	}

	ThreadSetPriority(pthread_self(), SCHED_RR, 50);

	if (context == NULL)
	{
		NETERROR(MSIP,
			("%s Null Context passed in args - malloc error\n", fn));
		goto _error;
	}

	decopt.dOption = SIP_OPT_BADMESSAGE;

	/* Message received from network. 
	 * Call SIP Core stack to decode the message now.
	 * The stack will callback upon decoding the message
	 */

	 do {
		DEBUG(MSIP, NETLOG_DEBUG4, 
			("%s Calling sip_decodeMessage for socket %d with %d bytes\n",
			fn, csock, nbytes));

		if((dup_context = (SipEventContext *) SipDupContext (context)))
		{
			if (sip_decodeMessage(pkt, &decopt, nbytes, 
									&nextmsg, dup_context, &siperror) == SipFail)
			{
				NETERROR(MSIP, 
							("%s SipDecodeMessage: Received Bad message - error %d!\n", 
									fn, siperror));
				NETERROR(MSIP, ("%s", pkt));
				NETERROR(MSIP, ("\n"));

				SipFreeContext(dup_context);
			}
		}

		SipCheckFree(pkt);
		pkt = nextmsg;

	 } while (pkt != SIP_NULL);

_return:
_error:
	SipFreeContext(context);
	SipCheckFree(arg);

	return NULL;
}

