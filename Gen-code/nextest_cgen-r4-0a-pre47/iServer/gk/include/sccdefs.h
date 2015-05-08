#ifndef __SCCDEFS_H__
#define __SCCDEFS_H__

/******************************************************************************
*	Defines building blocks for SCC FSM - Totally independent of
*	protocol, states and events						
*******************************************************************************/
#include "calldefs.h"

#define SCCF_BRIDGE_PORTALLOCATED	0x00000001
#define SCCF_BRIDGE_HANDLEALLOCATED	0x00000002

typedef struct
{
	int			event;	/* Event which came in. Using int makes it generic*/
	int			subEvent; /* Sub Event. Meant to be non triggering */
	char		callID[CALL_ID_LEN];
	char		confID[CONF_ID_LEN];
	
	void		*data;
	int			(*evtProcessor)();	/* evt processor fn */

	CallDetails	callDetails;

	unsigned int flags;

} SCC_EventBlock;


/* Action routine returns 0 => no error, follow state m/c
 * returns -1 => error, follow error actions in state m/c
 * returns 1 => not an error, but bypass state m/c
 */
typedef int (*SCC_ActionRoutine) (SCC_EventBlock *);
typedef int (*SCC_EventProcessor) (SCC_EventBlock *);


#define SCC_MAX_ACTIONS	6
#define SCC_MAX_ERRFNS  2

/* Both the leg1 and leg2 state machines use the same entry format */
typedef struct
{
	int					nextState;
	SCC_ActionRoutine	action;
} SCC_StateMachineEntry;


extern char * sccEventBlockToStr(SCC_EventBlock *p, char *);
extern SCC_EventBlock * sccAllocEventBlock(void);
void sccFreeEventBlock(SCC_EventBlock *);
void sccCopyEventBlock (SCC_EventBlock *dest, const SCC_EventBlock *src);

char * GetUh323ControlState(int state);

#endif
