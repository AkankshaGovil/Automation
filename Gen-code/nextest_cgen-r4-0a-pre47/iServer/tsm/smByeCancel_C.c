#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include <malloc.h>

SipTranSMEntry 
SipByeCancelSM_Client[SipByeCancelSM_ClientMaxStates][SipByeCancelSM_ClientMaxEvents]=
{
	/* SipByeCancel_ClientIdle */
	{
		/* SipByeCancelFrCSM */ 
		{
			SipByeCancel_ClientProc,
			{SipSendRequest, SipByeCancelClientReTxTimer},
		},
		/* SipByeCancelClientReTxExpire */
		{
			SipByeCancel_ClientIdle,
			{SipNop},
		},
		/* SipByeCancel11Sent */
		{
			SipByeCancel_ClientIdle,
			{SipNop},
		},
		/* SipByeCancel1xxFrNet */
		{
			SipByeCancel_ClientIdle,
			{SipNop},
		},
		/* SipByeCanceFinalFrNet */
		{
			SipByeCancel_ClientIdle,
			{SipNop},
		},
	},

	/* SipByeCancel_ClientProc */
	{
		/* SipByeCancelFrCSM */ 
		{
			SipByeCancel_ClientProc,
			{SipNop},
		},
		/* SipByeCancelClientReTxExpire */
		{
			SipByeCancel_ClientProc,
			{SipSendRequest, SipByeCancelClientReTxTimer},
		},
		/* SipByeCancel11Sent */
		{
			SipByeCancel_ClientCompleted,
//			{SipRemoveTSM},
			{SipByeCancelErrorCSM2},
		},
		/* SipByeCancel1xxFrNet */
		{
			SipByeCancel_ClientProc,
			{SipByeCancelRespCSM2, SipDeleteTimer, SipByeCancelClientReTxTimer},
		},
		/* SipByeCanceFinalFrNet */
		{
			SipByeCancel_ClientCompleted,
//			{SipByeCancelRespCSM2, SipRemoveTSM},
			{SipByeCancelRespCSM2},
		},
	},

	/* SipByeCancel_ClientCompleted */
	{
		/* SipByeCancelFrCSM */ 
		{
			SipByeCancel_ClientCompleted,
			{SipNop},
		},
		/* SipByeCancelClientReTxExpire */
		{
			SipByeCancel_ClientCompleted,
			{SipByeCancelRemoveTSM},
		},
		/* SipByeCancel11Sent */
		{
			SipByeCancel_ClientCompleted,
			{SipNop},
		},
		/* SipByeCancel1xxFrNet */
		{
			SipByeCancel_ClientCompleted,
			{SipNop},
		},
		/* SipByeCanceFinalFrNet */
		{
			SipByeCancel_ClientCompleted,
			{SipNop},
		}
	}
};

char SipByeCancelSM_ClientStates[][32]={
	"SipByeCancel_ClientIdle", 
	"SipByeCancel_ClientProc",
	"SipByeCancel_ClientCompleted",
};

char SipByeCancelSM_ClientEvents[][32]={
	"SipByeCancelFrCSM", 
	"SipByeCancelClientReTxExpire", 
	"SipByeCancel11Sent",
	"SipByeCancel1xxFrNet", 
	"SipByeCanceFinalFrNet", 
};
