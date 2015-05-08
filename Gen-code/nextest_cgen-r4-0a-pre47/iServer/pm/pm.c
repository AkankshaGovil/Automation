#include <stdio.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "nxosd.h"
#include "pids.h"
#include "pmpoll.h"
#include "srvrlog.h"
#include "spversion.h"
#include "mem.h"
#include "lsconfig.h"
#include "ifs.h"
#include "errno.h"
#include "defaultpath.h"
#include "serverp.h"
#ifndef NETOID_LINUX
#include <sys/priocntl.h>
#include <sys/tspriocntl.h>
#endif
#include "server.h"
#include "pm.h"



/* Define SERVER_DBG to debug this mammoth... */
#ifndef SERVER_DBG
#define USE_FORK
#define USE_DAEMONIZE
#endif

char	config_file[60] = CONFIG_FILENAME;
char	pidfile[256];
                                /* Default value for config_file */

int	DebugMode =0; /* when set to 1 the iserver is not shutdown on poll failure */
int debug = 0;
char    * serplexpath;
static int pmStartTime;
static int pmConfiguredPriority;

/* Variables to set all child processes as non-realtime */
int TSClassInited = 0;
#ifndef NETOID_LINUX
pcinfo_t ts_pcinfo;
pcparms_t ts_pcparms;
#endif

extern MonitoredPsGrp psGrplist[];
extern int restartflag;

LsMemStruct *lsMem;

int
main(int argc, char *argv[])
{
	int 	restarts = 0;
	int 	falsestarts = 0;
	sigset_t	zeromask,newmask,oldmask,allmask,mymask;
	int	mypid;
	fd_set  rset;
	time_t 	last,now;
	struct 	timeval polltout;
	double 	delta;
	int	sd;
	int	nready;
	int	maxsd;
	int	polltimeout = POLL_TIME_OUT;
	char    * penv;
	char 	*basedir;
	int		j;
	int 	flags;
	int		rc = 0, nrestarts = 0;
	int		psindex = 0;



	pmStartTime = time(NULL);

	if(argc>1)
	{
		polltimeout = atoi(argv[1]);
		if(polltimeout == 0 )
		{
			/* arbitrary large value */
			polltimeout = 9999;
			DebugMode =1;
		}
	}

	/* Change the directory to the DEF_MSW_BASE_DIR */
	if ( ((basedir = getenv("SERPLEXPATH")) || (basedir = DEF_MSW_BASE_DIR) ) &&
			chdir(basedir) ) {
		NETERROR(MPMGR, ("Error changing to dir - %s: %s\n", basedir, \
			strerror(errno)));
        fprintf(stderr, "Error changing to dir - %s: %s\npm exiting ...", 
			basedir, strerror(errno));
		exit(1);
	}

        /* If the env variable exists and a +ve number..,
         * set debug to that value.
         */
        penv = (char *)getenv ("ALOID_DEBUG");

        if ((penv && (atoi(penv) > 0)) )
        {
                debug = atoi(penv);
        }
        else
        {
#ifdef USE_DAEMONIZE
                daemonize ();
#endif
        }

	// SignalInit must be after daemonize()!!

	SignalInit();

		if(!(serplexpath = (char *)getenv("SERPLEXPATH")))
		{
			serplexpath = strdup("/usr/local/nextone/bin/");
		}

#if 0
	NetLogInit();
//use real logging
	penv = (char *)getenv("PM_LOG");
        if ((penv && (atoi(penv) > 0)) )
        {
		flags = atoi(penv);
		netLogStruct.flags |= NETLOG_ASYNC;
		NetLogStatus[MPMGR] |= NETLOG_DEBUG4; 
		NetLogStatus[MPMGR] |= NETLOG_DEBUG3; 
		NetLogStatus[MPMGR] |= NETLOG_DEBUG2; 
		NetLogStatus[MPMGR] |= NETLOG_DEBUG1; 
	}
	flags|=NETLOG_ASYNC;
	NetSyslogOpen("PM",flags);
#endif
  setConfigFile();
	ifihead = initIfs();
	myConfigServerType = CONFIG_SERPLEX_PM;

  DoConfig(ProcessConfig);

	/* ensure that its not running */

	sprintf(pidfile, "%s/%s", PIDS_DIRECTORY, PM_PID_FILE);
	mypid = ReadPid(pidfile);

        if ((mypid > 0) && (kill(mypid, 0) == 0))
        {
                ERROR(MPMGR,("%s seems to be running already\n", argv[0]));
                exit(0);
        }
        StorePid(pidfile);
	
#ifndef NETOID_LINUX
//	Make the main thread realtime. 
	ThreadInitRT();
	ThreadSetRT();
#endif

//	Add a pthread_atfork handler which sets all subsequent children to non-RT    
	if (realTimeEnable)
	{	
#ifndef NETOID_LINUX
		InitNonRTParams();
#endif
		pthread_atfork(NULL, NULL, ChildSetNonRT);
	}

	for (;;)
	{
		/* start the udp server to receive keep alives */
		if((sd = initserver())>=0)
		{
			break;
		} else 
		{
			ERROR(MPMGR,("unable to setup UDP Server\n"));
			sleep(10);
		}
	}

		
	/* setup data structures for polling */
	initPsGrp();

	/* Sleep for START_TIME_OUT before starting the poll */
	/* sleep(START_TIME_OUT);*/

	/* Prepare for select */
	FD_ZERO(&rset);
	maxsd = sd+1;
	
	// Cleanup anything which may exist
	system("/usr/local/nextone/bin/ipcrmall");

	/* Start the Processes */
	for(rc = startProcesses(sd);rc!=0 && nrestarts <5;nrestarts++)
	{
		NETERROR(MPMGR,("Failed to start processes. %d attempts \n",nrestarts+1));
		system(psGrplist[0].stopcmd);
		sleep(3);
		rc= startProcesses(sd);
	}

	if(nrestarts == 5)
	{
		NETERROR(MPMGR,
		("Too many failed attempts. iServer could not be started. Exiting!!\n"));
		system(psGrplist[0].stopcmd);
		exit (-1);
	}


        /* Set the priority */
#ifdef print_priority
        fprintf(stderr, "setting priority to %d\n", pmConfiguredPriority);
#endif
        setpriority(PRIO_PROCESS, 0, pmConfiguredPriority);

	last = 0;	
	for(;;)
	{
		now = time(NULL);
		/* reset the polltout here */
		delta = difftime(now,last);
		if(delta <polltimeout && !DebugMode)
		{
			polltout.tv_sec = polltimeout - delta; 
		}
		else 
		{
				polltout.tv_sec = polltimeout;
				last = now;
		}

		polltout.tv_usec = 0;
		DEBUG(MPMGR,NETLOG_DEBUG4,
			("PM main - set timeout = %ld\n",polltout.tv_sec));

		FD_SET(sd,&rset);
		if((nready = select(maxsd,&rset,NULL,NULL,&polltout))<0)
		{
			if(errno == EINTR)
			{
				NETDEBUG(MH323,NETLOG_DEBUG4,("Select Error."));
			}
			errno = 0;
			continue;
		}
		if(FD_ISSET(sd,&rset))
		{
			handleKA();
		}
		now = time(NULL);
		delta = difftime(now,last);
		if((delta >= polltimeout) || (nready ==0) ||  (restartflag ==TRUE)) 
		{
		  PsEntry *ps;
		  for(j=0;j<MAX_PS_GRP;++j)
		  {
			if(checkPoll(j,sd)!=0 )
			{
				/* give more time before nextpoll */
				last = time(NULL);
			}
		  }
		  restartflag = FALSE;
		} 
	}/* End Poll Loop */
}	


void blockallsignals(void)
{

  char 		fn[] = "blockallsignals() :";
  sigset_t	zeromask,newmask,oldmask;
  sigset_t	allmask;
  	
	sigfillset(&allmask);

        if ( sigprocmask (SIG_BLOCK , &allmask , &oldmask) <0)
	{
		printf( "%s %s\n", fn,strerror(errno));
	}
}

/* fills the buffer with uptime information for the servers 
 * currently registered
 *
 * returns the number of bytes to send
 */
int FillUptimeInfo (char *buf) {
   char sinfo[3][256] = {0};
   long stime[3] = {0};
   int i, len;
   short slen;

   sprintf(sinfo[0], "Uptime for: %s %s%s, %s", PM_NAME, PM_VERSION, PM_MINOR, PM_BUILDDATE);
   stime[0] = pmStartTime;

   sprintf(sinfo[1], "Uptime for: %s %s%s, %s", GIS_NAME, GIS_VERSION, GIS_MINOR, GIS_BUILDDATE);
   stime[1] = psGrplist[SERPLEX_GID].pslist[GIS_ID].startupTime;

   len = 0;
   slen = 2;
   buf[len++] = ((slen & 0xff00)>>8);
   buf[len++] = (slen & 0x00ff);

   for (i = 0; i < 2; i++) {
	  /* write the version string as a java 'UTF' 
		 (2 byte length followed by the string */
	  slen = strlen(sinfo[i]);
	  buf[len++] = ((slen & 0xff00)>>8);
	  buf[len++] = (slen & 0x00ff);
	  memcpy(&buf[len], sinfo[i], slen);
	  len += slen;

	  /* write time as a java 'long' (8 bytes) */
	  for (slen = 0; slen < 4; slen++)
		 buf[len++] = 0;
	  buf[len++] = (unsigned char)((stime[i] & 0xff000000)>>24);
	  buf[len++] = (unsigned char)((stime[i] & 0x00ff0000)>>16);
	  buf[len++] = (unsigned char)((stime[i] & 0x0000ff00)>>8);
	  buf[len++] = (unsigned char)(stime[i] & 0x000000ff);
   }

   return len;
}

static int
ProcessConfig(void)
{
        int match = -1;

        /* Process Configuration, read from the config file */
        match = FindServerConfig();

        if (match == -1)
        {
                fprintf(stderr, "Could not find log level...\n");
	            return(-1);
        }

        iserver = &serplexes[match];

        if (iserver == NULL)
        {
                fprintf(stderr, "Iserver Index Invalid %d\n", match);
                exit(0);
        }

        pmConfiguredPriority = serplexes[match].prio;

        if (serplexes[match].max_endpts > 0)
        {
                max_endpts = serplexes[match].max_endpts;
        }

        if (serplexes[match].max_gws > 0)
        {
                max_gws = serplexes[match].max_gws;
        }

        ServerSetLogging("pm", &serplexes[match]);
		return 0;
}

int
WaitForAck(PsEntry *ps,int sd,int waittime)
{
	static  char fn[] = "WaitForAck";
	int 	maxsd;
	time_t	now,endtime;
	int 	nready;
	struct timeval polltout, *pTimeval = NULL;
	fd_set  rset;

	endtime = now;

	if(waittime > 0 )
	{
		pTimeval = &polltout;
		time(&now);
		endtime = now+waittime;
	}	
		
	FD_ZERO(&rset);
	maxsd = sd + 1;

	ps->poll = 0;
	for(;;)
	{
		if(waittime >=0)
		{
			time(&now);
			if(now >endtime) 
			{
				return -1;
			}
			polltout.tv_sec = endtime - now;
			polltout.tv_usec = 0;
		}

		FD_SET(sd,&rset);

		if((nready = select(maxsd,&rset,NULL,NULL,pTimeval))<0)
		{
			ERROR(MPMGR,("Select Error %d \n",errno));
			errno = 0;
			continue;
		}

		if(FD_ISSET(sd,&rset))
		{
			handleKA();
		}

		if(ps->poll == 1)
		{
			DEBUG(MPMGR,NETLOG_DEBUG1,("%s received ack from %s\n",
				fn,ps->psname));
			return 0;
		}

	}/* End Poll Loop */
	return 0;
}

/* generic signal handler */
static void
sig_handler(int signo)
{
	char fn[] = "sig_handler";
    switch (signo)
    {
    case SIGINT:
        sig_int(SIGINT);
        break;
    case SIGCHLD:
        sig_chld(SIGCHLD);
        break;
    case SIGHUP:
	    NetLogClose();
		DoConfig(ProcessConfig);
		/* Update pslist based on reconfig */
		if(!RSDConfigured())
		{
			psGrplist[0].pslist[RSD_ID].adminStatus = PM_ePsDisabled;
		}
		else {
			psGrplist[0].pslist[RSD_ID].adminStatus = PM_ePsEnabled;
		}
		DEBUG(MPMGR,NETLOG_DEBUG4,
			("%s Process %s is %s\n",
			fn,psGrplist[0].pslist[RSD_ID].psname,
			(psGrplist[0].pslist[RSD_ID].adminStatus==PM_ePsEnabled)? "Enabled":"Disabled"));

    default:
        break;
    }
}

int 
startProcesses(int sd)
{

	static char fn[]="startProcesses()";
	int i;
	PsEntry *ps;
	int	rc;

	NETINFOMSG(MDEF, ("*** Starting NexTone iServer ***\n"));

	DEBUG(MPMGR,NETLOG_DEBUG4,("inside start process\n"));	
	// Just Start JServer and Gis
	ps = psGrplist[0].pslist;
	for(i = 0; i<psGrplist[0].pscnt; ++i,ps++)
	{
		if(ps->adminStatus!= PM_ePsEnabled)
		{
			DEBUG(MPMGR,NETLOG_DEBUG4,
				("*********%s %s is disabled. Not starting\n",fn,ps->psname));	
			continue;
		}
	DEBUG(MPMGR,NETLOG_DEBUG4,("*********%s %s starting.\n",fn,ps->psname));	

        // set the priority that the child process desires
        setpriority(PRIO_PROCESS, 0, pmConfiguredPriority+ps->relativePriority);
	
		//start process
		if(start(ps)!=0)
		{
                ERROR(MPMGR,("%s Unable to start process, %s \n",
                      fn,
                      ps->psname));
				return -1;
		}
		if(ps->startMode == PM_ePsSyncStart)
		{
			DEBUG(MPMGR,NETLOG_DEBUG3,("%s %s start mode is SyncStart.\n",
				fn,ps->psname));
			// wait for poll
			if(WaitForAck(ps,sd,30)!=0)
			{
                ERROR(MPMGR,("%s Did not get Ack from process, %s\n",
                      fn,
                      ps->psname));
				return -1;
			}
		}	
	}

	NETINFOMSG(MDEF, ("*** NexTone iServer started ***\n"));

	return 0;
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

	// Setup SIGCHLD special case

	sigemptyset( &sigact.sa_mask );
	sigact.sa_flags = (SA_RESTART|SA_NOCLDSTOP|SA_ONSTACK);
	sigact.sa_handler = SIG_IGN;

	sigaction( SIGCHLD, &sigact, NULL );

	sigset( SIGCHLD, DummySigHandler );

	// Setup async signals to be ignored

	sigemptyset( &sigact.sa_mask );

	sigact.sa_flags = (SA_RESTART|SA_ONSTACK);
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
#ifndef NETOID_LINUX
	sigaction(SIGEMT,	&sigact, NULL );
#endif
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
		
		nx_sig2str( signo, c_sig,sizeof(c_sig) );

		switch (signo)
		{
		case SIGINT:
			sig_handler(signo);
			break;

		case SIGTERM:
			sig_handler(SIGINT);
			break;

		case SIGCHLD:
			sig_handler(signo);
			break;

		case SIGHUP:
			sig_handler(signo);
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
//		DummySigHandler()
//
//	Description :
//		This routine is a dummy signal catcher used for
//		SIGCHLD signals. (never called)
//
static void
DummySigHandler(int signo)
{
	NETERROR(	MDEF,
						("DummySigHandler() : Caught %d signal\n",
						signo ));
	return;
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
	int32_t thread = pthread_self();
	char	c_sig[32];
	int		pid;

	nx_sig2str( signo, c_sig, sizeof(c_sig) );

	switch (signo)
	{
	case SIGBUS:
#ifndef NETOID_LINUX
	case SIGEMT:
#endif
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

void
ChildSetNonRT(void)
{
	char fn[] = "ChildSetNonRT():";

	// Check to see if the initialization routine has been called,
	// as only then we will proceed to non-realtime mode
	if (TSClassInited == 0)
	{
 		return;
	}

#ifndef NETOID_LINUX
	if (priocntl(P_PID, P_MYID, PC_SETPARMS, (caddr_t)&ts_pcparms) < 0)
	{
		NETERROR(MPMGR, ("%s priocntl error %d\n", fn, errno));
	}
#endif

	return;
}

#ifndef NETOID_LINUX
int
InitNonRTParams(void)
{
	char fn[] = "InitNonRTParams():";
	tsparms_t   *tsparmsp = (struct tsparms *) ts_pcparms.pc_clparms;

	// initialize the class id for thread class
	strcpy(ts_pcinfo.pc_clname, "TS");

	if (priocntl(0, 0, PC_GETCID, (caddr_t)&ts_pcinfo) < 0)
	{
		NETERROR(MINIT, ("%s priocntl error %d\n", fn, errno));
			return -1;
	}

	// set up the tpcparms for everyone to use
	memset( &ts_pcparms, 0, sizeof(pcparms_t) );

	ts_pcparms.pc_cid = ts_pcinfo.pc_cid;
	tsparmsp->ts_uprilim = TS_NOCHANGE;
	tsparmsp->ts_upri = 59;

	TSClassInited = 1;

	return 0;
}
#endif
