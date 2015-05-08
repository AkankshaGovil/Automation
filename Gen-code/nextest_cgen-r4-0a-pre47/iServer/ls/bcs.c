/**************************************************************************
 * FILE:  ls.c
 *
 * DATE:  MARCH 8th 1998
 *
 * Copyright (c) 1998 Netoids Inc.
 ***************************************************************************/ 	
static char const rcsid[] = "$Id: bcs.c,v 1.34.2.1 2004/07/15 02:08:49 sshetty Exp $";

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
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>

#include <signal.h>
#include <sys/wait.h>

#define LS_READ_SINGLESHOT	4096	/* Attempt to read 4k */

#define SERVER_DBG
#undef SERVER_DBG

/* Define SERVER_DBG to debug this mammoth... */
#ifndef SERVER_DBG
#define USE_FORK 
#define USE_DAEMONIZE
#endif

#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "serverdb.h"
/* Server prototypes */
#include "protos.h"
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "pef.h"
#include "lsconfig.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "xmltags.h"
#include "timer.h"
#include "poll.h"

#include "spversion.h"

/* Local prototypes */
static int InitNetPort (void);
static int IpcInit (void);
static int ProcessConfig(void);

/* Default value for config_file */
char config_file[60] = CONFIG_FILENAME;
char 	pidfile[256];

/*
 * GLOBALS declared here.
 */
static int	_netfd;
TimerPrivate    	timerPrivate;

int debug = 0;

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

int
BcsStartElem(void *userData, int tag, int depth, const char **atts)
{
	int rc = 0;
  	XMLCacheCb *cb = (XMLCacheCb *)userData;
	int i;

	switch (tag)
	{
	case TAG_CDR:
		NETDEBUG(MAGE, NETLOG_DEBUG1, ("CDR start\n"));
		ProcessCdrLocal(cb);
		break;
	default:
		break;
	}

	return rc;
}

int
BcsEndElem(void *userData, int tag, int depth)
{
	int rc = 0;
  	XMLCacheCb *cb = (XMLCacheCb *)userData;

	switch (tag)
	{
	case TAG_CDR:
		NETDEBUG(MAGE, NETLOG_DEBUG1, ("CDR end\n"));
		ProcessCdrRemote(cb);
		break;
	case TAG_HRTBT:
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
	int	retval = 0;
	char	* penv;
	int	logfd;
	FILE	*fp;
	int	mypid;
        struct itimerval 	polltmr;
        struct timeval  	tout;




	ifihead = initIfs();

	/* 
		Set the config file path if SERPLEXPATH is defined. 
		This can also be modified with the -f switch given in 
		the command line. This is tested in IpcParse.
	*/
	setConfigFile();
	myConfigServerType = CONFIG_SERPLEX_BCS;
	
	/* Parse command line arguments */
	IpcParse (argc, argv);

	DoConfig(ProcessConfig);

	penv = (char *)getenv ("ALOID_DEBUG");

	if ((penv && (atoi(penv) == 1)) || (debug > 0))
	{
	}
	else
	{
#ifdef USE_DAEMONIZE
		daemonize ();
#endif
	}

	sprintf(pidfile, "%s/%s", PIDS_DIRECTORY, BCS_PID_FILE);

	mypid = ReadPid(pidfile);

	if ((mypid > 0) && (kill(mypid, 0) == 0))
	{
		ERROR(MINIT, ("%s seems to be running already\n", argv[0]));
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

	InitXMLDecoder(BcsStartElem, BcsEndElem);

        /* Init udp client and the keep alive message structure for poll */
        Initpoll(BCS_ID,SERPLEX_GID);

        memset(&timerPrivate,0,sizeof(TimerPrivate));
        timerLibInit();
        timerInit(&timerPrivate, 10, 0);

        memset(&polltmr, 0, sizeof(struct itimerval));
        polltmr.it_interval.tv_sec = POLL_TIME_OUT;

        timerAddToList(&timerPrivate, &polltmr,
                0,PSOS_TIMER_REL, "KATimer",Sendpoll, NULL);



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

	printf ("%s %s.%s, %s\n%s\n",
                BCS_NAME,
                BCS_VERSION,
                BCS_MINOR,     
                BCS_BUILDDATE,
                BCS_COPYRIGHT);

	printf("\n");
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
	myaddr.sin_port  = htons (ALOID_AGING_PORT_NUMBER);
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
	int	i = argc;
	char *progname = argv[0];

	argv ++;
	argc --;

	while (argc > 0)
	{
		if (argv[0][0] == '-')
		{
			switch (argv[0][1])
			{
			case 'v':
				printf ("\n");
				printf ("%s %s.%s, %s\n%s\n",
					BCS_NAME,
					BCS_VERSION,
					BCS_MINOR,
					BCS_BUILDDATE,
					BCS_COPYRIGHT);
				printf ("\n");

				/* NOTE that this exits */
				exit (0);

				break;
			
			case 'f':
				strcpy(config_file, argv[1]);
		
				argc --;
				argv ++;
				
				break;
			}
		}

		argc --;
		argv ++;
	}

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

static int
IpcMainLoop (void)
{
	struct sockaddr_in	client;
	int	clilen;
	int	newsockfd;
	int	retval = 0;
	pid_t	pid;
	fd_set	_ReadFD;
	fd_set 	_WriteFD;
	sigset_t o_signal_mask, n_signal_mask, p_signal_mask;
	struct timeval tout;

	/* Signals we want to handle */
	Signal(SIGINT, sig_int);
	Signal(SIGHUP, sig_hup);

	/* Signals we want to ignore */
	Signal(SIGCHLD, SIG_IGN);
        Signal(SIGPIPE, SIG_IGN);
        Signal(SIGWINCH, SIG_IGN);
        Signal(SIGTTIN, SIG_IGN);
        Signal(SIGTTOU, SIG_IGN);

	BlockCommonSignals(&o_signal_mask);

	/* Loop forever */
	for (; ;)
	{
		clilen = sizeof(client);
		bzero (&client, sizeof(client));


		retval =  setupTimeout(&timerPrivate,&tout);

                if (retval < 0)
                {
                        serviceTimers(&timerPrivate);
                        continue;
                }

		FD_ZERO (&_ReadFD);
		FD_ZERO (&_WriteFD);
		FD_SET (_netfd, &_ReadFD);

		UnblockAllSignals(&o_signal_mask, &p_signal_mask);
		ExamineSignals(&p_signal_mask, 1, sizeof(sigset_t), sig_handler);

		if(retval == 0)
		{
			retval = select (_netfd+1, &_ReadFD, NULL, NULL, NULL);
		}
		else
		{
                        retval = select (_netfd+1,
                                &_ReadFD, NULL, NULL, &tout);
                }

		BlockCommonSignals(&o_signal_mask);

		if (retval < 0)
		{
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("select %m"));
			goto _continue;
		}
		if(retval == 0)
		{
                        serviceTimers(&timerPrivate);
			continue;	
		}
		if ( FD_ISSET (_netfd, &_ReadFD) )
		{
			ProcessData (_netfd, client);
		}

     _continue:
		;
	}
	return 0;
}
	
int
PktLen(char *buf)
{
	PktHeader *hdr = (PktHeader *)buf;

	return (ntohl(hdr->totalLen));
}

/* Just handle the encapsulations, and transfer control
 * to lower layer, if necessary. All packet replies
 * to be sent back, by this layer
 */
int
ProcessData (int sockfd, struct sockaddr_in client)
{
	char *start, *end;
	static long buffer[4096/sizeof(long)+1];
	PktHeader *pkt_hdr;
	Pkt	*data_pkt, reply_pkt;
	int	retval = 1;
	int     read_bytes = 0;
	int 	type;
	sigset_t o_signal_mask, n_signal_mask;
	PefHeader *pef;
	PktOpaque opaque;

	bzero (&reply_pkt, sizeof(Pkt));

	start = end = (char *)&buffer[0];

	read_bytes = retval = read(sockfd, (char *)buffer, 4096);

	if (retval <= 0)
	{
	     /* We dont know now, where we are in the stream... */
	     retval = 1;	/* terminate... */
	     goto _do_nothing;
	}

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
		NETDEBUG(MAGE, NETLOG_DEBUG1, ("Recd. PKT_PROFILE \n"));
             	ProcessProfile(sockfd, data_pkt, &reply_pkt,
                            0, 0, 0);

		ProcessHeartbeat(sockfd, data_pkt, &reply_pkt,
			0, 0, 0);
		break;
	case PKT_HEARTBEAT:
	
		/* We got a heartbeat */
		NETDEBUG(MAGE, NETLOG_DEBUG1, ("Recd. PKT_HEARTBEAT\n"));
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
	     data_pkt->type = (PKT_ERROR);
		htonPkt(PKT_ERROR, data_pkt);	
	     PktSend(sockfd, data_pkt);
	     
	     break;
	}
	
 _do_nothing:
	return (retval);
}

int
ProcessProfile (int sockfd, Pkt * data_pkt, Pkt * reply_pkt,
		 void *opaque, int opaquelen, /* Arguments to be passed
					       * to the write call back.
					       */
		 int (*writecb)())		
{
     	struct in_addr in;
        char buf[INET_ADDRSTRLEN];
     	int h, m, secs, dur;

     	in.s_addr = htonl(data_pkt->credo.ipaddress.l);
     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR START %s (%s.%d) ******************\n", 
		BIT_TEST(data_pkt->data.profile.flags, CDR_CALLSETUP) ?
		"Call Setup": "Call Termination",
		inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN), (data_pkt->credo.uport)));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		("Local (%s)\n", 
		BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin": "Call Destination"));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Registration ID %s\n", data_pkt->data.profile.local.regid));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Port %d\n", (data_pkt->data.profile.local.uport)));
     	NETCDR(MCDR, NETLOG_DEBUG1,(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Phone %s\n", data_pkt->data.profile.local.phone));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o VpnPhone %s\n", data_pkt->data.profile.local.vpnPhone));


     	NETCDR(MCDR, NETLOG_DEBUG1,("Remote (%s)\n",
		!BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin": "Call Destination"));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Registration ID %s\n", data_pkt->data.profile.remote.regid));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Port %d\n", (data_pkt->data.profile.remote.uport)));
     	in.s_addr = htonl(data_pkt->data.profile.remote.ipaddress.l);
     	NETCDR(MCDR, NETLOG_DEBUG1,(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Phone %s\n", data_pkt->data.profile.remote.phone));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o VpnPhone %s\n", data_pkt->data.profile.remote.vpnPhone));

     	if (!BIT_TEST(data_pkt->data.profile.flags, CDR_CALLSETUP))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,("Duration\n"));
     		dur = (data_pkt->data.profile.ftime) - 
			(data_pkt->data.profile.ctime);
     		h = dur/3600; m = dur/60 - h*60; secs = dur - h*3600 - m*60;

     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o %d Hrs, %d Mts and %d Secs\n", h, m, secs));
	}

     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR END ******************\n"));

     return 0;
}

int
ProcessCdrLocal(XMLCacheCb *cb)
{
	InfoEntry *entry = &cb->infoEntry;
     	struct in_addr in;
        char buf[INET_ADDRSTRLEN];
     	int h, m, s, dur, rdur, sdur;

	/* The local node is captured in the infoEntry struct */
     	in.s_addr = htonl(entry->ipaddress.l);

     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR START (%s.%d) ******************\n", 
		inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN), (entry->uport)));

	if (BIT_TEST(entry->sflags, ISSET_REGID))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Registration ID %s\n", entry->regid));
	}

	if (BIT_TEST(entry->sflags, ISSET_UPORT))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Port %d\n", (entry->uport)));
	}

	if (BIT_TEST(entry->sflags, ISSET_IPADDRESS))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
	}

	if (BIT_TEST(entry->sflags, ISSET_PHONE))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Phone %s\n", entry->phone));
	}

	if (BIT_TEST(entry->sflags, ISSET_VPNPHONE))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o VpnPhone %s\n", entry->vpnPhone));
	}

     	return 0;
}

int
ProcessCdrRemote(XMLCacheCb *cb)
{
	InfoEntry *entry = &cb->infoEntry;
     	struct in_addr in;

     	in.s_addr = htonl(entry->ipaddress.l);

	if (BITA_TEST(cb->tagh, TAG_CALLANSWER))
	{
		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Call Recepient:\n"));
	}
	else
	{
		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Call Originator:\n"));
	}

	if (BIT_TEST(entry->sflags, ISSET_REGID))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Registration ID %s\n", entry->regid));
	}
	
	if (BIT_TEST(entry->sflags, ISSET_UPORT))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Port %d\n", (entry->uport)));
	}

	if (BIT_TEST(entry->sflags, ISSET_IPADDRESS))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
	}

	if (BIT_TEST(entry->sflags, ISSET_PHONE))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Phone %s\n", entry->phone));
	}

	if (BIT_TEST(entry->sflags, ISSET_VPNPHONE))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o VpnPhone %s\n", entry->vpnPhone));
	}

     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR END ******************\n"));
}

int
ProcessHeartbeatCb(XMLCacheCb *cb)
{
	InfoEntry *entry = &cb->infoEntry;

	NETDEBUG(MAGE, NETLOG_DEBUG4,
		("Heartbeat from %s %d has lus address 0x%x vpns address 0x%x\n",
			entry->regid,
			(entry->uport), 
			(cb->aloidIpAddress),
			(cb->vpnsIpAddress)));

	/* In 1.2 its the bcs which gets the heartbeats forwarded to ! */
	/* ForwardHeartbeat(0, cb->buf, cb->buflen, cb->aloidIpAddress, cb->vpnsIpAddress); */
}

int
ProcessHeartbeat (int sockfd, Pkt* data_pkt, Pkt * reply_pkt,
                 void *opaque, int opaquelen, /* Arguments to be passed
                                               * to the write call back.
                                               */
                 int (*writecb)())
{
	int i;

	/* Heartbeat has arrived */
	/* Forward it to the age daemons, if they are running
	 * somewhere in the system
	 */	
	ForwardHeartbeat(0, data_pkt, (data_pkt->totalLen), 
		data_pkt->data.profile.aloidIpAddress, data_pkt->data.profile.vpnsIpAddress);

	return 1;
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
		case CONFIG_SERPLEX_GIS:
			if ((aloidIpAddress != 0) || (vpnsIpAddress != 0))
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Delivering heartbeat to 0x%x:%d",
				ntohl(serplexes[i].age.location.address.sin_addr.s_addr), 
				ntohs(serplexes[i].age.location.address.sin_port)));
	
				if ((nbytes = sendto(_netfd, buf, buflen, 0, 
					(struct sockaddr *)&serplexes[i].age.location.address,
					sizeof(struct sockaddr_in))) < 0)
				{
					ERROR(MAGE, ("Server %d sendto %m\n", i));
					perror("sendtoage");
				}
			}
			else
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Heartbeat does not have lus/vpns address\n"));
			}

			break;
		case CONFIG_SERPLEX_LUS:
			if (aloidIpAddress != 0)
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Delivering heartbeat to 0x%x:%d",
				serplexes[i].age.location.address.sin_addr.s_addr, 
				serplexes[i].age.location.address.sin_port));
	
				if (sendto(_netfd, buf, buflen, 0, 
					(struct sockaddr *)&serplexes[i].age.location.address,
					sizeof(struct sockaddr_in)) < 0)
				{
					ERROR(MAGE, ("Server %d sendto %m\n", i));
					perror("sendtoage");
				}
			}
			else
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Heartbeat does not have lus address\n"));
			}

			break;
		case CONFIG_SERPLEX_VPNS:
			if (vpnsIpAddress != 0)
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Delivering heartbeat to 0x%x:%d",
				serplexes[i].age.location.address.sin_addr.s_addr, 
				serplexes[i].age.location.address.sin_port));
	
				if (sendto(_netfd, buf, buflen, 0, 
					(struct sockaddr *)&serplexes[i].age.location.address,
					sizeof(struct sockaddr_in)) < 0)
				{
					ERROR(MAGE, ("Server %d sendto %m\n", i));
					perror("sendtoage");
				}
			}
			else
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
					("Heartbeat does not have vpns address\n"));
			}

			break;
		default:
			break;
		}
	}

	return 1;
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

	ServerSetLogging("bcs", &serplexes[match]);

	/* Set the priority */
#ifdef _print_priority
	fprintf(stderr, "setting priority to %d\n",  serplexes[match].prio);
#endif
	setpriority(PRIO_PROCESS, 0, serplexes[match].prio);
}
	
