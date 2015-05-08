

/*
 * net.c
 *
 *	Copyright 1998, Netoids Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <strings.h>

#include "net.h"
#include "srvrlog.h"

/* Server prototypes */
#include "server.h"

#include "lsconfig.h"
#include "addrtype.h"
#include "nxosd.h"

/*
 * Returns the server socket created 
 */
int
srvInitNetTCPPort (unsigned short portnumber)
{
	int	retval = 0;
	struct sockaddr_in myaddr;
	int	sockfd = 0;

	/* Create Socket */
	sockfd = socket (AF_INET, SOCK_STREAM, 0 );

	if (sockfd < 0)
	{
		perror ("Unable to create stream socket ");
		return -1;
	}

	bzero ((char *)&myaddr, sizeof(myaddr));  /*  Zeroes the struct */

	myaddr.sin_family = AF_INET;
	myaddr.sin_port  = htons (portnumber);
	myaddr.sin_addr.s_addr  = htonl (INADDR_ANY);

	/* Bind */
	retval = bind (sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	
	if ( retval < 0 )
	{
		perror ("Unable to bind socket");
		return -1;
	}

	/* Listen */
	retval = listen (sockfd, 10);
	if ( retval < 0 )
	{
		perror ("Unable to listen on socket");
		return -1;
	}

	/* Return the socket descriptor */
	return (sockfd);
}


// returns host byte order
unsigned long
ResolveDNS(char *hostname, int *herror)
{
	char fn[] = "ResolveDNS():";
	static time_t dns_failed = 0;
	static pthread_mutex_t dnsFailedMutex = PTHREAD_MUTEX_INITIALIZER;
	int addr_type;
	struct hostent hostentry, *hostp;
	char buffer[256];
	long ipaddr;
	char *hostname2;

	if((addr_type = DetermineAddrType(hostname)) == IPADDR)
	{
		return(ntohl(inet_addr(hostname)));
	}
	else
	{
		if(!dns_failed)
		{
_retry:
			switch(addr_type)
			{
				case HOSTNAME:
				case ABS_DN:
					hostp = nx_gethostbyname_r(hostname, &hostentry, buffer, 256, herror);
					break;

				case DN:
					if((hostname2 = malloc(strlen(hostname) + 2)) != NULL)
					{
						sprintf(hostname2, "%s.", hostname);
						hostp = nx_gethostbyname_r(hostname2, &hostentry, buffer, 256, herror);
						free(hostname2);
					}
					else
					{
						NETERROR(MFIND, ("%s malloc failed\n", fn));
						*herror = NO_RECOVERY;
						return -1;
					}
					break;
		
				default:
					NETERROR(MFIND, ("%s invalid addr type\n", fn));
					*herror = NO_RECOVERY;
					return -1;
			}

			if (hostp)
			{
				ipaddr = ntohl(*(int *)hostp->h_addr_list[0]);
			}
			else
			{
				ipaddr = -1;

				if(*herror == TRY_AGAIN || *herror == NO_RECOVERY)
				{
					NETERROR(MFIND, ("%s DNS failed\n", fn));

					pthread_mutex_lock(&dnsFailedMutex);
					dns_failed = time(0) + dns_recovery_timeout;
					pthread_mutex_unlock(&dnsFailedMutex);
				}
			}

			return ipaddr;
		}
		else
		{
			if(difftime(dns_failed, time(0)) > 0)
			{
				*herror = TRY_AGAIN;
				return -1;
			}
			else
			{
				pthread_mutex_lock(&dnsFailedMutex);
				dns_failed = 0;
				pthread_mutex_unlock(&dnsFailedMutex);
				goto _retry;
			}
		}
	}
}
