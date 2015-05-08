
/*
 * daemon.c
 *
 *	Copyright 1998, Netoids Inc.
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "ipc.h"

/*
 *********************************************
 * Function: int daemonize (void)
 *
 * Description:  Daemonizes a process.
 *********************************************
 */
int
daemonize (void)
{
	pid_t	pid;

	if ( (pid = fork()) < 0 )
	{
		perror ("Error in forking:");
		return -1;
	}
	else if (pid != 0)
		exit (0);	/* for the parent */

	/* Child chugs on... */
	setpgrp();

	setsid ();	/* become session leader */

	// Take on the shell/parent umask - umask(0) taken out

	return (0);
}
