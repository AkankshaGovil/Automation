
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
/*
 * Author(s):
 *  pds - Paul D. Smith (paul_smith@dg.com)
 */
/*
 * This module defines several libc internal interfaces to the allocation
 * and/or memory routines under DG/UX.
 */

/*
 * Added "_" components to make sure that libc versions of other
 * malloc interfaces called by various libc routines (eg. getcwd) that
 * must be used from libc do not have competing malloc implementations.
 */
#include <stdio.h>
#include "mallocin.h"

DATATYPE *
_malloc( SIZETYPE size )
{
	return( debug_malloc( NULL, -1, size ) );
}

DATATYPE *
_realloc(	DATATYPE *cptr,
			SIZETYPE size )
{
	return (debug_realloc(NULL, -1, cptr, size));
}

DATATYPE *
_calloc(	SIZETYPE nelem,
			SIZETYPE elsize )
{
	return( debug_calloc( NULL, -1, nelem, elsize ) );
}

void
_free(	DATATYPE *cptr )
{
	debug_free(NULL, 0, cptr);
}

int
_mallopt(	int					cmd,
			union dbmalloptarg	value )
{
	return( dbmallopt( cmd, &value ) );
}

MEMDATA *
_bcopy(	CONST MEMDATA *ptr2,
		MEMDATA *ptr1,
		MEMSIZE len )
{
	return( DBbcopy( (char *) NULL, 0, ptr2, ptr1, len ) );
}

MEMDATA *
_bzero(	MEMDATA *ptr1,
		MEMSIZE len )
{
	return( DBbzero( (char *) NULL, 0, ptr1, len ) );
}

int
_bcmp(	CONST MEMDATA *ptr1,
		CONST MEMDATA *ptr2,
		MEMSIZE len )
{
	return( DBbcmp( (char *) NULL, 0, ptr2, ptr1, len ) );
}

MEMDATA *
__dg_bcopy( CONST MEMDATA *ptr2,
			MEMDATA *ptr1,
			MEMSIZE len )
{
	return( DBbcopy( (char *) NULL, 0, ptr2, ptr1, len ) );
}

MEMDATA *
__dg_bzero(	MEMDATA *ptr1,
			MEMSIZE len )
{
	return( DBbzero( (char *) NULL, 0, ptr1, len ) );
}

int
__dg_bcmp(	CONST MEMDATA *ptr1,
			CONST MEMDATA *ptr2,
			MEMSIZE len )
{
	return( DBbcmp( (char *) NULL, 0, ptr2, ptr1, len ) );
}


/*
 * $Log: dgmalloc.c,v $
 * Revision 1.1  2002/07/04 12:53:10  agoyal
 * Add
 *
 * Revision 1.1.1.1  2001/09/17 15:27:33  sturt
 * Checkin of dbmalloc library.
 *
 * Revision 1.2  2001/09/14 17:33:29  sturt
 * Reformating of c files.
 *
 * Revision 1.1.1.1  2001/09/14 15:06:32  sturt
 * Initial checkin of dbmalloc library.
 *
 * Revision 1.4  1992/08/22  16:27:13  cpcahil
 * final changes for pl14
 *
 * Revision 1.3  1992/07/03  00:03:25  cpcahil
 * more fixes for pl13, several suggestons from Rich Salz.
 *
 * Revision 1.2  1992/07/02  15:35:52  cpcahil
 * misc cleanups for PL13
 *
 * Revision 1.1  1992/05/06  04:53:29  cpcahil
 * performance enhancments
 *
 */
