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

initWaitSdEntry(int maxfds)
{
}

// Server routines
void InsertWaitSdEntry(int sd)
{
}

void DeleteWaitSdEntry(int sd)
{
}

void CleanHalfClose(void)
{
}
