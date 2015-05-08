#include "srvrlog.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

NetLogStruct netLogStruct;
unsigned char NetLogStatus[MNETLOGMAX];

int NetLogSysPri[NetLogMaxTypes] = 
{ 
	LOG_LOCAL1|LOG_DEBUG,
	LOG_LOCAL1|LOG_ERR,
	LOG_LOCAL1|LOG_INFO, 
	LOG_LOCAL1|LOG_NOTICE,
	LOG_LOCAL2|LOG_DEBUG
};

char NetLogModuleNames[MNETLOGMAX][10] = {
	"MDEF",
	"MREGISTER",
	"MFIND",
	"MAGE",
	"MCACHE",
	"MINIT",
	"MSEL",
	"MPKT",
	"MDB",
	"MSHM",
	"MCDR",
	"MFAXP",
	"MCONN",
	"MTMR",
	"MRED",
	"MXML",
	"MCLI",
	"MLMGR",
	"MPMGR",
	"MH323",
	"MLRQ",
	"MRRQ",
	"MARQ",
	"MSIP",
	"MQ931",
	"MSCC",
	"MIWF",
	"MBRIDGE",
	"MFCE",
	"MRADC",
	"MISPD",
	"MRSD",
	"MDLIC",
	"MIRQ",
	"MICMPD",
	"MEXECD",
	"MSCM",
	"MSCMRPC",
};

int
NetLogModuleId(char *modname)
{
	int id;
	
	for (id = 0; id < MNETLOGMAX; id ++)
	{
		if (strcmp(modname, NetLogModuleNames[id]) == 0)
		{
			return id;
		}
	}

	return -1;
}

int
NetLogInit(void)
{
	memset(&NetLogStatus[0], 0, MNETLOGMAX);
	memset(&netLogStruct, 0, sizeof(NetLogStruct));
	
	return 0;
}

int
NetLogOpen(struct sockaddr_in *remote, 
		int nbuffs, unsigned short flags)
{
	if (flags & NETLOG_ASYNC)
	{
		uint32_t facility = (NetLogSysPri[0] & ~LOG_PRIMASK);

		/* Open the log */

		setlogmask( LOG_UPTO(LOG_DEBUG) );

		openlog( "", LOG_PID|LOG_NDELAY, facility );

		setlogmask( LOG_UPTO(LOG_DEBUG) );
	}
	else
	{
		closelog();
	}

	netLogStruct.flags = flags;
	return( 0 );
}

int
NetSyslogOpen(char *name, unsigned short flags)
{
	if (flags & NETLOG_ASYNC)
	{
		uint32_t facility = (NetLogSysPri[0] & ~LOG_PRIMASK);

		setlogmask( LOG_UPTO(LOG_DEBUG) );

		/* Open the log */

		setlogmask( LOG_UPTO(LOG_DEBUG) );

		openlog(name, LOG_PID|LOG_NDELAY, facility );

		setlogmask( LOG_UPTO(LOG_DEBUG) );
	}
	else
	{
		closelog();
	}

	netLogStruct.flags = flags;

	return( 0 );
}

int
NetLogClose(void)
{
	if ( !(netLogStruct.flags & NETLOG_ASYNC))
	{
		closelog();
	}
	return( 0 );
}

char *
NetLogSprintf(char *fmt, ...)
{
	static char s[NETLOG_MAXLINE];

	va_list ap;

	va_start(ap, fmt);

	vsnprintf(s, sizeof(s), fmt, ap);

	s[NETLOG_MAXLINE-1] = '\0';

	va_end(ap);

	return s;
}	

int
NetLogDebugSprintf(char *fmt, ...)
{
	char s[NETLOG_MAXLINE];
	uint32_t priority = ( NetLogSysPri[NetLogDebug] & LOG_PRIMASK );

	va_list ap;

	va_start( ap, fmt );

	vsnprintf( s, sizeof(s), fmt, ap );

	s[NETLOG_MAXLINE-1] = '\0';

	va_end(ap);

	syslog( priority, "%s", s);
	
	return 0;
}	

int
NetLogErrorSprintf(char *fmt, ...)
{
	char s[NETLOG_MAXLINE];
	uint32_t priority = ( NetLogSysPri[NetLogError] & LOG_PRIMASK );

	va_list ap;

	va_start(ap, fmt);

	vsnprintf(s, sizeof(s), fmt, ap);

	s[NETLOG_MAXLINE-1] = '\0';

	va_end(ap);

	syslog( priority, "%s", s);
	
	return 0;
}	

int
NetLogCdrSprintf(char *fmt, ...)
{
	char s[NETLOG_MAXLINE];
	/* For CDR's we use a differnt facility. The syslog's argument
	   priority can contain both facility and level. so we do not 
	   mask the facility by LOG_PRIMASK */
	//uint32_t priority = ( NetLogSysPri[NetLogCdr] & LOG_PRIMASK );
	uint32_t priority = ( NetLogSysPri[NetLogCdr] );

	va_list ap;

	va_start(ap, fmt);

	vsnprintf(s, sizeof(s), fmt, ap);

	s[NETLOG_MAXLINE-1] = '\0';

	va_end(ap);

	syslog( priority, "%s", s);
	
	return 0;
}	

void
NetLogH323printf(int type, const char *fmt, ...)
{
	char s[NETLOG_MAXLINE];
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(s, sizeof(s), fmt, ap);

	s[NETLOG_MAXLINE-1] = '\0';

	va_end(ap);

	printf(s);

	return;
}	

