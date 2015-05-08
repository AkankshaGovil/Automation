/////////////////////////////////////////////////////////////////////
//
//	Name:
//		threads.h
//
//	Description:
//			This header file contains common definitions of
//		structures used for thread control in the media gateway
//		process (mg) on the iEdge 1000. Access to threads and
//		resources within within the mg process are controlled via
//		control variables. The definition of a control variable
//		used in the mg process is defined below as a cv_t
//		structure. The availability of a full pthread library
//		capability is assumed in the implemention of these
//		functions.
//			The static inline functions defined in this header
//		file are used to use and manipulate cv_t structures
//		defined though out the mg process.
//
/////////////////////////////////////////////////////////////////////

#ifndef _THREADS_H_
#define	_THREADS_H_

#include <pthread.h>


/* static functions */
static inline void millisleep( uint32_t milliseconds );

#define MAX_TEMPORARY_THREADS	32
#define MAX_PERMANANT_THREADS	32
#define MAX_DESCR_SIZE			256


//
//	Media Gateway Control variable structure.
//

typedef struct _cv
{
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	int					value;
} cv_t;

//
//	Thread Argument data structure
//

typedef struct _thread_args
{
	cv_t				thread_state_cv;				// cv to syncronize interaction
														// between thread_create() and
														// thread_register() during
														// thread creation.
	pthread_mutexattr_t	mattr;
	pthread_condattr_t	cattr;

	void*				input_args;
	char				description[MAX_DESCR_SIZE];

	int32_t				transient;						// Flag indicating a temporary
														//	thread.

	int32_t				rt_thread;						// Flag indicating a rt thread.
} thread_args_t;

//
//	Media Gateway Thread Descriptor structure.
//

typedef struct thread_descr
{
	pthread_t		thread;							// pthread thread descriptor
	void*			retval;							// return code
	cv_t			cv;								// control variable indicating state
	char			description[MAX_DESCR_SIZE];	// string identifier passed by
													// 	caller of thread_create()
	int32_t			transient;						// Flag indicating a temporary
													//	thread.
	int32_t			rt_thread;						// Flag indicating a rt thread.
} thread_descr_t; 

extern	thread_descr_t	temporary_threads[MAX_TEMPORARY_THREADS];
extern	thread_descr_t	permanant_threads[MAX_PERMANANT_THREADS];
extern	cv_t			init_thread_cv;

//
// condition variable states for threads
//

#define THRD_INIT		1
#define THRD_DONE		2
#define THRD_ABORT		4
#define THRD_RUNNING	8
#define THRD_READY		16
#define THRD_STARTED	32
#define THRD_CONT		64
#define THRD_TIMEOUT	128
#define THRD_NEVER		256
#define THRD_FALSE		512
#define THRD_TRUE		1024

#define CMD_INIT		1
#define CMD_RUNNING		2
#define CMD_COMPLETE	4
#define CMD_FAILED		8
#define QRY_RUNNING		16
#define QRY_COMPLETE	32

// Forward declaration of functions

static inline void lock_mutex( pthread_mutex_t * lock );
static inline void unlock_mutex( pthread_mutex_t * lock );

//
//	Function		:
//		cv_signal()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//		newval			new value to be signaled for the
//						input condition variable.
//
//	Description		:
//		Change condition variable to new value and signal
//		that it has changed to other threads interested in
//		this condition variable. Before any changes are made
//		the mutex in the condition variable is locked to 
//		serialize access to the condition variable amongst
//		competing threads.
//
//	Return value	:
//		None		on success
//		Abort		process on any failure - no bugs allowed
//
static inline void
cv_signal( cv_t *ptr, int32_t newval )
{
	int32_t status;

	lock_mutex( &(ptr->mutex) );

	ptr->value = newval;

	if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
	{
		NETERROR(MISPD, ("THRD : cond signal error - %s\n", strerror(status)));
		abort();
	}

	unlock_mutex( &(ptr->mutex) );
}

//
//	Function		:
//		cv_wait()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//		tstval			value to wait for on target
//						condition variable specified.
//						If states for a condition
//						variable are specified as
//						unique bits this argument
//						may be an or'ed boolean.
//
//	Description		:
//		Waits for a condition variable to enter one
//		of the states specified by tstval. Before
//		waiting the mutex in the condition variable
//		is locked to serialize access to the
//		condition variable amongst competing threads.
//		The call to pthread_cond_wait() unlocks the
//		mutex while the calling thread waits. This
//		enables other threads to access the condition
//		variable to change its state.
//
//	Return value	:
//		newstate	on success this function returns the
//						new state into which the condition
//						variable has entered.
//
//		Abort		process on any failure - no bugs allowed
//
static inline int32_t
cv_wait( cv_t *ptr, int32_t tstval )
{
	int32_t status;
	int32_t retval;

	lock_mutex( &(ptr->mutex) );

	while ( !(ptr->value & tstval) )
		if ( (status = pthread_cond_wait( &(ptr->cond), &(ptr->mutex) )) != 0 )
		{
			NETERROR(MISPD, ("THRD : cond wait error - %s", strerror(status)));
			abort();
		}

	retval = ptr->value;

	unlock_mutex( &(ptr->mutex) );

	return(retval);
}

//
//	Function		:
//		cv_wait_timed()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//		tstval			value to wait for on target
//						condition variable specified.
//						If states for a condition
//						variable are specified as
//						unique bits this argument
//						may be an or'ed boolean.
//
//		abstime			a timespec specifying in
//						absolute time when the
//						wait should expire.
//
//		timeout			pointer to integer which will
//						contain zero on return if
//						condition was met within time.
//						Otherwise 1 indicating timeout.
//
//	Description		:
//		Waits for a condition variable to enter one
//		of the states specified by tstval or for the
//		timeout interval to expire. Before waiting 
//		the mutex in the condition variable	is 
//		locked to serialize access to the condition
//		variable amongst competing threads.
//		The call to pthread_cond_timedwait() unlocks the
//		mutex while the calling thread waits. This
//		enables other threads to access the condition
//		variable to change its state.
//
//	NOTE:
//		use incr_timespec() function defined above to
//		increment the abstime timespec correctly prior 
//		to calling this function.
//
//	Return value	:
//		newstate	on success this function returns the
//						new state into which the condition
//						variable has entered or the value
//						THRD_TIMEOUT if the timeout has
//						expired.
//
//		Abort		process on any failure - no bugs allowed
//
static inline int32_t
cv_wait_timed( cv_t *ptr, int32_t tstval, struct timespec* abstime, int32_t *timeout )
{
	int32_t status;
	int32_t retval;

	*timeout = 0;

	lock_mutex( &(ptr->mutex) );

	while ( !(ptr->value & tstval) )
	{
		status = pthread_cond_timedwait( &(ptr->cond), &(ptr->mutex), abstime );

		if (!status)
			break;

		if ( status == ETIMEDOUT )	// condition change signalled - go checkit
		{
			*timeout = 1;
			break;
		}

		NETERROR(MISPD, ("THRD : cond wait timed error - %s\n", strerror(status)));
		abort();
	}

	retval = ptr->value;

	unlock_mutex( &(ptr->mutex) );

	return(retval);
}

//
//	Function		:
//		cv_wait_nonzero()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//	Description		:
//		Waits for a condition variable value to
//		be set to a non-zero value. This function
//		is used by the mg_q_recv() routine to block
//		receiving threads while they wait for entries
//		to be added to an empty queue.
//
//	Return value	:
//		None		on success
//
//		Abort		process on any failure - no bugs allowed
//
static inline void
cv_wait_nonzero( cv_t *ptr )
{
	int32_t status;

	lock_mutex( &(ptr->mutex) );

	while ( ptr->value == 0 )
	{
		if ( (status = pthread_cond_wait( &(ptr->cond), &(ptr->mutex) )) != 0 )
		{
			NETERROR(MISPD, ("THRD : cond wait error - %s\n", strerror(status)));
			abort();
		}
	}

	unlock_mutex( &(ptr->mutex) );
}

//
//	Function		:
//		cv_wait_lessthan()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//		value			value to wait for on target
//						condition variable specified.
//
//	Description		:
//		Waits for a condition variable value to
//		be less than the value specified. This function
//		is used by the mg_q_send() routine to block
//		writing threads when the queue reaches the high
//		water mark of entries in the queue.
//
//	Return value	:
//		None		on success
//
//		Abort		process on any failure - no bugs tolarated here.
//
static inline void
cv_wait_lessthan( cv_t *ptr, int32_t value )
{
	int32_t status;

	lock_mutex( &(ptr->mutex) );

	while ( ptr->value >= value )
		if ( (status = pthread_cond_wait( &(ptr->cond), &(ptr->mutex) )) != 0 )
		{
			NETERROR(MISPD, ("THRD : cond wait error - %s\n", strerror(status) ));
			abort();
		}

	unlock_mutex( &(ptr->mutex) );
}

//
//	Function		:
//		cv_get()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//	Description		:
//		Gets the current value of the specified condition
//		variable.
//
//	Return value	:
//		value		on success returns the value to which
//					the condition variable is set.
//
//		Abort		process on any failure - no bugs tolarated here.
//
static inline int32_t
cv_get( cv_t *ptr )
{
	int32_t retval;

	lock_mutex( &(ptr->mutex) );

	retval = ptr->value;

	unlock_mutex( &(ptr->mutex) );

	return(retval);
}

//
//	Function		:
//		cv_test_and_set()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//		tstval			value to test for on
//						condition variable specified.
//
//		neval			new value to which the condition
//						variable should be set if tstval
//						is true
//
//	Description		:
//		This function provides atomic access to testing and
//		setting condition variable values shared amongst 
//		competing threads
//
//	Return value	:
//		0			if tstval did not match and new value was
//					not set
//
//		1			if tstval did matched and new value was
//					set
//
//		Abort		process on any failure - no bugs tolarated here.
//
static inline int32_t
cv_test_and_set( cv_t *ptr, int32_t tstval, int32_t newval )
{
	int32_t status;
	int32_t retval;

	lock_mutex( &(ptr->mutex) );

	if ( (ptr->value & tstval) )
	{
		ptr->value = newval;

		if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
		{
			NETERROR(MISPD, ("THRD : cv_test_and_set error -  %s\n",strerror(status)));
			abort();
		}

		retval = 1;
	}
	else
		retval = 0;

	unlock_mutex( &(ptr->mutex) );

	return(retval);
}

//
//	Function		:
//		cv_increment()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//	Description		:
//		This function atomically increments the value of
//		the specified condition variable. The routine
//		is used by mg_q_write() to add queue entries to
//		a target queue.
//
//	Return value	:
//		None		on success
//
//		Abort		process on any failure - no bugs tolarated here.
//
static inline void
cv_increment( cv_t *ptr )
{ 
	int32_t status;

	lock_mutex( &(ptr->mutex) );


	ptr->value++;

	if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
	{
		NETERROR(MISPD, ("THRD : cond signal error - %s\n", strerror(status) ));
		abort();
	}

	unlock_mutex( &(ptr->mutex) );
}

//
//	Function		:
//		cv_decrement()
//
//	Arguments		:
//		ptr				pointer to a cv_t structure
//
//	Description		:
//		This function atomically decrements the value of
//		the specified condition variable. The routine
//		is used by mg_q_read() to remove queue entries
//		from a target queue.
//
//	Return value	:
//		None		on success
//
//		Abort		process on any failure - no bugs tolarated here.
//
static inline void
cv_decrement( cv_t *ptr )
{
	int32_t status;

	lock_mutex( &(ptr->mutex) );

	ptr->value--;

    if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
    {
		NETERROR(MISPD, ("THRD : cond signal error - %s\n", strerror(status)));
		abort();
	}

	unlock_mutex( &(ptr->mutex) );
}

//
//	Function		:
//		thread_create()
//
//	Arguments		:
//
//		thread_handle		pointer to a pthread_t in which
//							this routine stores the pthread_t
//							given to it when pthread_create()
//							is called
//
//		start_routine		function pointer to routine to
//							be started as a thread.
//
//		thread_arg			void * representing the argument
//							to be passed to the routine by
//							pthread_create()
//
//		description			descriptive string to be 
//							associated with the started 
//							thread. 
//
//		temporary			integer flag indicating whether thread
//							is a temporary thread as opposed to one
//							that will exist for the lifetime of the
//							mgd process.
//
//	Description		:
//		This function is used in lew of pthread_create()
//		to allow us to monitor the threads in a process.
//		It keeps track of the threads started in a global
//		thread table and associates a descriptive string
//		identifier with each thread. Threads are identified
//		by their thread id.
//
//	Return value	:
//		None
//
static inline void
thread_create(	pthread_t *thread_handle, void* (*start_routine)(void*),
				void *thread_arg, char *description, int32_t temporary )
{
	thread_args_t	args;			// Argument structure to be passed to new thread
	int32_t			status;
	pthread_attr_t	thread_attr;

	strcpy( args.description, description );

	// Initialize cv_t used to syncronize interaction between
	// thread_create() and thread_register() routines.
	// thread_create() will not exit until thread_register()
	// signals it to using a cv_t in args.

	pthread_mutexattr_init( &args.mattr );
	pthread_condattr_init( &args.cattr );
	pthread_mutex_init( &args.thread_state_cv.mutex, &args.mattr );
	pthread_cond_init( &args.thread_state_cv.cond, &args.cattr );
	args.thread_state_cv.value = THRD_INIT;


	// Fill in arguments to be passed to new thread

	args.transient = temporary;
	args.input_args = thread_arg;
	args.rt_thread = 0;

	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
	pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_SYSTEM );
	pthread_attr_setguardsize( &thread_attr, (size_t) (2*getpagesize()) );
	pthread_attr_setstacksize( &thread_attr, 256*1024 );

    if ( (status = pthread_create(	thread_handle, &thread_attr,
									start_routine, (void*) &args ) ) != 0 )
	{
		NETERROR(MISPD, ("THRD : create  - failed to start thead : %s - status = %d\n",
					description, status ));
		abort();
	}

	// Wait for started child thread to extract information
	// from thread_args_t args in thread_register()
	// before continuing since it is allocated from
	// the stack frame for this routine.

	cv_wait( &args.thread_state_cv, THRD_RUNNING );


	// Destroy data structures allocated to syncronize interaction
	// between thread_create() and thread_register() routines

	pthread_mutex_destroy( &args.thread_state_cv.mutex );
	pthread_cond_destroy( &args.thread_state_cv.cond );

	pthread_mutexattr_destroy( &args.mattr );
	pthread_condattr_destroy( &args.cattr );

	// Destroy pthread_attr allocated by pthread_attr_init()

	pthread_attr_destroy( &thread_attr );
	
	return;
}

//
//	Function		:
//		thread_exit()
//
//	Arguments		:
//		None
//		
//
//	Description		:
//		This function is used in lew of pthread_exit()
//		to allow us to monitor the exiting of threads
//		in a process. The thread id of the calling thread
//		is used to identify the exiting thread based
//		on information stored by the thread_create()
//		routine.
//
//	Return value	:
//		void*		in reality the routine never returns
//					to the caller because the thread exits.
//
static inline void
thread_exit( int32_t temporary )
{
	uint32_t	thread 	= pthread_self();

	if ( temporary )
	{
		uint32_t	mask	= (MAX_TEMPORARY_THREADS - 1);
		uint32_t	index	= (thread & mask);

		NETDEBUG(MISPD, NETLOG_DEBUG2,
					("THRD : exiting temp %2s thread : %s\n",
					((temporary_threads[index].rt_thread != 0 )? "RT" : "TS" ),
					temporary_threads[index].description ));

		memset( &temporary_threads[index], (int32_t) 0, sizeof( thread_descr_t ) );

		// Reinit thread control variable

		temporary_threads[index].cv = init_thread_cv;

		pthread_exit( (void*) NULL );
	}
	else
	{
		NETDEBUG(MISPD, NETLOG_DEBUG2,
					("THRD : exiting perm %2s thread : %s\n",
					((permanant_threads[thread].rt_thread != 0 )? "RT" : "TS" ),
					permanant_threads[thread].description));

		memset( &permanant_threads[thread], (int32_t) 0, sizeof( thread_descr_t ) );

		// Reinit thread control variable

		permanant_threads[thread].cv = init_thread_cv;

		pthread_exit( (void*) NULL );
	}

	return;
}

//
//	Function		:
//		thread_register()
//
//	Arguments		:
//		thread_args		a pointer to the thread_args_t structure for
//						the thread being started.
//		
//
//	Description		:
//		This function is the first thing called by a
//		started thread. The routine makes sure the
//		thread has been recorded by the spawner of the
//		thread prior to continuing.
//
//	Return value	:
//		None
//
static inline thread_descr_t*
thread_register( thread_args_t *thread_args )
{
	uint32_t	thread = pthread_self();

	if ( thread_args->transient )
	{
		uint32_t	mask	= (MAX_TEMPORARY_THREADS - 1);
		uint32_t	index	= (thread & mask);

		strcpy( temporary_threads[index].description, thread_args->description );
		temporary_threads[index].rt_thread = thread_args->rt_thread;
		temporary_threads[index].transient = thread_args->transient;

		NETDEBUG(MISPD, NETLOG_DEBUG2,
					("THRD : create  temp %2s thread : %s\n",
					((thread_args->rt_thread != 0 )? "RT" : "TS" ),
					temporary_threads[index].description));

		cv_signal( &temporary_threads[index].cv, THRD_RUNNING );

		// signal parent thread in thread_create() that thread
		// info has been recorded and thread_create() can exit,
		// releasing thread_args_t args on its stack frame.

		cv_signal( &thread_args->thread_state_cv, THRD_RUNNING );

		// Return pointer to thread descriptor used by thread

		return( &temporary_threads[index] );
	}
	else
	{
		strcpy( permanant_threads[thread].description, thread_args->description );
		permanant_threads[thread].rt_thread = thread_args->rt_thread;
		permanant_threads[thread].transient = thread_args->transient;

		NETDEBUG(MISPD, NETLOG_DEBUG2,
					("THRD : create  perm %2s thread : %s\n",
					((thread_args->rt_thread != 0 )? "RT" : "TS" ),
					permanant_threads[thread].description));

		cv_signal( &permanant_threads[thread].cv, THRD_RUNNING );

		// signal parent thread in thread_create() that thread
		// info has been recorded and thread_create() can exit,
		// releasing thread_args_t args on its stack frame.

		cv_signal( &thread_args->thread_state_cv, THRD_RUNNING );

		// Return pointer to thread descriptor used by thread

		return( &permanant_threads[thread] );
	}
}

//
//	Function		:
//		threads_init()
//
//	Arguments		:
//		None
//		
//
//	Description		:
//		This function initializes the control variables
//		in the global thread table to a known state.
//
//	Return value	:
//		None
//
static inline void
threads_init( void )
{
	int32_t	i;

	for ( i = 0; i < MAX_TEMPORARY_THREADS; i++ )
		temporary_threads[ i ].cv = init_thread_cv;

	for ( i = 0; i < MAX_PERMANANT_THREADS; i++ )
		permanant_threads[ i ].cv = init_thread_cv;

}

//
//	Function		:
//		lock_mutex()
//
//	Arguments		:
//		lock			a pointer to an initialized mutex
//						to lock.
//
//	Description		:
//		Locks a mutex for a thread.
//
//	Return value	:
//		None
//
static inline void
lock_mutex( pthread_mutex_t * lock )
{
	int	status;

	for (;;)
	{
		if ( (status = pthread_mutex_trylock( lock )) == 0 )
			break;
		else 
		if ( status != EBUSY )
		{
			NETERROR(MISPD, ("THRD : mutex lock error - %s\n",
						strerror(status)));
			abort();
		}

		millisleep( 500 );
	}
}

//
//	Function		:
//		unlock_mutex()
//
//	Arguments		:
//		lock			a pointer to an initialized mutex
//						which is locked.
//
//	Description		:
//		Unlocks a locked mutex for a thread.
//
//	Return value	:
//		None
//
static inline void
unlock_mutex( pthread_mutex_t * lock )
{
	int	status;

	if ( ( status = pthread_mutex_unlock( lock ) ) != 0 )
	{
		NETERROR(MISPD, ("THRD : mutex unlock error - %s\n",
					strerror(status)));
		abort();
	}
}
//
//	Function		:
//		millisleep()
//
//	Arguments		:
//
//		milliseconds	# of milliseconds to sleep
//
//	Purpose			:
//		Use nanosleep() to sleep for a number of 
//		milliseconds.
//
//	Description		:
//		This function is given an integer containing
//		the number of milliseconds to sleep. 
//
//	Return value	:
//		None
//
static inline void
millisleep( uint32_t milliseconds )
{
	struct	timespec	delay;
	struct	timespec	remaining;
	int32_t				retval;
	extern	int			errno;

	if ( milliseconds )
	{
		delay.tv_sec = milliseconds/1000;
		delay.tv_nsec = (milliseconds%1000)*1000000;
	}
	else
		return;


	// Increment timespec_t by # of milliseconds

	while ( (retval = nanosleep( &delay, &remaining )) )
	{
		if ( errno == EINTR )
		{
			delay = remaining;
			continue;
		}

		if ( errno == EINVAL )
		{
			//trc_error(	MMG_THRD, 
			//			"THRD : incr_timespec(): errno = EINVAL\n" );
			break;
		}
	}
}

#endif /* _THREADS_H_ */
