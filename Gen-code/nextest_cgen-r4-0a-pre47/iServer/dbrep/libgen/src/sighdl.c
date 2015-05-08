#include "unp.h"

void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ((pid = waitpid((pid_t)-1, &stat, WNOHANG)) > 0)
		fprintf(stderr, "Child terminated\n");
	return;
}
