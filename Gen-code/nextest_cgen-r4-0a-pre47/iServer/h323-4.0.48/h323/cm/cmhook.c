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
#include <cmintr.h>
#include <cmdebprn.h>
#include <psyntreeStackApi.h>
#include <rasutils.h>
#include <cmutils.h>
#include <cmCall.h>

RVAPI
int RVCALLCONV cmSetProtocolEventHandlerReplacement(
        IN  HAPP        hApp,
        IN  CMPROTOCOLEVENT cmProtocolEventReplacement,
        IN  int     size)
{
    cmElem* app=(cmElem*)hApp;
    cmiAPIEnter(hApp,"cmSetProtocolEventHandlerReplacement(hApp=0x%x,cmProtocolEventReplacement=%p,size=%d)",hApp,cmProtocolEventReplacement,size);
    app->cmMyProtocolEventReplacement=cmProtocolEventReplacement;
    app->cmMyProtocolEventReplacementSize=size;
    cmiAPIExit(hApp,"cmSetProtocolEventHandlerReplacement=%d",0);
    return 0;
}
RVAPI
int RVCALLCONV cmSetProtocolEventHandler(
        IN  HAPP        hApp,
        IN  CMPROTOCOLEVENT cmProtocolEvent,
        IN  int     size)
{
    cmElem* app=(cmElem*)hApp;
    cmiAPIEnter(hApp,"cmSetProtocolEventHandler(hApp=0x%x,cmProtocolEvent,size=%d)",hApp,size);
    if (app->cmMyProtocolEventReplacement)
    {
        memset(((cmElem*)hApp)->cmMyProtocolEventReplacement,0,app->cmMyProtocolEventReplacementSize);
        memcpy(((cmElem*)hApp)->cmMyProtocolEventReplacement,cmProtocolEvent,min(app->cmMyProtocolEventReplacementSize,size));
    }
    else
    {
        memset(&((cmElem*)hApp)->cmMyProtocolEvent,0,sizeof(((cmElem*)hApp)->cmMyProtocolEvent));
        memcpy(&((cmElem*)hApp)->cmMyProtocolEvent,cmProtocolEvent,min((int)sizeof(((cmElem*)hApp)->cmMyProtocolEvent),size));
    }
    cmiAPIExit(hApp,"cmSetProtocolEventHandler=%d",0);
    return 0;
}


RVAPI
int RVCALLCONV cmProtocolSendMessage(
        IN  HAPP        hApp,
        IN  HPROTCONN       hConn,
        IN  int             msg)
{
    TRANSERR result=cmTransErr;
    cmiAPIEnter(hApp,"cmProtocolSendMessage(hApp=0x%x)",hApp);
    result = cmTransHostSendMessage((HSTRANSHOST)hConn,  msg);
    if (result > 0)
        result = cmTransErr;
    cmiAPIExit(hApp,"cmProtocolSendMessage() = %d", result);

    return (int)result;
}

RVAPI
int RVCALLCONV cmProtocolSendMessageTo(
        IN  HAPP        hApp,
        IN  HPROTCONN   hConn,
        IN  int         msg,
        IN  int         addr)
{
    int result=RVERROR;
    cmElem* app=(cmElem*)hApp;
    cmTransportAddress ta;

    cmiAPIEnter(hApp,"cmProtocolSendMessageTo(hApp=0x%x)",hApp);

    cmVtToTA(app->hVal,addr,&ta);
    result = rasEncodeAndSend((rasModule*)app->rasManager,NULL,cmRASTrStageDummy,msg,hConn==cmGetRASConnectionHandle(hApp),&ta,FALSE,NULL);

    cmiAPIExit(hApp,"cmProtocolSendMessageTo=%d", result);
    return result;
}


RVAPI
int RVCALLCONV
cmProtocolCreateMessage(
                IN      HAPP                    hApp,
                IN      cmProtocol              protocol)
{
    cmElem* app=(cmElem*)hApp;
    int rootId;
    cmiAPIEnter(hApp,"cmProtocolCreateMessage (hApp=0x%x)",hApp);

    switch(protocol)
    {
        case cmProtocolQ931:
            rootId = pvtAddRoot(cmGetValTree(hApp),app->synProtQ931,0,NULL);
            break;
        case cmProtocolH245:
            rootId= pvtAddRoot(cmGetValTree(hApp),app->synProtH245,0,NULL);
            break;
        case cmProtocolRAS:
            rootId= pvtAddRoot(cmGetValTree(hApp),app->synProtRAS,0,NULL);
            break;
        default:
            cmiAPIExit(hApp,"cmProtocolCreateMessage RVERROR");
            return RVERROR;
    }
    cmiAPIExit(hApp,"cmProtocolCreateMessage root=%d",rootId);
    return rootId;
}

RVAPI
cmProtocol RVCALLCONV cmProtocolGetProtocol(
                IN      HAPP                    hApp,
                IN      int                     msg)
{
    cmElem* app=(cmElem*)hApp;
    HPVT hVal=cmGetValTree(hApp);
    HPST hSyn=pvtGetSynTree(hVal,msg);
    int synNodeId;
    if (!app || msg<0)
        return (cmProtocol)RVERROR;

    cmiAPIEnter(hApp,"cmProtocolGetProtocol hApp=0x%x",hApp);

    pvtGet(hVal, msg, NULL, &synNodeId, NULL, NULL);

    if (pstAreNodesCongruent(hSyn,synNodeId,app->synProtQ931,pstGetRoot(app->synProtQ931)))
    {
        cmiAPIExit(hApp,"cmProtocolGetProtocol hApp=0x%x cmProtocolQ931",hApp);
        return cmProtocolQ931;
    }
    if (pstAreNodesCongruent(hSyn,synNodeId,app->synProtRAS,pstGetRoot(app->synProtRAS)))
    {
        cmiAPIExit(hApp,"cmProtocolGetProtocol hApp=0x%x cmProtocolRAS",hApp);
        return cmProtocolRAS;
    }
    if (pstAreNodesCongruent(hSyn,synNodeId,app->synProtH245,pstGetRoot(app->synProtH245)))
    {
        cmiAPIExit(hApp,"cmProtocolGetProtocol hApp=0x%x cmProtocolH245",hApp);
        return cmProtocolH245;
    }

    cmiAPIExit(hApp,"cmProtocolGetProtocol hApp=0x%x cmProtocolUnknown",hApp);
    return cmProtocolUnknown;
}

RVAPI
char* RVCALLCONV cmProtocolGetProtocolName(
                IN      cmProtocol              protocol)
{
    switch(protocol)
    {
        case cmProtocolQ931:return (char*)"Q931";
        case cmProtocolH245:return (char*)"H245";
        case cmProtocolRAS: return (char*)"RAS";
        default:            return (char*)"Unknown";
    }
}


#ifdef __cplusplus
}
#endif



