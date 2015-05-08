/*
 * Q Interface APIs
 *
 * On the server side sequence of apis
 *    - QSrvInit
 *    - QSrvRegister for message types
 *    - QSrvReader
 *    Reader will call registered function that can call
 *    the functions of dispatch the work to worker threads. In either
 *    case QSrvSendResp should be called.
 *
 * On the client side sequence of apis
 *		- QCliSendReq	
 */
#include "qmsg.h"

int
QCreateDispatchTable(QDispatchTable *qdt, int maxentries)
{
	qdt->dispatchTbl = (QDispatchEntry *)
		malloc (sizeof(QDispatchEntry)*maxentries);

	if (qdt == NULL)
	{
		return -1;
	}

	memset(qdt->dispatchTbl, 0, sizeof(QDispatchEntry)*maxentries);
	qdt->nentries = maxentries;

	return 0;
}

/*
 * Register for a message type defined in the context of queue
 * interface q. Function fn is called with data and incoming msg
 * as params
 * Assumption is that message values are #def'd begining with 0
 */
int
QRegisterMsgType(QDispatchTable *q, QMsgType mtype, void *fn, void *data)
{
	QDispatchEntry  *l = &q->dispatchTbl[mtype];

	if (l->fn == NULL )
	{
		l->fn = fn;
		l->data = data;
		return 0;
	}
	
	return -1;
}

/*
 * Deregister for message mtype
 * Assumption is that message values are #def'd begining with 0
 */

int
QDeRegisterMsgType(QDispatchTable *q, QMsgType mtype)
{
	QDispatchEntry  *l = &q->dispatchTbl[mtype];
	
	memset(l, 0, sizeof(QDispatchEntry));

	return 0;
}

int
QDestroyDispatchTable(QDispatchTable *q)
{
	free (q->dispatchTbl);
	
	return 0;
}

/*
 * Gets id of an existing queue to a given server type
 */
int
QGet(QDesc *qdesc, int qmux, int msgq)
{
	int msgqid; 
	key_t  key;

	if ((key = ftok(ISERVER_FTOK_PATH, qmux)) < 0) 
	{
		return -1;
	}

	if (q_vopen(key,0, &msgqid) < 0) 
	{
		return -1;
	}

	qdesc->qid = msgqid;
	qdesc->pid = msgq;

	return 0; 
}

int
QCreate(QDesc *qdesc, int qmux, int msgq, unsigned int flags, size_t nmsgs, size_t nmsgbytes)
{
	int msgqid; 
	key_t	key;

	if ((key = ftok(ISERVER_FTOK_PATH, qmux)) < 0)
	{
		NETERROR(MINIT, ("ftok: %s\n", strerror(errno)));
		return -1;
	}

	if (q_vget(key, flags, nmsgs, nmsgbytes, &msgqid) < 0)
	{
		NETERROR(MINIT, ("q_vget: %s\n", strerror(errno)));
		return -1;
	}
		
    qdesc->qid = msgqid;
	qdesc->pid = msgq;

	return 0;
}

int
QDestroy(QDesc *qdesc)
{
	/*destroy the queue */
	return(0);
}


int
QSendto(QDesc *dqdesc, QDesc *sqdesc, QMsg *msg, QMsgType mtype, int len, unsigned int flags)
{
	memcpy(&msg->destdesc, dqdesc, sizeof(QDesc));
	memcpy(&msg->srcdesc, sqdesc, sizeof(QDesc));

	msg->mtype = mtype;
	msg->len = len;

	if (q_vsend(dqdesc->qid, msg, len, flags) < 0) 
	{
		NETERROR(MINIT, ("q_vsend: %s\n", strerror(errno)));
		return -1;
	}

	return 0;
}

int
QRcvfrom(QDesc *qdesc, QMsg *msg, size_t len, unsigned int flags, 
	QDispatchTable *tbl)
{
	QDispatchEntry 		*entry=NULL;
	ssize_t rcvlen;

	while(1)
	{
		if ( q_vreceive(qdesc->qid, msg, len, qdesc->pid, flags, &rcvlen) < 0)
		{
			if (errno != EINTR)
			{
				NETERROR(MINIT, ("q_vreceive: %s\n", strerror(errno)));
				return -1;
			} else {
				continue;
			}
		} else {
			break;
		}
	}

	if ((msg->mtype < 0) || (msg->mtype >= tbl->nentries))
	{
		NETERROR(MINIT, ("bad msg type %lu\n", msg->mtype));
		return -1;
	}

	entry = &tbl->dispatchTbl[msg->mtype];
	if (entry->fn == NULL)
	{
		NETERROR(MINIT,("Unregistered msg type[%lu]\n", msg->mtype));
		return -1;
	}
	
	// At this time b4 calling the callback, revese the src and dest
	memswap(&msg->destdesc, &msg->srcdesc, sizeof(QDesc));

	entry->fn(entry->data, msg->mtype, msg, rcvlen);

	return 0;
}

int
QRcvfrom2(QDesc *qdesc, QMsg *msg, size_t len, unsigned int flags, 
	int (*fncb)())
{
	QDispatchEntry 		*entry=NULL;
	ssize_t rcvlen;

	while(1)
	{
		if ( q_vreceive(qdesc->qid, msg, len, qdesc->pid, flags, &rcvlen) < 0)
		{
			if (errno != EINTR)
			{
				NETERROR(MINIT, ("q_vreceive: %s\n", strerror(errno)));
				return -1;
			} else {
				continue;
			}
		} else {
			break;
		}
	}

	if (msg->mtype < 0)
	{
		NETERROR(MINIT, ("bad msg type %lu\n", msg->mtype));
		return -1;
	}

	// At this time b4 calling the callback, revese the src and dest
	memswap(&msg->destdesc, &msg->srcdesc, sizeof(QDesc));

	if (fncb != NULL)
	{
		fncb(entry->data, msg->mtype, msg, rcvlen);
	}

	return 0;
}
