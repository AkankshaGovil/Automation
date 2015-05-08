#include "gis.h"
#include "sipcallactions.h"
#include <malloc.h>

SipNetworkSMEntry 
SipNetworkSM[Sip_sMaxStates][Sip_eMaxNetworkEvents] = 
{
	// Sip_sIdle
	{
		// Sip_eNetworkInvite
		{
			Sip_sWORRing,
			SipHandleNetworkInvite
		},

		// Sip_eNetwork1xx
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetwork200
		{
			Sip_sIdle,
			SipHandleNetworkError
		},	

		// Sip_eNetworkAck
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetwork3xx
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetworkFinalResponse
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetworkBye
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetworkCancel
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetworkInfo
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetworkInfoFinalResponse
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetworkError
		{
			Sip_sIdle,
			SipHandleNetworkError
		},

		// Sip_eNetworkNoResponseError
		{
			Sip_sIdle,
			SipHandleNetworkError
		},
		// Sip_eNetworkRefer
		{
			Sip_sIdle,
			SipHandleNetworkError
		},
		// Sip_eNetwork202
		{
			Sip_sIdle,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotify
		{
			Sip_sIdle,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotifyResonse
		{
			Sip_sIdle,
			SipHandleNetworkError
		},
	},

	// Sip_sWORRing
	{
		// Sip_eNetworkInvite
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},

		// Sip_eNetwork1xx
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},

		// Sip_eNetwork200
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},	

		// Sip_eNetworkAck
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},

		// Sip_eNetwork3xx
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},

		// Sip_eNetworkFinalResponse
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},

		// Sip_eNetworkBye
		{
			Sip_sIdle,
			SipHandleNetworkCancel
		},

		// Sip_eNetworkCancel
		{
			Sip_sIdle,
			SipHandleNetworkCancel
		},

		// Sip_eNetworkInfo
		{
			Sip_sWORRing,
			SipHandleNetworkInfo
		},

		// Sip_eNetworkInfoFinalResponse
		{
			Sip_sWORRing,
			SipHandleNetworkInfoResponse
		},

		// Sip_eNetworkError
		{
			Sip_sIdle,
			SipHandleNetworkError500
		},

		// Sip_eNetworkNoResponseError
		{
			Sip_sIdle,
			SipHandleNetworkError500
		},
		// Sip_eNetworkRefer
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},
		// Sip_eNetwork202
		{
			Sip_sWORRing,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotify
		{
			Sip_sWORRing,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotifyResonse
		{
			Sip_sWORRing,
			SipHandleNetworkError
		},
	},

	// Sip_sRingWOR
	{
		// Sip_eNetworkInvite
		{
			Sip_sRingWOR,
			SipHandleNetworkError
		},

		// Sip_eNetwork1xx
		{
			Sip_sRingWOR,
			SipHandleNetworkAlerting
		},

		// Sip_eNetwork200
		{
			/*
			Sip_sConnectedOK,
			SipHandleNetworkConnect
			* Fix this when we are ready to handle inv nosdp
			*/
			Sip_sConnectedAck,
			SipHandleNetworkConnect
		},	

		// Sip_eNetworkAck
		{
			Sip_sRingWOR,
			SipHandleNetworkError
		},

		// Sip_eNetwork3xx
		{
			Sip_sRingWOR,
			SipHandleNetwork3xx
		},

		// Sip_eNetworkFinalResponse
		{
			//Sip_sIdle,
			Sip_sConnectedAck,
			SipHandleNetworkFinalResponse
		},

		// Sip_eNetworkBye
		{
			Sip_sIdle,
			SipHandleNetworkBye
		},

		// Sip_eNetworkCancel
		{
			Sip_sRingWOR,
			SipHandleNetworkError
		},

		// Sip_eNetworkInfo
		{
			Sip_sRingWOR,
			SipHandleNetworkInfo
		},

		// Sip_eNetworkInfoFinalResponse
		{
			Sip_sRingWOR,
			SipHandleNetworkInfoResponse
		},

		// Sip_eNetworkError
		{
			Sip_sIdle,
			SipHandleNetworkErrorCancel
		},

		// Sip_eNetworkNoResponseError
		{
			Sip_sIdle,
			SipHandleNetworkNoResponseError
		},
		// Sip_eNetworkRefer
		{
			Sip_sRingWOR,
			SipHandleNetworkError
		},
		// Sip_eNetwork202
		{
			Sip_sRingWOR,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotify
		{
			Sip_sRingWOR,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotifyResonse
		{
			Sip_sRingWOR,
			SipHandleNetworkError
		},
	},

	// Sip_sConnecting
	{
		// Sip_eNetworkInvite
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},

		// Sip_eNetwork1xx
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},

		// Sip_eNetwork200
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},	

		// Sip_eNetworkAck
		{
			Sip_sConnectedAck,
			SipHandleNetworkAck
		},

		// Sip_eNetwork3xx
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},

		// Sip_eNetworkFinalResponse
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},

		// Sip_eNetworkBye
		{
			Sip_sIdle,
			SipHandleNetworkBye
		},

		// Sip_eNetworkCancel
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},

		// Sip_eNetworkInfo
		{
			Sip_sConnecting,
			SipHandleNetworkInfo
		},

		// Sip_eNetworkInfoFinalResponse
		{
			Sip_sConnecting,
			SipHandleNetworkInfoResponse
		},

		// Sip_eNetworkError
		{
			Sip_sIdle,
			SipHandleNetworkErrorBye
		},

		// Sip_eNetworkNoResponseError
		{
			Sip_sIdle,
			SipHandleNetworkErrorBye
		},
		// Sip_eNetworkRefer
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},
		// Sip_eNetwork202
		{
			Sip_sConnecting,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotify
		{
			Sip_sConnecting,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotifyResonse
		{
			Sip_sConnecting,
			SipHandleNetworkError
		},
	},	

	// Sip_sConnectedOK
	{
		// Sip_eNetworkInvite
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},

		// Sip_eNetwork1xx
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},

		// Sip_eNetwork200
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},	

		// Sip_eNetworkAck
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},

		// Sip_eNetwork3xx
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},

		// Sip_eNetworkFinalResponse
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},

		// Sip_eNetworkBye
		{
			Sip_sIdle,
			SipHandleNetworkBye
		},

		// Sip_eNetworkCancel
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},

		// Sip_eNetworkInfo
		{
			Sip_sConnectedOK,
			SipHandleNetworkInfo
		},

		// Sip_eNetworkInfoFinalResponse
		{
			Sip_sConnectedOK,
			SipHandleNetworkInfoResponse
		},

		// Sip_eNetworkError
		{
			Sip_sIdle,
			SipHandleNetworkErrorBye
		},

		// Sip_eNetworkNoResponseError
		{
			Sip_sIdle,
			SipHandleNetworkErrorBye
		},
		// Sip_eNetworkRefer
		{

			Sip_sConnectedOK,
			SipHandleNetworkError
		},
		// Sip_eNetwork202
		{
			Sip_sConnectedOK,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotify
		{
			Sip_sConnectedOK,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotifyResonse
		{
			Sip_sConnectedOK,
			SipHandleNetworkError
		},
	},	

	// Sip_sConnectedAck
	{
		// Sip_eNetworkInvite
		{
			Sip_sWORRing,
			SipHandleNetworkReInvite
		},

		// Sip_eNetwork1xx
		{
			Sip_sConnectedAck,
			SipHandleNetworkError
		},

		// Sip_eNetwork200
		{
			Sip_sConnectedAck,
			SipHandleNetworkError
		},	

		// Sip_eNetworkAck
		{
			Sip_sConnectedAck,
			SipHandleNetworkAck
		},

		// Sip_eNetwork3xx
		{
			Sip_sConnectedAck,
			SipHandleNetworkError
		},

		// Sip_eNetworkFinalResponse
		{
			Sip_sConnectedAck,
			SipHandleNetworkError
		},

		// Sip_eNetworkBye
		{
			Sip_sIdle,
			SipHandleNetworkBye
		},

		// Sip_eNetworkCancel
		{
			Sip_sConnectedAck,
			SipHandleNetworkBye
		},

		// Sip_eNetworkInfo
		{
			Sip_sConnectedAck,
			SipHandleNetworkInfo
		},

		// Sip_eNetworkInfoFinalResponse
		{
			Sip_sConnectedAck,
			SipHandleNetworkInfoResponse
		},

		// Sip_eNetworkError
		{
			Sip_sIdle,
			SipHandleNetworkErrorBye
		},

		// Sip_eNetworkNoResponseError
		{
			Sip_sIdle,
			SipHandleNetworkErrorBye
		},
		// Sip_eNetworkRefer
		{
			Sip_sConnectedAck,
			SipHandleNetworkRefer
		},
		// Sip_eNetwork202
		{
			Sip_sConnectedAck,
			SipHandleNetwork202			
		},
		// Sip_eNetworkNotify
		{
			Sip_sConnectedAck,
			SipHandleNetworkNotify			
		},
		// Sip_eNetworkNotifyResonse
		{
			Sip_sConnectedAck,
			SipHandleNetworkNotifyResponse
		},
	},	

	// Sip_sConnectedWOR
	{
		// Sip_eNetworkInvite
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError
		},

		// Sip_eNetwork1xx
		{
			Sip_sConnectedWOR,
			SipHandleNetworkAlerting
		},

		// Sip_eNetwork200
		{
			/*
			Sip_sConnectedOK,
			SipHandleNetworkConnect
			* Fix this when we are ready to handle inv nosdp
			*/
			Sip_sConnectedAck,
			SipHandleNetworkConnect
		},	

		// Sip_eNetworkAck
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError
		},

		// Sip_eNetwork3xx
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError
		},

		// Sip_eNetworkFinalResponse
		{
			//Sip_sIdle,
			Sip_sConnectedAck,
			SipHandleNetworkFinalResponse
		},

		// Sip_eNetworkBye
		{
			Sip_sIdle,
			SipHandleNetworkBye
		},

		// Sip_eNetworkCancel
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError
		},

		// Sip_eNetworkInfo
		{
			Sip_sConnectedWOR,
			SipHandleNetworkInfo
		},

		// Sip_eNetworkInfoFinalResponse
		{
			Sip_sConnectedWOR,
			SipHandleNetworkInfoResponse
		},

		// Sip_eNetworkError
		{
			Sip_sIdle,
			SipHandleNetworkErrorCancel
		},

		// Sip_eNetworkNoResponseError
		{
			Sip_sIdle,
			SipHandleNetworkErrorCancel
		},
		// Sip_eNetworkRefer
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError
		},
		// Sip_eNetwork202
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotify
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError			
		},
		// Sip_eNetworkNotifyResonse
		{
			Sip_sConnectedWOR,
			SipHandleNetworkError
		},
	},

};

