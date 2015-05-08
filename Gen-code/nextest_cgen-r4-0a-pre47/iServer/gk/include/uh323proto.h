#ifndef _UH323PROTO_H_
#define _UH323PROTO_H_
#include "gis.h"

int
GkInitChannelAddress( CallHandle *callHandle, BOOL	origin, unsigned long ip, unsigned short port,int sid);

int genMsdNumber(void);
void initMsdSeed(void);
int h323HuntError(int callError,int cause);
int ise164(char phone[PHONE_NUM_LEN]);

void CloseFceHoles (CallHandle *callHandle);
void CloseFceH245Holes (CallHandle *callHandle);

void saveEgressH323Id(char *confID, char *egressH323Id);
int _handleHookError(HPROTCONN  hConn);

#endif

