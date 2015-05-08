#ifndef _SCM_H_
#define _SCM_H_

#define SCM_HEARTBEAT_PERIOD	5
#define SCM_DEADWAIT_PERIOD		5
#define SCM_IDLE_PERIOD			5

#define SCM_IsBackup()			IServerIsSecondary()
#define SCM_Mode()				(IServerIsSecondary()?"BKUP ":"MASTER ")

// Callback functions
void SCM_CallUpdateRx(CallHandle *c);
void SCM_CallDeleteRx(char *callid);
void SCM_HeartbeatRx(ulong nbrid, ulong peerip);

typedef enum
{
	SCM_Error = -1,
	SCM_Ok = 0,
	SCM_ErrorNoEntry,
	SCM_ErrorNoPeer,
	SCM_ErrorBadEntry,
	SCM_ErrorDeleteEntry,

} SCMError_e;

typedef struct scm_queue_item
{
	struct scm_queue_item *prev, *next;

	void *data;
	int (*repfn)();	// replication function which must be called with the data

	int (*appinfprefn)();	// application inform function which is called just before replication happens
	int (*appinfpostfn)();	// application inform function which is called just after replication happens
	
	int passed;				// queue pointer is past this entry
	int refCount;			// count of threads referencing this instance
	int delete;				// this entry has to be deleted
} SCM_QueueItem;

typedef struct {
	
	// replicator queue
	cache_t	queue;
	
	// pointer which tracks the point upto which replication is done
	SCM_QueueItem	*scmPtr;

	// lock for protecting this whole data structure
	// Both caches are pointing to this lock
	Lock	scmLock;

	// callback function which informs the application when an element is inserted
	// into this queue
	int 	(*appinfqins)(SCM_QueueItem *);
	int 	(*appinfqdel)(void *);

} SCM_StateQueue_t;

#define SCM_SetQInsAppCb(x)		scmQueue.appinfqins = x
#define SCM_SetQDelAppCb(x)		scmQueue.appinfqdel = x

extern SCM_StateQueue_t scmQueue;
void * SCM_GetEntry();

#define SCM_HasProcessed(elt)	(elt->passed)

extern cvtuple 		*scmcvp;


//
//	Function		:
//		incr_timespec()
//
//	Arguments		:
//		now				pointer to a timespec to be incremented.
//
//		seconds			# of seconds to increment timespec
//
//		nanoseconds		# of nanoseconds to increment timespec
//
//	Purpose			:
//		Make adjustments to input timespec given nanoseconds
//		to increment.
//
//	Description		:
//		This function is given a pointer to a timespec 
//		structure to be incremented by an input number
//		of nanoseconds. It correctly adjusts the timespec,
//		adding the number of nanoseconds specified correctly.
//
//	Return value	:
//		None
//
static inline void
incr_timespec( struct timespec * now, uint32_t seconds, uint32_t nanoseconds )
{
	uint32_t result;

	now->tv_sec += seconds;

	//
	// Check for greater than or equal to more than
	// a seconds worth of nanoseconds in input
	//

	if ( nanoseconds > 1000000000 )
	{
		//trc_error( 	MMG_THRD,
		//			"THRD : incr_timespec(): invalid value for nanoseconds (%u)\n",
		//			nanoseconds );
		abort();
	}

	// increment by nanoseconds

	if ( ( result = ((uint32_t) now->tv_nsec + nanoseconds) ) > (uint32_t) 999999999 )
	{
		now->tv_sec++;
		now->tv_nsec = result - 1000000000;
	}
	else
		now->tv_nsec = result;
}


extern int SCM_Init (void);
extern int SCM_PeerReset (void);
extern int SCM_PeerConnect (void);
extern int SCM_HeartbeatTx (void);
extern int SCM_BackupExists (void);
extern int SCM_SetBackup (ulong nbrid, ulong peerip);
extern int SCM_CheckBackup (long unsigned int *addr);
extern void SCM_NowBackup (void);
extern void SCM_NowPrimary (void);

extern void SCM_QueueInit (void);
extern int SCM_EnQueue(void *data, int (*repfn)(), int (*appinfprefn)(), int (*appinfpostfn)());
extern int SCM_QueueReset (void);

extern int SCM_Push (void);
extern int SCM_DeleteEntry (SCM_QueueItem *ptr);

extern int SCM_ReplicatorDoNextReplication (void);
extern int SCM_Signal (void);
extern int SCM_Wait (void);
extern int SCM_ApplicationInit (void);

#endif /* _SCM_H_ */
