#include "gis.h"
#include "sipcallactions.h"
#include <malloc.h>

// Events can come out of order here,
// as the sm/s are multithreaded. This problem
// needs to be fixed, but can be ciurcumvented,
// by fixing the sm.
SipBridgeSMEntry 
SipBridgeSM[Sip_sMaxStates][Sip_eMaxBridgeEvents] = 
{
	// Sip_sIdle
	{
		// Sip_eBridgeInvite
		{
			Sip_sRingWOR,
			SipHandleBridgeInvite
		},

		// Sip_eBridge1xx
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridge200
		{
			Sip_sConnectedWOR,
			SipHandleBridgeInvite
		},	

		// Sip_eBridgeAck
		{
			Sip_sConnectedWOR,
			SipHandleBridgeInvite
		},

		// Sip_eBridge3xx
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeFinalResponse
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeBye
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeCancel
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeInfo
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeInfoFinalResponse
		{
			Sip_sIdle,
			SipHandleBridgeError
		},
			
		// Sip_eBridgeError
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridge491Expired
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeCExpired
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeSessionExpired
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeNoResponseError
		{
			Sip_sIdle,
			SipHandleBridgeError
		},
		// Sip_eBridgeRefer
		{
			Sip_sIdle,
			SipHandleBridgeError
		},
		// Sip_eBridge202
		{
			Sip_sIdle,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotify
		{
			Sip_sIdle,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotifyResonse
		{
			Sip_sIdle,
			SipHandleBridgeError
		},
	},

	// Sip_sWORRing
	{
		// Sip_eBridgeInvite
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},

		// Sip_eBridge1xx
		{
			Sip_sWORRing,
			SipHandleBridgeAlerting
		},

		// Sip_eBridge200
		{
			/*Sip_sConnecting,*/
			Sip_sConnectedAck,
			SipHandleBridgeConnect
		},	

		// Sip_eBridgeAck
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},

		// Sip_eBridge3xx
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},

		// Sip_eBridgeFinalResponse
		{
			Sip_sConnectedAck,
			//Sip_sIdle,
			SipHandleBridgeFinalResponse,
		},

		// Sip_eBridgeBye
		{
			Sip_sIdle,
			//SipHandleBridgeBye
			SipBridgeError500
		},

		// Sip_eBridgeCancel
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},

		// Sip_eBridgeInfo
		{
			Sip_sWORRing,
			SipBridgeError500
		},

		// Sip_eBridgeInfoFinalResponse
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},

		// Sip_eBridgeError
		{
			Sip_sIdle,
			SipBridgeError408
		},

		// Sip_eBridge491Expired
		{
			Sip_sWORRing,
			SipBridgeRestart491Timer
		},

		// Sip_eBridgeCExpired
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},

		// Sip_eBridgeSessionExpired
		{
			Sip_sWORRing,
			SipBridgeRestartSessionTimer
		},

		// Sip_eBridgeNoResponseError
		{
			Sip_sIdle,
			SipBridgeError504
		},
		// Sip_eBridgeRefer
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},
		// Sip_eBridge202
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotify
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotifyResonse
		{
			Sip_sWORRing,
			SipHandleBridgeError
		},
	},

	// Sip_sRingWOR
	{
		// Sip_eBridgeInvite
		{
			Sip_sRingWOR,
			// SipHandleBridgeError
			SipQueueBridgeInvite
		},

		// Sip_eBridge1xx
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},

		// Sip_eBridge200
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},	

		// Sip_eBridgeAck
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},

		// Sip_eBridge3xx
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},

		// Sip_eBridgeFinalResponse
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},

		// Sip_eBridgeBye
		{
			Sip_sIdle,
			SipHandleBridgeBye
			//SipHandleBridgeCancel
		},

		// Sip_eBridgeCancel
		{
			Sip_sIdle,
			SipHandleBridgeCancel
		},

		// Sip_eBridgeInfo
		{
			Sip_sRingWOR,
			SipHandleBridgeInfo
		},

		// Sip_eBridgeInfoFinalResponse
		{
			Sip_sRingWOR,
			SipHandleBridgeInfoResponse
		},

		// Sip_eBridgeError
		{
			Sip_sIdle,
			SipBridgeErrorCancel
		},

		// Sip_eBridge491Expired
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},

		// Sip_eBridgeCExpired
		{
			Sip_sIdle,
			SipHandleBridgeCExpired
		},

		// Sip_eBridgeSessionExpired
		{
			Sip_sWORRing,
			SipBridgeRestartSessionTimer
		},

		// Sip_eBridgeNoResponseError
		{
			Sip_sIdle,
			SipHandleBridgeError
		},
		// Sip_eBridgeRefer
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},
		// Sip_eBridge202
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotify
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotifyResonse
		{
			Sip_sRingWOR,
			SipHandleBridgeError
		},
	},

	// Sip_sConnecting
	{
		// Sip_eBridgeInvite
		{
			Sip_sConnecting,
			SipQueueNetworkInvite
		},

		// Sip_eBridge1xx
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},

		// Sip_eBridge200
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},	

		// Sip_eBridgeAck
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},

		// Sip_eBridge3xx
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},

		// Sip_eBridgeFinalResponse
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},

		// Sip_eBridgeBye
		{
			Sip_sIdle,
			SipHandleBridgeBye
		},

		// Sip_eBridgeCancel
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},

		// Sip_eBridgeInfo
		{
			Sip_sConnecting,
			SipHandleBridgeInfo
		},

		// Sip_eBridgeInfoFinalResponse
		{
			Sip_sConnecting,
			SipHandleBridgeInfoResponse
		},

		// Sip_eBridgeError
		{
			Sip_sIdle,
			SipBridgeErrorBye
		},

		// Sip_eBridge491Expired
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeCExpired
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeSessionExpired
		{
			Sip_sIdle,
			SipHandleBridgeError
		},

		// Sip_eBridgeNoResponseError
		{
			Sip_sIdle,
			SipBridgeErrorBye
		},
		// Sip_eBridgeRefer
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},
		// Sip_eBridge202
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotify
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotifyResonse
		{
			Sip_sConnecting,
			SipHandleBridgeError
		},
	},	

	// Sip_sConnectedOK
	{
		// Sip_eBridgeInvite
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},

		// Sip_eBridge1xx
		{
			Sip_sConnectedOK,
			//SipHandleBridgeError
			SipHandleBridgeNoOp
		},

		// Sip_eBridge200
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},	

		// Sip_eBridgeAck
		{
			Sip_sConnectedAck,
			SipHandleBridgeAck
		},

		// Sip_eBridge3xx
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},

		// Sip_eBridgeFinalResponse
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},

		// Sip_eBridgeBye
		{
			Sip_sIdle,
			SipHandleBridgeBye
		},

		// Sip_eBridgeCancel
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},

		// Sip_eBridgeInfo
		{
			Sip_sConnectedOK,
			SipHandleBridgeInfo
		},

		// Sip_eBridgeInfoFinalResponse
		{
			Sip_sConnectedOK,
			SipHandleBridgeInfoResponse
		},

		// Sip_eBridgeError
		{
			Sip_sIdle,
			SipBridgeErrorBye
		},

		// Sip_eBridge491Expired
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},

		// Sip_eBridgeCExpired
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},

		// Sip_eBridgeSessionExpired
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},

		// Sip_eBridgeNoResponseError
		{
			Sip_sIdle,
			SipBridgeErrorBye
		},
		// Sip_eBridgeRefer
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},
		// Sip_eBridge202
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotify
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotifyResonse
		{
			Sip_sConnectedOK,
			SipHandleBridgeError
		},
	},	

	// Sip_sConnectedAck
	{
		// Sip_eBridgeInvite
		{
			Sip_sRingWOR,
			SipHandleBridgeReInvite
		},

		// Sip_eBridge1xx
		{
			Sip_sConnectedAck,
			//SipHandleBridgeError
			SipHandleBridgeNoOp
		},

		// Sip_eBridge200
		{
			Sip_sConnectedWOR,
			SipHandleBridgeReInvite
		},	

		// Sip_eBridgeAck
		{
			Sip_sConnectedAck,
			SipHandleBridgeAck
			/*
			Sip_sConnectedWOR,
			SipHandleBridgeReInvite
			*/
		},

		// Sip_eBridge3xx
		{
			Sip_sConnectedAck,
			SipHandleBridgeError
		},

		// Sip_eBridgeFinalResponse
		{
			Sip_sConnectedAck,
			SipHandleBridgeError
		},

		// Sip_eBridgeBye
		{
			Sip_sIdle,
			SipHandleBridgeBye
		},

		// Sip_eBridgeCancel
		{
			Sip_sConnectedAck,
			SipHandleBridgeBye
		},

		// Sip_eBridgeInfo
		{
			Sip_sConnectedAck,
			SipHandleBridgeInfo
		},

		// Sip_eBridgeInfoFinalResponse
		{
			Sip_sConnectedAck,
			SipHandleBridgeInfoResponse
		},

		// Sip_eBridgeError
		{
			Sip_sIdle,
			SipBridgeErrorBye
		},

		// Sip_eBridge491Expired
		{
			Sip_sConnectedAck,
			SipHandleBridge491Expired
		},

		// Sip_eBridgeCExpired
		{
			Sip_sConnectedAck,
			SipHandleBridgeError
		},

		// Sip_eBridgeSessionExpired
		{
			Sip_sConnectedAck,
			SipHandleBridgeSessionExpired
		},

		// Sip_eBridgeNoResponseError
		{
			Sip_sIdle,
			SipBridgeErrorBye
		},
		// Sip_eBridgeRefer
		{
			Sip_sConnectedAck,
			SipHandleBridgeRefer
		},
		// Sip_eBridge202
		{
			Sip_sConnectedAck,
			SipHandleBridge202
		},
		// Sip_eBridgeNotify
		{
			Sip_sConnectedAck,
			SipHandleBridgeNotify
		},
		// Sip_eBridgeNotifyResonse
		{
			Sip_sConnectedAck,
			SipHandleBridgeNotifyResponse
		},
	},	

	// Sip_sConnectedWOR
	{
		// Sip_eBridgeInvite
		{
			Sip_sConnectedWOR,
			SipQueueNetworkInvite
		},

		// Sip_eBridge1xx
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},

		// Sip_eBridge200
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},	

		// Sip_eBridgeAck
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},

		// Sip_eBridge3xx
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},

		// Sip_eBridgeFinalResponse
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},

		// Sip_eBridgeBye
		{
			Sip_sIdle,
			SipHandleBridgeBye
		},

		// Sip_eBridgeCancel
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},

		// Sip_eBridgeInfo
		{
			Sip_sConnectedWOR,
			SipHandleBridgeInfo
		},

		// Sip_eBridgeInfoFinalResponse
		{
			Sip_sConnectedWOR,
			SipHandleBridgeInfoResponse
		},

		// Sip_eBridgeError
		{
			Sip_sIdle,
			SipBridgeErrorCancel
		},

		// Sip_eBridge491Expired
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},

		// Sip_eBridgeCExpired
		{
			Sip_sIdle,
			SipHandleBridgeCExpired
		},

		// Sip_eBridgeSessionExpired
		{
			Sip_sConnectedWOR,
			SipBridgeRestartSessionTimer
		},

		// Sip_eBridgeNoResponseError
		{
			Sip_sIdle,
			SipBridgeErrorCancel
		},
		// Sip_eBridgeRefer
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},
		// Sip_eBridge202
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotify
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},
		// Sip_eBridgeNotifyResonse
		{
			Sip_sConnectedWOR,
			SipHandleBridgeError
		},
	},	

};

