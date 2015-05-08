#ifndef _READ_ARAVOX_H
#define _READ_ARAVOX_H

#include "aravox.h"
#include "portalloc.h"

#define ARAVOX_CONFIG_FILENAME "/usr/local/nextone/bin/aravox.xml"

#define ARAVOXCFG_TAG         "AravoxCfg"
#define FIREWALL_TAG          "Firewall"
#define IP_ADDRESS_TAG        "IpAddress"
#define PUBLIC_ADDRESS_TAG    "PublicAddress"
#define PPC_TAG               "PPC"
#define USER_ID_TAG           "UserId"
#define ID_TAG                "id"
#define AUTH_TAG              "Auth"
#define AUTH_DATA_TAG         "AuthData"
#define NAT_TAG               "NAT"
#define SIGNALING_TAG         "Signaling"
#define PACKET_STEERING_TAG   "PacketSteering"
#define PPC_STATS_TAG         "PPCStats"
#define TRANSLATION_TAG       "translationFailed"
#define PERMISSION_TAG        "permissionFailed"
#define PORTALLOC_TAG         "PortAlloc"

#define PINHOLES_ATTR         "pinholes"
#define TOTAL_ATTR            "total"
#define CODE_ATTR             "code"
#define ID_ATTR               "id"
#define ENABLE_ATTR           "enable"
#define IP_ADDRESS_ATTR       IP_ADDRESS_TAG
#define LOW_ATTR              "low"
#define HIGH_ATTR             "high"

#define TRUE_STR              "TRUE"
#define FALSE_STR             "FALSE"

/*
 * The MAX PPC Cards for an Aravox 3100 is 3.
 */
#define MAX_PPC_CARDS 3
/*#define MAX_FIREWALLS 50*/

/* Restrictions on XML formatting */
#define MAX_TAG_NAME_LEN 128
#define MAX_ELEM_DATA_LEN 128

/**
 * Redundnancy role of PPC.
 */
#define ROLE_ATTR    "role"
#define MASTER_ROLE  "MASTER"
#define ACTIVE_ROLE  "ACTIVE"
#define STANDBY_ROLE "STANDBY"
#define UNKNOWN_ROLE "UNKNOWN"
typedef enum 
{
  MASTER,
  ACTIVE,
  STANDBY,
  UNKNOWN  
} tROLE;

/**
 * Authorization types.
 */
#define TYPE_ATTR              "type"
#define MD5_TYPE               "MD5"
#define SHA1_TYPE              "SHA1"
#define NONE_TYPE              "NONE"

/**
 * PPC admin status
 */
#define ADMIN_STATUS_ATTR      "adminStatus"
#define PPC_ADMIN_UP           "UP"
#define PPC_ADMIN_DOWN         "DOWN"

typedef enum
{
  PPC_UP,
  PPC_DOWN
} tPPCStatus;

typedef struct _firewallParams FirewallParams;
typedef struct _ppcParams PPCParams;
typedef struct _ppcStats PPCStats;

/*
 * statistics for each PPC card
 */
struct _ppcStats {
  /* The number of pinholes on the PPC card */
  unsigned int pinholes;

  /* The number of times calls to translation API have failed */
  unsigned int numTranslationFailed;

  /* last known translation failed error code */
  unsigned int lastKnownTranslationFailCode;

  /* The number of times call to pinhole API have failed */
  unsigned int numPermissionFailed;

  /* last known pinhole failed error code */
  unsigned int lastKnownPermissionFailCode;
};

/*
 * The parameters unique to a PPC card.
 */
struct _ppcParams {
  /* TRUE if this PPC is to be used for signaling. */
  unsigned int signalingEnabled;

  /* Address advertised to external hosts. */
  unsigned int publicAddr;

  /* The firewall ID returned by the aravox firewall */
  unsigned int firewallAravoxId;

  /* The device ID of this PPC card. */
  unsigned int subDeviceId;

  /* Specifiy the Role of the PPC. This can be either MASTER, ACTIVE or STANDBY */
  tROLE role;

  /* administration status, to set the PPC up/down */
  tPPCStatus adminStatus;

  /* current operational status of the PPC */
  tPPCStatus operStatus;

  /* the firewall ptr in the internal data structure to which this PPC belongs to */
  FirewallParams *pFirewall;

  /* Statistics */
  PPCStats *stats;

  /* the aravox translations list for this ppc */
  AravoxTranslation *translationList;

  /* the ip address to be used in the port allocation */
  unsigned long portAllocIpAddr;

  /* the low port number in the allocation range */
  int lowPort;

  /* the high port number in the allocation range */
  int highPort;

  /* the port allocation structure for this PPC */
  PortAlloc *portAlloc;
};

/*
 * The parameters unique to a firewall.
 */
struct _firewallParams {
  /* The firewallID is unique for each PPC card, configured by the user */
  unsigned int firewallConfigId;

  /* IP Address of the firewall Processor Card */
  unsigned long firewallMPAddr;

  /* Authentication information */
  unsigned int userId;
  unsigned int authType;
  char authString[MAX_ELEM_DATA_LEN];
  nsAuthenticationData *authData;
  nsAuthenticationData authDataBlock;

  /* PPC specific data */
  unsigned int numPPCs;
  PPCParams ppc[MAX_PPC_CARDS];

  /* Flag specifying whether NAT is enabled for the private network. Make all PPCs in a chassis the same. */
  unsigned int localNatEnabled;

  /* Flag specifying whether Packet Steering is enabled. Make all PPCs in a chassis the same. */
  unsigned int remoteNatEnabled;

  /* pointer to the previous firewall structure */
  FirewallParams *prevFirewall;

  /* pointer to the next firewall structure */
  FirewallParams *nextFirewall;
};


extern FirewallParams* ReadAravoxXML (void);
extern FirewallParams* DeleteFwParamOne (FirewallParams*);
extern void DeleteFwParamAll (FirewallParams*);
extern char* GetPPCStatusString (tPPCStatus status);

#endif
