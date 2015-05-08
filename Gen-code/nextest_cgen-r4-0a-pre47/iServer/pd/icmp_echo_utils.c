// Liberally borrowed from "Stevens, Unix Network Programming - Vol. 2"

//  "$Id: icmp_echo_utils.c,v 1.6.2.10 2004/05/03 19:09:34 ptyagi Exp $"

#include <ispd.h>
#include <icmp_echo_utils.h>

//
// Array of ping_host_t structures that can be registered.
//

static int			ping_host_count;
static int			max_failed_echoes = 5;		// maximum # of sequential failed
												// echoes before vip is brought 
												// down

static int			icmp_interval = 200;		// interval between checks
												// interval between sends

#define ISPD_MAX_ICMP_SEQ 8192

static ping_host_t	ping_hosts[ MAX_PING_HOSTS ];
static int			icmp_sockfd;
static pid_t		icmp_pid;
static int			icmpproto = IPPROTO_ICMP;
static socklen_t	salen;

static pthread_t	icmp_sender_thread;
static pthread_t	icmp_receiver_thread;
static pthread_t	icmp_monitor_thread;

//	Local Function prototypes

static void send_icmp_echo_v4( ping_host_t * pr );

static void process_resp_v4(	char *				ptr,
								struct	sockaddr *	recvaddr,
								ssize_t				len,
								struct	timeval	*	tvrecv );

static uint16_t in_cksum(	uint16_t *	addr,
							int			len );

static struct addrinfo * host_serv( const char* hostname,
            						const char* service,
            						int         family,
            						int         socktype );

static void tv_sub(	struct timeval *	out,
					struct timeval *	in );

static int Socket(	int	family,
					int	type,
					int	protocol );

static char *		icmp_type_string(	uint32_t	type,
										char **		result );

static char * sock_ntop_host(	const struct sockaddr *	sa,
								socklen_t				salen );

static void * icmp_echo_send( void* args );

static void * icmp_echo_receive( void* args );

static void * icmp_echo_monitor( void* args );

//
//	Function:
//		echo_register()
//
//	Argument:
//		host		character string of hostname or x.x.x.x
//					ip address
//
//		type		specifies interface over which echo traffic
//					is passing.
//
//	Description:
//			Register a specified host for ping processing.
//		Up to MAX_PING_HOSTS names can be registered
//		with the ping utilities. The hosts are monitored
//		via ICMP ECHO REQUESTs to make sure that they are
//		reachable.
//			All hosts to be pinged should be registered
//		prior to starting the echoing process via
//		echo_start().
//
int
echo_register(	char*		host,
				echo_type_t	type )
{

	ping_host_t	* ph_ptr;

	if ( ping_host_count >= ( MAX_PING_HOSTS - 1 ) )
	{
		trc_error(	MISPD,
					"ICMP : echo_register() : Maximum hosts, %d, "
					"already specified\n",
					ping_host_count );
		return( -1 );
	}

	if ( strlen(host) == 0 )
	{
		return( -1 );
	}

	ph_ptr = &ping_hosts[ ping_host_count ];

	ph_ptr->datalen = 56;
	ph_ptr->target_address = host_serv( host, NULL, 0, 0 );
	strcpy( ph_ptr->host, host );
	salen = ph_ptr->target_address->ai_addrlen;
	ph_ptr->salen = ph_ptr->target_address->ai_addrlen;
	ph_ptr->echo_status = ECHO_STATUS_UNKNOWN;
	ph_ptr->echo_type = type;

	if ( pthread_mutex_init( &ph_ptr->lock, NULL ) )
	{
		trc_error(	MISPD,
					"ICMP : echo_register(): pthread_mutex_init() failed\n" );
		return(-1);
	}

	memset( &ph_ptr->sasend, (int) 0, salen );

	memcpy( &ph_ptr->sasend, ph_ptr->target_address->ai_addr, ph_ptr->salen );


	trc_debug(	MISPD, NETLOG_DEBUG1,
				"ICMP : echo_register(): host %s\n",
				sock_ntop_host( ph_ptr->target_address->ai_addr,
								ph_ptr->target_address->ai_addrlen ));
	
	ping_host_count++;
	return( 0 );
}

//
//	Function:
//		echo_start()
//
//	Argument:
//		None.
//
//	Description:
//			Starts echo service processing, monitoring
//		registered hosts via the ICMP ECHO service.
//			This routine starts three threads that
//		monitor the state of the links between the host
//		on which the program is running and the registered
//		hosts.
//
void
echo_start( void )
{

	int					size;
	static	int			debug = 1;
	char				description[ MAX_DESCR_SIZE ];

	trc_debug(	MISPD, NETLOG_DEBUG1,
				"ICMP : echo_start() :  called for %d hosts\n",
				ping_host_count );

	icmp_pid = getpid();

	if ( ( icmp_sockfd = Socket( AF_INET, SOCK_RAW, icmpproto ) ) == - 1 )
	{
		trc_error(	MISPD,
					"echo_start() : Unable to open raw socket for "
					"ICMP - exiting\n" );
		exit(0);
	}

	setuid( getuid() );

	size = 60 * 1024;
	
	setsockopt( icmp_sockfd, SOL_SOCKET, SO_RCVBUF, &size,  sizeof( size ) );

	setsockopt( icmp_sockfd, SOL_SOCKET, SO_DEBUG, &debug,  sizeof( debug ) );

	sprintf( description, "icmp_sender_thread()" );

	thread_create( &icmp_sender_thread,
					icmp_echo_send,
					(void*) NULL,
					description,
					0 );

	sprintf( description, "icmp_receiver_thread()" );

	thread_create( &icmp_receiver_thread,
					icmp_echo_receive,
					(void*) NULL,
					description,
					0 );

	sprintf( description, "icmp_monitor_thread()" );

	thread_create( &icmp_monitor_thread,
					icmp_echo_monitor,
					(void*) NULL,
					description,
					0 );
}

//
//	Function:
//		echo_stop()
//
//	Argument:
//		None.
//
//	Description:
//		Stops ICMP Echo Service processing.
//
void
echo_stop( void )
{
	pthread_cancel( icmp_monitor_thread );
	pthread_cancel( icmp_sender_thread );
	pthread_cancel( icmp_receiver_thread );
	return;
}

//
// Local function definitions
//

//
//	Function:
//		icmp_echo_monitor()
//
//	Description:
//		This function is started as a thread to
//		monitor the health of connections via
//		reachability of hosts via ICMP ECHO requests
//		and responses.
//
static void *
icmp_echo_monitor( void* args )
{
	thread_descr_t*		thread_descr;
	int					cur_host;
	ping_host_t *		ph_ptr;
	ispd_event_entry_t	event;

	thread_descr = thread_register( args );

	for (;;) {
		for ( cur_host = 0; cur_host < ping_host_count; cur_host++ ) {
			ph_ptr = &ping_hosts[ cur_host ];

			memset( &event, (int32_t) 0, sizeof( ispd_event_entry_t ) );
		
			switch( ph_ptr->echo_type )
			{
			case ECHO_TYPE_PRIMARY_ROUTER:
				event.type = ISPD_EVENT_PRIMARY_ROUTER_ECHO_STATUS;
				break;
			case ECHO_TYPE_SECONDARY_ROUTER:
				event.type = ISPD_EVENT_SECONDARY_ROUTER_ECHO_STATUS;
				break;
			case ECHO_TYPE_CTL:
				event.type = ISPD_EVENT_CTL_IFACE_ECHO_STATUS;
				break;
			case ECHO_TYPE_PRIMARY_VIP:
				event.type = ISPD_EVENT_PRIMARY_VIP_ECHO_STATUS;
				break;
			case ECHO_TYPE_SECONDARY_VIP:
				event.type = ISPD_EVENT_SECONDARY_VIP_ECHO_STATUS;
				break;
			}

			lock_mutex( &ph_ptr->lock );

			switch( ph_ptr->echo_status )
			{
			case ECHO_STATUS_UNKNOWN:
				if ( ph_ptr->sentseq > max_failed_echoes ) {
					if ( abs( ph_ptr->sentseq - ph_ptr->lastreplyseq ) > max_failed_echoes ) {
						ph_ptr->echo_status = (echo_status_t) event.value = ECHO_STATUS_UNREACHABLE;

						if ( ph_ptr->prev_echo_status != ECHO_STATUS_UNREACHABLE ) {
							trc_debug(	MISPD, NETLOG_DEBUG2,
								"ICMP : link to host, %s, detected down - sentseq %d, replyseq %d - "
								"status ROLLOVER\n",
								sock_ntop_host( &ph_ptr->sasend,
												ph_ptr->salen ),
								ph_ptr->sentseq,
								ph_ptr->lastreplyseq );
							strcpy( event.host, ph_ptr->host );
							ispd_q_send( ispd_event_qid, &event );
						}
					} else {
						ph_ptr->echo_status = (echo_status_t) event.value = ECHO_STATUS_REACHABLE;

						if ( ph_ptr->prev_echo_status != ECHO_STATUS_REACHABLE ) {
							trc_debug(	MISPD, NETLOG_DEBUG2,
								"ICMP : link to host, %s, detected up   - sentseq %d, replyseq %d - "
								"status ROLLOVER\n",
								sock_ntop_host( &ph_ptr->sasend,
												ph_ptr->salen ),
								ph_ptr->sentseq,
								ph_ptr->lastreplyseq );
							strcpy( event.host, ph_ptr->host );
							ispd_q_send( ispd_event_qid, &event );
						}
					}
				}
				break;

			case ECHO_STATUS_REACHABLE:
				if ( abs( ph_ptr->sentseq - ph_ptr->lastreplyseq ) > max_failed_echoes ) {
					trc_debug(	MISPD, NETLOG_DEBUG1,
								"ICMP : access to host, %s, no longer available\n",
								sock_ntop_host( &ph_ptr->sasend,
												ph_ptr->salen ) );

					ph_ptr->echo_status = (echo_status_t) event.value = ECHO_STATUS_UNREACHABLE;
					strcpy( event.host, ph_ptr->host );
					ispd_q_send( ispd_event_qid, &event );
				}
				break;

			case ECHO_STATUS_UNREACHABLE:
				if ( abs( ph_ptr->sentseq - ph_ptr->lastreplyseq ) <= max_failed_echoes ) {
					trc_debug(	MISPD, NETLOG_DEBUG1,
								"ICMP : access to host, %s, available\n",
								sock_ntop_host( &ph_ptr->sasend,
												ph_ptr->salen ) );
					ph_ptr->echo_status = (echo_status_t) event.value = ECHO_STATUS_REACHABLE;
					strcpy( event.host, ph_ptr->host );
					ispd_q_send( ispd_event_qid, &event );
				}
				break;

			default:
				trc_error(	MISPD,
							"ICMP : Unexpected current state %d for host %s\n",
							sock_ntop_host( &ph_ptr->sasend,
											ph_ptr->salen ));
				break;
			}

			unlock_mutex( &ph_ptr->lock );
		}

		millisleep(icmp_interval);
	}

	thread_exit(thread_descr->transient);
	return((void*) NULL );
}

//
//	Function:
//		icmp_echo_send()
//
//	Description:
//		This function is started as a thread to
//		send ICMP ECHO Requests to hosts registered
//		via echo_register(). It loops through the
//		ping_hosts[] array sending an echo request
//		to each host in the list then sleeps for
//		a specified time interval and repeats the
//		process until killed.
//
static void *
icmp_echo_send( void* args )
{
	thread_descr_t*	thread_descr;
	int				i;

	thread_descr = thread_register( args );

	for (;;)
	{
		for ( i = 0; i < ping_host_count; i++ )
		{
			send_icmp_echo_v4( &ping_hosts[i] );
		}

		millisleep(icmp_interval);
	}

	thread_exit(thread_descr->transient);
	return((void*) NULL );
}

//
//	Function:
//		icmp_echo_receive()
//
//	Description:
//		This function is started as a thread to
//		receive ICMP ECHO Replies to hosts registered
//		via echo_register(). It loops receiving
//		responces and updating the ping_hosts[] array
//		element associated with the responce.
//
static void *
icmp_echo_receive( void* args )
{
	thread_descr_t*		thread_descr;
	char				recvbuf[ BUFSIZE ];
	socklen_t			addrlen;
	struct	sockaddr	recvaddr;	
	struct timeval		current_tv;
	ssize_t				recvsize;

	thread_descr = thread_register( args );

	for (;;)
	{
		addrlen = salen;

		recvsize = recvfrom(	icmp_sockfd,
								recvbuf,
								sizeof( recvbuf ),
								0,
								&recvaddr,
								&addrlen );

		if ( recvsize < 0 )
		{
			if ( errno == EINTR )
				continue;
			else
			{
				int lerrno = errno;

				trc_error(	MISPD,
							"ICMP : icmp_echo_receive() : error : recvfrom error"
							" - errno %d - %s\n",
							lerrno,
							strerror( lerrno ) );
				continue;
			}
		}

		gettimeofday( &current_tv, NULL );

		process_resp_v4( recvbuf, &recvaddr, recvsize, &current_tv );
	}

	thread_exit(thread_descr->transient);
	return((void*) NULL );
}

static void
send_icmp_echo_v4( ping_host_t * pr )
{
	int				len;
	struct icmp *	icmp;

	icmp = (struct icmp *) pr->sendbuf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = icmp_pid;

	// Is next seq # to send greater or equal to ISPD_MAX_ICMP_SEQ - rollover situation ?

	if ( pr->sentseq >= ISPD_MAX_ICMP_SEQ )
	{
		// Yes, wrap back to 0 and set status to UNKNOWN
		// to let lastreplyseq catch up, so we dont fail
		//
		// Make sure monitor thread does not due any evaluation
		// during the assignments by acquiring mutex lock prior
		// to assignments

		lock_mutex( &pr->lock );

		icmp->icmp_seq = 0;				// sequence # to send this time
		pr->sentseq = 1;				// next sequence # to send
		pr->prev_echo_status = pr->echo_status;
		pr->echo_status = ECHO_STATUS_UNKNOWN;

		unlock_mutex( &pr->lock );
	}
	else
	{
		// No, update seq # and increment pr->sentseq so it
		// contains next seq number to send
		
		icmp->icmp_seq = pr->sentseq++;
	}

	gettimeofday( (struct timeval *) icmp->icmp_data, NULL );

	len = 8 + pr->datalen;

	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum( ( u_short *) icmp, len );

	sendto(	icmp_sockfd,
			pr->sendbuf,
			len,
			0,
			&pr->sasend,
			pr->salen );

	trc_debug(	MISPD, NETLOG_DEBUG2,
		"ICMP SEND : %d bytes to   %s: seq=%u\n",
					len,
					sock_ntop_host( &pr->sasend, pr->salen ),
					icmp->icmp_seq );
}

static void
process_resp_v4(	char *				ptr,
					struct	sockaddr *	recvaddr,
					ssize_t				len,
					struct	timeval	*	tvrecv )
{
	int					hlen1, icmplen;
	struct ip *			ip;
	struct icmp *		icmp;
	struct timeval *	tvsend;
	int					found = 0, cur_host;
	ping_host_t *		ph_ptr;
	char *				resstr;

	ip = ( struct ip * ) ptr;	// start of IP header
	hlen1 = ip->ip_hl << 2;		// length of IP header

	icmp = ( struct icmp * ) ( ptr + hlen1 );	// start of ICMP header

	//
	// Find the host entry for the host which the reply came from.
	// If not found disregard reply.
	//

	for ( cur_host = 0; cur_host < ping_host_count; cur_host++ )
	{
		ph_ptr = &ping_hosts[ cur_host ];

		if ( !memcmp( recvaddr, &ph_ptr->sasend, ph_ptr->salen ) )
		{
			found = 1;
			break;
		}
	}

	if ( found )
	{
		if ( ( icmplen = len - hlen1 ) < 8 )
		{
			trc_error(	MISPD, 
						"ICMP : process_resp_v4() : error : icmplen (%d) < 8\n",
						icmplen );
			return;
		}

		if ( icmp->icmp_type == ICMP_ECHOREPLY )
		{
			// Did we send this packet - does it have our pid as the id ?

			if ( icmp->icmp_id != icmp_pid )
				return;	// No, forget it

			if ( icmplen < 16 )
			{
				trc_error(	MISPD, 
							"ICMP : process_resp_v4() : error : icmplen (%d) < 16\n",
							icmplen );
				return;
			}

			// OK, we think we got a reply

			ph_ptr->lastreplyseq = icmp->icmp_seq;	// save the seq # from
													// reply for comparison
													// in monitor thread

			tvsend = (struct timeval *) icmp->icmp_data;// Get timeval data
														// sent in the
														// request and 
														// returned in the
														// reply to measure
														// the roundtrip
														// latency

			// Calculate difference between timeval at time of send
			// and timeval gotten when echo reply was received

			tv_sub( tvrecv, tvsend );

			trc_debug(	MISPD,
						NETLOG_DEBUG2,
						"ICMP RECV : %d bytes from %s: seq=%u, ttl=%d\n",
						icmplen,
						sock_ntop_host( recvaddr, salen ),
						icmp->icmp_seq,
						ip->ip_ttl );
		}
		else
		{
			trc_debug(	MISPD,
						NETLOG_DEBUG4,
						"ICMP : Not ICMP_ECHO_REPLY : %d bytes from %s: "
						"type %s, code %d\n",
						icmplen,
						sock_ntop_host( recvaddr, salen ),
						icmp_type_string( icmp->icmp_type, &resstr ),
						icmp->icmp_code );
		}
	}

	return;
}

static uint16_t
in_cksum(	uint16_t *	addr,
			int			len )
{
	int			nleft = len;
	int			sum = 0;
	uint16_t *	w = addr;	
	uint16_t 	answer = 0;

	//
	// Our algorithm is simple, using a 32-bit accumulator (sum),
	// we add sequential 16 bit words to it, and at the end, fold
	// back all the carry bits from the top 16 bits into the lower 
	// 16 bits.
	//

	while ( nleft > 1 )
	{
		sum += *w++;
		nleft -= 2;
	}

	// mop up an odd byte, if needed

	if ( nleft == 1 )
	{
		*( uint8_t * ) (&answer) = *( uint8_t * ) w;
		sum += answer;
	}

	// add back carry outs from top 16 bits to low 16 bits

	sum = ( sum >> 16 ) + ( sum & 0xffff );		// add hi 16 to low 16
	sum += ( sum >> 16 );						// add carry
	answer = ~sum;								// truncate to 16 bits
	
	return( answer );

}

static void
tv_sub(	struct timeval *	out,
		struct timeval *	in )
{

	if ( ( out->tv_usec -= in->tv_usec ) < 0 ) 
	{
		--out->tv_sec;
		out->tv_usec += 1000000;
	}

	out->tv_sec -= in->tv_sec;
}

static struct addrinfo *
host_serv(  const char* hostname,
            const char* service,
            int         family,
            int         socktype )
{
	int                 n;
	struct  addrinfo    hints, * res;

	memset( &hints, (int) 0, sizeof( struct addrinfo ) );

	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = family;
	hints.ai_socktype = socktype;

	if ( ( n = getaddrinfo( hostname, service, &hints, &res )) != 0 )
		return( NULL );

	return( res );
}

static char *
sock_ntop_host( const struct sockaddr *		sa,
				socklen_t					salen )
{

	char			portstr[7];
	static	char	str[128];

	switch( sa->sa_family )
	{
	case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in *) sa;

			if ( inet_ntop( AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL )
				return( NULL );

			if ( ntohs( sin->sin_port ) != 0 ) 
			{
				snprintf(	portstr,
							sizeof( portstr ),
							".%d",
							ntohs( sin->sin_port));
				strcat( str, portstr);
			}
			return( str );
		}
		break;
	default:
		return( NULL );
		break;
	}
}

static int
Socket( int	family,
		int	type,
		int	protocol )
{
	int n;

	if ( ( n = socket( family, type, protocol ) ) < 0 )
	{
		int lerrno = errno;

		trc_error(	MISPD,
					"ICMP : Socket() : socket error - errno %d - %s\n",
					lerrno,
					strerror(lerrno) );

		return( -1 );
	}
	return( n );
}

//
//	Function:
//		icmp_type_string()
//
//	Argument:
//
//		type		interger type value of icmp operation
//
//		result		character pointer in which to return result
//
//	Description:
//			Convert icmp type string into meaningful char string.
//			Returns string value of icmp type input.
//
static char *
icmp_type_string(	uint32_t	type,
					char **		result )
{

	static	char	tmpstr[100];

	switch( type )
	{
	case ICMP_ECHOREPLY:
		*result = 	"ECHO REPLY        (5) ";
		break;
	case ICMP_UNREACH:
		*result = 	"UNREACHABLE       (5) ";
		break;
	case ICMP_SOURCEQUENCH:
		*result = 	"SOURCE QUENCH     (5) ";
		break;
	case ICMP_REDIRECT:
		*result = 	"REDIRECT          (5) ";
		break;
	case ICMP_ECHO:
		*result = 	"ECHO REQUEST      (8) ";
		break;
	case ICMP_ROUTERADVERT:
		*result = 	"ROUTER ADVERTISE  (9) ";
		break;
	case ICMP_ROUTERSOLICIT:
		*result =	"ROUTER SOLICIT    (10)";
		break;
	case ICMP_TIMXCEED:
		*result =	"TIME EXCEED       (11)";
		break;
	case ICMP_PARAMPROB:
		*result =	"INVALID PARAM     (12)";
		break;
	case ICMP_TSTAMP:
		*result =	"TIMESTAMP REQ     (13)";
		break;
	case ICMP_TSTAMPREPLY:
		*result =	"TIMESTAMP REPLY   (14)";
		break;
	case ICMP_IREQ:
		*result =	"INFO REQUEST      (15)";
		break;
	case ICMP_IREQREPLY:
		*result =	"INFO REPLY        (16)";
		break;
	case ICMP_MASKREQ:
		*result =	"MASK REQUEST      (18)";
		break;
	case ICMP_MASKREPLY:
		*result =	"MASK REPLY        (18)";
		break;
	default:
		sprintf(	tmpstr,
					"Unknown           (%2d}",
					type );
		*result = tmpstr;
		break;
	}

	return( *result );
}
