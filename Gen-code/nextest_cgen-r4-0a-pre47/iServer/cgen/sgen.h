#ifndef _sgen_h_
#define _sgen_h_

#include <sys/types.h>
#include <pthread.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>

#include <sys/poll.h>
#include "cv.h"
#include <sys/syscall.h>
#include <netinet/tcp.h>

#ifndef NETOID_LINUX
#include <sys/processor.h>
#include <sys/procset.h>
#include <sys/lwp.h>
#include <sys/corectl.h>
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#endif //NETOID_LINUX

#define MODE_TRANSMIT		0x1
#define MODE_RECEIVE		0x2
#define MODE_MGCP			0x4
#define MODE_ITERATIVE		0x8
#define MODE_REGISTER		0x10
#define MODE_IXIA		0x20


// Invite types
#define        	INVITE_THRU_OBP  0x1
#define			INVITE_REGULAR  0x2
#define         INVITE_NOSDP  0x8
#define        	INVITE_URI_DIAL  0x10

// Hold types
#define         INVITE_HOLD  0x1
#define 		HOLD_NONE 0x2
#define         INVITE_HOLD_3264  0x4

//Refer types
#define			BLIND_XFER		0
#define			ATTENDED_XFER	1

// Reinvite types
typedef enum
{
        REINVITE_TYPE_NEWPORT,
        REINVITE_TYPE_NOSDP,
        REINVITE_TYPE_SAMEPORT,
        REINVITE_TYPE_HOLD,
        REINVITE_TYPE_HOLD_3264,
        REINVITE_TYPE_RESUME
} reinviteTypes;



typedef struct
{
	unsigned long				ip;		// RTP ip value for the channel
	int				port;		// RTP port value for the channel
	int				chNum;
	int				mgenFd;
} Channel;

typedef struct _call
{
	struct _call	*prev, *next;
	int				cNum;
	int				success;
	int 			state;
	time_t			tsOriginate;	// TCP connection was originated
	time_t			tsUse;			// Call Entry was used up
	time_t			tsSetup;		// Setup was originated
	time_t			tsProceeding;
	time_t			tsRingBack;
	time_t			tsConnect;
	time_t			tsDrop;			// Call was dropped
	time_t			tsIdle;			// Call went to idle state
	int				ntimes;			// no of times call make was called
	int				dofax;
	time_t			timeNextEvt;	// Duration timer
	Channel			chIn[1];
	Channel			chOut[1];
} Call;

#endif /* _sgen_h_ */
