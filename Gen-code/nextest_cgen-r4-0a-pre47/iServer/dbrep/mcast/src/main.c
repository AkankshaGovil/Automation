#include	"unp.h"
#include	"rs.h"
#include 	"hello.h"
#include 	"cli.h"
#include	"hello_common.h"
#include	"spversion.h"
#include 	"defaultpath.h"
#include 	<net/if.h>
#include	"nxosd.h"

extern void RPC_Init( void );

/* typedef void *(*PFVP)(void *); */

extern int	sconfigdebug;
extern int	histdb_size; 	/* imported from sconfig.h */
extern int	RunRSSD(int argc, char **argv);

/* Private Routines */
static	void	SignalInit(void);
static	void	SyncSigHandler(int signo);
static	void	*AsyncSigHandlerThread(void * args);
static	void 	HandleUSR1(int sig);
static	void 	HandleUSR2(int sig);
static	void 	HandleINT(int sig);
static	void 	HandleCHLD(int sig);
static	void 	HandleTERM(int sig);
static	void 	HandleHUP(int sig);

/* The following macro is used to update the maxfdp1 when a new fd is added */
#define	det_fd(fd_a, p_max_fd)	max(((fd_a)+1), (p_max_fd))

int		rs_cur_ind;						/* Index of last command */
char	rs_cli_cmd_str[RS_LINELEN];		/* cli command for master RS */
char	rs_slave_cli_cmd_str[RS_LINELEN]; /* cli command for slave RS */
char	rs_reg_cli_cmd_str[RS_LINELEN]; /* cli command for registration */
char 	rs_hostname[MAXHOSTNAMELEN];  	/* host name of the machine */
int		rs_daemonize = 1;
int		rs_wait_for_cache = 1;
int		rs_start = 1;
int		db_inited = 0;
RSConfp	the_rscp;
char	pname[RS_LINELEN];
//char	config_file[RS_LINELEN] = CONFIG_FILENAME;
char	pidf[RS_LINELEN];
int		rsdpid;
cvtuple	rolecv = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};
sharvar host_dbrev = {PTHREAD_MUTEX_INITIALIZER, 0};
int		match = -1;
char	host_ver[RS_LINELEN];

/* Global Variables related to the clicmd thread */
int 	clipoolid, cliclassid;
int 	nclithreads = 1;
struct timespec	clicmdtmout;


int
main(int argc, char **argv)
{
	RSConfp		rscp;
	char		*basedir;
	int			i;
	AppCB		*rsd_cb;
	FILE		*pp;

#ifdef _DMALLOC_
	unsigned int mark;
#endif

/* The following line is just to debug the cfg file parser */
//	sconfigdebug = 1;

	/* If SERPLEXPATH environment variable is set then append it to the config_file */
	parse_rs_opt(argc, argv);

	sprintf(pname, "%s", basename(argv[0]));

	setConfigFile();
	myConfigServerType = CONFIG_SERPLEX_RSD;

	/* Setup signal handlers */

#if 0
	NETLOG_SETLEVEL(MRSD, NETLOG_DEBUG2);
	NETLOG_SETLEVELE(MRSD, NETLOG_DEBUGMASK);
	NETLOG_SETLEVELE(MRSD, NETLOG_ERRORMASK);
#endif

	/* Read the server.cfg file and configure the hello variables
	   accordingly 												*/
	DoConfig(ConfigureRSD);	

	/* Lets get our version initialised */
	if ((pp = popen("/bin/arch", "r")) != NULL) {
		fgets(host_ver, RS_LINELEN, pp);
		host_ver[strlen(host_ver)-1] = '\0';
		strncat(host_ver, "-", RS_LINELEN);
		strncat(host_ver, VERSION MINOR, RS_LINELEN);
		NETDEBUG(MRSD, NETLOG_DEBUG1, ("host version is %s\n", host_ver));
		pclose(pp);
	}

	strcpy(hello_mcast_addr, rs_mcast_addr); /* Use rs mcastadrr */
	strcpy(hello_ifname, rs_ifname); 		 /* Use rs ifname */
	hello_priority = rs_host_prio; 			 /* Set host priority */

	NETDEBUG(MRSD, NETLOG_DEBUG1, ("process rsd starting \n"));
	NETINFOMSG(MRSD, ("*** NexTone Replication Server started ***\n"));

	/* Initialize the RSConf Structure */
	the_rscp = rscp = Calloc(sizeof(RSConf), 1);
	rscp->sendfd = rscp->recvfd = rscp->rcmdfd = rscp->listenfd = -1;
	pthread_mutex_init(&(rscp->tid_mutex), NULL);
	CVInit(&(rscp->replcv));
	CVInit(&(rscp->copycv));
	for (i = 0; i < THREAD_MAX; i++) {
		rscp->tids[i] = NULL_TID;
	}

	if (rs_daemonize) {
		daemon_init(LEAVE_IO_FD);
		(void) freopen( "/dev/null", "r", stdin );
		(void) freopen( "/var/log/iserverout.log", "a", stdout );
		(void) freopen( "/var/log/iservererr.log", "a", stderr ); 
	}
//		RPC server to RSD not currently used
//	{
//		serplex_config *iserver = &serplexes[ match ];
//
//		if ( ( iserver != NULL ) &&
//			 ( iserver->ispd.location.type != CONFIG_LOCATION_NONE ) &&
//			 ( ispd_type != ISPD_TYPE_DISABLED ) )
//			RPC_Init();
//	}

	SignalInit();

	NetSyslogOpen(pname, NETLOG_ASYNC);
	sprintf(pidf, "%s/%s", PIDS_DIRECTORY, RSD_PID_FILE);
	rsdpid = ReadPid(pidf);

	if ( rsdpid > 0 ) {
		if ((kill(rsdpid, 0) == 0) || (errno != ESRCH)) {
			NETERROR(MRSD, ("%s seems to be running already - exiting\n", basename(argv[0])));
			exit(0);
		}
		else  /* Get rid of the leftover file */
			UnlinkPid(pidf);
	}

	StorePid(pidf);

	Initpoll(RSD_ID, SERPLEX_GID);
	Sendpoll(0);

	clicmdtmout.tv_sec = 5;
	CliCmdThreadInit();

	Pthread_mutex_lock(&(host_dbrev.mutex));
	host_dbrev.value = ReadDBRevNum();
	Pthread_mutex_unlock(&(host_dbrev.mutex));

	if (rs_wait_for_cache) {
		/* Let's wait for the cache to get ready */
		for (i = POLL_TIME_OUT; (CacheAttach() <= 0); i--) {
			NETDEBUG(MRSD, NETLOG_DEBUG2, ("waiting for Cache to become active\n"));
			sleep(1);
			if (i == 0) {
				i = POLL_TIME_OUT;
				Sendpoll(0);
			}
		}

		/* Finally we are ready */
		CacheDetach();
	}

#ifndef NETOID_LINUX
	ThreadInitRT();

	ThreadSetRT();
#endif // NETOID_LINUX

	/* Change the directory to the DEF_MSW_BASE_DIR */
	if ( ((basedir = getenv("SERPLEXPATH")) || (basedir = DEF_MSW_BASE_DIR) ) && 
			chdir(basedir) ) { 
		NETERROR(MRSD, ("Error changing to dir - %s: %s\n", basedir, \
			strerror(errno)));
        fprintf(stderr, "Error changing to dir - %s: %s\nrsd exiting ...", 
			basedir, strerror(errno));
		exit(1);
	}

	/* Create a temporary directory to do rsd work */
	if ((mkdir(rs_tmp_dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0) \
		&& (errno != EEXIST)) {
		NETERROR(MRSD, ("CreateDir: creating %s error - %s\n", rs_tmp_dir, \
		strerror(errno)));
	}

#if 0
	if (ThreadLaunch2((PFVP)RunRSSD, NULL, 1,
		PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, PTHREAD_CREATE_DETACHED, 
		&rscp->tids[THREAD_RSSD]) != 0) {
		NETERROR(MRSD, ("Failed to start the status daemon\n"));
	}
#endif

	if (ThreadLaunch2((PFVP)RunHelloProtocol, NULL, 1,
		PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, PTHREAD_CREATE_DETACHED,
		&rscp->tids[THREAD_HELLO]) != 0) {
		NETERROR(MRSD, ("Failed to start the Hello Protocol\n"));
	}

	Pthread_mutex_lock(&(hellocv.mutex));
	while (hellocv.value != 1) 
		Pthread_cond_wait(&(hellocv.cond), &(hellocv.mutex));
	Pthread_mutex_unlock(&(hellocv.mutex));

	if (rs_start) {
		// Set callbacks to inform rsd whether it should start or stop.
		rsd_cb = (AppCB *)Malloc(sizeof(AppCB));
		rsd_cb->cb_init = (PFI)RS_Init;
		rsd_cb->init_arg = rscp;
		rsd_cb->cb_kill = (PFI)RS_Kill;
		rsd_cb->kill_arg = rscp;

		HelloRegCb(rsd_cb);
		Free(rsd_cb);
	}

	for(;;) {
		Sendpoll(0);
#ifdef _DMALLOC_
		dmalloc_message("starting new log");
		mark = dmalloc_mark();
#endif
		sleep(POLL_TIME_OUT);
#ifdef _DMALLOC_
		dmalloc_log_changed(mark, 1, 0, 1);
		dmalloc_message("end of log");
#endif
	}

	pthread_mutex_destroy(&(rscp->tid_mutex));
	exit(0);
}

int 
RS_Init(RSConfp rscp, HelloConfp hp)
{
	char fn[] = "RS_Init";

	//LockGetLock(hcp_lock, 0, 0);
	NETDEBUG(MRSD, NETLOG_DEBUG1, ("%s callback function called \n", fn));
	NETINFOMSG(MRSD, ("Replication server initializing as %s\n", 
		(hp->role == RS_MASTER) ? ("Master") : ("Slave")));

	rs_cur_ind = ReadSeqNum();

	gethostname(rs_hostname, RS_LINELEN);

	rscp->role = hp->role;

	nx_strlcpy(rs_cli_cmd_str, RS_CLI_CMD_STR, sizeof(rs_cli_cmd_str));

	snprintf(rs_slave_cli_cmd_str, sizeof(rs_slave_cli_cmd_str),"%s %s %s %s %s ",
			 RS_CLI_CMD_STR, RS_CLI_SLAVE_SUFF, RS_CLI_NO_OUT_SUFF, 
			 RS_CLI_DIR_SUFF, rs_tmp_dir);
	snprintf(rs_reg_cli_cmd_str, sizeof(rs_reg_cli_cmd_str),"%s %s %s %s ",
			 RS_CLI_CMD_STR, RS_CLI_SLAVE_SUFF, RS_CLI_NO_OUT_SUFF, RS_CLI_REG_SUFF);
	if Is_Sec_Serv(rscp)
		nx_strlcpy(rs_cli_cmd_str, rs_slave_cli_cmd_str, sizeof(rs_cli_cmd_str));
#if 0
	nx_strlcpy(rs_slave_cli_cmd_str, RS_CLI_CMD_STR, 
		sizeof(rs_slave_cli_cmd_str));
	nx_strlcat(rs_slave_cli_cmd_str, RS_CLI_SLAVE_SUFF, 
		sizeof(rs_slave_cli_cmd_str));
	nx_strlcat(rs_slave_cli_cmd_str, RS_CLI_DIR_SUFF, 
		sizeof(rs_slave_cli_cmd_str));
	nx_strlcat(rs_slave_cli_cmd_str, rs_tmp_dir, 
		sizeof(rs_slave_cli_cmd_str));

	nx_strlcpy(rs_reg_cli_cmd_str, rs_slave_cli_cmd_str, 
		sizeof(rs_slave_cli_cmd_str));
	nx_strlcat(rs_reg_cli_cmd_str, RS_CLI_REG_SUFF, 
		sizeof(rs_reg_cli_cmd_str));
	if Is_Sec_Serv(rscp)
		nx_strlcat(rs_cli_cmd_str, RS_CLI_SLAVE_SUFF, sizeof(rs_cli_cmd_str));
#endif

	Pthread_mutex_lock(&(host_dbrev.mutex));
	host_dbrev.value = ReadDBRevNum();
	Pthread_mutex_unlock(&(host_dbrev.mutex));

	if (Is_Pri_Serv(rscp)) {
	/* Start the copy server */
	ThreadLaunch2((PFVP)start_copy_server, (void *)rscp, 1,
		PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, PTHREAD_CREATE_JOINABLE, 
		&rscp->tids[THREAD_COPY_SERVER]);
	}

	/* Do it all */
	ThreadLaunch2((PFVP)start_server, (void *)rscp, 1,
		PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, PTHREAD_CREATE_JOINABLE,
		&rscp->tids[THREAD_REPL_SERVER]);

	//LockReleaseLock(hcp_lock);

	return(0);
}

int
RS_Kill(RSConfp rscp, int role)
{
	int i;
	int	rval;
	char	cmd[RS_LINELEN];

	NETDEBUG(MRSD, NETLOG_DEBUG1, ("RS_Kill callback function called \n"));

	for (i = 1; i < THREAD_MAX; i++) {
		if (rscp->tids[i] != NULL_TID) {
			NETDEBUG(MRSD, NETLOG_DEBUG2, ("thread %d - pthread_cancel called\n", 
				rscp->tids[i]));
			if ((rval = pthread_cancel(rscp->tids[i])) != 0) {
				NETERROR(MRSD, ("pthread_cancel: thread id %d - %s\n",
					rscp->tids[i], strerror(rval)));
			}
			if ( i < THREAD_MIN ) {
				Pthread_join(rscp->tids[i], NULL);
				NETDEBUG(MRSD, NETLOG_DEBUG2, ("thread id %d - joined\n", 
					rscp->tids[i]));
			}
			Pthread_mutex_lock(&(rscp->tid_mutex));
			rscp->tids[i] = NULL_TID;
			Pthread_mutex_unlock(&(rscp->tid_mutex));
		}
	}

	snprintf(cmd, RS_LINELEN, "%s rsd clear", RS_CLI_CMD_STR);

//	ExecuteCliCommand(cmd);
	System(cmd);
	NETDEBUG(MRSD, NETLOG_DEBUG2, ("executed - %s\n", cmd)); 

	return(0);
}

void
start_server(RSConfp rscp)
{
	int			maxfdp1;
	int			connfd, locfd, listenfd = 0;
	int			maxi, nready, *client;
	int 		i;
	fd_set		rset, allset;
	Msg			*msgp;
	socklen_t 	clilen;
	struct sockaddr_un	cliaddr;
	char 		tmpstr[40];
	char 		fn[] = "start_server";
	struct sockaddr_in	*sap;


#if 0
	/* We assume that this function is called by a thread */
	if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
		NETERROR(MRSD, ("%s: pthread_setcanceltype error - %s\n", fn, strerror(errno)));
	}
#endif

	/* Disable canceling */
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	Pthread_mutex_lock(&(rscp->replcv.mutex));
	while (rscp->replcv.value != 0)
		Pthread_cond_wait(&(rscp->replcv.cond), &(rscp->replcv.mutex));

	/* If the Server is secondary, get the database from the primary, this
	   also sets the rs_cur_ind to the new database */
	sap = (struct sockaddr_in *)(hcp->pri_sa);

	if (Is_Sec_Serv(rscp)) {
		if (GetDB((struct sockaddr *)sap, (socklen_t)(sizeof(struct sockaddr_in))) != 0) {
			NETERROR(MRSD, ("%s: Initial Database acquisition from %s:%d failed\n", fn,
			FormatIpAddress(ntohl(sap->sin_addr.s_addr), tmpstr), sap->sin_port));
		}
		else
			db_inited = 1;
	}

	/* set up the multicast server */
	rscp->salen = sizeof(struct sockaddr_in);
	if (mcast_serv(rs_mcast_addr, rs_port, rs_ifname, &(rscp->sendfd),
			&(rscp->recvfd), (void **)&(rscp->sa)) < 0) {
		NETERROR(MRSD, ("Unable to set up sockets for multicast communication\n"));
	}

	/* Signal that start_server has started */
	rscp->replcv.value = 1;
	Pthread_cond_signal(&(rscp->replcv.cond));
	Pthread_mutex_unlock(&(rscp->replcv.mutex));
	
	/* Enable canceling */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	/* prepare multicast fds for select() */
	maxi = -1;
	FD_ZERO(&rset);
	FD_ZERO(&allset);

	FD_SET(rscp->recvfd, &allset);
	maxfdp1 = det_fd(rscp->recvfd, 0);

	FD_SET(rscp->sendfd, &allset);
	maxfdp1 = det_fd(rscp->sendfd, maxfdp1);

	rscp->loc_fds = client = Calloc(FD_SETSIZE, sizeof(int));

	/* If rscp server is primary then create a server for local communication */
	if Is_Pri_Serv(rscp) {
		/* Create a socket for local communication */
		rscp->listenfd = unix_serv(RS_STR_FNAME, MAX_CLIENTS);
		listenfd = rscp->listenfd;
		if (listenfd < 0) {
			NETERROR(MRSD, ("Failed to start server for local communication\n"));
			exit(-1);
		}
		FD_SET(listenfd, &allset);
		maxfdp1 = det_fd(listenfd, maxfdp1);

		for(i = 0; i < FD_SETSIZE; i++)
			*(client + i) = -1;

		/* Do it all */
		ThreadLaunch2((PFVP)SendSeqNum, (void *)rscp, 1,
			PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, PTHREAD_CREATE_JOINABLE,
			&rscp->tids[THREAD_SNO_SERVER]);
	}

	/* Push the cleanup Handler */
	pthread_cleanup_push(repl_serv_end, (void *)rscp);

	/* Check if any of the sockets is readable */
	for ( ; ; ) {
		/* Enable canceling */
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		rset = allset;
		nready = Select(maxfdp1, &rset, NULL, NULL, NULL);

		/* Disable canceling */
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		/* Remote socket is readable with a Multicast Pkt*/
		if (FD_ISSET(rscp->recvfd, &rset)) {
			nready--;
			msgp = ReadNetPkt(rscp, rscp->recvfd);
			ProcessNetRcvd(rscp, msgp);
			FreeMsg(msgp);
			if (--nready <= 0)
				continue;
		}

		/* Remote socket is readable with a Unicast Pkt*/
		if (FD_ISSET(rscp->sendfd, &rset)) {
			nready--;
			msgp = ReadNetPkt(rscp, rscp->sendfd);
			ProcessNetRcvd(rscp, msgp);
			FreeMsg(msgp);
			if (--nready <= 0)
				continue;
		}

		if Is_Pri_Serv(rscp) {
			/* Local client trying to connect */
			if (FD_ISSET(listenfd, &rset)) { 
				clilen = sizeof(cliaddr);
				connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
				for(i=0; i < FD_SETSIZE; i++)
					if (*(client + i) < 0) {
						*(client + i) = connfd;
						break;
					}		
				if (i == FD_SETSIZE)  {
					NETERROR(MRSD, ("Too many local cli clients\n"));
					exit(-2);
				}
				/* Add the new fd to the select fdset */
				FD_SET(connfd, &allset);
				maxfdp1 = det_fd(connfd, maxfdp1);
				maxi = max(i, maxi); 
		
				if (--nready <= 0)
					continue;
			}			
			
			/* Check all local clients for data */
			for (i = 0; ((i <= maxi) && (nready > 0)); i++) { 
				if ( (locfd = *(client + i)) < 0)
					continue;
				if FD_ISSET(locfd, &rset) {
					if ((msgp = CreatePkt(rscp, locfd)) == NULLMSG) {
					/* Client closed connection*/
						Close(locfd);
						FD_CLR(locfd, &allset);
						*(client + i) = -1;
						continue;
					}
					SendNetPkt(rscp, msgp);
					FreeMsg(msgp);
					--nready;
				}
			}	/* End - for */
		}	/* End - if */

	}	/* End - loop for select */	

	/* Pop the cleanup Handler */
	pthread_cleanup_pop(0);

}	/* End - startserver */

void
start_copy_server(RSConfp rscp)
{
	int			rcmdfd, connfd = -1;
	int			nconn;
	int			i;
	socklen_t 	clilen;
	struct sockaddr_un 	cliaddr;
	fd_set		rset, lset;	
	in_port_t	port;
#if 0
	char		fn[] = "start_copy_server";

	/* We assume that this function is called by a thread */
	if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
		NETERROR(MRSD, ("%s: pthread_setcanceltype error - %s\n", fn, strerror(errno)));
	}
#endif

	Pthread_mutex_lock(&(rscp->copycv.mutex));
	while (rscp->copycv.value != 0)
		Pthread_cond_wait(&(rscp->copycv.cond), &(rscp->copycv.mutex));
	
	/* Create a socket for remote communication */
	port = atoi(rs_port);
	rcmdfd = tcp_serv(port, MAX_CLIENTS);
	if (rcmdfd < 0) {
		NETERROR(MRSD, ("Failed to start copy server \n"));
	}
	rscp->rcmdfd = rcmdfd; 

	rscp->copycv.value = 1;
	Pthread_cond_signal(&(rscp->copycv.cond));
	Pthread_mutex_unlock(&(rscp->copycv.mutex));
	
	FD_ZERO(&lset);
	FD_SET(rscp->rcmdfd, &lset);

	/* Push the cleanup handler */
	pthread_cleanup_push(copy_serv_end, (void *)rscp);

	for ( ; ; ) {

		clilen = sizeof(cliaddr);
		
		rset = lset;

/*
 * 	The following select call is to make sure that 
 *	an lpthread_cancel is caught when blocked in the call. 
 *	accept does not catch the cancel signal.
 */
		if ( (nconn = Select(rcmdfd + 1, &rset, NULL, NULL, NULL)) && 
			(connfd = Accept(rcmdfd, (SA *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;
			else {
				NETERROR(MRSD, ("start_copy_server: accept error\n"));
			}
		}

		/* Disable canceling */
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

#if 1
		for (i = THREAD_MIN; i <= THREAD_MAX; i++) {
			if (rscp->tids[i] == NULL_TID)
				break;
		}

		if (i > THREAD_MAX) {
			NETERROR(MRSD, ("All %d threads busy. No thread for ProcessDBSync\n",
				THREAD_MAX));
		}
		else if (connfd > 0) {
		ThreadLaunch2((PFVP)ProcessDBSync, (void *)connfd, 1, 
			PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, PTHREAD_CREATE_DETACHED,
			&(rscp->tids[i]));
		}
#endif

		/* Enable canceling */
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

   	}	/* End - for loop */

	/* Pop the cleanup handler */
	pthread_cleanup_pop(0);

}	/* End - start_copy_server */

void
SendSeqNum(RSConfp rscp)
{
	Msg     msg; 
	char	sendbuf[RS_LINELEN];
	RSPkt	*pktp = (RSPkt *)sendbuf;
	Cmd		*cmdp = ((Cmd *)(pktp+1));
#if 0
	char	fn[] = "SendSeqNum";

	/* We assume that this function is called by a thread */
	if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
		NETERROR(MRSD, ("%s: pthread_setcanceltype error - %s\n", fn, strerror(errno)));
	}
#endif

	for (;;) {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		bzero(sendbuf, sizeof(sendbuf));
		bzero(&msg, sizeof(Msg));

		cmdp->cmdseq = htonl(rs_cur_ind);
		cmdp->cmdtyp = htonl(CMD_SNO);
		cmdp->cmdrval = htonl(0);
		cmdp->cmdact = htonl(0);
		cmdp->cmdlen = htonl(sizeof(Cmd));
		pktp->type |=  (PKT_FROM_PRI | CLI_SNO_BCAST);
		pktp->datalen = sizeof(Cmd);
		msg.Pktp = pktp;

		SendNetPkt(rscp, &msg);	
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		sleep(rs_ssn_int); 			
	}
}

void
repl_serv_end(void *arg)
{
	RSConfp		rsp = arg;
	int			i;
	int			*client = rsp->loc_fds;

	Pthread_mutex_lock(&(rsp->replcv.mutex));
	while (rsp->replcv.value != 1)
		Pthread_cond_wait(&(rsp->replcv.cond), &(rsp->replcv.mutex));
	
	NETDEBUG(MRSD, NETLOG_DEBUG2, ("Ending thread %d  - repl_serv_end called\n", 
		pthread_self()));
	close(rsp->recvfd);
	close(rsp->sendfd);
	if (rsp->listenfd != -1)
		close(rsp->listenfd);
	for(i = 0; i < FD_SETSIZE; i++) {
		if (*(client + i) >= 0)
			close(*(client +i));
	}
	Free(client);
	Free(rsp->sa);

	rsp->replcv.value = 0;
	Pthread_cond_signal(&(rsp->replcv.cond));
	Pthread_mutex_unlock(&(rsp->replcv.mutex));
}

void
copy_serv_end(void *arg)
{
	RSConfp		rsp = arg;

	Pthread_mutex_lock(&(rsp->copycv.mutex));
	while (rsp->copycv.value != 1)
		Pthread_cond_wait(&(rsp->copycv.cond), &(rsp->copycv.mutex));
	
	NETDEBUG(MRSD, NETLOG_DEBUG2, ("Ending thread %d - copy_serv_end called\n", 
		pthread_self()));
	if (rsp->rcmdfd != -1)
		close(rsp->rcmdfd);

	rsp->copycv.value = 0;
	Pthread_cond_signal(&(rsp->copycv.cond));
	Pthread_mutex_unlock(&(rsp->copycv.mutex));
}

int
ConfigureRSD(void)
{
	char	cmd[RS_LINELEN], s[RS_LINELEN];

	// Process Configuration, read from the config file

	match = FindServerConfig();

	if (match == -1) {
		NETERROR(MRSD, ("Not configured to run...\n"));
		exit(0);
	}

	if (serplexes[match].location.type == CONFIG_LOCATION_NONE) {
		NETERROR(MRSD, ("Not Configured to run in server configuration file\n"));
	}

	ServerSetLogging(pname, &serplexes[match]);

	/* Issue a route add command for the multicast host */
	snprintf(cmd, (size_t)RS_LINELEN, 
	"/usr/sbin/route add %s %s -interface >/dev/null 2>&1\n", 
	rs_mcast_addr, FormatIpAddress(ntohl(GetIpAddr(rs_ifname)), s));	
	
	system(cmd);

	return(0);
}

sigset_t 			async_signal_mask;
struct sigaction	sigact;
stack_t				s;

//
//	Function :
//		SignalInit()
//
//	Description :
//		Setup process wide signal handling, adhered to
//		by all threads via sigaction(2). This routine
//		should be called very early in the startup
//		process after daemonizing and prior to starting
//		any threads. In this way the blocked signal mask
//		will be inheritted by any threads.
//
static void
SignalInit(void)
{
	// Setup up alternate signal stack to be 
	// used by syncronous signal handler

	s.ss_sp = malloc( SIGSTKSZ );
	s.ss_size = SIGSTKSZ;
	s.ss_flags = 0;

	if ( sigaltstack( &s, 0 ) == -1 )
		perror( "sigaltstack" );

	// Create mask for blocking asyncronous signals
	// They must be blocked for sigwait() to work!!

	sigemptyset( &async_signal_mask );

	sigaddset( &async_signal_mask, SIGTERM );
	sigaddset( &async_signal_mask, SIGPOLL );
	sigaddset( &async_signal_mask, SIGCHLD );
	sigaddset( &async_signal_mask, SIGINT );
	sigaddset( &async_signal_mask, SIGHUP );
	sigaddset( &async_signal_mask, SIGALRM );

	// Setup SIGCHLD special case

	sigemptyset( &sigact.sa_mask );
	sigact.sa_flags = (SA_RESTART|SA_NOCLDSTOP|SA_ONSTACK);
	sigact.sa_handler = SIG_IGN;

	sigaction( SIGCHLD, &sigact, NULL );

	sigset( SIGCHLD, HandleCHLD );

	// Setup async signals to be ignored

	sigact.sa_flags = (SA_RESTART|SA_ONSTACK);

	sigaction( SIGPIPE, &sigact, NULL );
	sigaction( SIGWINCH, &sigact, NULL );
	sigaction( SIGTTIN,	&sigact, NULL );
	sigaction( SIGTTOU, &sigact, NULL );

	// Setup Signal handler to be called for
	// syncronous signals (fatal traps)

	sigact.sa_handler = SyncSigHandler;
	sigact.sa_flags |= (SA_RESTART|SA_ONSTACK);

	sigaction(SIGBUS,	&sigact, NULL );
#ifndef NETOID_LINUX
	sigaction(SIGEMT,	&sigact, NULL );
#endif // NETOID_LINUX
	sigaction(SIGFPE,	&sigact, NULL );
	sigaction(SIGILL,	&sigact, NULL );
	sigaction(SIGSEGV,	&sigact, NULL );

	// Setup block mask for asyncronous signals to be
	// processed

	pthread_sigmask( SIG_BLOCK, &async_signal_mask, NULL );

	// Launch thread to process asyncronous signals

	ThreadLaunch( AsyncSigHandlerThread, NULL, 1);
}

//
//	Function :
//		AsyncSigHandlerThread()
//
//	Description :
//		This routine contains logic for a thread
//		to handle asyncronous signals in a syncronous
//		fashion for the rsd process. The thread is 
//		spawned at initialization time. sigwait(2)
//		is used to process the signals correctly.
//		The routine is not a signal handler so any
//		function call may be called from it.
//
static void *
AsyncSigHandlerThread( void * args )
{
	char			c_sig[256];
	int				signo;
	extern	void	restart_ispd(void);

	memset( c_sig, (int) 0, 256 );

	for (;;) {

		sigwait( &async_signal_mask, &signo );

		nx_sig2str( signo, c_sig, sizeof(c_sig) );

		switch (signo) {

		case SIGINT:
			HandleINT(signo);
			break;

		case SIGTERM:
			HandleTERM(signo);
			break;

		case SIGCHLD:
			HandleCHLD(signo);
			break;

		case SIGHUP:
			HandleHUP(signo);
			break;

		case SIGUSR1:
			HandleUSR1(signo);
			break;

		case SIGUSR2:
			HandleUSR2(signo);
			break;

		default:
			NETERROR(	MRSD,
						("Caught %s signal - ignoring\n",
						c_sig ));
			break;
		}
	}
}

//
//	Function :
//		SyncSigHandler()
//
//	Description :
//		This routine is a signal catcher used for
//		Synchronous signals. (fatal traps)
//
static void
SyncSigHandler(int signo)
{
	int32_t thread = pthread_self();
	char	c_sig[32];

	nx_sig2str( signo, c_sig, sizeof(c_sig) );

	switch (signo) {
		case SIGBUS:
#ifndef NETOID_LINUX
		case SIGEMT:
#endif // NETOID_LINUX
		case SIGFPE:
		case SIGILL:
		case SIGSEGV:
			abort();
			break;
		default:
			NETERROR(	MRSD,
					("Caught %s signal in LWP %d\n",
					c_sig,
					thread ));
			break;
	}
}

static void
HandleUSR1(int sig)
{
	char fn[] = "HandleUSR1";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: Signal SIGUSR1 caught - calling RSKill\n", fn));
	RS_Kill(the_rscp, 0); 		/* Second argument is dummy */
}

static void
HandleUSR2(int sig)
{
	char fn[] = "HandleUSR2";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: Signal SIGUSR2 caught - calling RSInit\n", fn));
	RS_Init(the_rscp, hcp); 	
}

static void
HandleINT(int sig)
{
	char fn[] = "HandleINT";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: Signal SIGINT caught - stopping process\n", fn));
	NETINFOMSG(MRSD, ("*** NexTone Replication Server shutdown: started ***\n"));
	RS_Kill(the_rscp, 0);
	exit(0);
}

static void
HandleTERM(int sig)
{
	char fn[] = "HandleTERM";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: Signal SIGTERM caught - stopping process\n", fn));
//	RS_Kill(the_rscp, 0);
	exit(0);
}

static void
HandleHUP(int sig)
{
	char fn[] = "HandleHUP";

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: Signal SIGHUP caught - Reconfiguring not supported\n",
		fn));
//	exit(0);
	return;
}

static void
HandleCHLD(int sig)
{
	char fn[] = "HandleCHLD";
	pid_t		pid;
	int 		stat;

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: Signal SIGCHLD caught - waiting for child\n", fn));

	while ((pid = waitpid((pid_t)-1, &stat, WNOHANG)) > 0)
		fprintf(stderr, "Child terminated\n");
	return;
}

// create a pool of threads to execute cli cmds
void
CliCmdThreadInit(void)
{
	clipoolid = ThreadPoolInit("clicmd", nclithreads, PTHREAD_SCOPE_PROCESS, -1, 0);
	cliclassid = ThreadAddPoolClass("clicmd", clipoolid, 0, 0);
	ThreadPoolStart(clipoolid);
}

int
ExecuteCliCommand(char *cmd)
{
	//pthread_t 	clitid;
	int			i = 0;
	int 		ret;
	void 		*args[2];
	cvtuple		statcv = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 1};

	// statcv.value is initialized to 1. Because ProcessCommand status is always
	// less than equal to 0. If this changes, we may need a change in the above line

	args[0] = cmd;
	args[1] = (char *)&statcv;

//	ThreadLaunch2((PFVP)CliCmdWorker, (void *)args, 1,
//		PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, PTHREAD_CREATE_DETACHED, &clitid);

	if (ThreadDispatch(clipoolid, cliclassid, (PFVP)CliCmdWorker,
		(void *)args, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0) {
	}

	Pthread_mutex_lock(&(statcv.mutex));
	while ((statcv.value > 0) ) {
		errno = 0;
		Pthread_cond_timedwait(&(statcv.cond), &(statcv.mutex), &clicmdtmout);
		if (errno == ETIMEDOUT) {
			i++;
			NETDEBUG(MRSD, NETLOG_DEBUG1, ("cli command - %s  taking longer than %ld secs\n",
				cmd, i*clicmdtmout.tv_sec	));
		}
	}
	ret = statcv.value;
	Pthread_mutex_unlock(&(statcv.mutex));

	return ret;
}

void
CliCmdWorker(char *args[])
{
	int 	argc;
	char 	*argv[254];
	int		i = 0;
	int 	ret;
	char	*cmd;
	cvtuple	*cvp;

	cmd = (char *)args[0];
	cvp = (cvtuple *)args[1];

	ExtractArgs(cmd, &argc, argv, 254);
	
	if (!strncmp(argv[0]+strlen(argv[0])-3, "cli", 3)) {
		//skip "cli"
		i = 1;
	}

	ret = ProcessCommand(argc - i, argv + i);

	Pthread_mutex_lock(&(cvp->mutex));
	cvp->value = ret;
	Pthread_cond_signal(&(cvp->cond));
	Pthread_mutex_unlock(&(cvp->mutex));

	NETDEBUG(MRSD, NETLOG_DEBUG4, ("%s\n", (ret == xleOk) ? "success" : "error"));

	return;
}

void
parse_rs_opt(int argc, char *argv[])
{
	int	c, errflg = 0;
	char usage[] = "usage: rsd [-d] [-v] [-w] [-h] [-r]\n\t-d does not daemonize\n\t-v print version\n\t-w do not wait for the cache\n\t-r do not start rs, just hello protocol";

	while ((c = getopt(argc, argv, "dvwr")) != EOF)
		switch (c) {
			case 'd':
				rs_daemonize = 0;
				break;
			case 'h':
				fprintf(stdout, "%s\n", usage);
			case 'v':
				fprintf(stdout, "%s %s.%s, %s\n%s\n",
					RS_NAME, 
					RS_VERSION, 
					RS_MINOR,
					RS_BUILDDATE,
					RS_COPYRIGHT);
				fprintf(stdout, "\n");
				exit(0);
				break;
			case 'w':
				rs_wait_for_cache = 0;
				break;
			case 'r':
				rs_start = 0;
				break;
			default:
				errflg++;
				break;
		}

	if (errflg) 
		err_quit("%s\n", usage);
}
