#ifndef _systemlog_h_
#define _systemlog_h_

#ifdef SYSTEM_UNIX
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

/* Note that CONSOLE, when used basically will use the local syslog.
 * The local syslog may point to a remote syslogd, in turn achieving the
 * same effect as log 'A.B.C.D'
 */
#define NETLOG_ASYNC		0x0001		/* Corresponds to syslog */
#define NETLOG_RAM			0x0002		/* Client */
#define NETLOG_CONSOLE		0x0004		/* Client */
#define NETLOG_TERMINAL		0x0008		/* Server */
#define NETLOG_FILE			0x0010		/* Server */
#define NETLOG_SIPTERMINAL	0x0020		/* SIP logging */	
#define NETLOG_H323TERMINAL	0x0040		/* H.323 logging */
#define NETLOG_SIPSYSLOG	0x0080		/* SIP logging */	

typedef struct
{
	int 			rfd;		/* Remote's fd */
	struct sockaddr_in 	remote;		/* Remote addr */
	unsigned short		nbuffs;		/* Max Queue Len */
	void			*memaddr;	/* If Async start */
	unsigned short		flags;		
	int			cleanup;	/* Read by the log task
						 * On SIGHUP
						 */
} NetLogStruct;

extern NetLogStruct netLogStruct;

typedef enum
{
	NetLogDebug = 0,
	NetLogError,
	NetLogAlarm,
	NetLogInfo,
	NetLogCdr,		/* CDR logging is special */
	
	NetLogMaxTypes,
} NetLogType;
	
/* Must be defined by the user app */
extern unsigned char NetLogStatus[];
extern char NetLogModuleNames[][10];

#define NETLOG_DEBUGLEVELMASK	0x0f
#define NETLOG_DEBUGMASK	0x10
#define NETLOG_ERRORMASK	0x20
#define NETLOG_ALARMMASK	0x40
#define	NETLOG_HEXDUMPMASK	0x80

#define NETLOG_DEBUG1		0x01
#define NETLOG_DEBUG2		0x02
#define NETLOG_DEBUG3		0x04
#define NETLOG_DEBUG4		0x08

#define NETLOG_SETLEVELE(module, level) \
				(NetLogStatus[module] |= (level))

#define NETLOG_SETLEVEL(module, level) \
				NETLOG_SETLEVELE(module, level|(level -1))

#define NETLOG_RESETLEVELE(module, level) \
				(NetLogStatus[module] &= ~(level))

#define NETLOG_RESETLEVEL(module, level) \
				NETLOG_RESETLEVELE(module, level|(level -1))

#ifndef DEBUG
#ifndef NODEBUGLOG
#define DEBUG(module, level, dmsg) do { \
	if (NetLogStatus[module] & level) \
		NETLOG(	NetLogDebug, \
			NetLogModuleNames[module], \
			": ", \
			dmsg) \
	} while (0)
#else
#define DEBUG(x, y, z)
#endif
#endif

#ifndef NODEBUGLOG
#define NETDEBUG(module, level, dmsg) do { \
	if (NetLogStatus[module] & level) \
		NETLOG(	NetLogDebug, \
			NetLogModuleNames[module], \
			": ", \
			dmsg) \
	} while (0)
#else
#define NETDEBUG(x, y, z)
#endif

#ifndef ERROR
#define ERROR(module, dmsg) \
		NETLOG(	NetLogError, \
			NetLogModuleNames[module], \
			": ", \
			dmsg)
#endif

#define NETERROR(module, dmsg) \
		NETLOG(	NetLogError, \
			NetLogModuleNames[module], \
			": ", \
			dmsg)
	
#ifndef INFOMSG
#define INFOMSG(module, dmsg) \
		NETLOG(	NetLogInfo, \
			NetLogModuleNames[module], \
			": ", \
			dmsg)
#endif

#define NETINFOMSG(module, dmsg) \
		NETLOG(	NetLogInfo, \
			NetLogModuleNames[module], \
			": ", \
			dmsg)


#define PERROR(s) \
		NETERROR(MDEF, ("%s: - SYSTEM ERROR 0x%x\n", s, errno))

#ifndef ALARM
#define ALARM(module, dmsg) \
		NETLOG(	NetLogAlarm, \
			NetLogModuleNames[module], \
			": ", \
			dmsg)
#endif

#define NETALARM(module, dmsg) \
		NETLOG(	NetLogAlarm, \
			NetLogModuleNames[module], \
			": ", \
			dmsg)
		
#define HEXDUMP(module, buffer)

#define NETCDR(module, level, dmsg) do { \
	if (NetLogStatus[module] & level) \
		NETLOG(	NetLogCdr, \
			NetLogModuleNames[module], \
			": ", \
			dmsg) \
	} while (0)

int	NetLogInit(void);
int NetLogOpen(struct sockaddr_in *remote, int nbuffs, unsigned short flags);
int NetLogModuleId(char *modname);
int NetLogClose(void);

#define IsModuleLevelOn(module, level) (NetLogStatus[module] & level)

#endif /* _systemlog_h_ */
