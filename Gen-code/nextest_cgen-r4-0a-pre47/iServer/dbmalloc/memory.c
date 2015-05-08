
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
static char rcs_hdr[] = "$Id: memory.c,v 1.1 2002/07/04 12:53:10 agoyal Exp $";
#endif

#include <stdio.h>
#include "mallocin.h"

/*
 * memccpy - copy memory region up to specified byte or length
 */
MEMDATA *
memccpy(	MEMDATA *		ptr1,
			CONST MEMDATA *	ptr2,
			int				ch,
			MEMSIZE			len )
{
	return( DBmemccpy( (char *) NULL, 0, ptr1, ptr2, ch, len ) );
}

MEMDATA *
DBmemccpy(	CONST char *	file,
			int				line,
			MEMDATA *		ptr1,
			CONST MEMDATA *	ptr2,
			int				ch,
			MEMSIZE			len )
{
	register CONST char *myptr2;
	register MEMSIZE i;
	MEMDATA *rtn;

	myptr2 = (CONST char *) ptr2;

	//
	// I know that the assignment could be done in the following, but
	// I wanted to perform a check before any assignment, so first I 
	// determine the length, check the pointers and then do the assignment.
	//

	for (i = 0; (i < len) && (myptr2[i] != ch); i++)
	{
	}

	// if we found the character...

	if (i < len)
	{
		rtn = ((char *) ptr1) + i + 1;
		i++;
	}
	else
		rtn = (char *) 0;

	// make sure we have enough room in both ptr1 and ptr2

	malloc_check_data("memccpy", file, line, ptr1, i);
	malloc_check_data("memccpy", file, line, ptr2, i);

	DataMC(ptr1, ptr2, i);

	return( rtn );
}

/*
 * memchr - find a byte in a memory region
 */
MEMDATA *
memchr(	CONST MEMDATA *	ptr1,
		register		int ch,
		MEMSIZE			len )
{
	return( DBmemchr( (char *) NULL, 0, ptr1, ch, len ) );
}

MEMDATA *
DBmemchr(	CONST char *	file,
			int				line,
			CONST MEMDATA *	ptr1,
			register int	ch,
			MEMSIZE			len )
{
	register CONST char *myptr1;
	MEMSIZE i;

	malloc_check_data("memchr", file, line, ptr1, len);

	myptr1 = (CONST char *) ptr1;

	for (i = 0; (i < len) && (myptr1[i] != (char) ch); i++)
	{
	}

	if (i < len)
		return( (MEMDATA *) (myptr1 + i) );
	else
		return( (MEMDATA *) 0 );
}

/*
 * memcpy  - copy one memory area to another
 * memmove - copy one memory area to another
 */
MEMDATA *
memmove(	MEMDATA *			ptr1,
			CONST MEMDATA *		ptr2,
			register MEMSIZE	len )
{
	return( DBmemmove( (char *) NULL, 0, ptr1, ptr2, len ) );
}

MEMDATA *
DBmemmove(	CONST char *		file,
			int					line,
			MEMDATA *			ptr1,
			CONST MEMDATA *		ptr2,
			register MEMSIZE	len )
{
	return (DBFmemcpy("memmove", file, line, ptr1, ptr2, len));
}


MEMDATA *
memcpy(	MEMDATA *			ptr1,
		CONST MEMDATA *		ptr2,
		register MEMSIZE	len )
{
	return( DBmemcpy( (char *) NULL, 0, ptr1, ptr2, len ) );
}

MEMDATA *
DBmemcpy(	CONST char *		file,
			int					line,
			MEMDATA *			ptr1,
			CONST MEMDATA *		ptr2,
			register MEMSIZE	len )
{
	return( DBFmemcpy( "memcpy", file, line, ptr1, ptr2, len ) );
}

MEMDATA *
DBFmemcpy(	CONST char *		func,
			CONST char *		file,
			int					line,
			MEMDATA *			ptr1,
			CONST MEMDATA *		ptr2,
			register MEMSIZE	len )
{
	MEMDATA *rtn = ptr1;

	malloc_check_data(func, file, line, ptr1, len);
	malloc_check_data(func, file, line, ptr2, len);

	DataMC(ptr1, ptr2, len);

	return( rtn );
}

/*
 * memcmp - compare two memory regions
 */
int
memcmp(	CONST MEMDATA *		ptr1,
		CONST MEMDATA *		ptr2,
		register MEMSIZE	len )
{
	return( DBmemcmp( (char *) NULL, 0, ptr1, ptr2, len ) );
}

int
DBmemcmp(	CONST char *		file,
			int					line,
			CONST MEMDATA *		ptr1,
			CONST MEMDATA *		ptr2,
			register MEMSIZE	len )
{
	return( DBFmemcmp( "memcmp", file, line, ptr1, ptr2, len ) );
}

int
DBFmemcmp(	CONST char *		func,
			CONST char *		file,
			int					line,
			CONST MEMDATA *		ptr1,
			CONST MEMDATA *		ptr2,
			register MEMSIZE	len )
{
	register CONST char *myptr1;
	register CONST char *myptr2;

	malloc_check_data(func, file, line, ptr1, len);
	malloc_check_data(func, file, line, ptr2, len);

	myptr1 = (CONST char *) ptr1;
	myptr2 = (CONST char *) ptr2;

	while (len > 0 && (*myptr1 == *myptr2))
	{
		len--;
		myptr1++;
		myptr2++;
	}

	// If stopped by len, return zero

	if (len == 0)
		return( 0 );

	return( *(CONST MEMCMPTYPE *) myptr1 - *(CONST MEMCMPTYPE *) myptr2 );
}

/*
 * memset - set all bytes of a memory block to a specified value
 */
MEMDATA *
memset(	MEMDATA *			ptr1,
		register int		ch,
		register MEMSIZE	len )
{

	#ifdef MALLOC_PREAMBLE
		if ( malloc_preamble )
			return ( (MEMDATA*) _memset( ptr1,ch, len ) );
		else
	#endif

	return( DBmemset( (char *) NULL, 0, ptr1, ch, len ) );
}

MEMDATA *
DBmemset(	CONST char *		file,
			int					line,
			MEMDATA *			ptr1,
			register int		ch,
			register MEMSIZE	len )
{
	return( DBFmemset( "memset", file, line, ptr1, ch, len ) );
}

MEMDATA *
DBFmemset(	CONST char *		func,
			CONST char *		file,
			int					line,
			MEMDATA *			ptr1,
			register int		ch,
			register MEMSIZE	len )
{
	MEMDATA *rtn = ptr1;

	malloc_check_data(func, file, line, ptr1, len);

	DataMS(ptr1, ch, len);

	return( rtn );
}

#ifndef ibm032

/*
 * bcopy - copy memory block to another area
 */
MEMDATA *
bcopy(	CONST MEMDATA *	ptr2,
		MEMDATA *		ptr1,
		MEMSIZE			len )
{
	return( DBbcopy( (char *) NULL, 0, ptr2, ptr1, len ) );
}

#endif /* ibm032 */

MEMDATA *
DBbcopy(	CONST char *	file,
			int				line,
			CONST MEMDATA *	ptr2,
			MEMDATA *		ptr1,
			MEMSIZE			len )
{
	return( DBFmemcpy( "bcopy", file, line, ptr1, ptr2, len ) );
}

/*
 * bzero - clear block of memory to zeros
 */
MEMDATA *
bzero(	MEMDATA *	ptr1,
		MEMSIZE		len )
{
	return( DBbzero( (char *) NULL, 0, ptr1, len ) );
}

MEMDATA *
DBbzero(	CONST char *	file,
			int				line,
			MEMDATA *		ptr1,
			MEMSIZE			len )
{
	return( DBFmemset( "bzero", file, line, ptr1, '\0', len ) );
}

/*
 * bcmp - compary memory blocks
 */
int
bcmp(	CONST MEMDATA *	ptr1,
		CONST MEMDATA *	ptr2,
		MEMSIZE			len )
{
	return( DBbcmp( (char *) NULL, 0, ptr2, ptr1, len ) );
}

int
DBbcmp(	CONST char *	file,
		int				line,
		CONST MEMDATA *	ptr1,
		CONST MEMDATA *	ptr2,
		MEMSIZE			len )
{
	return( DBFmemcmp( "bcmp", file, line, ptr1, ptr2, len ) );
}

/*
 * $Log: memory.c,v $
 * Revision 1.1  2002/07/04 12:53:10  agoyal
 * Add
 *
 * Revision 1.1.1.1  2001/09/17 15:27:33  sturt
 * Checkin of dbmalloc library.
 *
 * Revision 1.3  2001/09/15 15:21:29  sturt
 * Add malloc_preamble stuff for sparc SunOS to prevent dbmalloc
 * initialization during _init() prior to entry into process
 * main().
 *
 * Revision 1.2  2001/09/14 21:33:39  sturt
 * Updates to formating for c files.
 *
 * Revision 1.1.1.1  2001/09/14 15:06:32  sturt
 * Initial checkin of dbmalloc library.
 *
 * Revision 1.22  1992/08/22  16:27:13  cpcahil
 * final changes for pl14
 *
 * Revision 1.21  1992/07/12  15:30:58  cpcahil
 * Merged in Jonathan I Kamens' changes
 *
 * Revision 1.20  1992/06/22  23:40:10  cpcahil
 * many fixes for working on small int systems
 *
 * Revision 1.19  1992/05/09  21:27:09  cpcahil
 * final (hopefully) changes for patch 11
 *
 * Revision 1.18  1992/05/09  00:16:16  cpcahil
 * port to hpux and lots of fixes
 *
 * Revision 1.17  1992/05/08  02:30:35  cpcahil
 * minor cleanups from minix/atari port
 *
 * Revision 1.16  1992/05/08  01:44:11  cpcahil
 * more performance enhancements
 *
 * Revision 1.15  1992/04/13  03:06:33  cpcahil
 * Added Stack support, marking of non-leaks, auto-config, auto-testing
 *
 * Revision 1.14  1992/01/30  12:23:06  cpcahil
 * renamed mallocint.h -> mallocin.h
 *
 * Revision 1.13  1992/01/24  04:49:05  cpcahil
 * changed memccpy to only check number of chars it will copy.
 *
 * Revision 1.12  1991/12/31  21:31:26  cpcahil
 * changes for patch 6.  See CHANGES file for more info
 *
 * Revision 1.11  1991/12/02  19:10:13  cpcahil
 * changes for patch release 5
 *
 * Revision 1.10  91/11/25  14:42:03  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.9  91/11/24  00:49:31  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.8  91/05/21  18:33:47  cpcahil
 * fixed bug in memccpy() which checked an extra byte if the first character
 * after the specified length matched the search character.
 * 
 * Revision 1.7  90/08/29  21:27:58  cpcahil
 * fixed value of check in memccpy when character was not found.
 * 
 * Revision 1.6  90/07/16  20:06:26  cpcahil
 * fixed several minor bugs found with Henry Spencer's string/mem tester 
 * program.
 * 
 * 
 * Revision 1.5  90/05/11  15:39:36  cpcahil
 * fixed bug in memccpy().
 * 
 * Revision 1.4  90/05/11  00:13:10  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/24  21:50:29  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.2  90/02/24  17:29:41  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs 
 * id string
 * 
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 * 
 */
