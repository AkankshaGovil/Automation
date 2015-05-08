#include "gis.h"

#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"

#include "include/tsm.h"
#include <malloc.h>

int SipTranSMProcessor(SipTrans *siptran, SipTranSMEntry *siptransm)
{
	char fn[]="SipTranSMProcessor():";
	int i;

	if(siptransm == NULL || siptran == NULL)
	{
		NETERROR(MSIP, ("%s argument is NULL %p %p\n",fn, siptran, 					siptransm));
		return -1;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s State=%d(%s) Event=%d(%s)\n",fn,
				       siptran->currState, 
				       (siptran->StateName?siptran->StateName:""),
				       siptran->event,
				       (siptran->EventName?siptran->EventName:"")));

	for(i=0;i<SipTranSM_MaxActions;i++)
	{
		if(siptransm->actions[i])
		{
			if( ((siptransm->actions[i]) (siptran)) != 0)
			{
				/* error */
				NETERROR(MSIP, ("%s one action (%d) failed\n",fn, i));
				goto _error;
			}
		}
		else
		{
			break;
		}
	}

	/* all actions successful, update state */
	siptran->currState = siptransm->nextState;
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Leaving\n", fn));
	return 0;

 _error:
	return -1;
} 
