
/*
 * Copyright (c) 2003 NExtone Communications, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


#define MFCP_PRIVATE_INTERFACE /* make mfcp.h include the private structs */
#define _MFCP_SESS_STATE_

#include "mfcp.h"
#include "mfcpproto.h"
#include "fclocal.h"
#include "nsfglue.h"
#include "fcemacro.h"

/* list macros (depends on clist.h) */

#include "clist.h"

/* walk eash entry in the list (except the list head) and execute the func with the ptr as the arg */
#define ListWalk(list,func) { ListEntry *relem; for(relem = (ListEntry *) ((ListEntry *) (list))->next; relem != (ListEntry *)list; relem = (relem)->next) { (void *)(func)((void *)relem); } }

pthread_t parserThread;          /* thread for the parser/reader */
pthread_t reconnectThread;          /* thread for the reconnectreader */

void mfcp_crs_cb(void *rPtr);

/*****************************************************************************
 *
 *  Static Definitions (lookup tables)
 *
*****************************************************************************/

static struct _MFCP_ParamLookup
{
  MFCP_ParameterTypes type;
  char *pname;
} mfcpLookupParamTable[] = {
  {
  MFCP_PARAM_SESSION_ID, "session_id"}, {
  MFCP_PARAM_BUNDLE_ID, "bundle_id"}, {
  MFCP_PARAM_RESOURCE_ID, "resource_id"}, {
  MFCP_PARAM_REPORT_ERRORS, "report_errors"}, {
  MFCP_PARAM_SHUTDOWN_INTERVAL, "shutdown_interval"}, {
  MFCP_PARAM_ID, "id"}, {
  MFCP_PARAM_SRC, "src"}, {
  MFCP_PARAM_DEST, "dst"}, {
  MFCP_PARAM_PROTOCOL, "protocol"}, {
  MFCP_PARAM_SRC_POOL, "src_pool"}, {
  MFCP_PARAM_DEST_POOL, "dst_pool"}, {
  MFCP_PARAM_TYPE, "type"}, {
  MFCP_PARAM_BANDWIDTH, "bandwidth"}, {
  MFCP_PARAM_DATA, "data"}, {
  MFCP_PARAM_OPTYPE, "optype"}, {
  MFCP_PARAM_PEER_RESOURCE_ID, "peer_resource_id"}, {
  MFCP_PARAM_NAT_SRC, "nat_src"}, {
  MFCP_PARAM_NAT_DEST, "nat_dst"}, {
  MFCP_PARAM_DEST_SYM, "dst_sym"}, {
  MFCP_PARAM_END, NULL}
};

static struct _MFCP_RequestLookup
{
  MFCP_RequestType type;
  char *rname;
} mfcpLookupTable[] = {
  {
  MFCP_REQ_CRS, "CRS"}, {
  MFCP_REQ_MDS, "MDS"}, {
  MFCP_REQ_CRB, "CRB"}, {
  MFCP_REQ_CRR, "CRR"}, {
  MFCP_REQ_MDR, "MDR"}, {
  MFCP_REQ_CHR, "CHR"}, {
  MFCP_REQ_DLR, "DLR"}, {
  MFCP_REQ_DLB, "DLB"}, {
  MFCP_REQ_DLS, "DLS"}, {
  MFCP_REQ_AUS, "AUS"}, {
  MFCP_REQ_HLP, "HLP"}, {
  MFCP_REQ_PNG, "PNG"}, {
  MFCP_REQ_NTF, "NTF"}, {
  MFCP_REQ_RQT, "RQT"}, {
  MFCP_REQ_RSP, "RSP"}, {
  MFCP_REQ_END, NULL}
};


/* mfcp_logit 
   temporary logger 
*/


extern int errno;
FILE *tracefd = NULL;

int mfcpReqPool, mfcpReqClass;
int mfcpResPool, mfcpResClass;

int
mfcp_trace (char *format, ...)
{
  va_list arglist;

  if (tracefd || (tracefd = fopen ("mfcp_trace.txt", "a"))) {
    va_start (arglist, format);
    vfprintf (tracefd, format, arglist);
    va_end (arglist);
  }
  return MFCP_RET_OK;
}

#ifdef MFCP_DEBUG
void *
mydebug_free(void *m,  int lineno) {
  NETDEBUG(MFCE,  NETLOG_DEBUG4, ("%x - %s (%d) free\n", (unsigned int)m, __FILE__, lineno ));
  free(m);
}


void *
mydebug_malloc(size_t n,  int lineno) {
  void *m;
  m = malloc(n);
  NETDEBUG(MFCE,  NETLOG_DEBUG4, ("%x - %s (%d) malloc bytes=%d\n",(unsigned int)m, __FILE__, lineno, n ));
  return m;
}

void *
mydebug_calloc(size_t s,size_t n,  int lineno) {
  void *m;
  m = calloc(s,n);
  NETDEBUG(MFCE,  NETLOG_DEBUG4, ("%x - %s (%d) calloc bytes=%d\n",(unsigned int)m, __FILE__, lineno, n*s ));
  return m;
}


#define free(x) mydebug_free(x,__LINE__)
#define malloc(x) mydebug_malloc(x, __LINE__)
#define calloc(s,x) mydebug_calloc(s,x, __LINE__)

#endif

/*****************************************************************************
 * MFCP API routines
 *****************************************************************************/

int 
mfcp_init(void) {
  // +++ this doesn't need to be done if it's a remote firewall

  mfcpReqPool = ThreadPoolInit("mfcpReqPool", 4, PTHREAD_SCOPE_PROCESS, 0, 0);
  mfcpReqClass = ThreadAddPoolClass("mfcpReqClass", mfcpReqPool, 0, 100000000);
  ThreadPoolStart(mfcpReqPool);

  mfcpResPool = ThreadPoolInit("mfcpResPool", 4, PTHREAD_SCOPE_PROCESS, 0, 0);
  mfcpResClass = ThreadAddPoolClass("mfcpResClass", mfcpResPool, 0, 100000000);
  ThreadPoolStart(mfcpResPool);

  return MFCP_RET_OK;
}

/* mfcp_new_session
   allocate a session structure and initialize it
*/

MFCP_Session *
mfcp_sess_new (void)
{
  MFCP_Session *sess;

  sess = (MFCP_Session *) calloc (1,sizeof (MFCP_Session));
  if (sess) {
    sess->state = MFCP_EOF;
    sess->appType = MFCP_APP_CLIENT;
//  sess->id = 1;
    sess->lastRequestIdSent = 1;
    ListInitElem (&(sess->requestHead));
    pthread_mutex_init(&(sess->sessLock),NULL);
  } else {
    mfcp_logit ("Memory Allocation Failure");
    return (NULL);
  }
  return sess;
}

/* Free a session and all of it's malloced data */
MFCP_Session *
mfcp_sess_free (MFCP_Session * sess)
{
  MFCP_Request *rPtr;
  if (sess) {
    pthread_mutex_destroy(&sess->sessLock);

    // loop through the list and remove the sess pointer but leave the requests
    for(rPtr = sess->requestHead.sentFirst; rPtr != (MFCP_Request *)&sess->requestHead; rPtr = rPtr->prev) {
      rPtr->sess = NULL;
      mfcp_sess_rem_sent_req(sess,rPtr);
    }
    bzero(sess,sizeof(sess));                          /* zero the array to assist the debug */
    free ((void *) sess);
  } else {
    mfcp_logit("Attempt to free a NULL session");
  }
  return NULL;
}

/* mfcp_sess_connect
 * Create a session and make the connection
 * paddr = the IP address of the MFCP server or if 0
 *         then local calls are made to the fclocal/nfs
 * keepAlive = application layer keep alive (not implimented)
 */

MFCP_Session
  * mfcp_sess_connect (unsigned long int paddr, 
		       unsigned int keepAlive,
		       void (*asyncFunc)(void *))
{
  MFCP_Session *sess;
//MFCP_Request *rPtr;
//int len = 0;
//char *fn = "mfcp_sess_connect";
//char *errmsg;
//int one = 1;

  sess = mfcp_sess_new ();
  pthread_mutex_lock (&sess->sessLock);  
  sess->asyncFunc = asyncFunc;

  if (!paddr) {
    /* here if the addr is 0 so we assume a local control */
    sess->sessType = MFCP_SESS_LOCAL;
    sess->state = MFCP_SESS_CONNECTED;
    pthread_mutex_unlock (&sess->sessLock);  
    nsfGlueInit(); /* init the nsf since we're local */
  } else {

    sess->localAddr.sin_family = AF_INET;

    sess->peerAddr.sin_addr.s_addr = htonl (paddr);
    sess->peerAddr.sin_port = htons (MFCP_PORT);
    sess->peerAddr.sin_family = AF_INET;

    sess->state = MFCP_EOF;
    sess->sessType = MFCP_SESS_TCP;

    /* spawn off the parser on a seperate thread */
    pthread_mutex_unlock (&sess->sessLock);  
    mfcp_sess_reconnect(sess); 
    pthread_create(&parserThread,NULL,mfcp_parse,sess);
  }

  return sess;
}

/* reconnect the previous session this thread lives until the connection is made */
void *mfcp_sess_reconnect(void *vsess) {

  //MFCP_Request *rPtr;
  MFCP_Session *sess = (MFCP_Session *)vsess;
  int len;
  int one = 1;
  char *fn = "mfcp_sess_reconnect";
  char* errmsg;
  int fd = -1;;

  if (sess->sessType == MFCP_SESS_TCP) {
    pthread_mutex_lock (&sess->sessLock);  
    if (sess->state == MFCP_EOF) {
      if (sess->fd >= 0) {
      	shutdown (sess->fd, SHUT_RDWR);
      	close(sess->fd);
      	NETDEBUG(MFCE,  NETLOG_DEBUG4, ("MFCP socket close fd:%d\n",sess->fd) );
      	sess->fd = -1;
      }
      pthread_mutex_unlock (&sess->sessLock);  

      if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        NETERROR(MFCE,("Error creating MFCP socket, aborting MFCP"));
//      pthread_mutex_unlock (&sess->sessLock);
        return NULL;
      }

      if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one)) < 0) {
        errmsg = strerror( errno );
        NETERROR(MFCE, ("%s Could not set the SO_KEEPALIVE option %s", fn, errmsg ));
      }

      // connect to the firewall, We stay in this loop until we connect 
      while (connect (fd, (struct sockaddr *) &sess->peerAddr, sizeof (sess->peerAddr)) < 0) {
        NETDEBUG(MFCE,NETLOG_DEBUG4, ("Error socket connect, sleep and try again"));
        sleep(5);
      }     

      pthread_mutex_lock (&sess->sessLock);
      //get the local address and port
      len = sizeof (sess->localAddr);
      bzero (&sess->localAddr, len);
      if (getsockname (fd, (struct sockaddr *) &sess->localAddr, &len) < 0) {
        mfcp_logit ("Error in getsockname, will close socket");
	close(fd);
        pthread_mutex_unlock (&sess->sessLock);  
        return NULL;
      }

      sess->fd = fd;
      sess->state = MFCP_TCP_CONNECTED;
//    sess->sessType = MFCP_SESS_TCP;
      pthread_mutex_unlock (&sess->sessLock);  
    } else {
      pthread_mutex_unlock (&sess->sessLock);  
    }

//  /* spawn off the parser on a seperate thread */
//  /* this means only call this if the previous parser thread is not running */
//  pthread_create(&parserThread,NULL,mfcp_parse,sess);

    if (mfcp_req_crs(sess,2, mfcp_crs_cb, sess) != MFCP_RET_OK) {
      NETERROR(MFCE, ("%s: Error in CRS from the firewall", fn) );
      // Don't know why this failed. 
      pthread_mutex_lock (&sess->sessLock);
      close(sess->fd);
      sess->fd = -1;
      sess->state = MFCP_EOF;
      pthread_mutex_unlock (&sess->sessLock);  
    }
    else {
      pthread_mutex_lock (&sess->sessLock);
      sess->state = MFCP_SESS_CONNECTING;
      pthread_mutex_unlock (&sess->sessLock);  
    }
  } else {
    /* here if the addr is 0 so we assume a local control */
    pthread_mutex_lock (&sess->sessLock);  
    sess->sessType = MFCP_SESS_LOCAL;
    sess->state = MFCP_EOF;
    pthread_mutex_unlock (&sess->sessLock);  
    nsfGlueInit(); /* init the nsf since we're local */
  }
  return NULL;

}

void
mfcp_crs_cb(void *r)
{
  MFCP_Request *rPtr = (MFCP_Request *)r;
  MFCP_Status status;
  MFCP_Session *sess = (MFCP_Session *)mfcp_get_res_appdata(rPtr);

  if ((status = mfcp_get_res_status(rPtr)) == MFCP_RSTATUS_OK) {
    pthread_mutex_lock (&sess->sessLock);  
    sess->id = mfcp_get_int(rPtr,MFCP_PARAM_SESSION_ID);
    sess->state = MFCP_SESS_CONNECTED;
    pthread_mutex_unlock (&sess->sessLock);  
    NETINFOMSG(MFCE, ("Established Session with firewall\n")); 
  }
  else if (status == MFCP_RSTATUS_NOTOK) {
    // What kind of error is this?
    pthread_mutex_lock (&sess->sessLock);  
    if (!strncmp("Error - Session could not be found", mfcp_get_res_estring(rPtr), 128)) {
     sess->id = 0;
    }
    sess->state = MFCP_TCP_CONNECTED;
    pthread_mutex_unlock (&sess->sessLock);  
    NETDEBUG(MFCE,NETLOG_DEBUG4, ("will try session reconnect again with session id %d\n", sess->id));
    mfcp_sess_reconnect(sess);
  }

  mfcp_req_free(rPtr);
}

/* the listen and accept are for testing a server */
int
mfcp_sess_listen (unsigned long int paddr, unsigned int port)
{
  int on = 1;
  int lfd;
  struct sockaddr_in localAddr;
  int len = sizeof (localAddr);

  if ((lfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    mfcp_logit ("Error creating listen Socket");
  } else {


    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
		   (void *)&on, sizeof(on)) < 0) {
      NETERROR(MDB, ("mfcp_sess_listen: setsockopt SO_REUSEADDR - %s\n",
		     strerror(errno)));
    }

    bzero (&localAddr, len);

    localAddr.sin_addr.s_addr = htonl (paddr);
    localAddr.sin_port = htons (port);
    localAddr.sin_family = AF_INET;

    // listen for a connection 
    if (bind (lfd, (struct sockaddr *) &localAddr, sizeof (localAddr)) != -1) {
      listen (lfd, 10);
    } else {
      mfcp_logit (strerror (errno));
      shutdown (lfd, SHUT_RDWR);        // ++ should i use close?
      return MFCP_RET_BINDERR;
    }
  }
  return lfd;
}

MFCP_Session *
mfcp_sess_accept (int lfd)
{
  int len;
  int fd;
  struct sockaddr_in peerAddr;
  MFCP_Session *sess;

  len = sizeof(peerAddr);

  if ((fd = accept (lfd, (struct sockaddr *) &peerAddr, &len)) > 0) {
    sess = mfcp_sess_new ();
    memcpy (&sess->peerAddr, &peerAddr, len);
    sess->fd = fd;

    if (getsockname (sess->fd, (struct sockaddr *) &sess->localAddr, &len) <
        0) {
      mfcp_logit ("Error in getsockname");
      return sess;
    }
    sess->state = MFCP_TCP_CONNECTED;
    sess->sessType = MFCP_SESS_TCP;
    sess->appType = MFCP_APP_SERVER;
  } else {
    mfcp_logit (strerror (errno));
    return NULL;
  }
  return sess;
}

/* get the id and increment it */
int
mfcp_sess_seqid (MFCP_Session * sess)
{
  int seqId;
  pthread_mutex_lock (&sess->sessLock);
  seqId = sess->lastRequestIdSent++;
  pthread_mutex_unlock (&sess->sessLock);
  return seqId;
}

/* Connect up the double linked list of requests into the first
   and last pointers in the sess structure */

int
mfcp_sess_add_sent_req (MFCP_Session * sess, MFCP_Request * rPtr)
{
  int ret = MFCP_RET_NOTOK;

  pthread_mutex_lock (&sess->sessLock);
  if (rPtr->sess->numReq < MFCP_REQ_QLEN) {
    rPtr->sess->numReq++;
    ListInsert (&(sess->requestHead), rPtr);
    ret = MFCP_RET_OK;
  }
  pthread_mutex_unlock (&sess->sessLock);

  return ret;
}

/* remove the request from the sent list */
int
mfcp_sess_rem_sent_req (MFCP_Session * sess, MFCP_Request * rPtr)
{
  if (rPtr->sess) {  // if there is still a session active 
    pthread_mutex_lock (&sess->sessLock);

    rPtr->sess->numReq--;
    ListDelete (rPtr);
    
    pthread_mutex_unlock (&sess->sessLock);
  }
  return MFCP_RET_OK;
}

int
mfcp_sess_send_req (MFCP_Session * sess, MFCP_Request * rPtr)
{
  int seqId, ret;
  char cbuf[4096];

  if (sess == NULL) {
    mfcp_logit("Null sess");
    return (MFCP_RET_NOTOK);
  }

  if (rPtr == NULL) {
    mfcp_logit("Null rPtr");
    return (MFCP_RET_NOTOK);
  }

  mfcp_sess_display(sess,"mfcp_sess_send_req");

  /* we set the session of this packet to the session it's sent on */
  rPtr->sess = sess;
  seqId = mfcp_sess_seqid (sess);
  
  rPtr->seqId = seqId;
  if (gettimeofday(&rPtr->time, NULL) < 0) 
    memset(&rPtr->time, 0, sizeof(struct timeval));

  if (sess->appType == MFCP_APP_CLIENT) {
    if (mfcp_sess_add_sent_req (sess, rPtr) != MFCP_RET_OK) {
	NETERROR(MFCE, ("mfcp_sess_send_req: snd q full\n"));
	return MFCP_RET_SNDQFULL;
    }
  }

  if (sess->sessType == MFCP_SESS_TCP) {
    if ( (sess->state == MFCP_SESS_CONNECTED) || ((sess->state >= MFCP_TCP_CONNECTED) && (rPtr->type == MFCP_REQ_CRS)) ) {
      NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_sess_send_req: send tcp request\n"));
      mfcp_req_display(rPtr,"mfcp_sess_send_req");
      mfcp_req_mkpkt (rPtr, cbuf, sizeof (cbuf), seqId);
      if ((ret = mfcp_send (sess, cbuf, strlen (cbuf), 0)) < 0) {
      	NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_sess_send_req: send error - %s\n", strerror(errno)));
    		if (mfcp_sess_rem_sent_req (sess, rPtr) != MFCP_RET_OK) {
      		NETERROR(MFCE,("mfcp_sess_send_req: couldn't remove request from list.\n"));
				}
      	return MFCP_RET_NOTOK;
      }
    } else {
    	if (mfcp_sess_rem_sent_req (sess, rPtr) != MFCP_RET_OK) {
      	NETERROR(MFCE,("mfcp_sess_send_req: couldn't remove request from list.\n"));
			}
      return MFCP_RET_NOTOK;
    }
  } else {
    NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_sess_send_req: Dispatch request\n"));
    if (ThreadDispatch(mfcpReqPool, 0, (void*(*)(void*)) fc_dorequest, rPtr,
		       1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59)) {
      NETERROR(MFCE, ("Error in dispatching fc_dorequest\n"));
    	if (mfcp_sess_rem_sent_req (sess, rPtr) != MFCP_RET_OK) {
      	NETERROR(MFCE,("mfcp_sess_send_req: couldn't remove request from list.\n"));
			}
      return(MFCP_RET_NODISP);
    }
  }

  return MFCP_RET_OK;
}

int
mfcp_sess_send_res (MFCP_Session * sess, MFCP_Request * rPtr)
{
  int seqId;
  char cbuf[4096];

  if (sess == NULL) {
    mfcp_logit("Null sess");
    return (MFCP_RET_NOTOK);
  }

  if (rPtr == NULL) {
    mfcp_logit("Null rPtr");
    return (MFCP_RET_NOTOK);
  }
  mfcp_sess_display(sess,"mfcp_sess_send_res");
  /* we set the session of this packet to the session it's sent on */
  rPtr->sess = sess;
  if (rPtr->respStatus == 0) {
    rPtr->respStatus = 200;
  }
  seqId = rPtr->seqId;

  if (sess->appType == MFCP_APP_CLIENT) {
    mfcp_sess_rem_sent_req(sess,rPtr);
  }
  
  if (sess->sessType == MFCP_SESS_TCP) {
    if (sess->state >= MFCP_TCP_CONNECTED) {
    NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_sess_send_res: send tcp response\n"));
    mfcp_req_display(rPtr,"mfcp_sess_send_res");
    mfcp_res_mkpkt(rPtr,cbuf,sizeof(cbuf),seqId);
    mfcp_send(sess,cbuf,strlen(cbuf),0);
    } else {
      return MFCP_RET_NOTOK;
    }
  } else {
    NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_sess_send_res: Dispatch response\n"));
    if (ThreadDispatch(mfcpResPool, 0, (void*(*)(void*)) fc_doresponse, rPtr,
		       1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59)) {
      NETERROR(MFCE, ("Error in dispatching fc_doresponse\n"));
      return(MFCP_RET_NODISP);
    }
  }
  return MFCP_RET_OK;
}

int
mfcp_send_failure(MFCP_Session *sess, MFCP_Request *rPtr, int respStatus, char *respString) {
  rPtr->respStatus = respStatus;
  strncpy(rPtr->respString,respString,sizeof(rPtr->respString));
  return (mfcp_sess_send_res(sess,rPtr));
}

int
mfcp_sess_close (MFCP_Session * sess)
{
  if (sess != NULL) {
    if ( sess->sessType == MFCP_SESS_TCP) {
      mfcp_sreq_dls(sess);
      shutdown (sess->fd, SHUT_RDWR);
      close(sess->fd);
    } else {
      nsfGlueShutdown();
    }
    sess = mfcp_sess_free (sess);
  }
  return MFCP_RET_OK;
}

MFCP_Value *
mfcp_value_new_i (int i)
{
  MFCP_Value *value;
  if (mfcp_value_setint (value = mfcp_value_new (MFCP_VALUE_INTEGER), i) == 0) {
    return value;
  }
  return NULL;
}

MFCP_Value *
mfcp_value_new_s (char *s)
{
  MFCP_Value *value;
  if (mfcp_value_setstr (value = mfcp_value_new (MFCP_VALUE_STRING), s) == 0) {
    return value;
  }
  return NULL;
}


MFCP_Value *
mfcp_value_new (MFCP_ValueTypes t)
{
  MFCP_Value *value;
  if ((value = calloc (1,sizeof (MFCP_Value))) == NULL) {
    mfcp_logit ("Malloc error");
  }
  value->next = NULL;
  value->s = value->sbuf;
  value->type = t;
  return value;
}

MFCP_Value *
mfcp_value_free (MFCP_Value * v)
{
  MFCP_Value *p, *nextp;
  p = v;
  while (p) {
    switch (p->type) {
    case MFCP_VALUE_NONE:
      break;
    case MFCP_VALUE_INTEGER:
      break;
    case MFCP_VALUE_STRING:
      if (p->s != p->sbuf) {
        free (p->s);
      }
      break;
    case MFCP_VALUE_END:
    default:
      mfcp_logit ("Unknown value type");
      break;
    }
    nextp = p->next;
    free (p);
    p = nextp;
  }
  return NULL;
}


int
mfcp_value_setint (MFCP_Value * p, int v)
{
  if (p && (p->type == MFCP_VALUE_INTEGER)) {
    p->i = v;
    sprintf (p->s, "%d", v);
    p->type = MFCP_VALUE_INTEGER;
  } else {
    mfcp_logit ("Setint called with NULL or incorrect type");
    return -1;
  }
  return 0;
}

int
mfcp_value_getint (MFCP_Value * p)
{
  if (p && (p->type == MFCP_VALUE_INTEGER)) {
    return p->i;
  }
  return MFCP_RET_BADINT;
}

int
mfcp_value_setstr (MFCP_Value * p, char *c)
{
  int nc;

  if (p && (p->type == MFCP_VALUE_STRING)) {
    if (p->s != p->sbuf) {
      free (p->s);
    }
    nc = strlen (c);
    if (nc >= MFCP_MINSTR_SIZE) {
      p->s = malloc (nc + 1);
    }
    strncpy (p->s, c, nc + 1);
    p->s[nc] = '\0';
  }
  return 0;
}

char *
mfcp_value_getstr (MFCP_Value * p)
{
  if (p && (p->type == MFCP_VALUE_STRING)) {
    return p->s;
  }
  return NULL;
}

MFCP_Value *
mfcp_value_add_list (MFCP_Value * p1, MFCP_Value * p2)
{
  MFCP_Value *tmpptr = p1;

  while (tmpptr->next) {
    tmpptr = tmpptr->next;
  }
  tmpptr->next = p2;
  return p2;
}

char *
mfcp_value_vtoa (MFCP_Value * value, char *cbuf, unsigned int maxlen)
{
  MFCP_Value *pv;
  char *cp = cbuf;
  int len = 0;
  int slen = 0;

  // if a length longer than the maximum string length then max len  
  if (maxlen > MFCP_MAX_STRING) {
    maxlen = MFCP_MAX_STRING;
  }

  pv = value;
  while (pv) {
    switch (pv->type) {
    case MFCP_VALUE_NONE:
      mfcp_logit ("Non initialized value");
      return NULL;
    case MFCP_VALUE_STRING:
      if (pv->s) {
        if ((len + (slen = strlen (pv->s)) + 2) >= maxlen) {
          mfcp_logit ("String length exceeded");
          return NULL;
        }
        *cp++ = '"';
        strcpy (cp, pv->s);
        cp += slen;
        *cp++ = '"';
      }
      break;
    case MFCP_VALUE_INTEGER:
      if (pv->s) {
        if ((len + (slen = strlen (pv->s)) + 2) >= maxlen) {
          mfcp_logit ("String length exceeded");
          return NULL;
        }
        strcpy (cp, pv->s);
        cp += slen;
      }
      break;
    case MFCP_VALUE_END:
    default:
      mfcp_logit ("Unknown value type");
      break;
    }
    pv = pv->next;
    if (pv) {
      len += slen + 1;
      *cp++ = MFCP_LIST_DELIM;
    }
  }
  *cp = '\0';
  return cbuf;
}

/* Parameter Methods */
MFCP_Parameter *
mfcp_param_new (MFCP_ParameterTypes type)
{
  MFCP_Parameter *paramPtr;
  if ((paramPtr = calloc (1,sizeof (MFCP_Parameter))) == NULL) {
    mfcp_logit ("Malloc Error");
    return NULL;
  }
  paramPtr->type = type;
  paramPtr->value = NULL;
  paramPtr->next = NULL;
  paramPtr->prev = NULL;
  return paramPtr;
}

MFCP_Parameter *
mfcp_param_free (MFCP_Parameter * p)
{
  MFCP_Parameter *tmpPtr;

  while (p) {
    mfcp_value_free (p->value);
    tmpPtr = p->next;
    free (p);
    p = tmpPtr;
  }
  return NULL;
}

char *
mfcp_param_str (MFCP_ParameterTypes type)
{
  switch (type) {
  case MFCP_PARAM_SESSION_ID:
    return "session_id";
  case MFCP_PARAM_BUNDLE_ID:
    return "bundle_id";
  case MFCP_PARAM_RESOURCE_ID:
    return "resource_id";
  case MFCP_PARAM_REPORT_ERRORS:
    return "report_errors";
  case MFCP_PARAM_SHUTDOWN_INTERVAL:
    return "shutdown_interval";
  case MFCP_PARAM_ID:
    return "id";
  case MFCP_PARAM_SRC:
    return "src";
  case MFCP_PARAM_DEST:
    return "dst";
  case MFCP_PARAM_PROTOCOL:
    return "protocol";
  case MFCP_PARAM_SRC_POOL:
    return "src_pool";
  case MFCP_PARAM_DEST_POOL:
    return "dst_pool";
  case MFCP_PARAM_TYPE:
    return "type";
  case MFCP_PARAM_BANDWIDTH:
    return "bandwith";
  case MFCP_PARAM_DATA:
    return "data";
  case MFCP_PARAM_PEER_RESOURCE_ID:
    return "peer_resource_id";
  case MFCP_PARAM_OPTYPE:
    return "optype";
  case MFCP_PARAM_DTMFDETECT:
    return "dtmf_detect";
  case MFCP_PARAM_DTMFDETECTPARAM:
    return "dtmf_detect_param";
  case MFCP_PARAM_NAT_SRC:
    return "nat_src";
  case MFCP_PARAM_NAT_DEST:
    return "nat_dst";
  case MFCP_PARAM_DEST_SYM:
    return "dst_sym";
  case MFCP_PARAM_NONE:
    return NULL;
  case MFCP_PARAM_END:
    return NULL;
  }
  return NULL;
}

/* returns string from dst_sym parameter type */

char *
mfcp_dst_sym_param_str (MFCP_DestSymType type)
{
  switch(type)
  {
    case MFCP_DEST_SYM_DISC:
      return "disc";
    case MFCP_DEST_SYM_FORCE:
      return "force";
    case MFCP_DEST_SYM_NONE:
      return NULL;
  }
  return NULL;
}

int
mfcp_dst_sym_str_to_int(const char *val)
{
  if(val == NULL)
    return MFCP_DEST_SYM_NONE;
  else if(strcmp(val, "disc") == 0)
    return MFCP_DEST_SYM_DISC;
  else if(strcmp(val, "force") == 0)
    return MFCP_DEST_SYM_FORCE;
  else return MFCP_DEST_SYM_NONE;
}

MFCP_ParameterTypes
mfcp_param_gettype (char *str)
{
  int i;
  for (i = 0 ; mfcpLookupParamTable[i].pname ; i++) {
    if (strcasecmp( mfcpLookupParamTable[i].pname, str) == 0) {
      return mfcpLookupParamTable[i].type;
    }
  }
  return MFCP_PARAM_NONE;
}

/* Request Methods */

MFCP_Request *
mfcp_req_new (MFCP_RequestType type)
{
  MFCP_Request *reqPtr;
  if ((reqPtr = calloc (1,sizeof (MFCP_Request))) == NULL) {
    mfcp_logit ("Malloc Error");
    return NULL;
  }
  reqPtr->type = type;
  reqPtr->state = MFCP_RSTATE_NONE;

  pthread_mutex_init(&reqPtr->syncLock,NULL);
  NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_req_new %p\n",reqPtr));
  return reqPtr;
}

/* set the callback and opaque pointer into the request struct */
int
mfcp_req_callback(MFCP_Request *rPtr,void (*func)(void *), void *appData) {
  rPtr->func = func;
  rPtr->appData = appData;
  return MFCP_RET_OK;
}

MFCP_Request *
mfcp_req_free (MFCP_Request * r)
{
  mfcp_param_free (r->pFirst);  /* free the request parameters */
  mfcp_param_free (r->rFirst);  /* free the response parameters */
  pthread_mutex_destroy(&r->syncLock);

  NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_req_free %p\n",r));
  free (r);
  mfcp_logit("Freed request struct");
  return NULL;
}

char *
mfcp_req_str (MFCP_RequestType type)
{
  switch (type) {
  case MFCP_REQ_CRS:
    return "CRS";
  case MFCP_REQ_MDS:
    return "MDS";
  case MFCP_REQ_CRB:
    return "CRB";
  case MFCP_REQ_CRR:
    return "CRR";
  case MFCP_REQ_MDR:
    return "MDR";
  case MFCP_REQ_CHR:
    return "CHR";
  case MFCP_REQ_DLR:
    return "DLR";
  case MFCP_REQ_DLB:
    return "DLB";
  case MFCP_REQ_DLS:
    return "DLS";
  case MFCP_REQ_AUS:
    return "AUS";
  case MFCP_REQ_HLP:
    return "HLP";
  case MFCP_REQ_PNG:
    return "PNG";
  case MFCP_REQ_NTF:
    return "NTF";
  case MFCP_REQ_RQT:
    return "RQT";
  case MFCP_REQ_RSP:
    return "RSP";
  default:
    return "";
  }
}

int
mfcp_req_add_param (MFCP_Request * req, MFCP_ParameterTypes param,
                    MFCP_Value * value)
{
  MFCP_Parameter *paramPtr;

  paramPtr = mfcp_param_new (param);
  paramPtr->value = value;

  if (req->pFirst == NULL) {
    req->pFirst = paramPtr;
    req->pLast = paramPtr;
  } else {
    req->pLast->next = paramPtr;
    req->pLast = paramPtr;
  }
  return MFCP_RET_OK;
}

int
mfcp_res_add_param (MFCP_Request * req, MFCP_ParameterTypes param,
                    MFCP_Value * value)
{
  MFCP_Parameter *paramPtr;

  paramPtr = mfcp_param_new (param);
  paramPtr->value = value;

  if (req->rFirst == NULL) {
    req->rFirst = paramPtr;
    req->rLast = paramPtr;
  } else {
    req->rLast->next = paramPtr;
    req->rLast = paramPtr;
  }
  return MFCP_RET_OK;
}

int
mfcp_mkpkt (MFCP_Request * req, char *buff, unsigned int bufferSize,
                int seq)
{
  MFCP_Parameter *pPtr;
  char *cp = buff;

  //+++ need to check the buf size and deal with it  

  if (req->type == MFCP_REQ_RSP) {
    sprintf (cp, "MFCP/1.0 %s %d %d \"%s\"\n", mfcp_req_str (req->type), seq,
             req->respStatus,req->respString);
    pPtr = req->rFirst;
  } else {
    sprintf (cp, "MFCP/1.0 %s %d\n", mfcp_req_str (req->type), seq);
    pPtr = req->pFirst;
  }
  cp += strlen (cp);

  for ( ; pPtr != NULL; pPtr = pPtr->next) {
    sprintf (cp, "%s=", mfcp_param_str (pPtr->type));
    cp += strlen (cp);
    mfcp_value_vtoa (pPtr->value, cp, MFCP_MAX_STRING);
    cp += strlen (cp);
    *cp++ = '\n';
  }
  *cp++ = '\n';
  *cp = '\0';
  return MFCP_RET_OK;
}



int
mfcp_req_mkpkt (MFCP_Request * req, char *buff, unsigned int bufferSize,
                int seq)
{
  return mfcp_mkpkt (req, buff,bufferSize, seq);
}

int
mfcp_res_mkpkt (MFCP_Request * req, char *buff, unsigned int bufferSize,
                int seq)
{
  req->type = MFCP_REQ_RSP;
  return mfcp_mkpkt (req, buff,bufferSize, seq);
}

MFCP_Request *
mfcp_req_lookup (MFCP_Session * sess, int rId)
{
  return mfcp_req_check_expiry_lookup(sess, rId, 0);
}

MFCP_Request *
mfcp_req_check_expiry_lookup (MFCP_Session * sess, int rId, int delete_expired)
{
  MFCP_Request *hPtr = (MFCP_Request *) &sess->requestHead;
  MFCP_Request *nextPtr, *rPtr = sess->requestHead.sentFirst;
  struct timeval now;

  if (gettimeofday(&now, NULL) < 0) {
    perror("mfcp_req_lookup");
    delete_expired = 0;
  }

  while (rPtr != hPtr) {
    if (rPtr->seqId == rId) {
      return rPtr;
    }
    nextPtr = rPtr->prev;
    if (delete_expired && MFCP_REQ_EXPIRED(&(rPtr->time), &now)) {
          NETDEBUG(MFCE, NETLOG_DEBUG4, ("mfcp_check_expiry: seqid = %d expired. executing callback\n", rPtr->seqId));	
	  mfcp_process_rsp(sess, rPtr);
    } 
    rPtr = nextPtr;
  }
  return NULL;
}

/* return the a pointer to the value struct or NULL on failure */
MFCP_Value *
mfcp_req_getval (MFCP_Request * rPtr, MFCP_ParameterTypes type)
{
  MFCP_Parameter *pPtr;
  for (pPtr = rPtr->pFirst; pPtr != NULL; pPtr = pPtr->next) {
    if (pPtr->type == type) {
      return pPtr->value;
    }
  }
  return NULL;
}

/* return the pointer to the value struct or NULL on failure */
MFCP_Value *
mfcp_res_getval (MFCP_Request * rPtr, MFCP_ParameterTypes type)
{
  MFCP_Parameter *pPtr;
  for (pPtr = rPtr->rFirst; pPtr != NULL; pPtr = pPtr->next) {
    if (pPtr->type == type) {
      return pPtr->value;
    }
  }
  return NULL;
}

/* display a packet  debug */
int
mfcp_req_display (MFCP_Request * rPtr,char *caller)
{
  char dbuff[1024];
  char vbuff[1024];
  MFCP_Parameter *pPtr;
  snprintf (dbuff, sizeof (dbuff), "type: %s, id: %d, status: %d, func %s",
            mfcp_req_str (rPtr->type), rPtr->seqId, rPtr->respStatus, rPtr->func?"yes":"no");
  NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: %s\n",caller,dbuff));

  for (pPtr = rPtr->pFirst; pPtr != NULL; pPtr = pPtr->next) {
    mfcp_value_vtoa (pPtr->value, vbuff, sizeof (vbuff));
    snprintf (dbuff, sizeof (dbuff), "req: param: %s = %s",
              mfcp_param_str (pPtr->type), vbuff);
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: %s\n",caller,dbuff));
  }

  for (pPtr = rPtr->rFirst; pPtr != NULL; pPtr = pPtr->next) {
    mfcp_value_vtoa (pPtr->value, vbuff, sizeof (vbuff));
    snprintf (dbuff, sizeof (dbuff), "rsp: param: %s = %s",
              mfcp_param_str (pPtr->type), vbuff);
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: %s\n",caller,dbuff));
  }

  return(MFCP_RET_OK);
}

int 
mfcp_sess_display(MFCP_Session *sess, char *caller) {
  // MFCP_Request *rPtr;

  pthread_mutex_lock (&sess->sessLock);    
  if (sess) {
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: id = %d\n",caller,sess->id));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: seqId = %d\n",caller,sess->seqId));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: sessType = %d\n",caller,sess->sessType));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: state = %s\n",caller,mfcp_sess_state_str[sess->state]));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: appType = %d\n",caller,sess->appType));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: numReq = %d\n",caller,sess->numReq));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: fd = %d\n",caller,sess->fd));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: peer Address = %x\n",caller,sess->peerAddr.sin_addr.s_addr));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: peer Port = %d\n",caller,sess->peerAddr.sin_port));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: local Address = %x\n",caller,sess->localAddr.sin_addr.s_addr));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: local Port = %d\n",caller,sess->localAddr.sin_port));

    /* +++BRM
    for (rPtr = sess->requestHead.sentLast; rPtr != &sess->requestHead.sentLast; rPtr = rPtr->prev) {
      NETDEBUG(MFCE,NETLOG_DEBUG4,("%s: sent request %x\n",caller,(int)rPtr));
      mfcp_req_display(rPtr,caller);
    }
    */
  } 
  pthread_mutex_unlock (&sess->sessLock);
  return 0;
}

/*
 ****************************************************************
 * 
 * convience routines to make the packets and send them
 *
 ****************************************************************
 */

MFCP_Return
mfcp_req_crs (MFCP_Session * sess, int shutdownInterval,
	      void (*func)(void *), void *appData)
{

  MFCP_Request *rPtr;
  MFCP_Return ret;

  if (!sess) {
    return (MFCP_RET_NOTOK);
  }

  rPtr = mfcp_req_new (MFCP_REQ_CRS);
  mfcp_req_callback(rPtr,func,appData);

  mfcp_req_add_param (rPtr, MFCP_PARAM_SESSION_ID,
                      mfcp_value_new_i (sess->id));
  mfcp_req_add_param (rPtr, MFCP_PARAM_REPORT_ERRORS,
                      mfcp_value_new_s ("yes"));
  mfcp_req_add_param (rPtr, MFCP_PARAM_SHUTDOWN_INTERVAL,
                      mfcp_value_new_i (shutdownInterval));

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { 
    pthread_mutex_lock(&rPtr->syncLock);
    *(MFCP_Request **)appData = rPtr;
  } /* wait for the response to unlock */

  return(ret);

}

MFCP_Return
mfcp_req_mds (MFCP_Session * sess, int shutdownInterval,
	      void (*func)(void *), void *appData)
{
  MFCP_Request *rPtr;
  MFCP_Return ret;

  rPtr = mfcp_req_new (MFCP_REQ_MDS);
  mfcp_req_callback(rPtr,func,appData);

  mfcp_req_add_param (rPtr, MFCP_PARAM_SHUTDOWN_INTERVAL,
                      mfcp_value_new_i (shutdownInterval));

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { 
    pthread_mutex_lock(&rPtr->syncLock);
    *(MFCP_Request **)appData = rPtr;
  } /* wait for the response to unlock */

  return(ret);

}

MFCP_Return
mfcp_req_dls (MFCP_Session * sess,
	      void (*func)(void *), void *appData)
{
  MFCP_Request *rPtr;
  MFCP_Return ret;

  if (!sess) {
    mfcp_logit("NULL session pointer");
    return (MFCP_RET_NOTOK);
  }

  rPtr = mfcp_req_new (MFCP_REQ_DLS);
  mfcp_req_callback(rPtr,func,appData);

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { 
    pthread_mutex_lock(&rPtr->syncLock);
    *(MFCP_Request **)appData = rPtr;
  } /* wait for the response to unlock */

  return(ret);
}

MFCP_Return
mfcp_req_crr (MFCP_Session * sess,
              int bundleId,
              MFCP_ResourceId peerResourceId,
              unsigned int saddr, unsigned short sport,
              unsigned int daddr, unsigned short dport, 
              unsigned int nat_saddr, unsigned short nat_sport,
              unsigned int nat_daddr, unsigned short nat_dport, 
	      unsigned int ingressPool, unsigned int egressPool,
	      char *protocol, int dtmf_detect, int dtmf_detect_param,
	      unsigned int dst_sym, int optype,
	      void (*func)(void *), void *appData)
{

  MFCP_Request *rPtr;
  MFCP_Return ret;
  char saddrString[25];
  char daddrString[25];
  char nat_saddrString[25];
  char nat_daddrString[25];

  if (!sess) {
    mfcp_logit("NULL session pointer");
    return (MFCP_RET_NOTOK);
  }

  rPtr = mfcp_req_new (MFCP_REQ_CRR);
  mfcp_req_callback(rPtr,func,appData);

  mfcp_req_add_param (rPtr, MFCP_PARAM_BUNDLE_ID,
                      mfcp_value_new_i (bundleId));

  mfcp_req_add_param (rPtr, MFCP_PARAM_SRC,
                      mfcp_value_new_s (mfcp_make_addrstring
                                        (saddrString, saddr, sport)));
  mfcp_req_add_param (rPtr, MFCP_PARAM_DEST,
                      mfcp_value_new_s (mfcp_make_addrstring
                                        (daddrString, daddr, dport)));
  mfcp_req_add_param(rPtr, MFCP_PARAM_DEST_POOL,
		     mfcp_value_new_i(ingressPool));

  mfcp_req_add_param(rPtr, MFCP_PARAM_SRC_POOL,
		     mfcp_value_new_i(egressPool));

  mfcp_req_add_param (rPtr, MFCP_PARAM_PROTOCOL, mfcp_value_new_s (protocol));

  if (IS_LOCAL_FW) {
  	if(nat_saddr || nat_sport) {
  		mfcp_req_add_param (rPtr, MFCP_PARAM_NAT_SRC,
       	               mfcp_value_new_s (mfcp_make_addrstring
       	                                 (nat_saddrString, nat_saddr, nat_sport)));
  	}

  	if(nat_daddr || nat_dport) {
  		mfcp_req_add_param (rPtr, MFCP_PARAM_NAT_DEST,
       	               mfcp_value_new_s (mfcp_make_addrstring
       	                                 (nat_daddrString, nat_daddr, nat_dport)));
  	}

	mfcp_req_add_param (rPtr, MFCP_PARAM_PEER_RESOURCE_ID,
				mfcp_value_new_i (peerResourceId));

	mfcp_req_add_param(rPtr, MFCP_PARAM_OPTYPE,
				mfcp_value_new_i(optype));

	if(dst_sym) {
		mfcp_req_add_param (rPtr, MFCP_PARAM_DEST_SYM,
			mfcp_value_new_s (mfcp_dst_sym_param_str(dst_sym)));
	}
  }

  if (dtmf_detect) {
  	mfcp_req_add_param (rPtr, MFCP_PARAM_DTMFDETECT,
                      mfcp_value_new_s ("yes"));
  }

  if (dtmf_detect_param) {
  	mfcp_req_add_param(rPtr, MFCP_PARAM_DTMFDETECTPARAM,
		     mfcp_value_new_i(dtmf_detect_param));
  }

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { 
    pthread_mutex_lock(&rPtr->syncLock);
    *(MFCP_Request **)appData = rPtr;
  } /* wait for the response to unlock */

  return(ret);
}

MFCP_Return mfcp_req_chr(MFCP_Session *sess, 
			 int bundleId,
			 MFCP_ResourceId resourceId, 
			 MFCP_ResourceId peerResourceId, 
			 unsigned int saddr, unsigned short sport,
			 unsigned int daddr, unsigned short dport, 
			 unsigned int nat_saddr, unsigned short nat_sport,
			 unsigned int nat_daddr, unsigned short nat_dport, 
			 unsigned int ingressPool, unsigned int egressPool,
			 char *protocol, int dtmf_detect, int dtmf_detect_param,
			 int optype,
			 void (*func)(void *), void *appData)
{
  MFCP_Request *rPtr;
  MFCP_Return ret;
  char daddrString[25];
  char saddrString[25];
  char nat_daddrString[25];
  char nat_saddrString[25];

  if (!sess) {
    mfcp_logit("NULL session pointer");
    return (MFCP_RET_NOTOK);
  }

  /* +++ if we have a ingressPool then we assume it's a create of a new resource 
     and we leave the original resource which will get cleaned up when the delete bundle
     gets called
  */
  if (IS_LOCAL_FW) {
  	rPtr = mfcp_req_new (MFCP_REQ_CHR);
  }
  else
  {
  	rPtr = mfcp_req_new (MFCP_REQ_MDR);
  }

  mfcp_req_add_param (rPtr, MFCP_PARAM_RESOURCE_ID,
		     mfcp_value_new_i(resourceId));

  mfcp_req_callback(rPtr,func,appData);

  mfcp_req_add_param (rPtr, MFCP_PARAM_BUNDLE_ID,
                      mfcp_value_new_i (bundleId));

  mfcp_req_add_param (rPtr, MFCP_PARAM_SRC,
                      mfcp_value_new_s (mfcp_make_addrstring
                                        (saddrString, saddr, sport)));

  mfcp_req_add_param (rPtr, MFCP_PARAM_DEST,
                      mfcp_value_new_s (mfcp_make_addrstring
                                        (daddrString, daddr, dport)));

  mfcp_req_add_param(rPtr, MFCP_PARAM_DEST_POOL,
		     mfcp_value_new_i(ingressPool));

  mfcp_req_add_param(rPtr, MFCP_PARAM_SRC_POOL,
		     mfcp_value_new_i(egressPool));

  mfcp_req_add_param (rPtr, MFCP_PARAM_PROTOCOL, mfcp_value_new_s (protocol));

  if (IS_LOCAL_FW) {

  	if(nat_saddr || nat_sport) {
  		mfcp_req_add_param (rPtr, MFCP_PARAM_NAT_SRC,
       	               mfcp_value_new_s (mfcp_make_addrstring
       	                                 (nat_saddrString, nat_saddr, nat_sport)));
  	}

  	if(nat_daddr || nat_dport) {
  		mfcp_req_add_param (rPtr, MFCP_PARAM_NAT_DEST,
       	               mfcp_value_new_s (mfcp_make_addrstring
       	                                 (nat_daddrString, nat_daddr, nat_dport)));
  	}

  	mfcp_req_add_param (rPtr, MFCP_PARAM_PEER_RESOURCE_ID,
		     mfcp_value_new_i(peerResourceId));

	mfcp_req_add_param(rPtr, MFCP_PARAM_OPTYPE,
		     mfcp_value_new_i(optype));
  }

  if (dtmf_detect) {
  	mfcp_req_add_param (rPtr, MFCP_PARAM_DTMFDETECT,
                      mfcp_value_new_s ("yes"));
  }

  if (dtmf_detect_param) {
  	mfcp_req_add_param(rPtr, MFCP_PARAM_DTMFDETECTPARAM,
		     mfcp_value_new_i(dtmf_detect_param));
  }

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { pthread_mutex_lock(&rPtr->syncLock);
    *(MFCP_Request **)appData = rPtr;
  } /* wait for the response to unlock */
  
  return(ret);
}


MFCP_Return
mfcp_req_dlr (MFCP_Session * sess, int bundleId, int resourceId,
	      void (*func)(void *), void *appData)
{
  MFCP_Request *rPtr;
  MFCP_Return ret;

  if (!sess) {
    mfcp_logit("NULL session pointer");
    return (MFCP_RET_NOTOK);
  }

  rPtr = mfcp_req_new (MFCP_REQ_DLR);
  mfcp_req_callback(rPtr,func,appData);

  mfcp_req_add_param (rPtr, MFCP_PARAM_RESOURCE_ID,
                      mfcp_value_new_i (resourceId));
  mfcp_req_add_param (rPtr, MFCP_PARAM_BUNDLE_ID,
                      mfcp_value_new_i (bundleId));

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* wait for the response to unlock */

  return(MFCP_RET_OK);
}

MFCP_Return
mfcp_req_dlb (MFCP_Session * sess, int bundleId,
	      void (*func)(void *), void *appData)
{

  MFCP_Request *rPtr;
  MFCP_Return ret;

  if (!sess) {
    mfcp_logit("NULL session pointer");
    return (MFCP_RET_NOTOK);
  }

  rPtr = mfcp_req_new (MFCP_REQ_DLB);

  mfcp_req_callback(rPtr,func,appData);

  mfcp_req_add_param (rPtr, MFCP_PARAM_BUNDLE_ID,
                      mfcp_value_new_i (bundleId));

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* wait for the response to unlock */

  return(ret);
}

MFCP_Return
mfcp_req_aus (MFCP_Session * sess,
	      void (*func)(void *), void *appData)
{
  MFCP_Request *rPtr;
  MFCP_Return ret;

  if (!sess) {
    mfcp_logit("NULL session pointer");
    return (MFCP_RET_NOTOK);
  }

  rPtr = mfcp_req_new (MFCP_REQ_AUS);
  mfcp_req_callback(rPtr,func,appData);

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* take the initial lock */

  if ((ret = mfcp_sess_send_req (sess, rPtr)) != MFCP_RET_OK) {
    if (!func) { pthread_mutex_unlock(&rPtr->syncLock); }
    mfcp_req_free(rPtr);
    return ret;
  }

  if (!func) { pthread_mutex_lock(&rPtr->syncLock); } /* wait for the response to unlock */


  return(ret);
}

/***************************** sync routines ***********************************
 * if the function pointer is NULL then the appData pointer is used as a 
 * handle to pass the rPtr back up.
 */

MFCP_Request  *mfcp_sreq_dlr (MFCP_Session * sess, int bundleId, int resourceId) {
  MFCP_Request *rPtr = NULL;
  mfcp_req_dlr(sess,bundleId,resourceId,NULL,(void *)&rPtr);
  return (rPtr);
}

MFCP_Request  *mfcp_sreq_crr (MFCP_Session * sess,
                           int bundleId,
              			   MFCP_ResourceId peerResourceId,
                           unsigned int saddr, unsigned short sport,
                           unsigned int daddr, unsigned short dport,
                           unsigned int nat_saddr, unsigned short nat_sport,
                           unsigned int nat_daddr, unsigned short nat_dport,
                           unsigned int egressPool, unsigned int ingressPool,
			     char *protocol, int dtmf_detect, int dtmf_detect_param, unsigned int dst_sym) {
  MFCP_Request *rPtr = NULL;
  mfcp_req_crr (sess,
		bundleId,
		peerResourceId,
		saddr,sport,
		daddr, dport,
		nat_saddr,nat_sport,
		nat_daddr, nat_dport,
		egressPool, ingressPool,
		protocol, dtmf_detect, dtmf_detect_param, dst_sym,
		0, NULL, (void *)&rPtr);
  
  return(rPtr);
}

MFCP_Request *mfcp_sreq_chr(MFCP_Session *sess,
                         int bundleId,
                         MFCP_ResourceId resourceId,
			 			 MFCP_ResourceId peerResourceId, 
                         unsigned int saddr, unsigned short sport,
                         unsigned int daddr, unsigned short dport,
                         unsigned int nat_saddr, unsigned short nat_sport,
                         unsigned int nat_daddr, unsigned short nat_dport,
                         unsigned int ingressPool, unsigned int egressPool,
			   char *protocol, int dtmf_detect, int dtmf_detect_param) {
  MFCP_Request *rPtr = NULL;

  mfcp_req_chr(sess,
	       resourceId,
		   peerResourceId,
	       bundleId,
	       saddr, sport,
	       daddr,  dport,
	       nat_saddr, nat_sport,
	       nat_daddr,  nat_dport,
	       ingressPool, egressPool,
	       protocol, dtmf_detect, dtmf_detect_param,
		   0, NULL, (void *)&rPtr);
  return (rPtr);
}


MFCP_Request *mfcp_sreq_dlb (MFCP_Session * sess, int bundleId) {
  MFCP_Request *rPtr = NULL;

  mfcp_req_dlb (sess, bundleId,NULL,(void *)&rPtr);
  return (rPtr);
}


MFCP_Request *mfcp_sreq_crs (MFCP_Session * sess, int shutdownInterval) {
  MFCP_Request *rPtr = NULL;

  mfcp_req_crs(sess,
	       shutdownInterval,
	       NULL, (void *)&rPtr);
  return (rPtr);
}

/*
MFCP_Return mfcp_req_aus (MFCP_Session * sess,
                  void (*func)(void *), void *appData);
*/

MFCP_Request *mfcp_sreq_dls (MFCP_Session * sess) {
  MFCP_Request *rPtr = NULL;

  mfcp_req_dls (sess, NULL, (void *) &rPtr);
  return (rPtr);
}

/********************************** end of syn routines ************************************/


/* response methods to be used in callback */

unsigned int mfcp_get_dest_addr(MFCP_Request *rPtr) {
  char *addrStr;
  unsigned int addr = 0;
  uint16_t port = 0;
  addrStr = mfcp_value_getstr(mfcp_res_getval(rPtr,MFCP_PARAM_DEST));
  mfcp_get_addrport (addrStr, &addr, &port);
  return (addr);
}

int mfcp_get_dest_port(MFCP_Request *rPtr) {
  char *addrStr;
  unsigned int addr =0;
  uint16_t port =0;
  addrStr = mfcp_value_getstr(mfcp_res_getval(rPtr,MFCP_PARAM_DEST));
  mfcp_get_addrport (addrStr, &addr, &port);
  return (port);
}

unsigned int mfcp_get_nat_dest_addr(MFCP_Request *rPtr) {
  char *addrStr;
  unsigned int addr = 0;
  uint16_t port = 0;
  addrStr = mfcp_value_getstr(mfcp_res_getval(rPtr, IS_LOCAL_FW ? MFCP_PARAM_NAT_DEST : MFCP_PARAM_DEST));
  mfcp_get_addrport (addrStr, &addr, &port);
  return (addr);
}

int mfcp_get_nat_dest_port(MFCP_Request *rPtr) {
  char *addrStr;
  unsigned int addr =0;
  uint16_t port =0;
  addrStr = mfcp_value_getstr(mfcp_res_getval(rPtr, IS_LOCAL_FW ? MFCP_PARAM_NAT_DEST : MFCP_PARAM_DEST));
  mfcp_get_addrport (addrStr, &addr, &port);
  return (port);
}

unsigned int mfcp_get_nat_src_addr(MFCP_Request *rPtr) {
  char *addrStr;
  unsigned int addr = 0;
  uint16_t port = 0;
  addrStr = mfcp_value_getstr(mfcp_res_getval(rPtr, IS_LOCAL_FW ? MFCP_PARAM_NAT_SRC : MFCP_PARAM_SRC));
  mfcp_get_addrport (addrStr, &addr, &port);
  return (addr);
}

int mfcp_get_nat_src_port(MFCP_Request *rPtr) {
  char *addrStr;
  unsigned int addr =0;
  uint16_t port =0;
  addrStr = mfcp_value_getstr(mfcp_res_getval(rPtr, IS_LOCAL_FW ? MFCP_PARAM_NAT_SRC : MFCP_PARAM_SRC));
  mfcp_get_addrport (addrStr, &addr, &port);
  return (port);
}

MFCP_Status mfcp_get_res_status(MFCP_Request *rPtr) {
  return (rPtr->respStatus);
}

MFCP_Session *mfcp_get_sess(MFCP_Request *rPtr) {
	return (rPtr->sess);
}

char * mfcp_get_res_estring(MFCP_Request *rPtr) {
  return (rPtr->respString);
}

void *mfcp_get_res_appdata(MFCP_Request *rPtr) {
  return((void *)rPtr->appData);
}

MFCP_RequestType mfcp_get_res_type(MFCP_Request *rPtr) {
  return(rPtr->type);
}

int mfcp_get_int(MFCP_Request *rPtr, MFCP_ParameterTypes param) {
  return (mfcp_value_getint(mfcp_res_getval(rPtr,param)));
}

char *
 mfcp_get_str(MFCP_Request *rPtr, MFCP_ParameterTypes param) {
  return (mfcp_value_getstr(mfcp_res_getval(rPtr,param)));
}


/* misc functions */

void *mfcp_parse(void *tmpSess) {
  MFCP_Request *rPtr;
  MFCP_Session *sess = (MFCP_Session *)tmpSess;

  while(1) {

    rPtr = lexor_parseit(sess);
    if (rPtr == NULL) {
      if (sess->state == MFCP_EOF) {
	NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_parse: spawn mfcp_reconnect\n"));
	mfcp_sess_reconnect(sess);
      }
    } else {
      /* here we dispatch the response back */
      mfcp_process_rsp(sess, rPtr);
    }
  }

  return 0;
}

void mfcp_process_rsp(MFCP_Session *sess, MFCP_Request *rPtr) {
  mfcp_sess_rem_sent_req(sess, rPtr);
  mfcp_req_display(rPtr,"mfcp_rsp");
  if (rPtr->func) {                        /* if the func is a valid function then we assume is a async call */
    NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_rsp: call the callback\n"));
    (rPtr->func)((void *)rPtr);
  } else {                                 /* Here if the request was a sync call */
    NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp_rsp: unlock the request pointer mutex\n"));
    pthread_mutex_unlock(&rPtr->syncLock); /* let the caller continue */
  }
}

MFCP_RequestType
mfcp_req_gettype (char *str)
{
  int i;
  char *cp;
  char *sp;
  for (i = 0; mfcpLookupTable[i].rname; i++) {
    sp = str;
    for (cp = mfcpLookupTable[i].rname; *cp; cp++) {
      if (*cp != *sp++) {
        break;
      }
    }
    if (!*cp) {
      return mfcpLookupTable[i].type;
    }
  }
  return MFCP_REQ_NONE;
}


/* convert a address/port into a string */
char *
mfcp_make_addrstring (char *buff, unsigned int addr, unsigned short port)
{
  sprintf (buff, "%d.%d.%d.%d:%d", addr >> 24, (addr & 0xFF0000) >> 16,
           (addr & 0xFF00) >> 8, addr & 0xFF, port);
  return buff;
}

int
mfcp_get_addrport (char *addrStr, in_addr_t * addr, uint16_t *port)
{
  char buff[20];
  char *cp, *cpd;

  if (addrStr == NULL || addr == NULL || port == NULL) {
    return(MFCP_RET_NOTOK);
  }

  cpd = buff;
  for (cp = addrStr; (*cp != '\0') && (*cp != ':'); cp++) {
    *cpd++ = *cp;
  }
  *cpd = '\0';
  if ((*addr = ntohl(inet_addr (buff))) == -1) {
    return (MFCP_RET_NOTOK);
  }
  cp++;
  *port = atoi(cp);
  return (MFCP_RET_OK);
}

int
mfcp_send (MFCP_Session *sess, char *cbuf, int size, int flags)
{
  int ret;
  ret = send (sess->fd, cbuf, strlen (cbuf), 0);
  return ret;
}

int
mfcp_version_check (int majorVersion, int minorVersion)
{
  if (majorVersion == MFCP_VERSION_MAJOR) {
    return 1;
  }
  return 0;
}

void
mfcp_reconfig(MFCP_Session *sess)
{
  if (!sess) {
    NETDEBUG(MFCE, NETLOG_DEBUG2, ("mfcp_reconfig: session not found, nothing to reconfig\n"));
    return;
  }

  if (sess->sessType == MFCP_SESS_LOCAL) {
    nsfGlueReconfig();
    NETDEBUG(MFCE, NETLOG_DEBUG2, ("mfcp_reconfig: reconfiguring nsf\n"));
  } else {
    NETDEBUG(MFCE, NETLOG_DEBUG2, ("mfcp_reconfig: no reconfiguring done, please stop and start iServer.\n"));
  }

}
