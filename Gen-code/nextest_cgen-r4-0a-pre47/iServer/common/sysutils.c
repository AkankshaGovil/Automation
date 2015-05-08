#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sysutils.h"
#include "srvrlog.h"
#include "threads.h"


//
// Free memory if the pointer and zero the pointer.
//
// NOTE :  	You are explicitly allowed to pass NULL pointers --
//			they will always be ignored.
//

#define SAFE_FREE(x) do { if ((x) != NULL) {free(x); x=NULL;} } while(0)

//
//	Function :
//		extract_args()
//
//  Arguments       :
//
//		command		command string which is to be converted to a
//					an argument list that can be used by execv(2).
//
//	Description :
//		This routine is a local utility function used by
//		the sys_popen() function defined below. The function
//		creates an argument list which can be used by the execv(2)
//		system call.
//
//	Return Values:
//		char **		pointer to an argument list array for use
//					with execv.
//
static char **
extract_args( const char *command )
{
	static char		trunc_cmd[ 1024 ];
	char *			ptr;
	int				argcl;
	char **			argl = NULL;
	int				i;

	memset( trunc_cmd, (int) 0, 1024 );
	strcpy( trunc_cmd, command );

	if( !( ptr = strtok(trunc_cmd, " \t" ) ) )
	{
		errno = EINVAL;
		return NULL;
	}

	// Count the args.

	for( argcl = 1; ptr; ptr = strtok(NULL, " \t"))
		argcl++;

	if( ( argl = (char **)malloc( (argcl + 1) * sizeof(char *) ) ) == NULL)
		return( NULL );

	// Now do the extraction.

	strcpy( trunc_cmd, command );

	ptr = strtok( trunc_cmd, " \t" );
	i = 0;
	argl[i++] = ptr;

	while( ( ptr = strtok(NULL, " \t") ) != NULL )
		argl[i++] = ptr;

	argl[i++] = NULL;
	return( argl );
}

//
// Replacement for popen and pclose(). Safer as it 
// doesn't search a path. Modified from the glibc sources.
// modified by to return a file descriptor.
// We must kick our FILE* habit
//

typedef struct _popen_list
{
	int fd;
	pid_t child_pid;
	struct _popen_list *next;
} popen_list_t;

pthread_mutex_t			popen_lock;
static popen_list_t *	popen_chain;


//
//	Function :
//		sys_utils_init()
//
//  Arguments       :
//
//		None.
//
//	Description :
//		Performs initialization of sysutils functions.
//		Allocates mutex to protect popen_chain.
//
//	Return Values:
//		None.
//
void
sys_utils_init( void )
{

	if ( pthread_mutex_init( &popen_lock, NULL ) )
	{
		NETERROR(MISPD, ("INIT : pthread_mutex_init() failed for popen_lock\n"));
		exit(1);
	}
}

//
//	Function :
//		sys_popen()
//
//  Arguments       :
//
//		command		pointer to a command string to be executed by
//					the sys_popen() function.
//
//	Description :
//		This routine is a thread-safe version of the popen(3c)
//		function. The popen(3c) function causes a memory-leak if
//		used frequently in a threaded process implementation. 
//
//		NOTE :	The sys_popen() function uses file descriptors instead
//				of stream I/O, which is used in popen(3c)
//
//	Return Values:
//		fd		a file descriptor to the child process which
//				is invoking the command specified in string.
//
//		-1		on failure.
//
int
sys_popen( const char *command, int exec_shell )
{
	int parent_end, child_end;
	int pipe_fds[2];
	popen_list_t *	entry = NULL;
	char **argl = NULL;

	if ( pipe( pipe_fds ) < 0 )
		return( -1 );

	parent_end = pipe_fds[0];
	child_end = pipe_fds[1];

	if (!*command)
	{
		errno = EINVAL;
		goto err_exit;
	}

	if ((entry = (popen_list_t *) malloc(sizeof(popen_list_t))) == NULL)
		goto err_exit;

	memset( entry, (int) 0, sizeof( popen_list_t ) );

	// Extract the command and args into a NULL terminated array.

	if (exec_shell)
	{
		if( ( argl = (char **)malloc( 4 * sizeof(char *) ) ) == NULL)
			goto err_exit;
		argl[0] = "/bin/sh";
		argl[1] = "-c";
		argl[2] = (char *)command;
		argl[4] = NULL;
	}
	else if ( !( argl = extract_args(command) ) )
		goto err_exit;

	entry->child_pid = fork();

	if ( entry->child_pid == -1 )
	{
		goto err_exit;
	}

	if (entry->child_pid == 0)
	{
		// Child !

		int				child_std_end = STDOUT_FILENO;
		popen_list_t *	p;

		close( parent_end );

		if (child_end != child_std_end)
		{
			dup2(child_end, child_std_end);
			close(child_end);
		}

		// POSIX.2:  "popen() shall ensure that any streams from previous
		// popen() calls that remain open in the parent process are closed
		// in the new child process."

		for ( p = popen_chain; p; p = p->next )
			close( p->fd );

		execv( argl[0], argl );

		_exit(127);
	}

	// Parent.

	close(child_end);
	SAFE_FREE( argl );

	lock_mutex( &popen_lock );

	// Link into popen_chain. 

	entry->next = popen_chain;
	popen_chain = entry;
	entry->fd = parent_end;

	unlock_mutex( &popen_lock );

	return( entry->fd );

err_exit:

	SAFE_FREE(entry);
	SAFE_FREE(argl);
	close(pipe_fds[0]);
	close(pipe_fds[1]);
	return( -1 );
}

//
//	Function :
//		sys_pclose()
//
//  Arguments       :
//
//		fd			file descriptor returned from sys_popen()
//					function call.
//
//	Description :
//		This routine is a thread-safe version of the pclose(3c)
//		function. sys_pclose() function frees up the popen_list_t 
//		entry on the local chain anchored at the static popen_chain.
//		sys_pclose() then closes the file descriptor to the child,
//		and waits for the child to die before returning.
//
//	Return Values:
//		wait_pid	waitpid status.
//
//		-1			on error.
//
int
sys_pclose(int fd)
{
	int				wstatus;
	popen_list_t **	ptr = &popen_chain;
	popen_list_t *	entry = NULL;
	pid_t			wait_pid;
	int				status = -1;

	// Unlink from popen_chain. 

	lock_mutex( &popen_lock );

	for (; *ptr != NULL; ptr = &(*ptr)->next)
	{
		if ((*ptr)->fd == fd)
		{
			entry = *ptr;
			*ptr = (*ptr)->next;
			status = 0;
			break;
		}
	}

	unlock_mutex( &popen_lock );

	if (status < 0 || close(entry->fd) < 0)
	{
		NETERROR(MISPD, ("SYS  : sys_pclose() error - popen child not found on chain\n"));
		return( -1 );
	}

	// As Process is catching and eating child process
	// exits we don't really care about the child exit
	// code, a -1 with errno = ECHILD will do fine for us.

	do
	{
		wait_pid = waitpid( entry->child_pid, &wstatus, 0 );
	}
	while ( wait_pid == -1 && errno == EINTR );

	SAFE_FREE(entry);

	if ( wait_pid == -1 )
	{
		NETERROR(MISPD, ("SYS  : sys_pclose() error - waitpid error\n"));
		return( -1 );
	}

	return( wstatus );
}
