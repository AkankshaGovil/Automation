#include "unp.h"

//
//	Function		:
//		msleep()
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
void
msleep( uint32_t milliseconds )
{
	struct	timespec	delay;
	struct	timespec	remaining;
	int32_t				retval;

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
			fprintf( stderr, "THRD : incr_timespec(): errno = EINVAL\n" );
			break;
		}
	}
}

