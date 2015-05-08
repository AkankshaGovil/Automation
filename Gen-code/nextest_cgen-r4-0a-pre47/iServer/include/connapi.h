#ifndef _connapi_h_
#define _connapi_h_

/* Connection Entry. Externally made available
 * by caller, and used by connection establishment
 * mechanism. For registration purposes its usually
 * part of the SrvrEntry structure
 */
typedef struct 
{
	struct sockaddr_in 	addr;
	struct itimerval	tmout;
	unsigned short 		xTries;
} ConnEntry;

#define APP_CONNECTED	0
#define APP_CONNINPROG	1
#define APP_CONNERROR	2
#define APP_CONNTIMEOUT	3
#define APP_CONNEXISTS 	4

#define APP_CONNMAXCBS	5

typedef int (*connAppCB)(int, void *);

/* Private to the Connection Handling */
/* This is allocated inside the connection handling procs,
 * and freed by calling the macro below
 */
/*
 * The ConnEntry structure in the ConnHandle may be
 * a SrvrEntry (see below), or any structure which
 * is top-identical to ConnEntry
 */
typedef struct
{
	ConnEntry		*connEntry;

	unsigned short		nblock;
	tid			timer;
	int			fd;
	
	/* How many people are using this connection entry */
     	unsigned short 		refCount;

	/* No of times we will try connecting to the server.
	 * This parameter is only for connection establishment
	 */
	short			nTries;

	/* Data Stored by the user, if required, supplied
	 * as callback within the handle
	 */
	void 			*storeData;

	/* Callback specified by user, which will be called 
	 * with appropriate return values, to specify what 
	 * happenened, and this handle as the second argument.
	 * the connEntry, and stored Data can be accessed
	 * using the two macros defined below.
	 */
	int			(*userCB)(int, void *);

	/* The following call backs are for the application,
	 * ideally belonging to a handle which contains the
	 * connhandle, as they are called by the userCB
	 * callback.
	 */
	connAppCB		appCB[APP_CONNMAXCBS];

	/* App timer handle */
	TimerPrivate		*appTimers;

	/* What fds would we add/remove from */
	NetFds			*netfds;
} ConnHandle;

#define CONN_GetData(handle) 	((handle)->storeData)
#define CONN_FreeHandle(handle)	(free(handle))

#define CONN_ConnEntry(handle) 	((handle)->connEntry)
#define CONN_SrvrEntry(handle)	((SrvrEntry *)((handle)->connEntry))
#define CONN_ConnFd(handle)	((handle)->fd)
#define CONN_ConnTimer(handle)	((handle)->timer)
#define CONN_RefCount(handle)	((handle)->refCount)
#define CONN_NBlock(handle)	((handle)->nblock)

/* We use the following event block for the combined
 * state machine.
 */
/* Connections
 * Registration
 */

/* Basic functions */
ConnHandle *CONN_GetNewHandle(void);
int CONN_Connect(ConnHandle *handle);
int CONN_ConnInProgress(int sock, FD_MODE rw, void *data);

/* Related to registration... */

/* Totally user specific */
int USER_ConnTimeout(struct Timer *t);

/* callback called by CONN_* functions  - as useCB */
int USER_ConnectResponse(int rc, void *handle);

#endif /* _connapi_h_ */
