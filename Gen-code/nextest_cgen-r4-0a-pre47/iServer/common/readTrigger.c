#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include "srvrlog.h"
#include "xmlparse.h"
#include "readTrigger.h"
#include "trigger.h"
#include "shm.h"

#define   CONTENT_SIZE  32


 typedef enum {
     PARSER_SIP,
     PARSER_HUNT,
     PARSER_NOHUNT,
     PARSER_H323,
     PARSER_UNKNOWN
 } ParserState;


int huntCount;
int noHuntCount;

/******************** User Data ******************/
/**
 * The following functions are used for translating the entries
 * in from the XML document into the correct format. They are 
 * @param result a pointer to the variable that is the target of the result.
 * @param value a pointer to the XML data value.
 */
struct _UserData 
{

  ParserState state;
  ParserState prevState;
  void* content;
  /* pointer to the trigger structure */
  TriggerParams *triggerParams;

};
typedef struct _UserData UserData;



/**
 * Init the UserData.
 * @param userData the userData structure to be initialised.
 */
void
initializeUserData (UserData *userData) 
{
  userData->state         = PARSER_UNKNOWN;
  userData->prevState     = PARSER_UNKNOWN;
  userData->triggerParams = (TriggerParams *)SHM_Malloc(sizeof(TriggerParams));
  userData->content       = SHM_Malloc(sizeof(char) * CONTENT_SIZE);
  huntCount = 0;
  noHuntCount = 0;

}



/**
 * Retrieve the data between a start and end tag.
 * This data could actually be tags if there is nesting involved.
 * The function startElement sets the flag to indicate whether data
 * is expected.
 * @param ud the User data.
 * @param s the XML data
 * @param len the length of the XML data.
 */
static void
dataHandler (void *ud, const XML_Char *s, int len)
{
  UserData *userData = (UserData*)ud;

  /* If no user data, do nothing. */
  if (userData->state == PARSER_HUNT  ||
      userData->state == PARSER_NOHUNT   
    )
  {
    if(sizeof(userData->content) <= len){
        SHM_Free(userData->content);
      userData->content = SHM_Malloc((sizeof(char) * len)+1);
    }
    memset(userData->content , '\0', len+1);
    strncpy((char*)userData->content,s,len);
  }

}


/**************** Functions for retrieving attributes *******************/

/**
 * This code must match the aravox.xml file. Any mismatches may be trouble.
 * This tag is encountered when the configuration is received.
 * @param userData 
 * @param name 
 * @param nameLen 
 * @param atts 
 */
static void
extractAttributes(UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  int j;
  Trigger *trigger  = NULL;

  if(userData->state == PARSER_SIP){
    trigger = &userData->triggerParams->sip;
    NETDEBUG(MINIT, NETLOG_DEBUG4, ("Extracting SIP attributes \n"));
  }
  else if(userData->state == PARSER_H323){
    trigger = &userData->triggerParams->h323;
    NETDEBUG(MINIT, NETLOG_DEBUG4, ("Extracting H323 attributes \n"));
  }
  if(trigger  ==  NULL)
    return;


  for (j=0; atts[j] != NULL; j+=2)
  {
    if (!strcmp(ATTR_DEFAULT, atts[j]))
    {
      trigger->huntDefault = strtol(atts[j+1], (char **)NULL, 10);
      NETDEBUG(MINIT, NETLOG_DEBUG4, ("default hunt%d\n", trigger->huntDefault));
    }
    else if (!strcmp(ATTR_HUNT_MAX, atts[j]))
    {
      trigger->huntMax = strtol(atts[j+1], (char **)NULL, 10);
      NETDEBUG(MINIT, NETLOG_DEBUG4, ("max hunt%d\n", trigger->huntMax));
      memset(trigger->hunt,0,(sizeof(int) * trigger->huntMax));
    }
    else if (!strcmp(ATTR_NO_HUNT_MAX, atts[j]))
    {
      trigger->noHuntMax  = strtol(atts[j+1], (char **)NULL, 10);
      NETDEBUG(MINIT, NETLOG_DEBUG4, ("max no hunt%d\n", trigger->noHuntMax));
      memset(trigger->noHunt,0,(sizeof(int) * trigger->noHuntMax));
    }
    huntCount   = 0;
    noHuntCount = 0;
  }
}

/*********************** SAX Callbacks ******************/

/**
 * Process the start of an XML element.
 * Any attributes are retrieved at this point. Any data is retrieved by the data handler.
 * @param userData A pointer to user data selected with the XML_SetUserData() call.
 * @param name The name of the tag.
 * @param atts An array of the attributes associated with the tag.
 */
static void
startElement (void *ud, const char *name, const char **atts)
{
  unsigned int nameLen = strlen(name)+1;
  UserData *userData = (UserData*)ud;

  /* Check for the new firewall tag */
  if (!strcmp(TAG_SIP, name))
  {
    NETDEBUG(MINIT, NETLOG_DEBUG4, ("Encountered SIP Triggers"));
    userData->state = PARSER_SIP;
    userData->prevState = PARSER_SIP;
    extractAttributes(userData, name, nameLen, atts);
  }else if(!strcmp(TAG_HUNT, name)){
    userData->state = PARSER_HUNT;
  }else if(!strcmp(TAG_NOHUNT, name)){
    userData->state = PARSER_NOHUNT;
  }else if(!strcmp(TAG_H323, name)){
    NETDEBUG(MINIT, NETLOG_DEBUG4, ("Encountered H323 Triggers"));
    userData->state = PARSER_H323;
    userData->prevState = PARSER_H323;
    extractAttributes(userData, name, nameLen, atts);
  }


}


/**
 * Process the end of an XML element.
 * @param userData A pointer to user data selected with the XML_SetUserData() call.
 * @param name
 */
static void
endElement (void *ud, const char *name)
{
  UserData *userData = (UserData*)ud;

  Trigger *trigger  = NULL;
  if(userData->prevState  ==  PARSER_SIP){
    trigger = &userData->triggerParams->sip;
  }else if(userData->prevState  ==  PARSER_H323){
    trigger = &userData->triggerParams->h323;
  }
  if(trigger  ==  NULL)
    return;
  switch(userData->state){
      case  PARSER_HUNT:
        if (!strcmp(name, TAG_HUNT)){
          trigger->hunt[huntCount++]= strtol((char*)userData->content, (char **)NULL, 10);       
        }
        break;
      case  PARSER_NOHUNT:
        if (!strcmp(name, TAG_NOHUNT)){
          trigger->noHunt[noHuntCount++]= strtol((char*)userData->content, (char **)NULL, 10);       
        }
        break;
      default:
        userData->prevState = PARSER_UNKNOWN;  
        break;
  }
  userData->state = PARSER_UNKNOWN;  
}


void cleanup(UserData *userData){
  if(userData->content  !=  NULL)
    SHM_Free(userData->content);
}

/**
 * Read the triggers xml file.
 * mallocs memory required for the triggers structure, it is up to the caller to
 * eventually free the malloc'ed memory.
 *
 * @return pointer to the Protocol structure containing the information just read
 */
TriggerParams*
ReadTriggerXML ()
{
  char buf[BUFSIZ];
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;
  FILE *file = NULL;
  UserData userData;

  /* open the trigger  file */
  if ((file = fopen(TRIGGER_FILENAME, "r")) == NULL)
  {
    NETERROR(MINIT, ("Cannot open trigger.xml: %s\n", strerror(errno)));
    return NULL;
  }

  initializeUserData(&userData);

  XML_SetUserData(parser, &userData);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, dataHandler);

  do {
    size_t len = fread(buf, 1, sizeof(buf), file);
    done = len < sizeof(buf);
    if (0 == XML_Parse(parser, buf, len, done)) {
      NETERROR(MINIT, ( "Error parsing trigger.xml: %s at line %d\n",
	      XML_ErrorString(XML_GetErrorCode(parser)),
	      XML_GetCurrentLineNumber(parser)));
      return NULL;
    }
  } while (!done);
  XML_ParserFree(parser);

  cleanup(&userData);
  fclose(file);
  return userData.triggerParams;
}


