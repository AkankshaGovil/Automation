#ifndef TPKT_H
#define TPKT_H

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
#include <rvinternal.h>
#include <li.h>
#include <mei.h>
#include <rpool.h>

DECLARE_OPAQUE(HTPKT);
DECLARE_OPAQUE(HTPKTCTRL);

typedef enum {tpktClient,tpktServer,tpktMultiServer} tpktTypes;
typedef void (*LPTPKTEVENTHANDLER)(HTPKT tpkt,liEvents event,int length,int error,void*context);

/*******************************************************************************************
 * tpktInit
 *
 * Purpose: Initialize the structures of the tpkt module
 *
 * Input:   sessions - Maximal number of allowed connections
 *          logMgr   - Log manager to use
 * Return Value: Handle to the list of tpktInfo elements
 *******************************************************************************************/
HTPKTCTRL tpktInit(int sessions, RVHLOGMGR logMgr);

/*******************************************************************************************
 * tpktEnd
 *
 * Purpose: Destroy the tpktInfo elements and close all connections
 *
 * Input:   hCtrl - Handle to the list of tpktInfo elements
 *
 * Return Value: 0
 *******************************************************************************************/
int tpktEnd(HTPKTCTRL hCtrl);

/*******************************************************************************************
 * tpktOpen
 *
 * Purpose: Allocate a socket and start either a connect process in case of a client which
 *      supplied a full address, or a listenning process in case of server.
 *      For a client that didn't supply an address yet, just create the socket.
 *
 * Input:   hTpktCtrl - Handle to the list of tpktInfo elements
 *      localIP   - ip address to use as local address
 *      localPort - port to use as local port
 *      type      - What type of connection we request:
 *          tpktClient - We want to connect (if address is supplied)
 *          tpktServer - we want to listen and then accept one connection
 *          tpktMultiServer - We want to listen, accept and remain listenning
 *      eventHandler - Callback routine to be called on any event on the connection
 *      context  - Additional data to be supplied on the callback
 *
 *
 * Return Value: A handle to the tpkt object created
 *******************************************************************************************/
HTPKT tpktOpen(HTPKTCTRL hTpktCtrl, UINT32 localIP, UINT16 localPort, tpktTypes type, LPTPKTEVENTHANDLER eventHandler,void*context);

/*******************************************************************************************
 * tpktConnect
 *
 * Purpose: This routine supplements the tpktOpen routine, for clients which didn't supply
 *			an address and now wish to do the actual connect operation on the already allocated
 *			socket.
 *
 * Input:   hTpkt     - Handle to the tpktInfo element
 *			ip        - ip address to connect to
 *			port      - port to connect to
 *
 * Return Value: 0
 *******************************************************************************************/
int tpktConnect(HTPKT hTpkt, UINT32 ip, UINT16 port );

/*******************************************************************************************
 * tpktClose
 *
 * Purpose: Releases a tpkt object and frees all its resources
 *
 * Input:   hTpkt - Handle to the tpktInfo element
 *
 * Return Value: TRUE  - all is ok
 *       RVERROR - an error occured
 *******************************************************************************************/
int tpktClose(HTPKT hTpkt);

/*******************************************************************************************
 * tpktSendFromRpool
 *
 * Purpose: This routine sends a given rpool message over a connection, assuming that the message
 *      already contains a tpkt header.
 *
 * Input:   hTpkt           - Handle to the tpktInfo element
 *          hRpool          - handle to the rpool.
 *          message         - points to the message to be sent.
 *          offset          - where in the message to start the sending.
 *          rpoolLock       - The lock of the rpool
 *
 * Return Value: 0 or RVERROR
 *******************************************************************************************/
int tpktSendFromRpool(HTPKT hTpkt,
                      HRPOOL hRpool,
                      void *message,
                      int offset,
                      HMEI rpoolLock);

/*******************************************************************************************
 * tpktRecvNotify
 *
 * Purpose: to set a CallBack routine to be called when data arrives.
 *      This routine is called on EvAccept since it's fast.
 *
 * Input:   hTpkt - A handle to the TPKT module
 *      eventHandler - The callback to be read on receiving data
 *      context - To be returned with the callback (may be appl handle etc.)
 * Return Value: -1 : on Error;
 *        0 : OK
 *******************************************************************************************/
int tpktRecvNotify(HTPKT hTpkt,LPTPKTEVENTHANDLER eventHandler,void*context);

/*******************************************************************************************
 * tpktRecvIntoRpool
 *
 * Purpose: to initiate the reading of a new message while seting a new CallBack routine
 *          to be called when data arrives.
 *          The routine handles any problems that arise from the fact that an rpool message
 *          may be not contiguous.
 *          This routine is called on each EvRead event, but NOT on EvAccept since it's
 *          too slow.
 *
 * Input:   hTpkt - A handle to the TPKT module
 *      buffer - The buffer into which to read the received data
 *      length - Maximal size of the buffer
 *
 * Return Value: -1 : on Error;
 *        0 : on not receiving a full buffer or a full message;
 *       >0 : Number of bytes read (may be either the length of the bufer
 *            in case of an incomplete message or the message length).
 *******************************************************************************************/
int tpktRecvIntoRpool(HTPKT hTpkt,
                      HRPOOL hRpool,
                      void *message,
                      HMEI rpoolLock);

/*******************************************************************************************
 * tpktGetSock
 *
 * Purpose: return the socket associated with the given tpkt object.
 *
 * Input:   hTpkt - A handle to the TPKT module
 *
 * Return Value: The number of the socket
 *******************************************************************************************/
int tpktGetSock(HTPKT hTpkt);

#ifdef __cplusplus
}
#endif

#endif



