//
//
//
//
//
//
//
//
//
//
//
// this file replaced with one in server directory
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#ifdef SUNOS
#include <sys/sockio.h>
#include <sys/filio.h>
#else
#include <linux/sockios.h>
#endif
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>

#include <signal.h>
#include <sys/wait.h>
#include <license.h>
#include "poll.h"

#include "spversion.h"

#include "generic.h"
#include "bits.h"
#include "ipc.h"
#include "serverdb.h"
#include "srvrlog.h"
#include "key.h"
#include "mem.h"

#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "entry.h"
#include "pef.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"
#include "timer.h"
#include "fdsets.h"
#include "db.h"
#include "connapi.h"
#include "shm.h"
#include "shmapp.h"
#include "xmltags.h"
#include "sconfig.h"

#include "gis.h"
#include "lrq.h"
#include <malloc.h>
#include "uh323inc.h"
#include "fdsets.h"
#include "list.h"

/*************************** CHANDLES ******************************/

ClientHandle *
GisAllocNativeCHandle(void)
{
	 ClientHandle *handle = NULL;

	 /* Allocate new handle */
	 handle = (ClientHandle *)malloc(sizeof(ClientHandle));

	 memset(handle, 0, sizeof(ClientHandle));

	 CHFree(handle) = 0;

	 return handle;
}

ClientHandle *
GisAllocCHandle(ClientHandle *chandle, int type)
{
	 char fn[] = "GisAllocCHandle():";

	 if (chandle == NULL)
	 {
		  chandle = GisAllocNativeCHandle();
	 }

	 chandle->type = type;

	 switch (type)
	 {
	 case CLIENTHANDLE_UCC:
	   CHUCCHandle(chandle) = GisAllocUCCHandle();
	   break;
#ifdef MEDIATION
	 case CLIENTHANDLE_SIP:
	   CHSIPHandle(chandle) = GisAllocSIPHandle();
	   break;
#endif
	 default:
	   NETERROR(MSEL, ("%s Invalid Handle Type\n", fn));
	   break;
	 }

	 return chandle;
}

void
GisFreeCHandle(void *ptr)
{
	 char fn[] = "GisFreeCHandle():";
	 ClientHandle *chandle = (ClientHandle *)ptr;

	 if (chandle == NULL)
	 {
		  return;
	 }

	 if (CHFree(chandle))
	 {
		  return;
	 }

	 CHFree(chandle) = 1;

	 switch (chandle->type)
	 {
	 case CLIENTHANDLE_UCC:
	   DEBUG(MSEL, NETLOG_DEBUG4, ("%s Freeing CLIENTHANDLE_UCC\n", fn));
	   GisFreeUCCHandle(CHUCCHandle(chandle));
	   break;
#ifdef MEDIATION
	 case CLIENTHANDLE_SIP:
	   DEBUG(MSEL, NETLOG_DEBUG4, ("%s Freeing CLIENTHANDLE_SIPINVITE\n", fn));
	   GisFreeSIPHandle(CHSIPHandle(chandle));
	   break;
#endif
	 default:
		break;
	 }

	 /* Just use free right now */
	 free(chandle);

	 return;
}

int
GisDisableCHandle(ClientHandle *chandle)
{
	 char fn[] = "GisDisableCHandle():";
	 
	 switch (chandle->type)
	 {
	 case CLIENTHANDLE_UCC:
	   DEBUG(MSEL, NETLOG_DEBUG4, ("%s Disabling CLIENTHANDLE_UCC\n", fn));
	   GisDisableUCCHandle(CHUCCHandle(chandle));
	   break;
#ifdef MEDIATION
	 case CLIENTHANDLE_SIP:
	   DEBUG(MSEL, NETLOG_DEBUG4, ("%s Disabling CLIENTHANDLE_SIPINVITE\n", fn));
	   GisDisableSIPHandle(CHSIPHandle(chandle));
	   break;
#endif
	 default:
	   break;
	 }
	 return(0);
 
}

/* Examine the client connections,
 * determine which are the ones which need
 * to be freed
 */
int
GisExamineList(List rList, time_t tm)
{
	 char fn[] = "GisExamineList():";
	 ListHead *lh = rList->head;
	 ListStruct *ptr = lh->begin, *nptr = NULL;
	 ClientHandle *handle;

	 NETDEBUG(MSEL, NETLOG_DEBUG4, 
		   ("%s Examining Remote List\n", fn));

	 while (ptr)
	 {
		  nptr = ptr->next;
		  handle = (ClientHandle *)ptr->item;

		  if (handle)
		  {
			   if ((tm - handle->rtime) > 15)
			   {
					NETDEBUG(MSEL, NETLOG_DEBUG4,
						  ("%s Connection aged out, disconnecting\n", fn));
					/* This one needs to be freed */
					GisDisableCHandle(handle);
					GisFreeCHandle(handle);
					listDeleteListEntry(rList, ptr);
			   }
		  }

		  ptr = nptr;
	 }

	 return 0;
}

/************************************ UCC Handles **************************/

UCCClientHandle *
GisAllocUCCHandle(void)
{
	 UCCClientHandle *ptr;

	 ptr = (UCCClientHandle *)malloc(sizeof(UCCClientHandle));
	 memset(ptr, 0, sizeof(UCCClientHandle));

	 return ptr;
}

void 
GisFreeUCCHandle(void *ptr)
{
	 if (ptr == NULL)
	 {
		  return;
	 }

	 free(ptr);
}

int
GisDisableUCCHandle(UCCClientHandle *handle)
{
	 char fn[] = "GisDisableUCCHandle():";
	 int fd = handle->fd;

	 if (fd < 0)
	 {
		  return -1;
	 }

	 NETDEBUG(MSEL, NETLOG_DEBUG4,
		   ("%s connection %d\n", fn, fd));

	 /* Set the cb data of the fd to NULL */
	 NetFdsSetCbData (&lsnetfds, fd, NULL, NULL);

	 /* Deactivate the fd associated with this handle */
	 NetFdsDeactivate(&lsnetfds, fd, FD_RW);

	 /* Shutdown the connection */
	 shutdown(fd, 2);

	 /* Close the connection */
	 close(fd);

	 handle->fd = -1;

	 return 0;
}

/****************************** MISC *******************************/


int
RCTimeout(struct Timer *t)
{
	 List list = (List)(t->data);
	 time_t ctime;

	 time(&ctime);
	 
	 GisExamineList(list, ctime);

	 return 0;
	 
}

int
GisDeleteCHandle(ClientHandle *handle)
{
	 GisFreeCHandle(handle);
	 return(0);
}

void
GisPurgeCHandle(void *ptr)
{
	 ClientHandle *handle = (ClientHandle *)ptr;

	 GisDisableCHandle(handle);
	 GisDeleteCHandle(handle);
}

#if 0
int
GisInitRemoteList(void)
{
	 struct 	itimerval rctmr;
	 tid 		rctid;

#if 0
	 /* Initialize the peer handle list */
	 gisActiveCHandles = listInit();

	 /* Start the remote list timer */
	 memset(&rctmr, 0, sizeof(struct itimerval));
	 rctmr.it_interval.tv_sec = 15;

	 rctid = timerAddToList(&localConfig.timerPrivate, &rctmr,
		0, PSOS_TIMER_REL, "RCTimer", RCTimeout, gisActiveCHandles);

#endif
	 return 0;
}


ClientHandle *
GisAddRemote(List rList, int fd, struct sockaddr_in *client)
{
	 ClientHandle *handle = NULL;

	 /* Allocate new handle */
	 handle = GisAllocCHandle(NULL, CLIENTHANDLE_UCC);

	 CHUCCHandle(handle)->fd = fd;

	 memcpy(&handle->client, client, sizeof(struct sockaddr_in));
	 time(&handle->ctime);
	 time(&handle->rtime);

	 /* Add it to the list */
	 listAddItem(rList, handle);

	 /* Add the fd to our active list of fd's */
	 NetFdsAdd(&lsnetfds, fd, FD_READ, (NetFn)HandleRemoteClient, (NetFn)NULL,
			   handle, GisFreeCHandle);

	 return handle;
}
#endif
