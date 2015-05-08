#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"


SipTranSMEntry
SipInfoSM_Server[SipInfoSM_ServerMaxStates][SipInfoSM_ServerMaxEvents] = 
{
	/* SipInfo_ServerIdle */
	{
	/* SipInfoFrNet */				{ SipInfo_ServerProc, { SipNotifyCSM2 }, },
	/* SipInfo1xxFrCSM */			{ SipInfo_ServerIdle, { SipNop }, },
	/* SipInfoFinalFrCSM */			{ SipInfo_ServerIdle, { SipNop }, },
	/* SipInfoServerTermExpire */	{ SipInfo_ServerIdle, { SipNop }, },
	},
	/* SipInfo_ServerProc */
	{
	/* SipInfoFrNet */				{ SipInfo_ServerProc, { SipNop }, },	/* Our client side does retransmit */
	/* SipInfo1xxFrCSM */			{ SipInfo_ServerProc, { SipSendResp }, },
	/* SipInfoFinalFrCSM  */		{ SipInfo_ServerConfirmed, { SipSendResp,  SipInfoServerTermTimer } },
	/* SipInfoServerTermExpire */	{ SipInfo_ServerProc, { SipNop }, }
	},
	/* SipInfo_ServerConfirmed */
	{
	/* SipInfoFrNet */				{ SipInfo_ServerConfirmed, { SipNop }, },
	/* SipInfo1xxFrCSM */			{ SipInfo_ServerConfirmed, { SipNop }, },
	/* SipInfoFinalFrCSM  */		{ SipInfo_ServerConfirmed, { SipNop } },
	/* SipInfoServerTermExpire */	{ SipInfo_ServerIdle, { SipRemoveTSM }, }
	}
};

char SipInfoSM_ServerStates[][32] = {
	"SipInfo_ServerIdle",
	"SipInfo_ServerProc",
	"SipInfo_ServerConfirmed"
};

char SipInfoSM_ServerEvents[][32] = {
	"SipInfoFrNet",
	"SipInfo1xxFrCSM",
	"SipInfoFinalFrCSM",
	"SipInfoServerTermExpire"
};

