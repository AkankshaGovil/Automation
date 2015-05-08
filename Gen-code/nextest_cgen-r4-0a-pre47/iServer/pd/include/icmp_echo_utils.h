#ifndef _ICMP_ECHO_UTILS_H_
#define _ICMP_ECHO_UTILS_H_

// Liberally borrowed from "Stevens, Unix Network Programming - Vol. 2"

//  "$Id: icmp_echo_utils.h,v 1.6.2.2 2004/10/14 00:21:09 amar Exp $"

// System include files

#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sched.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#define BUFSIZE			1500

#define MAX_PING_HOSTS	10


typedef enum _echo_status
{
	ECHO_STATUS_UNKNOWN         =  0,
	ECHO_STATUS_UNREACHABLE     =  1,
	ECHO_STATUS_REACHABLE       =  2,
} echo_status_t;

typedef enum _echo_type
{
	ECHO_TYPE_PRIMARY_ROUTER    =  1,
	ECHO_TYPE_SECONDARY_ROUTER  =  2,
	ECHO_TYPE_CTL               =  3,
	ECHO_TYPE_PRIMARY_VIP       =  4,
	ECHO_TYPE_SECONDARY_VIP     =  5,
} echo_type_t;

typedef struct _ping_host
{
	char				host[ STRSZ ];
	struct addrinfo *	target_address;		// address info for endpoint
											//   to ping


	int32_t				pid;				// process id

	char				recvbuf[ BUFSIZE ];
	char				sendbuf[ BUFSIZE ];

	int					datalen;			// # of bytes of data,
											//	following ICMP header

	uint16_t			sentseq;			// Sequence # of last sent packet

	uint16_t			lastreplyseq;		// Sequence # of last reply

	struct	sockaddr	sasend;				// sockaddr() for sending, 
											//	from getaddrinfo

	struct	sockaddr	sarecv;				// sockaddr() for receiving

	int					salen;

	echo_status_t		echo_status;
	echo_status_t		prev_echo_status;

	echo_type_t			echo_type;			// PRIMARY_ROUTER, SECONDARY_ROUTER
											// or CTL
									
	pthread_mutex_t		lock;				// lock used by monitor and send 
											// threads to prevent problems during
											// rollover processing
} ping_host_t;

#if SOLARIS_REL >= 8
	#include <netinet/ip6.h>
	#include <netinet/icmp6.h>
#endif

//
// External interface routines to icmp_echo_utils.c
//

int echo_register(	char*		host,
					echo_type_t	type );

void echo_start( void );

void echo_stop( void );

#endif	// _ICMP_ECHO_UTILS_H_

