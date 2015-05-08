#include "unp.h"

int
tcp_serv(in_port_t port, int MaxClient)
{
	struct sockaddr_in 	servaddr;
	int 			listenfd;
	int				on = 1;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
	
	Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, MaxClient);

	return(listenfd);	
}

