/*
 * signals.c
 * Medhavi, 2/99.
 * Default signal handling
 */

#include <pthread.h>

#include <signal.h>

/* Block all signals, and store the old set in old.
 * Note, we dont try to complicate by remembering the old actions
 * associated with the old sigset. 
 */

int
FillAllSignals(sigset_t *all)
{
	sigemptyset(all);
	sigfillset(all);

	return(0);
}
	
int
BlockAllSignals(sigset_t *new, sigset_t *old)
{
	sigemptyset(old);

	pthread_sigmask( SIG_BLOCK, new, old );
		
	return 0;
}

int
BlockCommonSignals(sigset_t *old)
{
	 sigset_t n_signal_mask;

	 sigemptyset(&n_signal_mask);
	 sigaddset(&n_signal_mask, SIGHUP);
	 sigaddset(&n_signal_mask, SIGCHLD);
	 pthread_sigmask(SIG_BLOCK, &n_signal_mask, old);

	return 0;
}

/* Unblock all the signals, and return pending ones.
 * Note - its upto application to re-install the signal handlers,
 * for the signals it wants to catch. We return the signals which arrived
 * while blocked (if pending is set to a non-null value).
 */
int
UnblockAllSignals(sigset_t *new, sigset_t *pending)
{
	sigfillset(new);
	pthread_sigmask(SIG_UNBLOCK, new, 0);
	return 0;
}

/* Useful for converting the examining process, 
 * into the normal callback process
 */
int
ExamineSignals(sigset_t *pending, int min, int max, void(*handler)(int))
{
	int i;

	/* Examine all signals, easy way w/o
	 * listing all signals. Usually the system uses, 
	 */

	if (pending != (sigset_t *)0)
	{
		sigemptyset(pending);
		sigpending(pending);
	}

	if (min > max)
	{
		return -1;
	}

	for (i=min; i<max; i++)
	{
		if (sigismember(pending, i))
		{
			handler(i);
		}				
	}

	return 0;
}

void ( *Signal( int signo, void(*disp)(int) ) )(int)
{
	struct sigaction        act, oact;

	act.sa_handler = disp;

	// Mask represents additional signals that
	// should be blocked while in the signal
	// handler

	sigemptyset(&act.sa_mask);

	act.sa_flags = 0;

	if (signo == SIGALRM) 
	{
		#ifdef  SA_INTERRUPT
			act.sa_flags |= (SA_INTERRUPT|SA_ONSTACK);   /* SunOS 4.x */
		#endif
	}
	else
	{
		#ifdef  SA_RESTART
			act.sa_flags |= (SA_RESTART|SA_ONSTACK);		/* SVR4, 44BSD */
		#endif
	}

	if ( sigaction( signo, &act, &oact) < 0 )
		return(SIG_ERR);

	return( oact.sa_handler );
}
