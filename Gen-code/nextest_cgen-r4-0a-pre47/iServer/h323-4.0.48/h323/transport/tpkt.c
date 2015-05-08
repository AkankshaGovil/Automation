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

#include <stdio.h>
#include <rvinternal.h>
#include <transportint.h>
#include <li.h>
#include <ema.h>
#include <rpool.h>
#include <tpkt.h>
#include <Threads_API.h>
#include <tls.h>

#ifndef _NUCLEUS
static char tpktDummyBuff[512];
#endif

typedef struct
{
    RvH323ThreadHandle  thisThread;
    RVHLOG              log;
} tpktGlobals;

typedef struct
{
    LPTPKTEVENTHANDLER  eventHandler;               /* Callback routine on transaction event */
    UINT8               header[TPKT_HEADER_SIZE];   /* TPKT header */
    void*               context;                    /* additional data to be sent on the callback */
    int                 headerCount;                /* an index into the buffer indicationg how much
                                                       of the header was processsed */
    HRPOOL              hRpool;                     /* the rpool where the message is */
    void                *message;                   /* the message to be sent/read */
    int                 offset;                     /* the offset into the message */
    HMEI                rpoolLock;                  /* the lock to protect the rpool */
} tpktTrans;


typedef struct
{
    int         socket;  /* The socket of the connection */
    HEMA        hEma;    /* an EMA pool of all active tpktInfo elements (connections) */
    liEvents    event;   /* events bit map of the events on the connection */
    tpktTypes   type;    /* The type of connection (MultiServer, Server or Client */
    tpktTrans   recv;    /* The receiving transaction data */
    tpktTrans   send;    /* The sending transaction data */
    BOOL        isConnected; /* connection is established */
    BOOL        close;   /* Indicates if the connection is shutting down now (tpktClose was called) */
} tpktInfo;


#define liTcpSendIfConnected(tpkt, finished) \
                                 (((tpkt)->isConnected)?(tpktLiTcpSend(tpkt, (finished))):0)

#define liTcpRecvDirectIfConnected(tpkt, buf, size) \
                                 (((tpkt)->isConnected)?(liTcpRecv((tpkt)->socket, (buf), (size))):0)

#define liTcpRecvIfConnected(tpkt, finished) \
                                 (((tpkt)->isConnected)?(tpktLiTcpRecv(tpkt, (finished))):0)


/*******************************************************************************************
 * tpktLiTcpSend
 *
 * Purpose: internal routine that simulates liTcpSend using rpool buffer rather than
 *          a contiguous one, handling the problems arising that.
 *
 * Input:   tpkt - The object of the send, having all the necessary data abour rpool
 *
 * Output: finished - whether the send was finished already or not
 *
 * Return Value: number of bytes sent.
 *******************************************************************************************/
int tpktLiTcpSend(tpktInfo* tpkt, BOOL *finished)
{
    int sent;
    int len;
    BYTE *buffer;
    int totalLength;
    int totalSent = 0;

    /* lock around operation with the rpool */
    meiEnter(tpkt->send.rpoolLock);

    /* get the total length of the message */
    totalLength = rpoolChunkSize(tpkt->send.hRpool, tpkt->send.message);

    /* send contiguous segment at a time until all the message is sent or TCP is exhausted */
    do {
        /* get the current segment pointer and length */
        len = rpoolGetPtr(tpkt->send.hRpool,
                          tpkt->send.message,
                          tpkt->send.offset,
                          (void **)&buffer,
                          totalLength);

        /* do the actual send */
        sent = liTcpSend(tpkt->socket, buffer, len);

        /* update the pointer into the message and the current call counter */
        if (sent >= 0)
        {
            tpkt->send.offset += sent;
            totalSent += sent;
        }
        else
        {
            /* Error: we couldn't write even one segment */
            *finished = FALSE;
            /* release rpool */
            meiExit(tpkt->send.rpoolLock);
            /* report the error */
            return sent;
        }

        /* if TCP was exhausted before all we wanted was sent */
        if (sent < len)
        {
            /* we couldn't write even one segment, wait for event write */
            *finished = FALSE;
            /* release rpool */
            meiExit(tpkt->send.rpoolLock);
            /* report how many bytes were read in this call */
            return totalSent;
        }

    } while (tpkt->send.offset < totalLength);

    /* if we're here then all the message was sent */
    *finished = TRUE;
    meiExit(tpkt->send.rpoolLock);
    /* report that the whole message was sent */
    return totalSent;
}

/*******************************************************************************************
 * tpktLiTcpRecv
 *
 * Purpose: internal routine that simulates liTcpRecv using rpool buffer rather than
 *          a contiguous one, handling the problems arising from that.
 *
 * Input:   tpkt - The object of the receive, having all the necessary data abour rpool
 *
 * Output: finished - whether the receive was finished already or not
 *
 * Return Value: number of bytes sent.
 *******************************************************************************************/
int tpktLiTcpRecv(tpktInfo* tpkt, BOOL *finished)
{
    int received;
    int len;
    BYTE *buffer;
    int totalLength;
    int totalReceived = 0;

    /* lock around operation with the rpool */
    meiEnter(tpkt->recv.rpoolLock);

    /* get the total length of the currently allocated rpool message
       This should be exactly the size of the message as was received in the header */
    totalLength = rpoolChunkSize(tpkt->recv.hRpool, tpkt->recv.message);

    /* read one segment after the other until the whole message is
       read or TCP is empty for a while */
    do {
        /* get a pointer and length for the current segment */
        len = rpoolGetPtr(tpkt->recv.hRpool,
                          tpkt->recv.message,
                          tpkt->recv.offset,
                          (void **)&buffer,
                          totalLength);

        /* do the actual receive */
        received = liTcpRecv(tpkt->socket, buffer, len);

        /* update the offset into the rpool message and the current call counter */
        tpkt->recv.offset += received;
        totalReceived += received;

        /* if TCP was empty before we read all we wanted wait for next event */
        if (received < len)
        {
            /* we couldn't read even one segment, wait for event write */
            *finished = FALSE;
            /* release rpool */
            meiExit(tpkt->recv.rpoolLock);
            /* report how many bytes were read in this call */
            return totalReceived;
        }
    } while (tpkt->recv.offset < totalLength);

    /* here it means that the whole message was read */
    *finished = TRUE;
    meiExit(tpkt->recv.rpoolLock);
    return totalLength;
}

/*******************************************************************************************
 * tpktCloseElement
 *
 * Purpose: internal routine that closes one tpkt element. Used at shutdown to be called
 *          on all the remaining elements.
 *
 * Input:   elem - The tpkt element
 *          param - Dummy parameter.
 *
 *******************************************************************************************/
void *tpktCloseElement(EMAElement elem, void* param)
{
    tpktInfo *tpkt = (tpktInfo *)elem;

    if (param);

    if (tpkt)
    {
        tpkt->close = TRUE;
        /* Close the connection */
        tpktClose((HTPKT)tpkt);
    }
    return NULL;
}

/*******************************************************************************************
 * tpktInit
 *
 * Purpose: Initialize the structures of the tpkt module
 *
 * Input:   sessions - Maximal number of allowed connections
 *          logMgr   - Log manager to use
 * Return Value: Handle to the EMA of tpktInfo elements
 *******************************************************************************************/
HTPKTCTRL tpktInit(int sessions, RVHLOGMGR logMgr)
{
    HEMA     hEma;
    tpktGlobals *globals;

    liInit();

    /* allocate global area for tpkt package */
    globals = (tpktGlobals *)calloc(1, sizeof(tpktGlobals));

    if (!globals)
        return NULL;

    /* remember the thread id that created this instance */
    globals->thisThread = RvH323ThreadGetHandle();
    globals->log        = logRegister(logMgr,"TCP","Lower TPKT layer");

    /* Build the EMA pool of tpktInfo elements */
    hEma = emaConstruct(sizeof(tpktInfo),
                        sessions+1,
                        emaNormalLocks,
                        logMgr,
                        "TPKT elements",
                        0,
                        globals,
                        NULL);

    return (HTPKTCTRL)hEma;
}

/*******************************************************************************************
 * tpktEnd
 *
 * Purpose: Destroy the tpktInfo elements and close all connections
 *
 * Input:   hCtrl - Handle to the EMA of tpktInfo elements
 *
 * Return Value: 0
 *******************************************************************************************/
int tpktEnd(HTPKTCTRL hCtrl)
{
    tpktGlobals *globals;
    HEMA hEma = (HEMA)hCtrl;

    /* Go over all tpktInfo elements in the EMA pool and close them */
    emaDoAll(hEma, tpktCloseElement, NULL);

    /* release the global area of tpkt */
    globals = (tpktGlobals *)emaGetUserDataByInstance(hEma);

    if (globals)
        free(globals);

    /* destroy the EMA pool of tpktInfo elements */
    emaDestruct(hEma);

    liEnd();
    return 0;
}


/*******************************************************************************************
 * tpktEvent
 *
 * Purpose: Treats the events that may occure on a connection (Accept, Connect, Write, Read,
 *      close).
 *
 * Input:   socket  - The listenning socket (in case of accept event)
 *          event   - The type of the event
 *          error   - indicates whether an error occured in the li level
 *          context - The tpkt object
 *
 * Output:  None.
 *******************************************************************************************/
void tpktEvent(int socket,liEvents event,int error,void* context)
{
    tpktInfo* tpkt=(tpktInfo*)context;
    tpktGlobals* globals;

    if (tpkt == NULL) return;
    if (*(int*)tpkt==RVERROR) return;

    if (!emaLock((EMAElement)tpkt))
        return;

    globals = (tpktGlobals *)emaGetUserData((EMAElement)tpkt);
    if (globals == NULL)
    {
        emaUnlock((EMAElement)tpkt);
        return;
    }

    /* Treat the events on the given socket */
    switch(event)
    {
        /* accept was done in order to open and incoming connection request */
        case liEvAccept:
        {
            int newSocket;
            int numOfLocks;

            logPrint(globals->log, RV_DEBUG,
                    (globals->log, RV_DEBUG,
                    "tpktEvent(socket=%d, liEvAccept,error=%d,context=%x)",
                    socket, error, context));

             /* Notify application that there is incoming connection request */
            numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
            tpkt->send.eventHandler((HTPKT)tpkt,(liEvents)0,0,FALSE,tpkt->send.context);
            emaReturnFromCallback((EMAElement)tpkt, numOfLocks);

            /*if during the event handler tpkt was not released then accept socket */
            if (!emaWasDeleted((EMAElement)tpkt))
                /* Do the accept and get the connection socket */
                newSocket=liAccept(socket);
            else
            {
                emaUnlock((EMAElement)tpkt);
                return;
            }
            if (newSocket==RVERROR)
            {
                int numOfLocks;
                /* call the callback with error indication */
                numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                tpkt->send.eventHandler((HTPKT)tpkt,event,0,TRUE,tpkt->send.context);
                emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
            }
            else
            /* We want to leave the listenning socket active */
            if (tpkt->type==tpktMultiServer)
            {
                tpktInfo *newTpkt = (tpktInfo *)emaAdd(tpkt->hEma, NULL);

                /* check that a new element was allocated */
                if (!newTpkt)
                {
                    int numOfLocks;
                    liClose(newSocket);
                    numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                    tpkt->send.eventHandler((HTPKT)tpkt,event,0,TRUE,tpkt->send.context);
                    emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
                    break;
                }

                /* initialize the new tpktInfo element */
                newTpkt->hEma=tpkt->hEma;
                newTpkt->socket=newSocket;
                newTpkt->close=FALSE;
                newTpkt->type=tpktServer; /* The connection type is tpktServer for the individiual
                                connection */
                newTpkt->event=liEvClose; /* set the ability to receive close events on the connection */
                newTpkt->recv.header[0]=0;
                newTpkt->isConnected=FALSE;

                newTpkt->recv.headerCount   = 0;
                newTpkt->recv.hRpool        = 0;
                newTpkt->recv.message       = 0;
                newTpkt->recv.offset        = 0;
                newTpkt->recv.rpoolLock     = 0;

                newTpkt->send.headerCount   = 0;
                newTpkt->send.hRpool        = 0;
                newTpkt->send.message       = 0;
                newTpkt->send.offset        = 0;
                newTpkt->send.rpoolLock     = 0;

                if (!error) newTpkt->isConnected=TRUE;

                /* register the event (close event) for the connection */
                liCallOnInstance(newTpkt->socket,newTpkt->event,tpktEvent,newTpkt,globals->thisThread);
                /* notify the appl that the accept was done */
                tpkt->send.eventHandler((HTPKT)newTpkt,event,0,FALSE,tpkt->send.context);
            }
            else
            /* we don't want the listenning socket anymore */
            {
                int numOfLocks;

                liClose(socket);
                if (!error) tpkt->isConnected=TRUE;

                /* use the object created as the connection object */
                tpkt->socket=newSocket;
                tpkt->event=liEvClose; /* set the ability to get close event on the connection */
                liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,context,globals->thisThread);
                /* notify that a connection was created */
                numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                tpkt->send.eventHandler((HTPKT)tpkt,event,0,FALSE,tpkt->send.context);
                emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
            }
        }
        break;

        /* Treat a connect event */
        case liEvConnect:
        {
            int numOfLocks;

            logPrint(globals->log, RV_DEBUG,
                    (globals->log, RV_DEBUG,
                    "tpktEvent(socket=%d, liEvConnect,error=%d,context=%x)",
                    socket, error, context));

            /* remove the connect event from the allowed events bitmap and add a close event ability*/
            tpkt->event = (liEvents)( tpkt->event & ~liEvConnect);
            tpkt->event = (liEvents)( tpkt->event | liEvClose);
            /* mark the connection as open */
            if (!error) tpkt->isConnected=TRUE;
            /* register the events for the connection */
            liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,context,globals->thisThread);

            /* notify the appl that the connect was done */
            numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
            tpkt->send.eventHandler((HTPKT)tpkt,event,0,error,tpkt->send.context);
            emaReturnFromCallback((EMAElement)tpkt, numOfLocks);

            if (error || tpkt->isConnected==FALSE) break;
        }
        break;

        /* treat a write event */
        case liEvWrite:
        {
            int  sent;
            BOOL finished = FALSE;

            logPrint(globals->log, RV_DEBUG,
                    (globals->log, RV_DEBUG,
                    "tpktEvent(socket=%d, liEvWrite,error=%d,context=%x)",
                    socket, error, context));

            /* send the rest of the message */
            if ((sent=liTcpSendIfConnected(tpkt, &finished)) == RVERROR)
            {
                int numOfLocks;

                /* remove the write event from the connection */
                tpkt->event= (liEvents)(tpkt->event & ~liEvWrite);

                /* notify that the message was sent but not completed */
                liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,context,globals->thisThread);

                numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                tpkt->send.eventHandler((HTPKT)tpkt,liEvWrite,0,TRUE,tpkt->send.context);
                emaReturnFromCallback((EMAElement)tpkt, numOfLocks);

                break;
            }

            /* if ALL the message was sent already */
            if (finished)
            {
                int numOfLocks;

                /* remove the write event from the connection */
                tpkt->event=(liEvents)(tpkt->event & ~liEvWrite);
                liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,context,globals->thisThread);

                /* remove the rPool pointer */
                tpkt->send.hRpool = NULL;

                /* notify that the message was completely sent and give the number of bytes sent */
                numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                tpkt->send.eventHandler((HTPKT)tpkt,liEvWrite,0,FALSE,tpkt->send.context);
                emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
            }
        }
        break;

        /* treat a read event */
        case liEvRead:
        {
            int recv;
            BOOL finished = FALSE;

            logPrint(globals->log, RV_DEBUG,
                    (globals->log, RV_DEBUG,
                    "tpktEvent(socket=%d, liEvRead,error=%d,context=%x)",
                    socket, error, context));

            if (error)
            {
                /* If we had an error on a read event, we assume there's nothing to do with the
                   connection and we close it */
                tpkt->close = TRUE;
            }

            /* if the connection is closing */
            if (tpkt->close)
            {
                int rc = 1;
                int safetyCounter=0;
#ifndef _NUCLEUS
                /* read everything still on the connection and drop it */
                for(safetyCounter = 0; (safetyCounter < 10) && (rc>0);safetyCounter++)
                    rc=liTcpRecv(tpkt->socket,(UINT8*)tpktDummyBuff,sizeof(tpktDummyBuff));
#endif
                /* if all was read, the connection was emptied */
                if ((rc<0) || (safetyCounter == 10))
                {
                    /* close the connection */
                    tpktClose((HTPKT)tpkt);
                }
                break;
            }

            /* if there is no buffer */
            if (!tpkt->recv.hRpool)
            {
                int numOfLocks;
                /* notify that there was nothing read */
                numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                tpkt->recv.eventHandler((HTPKT)tpkt,event,0,FALSE,tpkt->recv.context);
                emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
                break;
            }

            /* if we are still reading the header */
            if (tpkt->recv.headerCount < TPKT_HEADER_SIZE)
            {
                /* get the rest of the header */
                recv=liTcpRecvDirectIfConnected(tpkt,
                                                tpkt->recv.header+tpkt->recv.headerCount,
                                                TPKT_HEADER_SIZE-tpkt->recv.headerCount);
                if (recv == RVERROR)
                {
                    int numOfLocks;

                    /* On error, close down the read event capability and notify the error */
                    tpkt->event=(liEvents)(tpkt->event & ~liEvRead);
                    liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,context,globals->thisThread);

                    numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                    tpkt->recv.eventHandler((HTPKT)tpkt,event,0,TRUE,tpkt->recv.context);
                    emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
                    break;
                }
                tpkt->recv.headerCount += recv;
            }

            /* if we have the header we can allocate the rpool message */
            if (tpkt->recv.headerCount == TPKT_HEADER_SIZE)
            {
                int tpktLen=(((int)(tpkt->recv.header[2])<<8)+tpkt->recv.header[3])-TPKT_HEADER_SIZE;

                if (tpktLen > 0)
                {
                    meiEnter(tpkt->recv.rpoolLock);
                    tpkt->recv.message = rpoolRealloc(tpkt->recv.hRpool,
                                                      tpkt->recv.message,
                                                      tpktLen);
                    meiExit(tpkt->recv.rpoolLock);
                }
            }

            /* we are reading a new message body */
            if (tpkt->recv.headerCount >= TPKT_HEADER_SIZE)
            {
                int tpktLen=(((int)(tpkt->recv.header[2])<<8)+tpkt->recv.header[3])-TPKT_HEADER_SIZE;

                /* Do the actual reading */
                if (tpktLen > 0)
                {
                    if ((recv=liTcpRecvIfConnected(tpkt, &finished))==RVERROR)
                    {
                        int numOfLocks;

                        /* On error, close th eread event and notify */
                        tpkt->event= (liEvents)(tpkt->event & ~liEvRead);
                        liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,context,globals->thisThread);

                        numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                        tpkt->recv.eventHandler((HTPKT)tpkt,event,0,TRUE,tpkt->recv.context);
                        emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
                        break;
                    }
                }
                else
                {
                    /* finish the reading, we got a corrupted header */
                    tpktLen  = 0;
                    finished = TRUE;
                }

                /* if we read all that we asked */
                if (finished)
                {
                    int numOfLocks;
                    /* Close the read event, notify how many bytes were read */
                    tpkt->recv.hRpool = NULL;
                    tpkt->event= (liEvents)(tpkt->event & ~liEvRead);
                    liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,context,globals->thisThread);
                    numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                    tpkt->recv.eventHandler((HTPKT)tpkt,event,tpktLen,FALSE,tpkt->recv.context);
                    emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
                }
            }
        }
        break;


        /* Treat the event of closing of a connection */
        case liEvClose:
        {
            logPrint(globals->log, RV_DEBUG,
                    (globals->log, RV_DEBUG,
                    "tpktEvent(socket=%d, liEvClose,error=%d,context=%x)",
                    socket, error, context));

            /* In case we are not in a closing process already */
            if (!tpkt->close)
            {
                /* mark the connection as closing */
                tpkt->close=TRUE;

                /* notify that the close occured (as a recv event) */
                if (tpkt->recv.eventHandler != NULL)
                {
                    int numOfLocks;

                    numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                    tpkt->recv.eventHandler((HTPKT)tpkt,event,0,FALSE,tpkt->recv.context);
                    emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
                }
                else
                /* if no recv callback use the send callback (?) */
                if (tpkt->send.eventHandler != NULL)
                {
                    int numOfLocks;

                    numOfLocks = emaPrepareForCallback((EMAElement)tpkt);
                    tpkt->send.eventHandler((HTPKT)tpkt,event,0,FALSE,tpkt->send.context);
                    emaReturnFromCallback((EMAElement)tpkt, numOfLocks);
                }
            }
            /* if we already got a close event on this connection */
            else
            {
#ifndef _NUCLEUS
                int rc = 1;
                int safetyCounter=0;

                /* empty the connection from any impeding message */
                for(safetyCounter = 0; (safetyCounter < 10) && (rc>0);safetyCounter++)
                    rc = liTcpRecv(tpkt->socket,(UINT8*)tpktDummyBuff,sizeof(tpktDummyBuff));
#endif
                /* Close the tpkt object */
                tpktClose((HTPKT)tpkt);
            }
        }
        break;
    }
    emaUnlock((EMAElement)tpkt);
}

/*******************************************************************************************
 * tpktOpen
 *
 * Purpose: Allocate a socket and start either a connect process in case of a client which
 *      supplied a full address, or a listenning process in case of server.
 *      For a client that didn't supply an address yet, just create the socket.
 *
 * Input:   hTpktCtrl - Handle to the EMA pool of tpktInfo elements
 *          localIP   - ip address to use as local address
 *          localPort - port to use as local port
 *          type      - What type of connection we request:
 *          tpktClient - We want to connect (if address is supplied)
 *          tpktServer - we want to listen and then accept one connection
 *          tpktMultiServer - We want to listen, accept and remain listenning
 *          eventHandler - Callback routine to be called on any event on the connection
 *          context  - Additional data to be supplied on the callback
 *
 *
 * Return Value: A handle to the tpkt object created
 *******************************************************************************************/
HTPKT tpktOpen(HTPKTCTRL hTpktCtrl, UINT32 localIP, UINT16 localPort, tpktTypes type, LPTPKTEVENTHANDLER eventHandler,void*context)
{
    int socket;
    HEMA hEma = (HEMA)hTpktCtrl;
    tpktInfo* tpktNew = (tpktInfo*)emaAdd(hEma, NULL);
    tpktGlobals* globals = (tpktGlobals *)emaGetUserData((EMAElement)tpktNew);

    /* check that an element was created */
    if (!tpktNew)
        return NULL;

    if (!globals)
        return NULL;

     /* create a socket bound to the given port (for tpkt(Multi)Server) if any */
    socket=liOpen(localIP,localPort,LI_TCP);

    logPrint(globals->log, RV_INFO,
             (globals->log, RV_INFO,
            "tpktOpen(HTPKT=%x socket=%d)", tpktNew, socket));

    if (socket==RVERROR)
    {
        emaDelete((EMAElement)tpktNew);
        return NULL;
    }

    /* Create a local tpkt object */
    tpktNew->hEma=hEma;

    tpktNew->send.eventHandler=eventHandler;
    tpktNew->recv.eventHandler=eventHandler;

    tpktNew->send.context=context;
    tpktNew->recv.context=context;

    tpktNew->recv.header[0]=0;

    tpktNew->recv.headerCount = 0;
    tpktNew->recv.hRpool      = 0;
    tpktNew->recv.message     = 0;
    tpktNew->recv.offset      = 0;
    tpktNew->recv.rpoolLock   = 0;

    tpktNew->send.headerCount = 0;
    tpktNew->send.hRpool      = 0;
    tpktNew->send.message     = 0;
    tpktNew->send.offset      = 0;
    tpktNew->send.rpoolLock   = 0;

    tpktNew->socket=socket;
    tpktNew->type=type;

    tpktNew->close=FALSE;

    /* Prepare to start waiting for Connect and Accept events on the newly created socket */
    tpktNew->event=(liEvents)(((type==tpktClient)?liEvConnect:liEvAccept)/*|liEvClose*/);

    tpktNew->isConnected=FALSE;

    /* for servers start listenning on the socket */
    if (type!=tpktClient) {
      /* trigger the events */
      liCallOnInstance(socket,tpktNew->event,tpktEvent,(void*)tpktNew,globals->thisThread);
      liListen(socket,200);
    }

    return (HTPKT)tpktNew;
}


/*******************************************************************************************
 * tpktConnect
 *
 * Purpose: This routine supplements the tpktOpen routine, for clients which didn't supply
 *          an address and now wish to do the actual connect operation on the already allocated
 *          socket.
 *
 * Input:   hTpkt     - Handle to the tpktInfo element
 *          ip        - ip address to connect to
 *          port      - port to connect to
 *
 * Return Value: 0
 *******************************************************************************************/
int tpktConnect(HTPKT hTpkt, UINT32 ip, UINT16 port)
{
    tpktInfo* tpkt=(tpktInfo*)hTpkt;
    tpktGlobals* globals;

    if (!emaLock((EMAElement)tpkt)) return RVERROR;
    globals = (tpktGlobals *)emaGetUserData((EMAElement)tpkt);
    if (!globals) return RVERROR;

    /* only valid for a client */
    if (tpkt->type==tpktClient)
    {
#ifdef WIN32
        /* Trigger the events on the connection */
        liCallOnInstance(tpkt->socket, tpkt->event, tpktEvent, (void*)tpkt, globals->thisThread);
#endif
        /* Do the actual connect */
        liConnect(tpkt->socket,ip,port);
#ifndef WIN32
        /* Trigger the events on the connection */
        liCallOnInstance(tpkt->socket, tpkt->event, tpktEvent, (void*)tpkt, globals->thisThread);
#endif
    }

    emaUnlock((EMAElement)tpkt);

    return TRUE;
}

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
int tpktClose(HTPKT hTpkt)
{
    tpktInfo* tpkt=(tpktInfo*)hTpkt;
    tpktGlobals* globals;

    if (!emaLock((EMAElement)tpkt)) return RVERROR;
    globals = (tpktGlobals *)emaGetUserData((EMAElement)tpkt);
    if (!globals) return RVERROR;

    logPrint(globals->log, RV_INFO,
             (globals->log, RV_INFO,
            "tpktClose(HTPKT %x, socket=%d)", hTpkt, tpkt->socket));

    tpkt->send.context = NULL;
    tpkt->recv.context = NULL;
    tpkt->send.hRpool = NULL;
    tpkt->recv.hRpool = NULL;

    /* if indeed the connection is connected and not yet in a closing state */
    if (tpkt->isConnected==TRUE && !tpkt->close)
    {
        /* Mark the connection as closing */
        tpkt->close=TRUE;

        /* Triger only read event and close event, we are no longer interested in all
           the other events */
        liCallOnInstance(tpkt->socket,(liEvents)(liEvClose|liEvRead),tpktEvent,(void*)tpkt,globals->thisThread);

        /* Start a shutdown on the socket in the low level */
        liShutdown(tpkt->socket);

        {
            /* empty the messages still on the connection */
            int rc = 1;
            int safetyCounter=0;
#ifndef _NUCLEUS
            for(safetyCounter = 0; (safetyCounter < 10) && (rc>0);safetyCounter++)
                rc=liTcpRecvDirectIfConnected(tpkt,(UINT8*)tpktDummyBuff,sizeof(tpktDummyBuff));
#endif
            if ( (rc<0) || (safetyCounter == 10) )
            {
                /* All messages were cleared, close the socket and remove
                the tpkt object from the EMA pool */
                liClose(tpkt->socket);

                emaDelete((EMAElement)tpkt);
                emaUnlock((EMAElement)tpkt);

                return TRUE;
            }
        }

        /* mark the connection as no longer connected.
           We may reach this point only if there are still messages on the connection
           which are not yet ready to be read (rc above returned 0) */
        tpkt->isConnected=FALSE;
    }
    /* The connection is already closing */
    else
    {
        /* Vehemently close the socket and release the tpkt object */
        liClose(tpkt->socket);

        emaDelete((EMAElement)tpkt);
        emaUnlock((EMAElement)tpkt);

        return TRUE;
    }

    emaUnlock((EMAElement)tpkt);
    return TRUE;
}

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
                      HMEI rpoolLock)
{
    tpktInfo    *tpkt=(tpktInfo*)hTpkt;
    tpktGlobals *globals;

    int         sent;
    int         length;
    BYTE        *buffer;
    BOOL        finished = FALSE;

    if (!emaLock((EMAElement)tpkt)) return RVERROR;
    globals = (tpktGlobals *)emaGetUserData((EMAElement)tpkt);
    if (!globals) return RVERROR;
    if ((tpkt->send.hRpool) || (!tpkt->isConnected))
    {
        emaUnlock((EMAElement)tpkt);
        return RVERROR;
    }

    /* update the tpkt object with the rpool data */
    tpkt->send.hRpool    = hRpool;
    tpkt->send.message   = message;
    tpkt->send.offset    = offset;
    tpkt->send.rpoolLock = rpoolLock;

    meiEnter(rpoolLock);

    /* get the total message size */
    length = rpoolChunkSize(hRpool, message) - MSG_HEADER_SIZE; /* minus the pointer to the next message */

    /* Mark the tpkt header as a new message (tpkt ver) and set into it the length
       of the buffer + header */
    rpoolGetPtr(tpkt->send.hRpool, tpkt->send.message, tpkt->send.offset, (void **)&buffer, length + TPKT_HEADER_SIZE);

    buffer[0]=tpkt->send.header[0]=3;
    buffer[1]=tpkt->send.header[1]=0;
    buffer[2]=tpkt->send.header[2]=(unsigned char)((length)>>8);
    buffer[3]=tpkt->send.header[3]=(unsigned char)(length);

    meiExit(rpoolLock);

    /* Do the actual send of header + buffer */
    if ((sent=liTcpSendIfConnected(tpkt, &finished))==RVERROR)
    {
        tpkt->send.hRpool = NULL;
        emaUnlock((EMAElement)tpkt);
        return RVERROR;
    }

    /* if all was sent, return the num of bytes sent */
    if (finished)
    {
        tpkt->send.hRpool = NULL;
        emaUnlock((EMAElement)tpkt);
        return length;
    }

    /* trigger the read event for the next chunk of data */
    tpkt->event=(liEvents)(tpkt->event | liEvWrite);
    liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,(void*)tpkt,globals->thisThread);

    emaUnlock((EMAElement)tpkt);
    return 0;
}

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
int tpktRecvNotify(HTPKT hTpkt,LPTPKTEVENTHANDLER eventHandler,void*context)
{
    tpktInfo *tpkt=(tpktInfo*)hTpkt;
    tpktGlobals *globals;

    if (!emaLock((EMAElement)tpkt)) return RVERROR;
    globals = (tpktGlobals *)emaGetUserData((EMAElement)tpkt);
    if (!globals) return RVERROR;

    if (tpkt->recv.hRpool)
    {
        emaUnlock((EMAElement)tpkt);
        return RVERROR;
    }

    tpkt->recv.context=context;
    tpkt->recv.eventHandler=eventHandler;

    /* Trigger the evRead event mechanism */
    tpkt->event=(liEvents)(tpkt->event | liEvRead);
    liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,(void*)tpkt,globals->thisThread);

    emaUnlock((EMAElement)tpkt);
    return 0;
}

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
                      HMEI rpoolLock)
{
    tpktInfo* tpkt=(tpktInfo*)hTpkt;
    tpktGlobals* globals;

    int recv;
    int length;
    BOOL finished = FALSE;

    if (!emaLock((EMAElement)tpkt)) return RVERROR;
    globals = (tpktGlobals *)emaGetUserData((EMAElement)tpkt);
    if (!globals) return RVERROR;
    if ((tpkt->recv.hRpool) || (!tpkt->isConnected))
    {
        emaUnlock((EMAElement)tpkt);
        return RVERROR;
    }

    /* update the tpkt object with the rpool data */
    tpkt->recv.hRpool    = hRpool;
    tpkt->recv.message   = message;
    tpkt->recv.offset    = 0;
    tpkt->recv.rpoolLock = rpoolLock;

    /* get the total message size */
    length = rpoolChunkSize(hRpool, message);

    /* We can start getting the new message header */
    if ((recv=liTcpRecvDirectIfConnected(tpkt,tpkt->recv.header,TPKT_HEADER_SIZE))==RVERROR)
    {
        /* We had an error, and the transnet is going to free the rpool buffer -
           we should do the same before we let it know of the error */
        tpkt->recv.hRpool = NULL;
        tpkt->event= (liEvents)(tpkt->event & ~liEvRead);
        liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,(void*)tpkt,globals->thisThread);
        emaUnlock((EMAElement)tpkt);
        return RVERROR;
    }

    tpkt->recv.headerCount = recv;

    /* we have a valid header */
    if (tpkt->recv.headerCount == TPKT_HEADER_SIZE)
    {
        /* Calculate the message length */
        int tpktLen=(((int)(tpkt->recv.header[2])<<8)+tpkt->recv.header[3])-TPKT_HEADER_SIZE;

        /* allocate the rest of the rpool message by the size in the header */
        meiEnter(tpkt->recv.rpoolLock);
        tpkt->recv.message = rpoolRealloc(tpkt->recv.hRpool,
                                          tpkt->recv.message,
                                          tpktLen);
        meiExit(tpkt->recv.rpoolLock);

        /* mark as new message and calculate how many bytes we may attempt to read */
        tpkt->recv.header[0]=3;

        /* Do the actual reading */
        if ((recv=liTcpRecvIfConnected(tpkt, &finished))==RVERROR)
        {
            tpkt->recv.hRpool = NULL;
            emaUnlock((EMAElement)tpkt);
            return RVERROR;
        }

        tpkt->recv.headerCount += recv;

        /* we read all we requested */
        if (finished)
        {
            tpkt->recv.hRpool = NULL;
            tpkt->event= (liEvents)(tpkt->event & ~liEvRead);
            liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,(void*)tpkt,globals->thisThread);
            emaUnlock((EMAElement)tpkt);
            return tpktLen;
        }
    }

    /* We have incomplete message, set the EvRead event and exit */
    tpkt->event=(liEvents)(tpkt->event | liEvRead);
    liCallOnInstance(tpkt->socket,tpkt->event,tpktEvent,(void*)tpkt,globals->thisThread);

    emaUnlock((EMAElement)tpkt);
    return 0;
}


/*******************************************************************************************
 * tpktGetSock
 *
 * Purpose: return the socket associated with the given tpkt object.
 *
 * Input:   hTpkt - A handle to the TPKT module
 *
 * Return Value: The number of the socket
 *******************************************************************************************/
int tpktGetSock(HTPKT hTpkt)
{
    tpktInfo* tpkt=(tpktInfo*)hTpkt;
    int       socket = RVERROR;

    if (!emaLock((EMAElement)tpkt)) return RVERROR;

    socket = tpkt->socket;
    emaUnlock((EMAElement)tpkt);

    return socket;
}

#ifdef __cplusplus
}
#endif



