#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <strings.h>

#include "pmpoll.h"
#include "srvrlog.h"
#include "nxosd.h"


static int sd;
static KeepAlive kamsg;
static int myid;
static int mygid;
static unsigned long startupTime = 0;


int Initpoll(int id,int gid)
{
    struct timeval tv;
	static char *fn = "Initpoll() : ";
	struct sockaddr_in servaddr;
	static int servlen = sizeof(servaddr);

	myid = id;
	mygid = gid;

	if (gettimeofday(&tv, (void*) NULL )) {
	   DEBUG(MPMGR, NETLOG_DEBUG2, ("Initpoll: error in gettimeofday [%d]\n", errno));
	} else {
	   startupTime = (unsigned long)tv.tv_sec;  /* time since epoch */
	   DEBUG(MPMGR, NETLOG_DEBUG4, ("Initpoll: %d:%d init at %ld\n", gid, id, tv.tv_sec));
	}
	
	if((sd = socket(AF_INET,SOCK_DGRAM,0)) <0)
	{
		ERROR(MPMGR,("initpoll: socket %s\n",strerror(errno)));
		return -1;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = inet_addr(LOCALHOST);

	if(connect(sd,(struct sockaddr *)&servaddr,servlen) <0)
	{
		ERROR(MPMGR,("initpoll: connect %s\n",strerror(errno)));
		return -1;
	}
	return sd;
}

int Sendpoll(int t)
{
	static int kasize = sizeof(KeepAlive);
	/* sendto(sd,(char *) &servaddr,kasize,0,pservaddr,servlen);*/

	kamsg.id = htonl(myid);
	kamsg.gid = htonl(mygid);
	kamsg.pid = htonl(getpid());
	kamsg.startupTime = htonl(startupTime);
        
	if ( send(sd,(char *)&kamsg,kasize,0) < 0)
	{
		NETDEBUG(MPMGR,NETLOG_DEBUG2,("Sendpoll: send %s\n",strerror(errno)));
		return -1;
	}
	DEBUG(MPMGR,NETLOG_DEBUG2,("sendpoll pid = %lu \n",ULONG_FMT(getpid())));

	kamsg.restartid = ISERVER_NONE;  // reset the restart id

	return 0;
}

void setRestart(int id){
  kamsg.restartid = id;
}
