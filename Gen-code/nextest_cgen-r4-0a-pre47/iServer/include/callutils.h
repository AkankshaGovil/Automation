#ifndef _CALLUTILS_H_
#define _CALLUTILS_H_

#include "gis.h"

int GkCallSetRemoteErrorCodes(CallHandle *callHandle, CallDetails *callDetails);
int GkCallResetRemoteErrorCodes(CallHandle *callHandle);
char *GetSipBridgeEvent (int event);
int IpcEnd (void);
int GkCallSetRemoteErrorCodes (CallHandle *callHandle, CallDetails *callDetails);
int GkCallResetRemoteErrorCodes (CallHandle *callHandle);
CallHandle *CallDelete (cache_t cache, char *callid);
int ConfGetLegs (ConfHandle *confHandle, CallHandle **callHandle1, CallHandle **callHandle2);
char *SCC_BridgeEventToStr (int event, char *str);
char *SCC_EventToStr (int event, char *str);
char *EventToStr (int event, int protocol, char *str);
int getDestCalledPartyNumType (char q931ie);
char * h323InstanceName(int instance);
char * GetSipRegState(int state);
char * GetSipRegEvent(int type, int event);
#endif /* _CALLUTILS_H_ */
