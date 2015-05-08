#ifndef _callstats_h_
#define _callstats_h_


typedef struct {
	int		setup;
	int		connect;
	int		droppedSetup;
	int		outSetup;
	long	time;
	} IntervalStats;

typedef struct {
	IntervalStats 	*secStat;
	IntervalStats 	*minStat;
	IntervalStats 	*hourStat;
	int             h323PollQ;
	int             h323PoolQ;
	} CallStats;
	
extern IntervalStats *issptr,*ismptr,*ishptr;

#define ST_setup(){ issptr->setup++; ismptr->setup++; ishptr->setup++;}
#define ST_connect(){ issptr->connect++; ismptr->connect++; ishptr->connect++;}
#define ST_outSetup(){ issptr->outSetup++; ismptr->outSetup++; ishptr->outSetup++;}

void callStatsInit(void);

#endif

