#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "bits.h"
#include "ipc.h"
#include "ipcerror.h"
#include "serverdb.h"
#include "key.h"
#include "mem.h"
#include "entry.h"
#include "phone.h"
/* Server prototypes */

#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "pef.h"
#include "age.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"

#include "xmlparse.h"
#include "xmltags.h"
#include "log.h"

XMLTagTypes tags[] = 
{
	{ "ADV",	TAG_ADV,		"TAG_ADV" },
	{ "AX",		TAG_ACTIVE,		"TAG_ACTIVE" },
	{ "AZ",		TAG_AGED,		"TAG_AGED" },
	{ "CANS",	TAG_CALLANSWER,		"TAG_CALLANSWER" },	
	{ "CDR",	TAG_CDR,		"TAG_CDR" },
	{ "CE", 	TAG_CACHEENTRY, 	"TAG_CACHEENTRY" },
	{ "CM", 	TAG_COMMENTS, 		"TAG_COMMENTS" },
	{ "CP",		TAG_CP,			"TAG_CP" },
	{ "CPNAME",	TAG_CPNAME,		"TAG_CPNAME" },
	{ "CR",		TAG_CR,			"TAG_CR" },
	{ "CRDEST",	TAG_CRDEST,		"TAG_CRDEST" },
	{ "CRPREFIX",	TAG_CRPREFIX,		"TAG_CRPREFIX" },
	{ "CU", 	TAG_CACHEUPDATE, 	"TAG_CACHEUPDATE" },
	{ "CN",		TAG_COUNTRY, 		"TAG_COUNTRY" },
	{ "DE",		TAG_DBENTRY, 		"TAG_DBENTRY" },
	{ "DD",		TAG_DELETE,		"TAG_DELETE" },
	{ "DU",		TAG_DBUPDATE, 		"TAG_DBUPDATE" },
	{ "DND",	TAG_DND, 		"TAG_DND" },
	{ "DSYN",	TAG_DSYN, 		"TAG_DSYN" },
	{ "EM",		TAG_EMAIL, 		"TAG_EMAIL" },
	{ "ET",		TAG_ENDPTTYPE, 		"TAG_ENDPTTYPE" },
	{ "FN",		TAG_FNAME, 		"TAG_FNAME" },
	{ "FW",		TAG_FWDINFO, 		"TAG_FWDINFO" },
	{ "FP",		TAG_FWDPROTO, 		"TAG_FWDPROTO" },
	{ "GW",		TAG_GATEWAY,		"TAG_GATEWAY" },
	{ "GISP",	TAG_GISPREFIX, 		"TAG_GISPREFIX" },
	{ "H323SP",	TAG_H323SIGPT, 		"TAG_H323SIGPT" },
	{ "HRT",	TAG_HRTBT,		"TAG_HRTBT" },
	{ "IE",		TAG_IEDGE, 		"TAG_IEDGE" },
	{ "IP", 	TAG_IPADDRESS, 		"TAG_IPADDRESS" },
	{ "IT",		TAG_ITIME, 		"TAG_ITIME" },
	{ "LC",		TAG_LOCATION, 		"TAG_LOCATION" },
	{ "LCL",	TAG_LOCALNODE,		"TAG_LOCALNODE" },
	{ "LN",		TAG_LNAME, 		"TAG_LNAME" },
	{ "LUS",	TAG_LUS,		"TAG_LUS" },
	{ "MSG",	TAG_MESSAGE, 		"TAG_MESSAGE" },
	{ "MT", 	TAG_MTIME,		"TAG_MTIME" },
	{ "NEXTXML",	TAG_NEXTXML, 		"TAG_NEXTXML" },
	{ "PH", 	TAG_PHONE, 		"TAG_PHONE" },
	{ "PXC",	TAG_PROXYC, 		"TAG_PROXYC" },
	{ "PXI",	TAG_PROXYINFO, 		"TAG_PROXYINFO" },
	{ "PXS",	TAG_PROXYS, 		"TAG_PROXYS" },
	{ "QY",		TAG_QUERY,		"TAG_QUERY" },
	{ "RASIP",	TAG_RASIP,		"TAG_RASIP" },
	{ "RASPT",	TAG_RASPT,		"TAG_RASPT" },
	{ "REG", 	TAG_REG,		"TAG_REG" },
	{ "RT",		TAG_RTIME,		"TAG_RTIME" },
	{ "SN", 	TAG_REGID, 		"TAG_REGID" },
	{ "SYN",	TAG_SYN, 		"TAG_SYN" },
	{ "UP", 	TAG_UPORT, 		"TAG_UPORT" },
	{ "VPH", 	TAG_VPNPHONE, 		"TAG_VPNPHONE" },
	{ "VPN",	TAG_VPN,		"TAG_VPN" },
	{ "VPNG",	TAG_VPNG,		"TAG_VPNG" },
	{ "VPNS",	TAG_VPNS,		"TAG_VPNS" },
	{ "VPX", 	TAG_VPNEXTLEN, 		"TAG_VPNEXTLEN" },
	{ "ZN",         TAG_ZONE,               "TAG_ZONE" },
	{ "VENDOR",	TAG_VENDOR,		"TAG_VENDOR" },

	{ "*N*O*N*E*", 	TAG_NONE, "TAG_NONE" },	/* Keep In End */
};

XML_Parser parser;
XMLCacheCb cacheCb;
char s1[30];
long offsetSecs = 0;

int
GetTagFieldType(const char *name)
{
	int i;

	for (i=0; tags[i].type != TAG_NONE; i++)
	{
		if (!strcmp(tags[i].name, name))
		{
			return tags[i].type;
		}
	}

	return TAG_NONE;
}

/* Do Tag based initialization here */
void 
startElement(void *userData, const char *name, const char **atts)
{
  	XMLCacheCb *cb = (XMLCacheCb *)userData;
	
	if (cb->inFieldType[cb->depth] != 0)
	{
		NETERROR(MXML, ("XML Error\n"));
		return;
	}
	
	/* Else parse the name and assign a type */
	cb->inFieldType[cb->depth] = GetTagFieldType(name);

	/* Mark history */
	BITA_SET(cb->tagh, cb->inFieldType[cb->depth]);

	if (cb->startElem(cb, cb->inFieldType[cb->depth], 
		cb->depth, atts) != 0)
	{
		goto _return;
	}

	/* Else we must do default handling for the app */

	switch (cb->inFieldType[cb->depth])
	{
	case TAG_REGID:
		break;
	case TAG_UPORT:
		cb->infoEntry.uport = (-1);
		break;
	case TAG_IPADDRESS:
		cb->infoEntry.ipaddress.l = 0;
		break;
	case TAG_PHONE:
		break;
	case TAG_VPNPHONE:
		break;
	case TAG_DND:
		break;
	case TAG_ACTIVE:
		break;
	case TAG_ENDPTTYPE:
		break;
	case TAG_FWDINFO:
		cb->infoEntry.protocol = (NEXTONE_REDIRECT_FORWARD);
		break;
	case TAG_FWDPROTO:
		break;
	case TAG_ITIME:
		break;
	case TAG_PROXYS:
		break;
	case TAG_PROXYC:
		break;
	case TAG_VPNEXTLEN:
		break;
	case TAG_IEDGE:
		memset(&cb->infoEntry, 0, sizeof(InfoEntry));
		break;
	case TAG_VPN:
		memset(&cb->vpnEntry, 0, sizeof(VpnEntry));
		break;
	case TAG_VPNG:
		memset(&cb->vpnGroupEntry, 0, sizeof(VpnGroupEntry));
		break;
	case TAG_CP:
	    memset(&cb->cpEntry, 0, sizeof(CallPlanEntry));
		break;
	case TAG_CR:
	    memset(&cb->routeEntry, 0, sizeof(VpnRouteEntry));
		break;
	case TAG_HRTBT:
		cb->pktType = PKT_HEARTBEAT;
		break;
	case TAG_REG:
		cb->pktType = PKT_REGISTER;
		break;
	case TAG_LUS:
	case TAG_VPNS:
	case TAG_LOCALNODE:
	case TAG_NONE:
		break;
	default:
		break;
	}

_return:
	cb->depth ++;
}

void 
endElement(void *userData, const char *name)
{
  	XMLCacheCb *cb = (XMLCacheCb *)userData;

	cb->depth --;

	cb->endElem(cb, cb->inFieldType[cb->depth], 
		cb->depth);

	cb->inFieldType[cb->depth] = 0;	
}

void 
dataHandler(void *userData, const XML_Char *s, int len)
{
  	XMLCacheCb *cb = (XMLCacheCb *)userData;
	char tmpStr[128] = { 0 };

#ifdef needed
	if (IsModuleLevelOn(MRED, NETLOG_DEBUG4))
	{ 
		char s1[256];
		strncpy(s1, s, len); s1[len] = '\0';
		NETDEBUG(MRED, NETLOG_DEBUG4, ("data %s", s1));
	}
#endif

	{
		char s1[256];
		strncpy(s1, s, len); s1[len] = '\0';
		NETDEBUG(MXML, NETLOG_DEBUG4, ("dataHandler: len %d\n", len));
		NETDEBUG(MXML, NETLOG_DEBUG4, ("dataHandler: data == %s\n", s1));
	}
	
	/* Just copy the string for everybody's use */
	strncpy(tmpStr, s, len);
	tmpStr[len] = '\0'; /* MUST */
	
	switch (cb->inFieldType[cb->depth-1])
	{
	case TAG_REGID:
		strncpy(cb->infoEntry.regid, s, len);
		cb->infoEntry.regid[len] = '\0';
		BIT_SET(cb->infoEntry.sflags, ISSET_REGID);
		break;
	case TAG_UPORT:
		cb->infoEntry.uport = (atoi(tmpStr));
		BIT_SET(cb->infoEntry.sflags, ISSET_UPORT);

		if (!BITA_TEST(cb->infoEntry.ports, cb->infoEntry.uport))
		{
			 /* Increment the noprts by 1 */
			 cb->infoEntry.nports ++;
			 BITA_SET(cb->infoEntry.ports, cb->infoEntry.uport);
		}

		break;
	case TAG_IPADDRESS:
		if (cb->inFieldType[cb->depth-2] == TAG_LUS)
		{
			cb->aloidIpAddress = ntohl(inet_addr(tmpStr));
		}
		else if (cb->inFieldType[cb->depth-2] == TAG_VPNS)
		{
			cb->vpnsIpAddress = ntohl(inet_addr(tmpStr));
		}
		else
		{
			cb->infoEntry.ipaddress.l = ntohl(inet_addr(tmpStr));
			BIT_SET(cb->infoEntry.sflags, ISSET_IPADDRESS);
		}
		break;
	case TAG_PHONE:
		if (cb->inFieldType[cb->depth-2] == TAG_FWDINFO)
		{
			strncpy(cb->infoEntry.nphone, s, len);
			cb->infoEntry.nphone[len] = '\0';
			BIT_SET(cb->infoEntry.nsflags, ISSET_PHONE);
			cb->infoEntry.stateFlags |= CL_FORWARD;
		} else if (cb->inFieldType[cb->depth-2] == TAG_PROXYINFO)
		{
#if 0
			strncpy(cb->infoEntry.xphone, s, len);
			cb->infoEntry.xphone[len] = '\0';
#endif
		}
		else
		{
			strncpy(cb->infoEntry.phone, s, len);
			cb->infoEntry.phone[len] = '\0';
			BIT_SET(cb->infoEntry.sflags, ISSET_PHONE);
		}
		break;
	case TAG_VPNPHONE:
		if (cb->inFieldType[cb->depth-2] == TAG_FWDINFO)
		{
			strncpy(cb->infoEntry.nvpnPhone, s, len);
			cb->infoEntry.nvpnPhone[len] = '\0';
			BIT_SET(cb->infoEntry.nsflags, ISSET_VPNPHONE);
			cb->infoEntry.stateFlags |= CL_FORWARD;
		}
		else if (cb->inFieldType[cb->depth-2] == TAG_PROXYINFO)
		{
#if 0
			strncpy(cb->infoEntry.xvpnPhone, s, len);
			cb->infoEntry.xvpnPhone[len] = '\0';
#endif
		}
		else
		{
			strncpy(cb->infoEntry.vpnPhone, s, len);
			cb->infoEntry.vpnPhone[len] = '\0';
			BIT_SET(cb->infoEntry.sflags, ISSET_VPNPHONE);
		}
		break;
	case TAG_EMAIL:
		strncpy(cb->infoEntry.email, s, len);
		cb->infoEntry.email[len] = '\0';
		BIT_SET(cb->infoEntry.sflags, ISSET_EMAIL);
		break;
	case TAG_ZONE:
		strncpy(cb->infoEntry.zone, s, len);
		cb->infoEntry.zone[len] = '\0';
		break;
#if 0
	case TAG_FNAME:
		strncpy(cb->infoEntry.clFname, s, len);
		cb->infoEntry.clFname[len] = '\0';
		break;
	case TAG_LNAME:
		strncpy(cb->infoEntry.clLname, s, len);
		cb->infoEntry.clLname[len] = '\0';
		break;
	case TAG_COUNTRY:
		strncpy(cb->infoEntry.clCountry, s, len);
		cb->infoEntry.clCountry[len] = '\0';
		break;
	case TAG_LOCATION:
		strncpy(cb->infoEntry.clLoc, s, len);
		cb->infoEntry.clLoc[len] = '\0';
		break;
	case TAG_COMMENTS:
		strncpy(cb->infoEntry.clComments, s, len);
		cb->infoEntry.clComments[len] = '\0';
		break;
	case TAG_GISPREFIX:
		strncpy(cb->infoEntry.gisprefix, s, len);
		cb->infoEntry.gisprefix[len] = '\0';
		break;
#endif
	case TAG_VPNEXTLEN:
		if (cb->inFieldType[cb->depth-2] == TAG_FWDINFO)
		{
			cb->infoEntry.nvpnExtLen = (atoi(tmpStr));
			BIT_SET(cb->infoEntry.nsflags, ISSET_VPNPHONE);
		}
		else
		{
			cb->infoEntry.vpnExtLen = (atoi(tmpStr));
			BIT_SET(cb->infoEntry.sflags, ISSET_VPNPHONE);
		}
	
		/* If this is for a vpn entry */
		/* Mark the ext length of the vpn entry */
		cb->vpnEntry.vpnExtLen = (atoi(tmpStr));

		break;
	case TAG_DND:
		if (strncmp(tmpStr, "ON", len) == 0)
		{
			cb->infoEntry.stateFlags |= CL_DND;
		}
		break;
	case TAG_ENDPTTYPE:
		{ 
			int type = FindIedgeType(tmpStr);

			if (type >= 0)
			{
				cb->infoEntry.ispcorgw = type;
			}
			else if (!strncmp(s, "PC", len))	
			{
				cb->infoEntry.ispcorgw = 1;
			}
			else if (!strncmp(s, "GW", len))
			{
				cb->infoEntry.ispcorgw = 2;
			}
		}
		break;
	case TAG_FWDINFO:
	case TAG_FWDPROTO:
		{
			if (!strcmp(tmpStr, "ROLLOVER"))
			{
				cb->infoEntry.protocol = 
					NEXTONE_REDIRECT_ROLLOVER;
			}
		}
	case TAG_ITIME:
		cb->infoEntry.iTime = (atoi(tmpStr)-offsetSecs);
		break;
	case TAG_RTIME:
		cb->infoEntry.rTime = (atoi(tmpStr)-offsetSecs);
		break;
	case TAG_MTIME:
		cb->infoEntry.mTime = (atoi(tmpStr)-offsetSecs);
		cb->vpnEntry.mTime = (atoi(tmpStr)-offsetSecs);
		cb->vpnGroupEntry.mTime = (atoi(tmpStr)-offsetSecs);
		cb->cpEntry.mTime = (atoi(tmpStr)-offsetSecs);
		cb->routeEntry.mTime = (atoi(tmpStr)-offsetSecs);

		break;
	case TAG_PROXYS:
		if (!strncmp(tmpStr, "ON", len))
		{
			cb->infoEntry.stateFlags |= CL_PROXY;
		}
		break;
	case TAG_PROXYC:
		if (!strncmp(tmpStr, "ON", len))
		{
			cb->infoEntry.stateFlags |= CL_PROXYING;
		}
		break;
	case TAG_VPN:
		strncpy(cb->vpnEntry.vpnId, s, len);
		cb->vpnEntry.vpnId[len] = '\0';
		break;
	case TAG_VPNG:
		/* If its a vpnEntry */
		strncpy(cb->vpnEntry.vpnGroup, s, len);
		cb->vpnEntry.vpnGroup[len] = '\0';

		/* If its a vpnGroup */
		strncpy(cb->vpnGroupEntry.vpnGroup, s, len);
		cb->vpnGroupEntry.vpnGroup[len] = '\0';

		/* If its a calling plan */
		strncpy(cb->cpEntry.vpnGroup, s, len);
		cb->vpnGroupEntry.vpnGroup[len] = '\0';

		break;
	case TAG_CP:
		strncpy(cb->cpEntry.cpname, s, len);
		cb->cpEntry.cpname[len] = '\0';
		break;
	case TAG_CR:
		strncpy(cb->routeEntry.crname, s, len);
		cb->routeEntry.crname[len] = '\0';
		break;
	case TAG_CRDEST:
		strncpy(cb->routeEntry.dest, s, len);
		cb->routeEntry.dest[len] = '\0';
		break;
	case TAG_CRPREFIX:
		strncpy(cb->routeEntry.prefix, s, len);
		cb->routeEntry.prefix[len] = '\0';
		break;
	case TAG_ACTIVE:
		if (!strncmp(tmpStr, "ON", len))
		{
			cb->infoEntry.stateFlags |= CL_ACTIVE;
		}
		break;
	case TAG_GATEWAY:
		if (!strncmp(tmpStr, "ON", len))
		{
			BIT_SET(cb->infoEntry.cap, CAP_IGATEWAY);
		}
		else if (!strncmp(tmpStr, "OFF", len))
		{
			BIT_RESET(cb->infoEntry.cap, CAP_IGATEWAY);
		}
		break;
	case TAG_CPNAME:
	  strcpy(cb->infoEntry.cpname, tmpStr);
	  strcpy(cb->cpEntry.cpname, tmpStr);
	  /* strcpy(cb->cpbEntry.cpname, tmpStr); */
	  break;
	case TAG_RASIP:
		cb->infoEntry.rasip = ntohl(inet_addr(tmpStr));
		break;
	case TAG_RASPT:
		cb->infoEntry.rasport = atoi(tmpStr);
		break;
	case TAG_H323SIGPT:
		cb->infoEntry.callsigport = atoi(tmpStr);
		break;
	case TAG_IEDGE:
	case TAG_NONE:
		break;
	case TAG_VENDOR:
		cb->infoEntry.vendor = atoi(tmpStr);
		break;
	default:
		break;
	}
}

void 
startCdata(void *userData)
{
	printf("start cdata\n");
}

void 
endCdata(void *userData)
{
	printf("end cdata\n");
}

int
InitXMLDecoder(XMLStartElemHandler startElem, XMLEndElemHandler endElem)
{
	char buf[32];
	int len;

	parser = XML_ParserCreate(NULL);

	memset(&cacheCb, 0, sizeof(XMLCacheCb));
	cacheCb.startElem = startElem;
	cacheCb.endElem = endElem;
	
	XML_SetUserData(parser, &cacheCb);
	XML_SetElementHandler(parser, startElement, endElement);
	/* XML_SetCdataSectionHandler(parser, startCdata, endCdata); */
	XML_SetCharacterDataHandler(parser, dataHandler);

	len = sprintf(buf, "<NEXTXML>");
	if (!XML_Parse(parser, buf, len, 0))
	{
		NETERROR(MXML, ("XML Initialization Error"));
	}
	return(0);
}

int
DestroyXMLDecoder()
{
	XML_ParserFree(parser);
	return(0);
}

int
ProcessXMLEncoded (int sockfd, Pkt* data_pkt, Pkt * reply_pkt,
                 void *opaque, int opaquelen, /* Arguments to be passed
                                               * to the write call back.
                                               */
                 int (*writecb)())
{
	XMLStartElemHandler oldStartElem;
	XMLEndElemHandler oldEndElem;
	char *buf;
	int len;

	/* Store the exact packet, as it is. This cannot be used
	 * outside context, as data_pkt is an automatic variable
	 */
	oldStartElem = cacheCb.startElem;
	oldEndElem = cacheCb.endElem;

	memset(&cacheCb, 0, sizeof(XMLCacheCb));

	/* Copy the handlers back */
	cacheCb.startElem = oldStartElem;
	cacheCb.endElem = oldEndElem;

	cacheCb.buf = (char *)data_pkt;
	cacheCb.buflen = data_pkt->totalLen;

	/* The XML parser should already be initialized here */
	buf = (char *)data_pkt + sizeof(GPktHeader);
	len = (data_pkt->totalLen) - sizeof(GPktHeader);	

	DEBUG_PrintBuffer(MXML, NETLOG_DEBUG4, buf, len);

	if (!XML_Parse(parser, buf, len, 0))
	{
		NETERROR(MXML, ("XML Error Encountered: \n"));
		DEBUG_PrintBuffer(MXML, NETLOG_DEBUG4, buf, len);

		/* Re-initialize the xml encoder */
		NETERROR(MXML, ("Re-Initializing XML\n"));
		DestroyXMLDecoder();
		InitXMLDecoder(cacheCb.startElem, cacheCb.endElem);
	}	
	return(0);
}

/* return -1, if an error occurred,
 * or actual length encoded.
 */
int
XMLEncodeInfoEntry(InfoEntry *infoEntry, char *buf, int buflen, unsigned char *tags)
{
	int len = 0;
	char *ptr = buf;
	int all = (tags == NULL);
	int pxs = 0, pxc = 0;

#define CHECKLEN(x) if ((buf-ptr) >  buflen) return -999999

	buf += sprintf(buf, "<IE>");
	CHECKLEN(len);

	if ((all || BITA_TEST(tags, TAG_REGID)) &&
		BIT_TEST(infoEntry->sflags, ISSET_REGID))
	{
		buf += sprintf(buf, "<SN><![CDATA[%s]]></SN>",
			infoEntry->regid);
		CHECKLEN(len);
	}
	
	if ((all || BITA_TEST(tags, TAG_UPORT)) &&
		BIT_TEST(infoEntry->sflags, ISSET_UPORT))
	{
		buf += sprintf(buf, "<UP>%lu</UP>",
			(infoEntry->uport));
		CHECKLEN(len);
	}

	if (tags && BITA_TEST(tags, TAG_DELETE))
	{
		buf += sprintf(buf, "<DD></DD>");
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_ITIME))
	{
		buf += sprintf(buf, "<IT>%lu</IT>",
			(infoEntry->iTime));
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_RTIME))
	{
		buf += sprintf(buf, "<RT>%lu</RT>",
			(infoEntry->rTime));
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_MTIME))
	{
		buf += sprintf(buf, "<MT>%lu</MT>",
			(infoEntry->mTime));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_PHONE)) &&
		BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
	{
		buf += sprintf(buf, "<PH><![CDATA[%s]]></PH>",
		  	infoEntry->phone);
		CHECKLEN(len);
	}
     
	if ((all || BITA_TEST(tags, TAG_VPNPHONE)) &&
		BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_VPNPHONE))
	{
		buf += sprintf(buf, "<VPH><![CDATA[%s]]></VPH>",
			infoEntry->vpnPhone);
		CHECKLEN(len);
		buf += sprintf(buf, "<VPX>%lu</VPX>",
			(infoEntry->vpnExtLen));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_IPADDRESS)) &&
		BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_IPADDRESS))
	{
		struct in_addr in;
                char str[INET_ADDRSTRLEN];

		in.s_addr = htonl(infoEntry->ipaddress.l);
		buf += sprintf(buf, "<IP><![CDATA[%s]]></IP>", inet_ntop( AF_INET, &in, str, INET_ADDRSTRLEN));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_EMAIL)) &&
		BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_EMAIL))
	{
		buf += sprintf(buf, "<EM><![CDATA[%s]]></EM>",
			infoEntry->email);
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_ZONE))
	{
		buf += sprintf(buf, "<ZN><![CDATA[%s]]></ZN>",
			infoEntry->zone);
		CHECKLEN(len);
	}

#if 0
	if (all || BITA_TEST(tags, TAG_FNAME))
	{
		buf += sprintf(buf, "<FN><![CDATA[%s]]></FN>",
			infoEntry->clFname);
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_LNAME))
	{
		buf += sprintf(buf, "<LN><![CDATA[%s]]></LN>",
			infoEntry->clLname);
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_COUNTRY))
	{
		buf += sprintf(buf, "<CN><![CDATA[%s]]></CN>",
			infoEntry->clCountry);
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_LOCATION))
	{
		buf += sprintf(buf, "<LC><![CDATA[%s]]></LC>",
			infoEntry->clLoc);
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_COMMENTS))
	{
		buf += sprintf(buf, "<CM><![CDATA[%s]]></CM>",
			infoEntry->clComments);
		CHECKLEN(len);
	}
#endif

	if ((all || BITA_TEST(tags, TAG_PROXYS)) &&
		(infoEntry->stateFlags & CL_PROXY))
	{
		buf += sprintf(buf, "<PXS>ON</PXS>");
		CHECKLEN(len);
		pxs = 1;
	}

	if (( all || BITA_TEST(tags, TAG_PROXYC)) &&
		(infoEntry->stateFlags & CL_PROXYING) )
	{
		 buf += sprintf(buf, "<PXC>ON</PXC>");
		 CHECKLEN(len);
		 pxc = 1;
	}

	/* If either the PROXYS or PROXYC is set we must send the
	 * proxy info, if there exists any
	 */	
#if 0
	if ((pxs || pxc) &&
		(strlen(infoEntry->xphone) || strlen(infoEntry->xvpnPhone)) )
	{
		buf += sprintf(buf, "<PXI>");
		CHECKLEN(len);

		if (strlen(infoEntry->xphone))
		{
			buf += sprintf(buf, "<PH><![CDATA[%s]]></PH>", 
				infoEntry->xphone);
			CHECKLEN(len);
		}
		if (strlen(infoEntry->xvpnPhone))
		{
			buf += sprintf(buf, "<VPH><![CDATA[%s]]></VPH>", 
				infoEntry->xvpnPhone);
			CHECKLEN(len);
		}

		buf += sprintf(buf, "</PXI>");
		CHECKLEN(len);
	}

#endif
	if ((all || BITA_TEST(tags, TAG_FWDINFO)) &&
		(infoEntry->stateFlags & CL_FORWARD))
	{
		buf += sprintf(buf, "<FW>");
		if ((infoEntry->protocol) == NEXTONE_REDIRECT_ROLLOVER)
		{
			buf += sprintf(buf, "<FP>ROLLOVER</FP>");
			CHECKLEN(len);
		}
	  	if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE))
	  	{
			buf += sprintf(buf, "<PH><![CDATA[%s]]></PH>",
		  		infoEntry->nphone);
			CHECKLEN(len);
	  	}
	  	if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE))
	  	{
			buf += sprintf(buf, "<VPH><![CDATA[%s]]></VPH>",
				infoEntry->nvpnPhone);
			CHECKLEN(len);
			buf += sprintf(buf, "<VPX>%lu</VPX>",
				(infoEntry->nvpnExtLen));
			CHECKLEN(len);
	  	}
		buf += sprintf(buf, "</FW>");
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_DND)) &&
		(infoEntry->stateFlags & CL_DND))
	{
		buf += sprintf(buf, "<DND>ON</DND>");
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_ACTIVE)) &&
		(infoEntry->stateFlags & CL_ACTIVE))
	{
		buf += sprintf(buf, "<AX>ON</AX>");
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_AGED)) &&
		!(infoEntry->stateFlags & CL_ACTIVE))
	{
		buf += sprintf(buf, "<AZ/>");
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_ENDPTTYPE)))
	{
		buf += sprintf(buf, "<ET>%s</ET>",
			IedgeName(infoEntry->ispcorgw));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_GATEWAY)))
	{
		buf += sprintf(buf, "<GW>%s</GW>", 
				BIT_TEST(infoEntry->cap, CAP_IGATEWAY)?"ON":"OFF");
		CHECKLEN(len);
	}

	/* Add Calling plan */
	if ((all || BITA_TEST(tags, TAG_CPNAME)))
	{
		buf += sprintf(buf, "<CPNAME>%s</CPNAME>",
					   infoEntry->cpname);
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_RASIP)) &&
		(infoEntry->rasip != 0))
	{
		struct in_addr in;
                char str[INET_ADDRSTRLEN];

		in.s_addr = htonl(infoEntry->rasip);
		buf += sprintf(buf, "<RASIP><![CDATA[%s]]></RASIP>", inet_ntop( AF_INET, &in, str, INET_ADDRSTRLEN));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_RASPT)) &&
		(infoEntry->rasport != 0))
	{
		buf += sprintf(buf, "<RASPT>%d</RASPT>",
			(infoEntry->rasport));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_H323SIGPT)) &&
		(infoEntry->rasport != 0))
	{
		buf += sprintf(buf, "<H323SP>%d</H323SP>",
			(infoEntry->callsigport));
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_VENDOR))
	{
		buf += sprintf(buf, "<VENDOR>%d</VENDOR>",
			infoEntry->vendor);
		CHECKLEN(len);
	}

	buf += sprintf(buf, "</IE>");
	CHECKLEN(len);

	return (buf-ptr);
}

int
XMLEncodeInfoEntryUpdate(InfoEntry *infoEntry, char *buf, int buflen, unsigned char *tags)
{
	int len = 0;
	char *ptr = buf;
	int all = (tags == NULL);
	int pxs = 0, pxc = 0;

#define CHECKLEN(x) if ((buf-ptr) >  buflen) return -999999

	buf += sprintf(buf, "<IE>");
	CHECKLEN(len);

	if ((all || BITA_TEST(tags, TAG_REGID)) &&
		BIT_TEST(infoEntry->sflags, ISSET_REGID))
	{
		buf += sprintf(buf, "<SN><![CDATA[%s]]></SN>",
			infoEntry->regid);
		CHECKLEN(len);
	}
	
     	if ((all || BITA_TEST(tags, TAG_UPORT)) &&
		BIT_TEST(infoEntry->sflags, ISSET_UPORT))
     	{
		buf += sprintf(buf, "<UP>%lu</UP>",
			(infoEntry->uport));
		CHECKLEN(len);
     	}

	if (tags && BITA_TEST(tags, TAG_DELETE))
	{
		buf += sprintf(buf, "<DD></DD>");
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_ITIME))
	{
		buf += sprintf(buf, "<IT>%lu</IT>",
			(infoEntry->iTime));
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_RTIME))
	{
		buf += sprintf(buf, "<RT>%lu</RT>",
			(infoEntry->rTime));
		CHECKLEN(len);
	}

	if (all || BITA_TEST(tags, TAG_MTIME))
	{
		buf += sprintf(buf, "<MT>%lu</MT>",
			(infoEntry->mTime));
		CHECKLEN(len);
	}

     	if (all || BITA_TEST(tags, TAG_PHONE))
	{
		if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
		{
			buf += sprintf(buf, "<PH><![CDATA[%s]]></PH>",
		  		infoEntry->phone);
		}
		else
		{
			buf += sprintf(buf, "<PH/>");
		}

		CHECKLEN(len);
	}
     
     	if (all || BITA_TEST(tags, TAG_VPNPHONE))
     	{
		if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_VPNPHONE))
		{
			buf += sprintf(buf, "<VPH><![CDATA[%s]]></VPH>",
				infoEntry->vpnPhone);
			CHECKLEN(len);
			buf += sprintf(buf, "<VPX>%lu</VPX>",
				(infoEntry->vpnExtLen));
			CHECKLEN(len);
		}
		else
		{
			buf += sprintf(buf, "<VPH/>");
			CHECKLEN(len);
		}
	}

     	if (all || BITA_TEST(tags, TAG_IPADDRESS))
     	{
		struct in_addr in;
                char str[INET_ADDRSTRLEN];

		if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_IPADDRESS))
		{
			in.s_addr = htonl(infoEntry->ipaddress.l);
			buf += sprintf(buf, "<IP><![CDATA[%s]]></IP>", inet_ntop( AF_INET, &in, str, INET_ADDRSTRLEN));
			CHECKLEN(len);
		}
		else
		{
			buf += sprintf(buf, "<IP/>");
			CHECKLEN(len);
		}
     	}

     	if ((all || BITA_TEST(tags, TAG_EMAIL)) &&
		BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_EMAIL))
     	{
		buf += sprintf(buf, "<EM><![CDATA[%s]]></EM>",
			infoEntry->email);
		CHECKLEN(len);
	}

     	if (all || BITA_TEST(tags, TAG_ZONE))
	{
		buf += sprintf(buf, "<ZN><![CDATA[%s]]></ZN>",
			infoEntry->zone);
		CHECKLEN(len);
	}

#if 0
     	if (all || BITA_TEST(tags, TAG_FNAME))
	{
		buf += sprintf(buf, "<FN><![CDATA[%s]]></FN>",
			infoEntry->clFname);
		CHECKLEN(len);
	}

     	if (all || BITA_TEST(tags, TAG_LNAME))
	{
		buf += sprintf(buf, "<LN><![CDATA[%s]]></LN>",
			infoEntry->clLname);
		CHECKLEN(len);
	}

     	if (all || BITA_TEST(tags, TAG_COUNTRY))
	{
		buf += sprintf(buf, "<CN><![CDATA[%s]]></CN>",
			infoEntry->clCountry);
		CHECKLEN(len);
	}

     	if (all || BITA_TEST(tags, TAG_LOCATION))
	{
		buf += sprintf(buf, "<LC><![CDATA[%s]]></LC>",
			infoEntry->clLoc);
		CHECKLEN(len);
	}

     	if (all || BITA_TEST(tags, TAG_COMMENTS))
	{
		buf += sprintf(buf, "<CM><![CDATA[%s]]></CM>",
			infoEntry->clComments);
		CHECKLEN(len);
	}

     	if (all || BITA_TEST(tags, TAG_GISPREFIX))
	{
		buf += sprintf(buf, "<GISP><![CDATA[%s]]></GISP>",
			infoEntry->gisprefix);
		CHECKLEN(len);
	}
#endif

     	if (all || BITA_TEST(tags, TAG_PROXYS)) 
     	{
		if (infoEntry->stateFlags & CL_PROXY)
		{
			buf += sprintf(buf, "<PXS>ON</PXS>");
			pxs = 1;
		}
		else
		{
			buf += sprintf(buf, "<PXS>OFF</PXS>");
		}
		CHECKLEN(len);
     	}

     	if ( all || BITA_TEST(tags, TAG_PROXYC))
     	{
		if (infoEntry->stateFlags & CL_PROXYING)
		{
			buf += sprintf(buf, "<PXC>ON</PXC>");
			pxc = 1;
		}
		else
		{
			buf += sprintf(buf, "<PXC>OFF</PXC>");
		}

		CHECKLEN(len);
     	}

	/* If either the PROXYS or PROXYC is set we must send the
	 * proxy info, if there exisst any
	 */	
#if 0
     	if ((pxs || pxc) &&
		(strlen(infoEntry->xphone) || strlen(infoEntry->xvpnPhone)) )
	{
		buf += sprintf(buf, "<PXI>");
		CHECKLEN(len);

		if (strlen(infoEntry->xphone))
		{
			buf += sprintf(buf, "<PH><![CDATA[%s]]></PH>", 
				infoEntry->xphone);
			CHECKLEN(len);
		}
		if (strlen(infoEntry->xvpnPhone))
		{
			buf += sprintf(buf, "<VPH><![CDATA[%s]]></VPH>", 
				infoEntry->xvpnPhone);
			CHECKLEN(len);
		}

		buf += sprintf(buf, "</PXI>");
		CHECKLEN(len);
	}
#endif

     	if (all || BITA_TEST(tags, TAG_FWDINFO))
     	{
		if (infoEntry->stateFlags & CL_FORWARD)
		{
			buf += sprintf(buf, "<FW>");
			if ((infoEntry->protocol) == NEXTONE_REDIRECT_ROLLOVER)
			{
				buf += sprintf(buf, "<FP>ROLLOVER</FP>");
				CHECKLEN(len);
			}
	  		if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE))
	  		{
				buf += sprintf(buf, "<PH><![CDATA[%s]]></PH>",
		  			infoEntry->nphone);
				CHECKLEN(len);
	  		}
	  		if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE))
	  		{
				buf += sprintf(buf, "<VPH><![CDATA[%s]]></VPH>",
					infoEntry->nvpnPhone);
				CHECKLEN(len);	
				buf += sprintf(buf, "<VPX>%lu</VPX>",
					(infoEntry->nvpnExtLen));
				CHECKLEN(len);
	  		}
			buf += sprintf(buf, "</FW>");
		}
		else
		{
			buf += sprintf(buf, "<FW/>");
		}

		CHECKLEN(len);
     	}

     	if (all || BITA_TEST(tags, TAG_DND))
     	{
		if (infoEntry->stateFlags & CL_DND)
		{
			buf += sprintf(buf, "<DND>ON</DND>");
		}
		else
		{
			buf += sprintf(buf, "<DND>OFF</DND>");
		}

		CHECKLEN(len);
     	}

     	if (all || BITA_TEST(tags, TAG_ACTIVE))
     	{
		if (infoEntry->stateFlags & CL_ACTIVE)
		{
			buf += sprintf(buf, "<AX>ON</AX>");
		}
		else
		{
			buf += sprintf(buf, "<AX>OFF</AX>");
		}

		CHECKLEN(len);
     	}

     	if (all || BITA_TEST(tags, TAG_AGED))
     	{
		if (!(infoEntry->stateFlags & CL_ACTIVE))
		{
			buf += sprintf(buf, "<AZ/>");
			CHECKLEN(len);
		}
     	}

	if (all || BITA_TEST(tags, TAG_ENDPTTYPE))
	{
		buf += sprintf(buf, "<ET>%s</ET>",
			IedgeName(infoEntry->ispcorgw));

		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_GATEWAY)))
	{
		buf += sprintf(buf, "<GW>%s</GW>",
				BIT_TEST(infoEntry->cap, CAP_IGATEWAY)?"ON":"OFF");
		CHECKLEN(len);
	}

	/* Add Calling plan */
	if ((all || BITA_TEST(tags, TAG_CPNAME)))
	{
		buf += sprintf(buf, "<CPNAME>%s</CPNAME>",
					   infoEntry->cpname);
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_RASIP)) &&
		(infoEntry->rasip != 0))
	{
		struct in_addr in;
                char str[INET_ADDRSTRLEN];

		in.s_addr = htonl(infoEntry->rasip);
		buf += sprintf(buf, "<RASIP><![CDATA[%s]]></RASIP>", inet_ntop( AF_INET, &in, str, INET_ADDRSTRLEN));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_RASPT)) &&
		(infoEntry->rasport != 0))
	{
		buf += sprintf(buf, "<RASPT>%d</RASPT>",
			(infoEntry->rasport));
		CHECKLEN(len);
	}

	if ((all || BITA_TEST(tags, TAG_H323SIGPT)) &&
		(infoEntry->rasport != 0))
	{
		buf += sprintf(buf, "<H323SP>%d</H323SP>",
			(infoEntry->callsigport));
		CHECKLEN(len);
	}

	buf += sprintf(buf, "</IE>");
	CHECKLEN(len);

	return (buf-ptr);
}

int
XMLEncodeVpnEntry(VpnEntry *vpnEntry, char *buf, int buflen, unsigned char *tags)
{
	int len = 0;
	char *ptr = buf;
	int all = (tags == NULL);

#define CHECKLEN(x) if ((buf-ptr) >  buflen) return -999999

	buf += sprintf(buf, "<VPN>");
	CHECKLEN(len);

	buf += sprintf(buf, "<![CDATA[%s]]>",
		vpnEntry->vpnId);
	CHECKLEN(len);

	if (all || BITA_TEST(tags, TAG_VPNEXTLEN))
	{
		buf += sprintf(buf, "<VPX><![CDATA[%d]]></VPX>",
			(vpnEntry->vpnExtLen));
		CHECKLEN(len);
	}
	
	if (all || BITA_TEST(tags, TAG_VPNG))
	{
		buf += sprintf(buf, "<VPNG><![CDATA[%s]]></VPNG>",
			vpnEntry->vpnGroup);
		CHECKLEN(len);
	}
	
	buf += sprintf(buf, "<MT>%lu</MT>",
		(vpnEntry->mTime));
	CHECKLEN(len);

	if (tags && BITA_TEST(tags, TAG_DELETE))
	{
		buf += sprintf(buf, "<DD></DD>");
		CHECKLEN(len);
	}

	buf += sprintf(buf, "</VPN>");
	CHECKLEN(len);

	return (buf-ptr);
}

int
XMLEncodeCPEntry(CallPlanEntry *entry, char *buf, int buflen, unsigned char *tags)
{
	int len = 0;
	char *ptr = buf;
	int all = (tags == NULL);

#define CHECKLEN(x) if ((buf-ptr) >  buflen) return -999999

	buf += sprintf(buf, "<CP>");
	CHECKLEN(len);

	buf += sprintf(buf, "<![CDATA[%s]]>",
		entry->cpname);
	CHECKLEN(len);

	if (all || BITA_TEST(tags, TAG_VPNG))
	{
		buf += sprintf(buf, "<VPNG><![CDATA[%s]]></VPNG>",
			entry->vpnGroup);
		CHECKLEN(len);
	}
	
	buf += sprintf(buf, "<MT>%lu</MT>",
		(entry->mTime));

	CHECKLEN(len);

	if (tags && BITA_TEST(tags, TAG_DELETE))
	{
		buf += sprintf(buf, "<DD></DD>");
		CHECKLEN(len);
	}

	buf += sprintf(buf, "</CP>");
	CHECKLEN(len);

	return (buf-ptr);
}

int
XMLEncodeCREntry(VpnRouteEntry *entry, char *buf, int buflen, unsigned char *tags)
{
	int len = 0;
	char *ptr = buf;
	int all = (tags == NULL);

#define CHECKLEN(x) if ((buf-ptr) >  buflen) return -999999

	buf += sprintf(buf, "<CR>");
	CHECKLEN(len);

	buf += sprintf(buf, "<![CDATA[%s]]>",
		entry->crname);
	CHECKLEN(len);

	if (all || BITA_TEST(tags, TAG_CRDEST))
	{
		buf += sprintf(buf, "<CRDEST><![CDATA[%s]]></CRDEST>",
			entry->dest);
		CHECKLEN(len);
	}
	
	if (all || BITA_TEST(tags, TAG_CRPREFIX))
	{
		buf += sprintf(buf, "<CRPREFIX><![CDATA[%s]]></CRPREFIX>",
			entry->prefix);
		CHECKLEN(len);
	}
	
	buf += sprintf(buf, "<MT>%lu</MT>",
		(entry->mTime));

	CHECKLEN(len);

	if (tags && BITA_TEST(tags, TAG_DELETE))
	{
		buf += sprintf(buf, "<DD></DD>");
		CHECKLEN(len);
	}

	buf += sprintf(buf, "</CR>");
	CHECKLEN(len);

	return (buf-ptr);
}

int
XMLEncodeVpnGEntry(VpnGroupEntry *vpnGroupEntry, char *buf, int buflen, unsigned char *tags)
{
	int len = 0;
	char *ptr = buf;
	int all = (tags == NULL);

#define CHECKLEN(x) if ((buf-ptr) >  buflen) return -999999

	buf += sprintf(buf, "<VPNG>");
	CHECKLEN(len);

	buf += sprintf(buf, "<![CDATA[%s]]>",
		vpnGroupEntry->vpnGroup);
	CHECKLEN(len);

	buf += sprintf(buf, "<MT>%lu</MT>",
		(vpnGroupEntry->mTime));
	CHECKLEN(len);

	if (tags && BITA_TEST(tags, TAG_DELETE))
	{
		buf += sprintf(buf, "<DD></DD>");
		CHECKLEN(len);
	}

	buf += sprintf(buf, "</VPNG>");
	CHECKLEN(len);

	return (buf-ptr);
}

int
TaghPrint(int module, int level, char *a, int len)
{
	int i;
	
	for (i=0; i<TAG_MAX; i++)
	{
		if (BITA_TEST(a, tags[i].type))
		{
			NETDEBUG(module, level, 
				("Bit %s is set\n", tags[i].cname));
		}
	}
	return(0);
}

int
SendXML(int fd, char *xmlEnc, int len)
{
	 char fn[] = "SendXML():";
	static GPktHeader hdr = { 0, 0 };
	struct iovec bufs[2];
	struct msghdr msg = {0};
	int rc = 1;

	if (len < 0)
	{
		 NETERROR(MRED, ("%s Length passed is invalid", fn));	
		 return -1;
	}

	hdr.type = htonl(PKT_XML);
	hdr.totalLen = htonl(sizeof(GPktHeader) + len);
	msg.msg_iov = bufs;
	msg.msg_iovlen = 2;
	
	bufs[0].iov_base = (char *)&hdr;
	bufs[0].iov_len = sizeof(GPktHeader);
	bufs[1].iov_base = xmlEnc;
	bufs[1].iov_len = len;

	if (sendmsg(fd, &msg, 0) < 0)
	{
		NETERROR(MRED, ("%s error %d, sending xml packet", fn, errno));
		rc = -1;
	}

	return rc;
}

int
AddEntry(int type, char *buf, int buflen, void *entry)
{
	NetoidInfoEntry *netInfo = (NetoidInfoEntry *)entry;
	VpnEntry *vpnEntry = (VpnEntry *)entry;
	VpnGroupEntry *vpnGroupEntry = (VpnGroupEntry *)entry;
	VpnRouteEntry *routeEntry = (VpnRouteEntry *)entry;
	CallPlanEntry *cpEntry = (CallPlanEntry *)entry;
	int len = 0;

	len += sprintf(buf+len, "<CE>");

	switch (type)
	{
	case TAG_IEDGE:
		len += XMLEncodeInfoEntry(netInfo, buf+len, buflen-len, NULL);
		break;
	case TAG_VPN:
		len += XMLEncodeVpnEntry(vpnEntry, buf+len, buflen-len, NULL);
		break;
	case TAG_VPNG:
		len += XMLEncodeVpnGEntry(vpnGroupEntry, buf+len, buflen-len, NULL);
		break;
	case TAG_CP:
		len += XMLEncodeCPEntry(cpEntry, buf+len, buflen-len, NULL);
		break;
	case TAG_CR:
		len += XMLEncodeCREntry(routeEntry, buf+len, buflen-len, NULL);
		break;
	default:
		break;
	}

	len += sprintf(buf+len, "</CE>");

	return len;
}

int
AddUpdate(int type, char *buf, int buflen, void *entry, unsigned char tags[])
{
	NetoidInfoEntry *netInfo = (NetoidInfoEntry *)entry;
	VpnEntry *vpnEntry = (VpnEntry *)entry;
	VpnGroupEntry *vpnGroupEntry = (VpnGroupEntry *)entry;
	VpnRouteEntry *routeEntry = (VpnRouteEntry *)entry;
	CallPlanEntry *cpEntry = (CallPlanEntry *)entry;
	int len = 0;

	len += sprintf(buf+len, "<CU>");

	switch (type)
	{
	case TAG_IEDGE:
		if (tags)
		{
			BITA_SET(tags, TAG_IEDGE);
			BITA_SET(tags, TAG_REGID);
			BITA_SET(tags, TAG_UPORT);
			BITA_SET(tags, TAG_PHONE);
			BITA_SET(tags, TAG_VPNPHONE);
		}

		len += XMLEncodeInfoEntryUpdate(netInfo, buf+len, buflen-len, tags);

		break;
	case TAG_VPN:
		if (tags)
		{
			BITA_SET(tags, TAG_VPN);
			BITA_SET(tags, TAG_VPNEXTLEN);
			BITA_SET(tags, TAG_VPNG);
		}

		len += XMLEncodeVpnEntry(vpnEntry, buf+len, buflen-len, tags);

		break;

	case TAG_VPNG:
		if (tags)
		{
			BITA_SET(tags, TAG_VPNG);
		}

		len += XMLEncodeVpnGEntry(vpnGroupEntry, buf+len, buflen-len, tags);
	
		break;

	case TAG_CP:
		if (tags)
		{
			BITA_SET(tags, TAG_CP);
		}

		len += XMLEncodeCPEntry(cpEntry, buf+len, buflen-len, tags);
	
		break;

	case TAG_CR:
		if (tags)
		{
			BITA_SET(tags, TAG_CR);
		}

		len += XMLEncodeCREntry(routeEntry, buf+len, buflen-len, tags);

		break;
	default:
		break;
	}

	len += sprintf(buf+len, "</CU>");

	return len;
}

