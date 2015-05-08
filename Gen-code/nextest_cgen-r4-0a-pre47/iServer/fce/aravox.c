/**
 * this file contains aravox firewall specific api
 *
 * for i86pc architecture, only empty stubs will get compiled, the rest of the
 * code is relevant only to sun4 architecture
 */

#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <strings.h>

#include "cache.h"
#include "lock.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "aravox.h"
#include "readconfig.h"
#include "fceutils.h"
#include "xmlparse.h"
#include "readAravox.h"
#include "firewallcontrol.h"


extern int CheckFwLock (void);


#ifndef I86PC
/* hashtable cache for translation stored according to the assigned IDs */
cache_t aidCache;
/* hashtable cache for translation stored according to the session IDs */
cache_t sidCache;
#endif


void
AravoxPrintDebug ()
{
  AravoxPrintConfigParams((FirewallParams *)lsMem->fwParams);
}


/* print the config information for debug */
void
AravoxPrintConfigParams (FirewallParams *ptr)
{
  int ppcCount;
  char str[32];
 
  for (;ptr != NULL; ptr = ptr->nextFirewall)
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("Firewall %d\n", ptr->firewallConfigId));
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("MP Address: %s\n", FormatIpAddress(ptr->firewallMPAddr, str)));
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("User Id: %d\n", ptr->userId));
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("Auth Type: %d  Auth String: %s\n", ptr->authType, ptr->authString));
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("localNatEnabled: %d  remoteNatEnabled: %d\n", ptr->localNatEnabled, ptr->remoteNatEnabled));
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("Number of PPCs: %d\n", ptr->numPPCs));
    for (ppcCount = 0; ppcCount < ptr->numPPCs; ppcCount++)
    {
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("PPC %d   role: %d\n", ptr->ppc[ppcCount].subDeviceId, ptr->ppc[ppcCount].role));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Public Address: %s\n", FormatIpAddress(ptr->ppc[ppcCount].publicAddr, str)));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("signalingEnabled: %d\n", ptr->ppc[ppcCount].signalingEnabled));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("firewallAravoxId: 0x%x\n", ptr->ppc[ppcCount].firewallAravoxId));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Admin Status: %s  Oper Status: %s\n", GetPPCStatusString(ptr->ppc[ppcCount].adminStatus), GetPPCStatusString(ptr->ppc[ppcCount].operStatus)));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Firewall Index: %d\n", ptr->ppc[ppcCount].pFirewall->firewallConfigId));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Port Allocation: IP=%s  low=%d  high=%d\n", FormatIpAddress(ptr->ppc[ppcCount].portAllocIpAddr, str), ptr->ppc[ppcCount].lowPort, ptr->ppc[ppcCount].highPort));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Total pinholes open: %d\n", ptr->ppc[ppcCount].stats->pinholes));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Total translation failures: %d\n", ptr->ppc[ppcCount].stats->numTranslationFailed));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Last Translation fail reason: 0x%x\n", ptr->ppc[ppcCount].stats->lastKnownTranslationFailCode));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Total Permission failures: %d\n", ptr->ppc[ppcCount].stats->numPermissionFailed));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Last Permission fail reason: 0x%x\n", ptr->ppc[ppcCount].stats->lastKnownPermissionFailCode));
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("Translation ID count: %d\n", ptr->ppc[ppcCount].translationList->count));
    }
  }

#ifndef I86PC
  NETDEBUG(MFCE, NETLOG_DEBUG4, ("Number of translations in assigned Id table: %d\n", aidCache->nitems));
  NETDEBUG(MFCE, NETLOG_DEBUG4, ("Number of translations in session Id table: %d\n", sidCache->nitems));
#endif

}


/**
 * Validate the authorization data. Ensure that 
 * the parameters given MAY be valid.
 * @param fw the fireallParams containing the auth data
 * @return TRUE of the data is valid.
 */
static unsigned int
validateAuthData (FirewallParams *fw)
{
  /* a valid user id is required */
  if (fw->userId == -1)
    return FALSE;

  switch (fw->authType)
  {
  case NS_AUTH_MD5:
  case NS_AUTH_SHA1:
    /* a valid authstring of minimum length 16 is required */
    if (strlen(fw->authString) < 16)
      return FALSE;
    break;

  case NS_AUTH_NONE:
    /* don't care about the auth string here */
    break;

  default:
    return FALSE;  /* invalid auth type */
  }

  return TRUE;
}


/**
 * Validate that we have the minimum parameters required:
 *   o At least one PPC is available
 *   o At least one signaling PPC
 *   o firewallMPAddr
 *   o Auth data
 * @param params the firewall params to validate.
 * @return TRUE if the config is valid else FALSE.
 */
unsigned int
validateAravoxConfig (FirewallParams *params)
{
  int j;

  if (params->firewallMPAddr != 0 &&
      validateAuthData(params) == TRUE)
  {
    if (params->numPPCs > 0 && IsFCESigEnabled() == TRUE)
    {
      for (j = 0; j < params->numPPCs; j++)
      {
	if (params->ppc[j].signalingEnabled == TRUE)
        {
          return TRUE;
        }
      }
    }
    else
    {
      return TRUE;  // signaling not enabled
    }
  }

  return FALSE;
}


/**
 * get the number of firewalls currently configured
 */
static int
GetFirewallCount (FirewallParams *data)
{
  int i;
  FirewallParams *ptr;

  for (i = 0, ptr = data; ptr != NULL; ptr = ptr->nextFirewall, i++);

  return i;
}


/**
 * get a pointer to the PPC configuration for the given sub device id
 */
static PPCParams*
GetPPCParamForDeviceId (FirewallParams *data, unsigned int devId)
{
  int i;

  for (i = 0; i < data->numPPCs; i++)
  {
    if (data->ppc[i].subDeviceId == devId)
      return &data->ppc[i];
  }

  return NULL;
}


/**
 * get a pointer to the firewall configuration which has the given MP address
 */
static FirewallParams*
GetFirewallParamForMPAddr (FirewallParams *data, unsigned int addr)
{
  FirewallParams *ptr;

  for (ptr = data; ptr != NULL; ptr = ptr->nextFirewall)
  {
    if (ptr->firewallMPAddr == addr)
      return ptr;
  }

  return NULL;
}


/**
 * get a pointer to the PPC configuration for the given MP address/sub device id
 */
static PPCParams*
GetPPCParamForMPDevId (FirewallParams *data, unsigned int addr, unsigned int devId)
{
  void *dummy = (void *)GetPPCParamForMPDevId;  // prevent compiler warning
  FirewallParams *ptr = GetFirewallParamForMPAddr(data, addr);
  if (ptr != NULL)
    return GetPPCParamForDeviceId(ptr, devId);
  dummy = NULL;
  return NULL;
}


/**
 * Get the signaling address to be used by external clients.
 * If NAT is enabled, the returned value is the same as the
 * router address used inside of the firewall.
 *
 * @param addr the address of the FCE inside the firewall (non-NATed address)
 */
unsigned int
AravoxGetSigPublicAddress (unsigned int addr)
{
  unsigned int retval = addr;
#ifndef I86PC
  int j;
  FirewallParams *ptr;

  /*  Find the relevant firewall. If we use more than one PPC for signaling
      the address will change per PPC */
  LockGetLock(lsMem->fwLock, 0, 0);

  for (ptr = (FirewallParams *)lsMem->fwParams; ptr != NULL; ptr = ptr->nextFirewall)
  {
    for (j = 0; j < ptr->numPPCs; j++)
    {
      if (ptr->ppc[j].adminStatus == PPC_UP &&
	  ptr->ppc[j].signalingEnabled == TRUE &&
	  ptr->localNatEnabled == TRUE)
      {
	retval = ptr->ppc[j].publicAddr;
	goto _finishloop;
      }
    }
  }

 _finishloop:
  LockReleaseLock(lsMem->fwLock);
#endif

  return retval;
}


#ifdef I86PC
/* empty stubs for i86pc architecture */
int
AravoxOpenHole (unsigned int protocol,
		unsigned int mediaIpAddress,
		unsigned short mediaPort,
		eFCETrafficDirection direction,
		unsigned int sessionId,
		unsigned int sessionIdLeg2,
		int numTranslations,
		unsigned int *returnedMediaIpAddress,
		unsigned short *returnedMediaPort,
		int allocatePrivatePorts)
{
  return -1;
}


int
AravoxReopenHole (unsigned int protocol,
		  unsigned int mediaIpAddress,
		  unsigned short mediaPort,
		  eFCETrafficDirection direction,
		  unsigned int sessionIdCurrentLeg,
		  unsigned int sessionIdLeg2,
		  int numTranslations,
		  unsigned int *returnedMediaIpAddress,
		  unsigned short *returnedMediaPort,
		  int allocatePrivatePorts)
{
  return -1;
}


int
AravoxCloseHole (unsigned int permissionId)
{
  return -1;
}


int
AravoxCloseSession (unsigned int sessionId)
{
  return -1;
}

#else


/* for external to external translations, these are the array locations in which the
   translation id and the permission ids are stored */
#define INWARD     0  /* from external towards the internal router */
#define OUTWARD    1  /* from internal router towards the external router */

typedef enum
{
  OPFAILED,
  PID_ONLY,
  PID_AND_TID
} tRetStatus;


static int lastAssignedId = 0;


static void
PrintAravoxNat5Tuple (char *fieldName, aravoxNat5Tuple *tuple)
{
  char str1[32], str2[32];

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s:  src-%s/%d   dst-%s/%d  prot-%d\n", fieldName, FormatIpAddress(tuple->sourceIpAddress, str1), tuple->sourcePort, FormatIpAddress(tuple->destinationIpAddress, str2), tuple->destinationPort, tuple->protocol));
}


static void
PrintAravoxNatRule (aravoxNatRule *rule)
{
  NETDEBUG(MFCE, NETLOG_DEBUG4, ("\nAravox Translation:\n-------------------\nRule Name: %s\nRule Type: %u\nNat ID: %u\nTag: %u\nFlags: %u\nAllowed Translations: %u\n", rule->ruleName, rule->ruleType, rule->natId, rule->tag, rule->flags, rule->allowedTranslationCount));
  PrintAravoxNat5Tuple("match", &rule->match);
  PrintAravoxNat5Tuple("mask", &rule->mask);
  PrintAravoxNat5Tuple("packet", &rule->packet);
  PrintAravoxNat5Tuple("alloc", &rule->alloc);
  PrintAravoxNat5Tuple("alloc_low", &rule->alloc_low);
  PrintAravoxNat5Tuple("alloc_hi", &rule->alloc_hi);
}


/**
 * allocates space for an AravoxTranslation, initializes it and returns a pointer to it
 */
static AravoxTranslation*
AravoxTranslationAlloc (int numPids, int numTids)
{
  AravoxTranslation *ptr = NULL;
  unsigned int *pidPtr = NULL;
  unsigned int *tidPtr = NULL;

  ptr = (AravoxTranslation *)malloc(sizeof(AravoxTranslation));
  if (ptr == NULL)
  {
    NETERROR(MFCE, ("AravoxTranslationAlloc: Cannot allocate AravoxTranslation\n"));
    return NULL;
  }
  if (numPids > 0)
  {
    pidPtr = (unsigned int *)malloc(sizeof(unsigned int)*numPids);
    if (pidPtr == NULL)
    {
      NETERROR(MFCE, ("AravoxTranslationAlloc: Cannot allocate numPids\n"));
      free(ptr);
      return NULL;
    }
  }
  if (numTids > 0)
  {
    tidPtr = (unsigned int *)malloc(sizeof(unsigned int)*numTids);
    if (tidPtr == NULL)
    {
      NETERROR(MFCE, ("AravoxTranslationAlloc: Cannot allocate numTids\n"));
      if (pidPtr != NULL)
	free(pidPtr);
      free(ptr);
      return NULL;
    }
  }

  lastAssignedId++;
  if (lastAssignedId == -1)
    lastAssignedId = 1;

  ListgInitElem(ptr, SID_LIST_OFFSET);
  ListgInitElem(ptr, AID_LIST_OFFSET);
  ListgInitElem(ptr, PPC_LIST_OFFSET);

  ptr->assignedId = lastAssignedId;
  ptr->sessionId = 0;
  ptr->pidArray = pidPtr;
  ptr->numPids = numPids;
  ptr->tidArray = tidPtr;
  ptr->numTids = numTids;
  ptr->mpAddr = 0;
  ptr->devId = 0;
  ptr->increment = 0;
  ptr->count = 0;
  ptr->port = 0;
  ptr->privatePortsAllocated = 0;

  return ptr;
}


/**
 * takes in a permission id and translation ids, and returns another id
 */
static unsigned int
AddAravoxTranslation (unsigned int *pidArray,
		      int numPids,
		      unsigned int *tidArray,
		      int numTids,
		      unsigned int sid,
		      PPCParams *ppc,
		      int port,
		      int allocatePrivatePorts)
{
  AravoxTranslation *ptr;
  unsigned int *uintPtr;
  int i;

  ptr = AravoxTranslationAlloc(numPids, numTids);
  if (ptr == NULL)
    return 0;

  for (i = 0, uintPtr = ptr->pidArray; i < numPids; i++, uintPtr++, pidArray++)
    *uintPtr = *pidArray;
  for (i = 0, uintPtr = ptr->tidArray; i < numTids; i++, uintPtr++, tidArray++)
    *uintPtr = *tidArray;
  ptr->sessionId = sid;
  ptr->mpAddr = ppc->pFirewall->firewallMPAddr;
  ptr->devId = ppc->subDeviceId;
  ptr->increment = numPids;
  ppc->stats->pinholes += numPids;
  ptr->port = port;
  ptr->privatePortsAllocated = allocatePrivatePorts;

  /* add the new block to the session id hashtable cache */
  CacheInsert(sidCache, ptr);

  /* add the new block to the assigned id hashtable cache */
  CacheInsert(aidCache, ptr);

  /* add the new block to the ppc linked list */
  ListgInsert(ppc->translationList, ptr, PPC_LIST_OFFSET);
  ppc->translationList->count++;

  return ptr->assignedId;
}


static inline void
DeleteFromCacheList (cache_t cache, void *elem, int offset)
{
  ListgDelete(elem, offset);
  cache->nitems--;
}


static inline void
DeleteFromAssignedIdList (AravoxTranslation *ptr)
{
  DeleteFromCacheList(aidCache, ptr, AID_LIST_OFFSET);
}


static inline void
DeleteFromSessionIdList (AravoxTranslation *ptr)
{
  DeleteFromCacheList(sidCache, ptr, SID_LIST_OFFSET);
}


static inline void
DeleteFromPPCList (PPCParams *ppc, AravoxTranslation *ptr)
{
  ListgDelete(ptr, PPC_LIST_OFFSET);
  ppc->translationList->count--;
}

  
/**
 * frees memory for the given AravoxTranslation
 */
static void
FreeAravoxTranslation (AravoxTranslation *ptr)
{
  if (ptr->pidArray != NULL)
    free(ptr->pidArray);
  if (ptr->tidArray != NULL)
    free(ptr->tidArray);
  free(ptr);
}  


/**
 * deletes all the translations which contain the given PPC
 */
static void
DeleteAravoxTranslationForPPC (PPCParams *ppc)
{
  AravoxTranslation *list, *ptr, *tmp;

  list = ppc->translationList;
  ptr = (AravoxTranslation *)(Listg(list, PPC_LIST_OFFSET)->next);
  while (ptr != list)
  {
    tmp = ptr;
    ptr = (AravoxTranslation *)(Listg(ptr, PPC_LIST_OFFSET)->next);

    DeleteFromAssignedIdList(tmp);
    DeleteFromSessionIdList(tmp);
    DeleteFromPPCList(ppc, tmp);

    if (ppc->portAlloc && tmp->port > 0)
      freePort(ppc->portAlloc, tmp->port);
    FreeAravoxTranslation(tmp);
  }
}

  
/**
 * print a verbose message of the status code returned from one of aravox
 * API calls
 *
 * @param debugLevel the debug level to log at
 * @param fn the function name calling this method
 * @param debugStr any extra debug string to log
 * @param statusCode the status code returned by an aravox API call
 */
static void
PrintStatusCode (int debugLevel, char *fn, char *debugStr, int statusCode)
{
  char mesg[1024] = {0};
  char stext[NSVOIP_MAX_STATUS_TEXT_LENGTH];

  nsVoipGetStatusText(stext);
  if (nsVoipGetStatusCodeString(statusCode, mesg) == TRUE)
  {
    if (debugLevel == 0)
    {
      NETERROR(MFCE, ("%s: %s [0x%x]: %s %s\n", fn, debugStr, statusCode, mesg, stext));
    }
    else
    {
      NETDEBUG(MFCE, debugLevel, ("%s: %s [0x%x]: %s %s\n", fn, debugStr, statusCode, mesg, stext));
    }
  }
  else
  {
    if (debugLevel == 0)
    {
      NETERROR(MFCE, ("%s: %s [0x%x] %s\n", fn, debugStr, statusCode, stext));
    }
    else
    {
      NETDEBUG(MFCE, debugLevel, ("%s: %s [0x%x] %s\n", fn, debugStr, statusCode, stext));
    }
  }
}


/**
 * Create the authorization data used in the Aravox calls for the given firewall
 */
static void
createAuthDataForFirewall (FirewallParams *ptr)
{
  int status = -1;
  char *fn = "createAuthDataForFirewall";

  ptr->authData = NULL;
  if (ptr->authType == NS_AUTH_MD5)
  {
    status = nsConvertStringToMD5Secret(ptr->authString, &ptr->authDataBlock);
  }
  else if (ptr->authType == NS_AUTH_SHA1)
  {
    status = nsConvertStringToSHA1Secret(ptr->authString, &ptr->authDataBlock);
  }
  else if (ptr->authType == NS_AUTH_NONE)
  {
    return;  /* authData remains NULL for this auth type */
  }
  else
  {
    NETERROR(MFCE, ("%s: Invalid authentication type [%d] for firewall %d\n", fn, ptr->authType, ptr->firewallConfigId));
    return;
  }

  if (status == NS_AUTH_SUCCESS)
  {
    ptr->authData = &ptr->authDataBlock;
  }
  else
  {
    PrintStatusCode(0, fn, "Error converting authString to encrypted secret", status);
    NETERROR(MFCE, ("%s: Error converting authorization string into an encrypted secret for firewall %d\n", fn, ptr->firewallConfigId));
  }
}


/**
 * validates if the parameters requires for port allocation are valid
 *
 * @return TRUE if they are valid, false if not
 */
int
validatePortAllocParams (PPCParams *ppc)
{
  if (ppc->portAllocIpAddr != 0 &&
      ppc->lowPort > 0 &&
      ppc->highPort > ppc->lowPort)
    return TRUE;

  return FALSE;
}


/**
 * initialise the given PPC connection, if the admin status is PPC_UP
 *
 * @return TRUE if firewall connection was initialised, FALSE otherwise
 */
static int
AravoxPPCInit (PPCParams *ppc, int forceClearHoles)
{
  unsigned int status;
  char str[32];
  char *fn = "AravoxPPCInit";

  if (ppc->adminStatus == PPC_UP)
  {
    status = nsVoipFirewallInit(ppc->pFirewall->firewallMPAddr,
                                NSVOIP_FIREWALL_TYPE_ARAVOX,
                                ppc->pFirewall->userId,
                                ppc->pFirewall->authType,
                                ppc->pFirewall->authData,
                                ppc->subDeviceId,
                                0, 0,
                                (ppc->pFirewall->localNatEnabled == TRUE ||
                                 ppc->pFirewall->remoteNatEnabled == TRUE)?TRUE:FALSE,
                                (forceClearHoles || ppc->role==MASTER)?TRUE:FALSE,
                                &ppc->firewallAravoxId);
    if (status == NSVOIP_SUCCESS)
    {
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Successfully initialized with %s, firewall %d, PPC %d\n", fn, FormatIpAddress(ppc->pFirewall->firewallMPAddr, str), ppc->pFirewall->firewallConfigId, ppc->subDeviceId));

      ppc->operStatus = PPC_UP;
      ppc->stats->pinholes = 0;

      /* if we have to do private port allocation, initialize that also */
      if (validatePortAllocParams(ppc) == TRUE)
      {
	if ((ppc->portAlloc = initPortAllocation(ppc->lowPort, ppc->highPort)) == NULL)
	{
	  NETERROR(MFCE, ("%s: Error initializing port allocation\n", fn));
	}
      }
      else
      {
	NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Invalid port allocation parameters: %s:%d-%d\n", fn, FormatIpAddress(ppc->portAllocIpAddr, str), ppc->lowPort, ppc->highPort));
	ppc->portAlloc = NULL;
      }
    } else {
      PrintStatusCode(0, fn, "Error initializing Aravox Firewall Connection", status);
      NETERROR(MFCE, ("%s: Error initializing connection with %s, firewall %d, PPC %d\n", fn, FormatIpAddress(ppc->pFirewall->firewallMPAddr, str), ppc->pFirewall->firewallConfigId, ppc->subDeviceId));
      ppc->firewallAravoxId = -1;
      ppc->operStatus = PPC_DOWN;
    }
  }
  else
  {
    ppc->firewallAravoxId = -1;
    ppc->operStatus = PPC_DOWN;
  }

  return (ppc->operStatus == PPC_UP)?TRUE:FALSE;
}


/**
 * shutdown the connection to this PPC
 *
 * @return TRUE if the connection is closed correctly, FALSE otherwise
 */
static int
AravoxPPCShutdown (PPCParams *ppc)
{
  unsigned int status;
  int retcode = TRUE;
  char str[32];
  char *fn = "AravoxPPCShutdown";

  if (ppc->firewallAravoxId != -1)
  {
    status = nsVoipFirewallShutdown(ppc->firewallAravoxId);
    if (status == NSVOIP_SUCCESS)
    {
      NETDEBUG(MFCE, NETLOG_DEBUG4,
	       ("%s: Successfully shutdown connection with %s, firewall %d, PPC %d, id 0x%x\n",
		fn,
		FormatIpAddress(ppc->pFirewall->firewallMPAddr, str),
		ppc->pFirewall->firewallConfigId,
		ppc->subDeviceId,
		ppc->firewallAravoxId));
      ppc->operStatus = PPC_DOWN;
      ppc->firewallAravoxId = -1;
      ppc->stats->pinholes = 0;

      /* if private port allocation was used, release resources */
      if (ppc->portAlloc != NULL)
	clearPortAllocation(ppc->portAlloc);
      ppc->portAlloc = NULL;
    } else {
      PrintStatusCode(0, fn, "Error shutting down firewall connection", status);
      NETERROR(MFCE,
	       ("%s: Error shutting down firewall connection with %s, firewall %d, PPC %d, id 0x%x\n",
		fn,
		FormatIpAddress(ppc->pFirewall->firewallMPAddr, str),
		ppc->pFirewall->firewallConfigId,
		ppc->subDeviceId,
		ppc->firewallAravoxId));
      retcode = FALSE;
    }

    DeleteAravoxTranslationForPPC(ppc);
  }

  return retcode;
}


/**
 * initialise connections to all the PPCs in this firewall
 */
static void
AravoxFirewallInit (FirewallParams *fw, int forceClearHoles)
{
  int ppcCount;

  createAuthDataForFirewall(fw);

  for (ppcCount = 0; ppcCount < fw->numPPCs; ppcCount++)
  {
    if (forceClearHoles && fw->ppc[ppcCount].firewallAravoxId != -1)
      AravoxPPCShutdown(&fw->ppc[ppcCount]);
    AravoxPPCInit(&fw->ppc[ppcCount], forceClearHoles);
  }
}


/**
 * shutdown the connections to all the PPCs on this firewall
 */
static int
AravoxFirewallShutdown (FirewallParams *fw)
{
  int ppcCount;
  int retcode = TRUE;

  for (ppcCount = 0; ppcCount < fw->numPPCs; ppcCount++)
  {
    if (AravoxPPCShutdown(&fw->ppc[ppcCount]) == FALSE)
      retcode = FALSE;
  }

  return retcode;
}


/**
 * Get a PPC to use for this leg of a call.
 * @param sessionId the session ID for this call leg.
 * @return the PPC card to use else NULL if there are none available.
 */
static PPCParams*
GetPPCForCall ()
{
  FirewallParams *ptr;
  int j;
  PPCParams *ppc = NULL;
  PPCParams *sppc = NULL;

  /* search through the active firewalls and PPCs for the one with the least calls */
  for (ptr = (FirewallParams *)lsMem->fwParams; ptr != NULL; ptr = ptr->nextFirewall)
  {
    for (j = 0; j < ptr->numPPCs; j++)
    {
      if (ptr->ppc[j].operStatus == PPC_UP)
      {
        if (ptr->ppc[j].signalingEnabled == TRUE)
        {
          sppc = &ptr->ppc[j];
        }
	else if (ppc == NULL ||
	    ppc->stats->pinholes > ptr->ppc[j].stats->pinholes)
	{
	  ppc = &ptr->ppc[j];
	}
      }
    }
  }

  return (ppc == NULL)?sppc:ppc;
}


/**
 * traffic flows from a private endpoint to a public endpoint
 *
 * @return PID_ONLY if translation was not necessary, but the pinhole was opened
 * PID_AND_TID if both translation and pinhole operations were sucessfull
 * OPFAILED if any error happened
 */
static tRetStatus
AravoxOpenFromInt2Ext (PPCParams *ppc,
		       unsigned int translateProtocol,
		       unsigned int pinholeProtocol,
		       unsigned int remoteIpAddress,
		       unsigned short remotePort,
		       int numTranslations,
		       int sessionId,
		       unsigned int *returnedRemoteIpAddress,
		       unsigned short *returnedRemotePort,
		       unsigned int *rtid,
		       unsigned int *pid)
{
  unsigned int sot, status;
  aravoxNatRule translation;
  char ipstr1[32], ipstr2[32];
  char *fn = "AravoxOpenFromInt2Ext";
  tRetStatus retcode = PID_ONLY;

  /* open a translation if packet steering is enabled */
  *rtid = -1;
  if (ppc->pFirewall->remoteNatEnabled == TRUE)
  {
    status = nsVoipTranslateRemoteAddressAndGetRaw(ppc->firewallAravoxId, translateProtocol, remoteIpAddress, remotePort, FCE_ANY_IP, 0x0, FCE_ANY_PORT, 0x0, numTranslations, sessionId, returnedRemoteIpAddress, returnedRemotePort, rtid, &sot, &translation);
    if (status != NSVOIP_SUCCESS)
    {
      ppc->stats->numTranslationFailed++;
      ppc->stats->lastKnownTranslationFailCode = status;
      ppc->operStatus = PPC_DOWN;
      PrintStatusCode(0, fn, "Error creating remote translation", status);
      return OPFAILED;
    }

    retcode = PID_AND_TID;
    NETDEBUG(MFCE, NETLOG_DEBUG4,
	     ("%s: Successfully created remote translation: ANY -> (%s:%d ==> %s:%d) [tid: 0x%x]\n",
	      fn,
	      FormatIpAddress(remoteIpAddress, ipstr1),
	      remotePort,
	      FormatIpAddress(*returnedRemoteIpAddress, ipstr2),
	      *returnedRemotePort,
	      *rtid));
  }

  /* now open the pinhole */
  status = nsVoipOpenPermission3(ppc->firewallAravoxId, FCE_ANY_IP, FCE_ANY_PORT, remoteIpAddress, remotePort, pinholeProtocol, sessionId, NSVOIP_TWO_WAY, 0, 0, pid);
  if (status != NSVOIP_SUCCESS)
  {
    ppc->stats->numPermissionFailed++;
    ppc->stats->lastKnownPermissionFailCode = status;
    ppc->operStatus = PPC_DOWN;
    PrintStatusCode(0, fn, "Error opening pinhole", status);
    if (*rtid != -1)
    {
      status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *rtid);
      if (status != NSVOIP_SUCCESS)
      {
	PrintStatusCode(0, fn, "Error closing remote translation", status);
      }
    }
    return OPFAILED;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Successfully created pinhole: ANY -> %s:%d [pid - 0x%x]\n",
	    fn,
	    FormatIpAddress(remoteIpAddress, ipstr1),
	    remotePort,
	    *pid));

  return retcode;
}


/**
 * traffic flows from a public endpoint to a private endpoint
 *
 * @return PID_ONLY if translation was not necessary, but the pinhole was opened
 * PID_AND_TID if both translation and pinhole operations were sucessfull
 * OPFAILED if any error happened
 */
static tRetStatus
AravoxOpenFromExt2Int (PPCParams *ppc,
		       unsigned int translateProtocol,
		       unsigned int pinholeProtocol,
		       unsigned int localIpAddress,
		       unsigned short localPort,
		       int numTranslations,
		       int sessionId,
		       unsigned int *returnedLocalIpAddress,
		       unsigned short *returnedLocalPort,
		       unsigned int *ltid,
		       unsigned int *pid)
{
  unsigned int sot, status;
  aravoxNatRule translation;
  char ipstr1[32], ipstr2[32];
  char *fn = "AravoxOpenFromExt2Int";
  tRetStatus retcode = PID_ONLY;

  /* open a translation if NAT is enabled */
  *ltid = -1;
  if (ppc->pFirewall->localNatEnabled == TRUE)
  {
    status = nsVoipTranslateLocalAddressAndGetRaw(ppc->firewallAravoxId, translateProtocol, localIpAddress, localPort, FCE_ANY_IP, 0x0, FCE_ANY_PORT, 0x0, numTranslations, sessionId, returnedLocalIpAddress, returnedLocalPort, ltid, &sot, &translation);
    if (status != NSVOIP_SUCCESS)
    {
      ppc->stats->numTranslationFailed++;
      ppc->stats->lastKnownTranslationFailCode = status;
      ppc->operStatus = PPC_DOWN;
      PrintStatusCode(0, fn, "Error creating local translation", status);
      return OPFAILED;
    }

    retcode = PID_AND_TID;
    NETDEBUG(MFCE, NETLOG_DEBUG4,
	     ("%s: Successfully created local translation: ANY -> (%s:%d ==> %s:%d) [tid: 0x%x]\n",
	      fn,
	      FormatIpAddress(localIpAddress, ipstr1),
	      localPort,
	      FormatIpAddress(*returnedLocalIpAddress, ipstr2),
	      *returnedLocalPort,
	      *ltid));
  }

  /* now open the pinhole */
  status = nsVoipOpenPermission3(ppc->firewallAravoxId, FCE_ANY_IP, FCE_ANY_PORT, localIpAddress, localPort, pinholeProtocol, sessionId, NSVOIP_TWO_WAY, 0, 0, pid);
  if (status != NSVOIP_SUCCESS)
  {
    ppc->stats->numPermissionFailed++;
    ppc->stats->lastKnownPermissionFailCode = status;
    ppc->operStatus = PPC_DOWN;
    PrintStatusCode(0, fn, "Error opening pinhole", status);
    if (*ltid != -1)
    {
      status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *ltid);
      if (status != NSVOIP_SUCCESS)
      {
	PrintStatusCode(0, fn, "Error closing local translation", status);
      }
    }
    return OPFAILED;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Successfully created pinhole: ANY -> %s:%d [pid - 0x%x]\n",
	    fn,
	    FormatIpAddress(localIpAddress, ipstr1),
	    localPort,
	    *pid));

  return retcode;
}


/**
 * This method opens a remote translation on the firewall. If private port allocation is requested,
 * this will override the firewall provided ports with the configured ones on the PPC, and install
 * them back on the firewall
 *
 * @param ppc the PPC on the firewall
 * @param translateProtocol the translation protocol
 * @param remoteIpAddress the address to open the translation to
 * @param remotePort the port to open the translation to
 * @param numTranslations number of translations allowed
 * @param sessionId the session to associate this translation with
 * @param returnedRemoteIpAddress the ip address returned by the firewall (or the IP address
 * configured on the PPC for using in private port allocation)
 * @param returnedRemotePort the port number returned by the firewall (or the port number 
 * privately allocated by this method)
 * @param rtid the remote translation id returned by the firewall
 * @param port if private port allocation is enabled and the value in this field is > 0, then
 * this value is used in the translation, if not a new port number is allocated and used.
 * @return 0 on success -1 on failure
 */
static int
AravoxInstallRemoteTranslation (PPCParams *ppc,
				unsigned int translateProtocol,
				unsigned int remoteIpAddress,
				unsigned short remotePort,
				int numTranslations,
				int sessionId,
				unsigned int *returnedRemoteIpAddress,
				unsigned short *returnedRemotePort,
				unsigned int *rtid,
				int *port,
				int allocatePrivatePorts)
{
  unsigned int sot, status;
  aravoxNatRule translation;
  char ipstr1[32], ipstr2[32];
  char *fn = "AravoxInstallRemoteTranslation";
  int allocatedPort;

  /* open translation from ANY to the remote address */
  status = nsVoipTranslateRemoteAddressAndGetRaw(ppc->firewallAravoxId, translateProtocol, remoteIpAddress, remotePort, FCE_ANY_IP, 0x0, FCE_ANY_PORT, 0x0, numTranslations, sessionId, returnedRemoteIpAddress, returnedRemotePort, rtid, &sot, &translation);
  if (status != NSVOIP_SUCCESS)
  {
    ppc->stats->numTranslationFailed++;
    ppc->stats->lastKnownTranslationFailCode = status;
    ppc->operStatus = PPC_DOWN;
    PrintStatusCode(0, fn, "Error creating remote translation", status);
    return -1;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Successfully created remote translation: ANY -> (%s:%d ==> %s:%d) [tid: 0x%x]\n",
	    fn,
	    FormatIpAddress(remoteIpAddress, ipstr1),
	    remotePort,
	    FormatIpAddress(*returnedRemoteIpAddress, ipstr2),
	    *returnedRemotePort,
	    *rtid));

  /* if private port allocation is enabled, overwrite the given translation with our address */
  if (allocatePrivatePorts && ppc->portAlloc != NULL)
  {
    if (*port > 0)
    {
      allocatedPort = *port;
    }
    else
    {
      /* need to allocate a new port */
      allocatedPort = allocPort(ppc->portAlloc);
    }

    if (allocatedPort > 0)
    {
      PrintAravoxNatRule(&translation);

      *port = allocatedPort;
      *returnedRemotePort = translation.match.destinationPort = (unsigned short)*port;
      *returnedRemoteIpAddress = translation.match.destinationIpAddress = ppc->portAllocIpAddr;
      // sessionId+1 is used to store the remote translation, this way when reopening a remote
      // translation we can delete the old translation using the CloseSession call (deleting using
      // translation id does not seem to work)
      translation.tag = sessionId + 1;

      /* keep the source port the same as before */
      translation.alloc.sourcePort = 0;
      translation.alloc_low.sourcePort = (unsigned short)*port;
      translation.alloc_hi.sourcePort = (unsigned short)*port;

      /* close the translation we just opened */
      status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *rtid);
      if (status != NSVOIP_SUCCESS)
      {
	PrintStatusCode(0, fn, "Error closing remote translation", status);
      }
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: deleted the translation [tid: 0x%x] just created\n", fn, *rtid));

      /* now install the overridden translation */
      status = nsVoipInstallRawTranslationCopy(ppc->firewallAravoxId, sessionId+1, sot, &translation, rtid);
      if (status != NSVOIP_SUCCESS)
      {
	PrintStatusCode(0, fn, "Error installing raw translation info", status);
	return -1;
      }
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Installed new translation as [tid: 0x%x]:\n", fn, *rtid));
      PrintAravoxNatRule(&translation);
    }
    else
    {
      NETERROR(MFCE, ("%s: unable to allocate port, will proceed with port allocated by the firewall\n", fn));
    }
  }
  else
  {
    /* port allocation was turned off on this PPC, is the caller asking for it? */
    if (allocatePrivatePorts)
    {
      NETERROR(MFCE, ("%s: request arrived for reusing port, while port allocation config in PPC is invalid\n", fn));
    }
  }

  return 0;
}


/**
 * traffic flows from a public endpoint to a public endpoint
 *
 * We create the following two translation:
 *   ANYInternal -> External ==>translatedExternal
 *   translatedExternal -> ANYInternal ==>translatedInternal
 * When a packet arrives for translatedInternal, while going through the PPC, the
 * destination gets changed to translatedExternal. The packet turns around and goes
 * out to the translatedExternal. While going back on the PPC, the destination gets
 * changed to the original External.
 *
 * @return PID_ONLY if translation was not necessary, but the pinhole was opened
 * PID_AND_TID if both translation and pinhole operations were sucessfull
 * OPFAILED if any error happened
 */
static tRetStatus
AravoxOpenFromExt2Ext (PPCParams *ppc,
		       unsigned int translateProtocol,
		       unsigned int pinholeProtocol,
		       unsigned int remoteIpAddress,
		       unsigned short remotePort,
		       int numTranslations,
		       int sessionId,
		       unsigned int *returnedLocalIpAddress,
		       unsigned short *returnedLocalPort,
		       unsigned int *ltid,
		       unsigned int *rtid,
		       unsigned int *lpid,
		       unsigned int *rpid,
		       int *port,
		       int allocatePrivatePorts)
{
  unsigned int sot, status, returnedRemoteIpAddress;
  unsigned short returnedRemotePort;
  aravoxNatRule translation;
  char ipstr1[32], ipstr2[32];
  char *fn = "AravoxOpenFromExt2Ext";

  if (allocatePrivatePorts)
    numTranslations = 0;

  /* open translation from ANY to the remote address */
  if (AravoxInstallRemoteTranslation(ppc, translateProtocol, remoteIpAddress, remotePort, numTranslations, sessionId, &returnedRemoteIpAddress, &returnedRemotePort, rtid, port, allocatePrivatePorts) == -1)
  {
    NETERROR(MFCE, ("%s: Error installing remote translation\n", fn));
    return OPFAILED;
  }

  /* open translation from ANY to the translated remote address */
  status = nsVoipTranslateLocalAddressAndGetRaw(ppc->firewallAravoxId, translateProtocol, returnedRemoteIpAddress, returnedRemotePort, FCE_ANY_IP, 0x0, FCE_ANY_PORT, 0x0, numTranslations, sessionId, returnedLocalIpAddress, returnedLocalPort, ltid, &sot, &translation);
  if (status != NSVOIP_SUCCESS)
  { 
    ppc->stats->numTranslationFailed++;
    ppc->stats->lastKnownTranslationFailCode = status;
    ppc->operStatus = PPC_DOWN;
    PrintStatusCode(0, fn, "Error creating local translation", status);
    // delete the previous remote translation
    status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *rtid);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing remote translation", status);
    }
    return OPFAILED;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Successfully created local translation: ANY -> (%s:%d ==> %s:%d) [tid: 0x%x]\n",
	    fn,
	    FormatIpAddress(returnedRemoteIpAddress, ipstr1),
	    returnedRemotePort,
	    FormatIpAddress(*returnedLocalIpAddress, ipstr2),
	    *returnedLocalPort,
	    *ltid));
  //  PrintAravoxNatRule(&translation);

  /* open pinhole from any to remote */
  if (allocatePrivatePorts)
    status = nsVoipOpenPermission3(ppc->firewallAravoxId, FCE_ANY_IP, FCE_ANY_PORT, remoteIpAddress, remotePort, pinholeProtocol, sessionId+1, NSVOIP_TWO_WAY, 0, 0, rpid);
  else
    status = nsVoipOpenPermission3(ppc->firewallAravoxId, FCE_ANY_IP, FCE_ANY_PORT, remoteIpAddress, remotePort, pinholeProtocol, sessionId, NSVOIP_TWO_WAY, 0, 0, rpid);
  if (status != NSVOIP_SUCCESS)
  {
    ppc->stats->numPermissionFailed++;
    ppc->stats->lastKnownPermissionFailCode = status;
    ppc->operStatus = PPC_DOWN;
    PrintStatusCode(0, fn, "Error opening remote pinhole", status);
    status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *rtid);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing remote translation", status);
    }
    status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *ltid);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing local translation", status);
    }
    return OPFAILED;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Successfully created pinhole: ANY -> %s:%d [pid - 0x%x]\n",
	    fn,
	    FormatIpAddress(remoteIpAddress, ipstr1),
	    remotePort,
	    *rpid));

  /* open pinhole from any to translated remote */
  status = nsVoipOpenPermission3(ppc->firewallAravoxId, FCE_ANY_IP, FCE_ANY_PORT, returnedRemoteIpAddress, returnedRemotePort, pinholeProtocol, sessionId, NSVOIP_TWO_WAY, 0, 0, lpid);
  if (status != NSVOIP_SUCCESS)
  {
    ppc->stats->numPermissionFailed++;
    ppc->stats->lastKnownPermissionFailCode = status;
    ppc->operStatus = PPC_DOWN;
    PrintStatusCode(0, fn, "Error opening local pinhole", status);
    status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *rtid);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing remote translation", status);
    }
    status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *ltid);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing local translation", status);
    }
    status = nsVoipClosePermission(ppc->firewallAravoxId, *rpid);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing remote pinhole", status);
    }
    return OPFAILED;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Successfully created pinhole: ANY -> %s:%d [pid - 0x%x]\n",
	    fn,
	    FormatIpAddress(returnedRemoteIpAddress, ipstr1),
	    returnedRemotePort,
	    *lpid));

  return PID_AND_TID;
}


/**
 * given the protocol, fill in the aravox specific translation and the pinhole protocol
 */
static void
fillProtocols (int protocol, int *translateProtocol, int *pinholeProtocol)
{
  /* Change the protocol to the Aravox specifics. */
  switch (protocol)
  {
  case IPPROTO_UDP:
    *pinholeProtocol = NSVOIP_PROTOCOL_UDP;
    *translateProtocol = NSVOIP_PROTOCOL_UDP;
    break;
  case IPPROTO_TCP:
    *pinholeProtocol = NSVOIP_PROTOCOL_TCP;
    *translateProtocol = NSVOIP_PROTOCOL_TCP;
    break;
  case IPPROTO_RAW:
  default:
    *pinholeProtocol = NSVOIP_PROTOCOL_UDP;
    *translateProtocol = NSVOIP_PROTOCOL_RTP;
    break;
  }
}


// return the PPCParams for the given session id, also fill in the corresponding translation ptr
static PPCParams*
GetPPCParamForSessionId (unsigned int sessionId, AravoxTranslation **ptr)
{
  char *fn = "GetPPCParamForSessionId";
  PPCParams *ppc = NULL;
  AravoxTranslation *tptr;
  char str[32] = {0};

  /* get the cache entry corresponding to the given permission id */
  tptr = CacheGet(sidCache, &sessionId);
  if (tptr == NULL)
  {
    NETERROR(MFCE, ("%s: error finding cache entry for session id %d\n", fn, sessionId));
    return NULL;
  }

  ppc = GetPPCParamForMPDevId((FirewallParams *)lsMem->fwParams, tptr->mpAddr, tptr->devId);
  if (ppc == NULL)
  {
    NETERROR(MFCE, ("%s: Cannot find ppc for FW %s, PPC %d\n", fn, FormatIpAddress(tptr->mpAddr, str), tptr->devId));
  }

  if (ptr != NULL)
    *ptr = tptr;
  return ppc;
}


int
AravoxOpenHole (unsigned int protocol,
		unsigned int mediaIpAddress,
		unsigned short mediaPort,
		eFCETrafficDirection direction,
		unsigned int sessionId,
		unsigned int sessionIdLeg2,
		int numTranslations,
		unsigned int *returnedMediaIpAddress,
		unsigned short *returnedMediaPort,
		int allocatePrivatePorts)
{
  tRetStatus status;
  unsigned int pinholeProtocol;
  unsigned int translateProtocol;
  char str[32] = {0};
  char str1[32] = {0};
  PPCParams *ppc = NULL;
  unsigned int tidArray[2];  // max 2 translations possible (for Ext2Ext)
  unsigned int pidArray[2];  // max 2 pinholes possible (for Ext2Ext)
  char *fn = "AravoxOpenHole";
  int port = -1;  // private port allocation

  /* in case no translation is done, default to the given addresses */
  *returnedMediaIpAddress = mediaIpAddress;
  *returnedMediaPort = mediaPort;

  if (direction == FCE_eInt2Int)
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: nothing to for internal to internal traffic\n", fn));
    return 0;
  }

  /* mutex locked, remember to unlock it wherever you return from this method */
  LockGetLock(lsMem->fwLock, 0, 0);

  /* The sessionId increments for each call and uniquely identifies a call. It can
     be used for determining the PPC for load sharing. 
     The load sharing will also traverse firewalls if they are configured. */
  if (sessionIdLeg2 == 0)
    ppc = GetPPCForCall();  // other call leg does not exist, choose the best PPC for this call
  else
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: session id %d exists on the other call leg, will try to reuse the same PPC\n", fn, sessionIdLeg2));
    ppc = GetPPCParamForSessionId(sessionIdLeg2, NULL);
    if (ppc == NULL)
    {
      NETERROR(MFCE, ("%s: Unable to find PPC for the sessionIdLeg2 %d specified, picking some other ppc\n", fn, sessionIdLeg2));
      ppc = GetPPCForCall();
    }
  }

  if (ppc == NULL)
  {
    NETERROR(MFCE, ("%s: Unable to allocate PPC for session %d (check if the firewall connections were initialized successfully)\n", fn, sessionId));
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }

  if (direction == FCE_eExt2Ext &&
      (ppc->pFirewall->localNatEnabled != TRUE ||
       ppc->pFirewall->remoteNatEnabled != TRUE))
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: %s not enabled, nothing to do for external to external traffic\n", fn, (ppc->pFirewall->localNatEnabled != TRUE)?"NAT":"Packet Steering"));
    LockReleaseLock(lsMem->fwLock);
    return 0;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Using FW %d, PPC %d (id 0x%x) for call on session Id %d, media %s:%d, %d existing pinholes on this ppc\n",
	    fn,
	    ppc->pFirewall->firewallConfigId,
	    ppc->subDeviceId,
	    ppc->firewallAravoxId,
	    sessionId,
	    FormatIpAddress(mediaIpAddress, str),
	    mediaPort,
	    ppc->stats->pinholes));

  /* Change the protocol to the Aravox specifics. */
  fillProtocols(protocol, &translateProtocol, &pinholeProtocol);

  /* open translations and pinholes in the right direction */
  switch (direction) {
  case FCE_eExt2Int:
    /* set up translations and open pinhole */
    status = AravoxOpenFromExt2Int(ppc, translateProtocol, pinholeProtocol, mediaIpAddress, mediaPort, numTranslations, sessionId, returnedMediaIpAddress, returnedMediaPort, &tidArray[INWARD], &pidArray[INWARD]);
    if (status == OPFAILED)
    {
      LockReleaseLock(lsMem->fwLock);
      return -1;
    }
    break;

  case FCE_eInt2Ext:
    /* set up translations and open pinhole */
    status = AravoxOpenFromInt2Ext(ppc, translateProtocol, pinholeProtocol, mediaIpAddress, mediaPort, numTranslations, sessionId, returnedMediaIpAddress, returnedMediaPort, &tidArray[INWARD], &pidArray[INWARD]);
    if (status == OPFAILED)
    {
      LockReleaseLock(lsMem->fwLock);
      return -1;
    }
    break;

  case FCE_eExt2Ext:
    /* if media routing enabled, set up translations and open pinholes */
    status = AravoxOpenFromExt2Ext(ppc, translateProtocol, pinholeProtocol, mediaIpAddress, mediaPort, numTranslations, sessionId, returnedMediaIpAddress, returnedMediaPort, &tidArray[INWARD], &tidArray[OUTWARD], &pidArray[INWARD], &pidArray[OUTWARD], &port, allocatePrivatePorts);
    if (status == FALSE)
    {
      LockReleaseLock(lsMem->fwLock);
      return -1;
    }
    break;

  default:
    NETERROR(MFCE, ("%s: Traffic direction %d is not handled\n", fn, direction));
    LockReleaseLock(lsMem->fwLock);
    return 0;
  }

  /* store the translation and permission ids */
  if (direction == FCE_eExt2Ext)
  {
    /* 2 translations/pinholes opened here */
    AddAravoxTranslation(pidArray, 2, tidArray, 2, sessionId, ppc, port, allocatePrivatePorts);
  }
  else
  {
    if (status == PID_ONLY)
      AddAravoxTranslation(pidArray, 1, NULL, 0, sessionId, ppc, port, allocatePrivatePorts);
    else
      AddAravoxTranslation(pidArray, 1, tidArray, 1, sessionId, ppc, port, allocatePrivatePorts);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Installed hole on FW %d, PPC %d such that, ANY-->%s:%d  will be changed to  ANY-->%s:%d, session Id %d\n", fn, ppc->pFirewall->firewallConfigId, ppc->subDeviceId, FormatIpAddress(*returnedMediaIpAddress, str), *returnedMediaPort, FormatIpAddress(mediaIpAddress, str1), mediaPort, sessionId));

  LockReleaseLock(lsMem->fwLock);

  return 0;
}


/**
 * close the permissions and translations associated with the given assigned id
 *
 * @param assignedId this is the id created local to this driver, and is unique among the
 * assignedId cache
 */
int
AravoxCloseHole (unsigned int assignedId)
{
  AravoxTranslation *ptr;
  unsigned int status;
  unsigned int retcode = 0;
  char *fn = "AravoxCloseHole";
  int i;
  unsigned int *uintPtr;
  PPCParams *ppc;
  char str[32];

  LockGetLock(lsMem->fwLock, 0, 0);

  // CacheDelete returns a chain of elements with the same key
  ptr = CacheDelete(aidCache, &assignedId);
  if (ptr == NULL)
  {
    NETERROR(MFCE, ("%s: Cannot find assigned ID %d in list\n", fn, assignedId));
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }
  if (!ListgIsSingle(ptr, AID_LIST_OFFSET))
  {
    NETERROR(MFCE, ("%s: Found too many translations for assigned ID %d\n", fn, assignedId));
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }

  ppc = GetPPCParamForMPDevId((FirewallParams *)lsMem->fwParams, ptr->mpAddr, ptr->devId);
  if (ppc == NULL)
  {
    NETERROR(MFCE, ("%s: Cannot find ppc for FW %s, PPC %d\n", fn, FormatIpAddress(ptr->mpAddr, str), ptr->devId));
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: deleting pinholes/translations for assigned id %d\n", fn, assignedId));

  /* decrement the pinhole count */
  ppc->stats->pinholes -= ptr->increment;
  if (ppc->stats->pinholes < 0)
    ppc->stats->pinholes = 0;

  for (i = 0, uintPtr = ptr->pidArray; i < ptr->numPids; i++, uintPtr++)
  {
    status = nsVoipClosePermission(ppc->firewallAravoxId, *uintPtr);
    if (status != NSVOIP_SUCCESS)
    {
      NETERROR(MFCE, ("%s: Error closing aravox permission 0x%x\n", fn, *uintPtr));
      PrintStatusCode(0, fn, "Error closing aravox permission", status);
      retcode = -1;
    }
  }

  for (i = 0, uintPtr = ptr->tidArray; i < ptr->numTids; i++, uintPtr++)
  {
    status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, *uintPtr);
    if (status != NSVOIP_SUCCESS)
    {
      NETERROR(MFCE, ("%s: Error closing aravox translation 0x%x\n", fn, *uintPtr));
      PrintStatusCode(0, fn, "Error closing aravox local translation", status);
      retcode = -1;
    }
  }

  // unlink from the session ID cache and the PPC cache and free memory
  DeleteFromSessionIdList(ptr);
  DeleteFromPPCList(ppc, ptr);
  if (ppc->portAlloc && ptr->port > 0)
    freePort(ppc->portAlloc, ptr->port);
  FreeAravoxTranslation(ptr);

  LockReleaseLock(lsMem->fwLock);

  return retcode;
}


/**
 * re-opens a hole
 * Closes the translation and pinhole in the OUTWARD (remoteNat) direction of the given
 * sessionId and opens a new translation using the same port used in the previous
 * translation. Also opens a new pinhole for the given media address.
 */
int
AravoxReopenHole (unsigned int protocol,
		  unsigned int mediaIpAddress,
		  unsigned short mediaPort,
		  eFCETrafficDirection direction,
		  unsigned int sessionId,
		  unsigned int sessionIdLeg2,
		  int numTranslations,
		  unsigned int *returnedMediaIpAddress,
		  unsigned short *returnedMediaPort,
		  int allocatePrivatePorts)
{
  char *fn = "AravoxReopenHole";
  PPCParams *ppc = NULL;
  AravoxTranslation *ptr;
  char str[32] = {0};
  unsigned int status, translateProtocol, pinholeProtocol, returnedRemoteIpAddress;
  unsigned short returnedRemotePort;

  LockGetLock(lsMem->fwLock, 0, 0);

  /* get the ppc params and aravox translation for the session id */
  ppc = GetPPCParamForSessionId(sessionId, &ptr);
  if (ppc == NULL)
  {
    NETERROR(MFCE, ("%s: Cannot find ppc for session id %d\n", fn, sessionId));
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }

  /* fill in the aravox specific protocols */
  fillProtocols(protocol, &translateProtocol, &pinholeProtocol);

  if (allocatePrivatePorts) {
    // we already put the remote translation under a separate session id, use that to clear things up
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Deleting previous remote translation and pinhole (session id %d)\n", fn, sessionId+1));
    status = nsVoipCloseSession(ppc->firewallAravoxId, sessionId+1);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing session for remote translation", status);
    }
  }
  else
  {
    /* close the previous translation */
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Deleting previous remote translation tid 0x%x\n", fn, ptr->tidArray[OUTWARD]));
    status = nsVoipDeleteTranslationById(ppc->firewallAravoxId, ptr->tidArray[OUTWARD]);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing previous remote translation", status);
    }

    /* close the previous pinhole */
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Deleting previous remote pinhole pid 0x%x\n", fn, ptr->pidArray[OUTWARD]));
    status = nsVoipClosePermission(ppc->firewallAravoxId, ptr->pidArray[OUTWARD]);
    if (status != NSVOIP_SUCCESS)
    {
      PrintStatusCode(0, fn, "Error closing previous remote pinhole", status);
    }
  }

  /* open translation from ANY to the remote address, reusing the port in the previous translation */
  if (AravoxInstallRemoteTranslation(ppc, translateProtocol, mediaIpAddress, mediaPort, numTranslations, sessionId, &returnedRemoteIpAddress, &returnedRemotePort, &ptr->tidArray[OUTWARD], &ptr->port, allocatePrivatePorts) == -1)
  {
    NETERROR(MFCE, ("%s: Error installing remote translation\n", fn));
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }

  /* open a pinhole to the new media address */
  if (allocatePrivatePorts)
    status = nsVoipOpenPermission3(ppc->firewallAravoxId, FCE_ANY_IP, FCE_ANY_PORT, mediaIpAddress, mediaPort, pinholeProtocol, sessionId+1, NSVOIP_TWO_WAY, 0, 0, &ptr->pidArray[OUTWARD]);
  else
    status = nsVoipOpenPermission3(ppc->firewallAravoxId, FCE_ANY_IP, FCE_ANY_PORT, mediaIpAddress, mediaPort, pinholeProtocol, sessionId, NSVOIP_TWO_WAY, 0, 0, &ptr->pidArray[OUTWARD]);
  if (status != NSVOIP_SUCCESS)
  {
    ppc->stats->numPermissionFailed++;
    ppc->stats->lastKnownPermissionFailCode = status;
    ppc->operStatus = PPC_DOWN;
    PrintStatusCode(0, fn, "Error re-opening remote pinhole", status);
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4,
	   ("%s: Successfully created new pinhole: ANY -> %s:%d [pid - 0x%x]\n",
	    fn,
	    FormatIpAddress(mediaIpAddress, str),
	    mediaPort,
	    ptr->pidArray[OUTWARD]));

  ptr->privatePortsAllocated = allocatePrivatePorts;

  LockReleaseLock(lsMem->fwLock);

  return 0;
}


/**
 * Close all the pinholes opened across the PPCs for the given session id
 */
int
AravoxCloseSession (unsigned int sessionId)
{
  unsigned int status;
  unsigned int retcode = 0;
  int count, i;
  char *fn = "AravoxCloseSession";
  PPCParams *ppc;
  char str[32];
  AravoxTranslation *ptr, *list, *next;

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Deleting pinholes/translations for session Id %d\n", fn, sessionId));

  LockGetLock(lsMem->fwLock, 0, 0);

  // CacheDelete returns a chain of elements with the same key
  ptr = list = CacheDelete(sidCache, &sessionId);
  if (list == NULL)
  {
    NETERROR(MFCE, ("%s: Cannot find session ID %d in session id cache\n", fn, sessionId));
    LockReleaseLock(lsMem->fwLock);
    return -1;
  }

  for (count = HashCountList(list, SID_LIST_OFFSET), i = 0; i < count; i++)
  {
    next = (AravoxTranslation *)(Listg(ptr, SID_LIST_OFFSET)->next);

    ppc = GetPPCParamForMPDevId((FirewallParams *)lsMem->fwParams, ptr->mpAddr, ptr->devId);
    if (ppc == NULL)
    {
      NETERROR(MFCE, ("%s: Cannot find ppc for FW %s, PPC %d\n", fn, FormatIpAddress(ptr->mpAddr, str), ptr->devId));
    }
    else
    {
      /* decrement the pinhole count */
      ppc->stats->pinholes -= ptr->increment;
      // sometimes, after doing a reconfig, the pinhole count can get less
      // than zero, never let that happen...
      if (ppc->stats->pinholes < 0)
	ppc->stats->pinholes = 0;

      /* close the session on the firewall */
      status = nsVoipCloseSession(ppc->firewallAravoxId, sessionId);
      if (status == NSVOIP_SUCCESS)
      {
	NETDEBUG(MFCE, NETLOG_DEBUG4,
		 ("%s: Closed session %d on FW %d, PPC %d, aravox fid 0x%x, %d pinholes on this PPC\n",
		  fn,
		  sessionId,
		  ppc->pFirewall->firewallConfigId,
		  ppc->subDeviceId,
		  ppc->firewallAravoxId,
		  ppc->stats->pinholes));
      }
      else
      {
	PrintStatusCode(0, fn, "Error closing session", status);
	NETERROR(MFCE,
		 ("%s: Error closing session %d on FW %d, PPC %d, aravox fid 0x%x, %d pinholes on this PPC\n",
		  fn,
		  sessionId,
		  ppc->pFirewall->firewallConfigId,
		  ppc->subDeviceId,
		  ppc->firewallAravoxId,
		  ppc->stats->pinholes));
	retcode = -1;
      }

      /* in case of ext2ext, we might have stored the remote translation under a separate session
       * id, delete that also */
      if (ptr->privatePortsAllocated) {
	status = nsVoipCloseSession(ppc->firewallAravoxId, sessionId+1);
	if (status == NSVOIP_SUCCESS)
	{
	  NETDEBUG(MFCE, NETLOG_DEBUG4,
		   ("%s: Closed session %d on FW %d, PPC %d, aravox fid 0x%x, %d pinholes on this PPC\n",
		    fn,
		    sessionId+1,
		    ppc->pFirewall->firewallConfigId,
		    ppc->subDeviceId,
		    ppc->firewallAravoxId,
		    ppc->stats->pinholes));
	}
	else
	{
	  PrintStatusCode(0, fn, "Error closing session", status);
	  NETERROR(MFCE,
		   ("%s: Error closing session %d on FW %d, PPC %d, aravox fid 0x%x, %d pinholes on this PPC\n",
		    fn,
		    sessionId+1,
		    ppc->pFirewall->firewallConfigId,
		    ppc->subDeviceId,
		    ppc->firewallAravoxId,
		    ppc->stats->pinholes));
	  retcode = -1;
	}
      }
    }

    // unlink from the assigned ID cache and the PPC cache and free memory
    DeleteFromAssignedIdList(ptr);
    DeleteFromPPCList(ppc, ptr);
    if (ppc->portAlloc && ptr->port > 0)
      freePort(ppc->portAlloc, ptr->port);
    FreeAravoxTranslation(ptr);
    ptr = next;
  }

  LockReleaseLock(lsMem->fwLock);

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Closed session %d (%d pids)\n", fn, sessionId, count));

  return retcode;
}


/**
 * returns the bucket size to use for the internal hashtables
 * has a max of 16384, or tracks the max licenses (in powers of 2) for the range lower
 * than that
 */
static int
getHashtableBucketSize ()
{
  int buckets = 1;

  if (lsMem->maxCalls <= 0)
  {
    buckets = 1024;
  }
  else
  {
    while ( (buckets < 16384) &&
	    (buckets < lsMem->maxCalls) )
    {
      buckets <<= 1;
    }
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("getHashtableBucketSize: using bucket size %d\n", buckets));
  return buckets;
}
    

static int
commonHash (void *key)
{
  int *val = (int *)key;

  if (aidCache->_numBuckets == 1)
    return 0;
  else
    return (*val % aidCache->_numBuckets);  // aidCache->_numBuckets = sidCache->_numBuckets
}


static void*
aidData2Key (void *entry)
{
  return &(((AravoxTranslation *)entry)->assignedId);
}


static int
commonKeyCmp (void *key1, void *key2)
{
  int *val1 = (int *)key1;
  int *val2 = (int *)key2;

  return (*val1 - *val2);
}


static void*
sidData2Key (void *entry)
{
  return &(((AravoxTranslation *)entry)->sessionId);
}

#endif /* I86PC */

/**
 * called when it needs to re-read the config file
 *
 * @return -1 if config changed enough to cause a complete stop/start, 0 otherwise
 */
int
AravoxReconfig ()
{
  FirewallParams *tmp, *ptr, *curConfig, *newConfig;
  PPCParams *curPPC, *newPPC;
  PPCStats *tmpStats;
  AravoxTranslation *tmpList;
  PortAlloc *tmpPortAlloc;
  int i, numVerified, status;
  char str1[32];
  char *fn = "AravoxReconfig";

  /* re-read the file */
  if ((tmp = ReadAravoxXML()) == NULL)
  {
    NETERROR(MFCE, ("%s: Error in the new configuration, keeping the previous values\n", fn));
    return 0;  // config has not changed, the new config was not accepted
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: The configuration read is:\n", fn));
  AravoxPrintConfigParams(tmp);

  LockGetLock(lsMem->fwLock, 0, 0);

  /* if anything has changed, return -1 */
  ptr = (FirewallParams *)lsMem->fwParams;
  numVerified = 0;
  while (ptr != NULL)
  {
    curConfig = ptr;
    ptr = ptr->nextFirewall;
    newConfig = GetFirewallParamForMPAddr(tmp, curConfig->firewallMPAddr);
    if (newConfig == NULL)
    {
      /* the firewall has been deleted from the configuration, delete from the
	 internal structures. We don't return -1 here, because deleting a firewall
	 doesn't mean we need to restart connections with the rest of the firewalls */
      NETERROR(MFCE, ("%s: firewall %d (%s) has been removed from the new configuration\n", fn, curConfig->firewallConfigId, FormatIpAddress(curConfig->firewallMPAddr, str1)));
#ifndef I86PC
      AravoxFirewallShutdown(curConfig);
#endif
      continue; 
    }

    numVerified++;
    status = 0;
    /* see if any of this firewall's parameters have changed */
    if (curConfig->userId != newConfig->userId)
    {
      NETERROR(MFCE, ("%s: User Id for firewall %s has changed from %d to %d\n", fn, FormatIpAddress(curConfig->firewallMPAddr, str1), curConfig->userId, newConfig->userId));
      status = 1;  // need to re-init with this firewall
    }
    else if (curConfig->authType != newConfig->authType)
    {
      NETERROR(MFCE, ("%s: Auth Type for firewall %s has changed from %d to %d\n", fn, FormatIpAddress(curConfig->firewallMPAddr, str1), curConfig->authType, newConfig->authType));
      status = 1;  // need to re-init with this firewall
    }
    else if (curConfig->authType != NS_AUTH_NONE &&
	     strcmp(curConfig->authString, newConfig->authString) != 0)
    {
      NETERROR(MFCE, ("%s: Auth String for firewall %s has changed from %s to %s\n", fn, FormatIpAddress(curConfig->firewallMPAddr, str1), curConfig->authString, newConfig->authString));
      status = 1;  // need to re-init with this firewall
    }
    else if (curConfig->localNatEnabled != newConfig->localNatEnabled)
    {
      NETERROR(MFCE, ("%s: LocalNAT for firewall %s has changed from %d to %d\n", fn, FormatIpAddress(curConfig->firewallMPAddr, str1), curConfig->localNatEnabled, curConfig->localNatEnabled));
      status = 1;  // need to re-init with this firewall
    }
    else if (curConfig->remoteNatEnabled != newConfig->remoteNatEnabled)
    {
      NETERROR(MFCE, ("%s: RemoteNAT for firewall %s has changed from %d to %d\n", fn, FormatIpAddress(curConfig->firewallMPAddr, str1), curConfig->remoteNatEnabled, newConfig->remoteNatEnabled));
      status = 1;  // need to re-init with this firewall
    }

    /* if any of the firewall specific parameters have changed, reset
       connections with all of it's PPCs */
    if (status == 1)
    {
      NETERROR(MFCE, ("%s: firewall config changed, resetting connections with %s, firewall %d with new configurations\n", fn, FormatIpAddress(curConfig->firewallMPAddr, str1), curConfig->firewallConfigId));
#ifndef I86PC
      AravoxFirewallShutdown(curConfig);
      // now start the connections as specified in the new config
      AravoxFirewallInit(newConfig, 0);
#endif
      continue;
    }
    else
    {
      /* none of the firewall parameters changed, copy the auth data */
      memcpy(&newConfig->authDataBlock, &curConfig->authDataBlock, sizeof(nsAuthenticationData));
      newConfig->authData = (curConfig->authData == NULL)?NULL:&newConfig->authDataBlock;

      /* check if the PPC params have changed */
      for (i = 0; i < curConfig->numPPCs; i++)
      {
	curPPC = &curConfig->ppc[i];
	newPPC = GetPPCParamForDeviceId(newConfig, curPPC->subDeviceId);
	if (newPPC == NULL)
	{
	  // the PPC was deleted
	  NETERROR(MFCE, ("%s: Firewall %d, PPC %d has been deleted\n", fn, curConfig->firewallConfigId, curPPC->subDeviceId));
#ifndef I86PC
	  AravoxPPCShutdown(curPPC);
#endif
	  continue;
	}

	/* check the PPC params */
	status = 0;
	if (curPPC->role != newPPC->role)
	{
	  NETERROR(MFCE, ("%s: Role for firewall %d, PPC %d has changed from %d to %d\n", fn, curConfig->firewallConfigId, curPPC->subDeviceId, curPPC->role, newPPC->role));
	  status = 1;
	}
	else if (curPPC->adminStatus != newPPC->adminStatus)
	{
	  NETERROR(MFCE, ("%s: Admin status for firewall %d, PPC %d has changed from %s to %s\n", fn, curConfig->firewallConfigId, curPPC->subDeviceId, GetPPCStatusString(curPPC->adminStatus), GetPPCStatusString(newPPC->adminStatus)));
	  status = 1;
	}
	else if (curPPC->lowPort != newPPC->lowPort ||
		 curPPC->highPort != newPPC->highPort)
	{
	  NETERROR(MFCE, ("%s: Private port alloction for firewall %d, PPC %d has changed from %d-%d to %d-%d\n", fn, curConfig->firewallConfigId, curPPC->subDeviceId, curPPC->lowPort, curPPC->highPort, newPPC->lowPort, newPPC->highPort));
	  status = 1;
	}

	if (status == 1)
	{
	  NETERROR(MFCE, ("%s: PPC config changed, resetting connections with firewall %d, PPC %d\n", fn, curConfig->firewallConfigId, curPPC->subDeviceId));
#ifndef I86PC
	  AravoxPPCShutdown(curPPC);
	  AravoxPPCInit(newPPC, 0);
	  newPPC->stats->numTranslationFailed = curPPC->stats->numTranslationFailed;
	  newPPC->stats->lastKnownTranslationFailCode = curPPC->stats->lastKnownTranslationFailCode;
	  newPPC->stats->numPermissionFailed = curPPC->stats->numPermissionFailed;
	  newPPC->stats->lastKnownPermissionFailCode = curPPC->stats->lastKnownPermissionFailCode;
#endif
	}
	else
	{
	  /* no configuration has changed, copy the firewall id over */
	  newPPC->firewallAravoxId = curPPC->firewallAravoxId;
	  newPPC->operStatus = curPPC->operStatus;

	  /* take the statistics from the currentConfig and link to the newConfig,
	     we will be using the newConfig from now on... */
	  tmpStats = curPPC->stats;
	  curPPC->stats = newPPC->stats;
	  newPPC->stats = tmpStats;

	  /* move the translation list */
	  tmpList = curPPC->translationList;
	  curPPC->translationList = newPPC->translationList;
	  newPPC->translationList = tmpList;

	  /* move the port allocation info */
	  tmpPortAlloc = curPPC->portAlloc;
	  curPPC->portAlloc = newPPC->portAlloc;
	  newPPC->portAlloc = tmpPortAlloc;
	}
      }

      /* is any new PPC been added? */
      for (i = 0; i < newConfig->numPPCs; i++)
      {
	newPPC = &newConfig->ppc[i];
	curPPC = GetPPCParamForDeviceId(curConfig, newPPC->subDeviceId);
	if (curPPC != NULL)
	  continue;  // we already looked at this in the previous loop

	/* need to add this new PPC */
	NETERROR(MFCE, ("%s: New PPC (%d) added to firewall %d\n", fn, newPPC->subDeviceId, curConfig->firewallConfigId));
#ifndef I86PC
	AravoxPPCInit(newPPC, 0);
#endif
      }
    }
  }

  /* if the number of firewalls we verified is the same as the number of firewalls
     we just read, we have nothing to do */
  if (numVerified != GetFirewallCount(tmp))
  {
    ptr = tmp;
    while (ptr != NULL)
    {
      newConfig = ptr;
      ptr = ptr->nextFirewall;
      curConfig = GetFirewallParamForMPAddr((FirewallParams *)lsMem->fwParams, newConfig->firewallMPAddr);
      if (curConfig != NULL)
	continue;  // we already looked at this in the previous loop

      /* need to add this new firewall */
      NETERROR(MFCE, ("%s: New firewall %d added\n", fn, newConfig->firewallConfigId));
#ifndef I86PC
      AravoxFirewallInit(newConfig, 0);
#endif
    }
  }

  /* swap the parameters pointer */
  ptr = (FirewallParams *)lsMem->fwParams;
  lsMem->fwParams = tmp;

  LockReleaseLock(lsMem->fwLock);

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: current configuration is set to:\n", fn));
  AravoxPrintConfigParams((FirewallParams *)lsMem->fwParams);

  /* release the other set of configuration */
  DeleteFwParamAll(ptr);

  return 0;
}


/**
 * read the config parameters and initialize connection with the firewall so that
 * the other APIs can be used to open/close pinholes
 */
void
AravoxInit ()
{
  char *fn = "AravoxInit";
#ifndef I86PC
  FirewallParams *ptr;
  unsigned int status;
  int bucketSize;
#endif

  if (CheckFwLock())
    return;

  if ((lsMem->fwParams = ReadAravoxXML()) == NULL)
  {
    NETERROR(MFCE, ("%s: Cannot initialize Aravox firewall connection: no config parameters read\n", fn));
    return;
  }

#ifdef I86PC

  NETERROR(MFCE, ("%s: Cannot initialize Aravox firewall connection: feature not supported in i86pc architecture\n", fn));

#else

  /* initialize the cache */
  bucketSize = getHashtableBucketSize();
  aidCache = CacheCreate(CACHE_MALLOC_LOCAL);
  aidCache->dt = CACHE_DT_HASH;
  aidCache->_numBuckets = bucketSize;
  aidCache->_hashlistOffset = AID_LIST_OFFSET;
  aidCache->_hash = (int)commonHash;
  aidCache->_entry2key = (int)aidData2Key;
  aidCache->_keycmp = (int)commonKeyCmp;
  aidCache->lock = NULL;
  strcpy(aidCache->name, "FCE aid cache");
  if (!CacheInstantiate(aidCache))
  {
    NETERROR(MFCE, ("%s: Cannot initialize assigned ID cache\n", fn));
    DeleteFwParamAll((FirewallParams *)lsMem->fwParams);
    lsMem->fwParams = NULL;
    return;
  }

  sidCache = CacheCreate(CACHE_MALLOC_LOCAL);
  sidCache->dt = CACHE_DT_HASH;
  sidCache->_numBuckets = bucketSize;
  sidCache->_hashlistOffset = SID_LIST_OFFSET;
  sidCache->_hash = (int)commonHash;
  sidCache->_entry2key = (int)sidData2Key;
  sidCache->_keycmp = (int)commonKeyCmp;
  sidCache->lock = NULL;
  strcpy(sidCache->name, "FCE sid cache");
  if (!CacheInstantiate(sidCache))
  {
    NETERROR(MFCE, ("%s: Cannot initialize session ID cache\n", fn));
    CacheDestroy(aidCache);
    aidCache = NULL;
    DeleteFwParamAll((FirewallParams *)lsMem->fwParams);
    lsMem->fwParams = NULL;
    return;
  }

  /* initialize aravox voip library */
  status = nsVoipInit();
  if (status != NSVOIP_SUCCESS &&
      status != NSVOIP_ALREADY_INITIALIZED)
  {
    PrintStatusCode(0, fn, "Error initializing Aravox VoIP library", status);
    CacheDestroy(aidCache);
    aidCache = NULL;
    CacheDestroy(sidCache);
    sidCache = NULL;
    DeleteFwParamAll((FirewallParams *)lsMem->fwParams);
    lsMem->fwParams = NULL;
    return;
  }

  LockGetLock(lsMem->fwLock, 0, 0);
  /* initialize connection to each Firewall. */
  for (ptr = (FirewallParams *)lsMem->fwParams; ptr != NULL; ptr = ptr->nextFirewall)
  {
    AravoxFirewallInit(ptr, 0);
  }
  LockReleaseLock(lsMem->fwLock);
#endif

  AravoxPrintConfigParams((FirewallParams *)lsMem->fwParams);
}


/**
 * gracefully shuts down the session with the firewall, so that any pinholes opened
 * since the last time this connection was established will be closed now
 */
int
AravoxShutdown ()
{
  int retcode = TRUE;
#ifndef I86PC
  FirewallParams *ptr;
#endif

  LockGetLock(lsMem->fwLock, 0, 0);

#ifndef I86PC
  /* shutdown all active firewall connections, this take care of closing all the open holes */
  for (ptr = (FirewallParams *)lsMem->fwParams; ptr != NULL; ptr = ptr->nextFirewall)
  {
    if (AravoxFirewallShutdown(ptr) == FALSE)
      retcode = FALSE;
  }

  if (aidCache != NULL)
  {
    CacheDestroy(aidCache);
  }
  if (sidCache != NULL)
  {
    CacheDestroy(sidCache);
  }
#endif

  DeleteFwParamAll((FirewallParams *)lsMem->fwParams);
  lsMem->fwParams = NULL;

  LockReleaseLock(lsMem->fwLock);

  return retcode;
}


/**
 * stop firewall connection and release all resources
 */
void
AravoxRelease ()
{
  if (lsMem->fwLock != NULL)
  {
    AravoxShutdown();
    LockDestroy(lsMem->fwLock);
    SHM_Free(lsMem->fwLock);
  }
}


/**
 * check if any of the PPCs need re-init, i.e., if admin status is UP but oper status is
 * DOWN, initialize connection to that PPC
 */
void
AravoxCheckReinit ()
{
  FirewallParams *ptr;
  int i;

  LockGetLock(lsMem->fwLock, 0, 0);

  for (ptr = (FirewallParams *)lsMem->fwParams; ptr != NULL; ptr = ptr->nextFirewall)
  {
    for (i = 0; i < ptr->numPPCs; i++)
    {
      if (ptr->ppc[i].adminStatus == PPC_UP &&
	  ptr->ppc[i].operStatus == PPC_DOWN)
      {
#ifndef I86PC
	unsigned int status = FALSE;
	AravoxPPCShutdown(&ptr->ppc[i]);
	status = AravoxPPCInit(&ptr->ppc[i], 0);
	NETERROR(MFCE, ("AravoxCheckReinit: Firewall %d PPC %d needed re-init, re-init %s\n", ptr->firewallConfigId, ptr->ppc[i].subDeviceId, (status == TRUE)?"successful":"failed"));
#endif
      }
    }
  }

  LockReleaseLock(lsMem->fwLock);

}


/**
 * the iserver is now active, re-initialize with the firewall, if needed
 */
void AravoxServerActive ()
{
#ifndef I86PC
  FirewallParams *ptr;

  LockGetLock(lsMem->fwLock, 0, 0);

  /* initialize connection to each Firewall. */
  for (ptr = (FirewallParams *)lsMem->fwParams; ptr != NULL; ptr = ptr->nextFirewall)
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("AravoxServerActive: re-initing current fw connection\n"));
    AravoxFirewallShutdown(ptr);
    AravoxFirewallInit(ptr, 1);
  }

  LockReleaseLock(lsMem->fwLock);
#endif
}


/**
 * the iserver is now inactive, clear all the pinholes
 */
void AravoxServerInactive ()
{
#ifndef I86PC
//  FirewallParams *ptr;

//  LockGetLock(lsMem->fwLock, 0, 0);

  // when we come back up, we would have to force re-init
//  for (ptr = (FirewallParams *)lsMem->fwParams; ptr != NULL; ptr->connectionInitialized = FALSE, ptr = ptr->nextFirewall);

//  LockReleaseLock(lsMem->fwLock);

#endif
}
