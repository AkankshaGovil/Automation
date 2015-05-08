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


#ifndef _CMINTERNAL_
#define _CMINTERNAL_

#include <rvcommon.h>
#include <psyntree.h>
#include <pvaltree.h>
#include <cm.h>
#include <mti.h>

#include <cat.h>
#include <ra.h>
#include <ema.h>
#include <tpkt.h>
#include <mti.h>
#include <transport.h>
#include <cmiras.h>
#include <cmiAddOns.h>
#include <cmCrossReference.h>
#include <cmmib.h>
#include <intutils.h>

#define ifE(a) if(a)(a)

typedef
int
(*newProtocolEH)(
        IN  void*               haProtocolParent,
        IN  void*               hsProtocol,
        OUT void**              lphaProtocol,
        IN  const char*         protocolName,
        OUT void*               newProtocol,
        OUT void*               newMessage,
        OUT BOOL*               selfDestructing
);

typedef struct
{
    INT32 nameId;
} fieldNames;


typedef enum
{
    makeCall                        =1,
    useGKCallSignalAddressToMakeCall=2,
    answerCall                      =4,
    useGKCallSignalAddressToAnswer  =8
}preGrantedArqFlags;

typedef enum
{
    ctrlReqNo,
    ctrlReqConnect,
    ctrlReqSeparate
}controlFlags;

typedef enum
{
    pmodeFullProperty,
    pmodeCopySingleMessages,
    pmodeDeleteSingleMessages,
    pmodeDoNotUseProperty
} propertyMode;


/* Enumeration of syntax trees needed to hold information for activating cmParam
   functions without the use of a properties database */
typedef enum
{
    cmParamSynSOAliasAddress,
    cmParamSynAliasAddress,
    cmParamSynCalledPartyNumber,
    cmParamSynCalledSubaddress,
    cmParamSynCallingPartyNumber,
    cmParamSynCallingSubaddress,
    cmParamSynEndpointType,
    cmParamSynConferenceGoal,
    cmParamSynCallType,
    cmParamSynUserData,
    cmParamSynNonStandardParameter,
    cmParamSynReleaseCompleteReason,
    cmParamSynFacilityReason,
    cmParamSynLast
} cmParamSyn;



typedef struct __cmElem
{
    SCMEVENT                    cmMyEvent;
    SCMCALLEVENT                cmMyCallEvent;

    /* control */
    SCMSESSIONEVENT             cmMySessionEvent;
    SCMCHANEVENT                cmMyChannelEvent;
    SCMPROTOCOLEVENT            cmMyProtocolEvent;
    CMPROTOCOLEVENT             cmMyProtocolEventReplacement;
    int                         cmMyProtocolEventReplacementSize;

    cmEvCallNewRawMessageT      cmEvCallNewRawMessage;
    cmEvCallSendRawMessageT     cmEvCallSendRawMessage;
    cmEvCallReleaseMessageContextT
                                cmEvCallReleaseMessageContext;


    cmNewCallbackEH             newCallback;

    HAPPPARTNER                 hAppPartner;
    propertyMode                callPropertyMode;   /* Mode used for the property database of
                                                       the stack */

    int         h245Conf; /* configuration node id */
    int         q931Conf; /* configuration node id */
    int         rasConf;  /* configuration node id */

    HPST        synProtH245;
    HPST        synProtQ931;
    HPST        synProtRAS;

    HPST        synAnnexL;
    HPST        synAnnexM;

    char        dynamicPayloadNumber; /* 0-31 */
    RVHLOGMGR   logMgr;

    /* call */


    RVHLOG      log;
    RVHLOG      logAPI;
    RVHLOG      logCB;
    RVHLOG      logERR;

    RVHLOG      logTPKT;
    RVHLOG      logConfig;
    RVHLOG      logFastStart;
    RVHLOG      logAppl;

    int         level;
    INT32       gatekeeper;

    HPST        hSyn;
    HPST        hAddrSyn;
    HPST        hAddrSynH245;
    HPST        synOLC;
    HPVT        hVal;               /* PVT used by the stack */
    HCFG        hCfg;               /* Configuration of the stack */
    BOOL        start;              /* TRUE if stack is running, FALSE if only initialized */
    BOOL        cidAssociate;
    char        delimiter;
    BOOL        diffSrcAddressInSetupAndARQ;
    HAPPAPP     haApp;              /* Application's handle for the stack's instance */
    HMEI        hGlobalDataMEI;
    RVHCAT      hCat;
    /* RAS */
    HRASMGR     rasManager;
    HAUTORASMGR hAutoRas;
    INT32       manualRAS;
    HPST        synConfRAS;
    int         rasUnicastHandle;
    int         rasMulticastHandle;
    UINT32      localIPAddress;
    HPST        synConfQ931;
    HPST        synConfH245;
    HTPKTCTRL   hTpktCtrl;
    HAPPTRANS   hTransport;
    HPST        h245TransCap;
    HPST        h245RedEnc;
    HPST        h245DataType;
    /*Calls*/

    HEMA        hCalls;
    int         crv;
    int         appDB;      /* PVT root node if application's default messages */
    int         busy;       /* Number of calls currently connected or being connected */
    BOOL        retry;
    int         newRate;
    int         rate;
    int         multiRate;
    int         t302;
    int         t304;
    int         postControlDisconnectionDelay;
    /*RAS Call*/

    /*Endpoint*/
    int         ipCallsOK;
    int         rasGranted;
    cmTransportAddress
                annexEAddress;
    int         rasGrantedProtocol;
    int         irrFrequencyInCall;
    /*RAS*/
    HPST        hRASSyn;
    int         rasAddessID;/*Local RAS Address*/
    HSTIMER     hTimers;
    int         requestSeqNum;
    int         q931Chan;
    int         q931AnnexEChan;

    /*Control*/
    int         t106;
    int         t101;
    int         maxChannels;
    HEMA        hChannels;

    /* MIB - H341 */
    HSTATISTIC  hStatistic;     /* Current stack's statistics */
    MibEventT   mibEvent;       /* MIB Callbacks to call */
    HMibHandleT mibHandle;      /* Handle of MIB to use */

    /* Other */
    int         encodeBufferSize;
    HMEI        applicationLock;    /* Application specific lock. Used for cmMeiEnter/Exit */

    /* Syntax types used by cmGetSynTreeByRootName for backward compatability */
    HPST        synQ931UU;
    HPST        synTerminalLabel;
    HPST        synGkCrypto;
    HPST        synGkPwdCert;

    HPST        synParams[cmParamSynLast];

	/* Nextone */	
    HPST        synCiscoARQ;
    HPST        synCiscoACF;
    HPST        synCiscoARJ;
    HPST        synCiscoLRQ;
    HPST        synCiscoLCF;
    HPST        synCiscoLRJ;

    /*fast Start*/
    int*        fastStartBuff; /* Array of proposals for all calls in the stack.
                                  The size of this array is:
                                  maxCalls*(maxFsProposed+maxFsAccepted) */
    BYTE*       fastStartBuff2; /* Array of indexes to match acked channels to proposed ones */
    int         maxFsProposed; /* Maximum number of proposed channels in a single call */
    int         maxFsAccepted; /* Maximum number of accepted channels in a single call */
    RvRandomGenerator
                seed;
} cmElem;




typedef enum
{
    cmRasStateInit,
    cmRasStateAwaiting,
    cmRasStateIdle
}rasStateFlags;

DECLARE_OPAQUE(HAPPMS);
DECLARE_OPAQUE(HMS);

DECLARE_OPAQUE(HAPPCAP);
DECLARE_OPAQUE(HCAP);

DECLARE_OPAQUE(HAPPH245);
DECLARE_OPAQUE(H245);

fieldNames* getParamFieldName(
           IN  cmCallParam       param);
int cmiReportControl(IN HCALL call,
             IN cmControlState      state,
             IN cmControlStateMode  stateMode);

typedef enum
{
    fssNo,
    fssRequest,
    fssAck,
    fssRej
}fssFlags;

int cmFastStartChannelsConnect( HCALL hsCall);
int cmCallPreCallBack(HAPP hApp);


/************************************************************************
 * cmTimerSet
 * purpose: Set a timer of the stack, reseting its value if it had one
 *          previously.
 * input  : hApp            - Stack's instance handle
 *          eventHandler    - Callback to call when timer expires
 *          context         - Context to use as parameter for callback function
 *          timeOut         - Timeout of timer in milliseconds
 * output : none
 * return : Timer's handle on success
 ************************************************************************/
HTI cmTimerSet(
        IN    HAPP                 hApp,
        IN    LPMTIMEREVENTHANDLER eventHandler,
        IN    void*                context,
        IN    UINT32               timeOut);


/************************************************************************
 * cmTimerReset
 * purpose: Reset a timer if it's set
 *          Used mainly for call timers.
 * input  : hApp    - Stack's instance handle
 *          timer   - Timer to reset
 * output : timer   - Timer's value after it's reset
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmTimerReset(IN HAPP hApp, INOUT HTI* timer);


/************************************************************************
 * cmIsRoutedCall
 * purpose: Check if the call is routed or direct. Used to create perCallInfo
 *          inside IRR messages by the ras module.
 * input  : hsCall  - Call handle to check
 * output : none
 * return : TRUE if call is routed
 ************************************************************************/
BOOL cmIsRoutedCall(IN HCALL hsCall);


void cmLock(IN HAPP hApp);
void cmUnlock(IN HAPP hApp);


#endif
#ifdef __cplusplus
}
#endif



