#ifndef __HELLO_COMMON_H_
#define __HELLO_COMMON_H_

#include "queue.h"
#include "timer.h"
#include "list.h"
#include "netloopP.h"

#define HELLO_INTERVAL          (4)
#define HELLO_MCAST_ADDR        ("230.1.1.1")
#define HELLO_PORT_STR          ("6000")
#define HELLO_DEFAULT_GROUP     (0)
#define HELLO_DEFAULT_PRIORITY  (0)
#define HELLO_IFNAME            ("znb1")
#define DEAD_FACTOR             (4)

extern int		hello_interval;
extern char		hello_mcast_addr[];
extern char		hello_port[];
extern unsigned short hello_group;
extern unsigned short hello_priority;
extern char		hello_ifname[];
extern int		hello_dead_factor;

typedef int (*PFI)(void *, void *);

typedef struct AppCB {
	PFI		cb_init;
	void	*init_arg;
	PFI 	cb_kill;
	void	*kill_arg;
} 	AppCB;

/*
 * Hello Packets consist of Header followed by one or more host_recs
 * The first record is always the sender's own record 
 */
typedef struct HPktHdr {
	int	type;
#define HELLO_DEF_PKT_TYPE	1
	int	datalen;
}	HPktHdr;

typedef uint32_t nid;

typedef struct nbr_info {
	struct	sockaddr_in *sa;
	int		state;
#define		NBR_CONN_NONE	0
#define		NBR_CONN_UNIDIR	1
#define		NBR_CONN_BIDIR	2
	tid		alivetid;
	HPktHdr	*curpkt;
	List	records;
	nid 	nbrk; 		/* The key with which this nbr is id'ed */
}	nbr_info;

/*
 * The Msg conatins the packet received and the source address
 */
typedef struct HMsg {
	HPktHdr	*pktp;
	struct sockaddr *sa;
	int	nbrk; 			/* IP Address of the neighbour, which 
								   is used as a key to search for its
								   record in the nsm_list */
}	HMsg;

typedef struct HelloConf {
	TimerPrivate	Mtimer;
	long			start_time;
	tid				hellotid;
	tid				selftid;
	int				HPeriod;		
	int				DeadInt;
	List			nsm_list;		/* List of Neighbour State Machines */
	int				sendfd;
	int				recvfd;				
	struct sockaddr *sa;			/* Sockaddr for the multicast dest */
	NetFds			nfs;			/* List of Fd to which it is listening */
	HMsg			curmsg;			/* Current Pkt being processed */
	List			hrecs;			/* A list of hello records */
	AppCB			*reg;			/* Application Callback's registered */
	int				state;
	int				lockid;
	int				role;
#define	RS_MASTER		0x1 		/* consistent with REPL_SERV_PRI */
#define	RS_SLAVE		0x2 		/* consistent with REPL_SERV_SEC */
	Q				eventq;
	int				printflag;		/* This flag denotes whether the list
										of hosts is to be printed or not */
	struct sockaddr *pri_sa;		/* Sockaddr for the primary controller */
}	HelloConf, * HelloConfp;

typedef struct priority {
	long			dbrev;
	unsigned short	group;
	unsigned short	self;
	long			start_time_c; 	/* Ones complement of start-time */
}	priority;

typedef unsigned int IPADDRESS;

/* For Optimization purposes all fields are stored in network byte order*/
typedef struct host_rec {
	IPADDRESS	host;
	IPADDRESS	primary;
	priority	prio;
	int			HPeriod; 	/* Frequency of Hello Pkts in secs */
	int			DeadInt;	/* Time (s) after which nbr is assumed dead */
}	host_rec;

#endif 	/*	__HELLO_COMMON_H_ */
