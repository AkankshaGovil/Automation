#include "gis.h"
#include <malloc.h>

SipRegSMEntry 
SipRegSM[SipReg_sMaxStates][SipReg_eMaxEvents] = 
{
	// SipReg_sIdle
	{
		// SipReg_eBridgeRegister
		{
			SipReg_sNetworkWOR,
			SipRegHandleBridgeRegister
		},

		// SipReg_eNetworkRegister2xx
		{
			SipReg_sIdle,
			SipRegHandleSMError
		},

		// SipReg_eNetworkRegister3xx
		{
			SipReg_sIdle,
			SipRegHandleSMError
		},

		// SipReg_eNetworkRegisterFinalResponse
		{
			SipReg_sIdle,
			SipRegHandleSMError
		},

		// SipReg_eBridgeRegisterTimer
		{
			SipReg_sIdle,
			SipRegHandleSMError
		},
	},

	// SipReg_sNetworkWOR
	{
		// SipReg_eBridgeRegister
		{
			SipReg_sNetworkWOR,
			SipRegNoOp
		},

		// SipReg_eNetworkRegister2xx
		{
			SipReg_sConfirmed,
			SipRegHandleNetworkConfirm
		},

		// SipReg_eNetworkRegister3xx
		{
			SipReg_sNetworkWOR,
			SipRegHandleNetwork3xx
		},

		// SipReg_eNetworkRegisterFinalResponse
		{
			SipReg_sIdle,
			SipRegHandleNetworkFailure
		},

		// SipReg_eBridgeRegisterTimer
		{
			SipReg_sNetworkWOR,
			SipRegNoOp
		},
	},

	// SipReg_sConfirmed
	{
		// SipReg_eBridgeRegister
		{
			SipReg_sNetworkWOR,
			SipRegHandleBridgeReRegister
		},

		// SipReg_eNetworkRegister2xx
		{
			SipReg_sConfirmed,
			SipRegNoOp
		},

		// SipReg_eNetworkRegister3xx
		{
			SipReg_sConfirmed,
			SipRegNoOp
		},

		// SipReg_eNetworkRegisterFinalResponse
		{
			SipReg_sConfirmed,
			SipRegNoOp
		},

		// SipReg_eBridgeRegisterTimer
		{
			SipReg_sConfirmed,
			SipRegHandleBridgeRegisterTimer
		},
	},
};
