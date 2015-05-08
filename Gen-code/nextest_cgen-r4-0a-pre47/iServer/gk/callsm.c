#include "gis.h"
#include "uh323.h"
#include  "arq.h" 
#include "callsm.h"

/************** LEG 1 Network ***********************/


SCC_StateMachineEntry SCC_smH323Leg1Network[SCC_sMaxStates][H323MAXNETWORKEVENTS]=
{
	/****************** SCC_sError ****************/
	{
		/* SCC_eNetworkSetup */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkAlerting */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkConnect */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkControlConnected */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkChanConnected */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkCLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkReleaseComp */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sError, SCCErrorHandler,
		},

	},

	/***************** SCC_sIdle *********************/
	{
		/* SCC_eNetworkSetup */
		{
			SCC_sWaitOnRemote, SCCNetworkSetup,
		},

		/* SCC_eNetworkAlerting */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkConnect */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkControlConnected */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkChanConnected */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkCLC*/
		{
			SCC_sIdle, SCCLogEvent,
		},

		/* SCC_eNetworkReleaseComp */
		{
			// Will receive this event after we do 
			// initiateReleasecomp
			SCC_sIdle, SCCLogEvent,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

	},


	/***************** SCC_sWaitOnRemote *********************/
	{
		/* SCC_eNetworkSetup */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eNetworkAlerting */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eNetworkConnect */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sWaitOnRemote, SCCNetworkTransportConnected,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sWaitOnRemote, SCCNetworkCapability,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sWaitOnRemote, SCCNetworkTCSAck,
		},

		/* SCC_eNetworkControlConnected */
		{
			SCC_sWaitOnRemote, SCCNetworkControlConnected,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sWaitOnRemote, SCCNetworkOLC,
		},

		/* SCC_eNetworkChanConnected */
		{
			SCC_sWaitOnRemote, SCCNetworkChanConnect,
		},

		/* SCC_eNetworkCLC */
		{
			SCC_sWaitOnRemote, SCCLogEvent,
		},

		/* SCC_eNetworkReleaseComp */
		{
			SCC_sIdle, SCCNetworkReleaseComp,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sWaitOnRemote, SCCNetworkRequestMode,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},
	},

	/***************** SCC_sConnected *********************/
	{
		/* SCC_eNetworkSetup */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eNetworkAlerting */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eNetworkConnect */
		{
			//Ignore the event
			SCC_sConnected, SCCLogEvent,
		},

		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sConnected, SCCNetworkTransportConnected,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sConnected, SCCNetworkCapability,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sConnected, SCCNetworkTCSAck,
		},

		/* SCC_eNetworkControlConnected */
		{
			SCC_sConnected, SCCNetworkControlConnected,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sConnected, SCCNetworkOLC,
		},

		/* SCC_eNetworkChanConnected*/
		{
			SCC_sConnected, SCCNetworkChanConnect,
		},

		/* SCC_eNetworkCLC */
		{
			SCC_sConnected, SCCNetworkCLC,
		},


		/* SCC_eNetworkReleaseComp */
		{
			SCC_sIdle, SCCNetworkReleaseComp,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sConnected, SCCNetworkRequestMode,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sConnected, SCCNetworkGenericMsg,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sConnected, SCCErrorHandler,
		},
	},
};


/************** LEG 2 Network ***********************/


//	Actual State Machine States && events 
SCC_StateMachineEntry SCC_smH323Leg2Network[SCC_sMaxStates][H323MAXNETWORKEVENTS] = 
{
	/* SCC_sError */
	{
		/* SCC_eSetup */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_NetworkeAlerting */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sError, SCCErrorHandler,
		},


		/* SCC_eNetworkConnect */
		{
			SCC_sError, SCCErrorHandler,
		},


		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkControlConnected*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkChanConnect */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkCLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkReleaseComp */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sError, SCCErrorHandler,
		},
	},

	/* SCC_sIdle */
	{
		/* SCC_eNetworkSetup */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkAlerting */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkConnect */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkControlConnected*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkChanConnect */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkCLC */
		{
			SCC_sIdle, SCCLogEvent,
		},

		/* SCC_eNetworkReleaseComp */
		{
			// Will receive this event after we do 
			// initiateReleasecomp
			SCC_sIdle, SCCLogEvent,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sIdle, SCCErrorHandler,
		},
	},


	/* SCC_sWaitOnRemote */
	{
		/* SCC_eNetworkSetup */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eNetworkAlerting */
		{
			SCC_sWaitOnRemote, SCCNetworkAlerting,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sWaitOnRemote, SCCNetworkProceeding,
		},

		/* SCC_eNetworkConnect */
		{
			SCC_sConnected, SCCNetworkConnect,
		},

		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sWaitOnRemote, SCCNetworkTransportConnected,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sWaitOnRemote, SCCNetworkCapability,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sWaitOnRemote, SCCNetworkTCSAck,
		},

		/* SCC_eNetworkControlConnected*/
		{
			SCC_sWaitOnRemote, SCCNetworkControlConnected,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sWaitOnRemote, SCCNetworkOLC,
		},

		/* SCC_eNetworkChanConnect*/
		{
			SCC_sWaitOnRemote, SCCNetworkChanConnect,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sWaitOnRemote, SCCNetworkCLC,
		},


		/* SCC_eNetworkReleaseComp */
		{
			SCC_sIdle, SCCLeg2NetworkWORReleaseComp,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sWaitOnRemote, SCCNetworkRequestMode,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sWaitOnRemote, SCCNetworkProgress,
		},
	},

	/* SCC_sConnected */
	{
		/* SCC_eNetworkSetup */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eNetworkAlerting */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eNetworkProceeding */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eNetworkConnect */
		{
			SCC_sConnected, SCCLogEvent,
		},

		/* SCC_eNetworkTransportConnected*/
		{
			SCC_sConnected, SCCNetworkTransportConnected,
		},

		/* SCC_eNetworkCapabilities */
		{
			SCC_sConnected, SCCNetworkCapability,
		},

		/* SCC_eNetworkTCSAck*/
		{
			SCC_sConnected, SCCNetworkTCSAck,
		},

		/* SCC_eNetworkControlConnected*/
		{
			SCC_sConnected, SCCNetworkControlConnected,
		},

		/* SCC_eNetworkOLC */
		{
			SCC_sConnected, SCCNetworkOLC,
		},

		/* SCC_eNetworkChanConnect */
		{
			SCC_sConnected, SCCNetworkChanConnect,
		},

		/* SCC_eNetworkCLC */
		{
			SCC_sConnected, SCCNetworkCLC,
		},

		/* SCC_eNetworkReleaseComp */
		{
			SCC_sIdle, SCCNetworkReleaseComp,
		},

		/* SCC_eNetworkRequestMode */
		{
			SCC_sConnected, SCCNetworkRequestMode,
		},

		/* SCC_eNetworkGenericMsg */
		{
			SCC_sConnected, SCCNetworkGenericMsg,
		},

		/* SCC_eNetworkProgress*/
		{
			SCC_sConnected, SCCErrorHandler,
		},
	},
};


/************** LEG 1 Bridge ***********************/

SCC_StateMachineEntry SCC_smH323Leg1Bridge[SCC_sMaxStates][H323MAXBRIDGEEVENTS] = 
{
	/* SCC_sError */
	{
		/* SCC_eBridgeSetup */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeAlerting */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeConnect */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeControlConnected */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeChanConnect */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeReleaseComp */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sError, SCCErrorHandler,
		},
	},

	/* SCC_sIdle */
	{
		/* SCC_eBridgeSetup */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeAlerting */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeConnect */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeControlConnected*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeChanConnect */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeReleaseComp */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sIdle, SCCErrorHandler,
		},
	},

	/* SCC_sWaitOnRemote */
	{
		/* SCC_eBridgeSetup */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eBridgeAlerting */
		{
			SCC_sWaitOnRemote, SCCInitiateAlerting,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sWaitOnRemote, SCCInitiateProceeding,
		},


		/* SCC_eBridgeConnect */
		{ 
			SCC_sConnected, SCCLeg1ConnectCall,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sWaitOnRemote, SCCBridgeTransportConnected,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sWaitOnRemote, SCCBridgeCapability,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sWaitOnRemote, SCCBridgeTCSAck,
		},

		/* SCC_eBridgeControlConnected*/
		{
			SCC_sWaitOnRemote, SCCBridgeControlConnected,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sWaitOnRemote, SCCBridgeOLC,
		},

		/* SCC_eBridgeChannConnect */
		{
			SCC_sWaitOnRemote, SCCBridgeChanConnect, //SCCLogEvent,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sWaitOnRemote, SCCBridgeCLC,
		},

		/* SCC_eBridgeReleaseComp */
		{
			SCC_sIdle, SCCLeg1WORReleaseComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sWaitOnRemote, SCCBridgeRequestMode,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sWaitOnRemote, SCCInitiateProgress,
		},
	},

	/* SCC_sConnected */
	{
		/* SCC_eBridgeSetup */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eBridgeAlerting */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eBridgeConnect */
		{
			//SCC_sConnected, SCCLeg1ConnectCall,
			SCC_sConnected, SCCLogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sConnected, SCCBridgeTransportConnected,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sConnected, SCCBridgeCapability,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sConnected, SCCBridgeTCSAck,
		},

		/* SCC_eBridgeControlConnected*/
		{
			SCC_sConnected, SCCBridgeControlConnected,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sConnected, SCCBridgeOLC,
		},


		/* SCC_eBridgeChanConnect */
		{
			SCC_sConnected, SCCBridgeChanConnect,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sConnected, SCCBridgeCLC,
		},

		/* SCC_eBridgeReleaseComp */
		{
			SCC_sIdle, SCCInitiateReleaseComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sConnected, SCCBridgeRequestMode,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sConnected, SCCInitiateGenericMsg,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sConnected, SCCErrorHandler,
		},
	},
};

/************** LEG 2 Bridge ***********************/

SCC_StateMachineEntry SCC_smH323Leg2Bridge[SCC_sMaxStates][H323MAXBRIDGEEVENTS]=
{
	/****************** SCC_sError ****************/
	{
		/* SCC_eBridgeSetup */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeAlerting */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeConnect */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeControlConnected*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeChanConnect*/
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeReleaseComp */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sError, SCCErrorHandler,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sError, SCCErrorHandler,
		},
	},

	/***************** SCC_sIdle *********************/
	{
		/* SCC_eBridgeSetup */
		{
			SCC_sWaitOnRemote, SCCLeg2BridgeCreateCall,
		},

		/* SCC_eAlerting */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eConnect */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeControlConnected*/
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeChanConnect */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eReleaseComp */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sIdle, SCCErrorHandler,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sIdle, SCCErrorHandler,
		},
	},


	/* SCC_sWaitOnRemote */
	{
		/* SCC_eSetup */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eAlerting */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eConnect */
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sWaitOnRemote, SCCBridgeTransportConnected,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sWaitOnRemote, SCCBridgeCapability,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sWaitOnRemote, SCCBridgeTCSAck,
		},

		/* SCC_eBridgeControlConnected*/
		{
			SCC_sWaitOnRemote, SCCBridgeControlConnected,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sWaitOnRemote, SCCBridgeOLC,
		},

		/* SCC_eBridgeChanConnect */
		{
			SCC_sWaitOnRemote, SCCLogEvent,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sWaitOnRemote, SCCBridgeCLC,
		},

		/* SCC_eReleaseComp */
		{
			SCC_sIdle, SCCInitiateReleaseComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sWaitOnRemote, SCCBridgeRequestMode,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sWaitOnRemote, SCCInitiateGenericMsg,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sWaitOnRemote, SCCErrorHandler,
		},
	},

	/* SCC_sConnected */
	{
		/* SCC_eSetup */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eAlerting */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eBridgeProceeding */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eConnect */
		{
			SCC_sConnected, SCCErrorHandler,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			SCC_sConnected, SCCBridgeTransportConnected,
		},

		/* SCC_eBridgeCapabilities */
		{
			SCC_sConnected, SCCBridgeCapability,
		},

		/* SCC_eBridgeTCSAck*/
		{
			SCC_sConnected, SCCBridgeTCSAck,
		},

		/* SCC_eBridgeControlConnected*/
		{
			SCC_sConnected, SCCBridgeControlConnected,
		},

		/* SCC_eBridgeOLC */
		{
			SCC_sConnected, SCCBridgeOLC,
		},

		/* SCC_eBridgeChanConnect*/
		{
			SCC_sConnected, SCCBridgeChanConnect,
		},

		/* SCC_eBridgeCLC */
		{
			SCC_sConnected, SCCBridgeCLC,
		},

		/* SCC_eReleaseComp */
		{
			SCC_sIdle, SCCInitiateReleaseComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			SCC_sConnected, SCCBridgeRequestMode,
		},

		/* SCC_eBridgeGenericMsg */
		{
			SCC_sConnected, SCCInitiateGenericMsg,
		},

		/* SCC_eBridgeProgress*/
		{
			SCC_sConnected, SCCErrorHandler,
		},
	},

};
