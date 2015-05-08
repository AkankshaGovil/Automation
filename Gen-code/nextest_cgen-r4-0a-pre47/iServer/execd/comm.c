#include "execd.h"

int
open_execd()
{
	int msgqid;
	key_t key;

	if ((key = ftok(ISERVER_FTOK_PATH, ISERVER_EXECD_Q)) < 0) {
		return -1;
	}

	if (q_vopen(key,0, &msgqid) < 0) {
		return -1;
	}

	return msgqid;
}

int
sys_execd(int msgqid, unsigned int selfid, unsigned int peerid, unsigned int flag, 
	char *cmdstr, char *outbuf, int len) 
{
	char msg[MAX_MSGLEN];
	int cmdlen; 
	ssize_t pktlen;
	q_msg	*m = (q_msg *)msg;

	m->self_id = peerid;
	m->peer_id = selfid;
	m->flag = flag;
	
	if (!(cmdstr) || ((cmdlen = strlen(cmdstr)) >= (MAX_CMDLEN)) ) {
		errno = EINVAL;
		return -1;	
	}

	strncpy(&(m->cmd[0]), cmdstr, MAX_CMDLEN);

	pktlen = Q_MSG_HDRLEN + cmdlen + 1;

	NETDEBUG(MEXECD, NETLOG_DEBUG4, ("C> To: %x From: %x flag: %d Len: %zd Cmd: %s\n", 
		m->self_id, m->peer_id, m->flag, pktlen, &(m->cmd[0])));
		
	if (q_vsend(msgqid, (void *)m, pktlen, 0) < 0) {
		NETERROR(MEXECD, ("C> q_vsend: %s\n", strerror(errno)));
		return -1;
	}

	if (flag & (1<<REQ_BIT)) {
		if (q_vreceive(msgqid, (void *)m, MAX_MSGLEN, selfid, 0, &pktlen) < 0) {
			NETERROR(MEXECD, ("C> q_vreceive: %s\n", strerror(errno)));
			return -1;
		}
		else {
			NETDEBUG(MEXECD, NETLOG_DEBUG4, ("C> To: %x From: %x flag: %d Len: %zd Cmd: %s\n", 
				m->self_id, m->peer_id, m->flag, pktlen, &(m->cmd[0])));
			if ((flag & (1<<OUT_BIT)) && outbuf) {
				strncpy(outbuf, &(m->cmd[0]), len);
			}
			return (m->flag); /* It contains the status of the cmd */
		}
	}
	else {
		return 0;	//Success
	}
}
