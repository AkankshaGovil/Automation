#include "unpifi.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <netinet/in.h>
#include <unistd.h>
#ifndef ifr_index
#define ifr_index ifr_ifindex
#endif
struct ifi_info *
get_ifi_info(int family, int doaliases)
{
	struct ifi_info	*ifi, *ifihead, **ifipnext;
	int sockfd, len, lastlen, flags, myflags;
	char *ptr, *buf, lastname[IFNAMSIZ], *cptr;
	struct ifconf		ifc;
	struct ifreq		*ifr, ifrcopy;
	struct sockaddr_in	*sinptr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	lastlen = 0;
	len = 100 * sizeof(struct ifreq);	/* initial buffer size guess */
	for ( ; ; ) {
		buf = (char *)malloc(len);
		ifc.ifc_len = len;
		ifc.ifc_buf = buf;
		if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
			if (errno != EINVAL || lastlen != 0)
				perror("ioctl error");
		} else {
			if (ifc.ifc_len == lastlen)
				break;		/* success, len has not changed */
			lastlen = ifc.ifc_len;
		}
		len += 10 * sizeof(struct ifreq);	/* increment */
		free(buf);
	}
	ifihead = NULL;
	ifipnext = &ifihead;
	lastname[0] = 0;

	for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
		ifr = (struct ifreq *) ptr;

#ifdef	HAVE_SOCKADDR_SA_LEN
		len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
#else
		switch (ifr->ifr_addr.sa_family) {
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
#endif	/* HAVE_SOCKADDR_SA_LEN */
		ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */

		if (ifr->ifr_addr.sa_family != family)
			continue;	/* ignore if not desired address family */

		myflags = 0;
		if ( (cptr = strchr(ifr->ifr_name, ':')) != NULL)
			*cptr = 0;		/* replace colon will null */
		if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0) {
			if (doaliases == 0)
				continue;	/* already processed this interface */
			myflags = IFI_ALIAS;
		}
		memcpy(lastname, ifr->ifr_name, IFNAMSIZ);

		ifrcopy = *ifr;


		ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
		flags = ifrcopy.ifr_flags;

		if ((flags & IFF_UP) == 0)
			continue;	/* ignore if interface not up */

		ifi = ( struct ifi_info *)calloc(1, sizeof(struct ifi_info));
		*ifipnext = ifi;			/* prev points to this new one */
		ifipnext = &ifi->ifi_next;	/* pointer to next one goes here */

		ioctl(sockfd, SIOCGIFINDEX, &ifrcopy);

		ifi->ifi_index = ifrcopy.ifr_index;

		ifi->ifi_flags = flags;		/* IFF_xxx values */
		ifi->ifi_myflags = myflags;	/* IFI_xxx values */
		memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
		ifi->ifi_name[IFI_NAME-1] = '\0';

		switch (ifr->ifr_addr.sa_family) {
		case AF_INET:
			sinptr = (struct sockaddr_in *) &ifr->ifr_addr;
			if (ifi->ifi_addr == NULL) {
				ifi->ifi_addr = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
				memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));

#ifdef	SIOCGIFBRDADDR
				if (flags & IFF_BROADCAST) {
					ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
					sinptr = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr;
					ifi->ifi_brdaddr = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
					memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));
				}
#endif

#ifdef	SIOCGIFNETMASK
				ioctl(sockfd, SIOCGIFNETMASK, &ifrcopy);
				sinptr = (struct sockaddr_in *) &ifrcopy.ifr_addr;
				ifi->ifi_netmask = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
				memcpy(ifi->ifi_netmask, sinptr, sizeof(struct sockaddr_in));
#endif

#ifdef	SIOCGIFDSTADDR
				if (flags & IFF_POINTOPOINT) {
					ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy);
					sinptr = (struct sockaddr_in *) &ifrcopy.ifr_dstaddr;
					ifi->ifi_dstaddr = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
					memcpy(ifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in));
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
	return(ifihead);	/* pointer to first structure in linked list */
}

void
free_ifi_info(struct ifi_info *ifihead)
{
	struct ifi_info	*ifi, *ifinext;

	for (ifi = ifihead; ifi != NULL; ifi = ifinext) {
		if (ifi->ifi_addr != NULL)
			free(ifi->ifi_addr);
		if (ifi->ifi_netmask != NULL)
			free(ifi->ifi_netmask);
		if (ifi->ifi_brdaddr != NULL)
			free(ifi->ifi_brdaddr);
		if (ifi->ifi_dstaddr != NULL)
			free(ifi->ifi_dstaddr);
		ifinext = ifi->ifi_next; /* can't fetch ifi_next after free() */
		free(ifi);	/* the ifi_info{} itself */
	}
}


//
//	Description:
//		Modified version of Richard Steven's get_ifi_info.c routines.
//		This version does not change the name of logical interfaces
//		by removing the ":..." portion of the interface. It also gives
//		the true flags of logical interfaces. All interface information
//		is set up in the ifi_info list regardless of whether or not the
//		interface is up or down.
//
//
//	Function :
//		get_ifi_info2()
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
get_ifi_info2(	int family,
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
		return( (struct ifi_info*) NULL );
	}

	lastlen = 0;

#ifdef SIOCGIFNUM		
	if (ioctl( sockfd, SIOCGIFNUM, &lastlen ) < 0)
	{
		close(sockfd);
		abort();
	}
	len = lastlen* sizeof( struct ifreq );	// initial buffer size guess
#else
	// SIOCGIFCOUNT is not correctly implemented. It returns EINVAL
	// SIOCGIFCONF implements same functionality when the ifc_len and ifc_buf are set to 0
		ifc.ifc_len = 0;
		ifc.ifc_buf = 0;
		int ret;
		if ((ret=ioctl(sockfd, SIOCGIFCONF, &ifc)) < 0) {
			if (errno != EINVAL || lastlen != 0)
				perror("ioctl error");
		}
		lastlen = ifc.ifc_len/sizeof(struct ifreq);
		len = ifc.ifc_len;
#endif	

	buf = calloc( 1, len );
	ifc.ifc_len = len;
	ifc.ifc_buf = buf;

	if ( ioctl( sockfd, SIOCGIFCONF, &ifc ) < 0 )
	{
		lerrno = errno;

		if (errno != EINVAL || lastlen != 0)
		{
			close(sockfd);
			abort();
		}
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

		memcpy( lastname, ifr->ifr_name, IFNAMSIZ );

		ifrcopy = *ifr;

		if ( ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy) < 0 )
		{
			lerrno = errno;
		}

		flags = ifrcopy.ifr_flags;

		ifi = calloc( 1, sizeof(struct ifi_info));
		*ifipnext = ifi;		/* prev points to this new one */
		ifipnext = &ifi->ifi_next;	/* pointer to next one goes here */

		ioctl(sockfd, SIOCGIFINDEX, &ifrcopy);

		ifi->ifi_index = ifrcopy.ifr_index;

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

