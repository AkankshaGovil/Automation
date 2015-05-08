#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include <malloc.h>

/* Termination timer is started when first final response is sent out. That response
 * may be generated by TSM (if ALWAYS_... = 1) or by CSM (if ALWAYS_... = 0).
 */

SipTranSMEntry 
SipByeCancelSM_Server[SipByeCancelSM_ServerMaxStates][SipByeCancelSM_ServerMaxEvents]=
{
	/* SipByeCancel_ServerIdle */
	{
#if ALWAYS_RESPOND_WITH_200_TO_BYECANCEL
		/* SipByeCancelFrNet */ 
		{
			SipByeCancel_ServerConfirmed,
			{SipNotifyCSM2, /* SipSendResp, */ SipByeCancelServerTermTimer},
		},
#else
		/* SipByeCancelFrNet */ 
		{
			SipByeCancel_ServerProc,
			{SipNotifyCSM2},
		},
#endif
		/* SipByeCancel1xxFrCSM */
		{
			SipByeCancel_ServerIdle,
			{SipNop},
		},
		/* SipByeCancelFinalFrCSM */
		{
			SipByeCancel_ServerIdle,
			{SipNop},
		},
		/* SipByeCancelServerTermExpire */
		{
			SipByeCancel_ServerIdle,
			{SipNop},
		},
	},
	/* SipByeCancel_ServerProc */
	{
		/* SipByeCancelFrNet */ 
		{
			SipByeCancel_ServerProc,
			{SipSendResp},
		},
		/* SipByeCancel1xxFrCSM */
		{
			SipByeCancel_ServerProc,
			{SipSendResp},
		},
		/* SipByeCancelFinalFrCSM */
		{
			SipByeCancel_ServerConfirmed,
			{SipSendResp, SipByeCancelServerTermTimer},
		},
		/* SipByeCancelServerTermExpire */
		{
			SipByeCancel_ServerCompleted,
			{SipRemoveTSM},
		},
	},
	/* SipByeCancel_ServerConfirmed */
	{
		/* SipByeCancelFrNet */ 
		{
			SipByeCancel_ServerConfirmed,
			{SipSendResp},
		},
		/* SipByeCancel1xxFrCSM */
		{
			SipByeCancel_ServerConfirmed,
			{SipNop},
		},
		/* SipByeCancelFinalFrCSM */
		{
			SipByeCancel_ServerConfirmed,
			{SipSendXferMsg},
		},
		/* SipByeCancelServerTermExpire */
		{
			SipByeCancel_ServerCompleted,
			{SipRemoveTSM},
		},
	},
	/* SipByeCancel_ServerCompleted */
	{
		/* SipByeCancelFrNet */ 
		{
			SipByeCancel_ServerCompleted,
			{SipNop},
		},
		/* SipByeCancel1xxFrCSM */
		{
			SipByeCancel_ServerCompleted,
			{SipNop},
		},
		/* SipByeCancelFinalFrCSM */
		{
			SipByeCancel_ServerCompleted,
			{SipNop},
		},
		/* SipByeCancelServerTermExpire*/
		{
			SipByeCancel_ServerCompleted,
			{SipNop},
		}
	}
};

char SipByeCancelSM_ServerStates[][32]={
	"SipByeCancel_ServerIdle", 
	"SipByeCancel_ServerProc", 
	"SipByeCancel_ServerConfirmed", 
	"SipByeCancel_ServerCompleted",
};

char SipByeCancelSM_ServerEvents[][32]={
	"SipByeCancelFrNet", 
	"SipByeCancel1xxFrCSM", 
	"SipByeCancelFinalFrCSM", 
	"SipByeCancelServerTermExpire",
};
