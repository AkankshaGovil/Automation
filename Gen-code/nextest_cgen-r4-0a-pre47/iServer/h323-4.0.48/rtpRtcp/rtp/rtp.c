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

  rtp.c  --  RTP implementation.

  Abstract:       The main RTP module file.
  Platforms:      All.
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include <rvcommon.h>
#ifdef _ATM_
#include <rvatm.h>
#endif

#include "bitfield.h"
#include <li.h>
#include <ti.h>
#ifdef _ATM_
#include <liatm.h>
#endif

#include "rtp.h"
#include "rtcp.h"
#ifdef _ATM_
#include "rtpatm.h"
#endif

#define BUILD_VER_NUM(max, min, dot1, dot2) \
    ((UINT32)((max << 24) + (min << 16) + (dot1 << 8) + dot2))

#define VERSION_STR    "4.0.0.36"
#define VERSION_NUM    BUILD_VER_NUM(4, 0, 0, 36)


#define MAXIPS 20


static UINT32 ipBindTo=LI_ADDR_ANY;
static UINT32 myIPs[MAXIPS];
static UINT32 rtpInitialized = 0;

#ifdef _MTS_
static unsigned int seed;
#endif
/* local functions */
static BOOL isMyIP(UINT32 ip);
static void rtpEvent(int socket, liEvents event, int error, void *context);



                       /* == Basic RTP Functions == */


RVAPI
INT32 RVCALLCONV rtpInit(void)
{
    int i;
    UINT32 **ips;

    liInit();
    liThreadAttach(NULL);

    if (rtpInitialized == 0)
    {
        ips = liGetHostAddrs();
        if (!ips)
            return RVERROR;

        for (i=0; ips[i]; i++)
            myIPs[i] = *(ips[i]);

#ifdef _MTS_
        seed = (unsigned)timerGetTimeInSeconds() * myIPs[0];
#else
        srand((unsigned)timerGetTimeInSeconds() * myIPs[0]);
#endif
    }

    rtpInitialized++;

    return 0;
}


RVAPI
INT32 RVCALLCONV rtpInitEx(UINT32 ip)
{
    INT32 rc;

    if ((rc=rtpInit()) != RVERROR)
      ipBindTo=ip;

    return rc;
}


/************************************************************************************
 * rtpSetLocalAddress
 * description: Set the local address to use for calls to rtpOpenXXX functions.
 *              This parameter overrides the value given in rtpInitEx() for all
 *              subsequent calls.
 * input: ip    - Local IP address to use
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
int RVCALLCONV rtpSetLocalAddress(IN UINT32 ip)
{
    ipBindTo = ip;
    return 0;
}


RVAPI
void RVCALLCONV rtpEnd(void)
{
    liThreadDetach(NULL);
    liEnd();
    rtpInitialized--;
}


RVAPI
int RVCALLCONV rtpGetAllocationSize(void)
{
    return sizeof(rtpSession);
}

/************************************************************************************
 * rtpOpenFrom
 * description: Opens an RTP session in the memory that the application allocated.
 * input: port        - The UDP port number to be used for the RTP session.
 *        ssrcPattern - Synchronization source Pattern value for the RTP session.
 *        ssrcMask    - Synchronization source Mask value for the RTP session.
 *        buffer      - Application allocated buffer with a value no less than the
 *                      value returned by the function rtpGetAllocationSize().
 *        bufferSize  - size of the buffer.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the opened RTP
 *               session. Otherwise, it returns NULL.
 ***********************************************************************************/
RVAPI
HRTPSESSION RVCALLCONV rtpOpenFrom(
        IN  UINT16  port,
        IN  UINT32  ssrcPattern,
        IN  UINT32  ssrcMask,
        IN  void*   buffer,
        IN  int     bufferSize)
{
    rtpSession *s = (rtpSession *)buffer;

    if (bufferSize < rtpGetAllocationSize())
        return NULL;

    memset(buffer, 0 , rtpGetAllocationSize());

    s->isAllocated    = FALSE;
    s->sSrcPattern    = ssrcPattern;
    s->sSrcMask       = ssrcMask;
    s->sequenceNumber = (UINT16)
#ifdef _MTS_
      rand_r(&seed);
#else
      rand();
#endif
    s->socket         = liOpen(ipBindTo, port, LI_UDP);
/* h.e */
    if (s->socket == RVERROR)
    {
      return (HRTPSESSION)NULL;
    }
    else
/* === */
    {
      rtpRegenSSRC((HRTPSESSION)s);
      liBlock(s->socket);
      return (HRTPSESSION)s;
    }

}


/************************************************************************************
 * rtpOpen
 * description: Opens an RTP session. The RTP Stack allocates an object and the
 *              memory needed for the RTP session. It also opens a socket and waits
 *              for packets. rtpOpen() also returns the handle of this session to
 *              the application.
 * input: port        - The UDP port number to be used for the RTP session.
 *        ssrcPattern - Synchronization source Pattern value for the RTP session.
 *        ssrcMask    - Synchronization source Mask value for the RTP session.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the opened RTP
 *               session. Otherwise, it returns NULL.
 ***********************************************************************************/
RVAPI
HRTPSESSION RVCALLCONV rtpOpen(
        IN  UINT16  port,
        IN  UINT32  ssrcPattern,
        IN  UINT32  ssrcMask)
{
    return rtpOpenEx(port, ssrcPattern, ssrcMask, NULL);
}

/************************************************************************************
 * rtpOpenEx
 * description: Opens an RTP session and an associated RTCP session.
 * input: port        - The UDP port number to be used for the RTP session.
 *        ssrcPattern - Synchronization source Pattern value for the RTP session.
 *        ssrcMask    - Synchronization source Mask value for the RTP session.
 *        cname       - The unique name representing the source of the RTP data.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the open
 *               RTP session. Otherwise, the function returns NULL.
 ***********************************************************************************/
RVAPI
HRTPSESSION RVCALLCONV rtpOpenEx(
        IN  UINT16  port,
        IN  UINT32  ssrcPattern,
        IN  UINT32  ssrcMask,
        IN  char *  cname)
{
    /* allocate rtp session.*/
    rtpSession *s = (rtpSession*)calloc(rtpGetAllocationSize(),1);

    if (s==NULL)
        return NULL;

    if ((rtpSession *)rtpOpenFrom(port, ssrcPattern, ssrcMask, (void*)s, rtpGetAllocationSize())==NULL)
    {
        free(s);
        return NULL;
    }
    s->isAllocated=TRUE;
    if (cname)
    {
        /* Open a new rtcp session.The port for an RTCP session is always
           (RTP port + 1).*/
        s->hRTCP = rtcpOpen(s->sSrc, (UINT16)((port)?port+1:port), cname);
    }

    return (HRTPSESSION)s;
}

/************************************************************************************
 * rtpClose
 * description: Close RTP session.
 * input: hRTP - Handle of the RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
UINT32 RVCALLCONV rtpClose(
        IN  HRTPSESSION  hRTP)
{
    rtpSession *s = (rtpSession *)hRTP;

    if (s)
    {
        if (s->hRTCP)
            rtcpClose(s->hRTCP);

#ifndef RV_USE_R0
    /*h.e: I'm not sure we have to call liCallOn here for R0. liClose calls it.*/
    liCallOn(s->socket, (liEvents)0, NULL, NULL);
#endif

    /* Send udp data through specified socket to the local host */
        liUdpSend(s->socket, (UINT8*)"", 1, liConvertIp((char*)"127.0.0.1"),
            liGetSockPort(s->socket));
        /* This function closes the specified IP socket and all the socket's connections.*/
        liClose(s->socket);
        if (s->isAllocated)
            free(s);
    }

    return 0;
}

/************************************************************************************
 * rtpGetSSRC
 * description: Returns the current SSRC (synchronization source value) of the RTP session.
 * input: hRTP - Handle of the RTP session.
 * output: none.
 * return value: If no error occurs, the function returns the current SSRC value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
UINT32 RVCALLCONV rtpGetSSRC(
        IN  HRTPSESSION  hRTP)
{
    rtpSession *s = (rtpSession *)hRTP;

    return s->sSrc;
}

/************************************************************************************
 * rtpSetEventHandler
 * description: Set an Event Handler for the RTP session. The application must set
 *              an Event Handler for each RTP session.
 * input: hRTP          - Handle of the RTP session.
 *        eventHandler  - Pointer to the callback function that is called each time a
 *                        new RTP packet arrives to the RTP session.
 *        context       - The parameter is an application handle that identifies the
 *                        particular RTP session. The application passes the handle to
 *                        the Event Handler.
 * output: none.
 * return value: none.
 ***********************************************************************************/
RVAPI
void RVCALLCONV rtpSetEventHandler(
        IN  HRTPSESSION        hRTP,
        IN  LPRTPEVENTHANDLER  eventHandler,
        IN  void *             context)
{
    rtpSession *s = (rtpSession *)hRTP;

    if (s)
    {
        s->eventHandler = eventHandler;
        s->context      = context;

        liUnblock(s->socket);
        liCallOn(s->socket, liEvRead, (eventHandler) ? rtpEvent : NULL, s);
    }
}

/************************************************************************************
 * rtpSetRemoteAddress
 * description: Defines the address of the remote peer or the address of a multicast
 *              group to which the RTP stream will be sent.
 * input: hRTP  - Handle of the RTP session.
 *        ip    - IP address to which RTP packets should be sent.
 *        port  - UDP port to which RTP packets should be sent.
 * output: none.
 * return value: none.
 ***********************************************************************************/
RVAPI
void RVCALLCONV rtpSetRemoteAddress(
        IN HRTPSESSION  hRTP,   /* RTP Session Opaque Handle */
        IN UINT32       ip,
        IN UINT16       port)
{
    rtpSession *s = (rtpSession *)hRTP;

    if (s)
    {
    s->ip   = ip;
    s->port = port;
}
}
/************************************************************************************
 * rtpPack
 * description: This routine sets the RTP header.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *        p     - A struct of RTP param.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpPack(
        IN  HRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len,
        IN  rtpParam *   p)
{
    rtpSession *s = (rtpSession *)hRTP;
    UINT32 *header;
    UINT32 seq;

    p->sByte-=12;
    p->len=len - p->sByte;

    if (s->useSequenceNumber)
        s->sequenceNumber=p->sequenceNumber;
    p->sequenceNumber=s->sequenceNumber;
    seq = s->sequenceNumber;

    /* sets the fields inside RTP message.*/
    header=(UINT32*)((char*)buf + p->sByte);
    header[0]=0;
    header[0]=bitfieldSet(header[0],2,30,2);
    header[0]=bitfieldSet(header[0],p->marker,23,1);
    header[0]=bitfieldSet(header[0],p->payload,16,7);
    header[0]=bitfieldSet(header[0],seq++,0,16);
    header[1]=p->timestamp;
    header[2]=s->sSrc;

    /* increment the internal sequence number for this session */
    s->sequenceNumber++;

    /* converts an array of 4-byte integers from host format to network format.*/
    liConvertHeader2l((UINT8*)header,0,3);
    return 0;
}

/************************************************************************************
 * rtpWrite
 * description: This routine sends the RTP packet.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *        p     - A struct of RTP param.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpWrite(
        IN  HRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len,
        IN  rtpParam *   p)
{
    int retVal;
    rtpSession *s = (rtpSession *)hRTP;

    rtpPack(hRTP, buf, len, p);
    /* send udp data through the specified socket to the remote host.*/
    retVal = liUdpSend(s->socket, (UINT8*)buf+p->sByte, p->len, s->ip, s->port);

    if (s->hRTCP  &&  retVal >= 0)
    {
        /* inform the RTCP session about a packet that was sent in the corresponding RTP session.*/
        rtcpRTPPacketSent(s->hRTCP, p->len, p->timestamp);
    }

    return retVal;
}

RVAPI
INT32 RVCALLCONV rtpUnpack(
        IN  HRTPSESSION  hRTP,
        IN  void *buf,
        IN  INT32 len,
        OUT rtpParam* p)
{
    UINT32 *header=(UINT32*)buf;
    if (len || hRTP);

    if (p->len<12)
        return RVERROR;

    liConvertHeader2h((UINT8*)buf,3,(int)bitfieldGet(header[0],24,4));
    p->timestamp=header[1];
    p->sequenceNumber=(UINT16)bitfieldGet(header[0],0,16);
    p->sSrc=header[2];
    p->marker=bitfieldGet(header[0],23,1);
    p->payload=(unsigned char)bitfieldGet(header[0],16,7);

    p->sByte=12+bitfieldGet(header[0],24,4)*sizeof(UINT32);

    if (bitfieldGet(header[0],28,1))/*Extension Bit Set*/
    {
        int xStart=p->sByte / sizeof(UINT32);

        liConvertHeader2h((UINT8*)buf,xStart,1);
        p->sByte+=bitfieldGet(header[xStart],0,16)*sizeof(UINT32);
        if (p->sByte > p->len)
        {
            /* This packet is probably corrupted */
            p->sByte = 12;
            return RVERROR;
        }
    }

    if (bitfieldGet(header[0],29,1))/*Padding Bit Set*/
    {
        p->len-=((char*)buf)[p->len-1];
    }
    return 0;
}

/************************************************************************************
 * rtpRead
 * description: This routine sets the header of the RTP message.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *
 * output: p    - A struct of RTP param,contain the fields of RTP header.
 * return value: If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpRead(
        IN  HRTPSESSION  hRTP,
        IN  void *buf,
        IN  INT32 len,
        OUT rtpParam* p)
{
    rtpSession *s = (rtpSession *)hRTP;
    UINT32 ip;
    UINT16 port;

    p->len=liUdpRecv(s->socket,(unsigned char*)buf,len,&ip,&port);

    liConvertHeader2h((unsigned char*)buf,0,3);
    if (isMyIP(ip) && s->sSrc==((UINT32*)buf)[2]) return RVERROR;
    if (p->len==RVERROR) return RVERROR;

    return rtpUnpack(hRTP, buf, len, p);
}

/************************************************************************************
 * rtpReadEx
 * description: Receives an RTP packet and updates the corresponding RTCP session.
 * input: hRTP      - Handle of the RTP session.
 *        buf       - Pointer to buffer containing the RTP packet with room before first
 *                    payload byte for RTP header.
 *        len       - Length in bytes of buf.
 *        timestamp -
 *        p         - A struct of RTP param,contain the fields of RTP header.
 * output: none.
 * return value: If no error occurs, the function returns the non-neagtive value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpReadEx(
        IN  HRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len,
        IN  UINT32       timestamp,
        OUT rtpParam *   p)
{
    rtpSession *s = (rtpSession *)hRTP;
    int retVal;

    retVal = rtpRead(hRTP, buf, len, p);

    if (s->hRTCP  &&  retVal >= 0)
    {
        /* Informs the RTCP session about a packet that was received in the corresponding
           RTP session.*/
        rtcpRTPPacketRecv(s->hRTCP, p->sSrc, timestamp,  p->timestamp, p->sequenceNumber);
    }

    return retVal;
}


/************************************************************************************
 * rtpGetPort
 * description: Returns the current port of the RTP session.
 * input: hRTP - Handle of the RTP session.
 * output: none.
 * return value: If no error occurs, the function returns the current port value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
UINT16 RVCALLCONV rtpGetPort(
        IN HRTPSESSION  hRTP)   /* RTP Session Opaque Handle */
{
    UINT16 sockPort;
    rtpSession *s = (rtpSession *)hRTP;


    sockPort=liGetSockPort(s->socket);
    return sockPort;
}

/************************************************************************************
 * rtpGetVersion
 * description:  Returns the RTP version of the installed RTP Stack.
 * input:  none.
 * output: none.
 * return value: If no error occurs, the function returns the current version value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
char * RVCALLCONV rtpGetVersion(void)
{
    return (char*)VERSION_STR;
}

/************************************************************************************
 * rtpGetVersionNum
 * description:  Returns the RTP version of the installed RTP Stack.
 * input:  none.
 * output: none.
 * return value: If no error occurs, the function returns the current version value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
UINT32 RVCALLCONV rtpGetVersionNum(void)
{
    return VERSION_NUM;
}


                    /* == ENDS: Basic RTP Functions == */



                     /* == Accessory RTP Functions == */

/************************************************************************************
 * rtpGetRTCPSession
 * description:  Returns the RTCP session.
 * input:  hRTP - Handle of RTP session.
 * output: none.
 * return value: hRTCP - Handle of RTCP session.
 ***********************************************************************************/
RVAPI
HRTCPSESSION RVCALLCONV rtpGetRTCPSession(
        IN  HRTPSESSION  hRTP)
{
    rtpSession *s = (rtpSession *)hRTP;

    return (s->hRTCP);
}

/************************************************************************************
 * rtpSetRTCPSession
 * description:  set the RTCP session.
 * input:  hRTP  - Handle of RTP session.
 *         hRTCP - Handle of RTCP session.
 * output: none.
 * return value:return 0.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpSetRTCPSession(
        IN  HRTPSESSION   hRTP,
        IN  HRTCPSESSION  hRTCP)
{
    rtpSession *s = (rtpSession *)hRTP;

    s->hRTCP = hRTCP;

    return 0;
}

/************************************************************************************
 * rtpGetHeaderLength
 * description:  return the header of RTP message.
 * input:  none.
 * output: none.
 * return value:The return value is twelve.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpGetHeaderLength(void)
{
    return 12;
}

/************************************************************************************
 * rtpRegenSSRC
 * description:  Generates a new synchronization source value for the RTP session.
 *               This function, in conjunction with rtpGetSSRC() may be used to
 *               change the SSRC value when an SSRC collision is detected.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: ssrc - If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
UINT32 RVCALLCONV rtpRegenSSRC(
        IN  HRTPSESSION  hRTP)
{
    rtpSession *s = (rtpSession *)hRTP;

    /* those line is to prevent collision.*/
    s->sSrc =
#if defined( _MTS_ ) && ! defined( __LINUX__ )
      rand_r(&seed)
#else
    rand()
#endif
      * timerGetTimeInMilliseconds();
    s->sSrc &= ~s->sSrcMask;
    s->sSrc |= s->sSrcPattern;

    return s->sSrc;
}

/************************************************************************************
 * rtpSetGroupAddress
 * description:  Defines a multicast IP for the RTP session.
 * input:  hRTP  - Handle of RTP session.
 *         ip    - Multicast IP address for the RTP session.
 * output: none.
 * return value: return 0.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpSetGroupAddress(
        IN HRTPSESSION hRTP,    /* RTP Session Opaque Handle */
        IN UINT32       ip)
{
    rtpSession *s = (rtpSession *)hRTP;
    /* This function adds the specified address (in network format) to the specified
       Multicast interface for the specified socket.*/
    liJoinMulticastGroup(s->socket,ip,LI_ADDR_ANY);
    return 0;
}

/************************************************************************************
 * rtpResume
 * description:  Causes a blocked rtpRead() or rtpReadEx() function running in
 *               another thread to fail.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpResume(
        IN  HRTPSESSION hRTP)
{
    rtpSession *s = (rtpSession *)hRTP;
    UINT16 sockPort;

    sockPort=liGetSockPort(s->socket);
    liUdpSend(s->socket, (UINT8*)"", 1, liConvertIp((char*)"127.0.0.1"),
              sockPort);
    return 0;
}

/************************************************************************************
 * rtpUseSequenceNumber
 * description:  Forces the Stack to accept user input for the sequence number of
 *               the RTP packet. The RTP Stack usually determines the sequence number.
 *               However, the application can force its own sequence number.
 *               Call rtpUseSequenceNumber() at the beginning of the RTP session and
 *               then specify the sequence number in the rtpParam structure of the
 *               rtpWrite() function.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: return 0.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpUseSequenceNumber(
                IN HRTPSESSION  hRTP)
{
    rtpSession *s = (rtpSession *)hRTP;

    s->useSequenceNumber = 1;

    return 0;
}

/************************************************************************************
 * rtpSetReceiveBufferSize
 * description:  Changes the RTP session receive buffer size.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: return 0.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpSetReceiveBufferSize(
                IN HRTPSESSION  hRTP,
        IN int size)
{
    rtpSession *s = (rtpSession *)hRTP;
    /* This function sets the size of the read/write buffers of a socket*/
    liSetSocketBuffers(s->socket,-1,size);

    return 0;
}

/************************************************************************************
 * rtpSetTransmitBufferSize
 * description:  Changes the RTP session transmit buffer size.
 * input:  hRTP - Handle of RTP session.
 *         size - The new transmit buffer size.
 * output: none.
 * return value: return 0.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpSetTransmitBufferSize(
                IN HRTPSESSION  hRTP,
                IN int size)
{
    rtpSession *s = (rtpSession *)hRTP;
    /* This function sets the size of the read/write buffers of a socket*/
    liSetSocketBuffers(s->socket,size,-1);

    return 0;
}

/************************************************************************************
 * rtpSetTrasmitBufferSize
 * description:  Changes the RTP session transmit buffer size.
 * input:  hRTP - Handle of RTP session.
 *         size - The new transmit buffer size.
 * output: none.
 * return value: return 0.
 * comment     : obsolete function provided for compatibility with prev. version
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpSetTrasmitBufferSize(
                IN HRTPSESSION  hRTP,
                IN int size)
{
  return rtpSetTransmitBufferSize(hRTP, size);
}


/************************************************************************************
 * rtpGetAvailableBytes
 * description:  Gets the number of bytes available for reading in the RTP session.
 * input:  hRTP - Handle of RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpGetAvailableBytes(
                IN HRTPSESSION  hRTP)
{
    rtpSession *s = (rtpSession *)hRTP;
    int bytes=RVERROR;
    /* This function returns the number of bytes in the specified socket that are
       available for reading.*/
    liBytesAvailable(s->socket, &bytes);
    return bytes;
}


                  /* == ENDS: Accessory RTP Functions == */



                     /* == Internal RTP Functions == */


static void rtpEvent(int socket, liEvents event, int error, void *context)
{
    rtpSession* s=(rtpSession*)context;
    if(socket || event || error);

    if (s  &&  s->eventHandler)
    {
        s->eventHandler((HRTPSESSION)s,s->context);
    }
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

                  /* == ENDS: Internal RTP Functions == */

#ifdef _ATM_
                        /* == RTP ATM Functions == */


INT32 RVCALLCONV rtpatmInit(BOOL bListen, UINT8 atmSelector, UINT16 port)
{
    int res = rtpInit();
    liatmInit();
    return res;
}

RVAPI
void RVCALLCONV rtpatmEnd(void)
{
    rtpEnd();
    liatmEnd();
}


RVAPI
HRTPSESSION RVCALLCONV rtpatmOpenFrom(
        IN  UINT8               atmSelector,
        IN  UINT16              port,
        IN  RTPATMDIRECTION     Direction,
        IN  UINT32              ssrcPattern,
        IN  UINT32              ssrcMask,
        IN  void*               buffer,
        IN  int                 bufferSize)
{
    rtpSession * s  = (rtpSession *)buffer;

    if (bufferSize < rtpGetAllocationSize())
        return NULL;

    memset(buffer, 0 , rtpGetAllocationSize());

    s->bUseATM        = TRUE;
    s->isAllocated    = FALSE;
    s->sSrcPattern    = ssrcPattern;
    s->sSrcMask       = ssrcMask;
    s->sequenceNumber = (UINT16)
#ifdef _MTS_
      rand_r(&seed);
#else
    rand();
#endif
    s->socket = liatmCreateSocket((Direction==ATMLISTEN),atmSelector,port);

    rtpRegenSSRC((HRTPSESSION)s);
    liBlock(s->socket);

    return (HRTPSESSION)s;
}


RVAPI
HRTPSESSION RVCALLCONV rtpatmOpenEx(
        IN  UINT8               atmSelector,
        IN  UINT16              port,
        IN  RTPATMDIRECTION     Direction,
        IN  UINT32              ssrcPattern,
        IN  UINT32              ssrcMask,
        IN  char *              cname)
{
    rtpSession *s  = calloc(rtpGetAllocationSize(),1);

    if (s==NULL)
        return NULL;

    if (rtpatmOpenFrom(atmSelector, port, Direction, ssrcPattern, ssrcMask, (void*)s, rtpGetAllocationSize())==NULL)
    {
        free(s);
        return NULL;
    }
    s->isAllocated=TRUE;

    if (cname)
        s->hRTCP = rtcpOpen(s->sSrc, ++port, cname);

    return (HRTPSESSION) s;
}


RVAPI
HRTPSESSION RVCALLCONV rtpatmOpen(
        IN  UINT8               atmSelector,
        IN  UINT16              port,
        IN  RTPATMDIRECTION     Direction,
        IN  UINT32              ssrcPattern,
        IN  UINT32              ssrcMask)
{
    return rtpatmOpenEx(atmSelector, port, Direction, ssrcPattern, ssrcMask, NULL);
}


RVAPI
UINT32 RVCALLCONV rtpatmClose(IN  HRTPSESSION  hRTP)
{
    rtpSession * s     = (rtpSession *)hRTP;
    UINT8        bzero = 0;

    if (s->hRTCP)
        rtcpClose(s->hRTCP);

    liatmCallOn       (s->socket, 0, NULL, NULL);
    liatmSendData     (s->socket, &bzero, 1);
    liatmDeleteSocket (s->socket);

    if (s->isAllocated)
        free(s);

    return 0;
}

RVAPI
void RVCALLCONV rtpatmSetEventHandler(
        IN  HRTPSESSION        hRTP,
        IN  LPRTPEVENTHANDLER  eventHandler,
        IN  void *             context)
{
    if (hRTP)
    {
        ATMSOCKET socket = ((rtpSession *)hRTP)->socket;

        ((rtpSession *)hRTP)->eventHandler  = eventHandler;
        ((rtpSession *)hRTP)->context       = context;

        liUnblock   (((rtpSession*)hRTP)->socket);
        liatmCallOn (((rtpSession*)hRTP)->socket, liEvRead, (eventHandler) ? rtpEvent:NULL, (void*)hRTP);
    }
}


RVAPI
void RVCALLCONV rtpSetATMRemoteAddress(
    IN HRTPSESSION  hRTP,   /* RTP Session Opaque Handle */
    IN UINT8    *atmAddr,
    IN UINT32   addrLength,
    IN UINT16   selector)
{
    rtpSession *s = (rtpSession *)hRTP;

    if (atmAddr)
    {
        s->atmAddr     = atmAddr;
        s->addrLength  = addrLength;
    }

    if (selector)
        s->selector = selector;
    else
        s->selector = s->atmAddr[s->addrLength-1];
}


RVAPI
INT32 RVCALLCONV rtpatmConnect(IN HRTPSESSION hRTP)
{
    rtpSession * s = (rtpSession*) hRTP;
    int          res;

    res = liatmConnect(s->socket, s->atmAddr, s->addrLength,TRUE,0,(int*)&s->selector);

    /*if (res != RVERROR)
        liatmCallOn (s->socket, liEvRead, (s->eventHandler) ? rtpEvent:NULL, (void*)hRTP);*/

    return res;
}

RVAPI
INT32 RVCALLCONV rtpatmWrite(
        IN  HRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len,
        IN  rtpParam *   p)
{
    int     retVal;
    UINT32 *header;

    rtpSession *s = (rtpSession *)hRTP;

    if (s->socket==INVALID_SOCKET)
        return RVERROR;

    header = (UINT32*)((char*)buf+p->sByte-12);

    if (s->useSequenceNumber)
        s->sequenceNumber=p->sequenceNumber;
    p->sequenceNumber=s->sequenceNumber;

    header[0]=0;
    header[0]=bitfieldSet(header[0],2,30,2);
    header[0]=bitfieldSet(header[0],p->marker,23,1);
    header[0]=bitfieldSet(header[0],p->payload,16,7);
    header[0]=bitfieldSet(header[0],s->sequenceNumber++,0,16);
    header[1]=p->timestamp;
    header[2]=s->sSrc;
    p->len=len-p->sByte;

    liConvertHeader2l((UINT8*)header,0,3);
    retVal = liatmSendData(s->socket,(UINT8*)header,len-((char*)header-(char*)buf));

    if (s->hRTCP  &&  retVal >= 0)
    {
        rtcpRTPPacketSent(s->hRTCP, p->len, p->timestamp);
    }

    return retVal;
}


RVAPI
INT32 RVCALLCONV rtpatmWriteFirst(
        IN  HRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len)
{
    int retVal;
    rtpParam p;
    rtpSession *s;
    UINT32 *header;

    memset (&p,0,sizeof(rtpParam));
    p.timestamp = 0;
    p.marker    = TRUE;
    p.payload   = 0; /*PCMU;
    OUT     UINT32  sSrc;
    OUT     UINT16  sequenceNumber;*/
    p.sByte     = 12;
    /*OUT     int     len;*/

    s = (rtpSession *)hRTP;

    header=(UINT32*)((char*)buf+p.sByte-12);

    if (s->useSequenceNumber)
        s->sequenceNumber=p.sequenceNumber;
    p.sequenceNumber=s->sequenceNumber;

    header[0]=0;
    header[0]=bitfieldSet(header[0],2,30,2);
    header[0]=bitfieldSet(header[0],p.marker,23,1);
    header[0]=bitfieldSet(header[0],p.payload,16,7);
    header[0]=bitfieldSet(header[0],s->sequenceNumber++,0,16);
    header[1]=p.timestamp;
    header[2]=s->sSrc;
    p.len=len-p.sByte;

    liConvertHeader2l((UINT8*)header,0,3);

    if (retVal=liatmSendData(s->socket, (UINT8*)header, len-((char*)header-(char*)buf))<0)
        retVal=0;

    if (s->hRTCP  &&  retVal >= 0)
    {
        rtcpRTPPacketSent(s->hRTCP, p.len, p.timestamp);
    }

    return retVal;
}

RVAPI
INT32 RVCALLCONV rtpatmRead(
        IN  HRTPSESSION  hRTP,
        IN  void        *buf,
        IN  INT32        len,
        OUT rtpParam    *p)
{
    rtpSession * s      = (rtpSession *)hRTP;
    UINT32     * header = buf;


    if (s->socket==INVALID_SOCKET)
        return RVERROR;

    p->len = liatmRecvData(s->socket,(char*)buf,len);

    liConvertHeader2h(buf,0,3);

    /*if (isMyIP(ip) && s->sSrc==((UINT32*)buf)[2]) return RVERROR;*/
    if (p->len==RVERROR || p->len<12)
        return RVERROR;

    liConvertHeader2h(buf,3,bitfieldGet(header[0],24,4));

    p->timestamp        = header[1];
    p->sequenceNumber   = (UINT16)bitfieldGet(header[0],0,16);
    p->sSrc             = header[2];
    p->marker           = bitfieldGet(header[0],23,1);
    p->payload          = (unsigned char)bitfieldGet(header[0],16,7);
    p->sByte            = 12 + bitfieldGet(header[0],24,4) * sizeof(UINT32);

    if (bitfieldGet(header[0],28,1))
    {
        int xStart = p->sByte/sizeof(UINT32);

        liConvertHeader2h(buf,xStart,1);
        liConvertHeader2h(buf,xStart+1,bitfieldGet(header[xStart],0,16));
        p->sByte += bitfieldGet(header[xStart],0,16)*sizeof(UINT32);
    }

    return 0;
}


RVAPI
INT32 RVCALLCONV rtpatmReadEx(
        IN  HRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len,
        IN  UINT32       timestamp,
        OUT rtpParam *   p)
{
    rtpSession *s = (rtpSession *)hRTP;
    int retVal;

    retVal = rtpatmRead(hRTP, buf, len, p);

    if (s->hRTCP  &&  retVal >= 0)
    {
        rtcpRTPPacketRecv(s->hRTCP, p->sSrc, timestamp,  p->timestamp, p->sequenceNumber);
    }

    return retVal;
}


RVAPI
INT32 rtpatmGetATMAddr(IN HRTPSESSION  hRTP, BYTE Addr[ATM_ADDR_SIZE], UINT32 *NumofDigits)
{
    return liatmGetLocalAddr(((rtpSession*)hRTP)->socket,Addr,NumofDigits);
}

RVAPI
BOOL RVCALLCONV rtpatmIsATM(IN HRTPSESSION hRTP)
{
    return ((rtpSession*)hRTP)->bUseATM;
}

                 /* == ENDS: RTP ATM Functions == */
#endif



#ifdef __cplusplus
}
#endif



