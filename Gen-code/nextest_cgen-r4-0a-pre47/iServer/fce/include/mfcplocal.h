/*
 * Copyright (c) 2003 NExtone Communications, Inc.
 * All rights reserved.
 */

#ifndef _LIBMFCPLOCAL_H
#define _LIBMFCPLOCAL_H

#include <pthread.h>
#include <netinet/in.h>

/*****************************************************************************
 *
 *  Definitions
 *
*****************************************************************************/
#define MFCP_VERSION_MAJOR 1
#define MFCP_VERSION_MINOR 0
#define MFCP_PORT 8040
#define MFCP_MAX_STRING 1024
#define MFCP_LIST_DELIM ','
#define MFCP_MINSTR_SIZE 40

/*****************************************************************************
 *
 *  Enumerations
 *
*****************************************************************************/

typedef enum
{
  MFCP_NOT_INITED = 0,
  MFCP_EOF = 1,
  MFCP_TCP_CONNECTED = 2,
  MFCP_SESS_CONNECTING = 3,
  MFCP_SESS_CONNECTED = 4,
#define MFCP_SESS_STATE_MAX 5
} SessionState;

#ifdef _MFCP_SESS_STATE_
char mfcp_sess_state_str[MFCP_SESS_STATE_MAX][40] = {
  "MFCP_NOT_INITED",
  "MFCP_EOF",
  "MFCP_TCP_CONNECTED",
  "MFCP_SESS_CONNECTING",
  "MFCP_SESS_CONNECTED"
};
#endif

typedef enum
{
  MFCP_SESS_TCP = 1,
  MFCP_SESS_LOCAL = 2
} MFCP_SessionTypes;

typedef enum
{
  MFCP_VALUE_NONE = 0,
  MFCP_VALUE_INTEGER = 1,
  MFCP_VALUE_STRING = 2,
  MFCP_VALUE_END = 3
} MFCP_ValueTypes;

typedef enum
{
  MFCP_RSTATE_NONE = 0,
  MFCP_RSTATE_SENT = 1,
  MFCP_RSTATE_RECV = 2,
  MFCP_RSTATE_RSP  = 3,
  MFCP_RSTATE_END = 4
} MFCP_RequestState;

typedef enum
{
  MFCP_APP_CLIENT = 1,
  MFCP_APP_SERVER = 2
} MFCP_AppType;

/*****************************************************************************
 *
 *  Structure Definitions
 *
*****************************************************************************/

typedef struct
{
  int id;
  int seqId;
  MFCP_SessionTypes sessType;
  SessionState state;
  MFCP_AppType appType;
  pthread_mutex_t sessLock;
  void (*asyncFunc)(void *);
  int peerVersionMajor;
  int peerVersionMinor;
  int fd;
  struct sockaddr_in peerAddr;
  struct sockaddr_in localAddr;
  time_t lastPktTime;
  int keepAliveInterval;
  int numReq;
  unsigned int lastRequestIdSent;
  struct
  {
    struct _MFCP_Request *sentFirst, *sentLast; /* effective header of the clist links */
  } requestHead;
} MFCP_Session;

typedef struct _MFCP_Value
{
  struct _MFCP_Value *next;
  MFCP_ValueTypes type;
  int i;                        /* if a integer then the int value */
  char *s;                      /* ptr to the string of the value */
  char sbuf[MFCP_MINSTR_SIZE + 1];      /* if a small string then use this
                                           storage else malloc */
} MFCP_Value;


typedef struct _MFCP_Parameter
{
  struct _MFCP_Parameter *prev, *next;
  MFCP_ParameterTypes type;
  MFCP_Value *value;

} MFCP_Parameter;

typedef struct _MFCP_Request
{
  struct _MFCP_Request *prev, *next;    /* used to chain in a session */
  MFCP_Session *sess;           /* the session that this request was sent/recv */
  MFCP_RequestType type;        /* the type of request */
  MFCP_RequestState state;      /* current state of request */
  struct timeval time;		/* time when request was sent */
  pthread_mutex_t syncLock;     /* mutex to be used for sync call */
  int respStatus;               /* response status */
  char respString[128];         /* response string */
  int seqId;                    /* sequence ID */
  MFCP_Parameter *pFirst;       /* request params single linked */
  MFCP_Parameter *pLast;        /* pointer to the last entry in the list */
  MFCP_Parameter *rFirst;       /* response params  single linked */
  MFCP_Parameter *rLast;        /* pointer to the last entry in the list */
  void (*func)(void *);         /* call back function */
  void *appData;                /* opaque pointer for the application layer */
} MFCP_Request;

#define MFCP_REQ_EXPIRY 5
// #define MFCP_REQ_QLEN	500
#define MFCP_REQ_QLEN	        1000 /* Bob */

/* MFCP_REQ_EXPIRED returns non-zero if t is not zero and seconds part of
 * now is more than MFCP_REQ_EXPIRY seconds more than t 			*/
 
#define MFCP_REQ_EXPIRED(t, now) ( (!(((t)->tv_sec == 0) && ((t)->tv_usec == 0))) && \
				   ((now)->tv_sec > ((t)->tv_sec + MFCP_REQ_EXPIRY)) )

#endif /* _LIBMFCPLOCAL_H */
