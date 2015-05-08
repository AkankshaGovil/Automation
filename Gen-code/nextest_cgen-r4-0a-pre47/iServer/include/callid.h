#ifndef _CALLID_H_
#define _CALLID_H_

#include "gis.h"
#include "calldefs.h"

void initCallId(long ipaddr);
void generateCallId(char callID[CALL_ID_LEN]);
void generateConfId(char confID[CONF_ID_LEN]);

#endif /* _CALLID_H_ */
