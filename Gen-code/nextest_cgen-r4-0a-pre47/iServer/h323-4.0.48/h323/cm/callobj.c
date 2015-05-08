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
#include <q931asn1.h>
#include <cmutils.h>
#include <callobj.h>


/************************************************************************
 * callObjectCreate
 * purpose: Create a CAT struct for a given call by the call's object.
 * input  : app     - Stack's instance
 *          hsCall  - Stack's handle of the call
 *          call    - Type of call - Incoming or Outgoing
 * output : callObj - CAT struct for the call
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int
callObjectCreate(
          IN  cmElem *app,
          IN  HCALL hsCall,
          IN  callSrcEnum   call,
          OUT catStruct *callObj)
{
    if (!app || !hsCall || !callObj) return RVERROR;

    memset(callObj, 0, sizeof(catStruct));
    callObj->flags = 0;

  /* rasCrv */
    switch(call)
    {
        case Outgoing:
            if (!app->gatekeeper)
                if (cmCallGetParam(hsCall,cmParamCRV,0,(INT32*)&callObj->rasCRV,NULL)>=0)
                    callObj->flags |= catRasCRV;
        break;
        case Incoming:
            if (cmCallGetParam(hsCall,cmParamRASCRV,0,(INT32*)&callObj->rasCRV,NULL)>=0)
                callObj->flags |= catRasCRV;
        break;

        default:
        break;
    }

    /* callId */
    switch(call)
    {
        case Outgoing:
        case Incoming:
            if (cmCallGetParam(hsCall,cmParamCallID,0,NULL, (char *)callObj->callID)>=0)
                callObj->flags |= catCallId;
        break;

        default:
        break;
    }

    /* source call signalling address */
    switch(call)
    {
        case Outgoing:
        case Incoming:
            if (cmCallGetParam(hsCall,cmParamSrcCallSignalAddress,0,NULL, (char *)&callObj->srcCallSignalAddr)>=0)
                callObj->flags |= catSrcCallSignalAddr;
        break;

        default:
        break;
    }

    /* dest call signalling address */
    switch(call)
    {
        case Outgoing:
            if (cmCallGetParam(hsCall,cmParamDestCallSignalAddress,0,NULL, (char *)&callObj->destCallSignalAddr)>=0)
                callObj->flags |= catDestCallSignalAddr;
        break;

        case Incoming:
            /* No need to handle incoming in this case */
        default:
        break;
    }

    /* cid */
    {
        INT32 size=16;
        switch(call)
        {

            case Outgoing:
            case Incoming:
                if (cmCallGetParam(hsCall,cmParamCID,0,&size,(char*)callObj->cid)>=0)
                    callObj->flags |= catCID;
            break;

            default:
            break;
        }
    }


    /* answerCall */
    switch(call)
    {
        case Outgoing:
            callObj->flags |= catAnswerCall;
            callObj->answerCall=FALSE;
        break;
        case Incoming:
            callObj->flags |= catAnswerCall;
            callObj->answerCall=TRUE;
        break;

        default:
        break;
    }

    return 0;
}

int
callObjectCreateFromMessage(
          IN  cmElem *app,
          IN  int message,
          OUT catStruct *callObj)
{
    int q931NodeId,perNodeId;
    int tmpNodeId;
    HPVT hVal=app->hVal;

    if (!app || message<0 || !callObj) return RVERROR;

    memset(callObj, 0, sizeof(catStruct));

    /*CRV*/
    pvtGet(hVal,pvtChild(hVal,pvtGetChild(hVal,message,__q931(callReferenceValue),NULL)),NULL,NULL,(INT32*)&(callObj->crv),NULL);
    callObj->crv^=0x8000;
    callObj->flags |= catCRV;

    __pvtGetNodeIdByFieldIds(q931NodeId,hVal,message, {_q931(message) _anyField LAST_TOKEN});
    __pvtGetNodeIdByFieldIds(perNodeId,hVal,q931NodeId, {_q931(userUser) _q931(h323_UserInformation) _q931(h323_uu_pdu)
                                                         _q931(h323_message_body) _anyField LAST_TOKEN});

    /*CID*/
    pvtGetString(hVal,pvtGetChild(hVal,perNodeId,__q931(conferenceID),NULL),16,(char*)callObj->cid);
    callObj->flags |= catCID;


    /*CallID*/
    if ((tmpNodeId=pvtGetChild(hVal,perNodeId,__q931(callIdentifier),NULL))>=0)
    {
        pvtGetString(hVal,pvtGetChild(hVal,tmpNodeId,__q931(guid),NULL),16,(char*)callObj->callID);
        callObj->flags |= catCallId;
    }

    /*srcCallSignalAddr*/
    if ((tmpNodeId=pvtGetChild(hVal,perNodeId,__q931(sourceCallSignalAddress),NULL))>=0)
    {
        cmTransportAddress ta;
        cmVtToTA(hVal,tmpNodeId,&ta);
        callObj->flags |= catSrcCallSignalAddr;
    }
    /*destCallSignalAddr*/
    /*The destCallSignalAddr is not in the callObj intentionally,
      when GK receives Setup message it should not use destCallSignalAddr for call association*/

    /*answerCall*/
    callObj->answerCall=TRUE;
    callObj->flags |= catAnswerCall;
    return 0;
}


#ifdef __cplusplus
}
#endif



