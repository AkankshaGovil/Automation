#include "execd.h"

void
TestComm() 
{
	int msgqid, rc, selfid;
	char msg[MAX_MSGLEN], cmd[MAX_MSGLEN];
	q_msg *m = (q_msg *)msg;

	selfid = (((getpid() & 0xffff) << 16) | (pthread_self() & 0xffff));

	if ((msgqid = open_execd()) < 0) {
		printf("Error connecting to execd: %s\n", strerror(errno));
		exit(1);
	}

	while (gets(cmd)) {
		//send message
		if ((rc = sys_execd(msgqid, selfid, SRVR_MSG_TYP, (1<<REQ_BIT)|(1<<OUT_BIT), cmd, msg, MAX_MSGLEN)) < 0) {
			printf("Error executing command, rc = %d\n", rc);	
			continue;
		}
		printf("rc = %d, output = %s\n", rc, msg);
	}

	exit(0);
}

int
main(int argc, char *argv[])
{
	int msgqid;
	int req_resp = 1;
	ssize_t len;
	int selfid = (((getpid() & 0xffff) << 16) | (pthread_self() & 0xffff));
	char msg[MAX_MSGLEN];
	key_t key;
	q_msg *m = (q_msg *)msg;

	if (argc > 0) {
		TestComm();
	}

	if ((key = ftok(ISERVER_FTOK_PATH, ISERVER_EXECD_Q)) < 0) {
		printf("ftok: %s\n", strerror(errno));
		exit(0);
	}

	if (q_vget(key,0, MAX_NUM_MSG, MAX_MSGLEN, &msgqid) < 0) {
		printf("q_vget: %s\n", strerror(errno));
		exit(1);
	}

	while (gets(&(m->cmd[0]))) {
		m->self_id = SRVR_MSG_TYP;
		m->peer_id = selfid;
		m->flag = 0x1;
		len = -1;
		printf("S> To: %x From: %x flag: %d Cmd: %s\n", 
			m->self_id, m->peer_id, m->flag, &(m->cmd[0]));
		// send message
		if (q_vsend(msgqid, (void *)m, Q_MSG_HDRLEN + strlen(&(m->cmd[0])) + 1, 0)
			< 0) {
			printf("q_vsend: %s\n", strerror(errno));
		}
		// Wait for response
		if (req_resp) {
			if (q_vreceive(msgqid, (void *)m, MAX_MSGLEN, selfid, 0, &len) < 0) {
				printf("q_vreceive: %s\n", strerror(errno));
			}
			else {
				printf("R> To: %x From: %x flag: %d Len: %ld Cmd: %s\n", 
					m->self_id, m->peer_id, m->flag, len, &(m->cmd[0]));
			}
		}
	}
}
