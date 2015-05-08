/////////////////////////////////////////////////////////////////////
//
//	Name:
//		trace.h
//
//	Description:
//			This header file contains common definitions of
//		structures used for trace table control in the media
//		gateway process (mgd) on the iEdge 1000.
//			The static inline functions defined in this header
//		file are used to control and output to the trace table
//
/////////////////////////////////////////////////////////////////////

#ifndef _MG_TRACE_H_
#define	_MG_TRACE_H_

//	"$Id: trace.h,v 1.2.2.12 2004/10/20 21:22:21 bmoreland Exp $"

#include <sys/stat.h>
#include <pthread.h>
// #include <sys/lwp.h> this is not used on solaris too
// #include <synch.h> this is not used on solaris too

#define	TRACE_ENTRIES	2000

#define TRACE_NSF_TRACE_FILENAME  "/var/log/nsf.trc"

extern	int32_t			trace_disk;
extern	FILE*			trace_fdesc;
extern	char			trace_log[256]; // = "/var/log/nsf.trc";
extern	int32_t			nsf_reconfig;


//
//	Media Gateway trace table entry structure.
//

typedef	struct _trace_entry
{
	char	time[18];
	char	seperator_1;
	char	thread_id[8];
	char	seperator_2[2];
	char	descr[128];
} trace_entry_t;

//
//	Media Gateway trace table structure.
//

typedef struct	_trace_table
{
	pthread_mutex_t		mutex;
	uint32_t			first;
	uint32_t			current;
	uint32_t			total;

	trace_entry_t	entries[TRACE_ENTRIES];
} trace_table_t;

extern	trace_table_t	iserv_trace_tbl;

//
//	Function		:
//		incr_timespec()
//
//	Arguments		:
//		now				pointer to a timespec to be incremented.
//
//		seconds			# of seconds to increment timespec
//
//		nanoseconds		# of nanoseconds to increment timespec
//
//	Purpose			:
//		Make adjustments to input timespec given nanoseconds
//		to increment.
//
//	Description		:
//		This function is given a pointer to a timespec 
//		structure to be incremented by an input number
//		of nanoseconds. It correctly adjusts the timespec,
//		adding the number of nanoseconds specified correctly.
//
//	Return value	:
//		None
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
		//trc_error( 	MMG_THRD,
		//			"THRD : incr_timespec(): invalid value for nanoseconds (%u)\n",
		//			nanoseconds );
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

static	inline	void
trc_init(void)
{
	if ( nsf_reconfig )
		return;

	memset(	&iserv_trace_tbl,
			(int) 0,
			sizeof( trace_table_t ) );

	pthread_mutex_init( &iserv_trace_tbl.mutex, NULL );

	if	(trace_disk)
	{
		struct stat	trace_log_stat;
		char		target_name[1024], input_string[1024];
		char 		get_log_names[80];
		char 		get_log_count[80]; 
		FILE *		popen_handle;
		int			file_count, i, len;

		sprintf( get_log_names, "/usr/bin/ls -1r %s*", TRACE_NSF_TRACE_FILENAME );
		sprintf( get_log_count, "/usr/bin/ls -1r %s* | /usr/bin/wc -l", TRACE_NSF_TRACE_FILENAME );
		// Is size of current "nsf.trc" file in "var/adm/"
		// greater than zero ?

		if ( !stat( trace_log, &trace_log_stat ) &&
			 trace_log_stat.st_size > 0 )
		{
			// Yes, slide all trace files down by one
			// renaming each

			// get # of nsf.trc files

			popen_handle = popen( get_log_count, "r" );
			fgets( input_string, 200, popen_handle );
			pclose( popen_handle );
			file_count = atoi( input_string );

			// Open a pipe containing file names
			// In the for loop we rename the files 
			// one at a time.

			popen_handle = popen( get_log_names, "r" );

			for ( i = file_count; i >= 1; i-- )
			{
				fgets( input_string, 200, popen_handle );
				len = strlen( input_string );
				input_string[ len - 1 ] = '\000';

				if ( i > 10 ) {
					unlink( input_string );
				} else {
					sprintf( target_name, "%s-%04d", TRACE_NSF_TRACE_FILENAME, i );
					rename( input_string, target_name );
				}
			}

			pclose( popen_handle );
		}

		// Open new nsf.trc file
		trace_fdesc = fopen( trace_log, "a+" );
	}
}

static	inline	void
trc_close(void)
{
	if ( nsf_reconfig )
		return;

	pthread_mutex_destroy( &iserv_trace_tbl.mutex );

	if ( trace_fdesc )
	{
		fclose( trace_fdesc );
	}
}

static	inline	void
trc_noop( char* fmt, ... )
{
}

static	inline	void
trc_error( uint32_t module, char* fmt, ... )
{
	va_list 		ap;
	uint32_t		thread 	= pthread_self();
	struct timespec	curtime;
	char        	timestr[64];
	char			temp_str[16];
	char			fract_str[16];
	double			nsec_float, result;


	clock_gettime( CLOCK_REALTIME, &curtime );

	memset( temp_str, (int) 0, 16 );

	sprintf( temp_str, "%ld.", curtime.tv_nsec );

	ctime_r( (time_t*) &curtime.tv_sec, timestr );

	nsec_float = atof( temp_str );

	result = ( nsec_float/1000000000. );

	sprintf( fract_str, "%0.2f ", result );

	pthread_mutex_lock( &iserv_trace_tbl.mutex );

	memcpy( iserv_trace_tbl.entries[ iserv_trace_tbl.current ].time,
			(timestr + 4), 15 );

	memcpy( &iserv_trace_tbl.entries[ iserv_trace_tbl.current ].time[15],
			(fract_str + 1), 4 );

	sprintf(	iserv_trace_tbl.entries[ iserv_trace_tbl.current ].thread_id,
				"[%6d]", thread );

	sprintf(	iserv_trace_tbl.entries[ iserv_trace_tbl.current ].seperator_2,
				"%s", ": " );

	va_start( ap, fmt );

	vsprintf(	iserv_trace_tbl.entries[iserv_trace_tbl.current].descr,
				fmt, ap );

	va_end( ap );

	if ( trace_disk )
	{
		fprintf(	trace_fdesc, "%s", 
					(char*) &iserv_trace_tbl.entries[ iserv_trace_tbl.current ] );

		fflush( trace_fdesc );
	}
	else
	{
		if ( iserv_trace_tbl.current < (TRACE_ENTRIES - 1) )
			iserv_trace_tbl.current++;
		else
			iserv_trace_tbl.current = 0;

		if ( iserv_trace_tbl.total < TRACE_ENTRIES )
			iserv_trace_tbl.total++;
		else
			iserv_trace_tbl.first = iserv_trace_tbl.current;
	}

	pthread_mutex_unlock( &iserv_trace_tbl.mutex );
}

static	inline	void
trc_debug( uint32_t module, uint32_t level, char* fmt, ... )
{
	va_list ap;

	if ( NetLogStatus[ module ] & level )
	{
		uint32_t		thread 	= pthread_self();
		struct timespec	curtime;
		char        	timestr[64];
		char        	temp_str[16];
		char        	fract_str[16];
		double        	nsec_float, result;

		clock_gettime( CLOCK_REALTIME, &curtime );

		memset( temp_str, (int) 0, 16 );

		sprintf( temp_str, "%ld.", curtime.tv_nsec);

		ctime_r( (time_t*) &curtime.tv_sec, timestr );

		nsec_float = atof( temp_str );

		result = ( nsec_float/1000000000. );

		sprintf( fract_str, "%0.2f ", result );


		pthread_mutex_lock( &iserv_trace_tbl.mutex );

		memcpy( iserv_trace_tbl.entries[ iserv_trace_tbl.current ].time,
				(timestr + 4), 15 );

		memcpy( &iserv_trace_tbl.entries[ iserv_trace_tbl.current ].time[15],
				(fract_str + 1), 4 );

		sprintf(	iserv_trace_tbl.entries[ iserv_trace_tbl.current ].thread_id,
					"[%6d]", thread );

		sprintf(	iserv_trace_tbl.entries[ iserv_trace_tbl.current ].seperator_2,
					"%s", ": " );

		va_start( ap, fmt );

		vsprintf(	iserv_trace_tbl.entries[iserv_trace_tbl.current].descr,
					fmt, ap );

		va_end( ap );

		if ( trace_disk )
		{
			fprintf(	trace_fdesc, "%s", 
						(char*) &iserv_trace_tbl.entries[ iserv_trace_tbl.current ] );
			fflush( trace_fdesc );
		}
		else
		{
			if ( iserv_trace_tbl.current < (TRACE_ENTRIES - 1) )
				iserv_trace_tbl.current++;
			else
				iserv_trace_tbl.current = 0;

			if ( iserv_trace_tbl.total < TRACE_ENTRIES )
				iserv_trace_tbl.total++;
			else
				iserv_trace_tbl.first = iserv_trace_tbl.current;
		}

		pthread_mutex_unlock( &iserv_trace_tbl.mutex );

	}
}

#endif /* _MG_TRACE_H_ */
