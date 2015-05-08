

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

#include <q931asn1.h>
#include <cmutils.h>
#include <cmAutoRas.h>
#include <cmAutoRasEP.h>
#include <cmAutoRasCall.h>
#include <cmrasinit.h>
#include <cmdebprn.h>
#include <cmCall.h>

/************************************************************************
 *
 *                          Private functions
 *
 ************************************************************************/


/************************************************************************
 * handleAutoRasEvent
 * purpose: Callback function that handles Automatic RAS events
 * input  : hsCall  - Call handle of the transaction (NULL if not applicable)
 *          hsRas   - RAS transaction handle
 *          event   - Event that occured
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV handleAutoRasEvent(
    IN HCALL            hsCall,
    IN HRAS             hsRas,
    IN cmiAutoRasEvent  event)
{
    callElem* call = (callElem *)hsCall;
    HAPP hApp;
    cmElem* app;
    int chNodeId;
    HPVT hVal;

    if (hsRas != NULL)
        hApp = (HAPP)emaGetInstance((EMAElement)hsRas);
    else
        hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    app=(cmElem*)hApp;
    hVal=cmGetValTree(hApp);


    switch (event)
    {
        case cmiAutoRasEvGotRCF:
        {
            int responseId=cmiRASGetResponse(hsRas);
            responseId=pvtChild(app->hVal,responseId);

            app->rasGranted=0;
            if ((chNodeId = pvtGetChild(hVal, responseId, __q931(preGrantedARQ), NULL)) >= 0)
            {
                int tmpNodeId;
                INT32 value;
                value=0;
                pvtGet(hVal,pvtGetChild(hVal,chNodeId,__q931(makeCall),NULL),NULL,NULL,&value,NULL);
                if (value) app->rasGranted|=makeCall;
                value=0;
                pvtGet(hVal,pvtGetChild(hVal,chNodeId,__q931(useGKCallSignalAddressToMakeCall),NULL),NULL,NULL,&value,NULL);
                if (value) app->rasGranted|=useGKCallSignalAddressToMakeCall;
                value=0;
                pvtGet(hVal,pvtGetChild(hVal,chNodeId,__q931(answerCall),NULL),NULL,NULL,&value,NULL);
                if (value) app->rasGranted|=answerCall;
                value=0;
                pvtGet(hVal,pvtGetChild(hVal,chNodeId,__q931(useGKCallSignalAddressToAnswer),NULL),NULL,NULL,&value,NULL);
                if (value) app->rasGranted|=useGKCallSignalAddressToAnswer;

                value=0;
                pvtGet(hVal,pvtGetChild(hVal,chNodeId,__q931(irrFrequencyInCall),NULL),NULL,NULL,&value,NULL);
                if (value) app->irrFrequencyInCall=value;


                tmpNodeId=pvtChild(hVal,pvtGetChild2(hVal,chNodeId,__q931(alternateTransportAddresses),__q931(annexE)));
                if (tmpNodeId>=0)
                {
                      cmVtToTA(hVal,tmpNodeId,&(app->annexEAddress));
                }
                if ((chNodeId=pvtGetChild(hVal,chNodeId,__q931(useSpecifiedTransport),NULL)>=0))
                {
                    /* check if we must use annex E */
                    if(pvtGetChild(hVal,chNodeId,__q931(annexE),NULL)>=0)
                        app->rasGrantedProtocol=1;

                    /* check if we must use TCP */
                    if(pvtGetChild(hVal,chNodeId,__q931(tcp),NULL)>=0)
                        app->rasGrantedProtocol=0;
                }


            }
            break;
        }

        case cmiAutoRasEvGotACF:
        {
            cmTransportAddress tpktAddress,annexEAddess;
            INT32 bandwidth;
			int chNodeId, tpktAddressNodeId, annexEAddressNodeId;
            int responseId=cmiRASGetResponse(hsRas);
            responseId=pvtChild(hVal,responseId);
            tpktAddressNodeId=pvtGetChild(hVal,responseId,__q931(destCallSignalAddress),NULL);
            if (cmCallGetOrigin((HCALL)call,NULL))
            {
                /* if there is a clear instruction from GK about the protocol to be used */
                if ((chNodeId=pvtGetChild(hVal,responseId,__q931(useSpecifiedTransport),NULL)>=0))
                {
                    /* check if we must use TCP */
                    if(pvtGetChild(hVal,chNodeId,__q931(tcp),NULL)>=0)
                    {
                        cmCallSetParam((HCALL)hsCall,cmParamAnnexE,0, 0, NULL);
                    }
                    /* check if we must use annex E */
                    if(pvtGetChild(hVal,chNodeId,__q931(annexE),NULL)>=0)
                    {
                        cmCallSetParam((HCALL)hsCall,cmParamAnnexE,0, 1, NULL);
                        annexEAddressNodeId=pvtChild(hVal,pvtGetChild2(hVal,responseId,__q931(alternateTransportAddresses),__q931(annexE)));
                        cmVtToTA(hVal,annexEAddressNodeId,&annexEAddess);
                        cmCallSetParam((HCALL)hsCall,cmParamDestinationAnnexEAddress,0, sizeof(cmTransportAddress),(char*)&annexEAddess);
                    }
                }
                /* GK doesn't specify the protocol to be used */
                /* Do nothing , save the default of cmParamAnnexE */

                cmVtToTA(hVal,tpktAddressNodeId,&tpktAddress);
                cmCallSetParam((HCALL)hsCall,cmParamDestinationIpAddress,0, sizeof(cmTransportAddress),(char*)&tpktAddress);
                if (call->routeCallSignalAddress<0)
                {
                    cmCallSetParam((HCALL)hsCall,cmParamDestCallSignalAddress,0, sizeof(cmTransportAddress),(char*)&tpktAddress);
                }
            }
            
            /* Get the bandwidth */
            pvtGetChildValue(hVal,responseId,__q931(bandWidth),&bandwidth,NULL);
            call->rate=bandwidth*50;

            /* Check and set the call model */
            {
                INTPTR fieldId;
                pvtGet(hVal, pvtChild(hVal, pvtGetChild(hVal, responseId, __q931(callModel), NULL)), &fieldId, NULL, NULL, NULL);
                m_callset(call, gatekeeperRouted, (int)(fieldId != __q931(direct)));
            }

            callStartOK(call);
            break;
        }

        case cmiAutoRasEvCantRegister:
        {
            /* The endpoint couldn't register to a gatekeeper.
               We can only make this call if we have the DestCallSignalAddress and if
               the configuration allows IP calls without a gatekeeper */
            cmTransportAddress tpktAddress,annexEAddess;
            INT32 size=sizeof(cmTransportAddress);
            if (pvtGetChild(hVal,app->rasConf,__q931(allowCallsWhenNonReg),NULL)>=0 &&
                (!cmCallGetOrigin((HCALL)call,NULL) ||
                 cmCallGetParam((HCALL)hsCall,cmParamDestinationIpAddress,0,&size ,(char*)&tpktAddress)>=0 ||
                 cmCallGetParam((HCALL)hsCall,cmParamDestinationAnnexEAddress,0, &size,(char*)&annexEAddess)>=0))
            {
                m_callset(call,dummyRAS,TRUE);
                callStartOK(call);
            }
            else
            {
                callStartError(call);
                cmiAutoRASCallClose(hsCall);
            }
            break;
        }

        case cmiAutoRasEvTimedout:
        case cmiAutoRasEvFailedOnARQ:
        {
            /* if we received the call, we must raise the disconnected and idle events.
            otherwise, we are the initiators, and cmCallMake/Dial will just fail */
            if ((event == cmiAutoRasEvTimedout) || (!m_callget(call,callInitiator)))
                callStartError(call);
            cmiAutoRASCallClose(hsCall);
            break;
        }

        case cmiAutoRasEvCallRejected:
        {
            int responseId = cmiRASGetResponse(hsRas);
            int rejectId;
            INTPTR reasonType;

            /* Find the reason - we'll know what to do from there */
            rejectId = pvtChild(hVal, responseId);
            rejectId = pvtGetChild(hVal, rejectId, __q931(rejectReason), NULL);
            rejectId = pvtChild(hVal, rejectId);
            pvtGet(hVal, rejectId, &reasonType, NULL, NULL, NULL);

            /* See what's the reason */
            switch(reasonType)
            {
                case __q931(callerNotRegistered):
                    cmRegister((HAPP)hApp);
                    cmiAutoRASCallDial(hsCall);
                    break;

                case __q931(invalidEndpointIdentifier):
                    cmRegister((HAPP)hApp);
                    callStartError(call);
                    cmiAutoRASCallClose(hsCall);
                    break;

                case __q931(routeCallToGatekeeper):
                    cmiAutoRASCallClose(hsCall);
                    callStartRoute(call);
                    break;

                case __q931(incompleteAddress):
                    if (cmCallGetOrigin(hsCall,NULL))
                        callIncompleteAddress(call);
                    else
                    {
                        callStartError(call);
                        cmiAutoRASCallClose(hsCall);
                    }
                break;
                default:
                    callStartError(call);
                    cmiAutoRASCallClose(hsCall);
                break;
            }
            break;
        }

        case cmiAutoRasEvRateChanged:
        {
            /* BCF/BRQ received */
            INT32 bandwidth;
            int rasMessageId=cmiRASGetResponse(hsRas);
            if (rasMessageId>=0)
            {
                /* We've got a response message - get bandwidth from there */
                rasMessageId=pvtChild(hVal,rasMessageId);
                if ((pvtGetChildValue(hVal,rasMessageId,__q931(bandWidth),&bandwidth,NULL))>=0)
                    call->rate=bandwidth*50;
            }
            else
            {
                /* We've got a BRQ */
                rasMessageId=cmiRASGetRequest(hsRas);
                rasMessageId=pvtChild(hVal,rasMessageId);
                if ((pvtGetChildValue(hVal,rasMessageId,__q931(bandWidth),&bandwidth,NULL))>=0)
                    call->rate=bandwidth*50;
            }

            /* Notify the application about the change of rate */
            callNewRate(call);
            break;
        }

        case cmiAutoRasEvCallDropForced:
            rasCallDrop(call);
            break;

        case cmiAutoRasEvCallDropped:
            callStopOK(call);
            break;

        case cmiAutoRasEvPrepareIRR:
        {
            int requestId, responseId, perCallInfoId, nodeId;

            requestId  = pvtChild(hVal, cmiRASGetRequest(hsRas));
            responseId = pvtChild(hVal, cmiRASGetResponse(hsRas));
            __pvtBuildByFieldIds(perCallInfoId, hVal, responseId, {_q931(perCallInfo) _anyField LAST_TOKEN}, 0, NULL);

            pvtAdd(hVal, perCallInfoId, __q931(callReferenceValue), call->rascrv,NULL,NULL);
            pvtAdd(hVal,perCallInfoId,__q931(callIdentifier),16,(char*)call->callId,NULL);
            pvtAdd(hVal,perCallInfoId,__q931(conferenceID),16,(char*)call->cId,NULL);
            nodeId=pvtAddBranch(hVal,perCallInfoId,__q931(callSignaling));
            pvtAddBranch(hVal,perCallInfoId,__q931(h245));
            pvtAddBranch(hVal,perCallInfoId,__q931(substituteConfIDs));
            pvtAdd(hVal,perCallInfoId,__q931(originator),(INT32)m_callget(call,callInitiator),NULL,NULL);
            pvtAdd(hVal,perCallInfoId,__q931(bandWidth),call->rate/100,NULL,NULL);
            pvtSetTree(hVal,pvtAddBranch(hVal,perCallInfoId,__q931(callType)),
                       hVal,pvtGetChild(hVal,requestId,__q931(callType),NULL));
            pvtSetTree(hVal,pvtAddBranch(hVal,perCallInfoId,__q931(callModel)),
                       hVal,pvtGetChild(hVal,responseId,__q931(callModel),NULL));
            if (m_callget(call,callInitiator))
            {
                pvtSetTree(hVal,pvtAddBranch(hVal,nodeId,__q931(sendAddress)),
                           hVal,pvtGetChild(hVal,responseId,__q931(destCallSignalAddress),NULL));
                pvtSetTree(hVal,pvtAddBranch(hVal,nodeId,__q931(recvAddress)),
                           hVal,pvtGetChild(hVal,responseId,__q931(srcCallSignalAddress),NULL));
            }
            else
            {
                pvtSetTree(hVal,pvtAddBranch(hVal,nodeId,__q931(sendAddress)),
                           hVal,pvtGetChild(hVal,responseId,__q931(srcCallSignalAddress),NULL));
                pvtSetTree(hVal,pvtAddBranch(hVal,nodeId,__q931(recvAddress)),
                           hVal,pvtGetChild(hVal,responseId,__q931(destCallSignalAddress),NULL));
            }
            break;
        }
    }

    return 0;
}


/************************************************************************
 * recvRasMessage
 * purpose: Callback function called when an incoming RAS message arrives.
 * input  : socketId    - Socket of the message
 *                        Can be the unicast of the multicast socket
 *          event       - Event that occured (always liEvRead)
 *          error       - TRUE if errors occured
 *          context     - Context given (stack's application handle)
 * output : none
 * return : none
 ************************************************************************/
void recvRasMessage(
    IN int      socketId,
    IN liEvents event,
    IN int      error,
    IN void*    context)
{
    cmElem*             app;
    rasChanType         chanType;
    cmTransportAddress  srcAddr;

    app = (cmElem *)context;

    if (event);
    cmCallPreCallBack((HAPP)app);

    /* Determine if this message is unicast or multicast */
    if (socketId == app->rasUnicastHandle)
        chanType = rasChanUnicast;
    else
        chanType = rasChanMulticast;

    if (error == 0)
    {
        BYTE*   buffer;
        int     bytesRecv;

#if 1 /* nextone - in case of single instance, raise priority to RAS */
		while (1)
		{
#endif
        getEncodeDecodeBuffer(app->encodeBufferSize, &buffer);

        /* Get the message from the net */
        srcAddr.type = cmTransportTypeIP;
        bytesRecv = liUdpRecv(socketId, buffer, app->encodeBufferSize, &srcAddr.ip, &srcAddr.port);

        /* Let RAS handle this message */
        if (bytesRecv > 0)
            cmiRASReceiveMessage(app->rasManager, chanType, &srcAddr, buffer, bytesRecv);
#if 1	/* nextone */
		else
			break;
		} /* end of while */
#endif
    }
}

extern int dmr_liUdpSend (int handle,UINT8 *buff,int len,UINT32 ipAddr,UINT16 port);
/************************************************************************
 * sendRasMessage
 * purpose: Callback function called when an outgoing RAS message should
 *          be sent.
 * input  : hApp            - Stack's instance handle
 *          chanType        - Type of channel to send through
 *                            (for now, always through unicast)
 *          destAddress     - Address to send the message to
 *          messageBuf      - The message buffer to send
 *          messageLength   - The length of the message in bytes
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV sendRasMessage(
    IN HAPP             hApp,
    IN rasChanType      chanType,
    IN cmRASTransport*  destAddress,
    IN BYTE*            messageBuf,
    IN UINT32           messageLength)
{
    cmElem* app;
    int     socket;

    app = (cmElem *)hApp;
	
	/* if the chanType wasn't passed a socket then use the std value */
	if((socket = chanType >>1))
    	return dmr_liUdpSend(socket, messageBuf, (int)messageLength, destAddress->ip, destAddress->port);


	/* Else */
    if (chanType == rasChanUnicast)
        socket = app->rasUnicastHandle;
    else
        socket = app->rasMulticastHandle;

    return liUdpSend(socket, messageBuf, (int)messageLength, destAddress->ip, destAddress->port);
}





/************************************************************************
 *
 *                          Public functions
 *
 ************************************************************************/


/************************************************************************
 * rasInit
 * purpose: Initialize the RAS module and all the network related with
 *          RAS.
 * input  : app     - Stack handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasInit(IN cmElem* app)
{
    INT32               inTx, outTx;
    INT32               maxCalls = 1;
    INT32               maxBuffSize = 2048;

    /* Make sure we can check these handles later on for validity */
    app->rasMulticastHandle = -1;
    app->rasUnicastHandle = -1;
    app->rasAddessID = -1;

    /* Find out the number of calls - we might need that to calculate the number
       of transactions to support */
    if (ciGetValue(app->hCfg, "system.maxCalls", NULL, &maxCalls) < 0)
        if (ciGetValue(app->hCfg,"Q931.maxCalls",NULL, &maxCalls) < 0)
            maxCalls = 20;

    /* Check some of the configuration parameters related with RAS */
    /* - manual RAS */
    app->manualRAS = (pvtGetChild(app->hVal, app->rasConf, __q931(manualRAS), NULL) >= 0);
    /* - gatekeeper */
    app->gatekeeper = (pvtGetChild(app->hVal, app->rasConf, __q931(gatekeeper), NULL) >= 0);

    /* Estimate the number of transactions by the number of calls */
    if (app->gatekeeper)
    {
        outTx = maxCalls;
        inTx = maxCalls * 2 + 10;
    }
    else
    {
        outTx = maxCalls + 10;
        inTx = maxCalls;
    }

    /* Check if we've got explicit number of transactions supported in the configuration */
    ciGetValue(app->hCfg, "system.maxRasOutTransactions", NULL, &outTx);
    ciGetValue(app->hCfg, "system.maxRasInTransactions", NULL, &inTx);
    ciGetValue(app->hCfg, "system.allocations.maxBuffSize", NULL, &maxBuffSize);

    if (inTx != 0)
    {
        /* Calculate the true value of incoming transactions - this is done by checking
           the amount of incoming transactions per second and multiplying that with the
           timeout necessary to support */
		INT32 timeout = 4;
		INT32 maxRetries = 3;
	    ciGetValue(app->hCfg, "RAS.responseTimeOut",NULL, &timeout);
	    ciGetValue(app->hCfg, "RAS.maxRetries",NULL, &maxRetries);
	    timeout *= maxRetries;
	    if (timeout == 0) timeout = 1;
        inTx = inTx * timeout + 1;
    }

    /* Initialize the RAS module */
    app->rasManager =
        cmiRASInitialize((HAPP)app,
                         app->logMgr,
                         app->hCat,
                         app->hVal,
                         app->rasConf,
                         inTx,
                         outTx,
                         maxBuffSize);
    if (app->rasManager == NULL)
        return RESOURCES_PROBLEM;

    /* Initialize automatic RAS if we have to */
    if (!app->manualRAS)
    {
        app->hAutoRas = cmiAutoRASInit((HAPP)app,
                                       app->logMgr,
                                       app->hTimers,
                                       app->hVal,
                                       app->rasConf,
                                       handleAutoRasEvent);
        if (app->hAutoRas == NULL)
            return RESOURCES_PROBLEM;
    }

    cmiRASSetSendEventHandler(app->rasManager, sendRasMessage);
    cmiRASSetNewCallEventHandler((HAPP)app, cmEvRASNewCall);

    return 0;
}


/************************************************************************
 * rasStart
 * purpose: Start the RAS module and all the network related with
 *          RAS.
 * input  : app     - Stack handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasStart(IN cmElem* app)
{
    int nodeId;
	INT32 intParam;
    cmTransportAddress  uniAddr;
    cmTransportAddress  multiAddr;

    memset(&uniAddr, 0, sizeof(uniAddr));

    /* Find out the port the RAS is working with - we can work without one... */
    nodeId = pvtGetChild(app->hVal, app->rasConf, __q931(rasPort), NULL);
    intParam = 0;
    if (nodeId >= 0)
        pvtGet(app->hVal, nodeId, NULL, NULL, &intParam, NULL);

    /* Set the address for unicast messages */
    uniAddr.type = cmTransportTypeIP;
    uniAddr.port = (UINT16)intParam;
    uniAddr.ip = app->localIPAddress;

    /* Open the unicast address for listening */
    app->rasUnicastHandle = liOpen(uniAddr.ip, uniAddr.port, LI_UDP);
    if (uniAddr.port == 0)
    {
        int iPort;
        /* Update the RAS port inside the configuration - we know it now */
        iPort = uniAddr.port = liGetSockPort(app->rasUnicastHandle);
        pvtSet(app->hVal, nodeId, -1, iPort, NULL);
    }

    /* User wanted a RAS port to listen to, but it seems the port is a little bit taken... */
    if (app->rasUnicastHandle < 0)
        return -13; /* Network problem */

    /* Set the TTL value for the ras unicast handle */
    {
        INT32 maxTtl;
        UINT32 interfaceIp;

        if (pvtGetChildByFieldId(app->hVal, app->rasConf, __q931(maxMulticastTTL), &maxTtl, NULL) < 0)
            maxTtl = 20;

        liSetMulticastTTL(app->rasUnicastHandle, maxTtl);

        if (app->localIPAddress == 0)
            interfaceIp = **liGetHostAddrs();
        else
            interfaceIp = app->localIPAddress;
        liSetMulticastInterface(app->rasUnicastHandle, interfaceIp);
    }

    /* Gatekeepers should also set a multicast address for listening */
    if (app->gatekeeper)
    {
        /* Get the multicast address the RAS is working with and set it */
        multiAddr.type = cmTransportTypeIP;
        __pvtGetByFieldIds(nodeId, app->hVal, app->rasConf, {_q931(rasMulticastAddress) _q931(ipAddress) LAST_TOKEN}, NULL, NULL, NULL);
        if (nodeId >= 0)
        {
            pvtGetChildByFieldId(app->hVal, nodeId, __q931(port), &intParam, NULL);
            multiAddr.port = (UINT16)intParam;
            pvtGetString(app->hVal, pvtGetChild(app->hVal, nodeId, __q931(ip), NULL), sizeof(multiAddr.ip), (char*)&multiAddr.ip);

            app->rasMulticastHandle = liOpen(0, multiAddr.port, LI_UDP);
            liSetMulticastInterface(app->rasMulticastHandle, multiAddr.ip);
            liJoinMulticastGroup(app->rasMulticastHandle, multiAddr.ip, uniAddr.ip);
        }
    }

    /* Make sure we update the application about its RAS address */
    if (app->rasAddessID >= 0)
        pvtDelete(app->hVal, app->rasAddessID);
    app->rasAddessID = pvtAddRoot(app->hVal, app->hAddrSyn, 0, NULL);
    {
        if (uniAddr.ip == 0)
            uniAddr.ip = **liGetHostAddrs();
        cmTAToVt(app->hVal, app->rasAddessID, &uniAddr);
    }

    /* Start the RAS and set the CS address */
    cmiRASStart(app->rasManager, app->rasAddessID);
    cmiRASUpdateCallSignalingAddress(app->rasManager, app->q931Chan, app->q931AnnexEChan);

    /* Make sure we're waiting for unicast messages */
    if (app->rasUnicastHandle >= 0)
        liCallOn(app->rasUnicastHandle, liEvRead, recvRasMessage, app);

    /* Wait for multicast messages if we can */
    if (app->rasMulticastHandle >= 0)
        liCallOn(app->rasMulticastHandle, liEvRead, recvRasMessage, app);

    /* Start automatic RAS */
    if (!app->manualRAS)
    {
        cmiAutoRASStart((HAPP)app);

        if (pvtGetChild(app->hVal, app->rasConf, __q931(manualRegistration), NULL) < 0)
        {
            /* Seems like our automatic RAS should try and register when starting */
            cmRegister((HAPP)app);
        }
    }

    return 0;
}


/************************************************************************
 * rasStop
 * purpose: Stop the RAS module and all the network related with
 *          RAS.
 *          This makes the RAS ports to be removed and the endpoint to be
 *          unregistered.
 * input  : app     - Stack handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasStop(IN cmElem* app)
{
    int status = 0;

    if (!app->manualRAS)
        status = cmUnregister((HAPP)app);

    if (app->rasUnicastHandle >= 0)
        liClose(app->rasUnicastHandle);
    if (app->rasMulticastHandle >= 0)
    {
        int                 nodeId;
        UINT32              multiIP;
        liClose(app->rasMulticastHandle);

        __pvtGetByFieldIds(nodeId, app->hVal, app->rasConf, {_q931(rasMulticastAddress) _q931(ipAddress) LAST_TOKEN}, NULL, NULL, NULL);
        pvtGetString(app->hVal, pvtGetChild(app->hVal, nodeId, __q931(ip), NULL), sizeof(multiIP), (char*)&multiIP);
        liLeaveMulticastGroup(app->rasMulticastHandle, multiIP, app->localIPAddress);
    }
    cmiRASStop(app->rasManager);
    if (app->rasAddessID >= 0)
        pvtDelete(app->hVal, app->rasAddessID);
    app->rasAddessID = -1;
    app->rasMulticastHandle = -1;
    app->rasUnicastHandle = -1;

    return status;
}


/************************************************************************
 * rasEnd
 * purpose: End the RAS module and all the network related with
 *          RAS.
 * input  : app     - Stack handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasEnd(IN cmElem* app)
{
    if (app->rasManager != NULL)
        cmiRASEnd(app->rasManager);
    if (app->hAutoRas) 
        cmiAutoRASEnd((HAPP)app);
    return 0;
}


/************************************************************************
 *
 *                        Public API functions
 *
 ************************************************************************/


/************************************************************************
 * cmGetRASConfigurationHandle
 * purpose: Gets the root Node ID of the RAS configuration tree.
 *          The application can then get and change configuration parameters
 *          for the control procedures.
 * input  : hApp     - Application's stack handle
 * output : none
 * return : The PVT Node ID of the RASConfiguration subtree on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
INT32 RVCALLCONV cmGetRASConfigurationHandle(
        IN  HAPP             hApp)
{
    cmElem* app = (cmElem *)hApp;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmGetRASConfigurationHandle: hApp(0x%x)", hApp);

    cmiAPIExit(hApp, "cmGetRASConfigurationHandle: [%d].", app->rasConf);

    return app->rasConf;
}


/************************************************************************
 * cmGetLocalRASAddress
 * purpose: Gets the local RAS address
 * input  : hApp    - Application's stack handle
 * output : tr      - The local RAS signal transport address.
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
INT32 RVCALLCONV cmGetLocalRASAddress(
                IN  HAPP                hApp,
                OUT cmTransportAddress* tr)
{
    cmElem* app = (cmElem *)hApp;
    int status;

    if (!hApp) return RVERROR;
    cmiAPIEnter(hApp, "cmGetLocalRASAddress(hApp=0x%x)", hApp);

    /* Convert to address */
    tr->type = cmTransportTypeIP;
    tr->length = 0;
    status = cmVtToTA(app->hVal, app->rasAddessID, tr);

    cmiAPIExit(hApp, "cmGetLocalRASAddress: [%d].", status);
    return status;
}

/************************************************************************
 * recvRasMessage
 * purpose: Callback function called when an incoming RAS message arrives.
 * input  : socketId    - Socket of the message
 *                        Can be the unicast of the multicast socket
 *          event       - Event that occured (always liEvRead)
 *          error       - TRUE if errors occured
 *          context     - Context given (stack's application handle)
 * output : none
 * return : none
 ************************************************************************/
void dmr_recvRasMessage(
    IN int      socketId,
    IN liEvents event,
    IN int      error,
    IN void*    context)
{
    cmElem*             app;
    rasChanType         chanType;
	int					instanceChanType = socketId <<1;
    cmTransportAddress  srcAddr;
	event = liEvRead;
    app = (cmElem *)context;

/*   if (event); */ /* AGMI - NexTone fix this*/
/*    cmCallPreCallBack((HAPP)app); */

    /* Determine if this message is unicast or multicast */
/*    if (socketId == app->rasUnicastHandle) */
        chanType = rasChanUnicast;
/*   else
        chanType = rasChanMulticast; */

    if (error == 0)
    {
        BYTE*   buffer;
        int     bytesRecv;

#if 1 /* nextone - in case of single instance, raise priority to RAS */
		while (1)
		{
#endif
        getEncodeDecodeBuffer(app->encodeBufferSize, &buffer);

        /* Get the message from the net */
        srcAddr.type = cmTransportTypeIP;
        bytesRecv = dmr_liUdpRecv(socketId, buffer, app->encodeBufferSize, &srcAddr.ip, &srcAddr.port);

        /* Let RAS handle this message */
        if (bytesRecv > 0)
            cmiRASReceiveMessage(app->rasManager, instanceChanType, &srcAddr, buffer, bytesRecv);
#if 1	/* nextone */
		else
			break;
		} /* end of while */
#endif
    }
}
#if 0
int
dmr_liUdpRecv (int handle, UINT8 *buff,int len,UINT32*ipAddr,UINT16*port)
{
  static int WouldBlockErrorCounter=0;

  /* receives the next incoming data gram */
  struct sockaddr_in fsin;
  sockaddr_namelen fromlen = (sockaddr_namelen)sizeof(fsin);
  int ret= recvfrom (handle,(char *)buff,len,0,(SOCKADDRPTR)&fsin,&fromlen);
  if (ret<0 && ERRNOGET == EWOULDBLOCK) WouldBlockErrorCounter++;
  if (WouldBlockErrorCounter > 1000) {
#if 0 /* Nextone */
    fprintf(stderr, "    >>> li:UdpRecv: 1000 EWOULDBLOCK errors.\n");
#endif
    WouldBlockErrorCounter=0;
  }

  if (ret<0 && ERRNOGET != EWOULDBLOCK) {
    fprintf(stderr, "    >>> li:UdpRecv: socket=%d err=%d", handle, ERRNOGET);
    perror("Error:");
  }

  *port  =ntohs(fsin.sin_port);
  *ipAddr=(UINT32)/*ntohl*/(fsin.sin_addr.s_addr);

  printf("li:UDP RECEIVE: handle=%d  port=%d ip=0x%x, len=%d",
         handle, *port, *ipAddr, ret);

  //liCallOnForSeli(handle,liNodes[handle].event);

 /* reestablish all events */

  return (ret);
}
#endif

#ifdef __cplusplus
}
#endif


