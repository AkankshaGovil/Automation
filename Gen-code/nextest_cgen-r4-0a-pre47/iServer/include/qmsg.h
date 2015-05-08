#ifndef _qmsg_h_
#define _qmsg_h_

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "ipcutils.h"
#include "srvrlog.h"
#include "qmsg.h"
#include "ipckey.h"
#include "mem.h"

#include "lock.h"

typedef unsigned long	QMsgType;

/*
 * QMSG interface related definitions 
 */
// Uniquely identify a Q. Q is defined logically as a stream sitting
// on top of SYSV queues. The following pair uniquely identifies
// a Q participant

typedef struct
{
	int 			pid;	// participant on the queue
	int 			qid;	// system qid

} QDesc;

/*
 * Common Message header for all messages defined 
 * exchanged using Q Interface APIs,
 * Messages being sent and received on the Q must have 
 * this structure as a preamble. The structure MUST be filled in
 * with the receiver's QDesc, message type it is sending, any
 * application flags (like Q_RESPONSE_WAIT) etc. The length
 * field should also be filled in
 * Application can use QMsgHeader in it's structures. The API
 * will refer to it (typecast) as QMsg
 */
typedef struct
{
	QDesc		destdesc;	// Destination
	QDesc		srcdesc;	// Src

	QMsgType	mtype;		// message type at application level
	int 		len;		/* length of whole message including this header */

} QMsg, QMsgHdr;

typedef int  ( *QDispatchFn)(void *cbdata, QMsgType mtype, QMsg *msg, size_t msglen);

typedef struct dispatchentry
{
	QDispatchFn	fn; 
	void 		*data;
} QDispatchEntry;

// Dispatch table API
typedef struct
{
	QDispatchEntry	*dispatchTbl;

	int nentries;
} QDispatchTable;

// Dispatch table manipulation
int
QCreateDispatchTable(QDispatchTable *q, int maxentries);

int
QRegisterMsgType(QDispatchTable *q, QMsgType mtype, void *fn, void *data);

int
QDeRegisterMsgType(QDispatchTable *q, QMsgType mtype);

int
QDestroyDispatchTable(QDispatchTable *q);

// Q API
// Get a QDesc from the pair, do not create if not found
int
QGet(QDesc *qdesc, int qmux, int msgq);

// Create (if does not exist) a QDesc from the pair
int
QCreate(QDesc *qdesc, int qmux, int msgq, unsigned int flags, size_t nmsgs, size_t nmsgbytes);

// Destroy a queue - maybe we dont want to do it in the MSW code yet
// But exists because of a complete API
int
QDestroy(QDesc *qdesc);

// Send to a Q participant
int
QSendto(QDesc *dqdesc, QDesc *sqdesc, QMsg *msg, QMsgType mtype, int len, unsigned int flags);

// Receive from a Q participant and call the relevant function in
// the callback API using the dispatch table
// After that return
int
QRcvfrom(QDesc *qdesc, QMsg *msg, size_t len, unsigned int flags, QDispatchTable *tbl);

int
QRcvfrom2(QDesc *qdesc, QMsg *msg, size_t len, unsigned int flags, 
	int (*fncb)());

#define QGetSrc(qmsg) (&qmsg->srcdesc)
#define QGetDest(qmsg) (&qmsg->destdesc)

#define QSetSrc(qmsg, qdescp) (qmsg->srcdesc = *qdescp)
#define QSetDest(qmsg, qdescp) (qmsg->destdesc = *qdescp)

#endif /*_qmsg_h_ */
