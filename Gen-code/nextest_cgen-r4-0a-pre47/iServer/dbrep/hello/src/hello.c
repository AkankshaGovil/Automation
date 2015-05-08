#include "unp.h"
#include "rs.h"
#include "fdsets.h"
#include "thutils.h"
#include "timer.h"
#include "helloP.h"
#include "srvrlog.h"
#include "list.h"
#include "lock.h"
#include "queue.h"
#include "ipcutils.h"
#include "ipckey.h"
#include <net/if.h>
#include <signal.h>

#ifdef DMALLOC
#include "dmalloc.h"
#endif

extern pthread_t	timerThread;
extern NetFds		Hellotimerfds;
extern HelloConfp	hcp;
extern Lock 		*hcp_lock;   /* Used for common variables used by
									by multiple threads. only prio_sa is
									used by multiple threads */
extern cvtuple		hellocv;

static void HelloProtMainLoop();

int
RunHelloProtocol(int argc, char **argv)
{

//	ThreadSetUniprocessorMode();

	NETDEBUG(MRSD, NETLOG_DEBUG1, ("starting hello protocol ... \n"));
	libavl_init(1);
	timerLibInit();
	// hcp_lock = LockAlloc();
	// LockInit(hcp_lock, 0);     /* Not sharing with other processes */
	
	hcpInit();


	timerInit(&hcp->Mtimer, 128, 0);
	NetFdsInit(&Hellotimerfds);

	IpcInit();

	ThreadLaunch(HelloHandleTimers, NULL, 1);

	pthread_mutex_lock(&(hellocv.mutex));
	hellocv.value = 1;
	pthread_cond_signal(&(hellocv.cond));
	pthread_mutex_unlock(&(hellocv.mutex));

	/* Main Loop */
	HelloProtMainLoop();

	return(0);
}

void
hcpInit()
{
	host_rec	*myrec;

	hcp = (HelloConfp)malloc(sizeof(HelloConf));

	/* Hello Protocol Initialization */
	time(&hcp->start_time);
	hcp->HPeriod = hello_interval;
	hcp->DeadInt = hello_interval * hello_dead_factor;
	hcp->nsm_list = listInit();
	hcp->curmsg.pktp = NULL;
	hcp->curmsg.sa = malloc(sizeof(struct sockaddr_in));
	hcp->hrecs = listInit();
	myrec = (host_rec *)malloc(sizeof(host_rec));
	myrec->host = GetIpAddr(hello_ifname);
	myrec->primary = htonl(0);
	Pthread_mutex_lock(&(host_dbrev.mutex));
	myrec->prio.dbrev = htonl(host_dbrev.value);
	Pthread_mutex_unlock(&(host_dbrev.mutex));
	myrec->prio.group = htons(hello_group);
	myrec->prio.self = htons(hello_priority);
	myrec->prio.start_time_c = htonl(~(hcp->start_time)); 
	myrec->HPeriod = htonl(hello_interval);
	myrec->DeadInt = htonl(hello_interval*hello_dead_factor);
	listAddItem(hcp->hrecs, myrec);
	hcp->reg = Malloc(sizeof(default_reg));
	hcp->state = HSM_STATE_DOWN;
	memcpy(hcp->reg, &default_reg, sizeof(default_reg));
	NetFdsInit(&(hcp->nfs));
	HelloCreateLocks();
	hcp->lockid = 0;
	// LockGetLock(hcp_lock, 0, 0);
	hcp->pri_sa = NULL;
	// LockReleaseLock(hcp_lock);
	qInit(&hcp->eventq, HELLO_EVENT_SIZE, HELLO_EVENTQ_LEN);
}

void *
HelloHandleTimers(void *arg)
{
	int		rval = 0;
	static struct timeval tout;
	int		msec = -1;
	int		numus = 0;

	timerThread = pthread_self();

#ifndef NETOID_LINUX
	ThreadSetRT();
#endif // NETOID_LINUX

//	ThreadSetPriority(timerThread, SCHED_RR, 50);

	for (;;) {
		rval = HelloSetupTimeout(&tout);

		if (rval < 0) {
			NETDEBUG(MRSD, NETLOG_DEBUG4, ("Timer already timed out\n"));
			HelloProcessTimeout();
			continue;
		}

		msec = INFTIM;
		if (rval) {			/* Find the timeout */
			HelloAdjustTimeout(&tout, &msec);
		}

		numus = NetFdsSetupPoll(&Hellotimerfds, MRSD, NETLOG_DEBUG4);
		NETDEBUG(MRSD, NETLOG_DEBUG4, ("%d ready for Hello Protocol\n", numus));

		rval = poll(Hellotimerfds.pollA, numus, msec);
		switch (rval) {
			case -1:
				NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll failure %d", errno));
				break;
			case 0:
				NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll timeout"));
				break;
			default:
				//fprintf(stdout, "Timer activated: %d\n", rval);
				NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll process"));
				rval = NetFdsProcessPoll(&Hellotimerfds, MSEL, NETLOG_DEBUG4);
				if (rval < 0) {
					NETERROR(MSEL,
					("Found a bad fd %d, deactivating it!\n", -rval));
					NetFdsDeactivate(&Hellotimerfds, -rval, FD_RW);
				}
				break;
		}
	}

	return 0;
}

/*
 * Looks up in the Timer list and sets up timeout for the next timer
 */
int
HelloSetupTimeout(struct timeval *tout)
{
	static struct timeval 	now;
	static struct Timer 	front;
	int						rval = 0;
	long					secs, usecs;

	tout->tv_sec = 0xfffffff;
	tout->tv_usec = 0xfffffff;

	/* Set up the timeout based on our own timer list */
	if (timerFront(&hcp->Mtimer, &front) != 0) {
		gettimeofday(&now, NULL);

		/* compute front - now, to return the delta */
		secs = front.expire.tv_sec - now.tv_sec;
		usecs = front.expire.tv_usec - now.tv_usec;

		if (usecs < 0) {
			secs --;
			usecs += 1000000;
		}

		if ((secs < 0) || (!secs && !usecs)) {
			NETDEBUG(MRSD, NETLOG_DEBUG4, 
				("HelloSetupTimeout:: secs is negative\n"));
			return -1;
		}

		tout->tv_sec = secs;
		tout->tv_usec = usecs;

		rval = 1;
	}

	/* We are done... */
	return rval;
}

int
HelloAdjustTimeout(struct timeval *to, int *msec)
{
	int m1;

	// Find min
	if ((to->tv_sec == (unsigned long)-1) && (to->tv_usec == (unsigned long)-1)) {
		NETERROR(MTMR, ("HelloAdjustTimeout, bad arg\n"));
		return -1;
	}

	m1 = to->tv_sec*1000 + to->tv_usec/1000;	

	if ((*msec == INFTIM) || (m1 < *msec)) {
		*msec = m1;
		return 1;
	}

	return 0;
}

int
HelloProcessTimeout(void)
{
	serviceTimers(&hcp->Mtimer);
	return(0);
}

int
HandleNotify(int fd, FD_MODE rw, void *data)
{
	char 	buf[100];
	int		rval;
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;
	
	tcbdata->rnotify++;
	rval = read(fd, buf, 1);

	NETDEBUG(MRSD, NETLOG_DEBUG4, ("Read %d notifications\n", rval));

	return 0;
}

int
NotifyHelloProt(void *data)
{
	char fn[] = "NotifyHelloProt():";
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;

	if (tcbdata->wnotify != tcbdata->rnotify) {
	// already notification in queue
		NETDEBUG(MRSD, NETLOG_DEBUG4, ("%s Already Notified r=%x w=%x\n",
			fn, tcbdata->rnotify, tcbdata->wnotify));
		return 0;
	}

	if (write(tcbdata->notifyPipe[NOTIFY_WRITE]," ",1) < 0) {
		// Check the error
		if (errno == EAGAIN) {
			NETERROR(MRSD, ("Blocking error in notification\n"));
		}
	}
	else {
		tcbdata->wnotify ++;
	}

	return(0);
}

int
SendHello(struct Timer* t)
{
	char 		buf[MAXBUF];
	int			len;
	HPktHdr		*hpktp;


	hpktp = (HPktHdr *)buf;
	hpktp->type = htonl(HELLO_DEF_PKT_TYPE);
	len = CopyRecstoPkt(hcp->hrecs, (char *)(hpktp+1));
	if (len <= 0) {
		NETERROR(MRSD, ("Length of Hello Records is not a positive integer\n"));
		return(-1);
	}
	hpktp->datalen = htonl(len);
	len += sizeof(HPktHdr);
	NETDEBUG(MRSD, NETLOG_DEBUG4, ("Sending a Hello Pkt with %d recs \n",
		listItems(hcp->hrecs)));
	Sendto(hcp->sendfd, buf, len, 0, hcp->sa, sizeof(struct sockaddr_in));
	return(0);
}

int
OpenIpcPipe(int pipefd[2], int sflag)
{
	int		flags;

	/* Open notification pipe */
	if (pipe(pipefd) < 0) {
		perror("Unable to open pipe");
		return -1;
	}

	if((flags = fcntl(pipefd[NOTIFY_READ],F_GETFL,0)) <0) {
	 	perror("notify fcntl");
	 	return -1;
	}

	flags |= sflag;

	if((fcntl(pipefd[NOTIFY_READ],F_SETFL,flags)) <0) {
	 	perror("notify fcntl");
	 	return -1;
	}

	if((flags = fcntl(pipefd[NOTIFY_WRITE],F_GETFL,0)) <0) {
	 	perror("notify fcntl");
	 	return -1;
	}

	flags |= sflag;

	if((fcntl(pipefd[NOTIFY_WRITE],F_SETFL,flags)) <0) {
	 	perror("notify fcntl");
	 	return -1;
	}

	return(0);
}

int
OpenNetSocks()
{
	int		rval = 0;

	/* 
	 * We need a separate socket for sending Multicast packets
	 * and one for receiving packets. See Stevens page 508. 
	 * The receiving socket is bound to the multicast address
	 * the sending socket is bound to a local address. 
	 */

	/* Open a multicast server for the hello protocol */
	if (mcast_serv(hello_mcast_addr, hello_port, hello_ifname, 
		&(hcp->sendfd), &(hcp->recvfd), (void **) &(hcp->sa)) < 0) {
		NETERROR(MRSD, ("Failed to start the multicast server for hello protocol\n"));
	}

	NetFdsAdd(&(hcp->nfs), hcp->sendfd, FD_READ,
		(NetFn)HandleUniMsg, (NetFn) NULL, NULL, NULL);

	NetFdsAdd(&(hcp->nfs), hcp->recvfd, FD_READ,
		(NetFn)HandleMulMsg, (NetFn) NULL, NULL, NULL);

	return(rval);
}

int
IpcInit()
{
	int		rval = 0;
	TimerNotifyCBData *tcbdata;

	tcbdata = TimerNotifyCBDataAlloc();
	if ((rval = OpenIpcPipe(tcbdata->notifyPipe, O_NONBLOCK)) < 0) {
		NETERROR(MRSD, ("OpenIpcPipe: failed to open the ipc pipe\n"));
		return(rval);
	}

	/* Add the callback */
	setTimerNotifyCb(&hcp->Mtimer, NotifyHelloProt, tcbdata);

	/* Add the pipe fd to the Hellotimerfds */
	NetFdsAdd(&Hellotimerfds, tcbdata->notifyPipe[NOTIFY_READ], FD_READ,
		(NetFn)HandleNotify, (NetFn) NULL, tcbdata, NULL);

	if ((rval = OpenNetSocks()) < 0) {
		NETERROR(MRSD, ("OpenNetSocks: failed to open network sockets\n"));
		return(rval);
	}

	return(rval);
}

static void
HelloProtMainLoop()
{
	int 	numus = 0;
	int 	rval = 0;

	exec_hsm_event(HSM_EVENT_START);

	for(;;) {
		numus = NetFdsSetupPoll(&hcp->nfs, MRSD, NETLOG_DEBUG4);
		NETDEBUG(MRSD, NETLOG_DEBUG4, ("%d ready for Hello Protocol\n", numus));
		rval = poll(hcp->nfs.pollA, numus, -1);
		switch (rval) {
			case -1:
				NETDEBUG(MRSD, NETLOG_DEBUG4, ("poll failure - %d", errno));
				break;
			case 0:
				NETDEBUG(MRSD, NETLOG_DEBUG4, ("poll timeout"));
				break;
			default:
				NETDEBUG(MRSD, NETLOG_DEBUG4, ("poll process"));
				rval = NetFdsProcessPoll(&hcp->nfs, MRSD, NETLOG_DEBUG4);
				if (rval < 0) {
					NETERROR(MRSD, ("found a bad fd %d, deactivating it!\n", -rval));
					NetFdsDeactivate(&hcp->nfs, -rval, FD_RW);
				}
				break;
		}
	}
}

int
HandleUniMsg()
{
	NETDEBUG(MRSD, NETLOG_DEBUG3, ("Received Unicast Hello Messages\n"));
	return(0);				
}

int
HandleMulMsg()
{
	socklen_t		salen = sizeof(struct sockaddr_in);
	nid				nbrk;

	/* Some Initializations */
	hcp->curmsg.nbrk = 0;

	if ((hcp->curmsg.pktp = (HPktHdr *)GetNetPkt(hcp->recvfd, \
		hcp->curmsg.sa, salen)) == NULL) {
		NETERROR(MRSD, ("HandleMulMsg: Invalid hello Packet\n"));
		return(-1);
	}

	/* Fix the Pkt Header in the right byteorder*/
	(hcp->curmsg.pktp)->type = ntohl(hcp->curmsg.pktp->type);
	(hcp->curmsg.pktp)->datalen = ntohl(hcp->curmsg.pktp->datalen);

	/* Do some error-checking */
	if ((hcp->curmsg.pktp)->type != HELLO_DEF_PKT_TYPE ) {
		NETERROR(MRSD, ("HandleMulMsg: Invalid hello Packet (type = %d)\n", 
			(hcp->curmsg.pktp)->type));
		Free(hcp->curmsg.pktp);
		return(-1);
	}

	nbrk = sock_2_nid((struct sockaddr_in *)hcp->curmsg.sa);

	exec_nsm_event(NULL, nbrk, NSM_EVENT_START);

	exec_nsm_event(NULL, nbrk, NSM_EVENT_HELLORCVD);

	exec_nsm_event(PresentInNbrRec, nbrk, NSM_EVENT_TWOWAYRCVD);

	exec_nsm_event(AbsentInNbrRec, nbrk, NSM_EVENT_ONEWAYRCVD);

	if (LeaderChanged()) 
		exec_hsm_event(HSM_EVENT_NBRCHNG);

	if (CheckNSMStates() && LeaderAgreed()) 
		exec_hsm_event(HSM_EVENT_FULLCONN);

	return(0);
}

int
CopyRecstoPkt(List reclist, char *pktp)
{
	int 	len = 0;
	void	*item;

	for (item = listGetFirstItem(reclist); item; \
		item = listGetNextItem(reclist, item)) {
		memcpy(pktp+len, (host_rec *)item, sizeof(host_rec));
		len += sizeof(host_rec);	

	}

	return(len);
}

int
PresentInNbrRec(nbr_info *nbrp)
{
	ListStruct	*item, *found_item;	

	if (!nbrp) {
		return(0);
	}

	item = listGetFirstItem(hcp->hrecs); 	/* The first list item is self */
	
	found_item = SearchListforMatch(nbrp->records, item, match_rec_self);

	if (found_item == NULL)
		return(0);	
	else
		return(1);	
}

int
AbsentInNbrRec(nbr_info *nbrp)
{
	return(!PresentInNbrRec(nbrp));
}

/* Returns non-zero if Leader is determined */
int
HostIsLeader(struct Timer *t)
{

	if (LeaderChanged()) 
		exec_hsm_event(HSM_EVENT_NBRCHNG);

	if (CheckNSMStates() && LeaderAgreed()) {
	  	if (listItems(hcp->hrecs) == 1) {
			NETINFOMSG(MRSD, ("No neighbour present. Host is Leader\n"));
		}
		exec_hsm_event(HSM_EVENT_FULLCONN);
	}

	timerFreeHandle(t);

	return(0);
}

int
LeaderChanged(void)
{
	host_rec 	*hitem;
	IPADDRESS	prev_ldr, cur_ldr, ldr;
	struct in_addr	lead;
        char buf[INET_ADDRSTRLEN];

	hitem = listGetFirstItem(hcp->hrecs);
	
	if (hitem == NULL) {
		/* We should never come here */
		NETERROR(MRSD, ("Fatal: Can not find self host record. Exiting... \n"));
		kill(getpid(), SIGTERM);
	}

	prev_ldr = hitem->primary;	

	cur_ldr = UpdateSelfRec();

	ldr = cur_ldr;
	memcpy(&lead, &ldr, sizeof(ldr));
	if ( prev_ldr != cur_ldr ) {
		NETDEBUG(MRSD, NETLOG_DEBUG1, ("Leader changed to %s\n", inet_ntop( AF_INET, &lead, buf, INET_ADDRSTRLEN)));
		NETINFOMSG(MRSD, ("Leader changed to %s based on priority calculations\n", inet_ntop( AF_INET, &lead, buf, INET_ADDRSTRLEN)));
		hcp->printflag = 1;
		publish_host_table();
		return(1);
	}
	else 
		return(0);
}

/* Update our choice of leader. Return non-zero if leader changed */
IPADDRESS
UpdateSelfRec(void)
{
	List		list;
	host_rec	*hi_prio = NULL;
	host_rec	*item, *hitem;

	list = hcp->hrecs;
	
	for (item = listGetFirstItem(list), hitem = item; item; \
		item = listGetNextItem(list, item)) {
		hi_prio = cmp_prio(hi_prio, (host_rec *)item);
	}

	if (hi_prio->host == -1) {
		NETERROR(MRSD, ("Two neighbours have exactly same priority\n"));
		Free(hi_prio);
		return(0);
	}

	memcpy(&(hitem->primary), &(hi_prio->host), sizeof(IPADDRESS));

	if ( hitem->prio.dbrev != htonl(host_dbrev.value) ) {
		Pthread_mutex_lock(&(host_dbrev.mutex));
		hitem->prio.dbrev = htonl(host_dbrev.value);
		Pthread_mutex_unlock(&(host_dbrev.mutex));
	}

	/* Also update our role MASTER/SLAVE */
	if (memcmp(&(hitem->primary), &(hitem->host), sizeof(IPADDRESS)) == 0)
		hcp->role = RS_MASTER;
	else
		hcp->role = RS_SLAVE;

	return(hitem->primary);
}

/* The host records are in network byte order */

host_rec *
cmp_prio(host_rec *item1, host_rec *item2)
{
	host_rec	*invalid;
	int			rval;

	if (item1 == NULL) {
		return(item2);
	}

	if (item2 == NULL) {
		return(item1);
	}

	/* 	Since we are using memcmp to compare the two priorities,
		we need to keep the values in network byte order. 
		Also, as we have store ones complement of start time in
		the priorities we can just compare the the two priorities
		as byte strings */

	rval = memcmp(&(item1->prio), &(item2->prio), sizeof(priority));

	if (rval == 0) {
		invalid = calloc(sizeof(host_rec), 1);
		invalid->host = htonl(-1);
		return(invalid);
	} 
	else if (rval > 0) 
		return(item1);
	else
		return(item2);
}

/* Returns non-zero if everbody agrees on the leader */
int
LeaderAgreed(void)
{
	List		list;
	host_rec	*item;
	IPADDRESS	leader;
	struct sockaddr_in	*sap;

	list = hcp->hrecs;

	for (item = (host_rec *)listGetFirstItem(list), leader = item->primary;\
		item; item = (host_rec *)listGetNextItem(list, item)) {
		if (item->primary != leader)
			return(0);
	}

	/* Everybody agrees on the leader. Let's fill in his address */
//	LockGetLock(hcp_lock, 0, 0);
	sap = (struct sockaddr_in *)hcp->pri_sa =  Calloc(sizeof(struct sockaddr_in), 1);
	sap->sin_family = AF_INET;
	sap->sin_addr.s_addr = leader;
	sap->sin_port = htons(atoi(rs_port));
//	LockReleaseLock(hcp_lock);

	return(1);
}

/* Returns non-zero if all NSM are in bidirectional state */
int
CheckNSMStates(void)
{
	List	list;
	void	*item;

	list = hcp->nsm_list;

	//fprintf(stdout, "CheckNSMStates: \t");
	for (item = listGetFirstItem(list); item; \
		item = listGetNextItem(list, item)) {
		//fprintf(stdout, "nbr state = %d\t", ((nbr_info *)item)->state);
		//fprintf(stdout, "no. recs = %d\n", listItems(hcp->hrecs));
		if (((nbr_info *)item)->state != NSM_STATE_BIDIR)
			return(0);
	}

	return(1);
}

List
CreateRecordList(HPktHdr *hpktp)
{
	List	reclist;
	int		i, numrecs;
	void	*item;

	reclist = listInit();

	if ( (hpktp->datalen)%sizeof(host_rec) != 0) {
		NETERROR(MRSD, ("Hello Pkt Corrupt\n"));
		return (NULL);
	}

	numrecs = (hpktp->datalen)/sizeof(host_rec);	

	for (i = 0; i < numrecs; i++) {
		item = Malloc(sizeof(host_rec));
		memcpy(item, (void *)((host_rec *)(hpktp+1) + i), sizeof(host_rec));
		if ( listAddItem(reclist, item) != 0) {
			NETERROR(MRSD, ("Error Adding Item to record list \n"));
			listDestroy(reclist);
			return (NULL);
		}
	}

	return (reclist);
}

void
exec_nsm_event(PFIN precond, nid nbrk, int event)
{
	nbr_info	*ni; 

	HelloGetLock(LOCK_NSM);

	if ((ni =  get_nbr_info(nbrk)) == NULL) {
		nbrk = 0; 		/* Since we didn't find the nbrk, make it null */
	}

	if ((precond == NULL) || ((*precond)(ni))) {
		if (ni != 0) {   /* nbr already exists */
			if (!(NSM[ni->state][event].action(ni))) {
				NETERROR(MRSD, ("Error Executing nsm event \n"));
			}
			ni->state = NSM[ni->state][event].next_state;
		}
		else {   		 /* Start a new nbr */
			if (!(NSM[NSM_STATE_DOWN][event].action(ni))) {
				NETERROR(MRSD, ("Error Executing nsm event \n"));
			}
		}
	}

	HelloRelLock(LOCK_NSM);
}

void
exec_hsm_event(int event)
{
	HelloGetLock(LOCK_HSM);
	if (HSM[hcp->state][event].action())
		hcp->state = HSM[hcp->state][event].next_state;
	else {
		NETERROR(MRSD, ("Error Executing hsm event \n"));
	}
	HelloRelLock(LOCK_HSM);
}

void
sched_hsm_event(int event, int	ms)
{
	struct itimerval evtmr;
	
	if (ms == 0)
		ms = 10;
	/* Add a timer which calls the event after ms milliseconds*/
	memset(&evtmr, 0, sizeof(evtmr));
	evtmr.it_value.tv_usec = ms*1000;
	timerAddToList(&hcp->Mtimer, &evtmr, 0, \
		PSOS_TIMER_REL, "HSM Event", hsm_event_cb, (void *)event);

}

int
hsm_event_cb(struct Timer *t)
{
	exec_hsm_event((int)(t->data));

	timerFreeHandle(t);

	return(0);
}

int
match_nbr_addr(const void *item1, const void *item2)
{
	nbr_info	*ni1, *ni2;

	ni1 = (nbr_info *)item1;
	ni2 = (nbr_info *)item2;

	/* No match, if any one of them is a null pointer */
	if ( (ni1 == NULL) || (ni2 == NULL))
		return(0);	
	
	/* If match Nbr ipaddr return 1 else return 0 */
	return (memcmp(ni1->sa, ni2->sa, sizeof(struct sockaddr_in)) ? 0 : 1);
}

int
match_nbr_key(const void *item1, const void *item2)
{
	nbr_info	*ni1, *ni2;

	ni1 = (nbr_info *)item1;
	ni2 = (nbr_info *)item2;

	/* No match, if any one of them is a null pointer */
	if ( (ni1 == NULL) || (ni2 == NULL))
		return(0);	
	
	/* If match Nbr ipaddr return 1 else return 0 */
	return (memcmp(&(ni1->nbrk), &(ni2->nbrk), sizeof(nid)) ? 0 : 1);
}

int
match_rec_self(const void *item1, const void *item2)
{
	host_rec	*hr1, *hr2;

	hr1 = (host_rec *)item1;
	hr2 = (host_rec *)item2;

	/* No match, if any one of them is a null pointer */
	if ( (hr1 == NULL) || (hr2 == NULL))
		return(0);	

	/* If match Nbr ipaddr return 1 else return 0 */
	return (memcmp(hr1, hr2, sizeof(host_rec)) ? 0 : 1);
}

int
match_rec_nbr(const void *item1, const void *item2)
{
	host_rec	*hr1, *hr2;

	hr1 = (host_rec *)item1;
	hr2 = (host_rec *)item2;

	/* No match, if any one of them is a null pointer */
	if ( (hr1 == NULL) || (hr2 == NULL))
		return(0);	

	/* If match Nbr ipaddr return 1 else return 0 */
	return (memcmp(&(hr1->host), &(hr2->host), sizeof(IPADDRESS)) ? 0 : 1);
}

void
HSM_Init(void)
{
		
}

char *
GetNetPkt(int fd, struct sockaddr *sa, socklen_t salen)
{
	char 	*buf;
	char 	from[RS_LINELEN];

	buf = Calloc(MAXBUF, 1);

	if (Recvfrom(fd, buf, MAXBUF, 0, sa, &salen) < 0) {
		NETERROR(MRSD, ("GetNetPkt: %s", strerror(errno)));
		Free(buf);
		return(NULL);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG3, ("Received from %s\n", 
		Sock_ntop_r(sa, salen, from, sizeof(from))) );

	return buf;
}

int
def_init_cb(void *aarg, void *harg)
{
	NETDEBUG(MRSD, NETLOG_DEBUG1, ("Init callback function called \n"));
	return(1);
}

int
def_kill_cb(void *aarg, void *harg)
{
	NETDEBUG(MRSD, NETLOG_DEBUG1, ("Kill callback function called \n"));
	return(1);
}

int
nsm_hellorcvd(nbr_info *nbrp)
{
	host_rec 	*new_nbr_rec, *old_nbr_rec, *tmp_rec;
	ListStruct	*tmplsi;
	List		records;
	struct 		itimerval	*alivetmrp;
	char 		fn[] = "nsm_hellorcvd";
	void		*timerdata;

	if (nbrp == NULL) {
		return(1);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));

	if (nbrp->alivetid != 0) {		/* Timer not initialized */
		if (timerDeleteFromList(&hcp->Mtimer, nbrp->alivetid, &timerdata) == 0)
		{
			NETDEBUG(MRSD, NETLOG_DEBUG2,
				("Hello Received too late\n"));
			return (1);
		}
	}

	// Valid scenario - response received in time

	/* Hello Pkt from a known neighbour */
	if ((records = CreateRecordList(hcp->curmsg.pktp)) == NULL) {
		NETERROR(MRSD, ("Failed to Create the Record List"));
		return(-1);
	}

	if (nbrp->records != NULL) {
		listDestroy(nbrp->records);
		Free(nbrp->curpkt);
	}
	nbrp->records = records;
	nbrp->curpkt = hcp->curmsg.pktp;

	new_nbr_rec = (host_rec *)listGetFirstItem(nbrp->records);

	/* Add the sender entry in the host records */
	if ((tmplsi = SearchListforMatch(hcp->hrecs, (void *)new_nbr_rec, \
		match_rec_nbr)) != NULL) {
		old_nbr_rec = (host_rec *)(tmplsi->item);
		if (memcmp(old_nbr_rec, new_nbr_rec, sizeof(host_rec)) != 0) {
			/* Update the record and call a callback */
			tmplsi->item = malloc(sizeof(host_rec));
			bcopy(new_nbr_rec, tmplsi->item, sizeof(host_rec));
			Free(old_nbr_rec); 
		}
	}
	else {
		/* Update the record and call a callback */
		tmp_rec = (host_rec *)malloc(sizeof(host_rec));
		memcpy((void *)tmp_rec, (void *)new_nbr_rec, sizeof(host_rec));
		listAddItem(hcp->hrecs, (void *)tmp_rec);
	}

	/* Extend the lease of life for this nbr */
	alivetmrp = (struct itimerval *)Calloc(1, sizeof(struct itimerval));
	alivetmrp->it_value.tv_sec = ntohl(new_nbr_rec->DeadInt);
	nbrp->alivetid = timerAddToList(&hcp->Mtimer, alivetmrp, 1, \
		PSOS_TIMER_REL, "AliveTimer", nsm_kill_cb, (void *)nbrp->nbrk);
	Free(alivetmrp);

	return(1);
}

int
nsm_oneway(nbr_info *nbrp)
{
	char fn[] = "nsm_oneway";

	if (nbrp == NULL) {
		return(1);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));

	nbrp->state = NSM_STATE_UNIDIR;
	return(1);
}

int
nsm_init(nbr_info *nbrp)
{
	char fn[] = "nsm_init";

	if (nbrp != NULL) {
		return(1);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));

	/* A new neighbour is seen */
	nbrp = calloc(sizeof(nbr_info), 1);
	listAddItem(hcp->nsm_list, (void *)nbrp);
	nbrp->sa = Malloc(sizeof(struct sockaddr_in));
	memcpy(nbrp->sa, (struct sockaddr_in *)hcp->curmsg.sa, \
		sizeof(struct sockaddr_in));

	hcp->curmsg.nbrk = nbrp->nbrk = sock_2_nid(nbrp->sa);
	nbrp->state = NSM_STATE_WAIT;

	return(1);
}

int
nsm_up(nbr_info *nbrp)
{
	char fn[] = "nsm_up";

	if (nbrp == NULL) {
		return(1);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));

	return(1);
}

int
nsm_twoway(nbr_info *nbrp)
{
	char fn[] = "nsm_twoway";

	if (nbrp == NULL) {
		return(1);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));

	nbrp->state = NSM_STATE_BIDIR;

	hcp->printflag = 1;	  /* Print list is true */
	publish_host_table();

	return(1);
}

int
nsm_ignore(nbr_info *nbrp)
{
	char fn[] = "nsm_ignore";

	if (nbrp == NULL) {
		return(1);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG3, ("%s: called \n", fn));
	return(1);
}

int
nsm_kill_cb(struct Timer *t)
{
	nid nbrk = (nid)(t->data);
	nbr_info *nbrp;
	int rval;

	NETDEBUG(MRSD, NETLOG_DEBUG1, ("nsm_kill_cb: Neighbour died \n"));
	HelloGetLock(hcp->lockid);
//	rval = nsm_kill(nbrp);
	nbrp = get_nbr_info(nbrk);
	rval = nsm_kill(nbrp);
	HelloRelLock(hcp->lockid);

	timerFreeHandle(t);

	return(rval);
}

int
nsm_kill(nbr_info *nbrp)
{
	host_rec	*nbr_rec;
	List		tmp;
	char fn[] = "nsm_kill";
	struct in_addr nbr_ip_addr;
        char buf[INET_ADDRSTRLEN];

	if (nbrp == NULL) {
		return(1);
	}

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));

	nbr_rec = (host_rec *)listGetFirstItem(nbrp->records);
	if ((tmp = SearchListforMatch(hcp->hrecs, (void *)nbr_rec, \
		match_rec_nbr)) == NULL) {
		NETDEBUG(MRSD, NETLOG_DEBUG1, ("host does not contain record for neigbour which died\n"));
	}
	else {
		nbr_ip_addr.s_addr = nbr_rec->host;
		NETINFOMSG(MRSD, ("Neighbour %s died \n", 
			inet_ntop( AF_INET, &nbr_ip_addr, buf, INET_ADDRSTRLEN) ));
		listDeleteItem(hcp->hrecs, tmp->item);
	}

	hcp->printflag = 1;	  /* Print list is true */
	publish_host_table();

	Free(nbrp->sa);
	Free(nbrp->curpkt);
	listDestroy(nbrp->records);
	listDeleteItem(hcp->nsm_list, nbrp);
	Free(nbrp);

	if (LeaderChanged()) 
		exec_hsm_event(HSM_EVENT_NBRCHNG);

	if (CheckNSMStates() && LeaderAgreed()) {
		/* A kludgy way to give system some time before we call init */
		msleep(500);
		exec_hsm_event(HSM_EVENT_FULLCONN);
	}

	return(1);
}

int
hsm_ignore(void)
{
	char fn[] = "hsm_ignore";

	NETDEBUG(MRSD, NETLOG_DEBUG3, ("%s: called \n", fn));
	return(1);
}

int
hsm_negt(void)
{
	char fn[] = "hsm_negt";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));
	hcp->reg->cb_kill(hcp->reg->kill_arg, (void *)hcp->role);
	return(1);
}

int
hsm_kill(void)
{
	char fn[] = "hsm_kill";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));
	HelloDeleteLocks();
	return(1);
}

int
hsm_init(void)
{
	struct itimerval 	hellotmr, selftmr;
	char fn[] = "hsm_init";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));

	/* DOWN State */
	hcp->nsm_list = listInit();

	/* Add an interval timer which wakes up and sends a hello message */
	memset(&hellotmr, 0, sizeof(hellotmr));
	hellotmr.it_interval.tv_sec = HELLO_INTERVAL;
	hcp->hellotid = timerAddToList(&hcp->Mtimer, &hellotmr, 0, \
		PSOS_TIMER_REL, "HelloTimer", SendHello, NULL);

	/* Add a timer which makes the host master if there is no response seen */
	memset(&selftmr, 0, sizeof(selftmr));
	selftmr.it_value.tv_sec = HELLO_INTERVAL*DEAD_FACTOR;
	hcp->selftid = timerAddToList(&hcp->Mtimer, &selftmr, 0, \
		PSOS_TIMER_REL, "SelfTimer", (TimerFn)HostIsLeader, NULL);

	return(1);
}

int
hsm_up(void)
{
	char fn[] = "hsm_up";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: called \n", fn));
	hcp->reg->cb_init(hcp->reg->init_arg, (void *)hcp);

	hcp->printflag = 1;	  /* Print list is true */
	publish_host_table();

	return(1);
}

int
HelloCreateLocks(void)
{
	int semid;
	char fn[] = "HelloCreateLock():";

	if (smn_create(ftok(DBLOCKDIR, HELLO_SEM_PROJ), HELLO_NUM, 1, 0, &semid) \
		< 0) {
		NETERROR(MRSD, ("%s could not create hello locks\n", fn));
		return(-1);
	}

	return(0);
}

int
HelloDeleteLocks(void)
{
	int semid;

	if (smn_get(ftok(DBLOCKDIR, HELLO_SEM_PROJ), HELLO_NUM, 1, 0, &semid) \
		>= 0) {
		smn_delete(semid);
	}

	return(0);
}

void
HelloGetLock(int lockid)
{
	int semid;
	char fn[] = "HelloGetLock():";

	if (smn_get(ftok(DBLOCKDIR, HELLO_SEM_PROJ), HELLO_NUM, 1, 0, &semid) \
		< 0) {
		NETERROR(MRSD, ("%s could not get hello locks\n", fn));
	}
	else {
		smn_p(semid, lockid, 0, 0);
		NETDEBUG(MRSD, NETLOG_DEBUG4, ("pid %d:%d acquired hello locks %d\n",\
			(int)getpid(), pthread_self(), lockid));
	}
}

void
HelloRelLock(int lockid)
{
	int semid;
	char fn[] = "HelloRelLock():";

	if (smn_get(ftok(DBLOCKDIR, HELLO_SEM_PROJ), HELLO_NUM, 1, 0, &semid) \
		< 0) {
		NETERROR(MRSD, ("%s could not get hello locks\n", fn));
	}
	else {
		smn_v(semid, lockid);
		NETDEBUG(MRSD, NETLOG_DEBUG4, ("pid %d:%d released hello locks %d\n",\
			(int)getpid(), pthread_self(), lockid));
	}
}

int
HelloRegCb(AppCB *appcb)
{
	memcpy(hcp->reg, appcb, sizeof(AppCB));

	NETDEBUG(MRSD, NETLOG_DEBUG3, ("Application in thread %d registered callback with hello protocol\n", pthread_self()));
	
	return(0);
}

int
publish_host_table(void)
{
	List	list = hcp->hrecs;
	host_rec	*item, *hitem;
	char	cmd[RS_LINELEN];
	int		len, flag;
	int		rval;

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("publish_host_table called - printflag = %d\n", hcp->printflag));

	if (!(hcp->printflag))
		return(0);

	snprintf(cmd, RS_LINELEN, "%s rsd clear", RS_CLI_CMD_STR);

//	rval = ExecuteCliCommand(cmd);
//	hcp->printflag = rval;
	rval = System(cmd);
	hcp->printflag = rval;
	NETDEBUG(MRSD, NETLOG_DEBUG2, ("executed - %s, rval =%d\n", cmd, rval));

	for (item = listGetFirstItem(list), hitem = item; item; \
		item = listGetNextItem(list, item)) {
		flag = (memcmp(&(hitem->primary), &(item->host), sizeof(IPADDRESS))==0);
		len = 0;
		//len += snprintf(cmd, RS_LINELEN, "%s rsd add ", RS_CLI_CMD_STR);
		//len += snprintf(cmd, RS_LINELEN, "%d", ntohs(item->port));
		len += snprintf(cmd+len, RS_LINELEN-len, "%s rsd add ", RS_CLI_CMD_STR);
		len += snprintf(cmd+len, RS_LINELEN-len, "%d", (atoi(rs_port)));
		len += snprintf(cmd+len, RS_LINELEN-len, " ");
		FormatIpAddress(ntohl(item->host), cmd+len);
		strcat(cmd+len, flag?" master":" slave");
	//	rval = ExecuteCliCommand(cmd);
	//	hcp->printflag = rval;
		rval = System(cmd);
		hcp->printflag = rval;
		NETDEBUG(MRSD, NETLOG_DEBUG2, ("executed - %s, rval = %d\n", cmd, rval));
	}

	/* We should check the status of all the system() and return an
	   appropriate status */
	return(0);
}

nbr_info *
get_nbr_info(nid nbrk)
{
	nbr_info tmp_ni, *ni;
	ListStruct	*lsi;

	tmp_ni.nbrk = nbrk;

	if ((lsi = SearchListforMatch(hcp->nsm_list, (void *)&tmp_ni,
			match_nbr_key)) != NULL) {
		ni = (nbr_info *)lsi->item;
	}
	else {
		ni = NULL;
	}

	return (ni);
}

nid
sock_2_nid(struct sockaddr_in *sa)
{
	if (sa != NULL)
		return(sa->sin_addr.s_addr);
	else
		return(0);
}

char *
nid_2_str(nid nbrk, char *str)
{
	sprintf(str, "%ld", (unsigned long)nbrk);
	return(str);
}
