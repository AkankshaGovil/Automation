/**************************************************************************
 * FILE:  age.c
 *
 * DATE:  MARCH 8th 1998
 *
 * Copyright (c) 1998 Netoids Inc.
 ***************************************************************************/ 	
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define LS_READ_SINGLESHOT	4096	/* Attempt to read 4k */

#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "ipcerror.h"
#include "serverdb.h"
#include "key.h"
#include "mem.h"
#include "entry.h"
#include "phone.h"
/* Server prototypes */
#include "protos.h"
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "pef.h"
#include "age.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"
#include "timer.h"
#include "connapi.h"
#include "sconfig.h"
#include "pmpoll.h"

#include "xmltags.h"

#include "gis.h"
#include "db.h"
#include <malloc.h>

#undef SERVER_DBG
#define SERVER_DBG

// H.323 not needed here anymore
#undef ALLOW_ISERVER_H323

/* Define SERVER_DBG to debug this mammoth... */
#ifndef SERVER_DBG
#define USE_FORK 
#define USE_DAEMONIZE
#endif

/* Local prototypes */
static int InitNetPort (void);
static int IpcInit (void);
int AgeReceivePacket(int fd, FD_MODE rw, void *data);
static int ProcessConfig(void);

char 		config_file[60] = CONFIG_FILENAME;
char 		pidfile[256];
NetFds 	 	agenetfds;
tid 		pmtid, cpolltid, rastid;
int			sleep_time = 0;

CacheEntry 	*updateList;

MemoryMap 	*map;
LsMemStruct *lsMem;
int 		shmId;
Config      localConfig;

/*
 * GLOBALS declared here.
 */
static int	_netfd;
static time_t lastCachePollTime, presentTime, remainingTime;
static int cacheTimeout;

int debug = 0;
static int idaemon = 1;

static void SignalInit(void);
static void* AsyncSigHandlerThread( void* args );
static void SyncSigHandler(int signo);

/* Pef Opaque structure passed to higher layer
 * application routines. Note that the header (similar to
 * PktHeader, must be the first field in all of the
 * opaque structures.
 */
typedef struct 
{
     PktHeader *hdr;
     SA *sa;
} PktOpaque;

int AgeCache(tid t);

int
AgeStartElem(void *userData, int tag, int depth, const char **atts)
{
	int rc = 0;
  	XMLCacheCb *cb = (XMLCacheCb *)userData;
	int i;

	switch (tag)
	{
	default:
		break;
	}

	return rc;
}

int
AgeEndElem(void *userData, int tag, int depth)
{
	int rc = 0;
  	XMLCacheCb *cb = (XMLCacheCb *)userData;

	switch (tag)
	{
	case TAG_UPORT:
		NETDEBUG(MAGE, NETLOG_DEBUG1, ("Recd. PKT_HEARTBEAT (XML)\n"));
		ProcessHeartbeatCb(cb);
		break;
	default:
		break;
	}

	return rc;
}

int
main (int argc, char * argv[])
{
	int		retval = 0;
	char	* penv = NULL;
	int		logfd;
	FILE	*fp;
	int		mypid;
    struct  itimerval polltmr;
    struct  itimerval cachetmr, rastmr;
	int 	uh323init = 0;
	int		fd;

    (void) freopen( "/dev/null", "r", stdin );
    (void) freopen( "/var/log/iserverout.log", "a", stdout );
    (void) freopen( "/var/log/iservererr.log", "a", stderr );
	fd = open("/dev/null/",O_RDWR);

	SignalInit();

	ifihead = initIfs();

	/* set the config file path */
	setConfigFile();

	/* Parse command line arguments */
	IpcParse (argc, argv);

	/* Initialize config struct */
	memset(&localConfig, 0, sizeof(Config));
	
	/* Initialize the fd list */
	NetFdsInit(&agenetfds);

#ifdef ALLOW_ISERVER_H323
	//uh323init = UH323AgeInit();
#endif

	myConfigServerType = CONFIG_SERPLEX_GIS;
	DoConfig(ProcessConfig);
	
	if (idaemon)
	{
		daemonize ();
	}

	sprintf(pidfile, "%s/%s", PIDS_DIRECTORY, GISAGE_PID_FILE);

	if (sleep_time != 0)
	{
		printf("pid=%d sleeping for %ds\n", getpid(), sleep_time);
		sleep(sleep_time);
	}

	mypid = ReadPid(pidfile);

	if ((mypid > 0) && (kill(mypid, 0) == 0))
	{
		NETERROR(MINIT, ("%s seems to be running already\n", argv[0]));
		exit(0);
	}

	StorePid(pidfile);

	/* Initialization */
	retval = IpcInit ();

	if (retval < 0)
	{
		syslog ( LOG_NOTICE, "Ipc: call to IpcInit failed" );
		/* No point proceeding further */
		exit (retval);
	}

	/* Init udp client and the keep alive message structure for poll */
	Initpoll(GISAGE_ID,SERPLEX_GID);

	if (SHM_Init(ISERVER_CACHE_INDEX) < 0)
	{
		NETERROR(MINIT, ("SHM_Init failed\n"));
		exit(-1);
	}

	/* Attach to shared memory */
	while (CacheAttach() < 0)
	{
		NETDEBUG(MINIT, NETLOG_DEBUG4,
			("CacheAttach failed\n"));
		Sendpoll(NULL);
		sleep(5);
	}

	NETDEBUG(MINIT, NETLOG_DEBUG4,
			("gisage CacheAttach success\n"));

	/* Initialize timer library */
	timerLibInit();
	timerInit(&localConfig.timerPrivate, 10, 0);
	
	InitXMLDecoder(AgeStartElem, AgeEndElem);

	/* Send immediate poll - since the timer is going to fire only after 15
	*  sec */
	Sendpoll(NULL);


	/* Add polltimer for keep alive */
	memset(&polltmr, 0, sizeof(struct itimerval));
	polltmr.it_interval.tv_sec = POLL_TIME_OUT;
	
	pmtid = timerAddToList(&localConfig.timerPrivate, &polltmr,
		0,PSOS_TIMER_REL, "KATimer", Sendpoll, NULL);
	
	/* Add cachetimer for Cache Poll*/
	memset(&cachetmr, 0, sizeof(struct itimerval));
	cachetmr.it_interval.tv_sec = CACHE_POLL_TIME;
	
#if 0
	cpolltid = timerAddToList(&localConfig.timerPrivate, &cachetmr,
						   0,PSOS_TIMER_REL, "CachePollTimer", 
						   AgeCache, NULL);
#endif

#ifdef ALLOW_ISERVER_H323
	//if (uh323init >= 0)
	//	UH323AgeInitCont();
#endif

	/* Main Loop */
	IpcMainLoop ();

	/* Should never get here */
	IpcTerminate ();

	fclose (fp);
	close (logfd);

	return 0;
}

static int
IpcInit (void)
{

	/* Open socket and listen for commands */
	if (InitNetPort () < 0)
	{
		fprintf (stderr, "Unable to initialize network port \n");
		return -1;
	}

#if 0
	NetFdsAdd(&agenetfds, _netfd, FD_READ, 
		(NetFn) AgeReceivePacket, (NetFn) NULL,
		NULL, NULL);
#endif
	return 0;
}


static int
InitNetPort (void)
{
	int	retval = 0, i = 1;
	struct sockaddr_in myaddr;
	struct hostent * hent = 0x0;

	/* Create Socket */
	_netfd = socket (AF_INET, SOCK_DGRAM, 0 );
	setsockopt(_netfd, SOL_SOCKET, SO_REUSEADDR,  
				(void *)&i, sizeof(i));

	if (_netfd < 0)
	{
		perror ("Unable to create stream socket ");
		return -1;
	}

	bzero ((char *)&myaddr, sizeof(myaddr));  /*  Zeroes the struct */

#ifdef needed
	hent = gethostent ();

	if (hent)
	{
		int	i = 0;

		while (hent->h_aliases[i] != 0x0)
		{
			fprintf (stderr, "Host: [%s] \n", 
					hent->h_aliases[i]);
			fprintf (stderr, "IP: [%d.%d.%d.%d] \n", 
					hent->h_addr_list[i][0],
					hent->h_addr_list[i][1], 
					hent->h_addr_list[i][2],
					hent->h_addr_list[i][3]);
			i++;
		}
	}
#endif
	myaddr.sin_family = AF_INET;
	myaddr.sin_port  = htons (BCS_PORT_NUMBER);
	myaddr.sin_addr.s_addr  = htonl (INADDR_ANY);

	/* Bind */
	retval = bind (_netfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	
	if ( retval < 0 )
	{
		perror ("Unable to bind socket");
		return -1;
	}

	return (retval);
}

int
IpcParse (int argc, char * argv[] )
{
	return 0;
}

int
IpcTerminate (void)
{
	if (_netfd)
		close (_netfd);

	return 0;
}

/* generic signal handler */
static void
sig_handler(int signo)
{
        switch (signo)
        {
        case SIGINT:
                sig_int(SIGINT);
                break;
        case SIGCHLD:
                sig_chld(SIGCHLD);
                break;
        case SIGHUP:
                sig_hup(SIGHUP);
                break;
        default:
                break;
        }
}

int
IpcMainLoop (void)
{
	int	retval = 0;
	pid_t	pid;
	sigset_t o_signal_mask, n_signal_mask, p_signal_mask;
	struct timeval tout;
	unsigned int msec  = (unsigned int)-1;
	int num, rc;

	/* Set the last cache poll time */
	lastCachePollTime = time(0);
	presentTime = time(0);

	/* Loop forever */
	for (; ;)
	{
		NetFdsZero(&agenetfds);

#ifdef ALLOW_ISERVER_H323
		rc = seliSelectEventsRegistration(
			 /* FD_SETSIZE */5000,
			 &num,
			 &agenetfds.readfds,
			 &agenetfds.writefds,
			 &agenetfds.exceptfds,
			 &msec);

		NETDEBUG(MSEL, NETLOG_DEBUG1, 
			  ("seliSelectEventsRegistration: msec is %d\n", msec));

		if (rc != 0)
		{
			NETDEBUG(MSEL, NETLOG_DEBUG1, 
				  ("seliSelectEventsRegistration returned error\n"));

			msec = (unsigned int)-1;
		}
#endif
 
		retval = setupTimeout(&localConfig.timerPrivate, &tout);
		
		if (retval < 0)
		{
			 serviceTimers(&localConfig.timerPrivate);
			 continue;
		}
		
		NetFdsSetup(&agenetfds, MSEL, NETLOG_DEBUG4);

		if ((retval == 0) && (msec == (unsigned int)-1))
		{
			 retval = select (500, &agenetfds.readfds, &agenetfds.writefds,
							  NULL, NULL);
		}
		else
		{
			 retval = select (500, &agenetfds.readfds, &agenetfds.writefds, 
							  NULL, &tout);
		}

		switch (retval)
		{
		case -1:
		  NETDEBUG(MSEL, NETLOG_DEBUG4, ("select %d", errno));
		  break;
		case 0:
#ifdef ALLOW_ISERVER_H323
			seliSelectEventsHandling(
				&agenetfds.readfds,
				&agenetfds.writefds, 
				&agenetfds.exceptfds, 
				num, retval );
#endif
		  serviceTimers(&localConfig.timerPrivate);
		  break;
		default:
#ifdef ALLOW_ISERVER_H323
		  seliSelectEventsHandling(
			   &agenetfds.readfds,
			   &agenetfds.writefds, 
			   &agenetfds.exceptfds, 
			   num, retval );
#endif	
		  NetFdsProcess(&agenetfds, MSEL, NETLOG_DEBUG4);
		  break;
		}
	}

	return 0;
}

int
AgeReceivePacket(int fd, FD_MODE rw, void *data)
{
	 struct sockaddr_in	client;
	 int	clilen;
	 char fn[] = "AgeReceivePacket():";

	 ProcessData (fd, &client);
	 return(0);
}
	
/********************************************************************
 * The following global variable must be made a parameter,
 * if this program is ever multithreaded, or can process
 * multiple messages simultaneously.
 *******************************************************************/
struct sockaddr_in remoteClient;

/* Just handle the encapsulations, and transfer control
 * to lower layer, if necessary. All packet replies
 * to be sent back, by this layer
 */
int
ProcessData (int sockfd, struct sockaddr_in *client)
{
	char fn[] = "ProcessData():";
	long 	buffer[LS_READ_SINGLESHOT/sizeof(long)+1];
	char	*start, *end;
	PktHeader *pkt_hdr;
	Pkt	*data_pkt, reply_pkt = { 0 };
	int	retval = 1;
	int     read_bytes = 0;
	int 	type, remotelen;
	sigset_t o_signal_mask, n_signal_mask;
	static struct sockaddr_in  remoteaddr;

	bzero (&reply_pkt, sizeof(Pkt));

	/* Read from ephemeral socket */
	start = end = (char *)&buffer[0];
	remotelen = sizeof(struct sockaddr_in);

	read_bytes = retval = get_next_packet_from_udp(sockfd, buffer,
				&start, &end,  
				LS_READ_SINGLESHOT,
				&remoteaddr, &remotelen);

	if (retval <= 0)
	{
	     /* We dont know now, where we are in the stream... */
	     retval = 1;	/* terminate... */
	     goto _do_nothing;
	}

	memcpy(&remoteClient, &remoteaddr, sizeof(struct sockaddr_in));

	NETDEBUG(MAGE, NETLOG_DEBUG1,
		("%s received packet from %s (%d bytes)\n", 
			fn, ULIPtostring(ntohl(remoteaddr.sin_addr.s_addr)),
			read_bytes));

	/* We have a valid PktHeader now,
	 * but we will typecast both quantities.
	 */
	
	pkt_hdr = (PktHeader *)buffer;
	data_pkt = (Pkt *)buffer;

	type = ntohl(pkt_hdr->type);

	ntohPkt(type, data_pkt);

	switch (type)
	{
	case PKT_PROFILE:

		/* This is our heartbeat packet... */
		NETDEBUG(MAGE, NETLOG_DEBUG4, ("Recd. PKT_PROFILE \n"));
		ProcessHeartbeat(sockfd, data_pkt, &reply_pkt,
			0, 0, 0);
		break;
	case PKT_HEARTBEAT:
	
		/* We got a heartbeat */
		NETDEBUG(MAGE, NETLOG_DEBUG4, ("Recd. PKT_HEARTBEAT\n"));
		ProcessHeartbeat(sockfd, data_pkt, &reply_pkt,
			0, 0, 0);
		break;
	case PKT_XML:
	
		/* We got an xml encoded packet */
		NETDEBUG(MAGE, NETLOG_DEBUG4, ("Recd PKT_XML\n"));

		ProcessXMLEncoded(sockfd, buffer, NULL,
			0, 0, 0);
		break;
	default:
	     fprintf (stderr, 
		      "Unhandled case [0x%x]-- not implemented yet \n", 
		      type);
	     /* Reflect this packet back with the type changed to error */
	     data_pkt->type = PKT_ERROR;

	     htonPkt(PKT_ERROR, data_pkt);
	     PktSend(sockfd, data_pkt);
	     
	     break;
	}
	
 _do_nothing:
	return (retval);
}

int
ProcessProfile (int sockfd, Pkt* data_pkt, Pkt * reply_pkt,
                 void *opaque, int opaquelen, /* Arguments to be passed
                                               * to the write call back.
                                               */
                 int (*writecb)())
{
     struct in_addr in;
     char buf[INET_ADDRSTRLEN];
     int h, m, s, dur, rdur, sdur;

     in.s_addr = htonl(data_pkt->credo.ipaddress.l);
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	("\n***************** CDR START (%s.%d) ******************\n", 
		inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN), (data_pkt->credo.uport)));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	("Local (%s)\n", 
	BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin": "Call Destination"));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Registration ID %s\n", data_pkt->data.profile.local.regid));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Port %d\n", (data_pkt->data.profile.local.uport)));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Phone %s\n", data_pkt->data.profile.local.phone));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o VpnPhone %s\n", data_pkt->data.profile.local.vpnPhone));

     NETDEBUG(MCDR, NETLOG_DEBUG1,
	("Remote (%s)\n",
	!BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin": "Call Destination"));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Registration ID %s\n", data_pkt->data.profile.remote.regid));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Port %d\n", (data_pkt->data.profile.remote.uport)));
     in.s_addr = htonl(data_pkt->data.profile.remote.ipaddress.l);
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Phone %s\n", data_pkt->data.profile.remote.phone));
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o VpnPhone %s\n", data_pkt->data.profile.remote.vpnPhone));

     NETDEBUG(MCDR, NETLOG_DEBUG1,
	("Duration\n"));
     dur = (data_pkt->data.profile.ftime) - 
		(data_pkt->data.profile.ctime);
     h = dur/3600; m = dur/60 - h*60; s = dur - h*3600 - m*60;

     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o Hrs %d Mts %d Secs %d\n", h, m, s));
     rdur = (data_pkt->data.profile.ftime) - 
		(data_pkt->data.profile.rltime);
     (rdur <= 0) ? (rdur = 0x100000) : 0;
     sdur = (data_pkt->data.profile.ftime) - 
		(data_pkt->data.profile.sltime);
     (sdur <= 0) ? (sdur = 0x100000) : 0;
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o sRate %d(%ds) rRate %d(%ds)\n", 
	(data_pkt->data.profile.sRate)/sdur, sdur,
	(data_pkt->data.profile.rRate)/rdur, rdur));

#ifdef needed
     NETDEBUG(MCDR, NETLOG_DEBUG1,
	(" o DSP Load %d\n", data_pkt->data.profile.dspLoad));
#endif

     NETDEBUG(MCDR, NETLOG_DEBUG1,
	("\n***************** CDR END ******************\n"));

     return 0;
}

int
ProcessHeartbeatCb(XMLCacheCb *cb)
{
	InfoEntry *entry = &cb->infoEntry;
	PhoNode phonode;

	NETDEBUG(MAGE, NETLOG_DEBUG4,
		("Heartbeat from %s %d has lus address 0x%x vpns address 0x%x\n",
			entry->regid,
			(entry->uport), 
			(cb->aloidIpAddress),
			(cb->vpnsIpAddress)));

	InitPhonodeFromInfoEntry(entry, &phonode);

	UpdateNetoidFromHeartbeat(0, &phonode, cb->aloidIpAddress, cb->vpnsIpAddress);

	/* Forward this heartbeat to the BCS */
	/* ForwardHeartbeat(0, cb->buf, cb->buflen, cb->aloidIpAddress, cb->vpnsIpAddress); */
	return(0);
}


int
ForwardHeartbeat (int sockfd, char *buf, int buflen, 
	unsigned long aloidIpAddress, unsigned long vpnsIpAddress)
{
	int i, nbytes;
	Pkt *data_pkt;

	if ((buf == NULL) || (buflen <= 0))
	{
		return -1;
	}

	data_pkt = (Pkt *)buf;

	htonPkt(data_pkt->type, data_pkt);

	/* Heartbeat has arrived */
	/* Forward it to the age daemons, if they are running
	 * somewhere in the system
	 */	
	for (i=0; i < max_servers; i++)
	{
		if ((serplexes[i].location.type != CONFIG_LOCATION_LOCAL) ||
			(serplexes[i].age.location.type != 
				CONFIG_LOCATION_LOCAL))
		{
			continue;
		}

		switch (serplexes[i].type)
		{
		case CONFIG_SERPLEX_BCS:
			if ((aloidIpAddress != 0) || (vpnsIpAddress != 0))
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Delivering heartbeat to 0x%x:%d",
				ntohl(serplexes[i].location.address.sin_addr.s_addr), 
				ntohs(serplexes[i].location.address.sin_port)));
	
				if ((nbytes = sendto(_netfd, buf, buflen, 0, 
					(struct sockaddr *)&serplexes[i].location.address,
					sizeof(struct sockaddr_in))) < 0)
				{
					NETERROR(MAGE, ("Server %d sendto\n", i));
					perror("sendtobcs");
				}
			}
			else
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Heartbeat does not have lus/vpns address\n"));
			}

			break;
		default:
			break;
		}
	}

	return 1;
}

/* Heartbeat processing 
 *
 * Check the heartbeat against the cache.
 * Use the heartbeat to refresh the cache, if the entry
 * is in the cache. right now we wont check the validity.
 * The primary ls will inform us, if there is any error.
 * If the entry is not in cache, bring it in cache, if
 * it matches the database entry, otherwise dont bring it.
 */

int
ProcessHeartbeat (int sockfd, Pkt* data_pkt, Pkt * reply_pkt,
                 void *opaque, int opaquelen, /* Arguments to be passed
                                               * to the write call back.
                                               */
                 int (*writecb)())
{
	char *sphone = 0;
	unsigned long pkey;
	CacheTableInfo *cacheInfo;
	CacheTableEntry *cacheHandle;
	NetoidInfoEntry *netInfo, tmpInfo;
	int isGateway = 0, isProxy = 0;

	UpdateNetoidFromHeartbeat(0, &data_pkt->data.profile.local, 
		data_pkt->data.profile.aloidIpAddress, data_pkt->data.profile.vpnsIpAddress);

	return 1;
}

int
UpdateNetoidFromHeartbeat (int sockfd, PhoNode *phonodep, 
	unsigned long aloidIpAddress,
	unsigned long vpnsIpAddress)
{
	 char fn[] = "UpdateNetoidFromHeartbeat():";
	 char *sphone = 0;
	 CacheTableInfo *cacheInfo, *cacheInfoMain;
	 NetoidInfoEntry *netInfo, tmpInfo;
	 RealmIP    realmip;
	 int writedb = 0;

	 CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	 /* Heartbeat has arrived */
	 cacheInfo = (CacheTableInfo *)CacheGet(regCache, phonodep);

	 if (cacheInfo == NULL)
	 {
		  NETERROR(MAGE, ("%s No cache entry present for %s/%d\n", 
			fn, phonodep->regid, phonodep->uport));
		  goto _release_locks;
	 }

	 NETDEBUG(MAGE, NETLOG_DEBUG4, 
		("from %s %d\n", phonodep->regid, (phonodep->uport)));

	 DEBUG(MAGE,NETLOG_DEBUG4,
		   ("s_addr = %x Phonode ipaddr(network byte order) = %x flags = %x \n",
			remoteClient.sin_addr.s_addr,htonl(phonodep->ipaddress.l),
			cacheInfo->data.stateFlags));

	 if ( cacheInfo && 
		  (cacheInfo->data.stateFlags & CL_ACTIVE) &&
		  (cacheInfo->data.stateFlags & CL_REGISTERED) 
		  && (remoteClient.sin_addr.s_addr ==
			  htonl(phonodep->ipaddress.l)) 
		  )
	 {
		  /* Update the refresh time */
		  netInfo = &cacheInfo->data;	
		  
		  netInfo->rTime = (time(0));
		  
		  NETDEBUG(MAGE, NETLOG_DEBUG4, 
				("%s Entry refreshed at rTime %d", fn, (netInfo->rTime)));

		  writedb = 1;
	 }
	 else 
	 {
		  if (cacheInfo &&
			  (cacheInfo->data.stateFlags & CL_ACTIVE))
		  {
			   /* Update the refresh time */
			   netInfo = &cacheInfo->data;	

			   netInfo->rTime = (time(0));

			   NETDEBUG(MAGE, NETLOG_DEBUG4, 
						("%s Entry refreshed at rTime %d",
						 fn, (netInfo->rTime)));
		  	   writedb = 1;
		  }

		  NETDEBUG(MAGE, NETLOG_DEBUG4,
				   ("%s Endpoint aged / not registered / behind firewall\n", fn));

		  /* Send an error message back indicating that the client
		   * has been aged.
		   */
		  if (matchIf(ifihead, 
					  htonl(aloidIpAddress)) != NULL)
		  {
			   /* We are the primary server */
			   /* Depending on db verification, send
				* an ERROR or REGISTER back.
				*/	
			   SrvrReportErrorToPhoNode(XERRT_SERPLEX,
										XERRS_REGISTER,
										(aloidIpAddress),
										phonodep);	
		  }
		  else if (matchIf(ifihead, 
						   htonl(vpnsIpAddress)) != NULL)
		  {
			   /* We are the primary server */
			   /* Depending on db verification, send
				* an ERROR or REGISTER back.
				*/	
			   SrvrReportErrorToPhoNode(XERRT_SERPLEX,
										XERRS_REGISTER,
										(vpnsIpAddress),
										phonodep);
		  }
		  else
		  {
			   NETERROR(MAGE,
						("Could Not match incoming interface with address 0x%x or 0x%x\n",
						 vpnsIpAddress,
						 aloidIpAddress));
		  }

	 }

	 if (writedb && cacheInfo)
	 {
		//UpdateNetoidDatabase(&cacheInfo->data);
		DbScheduleIedgeUpdate(&cacheInfo->data);
	 }

	 /* Update the IP address cache */
	 realmip.ipaddress = phonodep->ipaddress.l;
	 realmip.realmId = phonodep->realmId;
	 if (cacheInfo = (CacheTableInfo *)CacheGet(ipCache, &realmip))
	 {
		  if (cacheInfo->data.stateFlags & CL_ACTIVE)
		  {
			   /* Update the refresh time */
			   netInfo = &cacheInfo->data;	

			   netInfo->rTime = (time(0));
		  }
	 }

 _release_locks:
	 CacheReleaseLocks(regCache);

	 return 0;
}

int
RedundsUpdateAging(InfoEntry *entry)
{
	char buffer[2048];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("RedundsUpdateAging: Informing Redundant server of aged iedge"));

	BITA_SET(tags, TAG_REGID);
	BITA_SET(tags, TAG_UPORT);
	BITA_SET(tags, TAG_PHONE);
	BITA_SET(tags, TAG_VPNPHONE);
	BITA_SET(tags, TAG_PROXYS);
	BITA_SET(tags, TAG_PROXYC);
	BITA_SET(tags, TAG_RTIME);
	BITA_SET(tags, TAG_AGED);

	len += sprintf(buffer, "<CU>");
	len += XMLEncodeInfoEntryUpdate(entry, buffer+len, 2048-len, tags);
	len += sprintf(buffer+len, "</CU>");

	SendXML(lsMem->agefd, buffer, len);
	return(0);
}

int
HandleAgedIedgeIpAddr(NetoidInfoEntry *infoEntry)
{
	 int isproxyc = 0, i;
	 PhoNode phonode;
	 CacheTableInfo *info = 0;
	 InfoEntry tmpInfo;
	DB db;
	DB_tDb dbstruct;

	 /* This aged ip address represents a bunch of 
	  * iedge ports, which we must now disable.
	  * Using the ports mentioned in this iedge, 
	  * traverse the regCache, and disable the ports
	  */
	 memcpy(&phonode, infoEntry, REG_ID_LEN);

	dbstruct.read_write = GDBM_WRCREAT;
	 
	if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
	{ 
		  NETERROR(MCACHE, 
				("Unable to open database %s\n", 
				 NETOIDS_DB_FILE));
		  return 0;
	}

	 CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	 for (i=0; i<MAX_IEDGE_PORTS; i++)
	 {
		  isproxyc = 0;

		  if (BITA_TEST(infoEntry->ports, i))
		  {
			   /* Look for this iedge port */
			   phonode.uport = i;

			   info = (CacheTableInfo *)CacheGet(regCache, &phonode);

			   if (info)
			   {
					/* This iedge port is to be aged now */

					/* We must mark in the db that this
					 * netoid is inactive now
					 */
					isproxyc = info->data.stateFlags & CL_PROXYING;
					
					info->data.stateFlags &= ~CL_ACTIVE;

					/* If its not static */
					if (!(info->data.stateFlags & CL_STATIC))
					{
						info->data.stateFlags &= ~CL_PROXYING;
					}
					else
					{
						isproxyc = 0;
					}

#if 0
					memcpy(&tmpInfo, &info->data, sizeof(tmpInfo));

					if (isproxyc)
					{
						UnregisterProxy(&tmpInfo);
					}
#endif

	 				/* Update the database also */
	 				DbStoreInfoEntry(db, &info->data, 
					  (char *)&info->data, sizeof(NetoidSNKey));

					if (!(info->data.stateFlags & CL_STATIC))
					{
						DeleteIedgeIpAddr(ipCache, &info->data);
					}

#ifdef _ALLOW_REDUNDANCY_
					/* Inform Server */
					RedundsUpdateAging(&tmpInfo);
#endif
			   }
		  }
	 }			   
	 
	 CacheReleaseLocks(regCache);
     DbClose(&dbstruct);
     return(1);

}

int
AgeCache(tid t)
{
	 char fn[] = "AgeCache():";
	 int i;
	 CacheTableInfo *info = 0, cacheInfoEntry, nextCacheInfoEntry;
	 CacheVpnEntry *cacheVpnEntry;
	 CacheVpnGEntry *cacheVpnGEntry;
	 CacheVpnRouteEntry *cacheRouteEntry;
	 PhoNode phonode;
	 int isproxyc = 0, present = 0, nextPresent = 0;

	 NETDEBUG(MAGE, NETLOG_DEBUG4, ("%s Examining Iedg Cache", fn));

	 presentTime = time(0);

	 info = &cacheInfoEntry;
	 present = CacheFindFirst(ipCache, info, sizeof(CacheTableInfo));

	 while (present > 0)
	 {
		  /* Compute the next one, as this entry might get deleted */
		  nextPresent = CacheFindNext(ipCache, &info->data, &nextCacheInfoEntry, 
								  sizeof(CacheTableInfo));

		  if (info->iserver_addr != 0)
		  {
			   NETDEBUG(MAGE, NETLOG_DEBUG4,
						("Found an iedge not owned by me %s %d",
						 info->data.regid, info->data.uport));
			   goto _next;
		  }

		  if ((info->data.stateFlags & CL_ACTIVE) &&
			  IsAged((info->data.rTime), presentTime))
		  {
			   NETDEBUG(MAGE, NETLOG_DEBUG4,
						("%s This Endpoint is Aged Now \n", fn));

			   NETDEBUG(MAGE, NETLOG_DEBUG4,
						("rTime = %d, presentTime = %d, diff = %d",
						 (info->data.rTime),
						 presentTime, 
						 presentTime-(info->data.rTime)));
	
			   /* Delete this Netoid */
			   DEBUG_PrintInfoEntry(MAGE, NETLOG_DEBUG4,
									&info->data);

			   HandleAgedIedgeIpAddr(&info->data);
			   
			   InitPhonodeFromInfoEntry(&info->data, &phonode);
					
		  	   if (!(info->data.stateFlags & CL_STATIC) && 
					BIT_TEST(info->data.cap, CAP_UCC))
			   {
					SrvrReportErrorToPhoNode(XERRT_SERPLEX,
											 XERRS_REGISTER,
											 0,
											 &phonode);	
			   }

		  	   if (!(info->data.stateFlags & CL_STATIC) && 
					BIT_TEST(info->data.cap, CAP_IRQ))
			   {
#ifdef ALLOW_ISERVER_H323
					/* Send URQ */
					GkSendURQ(&info->data);
#endif
			   }

			   goto _next;
		  }
#ifdef INITIATE_IRQS
#ifdef ALLOW_ISERVER_H323
		  /* Check to see if we have to send an IRQ to this
		   * client
		   */
		  if (!(info->data.stateFlags & CL_STATIC) &&
			  (info->data.stateFlags & CL_ACTIVE) &&
			  BIT_TEST(info->data.cap, CAP_IRQ))
		  {
			   DEBUG(MH323, NETLOG_DEBUG4,
					 ("%s Sending IRQ to %s/%d\n",
					  fn, info->data.regid,
					  info->data.uport));
			  
			   GkSendIRQ(&info->data);

			   goto _next;
		  }
#endif
#endif
	 _next:
		  present = nextPresent;
		  info = &nextCacheInfoEntry;
	 }

	CacheFreeIterator(ipCache);

#if 0
	 /* Now examine the updates cache */
	 ExamineUpdateCache(lsMem, lsMem->agefd);

	 /* Examine the call cache */
	 ExamineCallCache(lsMem, lsMem->agefd);
#endif
	return(0);
}

static int
ProcessConfig(void)
{
	int match = -1;

	/* Process Configuration, read from the config file */
	match = FindServerConfig();

	if (match == -1)
	{
		fprintf(stderr, "Not Configured to run...\n");
		exit(0);
	}

	if (serplexes[match].age.location.type == CONFIG_LOCATION_NONE)
	{
		fprintf(stderr, "Not Configured to run...\n");
		exit(0);
	}

	/* Read the timeout */
	cacheTimeout = serplexes[match].age.cache_timeout;	
	idaemon = serplexes[match].daemonize;
	ServerSetLogging("gisage", &serplexes[match]);
	sleep_time = serplexes[match].age.sleep_time;

	/* Set the priority */
	setpriority(PRIO_PROCESS, 0, serplexes[match].age.prio);
	return(0);
}
	
int
SrvrReportErrorToPhoNode(int errtype, int errcode, unsigned long myip,
				PhoNode *phonodep)
{
	char fn[] = "SrvrReportErrorToPhoNode():";
	Pkt pkt;
	struct sockaddr_in addr;

	if (!BIT_TEST(phonodep->sflags, ISSET_IPADDRESS))
	{
		return 0;
	}

	pkt.type = PKT_ERROR;
	pkt.totalLen = (sizeof(Pkt));
	
	pkt.data.errormsg.node.type = (NODE_PHONODE);
	memcpy(&pkt.data.errormsg.node.unode.phonode,
		phonodep, sizeof(PhoNode));

	pkt.data.errormsg.snode.type = (NODE_SRVR);

	pkt.data.errormsg.snode.unode.srvr.ipaddress = myip;
	pkt.data.errormsg.snode.unode.srvr.cport = (ALOID_LOOKUP_PORT_NUMBER);

	pkt.data.errormsg.errortype = (errtype);
	pkt.data.errormsg.errornumber = (errcode);

	switch (errtype)
	{
	case XERRT_SERPLEX:
		strcpy(pkt.data.errormsg.msg, XErrorSStrings[errcode]);
		break;
	default:
		break;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;

	/* addr.sin_addr.s_addr = htonl(phonodep->ipaddress.l); */

	/* Send the error packet to the right remote address, which
	 * may be the firewall address.
	 */
#ifdef _no_bcs_
	addr.sin_addr.s_addr = remoteClient.sin_addr.s_addr;
#endif
	addr.sin_addr.s_addr = htonl(phonodep->ipaddress.l); 
	addr.sin_port = htons(CONTROL_PORT);
	
	htonPkt(PKT_ERROR, &pkt);

	/* Send this to the phonode as UDP */
	if (sendto(_netfd, (char *)&pkt, sizeof(Pkt), 0, 
			(struct sockaddr *)&addr,
			sizeof(struct sockaddr_in)) < 0)
	{
		NETERROR(MAGE, ("%s for 0x%x", fn,
			ntohl(addr.sin_addr.s_addr)));
	}
	else
	{
		NETDEBUG(MAGE, NETLOG_DEBUG4,
			("%s Error Packet Sent to 0x%x\n",
				fn, ntohl(addr.sin_addr.s_addr)));
	}
	return(1);
}

int
ExamineCallCache(LsMemStruct *lsMem, int fd)
{
	char fn[] = "ExamineCallCache():";
	int present = 0, nextPresent = 0;
	CallHandle *callHandle, callHandleEntry = { 0 }, nextCallHandleEntry;
	
	if (sharecall == 0)
	{
		return 0;
	}

	NETDEBUG(MAGE, NETLOG_DEBUG4, ("%s Examining Call Cache\n", fn));

	/* Walk over the Call Cache, send IRQ's for call */

	callHandle = &callHandleEntry;
	present = CacheFindFirst(callCache, callHandle, sizeof(CallHandle));
	
	while (present > 0)
	{
		 /* Compute the next one, as this entry might get deleted */
		 nextPresent = CacheFindNext(callCache, callHandle->callID, 
									&nextCallHandleEntry, 
									sizeof(CallHandle));

		 if ((nocallstate == 1) &&
			(H323hsCall(callHandle) == NULL) &&
			(H323inhsRas(callHandle) == NULL) &&
			(H323outhsRas(callHandle) == NULL)) 
		 {
			  /* Delete this entry */
			  CacheRemove(confCache, callHandle->confID);
			  CacheRemove(callCache, callHandle->callID);
		 }
		 else if (nocallstate == 1)
		 {
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Call no %d, inhsRas %x outhsRas %x hsCall %x\n",
				fn,
				callHandle->callNo,
				(uint32_t) H323inhsRas(callHandle), 
				(uint32_t) H323outhsRas(callHandle),
				(uint32_t) H323hsCall(callHandle)) );
		 }
		 else
		 {
#ifdef INITIATE_IRQS
			  /* Send IRQ for Call */
			  GkSendCallIRQ(callHandle);
#endif
		 }

		 goto _next;
	_next:
		 present = nextPresent;
		 callHandle = &nextCallHandleEntry;
	}

	CacheFreeIterator(callCache);
	return(1);
}

sigset_t 			async_signal_mask;
struct sigaction	sigact;
stack_t				s;

//
//	Function :
//		SignalInit()
//
//	Description :
//		Setup process wide signal handling, adhered to
//		by all threads via sigaction(2). This routine
//		should be called very early in the startup
//		process after daemonizing and prior to starting
//		any threads. In this way the blocked signal mask
//		will be inheritted by any threads.
//
static void
SignalInit(void)
{

	// Setup up alternate signal stack to be 
	// used by syncronous signal handler

	s.ss_sp = malloc( SIGSTKSZ );
	s.ss_size = SIGSTKSZ;
	s.ss_flags = 0;

	if ( sigaltstack( &s, 0 ) == -1 )
		perror( "sigaltstack" );

	// Create mask for blocking asyncronous signals
	// They must be blocked for sigwait() to work!!

	sigemptyset( &async_signal_mask );

	sigaddset( &async_signal_mask, SIGTERM );
	sigaddset( &async_signal_mask, SIGPOLL );
	sigaddset( &async_signal_mask, SIGCHLD );
	sigaddset( &async_signal_mask, SIGINT );
	sigaddset( &async_signal_mask, SIGHUP );
	sigaddset( &async_signal_mask, SIGALRM );
	
	// Setup async signals to be ignored

	sigemptyset( &sigact.sa_mask );
	sigact.sa_flags |= (SA_RESTART|SA_ONSTACK);

	sigact.sa_handler = SIG_IGN;

	sigaction( SIGPIPE, &sigact, NULL );
	sigaction( SIGWINCH, &sigact, NULL );
	sigaction( SIGTTIN,	&sigact, NULL );
	sigaction( SIGTTOU, &sigact, NULL );

	// Setup Signal handler to be called for
	// syncronous signals (fatal traps)

	sigact.sa_handler = SyncSigHandler;
	sigact.sa_flags |= (SA_RESTART|SA_ONSTACK);

	sigaction(SIGBUS,	&sigact, NULL );
	sigaction(SIGEMT,	&sigact, NULL );
	sigaction(SIGFPE,	&sigact, NULL );
	sigaction(SIGILL,	&sigact, NULL );
	sigaction(SIGSEGV,	&sigact, NULL );

	// Setup block mask for asyncronous signals to be
	// processed

	pthread_sigmask( SIG_BLOCK, &async_signal_mask, NULL );

	// Launch thread to process asyncronous signals

	ThreadLaunch( AsyncSigHandlerThread, NULL, 1);
}


//
//	Function :
//		AsyncSigHandlerThread()
//
//	Description :
//		This routine is contains logic for a thread
//		to handle asyncronous signals in a syncronous
//		fashion for the gis process. The thread is 
//		spawned at initialization time. sigwait(2)
//		is used to process the signals correctly.
//		The routine is not a signal handler so any
//		function call may be called from it.
//
static void *
AsyncSigHandlerThread( void * args )
{
	char	c_sig[256];
	int		signo;

	memset( c_sig, (int) 0, 256 );

	for (;;)
	{

		sigwait( &async_signal_mask, &signo );

		sig2str( signo, c_sig );

		switch (signo)
		{
		case SIGINT:
			sig_int(signo);
			break;

		case SIGTERM:
			sig_int(SIGINT);
			break;

		case SIGCHLD:
			sig_chld(signo);
			break;

		case SIGHUP:
			sig_hup(signo);
			break;

		default:
			NETERROR(	MDEF,
						("Caught %s signal - ignoring\n",
						c_sig ));
			break;
		}
	}
}

//
//	Function :
//		SyncSigHandler()
//
//	Description :
//		This routine is a signal catcher used for
//		Synchronous signals. (fatal traps)
//
static void
SyncSigHandler(int signo)
{
	int32_t thread = _lwp_self();
	char	c_sig[32];
	int		pid;

	sig2str( signo, c_sig );

	switch (signo)
	{
	case SIGBUS:
	case SIGEMT:
	case SIGFPE:
	case SIGILL:
	case SIGSEGV:

	#if 0
		if ((pid = fork()) == 0)
		{
			NETERROR(	MDEF,
						("Caught fatal %s signal in LWP %d\n",
						c_sig,
						thread ));
			exit(0);
		}
		else
	#endif
			abort();
		break;
	default:
		NETERROR(	MDEF,
					("Caught %s signal in LWP %d\n",
					c_sig,
					thread ));
		break;
	}
}
