#include <stdio.h>
#include <sys/time.h>
#include "gis.h"
#include "calldefs.h"
#include "nxosd.h"
#include "nxosdtypes.h"
#include "callid.h"

#define LONGLONG_T longlong_t

static char myCallID[CALL_ID_LEN];
char *pseqnum;
LONGLONG_T seqnum;
static long callidip;


void initCallId(long ipaddr)
{
	long now = time(NULL);
	unsigned long *p = (unsigned long *)myCallID;
	memset(myCallID,0, CALL_ID_LEN);
	memcpy(myCallID,&ipaddr,sizeof(ipaddr));	
	*p++ = ipaddr;
	*p++ = now;
	pseqnum = (char *)p;
	seqnum = 0;

	callidip = ipaddr;
}	

void generateCallId(char callID[CALL_ID_LEN])
{
	hrtime_t hrtime;
	pthread_t	threadid;
#if 0
        ++seqnum;
		memcpy(pseqnum,&seqnum,sizeof(LONGLONG_T));
		memcpy(callID,myCallID,CALL_ID_LEN);
#endif
		// ip address 4 bytes
		// thread id 4 bytes
		// hrtime 4 bytes
	hrtime = nx_gethrtime();
	threadid = pthread_self();

	memcpy(callID, (char *)&callidip, 4);
	memcpy(callID+4, (char *)&threadid, 4);
	memcpy(callID+8, (char *)&hrtime, 8);
}

void generateConfId(char confID[CONF_ID_LEN])
{
	generateCallId(confID);
}
