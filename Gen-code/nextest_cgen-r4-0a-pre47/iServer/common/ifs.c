#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "unpifi.h"
#include "ifs.h"

struct ifi_info *
initIfs()
{
	return get_ifi_info(AF_INET, 0);	
}

struct ifi_info *
initIfs2(int doalias)
{
	return get_ifi_info2(AF_INET, doalias);
}

void
freeIfs(struct ifi_info *ifi_head)
{
	struct ifi_info *ifp, *ifpnext;
	for (ifp = ifi_head; ifp; ifp = ifpnext)
	{
		ifpnext = ifp->ifi_next;
		free(ifp);
	}
	return ;
}
/* Input in net order */
struct ifi_info *
findIf(struct ifi_info *ifihead, 
	unsigned long ipaddr)
{
	struct ifi_info	*ifi, *ifinext;

	/* Go through the list of interfaces given and match
	 * the right one
	 */
	for (ifi = ifihead; ifi != NULL; ifi = ifinext)
	{
		if ((ifi->ifi_addr != NULL) &&
			 (ifi->ifi_netmask != NULL))
		{
			if ((ipaddr & ifi->ifi_netmask->sin_addr.s_addr) ==
				(ifi->ifi_addr->sin_addr.s_addr &
					ifi->ifi_netmask->sin_addr.s_addr))
			{
				return ifi;
			}
		}
		ifinext = ifi->ifi_next;
	}

	return 0;
}

struct ifi_info *
matchIf(struct ifi_info *ifihead, unsigned long ipaddr)
{
	struct ifi_info	*ifi, *ifinext;

	for (ifi = ifihead; ifi != NULL; ifi = ifinext)
	{
		if ((ifi->ifi_addr != NULL) && 
                         (ifi->ifi_netmask != NULL))
		{
			if (ipaddr == ifi->ifi_addr->sin_addr.s_addr)
			{
				return ifi;
			}
		}
		ifinext = ifi->ifi_next;
	}

	return 0;
}

unsigned long
getLocalIf(struct ifi_info *ifihead)
{
	struct ifi_info	*ifi, *ifinext;

	for (ifi = ifihead; ifi != NULL; ifi = ifinext)
	{
		if (ifi->ifi_flags & (IFF_LOOPBACK|IFF_POINTOPOINT))
		{
			goto _continue;
		}

		if (!(ifi->ifi_flags & IFF_UP))
		{
			goto _continue;
		}

		if ((ifi->ifi_addr != NULL) && 
                         (ifi->ifi_netmask != NULL))
        {
			return ifi->ifi_addr->sin_addr.s_addr;
		}

	_continue:
		ifinext = ifi->ifi_next;
	}

	return 0;
}

struct ifi_info *
findIfByIfname(	struct ifi_info *ifihead, 
				char * ifname )
{
	struct ifi_info	*ifi, *ifinext;

	/* Go through the list of interfaces given and match
	 * the right one
	 */

	for (ifi = ifihead; ifi != NULL; ifi = ifinext)
	{
		if ( ifi->ifi_addr != NULL )
		{
			if ( !strcmp( ifname, ifi->ifi_name ) )
			{
				return ifi;
			}
		}
		ifinext = ifi->ifi_next;
	}

	return 0;
}

char *
GetNextIfname(	struct ifi_info *ifihead, 
				char * ifname )
{
	struct ifi_info	*ifi, *ifinext;

	/* Go through the list of interfaces given and match
	 * the right one
	 */

	if ( ifname != NULL )
	{
		for (ifi = ifihead; ifi != NULL; ifi = ifinext)
		{
			if ( ifi->ifi_addr != NULL )
			{
				if ( !strcmp( ifname, ifi->ifi_name ) )
					break;
			}
			ifinext = ifi->ifi_next;
		}
	}
	else
	{
		ifi = ifihead;

		if ( ifi != NULL )
			return( ifi->ifi_name );
	
		return( (char*) NULL );
	}

	if ( ifi->ifi_next != NULL )
		return( ifi->ifi_next->ifi_name );

	return( (char*) NULL );
}

int
checkIfName(struct ifi_info  *ifi_head, char *ifname)
{
	struct ifi_info *ifp;
	unsigned short flags;
	unsigned char flag=0;

	if (!ifi_head)
	{
		ifi_head = initIfs();
		flag=1;
	}

	for (ifp = ifi_head; ifp; ifp= ifp->ifi_next)
	{
		if (!strcmp(ifname, ifp->ifi_name))
			break;
	}

	if (!ifp)
	{
		return -1;
	}

    flags = GetIfFlags(ifname);
    if ((flags & (IFF_RUNNING|IFF_UP)) != (IFF_RUNNING|IFF_UP))
    {
        return -1;
    }

	if (flag)
	{
		freeIfs(ifi_head);
	}
	return 0;
}

/* we might want to define this with ifdef BYTE_ORDER== */
#define LSB_MASK	0x0001
/*
 * Expects netmask in host byte order
 *  
 */
int
checkMask(unsigned long mask)
{
	unsigned long val=0;
	unsigned long lsb;

	val = ~mask;
	lsb = val & LSB_MASK;

	while(lsb) 
	{
		val >>= 1;
		lsb = val & LSB_MASK;
	}

	return (val ? -1 : 0);
}
