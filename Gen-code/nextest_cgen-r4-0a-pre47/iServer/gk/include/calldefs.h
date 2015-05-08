#ifndef __CALL_DEFS_H__
#define __CALL_DEFS_H__
// This file contains all defines that do not have any dependancy on any other header file

#include "mfcp.h"
#include "call_details.h"

#define CALL_ID_LEN			256	
#define CONF_ID_LEN			256	
#define GUID_LEN			16

#define CALL_ID_STRLEN		CALL_ID_LEN	
#define CONF_ID_STRLEN		CONF_ID_LEN	

#define CALL_LEG1			1
#define CALL_LEG2			2
#define USER_USER_STR_SIZE		132
#define MAX_CONF_CALLS			2
#define MAX_ISDN_CAUSE	128

#define	 EVENT_STRLEN	80

#if 0
#define FCE_NOSTATUS				0 
#define FCE_INPROGRESS              (1 << 0)					// bit 1
#define FCE_FAILURE					(1 << 1)					// bit 2
#define FCE_SUCCESS					(1 << 2)					// bit 3
#define FCE_OP_NOOP					((1 << 3) | FCE_SUCCESS)	// bit 4,5,6 (7 values)
#define FCE_OP_OPEN					((2 << 3) | FCE_SUCCESS)
#define FCE_OP_MODIFY				((3 << 3) | FCE_SUCCESS)
#define FCE_OP_CLOSE				((4 << 3) | FCE_SUCCESS)
#define FCE_GENERAL_FAILURE         ((1 << 6) | FCE_FAILURE) 	// bit 7,8 (3 values)
#define FCE_LICENSE_FAILURE         ((2 << 6) | FCE_FAILURE) 
#endif

typedef enum  {
	SCC_eH323CallHandle,
	SCC_eSipCallHandle,
	SCC_eMaxCallHandleType,
	} SCC_CallHandleType ;

typedef enum
{
	SCC_errorNone = 0,

	SCC_errorBusy = 1,
	SCC_errorAbandoned = 2,		// The caller disconnected before call
	SCC_errorInvalidPhone = 3,
	SCC_errorBlockedUser = 7,	// cmReasonTypeNoPermision
	SCC_errorNetwork = 12,
	SCC_errorNoRoute = 13,		// cmReasonTypeUnreachableDestination
	SCC_errorNoPorts = 14,		// cmReasonTypeUnreachableDestination
	SCC_errorGeneral = 15,
	SCC_errorMaxCallDuration = 99,
	SCC_errorResourceUnavailable = 1001,	
	SCC_errorDestinationUnreachable = 1002,
	SCC_errorUndefinedReason = 1003,
	SCC_errorInadequateBandwidth = 1004,
	SCC_errorH245Incomplete = 1005,
	SCC_errorIncompleteAddress = 1006,
	SCC_errorLocalDisconnect = 1007,
	SCC_errorH323Internal = 1008,
	SCC_errorH323Protocol = 1009,
	SCC_errorNoCallHandle = 1010,
	SCC_errorGatewayResourceUnavailable = 1011,
	SCC_errorFCECallSetup = 1012,
	SCC_errorFCE = 1013,
	SCC_errorNoVports = 1014,
	SCC_errorHairPin = 1015,
	SCC_errorShutdown = 1016,
	SCC_errorDisconnectUnreachable = 1017,
	SCC_errorTemporarilyUnavailable = 1018,
	SCC_errorSwitchover = 1019,
	SCC_errorDestRelComp = 1020, // Call Dropped before connect by dest, check ISDN CC
	SCC_errorNoFCEVports = 1021,
	SCC_errorH323MaxCalls = 1022,
	SCC_errorMswInvalidEpId = 1023,
	SCC_errorMswRouteCallToGk = 1024,
	SCC_errorMswCallerNotRegistered = 1025,
	SCC_errorDestBlockedUser = 1026,	
	SCC_errorDestNoRoute = 1027,
	SCC_errorDestTimeout = 1028,
	SCC_errorDestGone = 1029,
	SCC_errorRejectRoute = 1030,
	SCC_errorAdmin = 1031,
	SCC_errorSCM = 1032,
	SCC_errorNoNATTLicense = 1033,

	SCC_errorSipRedirect = 5300,
	SCC_errorSipAuthRequired = 5401,
	SCC_errorSipForbidden = 5403,
	SCC_errorSipProxyAuthRequired = 5407,
	SCC_errorSipInternalError = 5500,
	SCC_errorSipNotImplemented = 5501,
	SCC_errorSipServiceUnavailable = 5503,

} SCC_CallError;

#if 0 /* moved to ls/rpc/call_details.h */ 
typedef struct
{
	int callError;
	int lastEvent;
	int flags;
	#define HUNT_TRIGGER 0x1
	#define REDIRECT	 0x2

	int	callType;		//either CAP_SIP or CAP_H323
	MFCP_Session	*fceSession;	//MFCP session where the following resource resides
	unsigned int	fceResource;	//resource id associated with media holes on this call
	int	fceStatus;		//either FCE_SUCCESS or FCE_GENERAL_FAILURE

	// H.323
	int	cause;			// ISDN CC
	int	h225Reason;		
	int	rasReason;	

	// SIP
	int	responseCode;

} CallDetails;
#endif

typedef enum 
{
	SCC_evtNone = 0,
	SCC_evtInvalid,
	SCC_evtUnknown,
	SCC_evtHunt,
	SCC_evtResolve,
	SCC_evt323SetupTx,
	SCC_evt323SetupRx,
	SCC_evt323ProcTx,
	SCC_evt323ProcRx,
	SCC_evt323AlertTx,
	SCC_evt323AlertRx,
	SCC_evt323ConnTx,
	SCC_evt323ConnRx,
	SCC_evt323ARQRx,
	SCC_evt323ACFTx,
	SCC_evt323ARQTx,
	SCC_evt323ACFRx,
	SCC_evt323ARJRx,
	SCC_evt323ARQTmout,
	SCC_evt323ARJTx,
	SCC_evt323DestARQRx,
	SCC_evt323DestACFTx,
	SCC_evt323NewCall,
	SCC_evt323LRQTx,
	SCC_evt323LRQRx,
	SCC_evt323LCFRx,
	SCC_evt323LRJRx,
	SCC_evt323LRJTx,
	SCC_evt323LRQTmout,
	SCC_evtSipInviteTx,
	SCC_evtSipInviteRx,
	SCC_evtSipReInviteTx,
	SCC_evtSipReInviteRx,
	SCC_evtSip100xTx,
	SCC_evtSip100xRx,
	SCC_evtSip200Tx,
	SCC_evtSip200Rx,
	SCC_evtSipAckTx,
	SCC_evtSipAckRx,
	SCC_evtSip3xxRx,
	SCC_evtSipInviteRedirected,
	SCC_evtSip491Tx,
	SCC_evtSip491Rx,
	SCC_evtSip491TimerExp,
	SCC_evt323ProgTx,
	SCC_evt323ProgRx,
	SCC_evt323DRQForceDropRx,
	SCC_evtSipAuthReqRx,
	SCC_evtSipAuthReqTmout,
	SCC_evtSipReferTx,
	SCC_evtSip202Rx,
	SCC_evtSipNotifyTx,
} SCC_Evt;

/* from Q.850 */
typedef enum {
Cause_eUnassignedNumber = 1,
Cause_eNoRouteTransit = 2,
Cause_eNoRouteDest = 3,
Cause_eNormalCallClearing = 16,
Cause_eUserBusy = 17,
Cause_eUserNotResponding = 18,
Cause_eUserNoAnswer = 19, /* Alerted */
Cause_eSubscriberAbsent = 20,
Cause_eCallRejected = 21, 
Cause_eNumberChanged = 22,
Cause_eNonSelectedUser = 26,
Cause_eDestinationOutOfOrder = 27,
Cause_eInvalidNumberFormat = 28,
Cause_eNormalUnspecified = 31,
Cause_eNoCircuitAvailable = 34,
Cause_eNetworkOutOfOrder = 38,
Cause_eTemporaryFailure = 41, 
Cause_eSwitchingEquipmentCongestion = 42,
Cause_eNoResource= 47,
Cause_eIncomingClassBarredInCUG = 55,
Cause_eBearerCapNotAuthorized = 57,
Cause_eBearerCapNotPresentlyAvailable = 58,
Cause_eServiceOrOptionNotAvailable = 63,
Cause_eServiceOrOptionNotImplemented = 79,
Cause_eInvalidTransit = 91,
Cause_eRecoveryOnExpiresTimeout = 102,
Cause_eInterworking = 127,
Cause_eMax= 128 

} Cause_eRelease;

#define HuntError(_e_) ((_e_==SCC_errorNetwork)||\
						(_e_==SCC_errorDestNoRoute)||\
						(_e_==SCC_errorGeneral)||\
						(_e_==SCC_errorResourceUnavailable)||\
						(_e_==SCC_errorH323Protocol)||\
						(_e_==SCC_errorDestinationUnreachable)||\
						(_e_==SCC_errorUndefinedReason)||\
						(_e_==SCC_errorLocalDisconnect)||\
						(_e_==SCC_errorGatewayResourceUnavailable)||\
						(_e_==SCC_errorDisconnectUnreachable)||\
						(_e_==SCC_errorMswInvalidEpId)||\
						(_e_==SCC_errorMswCallerNotRegistered)||\
						(_e_==SCC_errorTemporarilyUnavailable)||\
						(_e_==SCC_errorDestTimeout)||\
						(_e_==SCC_errorSipInternalError)||\
						(_e_==SCC_errorSipServiceUnavailable))

char * CallGetErrorStr(int callerrno);
char * CallGetEvtStr(int evt);

char * getIsdnCCString(int cause);
#endif
