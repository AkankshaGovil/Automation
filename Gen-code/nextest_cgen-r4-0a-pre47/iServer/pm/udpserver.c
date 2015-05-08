#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>

#include "pids.h"
#include "pmpoll.h"
#include "srvrlog.h"

#include "pm.h"

static int sd;


int initserver(void)
{
	static char fn[] = "initserver():";
	int on =1;
	int flags;
	struct sockaddr_in servaddr,cliaddr;
	struct sockaddr my_addr;

	if((sd = socket(AF_INET,SOCK_DGRAM,0)) <0)
	{
		ERROR(MPMGR,("initserver socket %s\n",strerror(errno)));
		return -1;
	}
	if((flags = fcntl(sd,F_GETFL,0)) <0)
	{
		ERROR(MPMGR,("initserver fcntl %s\n",strerror(errno)));
		return -1;
	}
	flags |= O_NONBLOCK;

	if((fcntl(sd,F_SETFL,flags)) <0)
	{
		ERROR(MPMGR,("initserver fcntl2 %s\n",strerror(errno)));
		return -1;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(LOCALHOST);
	servaddr.sin_port = htons(SERV_PORT);

	

	
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

	if(bind(sd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		ERROR(MPMGR,("%s bind %s\n", fn, strerror(errno)));
                return -1;
        }
	
	return sd;
}

	
int handlepkt(KeepAlive *pmsg)
{
	static char fn[] = "handlepkt() : ";
	struct sockaddr_in servaddr,cliaddr;
	static int kasize = sizeof(KeepAlive);
	char buf[1024];
	int len = sizeof(struct sockaddr_in);
	int status;


	len = kasize;
	status = recvfrom(sd,(char *)pmsg,sizeof(KeepAlive),0,(struct sockaddr *)&cliaddr,&len);
	if(status == -1)
	{
		if (errno != EAGAIN) {
		NETERROR(MPMGR, ("%s recvfrom error - %s \n", fn, strerror(errno)));
		}
		return -1;
	}

	pmsg->id = ntohl(pmsg->id);
	pmsg->gid = ntohl(pmsg->gid);
	pmsg->pid = ntohl(pmsg->pid);
	pmsg->startupTime = ntohl(pmsg->startupTime);

	// if the pid is -1, it means the source is requesting the uptime data
	if (pmsg->pid == -1) {
	   memset(buf, 0, 1024);
	   len = FillUptimeInfo(buf);
	   status = sendto(sd, buf, len, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
	   if (status <= 0) {
		  DEBUG(MPMGR, NETLOG_DEBUG2, ("Error sending uptime information to %x [%d]\n", cliaddr.sin_addr.s_addr, errno));
	   }
	   return 0;
	}

	DEBUG(MPMGR,NETLOG_DEBUG4,("%s Poll recd from id = %d pid = %d\n",
				fn,pmsg->id,pmsg->pid));
	return 0;
}
