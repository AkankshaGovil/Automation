#ifndef _callsm_h_
#define _callsm_h_

#include "calldefs.h"
#include "sccdefs.h"

typedef enum
{
	SCC_eEventNone = -1,

	SCC_eNetworkEventsMin = 0,
	SCC_eNetworkSetup = SCC_eNetworkEventsMin,
	SCC_eNetworkAlerting, 
	SCC_eNetworkProceeding, 
	SCC_eNetworkConnect, 
	SCC_eNetworkTransportConnected,
	SCC_eNetworkCapabilities,
	SCC_eNetworkTCSAck,
	SCC_eNetworkControlConnected,
	SCC_eNetworkOLC,
	SCC_eNetworkChanConnect,
	SCC_eNetworkCLC,
	SCC_eNetworkReleaseComp,
	SCC_eNetworkRequestMode,
	SCC_eNetworkGenericMsg,
	SCC_eNetworkProgress, 
	SCC_eNetworkEventsMax = SCC_eNetworkProgress,

	SCC_eBridgeEventsMin = SCC_eNetworkEventsMax + 1,
	SCC_eBridgeSetup = SCC_eBridgeEventsMin,
	SCC_eBridgeAlerting,
	SCC_eBridgeProceeding,
	SCC_eBridgeConnect,
	SCC_eBridgeTransportConnected,
	SCC_eBridgeCapabilities,
	SCC_eBridgeTCSAck,
	SCC_eBridgeControlConnected,
	SCC_eBridgeOLC,
	SCC_eBridgeChanConnect,
	SCC_eBridgeCLC,
	SCC_eBridgeReleaseComp,
	SCC_eBridgeRequestMode,
	SCC_eBridgeGenericMsg,
	SCC_eBridgeProgress,
	SCC_eBridgeEventsMax = SCC_eBridgeProgress,
	SCC_eMax,

} SCC_CallEvent;


typedef enum
{
	SCC_eDTMF = 1,	
} SCC_CallSubEvent;

#define SCC_IsNetworkEvent(event)	((event >= SCC_eNetworkEventsMin) && \
					(event <= SCC_eNetworkEventsMax))
#define SCC_IsBridgeEvent(event)	((event >= SCC_eBridgeEventsMin) && \
					(event <= SCC_eBridgeEventsMax))

#define SCC_IsLeg1Event(event)	((event >= SCC_eLeg1EventsMin) && \
			(event <= SCC_eLeg1EventsMax))
#define SCC_IsLeg2Event(event)	((event >= SCC_eLeg2EventsMin) && \
			(event <= SCC_eLeg2EventsMax))
#define H323MAXNETWORKEVENTS	(SCC_eNetworkEventsMax-SCC_eNetworkEventsMin+1)
#define H323MAXBRIDGEEVENTS		(SCC_eBridgeEventsMax-SCC_eBridgeEventsMin+1)
#define H323MAXH323LEG1EVENTS	(SCC_eLeg1EventsMax-SCC_e_Leg1EventsMin+1)

#define H323MAXLEG2EVENTS		(SCC_eLeg2EventsMax-SCC_e_Leg2EventsMin+1)

typedef enum
{
	SCC_sMinState = 0,
	SCC_sError = SCC_sMinState,
	SCC_sIdle,
	SCC_sWaitOnRemote,
	SCC_sConnected,
	SCC_sMaxStates,
} SCC_CallState;


typedef enum
{
	UH323_sControlMinState = 0,
	UH323_sControlError = UH323_sControlMinState,
	UH323_sControlIdle,
	UH323_sControlTransportConnected,
	UH323_sControlCapSent,
	UH323_sControlConnected,
	UH323_sControlTransportDisconnected,
	UH323_sControlHeldByLocal,
	UH323_sControlHeldByRemote,
	UH323_sControlHeldByBoth,
	UH323_sControlHeldCapSent, // TCS sent after sending out null TCS
	UH323_sControlMaxStates,
} UH323_ControlState;

extern char * SCC_CallStateToStr(int state, char str[80]);
extern char * SCC_EventToStr(int state, char str[80]);
extern char * SCC_BridgeEventToStr(int state, char str[80]);

/* A Call leg is identified as either leg1 or leg2,
 * as an gateway/proxy would identify two links of
 * a logical call, which pass through it. Leg1, is
 * usually initiated by a remote, and leg2 is initiated
 * by the proxy.
 */
typedef enum
{
	SCC_CallLeg1 = 1,
	SCC_CallLeg2 = 2,
	SCC_CallLegMax = SCC_CallLeg2,

} SCC_CallLeg;

/* Action Routine Prototypes */
int SCCErrorHandler(SCC_EventBlock *);

int SCCNetworkSetup(SCC_EventBlock *);
int SCCNetworkAlerting(SCC_EventBlock *);
int SCCNetworkProceeding(SCC_EventBlock *);
int SCCNetworkConnect(SCC_EventBlock *);
int SCCNetworkOLC(SCC_EventBlock *);
int SCCNetworkChanConnect(SCC_EventBlock *);
int SCCNetworkCLC(SCC_EventBlock *);
int SCCNetworkReleaseComp(SCC_EventBlock *);
int SCCNetworkTCSAck(SCC_EventBlock *);
int SCCNetworkControlConnected(SCC_EventBlock *);
int SCCNetworkGenericMsg(SCC_EventBlock *);
int SCCNetworkProgress(SCC_EventBlock *);

int SCCBridgeSetup(SCC_EventBlock *);
int SCCBridgeAlerting(SCC_EventBlock *);
int SCCBridgeProceeding(SCC_EventBlock *);
int SCCBridgeConnect(SCC_EventBlock *);
int SCCBridgeReleaseComp(SCC_EventBlock *);

int SCCInitiateSetup(SCC_EventBlock *);
int SCCInitiateAlerting(SCC_EventBlock *);
int SCCInitiateProceeding(SCC_EventBlock *);
int SCCInitiateConnect(SCC_EventBlock *);
int SCCInitiateReleaseComp(SCC_EventBlock *);
int SCCInitiateGenericMsg(SCC_EventBlock *);
int SCCInitiateProgress(SCC_EventBlock *);
int SCCLeg2BridgeSetup(SCC_EventBlock *);
int SCCLeg2BridgeCreateCall(SCC_EventBlock *);
int SCCLeg2BridgeMakeCall(SCC_EventBlock *);
int SCCLeg1ConnectCall(SCC_EventBlock *);
int SCCLogEvent(SCC_EventBlock *);
int SCCFreeResources(SCC_EventBlock *);
int SCCNetworkTransportConnected(SCC_EventBlock *);
int SCCLeg2NetworkWORReleaseComp(SCC_EventBlock *);
int SCCBridgeTransportConnected(SCC_EventBlock *);
int SCCBridgeChanConnect(SCC_EventBlock *);
int SCCBridgeOLC(SCC_EventBlock *);
int SCCBridgeCLC(SCC_EventBlock *);
int SCCBridgeTCSAck(SCC_EventBlock *);
int SCCBridgeControlConnected(SCC_EventBlock *);
int SCC_NetworkCallStateIdle(SCC_EventBlock *evtPtr);
int SCC_NetworkDRQ(SCC_EventBlock *evtPtr);
int SCC_ChannelStateConnected(SCC_EventBlock *evtPtr);
int SCCLeg1WORReleaseComp(SCC_EventBlock *);

// if butterfly tell him to shut up
// else not much use. set a flag or something to indicate we have the endpoint
// capability
int SCCNetworkCapability(SCC_EventBlock *);

// if capability not sent already then forward the capability to this leg 
// move to capability sent state
int SCCBridgeCapability(SCC_EventBlock *);

int SCCNetworkRequestMode(SCC_EventBlock *);
int SCCNetworkRequestModeAck(SCC_EventBlock *);
int SCCNetworkRequestModeReject(SCC_EventBlock *);
int SCCBridgeRequestMode(SCC_EventBlock *);
int SCCBridgeRequestModeAck(SCC_EventBlock *);
int SCCBridgeRequestModeReject(SCC_EventBlock *);

int SCC_H323Leg1NetworkEventProcessor(SCC_EventBlock *);
int SCC_H323Leg1BridgeEventProcessor(SCC_EventBlock *);
int SCC_H323Leg2NetworkEventProcessor(SCC_EventBlock *);
int SCC_H323Leg2BridgeEventProcessor(SCC_EventBlock *);



extern SCC_StateMachineEntry SCC_smH323Leg1Network[SCC_sMaxStates][H323MAXNETWORKEVENTS];
extern SCC_StateMachineEntry SCC_smH323Leg1Bridge[SCC_sMaxStates][H323MAXNETWORKEVENTS];
extern SCC_StateMachineEntry SCC_smH323Leg2Network[SCC_sMaxStates][H323MAXNETWORKEVENTS];
extern SCC_StateMachineEntry SCC_smH323Leg2Bridge[SCC_sMaxStates][H323MAXNETWORKEVENTS];

extern char *CallGetErrorStr(int);

#endif /* _callsm_h_ */
