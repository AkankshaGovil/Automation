/* vnsp.h */
/* This file contains the header definitions for an API     */
/* designed to allow a Voice over IP application to control */
/* a Aravox Convergent Firewall device.                   */
 
/* Copyright (c) 2000-2001 Aravox Technologies, Inc.                          */
/* Portions copyright (c) 1998 StorageTek Storage Network Business Group */                   

#ifndef NS_VNSP_H
#define NS_VNSP_H


#ifdef __STDC__
#define __ARGS(x) x
#else
#define __ARGS(x) ()
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#ifdef UNIX
#define __cdecl
#endif
 
#ifndef FALSE
#define FALSE 0
#endif
 
#ifndef TRUE
#define TRUE 1
#endif

/* Authentication defines and typedefs */

#define NS_AUTH_STRATEGY_PREFIX				0
#define NS_AUTH_STRATEGY_LOWER_EDGE			1	/* Do NOT renumber, or delete.  Add new entries to the end */
#define NS_AUTH_NONE						1
#define NS_AUTH_MD5							2
#define NS_AUTH_PASSWORD					3
#define NS_AUTH_SHA1						4
#define NS_AUTH_STRATEGY_UPPER_EDGE			4

#ifndef NS_AUTH_BAD_PARAM
#define NS_AUTH_BAD_PARAM 0xea4a0003
#endif

#ifndef NS_AUTH_SUCCESS
#define NS_AUTH_SUCCESS 0xea4a0001
#endif 


#ifdef NS_API_STRINGS
char nsAuthStrategyStr[][15] = {
	"none",
	"MD5",
	"password",
	"SHA1"
};
#endif

typedef struct _nsSHA1Data {
	unsigned char	messageDigest[20];
} nsSHA1Data;

typedef struct _nsMD5Data {
	unsigned char md5_secret[16];
} nsMD5Data;

#ifndef MAX_NAME_LENGTH
#define MAX_NAME_LENGTH 31 /* make sure that this + 1 is a multiple of 4! */
#endif

#define MAX_PASSWORD_LENGTH 11 /* make sure that this +1 is multiple of 4!! */

typedef struct _nsPassword {
	char			userName[MAX_NAME_LENGTH+1];
	char			password[MAX_PASSWORD_LENGTH+1];
} nsPassword;

typedef union _nsAuthenticationData {
		nsMD5Data	md5;
		nsPassword	password;
		nsSHA1Data	sha1;
		/*			none; */
} nsAuthenticationData;

/* Voip defines */
#define NSVOIP_API_MAJOR_VERSION	2
#define NSVOIP_API_MINOR_VERSION	2
#define NSVOIP_API_MAINT_VERSION	0


#define NSVOIP_FILTER_LIST_SAFETY_VALUE     0xa1881234
#define NSVOIP_FIREWALL_LIST_SAFETY_VALUE   0xa1885678
#define NSVOIP_TRASHCAN_SAFETY_VALUE		0xa1882468

#define NSVOIP_PROTOCOL_TCP                          6
#define NSVOIP_PROTOCOL_UDP                         17
#define NSVOIP_PROTOCOL_RTP                        255

#define NSVOIP_ONE_WAY                               1
#define NSVOIP_TWO_WAY                               2

#define NSVOIP_FIREWALL_TYPE_PORTABLE_NS    0xa1880001
#define NSVOIP_SERVER_TYPE_PORTABLE_NS		0xa1880001	/* replaced by above #define, but keep for Siemens */
#define NSVOIP_FIREWALL_TYPE_SLIC           0xa1880002
#define NSVOIP_SERVER_TYPE_SLIC		        0xa1880002	/* replaced by above #define, but keep for Siemens */
#define NSVOIP_FIREWALL_TYPE_ARAVOX			0xa1880003
#define NSVOIP_FIREWALL_TYPE_NETSENTRY_II   0xa1880003  /* replaced by above #define, but keep for Siemens */

#define NSVOIP_STATUS_LOWER_EDGE			0xa1881001
#define NSVOIP_ALREADY_INITIALIZED          0xa1881001
#define NSVOIP_BAD_ADDRESS1					0xa1881002
#define NSVOIP_BAD_SOURCE_ADDRESS			0xa1881002	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_ADDRESS2					0xa1881003
#define NSVOIP_BAD_DEST_ADDRESS				0xa1881003	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_AUTH_DATA				0xa1881004
#define NSVOIP_BAD_AUTH_TYPE				0xa1881005
#define NSVOIP_BAD_DEVICE_ID				0xa1881006
#define NSVOIP_BAD_FIREWALL_ADDRESS			0xa1881007
#define NSVOIP_BAD_SERVER_ADDRESS			0xa1881007	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_FIREWALL_ID				0xa1881008
#define NSVOIP_BAD_SERVER_ID				0xa1881008	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_FIREWALL_TYPE			0xa1881009
#define NSVOIP_BAD_SERVER_TYPE				0xa1881009	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_GATEWAY_ADDRESS			0xa188100a
#define NSVOIP_BAD_GATEWAY_PORT				0xa188100b
#define NSVOIP_BAD_PERMISSION_ID			0xa188100c
#define NSVOIP_BAD_TAG						0xa188100c	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_PORT1					0xa188100d
#define NSVOIP_BAD_SOURCE_PORT				0xa188100d	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_PORT2					0xa188100e
#define NSVOIP_BAD_DEST_PORT				0xa188100e	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_PROTOCOL					0xa188100f
#define NSVOIP_BAD_SESSION_ID				0xa1881010
#define NSVOIP_BAD_SESSION					0xa1881010	/* obsoleted by above #define, but keep for Siemens */
#define NSVOIP_BAD_USER_ID					0xa1881011
#define NSVOIP_COMMUNICATION_ERROR          0xa1881012
#define NSVOIP_MEMORY_ALLOCATION_ERROR      0xa1881013
#define NSVOIP_MEMORY_CORRUPTION_ERROR      0xa1881014
#define NSVOIP_NOT_INITIALIZED				0xa1881015
#define NSVOIP_PROVISIONING_ERROR           0xa1881016
#define NSVOIP_SUCCESS                      0xa1881017
#define NSVOIP_SYSTEM_ERROR					0xa1881018
#define NSVOIP_BAD_DIRECTION				0xa1881019
#define NSVOIP_NO_MATCHING_RULE				0xa188101a
#define NSVOIP_BAD_TRANSLATION_ID			0xa188101b
#define NSVOIP_BAD_NAT_ENABLED              0xa188101c
#define NSVOIP_BAD_PRIMARY_CONTROLLER       0xa188101d
#define NSVOIP_NAT_NOT_ENABLED              0xa188101e
#define NSVOIP_DUPLICATE_ENTRY				0xa188101f
#define NSVOIP_SESSION_ID_IN_USE			0xa1881020	/* returned by nsVoipSessionIdInUseQuery */
#define NSVOIP_SESSION_ID_AVAILABLE			0xa1881021	/* returned by nsVoipSessionIdInUseQuery */
#define NSVOIP_NO_MATCHING_SESSION			0xa1881022
#define NSVOIP_BAD_RETURNED_ADDRESS			0xa1881023
#define NSVOIP_BAD_RETURNED_PORT			0xa1881024
#define NSVOIP_BAD_RETURNED_TRANSLATION_ID	0xa1881025
#define NSVOIP_BAD_RETURNED_TRANSLATION_SIZE	0xa1881026
#define NSVOIP_BAD_RETURNED_TRANSLATION		0xa1881027
#define NSVOIP_BAD_TRANSLATION_TYPE			0xa1881028
#define NSVOIP_BAD_TRANSLATION_NAME			0xa1881029
#define NSVOIP_BAD_TRANSLATION_SIZE			0xa188102a
#define NSVOIP_STATUS_UPPER_EDGE			0xa188102a

#define NSVOIP_FILTER_NAME "nsvoip_permit\0"

#define NSVOIP_NAT_NAME "nsvoip_nat\0"

#define NSVOIP_MAX_STATUS_TEXT_LENGTH       256
#define NSVOIP_MAX_CODE_STRING_LENGTH		256

  
typedef struct _aravoxNat5Tuple {
        unsigned int    sourceIpAddress;
        unsigned int    destinationIpAddress;
        unsigned short  sourcePort;
        unsigned short  destinationPort;
        unsigned char   protocol;
} aravoxNat5Tuple;

typedef struct _aravoxNatRule {
        char                    ruleName[MAX_NAME_LENGTH+1];
        unsigned int    ruleType;
        unsigned int    natId;
        unsigned int    tag;
        unsigned int    flags;
        unsigned int    allowedTranslationCount;
        aravoxNat5Tuple match;
        aravoxNat5Tuple mask;
        aravoxNat5Tuple packet;
        aravoxNat5Tuple alloc;
        aravoxNat5Tuple alloc_low;
        aravoxNat5Tuple alloc_hi;
} aravoxNatRule;     


#ifdef NSVOIP_STRINGS
char nsVoipStatusStr[][NSVOIP_MAX_CODE_STRING_LENGTH+1]={
	"NSVOIP_ALREADY_INITIALIZED",
	"NSVOIP_BAD_ADDRESS1",
	"NSVOIP_BAD_ADDRESS2",
	"NSVOIP_BAD_AUTH_DATA",
	"NSVOIP_BAD_AUTH_TYPE",
	"NSVOIP_BAD_DEVICE_ID",
	"NSVOIP_BAD_FIREWALL_ADDRESS",
	"NSVOIP_BAD_FIREWALL_ID",
	"NSVOIP_BAD_FIREWALL_TYPE",
	"NSVOIP_BAD_GATEWAY_ADDRESS",
	"NSVOIP_BAD_GATEWAY_PORT",
	"NSVOIP_BAD_PERMISSION_ID",
	"NSVOIP_BAD_PORT1",
	"NSVOIP_BAD_PORT2",
	"NSVOIP_BAD_PROTOCOL",
	"NSVOIP_BAD_SESSION_ID",
	"NSVOIP_BAD_USER_ID",
	"NSVOIP_COMMUNICATION_ERROR",
	"NSVOIP_MEMORY_ALLOCATION_ERROR",
	"NSVOIP_MEMORY_CORRUPTION_ERROR",
	"NSVOIP_NOT_INITIALIZED",
	"NSVOIP_PROVISIONING_ERROR",
	"NSVOIP_SUCCESS",
	"NSVOIP_SYSTEM_ERROR",
	"NSVOIP_BAD_DIRECTION",
	"NSVOIP_NO_MATCHING_RULE",
	"NSVOIP_BAD_TRANSLATION_ID",
	"NSVOIP_BAD_NAT_ENABLED",
	"NSVOIP_BAD_PRIMARY_CONTROLLER",
	"NSVOIP_NAT_NOT_ENABLED",
	"NSVOIP_DUPLICATE_ENTRY",
	"NSVOIP_SESSION_ID_IN_USE",
	"NSVOIP_SESSION_ID_AVAILABLE",
	"NSVOIP_NO_MATCHING_SESSION",
	"NSVOIP_BAD_RETURNED_ADDRESS",
	"NSVOIP_BAD_RETURNED_PORT",
	"NSVOIP_BAD_RETURNED_TRANSLATION_ID",
	"NSVOIP_BAD_RETURNED_TRANSLATION_SIZE",
	"NSVOIP_BAD_RETURNED_TRANSLATION",
	"NSVOIP_BAD_TRANSLATION_TYPE",
	"NSVOIP_BAD_TRANSLATION_NAME",
	"NSVOIP_BAD_TRANSLATION_SIZE"
};
#endif


/* function prototypes for externally visible functions */


int __cdecl
nsConvertStringToMD5Secret(
		char *stringToConvert, 
		nsAuthenticationData *authenticationData);

int __cdecl 
nsConvertStringToSHA1Secret  (
		char *stringToConvert,
		nsAuthenticationData *authenticationData);

unsigned int __cdecl
nsVoipInit();

unsigned int __cdecl
nsVoipFirewallInit(
		unsigned int firewallIpAddress,				/* in  - Administrative IP address of the firewall     */
		unsigned int firewallType,					/* in  - Identifies what type of unit the firewall is  */
		unsigned int userId,						/* in  - Valid user id for loggin in to the firewall   */
		unsigned int authenticationType,			/* in  - Authentication method used for this user id   */
		nsAuthenticationData *authenticationData,	/* in  - Data used to authenticate this user id        */
		unsigned int subDeviceId,					/* in  - which device within firewall to connect to    */
		unsigned int h323GatewayIpAddress,			/* in  - IP address of VOIP gateway                    */
		unsigned short h323GatewayPort,				/* in  - VOIP gateway TCP port for H.323 connections   */
		unsigned int natEnabled,                    /* in  - TRUE = doing NAT on this firewall             */
		unsigned int primaryController,             /* in  - TRUE = this is the primary control connection */
		unsigned int *returnedFirewallId);			/* out - unique identifier for this firewall           */

unsigned int __cdecl 
nsVoipOpenPermission3(
		unsigned int firewallId,				/* in  - firewall identifier                            */
		unsigned int ipAddress1,				/* in  - first IP address                               */
		unsigned short port1,					/* in  - port at first IP address                       */
		unsigned int ipAddress2,				/* in  - second IP address                              */
		unsigned short port2,					/* in  - port at second IP address                      */
		unsigned int protocol,					/* in  - IP protocol to permit for this opening         */
		unsigned int sessionId,					/* in  - phone call session number                      */
		unsigned int direction,					/* in  - NSVOIP_ONE_WAY (to) or NSVOIP_TWO_WAY (tofrom) */
		unsigned int timeout,					/* in  - timeout in seconds								*/
		unsigned int bandwidthMaximum,			/* in  - bandwidth maximum in bits per second			*/
		unsigned int *returnedPermissionId);	/* out - unique identifier for this firewall opening    */

unsigned int __cdecl
nsVoipClosePermission(
		unsigned int firewallId,	/* in  - firewall identifier                */
		unsigned int permissionId);	/* in  - identifier of opening to be closed */

unsigned int __cdecl
nsVoipCloseSession(
		unsigned int firewallId,	/* in  - firewall identifier                */
		unsigned int sessionId);	/* in  - phone call session number to close */

unsigned int __cdecl
nsVoipFirewallShutdown(
		unsigned int firewallId);	/* in  - firewall identifier */

unsigned int __cdecl
nsVoipGetStatusText(
		char *returnedMessage);		/* out - most recent status message */

unsigned int __cdecl
nsVoipGetStatusCodeString(
		unsigned int code,			/* in  - numeric code returned from another API function */
		char *returnedCodeString);	/* out - ASCII name for the return code                  */

unsigned int __cdecl
nsVoipDeleteTranslationById(
		unsigned int firewallId,				/* in - firewall identifier											*/
		unsigned int translationId);			/* in - identifier of translation to be deleted						*/

unsigned int __cdecl
nsVoipDeleteTranslationBySession(
		unsigned int firewallId,				/* in - firewall identifier											*/
		unsigned int sessionId);				/* in - identifier of session translations are to be deleted from	*/

unsigned int __cdecl
nsVoipInstallRawTranslationCopy(
		unsigned int firewallId,				/* in  - firewall identifier to install the translation on 			*/
		unsigned int sessionId,					/* in  - the session to which this translation belongs				*/
		unsigned int sizeOfTranslation,			/* in  - size of the translation data structure						*/
		aravoxNatRule *translation,				/* in  - address of 'black box' translation							*/
		unsigned int *returnedTranslationId);	/* out - identifier of translation installed						*/

unsigned int __cdecl
nsVoipTranslateLocalAddressAndGetRaw(
		unsigned int firewallId,				/* in  - firewall identifier										*/
		unsigned int protocol,					/* in  - Either TCP, UDP or RTP											*/
		unsigned int localIpAddress,			/* in  - local IP address (before translation)						*/
		unsigned short localPort,				/* in  - local port (before translation)							*/
		unsigned int remoteIpAddress,			/* in  - remote IP address											*/
		unsigned int remoteIpAddressMask,		/* in  - defines which bits are known in the remote IP address		*/
		unsigned short remotePort,				/* in  - remote port												*/
		unsigned short remotePortMask,			/* in  - defines which bits are known in the remote port			*/
		unsigned int numExpectedConnections,	/* in  - # of expected incoming connections for this translation	*/
		unsigned int sessionId,					/* in  - the session to which this translation belongs				*/
		unsigned int *returnedLocalIpAddress,	/* out - translated local IP address								*/
		unsigned short *returnedLocalPort,		/* out - translated local port										*/
		unsigned int *returnedTranslationId,	/* out - identifier of translation installed						*/
		unsigned int *returnedSizeOfTranslation,		/* out - size of the translation data structure						*/
		aravoxNatRule *returnedTranslation);				/* out - address of where to place the 'black box' translation		*/


unsigned int __cdecl
nsVoipTranslateRemoteAddressAndGetRaw(
		unsigned int firewallId,				/* in  - firewall identifier										*/
		unsigned int protocol,					/* in  - Either TCP, UDP or RTP										*/
		unsigned int remoteIpAddress,			/* in  - remote IP address (before translation)						*/
		unsigned short remotePort,				/* in  - remote port (before translation)							*/
		unsigned int localIpAddress,			/* in  - local IP address											*/
		unsigned int localIpAddressMask,		/* in  - defines which bits are known in the local IP address		*/
		unsigned short localPort,				/* in  - local port													*/
		unsigned short localPortMask,			/* in  - defines which bits are known in the local port				*/
		unsigned int numExpectedConnections,	/* in  - # of expected incoming connections for this translation	*/
		unsigned int sessionId,					/* in  - the session to which this translation belongs				*/
		unsigned int *returnedRemoteIpAddress,	/* out - translated remote IP address								*/
		unsigned short *returnedRemotePort,		/* out - translated remote port										*/
		unsigned int *returnedTranslationId,	/* out - identifier of translation installed						*/
		unsigned int *returnedSizeOfTranslation,/* out - size of the translation data structure						*/
		aravoxNatRule *returnedTranslation);		/* out - address of where to place the 'black box' translation		*/


#ifdef __cplusplus
}
#endif

#endif
/* end */

