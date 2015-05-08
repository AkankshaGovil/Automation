/*
 * (c) Copyright 1990, 1991, 1992 Conor P. Cahill (cpcahil@virtech.vti.com)
 *
 * This software may be distributed freely as long as the following conditions
 * are met:
 *      * the distribution, or any derivative thereof, may not be
 *        included as part of a commercial product
 *      * full source code is provided including this copyright
 *      * there is no charge for the software itself (there may be
 *        a minimal charge for the copying or distribution effort)
 *      * this copyright notice is not modified or removed from any
 *        source file
 */

#ifndef lint
static char rcs_hdr[] = "$Id: leak.c,v 1.1 2002/07/04 12:53:10 agoyal Exp $";
#endif

#include <stdio.h>

#include "mallocin.h"

#ifdef MALLOC_PTHREAD
	extern pthread_mutex_t		malloc_mutex;
#endif

/*
 * Function:    malloc_inuse()
 *
 * Purpose: to determine the amount of memory in use
 *
 * Arguments:   histidptr - pointer to hist id area
 *
 * Returns: Number of bytes currently in use
 *
 * Narrative:   make sure the malloc chain is ok
 *      for each element in the malloc chain
 *          if it is in use
 *              add size to total size
 *      if hist id is wanted
 *                  set it
 *      return total in-use size
 *      
 */
unsigned long
malloc_inuse(	unsigned long *histptr )
{
	return( DBmalloc_inuse( (char *) NULL, 0, histptr) );
}

unsigned long
DBmalloc_inuse(	CONST char *	file,
				int				line,
				unsigned long *	histptr )
{
	unsigned long size = 0;
	struct mlist *ptr;

	MALLOC_INIT();

	#ifdef MALLOC_PTHREAD

		if ( !malloc_preamble )
			pthread_mutex_lock( &malloc_mutex );

	#endif

	//
	// make sure the chain is ok (otherwise we will have a problem
	// parsing through it
	//

	VOIDCAST DBFmalloc_chain_check("malloc_inuse", file, line, 1);

	// for each element in the malloc chain

	for (ptr = &malloc_start; ptr; ptr = ptr->next)
	{
		//
		// if the element is in use and it is not marked and it is
		// not a stack segment (an internal allocation used by the
		// malloc subsystem when tracking program stack)
		//

		if (((ptr->flag & M_INUSE) == M_INUSE)
			&& ((ptr->flag & M_MARKED) != M_MARKED) && (GETTYPE(ptr) != M_T_STACK))
		{
			// add its requested size into the total size

			size += ptr->r_size;
		}
	}

	// if the hist id is desired, give it to em.

	if (histptr != NULL)
	{
		*histptr = malloc_hist_id;
	}

	#ifdef MALLOC_PTHREAD

		if ( !malloc_preamble )
			pthread_mutex_unlock( &malloc_mutex );

	#endif

	// return the size

	return( size );
}		// end of DBmalloc_inuse(...)

/*
 * Function:    malloc_mark()
 *
 * Purpose: to mark memory as validly in-use. This is used in order to 
 *      exempt verified segments from the leak list/counters so that
 *      once you verify that it is valid to leave the segment around
 *      forever, you can mark the segment and it won't be counted in
 *      the leak memory counts, no the leak segment list
 *
 * Arguments:   ptr - pointer to data area to mark
 *
 * Returns: true    - segment has been marked
 *      false   - segment not marked because it is invalid
 *
 * Narrative:
 *      make sure malloc subsystem is initialized
 *      if necessary, check malloc chain
 *      verify pointer is within malloc region
 *      get mlist pointer from passed address
 *      verify magic number
 *      verify inuse flag
 *      verify valid linkage
 *      mark segment
 */

VOIDTYPE
malloc_mark( DATATYPE *cptr )
{
	DBmalloc_mark((char *) NULL, 0, cptr);
}

VOIDTYPE
DBmalloc_mark(	CONST char *	file,
				int				line,
				DATATYPE *		cptr )
{
	CONST char *func = "malloc_mark";
	register struct mlist *ptr;

	// initialize the malloc sub-system.

	MALLOC_INIT();

	#ifdef MALLOC_PTHREAD

		if ( !malloc_preamble )
			pthread_mutex_lock( &malloc_mutex );

	#endif

	// If malloc chain checking is on, go do it.

	if (malloc_opts & MOPT_CKCHAIN)
	{
		VOIDCAST DBFmalloc_chain_check(func, file, line, 1);
	}

	//
	// verify that cptr is within the malloc region and that it is on
	// the correct alignment
	//

	if (	( cptr < malloc_data_start )				||
			(cptr > malloc_data_end)					||
			( ( ( (long) cptr ) & malloc_round) != 0) )
	{
		malloc_errno = M_CODE_BAD_PTR;
		malloc_warning(func, file, line, (struct mlist *) NULL);

		#ifdef MALLOC_PTHREAD

			if ( !malloc_preamble )
				pthread_mutex_unlock( &malloc_mutex );

		#endif

		return;
	}

	//
	// convert pointer to mlist struct pointer.  To do this we must 
	// move the pointer backwards the correct number of bytes...
	//

	ptr = (struct mlist *) (((char *) cptr) - M_SIZE);

	// check the magic number 

	if ((ptr->flag & M_MAGIC_BITS) != M_MAGIC)
	{
		malloc_errno = M_CODE_BAD_MAGIC;
		malloc_warning(func, file, line, (struct mlist *) NULL);

		#ifdef MALLOC_PTHREAD

			if ( !malloc_preamble )
				pthread_mutex_unlock( &malloc_mutex );

		#endif

		return;
	}

	// if this segment is not flagged as being in use

	if (!(ptr->flag & M_INUSE))
	{
		malloc_errno = M_CODE_NOT_INUSE;
		malloc_warning(func, file, line, ptr);

		#ifdef MALLOC_PTHREAD

			if ( !malloc_preamble )
				pthread_mutex_unlock( &malloc_mutex );

		#endif

		return;
	}

	// check to see that the pointers are still connected

	if ((ptr->prev && (ptr->prev->next != ptr)) ||
		(ptr->next && (ptr->next->prev != ptr)) ||
		((ptr->next == NULL) && (ptr->prev == NULL)))
	{
		malloc_errno = M_CODE_BAD_CONNECT;
		malloc_warning(func, file, line, ptr);

		#ifdef MALLOC_PTHREAD

			if ( !malloc_preamble )
				pthread_mutex_unlock( &malloc_mutex );

		#endif

		return;
	}

	#ifdef MALLOC_PTHREAD

		if ( !malloc_preamble )
			pthread_mutex_unlock( &malloc_mutex );

	#endif

	ptr->flag |= M_MARKED;

}		// end of DBmalloc_mark(...)

/*
 * $Log: leak.c,v $
 * Revision 1.1  2002/07/04 12:53:10  agoyal
 * Add
 *
 * Revision 1.2  2001/09/17 18:58:41  sturt
 * Dont do any pthread mutex stuff prior to entering main() - i.e - in preamble.
 *
 * Revision 1.1.1.1  2001/09/17 15:27:33  sturt
 * Checkin of dbmalloc library.
 *
 * Revision 1.3  2001/09/15 18:31:23  sturt
 * Add pthread mutex locking of malloc_mutex for solaris systems.
 *
 * Revision 1.2  2001/09/14 17:33:29  sturt
 * Reformating of c files.
 *
 * Revision 1.1.1.1  2001/09/14 15:06:32  sturt
 * Initial checkin of dbmalloc library.
 *
 * Revision 1.12  1992/08/22  16:27:13  cpcahil
 * final changes for pl14
 *
 * Revision 1.11  1992/07/02  13:49:54  cpcahil
 * added support for new malloc_size function and additional tests to testerr
 *
 * Revision 1.10  1992/06/30  13:06:39  cpcahil
 * added support for aligned allocations
 *
 * Revision 1.9  1992/06/27  22:48:48  cpcahil
 * misc fixes per bug reports from first week of reviews
 *
 * Revision 1.8  1992/06/22  23:40:10  cpcahil
 * many fixes for working on small int systems
 *
 * Revision 1.7  1992/04/13  19:57:15  cpcahil
 * more patch 8 fixes
 *
 * Revision 1.6  1992/04/13  03:06:33  cpcahil
 * Added Stack support, marking of non-leaks, auto-config, auto-testing
 *
 * Revision 1.5  1992/01/30  12:23:06  cpcahil
 * renamed mallocint.h -> mallocin.h
 *
 * Revision 1.4  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.3  1991/12/02  19:10:09  cpcahil
 * changes for patch release 5
 *
 * Revision 1.2  91/11/25  14:41:54  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.1  91/11/24  00:49:26  cpcahil
 * first cut at patch 4
 */
