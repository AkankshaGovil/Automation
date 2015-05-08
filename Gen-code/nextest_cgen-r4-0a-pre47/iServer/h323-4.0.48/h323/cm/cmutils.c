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

#include <cm.h>
#include <cmintr.h>
#include <cmutils.h>
#include <q931asn1.h>
#include <oidutils.h>
#include <h245.h>



typedef struct
{
    UINT32  bufferSize;
    BYTE*   buffer;
} cmThreadCoderLocalStorage;



int setNonStandard(
        IN  HPVT  hVal,
        IN  int   nodeId,
        IN  cmNonStandard*nonStandard
)
{
    int tmp,ret;
    tmp=nonStandard->t35CountryCode;
    if ((ret=pvtBuildByPath(hVal,nodeId,"t35CountryCode",tmp,NULL))<0)
    return ret;
    tmp=nonStandard->t35Extension;
    if ((ret=pvtBuildByPath(hVal,nodeId,"t35Extension",tmp,NULL))<0)
    return ret;
    tmp=nonStandard->manufacturerCode;
    if ((ret=pvtBuildByPath(hVal,nodeId,"manufacturerCode",tmp,NULL))<0)
    return ret;
    return 0;
}


int getNonStandard(
        IN  HPVT  hVal,
        IN  int   nodeId,
        OUT cmNonStandard*nonStandard
)
{
    INT32 tmp;
    pvtGetByPath(hVal,nodeId,"t35CountryCode",NULL,&tmp,NULL);
    nonStandard->t35CountryCode=(BYTE)tmp;
    pvtGetByPath(hVal,nodeId,"t35Extension",NULL,&tmp,NULL);
    nonStandard->t35Extension=(BYTE)tmp;
    pvtGetByPath(hVal,nodeId,"manufacturerCode",NULL,&tmp,NULL);
    nonStandard->manufacturerCode=(UINT16)tmp;
    return 0;
}

int setNonStandardParam(
        IN  HPVT  hVal,
        IN  int   vNodeId,
        IN  cmNonStandardParam*nonStandard
)
{
    if (!nonStandard)
        return RVERROR;
    return cmNonStandardParameterCreate(hVal,vNodeId,
                 &nonStandard->info,
                 nonStandard->data,
                 nonStandard->length);




}


int getNonStandardParam(
        IN  HPVT  hVal,
        IN  int   vNodeId,
        OUT cmNonStandardParam*nonStandard
)
{
    return cmNonStandardParameterGet(hVal,vNodeId,
                    &nonStandard->info,
                    nonStandard->data,
                    (INT32 *)&nonStandard->length);
}


INT32 getEnumNameId(
          IN    fieldNames* enumFieldNames,
          IN    int     enumFieldNamesSize,
          IN    unsigned    enumValue)
{
    return ((enumValue<enumFieldNamesSize/sizeof(fieldNames))?
        enumFieldNames[enumValue].nameId:LAST_TOKEN);
}

int getEnumValueByNameId(
            IN  fieldNames* enumFieldNames,
            IN  INT32       nameId)
{
    int i=0;
    while(enumFieldNames[i].nameId != LAST_TOKEN)
        if (enumFieldNames[i++].nameId==nameId) return i-1;
    return RVERROR;
}



int aliasToVt(IN  HPVT    hVal,
          IN  cmAlias*alias,
          IN  int     nodeId)
{
    int ret=0;
    switch(alias->type)
    {
    case   cmAliasTypeE164          :
    {   int iLen = alias->length;
        ret=pvtBuildByPath(hVal,nodeId,"e164",iLen,alias->string);
    }
    break;
    case   cmAliasTypeH323ID        :
    {   int iLen = alias->length;
        ret=pvtBuildByPath(hVal,nodeId,"h323-ID",iLen,alias->string);
    }
    break;
    case   cmAliasTypeEndpointID        :
    {   int iLen = alias->length;
        pvtSet(hVal,nodeId,-1,iLen,alias->string);
    }
    break;
    case   cmAliasTypeGatekeeperID      :
    {   int iLen = alias->length;
        pvtSet(hVal,nodeId,-1,iLen,alias->string);
    }
    break;
    case   cmAliasTypeURLID         :
    {   int iLen = alias->length;
        ret=pvtBuildByPath(hVal,nodeId,"url-ID",iLen,alias->string);
    }
    break;
    case   cmAliasTypeEMailID       :
    {   int iLen = alias->length;
        ret=pvtBuildByPath(hVal,nodeId,"email-ID",iLen,alias->string);
    }
    break;
    case   cmAliasTypeTransportAddress  :
        nodeId = pvtBuildByPath(hVal,nodeId,"transportID",0,NULL);
        if (nodeId<0)
            return nodeId;
        ret = cmTAToVt(hVal,nodeId, &(alias->transport));
        if (ret<0)
            return ret;
    break;
    case   cmAliasTypePartyNumber       :
    {
        int iLen;
        char *t=NULL,*v=NULL;
        int newNode=pvtBuildByPath(hVal,nodeId,"partyNumber",0,NULL);
        if (newNode<0)
        return newNode;
        switch (alias->pnType)
        {
        case cmPartyNumberPublicUnknown         :
            t=(char*)"publicNumber.publicTypeOfNumber.unknown";
            v=(char*)"publicNumber.publicNumberDigits";
        break;
            case cmPartyNumberPublicInternationalNumber :
            t=(char*)"publicNumber.publicTypeOfNumber.internationalNumber";
            v=(char*)"publicNumber.publicNumberDigits";
        break;
        case cmPartyNumberPublicNationalNumber      :
            t=(char*)"publicNumber.publicTypeOfNumber.nationalNumber";
            v=(char*)"publicNumber.publicNumberDigits";
        break;
        case cmPartyNumberPublicNetworkSpecificNumber   :
            t=(char*)"publicNumber.publicTypeOfNumber.networkSpecificNumber";
            v=(char*)"publicNumber.publicNumberDigits";
        break;
        case cmPartyNumberPublicSubscriberNumber    :
            t=(char*)"publicNumber.publicTypeOfNumber.subscriberNumber";
            v=(char*)"publicNumber.publicNumberDigits";
        break;
        case cmPartyNumberPublicAbbreviatedNumber   :
            t=(char*)"publicNumber.publicTypeOfNumber.abbreviatedNumber";
            v=(char*)"publicNumber.publicNumberDigits";
        break;
        case cmPartyNumberDataPartyNumber       :
            t=NULL;
            v=(char*)"dataPartyNumber";
        break;
        case cmPartyNumberTelexPartyNumber      :
            t=NULL;
            v=(char*)"telexPartyNumber";
        break;
        case cmPartyNumberPrivateUnknown        :
            t=(char*)"privateNumber.privateTypeOfNumber.unknown";
            v=(char*)"privateNumber.privateNumberDigits";
        break;
        case cmPartyNumberPrivateLevel2RegionalNumber   :
            t=(char*)"privateNumber.privateTypeOfNumber.level2RegionalNumber";
            v=(char*)"privateNumber.privateNumberDigits";
        break;
        case cmPartyNumberPrivateLevel1RegionalNumber   :
            t=(char*)"privateNumber.privateTypeOfNumber.level1RegionalNumber";
            v=(char*)"privateNumber.privateNumberDigits";
        break;
        case cmPartyNumberPrivatePISNSpecificNumber :
            t=(char*)"privateNumber.privateTypeOfNumber.pISNSpecificNumber";
            v=(char*)"privateNumber.privateNumberDigits";
        break;
        case cmPartyNumberPrivateLocalNumber        :
            t=(char*)"privateNumber.privateTypeOfNumber.localNumber";
            v=(char*)"privateNumber.privateNumberDigits";
        break;
        case cmPartyNumberPrivateAbbreviatedNumber  :
            t=(char*)"privateNumber.privateTypeOfNumber.abbreviatedNumber";
            v=(char*)"privateNumber.privateNumberDigits";
        break;
        case cmPartyNumberNationalStandardPartyNumber   :
            t=NULL;
            v=(char*)"nationalStandardPartyNumber";
        break;
        }
        if (t)
        {
            ret = pvtBuildByPath(hVal,newNode,t,0,NULL);
            if (ret<0)
                return ret;
        }
        iLen = alias->length;
        ret = pvtBuildByPath(hVal,newNode,v,iLen,alias->string);
    }
    break;
    }
    return ret;
}

typedef enum {
    cmNTPublicNumber,
    cmNTDataPartyNumber,
    cmNTTelexPartyNumber,
    cmNTPrivateNumber,
    cmNTNationalStandardPartyNumber
} cmNT;


int vtToAlias(IN  HPVT      hVal,
              IN  cmAlias*  alias,
              IN  int       nodeId)
{
    /* We can't use _q931() macros here since this function can be used for H450 ASN.1, which
       has different values for those fieldIDs. */
    int chNodeId;
    int tmpNodeId;
    chNodeId=pvtChild(hVal,nodeId);

    alias->type=(cmAliasType)(pvtGetSyntaxIndex(hVal,chNodeId)+cmAliasTypeE164-1);
    if (alias->type>=cmAliasTypeEndpointID) alias->type=(cmAliasType)(2 + alias->type);
    if (alias->type==cmAliasTypeTransportAddress)
    {
        alias->length = 0;
        cmVtToTA(hVal,chNodeId, &(alias->transport));
        return 0;
    }
    if (alias->type==cmAliasTypePartyNumber)
    {
        chNodeId=pvtChild(hVal,chNodeId);
        switch(pvtGetSyntaxIndex(hVal,chNodeId)-1)
        {
        case cmNTPublicNumber       :
            {
                /* ASN.1:
                    PublicPartyNumber ::= SEQUENCE
                    {
                        publicTypeOfNumber  PublicTypeOfNumber,
                        publicNumberDigits  NumberDigits
                    }
                */
                tmpNodeId = pvtChild(hVal, chNodeId); /* "publicTypeOfNumber" */
                chNodeId = pvtBrother(hVal, tmpNodeId); /* "publicNumberDigits" */
                tmpNodeId = pvtChild(hVal, tmpNodeId); /* "publicTypeOfNumber.*" */
                alias->pnType=(cmPartyNumberType)(pvtGetSyntaxIndex(hVal,tmpNodeId)+cmPartyNumberPublicUnknown-1);
            }
            break;
        case cmNTDataPartyNumber        :
            alias->pnType=cmPartyNumberDataPartyNumber;
            break;
        case cmNTTelexPartyNumber       :
            alias->pnType=cmPartyNumberTelexPartyNumber;
            break;
        case cmNTPrivateNumber      :
            {
                /* ASN.1:
                    PrivatePartyNumber ::= SEQUENCE
                    {
                        privateTypeOfNumber PrivateTypeOfNumber,
                        privateNumberDigits NumberDigits
                    }
                */
                tmpNodeId = pvtChild(hVal, chNodeId); /* "privateTypeOfNumber" */
                chNodeId = pvtBrother(hVal, tmpNodeId); /* privateNumberDigits */
                tmpNodeId = pvtChild(hVal, tmpNodeId); /* "privateTypeOfNumber.*" */
                alias->pnType=(cmPartyNumberType)(pvtGetSyntaxIndex(hVal,tmpNodeId)+cmPartyNumberPrivateUnknown-1);
            }
            break;
        case cmNTNationalStandardPartyNumber:
            alias->pnType=cmPartyNumberNationalStandardPartyNumber;
            break;
        }
    }

    if (chNodeId < 0)
        return chNodeId;

    {
        BOOL isString;
        INT32 length;
        pvtGet(hVal,chNodeId,NULL,NULL,&length,&isString);

        /* We don't always have the length of the alias, so we just take it from the
           string itself. This kind of thing isn't the best of choices, but it allows
           us to support existing applications that don't supply the length of their
           buffer and instead give (hopefully) a big enough buffer. */

        /* Added by NexTone */
        length = length < alias->length ? length : alias->length;

        alias->length=(UINT16)length;
        if (isString)
            pvtGetString(hVal,chNodeId,length,alias->string);

        /* For numbers we put a NULL termination. Other types don't get such treatment
           since we're not sure if the user gave us a buffer of 512 bytes or 513 bytes
           for it */
        if (alias->type==cmAliasTypeE164 || alias->type==cmAliasTypePartyNumber)
        {
            alias->string[length]=0;
            alias->length=(UINT16)strlen(alias->string);
        }
    }
    return 0;
}


RVAPI int RVCALLCONV cmAlias2Vt(IN  HPVT    hVal,
                                IN  cmAlias*alias,
                                IN  int     nodeId)
{
    return aliasToVt(hVal,alias,nodeId);
}

/************************************************************************
 * cmVt2Alias
 * purpose: Converts a PVT subtree of AliasAddress type into an Alias
 * input  : hVal    - The PVT handle
 *          nodeId  - PVT subtree of AliasAddress type we want to convert
 * output : alias   - Alias struct converted. The application is responsible
 *                    to supply the string field inside the cmAlias struct
 *                    with enough allocated size.
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV cmVt2Alias(
    IN  HPVT       hVal,
    OUT cmAlias*   alias,
    IN  int        nodeId)
{
    return vtToAlias(hVal,alias,nodeId);

}


int cmVtToTA(HPVT hVal,int nodeId, cmTransportAddress* ta)
{
    /* We can't use _q931() macros here since this function can be used for H450 ASN.1, which
       has different values for those fieldIDs. */
    UINT32 p;
    int vNodeId, addrNodeId;
    INT32 len;
    static const cmTransportType asn2type[]={
        cmTransportTypeIP,
        cmTransportTypeIPStrictRoute,
        (cmTransportType)-1,
        (cmTransportType)-1,
        (cmTransportType)-1,
        cmTransportTypeNSAP,
        (cmTransportType)-1};

    memset(ta,0,sizeof(cmTransportAddress));
    if (nodeId<0)
        return RVERROR;
    addrNodeId = pvtChild(hVal,nodeId);
    if (addrNodeId<0) return RVERROR;

    ta->type=asn2type[pvtGetSyntaxIndex(hVal,addrNodeId)-1];
    switch(ta->type)
    {
        case cmTransportTypeIP:
            /* ASN.1:
                ipAddress   SEQUENCE
                {
                    ip          OCTET STRING (SIZE(4)),
                    port            INTEGER(0..65535)
                },
            */
            vNodeId = pvtChild(hVal, addrNodeId); /* ip */
            if (pvtGet(hVal, vNodeId, NULL, NULL, &len, NULL) < 0)
                return RVERROR;
            if (len != sizeof(UINT32))
                ta->ip = 0;
            else
                pvtGetString(hVal, vNodeId, sizeof(UINT32), (char*)&(ta->ip));

            vNodeId = pvtBrother(hVal, vNodeId); /* port */
            if (pvtGet(hVal, vNodeId, NULL, NULL, (INT32*)&p, NULL) < 0)
                return RVERROR;
            ta->port = (UINT16)p;
            break;
        default:
            return RVERROR;
    }
    return 0;
}

int cmTAToVt(HPVT hVal,int nodeId, cmTransportAddress* ta)
{
    /* We can't use _q931() macros here since this function can be used for H450 ASN.1, which
       has different values for those fieldIDs. */
    int addrNodeId;
    int iPort;

    addrNodeId = pvtBuildByPath(hVal, nodeId, "ipAddress", 0, NULL);
    pvtBuildByPath(hVal, addrNodeId, "ip", sizeof(UINT32), (ta->ip)?((char*)&(ta->ip)):(char*)"\0\0\0");
    iPort = ta->port;
    return pvtBuildByPath(hVal, addrNodeId, "port", iPort, NULL);
}

int cmVtToTA_H245(HPVT hVal,int nodeId, cmTransportAddress* ta)
{
    UINT32 p;
    int vNodeId, addrNodeId;
    INT32 len;
    static const cmTransportType asn2typeU[]={
        cmTransportTypeIP,
        (cmTransportType)-1,
        (cmTransportType)-1,
        (cmTransportType)-1,
        cmTransportTypeIPStrictRoute,
        cmTransportTypeNSAP,
        (cmTransportType)-1};
    static const cmTransportType asn2typeM[]={
        cmTransportTypeIP,
        (cmTransportType)-1,
        cmTransportTypeNSAP,
        (cmTransportType)-1};

    memset(ta,0,sizeof(cmTransportAddress));
    if (nodeId<0)
        return RVERROR;
    addrNodeId=pvtChild(hVal,nodeId);

    ta->distribution=(cmDistribution)(pvtGetSyntaxIndex(hVal,addrNodeId)-1);
    addrNodeId=pvtChild(hVal,addrNodeId);
    ta->type=(ta->distribution==cmDistributionMulticast)?
        asn2typeM[pvtGetSyntaxIndex(hVal,addrNodeId)-1]:
        asn2typeU[pvtGetSyntaxIndex(hVal,addrNodeId)-1];
    switch(ta->type)
    {
        case cmTransportTypeIP:
            if (pvtGetChild(hVal, addrNodeId, __h245(network), &vNodeId)<0) return RVERROR;
            if (pvtGet(hVal, vNodeId,NULL,NULL,&len,NULL)<0) return RVERROR;
            if (len!=sizeof(UINT32)) ta->ip=0;
            else pvtGetString(hVal,vNodeId,sizeof(UINT32),(char*)&(ta->ip));

            if (pvtGetChild(hVal, addrNodeId, __h245(tsapIdentifier), &vNodeId)<0) return RVERROR;

            if (pvtGet(hVal, vNodeId,NULL,NULL,(INT32*)&p,NULL)<0) return RVERROR;
            ta->port=(UINT16)p;
        break;
        case cmTransportTypeNSAP:
            ta->length=(UINT16)pvtGetString(hVal,addrNodeId,20,(char*)ta->route);
        break;
        default:
        break;
    }
    return 0;
}

int cmTAToVt_H245(HPVT hVal,int nodeId, cmTransportAddress* ta)
{
    int addrNodeId;
    int iPort;
    addrNodeId= pvtAddBranch(hVal,nodeId,(ta->distribution)?__h245(multicastAddress):__h245(unicastAddress));
    addrNodeId= pvtAddBranch(hVal, addrNodeId, __h245(iPAddress));
    pvtAdd(hVal, addrNodeId, __h245(network),sizeof(UINT32),(ta->ip)?((char*)&(ta->ip)):(char*)"\0\0\0", NULL);
    iPort = ta->port;
    return pvtAdd(hVal, addrNodeId,__h245(tsapIdentifier),iPort,NULL,NULL);
}

/* non standard parameter________________________________________________________________________________ */




RVAPI int RVCALLCONV /* TRUE or RVERROR */
cmNonStandardParameterCreate(
                 /* Create NonStandardParameter tree */
                 IN  HPVT hVal,
                 IN  int rootId, /* nonStandardData */

                 IN  cmNonStandardIdentifier *identifier,
                 IN  char *data,
                 IN  int dataLength /* in bytes */
                 )
{
    int ret = -1;
    
    if (!hVal || rootId<0 || !data || dataLength<0 || !identifier) return RVERROR;
    
    if (identifier->objectLength >0) {
        char buff[128];
        int buffLen;
        
        strcpy(buff, "");
        
        buffLen = oidEncodeOID(sizeof(buff), buff, identifier->object);
        if (buffLen>0)
        {
            pvtBuildByPath(hVal, rootId, "data", dataLength, data);
            ret=pvtBuildByPath(hVal, rootId, "nonStandardIdentifier.object", buffLen, buff);
        }
    }
    else {
        int h221Id = pvtBuildByPath(hVal, rootId, "nonStandardIdentifier.h221NonStandard", 0, NULL);
        int iCountryCode = identifier->t35CountryCode;
        int iExtension = identifier->t35Extension;
        int iManufacturerCode = identifier->manufacturerCode;
        pvtBuildByPath(hVal, rootId, "data", dataLength, data);
        pvtBuildByPath(hVal, h221Id, "t35CountryCode", iCountryCode, NULL);
        pvtBuildByPath(hVal, h221Id, "t35Extension", iExtension, NULL);
        ret=pvtBuildByPath(hVal, h221Id, "manufacturerCode", iManufacturerCode, NULL);
    }
    
    return ret;
}


RVAPI int RVCALLCONV /* TRUE or RVERROR */
cmNonStandardParameterGet(
              /* Convert NonStandardParameter tree to C structures */
              IN  HPVT hVal,
              IN  int rootId, /* nonStandardData */

              OUT cmNonStandardIdentifier *identifier,
              OUT char *data,
              INOUT INT32 *dataLength /* (in bytes) IN: data allocation. OUT: correct size */
              )
{
    int id;
    INT32 len;
    
    if (!hVal ||
        !data || !dataLength || *dataLength<0 ||
        !identifier) return RVERROR;
    if (rootId<0)
    {
        *dataLength=RVERROR;
        return RVERROR;
    }
    
    id = pvtGetByPath(hVal, rootId, "data", NULL, &len, NULL);
    pvtGetString(hVal, id, *dataLength, data);
    *dataLength = len;
    
    /* -- object id */
    identifier->object[0]=0;
    identifier->objectLength=0;
    if ( (id = pvtGetByPath(hVal, rootId, "nonStandardIdentifier.object", NULL, NULL, NULL)) >0) {
        char buff[128];
        int buffLen;
        
        strcpy(buff, "");
        
        buffLen=pvtGetString(hVal, id, sizeof(buff), buff);
        oidDecodeOID(buffLen, buff, sizeof(identifier->object), identifier->object, numberForm);
        identifier->objectLength=strlen(identifier->object);
    }
    
    /* -- h221 id */
    identifier->t35CountryCode=0;
    identifier->t35Extension=0;
    identifier->manufacturerCode=0;
    if ( (id = pvtGetByPath(hVal, rootId, "nonStandardIdentifier.h221NonStandard", NULL, NULL, NULL)) >0) {
        INT32 value;
        
        pvtGetByPath(hVal, id, "t35CountryCode", NULL, &value, NULL);
        identifier->t35CountryCode=(UINT8)value;
        pvtGetByPath(hVal, id, "t35Extension", NULL, &value, NULL);
        identifier->t35Extension=(UINT8)value;
        pvtGetByPath(hVal, id, "manufacturerCode", NULL, &value, NULL);
        identifier->manufacturerCode=(UINT16)value;
    }
    
    return TRUE;
}




/********************************************************************************************
 * freeThreadBuffer
 * An exit callback of a specific thread encode/decode buffer.
 * This function frees the allocation of the encode/decode buffer
 * context  - The pointer to the buffer to free
 ********************************************************************************************/
static
void RVCALLCONV freeThreadBuffer(IN void*   context)
{
    cmThreadCoderLocalStorage* cmTls = (cmThreadCoderLocalStorage *)context;
    free(cmTls->buffer);
    cmTls->buffer = NULL;
}

/**************************************************************************************
 * getEncodeDecodeBuffer
 *
 * Purpose: To get the buffer for coding purposes.
 *
 * Input:   bufferSize   - The size of buffer required.
 *          buffer       - A pointer to the buffer.
 *          
 *
 * Output:  buffer       - A pointer to the buffer .
 *
 **************************************************************************************/
void getEncodeDecodeBuffer(IN int bufferSize, OUT BYTE **buffer)
{
    cmThreadCoderLocalStorage*  cmTls;
    RvH323ThreadHandle          threadId;
    BOOL                        anyExitFunc = FALSE;

    threadId = RvH323ThreadGetHandle();
    cmTls = (cmThreadCoderLocalStorage *)THREAD_GetLocalStorage(  threadId,
                                                                  tlsIntrCm,
                                                                  sizeof(cmThreadCoderLocalStorage));
    if (cmTls == NULL)
    {
        if (buffer)
            *buffer = NULL;
        return;
    }

    if ( (cmTls->buffer != NULL) &&
         ((int)cmTls->bufferSize < bufferSize))
    {
        /* Current allocation too small - make sure we fix this situation... */
        free(cmTls->buffer);
        cmTls->buffer = NULL;
        anyExitFunc = TRUE;
    }

    if (cmTls->buffer == NULL)
    {
        cmTls->bufferSize = bufferSize;
        cmTls->buffer = (BYTE *)malloc(cmTls->bufferSize);

        if (cmTls->buffer == NULL)
        {
            if (buffer)
                *buffer = NULL;
            return;
        }
        else if (!anyExitFunc)
            H323ThreadSetExitFunction(threadId, freeThreadBuffer, cmTls);
    }

    if (buffer)
        *buffer     = cmTls->buffer;
}

/**************************************************************************************
 * cleanMessage
 *
 * Purpose: Deletes all tunneling elements (H.245, H.450, annex M, annex L) from the
 *          given message. This is to avoid resending tunneled messages when reusing 
 *          messages from the cm Property tree.
 *
 * Input:   hPvt - handle to the PVT of the message
 *          message  - A pvt node to the message.
 *
 * Output:  None.
 *
 **************************************************************************************/
void cleanMessage(IN HPVT hPvt, IN int message)
{
    int messageBodyNode;
    int node;

    /* position on the UU-IE part of the message */
    __pvtGetNodeIdByFieldIds(   messageBodyNode,
                                hPvt,
                                message,
                                {   _q931(message)
                                    _anyField
                                    _q931(userUser)
                                    _q931(h323_UserInformation)
                                    _q931(h323_uu_pdu)
                                    LAST_TOKEN
                                });
    if (messageBodyNode < 0)
        return;

    /* Clean the parallel tunneling element, if exists */
    __pvtGetNodeIdByFieldIds( node,
                              hPvt,
                              messageBodyNode,
                              {   _q931(h323_message_body)
                                  _q931(setup)
                                  _q931(parallelH245Control)
                                  LAST_TOKEN
                               });
    if (node >= 0)
        pvtDelete(hPvt, node);

    /* Clean the H.245 tunneling element, if exists */
    __pvtGetNodeIdByFieldIds( node,
                              hPvt,
                              messageBodyNode,
                              {   _q931(h245Control)
                                  LAST_TOKEN
                              });
    if (node >= 0)
        pvtDelete(hPvt, node);

    /* Clean the H.450 element, if exists */
    __pvtGetNodeIdByFieldIds( node,
                              hPvt,
                              messageBodyNode,
                                {   _q931(h4501SupplementaryService) 
                                    LAST_TOKEN
                                });
    if (node >= 0)
        pvtDelete(hPvt, node);

    /* Clean the Annex L element, if exists */
    __pvtGetNodeIdByFieldIds( node,
                              hPvt,
                              messageBodyNode,
                                {   _q931(stimulusControl)
                                    LAST_TOKEN
                                });
    if (node >= 0)
        pvtDelete(hPvt, node);

    /* Clean the annex M element, if exists */
    __pvtGetNodeIdByFieldIds( node,
                              hPvt,
                              messageBodyNode,
                                {   _q931(tunnelledSignallingMessage)
                                    LAST_TOKEN
                                });
    if (node >= 0)
        pvtDelete(hPvt, node);

}


#ifdef __cplusplus
}
#endif



