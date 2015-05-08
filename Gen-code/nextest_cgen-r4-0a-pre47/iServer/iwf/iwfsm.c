#include "gis.h"
#include "uh323.h"
#include  "arq.h" 
#include "callsm.h"
#include "iwfsm.h"
#include "iwfsmproto.h"

/************** H323 To Sip Side ***********************/


MLSM_StateMachineEntry IWF_smH3232Sip[IWF_sMaxStates][H323MAXBRIDGEEVENTS] = 
{
	/****************** IWF_sError ****************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sError, 
			iwfH323Ignore ,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sError, 
			iwfH323Ignore,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sError, 
			iwfH323Ignore,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sConnected,
			iwfH323Ignore,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sError,
			iwfH323Ignore,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sError, 
			iwfH323Ignore,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sError, 
			iwfH323Ignore,
		},

	},

	/***************** IWF_sIdle *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sWaitOnSip,
			iwfInitiateInvite,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sIdle,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sIdle,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sIdle,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sIdle,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sIdle,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sIdle,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sIdle, 
			iwfH323LogEvent,
		},
	},


	/***************** IWF_sWaitOnH323 *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sWaitOnH323, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sWaitOnH323,
			iwfAlertingTo1xx,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sWaitOnH323,
			iwfH323Ignore,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sCallConnected,
			iwfConnectTo200Ok,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sWaitOnH323,
			// iwfBridgeTransportConnected,
			// Non Fast Start - Not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sWaitOnH323,
			// iwfBridgeCapability,
			// Non Fast Start - Not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sWaitOnH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sWaitOnH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sWaitOnH323,
			iwfH323Ignore,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sWaitOnH323,
			iwfH323Ignore,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sWaitOnH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError,
			iwfInitiateFinalResponse,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sWaitOnH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sWaitOnH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sWaitOnH323,
			iwfProgressTo1xx,
		},
	},

	/***************** IWF_sWaitOnSip *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sWaitOnSip, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sWaitOnSip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sWaitOnSip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sWaitOnSip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sWaitOnSip,
			// iwfBridgeTransportConnected,
			// Non Fast Start - Not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sWaitOnSip,
			// iwfBridgeCapability,
			// Non Fast Start - Not implementing yet
			iwfRcvBridgeTcs,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sWaitOnSip,
			iwfRcvBridgeTcsAck, //iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sWaitOnSip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sWaitOnSip,
			iwfRcvBridgeOlc,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sWaitOnSip,
			iwfH323Ignore,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sWaitOnSip,
			iwfH323LogEvent,
		},


		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError,
			iwfInitiateCancel,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sWaitOnSip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sWaitOnSip, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sWaitOnSip,
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sCallConnected *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sCallConnected, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sCallConnected, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sCallConnected, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			//Ignore the event
			IWF_sCallConnected, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sCallConnected, 
			// SCCBridgeTransportConnected,
			// for not fast start - not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sCallConnected, 
			// SCCBridgeCapability,
			// for non fast start - not implementing yet
			iwfCallConnectedTCS,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sCallConnected, 
			/* This should be moved into ReInvite once we get televerse clutter fixed */
		//	iwfReInviteTCSAck,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sConnected,
			iwfControlConnected,
		},

		/* SCC_eBridgeOLC */
		{
			/* Needed for hold to work */
			IWF_sCallConnected, 
			iwfReInviteOLC,
		},

		/* SCC_eBridgeChanConnect */
		{
			/* This case arises if we did not have remote rtp in connect event*/
/* This causes problems in case of setup coming from H323 side. We get this 
* event when the fast start channels get connected on h323 side and we send 
* 200Ok. We shud just ignore this event.
*/
			IWF_sCallConnected, 
			iwfH323Ignore,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sCallConnected, // IWF_sAnnexF, 
			iwfCallConnectedCLC, // iwfH323LogEvent, 
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError, 
			iwfInitiateBye,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sCallConnected, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sCallConnected, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sCallConnected, 
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sConnected *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sConnected, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sConnected,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sConnected,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			//Ignore the event
			IWF_sConnected,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sConnected, 
			// SCCBridgeTransportConnected,
			// for not fast start - not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sConnected, 
			// SCCBridgeCapability,
			// for non fast start - not implementing yet
			/* iwfH323LogEvent */
			iwfConnectedNullTCS,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sConnected,
			/* This should be moved into ReInvite once we get televerse clutter fixed */
		//	iwfReInviteTCSAck,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sConnected,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			/* Needed for hold to work */
			IWF_sConnected,
			iwfReInviteOLC,
		},

		/* SCC_eBridgeChanConnect */
		{
			/* This case arises if we did not have remote rtp in connect event*/
/* This causes problems in case of setup coming from H323 side. We get this 
* event when the fast start channels get connected on h323 side and we send 
* 200Ok. We shud just ignore this event.
*/
			IWF_sConnected, 
			iwfChanConnectTo200Ok,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sConnected,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError, 
			iwfInitiateBye,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sReqMode,
			iwfReqMode,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sConnected, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sConnected,
			iwfH323LogEvent,
		},
	},
	
	/***************** IWF_sHeldBySip *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sHeldBySip, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sHeldBySip, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sHeldBySip, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			//Ignore the event
			IWF_sHeldBySip, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sHeldBySip, 
			// SCCBridgeTransportConnected,
			// for not fast start - not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sHeldBySip, 
			// SCCBridgeCapability,
			// for non fast start - not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sHeldBySip,
			iwfHeldBySipTCSAck,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sHeldBySip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sConnected,
			iwfOLC2ChanConnect,
		},

		/* SCC_eBridgeChanConnect */
		{
			/* This case arises if we did not have remote rtp in connect event*/
			IWF_sHeldBySip, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sHeldBySip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError, 
			iwfInitiateBye,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sHeldBySip,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sHeldBySip, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sHeldBySip, 
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sHeldByH323 *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sHeldByH323, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sHeldByH323, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sHeldByH323, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			//Ignore the event
			IWF_sHeldByH323, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sHeldByH323, 
			// SCCBridgeTransportConnected,
			// for not fast start - not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sHeldByH323, 
			// SCCBridgeCapability,
			// for non fast start - not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sHeldByH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected  */
		{
			IWF_sHeldByH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sConnected,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeChanConnect */
		{
			/* This case arises if we did not have remote rtp in connect event*/
			IWF_sConnected, 
			iwfChanConnectTo200Ok,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sHeldByH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError, 
			iwfInitiateBye,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sHeldByH323,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sHeldByH323, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sHeldByH323, 
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sHeldByBoth *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sHeldByBoth, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sHeldByBoth, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sHeldByBoth, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			//Ignore the event
			IWF_sHeldByBoth, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sHeldByBoth, 
			// SCCBridgeTransportConnected,
			// for not fast start - not implementing yet
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sHeldByBoth, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sHeldByBoth,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sHeldByBoth,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sConnected,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeChanConnect */
		{
			/* This case arises if we did not have remote rtp in connect event*/
			IWF_sConnected, 
			iwfChanConnectTo200Ok,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sHeldByBoth,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError, 
			iwfInitiateBye,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sHeldByBoth,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sHeldByH323, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sHeldByBoth, 
			iwfH323LogEvent,
		},
	},
	
	/*********************** IWF_sReInvite ********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sReInvite,
			iwfReInviteBridgeTCS,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sReInvite,
			iwfReInviteTCSAck,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sReInvite,
			iwfReInviteOLC,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sReInvite,
			iwfReInviteOLCAck,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError, 
			iwfReInviteRelComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sReInvite,
			iwfReInviteReqModeAck,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sReInvite, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sReInvite,
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sNonFS200OK *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			/* H323 will generate automatic TCS - else we cud have sent TCS from here */
			IWF_sNonFS200OK,
			iwfCallConnectedTxConnected, // Sonus Null Sdp Hack
		},

		/* SCC_eBridgeCapabilities */
		{
			/* Send OLC */
			IWF_sNonFS200OK,
			iwfCallConnectedTCS,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sNonFS200OK,
			iwfOLC2ChanConnect,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sConnected,
			iwfChanConnectToAck,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sNonFS200OK,
			iwfInitiateAckBye,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sNonFS200OK, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sNonFS200OK,
			iwfH323LogEvent,
		},
	},
	
	/*********************** IWF_sReqMode ********************/
	/* Handle the transitions arising out of ReqMode Event  initiated by 323 */
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sReqMode,
			iwfReqModeOLC,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sConnected,
			iwfReqModeOLCAck,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError, 
			iwfInitiateBye,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sReqMode, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sReqMode,
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sWaitOnSipNewOffer *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sWaitOnSipNewOffer, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323Ignore,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},


		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError,
			iwfReInviteRelComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sWaitOnSipNewOffer, 
			iwfHandleH323Dtmf,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sWaitOnSipNewOffer,
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sNullTCS *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			/* Getting First Non Null TCS here */
			// IWF_sReqMode, 
			// iwfH323LogEvent,
			IWF_sNullTCS,
			iwfNullTCSCapabilitiesRx
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sNullTCS, 
			iwfNullTCSOLC
		//	iwfH323LogEvent, 
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sNullTCS, 
			iwfH323Ignore,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},


		/* SCC_eBridgeReleaseComp */
		{
			IWF_sError,
			//iwfInitiateBye,
			iwfNullTCSReleaseComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sNullTCS, 
			iwfH323LogEvent,
		},
	},

	/***************** IWF_sAnnexF *********************/
	{
		/* SCC_eBridgeSetup */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeAlerting */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeProceeding */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeConnect */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTransportConnected*/
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeCapabilities */
		{
			/* Getting First Non Null TCS here */
			IWF_sReqMode, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeTCSAck */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeControlConnected */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeOLC */
		{
			IWF_sAnnexF, 
			iwfAnnexFOLC, 
		},

		/* SCC_eBridgeChanConnect */
		{
			IWF_sAnnexF, 
			iwfChanConnectTo200Ok,
		},

		/* SCC_eBridgeCLC */
		{
			IWF_sAnnexF, 
			iwfAnnexFCLC,
		},


		/* SCC_eBridgeReleaseComp */
		{
			IWF_sIdle,
			iwfReInviteRelComp,
		},

		/* SCC_eBridgeRequestMode */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		},

		/* SCC_eBridgeGenericMsg */
		{
			IWF_sAnnexF, 
			iwfHandleH323Dtmf,  //iwfH323LogEvent,
			
		},

		/* SCC_eBridgeProgress */
		{
			IWF_sAnnexF, 
			iwfH323LogEvent,
		}
	}
};


/*** Sip To H323 Side. All events coming from Sip side handled here  *******/

MLSM_StateMachineEntry IWF_smSip2H323[IWF_sMaxStates][Sip_eMaxBridgeEvents] = 
{
	/* IWF_sError */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sError,
			iwfSipIgnore,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sError,
			iwfSipIgnore,
		},

		/* Sip_eBridge200 */
		{
			IWF_sError,
			iwfSipIgnore,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sError,
			iwfSipIgnore,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sError,
			iwfSipIgnore,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sError,
			iwfSipIgnore,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfSipIgnore,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sError,
			iwfSipIgnore,
		},
		/* Sip_eBridgeInfo */
		{
			IWF_sError,
			iwfSipIgnore,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sError,
			iwfSipIgnore,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sError,
			iwfSipLogEvent,
		},
	},

	/* IWF_sIdle */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sWaitOnH323,
			iwfInitiateSetup,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},

		/* Sip_eBridge200 */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},
		/* Sip_eBridgeInfo */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sIdle,
			iwfSipIgnore,
		},

		/* Sip_eBridgeError */
		{
			IWF_sIdle,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sIdle,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sIdle,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sIdle,
			iwfSipLogEvent,
		},
	},

	/* IWF_sWaitOnH323 */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sWaitOnH323,
			iwfSipLogEvent,
		},
	},

	/* IWF_sWaitOnSip */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sWaitOnSip,
			iwfSipLogEvent,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sWaitOnSip,
			iwfInitiateAlerting,
		},

		/* Sip_eBridge200 */
		{
			IWF_sCallConnected,
			iwfInitiateConnect,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sWaitOnSip,
			iwfSipLogEvent,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sWaitOnSip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sWaitOnSip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sWaitOnSip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sWaitOnSip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sWaitOnSip,
			iwfSipLogEvent,
		},
	},

	/* IWF_sCallConnected*/
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sCallConnected,
			iwfCallConnectedReInvite,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sCallConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sCallConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeAck */
		{
			IWF_sCallConnected,
			iwfSipAck,
		},

		/* Sip_eBridge3xx  */
		{
			IWF_sCallConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sCallConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sCallConnected,
			iwfCancelInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sCallConnected,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sCallConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sCallConnected,
			iwfSipLogEvent,
		},
	} ,

	/* IWF_sConnected*/
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sReInvite,
			iwfConnectedReInvite,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeAck */
		{
			IWF_sConnected,
			iwfSipAck,
		},

		/* Sip_eBridge3xx  */
		{
			IWF_sConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sConnected,
			iwfCancelInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sConnected,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sConnected,
			iwfSipLogEvent,
		},
	} ,

	/* IWF_sHeldBySip*/
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sReInvite,
			iwfSipHeldReInvite,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sHeldBySip,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sHeldBySip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeAck */
		{
			IWF_sHeldBySip,
			iwfSipAck,
		},

		/* Sip_eBridge3xx  */
		{
			IWF_sHeldBySip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sHeldBySip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sHeldBySip,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sHeldBySip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sHeldBySip,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sHeldBySip,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sHeldBySip,
			iwfSipLogEvent,
		},
	} ,

	/* IWF_sHeldByH323 */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sHeldByH323,
			iwfH323HeldReInvite,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sHeldByH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sHeldByH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeAck */
		{
			IWF_sHeldByH323,
			iwfSipAck,
		},

		/* Sip_eBridge3xx  */
		{
			IWF_sHeldByH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sHeldByH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sHeldByH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sHeldByH323,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sHeldByH323,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sHeldByH323,
			iwfSipLogEvent,
		},
	},

	/* IWF_sHeldByBoth*/
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sHeldBySip,
			iwfBothHeldReInvite,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sHeldByBoth,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sConnected,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeAck */
		{
			IWF_sHeldByBoth,
			iwfSipAck,
		},

		/* Sip_eBridge3xx  */
		{
			IWF_sHeldByBoth,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sHeldByBoth,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sHeldByBoth,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sHeldByBoth,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sHeldByBoth,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sHeldByBoth,
			iwfSipLogEvent,
		},
	},

	/* IWF_sReInvite*/
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sConnected,
			iwfReInvite200Ok,
		},

		/* Sip_eBridgeAck */
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},

		/* Sip_eBridge3xx  */
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sReInvite,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sReInvite,
			iwfSipLogEvent,
		},
	} ,

	/* IWF_sNonFS200OK */
	/* 200 OK Received for Non FastStart setup. Waiting for OLCAck. */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sNonFS200OK,
			iwfSipLogEvent,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sNonFS200OK,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sCallConnected,
			iwfSipLogEvent,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sNonFS200OK,
			iwfSipLogEvent,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sNonFS200OK,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sNonFS200OK,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sNonFS200OK,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sNonFS200OK,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sNonFS200OK,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sNonFS200OK,
			iwfSipLogEvent,
		},
	},

	/*********************** IWF_sReqMode ********************/
	/* Handle the transitions arising out of ReqMode Event  initiated by 323 */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sReqMode,
			iwfSipLogEvent,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sReqMode,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sReqMode,
			iwfReqMode200Ok,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sReqMode,
			iwfSipLogEvent,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sReqMode,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sError,
			iwfReInviteFinalResponse,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sReqMode,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sReqMode,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sReqMode,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sReqMode,
			iwfSipLogEvent,
		},
	},

	/* IWF_sWaitOnSipNewOffer */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sNonFS200OK,
			iwfInitiateConnectOnNewOffer,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sError,
			iwfReInviteFinalResponse,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sWaitOnSipNewOffer,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},
	},

	/* IWF_sNullTCS */
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sNullTCS,
			iwfSipLogEvent,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sNullTCS,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sNullTCS,
			iwfNullTCS200Ok,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sNullTCS,
			iwfSipLogEvent,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sNullTCS,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sError,
			iwfReInviteFinalResponse,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sNullTCS,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeInfo */
		{
			IWF_sWaitOnSipNewOffer,
			iwfHandleSipInfo,
		},

		/* Sip_eBridgeInfoFinalResponse */
		{
			IWF_sWaitOnSipNewOffer,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sError,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeInfo200 */
		{
			IWF_sNullTCS,
			iwfSipLogEvent,
		},
	},

	/* IWF_sAnnexF*/
	{
		/* Sip_eBridgeInvite*/
		{
			IWF_sAnnexF,
			iwfAnnexFReInvite,
		},

		/* Sip_eBridge1xx	 */
		{
			IWF_sAnnexF,
			iwfSipLogEvent,
		},

		/* Sip_eBridge200 */
		{
			IWF_sAnnexF,
			iwfAnnexF200Ok,
		},


		/* Sip_eBridgeAck */
		{
			IWF_sAnnexF,
			iwfSipLogEvent,
		},


		/* Sip_eBridge3xx  */
		{
			IWF_sAnnexF,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeFinalResponse */
		{
			IWF_sIdle,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeBye */
		{
			IWF_sIdle,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgCancel */
		{
			IWF_sIdle,
			iwfSipLogEvent,
		},

		/* Sip_eBridgeError */
		{
			IWF_sIdle,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeCExpired */
		{
			IWF_sIdle,
			iwfInitiateReleaseComp,
		},

		/* Sip_eBridgeNoResponseError */
		{
			IWF_sIdle,
			iwfInitiateReleaseComp,
		}
	}
};
