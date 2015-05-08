/////////////////////////////////////////////////////////////////////
//
//	Name:
//		trace.h
//
//	Description:
//			This header file contains common definitions of
//		structures used for trace table control.
//			The static inline functions defined in this header
//		file are used to control and output to the trace table
//
/////////////////////////////////////////////////////////////////////

#ifndef _H323_TRACE_H_
#define	_H323_TRACE_H_

//	"$Id: trace.h,v 1.1.6.3 2004/09/09 23:48:15 amar Exp $"

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <termios.h>
#include <pthread.h>

// #include <synch.h> not used on solaris too
#include <stdarg.h>

#define	TRACE_ENTRIES		20000

static	inline	void trc_dump( void );

//
//	Media Gateway trace table entry structure.
//

typedef	struct _trace_entry
{
	char	descr[256];
} trace_entry_t;

//
//	Media Gateway trace table structure.
//

typedef struct	_trace_table
{
	pthread_mutex_t		mutex;
	uint32_t			first;		// index to first entry to be output by trc_dump()
	uint32_t			current;	// index to next entries[] element to be used
	uint32_t			total;		// total entries[] elements in use

	trace_entry_t	entries[TRACE_ENTRIES];
} trace_table_t;

extern	trace_table_t	h323_trace_tbl;
extern	int				shutdown_calls;
extern	FILE*			h323_fdesc;

static	inline	void
trc_init(void)
{
	char		log_file[256];

	memset(	&h323_trace_tbl, (int) 0, sizeof( trace_table_t ) );
	pthread_mutex_init( &h323_trace_tbl.mutex, NULL );

	memset( log_file, (int32_t) 0, 256 );

	sprintf( log_file, "./genh323.log" );

	h323_fdesc = fopen( log_file, "a" );
}

static	inline	void
trc_dump( void )
{
	uint32_t	recs, cur_rec;

	pthread_mutex_lock( &h323_trace_tbl.mutex );

	cur_rec = h323_trace_tbl.first;

	for ( recs = 0; recs < h323_trace_tbl.total; recs++ )
	{
		if ( cur_rec >= TRACE_ENTRIES )
			cur_rec = 0;

		fprintf( h323_fdesc, "%s\n", (char*) &h323_trace_tbl.entries[cur_rec].descr );
		cur_rec++;
	}

	fprintf( h323_fdesc, "trace_table counters: first %d, current %d, total %d\n",
			h323_trace_tbl.first,
			h323_trace_tbl.current,
			h323_trace_tbl.total );

	fclose( h323_fdesc );

	pthread_mutex_unlock( &h323_trace_tbl.mutex );
}

#endif /* _H323_TRACE_H_ */
