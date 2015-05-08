#include    <unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<sys/syslog.h>
#include 	"serverp.h"
#include 	"srvrlog.h"
#include	"unpifi.h"
#include 	<malloc.h>


static void populateHwAddr(struct ifi_info *pif,int sock);
static struct ifi_info *
getifiinfo(int family, int doaliases)
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
		{
			continue;	/* ignore if interface not up */
		}

		ifi = ( struct ifi_info *)calloc(1, sizeof(struct ifi_info));
		*ifipnext = ifi;			/* prev points to this new one */
		ifipnext = &ifi->ifi_next;	/* pointer to next one goes here */

		ifi->ifi_flags = flags;		/* IFF_xxx values */
		ifi->ifi_myflags = myflags;	/* IFI_xxx values */
		memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
		ifi->ifi_name[IFI_NAME-1] = '\0';

		switch (ifr->ifr_addr.sa_family) {
		case AF_INET:
                if (!(ifi->ifi_flags & IFF_LOOPBACK))
		{

			populateHwAddr(ifi,sockfd);
		}
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


static void populateHwAddr(struct ifi_info *pif,int sock)
{

                struct ifreq ifr;
                struct sockaddr sa;
                int b, sk;
		int i;

		memset(&ifr,0,sizeof(struct ifreq));
		
		strcpy (ifr.ifr_name, pif -> ifi_name);
                /* Read the hardware address from this interface. */
#ifdef SIOCGIFHWADDR
                if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
		{
			DEBUG(MLMGR,NETLOG_DEBUG4,
			("populatehwaddr SIOCGENADDR returned error  %d\n",errno));
			pif->ifi_hlen = 0;
			return;
		}
                sa = *(struct sockaddr *)&ifr.ifr_hwaddr;
		pif-> ifi_hlen = 6;
		memcpy (pif->ifi_haddr, sa.sa_data, 6);

#endif
	return;
}

static void
freeifiinfo(struct ifi_info *ifihead)
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

/* Returns OK(0) if the address is one of the local addresses, else -1 ERROR*/
int validateHwAddress(int hwlen, unsigned char *hwaddr)
{
	
    struct ifi_info  *ifihead, *ifi, *ifinext;
    int status = -1 ;
    int i;
/* convert from string to hex */
    unsigned long long int ethaddr = strtoull((const char *)hwaddr, (char **)NULL, 16);
    int ethlen = hwlen/2;
    unsigned char etheradd[ethlen];
    unsigned char *p = (unsigned char *)&ethaddr;
    for(i = 0; i < ethlen; i++)
    {
#if BYTE_ORDER == BIG_ENDIAN
    	etheradd[i] = p[i];
#elif BYTE_ORDER == LITTLE_ENDIAN
    	etheradd[i] = p[ethlen - 1 - i];
#endif
    }

	ifihead = getifiinfo(AF_INET,0);
	if (!ifihead)
	{
		fprintf(stderr,"getHwAddress: Failed to get interfaceInfo\n");
		exit(1);
	}
	
	for (ifi = ifihead; ifi != NULL; ifi = ifi->ifi_next) {


		if(ethlen == ifi->ifi_hlen && !memcmp((void *)&etheradd,ifi->ifi_haddr,ethlen))
		{
			status = 0;	
			break;
		}
	}
	freeifiinfo(ifihead);

	return status;
	
}

int validateHostid(char *hwaddr)
{
    char hoststr[9] = {0};
    char *p;
    int licId;
    int i;
    union {
            unsigned char ch[4];
            long  l;
        } hostid;


        hostid.l = htonl(gethostid());
        for(p = hoststr,i = 0;i<4;++i,p+=2)
        {
                sprintf(p,"%02x",hostid.ch[i]);
        }

	if(!hwaddr)
	{
	
		ERROR(MLMGR,
		("Null hardware address, %s, host, %s\n",hwaddr,hoststr));
		return -1;
	}

	DEBUG(MLMGR,NETLOG_DEBUG4,("validate hostid licId = %s hostid = %s\n",hwaddr,hoststr));
	
	if(!strncasecmp(hoststr,hwaddr,8))
	{
		return 0;
	}
	ERROR(MLMGR,
	("Invalid hostid (%s) in License file. Expected  hostid = %s\n",
	hwaddr,hoststr));
	
	return -1;
}
