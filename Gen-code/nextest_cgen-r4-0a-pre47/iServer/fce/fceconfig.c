#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "lock.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "vnsp.h"
#include "firewallcontrol.h"
#include "aravox.h"
#include "readAravox.h"
#include <malloc.h>


NetworkList *nlListHead = NULL;
Lock fceLock;
static int mutexInited = 0;


int
CheckFwLock ()
{
  /* initialize the mutex if first time */
  if (lsMem->fwLock == NULL)
  {
    lsMem->fwLock = (Lock *)SHM_Malloc(sizeof(Lock));
    if (LockInit(lsMem->fwLock, 1))
    {
      NETERROR(MFCE, ("CheckFwLock: Cannot initialize aravox mutex: %s\n", strerror(errno)));
      return -1;
    }
  }

  return 0;
}

int
FCEMutexInit ()
{
  int retval = 0;

  if (mutexInited == 0)
  {
    retval = LockInit(&fceLock, 0);
    if (retval == 0)
      mutexInited = 1;
  }

  return retval;
}


void
FCEMutexDestroy ()
{
  if (mutexInited != 0)
  {
    LockDestroy(&fceLock);
    mutexInited = 0;
  }
}


static void
freePnList ()
{
  NetworkList *tmp;

  LockGetLock(&fceLock, 0, 0);

  while (nlListHead != NULL)
  {
    tmp = nlListHead;
    nlListHead = nlListHead->next;
    free(tmp);
  }

  LockReleaseLock(&fceLock);
}


static void
addPnList (char *str, int isPublic)
{
  NetworkList *ptr;
  char addr[64] = {0}, mask[64] = {0};
  int index, len;
  unsigned int value;

  /* extract values from the given string */
  len = strlen(str);
  for (index = 0; index < len && str[index] != '/'; index++);
  if (index == len)
  {
    /* no '/' found */
    NETERROR(MFCE, ("addPnList: Invalid network list configuration: %s\n", str));
    return;
  }

  memcpy(addr, str, index);
  if ((index+1) < len)
    memcpy(mask, &str[index+1], len-index-1);
  if (strlen(addr) == 0 || strlen(mask) == 0)
  {
    NETERROR(MFCE, ("addPnList: Invalid network list configuration: %s\n", str));
    return;
  }

  /* allocate space for the new element */
  ptr = (NetworkList *)malloc(sizeof(NetworkList));
  if (ptr == NULL)
  {
    NETERROR(MFCE, ("addPnList: Cannot allocate list element for network list (%s) [%d]\n", str, errno));
    return;
  }
  if (inet_pton(AF_INET, addr, &value) <= 0)
  {
    NETERROR(MFCE, ("addPnList: Address %s is in invalid format\n", addr));
    free(ptr);
    return;
  }
  else
    ptr->addr = ntohl(value);
  if (inet_pton(AF_INET, mask, &value) <= 0)
  {
    NETERROR(MFCE, ("addPnList: Address %s is in invalid format\n", addr));
    free(ptr);
    return;
  }
  else
    ptr->mask = ntohl(value);
  ptr->isPublic = isPublic;
  ptr->next = NULL;

  LockGetLock(&fceLock, 0, 0);

  /* link to the current list */
  if (nlListHead == NULL)
    nlListHead = ptr;
  else
  {
    ptr->next = nlListHead;
    nlListHead = ptr;
  }

  LockReleaseLock(&fceLock);
}


/**
 * this stores that list of known private network addresses
 *
 * @param str the string that has network/mask in xx.xx.xx.xx/xx.xx.xx.xx format
 * or NULL if the current list has to be deleted
 * @param isPublic If TRUE the given network is in the public space, else in private
 * space
 */
void
FCENetworkListConfig (char *str, int isPublic)
{
  /* this could happen sometimes even before FCEStart is called */
  if (FCEMutexInit())
  {
    NETERROR(MFCE, ("cannot initialize FCE mutex\n"));
    return;
  }

  if (str == NULL)
  {
    freePnList();
  }
  else
  {
    addPnList(str, isPublic);
  }
}


int
FCEListNetworkList (int (*listFunc)(int, int, int, void*, void*),
		    void *passAlong1,
		    void *passAlong2)
{
  NetworkList *ptr;

  LockGetLock(&fceLock, 0, 0);
  for (ptr = nlListHead; ptr != NULL; ptr = ptr->next)
  {
    if (listFunc(ptr->addr, ptr->mask, ptr->isPublic, passAlong1, passAlong2)) {
      LockReleaseLock(&fceLock);
      return -1;
    }
  }

  LockReleaseLock(&fceLock);
  return 0;
}


static char*
GetAuthTypeString (int authType)
{
  if (authType == NS_AUTH_MD5)
    return MD5_TYPE;
  else if (authType == NS_AUTH_SHA1)
    return SHA1_TYPE;

  return NONE_TYPE;
}


char*
GetPPCStatusString (tPPCStatus status)
{
  return (status == PPC_UP)?PPC_ADMIN_UP:PPC_ADMIN_DOWN;
}


static char*
GetRoleString (tROLE role)
{
  if (role == MASTER)
    return MASTER_ROLE;
  else if (role == ACTIVE)
    return ACTIVE_ROLE;
  else if (role == STANDBY)
    return STANDBY_ROLE;

  return UNKNOWN_ROLE;
}


static char*
GetFailCodeString (int code, char *txt, int sizeOfTxt)
{
#ifndef I86PC
  if (nsVoipGetStatusCodeString(code, txt) == TRUE)
  {
    txt[sizeOfTxt-1] = '\0';
    return txt;
  }
#endif

  txt[0] = 'N';
  txt[1] = 'A';
  txt[2] = '\0';
  return txt;
}


#define STATUS_MESG_SIZE 1024
static char*
fillBuffer (char *buffer, int *maxSize, char *fmt, ...)
{
  va_list ap;
  char *ptr;
  char mesg[2*STATUS_MESG_SIZE] = {0};

  if (buffer == NULL)
    return NULL;

  va_start(ap, fmt);
  vsprintf(mesg, fmt, ap);
  va_end(ap);

  // if mesg length is greater than buffer size, increase the buffer size
  if ((strlen(mesg) + strlen(buffer)) >= *maxSize)
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("fillBuffer: allocating more memory to create the XML file, cursize = %d\n", *maxSize));
    *maxSize *= 2;
    ptr = (char *)realloc(buffer, *maxSize);
    if (ptr == NULL)
    {
      NETERROR(MFCE, ("fillBuffer: Unable to allocate memory for XML string: %s\n", strerror(errno)));
      free(buffer);
      return NULL;
    }
    buffer = ptr;
  }

  return strcat(buffer, mesg);
}

char*
FCECreateXMLFromConfig ()
{
  char *xml;
  int maxSize, i;
  char *fn = "FCECreateXMLFromConfig";
  struct stat stats;
  FirewallParams *fw;
  char ipstr[16];
  char statusMesg[STATUS_MESG_SIZE] = {0};

  /* allocate space for the xml message */
  if (stat(ARAVOX_CONFIG_FILENAME, &stats))
  {
    NETERROR(MFCE, ("%s: Cannot stat file %s: %s\n", fn, ARAVOX_CONFIG_FILENAME, strerror(errno)));
    maxSize = 2048;
  } else
    maxSize = 2*stats.st_size;

  xml = (char *)calloc(maxSize, sizeof(char));
  if (xml == NULL)
  {
    NETERROR(MFCE, ("%s: Unable to allocate memory for XML string: %s\n", fn, strerror(errno)));
    return NULL;
  }

  if (CheckFwLock()) {
    free(xml);
    return NULL;
  }

  xml = fillBuffer(xml, &maxSize, "<?xml version=\"1.0\"?>\n");
  xml = fillBuffer(xml, &maxSize, "<%s version=\"1.0\">\n", ARAVOXCFG_TAG);

  LockGetLock(lsMem->fwLock, 0, 0);
  for (fw = (FirewallParams *)lsMem->fwParams; fw != NULL; fw = fw->nextFirewall)
  {
    xml = fillBuffer(xml, &maxSize, "<%s %s=\"%d\">\n", FIREWALL_TAG, ID_ATTR, fw->firewallConfigId);

    xml = fillBuffer(xml, &maxSize, "\t<%s>%s</%s>\n", IP_ADDRESS_TAG, FormatIpAddress(fw->firewallMPAddr, ipstr), IP_ADDRESS_TAG);

    xml = fillBuffer(xml, &maxSize, "\t<%s %s=\"%s\">\n", AUTH_TAG, TYPE_ATTR, GetAuthTypeString(fw->authType));
    xml = fillBuffer(xml, &maxSize, "\t\t<%s>%d</%s>\n", USER_ID_TAG, fw->userId, USER_ID_TAG);
    xml = fillBuffer(xml, &maxSize, "\t\t<%s>%s</%s>\n", AUTH_DATA_TAG, fw->authString, AUTH_DATA_TAG);
    xml = fillBuffer(xml, &maxSize, "\t</%s>\n", AUTH_TAG);

    xml = fillBuffer(xml, &maxSize, "\t<%s %s=\"%s\" />\n", NAT_TAG, ENABLE_ATTR, fw->localNatEnabled?TRUE_STR:FALSE_STR);

    xml = fillBuffer(xml, &maxSize, "\t<%s %s=\"%s\" />\n", PACKET_STEERING_TAG, ENABLE_ATTR, fw->remoteNatEnabled?TRUE_STR:FALSE_STR);

    for (i = 0; i < fw->numPPCs; i++)
    {
      PPCParams *ppc = &fw->ppc[i];
      PPCStats *stats = ppc->stats;
      xml = fillBuffer(xml, &maxSize, "\t<%s %s=\"%s\" %s=\"%s\">\n", PPC_TAG, ROLE_ATTR, GetRoleString(ppc->role), ADMIN_STATUS_ATTR, GetPPCStatusString(ppc->adminStatus));
      xml = fillBuffer(xml, &maxSize, "\t\t<%s>%d</%s>\n", ID_TAG, ppc->subDeviceId, ID_TAG);
      xml = fillBuffer(xml, &maxSize, "\t\t<%s %s=\"%s\" />\n", SIGNALING_TAG, ENABLE_ATTR, ppc->signalingEnabled?TRUE_STR:FALSE_STR);
      xml = fillBuffer(xml, &maxSize, "\t\t<%s>%s</%s>\n", PUBLIC_ADDRESS_TAG, FormatIpAddress(ppc->publicAddr, ipstr), PUBLIC_ADDRESS_TAG);
      xml = fillBuffer(xml, &maxSize, "\t\t<%s %s=\"%s\" %s=\"%d\" %s=\"%d\" />\n", PORTALLOC_TAG, IP_ADDRESS_ATTR, FormatIpAddress(ppc->portAllocIpAddr, ipstr), LOW_ATTR, ppc->lowPort, HIGH_ATTR, ppc->highPort);

      xml = fillBuffer(xml, &maxSize, "\t\t<%s %s=\"%d\">\n", PPC_STATS_TAG, PINHOLES_ATTR, stats->pinholes);
      xml = fillBuffer(xml, &maxSize, "\t\t\t<%s %s=\"%d\" %s=\"%d\">%s</%s>\n", TRANSLATION_TAG, TOTAL_ATTR, stats->numTranslationFailed, CODE_ATTR, stats->lastKnownTranslationFailCode, GetFailCodeString(stats->lastKnownTranslationFailCode, statusMesg, STATUS_MESG_SIZE), TRANSLATION_TAG);
      xml = fillBuffer(xml, &maxSize, "\t\t\t<%s %s=\"%d\" %s=\"%d\">%s</%s>\n", PERMISSION_TAG, TOTAL_ATTR, stats->numPermissionFailed, CODE_ATTR, stats->lastKnownPermissionFailCode, GetFailCodeString(stats->lastKnownPermissionFailCode, statusMesg, STATUS_MESG_SIZE), PERMISSION_TAG);
      xml = fillBuffer(xml, &maxSize, "\t\t</%s>\n", PPC_STATS_TAG);

      xml = fillBuffer(xml, &maxSize, "\t</%s>\n", PPC_TAG);
    }

    xml = fillBuffer(xml, &maxSize, "</%s>\n", FIREWALL_TAG);
  }
  LockReleaseLock(lsMem->fwLock);

  xml = fillBuffer(xml, &maxSize, "</%s>\n", ARAVOXCFG_TAG);

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: XML message created: \n%s\n", fn, xml?xml:"NULL"));

  return xml;
}

