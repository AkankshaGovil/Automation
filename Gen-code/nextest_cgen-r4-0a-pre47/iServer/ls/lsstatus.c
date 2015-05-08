#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gis.h"
#include "cdr.h"
#include "lock.h"
#include "nxioctl.h"
#include "scm.h"
#include "ls.h"
#include "thutils.h"

extern int *bridgeinPool, *bridgeinClass;
void *BridgeEndCalls(void *arg);
void *BridgeInitCalls(void *arg);
int iserverPrimary = 1;
extern cvtuple 		*scmcvp;

void 
IserverVipStatus(int status)
{
	if ((status == 0) && iserverPrimary)
	{
		// Secondary
		IServerSecondary();
	}
	else if (!iserverPrimary)
	{
		IServerPrimary();
	}
}

void
IServerPrimary(void)
{
	char fn[] = "IServerPrimary():";

    GisExecuteCmd(GIS_REALM_UP_ALL_CMDID);
	NETINFOMSG(MDEF, ("*** NexTone GIS Server ACTIVE ***\n"));
	iserverPrimary = 1;

	SCM_Signal();

	if (ThreadDispatchAction(bridgeinPool[0],
		bridgeinClass[0],
		BridgeInitCalls, NULL,
		1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59, 1))
	{
		NETERROR(MBRIDGE, ("Error in dispatching BridgeInitCalls\n"));
	}
}

void
IServerSecondary(void)
{
	char fn[] = "IServerSecondary()";

    GisExecuteCmd(GIS_REALM_DOWN_ALL_CMDID);
	NETINFOMSG(MDEF, ("*** NexTone GIS Server INACTIVE ***\n"));

	iserverPrimary = 0;

	SCM_Signal();

	if (ThreadDispatchAction(bridgeinPool[0],
		bridgeinClass[0],
		BridgeEndCalls, NULL,
		1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59, 1))
	{
		NETERROR(MBRIDGE, ("Error in dispatching BridgeEndCalls\n"));
	}
}

