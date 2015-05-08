#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef CMHOOK_H
#define CMHOOK_H


typedef BOOL
    (RVCALLCONV *cmHookListenT)(/*Before listen*/
        IN      HPROTCONN           hConn,
        IN      int                 addr
        );


typedef int
    (RVCALLCONV *cmHookListeningT)(/*After listen*/
        IN      HPROTCONN           hConn,
        IN      int                 addr,
        IN      BOOL                error
        );

typedef int
    (RVCALLCONV*cmHookConnectingT)(/*Before connect*/
        IN      HPROTCONN           hConn,
        IN      int                 addr
        );

typedef int
    (RVCALLCONV*cmHookInConnT)(/*Incomming connect*/
        IN      HPROTCONN           hConn,
        IN      int                 addrFrom,
        IN      int                 addrTo
        );
typedef int
    (RVCALLCONV*cmHookOutConnT)(/*Outgoing connect*/
        IN      HPROTCONN           hConn,
        IN      int                 addrFrom,
        IN      int                 addrTo,
        IN      BOOL                error
        );
typedef BOOL
    (RVCALLCONV*cmHookSendT)(
        IN      HPROTCONN           hConn,
        IN      int                 nodeId,
        IN      BOOL                error
        );
typedef BOOL
    (RVCALLCONV*cmHookRecvT)(
        IN      HPROTCONN           hConn,
        IN      int                 nodeId,
        IN      BOOL                error
        );

typedef BOOL
    (RVCALLCONV*cmHookSendToT)(
        IN      HPROTCONN           hConn,
        IN      int                 nodeId,
        IN      int                 addrTo,
        IN      BOOL                error
        );
typedef BOOL
    (RVCALLCONV*cmHookRecvFromT)(
        IN      HPROTCONN           hConn,
        IN      int                 nodeId,
        IN      int                 addrFrom,
        IN      BOOL                multicast,
        IN      BOOL                error
        );

typedef void
    (RVCALLCONV*cmHookCloseT)(
        IN      HPROTCONN           hConn
        );


typedef  struct
{
   cmHookListenT      hookListen;
   cmHookListeningT   hookListening;
   cmHookConnectingT  hookConnecting;
   cmHookInConnT      hookInConn;
   cmHookOutConnT     hookOutConn;
   cmHookSendT        hookSend;
   cmHookRecvT        hookRecv;
   cmHookSendToT      hookSendTo;
   cmHookRecvFromT    hookRecvFrom;
   cmHookCloseT       hookClose;
} SCMPROTOCOLEVENT,*CMPROTOCOLEVENT;


/************************************************************************
 * cmSetProtocolEventHandler
 * purpose: Installs the protocol event handler.
 * input  : hApp            - Stack handle for the application
 *          cmProtocolEvent - Pointer to array of pointers to general event callback functions
 *          size            - Size of *cmProtocolEvent in bytes
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmSetProtocolEventHandler(
        IN  HAPP            hApp,
        IN  CMPROTOCOLEVENT cmProtocolEvent,
        IN  int             size);

RVAPI
int RVCALLCONV cmSetProtocolEventHandlerReplacement(
        IN  HAPP        hApp,
        IN  CMPROTOCOLEVENT cmProtocolEventReplacement,
        IN  int     size);


RVAPI
int RVCALLCONV cmProtocolSendMessage(
        IN  HAPP        hApp,
                IN  HPROTCONN       hConn,
        IN  int             msg);

RVAPI
int RVCALLCONV cmProtocolSendMessageTo(
        IN  HAPP        hApp,
                IN  HPROTCONN       hConn,
        IN  int             msg,
                IN  int             addr);

typedef enum {
    cmProtocolUnknown,
    cmProtocolRAS,
    cmProtocolQ931,
    cmProtocolH245
} cmProtocol;


/* Creates new message value tree*/
RVAPI int /* New message nodeId*/
RVCALLCONV cmProtocolCreateMessage(
                IN      HAPP                    hApp,
                IN      cmProtocol              protocol);

RVAPI/* returns protocol of the message tree*/
cmProtocol RVCALLCONV cmProtocolGetProtocol(
                IN      HAPP                    hApp,
                IN      int                     msg);/* message nodeId*/


RVAPI
char* RVCALLCONV cmProtocolGetProtocolName(
                IN      cmProtocol              protocol);


#endif
#ifdef __cplusplus
}
#endif



