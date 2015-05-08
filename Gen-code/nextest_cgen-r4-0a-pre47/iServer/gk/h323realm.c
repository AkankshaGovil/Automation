#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include "seli.h"
#include "rvcommon.h"
#include "uh323.h"
#include "h323realm.h"
#include "systemlog.h"
static int nListen = 0;

/* Gkul:Defining an array is not very scalable approach
 * with dynamic realm addition/deletion
 */
RasListener rasListenerTbl[MAX_RAS_LISTENER]; 

/* callback for sd */
void rasRecv(int sd, seliEvents sEvent, BOOL error)
{
	int instanceChantype = sd <<1;
  	if (error)
	{
		NETERROR(MH323,("error %s on fd = %d\n",strerror(errno), sd));
		return;
	}
	dmr_recvRasMessage(sd,0x1,0,UH323Globals()->hApp);
}


int createRasListener(int realmId,Ras_type type, uint32_t ip, unsigned short port, unsigned short q931port)
{
	int sd = -1;

	if (nListen < MAX_RAS_LISTENER -1 )
	{
		sd = startServer(ip,port);
		rasListenerTbl[nListen].destIP = ip;
		rasListenerTbl[nListen].destPort = port;
		rasListenerTbl[nListen].q931Port = q931port;
		rasListenerTbl[nListen].sd = sd;
		rasListenerTbl[nListen].realmId = realmId;
		rasListenerTbl[nListen].type = type;
		nListen++;
	}
	return sd;
}

int destroyRasListener(int realmId, Ras_type type,unsigned short port)
{
	int i;
	int sd;

	for (i = 0; i < nListen; ++i)
	{
		if ( (rasListenerTbl[i].realmId == realmId) && (rasListenerTbl[i].destPort == port) &&
			((nh323Instances ==1) || (nh323Instances >1 && rasListenerTbl[i].type == type)))
		{
			/* close the socket and mark entry as invalid */
			close(rasListenerTbl[i].sd);
			rasListenerTbl[i].realmId = -1;
			sd = rasListenerTbl[i].sd;
			rasListenerTbl[i].sd = 0;
			return(sd);
		}
	}
	return -1;
}

int getRasInstances(void)
{
	return nListen;
}

#if 0
int addRasListener(int realmId,Ras_type type, uint32_t ip, unsigned short port, unsigned short q931port)
{
	int sd;
	sd = startServer(ip,port);
	rasListenerTbl[nListen].destIP = ip;
	rasListenerTbl[nListen].destPort = port;
	rasListenerTbl[nListen].q931Port = q931port;
	rasListenerTbl[nListen].sd = sd;
	rasListenerTbl[nListen].realmId = realmId;
	rasListenerTbl[nListen].type = type;
	nListen++;
	return sd;
}
#endif

// Client return sd of the socket
int startServer(uint32_t ip, short port)
{
	static char fn[] = "startserver():";
	int on =1;
	int sd,flags;
	struct sockaddr_in servaddr,cliaddr;
	struct sockaddr my_addr;

	if((sd = socket(AF_INET,SOCK_DGRAM,0)) <0)
	{
		NETERROR(MH323,("initserver socket %s\n",strerror(errno)));
		return -1;
	}
	if((flags = fcntl(sd,F_GETFL,0)) <0)
	{
		NETERROR(MH323,("initserver fcntl %s\n",strerror(errno)));
		return -1;
	}
	flags |= O_NONBLOCK;

	if((fcntl(sd,F_SETFL,flags)) <0)
	{
		NETERROR(MH323,("initserver fcntl2 %s\n",strerror(errno)));
		return -1;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(ip);
	servaddr.sin_port = htons(port);
	
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

	if(bind(sd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		NETERROR(MH323,("%s bind %s\n", fn, strerror(errno)));
                return -1;
        }
	
	return sd;
}

int getRasRealmInfo(int sd, int *ipptr, unsigned short *portptr, int *realmIdptr)
{
	int i;
	// optimize it - use realmindex
	for(i = 0; i < nListen; ++i)
	{
		if(rasListenerTbl[i].sd == sd)
		{
			*portptr = rasListenerTbl[i].destPort;
			*ipptr = rasListenerTbl[i].destIP;
			*realmIdptr = rasListenerTbl[i].realmId;
			return 0;
		}
	}
	return -1;
}

int getQ931RealmInfo(int sd, int *ipptr, unsigned short *portptr, int *realmIdptr)
{
	int i;
	// optimize it - use realmindex
	for(i = 0; i < nListen; ++i)
	{
		if(rasListenerTbl[i].sd == sd)
		{
			*portptr = rasListenerTbl[i].q931Port;
			*ipptr = rasListenerTbl[i].destIP;
			*realmIdptr = rasListenerTbl[i].realmId;
			return 0;
		}
	}
	return -1;
}
int getRasInfoFromRealmId(int realmId,Ras_type type, int *sdptr,int *ipptr, unsigned short *portptr)
{
	int i;
	for(i = 0; i < nListen; ++i)
	{
		if(rasListenerTbl[i].realmId == realmId && 
			((nh323Instances ==1) || (nh323Instances >1 && rasListenerTbl[i].type == type)))
		{
			*portptr = rasListenerTbl[i].destPort;
			*ipptr = rasListenerTbl[i].destIP;
			*sdptr = rasListenerTbl[i].sd;
			return 0;
		}
	}
	return -1;
}

int getQ931InfoFromRealmId(int realmId,Ras_type type, int *sdptr,int *ipptr, unsigned short *portptr)
{
	int i;
	for(i = 0; i < nListen; ++i)
	{
		if(rasListenerTbl[i].realmId == realmId && 
			((nh323Instances ==1) || (nh323Instances >1 && rasListenerTbl[i].type == type)))
		{
			*portptr = rasListenerTbl[i].q931Port;
			*ipptr = rasListenerTbl[i].destIP;
			*sdptr = rasListenerTbl[i].sd;
			return 0;
		}
	}
	return -1;
}
