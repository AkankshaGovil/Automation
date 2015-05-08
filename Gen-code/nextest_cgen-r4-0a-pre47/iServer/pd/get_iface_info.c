#include <ispd.h>
//
//	Description:
//		Modified version of Richard Steven's get_ifi_info.c routines.
//		This version does not change the name of logical interfaces
//		by removing the ":..." portion of the interface. It also gives
//		the true flags of logical interfaces. All interface information
//		is set up in the ifi_info list regardless of whether or not the
//		interface is up or down.
//

void free_iface_info( struct ifi_info *ifihead );

//
//	Function :
//		get_iface_info()
//
//  Arguments       :
//
//		family		Network Address family of interfaces we are
//					interested in - i.e. AF_INET, AF_INET6 ...
//
//		doaliases	flag indicating whether we want logical network
//					interfaces as well as physical.
//
//	Description :
//		Builds a link list of ifi_info structures describing the
//		desired network interfaces. Returns a pointer to the first
//		ifi_info structure in the built list.
//
//	Return Values:
//		ifi_info *	head of built list of ifi_info structures
//
struct ifi_info *
get_iface_info(	int family,
				int doaliases )
{
	struct ifi_info *		ifi;
	struct ifi_info *		ifihead;
	struct ifi_info **		ifipnext;

	int						sockfd;
	int						len;
	int						lastlen;
	int						flags;
	int						myflags;
	int						lerrno;
	char *					ptr;
	char *					buf;
	char					lastname[IFNAMSIZ];

	struct ifconf			ifc;
	struct ifreq *			ifr;
	struct ifreq			ifrcopy;
	struct sockaddr_in *	sinptr;

	if ( ( sockfd = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 )
	{
		lerrno = errno;

		trc_error(	MISPD,
					"GIFI : socket() call failure - errno %d - %s\n",
					lerrno,
					strerror( lerrno ) );
		return( (struct ifi_info*) NULL );
	}

	lastlen = 0;

	len = 100 * sizeof( struct ifreq );	// initial buffer size guess

	for (;;)
	{
		buf = calloc( 1, len );
		ifc.ifc_len = len;
		ifc.ifc_buf = buf;

		if ( ioctl( sockfd, SIOCGIFCONF, &ifc ) < 0 )
		{
			lerrno = errno;

			if (errno != EINVAL || lastlen != 0)
			{
				trc_error(	MISPD,
							"GIFI : ioctl SIOCGIFCONF - errno %d - %s\n",
							lerrno,
							strerror( lerrno ) );
				abort();
			}
		}
		else
		{
			if (ifc.ifc_len == lastlen)
				break;			/* success, len has not changed */
			lastlen = ifc.ifc_len;
		}

		len += 10 * sizeof(struct ifreq);	/* increment */
		free(buf);
	}

	ifihead = NULL;
	ifipnext = &ifihead;
	lastname[0] = 0;

	for ( ptr = buf; ptr < buf + ifc.ifc_len; )
	{
		ifr = (struct ifreq *) ptr;

		#ifdef	HAVE_SOCKADDR_SA_LEN

			len = max(	sizeof(struct sockaddr),
						ifr->ifr_addr.sa_len );

		#else

			switch (ifr->ifr_addr.sa_family)
			{
				#ifdef	IPV6
					case AF_INET6:
						len = sizeof(struct sockaddr_in6);
						break;
				#endif

			case AF_INET:
			default:
				len = sizeof(struct sockaddr);
				break;
			}

		#endif /* HAVE_SOCKADDR_SA_LEN */

		ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */

		if (ifr->ifr_addr.sa_family != family)
			continue;			/* ignore if not desired address family */

		myflags = 0;

		if ( strlen( lastname ) != 0 )
		{
			if ( strncmp( lastname, ifr->ifr_name, strlen( lastname ) ) == 0 )
			{
				if (doaliases == 0)
					continue;		// already processed this interface

				myflags = IFI_ALIAS;
			}
		}

		if (!myflags)
		{
			memcpy( lastname, ifr->ifr_name, IFNAMSIZ );
		}

		ifrcopy = *ifr;

		if ( ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy) < 0 )
		{
			lerrno = errno;

			trc_error(	MISPD,
						"GIFI : ioctl SIOCGIFFLAGS - errno %d - %s\n",
							lerrno,
							strerror( lerrno ) );
		}

		flags = ifrcopy.ifr_flags;

		ifi = calloc( 1, sizeof(struct ifi_info));
		*ifipnext = ifi;		/* prev points to this new one */
		ifipnext = &ifi->ifi_next;	/* pointer to next one goes here */

		ioctl(sockfd, SIOCGIFINDEX, &ifrcopy);

#ifdef ifr_ifindex
		ifi->ifi_index = ifrcopy.ifr_ifindex;
#else
		ifi->ifi_index = ifrcopy.ifr_index;
#endif

		ifi->ifi_flags = flags;	/* IFF_xxx values */
		ifi->ifi_myflags = myflags;	/* IFI_xxx values */
		memcpy( ifi->ifi_name, ifr->ifr_name, IFI_NAME );
		ifi->ifi_name[IFI_NAME - 1] = '\0';

		switch (ifr->ifr_addr.sa_family)
		{
		case AF_INET:
			sinptr = (struct sockaddr_in *) &ifr->ifr_addr;

			if (ifi->ifi_addr == NULL)
			{
				ifi->ifi_addr = calloc( 1, sizeof(struct sockaddr_in) );

				memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));

				#ifdef	SIOCGIFBRDADDR
					if (flags & IFF_BROADCAST)
					{
						if ( ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy) < 0 )
						{
							lerrno = errno;

							trc_error(	MISPD,
										"GIFI : ioctl SIOCGIFBRDADDR "
											"on \"%s\" - errno %d - %s\n",
										lastname,
										lerrno,
										strerror( lerrno ) );
						}

						sinptr = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr;

						ifi->ifi_brdaddr = calloc(	1,
													sizeof(struct sockaddr_in) );

						memcpy(	ifi->ifi_brdaddr,
								sinptr,
								sizeof(struct sockaddr_in) );
					}
				#endif

				#ifdef  SIOCGIFNETMASK
                	ioctl(sockfd, SIOCGIFNETMASK, &ifrcopy);
                	sinptr = (struct sockaddr_in *) &ifrcopy.ifr_addr;

                	ifi->ifi_netmask =
						(struct sockaddr_in *) calloc(	1,
														sizeof(struct sockaddr_in));
                	memcpy(	ifi->ifi_netmask,
							sinptr,
							sizeof(struct sockaddr_in) );
				#endif


				#ifdef	SIOCGIFDSTADDR
					if (flags & IFF_POINTOPOINT)
					{
						if ( ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy) < 0 )
						{
							lerrno = errno;

							trc_error(	MISPD,
										"GIFI : ioctl SIOCGIFDSTADDR on \"%s\""
										" - errno %d - %s\n",
										lastname,
										lerrno,
										strerror( lerrno ) );
						}

						sinptr = (struct sockaddr_in *) &ifrcopy.ifr_dstaddr;

						ifi->ifi_dstaddr = calloc(	1,
													sizeof(struct sockaddr_in));

						memcpy(	ifi->ifi_dstaddr,
								sinptr,
								sizeof(struct sockaddr_in));
					}
				#endif
			}
			break;

		default:
			break;
		}
	}

	free(buf);

	close(sockfd);

	return( ifihead );			/* pointer to first structure in linked list */
}

//
//	Function :
//		free_iface_info()
//
//  Arguments       :
//
//		ifi_head	address of first ifi_info entry to be freed
//
//	Description :
//		Returns the list of ifi_info entries to the gods of free
//		space.
//
//	Return Values:
//		None.
//
void
free_iface_info( struct ifi_info *ifihead )
{
	struct ifi_info *ifi,
	        *ifinext;

	for (ifi = ifihead; ifi != NULL; ifi = ifinext)
	{
		if (ifi->ifi_addr != NULL)
			free(ifi->ifi_addr);

		if (ifi->ifi_brdaddr != NULL)
			free(ifi->ifi_brdaddr);

		if (ifi->ifi_netmask != NULL)
			free(ifi->ifi_netmask );

		if (ifi->ifi_dstaddr != NULL)
			free(ifi->ifi_dstaddr);

		ifinext = ifi->ifi_next;	/* can't fetch ifi_next after free() */

		free(ifi);				/* the ifi_info{} itself */
	}
}
