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

#include <stdlib.h>
#include <cm.h>
#include <cmintr.h>
#include <cmictrl.h>
#include <cmConfig.h>
#include <cmParam.h>
#include <log.h>
#include <ms.h>
#include <memfunc.h>
#include <conf.h>
#include <q931asn1.h>
#include <stkutils.h>
#include <cmutils.h>
#include <intutils.h>
#include <cmdebprn.h>
#include <cmCrossReference.h>
#include <cmiFastStart.h>
#include <cmrasinit.h>
#include <statistic.h>
#include <tls.h>
#include <ti.h>
#include <h323Version.h>
#include <syslog.h>




/* Indentification string of this version.
   Unix systems like to search for such a string in the binary file itself */
char rvcm_ident[] = "@(#)RADVISION H323 Protocol Stack "RV_H323_STACK_VERSION;

char* getCID(void);
/* Nextone */
void endCiscoPVT_PST(IN cmElem* app);
void initCiscoPVT_PST(IN cmElem*app);

/************************************************************************
 * endPVT_PST
 * purpose: Deallocate any information related to PVT and PST
 * input  : app         - Stack instance handle
 * output : none
 * return : none
 ************************************************************************/
void endPVT_PST(IN cmElem* app)
{
    pstDestruct(app->hSyn);
    pstDestruct(app->hRASSyn);

    pstDestruct(app->synProtQ931);
    pstDestruct(app->synProtH245);
    pstDestruct(app->synProtRAS);

    pstDestruct(app->h245TransCap);
    pstDestruct(app->h245RedEnc);
    pstDestruct(app->h245DataType);

    pstDestruct(app->hAddrSyn);
    pstDestruct(app->hAddrSynH245);
    pstDestruct(app->synOLC);
    pstDestruct(app->synConfQ931);
    pstDestruct(app->synConfH245);
    pstDestruct(app->synConfRAS);

    pstDestruct(app->synAnnexL);
    pstDestruct(app->synAnnexM);

    pstDestruct(app->synQ931UU);
    pstDestruct(app->synTerminalLabel);
    pstDestruct(app->synGkCrypto);
    pstDestruct(app->synGkPwdCert);
	/* Nextone */
 	endCiscoPVT_PST(app);

    endParamSyntax(app);

    pvtDestruct(app->hVal);
}


/************************************************************************
 * initPVT_PST
 * purpose: Initialize any information related to PVT and PST
 * input  : app         - Stack instance handle
 *          vtNodeCount - Number of nodes in PVT node pool
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int initPVT_PST(IN cmElem*app, IN int vtNodeCount)
{
    int res;

    app->hSyn    =pstConstruct(cmEmGetQ931Syntax(),(char *)"Q931ApplicationMessage");
    app->hRASSyn =pstConstruct(cmEmGetQ931Syntax(),(char *)"RasApplicationMessage");

    app->synProtQ931=pstConstruct(cmEmGetQ931Syntax(),(char *)"Q931Message");
    app->synProtH245=pstConstruct(cmEmGetH245Syntax(),(char *)"MultimediaSystemControlMessage");
    app->synProtRAS =pstConstruct(cmEmGetQ931Syntax(),(char *)"RasMessage");

    app->h245TransCap=pstConstruct(cmEmGetH245Syntax(),(char *)"TransportCapability");
    app->h245RedEnc=pstConstruct(cmEmGetH245Syntax(),(char *)"RedundancyEncodingMethod");
    app->h245DataType=pstConstruct(cmEmGetH245Syntax(),(char *)"DataType");

    app->hAddrSyn=pstConstruct(cmEmGetQ931Syntax(),(char *)"TransportAddress");
    app->hAddrSynH245=pstConstruct(cmEmGetH245Syntax(),(char *)"TransportAddress");
    app->synOLC=pstConstruct(cmEmGetH245Syntax(),(char *)"OpenLogicalChannel");
    app->synConfQ931=pstConstruct(cmEmGetQ931Syntax(),(char *)"Q931Configuration");
    app->synConfH245=pstConstruct(cmEmGetH245Syntax(),(char *)"H245Configuration");
    app->synConfRAS=pstConstruct(cmEmGetQ931Syntax(),(char *)"RASConfiguration");

    app->synAnnexL=pstConstruct(cmEmGetQ931Syntax(),(char *)"StimulusControl");
    app->synAnnexM=pstConstruct(cmEmGetQ931Syntax(),(char *)"H323-UU-PDU.tunnelledSignallingMessage");

    app->synQ931UU=pstConstruct(cmEmGetQ931Syntax(),(char *)"H323-UU-PDU");
    app->synTerminalLabel=pstConstruct(cmEmGetH245Syntax(),(char *)"TerminalLabel");
    app->synGkCrypto=pstConstruct(cmEmGetQ931Syntax(),(char *)"CryptoH323Token");
    app->synGkPwdCert=pstConstruct(cmEmGetQ931Syntax(),(char *)"PwdCertToken");
	/* Nextone */
	initCiscoPVT_PST(app);

    res = initParamSyntax(app);

    app->hVal=pvtConstruct(0, vtNodeCount);

    /* Make sure we were able to construct everything */
    if (app->hSyn && app->hRASSyn && app->synProtQ931 && app->synProtH245 && app->synProtRAS &&
        app->h245TransCap && app->h245RedEnc && app->h245DataType && app->hAddrSyn && app->hAddrSynH245 &&
        app->synOLC && app->synConfQ931 &&
        app->synConfH245 && app->synConfRAS && app->synAnnexL && app->synAnnexM && app->hVal && (res >= 0))

            return 0;

    endPVT_PST(app);
    syslog(LOG_LOCAL3|LOG_NOTICE, "synOLC=%d", app->synOLC);
    return RVERROR;
}


/************************************************************************
 * configTreesInit
 * purpose: Load the configuration related to the RAS, Q931 and H245
 *          standards into PVT structures.
 * input  : app         - Stack instance handle
 * output : none
 * return : none
 ************************************************************************/
void configTreesInit(cmElem*app)
{
    app->rasConf=pvtAddRoot(app->hVal,app->synConfRAS,0,NULL);
    pvtLoadFromConfig(app->hCfg, app->hVal, (char *)"RAS", app->rasConf, app->logConfig);

    app->q931Conf = pvtAddRoot(app->hVal,app->synConfQ931,0,NULL);
    pvtLoadFromConfig(app->hCfg, app->hVal, (char *)"Q931", app->q931Conf, app->logConfig);

    app->h245Conf = pvtAddRoot(app->hVal,app->synConfH245,0,NULL);
    pvtLoadFromConfig(app->hCfg, app->hVal, (char *)"h245", app->h245Conf, app->logConfig);

}

/************************************************************************
 * configTreesEnd
 * purpose: Delete the PVT structures of the configuration.
 * input  : app         - Stack instance handle
 * output : none
 * return : none
 ************************************************************************/
void configTreesEnd(cmElem*app)
{
    if(app->rasConf >= 0)
    {
        pvtDelete(app->hVal,app->rasConf);
        app->rasConf = RVERROR;
    }
    if(app->q931Conf >= 0)
    {
        pvtDelete(app->hVal,app->q931Conf);
        app->q931Conf = RVERROR;
    }
    if(app->h245Conf >= 0)
    {
        pvtDelete(app->hVal,app->h245Conf);
        app->h245Conf = RVERROR;
    }
}


/************************************************************************
 * readSystemParameters
 * purpose: Get the system parameters from the configuration.
 *          These parameters are found under the "system" root of the
 *          configuration.
 * input  : app         - Stack instance handle
 * output : maxCalls    - Maximum number of calls supported
 *          maxChannels - Maximum number of channels sopported
 *          vtNodeCount - Number of nodes in PVT trees
 *          maxBuffSize - Maximum size of encoded message buffers
 *          maxPoolSizeInKB - Maximum size of the transport pool to save
 *                            encoded but still not sent messages
 * return : none
 ************************************************************************/
void readSystemParameters(
    IN  cmElem* app,
    OUT int*    maxCalls,
    OUT int*    maxChannels,
    OUT int*    vtNodeCount,
    OUT int*    maxBuffSize,
    OUT int*    maxPoolSizeInKB)
{
    /* Find out which property mode is used for the property database of the calls */
    app->callPropertyMode=pmodeFullProperty;
    if (ciGetValue(app->hCfg,"system.callPropertyMode.copySingleMessages",NULL,NULL)>=0)
        app->callPropertyMode=pmodeCopySingleMessages;
    else if (ciGetValue(app->hCfg,"system.callPropertyMode.deleteSingleMessages",NULL,NULL)>=0)
        app->callPropertyMode=pmodeDeleteSingleMessages;
    else if (ciGetValue(app->hCfg,"system.callPropertyMode.doNotUseProperty",NULL,NULL)>=0)
        app->callPropertyMode=pmodeDoNotUseProperty;

    /* Find the number of calls and channels if written */
    ciGetValue(app->hCfg,"system.maxCalls" ,NULL,(INT32 *)maxCalls);
    ciGetValue(app->hCfg,"system.maxChannels" ,NULL,(INT32 *)maxChannels);

    /* Calculate the number of nodes if we won't have it later on */
    if (*maxCalls > 0)
    {
        switch (app->callPropertyMode)
        {
            case pmodeFullProperty:
            case pmodeCopySingleMessages:
            case pmodeDeleteSingleMessages:
                          /* maxCalls *(525call +75ras+maxChannels *75)+ 700init+200ras*/
                *vtNodeCount = (*maxCalls) * (600 + (*maxChannels) * 75) + 900;
                break;

            case pmodeDoNotUseProperty:
                /* Here we want to use less resources... */
                *vtNodeCount = (*maxCalls) * (300 + (*maxChannels) * 45) + 900;
                break;
        }
    }

    /* Find number of nodes , buffer  and pool size */
    ciGetValue(app->hCfg,"system.allocations.vtNodeCount",NULL,(INT32 *)vtNodeCount);
    ciGetValue(app->hCfg,"system.allocations.maxBuffSize",NULL,(INT32 *)maxBuffSize);
    *maxPoolSizeInKB=*maxCalls*
                     *maxBuffSize/1024+ /*One message per call in the pool*/
                     ((*maxCalls==1)?1:0); /* when we have one call, we need a little extra */
    ciGetValue(app->hCfg,"system.allocations.maxPoolSizeInKB",NULL,(INT32 *)maxPoolSizeInKB);

    /* port range allocation */
    {
        INT32 portFrom=-1;
        INT32 portTo=-1;
        if (ciGetValue(app->hCfg,"system.portFrom",NULL,&portFrom)>=0 &&
            ciGetValue(app->hCfg,"system.portTo",NULL,&portTo)>=0)
                liSetPortsRange((UINT16)portFrom, (UINT16)portTo);
    }

    app->cidAssociate = (ciGetValue(app->hCfg,"system.cidAssociate",NULL,NULL)>=0);
    app->delimiter=',';
    {
        char delimiter[2];
        if (ciGetString(app->hCfg,"system.delimiter",delimiter,2)>=0)
            app->delimiter=delimiter[0];
    }
}

TRANSERR cmEvTransNewSession(   IN  HAPPTRANS        hsTrans,
                                IN  HAPPATRANS       haTrans,
                                IN  HSTRANSSESSION   hsTransSession,
                                OUT HATRANSSESSION   *haTransSession,
                                IN  int              pvtNode,
                                OUT int              *cause,
                                OUT INTPTR           *reasonNameId);

TRANSERR cmEvTransConnectionOnSessionClosed(IN HSTRANSSESSION hsTransSession,
                                            IN HATRANSSESSION haTransSession,
                                            IN TRANSCONNTYPE  type);

TRANSERR cmEvTransSessionNewConnection( IN HSTRANSSESSION   hsTransSession,
                                        IN HATRANSSESSION   haTransSession,
                                        IN TRANSCONNTYPE    type);


TRANSERR cmEvTransNewMessage(IN HSTRANSSESSION hsTransSession,
                             IN HATRANSSESSION haTransSession,
                             IN TRANSTYPE      type,
                             IN int            pvtNode,
                             IN void           *hMsgContext);

TRANSERR cmEvTransWrite(IN HSTRANSSESSION hsTransSession,
                        IN HATRANSSESSION haTransSession );

TRANSERR cmEvTransBadMessage(IN HSTRANSSESSION  hsTransSession,
                             IN HATRANSSESSION  haTransSession,
                             IN TRANSTYPE       type,
                             BYTE               *msg,
                             int                msgSize,
                             BOOL               outgoing);

TRANSERR cmEvTransGetMessageNode(IN  HAPPATRANS         hAppATrans,
                                 IN  cmCallQ931MsgType  msgType,
                                 OUT int                *nodeId);

TRANSERR cmEvTransNewH450Message(IN HSTRANSSESSION  hsTransSession,
                                 IN HATRANSSESSION  haTransSession,
                                 IN int             msg,
                                 IN int             msgSize,
                                 IN int             msgType);

TRANSERR cmEvTransNewAnnexMMessage(IN HSTRANSSESSION    hsTransSession,
                                   IN HATRANSSESSION    haTransSession,
                                   IN int               annexMElement,
                                   IN int               msgType);

TRANSERR cmEvTransNewAnnexLMessage(IN HSTRANSSESSION    hsTransSession,
                                   IN HATRANSSESSION    haTransSession,
                                   IN int               annexLElement,
                                   IN int               msgType);


TRANSSESSIONEVENTS tse={cmEvTransNewSession,cmEvTransConnectionOnSessionClosed,
                        cmEvTransSessionNewConnection,cmEvTransNewMessage,
                        NULL/*cmEvTransWrite*/,cmEvTransBadMessage,cmEvTransGetMessageNode,
                        cmEvTransNewH450Message,cmEvTransNewAnnexMMessage,cmEvTransNewAnnexLMessage};


TRANSERR cmEvTransHostConnected(
            IN HSTRANSHOST   hsTransHost,
            IN HATRANSHOST   haTransHost,
            IN TRANSCONNTYPE type,
            IN BOOL          isOutgoing);


TRANSERR cmEvTransHostClosed(
            IN HSTRANSHOST hsTransHost,
            IN HATRANSHOST haTransHost,
            IN BOOL        wasConnected);

TRANSERR cmEvTransNewRawMessage(
            IN  HSTRANSHOST hsTransHost,
            IN  HATRANSHOST haTransHost,
            IN  TRANSTYPE   type,
            INOUT int       pvtNode,
            IN  BYTE        *msg,
            IN  int         msgSize,
            OUT int         *decoded,
            OUT void        **hMsgContext);

TRANSERR cmEvTransSendRawMessage(
            IN  HSTRANSHOST         hsTransHost,
            IN  HATRANSHOST         haTransHost,
            IN  HSTRANSSESSION      hsSession,
            IN  HATRANSSESSION      haSession,
            IN  int                 pvtNode,
            IN  int                 size,
            OUT BYTE                *msg,
            OUT int                 *msgSize);

TRANSERR cmEvTransHostNewMessage(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSTYPE            type,
            IN int                  pvtNode,
            IN void                 *hMsgContext);

TRANSERR cmEvTransHostBadMessage(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSTYPE            type,
            IN BYTE                 *msg,
            IN int                  msgSize,
            IN BOOL                 outgoing,
            IN void                 *hMsgContext);

TRANSERR cmEvTransHostListen(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSCONNTYPE        type,
            IN cmTransportAddress   *address);

TRANSERR cmEvTransHostListening(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSCONNTYPE        type,
            IN cmTransportAddress   *address,
            IN BOOL                 error);

TRANSERR cmEvTransHostConnecting(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSCONNTYPE        type,
            IN cmTransportAddress   *address);



TRANSHOSTEVENTS the={cmEvTransHostConnected,
                     cmEvTransHostClosed,
                     cmEvTransNewRawMessage,
                     cmEvTransSendRawMessage,
                     NULL/*cmEvTransHostNewMessage*/,
                     NULL/*cmEvTransHostBadMessage*/,
                     NULL/*cmEvTransHostMultiplexChangeState*/,
                     cmEvTransHostListen,
                     cmEvTransHostListening,
                     cmEvTransHostConnecting};


/********************************************************************************************
 * cmStartUp
 * purpose : This function should be called prior to any cmInit() or cmInitialize() calls.
 *           It must be called when several cmInitialize() functions are called from
 *           different threads at the same time.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI
void RVCALLCONV cmStartUp(void)
{
    /* Make sure we've got a global mutex to synchronize between several stack instances */
    tlsStartUp();
}


/************************************************************************
 * cmInitialize
 * purpose: Initializes the Conference Manager instance.
 *          This function must be called before any other H.323 function
 *          except cmGetVersion().
 * input  : name    - Configuration file name to use
 * output : lphApp  - Application handle created for the initialized
 *                    stack instance
 * return : non-negative value on success
 *          negative value on failure
 *          RESOURCES_PROBLEM   - Resource problem
 *          -10                 - Memory problem
 *          -11                 - Configuration problem
 *          -13                 - Network problem
 ************************************************************************/
RVAPI
int RVCALLCONV cmInitialize(IN char * name, OUT LPHAPP lphApp)
{
    cmElem* app;
    int     status;
    BOOL    useAnnexE;

    /* Define some stack defaults if not found in the configuration */
    int maxCalls    =   20;
    int maxChannels =   2;
    int vtNodeCount =   2700;
    int maxBuffSize =   2048;
    int maxPoolSizeInKB = maxCalls*maxBuffSize/1024;

    int proposed,accepted;

    /* Allocate an instance for the stack */
    app=(cmElem*)calloc(1,sizeof(cmElem));
    *lphApp=NULL;
    if (!app) return -10;

    tlsLockAll();

    /* Initialize a log instance */
    cmiCreateLog(app);

    /* Initialize memory allocator */
    memfInit(app->logMgr);

    app->hGlobalDataMEI=meiInit();
    app->applicationLock = meiInit();

    /* Make sure to notify about the version of the stack... */
    cmiAPIEnter((HAPP)app,(char*)"cmInititialize(name=%s,lphApp)",nprn(name));
    logPrint(app->logAPI, RV_DEBUG,
             (app->logAPI, RV_DEBUG, "Conference Manager: Version " RV_H323_STACK_VERSION));

    /* Start with the configuration */
    if ( (app->hCfg=ciConstruct(name)) == NULL) {
        logPrint(app->logERR, RV_ERROR,
                (app->logERR, RV_ERROR, "cmInit: Configuration construction error (%s).", nprn(name)));
        cmiAPIExit((HAPP)app,(char*)"cmInitialize()=ciConstruct Failed",app);
        free(app);
        tlsUnlockAll();
        return -11;
    }

    /* Initialize the MIB */
    app->hStatistic = mibCreateStatistic();

    readSystemParameters(app, &maxCalls,&maxChannels,&vtNodeCount,&maxBuffSize,&maxPoolSizeInKB);
    if( (maxCalls<0)||(maxChannels<0)||(maxBuffSize<0)||(maxPoolSizeInKB<0) )
        return RVERROR;
    app->maxChannels=maxChannels;
    app->encodeBufferSize=maxBuffSize;

    /* Initialize the LAN interface */
    if (liInit()<0 || (!app->localIPAddress && !liGetHostAddrs()))
    {
        ciDestruct(app->hCfg);
        cmiAPIExit((HAPP)app,(char*)"cmInitialize()=liInit Failed");
        free(app);
        tlsUnlockAll();
        return -13;
    }
    UTILS_RandomGeneratorConstruct(&(app->seed), timerGetTimeInMilliseconds());

    /* Initialize ASN related modules (coder, pvt, pst) */
    cmEmInstall(maxBuffSize);
    status = initPVT_PST(app, vtNodeCount);
    configTreesInit(app);


    /*
        The following simultaneous timers are possible per call

        1.call.timer
        2.call.q931.timer
        3.call.q931.timerSE
        4.call.control.outcap.timer
        5.call.control.msd.timer
        6.call.control.out_RM.timer
        7.call.control.rtd.timer

        1.call.control.channel[].timer
        2.call.control.channel[].ml_timer
        3.call.control.channel[].rc_timer
    */
    app->hTimers=mtimerInit(maxCalls*(7+maxChannels*3),NULL);

    /* Initialize calls and channels objects */
    app->hCalls=cmiInitCalls((HAPP)app,maxCalls,maxChannels, app->logMgr);
    app->hChannels=cmiInitChannels((HAPP)app,maxCalls,maxChannels, app->logMgr);
#if 0 /* Nextone */
    proposed=maxChannels*5;
#endif
    proposed=maxChannels*10;
    accepted=maxChannels;
    if (!fastStartInit(app,maxCalls,proposed,accepted) || (status < 0))
    {
        cmiEndChannels(app->hChannels);
        cmiEndCalls(app->hCalls);
        mtimerEnd(app->hTimers);
        liEnd();
        ciDestruct(app->hCfg);
        cmiAPIExit((HAPP)app,(char*)"cmInitialize()=liInit Failed");
        free(app);
        tlsUnlockAll();
        return -10;
    }

    /* Make sure we allocated an encode/decode buffer */
    getEncodeDecodeBuffer(app->encodeBufferSize, NULL);

    /* Initialize transport */
    useAnnexE = FALSE;
    if ((pvtGetChild(app->hVal,app->q931Conf,__q931(useAnnexE),NULL))>=0)
        useAnnexE = TRUE;
    if (!(app->hTransport= cmTransInit((HAPPATRANS)app,app->logMgr,app->hVal,maxCalls, -1, maxPoolSizeInKB, maxBuffSize, useAnnexE)))
    {
      cmiAPIExit((HAPP)app,(char*)"cmInitialize()=cmTransInit failed",app);
      liEnd();
      ciDestruct(app->hCfg);
      free(app);
      tlsUnlockAll();
      return RVERROR;
    };

    cmTransSetSessionEventHandler(  app->hTransport, &tse,sizeof(TRANSSESSIONEVENTS));
    cmTransSetHostEventHandler( app->hTransport, &the,sizeof(TRANSHOSTEVENTS));

    /* Create CAT (Call Association Table) */
    if ((app->hCat = catConstruct(maxCalls,
                    (ciGetValue(app->hCfg,"RAS.gatekeeper" ,NULL,NULL)>= 0),
                    (ciGetValue(app->hCfg,"RAS.compare15bitRasCrv" ,NULL,NULL)>= 0),
                    app->cidAssociate!= FALSE, app->logMgr)) == NULL )
    {
      cmiAPIExit((HAPP)app,(char*)"cmInitialize()=catConstruct failed",app);
      liEnd();
      ciDestruct(app->hCfg);
      free(app);
      tlsUnlockAll();
      return RESOURCES_PROBLEM;
    }

    /* Initialize the RAS module */
    if ((status = rasInit(app)) < 0)
    {
      cmiAPIExit((HAPP)app,(char*)"cmInitialize()=rasInit failed",app);
      rasStop(app);
      liEnd();
      ciDestruct(app->hCfg);
      free(app);
      tlsUnlockAll();
      return status;
    };

    /* Start running the stack if we're supposed to */
    if (name && ciGetValue(app->hCfg,"system.manualStart",NULL,NULL)<0)
        if (cmStart((HAPP)app)<0)
        {
            cmiAPIExit((HAPP)app,(char*)"cmInitialize()=cmStart Failed");
            rasStop(app);
            liEnd();
            ciDestruct(app->hCfg);
            free(app);
            tlsUnlockAll();
            return RVERROR;
        }

    *lphApp=(HAPP)app;

    if (ciGetValue(app->hCfg, "system.printMemory", NULL, NULL) >= 0)
    {
        /* This piece of code allows us to use a performance test program with the release
           mode compilation that will display on the standard output the amount of memory
           allocated. Calling malloc(-1) causes the information to be displayed. */
        malloc(-1);
    }

    /* We're done... */
    cmiAPIExit((HAPP)app,(char*)"cmInitialize(*lphApp=0x%x)=0",app);
    tlsUnlockAll();
    return 0;
}

/************************************************************************
 * cmInit
 * purpose: Initializes the Conference Manager instance.
 *          This function must be called before any other H.323 function
 *          except cmGetVersion().
 * input  : name    - Configuration file name to use
 * output : none
 * return : Stack's application handle on success
 *          NULL on failure
 ************************************************************************/
RVAPI
HAPP RVCALLCONV cmInit(IN char * name)
{
    HAPP hApp;

    if (cmInitialize(name, &hApp) >= 0)
        return hApp;
    else
        return NULL;
}

/************************************************************************
 * cmStart
 * purpose: Starts the stack's activity
 *
 *          This function is only applicable when system.manualStart key
 *          is defined in the configuration. In manualStart mode cmInitialize()
 *          function does not automatically start Stack activity and accesses only
 *          the "system" configuration tree. To start the Stack use cmStart() function.
 *          The application may change configuration settings between cmInitialize()
 *          and cmStart() using cmGetXXXConfigurationHandle() functions.
 *
 * input  : hApp    - Stack handle for the application
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmStart(
        IN  HAPP        hApp)
{
    int rv=RVERROR;
    cmElem* app=(cmElem*)hApp;
    if (!hApp) return RVERROR;
    cmiAPIEnter(hApp,(char*)"cmStart(hApp=0x%x)",app);

    /* Make sure we haven't started yet */
    if (!app->start)
    {
        CMTRANSANNEXEPARAM eParams;
        /* Activate the transport module */
        INT32 port;
        int annexENodeId;
        cmTransportAddress tpktTA;
        cmTransportAddress annexETA;

        app->diffSrcAddressInSetupAndARQ=FALSE;
        if (pvtGetChild(app->hVal,app->q931Conf,__q931(diffSrcAddressInSetupAndARQ),NULL)>=0)
        {
            app->diffSrcAddressInSetupAndARQ=TRUE;
        }

        /* local IP address */
        {
            char tempIp[5];
            memset(tempIp, 0, sizeof(tempIp));
            ciGetString(app->hCfg,"system.localIPAddress",(char*)tempIp,sizeof(tempIp));
            memcpy((char*)&app->localIPAddress, (char*)tempIp, sizeof(app->localIPAddress));
        }

        pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(callSignalingPort),NULL),NULL,NULL,&port,NULL);
        tpktTA.type=cmTransportTypeIP;
        tpktTA.ip=app->localIPAddress;
        tpktTA.port=(UINT16)port;

        __pvtGetByFieldIds(annexENodeId, app->hVal, app->rasConf, {_q931(alternateTransportAddresses) _q931(annexE) _nul(1) _q931(ipAddress) _q931(port) LAST_TOKEN}, NULL, &port, NULL);
        if (annexENodeId < 0)
            port = 2517;
        annexETA.distribution=cmDistributionUnicast;
        annexETA.type=cmTransportTypeIP;
        annexETA.ip=app->localIPAddress;
        annexETA.port=(UINT16)port;

        {
            int nodeId;

            nodeId=cmGetQ931ConfigurationHandle(hApp);
            /* Annex E parameters */
            eParams.useAnnexE = FALSE;
            if ((nodeId=pvtGetChild(app->hVal,nodeId,__q931(useAnnexE) ,NULL))>=0)
                eParams.useAnnexE = TRUE;

            /* set the Annex E parameters */
            eParams.t_R1    = 500;
            eParams.t_R2    = (UINT)(eParams.t_R1 * 21 / 10);
            eParams.n_R1    = 8;
            eParams.t_IMA1  = 6000;
            eParams.n_IMA1  = 6;
            eParams.t_DT    = 1;

            if (eParams.useAnnexE)
            {
                if (pvtGetChildValue(app->hVal,nodeId,__q931(t_R1) ,(INT32 *)&eParams.t_R1,NULL)<0)
                    eParams.t_R1 = 500;
                eParams.t_R2 = (UINT)(eParams.t_R1 * 21 / 10);
                if (pvtGetChildValue(app->hVal,nodeId,__q931(n_R1) ,(INT32 *)&eParams.n_R1,NULL)<0)
                    eParams.n_R1 = 8;
                if (pvtGetChildValue(app->hVal,nodeId,__q931(t_IMA1) ,(INT32 *)&eParams.t_IMA1,NULL)<0)
                    eParams.t_IMA1 = 6000;
                if (pvtGetChildValue(app->hVal,nodeId,__q931(n_IMA1) ,(INT32 *)&eParams.n_IMA1,NULL)<0)
                    eParams.n_IMA1 = 6;
                if (pvtGetChildValue(app->hVal,nodeId,__q931(t_DT) ,(INT32 *)&eParams.t_DT,NULL)<0)
                    eParams.t_DT = 1;
            }

        }
        if (cmTransStart(app->hTransport,&tpktTA,&annexETA,(int)(app->localIPAddress),&eParams)==cmTransOK)
        {
            rv=0;
            if (!tpktTA.ip)
            {
                UINT32 **ipA=liGetHostAddrs();
                if (ipA && *ipA && **ipA)
                    tpktTA.ip = **ipA;
            }
            if (eParams.useAnnexE && !annexETA.ip)
            {
                UINT32 **ipA=liGetHostAddrs();
                if (ipA && *ipA && **ipA)
                    annexETA.ip = **ipA;
            }

            /* Set the Q931 address for the log */
            app->q931Chan=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
            cmTAToVt(app->hVal,app->q931Chan, &tpktTA);

            /* Set the Q931 address for the log */
            if (eParams.useAnnexE)
            {
                /* Make sure to "fix" the Annex E port inside RAS configuration if necessary */
                __pvtGetByFieldIds(annexENodeId, app->hVal, app->rasConf, {_q931(alternateTransportAddresses) _q931(annexE) _nul(1) LAST_TOKEN}, NULL, NULL, NULL);
                if (annexENodeId >= 0)
                    cmTAToVt(app->hVal, annexENodeId, &annexETA);

                app->q931AnnexEChan=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
                cmTAToVt(app->hVal,app->q931AnnexEChan, &annexETA);
            }
            else
                app->q931AnnexEChan=-1;

            /* Create a database of default values to messages */
            app->appDB=pvtAddRoot(app->hVal,app->hSyn,0,NULL);

            {
                int tmpNodeId,tmpNodeId1;
                int msgNodeId;
                char OID[10];
                int length;

                length=utlEncodeOID(sizeof(OID),OID,PROTOCOL_IDENTIFIER);

                app->retry=0;
                app->newRate=0;
                app->rate=0;
                app->multiRate=1;

                /*Setup*/
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(setup),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(setup) LAST_TOKEN},0,NULL);

                tmpNodeId=pvtAdd(app->hVal,msgNodeId,__q931(bearerCapability),0,NULL,NULL);
                tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet3),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(codingStandard),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(informationTransferCapability),8,NULL,NULL);

                tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet4),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(transferMode),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(informationTransferRate),16,NULL,NULL);

                tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet5),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(layer1Ident),1,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(userInformationLayer1Protocol),5,NULL,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(setup) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(activeMC),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(mediaWaitForConnect),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(multipleCalls),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(maintainConnection),0,NULL,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(conferenceGoal) _q931(create) LAST_TOKEN},0,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(callType) _q931(pointToPoint) LAST_TOKEN},0,NULL);
#if  0 /* Nextone */
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(screeningIndicator) _q931(userProvidedVerifiedAndFailed) LAST_TOKEN},0,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(presentationIndicator) _q931(presentationAllowed) LAST_TOKEN},0,NULL);
#endif

                /*CallProceeding*/
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(callProceeding),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(callProceeding) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(callProceeding) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(multipleCalls),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(maintainConnection),0,NULL,NULL);
#if  0 /* Nextone */
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(presentationIndicator) _q931(presentationAllowed) LAST_TOKEN},0,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(screeningIndicator) _q931(userProvidedVerifiedAndFailed) LAST_TOKEN},0,NULL);
#endif


                /*Alerting*/
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(alerting),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(alerting) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(alerting) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(multipleCalls),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(maintainConnection),0,NULL,NULL);
#if  0 /* Nextone */
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(presentationIndicator) _q931(presentationAllowed) LAST_TOKEN},0,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(screeningIndicator) _q931(userProvidedVerifiedAndFailed) LAST_TOKEN},0,NULL);

#endif
                /*Connect*/
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(connect),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(connect) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(connect) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(multipleCalls),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(maintainConnection),0,NULL,NULL);
#if  0 /* Nextone */
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(presentationIndicator) _q931(presentationAllowed) LAST_TOKEN},0,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(screeningIndicator) _q931(userProvidedVerifiedAndFailed) LAST_TOKEN},0,NULL);
#endif

                /*Release Complete*/
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(releaseComplete),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(releaseComplete) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(releaseComplete) LAST_TOKEN},0,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);
#if  0 /* Nextone */
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(presentationIndicator) _q931(presentationAllowed) LAST_TOKEN},0,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(screeningIndicator) _q931(userProvidedVerifiedAndFailed) LAST_TOKEN},0,NULL);
#endif

                /* Facility */
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(facility),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(facility) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(facility),0,NULL,NULL);
                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(facility) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(multipleCalls),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(maintainConnection),0,NULL,NULL);
#if  0 /* Nextone */
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(presentationIndicator) _q931(presentationAllowed) LAST_TOKEN},0,NULL);
                __pvtBuildByFieldIds(tmpNodeId,app->hVal,tmpNodeId1,
                    {_q931(screeningIndicator) _q931(userProvidedVerifiedAndFailed) LAST_TOKEN},0,NULL);
#endif
                pvtAddBranch2(app->hVal,tmpNodeId1,__q931(reason),__q931(undefinedReason));

                /* Status Enquiry */
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(statusEnquiry),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(statusEnquiry) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(statusInquiry) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);

                /* Status */
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(status),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(status) LAST_TOKEN},0,NULL);

                tmpNodeId=pvtAdd(app->hVal,msgNodeId,__q931(cause),0,NULL,NULL);
                tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet3),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(codingStandard),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(spare),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(location),0,NULL,NULL);
                tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet4),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(causeValue),0,NULL,NULL);

                tmpNodeId=pvtAdd(app->hVal,msgNodeId,__q931(callState),0,NULL,NULL);
                tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(codingStandard),0,NULL,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(status) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);

                /* Information */
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(information),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(information) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(information) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);

                /* Setup Acknowledge */
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(setupAck),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(setupAck) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(setupAcknowledge) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);

                /* Progress */
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(progress),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(progress) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(progress) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);

                /* Notify */
                tmpNodeId=pvtAdd(app->hVal,app->appDB,__q931(notify),0,NULL,NULL);

                pvtAdd(app->hVal,tmpNodeId,__q931(protocolDiscriminator),8,NULL,NULL);

                __pvtBuildByFieldIds(msgNodeId,app->hVal,tmpNodeId,
                    {_q931(message) _q931(notify) LAST_TOKEN},0,NULL);

                tmpNodeId1=pvtAdd(app->hVal,msgNodeId,__q931(userUser),0,NULL,NULL);
                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolDiscriminator),5,NULL,NULL);

                __pvtBuildByFieldIds(tmpNodeId1,app->hVal,tmpNodeId1,
                    {_q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(notify) LAST_TOKEN},0,NULL);

                pvtAdd(app->hVal,tmpNodeId1,__q931(protocolIdentifier),length,OID,NULL);

                cmiRASUpdateCallSignalingAddress(app->rasManager,app->q931Chan, app->q931AnnexEChan);
            }
            app->crv = UTILS_RandomGeneratorGetValue(&(app->seed))%32767;

            /* Initialize RAS */
            rv = rasStart(app);

            app->postControlDisconnectionDelay=2000;
            pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(postControlDisconnectionDelay),NULL),NULL,NULL,(INT32 *)&app->postControlDisconnectionDelay,NULL);


            /* Initialize H245 */
            cmH245Start(hApp);

            /* We're done */
            app->start=TRUE;
        }
    }
    cmiAPIExit(hApp,(char*)"cmStart=%d",rv);
    return rv;
}


/************************************************************************
 * cmStop
 * purpose: Stops the stack's activity
 *
 *          After the Stack is stopped by cmStop(), the application may change
 *          configuration settings and then use cmStart() to start Stack
 *          activity again.
 *
 * input  : hApp    - Stack handle for the application
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmStop(
        IN  HAPP        hApp)
{
    cmElem* app=(cmElem*)hApp;
    if (!app)
        return RVERROR;

    cmiAPIEnter((HAPP)app,(char*)"cmStop(hApp=0x%x)",app);
    if (app->start)
    {
        if(app->q931Chan>=0)
        {
            pvtDelete(app->hVal, app->q931Chan);
            app->q931Chan = RVERROR;
        }
        if(app->q931AnnexEChan>=0)
        {
            pvtDelete(app->hVal, app->q931AnnexEChan);
            app->q931AnnexEChan = RVERROR;
        }
        if(app->appDB>=0)
        {
            pvtDelete(app->hVal, app->appDB);
            app->appDB = RVERROR;
        }
        rasStop(app);
        cmH245Stop(hApp);
        cmTransStop(app->hTransport);
        app->start=FALSE;
    }
    cmiAPIExit((HAPP)app,(char*)"cmStop=0",0);
    return 0;
}

/************************************************************************
 * cmEnd
 * purpose: Shuts down the Conference Manager instance and releases all
 *          resources that were in use by the Conference Manager instance.
 * input  : hApp    - Stack handle for the application
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmEnd(
        IN   HAPP        hApp)
{
    cmElem* app=(cmElem*)hApp;
    RVHLOGMGR logMgr=NULL;
    tlsLockAll();

    if (app)
    {
        logMgr=app->logMgr;
        cmStop((HAPP)app);
        cmTransEnd(app->hTransport);
        cmiEndChannels(app->hChannels);
        cmiEndCalls(app->hCalls);
        mtimerEnd(app->hTimers);
        configTreesEnd(app);
        rasEnd(app);
        ciDestruct(app->hCfg);
        catDestruct(app->hCat);
        fastStartEnd(app);
        endPVT_PST(app);
        mibDestroyStatistic(app->hStatistic);
        meiEnd(app->hGlobalDataMEI);
        meiEnd(app->applicationLock);
        free(app);
    }
    liEnd();
    /*logEnd(logMgr);*/
    tlsUnlockAll();
    return 0;
}


/************************************************************************
 * cmGetVersion
 * purpose: Returns the version of the Conference Manager in use.
 * input  : none
 * output : none
 * return : Pointer to the string representing the version of the
 *          Conference Manager. For example, "3.0.0.0" or "2.5".
 ************************************************************************/
RVAPI
char* RVCALLCONV cmGetVersion(void)
{
    return (char*)RV_H323_STACK_VERSION;
}


/************************************************************************
 * cmGetVersionName
 * purpose: Returns the version of the Conference Manager in use.
 * input  : buff    - Buffer to set the version in
 *          length  - Maximum length of the buffer in bytes
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmGetVersionName(
        IN    char*   buff,
        IN    int     length)
{
    strncpy(buff,RV_H323_STACK_VERSION,length);
    return 0;
}


RVAPI
int RVCALLCONV cmSetGenEventHandler(
                              IN    HAPP    hApp,
                              IN    CMEVENT cmEvent,
                              IN    int     size)
{
    cmElem* app=(cmElem*)hApp;
    if (!hApp) return RVERROR;
    cmiAPIEnter((HAPP)app,(char*)"cmSetGenEventHandler(hApp=0x%x,cmEvent,size=%d)",hApp,size);
    memset(&((cmElem*)hApp)->cmMyEvent,0,sizeof(((cmElem*)hApp)->cmMyEvent));
    memcpy(&((cmElem*)hApp)->cmMyEvent,cmEvent,min((int)sizeof(((cmElem*)hApp)->cmMyEvent),size));
    cmiAPIExit((HAPP)app,(char*)"cmSetGenEventHandler=%d",0);
    return 0;
}

RVAPI
int RVCALLCONV cmGetGenEventHandler(
                              IN    HAPP    hApp,
                              OUT   CMEVENT cmEvent,
                              IN    int     size)
{
    cmElem* app=(cmElem*)hApp;
    if (!hApp) return RVERROR;
    cmiAPIEnter((HAPP)app,(char*)"cmGetGenEventHandler(hApp=0x%x,cmEvent,size=%d)",hApp,size);
    memset(cmEvent,0,size);
    memcpy(cmEvent,&((cmElem*)hApp)->cmMyEvent,min((int)sizeof(((cmElem*)hApp)->cmMyEvent),size));
    cmiAPIExit((HAPP)app,(char*)"cmGetGenEventHandler=%d",0);
    return 0;

}


RVAPI
int RVCALLCONV cmSetCallEventHandler(
                                   IN   HAPP        hApp,
                                   IN   CMCALLEVENT cmCallEvent,
                                   IN   int     size)
{
    cmElem* app=(cmElem*)hApp;
    if (!hApp) return RVERROR;
    cmiAPIEnter((HAPP)app,(char*)"cmSetCallEventHandler(hApp=0x%x,cmCallEvent,size=%d)",hApp,size);
    memset(&((cmElem*)hApp)->cmMyCallEvent,0,sizeof(((cmElem*)hApp)->cmMyCallEvent));
    memcpy(&((cmElem*)hApp)->cmMyCallEvent,cmCallEvent,min((int)sizeof(((cmElem*)hApp)->cmMyCallEvent),size));
    cmiAPIExit((HAPP)app,(char*)"cmSetCallEventHandler=%d",0);
    return 0;
}



RVAPI
int RVCALLCONV cmGetCallEventHandler(
                                   IN      HAPP        hApp,
                                   OUT     CMCALLEVENT cmCallEvent,
                                   INOUT   int         size)
{
    cmElem* app=(cmElem*)hApp;
    if (!hApp) return RVERROR;
    cmiAPIEnter((HAPP)app,(char*)"cmGetCallEventHandler(hApp=0x%x,cmCallEvent,size=%d)",hApp,size);
    memset(cmCallEvent,0,size);
    memcpy(cmCallEvent,&((cmElem*)hApp)->cmMyCallEvent,min((int)sizeof(((cmElem*)hApp)->cmMyCallEvent),size));
    cmiAPIExit((HAPP)app,(char*)"cmGetCallEventHandler=%d",0);
    return 0;

}


RVAPI int RVCALLCONV /* Real number of channels in configuration or RVERROR */
cmGetConfigChannels(
/* build array containing the channel names as appear in
the configuration. The strings are copied into array elements */
                    IN  HAPP        hApp,
                    IN  int         arraySize,
                    IN  int         elementLength, /* sizeof each string in array */
                    OUT char*       array[] /* allocated with elements */
)
{
    int result;
    cmElem* app=(cmElem*)hApp;

    if (!app) return RVERROR;
    cmiAPIEnter((HAPP)app,(char*)"cmGetConfigChannels: hApp=0x%x",hApp);

    result = confGetChannelNames(app->hVal, app->h245Conf, arraySize, elementLength, array);
    cmiAPIExit((HAPP)app,(char*)"cmGetConfigChannels() = %d",result);
    return result;
}



RVAPI
INT32 RVCALLCONV cmGetQ931ConfigurationHandle(
                                            IN  HAPP             hApp)
{
    cmElem *app = (cmElem *)hApp;

    if (!app) return RVERROR;
    cmiAPIEnter((HAPP)app, (char*)"cmGetQ931ConfigurationHandle: hApp=0x%x.", hApp);

    cmiAPIExit((HAPP)app, (char*)"cmGetQ931ConfigurationHandle: [%d].", app->q931Conf);
    return app->q931Conf;
}


int cmCallPreCallBack(HAPP hApp)
{
    cmElem* app=(cmElem*)hApp;
    if (!app || !app->newCallback) return RVERROR;
    return app->newCallback();
}


RVAPI int RVCALLCONV
cmSetPreCallbackEvent(
                      IN    HAPP                hApp,
                      IN    cmNewCallbackEH     newCallback
                      )
{
    cmElem* app=(cmElem*)hApp;
    if (!app) return RVERROR;

    cmiAPIEnter((HAPP)app, (char*)"cmSetPreCallbackEvent: hApp=0x%x.", hApp);
    app->newCallback=newCallback;
    cmiAPIExit((HAPP)app, (char*)"cmSetPreCallbackEvent() = [1]");

    return TRUE;
}

RVAPI HPVT RVCALLCONV
cmGetValTree(
             IN  HAPP            hApp
             )
{
    HPVT hPvt;
    cmElem* app=(cmElem*)hApp;

    if (!app) return NULL;
    cmiAPIEnter((HAPP)app, (char*)"cmGetValTree: hApp=0x%x.", hApp);

    hPvt=app->hVal;
    cmiAPIExit((HAPP)app, (char*)"cmGetValTree() = 0x%x",hPvt);

    return hPvt;
}

RVAPI HPST RVCALLCONV
cmGetSynTreeByRootName(
                       IN   HAPP            hApp,
                       IN   char*           name)
{
    HPST hPst=NULL;
    cmElem* app=(cmElem*)hApp;

    if (!app) return NULL;
    cmiAPIEnter((HAPP)app, (char*)"cmGetSynTreeByRootName: hApp=0x%x, name=%s.", hApp, name);

         if (!strcmp("ras",name))                   hPst=app->synProtRAS;
    else if (!strcmp("q931",name))                  hPst=app->synProtQ931;
    else if (!strcmp("h245",name))                  hPst=app->synProtH245;

    else if (!strcmp("RASConfiguration",name))      hPst=app->synConfRAS;
    else if (!strcmp("Q931Configuration",name))     hPst=app->synConfQ931;
    else if (!strcmp("H245Configuration",name))     hPst=app->synConfH245;

    else if (!strcmp("q931App",name))               hPst=app->hSyn;
    else if (!strcmp("rasApp",name))                hPst=app->hRASSyn;

    else if (!strcmp("redEncoding",name))           hPst=app->h245RedEnc;
    else if (!strcmp("capTransport",name))          hPst=app->h245TransCap;
    else if (!strcmp("capData",name))               hPst=app->h245DataType;

    else if (!strcmp("q931UU",name))                hPst=app->synQ931UU;
    else if (!strcmp("terminalLabel",name))         hPst=app->synTerminalLabel;
    else if (!strcmp("addr",name))                  hPst=app->hAddrSyn;

    else if (!strcmp("fsOpenLcn",name))             hPst=app->synOLC;

    else if (!strcmp("CryptoH323Token",name))       hPst=app->synGkCrypto;
    else if (!strcmp("PwdCertToken",name))          hPst=app->synGkPwdCert;
	/* Nextone */
    else if (!strcmp("ciscoarq",name))                 hPst=app->synCiscoARQ;
    else if (!strcmp("ciscoacf",name))                 hPst=app->synCiscoACF;
    else if (!strcmp("ciscoarj",name))                 hPst=app->synCiscoARJ;
    else if (!strcmp("ciscolrq",name))                 hPst=app->synCiscoLRQ;
    else if (!strcmp("ciscolcf",name))                 hPst=app->synCiscoLCF;
    else if (!strcmp("ciscolrj",name))                 hPst=app->synCiscoLRJ;

    cmiAPIExit((HAPP)app, (char*)"cmGetSynTreeByRootName: hApp=0x%x.", hApp);
    return hPst;
}


RVAPI int RVCALLCONV
cmSetHandle(
            IN      HAPP        hApp,
            IN      HAPPAPP     haApp)
{
    cmElem* app=(cmElem*)hApp;
    if (!app) return RVERROR;
    cmiAPIEnter((HAPP)app, (char*)"cmSetHandle: hApp=0x%x. haApp=0x%x.", hApp, haApp);
    app->haApp = haApp;
    cmiAPIExit((HAPP)app, (char*)"cmSetHandle: 0");
    return 0;
}

RVAPI int RVCALLCONV
cmGetHandle(
            IN      HAPP                hApp,
            IN      HAPPAPP*        haApp)
{
    cmElem* app=(cmElem*)hApp;
    if (!app) return RVERROR;
    cmiAPIEnter((HAPP)app, (char*)"cmGetHandle: hApp=0x%x.", hApp);
    *haApp=app->haApp;
    cmiAPIExit((HAPP)app, (char*)"cmGetHandle=(haApp=0x%x).", haApp);
    return 0;
}


RVAPI
int RVCALLCONV cmSetMessageEventHandler(
    IN HAPP hApp,
    IN cmEvCallNewRawMessageT   cmEvCallNewRawMessage,
    IN cmEvCallSendRawMessageT  cmEvCallSendRawMessage,
    IN cmEvCallReleaseMessageContextT
                                cmEvCallReleaseMessageContext)
{
    cmElem*  app= (cmElem *)hApp;

    if (hApp == NULL) return RVERROR;

    cmiAPIEnter(hApp, "cmSetMessageEventHandler(hApp=0x%x)", hApp);

    app->cmEvCallNewRawMessage=cmEvCallNewRawMessage;
    app->cmEvCallSendRawMessage=cmEvCallSendRawMessage;
    app->cmEvCallReleaseMessageContext=cmEvCallReleaseMessageContext;

    cmiAPIExit(hApp, "cmSetMessageEventHandler(hApp=0x%x) ret=0", hApp);
    return 0;
}



/************************************************************************
 * cmGetConfiguration
 * purpose: Gives access to the session's configuration context.
 *          The returned HCFG should be considered read-only, and should
 *          never be destroyed.
 * input  : hApp    - Stack instance handle
 * output : none
 * return : HCFG handle of the configuration on success
 *          NULL on failure
 ************************************************************************/
RVAPI
HCFG RVCALLCONV cmGetConfiguration(IN HAPP hApp)
{
    cmElem* app=(cmElem*)hApp;
    cmiAPIEnter(hApp, "cmGetConfiguration(hApp=0x%x)", hApp);
    cmiAPIExit(hApp, "cmGetConfiguration(hApp=0x%x) = 0x%x", hApp, app->hCfg);
    return app->hCfg;
}


/************************************************************************
 * cmGetDelimiter
 * purpose: Returns the character used as a delimiter between fields,
 *          such as in the destAddress and srcAddress fields in the
 *          cmCallMake function.
 * input  : hApp    - Stack instance handle
 * output : none
 * return : Delimiter character used
 ************************************************************************/
RVAPI char RVCALLCONV
cmGetDelimiter(HAPP hApp)
{
    cmElem* app=(cmElem*)hApp;
    cmiAPIEnter(hApp, "cmGetDelimiter(hApp=0x%x)", hApp);
    cmiAPIExit(hApp, "cmGetDelimiter(hApp=0x%x) = %c", hApp, app->delimiter);
    return app->delimiter;
}

/************************************************************************
 * cmTimerSet
 * purpose: Set a timer of the stack, reseting its value if it had one
 *          previously.
 * input  : hApp            - Stack's instance handle
 *          eventHandler    - Callback to call when timer expires
 *          context         - Context to use as parameter for callback function
 *          timeOut         - Timeout of timer in milliseconds
 * output : none
 * return : Timer's handle on success
 ************************************************************************/
HTI cmTimerSet(
        IN    HAPP                 hApp,
        IN    LPMTIMEREVENTHANDLER eventHandler,
        IN    void*                context,
        IN    UINT32               timeOut)
{
    cmElem* app=(cmElem*)hApp;
    if (timeOut > 0)
        return mtimerSet(app->hTimers, eventHandler, context, timeOut);
    else
        return (HTI)RVERROR;
}

/************************************************************************
 * cmTimerReset
 * purpose: Reset a timer if it's set
 *          Used mainly for call timers.
 * input  : hApp    - Stack's instance handle
 *          timer   - Timer to reset
 * output : timer   - Timer's value after it's reset
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmTimerReset(IN HAPP hApp, INOUT HTI* timer)
{
    cmElem* app=(cmElem*)hApp;
    if (*timer!=(HTI)RVERROR)
    {
        mtimerReset(app->hTimers,*timer);
        *timer=(HTI)RVERROR;
    }
    return 0;
}


void cmLock(IN HAPP hApp)
{
    cmElem* app=(cmElem*)hApp;
    meiEnter(app->hGlobalDataMEI);
}

void cmUnlock(IN HAPP hApp)
{
    cmElem* app=(cmElem*)hApp;
    meiExit(app->hGlobalDataMEI);
}


/* TODO: It should not apear in cm.h */
RVAPI int RVCALLCONV
cmSetPartnerHandle(HAPP hApp, HAPPPARTNER hAppPartner)
{
    cmElem* app=(cmElem*)hApp;
    app->hAppPartner=hAppPartner;
    return TRUE;
}

RVAPI int RVCALLCONV
cmGetPartnerHandle(HAPP hApp, HAPPPARTNER* hAppPartner)
{
    cmElem* app=(cmElem*)hApp;
    if (hAppPartner)
        *hAppPartner=app->hAppPartner;
    return TRUE;
}




/************************************************************************
 * cmMeiEnter
 * purpose: Enters critical section for the specified stack instance.
 * input  : hApp        - Stack's application handle
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmMeiEnter(IN HAPP hApp)
{
/* nextone */
#ifdef NEXTONE_USE_CMMEI
    cmElem* app = (cmElem *)hApp;

    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmMeiEnter(hApp=0x%x)", hApp);
    meiEnter(app->applicationLock);
    cmiAPIExit(hApp, "cmMeiEnter(hApp=0x%x) = 0", hApp);
#endif
    return 0;
}


/************************************************************************
 * cmMeiExit
 * purpose: Exits critical section for the specified stack instance.
 * input  : hApp        - Stack's application handle
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmMeiExit(IN HAPP hApp)
{
/* nextone */
#ifdef NEXTONE_USE_CMMEI
    cmElem* app = (cmElem *)hApp;

    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmMeiExit(hApp=0x%x)", hApp);
    meiExit(app->applicationLock);
    cmiAPIExit(hApp, "cmMeiExit(hApp=0x%x) = 0", hApp);
#endif
    return 0;
}


/************************************************************************
 * cmThreadAttach
 * purpose: Indicate that the current running thread can be used to catch
 *          events from the network.
 *          Threads that called cmInitialize() don't have to use this
 *          function.
 *          Although this function has hApp as a parameter, the result
 *          will take place for all stack instances
 * input  : hApp        - Stack's application handle
 *          threadId    - Unused
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmThreadAttach(HAPP hApp,UINT32 threadId)
{
    int ret;

    if (threadId);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmThreadAttach(hApp=0x%x)", hApp);
    ret = liInit();
    if (ret >= 0)
        ret = liThreadAttach(NULL);
    cmiAPIExit(hApp, "cmThreadAttach(hApp=0x%x) = %d", hApp, ret);
    return ret;
}


/************************************************************************
 * cmThreadDetach
 * purpose: Indicate that the current running thread cannot be used to catch
 *          events from the network.
 *          Although this function has hApp as a parameter, the result
 *          will take place for all stack instances
 * input  : hApp        - Stack's application handle
 *          threadId    - Unused
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmThreadDetach(HAPP hApp,UINT32 threadId)
{
    int ret;

    if (threadId);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmThreadDetach(hApp=0x%x)", hApp);
    liThreadDetach(NULL);
    ret = liEnd();
    cmiAPIExit(hApp, "cmThreadDetach(hApp=0x%x) = %d", hApp, ret);
    return ret;
}

/************************************************************************
 * cmLogMessage
 * purpose: Writes a user message into the log file under the APPL filter
 * input  : hApp        - Stack's application handle
 *          line        - The message to be printed (must be null-terminated)
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmLogMessage(IN HAPP hApp, IN const char *line)
{
    cmElem* app = (cmElem *)hApp;
    if (line); /* This line is for release mode warnings */

    if (!app) return RVERROR;

    logPrint(app->logAppl, RV_INFO,
             (app->logAppl, RV_INFO, "%.100s", line));
    return 0;
}

/* Nextone */
/************************************************************************
 * endCiscoPVT_PST
 * purpose: Deallocate any information related to PVT for Cisco non std 
 * input  : app         - Stack instance handle
 * output : none
 * return : none
 ************************************************************************/
void endCiscoPVT_PST(IN cmElem* app)
{
    pstDestruct(app->synCiscoARQ);
    pstDestruct(app->synCiscoACF);
    pstDestruct(app->synCiscoARJ);
    pstDestruct(app->synCiscoLRQ);
    pstDestruct(app->synCiscoLCF);
    pstDestruct(app->synCiscoLRJ);
}

/* Nextone */
/************************************************************************
 * initCiscoPVT_PST
 * purpose: Initialize any information related to PVT and PST
 * input  : app         - Stack instance handle
 *          vtNodeCount - Number of nodes in PVT node pool
 * output : none
 * return : none
 ************************************************************************/
void initCiscoPVT_PST(IN cmElem*app)
{
    app->synCiscoARQ    =pstConstruct(cmEmGetCiscoSyntax(),(char *)"ARQnonStandardInfo");
    app->synCiscoACF    =pstConstruct(cmEmGetCiscoSyntax(),(char *)"ACFnonStandardInfo");
    app->synCiscoARJ    =pstConstruct(cmEmGetCiscoSyntax(),(char *)"ARJnonStandardInfo");
    app->synCiscoLRQ    =pstConstruct(cmEmGetCiscoSyntax(),(char *)"LRQnonStandardInfo");
    app->synCiscoLCF    =pstConstruct(cmEmGetCiscoSyntax(),(char *)"LCFnonStandardInfo");
    app->synCiscoLRJ    =pstConstruct(cmEmGetCiscoSyntax(),(char *)"LRJnonStandardInfo");
}


#ifdef __cplusplus
}
#endif
