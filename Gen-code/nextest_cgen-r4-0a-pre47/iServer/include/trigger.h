#ifndef _TRIGGER_H
#define _TRIGGER_H

#include "serverp.h"

#define TRIGGER_FILENAME "/usr/local/nextone/bin/trigger.xml"



extern void
TriggerReconfig (void);

extern char*
CreateTriggerXML();
int TriggerInstall (CacheTriggerEntry *tgPtr, CallHandle *callHandle1, 
		CallHandle *callHandle2);

#endif
