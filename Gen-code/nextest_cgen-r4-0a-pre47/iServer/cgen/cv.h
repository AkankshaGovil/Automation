/////////////////////////////////////////////////////////////////////
//
//	Name:
//		cv.h
//
//	Description:
//			The static inline functions defined in this header
//		file are used to use and manipulate cv_t structures.
//
/////////////////////////////////////////////////////////////////////

#ifndef _CV_H_
#define	_CV_H_

//	"$Id: cv.h,v 1.1 2001/09/13 22:48:34 sturt Exp $"

typedef struct _cv
{
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	int					value;
} cv_t;

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

//
//  Function        :
//      incr_timespec()
//
//  Arguments       :
//      now             pointer to a timespec to be incremented.
//
//      seconds         # of seconds to increment timespec
//
//      nanoseconds     # of nanoseconds to increment timespec
//
//  Purpose         :
//      Make adjustments to input timespec given nanoseconds
//      to increment.
//
//  Description     :
//      This function is given a pointer to a timespec
//      structure to be incremented by an input number
//      of nanoseconds. It correctly adjusts the timespec,
//      adding the number of nanoseconds specified correctly.
//
//  Return value    :
//      None
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
        fprintf(  stderr,
					"THRD : incr_timespec(): invalid value for nanoseconds (%u)\n",
					nanoseconds );
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
			fprintf( stderr, "THRD : incr_timespec(): errno = EINVAL\n" );
			break;
		}
	}
}

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

	if ( (status = pthread_mutex_lock( &(ptr->mutex))) != 0 )
	{
		fprintf( 	stderr,
					"THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	ptr->value = newval;

	if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
	{
		fprintf( 	stderr,
					"THRD : cond signal error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
		fprintf( 	stderr,
					"THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
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

	if ( (status = pthread_mutex_lock( &(ptr->mutex) )) != 0 )
	{
		fprintf( 	stderr,
					"THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	while ( !(ptr->value & tstval) )
		if ( (status = pthread_cond_wait( &(ptr->cond), &(ptr->mutex) )) != 0 )
		{
			fprintf( 	stderr,
						"THRD : cond wait error - \"%s\":%d: %s\n",
						__FILE__, __LINE__, strerror(status));
			abort();
		}

	retval = ptr->value;

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
		fprintf( 	stderr,
					"THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
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

	if ( (status = pthread_mutex_lock( &(ptr->mutex) )) != 0 )
	{
		fprintf( 	stderr,
					"THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

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

		fprintf( 	stderr,
					"THRD : cond wait timed error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	retval = ptr->value;

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
		fprintf( 	stderr,
					"THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
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

	if ( (status = pthread_mutex_lock( &(ptr->mutex) )) != 0 )
	{
		fprintf( stderr, "THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	while ( ptr->value == 0 )
	{
		if ( (status = pthread_cond_wait( &(ptr->cond), &(ptr->mutex) )) != 0 )
		{
			fprintf( stderr, "THRD : cond wait error - \"%s\":%d: %s\n",
						__FILE__, __LINE__, strerror(status));
			abort();
		}
	}

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
		fprintf( stderr, "THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
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

	if ( (status = pthread_mutex_lock( &(ptr->mutex) )) != 0 )
	{
		fprintf( stderr, "THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	while ( ptr->value >= value )
		if ( (status = pthread_cond_wait( &(ptr->cond), &(ptr->mutex) )) != 0 )
		{
			fprintf( stderr, "THRD : cond wait error - \"%s\":%d: %s\n",
						__FILE__, __LINE__, strerror(status));
			abort();
		}

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
		fprintf( stderr, "THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
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
	int32_t status;
	int32_t retval;

	if ( (status = pthread_mutex_lock( &(ptr->mutex) )) != 0 )
	{
		fprintf( stderr, "THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	retval = ptr->value;

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
		fprintf( stderr, "THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
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

	if ( (status = pthread_mutex_lock( &(ptr->mutex) )) != 0 )
	{
			fprintf( stderr, "THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	if ( (ptr->value & tstval) )
	{
		ptr->value = newval;

		if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
		{
				fprintf( stderr, "THRD : cv_test_and_set error - \"%s\":%d: %s\n",
						__FILE__, __LINE__, strerror(status));
			abort();
		}

		retval = 1;
	}
	else
		retval = 0;

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
			fprintf( stderr, "THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

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

	if ( (status = pthread_mutex_lock( &(ptr->mutex))) != 0 )
	{
		fprintf( stderr, "THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	ptr->value++;

	if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
	{
		fprintf(stderr, "THRD : cond signal error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
		fprintf(stderr, "THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
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

	if ( (status = pthread_mutex_lock( &(ptr->mutex))) != 0 )
	{
			fprintf(stderr, "THRD : mutex lock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	ptr->value--;

	if ( (status = pthread_cond_broadcast( &(ptr->cond) )) != 0 )
	{
			fprintf(stderr, "THRD : cond signal error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}

	if ( (status = pthread_mutex_unlock( &(ptr->mutex) )) != 0 )
	{
			fprintf(stderr, "THRD : mutex unlock error - \"%s\":%d: %s\n",
					__FILE__, __LINE__, strerror(status));
		abort();
	}
}

#endif /* _CV_H_ */
