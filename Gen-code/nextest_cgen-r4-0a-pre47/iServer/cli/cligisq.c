#include "qmsg.h"
#if 0
#include "cli.h"

/* Block if the caller indicates that a response is expected 
 * Fill up the common header of message
 * 
 * If Response is request then the function returns length of
 * message received. Received message is copied in msg
 */
int
CliQSendReq(QHandle numqHandle, QMsgId msgid, void * msg, int size)
{
	QDesc  qid; 
	QListener	selfid; 
	int  len=0;
	char rbuf[MAX_QMSG_LEN];

	if ((qid = QGetQid(numqHandle)) < 0)
	{
		NETERROR(MINIT, ("Failed to find qid for server[%d]\n", numqHandle));
		return -1;
	}

	selfid = QFillHdrToServer(msg, msgid, size);

	if(QSendto(qid, (void *)msg, size, 0) < 0) 
	{
		return -1;
	}

	if (QCheckAppFlag(msg, Q_RESPONSE_WAIT))
	{
		if (QRcvfrom(qid, (void *)rbuf, sizeof(rbuf), selfid, 0, &len) < 0)
		{       
			NETERROR(MINIT,("CliQSendReq: Rcv failed for q [%s]\n",numqHandle));
			return -1;
		}
	}
	return len;
}

#if 0
int
CliopenGisQ(void)
{
	int msgqid;
	key_t  key;

	if ((key = ftok(ISERVER_FTOK_PATH, ISERVER_GIS_Q)) < 0) 
	{
		return -1;
	}

	if (q_vopen(key,0, &msgqid) < 0) 
	{
	return -1;
	}

	return msgqid;
}

int
CliSendtoGisQ(GisCliQMsg *msg)
{
	int qid, selfid;
        ssize_t	len;

	if ((qid = CliopenGisQ()) < 0)
	{
		NETERROR(MINIT, ("Error connecting to Gis: %s\n", strerror(errno)));
		return -1;
	}
	selfid = (((getpid() & 0xffff) << 16) | (pthread_self() & 0xffff));
	msg->self_id = GIS_SRVR_MSG_TYPE;
	msg->peer_id = selfid;
	
	if (q_vsend(qid, (void *)msg, sizeof(GisCliQMsg), 0) < 0)
	{
		NETERROR(MINIT, ("q_vsend %s\n", strerror(errno)));
		return -1;
	}

	if (msg->flags & GIS_CLI_REQ_FLAG)
	{
		if (q_vreceive(qid, (void *)msg, sizeof(GisCliQMsg), 
					selfid, 0, &len) < 0)
		{
			NETERROR(MINIT,("q_vsend: %s\n", strerror(errno)));
			return -1;
		}
	}
	return 0;
}
#endif
#endif
