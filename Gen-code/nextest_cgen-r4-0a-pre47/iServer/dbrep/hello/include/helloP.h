#ifndef __HELLO_H_
#define __HELLO_H_

#include "queue.h"
#include "hello_common.h"

#define HELLO_EVENTQ_LEN		5
#define HELLO_EVENT_SIZE		(sizeof(int))

#define	HSM_STATE_DOWN  		0
#define	HSM_STATE_NEGT			1
#define	HSM_STATE_UP			2
#define HSM_STATE_MAX			3

#define	HSM_EVENT_NONE			0
#define HSM_EVENT_START 		1
#define HSM_EVENT_FULLCONN		2
#define HSM_EVENT_NBRCHNG		3
#define HSM_EVENT_SHUTDOWN		4
#define	HSM_EVENT_MAX			5

#define NSM_STATE_DOWN 			0
#define	NSM_STATE_WAIT			1
#define	NSM_STATE_UNIDIR		2
#define	NSM_STATE_BIDIR			3
#define	NSM_STATE_MAX			4

#define	NSM_EVENT_NONE 			0	
#define	NSM_EVENT_START			1
#define	NSM_EVENT_SHUTDOWN		2
#define	NSM_EVENT_HELLORCVD		3
#define	NSM_EVENT_ONEWAYRCVD	4
#define	NSM_EVENT_TWOWAYRCVD	5
#define	NSM_EVENT_MAX			6

#define	LOCK_NSM	0
#define LOCK_HSM	1
#define	HELLO_NUM	2

typedef int (*PFIN)(nbr_info *);

int		publish_host_table(void);
int 		SendHello(struct Timer*);
int		NotifyHelloProt(void *data);
void	*HelloHandleTimers(void *arg);
int		HandleNotify(int fd, FD_MODE rw, void *data);
int		HelloSetupTimeout(struct timeval *tout);
int		HelloAdjustTimeout(struct timeval *tout, int *msec);
int		HelloProcessTimeout(void);
void 	hcpInit(void);
int		IpcInit(void);
int		HandleMulMsg(void);
int		HandleUniMsg(void);
static void	HelloProtMainLoop(void);
char	*GetNetPkt(int fd, struct sockaddr *sa, socklen_t salen);
int		PresentInNbrRec(nbr_info *ni);
int		AbsentInNbrRec(nbr_info *ni);
int		CheckNSMStates(void);
List	CreateRecordList(HPktHdr *hpktp);
int		CopyRecstoPkt(List reclist, char *pktp);
IPADDRESS 	UpdateSelfRec(void);
host_rec 	*cmp_prio(host_rec *item1, host_rec *item2);
int 	HostIsLeader(struct Timer*);
int 	LeaderAgreed(void);
int 	LeaderChanged(void);
void	exec_nsm_event(PFIN precond, nid nbrk, int event);
void	exec_hsm_event(int event);
int		hsm_event_cb(struct Timer *t);
void	sched_hsm_event(int event, int ms);
int		match_nbr_addr(const void *item1, const void *item2);
int		match_rec_self(const void *item1, const void *item2);
int		match_rec_nbr(const void *item1, const void *item2);
int		def_init_cb(void *, void *);
int		def_kill_cb(void *, void *);
int 	nsm_kill_cb(struct Timer *t);
int		HelloCreateLocks(void);
int		HelloDeleteLocks(void);
void 	HelloGetLock(int lockid);
void 	HelloRelLock(int lockid);
nid		sock_2_nid(struct sockaddr_in *);
char 	*nid_2_str(nid nbrk, char *str);
nbr_info	*get_nbr_info(nid nbrk);

/* Actions */
int 	hsm_ignore(void);
int 	hsm_init(void);
int 	hsm_kill(void);
int 	hsm_up(void);
int 	hsm_negt(void);

int 	nsm_ignore(nbr_info *);
int 	nsm_kill(nbr_info *);
int 	nsm_init(nbr_info *);
int 	nsm_oneway(nbr_info *);
int 	nsm_twoway(nbr_info *);
int 	nsm_hellorcvd(nbr_info *);

AppCB	default_reg = { def_init_cb, NULL, def_kill_cb, NULL};

struct {
	int (*action) (void);
	int next_state;
} HSM [HSM_STATE_MAX][HSM_EVENT_MAX] = 
{
	{	/* Down State */
		{ hsm_ignore	, HSM_STATE_DOWN	},	/* No Event */
		{ hsm_init		, HSM_STATE_NEGT	},	/* Start */
		{ hsm_ignore	, HSM_STATE_DOWN	},	/* FullConnected */
		{ hsm_ignore 	, HSM_STATE_DOWN	},	/* NbrChng */
		{ hsm_ignore	, HSM_STATE_DOWN	}	/* Shutdown */
	},
	{	/* Negotiating State */
		{ hsm_ignore	, HSM_STATE_NEGT	},	/* No Event */
		{ hsm_ignore	, HSM_STATE_NEGT	},	/* Start */
		{ hsm_up		, HSM_STATE_UP		},	/* FullConnected */
		{ hsm_ignore	, HSM_STATE_NEGT	},	/* NbrChng */
		{ hsm_kill		, HSM_STATE_DOWN	}	/* Shutdown */
	},
	{	/* Up State */
		{ hsm_ignore	, HSM_STATE_UP		},	/* No Event */
		{ hsm_ignore	, HSM_STATE_UP		},	/* Start */
		{ hsm_ignore	, HSM_STATE_UP		},	/* FullConnected */
		{ hsm_negt		, HSM_STATE_NEGT	},	/* NbrChng */
		{ hsm_kill		, HSM_STATE_DOWN	}	/* Shutdown */
	}
};

struct {
	int (*action) ();
	int next_state;
} NSM [NSM_STATE_MAX][NSM_EVENT_MAX] = 
{
	{	/* Down State */
		{ nsm_ignore	, NSM_STATE_DOWN	},	/* No Event */
		{ nsm_init		, NSM_STATE_WAIT	},	/* Start */
		{ nsm_ignore	, NSM_STATE_DOWN	},	/* KillEvent */
		{ nsm_ignore	, NSM_STATE_DOWN	},	/* HelloRcvd */
		{ nsm_ignore	, NSM_STATE_DOWN	},	/* OneWayRcvd */
		{ nsm_ignore	, NSM_STATE_DOWN	}	/* TwoWayRcvd */
	},
	{	/* Waiting State */
		{ nsm_ignore	, NSM_STATE_WAIT	},	/* No Event */
		{ nsm_ignore	, NSM_STATE_WAIT	},	/* Start */
		{ nsm_kill		, NSM_STATE_DOWN	},	/* KillEvent */
		{ nsm_hellorcvd	, NSM_STATE_UNIDIR	},	/* HelloRcvd */
		{ nsm_ignore	, NSM_STATE_WAIT	},	/* OneWayRcvd */
		{ nsm_ignore	, NSM_STATE_WAIT	}	/* TwoWayRcvd */
	},
	{	/* OneWay State */
		{ nsm_ignore	, NSM_STATE_UNIDIR	},	/* No Event */
		{ nsm_ignore	, NSM_STATE_UNIDIR	},	/* Start */
		{ nsm_kill		, NSM_STATE_DOWN	},	/* KillEvent */
		{ nsm_hellorcvd	, NSM_STATE_UNIDIR	},	/* HelloRcvd */
		{ nsm_ignore 	, NSM_STATE_UNIDIR	},	/* OneWayRcvd */
		{ nsm_twoway	, NSM_STATE_BIDIR	}	/* TwoWayRcvd */
	},
	{	/* TwoWay State */
		{ nsm_ignore	, NSM_STATE_BIDIR	},	/* No Event */
		{ nsm_ignore	, NSM_STATE_BIDIR	},	/* Start */
		{ nsm_kill		, NSM_STATE_DOWN	},	/* KillEvent */
		{ nsm_hellorcvd	, NSM_STATE_BIDIR	},	/* HelloRcvd */
		{ nsm_oneway	, NSM_STATE_UNIDIR	},	/* OneWayRcvd */
		{ nsm_ignore	, NSM_STATE_BIDIR	}	/* TwoWayRcvd */
	}
};

#endif	/* __HELLO_H_ */
