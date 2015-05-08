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

//	"$Id: trace.h,v 1.6.2.8 2004/10/14 00:21:12 amar Exp $"

#ifndef NETOID_LINUX
#include <sys/lwp.h>
#include <synch.h>
#endif

#define	TRACE_ENTRIES	2000

extern	int32_t			trace_disk;
extern	int32_t			trace_initialized;
extern	char			trace_log[256]; // = "/var/log/ispd.log";
extern	char 			progname[];

extern	FILE*			trace_fdesc;
extern	int32_t			trace_fd;


//
//	Media Gateway trace table entry structure.
//

typedef	struct _trace_entry
{
	char	time[18];
	char	seperator_1;
	char	thread_id[5];
	char	seperator_2[2];
	char	progname[11];
	char	seperator_3[2];
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

extern	trace_table_t	_trace_tbl;

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
//		trc_init()
//
//	Arguments		:
//
//		None
//
//	Description		:
//		Initialize ispd trace mutex and open up stream
//		for trace output for ispd process.
//
//	Return value	:
//		None
//
static	inline	void
trc_init(void)
{
	memset(	&_trace_tbl,
			(int) 0,
			sizeof( trace_table_t ) );

	pthread_mutex_init( &_trace_tbl.mutex, NULL );

	if	(trace_disk) {
		struct stat	trace_log_stat;
		char		target_name[1024], input_string[1024];
		char *		get_log_names = 
						"/usr/bin/ls -1r /var/log/ispd.log*";
		char *		get_log_count = 
						"/usr/bin/ls -1r /var/log/ispd.log* | /usr/bin/wc -l";
		FILE *		popen_handle;
		int			file_count, i, len;

		// Is size of current "ispd.log" file in "/var/log/"
		// greater than zero ?

		if ( !stat( trace_log, &trace_log_stat ) &&
			 trace_log_stat.st_size > 0 ) {
			// Yes, slide all trace files down by one
			// renaming each

			// get # of ispd.log files

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
				if ( i > 5 ) {
					fgets( input_string, 200, popen_handle );
					len = strlen( input_string );
					input_string[ len - 1 ] = '\000';
					unlink( input_string );
				} else {
					fgets( input_string, 200, popen_handle );
					len = strlen( input_string );
					input_string[ len - 1 ] = '\000';
					sprintf( target_name, "/var/log/ispd.log-%04d", i );
					rename( input_string, target_name );
				}
			}

			pclose( popen_handle );
		}

		// Open new ispd.log file

		trace_fd = open( trace_log, (O_WRONLY|O_CREAT), 0644 );

		trace_fdesc = fdopen( trace_fd, "a+" );
	}

	trace_initialized = 1;
}

//
//	Function		:
//		trc_error()
//
//	Arguments		:
//
//		module		integer define representing ispd module.
//
//		fmt			fprintf format for trace output.
//
//		...			variable number of arguments used to
//					fill in format.
//
//	Description		:
//		Print error output to the ispd log file, 
//		/var/log/ispd.log
//
//	Return value	:
//		None
//
static	inline	void
trc_error( uint32_t module, char* fmt, ... )
{
	va_list 		ap;
	uint32_t		thread 	= pthread_self();
	struct timespec	curtime;
	char        	timestr[64];
	char        	temp_str[16];
	char        	fract_str[16];
	double			nsec_float, result;
	struct			stat	statbuf;
	int32_t			check_seek = 0;

	if (!trace_initialized)
		trc_init();
	else
	{
		if ( fstat( trace_fd, &statbuf ) >= 0 )
			check_seek = 1;
	}

	clock_gettime( CLOCK_REALTIME, &curtime ); 

	memset( temp_str, (int) 0, 16 );

	sprintf( temp_str, "%ld.", curtime.tv_nsec );

	ctime_r( (time_t*) &curtime.tv_sec, timestr );

	nsec_float = atof( temp_str );

	result = ( nsec_float/1000000000. );

	sprintf( fract_str, "%0.2f ", result );

	pthread_mutex_lock( &_trace_tbl.mutex );

	memcpy( _trace_tbl.entries[ _trace_tbl.current ].time,
			(timestr + 4), 15 );

	memcpy( &_trace_tbl.entries[ _trace_tbl.current ].time[ 15 ],
			(fract_str + 1), 4 );

	sprintf(	_trace_tbl.entries[ _trace_tbl.current ].thread_id,
				"[%3d]", thread );

	sprintf(	_trace_tbl.entries[ _trace_tbl.current ].seperator_2,
				"%s", ": " );

	sprintf(	_trace_tbl.entries[ _trace_tbl.current ].progname,
				"%-11s", progname );

	sprintf(	_trace_tbl.entries[ _trace_tbl.current ].seperator_3,
				"%s", ": " );

	va_start( ap, fmt );

	vsprintf(	_trace_tbl.entries[ _trace_tbl.current ].descr,
				fmt, ap );

	va_end( ap );


	if ( trace_disk )
	{
		if ( check_seek )
		{
			if ( !statbuf.st_size )
			{
				fclose( trace_fdesc );
				close( trace_fd );

				trace_fd = open( trace_log, (O_WRONLY|O_CREAT) );
				trace_fdesc = fdopen( trace_fd, "a+" );
			}
		}

		fprintf(	trace_fdesc, "%s",
					(char*) &_trace_tbl.entries[ _trace_tbl.current ] );

		fflush( trace_fdesc );
	}
	else
	{
		if ( _trace_tbl.current < (TRACE_ENTRIES - 1) )
			_trace_tbl.current++;
		else
			_trace_tbl.current = 0;

		if ( _trace_tbl.total < TRACE_ENTRIES )
			_trace_tbl.total++;
		else
			_trace_tbl.first = _trace_tbl.current;
	}

	pthread_mutex_unlock( &_trace_tbl.mutex );
}

//
//	Function		:
//		trc_debug()
//
//	Arguments		:
//
//		module		integer define representing ispd module.
//
//		level		integer level at which message is to be output.
//
//		fmt			fprintf format for trace output.
//
//		...			variable number of arguments used to
//					fill in format.
//
//	Description		:
//		Print debugging output to the ispd log file, 
//		/var/log/ispd.log
//
//	Return value	:
//		None
//
static	inline	void
trc_debug( uint32_t module, uint32_t level, char* fmt, ... )
{
	va_list ap;

	if ( ( 	(uint32_t) NetLogStatus[ module ] & NETLOG_DEBUGLEVELMASK ) >= 
			(uint32_t) level )
	{
		uint32_t		thread 	= pthread_self();
		struct timespec	curtime;
		char        	timestr[120];
		char        	temp_str[16];
		char        	fract_str[16];
		double			nsec_float, result;
		struct			stat	statbuf;
		int32_t			check_seek = 0;

		if (!trace_initialized)
			trc_init();
		else
		{
			if ( fstat( trace_fd, &statbuf ) >= 0 )
				check_seek = 1;
		}

		clock_gettime( CLOCK_REALTIME, &curtime ); 

		memset( temp_str, (int) 0, 16 );
		sprintf( temp_str, "%ld.", curtime.tv_nsec );

		ctime_r( (time_t*) &curtime.tv_sec, timestr );

		nsec_float = atof( temp_str );

		result = ( nsec_float/1000000000. );

		sprintf( fract_str, "%0.2f ", result );

		pthread_mutex_lock( &_trace_tbl.mutex );

		memcpy( _trace_tbl.entries[ _trace_tbl.current ].time,
				(timestr + 4), 15 );

		memcpy( &_trace_tbl.entries[ _trace_tbl.current ].time[ 15 ],
				(fract_str + 1), 4 );

		sprintf(	_trace_tbl.entries[ _trace_tbl.current ].thread_id,
					"[%3d]", thread );

		sprintf(	_trace_tbl.entries[ _trace_tbl.current ].seperator_2,
					"%s", ": " );

		sprintf(	_trace_tbl.entries[ _trace_tbl.current ].progname,
					"%-11s", progname );

		sprintf(	_trace_tbl.entries[ _trace_tbl.current ].seperator_3,
					"%s", ": " );

		va_start( ap, fmt );

		vsprintf(	_trace_tbl.entries[ _trace_tbl.current ].descr,
					fmt, ap );

		va_end( ap );

		if ( trace_disk )
		{
			if ( check_seek )
			{
				if ( !statbuf.st_size )
				{
					fclose( trace_fdesc );
					close( trace_fd );
					trace_fd = open( trace_log, (O_WRONLY|O_CREAT) );
					trace_fdesc = fdopen( trace_fd, "a+" );
				}
			}

			fprintf(	trace_fdesc, "%s", 
						(char*) &_trace_tbl.entries[ _trace_tbl.current ] );
			fflush( trace_fdesc );
		}
		else
		{
			if ( _trace_tbl.current < (TRACE_ENTRIES - 1) )
				_trace_tbl.current++;
			else
				_trace_tbl.current = 0;

			if ( _trace_tbl.total < TRACE_ENTRIES )
				_trace_tbl.total++;
			else
				_trace_tbl.first = _trace_tbl.current;
		}

		pthread_mutex_unlock( &_trace_tbl.mutex );

	}
}

#endif /* _MG_TRACE_H_ */
