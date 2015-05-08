/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __RTP_H
#define __RTP_H


#define RVVXDAPI RVAPI
#define VXDCALLCONV RVCALLCONV


#include <rvcommon.h>
#ifdef _ATM_
#include <rvws2atm.h>
#endif

DECLARE_OPAQUE(HRTPSESSION);

#ifndef __RTCP_H
DECLARE_OPAQUE(HRTCPSESSION);
#endif


typedef struct {
    IN OUT  UINT32  timestamp;
    IN OUT  BOOL    marker;
    IN OUT  BYTE    payload;

    OUT     UINT32  sSrc;
    OUT     UINT16  sequenceNumber;
    OUT     int     sByte;
    OUT     int     len;
} rtpParam;


typedef void (*LPRTPEVENTHANDLER)
    (
        IN  HRTPSESSION  hRTP,
        IN  void *       context
    );



typedef struct
{
    int            isAllocated;
    int                socket;
    UINT32             sSrc;
    UINT32             sSrcMask;
    UINT32             sSrcPattern;
    LPRTPEVENTHANDLER  eventHandler;
    void *             context;
    UINT16             sequenceNumber;
    UINT32             ip;
    UINT16             port;
    BOOL               useSequenceNumber;
    HRTCPSESSION       hRTCP;
#ifdef _ATM_
    ATM_ADDRESS        HostAddr;
    BOOL               bUseATM;
#endif
} rtpSession;       /* HRTPSESSION */

                       /* == Basic RTP Functions == */


RVAPI
INT32 RVCALLCONV rtpInit(void);


RVAPI
INT32 RVCALLCONV rtpInitEx(UINT32);

RVAPI
void RVCALLCONV rtpEnd(void);


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
int RVCALLCONV rtpSetLocalAddress(IN UINT32 ip);


RVAPI
int RVCALLCONV rtpGetAllocationSize(void);

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
    IN  int     bufferSize);

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
        IN  UINT32  ssrcMask);

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
        IN  char *  cname);


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
        IN  HRTPSESSION  hRTP);

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
        IN  HRTPSESSION  hRTP);

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
        IN  void *             context);

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
        IN  HRTPSESSION  hRTP,
        IN  UINT32       ip,
        IN  UINT16       port);

/************************************************************************************
 * rtpWrite
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
INT32 RVCALLCONV rtpWrite(
        IN  HRTPSESSION  hRTP,
        IN  void  *      buf,
        IN  INT32        len,
        IN  rtpParam *   p);


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
        IN  rtpParam *   p);

RVAPI
INT32 RVCALLCONV rtpUnpack(
        IN  HRTPSESSION  hRTP,
        IN  void *buf,
        IN  INT32 len,
        OUT rtpParam* p);


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
        IN   HRTPSESSION  hRTP,
        IN   void *       buf,
        IN   INT32        len,
        OUT  rtpParam *   p);

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
        IN   HRTPSESSION  hRTP,
        IN   void *       buf,
        IN   INT32        len,
        IN   UINT32       timestamp,
        OUT  rtpParam *   p);

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
        IN  HRTPSESSION  hRTP);

/************************************************************************************
 * rtpGetVersion
 * description:  Returns the RTP version of the installed RTP Stack.
 * input:  none.
 * output: none.
 * return value: If no error occurs, the function returns the current version value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
char * RVCALLCONV rtpGetVersion(void);

/************************************************************************************
 * rtpGetVersionNum
 * description:  Returns the RTP version of the installed RTP Stack.
 * input:  none.
 * output: none.
 * return value: If no error occurs, the function returns the current version value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
UINT32 RVCALLCONV rtpGetVersionNum(void);


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
        IN  HRTPSESSION  hRTP);


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
        IN  HRTCPSESSION  hRTCP);

/************************************************************************************
 * rtpGetHeaderLength
 * description:  return the header of RTP message.
 * input:  none.
 * output: none.
 * return value:The return value is twelve.
 ***********************************************************************************/
RVAPI
INT32 RVCALLCONV rtpGetHeaderLength(void);

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
        IN  HRTPSESSION  hRTP);

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
        IN  HRTPSESSION  hRTP,
        IN  UINT32       ip);

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
        IN  HRTPSESSION  hRTP);

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
        IN  HRTPSESSION  hRTP);

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
        IN int size);

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
                IN int size);


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
                IN int size);


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
                IN HRTPSESSION  hRTP);

               /* == ENDS: Accessory RTP Functions == */




#endif  /* __RTP_H */
#ifdef __cplusplus
}
#endif
