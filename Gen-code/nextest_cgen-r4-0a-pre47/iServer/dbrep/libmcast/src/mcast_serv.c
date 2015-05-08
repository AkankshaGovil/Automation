#include "unp.h"
#include "mcast.h"
#include <net/if.h>

int
mcast_serv(const char *maddr, const char *port, const char *ifname, 
			int * const sendfdp, int * const recvfdp, void **sapp)
{
	int					sendfd, recvfd;
	struct sockaddr_in	tmpsa;
	socklen_t			salen, tmpsalen;
	struct	ifreq		ifr;
	int					on = 1;

	tmpsalen = salen = sizeof(struct sockaddr_in);

	/* Create a socket for remote communication */
	sendfd = Udp_client(maddr, port, sapp, &salen);
	if (sendfd < 0) {
		err_sys("mcast_serv: Failed to start send server for remote communication\n");
		return(-1);
	}

	/* Bind it to the local port */
	bzero(&tmpsa, sizeof(tmpsa));
	tmpsa.sin_family = AF_INET;
	tmpsa.sin_addr.s_addr = htonl(INADDR_ANY);
	tmpsa.sin_port = htons(atoi(port));

	/* Find the local interface's address to which it is going to bind */
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sendfd, SIOCGIFADDR, &ifr) < 0) {
		err_sys("Request to get interface address of %s failed\n", ifname);
		return(-1);
	}
	else
		memcpy(&tmpsa.sin_addr, &((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr,
			sizeof(struct in_addr));

	Setsockopt(sendfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	Bind(sendfd, (struct sockaddr *)&tmpsa, sizeof(tmpsa));

	recvfd = Udp_server(maddr, port, &tmpsalen);
	if (recvfd < 0) 
		err_quit("mcast_serv: Failed to start recv server for remote communication\n");

	Mcast_join(recvfd, *sapp, salen, ifname, 0);
	Mcast_set_loop(sendfd, 0);     /* Disable LoopBack Packets */

	/* return values */
	*sendfdp = sendfd;
	*recvfdp = recvfd;

	return(0);
}
