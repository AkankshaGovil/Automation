#ifndef __POLL_H__
#define __POLL_H__
// #include "timer.h"



#define MAX_POLL_MISSES 3
#define START_TIME_OUT 45 /*Need larger time out to account for startup time */ 
#define POLL_TIME_OUT 15   
#define SERV_PORT 2101
#define LOCALHOST "127.0.0.1"
#define MAX_ENV_STR 120

#define ISERVER_NONE    0x0000
#define ISERVER_RESTART 0x0001
#define RSD_RESTART     0x0002

#define TRUE    0x0001
#define FALSE    0x0000

typedef enum {
	PM_ePsSyncStart,
	PM_ePsAsyncStart,
} PM_PsStartMode;

typedef enum {
	PM_ePsDisabled,
	PM_ePsEnabled,
} PM_PsAdminStatus;

typedef	struct {
		int     psindex;
		int		pid;
		int		poll;
		int		pollmissed;
		PM_PsStartMode startMode;
		char	psname[256];
		char	cmdline[256];
		char 	pidfile[256];
	    unsigned long startupTime;
		int 	independentlyRestartable;
		PM_PsAdminStatus adminStatus; /* enabled or disabled */
		char	stopcmd[256];
		int	relativePriority;
} PsEntry;

typedef struct {
	PsEntry 	*pslist;
	int			pscnt;	/* No. of processes in process list */
	char		stopcmd[512];
	char 		startcmd[512];	
	int			restarts;
	char 		grpName[256];
} MonitoredPsGrp;


typedef struct {
	int id;       	/* logical id of the process sending keepalive */
	int gid;        /* logical id of the process grp to which it belongs */
	int pid; 	/* unix pid of the process sendig keepalive */
	unsigned long startupTime;  /* time since epoch, when this server started */
  int restartid;
} KeepAlive;


extern int Initpoll(int myid,int grpid);
extern int Sendpoll(int t);
extern void setRestart(int restart);  
#endif

