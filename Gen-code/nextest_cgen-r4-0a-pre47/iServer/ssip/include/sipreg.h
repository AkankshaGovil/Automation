#ifndef _sipreg_h_
#define _sipreg_h_

#include "sipcall.h"

// Registration state machines
typedef enum
{
	SipReg_eBridgeRegister = 0,
	SipReg_eNetworkRegister200,
	SipReg_eNetworkRegister3xx,
	SipReg_eNetworkRegisterFinalResponse,
	SipReg_eBridgeRegisterTimer,
	
	SipReg_eMaxEvents,
} SipReg_Events;

typedef enum
{
	SipReg_sIdle = 0,
	SipReg_sNetworkWOR,
	SipReg_sConfirmed,

	SipReg_sMaxStates,
} SipReg_State;

typedef SIP_StateMachineEntry 	SipRegSMEntry;
extern SipRegSMEntry SipRegSM[SipReg_sMaxStates][SipReg_eMaxEvents];

typedef struct 
{
	int 			state;
	
	char 			callID[CALL_ID_LEN];

	SipCallHandle 	sipch;

	// If this Registration is associated with a registration
	// group. The registration group may define the registrar, OBP,
	// q-value for the set of endpoints which are in the reggrp.
	// The configuration of reggrp may be global, defined in server.cfg
	// or derived from database (in which case multiple registration
	// groups may be defined).
	char 			*reggrp;		
	
} SipRegistration;

int SipRegHandleBridgeRegister (SipEventHandle *);
int SipRegHandleBridgeReRegister (SipEventHandle *);
int SipRegHandleSMError (SipEventHandle *);
int SipRegNoOp (SipEventHandle *);
int SipRegHandleNetworkConfirm (SipEventHandle *);
int SipRegHandleNetwork3xx (SipEventHandle *);
int SipRegHandleNetworkFailure (SipEventHandle *);
int SipRegHandleBridgeRegister (SipEventHandle *);
int SipRegHandleBridgeRegisterTimer (SipEventHandle *);

SipRegistration * SipRegistrationNew(int mallocfn);
void SipRegistrationFree(SipRegistration *sipreg, int freefn);

extern int SipRegProcessEvent (SipEventHandle *evHandle);
extern int SipRegChangeState (SipEventHandle *evHandle, 
                              SipAppMsgHandle *appMsgHandle, int *prevState);
extern int SipRegChangeExistingState (SipEventHandle *evHandle, 
                                      SipRegistration *sipreg);
extern int SipRegFindAppCallID (SipMsgHandle *msgHandle, 
                                char *callID);
#endif /* _sipreg_h_ */
