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
#include <cmintr.h>
#include <log.h>
#include <q931asn1.h>
#include <stkutils.h>
#include <cmdebprn.h>
#include <cmutils.h>
#include <cmParam.h>


int cmCallStatusMsg2Struct(HPVT hVal,int handle,cmCallStatusMessage * callStatusMsg)
{
    int nodeId;
    UINT32 value;
    nodeId = pvtGetChild(hVal,handle,__q931(cause),NULL);
    if (nodeId>=0)
    {
        int octet;
        octet = pvtGetChild(hVal,nodeId,__q931(octet3),NULL);

        pvtGetChildValue(hVal,octet,__q931(codingStandard),(INT32*)&value,NULL);
        callStatusMsg->callCauseInfo.cmCodingStandard = (UINT8)value;
        callStatusMsg->callCauseInfo.cmSpare = 0;
        pvtGetChildValue(hVal,octet,__q931(location),(INT32*)&value,NULL);
        callStatusMsg->callCauseInfo.cmLocation = (UINT8)value;

        value=(UINT)-1;
        pvtGetChildValue(hVal,octet,__q931(recomendation),(INT32*)&value,NULL);
        callStatusMsg->callCauseInfo.cmRecomendation = (UINT8)value;

        octet = pvtGetChild(hVal,nodeId,__q931(octet4),NULL);
        pvtGetChildValue(hVal,octet,__q931(causeValue),(INT32*)&value,NULL);
        callStatusMsg->callCauseInfo.cmCauseValue = (UINT8)value;
    }
    nodeId = pvtGetChild(hVal,handle,__q931(callState), NULL);
    if (nodeId>=0)
    {
        pvtGetChildByFieldId(hVal,nodeId,__q931(codingStandard),(INT32*)&value,NULL);
        callStatusMsg->callStateInfo.cmCodingStandard = (UINT8)value;

        pvtGetChildByFieldId(hVal,nodeId,__q931(callStateValue),(INT32*)&value,NULL);
        callStatusMsg->callStateInfo.cmCallStateValue = (UINT8)value;

    }
    nodeId = pvtGetChild(hVal,handle,__q931(display), NULL);
    if (nodeId >=0)
        pvtGetString(hVal,nodeId,sizeof(callStatusMsg->display),callStatusMsg->display);
    else
        callStatusMsg->display[0]=0;

    return TRUE;
}

RVAPI
int RVCALLCONV cmCallUserInformationCreate(IN HCALL hsCall,char * display,cmNonStandardParam* param)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    int nodeId,rootId,ret=1,nonStandardId;
    if (!hsCall || !app) return RVERROR;

    cmiAPIEnter((HAPP)app,(char*)"cmCallUserInformationCreate: hsCall=0x%x, Display %s",hsCall,nprn(display));
    rootId=pvtAddRoot(app->hVal,app->synProtQ931,0,NULL);
    if (rootId<0)
    {
        cmiAPIExit((HAPP)app,(char*)"cmCallUserInformationCreate: [%d]",rootId);
        return rootId;
    }
    if (emaLock((EMAElement)hsCall))
    {
        nodeId=callGetMessage(hsCall,cmQ931information);
        ret =pvtSetTree(app->hVal,rootId,app->hVal,nodeId);
        if (ret<0)
        {
            emaUnlock((EMAElement)hsCall);
            cmiAPIExit((HAPP)app,(char*)"cmCallUserInformationCreate: [%d]",ret);
            return ret;
        }
        if (display!=NULL)
        {
            __pvtBuildByFieldIds(ret,app->hVal,rootId,
                                    {_q931(message) _q931(information) _q931(display) LAST_TOKEN},
                                    (int)strlen(display),display);
        }
        if (param!=NULL)
        {
            __pvtBuildByFieldIds(nonStandardId,app->hVal,rootId,
                                {_q931(message)
                                _q931(information)
                                _q931(userUser)
                                _q931(h323_UserInformation)
                                _q931(h323_uu_pdu)
                                _q931(nonStandardData)
                                LAST_TOKEN},0,NULL);

            ret = setNonStandardParam(app->hVal,nonStandardId,param);
        }
        emaUnlock((EMAElement)hsCall);
    }

    if (ret >= 0)
        ret = rootId;

    cmiAPIExit((HAPP)app,(char*)"cmCallUserInformationCreate: [%d]", ret);
    return ret;
}

/*
display is  user allocated buffer
displaySize is the size of display
nodeId is the node id passed to callback function
"q931...message.information"
*/
RVAPI
int RVCALLCONV cmCallUserInformationGet(IN HAPP hApp,IN int message,
                                      OUT char *display,IN int displaySize,
                                      OUT cmNonStandardParam* param)
{
    cmElem* app;
    int tmpNodeId,nodeId;
    HPVT hVal;
    app = (cmElem* )hApp;
    if (!app)return RVERROR;
    hVal=app->hVal;

    cmiAPIEnter((HAPP)app,(char*)"cmCallUserInformationGet: hApp=0x%x,nodeId=%d ",hApp,message);

    tmpNodeId=pvtGetChild2(hVal,message,__q931(message),__q931(information));

	if (display)
	{
		display[0]=0;
	    nodeId=pvtGetChild(hVal,tmpNodeId,__q931(display),NULL);
		if (nodeId>=0)
		{
			int size;

			size=pvtGetString(app->hVal,nodeId,displaySize,display);
			if(size<displaySize) display[size]=0;
		}
	}
    if (param)
    {
        __pvtGetNodeIdByFieldIds(nodeId,hVal,tmpNodeId,
                                {_q931(userUser)
                                _q931(h323_UserInformation)
                                _q931(h323_uu_pdu)
                                _q931(nonStandardData)
                                LAST_TOKEN});
        if (nodeId >= 0)
            getNonStandardParam(hVal,nodeId,param);
        else
            param->length = -1;
    }
    cmiAPIExit((HAPP)app,(char*)"cmCallUserInformationGet: [OK]");
    return TRUE;
}

/************************************************************************************
 *
 * cmCallProgressCreate
 *
 * Purpose: To create PROGRESS message and fill it with the specified parameters
 *
 * Input:   hsCall                  - The stack handle to the call
 *          display                 - The display field
 *          cause                   - The causeValue (-1 if not used)
 *          pi_IE                   - Indicator whether IE to use for progressIndicator IE
 *                                    30 , 31 or both
 *          progressDescription     - The value of the progressIndicator's progressDescription (-1 if not used)
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if not used)
 *          param                   - The nonStandardParam to be put into the message (NULL if not used)
 *
 * Output:  None.
 *
 * Returned value: The root node ID of the created progress message on success
 *                 This root value should be passed to cmCallProgress() function.
 *                 Negative value on failure
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallProgressCreate(
        IN HCALL                hsCall,
                                  IN char*          display,
                                  IN int            cause,
                                  IN progressInd_IE pi_IE,
                                  IN int            progressDescription,
                                  IN int            notificationDescription,
                                  IN cmNonStandardParam* param)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    int nodeId,rootId,ret=1,q931NodeId,nonStandardId;
    if (!hsCall || !app) return RVERROR;

    cmiAPIEnter((HAPP)app,(char*)"cmCallProgressCreate: hsCall=0x%x, Display %s",hsCall,nprn(display));
    rootId=pvtAddRoot(app->hVal,app->synProtQ931,0,NULL);
    if (rootId<0)
    {
        cmiAPIExit((HAPP)app,(char*)"cmCallProgessCreate: [%d]",rootId);
        return rootId;
    }

    if (emaLock((EMAElement)hsCall))
    {
        /* Get the progress message for this call */

        nodeId=callGetMessage(hsCall,cmQ931progress);

        /* Copy it to this specific message - we need it only for the send process of the Progress message */
        ret =pvtSetTree(app->hVal,rootId,app->hVal,nodeId);
        if (ret<0)
        {
            emaUnlock((EMAElement)hsCall);
            cmiAPIExit((HAPP)app,(char*)"cmCallProgressCreate: [%d]",ret);
            return ret;
        }
        __pvtBuildByFieldIds(q931NodeId,app->hVal,rootId,
                                {_q931(message) _q931(progress) LAST_TOKEN},0,NULL);

        /* Build the Cause struct in Q931 if we have to */
        if (cause>=0)
        {
            int tmpNodeId,tmpNodeId1;

            tmpNodeId=pvtAdd(app->hVal,q931NodeId,__q931(cause) ,0,NULL,NULL);
            tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet3),0,NULL,NULL);
            pvtAdd(app->hVal,tmpNodeId1,__q931(codingStandard),0,NULL,NULL);
            pvtAdd(app->hVal,tmpNodeId1,__q931(spare),0,NULL,NULL);
            pvtAdd(app->hVal,tmpNodeId1,__q931(location),0,NULL,NULL);
            tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet4),0,NULL,NULL);
            pvtAdd(app->hVal,tmpNodeId1,__q931(causeValue),cause,NULL,NULL);
        }
        /* Build the progressIndicator if we have to */
        if (progressDescription>=0)
        {
            int tmpNodeId,tmpNodeId1;
            int i;
            INTPTR fieldIds[]={__q931(progressIndicator),__q931(progressIndicator31)};
            INTPTR pi_IEs[]={use30asPI_IE,use31asPI_IE};
            /* We can put progress in tag 30 or 31. We can also do it in both of them */
            for (i=0;i<2;i++)
            {
                if (pi_IEs[i]&pi_IE)
                {
                    tmpNodeId=pvtAdd(app->hVal,q931NodeId,fieldIds[i],0,NULL,NULL);
                    tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet3),0,NULL,NULL);
                    pvtAdd(app->hVal,tmpNodeId1,__q931(codingStandard),0,NULL,NULL);
                    pvtAdd(app->hVal,tmpNodeId1,__q931(spare),0,NULL,NULL);
                    pvtAdd(app->hVal,tmpNodeId1,__q931(location),0,NULL,NULL);
                    tmpNodeId1=pvtAdd(app->hVal,tmpNodeId,__q931(octet4),0,NULL,NULL);
                    pvtAdd(app->hVal,tmpNodeId1,__q931(progressDescription),progressDescription,NULL,NULL);
                }
            }
        }

        /* Add notification indicator if we have to */
        if (notificationDescription>=0)
        {
            __pvtBuildByFieldIds(ret,app->hVal,q931NodeId,
                                    {_q931(notificationIndicator)
                                     _q931(octet3) _q931(notificationDescription) LAST_TOKEN},notificationDescription,NULL);
        }
        /* Build Display if we have to */
        if (display!=NULL)
            pvtAdd(app->hVal,q931NodeId,__q931(display),(int)strlen(display),display,NULL);

        /* Add non standard data if we have to */
        if (param!=NULL)
        {
            __pvtBuildByFieldIds(nonStandardId,app->hVal,q931NodeId,
                                {_q931(userUser)
                                 _q931(h323_UserInformation)
                                 _q931(h323_uu_pdu)
                                 _q931(nonStandardData)
                                 LAST_TOKEN},0,NULL);

            ret = setNonStandardParam(app->hVal,nonStandardId,param);
        }
        emaUnlock((EMAElement)hsCall);
    }

    if (ret >= 0)
        ret = rootId;

    cmiAPIExit((HAPP)app,(char*)"cmCallProgressCreate: [%d]", ret);
    return ret;
}


/************************************************************************************
 *
 * cmCallProgressGet
 *
 * Purpose: To decode PROGRESS message
 *
 * Input:   hApp                    - The stack instance handle
 *          message                 - Progress message to decode
 *          displaySize             - The display field size in bytes.
 *
 * Output:  display                 - The display field (user allocated memory)
 *          cause                   - The causeValue (-1 if absent)
 *          pi_IE                   - Indicator whether IE to use for progressIndicator IE
 *                                    30 , 31 or both
 *          progressDescription     - The value of the progressIndicator's progressDescription (-1 if absent)
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if absent)
 *          param                   - The nonStandardParam from the message (param->length==RVERROR if it wasn't there)
 *          ** Any of the output fields can be passed as NULL if the
 *             application doesn't care about their values
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallProgressGet(
        IN  HAPP                hApp,
                               IN  int              message,
                               OUT char*            display,
                               IN  int              displaySize,
                               OUT int*             cause,
                               OUT progressInd_IE*  pi_IE,
                               OUT int*             progressDescription,
                               OUT int*             notificationDescription,
                               OUT cmNonStandardParam* param)
{
    cmElem* app;
    int q931NodeId,nodeId;
    HPVT hVal;
    app = (cmElem* )hApp;
    if (!app)return RVERROR;
    hVal=app->hVal;

    cmiAPIEnter((HAPP)app,(char*)"cmCallProgressGet: hApp=0x%x,nodeId=%d ",hApp,message);

    q931NodeId=pvtGetChild2(hVal,message,__q931(message),__q931(progress));
    if (display)
    {
        nodeId=pvtGetChild(hVal,q931NodeId,__q931(display),NULL);
        if (displaySize > 0)
            display[0] = 0;
        if (nodeId>=0)
        {
            int size;
            size=pvtGetString(app->hVal,nodeId,displaySize,display);
            if(size<displaySize) display[size]=0;
        }
    }
    if (cause)
    {
        *cause=RVERROR;
        __pvtGetByFieldIds(nodeId,hVal,q931NodeId,
                        {_q931(cause)
                         _q931(octet4)
                         _q931(causeValue)
                         LAST_TOKEN}, NULL, (INT32 *)cause, NULL);
    }
    if (progressDescription)
    {
        *progressDescription=RVERROR;
        *pi_IE=(progressInd_IE)RVERROR;
        __pvtGetByFieldIds(nodeId,hVal,q931NodeId,
                            {_q931(progressIndicator)
                             _q931(octet4)
                             _q931(progressDescription)
                             LAST_TOKEN}, NULL,(INT32 *)progressDescription, NULL);
        if (nodeId>=0)
            *pi_IE=use30asPI_IE;
        else
        {
            __pvtGetByFieldIds(nodeId,hVal,q931NodeId,
                            {_q931(progressIndicator)
                             _q931(octet4)
                             _q931(progressDescription)
                             LAST_TOKEN}, NULL, (INT32 *)progressDescription, NULL);

            if (nodeId>=0)
               *pi_IE=use31asPI_IE;
        }
    }
    if (notificationDescription)
    {
        *notificationDescription=RVERROR;
        __pvtGetByFieldIds(nodeId,hVal,q931NodeId,
                        {_q931(notificationIndicator)
                         _q931(octet3)
                         _q931(notificationDescription)
                         LAST_TOKEN}, NULL, (INT32 *)notificationDescription, NULL);
    }
    if (param)
    {
        __pvtGetNodeIdByFieldIds(nodeId,hVal,q931NodeId,
                                {_q931(userUser)
                                _q931(h323_UserInformation)
                                _q931(h323_uu_pdu)
                                _q931(nonStandardData)
                                LAST_TOKEN});
        if (nodeId>=0)
            getNonStandardParam(hVal,nodeId,param);
        else
            param->length = -1;
    }
    cmiAPIExit((HAPP)app,(char*)"cmCallProgressGet: [OK]");
    return TRUE;
}


/************************************************************************************
 *
 * cmCallNotifyCreate
 *
 * Purpose: To create NOTIFY message and fill it with the specified parameters
 *
 * Input:   hsCall                  - The stack handle to the call
 *          display                 - The display field
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if not used)
 *          param                   - The nonStandardParam to be put into the message
 *
 * Output:  None.
 *
 * Returned value: The root node ID of the created notify message on success
 *                 This root value should be passed to cmCallNotify() function.
 *                 Negative value on failure
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallNotifyCreate(
        IN HCALL                hsCall,
                                  IN char*          display,
                                  IN int            notificationDescription,
                                  IN cmNonStandardParam* param)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    int nodeId,rootId,ret=1,q931NodeId,nonStandardId;
    if (!hsCall || !app) return RVERROR;

    cmiAPIEnter((HAPP)app,(char*)"cmCallNotifyCreate: hsCall=0x%x, Display %s",hsCall,nprn(display));
    rootId=pvtAddRoot(app->hVal,app->synProtQ931,0,NULL);
    if (rootId<0)
    {
        cmiAPIExit((HAPP)app,(char*)"cmCallNotifyCreate: [%d]",rootId);
        return rootId;
    }
    if (emaLock((EMAElement)hsCall))
    {
        nodeId=callGetMessage(hsCall,cmQ931notify);

        ret =pvtSetTree(app->hVal,rootId,app->hVal,nodeId);
        if (ret<0)
        {
            emaUnlock((EMAElement)hsCall);
            cmiAPIExit((HAPP)app,(char*)"cmCallNotifyCreate: [%d]",ret);
            return ret;
        }
        __pvtBuildByFieldIds(q931NodeId,app->hVal,rootId,
                                {_q931(message) _q931(notify) LAST_TOKEN},0,NULL);

        if (notificationDescription>=0)
        {
            __pvtBuildByFieldIds(ret,app->hVal,q931NodeId,
                                    {_q931(notificationIndicator)
                                     _q931(octet3) _q931(notificationDescription) LAST_TOKEN},notificationDescription,NULL);
        }
        if (display!=NULL)
            pvtAdd(app->hVal,q931NodeId,__q931(display),(int)strlen(display),display,NULL);

        if (param!=NULL)
        {
            __pvtBuildByFieldIds(nonStandardId,app->hVal,q931NodeId,
                                {_q931(userUser)
                                 _q931(h323_UserInformation)
                                 _q931(h323_uu_pdu)
                                 _q931(nonStandardData)
                                 LAST_TOKEN},0,NULL);

            ret = setNonStandardParam(app->hVal,nonStandardId,param);
        }
        emaUnlock((EMAElement)hsCall);
    }

    if (ret >= 0)
        ret = rootId;

    cmiAPIExit((HAPP)app,(char*)"cmCallNotifyCreate: [%d]",ret);
    return ret;
}


/************************************************************************************
 *
 * cmCallNotifyGet
 *
 * Purpose: To decode NOTIFY message
 *
 * Input:   hApp                    - The stack instance handle
 *          message                 - Progress message to decode
 *          displaySize             - The display field size in bytes.
 *
 * Output:  display                 - The display field (user allocated memory)
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if absent)
 *          param                   - The nonStandardParam from the message (param->length==RVERROR if it wasn't there)
 *          ** Any of the output fields can be passed as NULL if the
 *             application doesn't care about their values
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallNotifyGet(
                           IN HAPP hApp,
                           IN  int              message,
                           OUT char*            display,
                           IN  int              displaySize,
                           OUT int*             notificationDescription,
                           OUT cmNonStandardParam* param)
{
    cmElem* app;
    int q931NodeId,nodeId;
    HPVT hVal;
    app = (cmElem* )hApp;
    if (!app)return RVERROR;
    hVal=app->hVal;

    cmiAPIEnter((HAPP)app,(char*)"cmCallNotifyGet: hApp=0x%x,nodeId=%d ",hApp,message);

    q931NodeId=pvtGetChild2(hVal,message,__q931(message),__q931(notify));
    if (q931NodeId < 0) return q931NodeId;
    if (display)
    {
        nodeId=pvtGetChild(hVal,q931NodeId,__q931(display),NULL);
        if (displaySize > 0)
            display[0] = 0;
        if (nodeId>=0)
        {
            int size;
            size=pvtGetString(app->hVal,nodeId,displaySize,display);
            if(size<displaySize) display[size]=0;
        }
    }
    if (notificationDescription)
    {
        *notificationDescription=RVERROR;
        __pvtGetByFieldIds(nodeId,hVal,q931NodeId,
                        {_q931(notificationIndicator)
                         _q931(octet3)
                         _q931(notificationDescription)
                         LAST_TOKEN}, NULL, (INT32 *)notificationDescription, NULL);
    }
    if (param)
    {
        __pvtGetNodeIdByFieldIds(nodeId,hVal,q931NodeId,
                                {_q931(userUser)
                                _q931(h323_UserInformation)
                                _q931(h323_uu_pdu)
                                _q931(nonStandardData)
                                LAST_TOKEN});
        if (nodeId>=0) 
            getNonStandardParam(hVal,nodeId,param);
        else
            param->length = -1;        
    }
    cmiAPIExit((HAPP)app,(char*)"cmCallNotifyGet: [OK]");
    return TRUE;
}


#ifdef __cplusplus
}
#endif
