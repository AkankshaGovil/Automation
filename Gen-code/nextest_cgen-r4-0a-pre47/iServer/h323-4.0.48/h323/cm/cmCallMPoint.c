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


#include <cm.h>
#include <cmdebprn.h>
#include <cmCall.h>
#include <cmParam.h>

RVAPI
int RVCALLCONV cmCallJoin(
                        IN      HCALL               hsCall,
                        IN      HCALL               hsSameConferenceCall)
{
    HAPP hApp=cmGetAHandle((HPROTOCOL)hsCall);
    HPVT hVal;
    char CID[16];
    cmTransportAddress qAddress;
    char number[256];
    cmAlias alias;
    int status = 0;
    if (!hsCall || !hApp) return RVERROR;

    hVal=cmGetValTree(hApp);

    cmiAPIEnter(hApp,(char*)"cmCallJoin(hsCall=0x%lx,hsSameConferenceCall=0x%lx)",hsCall,hsSameConferenceCall);
    alias.string=number;
    cmCallGetParam(hsSameConferenceCall,cmParamCID,0,0,CID);
    if (!cmCallGetOrigin(hsCall,NULL))
    {
        switch(cmCallMasterSlaveStatus(hsSameConferenceCall))
        {
            case    cmMSSlave   :
            if (cmCallGetOrigin(hsSameConferenceCall,NULL))
            {
                int i=0;
                if (cmCallGetParam(hsSameConferenceCall,cmParamCalledPartyNumber,0,0,(char *)&alias)>=0)
                    cmCallSetParam(hsCall,cmParamCalledPartyNumber,0,sizeof(cmAlias),(char *)&alias);
                while (cmCallGetParam(hsSameConferenceCall,cmParamDestinationAddress,i,0,(char *)&alias)>=0)
                    cmCallSetParam(hsCall,cmParamAlternativeAliasAddress,i++,sizeof(cmAlias),(char *)&alias);
                if (i==0)
                {
                    if (cmCallGetParam(hsSameConferenceCall,cmParamDestCallSignalAddress,0,0,(char*)&qAddress)>=0)
                        cmCallSetParam(hsCall,cmParamAlternativeAddress,0,sizeof(cmTransportAddress),(char*)&qAddress);
                    else
                    {
                        cmiAPIExit(hApp,(char*)"cmCallJoin=-1");
                        return RVERROR;
                    }
                }
            }
            else
            {
                if (cmCallGetMCAddress(hsSameConferenceCall,&qAddress.ip,&qAddress.port)<0)
                {
                    cmiAPIExit(hApp,(char*)"cmCallJoin=-1");
                    return RVERROR;
                }
                /* if we are not using an MC, just tell the other side to join this call,
                slightly non-standard, but not that much */
                if (qAddress.port != 0)
                {
                    /* we are using an MC */
                    cmCallSetParam(hsCall,cmParamAlternativeAddress,0,
                        sizeof(cmTransportAddress),(char*)&qAddress);
                }
            }

            break;
            case    cmMSMaster  :
            cmCallGetParam(hsSameConferenceCall,cmParamCallSignalAddress,0,0,(char*)&qAddress);
            cmCallSetParam(hsCall,cmParamAlternativeAddress,0,
                sizeof(cmTransportAddress),(char*)&qAddress);
            break;
            case    cmMSError   :
            {
                cmiAPIExit(hApp,(char*)"cmCallJoin=-1");
                return RVERROR;
            }
        }
        cmCallSetParam(hsCall,cmParamFacilityCID,0,0,CID);
        {
            cmCallSetParam(hsCall,cmParamFacilityReason,0,cmReasonTypeRouteCallToMC,NULL);

            /* When sending this FACILITY message, we don't want the stack to set the CID of
               it to that of the call, since the whole point in sending this message is the
               new CID that we've set */
            m_callset((callElem*)hsCall, overrideCID, FALSE);
            status = cmCallFacility(hsCall,RVERROR);
            m_callset((callElem*)hsCall, overrideCID, TRUE);
        }
    }
    cmiAPIExit(hApp,(char*)"cmCallJoin=%d",status);
    return status;
}

RVAPI
int RVCALLCONV cmCallInvite(
                          IN      HCALL               hsCall,
                          IN      HCALL               hsSameConferenceCall)
{
    HAPP hApp=cmGetAHandle((HPROTOCOL)hsCall);
    char CID[16];
    cmiAPIEnter(hApp,(char*)"cmCallInvite(hsCall=0x%lx,hsSameConferenceCall=0x%lx)",hsCall,hsSameConferenceCall);
    cmCallGetParam(hsSameConferenceCall,cmParamCID,0,0,CID);
    if (cmCallGetOrigin(hsCall,NULL))
    {
        cmCallSetParam(hsCall,cmParamCID,0,0,CID);
        cmCallSetParam(hsCall,cmParamConferenceGoal,0,cmInvite,0);
    }
    cmiAPIExit(hApp,(char*)"cmCallInvite=0");
    return 0;
}


RVAPI
int RVCALLCONV cmCallIsSameConference(
                                    IN  HCALL       hsCall,
                                    IN  HCALL       hsAnotherCall
                                    )
{
    HAPP hApp=cmGetAHandle((HPROTOCOL)hsCall);
    char cid1[16],cid2[16];
    BOOL same;
    if (!hApp || !hsCall || !hsAnotherCall)
        return RVERROR;
    cmiAPIEnter(hApp,(char*)"cmCallIsSameConference(hsCall=0x%lx,hsAnotherCall=0x%lx)",hsCall,hsAnotherCall);
    cmCallGetParam(hsCall,cmParamCID,0,NULL,cid1);
    cmCallGetParam(hsAnotherCall,cmParamCID,0,NULL,cid2);
    same=!memcmp(cid1,cid2,16);
    cmiAPIExit(hApp,(char*)"cmCallIsSameConference=0");
    return same;
}

#ifdef __cplusplus
}
#endif
