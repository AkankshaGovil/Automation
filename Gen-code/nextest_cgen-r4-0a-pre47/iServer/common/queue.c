

#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "queueP.h"
#include <malloc.h>


int
qInit (Q * q, int size, int depth)
{
	Queue  * pq;

	pq = (Queue *) malloc (sizeof(Queue));

	if ( ! pq )
		return -1;

	pq->size   = size;
	pq->depth  = depth;
	pq->nitems = 0;

	/* Alloc space for the items */
	pq->items = (void *) malloc (size * depth);

	if (! pq->items)
	{
		/* No memory */
		return -1;
	}
	pq->begin = pq->end = pq->items;

	*q = pq;

	return 0;
}

int
qDestroy (Q q)
{
	Queue * pq = q;

	free (pq->items);

	free (pq);

	return 0;
}

int
qAdd (Q q, void * item)
{
	Queue * pq = q;

	/* Invalid q */
	if (! pq)
		return -1;

	/* Out of space */
	if (pq->nitems >= pq->depth)
		return -1;

	/* Add to end */
	memcpy (pq->end, item, pq->size);

	/* Move end */
	pq->end += pq->size;

	/* Increment */
	pq->nitems += 1;

	/* Return number of items */
	return (pq->nitems);
}


int
qRemove (Q q, void ** item)
{
	Queue * pq = q;

	if (pq->begin >= pq->end)
	{
		/* Queue overflow */
		return -1;
	}

	memcpy (*item, pq->begin, pq->size);

	pq->begin += pq->size; 

	/* Decrement number of items */
	pq->nitems -= 1;

	if (pq->begin == pq->end)
	{
		/* Reset */
		pq->begin = pq->end = pq->items;
	}

	return 0;
}


int
qStatus (Q q)
{
	return 0;
}

int
qItems (Q q)
{
	Queue * pq = q;

	return (pq->nitems);
}
