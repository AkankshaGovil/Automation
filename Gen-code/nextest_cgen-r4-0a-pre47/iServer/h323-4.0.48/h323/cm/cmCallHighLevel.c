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
#include <ctype.h>
#include <cm.h>
#include <stkutils.h>
#include <cmdebprn.h>
#include <li.h>
#include <q931asn1.h>
#include <cmAutoRasCall.h>
#include <cmCall.h>
#include <strutils.h>


/************************************************************************
 * insertAlias
 * purpose: Convert a string into an alias struct
 * input  : numPtr  - Pointer to the string to convert
 * output : alias   - Alias representaiton of the string
 *          bmpStr  - String value of the alias
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int insertAlias(IN char **numPtr, OUT cmAlias * alias, OUT char* bmpStr)
{
    char *ptr;

    if (!strncmp("TEL:",*numPtr,4))
    {
        /* e164 alias type */
        *numPtr+=4;
        alias->string=*numPtr;
        alias->length=(UINT16)strlen(alias->string);
        alias->type=cmAliasTypeE164;

    }
    else if (!strncmp("URL:",*numPtr,4))
    {
        /* URL */
        *numPtr+=4;
        alias->string=*numPtr;
        alias->length=(UINT16)strlen(alias->string);
        alias->type=cmAliasTypeURLID;

    }
    else if (!strncmp("EMAIL:",*numPtr,6))
    {
        /* eMail */
        *numPtr+=6;
        alias->string=*numPtr;
        alias->length=(UINT16)strlen(alias->string);
        alias->type=cmAliasTypeEMailID;

    }
    else if (!strncmp("PN:",*numPtr,3))
    {
        /* partyNumber */
        *numPtr+=3;
        alias->string=*numPtr;
        alias->type=cmAliasTypePartyNumber;
        alias->pnType=(cmPartyNumberType )cmPartyNumberPublicUnknown;
        if ((ptr=strchr(*numPtr,'$'))!=NULL)
        {
            switch(ptr[1])
            {
            case 'P':
                switch(ptr[2])
                {
                case 'U':
                    switch(ptr[3])
                    {
                    case 'U':alias->pnType=cmPartyNumberPublicUnknown;break;
                    case 'I':alias->pnType=cmPartyNumberPublicInternationalNumber;break;
                    case 'N':
                        switch(ptr[4])
                        {
                        case   0:alias->pnType=cmPartyNumberPublicNationalNumber;break;
                        case 'S':alias->pnType=cmPartyNumberPublicNetworkSpecificNumber;break;
                        }
                        break;
                    case 'S':alias->pnType=cmPartyNumberPublicSubscriberNumber;break;
                    case 'A':alias->pnType=cmPartyNumberPublicAbbreviatedNumber;break;
                    }
                    break;
                case 'R':
                    switch(ptr[3])
                    {
                    case 'U':alias->pnType=cmPartyNumberPrivateUnknown;break;
                    case 'L':
                        switch(ptr[4])
                        {
                        case '2':alias->pnType=cmPartyNumberPrivateLevel2RegionalNumber;break;
                        case '1':alias->pnType=cmPartyNumberPrivateLevel1RegionalNumber;break;
                        case   0:alias->pnType=cmPartyNumberPrivateLocalNumber;break;
                        }
                        break;
                    case 'P':alias->pnType=cmPartyNumberPrivatePISNSpecificNumber;break;
                    case 'A':alias->pnType=cmPartyNumberPrivateAbbreviatedNumber;break;
                    }
                    break;
                }
                break;
            case 'D':alias->pnType=cmPartyNumberDataPartyNumber;break;
            case 'T':alias->pnType=cmPartyNumberTelexPartyNumber;break;
            case 'N':alias->pnType=cmPartyNumberNationalStandardPartyNumber;break;
            }
            if (((int)alias->pnType) < 0)
                alias->pnType=(cmPartyNumberType )atoi(ptr+1);
            *ptr=0;
        }
        alias->length=(UINT16)strlen(alias->string);
    }
    else if (!strncmp("TNAME:",*numPtr,6))
    {
        /* transport address */
        *numPtr+=6;
        {
            char* ipPtr;
            alias->type=cmAliasTypeTransportAddress;
            alias->transport.type = cmTransportTypeIP;
            alias->transport.ip = 0;
            alias->transport.port = 1720;

            ipPtr=*numPtr;
            if ((ptr=strchr(*numPtr,':'))!=NULL)
            {

                alias->transport.port=(UINT16)atoi(ptr+1);
                *ptr=0;
            }
            alias->transport.ip=liConvertIp(ipPtr);
        }
        alias->string=NULL;
        alias->length=0;
    }
    else if (!strncmp("NAME:",*numPtr,5))
    {
        /* h323ID */
        *numPtr+=5;
        alias->type=cmAliasTypeH323ID;
        alias->length=(UINT16)utlChr2Bmp(*numPtr, (BYTE*)bmpStr);
        alias->string=bmpStr;
    }
    else return RVERROR;

    return 0;
}



/************************************************************************
 * cmCallMake
 * purpose: Starts a new call setup with the given parameters
 * input  : hsCall      - Stack handle for the new call
 *          maxRate     - Maximum rate allowed for the new call
 *          minRate     - Minimum rate allowed for the new call.
 *                        This parameter is not currently in use, so set it to zero (0).
 *          destAddress - Called party address list
 *          srcAddress  - Calling party address list
 *          display     - String representing display information for reporting
 *                        to the called party
 *          userUser    - String representing user-to-user information for reporting
 *                        to the called party
 *          userUserSize- Length of the string representing user-to-user information
 *                        to report to the called party
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallMake(
        IN  HCALL       hsCall,
        IN  UINT32      maxRate,
        IN  UINT32      minRate,
        IN  char*       destAddress,
        IN  char*       srcAddress,
        IN  char*       display,
        IN  char*       userUser,
        IN  int         userUserSize)
{
    char *ptr;
    BOOL fta,eta,wasTA=FALSE,wasETA=FALSE,firstETA=FALSE;
    HAPP hApp;
    char bmpStr[512];
    char destA[512];
    char srcA[512];
    char* numPtr=NULL;
    char* nextPtr=NULL;
    int id=0,i, ret;
    BOOL namedID=TRUE;
    char delimiter;

    hApp=cmGetAHandle((HPROTOCOL)hsCall);

    if (!hApp || !hsCall) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallMake(hsCall=0x%lx,maxRate=%ld,minRate=%ld,destAddr=%.100s,srcAddr=%.100s,display=%.100s,userUser=%.100s,userUserSize=%d)",
        hsCall,maxRate,minRate,nprn(destAddress),nprn(srcAddress),nprn(display),nprn(userUser),userUserSize);

    delimiter=cmGetDelimiter(hApp);

    /* Set the source addresses */
    if (srcAddress && srcAddress[0])
    {
        strncpy(srcA,srcAddress,sizeof(srcA));
        numPtr=srcA;

        while(numPtr)
        {
            int iDelimiter = delimiter;
            nextPtr=strchr(numPtr,iDelimiter);
            {
                cmAlias alias;
                while(isspace((int)*numPtr)) numPtr++;
                if (nextPtr)
                {
                    *nextPtr=0;
                    nextPtr++;
                }
                if (insertAlias(&numPtr,&alias, bmpStr) >= 0)
                    cmCallSetParam(hsCall,cmParamSourceAddress,id++,0,(char *)&alias);
                else
                {
                    /* No specific type - take as e164 */
                    alias.string=numPtr;
                    alias.length=(UINT16)strlen(alias.string);
                    alias.type=cmAliasTypeE164;

                    cmCallSetParam(hsCall,cmParamCallingPartyNumber,0,0,(char *)&alias);
                    break;
                }
            }
            numPtr=nextPtr;
        }
    }

    /* Set destination address parameters */
    if (destAddress && destAddress[0])
    {
        strncpy(destA,destAddress,sizeof(destA));
        numPtr=destA;
        id=0;
        while(numPtr&&namedID)
        {
            int iDelimiter = delimiter;
            namedID=TRUE;
            fta=0;
            nextPtr=strchr(numPtr,iDelimiter);
            {

                cmAlias alias;
                while(isspace((int)*numPtr)) numPtr++;
                if (nextPtr)
                {
                    *nextPtr=0;
                    nextPtr++;
                }
                if (insertAlias(&numPtr,&alias,bmpStr) >= 0)
                    cmCallSetParam(hsCall,cmParamDestinationAddress,id++,0,(char *)&alias);
                else if (!strncmp("SUB:",numPtr,4))
                {
                    numPtr+=4;
                    alias.string=numPtr;
                    alias.length=(UINT16)strlen(alias.string);
                    alias.type=cmAliasTypeE164;

                    cmCallSetParam(hsCall,cmParamCalledPartySubAddress,0,0,(char *)&alias);
                }
                else if (!strncmp("EXT:",numPtr,4))
                {
                    numPtr+=4;
                    alias.string=numPtr;
                    alias.length=(UINT16)strlen(alias.string);
                    alias.type=cmAliasTypeE164;

                    cmCallSetParam(hsCall,cmParamExtention,0,0,(char *)&alias);
                }
                else if (!strncmp("EXTID:",numPtr,6))
                {
                    numPtr+=6;
                    alias.type=cmAliasTypeH323ID;
                    alias.length=(UINT16)utlChr2Bmp(numPtr, (BYTE*)bmpStr);
                    alias.string=bmpStr;

                    cmCallSetParam(hsCall,cmParamExtention,0,0,(char *)&alias);
                }
                else if (!strncmp("TA:",numPtr,3) || !strncmp("ETA:",numPtr,3) || !strncmp("FTA:",numPtr,3))
                {
                    char* ipPtr;
                    int port=-1;
                    cmTransportAddress qAddress;
                    fta=(strncmp("FTA:",numPtr,4)==0);
                    eta=(strncmp("ETA:",numPtr,4)==0);

                    numPtr+=3+(fta||eta);


                    qAddress.type = cmTransportTypeIP;
                    qAddress.ip = 0;

                    ipPtr=numPtr;
                    if ((ptr=strchr(numPtr,':'))!=NULL)
                    {
                        port=(UINT16)atoi(ptr+1);
                        *ptr=0;
                    }
                    qAddress.ip=liConvertIp(ipPtr);
                    qAddress.port=(UINT16)port;

                    if (eta && !wasTA)      firstETA=TRUE;
                    if (eta)                wasETA=TRUE;
                    if (!eta && !fta)       wasTA=TRUE;


                    if (!fta)
                    {

                        if (port<0 && eta)
                            qAddress.port = 2517;
                        else
                        if (port<0)
                            qAddress.port = 1720;

                        if (!eta)
                            cmCallSetParam((HCALL)hsCall,cmParamDestinationIpAddress,0, sizeof(cmTransportAddress),(char*)&qAddress);
                        else
                            cmCallSetParam((HCALL)hsCall,cmParamDestinationAnnexEAddress,0, sizeof(cmTransportAddress),(char*)&qAddress);
                    }
                    if (port<0)
                        qAddress.port = 1720;

                    if (!eta)
                        cmCallSetParam((HCALL)hsCall,cmParamDestCallSignalAddress,0, sizeof(cmTransportAddress),(char*)&qAddress);
                }
                else
                {
                    namedID=FALSE;
                }
            }

            if (namedID)
                numPtr=nextPtr;
        }

        /* Deal with extra call info (used for calls that need 2 numbers) */
        i=-1;
        while(numPtr)
        {
            cmAlias alias;

            alias.string=numPtr;
            numPtr=strchr(numPtr,';');
            if (numPtr)
            {
                *numPtr=0;
                numPtr++;
            }
            alias.type=cmAliasTypeE164;
            alias.length=(UINT16)strlen(alias.string);
            cmCallSetParam((HCALL)hsCall,(i<0)?cmParamCalledPartyNumber:cmParamDestExtraCallInfo,(i<0)?0:i,
                sizeof(cmAlias),(char*)&alias);
            i++;
        }
    }

    /*Set annexE type*/

    {
        cmAnnexEUsageMode annexE=cmTransNoAnnexE;
        if (wasTA && wasETA && firstETA)  annexE=cmTransPreferedAnnexE;
        else
        if (wasTA && wasETA && !firstETA) annexE=cmTransPreferedTPKT;
        else
        if (wasETA && !wasTA) annexE=cmTransUseAnnexE;

        cmCallSetParam((HCALL)hsCall,cmParamAnnexE,0,annexE,NULL);
    }


    /* Set other parameters - rate, display, userUser */
    cmCallSetParam(hsCall,cmParamRequestedRate,0,(INT32)maxRate,0);
    if (display)
        cmCallSetParam(hsCall,cmParamDisplay,0,0,display);
    if (userUser)
        cmCallSetParam(hsCall,cmParamUserUser,0,userUserSize,userUser);

    /* Make sure we dial */
    ret = cmCallDial(hsCall);

    cmiAPIExit(hApp,(char*)"cmCallMake=%d", ret);
    return ret;
}

/*-----------------------------------------------------------------------------------------
 * alias2str
 *
 * convert from cmAlias to string with the alias (with the appropriate prefix of the alias)
 *
 * Input:  alias - alias struct.
 *         len   - length of str (how much space was allocated).
 * Output: str   - string with alias. must be allocated before calling the function.
 *
 * Return: the length of the string that was copied to str.
 *-----------------------------------------------------------------------------------------*/
int alias2str(
    IN  cmAlias alias,
    OUT char * str,
    IN  int len)
{
    switch(alias.type)
    {
    case(cmAliasTypeE164):
        {
            if (strncmp("EXT:",str,4))
            {
                if(len>(alias.length+4))
                {
                    sprintf(str,"TEL:%s",alias.string);
                    return 4+alias.length;
                }
            }
            else    /* if str begins with "EXT:", it is e164 because of EXT and not TEL, */
                /* then don't write the "TEL:", only the name and return only length */
            {
                if(len>alias.length)
                    sprintf(str,"%s",alias.string);
                return alias.length;
            }
            return 0;
        }
    case(cmAliasTypeH323ID):
        {
            char nameId[257];
            int res = bmp2chr(nameId,(BYTE* )alias.string,alias.length);
            if ((res >= 0) && (len > ((alias.length/2)+5)))
            {
                sprintf(str,"NAME:%s",nameId);
                return 5+(alias.length/2);
            }
            return 0;
        }
    case(cmAliasTypeURLID):
        {
            if(len>(alias.length+4))
            {
                sprintf(str,"URL:%s",alias.string);
                return 4+alias.length;
            }
            return 0;
        }
    case(cmAliasTypeEMailID):
        {
            if(len>(alias.length+6))
            {
                sprintf(str,"EMAIL:%s",alias.string);
                return 6+alias.length;
            }
            return 0;
        }
    case(cmAliasTypeTransportAddress):
        {
            if (len>25)
            {
                unsigned char * ip = (unsigned char *) &alias.transport.ip;
                int length=sprintf(str,"TA:%u.%u.%u.%u:%u",
                    (unsigned)ip[0],
                    (unsigned)ip[1],
                    (unsigned)ip[2],
                    (unsigned)ip[3],
                    alias.transport.port);
                return length;
            }
            return 0;
        }
    case(cmAliasTypePartyNumber):
        {
            if(len>(alias.length+7))
            {
                static char pnType[15][6] = { "PUU", "PUI", "PUN", "PUNS", "PUS", "PUA", "D", "T", "PRU", "PRL2", "PRL1", "PRP", "PRL", "PRA", "N" };
                sprintf(str,"PN:%s$%s",alias.string,pnType[alias.pnType]);
                return 6+alias.length;
            }
            return 0;
        }
    case(cmAliasTypeEndpointID):
    case(cmAliasTypeGatekeeperID):
        return RVERROR;
    }
    return 0;
}


RVAPI
int RVCALLCONV cmCallGetCallingPartyId(
                                     IN     HCALL       hsCall,
                                     OUT    char*       partyId,
                                     IN      int        len)
{
    cmTransportAddress tr;
    cmAlias alias;
    char number[513];
    HAPP hApp;
    int i;
    char ipBuf[18];
    char delimiter[2];
    int error=RVERROR;
    if (!hsCall) return RVERROR;

    strcpy(delimiter, ",");
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallGetCallingPartyId(hsCall=0x%lx,partyId,len=%d)",hsCall,len);
    alias.string=number;
    delimiter[0]=cmGetDelimiter(hApp);

    if (len>=25)
    {
        partyId[0]=0;
        if (cmCallGetParam(hsCall,cmParamSrcCallSignalAddress,0,0,(char*)&tr)>=0)
            sprintf(partyId,"TA:%s:%d", liIpToString(tr.ip, ipBuf), tr.port);

        i=0;
        while (cmCallGetParam(hsCall,cmParamSourceAddress,i++,0,(char *)&alias)>=0)
        {
            int length = strlen(partyId);
            if ((length) && ((len-length) > 1)) strcat(partyId,delimiter);
            alias.string[alias.length] = 0;
            if(alias2str(alias, &partyId[length+1], len-length-1)<=0)
            {
                /* remove the delimiter */
                partyId[length] = 0;
            }
        }
        if (cmCallGetParam(hsCall,cmParamCallingPartySubAddress,0,0,(char *)&alias)>=0)
        {
            int length = strlen(partyId);
            if ((len-length-5-strlen(number)) > 0)
            {
                if (length) strcat(partyId,delimiter);
                strcat(partyId,"SUB:");
                strcat(partyId,number);
            }
        }
        if (cmCallGetParam(hsCall,cmParamCallingPartyNumber,0,0,(char *)&alias)>=0)
        {
            int length = strlen(partyId);
            if ((len-length-1-strlen(number)) > 0)
            {
                if (length) strcat(partyId,delimiter);
                strcat(partyId,number);
            }
        }
        error=0;
    }
    cmiAPIExit(hApp,(char*)"cmCallGetCallingPartyId(partyId=%.100s)=%d",nprn(partyId),error);
    return error;
}


RVAPI
int RVCALLCONV cmCallGetCalledPartyId(
                                    IN      HCALL       hsCall,
                                    OUT     char*       partyId,
                                    IN      int         len)
{
    cmTransportAddress tr;
    cmAlias alias;
    char number[513];
    int i;
    char ipBuf[18];
    char delimiter[2];
    int error=RVERROR;
    HAPP hApp;

    strcpy(delimiter, ",");
    if (!hsCall) return RVERROR;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;
    cmiAPIEnter(hApp,(char*)"cmCallGetCalledPartyId(hsCall=0x%lx,partyId,len=%d)",hsCall,len);
    alias.string=number;
    delimiter[0]=cmGetDelimiter(hApp);

    if (len>=25)
    {
        partyId[0]=0;
        if (cmCallGetParam(hsCall,cmParamDestCallSignalAddress,0,0,(char*)&tr)>=0)
            sprintf(partyId,"TA:%s:%d", liIpToString(tr.ip, ipBuf), tr.port);

        i=0;
        while(cmCallGetParam(hsCall,cmParamDestinationAddress,i++,0,(char *)&alias)>=0)
        {
            int length = strlen(partyId);
            BOOL addedDelimiter = FALSE;
            if ((length) && ((len-length) > 1))
            {
                strcat(partyId,delimiter);
                length++;
                addedDelimiter = TRUE;
            }
            alias.string[alias.length] = 0;
            if(alias2str(alias, &partyId[length], len-length-1) <= 0)
            {
                /* remove the delimiter */
                if(addedDelimiter)
                    partyId[length-1] = 0;
            }
        }
        if (cmCallGetParam(hsCall,cmParamCalledPartySubAddress,0,0,(char *)&alias)>=0)
        {
            int length = strlen(partyId);
            if ((len-length-5-strlen(number)) > 0)
            {
                if (length) strcat(partyId,delimiter);
                strcat(partyId,"SUB:");
                strcat(partyId,number);
            }
        }
        if (cmCallGetParam(hsCall,cmParamExtention,0,0,(char *)&alias)>=0)
        {
            int length = strlen(partyId);
            
            if (alias.type==cmAliasTypeH323ID)
            {
                char str[257];
                int iLength = alias.length;
                utlBmp2Chr(str,(BYTE*)number,iLength);
                strncpy(number, str, 256);
                number[256] = 0;
            }
            
            if ((len-length-1-((alias.type==cmAliasTypeE164)?4:6)-strlen(number)) > 0)
            {
                if (length) strcat(partyId,delimiter);
                strcat(partyId,(alias.type==cmAliasTypeE164)?"EXT:":"EXTID:");
                strcat(partyId,number);
            }
        }
        if (cmCallGetParam(hsCall,cmParamCalledPartyNumber,0,0,(char *)&alias)>=0)
        {
            int length = strlen(partyId);
            if ((len-length-1-strlen(number)) > 0)
            {
                if (length) strcat(partyId,delimiter);
                strcat(partyId,number);
            }
            i=0;
            while (cmCallGetParam(hsCall,cmParamDestExtraCallInfo,i++,0,(char *)&alias)>=0)
            {
                length = strlen(partyId);
                if ((len-length-1-strlen(number)) > 0)
                {
                    strcat(partyId,";");
                    strcat(partyId,number);
                }
            }
        }
        error=0;
    }
    cmiAPIExit(hApp,(char*)"cmCallGetCalledPartyId(callerId=%.100s)=%d",nprn(partyId),error);
    return error;
}

RVAPI
int RVCALLCONV cmCallSetIndependentSupplementaryService(
                                                      IN      HCALL               hsCall
                                                      )
{
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallIndependentSupplementaryService(hsCall=0x%lx)",hsCall);
    if (cmCallGetOrigin(hsCall,NULL))
    {
        cmCallSetParam(hsCall,cmParamMultiRate,0,0,0);
        cmCallSetParam(hsCall,cmParamConferenceGoal,0,cmCallIndependentSupplementaryService,0);
        cmCallSetParam(hsCall,cmParamInformationTransferCapability,0,cmITCUnrestricted,0);
        cmCallSetParam(hsCall,cmParamRequestedRate,0,0,0);
    }
    cmiAPIExit(hApp,(char*)"cmCallIndependentSupplementaryService=0");
    return 0;
}

RVAPI
int RVCALLCONV cmCallAnswerExt(
                             IN     HCALL       hsCall,
                             IN     char*       display,
                             IN     char*       userUser,
                             IN     int         userUserSize)
{
    int status = RVERROR;
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallAnswerExt(hsCall=0x%lx,display=%.100s,userUser=,userUserSize=%d)",hsCall,nprn(display),userUserSize);

    if (display)
        cmCallSetParam((HCALL)hsCall,cmParamConnectDisplay,0,0,display);
    if (userUser)
        cmCallSetParam((HCALL)hsCall,cmParamConnectUserUser,0,userUserSize,userUser);
    status = cmCallAnswer(hsCall);
    cmiAPIExit(hApp,(char*)"cmCallAnswerExt=%d",status);
    return status;
}

RVAPI
int RVCALLCONV cmCallAnswerDisplay(
                                 IN     HCALL       hsCall,
                                 IN     char*       display)
{
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallAnswerDisplay(hsCall=0x%lx,display=%.100s)",hsCall,nprn(display));
    cmCallAnswerExt(hsCall,display,NULL,0);
    cmiAPIExit(hApp,(char*)"cmCallAnswerDisplay=%d",0);
    return 0;
}


RVAPI
int RVCALLCONV cmCallGetDisplayInfo(
                                  IN    HCALL       hsCall,
                                  OUT   char*       display,
                                  IN    int         displaySize)
{
    INT32 size=displaySize-1;
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallGetDisplayInfo(hsCall=0x%lx,display,displaySize=%d)",hsCall,displaySize);
    if (display && displaySize>0)
    {
        if (size)
            display[0]=0;
        cmCallGetParam((HCALL)hsCall,(cmCallGetOrigin(hsCall,NULL)?cmParamConnectDisplay:cmParamDisplay),0,(INT32*)&size,display);
        if (size<displaySize)
            display[size]=0;
    }
    cmiAPIExit(hApp,(char*)"cmCallGetDisplayInfo(%.100s)=%d",nprn(display),size);
    return size;
}

RVAPI
int RVCALLCONV cmCallGetUserUserInfo(
                                   IN   HCALL       hsCall,
                                   OUT  char*       userUser,
                                   IN   int         userUserSize)
{
    UINT32 size=userUserSize-1;
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallGetUserUserInfo(hsCall=0x%lx,userUser,userUserSize=%d)",hsCall,userUserSize);
    if (userUser)
    {
        cmCallGetParam((HCALL)hsCall,(cmCallGetOrigin(hsCall,NULL)?cmParamConnectUserUser:cmParamUserUser),0,(INT32*)&size,userUser);
    }
    cmiAPIExit(hApp,(char*)"cmCallGetUserUserInfo(%.16s)=%d",nprn(userUser),size);
    return size;
}


RVAPI
int RVCALLCONV cmCallGetRemoteEndpointInfoHandle(
                                               IN   HCALL       hsCall)
{

    INT32 endpointInfo;
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallGetRemoteEndpointInfoHandle(hsCall=0x%lx)",hsCall);
    cmCallGetParam((HCALL)hsCall,(cmCallGetOrigin(hsCall,NULL)?cmParamFullDestinationInfo:cmParamFullSourceInfo),0,&endpointInfo,NULL);
    cmiAPIExit(hApp,(char*)"cmCallGetRemoteEndpointInfoHandle=%d",endpointInfo);
    return endpointInfo;
}

RVAPI
int RVCALLCONV cmCallGetLocalEndpointInfoHandle(
                                              IN    HCALL       hsCall)
{
    INT32 endpointInfo;
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallGetLocalEndpointInfoHandle(hsCall=0x%lx)",hsCall);
    cmCallGetParam((HCALL)hsCall,(cmCallGetOrigin(hsCall,NULL)?cmParamFullSourceInfo:cmParamFullDestinationInfo),0,&endpointInfo,NULL);
    cmiAPIExit(hApp,(char*)"cmCallGetLocalEndpointInfoHandle=%d",endpointInfo);
    return endpointInfo;
}

RVAPI
int RVCALLCONV cmGetEndpointInfoHandle(
                                     IN     HAPP        hApp)
{
    int endpointInfo;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmGetEndpointInfoHandle(hApp=0x%lx)",hApp);
    __pvtGetNodeIdByFieldIds(endpointInfo,cmGetValTree(hApp),cmGetRASConfigurationHandle(hApp),
                            {_q931(registrationInfo)
                             _q931(terminalType)
                             LAST_TOKEN });
    cmiAPIExit(hApp,(char*)"cmGetEndpointInfoHandle=%d",endpointInfo);
    return endpointInfo;
}

RVAPI
int RVCALLCONV cmCallGetRate(
                           IN   HCALL       hsCall,
                           OUT  UINT32*     rate)
{
    HAPP hApp;
    INT32 _rate;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallGetRate(hsCall=0x%lx,rate)",hsCall);
    cmCallGetParam(hsCall,cmParamRate,0,&_rate,NULL);
    cmiAPIExit(hApp,(char*)"cmCallGetRate(rate=%ld)=%d",_rate,0);
    if (rate) *rate=_rate;
    return _rate;
}

RVAPI
int RVCALLCONV cmCallSetRate(
                           IN   HCALL       hsCall,
                           IN   UINT32      rate)
{
    HAPP hApp;
    hApp=cmGetAHandle((HPROTOCOL)hsCall);
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallSetRate(hsCall=0x%lx,rate=%ld)",hsCall,rate);
    cmCallSetParam(hsCall,cmParamRequestedRate,0,(INT32)rate,NULL);
    if (cmiAutoRAS(hApp))
        cmiAutoRASCallSetRate(hsCall,(int)rate);
    else
        if (!((cmElem *)hApp)->gatekeeper)
        {
            /* We're not a GK, and there's no automatic ras - notify application about it */
            cmCallSetParam(hsCall,cmParamRate,0,(INT32)rate,NULL);
            callNewRate((callElem *)hsCall);
        }
    cmiAPIExit(hApp,(char*)"cmCallSetRate=%d",0);
    return 0;
}



RVAPI
int RVCALLCONV cmCallOverlapSending(IN HCALL hsCall,char * address)
{
    return cmCallOverlapSendingExt(hsCall,address,FALSE);
}

RVAPI
int RVCALLCONV cmCallForward(
                           IN   HCALL       hsCall,
                           IN   char*       destAddress)
{
    char destA[512];
    char bmpStr[512];
    char* numPtr=NULL;
    int id=0;
    int i;
    BOOL namedID=TRUE;
    char* nextPtr=NULL;
    char *ptr;
    HAPP hApp=cmGetAHandle((HPROTOCOL)hsCall);
    char delimiter;

    if (!hApp || !hsCall) return RVERROR;
    cmiAPIEnter(hApp,(char*)"cmCallForward(hsCall=0x%lx,destAddr=%.100s)",hsCall,nprn(destAddress));

    delimiter=cmGetDelimiter(hApp);
    strncpy(destA,destAddress,sizeof(destA));
    numPtr=destA;
    id=0;
    while(numPtr&&namedID)
    {
        int iDelimiter = delimiter;
        namedID=FALSE;
        nextPtr=strchr(numPtr,iDelimiter);
        {
            cmAlias alias;
            while(isspace((int)*numPtr)) numPtr++;
            if (nextPtr)
            {
                *nextPtr=0;
                nextPtr++;
            }

            if (insertAlias(&numPtr,&alias,bmpStr) >= 0)
            {
                namedID=TRUE;
                cmCallSetParam(hsCall,cmParamAlternativeAliasAddress,id++,0,(char *)&alias);
            }
            else    if (!strncmp("EXT:",numPtr,4))
                    {
                        namedID=TRUE;
                        numPtr+=4;
                        alias.string=numPtr;
                        alias.length=(UINT16)strlen(alias.string);
                        alias.type=cmAliasTypeE164;

                        cmCallSetParam(hsCall,cmParamAlternativeExtention,0,0,(char *)&alias);
                    }
                    else
                        if (!strncmp("EXTID:",numPtr,6))
                        {
                            namedID=TRUE;
                            numPtr+=6;
                            alias.type=cmAliasTypeH323ID;
                            alias.length=(UINT16)utlChr2Bmp(numPtr, (BYTE*)bmpStr);
                            alias.string=bmpStr;
                            cmCallSetParam(hsCall,cmParamAlternativeExtention,0,0,(char *)&alias);
                        }
                        else
                            if (strlen(numPtr)==strspn(numPtr,"0123456789,#*;"))
                            {
                                i=-1;
                                while(numPtr)
                                {
                                    alias.string=numPtr;
                                    numPtr=strchr(numPtr,';');
                                    if (numPtr)
                                    {
                                        *numPtr=0;
                                        numPtr++;
                                    }
                                    alias.type=cmAliasTypeE164;
                                    alias.length=(UINT16)strlen(alias.string);
                                    cmCallSetParam((HCALL)hsCall,(i<0)?cmParamAlternativeAliasAddress:cmParamAlternativeDestExtraCallInfo,(i<0)?id++:i,sizeof(cmAlias),(char*)&alias);
                                    i++;
                                }

                                alias.string=numPtr;
                                break;
                            }
        }
        if (!strncmp("TA:",numPtr,3))
        {
            char* ipPtr;
            cmTransportAddress qAddress;
            namedID=TRUE;
            numPtr+=3;

            qAddress.length = (UINT16)cmTransportTypeIP;
            qAddress.ip = 0;
            qAddress.port = 1720;
            ipPtr=numPtr;
            if ((ptr=strchr(numPtr,':'))!=NULL)
            {

                qAddress.port=(UINT16)atoi(ptr+1);
                *ptr=0;
            }
            qAddress.ip=liConvertIp(ipPtr);

            cmCallSetParam((HCALL)hsCall,cmParamAlternativeAddress,0,
                sizeof(cmTransportAddress),(char*)&qAddress);
        }
        if (namedID)
            numPtr=nextPtr;
    }

    cmCallSetParam((HCALL)hsCall,cmParamFacilityReason,0,cmReasonTypeCallForwarded,NULL);
    cmCallFacility(hsCall,-1);
    cmiAPIExit(hApp,(char*)"cmCallForward()=0");
    return 0;
}


#ifdef __cplusplus
}
#endif
