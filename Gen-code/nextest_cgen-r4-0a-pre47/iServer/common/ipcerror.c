#include "ipcerror.h"

char XErrorSStrings[XERRS_MAX][ERROR_MSG_LEN] = 
{ 
	"Not An Error", 
	"Register Serv", 
	"Register All", 
};

char NextoneReasonNames[nextoneReasonMax][ERROR_MSG_LEN] = 
{
	"NoError",
	"ResourceUnavailable",
	"InvalidEndpointID",
	"InvalidAlias",
	"MismatchedAlias",
	"NoAlias",		
	"FirewallMismatch",
	"Forwarded",
	"Rollover",
	"Proxied",
	"DND",
	"VpnGroupMismatch",
	"ZoneMismatch",
	"MaxRecursion",
	"NoEntry",
	"InvalidVpn",
	"GatewayInactive",
	"GatewayInCall",
	"HasReject",
	"NoRoute",
	"BetterMatch",
	"Registered",
	"Forwarded-CP",
	"Rollover-CP",
	"TimeMismatch",
	"PreferCallingPlan",
	"PreferPriority",
	"PreferLRU",
	"PreferVpn",
	"PreferZone",
	"InferiorCallingPlan",
	"InferiorPriority",
	"InferiorLRU",
	"InferiorVpn",
	"InferiorZone",
	"HigherUtilz",
	"LowerUtilz",
	"RouteLenMismatch",
	"RouteTypeMismatch",
	"NoPorts",
	"NoRouteBinding",
	"NoGateway",
	"RejectRoute",
	"PreferSticky",
	"StickyExists",
};
