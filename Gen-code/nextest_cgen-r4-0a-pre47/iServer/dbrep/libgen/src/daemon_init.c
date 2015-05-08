#include	"unp.h"
#include	<syslog.h>

#define	MAXFD	64

extern int	daemon_proc;	/* defined in error.c */

void
daemon_init(int fdflag)
{
	int		i, minfd;
	pid_t	pid;

	if (fdflag == 0) {
		minfd = 0; 		/* Close all descriptors */
	}
	else {
		minfd = 2;			/* Leave stdin. stdout. stderr open */
	}

	if ( (pid = Fork()) != 0)
		exit(0);			/* parent terminates */

	/* 41st child continues */
	setsid();				/* become session leader */

	RSSignal(SIGHUP, SIG_IGN);
	if ( (pid = Fork()) != 0)
		exit(0);			/* 1st child terminates */

	/* 42nd child continues */
	daemon_proc = 1;		/* for our err_XXX() functions */

	chdir("/");				/* change working directory */

	umask(0);				/* clear our file mode creation mask */

#ifndef _DMALLOC_
	for (i = minfd; i < MAXFD; i++)
		close(i);
#endif

//	openlog(pname, LOG_PID, facility);
}
