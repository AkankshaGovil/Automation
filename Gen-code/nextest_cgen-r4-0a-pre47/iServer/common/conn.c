#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#ifdef SUNOS
#include <sys/filio.h>
#include <sys/sockio.h>
#else
#include <linux/sockios.h>
#endif
#include <malloc.h>
#include "generic.h"
#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"

ConnHandle *
CONN_GetNewHandle(void)
{
	ConnHandle *handle;

	handle = (ConnHandle *)malloc(sizeof(ConnHandle));
	memset(handle, 0, sizeof(ConnHandle));

	return handle;
}

/* Basic function to initiate a connect: synchronous
 * or asynchronous, based on the nblock setting in the
 * evb. The return value will indicate whether the caller
 * should add the fd, or not care about it.
 *
 * retval = -1, and fd < 0, means cant establish.
 * retval = -1, and fd >= 0, means fd needs to be 
 *  	removed from the main fd list.
 * retval = 0, means we have to wait for connection completion.
 * retval = 1, means fd is positive also, and
 * connection is established.
 */
int 
CONN_Connect(ConnHandle *handle)
{
	int sock;
	short rc;

	if (CONN_ConnFd(handle) >= 0)
	{
		NETERROR(MCONN,
			("Cannot establish connection over already active connection\n"));
		return APP_CONNEXISTS;
	}

	/* Start the connection, and install the callback */

     	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
     	{
		PERROR("socket");
	  	CONN_ConnFd(handle) = -1;
		return APP_CONNERROR;
     	}

	DEBUG(MCONN, NETLOG_DEBUG4,
		("CONN_Connect:: Opened socket %d\n", sock));

     	if (CONN_NBlock(handle) == 1)
     	{
		if (ioctl(sock, FIONBIO, (char *)&CONN_NBlock(handle)) < 0)
		{
		     NETERROR(MCONN,
			("ioctl failed to make socket non-blocking\n"));
		}
     	}
     
     	/* Even if the fcntl fails, what's probably going to happen
      	* in the worst case is that we may block. If we do, that's
      	* fine too... ;-)
      	*/
     
     	/* Now attempt a connect */
     	rc = connect(sock, (PROTOID_SOCKADDR *)&(CONN_ConnEntry(handle)->addr), 
			  sizeof(struct sockaddr_in));
     
     	if (rc == 0)
     	{
	  	/* This seems to be really fast, isnt it..
	   	* Well, just set the control fd then... 
	   	*/
	  	CONN_ConnFd(handle) = sock;
	  	return APP_CONNECTED;
     	}
	  
     	/* Ah... this should be happening normally...*/
	if (rc < 0)
	{
		DEBUG(MCONN, NETLOG_DEBUG4,
			("CONNECT returned errno 0x%x\n", errno));
     		if (errno != EINPROGRESS)
     		{
			PERROR("connect");
			close(sock);
			CONN_ConnFd(handle)= -1;
			return APP_CONNERROR;
		}
		
		CONN_ConnFd(handle) = sock;
		return APP_CONNINPROG;	
     	}

	/* Else, print an error and return an error */
	NETERROR(MCONN, 
		("CONNECT returned positive value %d", rc));
	close(sock);
	CONN_ConnFd(handle) = -1;
	return APP_CONNERROR;
}

int
CONN_ConnInProgress(int sock, FD_MODE rw, void *data)
{
	ConnHandle *handle = (ConnHandle *)data;
	int error = 0, len = sizeof(int);
	void *timerdata;

        /* Well looks like something has happened finally
      	* on the signalling channel...
      	*/

	handle->nTries ++;

	/* Delete the timer, if there is one */
	if (CONN_ConnFd(handle))
	{
		DEBUG(MCONN, NETLOG_DEBUG4, 
			("CONN_ConnInProgress:: removed the timer\n"));
		if(timerDeleteFromList(handle->appTimers, 
			CONN_ConnTimer(handle),
			&timerdata))
		{
			if(timerdata)
			{
				free(timerdata);
			}
		}

		CONN_ConnTimer(handle) = 0;
	}

	NetFdsDeactivate(handle->netfds, sock, FD_RW);
	       
	if (sock != CONN_ConnFd(handle))
	{
		NETERROR(MCONN,
			("CONN_ConnInProgress:: Fatal Error (%d %d)\n",
				sock, CONN_ConnFd(handle)));
		close(sock);
		/* No Callback is called here */

		return 0;
	}

     	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, 
		    (char *)&error, &len) < 0)
     	{
		/* Some other internal error maybe */
		PERROR("getsockopt");
		NETERROR(MCONN,
			("in progress error on %d\n", sock));

		close(sock);

		CONN_ConnFd(handle) = -1;

		(handle->userCB)(APP_CONNERROR, handle);
		return 0;
     	}
     	else
	if (error != 0)
	{
		DEBUG(MCONN, NETLOG_DEBUG4,
			("CONN_ConnInProgress:: Error....0x%x on %d\n", 
			error, sock));

		close(sock);

		CONN_ConnFd(handle) = -1;

		(handle->userCB)(APP_CONNERROR, handle);
		return 0;
	}

	DEBUG(MCONN, NETLOG_DEBUG4, 
		("CONN_ConnInProgress:: Got an async CONNECT -------\n"));

	(handle->userCB)(APP_CONNECTED, handle);

     	return 0;
}

int
USER_ConnTimeout(struct Timer *t)
{
	ConnHandle *handle = (ConnHandle *)(t->data);

	/* A registration has timed out 
	 * Kill the previous connection, remove it
	 * from our net fds list, and initiate a new one... 
	 */
DEBUG(MCONN, NETLOG_DEBUG4, ("USER_ConnTimeout:: entering\n"));
	
	/* One try is over here */
	handle->nTries ++;
	close(CONN_ConnFd(handle));
	NetFdsDeactivate(handle->netfds, CONN_ConnFd(handle), FD_RW);
	DEBUG(MCONN, NETLOG_DEBUG4, 
		("USER_ConnTimeout:: Deactivated %d\n", 
			CONN_ConnFd(handle)));

	CONN_ConnFd(handle) = -1;

	/* Timer should be gone... */
	CONN_ConnTimer(handle) = 0;

	((handle->appCB)[APP_CONNTIMEOUT])(APP_CONNTIMEOUT, handle);

	return 0;
}	
	
int
USER_ConnectResponse(int rc, void *data)
{
	ConnHandle *handle = (ConnHandle *)data;
	int nblock = 0;
	void *timerdata;

	/* Handle Response...
	 * Either we have to start a timer, if the
	 * connection is in progress, or either there is 
	 * a valid error, which we must handle, or the
	 * connection has been established, so that we can jump
	 * to the registration state machine
	 */

	switch (rc)
	{	
	case APP_CONNECTED:
		DEBUG(MCONN, NETLOG_DEBUG4,
			("USER_ConnectResponse:: APP_CONNECTED\n"));
		/* Connection has been established. 
		 * kill any timers running...
		 */
		
		/* Kill the timer running on connection */
		if (CONN_ConnTimer(handle))
		{
			if(timerDeleteFromList(handle->appTimers, 
				CONN_ConnTimer(handle),
				&timerdata))
			{
				if(timerdata)
				{
					free(timerdata);
				}
			}

			CONN_ConnTimer(handle) = 0;
		}

		/* First remove the old callback... */
		NetFdsDeactivate(handle->netfds, CONN_ConnFd(handle), FD_RW);

		DEBUG(MCONN, NETLOG_DEBUG4, 
			("REG_ConnectResponse:: Swapping CBS on %d\n", 
			CONN_ConnFd(handle)));
		/* Make the fd blocking also */
		if (ioctl(CONN_ConnFd(handle), FIONBIO, 
				(char *)&nblock) < 0) 
                {
                     NETERROR(MCONN,
			("ioctl failed to make socket blocking\n"));
                }

		((handle->appCB)[APP_CONNECTED])(APP_CONNECTED, handle);

		break;
	case APP_CONNINPROG:
		/* We must wait... 
		 * Start a timer, and add the fd inside our
		 * net fd list.
		 */
		DEBUG(MCONN, NETLOG_DEBUG4, 
			("USER_ConnectResponse:: APP_CONNINPROG\n"));
		if (CONN_ConnTimer(handle))
		{
			NETERROR(MCONN,
			("REG_ConnectResponse:: timer already running\n"));
		}

		DEBUG(MCONN, NETLOG_DEBUG4,
			("USER_ConnectResponse:: starting a timer %lds %ldus\n",
				CONN_ConnEntry(handle)->tmout.it_value.tv_sec,
				CONN_ConnEntry(handle)->tmout.it_value.tv_usec));
		CONN_ConnTimer(handle)= timerAddToList(handle->appTimers,
			&CONN_ConnEntry(handle)->tmout, 
			1, PSOS_TIMER_REL, "Conn", USER_ConnTimeout, 
			handle);

		NetFdsAdd (handle->netfds, CONN_ConnFd(handle), FD_RW, 
			(NetFn) CONN_ConnInProgress, 
	    		(NetFn) CONN_ConnInProgress, handle, 0);
		break;
	case APP_CONNERROR:
	case APP_CONNTIMEOUT:
		NETERROR(MCONN,
			("USER_ConnectResponse:: APP_CONNTIMEOUT/ERROR\n"));
		/* There is a real error. Go to the next server
		 * in the list, and kick off a timer to start
		 * the registration with the new server.
		 * Using a timer here will also keep us free
		 * from stack overflows (in addition to making
		 * sure that if we are retrying with the same
		 * server, we wait for a while)
		 */
		DEBUG(MCONN, NETLOG_DEBUG4,
			("REG_ConnectResponse:: Error... deactivated %d\n", 
				CONN_ConnFd(handle)));
		if (CONN_ConnFd(handle)>= 0)
		{
			/* Must remove this fd from net fds... */
			NetFdsDeactivate(handle->netfds, CONN_ConnFd(handle), FD_RW);
			close(CONN_ConnFd(handle));

			CONN_ConnFd(handle) = -1;
		}

		/* Kill the timer running on connection */
		if (CONN_ConnTimer(handle))
		{
			DEBUG(MCONN, NETLOG_DEBUG4,
				("Deleting the timer on conn...\n"));

			if(timerDeleteFromList(handle->appTimers, 
				CONN_ConnTimer(handle),
				&timerdata))
			{
				if(timerdata)
				{
					free(timerdata);
				}
			}

			CONN_ConnTimer(handle)= 0;
		}
			
		((handle->appCB)[rc])(rc, handle);

		break;
	case APP_CONNEXISTS:
		/* Handle this if needed */
		break;

	default:
		NETERROR(MCONN,
			("Invalid rc valud %d from CONN\n", rc));
		break;	
	}
	
	return 0;
}
