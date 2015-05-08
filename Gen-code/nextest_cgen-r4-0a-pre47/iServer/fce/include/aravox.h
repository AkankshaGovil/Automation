#ifndef __ARAVOX_H
#define __ARAVOX_H

#include "serverp.h"
#include "vnsp.h"
#include "firewallcontrol.h"


typedef struct _aravoxTranslation AravoxTranslation;

struct _aravoxTranslation {
  struct _aravoxTranslation *sidPrev, *sidNext;
  struct _aravoxTranslation *aidPrev, *aidNext;
  struct _aravoxTranslation *ppcPrev, *ppcNext;

  unsigned int assignedId;
  unsigned int sessionId;
  unsigned int mpAddr;
  unsigned int devId;

  int numPids;
  unsigned int *pidArray;
  int numTids;
  unsigned int *tidArray;

  int privatePortsAllocated;

  int increment;

  int count;

  int port;   // is we allocated any private port, it goes here
};


#define SID_LIST_OFFSET  (0)
#define AID_LIST_OFFSET  (2*sizeof(struct _aravoxTranslation *))
#define PPC_LIST_OFFSET  (4*sizeof(struct _aravoxTranslation *))

#include "readAravox.h"

extern void AravoxInit (void);
/**
 * This is to open a pinhole and a nat translation entry on the firewall.
 * The pinholes are opened from ANY address to the given media address.
 *
 * @param protocol The protocol of the media packets, IPPROTO_UDP, IPPROTO_TCP or
 * IPPROTO_RAW (for RTP)
 * @param mediaIpAddress The IP address that will receive the media packets
 * @param mediaPort The port number that will receive the media packets
 * @param direction The traffic direction
 * @param sessionIdCurrentLeg An optional session ID, which could be used later to clear
 * multiple pinholes (possibly related to one phone call). Use 0 if no such
 * grouping is desired.
 * @param sessionIdLeg2 the session id for the other leg of the call
 * @param numTranslations The number of TCP/RTP connections to allow on this pinhole
 * @param returnedMediaIpAddress The media IP address to use in the SDP
 * @param returnedMediaPort The media port to use in the SDP
 *
 * @return 0 for success and -1 for failure
 */
extern int AravoxOpenHole (unsigned int protocol,
			   unsigned int mediaIpAddress,
			   unsigned short mediaPort,
			   eFCETrafficDirection direction,
			   unsigned int sessionIdCurrentLeg,
			   unsigned int sessionIdLeg2,
			   int numTranslations,
			   unsigned int *returnedMediaIpAddress,
			   unsigned short *returnedMediaPort,
			   int allocatePrivatePorts);
extern int AravoxReopenHole (unsigned int protocol,
			     unsigned int mediaIpAddress,
			     unsigned short mediaPort,
			     eFCETrafficDirection direction,
			     unsigned int sessionIdCurrentLeg,
			     unsigned int sessionIdLeg2,
			     int numTranslations,
			     unsigned int *returnedMediaIpAddress,
			     unsigned short *returnedMediaPort,
			     int allocatePrivatePorts);
extern int AravoxCloseHole (unsigned int permissionId);
extern int AravoxCloseSession (unsigned int sessionId);
extern int AravoxShutdown (void);
extern int AravoxReconfig (void);
extern unsigned int AravoxGetSigPublicAddress (unsigned int addr);
extern unsigned int validateAravoxConfig (FirewallParams *params);
extern void AravoxRelease (void);
extern void AravoxPrintConfigParams (FirewallParams*);
extern void AravoxPrintDebug (void);
extern void AravoxCheckReinit (void);
extern void AravoxServerActive (void);
extern void AravoxServerInactive (void);
#endif
