#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "srvrlog.h"
#include <unistd.h>
#include "packets.h"

/* read atleast min bytes, at most max bytes */
ssize_t
uiio_read(int fd, char *buf, size_t min, size_t max)
{
	unsigned int read_bytes = 0;
	ssize_t retval = -1;

 _do_read:
	/* Read from ephemeral socket */

	retval = read(fd, (buf + read_bytes), (max-read_bytes));

	if (retval < 0)
	{
		int	lerrno = errno;
		char *	errmsg = strerror(lerrno);

		if (lerrno == EAGAIN)
		{
			/* socket must be non-blocking. We are assuming that
			 * the call was made only when we know that there
			 * is data to be read. In that case we must wait 
			 * until the data is there or the connection breaks
			 */
			goto _do_read;
		}
	     /* We dont know now, where we are in the stream... */
		if ( errmsg != (char*) NULL )
		{
	     	NETERROR(	MPKT,
						("uiio_read(): read failed - errno %d - %s",
						lerrno,
						strerror(lerrno) ));
		}
		else
		{
	     	NETERROR(	MPKT,
						("uiio_read(): read failed - errno %d",
						lerrno ));
		}

	     return retval;
	}

	if (retval == 0)
	{
	     goto _do_nothing;
	}

	read_bytes += retval;
	
	if (read_bytes < min)
	{
	     /* We havent even read a decent amount of data... */
	     goto _do_read;
	}

_do_nothing:
	return read_bytes;
}

/* read atleast min bytes, at most max bytes */
ssize_t
uiio_recvfrom(int fd, char *buf, size_t min, size_t max, 
	      struct sockaddr_in *from, int *fromlen)
{
	unsigned int read_bytes = 0;
	ssize_t retval = -1;

 _do_read:
	/* Read from ephemeral socket */

	retval = recvfrom(fd, (buf + read_bytes), (max-read_bytes), 0,
			  (struct sockaddr *)from, fromlen);

	if (retval < 0)
	{
		int 	lerrno = errno;
		char *	errmsg = strerror(lerrno);

		if (lerrno == EAGAIN)
		{
			/* socket must be non-blocking. We are assuming that
			 * the call was made only when we know that there
			 * is data to be read. In that case we must wait 
			 * until the data is there or the connection breaks
			 */
			goto _do_read;
		}
	     /* We dont know now, where we are in the stream... */

		if ( errmsg != (char*) NULL )
		{
	     	NETERROR(	MPKT,
						("uiio_recvfrom(): read failed - errno %d - %s",
						lerrno,
						errmsg ));
		}
		else
		{
	     	NETERROR(	MPKT,
						("uiio_recvfrom(): read failed - errno %d",
						lerrno ));
		}

	     goto _do_nothing;
	}

	if (retval == 0)
	{
	     goto _do_nothing;
	}

	read_bytes += retval;
	
	if (read_bytes < min)
	{
	     /* We havent even read a decent amount of data... */
	     goto _do_read;
	}

_do_nothing:
	return read_bytes;
}
