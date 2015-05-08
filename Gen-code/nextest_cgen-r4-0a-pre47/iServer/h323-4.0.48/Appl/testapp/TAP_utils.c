#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                TAP_utils.c
 *
 * Application utility functions
 *
 * 1. Enumeration to string conversion functions
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Tsahi Levent-Levi                   26-Jun-2000
 *
 ********************************************************************************************/


#include <ctype.h>

#ifdef USE_H450
#include <h450.h>
#endif  /* USE_H450 */

#include "TAP_init.h"
#include "TAP_general.h"
#include "TAP_utils.h"




static char internalBuffer[10000];



/********************************************************************************************
 *
 * Internal functions
 *
 ********************************************************************************************/

typedef struct
{
    const char* strValue;
    INT32       enumValue;
} ConvertValue;



/********************************************************************************************
 * String2Enum
 * purpose : Convert a string value to its enumeration value
 *           Used as a general conversion function
 * input   : string     - String parameter to convert
 *           cvTable    - Conversion table to use
 * output  : none
 * return  : Parameter enumeration value
 ********************************************************************************************/
INT32 String2Enum(char* string, ConvertValue* cvTable)
{
    ConvertValue* val = cvTable;

    while (val->strValue != NULL)
    {
        if (strcmp(string, val->strValue) == 0)
            return val->enumValue;
        val++;
    }

    return -1;
}


/********************************************************************************************
 * Enum2String
 * purpose : Convert an enumeration value into a string
 *           Used as a general conversion function
 * input   : value      - Enumeration value to convert
 *           cvTable    - Conversion table to use
 * output  : none
 * return  : String enumeration value
 ********************************************************************************************/
char* Enum2String(INT32 value, ConvertValue* cvTable)
{
    ConvertValue* val = cvTable;

    while (val->strValue != NULL)
    {
        if (value == val->enumValue)
            return (char *)val->strValue;
        val++;
    }

    return (char *)"-unknown-";
}



/********************************************************************************************
 * BooleanStr
 * purpose : Return a string representing a boolean value
 * input   : value  - The value to convert
 * output  : none
 * return  : String value of the boolean
 ********************************************************************************************/
char* BooleanStr(BOOL value)
{
    switch (value)
    {
        case TRUE:  return (char *)"1";
        case FALSE: return (char *)"0";
        default:    return (char *)"-???-";
    }
}


/********************************************************************************************
 * hStrcmp
 * purpose : comparing strings as capitol representation.
 * input   : a,b - strings
 * output  : None.
 * return  : 0  - the strings are the same.
 *           -1 - b is "bigger" than a.
 *            1 - a is "bigger" than b.
 ********************************************************************************************/
int hStrcmp(const char* a, const char* b)
{
    char high_a,high_b;
    while(*a && *b)
    {
        high_a = toupper(*a);
        high_b = toupper(*b);
        if (high_a > high_b)
            return 1;
        else if (high_a < high_b)
            return -1;

        a++;
        b++;
    }
    if (*a)
    {
        if (*b)
            return 0;
        else
            return 1;
    }

    return -1;
}


/********************************************************************************************
 * ParseList
 * purpose : Parse a TCL list, getting an element from it
 * input   : string     - String to parse
 *           from       - Character to begin parsing from. (0-based index)
 * output  : firstChar  - First character in the string of the parsed element
 *                        -1 is returned if couldn't find an element
 *           numChars   - Number of characters in parsed element
 *                        0 is returned if couldn't find an element
 * return  : The offset of the next element
 *           -1 is returned if theres no next element
 ********************************************************************************************/
int ParseList(IN char* string, IN int from, OUT int* firstChar, OUT int* numChars)
{
    char* fromPtr;
    char* ptr;
    int anyBrackets = 0;
    int brackets = 0;
    ptr = string + from;

    /* Skip blanks */
    while (isspace((int)*ptr)) ptr++;

    *firstChar = -1;
    *numChars = 0;

    if (*ptr == '\0') return -1;

    /* See if we've reached a list */
    if (*ptr == '{')
    {
        brackets = 1;
        anyBrackets = 1;
        ptr++;
        *firstChar = ptr - string;
    }
    else
        *firstChar = ptr - string;
    fromPtr = ptr;

    while ((*ptr != '\0') && (!isspace((int)*ptr) || (brackets > 0)))
    {
        if (*ptr == '{')
            brackets++;
        else if (*ptr == '}')
            brackets--;
        ptr++;
    }

    *numChars = (ptr - fromPtr);
    if (anyBrackets)
        (*numChars)--;

    /* Skip blanks again */
    while (isspace((int)*ptr)) ptr++;

    if (*ptr == '\0')
        return -1;
    else
        return ptr - string;
}






/********************************************************************************************
 *
 * The next structures are used to get the string/value of enumerator wrapped by the testApp.
 *
 ********************************************************************************************/



/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*                      General (PRTCL)                                                    */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/


/*******************************************************************************************/
/*                  General state                                                          */
/*******************************************************************************************/

static ConvertValue cvGenStatus[] = {
    {"Success",             (INT32)0},
    {"Error -1",            (INT32)RVERROR},
    {"Resouces problem -2", (INT32)RESOURCES_PROBLEM},
    {"Object not found -3", (INT32)OBJECT_WAS_NOT_FOUND},
    {NULL, 1}
};

char* Status2String(IN int  status)
{
    static char result[20];

    if (status >= 0)
        sprintf(result, "Success (%d)", status);
    else
    {
        char* strName = Enum2String(status, cvGenStatus);
        if (strcmp(strName, "-unknown-") != 0)
            sprintf(result, "Failure (%s)", strName);
        else
            sprintf(result, "Failure (%d)", status);
    }

    return result;
}

int String2Status(IN char* string)
{
    if (strncmp("Success", string, 7) == 0)
        return 0;
    else
        return RVERROR;
}


/*******************************************************************************************/
/*                  PRTCL party number type                                                        */
/*******************************************************************************************/

static ConvertValue cvPartyNumber[] = {
        {"PUU",     (INT32)cmPartyNumberPublicUnknown},
        {"PUI",     (INT32)cmPartyNumberPublicInternationalNumber},
        {"PUN",     (INT32)cmPartyNumberPublicNationalNumber},
        {"PUNS",    (INT32)cmPartyNumberPublicNetworkSpecificNumber},
        {"PUS",     (INT32)cmPartyNumberPublicSubscriberNumber},
        {"PUA",     (INT32)cmPartyNumberPublicAbbreviatedNumber},
        {"D",       (INT32)cmPartyNumberDataPartyNumber},
        {"T",       (INT32)cmPartyNumberTelexPartyNumber},
        {"PRU",     (INT32)cmPartyNumberPrivateUnknown},
        {"PRL2",    (INT32)cmPartyNumberPrivateLevel2RegionalNumber},
        {"PRL1",    (INT32)cmPartyNumberPrivateLevel1RegionalNumber},
        {"PRP",     (INT32)cmPartyNumberPrivatePISNSpecificNumber},
        {"PRL",     (INT32)cmPartyNumberPrivateLocalNumber},
        {"PRA",     (INT32)cmPartyNumberPrivateAbbreviatedNumber},
        {"N",       (INT32)cmPartyNumberNationalStandardPartyNumber},
        {NULL, -1}
};

cmPartyNumberType String2PartyNumber(char *string)
{
    return (cmPartyNumberType)String2Enum(string, cvPartyNumber);
}

char* PartyNumber2String(cmPartyNumberType value)
{
    return Enum2String((INT32)value,cvPartyNumber);
}



/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*                                  CM                                                     */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/


/*******************************************************************************************/
/*                  CM call parameter                                                      */
/*******************************************************************************************/

static ConvertValue cvCMCallParam[] = {
    {"First",                   (INT32)cmParamFirst},
    {"SourceAddress",           (INT32)cmParamSourceAddress},
    {"SourceInfo",              (INT32)cmParamSourceInfo},
    {"DestinationAddress",      (INT32)cmParamDestinationAddress},
    {"DestCallSignalAddress",   (INT32)cmParamDestCallSignalAddress},
    {"SourceCallSignalAddr",    (INT32)cmParamSrcCallSignalAddress},
    {"DestExtraCallInfo",       (INT32)cmParamDestExtraCallInfo},
    {"ActiveMC",                (INT32)cmParamActiveMc},
    {"CID",                     (INT32)cmParamCID},
    {"ConferenceGoal",          (INT32)cmParamConferenceGoal},
    {"CallType",                (INT32)cmParamCallType},
    {"SetupH245Addr",           (INT32)cmParamSetupH245Address},
    {"H245Addr",                (INT32)cmParamH245Address},
    {"DestInfo",                (INT32)cmParamDestinationInfo},
    {"ReleaseCompleteReason",   (INT32)cmParamReleaseCompleteReason},
    {"ReleaseCompleteCause",    (INT32)cmParamReleaseCompleteCause},
    {"Rate",                    (INT32)cmParamRate},
    {"RequestedRate",           (INT32)cmParamRequestedRate},
    {"InfoTransCap",            (INT32)cmParamInformationTransferCapability},
    {"MultiRate",               (INT32)cmParamMultiRate},
    {"CalledPartyNumber",       (INT32)cmParamCalledPartyNumber},
    {"CalledPartySubAddr",      (INT32)cmParamCalledPartySubAddress},
    {"CallingPartyNumber",      (INT32)cmParamCallingPartyNumber},
    {"CallingPartySubAddr",     (INT32)cmParamCallingPartySubAddress},
    {"Extention",               (INT32)cmParamExtention},
    {"AlternativeAddr",         (INT32)cmParamAlternativeAddress},
    {"AlternativeAlias",        (INT32)cmParamAlternativeAliasAddress},
    {"FacilityReason",          (INT32)cmParamFacilityReason},
    {"DestinationIpAddress",    (INT32)cmParamDestinationIpAddress},
    {"RemoteIP",                (INT32)cmParamRemoteIpAddress},
    {"Q931CRV",                 (INT32)cmParamCRV},
    {"RASCRV",                  (INT32)cmParamRASCRV},
    {"CallSignalingAddr",       (INT32)cmParamCallSignalAddress},
    {"EstablishH245",           (INT32)cmParamEstablishH245},
    {"Display",                 (INT32)cmParamDisplay},
    {"FacilityCallID",          (INT32)cmParamFacilityCID},
    {"ConnectDisplay",          (INT32)cmParamConnectDisplay},
    {"UserUser",                (INT32)cmParamUserUser},
    {"ConnectUserUser",         (INT32)cmParamConnectUserUser},
    {"FullSourceInfo",          (INT32)cmParamFullSourceInfo},
    {"FullDestinationInfo",     (INT32)cmParamFullDestinationInfo},
    {"SetupNonStandardData",    (INT32)cmParamSetupNonStandardData},
    {"CallPrNonStandardData",   (INT32)cmParamCallProceedingNonStandardData},
    {"AlertNonStandardData",    (INT32)cmParamAlertingNonStandardData},
    {"ConnectNonStandardData",  (INT32)cmParamConnectNonStandardData},
    {"RelCompNonStandardData",  (INT32)cmParamReleaseCompleteNonStandardData},
    {"SetupNonStandard",        (INT32)cmParamSetupNonStandard},
    {"CallProcNonStandard",     (INT32)cmParamCallProceedingNonStandard},
    {"AlertNonStandard",        (INT32)cmParamAlertingNonStandard},
    {"ConnectNonStandard",      (INT32)cmParamConnectNonStandard},
    {"RelCompNonStandard",      (INT32)cmParamReleaseCompleteNonStandard},
    {"AltDestExtraCallInfo",    (INT32)cmParamAlternativeDestExtraCallInfo},
    {"AltExtention",            (INT32)cmParamAlternativeExtention},
    {"SetupFastStart",          (INT32)cmParamSetupFastStart},
    {"ConnectFastStart",        (INT32)cmParamConnectFastStart},
    {"EarlyH245",               (INT32)cmParamEarlyH245},
    {"CallID",                  (INT32)cmParamCallID},
    {"PreGrantedARQ",           (INT32)cmParamPreGrantedArq},
    {"AlertingFastStart",       (INT32)cmParamAlertingFastStart},
    {"CallProcFastStart",       (INT32)cmParamCallProcFastStart},
    {"AlertingH245Addr",        (INT32)cmParamAlertingH245Address},
    {"CallProcH245Addr",        (INT32)cmParamCallProcH245Address},
    {"CanOverlapSending",       (INT32)cmParamCanOverlapSending},
    {"IncompleteAddress",       (INT32)cmParamIncompleteAddress},
    {"H245Tunneling",           (INT32)cmParamH245Tunneling},
    {"FastStartInUseObsolete",  (INT32)cmParamFastStartInUseObsolete},
    {"SetupCanOverlapSend",     (INT32)cmParamSetupCanOverlapSending},
    {"SetupSendingComplete",    (INT32)cmParamSetupSendingComplete},
    {"FullSourceAddress",       (INT32)cmParamFullSourceAddress},
    {"FullDestinationAddress",  (INT32)cmParamFullDestinationAddress},
    {"RouteCallSignalAddress",  (INT32)cmParamRouteCallSignalAddress},
    {"H245Stage",               (INT32)cmParamH245Stage},
    {"H245Parallel",            (INT32)cmParamH245Parallel},
    {"ShutdownEmptyConnection", (INT32)cmParamShutdownEmptyConnection},
    {"IsMultiplexed",           (INT32)cmParamIsMultiplexed},
    {"AnnexE",                  (INT32)cmParamAnnexE},
    {"DestinationAnnexEAddress",(INT32)cmParamDestinationAnnexEAddress},
    {"ConnectedAddress",        (INT32)cmParamConnectedAddress},
    {NULL, -1}
};

cmCallParam String2CMCallParam(char* string)
{
    return (cmCallParam)String2Enum(string,cvCMCallParam);
}

char* CMCallParam2String(cmCallParam param)
{
    return Enum2String((INT32)param,cvCMCallParam);
}

static ConvertValue cvCMH245Stage[] = {
    {"setup",             (INT32)cmTransH245Stage_setup},
    {"call proceeding",   (INT32)cmTransH245Stage_callProceeding},
    {"alerting",          (INT32)cmTransH245Stage_alerting},
    {"connect",           (INT32)cmTransH245Stage_connect},
    {"early",             (INT32)cmTransH245Stage_early},
    {"facility",          (INT32)cmTransH245Stage_facility},
    {"no H245",           (INT32)cmTransH245Stage_noH245},
    {NULL, -1}
};

cmH245Stage String2CMH245Stage(char* string)
{
    return (cmH245Stage)String2Enum(string, cvCMH245Stage);
}

char* CMH245Stage2String(cmH245Stage stage)
{
    return Enum2String((INT32)stage ,cvCMH245Stage);
}

static ConvertValue cvCMAnnexEUsageMode[] = {
    {"AnnexE",      (INT32)cmTransUseAnnexE},
    {"TPKT",        (INT32)cmTransNoAnnexE},
    {"AnnexE/TPKT", (INT32)cmTransPreferedAnnexE},
    {"TPKT/AnnexE", (INT32)cmTransPreferedTPKT},
    {NULL, -1}
};

cmAnnexEUsageMode String2CMAnnexEUsageMode(char* string)
{
    return (cmAnnexEUsageMode)String2Enum(string, cvCMAnnexEUsageMode);
}

char* CMAnnexEUsageMode2String(cmAnnexEUsageMode stage)
{
    return Enum2String((INT32)stage ,cvCMAnnexEUsageMode);
}


/*******************************************************************************************/
/*                  CM call state                                                          */
/*******************************************************************************************/

static ConvertValue cvCMCallState[] = {
    {"DialTone",            (INT32)cmCallStateDialtone},
    {"Proceeding",          (INT32)cmCallStateProceeding},
    {"RingBack",            (INT32)cmCallStateRingBack},
    {"Connected",           (INT32)cmCallStateConnected},
    {"Disconnected",        (INT32)cmCallStateDisconnected},
    {"StateIdle",           (INT32)cmCallStateIdle},
    {"Offering",            (INT32)cmCallStateOffering},
    {"Transferring",        (INT32)cmCallStateTransfering},
    {"IncompleteAddress",   (INT32)cmCallStateIncompleteAddress},
    {"AddressAck",          (INT32)cmCallStateWaitAddressAck},
    {NULL, -1}};


static ConvertValue cvCMCallStateMode[] = {
    {"DisconnectedBusy",                    (INT32)cmCallStateModeDisconnectedBusy},
    {"DisconnectedNormal",                  (INT32)cmCallStateModeDisconnectedNormal},
    {"DisconnectedReject",                  (INT32)cmCallStateModeDisconnectedReject},
    {"DisconnectedUnreachable",             (INT32)cmCallStateModeDisconnectedUnreachable},
    {"DisconnectedUnknown",                 (INT32)cmCallStateModeDisconnectedUnknown},
    {"DisconnectedLocal",                   (INT32)cmCallStateModeDisconnectedLocal},
    {"ConnectedControl",                    (INT32)cmCallStateModeConnectedControl},
    {"ConnectedCallSetup",                  (INT32)cmCallStateModeConnectedCallSetup},
    {"ConnectedCall",                       (INT32)cmCallStateModeConnectedCall},
    {"ConnectedConference",                 (INT32)cmCallStateModeConnectedConference},
    {"CallSetupConnected",                  (INT32)cmCallStateModeCallSetupConnected},
    {"CallConnected",                       (INT32)cmCallStateModeCallConnected},
    {"OfferingCreate",                      (INT32)cmCallStateModeConnectedConference+1},
    {"OfferingInvite",                      (INT32)cmCallStateModeOfferingInvite},
    {"OfferingJoin",                        (INT32)cmCallStateModeOfferingJoin},
    {"OfferingCapabilityNegotiation",       (INT32)cmCallStateModeOfferingCapabilityNegotiation},
    {"CallIndependentSupplementaryService", (INT32)cmCallStateModeOfferingCallIndependentSupplementaryService},
    {"eDisconnectedIncompleteAddress",      (INT32)cmCallStateModeDisconnectedIncompleteAddress},
    {NULL,-1}};







cmCallState_e String2CMCallState(char* string)
{
    return (cmCallState_e)String2Enum(string,cvCMCallState);
}

char* CMCallState2String(cmCallState_e state)
{
    return Enum2String((INT32)state,cvCMCallState);
}


cmCallStateMode_e String2CMCallStateMode(char* string)
{
    return (cmCallStateMode_e)String2Enum(string,cvCMCallStateMode);
}

char* CMCallStateMode2String(cmCallStateMode_e stateMode)
{
    return Enum2String((INT32)stateMode,cvCMCallStateMode);
}

/*******************************************************************************************/
/*                  CM channel state                                                       */
/*******************************************************************************************/

static ConvertValue cvCMChannelState[] = {
    {"DialTone",     (INT32)cmChannelStateDialtone},
    {"RingBack",     (INT32)cmChannelStateRingBack},
    {"Connected",    (INT32)cmChannelStateConnected},
    {"Disconnected", (INT32)cmChannelStateDisconnected},
    {"Idle",         (INT32)cmChannelStateIdle},
    {"Offering",     (INT32)cmChannelStateOffering},
    {NULL, -1}
};


static ConvertValue cvCMChannelStateMode [] = {
    {"On",                                      (INT32)cmChannelStateModeOn},
    {"ModeOff",                                 (INT32)cmChannelStateModeOff},
    {"DisconnectedLocal",                       (INT32)cmChannelStateModeDisconnectedLocal},
    {"DisconnectedRemote",                      (INT32)cmChannelStateModeDisconnectedRemote},
    {"DisconnectedMasterSlaveConflict",         (INT32)cmChannelStateModeDisconnectedMasterSlaveConflict},
    {"ModeDuplex",                              (INT32)cmChannelStateModeDuplex},
    {"DisconnectedReasonUnknown",               (INT32)cmChannelStateModeDisconnectedReasonUnknown},
    {"DisconnectedReasonReopen",                (INT32)cmChannelStateModeDisconnectedReasonReopen},
    {"DisconnectedReasonReservationFailure",    (INT32)cmChannelStateModeDisconnectedReasonReservationFailure},
    {NULL,-1}
};


cmChannelState_e String2CMChannelState(char* string)
{
    return (cmChannelState_e)String2Enum(string,cvCMChannelState);
}

char* CMChannelState2String(cmChannelState_e state)
{
    return Enum2String((INT32)state,cvCMChannelState);
}

cmChannelStateMode_e String2CMChannelStateMode(char* string)
{
    return (cmChannelStateMode_e)String2Enum(string,cvCMChannelStateMode);
}

char* CMChannelStateMode2String(cmChannelStateMode_e stateMode)
{
    return Enum2String((INT32)stateMode,cvCMChannelStateMode);
}




/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*                                RAS                                                      */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/


/*******************************************************************************************/
/*                  RAS transaction type                                                   */
/*******************************************************************************************/

static ConvertValue cvRasTransaction[] = {
    {"Gatekeeper",              (INT32)cmRASGatekeeper},
    {"Registration",            (INT32)cmRASRegistration},
    {"Unregistration",          (INT32)cmRASUnregistration},
    {"Admission",               (INT32)cmRASAdmission},
    {"Disengage",               (INT32)cmRASDisengage},
    {"Bandwidth",               (INT32)cmRASBandwidth},
    {"Location",                (INT32)cmRASLocation},
    {"Info",                    (INT32)cmRASInfo},
    {"NonStandard",             (INT32)cmRASNonStandard},
    {"Unknown",                 (INT32)cmRASUnknown},
    {"ResourceAvailability",    (INT32)cmRASResourceAvailability},
    {"UnsolicitedIRR",          (INT32)cmRASUnsolicitedIRR},
    {"ServiceControl",          (INT32)cmRASServiceControl},
    {NULL, -1}
};

cmRASTransaction String2RASTransaction(char* string)
{
    return (cmRASTransaction)String2Enum(string, cvRasTransaction);
}

char* RASTransaction2String(cmRASTransaction transaction)
{
    return Enum2String((INT32)transaction ,cvRasTransaction);
}


/*******************************************************************************************/
/*                  RAS reason type                                                        */
/*******************************************************************************************/

static ConvertValue cvRasReason[] = {
    {"ResourceUnavailable",         (INT32)cmRASReasonResourceUnavailable},
    {"InsufficientResources",             (INT32)cmRASReasonInsufficientResources},
    {"InvalidRevision",                   (INT32)cmRASReasonInvalidRevision},
    {"InvalidCallSignalAddress",          (INT32)cmRASReasonInvalidCallSignalAddress},
    {"InvalidRASAddress",                 (INT32)cmRASReasonInvalidRASAddress},
    {"InvalidTerminalType",               (INT32)cmRASReasonInvalidTerminalType},
    {"InvalidPermission",                 (INT32)cmRASReasonInvalidPermission},
    {"InvalidConferenceID",               (INT32)cmRASReasonInvalidConferenceID},
    {"InvalidEndpointID",                 (INT32)cmRASReasonInvalidEndpointID},
    {"CallerNotRegistered",               (INT32)cmRASReasonCallerNotRegistered},
    {"CalledPartyNotRegistered",          (INT32)cmRASReasonCalledPartyNotRegistered},
    {"DiscoveryRequired",                 (INT32)cmRASReasonDiscoveryRequired},
    {"DuplicateAlias",                    (INT32)cmRASReasonDuplicateAlias},
    {"TransportNotSupported",             (INT32)cmRASReasonTransportNotSupported},
    {"CallInProgress",                    (INT32)cmRASReasonCallInProgress},
    {"RouteCallToGateKeeper",             (INT32)cmRASReasonRouteCallToGatekeeper},
    {"RequestToDropOther",                (INT32)cmRASReasonRequestToDropOther},
    {"NotRegistered",                     (INT32)cmRASReasonNotRegistered},
    {"Undefined",                         (INT32)cmRASReasonUndefined},
    {"TerminalExcluded",                  (INT32)cmRASReasonTerminalExcluded},
    {"NotBound",                          (INT32)cmRASReasonNotBound},
    {"NotCurrentlyRegistered",            (INT32)cmRASReasonNotCurrentlyRegistered},
    {"RequestDenied",                     (INT32)cmRASReasonRequestDenied},
    {"LocationNotFound",                  (INT32)cmRASReasonLocationNotFound},
    {"SecurityDenial",                    (INT32)cmRASReasonSecurityDenial},
    {"TransportQOSNotSupported",          (INT32)cmRASReasonTransportQOSNotSupported},
    {"ResourceUnaveliable",               (INT32)cmRASResourceUnavailable},
    {"InvalidAlias",                      (INT32)cmRASReasonInvalidAlias},
    {"PermissionDenied",                  (INT32)cmRASReasonPermissionDenied},
    {"QOSControlNotSupported",            (INT32)cmRASReasonQOSControlNotSupported},
    {"IncompleteAddress",                 (INT32)cmRASReasonIncompleteAddress},
    {"FullRegistrationRequired",          (INT32)cmRASReasonFullRegistrationRequired},
    {"RouteCallToSCN",                    (INT32)cmRASReasonRouteCallToSCN},
    {"AliasesInconsistent",               (INT32)cmRASReasonAliasesInconsistent},
    {"AdditiveRegistrationNotSupported",  (INT32)cmRASReasonAdditiveRegistrationNotSupported},
    {"InvalidTerminalAliases",            (INT32)cmRASReasonInvalidTerminalAliases},
    {"ExceedsCallCapacity",               (INT32)cmRASReasonExceedsCallCapacity},
    {"CollectDestination",                (INT32)cmRASReasonCollectDestination},
    {"CollectPIN",                        (INT32)cmRASReasonCollectPIN},
    {"GenericData",                       (INT32)cmRASReasonGenericData},
    {"NeededFeatureNotSupported",         (INT32)cmRASReasonNeededFeatureNotSupported},
    {"UnknownMessageResponse",            (INT32)cmRASReasonUnknownMessageResponse},
    {NULL, -1}
};

cmRASReason String2RASReason(char* string)
{
    return (cmRASReason)String2Enum(string, cvRasReason);
}

char* RASReason2String(cmRASReason reason)
{
    return Enum2String((INT32)reason ,cvRasReason);
}


/*******************************************************************************************/
/*                  RAS stage type                                                         */
/*******************************************************************************************/

static ConvertValue cvRasStage[] = {
    {"Request",     (INT32)cmRASTrStageRequest},
    {"Confirm",     (INT32)cmRASTrStageConfirm},
    {"Reject",      (INT32)cmRASTrStageReject},
    {NULL, -1}
};

cmRASTrStage String2RASTrStage(char* string)
{
    return (cmRASTrStage)String2Enum(string, cvRasStage);
}

char* RASTrStage2String(cmRASTrStage reason)
{
    return Enum2String((INT32)reason ,cvRasStage);
}


/*******************************************************************************************/
/*                  RAS param type                                                         */
/*******************************************************************************************/

static ConvertValue cvRasParam[] = {
    {"GatekeeperID",            (INT32)cmRASParamGatekeeperID},
    {"RASAddress",              (INT32)cmRASParamRASAddress},
    {"CallSignalAddress",       (INT32)cmRASParamCallSignalAddress},
    {"EndpointType",            (INT32)cmRASParamEndpointType},
    {"TerminalType",            (INT32)cmRASParamTerminalType},
    {"EndpointAlias",           (INT32)cmRASParamEndpointAlias},
    {"TerminalAlias",           (INT32)cmRASParamTerminalAlias},
    {"DiscoveryComplete",       (INT32)cmRASParamDiscoveryComplete},
    {"EndpointVendor",          (INT32)cmRASParamEndpointVendor},
    {"CallType",                (INT32)cmRASParamCallType},
    {"CallModel",               (INT32)cmRASParamCallModel},
    {"EndpointID",              (INT32)cmRASParamEndpointID},
    {"DestInfo",                (INT32)cmRASParamDestInfo},
    {"SrcInfo",                 (INT32)cmRASParamSrcInfo},
    {"DestExtraCallInfo",       (INT32)cmRASParamDestExtraCallInfo},
    {"DestCallSignalAddress",   (INT32)cmRASParamDestCallSignalAddress},
    {"SrcCallSignalAddress",    (INT32)cmRASParamSrcCallSignalAddress},
    {"Bandwidth",               (INT32)cmRASParamBandwidth},
    {"ActiveMC",                (INT32)cmRASParamActiveMC},
    {"AnswerCall",              (INT32)cmRASParamAnswerCall},
    {"IrrFrequency",            (INT32)cmRASParamIrrFrequency},
    {"ReplyAddress",            (INT32)cmRASParamReplyAddress},
    {"DisengageReason",         (INT32)cmRASParamDisengageReason},
    {"RejectedAlias",           (INT32)cmRASParamRejectedAlias},
    {"RejectReason",            (INT32)cmRASParamRejectReason},
    {"CID",                     (INT32)cmRASParamCID},
    {"DestinationIpAddress",    (INT32)cmRASParamDestinationIpAddress},
    {"NonStandard",             (INT32)cmRASParamNonStandard},
    {"NonStandardData",         (INT32)cmRASParamNonStandardData},
    {"CRV",                     (INT32)cmRASParamCRV},
    {"MulticastTransaction",    (INT32)cmRASParamMulticastTransaction},
    {"TransportQOS",            (INT32)cmRASParamTransportQOS},
    {"KeepAlive",               (INT32)cmRASParamKeepAlive},
    {"TimeToLive",              (INT32)cmRASParamTimeToLive},
    {"Delay",                   (INT32)cmRASParamDelay},
    {"CallID",                  (INT32)cmRASParamCallID},
    {"AnsweredCall",            (INT32)cmRASParamAnsweredCall},
    {"AlmostOutOfResources",    (INT32)cmRASParamAlmostOutOfResources},
    {"AlternateGatekeeper",     (INT32)cmRASParamAlternateGatekeeper},
    {"AltGKInfo",               (INT32)cmRASParamAltGKInfo},
    {"AltGKisPermanent",        (INT32)cmRASParamAltGKisPermanent},
    {"Empty",                   (INT32)cmRASParamEmpty},
    {"SourceInfo",              (INT32)cmRASParamSourceInfo},
    {"NeedResponse",            (INT32)cmRASParamNeedResponse},
    {"MaintainConnection",      (INT32)cmRASParamMaintainConnection},
    {"MultipleCalls",           (INT32)cmRASParamMultipleCalls},
    {"WillRespondToIRR",        (INT32)cmRASParamWillRespondToIRR},
    {"SupportsAltGk",           (INT32)cmRASParamSupportsAltGk},
    {"AdditiveRegistration",    (INT32)cmRASParamAdditiveRegistration},
    {"SupportsAdditiveRegistration",(INT32)cmRASParamSupportsAdditiveRegistration},
    {"SegmentedResponseSupported",  (INT32)cmRASParamSegmentedResponseSupported},
    {"NextSegmentRequested",    (INT32)cmRASParamNextSegmentRequested},
    {"CapacityInfoRequested",   (INT32)cmRASParamCapacityInfoRequested},
    {"HopCount",                (INT32)cmRASParamHopCount},
    {"InvalidTerminalAlias",    (INT32)cmRASParamInvalidTerminalAlias},
    {"UnregReason",             (INT32)cmRASParamUnregReason},
    {"IrrStatus",               (INT32)cmRASParamIrrStatus},
    {"CallHandle",              (INT32)cmRASParamCallHandle},
    {NULL, -1}
};

cmRASParam String2RASParam(char* string)
{
    return (cmRASParam)String2Enum(string, cvRasParam);
}

char* RASParam2String(cmRASParam reason)
{
    return Enum2String((INT32)reason ,cvRasParam);
}

/*******************************************************************************************/
/*                  RAS Endpoint type                                                      */
/*******************************************************************************************/

static ConvertValue cvCMEndpointType[] = {
    {"Terminal",  (INT32)cmEndpointTypeTerminal},
    {"Gateway",   (INT32)cmEndpointTypeGateway},
    {"MCU",       (INT32)cmEndpointTypeMCU},
    {"GK",        (INT32)cmEndpointTypeGK},
    {"Undefined", (INT32)cmEndpointTypeUndefined},
    {"SET",       (INT32)cmEndpointTypeSET},
    {NULL, -1}
};

cmEndpointType String2EndpointType(char* string)
{
    return (cmEndpointType)String2Enum(string, cvCMEndpointType);
}

char* EndpointType2String(cmEndpointType val)
{
    return Enum2String((INT32)val ,cvCMEndpointType);
}

/*******************************************************************************************/
/*                  RAS Call type                                                          */
/*******************************************************************************************/

static ConvertValue cvCMCallType[] = {
    {"P2P",   (INT32)cmCallTypeP2P},
    {"One2N", (INT32)cmCallTypeOne2N},
    {"N2One", (INT32)cmCallTypeN2One},
    {"N2Nw",  (INT32)cmCallTypeN2Nw},
    {NULL, -1}
};

cmCallType String2CallType(char* string)
{
    return (cmCallType)String2Enum(string, cvCMCallType);
}

char* CallType2String(cmCallType val)
{
    return Enum2String((INT32)val ,cvCMCallType);
}

/*******************************************************************************************/
/*                  RAS Call Model type                                                    */
/*******************************************************************************************/

static ConvertValue cvCMCallModelType[] = {
    {"Direct",  (INT32)cmCallModelTypeDirect},
    {"Routed",  (INT32)cmCallModelTypeGKRouted},
    {NULL, -1}
};

cmCallModelType String2CallModelType(char* string)
{
    return (cmCallModelType)String2Enum(string, cvCMCallModelType);
}

char* CallModelType2String(cmCallModelType val)
{
    return Enum2String((INT32)val ,cvCMCallModelType);
}

/*******************************************************************************************/
/*                  RAS Disengage Reason                                                   */
/*******************************************************************************************/

static ConvertValue cvCMDisengageReason[] = {
    {"Forced",     (INT32)cmRASDisengageReasonForcedDrop},
    {"Normal",     (INT32)cmRASDisengageReasonNormalDrop},
    {"Undefined",  (INT32)cmRASDisengageReasonUndefinedReason},
    {NULL, -1}
};

cmRASDisengageReason String2DisengageReason(char* string)
{
    return (cmRASDisengageReason)String2Enum(string, cvCMDisengageReason);
}

char* DisengageReason2String(cmRASDisengageReason val)
{
    return Enum2String((INT32)val ,cvCMDisengageReason);
}

/*******************************************************************************************/
/*                  RAS Unreg Reason                                                       */
/*******************************************************************************************/

static ConvertValue cvCMUnregReason[] = {
    {"RegistrationRequired",  (INT32)cmRASUnregReasonReregistrationRequired},
    {"TtlExpired",            (INT32)cmRASUnregReasonTtlExpired},
    {"SecurityDenial",        (INT32)cmRASUnregReasonSecurityDenial},
    {"UndefinedReason",       (INT32)cmRASUnregReasonUndefinedReason},
    {"Maintenance",           (INT32)cmRASUnregReasonMaintenance},
    {NULL, -1}
};

cmRASUnregReason String2UnregReason(char* string)
{
    return (cmRASUnregReason)String2Enum(string, cvCMUnregReason);
}

char* UnregReason2String(cmRASUnregReason val)
{
    return Enum2String((INT32)val ,cvCMUnregReason);
}

/*******************************************************************************************/
/*                  RAS Transport QOS                                                      */
/*******************************************************************************************/

static ConvertValue cvCMTransportQOS[] = {
    {"RegistrationRequired",  (INT32)cmTransportQOSEndpointControlled},
    {"TtlExpired",            (INT32)cmTransportQOSGatekeeperControlled},
    {"SecurityDenial",        (INT32)cmTransportQOSNoControl},
    {NULL, -1}
};

cmTransportQOS String2TransportQOS(char* string)
{
    return (cmTransportQOS)String2Enum(string, cvCMTransportQOS);
}

char* TransportQOS2String(cmTransportQOS val)
{
    return Enum2String((INT32)val ,cvCMTransportQOS);
}

/*******************************************************************************************/
/*                  RAS IRR Status                                                         */
/*******************************************************************************************/

static ConvertValue cvCMIrrStatus[] = {
    {"Complete",     (INT32)cmRASIrrComplete},
    {"Incomplete",   (INT32)cmRASIrrIncomplete},
    {"InvalidCall",  (INT32)cmRASIrrInvalidCall},
    {NULL, -1}
};

int String2IrrStatus(char* string)
{
    return (int)String2Enum(string, cvCMIrrStatus);
}

char* IrrStatus2String(int val)
{
    return Enum2String((INT32)val ,cvCMIrrStatus);
}


/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*                                Q931                                                     */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/


/*******************************************************************************************/
/*                  Q931 message type                                                      */
/*******************************************************************************************/

static ConvertValue cvQ931MsgType[] = {
    {"Setup",            (INT32)cmQ931setup},
    {"SetupAck",         (INT32)cmQ931setupAck},
    {"Information",      (INT32)cmQ931information},
    {"Progress",         (INT32)cmQ931progress},
    {"Alerting",         (INT32)cmQ931alerting},
    {"CallProceeding",   (INT32)cmQ931callProceeding},
    {"Connect",          (INT32)cmQ931connect},
    {"ReleaseComplete",  (INT32)cmQ931releaseComplete},
    {"StatusInquiry",    (INT32)cmQ931statusInquiry},
    {"Status",           (INT32)cmQ931status},
    {"Facility",         (INT32)cmQ931facility},
    {"Notify",           (INT32)cmQ931notify},
    {NULL, -1}
};



cmCallQ931MsgType String2Q931MsgType(char* string)
{
    return (cmCallQ931MsgType)String2Enum(string,cvQ931MsgType);
}


char* Q931MsgType2String(cmCallQ931MsgType Q931MsgType)
{
    return Enum2String((INT32)Q931MsgType,cvQ931MsgType);
}







/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*                                Conversions                                              */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/

int String2CMNonStandardParam(IN char* string, OUT cmNonStandardParam* param)
{
    int country, extension, manufacture;
    int next, first, numChars;
    char ch, type;

    next = ParseList(string, 0, &first, &numChars);
    if (first < 0)
        return first;
    type = string[0];
    string = string+next;
    next = ParseList(string, 0, &first, &numChars);
    if (first < 0)
        return first;

    ch = string[first+numChars];
    string[first+numChars] = '\0';
    if (type == 'H')
    {
        sscanf(string+first, "%d %d %d", &country, &extension, &manufacture);
        param->info.objectLength = 0;
        param->info.t35CountryCode = country;
        param->info.t35Extension = extension;
        param->info.manufacturerCode = manufacture;
    }
    if (type == 'O')
    {
        memcpy(param->info.object, string+first, numChars);
        param->info.objectLength = numChars;
        param->info.object[numChars] = 0;
    }
    string[first+numChars] = ch;

    /* Set the data */
    if (next < 0)
    {
        param->data = NULL;
        param->length = 0;
    }
    else
    {
        param->data = string + next;
        param->length = strlen(param->data);
    }

    return 0;
}


char* CMNonStandardParam2String(IN cmNonStandardParam* param)
{
    char* ptr = internalBuffer;
    int len;

    if (param->length < 0)
        return (char *)"";

    if (param->info.objectLength <= 0)
    {
        ptr += sprintf(ptr, "H221 {%d %d %d} ",
                       param->info.t35CountryCode,
                       param->info.t35Extension,
                       param->info.manufacturerCode);
    }
    else
    {
        param->info.object[param->info.objectLength] = '\0';
        ptr += sprintf(ptr, "Object { %s } ", param->info.object);
    }

    /* Add the data */
    len = param->length;
    if (len > (int)sizeof(internalBuffer)-128)
        len = sizeof(internalBuffer)-128;
    memcpy(ptr, param->data, len);
    ptr[len] = '\0';

    return internalBuffer;
}


int String2CMAlternateGatekeeper(IN char* string, OUT cmAlternateGatekeeper* param)
{
    char addr[100];
    int needToReg, priority;
    int status;

    if (sscanf(string, "%s %s %d %d", addr, internalBuffer, &needToReg, &priority) != 4)
        return -1;

    status = String2TransportAddress(addr, &param->rasAddress);
    if (status < 0) return status;

    param->gatekeeperIdentifier.type = cmAliasTypeGatekeeperID;
    param->gatekeeperIdentifier.length = strlen(internalBuffer);
    param->gatekeeperIdentifier.string = internalBuffer;

    param->needToRegister = needToReg;
    param->priority = priority;

    return 0;
}


char* CMAlternateGatekeeper2String(IN cmAlternateGatekeeper* param)
{
    char* ptr;

    TransportAddress2String(&param->rasAddress, internalBuffer);
    ptr = internalBuffer + strlen(internalBuffer);

    sprintf(internalBuffer, " %s %d %d", param->gatekeeperIdentifier.string, param->needToRegister, param->priority);

    return internalBuffer;
}





/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*                                H450                                                     */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/
#ifdef USE_H450

static ConvertValue cvH450Opcodes[] = {
    {"Request",                     (INT32)ssCIRequest},
    {"GetCIPL",                     (INT32)ssCIGetCIPL},
    {"Isolated",                    (INT32)ssCIIsolated},
    {"ForcedRelease",               (INT32)ssCIForcedRelease},
    {"WOBRequest",                  (INT32)ssCIWOBRequest},
    {"SilentMoitor",                (INT32)ssCISilentMoitor},
    {"Notification",                (INT32)ssCINotification},
    {"notBusy",                     (INT32)ssCInotBusy},
    {"temporarilyUnavailable",      (INT32)ssCItemporarilyUnavailable},
    {"notAuthorized",               (INT32)ssCInotAuthorized},
    {"MakeSilent",                  (INT32)ssCIMakeSilent},
    {NULL, -1}
};

int String2SSPrimitive(char* string)
{
    return (int)String2Enum(string, cvH450Opcodes);
}

char* SSPrimitive2String(int opcode)
{
    return Enum2String((INT32)opcode ,cvH450Opcodes);
}


static ConvertValue cvH450ccIndication[] = {
    {"Activated",   (INT32)ccActivated},
    {"Suspended",   (INT32)ccSuspended},
    {"Resume",      (INT32)ccResume},
    {"Ringout",     (INT32)ccRingout},
    {"Rejected",    (INT32)ccRejected},
    {"TimeOut",     (INT32)ccTimeOut},
    {"CallDropped", (INT32)ccCallDropped},
    {NULL, -1}
};

ccIndication String2CCIndication(char* string)
{
    return (ccIndication)String2Enum(string, cvH450ccIndication);
}

char* CCIndication2String(ccIndication indication)
{
    return Enum2String((INT32)indication,cvH450ccIndication);
}


static ConvertValue cvH450proc_e[] = {
    {"Unconditional",   (INT32)cfu_p},
    {"Busy",            (INT32)cfb_p},
    {"NoResponse",      (INT32)cfnr_p},
    {NULL, -1}
};

proc_e String2proc_e(char* string)
{
    return (proc_e)String2Enum(string, cvH450proc_e);
}

char* proc_e2String(proc_e proc)
{
    return Enum2String((INT32)proc, cvH450proc_e);
}


static ConvertValue cvH450reason_e[] = {
    {"Unknown",         (INT32)unknown_r},
    {"Unconditional",   (INT32)cfu_r},
    {"Busy",            (INT32)cfb_r},
    {"NoResponse",      (INT32)cfnr_r},
    {NULL, -1}
};

reason_e String2reason_e(char* string)
{
    return (reason_e)String2Enum(string, cvH450reason_e);
}

char* reason_e2String(reason_e reason)
{
    return Enum2String((INT32)reason, cvH450reason_e);
}

#endif  /* USE_H450 */





#ifdef __cplusplus
}
#endif
