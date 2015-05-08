/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/
#ifdef __cplusplus
extern "C" {
#endif


#ifndef __RTCP_H
#define __RTCP_H


#include <rvcommon.h>

#ifndef __RTP_H
DECLARE_OPAQUE(HRTCPSESSION);
#define RVVXDAPI RVAPI
#define VXDCALLCONV RVCALLCONV
#endif


#define ERR_RTCP_GENERALERROR     -350   /* general error */
#define ERR_RTCP_SSRCCOLLISION    -351   /* ssrc collision */
#define ERR_RTCP_ILLEGALSSRC      -352   /* an illegal ssrc was specified */
#define ERR_RTCP_ILLEGALSDES      -353   /* an illegal sdes was specified */
#define ERR_RTCP_ILLEGALPACKET    -354   /* illegal RTCP pkt encountered */


/*h.e 30.04.01*/
typedef void (RVCALLCONV *LPRTCPEVENTHANDLER)(
        IN HRTCPSESSION,
        IN void * context,
        IN UINT32 ssrc);
/*===*/

typedef BOOL (RVCALLCONV *LPSSRCENUM)(
        IN  HRTCPSESSION  hRTCP,
        IN  UINT32        ssrc);


typedef struct {
    BOOL    valid;
    UINT32  mNTPtimestamp;
    UINT32  lNTPtimestamp;
    UINT32  timestamp;
    UINT32  packets;
    UINT32  octets;
} RTCPSRINFO;

typedef struct {
    BOOL    valid;
    UINT32  fractionLost;
    UINT32  cumulativeLost;
    UINT32  sequenceNumber;
    UINT32  jitter;
    UINT32  lSR;
    UINT32  dlSR;

} RTCPRRINFO;

typedef struct {
    BOOL        selfNode;
    RTCPSRINFO  sr;
    RTCPRRINFO  rrFrom;
    RTCPRRINFO  rrTo;
    char        cname[255];
} RTCPINFO;



                      /* == Basic RTCP Functions == */


RVAPI
INT32 RVCALLCONV rtcpInit(void);


RVAPI
INT32 RVCALLCONV rtcpInitEx(UINT32);

RVAPI
INT32 RVCALLCONV rtcpEnd(void);


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
int RVCALLCONV rtcpSetLocalAddress(IN UINT32 ip);


RVAPI
INT32 RVCALLCONV rtcpSetRTCPRecvEventHandler(
    IN HRTCPSESSION         hRTCP,
    IN LPRTCPEVENTHANDLER   rtcpCallback,
    IN void *               context);


RVAPI
int RVCALLCONV rtcpGetAllocationSize(
    IN  int sessionMembers);

/************************************************************************************
 * rtcpOpenFrom
 * description: Opens an RTCP session in the memory that the application allocated.
 * input: ssrc        - Synchronization source value for the RTCP session.
 *        port        - The UDP port number to be used for the RTCP session.
 *        cname       - CNAME to use
 *        maxSessionMembers - Maximum number of different SSRC that can be handled
 *        buffer      - Application allocated buffer with a value no less than the
 *                      value returned by the function rtpGetAllocationSize().
 *        bufferSize  - size of the buffer.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the opened RTP
 *               session. Otherwise, it returns NULL.
 ***********************************************************************************/
RVAPI
HRTCPSESSION RVCALLCONV rtcpOpenFrom(
    IN  UINT32  ssrc,
    IN  UINT16  port,
    IN  char *  cname,
    IN  int     maxSessionMembers,
    IN  void *  buffer,
    IN  int     bufferSize);

RVAPI
HRTCPSESSION RVCALLCONV rtcpOpen(
        IN  UINT32  ssrc,
        IN  UINT16  port,
        IN  char *  cname);

RVAPI
INT32 RVCALLCONV rtcpClose(
        IN  HRTCPSESSION  hRCTP);

RVAPI
void RVCALLCONV rtcpSetRemoteAddress(
        IN  HRTCPSESSION  hRTCP,
        IN  UINT32        ip,
        IN  UINT16        port);

RVAPI
INT32 RVCALLCONV rtcpStop(
        IN  HRTCPSESSION  hRTCP);


RVAPI
INT32 RVCALLCONV rtcpRTPPacketRecv(
        IN  HRTCPSESSION  hRTCP,
        IN  UINT32        ssrc,
        IN  UINT32        localTimestamp,
        IN  UINT32        myTimestamp,
        IN  UINT16        sequence);

RVAPI
INT32 RVCALLCONV rtcpRTPPacketSent(
        IN  HRTCPSESSION  hRTCP,
        IN  INT32         bytes,
        IN  UINT32        timestamp);

RVAPI
UINT16 RVCALLCONV rtcpGetPort(
        IN  HRTCPSESSION  hRCTP);


                   /* == ENDS: Basic RTCP Functions == */



                    /* == Accessory RTCP Functions == */


RVAPI
BOOL RVCALLCONV rtcpCheckSSRCCollision(
        IN  HRTCPSESSION  hRTCP);

/* ouch! */
#define rtcpEnumParticipients  rtcpEnumParticipants

RVAPI
BOOL RVCALLCONV rtcpEnumParticipants(
        IN  HRTCPSESSION  hRTCP,
        IN  LPSSRCENUM    enumerator);

RVAPI
INT32 RVCALLCONV rtcpGetSourceInfo(
        IN   HRTCPSESSION hRTCP,
        IN   UINT32       ssrc,
        OUT  RTCPINFO*    info);

RVAPI
INT32 RVCALLCONV rtcpSetGroupAddress(
        IN  HRTCPSESSION  hRTCP,
        IN  UINT32        ip);

RVAPI
INT32 RVCALLCONV rtcpGetSSRC(
        IN  HRTCPSESSION  hRTCP);

RVAPI
INT32 RVCALLCONV rtcpSetSSRC(
        IN  HRTCPSESSION  hRTCP,
        IN  UINT32        ssrc);

                 /* == ENDS: Accessory RTCP Functions == */

RVAPI
INT32 RVCALLCONV rtcpGetEnumNext(
                IN  HRTCPSESSION  hRTCP,
                IN  INT32         prev,
                IN  INT32 *       ssrc);


RVAPI
INT32 RVCALLCONV rtcpGetEnumFirst(
                IN  HRTCPSESSION  hRTCP,
                IN  INT32 *       ssrc);


#endif  /* __RTCP_H */
#ifdef __cplusplus
}
#endif
