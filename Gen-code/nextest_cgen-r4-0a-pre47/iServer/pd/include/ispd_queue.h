#ifndef _ispd_queue_h_
#define _ispd_queue_h_

//	"$Id: ispd_queue.h,v 1.6 2002/03/15 19:03:46 sturt Exp $"

// 
//	Exported functions
//

int32_t ispd_q_create( int32_t element_size, int32_t total_elements );
int32_t ispd_q_destroy( int32_t msgQid );
int32_t ispd_q_send( int32_t msgQid, void *buffer );
int32_t ispd_q_recv( int32_t msgQid, void *buffer );

#endif // _ispd_queue_h_
