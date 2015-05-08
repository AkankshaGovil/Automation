#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "uh323.h"
#include "systemlog.h"
#include "seli.h"
extern time_t timeNow;
static int waitlistsize;

typedef struct {
	int sd;
	time_t time;
	int instance;
} WaitSdEntry;

//typedef WaitSdEntry * WaitSdEntryPtr;
WaitSdEntry *waitlist;

void
initWaitSdEntry(int maxfds)
{
	int i;
	waitlist = (WaitSdEntry *) malloc(maxfds*sizeof(WaitSdEntry));	
	memset(waitlist, 0 , maxfds*sizeof(WaitSdEntry));
	waitlistsize = maxfds;
}

// Server routines
void InsertWaitSdEntry(int sd)
{
	static char fn[] = "InsertWaitSdEntry";
	if( sd<0 || sd >= waitlistsize)
	{
		NETERROR(MH323,("%s sd out of range %d\n",fn,sd));
		return;
	}
	waitlist[sd].sd = sd;
	waitlist[sd].instance = UH323Globals()->instance;
	waitlist[sd].time = timeNow;
	
}

void DeleteWaitSdEntry(int sd)
{
	static char fn[] = "DeleteWaitSdEntry";
	if( sd<0 || sd >= waitlistsize)
	{
		NETERROR(MH323,("%s sd out of range %d\n",fn,sd));
		return;
	}
	waitlist[sd].time = 0;
}

void CleanHalfClose(void)
{
	int i;
	for (i = 0; i<waitlistsize;i++)
	{
		if((UH323Globals()->instance == waitlist[i].instance) && 
			(waitlist[i].time) && ((timeNow - waitlist[i].time) > 20))
		{
			waitlist[i].time = 0;
			liCallback(waitlist[i].sd,seliEvRead,0);	
		}
	}
 }
