#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

extern int getAppIp(void *p);

/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/



#include <rvinternal.h>
#include <transport.h>
#include <transportint.h>
#include <transutil.h>
#include <transStates.h>
#include <q931asn1.h>
#include <transnet.h>
#include <emanag.h>
#include <cmutils.h>
#include <prnutils.h>

/**************************************************************************************
 * createHostByType
 *
 * Purpose: To allocate a new host element and fill it with data according to its
 *          type, i.e. Q.931-TPKT, Q.931-Annex E, H.245
 *
 * Input:  transGlobals     - The global variables of the module
 *         hsTranSession    - The session for which the host is created (may be NULL)
 *         type             - The type of the connection (TPKT, H.245, Annex E)
 *         hTpkt            - The connection handle in TPKT for a new incoming connection
 *         annexEUsageMode  - is this an annex E host or other, i.e. TPKT for Q.931 or H.245
 *
 * Output: hsTransHost -  A handle for the newly created host element.
 *
 **************************************************************************************/
 TRANSERR createHostByType( cmTransGlobals      *transGlobals,
                            HSTRANSSESSION      hsTransSession,
                            HSTRANSHOST         *hsTransHost,
                            TRANSCONNTYPE       type,
                            HTPKT               hTpkt,
                            cmAnnexEUsageMode   annexEUsageMode)
 {
    cmTransSession *session = (cmTransSession *)hsTransSession;
    cmTransHost    *host;
    int            socket;

    cmTransportAddress local, remote;
    TRANSERR       res;

    /* allocate a new host element */
    *hsTransHost = (HSTRANSHOST)emaAdd(transGlobals->hEmaHosts, NULL);

    if (!*hsTransHost)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "createHostByType failed on allocating host element"));
        return cmTransErr;
    }
    else
    {
        host = (cmTransHost *)*hsTransHost;
        memset(host, 0, sizeof(cmTransHost));
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "createHostByType created host=%d(%x)",emaGetIndex((EMAElement)host),host));
    }

    memset(&local, 0, sizeof(local));
    local.type  = (cmTransportType)-1;
    memset(&remote, 0, sizeof(remote));
    remote.type = (cmTransportType)-1;

    /* set the params of the new host */
    switch (type)
    {
        case cmTransQ931Conn:
        {
            /* The host doesn't have a session attached to it yet,
               so fill it with default params and data from the new
               incoming connection */
            if (annexEUsageMode != cmTransNoAnnexE)
            {
                annexEStatus eStat;

                /* set parameters for the newly created host */
                host->closeOnNoSessions         = TRUE;
                host->isMultiplexed             = TRUE;
                host->remoteCloseOnNoSessions   = FALSE;
                host->remoteIsMultiplexed       = TRUE; /* Annex E is always multiplexed */
                host->annexEUsageMode           = cmTransUseAnnexE;
                host->firstSession      = NULL;
                host->state             = hostConnected;
                host->type              = type;
                host->incomingMessage   = NULL;
                host->hTpkt             = NULL;
                host->connectTimer      = (HTI)-1;

                /* get the local address of the annex E */
                eStat = annexEGetLocalAddress(transGlobals->annexECntrl, &local);
                if (eStat != annexEStatusNormal)
                {
                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                        "createHostByType failed on getting annex E local address"));
                }
            }
            else
            {
                host->closeOnNoSessions         = (session? session->closeOnNoSessions : FALSE);
                host->isMultiplexed             = (session? session->isMultiplexed : TRUE);
                host->remoteCloseOnNoSessions   = host->closeOnNoSessions;
                host->remoteIsMultiplexed       = (session ? !session->outgoing : TRUE); /* Assume non-multiplexed for outgoing connections until we know better */
                host->annexEUsageMode           = cmTransNoAnnexE;
                host->firstSession              = NULL;
                host->state                     = hostIdle;
                host->type                      = type;
                host->incomingMessage           = NULL;
                host->hTpkt                     = hTpkt;
                host->connectTimer              = (HTI)-1;

                if (hTpkt)
                {
                    socket = tpktGetSock(hTpkt);

                    remote.type = cmTransportTypeIP;
                    remote.ip   = liGetRemoteIP(socket);
                    remote.port = liGetRemotePort(socket);

                    local.type = cmTransportTypeIP;
                    local.ip   = liGetSockIP(socket);
                    local.port = liGetSockPort(socket);
                }
            }

            break;
        }

        case cmTransH245Conn:
        {
            /* must always have a session on whose behalf either the connect or the
               listen was done */
            host->closeOnNoSessions         = TRUE;
            host->isMultiplexed             = FALSE;
            host->remoteCloseOnNoSessions   = TRUE;
            host->remoteIsMultiplexed       = FALSE;
            host->annexEUsageMode   = cmTransNoAnnexE;
            host->firstSession      = session;
            host->state             = hostIdle;
            host->type              = type;
            host->incomingMessage   = NULL;
            host->hTpkt             = hTpkt;
            host->connectTimer      = (HTI)-1;

            if (hTpkt)
            {
                socket = tpktGetSock(hTpkt);

                remote.type = cmTransportTypeIP;
                remote.ip   = liGetRemoteIP(socket);
                remote.port = liGetRemotePort(socket);

                local.type = cmTransportTypeIP;
                local.ip   = liGetSockIP(socket);
                local.port = liGetSockPort(socket);
            }

            break;
        }
    }

    /* update the local and remote addresses of the host */
    host->localAddress = local;
    res = setRemoteAddress(host, &remote);
    if (res != cmTransOK)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "createHostByType failed on setting host remote address"));
        closeHostInternal(*hsTransHost, TRUE);
        return cmTransErr;
    }

    /* return the newly created host element */
    if (hsTransHost)
        *hsTransHost = (HSTRANSHOST)host;
    return cmTransOK;
}

/**************************************************************************************
 * determineWinningHost
 *
 * Purpose: check if there was a competition between TPKT and annex E, and if so
 *          close the Annex E.
 *
 * Input:   transGlobals  - The global variables of the module
 *          session       - The session for which the conection was made
 *          annexEWon     - which of the two hosts won..
 *
 **************************************************************************************/
BOOL determineWinningHost(cmTransGlobals *transGlobals, cmTransSession *session, BOOL annexEWon)
{
    cmTransHost *loosingHost = NULL;
    cmTransHost *winningHost = NULL;
    BOOL answer = TRUE;
    HATRANSHOST winHaTransHost = NULL;
    HATRANSHOST looseHaTransHost = NULL;

    if (!transGlobals)
        return answer;

    if (annexEWon)
    {
        winningHost = session->annexEConnection;
        loosingHost = session->Q931Connection;
    }
    else
    {
        winningHost = session->Q931Connection;
        loosingHost = session->annexEConnection;
    }

    /* check if we're not too late */
    if ( (loosingHost) && (loosingHost->hostIsApproved) && (!winningHost->hostIsApproved) )
    {
        cmTransHost *tempHost;

        /* reversal of fortunes */
        tempHost    = loosingHost;
        loosingHost = winningHost;
        winningHost = tempHost;

        annexEWon = !annexEWon;

        answer = FALSE;
    }


    /* print the result */
    if (annexEWon)
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "determineWinningHost: annex E has won (host=%d-%x)",
                emaGetIndex((EMAElement)session->annexEConnection), session->annexEConnection));
    }
    else
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "determineWinningHost: TPKT has won (host=%d-%x)",
                emaGetIndex((EMAElement)session->Q931Connection), session->Q931Connection));
    }

    /* disconnect the loosing host and mark the winning one */
    {
        winningHost->hostIsApproved = TRUE;

        if (emaLock((EMAElement)loosingHost))
        {
            looseHaTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)loosingHost);

            if ( annexEWon )
            {
                meiEnter(transGlobals->lock);

                /* disconnect it from the loosing host */
                loosingHost->firstSession = session->nextSession;
                if (session->nextSession)
                    session->nextSession->prevSession = NULL;

                meiExit(transGlobals->lock);
            }

            if ( (!loosingHost->firstSession) &&
                 ((loosingHost->closeOnNoSessions) || (loosingHost->remoteCloseOnNoSessions)) )
            {
                /* totally kill the host */
                closeHostInternal((HSTRANSHOST)loosingHost, FALSE);
            }

            emaUnlock((EMAElement)loosingHost);
        }

        meiEnter(transGlobals->lock);

        if ( annexEWon )
        {
            if (!session->connectedToHost)
            {
                /* connect the session to the winning host */
                session->nextSession = winningHost->firstSession;
                if (session->nextSession)
                    session->nextSession->prevSession = session;
                winningHost->firstSession = session;
                session->annexEUsageMode = cmTransUseAnnexE;
                session->connectedToHost = TRUE;
            }
        }
        else
            session->annexEUsageMode = cmTransNoAnnexE;

        /* save the application handle of the loosing host */
        winHaTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)winningHost);

        if (!winHaTransHost)
            emaSetApplicationHandle((EMAElement)winningHost, (void *)looseHaTransHost);

        meiExit(transGlobals->lock);

    }

    /* clear the loosing host from the session */
    if (session->Q931Connection == loosingHost)
        session->Q931Connection = NULL;
    if (session->annexEConnection == loosingHost)
        session->annexEConnection = NULL;

    return answer;
}

/**************************************************************************************
 * transReportConnect
 *
 * Purpose: To invoke the callbacks for the connected host and all its associated sessions
 *
 * Input: host         - The host which got connected.
 *        session      - The session on which to report the host connection (NULL for all).
 *        isConnected  - TRUE: A connect happened, FALSE: an accept happened.
 *
 **************************************************************************************/
void transReportConnect(cmTransHost *host, cmTransSession *session, BOOL isCOnnnected)
{
    cmTransSession *nextSession;
    HATRANSSESSION haTransSession;

    /* retrieve the transport module global area */
    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

    /* report the connection of the host only if its a TPKT connection  */
    if ( ((!session) || (session->annexEConnection != host)) &&
         (transGlobals->hostEventHandlers.cmEvTransHostConnected) )
    {
        HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);
        int NumOfLocks;

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "cmEvTransHostConnected(hsHost = %d(%x), haHost=%x, type=%d)",
                emaGetIndex((EMAElement)host), host, haTransHost,host->type));

        NumOfLocks = emaPrepareForCallback((EMAElement)host);
        (transGlobals->hostEventHandlers.cmEvTransHostConnected)(
                                            (HSTRANSHOST) host,
                                            haTransHost,
                                            host->type,
                                            isCOnnnected);
        emaReturnFromCallback((EMAElement)host, NumOfLocks);
    }

    /* if reporting was asked for just one session (in case of dummy connect for
       multiplexed hosts just report on that session */

    if ( (session) && (!emaWasDeleted((EMAElement)host)) )
    {
        if (transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection)
        {
            /* report just for the given session (if one is given) */
            if ( (!session->reportedQ931Connect) )
            {
                int sessNumOfLocks, hostNumOfLocks;
                haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransSessionNewConnection(hsSession = %d(%x), haSession=%x, type=%d)",
                        emaGetIndex((EMAElement)session), session, haTransSession, host->type));

                sessNumOfLocks = emaPrepareForCallback((EMAElement)session);
                hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection(
                                (HSTRANSSESSION)session, haTransSession, host->type);
                emaReturnFromCallback((EMAElement)session, sessNumOfLocks);
                emaReturnFromCallback((EMAElement)host, hostNumOfLocks);

                session->reportedQ931Connect = TRUE;
            }
        }
    }
    else
    /* go over the sessions that are connected to that host and report the connect */
    if (!emaWasDeleted((EMAElement)host))
    {
        int sessNumOfLocks, hostNumOfLocks;

        /* for Q.931 hosts, go over all the sessions connected to them and report them */
        if (host->type == cmTransQ931Conn)
        for (nextSession = host->firstSession; nextSession != NULL; nextSession = nextSession->nextSession)
        {
            if (emaLock((EMAElement)nextSession))
            {
                /* report the connection */
                haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)nextSession);

                /* for the Q.931 type connections (TPKT or annex E) report them always */
                if (!emaWasDeleted((EMAElement)nextSession))
                    if (transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection)
                    {
                        /* report just for the session */
                        if ( (!nextSession->reportedQ931Connect) )
                        {
                            logPrint(transGlobals->hLog, RV_INFO,
                                    (transGlobals->hLog, RV_INFO,
                                    "cmEvTransSessionNewConnection(hsSession = %d(%x), haSession=%x, type=%d)",
                                    emaGetIndex((EMAElement)nextSession), nextSession, haTransSession, host->type));

                            hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                            sessNumOfLocks = emaPrepareForCallback((EMAElement)nextSession);
                            transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection(
                                    (HSTRANSSESSION)nextSession, haTransSession, host->type);
                            emaReturnFromCallback((EMAElement)nextSession, sessNumOfLocks);
                            emaReturnFromCallback((EMAElement)host, hostNumOfLocks);

                            if (nextSession)
                            {
                                if (host->type == cmTransQ931Conn)
                                    nextSession->reportedQ931Connect = TRUE;
                                else
                                    nextSession->reportedH245Connect = TRUE;
                            }
                        }
                    }

                /* unlock and release the current session */
                emaUnlock((EMAElement)nextSession);
            }
        }

        /* for H.245, check if there is a session attached to it, and if so
           report the connection on that session */
        if (host->type == cmTransH245Conn)
        {
            nextSession = host->firstSession;

            if (nextSession)
            {
                haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)nextSession);

                /* Determine whether to report on H.245 connection */
                if (!nextSession->reportedH245Connect)
                {
                    /* mark that we already reported on an H.245 connection for this session */
                    nextSession->reportedH245Connect = TRUE;

                    if (transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection)
                    {
                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "cmEvTransSessionNewConnection(hsSession = %d(%x), haSession=%x, type=cmTransH245Conn)",
                                emaGetIndex((EMAElement)nextSession), nextSession, haTransSession));

                        hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                        sessNumOfLocks = emaPrepareForCallback((EMAElement)nextSession);
                        transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection(
                                            (HSTRANSSESSION)nextSession, haTransSession, cmTransH245Conn);
                        emaReturnFromCallback((EMAElement)nextSession, sessNumOfLocks);
                        emaReturnFromCallback((EMAElement)host, hostNumOfLocks);

                        if (nextSession)
                            nextSession->reportedH245Connect = TRUE;
                    }
                }
            }
        }
    }

    /* mark the host as connecte dso it will be reported when closed */
    if (!emaWasDeleted((EMAElement)host))
        host->reported = TRUE;
}

/**************************************************************************************
 * solveH245Conflict
 *
 * Purpose: To decide which one of two coliding H.245 connections will survive.
 *
 * Input: host         - The h.245 host which has a listen and connecting processes.
 *
 **************************************************************************************/
void solveH245Conflict(cmTransHost *host)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

    if (!transGlobals)
        return;

    /* if we are having a connecting request and a accepting request, determine
       which one should be closed */
    if (host->state == hostListenningConnecting)
    {
        int cmpRes;

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "solveH245Conflict on host=%d(%x) local:(ip=%x;port=%x) remote:(ip=%x;port=%x)",
                emaGetIndex((EMAElement)host), host, host->localAddress.ip, host->localAddress.port,
                host->remoteAddress.ip, host->remoteAddress.port));

        /* compare the listenning address (local) to the connecting address (remote) */
        cmpRes = memcmp((void *)&host->remoteAddress.ip,
                        (void *)&host->localAddress.ip,
                        sizeof(host->localAddress.ip));

        /* if the remote address is bigger or if both address are the same but the remote
           port is bigger, then close the connecting host and keep the accepting one,
           else do the opposite */
        if ( (cmpRes > 0) ||
             (
              (cmpRes == 0) &&
              (host->remoteAddress.port > host->localAddress.port)
             )
           )
        {
            /* close the connecting host */
            if (host->hTpkt)
            {
                tpktClose(host->hTpkt);
                host->hTpkt = NULL;
            }
            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "solveH245Conflict closed outgoing colliding connection"));

            /* the host now is just listenning */
            host->state = hostListenning;
        }
        else
        {
            /* close the accepting socket and don't touch the connecting one */
            if (host->h245Listen)
            {
                tpktClose(host->h245Listen);
                host->h245Listen = NULL;
            }
            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "solveH245Conflict closed incoming colliding connection"));
        }
    }
}


/**************************************************************************************
 * acceptHostTimeout
 *
 * Purpose: The callback routine for the socket that was accepted, making sure that
 *          no one tries to open a socket without sending any data on it.
 *
 * Input: the host to close
 *
 **************************************************************************************/
static void RVCALLCONV acceptHostTimeout(IN void*  userData)
{
    cmTransHost *host = (cmTransHost *)userData;
    cmTransGlobals *transGlobals;
    
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

    if (host->connectTimer != (HTI)-1)
    {
        mtimerReset(transGlobals->hTimers, host->connectTimer);
        host->connectTimer = (HTI)-1;
    }
    
    closeHostInternal((HSTRANSHOST)host, FALSE);
}

/**************************************************************************************
 * transQ931AcceptHandler
 *
 * Purpose: The callback routine for the socket that listens for incoming Q.931 requests
 *
 * Input: standard input of TPKT callback
 *
 **************************************************************************************/
void transQ931AcceptHandler(HTPKT tpkt,liEvents event,int length,int error,void*context)
{
    /* retrieve the transport module global area */
    cmTransGlobals *transGlobals = (cmTransGlobals *)context;

    if (length);

    if (transGlobals == NULL)
        return;

    if (error)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "transQ931AcceptHandler got error on event %d", event));
        return;
    }

    /* we don't treat the pre-accept event for Q.931 */
    if (event == 0)
        return;

    /* treat the accept event of a Q.931 TPKT connection */
    if (event == liEvAccept)
    {
        TRANSERR    res;
        cmTransHost *host;

        /* create a new connected host element for the new connection */
        res = createHostByType( transGlobals,
                                NULL,
                                (HSTRANSHOST *)&host,
                                cmTransQ931Conn,
                                tpkt,
                                cmTransNoAnnexE);
        if (res != cmTransOK)
        {
            tpktClose(tpkt);
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "transQ931AcceptHandler rejected connection (host wasnt created) res=%d", res));
            return;
        }

 		/* Start timer which will initiate a process to release resources in case when:
		   1. there we no activity after "accept"  
		   2. "SETUP" message was rejected by user in the "hook" function */
        host->connectTimer = mtimerSet(transGlobals->hTimers, acceptHostTimeout, host, 10000);

        /* mark the host as incoming one */
        host->incoming = TRUE;

        /* lock and mark the host before reporting it */
        emaLock((EMAElement)host);

        host->state = hostConnected;

        transReportConnect(host, NULL, FALSE);

        /* register a new callback */
        tpktRecvNotify(tpkt, transQ931Handler, host);

        emaUnlock((EMAElement)host);
    }
    else
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "transQ931AcceptHandler got event %d (this cannot happen !!!)", event));
    }
}

/**************************************************************************************
 * transH245Handler
 *
 * Purpose: The callback routine for a H245 connection
 *
 * Input: standard input of TPKT callback
 *
 **************************************************************************************/
void transH245Handler(HTPKT tpkt,liEvents event,int length,int error,void*context)
{
    cmTransHost    *host    = (cmTransHost *)context;
    cmTransSession *session;
    cmTransGlobals *transGlobals;
    int            sessNumOfLocks, hostNumOfLocks;

    if (!emaLock((EMAElement)host))
        return;

    session = host->firstSession;

    /* reverse the lcoking order so the session is locked BEFORE the host */
    hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
    if (!emaLock((EMAElement)session))
    {
        emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
        return;
    }
    if (emaReturnFromCallback((EMAElement)host, hostNumOfLocks) != TRUE)
    {
        emaUnlock((EMAElement)session);
        return;
    }

    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

        if (error)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "transH245Handler got error on event %d", event));

            /* in case of a connection error report it so the user may discard the session */
            if (event == liEvConnect)
            {
                if (transGlobals->sessionEventHandlers.cmEvTransConnectionOnSessionClosed)
                {
                    HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransConnectionOnSessionClosed(hsSession = %d(%x), haSessiont=%x, type=cmTransH245Conn)",
                            emaGetIndex((EMAElement)session),session,haTransSession));

                    hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                    sessNumOfLocks = emaPrepareForCallback((EMAElement)session);
                    transGlobals->sessionEventHandlers.cmEvTransConnectionOnSessionClosed(
                                                    (HSTRANSSESSION) session,
                                                    haTransSession,
                                                    cmTransH245Conn);
                    emaReturnFromCallback((EMAElement)session, sessNumOfLocks);
                    emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
                }

                /* report the host close itself */
                if ( (transGlobals->hostEventHandlers.cmEvTransHostClosed) && (host->reported) &&
                     (host->state != hostClosing) && (host->state != hostClosed) )
                {
                    int numOfLocks;
                    BOOL wasConnected  = ( (host->state == hostConnected) ||
                                           (host->state == hostBusy) );
                    HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

                    host->state = hostClosing;
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransHostClosed(hsHost = %d(%x), haHost=%x, wasConnected = %d)",
                            emaGetIndex((EMAElement)host), host, haTransHost, wasConnected));

                    numOfLocks = emaPrepareForCallback((EMAElement)host);
                    transGlobals->hostEventHandlers.cmEvTransHostClosed((HSTRANSHOST)host,
                                                                        haTransHost,
                                                                        wasConnected);
                    emaReturnFromCallback((EMAElement)host, numOfLocks);
                }
            }
        }
        else
        {
            switch (event)
            {
                case liEvConnect:
                {
                    TRANSERR res;

                    if (host->hTpkt != tpkt)
                        break;

                    /* all ok we can use that connection */
                    host->state = hostConnected;

                    /* set the recieve event */
                    tpktRecvNotify(tpkt, transH245Handler, context);

                    /* close the listennning process if there was one */
                    if (host->h245Listen)
                    {
                        tpktClose(host->h245Listen);
                        host->h245Listen = NULL;
                    }

                    /* set the new local address of the host */
                    {
                        int socket = tpktGetSock(host->hTpkt);
                        host->localAddress.ip   = liGetSockIP(socket);
                        host->localAddress.port = liGetSockPort(socket);
                    }

                    if ((host->dummyHost) && (host->remoteAddress.port != 0) )
                    {
                        /* we connected a dummy host, it is no longer dummy */
                        host->dummyHost = FALSE;
                        /* We are reporting this connection as a NEW H.245 connection, but it will NOT reset
                          control, because host->firstSession->reportedH245Connect is true. We should (for 
                          later versions) create a special callback for dummy host cone connecting */
                    }

                    /* try and send all the tunneled messages that weren't sent or acked yet */
                    host->firstMessage    = session->firstMessage;
                    session->firstMessage = NULL;

                    res = sendMessageOnHost(transGlobals, host);
                    if ( (res != cmTransOK) && (res != cmTransConnectionBusy) )
                    {
                        logPrint(transGlobals->hLog, RV_ERROR,
                                (transGlobals->hLog, RV_ERROR,
                                "transH245Handler Failed on sending ex-tunneled msgs for host %d(%x)",
                                emaGetIndex((EMAElement)host), host));
                    }

                    /* the H.245 connection is established, notify the user */
                    transReportConnect(host, NULL, TRUE);
                    break;
                }

                case liEvAccept:
                /* can't happen, treated by another callback */
                {
                    break;
                }

                case liEvClose:
                case liEvRead:
                {
                    int  pvtNode = -1;
                    int  res;
                    void *hMsgContext;
                    int  offset = 0;

                    do
                    {
                        /* try to read and decode a message */
                        res = decodeIncomingMessage(tpkt, &length, &offset, context,
                                                    &pvtNode, cmTransH245Type, &hMsgContext);

                        /* set the read event on tpkt */
                        tpktRecvNotify(tpkt, transH245Handler, host);

                        if (res>=0)
                        {
                            HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

                            /* report the message to the user */
                            if ( (transGlobals->sessionEventHandlers.cmEvTransNewMessage)  &&
                                 (!emaWasDeleted((EMAElement)session)) )
                            {
                                logPrint(transGlobals->hLog, RV_INFO,
                                        (transGlobals->hLog, RV_INFO,
                                        "cmEvTransNewMessage(hsSession = %d(%x), haSession=%x, type=cmTransH245Type, pvtNode=%d)",
                                        emaGetIndex((EMAElement)session), session, haTransSession, pvtNode));

                                sessNumOfLocks = emaPrepareForCallback((EMAElement)session);
                                hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                                transGlobals->sessionEventHandlers.cmEvTransNewMessage(
                                                            (HSTRANSSESSION) session,
                                                            (HATRANSSESSION) haTransSession,
                                                            cmTransH245Type,
                                                            pvtNode,
                                                            hMsgContext);
                                emaReturnFromCallback((EMAElement)session, sessNumOfLocks);
                                emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
                            }

                            /* get rid of the decoded message */
                            pvtDelete(transGlobals->hPvt, pvtNode);
                        }
                    } while ( (length > 0)  && (!emaWasDeleted((EMAElement)session)) );

                    if ( (event == liEvClose) && (!emaWasDeleted((EMAElement)host)) &&
                         (host->state != hostClosing) && (host->state != hostClosed) )
                    {
                        if ( (transGlobals->hostEventHandlers.cmEvTransHostClosed) && (host->reported) )
                        {
                            BOOL wasConnected       = ( (host->state == hostConnected) ||
                                                        (host->state == hostBusy) );
                            HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);
                            int numOfLocks;

                            host->state = hostClosing;
                            logPrint(transGlobals->hLog, RV_INFO,
                                    (transGlobals->hLog, RV_INFO,
                                    "cmEvTransHostClosed(hsHost = %d(%x), haHost=%x, wasConnected = %d)",
                                    emaGetIndex((EMAElement)host), host, haTransHost, wasConnected));

                            numOfLocks = emaPrepareForCallback((EMAElement)host);
                            transGlobals->hostEventHandlers.cmEvTransHostClosed((HSTRANSHOST)host,
                                                                                haTransHost,
                                                                                wasConnected);
                            emaReturnFromCallback((EMAElement)host, numOfLocks);
                        }
                        else
                        {
                            if (!emaWasDeleted((EMAElement)host))
                            {
                                host->state = hostClosed;
                                transHostClose((HSTRANSHOST)host, TRUE);
                            }
                        }
                    }

                    break;
                }
                case liEvWrite:
                {
                    TRANSERR res;

                    /* the last send was completed */
                    /* remove the message from the pool */
                    extractMessageFromPool(transGlobals, host, FALSE);

                    /* mark the host as not busy and try sending the remaining messages */
                    host->state = hostConnected;

                    res = sendMessageOnHost(transGlobals, host);
                    if ( (res != cmTransOK) && (res != cmTransConnectionBusy) )
                    {
                        logPrint(transGlobals->hLog, RV_ERROR,
                                (transGlobals->hLog, RV_ERROR,
                                "transH245Handler Failed on send for host %d(%x)",
                                emaGetIndex((EMAElement)host), host));
                    }
                    break;
                }
            }
        }

        /* unlock the host and its session */
        emaUnlock((EMAElement)host);
        emaUnlock((EMAElement)session);
    }
}

/**************************************************************************************
 * transH245AcceptHandler
 *
 * Purpose: The callback routine for the socket that listens for incoming H.245 request
 *
 * Input: standard input of TPKT callback
 *
 **************************************************************************************/
void transH245AcceptHandler(HTPKT tpkt,liEvents event,int length,int error,void*context)
{
    cmTransHost    *host    = (cmTransHost *)context;
    cmTransSession *session;
    cmTransGlobals *transGlobals;
    TRANSERR       res;
    int            hostNumOfLocks;

    if (length);

    if (!emaLock((EMAElement)host))
        return;

    session = host->firstSession;

    /* reverse th elcoking order so the session is locked BEFORE the host */
    hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
    if (!emaLock((EMAElement)session))
    {
        emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
        return;
    }
    if (emaReturnFromCallback((EMAElement)host, hostNumOfLocks) != TRUE)
    {
        emaUnlock((EMAElement)session);
        return;
    }

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* check if we realy want this connection or we already opened one */
    if (event == 0)
    {
        solveH245Conflict(host);

        /* after solving the problem (or if there was no connecting host at all)
           the host is in any case connecting now */

        host->state = hostConnecting;
    }
    else
    if (error)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "transH245AcceptHandler got error on event %d", event));
    }
    else
    if (session->H245Connection != host)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "transH245AcceptHandler mismatched session and host %x %x",session->H245Connection, host));
    }
    else
    /* treat the accept event of a H.245 TPKT connection */
    if ( (event == liEvAccept) && (context) )
    {
        /* set the new local address of the host */
        {
            int socket = tpktGetSock(tpkt);
            session->H245Connection->localAddress.ip   = liGetSockIP(socket);
            session->H245Connection->localAddress.port = liGetSockPort(socket);
        }

        session->H245Connection->state = hostConnected;

        /* set the H245 connection to point to the accepting socket */
        session->H245Connection->hTpkt      = tpkt;
        session->H245Connection->h245Listen = NULL;

        if (session->isTunnelingSupported)
            session->switchToSeparateWasAsked = TRUE;

        /* try and send all the tunneled messages that weren't sent or acked yet */
        session->H245Connection->firstMessage    = session->firstMessage;
        session->firstMessage = NULL;

        /* set the receive event and change the handler routine */
        tpktRecvNotify(tpkt, transH245Handler, context);

        res = sendMessageOnHost(transGlobals, session->H245Connection);
        if ( (res != cmTransOK) && (res != cmTransConnectionBusy) )
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "transH245AcceptHandler Failed on send of ex-tunneled msgs for host %d(%x)",
                    emaGetIndex((EMAElement)session->H245Connection), session->H245Connection));
        }

        /* the H.245 connection is established, notify the user */
        transReportConnect(session->H245Connection, NULL, FALSE);
    }
    else
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "transH245AcceptHandler got event %d (this cannot happen !!!)", event));
    }

    /* unlock and release the host and its session */
    emaUnlock((EMAElement)session);
    emaUnlock((EMAElement)host);
}

static void reportBadMessage(
    cmTransGlobals *transGlobals,
    cmTransHost    *host,
    TRANSTYPE       type,
    BYTE           *buffer,
    int             bytesToDecode,
    void          **hMsgContext);

/**************************************************************************************
 * transQ931Handler
 *
 * Purpose: The callback routine for a Q.931 connection
 *
 * Input: standard input of TPKT callback
 *
 *
 **************************************************************************************/
void transQ931Handler(HTPKT tpkt,liEvents event,int length,int error,void*context)
{
    cmTransHost *host = (cmTransHost *)context;
    cmTransGlobals *transGlobals;

    if (!emaLock((EMAElement)host))
        return;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

    if (error)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "transQ931Handler got error on event=%d error=%d", event, error));

        /* in case of a connection error report it so the user may discard the session(s),
           however if the host is in race with an annex E host, no report is needed */
        if (event == liEvConnect)
        {
            if (transGlobals->sessionEventHandlers.cmEvTransConnectionOnSessionClosed)
            {
                cmTransSession *session = host->firstSession;
                int            hostNumOfLocks;

                /* unlock the host so it won't be locked before a session lock */
                hostNumOfLocks = emaPrepareForCallback((EMAElement)host);

                while (emaLock((EMAElement)session))
                {
                    int             numOfLocks;
                    cmTransSession *previousSession;
                    HATRANSSESSION  haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

                    if ( (session->Q931Connection == host) && (session->annexEConnection) )
                    {
                        logPrint(transGlobals->hLog, RV_ERROR,
                                (transGlobals->hLog, RV_ERROR,
                                "transQ931Handler will no report this error on host (%x-%d) since annex E (%x-%d) may yet secceed",
                                host, emaGetIndex((EMAElement)host),
                                session->annexEConnection, emaGetIndex((EMAElement)session->annexEConnection)));
                    }
                    else
                    {
                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "cmEvTransConnectionOnSessionClosed(hsSession = %d(%x), haSessiont=%x, type=cmTransQ931Conn)",
                                emaGetIndex((EMAElement)session),session, haTransSession));

                        numOfLocks = emaPrepareForCallback((EMAElement)session);
                        transGlobals->sessionEventHandlers.cmEvTransConnectionOnSessionClosed(
                                                        (HSTRANSSESSION) session,
                                                        haTransSession,
                                                        cmTransQ931Conn);
                        emaReturnFromCallback((EMAElement)session, numOfLocks);
                    }

                    previousSession = session;
                    session         = session->nextSession;

                    /* release the session */
                    emaUnlock((EMAElement)previousSession);
                }

                /* reinstate the host lock */
                emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
            }

            if ( (transGlobals->hostEventHandlers.cmEvTransHostClosed) && (host->reported) &&
                 (host->state != hostClosing) && (host->state != hostClosed) )
            {
                /* report the host close itself */
                int  hostNumOfLocks;
                BOOL wasConnected  = ( (host->state == hostConnected) ||
                                       (host->state == hostBusy) );
                HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

                host->state = hostClosing;
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransHostClosed(hsHost = %d(%x), haHost=%x, wasConnected = %d)",
                        emaGetIndex((EMAElement)host), host, haTransHost, wasConnected));

                hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                transGlobals->hostEventHandlers.cmEvTransHostClosed((HSTRANSHOST)host,
                                                                    haTransHost,
                                                                    wasConnected);
                emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
            }
        }
    }
    else
    {
        switch (event)
        {
            case liEvConnect:
            {
                TRANSERR res;

                /* if the host is in Idle state it does not wait a connection,
                   probably it was Droped by TransDrop. */
                if (host->state != hostIdle)
                {
                    /* a connection was established, notify the user */
                    host->state = hostConnected;

                    /* set the new local address of the host */
                    {
                        int socket = tpktGetSock(host->hTpkt);
                        host->localAddress.ip   = liGetSockIP(socket);
                        host->localAddress.port = liGetSockPort(socket);
                    }

                    /* register a new callback */
                    tpktRecvNotify(tpkt, transQ931Handler, host);

                    transReportConnect(host, NULL, TRUE);

                    /* check if we have a stored SETUP message (already sent on Annex E
                       connection) that needs to be sent */
                    if (host->firstMessage)
                    {
                        res = sendMessageOnHost(transGlobals, host);
                        if ( (res != cmTransOK) && (res != cmTransConnectionBusy) )
                        {
                            logPrint(transGlobals->hLog, RV_ERROR,
                                    (transGlobals->hLog, RV_ERROR,
                                    "transQ931Handler Failed on send for host %d(%x)",
                                    emaGetIndex((EMAElement)host), host));
                        }
                    }
                }
                break;
            }

            case liEvAccept:
            /* can't happen, treated by another callback */
            {
                break;
            }

            case liEvClose:
            case liEvRead:
            {
                int  pvtNode = -1;
                int  res;
                void *hMsgContext;
                int  offset = 0;
                BOOL sessionWasLocked = FALSE;

                cmTransSession *session = NULL;
                HATRANSSESSION haTransSession = NULL;

                /* try to read and decode a message */
                res = decodeIncomingMessage(tpkt, &length, &offset, context,
                                            &pvtNode, cmTransQ931Type, &hMsgContext);

                /* set the read event on tpkt */
                tpktRecvNotify(tpkt, transQ931Handler, host);

                if (res>=0)
                {
                    BOOL ok             = FALSE;
                    BOOL acceptedCall   = FALSE;

                    /* check that we can accept new messages on this host */
                    acceptedCall = canWeAcceptTheCall(transGlobals, host, pvtNode);

                    /* find the session of the call */
                    if (acceptedCall)
                        ok = findSessionByMessage(transGlobals,
                                                  host,
                                                  pvtNode,
                                                  (host->annexEUsageMode != cmTransNoAnnexE),
                                                  &session);

                    /* we have a decoded Q.931 message, process it */
                    if (ok)
                    {
                        int msgType;
                        int hostNumOfLocks;

                        /* if we found the session of the message, lock it and mark it */
                        if (session)
                        {
                            /* unlock the host so it won't be locked before the session */
                            hostNumOfLocks = emaPrepareForCallback((EMAElement)host);

                            sessionWasLocked = emaLock((EMAElement)session);

                            /* reinstate the host lock */
                            emaReturnFromCallback((EMAElement)host, hostNumOfLocks);

                            if (sessionWasLocked == FALSE)
                            {
                                /* This session was closed somewhere in-between - we
                                   should continue as if we have no session */
                                session = NULL;
                            }
                        }

                        /* check what type of Q.931 message we have here */
                        msgType = pvtGetChildTagByPath(transGlobals->hPvt, pvtNode, "message", 1);

                        /* determine that the TPKT host has won and got the first response */
                        if (session)
                            ok = determineWinningHost(transGlobals, session, (tpkt == NULL));

                        /* report the new session */
                        if ( (transGlobals->sessionEventHandlers.cmEvTransNewSession) &&
                             (session) &&
                             (ok) &&
                             (msgType == cmQ931setup) )
                        {
                            TRANSERR err;
                            int      cause;
                            INTPTR   reasonNameId;
                            int      sessNumOfLocks;

                            /* We have to cancel timer which was set at "accept" time - we
                               got a SETUP message on this call. */
                            if (host->connectTimer != (HTI)-1)
                            {
                                mtimerReset(transGlobals->hTimers, host->connectTimer);
                                host->connectTimer = (HTI)-1;
                            }


                            logPrint(transGlobals->hLog, RV_INFO,
                                    (transGlobals->hLog, RV_INFO,
                                    "cmEvTransNewSession(hAppTrans=%x, hAppATrans=%x, hsSession=%d-%x)",
                                    transGlobals, transGlobals->hAppATrans, emaGetIndex((EMAElement)session), session));

                            sessNumOfLocks = emaPrepareForCallback((EMAElement)session);
                            hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                            err = transGlobals->sessionEventHandlers.cmEvTransNewSession(
                                                (HAPPTRANS) transGlobals,
                                                (HAPPATRANS) transGlobals->hAppATrans,
                                                (HSTRANSSESSION)session,
                                                &haTransSession,
                                                pvtNode,
                                                &cause,
                                                &reasonNameId);
                            emaReturnFromCallback((EMAElement)session, sessNumOfLocks);
                            emaReturnFromCallback((EMAElement)host, hostNumOfLocks);

                            if (err != cmTransOK)
                            {
                                sendReleaseCompleteMessage(transGlobals,
                                                           host,
                                                           session->CRV,
                                                           session->callId,
                                                           cause,
                                                           reasonNameId);

                                /* Delete the Session */
                                if (!emaWasDeleted((EMAElement)session))
                                    cmTransCloseSession((HSTRANSSESSION)session);

                                logPrint(transGlobals->hLog, RV_ERROR,
                                        (transGlobals->hLog, RV_ERROR,
                                        "transQ931Handler new session was refused by user"));
                                ok = FALSE;
                            }
                            else
                            {
                                emaSetApplicationHandle((EMAElement)session, (void *)haTransSession);
                                session->reportedQ931Connect = TRUE;
                            }

                        }

                        /* process the message being received */
                        if(ok)
                        {
                            if (session)
                                /* hold the sending of tunneled messages until all incoming ones are reported */
                                session->holdTunneling = TRUE;
                            msgType = processQ931IncomingMessage(host, session, pvtNode);
                        }

                        if (ok)
                        {
                            /* report the message to the user */
                            if ( (session) && (!emaWasDeleted((EMAElement)session)) )
                            {
                                HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

                                /* report the new message on the session */
                                if (transGlobals->sessionEventHandlers.cmEvTransNewMessage)
                                {
                                    int sessNumOfLocks;
                                    logPrint(transGlobals->hLog, RV_INFO,
                                            (transGlobals->hLog, RV_INFO,
                                            "cmEvTransNewMessage(session = %d(%x), haTransSession=%x, type=cmTransQ931Type, pvtNode = %d, hMsgContext=%x)",
                                            emaGetIndex((EMAElement)session), session, haTransSession, pvtNode, hMsgContext));

                                    sessNumOfLocks = emaPrepareForCallback((EMAElement)session);
                                    hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                                    transGlobals->sessionEventHandlers.cmEvTransNewMessage(
                                                                (HSTRANSSESSION) session,
                                                                (HATRANSSESSION) haTransSession,
                                                                cmTransQ931Type,
                                                                pvtNode,
                                                                hMsgContext);
                                    emaReturnFromCallback((EMAElement)session, sessNumOfLocks);
                                    emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
                                }

                                /* release the hold from sending tunneled messages */
                                session->holdTunneling = FALSE;

                                /* check if we need to send facility for the newly sent tunneled messages */
                                if (!emaWasDeleted((EMAElement)session))
                                    initiateTunnelingFacility(transGlobals, session, host);
                            }
                            else
                            if (!session)
                            {
                                HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

                                /* report the new message on the host */
                                if (transGlobals->hostEventHandlers.cmEvTransHostNewMessage)
                                {
                                    logPrint(transGlobals->hLog, RV_INFO,
                                            (transGlobals->hLog, RV_INFO,
                                            "cmEvTransHostNewMessage(hsHost = %d(%x), haHost=%x, type=cmTransQ931Conn, pvtNode = %d)",
                                            emaGetIndex((EMAElement)host), host, haTransHost, pvtNode));

                                    hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                                    transGlobals->hostEventHandlers.cmEvTransHostNewMessage(
                                                                (HSTRANSHOST) host,
                                                                haTransHost,
                                                                cmTransQ931Type,
                                                                pvtNode,
                                                                hMsgContext);
                                    emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
                                }
                            }
                        }
                    }
                    else
                    /* this message has no session, if it's possible
                       terminate the connection, i.e. if it's not a multiplexed host,
                       or alternatively it's a multiplexed host that may be closed on
                       no sessions, and indeed none is connected to it */
                    if ( (acceptedCall) && 
                         ( 
                           ((!host->isMultiplexed) || (!host->remoteIsMultiplexed)) ||
                           ((!host->firstSession) && ((host->closeOnNoSessions) || (host->remoteCloseOnNoSessions)))
                         ) 
                       )
                    {
                        /* report bad message */
                        reportBadMessage(transGlobals, host, cmTransQ931Type, NULL, 0, hMsgContext);
                        /* don't force a close on the host
                        sendReleaseCompleteMessage(transGlobals, host, session->CRV, session->callId, -1,
                            __q931(invalidCID));
                        event = liEvClose;*/
                    }

                    /* get rid of the decoded message */
                    if (pvtNode >= 0)
                        pvtDelete(transGlobals->hPvt, pvtNode);
                }

                if ( (event == liEvClose) && (!emaWasDeleted((EMAElement)host)) && (host->state != hostClosed) )
                {
                    /* the host connection was closed, report it for non annex E hosts */
                    if ( (transGlobals->hostEventHandlers.cmEvTransHostClosed) &&
                         (host->reported) && (host->state != hostClosing) &&
                         (host->annexEUsageMode == cmTransNoAnnexE) )
                    {
                        BOOL wasConnected       = ( (host->state == hostConnected) ||
                                                    (host->state == hostBusy) );
                        HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);
                        int hostNumOfLocks;

                        host->state = hostClosing;
                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "cmEvTransHostClosed(hsHost = %d(%x), haHost=%x, wasConnected = %d)",
                                emaGetIndex((EMAElement)host), host, haTransHost, wasConnected));

                        hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                        transGlobals->hostEventHandlers.cmEvTransHostClosed((HSTRANSHOST)host,
                                                                            haTransHost,
                                                                            wasConnected);
                        emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
                    }
                    else
                    {
                        if (!emaWasDeleted((EMAElement)host))
                        {
                            host->state = hostClosed;
                            transHostClose((HSTRANSHOST)host, TRUE);
                        }
                    }
                }

                if (sessionWasLocked)
                    emaUnlock((EMAElement)session);

                break;
            }

            case liEvWrite:
            {
                TRANSERR res;

                /* for TPKT the last send was completed */
                /* remove the message from the pool (for annex E
                   we must resend the last message that got WOULDBLOCK error) */
                if (host->annexEUsageMode == cmTransNoAnnexE)
                    extractMessageFromPool(transGlobals, host, FALSE);

                /* mark the host as not busy and try sending the remaining messages */
                host->state = hostConnected;
                res = sendMessageOnHost(transGlobals, host);
                if ( (res != cmTransOK) && (res != cmTransConnectionBusy) )
                {
                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "transQ931Handler Failed on send for host %d(%x)",
                            emaGetIndex((EMAElement)host), host));
                }
                break;
            }
        }
    }

    /* unlock and release the marked host */
    emaUnlock((EMAElement)host);
}

/**************************************************************************************
 * connectH245Connection
 *
 * Purpose: call back rotine that simulates a connect from the network on the H.245
 *          connection. It is used when a dummy control session is built which does
 *          not actually connect to the network.
 *
 * Input:   context  - the handle to the session on which we need to report the connect.
 *
 * Output:  None.
 *
 **************************************************************************************/
void RVCALLCONV connectH245Connection(void* context)
{
    cmTransSession *session = (cmTransSession *)context;

    if (emaLock((EMAElement)session))
    {
        cmTransHost *host = session->H245Connection;
        /* retrieve the transport module global area */
        cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

        /* release the timer */
        if (session->h245ConnectTimer != (HTI)-1)
        {
            mtimerReset(transGlobals->hTimers, session->h245ConnectTimer);
            session->h245ConnectTimer = (HTI)-1;
        }

        if (emaLock((EMAElement)host))
        {
            /* if it's an existing and connected host, just report it and return OK */
            if (host->state == hostConnecting)
            {
               /* all ok we can use that connection */
               host->state = hostConnected;

               /* the H.245 connection is established, notify the user */
               transReportConnect(host, NULL, TRUE);
            }
            emaUnlock((EMAElement)host);
        }
        emaUnlock((EMAElement)session);
    }
}

/**************************************************************************************
 * connectQ931Connection
 *
 * Purpose: call back rotine that simulates a connect from the network on the Q.931
 *          connection. It is used when a multiplexed host is already connected and we
 *          just want to simulate its connection event so the call signaling procedure
 *          will proceed.
 *
 * Input:   context  - the handle to the session on which we need to report the connect.
 *
 * Output:  None.
 *
 **************************************************************************************/
void RVCALLCONV connectQ931Connection(void* context)
{
    cmTransSession *session = (cmTransSession *)context;
    cmTransHost *host = session->Q931Connection;
    /* retrieve the transport module global area */
    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* release the timer */
    if (session->muxConnectTimer != (HTI)-1)
    {
        mtimerReset(transGlobals->hTimers, session->muxConnectTimer);
        session->muxConnectTimer = (HTI)-1;
    }

    if (emaLock((EMAElement)host))
    {
        /* if it's an existing and connected host, just report it and return OK */
        if ( (host->state == hostConnected) || (host->state == hostBusy) )
        {
            /* report to the session that the connection is established, no need
               to do anything with the annex E host or initiate a connect on the
               TPKT one */
            transReportConnect(host, session, TRUE);
        }
        emaUnlock((EMAElement)host);
    }
}

/**************************************************************************************
 * connectAnnexEConnection
 *
 * Purpose: call back rotine that simulates a connect from the network on the annex E
 *          connection. It is used when an annex E host already exists and we
 *          just want to simulate its connection event so the call signaling procedure
 *          will proceed.
 *
 * Input:   context  - the handle to the session on which we need to report the connect.
 *
 * Output:  None.
 *
 **************************************************************************************/
void RVCALLCONV connectAnnexEConnection(void* context)
{
    cmTransSession *session = (cmTransSession *)context;
    cmTransHost *host = session->annexEConnection;
    /* retrieve the transport module global area */
    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* release the timer */
    if (session->muxConnectTimer != (HTI)-1)
    {
        mtimerReset(transGlobals->hTimers, session->muxConnectTimer);
        session->muxConnectTimer = (HTI)-1;
    }

    if (emaLock((EMAElement)host))
    {
        /* if it's an existing and connected host, just report it and return OK */
        if ( (host->state == hostConnected) || (host->state == hostBusy) )
        {
            /* report to the session that the connection is established, no need
               to do anything with the annex E host or initiate a connect on the
               TPKT one */
            transReportConnect(host, session, TRUE);
        }
        emaUnlock((EMAElement)host);
    }
}


/**************************************************************************************
 * transHostConnect
 *
 * Purpose: Starts the process of connecting on the given host connection.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          type            - The type of the connection (annex E or TPKT)
 *
 * Output:  None.
 *
 * Returned Value:  cmTransWouldBlock - In case that a connect procedure was instigated but
 *                                      not yet completed.
 *                  cmTransOK         - In case that the connection is already opened.
 *                  cmTransErr        - couldn't create the hosts
 **************************************************************************************/
TRANSERR transHostConnect(IN HSTRANSSESSION hsTransSession,
                          IN TRANSINTCONNTYPE  type)
{
    cmTransSession          *session   = (cmTransSession *)hsTransSession;
    cmTransHost             *host      = NULL;
    cmTransHost             *extraHost = NULL;
    cmTransGlobals          *transGlobals;
    LPMTIMEREVENTHANDLER    cbRoutine = NULL;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

    switch (type)
    {
        case cmTransTPKTConn:
        {
            host        = session->Q931Connection;
            cbRoutine   = connectQ931Connection;
            break;
        }

        case cmTransAnnexEConn:
        {
            host        = session->annexEConnection;
            cbRoutine   = connectAnnexEConnection;
            break;
        }

        case cmTransTPKTAnnexECompetition:
        {
            host        = session->Q931Connection;
            extraHost   = session->annexEConnection;
            cbRoutine   = connectQ931Connection;
            type        = cmTransTPKTConn;
            break;
        }
    }

    if (emaLock((EMAElement)host))
    {
        /* mark the host, it might get deleted when reporting its connect */
        TRANSERR reply = cmTransOK;

        /* if it's an existing and connected host, just report it and return OK */
        if ( (host->state == hostConnected) || (host->state == hostBusy) )
        {
            /* report to the session that the connection is established, no need
               to do anything with the annex E host or initiate a connect on the
               TPKT one */

            /* release previous timer, if exists */
            if (session->muxConnectTimer != (HTI)-1)
            {
                mtimerReset(transGlobals->hTimers, session->muxConnectTimer);
                session->muxConnectTimer = (HTI)-1;
            }

            /* set the timer with a CB routine according to the connecting host type */
            session->muxConnectTimer = mtimerSet(transGlobals->hTimers,
                                                 cbRoutine,
                                                 session,
                                                 0);
            emaUnlock((EMAElement)host);
            return cmTransOK;
        }

        /* if we have an annex E but not a connected TPKT,
           report that the annex E host is connected */
        if (emaLock((EMAElement)extraHost))
        {
            /* report to the session that the connection is established */

            /* release previous timer, if exists */
            if (session->muxConnectTimer != (HTI)-1)
            {
                mtimerReset(transGlobals->hTimers, session->muxConnectTimer);
                session->muxConnectTimer = (HTI)-1;
            }
            /* set the timer with a CB routine according to the connecting host type */
            session->muxConnectTimer = mtimerSet(transGlobals->hTimers,
                                                 connectAnnexEConnection,
                                                 session,
                                                 0);
            emaUnlock((EMAElement)extraHost);
        }

        /* TPKT host is not connected or approved,
           we need to initiate a connect process for it */
        {
            /* allocate a new tpkt element for the connection and connect it*/
            if (host->state == hostIdle)
            {
                getGoodAddress(transGlobals,
                               host->hTpkt,
                               NULL,
                               cmTransQ931Conn,
                               &host->localAddress);

                host->hTpkt = tpktOpen( transGlobals->tpktCntrl,
                                        host->localAddress.ip,
                                        host->localAddress.port,
                                        tpktClient,
                                        transQ931Handler,
                                        (void *)host);
                if (host->hTpkt != NULL)
                    host->state = hostConnecting;
                else
                    reply = cmTransErr;
            }

            if (host->state == hostConnecting)
            {

                if (transGlobals->hostEventHandlers.cmEvTransHostConnecting)
                {
                    int         numOfLocks;
                    HATRANSHOST haTransHost =
                                 (HATRANSHOST)emaGetApplicationHandle(((EMAElement)host));

                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransHostConnecting(hsHost = %d(%x), haHost=%x, type=cmTransQ931Conn, address = (ip:%x,port:%d))",
                            emaGetIndex((EMAElement)host),
                            host,
                            haTransHost,
                            host->remoteAddress.ip,
                            host->remoteAddress.port));

                    numOfLocks = emaPrepareForCallback((EMAElement)host);
                    reply = transGlobals->hostEventHandlers.cmEvTransHostConnecting(
                                            (HSTRANSHOST)host,
                                            haTransHost,
                                            cmTransQ931Conn,
                                            &host->remoteAddress);
                    emaReturnFromCallback((EMAElement)host, numOfLocks);
                    host->reported = TRUE;
                }

                if (reply==cmTransOK)
                    tpktConnect(host->hTpkt, host->remoteAddress.ip, host->remoteAddress.port);
                else
                {
                    if (reply == cmTransIgnoreMessage)
                    {
                        logPrint(transGlobals->hLog, RV_INFO,
                                 (transGlobals->hLog, RV_INFO,
                                 "transHostConnect connect on Q.931 TPKT refused by user"));
                    }

                    /* Make sure we're not in some kind of an error state */
                    reply = cmTransOK;
                }
            }
        }

        emaUnlock((EMAElement)host);

        /* the process has begun but not yet ended, inform the user of that */
        if (reply == cmTransOK)
            return cmTransWouldBlock;
    }

    return cmTransErr;
}

/**************************************************************************************
 * getMsgDataFromRPool
 *
 * Purpose: reads the encoded message from the rpool, gets its size and its CRV
 *          from the message header and then extracts from the encoded message itself its type.
 *
 * Input:   hRpool  - Handle to the messages rpool.
 *          message - the message in the rpool
 *          buffer  - a buffer to put the message in.
 *
 * Output:  CRV     - The crv of the messsage, as was put on message header in the rpool.
 *          msgSize - The size of the encoded message (including its headers).
 *
 * Returned Value:  mstType - The type of the emssage as extracted from the encoded message
 *                            [the type is in the first byte after the CRV, and the CRV length
 *                             is in the second byte - offset 1].
 **************************************************************************************/
int getMsgDataFromRPool(HRPOOL hRpool, void *message, BYTE *buffer, UINT16 *CRV, int *msgSize)
{
    BYTE *encodedMsg;

    /* extract the message from the rpool to the buffer */
    rpoolCopyToExternal(hRpool,
                        buffer,
                        message,
                        0,
                        *msgSize);

    /* get the messsage size (including headers) */
    *msgSize = rpoolChunkSize(hRpool, message);

    /* extract the CRV from the message header */
    memcpy(CRV, &buffer[MSG_HEADER_SIZE], sizeof(*CRV));

    /* get the encoded message (without headers) */
    encodedMsg = &buffer[MSG_HEADER_SIZE + TPKT_HEADER_SIZE];

    /* return the message type from the necoded message */
    return encodedMsg[2 + encodedMsg[1]];

}
/**************************************************************************************
 * sendMessageOnHost
 *
 * Purpose: send a message on the given host according to its communication protocol.
 *
 * Input:   transGlobal - The global variables of the module.
 *          host        - The host on which to send the messages that await to be sent.
 *
 *
 * Output:  None.
 *
 * Returned Value:  cmTransOK             - All is ok.
 *                  cmTransConnectionBusy - The connection is still sending previous messages.
 **************************************************************************************/
TRANSERR sendMessageOnHost(cmTransGlobals *transGlobals, cmTransHost *host)
{
    int  bytes = 0;
    void *nextMessage;

    /* check if the host is ready for send */
    if (host->state == hostConnected)
    {
        /* go over all stored messages for sending */
        nextMessage = host->firstMessage;
        while (nextMessage)
        {
            switch (host->type)
            {
                case cmTransQ931Conn:
                {
                    /* send according to the type of communications, TPKT or annex E */
                    if (host->annexEUsageMode != cmTransNoAnnexE)
                    {
                        BYTE *buffer;
                        UINT16 CRV;
                        int msgType;
                        int msgSize;
                        annexEStatus eStat;

                        getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);
                        msgSize = transGlobals->codingBufferSize;
                        msgType = getMsgDataFromRPool(transGlobals->messagesRPool,
                                                      nextMessage,
                                                      buffer,
                                                      &CRV,
                                                      &msgSize);

                        /* remove the headers size */
                        msgSize = msgSize - (TPKT_HEADER_SIZE + MSG_HEADER_SIZE);

                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "sendMessageOnHost sending on annex E, size=%d",msgSize));

                        eStat = annexESendMessage(transGlobals->annexECntrl,
                                                  host->remoteAddress.ip,
                                                  host->remoteAddress.port,
                                                  CRV,
                                                  &buffer[TPKT_HEADER_SIZE + MSG_HEADER_SIZE],
                                                  msgSize,
                                                  (msgType != cmQ931releaseComplete),
                                                  ((msgType == cmQ931setup) || (msgType == cmQ931statusEnquiry)),
                                                  TRUE);

                        if (eStat == annexEStatusWouldBlock)
                            bytes = 0;
                        else
                        if (eStat != annexEStatusNormal)
                        {
                            logPrint(transGlobals->hLog, RV_ERROR,
                                    (transGlobals->hLog, RV_ERROR,
                                    "sendMessageOnHost failed on annex E send err=%d",eStat));
                        }
                        else
                            bytes = msgSize;
                        break;
                    }
                    else
                    {
                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "sendMessageOnHost sending on TPKT"));
                        bytes = tpktSendFromRpool(host->hTpkt,
                                                  transGlobals->messagesRPool,
                                                  nextMessage,
                                                  MSG_HEADER_SIZE,
                                                  transGlobals->tablesLock);
                    }
                    break;
                }

                case cmTransH245Conn:
                {
                    bytes = tpktSendFromRpool(host->hTpkt,
                                              transGlobals->messagesRPool,
                                              nextMessage,
                                              MSG_HEADER_SIZE,
                                              transGlobals->tablesLock);
                    break;
                }
            }

            /* check if the send was successful */
            if (bytes > 0)
            {
                nextMessage = extractMessageFromPool(transGlobals, host, FALSE);
            }
            else
            if (bytes == 0)
            {
                /* we need to wait for the send to complete */
                host->state = hostBusy;
                nextMessage = NULL;
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "sendMessageOnHost would block on send for host %d(%x)",
                        emaGetIndex((EMAElement)host), host));
                return cmTransConnectionBusy;
            }
            else
            {
                nextMessage = NULL;
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "sendMessageOnHost got error on send for host %d(%x)",
                        emaGetIndex((EMAElement)host), host));
                return cmTransErr;
            }
        }
    }
    else
    {
        /* the host is either temporarily unavaliable, or is closing */
        if ((host->state == hostClosing) || (host->state == hostClosed))
        {
            /* this host is closing, and there is no hope of ever sending messages on it.
            remove all the messages on it, just to make sure they are freed */
            nextMessage = host->firstMessage;
            while (nextMessage)
            {
                nextMessage = extractMessageFromPool(transGlobals, host, FALSE);
            }
            /* no more messages on host */
            host->firstMessage = NULL;
            return cmTransConnectionClosed;
        }
        /* it is just busy now. keep the messages for later */
        return cmTransConnectionBusy;
    }

    return cmTransOK;
}

static void reportBadMessage(
    cmTransGlobals *transGlobals,
    cmTransHost    *host,
    TRANSTYPE       type,
    BYTE           *buffer,
    int             bytesToDecode,
    void          **hMsgContext)
{
    HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

    /* see if we know the session of this message */
    if ((host->firstSession != NULL) &&
        (((type == cmTransQ931Type) && (!host->firstSession->isMultiplexed)) ||
         (host->type == cmTransH245Conn)))
    {
        /* Seems like its an H245 or a non-multiplexed Q931 message. Session is known! */
        if (transGlobals->sessionEventHandlers.cmEvTransBadMessage)
        {
            int numOfLocks;
            EMAElement session = host->firstSession;
            HATRANSSESSION haTransSession;

            haTransSession = (HATRANSSESSION)emaGetApplicationHandle(session);
            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransBadMessage(hsSession = %d(%p), haSession=%p, type=%d,outgoing=FALSE)",
                    emaGetIndex((EMAElement)session), session, haTransSession, type));

            numOfLocks = emaPrepareForCallback((EMAElement)host);
            transGlobals->sessionEventHandlers.cmEvTransBadMessage(
                        (HSTRANSSESSION) session,
                        haTransSession,
                        type,
                        buffer,
                        bytesToDecode,
                        FALSE);
            emaReturnFromCallback((EMAElement)host, numOfLocks);
        }
    }
    else
    {
        /* We don't really know the session - use host callback instead */
        if (transGlobals->hostEventHandlers.cmEvTransHostBadMessage)
        {
            int numOfLocks;

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransHostBadMessage(hsHost = %d(%p), haHost=%p, type=%d, outgoing=FALSE)",
                    emaGetIndex((EMAElement)host), host, haTransHost, type));

            numOfLocks = emaPrepareForCallback((EMAElement)host);
            transGlobals->hostEventHandlers.cmEvTransHostBadMessage(
                                            (HSTRANSHOST) host,
                                            haTransHost,
                                            type,
                                            buffer,
                                            bytesToDecode,
                                            FALSE,
                                            hMsgContext);
            emaReturnFromCallback((EMAElement)host, numOfLocks);
        }
    }
}



/**************************************************************************************
 * decodeIncomingMessage
 *
 * Purpose: tries to receive and decode an incoming message
 *
 * Input:   tpkt    - a handle to the TPKT connection
 *          length  - the length of the received message (0 if no buffers for reading the message)
 *          offset  - Where to start the decoding process in the encoded buffer
 *          context - The context associated with the connection , i.e. the host element
 *          node    - The pvt node into which to decode the message
 *          type    - The type of the message.
 *
 *
 * Output:  hMsgContext - An external context associated with the message.
 *
 * Returned Value:  number of bytes decoded, or <=0 in case of trouble.
 **************************************************************************************/
int  decodeIncomingMessage( HTPKT       tpkt,
                            int         *length,
                            int         *offset,
                            void        *context,
                            int         *node,
                            TRANSTYPE   type,
                            void        **hMsgContext)
{
    cmTransGlobals *transGlobals;
    cmTransHost *host = (cmTransHost *)context;
    HATRANSHOST haTransHost;
    int bytesToDecode;
    int pvtNode;
    BYTE *buffer;
    HPST syntax;


    TRANSERR decTransRes = cmTransOK;
    int      decoded;
    int      decResult = 0;

    *hMsgContext = NULL;
    *node = -1;

    if (!host)
    {
        *length = 0;
        return decResult;
    }

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

    /* retrieve the host's application handle */
    haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

    /* initialize the number of bytes to decode and the number already decoded */
    decoded         = 0;
    bytesToDecode   = *length;

    /* if no message, supply it through tpktRecvIntoRpool */
    if ( (!*length) && (tpkt) )
    {
        if (!host->incomingMessage)
        {
            meiEnter(transGlobals->tablesLock);
            host->incomingMessage = rpoolAlloc(transGlobals->messagesRPool, TRANS_RPOOL_CHUNK_SIZE);
            {
                INT32 poolSize, availableSize, allocatedSize;

                if (rpoolStatistics(transGlobals->messagesRPool,
                                    &poolSize,
                                    &availableSize,
                                    &allocatedSize))
                {
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "rpoolAlloc[decodeIncomingMessage] statistics: max=%d, available=%d, allocated=%d",
                            poolSize,availableSize, allocatedSize));
                }
            }
            meiExit(transGlobals->tablesLock);
        }

        bytesToDecode = tpktRecvIntoRpool(tpkt,
                                          transGlobals->messagesRPool,
                                          host->incomingMessage,
                                          transGlobals->tablesLock);
    }

    if (bytesToDecode > 0)
    {
        getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);

        /* check if the buffer is sufficient for the message */
        if (transGlobals->codingBufferSize < bytesToDecode)
        {
            *length = 0;
            decoded = -1;
            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "decodeIncomingMessage has too small a buffer. bufferSize=%d length=%d",
                    transGlobals->codingBufferSize, bytesToDecode));
        }
        else
        {
            /* copy the message to the buffer */
            rpoolCopyToExternal(transGlobals->messagesRPool,
                                buffer,
                                host->incomingMessage,
                                *offset,
                                bytesToDecode);

            /* decode the message */
            if (type == cmTransQ931Type)
                syntax = transGlobals->synProtQ931;
            else
                syntax = transGlobals->synProtH245;
            pvtNode   = pvtAddRoot(transGlobals->hPvt, syntax, 0, NULL);

            if (transGlobals->hostEventHandlers.cmEvTransNewRawMessage)
            {
                int numOfLocks;

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransNewRawMessage(hsHost = %d(%p), haHost=%p, type = %d pvt=%d bytesToDecode=%d)",
                        emaGetIndex((EMAElement)host), host, haTransHost, type, pvtNode, bytesToDecode));

                numOfLocks = emaPrepareForCallback((EMAElement)host);
                decTransRes = transGlobals->hostEventHandlers.cmEvTransNewRawMessage(
                                            (HSTRANSHOST) host,
                                            haTransHost,
                                            type,
                                            pvtNode,
                                            &buffer[decoded],
                                            bytesToDecode,
                                            &decoded,
                                            hMsgContext);
                emaReturnFromCallback((EMAElement)host, numOfLocks);
            }
            else
            {
                decResult = cmEmDecode(transGlobals->hPvt, pvtNode, &buffer[decoded], bytesToDecode, &decoded);
            }

            /* check if we need to ignore the message */
            if (decTransRes == cmTransIgnoreMessage)
            {
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "decodeIncomingMessage was instructed to ignore the message"));
                if (pvtNode >= 0)
                    pvtDelete(transGlobals->hPvt, pvtNode);
                *length = 0;
                decoded = -1;
            }
            else
            if ( (decResult < 0) || (decTransRes != cmTransOK) )
            {
                decoded = -1;
                *length = 0;
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "decodeIncomingMessage failed on decoding"));
                if (pvtNode >= 0)
                    pvtDelete(transGlobals->hPvt, pvtNode);
                /* We can't decode this message - report it */
                reportBadMessage(transGlobals, host, type, buffer, bytesToDecode, hMsgContext);
            }
            else
            {
                *node = pvtNode;
                *length = bytesToDecode - decoded;
                offset += decoded;
            }
        }

        /* exit procedures */
        if (*length <= 0)
        {
            /* get rid of the rpool message */
            meiEnter(transGlobals->tablesLock);
            rpoolFree(transGlobals->messagesRPool,
                      host->incomingMessage);
            host->incomingMessage = NULL;
            {
                INT32 poolSize, availableSize, allocatedSize;

                if (rpoolStatistics(transGlobals->messagesRPool,
                                    &poolSize,
                                    &availableSize,
                                    &allocatedSize))
                {
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "rpoolFree[decodeIncomingMessage] statistics: max=%d, available=%d, allocated=%d",
                            poolSize,availableSize, allocatedSize));
                }
            }
            meiExit(transGlobals->tablesLock);
        }
    }
    else
    if (bytesToDecode < 0)
    {
        if (host->incomingMessage)
        {
            /* get rid of the rpool message */
            meiEnter(transGlobals->tablesLock);
            rpoolFree(transGlobals->messagesRPool,
                      host->incomingMessage);
            host->incomingMessage = NULL;
            {
                INT32 poolSize, availableSize, allocatedSize;

                if (rpoolStatistics(transGlobals->messagesRPool,
                                    &poolSize,
                                    &availableSize,
                                    &allocatedSize))
                {
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "rpoolFree[decodeIncomingMessage-err] statistics: max=%d, available=%d, allocated=%d",
                            poolSize,availableSize, allocatedSize));
                }
            }
            meiExit(transGlobals->tablesLock);
        }
        *length = 0;
        decoded = -1;
    }
    else
    {
        *length = 0;
        decoded = -1;
    }

    return decoded;
}


/**************************************************************************************
 * processQ931IncomingMessage
 *
 * Purpose: handles incoming messages:
 *          extract the states of the faststart and tunneling (including parallel one).
 *          updates the multiplexing parameters of the host
 *          decides whether to report the openning of a H.245 connection
 *          decides whether to initiate a connect for H.245
 *          extract all kinds of tunneled messages from the message
 *
 * Input:   host    - The host on which the message arrived
 *          session - The session on which the message arrived (i.e. the call)
 *          pvtNode - The decoded message itself
 *
 * Output:  None.
 *
 * Return value: msgType - the message type.
 *
 **************************************************************************************/
int processQ931IncomingMessage(cmTransHost *host, cmTransSession *session, int pvtNode)
{
    cmTransGlobals  *transGlobals;
    int             messageBodyNode;
    int             msgType;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

    /* check what type of Q.931 message we have here */
    msgType = pvtGetChildTagByPath(transGlobals->hPvt, pvtNode, "message", 1);
    if (msgType < 0)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "processQ931IncomingMessage failed to get message type tag"));
        return -1;
    }

    /* position on the UU-IE part of the message */
    __pvtGetNodeIdByFieldIds(   messageBodyNode,
                                transGlobals->hPvt,
                                pvtNode,
                                {   _q931(message)
                                    _anyField
                                    _q931(userUser)
                                    _q931(h323_UserInformation)
                                    _q931(h323_uu_pdu)
                                    LAST_TOKEN
                                });
    /* if the message doesn't have UU-IE part, we have nothing to do with it, just
       send it */
    if (messageBodyNode < 0)
        return msgType;

    /***********************************************************************/
    /* get the multipleCalls and maintainConnection parameters (multiplexing stuff) */
    /***********************************************************************/
    getMultiplexedParams(transGlobals, host, pvtNode, msgType);

    /***********************************************************************/
    /* check the fast start state of the session */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        setTheFastStartStateByMessage(transGlobals, session, messageBodyNode, msgType);

    /***********************************************************************/
    /* check the tunneling state of the session  */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        setTheTunnelingStateByMessage(transGlobals, session, messageBodyNode, msgType);

    /***********************************************************************/
    /* check the parallel tunneling state of the session  */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        setTheParallelTunnelingStateByMessage(transGlobals,
                                              session,
                                              msgType,
                                              messageBodyNode,
                                              FALSE /* incoming */);

    /***********************************************************************/
    /* handling whether to start listenning or establish connection for H.245 */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        if (!session->notEstablishControl)
            determineIfToOpenH245(FALSE /* incoming message */,
                                  transGlobals,
                                  session,
                                  messageBodyNode,
                                  msgType);

    /***********************************************************************/
    /* handle reporting of a new H.245 connection to the user    */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        if (!session->notEstablishControl)
            reportH245(FALSE /* incoming message */,
                       transGlobals,
                       session,
                       host,
                       messageBodyNode,
                       msgType);

    /***********************************************************************/
    /* handle extracting the tunneled H.245 messages from the Q.931 message    */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
    {
        if (!session->notEstablishControl)
            extractH245Messages(transGlobals, session, host, messageBodyNode, msgType);

        if (!emaWasDeleted((EMAElement)session))
            extractH450Messages(transGlobals, session, pvtNode, msgType);
        if (!emaWasDeleted((EMAElement)session))
            extractAnnexMMessages(transGlobals, session, messageBodyNode, msgType);
        if (!emaWasDeleted((EMAElement)session))
            extractAnnexLMessages(transGlobals, session, messageBodyNode, msgType);
    }

    return msgType;
}


/**************************************************************************************
 * connectH245
 *
 * Purpose: connects if the host state permits, a H.245 connection.
 *
 * Input:   transGlobal - The global variables of the module.
 *          session     - The session for which the connection is done
 *          msgType     - The type of the message that is being checked
 *
 * Output:  None.
 *
 **************************************************************************************/
void connectH245(cmTransGlobals *transGlobals,
                 cmTransSession *session,
                 int            msgType)
{
    if (!emaLock((EMAElement)session->H245Connection))
        return;

    /* if host is not connecting or connected yet we may start to connect */
    if ((session->H245Connection->state == hostIdle) ||
        (session->H245Connection->state == hostListenning) ||
        (session->H245Connection->dummyHost) )
    {
        /* Check if we have a remote address to open to,
           if we do continue with connecting, if not we need to issue
           a startH245 FACILIRY message */
        if (session->H245Connection->remoteAddress.ip != 0)
        {
            TRANSERR reply = cmTransOK;

            /* start a connect process */
            if (transGlobals->hostEventHandlers.cmEvTransHostConnecting && !session->H245Connection->dummyHost)
            {
                int         numOfLocks;
                HATRANSHOST haTransHost =
                             (HATRANSHOST)emaGetApplicationHandle(((EMAElement)session->H245Connection));

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransHostConnecting(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d))",
                        emaGetIndex((EMAElement)session->H245Connection),
                        session->H245Connection,
                        haTransHost,
                        session->H245Connection->remoteAddress.ip,
                        session->H245Connection->remoteAddress.port));

                numOfLocks = emaPrepareForCallback((EMAElement)session->H245Connection);
                reply = transGlobals->hostEventHandlers.cmEvTransHostConnecting(
                                        (HSTRANSHOST)session->H245Connection,
                                        haTransHost,
                                        cmTransH245Conn,
                                        &session->H245Connection->remoteAddress);
                emaReturnFromCallback((EMAElement)session->H245Connection, numOfLocks);
                session->H245Connection->reported = TRUE;
            }

            if (reply == cmTransOK)
            {
	void * appHandle;
	int appIp;
	appHandle = emaGetApplicationHandle((EMAElement)session);
	if(appHandle)
	{
		appIp = getAppIp(appHandle);
	}
                /* do the actuall connect */
                session->H245Connection->hTpkt =
                            tpktOpen(   transGlobals->tpktCntrl,
           /*                             transGlobals->localIPAddress , */ /* Ashish dmr */
					htonl(appIp),
					0,
                                        tpktClient,
                                        transH245Handler,
                                        (void *)session->H245Connection);
                if (session->H245Connection->hTpkt)
                    tpktConnect(session->H245Connection->hTpkt,
                                session->H245Connection->remoteAddress.ip,
                                session->H245Connection->remoteAddress.port);
            }
            else
                session->H245Connection->hTpkt = NULL;

            /* report failures */
            if ( (!session->H245Connection->hTpkt) && (reply != cmTransIgnoreMessage) )
            {
                logPrint(transGlobals->hLog, RV_ERROR,
                         (transGlobals->hLog, RV_ERROR,
                         "connectH245 failed to initiate connect on H.245 TPKT"));
            }
            else
            if (reply == cmTransIgnoreMessage)
            {
                logPrint(transGlobals->hLog, RV_INFO,
                         (transGlobals->hLog, RV_INFO,
                         "connectH245 connect on H.245 TPKT refused by user"));
            }
            else
            {
                /* mark the host as connecting if it was closed.
                   If it was listenning, mark it as Listenning-Connecting,
                   i.e. we both trying to connect and listen
                   "and may the best man win" */
                if (session->H245Connection->state == hostListenning)
                    session->H245Connection->state = hostListenningConnecting;
                else
                    session->H245Connection->state = hostConnecting;
            }
        }
        else
        {
            /* If this was a CONNECT message and still no address was given,
               we need to send facility with startH245 */
            if (
                 (msgType == cmQ931connect) &&
                 ((session->openControlWasAsked) || (session->switchToSeparateWasAsked))
               )
                session->needToSendFacilityStartH245 = TRUE;
        }
    }

    emaUnlock((EMAElement)session->H245Connection);

}


/**************************************************************************************
 * reportH245Establishment
 *
 * Purpose: if not done so yet, report to the user about an active H.245 connection.
 *
 * Input:   transGlobal - The global variables of the module.
 *          session     - The session for which the connection is done
 *          host        - The host for which the connection is done
 *
 * Output:  None.
 *
 **************************************************************************************/
void reportH245Establishment(cmTransGlobals *transGlobals, cmTransSession *session, cmTransHost* host)
{
    if ( (!session->reportedH245Connect) &&
         (session->h245Stage != cmTransH245Stage_noH245) )
    {
        HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

        session->reportedH245Connect = TRUE;

        if (transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection)
        {
            int numOfLocks, hostNumOfLocks;

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransSessionNewConnection(hsSession = %d(%x), haSession=%x, type=cmTransH245Conn)",
                    emaGetIndex((EMAElement)session), session, haTransSession));

            hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
            numOfLocks = emaPrepareForCallback((EMAElement)session);
            transGlobals->sessionEventHandlers.cmEvTransSessionNewConnection(
                                    (HSTRANSSESSION) session,
                                    haTransSession,
                                    cmTransH245Conn);
            emaReturnFromCallback((EMAElement)session, numOfLocks);
            emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
        }
    }
}

/**************************************************************************************
 * annexEEvNewMessage
 *
 * Purpose: Callback for Annex E upon receiving new incoming message.
 *          The call back looks for the host (or create one if it's new) and
 *          then calls the regular event handler (of TPKT) with event liEvRead.
 *
 * Input:   hAnnexE     - The annex E module handle
 *          hAppAnnexE  - The application handle of the annex E module (actualy
 *                        our very own transGlobals structure).
 *          nIP         - The remote ip whence the message arrived.
 *          nPort       - The remote port whence the message arrived.
 *          pMessage    - The encoded message itself
 *          nSize       - The size of the message.
 *
 * Output:  None.
 *
 **************************************************************************************/
annexEStatus RVCALLCONV annexEEvNewMessage(
    IN  HANNEXE     hAnnexE,
    IN  HAPPANNEXE  hAppAnnexE,
    IN  UINT32      nIP,
    IN  UINT16      nPort,
    IN  void*       pMessage,
    IN  int         nSize
)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppAnnexE;
    hostKey key;
    void    *loc;
    cmTransHost *host;
    TRANSERR res;
    cmTransportAddress remoteAddress;
    BOOL               ok = TRUE;

    if (!hAnnexE)
        return annexEStatusBadParameter;

    if (!transGlobals)
        return annexEStatusBadParameter;

    {
        BYTE *msg = (BYTE *)pMessage;
        int msgType = msg[2 + msg[1]];
        int dirFlag;

        if (msgType != cmQ931setup)
            dirFlag = outgoingHost;
        else
            dirFlag = incomingHost;

        key.ip = nIP;
        key.port = nPort;
        key.type = TRUE; /* annex E */
    }

    loc  = hashFind(transGlobals->hHashHosts, &key);

    if (!loc)
    {
        /* create a new connected host element for the new connection */
        res = createHostByType( transGlobals,
                                NULL,
                                (HSTRANSHOST *)&host,
                                cmTransQ931Conn,
                                NULL,
                                cmTransUseAnnexE);
        if (res != cmTransOK)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "annexEEvNewMessage rejected incoming message (host wasnt created) res=%d", res));
            return annexEStatusResourceProblem;
        }

        /* mark the host as incoming one */
        host->incoming = TRUE;

        /* set the remote address to the host */
        remoteAddress.type          = cmTransportTypeIP;
        remoteAddress.distribution  = cmDistributionUnicast;
        remoteAddress.ip            = nIP;
        remoteAddress.port          = nPort;

        setRemoteAddress(host, &remoteAddress);

        /* lock the host before reporting it */
        ok = FALSE;
        if(emaLock((EMAElement)host))
            ok = TRUE;

        host->state = hostConnected;

        transReportConnect(host, NULL, FALSE);
    }
    else
    {
        ok = FALSE;

        host = *(cmTransHost **)hashGetElement(transGlobals->hHashHosts, loc);
        if (!emaWasDeleted((EMAElement)host))
            if (emaLock((EMAElement)host))
                ok = TRUE;
    }

    if (ok)
    {
        void *msg;

        meiEnter(transGlobals->tablesLock);
        msg = rpoolAllocCopyExternal(transGlobals->messagesRPool, pMessage, nSize);

        {
            INT32 poolSize, availableSize, allocatedSize;

            if (rpoolStatistics(transGlobals->messagesRPool,
                                &poolSize,
                                &availableSize,
                                &allocatedSize))
            {
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "rpoolAllocCopyExternal[annexEEvNewMessage] statistics: max=%d, available=%d, allocated=%d",
                        poolSize,availableSize, allocatedSize));
            }
        }
        meiExit(transGlobals->tablesLock);

        if (msg)
        {
            host->incomingMessage = msg;
            transQ931Handler(NULL, liEvRead, nSize, 0, (void *) host);
        }
        else
        {
        }

        emaUnlock((EMAElement)host);
    }

    return annexEStatusNormal;
}

/**************************************************************************************
 * annexEEvConnectionBroken
 *
 * Purpose: Callback for Annex E upon expiration of I-AM-ALIVE timer & retries.
 *          The call back looks for the host and then calls the regular event handler
 *          (of TPKT) with event liEvClose.
 *
 * Input:   hAnnexE     - The annex E module handle
 *          hAppAnnexE  - The application handle of the annex E module (actualy
 *                        our very own transGlobals structure).
 *          nIP         - The remote ip whence the message arrived.
 *          nPort       - The remote port whence the message arrived.
 *
 * Output:  None.
 *
 **************************************************************************************/
annexEStatus RVCALLCONV annexEEvConnectionBroken(
    IN  HANNEXE     hAnnexE,
    IN  HAPPANNEXE  hAppAnnexE,
    IN  UINT32      nIP,
    IN  UINT16      nPort
)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppAnnexE;
    hostKey key;
    void    *loc;
    cmTransHost *host=NULL;

    if (!hAnnexE)
        return annexEStatusBadParameter;

    if (!transGlobals)
        return annexEStatusBadParameter;

    key.ip = nIP;
    key.port = nPort;
    key.type = TRUE; /* annex E */

    loc  = hashFind(transGlobals->hHashHosts, &key);
    if (loc)
        host = *(cmTransHost **)hashGetElement(transGlobals->hHashHosts, loc);

    if (host)
        transQ931Handler(NULL, liEvClose, 0, 0, host);

    return annexEStatusNormal;
}

/**************************************************************************************
 * annexEEvUseOtherAddress
 *
 * Purpose: Callback for Annex E upon receiving a request to change the remote address.
 *          The call back looks for the host and then deletes it from the host hash only
 *          to put it there agian with the new address.
 *
 * Input:   hAnnexE     - The annex E module handle
 *          hAppAnnexE  - The application handle of the annex E module (actualy
 *                        our very own transGlobals structure).
 *          nIP         - The remote ip whence the message arrived.
 *          nPort       - The remote port whence the message arrived.
 *          pNewAddress - the new address to set the host to.
 *
 * Output:  None.
 *
 **************************************************************************************/
annexEStatus RVCALLCONV annexEEvUseOtherAddress(
    IN  HANNEXE                 hAnnexE,
    IN  HAPPANNEXE              hAppAnnexE,
    IN  UINT32                  nIP,
    IN  UINT16                  nPort,
    IN  cmTransportAddress*     pNewAddress
)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppAnnexE;
    hostKey key;
    void    *loc;
    cmTransHost *host=NULL;

    if (!hAnnexE)
        return annexEStatusBadParameter;

    if (!transGlobals)
        return annexEStatusBadParameter;

    key.ip = nIP;
    key.port = nPort;
    key.type = TRUE; /* annex E */

    loc  = hashFind(transGlobals->hHashHosts, &key);
    if (loc)
        host = *(cmTransHost **)hashGetElement(transGlobals->hHashHosts, loc);

    if (host)
    {
        hashDelete(transGlobals->hHashHosts, loc);
        setRemoteAddress(host, pNewAddress);
    }
    else
        return annexEStatusBadParameter;

    return annexEStatusNormal;
}

/**************************************************************************************
 * annexEEvWriteable
 *
 * Purpose: Callback for Annex E when resources are available again for sending, after
 *          receiving would block error on send.
 *          The call back looks for the host and then calls the regular event handler
 *          (of TPKT) with event liEvWrite.
 *
 * Input:   hAnnexE     - The annex E module handle
 *          hAppAnnexE  - The application handle of the annex E module (actualy
 *                        our very own transGlobals structure).
 *          nIP         - The remote ip whence the message arrived.
 *          nPort       - The remote port whence the message arrived.
 *
 * Output:  None.
 *
 **************************************************************************************/
annexEStatus RVCALLCONV annexEEvWriteable(
    IN  HANNEXE     hAnnexE,
    IN  HAPPANNEXE  hAppAnnexE,
    IN  UINT32      nIP,
    IN  UINT16      nPort
)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppAnnexE;
    hostKey key;
    void    *loc;
    cmTransHost *host=NULL;

    if (!hAnnexE)
        return annexEStatusBadParameter;

    if (!transGlobals)
        return annexEStatusBadParameter;

    key.ip = nIP;
    key.port = nPort;
    key.type = TRUE; /* annex E */

    loc  = hashFind(transGlobals->hHashHosts, &key);
    if (loc)
        host = *(cmTransHost **)hashGetElement(transGlobals->hHashHosts, loc);

    if (host)
        transQ931Handler(NULL, liEvWrite, 0, 0, host);

    return annexEStatusNormal;
}

#ifdef __cplusplus
}
#endif /* __cplusplus*/
