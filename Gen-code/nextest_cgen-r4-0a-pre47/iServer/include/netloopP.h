#ifndef _net_P_h_
#define _net_P_h_

#include "netloop.h"

/*
 * NetStruct
 */
typedef struct _netstruct
{
	 int	fd;
	 int	commision;
	 int	active;
	 int	rw;
	 NetFn	readfn;
	 NetFn	writefn;
	 NetFn	errorfn;
	 void *  cbdata;				    /* call back data for the read */
	 void	(*destroy)(void *);     /* to free cbdata.. */

	 // optimization: keep track of index in the poll array
	 int	pollAIndex;
} NetStruct;

typedef struct
{
	// for select functionality
	 fd_set 	readfds;
	 fd_set 	writefds;
	 fd_set 	exceptfds;

	// for poll functionality
	int 	maxfds;
	struct pollfd *pollA;

	 List 	netlist;
	 int	numready;
} NetFds;		

#endif	/* _net_P_h_ */
