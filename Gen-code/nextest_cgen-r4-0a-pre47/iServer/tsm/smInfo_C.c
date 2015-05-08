#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"


SipTranSMEntry
SipInfoSM_Client[SipInfoSM_ClientMaxStates][SipInfoSM_ClientMaxEvents] = 
{
	/* SipInfo_ClientIdle */
	{
	/* SipInfoFromCSM */			{ SipInfo_ClientProc, { SipSendRequest, SipInfoClientReTxTimer }, },
	/* SipInfoClientReTxExpire */	{ SipInfo_ClientIdle, {	SipNop }, },
	/* SipInfoAllSent */			{ SipInfo_ClientIdle, {	SipNop}, },
	/* SipInfo1xxFrNet */			{ SipInfo_ClientIdle, {	SipNop}, },
	/* SipInfoFinalFrNet */			{ SipInfo_ClientIdle, {	SipNop}, },
	/* SipInfoClientTermExpire */	{ SipInfo_ClientIdle, {	SipNop}, }
	},
	/* SipInfo_ClientProc */
	{
	/* SipInfoFromCSM */			{ SipInfo_ClientProc,		{ SipNop }, },
	/* SipInfoClientReTxExpire */	{ SipInfo_ClientProc,		{ SipSendRequest, SipInfoClientReTxTimer }, },
	/* SipInfoAllSent */			{ SipInfo_ClientCompleted,	{ SipInfoNotifyError, SipInfoClientTermTimer }, },
	/* SipInfo1xxFrNet */			{ SipInfo_ClientProc,		{ SipInfoClientRespCSM }, },
	/* SipInfoFinalFrNet */			{ SipInfo_ClientCompleted,	{ SipInfoClientRespCSM,
																  SipDeleteTimer, SipInfoClientTermTimer }, },
	/* SipInfoClientTermExpire */	{ SipInfo_ClientIdle,		{ SipNop}, }
	},
	/* SipInfo_ClientCompleted */
	{
	/* SipInfoFromCSM */			{ SipInfo_ClientCompleted,	{ SipNop }, },
	/* SipInfoClientReTxExpire */	{ SipInfo_ClientCompleted,	{ SipNop }, },
	/* SipInfoAllSent */			{ SipInfo_ClientCompleted,	{ SipNop }, },
	/* SipInfo1xxFrNet */			{ SipInfo_ClientCompleted,	{ SipNop }, },
	/* SipInfoFinalFrNet */			{ SipInfo_ClientCompleted,	{ SipNop }, },
	/* SipInfoClientTermExpire */	{ SipInfo_ClientIdle,		{ SipRemoveTSM }, }
	}
};

char SipInfoSM_ClientStates[][32] = {
	"SipInfo_ClientIdle",
	"SipInfo_ClientProc",
	"SipInfo_ClientCompleted"
};

char SipInfoSM_ClientEvents[][32] = {
	"SipRequestFrCSM",
	"SipInfoClientReTxExpire",
	"SipInfoAllSent",
	"SipInfo1xxFrNet",
	"SipInfoFinalFrNet",
	"SipInfoClientTermExpire"
};
