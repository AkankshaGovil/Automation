#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

#include "srvrlog.h"
#include "xmlparse.h"
#include "shm.h"
#include "aravox.h"
#include "readAravox.h"
#include "lsconfig.h"

#define MIN(a,b) ((a<b)?a:b)

/******************** User Data ******************/
/**
 * The following functions are used for translating the entries
 * in from the XML document into the correct format. They are 
 * @param result a pointer to the variable that is the target of the result.
 * @param value a pointer to the XML data value.
 */
struct _UserData 
{
  /* Flag used to indicate if any data is expected. */
  unsigned int dataExpected;

  /* TAG currently being parsed. */
  char *tagName;

  /* Generic value retrieval. */
  void (*retrieveValue)(void *, const XML_Char *) ;

  /* Used to target the resultant address. */
  void *result;

  /* pointer to the current FirewallParam structure */
  FirewallParams *fwParams;

  /* Current index for PPC */
  unsigned int ppcIndex;
};
typedef struct _UserData UserData;


static FirewallParams*
FwParamsAlloc ()
{
  FirewallParams *ptr;
  int i;

  ptr = (FirewallParams *)SHM_Malloc(sizeof(FirewallParams));
  if (ptr == NULL)
    return NULL;

  ptr->firewallConfigId = -1;
  ptr->firewallMPAddr = 0;
  ptr->userId = -1;
  ptr->authType = -1;
  memset(ptr->authString, 0, MAX_ELEM_DATA_LEN);
  ptr->authData = NULL;
  memset(&ptr->authDataBlock, 0, sizeof(nsAuthenticationData));
  ptr->numPPCs = 0;
  for (i = 0; i < MAX_PPC_CARDS; i++)
  {
    ptr->ppc[i].stats = (PPCStats *)SHM_Malloc(sizeof(PPCStats));
    if (ptr->ppc[i].stats == NULL)
      break;
    memset(ptr->ppc[i].stats, 0, sizeof(PPCStats));

    ptr->ppc[i].translationList = malloc(sizeof(AravoxTranslation)); // a dummy header
    if (ptr->ppc[i].translationList == NULL)
    {
      SHM_Free(ptr->ppc[i].stats);
      break;
    }
    memset(ptr->ppc[i].translationList, 0, sizeof(AravoxTranslation));

    ptr->ppc[i].translationList->count = 0;
    ListgInitElem(ptr->ppc[i].translationList, PPC_LIST_OFFSET);

    ptr->ppc[i].signalingEnabled = FALSE;
    ptr->ppc[i].publicAddr = 0;
    ptr->ppc[i].firewallAravoxId = -1;
    ptr->ppc[i].subDeviceId = -1;
    ptr->ppc[i].role = UNKNOWN;
    ptr->ppc[i].adminStatus = PPC_DOWN;
    ptr->ppc[i].operStatus = PPC_DOWN;
    ptr->ppc[i].pFirewall = NULL;
    ptr->ppc[i].portAllocIpAddr = 0;
    ptr->ppc[i].lowPort = 0;
    ptr->ppc[i].highPort = 0;
    ptr->ppc[i].portAlloc = NULL;
  }
  /* check if malloc failed */
  if (i != MAX_PPC_CARDS)
  {
    for (i--; i >= 0; free(ptr->ppc[i].translationList), SHM_Free(ptr->ppc[i--].stats));
    SHM_Free(ptr);
    return NULL;
  }

  ptr->localNatEnabled = FALSE;
  ptr->remoteNatEnabled = FALSE;

  ptr->prevFirewall = NULL;
  ptr->nextFirewall = NULL;

  return ptr;
}

/**
 * deletes the given ptr from the doubly linked list
 *
 * @return either the next item on the list or the previous item on the list or NULL
 */
FirewallParams*
DeleteFwParamOne (FirewallParams *ptr)
{
  FirewallParams *tmp = NULL;
  int i;

  if (ptr->prevFirewall != NULL)
  {
    ptr->prevFirewall->nextFirewall = ptr->nextFirewall;
    tmp = ptr->prevFirewall;
  }
  if (ptr->nextFirewall != NULL)
  {
    ptr->nextFirewall->prevFirewall = ptr->prevFirewall;
    tmp = ptr->nextFirewall;
  }

  for (i = 0; i < MAX_PPC_CARDS; free(ptr->ppc[i].translationList), SHM_Free(ptr->ppc[i++].stats));
  SHM_Free(ptr);

  return tmp;
}

/**
 * deletes the entire doubly linked list, no matter which element in the list is given
 */
void
DeleteFwParamAll (FirewallParams *ptr)
{
  FirewallParams *tmp1 = ptr;
  FirewallParams *tmp2 = NULL;
  int i;

  if (ptr != NULL)
    tmp2 = ptr->prevFirewall;

  /* delete the list on the forward direction */
  while (tmp1 != NULL)
  {
    tmp1 = tmp1->nextFirewall;
    for (i = 0; i < MAX_PPC_CARDS; free(ptr->ppc[i].translationList), SHM_Free(ptr->ppc[i++].stats));
    SHM_Free(ptr);
    ptr = tmp1;
  }

  /* delete the list in the reverse direction */
  ptr = tmp2;
  while (tmp2 != NULL)
  {
    tmp2 = tmp2->prevFirewall;
    for (i = 0; i < MAX_PPC_CARDS; free(ptr->ppc[i].translationList), SHM_Free(ptr->ppc[i++].stats));
    SHM_Free(ptr);
    ptr = tmp2;
  }
}

/**
 * Init the UserData.
 * @param userData the userData structure to be initialised.
 */
void
initUserData (UserData *userData) 
{
  userData->dataExpected = FALSE;
  userData->retrieveValue = NULL;
  userData->result = NULL;
  userData->tagName = NULL;
  userData->ppcIndex = -1;
  userData->fwParams = NULL;
}

/**
 * Reset the UserData. This does not change the 
 * index information stored for the firewalls.
 * @param userData the userData structure to be initialised.
 */
void
resetUserData (UserData *userData) 
{
  userData->dataExpected = FALSE;
  userData->retrieveValue = NULL;
  userData->result = NULL;
  if (userData->tagName)
    free(userData->tagName);
  userData->tagName = NULL;
}

/**
 * Copy the element name into the user data structure.
 * @param userData the structure used to maintain state through the document traversal.
 * @param name the elemetn name.
 * @param nameLen the length of the element name.
 */
inline void
copyTagName (UserData *userData, const char *name, unsigned int nameLen)
{
  userData->tagName = (char *)malloc(nameLen);
  if (userData->tagName != NULL)
  {
    strncpy(userData->tagName, name, nameLen);
    userData->tagName[nameLen-1] = '\0';
  }
  else
  {
    NETERROR(MFCE, ("Unable to allocate memory for TAG name %s\n",name));
  }
}

/**
 * Copy the XML data into an a character array
 * The result MUST be large enought to hold the returned value. The bounds
 * checking is performed before the call to this function.
 * @param result a pointer to a character array to store the result.
 * @param value the XML Chars to be translated.
 */
static void
readCharacters (char *result, const char *value) 
{
  strcpy(result, value);
}

/**
 * Translate the XML data into an IP address in host byte order.
 * @param result a pointer to an unsigned long to store the result.
 * @param value the XML Chars to be translated.
 */
static void
readIPAddress (unsigned long *result, const char *value) 
{
  struct hostent hostentry, *hostp;
  char buffer[256];
  int herror;

  hostp = &hostentry;
  if (gethostbyname_r(value, hostp, buffer, 256, &herror))
  {
    *result = ntohl(*(int *)hostp->h_addr_list[0]);
  }
  else
  {
    *result = 0;
    NETERROR(MFCE, ("Unable to get IP address for %s",value));
  }
}

/**
 * Translate the XML data into an unsigned int.
 * @param result a pointer to an unsigned int to store the result.
 * @param value the XML Chars to be translated.
 */
static void
readUInt (unsigned int *result, const char *value) 
{
  errno = 0;
  *result = (unsigned int)strtol(value,(char**)NULL,10);
  if (errno == ERANGE)
  {
    NETERROR(MFCE, ("Unable to convert value [%s] to int",value));
  }
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
  char *tmpStr = 0;

  /* If no user data, do nothing. */
  if (userData->dataExpected == FALSE)
  {
    return;
  }

  /* Make sure we do not go over our allocated space. */
  len = MIN(len, MAX_ELEM_DATA_LEN-1);

  tmpStr = (char*)calloc(1,len+1);
  if (tmpStr)
  {
    strncpy(tmpStr, s, len);
    tmpStr[len] = '\0';

    (*userData->retrieveValue)(userData->result, tmpStr);

    free(tmpStr);
  }
}

/**
 * Read the value from the XML tag.
 * @param userData the user data passed between the calls.
 * @param name the XML tag
 * @param nameLen
 */
static void
extractCharacters (UserData *userData, const char *name, unsigned int nameLen)
{
  // Useful for debug
  copyTagName(userData, name, nameLen);

  userData->dataExpected = TRUE;
  userData->retrieveValue = (void (*)(void *, const char*))readCharacters;
}

/**
 *
 */
static void
extractIPAddress (UserData *userData, const char *name, unsigned int nameLen)
{
  // Useful for debug
  copyTagName(userData, name, nameLen);

  userData->dataExpected = TRUE;
  userData->retrieveValue = (void (*)(void *, const char*))readIPAddress;
}

/**
 * Get an usigned integer from the document.
 * @param userData the user data structure. Contains element specific values.
 * @param fwParams the target for the actual firewall parameters.
 * @param name the element name.
 * @param nameLen the length of the element name.
 * @param an array containing pairs of the attribute names and values.
 */
static void
extractUInt (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  // Useful for debug
  copyTagName(userData, name, nameLen);

  userData->dataExpected = TRUE;
  userData->retrieveValue = (void (*)(void *, const char*))readUInt;
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
extractFirewallAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  int j;

  for (j=0; atts[j] != NULL; j+=2)
  {
    if (!strcmp(ID_ATTR, atts[j]))
    {
      errno = 0;
      userData->fwParams->firewallConfigId = strtol(atts[j+1], (char **)NULL, 10);
      if (errno != 0 && userData->fwParams->firewallConfigId == 0)
      {
	NETERROR(MFCE, ("Unable to read firewall ID %s\n", atts[j+1]));
      }
      userData->fwParams->numPPCs = 0;
      userData->ppcIndex = 0;
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("extractFirewallAttributes: Reading firewall %d\n", userData->fwParams->firewallConfigId));
      break;
    }
  }
}

/**
 * Get the attributes from the PPC element.
 * Attributes: role
 * @param userData the user data structure. Contains element specific values.
 * @param name the element name.
 * @param nameLen the length of the element name.
 * @param an array containing pairs of the attribute names and values.
 */
static void
extractPPCAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  int j;

  /* fill in the firewall Index */
  userData->fwParams->ppc[userData->ppcIndex].pFirewall = userData->fwParams;

  /* by default the admin status is UP, (for older aravox.xml files) */
  userData->fwParams->ppc[userData->ppcIndex].adminStatus = PPC_UP;

  for (j = 0; atts[j] != 0; j += 2)
  {
    if (!strcmp(ROLE_ATTR, atts[j]))
    {
      if (!strcmp(atts[j+1], MASTER_ROLE))
      {
        userData->fwParams->ppc[userData->ppcIndex].role = MASTER;
      }
      else if (!strcmp(atts[j+1], ACTIVE_ROLE))
      {
        userData->fwParams->ppc[userData->ppcIndex].role = ACTIVE;
      }
      else if (!strcmp(atts[j+1], STANDBY_ROLE))
      {
        userData->fwParams->ppc[userData->ppcIndex].role = STANDBY;
      }
      else 
      {
        userData->fwParams->ppc[userData->ppcIndex].role = UNKNOWN;
      }
    }
    else if (!strcmp(ADMIN_STATUS_ATTR, atts[j]))
    {
      if (!strcmp(atts[j+1], PPC_ADMIN_UP))
      {
	userData->fwParams->ppc[userData->ppcIndex].adminStatus = PPC_UP;
      }
      else
      {
	userData->fwParams->ppc[userData->ppcIndex].adminStatus = PPC_DOWN;
      }
    }
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("extractPPCAttributes: Firewall %d, PPC index %d, FirewallIndex = %d, adminStatus = %d, role = %d\n", userData->fwParams->firewallConfigId, userData->ppcIndex, userData->fwParams->ppc[userData->ppcIndex].pFirewall->firewallConfigId, userData->fwParams->ppc[userData->ppcIndex].adminStatus, userData->fwParams->ppc[userData->ppcIndex].role));
}


/**
 * Get the attributes from the PortAlloc element
 * Attributes: enable, low, high
 * @param userData the user data structure. Contains element specific values.
 * @param name the element name.
 * @param nameLen the length of the element name.
 * @param an array containing pairs of the attribute names and values.
 */
static void
extractPortAllocAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  int j;
  char str[32];

  for (j = 0; atts[j] != 0; j += 2)
  {
    if (!strcmp(IP_ADDRESS_ATTR, atts[j]))
    {
      readIPAddress(&userData->fwParams->ppc[userData->ppcIndex].portAllocIpAddr, atts[j+1]);
    }
    else if (!strcmp(LOW_ATTR, atts[j]))
    {
      userData->fwParams->ppc[userData->ppcIndex].lowPort = strtol(atts[j+1], (char **)NULL, 10);
    }
    else if (!strcmp(HIGH_ATTR, atts[j]))
    {
      userData->fwParams->ppc[userData->ppcIndex].highPort = strtol(atts[j+1], (char **)NULL, 10);
    }
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("extractPortAllocAttributes: ipaddr = %s low = %d, high = %d\n", FormatIpAddress(userData->fwParams->ppc[userData->ppcIndex].portAllocIpAddr, str), userData->fwParams->ppc[userData->ppcIndex].lowPort, userData->fwParams->ppc[userData->ppcIndex].highPort));
}

/**
 * Get the attributes from the Auth element.
 * Attributes: type
 * @param userData the user data structure. Contains element specific values.
 * @param name the element name.
 * @param nameLen the length of the element name.
 * @param an array containing pairs of the attribute names and values.
 */
static void
extractAuthAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  int j;

  for (j=0; atts[j]; j+=2)
  {
    if (!strcmp(TYPE_ATTR, atts[j]))
    {
      if (!strcmp(atts[j+1],MD5_TYPE))
      {
        userData->fwParams->authType = NS_AUTH_MD5;
      }
      else if (!strcmp(atts[j+1],SHA1_TYPE))
      {
        userData->fwParams->authType = NS_AUTH_SHA1;
      }
      else 
      {
        userData->fwParams->authType = NS_AUTH_NONE;
      }
      break;
    }
  }
}

/**
 * Determine if the given flag is set to TRUE.
 * @param userData
 * @param fwPara
 */
static void
isEnableSet (const char **atts, unsigned int *flag)
{
  int j;

  for (j=0; atts[j]; j+=2)
  {
    if (!strcmp(ENABLE_ATTR, atts[j]))
    {
      if (!strcmp(atts[j+1], TRUE_STR))
      {
        *flag = TRUE;
      }
      else 
      {
        *flag = FALSE;
      }
      break;
    }
  }
}

/**
 * Get the attributes from the PacketSteering element.
 * Attributes: enable
 * @param userData the user data structure. Contains element specific values.
 * @param name the element name.
 * @param nameLen the length of the element name.
 * @param an array containing pairs of the attribute names and values.
 */
static void
extractPacketSteeringAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  isEnableSet(atts, &userData->fwParams->remoteNatEnabled);
}

/**
 * Get the attributes from the NAT element.
 * Attributes: enable
 * @param userData the user data structure. Contains element specific values.
 * @param name the element name.
 * @param nameLen the length of the element name.
 * @param an array containing pairs of the attribute names and values.
 */
static void
extractNATAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  isEnableSet(atts, &userData->fwParams->localNatEnabled);
}

/**
 * Get the attributes from the Signaling element.
 * Attributes: enable
 * @param userData the user data structure. Contains element specific values.
 * @param name the element name.
 * @param nameLen the length of the element name.
 * @param an array containing pairs of the attribute names and values.
 */
static void
extractSignalingAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts)
{
  isEnableSet(atts, &userData->fwParams->ppc[userData->ppcIndex].signalingEnabled);
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
  FirewallParams *ptr;
  unsigned int nameLen = strlen(name)+1;

  UserData *userData = (UserData*)ud;

  /* Check for the new firewall tag */
  if (!strcmp(FIREWALL_TAG, name))
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("Encountered new Firewall in the config file"));
    if (userData->fwParams == NULL)
    {
      // first firewall being read
      ptr = FwParamsAlloc();
    }
    else
    {
      // there is atleast one element there already, link a new one in
      ptr = FwParamsAlloc();
      if (ptr != NULL)
      {
	ptr->nextFirewall = userData->fwParams;
	userData->fwParams->prevFirewall = ptr;
      }
    }

    if (ptr == NULL)
    {
      NETERROR(MFCE, ("Cannot allocate memory for FirewallParam: %s\n", strerror(errno)));
      return;
    }
    else
    {
      userData->fwParams = ptr;
    }

    extractFirewallAttributes(userData, name, nameLen, atts);
    return;
  }

  /* if we were unable to malloc memory earlier, we can't proceed.. */
  if (userData->fwParams == NULL)
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("fwParams not allocated, ignoring XML start tag %s\n", name));
    return;
  }

  if (!strcmp(SIGNALING_TAG, name))
  {
    extractSignalingAttributes(userData, name, nameLen, atts);
  }
  else if (!strcmp(IP_ADDRESS_TAG, name))
  { /* MP IP Address. */
    extractIPAddress(userData, name, nameLen);
    userData->result = &userData->fwParams->firewallMPAddr;
  }
  else if (!strcmp(PUBLIC_ADDRESS_TAG, name))
  { /* Public IP Address. */
    extractIPAddress(userData, name, nameLen);
    userData->result = &userData->fwParams->ppc[userData->ppcIndex].publicAddr;
  }
  else if (!strcmp(PORTALLOC_TAG, name))
  { /* private port allocation */
    extractPortAllocAttributes(userData, name, nameLen, atts);
  }
  else if (!strcmp(PPC_TAG, name))
  {
    extractPPCAttributes(userData, name, nameLen, atts);
  }
  else if (!strcmp(ID_TAG, name))
  { /* PPC Id */
    extractUInt(userData, name, nameLen, atts);
    userData->result = &userData->fwParams->ppc[userData->ppcIndex].subDeviceId;
  }
  else if (!strcmp(AUTH_TAG, name))
  {
    // This pulls out the Authentication type
    extractAuthAttributes(userData, name, nameLen, atts);
  }
  else if (!strcmp(USER_ID_TAG, name))
  {
    extractUInt(userData, name, nameLen, atts);
    userData->result = &userData->fwParams->userId;
  }
  else if (!strcmp(AUTH_DATA_TAG, name))
  {
    extractCharacters(userData, name, nameLen);
    userData->result = userData->fwParams->authString;
  }
  else if (!strcmp(NAT_TAG, name))
  {
    extractNATAttributes(userData, name, nameLen, atts);
  }
  else if (!strcmp(PACKET_STEERING_TAG, name))
  {
    extractPacketSteeringAttributes(userData, name, nameLen, atts);
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
  FirewallParams *fwParams = (FirewallParams*)userData->fwParams;

  resetUserData(userData);

  /* Increment the PPC index */
  if (!strcmp(name, PPC_TAG))
  {
    if (++userData->ppcIndex > MAX_PPC_CARDS)
    {
      NETERROR(MFCE, ("Attempting to create more PPCs than the MAX value [%d] on Firewall %d", MAX_PPC_CARDS, fwParams->firewallConfigId));
      --userData->ppcIndex;
      return;
    }
    fwParams->numPPCs++;
    return;
  }

}

/**
 * Read the Aravox config file.
 * mallocs memory required for the configuration structure, it is up to the caller to
 * eventually free the malloc'ed memory.
 *
 * @return pointer to the FirewallParam structure containing the information just read
 */
FirewallParams*
ReadAravoxXML ()
{
  char buf[BUFSIZ];
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;
  FILE *file = NULL;
  UserData userData;
  FirewallParams *tmp, *ptr;

  /* open the config file */
  if ((file = fopen(ARAVOX_CONFIG_FILENAME, "r")) == NULL)
  {
    NETERROR(MFCE, ("Cannot open aravox.xml: %s\n", strerror(errno)));
    return NULL;
  }

  initUserData(&userData);

  XML_SetUserData(parser, &userData);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, dataHandler);

  do {
    size_t len = fread(buf, 1, sizeof(buf), file);
    done = len < sizeof(buf);
    if (0 == XML_Parse(parser, buf, len, done)) {
      NETERROR(MFCE, ( "Error parsing aravox.xml: %s at line %d\n",
	      XML_ErrorString(XML_GetErrorCode(parser)),
	      XML_GetCurrentLineNumber(parser)));
      return FALSE;
    }
  } while (!done);
  XML_ParserFree(parser);

  fclose(file);

  /* validate the configuration */
  tmp = userData.fwParams;
  while (tmp != NULL)
  {
    ptr = tmp;
    tmp = tmp->nextFirewall;
    if (validateAravoxConfig(ptr) == FALSE)
    {
      NETDEBUG(MFCE, NETLOG_DEBUG2, ("ReadAravoxXML: Firewall %d configuration is invalid\n", ptr->firewallConfigId));
      if (ptr == userData.fwParams)
	userData.fwParams = tmp;
      DeleteFwParamOne(ptr);
    }
  }

  return userData.fwParams;
}

