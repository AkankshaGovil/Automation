#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


/****************************************************************************

  rtcp.c  --  RTCP implementation.

  Abstract:       The main RTCP module file.

  Platforms:      All.


****************************************************************************/

#include <stdio.h>
#include <string.h>

#include <rvcommon.h>
#include <rlist.h>
#include "buffer.h"
#include "bitfield.h"
#include <mti.h>
#include <ti.h>
#include <li.h>
#include <time.h>
#include <mei.h>

#include "rtcp.h"

#define MAXSDES                   255
#define MAXRTPSESSIONS            10
#define MAXRTPSESSIONMEMBERS      50
#define MAXRTCPPACKET             1470

#define MAXIPS                    20
#define MAX_DROPOUT               3000
#define MAX_MISORDER              100
#define MIN_SEQUENTIAL            2
#define RTP_SEQ_MOD               0x10000

#define ALIGNMENT                 0x10

/* RTCP header bit locations - see the standard */
#define HEADER_V                  30      /* version                       */
#define HEADER_P                  29      /* padding                       */
#define HEADER_RC                 24      /* reception report count        */
#define HEADER_PT                 16      /* packet type                   */
#define HEADER_len                0       /* packet length in 32-bit words */

/* RTCP header bit field lengths - see the standard */
#define HDR_LEN_V                 2       /* version                       */
#define HDR_LEN_P                 1       /* padding                       */
#define HDR_LEN_RC                5       /* reception report count        */
#define HDR_LEN_PT                8       /* packet type                   */
#define HDR_LEN_len               16      /* packet length in 32-bit words */


/* used to overcome byte-allignment issues */
#define SIZEOF_RTCPHEADER         (sizeof(UINT32) * 2)
#define SIZEOF_SR                 (sizeof(UINT32) * 5)
#define SIZEOF_RR                 (sizeof(UINT32) * 6)

#define SIZEOF_SDES(sdes)         (((sdes).length + 6) & 0xfc)

/* initial bit field value for RTCP headers: V=2,P=0,RC=0,PT=0,len=0 */
#define RTCP_HEADER_INIT          0x80000000

typedef enum {
   RTCP_SR   = 200,               /* sender report            */
   RTCP_RR   = 201,               /* receiver report          */
   RTCP_SDES = 202,               /* source description items */
   RTCP_BYE  = 203,               /* end of participation     */
   RTCP_APP  = 204                /* application specific     */
} rtcpType;

typedef enum {
   RTCP_SDES_END   = 0,
   RTCP_SDES_CNAME = 1,
   RTCP_SDES_NAME  = 2,
   RTCP_SDES_EMAIL = 3,
   RTCP_SDES_PHONE = 4,
   RTCP_SDES_LOC   = 5,
   RTCP_SDES_TOOL  = 6,
   RTCP_SDES_NOTE  = 7,
   RTCP_SDES_PRIV  = 8
} rtcpSDesType;

typedef struct
{
   UINT32  msdw;
   UINT32  lsdw;
} RV_UINT64;

typedef struct
{
   RV_UINT64  tNNTP;
   UINT32  tRTP;

   UINT32  nPackets;
   UINT32  nBytes;
} rtcpSR;

typedef struct
{
   UINT32  ssrc;
   UINT32  bfLost;      /* 8Bit fraction lost and 24 bit cumulative lost */
   UINT32  nExtMaxSeq;
   UINT32  nJitter;
   UINT32  tLSR;
   UINT32  tDLSR;
} rtcpRR;

typedef struct {
   UINT16  max_seq;               /* highest seq. number seen */
   UINT32  cycles;                /* shifted count of seq. number cycles */
   UINT32  base_seq;              /* base seq number */
   UINT32  bad_seq;               /* last 'bad' seq number + 1 */
   UINT32  probation;             /* sequ. packets till source is valid */
   UINT32  received;              /* packets received */
   UINT32  expected_prior;        /* packet expected at last interval */
   UINT32  received_prior;        /* packet received at last interval */
   UINT32  transit;               /* relative trans time for prev pkt */
   UINT32  jitter;                /* estimated jitter */
   /* ... */
} rtpSource;

typedef struct
{
   UINT8  type;
   UINT8  length;
   char   value[MAXSDES + 1];     /* leave a place for an asciiz */
} rtcpSDES;

typedef struct
{
   int        invalid;
   BOOL       active;
   rtpSource  src;

   UINT32     ssrc;
   UINT32     tLSRmyTime;
   rtcpSR     eSR;
   rtcpRR     eToRR;
   rtcpRR     eFromRR;
   rtcpSDES   eCName;
} rtcpInfo;

typedef struct
{
   UINT32  bits;
   UINT32  ssrc;
} rtcpHeader;


typedef struct
{
   BOOL      active;
   int       collision;
   UINT32    ssrc;
   UINT32    timestamp;
   rtcpSR    eSR;
   rtcpSDES  eCName;
} rtcpMyInfo;

typedef struct
{
   BOOL        isAllocated;
   int         socket;
   rtcpMyInfo  myInfo;
   UINT32      ip;
   UINT16      port;
   HTI         tElem;
   HMEI        hMutex;            /* Mutex handle*/
   rtcpInfo    *participantsArray;
   int         sessionMembers;
   int         maxSessionMembers;
/*h.e 30.04.01*/
   LPRTCPEVENTHANDLER rtcpRecvCallback;
   void*                haRtcp;
/*==*/
} rtcpSession;

static UINT32 rtcpInitialized = 0;

static UINT32 localIP;
static UINT32 myIPs[MAXIPS];
#ifndef RV_USE_R0
static HSTIMER hst;
#endif

#define reduceNNTP(a) (((a).msdw<<16)+((a).lsdw>>16))

#define W32Len(l)  ((l + 3) / 4)  /* length in 32-bit words */


/* local functions */
static RV_UINT64 getNNTPTime(void);
static void   RVCALLCONV rtcpTimerCallback(void* key);
static void   rtcpLiCallback   (int socket, liEvents event, int error, void *context);
static UINT32 ipBindTo=LI_ADDR_ANY;
static BOOL   isMyIP(UINT32 ip);
static void   setSDES(rtcpSDesType type, rtcpSDES* sdes, BYTE *data,
                      int length);
static void   init_seq  (rtpSource *s, UINT16 seq);
static int    update_seq(rtpSource *s, UINT16 seq, UINT32 ts, UINT32 arrival);

static UINT32 getLost    (rtpSource *s);
static UINT32 getJitter  (rtpSource *s);
static UINT32 getSequence(rtpSource *s);
/*h.e 30.05.01*/
static UINT32 getSSRCfrom(BYTE *);
/*===*/

static rtcpHeader makeHeader(UINT32 ssrc, UINT8 count, rtcpType type,
                             UINT16 dataLen);

static rtcpInfo * findSSrc(rtcpSession *,UINT32);

static rtcpInfo *insertNewSSRC(rtcpSession *s, UINT32 ssrc);

int align(int addr, int alignVal)
{
  return  ((addr / alignVal) * alignVal + alignVal);
}


RVAPI
INT32 RVCALLCONV rtcpGetEnumFirst(
                IN  HRTCPSESSION  hRTCP,
                IN  INT32 *       ssrc);

RVAPI
INT32 RVCALLCONV rtcpGetEnumNext(
                IN  HRTCPSESSION  hRTCP,
                IN  INT32         prev,
                IN  INT32 *       ssrc);

RVAPI
INT32 RVCALLCONV rtcpProcessCompoundRTCPPacket(
        IN      HRTCPSESSION  hRTCP,
        IN OUT  BUFFER *      buf,
        IN      RV_UINT64        myTime);


RVAPI
INT32 RVCALLCONV rtcpProcessRTCPPacket(
        IN  rtcpSession *  s,
        IN  BYTE *         data,
        IN  INT32          dataLen,
        IN  rtcpType       type,
        IN  INT32          reportCount,
        IN  RV_UINT64         myTime);




                      /* == Basic RTCP Functions == */


/*=========================================================================**
**  == rtcpInit() ==                                                       **
**                                                                         **
**  Initializes the RTCP module.                                           **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpInit(void)
{
    int i,err;
    UINT32 **ips;

#ifdef RV_USE_R0
    HSTIMER hst;
#else
    if (hst) return ERR_RTCP_GENERALERROR;
#endif

    if ((err = liInit()) < 0)
        return ERR_RTCP_GENERALERROR;

    if (rtcpInitialized == 0)
    {
        if ((hst = mtimerInit(1024,(HAPPTIMER)myIPs)) == NULL)
        {
            liEnd();
            return ERR_RTCP_GENERALERROR;
        }
        ips = liGetHostAddrs();

        if (!ips)
        {
            liEnd();
            mtimerEnd(hst);
            hst = NULL;

            return ERR_RTCP_GENERALERROR;
        }

        localIP = liConvertIp((char*)"127.0.0.1");

        for (i=0; ips[i]; i++)
            myIPs[i] = *(ips[i]);
    }

    liThreadAttach(NULL);

    rtcpInitialized++;

    return 0;
}


/*=========================================================================**
**  == rtcpInitEx() ==                                                     **
**                                                                         **
**  Initializes the RTCP module.                                           **
**  Parameter: ip - ip address to 'bind' RTCP session                      **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpInitEx(UINT32 ip)
{
    INT32 rc;

    if ((rc=rtcpInit()) != ERR_RTCP_GENERALERROR)
      ipBindTo=ip;

    return rc;
}

/*=========================================================================**
**  == rtcpEnd() ==                                                        **
**                                                                         **
**  Shuts down the RTCP module.                                            **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpEnd(void)
{
    int err = 0;

    liThreadDetach(NULL);
    err = liEnd();
#ifdef RV_USE_R0
    mtimerEndByHandle((HAPPTIMER)myIPs);
#else
    mtimerEnd(hst);
    hst = NULL;
#endif

    rtcpInitialized--;

    return err;
}


/*h.e 30.04.01*/
RVAPI
INT32 RVCALLCONV rtcpSetRTCPRecvEventHandler(
    IN HRTCPSESSION         hRTCP,
    IN LPRTCPEVENTHANDLER   rtcpCallback,
    IN void *               context)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
  s->rtcpRecvCallback=rtcpCallback;
  s->haRtcp=context;                     /*context is Event to inform Ring3 about RTCP arrival*/
  return 0;
}
/*===*/


/*=========================================================================**
**  == rtcpGetAllocationSize()                                             **
**                                                                         **
**  Calculates an allocation size for RTCP session             **
**                                                                         **
**  PARAMETERS:                                                            **
**      sessionMembers  Maximum number of participants in the session      **
**                                                                         **
**                                                                         **
**  RETURNS:                                                               **
**      If no error occurs, the function returns an allocation size for    **
**      RTCP session.  Otherwise it returns NULL.                          **
**                                                                         **
**=========================================================================*/

RVAPI
int RVCALLCONV rtcpGetAllocationSize(
    IN  int sessionMembers)
{
    return sizeof(rtcpSession) + ALIGNMENT
        +  sizeof(rtcpInfo) * sessionMembers + ALIGNMENT
        +  mutexGetAllocationSize();
}


/************************************************************************************
 * rtcpSetLocalAddress
 * description: Set the local address to use for calls to rtcpOpenXXX functions.
 *              This parameter overrides the value given in rtcpInitEx() for all
 *              subsequent calls.
 * input: ip    - Local IP address to use
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
int RVCALLCONV rtcpSetLocalAddress(IN UINT32 ip)
{
    ipBindTo = ip;
    return 0;
}



/*=========================================================================**
**  == rtcpOpenFrom() ==                                                   **
**                                                                         **
**  Opens a new RTCP session in provided buffer.                           **
**                                                                         **
**  PARAMETERS:                                                            **
**      ssrc     The synchronization source value for the RTCP session.    **
**                                                                         **
**      port     The UDP port number to be used for the RTCP session.      **
**                                                                         **
**      cname    A unique name representing the source of the RTP data.    **
**               Must not be NULL.                                         **
**      maxSessionMembers   Maximum number of participants in the session  **
**                                                                         **
**  buffer   pointer to at least rtcpGetAllocationSize byte of memory      **
**                                                                         **
**  bufferSize size of the buffer                                          **
**                                                                         **
**  RETURNS:                                                               **
**      If no error occurs, the function returns a handle for the new      **
**      RTCP session.  Otherwise it returns NULL.                          **
**                                                                         **
**  COMMENTS:                                                              **
**              -----------------------------------------------            **
**     buffer  | rtcpSession |X| participantsArray |X| "mutex" |           **
**              -----------------------------------------------            **
**                            X - alignment area                           **
**=========================================================================*/

RVAPI
HRTCPSESSION RVCALLCONV rtcpOpenFrom(
        IN  UINT32  ssrc,
        IN  UINT16  port,
        IN  char *  cname,
        IN  int     maxSessionMembers,
        IN  void *  buffer,
        IN  int     bufferSize)
{
    rtcpSession* s=(rtcpSession*)buffer;
    int allocSize=rtcpGetAllocationSize(maxSessionMembers);
    int participantsArrayOfs;
    int mutexSectionOfs;

    if (!cname  ||  strlen(cname) > MAXSDES || allocSize > bufferSize)
    {
        return NULL;
    }

    memset(buffer, 0, allocSize);

    s->sessionMembers = 0;
    s->maxSessionMembers = maxSessionMembers;
    s->rtcpRecvCallback = NULL;
    s->isAllocated=FALSE;

    participantsArrayOfs = align (sizeof(rtcpSession), ALIGNMENT);
    s->participantsArray = (rtcpInfo *) ((char *)s + participantsArrayOfs);
    s->participantsArray[0].ssrc = s->myInfo.ssrc = ssrc;

    s->socket = liOpen(ipBindTo,port,LI_UDP);
    if (s->socket==RVERROR)
    {
        return NULL;
    }

    s->tElem = (HTI)RVERROR;

    /* initialize "mutex" in supplied buffer*/
    mutexSectionOfs = participantsArrayOfs + align(sizeof(rtcpInfo)*maxSessionMembers, ALIGNMENT);
    s->hMutex = mutexInitFrom( (void *) ((char *)s + mutexSectionOfs) );
#ifndef UNDER_CE
    if (!s->hMutex)
    {
        return NULL;
    }
#endif

    liCallOn(s->socket,liEvRead,rtcpLiCallback,s);
    setSDES(RTCP_SDES_CNAME, &(s->myInfo.eCName), (BYTE*)cname, (int)strlen(cname));

    return (HRTCPSESSION)s;
}

/*=========================================================================**
**  == rtcpOpen() ==                                                       **
**                                                                         **
**  Opens a new RTCP session.                                              **
**                                                                         **
**  PARAMETERS:                                                            **
**      ssrc     The synchronization source value for the RTCP session.    **
**                                                                         **
**      port     The UDP port number to be used for the RTCP session.      **
**                                                                         **
**      cname    A unique name representing the source of the RTP data.    **
**               Must not be NULL.                                         **
**                                                                         **
**  RETURNS:                                                               **
**      If no error occurs, the function returns a handle for the new      **
**      RTCP session.  Otherwise it returns NULL.                          **
**                                                                         **
**=========================================================================*/

RVAPI
HRTCPSESSION RVCALLCONV rtcpOpen(
        IN  UINT32  ssrc,
        IN  UINT16  port,
        IN  char *  cname)
{
    rtcpSession* s;
    int allocSize=rtcpGetAllocationSize(MAXRTPSESSIONMEMBERS);
    s = (rtcpSession*)calloc(allocSize,1);
    if (s==NULL)
        return NULL;

    if((rtcpSession*)rtcpOpenFrom(ssrc, port, cname, MAXRTPSESSIONMEMBERS, (void*)s, allocSize)==NULL)
    {
        free(s);
        return NULL;
    }
    s->isAllocated=TRUE;
    return (HRTCPSESSION)s;
}


/*=========================================================================**
**  == rtcpClose() ==                                                      **
**                                                                         **
**  Closes an RTCP session.                                                **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpClose(
                IN  HRTCPSESSION  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    if (s->tElem!=(HTI)RVERROR)
      {
#ifdef RV_USE_R0
        mtimerResetByHandle((HAPPTIMER)myIPs, s->tElem);
#else
        mtimerReset(hst,s->tElem);
#endif
        s->tElem=(HTI)RVERROR;
      }
    liClose(s->socket);

    mutexEnd(s->hMutex);

    /* free memory allocated for rtcpSession */
    if (s->isAllocated)
    free(s);

    return 0;
}


/*=========================================================================**
**  == rtcpSetRemoteAddress() ==                                           **
**                                                                         **
**  Defines the address of the remote peer or of the multicast groop.      **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP    The handle of the RTCP session.                           **
**                                                                         **
**      ip       The IP address to which the RTCP packets will be sent.    **
**                                                                         **
**      port     The UDP port to which the RTCP packets should be sent.    **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
void RVCALLCONV rtcpSetRemoteAddress(
                IN  HRTCPSESSION  hRTCP,     /* RTCP Session Opaque Handle */
                IN  UINT32        ip,        /* target ip address */
                IN  UINT16        port)      /* target UDP port */
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    s->ip   = ip;
    s->port = port;
    if (ip&&port)
    {
        if (s->tElem==(HTI)RVERROR)
#ifdef RV_USE_R0
            s->tElem =mtimerSetByHandle((HAPPTIMER)myIPs, rtcpTimerCallback, s, 5000/* should be calculated*/);
#else
            s->tElem =mtimerSet(hst, rtcpTimerCallback, s, 5000/* should be calculated*/);
#endif
    }
    else
    {
        rtcpStop(hRTCP);
    }
}
/*=========================================================================**
**  == rtcpStop() ==                                                       **
**                                                                         **
**  Stop RTCP transmisions .                                               **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP    The handle of the RTCP session.                           **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpStop(
                IN  HRTCPSESSION  hRTCP)     /* RTCP Session Opaque Handle */
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    s->ip   = 0;
    s->port = 0;


    /* Clear the list*/
    memset(s->participantsArray,0,(sizeof(rtcpInfo))*(s->sessionMembers));
    s->myInfo.collision = 0;
    s->myInfo.active = 0;
    s->myInfo.timestamp = 0;
    memset(&(s->myInfo.eSR),0,sizeof(s->myInfo.eSR));
    if (s->tElem!=(HTI)RVERROR)
    {
#ifdef RV_USE_R0
        mtimerResetByHandle((HAPPTIMER)myIPs,s->tElem);
#else
        mtimerReset(hst,s->tElem);
#endif
        s->tElem=(HTI)RVERROR;
    }

    return 0;
}

/*=========================================================================**
**  == rtcpRTPPacketRecv() ==                                              **
**                                                                         **
**  Informs the RTCP session that a packet was received in the             **
**  corresponding RTP session.                                             **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      localTimestamp  The local timestamp for the received packet.       **
**                                                                         **
**      timestamp  The RTP timestamp from the received packet.             **
**                                                                         **
**      sequence   The packet sequence number.                             **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpRTPPacketRecv(
                IN  HRTCPSESSION  hRTCP,
                IN  UINT32        ssrc,
                IN  UINT32        localTimestamp,
                IN  UINT32        myTimestamp,
                IN  UINT16        sequence)

{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpInfo info, *fInfo;

    if (ssrc == s->myInfo.ssrc)
    {
      s->myInfo.collision = 1;
      return ERR_RTCP_SSRCCOLLISION;
    }

    info.ssrc = ssrc;

    /* See if we can find this source or not */
    fInfo = findSSrc(s,ssrc);

#ifdef RTP_NOLOCKS
    if (!fInfo) /* New source */
    {
        rtcpHeader head;

        /* create an invalid record */
        head = makeHeader(ssrc, 0, RTCP_RR, SIZEOF_RTCPHEADER);

        /* we'll send it to ourselves to make sure we can handle the change
           in the RTCP's database in multi-threaded environments without locking it. */
        liUdpSend(s->socket, (UINT8 *)&head, (int)SIZEOF_RTCPHEADER, localIP,
                  liGetSockPort(s->socket));
    }

#else  /* RTP_NOLOCKS */
    /* The new way of doing things: If we didn't find this SSRC, we lock the
       RTCP database and search for the SSRC again. If we don't find it again - we
       insert it to the list, and finally we unlock... */

    if (!fInfo)  /* New source */
    {
      /* this section is working with threads.*/
      /* Lock the rtcp session.*/
      mutexLock(s->hMutex);

      /* check if the ssrc is exist*/
      fInfo = findSSrc(s,ssrc);

      if (!fInfo)
      {
          /* Still no SSRC - we should add it to the list ourselves */
          fInfo = insertNewSSRC(s, ssrc);
      }

      /* unlock the rtcp session.*/
      mutexUnlock(s->hMutex);
    }
#endif  /* RTP_NOLOCKS */

    if (fInfo)
    {
        if (!fInfo->invalid)
        {
            fInfo->active = TRUE;
            update_seq(&(fInfo->src), sequence, localTimestamp, myTimestamp);
        }
    }

    return 0;
}


/*=========================================================================**
**  == rtcpRTPPacketSent() ==                                              **
**                                                                         **
**  Informs the RTCP session that a packet was sent in the corresponding   **
**  RTP session.                                                           **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      bytes      The number of bytes in the sent packet.                 **
**                                                                         **
**      timestamp  The RTP timestamp from the received packet.             **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpRTPPacketSent(
                IN  HRTCPSESSION  hRTCP,
                IN  INT32         bytes,
                IN  UINT32        timestamp)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    s->myInfo.active = TRUE;
    s->myInfo.eSR.nPackets++;
    s->myInfo.eSR.nBytes += bytes;
    s->myInfo.eSR.tNNTP = getNNTPTime();
    s->myInfo.eSR.tRTP = timestamp;

    if (s->myInfo.collision)
      return ERR_RTCP_SSRCCOLLISION;

    return 0;
}


/*=========================================================================**
**  == rtcpGetPort() ==                                                    **
**                                                                         **
**  Gets the UDP port of an RTCP session.                                  **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
UINT16 RVCALLCONV rtcpGetPort(
                IN  HRTCPSESSION  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    UINT16 sockPort;

    sockPort=liGetSockPort(s->socket);
    return sockPort;
}

                   /* == ENDS: Basic RTCP Functions == */



                    /* == Accessory RTCP Functions == */


/*=========================================================================**
**  == rtcpCheckSSRCCollision() ==                                         **
**                                                                         **
**  Checks for SSRC collisions in the RTCP session and the corresponding   **
**  RTP session.                                                           **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**  RETURNS:                                                               **
**      TRUE is returned if a collision was detected, otherwise FALSE.     **
**                                                                         **
**=========================================================================*/

RVAPI
BOOL RVCALLCONV rtcpCheckSSRCCollision(
                IN  HRTCPSESSION  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    return (s->myInfo.collision != 0);
}


/*=========================================================================**
**  == rtcpEnumParticipants() ==                                           **
**                                                                         **
**  Provides information about in the RTCP session and the corresponding   **
**  RTP session.                                                           **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      enumerator A pointer to the function that will be called once per  **
**                 SSRC in the session.                                    **
**                                                                         **
**  RETURNS:                                                               **
**      If the enumeration process was stopped by the enumerator, the      **
**      function returns FALSE, otherwise TRUE.                            **
**                                                                         **
**  The prototype of the SSRC enumerator is as follows:                    **
**                                                                         **
**      BOOL                                                               **
**      SSRCENUM(                                                          **
**        IN  HRTPSESSION  hTRCP,                                          **
**        IN  UINT32       ssrc                                            **
**      );                                                                 **
**                                                                         **
**  The parameters passed to the enumerator are as follows:                **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      ssrc       A synchronization source that participates in the       **
**                 session.                                                **
**                                                                         **
**  The enumerator should return FALSE if it wants the enumeration process **
**  to continue.  Returning TRUE will cause rtcpEnumParticipant() to       **
**  return immediately.                                                    **
**                                                                         **
**=========================================================================*/

RVAPI
BOOL RVCALLCONV rtcpEnumParticipants(
                IN HRTCPSESSION hRTCP,
                IN LPSSRCENUM   enumerator)
{
    int elem, ssrc=0;

    elem = rtcpGetEnumFirst(hRTCP, &ssrc);

    while (elem >= 0)
    {
        if (enumerator(hRTCP, ssrc))
        {
            return FALSE;
        }

        elem = rtcpGetEnumNext(hRTCP, elem, &ssrc);
    }

    return TRUE;
}


/*=========================================================================**
**  == rtcpGetSourceInfo() ==                                              **
**                                                                         **
**  Provides information about a particular synchronization source.        **
**                                                                         **
**  TBD                                                                    **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpGetSourceInfo(
                IN   HRTCPSESSION  hRTCP,
                IN   UINT32        ssrc,
                OUT  RTCPINFO *    info)


{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpInfo *fInfo, intInfo;

    if (ssrc == s->myInfo.ssrc)
    {
        info->selfNode         = TRUE;
        info->sr.valid         = s->myInfo.active;
        info->sr.mNTPtimestamp = s->myInfo.eSR.tNNTP.msdw;
        info->sr.lNTPtimestamp = s->myInfo.eSR.tNNTP.lsdw;
        info->sr.timestamp     = s->myInfo.eSR.tRTP;
        info->sr.packets       = s->myInfo.eSR.nPackets;
        info->sr.octets        = s->myInfo.eSR.nBytes;

        strncpy(info->cname, s->myInfo.eCName.value, sizeof(info->cname));

        return 0;
    }

    intInfo.ssrc = ssrc;

    fInfo = findSSrc(s,ssrc);

    if (fInfo)
    {
        info->selfNode              = FALSE;

        info->sr.valid              = !fInfo->invalid;
        info->sr.mNTPtimestamp      = fInfo->eSR.tNNTP.msdw;
        info->sr.lNTPtimestamp      = fInfo->eSR.tNNTP.lsdw;
        info->sr.timestamp          = fInfo->eSR.tRTP;
        info->sr.packets            = fInfo->eSR.nPackets;
        info->sr.octets             = fInfo->eSR.nBytes;

        info->rrFrom.valid          = TRUE;
        info->rrFrom.fractionLost   = (fInfo->eFromRR.bfLost >> 24);
        info->rrFrom.cumulativeLost = (fInfo->eFromRR.bfLost & 0xffffff);
        info->rrFrom.sequenceNumber = fInfo->eFromRR.nExtMaxSeq;
        info->rrFrom.jitter         = fInfo->eFromRR.nJitter;
        info->rrFrom.lSR            = fInfo->eFromRR.tLSR;
        info->rrFrom.dlSR           = fInfo->eFromRR.tDLSR;

        info->rrTo.valid            = TRUE;
        info->rrTo.fractionLost     = (fInfo->eToRR.bfLost >> 24);
        info->rrTo.cumulativeLost   = (fInfo->eToRR.bfLost & 0xffffff);
        info->rrTo.sequenceNumber   = fInfo->eToRR.nExtMaxSeq;
        info->rrTo.jitter           = fInfo->eToRR.nJitter;
        info->rrTo.lSR              = fInfo->eToRR.tLSR;
        info->rrTo.dlSR             = fInfo->eToRR.tDLSR;

        strncpy(info->cname, fInfo->eCName.value, sizeof(info->cname));
    }

    return (!fInfo) ? ERR_RTCP_ILLEGALSSRC : 0;
}


/*=========================================================================**
**  == rtcpSetGroupAddress() ==                                            **
**                                                                         **
**  Specifies a multicast IP for an RTCP session.                          **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      ip         The multicast IP address for the RTCP session.          **
**                 SSRC in the session.                                    **
**                                                                         **
**  RETURNS:                                                               **
**      If the enumeration process was stopped by the enumerator, the      **
**      function returns FALSE, otherwise TRUE.                            **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpSetGroupAddress(
                IN  HRTCPSESSION  hRTCP,
                IN  UINT32        ip)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    liJoinMulticastGroup(s->socket, ip, LI_ADDR_ANY);

    return 0;
}


/*=========================================================================**
**  == rtcpGetSSRC() ==                                                    **
**                                                                         **
**  Returns the synchronization source value for an RTCP session.          **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**  RETURNS:                                                               **
**      The synchronization source value for the specified RTCP session.   **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpGetSSRC(
                IN  HRTCPSESSION  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    return s->myInfo.ssrc;
}


/*=========================================================================**
**  == rtcpSetSSRC() ==                                                    **
**                                                                         **
**  Changes the synchronization source value for an RTCP session.          **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      ssrc       A synchronization srouce value for the RTCP session.    **
**                                                                         **
**  RETURNS:                                                               **
**      If the enumeration process was stopped by the enumerator, the      **
**      function returns FALSE, otherwise TRUE.                            **
**                                                                         **
**=========================================================================*/

RVAPI
INT32 RVCALLCONV rtcpSetSSRC(
                IN  HRTCPSESSION  hRTCP,
                IN  UINT32        ssrc)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    s->myInfo.ssrc      = ssrc;
    s->myInfo.collision = 0;

    return 0;
}

                 /* == ENDS: Accessory RTCP Functions == */



                     /* == Internal RTCP Functions == */


RVAPI
INT32 RVCALLCONV rtcpGetEnumFirst(
                IN  HRTCPSESSION  hRTCP,
                IN  INT32 *       ssrc)
{
    return rtcpGetEnumNext(hRTCP, -1, ssrc);
}

RVAPI
INT32 RVCALLCONV rtcpGetEnumNext(
                IN  HRTCPSESSION  hRTCP,
                IN  INT32         prev,
                IN  INT32 *       ssrc)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpInfo* info;
    INT32 index, doAgain = 1;

    if (prev < 0)
        index = 0;
    else
        index = prev+1;

    while ((doAgain == 1)&&(index < s->sessionMembers))
    {
        info = &s->participantsArray[index];
        if (!info->invalid)
        {
            doAgain = 0;
            *ssrc = info->ssrc;
        }
        else
        {
            index++;
        }
    }
    if (index < s->sessionMembers)
        return index;
    else
        return -1;
}

RVAPI
INT32 RVCALLCONV rtcpCreateRTCPPacket(
                IN      HRTCPSESSION  hRTCP,
                IN OUT  BUFFER *      buf)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpHeader head;
    UINT32 allocated = 0;
    BUFFER bufC;
    rtcpType type = RTCP_SR;
    int            index;

    if (buffValid(buf, SIZEOF_RTCPHEADER + SIZEOF_SR))
    {
        RV_UINT64 myTime = s->myInfo.eSR.tNNTP;
        UINT8 cc = 0;
        rtcpInfo *info;

        allocated = SIZEOF_RTCPHEADER;

        if (s->myInfo.active)
        {
            s->myInfo.active = FALSE;
            bufC = buffCreate(&(s->myInfo.eSR), SIZEOF_SR);
            buffAddToBuffer(buf, &bufC, allocated);
            liConvertHeader2l(buf->buffer + allocated, 0, (int)W32Len(bufC.length));
            allocated += SIZEOF_SR;
        }
        else
        {
            type = RTCP_RR;
        }


        index = 0;

        while( index < s->sessionMembers)
        {
            info = &s->participantsArray[index];
            if (info->active)
            {
                info->eToRR.bfLost     = getLost    (&(info->src));
                info->eToRR.nJitter    = getJitter  (&(info->src));
                info->eToRR.nExtMaxSeq = getSequence(&(info->src));
                info->eToRR.tDLSR      =
                    (info->tLSRmyTime) ?
                    (reduceNNTP(myTime)-info->tLSRmyTime) :
                    0;

                bufC = buffCreate(&(info->eToRR), SIZEOF_RR);

                if (buffAddToBuffer(buf, &bufC, allocated))
                {
                    cc++;
                    if (cc == 32)
                        break;
                    liConvertHeader2l(buf->buffer + allocated, 0,
                                      (int)W32Len(bufC.length));
                    allocated += SIZEOF_RR;
                }
                info->active = FALSE;
            }

           index++;
        }

        head = makeHeader(s->myInfo.ssrc, cc, type, (UINT16)allocated);
        bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
        buffAddToBuffer(buf, &bufC, 0);

        /* add an CNAME SDES packet to the compound packet */
        if (buffValid(buf,
            allocated + SIZEOF_RTCPHEADER + SIZEOF_SDES(s->myInfo.eCName)))
        {
            BUFFER sdes_buf;

            /* 'sdes_buf' is inside the compound buffer 'buf' */
            sdes_buf = buffCreate(buf->buffer + allocated,
                (SIZEOF_RTCPHEADER + SIZEOF_SDES(s->myInfo.eCName)));

            head = makeHeader(s->myInfo.ssrc, 1, RTCP_SDES,
              (UINT16)sdes_buf.length);

            memcpy(sdes_buf.buffer, (char *)&head, SIZEOF_RTCPHEADER);
            memcpy(sdes_buf.buffer + SIZEOF_RTCPHEADER, &(s->myInfo.eCName),
                   SIZEOF_SDES(s->myInfo.eCName));

            allocated += sdes_buf.length;
        }

        if (s->myInfo.collision == 1  &&
            buffValid(buf, allocated + SIZEOF_RTCPHEADER))
        {
            head = makeHeader(s->myInfo.ssrc, 1, RTCP_BYE,
                              SIZEOF_RTCPHEADER);

            bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
            buffAddToBuffer(buf, &bufC, allocated);
            s->myInfo.collision = 2;
            allocated += SIZEOF_RTCPHEADER;
        }
    }

    buf->length = allocated;

    return 0;
}


RVAPI
INT32 RVCALLCONV rtcpProcessCompoundRTCPPacket(
        IN      HRTCPSESSION  hRTCP,
        IN OUT  BUFFER *      buf,
        IN      RV_UINT64        myTime)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpHeader *head;
    BYTE *currPtr = buf->buffer, *dataPtr, *compoundEnd;
    int hdr_count, hdr_len;
    rtcpType hdr_type;

    compoundEnd = buf->buffer + buf->length;

    while (currPtr < compoundEnd)
    {
        if ((compoundEnd + 1 - currPtr) < 1)
        {
            return ERR_RTCP_ILLEGALPACKET;
        }

        head = (rtcpHeader*)(currPtr);
        liConvertHeader2l(currPtr, 0, 1);

        hdr_count = bitfieldGet(head->bits, HEADER_RC, HDR_LEN_RC);
        hdr_type  = (rtcpType)bitfieldGet(head->bits, HEADER_PT, HDR_LEN_PT);
        hdr_len   = sizeof(UINT32) *
                    (bitfieldGet(head->bits, HEADER_len, HDR_LEN_len));

        if ((compoundEnd - currPtr) < hdr_len)
        {
            return ERR_RTCP_ILLEGALPACKET;
        }

        dataPtr = (BYTE *)head + sizeof(UINT32);

        rtcpProcessRTCPPacket(s, dataPtr, hdr_len, hdr_type, hdr_count,
                              myTime);

        currPtr += hdr_len + sizeof(UINT32);
    }

    return 0;
}



RVAPI
INT32 RVCALLCONV rtcpProcessRTCPPacket(
        IN  rtcpSession *  s,
        IN  BYTE *         data,
        IN  INT32          dataLen,
        IN  rtcpType       type,
        IN  INT32          reportCount,
        IN  RV_UINT64      myTime)
{
    unsigned scanned = 0;
    rtcpInfo info, *fInfo=NULL;

    if (dataLen == 0)
        return 0;

    switch(type)
    {
        case RTCP_SR:
        case RTCP_RR:
        {
            liConvertHeader2l(data, 0, 1);
            info.ssrc = *(UINT32 *)(data);
            scanned = sizeof(UINT32);

            if (info.ssrc == s->myInfo.ssrc)
            {
                s->myInfo.collision = 1;

                return ERR_RTCP_SSRCCOLLISION;
            }

            fInfo = findSSrc(s,info.ssrc);

            if (!fInfo) /* New source */
            {
                /* insert the new source */
#ifndef RTP_NOLOCKS
                mutexLock(s->hMutex);
#endif
                fInfo = insertNewSSRC(s, *(UINT32 *)data);
#ifndef RTP_NOLOCKS
                mutexUnlock(s->hMutex);
#endif
            }
            break;
        }

        default:
            break;
    }

    /* process the information */
    switch(type)
    {
        case RTCP_SR:
        {
            liConvertHeader2l(data + scanned, 0, W32Len(sizeof(rtcpSR)));

            if (fInfo)
            {
                fInfo->eSR        = *(rtcpSR *)(data + scanned);
                fInfo->eToRR.tLSR = reduceNNTP(fInfo->eSR.tNNTP);
                fInfo->tLSRmyTime = reduceNNTP(myTime);
            }

            scanned += SIZEOF_SR;
        }

        /* fall into RR */

        case RTCP_RR:
        {
            if (fInfo)
            {
                int i;
                rtcpRR* rr = (rtcpRR*)(data + scanned);

                liConvertHeader2l(data + scanned, 0,
                                  reportCount * W32Len(sizeof(rtcpRR)));

                for (i=0; i < reportCount; i++)
                {
                    if (rr[i].ssrc == s->myInfo.ssrc)
                    {
                        fInfo->eFromRR = rr[i];
                        break;
                    }
                }
            }

            break;
        }

        case RTCP_SDES:
        {
            int i;
            rtcpSDES *sdes;

            for (i = 0; i < reportCount; i++)
            {
                liConvertHeader2l(data + scanned, 0, 1);
                info.ssrc = *(UINT32 *)(data + scanned);

                sdes = (rtcpSDES *)(data + scanned + sizeof(info.ssrc));

                fInfo = findSSrc(s,info.ssrc);
                if (fInfo)
                {
                    switch(sdes->type)
                    {
                        case RTCP_SDES_CNAME:
                            memcpy(&(fInfo->eCName), sdes,
                                   SIZEOF_SDES(*sdes));
                            fInfo->eCName.value[sdes->length] = 0;
                            break;
/* known SDES types that are not handled:
                        case RTCP_SDES_END:
                        case RTCP_SDES_NAME:
                        case RTCP_SDES_EMAIL:
                        case RTCP_SDES_PHONE:
                        case RTCP_SDES_LOC:
                        case RTCP_SDES_TOOL:
                        case RTCP_SDES_NOTE:
                        case RTCP_SDES_PRIV:
                            break;
*/
                        }
                    }

                    scanned += SIZEOF_SDES(*sdes) + sizeof(UINT32);
                }

            break;
        }

        case RTCP_BYE:
        {
            int i;

            for (i = 0; i < reportCount; i++)
            {
                liConvertHeader2l(data + scanned, 0, 1);
                info.ssrc = *(UINT32 *)(data + scanned);
                scanned += sizeof(info.ssrc);

                fInfo = findSSrc(s,info.ssrc);
                if (fInfo)
                {
                    /* We don't really delete this SSRC, we just mark it as invalid */
                    fInfo->invalid = TRUE;
                    fInfo->ssrc    = 0;
                }
            }

            break;
        }

        case RTCP_APP:
            break;
    }

    return 0;
}



static void RVCALLCONV rtcpTimerCallback(void* key)
{
    rtcpSession* s = (rtcpSession*)key;
    UINT32 buffer[MAXRTCPPACKET/sizeof(UINT32)+1];
    BUFFER buf;

    buf = buffCreate(buffer,MAXRTCPPACKET);

    /* s->tElem =mtimerSet(hst,rtcpTimerCallback,s,5000  should be calculated); */
    rtcpCreateRTCPPacket((HRTCPSESSION)s, &buf);
    liUdpSend(s->socket, buf.buffer, (int)buf.length, s->ip, s->port);
}


static void rtcpLiCallback(int socket, liEvents event, int error,
                           void *context)
{
    rtcpSession* s = (rtcpSession*)context;
    UINT32 ip;
    UINT16 port;
    UINT32 buffer[MAXRTCPPACKET/sizeof(UINT32)+1];
    BUFFER buf;

    if(socket || event || error);

    if (s == NULL)
        return;

    buf = buffCreate(buffer, MAXRTCPPACKET);

    buf.length = liUdpRecv(s->socket, buf.buffer, (int)buf.length, &ip, &port);

    if (port != liGetSockPort(s->socket) || !isMyIP(ip))
    {
        INT32 res=rtcpProcessCompoundRTCPPacket((HRTCPSESSION)s, &buf, getNNTPTime());
        if ((res==0) && (s->rtcpRecvCallback != NULL))
        {
           UINT32 ssrc=getSSRCfrom(buf.buffer);
           s->rtcpRecvCallback((HRTCPSESSION)s, s->haRtcp, ssrc);
        }
    }
}

static RV_UINT64 getNNTPTime(void)
{
    const  UINT32 from1900Till1970 = (UINT32)2208988800ul;
    static UINT32 startTime = 0;
    static UINT32 startMilliTime = 0;
    RV_UINT64 nntpTime;

    if (!startTime)
    {
        startTime = (UINT32)time(NULL) + from1900Till1970;
        startMilliTime = timerGetTimeInMilliseconds();
    }

    nntpTime.msdw = startTime;
    nntpTime.lsdw = timerGetTimeInMilliseconds() - startMilliTime;
    nntpTime.msdw += nntpTime.lsdw/1000;
    nntpTime.lsdw %= 1000;
    nntpTime.lsdw *= 4294967;

    return nntpTime;
}

static BOOL isMyIP(UINT32 ip)
{
    int i;

    for (i=0; myIPs[i]; i++)
    {
        if (ip == myIPs[i])
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void setSDES(rtcpSDesType type, rtcpSDES* sdes, BYTE *data, int length)
{
    sdes->type   = (unsigned char)type;
    sdes->length = (unsigned char)length;
    memcpy(sdes->value,        data, length);
    memset(sdes->value+length, 0,    4-((length+2)%sizeof(UINT32)));
}

static void init_seq(rtpSource *s, UINT16 seq)
{
   s->base_seq       = seq;
   s->max_seq        = seq;
   s->bad_seq        = RTP_SEQ_MOD + 1;
   s->cycles         = 0;
   s->received       = 0;
   s->received_prior = 0;
   s->expected_prior = 0;
}


static int update_seq(rtpSource *s, UINT16 seq, UINT32 ts, UINT32 arrival)
{
    UINT16 udelta = (UINT16)(seq - s->max_seq);

    if (s->probation)
    {
        if (seq == s->max_seq + 1)
        {
            s->probation--;
            s->max_seq = seq;
            if (s->probation == 0)
            {
                init_seq(s, seq);
                s->received++;
                return 1;
            }
        }
        else
        {
            s->probation = MIN_SEQUENTIAL - 1;
            s->max_seq = seq;
        }
        return 0;
    }
    else if (udelta < MAX_DROPOUT)
    {
        if (seq < s->max_seq) s->cycles += RTP_SEQ_MOD;
        s->max_seq = seq;
    }
    else if (udelta <= RTP_SEQ_MOD - MAX_MISORDER)
    {
        if (seq == s->bad_seq)
        {
            init_seq(s, seq);
        }
        else
        {
            s->bad_seq = (seq + 1) & (RTP_SEQ_MOD-1);
            return 0;
        }
    }
    else
    {
   /* duplicate or reordered packet */
    }
    {
        INT32  transit = (INT32)(arrival - ts);
        INT32  d = (INT32)(transit - s->transit);
        s->transit = transit;
        if (d < 0) d = -d;
        s->jitter += d - ((s->jitter + 8) >> 4);
    }
    s->received++;
    return 1;
}


/*=========================================================================**
**  == makeHeader() ==                                                     **
**                                                                         **
**  Creates an RTCP packet header.                                         **
**                                                                         **
**  PARAMETERS:                                                            **
**      ssrc       A synchronization source value for the RTCP session.    **
**                                                                         **
**      count      A count of sender and receiver reports in the packet.   **
**                                                                         **
**      type       The RTCP packet type.                                   **
**                                                                         **
**      dataLen    The length of the data in the packet buffer, in         **
**                 octets, including the size of the header.               **
**                                                                         **
**  RETURNS:                                                               **
**      The function returns a header with the appropriate parameters.     **
**                                                                         **
**=========================================================================*/

static rtcpHeader makeHeader(UINT32 ssrc, UINT8 count, rtcpType type,
                             UINT16 dataLen)
{
    rtcpHeader header;

    header.ssrc = ssrc;

    header.bits = RTCP_HEADER_INIT;
    header.bits = bitfieldSet(header.bits, count, HEADER_RC, HDR_LEN_RC);
    header.bits = bitfieldSet(header.bits, type,  HEADER_PT, HDR_LEN_PT);
    header.bits = bitfieldSet(header.bits, W32Len(dataLen) - 1,
                              HEADER_len, HDR_LEN_len);

    liConvertHeader2h((UINT8 *)&header, 0, W32Len(SIZEOF_RTCPHEADER));

    return header;
}


static UINT32 getLost(rtpSource *s)
{
    UINT32 extended_max;
    UINT32 expected;
    INT32  received_interval;
    INT32  expected_interval;
    INT32  lost;
    INT32  lost_interval;
    UINT8  fraction;

    extended_max = s->cycles + s->max_seq;
    expected = extended_max - s->base_seq + 1;
    lost = expected - s->received;
    expected_interval = expected - s->expected_prior;
    s->expected_prior = expected;
    received_interval = s->received - s->received_prior;
    s->received_prior = s->received;
    lost_interval = expected_interval - received_interval;

    if (expected_interval == 0  ||  lost_interval <= 0)
        fraction = 0;
    else
        fraction = (UINT8)((lost_interval << 8) / expected_interval);

    return (fraction << 24) + lost;
}


static UINT32 getJitter(rtpSource *s)
{
    return s->jitter >> 4;
}

static UINT32 getSequence(rtpSource *s)
{
    return s->max_seq + s->cycles;
}



static UINT32 getSSRCfrom(BYTE *head)
{
   BYTE *ssrcPtr = (BYTE *)head + sizeof(UINT32);
   return *(UINT32 *)(ssrcPtr);
}


static rtcpInfo *findSSrc(rtcpSession *s, UINT32 ssrc)
{
    int     index = 0;
    BOOL    doAgain = TRUE;
    rtcpInfo *pInfo;
    if (s == NULL)
        return NULL;

    /* Look for the given SSRC */
    while ((doAgain) && (index < s->sessionMembers))
    {
       if (s->participantsArray[index].ssrc == ssrc)
            doAgain = FALSE;
       else
           index ++;

    }
    if (index < s->sessionMembers )
        pInfo = &s->participantsArray[index];
    else
        pInfo = NULL;

    return pInfo;
}


static rtcpInfo *insertNewSSRC(rtcpSession *s, UINT32 ssrc)
{
    rtcpInfo* pInfo = NULL;
    int index;

    if (s->sessionMembers >= s->maxSessionMembers)
    {
        /* We've got too many - see if we can remove some old ones */
        index = 0;
        while ((index < s->sessionMembers) &&
               (s->participantsArray[index].invalid && s->participantsArray[index].ssrc == 0))
            index++;
    }
    else
    {
        /* Add it as a new one to the list */
        index = s->sessionMembers;
        s->sessionMembers++;
    }

    if (index < s->sessionMembers)
    {
        /* Got a place for it ! */
        pInfo = &s->participantsArray[index];
        memset(pInfo, 0, sizeof(rtcpInfo));
        pInfo->ssrc             = ssrc;
        pInfo->eToRR.ssrc       = ssrc;
        pInfo->active           = FALSE;
        pInfo->src.probation    = MIN_SEQUENTIAL - 1;
    }

    return pInfo;
}



                  /* == ENDS: Internal RTCP Functions == */
#ifdef __cplusplus
}
#endif



