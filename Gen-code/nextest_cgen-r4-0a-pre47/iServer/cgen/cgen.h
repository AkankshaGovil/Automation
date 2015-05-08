/////////////////////////////////////////////////////////////////////
//
//	Name:
//		cgen.h
//
//	Description:
//			The contains includes and structures used by gen
//		program.
//
/////////////////////////////////////////////////////////////////////

#ifndef _CGEN_H_
#define	_CGEN_H_

#ifdef _WIN32

	#include <process.h>
	#include <windows.h>
	#include <conio.h>

#else

	#include <sys/types.h>
	#include <inttypes.h>
#ifndef NETOID_LINUX
	#include <sys/processor.h>
	#include <sys/procset.h>
#endif
	#include <pthread.h>
	#include <sys/resource.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <signal.h>
	#include <errno.h>

	#include <cm.h>

	//#define USE_RTP
	#ifdef USE_RTP
		#include "rtptest.h"
	#endif

	#include "fdsets.h"
	#include <sys/poll.h>
	#include "cv.h"
	#include "trace.h"
	#include <sys/syscall.h>
#ifndef NETOID_LINUX
	#include <sys/lwp.h>
	#include <sys/syscall.h>
	#include <sys/corectl.h>
	#include <sys/priocntl.h>
	#include <sys/rtpriocntl.h>
#endif //NETOID_LINUX
	#include <netinet/tcp.h>

#endif

#define MODE_TRANSMIT			0x1
#define MODE_RECEIVE			0x2
#define MODE_MGCP			0x4
#define MODE_ITERATIVE			0x8
#define MODE_REGISTER			0x10
#define MODE_IXIA			0x20

#define BLIND_XFER			0
#define ATT_XFER			1


// Definitions for counter structures
// used to keep track of state changes
// and call states for all calls

typedef struct _gen_counters
{
	int32_t	EvNewCall_counter;
	int32_t	EvRegEvent_counter;
} gen_counters_t;

typedef struct _call_counters
{
	int32_t	EvCallStateChanged_counter;
	int32_t	EvCallNewRate_counter;
	int32_t	EvCallInfo_counter;
	int32_t CallNonStandardParam_counter;
	int32_t	EvCallFacility_counter;
	int32_t EvCallFastStartSetup_counter;
} call_counters_t;

typedef struct _call_state_counters
{
	int32_t	StateDialtone;
	int32_t	StateProceeding;
	int32_t	StateRingBack;
	int32_t StateConnected;
	int32_t	StateDisconnected;
	int32_t StateIdle;
	int32_t StateOffering;
	int32_t StateTransfering;
	int32_t StateAdmissionConfirm;
	int32_t StateAdmissionReject;
	int32_t StateIncompleteAddress;
	int32_t StateWaitAddressAck;
	int32_t StateFastStartReceived;
	int32_t	Unknown;
} call_state_counters_t;

typedef struct _call_state_mode_counters
{
	int32_t	DisconnectedBusy;
	int32_t	DisconnectedNormal;
	int32_t	DisconnectedReject;
	int32_t	DisconnectedUnreachable;
	int32_t	DisconnectedUnknown;
	int32_t	DisconnectedLocal;
	int32_t	ConnectedControl;
	int32_t	ConnectedCallSetup;
	int32_t	ConnectedCall;
	int32_t	ConnectedConference;
	int32_t	OfferingCreate;
	int32_t	OfferingInvite;
	int32_t	OfferingJoin;
	int32_t	OfferingCapabilityNegotiation;
	int32_t	OfferingCallIndependentSupplementaryService;
	int32_t	DisconnectedIncompleteAddress;
	int32_t	Unknown;
} call_state_mode_counters_t;

typedef struct _control_counters
{
	int32_t	EvCallCapabilities_counter;
	int32_t	EvCallCapabilitiesExt_counter;
	int32_t	EvCallNewChannel_counter;
	int32_t	EvCallCapabilitiesResponse_counter;
	int32_t	EvCallMasterSlaveStatus_counter;
	int32_t EvCallRoundTripDelay_counter;
	int32_t	EvCallUserInput_counter;
	int32_t	EvCallRequestMode_counter;
	int32_t	EvCallMiscStatus_counter;
	int32_t	EvCallControlStateChanged_counter;
	int32_t	EvCallMasterSlave_counter;
} control_counters_t;

typedef struct _control_state_counters
{
	int32_t	StateConnected;
	int32_t	StateConference;
	int32_t	StateTransportConnected;
	int32_t	StateTransportDisconnected;
	int32_t	StateFastStart;
	int32_t	Unknown;
} control_state_counters_t;

typedef struct _channel_counters
{
	int32_t	EvChannelStateChanged_counter;
	int32_t	EvChannelNewRate_counter;
	int32_t	EvChannelMaxSkew_counter;
	int32_t	EvChannelSetAddress_counter;
	int32_t	EvChannelSetRTCPAddress_counter;
	int32_t	EvChannelParameters_counter;
	int32_t	EvChannelRTPDynamicPayloadType_counter;
	int32_t	EvChannelVideoFastUpdatePicture_counter;
	int32_t	EvChannelVideoFastUpdateGOB_counter;
	int32_t	EvChannelVideoFastUpdateMB_counter;
	int32_t	EvChannelHandle_counter;
	int32_t	EvChannelGetRTCPAddress_counter;
	int32_t	EvChannelRequestCloseStatus_counter;
	int32_t	EvChannelTSTO_counter;
	int32_t	EvChannelMediaLoopStatus_counter;
	int32_t	EvChannelReplace_counter;
	int32_t	EvChannelFlowControlToZero_counter;
} channel_counters_t;

typedef struct _channel_state_counters
{
	int32_t	StateDialtone;
	int32_t	StateRingBack;
	int32_t	StateConnected;
	int32_t	StateDisconnected;
	int32_t	StateIdle;
	int32_t	StateOffering;
	int32_t Unknown;
} channel_state_counters_t;

typedef struct _channel_state_mode_counters
{
	int32_t	On;
	int32_t	Off;
	int32_t	DisconnectedLocal;
	int32_t	DisconnectedRemote;
	int32_t	DisconnectedMasterSlaveConflict;
	int32_t	Duplex;
	int32_t	DisconnectedReasonUnknown;
	int32_t	DisconnectedReasonReopen;
	int32_t	DisconnectedReasonReservationFailure;
	int32_t Unknown;
} channel_state_mode_counters_t;

typedef struct _protocol_counters
{
	int32_t HookListen_counter;
	int32_t HookListening_counter;
	int32_t	HookConnecting_counter;
	int32_t	HookInConn_counter;
	int32_t	HookOutConn_counter;
	int32_t	HookSend_counter;
	int32_t	HookRecv_counter;
	int32_t	HookSendTo_counter;
	int32_t	HookRecvFrom_counter;
	int32_t	HookClose_counter;
} protocol_counters_t;

typedef struct _callback_counters
{
	gen_counters_t					gen;
	call_counters_t					call;
	call_state_counters_t			call_state;
	call_state_mode_counters_t		call_state_mode;
	control_counters_t				control;
	control_state_counters_t		control_state;
	channel_counters_t				channel;
	channel_state_counters_t		channel_state;
	channel_state_mode_counters_t	channel_state_mode;
	protocol_counters_t	protocol;
} callback_counters_t;

typedef enum
{
	Scene_eDiscAfterARQ = 0,
	Scene_eDiscAfterSetup,
	Scene_eDiscAfterProceeding,
	Scene_eDiscAfterAlerting,
	Scene_eNoMSD,
	Scene_eNoMSDAck,
	Scene_eNoTCS,
	Scene_eNoTCSAck,
	Scene_eBadH245Address,
	Scene_eNoAutoAnswer,
	
	Scene_eNone,
} Scene_eError;

char * ErrorScenarios(Scene_eError no);

/* Added for GK mode of operation */
typedef struct EpInfoStruct {
    int     id;
    char    callSignalIp[20];
    char    rasIp[20];
    int     rasPort;
    int     callSignalPort;
    char    aliasType[15];
    char    alias[20];
    int     priority;
} EpInfoStruct;

typedef struct
{
    int     isACF; // 1-ACF,0-ARJ 
    int     sessionId;
    char    serviceDesc[1024];
} SerCtrlInfoStruct;

enum GTD_TYPE
{
    ACF=0,
    SETUP,
    CONNECT,
    FACILITY,
    PROCEEDING,
    ALERTING
};

typedef struct 
{
   char gtd[2048];
   int  gtd_len;
} GtdCollectionStruct;

#endif  // _CGEN_H_
