#ifndef _iwfsm_h_
#define _iwfsm_h_

#include "mealeysm.h"
#include "calldefs.h"
#include "sccdefs.h"
#include "sipcall.h"


typedef enum
{
	IWF_sMinState = 0,
	IWF_sError = SCC_sMinState,
	IWF_sIdle,
	IWF_sWaitOnH323,
	IWF_sWaitOnSip,
	IWF_sCallConnected,
	IWF_sConnected,
	IWF_sHeldBySip,
	IWF_sHeldByH323,
	IWF_sHeldByBoth,
	IWF_sReInvite,
	IWF_sNonFS200OK,
	IWF_sReqMode,
	IWF_sWaitOnSipNewOffer,
	IWF_sNullTCS,
	IWF_sAnnexF,
	IWF_sMaxStates,
} IWF_CallState;

/* ReInvite Sub States */
typedef enum
{
	RI_sIdle,
	RI_sWONullTCSAck,
	RI_sWOTCSAck,
	RI_sWOOLCAck,
	RI_sMaxStates,
} RI_State;

typedef enum
{
	media_sConnected,
	media_sNotConnected,
} media_State; 

typedef enum
{
	AnnexF_sAwaiting_CLC_Ack = media_sNotConnected + 1,
	AnnexF_sAwaiting_OLC_Ack,
} AnnexF_State; 

extern MLSM_StateMachineEntry IWF_smH3232Sip[IWF_sMaxStates][H323MAXBRIDGEEVENTS]; 
extern MLSM_StateMachineEntry IWF_smSip2H323[IWF_sMaxStates][Sip_eMaxBridgeEvents];



// Event Processor's for IWF 
int iwfSipEventProcessor(SCC_EventBlock *);
int iwfH323EventProcessor(SCC_EventBlock *);

int iwfSendH323Event (SCC_EventBlock *evtPtr);
int iwfQueueEvent (SCC_EventBlock *evtPtr);

#define IWF_STATE_STR 80

#endif /* _iwfsm_h_ */
