#include "gis.h"
#include "uh323.h"
#include  "arq.h" 
#include "callsm.h"
#include "log.h"
#include "packets.h"
#include "iwfsm.h"
#include <malloc.h>
#include <ctype.h>
#include "callutils.h"
#include "ipcutils.h"

static int eventBlockAllocs = 0,eventBlockFrees = 0;
char SCC_eEventNames[][50]= {
        "SCC_eNetworkSetup", 
        "SCC_eNetworkAlerting", 
        "SCC_eNetworkProceeding",
        "SCC_eNetworkConnect", 
		"SCC_eNetworkTransportConnected",
		"SCC_eNetworkCapabilities",
		"SCC_eNetworkTCSAck",
		"SCC_eNetworkControlConnected",
		"SCC_eNetworkOLC",
		"SCC_eNetworkChanConnect",
		"SCC_eNetworkCLC",
        "SCC_eNetworkReleaseComp",
        "SCC_eNetworkRequestMode",
        "SCC_eNetworkGeneralMsg",
        "SCC_eNetworkProgress",
        "SCC_eBridgeSetup",
        "SCC_eBridgeAlerting",
        "SCC_eBridgeProceeding",
        "SCC_eBridgeConnect",
		"SCC_eBridgeTransportConnected",
		"SCC_eBridgeCapabilities",
		"SCC_eBridgeTCSAck",
		"SCC_eBridgeControlConnected",
		"SCC_eBridgeOLC",
		"SCC_eBridgeChanConnect",
		"SCC_eBridgeCLC",
        "SCC_eBridgeReleaseComp",
        "SCC_eBridgeRequestMode",
        "SCC_eBridgeGeneralMsg",
        "SCC_eBridgeProgress",
};

char SCC_eEventTypeNames[][30] = {
	"SCC_eEventTypeNone",
 	"SCC_Leg1Network",
        "SCC_Leg1Bridge",
        "SCC_BridgeLeg1",
        "SCC_BridgeLeg2",
        "SCC_Leg2Network",
        "SCC_Leg2Bridge"
};

char SCC_CallStateNames[][30] = {
	"SCC_sError",
	"SCC_sIdle",
	"SCC_sWaitOnRemote",
	"SCC_sConnected",
};

char * SCC_CallStateToStr(int state,char str[80])
{
	if(state <SCC_sMaxStates && state >= SCC_sMinState)
	 	sprintf(str," %s",SCC_CallStateNames[state]);
	else
	  	sprintf(str,"%d(Invalid State)",state);
	return str;
}


char * SCC_BridgeEventToStr(int event,char str[80])
{
	SCC_EventToStr(event+SCC_eBridgeEventsMin,str);
	return str;
}

char * SCC_EventToStr(int event,char str[80])
{
	if(event < SCC_eEventNone || event > SCC_eMax)
	{
		sprintf(str,"Unknown Event(%d)",event);
	} else 
	{
		sprintf(str,"%s",SCC_eEventNames[event]);
	}
	return str;
}


char * sccEventBlockToStr(SCC_EventBlock *e, char *str)
{
	char tmpstr[80];
	char *p = str;
	char callIDStr[32];

	if(!e)
	{
		sprintf(p,"Null Event Handle\n");
		return str;
	}
	
	sprintf(p,"event = %s\t",SCC_EventToStr(e->event, tmpstr));
	p = str + strlen(str);

	sprintf(p,"CallID = %s\t",CallID2String(e->callID,callIDStr));
	p = str + strlen(str);

	sprintf(p,"ConfID = %s\n",CallID2String(e->confID,callIDStr));
	p = str + strlen(str);

	return str;
}

char *
GetH323Event(int event)
{
	if(event < SCC_eEventNone || event > SCC_eMax)
	{
		return "Unknown";
	} else 
	{
		return SCC_eEventNames[event];
	}
}

char *
GetSipBridgeEvent(int event)
{
	switch (event)
	{
	case Sip_eBridgeInvite:
		return "BRIDGE Invite";
	case Sip_eBridge1xx:
		return "BRIDGE 1xx";
	case Sip_eBridge200:
		return "BRIDGE 200";
	case Sip_eBridgeAck:
		return "BRIDGE Ack";
	case Sip_eBridge3xx:
		return "BRIDGE 3xx";
	case Sip_eBridgeFinalResponse:
		return "BRIDGE Final Response";
	case Sip_eBridgeBye:
		return "BRIDGE Bye";
	case Sip_eBridgeCancel:
		return "BRIDGE Cancel";
	case Sip_eBridgeInfo:
		return "BRIDGE Info";
	case Sip_eBridgeError:
		return "BRIDGE Error";
	case Sip_eBridge491Expired:
		return "BRIDGE 491 Expired";
	case Sip_eBridgeCExpired:
		return "BRIDGE C Expired";
	case Sip_eBridgeSessionExpired:
		return "BRIDGE Session Expired";
	case Sip_eBridgeNoResponseError:
		return "BRIDGE No Response Error";
	case Sip_eBridgeInfoFinalResponse:
		return "BRIDGE Info Final Response";
	case Sip_eBridgeRefer:
	        return "BRIDGE Refer";
	case Sip_eBridge202:
		return "BRIDGE 202";
	case Sip_eBridgeNotify:
		return "BRIDGE Notify";
	case Sip_eBridgeNotifyResponse:
		return "BRIDGE NotifyResponse";
	default:
		return "UNKNOWN";
	}

	return "UNKNOWN";
}

char *
GetSipNetworkEvent(int event)
{
	switch (event)
	{
	case Sip_eNetworkInvite:
		return "NETWORK Invite";
	case Sip_eNetwork1xx:
		return "NETWORK 1xx";
	case Sip_eNetwork200:
		return "NETWORK 200";
	case Sip_eNetworkAck:
		return "NETWORK Ack";
	case Sip_eNetwork3xx:
		return "NETWORK 3xx";
	case Sip_eNetworkFinalResponse:
		return "NETWORK Final Response";
	case Sip_eNetworkBye:
		return "NETWORK Bye";
	case Sip_eNetworkCancel:
		return "NETWORK Cancel";
	case Sip_eNetworkError:
		return "NETWORK Error";
	case Sip_eNetworkInfo:
		return "NETWORK Info";
	case Sip_eNetworkInfoFinalResponse:
		return "NETWORK Info Final Response";
	case Sip_eNetworkNoResponseError:
		return "NETWORK No Response Error";
	case Sip_eNetworkRefer:
		return "NETWORK Refer";
	case Sip_eNetwork202:
	        return "NETWORK 202";
	case Sip_eNetworkNotify:
		return "NETWORK Notify";
	case Sip_eNetworkNotifyResponse:
	        return "NETWORK Notify Response";
	default:
		return "UNKNOWN";
	}

	return "UNKNOWN";
}

char *
GetSipState(int state)
{
	switch (state)
	{
	case Sip_sIdle:
		return "IDLE";
	case Sip_sWORRing:
		return "WOR-Ringing";
	case Sip_sRingWOR:
		return "Ringing-WOR";
	case Sip_sConnecting:
		return "Connecting-WaitingForAck";
	case Sip_sConnectedOK:
		return "Connected-ReceivedOK";
	case Sip_sConnectedAck:
		return "Connected-SentACK";
	case Sip_sConnectedWOR:
		return "Connected-WOR";
	default:
		return "UNKNOWN";
	}

	return "UNKNOWN";
}

char *
GetSipEvent(int type, int event)
{
	switch (type)
	{
	case Sip_eNetworkEvent:
		return GetSipNetworkEvent(event);
	case Sip_eBridgeEvent:
		return GetSipBridgeEvent(event);
	default:
		return "UNKNOWN";
	}
}

char *
GetUh323ControlState(int state)
{
	static char stateList[UH323_sControlMaxStates][40] = {
			"UH323_sControlError",
			"UH323_sControlIdle",
			"UH323_sControlTransportConnected",
			"UH323_sControlCapSent",
			"UH323_sControlConnected",
			"UH323_sControlTransportDisconnected",
			"UH323_sControlHeldByLocal",
			"UH323_sControlHeldByRemote",
			"UH323_sControlHeldByBoth",
		};			
	if((state < UH323_sControlMinState) ||(state >=UH323_sControlMaxStates))
	{
		return "Invalid State";
	}
	return (stateList[state]);
}

int
IpcEnd(void)
{
	int qid = -1;

	q_vget(ftok(ISERVER_FTOK_PATH, ISERVER_FTOK_PROJ_BRIDGEH323Q),
			0, 10, 10, &qid);

	if (qid < 0)
	{
		return 0;
	}

	if (q_vdelete(qid) < 0)
	{
		NETERROR(MINIT, ("Could not delete bridge queue\n"));
	}

	return 0;
}

char *
MemDup(char *in, int len, void *(*malloc)())
{
	char *copy;

	copy = (char *)malloc(len);
	if (copy) memcpy(copy, in, len);
	return copy;
}

int
IsValidH323Phone(char *phone, int len)
{
	int i;

	// check for 0123456789No.*,#

	for (i=0; i<len; i++)
	{
		if (isdigit(phone[i])||
			(phone[i]=='*')||
			(phone[i]=='#')||
			(phone[i]==','))
		{
			return 1;
		}
	}

	return 0;
}

char *
CallGetErrorStr(int callerrno)
{
	char *errorstr = "";

	switch (callerrno)
	{
	case SCC_errorAbandoned:
		errorstr = "abandoned";
		break;
	case SCC_errorBlockedUser:
		errorstr = "user-blocked";
		break;
	case SCC_errorDestBlockedUser:
		errorstr = "user-blocked-at-dest";
		break;
	case SCC_errorNoRoute:
		errorstr = "no-route";
		break;
	case SCC_errorDestNoRoute:
		errorstr = "no-route-at-dest";
		break;
	case SCC_errorBusy:
		errorstr = "busy";
		break;
	case SCC_errorResourceUnavailable:
		errorstr = "resource-unavailable";
		break;
	case SCC_errorInvalidPhone:
		errorstr = "invalid-phone";
		break;
	case SCC_errorNetwork:
		errorstr = "network-error";
		break;
	case SCC_errorNoPorts:
		errorstr = "no-ports";
		break;
	case SCC_errorGeneral:
		errorstr = "general-error";
		break;
	case SCC_errorDestinationUnreachable:
		errorstr = "dest-unreach";
		break;
	case SCC_errorUndefinedReason:
		errorstr = "undefined";
		break;
	case SCC_errorInadequateBandwidth:
		errorstr = "no-bandwidth";
		break;
	case SCC_errorH245Incomplete:
		errorstr = "h245-error";
		break;
	case SCC_errorIncompleteAddress:
		errorstr = "incomp-addr";
		break;
	case SCC_errorLocalDisconnect:
		errorstr = "local-disconnect";
		break;
	case SCC_errorH323Internal:
		errorstr = "h323-internal";
		break;
	case SCC_errorH323Protocol:
		errorstr = "h323-proto";
		break;
	case SCC_errorNoCallHandle:
		errorstr = "no-call-handle";
		break;
	case SCC_errorGatewayResourceUnavailable:
		errorstr = "gw-resource-unavailable";
		break;
	case SCC_errorFCECallSetup:
		errorstr = "fce-error-setup";
		break;
	case SCC_errorFCE:
		errorstr = "fce-error";
		break;
	case SCC_errorNoFCEVports:
		errorstr = "fce-no-vports";
		break;
	case SCC_errorNoVports:
		errorstr = "no-vports";
		break;
	case SCC_errorH323MaxCalls:
		errorstr = "h323-maxcalls";
		break;
	case SCC_errorMswInvalidEpId:
		errorstr = "msw-invalid-epid";
		break;
	case SCC_errorMswRouteCallToGk:
		errorstr = "msw-routecallgk";
		break;
	case SCC_errorMswCallerNotRegistered:
		errorstr = "msw-notreg";
		break;
	case SCC_errorHairPin:
		errorstr = "hairpin";
		break;
	case SCC_errorShutdown:
		errorstr = "shutdown";
		break;
	case SCC_errorSwitchover:
		errorstr = "switchover";
		break;
	case SCC_errorDisconnectUnreachable:
		errorstr = "disconnect-unreach";
		break;
	case SCC_errorDestRelComp:
		errorstr = "dest-relcomp";
		break;
	case SCC_errorTemporarilyUnavailable:
		errorstr = "temporarily-unavailable";
		break;
	case SCC_errorDestGone:
		errorstr = "dest-gone";
		break;
	case SCC_errorDestTimeout:
		errorstr = "dest-timeout";
		break;
	case SCC_errorRejectRoute:
		errorstr = "reject-route";
		break;
	case SCC_errorSipAuthRequired:
		errorstr = "401 authorization required";
		break;
	case SCC_errorSipForbidden:
		errorstr = "403 forbidden";
		break;
	case SCC_errorSipProxyAuthRequired:
		errorstr = "407 proxy authorization required";
		break;
	case SCC_errorSipInternalError:
		errorstr = "500 internal error";
		break;
	case SCC_errorSipNotImplemented:
		errorstr = "501 not implemented";
		break;
	case SCC_errorSipServiceUnavailable:
		errorstr = "503 service unavailable";
		break;
	case SCC_errorSipRedirect:
		errorstr = "sip-redirect";
		break;
	case SCC_errorMaxCallDuration:
		errorstr = "max-call-duration";
		break;
	case SCC_errorAdmin:
		errorstr = "admin";
		break;
	case SCC_errorSCM:
		errorstr = "scm";
		break;
	case SCC_errorNoNATTLicense:
		errorstr = "no-nat-t-license";
		break;
	case SCC_errorNone:
		break;
	default:
		break;
	}

	return errorstr;
}

char IWF_CallStateNames[][IWF_STATE_STR] = {
    "IWF_sError",
    "IWF_sIdle",
    "IWF_sWaitOnH323",
    "IWF_sWaitOnSip",
    "IWF_sCallConnected",
    "IWF_sConnected",
    "IWF_sHeldBySip",
    "IWF_sHeldByH323",
    "IWF_sHeldByBoth",
    "IWF_sReInvite",
    "IWF_sNonFS200OK",
    "IWF_sReqMode",
	"IWF_sWaitOnSipNewOffer",
	"IWF_sNullTCS",
	"IWF_sAnnexF"
};

char * iwfState2Str(int state,char str[IWF_STATE_STR])
{
    if(state <IWF_sMaxStates && state >= IWF_sMinState)
        sprintf(str," %s",IWF_CallStateNames[state]);
    else
        sprintf(str,"%d(Invalid State)",state);
    return str;
}

int
GkCallSetRemoteErrorCodes(CallHandle *callHandle, CallDetails *callDetails)
{
	callHandle->callError = callDetails->callError;
	callHandle->cause = callDetails->cause;
	callHandle->h225Reason = callDetails->h225Reason;
	callHandle->rasReason = callDetails->rasReason;
	callHandle->responseCode = callDetails->responseCode;

	return 0;
}

int
GkCallResetRemoteErrorCodes(CallHandle *callHandle)
{
	callHandle->callError = 0;
	callHandle->cause = 0;
	callHandle->h225Reason = 0;
	callHandle->rasReason = 0;
	callHandle->responseCode = 0;

	return 0;
}

CallHandle *
CallDelete(cache_t cache, char *callid)
{
	char fn[] = "CallDelete():";
	CallHandle *callHandle;

	callHandle = CacheDelete(callCache, callid);
	
	if (!callHandle)
	{
		return 0;
	}

	if ( CallFceTranslatedIpPort(callHandle) )
	{
		if (CacheDelete(tipCache, &CallFceNatDstIp(callHandle)) != callHandle)
		{
			NETERROR(MCACHE, ("%s entry deleted from tipCache does not match\n",
				fn));
		}
	}

	return callHandle;
}

char *
CallGetEvtStr(int evt)
{
	char *evtstr = "";

	switch (evt)
	{
	case SCC_evtNone:
		evtstr = "no-state";	
		break;
	case SCC_evtInvalid:
		evtstr = "invalid";
		break;
	case SCC_evtUnknown:
		evtstr = "unknown";
		break;
	case SCC_evtHunt:
		evtstr = "hunt";
		break;
	case SCC_evtResolve:
		evtstr = "resolve";
		break;
	case SCC_evt323SetupTx:
		evtstr = "setup-tx";
		break;
	case SCC_evt323SetupRx:
		evtstr = "setup-rx";
		break;
	case SCC_evt323ProcTx:
		evtstr = "proc-tx";
		break;
	case SCC_evt323ProcRx:
		evtstr = "proc-rx";
		break;
	case SCC_evt323AlertTx:
		evtstr = "alert-tx";
		break;
	case SCC_evt323AlertRx:
		evtstr = "alert-rx";
		break;
	case SCC_evt323ConnTx:
		evtstr = "conn-tx";
		break;
	case SCC_evt323ConnRx:
		evtstr = "conn-rx";
		break;
	case SCC_evt323ARQRx:
		evtstr = "arq-rx";
		break;
	case SCC_evt323ACFTx:
		evtstr = "acf-tx";
		break;
	case SCC_evt323ARQTx:
		evtstr = "arq-tx";
		break;
	case SCC_evt323ACFRx:
		evtstr = "acf-rx";
		break;
	case SCC_evt323ARJRx:
		evtstr = "arj-rx";
		break;
	case SCC_evt323ARQTmout:
		evtstr = "arq-tmout";
		break;
	case SCC_evt323ARJTx:
		evtstr = "arj-tx";
		break;
	case SCC_evt323DestARQRx:
		evtstr = "destarq-rx";
		break;
	case SCC_evt323DestACFTx:
		evtstr = "destacf-tx";
		break;
	case SCC_evt323NewCall:
		evtstr = "newcall";
		break;
	case SCC_evtSipInviteTx:
		evtstr = "inv-tx";
		break;
	case SCC_evtSipInviteRx:
		evtstr = "inv-rx";
		break;
	case SCC_evtSipReInviteTx:
		evtstr = "reinv-tx";
		break;
	case SCC_evtSipReInviteRx:
		evtstr = "reinv-rx";
		break;
	case SCC_evtSip100xTx:
		evtstr = "100x-tx";
		break;
	case SCC_evtSip100xRx:
		evtstr = "100x-rx";
		break;
	case SCC_evtSip200Tx:
		evtstr = "200-tx";
		break;
	case SCC_evtSip200Rx:
		evtstr = "200-rx";
		break;
	case SCC_evtSipAckTx:
		evtstr = "ack-tx";
		break;
	case SCC_evtSipAckRx:
		evtstr = "ack-rx";
		break;
	case SCC_evtSip3xxRx:
		evtstr = "3xx-rx";
		break;
	case SCC_evtSipInviteRedirected:
		evtstr = "inv-red";
		break;
	case SCC_evt323LRQTx:
		evtstr = "lrq-tx";
		break;
	case SCC_evt323LRQRx:
		evtstr = "lrq-rx";
		break;
	case SCC_evt323LCFRx:
		evtstr = "lcf-rx";
		break;
	case SCC_evt323LRJRx:
		evtstr = "lrj-rx";
		break;
	case SCC_evt323LRJTx:
		evtstr = "lrj-tx";
		break;
	case SCC_evt323LRQTmout:
		evtstr = "lrq-tmout";
		break;
	case SCC_evt323ProgTx:
		evtstr = "prog-tx";
		break;
	case SCC_evt323ProgRx:
		evtstr = "prog-rx";
		break;
	case SCC_evt323DRQForceDropRx:
		evtstr = "drq-forcedrop-rx";
		break;
	case SCC_evtSipAuthReqRx:
		evtstr = "authreq-rx";
		break;
	case SCC_evtSipAuthReqTmout:
		evtstr = "authreq-tmout";
		break;
	default:
		break;
	}

	return evtstr;
}

int
ConfGetLegs(ConfHandle *confHandle, CallHandle **callHandle1, CallHandle **callHandle2)
{
	CallHandle *callHandle;
	int i;

	*callHandle1 = NULL;
	*callHandle2 = NULL;

	for(i = 0; i<confHandle->ncalls;++i)
	{
		if (confHandle->ncalls > 0)
		{
			callHandle = CacheGet(callCache, confHandle->callID[i]);

			if (callHandle && (callHandle->leg == SCC_CallLeg1))
			{
				*callHandle1 = callHandle;
			}

			if (callHandle && (callHandle->leg == SCC_CallLeg2))
			{
				*callHandle2 = callHandle;
			}
		}
	}

	return 0;
}

char *
GetSipRegState(int state)
{
	switch (state)
	{
	case SipReg_sIdle:
		return "IDLE";
	case SipReg_sNetworkWOR:
		return "WOR";
	case SipReg_sConfirmed:
		return "Confirmed";
	default:
		return "UNKNOWN";
	}

	return "UNKNOWN";
}

char *
GetSipRegEvent(int type, int event)
{
	switch (event)
	{
	case SipReg_eBridgeRegister:
		return "bridge register";
	case SipReg_eNetworkRegister200:
		return "register 200";
	case SipReg_eNetworkRegister3xx:
		return "register 3xx";
	case SipReg_eNetworkRegisterFinalResponse:
		return "register final resp";
	case SipReg_eBridgeRegisterTimer:
		return "register timer";
	default:
		return "UNKNOWN";
	}
}

char *
h323InstanceName(int instance)
{
	if (nh323Instances > 1)
	{
		if (instance == ARQ_MINSTANCE_ID)
			return "ARQ Instance";
		if (instance == SGK_MINSTANCE_ID)
			return "SGK Instance";
		if (instance == TMP_MINSTANCE_ID)
			return "TMP Instance";

		if (instance %2)
		{
			return "Egress Instance";
		}
		else
		{
			return "Ingress Instance";
		}
	}

	return "Single Instance";
}

char *
EventToStr(int event, int protocol, char *str)
{
	if (protocol == CAP_SIP) {
		/* Assume the events are network events */
		sprintf(str, "%s", GetSipNetworkEvent(event));	
	}
	else if (protocol == CAP_H323) {
		if(event < SCC_eEventNone || event > SCC_eMax)
		{
			sprintf(str,"Unknown Event(%d)",event);
		} else {
			sprintf(str,"%s",SCC_eEventNames[event]);
		}
	}
	else {
		sprintf(str,"Event(%d) Unknown Protocol(%d)",event, protocol);
	}

	return str;
}
