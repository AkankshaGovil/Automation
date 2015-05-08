#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>
#include <memory.h>
#include <malloc.h>

#include "shm.h"
#include "srvrlog.h"
#include "trigger.h"
#include "readTrigger.h"

#define MSG_SIZE 1024


void
TriggerReconfig (void)
{
  char* fn  = "TriggerReconfig";
  int i = 0;
  TriggerParams *triggerParams = ReadTriggerXML();

  NETDEBUG(MINIT, NETLOG_DEBUG4, ("Trigger: reconfig request received\n"));


  if (triggerParams == NULL)
  {
    NETERROR(MINIT, ("%s: Cannot initialize triggers: no config parameters read\n", fn));
    return;
  }

  fillData(&triggerParams->sip,&(lsMem->routeTriggers.sip));
  fillData(&triggerParams->h323,&(lsMem->routeTriggers.h323));
  SHM_Free(triggerParams);
}

void
fillData(Trigger* input, Trigger* output){
  int i;

  memset(output->noHunt ,0,(sizeof(int) * MAX_TRIGGER));
  for(i=0; i < input->noHuntMax; i++){
    output->noHunt[i] = input->noHunt[i];
 }
  memset(output->hunt ,0,(sizeof(int) * MAX_TRIGGER));
  for(i=0; i < input->huntMax; i++){
    output->hunt[i] = input->hunt[i];
  }

  output->huntMax     = input->huntMax;
  output->noHuntMax   = input->noHuntMax;
  output->huntDefault = input->huntDefault;
}

char*
fillBuffer (char *buffer, int *maxSize, char *fmt, ...)
{
  va_list ap;
  char *ptr;
  char mesg[2*MSG_SIZE] = {0};

  if (buffer == NULL)
    return NULL;

  va_start(ap, fmt);
  vsprintf(mesg, fmt, ap);
  va_end(ap);

  // if mesg length is greater than buffer size, increase the buffer size
  if ((strlen(mesg) + strlen(buffer)) >= *maxSize)
  {
    NETDEBUG(MINIT, NETLOG_DEBUG4, ("fillBuffer: allocating more memory to create the XML file, cursize = %d\n", *maxSize));
    *maxSize *= 2;
    ptr = (char *)realloc(buffer, *maxSize);
    if (ptr == NULL)
    {
      NETERROR(MINIT, ("fillBuffer: Unable to allocate memory for XML string: %s\n", strerror(errno)));
      free(buffer);
      return NULL;
    }
    buffer = ptr;
  }

  return strcat(buffer, mesg);
}




char*
CreateTriggerXML()
{
  char *xml;
  int maxSize, i;
  char *fn = "CreateTriggerXML";
  struct stat stats;

  /* allocate space for the xml message */
  if (stat(TRIGGER_FILENAME, &stats))
  {
    NETERROR(MINIT, ("%s: Cannot stat file %s: %s\n", fn, TRIGGER_FILENAME, strerror(errno)));
    maxSize = 2048;
  } else
    maxSize = 2*stats.st_size;

  xml = (char *)calloc(maxSize, sizeof(char));
  if (xml == NULL)
  {
    NETERROR(MINIT, ("%s: Unable to allocate memory for XML string: %s\n", fn, strerror(errno)));
    return NULL;
  }
  memset(xml,0,maxSize*sizeof(char));

  xml = fillBuffer(xml, &maxSize, "<?xml version=\"1.0\"?>\n");
  xml = fillBuffer(xml, &maxSize, "<!-- Route Triggers version=\"1.0\"-->\n");

  xml = fillBuffer(xml, &maxSize, "<%s >\n", TAG_TRIGGERS);

  xml = fillBuffer(xml, &maxSize, "\t<%s %s=\"%d\" %s=\"%d\" %s=\"%d\">\n", TAG_SIP, ATTR_DEFAULT,lsMem->routeTriggers.sip.huntDefault,ATTR_HUNT_MAX ,lsMem->routeTriggers.sip.huntMax,ATTR_NO_HUNT_MAX,lsMem->routeTriggers.sip.noHuntMax);

  for(i=0; i < lsMem->routeTriggers.sip.huntMax; i++)
    xml = fillBuffer(xml, &maxSize, "\t\t<%s>%d</%s>\n", TAG_HUNT, lsMem->routeTriggers.sip.hunt[i], TAG_HUNT);
  for(i=0; i < lsMem->routeTriggers.sip.noHuntMax; i++)
    xml = fillBuffer(xml, &maxSize, "\t\t<%s>%d</%s>\n", TAG_NOHUNT, lsMem->routeTriggers.sip.noHunt[i], TAG_NOHUNT);
  xml = fillBuffer(xml, &maxSize, "\t</%s> \n", TAG_SIP);

  xml = fillBuffer(xml, &maxSize, "\t<%s %s=\"%d\" %s=\"%d\" %s=\"%d\">\n", TAG_H323, ATTR_DEFAULT,lsMem->routeTriggers.h323.huntDefault,ATTR_HUNT_MAX ,lsMem->routeTriggers.h323.huntMax,ATTR_NO_HUNT_MAX,lsMem->routeTriggers.h323.noHuntMax);
  for(i=0; i < lsMem->routeTriggers.h323.huntMax; i++)
    xml = fillBuffer(xml, &maxSize, "\t\t<%s>%d</%s>\n", TAG_HUNT, lsMem->routeTriggers.h323.hunt[i], TAG_HUNT);
  for(i=0; i < lsMem->routeTriggers.h323.noHuntMax; i++)
    xml = fillBuffer(xml, &maxSize, "\t\t<%s>%d</%s>\n", TAG_NOHUNT, lsMem->routeTriggers.h323.noHunt[i], TAG_NOHUNT);
  xml = fillBuffer(xml, &maxSize, "\t</%s> \n", TAG_H323);
  xml = fillBuffer(xml, &maxSize, "</%s> \n", TAG_TRIGGERS);

  NETDEBUG(MINIT, NETLOG_DEBUG4, ("%s: XML message created: \n%s\n", fn, xml?xml:"NULL"));
  return xml;
}

