#ifndef _READTRIGGER_H
#define _READTRIGGER_H

#include "trigger.h"

#define TRIGGER_FILENAME "/usr/local/nextone/bin/trigger.xml"
#define TAG_TRIGGERS "Triggers"
#define TAG_SIP     "SIP"
#define TAG_H323     "H323"
#define TAG_HUNT    "Hunt"
#define TAG_NOHUNT  "NoHunt"

#define ATTR_DEFAULT          "default"
#define ATTR_HUNT_MAX     "hunt_max"
#define ATTR_NO_HUNT_MAX  "no_hunt_max"
extern TriggerParams* ReadTriggerXML ();

#endif
