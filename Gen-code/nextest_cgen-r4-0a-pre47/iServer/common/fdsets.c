/*
 *******************************************************************
 * File:  fdsets.c
 *
 * Copyright (c) 1998, Netoids Inc.
 *******************************************************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <strings.h>

#include "list.h"
#include "netloopP.h"
#include "srvrlog.h"

#ifdef NOFDLOG
#include "nodebuglog.h"
#endif
#include <malloc.h>

/*
 * File and socket descriptors.
 */

static int maxfds = 1024;

int
NetFdsSetMax(int n)
{
	maxfds = n;

	return n;
}

int
NetFdsZero(NetFds *netfds)
{
	FD_ZERO (&netfds->readfds);
	FD_ZERO (&netfds->writefds);
	FD_ZERO (&netfds->exceptfds);

	return( 0 );
}

/*
 * Initializes the 'net' library for main loop control.
 */
int
NetFdsInit (NetFds *netfds)
{
	memset(netfds, 0, sizeof(NetFds));

	/* Init list */
	netfds->netlist = listInit ();

	NetFdsZero(netfds);

	netfds->maxfds = maxfds;

	netfds->pollA = (struct pollfd *)
						malloc(netfds->maxfds*sizeof(struct pollfd));

	if (netfds->maxfds)
	{
		memset(netfds->pollA, 0, netfds->maxfds*sizeof(struct pollfd));
	}

	if (netfds->netlist == (List) 0)
	{
		DEBUG(MDEF, NETLOG_DEBUG1, ("Memory failure...\n"));
		return -1;
	}

	return 0;
}

int
NetFdsCleanup (NetFds *netfds)
{
	return (listDestroy (netfds->netlist));
}


int
NetFdsAdd (NetFds *netfds, int fd, int rw, 
		NetFn readfn, NetFn writefn,
		void * cbdata, void (*destroy)(void *))
{
	NetStruct  *nitem = NULL;

	nitem = (NetStruct *) malloc (sizeof(NetStruct));

	if (! nitem)
		return -1;

	/* Zero it out */
	bzero (nitem, sizeof(NetStruct));

	nitem->fd = fd;
	nitem->rw = rw;
	nitem->readfn = readfn;
	nitem->writefn = writefn;
	nitem->errorfn = 0;
	nitem->cbdata  = cbdata;
	nitem->commision = 1;
	nitem->destroy = destroy;

	DEBUG(MDEF, NETLOG_DEBUG1, 
		("NetFdsAdd:: Commisioning fd %d for main loop\n", fd));

	return (listAddItem (netfds->netlist, (void *)nitem));
}

int
NetFdsSetCbData (NetFds *netfds, int fd, void * cbdata, void (*destroy)(void *))
{
	List	ptr = netfds->netlist;
	NetStruct  *nitem = NULL;

	while (ptr)
	{
		nitem = ptr->item;
		if (nitem && (nitem->fd == fd))
		{
			nitem->destroy = destroy;
			nitem->cbdata = cbdata;
		}

		ptr = ptr->next;
	}

	return( 0 );
}

int
NetFdsRemove (NetFds *netfds, int fd, int rw)
{
	List	ptr = netfds->netlist;
	int	deleted = 0;

	while (ptr)
	{
		NetStruct *nitem = ptr->item;
		if (nitem && (nitem->fd == fd))
		{
			if (nitem->rw == rw)
			{
				if (nitem->destroy)
				{
					(nitem->destroy)(nitem->cbdata);
				}

				listDeleteItem (netfds->netlist, nitem);

				free(nitem);
				deleted = 1;
				break;	/* out of loop */
			}
			else
			{
				if (rw == FD_READ)
					nitem->readfn = 0;
				if (rw == FD_WRITE)
					nitem->writefn = 0;
			}
			break;
		}

		ptr = ptr->next;
	}

	if (deleted)
		return 0;
	else
		/* Not found */
		return -1;
}

int
NetFdsDeactivate(NetFds *netfds, int fd, int rw)
{
	List	ptr = netfds->netlist;

        while (ptr)
        {
             NetStruct *nitem = ptr->item;
             if (nitem && (nitem->fd == fd))
             {
		switch (rw)
		{
			case FD_READ:
				nitem->readfn = 0;
				break;
			case FD_WRITE:
				nitem->writefn = 0;
				break;	
			case FD_RW:
                       		nitem->active = 0;
				nitem->readfn = 0;
				nitem->writefn = 0;
				break;
			default:
				break;
		}
#ifdef _do_selctive_deactivation_
                if (nitem->rw == rw)
#endif
                {
                     nitem->active = 0;
                }

		if (nitem->active)
                {
DEBUG(MDEF, NETLOG_DEBUG1, 
	("NetFdsDeactivate:: fd %d not being deactivated completely\n", fd));
DEBUG(MDEF, NETLOG_DEBUG1, ("NetFdsDeactivate:: %p %p\n", nitem->readfn, nitem->writefn));
                }
                break;
             }
             ptr = ptr->next;
        }
	
	return 0;
}

int
NetFdsUpdate (NetFds *netfds, int fd, int rw, NetFn fn)
{
	List	ptr = netfds->netlist;
	int	updated = 0;

	while (ptr)
	{
		NetStruct *nitem = ptr->item;
		if (nitem && (nitem->fd == fd))
		{
			if (rw == FD_READ)
			{
				nitem->readfn = fn;
			}
			if (rw == FD_WRITE)
			{
				nitem->writefn = fn;
			}
			updated = 1;
		}

		ptr = ptr->next;
	}

	if (updated)
	{
		DEBUG(MDEF, NETLOG_DEBUG4, ("Update successful\n"));
		return 0;
	}
	else
	{
		/* Not found */
		DEBUG(MDEF, NETLOG_DEBUG4, ("Update unsuccessful\n"));
		return -1;
	}
}

int
NetFdsSetup (NetFds *netfds, int module, int level)
{
	List ptr = netfds->netlist, nextPtr;

	DEBUG(module, level, 
		("NetFdsSetup:: %p, %p\n", netfds, ptr));

	while (ptr)
	{
		NetStruct * netitem = (NetStruct *)ptr->item;
		nextPtr = ptr->next;

		if (netitem && (netitem->fd >= 0))
		{
			if (netitem->commision)
			{
				netitem->active = 1;
				netitem->commision = 0;
			}

			if (netitem->active)
			{
				switch (netitem->rw)
				{
				case FD_READ:
					FD_SET (netitem->fd, &netfds->readfds);
					DEBUG(module, level, 
			("SetupFds:: setting up %d for READ\n", netitem->fd));
				break;
				case FD_WRITE:
					FD_SET (netitem->fd, &netfds->writefds);
					DEBUG(module, level, 
			("SetupFds:: setting up %d for WRITE\n", netitem->fd));
				break;
				case FD_RW:
					FD_SET (netitem->fd, &netfds->readfds);
					FD_SET (netitem->fd, &netfds->writefds);
					DEBUG(module, level, 
		("SetupFds:: setting up %d forREAD+WRITE\n", netitem->fd));
				break;
				}
			}
			else
			{
				DEBUG(module, level, 
		("SetUp Fds:: Removing fd %d\n",
						netitem->fd));

				if (netitem->destroy)
				{
					(netitem->destroy)(netitem->cbdata);
				}

				listDeleteItem (netfds->netlist, netitem);

				/* Free this netitem */
				free(netitem);
			}
		}
		ptr = nextPtr;
	}

	return 0;
}

int
NetFdsProcess(NetFds *netfds, int module, int level)
{
	List ptr = netfds->netlist, nextPtr;

	while (ptr)
	{
		NetStruct * netitem = (NetStruct *)ptr->item;
		nextPtr = ptr->next;

		if (netitem && (netitem->active))
		{
			switch (netitem->rw)
			{
			case FD_RW:
			case FD_READ:
				if (FD_ISSET (netitem->fd, &netfds->readfds))
				{
					DEBUG(module, level, 
				("Processing fd %d for READ\n", netitem->fd));
					if (netitem->readfn)
						(netitem->readfn) 
							(netitem->fd, FD_READ, 
							(void *)netitem->cbdata);
				}
				if ((netitem->rw == FD_READ) ||
					(netitem->active == 0))
				{
					break;
				}
			case FD_WRITE:
				if (FD_ISSET (netitem->fd, &netfds->writefds))
				{
					DEBUG(module, level, 
				("Processing fd %d for WRITE\n", netitem->fd));
					if (netitem->writefn)
					(netitem->writefn) (netitem->fd, FD_WRITE, 
						(void *)netitem->cbdata);
				}
			break;
			}
		}

		ptr = nextPtr;
	}

	return 0;
}

int
NetFdsStats(NetFds *netfds, int module, int level )
{
	List ptr = netfds->netlist;

	while (ptr)
	{
		NetStruct * netitem = (NetStruct *)ptr->item;

		if (netitem)
		{
			printf("------------------\n");
			printf("active %d rw %d\n", netitem->active,
				netitem->rw);
			printf("commision %d\n", netitem->commision);
			printf("cbdata %p\n", netitem->cbdata);
			printf("destroy %p\n", netitem->destroy);

			if (netitem->active)
			{
				switch (netitem->rw)
				{
				case FD_RW:
				case FD_READ:
					if (FD_ISSET (netitem->fd, &netfds->readfds))
					{
						printf("fd %d READ %p\n", netitem->fd, netitem->readfn);
					}
					if (netitem->rw == FD_READ)
					{
						break;
					}
				case FD_WRITE:
					if (FD_ISSET (netitem->fd, &netfds->writefds))
					{
						printf("fd %d for WRITE %p\n", netitem->fd, netitem->writefn);
					}
					break;
				default:
					printf("Invalid rw %d\n", netitem->rw);
					break;

				}
			}
		}
		ptr = ptr->next;
	}

	return 0;
}

/* The following function will call select on each of
 * the fds in turn, and once it detects a bad fd,
 * it will return it back to the application.
 * The application MUST deal with the bad fd,
 * On subsequent calls, if the same fd is set again
 * in the masks, it will get reported again
 */
int
NetDetectBadFd(NetFds *netfds, int *result, int module, int level)
{
    List ptr = netfds->netlist, nextPtr;
    fd_set  readfds, writefds;
    static struct timeval tout;
    int retval;

    *result = 0;

    while (ptr)
    {
        NetStruct * netitem = (NetStruct *)ptr->item;
        nextPtr = ptr->next;

        FD_ZERO (&readfds);
        FD_ZERO (&writefds);

        if (netitem)
        {
            if (netitem->active)
            {
                switch (netitem->rw)
                {
                case FD_READ:
                    FD_SET (netitem->fd, &readfds);
                    DEBUG(module, level,
                    ("NetDetectBadFd:: setting up %d/%p for READ\n",
                        netitem->fd, netitem->readfn));
                break;
                case FD_WRITE:
                    FD_SET (netitem->fd, &writefds);
                    DEBUG(module, level,
                    ("NetDetectBadFd:: setting up %d/%p for WRITE\n",
                        netitem->fd, netitem->writefn));
                break;
                case FD_RW:
                    FD_SET (netitem->fd, &readfds);
                    FD_SET (netitem->fd, &writefds);
                    DEBUG(module, level,
                    ("NetDetectBadFd:: setting up %d/%p,%p for RW\n",
                    netitem->fd, netitem->readfn, netitem->writefn));
                break;
                }

                tout.tv_sec = 0;
                tout.tv_usec = 0;

                /* do the select here */
                retval = select ( FD_SETSIZE,
                    &readfds,
                    &writefds,
                    NULL,
                    &tout);

                if (retval < 0)
                {
                    NETDEBUG(module, level,
                    ("NetDetectBadFd: Found a bad fd %d/%p,%p\n",
                    netitem->fd, netitem->readfn, netitem->writefn));

                    *result = 1;

                    return netitem->fd;
                }
            }
            else
            {
                DEBUG(module, level,
                    ("NetDetectBadFd:: Found fd %d/%p,%p, is inactive\n",
                    netitem->fd, netitem->readfn, netitem->writefn));
            }
        }
        ptr = nextPtr;
    }

    return -1;
}

// returns the no of fds ready in the list
int
NetFdsSetupPoll (NetFds *netfds, int module, int level)
{
	List ptr = netfds->netlist, nextPtr;
	int num = 0;

	DEBUG(module, level, 
		("NetFdsSetupPoll:: %p, %p\n", netfds, ptr));

	while (ptr)
	{
		NetStruct * netitem = (NetStruct *)ptr->item;
		nextPtr = ptr->next;

		if (netitem && (netitem->fd >= 0))
		{
			if (netitem->commision)
			{
				netitem->active = 1;
				netitem->commision = 0;
			}

			netfds->pollA[num].events = 0;
			netfds->pollA[num].revents = 0;
			netitem->pollAIndex = 0;

			if (netitem->active)
			{
				switch (netitem->rw)
				{
				case FD_READ:
					DEBUG(module, level, 
					("SetupFds:: setting up %d for READ\n", netitem->fd));
					netfds->pollA[num].fd = netitem->fd;
					netfds->pollA[num].events |=POLLIN;					
					netitem->pollAIndex = num;
					num++;
				break;
				case FD_WRITE:
					DEBUG(module, level, 
					("SetupFds:: setting up %d for WRITE\n", netitem->fd));
					netfds->pollA[num].fd = netitem->fd;
					netfds->pollA[num].events |=POLLOUT;
					netitem->pollAIndex = num;
					num++;
				break;
				case FD_RW:
					DEBUG(module, level, 
					("SetupFds:: setting up %d forREAD+WRITE\n", netitem->fd));
					netfds->pollA[num].fd = netitem->fd;
					netfds->pollA[num].events |=POLLIN;					
					netfds->pollA[num].events |=POLLOUT;
					netitem->pollAIndex = num;
					num++;
				break;
				}
			}
			else
			{
				DEBUG(module, level, ("SetUp Fds:: Removing fd %d\n",
						netitem->fd));

				if (netitem->destroy)
				{
					(netitem->destroy)(netitem->cbdata);
				}

				listDeleteItem (netfds->netlist, netitem);

				/* Free this netitem */
				free(netitem);
			}
		}
		ptr = nextPtr;
	}

	return num;
}

// returns the no of filedes processed.
// If an error happens, returns a negative number
// depicting the error fd.
int
NetFdsProcessPoll(NetFds *netfds, int module, int level)
{
	List ptr = netfds->netlist, nextPtr;
	int n = 0, processed;

	while (ptr)
	{
		NetStruct * netitem = (NetStruct *)ptr->item;
		nextPtr = ptr->next;

		processed = 0;

		if (netitem && (netitem->active))
		{
			switch (netitem->rw)
			{
			case FD_RW:
			case FD_READ:
				if (netfds->pollA[netitem->pollAIndex].revents & 
						(POLLIN|POLLHUP))
				{
					DEBUG(module, level, 
				("Processing fd %d for READ\n", netitem->fd));
					if (netitem->readfn)
						(netitem->readfn) 
							(netitem->fd, FD_READ, 
							(void *)netitem->cbdata);
					processed = 1;
				}
				if ((netitem->rw == FD_READ) ||
					(netitem->active == 0))
				{
					break;
				}
			case FD_WRITE:
				if (netfds->pollA[netitem->pollAIndex].revents & 
						(POLLOUT|POLLERR))
				{
					DEBUG(module, level, 
				("Processing fd %d for WRITE\n", netitem->fd));
					if (netitem->writefn)
					(netitem->writefn) (netitem->fd, FD_WRITE, 
						(void *)netitem->cbdata);

					processed = 1;

					if (netfds->pollA[netitem->pollAIndex].revents & POLLERR)
					{
						// just return
						return -(netfds->pollA[netitem->pollAIndex].fd);
					}
				}
			break;
			}
		}

		n += processed;

		ptr = nextPtr;
	}

	return n;
}
