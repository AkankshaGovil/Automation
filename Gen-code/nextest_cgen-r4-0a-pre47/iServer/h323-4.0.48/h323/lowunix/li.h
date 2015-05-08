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
#ifndef LI_H
#define LI_H

#include <tls.h>


typedef enum
{
    LI_UDP,
    LI_TCP,
    LI_XTP
}   liProtocol_t;

typedef struct
{
    UINT32 length;
    BYTE* buffer;
}   liBuffer;

#define LI_ADDR_ANY 0
#define LI_ADDR_NONE 0xfffffffful
#define LI_PORT_ANY 0

typedef enum
{
    liEvRead=0x01,
    liEvWrite=0x02,
    liEvAccept=0x08,
    liEvConnect=0x10,
    liEvClose=0x20
} liEvents;

typedef void(*LPLIEVENTHANDLER)(int socketId,liEvents event,int error,void* context);

typedef void(*LPLIEXTEVENTHANDLER)(int socketId,int error,void* context);


int  liInit (void);
int  liEnd  (void);
void
liSetPortsRange(IN UINT16 from,IN UINT16 to);
int  liOpen (UINT32 ipAddr,UINT16 port,liProtocol_t protocol);
int  liConnect (int handle,UINT32 ipAddr,UINT16 port);
int  liListen (int handle,int queueLen);
int  liAccept (int mHandle);
int  liClose (int handle);
int  liShutdown(int handle);
int  liUdpRecv (int handle, UINT8 *buff,int len,UINT32*ipAddr,UINT16*port);
int  liUdpSend (int handle, UINT8 *buff,int len,UINT32 ipAddr,UINT16 port);
int  liTcpRecv (int handle, UINT8 *buff,int len);
int  liTcpSend (int handle, UINT8 *buff,int len);
int  liUdpRecvExt(int handle,liBuffer*buff,int buffers, UINT32*ipAddr,UINT16*port,
        LPLIEXTEVENTHANDLER eventHandler, void* context);
int  liUdpSendExt(int handle,liBuffer*buff,int buffers, UINT32 ipAddr,UINT16 port,
        LPLIEXTEVENTHANDLER eventHandler, void* context);
int  liConvertHeader2l (UINT8 *buff,int startIndex,int size);
int  liConvertHeader2h (UINT8 *buff,int startIndex,int size);
UINT32  liConvertIp(char *ipAddr);
char *liIpToString(UINT32 ipAddr, char* buf);
int  liBlock (int sockId);
int  liUnblock (int sockId);
UINT32  liGetSockIP (int socketId);
UINT16  liGetSockPort (int socketId);
UINT32  liGetRemoteIP (int socketId);
UINT16  liGetRemotePort (int socketId);
int     liBytesAvailable (int sockId,int *bytesAvailable);

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
    IN RvH323ThreadHandle   threadId);

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
    IN int                  socket,
    IN liEvents             lEvent,
    IN LPLIEVENTHANDLER     eventHandler,
    IN void*                context,
    IN RvH323ThreadHandle   threadId);

int     liCallOn(int socketId,liEvents lEvent,LPLIEVENTHANDLER eventHandler,void* context);

int     liSetMulticastInterface(int socketId,UINT32 ipaddr);
int     liSetMulticastTTL(int socketId,int ttl);
int     liJoinMulticastGroup(int socketId,UINT32 mcastip,UINT32 ipaddr);
int     liLeaveMulticastGroup(int socketId,UINT32 mcastip,UINT32 ipaddr);
UINT32** liGetHostAddrs(void);
char*   liGetHostName(void);
int    liSetTcpNoDelay(int socketId, int value);
int
liSetSocketBuffers(
           /* Set the size of send and receive buffers for socket. */
           int socket,
           int sendSize, /* Negative size does not effect current size */
           int recvSize  /* Negative size does not effect current size */
           );


/************************************************************************
 * liThreadAttach
 * purpose: Indicate that a thread can catch network events.
 * input  : threadId        - Thread using a message loop
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int liThreadAttach(IN RvH323ThreadHandle    threadId);

/************************************************************************
 * liThreadDetach
 * purpose: Indicate that a thread cannot catch network events anymore.
 *          Should be called after calling liThreadAttach()
 * input  : threadId        - Thread using a message loop
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int liThreadDetach(IN RvH323ThreadHandle    threadId);


#endif

#ifdef __cplusplus
}
#endif



