#include <syslog.h>
#ifndef SUNOS
#include <stdarg.h> 
#include <strings.h>
#else
//#include <sys/varargs.h>
#endif
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "bits.h"
#include "ipc.h"
#include "serverdb.h"
#include "key.h"
#include "srvrlog.h"
#include "gis.h"
#include "ifs.h"

#ifdef ALLOW_ISERVER_SIP
#include "sipaddr.h"
#endif
#include "callsm.h"
#include "age.h"
#include "log.h"
#include "phonode.h"
#include "nxosd.h"
#include "iwfutils.h"
#include "ipstring.h"
#include "callutils.h"
#include "callconf.h"

typedef struct _iedge_types
{
	int 	type;
	char 	name[CLIENT_ATTR_LEN];
} iedge_types_t;

/* pl order in order of types, just to make
 * int->char lookups faster.
 */
iedge_types_t iedge_types[IEDGE_MAXTYPES] = 
{
	{ IEDGE_500, 		"iedge500" },
	{ IEDGE_IMOBILE, 	"imobile" },
	{ IEDGE_510,		"iedge510" },
	{ IEDGE_1000,		"iedge1000" },
	{ IEDGE_XGW,		"xgateway" },
	{ IEDGE_ISERVER,	"iserver" },
	{ IEDGE_XGK,		"xgatekeeper" },
	{ IEDGE_SGK,		"sgatekeeper" },
	{ IEDGE_USERACC,	"user" },
	{ IEDGE_ENUMS,		"enum" },
	{ IEDGE_IPPHONE,	"ipphone" },
	{ IEDGE_SIPPROXY,	"sipproxy" },
	{ IEDGE_SOFTSW,		"softswitch" },
	{ IEDGE_SIPGW,		"sipgw" },
	{ IEDGE_1000G,		"iedge1000g" },
	{ IEDGE_SUBNET,		"subnet" },
};

char gkRegState[GKREG_MAX][GKREG_LEN] = 
{
	"unknown",
	"registered",
	"registering",
	"discovered",
	"discovering",
	"idle",
	"trying alt",
};

/**********************************************************************
 * This method of logging is not used. See netlog.c
 **********************************************************************/

/* Log command:
 * In general, to debug use level 4, and severity set
 * to whatever is appropriate, more important than
 * LOG_NOTICE (ie., EMERG, ALERT, CRIT, ERR, WARN and NOTICE
 * When DEBUG and INFO are used, syslog wont be used and 
 * stderr will be used. If debug is 0, only syslog is used,
 * for stuff other than DEBUG and INFO
 */

extern int debug;

static FILE * 	__logStream;
static int	__log_inited;

void
logInit (FILE * stream)
{
	__logStream = stream;

	__log_inited++;
}


void
log(int severity, int syserr, char *format, ...)
{
#ifdef needed
    va_list ap;
    static char fmt[211] = "warning - ";
    char *msg;
    char tbuf[20];
    struct timeval now;
    struct tm *thyme;
    FILE * stream;

    if (! __log_inited)
    {
	__logStream = stderr;
	__log_inited++;
    }

    va_start(ap, format);
    vsprintf(&fmt[10], format, ap);
    va_end(ap);
    msg = (severity == LOG_WARNING) ? fmt : &fmt[10];

    switch (debug) {
	case 0: break;
	case 1: if (severity > LOG_NOTICE) break;
	case 2: if (severity > LOG_INFO  ) break;
	default:
	    gettimeofday(&now,NULL);
	    thyme = localtime((long *)&now.tv_sec);
	    strftime(tbuf, sizeof(tbuf), "%X.%%03d ", thyme);
	    fprintf(__logStream, tbuf, now.tv_usec / 1000);
	    fprintf(__logStream, "%s", msg);
	    if (syserr == 0)
		fprintf(__logStream, "\n");
	    else if (syserr < sys_nerr)
		fprintf(__logStream, ": %s\n", sys_errlist[syserr]);
	    else
		fprintf(__logStream, ": errno %d\n", syserr);
    }
    if (severity <= LOG_NOTICE) {
	    if (syserr != 0) {
		errno = syserr;
		syslog(severity, "%s: %m", msg);
	    } else
		syslog(severity, "%s", msg);

	if (severity <= LOG_ERR) exit(-1);
    }
#endif
}

int
format(FILE *stream, int level)
{
     int i;

     for (i=0; i<level; i++)
	  fprintf(stream, "\t");
	return 0;
}

/**********************************************************************/

char*
GetVendorDescription (Vendor vendor)
{
	switch (vendor)
	{
		case Vendor_eClarent:
			return "Clarent";
		case Vendor_eSonusGSX:
			return "SonusGSX";
		case Vendor_eBroadsoft:
			return "Broadsoft";
		case Vendor_eCisco:
			return "Cisco";
		case Vendor_eVocalTec:
			return "VocalTec";
		case Vendor_eLucentTnt:
			return "LucentTNT";
		case Vendor_eExcel:
			return "Excel";
		case Vendor_eAvaya:
			return "Avaya";
		case Vendor_eNortel:
			return "Nortel";
		default:
			return "Generic";
	}
}

int
GetVendor (char *v)
{
	if (!strcasecmp(v, "clarent"))
		return Vendor_eClarent;
	else if (!strcasecmp(v, "sonusgsx"))
		return Vendor_eSonusGSX;
	else if (!strcasecmp(v, "broadsoft"))
		return Vendor_eBroadsoft;
	else if (!strcasecmp(v, "cisco"))
		return Vendor_eCisco;
	else if (!strcasecmp(v, "vocaltec"))
		return Vendor_eVocalTec;
	else if (!strcasecmp(v, "lucenttnt"))
		return Vendor_eLucentTnt;
	else if (!strcasecmp(v, "excel"))
		return Vendor_eExcel;
	else if (!strcasecmp(v, "avaya"))
		return Vendor_eAvaya;
	else if (!strcasecmp(v, "nortel"))
		return Vendor_eNortel;
	else
		return Vendor_eGeneric;
}

char
GetBcapLayer1(char *str)
{
	if(!strcasecmp(str,"default"))
		return BCAPLAYER1_Default;
	if(!strcasecmp(str,"pass"))
		return BCAPLAYER1_Pass;
	if(!strcasecmp(str,"g711ulaw"))
		return BCAPLAYER1_G711ulaw;
	if(!strcasecmp(str,"g711alaw"))
		return BCAPLAYER1_G711alaw;
	if(!strcasecmp(str,"h221"))
		return BCAPLAYER1_H221;
	return -1;
}

char*
GetBcapLayer1Str(char proto)
{
	switch (proto)
	{
		case BCAPLAYER1_Default:
			return "default";
		case BCAPLAYER1_Pass:
			return "pass";
		case BCAPLAYER1_G711ulaw:
			return "g711ulaw";
		case BCAPLAYER1_G711alaw:
			return "g711alaw";
		case BCAPLAYER1_H221:
			return "h221";
		default:
			return "invalid";
	}
}


char*
GetQ931CdpnStr(char cdpn)
{
    switch(cdpn)
    {
        case(Q931CDPN_Default):
            return "default";
        case(Q931CDPN_Unknown):
            return "unknown";
        case(Q931CDPN_Pass):
            return "pass";
        case(Q931CDPN_International):
            return "international";
        case(Q931CDPN_National):
            return "national";
        case(Q931CDPN_Specific):
            return "specific";
        case(Q931CDPN_Subscriber):
            return "subscriber";
        case(Q931CDPN_Abbreviated):
            return "abbreviated";
        default:
            return "invalid";
    }
}

char*
GetQ931CgpnStr(char cgpn)
{
    switch(cgpn)
    {
        case Q931CGPN_Pass:
            return "pass";
        case Q931CGPN_Unknown:
            return "unknown";
        case Q931CGPN_International:
            return "international";
        case Q931CGPN_National:
            return "national";
        case Q931CGPN_Specific:
            return "specific";
        case Q931CGPN_Subscriber:
            return "subscriber";
        case Q931CGPN_Abbreviated:
            return "abbreviated";
        default:
            return "invalid";
    }
}


int
PrintInfoEntry(FILE *stream, InfoEntry *infoEntry)
{
    time_t	tmptime;
	char	ctimebuf[256];

	PrintDbInfoEntry(stream, infoEntry);

	if (infoEntry->stateFlags & CL_ACTIVE)
	{
     	tmptime = infoEntry->rTime + cacheTimeout;

		ctime_r( &tmptime, ctimebuf );

		fprintf(stream, "\tAge Interval \tfrom %s", ctimebuf );

     	tmptime = (tmptime + cacheTimeout);

		ctime_r( &tmptime, ctimebuf );

		fprintf(stream, "\t\t\tto %s", ctimebuf );
	}

	fprintf(stream, "\tState\t\t");
	if (infoEntry->stateFlags & CL_ACTIVE)
	{
		fprintf(stream, "Active ");

		if (!(infoEntry->stateFlags & CL_REGISTERED))
		{
			fprintf(stream, "\(Awaiting Re-Registration) ");
		}
    }

    if (infoEntry->stateFlags & CL_STATIC)
    {
	  	fprintf(stream, "Static");
    }
	else if (infoEntry->stateFlags & CL_DYNAMIC)
    {
	  	fprintf(stream, "Dynamic");
    }

	if (!(infoEntry->stateFlags & CL_ACTIVE) &&
		!(infoEntry->stateFlags & CL_STATIC))
    {
	  	fprintf(stream, "Inactive");
    }

	fprintf(stream, "\n\n");

	return 0;
}

int
PrintCallID(FILE *stream, char *callID)
{
	 int i;
	 
	 for (i=0;i<16;i++)
	 {
		  fprintf(stream, "%2.2x", (callID[i])&0xff);
	 }

	return 0;
}

char *
CallID2String(char *callID, char *str)
{
	int i;
	 
	for (i=0;i<16;i++)
	{
		sprintf(&str[i*2], "%2.2x", (callID[i])&0xff);
	}

	// since we are returning a string, it should be terminated!!
	str[32] = '\0';

	return str;
}

void
String2Guid(char *str, char *callID)
{
	int i;
	char hexstr[3] = { 0 };

	for (i=0; i<16; i++)
	{
		hexstr[0] = str[2*i];
		hexstr[1] = str[2*i+1];
		callID[i] = strtol(hexstr, NULL, 16);
	}
}

char *
ConfID2String(char *confID, char *str)
{
	int i;
	 
	for (i=0;i<16;i++)
	{
		sprintf(&str[i*2], "%2.2x", (confID[i])&0xff);
	}

	return str;
}

char *
ConfID2String2(char *confID, char *str)
{
	if(rad_acct)
	{
		sprintf(str, "%8.8X %8.8X %8.8X %8.8X",
						ntohl(*(int*)(confID + 0)),
						ntohl(*(int*)(confID + 4)),
						ntohl(*(int*)(confID + 8)),
						ntohl(*(int*)(confID + 12)));
	}
	else
	{
		ConfID2String(confID, str);
	}

	return str;
}

int
PrintConfID(FILE *stream, char *confID)
{
	 int i;
	 
	 for (i=0;i<16;i++)
	 {
		  fprintf(stream, "%2.2x", (confID[i])&0xff);
	 }

	return 0;
}

int
PrintCallEntry(FILE *stream, CallHandle *callHandle)
{
	time_t tmptime;
	char 	str[80];
	int	sid = 0;
	char	ctimebuf[256];
	 char 	str1[CALL_ID_STRLEN],str2[CONF_ID_STRLEN];

	// General Information is printed first and then protocol 
	// specific information

	fprintf(stream, "MSW: CID %s CFID %s\n",
		CallID2String(callHandle->callID, str1),
		CallID2String(callHandle->confID, str2));

 	fprintf(stream, "%s %s call leg %d, origin=%s:\n", 
		(callHandle->handleType == SCC_eH323CallHandle)?"H.323":
		(callHandle->handleType == SCC_eSipCallHandle)?"SIP":"UNKNOWN",	
		(callHandle->flags & FL_CALL_FAX)?"Fax":"Voice",
		callHandle->leg, callHandle->callSource?"LOCAL":"REMOTE");

	fprintf(stream, "source %s/%lu/%s\n", callHandle->phonode.regid,
		callHandle->phonode.uport, 
		ULIPtostring(callHandle->phonode.ipaddress.l));

	fprintf(stream, "dest %s/%lu/%s:%d\n", callHandle->rfphonode.regid,
		callHandle->rfphonode.uport, 
		ULIPtostring(callHandle->rfphonode.ipaddress.l), 
		callHandle->rfcallsigport);

	fprintf(stream, "trunk group %s\n", callHandle->tg?callHandle->tg:"none");
	fprintf(stream, "customer %s\n", callHandle->custID?callHandle->custID:"none");

	fprintf(stream,"Signalling PeerIp = %s:%d\n",
		ULIPtostring(callHandle->peerIp),callHandle->peerPort);

	fprintf(stream,"CallingPartyNumber = %s\n",
	 	(H323callingPartyNumber(callHandle) == NULL)?"not specified":
		H323callingPartyNumber(callHandle));
	fprintf(stream, "input ANI %s, new ANI %s\n", 
		callHandle->inputANI, callHandle->phonode.phone);
	fprintf(stream, "input DNIS %s, before dest plan %s, after dest plan %s\n", 
		callHandle->inputNumber, callHandle->dialledNumber,
		callHandle->rfphonode.phone);

	fprintf(stream, "route %s\n", callHandle->crname);
	fprintf(stream, "vendor %s\n", 
		GetVendorDescription(callHandle->vendor));

	fprintf(stream, "Call Details: %s %s\n",
		CallGetEvtStr(callHandle->callDetails2.lastEvent), 
		CallGetErrorStr(callHandle->callDetails2.callError));

	fprintf(stream, "Call Error = %s, hunts = %d, last evt = %s\n", 
		CallGetErrorStr(callHandle->callError), 
		callHandle->nhunts, 
		CallGetErrorStr(callHandle->lastEvent));

	if (tmptime = timedef_sec(&callHandle->callStartTime))
	{
		ctime_r( &tmptime, ctimebuf );
		fprintf(stream, "Start Time %s", ctimebuf);
	}

	if (tmptime = timedef_sec(&callHandle->callRingbackTime))
	{
		ctime_r( &tmptime, ctimebuf );
		fprintf(stream, "Ring Time %s", ctimebuf);
	}

	if (tmptime = timedef_sec(&callHandle->callConnectTime))
	{
		ctime_r( &tmptime, ctimebuf );
		fprintf(stream, "Connect Time %s", ctimebuf);
	}

	if (tmptime = timedef_sec(&callHandle->callEndTime))
	{
		ctime_r( &tmptime, ctimebuf );
		fprintf(stream, "End Time %s", ctimebuf);
	}

	fprintf(stream, "flags = %x, evtList = %p\n", 
		callHandle->flags, callHandle->evtList);

	// Firewall Control Information
	fprintf(stream,"BundleId = %d\nTranslatedIp = %s:%d\n",
		CallFceBundleId(callHandle),
		ULIPtostring(CallFceTranslatedIp(callHandle)),
		CallFceTranslatedPort(callHandle));

	if(callHandle->handleType == SCC_eH323CallHandle)
	{	
		PrintH323CallEntry(stream,callHandle);
 	}
	else if(callHandle->handleType == SCC_eSipCallHandle)
	{
		PrintSipCallEntry(stream,callHandle);
	}

	 return 0;
}

int
PrintH323CallEntry(FILE *stream, CallHandle *callHandle)
{
	 time_t tmptime;
	 char 	str[80],str1[CALL_ID_STRLEN],str2[CONF_ID_STRLEN];
	 int	i;
	 H323RTPSet *pRtp;
	 ChanInfo	*pCh;
	 int	sid = 0;

	fprintf(stream, "H.323 Information:\n");
	 fprintf(stream, "Internal address (ras=%p,%p) (call=%p)\n",
			(void *)H323inhsRas(callHandle), 
			H323outhsRas(callHandle),
			(void *)H323hsCall(callHandle));

	 fprintf(stream, "Internal Call State = %s Control State = %s\n",
		SCC_CallStateToStr(callHandle->state,str),
		GetUh323ControlState(H323controlState(callHandle)));

	 fprintf(stream,"h245Address = %s:%lu\n",
		ULIPtostring(H323h245ip(callHandle)),H323h245port(callHandle));

	 fprintf(stream,"WaitForDRQ = %d\n",H323waitForDRQ(callHandle));

	 pCh = &H323inChan(callHandle)[sid];
	 fprintf(stream,"codec = %2d param = %2d RTP=%s/%d RTCP=%s/%d DataType= %d ",
		pCh->codec,pCh->param,ULIPtostring(pCh->ip),pCh->port,
		ULIPtostring(pCh->rtcpip),pCh->rtcpport,pCh->dataType );
	 fprintf(stream,"inhsChannel = %p\n",
		pCh->hsChan);

	 pCh = &H323outChan(callHandle)[sid];
	 fprintf(stream,"codec = %2d param = %2d RTP=%s/%d RTCP=%s/%d DataType= %d ",
		pCh->codec,pCh->param,ULIPtostring(pCh->ip),pCh->port,
		ULIPtostring(pCh->rtcpip),pCh->rtcpport,pCh->dataType );
	 fprintf(stream,"outhsChannel = %p\n",
		pCh->hsChan);

	 pRtp = H323localSet(callHandle);
	 fprintf(stream,"LocalSet:\n");
	 for(i=0;i<H323nlocalset(callHandle);++i)
	 {
		 fprintf(stream,"codec = %2d param = %2d RTP = %s/%d\n",
			pRtp[i].codecType,pRtp[i].param,ULIPtostring(pRtp[i].rtpaddr),
			pRtp[i].rtpport);
	 }

	 pRtp = H323remoteSet(callHandle);
	 fprintf(stream,"RemoteSet:\n");
	 for(i=0;i<H323nremoteset(callHandle);++i)
	 {
		 fprintf(stream,"codec = %2d param = %2d RTP = %s/%d\n",
			pRtp[i].codecType,pRtp[i].param,ULIPtostring(pRtp[i].rtpaddr),
			pRtp[i].rtpport);
	 }

	 return 0;
}

int
PrintSipCallEntry(FILE *stream, CallHandle *callHandle)
{
	 time_t tmptime;
	 char 	str[80],str1[CALL_ID_STRLEN],str2[CONF_ID_STRLEN];
	 SipCallHandle *sipCallHandle;

	fprintf(stream, "SIP Information:\n");
	 fprintf(stream, "CallState %s\n",
		GetSipState(callHandle->state));

	sipCallHandle = SipCallHandle(callHandle);
	if (callHandle->handleType != SCC_eSipCallHandle)
	{
		fprintf(stream, "malformed call handle\n");
	}
	if (sipCallHandle->callLeg.local)
		fprintf(stream, "local: %s@%s:%d; tag=%s\n", 
			SVal(sipCallHandle->callLeg.local->name),
			SVal(sipCallHandle->callLeg.local->host), 
			sipCallHandle->callLeg.local->port,
			SVal(sipCallHandle->callLeg.local->tag));
	if (sipCallHandle->callLeg.remote)
		fprintf(stream, "remote: %s@%s:%d; tag=%s\n", 
			SVal(sipCallHandle->callLeg.remote->name),
			SVal(sipCallHandle->callLeg.remote->host), 
			sipCallHandle->callLeg.remote->port,
			SVal(sipCallHandle->callLeg.remote->tag));
	fprintf(stream, "callid: %s\n", sipCallHandle->callLeg.callid);
	if (sipCallHandle->requri)
		fprintf(stream, "requri: %s@%s:%d\n", 
			SVal(sipCallHandle->requri->name),
			SVal(sipCallHandle->requri->host), 
			sipCallHandle->requri->port);
	if (sipCallHandle->localContact)
		fprintf(stream, "local contact: %s@%s:%d\n", 
			SVal(sipCallHandle->localContact->name),
			SVal(sipCallHandle->localContact->host), 
			sipCallHandle->localContact->port);
	if (sipCallHandle->remoteContact)
		fprintf(stream, "remote contact: %s@%s:%d\n", 
			SVal(sipCallHandle->remoteContact->name),
			SVal(sipCallHandle->remoteContact->host), 
			sipCallHandle->remoteContact->port);

	fprintf(stream, "session timer %s expires %d, minSE %d, refresher %s\n",
		callHandle->timerSupported?"supported":"not supported",
		callHandle->sessionExpires,
		callHandle->minSE,
		(callHandle->refresher==SESSION_REFRESHER_NONE)?"none":
		(callHandle->refresher==SESSION_REFRESHER_UAC)?"uac":
		(callHandle->refresher==SESSION_REFRESHER_UAS)?"uas":"unknown");

	fprintf(stdout, "successful invites = %d\n", sipCallHandle->successfulInvites);

	fprintf(stream, "\n");

	return 0;
}

void
PrintConfEntry(FILE *stream, ConfHandle *confHandle)
{
	 char str[80];
	 fprintf(stream,"ncalls = %d\t",confHandle->ncalls);
	 fprintf(stream, "state = %s substate = %d\n",
		iwfState2Str(confHandle->state,str),confHandle->subState);
	 PrintConfID(stream, confHandle->confID);

}

int
PrintDbInfoEntry(FILE *stream, InfoEntry *infoEntry)
{
     char s1[25], *pStr;
     time_t tmptime;

     if (BIT_TEST(infoEntry->sflags, ISSET_REGID))
     {
	  fprintf(stream, "\tRegistration ID %s\n", infoEntry->regid);
     }


     if (BIT_TEST(infoEntry->sflags, ISSET_UPORT))
     {
	  fprintf(stream, "\tPort \t\t%lu\n", infoEntry->uport);
     }

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
     {
	  fprintf(stream, "\tNetwork Phone \t%s\n", infoEntry->phone);
     }
     else
     {
	  fprintf(stream, "\tNetwork Phone \tNOTASSIGNED\n");
     }
     
     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_VPNPHONE))
     {
	  fprintf(stream, "\tPhone \t\t%s\n", infoEntry->vpnPhone);
     }
     else
     {
	  fprintf(stream, "\tPhone \t\tNOTASSIGNED\n");
     }
     
     if (strlen(infoEntry->vpnName))
     {
	  fprintf(stream, "\tVpn \t\t%s\n", infoEntry->vpnName);
     }
     else
     {
	  fprintf(stream, "\tVpn \t\tNOTASSIGNED\n");
     }

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_IPADDRESS))
     {
		  fprintf(stream, "\tIpAddr \t\t%s/%d\n", 
				  FormatIpAddress(infoEntry->ipaddress.l, s1),
				  infoEntry->callsigport);
     }

	 if (strcmp(infoEntry->realmName, REALM_UNASSIGNED))
	 {
		if (!strcmp(infoEntry->realmName, REALM_ANY))
		{
			if (infoEntry->realmId != REALM_ID_ANY)
			{
	 			fprintf(stream,"\tRealm\t\t%s/%u\n", infoEntry->realmName, infoEntry->realmId );
			}
			else
			{
	 			fprintf(stream,"\tRealm\t\t%s\n", infoEntry->realmName);
			}
		}
		else
		{
	 		fprintf(stream,"\tRealm\t\t%s/%u\n", infoEntry->realmName, infoEntry->realmId );
		}
	 }
	 else
	 {
	 	fprintf(stream,"\tRealm\t\tUNASSIGNED\n");
	 }

	 if (infoEntry->igrpName[0] != 0)
	 {
	 		fprintf(stream,"\tIedge Group\t%s\n", infoEntry->igrpName);
	 }

	 fprintf(stream,
			 "\tRasIpAddr \t%s/%d\n", 
			 FormatIpAddress(infoEntry->rasip, s1), infoEntry->rasport);

#if 0
     if (infoEntry->stateFlags & CL_PROXY)
     {
		  if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == 
			  (CL_PROXY|CL_PROXYING))
		  {
			   fprintf(stream, "\tProxied (%s %s)\n", 
					   infoEntry->xphone, infoEntry->xvpnPhone);
		  }
		  else
		  {
			   fprintf(stream, "\tProxied (Absent)\n");
		  }
     }

     if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == CL_PROXYING)
     {
		  fprintf(stream, "\tProxying (%s %s)\n", 
				  infoEntry->xphone, infoEntry->xvpnPhone);
     }
#endif

     if (infoEntry->stateFlags & CL_FORWARD)
     {
		  fprintf(stream, "\tForwarding \tOn %s\n",
				  infoEntry->protocol?"Rollover":"");
		  if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE))
		  {
			   fprintf(stream, "\tFwd Phone \t%s\n", infoEntry->nphone);
		  }
		  if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE))
		  {
			   fprintf(stream, "\tFwd vpn phone \t%s/%lu\n", 
					   infoEntry->nvpnPhone, infoEntry->nvpnExtLen);
		  }
     }

     if (infoEntry->stateFlags & CL_DND)
     {
		  fprintf(stream, "\tDND \t\tOn\n");
     }

     fprintf(stream, "\tType\t\t%s\n", 
			 IedgeName(infoEntry->ispcorgw));

     if (infoEntry->subnetip || infoEntry->subnetmask)
     {
		  fprintf(stream, "\tSubnetIP \t%s\n", 
				  FormatIpAddress(infoEntry->subnetip, s1));
		  fprintf(stream, "\tSubnetMask \t%s\n", 
				  FormatIpAddress(infoEntry->subnetmask, s1));
     }

	 fprintf(stream, "\tMax Calls \t%d (%d:%d)\n", 
		IedgeXCalls(infoEntry), 
		IedgeXInCalls(infoEntry), IedgeXOutCalls(infoEntry));

	 fprintf(stream, "\tOngoing Calls \t%d (%d:%d)\n", 
		IedgeCalls(infoEntry),
		IedgeInCalls(infoEntry), IedgeOutCalls(infoEntry));

	 fprintf(stream, "\tMax Src Hunting\t%d\n",
			infoEntry->maxHunts);

	 fprintf(stream, "\tPriority\t%d\n",
			infoEntry->priority);

     if (infoEntry->stateFlags & CL_TAP)
     {
		  fprintf(stream, "\tTapping enabled\n");
     }

     fprintf(stream, "\tCapabilities\t%s %s %s\n",
			 IsGateway(infoEntry)?"gateway":"",
			 IsSGatekeeper(infoEntry)?"sgatekeeper":"",
			BIT_TEST(infoEntry->cap, CAP_ENUMS)?"enum":"");

#if 0
	 fprintf(stream, "\tRouteCalls\t\t%s\n", BIT_TEST(infoEntry->cap, CAP_ROUTEDIRECT)?
		"disabled":"enabled");
#endif

	 fprintf(stream, "\tSIP\t\t%s\n", BIT_TEST(infoEntry->cap, CAP_SIP)?
		"enabled":"disabled");

	 fprintf(stream, "\tH323\t\t%s\n", BIT_TEST(infoEntry->cap, CAP_H323)?
		"enabled":"disabled");

#if 0
	 fprintf(stream, "\tEnum\t\t%s\n", BIT_TEST(infoEntry->cap, CAP_ENUMS)?
		"enabled":"disabled");
#endif

     if (strlen(infoEntry->zone))
     {
		  fprintf(stream, "\tZone \t\t%s\n", infoEntry->zone);
     }

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_EMAIL))
     {
		  fprintf(stream, "\tEmail \t\t%s\n", infoEntry->email);
     }

	if (strlen(infoEntry->h323id))
	{ 
		fprintf(stream, "\tH323 ID \t%s\n", infoEntry->h323id);
	}

	if (strlen(infoEntry->pgkid))
	{ 
		fprintf(stream, "\tPeer GK ID \t%s\n", infoEntry->pgkid);
	}

	 fprintf(stream, "\tGRQ\t\t%s\n", BIT_TEST(infoEntry->cap, CAP_GRQ)?
		"enabled":"disabled");

	 fprintf(stream, "\tRAI\t\t%s\n", BIT_TEST(infoEntry->cap, CAP_RAI)?
		"enabled":"disabled");

	 fprintf(stream, "\tTech Prefix Gw\t%s\n", BIT_TEST(infoEntry->cap, CAP_TPG)?
		"enabled":"disabled");

	if (strlen(infoEntry->techprefix))
	{ 
		fprintf(stream, "\tTech Prefix \t%s\n", infoEntry->techprefix);
	}

     fprintf(stream, "\tCall Route Plan %s\n", infoEntry->cpname);
	
	 if (infoEntry->custID[0])
	 {
     	fprintf(stream, "\tCustomer ID \t%s\n", infoEntry->custID);
	 }

	 if (infoEntry->tg[0])
	 {
     	fprintf(stream, "\tTrunk Group \t%s\n", infoEntry->tg);
	 }

	 if (infoEntry->srcIngressTG[0])
	 {
     	fprintf(stream, "\tMSW New Source Ingress Trunk Group \t%s\n", infoEntry->srcIngressTG);
	 }

	 if (infoEntry->dtg[0])
	 {
     	fprintf(stream, "\tDestination Trunk Group \t%s\n", infoEntry->dtg);
	 }

	 if (infoEntry->srcEgressTG[0])
	 {
     	fprintf(stream, "\tMSW New Source Egress Trunk Group \t%s\n", infoEntry->srcEgressTG);
	 }

	 if (infoEntry->ogprefix[0])
	 {
     	fprintf(stream, "\tOutgoing Prefix %s\n", infoEntry->ogprefix);
	 }

	 fprintf(stream, "\tSip Uri \t%s\n", infoEntry->uri);
	 fprintf(stream, "\tSip Contact \t%s\n", infoEntry->contact);

	fprintf(stream, "\tSIP Registrations \t%s\n", 
			(infoEntry->stateFlags & CL_UAREG)?"enabled":"disabled");

	fprintf(stream, "\tNAT detect \t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NATDETECT)? "enabled":"disabled");

	fprintf(stream, "\tNAT IP \t%s\n", BIT_TEST(infoEntry->sflags, ISSET_NATIP) ? FormatIpAddress(infoEntry->natIp, s1) : "");

	fprintf(stream, "\tNAT Port \t%u\n", BIT_TEST(infoEntry->sflags, ISSET_NATPORT) ? infoEntry->natPort : 0);

     tmptime = infoEntry->iTime;

     fprintf(stream, "\tInceptionTime \t%s", 
			 ctime(&tmptime));

     tmptime = infoEntry->rTime;

     fprintf(stream, "\tRefreshTime \t%s", 
			 ctime(&tmptime));

     tmptime = infoEntry->mTime;

     fprintf(stream, "\tModTime \t%s", 
			 ctime(&tmptime));

     fprintf(stream, "\tLastUseTime \t%ld\n", infoEntry->cTime);

     if (tmptime = infoEntry->iaTime)
	 {
     	fprintf(stream, "\tLastInactive \t%s", 
			 ctime(&tmptime));
	 }

     if (tmptime = infoEntry->attTime)
	 {
     	fprintf(stream, "\tLastAttempt \t%s", 
			 ctime(&tmptime));
	 }

     if (tmptime = infoEntry->dndTime)
	 {
     	fprintf(stream, "\tLastDND \t%s", 
			 ctime(&tmptime));
	 }

	fprintf(stream,"\tVendor \t\t%s\n",
						GetVendorDescription (infoEntry->vendor));

	fprintf(stream,"\tPassword\t%s\n",infoEntry->passwd);

	fprintf(stream, "\tMedia Routing\t%s\n", 
			IsMediaRoutingEnabled(infoEntry)?"enabled":"disabled");
	fprintf(stream, "\tNever Media Route\t%s\n", 
			IsNeverMediaRouteEnabled(infoEntry)?"enabled":"disabled");
	fprintf(stream, "\tHide Address Change\t%s\n", 
			IsHideAddressChangeEnabled(infoEntry)?"enabled":"disabled");
	fprintf(stream, "\tH323 Display\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NOH323DISPLAY)?"disabled":"enabled");
	fprintf(stream, "\tDirect Calls\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NODIRECTCALLS)?"disabled":"enabled");
	fprintf(stream, "\tMap Alias\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_MAPALIAS)?"enabled":"disabled");

	fprintf(stream, "\tController Id\t%d\n", infoEntry->crId);
	fprintf(stream, "\tForce H245\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_FORCEH245)?"enabled":"disabled");
	fprintf(stream, "\tPI on Fast Start\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_PIONFASTSTART)?"enabled":"disabled");
	fprintf(stream, "\tCalled Party Number Type\t%s\n", 
			GetQ931CdpnStr(infoEntry->q931IE[Q931IE_CDPN]));
	fprintf(stream, "\tCalling Party Number Type\t%s\n", 
			GetQ931CgpnStr(infoEntry->q931IE[Q931IE_CGPN]));
	fprintf(stream, "\tBearer Capability Layer1\t%s\n", 
		GetBcapLayer1Str(infoEntry->bcap[BCAP_LAYER1]));
	fprintf(stream, "\tConn H.245 addr\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NOCONNH245)?"disabled":"enabled");
	fprintf(stream, "\tSet Destination Trunk Group\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_SETDESTTG)?"enabled":"disabled");
	if ( !(infoEntry->ecaps1 & ECAPS1_DELTCST38DFT) )
	{
		fprintf(stream, "\tRemove T.38 from TCS\tdefault\n" );
	}
	else
	{
		fprintf(stream, "\tRemove T.38 from TCS\t%s\n", 
				(infoEntry->ecaps1 & ECAPS1_DELTCST38)?"enabled":"disabled");
	}
	if ( !(infoEntry->ecaps1 & ECAPS1_DELTCSRFC2833DFT) )
	{
		fprintf(stream, "\tRemove RFC2833 Capability from TCS\tdefault\n" );
	}
	else
	{
		fprintf(stream, "\tRemove RFC2833 Capability from TCS\t%s\n", 
				(infoEntry->ecaps1 & ECAPS1_DELTCSRFC2833)?"enabled":"disabled");
	}
	fprintf(stream, "\tInformation Transfer Capability \t " );

	pStr = enum2str( infoTransCapOptions,IedgeInfoTransCap(infoEntry));
	fprintf(stream, "%s\n", ( pStr == NULL ? "unknown" : pStr));

	fprintf(stream, "\tRemove Trunk Group ID\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_NOTG)?"enabled":"disabled");

	fprintf(stream, "\t2833 Capable\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_CAP2833_KNOWN) ? ( (infoEntry->ecaps1 & ECAPS1_CAP2833)?"yes":"no") : "unknown");

	fprintf(stream, "\tMap ISDN Code Code\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_MAPISDNCC)?"enabled":"disabled");

	if((infoEntry->ecaps1 & ECAPS1_SIP_PRIVACY_RFC3325) && (infoEntry->ecaps1 & ECAPS1_SIP_PRIVACY_DRAFT01)) {
	  fprintf(stream,"\tSip Privacy \t%s\n","both");
	}
	else if((infoEntry->ecaps1 & ECAPS1_SIP_PRIVACY_RFC3325) == ECAPS1_SIP_PRIVACY_RFC3325) {
	  fprintf(stream,"\tSip Privacy \t%s\n","rfc3325");
	}
	else if((infoEntry->ecaps1 & ECAPS1_SIP_PRIVACY_DRAFT01) == ECAPS1_SIP_PRIVACY_DRAFT01) {
	  fprintf(stream,"\tSip Privacy \t%s\n","draft01");
	} 
        fprintf(stream, "\tCid Block\t%s\n",infoEntry->cidblock == 1 ? "enabled":"disabled");
        
    if (infoEntry->stateFlags & CL_STICKY)
    {
	  	fprintf(stream, "Sticky Enabled");
    }

	return 0;
}

int
PrintGkInfoEntry(FILE *stream, GkInfo *gkInfo)
{
	time_t now, delta;
	char epid[ENDPOINTID_LEN];

	fprintf(stream, "\tRegistration ID %s\n", gkInfo->regid);

	fprintf(stream, "\tPort \t\t%lu\n", gkInfo->uport);

	fprintf(stream, "\tReg State\t%s\n",
			gkRegState[gkInfo->regState]);

	fprintf(stream, "\tTTL \t\t%d\n", gkInfo->regttl);
	
	if (gkInfo->flags & GKCAP_LTWTREG)
	{
		fprintf(stream, "\tSupports LRRQ\n");
	}
	else
	{
		fprintf(stream, "\tDoes not Support LRRQ\n");
	}

	hex2chr(epid, ENDPOINTID_LEN,  
		gkInfo->endpointIDString, gkInfo->endpointIDLen);

	fprintf(stream, "\tepid \t\t%s\n", epid);

	now = time(NULL);	
	delta = gkInfo->nextEvent - now;

	fprintf(stream, "\tSchedule \t%lus\n", delta);

	return 0;
}

int
PrintDbAttrEntry(FILE *stream, ClientAttribs *clAttribs)
{
	if (clAttribs == NULL)
	{
		return 0;
	}

     /* Print some attribute Information */
     fprintf(stream, "\tFirst Name \t%s \n\tLast Name \t%s \n\tLocation \t%s \n\tCountry \t%s\n",
			 clAttribs->clFname, clAttribs->clLname, clAttribs->clLoc,
			 clAttribs->clCountry);
     fprintf(stream, "\tComments \t%s\n", clAttribs->clComments);

	return 0;
}

int
DEBUG_PrintInfoEntry(int module, int level, InfoEntry *infoEntry)
{
     char s1[25], vpnId[PHONE_NUM_LEN];
     time_t tmptime;

     if (BIT_TEST(infoEntry->sflags, ISSET_REGID))
     {
		  DEBUG(module, level,
				("\tRegistrationID \t%s\n", infoEntry->regid));
     }

     if (BIT_TEST(infoEntry->sflags, ISSET_UPORT))
     {
		  DEBUG(module, level,
				("\tPort \t%lu\n", infoEntry->uport));
     }

     tmptime = infoEntry->iTime;

     DEBUG(module, level,
		   ("\tInceptionTime \t%s", ctime(&tmptime)));

     tmptime = infoEntry->rTime;

     DEBUG(module, level, ("\tRefreshTime \t%s", 
						   ctime(&tmptime)));

     tmptime = infoEntry->mTime;

     DEBUG(module, level, ("\tRefreshTime \t%s", 
						   ctime(&tmptime)));

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
     {
		  DEBUG(module, level, ("\tPhone \t%s\n",
								infoEntry->phone));
     }
     
     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_VPNPHONE))
     {
		  memset(vpnId, 0, PHONE_NUM_LEN);

		  if (strlen(infoEntry->vpnPhone) < infoEntry->vpnExtLen)
		  {
			   NETERROR(module, (" Db/Cache Info Format Error !!\n"));
			   return 0;
		  }

		  memcpy(vpnId, infoEntry->vpnPhone, 
				 strlen(infoEntry->vpnPhone)-infoEntry->vpnExtLen);

		  DEBUG(module, level, ("\tVpnPhone \t%s %s\n",
								vpnId, ((char *)infoEntry->vpnPhone) + 
								strlen(infoEntry->vpnPhone)-
								infoEntry->vpnExtLen));
     }

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_IPADDRESS))
     {
		  DEBUG(module, level,
				("\tIpAddr \t%s\n", 
				 FormatIpAddress(infoEntry->ipaddress.l, s1)));
     }
	 
	 DEBUG(module, level,
		   ("\tRasIpAddr \t%s/%d\n", 
			FormatIpAddress(infoEntry->rasip, s1), infoEntry->rasport));

     if (infoEntry->stateFlags & CL_ACTIVE)
     {
		  DEBUG(module, level, ("\tActive "));
     }
     else if (infoEntry->stateFlags & CL_STATIC)
     {
		  DEBUG(module, level, ("\tStatic "));
     }
     else
     {
		  DEBUG(module, level, ("\tInactive "));
     }

#if 0
     if (infoEntry->stateFlags & CL_PROXY)
     {
		  if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == 
			  (CL_PROXY|CL_PROXYING))
		  {
			   DEBUG(module, level, ("\tProxied (%s %s)", 
									 infoEntry->xphone, infoEntry->xvpnPhone));
		  }
		  else
		  {
			   DEBUG(module, level, ("\tProxied (Absent)"));
		  }
     }

     if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == CL_PROXYING)
     {
		  DEBUG(module, level, ("\tProxying (%s %s)", 
								infoEntry->xphone, infoEntry->xvpnPhone));
     }
#endif

     if (infoEntry->stateFlags & CL_FORWARD)
     {
		  DEBUG(module, level, ("\t Forwarding On"));
		  if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE))
		  {
			   DEBUG(module, level, ("\t Fwd Phone %s\n", infoEntry->nphone));
		  }
		  if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE))
		  {
			   DEBUG(module, level, ("\t Fwd vpn phone %s/%lu\n", 
									 infoEntry->nvpnPhone, infoEntry->nvpnExtLen));
		  }
     }

     if (strlen(infoEntry->zone))
     {
		  DEBUG(module, level, ("\t zone %s\n", infoEntry->zone));
     }

     if (infoEntry->stateFlags & CL_DND)
     {
		  DEBUG(module, level, ("\t DND On\n"));
     }

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_EMAIL))
     {
		  DEBUG(module, level, ("\tEmail \t%s\n", infoEntry->email));
     }

     DEBUG(module, level, ("\tcalling plan %s\n", infoEntry->cpname));

     DEBUG(module, level, ("\tMedia Routing %s\n", IsMediaRoutingEnabled(infoEntry)?"enabled":"disabled"));
     DEBUG(module, level, ("\tNever Media Route %s\n", IsNeverMediaRouteEnabled(infoEntry)?"enabled":"disabled"));
     DEBUG(module, level, ("\tHide Address Change %s\n", IsHideAddressChangeEnabled(infoEntry)?"enabled":"disabled"));
	 DEBUG(module, level, ("\tH323 Display\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NOH323DISPLAY)?"disabled":"enabled"));
	 DEBUG(module, level, ("\tDirect Calls\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NODIRECTCALLS)?"disabled":"enabled"));
	 DEBUG(module, level, ("\tMap Alias\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_MAPALIAS)?"enabled":"disabled"));

	 DEBUG(module, level, ("\tController Id\t%d\n", infoEntry->crId));
	 DEBUG(module, level, ("\tForce H245\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_FORCEH245)?"enabled":"disabled"));
	 DEBUG(module, level, ("\tPI on Fast Start\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_PIONFASTSTART)?"enabled":"disabled"));
	 DEBUG(module, level, ("\tCalled Party Number Type\t%s\n", 
			GetQ931CdpnStr(infoEntry->q931IE[Q931IE_CDPN])));
	 DEBUG(module, level, ("\tCalling Party Number Type\t%s\n", 
			GetQ931CgpnStr(infoEntry->q931IE[Q931IE_CGPN])));

     DEBUG(module, level, ("\n\n"));

	return 0;
}

int
ERROR_PrintInfoEntry(int module, InfoEntry *infoEntry)
{
     char s1[25], vpnId[PHONE_NUM_LEN];
     time_t tmptime;

     if (BIT_TEST(infoEntry->sflags, ISSET_REGID))
     {
	  NETERROR(module,
		("\tRegistrationID \t%s\n", infoEntry->regid));
     }

     if (BIT_TEST(infoEntry->sflags, ISSET_UPORT))
     {
	  NETERROR(module, 
		("\tPort \t%lu\n", infoEntry->uport));
     }

     tmptime = infoEntry->iTime;

     NETERROR(module, 
	("\tInceptionTime \t%s", ctime(&tmptime)));

     tmptime = infoEntry->rTime;

     NETERROR(module, ("\tRefreshTime \t%s", 
	     ctime(&tmptime)));

     tmptime = infoEntry->mTime;

     NETERROR(module, ("\tRefreshTime \t%s", 
	     ctime(&tmptime)));

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
     {
	  NETERROR(module, ("\tPhone \t%s\n",
		  infoEntry->phone));
     }
     
     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_VPNPHONE))
     {
	  memset(vpnId, 0, PHONE_NUM_LEN);

	  if (strlen(infoEntry->vpnPhone) < infoEntry->vpnExtLen)
	  {
		NETERROR(module, (" Db/Cache Info Format Error !!\n"));
		return 0;
	  }

	  memcpy(vpnId, infoEntry->vpnPhone, 
			strlen(infoEntry->vpnPhone)-infoEntry->vpnExtLen);

	  NETERROR(module, ("\tVpnPhone \t%s %s\n",
		  vpnId, ((char *)infoEntry->vpnPhone) + 
			strlen(infoEntry->vpnPhone)-
		infoEntry->vpnExtLen));
     }

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_IPADDRESS))
     {
	  NETERROR(module,
		("\tIpAddr \t%s/%d\n", 
		  FormatIpAddress(infoEntry->ipaddress.l, s1),infoEntry->callsigport));
     }

     if (infoEntry->stateFlags & CL_ACTIVE)
     {
	  NETERROR(module, ("\tActive "));
     }
     else if (infoEntry->stateFlags & CL_STATIC)
     {
	  NETERROR(module, ("\tStatic "));
     }
     else
     {
	  NETERROR(module, ("\tInactive "));
     }

#if 0
     if (infoEntry->stateFlags & CL_PROXY)
     {
     	if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == 
			(CL_PROXY|CL_PROXYING))
     	{
		NETERROR(module, ("\tProxied (%s %s)", 
			infoEntry->xphone, infoEntry->xvpnPhone));
     	}
	else
	{
	  	NETERROR(module, ("\tProxied (Absent)"));
	}
     }

     if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == CL_PROXYING)
     {
	  NETERROR(module, ("\tProxying (%s %s)", 
		infoEntry->xphone, infoEntry->xvpnPhone));
     }
#endif
     if (infoEntry->stateFlags & CL_FORWARD)
     {
	  NETERROR(module, ("\t Forwarding On"));
	  if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE))
	  {
		NETERROR(module, ("\t Fwd Phone %s\n", infoEntry->nphone));
	  }
	  if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE))
	  {
		NETERROR(module, ("\t Fwd vpn phone %s/%lu\n", 
			infoEntry->nvpnPhone, infoEntry->nvpnExtLen));
	  }
     }

     if (strlen(infoEntry->zone))
     {
	NETERROR(module, ("\t zone %s\n", infoEntry->zone));
     }

     if (infoEntry->stateFlags & CL_DND)
     {
	NETERROR(module, ("\t DND On\n"));
     }

     if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_EMAIL))
     {
	 NETERROR(module, ("\tEmail \t%s\n", infoEntry->email));
     }

     NETERROR(module, ("\tcalling plan %s\n", infoEntry->cpname));

     NETERROR(module, ("\tMedia Routing %s\n", IsMediaRoutingEnabled(infoEntry)?"enabled":"disabled"));

     NETERROR(module, ("\tNever Media Route %s\n", IsNeverMediaRouteEnabled(infoEntry)?"enabled":"disabled"));

     NETERROR(module, ("\tHide Address Change %s\n", IsHideAddressChangeEnabled(infoEntry)?"enabled":"disabled"));

	 NETERROR(module, ("\tH323 Display\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NOH323DISPLAY)?"disabled":"enabled"));
	 NETERROR(module, ("\tDirect Calls\t%s\n", 
			(infoEntry->ecaps1 & ECAPS1_NODIRECTCALLS)?"disabled":"enabled"));
	 NETERROR(module, ("\tMap Alias\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_MAPALIAS)?"enabled":"disabled"));

	 NETERROR(module, ("\tController Id\t%d\n", infoEntry->crId));
	 NETERROR(module, ("\tForce H245\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_FORCEH245)?"enabled":"disabled"));
	 NETERROR(module, ("\tPI on Fast Start\t%s\n", 
		(infoEntry->ecaps1 & ECAPS1_PIONFASTSTART)?"enabled":"disabled"));
	 NETERROR(module, ("\tCalled Party Number Type\t%s\n", 
			GetQ931CdpnStr(infoEntry->q931IE[Q931IE_CDPN])));
	 NETERROR(module, ("\tCalling Party Number Type\t%s\n", 
			GetQ931CgpnStr(infoEntry->q931IE[Q931IE_CGPN])));

     NETERROR(module, ("\n\n"));

	return 0;
}


int
PrintVpnEntry(FILE *stream, VpnEntry *vpnEntry)
{
		fprintf(stream, "Retrieved vpn name %s\n", vpnEntry->vpnName);
	 	fprintf(stream, "Retrieved vpn id %s\n", vpnEntry->vpnId);
	 	fprintf(stream, "Retrieved vpn prefix %s\n", vpnEntry->prefix);
		fprintf(stream, "Retrieved vpn location %s\n", vpnEntry->vpnLoc);
	  	fprintf(stream, "\tvpn entry ext len %d\n", vpnEntry->vpnExtLen);
	  	fprintf(stream, "\tvpn entry group \"%s\"\n", vpnEntry->vpnGroup);

	return 0;
}

int
PrintCPEntry(FILE *stream, CallPlanEntry *entry)
{
   	time_t tmptime;

	fprintf(stream, "%s\n", entry->cpname);
   	tmptime = entry->mTime;
   	fprintf(stream, "\tRefreshTime \t%s", ctime(&tmptime));

	return 0;
}

int
PrintCPBEntry(FILE *stream, CallPlanBindEntry *entry)
{
   	time_t tmptime;

	fprintf(stream, "\t\t%s/%s\n", entry->cpname, entry->crname);
	fprintf(stream, "\t\tPriority \t%d\n", entry->priority);
	if (entry->crflags & (CRF_FORWARD|CRF_ROLLOVER))
	{
		fprintf(stream, "\t\tType \t\t%s\n", 
			(entry->crflags&CRF_FORWARD)?"forward":"rollover");
	}

	fprintf(stream, "\t\tType \t\t%s\n",
		(entry->crflags&CRF_REJECT)?"reject":"normal");

	fprintf(stream, "\t\tStart Time \tHH:%d/MM:%d/SS:%d\n",
		entry->sTime.tm_hour,
		entry->sTime.tm_min,
		entry->sTime.tm_sec);
		
	fprintf(stream, "\t\tStart Time \tYYYY:%d/DOY:%d/DOW:%d/MM:%d/DOM:%d\n",
			(entry->sTime.tm_year>=0)?entry->sTime.tm_year+1900:entry->sTime.tm_year,
			entry->sTime.tm_yday,
			entry->sTime.tm_wday,
            entry->sTime.tm_mon,
            entry->sTime.tm_mday);

	fprintf(stream, "\t\tFinish Time \tHH:%d/MM:%d/SS:%d\n",
		entry->fTime.tm_hour,
		entry->fTime.tm_min,
		entry->fTime.tm_sec);

	fprintf(stream, "\t\tFinish Time \tYYYY:%d/DOY:%d/DOW:%d/MM:%d/DOM:%d\n",
			(entry->fTime.tm_year>=0)?entry->fTime.tm_year+1900:entry->fTime.tm_year,
			entry->fTime.tm_yday,
			entry->fTime.tm_wday,
            entry->fTime.tm_mon,
            entry->fTime.tm_mday);

   	tmptime = entry->mTime;
   	fprintf(stream, "\t\tRefreshTime \t%s", ctime(&tmptime));

	return 0;
}

int
PrintCREntry(FILE *stream, VpnRouteEntry *entry)
{
   	time_t tmptime;

	fprintf(stream, "\t%s\n", entry->crname);
	fprintf(stream, "\t\tdest = %s/%s\n", entry->dest, entry->prefix);
	if (entry->destlen != 0)
	{
		fprintf(stream, "\t\tdestlen %d\n", entry->destlen);
	}
	fprintf(stream, "\t\tsrc = %s/%s\n", entry->src, entry->srcprefix);
	if (entry->srclen != 0)
	{
		fprintf(stream, "\t\tsrclen %d\n", entry->srclen);
	}

	if (entry->crflags != 0)
	{
		fprintf(stream, "\t\t%s\n", RouteFlagsString(entry->crflags&(CRF_CALLORIGIN|CRF_CALLDEST|CRF_TRANSIT)));
		fprintf(stream, "\t\tdnis default %s\n",
			entry->crflags&CRF_DNISDEFAULT?"enabled":"disabled");
	}

	fprintf(stream, "\t\tType \t\t%s\n",
		(entry->crflags&CRF_REJECT)?"reject":"normal");

	if (entry->crflags&CRF_TEMPLATE)
	{
		fprintf(stream, "\t\tTemplate Route\n");
	}

	if (entry->crflags&CRF_STICKY)
	{
		fprintf(stream, "\t\tSticky Enabled\n");
	}

	if (entry->trname[0])
	{
		fprintf(stream, "\t\tTrigger Route based on %s\n", 
			entry->trname);
	}

   	tmptime = entry->mTime;
   	fprintf(stream, "\t\tRefreshTime \t%s", ctime(&tmptime));

	return 0;
}

void
PrintTriggerEntry(FILE *stream, TriggerEntry *tgEntry)
{
	fprintf(stream, "trigger %s\n", tgEntry->name);

	if(tgEntry->event == TRIGGER_EVENT_H323REQMODEFAX)
	{
		fprintf(stream, "\tEvent \t\th.323 request mode\n");
	}
	else
	{
		fprintf(stream, "\tEvent \t\th.323 t.38 fax\n");
	}
	fprintf(stream, "\tSrc Vendor \t\t%s\n",
						GetVendorDescription (tgEntry->srcvendor));
	fprintf(stream, "\tDst Vendor \t\t%s\n",
						GetVendorDescription (tgEntry->dstvendor));

	fprintf(stream, "\tScript \t\tinsert route\n");
	fprintf(stream, "\tScript Data \t\t%s\n", tgEntry->actiondata);
	if(tgEntry->actionflags & TRIGGER_FLAG_ROUTE_OVERRIDE)
	{
		fprintf(stream, "\tOverride route \t\t%s\n", "enabled");
	}
	else
	{
		fprintf(stream, "\tOverride route \t\t%s\n", "disabled");
	}
}

static char *mediaRoutingStr_a[] = 
        {"Don't Care", "Always On", "Always Off", "On"};
int
PrintRealmEntry(FILE *stream, RealmEntry *rmEntry)
{
	fprintf(stream, "Realm %s/%lu\n", rmEntry->realmName, rmEntry->realmId);

	fprintf(stream, "\tRsa\t\t\t%s\n", ULIPtostring(rmEntry->rsa));
	fprintf(stream, "\tMask\t\t\t%s\n", ULIPtostring(rmEntry->mask));
    fprintf(stream, "\tAdm Status\t\t%s\n", rmEntry->adminStatus ? "ENABLED" : "DISABLED");
    fprintf(stream, "\tOper Status\t\t%s\n", rmEntry->operStatus ? "UP" : "DOWN");
	fprintf(stream, "\tSig Poolid\t\t%u\n",rmEntry->sigPoolId);
	fprintf(stream, "\tMedia Poolid\t\t%u\n",rmEntry->medPoolId);
	fprintf(stream, "\tAddressing Type\t\t%s\n",
						(rmEntry->addrType? "Private" : "Public" ));

	fprintf(stream, "\tBetween-Realms MR\t%s\n", mediaRoutingStr_a[rmEntry->interRealm_mr]);
	fprintf(stream, "\tWithin-Realm MR\t\t%s\n", mediaRoutingStr_a[rmEntry->intraRealm_mr]);
    fprintf(stream, "\tVirt Interface\t\t%s\n", rmEntry->vipName[0]=='\0'?"UNASSIGNED":rmEntry->vipName);

    if( strlen(rmEntry->mp.regid) )
    {            
            fprintf(stream, "\tproxy \t%s\t%lu\n", rmEntry->mp.regid,rmEntry->mp.uport);
    } 
    else 
    {
            fprintf(stream, "\tproxy \n");            
    }
        
    fprintf(stream, "\tSip Authorization\t\t%s %s %s\n", (rmEntry->authFlags & REALM_SIP_AUTH_INVITE)? "inv": "", (rmEntry->authFlags & REALM_SIP_AUTH_REGISTER)? "reg": "", (rmEntry->authFlags & REALM_SIP_AUTH_BYE)? "bye": "");

    fprintf(stream, "\tCIDBlock \t\t%s\n",strlen(rmEntry->cidblk)==0?"":rmEntry->cidblk);
    fprintf(stream, "\tCIDUnBlock \t\t%s\n",strlen(rmEntry->cidunblk)==0?"":rmEntry->cidunblk);

    if (!strcmp(rmEntry->vnetName, VNET_UNASSIGNED))
    {
      fprintf(stream, "\tVNET \t\t\t%s\n", "UNASSIGNED");
    } else
    {
      fprintf(stream, "\tVNET \t\t\t%s\n", rmEntry->vnetName);
    }

	fprintf(stream, "\n");

	return 0;
}

int
PrintIgrpEntry(FILE *stream, IgrpInfo *igrp)
{
	fprintf(stream, "Iedge Group\t%s\n", igrp->igrpName);

	fprintf(stream, "\tCallsIn\t\t%d\n", IgrpInCalls(igrp));
	fprintf(stream, "\tCallsOut\t%d\n", IgrpOutCalls(igrp));
	fprintf(stream, "\tCallsTotal\t%d\n", IgrpCallsTotal(igrp));

	fprintf(stream, "\tmaxCallsIn\t%d\n", IgrpXInCalls(igrp));
	fprintf(stream, "\tmaxCallsOut\t%d\n", IgrpXOutCalls(igrp));
	fprintf(stream, "\tmaxCallsTotal\t%d\n", IgrpXCallsTotal(igrp));

	fprintf(stream, "\tLast Dnd Time\t%s",ctime(&igrp->dndTime));
	return 0;
}

int
PrintVnetEntry(FILE *stream, VnetEntry *vnetEntry)
{
	fprintf(stream, "Vnet\t%s\n", vnetEntry->vnetName);

	fprintf(stream, "\tMain Interface\t\t%s\n",vnetEntry->ifName[0]=='\0'?"UNASSIGNED":vnetEntry->ifName);
#ifdef NETOID_LINUX
	if (vnetEntry->vlanid == VLANID_NONE)
	{
		fprintf(stream, "\tVLAN Id\t\t\t%s\n", "None");
	} else
	{
		fprintf(stream, "\tVLAN Id\t\t\t%d\n", vnetEntry->vlanid);
	}
	fprintf(stream, "\tRouting Table Id\t%d\n", vnetEntry->rtgTblId);
	fprintf(stream, "\tGateway\t\t\t%s\n", ULIPtostring(vnetEntry->gateway));
#endif

	return 0;
}


int
DEBUG_PrintPhoNode(int module, int level, PhoNode *node)
{
        if (BIT_TEST(node->sflags, ISSET_REGID))                             
        {                                                                       
                DEBUG(module, level, ("\tRegistration ID %s\n", node->regid));             
        }                                                                       
                                                                                
        if (BIT_TEST(node->sflags, ISSET_UPORT))                                
        {                                                                       
                DEBUG(module, level, ("\tUport %lu\n", node->uport));                           
        }                                                                       
                                                                                
        if (BIT_TEST(node->sflags, ISSET_IPADDRESS))                            
        {                                                                       
                DEBUG(module, level, ("\tIPADDRESS %s\n", ULIPtostring(node->ipaddress.l)));   
        }                                                                       
                                                                                
        if (BIT_TEST(node->sflags, ISSET_PHONE))                                
        {                                                                       
                DEBUG(module, level, ("\tPhone %s\n", node->phone));                           
        }                                                                       
                                                                                
        if (BIT_TEST(node->sflags, ISSET_VPNPHONE))                             
        {                                                                       
                DEBUG(module, level, 
			("\tVpn Phone %s/%lu\n", 
			node->vpnPhone, node->vpnExtLen)); 
        }                                                                       

	return 0;
}

#ifndef _DMALLOC_
#ifdef SUNOS
char *
strsep(char **stringp, const char *delim)
{
	char *ptrptr;
	return strtok_r(*stringp, delim, &ptrptr);
}
#endif
#endif

char *
strdupextra(char *src, int extra)
{
	char *dest;
	
	dest = (char *)malloc(strlen(src)+1+extra);
	strcpy(dest, src);
	return dest;
}

int
DEBUG_PrintBuffer(int module, int level, char *buf, int len)
{
#ifndef MIN
#define MIN(x,y) ((x<y)?x:y)
#endif
	int i, plen;
	char string[49];

	for (i=0; i<len; i+=48)
	{
		plen = MIN(len-i, 48);
		memcpy(string, buf+i, plen);
		string[plen] = '\0';
		DEBUG(module, level, ("%s", string));
	}
	DEBUG(module, level, ("\n"));

	return 0;
}

char *
GetInput(FILE *stream, char *buf, int xinlen)
{
	int i;
	int c;

	memset(buf, 0, xinlen);

	for (i = 0; (c = fgetc(stream)); i ++) 
	{
		if ((c == EOF) || (c == '\n') || (c == '\r'))
		{
			break;
		}

		if (i < xinlen)
		{
			buf[i] = (char )c;
		}
	}

	buf[xinlen-1] = '\0';

#if 0
	fgets(buf, xinlen + 1, stream);
	len = strcspn(buf, reject);
	
	if (len > xinlen)
	{
		fprintf(stderr, "Error in in strcspn\n");
	}

	buf[len] = '\0';
#endif
	return buf;
}

int
FindIedgeType(char *type)
{
	int i;

	for (i=0; i<IEDGE_MAXTYPES; i++)
	{
		if (!strcmp(type, iedge_types[i].name))
		{
			return iedge_types[i].type;
		}
	}

	return -1;
}

/* ONLY for endpoints which can register w/ the iServer */
int
IedgeTypeFromCap(PhoNode *phonodep)
{
	int 	is510 = (BIT_TEST(phonodep->cap, CAP_IGATEWAY) &&
			!BIT_TEST(phonodep->cap, CAP_I1000)),
		is1000 = BIT_TEST(phonodep->cap, CAP_I1000),
		is500 = (!BIT_TEST(phonodep->cap, CAP_I1000) &&
			!BIT_TEST(phonodep->cap, CAP_IGATEWAY) &&
			!BIT_TEST(phonodep->cap, CAP_IMOBILE)),
		ispc = BIT_TEST(phonodep->cap, CAP_IMOBILE);
	int 	eptype;

	eptype = 	(is500) ? IEDGE_500: 
			((is510) ? IEDGE_510:
			((ispc) ? IEDGE_IMOBILE:
			((is1000) ? IEDGE_1000: -1)));

	return eptype;
}

int
SetIedgeType(NetoidInfoEntry *netInfo, int type)
{
	SetIedgeTypeMandatory(netInfo, type);
	SetIedgeTypeOptional(netInfo, type);
	return(0);
}

#define UNINITIALIZED_MAXCALLS(_x_)	(!(_x_))

// set defaults for optional values
int
SetIedgeTypeOptional(NetoidInfoEntry *netInfo, int type)
{
	netInfo->ispcorgw = type;

	switch (type)
	{
	case IEDGE_500:
	case IEDGE_IMOBILE:
	case IEDGE_1000:

	if (UNINITIALIZED_MAXCALLS(IedgeXCalls(netInfo)))
	{
		IedgeXCalls(netInfo) = 2;
	}

	break;

	case IEDGE_510:
	case IEDGE_1000G:

	if (UNINITIALIZED_MAXCALLS(IedgeXCalls(netInfo)))
	{
		IedgeXCalls(netInfo) = 1;
	}

	break;

	case IEDGE_ISERVER:
	case IEDGE_XGK:
	case IEDGE_XGW:
	case IEDGE_SGK:
	case IEDGE_ENUMS:
	case IEDGE_SIPPROXY:
	case IEDGE_SIPGW:
	case IEDGE_SOFTSW:
	case IEDGE_USERACC:
	case IEDGE_IPPHONE:

	break;

	default:

	IedgeXCalls(netInfo) = 0;
	break;
	}

	return 0;
}

int
SetIedgeTypeMandatory(NetoidInfoEntry *netInfo, int type)
{
	netInfo->ispcorgw = type;

	switch (type)
	{
	case IEDGE_500:
	case IEDGE_510:
	case IEDGE_1000:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ARQ);
	BIT_RESET(netInfo->cap, CAP_TPG);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_IGATEWAY);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_GRQ);
	BIT_RESET(netInfo->cap, CAP_RAI);

	if (type == IEDGE_510)
	{
		BIT_SET(netInfo->cap, CAP_IGATEWAY);
	}

	if (type == IEDGE_1000)
	{
		BIT_SET(netInfo->cap, CAP_I1000);
	}

	if (type == IEDGE_1000G)
	{
		BIT_SET(netInfo->cap, CAP_I1000);
		BIT_SET(netInfo->cap, CAP_IGATEWAY);
	}

	break;

	case IEDGE_IMOBILE:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ARQ);
	BIT_RESET(netInfo->cap, CAP_TPG);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IGATEWAY);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_GRQ);
	BIT_RESET(netInfo->cap, CAP_RAI);

	BIT_SET(netInfo->cap, CAP_IMOBILE);

	break;

	case IEDGE_XGW:
	case IEDGE_SOFTSW:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ARQ);
	BIT_RESET(netInfo->cap, CAP_TPG);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_GRQ);
	BIT_RESET(netInfo->cap, CAP_RAI);

	BIT_SET(netInfo->cap, CAP_IGATEWAY);

	break;

	case IEDGE_XGK:

	BIT_RESET(netInfo->cap, CAP_ARQ);
	BIT_RESET(netInfo->cap, CAP_TPG);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_GRQ);
	BIT_RESET(netInfo->cap, CAP_RAI);
	BIT_RESET(netInfo->cap, CAP_SIP);

	BIT_SET(netInfo->cap, CAP_LRQ); 
	BIT_SET(netInfo->cap, CAP_IGATEWAY);

	break;

	case IEDGE_SGK:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_H323);
	BIT_RESET(netInfo->cap, CAP_SIP);

	BIT_SET(netInfo->cap, CAP_ARQ);
	BIT_SET(netInfo->cap, CAP_IGATEWAY);
	BIT_SET(netInfo->cap, CAP_H323);

	break;

	case IEDGE_ISERVER:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_H323);
	BIT_RESET(netInfo->cap, CAP_SIP);

	BIT_SET(netInfo->cap, CAP_ARQ);
	BIT_SET(netInfo->cap, CAP_IGATEWAY);
	BIT_SET(netInfo->cap, CAP_H323);
	BIT_SET(netInfo->cap, CAP_MSW);

	break;

	case IEDGE_ENUMS:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ARQ);
	BIT_RESET(netInfo->cap, CAP_TPG);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_GRQ);
	BIT_RESET(netInfo->cap, CAP_RAI);

	BIT_SET(netInfo->cap, CAP_IGATEWAY);
	BIT_SET(netInfo->cap, CAP_H323);
	BIT_SET(netInfo->cap, CAP_SIP);
	BIT_SET(netInfo->cap, CAP_ENUMS);

	/* will never be non-static */
	netInfo->stateFlags |= CL_STATIC;

	break;

	case IEDGE_USERACC:

	BIT_RESET(netInfo->cap, CAP_IGATEWAY);
	BIT_RESET(netInfo->cap, CAP_GRQ);
	BIT_RESET(netInfo->cap, CAP_RAI);

	case IEDGE_IPPHONE:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ARQ);
	BIT_RESET(netInfo->cap, CAP_TPG);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_I1000);

	break;

	case IEDGE_SIPPROXY:
	case IEDGE_SIPGW:

	BIT_RESET(netInfo->cap, CAP_LRQ);
	BIT_RESET(netInfo->cap, CAP_ARQ);
	BIT_RESET(netInfo->cap, CAP_TPG);
	BIT_RESET(netInfo->cap, CAP_ENUMS);
	BIT_RESET(netInfo->cap, CAP_IMOBILE);
	BIT_RESET(netInfo->cap, CAP_I1000);
	BIT_RESET(netInfo->cap, CAP_H323);
	BIT_RESET(netInfo->cap, CAP_GRQ);
	BIT_RESET(netInfo->cap, CAP_RAI);

	BIT_SET(netInfo->cap, CAP_IGATEWAY);
	BIT_SET(netInfo->cap, CAP_SIP);

	break;

	case IEDGE_SUBNET:

	netInfo->cap = 0;

	break;

	default:

	break;

	}

	return 0;
}

char *
IedgeName(int type)
{
	if ((type<0)||(type>=IEDGE_MAXTYPES))
	{
		return "undefined";
	}
	
	return iedge_types[type].name;
}

char *
RouteFlagsString(unsigned long flags)
{
	if (flags & CRF_CALLORIGIN)
	{
		return "origin";
	}
	else if (flags & CRF_TRANSIT)
	{
		return "transit";
	}
	else if (flags & CRF_CALLDEST)
	{
		return "dest";
	}

	return "";
}

/* NOTE: if you add a new route type make sure you add it
 *       to cr_attr_list ENDTAG in cli/one.y
 */
int
RouteFlagsValue(unsigned int crflags, char *string)
{
	crflags &= ~(CRF_CALLDEST|CRF_CALLORIGIN|CRF_TRANSIT);

	if (!strcmp(string, "origin") || !strcmp(string, "source"))
	{
		crflags |= CRF_CALLORIGIN;
		return crflags;
	}	
	
	if (!strcmp(string, "dest"))
	{
		crflags |= CRF_CALLDEST;
		return crflags;
	}
	
	if (!strcmp(string, "transit"))
	{
		crflags |= CRF_TRANSIT;
		return crflags;
	}
	
	return crflags|CRF_CALLDEST;
}

int
TimeStamp(int module, int level, char *msg)
{
	hrtime_t hrtime;

	hrtime = nx_gethrtime();
	NETDEBUG(module, level, ("t=%lld %s\n", hrtime, msg));
	return(0);
}

// extract the file name from a path
char *
getfilename(char *path, char *dirdelim)
{
	char *fname = path;
	char *ptrptr = NULL;

	if ((strtok_r(path, dirdelim, &ptrptr)) && ptrptr)
	{
		fname = ptrptr;
	}

	while ((strtok_r(NULL, dirdelim, &ptrptr)) && ptrptr)
	{
		fname = ptrptr;
	}

	return (fname);
}

int 
chrn2hex(char *str, int maxStrLen, unsigned char *hexStr)
{
    int i, i2;

    for (i = 0, i2 = 0; i < maxStrLen; i++, i2 += 2)
    {
      hexStr[i2 + 0] = 0;
      hexStr[i2 + 1] = str[i];
    }

    return i2;
}

int 
chr2hex(char *str, unsigned char *hexStr)
{
    return chrn2hex(str, strlen(str), hexStr);
}

int 
hex2chr(char *str, int xlen, unsigned char *hexStr, int hexLen)
{
    int i, i2;

    for (i = 0, i2 = 0; (i < hexLen/2) && (i < xlen); 
		i++, i2 += 2)
    {
      str[i] = hexStr[i2 + 1];
    }

    str[i] = 0;

    return i;
}

#define CLIPRINTF(x) fprintf x

int
PrintCallCache(FILE *out)
{
	char fn[] = "PrintCallCache():";
	CallHandle *callHandle1, *callHandle2;
	SipCallHandle   *sipCallHandle = NULL;
	ConfHandle *confHandle;
	char	ctimebuf[256];
	time_t now;
	char str1[CALL_ID_LEN], str2[CALL_ID_LEN], cfidNext[CONF_ID_LEN];
	int n = 0, m = 0;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	time(&now);

	for (confHandle = CacheGetFirst(confCache);
		 confHandle;
		 confHandle = CacheGetNext(confCache, cfidNext))
	{
		n++;

		ConfGetLegs(confHandle, &callHandle1, &callHandle2);

		CLIPRINTF((out, "Call %d\n", n));
		CLIPRINTF((out, "\t%s ---> %s\n", 
			callHandle1?CallID2String(callHandle1->callID, str1):"none",
			callHandle2?CallID2String(callHandle2->callID, str2):"none"));

		if (callHandle1)
		{
			CLIPRINTF((out, "\t%s/%lu ip:%s ani:%s ---> %s/%lu ip:%s dnis:%s dur:%lds\n",
				callHandle1->phonode.regid,
				callHandle1->phonode.uport, 
				FormatIpAddress(callHandle1->phonode.ipaddress.l, str1),
				callHandle1->inputANI,
				callHandle1->rfphonode.regid,
				callHandle1->rfphonode.uport, 
				FormatIpAddress(callHandle1->rfphonode.ipaddress.l, str2), 
				callHandle1->rfphonode.phone,
				now-timedef_sec(&callHandle1->callStartTime)));

			if (CallFceBundleId(callHandle1))
			{
				if (callHandle1->handleType == SCC_eH323CallHandle)
				{
					if (H323nremoteset(callHandle1))
					{
						CLIPRINTF((out, "\t(leg1) %s:%d ---> %s:%d\n",
							FormatIpAddress(CallFceTranslatedIp(callHandle1), str1),
							CallFceTranslatedPort(callHandle1),
							FormatIpAddress(CallFceUntranslatedIp(callHandle1), str2),
							CallFceUntranslatedPort(callHandle1)));
					}
					else
					{
						CLIPRINTF((out, "\tnone ---> none\n"));
					}
				}
				else if (callHandle1->handleType == SCC_eSipCallHandle)
				{
					sipCallHandle = SipCallHandle(callHandle1);
					if (sipCallHandle->localSet.localSet_len)
					{
						CLIPRINTF((out, "\t(leg1) %s:%d ---> %s:%d\n",
							FormatIpAddress(CallFceTranslatedIp(callHandle1), str1),
							CallFceTranslatedPort(callHandle1),
							FormatIpAddress(CallFceUntranslatedIp(callHandle1), str2),
							CallFceUntranslatedPort(callHandle1)));
					}
					else
					{
						CLIPRINTF((out, "\tnone ---> none\n"));
					}
				}
			} 
			else	
			{
				CLIPRINTF((out, "\tno fce translations\n"));
			}
		}

		if (callHandle2)
		{
			if (callHandle1)
			{
				CLIPRINTF((out, "\t%s/%lu ip:%s ani:%s ---> %s/%lu ip:%s dnis:%s dur:%lds\n",
					callHandle1->rfphonode.regid,
					callHandle1->rfphonode.uport, 
					ULIPtostring(callHandle1->rfphonode.ipaddress.l), 
					callHandle1->rfphonode.phone,
					callHandle1->phonode.regid,
					callHandle1->phonode.uport, 
					ULIPtostring(callHandle1->phonode.ipaddress.l),
					callHandle1->inputANI,
					now-timedef_sec(&callHandle1->callStartTime)));
			}

			if (CallFceBundleId(callHandle2))
			{
				if (callHandle1->handleType == SCC_eH323CallHandle)
				{
					if (H323nremoteset(callHandle2))
					{
						CLIPRINTF((out, "\t(leg2) %s:%d ---> %s:%d\n",
							FormatIpAddress(CallFceTranslatedIp(callHandle2), str1),
							CallFceTranslatedPort(callHandle2),
							FormatIpAddress(H323localSet(callHandle2)[0].rtpaddr, str2),
							H323localSet(callHandle2)[0].rtpport));
					}
					else
					{
						CLIPRINTF((out, "\tnone ---> none\n"));
					}
				}
				else if (callHandle1->handleType == SCC_eSipCallHandle)
				{
					sipCallHandle = SipCallHandle(callHandle2);
					if (sipCallHandle->localSet.localSet_len)
					{
						CLIPRINTF((out, "\t(leg1) %s:%d ---> %s:%d\n",
							FormatIpAddress(CallFceTranslatedIp(callHandle2), str1),
							CallFceTranslatedPort(callHandle2),
							FormatIpAddress(sipCallHandle->localSet.localSet_val[0].rtpaddr, str2),
							sipCallHandle->localSet.localSet_val[0].rtpport));
					}
					else
					{
						CLIPRINTF((out, "\tnone ---> none\n"));
					}
				}
			} 
			else	
			{
				CLIPRINTF((out, "\tno fce translations\n"));
			}
		}

		callHandle1?m++:m;
		callHandle2?m++:m;

		CLIPRINTF((out, "\n"));
		memcpy(cfidNext, confHandle->confID, CONF_ID_LEN);

		// Release the locks again to make sure
		// someone else can get a chance
		CacheReleaseLocks(callCache);
		CacheReleaseLocks(confCache);

		CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
		CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);
	}
			
	CacheReleaseLocks(callCache);
	CacheReleaseLocks(confCache);

	CLIPRINTF((out, "%d Calls, %d Legs\n", n, m));

	return xleOk; 
}
