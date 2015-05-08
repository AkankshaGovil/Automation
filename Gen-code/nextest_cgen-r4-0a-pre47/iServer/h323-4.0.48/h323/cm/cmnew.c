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



#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <log.h>
#include <cm.h>
#include <cmutils.h>
#include <cmdebprn.h>
#include <cmParam.h>
#include <cmCall.h>
#include <psyntreeStackApi.h>
#include <cmCrossReference.h>
#include <q931asn1.h>
#include <statistic.h>


RVAPI char* RVCALLCONV
cmGetProtocolMessageName(
                         IN      HAPP                hApp,
                         IN      int                 msg)
{
    INTPTR fieldId=RVERROR;
    char * ret;
    HPVT hVal=cmGetValTree(hApp);
    HPST hSyn=pvtGetSynTree(hVal,msg);
    cmElem* app=(cmElem*)hApp;
    int synNodeId;
    if (!app || msg<0) return NULL;
    cmiAPIEnter((HAPP)app, (char*)"cmGetProtocolMessageName: hApp=0x%x.", hApp);

    if (pvtGet(hVal, msg, NULL, &synNodeId, NULL, NULL) < 0)
        ret = NULL;
    else
    {
        if (pstAreNodesCongruent(hSyn,synNodeId,app->synProtRAS,pstGetRoot(app->synProtRAS)))
            pvtGet(hVal, pvtChild(hVal, msg),&fieldId, NULL, NULL, NULL);
        else
        if (pstAreNodesCongruent(hSyn,synNodeId,app->synProtQ931,pstGetRoot(app->synProtQ931)))
            pvtGet(hVal, pvtChild(hVal, pvtGetChild(hVal, msg,__q931(message), NULL)),&fieldId, NULL, NULL, NULL);
        else
        if (pstAreNodesCongruent(hSyn,synNodeId,app->synProtH245,pstGetRoot(app->synProtH245)))
            pvtGet(hVal, pvtChild(hVal, pvtChild(hVal, msg)),&fieldId, NULL, NULL, NULL);
        ret = pstGetFieldNamePtr(hSyn, fieldId);
    }

    cmiAPIExit((HAPP)app, (char*)"cmGetProtocolMessageName: hApp=0x%x,name %s", hApp,nprn(ret));
    return ret;

}


RVAPI
INT32 RVCALLCONV cmGetLocalCallSignalAddress(
                                           IN   HAPP             hApp,
                                           OUT     cmTransportAddress*  tr)
{
    cmElem *app = (cmElem *)hApp;
    int rc;
    if (!app) return RVERROR;
    cmiAPIEnter((HAPP)app, (char*)"cmGetLocalCallSignalAddress: hApp=0x%x.", hApp);
    rc= cmVtToTA(app->hVal,app->q931Chan, tr);

    cmiAPIExit((HAPP)app, (char*)"cmGetLocalCallSignalAddress: [%d].", rc);
    return rc;
}

/************************************************************************
 * cmGetLocalAnnexEAddress
 * purpose: Gets the local AnnexE address
 *          Annex E is used to send Q931 messages on UDP.
 * input  : hApp    - Application's stack handle
 * output : tr      - The local Annex E transport address.
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
INT32 RVCALLCONV cmGetLocalAnnexEAddress(
    IN   HAPP                   hApp,
    OUT  cmTransportAddress*    tr)
{
    cmElem *app = (cmElem *)hApp;
    int rc;
    if (!app) return RVERROR;
    cmiAPIEnter((HAPP)app, (char*)"cmGetLocalAnnexEAddress: hApp=0x%x.", hApp);
    rc= cmVtToTA(app->hVal,app->q931AnnexEChan, tr);

    cmiAPIExit((HAPP)app, (char*)"cmGetLocalAnnexEAddress: [%d].", rc);
    return rc;
}



RVAPI int RVCALLCONV cmGetProperty( IN  HPROTOCOL  hsProtocol)
{
    switch(emaGetType((EMAElement)hsProtocol))
    {
        case RAS_OUT_TX:
        case RAS_IN_TX : return cmiRASGetProperty((HRAS)hsProtocol);
        case CALL      : return getCallProperty((HCALL)hsProtocol);
        case CHAN      : return RVERROR;/*We do not support property for channels*/
    }
    return RVERROR;
}

RVAPI HAPP RVCALLCONV cmGetAHandle(IN  HPROTOCOL   hsProtocol)
{
    return (HAPP)emaGetInstance((EMAElement)hsProtocol);
}



RVAPI
HPROTCONN RVCALLCONV cmGetTpktChanHandle( IN HCALL hsCall,cmTpktChanHandleType tpktChanHandleType)
{
    HPROTCONN hProtCon=NULL;
    callElem*call=(callElem*)hsCall;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    if (!hsCall) return NULL;
    if (!app) return NULL;

    cmiAPIEnter((HAPP)app,(char*)"cmGetTpktChanHandle: hsCall=0x%x tpktChanHandleType = %d",hsCall,tpktChanHandleType);
    if (emaLock((EMAElement)hsCall))
    {
        cmTransGetSessionParam(call->hsTransSession,
            (tpktChanHandleType==cmQ931TpktChannel)?cmTransParam_host:cmTransParam_H245Connection,(void*)&hProtCon);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit((HAPP)app,(char*)"cmGetTpktChanHandle: hProtConn 0x%x",hProtCon);

    return hProtCon;
}

RVAPI
int RVCALLCONV cmSetTpktChanApplHandle(   IN HPROTCONN hConn,HAPPCONN hAppConn)
{
    if (hConn)
    {
        if (cmTransSetHostApplHandle((HSTRANSHOST)hConn, (HATRANSHOST)hAppConn)==cmTransOK)
            return TRUE;
    }
    return RVERROR;
}


RVAPI
int RVCALLCONV cmGetTpktChanApplHandle(   IN HPROTCONN hConn,HAPPCONN * hAppConn)
{
    if (hConn && hAppConn)
    {
        if (cmTransGetHostApplHandle((HSTRANSHOST)hConn, (HATRANSHOST*)hAppConn)==cmTransOK)
            return TRUE;
    }
    return RVERROR;
}





/************************************************************************
 * mibGetStatistic
 * purpose: Get the value of a statistics parameter for the MIB
 * input  : hApp    - Stack's application handle
 *          type    - Parameter type to get
 * output : none
 * return : Parameter's value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetStatistic(IN HAPP hApp, IN mibStatisticParamEnumT type)
{
    cmElem* app =     (cmElem* )hApp;
    h341StatisticParametersT * pStatistic = (h341StatisticParametersT *)(app->hStatistic);
    if (pStatistic==NULL)
        return RVERROR;

    return getStatistic( pStatistic,type);
}


/************************************************************************
 * cmMibEventSet
 * purpose: Set MIB notifications from the stack to an SNMP application
 * input  : hApp        - Stack's application handle
 *          mibEvent    - Event callbacks to set
 *          mibHandle   - Handle of MIB instance for the stack
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmMibEventSet(IN HAPP hApp, IN MibEventT* mibEvent, IN HMibHandleT mibHandle)
{
    cmElem* app =     (cmElem* )hApp;
    app->mibHandle = mibHandle;
    memset(&app->mibEvent,0,sizeof(MibEventT));
    if (mibEvent!=NULL)
        memcpy (&app->mibEvent,mibEvent,sizeof(MibEventT));
    return 0;
}




#ifdef __cplusplus
}
#endif
