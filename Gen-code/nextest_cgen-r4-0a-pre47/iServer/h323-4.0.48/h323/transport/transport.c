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


/*************************************************************************************
 * Transport module
 * ----------------
 *
 * This is the module that interacts with the network on one hand, while on the other
 * it communicates with the other protocol modules, such as Q.931, H.245, H.450 etc.
 *
 * The lower level can handle TPKT, Annex E types of communications in a transparent
 * way to th eupper layers.
 *
 * The upper level exports and imports (by means of APIs and callbacks) messages to the
 * different modules: Mainly Q.931, H.245 (including tunneled messages) and the
 * rest of the tunneled protocols (Annex M, Annex L).
 *
 **************************************************************************************/
#include <rvinternal.h>
#include <transport.h>
#include <transportint.h>
#include <transnet.h>
#include <transStates.h>
#include <transutil.h>
#include <cmutils.h>
#include <mei.h>
#include <log.h>
#include <ema.h>
#include <hash.h>
#include <tpkt.h>
#include <annexe.h>

#include <q931asn1.h>
#include <h245.h>

#define justLocal           1
#define justRemote          2



/*---------------------------------------------------------------------------------------
 * dummyControlSession :
 *
 * defines if the control session is dummy
 *
 * Input:  hsTranSession   - An handle to the created session element
 * Output: none
 * Return: 1 - if control session is DUMMY.
 *         0 - otherwise
 *---------------------------------------------------------------------------------------*/
BOOL dummyControlSession(HSTRANSSESSION hsTransSession)
{
    cmTransSession  *session    = (cmTransSession *)hsTransSession;
    int result=(BOOL)0; /* init the result as NOT dummy */

    if ((session->tunnelingState == tunnNo) ||
        (session->tunnelingState == tunnRejected))
    {
        if ((session->H245Connection) && (session->H245Connection->dummyHost))
        {
            /* We have a reported existing host (i.e. no tunneling)
            which has no TPKT object,
            i.e. it's a dummy control session that is not connnected to
            the network and thus not allowed to send messages */
            result=(BOOL)1;
        }
    }
    return result;
}


/**************************************************************************************
 * cmTransInit
 *
 * Purpose: To initialize the entire transport module
 *
 * Input:   hAppATrans              - The application handle to the entire transport module
            cmLogMgr                - The stack instance log manager handle.
 *          hPvt                    - The handle to the stack's PVT.
 *          numOfSessions           - The maximal amount of session elemnts to allocate.
 *          numOfHostConnections    - The maximal amount of host elemnts to allocate.
 *          poolSizeInKB            - maximum amount of storage for messages before send
 *          maxMessageSize          - Maximal size of encoded message.
 *          annexESupported         - Do we support annex E.
 *
 * Output:  None.
 *
 * Returned value: A handle to the entire transport module
 *
 **************************************************************************************/
HAPPTRANS cmTransInit(IN HAPPATRANS hAppATrans,
                      IN RVHLOGMGR  cmLogMgr,
                      IN HPVT       hPvt,
                      IN int        numOfSessions,
                      IN int        numOfHostConnections,
                      IN int        poolSizeInKB,
                      IN int        maxMessageSize,
                      IN BOOL       annexESupported)
{
    annexEStatus eStat;

    /* Allocate global element of the module */
    cmTransGlobals *transGlobals = (cmTransGlobals *)calloc(1,sizeof(cmTransGlobals));

    if (!transGlobals) return NULL;

    /* set the application handle to the module's instance */
    transGlobals->hAppATrans = hAppATrans;

    /* Allocate the global lock */
    transGlobals->lock = meiInit();

    /* Allocate a lock for the messages rpool */
    transGlobals->tablesLock = meiInit();

    /* lock the module */
    meiEnter(transGlobals->lock);


    /* Start the log */
    transGlobals->hLog      = logRegister(cmLogMgr, "Transport", "Transport Layer");
    transGlobals->hTPKTCHAN = logRegister(cmLogMgr, "TPKTCHAN", "Transport message print");

    /* save pvt handle */
    transGlobals->hPvt = hPvt;


    logPrint(transGlobals->hLog, RV_INFO,
             (transGlobals->hLog, RV_INFO,
              "Transport layer initializing with: hAppATrans=%x, cmLogMgr=%x, hPvt=%x, numOfSessions=%d, numOfHostConnections=%d, poolSizeInKB=%d, maxMessageSize=%d",
              hAppATrans, cmLogMgr, hPvt, numOfSessions, numOfHostConnections, poolSizeInKB, maxMessageSize));

    /* Allocate reservoir of sessions */
    transGlobals->hEmaSessions =  emaConstruct( sizeof(cmTransSession),
                                                numOfSessions,
                                                emaNormalLocks,
                                                cmLogMgr,
                                                "TRANSPORT Sessions",
                                                0,
                                                transGlobals,
                                                hAppATrans);
    if (transGlobals->hEmaSessions == NULL)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                 (transGlobals->hLog, RV_ERROR,
                  "Transport layer can't allocate sessions pool."));

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release all */
        cmTransEnd( (HAPPTRANS)transGlobals );

        return NULL;
    }

    /* Calculate the host quantity in the case user did not provide us with one*/
    if (numOfHostConnections<0)
    {
        if (annexESupported)
            numOfHostConnections = numOfSessions * 3 + 1;
        else
            numOfHostConnections = numOfSessions * 2 + 1;
    }

    /* Allocate reservoir of hosts    */
    transGlobals->hEmaHosts = emaConstruct(sizeof(cmTransHost),
                                            numOfHostConnections,
                                            emaNormalLocks,
                                            cmLogMgr,
                                            "TRANSPORT Hosts",
                                            0,
                                            transGlobals,
                                            hAppATrans);
    if (transGlobals->hEmaHosts == NULL)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                 (transGlobals->hLog, RV_ERROR,
                  "Transport layer can't allocate hosts pool."));

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release all */
        cmTransEnd( (HAPPTRANS)transGlobals );

        return NULL;
    }

    /* build the hash table for the host by remoteAddress + type */
    transGlobals->hHashHosts = hashConstruct(   numOfHostConnections*2 + 1,
                                                numOfHostConnections,
                                                hashstr,
                                                hashDefaultCompare,
                                                sizeof_hostKey,
                                                sizeof(HSTRANSHOST),
                                                cmLogMgr,
                                                "Transport Hosts hash");

    if (transGlobals->hHashHosts == NULL)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                 (transGlobals->hLog, RV_ERROR,
                  "Transport layer can't build hosts hash table."));

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release all */
        cmTransEnd( (HAPPTRANS)transGlobals );

        return NULL;
    }


    /* build the hash table for the sessions by CRV + host */
    transGlobals->hHashSessions = hashConstruct(numOfSessions*2 + 1,
                                                numOfSessions,
                                                hashstr,
                                                hashDefaultCompare,
                                                sizeof_sessionKey,
                                                sizeof(HSTRANSSESSION),
                                                cmLogMgr,
                                                "Transport Sessions hash");

    if (transGlobals->hHashSessions == NULL)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                 (transGlobals->hLog, RV_ERROR,
                  "Transport layer can't build sessions hash table."));

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release all */
        cmTransEnd( (HAPPTRANS)transGlobals );

        return NULL;
    }

    /* build the rpool reservoir for outgoing messages */
    transGlobals->curUsedNumOfMessagesInRpool = 0;
    transGlobals->maxUsedNumOfMessagesInRpool = 0;

    transGlobals->messagesRPool = rpoolConstruct( TRANS_RPOOL_CHUNK_SIZE,
                                                  (poolSizeInKB * 1024)/TRANS_RPOOL_CHUNK_SIZE,
                                                  cmLogMgr,
                                                  "outgoing/tunneled messages");

    if (transGlobals->messagesRPool == NULL)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                 (transGlobals->hLog, RV_ERROR,
                  "Transport layer can't build rPool of messages."));

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release all */
        cmTransEnd( (HAPPTRANS)transGlobals );

        return NULL;
    }

    /* allocate the encoding buffer (the extra bytes are for TPKT header and pointer
       to next message */
    {
        BYTE *buffer;

        transGlobals->codingBufferSize = maxMessageSize+MSG_HEADER_SIZE+TPKT_HEADER_SIZE;
        getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);

        if (buffer == NULL)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                     (transGlobals->hLog, RV_ERROR,
                      "Transport layer can't allocate coding buffer."));

            /* unlock the module */
            meiExit(transGlobals->lock);

            /* release all */
            cmTransEnd( (HAPPTRANS)transGlobals );

            return NULL;
        }
    }

    /* start the network TPKT module */

    /* for each session there is one Q.931 connecting socket and two H.245 sockets,
       one for listenning and one for connecting. The extra TPKT element is for
       the Q.931 listenning socket.
       Note: We may statisticaly allocate less TPKT elements since simultaneous
             listen and connect in H.245 is rare */
    transGlobals->tpktCntrl = tpktInit(numOfSessions*3 + 1, cmLogMgr);
    if (transGlobals->tpktCntrl == NULL)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                 (transGlobals->hLog, RV_ERROR,
                  "Transport layer can't open TPKT package."));

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release all */
        cmTransEnd( (HAPPTRANS)transGlobals );

        return NULL;
    }

    /* create timers pool (two for transport (mux & dummy h245) and one for annex E) */
    transGlobals->hTimers = mtimerInit( numOfSessions * 3, NULL );
    if (transGlobals->hTimers == NULL)
    {
         logPrint(transGlobals->hLog, RV_ERROR,
                  (transGlobals->hLog, RV_ERROR,
                  "Transport layer can't allocate timers."));

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release all */
        cmTransEnd( (HAPPTRANS)transGlobals );

        return NULL;
    }

    /* start the network annex E module */
    if (annexESupported)
    {
        transGlobals->annexESupported = TRUE;

        eStat = annexEInit(&(transGlobals->annexECntrl),
                          (HAPPANNEXE)transGlobals,
                          numOfSessions + 1, /*  - allocate one more, just to reject incoming calls */
                          maxMessageSize,
                          numOfSessions,
                          transGlobals->hTimers,
                          cmLogMgr);

        if (eStat != annexEStatusNormal)
        {
           logPrint(transGlobals->hLog, RV_ERROR,
                   (transGlobals->hLog, RV_ERROR,
                   "Transport layer can't open Annex E package."));

            /* unlock the module */
            meiExit(transGlobals->lock);

            /* release all */
            cmTransEnd( (HAPPTRANS)transGlobals );

            return NULL;
       }
       else
       {
         annexEEvents annexEEventHandlers = {   NULL /*annexEEvRestart*/,
                                                annexEEvNewMessage,
                                                annexEEvConnectionBroken,
                                                annexEEvUseOtherAddress,
                                                NULL /*annexEEvNotSupported*/,
                                                annexEEvWriteable
                                            };

         eStat = annexESetEventHandler(transGlobals->annexECntrl,
                                       &annexEEventHandlers);
         if (eStat != annexEStatusNormal)
         {
             logPrint(transGlobals->hLog, RV_ERROR,
                     (transGlobals->hLog, RV_ERROR,
                     "Transport layer can't set Annex E callbacks."));
         }
       }
    }
    else
        transGlobals->annexESupported = FALSE;


    /* create syntax trees for decoding routines */
    transGlobals->synProtQ931 = pstConstruct(q931asn1GetSyntax(), (char *)"Q931Message");
    transGlobals->synProtH245 = pstConstruct(h245GetSyntax(), (char *)"MultimediaSystemControlMessage");
    transGlobals->synProtH450 = pstConstruct(q931asn1GetSyntax(),
                   (char *)"Q931Message.message.setup.userUser.h323-UserInformation.h323-uu-pdu.h4501SupplementaryService");
    /* unlock the module */
    meiExit(transGlobals->lock);

    /* return the handle to the module global element */
    logPrint(transGlobals->hLog, RV_INFO,
             (transGlobals->hLog, RV_INFO, "Transport layer initialized. hAppTrans=%x",transGlobals));
    return (HAPPTRANS)transGlobals;
}

/**************************************************************************************
 * cmTransStart
 *
 * Purpose: To start the listenning process on TPKT and/or Annex E
 *
 * Input:   happTrans               - The handle to the instance of the transport module.
 *          TPKTcallSignalingAddr   - The TPKT Q.931 listenning address.
 *          annexEcallSignalingAddr - The annex E address.
 *          localIPAddress          - The local IP address as was received from the config.
 *
 * Output:  None.
 *
 * Returned value: cmTransOK - if success; cmTransErr - otherwise
 *
 **************************************************************************************/
TRANSERR cmTransStart(IN HAPPTRANS happTrans,
                      IN cmTransportAddress *TPKTcallSignalingAddr,
                      IN cmTransportAddress *annexEcallSignalingAddr,
                      IN int                localIPAddress,
                      IN CMTRANSANNEXEPARAM *cmTransAnnexEParam)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)happTrans;
    TRANSERR        reply = cmTransOK;

    if (!transGlobals)
        return cmTransErr;

    /* lock the module */
    meiEnter(transGlobals->lock);

    /* save the local IP into the global area */
    transGlobals->localIPAddress = localIPAddress;

    /* start the listening process for TPKT */
    if ((int)TPKTcallSignalingAddr->type != -1)
    {
        if (cmTransCreateHost(happTrans, NULL, (HSTRANSHOST *)&transGlobals->hTPKTListen) == cmTransOK)
        {
            if (!TPKTcallSignalingAddr->ip)
                TPKTcallSignalingAddr->ip = transGlobals->localIPAddress;

            if (transGlobals->hostEventHandlers.cmEvTransHostListen)
            {
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransHostListen(hsHost = %d(%x), haHost=NULL, type=cmTransQ931Conn, address = (ip:%x,port:%d))",
                        emaGetIndex((EMAElement)transGlobals->hTPKTListen),
                        transGlobals->hTPKTListen,
                        TPKTcallSignalingAddr->ip,
                        TPKTcallSignalingAddr->port));

                reply = transGlobals->hostEventHandlers.cmEvTransHostListen(
                                        (HSTRANSHOST)transGlobals->hTPKTListen,
                                        NULL,
                                        cmTransQ931Conn,
                                        TPKTcallSignalingAddr);
                transGlobals->hTPKTListen->reported = TRUE;
            }

            /* if user rejected the listen */
            if (reply == cmTransIgnoreMessage)
            {
                closeHostInternal((HSTRANSHOST)transGlobals->hTPKTListen, TRUE);
                transGlobals->hTPKTListen = NULL;

                logPrint(transGlobals->hLog, RV_INFO,
                         (transGlobals->hLog, RV_INFO,
                        "cmTransStart initiate listen on TPKT refused by user"));
            }
            else
            {
                transGlobals->hTPKTListen->hTpkt =  tpktOpen(transGlobals->tpktCntrl,
                                                            TPKTcallSignalingAddr->ip,
                                                            TPKTcallSignalingAddr->port,
                                                            tpktMultiServer,
                                                            transQ931AcceptHandler,
                                                            (void *)happTrans);

                /* if listenning started OK, get the allocated address for listen */
                if (transGlobals->hTPKTListen->hTpkt)
                    getGoodAddress(transGlobals, transGlobals->hTPKTListen->hTpkt, NULL, cmTransQ931Conn, TPKTcallSignalingAddr);


                /* report that the listen started (or failed) */
                if (transGlobals->hostEventHandlers.cmEvTransHostListening)
                {
                    int numOfLocks;
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransHostListening(hsHost = %d(%x), haHost=NULL, type=cmTransQ931Conn, address = (ip:%x,port:%d), error=%d)",
                            emaGetIndex((EMAElement)transGlobals->hTPKTListen),
                            transGlobals->hTPKTListen,
                            TPKTcallSignalingAddr->ip,
                            TPKTcallSignalingAddr->port,
                            (transGlobals->hTPKTListen->hTpkt == NULL)));

                    numOfLocks = emaPrepareForCallback((EMAElement)transGlobals->hTPKTListen);
                    transGlobals->hostEventHandlers.cmEvTransHostListening(
                                            (HSTRANSHOST)transGlobals->hTPKTListen,
                                            NULL,
                                            cmTransQ931Conn,
                                            TPKTcallSignalingAddr,
                                            (transGlobals->hTPKTListen->hTpkt == NULL));
                    emaReturnFromCallback((EMAElement)transGlobals->hTPKTListen, numOfLocks);
                    transGlobals->hTPKTListen->reported = TRUE;
                }

                /* if we had an error starting the listenning, call the whole thing off */
                if ( !(transGlobals->hTPKTListen->hTpkt) )
                {
                    closeHostInternal((HSTRANSHOST)transGlobals->hTPKTListen, TRUE);
                    transGlobals->hTPKTListen = NULL;

                    meiExit(transGlobals->lock);
                    logPrint(transGlobals->hLog, RV_ERROR,
                             (transGlobals->hLog, RV_ERROR,
                             "cmTransStart failed to initiate listen on TPKT"));
                    return cmTransErr;
                }
            }
        }
    }

    /* set the annex E parameters */
    if (transGlobals->annexESupported)
    {
        annexEStatus eStat;

        eStat = annexESetProtocolParams(transGlobals->annexECntrl,
                                        (int)cmTransAnnexEParam->t_R1,
                                        (int)cmTransAnnexEParam->t_R2,
                                        (int)cmTransAnnexEParam->n_R1,
                                        (int)cmTransAnnexEParam->t_IMA1,
                                        (int)cmTransAnnexEParam->n_IMA1,
                                        (int)cmTransAnnexEParam->t_DT);

        if (eStat != annexEStatusNormal)
        {
            meiExit(transGlobals->lock);
            logPrint(transGlobals->hLog, RV_ERROR,
                     (transGlobals->hLog, RV_ERROR,
                     "cmTransStart failed to set annex E protocol params eStat=%d", eStat));
            return cmTransErr;
        }
    }


    /* Start the annex E "listenning" process (openning the UDP socket on the given address) */
    if ( (transGlobals->annexESupported) && ((int)annexEcallSignalingAddr->type != -1) )
    {
        annexEStatus eStat = annexESetLocalAddress(transGlobals->annexECntrl, annexEcallSignalingAddr);
        if (eStat != annexEStatusNormal)
        {
            meiExit(transGlobals->lock);
            logPrint(transGlobals->hLog, RV_ERROR,
                     (transGlobals->hLog, RV_ERROR,
                     "cmTransStart failed to set Local address for annex E eStat=%d", eStat));
            return cmTransErr;
        }

        eStat = annexEStart(transGlobals->annexECntrl);
        if (eStat != annexEStatusNormal)
        {
            meiExit(transGlobals->lock);
            logPrint(transGlobals->hLog, RV_ERROR,
                     (transGlobals->hLog, RV_ERROR,
                     "cmTransStart failed annexEStart eStat=%d", eStat));
            return cmTransErr;
        }

        eStat = annexEGetLocalAddress(transGlobals->annexECntrl, annexEcallSignalingAddr);
        if (eStat != annexEStatusNormal)
        {
            meiExit(transGlobals->lock);
            logPrint(transGlobals->hLog, RV_ERROR,
                     (transGlobals->hLog, RV_ERROR,
                     "cmTransStart failed annexEGetLocalAddress eStat=%d", eStat));
            return cmTransErr;
        }
    }

    meiExit(transGlobals->lock);

    return cmTransOK;
}

/**************************************************************************************
 * cmTransStop
 *
 * Purpose: Stops all the listenning processes that were started by cmTransStart.
 *
 * Input:   hAppTrans - A handle to the entire transport module
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransStop(IN HAPPTRANS hAppTrans)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;

    if (!transGlobals)
        return cmTransErr;

    /* lock the module */
    meiEnter(transGlobals->lock);

    /* close the listenning process of TPKT for Q.931 connections */
    if (transGlobals->hTPKTListen)
    {
        closeHostInternal((HSTRANSHOST)transGlobals->hTPKTListen, TRUE);
        transGlobals->hTPKTListen = NULL;
    }

    /* stop the annex E */
    if (transGlobals->annexESupported)
        annexEStop(transGlobals->annexECntrl);

    meiExit(transGlobals->lock);

    return cmTransOK;
}

/**************************************************************************************
 * cmTransEnd
 *
 * Purpose: Shuts down the module and deallocates all its components.
 *
 * Input:   hAppTrans - A handle to the entire transport module
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransEnd(IN HAPPTRANS hAppTrans)
{

    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;

    if (hAppTrans)
    {
        /* lock the module */
        meiEnter(transGlobals->lock);

        /* close the annex E package */
        if (transGlobals->annexECntrl)
            annexEEnd(transGlobals->annexECntrl);

        /* close the tpkt package */
        if (transGlobals->tpktCntrl)
            tpktEnd(transGlobals->tpktCntrl);

        /* release the messages rPool */
        if (transGlobals->messagesRPool)
            rpoolDestruct(transGlobals->messagesRPool);

        /* release the sessions hash table */
        if (transGlobals->hHashSessions)
            hashDestruct(transGlobals->hHashSessions);

        /* release the hosts hash table */
        if (transGlobals->hHashHosts)
            hashDestruct(transGlobals->hHashHosts);

        /* release the resvoir of hosts */
        if (transGlobals->hEmaHosts)
            emaDestruct(transGlobals->hEmaHosts);

        /* release the reservoir of sessions */
        if (transGlobals->hEmaSessions)
            emaDestruct(transGlobals->hEmaSessions);

        /* destroy timers pool */
        if (transGlobals->hTimers)
            mtimerEnd( transGlobals->hTimers );

        /* destruct the syntax trees */
        pstDestruct(transGlobals->synProtH450);
        pstDestruct(transGlobals->synProtH245);
        pstDestruct(transGlobals->synProtQ931);

        /* unlock the module */
        meiExit(transGlobals->lock);

        /* release the rpool lock */
        meiEnd(transGlobals->tablesLock);

        /* release the global lock */
        meiEnd(transGlobals->lock);

        logPrint(transGlobals->hLog, RV_INFO,
                 (transGlobals->hLog, RV_INFO, "Transport layer was shutdown"));
        /* deallocate the global element of the module */
        free((void *)hAppTrans);
    }

    return cmTransOK;
}

/**************************************************************************************
 * cmTransCreateSession
 *
 * Purpose: Creates a new session element according to its parameters.
 *
 * Input:   hAppTrans       - A handle to the entire transport module
 *          haTranSession   - An application handle to be set to the session, ususally
 *                            would be the call handle that is associated with this session.
 *
 * Output:  hsTranSession   - An handle to the created session element
 *
 **************************************************************************************/
TRANSERR cmTransCreateSession(  IN  HAPPTRANS       hAppTrans,
                                IN  HATRANSSESSION  haTransSession,
                                OUT HSTRANSSESSION  *hsTransSession)
{
    cmTransGlobals  *transGlobals = (cmTransGlobals *)hAppTrans;
    EMAElement      newSession;

    emaStatistics stat;

    /* if the module was initialized */
    if (hAppTrans)
    {
        /* allocate a new session element */
        newSession = emaAdd(transGlobals->hEmaSessions, (void*)haTransSession);

        emaGetStatistics(transGlobals->hEmaSessions, &stat);

        /* set the output parameter to the allocated element (maybe NULL) */
        if (hsTransSession)
            *hsTransSession = (HSTRANSSESSION)newSession;

        /* if an element was indeed allocated return OK */
        if (newSession)
        {
            cmTransSession *session = (cmTransSession *)newSession;
            memset(session, 0, sizeof(cmTransSession));
            session->h245Stage                      = cmTransH245Stage_connect;
            session->tunnelingState                 = tunnNo;
            session->isTunnelingSupported           = FALSE;
            session->faststartState                 = fsNo;
            session->firstMessageSent               = FALSE;
            session->connectedToHost                = FALSE;
            session->reportedH245Connect            = FALSE;
            session->outgoing                       = TRUE;
            session->closeOnNoSessions              = TRUE;
            session->h450Element                    = -1;
            session->annexLElement                  = -1;
            session->annexMElement                  = -1;
            session->annexEUsageMode                = cmTransNoAnnexE;
            session->muxConnectTimer                = (HTI)-1;
            session->h245ConnectTimer               = (HTI)-1;

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmTransCreateSession = %d(%x)", emaGetIndex((EMAElement)session), session));

            return cmTransOK;
        }
    }

    /* in cases that OK was NOT sent, return error indication */
    logPrint(transGlobals->hLog, RV_ERROR,
            (transGlobals->hLog, RV_ERROR,
            "cmTransCreateSession [FAILURE](haSession=%x)", haTransSession));

    return cmTransErr;
}

/**************************************************************************************
 * cmTransCloseSession
 *
 * Purpose: Deletes a session element.
 *
 * Input:   hsTranSession   - An handle to the session element
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransCloseSession(IN HSTRANSSESSION hsTransSession)
{
    TRANSERR res = cmTransErr;
    /* check if an element exists */
    if (hsTransSession)
    {
        /* lock the whole module for this major change */
        if(emaLock((EMAElement)hsTransSession))
        {
            /* retrieve the transport module global area */
#ifndef NOLOGSUPPORT
            cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);
#endif

            logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                    "cmTransCloseSession = %d(%x)", emaGetIndex((EMAElement)hsTransSession), hsTransSession));

            transSessionClose((cmTransSession *)hsTransSession);

            /* delete the host element */
            if (!emaWasDeleted((EMAElement)hsTransSession))
            {
                res = ((emaDelete( (EMAElement)hsTransSession )!= 0) ? cmTransErr : cmTransOK);
            }
            else
                res = cmTransOK;

            emaUnlock((EMAElement)hsTransSession);
        }
    }
    return res;
}


/**************************************************************************************
 * cmTransDrop
 *
 * Purpose: Drop a session (put its state in Idle).
 *
 * Input:   hsTranSession   - An handle to the session element
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransDrop(IN HSTRANSSESSION hsTransSession)
{
    cmTransSession *session=(cmTransSession *)hsTransSession;
    if (emaLock((EMAElement)hsTransSession))
    {
        if (session->Q931Connection != NULL)
        {
            cmTransHost* host = session->Q931Connection;

            if (emaLock((EMAElement)host))
            {
				/* see if this host is handling other sessions as well */
				if ((host->firstSession == NULL) || 
					((host->firstSession == session) && (host->firstSession->nextSession == NULL)))
				{
					/* host handles no other sessions than the current one (or none at all) */
                    if (host->closeOnNoSessions || host->remoteCloseOnNoSessions)
                        host->state = hostIdle;
				}
                emaUnlock((EMAElement)host);
            }
        }
        emaUnlock((EMAElement)hsTransSession);
    }
    return cmTransOK;
}


/**************************************************************************************
 * cmTransCreateHost
 *
 * Purpose: Creates a new host element according to its parameters.
 *
 * Input:   hAppTrans   - A handle to the entire transport module
 *          haTranHost  - An application handle to be set to the host element.
 *
 * Output:  hsTranHost  - An handle to the created host element
 *
 **************************************************************************************/
TRANSERR cmTransCreateHost( IN  HAPPTRANS   hAppTrans,
                            IN  HATRANSHOST haTransHost,
                            OUT HSTRANSHOST *hsTransHost)
{
    cmTransGlobals  *transGlobals = (cmTransGlobals *)hAppTrans;
    EMAElement      newHost;

    /* if the module was initialized */
    if (hAppTrans)
    {
        /* allocate a new host element */
        newHost = emaAdd(transGlobals->hEmaHosts, (void*)haTransHost);

        /* set the output parameter to the allocated element (maybe NULL) */
        if (hsTransHost)
            *hsTransHost = (HSTRANSHOST)newHost;

        /* if an element was indeed allocated return OK */
        if (newHost)
        {
            cmTransHost *host = (cmTransHost *)newHost;

            memset(host, 0, sizeof(cmTransHost));

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmTransCreateHost created host=%d(%x)",emaGetIndex((EMAElement)host), host));

            return cmTransOK;
        }
    }

    /* in cases that OK was NOT sent, return error indication */
    return cmTransErr;
}

/**************************************************************************************
 * cmTransCloseHost
 *
 * Purpose: Deletes a host element. Will notify all its associates sessions.
 *
 * Input:   hsTranHost  - An handle to the host element
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransCloseHost(IN HSTRANSHOST hsTransHost)
{
    return closeHostInternal(hsTransHost, FALSE);
}

/**************************************************************************************
 * cmTransSetSessionParam
 *
 * Purpose: Sets a specific parameter for the given session element.
 *
 * Input:   hsTranSession   - An handle to the session element
 *          param           - The name of the parameter to be set.
 *          value           - An integer or pointer to set as the new parameter.
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransSetSessionParam(IN HSTRANSSESSION       hsTransSession,
                                IN TRANSSESSIONPARAM    param,
                                IN void                 *value)
{
    cmTransSession *session = (cmTransSession *)hsTransSession;

    if (emaLock((EMAElement)session))
    {
        switch (param)
        {
            case cmTransParam_isTunnelingSupported:
                {
                    session->isTunnelingSupported = *(BOOL *)value;
                }
                break;

            case cmTransParam_notEstablishControl:
                {
                    session->notEstablishControl = *(BOOL *)value;
                }
                break;

            case cmTransParam_H245Stage:
                {
                    session->h245Stage = *(cmH245Stage *)value;
                }
                break;

            case cmTransParam_isParallelTunnelingSupported:
                {
                    session->isParallelTunnelingSupported = *(BOOL *)value;
                }
                break;

            case cmTransParam_shutdownEmptyConnection:
                {
                    session->closeOnNoSessions = *(BOOL *)value;
                    if (session->Q931Connection)
                        cmTransSetHostParam((HSTRANSHOST)session->Q931Connection,
                                             cmTransHostParam_shutdownEmptyConnection,
                                             value,
                                             FALSE);
                }
                break;

            case cmTransParam_isMultiplexed:
                {
                    session->isMultiplexed = *(BOOL *)value;
                    if (session->Q931Connection)
                        cmTransSetHostParam((HSTRANSHOST)session->Q931Connection,
                                             cmTransHostParam_isMultiplexed,
                                             value,
                                             FALSE);
                }
                break;

            case cmTransParam_isAnnexESupported:
                {
                    session->annexEUsageMode = *(cmAnnexEUsageMode *)value;
                }
                break;

            default:
                break;
        }

        /* unlock the session */
        emaUnlock((EMAElement)session);
        return cmTransOK;
    }

    return cmTransErr;
}

/**************************************************************************************
 * cmTransGetSessionParam
 *
 * Purpose: Gets a specific parameter from a given session element.
 *
 * Input:   hsTranSession   - An handle to the session element
 *          param           - The name of the parameter to get.
 *
 * Output:  value           - An integer or pointer which is the value of the parameter.
 *
 **************************************************************************************/
TRANSERR cmTransGetSessionParam(IN  HSTRANSSESSION      hsTransSession,
                                IN  TRANSSESSIONPARAM   param,
                                OUT void                *value)
{
    cmTransSession *session = (cmTransSession *)hsTransSession;

    if (emaLock((EMAElement)session))
    {
        switch (param)
        {
            case cmTransParam_host:
                {
                    cmTransHost *host;

                    if (session->annexEUsageMode != cmTransNoAnnexE)
                        host = session->annexEConnection;
                    else
                        host = session->Q931Connection;

                    /* lock the host */
                    if (host)
                    {
                        if (emaLock((EMAElement)host))
                        {
                            *(cmTransHost **)value = host;
                            emaUnlock((EMAElement)host);
                        }
                    }
                }
                break;

            case cmTransParam_H245Connection:
                {
                    *(cmTransHost **)value = session->H245Connection;
                }
                break;

            case cmTransParam_isTunnelingSupported:
                {
                    if ((session->tunnelingState == tunnRejected) || session->switchToSeparateWasAsked)
                    {
                        /* Tunneling was rejected - no way we're going to
                           have a tunneled call here */
                        *(BOOL *)value = FALSE;
                    }
                    else
                    {
                        /* We're not sure if we're going tunneled or not, so we're just
                           assuming the user will be happy to know what his side is
                           doing. */
                        *(BOOL *)value = session->isTunnelingSupported;
                    }
                }
                break;

            case cmTransParam_notEstablishControl:
                {
                    *(BOOL *)value = session->notEstablishControl;
                }
                break;

            case cmTransParam_H245Stage:
                {
                    *(cmH245Stage *)value = session->h245Stage;
                }
                break;

            case cmTransParam_isParallelTunnelingSupported:
                {
                    *(BOOL *)value = session->isParallelTunnelingSupported;
                }
                break;

            case cmTransParam_shutdownEmptyConnection:
                {
                    *(BOOL *)value = session->closeOnNoSessions;
                }
                break;

            case cmTransParam_isMultiplexed:
                {
                    *(BOOL *)value = session->isMultiplexed;
                }
                break;

            case cmTransParam_isAnnexESupported:
                {
                    *(BOOL *)value = session->annexEUsageMode;
                }
                break;
        }

        /* release the lock on the session */
        emaUnlock((EMAElement)session);
        return cmTransOK;
    }

    return cmTransErr;
}

/**************************************************************************************
 * cmTransSetHostParam
 *
 * Purpose: Sets a specific parameter for the given host element.
 *
 * Input:   hsTranHost  - An handle to the host element
 *          param       - The name of the parameter to be set.
 *          value       - An integer or pointer to set as the new parameter.
 *          force       - In case of multiplexing parameters force the sending of FACILITY
 *                        to the remote with the new parameters.
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransSetHostParam(   IN HSTRANSHOST          hsTransHost,
                                IN TRANSHOSTPARAM       param,
                                IN void                 *value,
                                IN BOOL                 force)
{
    cmTransHost     *host = (cmTransHost *)hsTransHost;
    cmTransGlobals  *transGlobals;

    if (emaLock((EMAElement)host))
    {
        switch (param)
        {
            case cmTransHostParam_shutdownEmptyConnection:
                {
                    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);

                    host->closeOnNoSessions = *(BOOL *)value;

                    /* if we have a connected host, which is TPKT and we are
                       forced to send a facility about the change, do it */
                    if ( ( (host->state == hostConnected) || (host->state == hostBusy) ) &&
                         (force) &&
                         (host->annexEUsageMode == cmTransNoAnnexE) )
                        sendGeneralFacility(transGlobals, host);
                }
                break;

            case cmTransHostParam_isMultiplexed:
                {
                    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);

                    host->isMultiplexed = *(BOOL *)value;
                    /* if we have a connected host, which is TPKT and we are
                       forced to send a facility about the change, do it */
                    if ( ( (host->state == hostConnected) || (host->state == hostBusy) ) &&
                         (force) &&
                         (host->annexEUsageMode == cmTransNoAnnexE) )
                        sendGeneralFacility(transGlobals, host);
                }
                break;

            case cmTransHostParam_remoteAddress:
                {
                    if (setRemoteAddress((cmTransHost *)hsTransHost,
                                         (cmTransportAddress *)value) == cmTransErr)
                    {
                        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);

                        emaUnlock((EMAElement)host);
                        logPrint(transGlobals->hLog, RV_ERROR,
                                (transGlobals->hLog, RV_ERROR,
                                "cmTransSetHostParam failed on updating hosts hash table"));
                        return cmTransErr;
                    }
                }
                break;

            case cmTransHostParam_localAddress:
                {
                    memcpy( (void *)&(host->localAddress),
                            value,
                            sizeof(cmTransportAddress));
                }
                break;

            case cmTransHostParam_isAnnexESupported:
                {
                    host->annexEUsageMode = *(cmAnnexEUsageMode *)value;
                }
                break;
        }

        emaUnlock((EMAElement)host);
        return cmTransOK;
    }

    return cmTransErr;
}

/**************************************************************************************
 * cmTransGetHostParam
 *
 * Purpose: Gets a specific parameter from a given host element.
 *
 * Input:   hsTranHost  - An handle to the host element
 *          param       - The name of the parameter to get.
 *
 * Output:  value       - An integer or pointer which is the value of the parameter.
 *
 **************************************************************************************/
TRANSERR cmTransGetHostParam(   IN  HSTRANSHOST     hsTransHost,
                                IN  TRANSHOSTPARAM  param,
                                OUT void            *value)
{
    cmTransHost *host = (cmTransHost *)hsTransHost;

    if (emaLock((EMAElement)host))
    {
        switch (param)
        {
            case cmTransHostParam_shutdownEmptyConnection:
                {
                    *(BOOL *)value = host->closeOnNoSessions;
                }
                break;

            case cmTransHostParam_isMultiplexed:
                {
                    *(BOOL *)value = host->isMultiplexed;
                }
                break;

            case cmTransHostParam_remoteAddress:
                if (host->hTpkt && (host->state >= hostConnected))
                {
                    cmTransportAddress remote;
                    int socket = tpktGetSock(host->hTpkt);
                    memset(&remote, 0, sizeof(cmTransportAddress));

                    remote.ip = liGetRemoteIP(socket);
                    remote.port = liGetRemotePort(socket);
                    memcpy( value,
                            (void *)&(remote),
                            sizeof(cmTransportAddress));
                }
                else
                {
                    memcpy( value,
                            (void *)&(host->remoteAddress),
                            sizeof(cmTransportAddress));
                }
                break;

            case cmTransHostParam_localAddress:
                if (host->hTpkt && (host->state >= hostConnected))
                {
                    cmTransportAddress local;
                    int socket = tpktGetSock(host->hTpkt);
                    memset(&local,  0, sizeof(cmTransportAddress));

                    local.ip = liGetSockIP(socket);
                    local.port = liGetSockPort(socket);
                    memcpy( value,
                            (void *)&(local),
                            sizeof(cmTransportAddress));
                }
                else
                {
                    memcpy( value,
                            (void *)&(host->localAddress),
                            sizeof(cmTransportAddress));
                }
                break;

            case cmTransHostParam_isAnnexESupported:
                {
                    *(BOOL *)value = host->annexEUsageMode;
                }
                break;
        }

        emaUnlock((EMAElement)host);
        return cmTransOK;
    }

    return cmTransErr;
}

/**************************************************************************************
 * cmTransSetSessionEventHandler
 *
 * Purpose: Sets the event handlers' pointers for session related callbacks
 *
 * Input:   hAppTrans           - A handle to the entire transport module
 *          transSessionEvents  - A structure with the pointers to the callbacks
 *          size                - The size of that structure.
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransSetSessionEventHandler( IN HAPPTRANS            hAppTrans,
                                        IN TRANSSESSIONEVENTS   *transSessionEvents,
                                        IN int                  size)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;

    /* lock the entire module before intalling the handlers */
    meiEnter(transGlobals->lock);
    memcpy((void *)&(transGlobals->sessionEventHandlers),
           (void *)transSessionEvents,
           size);
    meiExit(transGlobals->lock);

    return cmTransOK;
}

/**************************************************************************************
 * cmTransGetSessionEventHandler
 *
 * Purpose: Gets the event handlers' pointers for session related callbacks
 *
 * Input:   hAppTrans           - A handle to the entire transport module
 *          size                - The size of the given structure.
 *
 * Output:  transSessionEvents  - A structure with the pointers to the callbacks
 *
 **************************************************************************************/
TRANSERR cmTransGetSessionEventHandler( IN  HAPPTRANS           hAppTrans,
                                        OUT TRANSSESSIONEVENTS  *transSessionEvents,
                                        IN  int                 size)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;

    /* lock the entire module before getting the handlers */
    meiEnter(transGlobals->lock);
    memcpy( (void *)transSessionEvents,
            (void *)&(transGlobals->sessionEventHandlers),
            size);
    meiExit(transGlobals->lock);

    return cmTransOK;
}

/**************************************************************************************
 * cmTransSetHostEventHandler
 *
 * Purpose: Sets the event handlers' pointers for host related callbacks
 *
 * Input:   hAppTrans       - A handle to the entire transport module
 *          transHostEvents - A structure with the pointers to the callbacks
 *          size            - The size of that structure.
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransSetHostEventHandler(    IN HAPPTRANS        hAppTrans,
                                        IN TRANSHOSTEVENTS  *transHostEvents,
                                        IN int              size)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;

    /* lock the entire module before intalling the handlers */
    meiEnter(transGlobals->lock);
    memcpy((void *)&(transGlobals->hostEventHandlers),
           (void *)transHostEvents,
           size);
    meiExit(transGlobals->lock);

    return cmTransOK;
}

/**************************************************************************************
 * cmTransGetHostEventHandler
 *
 * Purpose: Gets the event handlers' pointers for host related callbacks
 *
 * Input:   hAppTrans       - A handle to the entire transport module
 *          size            - The size of the given structure.
 *
 * Output:  transHostEvents - A structure with the pointers to the callbacks
 *
 **************************************************************************************/
TRANSERR cmTransGetHostEventHandler(    IN  HAPPTRANS       hAppTrans,
                                        OUT TRANSHOSTEVENTS *transHostEvents,
                                        IN  int             size)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;

    /* lock the entire module before getting the handlers */
    meiEnter(transGlobals->lock);
    memcpy( (void *)transHostEvents,
            (void *)&(transGlobals->hostEventHandlers),
            size);
    meiExit(transGlobals->lock);

    return cmTransOK;
}


/**************************************************************************************
 * setQ931Host
 *
 * Purpose: chooses a Q.931 host (existing or new) and sets its local address.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          localAddress    - the local address to connect from.
 *          remoteAddress   - The remote address to connect to.
 *          createNewHost   - TRUE:  create new host even if multiplexed
 *                            FALSE: for multiplex use existing host, if exists.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransErr            - In case that an error occured.
 *                  cmTransNotMultiplexed - The given host is not multiplexed.
 *                  cmTransOK             - In case that the connection is already opened.
 *
 **************************************************************************************/
TRANSERR setQ931Host(   IN    cmTransSession        *session,
                        INOUT cmTransportAddress    *localAddress,
                        IN    cmTransportAddress    *remoteAddress,
                        IN    cmTransHost           *oldHost,
                        IN    BOOL                  createNewHost)
{
    BOOL        hostIsLocked = FALSE;
    cmTransHost *host = NULL;
    TRANSERR    res = cmTransOK;

    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* If allowed, look for a suitable host in the host's hash */
    if ( (!createNewHost) && (session->isMultiplexed) )
        host = findHostInHash(transGlobals,
                              remoteAddress,
                              session->isMultiplexed,
                              FALSE);

    /* if host was not found and we have an old host, use it */
    if ( (!host) && (oldHost) )
    {
        /* use the existing host (created when the local address was given) */
        host = oldHost;

        /* lock the host */
        if(emaLock((EMAElement) host))
            hostIsLocked = TRUE;
        else
            return cmTransErr;

        /* Ignore the previous session, it is the same as this one */
        host->firstSession = NULL;

        /* set the given remote address into the hash, if none was determined yet */
        if (host->remoteAddress.ip == 0)
            setRemoteAddress(host, remoteAddress);
    }
    else
        /* We have a new host, get rid of the old one which was created
           just for the local address (what a waste, next time use diffSrcAddressInSetupAndARQ
           in the configuration */
        if (oldHost)
            closeHostInternal((HSTRANSHOST)oldHost, TRUE);

    /* Still no host? create a new host */
    if (!host)
    {
        res = createHostByType(transGlobals,
                               (HSTRANSSESSION)session,
                               (HSTRANSHOST *)&host,
                               cmTransQ931Conn,
                               NULL,
                               cmTransNoAnnexE);
        if (res != cmTransOK)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "allocateQ931Host failed on allocating host element"));
            return cmTransErr;
        }

        /* update the given address to the host */
        host->remoteAddress.type = (cmTransportType)-1;
        host->remoteAddress.ip   = 0;
        host->localAddress.type  = (cmTransportType)-1;
        host->localAddress.ip    = 0;

        /* Write the remote address, if exists, to the hash */
        if (remoteAddress)
        {
            if (setRemoteAddress(host, remoteAddress) == cmTransErr)
            {
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "allocateQ931Host failed on updating hosts hash table"));
                return cmTransErr;
            }
        }

        /* Update the local address, if exists */
        if (localAddress)
            host->localAddress = *localAddress;
        else
        {
            host->localAddress.ip   = 0;
            host->localAddress.port = 0;
        }
    }

    /* lock the host */
    if (!hostIsLocked)
    {
        /* lock the host */
        if(emaLock((EMAElement) host))
            hostIsLocked = TRUE;
        else
            return cmTransErr;
    }

    session->Q931Connection = host;

    /* add the session to the host */
    meiEnter(transGlobals->lock);
    session->nextSession = host->firstSession;
    if (session->nextSession)
        session->nextSession->prevSession = session;
    meiExit(transGlobals->lock);
    host->firstSession = session;

    /* allocate a new tpkt element for the connection */
    if (host->state == hostIdle)
    {
	void * appHandle;
	int appIp;
        getGoodAddress(transGlobals,
                       host->hTpkt,
                       NULL,
                       cmTransQ931Conn,
                       &host->localAddress);

	appHandle = emaGetApplicationHandle((EMAElement)session);
	if(appHandle)
	{
		appIp = getAppIp(appHandle);
	}

        host->hTpkt = tpktOpen( transGlobals->tpktCntrl,
                                /*host->localAddress.ip, */ /* Ashish dmr */
								htonl(appIp),
                                host->localAddress.port,
                                tpktClient,
                                transQ931Handler,
                                (void *)host);
        if (host->hTpkt != NULL)
            host->state = hostConnecting;
        else
            res = cmTransErr;
    }

    /* update selected local address on the host */
    if (host->hTpkt)
    {
        getGoodAddress(transGlobals,
                       session->Q931Connection->hTpkt,
                       NULL,
                       cmTransQ931Conn,
                       &host->localAddress);

        if (localAddress)
            *localAddress = host->localAddress;
    }

    /* Unlock the host */
    emaUnlock((EMAElement) host);

    return res;
}

/**************************************************************************************
 * setAddressForH245Host
 *
 * Purpose: allocates a new H.245 host and start listenning on it.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          localAddress    - the local address to listen on.ng host, if exists.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransErr            - In case that an error occured.
 *                  cmTransOK             - All OK
 *
 **************************************************************************************/
TRANSERR setAddressForH245Host( IN    cmTransSession      *session,
                                IN    cmTransportAddress  *localAddress)
{
    TRANSERR res;
    cmTransHost *host = NULL;
    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    res = createHostByType(transGlobals,
                           (HSTRANSSESSION)session,
                           (HSTRANSHOST *)&host,
                           cmTransH245Conn,
                           NULL,
                           cmTransNoAnnexE);
    if (res != cmTransOK)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "setAddressForH245Host failed on allocating host element"));
        return cmTransErr;
    }
    else
    {
        TRANSERR reply = cmTransOK;

        /* start listenning */
        if (transGlobals->hostEventHandlers.cmEvTransHostListen)
        {
            HATRANSHOST haTransHost =
                         (HATRANSHOST)emaGetApplicationHandle(((EMAElement)host));

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransHostListen(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d))",
                    emaGetIndex((EMAElement)host),
                    host,
                    haTransHost,
                    localAddress->ip,
                    localAddress->port));

            /* no need to lock or to prepare for callback - the element is brand new */
            emaMark((EMAElement)host);
            reply = transGlobals->hostEventHandlers.cmEvTransHostListen(
                                    (HSTRANSHOST)host,
                                    haTransHost,
                                    cmTransH245Conn,
                                    localAddress);
            emaRelease((EMAElement)host);
            host->reported = TRUE;
        }

        if (reply == cmTransOK)
            host->h245Listen =
                    tpktOpen(transGlobals->tpktCntrl,
                             localAddress->ip,
                             localAddress->port,
                             tpktServer,
                             transH245AcceptHandler,
                            (void *)host);
        else
            host->h245Listen = NULL;

        /* get the allocated local address */
        getGoodAddress(transGlobals,
                       host->h245Listen,
                       NULL,
                       cmTransH245Conn,
                       localAddress);

        if (transGlobals->hostEventHandlers.cmEvTransHostListening)
        {
            int         numOfLocks;
            HATRANSHOST haTransHost =
                         (HATRANSHOST)emaGetApplicationHandle(((EMAElement)host));

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransHostListening(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d) error = %d)",
                    emaGetIndex((EMAElement)host),
                    host,
                    haTransHost,
                    localAddress->ip,
                    localAddress->port,
                    (host->h245Listen == NULL)));

            numOfLocks = emaPrepareForCallback((EMAElement)host);
            transGlobals->hostEventHandlers.cmEvTransHostListening(
                                    (HSTRANSHOST)host,
                                    haTransHost,
                                    cmTransH245Conn,
                                    localAddress,
                                    (host->h245Listen == NULL));
            emaReturnFromCallback((EMAElement)host, numOfLocks);
            host->reported = TRUE;
        }

        session->H245Connection = host;

        if ( (!host->h245Listen) && (reply != cmTransIgnoreMessage) )
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                     (transGlobals->hLog, RV_ERROR,
                     "setAddressForH245Host failed to initiate listen on H.245 TPKT"));
            return cmTransErr;
        }
        else
        if (reply == cmTransIgnoreMessage)
        {
            logPrint(transGlobals->hLog, RV_INFO,
                     (transGlobals->hLog, RV_INFO,
                     "setAddressForH245Host initiate listen on H.245 TPKT refused by user"));
        }
        else
        {
            host->localAddress = *localAddress;

            host->remoteAddress.type = (cmTransportType)-1;
            host->remoteAddress.ip   = 0;

            /* mark that the h245 host is in listenning mode */
            host->state = hostListenning;
        }

        host->firstSession = session;
    }

    return res;
}

/**************************************************************************************
 * setLocalAddressForQ931
 *
 * Purpose: allocates a new Q.931 host on a given local address.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          localAddress    - the local address.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransErr            - In case that an error occured.
 *                  cmTransOK             - All OK
 *
 **************************************************************************************/
TRANSERR setLocalAddressForQ931(cmTransSession *session, cmTransportAddress *localAddress)
{
    /* if we already have a host, just get its local address */
    if (session->Q931Connection)
    {
        if (emaLock((EMAElement)session->Q931Connection))
        {
                memcpy(localAddress,
                       &session->Q931Connection->localAddress,
                       sizeof(cmTransportAddress));
                emaUnlock((EMAElement)session->Q931Connection);
        }
        return cmTransOK;
    }
    else
    if (session->annexEConnection)
    {
        if (emaLock((EMAElement)session->annexEConnection))
        {
                memcpy(localAddress,
                       &session->annexEConnection->localAddress,
                       sizeof(cmTransportAddress));
                emaUnlock((EMAElement)session->annexEConnection);
        }
        return cmTransOK;
    }
    else
        /* we need to allocate a new host,
           go and find/create a Q.931 host for the given address */
        return setQ931Host(session, localAddress, NULL, NULL, TRUE);
}

/**************************************************************************************
 * setRemoteAddressForQ931
 *
 * Purpose: sets the remote address to a Q.931, assumes that a host exists..
 *
 * Input:   hsTransSession  - the handle to the session.
 *          localAddress    - the local address.
 *          remoteAddress   = the remote address.
 *          createNewHost   -   TRUE:  create new host even if multiplexed
 *                              FALSE: for multiplex use existing host, if exists.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransErr            - In case that an error occured.
 *                  cmTransOK             - All OK
 *
 **************************************************************************************/
TRANSERR setRemoteAddressForQ931(cmTransSession *session,
                                 cmTransportAddress *localAddress,
                                 cmTransportAddress *remoteAddress,
                                 BOOL               createNewHost)
{
    cmTransHost *oldHost = NULL;

    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* in case of multiplexing we need first to see if we have already a suitable host
       for the given address */
    if (session->isMultiplexed)
    {
        /* if the existing host is for the requested address,
           this is the host we need so do nothing */
        if (emaLock((EMAElement)session->Q931Connection))
        {
            if (transCompareAddresses(&session->Q931Connection->remoteAddress,
                                      remoteAddress))
            {
                emaUnlock((EMAElement)session->Q931Connection);
                return cmTransOK;
            }

            /* we need to look for a new host, so remember the old one,
               and look for a new one matching the address */
            oldHost                 = session->Q931Connection;
            session->Q931Connection = NULL;
            emaUnlock((EMAElement)oldHost);
        }

        /* Go and find/create a Q.931 host for the given address */
        return setQ931Host(session, localAddress, remoteAddress, oldHost, createNewHost);
    }
    else
    {
        /* for normal (non multiplexed) host, create one if not exists already */
        TRANSERR answer;
        BOOL     newHost = FALSE;

        if (!session->Q931Connection)
        {
            TRANSERR err;

            err = createHostByType(transGlobals,
                                   (HSTRANSSESSION)session,
                                   (HSTRANSHOST *)&session->Q931Connection,
                                   cmTransQ931Conn,
                                   NULL,
                                   cmTransNoAnnexE);

            if (err != cmTransOK)
            {
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "setRemoteAddressForQ931 failed on creating a new host"));
                return cmTransErr;
            }
            else
                newHost = TRUE;
        }

        /* now we surely have a host, set its addresses */
        if (emaLock((EMAElement)session->Q931Connection))
        {
            /* set the local address, if given */
            if (localAddress)
            {
                session->Q931Connection->localAddress.ip   = localAddress->ip;
                session->Q931Connection->localAddress.port = localAddress->port;
            }
            else
            {
                session->Q931Connection->localAddress.ip   = 0;
                session->Q931Connection->localAddress.port = 0;
            }

            /* Add the remote address to the host's hash */
            if (setRemoteAddress(session->Q931Connection, remoteAddress) == cmTransErr)
            {
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "setRemoteAddressForQ931 failed on updating hosts hash table [1]"));
                answer = cmTransErr;
            }
            else
            {
                /* all is well */
                session->Q931Connection->firstSession = session;
                answer = cmTransOK;
            }
            emaUnlock((EMAElement) session->Q931Connection);
        }
        else
            /* no host in this stage means a problem */
            answer = cmTransErr;

        return answer;
    }
}

/**************************************************************************************
 * allocateAnnexEHost
 *
 * Purpose: looks for an existing annex E host to the given address, if one doesn't
 *          exist allocate it.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          remoteAddress   - The remote address to connect to.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransErr            - In case that an error occured.
 *                  cmTransOK             - All OK.
 *
 **************************************************************************************/
void allocateAnnexEHost(cmTransSession *session, cmTransportAddress *remoteAddress)
{
    cmTransHost *host = session->annexEConnection;

    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* If no annex E host, try first to look for an existing one in the hash */
    if (!host)
        host = findHostInHash(transGlobals,
                              remoteAddress,
                              TRUE,
                              TRUE);
    /* Still no host, we must create a new one */
    if (!host)
    {
        TRANSERR res = createHostByType(transGlobals,
                                        (HSTRANSSESSION)session,
                                        (HSTRANSHOST *)&host,
                                        cmTransQ931Conn,
                                        NULL,
                                        cmTransUseAnnexE);
        if (res != cmTransOK)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "allocateAnnexEHost failed on allocating new host"));
            return;
        }
        else
        {
            /* Write the new host to the hash and get its local address */
            setRemoteAddress(host, remoteAddress);
            annexEGetLocalAddress(transGlobals->annexECntrl, &host->localAddress);
        }
    }

    session->annexEConnection = host;
}

/**************************************************************************************
 * cmTransSetAddress
 *
 * Purpose: sets the address, local and remote, or a host for a given session.
 *          This routine serves both Q.931 (TPKT & annex E) and H.245 connections.
 *
 * Input:   hsTransSession          - the handle to the session.
 *          localAddress            - the local address to connect from or listen on.
 *          remoteAddress           - The remote TPKT address to connect to.
 *          annexEremoteAddress     - The remote annex E address to connect to.
 *          hsTransHost             - An optional host that overrides the given addresses.
 *          type                    - Q.931 TPKT or H.245 addresses.
 *          newMultiplex            -   TRUE:  create new host even if multiplexed
 *                                      FALSE: for multiplex use existing host, if exists.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransErr            - In case that an error occured.
 *                  cmTransNotMultiplexed - The given host is not multiplexed.
 *                  cmTransOK             - In case that the connection is already opened.
 *
 **************************************************************************************/
TRANSERR cmTransSetAddress(IN    HSTRANSSESSION      hsTransSession,
                           INOUT cmTransportAddress  *localAddress,
                           IN    cmTransportAddress  *remoteAddress,
                           IN    cmTransportAddress  *annexEremoteAddress,
                           IN    HSTRANSHOST         hsTransHost,
                           IN    TRANSCONNTYPE       type,
                           IN    BOOL                newMultiplex)
{
    cmTransSession *session = (cmTransSession *)hsTransSession;
    cmTransHost    *host    = (cmTransHost *)hsTransHost;
    cmTransGlobals *transGlobals;
    TRANSERR       answer = cmTransOK;
    int            givenAddresses = 0;

    if (!hsTransSession)
        return cmTransErr;

    /* lock the session */
    if(emaLock((EMAElement) session))
    {
        /* get the global data of the module */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

        /* determine which type of addresses were given */
        if ( (localAddress) && (!remoteAddress) && (!annexEremoteAddress) )
            givenAddresses = justLocal;
        else
            givenAddresses = justRemote;

        /* according to the connection type, set the addresses */
        switch (type)
        {
            case cmTransH245Conn:
            {
                if ( (session->H245Connection) || (remoteAddress) )
                    return cmTransErr;
                else
                    answer = setAddressForH245Host(session, localAddress);
            }
            break;

            case cmTransQ931Conn:
            {
                /* treat the Q.931 according to the parameters given */

                /* In case that a host was given rather than addresses */
                if (emaLock((EMAElement)host))
                {
                    if ( (!host->isMultiplexed) || (!host->remoteIsMultiplexed) )
                    {
                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "allocateQ931Host can't use the given host, its not multiplexed"));
                        emaUnlock((EMAElement) host);
                        return cmTransHostNotMultiplexed;
                    }
                    else
                    if (host->annexEUsageMode != cmTransNoAnnexE)
                    {
                        session->annexEConnection = host;
                        session->Q931Connection   = NULL;
                    }
                    else
                    {
                        session->Q931Connection   = host;
                        session->annexEConnection = NULL;

                        /* add the session to the host */
                        meiEnter(transGlobals->lock);
                        session->nextSession = host->firstSession;
                        if (session->nextSession)
                            session->nextSession->prevSession = session;
                        meiExit(transGlobals->lock);
                        host->firstSession = session;

                    }

                    session->hostWasSet = TRUE;
                    emaUnlock((EMAElement) host);
                    answer = cmTransOK;
                }
                else
                /* Treat setting a local address for Q.931 */
                if (givenAddresses == justLocal)
                    answer = setLocalAddressForQ931(session, localAddress);
                else
                /* treat setting a remote address for Q.931 */
                if (givenAddresses == justRemote)
                {
                    /* If a host was given it overrides the address setting */
                    if (session->hostWasSet)
                    {
                        emaUnlock((EMAElement)session);
                        return cmTransOK;
                    }
                    /* do we need an annex E host:
                       if we have a session with a potential annex E connection
                       we need to create an annex E host */
                    if (session->annexEUsageMode != cmTransNoAnnexE)
                    {
                        if ( (annexEremoteAddress) && (transGlobals->annexESupported) )
                            allocateAnnexEHost(session, annexEremoteAddress);
                        else
                        {
                            logPrint(transGlobals->hLog, RV_ERROR,
                                    (transGlobals->hLog, RV_ERROR,
                                    "cmTransSetAddress needs annex E remote address session=%d(%x)",
                                    emaGetIndex((EMAElement)session), session));
                            answer = cmTransErr;
                        }
                    }
                    else
                    {
                        /* if we have annex E host, close it (probably will never occur) */
                        if (session->annexEConnection)
                        {
                            cmTransHost * annexEConnection = session->annexEConnection;
                            if(emaLock((EMAElement)annexEConnection))
                            {
                                transHostClose((HSTRANSHOST)annexEConnection, FALSE);
                                emaUnlock((EMAElement)annexEConnection);
                            }
                        }
                    }

                    /* do we need an TPKT host:
                       if we have a session with a potential TPKT connection
                       we need to create a TPKT host too */
                    if (session->annexEUsageMode != cmTransUseAnnexE)
                    {
                        if (remoteAddress)
                            answer = setRemoteAddressForQ931(session,
                                                             localAddress,
                                                             remoteAddress,
                                                             newMultiplex);
                        else
                        {
                            logPrint(transGlobals->hLog, RV_ERROR,
                                    (transGlobals->hLog, RV_ERROR,
                                    "cmTransSetAddress needs tpkt remote address session=%d(%x)",
                                    emaGetIndex((EMAElement)session), session));
                            answer = cmTransErr;
                        }
                    }
                    else
                    {
                        /* if we have TPKT host, close it (it may have been created for the local
                           address) */
                        if (session->Q931Connection)
                            closeHostInternal((HSTRANSHOST)session->Q931Connection, TRUE);
                    }
                }
                else
                {
                    /* No locala ddress nor remote one nor host, must be a mistake */
                    emaUnlock((EMAElement)session);
                    return cmTransErr;
                }
            }
            break;
        }
        emaUnlock((EMAElement)hsTransSession);
        return answer;
    }
    else
        /* no session was given !!! */
        return cmTransErr;
}


/**************************************************************************************
 * cmTransQ931Connect
 *
 * Purpose: Starts the process of connecting on behalf of the given session.
 *          For regular connection this will initiate a connect procedure,
 *          for multiplexed operation on an existing connection, this will
 *          report that a connection was established.
 *
 * Input:   hsTransSession  - the handle to the session.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransWouldBlock - In case that a connect procedure was instigated but
 *                                      not yet completed.
 *                  cmTransOK         - In case that the connection is already opened.
 *
 **************************************************************************************/
TRANSERR cmTransQ931Connect(IN HSTRANSSESSION   hsTransSession)
{
    cmTransSession      *session = (cmTransSession *)hsTransSession;
    cmTransGlobals      *transGlobals;
    TRANSINTCONNTYPE    type = (TRANSINTCONNTYPE)-1;
    TRANSERR            res;

    /* lock the session */
    if(emaLock((EMAElement) hsTransSession))
    {
        cmTransHost *q931host   = session->Q931Connection;
        cmTransHost *annexEhost = session->annexEConnection;

        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        /* if we have both connections for TPKT and Annex E we need
           to see if they are connected.
           If one is connected       - use it
           if both are connected     - choose acccording to the annexEUsageMode flag
           if both are NOT connected - make a competition
        */
        if ( (q931host) && (annexEhost) )
        {
            if (emaLock((EMAElement)q931host))
            {
                if (emaLock((EMAElement)annexEhost))
                {
                    if ((q931host->hostIsApproved) && (annexEhost->hostIsApproved))
                    {
                        if (session->annexEUsageMode == cmTransPreferedAnnexE)
                        {
                            type = cmTransAnnexEConn;
                            determineWinningHost(transGlobals, session, TRUE);
                        }
                        else
                        {
                            type = cmTransTPKTConn;
                            determineWinningHost(transGlobals, session, FALSE);
                        }
                    }
                    else
                    if (q931host->hostIsApproved)
                    {
                        type = cmTransTPKTConn;
                        determineWinningHost(transGlobals, session, FALSE);
                    }
                    else
                    if (annexEhost->hostIsApproved)
                    {
                        type = cmTransAnnexEConn;
                        determineWinningHost(transGlobals, session, TRUE);
                    }
                    else
                        type = cmTransTPKTAnnexECompetition;

                    emaUnlock((EMAElement)q931host);
                    emaUnlock((EMAElement)annexEhost);
                }
                else
                {
                    emaUnlock((EMAElement)q931host);
                    emaUnlock((EMAElement)hsTransSession);
                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "cmTransQ931Connect failed on lock of host %d(%x)",
                            emaGetIndex((EMAElement)session->annexEConnection), session->annexEConnection));
                    return cmTransErr;
                }
            }
            else
            {
                emaUnlock((EMAElement)hsTransSession);
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "cmTransQ931Connect failed on lock of host %d(%x)",
                        emaGetIndex((EMAElement)session->Q931Connection), session->Q931Connection));
                return cmTransErr;
            }
        }
        else
        if (q931host)
        {
            type = cmTransTPKTConn;
            determineWinningHost(transGlobals, session, FALSE);
        }
        else
        if (annexEhost)
        {
            type = cmTransAnnexEConn;
            determineWinningHost(transGlobals, session, TRUE);
        }
        else
        {
            /* A problem! We have no addresses, no given host, we
               can't connect */
            emaUnlock((EMAElement)hsTransSession);
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "cmTransQ931Connect received no host and no addresses"));
            return cmTransErr;
        }

        /* set the tunneling state of the session */
        if (session->isTunnelingSupported)
            session->tunnelingState = tunnPossible;
        else
            session->tunnelingState = tunnNo;

        /* connect the required host according to type */
        res = transHostConnect(hsTransSession, type);

        emaUnlock((EMAElement)hsTransSession);

        return res;
    }
    else
        return cmTransErr;

}

/**************************************************************************************
 * cmTransSendMessage
 *
 * Purpose: Decodes and sends a message for the given session thru its host connection.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          pvtNode         - The pvt of the given decoded message.
 *          type            - The type of the message (Q.931/H.245)
 *
 * Output:  None.
 *
 * Returned Value:  cmTransWouldBlock - In case that there aren't enough buffers.
 *                  cmTransOK         - In case that the send was successful.
 *                  cmTrandErr        - In case of fatal error.
 *
 **************************************************************************************/
TRANSERR cmTransSendMessage(IN HSTRANSSESSION   hsTransSession,
                            IN int              pvtNode,
                            IN TRANSTYPE        type)
{
    cmTransSession  *session    = (cmTransSession *)hsTransSession;
    cmTransHost     *host = NULL;
    cmTransGlobals  *transGlobals;
    BYTE            *buffer;
    BOOL            needToSend;
    int             encMsgSize;
    TRANSERR        res;
    BOOL            needToSaveSetupMessage = FALSE;
    int             msgType;
    void            *savedMessage;

    /* lock the session */
    if(emaLock((EMAElement) hsTransSession))
    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        msgType = pvtGetChildTagByPath(transGlobals->hPvt, pvtNode, "message", 1);

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "cmTransSendMessage called(hsTransSession=%d(%x), pvtNode=%d (type=%d), type=%d",
                emaGetIndex((EMAElement)hsTransSession), hsTransSession, pvtNode, msgType, type));


        if (pvtNode < 0)
        {
            /* unlock the session */
            emaUnlock((EMAElement) hsTransSession);
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "cmTransSendMessage called with wrong pvtNode"));
            return cmTransErr;
        }

        /* find the host to be used */
        switch (type)
        {
            case cmTransQ931Type:
            {
                if (msgType == cmQ931setup)
                {
                    if (session->annexEConnection)
                    {
                        host = session->annexEConnection;

                        /* if we also have TPKT host, we're in a race, so the SETUP will need to be sent again */
                        if (session->Q931Connection)
                            needToSaveSetupMessage = TRUE;
                    }
                    else
                        host = session->Q931Connection;
                }
                else
                {
                    if ( (session->annexEConnection) && (session->annexEConnection->hostIsApproved) )
                        host = session->annexEConnection;
                    else
                    if ( (session->Q931Connection) && (session->Q931Connection->hostIsApproved) )
                        host = session->Q931Connection;
                    else
                    {
                        /* unlock the session */
                        emaUnlock((EMAElement) hsTransSession);
                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "cmTransSendMessage no available host for non-setup message (%d-%x) [%x - %d, %x - %d]",
                                emaGetIndex((EMAElement)hsTransSession), hsTransSession,
                                session->Q931Connection, (session->Q931Connection?session->Q931Connection->hostIsApproved:0),
                                session->annexEConnection,(session->annexEConnection?session->annexEConnection->hostIsApproved:0)));
                        return cmTransErr;
                    }
                }

                break;
            }

            case cmTransH245Type:
            {
                /* if no tunneling, get the real H.245 host */
                if ((session->tunnelingState == tunnNo) ||
                    (session->tunnelingState == tunnRejected) ||
                    session->switchToSeparateWasAsked)
                {
                    if (!session->reportedH245Connect)
                    {
                        /* unlock the session */
                        emaUnlock((EMAElement) hsTransSession);
                        return cmTransErr;
                    }
                    else if ((session->H245Connection) && (session->H245Connection->dummyHost))
                    {
                        /* We have a reported existing host (i.e. no tunneling)
                        which has no TPKT object,
                        i.e. it's a dummy control session that is not connnected to
                        the network and thus not allowed to send messages */
                        emaUnlock((EMAElement) hsTransSession);
                        return cmTransOK;
                    }

                    host = session->H245Connection;
                    /* if h.245 doesn't exist and we're NOT in tunneling, reject message */
                    if (!host)
                    {
                        /* unlock the session */
                        emaUnlock((EMAElement) hsTransSession);
                        return cmTransErr;
                    }
                }
                break;
            }

            default:
                break;
        }

        /* lock the host */
        if (host)
            if(!emaLock((EMAElement)host))
            {
                emaUnlock((EMAElement) hsTransSession);

                logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "cmTransSendMessage failed to lock host = %x", host));

                return cmTransErr;
            }

        /* if a host was found or it's a tunneled message, handle the message */
        if ( (host) || (type == cmTransH245Type) )
        {
            BOOL waitForSetup = (!session->firstMessageSent && session->outgoing && (msgType != cmQ931setup));
            /* - Add fields to Q.931 messages
               - Save H.245 tunneled messages
               - Insert H.245 addresses, if necessary to the messages
               - Insert tunneled messages
               - on first message insert to CAT
            */

            /* We want to accumulate tunneled messages that might be
               instigated by the sending of this message.
               In case it's the first send we hold tunneling, in all other
               cases we leave it in hold state */
            if (!session->holdTunneling)
            {
                session->holdTunneling = TRUE;
                needToSend = processOutgoingMessage(session, host, pvtNode, type);
                session->holdTunneling = FALSE;
            }
            else
                needToSend = processOutgoingMessage(session, host, pvtNode, type);

            /* It might have been a tunneled messsage so no actual network activity is needed */
            if (!needToSend)
            {
                /* check if we need to send facility for the newly sent tunneled messages */
                initiateTunnelingFacility(transGlobals, session, host);

                /* release the locks and exit */
                if (host)
                    emaUnlock((EMAElement)host);
                emaUnlock((EMAElement)session);
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmTransSendMessage inhibited the actual send of the message"));
                return cmTransOK;
            }

            /* Start the encoding process */
            getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);

            res = transEncodeMessage(host,
                                     session,
                                     transGlobals,
                                     pvtNode,
                                     buffer,
                                     &encMsgSize);
            if (res != cmTransOK)
            {
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "cmTransSendMessage failed to obtain Encoding buffer res = %d",res));

                if (host)
                    emaUnlock((EMAElement)host);
                emaUnlock((EMAElement)session);
                return res;
            }

            savedMessage = saveMessageToPool(transGlobals,
                                            (void *)host,
                                            buffer,
                                            encMsgSize,
                                            FALSE,
                                            (msgType == cmQ931setup), /* setup must be the first to be sent */
                                            session->CRV);
            if ((savedMessage) && (needToSaveSetupMessage))
                savedMessage = saveMessageToPool(transGlobals,
                                                (void *)session->Q931Connection,
                                                buffer,
                                                encMsgSize,
                                                FALSE,
                                                TRUE,  /* setup must be the first to be sent */
                                                session->CRV);

            if (!savedMessage)
            {
                /* NO BUFFERS, add the session to the list to be notified on buffers released */
                addSessionToPendingList(transGlobals, session);
                if (host)
                    emaUnlock((EMAElement)host);
                emaUnlock((EMAElement)session);
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "cmTransSendMessage has no buffers for host %d(%x)",
                        emaGetIndex((EMAElement)host), host));
                return cmTransWouldBlock;
            }

            if (waitForSetup)
            {
                if (host)
                    emaUnlock((EMAElement)host);
                emaUnlock((EMAElement)session);
                logPrint(transGlobals->hLog, RV_DEBUG,
                        (transGlobals->hLog, RV_DEBUG,
                        "cmTransSendMessage type %d will wait for setup on host %d(%x)",
                        msgType, emaGetIndex((EMAElement)host), host));
                return cmTransOK;
            }

            /* initiate the actual send through the appropriate protocol */
            logPrint(transGlobals->hLog, RV_DEBUG,
                    (transGlobals->hLog, RV_DEBUG,
                    "cmTransSendMessage calls sendMessageOnHost"));

            res = sendMessageOnHost(transGlobals, host);
            if ( (res != cmTransOK) && (res != cmTransConnectionBusy) )
            {
                if (host)
                    emaUnlock((EMAElement)host);
                emaUnlock((EMAElement)session);
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "cmTransSendMessage Failed on send for host %d(%x)",
                        emaGetIndex((EMAElement)host), host));
                return cmTransErr;
            }

            /* check if the above message caused a startH245 facility message, if so send it */
            if (!emaWasDeleted((EMAElement)session))
                sendStartH245Facility(transGlobals, session);
        }
        else
        {
            emaUnlock((EMAElement)session);
            logPrint(transGlobals->hLog, RV_DEBUG,
                    (transGlobals->hLog, RV_DEBUG,
                    "cmTransSendMessage has no host !!!"));
            return cmTransErr;
        }

        if (host)
            emaUnlock((EMAElement)host);
        emaUnlock((EMAElement)session);


       logPrint(transGlobals->hLog, RV_INFO,
               (transGlobals->hLog, RV_INFO,
               "cmTransSendMessage succeeded !!!"));
        return cmTransOK;
    }
    else
        return cmTransErr;
}

/**************************************************************************************
 * cmTransSendH450Message
 *
 * Purpose: Sends a tunneled encoded message for the given session thru its host connection.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          buffer          - The encoded tunneled message.
 *          size            - The size of the encoded message.
 *          force           - Should we send a facility immediately with the message.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransWouldBlock - Last message was not sent yet, can't accept new one.
 *                  cmTransOK         - In case that the send was successful.
 *                  cmTrandErr        - In case of fatal error.
 *
 **************************************************************************************/
TRANSERR cmTransSendH450Message(IN HSTRANSSESSION   hsTransSession,
                                IN BYTE             *buffer,
                                IN int              size,
                                IN BOOL             force)
{
    cmTransSession  *session    = (cmTransSession *)hsTransSession;
    cmTransGlobals  *transGlobals;
    BOOL            newNodeWasCreated = FALSE;

    /* lock the session */
    if (emaLock((EMAElement) hsTransSession))
    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        /* if no H.450 elemnt exists in for the call, create it */
        if (session->h450Element < 0)
        {
            session->h450Element =  pvtAddRoot(transGlobals->hPvt,
                                               transGlobals->synProtH450,
                                               0,
                                               NULL);
            if (session->h450Element)
                newNodeWasCreated = TRUE;
            else
            {
                emaUnlock((EMAElement)session);
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "cmTransSendH450Message Failed on send for session %d(%x)",
                        emaGetIndex((EMAElement)session),session));
                return cmTransErr;
            }
        }

        /* add the new message to the H.450 element of the call */
        if (session->h450Element >= 0)
        {
            int nodeId;

            __pvtBuildByFieldIds(nodeId,
                                 transGlobals->hPvt,
                                 session->h450Element,
                                { _nul(0) LAST_TOKEN},
                                size, (char *)buffer);
            if (nodeId < 0)
            {
                /* if the element was just created, it's empty so get rid of it */
                if (newNodeWasCreated)
                {
                    pvtDelete(transGlobals->hPvt, session->h450Element);
                    session->h450Element = -1;
                }

                emaUnlock((EMAElement)session);
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "cmTransSendH450Message Failed on send for session %d(%x)",
                        emaGetIndex((EMAElement)session), session));
                return cmTransErr;
            }
        }

        /* check if we need to send facility for the newly sent tunneled messages */
        if (force)
        {
            session->forceTunneledMessage |= force;
            initiateTunnelingFacility(transGlobals, session, NULL);
        }

        emaUnlock((EMAElement)session);
        return cmTransOK;
    }
    else
        return cmTransErr;
}

/**************************************************************************************
 * cmTransSetH450Element
 *
 * Purpose: Sets the H.450 elements to send in the next outgoin message.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          elementNodeId   - PVT node ID of the new H.450 element.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransOK         - In case that the send was successful.
 *                  cmTrandErr        - In case of fatal error.
 *
 **************************************************************************************/
TRANSERR cmTransSetH450Element(IN HSTRANSSESSION   hsTransSession,
                               IN int              elementNodeId)
{
    cmTransSession  *session    = (cmTransSession *)hsTransSession;
    cmTransGlobals  *transGlobals;

    /* lock the session */
    if (emaLock((EMAElement) hsTransSession))
    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        /* if no H.450 element exists in for the call, create it */
        if (session->h450Element >= 0)
        {
            pvtDelete(transGlobals->hPvt, session->h450Element);
        }
        session->h450Element = elementNodeId;

        emaUnlock((EMAElement)session);
        return cmTransOK;
    }
    else
        return cmTransErr;
}

/**************************************************************************************
 * cmTransSendTunneledMessage
 *
 * Purpose: Sends a tunneled encoded message for the given session thru its host connection.
 *
 * Input:   hsTransSession  - the handle to the session.
 *          msg             - The node id of the encoded tunneled message.
 *          type            - The type of the message (AnnexM/AnnexL etc.)
 *          force           - Should we send a facility immediately with the message.
 *
 * Output:  None.
 *
 * Returned Value:  cmTransWouldBlock - Last message was not sent yet, can't accept new one.
 *                  cmTransOK         - In case that the send was successful.
 *                  cmTrandErr        - In case of fatal error.
 *
 **************************************************************************************/
TRANSERR cmTransSendTunneledMessage(IN HSTRANSSESSION   hsTransSession,
                                    IN int              msg,
                                    IN TRANSTYPE        type,
                                    IN BOOL             force)
{
    cmTransSession  *session = (cmTransSession *)hsTransSession;
    cmTransGlobals  *transGlobals;

    /* lock the session */
    if (emaLock((EMAElement) hsTransSession))
    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        switch (type)
        {
            case cmTransAnnexLType:
            {
                /* if no annex L element exists in for the call we can accept a new one */
                if (session->annexLElement < 0)
                    session->annexLElement = msg;
                else
                {
                    emaUnlock((EMAElement)session);
                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "cmTransSendTunneledMessage Failed on send annex L msg for session %d(%x)",
                            emaGetIndex((EMAElement)session), session));
                    return cmTransWouldBlock;
                }
            }
            break;

            case cmTransAnnexMType:
            {
                /* if no annex L element exists in for the call we can accept a new one */
                if (session->annexMElement < 0)
                    session->annexMElement = msg;
                else
                {
                    emaUnlock((EMAElement)session);
                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "cmTransSendTunneledMessage Failed on send annex M for session %d(%x)",
                            emaGetIndex((EMAElement)session), session));
                    return cmTransWouldBlock;
                }
            }
            break;

            default:
            {
                emaUnlock((EMAElement)session);
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "cmTransSendTunneledMessage illegal type = %d  for session %d(%x)",
                        type, emaGetIndex((EMAElement)session), session));
                return cmTransErr;
            }
        }

        /* check if we need to send facility for the newly sent tunneled messages */
        if (force)
        {
            session->forceTunneledMessage |= force;
            initiateTunnelingFacility(transGlobals, session, NULL);
        }

        emaUnlock((EMAElement)session);
        return cmTransOK;
    }
    else
        return cmTransErr;

}

/**************************************************************************************
 * cmTransEstablishControl
 *
 * Purpose: Starts a H.245 control for a given session after fast start or no control
 *          exists. If tunneling allowed, will use tunneling, else will open a "true"
 *          H.245 connection.
 *
 * Input:   hsTransSession  - the handle to the session.
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransEstablishControl(IN HSTRANSSESSION hsTransSession)
{
    cmTransSession *session = (cmTransSession *)hsTransSession;
    cmTransGlobals *transGlobals;

    BOOL stableFaststartState = FALSE;

    if (emaLock((EMAElement)session))
    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        /* if we are not establishing control or not supporting H.245 altogether, reject the
           call */
        if ( (session->notEstablishControl) || (session->h245Stage == cmTransH245Stage_noH245) )
        {
            emaUnlock((EMAElement)session);
            return cmTransErr;
        }

        /* determine if we are in a stable fast start state */
        switch (session->faststartState)
        {
            case fsApproved:
            case fsRejected:
            case fsNo:
            {
                stableFaststartState = TRUE;
            }
            break;

            default:
            break;
        }

        /* mark that openning control was asked */
        session->openControlWasAsked = TRUE;

        /* if we are in a stable state of fast start, decide what to do according
           to the tunneling state */
        if (stableFaststartState)
        {
            /* if tunneling exists, just report that a control was established */
            if (session->tunnelingState == tunnApproved)
                reportH245Establishment(transGlobals, session, NULL);
            else
            /* if tunneling does not exist, try to establish a real H.245 connection */
            if ( (session->tunnelingState == tunnNo) || (session->tunnelingState == tunnRejected) )
            {
                cmTransportAddress nullAddr;
				void * appHandle;
				unsigned long appIp;

				appHandle = emaGetApplicationHandle((EMAElement)session);
				if(appHandle)
				{
					appIp = getAppIp(appHandle);
				}

                nullAddr.ip = htonl(appIp);
                nullAddr.port = 0;
                nullAddr.type = cmTransportTypeIP;

                /* if no host exists yet, create it */
                if (!session->H245Connection)
                    cmTransSetAddress((HSTRANSSESSION)session,
                                      &nullAddr,
                                      NULL,
                                      NULL,
                                      NULL,
                                      cmTransH245Conn,
                                      TRUE);

                connectH245(transGlobals, session, cmQ931connect);

                /* check if the above message caused a startH245 facility message, if so send it */
                if (!emaWasDeleted((EMAElement)session))
                    sendStartH245Facility(transGlobals, session);
            }
        }

        emaUnlock((EMAElement)session);
        return cmTransOK;
    }
    else
        return cmTransErr;
}

/**************************************************************************************
 * cmTransSwitchToSeparate
 *
 * Purpose: Starts a "true" H.245 control connection for a given session.
 *          Can be called when the call is in Fast Start, tunneling or no control.
 *
 * Input:   hsTransSession  - the handle to the session.
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransSwitchToSeparate(IN HSTRANSSESSION hsTransSession)
{
    cmTransSession *session = (cmTransSession *)hsTransSession;
    cmTransGlobals *transGlobals;

    BOOL stableFaststartState = FALSE;
    BOOL stableTunnelingState = FALSE;

    if (emaLock((EMAElement)session))
    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        /* raise the flag, even if we don't really have control */
        session->switchToSeparateWasAsked = TRUE;

        /* if we are not establishing control or not supporting H.245 altogether, reject the
           call */
        if ( (session->notEstablishControl) || (session->h245Stage == cmTransH245Stage_noH245) )
        {
            emaUnlock((EMAElement)session);
            return cmTransErr;
        }

        /* determine if we are in a stable fast start state */
        switch (session->faststartState)
        {
            case fsApproved:
            case fsRejected:
            case fsNo:
            {
                stableFaststartState = TRUE;
            }
            break;

            default:
                break;
        }

        /* determine if we are in a stable tunneling state */
        switch (session->tunnelingState)
        {
            case tunnApproved:
            case tunnRejected:
            case tunnNo:
            {
                stableTunnelingState = TRUE;
            }
            break;

            default:
                break;
        }

        if ( (stableFaststartState) && (stableTunnelingState) )
        {
            cmTransportAddress nullAddr;

            nullAddr.ip = 0;
            nullAddr.port = 0;
            nullAddr.type = cmTransportTypeIP;

            /* if no host exists yet, create it */
            if (!session->H245Connection)
                cmTransSetAddress((HSTRANSSESSION)session,
                                  &nullAddr,
                                  NULL,
                                  NULL,
                                  NULL,
                                  cmTransH245Conn,
                                  TRUE);

            connectH245(transGlobals, session, cmQ931connect);

            /* check if the above message caused a startH245 facility message, if so send it */
            sendStartH245Facility(transGlobals, session);
        }

        emaUnlock((EMAElement)session);
        return cmTransOK;
    }
    else
        return cmTransErr;
}

/**************************************************************************************
 * cmTransCreateControlSession
 *
 * Purpose: Starts a "true" H.245 control connection, in case that H.245 is allowed.
 *
 * Input:   hsTransSession  -       the handle to the session with which the new H.245
 *                                  control is to be associated.
 *          addr            -       the address to listen or connect to.
 *          startListen     -       TRUE:  start listenning on the address.
 *                                  FALSE: try to connect to the addr.
 *          nullControlSession -    TRUE:  Create a dummy control session that is not connected
 *                                         to anywhere. (This imposes the startListen to FALSE).
 *                                  FALSE: Create a normal control session according to
 *                                         the given params.
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR cmTransCreateControlSession(IN  HSTRANSSESSION     hsTransSession,
                                     IN  cmTransportAddress *addr,
                                     IN  BOOL               startListen,
                                     IN  BOOL               nullControlSession)
{
    HSTRANSHOST    hsTransHost = NULL;
    cmTransHost    *host;
    cmTransSession *session = (cmTransSession *)hsTransSession;
    cmTransGlobals *transGlobals;
    TRANSERR       err;

    if (emaLock((EMAElement)session))
    {
        /* retrieve the transport module global area */
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "cmTransCreateControlSession(hsTransSession=%x, addr=[%x:%d], startListen=%d, nullControlSession=%d",
                hsTransSession, addr->ip, addr->port, startListen, nullControlSession));

        /* if we are not supporting H.245 altogather,
         exit */
        if ( session->h245Stage == cmTransH245Stage_noH245)
        {
            emaUnlock((EMAElement)session);
            return cmTransErr;
        }

        /* if we already have a H.245 on this session, reject it */
        if (session->H245Connection)
        {
            emaUnlock((EMAElement)session);
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "cmTransCreateControlSession was called with session having H.245 host"));
            return cmTransErr;
        }

        /* create a new host element */
        err = cmTransCreateHost((HAPPTRANS)transGlobals, NULL, &hsTransHost);
        if (err != cmTransOK)
        {
            emaUnlock((EMAElement)session);
            return err;
        }
        else
        {
            host = (cmTransHost *)hsTransHost;
            host->firstSession  = session;
            host->type          = cmTransH245Conn;

            /* Make sure this session haven't notified about H245 being connected... */
            session->reportedH245Connect = FALSE;
        }

        /* in case of NULL connection set the address to dummy one */
        if (nullControlSession)
        {
            addr->ip   = 0;
            addr->port = 0;
            addr->type = cmTransportTypeIP;
            addr->distribution = cmDistributionUnicast;

            startListen = FALSE;
        }

        /* if a listenning control session was requested */
        if (startListen)
        {
            TRANSERR reply = cmTransOK;

            /* get the address for listenning, in case of NULL connection
               just send NULL and nothing will happen */
            getGoodAddress(transGlobals,
                           host->h245Listen,
                           session->Q931Connection,
                           cmTransH245Conn,
                           addr);

            /* start a listenning process on the host */
            if (transGlobals->hostEventHandlers.cmEvTransHostListen)
            {
                int         numOfLocks;
                HATRANSHOST haTransHost =
                             (HATRANSHOST)emaGetApplicationHandle(((EMAElement)host));
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransHostListen(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d))",
                        emaGetIndex((EMAElement)host),
                        host,
                        haTransHost,
                        addr->ip,
                        addr->port));

                numOfLocks = emaPrepareForCallback((EMAElement)host);
                reply = transGlobals->hostEventHandlers.cmEvTransHostListen(
                                        (HSTRANSHOST)host,
                                        haTransHost,
                                        cmTransH245Conn,
                                        addr);
                emaReturnFromCallback((EMAElement)host, numOfLocks);
                host->reported = TRUE;
            }

            /* if user approved the listen and it's a real request,
               start listenning */
            if (reply == cmTransOK)
                host->h245Listen = tpktOpen( transGlobals->tpktCntrl,
                                             addr->ip, addr->port,
                                             tpktServer,
                                             transH245AcceptHandler, host);
            else
                host->h245Listen = NULL;

            /* set the host state, and save the address */
            getGoodAddress(transGlobals,
                           host->h245Listen,
                           session->Q931Connection,
                           cmTransH245Conn,
                           &host->localAddress);

            if (transGlobals->hostEventHandlers.cmEvTransHostListening)
            {
                int         numOfLocks;
                HATRANSHOST haTransHost =
                             (HATRANSHOST)emaGetApplicationHandle(((EMAElement)host));

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransHostListening(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d) error = %d)",
                        emaGetIndex((EMAElement)host),
                        host,
                        haTransHost,
                        host->localAddress.ip,
                        host->localAddress.port,
                        (host->h245Listen == NULL)));

                numOfLocks = emaPrepareForCallback((EMAElement)host);
                transGlobals->hostEventHandlers.cmEvTransHostListening(
                                        (HSTRANSHOST)host,
                                        haTransHost,
                                        cmTransH245Conn,
                                        &host->localAddress,
                                        (host->h245Listen == NULL));
                emaReturnFromCallback((EMAElement)host, numOfLocks);
            }

            /* the User wants the connection */
            if (reply != cmTransIgnoreMessage)
            {
                /* if not null connection and yet the listen failed
                   we have a real problem */
                if (!host->h245Listen)
                {
                    closeHostInternal(hsTransHost, TRUE);
                    emaUnlock((EMAElement)session);

                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "cmTransCreateControlSession Failed start listen on tpkt"));
                    return cmTransErr;
                }
            }
            else
            {
                closeHostInternal(hsTransHost, TRUE);
                emaUnlock((EMAElement)session);

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmTransCreateControlSession start listen on tpkt refused by user"));
                return cmTransErr;
            }

            *addr = host->localAddress;

            host->state = hostListenning;
        }
        else
        {
            TRANSERR reply = cmTransOK;

            /* set the remote address */
            if (!nullControlSession)
                setRemoteAddress(host, addr);

            /* start a connect to the given address */
            if (transGlobals->hostEventHandlers.cmEvTransHostConnecting)
            {
                int         numOfLocks;
                HATRANSHOST haTransHost =
                             (HATRANSHOST)emaGetApplicationHandle(((EMAElement)host));

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransHostConnecting(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d))",
                        emaGetIndex((EMAElement)host),
                        host,
                        haTransHost,
                        addr->ip,
                        addr->port));

                numOfLocks = emaPrepareForCallback((EMAElement)host);
                reply = transGlobals->hostEventHandlers.cmEvTransHostConnecting(
                                        (HSTRANSHOST)host,
                                        haTransHost,
                                        cmTransH245Conn,
                                        addr);
                emaReturnFromCallback((EMAElement)host, numOfLocks);
                host->reported = TRUE;
            }

            if ( (reply == cmTransOK) && (!nullControlSession) )
            {
                host->hTpkt = tpktOpen( transGlobals->tpktCntrl,
                                        transGlobals->localIPAddress, 0,
                                        tpktClient,
                                        transH245Handler, host);
                if (host->hTpkt)
                    tpktConnect(host->hTpkt, addr->ip, addr->port);
            }
            else
                host->hTpkt = NULL;

            /* the user wants the control session to connect */
            if (reply != cmTransIgnoreMessage)
            {
                /* a real problem occured */
                if ( (!host->hTpkt) && (!nullControlSession) )
                {
                    closeHostInternal(hsTransHost, TRUE);
                    emaUnlock((EMAElement)session);

                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "cmTransCreateControlSession Failed connect on tpkt"));
                    return cmTransErr;
                }
            }
            else
            {
                closeHostInternal(hsTransHost, TRUE);
                emaUnlock((EMAElement)session);

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmTransCreateControlSession connect on tpkt refused by user"));
                return cmTransErr;
            }

            /* set the host state and the addresses */
            if (nullControlSession)
            {
                host->localAddress = *addr;
            }
            else
                getGoodAddress(transGlobals,
                               host->hTpkt,
                               session->Q931Connection,
                               cmTransH245Conn,
                               &host->localAddress);

            host->remoteAddress     = *addr;

            host->state = hostConnecting;

            if (nullControlSession)
            {
                /* simulate a connected host */

                host->dummyHost = TRUE;

                /* release previous timer, if exists */
                if (session->h245ConnectTimer != (HTI)-1)
                {
                    mtimerReset(transGlobals->hTimers, session->h245ConnectTimer);
                    session->h245ConnectTimer = (HTI)-1;
                }

                /* set the timer with a CB routine according to the connecting host type */
                session->h245ConnectTimer = mtimerSet(transGlobals->hTimers,
                                                      connectH245Connection,
                                                      session,
                                                      0);
            }

            host->incomingMessage   = NULL;
        }

        /* connectthe H.245 host to the given session */
        session->H245Connection = host;

        emaUnlock((EMAElement)session);

        return cmTransOK;
    }
    return cmTransErr;
}

/**************************************************************************************
 * cmTransCloseControlSession
 *
 * Purpose: closes an H.245 connection for a given session.
 *
 * Input:   hsTransSession  - The handle of the new or old sesson associated with the
 *                            new H.245 control.
 *
 * Output: None.
 *
 * returned value: cmTransErr - in case of error, cmTransOK - otherwise
 *
 **************************************************************************************/
TRANSERR cmTransCloseControlSession(IN  HSTRANSSESSION  hsTransSession)
{
    cmTransSession *session=(cmTransSession *) hsTransSession;
    TRANSERR       err = cmTransOK;

    if (emaLock((EMAElement)session))
    {
        /* keep a local pointer to the host - we won't have one when we return*/
        cmTransHost * H245Connection = session->H245Connection;
        /* close tunneling (and parallel) */
        session->tunnelingState         = tunnNo;
        session->parallelTunnelingState = parllNo;

        /* kill the H.245 host, if exists */
        if (emaLock((EMAElement)H245Connection))
        {
            err = closeHostInternal((HSTRANSHOST)H245Connection, TRUE);
            /* aren't we lucky we kept a pointer to it? */
            emaUnlock((EMAElement)H245Connection);
        }
        else
            err = cmTransErr;

        emaUnlock((EMAElement)session);
    }
    else
        err = cmTransErr;

    return err;
}


/**************************************************************************************
 * cmTransTryControlAfterACF
 *
 * Purpose: sees if H245 control session should be opened after ACF.
 *
 * Input:   hsTransSession  - The handle of the new or old sesson associated with the
 *                            new H.245 control.
 *
 * Output: None.
 *
 * returned value: cmTransErr - in case of error, cmTransOK - otherwise
 *
 **************************************************************************************/
TRANSERR cmTransTryControlAfterACF(IN  HSTRANSSESSION  hsTransSession)
{
    cmTransSession *session = (cmTransSession *) hsTransSession;
    cmTransGlobals *transGlobals;
    
    if (emaLock((EMAElement)session))
    {
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

        if (((session->h245Stage == cmTransH245Stage_setup) || (session->h245Stage == cmTransH245Stage_early)) &&
            (needToOpenH245(session, FALSE)) &&
            (session->H245Connection != NULL) && (session->H245Connection->remoteAddress.ip != 0))
        {
            /* all good to go. trigger H.245 connection */
            connectH245(transGlobals, session, cmQ931setup);
        }
        else
        {
            /* or maybe this is tunneled */
            initiateTunnelingFacility(transGlobals, session, session->Q931Connection);
        }
        emaUnlock((EMAElement)session);
    }
    return cmTransOK;
}


/**************************************************************************************
 * cmTransHasControlSession
 *
 * Purpose: reports whether an H.245 connection exists and connected.
 *
 * Input:   hsTransSession  - The handle of the sesson associated with the H.245 control.
 *
 * Output: None.
 *
 * returned value: TRUE - The h245 exists and connected; FALSE - Otherwise
 *
 **************************************************************************************/
BOOL cmTransHasControlSession(IN  HSTRANSSESSION    hsTransSession)
{
    cmTransSession *session=(cmTransSession *) hsTransSession;
    BOOL answer = FALSE;

    if (emaLock((EMAElement)session))
    {
        answer = session->reportedH245Connect;
        emaUnlock((EMAElement)session);
    }

    return answer;
}

/**************************************************************************************
 * cmTransGetGoodAddressForH245
 *
 * Purpose: calculates the local address of the h.245
 *
 * Input:   hsTransSession  - The handle of the sesson associated with the H.245 control.
 *
 * Output:  addr            - local address of the H.245 connection
 *
 * returned value: TRUE - The h245 exists and connected; FALSE - Otherwise
 *
 **************************************************************************************/
void cmTransGetGoodAddressForH245(IN    HSTRANSSESSION     hsTransSession,
                                  INOUT cmTransportAddress *addr)
{
    cmTransSession *session = (cmTransSession *) hsTransSession;

    if (emaLock((EMAElement)session))
    {
        cmTransGlobals *transGlobal = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransSession);
        getGoodAddress(transGlobal,
                       (session->H245Connection)?session->H245Connection->hTpkt:NULL,
                       session->Q931Connection,
                       cmTransH245Conn,
                       addr);
        emaUnlock((EMAElement)session);
    }
}


/**************************************************************************************
 * cmTransGetHApp
 *
 * Purpose: gets the hApp element according to the given host
 *
 * Input:   hsTransHost  - The handle of the host.
 *
 * returned value:  hAppATrans   - The user associated handle to the module instance.
 *
 **************************************************************************************/
HAPPATRANS cmTransGetHApp(IN HSTRANSHOST hsTransHost)
{
    cmTransGlobals  *transGlobals;
    HAPPATRANS      answer = NULL;

    if (emaLock((EMAElement)hsTransHost))
    {
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);
        answer = transGlobals->hAppATrans;
        emaUnlock((EMAElement)hsTransHost);
    }

    return answer;
}

/**************************************************************************************
 * cmTransGetHostSocket
 *
 * Purpose: gets the socket allocated for the given host
 *
 * Input:   hsTransHost  - The handle of the host.
 *
 * returned value:  The socket number (-1 of not allocated yet).
 *
 **************************************************************************************/
int cmTransGetHostSocket(IN HSTRANSHOST hsTransHost)
{
     cmTransHost *host = (cmTransHost *)hsTransHost;
     int         socket = -1;

     if (emaLock((EMAElement)host))
     {
         if (host->hTpkt)
            socket = tpktGetSock(host->hTpkt);
         emaUnlock((EMAElement)host);
     }

     return socket;
 }



/**************************************************************************************
 * cmTransSetHostApplHandle
 *
 * Purpose: set the application handle for the given host
 *
 * Input:   hsTransHost  - The handle of the host.
 *          haTransHost  - The application handle to be set.
 *
 **************************************************************************************/
TRANSERR cmTransSetHostApplHandle(IN  HSTRANSHOST hsTransHost,
                                  IN  HATRANSHOST haTransHost)
{
    cmTransGlobals  *transGlobals;
    cmTransHost *host = (cmTransHost *)hsTransHost;

    if (emaLock((EMAElement)host))
    {
        emaSetApplicationHandle((EMAElement)host, (void *)haTransHost);

        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);
        logPrint(transGlobals->hLog, RV_DEBUG,
           (transGlobals->hLog, RV_DEBUG,
           "cmTransSetHostApplHandle set host %p with handle %p", hsTransHost, haTransHost));

        emaUnlock((EMAElement)host);
    }

    return cmTransOK;
}

/**************************************************************************************
 * cmTransGetHostApplHandle
 *
 * Purpose: get the application handle for the given host
 *
 * Input:   hsTransHost  - The handle of the host.
 *
 * Output:  haTransHost  - The application handle.
 *
 **************************************************************************************/
TRANSERR cmTransGetHostApplHandle(IN  HSTRANSHOST  hsTransHost,
                                  OUT HATRANSHOST *haTransHost)
{
    cmTransGlobals  *transGlobals;
    cmTransHost *host = (cmTransHost *)hsTransHost;

    if (emaLock((EMAElement)host))
    {
        *haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);
        logPrint(transGlobals->hLog, RV_DEBUG,
           (transGlobals->hLog, RV_DEBUG,
           "cmTransGetHostApplHandle got host %p with handle %p", hsTransHost, *haTransHost));

        emaUnlock((EMAElement)host);
    }

    return cmTransOK;
}


/**************************************************************************************
 * cmTransHostSendMessage
 *
 * Purpose: send a given message on the the given host, no processing is done on the
 *          message.
 *
 * Input:   hsTransHost  - The handle of the host on which to send the message.
 *          pvtNode      - The decoded message to be sent.
 *
 **************************************************************************************/
TRANSERR cmTransHostSendMessage(IN HSTRANSHOST hsTransHost,
                                IN int         pvtNode)
{
     cmTransHost    *host = (cmTransHost *)hsTransHost;
     cmTransGlobals *transGlobals;
     TRANSERR       answer = cmTransOK;

     if (emaLock((EMAElement)host))
     {
        transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);
        if (!encodeAndSend(transGlobals, host, pvtNode))
        {
            answer = cmTransErr;
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "cmTransHostSendMessage Failed for host %d(%x)",
                    emaGetIndex((EMAElement)host), host));
        }
        emaUnlock((EMAElement)host);
     }

    return answer;
}

/**************************************************************************************
 * cmTransGetHostsStats
 *
 * Purpose: retrieves the current and max used number of hosts.
 *
 * Input:   happTrans - The handle to the instance of the transport module.
 *
 * Output:  curSize  - The current amount of hosts used (Q.931 (TPKT & Annex E) + H.245)
 *          maxSize  - The maximum amount used so far of hosts (Q.931 (TPKT & Annex E) + H.245)
 *
 **************************************************************************************/
TRANSERR cmTransGetHostsStats(IN HAPPTRANS hAppTrans, IN int *curSize, IN int *maxSize)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;
    emaStatistics stat;

    stat.elems.cur = 0;
    stat.elems.maxUsage = 0;


    /* if the module was initialized */
    if (hAppTrans)
    {
        if (emaGetStatistics(transGlobals->hEmaHosts, &stat) < 0)
            return cmTransErr;
    }
    else
        return cmTransErr;

    if (curSize)
        *curSize = stat.elems.cur;

    if (maxSize)
        *maxSize = stat.elems.maxUsage;

    return cmTransOK;
}

/**************************************************************************************
 * cmTransGetMessagesStats
 *
 * Purpose: retrieves the current and max used number of messages saved before sending.
 *
 * Input:   happTrans - The handle to the instance of the transport module.
 *
 * Output:  curSize  - The current amount of messages in the pool.
 *          maxSize  - The maximum amount that was so far in the pool.
 *
 **************************************************************************************/
TRANSERR cmTransGetMessagesStats(IN HAPPTRANS hAppTrans, IN int *curSize, IN int *maxSize)
{
    cmTransGlobals *transGlobals = (cmTransGlobals *)hAppTrans;

    if (!hAppTrans)
        return cmTransErr;

    if (curSize)
        (*curSize) = transGlobals->curUsedNumOfMessagesInRpool;

    if (maxSize)
        (*maxSize) = transGlobals->maxUsedNumOfMessagesInRpool;

    return cmTransOK;
}

/**************************************************************************************
 * cmTransIsParallel
 *
 * Purpose: tells whether we are in parallel (fast start and tunneling) operation.
 *
 * Input:   hsTransSession - the session.
 *
 * Output:  None.
 *
 * Returned Value: TRUE  - we are in parallel operation.
 *                 FALSE - Otherwise.
 **************************************************************************************/
BOOL cmTransIsParallel(IN HSTRANSSESSION hsTransSession)
{
    cmTransSession *session = (cmTransSession *)hsTransSession;

    if (session)
        return (session->parallelTunnelingState == parllApproved);
    else
        return FALSE;
}
#ifdef __cplusplus
}
#endif /* __cplusplus*/
