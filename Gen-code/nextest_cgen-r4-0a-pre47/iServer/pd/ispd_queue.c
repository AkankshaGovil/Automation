static char const rcsid[] = "$Id: ispd_queue.c,v 1.6 2002/03/15 19:03:46 sturt Exp $";

//
//	This file contains the queue management routines used by
//	the ispd (iServer Peering Daemon).	The routines are pthreads
//	specific so they will only be useful in the Unix environment.
//	They could be easily modified for another environment.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


#include <ispd.h>

//
//	exported functions
//

int32_t	mg_q_create( int32_t element_size, int32_t total_elements );
int32_t mg_q_destroy( int32_t msgQid );
int32_t	mg_q_send( int32_t msgQid, void *buffer );
int32_t	mg_q_recv( int32_t msgQid, void *buffer );

//
//	Statics 
//

#define	TOTAL_MG_QUEUES		4

typedef	struct _ispd_queue
{
	void*			msg_first;			// pointer to first element on queue
	void*			msg_last;			// pointer to last element on queue

	void*			msg_wptr;			// pointer to where the next message
										//	sent via ispd_q_send should be put

	pthread_mutex_t	msg_wptr_mutex;		// mutex to prevent writer race

	void*			msg_rptr;			// pointer to where the next message
										//	received via ispd_q_recv() should
										//	be gotten.

	pthread_mutex_t	msg_rptr_mutex;		// mutex to prevent reader race

	int32_t			element_size;		// size of each element on the queue
	int32_t			total_elements;		// total elements allocated for the queue

	cv_t			available_elements;	// count of available elements

	int32_t			valid;				// flag which is set for valid
										// message queues
}	ispd_queue_t;

//
//	Mutex to lock all local queues. Used during ispd_q_create()
//	and ispd_q_destroy()
//

pthread_mutex_t		all_queues_mutex = PTHREAD_MUTEX_INITIALIZER;

int32_t				total_queues;
ispd_queue_t		ispd_queues[TOTAL_MG_QUEUES];

static	cv_t		init_queue_cv =
	{	PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0 };

//
//	Function    :
//		ispd_q_create()
//
//	Arguments	:
//		element_size	size of a queue element in the queue
//
//		total_elements	total number of elements to allocate for
//						the queue
//
//	Description :
//			This subroutine creates and initializes a queue of
//		elements of the	specified size given the input arguments
//		and returns the queue's id to the caller.
//
//	Return value:
//
//		value >= 0 if successful. This value represents the
//			queue id used to reference a particular queue from
//			ispd_q_send() or ispd_q_recv().
//		value < 0 on failure
//	
//
int32_t
ispd_q_create( int32_t element_size, int32_t total_elements )
{
	int32_t	new_queue;

	// are either element_size or total_elements 0 ?

	if ( !element_size || !total_elements )
		return(-1);

	// Lock all_queue_mutex while in here

	lock_mutex( &all_queues_mutex );

	// Any queue slots left to be allocated? 

	if ( total_queues == TOTAL_MG_QUEUES )
	{
		// no, return failure indicator

		unlock_mutex( &all_queues_mutex );
		return(-1);
	}

	memset( &ispd_queues[total_queues], (int32_t) 0, sizeof( ispd_queue_t ));

	if ( ( ispd_queues[total_queues].msg_first = 
			malloc( (size_t) ( element_size * total_elements ) ) ) == (void*) NULL )
	{
		unlock_mutex( &all_queues_mutex );
		return(-1);
	}

	ispd_queues[total_queues].total_elements = total_elements;
	ispd_queues[total_queues].element_size = element_size;

	ispd_queues[total_queues].msg_last = 
		(void*) 	( (uint32_t) ispd_queues[total_queues].msg_first +
					( element_size * (total_elements - 1)) );

	ispd_queues[total_queues].msg_wptr = ispd_queues[total_queues].msg_first;
	ispd_queues[total_queues].msg_rptr = ispd_queues[total_queues].msg_first;

	pthread_mutex_init( &ispd_queues[total_queues].msg_wptr_mutex, NULL );
	pthread_mutex_init( &ispd_queues[total_queues].msg_rptr_mutex, NULL );

	new_queue = total_queues;
	ispd_queues[total_queues].available_elements = init_queue_cv;
	ispd_queues[total_queues].valid = 1;
	total_queues++;

	unlock_mutex( &all_queues_mutex );

	return(new_queue);
}

//
//	Function    :
//		ispd_q_destroy()
//
//	Arguments	:
//		msgQid		message queue identifier which identifies
//					the queue to be destroyed.
//
//	Description :
//			This subroutine destroys a message queue that has
//		been setup. It is meant for usage during shutdown 
//		processing for the application using this queue
//		implementation.
//
//	Return value:
//
//		value >= 0 if successful.
//		value < 0 on failure
//
int32_t
ispd_q_destroy( int32_t msgQid )
{
	if ( msgQid > (TOTAL_MG_QUEUES - 1) || !ispd_queues[msgQid].valid )
		return(-1);

	ispd_queues[msgQid].valid = 0;

	if ( ispd_queues[msgQid].msg_first ) 
		free( ispd_queues[msgQid].msg_first );

	pthread_mutex_destroy( &ispd_queues[msgQid].msg_wptr_mutex );
	pthread_mutex_destroy( &ispd_queues[msgQid].msg_rptr_mutex );

	return(0);
}

//
//	Function    :
//		ispd_q_send()
//
//	Arguments	:
//		msgQid		message queue identifier which identifies
//					the queue to send a message to.
//
//		buffer		pointer to a buffer in which contains the
//					message to be placed on the queue.
//
//	Description :
//			This subroutine places an element pointed to by
//		buffer on the specified	queue. 
//
//	Return value:
//		
//		value = 0 on success
//
//		value < 0 on failure
//			failure is indicated if the msgQid specified has
//			not been created or if the queue is full.
//
int32_t
ispd_q_send( int32_t msgQid, void *buffer )
{

	if ( msgQid > (TOTAL_MG_QUEUES - 1) || !ispd_queues[msgQid].valid )
		return(-1);

	//
	// If the queue is full wait for it to go below
	// high water mark
	//

	cv_wait_lessthan( &ispd_queues[msgQid].available_elements,
						ispd_queues[msgQid].total_elements );

	// Lock wptr mutex for queue while incrementing the queue to
	// protect against multiple writer race

	lock_mutex( &ispd_queues[msgQid].msg_wptr_mutex );

	memcpy( ispd_queues[msgQid].msg_wptr,
			buffer,
			ispd_queues[msgQid].element_size );

	if ( ispd_queues[msgQid].msg_wptr == ispd_queues[msgQid].msg_last )
		ispd_queues[msgQid].msg_wptr = ispd_queues[msgQid].msg_first;
	else
		ispd_queues[msgQid].msg_wptr += ispd_queues[msgQid].element_size;

	//
	// Unlock wptr mutex for queue
	//

	unlock_mutex( &ispd_queues[msgQid].msg_wptr_mutex );

	cv_increment( &ispd_queues[msgQid].available_elements );

	return(0);
}

//
//	Function    :
//		ispd_q_recv()
//
//	Arguments	:
//		msgQid		message queue identifier which identifies
//					the queue to receive a message from.
//
//		buffer		pointer to a buffer in which the message
//					should be returned.
//
//	Description :
//			This subroutine receives a message from the queue
//		specified in the specified buffer. If no messages are
//		currently available the function blocks until messages
//		are sent to the queue via ispd_q_send().
//
//	Return value:
//		
//		value = 0 on success
//
//		value < 0 on failure
//			failure is indicated if the msgQid specified has
//			not been created
//
int32_t	
ispd_q_recv( int32_t msgQid, void *buffer  )
{
	if ( msgQid > (TOTAL_MG_QUEUES - 1) || !ispd_queues[msgQid].valid )
		return(-1);

	cv_wait_nonzero( &ispd_queues[msgQid].available_elements );

	// Lock rptr mutex for queue while incrementing the queue to
	// protect against multiple reader race

	lock_mutex( &ispd_queues[msgQid].msg_rptr_mutex );

	memcpy( buffer,
			ispd_queues[msgQid].msg_rptr,
			ispd_queues[msgQid].element_size );

	if ( ispd_queues[msgQid].msg_rptr == ispd_queues[msgQid].msg_last )
		ispd_queues[msgQid].msg_rptr = ispd_queues[msgQid].msg_first;
	else
		ispd_queues[msgQid].msg_rptr += ispd_queues[msgQid].element_size;

	// Unlock rptr mutex for queue

	unlock_mutex( &ispd_queues[msgQid].msg_rptr_mutex );

	cv_decrement( &ispd_queues[msgQid].available_elements );

	return(0);
}
