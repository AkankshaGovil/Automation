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

#include <rvinternal.h>
#include <cmictrl.h>
#include <ms.h>
#include <conf.h>
#include <caputils.h>
#include <netutl.h>
#include <strutils.h>
#include <cmchan.h>
#include <oidutils.h>
#include <stkutils.h>
#include <h245.h>
#include <cmCtrlMSD.h>
#include <ti.h>
#include <cmdebprn.h>

#define ifE(a) if(a)(a)
#define CMERRMSG(f) {f; return RVERROR;}

/* request mode________________________________________________________________________________ */


int requestMode(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    inRM_T* in_RM=&ctrl->in_RM;
    INT32 sq=0;
    int nodeId;
    int nesting;

    pvtGetChildValue(hVal,message,__h245(sequenceNumber),&sq,NULL);
    in_RM->sq=(BYTE)sq;

    nodeId=pvtGetChild(hVal,message,__h245(requestedModes),NULL);
    cmiCBEnter(hApp, "cmEvCallRequestMode: haCall=0x%lx, hsCall=0x%lx, transfer, nodeId=%d.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), nodeId);
    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallRequestMode)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl),cmReqModeRequest, nodeId);
    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
    cmiCBExit(hApp, "cmEvCallRequestMode.");
    return TRUE;
}

int requestModeAck(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    outRM_T* out_RM=&ctrl->out_RM;
    int nodeId;
    INT32 sq;
    int nesting;

    pvtGetChildValue(hVal,message,__h245(sequenceNumber),&sq,NULL);

    if (out_RM->sq==(BYTE)sq)
    {
        cmTimerReset(hApp,&(out_RM->timer));
    }
    nodeId=pvtGetChild(hVal,message,__h245(response),NULL);
    cmiCBEnter(hApp, "cmEvCallRequestMode: haCall=0x%lx, hsCall=0x%lx, confirm, nodeId=%d.", (HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), nodeId);
    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallRequestMode)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), cmReqModeAccept, nodeId);
    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
    cmiCBExit(hApp, "cmEvCallRequestMode.");
    return TRUE;
}

int requestModeReject(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    outRM_T* out_RM=&ctrl->out_RM;
    int nodeId;
    INT32 sq;
    int nesting;

    pvtGetChildValue(hVal,message,__h245(sequenceNumber),&sq,NULL);

    if (out_RM->sq==(BYTE)sq)
    {
        cmTimerReset(hApp,&(out_RM->timer));
    }
    nodeId=pvtGetChild(hVal,message,__h245(cause),NULL);
    cmiCBEnter(hApp, "cmEvCallRequestMode: haCall=0x%lx, hsCall=0x%lx, reject, nodeId=%d.", (HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), nodeId);
    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallRequestMode)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), cmReqModeReject, nodeId);
    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
    cmiCBExit(hApp, "cmEvCallRequestMode.");
    return TRUE;
}

int requestModeRelease(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    inRM_T* in_RM=&ctrl->in_RM;
    INT32 sq=0;
    int nodeId;
    int nesting;

    pvtGetChildValue(hVal,message,__h245(sequenceNumber),&sq,NULL);
    in_RM->sq=(BYTE)sq;

    nodeId=pvtGetChildValue(hVal,message,__h245(requestedModes),&sq,NULL);
    cmiCBEnter(hApp, "cmEvCallRequestMode: haCall=0x%lx, hsCall=0x%lx, transfer, nodeId=%d.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), nodeId);
    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallRequestMode)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl),cmReqModeReleaseIn, nodeId);
    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
    cmiCBExit(hApp, "cmEvCallRequestMode.");
    return TRUE;
}

void RVCALLCONV reqMode_TimeoutEventsHandler(void*_ctrl)
{
    controlElem* ctrl=(controlElem*)_ctrl;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    int message,nodeId;
    outRM_T* out_RM=&ctrl->out_RM;

    if (!hApp)   return;

    if (emaLock((EMAElement)cmiGetByControl((HCONTROL)ctrl)))
    {
        int nesting;

        cmTimerReset(hApp,&(out_RM->timer));

        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(requestModeRelease));

        sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        cmiCBEnter(hApp, "cmEvCallRequestMode: haCall=0x%lx, hsCall=0x%lx, release.", (HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl));
        nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallRequestMode) ((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), cmReqModeReleaseOut, RVERROR);
        emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
        cmiCBExit(hApp, "cmEvCallRequestMode.");
        emaUnlock((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    }
}



RVAPI int RVCALLCONV
cmCallRequestMode(
          /* send request mode msg */
          /* Note: id shall be deleted when function returns */
          IN  HCALL hsCall,
          IN  INT32 modeDescriptorsId /* (requestMode.requestedModes) */
          )
{
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    int nodeId;
    outRM_T* out_RM;
    int message, res = RVERROR;
    int iSq;
    HPVT hVal=cmGetValTree(hApp);

    if (!hsCall || !hApp) return RVERROR;
    cmiAPIEnter(hApp, "cmCallRequestMode: hsCall=0x%lx, id=%d.", hsCall, modeDescriptorsId);

    if (emaLock((EMAElement)hsCall))
    {
        out_RM=&ctrl->out_RM;

        out_RM->sq++;
        out_RM->sq%=255;

        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(request),__h245(requestMode));
        iSq = out_RM->sq;
        pvtAdd(hVal,nodeId,__h245(sequenceNumber),iSq,NULL,NULL);
        nodeId = pvtAddBranch(hVal,nodeId,__h245(requestedModes));
        pvtMoveTree(hVal,nodeId,modeDescriptorsId);

        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
            INT32 timeout=10;
            pvtGetChildValue(hVal,((cmElem*)hApp)->h245Conf,__h245(requestModeTimeout),&(timeout),NULL);
            cmTimerReset(hApp,&(out_RM->timer));
            out_RM->timer=cmTimerSet(hApp,reqMode_TimeoutEventsHandler,(void*)ctrl,timeout*1000);
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallRequestMode=%d", res);
    return res;
}



RVAPI int RVCALLCONV
cmCallRequestModeAck(
          /* Acknowledge the request */
          IN  HCALL hsCall,
          IN  char* responseName /* (requestModeAck.response) */
          )
{
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    int nodeId;
    inRM_T* in_RM;
    int message, res = RVERROR;
    HPVT hVal;

    if (!hsCall || !hApp || !responseName) return RVERROR;
    hVal=cmGetValTree(hApp);


    cmiAPIEnter(hApp, "cmCallRequestModeAck: hsCall=0x%lx, '%s'.", hsCall, nprn(responseName));

    if (emaLock((EMAElement)hsCall))
    {
        int iSq;
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        in_RM=&ctrl->in_RM;
        nodeId=pvtAddBranch2(hVal,message, __h245(response),__h245(requestModeAck));
        iSq = in_RM->sq;
        pvtAdd(hVal,nodeId,__h245(sequenceNumber),iSq,NULL,NULL);
        pvtBuildByPath(((cmElem*)hApp)->hVal, pvtAddBranch(hVal,nodeId,__h245(response)), responseName, 0, NULL);

        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallRequestModeAck=%d", res);
    return res;
}



RVAPI int RVCALLCONV
cmCallRequestModeReject(
          /* Reject the request */
          IN  HCALL hsCall,
          IN  char* causeName /* requestModeReject.cause */
          )
{
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    int nodeId;
    inRM_T* in_RM;
    int message, res = RVERROR;
    HPVT hVal=cmGetValTree(hApp);

    if (!hsCall || !hApp || !causeName) return RVERROR;
    cmiAPIEnter(hApp, "cmCallRequestModeReject: hsCall=0x%lx, '%s'.", hsCall, nprn(causeName));
    if (emaLock((EMAElement)hsCall))
    {
        int iSq;
        in_RM=&ctrl->in_RM;
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(response),__h245(requestModeReject));
        iSq = in_RM->sq;
        pvtAdd(hVal,nodeId,__h245(sequenceNumber),iSq,NULL,NULL);
        pvtBuildByPath(hVal, pvtAddBranch(hVal,nodeId,__h245(cause)), causeName, 0, NULL);

        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallRequestModeReject=%d", res);
    return res;
}

RVAPI int RVCALLCONV  /* request mode node id or RVERROR */
cmRequestModeBuild(
           /* Build request mode msg */
           /* Note: entryId overrides name */
           IN  HAPP hApp,
           IN  cmReqModeEntry ** modes[] /* modes matrix in preference order. NULL terminated arrays */
           )
{
    int rootId, i, j, _descId, _entryId, ret;
    INT32 id;
    HPVT hVal=cmGetValTree(hApp);

    if (!hApp || !modes) return RVERROR;
    if (!modes[0]) return RVERROR; /* must contain at least one entry */
    cmiAPIEnter(hApp, "cmRequestModeBuild: hApp=0x%lx",hApp);

    if ((rootId=pvtAddRootByPath(hVal, ((cmElem*)hApp)->synProtH245,
                   (char*)"request.requestMode.requestedModes", 0, NULL)) <0)
    {
        cmiAPIExit(hApp,"cmRequestModeBuild: [%d]",rootId);
        return rootId;
    }
    for (i=0; modes[i]; i++)
    {
        _descId = pvtAdd(hVal, rootId, -444, 0, NULL, NULL);
        if (_descId<0)
        {
            cmiAPIExit(hApp,"cmRequestModeBuild: [%d]",_descId);
            return _descId;
        }
        for (j=0; modes[i][j]; j++)
        {
            if (modes[i][j]->entryId >=0)
            {
                _entryId = pvtAdd(hVal, _descId, -445, 0, NULL, NULL);
                if (_entryId<0)
                {
                    cmiAPIExit(hApp,"cmRequestModeBuild: [%d]",_entryId);
                    return _entryId;
                }

                pvtMoveTree(hVal, _entryId, modes[i][j]->entryId);
            }
            else
            {
                if (confGetModeEntry(hVal, ((cmElem*)hApp)->h245Conf, modes[i][j]->name, &id)==TRUE)
                {
                    ret =pvtAddTree(hVal, _descId, hVal, id);
                    if (ret<0)
                    {
                        cmiAPIExit(hApp,"cmRequestModeBuild: [%d]",ret);
                        return ret;
                    }
                }
            }
        }
    }
    cmiAPIExit(hApp,"cmRequestModeBuild: hApp=0x%lx, root =%d",hApp,rootId);
    return rootId;
}



/*
#define CM_MAX_MODES 32
#define CM_MAX_MODES_PTR (CM_MAX_MODES+10)
*/


int /* TRUE or RVERROR */
cmGetModeTypeName(
          /* Generate dataName using field name as in H.245 standard. */
          IN  HPVT hVal,
          IN  int dataTypeId, /* ModeElement node id */
          IN  int dataNameSize,
          OUT char *dataName, /* copied into user allocate space [128 chars] */
          OUT confDataType* type, /* of data */
          OUT INT32* typeId /* node id of media type */
          )
{
    HPST synConf = pvtGetSynTree(hVal, dataTypeId);
    int dataId = pvtChild(hVal, dataTypeId);
    int choice=-1;
    INTPTR fieldId=-1;
    INT32 _typeId=dataId;

    choice = pvtGetSyntaxIndex(hVal, dataId);
    switch(choice)
    {
        case 1: /* non standard */
        case 2:
        case 3: /* video and audio */
            _typeId = pvtChild(hVal, dataId);
        break;
        case 4: /* data */
            _typeId = pvtChild(hVal, pvtChild(hVal, dataId));
        break;
        default:
            strncpy(dataName, "encryptionData - not supported", dataNameSize);
        break;
    }

    if (dataName)
    {
        pvtGet(hVal, _typeId, &fieldId, NULL, NULL, NULL);
        pstGetFieldName(synConf, fieldId, dataNameSize, dataName);
    }
    if (type) *type=(confDataType)choice;
    if (typeId) *typeId = _typeId;

    return TRUE;
}


int
cmRequestModeBuildStructEntry(
                  /* Set the request mode structure by entry id */
                  IN  HPVT hVal,
                  IN  INT32 entryId,
                  cmReqModeEntry *entry
                  )
{
  entry->entryId = entryId;
  cmGetModeTypeName(hVal, pvtChild(hVal, entryId), sizeof(entry->name), entry->name, NULL, NULL);
  return TRUE;
}


RVAPI int RVCALLCONV /* TRUE or RVERROR */
cmRequestModeStructBuild(
             /* Build request mode matrix structure from msg */
             /* Note: entry name as in h.245 standard */
             IN  HAPP hApp,
             IN  INT32 descId, /* requestMode.requestedModes node id */
             IN  cmReqModeEntry **entries, /* user allocated entries */
             IN  int entriesSize, /* number of entries */
             IN  void **entryPtrs, /* pool of pointers to construct modes */
             IN  int ptrsSize, /* number of ptrs */

             /* modes matrix in preference order. NULL terminated arrays */
             OUT cmReqModeEntry ***modes[]
             )
{
  /*
     D: Descriptor
     E: Entry
     -: null

     ---------------------------------------------------------------------------
     | D(1) D(2) ... D - E(1,1) E2(1,2) ... E - E(2,1) - ... - E... E - - ===> |
     ---------------------------------------------------------------------------

     */


  /*
  cmReqModeEntry entries[CM_MAX_MODES];
  void *entryPtrs[CM_MAX_MODES_PTR];
  */

    int _descNum, _entryNum, _descId, _entryId, i, j;
    int _entryPos=0; /* vacant position in entries */
    int _entryPtrPos=0; /* next array position */
    HPVT hVal=cmGetValTree(hApp);

    if (!hApp || !modes || descId<0 || !entries || !entryPtrs) return RVERROR;
    cmiAPIEnter(hApp, "cmRequestModeStructBuild: hApp=0x%lx",hApp);

    if ((_descNum = pvtNumChilds(hVal, descId)) > ptrsSize)
    {
        cmiAPIExit(hApp, "cmRequestModeStructBuild: hApp=0x%lx: Overflow",hApp);
        return RVERROR; /* overflow */
    }
    entryPtrs[_descNum]=NULL; /* terminator */
    _entryPtrPos = _descNum+1;

    for (i=0; i<_descNum; i++)
    {
        _descId = pvtGetByIndex(hVal, descId, i+1, NULL);
        _entryNum = pvtNumChilds(hVal, _descId);
        entryPtrs[i]=&entryPtrs[_entryPtrPos];

        for (j=0; j<_entryNum; j++)
        {
            _entryId = pvtGetByIndex(hVal, _descId, j+1, NULL);
            if (_entryPtrPos+j >= ptrsSize || _entryPos >= entriesSize) /* overflow */
            {
                cmiAPIExit(hApp, "cmRequestModeStructBuild: RVERROR");
                return RVERROR;
            }
            cmRequestModeBuildStructEntry(hVal, _entryId, entries[_entryPos]);
            entryPtrs[_entryPtrPos+j] = entries[_entryPos];
            _entryPos++;
        }

        entryPtrs[_entryPtrPos+j]=NULL;
        _entryPtrPos+=_entryNum+1;

    }

    *modes = (cmReqModeEntry***)entryPtrs;
    cmiAPIExit(hApp, "cmRequestModeStructBuild: hApp=0x%lx",hApp);

    return TRUE;
}


/* NexTone: part of Radvision patch */
/************************************************************************
 * rmInit
 * purpose: Initialize the request mode process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
int rmInit(controlElem* ctrl)
{
    inRM_T* in_RM = &ctrl->in_RM;
    outRM_T* out_RM = &ctrl->out_RM;

    in_RM->sq=0;
    out_RM->sq=0;
    out_RM->timer = (HTI)RVERROR;

    return 0;
}

/* NexTone: part of Radvision patch */
/************************************************************************
 * rmEnd
 * purpose: Stop the request mode process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void rmEnd(IN controlElem* ctrl)
{
    outRM_T* out_RM = &ctrl->out_RM;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));

    /* Get rid of the timer if one exists */
    cmTimerReset(hApp,&(out_RM->timer));
}

#ifdef __cplusplus
}
#endif

