#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/


/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


#include <transStates.h>
#include <transportint.h>
#include <transutil.h>
#include <q931asn1.h>
#include <transnet.h>
#include <cmutils.h>
#include <emanag.h>
#include <prnutils.h>
#include <psyntreeStackApi.h>

/**************************************************************************************
 * setMultiplexedParams
 *
 * putrpose: to set the multiplexed parameters into an outgoing message. The parameters
 *           are taken from the host parameters which are constatntly updated by incoming
 *           messages and user API calls.
 *
 * Input: transGlobals - The global data of the module.
 *        session      - the session from which the message is sent.
 *        host         - the host on which the message is to be sent.
 *        pvtNode      - the message into which the parameters are to be set.
 *        msgType      - which message we are dealing with.
 *
 ***************************************************************************************/
void setMultiplexedParams(cmTransGlobals *transGlobals,
                          cmTransSession *session,
                          cmTransHost *host,
                          int pvtNode,
                          int msgType)
{
    int res;
    BOOL multipleCallsValue;
    BOOL maintainConnectionValue;

    switch (msgType)
    {
        case cmQ931alerting:
        case cmQ931callProceeding:
        case cmQ931connect:
        case cmQ931setup:
        case cmQ931facility:
        case cmQ931progress:
        {

            /* we want to put theses parameters just to non-annex E messages */
            if (host->annexEUsageMode == cmTransNoAnnexE)
            {
                /* regular TPKT connection */
                multipleCallsValue = ((host->isMultiplexed /*&& (host->remoteIsMultiplexed || msgType == cmQ931setup)*/));
                maintainConnectionValue =  ((!host->closeOnNoSessions)/*&& (!host->remoteCloseOnNoSessions)*/);
            }
            else
            if ( (msgType == cmQ931setup) && (session->Q931Connection) )
            {
                /* for SETUP message we want to put the parameters also when having both annex E host
                   and TPKT host, since we don't know who will win the race. We use the TPKT host params */
                multipleCallsValue = ((session->Q931Connection->isMultiplexed &&
                                       (session->Q931Connection->remoteIsMultiplexed || msgType == cmQ931setup)));
                maintainConnectionValue =  ((!session->Q931Connection->closeOnNoSessions) &&
                                            (!session->Q931Connection->remoteCloseOnNoSessions));
            }
            else
            {
                /* annex E host */
                multipleCallsValue      = FALSE;
                maintainConnectionValue = TRUE;
            }

            /* set the isMultiplexed element according to the host settings */
            __pvtBuildByFieldIds(res, transGlobals->hPvt, pvtNode,
                                    { _q931(message)
                                      _anyField
                                      _q931(userUser)
                                      _q931(h323_UserInformation)
                                      _q931(h323_uu_pdu)
                                      _q931(h323_message_body)
                                      _anyField
                                      _q931(multipleCalls)
                                      LAST_TOKEN
                                    },
                                 multipleCallsValue, NULL);

            /* set the maintainConnection element according to the host settings */
            __pvtBuildByFieldIds(res, transGlobals->hPvt, pvtNode,
                                    { _q931(message)
                                      _anyField
                                      _q931(userUser)
                                      _q931(h323_UserInformation)
                                      _q931(h323_uu_pdu)
                                      _q931(h323_message_body)
                                      _anyField
                                      _q931(maintainConnection)
                                      LAST_TOKEN
                                    },
                                 maintainConnectionValue, NULL);
        }

        break;

        default:
            break;
    }
}

/**************************************************************************************
 * setTheFastStartStateByMessage
 *
 * putrpose: to determine the faststart state of the session according to its previous
 *           state and the data within the outgoing message. This routine handles all
 *           outgoing messages (setup for outgoing calls and all the rest for incoming
 *           calls. The routine modifies the session state variables!
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        messageBodyNode   - the UU-IE part of the message to be inspected.
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
void setTheFastStartStateByMessage(cmTransGlobals *transGlobals,
                                   cmTransSession *session,
                                   int            messageBodyNode,
                                   int            msgType)
{
    int res;

    /* get the fastConnectRefuse element */
    __pvtGetNodeIdByFieldIds(   res,
                                transGlobals->hPvt,
                                messageBodyNode,
                                {   _q931(h323_message_body)
                                    _anyField
                                    _q931(fastConnectRefused)
                                    LAST_TOKEN
                                });

    if (res<0)
    {
        /* there was NO refusal, check if there was a fastStart elemnt in the message */
        __pvtGetNodeIdByFieldIds(   res,
                                    transGlobals->hPvt,
                                    messageBodyNode,
                                    {   _q931(h323_message_body)
                                        _anyField
                                        _q931(fastStart)
                                        LAST_TOKEN
                                    });
        if (msgType == cmQ931setup)
        {
            if (res < 0)
                /* there was no fastStart element in the message */
                /* for SETUP that means that no faststart on the call was even offered */
                session->faststartState = fsNo;
            else
                /* there WAS a fastStart element in the message */
                /* that means for SETUP that fast start was offered on the call */
                session->faststartState = fsOffered;
        }
        else
        {
            /* for all other messages */
            if (res < 0)
            {
                /* there WAS not a fastStart element in the message */
                /* this means that the fast start was not answered yet,
                   however, if we're already in the CONNECT message
                   and no response was given, that means a reject */
                if (msgType == cmQ931connect)
                    if (session->faststartState == fsOffered)
                        session->faststartState = fsRejected;
            }
            else
                /* there WAS a fastStart element in the message */
                /* this means that the fast start was approved */
                session->faststartState = fsApproved;
        }
    }
    else
    {
        /* There was a refusal, no fast start for this call */
        session->faststartState = fsRejected;
    }
}

/**************************************************************************************
 * setTheTunnelingStateByMessage
 *
 * putrpose: to determine the tunneling state of the session according to its previous
 *           state and the data within the incoming message. This routine handles all
 *           incoming messages (setup for incoming calls and all the rest for outgoing
 *           calls. The routine modifies the session state variables!
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        messageBodyNode   - the UU-IE part of the message to be inspected.
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
void setTheTunnelingStateByMessage(cmTransGlobals *transGlobals,
                                   cmTransSession *session,
                                   int            messageBodyNode,
                                   int            msgType)
{
    int res;
    int bTunneling, bProvisional;

    /* get the provisionalRespToH245Tunneling element */
    __pvtGetNodeIdByFieldIds(res, transGlobals->hPvt,messageBodyNode,
                        { _q931(provisionalRespToH245Tunneling)
                          LAST_TOKEN
                        });

    if (res < 0)
        bProvisional = FALSE;
    else
        bProvisional = TRUE;

    if (!bProvisional)
    {
        /* get the h245Tunneling element */
        __pvtGetByFieldIds(res, transGlobals->hPvt,messageBodyNode,
                            { _q931(h245Tunneling)
                              LAST_TOKEN
                            },
                            NULL,(INT32 *)&bTunneling,NULL);
        if (res < 0)
            bTunneling = FALSE;
    }
    else
        bTunneling = FALSE;

    /* for incoming setup message: if tunneling exists in the message and the
       session supports it, approve the tunneling */
    if (msgType == cmQ931setup)
    {
        if (session->isTunnelingSupported)
        {
            if (bTunneling)
                session->tunnelingState = tunnApproved;
            else
                session->tunnelingState = tunnRejected;
        }
        else
            session->tunnelingState = tunnNo;
    }
    else
    {
        /* for all other messages (outgoing calls): if we offered tunneling and the message
           has it, approve it */
        if (!bProvisional)
        {
            if (bTunneling)
            {
                if (session->tunnelingState == tunnOffered)
                {
                    void *nextMsg;
                    /* the element exists and is TRUE */
                    session->tunnelingState = tunnApproved;

                    /* clean the saved but sent tunneled messages */
                    if (session->parallelTunnelingState != parllOffered)
                    {
                        nextMsg = session->firstMessage;
                        while (nextMsg)
                            nextMsg = extractMessageFromPool(transGlobals, session, TRUE);
                    }
                }
            }
            else
                /* the element exists and is FALSE */
                session->tunnelingState = tunnRejected;
        }
    }

    /* if we reached a CONNECT with no response that means a reject */
    if ( (session->tunnelingState == tunnOffered) && (msgType == cmQ931connect) )
        session->tunnelingState = tunnRejected;
}

/**************************************************************************************
 * setTheParallelTunnelingStateByMessage
 *
 * putrpose: to determine the parallel tunneling state of the session according to its previous
 *           state and the faststart and tunneling states of the session.
 *           This routine handles all outgoing messages (setup for outgoing calls and all
 *           the rest for incoming calls. The routine modifies the session state variables!
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgType           - The type of the message (setup or other)
 *        msgBody           - the node to the message UU-IE part
 *        outgoing          - TRUE: an outgoing message; FALSE - an incoming message.
 *
 ***************************************************************************************/
void setTheParallelTunnelingStateByMessage(cmTransGlobals *transGlobals,
                                           cmTransSession *session,
                                           int            msgType,
                                           int            msgBody,
                                           BOOL           outgoing)
{
    /* for SETUP check if we have both tunneling and faststart and we support
       the parallel mode. */
    if (msgType == cmQ931setup)
    {
        if (outgoing)
        {
            if ((session->faststartState == fsOffered) &&
                ( (session->tunnelingState == tunnOffered) ||
                  (session->tunnelingState == tunnPossible) )&&
                (session->isParallelTunnelingSupported)
               )
                session->parallelTunnelingState = parllOffered;
            else
                session->parallelTunnelingState = parllNo;
        }
        else
        {   /* incoming setup */
            int res = -1;

            /* check if parallel tunneling was offered */
            __pvtGetNodeIdByFieldIds(res, transGlobals->hPvt, msgBody,
                                        {   _q931(h323_message_body)
                                            _q931(setup)
                                            _q931(parallelH245Control)
                                            LAST_TOKEN
                                        });

            if ( (session->isParallelTunnelingSupported) && (res >=0) )
                session->parallelTunnelingState = parllApproved;
        }
    }
}

/**************************************************************************************
 * needToOpenH245
 *
 * putrpose: to determine according to the session params whether
 *           we shall want to open an H.245 (actually doing the connect).
 * Input: session           - the session on which the message is to be sent.
 *        forceOpen         - we must open (due to a startH245 facility).
 *
 ***************************************************************************************/
BOOL needToOpenH245(cmTransSession *session, BOOL forceOpen)
{
    BOOL needToOpen = FALSE;

    {
        if (forceOpen)
            /* we were forced, so we obey */
            needToOpen = TRUE;
		else
        {
	        /* in case of no forcing, we need to check if we may open an 
			   H.245 connection according to the session's tunneling state
			   and fast start state */

            switch (session->tunnelingState)
            {
                case tunnNo:
                case tunnRejected:
                { 
			        if ( (session->faststartState == fsNo) ||
						 (session->faststartState == fsRejected) )
				        /* this the normal case (no tunneling, no faststart),
						   go ahead and try to open an H.245 connection */
					    needToOpen = TRUE;
					else
					{
						/* we don't have tunneling but we do have a fast start 
						   on the session so only an explicit user request may 
						   open a real H.245 connection, and even that only 
                           when FS is stable */
	                    if( ((session->openControlWasAsked) || (session->switchToSeparateWasAsked)) &&
                            (session->faststartState != fsOffered) )
			                needToOpen = TRUE;
					}
                    break;
                }
                case tunnApproved:
                {
                    /* we have tunneling, 
					   only user request for a sepearate connection may open */
                    if (session->switchToSeparateWasAsked)
                        needToOpen = TRUE;
                    break;
                }
                default:
				{
					/* all other cases mean that we are still not in a stable tunneling state
					   so we shouldn't open H.245 yet */
					needToOpen = FALSE;
                    break;
				}
            }
        }
    }

    return needToOpen;
}
/**************************************************************************************
 * determineIfToOpenH245
 *
 * putrpose: This routine checks if it's time to start a H.245 connection either by:
 *				- listenning if we have an H.245 address and it's time according to the 
 *				  H.245 stage to send that address. If so, the address is sent and the 
 *				  module starts listenning on that address. 
 *				- trying to connect, if we have the remotes address.
 *
 * Input: outgoing          - TRUE - if an outgoing message, FALSE - if incoming one.
 *        transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        messageBodyNode   - the UU-IE part of the message to be inspected.
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
BOOL determineIfToOpenH245(BOOL           outgoing,
                           cmTransGlobals *transGlobals,
                           cmTransSession *session,
                           int            messageBodyNode,
                           int            msgType)
{
    int   res;
    BOOL  proceed = FALSE;
    BOOL  forceOpen = FALSE;
    BOOL  needToOpen;

	/* Phase 1: Determine if the stage allows any H.245 operation */

    /* according to the message type and the H245 stage parameter
       determine whether we can do any operation now */
    switch (msgType)
    {
        case cmQ931setup:
        {
            if ( (session->h245Stage == cmTransH245Stage_setup) ||
                 (session->h245Stage == cmTransH245Stage_early) )
                proceed = TRUE;
            break;
        }

        case cmQ931callProceeding:
        {
            if ( (session->h245Stage == cmTransH245Stage_setup) ||
                 (session->h245Stage == cmTransH245Stage_callProceeding) )
                proceed = TRUE;
            break;
        }
        case cmQ931alerting:
        {
            if ( (session->h245Stage == cmTransH245Stage_setup) ||
                 (session->h245Stage == cmTransH245Stage_callProceeding) ||
                 (session->h245Stage == cmTransH245Stage_alerting) )
                proceed = TRUE;
            break;
        }
        case cmQ931connect:
        {
            if ( (session->h245Stage == cmTransH245Stage_setup) ||
                 (session->h245Stage == cmTransH245Stage_callProceeding) ||
                 (session->h245Stage == cmTransH245Stage_alerting) ||
                 (session->h245Stage == cmTransH245Stage_connect) ||
                 (session->h245Stage == cmTransH245Stage_early) )
                proceed = TRUE;
            break;
        }
    }

	/* Phase 2: Check for incoming startH245 FACILITY message */

    /* first check if we are forced to open H.245 by an incoming
       FACILITY with startH245 reason */

    if ( (!outgoing) && (msgType == cmQ931facility) )
    {
        int node;

        /* get the reason */
        __pvtGetNodeIdByFieldIds(node, transGlobals->hPvt, messageBodyNode,
                                { _q931(h323_message_body)
                                  _q931(facility)
                                  _q931(reason)
                                  _q931(startH245)
                                  LAST_TOKEN
                                });
        if (node >= 0)
        {
			/* if we don't support H.245 we must reply with a FACILITY with noH245 */
	        if (session->h245Stage == cmTransH245Stage_noH245)
	        {
	            int newMessage;

	            /* we do not support H.245, report it with facility with NoH245 reason */
	            if (createMessage(transGlobals,
	                              cmQ931facility,
	                              session->CRV,
	                              session->callId,
	                              &newMessage)
	               )
	            {
	                /* set the reason */
	                int nodeId;
	                TRANSERR err;
	                /* set reason */
	                __pvtBuildByFieldIds(nodeId, transGlobals->hPvt, newMessage,
	                                        { _q931(message)
	                                          _q931(facility)
	                                          _q931(userUser)
	                                          _q931(h323_UserInformation)
	                                          _q931(h323_uu_pdu)
	                                          _q931(h323_message_body)
	                                          _q931(facility)
	                                          _q931(reason)
	                                          _q931(noH245)
	                                         LAST_TOKEN
	                                        },
	                                     0, NULL);

	                err = cmTransSendMessage((HSTRANSSESSION)session, newMessage, cmTransQ931Type);
	                pvtDelete(transGlobals->hPvt, newMessage);
	                if (err != cmTransOK)
	                {
	                    logPrint(transGlobals->hLog, RV_ERROR,
	                            (transGlobals->hLog, RV_ERROR,
	                            "connectH245 failed to send facility (noH245) message"));
						return FALSE;
	                }
	            }
	            else
	            {
	                logPrint(transGlobals->hLog, RV_ERROR,
	                         (transGlobals->hLog, RV_ERROR,
	                         "connectH245 failed to create facility (noH245)"));
					return FALSE;
	            }

				return TRUE;
	        }
			else
			{
	            proceed = TRUE;
		        forceOpen = TRUE;
			}
        }
    }

	/* Phase 3: Start an H.245 connection if necessary, either as a listener or 
	            as a connector */


    /* if we're in the right stage we start the process of dealing with H.245 */
    if (proceed)
    {
        /* check if we need to issue a connect on the H.245 */
        needToOpen = needToOpenH245(session, forceOpen);

        /* if the H245 host already exists, lock it */
        if (session->H245Connection)
            if (!emaLock((EMAElement)session->H245Connection))
                return FALSE;

        /* if it's an outgoing message */
        if (outgoing)
        {
            int   addressNode;

			/* if we need to open the connection and we do have the remote address
			   to open to, just do it! */
			if (needToOpen)
			{
				if ( (session->H245Connection) && 
					 (session->H245Connection->remoteAddress.ip != 0) )
				{					
					connectH245(transGlobals, session, msgType);

	                /* check if the above message caused a startH245 facility message, if so send it */
		            if (!emaWasDeleted((EMAElement)session))
			            sendStartH245Facility(transGlobals, session);
				}
			}

			/* if we don't have yet an H.245 host (and also no remote address)
			   create such a host and start a listen on it */
			if (!session->H245Connection)
	        {
		        TRANSERR err;
                cmTransportAddress h245LocalAddr;

                /* get a local address for the listenning host */
                memset(&h245LocalAddr, 0, sizeof(h245LocalAddr));
                h245LocalAddr.type = cmTransportTypeIP;
                getGoodAddress(transGlobals,
                               0,
                               session->Q931Connection,
                               cmTransH245Conn,
                               &h245LocalAddr);
                h245LocalAddr.port  = 0;

				/* this will allocate the H.245 host and allocate a listenning port */
                err = cmTransSetAddress((HSTRANSSESSION)session,
                                         &h245LocalAddr,
                                         NULL,
                                         NULL,
                                         NULL,
                                         cmTransH245Conn,
                                         TRUE);
                if (err != cmTransOK)
                {
                    logPrint(transGlobals->hLog, RV_ERROR,
                             (transGlobals->hLog, RV_ERROR,
                             "determineIfToOpenH245 failed to create a H.245 host"));
                    return FALSE;
                }
                else
				/* we have a new host, lock it */
                if (!emaLock((EMAElement)session->H245Connection))
                    return FALSE;
            }

            /* If the host is still closed and we don't have a remote address,
			   that means that we need to start a listenning process on it */
            if ( (session->H245Connection->state == hostIdle) &&
                 (session->H245Connection->remoteAddress.ip == 0) )
            {
                TRANSERR    reply = cmTransOK;

                /* Call the call back that we are about to start listenning */
                if (transGlobals->hostEventHandlers.cmEvTransHostListen)
                {
                    int         numOfLocks;
                    HATRANSHOST haTransHost =
                                 (HATRANSHOST)emaGetApplicationHandle(((EMAElement)session->H245Connection));

                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransHostListen(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d))",
                            emaGetIndex((EMAElement)session->H245Connection),session->H245Connection,
                            haTransHost,
                            session->H245Connection->localAddress.ip,
                            session->H245Connection->localAddress.port));

                    numOfLocks = emaPrepareForCallback((EMAElement)session->H245Connection);
                    reply = transGlobals->hostEventHandlers.cmEvTransHostListen(
                                            (HSTRANSHOST)session->H245Connection,
                                            haTransHost,
                                            cmTransH245Conn,
                                            &session->H245Connection->localAddress);
                    emaReturnFromCallback((EMAElement)session->H245Connection, numOfLocks);
                    session->H245Connection->reported = TRUE;
                }

                if (reply == cmTransOK)
					/* actual listenning initiation */
                    session->H245Connection->h245Listen =
                            tpktOpen(transGlobals->tpktCntrl,
                                     session->H245Connection->localAddress.ip,
                                     session->H245Connection->localAddress.port,
                                     tpktServer,
                                     transH245AcceptHandler,
                                     (void *)session->H245Connection);
                else
                    session->H245Connection->h245Listen = NULL;


                /* get the allocated full local address (IP and port) */
                getGoodAddress(transGlobals,
                               session->H245Connection->h245Listen,
                               session->Q931Connection,
                               cmTransH245Conn,
                               &session->H245Connection->localAddress);

				/* call the call back that listenning has initiated */
                if (transGlobals->hostEventHandlers.cmEvTransHostListening)
                {
                    int         numOfLocks;
                    HATRANSHOST haTransHost =
                                 (HATRANSHOST)emaGetApplicationHandle(((EMAElement)session->H245Connection));

                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransHostListening(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d) error = %d)",
                            emaGetIndex((EMAElement)session->H245Connection),session->H245Connection,
                            haTransHost,
                            session->H245Connection->localAddress.ip,
                            session->H245Connection->localAddress.port,
                            (session->H245Connection->h245Listen == NULL)));

                    numOfLocks = emaPrepareForCallback((EMAElement)session->H245Connection);
                    transGlobals->hostEventHandlers.cmEvTransHostListening(
                                            (HSTRANSHOST)session->H245Connection,
                                            haTransHost,
                                            cmTransH245Conn,
                                            &session->H245Connection->localAddress,
                                            (session->H245Connection->h245Listen == NULL));
                    emaReturnFromCallback((EMAElement)session->H245Connection, numOfLocks);
                    session->H245Connection->reported = TRUE;
                }

				/* in case of failure or user abortion report to log,
				   upon real error exit */
                if ( (!session->H245Connection->h245Listen) && (reply != cmTransIgnoreMessage) )
                {
                    logPrint(transGlobals->hLog, RV_ERROR,
                             (transGlobals->hLog, RV_ERROR,
                             "determineIfToOpenH245 failed to initiate listen on H.245 TPKT"));
                    emaUnlock((EMAElement)session->H245Connection);
                    return FALSE;
                }
                else
                if (reply == cmTransIgnoreMessage)
                {
                    logPrint(transGlobals->hLog, RV_INFO,
                             (transGlobals->hLog, RV_INFO,
                             "determineIfToOpenH245 listen on H.245 TPKT refused by user"));
                }
                else
                {
                    /* mark that the h245 host is in listenning mode */
                    session->H245Connection->state = hostListenning;
                }
            }

            /* for listenning hosts we need to insert the address to the outgoing
			   message so do it */
            if ( (session->H245Connection->state == hostListenning) ||
                 (session->H245Connection->state == hostListenningConnecting) )
            {
                __pvtBuildByFieldIds(addressNode,
                                     transGlobals->hPvt,
                                     messageBodyNode,
                                    {   _q931(h323_message_body)
                                        _anyField
                                        _q931(h245Address)
                                        LAST_TOKEN
                                    },
                                    0, NULL);

                res = -1;
                if (addressNode >= 0)
                    res = cmTAToVt(transGlobals->hPvt,
                                   addressNode,
                                   &session->H245Connection->localAddress);
                if (res < 0)
                {
					/* if we can't transfer the address to the other side there
					   is no much use in leaving the poor connection listenning,
					   so kill it */
                    if (session->H245Connection->h245Listen)
                    {
                        tpktClose(session->H245Connection->h245Listen);
                        session->H245Connection->h245Listen = NULL;
                    }
                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "determineIfToOpenH245 failed on building h245Address"));
                    emaUnlock((EMAElement)session->H245Connection);
                    return FALSE;
                }
            }
        }
		else
        /* this section is done for incoming messages */
        {
            int  addressNode = -1;
            BOOL ok = FALSE;
            
            /* if we don't have the remote H245 address yet let's try and get one */
            if ((!session->H245Connection) ||
                ((session->H245Connection) && (session->H245Connection->remoteAddress.ip == 0)))
            {
				/* check if the incoming message has H245 address */
		        __pvtGetNodeIdByFieldIds(addressNode,
			                             transGlobals->hPvt,
				                         messageBodyNode,
					                    {   _q931(h323_message_body)
						                    _anyField
							                _q931(h245Address)
								            LAST_TOKEN
									    });

				/* if we got a remote address (or if it's a CONNECT message, which
				   means that we won't have another chance to get the address) try 
				   to connect the H.245 to it */
	            if ( (addressNode >= 0) || (msgType == cmQ931connect) )
				{
					/* we have the remote address, do we have a H245 host at all? */
					if (!session->H245Connection)
					{
	                    /* just create a non-listenning host */
		                res = createHostByType(transGlobals,
			                                  (HSTRANSSESSION)session,
				                              (HSTRANSHOST *)&session->H245Connection,
					                          cmTransH245Conn,
						                      NULL,
							                  cmTransNoAnnexE);
	                    if (res != cmTransOK)
		                {
			                logPrint(transGlobals->hLog, RV_ERROR,
				                    (transGlobals->hLog, RV_ERROR,
					                "determineIfToOpenH245 failed on allocating host element"));
						    return FALSE;
						}
						else
						{
			                /* create h245 local host address */
					        cmTransportAddress h245LocalAddr;
                            
                            memset(&h245LocalAddr, 0, sizeof(h245LocalAddr));
                            h245LocalAddr.type = cmTransportTypeIP;
			                getGoodAddress(transGlobals,
					                       0,
							               session->Q931Connection,
									       cmTransH245Conn,
										   &h245LocalAddr);
			                h245LocalAddr.port  = 0;

							session->H245Connection->localAddress = h245LocalAddr;
						}

		               /* this is a new host, lock it */
					   if (!emaLock((EMAElement)session->H245Connection))
							return FALSE;
					}

					/* Now we surly have a host, if we have a remote address, 
					   put the address into the host */
					if (addressNode >= 0)
			            cmVtToTA(transGlobals->hPvt,
				                 addressNode,
					             &session->H245Connection->remoteAddress);

                    /* if we receive a start H.245 message while already listening, we should be careful not
                    to create an H.245 conflict. */
                    if (forceOpen && (session->H245Connection->h245Listen != NULL))
                    {
                        /* first see if the listening connection is an "old" one or an initiated one: */
                        if ((!session->openControlWasAsked) && (!session->switchToSeparateWasAsked))
                        {
                            /* connection was not initiated - this is an "old" listening connection.
                            close it, for the sake of interoperability */
                            tpktClose(session->H245Connection->h245Listen);
                            session->H245Connection->h245Listen = NULL;
                        }
                        else
                        {
                            /* this is an "initiated" listening connection, so there might be a conflict */
                            HOSTSTATE state = session->H245Connection->state;

                            /* see in advance who would win the H.245 conflict, and act acordingly */
                            session->H245Connection->state = hostListenningConnecting;
                            solveH245Conflict(session->H245Connection);
                            if (session->H245Connection->state == hostListenning)
                            {
                                /* abort connecting */
                                needToOpen = FALSE;
                            }
                            else
                                session->H245Connection->state = state;
                        }
                    }

                    /* we hav ea new host and a remote address */
                    ok = TRUE;
                }
                else
                    /* we didn't manage to get an address but it's not CONNECT yet so maybe in the next message*/
                    ok = FALSE;
            }
                else
                    /* we have a host & a remote address */
                    ok = TRUE;

                /* if we need to open an H.245 connection and can do it, just do it, except for setup, in which we
                delay until after ACF */
                if (needToOpen && ok && (msgType != cmQ931setup))
                {
                    /* if we put a remote address into the host the connection will
                    happen, if we didn't, i.e. this is a CONNECT message and we don't
                    have a remote address yet, it will initiate a facility with startH245 */
                    connectH245(transGlobals, session, msgType);

                    /* check if the above message caused a startH245 facility message, if so send it */
                    if (!emaWasDeleted((EMAElement)session))
                        sendStartH245Facility(transGlobals, session);
                }
        }

        /* unlock the h245 host */
        emaUnlock((EMAElement)session->H245Connection);
    }

    return TRUE;
}


/**************************************************************************************
 * reportH245
 *
 * putrpose: This routine checks if it is necessary to report to the user that an H.245 is opened.
 *           It bases its decision accrding to the state of the session in regard to tunneling
 *           and parallel tunneling and to the faststart state of the session as well.
 *           If the connection was already reported, we eliminate the notification.
 *           The routine handles both incoming and outgoing messages.
 *
 * Input: outgoing          - TRUE - if an outgoing message, FALSE - if incoming one.
 *        transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        host              - the host on which the message arrived
 *        messageBodyNode   - The body of the message (incoming or outgoing)
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
void reportH245(BOOL            outgoing,
                cmTransGlobals  *transGlobals,
                cmTransSession  *session,
                cmTransHost     *host,
                int             messageBodyNode,
                int             msgType)
{
    BOOL needToReport = FALSE;

    if (outgoing)
    {
        /* handling outgoing messages */
        if (msgType == cmQ931setup)
        {
            /* in case of outgoing setup (outgoing call), we need to tell
               the user of a H.245 connection if we have tunneling without
               faststart, or in case of parallel tunneling, so the user
               may send H.245 tunneled messages before the setup is sent */
            if (session->parallelTunnelingState == parllOffered)
                needToReport = TRUE;
            else
            switch (session->tunnelingState)
            {
                case tunnPossible:
                case tunnOffered:
                case tunnApproved:
                {
                    if (session->faststartState == fsNo)
                        needToReport = TRUE;
                }
                break;

                default:
                    break;
            }
        }
        else
        {
            /* all other outgoing messages (incoming calls) */

            /* if we got some response for our faststart offering and tunneling
               was accepted by the remote we need to check if there is a need to
               report the H.245.
               in case of parallel we may notify the user. If not, that means
               that it's a 'simple' tunneling after faststart and we need to report
               only if the user explicitly requested to open a control session or
               if the faststart was refused */
            if ( (
                   (session->faststartState == fsApproved) ||
                   (session->faststartState == fsRejected)
                 )
                 &&
                 (session->tunnelingState == tunnApproved)
               )
            {
                if (session->parallelTunnelingState == parllApproved)
                    needToReport = TRUE;
                else
                    if ( (session->faststartState == fsRejected) || (session->openControlWasAsked) )
                        needToReport = TRUE;
            }
        }
    }
    else
    {
        /* incoming mesages */
        if (msgType == cmQ931setup)
        {
            /* in case of incoming SETUP message (new incoming call)
               we need to report H.245 in case that parallel was offered */
            if (session->parallelTunnelingState == parllApproved)
            {
                needToReport = TRUE;
            }
            else
            /* in case that no parallel was offered BUT we have a 'normal'
               tunneling WITHOUT faststart we can report that H.245 too */
            if ( (session->tunnelingState == tunnApproved) &&
                (session->faststartState != fsOffered) &&
                (session->faststartState != fsApproved) )
            {
                needToReport = TRUE;
            }
        }
        else
        {
            int tunnMsgNode;

            /* check if there are normal tunneled messages in the message */
           __pvtGetNodeIdByFieldIds(tunnMsgNode,
                                    transGlobals->hPvt,
                                    messageBodyNode,
                                    {   _q931(h245Control)
                                        LAST_TOKEN
                                    });
            /* all other incoming messages (outgoing calls) */
            /* if the tunneling was approved and no fast start
               or the other side started sending tunneling messages,
               we need to open the H.245 */
            if (session->tunnelingState == tunnApproved)
            {
                if ((session->faststartState == fsRejected) ||
                    (session->faststartState == fsNo)       ||
                    (tunnMsgNode >= 0)
                   )
                needToReport = TRUE;
            }
        }
    }

    /* if we need to report the H.245 connection and we haven't done so yet */
    if (needToReport)
        reportH245Establishment(transGlobals, session, host);
}

/**************************************************************************************
 * insertH245TunnelingFlag
 *
 * putrpose: This routine fills the H.245 tunneling flag 'h245Tunneling' into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *        msgType           - The message type into which the tunneled messages are inserted
 *
 ***************************************************************************************/
void insertH245TunnelingFlag(cmTransGlobals  *transGlobals,
                             cmTransSession  *session,
                             int             messageBodyNode,
                             int             msgType)
{
    int  node;

    /* set tunneling flag only if we are in a good tunneling state,
       i.e. it's allowed and not rejected yet, and we've never offered it yet,
       and we didn't ask to connect a separte H.245 connection yet */
    if ( (session->tunnelingState != tunnNo) &&
         (session->tunnelingState != tunnRejected) &&
         (session->tunnelingState != tunnOffered) &&
         !session->switchToSeparateWasAsked )
    {
        /* mark the message as having tunneled messages */
        __pvtBuildByFieldIds(node,
                             transGlobals->hPvt,
                             messageBodyNode,
                            {   _q931(h245Tunneling)
                                LAST_TOKEN
                            },
                            TRUE, NULL);

        /* for the following messages, do not insert tunneled messages but
           mark as provisional, so that the other side will not know yet whether
           we aproved the tunneling (just like previous version are when receiving
           these messages) */
        if ( (msgType == cmQ931setupAck) ||
             (msgType == cmQ931information) ||
             (msgType == cmQ931progress) ||
             (msgType == cmQ931status) ||
             (msgType == cmQ931statusInquiry)
           )
        {
            /* set the provisionalRespToH245Tunneling element */
            __pvtBuildByFieldIds(node, transGlobals->hPvt,messageBodyNode,
                        { _q931(provisionalRespToH245Tunneling)
                          LAST_TOKEN
                        },
                        0, NULL);
           return;
        }

        /* after sending for the first time, we must wait to see if the other
           side supports tunneling, until it does we may not continue using tunneling  */
        if (session->tunnelingState == tunnPossible)
            session->tunnelingState = tunnOffered;
    }
    else
    {
        if( ((session->tunnelingState == tunnOffered) || 
             ((msgType == cmQ931setup) && (session->tunnelingState == tunnNo) && (session->isTunnelingSupported))) &&
            !session->switchToSeparateWasAsked )
        {
            /* mark the message as supporting tunneled messages (we are either in tunnOffered
               state that was not answered yet, or supporting tunneling without having processed
               any messages yet. */
            __pvtBuildByFieldIds(node,
                                 transGlobals->hPvt,
                                 messageBodyNode,
                                {   _q931(h245Tunneling)
                                    LAST_TOKEN
                                },
                                TRUE, NULL);
        }
        else
        {
            /* mark the message as not supporting tunneled messages */
            __pvtBuildByFieldIds(node,
                                 transGlobals->hPvt,
                                 messageBodyNode,
                                {   _q931(h245Tunneling)
                                    LAST_TOKEN
                                },
                                FALSE, NULL);
        }
    }
}

/**************************************************************************************
 * insertH245TunneledMessages
 *
 * putrpose: This routine fills the H.245 tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *        msgType           - The message type into which the tunneled messages are inserted
 *
 ***************************************************************************************/
void insertH245TunneledMessages(cmTransGlobals  *transGlobals,
                                cmTransSession  *session,
                                int             messageBodyNode,
                                int             msgType)
{
    void *nextMsg = session->firstMessage;
    int  node;

    /* See if we have a cool message to send tunneling on */
    if ( (msgType == cmQ931setupAck) ||
        (msgType == cmQ931information) ||
        (msgType == cmQ931progress) ||
        (msgType == cmQ931status) ||
        (msgType == cmQ931statusInquiry)
        )
    {
        /* the above messages suck. No tunneling for you! */
        return;
    }
        
    if((session->parallelTunnelingState == parllApproved) && (session->faststartState == fsOffered))
    {
        /* hopefully, this will cover the case where we work in parallel mode, but did not
        ack the FS channels yet. In such a case, we should delay the sending of tunneled H.245 messages
        untill the FS channels are ready. cool? */
        logPrint(transGlobals->hLog, RV_INFO,
            (transGlobals->hLog, RV_INFO,
            "insertH245TunneledMessages delaying parellel tunneling untill FS ack"));
        return;
    }

    /* send tunneled messages only if we are in a good tunneling state,
       i.e. it's allowed and not rejected yet, and not offered (unless we 
	   were offering it in this very message which may be only first outgoing
	   setup message */
    if ( (session->tunnelingState != tunnNo) &&
         (session->tunnelingState != tunnRejected) &&
         ((session->tunnelingState != tunnOffered) || (msgType == cmQ931setup)) &&
         !session->switchToSeparateWasAsked )
    {
        /* go over all saved tunneled messages and put them into the Q.931 message */
        while (nextMsg)
        {
            BYTE *buffer;
            int  msgSize;

            /* read the next tunneled message */
            getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);

            /* lock the rpool */
            meiEnter(transGlobals->tablesLock);

            rpoolCopyToExternal(transGlobals->messagesRPool,
                                (void *)buffer,
                                nextMsg,
                                0,
                                transGlobals->codingBufferSize);

            msgSize = rpoolChunkSize(transGlobals->messagesRPool, nextMsg)
                                     -MSG_HEADER_SIZE-TPKT_HEADER_SIZE;

            /* unlock the rpool */
            meiExit(transGlobals->tablesLock);

            /* if it was not sent yet, insert it to the Q.931 message */
            node = 0;

            /* for parallel tunneling we insert it in a different place in the
               setup message than normal */
            if ( ((session->parallelTunnelingState == parllOffered) || (session->parallelTunnelingState == parllApproved))
                && (msgType == cmQ931setup) )
            {

                __pvtBuildByFieldIds(node,
                                     transGlobals->hPvt,
                                     messageBodyNode,
                                    {   _q931(h323_message_body)
                                        _q931(setup)
                                        _q931(parallelH245Control)
                                        _nul(0)
                                        LAST_TOKEN
                                    },
                                    msgSize,
                                    (char *)&buffer[MSG_HEADER_SIZE + TPKT_HEADER_SIZE]);
            }
            else
            {
                __pvtBuildByFieldIds(node,
                                     transGlobals->hPvt,
                                     messageBodyNode,
                                    {   _q931(h245Control)
                                        _nul(0)
                                        LAST_TOKEN
                                    },
                                    msgSize,
                                    (char *)&buffer[MSG_HEADER_SIZE + TPKT_HEADER_SIZE]);
            }

            if (node < 0)
            {
                nextMsg = NULL;
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "insertH245TunneledMessages failed on inserting tunneled message (%d)",node));
            }
            else
            {
                /* if tunneling is established we need not keep this messgae */
                if (session->tunnelingState == tunnApproved)
                    nextMsg = extractMessageFromPool(transGlobals, session, TRUE);
                else
                {
                    /* get the next message */
                    memcpy(&nextMsg, &buffer[0], MSG_HEADER_SIZE);
                }
            }
        }

        /* after sending for the first time, we must wait to see if the other
           side supports tunneling */
        if (session->tunnelingState == tunnPossible)
            session->tunnelingState = tunnOffered;
    }
}

/**************************************************************************************
 * insertH450TunneledMessages
 *
 * putrpose: This routine fills the H.450 tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *
 ***************************************************************************************/
void insertH450TunneledMessages(cmTransGlobals  *transGlobals,
                                cmTransSession  *session,
                                int             messageBodyNode)
{
    int nodeId;

    if (session->h450Element >= 0)
    {
        __pvtBuildByFieldIds(nodeId,
                             transGlobals->hPvt,
                             messageBodyNode,
                            {_q931(h4501SupplementaryService) LAST_TOKEN},
                            0, NULL);
        if (nodeId>=0)
        {
            pvtMoveTree(transGlobals->hPvt,nodeId, session->h450Element);
            session->h450Element = -1;
        }
        else
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "insertH450TunneledMessages failed on inserting tunneled message (session=%d-%x)",
                    emaGetIndex((EMAElement)session), session));
        }
    }
}


/**************************************************************************************
 * insertAnnexLTunneledMessages
 *
 * putrpose: This routine fills the annex L tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *
 ***************************************************************************************/
void insertAnnexLTunneledMessages(cmTransGlobals    *transGlobals,
                                  cmTransSession    *session,
                                  int               messageBodyNode)
{
    int nodeId;

    if (session->annexLElement >= 0)
    {
        __pvtBuildByFieldIds(nodeId,
                             transGlobals->hPvt,
                             messageBodyNode,
                            {_q931(stimulusControl) LAST_TOKEN},
                            0, NULL);
        if (nodeId>=0)
        {
            pvtMoveTree(transGlobals->hPvt,nodeId, session->annexLElement);
            session->annexLElement = -1;
        }
        else
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "insertAnnexLTunneledMessages failed on inserting tunneled message (session=%d-%x)",
                    emaGetIndex((EMAElement)session), session));
        }
    }
}


/**************************************************************************************
 * insertAnnexMTunneledMessages
 *
 * putrpose: This routine fills the annex M tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *
 ***************************************************************************************/
void insertAnnexMTunneledMessages(cmTransGlobals    *transGlobals,
                                  cmTransSession    *session,
                                  int               messageBodyNode)
{
    int nodeId;

    if (session->annexMElement >= 0)
    {
        __pvtBuildByFieldIds(nodeId,
                             transGlobals->hPvt,
                             messageBodyNode,
                            {_q931(tunnelledSignallingMessage) LAST_TOKEN},
                            0, NULL);
        if (nodeId>=0)
        {
            pvtMoveTree(transGlobals->hPvt,nodeId, session->annexMElement);
            session->annexMElement = -1;
        }
        else
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "insertAnnexMTunneledMessages failed on inserting tunneled message (session=%d-%x)",
                    emaGetIndex((EMAElement)session), session));
        }
    }
}


/**************************************************************************************
 * getMultiplexedParams
 *
 * putrpose: to get the multiplexed parameters from an incoming message. The parameters
 *           are taken from the message and set into the host parameters which are constatntly
 *           updated by the incoming messages and user API calls.
 *
 * Input: transGlobals - The global data of the module.
 *        host         - the host on which the message was received.
 *        pvtNode      - the message from which the parameters are to be get.
 *        msgType      - The type of the message.
 ***************************************************************************************/
void getMultiplexedParams(cmTransGlobals *transGlobals, cmTransHost *host, int pvtNode, int msgType)
{
    int res;
    BOOL value;
    BOOL supportsMultiplexing = FALSE;
    BOOL oldSupportsMultiplexing = FALSE;

    switch (msgType)
    {
        case cmQ931alerting:
        case cmQ931callProceeding:
        case cmQ931connect:
        case cmQ931setup:
        case cmQ931facility:
        case cmQ931progress:
        {
            if (host->annexEUsageMode == cmTransNoAnnexE)
            {
                /* we are interested in these parameters just for none annex E connections */
                if ( ((host->isMultiplexed) && (host->remoteIsMultiplexed)) ||
                     ((!host->closeOnNoSessions) && (!host->remoteCloseOnNoSessions)) )
                     oldSupportsMultiplexing = TRUE;

                /* get the isMultiplexed element */
                __pvtGetByFieldIds(res, transGlobals->hPvt, pvtNode,
                                        { _q931(message)
                                          _anyField
                                          _q931(userUser)
                                          _q931(h323_UserInformation)
                                          _q931(h323_uu_pdu)
                                          _q931(h323_message_body)
                                          _anyField
                                          _q931(multipleCalls)
                                          LAST_TOKEN
                                        },
                                     NULL, (INT32 *)&value, NULL);
                if (res >= 0)
                    host->remoteIsMultiplexed = value;
                else
                    host->remoteIsMultiplexed = FALSE;


            /* get the maintainConnection element */
                __pvtGetByFieldIds(res, transGlobals->hPvt, pvtNode,
                                        { _q931(message)
                                          _anyField
                                          _q931(userUser)
                                          _q931(h323_UserInformation)
                                          _q931(h323_uu_pdu)
                                          _q931(h323_message_body)
                                          _anyField
                                          _q931(maintainConnection)
                                          LAST_TOKEN
                                        },
                                     NULL, (INT32 *)&value, NULL);

                if (res >= 0)
                    host->remoteCloseOnNoSessions = !value;
                else
                    host->remoteCloseOnNoSessions = TRUE;

                if ( ((host->isMultiplexed) && (host->remoteIsMultiplexed)) ||
                     ((!host->closeOnNoSessions) && (!host->remoteCloseOnNoSessions)) )
                     supportsMultiplexing = TRUE;

            /* report if the multiplex state has changed */
                if (oldSupportsMultiplexing != supportsMultiplexing)
                {
                    HATRANSHOST haTransHost =
                                      (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

                    if (transGlobals->hostEventHandlers.cmEvTransHostMultiplexChangeState)
                    {
                        int numOfLocks;
                        logPrint(transGlobals->hLog, RV_INFO,
                                (transGlobals->hLog, RV_INFO,
                                "cmEvTransHostMultiplexChangeState(hsHost = %d(%x), haHost=%x, isMultiplex=%d",
                                emaGetIndex((EMAElement)host), host, haTransHost,supportsMultiplexing));

                        numOfLocks = emaPrepareForCallback((EMAElement)host);
                        (transGlobals->hostEventHandlers.cmEvTransHostMultiplexChangeState)(
                                                            (HSTRANSHOST) host,
                                                            haTransHost,
                                                            supportsMultiplexing);
                        emaReturnFromCallback((EMAElement)host, numOfLocks);
                    }
                }
            }
        }

        break;

        default:
            break;
    }
}

/**************************************************************************************
 * extractH245Messages
 *
 * putrpose: extracts, decodes and reports the tunneled H.245 messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        host              - the host on which the message was received.
 *        messageBodyNode   - the UU-IE part of the message.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractH245Messages(cmTransGlobals *transGlobals,
                         cmTransSession *session,
                         cmTransHost    *host,
                         int            messageBodyNode,
                         int            msgType)
{
    int     encodedOctedStrings;
    int     nodeId;
    BYTE    *buffer;
    int     msgLen;
    int     pvtNode;

    HATRANSSESSION haTransSession =
                      (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

    /* first verify that we are interested in the tunneled messages */
    if ( (session->tunnelingState == tunnRejected) || (session->tunnelingState == tunnNo) )
         return;

    /* position yourself on the octed strings of the tunneled messages according to
       the place where they are stored in the message. (in SETUP with parallel offering
       it's a different place than the regular tunneling place). */
    if ( (session->parallelTunnelingState == parllApproved) && (msgType == cmQ931setup) )
    {
        __pvtGetNodeIdByFieldIds(encodedOctedStrings, transGlobals->hPvt, messageBodyNode,
                                { _q931(h323_message_body)
                                  _q931(setup)
                                  _q931(parallelH245Control)
                                  LAST_TOKEN });
    }
    else
    {
        __pvtGetNodeIdByFieldIds(encodedOctedStrings, transGlobals->hPvt, messageBodyNode,
                                { _q931(h245Control) LAST_TOKEN });
    }

    /* loop over all the tunneled messages */
    nodeId = pvtChild(transGlobals->hPvt, encodedOctedStrings);
    while ( (nodeId > 0) && (!emaWasDeleted((EMAElement)session)) )
    {
        /* get the coding buffer for decoding */
        getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);

        /* read the encoded message from the pvt into the buffer */
        msgLen = pvtGetString(transGlobals->hPvt, nodeId, transGlobals->codingBufferSize, (char *)buffer);
        if (msgLen > 0)
        {
            int decoded_length = 0;

            /* build an H.245 message tree and decode the message into it */
            pvtNode = pvtAddRoot(transGlobals->hPvt, transGlobals->synProtH245, 0, NULL);

            decoded_length = cmEmDecode(transGlobals->hPvt, pvtNode, buffer, msgLen, &decoded_length);

            if (decoded_length > 0)
            {
                HATRANSSESSION haTransSession =
                                 (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);
                INTPTR fieldId;
                int nodeId;
                char *ret;

                nodeId = pvtChild(transGlobals->hPvt, pvtNode);
                nodeId = pvtChild(transGlobals->hPvt, nodeId);

                pvtGet(transGlobals->hPvt,
                       nodeId,
                       &fieldId,
                       NULL, NULL, NULL);
                ret = pstGetFieldNamePtr(transGlobals->synProtH245, fieldId);

                logPrint(transGlobals->hTPKTCHAN, RV_DEBUG,
                        (transGlobals->hTPKTCHAN, RV_DEBUG,
                        "New TUNNELED message (call %d-%x) recv <-- %s:",
                        emaGetIndex((EMAElement)haTransSession),haTransSession,(ret)?(ret):NULL));
                pvtPrintStd(transGlobals->hPvt, pvtNode, (int)transGlobals->hTPKTCHAN);
                printHexBuff(buffer, msgLen, transGlobals->hTPKTCHAN);

                /* check if parallel tunneling was approved */
                if ( (session->isParallelTunnelingSupported) &&
                     ((session->parallelTunnelingState == parllOffered) || (session->parallelTunnelingState == parllApproved)) &&
                     (msgType != cmQ931setup) &&
                     (!session->notFirstTunnelingMessage) )
                {
                    /* check if it's TCS Ack message */
                    int child=pvtChild(transGlobals->hPvt,pvtNode);
                    int grandchild=pvtChild(transGlobals->hPvt,child);
                    int type=pvtGetSyntaxIndex(transGlobals->hPvt,child);
                    int messageId=pvtGetSyntaxIndex(transGlobals->hPvt,grandchild);
                    int h245Response = 2;
                    int h245terminalCapabilitySetAck = 4;

                    if ( (type == h245Response) && (messageId == h245terminalCapabilitySetAck) )
                    {
                        void * nextMsg;

                        session->parallelTunnelingState = parllApproved;

                        /* clean the saved but sent tunneled messages */
                        nextMsg = session->firstMessage;
                        while (nextMsg)
                            nextMsg = extractMessageFromPool(transGlobals, session, TRUE);
                    }
                }

                /* at any rate we have passed the first tunneled response for the session */
                session->notFirstTunnelingMessage = TRUE;


                /* if we succeeded, report the message on the session to he user */
                if (transGlobals->sessionEventHandlers.cmEvTransNewMessage)
                {
                    int numOfLocks, hostNumOfLocks;

                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransNewMessage(hsSession = %d(%x), haSession=%x, type=cmTransH245Type, pvtNode = %d)",
                            emaGetIndex((EMAElement)session), session, haTransSession, pvtNode));

                    numOfLocks = emaPrepareForCallback((EMAElement)session);
                    hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
                    (transGlobals->sessionEventHandlers.cmEvTransNewMessage)(
                                                    (HSTRANSSESSION) session,
                                                    haTransSession,
                                                    cmTransH245Type,
                                                    pvtNode,
                                                    NULL);
                    emaReturnFromCallback((EMAElement)session, numOfLocks);
                    emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
                }
            }
            else
            {
                /* can't decode, report it to the user on the session */
                if (transGlobals->sessionEventHandlers.cmEvTransBadMessage)
                {
                    int numOfLocks;
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransBadMessage(hsSession = %d(%x), haSession=%x, type=cmTransH245Type, outgoing=FALSE)",
                            emaGetIndex((EMAElement)session), session,  haTransSession));

                    numOfLocks = emaPrepareForCallback((EMAElement)session);
                    transGlobals->sessionEventHandlers.cmEvTransBadMessage(
                                                    (HSTRANSSESSION) session,
                                                    haTransSession,
                                                    cmTransH245Type,
                                                    buffer,
                                                    msgLen,
                                                    FALSE);
                    emaReturnFromCallback((EMAElement)session, numOfLocks);
                }
            }

            /* get rid of the decoded message */
            pvtDelete(transGlobals->hPvt, pvtNode);
        }

        /* go to the next encoded tunneled message */
        nodeId = pvtBrother(transGlobals->hPvt, nodeId);
    }

	/* if this was an incomming CONNECT and no response yet, all hope is lost */
    if ( (msgType == cmQ931connect) &&
		 (session->parallelTunnelingState == parllOffered) )
		session->parallelTunnelingState = parllNo;
}

/**************************************************************************************
 * extractH450Messages
 *
 * putrpose: extracts and reports the tunneled H.450 messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        pvtNode           - The Q.931 messgae.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractH450Messages(cmTransGlobals *transGlobals,
                         cmTransSession *session,
                         int            pvtNode,
                         int            msgType)
{
    int encodedOctedStrings;

    HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

    /* position on the H.450 part of the message */
    __pvtGetNodeIdByFieldIds(   encodedOctedStrings,
                                transGlobals->hPvt,
                                pvtNode,
                                {   _q931(message)
                                    _anyField
                                    _q931(userUser)
                                    _q931(h323_UserInformation)
                                    _q931(h323_uu_pdu)
                                    _q931(h4501SupplementaryService)
                                    LAST_TOKEN
                                });

    if (encodedOctedStrings >= 0 && transGlobals->sessionEventHandlers.cmEvTransNewH450Message)
    {
        /* loop over all the tunneled messages */
        int nodeId = pvtChild(transGlobals->hPvt, encodedOctedStrings);
        while ( (nodeId > 0) && (!emaWasDeleted((EMAElement)session)) )
        {
            INT32 size;

            /* get the size of the current octed string */
            pvtGet(transGlobals->hPvt, nodeId, NULL, NULL, &size, NULL);

            /* if we succeeded, report the message on the session to the user */
            {
                int numOfLocks;

                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "cmEvTransNewH450Message(hsSession = %d(%x), haSession=%x, msgNode = %d, length = %d, type=%d)",
                        emaGetIndex((EMAElement)session), session, haTransSession, nodeId, size, msgType));

                numOfLocks = emaPrepareForCallback((EMAElement)session);
                (transGlobals->sessionEventHandlers.cmEvTransNewH450Message)(
                                                (HSTRANSSESSION) session,
                                                haTransSession,
                                                nodeId,
                                                size, msgType);
                emaReturnFromCallback((EMAElement)session, numOfLocks);
            }

            /* go to the next encoded tunneled message */
            nodeId = pvtBrother(transGlobals->hPvt, nodeId);
        }
    }
}

/**************************************************************************************
 * extractAnnexMMessages
 *
 * putrpose: extracts and reports the tunneled Annex M messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        messageBodyNode   - the UU-IE part of the message.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractAnnexMMessages(cmTransGlobals *transGlobals,
                           cmTransSession *session,
                           int            messageBodyNode,
                           int            msgType)
{
    int annexMElement;
    HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

    /* check if we have an annex M tunneling elemnt in the message */
    __pvtGetNodeIdByFieldIds(annexMElement, transGlobals->hPvt, messageBodyNode,
                            { _q931(tunnelledSignallingMessage) LAST_TOKEN });

    if (annexMElement >= 0)
    {
        /* report the message on the session to the user */
        if (transGlobals->sessionEventHandlers.cmEvTransNewAnnexMMessage)
        {
            int numOfLocks;
            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransNewAnnexMMessage(hsSession = %d(%x), haSession=%x, annexMElement=%d, type=%d)",
                    emaGetIndex((EMAElement)session), session, haTransSession, annexMElement, msgType));

            numOfLocks = emaPrepareForCallback((EMAElement)session);
            (transGlobals->sessionEventHandlers.cmEvTransNewAnnexMMessage)(
                                            (HSTRANSSESSION) session,
                                            haTransSession,
                                            annexMElement,
                                            msgType);
            emaReturnFromCallback((EMAElement)session, numOfLocks);
        }
    }
}

/**************************************************************************************
 * extractAnnexLMessages
 *
 * putrpose: extracts and reports the tunneled Annex L messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        messageBodyNode   - the UU-IE part of the message.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractAnnexLMessages(cmTransGlobals *transGlobals,
                           cmTransSession *session,
                           int            messageBodyNode,
                           int            msgType)
{
    int annexLElement;
    HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

    /* check if we have an annex L tunneling elemnt in the message */
    __pvtGetNodeIdByFieldIds(annexLElement, transGlobals->hPvt, messageBodyNode,
                            { _q931(stimulusControl) LAST_TOKEN });

    if (annexLElement >= 0)
    {
        /* report the message on the session to the user */
        if (transGlobals->sessionEventHandlers.cmEvTransNewAnnexLMessage)
        {
            int numOfLocks;

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransNewAnnexLMessage(hsSession = %d(%x), haSession=%x, annexMElement=%d, type=%d)",
                    emaGetIndex((EMAElement)session), session,  haTransSession, annexLElement, msgType));

            numOfLocks = emaPrepareForCallback((EMAElement)session);
            (transGlobals->sessionEventHandlers.cmEvTransNewAnnexLMessage)(
                                            (HSTRANSSESSION) session,
                                            haTransSession,
                                            annexLElement,
                                            msgType);
            emaReturnFromCallback((EMAElement)session, numOfLocks);
        }
    }
}


#ifdef __cplusplus
}
#endif /* __cplusplus*/
