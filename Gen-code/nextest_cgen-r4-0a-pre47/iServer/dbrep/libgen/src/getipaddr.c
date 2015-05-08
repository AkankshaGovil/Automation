#include "unp.h"
#include <net/if.h>

long
GetIpAddr(char *ifname)
{
	int fd;
	long	ipaddr = 0;
	struct ifreq	ifr;

	fd = Socket(AF_INET, SOCK_DGRAM, 0);

    if (ifname != NULL) {
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
        if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
            err_sys("Request to get interface address of %s failed\n", ifname);
            return(-1);
        }
        else {
			memcpy(&ipaddr, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr,\
			sizeof(long));
		}
    }
	
	return(ipaddr);
}
