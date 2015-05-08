#include "gis.h"
#include "uh323inc.h"

IntervalStats *issptr,*ismptr,*ishptr;
int	*shmPoolQ;

int	secIndex,minIndex,hrIndex;
int updateCallStatsTime(struct Timer* t);
extern int h323Cps;
extern int h323QLen;

extern int h323inPool;
extern int h323inClass, h323stackinClass;

void callStatsInit(void)
{
	struct timeval tp;
	IntervalStats *pIntervalStats;
	gettimeofday(&tp,NULL);
	secIndex = tp.tv_sec % 60;
	issptr = &lsMem->callStats->secStat[secIndex];

	/* Init the pointers */
	minIndex = (tp.tv_sec/60)%60;
	ismptr = &lsMem->callStats->minStat[minIndex];
	hrIndex = (tp.tv_sec/3600)%24;
	ishptr = &lsMem->callStats->hourStat[hrIndex];
	{
		// start a timer for H.245
		struct itimerval statsTimer;
		
		memset(&statsTimer, 0, sizeof(struct itimerval));
		statsTimer.it_interval.tv_sec = 1;

		timerAddToList(&h323timerPrivate[0], &statsTimer,
			0,PSOS_TIMER_REL, "statsTimer", updateCallStatsTime, NULL);
	}
	shmPoolQ = &lsMem->callStats->h323PoolQ;
}

void updateCallStats(int state,int stateMode)
{
	switch(state)
	{
		case cmCallStateOffering:
			issptr->setup++;
			ismptr->setup++;
			ishptr->setup++;
			break;
		default:
			break;
	}
}
int updateCallStatsTime(struct Timer* t)
{
	struct timeval tp;
	static int lastSecIndex = 0,lastMinIndex = 0;

	gettimeofday(&tp,NULL);
	lastSecIndex = secIndex;
	secIndex = tp.tv_sec % 60;

	issptr = &lsMem->callStats->secStat[secIndex];

	if(issptr->time != tp.tv_sec)
	{
		/* We have wrapped around the seconds buffer */
		memset(issptr,0,sizeof(IntervalStats));
		issptr->time = tp.tv_sec;
		if(secIndex <lastSecIndex)
		{
			/* lets update the minute stats */
			lastMinIndex = minIndex;
			minIndex = (tp.tv_sec/60)%60;
			ismptr = &lsMem->callStats->minStat[minIndex];
			memset(ismptr,0,sizeof(IntervalStats));
			if(minIndex < lastMinIndex)
			{
				hrIndex = (tp.tv_sec/3600)%24;
				ishptr = &lsMem->callStats->hourStat[hrIndex];
				memset(ishptr,0,sizeof(IntervalStats));
			}
		}

	}

	return (0);
}

/* Returns True if cps is exceeded - else returns FALSE */
int CpsExceeded(void)
{
	if(issptr->setup > h323Cps)
	{
		issptr->droppedSetup++;
		ismptr->droppedSetup++;
		ishptr->droppedSetup++;
		return TRUE;
	}
	else if((*shmPoolQ = ThreadPoolGetPending(h323inPool,h323inClass))>h323QLen)
	{
		issptr->droppedSetup++;
		ismptr->droppedSetup++;
		ishptr->droppedSetup++;
		return TRUE;
	}
	return FALSE;
}
