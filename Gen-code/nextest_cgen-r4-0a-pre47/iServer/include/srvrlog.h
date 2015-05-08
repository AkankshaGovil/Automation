#ifndef _srvrlog_h_
#define _srvrlog_h_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>


#include <signal.h>

#include "systemlog.h"

#define NETLOG_MAXLINE          1024

/* If a module is added here, remember to add its
 * printable string name in netlog.c
 */
typedef enum
{
	MDEF,
	MREGISTER,
	MFIND,
	MAGE,
	MCACHE,
	MINIT,
	MSEL,
	MPKT,
	MDB,
	MSHM,
	MCDR,
	MFAXP,
	MCONN,
	MTMR,
	MRED,
	MXML,
	MCLI,
	MLMGR,
	MPMGR,
	MH323,
	MLRQ,
	MRRQ,
	MARQ,
	MSIP,
	MQ931,
	MSCC,
	MIWF,
	MBRIDGE,
	MFCE,
	MRADC,
	MISPD,
	MRSD,
	MDLIC,
	MIRQ,
	MICMPD,
	MEXECD,
	MSCM,
	MSCMRPC,
	
	MNETLOGMAX,
	MQUEDB,
} NetLogModules;

typedef enum
{
  TPKTCHAN,
  UDPCHAN,
  PERERR,
  CM,
  CMAPICB,
  CMAPI,
  CMERR,
  LI,
  LIINFO,
  CUSLOGMAX
} CusLogModules;

extern int NetLogSysPri[NetLogMaxTypes];
extern unsigned char NetLogStatus[MNETLOGMAX];
extern char NetLogModuleNames[MNETLOGMAX][10];

extern int NetSyslogOpen( char *name, unsigned short flags );
extern char *NetLogSprintf(char *fmt, ...);
extern int NetLogDebugSprintf(char *fmt, ...);
extern int NetLogErrorSprintf(char *fmt, ...);
extern int NetLogCdrSprintf(char *fmt, ...);

#define NETLOG(_pri_, _mod_, _sep_, _logm_) \
	{ \
		char *_s_, _p_[NETLOG_MAXLINE];\
		if (netLogStruct.flags & NETLOG_ASYNC) \
		{ \
			switch (_pri_) { \
			case NetLogDebug: \
				NetLogDebugSprintf _logm_; \
				break; \
			case NetLogError: \
				NetLogErrorSprintf _logm_; \
				break; \
			case NetLogCdr: \
				NetLogCdrSprintf _logm_; \
				break; \
			default: \
				_s_ = NetLogSprintf _logm_; \
				snprintf(_p_, NETLOG_MAXLINE, "%s: %s", _mod_, _s_); \
				_p_[NETLOG_MAXLINE-1] = '\0'; \
				syslog(NetLogSysPri[_pri_], "%s", _p_); \
				break; \
			} \
		} \
		else if (netLogStruct.flags & NETLOG_TERMINAL) \
		{ \
			printf("%s%s", _mod_, _sep_); \
			printf _logm_; \
		} \
		else if (netLogStruct.flags & NETLOG_CONSOLE) \
		{ \
		} \
	}

#endif /* _srvrlog_h_ */
