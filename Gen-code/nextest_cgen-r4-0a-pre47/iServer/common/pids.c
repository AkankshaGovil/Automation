#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "pids.h"
#include "nxosd.h"

/* Finds the pid. If the pid does not exist,
 * returns a -1 / -2. 
 */
int
ReadPid(char *pathname)
{
	FILE *fptr;
	int mypid;

	/* Store my pid in the pathname */
	
	if ((fptr = fopen(pathname, "r")) == NULL)
	{
		return -2;
	}
	
	if (fscanf(fptr, "%d", &mypid) < 1)
	{
		fclose(fptr);	
		return -1;
	}

	fclose(fptr);	

	return mypid;
}

int
StorePid(char *pathname)
{
	FILE *fptr;

	/* Store my pid in the pathname */
	
	if ((fptr = fopen(pathname, "w")) == NULL)
	{
		fprintf(stderr, "Could not save off pid in %s\n", pathname);
		return -1;
	}

	fprintf(fptr, "%lu\n", ULONG_FMT(getpid()));

	fflush(fptr);

	fclose(fptr);	
	return( 0 );
}

int
UnlinkPid(char *pathname)
{
	if (unlink(pathname) < 0)
	{
		if (errno != ENOENT)
		{
			perror("unlink");
		}
	}
	return( 0 );
}	
