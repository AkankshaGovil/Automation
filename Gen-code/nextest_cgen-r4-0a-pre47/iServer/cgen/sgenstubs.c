#include <stdio.h>

int uh323Globals;
unsigned long   iServerIP;

int getChallenge(void * from, char* callid, void* challenge)
{
	return 0;
}


int sendCredentials(void * from, char* callid, void* credentials)
{

	return 0;
}


int SipCiscoRadiusAuthenticate(void *evb)
{
	return 0;
}

int
setRadiusAccountingSessionId(void *ch)
{
}

int
CallTap(void *callHandle, int status)
{
	return 0;
}

int
BillCall(void *callHandle, int flag)
{
	return 0;
}

int
BillCallPrevCdr(void *callHandle)
{
	return 0;
}

int
BillCallSetFromPrevCdr(void  *callHandle)
{
	return 0;
}

sys_execd () {}
open_execd  () {} 

/*
int
cmMeiEnter()
{
}

int
cmMeiExit()
{
}
*/

int
H323FreeEvent()
{
}

int
freeNodeTree()
{
}

int
UH323Globals()
{
}

int
H323FreeEvData()
{
 return 0;
}

int
CdrArgsFree()
{
 return 0;
}

int
SCMCALL_Replicate()
{
 return 0;
}

int
h323HuntError()
{ 
 return 0;
}

int
SCMCALL_Delete()
{ 
 return 0;
}

int
FreeH323EvtQueue()
{
 return 0;
}
int
SipHandleIncomingNotifyMessage(void *m, char *message, void *context)
{
	return 0;
}
