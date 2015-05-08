#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "ipc.h"
#include "ipcerror.h"
#include "srvrlog.h"
#include "serverp.h"
#include "lsconfig.h"
#include "ifs.h"
#include <malloc.h>
#include "pids.h"
#include "sconfig.h"

extern char 	pidfile[256];
serplex_config *redunds = NULL, *iserver = NULL;
static int (*ProcessConfig)(void);
struct ifi_info *ifihead = NULL;
int myConfigServerType = CONFIG_SERPLEX_NONE;

void
sig_int(int signo)
{
	if (signo != SIGINT)
	{
		return;
	}

	UnlinkPid(pidfile);

	exit(0);
}

void
sig_hup(int signo)
{
	if (signo != SIGHUP)
	{
		return;
	}

	NetLogClose();

	UH323ResetLogging();
	SipResetLogging();

	DoConfig(ProcessConfig);
	
	DEBUG(MINIT, NETLOG_DEBUG1, ("Reconfiguring\n"));
}

void
sig_chld(int signo)
{
	pid_t pid;
	int stat;

	if (signo != SIGCHLD)
	{
		return;
	}

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
	{
		NETDEBUG( MSEL, NETLOG_DEBUG4, ("child %lu terminated\n", ULONG_FMT(pid)));

		/* Do some analysis on the pid which just departed */

		if (WIFSIGNALED(stat))
		{
			NETERROR(	MDEF, ("Child process %lu exited due to signal %d\n",
						ULONG_FMT(pid), WTERMSIG(stat)));
		}
	}
	return;
}

int
DoConfig(int (*_ProcessConfig)(void))
{
	InitVars();
	InitConfig();

	ProcessConfig = _ProcessConfig;

	if (sconfig_parse_config(config_file) != 0)
	{
		printf("Configuration not readable... Using Defaults\n");
		InitVars();
		InitConfig();
	}
	else
	{
		if (ProcessConfig)
		{
			ProcessConfig();
		}
	}
	
	return 0;
}

int
ServerSetLogging(char *name, serplex_config *s)
{
	/* Set the logging */
	if (s->debconfigs.flags & NETLOG_ASYNC)
	{
		NetSyslogOpen(name, NETLOG_ASYNC);
	}

	netLogStruct.flags = s->debconfigs.flags;
	memcpy(&NetLogStatus[0], &s->debconfigs.netLogStatus[0],
		MNETLOGMAX);
	return(0);
}

int
FindServerConfig(void)
{
	int i, match = -1;
	
	redunds = NULL;

	/* Process Configuration, read from the config file */

	for (i=0; i<inserver; i++)
	{
		if (serplexes[i].location.type == 
			CONFIG_LOCATION_NONE)
		{
			continue;
		}

		if (serplexes[i].location.type == CONFIG_LOCATION_INADDR)
		{
			/* We must determine if we are the ip address mentioned
		 	* here
		 	*/
			if (ifihead && 
					(matchIf(ifihead, 
						serplexes[i].location.address.sin_addr.s_addr) == NULL))
			{
				if (serplexes[i].type == myConfigServerType)
				{
					/* This is probably a redundant guy */
					serplexes[i].flags |= CONFIG_SERPLEX_REDUNDF;
					redunds = &serplexes[i];
				}

				continue;
			}
			else
			{
				/* Change location type to local */
				serplexes[i].location.type = CONFIG_LOCATION_LOCAL;
		
				/* If Ageing daemon is configured to run,
				 * set its location too.
				 */
				if (serplexes[i].age.location.type != 
					CONFIG_LOCATION_NONE)
				{
					serplexes[i].age.location.type = 
						CONFIG_LOCATION_LOCAL;
				}

				if (serplexes[i].type == myConfigServerType)
				{
					match = i;
				}
			}
		}
		else if (serplexes[i].location.type == CONFIG_LOCATION_LOCAL)
		{
			/* If Ageing daemon is configured to run,
			 * set its location too.
			 */
			if (serplexes[i].age.location.type != 
				CONFIG_LOCATION_NONE)
			{
				serplexes[i].age.location.type = 
					CONFIG_LOCATION_LOCAL;
			}

			if (serplexes[i].type == myConfigServerType)
			{
				match = i;
			}
		}
	}
		
	return match;
}

int
UH323ResetLogging(void)
{
	if (_msSetDebugLevel)
		_msSetDebugLevel(0);

	if (_msDeleteAll)
		_msDeleteAll();
	return(0);
}

int
SipResetLogging(void)
{
	if (_sipSetTraceLevel)
	{
		_sipSetTraceLevel(0);
	}
	return(0);
}

void 
setConfigFile(void)
{
	/* 
		Test for the environment variable SERPLEXPATH and use it 
		if it exists. If it doesnt, then the default value will stand.
	*/
	char *path = getenv("SERPLEXPATH");
	if (path)
	{
		strcpy(config_file,path);
		strcat(config_file,CONFIG_FILENAME);
	}
}
