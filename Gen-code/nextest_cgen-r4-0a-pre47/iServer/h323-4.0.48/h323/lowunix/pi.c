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
  pi.h

  Pipe Interface.
  Provides interface to the system pipe functions.

  Revised By: Ron S.  14 Jan 1996

  Supporting platforms:
  - i960

  Parameter Description:
  - handle: pipe handler number (descriptor).
  - Returns: RVERROR if fault occurred.


  Supplumments:
  PI module uses a list to hold incoming events (from the application).
  PI registers callon requests in SELI.

  Note: i960:
  The fdRead is used for both read and write operations.




  Can't work with pipes ! use other ioctl and another structure instead of handle
  Keep msgSize and maxMsgs.

  */
#ifndef __PSOSPNA__

#define VCPLIH

#ifdef __VXWORKS__
#include <sys/types.h>
#include <stdio.h>
#include <vxWorks.h>
#include <taskLib.h>
#include <msgQLib.h>
#include <pipeDrv.h>
#include <ioLib.h>
#include <iosLib.h>
#include <string.h>

#elif __PSOSPNA__

#include <pna.h>
#define __Iunistd
	
#else

/*** UNIX Header files ***/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#if (defined(IS_PLATFORM_SOLARIS) || defined(IS_PLATFORM_SOLARISPC))
#include <sys/filio.h>
#endif


#ifdef UNIXWARE
#include <sys/xti.h>
#include <sys/sockmod.h>
#include <sys/osocket.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#endif


#include <unistd.h>

#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/stat.h> /* mknod */

#endif  /* __VXWORKS__ */

#include <pi.h>
#include <seli.h>

#include <rlist.h>
#include <ra.h>
#include <ms.h>



/* PI list of nodes. */
typedef struct
{
    int                 valid;
    int                 fdRead;   /* the handle */
    piEvents            event;    /* on what events the application want to be notified. */
    piCallback          callback; /* callback function */
    void*               context;  /* of application */
    RvH323ThreadHandle  threadId;
} piNode;


/*static HRA piTaskA = NULL;*/  /* array of tasks */

static int msa=0;

piNode* piNodes = NULL;

/*
*/


int seliGetMaxDescs(void);


void piHandleCallback(int handle, seliEvents sEvent, BOOL error);


/*____________________________module functions____________________________________*/

/*
  Desc: Init PI module.
  Init list of events and SELI.
  */
static INT32 piInitCalls = 0;

int
piInit(void)
{


  int piFdSetSizeDyn;

  piInitCalls++;
  if (piInitCalls == 1) {
    piFdSetSizeDyn = seliGetMaxDescs();

    /* liMuxSemH = semiConstruct(); */
    msa = msaRegister(0, "PI", "PIPE Interface");

    piNodes = (piNode *)  calloc(sizeof(piNode),piFdSetSizeDyn);
    if (!piNodes)
     return FALSE;
    memset(piNodes,0,sizeof(piNode)*piFdSetSizeDyn);

  }


  return TRUE;
}




/*
  Desc: End module operation.
  */
int
piEnd(void)
{


  piInitCalls--;
  if (!piInitCalls)
    {
      free(piNodes);
      piNodes=NULL;
      seliEnd();
      msaUnregister(msa);

    }
  return TRUE;
}



void
piCloseAll(void)
{

  int cur;
  int piFdSetSizeDyn;

  if (!piNodes) return;
  piFdSetSizeDyn = seliGetMaxDescs();
  for (cur=0;cur<piFdSetSizeDyn;cur++)
  {
      if (piNodes[cur].valid)
    {
    piClose(cur);

    }
  }
}

/*____________________________registration_______________________________________*/
/* Desc: Register new event for handle. The handle is registered for
   select()ing when using the seliSelect().
   Note: To unregister the socket, call the function with pEvent = 0
         Example: piCallOn(socketId, 0, 0);
   Logic: if handle is already registered, than last registration is erased.
   Note: non positive handle value is considered invalid.
   */
int
piCallOn(int handle, piEvents pEvent, piCallback callback, void* context)
{

  seliEvents sEvent =(seliEvents)0;

  if (!piNodes) return 0;

  if (handle<0) {
    msaPrintFormat(msa, "piCallOn: Illegal Descriptor [%d].", handle);
    return RVERROR;
  }

  piNodes[handle].fdRead = handle;
  piNodes[handle].event = pEvent;
  piNodes[handle].callback = callback;
  piNodes[handle].context = context;

  msaPrintFormat(msa, "piCallOn: fd=%d, events=0x%x, callback=0x%x, context=%p.",
         handle, pEvent, callback, context);


  if (pEvent & piEvRead || pEvent & piEvClose) sEvent  = (seliEvents)(sEvent | seliEvRead);
  if (pEvent & piEvWrite)                      sEvent  = (seliEvents)(sEvent | seliEvWrite);

  seliCallOnThread(handle, sEvent, piHandleCallback, piNodes[handle].threadId);

  if (!pEvent || !callback) return TRUE; /* unregistration */

  /* register socket */

  return TRUE;
}


/*
  Desc: handle events from select.
  Call application callback function with proper parameters.
  */
void
piHandleCallback(int handle, seliEvents sEvent, BOOL error)
{
  piNode *elem;
  piEvents pEvent=(piEvents)0;
  int bytes;

  if (!piNodes[handle].valid)
  {
    msaPrintFormat(msa, "piHandleCallback: Received event [0x%x] on unregistered pipe [%d].\n",
           sEvent, handle);
    return;
  }

  elem = &piNodes[handle];

  piBytesAvailable(handle, &bytes);
  msaPrintFormat(msa, "piHandleCallback: Received seli event %d, #bytes=%d on pipe %d.\n", sEvent, bytes,handle);

  if ((elem->event&piEvRead)  && (sEvent&seliEvRead) && bytes>0)  pEvent=piEvRead;
  if ((elem->event&piEvWrite) && (sEvent&seliEvWrite))            pEvent=piEvWrite;
  if ((elem->event&piEvClose) && (sEvent&seliEvRead) && bytes<=0) pEvent=piEvClose;
    msaPrintFormat(msa, "piCallback: Received event [0x%x] on unregistered pipe [%d].\n",
           sEvent, handle);

  if (pEvent)  elem->callback(handle, pEvent, error, elem->context);
}



/*_____________________________pipe functions__________________________________*/

/*****************************************************************
  Desc: Opens a non-blocked pipe.
  Input: ipAddr- ip address.
         port- port number.
     protocol- (TCP|UDP)
  Returns: handle for pipe.
  */


int
piOpen (char *name, int msgSize, int maxMsgs)
{
  int fdRead;

#ifdef __VXWORKS__
 {
  int status;
  char * pNameTail;
  iosDevFind(name, &pNameTail);
  if ((name + strlen(name))!= pNameTail)

    if ((status = pipeDevCreate (name, maxMsgs, msgSize)) ==RVERROR) {
      perror ("piOpen:create:");
      return RVERROR;
    }
 }
#else
  /* create named pipe with read/write permissions for all users. */
  mknod(name, 010777, 0);
#endif  /* __VXWORKS__ */
  if(msgSize || maxMsgs);

  if ((fdRead = open(name, O_RDWR, 644)) < 0)  {
    perror ("piOpen:open:");
    return RVERROR;
  }

  /* make descriptor NON BLOCKED -- Does not work for Linux 1.3.28 */
  /*
      Can't work with pipes ! use other ioctl and another structure instead of handle
      Keep msgSize and maxMsgs.


      if (ioctl(fdRead, FIONBIO, &on) < 0) {
    perror ("ioctl");
    return (RVERROR);
  }*/


  /* return pipe descriptor */
  piNodes[fdRead].valid = TRUE;
  piNodes[fdRead].threadId = RvH323ThreadGetHandle();

  return (fdRead);
}



/*******************************************************************
  Desc: Close pipe.
  */
int
piClose (int handle)
{
  int ret;

  if (handle <= 0)  return RVERROR;

  piCallOn(handle, (piEvents)0, 0, 0); /* dismiss from event array */
  ret = close(handle);

  return ret;
}


/*
  Desc: Get number of bytes available for reading on pipe.
  Input: handle- pipe handler.
  Output: bytesAvailable: number of bytes.
  */
int
piBytesAvailable (int handle,int *bytesAvailable)
{
  return (ioctl(handle, FIONREAD, (int)bytesAvailable));
}


/*
  Desc: read from a pipe.
  Return: number of bytes read or error.
  */
int
piRead(int handle, UINT8 *buf, int len)
{
  int ret;

  if (! (ret = (int)read(handle, (void *)buf, len)))
    perror("piRead");
  return ret;
}



/*
  Desc: write to a pipe.
  Return: number of bytes wrote or error.
  */
int
piWrite(int handle, UINT8 *buf, int len)
{
  int ret;

  if (! (ret = (int)write(handle, (void *)buf, len)))
    perror("piWrite");
  return ret;
}


int
piUnblock(
      /*  Desc: Make socket non-blocked.  */
      int handle
      )
{
  int rc = 0;
  UINT32 on = TRUE;

  if ((rc = ioctl(handle, FIONBIO, (int)&on)) < 0) /* make handle non-block */
    perror ("piUnblock:ioctl");

  return (rc);
}


int
piBlock(
    /* Desc: Make socket blocked.  */
    int handle
    )
{
  int rc = 0;
  UINT32 on = FALSE;

  if ((rc = ioctl(handle, FIONBIO, (int)&on)) < 0) /* make handle blocked */
    perror ("piBlock:ioctl");

  return (rc);
}

#else

int RV_dummyPi = 0;

#endif

#ifdef __cplusplus
}
#endif



