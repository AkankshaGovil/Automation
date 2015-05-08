#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>
#include "nxioctl.h"
#include "srvrlog.h"
#include "packets.h"

/* fragment incoming stream into packets. 
 * app should provide a valid fd, buf, and max
 * which is the max length of buf, so we would
 * not go beyond (buf+max)...
 * start initially points to buf, and in subsequent 
 * calls points somewhere inside buf, till the point
 * app wants to process data, if it gets more bytes than
 * it wants... 
 * end refers to what point the buf is stuffed upto by us.
 * pktLen is a function which must behave as follows:
 * when called as pktLen(char *), it should parse
 * the packet and tell how much comprises one packet.
 * When called with an arg of NULL, it must supply the
 * min length with which it would be able to determine
 * what is the length of the incoming packet (this
 * property may be used later on, but for now we avoid
 * one more fun call, by using min).
 * this function is best, if called in a loop ;-)
 * signal blocking is left to app.
 * function returns the num of readable bytes behind start,
 * which are in buf.
 */
int
get_next_packet(
		int fd, 			/* fd on which to get */
		char *buf, 			/* app supplied buffer */
		void **startp, 			/* start in buffer (in/out) */
		void **endp,			/* end in buffer (out) */
		unsigned int (*pkt_len)(void *),/* returns len of incoming packet */
		unsigned int min,		/* min to read before we call pktLen */
		unsigned int max		/* max size of buffer (read in 1 shot) */
)
{
	int len_read, len_req, len_left, max_left;
	int retval = -1;
	char *start = *startp, *end = *endp;	

	len_read = end - start;

	if (len_read < min)
	{
		/* We dont even have a min length read in ... */
		len_left = min - len_read;
		max_left = max - (end-buf);

		/* do phase 1 read... then we must return to check
		 * whether phase 2 was completed as well ?
		 */
		goto _do_read;
	}

_phase2:

	if (len_read >= min)
	{
		len_req = pkt_len(start);
		if (len_read >= len_req)
		{
			/* We have what we want... */
			return len_read;
		}
		
		len_left = len_req - len_read;
		max_left = max - (end-buf);
	}

	/* The reading part of the function, which reads len_left bytes,
	 * and reformats, if max_left is not adequate to fill it up.
	 */

_do_read:
	if (max_left < len_left)
	{
		/* if needed, insert a check here,
		 * for assert(max_left + (buf-start) >= len_left),
		 * but dont do that, as that usually wont happen.
		 */
		/* User error... */
		DEBUG(MPKT, NETLOG_DEBUG1,
			("Overflow in get-next-pkt\n"));

		len_left = max_left;
	}

	/* We have to read some more stuff ... */
	retval = uiio_read(fd, end, len_left, len_left);

#ifdef NOTYET
	/* May cause denial of service attacks, as we may keep on reading packets
	 */
	retval = uiio_read(fd, end, len_left, max_left);
#endif

	if (retval < 0)
	{
		ERROR(MPKT, ("error : uiio_read()" ));
		return retval;
	}
	else
	if (retval == 0)
	{
		goto _done;
	}	
	else
	{
		/* Now set the pointers properly */
		*endp = (end += retval);
	}

	if (len_read < min)
	{
		/* We have yet to complete phase 2 of the 
		 * reading, so set len_read back. Maybe we wont
		 * even have to read anything... 
		 */
		len_read = end - start;
		goto _phase2;
	}

	len_read = end - start;
_done:	
	return len_read;
}

int
get_next_packet_from(
		int fd, 			/* fd on which to get */
		char *buf, 			/* app supplied buffer */
		void **startp, 			/* start in buffer (in/out) */
		void **endp,			/* end in buffer (out) */
		unsigned int (*pkt_len)(void *),/* returns len of incoming packet */
		unsigned int min,		/* min to read before we call pktLen */
		unsigned int max,		/* max size of buffer (read in 1 shot) */
		struct sockaddr_in *from,
		int *fromlen
)
{
	int len_read, len_req, len_left, max_left;
	int retval = -1;
	char *start = *startp, *end = *endp;	

	len_read = end - start;

	if (len_read < min)
	{
		/* We dont even have a min length read in ... */
		len_left = min - len_read;
		max_left = max - (end-buf);

		/* do phase 1 read... then we must return to check
		 * whether phase 2 was completed as well ?
		 */
		goto _do_read;
	}

_phase2:

	if (len_read >= min)
	{
		len_req = pkt_len(start);
		if (len_read >= len_req)
		{
			/* We have what we want... */
			return len_read;
		}
		
		len_left = len_req - len_read;
		max_left = max - (end-buf);
	}

	/* The reading part of the function, which reads len_left bytes,
	 * and reformats, if max_left is not adequate to fill it up.
	 */

_do_read:
	if (max_left < len_left)
	{
		/* User error... */
		DEBUG(MPKT, NETLOG_DEBUG1, 
			("Overflow in get-next-pkt\n"));

		len_left = max_left; 
	}

	/* We have to read some more stuff ... */
	retval = uiio_recvfrom(fd, end, len_left, len_left, from, fromlen);

#ifdef NOTYET
	/* May cause denial of service attacks, as we may keep on reading packets
	 */
	retval = uiio_read(fd, end, len_left, max_left);
#endif

	if (retval < 0)
	{
		ERROR(MPKT, ("error : uiio_read()" ));
		return retval;
	}
	else
	if (retval == 0)
	{
		goto _done;
	}	
	else
	{
		/* Now set the pointers properly */
		*endp = (end += retval);
	}

	if (len_read < min)
	{
		/* We have yet to complete phase 2 of the 
		 * reading, so set len_read back. Maybe we wont
		 * even have to read anything... 
		 */
		len_read = end - start;
		goto _phase2;
	}

	len_read = end - start;
_done:	
	return len_read;
}

int
get_next_packet_from_udp(
		int fd, 			/* fd on which to get */
		char *buf, 			/* app supplied buffer */
		void **startp, 			/* start in buffer (in/out) */
		void **endp,			/* end in buffer (out) */
		unsigned int max,		/* max size of buffer (read in 1 shot) */
		struct sockaddr_in *from,
		int *fromlen
)
{
	int len_read, len_req, len_left, max_left;
	int retval = -1;
	char *start = *startp, *end = *endp;	

	len_read = end - start;

	/* We have to read some more stuff ... */
	retval = uiio_recvfrom(fd, end, 0, max, from, fromlen);

	if (retval < 0)
	{
		ERROR(MPKT, ("error : uiio_read()" ));
		return retval;
	}
	else if (retval == 0)
	{
		goto _done;
	}	
	else
	{
		/* Now set the pointers properly */
		*endp = (end += retval);
	}

	len_read = end - start;
_done:	
	return len_read;
}

int
GisSetUdpSockOpts(int fd)
{
	int i=1, len;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,  
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_REUSEADDR failed errno %d\n",
				errno ));
	}

#if 0
	// This option seems to be available only in nsl
	i = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_DGRAM_ERRIND,  
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_DGRAM_ERRIND failed errno %d\n",
				errno ));
	}
#endif

	len = sizeof(int);
	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
				(char *)&i, &len) < 0)
	{
		NETERROR(MDEF, ("getsockopt SO_SNDBUF failed errno %d\n",
				errno ));
	}

	i=65536;
	i=262144;

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, 
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_SNDBUF failed errno %d\n",
				errno ));
	}

	i=65536;
	i=262144;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, 
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_RCVBUF failed errno %d\n",
				errno ));
	}

	return 0;
}

int
GisSetSockOpts(int fd)
{
	int i=1, len;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,  
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_REUSEADDR failed errno %d\n",
				errno ));
	}

	len = sizeof(int);
	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
				(char *)&i, &len) < 0)
	{
		NETERROR(MDEF, ("getsockopt SO_SNDBUF failed errno %d\n",
				errno ));
	}

	i=65536;

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, 
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_SNDBUF failed errno %d\n",
				errno ));
	}

	i=65536;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, 
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_RCVBUF failed errno %d\n",
				errno ));
	}

	return 0;
}

int
GisSetSockBlock(int fd, int nbio)
{
	int 	flags;

	if (ioctl(fd, FIONBIO, &nbio) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

	if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

	if (nbio)
	{
		flags |= (O_NONBLOCK|O_NDELAY);
	}
	else
	{
		flags &= ~(O_NONBLOCK|O_NDELAY);
	}

	if (fcntl(fd, F_SETFL, flags) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

	return 0;
}
