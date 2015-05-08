#include	"unp.h"
#include	"execd.h"
#include	"rs.h"
#include	"flock.h"
#include	"nxosd.h"

Msg		*CreatePktQry(long seqno);

int
ProcessNetRcvd(RSConfp rscp, Msg *msgp)
{
	RSPkt			*pktp = msgp->Pktp;
	Cmd			*cmdp;
	int			rval = 0;
	CliEntry	**clipp, **space;
	HDB			*hdbp;

	NETDEBUG(MRSD, NETLOG_DEBUG3, ("ProcessNetRcvd: our server = %s, PktType = %s\n", 
		ServTyp(rscp->role), PacketTyp(PktTyp(pktp))));

	/* If a valid packet for a primary server */
	if Valid_Pkt_Pri(rscp, pktp) {
		if (PktTyp(pktp) & CLI_CMD_SEND) {
			cmdp = (Cmd *)(PktDatap(pktp));
			/* Convert the cmd header into the host endian-ness*/
			CmdSeq(cmdp) = ntohl(CmdSeq(cmdp));		
			CmdLen(cmdp) = ntohl(CmdLen(cmdp));		
			CmdTyp(cmdp) = ntohl(CmdTyp(cmdp));		
			CmdRval(cmdp) = ntohl(CmdRval(cmdp));		
			CmdAct(cmdp) = ntohl(CmdAct(cmdp));		

			hdbp = OpenCliHist();
			space = FindCliEntries(hdbp, CmdSeq(cmdp));
			CloseCliHist(hdbp);
			
			clipp = space; 				/* First entry */

			if (*clipp == NULL) {		
			/* There are no entries or error condition indicated. Assume buffer overflow. */
			/* We send a seqnum broadcast, so the slave can synchronize */

				Msg     msg; 

				bzero(&msg, sizeof(Msg));
				pktp = (RSPkt *)Calloc(RS_LINELEN, 1);
				cmdp = ((Cmd *)(pktp+1));

				cmdp->cmdseq = htonl(rs_cur_ind);
				cmdp->cmdtyp = htonl(CMD_SNO);
				cmdp->cmdrval = htonl(0);
				cmdp->cmdact = htonl(0);
				cmdp->cmdlen = htonl(sizeof(Cmd));
				pktp->type |=  (PKT_FROM_PRI | CLI_SNO_BCAST);
				pktp->datalen = sizeof(Cmd);
				msg.Pktp = pktp;

				SendNetPkt(rscp, &msg);	
				Free(pktp);
			}

			while(*clipp != NULL) {
				pktp = Calloc(MAX_PKT_LEN, 1);		/* Create a new Pkt to send reply */
				pktp->datalen += EntsToBuf(&clipp, PktDatap(pktp), MAX_PKT_LEN - sizeof(RSPkt));
				PktTyp(pktp) |= CLI_CMDS; 
				msgp->Pktp = pktp;
				SendNetPkt(rscp, msgp);
				Free(pktp);
			}					
			Free(space);

		}
		else 
			goto PNR_err;
	}

	/* If a valid packet for a secondary server */
	else if Valid_Pkt_Sec(rscp, pktp) {
		if (db_inited == 0) {		/* We just started. Copy the database */
			if ((rval = GetDB(msgp->pa, msgp->palen)) != 0) {
				NETERROR(MRSD, ("Failed to sync db\n"));
			}
			else
				db_inited = 1;
		}
		else if (PktTyp(pktp) & CLI_CMDS) {
			/* Execute each cli cmd */
			cmdp = (Cmd*)(PktDatap(pktp));
			while( cmdp != NULL ) {

				/* Convert the cmd header into the host endian-ness*/
				CmdSeq(cmdp) = ntohl(CmdSeq(cmdp));		
				CmdLen(cmdp) = ntohl(CmdLen(cmdp));		
				CmdTyp(cmdp) = ntohl(CmdTyp(cmdp));		
				CmdRval(cmdp) = ntohl(CmdRval(cmdp));		
				CmdAct(cmdp) = ntohl(CmdAct(cmdp));		
	
				rval = ProcessCmdWHist(msgp, cmdp);
				if (rval == RS_ERR_SEQ_NUM) {	/* The sequence number shows break in sequence */
					SendPktQry(rscp, rs_cur_ind+1, msgp->pa, msgp->palen);  
					break;			/* Can not execute further messages */
				}
				if (rval == RS_ERR_NEED_SYNC) {
					rval = GetDB(msgp->pa, msgp->palen);
				}
				else if (rval != 0) {
					NETERROR(MRSD, ("Cli Cmd exec error: %d\n", rval));
					break;
				}
	
				cmdp = SkipCmd(pktp, cmdp);
			}
		}

		else if (PktTyp(pktp) & CLI_SNO_BCAST) {
			/* Check the command for sequence number */
			cmdp = (Cmd*)(PktDatap(pktp));
			/* Convert the cmd header into the host endian-ness*/
			CmdSeq(cmdp) = ntohl(CmdSeq(cmdp));		
			CmdLen(cmdp) = ntohl(CmdLen(cmdp));		
			CmdTyp(cmdp) = ntohl(CmdTyp(cmdp));		
			CmdRval(cmdp) = ntohl(CmdRval(cmdp));		
			CmdAct(cmdp) = ntohl(CmdAct(cmdp));		

			NETDEBUG(MRSD, NETLOG_DEBUG4, ("Received a seq num bcast with seq no = %ld\n",
				cmdp->cmdseq));
			if (rs_cur_ind != cmdp->cmdseq) {
				NETERROR(MRSD, ("Our db index does not match sender's. Retrieving whole database \n"));
				rval = GetDB(msgp->pa, msgp->palen);
				if (rval != 0) {
					NETERROR(MRSD, ("Failed to sync db: %s\n", strerror(errno)));
				}
			}
		}
		else 
			goto PNR_err;
	}

	/* If a valid registration packet from any server */
	else if Valid_Reg_Pkt(pktp) {
		if (PktTyp(pktp) & CLI_CMDS) {
			/* Execute each cli cmd */
			cmdp = (Cmd*)(PktDatap(pktp));
			while( cmdp != NULL ) {
	
				/* Convert the cmd header into the host endian-ness*/
				CmdSeq(cmdp) = ntohl(CmdSeq(cmdp));		
				CmdLen(cmdp) = ntohl(CmdLen(cmdp));		
				CmdTyp(cmdp) = ntohl(CmdTyp(cmdp));		
				CmdRval(cmdp) = ntohl(CmdRval(cmdp));		
				CmdAct(cmdp) = ntohl(CmdAct(cmdp));		
	
				rval = ProcessCmdWOHist(msgp, cmdp);
				if (rval != 0) {
					NETERROR(MRSD, ("Cli Cmd exec error: %d\n", rval));
					break;
				}
	
				cmdp = SkipCmd(pktp, cmdp);
			}
		}
		else 
			goto PNR_err;
	}

	else 
		goto PNR_err;

	return(rval);

PNR_err: 
	NETERROR(MRSD, ("ProcessNetRcvd: Invalid Packet Received. our server = %s, PktType = %s\n", 
		ServTyp(rscp->role), PacketTyp(PktTyp(pktp))));
	rval = -1;
	
	return(rval);
}

/*
* Send the Packet over network. 
* If the destination address is not set then send it to the
* multicast address. 
*/
int
SendNetPkt(RSConfp rscp, Msg *msgp)
{
	RSPkt		*pktp =msgp->Pktp;
	int			pktlen;
	
	if (Is_Pri_Serv(rscp))
		pktp->type |= PKT_FROM_PRI;
	if (Is_Sec_Serv(rscp))
		pktp->type |= PKT_FROM_SEC;

	PktTyp(pktp) = htonl(PktTyp(pktp));
	pktlen = PktLen(pktp);
	DataLen(pktp) = htonl(DataLen(pktp));

	/* Check if the message contains the destination address */
	if(msgp->palen != 0) 
		Sendto(rscp->sendfd, pktp, pktlen, 0, msgp->pa, msgp->palen);
	else
		Sendto(rscp->sendfd, pktp, pktlen, 0, rscp->sa, rscp->salen);

	return(0);
}

/*
* Read packet from the socket and do the pre-processing
* to retrieve packet information to pass along to the Processing 
* Module
*/
Msg*
ReadNetPkt(RSConfp rscp, int fd)
{
	char	*buf;
	Msg		*msgp;
	RSPkt	*pktp;
	char 	from[RS_LINELEN];

	msgp = (Msg *)Calloc(sizeof(Msg), 1);
	msgp->palen = rscp->salen;
	msgp->pa = Calloc(msgp->palen, 1);
	buf = Calloc(MAXBUF, 1);
	pktp = msgp->Pktp = (RSPkt *)buf;

	if(Recvfrom(fd, buf, MAXBUF, 0, msgp->pa, &(msgp->palen)) < 0) {
		NETERROR(MRSD,("ReadNetPkt: %s", strerror(errno)));
		return(NULLMSG);
	}

	PktTyp(pktp) = ntohl(PktTyp(pktp));
	DataLen(pktp) = ntohl(DataLen(pktp));

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("Received from %s\n", Sock_ntop_r(msgp->pa, msgp->palen, from, sizeof(from))) );
	return(msgp);	/* Return the Packet */
}

/*
 *Create the Packet to be sent to remote
 * destination
 */
Msg*
CreatePkt(RSConfp rcsp, int fd)
{
	char	cliline[MAXLINE], *line;
	int		n;
	long	cmdseq;
	Msg		*msgp;
	RSPkt	*sendbuf;

	
	bzero(cliline,MAXLINE);
	if((n = Read(fd, cliline, MAXLINE)) == 0) {
		return(NULLMSG);
	}

	msgp = Calloc(sizeof(Msg), 1);
	/* Zero out the pkt because subsequent routines check if dest address is set */
	
	sendbuf = (RSPkt *)Calloc(sizeof(RSPkt)+n, 1);  	/* Pkt Header + Cmd */
	line = PktDatap(sendbuf);

	/* Fill the Header in the packet */
	PktTyp(sendbuf) |= CLI_CMDS;
	DataLen(sendbuf) = n; 				/* All the data that was read */
	
	/* Copy the data to the buffer */
	memcpy(sendbuf + 1, cliline, n);    /* Skip the header on the data */

	/* Do some preprocessing on the received data */
	cmdseq = ntohl(((Cmd *)PktDatap(sendbuf))->cmdseq);
	if (cmdseq <= rs_cur_ind) {
		NETERROR(MRSD, ("Received cli command seq num = %ld, cur ind = %d\n",
			cmdseq, rs_cur_ind));
	}
	else {
		rs_cur_ind = cmdseq;
		NETDEBUG(MRSD, NETLOG_DEBUG2, ("Received cli cmd from client, cmdseq = %ld, cur ind = %d\n",
		cmdseq, rs_cur_ind));
	}
	msgp->Pktp = sendbuf;

	return(msgp);
}

Msg*
CreatePktQry(long seqno)
{
	Cmd		*cmdp;
	Msg		*msgp;
	RSPkt	*sendbuf;

	msgp = Calloc(sizeof(Msg), 1);
	/* Zero out the pkt because subsequent routines check if dest address is set */
	
	sendbuf = (RSPkt *)Calloc(sizeof(RSPkt)+sizeof(Cmd), 1);
	cmdp = (Cmd *)PktDatap(sendbuf);

	/* Add a Sequnce Number to the line */
	cmdp->cmdtyp = htonl(CMD_SNO);
	cmdp->cmdseq = htonl(seqno);
	cmdp->cmdlen = htonl(sizeof(Cmd));

	/* Fill the Header in the packet */
	PktTyp(sendbuf) = (PKT_FROM_SEC | CLI_CMD_SEND);
	DataLen(sendbuf) = cmdp->cmdlen; 		/* Sequence Number in binary */

	/* Do some preprocessing on the received data */
	msgp->Pktp = sendbuf;

	return(msgp);
}

int
SendPktQry(RSConfp rscp, long seqno, struct sockaddr *sa, socklen_t salen)
{
	Msg*msgp;
	int  rval;

	msgp = CreatePktQry(seqno);

	msgp->palen = salen;
	msgp->pa = Malloc(msgp->palen);
	bcopy(sa,msgp->pa, msgp->palen);
	rval = SendNetPkt(rscp, msgp);
	FreeMsg(msgp);

	return(rval);
}

void
FreeMsg(Msg*msgp)
{
	Free(msgp->pa);
	Free(msgp->Pktp);
	Free(msgp);
	return;
}

int
ProcessDBSync(int sockfd)
{
	char	line[RS_LINELEN], sl[RS_MSGLEN], tmpstr[256];
	ssize_t	n;
	char	req[RS_LINELEN], tmpdir[RS_LINELEN], rem_ver[RS_LINELEN];
	int		len = 0;
	long	seqnum;
	int		rval = -1, val;
	int	incompat;
	CopyCli	*ccp;
	char fn[] = "ProcessDBSync";
	int msgqid, selfid;
	char msg[RS_MSGLEN];

	// get the selfid and the msgqid
	 selfid = (((getpid() & 0xffff) << 16) | (pthread_self() & 0xffff));
    
    if ((msgqid = open_execd()) < 0) 
	{
		NETERROR(MRSD, ("Error connecting to execd: %s\n", strerror(errno)));
        return -1;
    }

	NETDEBUG(MRSD, NETLOG_DEBUG4, ("Entering %s\n", fn));

	/* Push the cleanup handler */
	ccp = (CopyCli *)Malloc(sizeof(CopyCli));
	ccp->fd = sockfd;
	ccp->tid = pthread_self();
	pthread_cleanup_push(copy_cli_end, (void *)ccp);
	
	for ( ; ; ) {
		if ((n = Readline(sockfd, line, RS_LINELEN)) <= 0) 
			break;		/* Connection closed */

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		sscanf(line, "%s", req);

		if (strcmp(req, RS_NEED_DB_REQ) == 0) {
			sscanf(line, "%s %s %s", req, rem_ver, tmpdir);
			incompat = strcmp(rem_ver, host_ver) ? 1 : 0; 
			NETDEBUG(MRSD, NETLOG_DEBUG3,
				("host version = %s, remote version = %s are %s\n",
				host_ver, rem_ver, (incompat ? "different" : "same")));
			CreateDir(tmpdir);
			if ((seqnum = SaveDBinDir(tmpdir, incompat)) > 0) {  /* succesfully saved db directory */
				len += snprintf(sl + len, RS_LINELEN - len, incompat ? RS_NEED_DB_SUMI : RS_NEED_DB_SUCC);
				Pthread_mutex_lock(&(host_dbrev.mutex));
				len += snprintf(sl + len, RS_LINELEN - len, " %d", host_dbrev.value);
				Pthread_mutex_unlock(&(host_dbrev.mutex));
				len += snprintf(sl + len, RS_LINELEN - len, " %ld\n", seqnum);
				Writen(sockfd, sl, len); 		/* Don't send the terminating null char */
				rval = 0;
			}
			else {
				len += snprintf(sl + len, RS_LINELEN - len, RS_NEED_DB_FAIL);
				len += snprintf(sl + len, RS_LINELEN - len, "\n");
				Writen(sockfd, sl, len);	/* Don't send the terminating null char */
				rval = -1;
			}
			NETDEBUG(MRSD, NETLOG_DEBUG4, ("sent - %s\n", sl));
		}
		else if (strcmp (req, CLI_CMD_FRM_SLAVE) == 0) 
		{
			// this is a command entered on the slave.
			// process as a regular cli command on the master
			// read the next bytes
			if ((n = Readline(sockfd, line, RS_LINELEN)) <= 0) 
			{
				NETERROR(MRSD,("Readline failed.\n"));
				val = rval = -1;
				len = snprintf(sl, RS_MSGLEN, "%d\n", rval);
				Writen(sockfd, sl, len);
				break;			/* Connection closed */
			}

			strcpy (tmpstr, CLI_STR);
			strcat (tmpstr, line);
			if((val = sys_execd(msgqid, selfid, 1,(1<<REQ_BIT)|(1<<OUT_BIT),tmpstr, msg, RS_MSGLEN)) < 0)
			{
				//NETERROR(MRSD,("No bytes received."));
				val = -1;
			}

			len = snprintf(sl, RS_MSGLEN, "%d\n%s\n", val, msg);
			len++; 			/* for the trailing NUL character */
			NETINFOMSG(MRSD, ("len = %d\n", len));
			Writen(sockfd, sl, len);
			rval = 0;
		}
		else
		{
			NETERROR(MRSD, ("Unknown request to copy server - %s\n", req));
			NETDEBUG(MRSD, NETLOG_DEBUG4, ("line - %s", line));
			rval = -1;
		}

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			
	}

#if 1
	/* Pop the cleanup handler and execute it*/
	pthread_cleanup_pop(1);
	
#endif
	return(rval);
}

void
copy_cli_end(void *arg)
{
	int			i;
	pthread_t	tid;
	CopyCli		*ccp = (CopyCli *)arg;
	RSConfp		rsp;

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("Ending thread %d  - copy_cli_end called\n", 
		pthread_self()));

	close(ccp->fd);
	tid = ccp->tid;
	Free(ccp);
	rsp = the_rscp;

	for (i = THREAD_MIN; i <= THREAD_MAX; i++) {
		if (rsp->tids[i] == tid) {
			Pthread_mutex_lock(&(rsp->tid_mutex));
			rsp->tids[i] = NULL_TID;	
			Pthread_mutex_unlock(&(rsp->tid_mutex));
			break;
		}
	}

	if (i > THREAD_MAX) {
		NETERROR(MRSD, ("ProcessDBSync - Self tid %d not found in thread list\n",
			tid));
	}
}

int
GetDB(struct sockaddr *sap, socklen_t salen)
{
	char 	sendline[RS_LINELEN], recvline[RS_LINELEN];
	int		len = 0, n;
	int		sockfd;
	struct  sockaddr_in paddr;
	char 	dbcpcmd[MAXLEN];
	char	dirname[RS_LINELEN], stat[RS_LINELEN]; 
	long	dbrev, seqnum;
	char 	fn[] = "GetDB";
	int 	incompat;

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("GetDB called\n"));
	nx_strlcpy(dirname, rs_tmp_dir, RS_LINELEN); 		/* TMP_DIR name should end in '/' */
	nx_strlcat(dirname, "db-", RS_LINELEN);
	nx_strlcat(dirname, rs_hostname, RS_LINELEN);
	nx_strlcat(dirname, "/", RS_LINELEN); 			     /* Rsync has significance for last '/' */

	bzero(&paddr, sizeof(paddr));
	bcopy(sap, &paddr, salen);
	
	if ((sockfd = RSConnect(AF_INET, SOCK_STREAM, 0, (struct sockaddr *)&paddr, salen, 5, 1500))
		< 0) {
		NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: can not retrieve database\n", fn));
		return(RS_ERR_ABORT);
	}

	len += snprintf(sendline + len, RS_LINELEN - len, RS_NEED_DB_REQ);
	len += snprintf(sendline + len, RS_LINELEN - len, " ");
	len += snprintf(sendline + len, RS_LINELEN - len, host_ver);
	len += snprintf(sendline + len, RS_LINELEN - len, " ");
	len += snprintf(sendline + len, RS_LINELEN - len, dirname);
	len += snprintf(sendline + len, RS_LINELEN - len, " \n");    /* receiver expects newline */

	len += 1; 		/* Remember to save the terminating Null character */

	Writen(sockfd, sendline, len);
	NETDEBUG(MRSD, NETLOG_DEBUG4, ("Sending Request for DB Sync - %s", sendline));

	n = Readline(sockfd, recvline, RS_LINELEN);   /* Block on Read */

	sscanf(recvline, "%s %ld %ld", stat, &dbrev, &seqnum);
	NETDEBUG(MRSD, NETLOG_DEBUG4, ("Received response - %s", recvline));

	if (n > 0) {	/* We got a response back */
		incompat = -2;
		incompat = strcmp(stat, RS_NEED_DB_SUCC) ? incompat : 0;
		incompat = strcmp(stat, RS_NEED_DB_SUMI) ? incompat : 1;
		incompat = strcmp(stat, RS_NEED_DB_FAIL) ? incompat : -1;
		if (incompat < 0) {
			if (incompat < -1) {	
				NETERROR(MRSD, ("recvd invalid response for DB sync request - %s\n", recvline));
			}
			else {
				NETERROR(MRSD, ("Remote Site failed to save the database. Can't continue\n"));
			}
			return(RS_ERR_ABORT);
		}
		CreateDBCPCmd(dirname, dbcpcmd, sizeof(dbcpcmd));
		if (ProcessCPCmd((struct sockaddr *)&paddr, (Cmd *)dbcpcmd) != 0) {
			NETERROR(MRSD, ("remote cp (rsync) failed for DB sync - %s\n", recvline));
			return(RS_ERR_ABORT);
		}

		/* The file got copied locally */
		ImportDB(rs_tmp_dir, incompat);

		if (seqnum >= 0) {
			rs_cur_ind = seqnum;
		}
		if ((dbrev >= 0) && (dbrev != host_dbrev.value)) {
			Pthread_mutex_lock(&(host_dbrev.mutex));
			host_dbrev.value = dbrev;
			Pthread_mutex_unlock(&(host_dbrev.mutex));

			/* Modify our database */
			ModDBRevNum(dbrev);
		}

		NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: cur index = %d, db rev = %d\n",
			fn, rs_cur_ind, host_dbrev.value));
	}
	else {
		NETERROR(MRSD, ("GetDB: Remote TCP socket closed connection prematurely\n")); 
		Close(sockfd);
		return(RS_ERR_ABORT);
	}

	Close(sockfd);
	return(0);
}

void
CreateDBCPCmd(const char *dirname, char * const buf, int buflen)
{
	Cmd *cmdp;
	int len = sizeof(Cmd);

	len += snprintf(buf+len, buflen-len, dirname);
	len += 1;

	cmdp = (Cmd *)buf;
	cmdp->cmdtyp = CMD_FCP;
	cmdp->cmdlen = len;
	cmdp->cmdseq = 1; 		/* Some Random Number. Make sure that this does not get checkd */
	cmdp->cmdrval = 0; 		/* Return value. Make sure this is right value */
	cmdp->cmdact = 0; 		/* Action to perform. Make sure this is right value */

	return;
}

/*
 * Receives a command with command header. 
 * Checks whether the sequence number is one more than the current.
 * Checks the type of command (e.g. cli command or file copy).
 * Executes the appropriate command. 
 * If command is succesful returns 0. 
 * If error occurs while the cmd is executed, returns 
 * negative integers. 
 */
int
ProcessCmdWHist(Msg *msgp, Cmd*cmdp)
{
	long	diff;
	int		status;

	if (CHECK_CLI_ORDER) {
		/* Check whether the cmd is in sequence */
		diff = cmdp->cmdseq - rs_cur_ind;
	
		if(diff <= 0)
			return(0);  /* Cmd already executed */
	
		if((diff > 1) && (diff < CLI_MAX_HIST)) {
		/* need to send a request to get cli cmds */
			return(RS_ERR_SEQ_NUM);
		}
	
		if(diff >= CLI_MAX_HIST) {
		/* Need to retrieve the whole database */
			return(RS_ERR_NEED_SYNC);
		}
	}

	if (cmdp->cmdtyp == CMD_CLI) {
		status = ProcessCliCmd(cmdp);
		if (status == RS_ERR_CMD_FAIL) {	/* Need to retrieve the whole database */
			NETDEBUG(MRSD, NETLOG_DEBUG4, ("cli command failed. Retrieving database\n"));
			return(RS_ERR_NEED_SYNC);
		}
	}
	else if (cmdp->cmdtyp == CMD_FCP) {
		status = ProcessCPCmd(msgp->pa, cmdp);
		if (status == RS_ERR_CMD_FAIL) { 	/* Need to retrieve the whole database */
			NETDEBUG(MRSD, NETLOG_DEBUG4, ("copy command failed. Retrieving database\n"));
			return(RS_ERR_NEED_SYNC);
		}
	}
	else
		status = RS_ERR_UNKNOWN_CMD;

	return(status);
}

int
ProcessCmdWOHist(Msg *msgp, Cmd*cmdp)
{
	int		rval;
	char 	cmdstr[MAXLEN];
	int 	len = 0;
	char 	fn[] = "ProcessCmdWOHist";

	len += snprintf(cmdstr + len, MAXLEN - len, rs_reg_cli_cmd_str);
	len += snprintf(cmdstr + len, MAXLEN - len, CmdStr(cmdp));
	if (len > MAXLEN)
		NETERROR(MRSD, ("%s: Insuff. space for cmd in Array. Increase MAXLEN = %d\n", 
			fn, MAXLEN));

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("%s: executing %ld - %s\n", fn, 
		CmdSeq(cmdp), cmdstr));

	rval = System(cmdstr);	
	
	if (rval == cmdp->cmdrval)
		rval = 0;
	else {
		/* We don't really care about whether the command succeeded or not */
		NETDEBUG(MRSD, NETLOG_DEBUG1, ("cmd returned an error - %d\n", rval));
		rval = RS_ERR_CMD_FAIL;
	}
			
	return(rval);
}

/*
 * Executes the given cmd and returns the execution status
 */
int
ProcessCliCmd(Cmd *cmdp)
{
	int		rval;
	char 	cmdstr[MAXLEN];
	int 	len = 0;

	len += snprintf(cmdstr + len, MAXLEN - len, rs_cli_cmd_str);
	len += snprintf(cmdstr + len, MAXLEN - len, CmdStr(cmdp));
	if (len > MAXLEN)
		NETERROR(MRSD, ("ProcessCliCmd: Insuff. space for cmd in Array. Increase MAXLEN = %d\n", MAXLEN));

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("ProcessCliCmd: executing %ld - %s\n", CmdSeq(cmdp), cmdstr));
	rval = System(cmdstr);	
	
	if (rval == cmdp->cmdrval)
		rval = 0;
	else {
		NETDEBUG(MRSD, NETLOG_DEBUG1, ("cmd returned an error - %d\n", rval));
		rval = RS_ERR_CMD_FAIL;
	}

	if (rval == 0)						/* Command Successful */
		rs_cur_ind++;

	return(rval);
}

int
ProcessCPCmd(struct sockaddr *sa, Cmd *cmdp)
{
	int		rval;
	char 	cmdstr[MAXLEN];
	int 	len = 0;
	char 	ipaddrstr[INET_ADDRSTRLEN];
	char 	*dest;

	if (inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), ipaddrstr, 
													sizeof(ipaddrstr)) == NULL) {
		NETERROR(MRSD, ("Invalid ip address - %s\n", ipaddrstr));
		return(RS_ERR_ABORT);
	}

	dest = basename(CmdStr(cmdp));
	if (strlen(dest) == 0)
		dest = ".";

	len += snprintf(cmdstr + len, MAXLEN - len, rs_cp_cmd_str);
	len += snprintf(cmdstr + len, MAXLEN - len, ipaddrstr);
	len += snprintf(cmdstr + len, MAXLEN - len, "::nextone");
	len += snprintf(cmdstr + len, MAXLEN - len, CmdStr(cmdp));
	len += snprintf(cmdstr + len, MAXLEN - len, " ");
	len += snprintf(cmdstr + len, MAXLEN - len, rs_tmp_dir);     /* Dir name should end in '/' */
	len += snprintf(cmdstr + len, MAXLEN - len, dest);
	if (len > MAXLEN)
		NETERROR(MRSD, ("ProcessCPCmd: Insuff. space for cmd in Array. Increase MAXLEN = %d\n", MAXLEN));
	
	NETDEBUG(MRSD, NETLOG_DEBUG2, ("ProcessCPCmd: executing %ld - %s\n", CmdSeq(cmdp), cmdstr));
	rval = System(cmdstr);

	if (rval == cmdp->cmdrval)
		rval = 0;
	else 
		rval = RS_ERR_CMD_FAIL;
			
	if (rval == 0)						/* Command Successful */
		rs_cur_ind++;

	return(rval);
}

void
CreateDir(char *dirname)
{
	char  dir[RS_LINELEN],*ptrptr;
	char  *end;
	if (strlen(dirname) == 0) 
		return; 					/* nothing to do */

    strncpy(dir, dirname, RS_LINELEN);
    dirname = dir;

    while((end = strtok_r(dirname, "/", &ptrptr)) && dirname) {
		dirname = ptrptr;
        *(end-1) = '/';
        if ((mkdir(dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0) && (errno != EEXIST)) {
			NETERROR(MRSD, ("CreateDir: creating %s error - %s\n", dir, strerror(errno)));
		}
    }

	return;
}

int
CreateChildAndDo(char *cmdstr)
{
	pid_t	cmdpid;
	int		status = 0;

	if ((cmdpid = Fork()) == 0) {   /* Child */
		NETDEBUG(MRSD, NETLOG_DEBUG2, ("executing cmd - %s\n", cmdstr));
	//	status = execl(cmdstr);
		exit(status);
	}
	else if (cmdpid == -1)  		/* Fork Failed */
		return(RS_ERR_INTRPT);
	else {							/* Parent */
		Waitpid(cmdpid, &status, WUNTRACED);
		return(status);
	}

	return(status);
}

char *
basename(char *path)
{
	char *fname; 

	if ((fname = strrchr(path, '/')) == NULL)  /* No '/' found */
		fname = path;	
	else {
		if (*(fname +1) == '/')  			/* '/' is last char */
			fname = strrchr(fname, '/');	/* look for previous '/' */	
		fname += 1;							/* skip the character */
	}

	return(fname);
}

long
SaveDBinDir(const char *dirname, int incompat)
{
	char	clicmd[RS_LINELEN];
	char	dbcpcmd1[RS_LINELEN], dbcpcmd2[RS_LINELEN];
	int		fd, len = 0;
	long	seqnum = -1;
	char	fn[] = "SaveDBinDir";

	if (incompat) {
		/* Create .xml file */
		len += snprintf(clicmd + len, RS_LINELEN - len, rs_reg_cli_cmd_str);
		len += snprintf(clicmd + len, RS_LINELEN - len, RS_DB_EXPORT_STR);
		len += snprintf(clicmd + len, RS_LINELEN - len, dirname);
		len += snprintf(clicmd + len, RS_LINELEN - len, RS_TMP_XML_FNAME);
	}
	else {
		/* Create .gdbm file */
		len += snprintf(clicmd + len, RS_LINELEN - len, rs_reg_cli_cmd_str);
		len += snprintf(clicmd + len, RS_LINELEN - len, RS_DB_SAVE_STR);
		len += snprintf(clicmd + len, RS_LINELEN - len, dirname);
	}

	snprintf(dbcpcmd1, RS_LINELEN, "cp %s %s", CLI_HIST_FILE, dirname);
	snprintf(dbcpcmd2, RS_LINELEN, "cp %s %s", CLI_SEQNUM_FILE, dirname);

	NETDEBUG(MRSD, NETLOG_DEBUG3, ("Executing - %s\n", clicmd));

	fd = LockSeqNum();	
	
	if (System(clicmd) == 0) {
		seqnum = GetSeqNum(fd, SEQ_RD);
		HDBGetLock(HDB_hist);	
		if (System(dbcpcmd1) != 0) {
			NETERROR(MRSD, ("%s: failed to save %s file\n", fn, 
				CLI_HIST_FILE));
		}
		if (System(dbcpcmd2) != 0) {
			NETERROR(MRSD, ("%s: failed to save %s file\n", fn, 
				CLI_SEQNUM_FILE));
		}
		HDBRelLock(HDB_hist);	
	}
	else {
		NETERROR(MRSD, ("%s: failed to save the database in %s\n", fn,
			dirname));
	}
	UnlockSeqNum(fd);

	return(seqnum);
}

int
ImportDB(const char *dirname, int incmp)
{
	char	clicmd[RS_LINELEN], clicmd1[RS_LINELEN];
	int		len = 0, rval = 0;
	int		fd;
	char	dbcpcmd1[RS_LINELEN], dbcpcmd2[RS_LINELEN];
	char	fn[] = "ImportDB";

	if (incmp) {
		/* We got an xml file */
		len += snprintf(clicmd + len, RS_LINELEN - len, rs_reg_cli_cmd_str);
		len += snprintf(clicmd + len, RS_LINELEN - len, RS_DB_CREATE_STR);
		len += snprintf(clicmd + len, RS_LINELEN - len, dirname);
		len += snprintf(clicmd + len, RS_LINELEN - len, RS_TMP_XML_FNAME);

		len = 0;
		len += snprintf(clicmd1 + len, RS_LINELEN - len, rs_reg_cli_cmd_str);
		len += snprintf(clicmd1 + len, RS_LINELEN - len, RS_DB_COPY_STR);
		len += snprintf(clicmd1 + len, RS_LINELEN - len, dirname);
		len += snprintf(clicmd1 + len, RS_LINELEN - len, RS_TMP_XML_FNAME);
	}
	else {
		/* We got a gdb file */
		len += snprintf(clicmd + len, RS_LINELEN - len, rs_reg_cli_cmd_str);
		len += snprintf(clicmd + len, RS_LINELEN - len, RS_DB_COPY_STR);
		len += snprintf(clicmd + len, RS_LINELEN - len, dirname);
		*clicmd1='\0';
	}

	snprintf(dbcpcmd1, RS_LINELEN, "/bin/cp %s/%s %s", dirname, 
		CLI_HIST_FNAME, CLI_HIST_FILE);
	snprintf(dbcpcmd2, RS_LINELEN, "/bin/cp %s/%s %s", dirname, 
		CLI_SEQNUM_FNAME, CLI_SEQNUM_FILE);

	NETDEBUG(MRSD, NETLOG_DEBUG3, ("Executing - %s\n", clicmd));

	fd = LockSeqNum();	
	if (System(clicmd) == 0) {
		if (strlen(clicmd1)) {
			NETDEBUG(MRSD, NETLOG_DEBUG3, ("Executing - %s\n", clicmd1));
		}
		if (!(strlen(clicmd1)) || (System(clicmd1) == 0)) {
			HDBGetLock(HDB_hist);	
			if (System(dbcpcmd1) != 0) {
				NETERROR(MRSD, ("%s: failed to save %s file\n", fn, 
					CLI_HIST_FILE));
			}
			if (System(dbcpcmd2) != 0) {
				NETERROR(MRSD, ("%s: failed to save %s file\n", fn, 
					CLI_SEQNUM_FILE));
			}
			HDBRelLock(HDB_hist);	
		}
		else {
			NETERROR(MRSD, ("%s: Error executing cmd - %s\n", fn, clicmd1));
			rval = -1;
		}
	}
	else {
		NETERROR(MRSD, ("%s: Error executing cmd - %s\n", fn, clicmd));
		rval = -1;
	}
	UnlockSeqNum(fd);

	return(rval);
}

char *
ServTyp(int role)
{
	static char role_str[80];

	if (role == REPL_SERV_PRI)
		snprintf(role_str, 80, "Master");
	else if (role == REPL_SERV_SEC)
		snprintf(role_str, 80, "Slave");
	else 
		snprintf(role_str, 80, "Unknown (type = %d)", role);

	return(role_str);
}

char *
PacketTyp(int pt)
{
	static char pkt_str[80];
	int len = 80;

	int pkttype = (pt & 0xf0);
	int clitype = (pt & 0x0f);

	if (pkttype == PKT_FROM_PRI)
		len = snprintf(pkt_str, 80, "pkt-from-master ");
	else if (pkttype == PKT_FROM_SEC)
		len = snprintf(pkt_str, 80, "pkt-from-slave ");
	else if (pkttype == PKT_REG)
		len = snprintf(pkt_str, 80, "reg-pkt ");
	else
		len = snprintf(pkt_str, 80, "Unknown-pkt (type = %d) ", pkttype);
		

	if (clitype == CLI_CMDS)
		len = snprintf(pkt_str+len, 80, "contains cli cmds");
	else if (clitype== CLI_CMD_SEND)
		len = snprintf(pkt_str+len, 80, "requests for cli cmd");
	else if (clitype == CLI_FILE_CP)
		len = snprintf(pkt_str+len, 80, "contains file copy cmd");
	else if (clitype == CLI_SNO_BCAST)
		len = snprintf(pkt_str+len, 80, "contains cli seqnum bcast");
	else
		len = snprintf(pkt_str+len, 80, "Unknown-cmd (type = %d) ", clitype);

	return(pkt_str);
}
