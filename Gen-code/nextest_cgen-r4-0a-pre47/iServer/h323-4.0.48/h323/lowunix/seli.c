#ifdef __cplusplus
extern "C" {
#endif



/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

/*
  seli.c

  Select Interface.

  Provides absrtact selection of file descriptors upon read, write or exeption and
  timeouts per task.

  Supporting calls from different tasks (using thread local storage).

  Ron S.  14 Jan. 1996

  Revision 1: take care of timeout in case of intensive flow of events from network.
  Revision 2: multi-thread safe. Semaphores.
  Revision 3: 11 Dec. 1996: fix bug in seliSelect(). WRITE before READ.
  */

/* change the number of available file descroptors */



/************************************
 * Check which OS mechanism is used.
 *
 * The following are supported:
 * 1. select()
 * 2. poll()
 * 3. /dev/poll
 * 4. /dev/epoll
 ************************************/
#ifdef USE_POLL
#define SELI_USE_POLL
#endif

/* Nextone */	
#if ( !defined(SELI_USE_POLL) && !defined(SELI_USE_DEVPOLL) && !defined(SELI_USE_DEVEPOLL))
#define SELI_USE_SELECT
#endif





#ifdef SELI_USE_POLL
#include <pollfd.h>
#endif

#ifdef SELI_USE_DEVPOLL
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/devpoll.h>
#endif

/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
#include <sys/epoll.h>
#endif

#ifdef __VXWORKS__
/* ------------------------- vxWorks ----------------------------- */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/ioctl.h>
#include <inetLib.h>
#include <ioLib.h>
#include <selectLib.h>
#include <taskLib.h>
#include <time.h>
#include <tickLib.h>

#elif RV_OS_OSE
/* -------------------------- OSE ------------------------- */
#include <ose.h>
#include <inet.h>

#elif __PSOSPNA__
/* ------------------------- pSOS --------------------------- */
#include <stdio.h>
#include <configs.h>
#include <time.h>
#include <pna.h>
#define PF_INET AF_INET
#define FIONREAD FIOREAD
#define __Iunistd



#elif UNDER_CE
/* --------------------------- Windows CE ---------------------------- */
#pragma warning (disable : 4201 4214)
#define FD_SETSIZE 2048
#include <windows.h>
#include <winsock.h>


#else
/* ----------------------------- UNIX ------------------------------ */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <sys/times.h>
#include <limits.h>
#include <errno.h>
#endif  /* __VXWORKS__ / __PSOSPNA__ / UNDER_CE */


#ifdef UNIXWARE
#define _REENTRANT
#include <errno.h>
#endif

#ifdef __AIX__
#include <sys/select.h>
#endif


#include <rvinternal.h>
#include <seli.h>
#include <rlist.h>
#include <ra.h>
#include <ms.h>
#include <etimer.h>
#include <ts.h>
#include <mti.h>
#include <mei.h>
#include <ti.h>
#include <tls.h>

#ifndef NOTHREADS
#include <pthread.h>
#endif


#define READ_FD  0
#define WRITE_FD 1


/* Number of file descriptors by default */
/* Nextone */	
#if ( defined(SELI_USE_POLL) || defined(SELI_USE_DEVPOLL) || defined(SELI_USE_DEVEPOLL))
#define SELI_FD_SETSIZE 2048
#else
#define SELI_FD_SETSIZE FD_SETSIZE
#endif




/* SELI list of nodes. */
typedef struct
{
    int             valid;    /* key */
    seliEvents      event;    /* on what events the application want to be notified. */
    seliCallback    callback; /* callback function */
} seliNode;


/* per context */
typedef struct
{
    seliNode*       nodes; /*[SELI_FD_SETSIZE];*/ /* list of socket registrations. */
    INT32           counter; /* number of calls to liInit() in this task */

#ifdef SELI_USE_SELECT
    fd_set          selectRd;
    fd_set          selectWr;
    fd_set          selectEx;
#endif  /* SELI_USE_SELECT */

#ifdef SELI_USE_POLL
    struct pollfd*  fdArray;/* [SELI_FD_SETSIZE]; */
#endif  /* SELI_USE_POLL */

#ifdef SELI_USE_DEVPOLL
    struct pollfd*  fdArray;/* [SELI_FD_SETSIZE]; */
    int             fdDevPoll; /* /dev/poll's file descriptor (the driver used) */
    INT32           fdsUsed; /* Number of FDs currently used */
#endif  /* SELI_USE_DEVPOLL */

/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
	struct epoll_event 		*epollEvents;	/* [SELI_FD_SETSIZE]; */
    int             		fdDevEPoll; /* /dev/epoll's file descriptor (the driver used) */
    INT32           		fdsUsed; 	/* Number of FDs currently used */
#endif  /* SELI_USE_DEVEPOLL */

    int             pipefd[2]; /* Pipe used to preempt a select() call of the thread */
    HMEI            hMei;
} THREAD_SeliLocalStorage;




/*__________Local Variables */

static int msa=0;
static INT32 seliInitCalls = 0;
static INT32 seliFdSetSizeDyn = SELI_FD_SETSIZE;
static INT32 seliAllocSize = 0;

HTSTRUCT mtiGetTimersHandle(void);





/**********************************************************************************************
 *
 *                 Private functions used by SELI
 *
 **********************************************************************************************/


/* conver events to string */
static char * /* strEv */
seliEvent2Str(
          IN  seliEvents sEvent,
          OUT char* strEv) /* name of events */
{
  if (!strEv) return (char*)"(null)";

  strEv[0] = 0;
  if (sEvent & seliEvRead)    strcat(strEv, " Read ");
  if (sEvent & seliEvExept)   strcat(strEv, " Exception ");
  if (sEvent & seliEvWrite)   strcat(strEv, " Write ");
  if (sEvent & ~(seliEvRead|seliEvExept|seliEvWrite))
    strcat(strEv, " Unknown ");
  return strEv;
}


static
int seliTimerExpired(void)
{
    HTSTRUCT timerH = mtiGetTimersHandle();

    if (timerH != NULL)
        tsCheck(timerH);

    return 0;
}


#ifndef NOTHREADS

static int seliCreatePipe(IN THREAD_SeliLocalStorage* seliTls)
{
    int res = pipe(seliTls->pipefd);
    int ok;
    int flags;

    flags = fcntl(seliTls->pipefd[WRITE_FD], F_GETFL, 0);
    if (flags < 0)
    {
        puts("fcntl() ERROR");
        return 0;
    }

    flags |= O_NONBLOCK;

    ok = fcntl(seliTls->pipefd[WRITE_FD], F_SETFL, flags);
    if (ok == -1)
        puts("fcntl() ERROR");

    flags = fcntl(seliTls->pipefd[READ_FD], F_GETFL, 0);
    if (flags < 0 )
    {
        puts("fcntl() ERROR");
        return 0;
    }

    flags |= O_NONBLOCK;

    ok = fcntl(seliTls->pipefd[READ_FD], F_SETFL, flags);
    if (ok == -1)
        puts("fcntl() ERROR");

    return res;
}


void pipeCallback(int fd, seliEvents sEvent, BOOL error)
{
  char buff[100];
  int res=1;

  if (sEvent || error);

  msaPrintFormat(msa, " pipeCallback: %d %d.",fd,res);

  /*while(res != -1 )*/
  {
      res = read( fd, buff, 1/*sizeof(buff)*/ );
      if( res > 10 )
    {
      printf("I read already <%d> bytes out of pipe\n", res );
      fflush(stdout);
    }
  }
}

#endif





/**********************************************************************************************
 *
 *                 Private functions used by SELECT()
 *
 **********************************************************************************************/
#ifdef SELI_USE_SELECT

/* Desc: Add descriptor to read, write or except fd sets according
   to event.
   Input:  socketId- socket descriptor.
           sEvent- event enumeration.
   Output: rdSet-pointer to read fd set.
           wrSet-pointer to write fd set.
           exSet-pointer to except fd set.
   Returns True if any bit was set in fd set.
*/
static int
seliSelectAdd2fdSet(fd_set *rdSet, fd_set *wrSet, fd_set *exSet,
            int fd, seliEvents sEvent)
{
  int ret = FALSE;
#ifdef  UNDER_CE
  SOCKET fdNum = (SOCKET)fd;
#else
  int fdNum=fd;
#endif


  if (sEvent & seliEvRead) {
    FD_SET(fdNum, rdSet);
    ret = TRUE;
  }
  else
    FD_CLR(fdNum, rdSet);
  if (sEvent & seliEvWrite) {
    FD_SET(fdNum, wrSet);
    ret = TRUE;
  }
  else
    FD_CLR(fdNum, wrSet);
  if (sEvent & seliEvExept) {
    FD_SET(fdNum, exSet);
    ret = TRUE;
  }
  else
    FD_CLR(fdNum, exSet);

  return ret;
}

static int
seliSelectDetectEvent(
              IN fd_set *rdSet,
              IN fd_set *wrSet,
              IN fd_set *exSet,
              IN int fd,
              OUT seliEvents *sEvent)
{
    seliNode *elem;
    THREAD_SeliLocalStorage* seliTls;

    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if ( ! seliTls ) return RVERROR;
    elem = &seliTls->nodes[fd];

    if ( ! elem->valid )
        return RVERROR;

    if ( (elem->event & seliEvWrite) && FD_ISSET(fd, wrSet) )
    {
        *sEvent=seliEvWrite;
        return OK;
    }

    if ((elem->event & seliEvRead) && FD_ISSET(fd, rdSet) )
    {
        *sEvent=seliEvRead;
        return OK;
    }

    if ((elem->event & seliEvExept) && FD_ISSET(fd, exSet) )
    {
        *sEvent=seliEvExept;
        return OK;
    }

    return RVERROR;
}

static int
seliSelectList2fdSet(
           THREAD_SeliLocalStorage* seliTls, /* socket list of current task */
           fd_set*                  rdSet,
           fd_set*                  wrSet,
           fd_set*                  exSet)
{
    int fd;
    int lastBit = 0;

    for (fd=0; fd <seliFdSetSizeDyn; fd++)
    {
        if (seliTls->nodes[fd].valid)
            if (seliSelectAdd2fdSet(rdSet, wrSet, exSet, fd, seliTls->nodes[fd].event))
                lastBit = fd;
    }

    return lastBit;
}

int
seliSelectEventsRegistration(
                 IN  int fdSetLen,
                 OUT int *num,
                 OUT fd_set *rdSet,
                 OUT fd_set *wrSet,
                 OUT fd_set *exSet,
                 OUT UINT32 *timeOut
                 )
{
    THREAD_SeliLocalStorage* seliTls;
    UINT32 minT;

    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if (!seliTls)
        return RVERROR;

    meiEnter(seliTls->hMei);
    if (fdSetLen < seliFdSetSizeDyn)
        return RVERROR;
    *num = seliSelectList2fdSet(seliTls, rdSet, wrSet, exSet)+1;

    minT = tsGetMinTime(mtiGetTimersHandle());

    if ( minT == (UINT32)-1 )
        *timeOut = (UINT32)-1;
    else
    {
        BOOL  tcur_rollOver=FALSE;
        BOOL  minT_rollOver=FALSE;
        UINT32 tcur = timerGetTimeInMilliseconds();
        UINT32 diff;

        if (tcur>minT)
          diff=tcur - minT;
        else
          diff=minT - tcur;

        if ((minT < tcur) && (diff > 0x80000000))
        {
           minT_rollOver=TRUE;  /* the minT has rollOver */
        }

        if ((tcur < minT) && (diff > 0x80000000))
        {
           tcur_rollOver=TRUE;  /* the minT has rollOver */
        }

        if (minT < tcur)
        {
            if (!minT_rollOver)/* some timeouts should be called immediately */
              *timeOut = 0;
            else /* actually minT has passed 0xFFFFFFFF and is bigger that current time*/
              *timeOut = (0xFFFFFFFF - tcur) + minT;
        }
        else 
        { /* minT > t_current */  
            if (!tcur_rollOver) 
              *timeOut = minT - tcur;
            else /* the current time has passed 0xFFFFFFFF mark and actually bigger than minT*/
              *timeOut = 0;
        }
    }

    msaPrintFormat(msa, "seliSelectEventsRegistration: minT=%d timeout=%d", minT, *timeOut);

    meiExit(seliTls->hMei);

    return OK;
}

int
seliSelectEventsHandling(
            IN fd_set *rdSet,
            IN fd_set *wrSet,
            IN fd_set *exSet,
            IN int num,
            IN int numEvents
            )
{
    THREAD_SeliLocalStorage* seliTls;
    int fd;
    seliEvents sEvent=seliEvExept;
    seliNode *elem;

    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if ( !seliTls)
        return RVERROR;

    /* meiEnter(tNode->hMei);*/
    if (numEvents > 0)
    {
        /* We start looking from the end to the beginning - it works faster this way in
           most cases. */
        for (fd = num-1; fd >= 0; fd--)
        {
            if ( (rdSet && FD_ISSET(fd, rdSet)) ||
                 (wrSet && FD_ISSET(fd, wrSet)) ||
                 (exSet && FD_ISSET(fd, exSet)) )
            {
                elem = &seliTls->nodes[fd];
                if ( seliSelectDetectEvent( rdSet, wrSet, exSet, fd, &sEvent) == OK )
                {
                    msaPrintFormat(msa, "seliSelectEventsHandling: fd=%d event=%x callback=0x%p",
                        fd, sEvent, elem->callback);
                    elem->callback(fd, sEvent, FALSE);
                }

                numEvents--;
                if (numEvents == 0)
                    break;
            }
        }
    }

    seliTimerExpired();

    /* meiExit(tNode->hMei);*/
    return 0;
}

static int
seliSelectInit(
           OUT fd_set *rdSet,
           OUT fd_set *wrSet,
           OUT fd_set *exSet
           )
{
  FD_ZERO(rdSet);
  FD_ZERO(wrSet);
  FD_ZERO(exSet);

  return OK;
}

static int
seliSelectEvents(
         IN int fdSetLen,
         IN fd_set *rdSet,
         IN fd_set *wrSet,
         IN fd_set *exSet
         )
{
    struct timeval* pTv;
    struct timeval tv;
    UINT32 msec=0;
    int num=0, retval;

    /* Register events at select file discriptor vector */
    if ( seliSelectEventsRegistration( fdSetLen ,&num, rdSet, wrSet, exSet, &msec) == RVERROR )
        return RVERROR;

    /* Determine the amount of time select() function will be blocking */
    if ( msec == (UINT32)-1)
        pTv = NULL;
    else
    {
        tv.tv_sec = msec/1000;
        tv.tv_usec = (msec%1000)*1000;
        pTv = &tv;
    }

    retval = select(num, rdSet, wrSet, exSet, pTv);

    /* take care of events that occured */
    seliSelectEventsHandling( rdSet, wrSet, exSet, num, retval );

    return OK;
}


#endif  /* SELI_USE_SELECT */






/**********************************************************************************************
 *
 *                 Private functions used by POLL()
 *
 **********************************************************************************************/
#ifdef SELI_USE_POLL

static
int seliPollDetectEvent(
    IN  seliNode*       elem,
    IN  struct pollfd*  pollFdElem,
    OUT seliEvents*     sEvent,
    OUT BOOL*           error)
{
    *error =  FALSE;

    if ((elem->event & seliEvWrite) &&
        ((pollFdElem->revents&POLLOUT) || (pollFdElem->revents&POLLERR)))
    {
        *sEvent = seliEvWrite;
        *error = (pollFdElem->revents&POLLERR) ? TRUE : FALSE;
        return OK;
    }

    if ((elem->event & seliEvRead) &&
        ((pollFdElem->revents&POLLIN) || (pollFdElem->revents&POLLHUP)))
    {
        *sEvent = seliEvRead;
        return OK;
    }

    return RVERROR;
}

/* Poll-Events are handled according to  event prioriry */
/*  ACCEPT-READ EVENTS: I  priority */
/*  WRITE EVENTS:       II priority */
/*  READ  EVENTS:       III priority */
/*  EXEPT EVENTS:       IV priority */
int
seliPollEventsHandling(
               IN struct pollfd *pollFdSet,
               IN int  num,
               IN int numEvents
               )
{
    THREAD_SeliLocalStorage* seliTls;
    int i;
    BOOL error;
    seliEvents sEvent;
    seliNode* elem;
    struct pollfd *pollFdElem;
    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if (!seliTls)
        return RVERROR;
    if (numEvents < 0)
        return OK;

    meiEnter(seliTls->hMei);

    if (numEvents > 0)
    {
        pollFdElem = pollFdSet;

        /* Take care of Write/Read/Exept */
        for (i=0; i<num; i++)
        {
            elem = &(seliTls->nodes[pollFdElem->fd]);
            if ( seliPollDetectEvent(elem, pollFdElem, &sEvent, &error) == OK )
            {
                msaPrintFormat(msa, "seliPollEventsHandling: fd=%d event=%x callback=0x%p", pollFdElem->fd, sEvent, elem->callback);
                elem->callback(pollFdElem->fd, sEvent, error);
            }
            pollFdElem++;
        }
    }

    seliTimerExpired();
    meiExit(seliTls->hMei);
    return 0;
}

static int
seliPollAddFd2Set(struct pollfd *pollFdElem, int fd, seliEvents sEvent)
{
  pollFdElem->fd = fd;
  pollFdElem->events = 0;
  pollFdElem->revents = 0;
  if( sEvent & seliEvRead ) pollFdElem->events |=POLLIN;
  if( sEvent & seliEvWrite ) pollFdElem->events |= POLLOUT;

  return OK;
}

static int
seliPollList2PollSet(
         IN  THREAD_SeliLocalStorage* tNode,
         OUT struct pollfd *pollFdSet,
         OUT int *num
         )

{
    int i, fd;
    struct pollfd *pollFdElem;
    seliEvents sEvent;
    seliNode*       nodes ;


    for(i=0, fd=0,nodes = &tNode->nodes[0],pollFdElem = pollFdSet;i<seliFdSetSizeDyn;i++,nodes++)
    {
        if( nodes->event && nodes->valid)
        {
              sEvent = nodes->event;
              pollFdElem->fd = i;
              pollFdElem->events = pollFdElem->revents = 0;
              if( sEvent & seliEvRead ) pollFdElem->events |=POLLIN;
              if( sEvent & seliEvWrite ) pollFdElem->events |= POLLOUT;
              pollFdElem++;
              fd++;
        }
    }

    *num = fd;
    return OK;
}

int
seliPollEventsRegistration(
               IN  int len,
               OUT struct pollfd *pollFdSet,
               OUT int *num,
               OUT UINT32 *timeOut
               )
{
    THREAD_SeliLocalStorage* seliTls;
    UINT32 minT;
    INT32 t;
    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if (!seliTls) return RVERROR;
    if ( len < seliFdSetSizeDyn ) return RVERROR;

    meiEnter(seliTls->hMei);
    seliPollList2PollSet(seliTls, pollFdSet, num);

    minT = tsGetMinTime(mtiGetTimersHandle());

    if ( minT == (UINT32)-1 )
        *timeOut = (UINT32)-1;
    else
    {
        t = (int)minT - (int)timerGetTimeInMilliseconds();
        if ( t < 0)
            *timeOut = 0;
        else
            *timeOut = t;
    }

    msaPrintFormat(msa, "seliPollEventsRegistration: minT=%d timeout=%d", minT, *timeOut);

    meiExit(seliTls->hMei);
    return OK;
}

static int
seliPollInit(
         IN int len,
         OUT struct pollfd *pollFdSet
         )
{
    int i;
    for(i=0; i<len; i++)
    {
        pollFdSet[i].fd=0;
        pollFdSet[i].events=0;
        pollFdSet[i].revents=0;
    }
    return OK;
}


static int
seliPollEvents(
           IN int len,
           IN struct pollfd *pollSet
           )
{
  /*  UINT32 msec=0; */
  INT32 msec = 0;
  int num, retval;

  if(seliPollEventsRegistration( len , pollSet, &num, (UINT32 *)&msec) == RVERROR )
    return RVERROR;
  retval = poll(pollSet, num, msec);
  seliPollEventsHandling(pollSet, num, retval);

  return OK;
}


#endif  /* SELI_USE_POLL */







/**********************************************************************************************
 *
 *                 Private functions used by /DEV/POLL
 *
 **********************************************************************************************/
#ifdef SELI_USE_DEVPOLL

static
int seliDevPollDetectEvent(
    IN seliNode*        elem,
    IN struct pollfd*   pollFdElem,
    OUT seliEvents*     sEvent,
    OUT BOOL*           error)
{
    *error =  FALSE;

    if ((elem->event & seliEvWrite) &&
        ((pollFdElem->revents&POLLOUT) || (pollFdElem->revents&POLLERR)))
    {
        *sEvent = seliEvWrite;
        *error = (pollFdElem->revents&POLLERR) ? TRUE : FALSE;
        return OK;
    }

    if ((elem->event & seliEvRead) &&
        ((pollFdElem->revents&POLLIN) || (pollFdElem->revents&POLLHUP)))
    {
        *sEvent = seliEvRead;
        return OK;
    }

    return RVERROR;
}

int
seliDevPollEventsHandling(
    IN int              numEvents)
{
    THREAD_SeliLocalStorage* seliTls;
    int i;
    BOOL error;
    seliEvents sEvent;
    seliNode* elem;
    struct pollfd* pollFdElem;
    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if (!seliTls)
        return RVERROR;

    meiEnter(seliTls->hMei);

    if (numEvents > 0)
    {
        pollFdElem = seliTls->fdArray;

        /* Take care of Write/Read/Exept */
        for (i = 0; i < numEvents; i++)
        {
            elem = &(seliTls->nodes[pollFdElem->fd]);
            if ( seliDevPollDetectEvent(elem, pollFdElem, &sEvent, &error) == OK )
            {
                msaPrintFormat(msa, "seliDevPollEventsHandling: fd=%d event=%x callback=0x%p", pollFdElem->fd, sEvent, elem->callback);
                elem->callback(pollFdElem->fd, sEvent, error);
            }
            pollFdElem++;
        }
    }

    seliTimerExpired();
    meiExit(seliTls->hMei);
    return 0;
}

static int
seliDevPollAddFd2Set(
    IN  int         fdDevPoll,
    IN  int         fd,
    IN  seliEvents  sEvent,
    IN  BOOL        update,
    OUT INT32*      numFds)
{
    struct pollfd fdUpdate[2];
    int writeSize = sizeof(struct pollfd);

    if (sEvent == (seliEvents)0)
    {
        /* Removing */
#if (defined(IS_PLATFORM_SOLARIS) || defined(IS_PLATFORM_SOLARISPC)) /* NexTone */
        fdUpdate[0].fd = fd;
        fdUpdate[0].events = POLLREMOVE;
        fdUpdate[0].revents = 0;
#else
        fdUpdate[0].fd = fd;
        fdUpdate[0].events = 0;
        fdUpdate[0].revents = 0;
#endif

        (*numFds)--;
    }
    else
    {
        /* Adding */
        if (!update)
        {
            /* New fd to add to /dev/poll driver: */
            fdUpdate[0].fd = fd;
            fdUpdate[0].events = 0;
            if (sEvent & seliEvRead) fdUpdate[0].events |= POLLIN;
            if (sEvent & seliEvWrite) fdUpdate[0].events |= POLLOUT;
            fdUpdate[0].revents = 0;

            (*numFds)++;
        }
        else
        {
            /* Updating an existing fd inside /dev/poll driver: */
#if (defined(IS_PLATFORM_SOLARIS) || defined(IS_PLATFORM_SOLARISPC)) /* NexTone */
            /* On Solaris, we should remove existing mask and put the new one, since adding the new one as
               is will only update the existing mask. */
            fdUpdate[0].fd = fd;
            fdUpdate[0].events = POLLREMOVE;
            fdUpdate[0].revents = 0;
            fdUpdate[1].fd = fd;
            fdUpdate[1].events = 0;
            if (sEvent & seliEvRead) fdUpdate[1].events |= POLLIN;
            if (sEvent & seliEvWrite) fdUpdate[1].events |= POLLOUT;
            fdUpdate[1].revents = 0;

            writeSize = sizeof(struct pollfd) * 2;
#else
            fdUpdate[0].fd = fd;
            fdUpdate[0].events = 0;
            if (sEvent & seliEvRead) fdUpdate[0].events |= POLLIN;
            if (sEvent & seliEvWrite) fdUpdate[0].events |= POLLOUT;
            fdUpdate[0].revents = 0;
#endif

        }

    }

    /* Write the change to /dev/poll */
    if (write(fdDevPoll, fdUpdate, writeSize) != writeSize)
    {
        msaPrintFormat(msa, "seliDevPollAddFd2Set: Can't update fd=%d with %d (update=%d)", fd, sEvent, update);
        return RVERROR;
    }

    return OK;
}

static int
seliDevPollInit(
    IN  int             len,
    OUT struct pollfd*  pollFdSet,
    OUT int*            fdDevPoll)
{
    int i;

    /* Open /dev/poll file descriptor we'll use */
    if ((*fdDevPoll = open("/dev/poll", O_RDWR)) < 0)
        return RVERROR;

    for(i=0; i<len; i++)
    {
        pollFdSet[i].fd=0;
        pollFdSet[i].events=0;
        pollFdSet[i].revents=0;
    }

    return OK;
}


static int
seliDevPollEvents(
    IN struct pollfd*   fdArray,
    IN int              fdDevPoll,
    IN INT32            fdsUsed)
{
    UINT32 minT;
    INT32 t;
    int msec = 10;
    int numEvents;
    struct dvpoll dopoll;

#if 0 /* Nextone */
    /* Calculate the timeout to wait */
    minT = tsGetMinTime(mtiGetTimersHandle());
    if ( minT == (UINT32)-1 )
        msec = -1;
    else
    {
        t = (int)minT - (int)timerGetTimeInMilliseconds();
        if ( t < 0)
            msec = 0;
        else
            msec = (int)t;
    }
#endif

    msaPrintFormat(msa, "seliDevPollEvents: Polling for msec=%d, fdsUsed=%d", msec, fdsUsed);

    /* Create polling command for /dev/poll */
    dopoll.dp_timeout = msec;
    dopoll.dp_nfds = fdsUsed;
    dopoll.dp_fds = fdArray;

    /* Wait for I/O events that we're interested in */
    numEvents = ioctl(fdDevPoll, DP_POLL, &dopoll);
    if (numEvents == -1)
    {
        msaPrintFormat(msa, "seliDevPollEvents: Returned error while waiting for events (errno=%d)", errno);
        return RVERROR;
    }
	msaPrintFormat(msa, "seliDevPollEvents: Got %d events back", numEvents);
	seliDevPollEventsHandling(numEvents);

    return OK;
}


#endif  /* SELI_USE_DEVPOLL */



/* Nextone */	
/**********************************************************************************************
 *
 *                 Private functions used by /DEV/EPOLL
 *
 **********************************************************************************************/
#ifdef SELI_USE_DEVEPOLL

static
int seliDevEPollDetectEvent(
    IN seliNode*       			elem,
    IN struct epoll_event*   	epollEvent,
    OUT seliEvents*     		sEvent,
    OUT BOOL*           		error)
{
    *error =  FALSE;

    if ((elem->event & seliEvWrite) &&
        ((epollEvent->events&EPOLLOUT) || (epollEvent->events&EPOLLERR)))
    {
        *sEvent = seliEvWrite;
        *error = (epollEvent->events&EPOLLERR) ? TRUE : FALSE;
        return OK;
    }

    if ((elem->event & seliEvRead) &&
        ((epollEvent->events&EPOLLIN) || (epollEvent->events&EPOLLHUP)))
    {
        *sEvent = seliEvRead;
        return OK;
    }

    return RVERROR;
}

int
seliDevEPollEventsHandling(
    IN int              numEvents)
{
    THREAD_SeliLocalStorage		*seliTls;
    int 						i;
    BOOL 						error;
    seliEvents 					sEvent;
    seliNode					*elem;
	
    struct epoll_event			*epollEvent;
	
    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if (!seliTls)
        return RVERROR;

    meiEnter(seliTls->hMei);

    if (numEvents > 0)
    {
        epollEvent = seliTls->epollEvents;

        /* Take care of Write/Read/Exept */
        for (i = 0; i < numEvents; i++)
        {
            elem = &(seliTls->nodes[epollEvent->data.fd]);
            if ( seliDevEPollDetectEvent(elem, epollEvent, &sEvent, &error) == OK )
            {
                msaPrintFormat(msa, "seliDevEPollEventsHandling: fd=%d event=%x callback=0x%p", epollEvent->data.fd, sEvent, elem->callback);
                elem->callback(epollEvent->data.fd, sEvent, error);
            }
            epollEvent++;
        }
    }

    seliTimerExpired();
    meiExit(seliTls->hMei);
    return 0;
}

static int
seliDevEPollAddFd2Set(
    IN  int         fdDevEPoll,
    IN  int         fd,
    IN  seliEvents  sEvent,
    IN  BOOL        update,
    OUT INT32*      numFds)
{
    struct epoll_event  		epollEvent;
	int							operation = 0;

	memset(&epollEvent, 0, sizeof(epollEvent));
    if (sEvent == (seliEvents)0)
    {
        /* Removing */
    	epollEvent.data.fd = fd;
	    epollEvent.events = 0;
		operation = EPOLL_CTL_DEL;

        (*numFds)--;
    }
    else
    {
        /* Adding */
        if (!update)
        {
            /* New fd to add to /dev/epoll driver: */
    		epollEvent.data.fd = fd;
		    epollEvent.events = 0;
            if (sEvent & seliEvRead) epollEvent.events |= EPOLLIN;
            if (sEvent & seliEvWrite) epollEvent.events |= EPOLLOUT;
			operation = EPOLL_CTL_ADD;
            (*numFds)++;
        }
        else
        {
            /* Updating an existing fd inside /dev/epoll driver: */
    		epollEvent.data.fd = fd;
		    epollEvent.events = 0;
            if (sEvent & seliEvRead) epollEvent.events |= EPOLLIN;
            if (sEvent & seliEvWrite) epollEvent.events |= EPOLLOUT;
			operation = EPOLL_CTL_MOD;
        }
    }
    /* Write the change to /dev/epoll */
	if (epoll_ctl(fdDevEPoll, operation, fd, &epollEvent) < 0) {
        msaPrintFormat(msa, "seliDevEPollAddFd2Set: Can't update fd=%d with %d (update=%d)", fd, sEvent, update);
        return RVERROR;
	}

    return OK;
}

static int
seliDevEPollInit(
    IN  int             		len,
    OUT struct epoll_event*  	epollEvents,
    OUT int*            		fdDevEPoll)
{
    int i;

    /* Open /dev/epoll */
	if ((*fdDevEPoll = epoll_create(len)) < 0) 
        return RVERROR;

    for(i=0; i<len; i++)
    {
        /* epollEvents[i].data.fd=0;*/
        epollEvents[i].events=0;
        memset(&epollEvents[i].data, 0, sizeof(epoll_data_t));
    }

    return OK;
}


static int
seliDevEPollEvents(
    IN struct epoll_event	*epollEvents,
    IN int              	fdDevEPoll,
    IN INT32            	fdsUsed)
{
    UINT32 			minT;
    INT32		 	t;
    int				msec = 10;
    int 			numEvents;

    /*struct dvpoll dopoll;*/

#if 0 /* Nextone */
    /* Calculate the timeout to wait */
    minT = tsGetMinTime(mtiGetTimersHandle());
    if ( minT == (UINT32)-1 )
        msec = -1;
    else
    {
        t = (int)minT - (int)timerGetTimeInMilliseconds();
        if ( t < 0)
            msec = 0;
        else
            msec = (int)t;
    }
#endif

    msaPrintFormat(msa, "seliDevEPollEvents: Polling for msec=%d, fdsUsed=%d", msec, fdsUsed);

    /* Create polling command for /dev/epoll */
    /* Wait for I/O events that we're interested in */
	numEvents = epoll_wait(fdDevEPoll, epollEvents, fdsUsed, msec);

    if (numEvents == -1)
    {
        msaPrintFormat(msa, "seliDevEPollEvents: Returned error while waiting for events (errno=%d)", errno);
        return RVERROR;
    }
	msaPrintFormat(msa, "seliDevEPollEvents: Got %d events back", numEvents);
	seliDevEPollEventsHandling(numEvents);

    return OK;
}


#endif  /* SELI_USE_DEVEPOLL */
/* Nextone */	





/**********************************************************************************************
 *
 *                 General seli functions
 *
 **********************************************************************************************/




int seliInitializeThread(IN RvH323ThreadHandle threadId)
{
    THREAD_SeliLocalStorage* seliTls;
    int res = TRUE;

    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrSeli, seliAllocSize);

    msaPrintFormat(msa, "seliInitializeThread: #calls=%d.", seliInitCalls);

    seliTls->counter++;
    if (seliTls->counter == 1)
    {
        seliTls->nodes = (seliNode *)((char *)seliTls + (int)sizeof(THREAD_SeliLocalStorage));

        memset(seliTls->nodes, 0, sizeof(seliNode)*seliFdSetSizeDyn/*sizeof(tNode->nodes)*/);

        seliTls->counter = 1;

#ifndef NOTHREADS
        seliTls->hMei = meiInit();

        if (seliCreatePipe(seliTls) < 0)
        {
            msaPrintFormat(msa, "seliInititializeThread(%d): Create seli notification failed.", seliInitCalls);
            return RVERROR;
        }
#endif

#ifdef SELI_USE_SELECT
        res = seliSelectInit(&(seliTls->selectRd), &(seliTls->selectWr), &(seliTls->selectEx));
#endif  /* SELI_USE_SELECT */

#ifdef SELI_USE_POLL
        seliTls->fdArray = (struct pollfd *)(
            (char*)seliTls +
            (int)sizeof(THREAD_SeliLocalStorage) +
            (int)(sizeof(seliNode) * seliFdSetSizeDyn));
        res = seliPollInit(seliFdSetSizeDyn, seliTls->fdArray);
#endif  /* SELI_USE_POLL */

#ifdef SELI_USE_DEVPOLL
        seliTls->fdsUsed = 0;
        seliTls->fdArray = (struct pollfd *)(
            (char*)seliTls +
            (int)sizeof(THREAD_SeliLocalStorage) +
            (int)(sizeof(seliNode) * seliFdSetSizeDyn));
        res = seliDevPollInit(seliFdSetSizeDyn, seliTls->fdArray, &seliTls->fdDevPoll);
#endif  /* SELI_USE_DEVPOLL */

/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
        seliTls->fdsUsed = 0;
        seliTls->epollEvents = (struct epoll_event *)(
            (char*)seliTls +
            (int)(sizeof(THREAD_SeliLocalStorage)) +
            (int)(sizeof(seliNode) * seliFdSetSizeDyn));
        res = seliDevEPollInit(seliFdSetSizeDyn, seliTls->epollEvents, &seliTls->fdDevEPoll);
#endif  /* SELI_USE_DEVEPOLL */
    }

    if (res < 0)
    {
        msaPrintFormat(msa, "seliInitiazlieThread(%d): Cannot initialize OS driver", seliInitCalls);
        return RVERROR;
    }

    return TRUE;
}


int
seliEndThread(IN RvH323ThreadHandle threadId)
{
    THREAD_SeliLocalStorage* seliTls;
    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrSeli, seliAllocSize);

    if (seliTls == NULL) return TRUE;
    seliTls->counter--;

    if (seliTls->counter <= 0)
    { /* final end -> destroy element */
#ifndef NOTHREADS
        close(seliTls->pipefd[WRITE_FD]);
        close(seliTls->pipefd[READ_FD]);
        meiEnd(seliTls->hMei);
#endif

#ifdef SELI_USE_DEVPOLL
        close(seliTls->fdDevPoll);
#endif
		
/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
        close(seliTls->fdDevEPoll);
#endif
    }

    if (seliInitCalls < 0)
        msaPrintFormat(msa, "seliEndThread: too many calls. [%d].", seliInitCalls);

    return TRUE;
}



/**********************************************************************************************
 *
 *                 Public functions
 *
 **********************************************************************************************/



/*
  Desc: Init SELI module.
  Init list of events and SELI.
  */

RVAPI int RVCALLCONV seliInit(void)
{
    RvH323ThreadHandle threadId;

    /* Calculate size of local storage we need */
    if (seliAllocSize == 0)
        seliAllocSize =
            sizeof(THREAD_SeliLocalStorage) +
            sizeof(seliNode) * seliFdSetSizeDyn
#ifdef SELI_USE_POLL
            + sizeof (struct pollfd) * (seliFdSetSizeDyn)
#endif
#ifdef SELI_USE_DEVPOLL
            + sizeof (struct pollfd) * (seliFdSetSizeDyn)
#endif
/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
            + sizeof (struct epoll_event) * (seliFdSetSizeDyn)
#endif
            ;

    /* Get the threadId for this thread. We do that here since we might not have any TLS created for it yet, and
       this call will create it. */
    threadId = RvH323ThreadGetHandle();

    seliInitCalls++;
    if (seliInitCalls == 1)
    {
        /* seliMuxSemH = semiConstruct(); */
        msa = msaRegister(0, "SELI", "Select Interface");
    }

    msaPrintFormat(msa, "seliInit: #calls=%d.", seliInitCalls);

    return seliInitializeThread(NULL);
}


/*
  Desc: End module operation.
  */
RVAPI int RVCALLCONV seliEnd(void)
{
    int ret;

    msaPrintFormat(msa, "seliEnd #calls=%d.", seliInitCalls);
    seliInitCalls--;

    ret = seliEndThread(NULL);

    if (seliInitCalls == 0)
    {
        msaPrintFormat(msa, "seliEnd: Completed.");
        msaUnregister(msa);
    }

    return ret;
}


#ifndef NOTHREADS
int seliPreempt(IN RvH323ThreadHandle threadId)
{
    int res;
    THREAD_SeliLocalStorage* seliTls;

    if (threadId == RvH323ThreadGetHandle())
    {
        /* This is the same thread as the one running right now - no need to preempt */
        return 0;
    }

    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrSeli, seliAllocSize);

    res = write(seliTls->pipefd[WRITE_FD], " ", 1);
    if (res == -1 && errno == EAGAIN)
    {
        logPrint((RVHLOG)msa, RV_ERROR,
                 ((RVHLOG)msa, RV_ERROR,
                 "seliPreempt: I would have been blocked writing to pipe"));
    }
    return res;
}
#endif





/*____________________________registration_______________________________________*/
/*
  Desc: Register new event for descriptor. The descriptor is registered for
   select()ing when using the seliSelect().
   Note: To unregister the socket, call the function with lEvent = 0
         Example: seliCallOn(fd, 0, 0,FALSE);
   Logic: if socket is already registered, than last registration is erased.
   Note: non positive descriptor value is considered invalid   */
RVAPI
int RVCALLCONV seliCallOn(int fd, seliEvents sEvent, seliCallback _callback)
{
    return seliCallOnThread(fd, sEvent, _callback, NULL);
}


RVAPI
int RVCALLCONV seliCallOnThread(int fd, seliEvents sEvent, seliCallback _callback, RvH323ThreadHandle threadId)
{
    seliNode* elem;
    seliNode* nodes;
    THREAD_SeliLocalStorage* seliTls;
    char strEvent[32];

    if (threadId == NULL)
        threadId = RvH323ThreadGetHandle();

    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrSeli, seliAllocSize);

    if (fd < 0 || fd > seliFdSetSizeDyn || !seliTls)
    {
        msaPrintFormat(msa, "seliCallOn: Illegal Descriptor [%d].", fd);
        return RVERROR;
    }
    nodes = seliTls->nodes;
    elem = &nodes[fd];

    msaPrintFormat(msa, "seliCallOn: fd=%d, %s (0x%x), callback=0x%x.",
        fd, nprn(seliEvent2Str(sEvent, strEvent)), sEvent, _callback);

    if (!sEvent || !_callback)
    { /* -- unregistration */
#ifdef SELI_USE_SELECT
        seliSelectAdd2fdSet(&(seliTls->selectRd), &(seliTls->selectWr), &(seliTls->selectEx), fd,(seliEvents)0);
#endif  /* SELI_USE_SELECT */

#ifdef SELI_USE_POLL
        seliPollAddFd2Set(seliTls->fdArray, fd, (seliEvents)0);
#endif  /* SELI_USE_POLL */

#ifdef SELI_USE_DEVPOLL
        if (elem->valid)
            seliDevPollAddFd2Set(seliTls->fdDevPoll, fd, (seliEvents)0, FALSE, &seliTls->fdsUsed);
#endif  /* SELI_USE_DEVPOLL */

/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
        if (elem->valid)
            seliDevEPollAddFd2Set(seliTls->fdDevEPoll, fd, (seliEvents)0, FALSE, &seliTls->fdsUsed);
#endif  /* SELI_USE_DEVEPOLL */
        /* Clear this element in internal db */
        elem->valid = FALSE;
        elem->event= (seliEvents)0;
        elem->callback= NULL;

        /* Check if we have to preempt the thread because we updated an FD for it */
#ifndef NOTHREADS
        if (threadId != RvH323ThreadGetHandle())
            seliPreempt(threadId);
#endif  /* NOTHREADS */

        return TRUE;
    }

#ifdef SELI_USE_DEVPOLL
    /* Registration of /dev/poll fd's is done here and not just prior to the selection call.
       We only do the registration if we have any change in the events we're looking for from
       this fd. */
    if ((sEvent != elem->event) && (_callback != NULL))
    {
        seliDevPollAddFd2Set(seliTls->fdDevPoll, fd, sEvent, elem->valid, &seliTls->fdsUsed);
    }
#endif  /* SELI_USE_DEVPOLL */

/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
    /* Registration of /dev/epoll fd's is done here and not just prior to the selection call.
       We only do the registration if we have any change in the events we're looking for from
       this fd. */
    if ((sEvent != elem->event) && (_callback != NULL))
    {
        seliDevEPollAddFd2Set(seliTls->fdDevEPoll, fd, sEvent, elem->valid, &seliTls->fdsUsed);
    }
#endif  /* SELI_USE_DEVEPOLL */

    /* -- update */
    elem->valid = TRUE;
    elem->event = sEvent;
    elem->callback = _callback;

    /* Check if we have to preempt the thread because we updated an FD for it */
#ifndef NOTHREADS
    if (threadId != RvH323ThreadGetHandle())
        seliPreempt(threadId);
#endif  /* NOTHREADS */

    return TRUE;
}





/*************************************************************************/
/*                SELECT FUNCTION                                        */
/*************************************************************************/
RVAPI
int RVCALLCONV seliSelect(void)
{
    THREAD_SeliLocalStorage* seliTls;
    seliTls = (THREAD_SeliLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrSeli, seliAllocSize);

    if (seliTls == NULL) return RVERROR;

#ifndef NOTHREADS
    /* Make sure we register on the pipe in multithreaded environment.
       This way, we'll be able to preempt this thread when needed. */
    seliCallOn(seliTls->pipefd[READ_FD], seliEvRead, pipeCallback);
#endif

#ifdef SELI_USE_SELECT
    seliSelectEvents(seliFdSetSizeDyn, &(seliTls->selectRd), &(seliTls->selectWr), &(seliTls->selectEx));
#endif  /* SELI_USE_SELECT */

#ifdef SELI_USE_POLL
    seliPollEvents(seliFdSetSizeDyn, seliTls->fdArray);
#endif  /* SELI_USE_POLL */

#ifdef SELI_USE_DEVPOLL
    seliDevPollEvents(seliTls->fdArray, seliTls->fdDevPoll, seliTls->fdsUsed);
#endif  /* SELI_USE_DEVPOLL */

/* Nextone */	
#ifdef SELI_USE_DEVEPOLL
    seliDevEPollEvents(seliTls->epollEvents, seliTls->fdDevEPoll, seliTls->fdsUsed);
#endif  /* SELI_USE_DEVEPOLL */
    return OK;
}







/*                                   PUBLIC FUNCTIONS                                      */



int
seliSetMaxDescs(int maxDescs)
{
 if (seliInitCalls || maxDescs<0)
    return RVERROR;
  seliFdSetSizeDyn = maxDescs;
  return TRUE;
}

int
seliGetMaxDescs(void)
{

  return seliFdSetSizeDyn;
}





/* The following functions are here for backward compatability only.
   They have no purpose besides that. */

int
seliSetMaxTasks(int maxTasks)
{
    if (maxTasks);
    return TRUE;
}

int
seliGetMaxTasks(void)
{
    return -1;
}



#ifdef __cplusplus
}
#endif



