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

#include <pvaltree.h>
#include <mti.h>
#include <cm.h>
#include <cmControl.h>
#include <cmintr.h>

#include <userinput.h>

#include <h245.h>
#include <cmdebprn.h>


int userInput(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    int nesting;

    cmiCBEnter(hApp, "cmEvCallUserInput: haCall=0x%lx, hsCall=0x%lx, id=%d", (HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), message);
    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallUserInput) ((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), message);
    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
    cmiCBExit(hApp, "cmEvCallUserInput.");
    return TRUE;
}

RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputSupportIndicationBuild(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 IN cmUserInputSupportIndication userInputSupport,
                 OUT int * nodeId  /* nodeId of nonstandart UserInputSupportIndication */
                 )
{
    HPVT hVal=cmGetValTree(hApp);
    int rootId,supId,ret=0;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputSupportIndicationBuild: hApp=0x%lx, userInputSupport=%d", hApp, userInputSupport);

    if (nodeId)
        *nodeId=0;

    if ((rootId=pvtAddRootByPath(hVal, ((cmElem*)hApp)->synProtH245, (char*)"indication.userInput", 0, NULL)) <0)
    {
        cmiAPIExit(hApp, "cmUserInputBuildAlphanumeric: [%d]", rootId);
        return rootId;
    }
    supId = pvtAdd(hVal, rootId, __h245(userInputSupportIndication), 0, NULL, NULL);
    switch(userInputSupport)
    {
        case cmSupportNonStandard:
            *nodeId=pvtAdd(hVal, supId, __h245(nonStandard), 0, NULL, NULL);
            ret=*nodeId;
        break;
        case cmSupportBasicString:
            ret=pvtAdd(hVal, supId, __h245(basicString), 0, NULL, NULL);
        break;
        case cmSupportIA5String:
            ret=pvtAdd(hVal, supId, __h245(iA5String), 0, NULL, NULL);
        break;
        case cmSupportGeneralString:
            ret=pvtAdd(hVal, supId, __h245(generalString), 0, NULL, NULL);
        break;
    }

    if (ret >= 0)
        ret = rootId;

    cmiAPIExit(hApp, "cmUserInputBuildAlphanumeric: [%d]", ret);
    return ret;
}


RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputSignalBuild(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 cmUserInputSignalStruct *userInputSignalStruct
                 )
{
    HPVT hVal=cmGetValTree(hApp);
    int signalId,rtpId,rootId,ret=1;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputSignalBuild: hApp=0x%lx", hApp);

    if ((rootId=pvtAddRootByPath(hVal, ((cmElem*)hApp)->synProtH245, (char*)"indication.userInput", 0, NULL)) <0)
    {
        cmiAPIExit(hApp, "cmUserInputSignalBuild: [%d]", rootId);
        return rootId;
    }
    signalId = pvtAdd(hVal, rootId, __h245(signal), 0, NULL, NULL);
    ret=pvtAdd(hVal, signalId, __h245(signalType),1 ,(char *)&userInputSignalStruct->signalType, NULL);
    if (userInputSignalStruct->duration)
        ret=pvtAdd(hVal, signalId, __h245(duration),userInputSignalStruct->duration ,NULL, NULL);
    if (userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber)
    {
        rtpId = pvtAdd(hVal, signalId, __h245(rtp),0 ,NULL, NULL);
        ret=pvtAdd(hVal, rtpId, __h245(logicalChannelNumber),userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber ,NULL, NULL);
        if (userInputSignalStruct->cmUserInputSignalRtp.expirationTime)
            ret=pvtAdd(hVal, rtpId, __h245(expirationTime),userInputSignalStruct->cmUserInputSignalRtp.expirationTime ,NULL, NULL);
        if (userInputSignalStruct->cmUserInputSignalRtp.timestamp)
            ret=pvtAdd(hVal, rtpId, __h245(timestamp),userInputSignalStruct->cmUserInputSignalRtp.timestamp ,NULL, NULL);
    }

    if (ret >= 0)
        ret = rootId;

    cmiAPIExit(hApp, "cmUserInputSignalBuild: [%d]", ret);
    return ret;
}


RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputSignalUpdateBuild(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 cmUserInputSignalStruct *userInputSignalStruct
                 )
{
    HPVT hVal=cmGetValTree(hApp);
    int rootId,ret;
    int signalId,rtpId;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputSignalUpdateBuild: hApp=0x%lx", hApp);

    if ((rootId=pvtAddRootByPath(hVal, ((cmElem*)hApp)->synProtH245, (char*)"indication.userInput", 0, NULL)) <0)
    {
        cmiAPIExit(hApp, "cmUserInputSignalUpdateBuild: [%d]", rootId);
        return rootId;
    }
    signalId = pvtAdd(hVal, rootId, __h245(signalUpdate), 0, NULL, NULL);
    ret=pvtAdd(hVal, signalId, __h245(duration),userInputSignalStruct->duration ,NULL, NULL);
    if (userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber)
    {
        rtpId = pvtAdd(hVal, signalId, __h245(rtp),0 ,NULL, NULL);
        ret=pvtAdd(hVal, rtpId, __h245(logicalChannelNumber),userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber ,NULL, NULL);
    }

    if (ret >= 0)
        ret = rootId;

    cmiAPIExit(hApp, "cmUserInputSignalUpdateBuild: [%d]", ret);
    return ret;
}




RVAPI int RVCALLCONV  /* TRUE or RVERROR */
cmUserInputGetDetail(
            IN  HAPP hApp,
            IN  INT32 userInputId,
            OUT cmUserInputIndication* userInputIndication
           )
{
    HPVT hVal=cmGetValTree(hApp);
    int nsId;

    if (!hApp ) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputGetDetail: hApp=0x%lx, UI id=%d", hApp, userInputId);

    if ( (nsId = pvtGetChild(hVal, userInputId, __h245(nonStandard), NULL)) >=0)
    {
        *userInputIndication = cmUserInputNonStandard;
        cmiAPIExit(hApp, "cmUserInputGetDetail: [1] (nonStandard)");
        return nsId;
    }

    if ( (nsId = pvtGetChild(hVal, userInputId, __h245(alphanumeric), NULL)) >=0)
    {
        *userInputIndication = cmUserInputAlphanumeric;
        cmiAPIExit(hApp, "cmUserInputGetDetail: [1] (alphanumeric)");
        return nsId;
    }

    if ( (nsId = pvtGetChild(hVal, userInputId, __h245(userInputSupportIndication), NULL)) >=0)
    {
        *userInputIndication = cmUserInputSupport;
        cmiAPIExit(hApp, "cmUserInputGetDetail: [1] (userInputSupportIndication)");
        return nsId;
    }
    if ( (nsId = pvtGetChild(hVal, userInputId, __h245(signal), NULL)) >=0)
    {
        *userInputIndication = cmUserInputSignal;
        cmiAPIExit(hApp, "cmUserInputGetDetail: [1] (signal)");
        return nsId;
    }
    if ( (nsId = pvtGetChild(hVal, userInputId, __h245(signalUpdate), NULL)) >=0)
    {
        *userInputIndication = cmUserInputSignalUpdate;
        cmiAPIExit(hApp, "cmUserInputGetDetail: [1] (signalUpdate)");
        return nsId;
    }
    cmiAPIExit(hApp, "cmUserInputGetDetail: [-1]");
    return RVERROR;
}

RVAPI int RVCALLCONV
cmUserInputGetSignal(
           IN  HAPP hApp,
           IN  INT32 signalUserInputId,
           OUT cmUserInputSignalStruct * userInputSignalStruct
           )
{
    HPVT hVal=cmGetValTree(hApp);
    int nodeId,optId;
    INT32 length;
    if (!hApp ) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputGetSignal: hApp=0x%lx, UI id=%d", hApp, signalUserInputId);
    nodeId=pvtGetChildByFieldId(hVal, signalUserInputId, __h245(signalType), &length, NULL);
    pvtGetString(hVal, nodeId, 1, (char *)&userInputSignalStruct->signalType);

    nodeId=pvtGetChild(hVal,signalUserInputId,__h245(duration), NULL);
    if(nodeId>=0)
    pvtGet(hVal,nodeId,NULL,NULL,(INT32 *)&userInputSignalStruct->duration,NULL);
    else
    userInputSignalStruct->duration=0;
    nodeId = pvtGetChild(hVal,signalUserInputId,__h245(rtp),NULL);
    if (nodeId<0)
    userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber=0;
    else
    {
        pvtGetChildByFieldId(hVal, nodeId, __h245(logicalChannelNumber),
                    (INT32 *)&userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber, NULL);
        optId = pvtGetChildByFieldId(hVal, nodeId, __h245(timestamp),
                    (INT32 *)&userInputSignalStruct->cmUserInputSignalRtp.timestamp, NULL);
        if (optId<0)
          userInputSignalStruct->cmUserInputSignalRtp.timestamp =0;

        optId = pvtGetChildByFieldId(hVal, nodeId, __h245(expirationTime),
                    (INT32 *)&userInputSignalStruct->cmUserInputSignalRtp.expirationTime, NULL);
        if (optId<0)
          userInputSignalStruct->cmUserInputSignalRtp.expirationTime =0;

    }
    cmiAPIExit(hApp, "cmUserInputGetSignal: hApp=0x%lx", hApp);
    return TRUE;
}

RVAPI int RVCALLCONV
cmUserInputGetSignalUpdate(
           IN  HAPP hApp,
           IN  INT32 signalUserInputId,
         OUT cmUserInputSignalStruct * userInputSignalStruct
           )
{
    int nodeId;
    HPVT hVal=cmGetValTree(hApp);
    if (!hApp ) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputGetSignalUpdate: hApp=0x%lx, UI id=%d", hApp, signalUserInputId);

    nodeId=pvtGetChild(hVal,signalUserInputId,__h245(duration), NULL);
    pvtGet(hVal,nodeId,NULL,NULL,(INT32 *)&userInputSignalStruct->duration,NULL);
    nodeId = pvtGetChild(hVal,signalUserInputId,__h245(rtp), NULL);
    if (nodeId<0)
        userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber=0;
    else
    pvtGetChildByFieldId(hVal, nodeId, __h245(logicalChannelNumber),
                (INT32 *)&userInputSignalStruct->cmUserInputSignalRtp.logicalChannelNumber, NULL);

    cmiAPIExit(hApp, "cmUserInputGetSignalUpdate: hApp=0x%lx", hApp);
    return TRUE;
}


RVAPI int RVCALLCONV  /* return nodeId to make possible getting nonStandrd */
cmUserInputSupportGet(
           IN  HAPP hApp,
           IN  INT32 supportUserInputId,
         OUT cmUserInputSupportIndication * userInputSupportIndication
           )
{
    HPVT hVal=cmGetValTree(hApp);
    int nodeId;

    if (!hApp ) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputSupportGet: hApp=0x%lx, UI id=%d", hApp, supportUserInputId);

    nodeId =pvtChild(hVal,supportUserInputId);
    *userInputSupportIndication=(cmUserInputSupportIndication)(pvtGetSyntaxIndex(hVal,nodeId)-1);
#ifndef NOLOGSUPPORT
    {
        INTPTR   fieldId;
        char buff[30];
        pvtGet(hVal,nodeId,&fieldId,NULL,NULL,NULL);
        cmiAPIExit(hApp, "cmUserInputSupportGet: (%d) hApp=0x%lx",
            pstGetFieldName(pvtGetSynTree(hVal,supportUserInputId),fieldId,sizeof(buff),buff), hApp);
    }
#endif
    return *userInputSupportIndication;
}


/* user input________________________________________________________________________________ */


RVAPI int RVCALLCONV
cmCallSendUserInput(
            /* send user input msg: userInputId tree is deleted */
            /* Note: Select one of nonStandard or userData options */
            IN  HCALL hsCall,
            IN  INT32 userInputId /* indication.userInput tree node id */
            )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    int message, res = RVERROR;

    if (!hsCall || !hApp) return RVERROR;


    if (userInputId<0) return RVERROR;

    cmiAPIEnter(hApp, "cmCallSendUserInput: hsCall=0x%lx, id=%d", hsCall, userInputId);

    if (emaLock((EMAElement)hsCall))
    {
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        pvtMoveTree(hVal,pvtAddBranch2(hVal,message, __h245(indication), __h245(userInput)),userInputId);
        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallSendUserInput=%d", res);
    return res;
}




RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputBuildNonStandard(
                /* Build userUser message with non-standard data */
                IN  HAPP hApp,
                IN  cmNonStandardIdentifier *identifier,
                IN  char *data,
                IN  int dataLength /* in bytes */
                )
{
    HPVT hVal=cmGetValTree(hApp);
    int rootId, nsId,ret;

    if (!hApp) return RVERROR;
    if (!identifier) return RVERROR;
    if (!data || dataLength<1) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputBuildNonStandard: hApp=0x%lx, id=0x%lx, data=0x%lx (%d)",
          hApp, identifier, data, dataLength);

    if ((rootId=pvtAddRootByPath(hVal, ((cmElem*)hApp)->synProtH245, (char*)"indication.userInput", 0, NULL)) <0)
    {
        cmiAPIExit(hApp, "cmUserInputBuildNonStandard: [-1]");
        return rootId;
    }
    nsId = pvtBuildByPath(hVal, rootId, "nonStandard", 0, NULL);

    if ((ret=cmNonStandardParameterCreate(hVal, nsId, identifier, data, dataLength)) <0)
    {
        pvtDelete(hVal, rootId);
        cmiAPIExit(hApp, "cmUserInputBuildNonStandard: [%d]",ret);
        return RVERROR;
    }

    cmiAPIExit(hApp, "cmUserInputBuildNonStandard: [%d]", rootId);
    return rootId;
}




RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputBuildAlphanumeric(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 IN  cmUserInputData *userData
                 )
{
    HPVT hVal=cmGetValTree(hApp);
    int rootId,ret;

    if (!hApp) return RVERROR;
    if (!userData) return RVERROR;
    if (userData && (!userData->data || userData->length<1)) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputBuildAlphanumeric: hApp=0x%lx, userData=0x%lx", hApp, userData);

    if ((rootId=pvtAddRootByPath(hVal, ((cmElem*)hApp)->synProtH245, (char*)"indication.userInput", 0, NULL)) <0)
    {
        cmiAPIExit(hApp, "cmUserInputBuildAlphanumeric: [%d]", rootId);
        return rootId;
    }
    ret =pvtAdd(hVal, rootId, __h245(alphanumeric), userData->length, userData->data, NULL);

    if (ret >= 0)
        ret = rootId;

    cmiAPIExit(hApp, "cmUserInputBuildAlphanumeric: [%d]", ret);
    return ret;
}



RVAPI int RVCALLCONV  /* TRUE or RVERROR */
cmUserInputGet(
           /* Note: One of dataLength or userData->length is NULL (the other exists) */
           /* Note: pointers are set to addresses in the user input value tree */
           IN  HAPP hApp,
           IN  INT32 userInputId,

           /* 1) nonStandard */
           IN  cmNonStandardIdentifier *identifier, /* user allocated */
           IN  char *data, /* user allocated */
           INOUT INT32 *dataLength,  /* (in bytes) IN: data allocation. OUT: correct size */

           /* 2) userData */
           IN  cmUserInputData *userData /*.length: IN: data allocation. OUT: correct size */
           )
{
    HPVT hVal=cmGetValTree(hApp);
    int nsId, userDataLen=-1;

    if (!hApp || !identifier || !data || !dataLength || !userData) return RVERROR;

    cmiAPIEnter(hApp, "cmUserInputGet: hApp=0x%lx, UI id=%d", hApp, userInputId);

    if ( (nsId = pvtGetChildByFieldId(hVal, userInputId, __h245(nonStandard), NULL, NULL)) >=0)
    {
        cmNonStandardParameterGet(hVal, nsId, identifier, data, dataLength);
        userData->length=0;
        cmiAPIExit(hApp, "cmUserInputGet: [1] (nonStandard)");
        return TRUE;
    }

    userDataLen=userData->length;
    if ( (nsId = pvtGetChildByFieldId(hVal, userInputId, __h245(alphanumeric), (INT32 *)&(userData->length), NULL)) >=0)
    {
        userDataLen = pvtGetString(hVal, nsId, userDataLen-1, userData->data);
        userData->data[userDataLen]=0;
        *dataLength=0;
        cmiAPIExit(hApp, "cmUserInputGet: [1] (alphanumeric)");
        return TRUE;
    }

    cmiAPIExit(hApp, "cmUserInputGet: [-1] (RVERROR)");
    return RVERROR;
}


#ifdef __cplusplus
}
#endif



