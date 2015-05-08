
#ifndef _queue_P_h_
#define _queue_P_h_


#include "queue.h"

typedef struct _queue_
{
	int	size;
	int	depth;
	unsigned char * begin;
	unsigned char * end;
	void	* items;
	int	nitems;
} Queue;


#endif
