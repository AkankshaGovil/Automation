#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include <malloc.h>

/* Termination timer may be started by action routine "SipInviteServerReTxTimer" if the number
 * of retrans has exceeded limit. Otherwise, termination timer is started when ACK is received 
 * from the network
 */

SipTranSMEntry 
SipInviteSM_Server[SipInviteSM_ServerMaxStates][SipInviteSM_ServerMaxEvents]=
{
	/* SipInvite_ServerIdle */
	{
		/* SipRequestFrNet */ 
		{
			SipInvite_ServerProc,
			{/* SipSendResp, */ SipNotifyCSM2},
		},
		/* Sip1xxFrCSM */
		{
			SipInvite_ServerIdle,
			{SipNop},
		},
		/* Sip2xxFrCSM */
		{
			SipInvite_ServerIdle,
			{SipNop},
		},
		/* Sip4xxFrCSM */
		{
			SipInvite_ServerIdle,
			{SipNop},
		},
		/* SipAckFrNet */
		{
			SipInvite_ServerIdle,
			{SipNop},
		},
		/* SipInviteServerReTxExpire */
		{
			SipInvite_ServerIdle,
			{SipNop},
		},
		/* SipInviteServerTermExpire */
		{
			SipInvite_ServerIdle,
			{SipNop},
		},
	},

	/* SipInvite_ServerProc */
	{
		/* SipRequestFrNet */
		{
			SipInvite_ServerProc,
			{SipSendResp},
		},
		/* Sip1xxFrCSM */
		{
			SipInvite_ServerProc,
			{SipSendResp},
		},
		/* Sip2xxFrCSM */
		{
			SipInvite_ServerSuccess,
			{SipSendResp,SipInviteServerReTxTimer},
		},
		/* Sip4xxFrCSM */
		{
			SipInvite_ServerFailure,
			{SipSendResp,SipInviteServerReTxTimer},
		},
		/* SipAckFrNet */
		{
			SipInvite_ServerProc,
			{SipNop},
		},
		/* SipInviteServerReTxExpire */
		{
			SipInvite_ServerProc,
			{SipNop},
		},
		/* SipInviteServerTermExpire */
		{
			SipInvite_ServerProc,
			{SipNop},
		},
	},

	/* SipInvite_ServerSuccess */
	{
		/* SipRequestFrNet */
		{
			SipInvite_ServerSuccess,
			{SipSendResp},
		},
		/* Sip1xxFrCSM */
		{
			SipInvite_ServerSuccess,
			{SipNop},
		},
		/* Sip2xxFrCSM */
		{
			SipInvite_ServerSuccess,
			{SipNop},
		},
		/* Sip4xxFrCSM */
		{
			SipInvite_ServerSuccess,
			{SipNop},
		},
		/* SipAckFrNet */
		{
			SipInvite_ServerConfirmed,
			{SipAckCSM2,SipDeleteTimer,SipInviteServerTermTimer},
		},
		/* SipInviteServerReTxExpire */
		{
			SipInvite_ServerSuccess,
			{SipSendResp,SipInviteServerReTxTimer},
		},
		/* SipInviteServerTermExpire */
		{
			SipInvite_ServerCompleted,
			{SipErrorCSM, SipRemoveTSM},
		},
	},

	/* SipInvite_ServerFailure */
	{
		/* SipRequestFrNet */
		{
			SipInvite_ServerFailure,
			{SipSendResp},
		},
		/* Sip1xxFrCSM */
		{
			SipInvite_ServerFailure,
			{SipNop},
		},
		/* Sip2xxFrCSM */
		{
			SipInvite_ServerFailure,
			{SipNop},
		},
		/* Sip4xxFrCSM */
		{
			SipInvite_ServerFailure,
			{SipNop},
		},
		/* SipAckFrNet */
		{
			SipInvite_ServerConfirmed,
			{SipAckCSM2,SipDeleteTimer,SipInviteServerTermTimer},
		},
		/* SipInviteServerReTxExpire */
		{
			SipInvite_ServerFailure,
			{SipSendResp,SipInviteServerReTxTimer},
		},
		/* SipInviteServerTermExpire */
		{
			SipInvite_ServerCompleted,
			{SipRemoveTSM},
		},
	},

	/* SipInvite_ServerConfirmed */
	{
		/* SipRequestFrNet */
		{
			SipInvite_ServerConfirmed,
			{SipNop},
		},
		/* Sip1xxFrCSM */
		{
			SipInvite_ServerConfirmed,
			{SipNop},
		},
		/* Sip2xxFrCSM */
		{
			SipInvite_ServerConfirmed,
			{SipNop},
		},
		/* Sip4xxFrCSM */
		{
			SipInvite_ServerConfirmed,
			{SipNop},
		},
		/* SipAckFrNet */
		{
			SipInvite_ServerConfirmed,
			{SipNop},
		},
		/* SipInviteServerReTxExpire */
		{
			SipInvite_ServerConfirmed,
			{SipNop},
		},
		/* SipInviteServerTermExpire */
		{
			SipInvite_ServerCompleted,
			{SipRemoveTSM},
		},
	},
	/* SipInvite_ServerCompleted */
	{
		/* SipRequestFrNet */
		{
			SipInvite_ServerCompleted,
			{SipNop},
		},
		/* Sip1xxFrCSM */
		{
			SipInvite_ServerCompleted,
			{SipNop},
		},
		/* Sip2xxFrCSM */
		{
			SipInvite_ServerCompleted,
			{SipNop},
		},
		/* Sip4xxFrCSM */
		{
			SipInvite_ServerCompleted,
			{SipNop},
		},
		/* SipAckFrNet */
		{
			SipInvite_ServerCompleted,
			{SipNop},
		},
		/* SipInviteServerReTxExpire */
		{
			SipInvite_ServerCompleted,
			{SipNop},
		},
		/* SipInviteServerTermExpire */
		{
			SipInvite_ServerCompleted,
			{SipNop},
		}
	}
};

char SipInviteSM_ServerStates[][32]={
	"SipInvite_ServerIdle", 
	"SipInvite_ServerProc", 
	"SipInvite_ServerSuccess",
	"SipInvite_ServerFailure", 
	"SipInvite_ServerConfirmed", 
	"SipInvite_ServerCompleted",
};

char SipInviteSM_ServerEvents[][32]={
	"SipRequestFrNet", 
	"Sip1xxFrCSM", 
	"Sip2xxFrCSM", 
	"Sip4xxFrCSM",
	"SipAckFrNet", 
	"SipInviteServerReTxExpire", 
	"SipInviteServerTermExpire",
};

