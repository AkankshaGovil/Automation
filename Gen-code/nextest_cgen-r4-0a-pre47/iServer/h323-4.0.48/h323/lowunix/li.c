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
  li.h

  LAN Interface.
  Provides interface to the system socket functions.
  Handles TCP and UDP protocols.

  Revised By: Ron S. 26 Nov. 1996. Multithread treatment. Semaphores.

  Revised By: Ron S. 30 Sep. 1996. Change behaviour of read and write events. (as in Windows)
   Only once the event is notified, and it is re-registered after calling the read/write function.

  Revised By: Ron S.  14 Jan 1996
  Updated for multi-platform operation and the new interfaces.
  Integrated from the li and lih versions for windows and gateway.

  Revised By: Sergey M.  7 February 1999
  Function liBytesAvailable() was updated. Now its OUT parameter
  is a negative value that equals return value of ioctl(), if the
  last is negative, and NOT a garbage, as it was on UNIXWARE
  platform in this case.

  Origin: Sasha, Dani. Gateway and Windows versions.

  Supporting platforms:
  - SunOS
  - IRIX
  - HP-UX
  - Linux
  - i960

  Parameter Description:
  - handle: socket number.
  - Returns: RVERROR if fault occurred.


  Supplumments:
  LI module uses a list to hold incoming events (from the application).
  LI registers callon requests in SELI.

  */

#if (defined(__PSOSPNA__) || defined(__VXWORKS__) || defined(UNDER_CE)|| defined(RV_OS_OSE))
#define RV_EMBEDDED
#endif

#ifdef __PSOSPNA__
/* ---------------------- pSOS -------------------- */
#include <pna.h>
#define PF_INET AF_INET
#define FIONREAD FIOREAD

#include <errno.h>

#ifndef __CADUL__               /* AL - 06/10/99 */
#include <sys/types.h>
#include <memory.h>
#endif  /* __CADUL__ */

#define __Iunistd

typedef struct sockaddr_in * SOCKADDRPTR;  /* AL - 09/17/98 */
typedef char * ioctlOnPtrTypeCast;     /* AL - 10/28/98 */
typedef int sockaddr_namelen;

#define ERRNOGET errno
#define ERRNOSET(a) errno = a



#elif __VXWORKS__
/* ---------------------- vxWorks -------------------- */
#include <vxWorks.h>
#include <sys/types.h>
#include <stdio.h>
#include <msgQLib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sockLib.h>
#include <routeLib.h>
#include <net/protosw.h>
#include <usrConfig.h>
#include <inetLib.h>
#include <ioLib.h>
#include <ioctl.h>
#include <string.h>
#include <errnoLib.h>
#include <rvinternal.h>

typedef struct sockaddr * SOCKADDRPTR;  /* AL - 09/17/98 */
typedef INTPTR ioctlOnPtrTypeCast;         /* AL - 10/28/98 */
typedef int sockaddr_namelen;

#define ERRNOGET errnoGet()
#define ERRNOSET(a) errnoSet(a)

#elif RV_OS_OSE
/* ---------------------- OSE -------------------- */
#include <ose.h>
#include <string.h>
#include <inet.h>
#include <netdb.h>

typedef struct sockaddr * SOCKADDRPTR;
typedef char * ioctlOnPtrTypeCast;
typedef int sockaddr_namelen;

#define ERRNOGET errno
#define ERRNOSET(a) errno = a

#elif UNDER_CE
/* ---------------------- Windows CE -------------------- */
#pragma warning (disable : 4201 4214)

#include <windows.h>
#include <winsock.h>

typedef struct sockaddr * SOCKADDRPTR;  /* AL - 09/17/98 */
typedef unsigned long * ioctlOnPtrTypeCast;         /* AL - 11/25/98 */
typedef int sockaddr_namelen;
#define ioctl ioctlsocket
#define close closesocket

#define ERRNOGET GetLastError()
#define ERRNOSET(a) SetLastError(a)



#else
/* ---------------------- UNIX -------------------- */
#include <sys/types.h>
#include <rvinternal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <errno.h>
#include <sys/time.h>
#include <memory.h>

typedef struct sockaddr * SOCKADDRPTR;  /* AL - 09/17/98 */
typedef INTPTR ioctlOnPtrTypeCast;         /* AL - 10/28/98 */

#ifdef UNIXWARE
typedef size_t sockaddr_namelen;
#elif __REDHAT__
typedef socklen_t sockaddr_namelen;
#else
typedef int sockaddr_namelen;
#endif

#define ERRNOGET errno
#define ERRNOSET(a) errno = a


#endif  /* __VXWORKS__ / __PSOSPNA__ / UNDER_CE */



/* Specific UNIX operating systems */

#ifdef UNIXWARE
#include <sys/xti.h>
#include <sys/sockmod.h>
#include <sys/osocket.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#endif

#ifndef __REDHAT__
#ifdef __LINUX__
#include <linux/termios.h>
#endif
#endif

#ifdef __REDHAT__
int gethostname(char* name, size_t len);
#endif

#if (defined(IS_PLATFORM_SOLARIS) || defined(IS_PLATFORM_SOLARISPC))
#include <sys/filio.h>
#define RV_SOLARIS8 /* Nextone */
#endif

#ifdef IS_PLATFORM_HPUX
#include <sys/ioctl.h>
#endif


/* General include files of the stack */
#include <rvinternal.h>
#include <li.h>
#include <seli.h>
#include <sigcatch.h>
#include <mei.h>

#include <ra.h>
#include <rid.h>
#include <ms.h>


#define liGetMsa()        msa
#define liMsaInfo()       msaInfo



#define LINGER_TIME  0
typedef struct {
  char *pName;
  int  protNum;
  int  protType;
} rvprot_t;

rvprot_t liProtData [2] =
{
  {(char*)"UDP",IPPROTO_UDP,SOCK_DGRAM},
  {(char*)"TCP",IPPROTO_TCP,SOCK_STREAM}
};


/* LI list of nodes. */
typedef struct
{
  int                   valid;
  liEvents              event; /* on what events the application want to be notified. */
  LPLIEVENTHANDLER      callback; /* callback function */
  void*                 context; /* of application */
  liProtocol_t          protocol;
  RvH323ThreadHandle    threadId;
} liNode;


typedef struct
{
    int messageLoops;
} THREAD_LiLocalStorage;


static int msa=0;
static int msaInfo=0;

#if !( defined(__PSOSPNA__) || defined(__VXWORKS__) ||defined(RV_OS_OSE))
static UINT32  myIPs[10] = {0};
#endif

static INT32 liFdSetSizeDyn;
liNode* liNodes;/*index of this array is the socket descriptor*/
static HRID liPortsH = NULL;
HMEI liIntMei;

void liCallback(int socketId, seliEvents sEvent, BOOL error);
int liGetSocketPort(IN int socketId, OUT UINT16* port);

int seliGetMaxDescs(void);
int seliInitializeThread(IN RvH323ThreadHandle threadId);
int seliEndThread(IN RvH323ThreadHandle threadId);

/* adding a function for OSE - gethostname */
#ifdef RV_OS_OSE
int gethostname(char *buf, size_t size);
#endif



int liCheckIsValidFd(int handle)
{
  if( (handle<0) || (handle >= liFdSetSizeDyn))
    {
       fprintf(stderr, "    >>> li:liCheckIsValidFd socket=%d ", handle);
       perror("Error:");
       return RVERROR;
    }
  return OK;
}

/*________________________task array functions____________________________*/
/*
void *
liTaskNodePrint(RAElement elem, void *param)
{
  liTaskNode *e1 = (liTaskNode *)elem;
  int cur;

  if (!e1) return param;
  msaPrintF(liGetMsa(), "  id=%d, list=0x%p, counter=%d.\n",
        e1->taskId, e1->nodes, e1->counter);

  for (cur=0;cur<liFdSetSizeDyn;cur++)
  {
      if (e1->nodes[cur].valid)
    liNodePrint(e1->nodes+cur, cur);
  }

  return param;
}

*/


/* convert events to string */
static char * /* strEv */
liEvent2Str(
        IN  liEvents lEvent,
        OUT char* strEv) /* name of events */
{
  if (!strEv) return (char*)"(null)";

  strEv[0] = 0;
  if (lEvent & liEvRead)    strcat(strEv, " Read ");
  if (lEvent & liEvAccept)  strcat(strEv, " Accept ");
  if (lEvent & liEvClose)   strcat(strEv, " Close ");
  if (lEvent & liEvConnect) strcat(strEv, " Connect ");
  if (lEvent & liEvWrite)   strcat(strEv, " Write ");
  if (lEvent & ~(liEvRead|liEvAccept|liEvClose|liEvConnect|liEvWrite))
    strcat(strEv, " Unknown ");
  return strEv;
}

/*
void
liNodePrint(liNode* e1, int fd)
{
  char strEv[100];

  if (!e1) return;
  msaPrintF(liGetMsa(), "  [%d] callback=0x%x, context=%p, event=%s.\n",
          fd, e1->callback, e1->context, nprn(liEvent2Str(e1->event, strEv)));
}
*/


/*____________________________module functions____________________________________*/

static INT32 liInitCalls = 0;


/*
  Desc: Init LI module.
  Init list of events and SELI.
  */
int
liInit(void)
{
    THREAD_LiLocalStorage*  liTls;

    liTls = (THREAD_LiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrLi, sizeof(THREAD_LiLocalStorage));
    if (liTls == NULL) return RVERROR;

    liInitCalls++;
    if (liInitCalls == 1)
    {
        /* liMuxSemH = semiConstruct(); */
        liFdSetSizeDyn = seliGetMaxDescs();

#ifdef UNDER_CE
        {
            WSADATA wsadata;
            if (WSAStartup(0x0101,&wsadata) != 0)
                return RVERROR;
        }
#elif (defined(__PSOSPNA__) || defined(RV_OS_OSE))
        /* Nothing to initialize in pSOS that is special */
#else
        /* All other OSes should catch socket signals and ignore them */
        sigCatch();
#endif

      msa = msaRegister(0, "LI", "LAN Interface");
      msaInfo = msaRegister(0, "LIINFO", "Lan Interface Information");

      liNodes = (liNode*)calloc(sizeof(liNode),liFdSetSizeDyn);
      if (!liNodes)
        return RVERROR;

      memset(liNodes,0,sizeof(liNode)*liFdSetSizeDyn);

      liIntMei = meiInit();
    }

    liTls->messageLoops++;

    return seliInit();
}

void
liSetPortsRange(
        IN UINT16 from,
        IN UINT16 to)
{
  INT32 lFrom = (INT32)from;
  INT32 lTo   = (INT32)to;

  meiEnter(liIntMei);
  liPortsH = ridConstruct(lFrom, lTo);
  meiExit(liIntMei);
}

/*
  Desc: End module operation.
  */
int
liEnd(void)
{
    THREAD_LiLocalStorage*  liTls;

    liTls = (THREAD_LiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrLi, sizeof(THREAD_LiLocalStorage));

    meiEnter(liIntMei);
    if (liInitCalls>0)
        liInitCalls--;
    liTls->messageLoops--;

    if (liInitCalls==0)
    {
        free(liNodes);

        msaUnregister(msa);
        if (liPortsH) ridDestruct(liPortsH);
#if !( defined(__PSOSPNA__) || defined(__VXWORKS__) || defined(RV_OS_OSE) )
        myIPs[0] = 0;
#endif
    }

    seliEnd();
    meiExit(liIntMei);
    if (liInitCalls == 0)
    {
        meiEnd(liIntMei);
#ifdef UNDER_CE
        WSACleanup();
#endif
    }
    return TRUE;
}

void
liCloseAll(void)
{
  int cur;

  if (!liNodes) return;

  for (cur=0;cur<liFdSetSizeDyn;cur++)
  {
      if (liNodes[cur].valid)
    liClose(cur);
  }
}


/*____________________________registration_______________________________________*/


static seliEvents
liEventTranslate(
         /* Translate li event to seli event */
         IN  liEvents lEvent
         )
{

  seliEvents sEvent = (seliEvents)0;

  if ((lEvent & liEvRead) || (lEvent & liEvAccept) || (lEvent & liEvClose))
              sEvent = (seliEvents)(sEvent|seliEvRead);
  if ((lEvent & liEvWrite) || (lEvent & liEvConnect))
              sEvent = (seliEvents)(sEvent|seliEvWrite);


  return sEvent;
}


/************************************************************************
 * liCallOnInstance
 * purpose: Wait for an event for a specific stack instance.
 * input  : socket          - Socket to wait for
 *          lEvent          - Events to wait for
 *                            0 for removing any waiting events
 *          eventHandler    - Callback function to call when event occurs
 *                            Should be set to NULL if we're removing any
 *                            waiting events
 *          context         - Context to use with callback function
 *          threadId        - Thread of the stack instance using the socket
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int liCallOnInstance(
    IN int                  socket,
    IN liEvents             lEvent,
    IN LPLIEVENTHANDLER     eventHandler,
    IN void*                context,
    IN RvH323ThreadHandle   threadId)
{
    THREAD_LiLocalStorage* liTls;

    /* See if the current thread has a message loop */
    liTls = (THREAD_LiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrLi, sizeof(THREAD_LiLocalStorage));
    if (liTls->messageLoops > 0)
    {
        /* There's a message loop - use the current thread */
        return liCallOnThread(socket, lEvent, eventHandler, context, NULL);
    }
    else
    {
        /* No message loop - use the thread of the instance */
        return liCallOnThread(socket, lEvent, eventHandler, context, threadId);
    }
}


/************************************************************************
 * liCallOnThread
 * purpose: Wait for an event on a specific thread.
 * input  : socket          - Socket to wait for
 *          lEvent          - Events to wait for
 *                            0 for removing any waiting events
 *          eventHandler    - Callback function to call when event occurs
 *                            Should be set to NULL if we're removing any
 *                            waiting events
 *          context         - Context to use with callback function
 *          threadId        - Thread to return the callback in
 *                            If NULL, then current thread is used
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int liCallOnThread(
    IN int                  socketId,
    IN liEvents             lEvent,
    IN LPLIEVENTHANDLER     eventHandler,
    IN void*                context,
    IN RvH323ThreadHandle   threadId)
{
  liNode *elem;
  char strEv[100];
  liNode* nodes = liNodes;
  BOOL isNewNode=FALSE;
  THREAD_LiLocalStorage* liTls;

  if ( (socketId<0) ||  (! nodes))
  {
    msaPrintFormat(msa, "liCallOnThread: Illegal Descriptor [%d].", socketId);
    return RVERROR;
  }

  if (threadId == NULL)
      threadId = RvH323ThreadGetHandle();

  /* Make sure we've got a select loop in this thread */
  liTls = (THREAD_LiLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrLi, sizeof(THREAD_LiLocalStorage));
  if (liTls->messageLoops == 0)
  {
      logPrint((RVHLOG)msa, RV_ERROR,
               ((RVHLOG)msa, RV_ERROR,
               "liCallOnThread: No seliSelect() loop in this thread!"));
      return RVERROR;
  }

  if (!lEvent || !eventHandler)
  { /* -- unregistration */
    if (nodes[socketId].valid)
      nodes[socketId].valid=FALSE;
    nodes[socketId].callback = NULL;
    nodes[socketId].event=(liEvents)0;
    nodes[socketId].context=0;
    return seliCallOnThread(socketId, (seliEvents)0, NULL, nodes[socketId].threadId);
  }

  /* -- register socket */
  msaPrintFormat(msa, "liCallOnThread: s=%d %s (%x).", socketId, nprn(liEvent2Str(lEvent, strEv)), lEvent);

  if (!nodes[socketId].valid)
  { /* add new item */
    nodes[socketId].valid=TRUE;
    isNewNode=TRUE;
  }

  { /* -- update */
    elem = &nodes[socketId];
    elem->event = lEvent;
    elem->callback = eventHandler;
    elem->context = context;
    elem->threadId = threadId;

    if (isNewNode) elem->protocol = LI_UDP;
    if ((lEvent & liEvConnect) || (lEvent & liEvAccept) || (lEvent & liEvClose))
      elem->protocol = LI_TCP;

    return seliCallOnThread(socketId, liEventTranslate(lEvent), liCallback, elem->threadId);
  }
}


static int liCallOnForSeli(
     IN  int socketId,
     IN  liEvents lEvent
     )
{
    if ((socketId<0) ||  (! liNodes))
    {
        msaPrintFormat(msa, "liCallOnForSeli: Illegal Descriptor [%d].", socketId);
        return RVERROR;
    }

    return seliCallOnThread(socketId, liEventTranslate(lEvent), liCallback, liNodes[socketId].threadId);
}

int /* TRUE or RVERROR */
liCallOn(
     IN  int socketId,
     IN  liEvents lEvent,
     IN  LPLIEVENTHANDLER callback,
     IN  void* context)
{
    return liCallOnThread(socketId, lEvent, callback, context, NULL);
}


/*
  Desc: handle events from select.
  Call application callback function with proper parameters.
  */
void
liCallback(int socketId, seliEvents sEvent, BOOL error)
{
  liNode *elem;
  liEvents lEvent=(liEvents)0;
  int bytes;
  char strEv[100];
  seliEvents oldSeliEvent = (seliEvents)0;
  struct sockaddr Name; /* for getpeername() */
  sockaddr_namelen NameSize = (sockaddr_namelen)sizeof(struct sockaddr); /* for getpeermane() */

  /* if (error) perror("liCallback:"); */
  /*msaPrintFormat(liGetMsa(), "liCallback: s=%d, event=%x, error=%d.", socketId, sEvent, error);*/

  liBytesAvailable(socketId, &bytes);

  /* semiTake(liMuxSemH); */ /* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
  elem = liNodes + socketId;
  oldSeliEvent = liEventTranslate(elem->event);
  /* semiGive(liMuxSemH); */ /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

  if (!elem) {
    msaPrintFormat(liGetMsa(), "liCallback: Received event [0x%x] on unregistered socket [%d].\n",
           sEvent, socketId);
    return;
  }


  if (elem->event & liEvRead    && sEvent & seliEvRead && bytes>0)  lEvent=liEvRead;
  if (elem->event & liEvWrite   && sEvent & seliEvWrite)            lEvent=liEvWrite;
  if (elem->event & liEvClose   && sEvent & seliEvRead && bytes<=0)
  {
#ifdef RV_SOLARIS8
      /* Fix for Solaris 8 - we might have 0 bytes to read, but we still won't have
         to close the connection */
        char buffer[10];
        if ((bytes == 0 ) && (recv(socketId, buffer,1, MSG_PEEK|MSG_DONTWAIT ) >  0))
            lEvent=liEvRead;
        else
            lEvent=liEvClose;
#else
        lEvent=liEvClose;
#endif
  }

  if (elem->event & liEvAccept  && sEvent & seliEvRead)             lEvent=liEvAccept;
  if (elem->event & liEvConnect && sEvent & seliEvWrite)            lEvent=liEvConnect;
  if (elem->protocol == LI_UDP &&
      elem->event & liEvRead    && sEvent & seliEvRead)             lEvent=liEvRead;

  oldSeliEvent = (seliEvents)(oldSeliEvent & ~sEvent);     /* unregister reading/writing on socket */

  if (lEvent & (liEvWrite) || /* write event */
      /* read event without registration */
      (sEvent & seliEvRead && bytes>0 && !(elem->event&liEvRead || elem->event&liEvAccept ))
     )
  {
    msaPrintFormat(liMsaInfo(), "liCallback: Resseting socket %d to %x.", socketId, oldSeliEvent);
    seliCallOnThread(socketId, oldSeliEvent, liCallback, elem->threadId);
  }

  msaPrintFormat(liMsaInfo(), "liCallback: s=%d, {%s} (len=%d) %s [call %p(%p)].",
         socketId, nprn(liEvent2Str(lEvent, strEv)), bytes,
         (error)?"Error!":"ok",
         (elem->callback), (elem->context));
  if (!lEvent)
  {
    msaPrintFormat(liGetMsa(), "liCallback:Error: s=%d, levent=%d, elem {%s}, seli event {%s} bytes=%d.",
        socketId, lEvent, nprn(liEvent2Str(elem->event, strEv)),
        nprn(liEvent2Str((liEvents)sEvent, strEv)), bytes);
  }

  /* Mila */
  /* Make getpeername for write event */

  if (sEvent & seliEvWrite) {
#ifdef UNIXWARE
    if (getpeername(socketId, &Name, &NameSize) < 0) {
      error = ((error || ( ERRNOGET !=ENOTCONN)))?TRUE:FALSE; /* UnixWare bizzare behavior */
    }
    else
      error=(error)? TRUE:FALSE;
#else
    error = ((error)||(getpeername(socketId, (SOCKADDRPTR)&Name, &NameSize) < 0))?(TRUE):(FALSE);
#endif

  }

  if (lEvent && (elem->callback != NULL) )  elem->callback(socketId, lEvent, error, elem->context);
  /* if (lEvent == liEvClose) liClose(socketId); */
}



/*_____________________________socket functions__________________________________*/

int
liGetDynPort(int s,
         struct sockaddr_in *sin)
{
    int status;
    int rid;
    UINT16 port, portsave=0;
    INT32  lPort;
    BOOL first = TRUE;
    int portsToFail;

    portsToFail = (ridTo(liPortsH) - ridFrom(liPortsH)) + 10;

    do
    {
        meiEnter(liIntMei);
        rid = ridNew(liPortsH);
        if (rid <0)
        {
            status = RVERROR;
            break;
        }
        else
            port = (UINT16) rid;
        meiExit(liIntMei);
        sin->sin_port        = htons(port);
        if ((status = bind (s, (SOCKADDRPTR)sin ,sizeof (*sin))) <0)
            switch (ERRNOGET)
            {
                case EADDRINUSE:
                case EADDRNOTAVAIL:
#if ( !defined( __PSOSPNA__ ) && !defined(UNDER_CE) )
                case EACCES:
#endif
                case EAFNOSUPPORT:
                case EOPNOTSUPP:
                    if (first)
                    {
                        first = FALSE;
                        portsave = port;
                    }
                    meiEnter(liIntMei);
                    lPort = (INT32)port;
                    ridFreeSave(liPortsH, lPort);
                    meiExit(liIntMei);
                    portsToFail--;
                    break;
                default:
                    perror("liOpen:BIND");
                    meiEnter(liIntMei);
                    lPort = (INT32)port;
                    ridFree(liPortsH, lPort);
                    meiExit(liIntMei);
                    return RVERROR;
            }
    }
    while ((status < 0) && (portsToFail > 0));

    if (portsToFail < 0)
        status = -1;

    if (status <0)
        msaPrintFormat(liGetMsa(), "liOpen:bind error: s=%d,  cannot allocate port in the specified range (from %d to %d) ", s,  ridFrom(liPortsH), ridTo(liPortsH));
    meiEnter(liIntMei);
    lPort = (INT32)portsave;
    if (portsave)
        ridSetMinFree(liPortsH, lPort);
    meiExit(liIntMei);
    return status;
}


/*****************************************************************
  Desc: Opens a non-blocked socket.
  Input: ipAddr- ip address.
         port- port number.
     protocol- (TCP|UDP)
  Returns: socket number.
  Note: Socket is openned with given ip and port numbers.
  See the code for socket specific options (linger, delay etc.).
  */
int
liOpen (UINT32 ipAddr, UINT16 myPort, liProtocol_t protocol)
{
  int s;
  struct sockaddr_in sin; /* an Internet endpoint addresss */
  UINT32 on = TRUE;
  /* int buflen=4096;*/ /* The so called 'standard' on VxWorks */

  int yes = TRUE;
  char ipBuf[18];
  liNode* nodes;
#ifndef RV_OS_OSE   /* linger is not suppoted in OSE */
  struct linger Linger;

  Linger.l_onoff = TRUE;
  Linger.l_linger = LINGER_TIME;
#endif
  nodes = liNodes;

  /* allocate a socket */
  if ((s = socket (PF_INET, liProtData [protocol].protType,
           liProtData [protocol].protNum))< 0)    {
      perror ("SOCKET");
      return (RVERROR);
  }

  msaPrintFormat(liGetMsa(), "li:OPEN (%s): ip=%s, s=%d, port=%d\n",
         nprn(liProtData[protocol].pName), nprn(liIpToString(ipAddr, ipBuf)), s, myPort);

  /* -- set socket options */
  switch (protocol)
  {
      case LI_TCP:
      {
#ifndef RV_OS_OSE
          setsockopt(s, SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(Linger));
#endif
          setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));
          setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&yes, sizeof(yes));

#ifdef UNDER_CE
          {
              int buflen=2048;
              setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&buflen, sizeof(buflen));
              buflen=2048;
              setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&buflen, sizeof(buflen));
          }
#endif  /* UNDER_CE */
          break;
      }
      case LI_UDP:
      {
#ifdef UNDER_CE
          int buflen=65535;
          if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&buflen, sizeof(buflen)) < 0)
              perror("Error in setsockopt ");
          buflen=65535;
          if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&buflen, sizeof(buflen)) < 0)
              perror("Error in setsockopt ");
#endif  /* UNDER_CE */
          break;
      }

      default:
          break;
  }


  /* make socket NON BLOCKED -- Does not work for Linux 1.3.28 */
  if (ioctl(s,FIONBIO,(ioctlOnPtrTypeCast)&on) < 0) {  /* AL - 10/28/98 */
    perror ("ioctl");
    if (liClose(s) < 0)
        msaPrintFormat(liGetMsa(), "li:CLOSE (%s): s=%d\n", s);
    return (RVERROR);
  }

  setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)0, 0); /* reuse */

  /* bind the socket- if error return -1 */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family      = AF_INET;
  sin.sin_port        = htons(myPort);
  sin.sin_addr.s_addr = ipAddr;

  if ((liPortsH == NULL) || (myPort !=0)) {
    if (bind (s, (SOCKADDRPTR)&sin ,sizeof (sin)) < 0)    {
      msaPrintFormat(liGetMsa(), "liOpen:bind error: s=%d, ip=%s, port=%d.  ",
             s, nprn(liIpToString(ipAddr, ipBuf)), myPort);
      perror("liOpen:BIND");
    if (liClose(s) < 0)
        msaPrintFormat(liGetMsa(), "li:CLOSE (%s): s=%d\n", s);
      return (RVERROR);
    }
  }
  else /* user's ports range exists */
    if (liGetDynPort(s, &sin) < 0)
    {
        msaPrintFormat(liGetMsa(), "liOpen:bind error: s=%d, ip=%s, port=%d.  ",
             s, nprn(liIpToString(ipAddr, ipBuf)), myPort);;
        if (liClose(s) < 0)
            msaPrintFormat(liGetMsa(), "li:CLOSE (%s): s=%d\n", s);
        return (RVERROR);
    }

  nodes[s].valid=FALSE;

  /* return socket descriptor */
  return (s);
}

/*******************************************************************
 Desc: Create a connection to remote socket.
 Input: handle- socket descriptor.
        ipAddr- destination ip addr.
    port- destination ip port.
*/
int
liConnect (int handle,UINT32 ipAddr,UINT16 port)
{
    struct sockaddr_in sin;
    int status;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port    = htons(port);
    sin.sin_addr.s_addr=ipAddr;


    /* connect a socket */

    msaPrintFormat(liGetMsa(), "liConnect: s=%d, ip=0x%x, port=%d.\n",
        handle, ipAddr, port);

    if ( (status = connect (handle,(SOCKADDRPTR) &sin ,sizeof(sin))) < 0 )
    {
#ifdef UNDER_CE
        if ( (ERRNOGET == EINPROGRESS) || (ERRNOGET == WSAEWOULDBLOCK) ) /* connection in progress */
#else
        if (ERRNOGET == EINPROGRESS) /* connection in progress */
#endif
            return (0);
        else
        {
            perror ("CONNECT");
            return (RVERROR);
        }
    }

    msaPrintFormat(liGetMsa(), "liConnect: s=%d, ip=0x%x, port=%d. [%d]\n",
        handle, ipAddr, port, status);

    return (0);
}


/*******************************************************************
  Desc: Accept connection on socket.
  Input: mHandle- socket to accept on. (master socket).
  Returns: socket being accepted. (slave socket).
  */
int
liAccept(int mHandle)
{
  struct sockaddr_in fsin;  /* the request from addr. */
  int socketId;
  liNode* nodes;
  sockaddr_namelen alen = (sockaddr_namelen)sizeof(fsin);

  nodes = liNodes;

  socketId = accept(mHandle,(SOCKADDRPTR) &fsin,&alen);

  if (socketId >= 0)
  {
      int yes = TRUE;
#ifndef RV_OS_OSE
      struct linger Linger;
      Linger.l_onoff = TRUE;
      Linger.l_linger = LINGER_TIME;
      setsockopt(socketId,SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(Linger));
#endif
      setsockopt(socketId, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));

      nodes[socketId].valid = FALSE;
  }

  return (socketId);
}


/*******************************************************************
  Desc: listen to socket.
  Input: queueLen: maximum number of concurrent server request.
  */
int
liListen(int handle,int queueLen)
{
  if ((listen (handle,queueLen)) < 0)    {
    perror ("LISTEN");
    return (RVERROR);
  }

  return (0);
}


/*******************************************************************
  Desc: Close socket.
  */
int
liClose(int handle)
{
  UINT32 on = FALSE;
  int rc;
  UINT16 port;
  INT32  lPort;

  if (handle < 0)
    return 0;

  msaPrintFormat(liGetMsa(), "li:Close: handle=%d.", handle);

  rc = ioctl(handle, FIONBIO, (ioctlOnPtrTypeCast)&on); /* make socket blocked */  /* AL - 10/28/98 */
  liCallOn(handle, (liEvents)0, 0, 0); /* dismiss from event array */
  /* rc = shutdown(handle, 1); */

  if (liPortsH != NULL)
  {
      /* Make sure we know this port is not taken anymore */
      port = liGetSockPort(handle);
      meiEnter(liIntMei);
      lPort = (INT32)port;
      ridFree(liPortsH, lPort);
      meiExit(liIntMei);
  }

  rc = close(handle);
  DeleteWaitSdEntry(handle);
  return rc;
}



int
liShutdown(
       /* Shutdown the socket: don't allow further receives. */
       int handle
       )
{
  int rc;

  if (handle < 0) return 0;
  msaPrintFormat(liGetMsa(), "li:Shutdown: handle=%d.", handle);
  rc = shutdown(handle, 1);
  InsertWaitSdEntry(handle);
  return rc;
}






/*------------------------------------------------------
 *
 * FUNCTION  NAME          : liUdpRecv
 *
 * GENERAL DESCRIPTION     : This function receives data from a UDP
 *                           connection , after it has been established using
 *                           LI_open.
 *                           The function blocks the caller till data arrives.
 *                           If timeout is required the caller should use O.S
 *                           time out services. (in VxWorks the caller can
 *                           use select function).
 *
 *
 *
 *
 * INTERFACE DESCRIPTION   :
 *
 *
 * IN:       handle          int        handle that is returned after
 *                                      LI_open .used as index to connect
 *                                      buffer which holds open parameters
 *                                     (file descriptor,protocol,etc.)
 *           len             int       buffer length
 *
 * IN/OUT:   none
 *
 * OUT:     buff             UINT8 *  pointer to received data.
 *
 * RETURN:   status    int   (-1 = failure)
 *
 *
 * REMARKS: buffer that is supplied should be at least 1500 bytes.
 *
 *
 *------------------------------------------------------*/

int
liUdpRecv (int handle, UINT8 *buff,int len,UINT32*ipAddr,UINT16*port)
{
  static int WouldBlockErrorCounter=0;

  /* receives the next incoming data gram */
  struct sockaddr_in fsin;
  sockaddr_namelen fromlen = (sockaddr_namelen)sizeof(fsin);
  int ret= recvfrom (handle,(char *)buff,len,0,(SOCKADDRPTR)&fsin,&fromlen);
  /*  printf("UDP rcv \n"); */

  if (ret<0 && ERRNOGET == EWOULDBLOCK) WouldBlockErrorCounter++;
  if (WouldBlockErrorCounter > 1000) {
#if 0 /* Nextone */
    fprintf(stderr, "    >>> li:UdpRecv: 1000 EWOULDBLOCK errors.\n");
#endif
    WouldBlockErrorCounter=0;
  }

  if (ret<0 && ERRNOGET != EWOULDBLOCK) {
    fprintf(stderr, "    >>> li:UdpRecv: socket=%d err=%d", handle, ERRNOGET);
    perror("Error:");
  }

  *port  =ntohs(fsin.sin_port);
  *ipAddr=(UINT32)/*ntohl*/(fsin.sin_addr.s_addr);

  msaPrintFormat(liMsaInfo(), "li:UDP RECEIVE: handle=%d  port=%d ip=0x%x, len=%d",
         handle, *port, *ipAddr, ret);

  liCallOnForSeli(handle,liNodes[handle].event);

 /* reestablish all events */

  return (ret);
}


/********************************************************************************
 * GENERAL DESCRIPTION     : This function sends information packet in
 *                           datagram.  The function might block
 *                           the caller temporarily ,if system buffers are
 *                           full.
 * IN:        handle      int
 *            buff        UINT8 *
 *            len         int
 * RETURN:   length    int     number of bytes that have been sent. (-1 = failure)
 */
int
liUdpSend (int handle,UINT8 *buff,int len,UINT32 ipAddr,UINT16 port)
{
  struct sockaddr_in fsin;

  fsin.sin_family = AF_INET;
  fsin.sin_port= htons(port);

  fsin.sin_addr.s_addr=ipAddr;
  msaPrintFormat(liMsaInfo(), "li:UDP SEND: handle=%d  port=%d ip=0x%x, len=%d",
         handle, htons(fsin.sin_port), fsin.sin_addr.s_addr, len);

  liCallOnForSeli(handle,liNodes[handle].event);
 /* reestablish all events */

  /* if (liGetSocketPort(handle, NULL) <=0) return RVERROR; */
  return (sendto (handle,(char *)buff,len,0,
          (SOCKADDRPTR)&fsin,
          sizeof(struct sockaddr)));

}

/*-----------------------------------------------------
 * GENERAL DESCRIPTION     : This function sends information packet in
 *                           streams.  The function might block
 *                           the caller temporarily ,if system buffers are
 *                           full.
 * IN:        handle      int
 *            buff        UINT8*
 *            len         int
 * RETURN:   length      int   number of butes that have been sent (-1 = failure)
 */
int
liTcpSend (int handle,UINT8 *buff,int len)
{
 int ret;
 liEvents oldEvent = liNodes[handle].event;
 int port=0;

 port=liGetSocketPort(handle, NULL);
 msaPrintFormat(liMsaInfo(), "liTcpSend: handle=%d, len=%d, [%d].", handle, len, port);
 if (port <0) return RVERROR; /* not connected */

#ifdef RV_OS_OSE
 ret = inet_send(handle, (char *)buff, len, 0);
#else
 ret = send(handle, (char *)buff, len, 0);
#endif
 if (ret<0 && ERRNOGET == EWOULDBLOCK) ret=0; /*  ??????????????????? */
 liCallOnForSeli(handle, oldEvent);  /* reestablish all events */

 msaPrintFormat(liMsaInfo(), "liTcpSend: handle=%d, len=%d, wrote=%d.", handle, len, ret);
 return ret;
}


/********************************************************************************
 * GENERAL DESCRIPTION     : This function receives data from a  TCP
 *                           connection , after it has been established using
 *                           LI_open and LI_accept or LI_connect.
 *                           The function blocks the caller till data arrives.
 *                           If timeout is required the caller should use O.S
 *                           time out services. (in VxWorks the caller can
 *                           use select function).
 * INTERFACE DESCRIPTION   :
 * IN:       handle          int
 *           len             int     length of received buffer.
 * OUT:     buff             UINT8 *  pointer to received data.
 */
  /* acquire incoming data from a connection , using read function.*/
int
liTcpRecv (int handle, UINT8 *buff,int len)
{
  int ret, bytes;
  liEvents oldEvent = liNodes[handle].event;

  liBytesAvailable(handle, &bytes);
  if (bytes<=0)  {
    ERRNOSET(EWOULDBLOCK);
    return 0;
  }

  ret = recv (handle, (char *)buff, min(bytes,len), 0);
  if ((ret<0) && (ERRNOGET == EWOULDBLOCK)) ret=0;

  liCallOnForSeli(handle, oldEvent);  /* reestablish all events */
  msaPrintFormat(
         liMsaInfo(),
         "liTcpRecv: h=%d, len=%d ==>%d, '%.100s'.",
         handle, len, ret,
         (nprn((char *)buff)));
  if (ret<0) perror("liTcpRecv:");

  return ret;
}



UINT32
liGetSockIP (int socketId)
     /* Desc: Get the IP of the socket (as integer). */
{
  struct sockaddr_in fsin;  /* the request from addr. */
  UINT32             ipAddr;
  sockaddr_namelen   alen = (sockaddr_namelen)sizeof(fsin);

  if(liCheckIsValidFd(socketId) ==RVERROR)
     return 0;
  if (getsockname (socketId,(SOCKADDRPTR)&fsin,&alen) <0) return 0;

  memcpy (&ipAddr,&fsin.sin_addr,sizeof(ipAddr));
  return (ipAddr);
}


UINT16 /* The port number. */
liGetSockPort (int socketId)
     /* Desc: Get port associated with socket */
{
  struct sockaddr_in fsin;
  UINT16             ipPort;
  sockaddr_namelen   alen = (sockaddr_namelen)sizeof(fsin);

  if(liCheckIsValidFd(socketId) == RVERROR)
     return 0;
  if (getsockname (socketId,(SOCKADDRPTR)&fsin,&alen) <0) return 0;
  memcpy (&ipPort,&fsin.sin_port,sizeof(ipPort));

  return (ntohs(ipPort));
}

UINT32
liGetRemoteIP (int socketId)
     /* Desc: Get the IP of the connected peer socket (as integer). */
{
  struct sockaddr_in fsin;  /* the request from addr. */
  UINT32             ipAddr;
  sockaddr_namelen   alen = (sockaddr_namelen)sizeof(fsin);

  if(liCheckIsValidFd(socketId) == RVERROR)
     return 0;

  if (getpeername (socketId,(SOCKADDRPTR)&fsin,&alen) <0) return 0;

  memcpy (&ipAddr,&fsin.sin_addr,sizeof(ipAddr));
  return (ipAddr);
}


UINT16 /* The port number. */
liGetRemotePort (int socketId)
     /* Desc: Get port associated with socket */
{
  struct sockaddr_in fsin;
  UINT16             ipPort;
  sockaddr_namelen   alen = (sockaddr_namelen)sizeof(fsin);

  if(liCheckIsValidFd(socketId) == RVERROR)
     return 0;

  if (getpeername (socketId,(SOCKADDRPTR)&fsin,&alen) <0) return 0;
  memcpy (&ipPort,&fsin.sin_port,sizeof(ipPort));

  return (ntohs(ipPort));
}






int /* TRUE or RVERROR */
liGetSocketPort(
        /* Get port associated with socket */
        IN int socketId,
        OUT UINT16* port)
{
  struct sockaddr_in fsin;
  sockaddr_namelen alen = (sockaddr_namelen)sizeof(fsin);

  if(liCheckIsValidFd(socketId) == RVERROR)
     return RVERROR;

  if (getsockname (socketId,(SOCKADDRPTR)&fsin,&alen) <0)
    return RVERROR;

  if (port) {
    memcpy (port,&fsin.sin_port,sizeof(*port));
    *port = ntohs(*port);
  }
  return TRUE;
}




/*------------------------------------------------------
 * IN:      buff       UINT8 *    buffer with packet data
 *          size       int        size of packet header in UINT32 parts
 */
int
liConvertHeader2l (UINT8 *buff,int startIndex,int size)
{
 int i;

 for (i=startIndex;i< (startIndex+size);i++)
   ((UINT32*)buff)[i]=htonl(((UINT32*)buff)[i]);

 return (0);
}

/*------------------------------------------------------
 * IN:      buff       UINT8 *    buffer with packet data
 *          size       int        size of packet header in UINT32 parts
 */
int
liConvertHeader2h (UINT8 *buff,int startIndex,int size)
{
  int i;

  for (i=startIndex;i< (startIndex+size);i++)
    ((UINT32*)buff)[i]= ntohl (((UINT32*)buff)[i]);

  return (0);
}

/*------------------------------------------------------
 * IN:      ipAddr  char *   IP address in dotted notation
 */
UINT32
liConvertIp(char *ipAddr)
{
 return (inet_addr(ipAddr));
}


/*
  Desc: Make socket non-blocked.
  */
int
liUnblock (int sockId)
{
  int rc = 0;
  UINT32 on = TRUE;

  if ((rc = ioctl(sockId,FIONBIO,(ioctlOnPtrTypeCast)&on)) < 0) /* make socket non-block */  /* AL - 10/28/98 */
    perror ("liUnblock:ioctl");

  return (rc);
}


/*
  Desc: Make socket blocked.
  */
int
liBlock (int sockId)
{
  int rc = 0;
  UINT32 on = FALSE;

  if ((rc = ioctl(sockId,FIONBIO,(ioctlOnPtrTypeCast)&on)) < 0) /* make socket blocked */  /* AL - 10/28/98 */
    perror ("liBlock:ioctl");

  return (rc);
}




/*
  Desc: Get number of bytes available for reading on socket.
  Input: sockId- socket handler.
  Output: bytesAvailable: number of bytes.
  */
int
liBytesAvailable (int sockId,int *bytesAvailable)
{
 int return_value;
 return_value=ioctl(sockId,FIONREAD,(ioctlOnPtrTypeCast)bytesAvailable);
 if(return_value<0)
   *bytesAvailable=return_value;
 return(return_value);
}

/*  Desc: Convert an interger IP to dot notation (string).  */
/*
char *
liConvertToIp(UINT32 ipAddr)
{
  struct in_addr in;
  in.s_addr=ipAddr;
  return (inet_ntoa(in));
}
*/

char * /* buf */
liIpToString(
         /*  Desc: Convert an interger IP to dot notation (string).  */
         UINT32 ipAddr,
         char* buf) /* at least with 18 bytes allocated */
{
 BYTE* ip=(BYTE*)&ipAddr;
 sprintf(buf,"%d.%d.%d.%d", (int)ip[0],(int)ip[1],(int)ip[2],(int)ip[3]);
 return buf;
}


/*________________________________multicast____________________________*/
#ifdef IP_MULTICAST_IF

int liSetMulticastInterface(int socketId,UINT32 ipaddr)
{
  struct in_addr addr;

  addr.s_addr=ipaddr;
  setsockopt(socketId, IPPROTO_IP, IP_MULTICAST_IF, (char*)&addr, sizeof(addr));
  return 0;
}

int liSetMulticastTTL(int socketId,int ttl)
{
   char ttl_uc;
   ttl_uc = (char)ttl;

  if (setsockopt(socketId, IPPROTO_IP, IP_MULTICAST_TTL, &ttl_uc, sizeof(ttl_uc)) < 0)
  {
      perror("setsockopt: Error setting TTL");
      return -1;
  }
  else
      return 0;
}

int liJoinMulticastGroup(int socketId,UINT32 mcastip,UINT32 ipaddr)
{
  struct ip_mreq mreq;
  if(mcastip);

#ifdef __PSOSPNA__         /* AL - 09/17/98 */
  mreq.imr_mcastaddr.s_addr=mcastip;
#else
  mreq.imr_multiaddr.s_addr=mcastip;
#endif
  mreq.imr_interface.s_addr=ipaddr;
  setsockopt(socketId, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
  return 0;
}

int liLeaveMulticastGroup(int socketId,UINT32 mcastip,UINT32 ipaddr)
{
  struct ip_mreq mreq;


#ifdef __PSOSPNA__         /* AL - 09/17/98 */
  mreq.imr_mcastaddr.s_addr=mcastip;
#else
  mreq.imr_multiaddr.s_addr=mcastip;
#endif
  mreq.imr_interface.s_addr=ipaddr;
  setsockopt(socketId, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
  return 0;
}

#else
int liSetMulticastInterface(int socketId,UINT32 ipaddr)
{  return RVERROR;}

int liSetMulticastTTL(int socketId,int ttl)
{  return RVERROR;}

int liJoinMulticastGroup(int socketId,UINT32 mcastip,UINT32 ipaddr)
{  return RVERROR;}

int liLeaveMulticastGroup(int socketId,UINT32 mcastip,UINT32 ipaddr)
{  return RVERROR;}

#endif



/* nextone - vhostname */
static char *vhostname = (char *)NULL;
UINT32
liSetVHostName(char *vname)
{
	char *tmpname;

	tmpname = vhostname;

	vhostname = strdup(vname);

	if (tmpname) free (tmpname);

	return 0;
}

char *
liGetVHostName(char *buff, int bufflen)
{
	if (vhostname)
	{
		strncpy(buff, vhostname, bufflen);
		buff[bufflen-1] = '\0';
	}

	return vhostname;
}

UINT32** liGetHostAddrs(void)
{
#if defined (__VXWORKS__) || defined (__PSOSPNA__)
    {
        UINT32** lpGetIps(void);
        return lpGetIps();
    }
#elif defined RV_OS_OSE
    char buff[255];
    struct hostent *host;

    meiEnter(liIntMei);
    gethostname(buff,sizeof(buff));
    host = (struct hostent *)gethostbyname(buff);
    meiExit(liIntMei);
    if (host)
      return (UINT32**)(host->h_addr_list);
    else
      return (UINT32**) NULL;
#else
    {

        char buff[255];
        static UINT32* pmyIPs[11]={myIPs+0,myIPs+1,myIPs+2,myIPs+3,myIPs+4,
            myIPs+5,myIPs+6,myIPs+7,myIPs+8,myIPs+9,NULL};

#ifndef HOST_HAS_DYNAMIC_IP
        if (!myIPs[0])
#endif
        {
            UINT32** host=NULL;
            int i;
            struct hostent *entry;

            meiEnter(liIntMei);
			if (liGetVHostName(buff, sizeof(buff)) == NULL)
			{
				gethostname(buff,sizeof(buff));
			}
            entry = gethostbyname(buff);
            if (entry)
                host=(UINT32**)(entry->h_addr_list);
            meiExit(liIntMei);

            if (host)
            {
                for (i=0;i<10 && host[i];i++)
                {
                    myIPs[i]=*host[i];
                }
                pmyIPs[i]=0;
            }
        }

        if(myIPs[0])
            return pmyIPs;
        return NULL;
    }
#endif  /* __VXWORKS__ */
}

#ifdef RV_OS_OSE
/* since there is no host name in OSE, get it from the environent. */
int gethostname(char *buf, size_t size)
{
    char *result;

    result = get_env(current_process(), "HOSTNAME");
    if(result == NULL) {
        result = get_env(get_bid(current_process()), "HOSTNAME"); /* try block environment */
        if(result == NULL) {
            *buf = '\0';
            return -1;
        }
    }
    strncpy(buf, result, size);
    free_buf((union SIGNAL **)&result);
    return 0;
}
#endif

char* liGetHostName(void)
{
  static char buff[255];
  meiEnter(liIntMei);
  gethostname(buff,sizeof(buff));
  meiExit(liIntMei);
  return buff;
}


int
liSetSocketBuffers(
           /* Set the size of send and receive buffers for socket. */
           int socket,
           int sendSize, /* Negative size does not effect current size */
           int recvSize  /* Negative size does not effect current size */
           )
{
  int buflen=0;
  if (socket<0) return RVERROR;

  if (recvSize>=0) {
    buflen=recvSize;
    setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)&buflen, sizeof(buflen));
  }

  if (sendSize>=0) {
    buflen=sendSize;
    setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char*)&buflen, sizeof(buflen));
  }

  return TRUE;
}


/************************************************************************
 * liThreadAttach
 * purpose: Indicate that a thread can catch network events.
 * input  : threadId        - Thread using a message loop
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int liThreadAttach(IN RvH323ThreadHandle    threadId)
{
    THREAD_LiLocalStorage* liTls;

    liTls = (THREAD_LiLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrLi, sizeof(THREAD_LiLocalStorage));
    if (liTls == NULL)
        return RVERROR;
    liTls->messageLoops++;
    return seliInitializeThread(threadId);
}


/************************************************************************
 * liThreadDetach
 * purpose: Indicate that a thread cannot catch network events anymore.
 *          Should be called after calling liThreadAttach()
 * input  : threadId        - Thread using a message loop
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int liThreadDetach(IN RvH323ThreadHandle    threadId)
{
    THREAD_LiLocalStorage* liTls;

    liTls = (THREAD_LiLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrLi, sizeof(THREAD_LiLocalStorage));
    if ((liTls == NULL) || (liTls->messageLoops == 0))
        return RVERROR;
    liTls->messageLoops--;
    return seliEndThread(threadId);
}


int
dmr_liUdpRecv (int handle, UINT8 *buff,int len,UINT32*ipAddr,UINT16*port)
{
  static int WouldBlockErrorCounter=0;

  /* receives the next incoming data gram */
  struct sockaddr_in fsin;
  sockaddr_namelen fromlen = (sockaddr_namelen)sizeof(fsin);
  int ret= recvfrom (handle,(char *)buff,len,0,(SOCKADDRPTR)&fsin,&fromlen);
  if (ret<0 && ERRNOGET == EWOULDBLOCK) WouldBlockErrorCounter++;
  if (WouldBlockErrorCounter > 1000) {
#if 0 /* Nextone */
    fprintf(stderr, "    >>> li:UdpRecv: 1000 EWOULDBLOCK errors.\n");
#endif
    WouldBlockErrorCounter=0;
  }

  if (ret<0 && ERRNOGET != EWOULDBLOCK) {
    fprintf(stderr, "    >>> li:UdpRecv: socket=%d err=%d\n", handle, ERRNOGET);
    perror("Error:");
  }

  *port  =ntohs(fsin.sin_port);
  *ipAddr=(UINT32)/*ntohl*/(fsin.sin_addr.s_addr);

  /*liCallOnForSeli(handle,liNodes[handle].event); */

 /* reestablish all events */

  return (ret);
}

int
dmr_liUdpSend (int handle,UINT8 *buff,int len,UINT32 ipAddr,UINT16 port)
{
  struct sockaddr_in fsin;

  fsin.sin_family = AF_INET;
  fsin.sin_port= htons(port);

  fsin.sin_addr.s_addr=ipAddr;
  msaPrintFormat(liMsaInfo(), "li:UDP SEND: handle=%d  port=%d ip=0x%x, len=%d",
         handle, htons(fsin.sin_port), fsin.sin_addr.s_addr, len);

  /*liCallOnForSeli(handle,liNodes[handle].event);*/
 /* reestablish all events */

  /* if (liGetSocketPort(handle, NULL) <=0) return RVERROR; */
  return (sendto (handle,(char *)buff,len,0,
          (SOCKADDRPTR)&fsin,
          sizeof(struct sockaddr)));

}

#ifdef __cplusplus
}
#endif



