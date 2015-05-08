#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include <signal.h>
#include<errno.h>
#include<string.h>

#include "nxosd.h"
#include "srvrlog.h"
#include "pmpoll.h"
#include "pids.h"
#define  NO_AVL_NEEDED
#include "lsconfig.h"

#ifndef NETOID_LINUX
#define _KMEMUSER 
#include <sys/user.h>
#include <sys/proc.h>	
#include <kvm.h>
#else
#include <dirent.h>
#define PF_DUMPCORE 0x00000200      /* dumped core */
#endif
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "pm.h"
#include "serverp.h"

extern int flag;
extern int DebugMode;
extern	char *serplexpath;

int restartflag = FALSE;

// KEEP the PROCESS_COUNT in sync with the no of active processes in pslist
PsEntry pslist1[] = 
		{ 
			{ JSERVER_ID,-2,0,0,PM_ePsSyncStart,"jServer.jar","./iserver jserver start","XXXXX",0,0,PM_ePsEnabled,"./iserver jserver stop", 0},
	  		{ EXECD_ID,-2,0,0,PM_ePsAsyncStart,"execd","./execd",EXECD_PID_FILE,0,1,PM_ePsEnabled,"./iserver execd stop", 0}, 
			{ GIS_ID,-2,0,0,PM_ePsAsyncStart,"gis","./gis",GIS_PID_FILE,0,0,PM_ePsEnabled,"./iserver gis stop", 0},
	  		{ RSD_ID,-2,0,0,PM_ePsAsyncStart,"rsd","./rsd",RSD_PID_FILE,0,1,PM_ePsEnabled,"./iserver rsd stop", 0}, 
	      };
/*
PsEntry pslist2[] = 
			{
				{ JSERVER_ID,-2,0,0,"jserver","XXXXXX"} 
			};
*/

MonitoredPsGrp psGrplist[2];

/* static int idToGrpId[MAX_PS_ID];*/


void initPsGrp(void)
{
	static char fn[] = "initPsGrp()";
	int j;
	int psindex,psgrp;
	PsEntry	*ps;
	char	tmp[80];

	psGrplist[0].pslist = pslist1;
	psGrplist[0].pscnt = PROCESS_COUNT;
	nx_strlcpy(psGrplist[0].stopcmd,"date >> /var/log/pmrestart.log;"
			"/usr/local/nextone/bin/cli test passive >> /var/log/pmrestart.log;"
			"/usr/local/nextone/bin/cli stats >> /var/log/pmrestart.log;"
			"/usr/local/nextone/bin/iserver serplex stop>> /var/log/pmrestart.log",512);
	nx_strlcpy(psGrplist[0].startcmd,"/usr/local/nextone/bin/ipcrmall >>/dev/null;"
		"/usr/local/nextone/bin/iserver all start>>/dev/null",512);
	strcpy(psGrplist[0].grpName,"serplex");
	psGrplist[0].restarts = 0;
	//psGrplist[1].pslist = pslist2;
	//psGrplist[1].pscnt = 1;
	//psGrplist[1].restarts = 0;
	//strcpy(psGrplist[1].stopcmd,"/usr/local/nextone/bin/iserver jserver stop>>/dev/null");
	//strcpy(psGrplist[1].startcmd,"/usr/local/nextone/bin/iserver jserver start >>/dev/null");
	//strcpy(psGrplist[1].grpName,"jserver");


	/* Check Admin Status */
	if(!RSDConfigured())
	{
		psGrplist[0].pslist[RSD_ID].adminStatus = PM_ePsDisabled;
		DEBUG(MPMGR,NETLOG_DEBUG1,
			("%s Process %s is Disabled\n",
			fn,psGrplist[0].pslist[RSD_ID].psname));
	}

	for(j=0;j<MAX_PS_GRP;++j)
	{
		resetPoll(&psGrplist[j]);
	}
#if 0
	for(psgrp=0;psgrp<MAX_PS_GRP;++psgrp)
	{
		for(j=0,ps = &psGrplist[psgrp].pslist[0]; j<psGrplist[psgrp].pscnt;++j,ps++)
		{
			strcpy(tmp,ps->cmdline);
			sprintf(ps->cmdline,"%s%s",serplexpath,tmp);
		}
	}
#endif
}

void resetPoll(MonitoredPsGrp *psgrp)
{
	int i;
	for (i = 0; i< psgrp->pscnt; ++i)
	{
		psgrp->pslist[i].pollmissed = 0;
		psgrp->pslist[i].poll = 1;
	}
   	return;
}

#if 0
void initPollTbl (void)
{
	int i;
	for (i = 0; i< MAX_ISERVER_PS; ++i)
	{
		pollTbl[i].pollmissed = 0;
		pollTbl[i].poll = 0;
	}
}
#endif

#if 0
int initpids(void)
{
	static char fn[] = "initpids() :";
	int i,pid;
	char pidfilepath[256];
	int status = 0;
	
	for (i = 0; i<MAX_ISERVER_PS;++i)
	{
		sprintf(pidfilepath,"%s/%s",PIDS_DIRECTORY,pollTbl[i].pidfile);

		pid = ReadPid(pidfilepath);

		if ((pid > 0) && (kill(pid, 0) == 0))
		{
			pollTbl[i].pid = pid;
		}
		else
		{
			printf("%s %s not started. File = %s pid = %d\n",
				fn,pollTbl[i].psname,pidfilepath,pid);
			status = -1;
		}
	}
	return status;
}
#endif


/*
   checks and resets poll flag for all the processes. returns error if any ps 
   missed poll more than MAX_POLL_MISSES times. Else returns OK.
*/
int checkPoll(int grpid,int sd)
{
	static char fn[]="handlepolltimeout: ";
	int i;
	int status = 0;
	char	cmd[256];
	PsEntry *ps, *pslist = psGrplist[grpid].pslist;
	DEBUG(MPMGR,NETLOG_DEBUG4,("handlepolltimeout: Entering cnt = %d ps = %s\n",psGrplist[grpid].pscnt,psGrplist[grpid].grpName));
	for (i= 0,ps = pslist;i<psGrplist[grpid].pscnt;++i,ps++)
	{
		if(pslist[i].adminStatus!=PM_ePsEnabled)
		{
			continue;
		}
		DEBUG(MPMGR,NETLOG_DEBUG4,
			("%s Process %s poll = %d missed = %d\n",
			fn,ps->psname,ps->poll,ps->pollmissed));

		if(ps->poll == 1)
		{
			ps->poll = 0;
			ps->pollmissed = 0;
		}
		else {
			ps->pollmissed++;

			DEBUG(MPMGR,NETLOG_DEBUG1,
				("%s Process %s missed keepalive. pollmissed = %d\n",
				fn,ps->psname,ps->pollmissed));

			if(ps->pollmissed > MAX_POLL_MISSES && !DebugMode)
			{	
				ERROR(MPMGR,("%s missed poll too many times\n",
						ps->psname));
				// if we are restarting on user command, no need for a core
				if(ps->psindex!=JSERVER_ID && restartflag != TRUE &&
					wait_for_coredump (ps->psname) != TRUE)
				{
					sprintf(cmd, "./pmcore %s", ps->psname);
					NETERROR(MPMGR, ( "%s Generating core [%s]\n", 
						fn, cmd));
					system(cmd); 
				}
				status = -1;
				if(ps->independentlyRestartable)
				{
					ERROR(MPMGR,("%s Restarting Process %s.\n",
						fn,ps->psname));
					system(ps->stopcmd);
					//start process
					if(start(ps)!=0)
					{
						ERROR(MPMGR,("%s Unable to start process, %s \n",
							  fn,ps->psname));
					}
					/* reset poll */
					ps->pollmissed = 0;
					ps->poll = 1;
				}
				else {
					ERROR(MPMGR,("%s Restarting PsGrp %s.Restarts = %d\n",
					fn,psGrplist[grpid].grpName,
					++psGrplist[grpid].restarts));

					NETINFOMSG(MDEF, ("*** NexTone iServer shutdown started ***\n"));

					system(psGrplist[grpid].stopcmd);

					NETINFOMSG(MDEF, ("*** NexTone iServer shutdown finished ***\n"));

					startProcesses(sd);
					resetPoll(&psGrplist[grpid]);
				}
			}
		}
	}
	return status;
}

#if 0
/* returns TRUE if all poll Tbl processes have exited. Else returns FALSE  */
static int allPsExited(void)
{
  int i;
  int retval = 1;
  int	pid;

  for (i=0;i<MAX_ISERVER_PS; ++i)
  {
	if((pid = pollTbl[i].pid) <= 0 ) continue;
  	retval &= !(kill(pid,0));
   }
   return retval ;
}
#endif

void handleKA(void)
{
  static char fn[] = "handleKA() : ";
  int psid,grpid;
  KeepAlive kamsg;


  while (handlepkt(&kamsg) == 0)
  {
    if (kamsg.pid == -1) {
	   continue;
	}

  	psid = kamsg.id;
	grpid = kamsg.gid;
  	DEBUG(MPMGR,NETLOG_DEBUG4,("%s id = %d\tgrpid=%d\npid = %d\n",fn,psid,grpid,kamsg.pid));
	if(grpid>=MAX_PS_GRP || psid>=psGrplist[grpid].pscnt)
	{
		ERROR(MPMGR,("%s Received invalid id = %d\n",fn,psid));
		return;
	}

	/* The packet is valid only if it contains a startup time more recent than the
	 * startup time of the process maintained by pm */
	if (difftime(kamsg.startupTime, psGrplist[grpid].pslist[psid].startupTime) < 0) {
		DEBUG(MPMGR, NETLOG_DEBUG2, ("Discarding packet received from older instance of %s\n",
			psGrplist[grpid].pslist[psid].psname));
		continue;
	}

	psGrplist[grpid].pslist[psid].poll =  1;
	psGrplist[grpid].pslist[psid].pollmissed = 0;
	psGrplist[grpid].pslist[psid].startupTime = kamsg.startupTime;

	//  if restart field is set, increase the pollmissed count
	if (kamsg.restartid  ==  ISERVER_RESTART) {
	  psGrplist[SERPLEX_GID].pslist[GIS_ID].poll =  0;
	  psGrplist[SERPLEX_GID].pslist[GIS_ID].pollmissed = MAX_POLL_MISSES+1;
	  restartflag = TRUE;
	  NETINFOMSG(MPMGR, 
		("%s: Process %s is requesting to restart the iServer\n",
		 fn, psGrplist[grpid].pslist[psid].psname));
	  DEBUG(MPMGR, NETLOG_DEBUG1,
		("%s: Process %s requesting to restart iServer, setting GIS poll miss count to %d\n",
		 fn, psGrplist[grpid].pslist[psid].psname,
		 psGrplist[SERPLEX_GID].pslist[GIS_ID].pollmissed));
	}
	else if (kamsg.restartid == RSD_RESTART) {
	  psGrplist[SERPLEX_GID].pslist[RSD_ID].poll = 0;
	  psGrplist[SERPLEX_GID].pslist[RSD_ID].pollmissed = MAX_POLL_MISSES+1;
	  restartflag = TRUE;
	  // set the admin status to enabled, so that PM will attempt to restart him
	  psGrplist[SERPLEX_GID].pslist[RSD_ID].adminStatus = PM_ePsEnabled;
	  NETINFOMSG(MPMGR, 
		("%s: Process %s is requesting to restart RSD\n",
		 fn, psGrplist[grpid].pslist[psid].psname));
	  DEBUG(MPMGR, NETLOG_DEBUG1,
		("%s: Process %s requesting to restart RSD, setting RSD poll miss count to %d\n",
		 fn, psGrplist[grpid].pslist[psid].psname,
		 psGrplist[SERPLEX_GID].pslist[RSD_ID].pollmissed));
	}
	kamsg.restartid  = ISERVER_NONE; 

	DEBUG(MPMGR,NETLOG_DEBUG4,
		("%s Process %s pollmissed = %d\n",
		fn,psGrplist[grpid].pslist[psid].psname,psGrplist[grpid].pslist[psid].pollmissed));
  }
  return;

} 

int start(PsEntry *ps)
{
	char 		*envstr;
	sigset_t 	newmask;
	int 		fd, nfds, pid;
	int 		i;
	char 		*p;
	char 		*argv[10];

	ps->poll = 0;
	ps->pollmissed = 0;
	ps->startupTime = time(0);

	if((pid = fork()) == -1)
	{
		pid = 0;
		return -1;
	}

	if(pid == 0)
	{
#if 0
		idstr = (char *) malloc(MAX_ENV_STR*(sizeof(char)));
		sprintf(idstr,"CHILD_ID=%d",id);
		putenv(idstr);
#endif
		envstr = (char *) malloc(MAX_ENV_STR*(sizeof(char)));
		sprintf(envstr,"LD_LIBRARY_PATH=%s",serplexpath);
		putenv(envstr);
		//Don't Daemonize - lets gis dump sip and h323 logs on terminal
#if 0
		// daemonize but don't forks !

		sigfillset(&newmask);
		sigprocmask(SIG_UNBLOCK, &newmask, 0);
		sigignore( SIGTTOU );
		sigignore( SIGTTIN );
		// close down all parents file descriptors

		nfds = getdtablesize(); // get max # of filedescriptors

		for (fd = 0; fd < nfds; ++fd)
				(void) close(fd);

		//setpgrp();

		//setsid();               // detach from the terminal

		umask(0);               // clear our umask
#endif
		p = strtok(ps->cmdline," ");
        i = 0;
        while(p)
        {
                argv[i++] = strdup(p);
                p = strtok(NULL," ");
        }
		argv[i] = NULL;
		//	execl(ps->cmdline,ps->psname,NULL);
		chdir(serplexpath);
		execv(argv[0],argv);
		ERROR(MPMGR,("Exec Failed %s for \"%s\"\n",strerror(errno), argv[0]));
		exit(-1);
	}
	else
	{
		DEBUG(MPMGR,NETLOG_DEBUG1,
			("started %s cmdline = %s pid = %d\n",ps->psname,ps->cmdline,pid));
	}

	return 0;
}

int
RSDConfigured(void)
{
	int match;
	int oldCST;
	serplex_config *rsd = NULL;

	oldCST = myConfigServerType;
	myConfigServerType = CONFIG_SERPLEX_RSD;
	match = FindServerConfig();
	myConfigServerType = oldCST;

	if (match != -1) {
		rsd = &serplexes[match];
	} 

	if (rsd == NULL)
		RSDConfig = CONFIG_LOCATION_NONE; 
	else
		RSDConfig = rsd->location.type;

	return(RSDConfig);
}

#ifndef NETOID_LINUX
static int wait_for_coredump (char *pname)
{
	kvm_t	*kd; 	
	struct proc	*cur_proc;
	struct user	*cur_user;
	struct pid	pid_tmp;
	int	nread;
	int	cur_pid;

	kd = kvm_open (NULL, NULL, NULL, O_RDONLY, NULL);
	if (!kd)
	{
		NETERROR(MPMGR, ("kvm_open returned error\n"));
		return (FALSE);
	}

	while (cur_proc = kvm_nextproc (kd))
	{

		if (cur_user =  kvm_getu (kd, cur_proc))
		{
			if (strcmp(pname, cur_user->u_comm) == 0)
				break;
		}
		else
		{
			NETERROR(MPMGR, ("kvm_getu returned error\n"));
		}
	}

	if (cur_proc)
	{
		if ((cur_proc->p_flag & SDOCORE) || 
			(cur_proc->p_flag & COREDUMP))
		{
			nread = kvm_read (kd, (uintptr_t) cur_proc->p_pidp, 
								(void *)&pid_tmp, sizeof (pid_tmp)); 

			kvm_close (kd);

			if (nread != sizeof (pid_tmp))
			{
				DEBUG (MPMGR, NETLOG_DEBUG1, 
						("kvm_read failed nread = %d\n", nread));

				return (FALSE);
			}

			cur_pid = pid_tmp.pid_id;

			while (sigsend (P_PID, cur_pid, 0) == 0)
			{
				DEBUG (MPMGR, NETLOG_DEBUG4, 
					("%s (%d) dumping core, sleeping 1 sec\n", 
					pname, cur_pid));

				sleep (1);
			}

			if (errno != ESRCH)
			{
				NETERROR (MPMGR, ("sigsend pid = %d errno = %d\n", 
					cur_pid, errno));

				return (FALSE);
			}

		}
		else
		{

			kvm_close (kd);
			return (FALSE); 
		}
		
	}
	else
	{

		kvm_close (kd);
		return (FALSE);
	}	
		
	
	return (TRUE);
}

#else

struct linux_proc {
	int pid; // %d
	char comm[400]; // %s
	char state; // %c
	int ppid; // %d
	int pgrp; // %d
	int session; // %d
	int tty; // %d
	int tpgid; // %d
	unsigned int flags; // %u
	unsigned int minflt; // %u
	unsigned int cminflt; // %u
	unsigned int majflt; // %u
	unsigned int cmajflt; // %u
	int utime; // %d
	int stime; // %d
	int cutime; // %d
	int cstime; // %d
	int counter; // %d
	int priority; // %d
	unsigned int timeout; // %u
	unsigned int itrealvalue; // %u
	int starttime; // %d
	unsigned int vsize; // %u
	unsigned int rss; // %u
	unsigned int rlim; // %u
	unsigned int startcode; // %u
	unsigned int endcode; // %u
	unsigned int startstack; // %u
	unsigned int kstkesp; // %u
	unsigned int kstkeip; // %u
	int signal; // %d
	int blocked; // %d
	int sigignore; // %d
	int sigcatch; // %d
	unsigned int wchan; // %u
};


static int wait_for_coredump(char *pname)
{
	DIR *fd_proc;
	struct dirent *dirp;
	int fd_stat, cur_pid,cnt;
	char buf[1000], pdir[50], statdir[50],procname[50];
	struct linux_proc pinfo;
	
	/*Get pid from process name*/
	sprintf(pdir,"/proc/");
        if((fd_proc=opendir(pdir))<0){
		return FALSE;
	}
	while( (dirp = readdir(fd_proc) ) != NULL){
		if(atoi(dirp->d_name) <= 0)
			continue;
		sprintf(statdir,"/proc/%s/stat",dirp->d_name);
		if( ( fd_stat = open(statdir,O_RDONLY) ) < 0 )
		{
			NETERROR (MPMGR,( "Unable to open stat = %s\n", dirp->d_name));
			return(FALSE);
		}
		
		cnt=read(fd_stat,buf,1000);
		if(cnt>0){
	        buf[cnt]='\0';
	        sscanf(buf,
			"%d %s %c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
			&pinfo.pid, // %d
			pinfo.comm, // %s
			&pinfo.state, // %c
			&pinfo.ppid, // %d
			&pinfo.pgrp, // %d
			&pinfo.session, // %d
			&pinfo.tty, // %d
			&pinfo.tpgid, // %d
			&pinfo.flags, // %u
			&pinfo.minflt, // %u
			&pinfo.cminflt, // %u
			&pinfo.majflt, // %u
			&pinfo.cmajflt, // %u
			&pinfo.utime, // %d
			&pinfo.stime, // %d
			&pinfo.cutime, // %d
			&pinfo.cstime, // %d
			&pinfo.counter, // %d
			&pinfo.priority, // %d
			&pinfo.timeout, // %u
			&pinfo.itrealvalue, // %u
			&pinfo.starttime, // %d
			&pinfo.vsize, // %u
			&pinfo.rss, // %u
			&pinfo.rlim, // %u
			&pinfo.startcode, // %u
			&pinfo.endcode, // %u
			&pinfo.startstack, // %u
			&pinfo.kstkesp, // %u
			&pinfo.kstkeip, // %u
			&pinfo.signal, // %d
			&pinfo.blocked, // %d
			&pinfo.sigignore, // %d
			&pinfo.sigcatch, // %d
			&pinfo.wchan // %u
			);
			sprintf(procname,"(%s)",pname);
			if( !strcmp( pinfo.comm, procname))
			{

				break;
			}

		}
			close(fd_stat);
	}
		closedir(fd_proc);
		if(dirp != NULL){
			cur_pid=pinfo.pid;
			if(pinfo.flags & PF_DUMPCORE){
				while(kill(cur_pid,0) == 0){
				DEBUG (MPMGR, NETLOG_DEBUG4,( "%s (%d) dumping core, sleeping 1 sec\n",	pname, cur_pid));
				sleep (1);
				}
		return(TRUE);
			}
		}
	return(FALSE);
 }		
#endif
