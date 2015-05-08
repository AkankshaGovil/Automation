#include "unp.h"

int
unix_serv(const char * serverpath, int MaxClient)
{
	struct sockaddr_un 	servaddr;
	int 			listenfd;

	listenfd = Socket(AF_LOCAL, SOCK_STREAM, 0);
	
	unlink(serverpath);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, serverpath);
	
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, MaxClient);

	return(listenfd);	
}

