#ifndef _MLSM_H_
#define _MLSM_H_

/******************************************************************************
*	Defines building blocks for SCC FSM - Totally independent of
*	protocol, states and events						
*******************************************************************************/
#include "calldefs.h"
#include "sccdefs.h"


/* Both the leg1 and leg2 state machines use the same entry format */
typedef struct
{
	int					nextState;
	SCC_ActionRoutine	action;
} MLSM_StateMachineEntry;


#endif
