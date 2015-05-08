#include <unistd.h>
#include "cli.h"
#include "serverp.h"
#include "ifs.h"
#include "rs.h"
#include "lsconfig.h"
#include <malloc.h>
#include <sys/un.h>
#include "cacheinit.h"
#include "nxosd.h"
#include "log.h"
#include "ipstring.h"
#include "common.h"

#ifndef IN_CLASSD_NET
#define IN_CLASSD_NET 0xf0000000
#endif /* ifndef IN_CALSSDNET */


#define EXIT_CMD(_X_)	(!strcmp(_X_, "exit") || !strcmp(_X_, "x"))
#define HISTORY_CMD(_X_)	(!strcmp(_X_, "history") || !strcmp(_X_, "h"))

/*
 * Some  auxiliary functions
 */
int
StoreVpnInDb(Command *comm, VpnEntry *vpnEntry, int shmId)
{
	int rc = 0;
    char oldVpnGroup[VPN_GROUP_LEN], newVpnGroup[VPN_GROUP_LEN];

	if (strlen(vpnEntry->vpnName) == 0)
	{
		 return -1;
	}

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -1;
	}
	
	strcpy(newVpnGroup, vpnEntry->vpnGroup);

	rc = StoreVpn(GDBMF(comm->data, DB_eVpns), vpnEntry,
		oldVpnGroup, newVpnGroup);
	
	CloseDatabases((DefCommandData *)comm->data);

	/* Update the cache with this entry */
	
	if ((rc > 0) && (shmId > 0))
	{
     		MemAgeIedgesInVpnGs(oldVpnGroup, newVpnGroup);
	}

	return rc;
}

void
NetoidStoreRegPair(NetoidInfoEntry *netInfo, ClientAttribs *clAttribs,
					char *f, char *v)
{
	if (strcmp(f, "ip") == 0)
	{
		char ipaddrstr[20] = { 0 };

		/* We expect an ip address here */
		strncpy(ipaddrstr, v, 20);
		netInfo->ipaddress.l = inet_addr(ipaddrstr);
		if (!strcmp(v, "none") ||
			(netInfo->ipaddress.l == (unsigned long)-1) ||
			(netInfo->ipaddress.l == 0))
		{
			// not needed really - but a check against bad input
			BIT_RESET(netInfo->sflags, ISSET_IPADDRESS);
			netInfo->stateFlags &= ~(CL_ACTIVE|CL_REGISTERED);
		}
		else
		{
			netInfo->ipaddress.l = ntohl(netInfo->ipaddress.l);
			netInfo->rasip = netInfo->ipaddress.l;

#if 0 // use the reg enable / disable
			BIT_SET(netInfo->sflags, ISSET_IPADDRESS);
			netInfo->stateFlags |= CL_ACTIVE;
			netInfo->stateFlags |= CL_REGISTERED;
			time(&netInfo->rTime);	
#endif
		}
	}
	else if (strcmp(f, "gateway") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_IGATEWAY);
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_IGATEWAY);
		}
	}
	else if (strcmp(f, "ncalls") == 0)
	{
		IedgeCalls(netInfo) = atoi(v);
	}
	else if (strcmp(f, "nincalls") == 0)
	{
		IedgeInCalls(netInfo) = atoi(v);
	}
	else if (strcmp(f, "noutcalls") == 0)
	{
		IedgeOutCalls(netInfo) = atoi(v);
	}
	else if (strcmp(f, "reg") == 0)
	{
		if (!strcmp(v, "active"))
		{
			netInfo->stateFlags |= CL_ACTIVE;
			netInfo->stateFlags |= CL_REGISTERED;
			time(&netInfo->rTime);	
		}
		else
		{
			netInfo->stateFlags &= ~(CL_ACTIVE|CL_REGISTERED);
		}
	}
}

void
NetoidStorePair(NetoidInfoEntry *netInfo, ClientAttribs *clAttribs,
					char *f, char *v)
{
	char ch;
	if (clAttribs && strcmp(f, "fname") == 0)
	{
		strncpy(clAttribs->clFname, v, CLIENT_ATTR_LEN);
		clAttribs->clFname[CLIENT_ATTR_LEN-1] = '\0';
	}
	else if (clAttribs && strcmp(f, "lname") == 0)
	{
		strncpy(clAttribs->clLname, v, CLIENT_ATTR_LEN);
		clAttribs->clLname[CLIENT_ATTR_LEN-1] = '\0';
	}
	else if (clAttribs && strcmp(f, "country") == 0)
	{
		strncpy(clAttribs->clCountry, v, CLIENT_ATTR_LEN);
		clAttribs->clCountry[CLIENT_ATTR_LEN-1] = '\0';
	}
	else if (clAttribs && strcmp(f, "location") == 0)
	{
		strncpy(clAttribs->clLoc, v, CLIENT_ATTR_LEN);
		clAttribs->clLoc[CLIENT_ATTR_LEN-1] = '\0';
	}
	else if (clAttribs && strcmp(f, "comments") == 0)
	{
		strncpy(clAttribs->clComments, v, CLIENT_ATTR_LEN);
		clAttribs->clComments[CLIENT_ATTR_LEN-1] = '\0';
	}
	else if (strcmp(f, "uri") == 0)
	{
		strncpy(netInfo->uri, v, SIPURL_LEN);
		netInfo->uri[SIPURL_LEN-1] = '\0';
	}
	else if (strcmp(f, "pgkid") == 0)
	{
		strncpy(netInfo->pgkid, v, GKID_LEN);
		netInfo->pgkid[GKID_LEN-1] = '\0';
	}
	else if (strcmp(f, "techp") == 0)
	{
		strncpy(netInfo->techprefix, v, PHONE_NUM_LEN);
		netInfo->techprefix[PHONE_NUM_LEN-1] = '\0';
	}
	else if (strcmp(f, "type") == 0)
	{
		int type = FindIedgeType(v);
		if (type >= 0)
		{
			SetIedgeType(netInfo, type);
		}
		else
		{
			CLIPRINTF((stdout, "Type Unchanged\n"));
		}
	}
	else if (strcmp(f, "cp") == 0)
	{
		strncpy(netInfo->cpname, v, CALLPLAN_ATTR_LEN);
		netInfo->cpname[CALLPLAN_ATTR_LEN-1] = '\0';
	}
	else if (strcmp(f, "static") == 0)
	{
		char ipaddrstr[20] = { 0 };

		/* We expect an ip address here */
		strncpy(ipaddrstr, v, 20);
		netInfo->ipaddress.l = inet_addr(ipaddrstr);
		if (!strcmp(v, "none") ||
			(netInfo->ipaddress.l == (unsigned long)-1))
		{
			BIT_RESET(netInfo->sflags, ISSET_IPADDRESS);
			netInfo->stateFlags &= ~CL_STATIC;
		}
		else
		{
			netInfo->ipaddress.l = ntohl(netInfo->ipaddress.l);

			BIT_SET(netInfo->sflags, ISSET_IPADDRESS);
			netInfo->stateFlags |= CL_STATIC;
			netInfo->rasip = netInfo->ipaddress.l;
		}
	}
	else if (strcmp(f, "subnetip") == 0)
	{
		char ipaddrstr[20] = { 0 };

		/* We expect an ip address here */
		nx_strlcpy(ipaddrstr, v, 20);
		if (inet_pton(AF_INET, ipaddrstr, &netInfo->subnetip) != 1)
		{
			netInfo->subnetip = 0;	
		}
		else
		{
			netInfo->subnetip = ntohl(netInfo->subnetip);
		}

		if (netInfo->subnetip != 0 &&
		    netInfo->subnetmask == 0)
		  netInfo->subnetmask = IN_CLASSA(netInfo->subnetip)?IN_CLASSA_NET:(IN_CLASSB(netInfo->subnetip)?IN_CLASSB_NET:(IN_CLASSC(netInfo->subnetip)?IN_CLASSC_NET:(IN_CLASSD(netInfo->subnetip)?IN_CLASSD_NET:INADDR_BROADCAST)));
	}
	else if (strcmp(f, "subnetmask") == 0)
	{
		char ipaddrstr[20] = { 0 };

		/* We expect an ip address here */
		nx_strlcpy(ipaddrstr, v, 20);
		if (inet_pton(AF_INET, ipaddrstr, &netInfo->subnetmask) != 1)
		{
			netInfo->subnetmask = 0;	
		}
		else
		{
			netInfo->subnetmask = ntohl(netInfo->subnetmask);
		}
	}
	else if (strcmp(f, "h323id") == 0)
	{
		strncpy(netInfo->h323id, v, H323ID_LEN);
		netInfo->h323id[H323ID_LEN-1] = '\0';
	}
	else if (strcmp(f, "proxy") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			netInfo->stateFlags |= CL_PROXY;
			netInfo->stateFlags &= ~CL_FORWARD;
			netInfo->protocol = NEXTONE_REDIRECT_PROXY;
		}
		else
		{
			netInfo->stateFlags &= ~CL_PROXY;
			netInfo->protocol = NEXTONE_REDIRECT_OFF;
		}
	}
	else if (strcmp(f, "dnd") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			netInfo->stateFlags |= CL_DND;
		}
		else
		{
			netInfo->stateFlags &= ~CL_DND;
		}
	}
	else if (strcmp(f, "fwdtype") == 0)
	{
		if (!strcmp(v, "gis") || !strcmp(v, "forward"))
		{
			netInfo->stateFlags |= (CL_FORWARD|CL_FORWARDSTATIC);
			netInfo->stateFlags &= ~CL_PROXY;
			netInfo->protocol = NEXTONE_REDIRECT_FORWARD;
			BIT_SET(netInfo->nsflags, ISSET_PHONE);
		}
		else if (!strcmp(v, "rollover"))
		{
			netInfo->stateFlags |= (CL_FORWARD|CL_FORWARDSTATIC);
			netInfo->stateFlags &= ~CL_PROXY;
			netInfo->protocol = NEXTONE_REDIRECT_ROLLOVER;
			BIT_SET(netInfo->nsflags, ISSET_PHONE);
		}
		else
		{
			netInfo->stateFlags &= ~(CL_FORWARD|CL_FORWARDSTATIC);
			netInfo->protocol = NEXTONE_REDIRECT_OFF;
			BIT_RESET(netInfo->nsflags, ISSET_VPNPHONE);
			BIT_RESET(netInfo->nsflags, ISSET_PHONE);
		}
	}
	else if (strcmp(f, "fwdno") == 0)
	{
		if (BIT_TEST(netInfo->nsflags, ISSET_PHONE))
		{
			strncpy(netInfo->nphone, v, PHONE_NUM_LEN);
		}
	}
	else if (strcmp(f, "sip") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_SIP);

			// By Default - Sip Endpoints supports both versions of 
			// Sip Privacy (Draft01 and RFC3325)
			netInfo->ecaps1 |= ECAPS1_SIP_PRIVACY_RFC3325;
			netInfo->ecaps1 |= ECAPS1_SIP_PRIVACY_DRAFT01;
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_SIP);
		}
	}
	else if (strcmp(f, "h323") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_H323);
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_H323);
		}
	}
	else if (strcmp(f, "contact") == 0)
	{
		strncpy(netInfo->contact, v, SIPURL_LEN);
		netInfo->contact[SIPURL_LEN-1] = '\0';
	}
	else if (strcmp(f, "xcalls") == 0)
	{
		IedgeXCalls(netInfo) = atoi(v);
	}
	else if (strcmp(f, "infotranscap") == 0)
	{
		int tmpNum;
		
		tmpNum = str2enum( infoTransCapOptions, v );
		if ( tmpNum >= 0 )
		{
			IedgeInfoTransCap(netInfo) = tmpNum;
		}
	}
	else if (strcasecmp(f, "igrp") == 0)
	{
		if (strcmp(v, "none"))
		{
			IgrpDeleteCalls(netInfo->igrpName, IedgeInCalls(netInfo),
							IedgeOutCalls(netInfo), IedgeCalls(netInfo));	
			nx_strlcpy(netInfo->igrpName,v, IGRP_NAME_LEN);
			IgrpAddCalls(netInfo->igrpName, IedgeInCalls(netInfo),
							IedgeOutCalls(netInfo), IedgeCalls(netInfo));	
		}
		else
		{
			IgrpDeleteCalls(netInfo->igrpName, IedgeInCalls(netInfo),
							IedgeOutCalls(netInfo), IedgeCalls(netInfo));	
			memset(netInfo->igrpName, 0 , IGRP_NAME_LEN);
		}
	}
	else if (strcmp(f, "xincalls") == 0)
	{
		IedgeXInCalls(netInfo) = atoi(v);
	}
	else if (strcmp(f, "xoutcalls") == 0)
	{
		IedgeXOutCalls(netInfo) = atoi(v);
	}
	else if (strcmp(f, "priority") == 0)
	{
		netInfo->priority = atoi(v);
	}
	else if (strcmp(f, "grq") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_GRQ);
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_GRQ);
		}
	}
	else if (strcmp(f, "rai") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_RAI);
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_RAI);
		}
	}
	else if (strcmp(f, "tpg") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_TPG);
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_TPG);
		}
	}
	else if (strcmp(f, "rasport") == 0)
	{
		netInfo->rasport = atoi(v);
	}
	else if (strcmp(f, "q931port") == 0)
	{
		netInfo->callsigport = atoi(v);
	}
	else if (strcmp(f, "vendor") == 0)
	{
		netInfo->vendor = GetVendor(v);
	}
	else if (strcmp(f, "passwd") == 0)
	{
		nx_strlcpy(netInfo->passwd, v, PASSWD_LEN);
	}
	else if (strcmp(f, "mediarouting") == 0 ||
                 strcmp(f, "mr") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_MEDIAROUTE);
			netInfo->ecaps1 &= ~ECAPS1_NOMEDIAROUTE;
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_MEDIAROUTE);
		}
	}
        else if (strcmp(f, "nmr") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_NOMEDIAROUTE;
			BIT_RESET(netInfo->cap, CAP_MEDIAROUTE);
		}
		else
		{
			netInfo->ecaps1 &= ~ECAPS1_NOMEDIAROUTE;
		}
	}
	else if (strcmp(f, "hideaddresschange") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			BIT_SET(netInfo->cap, CAP_HIDEADDRESSCHANGE);
		}
		else
		{
			BIT_RESET(netInfo->cap, CAP_HIDEADDRESSCHANGE);
		}
	}
	else if (strcmp(f, "directcall") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			netInfo->ecaps1 &= ~ECAPS1_NODIRECTCALLS;
		}
		else
		{
			netInfo->ecaps1 |= ECAPS1_NODIRECTCALLS;
		}
	}
	else if (strcasecmp(f, "h323display") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 &= ~ECAPS1_NOH323DISPLAY;
		}
		else
		{
			netInfo->ecaps1 |= ECAPS1_NOH323DISPLAY;
		}
	}
	else if (strcasecmp(f, "mapalias") == 0)
	{
			if(!strcmp(v, "enable"))
			{
				netInfo->ecaps1 |= ECAPS1_MAPALIAS;
			}
			else
			{
				netInfo->ecaps1 &= ~ECAPS1_MAPALIAS;
			}
	}
	else if (strcasecmp(f, "forceh245") == 0)
	{
			if(!strcmp(v, "enable"))
			{
				netInfo->ecaps1 |= ECAPS1_FORCEH245;
			}
			else
			{
				netInfo->ecaps1 &= ~ECAPS1_FORCEH245;
			}
	}
	else if (strcasecmp(f, "pionfaststart") == 0)
	{
			if(!strcmp(v, "enable"))
			{
				netInfo->ecaps1 |= ECAPS1_PIONFASTSTART;
			}
			else
			{
				netInfo->ecaps1 &= ~ECAPS1_PIONFASTSTART;
			}
	}
	else if(strcasecmp(f,"bcaplayer1") == 0) 
	{
		if((ch = GetBcapLayer1(v))>=0) 
		{
			netInfo->bcap[BCAP_LAYER1] = ch;
		}
	}
	else if (strcasecmp(f,"cdpntype")==0)
	{
			if (!strcasecmp(v, "unknown"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_Unknown;
			}
			else if (!strcasecmp(v, "international"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_International;
			}
			else if (!strcasecmp(v, "national"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_National;
			}
			else if (!strcasecmp(v, "specific"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_Specific;
			}
			else if (!strcasecmp(v, "subscriber"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_Subscriber;
			}
			else if (!strcasecmp(v, "abbreviated"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_Abbreviated;
			}
			else if (!strcasecmp(v, "pass"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_Pass;
			}
			else if (!strcasecmp(v, "none") || !strcasecmp(v, "default"))
			{
				netInfo->q931IE[Q931IE_CDPN] = Q931CDPN_Default;
			}
	}
	else if (strcasecmp(f,"cgpntype")==0)
	{
			if (!strcasecmp(v, "pass"))
			{
				netInfo->q931IE[Q931IE_CGPN] = Q931CGPN_Pass;
			}
			else if (!strcasecmp(v, "unknown"))
			{
				netInfo->q931IE[Q931IE_CGPN] = Q931CGPN_Unknown;
			}
			else if (!strcasecmp(v, "international"))
			{
				netInfo->q931IE[Q931IE_CGPN] = Q931CGPN_International;
			}
			else if (!strcasecmp(v, "national"))
			{
				netInfo->q931IE[Q931IE_CGPN] = Q931CGPN_National;
			}
			else if (!strcasecmp(v, "specific"))
			{
				netInfo->q931IE[Q931IE_CGPN] = Q931CGPN_Specific;
			}
			else if (!strcasecmp(v, "subscriber"))
			{
				netInfo->q931IE[Q931IE_CGPN] = Q931CGPN_Subscriber;
			}
			else if (!strcasecmp(v, "abbreviated"))
			{
				netInfo->q931IE[Q931IE_CGPN] = Q931CGPN_Abbreviated;
			}
	}
	else if (strcmp(f, "maxhunts") == 0)
	{
		netInfo->maxHunts = atoi(v);
		if (netInfo->maxHunts > SYSTEM_MAX_HUNTS)
		{
			netInfo->maxHunts = SYSTEM_MAX_HUNTS;
		}
	}
	else if (strcmp(f, "crid") == 0)
	{
		netInfo->crId = atoi(v);
	}
	else if (strcmp(f, "custid") == 0)
	{
		nx_strlcpy(netInfo->custID, v, CLIENT_ATTR_LEN);
	}
	else if (strcmp(f, "tg") == 0)
	{
		nx_strlcpy(netInfo->tg, v, PHONE_NUM_LEN);
	}
	else if (strcmp(f, "newsrcitg") == 0)
	{
		nx_strlcpy(netInfo->srcIngressTG, (strcmp(v, "none")==0 ? "" : v), PHONE_NUM_LEN);
	}
	else if (strcmp(f, "newsrcdtg") == 0)
	{
		nx_strlcpy(netInfo->srcEgressTG, (strcmp(v, "none")==0 ? "" : v), PHONE_NUM_LEN);
	}
	else if (strcmp(f, "ogp") == 0)
	{
		nx_strlcpy(netInfo->ogprefix, v, PHONE_NUM_LEN);
	}
	else if (strcasecmp(f, "connh245addr") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 &= ~ECAPS1_NOCONNH245;
		}
		else
		{
			netInfo->ecaps1 |= ECAPS1_NOCONNH245;
		}
	} 
	else if (strcasecmp(f, "realmid") == 0)
	{
		netInfo->realmId = atoi (v);
	}
	else if (strcasecmp(f, "realm") == 0)
	{
		nx_strlcpy(netInfo->realmName,v, REALM_NAME_LEN);
	}
	else if (strcasecmp(f, "igrp") == 0)
	{
		nx_strlcpy(netInfo->igrpName,v, IGRP_NAME_LEN);
	}
	else if (strcasecmp(f, "sipdomain") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->stateFlags |= CL_SIPDOMAIN;
		}
		else
		{
			netInfo->stateFlags &= ~CL_SIPDOMAIN;
		}
	}
	else if (strcasecmp(f, "setdesttg") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_SETDESTTG;
		}
		else
		{
			netInfo->ecaps1 &= ~ECAPS1_SETDESTTG;
		}
	}
	else if (strcasecmp(f, "setdesttg") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_SETDESTTG;
		}
		else
		{
			netInfo->ecaps1 &= ~ECAPS1_SETDESTTG;
		}
	}
	else if (strcasecmp(f, "mapcc") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_MAPISDNCC;
		}
		else
		{
			netInfo->ecaps1 &= ~ECAPS1_MAPISDNCC;
		}
	}
	else if (strcasecmp(f, "deltcst38") == 0)
	{
		if(!strcmp(v, "default"))
		{
			netInfo->ecaps1 &= ~ECAPS1_DELTCST38DFT;
		}
		else if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_DELTCST38DFT;
			netInfo->ecaps1 |= ECAPS1_DELTCST38;
		}
		else
		{
			netInfo->ecaps1 |= ECAPS1_DELTCST38DFT;
			netInfo->ecaps1 &= ~ECAPS1_DELTCST38;
		}
	}
	else if (strcasecmp(f, "deltcs2833") == 0)
	{
		if(!strcmp(v, "default"))
		{
			netInfo->ecaps1 &= ~ECAPS1_DELTCSRFC2833DFT;
		}
		else if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_DELTCSRFC2833;
			netInfo->ecaps1 |= ECAPS1_DELTCSRFC2833DFT;
		}
		else
		{
			netInfo->ecaps1 &= ~ECAPS1_DELTCSRFC2833;
			netInfo->ecaps1 |= ECAPS1_DELTCSRFC2833DFT;
		}
	}
	else if (strcasecmp(f, "removetg") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_NOTG;
		}
		else
		{
			netInfo->ecaps1 &= ~ECAPS1_NOTG;
		}
	}
	else if (strcasecmp(f, "sticky") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->stateFlags |= CL_STICKY;
		}
		else
		{
			netInfo->stateFlags &= ~CL_STICKY;
		}
	}
	else if (strcasecmp(f, "uareg") == 0)
	{
		if(!strcmp(v, "enable"))
		{
			netInfo->stateFlags |= CL_UAREG;
		}
		else
		{
			netInfo->stateFlags &= ~CL_UAREG;
		}
	}
	else if (strcasecmp(f, "qval") == 0)
	{
		nx_strlcpy(netInfo->qval, v, 4);
	}
	else if (strcasecmp(f, "2833capable") == 0)
	{
		if(!strcmp(v, "yes"))
		{
				netInfo->ecaps1 |= ECAPS1_CAP2833;
				netInfo->ecaps1 |= ECAPS1_CAP2833_KNOWN;
		}
		else if(!strcmp(v, "no"))
		{
				netInfo->ecaps1 &= ~(ECAPS1_CAP2833);
				netInfo->ecaps1 |= ECAPS1_CAP2833_KNOWN;
		}
		else {
				//Unknown 2833 capability
				netInfo->ecaps1 &= ~(ECAPS1_CAP2833_KNOWN);
		}
	}
	else if (strcmp(f, "dtg") == 0)
	{
		nx_strlcpy(netInfo->dtg, v, PHONE_NUM_LEN);
	}
        else if (strcmp(f, "natdetect") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			netInfo->ecaps1 |= ECAPS1_NATDETECT;
		}
		else
		{
			netInfo->ecaps1 &= ~ECAPS1_NATDETECT;
		}
	}
	else if (strcmp(f, "natip") == 0)
	{
		char ipaddrstr[20] = { 0 };

		/* We expect an ip address here */
		strncpy(ipaddrstr, v, 20);
		netInfo->natIp = inet_addr(ipaddrstr);
		if (!strcmp(v, "none") ||
			(netInfo->natIp == (unsigned long)-1) ||
			(netInfo->natIp == 0))
		{
			BIT_RESET(netInfo->sflags, ISSET_NATIP);
		}
		else
		{
			netInfo->natIp = ntohl(netInfo->natIp);

			BIT_SET(netInfo->sflags, ISSET_NATIP);
		}
	}
	else if (strcmp(f, "natport") == 0)
	{
		netInfo->natPort = atoi(v);
		if(netInfo->natPort > 0)
		{
			BIT_SET(netInfo->sflags, ISSET_NATPORT);
		}
		else
		{
			BIT_RESET(netInfo->sflags, ISSET_NATPORT);
		}
	}
	else if(strcmp(f, "privacy") == 0) 
        {
	  
	  if(strcmp(v,"both") == 0){

	    netInfo->ecaps1 |= ECAPS1_SIP_PRIVACY_DRAFT01;
	    netInfo->ecaps1 |= ECAPS1_SIP_PRIVACY_RFC3325;
	  }
	  else if(strcmp(v,"draft01") == 0) {
	    
	    netInfo->ecaps1 &= ~ECAPS1_SIP_PRIVACY_RFC3325;
	    netInfo->ecaps1 |= ECAPS1_SIP_PRIVACY_DRAFT01;
	  }
	  else if(strcmp(v,"rfc3325") == 0) {
	    
	    netInfo->ecaps1 &= ~ECAPS1_SIP_PRIVACY_DRAFT01;
	    netInfo->ecaps1 |= ECAPS1_SIP_PRIVACY_RFC3325;	  
	  }
	} 
        else if (strcmp(f, "cidblock") == 0)
        {
                if(strcmp(v,"enable") == 0 )
                {
                        netInfo->cidblock = 1;
                } 
                else if (strcmp(v,"disable") == 0 )
                {
                        netInfo->cidblock = 0;
                }
        }        
	else
	{
		// Its one of the registration attributes
		NetoidStoreRegPair(netInfo, clAttribs, f, v);
	}
}
	
void
VpnStorePair(VpnEntry *vpnEntry, char *f, char *v)
{
	if (strcmp(f, "location") == 0)
	{
		strncpy(vpnEntry->vpnLoc, v, VPNS_ATTR_LEN);
		vpnEntry->vpnLoc[VPNS_ATTR_LEN-1] = '\0';
	}
	else if (strcmp(f, "contact") == 0)
	{
		strncpy(vpnEntry->vpnContact, v, VPNS_ATTR_LEN);
		vpnEntry->vpnContact[VPNS_ATTR_LEN-1] = '\0';
	}
	else if (strcmp(f, "cpname") == 0)
	{
		strncpy(vpnEntry->cpname, v, CALLPLAN_ATTR_LEN);
		vpnEntry->vpnContact[CALLPLAN_ATTR_LEN-1] = '\0';
	}
	else if (strcmp(f, "prefix") == 0)
	{
		strncpy(vpnEntry->prefix, v, PHONE_NUM_LEN);
		vpnEntry->prefix[PHONE_NUM_LEN-1] = '\0';
	}
}
	
void
VpnGStorePair(VpnGroupEntry *vpnGroupEntry, char *f, char *v)
{
	if (strcmp(f, "cpname") == 0)
	{
		strncpy(vpnGroupEntry->cpname, v, CALLPLAN_ATTR_LEN);
		vpnGroupEntry->cpname[CALLPLAN_ATTR_LEN-1] = '\0';
	}
}
	
void
CPStorePair(CallPlanEntry *cpEntry, char *f, char *v)
{
	if (strcmp(f, "pcpname") == 0)
	{
		strncpy(cpEntry->pcpname, v, CALLPLAN_ATTR_LEN);
		cpEntry->pcpname[CALLPLAN_ATTR_LEN-1] = '\0';
	}
}

void
CPBStorePair(CallPlanBindEntry *cpbEntry, char *f, char *v)
{
	char storec;

	if (strcmp(f, "priority") == 0)
	{
		cpbEntry->priority = atoi(v);
	}
	else if (strcmp(f, "flags") == 0)
	{
		cpbEntry->crflags = atoi(v);
	}
	else if (strcmp(f, "ftype") == 0)
	{
		if (!strcmp(v, "forward"))
		{
			cpbEntry->crflags &= ~(CRF_FORWARD|CRF_ROLLOVER);
			cpbEntry->crflags |= CRF_FORWARD;
		}
		else if (!strcmp(v, "rollover"))
		{
			cpbEntry->crflags &= ~(CRF_FORWARD|CRF_ROLLOVER);
			cpbEntry->crflags |= CRF_ROLLOVER;
		}
		else
		{
			cpbEntry->crflags &= ~(CRF_FORWARD|CRF_ROLLOVER);
		}
	}
	else if (strcmp(f, "type") == 0)
	{
		if (!strcmp(v, "reject"))
		{
			cpbEntry->crflags |= CRF_REJECT;
		}
		else
		{
			cpbEntry->crflags &= ~CRF_REJECT;
		}
	}
	else if (strcmp(f, "stime") == 0)
	{
		sscanf(v, "%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d",
			&cpbEntry->sTime.tm_year,
			&storec,
			&cpbEntry->sTime.tm_yday,
			&storec,
			&cpbEntry->sTime.tm_wday,
            &storec,
            &cpbEntry->sTime.tm_mon,
            &storec,
            &cpbEntry->sTime.tm_mday,
            &storec,
            &cpbEntry->sTime.tm_hour,
            &storec,
            &cpbEntry->sTime.tm_min,
            &storec,
            &cpbEntry->sTime.tm_sec);
    }
    else if (strcmp(f, "ftime") == 0)
	{
        sscanf(v, "%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d",
            &cpbEntry->fTime.tm_year,
            &storec,
            &cpbEntry->fTime.tm_yday,
            &storec,
            &cpbEntry->fTime.tm_wday,
            &storec,
            &cpbEntry->fTime.tm_mon,
            &storec,
            &cpbEntry->fTime.tm_mday,
            &storec,
            &cpbEntry->fTime.tm_hour,
            &storec,
            &cpbEntry->fTime.tm_min,
            &storec,
            &cpbEntry->fTime.tm_sec);
    }
}

void
RouteStorePair(VpnRouteEntry *vpnRouteEntry, char *f, char *v)
{
	if (strcmp(f, "prefix") == 0)
	{
		strncpy(vpnRouteEntry->prefix, v, VPN_LEN);
		vpnRouteEntry->prefix[VPN_LEN-1] = '\0';
	}
	else if (strcmp(f, "dest") == 0)
	{
		strncpy(vpnRouteEntry->dest, v, VPN_LEN);
		vpnRouteEntry->dest[VPN_LEN-1] = '\0';
	}
	else if (strcmp(f, "destlen") == 0)
	{
		vpnRouteEntry->destlen = atoi(v);
	}
	else if (strcmp(f, "src") == 0)
	{
		strncpy(vpnRouteEntry->src, v, VPN_LEN);
		vpnRouteEntry->src[VPN_LEN-1] = '\0';
	}
	else if (strcmp(f, "srclen") == 0)
	{
		vpnRouteEntry->srclen = atoi(v);
	}
	else if (strcmp(f, "srcprefix") == 0)
	{
		strncpy(vpnRouteEntry->srcprefix, v, VPN_LEN);
		vpnRouteEntry->srcprefix[VPN_LEN-1] = '\0';
	}
	else if (strcmp(f, "calltype") == 0)
	{
		vpnRouteEntry->crflags = 
			RouteFlagsValue(vpnRouteEntry->crflags, v);
	}
	else if (strcmp(f, "template") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			vpnRouteEntry->crflags |= CRF_TEMPLATE;
		}
		else
		{
			vpnRouteEntry->crflags &= ~CRF_TEMPLATE;
		}
	}
	else if (strcmp(f, "type") == 0)
	{
		if (!strcmp(v, "reject"))
		{
			vpnRouteEntry->crflags |= CRF_REJECT;
		}
		else
		{
			vpnRouteEntry->crflags &= ~CRF_REJECT;
		}
	}
	else if (strcmp(f, "flags") == 0)
	{
		vpnRouteEntry->crflags = atoi(v);
	}
	else if (strcmp(f, "dnisdefault") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			vpnRouteEntry->crflags |= CRF_DNISDEFAULT;
		}
		else
		{
			vpnRouteEntry->crflags &= ~CRF_DNISDEFAULT;
		}
	}
	else if (strcmp(f, "cpname") == 0)
	{
		nx_strlcpy(vpnRouteEntry->cpname, v, CALLPLAN_ATTR_LEN);
	}
	else if (strcmp(f, "sticky") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			vpnRouteEntry->crflags |= CRF_STICKY;
		}
		else
		{
			vpnRouteEntry->crflags &= ~CRF_STICKY;
		}
	}
}
	
void
TriggerStorePair(TriggerEntry *tgEntry, char *f, char *v)
{
	if (strcmp(f, "event") == 0)
	{
		if (!strcmp(v, "req-mode-fax"))
		{
			tgEntry->event = TRIGGER_EVENT_H323REQMODEFAX;
		}
		else if(!strcmp(v, "h323-t38-fax"))
		{
			tgEntry->event = TRIGGER_EVENT_H323T38FAX;
		}
	}
	else if (strcmp(f, "srcvendor") == 0)
	{
		tgEntry->srcvendor = GetVendor(v);
	}
	else if (strcmp(f, "dstvendor") == 0)
	{
		tgEntry->dstvendor = GetVendor(v);
	}
	else if (strcmp(f, "script") == 0)
	{
		if (!strcmp(v, "insert-route"))
		{
			tgEntry->action = TRIGGER_ACTION_INSERTROUTE;
		}
	}
	else if (strcmp(f, "sdata") == 0)
	{
		nx_strlcpy(tgEntry->actiondata, v, TRIGGER_ATTR_LEN);
	}
	else if(strcmp(f, "override") == 0)
	{
		if(strcmp(v, "enable") == 0)
		{
			tgEntry->actionflags |= TRIGGER_FLAG_ROUTE_OVERRIDE;
		}
		else
		{
			tgEntry->actionflags &= ~TRIGGER_FLAG_ROUTE_OVERRIDE;
		}
	}
}

int
RealmStorePair(RealmEntry *rmEntry, char *f, char *v)
{
	if(strcmp(f, "rsa") == 0)
	{
		rmEntry->rsa = StringToIp(v);
		return 0;
	}
	if(strcasecmp(f, "vnet") == 0)
	{
		nx_strlcpy(rmEntry->vnetName, v, VNET_NAME_LEN);
		return 0;
	}
	if(strcmp(f, "mask") == 0)
	{
		rmEntry->mask = StringToIp(v);
		return 0;
	}
	else if(strcmp(f, "sigpool") == 0)
	{
		 rmEntry->sigPoolId = atoi(v);
		return 0;
	}
	else if(strcmp(f, "medpool") == 0)
	{
		 rmEntry->medPoolId = atoi(v);
		return 0;
	}
	else if(strcmp(f, "addr") == 0)
	{
		if (!strcmp(v, "public"))
		{
			rmEntry->addrType= 0;
		}
		else if (!strcmp(v, "private"))
		{
			rmEntry->addrType= 1;
		}
		return 0;
	}
	else if(strcmp(f, "admin") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			rmEntry->adminStatus = 1;
		}
		else if (!strcmp(v, "disable"))
		{
			rmEntry->adminStatus = 0;
		}
		return 0;
	}
	else if(strcmp(f, "imr") == 0)
	{
		if (!strcmp(v, "xxx"))
		{
			rmEntry->intraRealm_mr = 0;
		}
		else if (!strcmp(v, "alwayson"))
		{
			rmEntry->intraRealm_mr = 1;
		}
		else if (!strcmp(v, "alwaysoff"))
		{
			rmEntry->intraRealm_mr = 2;
		}
		else if (!strcmp(v, "on"))
		{
			rmEntry->intraRealm_mr = 3;
		}
		return 0;
	}
	else if(strcmp(f, "emr") == 0)
	{
		if (!strcmp(v, "xxx"))
		{
			rmEntry->interRealm_mr = 0; 
		}
		else if (!strcmp(v, "alwayson"))
		{
			rmEntry->interRealm_mr = 1;
		}
		else if (!strcmp(v, "alwaysoff"))
		{
			rmEntry->interRealm_mr = 2;
		}
		else if (!strcmp(v, "on"))
		{
			rmEntry->interRealm_mr = 3;
		}
		return 0;
	}
        else if (strcmp(f,"cidblock") == 0)
        {
                if(strlen(v) < CID_BLK_UNBLK_LEN)
                {
                        nx_strlcpy(rmEntry->cidblk, v, CID_BLK_UNBLK_LEN);
                        return 0;                        
                }                
        } 
        else if (strcmp(f,"cidunblock") ==0)
        {
                if(strlen(v) < CID_BLK_UNBLK_LEN)
                {
                        nx_strlcpy(rmEntry->cidunblk, v, CID_BLK_UNBLK_LEN);
                        return 0;                        
                }                                
        }        

	else if(strcmp(f, "default") == 0)
	{
		if (!strcmp(v, "enable"))
		{
			rmEntry->flags |= REALMF_DEFAULT;
		}
		else if (!strcmp(v, "disable"))
		{
			rmEntry->flags &= ~REALMF_DEFAULT;
		}
		return 0;
	}
	else if(strcmp(f, "sipauth") == 0)
	{
		if(strcmp(v, "inv") == 0)
		{
			rmEntry->authFlags |= REALM_SIP_AUTH_INVITE;
		}
		else if(strcmp(v, "reg") == 0)
		{
			rmEntry->authFlags |= REALM_SIP_AUTH_REGISTER;
		}
		else if(strcmp(v, "bye") == 0)
		{
			rmEntry->authFlags |= REALM_SIP_AUTH_BYE;
		}
		else if(strcmp(v, "all") == 0)
		{
			rmEntry->authFlags |= REALM_SIP_AUTH_INVITE;
			rmEntry->authFlags |= REALM_SIP_AUTH_REGISTER;
			rmEntry->authFlags |= REALM_SIP_AUTH_BYE;
		}
		else if(strcmp(v, "none") == 0)
		{
			rmEntry->authFlags &= ~REALM_SIP_AUTH_INVITE;
			rmEntry->authFlags &= ~REALM_SIP_AUTH_REGISTER;
			rmEntry->authFlags &= ~REALM_SIP_AUTH_BYE;
		}
		return 0;
	}
	else if(strcmp(f,"proxy_regid") == 0 )
	{
		if(strlen(v) < REG_ID_LEN)
                {
                        nx_strlcpy(rmEntry->mp.regid, v, REG_ID_LEN);
                        return 0;                        
                }   
	}
	else if(strcmp(f,"proxy_uport") == 0 )
	{
		rmEntry->mp.uport = atoi(v);
		return 0;
	}
	return -1;
}

int
IgrpStorePair(IgrpInfo *ptr, char *f, char *v)
{
	if(strcmp(f, "maxcallsin") == 0)
	{
		IgrpXInCalls(ptr) = atoi(v);
		return 0;
	}
	else if(strcmp(f, "maxcallsout") == 0)
	{
		IgrpXOutCalls(ptr) = atoi(v);
		return 0;
	}
	else if(strcmp(f, "maxcallstotal") == 0)
	{
		IgrpXCallsTotal(ptr) = atoi(v);
		return 0;
	}
	return -1; 
}

int
VnetStorePair(VnetEntry *ptr, char *f, char *v)
{
	if(strcmp(f, "ifname") == 0)
	{
		if (strlen(v) < IFI_NAME)
		{
			nx_strlcpy(ptr->ifName, v, IFI_NAME);
			return 0;
		}
	} 
#ifdef NETOID_LINUX
	else if(strcmp(f, "vlanid") == 0)
	{
		if (!strcasecmp(v, VLANID_NONE_STR))
		{
			ptr->vlanid = VLANID_NONE;
		} else if (!IsAllNumStr(v))
		{
			ptr->vlanid = VLANID_INVALID;
		} else
		{
			ptr->vlanid = atoi(v);
		}
		return 0;
	}
	else if(strcmp(f, "gateway") == 0)
	{
		ptr->gateway = StringToIp(v);
		return 0;
	}
#endif
	return -1; 
}

int
GetNetoidAttrPairs(char *comm, int *argcp, char ***argvp, 
					void *entry, 
					ClientAttribs *clAttribs)
{
	int argc = *argcp;
	char **argv = *argvp;
	int alldone = 0;
	int type = 0;

	// The first "/" may or may not be present
	// or may be one of several characters: "/" or "-"
	if ((argv[0][0] == '/') || (argv[0][0] == '-'))
	{
		argc --; argv++;
		//CLIPRINTF((stdout, "%s: Error: Invalid Format\n", comm));
		//goto _error;
	}

	//argc --; argv++;

	while (argc >= 2)	/* attr, value */
	{
		switch (type)
		{
		case 0:
			NetoidStorePair(entry, clAttribs, argv[0], argv[1]);
			break;
		}

		argc -=2;
		argv += 2;
		
		if ((argc > 0) && ((argv[0][0] == '/') || (argv[0][0] == '-')))
		{
			break;
		}
	}

	if ((argc == 0) || 
		((argc > 0) && ((argv[0][0] == '/') || (argv[0][0] == '-'))))
	{
		argc --;
		argv ++;
		alldone = 1;
	}	

	/* Check if we did a complete scan */
	if (alldone == 0)
	{
		CLIPRINTF((stdout, "%s: Error: Insufficient Arguments\n", comm));
	}

_error:
	*argcp = argc;
	*argvp = argv;

	return 0;
}

int
GetAttrPairs(char *comm, int *argcp, char ***argvp, void *entry, int type)
{
	int argc = *argcp;
	char **argv = *argvp;
	int alldone = 0;

	if ((argv[0][0] == '/') || (argv[0][0] == '-'))
	{
		argc --; argv++;
		//CLIPRINTF((stdout, "%s: Error: Invalid Format\n", comm));
		//goto _error;
	}

	while (argc >= 2)
	{
		switch (type)
		{
		case CLI_GET_ATTR_VPN:
			VpnStorePair(entry, argv[0], argv[1]);
			break;
		case CLI_GET_ATTR_ROUTE:
			RouteStorePair(entry, argv[0], argv[1]);
			break;
		case CLI_GET_ATTR_VPNG:
			VpnGStorePair(entry, argv[0], argv[1]);
			break;
		case CLI_GET_ATTR_CP:
			CPStorePair(entry, argv[0], argv[1]);
			break;
		case CLI_GET_ATTR_CPB:
			CPBStorePair(entry, argv[0], argv[1]);
			break;
		case CLI_GET_ATTR_GK:
			GkStorePair(entry, argv[0], argv[1]);
			break;
		case CLI_GET_ATTR_TRIGGER:
			TriggerStorePair(entry, argv[0], argv[1]);
			break;
		case CLI_GET_ATTR_REALM:

		        if(RealmStorePair(entry, argv[0], argv[1]) < 0)
			{
			    CLIPRINTF((stdout, "Error:Unknown option %s\n", argv[0]));
			}                        
			break;

		case CLI_GET_ATTR_IGRP:
			if(IgrpStorePair(entry, argv[0], argv[1]) < 0)
			{
				CLIPRINTF((stdout, "Error:Unknown option %s\n", argv[0]));
			}
			break;
		case CLI_GET_ATTR_VNET:
			VnetStorePair(entry, argv[0], argv[1]);
			break;
		default:
			break;
		}

		argc -=2;
		argv += 2;
		
		if ((argc > 0) && ((argv[0][0] == '/') || (argv[0][0] == '-')))
		{
			break;
		}	
	}

	if ((argc == 0) || 
		((argc > 0) && ((argv[0][0] == '/') || (argv[0][0] == '-'))))
	{
		argc --;
		argv ++;
		alldone = 1;
	}	

	/* Check if we did a complete scan */
	if (alldone == 0)
	{
		CLIPRINTF((stdout, "%s: Error: Insufficient Arguments\n", comm));
	}

_error:
	*argcp = argc;
	*argvp = argv;

	return 0;
}

/* uport is on host order */
int
DeleteNetoidFromCache(InfoEntry *netInfo)
{
    char fn[] = "DeleteNetoidFromCache():";
    CacheTableInfo *info, cacheInfoEntry;
    int shmId;

	shmId = CacheAttach();

    if (shmId != -1)
    {
		
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		if (info = DeleteIedge(netInfo))
	  	{
	  	    CLIPRINTF((stdout, "Deleted entry from cache\n"));
	   	}
		else
		{
	  	    CLIPRINTF((stdout, "No such entry in cache\n"));
		}

		CacheReleaseLocks(regCache);

#if 0
		if (info && (MemGetRwLock(&lsMem->updatemutex, 
			LOCK_WRITE, LOCK_BLOCK) == AL_OK))
		{
			CacheEntry *ce;
			
			info->state |= CACHE_NEEDS_DELETE;
			info->data.mTime = time(0);
			ce = (CacheEntry *)SHM_Malloc(sizeof(CacheEntry));
			if (ce)
			{
				ce->entry = info;
				ce->type = CACHE_INFO_ENTRY;
				ListInsert(lsMem->updateList.prev, ce);
			}
			else
			{
				CLIPRINTF((stdout, "SHM: Out of memory\n"));
			}
			MemReleaseRwLock(&lsMem->updatemutex);
		}
		else
#endif
		if (info)
		{
			CFree(regCache)(info);
		}
		
		CacheDetach();
	}

    return 1;
}

/* return >=0, if no error, -1 if there is an error */
/* Note that because we have two caches and only one 
 * database, the entry my get inserted into the ls cache,
 * and its update list for redundant iserver, and fail the
 * vpns cache addition. If this happens, the entry will
 * fail the db addition, while on teh redundant iserver,
 * it will get added into the database. To prevent this,
 * we should return proper error codes from this
 * function, telling the caller what to do - future fix.
 */
int
UpdateNetoidInCache(NetoidInfoEntry *netInfo)
{
   	char fn[] = "UpdateNetoidInCache():";
   	CacheTableInfo *info, infoCopy;
   	NetoidSNKey key = {0};
   	int shmId;
   	int error = 1, found = 0;

   	strncpy(key.regid, netInfo->regid, REG_ID_LEN);
   	key.uport = netInfo->uport;

	shmId = CacheAttach();

   	if (shmId != -1)
   	{
		/* The iedge has to be added into each cache,
		 * and if there is a duplicate present for
		 * any iedge attribute, it is to be reported
		 */
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		info = GetDuplicateIedge(netInfo);

		if (info)
		{
			CLIPRINTF((stdout, "duplicate iedge in cache:\n"));
			CLIPRINTF((stdout, "reg id %s, uport %lu\n",
				info->data.regid, info->data.uport));

			error =  -xleExists;
			goto _return;
		}
	
		/* Otherwise, delete the iedge first and then add
		 * it back again.
		 */
	  	if (info = DeleteIedge(netInfo))
	  	{
			// update the db with this entry
			UpdateIedgeDynamicInfo(netInfo, &info->data);
			found = 1;

			SHM_Free(info);
	  	}

		info = CacheDupInfoEntry(netInfo);

		if (info)
		{
			ResetIedgeFields(info, 0);

			if (!found)
			{
				UpdateIedgeDynamicInfo(&info->data, NULL);
			}

			if (AddIedge(info) < 0)
			{
				CLIPRINTF((stdout, 
					"iedge duplicate found. Cache is corrupted\n"));

				DeleteIedge(netInfo);

				SHM_Free(info);

				error = -xleOpNoPerm;
				goto _return;
			}

			CLIPRINTF((stdout, "iedge added to cache\n"));
		}
		else
		{
			CLIPRINTF((stdout, "unable to add iedge to cache\n"));
			error = -xleOpNoPerm;
			goto _return;
		}

_return:
		CacheReleaseLocks(regCache);

		CacheDetach();
	}

    return error;
}

void
gdbmError(char *errorString)
{
     	NETERROR(MCLI, ("GDBM: %s", errorString));
}

int
OpenDatabases(DefCommandData *dbDefaults)
{
     	int k, rc = 0;
     	DB gdbmf;

     	for (k = 0; k < DB_eMax; k ++)
     	{
		DBSTRUCT(dbDefaults,k).read_write = GDBM_WRCREAT;
     		gdbmf = DbOpenByID(DBNAME(dbDefaults, k), k, &DBSTRUCT(dbDefaults,k));

     		if (gdbmf == NULL)
     		{
		  	CLIPRINTF((stdout, "Database %s Could not be opened!\n",
				DBNAME(dbDefaults, k)));
			rc = -1;
			break;
     		}

		GDBMF(dbDefaults, k) = gdbmf;
	  
     		log(LOG_DEBUG, 0, "Database %s opened successfully\n", 
						DBNAME(dbDefaults, k));
     	}

	if (rc < 0)
	{
		CloseDatabases(dbDefaults);
	}

     	return rc;
}

// This opens the gdbm file without acquiring any locks
// SHOULD NOT BE USED FOR /databases files
DB
OpenDBFile(char *fname)
{
	DB 		dbf;
	DB_tDb	dbinfo = {0};

	dbinfo.read_write = GDBM_WRCREAT;
	dbf = DbOpenByID(fname, -1, &dbinfo);
	if (dbf == NULL) {
		CLIPRINTF((stdout, "Database file %s could not be opened.\n",
			fname));
	}					

	return dbf;
}

// This closes the gdbm file without releasing any locks
// SHOULD NOT BE USED FOR /databases files
int
CloseDBFile(DB dbf)
{
	if (dbf) {
		gdbm_close(dbf);	
	}

	return 0;
}

int
CloseDatabases(DefCommandData *dbDefaults)
{
     	int k;
     	DB gdbmf;

     	for (k = 0; k < DB_eMax; k ++)
     	{
     		DbClose(&DBSTRUCT(dbDefaults,k));
	  
     		log(LOG_DEBUG, 0, "Database %s closed successfully\n", 
						dbDefaults[k].dbName);
     	}

     	return 0;
}

void
AgeIedgesInVpnGs(DefCommandData *dbs, char *vpng1, char *vpng2)
{
     	int shmId;

	shmId = CacheAttach();
	
     	if (shmId == -1)
     	{
	  	CLIPRINTF((stdout, "Unable to attach to VPNS cache\n"));
     	}
     	else
     	{
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		MemAgeIedgesInVpnGs(vpng1, vpng2);

		CacheReleaseLocks(vpnCache);
		CacheReleaseLocks(regCache);

		CacheDetach();
     	}
}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleVpnG(VpnGroupEntry *vpnGroupEntry, int op)
{
	CacheVpnGEntry *cacheVpnGEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		cacheVpnGEntry = CacheDelete(vpnGCache, vpnGroupEntry);
	
		CacheReleaseLocks(vpnGCache);

		if (cacheVpnGEntry)
		{
			CFree(vpnGCache)(cacheVpnGEntry);
		}

		rc = 0;
	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

		cacheVpnGEntry = CacheGet(vpnGCache, vpnGroupEntry->vpnGroup);

		if (cacheVpnGEntry)
		{
			/* Compare the two */
			if (memcmp(&cacheVpnGEntry->vpnGroupEntry, 
				vpnGroupEntry,
				sizeof(VpnGroupEntry)))
			{
				/* the two are different */
				memcpy(&cacheVpnGEntry->vpnGroupEntry,
					vpnGroupEntry, sizeof(VpnGroupEntry));
				/* Do the update */
				update = 1;
			}
		}
		else
		{

			CacheInsert(vpnGCache, 
				    CacheDupVpnGEntry(vpnGroupEntry));
			update = 1;
		}

		CacheReleaseLocks(vpnGCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

int
CacheHandleIedge(NetoidInfoEntry *netInfo, int op)
{
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		// license_release(1);
		rc = DeleteNetoidFromCache(netInfo);
	}

	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		if (op == CLIOP_ADD)
		{
#if 0
			if (license_allocate(1))
			{
				CLIPRINTF((stdout,"could not obtain license\n"));
				return -xleNoLicense;
			}
#endif
		}

		rc = UpdateNetoidInCache(netInfo);
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

int
CacheHandleVpn(VpnEntry *vpnEntry, int op)
{
	CacheVpnEntry *cacheVpnEntry;
	int update = 0;
	int rc = -1;
	
	if (lsMem == NULL)
	{
		return 0;
	}
	
	if ( op != CLIOP_DELETE && strlen(vpnEntry->vpnId) == 0)
	{
		 return 0;
	}

	/* First Delete the entry, if its there */
	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		/* First Delete the entry, if its there */
		cacheVpnEntry = CacheDelete(vpnCache, vpnEntry);
	
		CacheReleaseLocks(vpnCache);

		if (cacheVpnEntry)
		{
			CFree(vpnCache)(cacheVpnEntry);
		}

		rc = 0;
	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		cacheVpnEntry = CacheGet(vpnCache, vpnEntry->vpnName);

		if (cacheVpnEntry)
		{
			/* Compare the two */
			if (memcmp(&cacheVpnEntry->vpnEntry, vpnEntry,
				sizeof(VpnEntry)))
			{
				/* the two are different */
				memcpy(&cacheVpnEntry->vpnEntry,
					vpnEntry, sizeof(VpnEntry));
				/* Do the update */
				update = 1;
			}
		}
		else
		{

			CacheInsert(vpnCache, 
				    CacheDupVpnEntry(vpnEntry));
			update = 1;
		}

		CacheReleaseLocks(vpnCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

/* op = 0, means delete.
 * In case of delete  (op= CLIOP_DELETE), only the name of the route entry 
 *(crname member )  is required 
 * op = 1, means add
 */
int
CacheHandleCR(VpnRouteEntry *routeEntry, int op)
{
	CacheVpnRouteEntry *cacheRouteEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		//cacheRouteEntry = CacheDelete(cpCache, routeEntry);
		DeleteRoute(routeEntry);
	
		CacheReleaseLocks(cpbCache);
		CacheReleaseLocks(cpCache);

		rc = 0;
	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		DeleteRoute(routeEntry);
		AddRoute(CacheDupVpnRouteEntry(routeEntry));

		CacheReleaseLocks(cpbCache);
		CacheReleaseLocks(cpCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleCP(CallPlanEntry *cpEntry, int op)
{
	CacheEntry *ce = NULL;
	CacheCPEntry *cacheCPEntry = NULL;
	int rc = 0;

	if (lsMem == NULL)
	{
		return 0;
	}

	return rc;

}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleCPB(CallPlanBindEntry *cpbEntry, int op)
{
	CacheCPBEntry *cacheCPBEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		//cacheCPBEntry = CacheDelete(cpbCache, cpbEntry);
		DeleteCPB(cpbEntry);
	
		CacheReleaseLocks(cpbCache);

		rc = 0;

	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		DeleteCPB(cpbEntry);
		AddCPB(CacheDupCPBEntry(cpbEntry));

		CacheReleaseLocks(cpbCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

/* op = 0, means delete.
 * op = 1, means add
 */
int
CacheHandleTrigger(TriggerEntry *tgEntry, int op)
{
	CacheTriggerEntry *cacheTgEntry = NULL;
	int update = 0;
	int rc = -1;

	if (lsMem == NULL)
	{
		return 0;
	}

	if (op == CLIOP_DELETE)
	{
		CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete the entry, if its there */
		DeleteTrigger(tgEntry);
	
		CacheReleaseLocks(triggerCache);

		rc = 0;

	}
	
	if ((op == CLIOP_ADD) || (op == CLIOP_REPLACE))
	{
		CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);

		DeleteTrigger(tgEntry);
		AddTrigger(CacheDupTriggerEntry(tgEntry));

		CacheReleaseLocks(triggerCache);

		rc = 0;
	}

	if (op == CLIOP_CREAT)
	{
		rc = 0;
	}

	return rc;
}

int
CacheHandleRealm(RealmEntry *rmEntry, int op)
{
	CacheRealmEntry *cachermEntry = NULL;
	CacheRealmEntry *newrmEntry = NULL;
	CacheRealmEntry *tmprmEntry = NULL;
	int		rc = -1;

	if (!realmCache)
		return (1);		/* No realm cache, MSW must be down return as no error */

    switch (op)
    {
        case CLIOP_DELETE:
                CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);
		/* Delete the entry, if its there */
                rc = DeleteRealm(&rmEntry->realmId);
                CacheReleaseLocks(realmCache);

                break;
	
#if 0				
        case CLIOP_ADD:
                CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

                DeleteRealm(&rmEntry->realmId);
				newrmEntry = CacheDupRealmEntry(rmEntry);
				/* init socketId to erroneous value */
				newrmEntry->socketId = -1; 
                rc = AddRealm(newrmEntry);

                CacheReleaseLocks(realmCache);

	            break;
#endif

        case CLIOP_ADD:
        case CLIOP_REPLACE:
                CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

				tmprmEntry = CacheGet(realmCache, &rmEntry->realmId);
				if (tmprmEntry == NULL)
				{
					tmprmEntry = CacheGet(rsaCache, &rmEntry->rsa);
				}

				newrmEntry = CacheDupRealmEntry(rmEntry);
				if (newrmEntry && tmprmEntry)
				{
					newrmEntry->socketId = tmprmEntry->socketId;
					newrmEntry->realm.operStatus = tmprmEntry->realm.operStatus;
				}

                DeleteRealm(&rmEntry->realmId);
                rc = AddRealm(newrmEntry);

                CacheReleaseLocks(realmCache);

	            break;

		case CLIOP_CREAT:
				rc = 0;
	            break;

	}

	return rc;
}

int
CacheHandleIgrp(IgrpInfo *igrpEntry, int op)
{
	CacheIgrpInfo  *currCacheEntry = NULL;
	CacheIgrpInfo  *newCacheEntry = NULL;
	int     rc = 0;

    switch (op)
    {
        case CLIOP_DELETE:
                CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);
		/* Delete the entry, if its there */
                rc = DeleteIgrp(igrpEntry->igrpName);
                CacheReleaseLocks(igrpCache);

                break;
	
		case CLIOP_ADD:
		case CLIOP_REPLACE:
			CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

			/* Get the existing Cache entry */
			currCacheEntry = CacheGet(igrpCache, igrpEntry->igrpName);
			/* update calls stats from current entry */
			if (currCacheEntry)
			{
				UpdateIgrpDynamicInfo(igrpEntry, 
					&currCacheEntry->igrp);
			}
			newCacheEntry = CacheDupIgrpInfo(igrpEntry);

			DeleteIgrp (igrpEntry->igrpName);

			AddIgrp(newCacheEntry);

			CacheReleaseLocks(igrpCache);

			break;
	}

	return rc;
}

int
CacheHandleVnet(VnetEntry *vnetEntry, int op)
{
	CacheVnetEntry  *currCacheEntry = NULL;
	CacheVnetEntry  *newCacheEntry = NULL;
	int     rc = 0;

    switch (op)
    {
        case CLIOP_DELETE:
                CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);
		/* Delete the entry, if its there */
                rc = DeleteVnet(vnetEntry->vnetName);
                CacheReleaseLocks(vnetCache);

                break;
	
		case CLIOP_ADD:
		case CLIOP_REPLACE:
			CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);

			/* Get the existing Cache entry */
			currCacheEntry = CacheGet(vnetCache, vnetEntry->vnetName);

			newCacheEntry = CacheDupVnetEntry(vnetEntry);

			DeleteVnet (vnetEntry->vnetName);

			AddVnet(newCacheEntry);

			CacheReleaseLocks(vnetCache);

			break;
	}

	return rc;
}

int
UpdateNetoidPorts(char *regid, int (*cbfn)(void *entry, void *arg1, void *arg2, void *arg3), 
	void *arg1, void *arg2, void *arg3)
{
   	char fn[] = "UpdateNetoidPorts():";
	int shmId = -1;
   	CacheTableInfo *info = NULL, *cacheInfo = NULL;
   	NetoidSNKey key = {0};
   	int error = 1;
	int i;

	/* Attach to cache */
	/* Extract the first port of the netoid, and just walk
	 * over the entire cache
	 */

   	strncpy(key.regid, regid, REG_ID_LEN);

	shmId = CacheAttach();

   	if (shmId != -1)
   	{
		/* The iedge has to be added into each cache,
		 * and if there is a duplicate present for
		 * any iedge attribute, it is to be reported
		 */
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		info = CacheGet(regidCache, &key);

		if (info == NULL)
		{
			goto _return;
		}

		for (i=0; i<MAX_IEDGE_PORTS; i++)
		{	
			if (BITA_TEST(info->data.cfgports, i))
			{
				key.uport = i;
				cacheInfo = CacheGet(regCache, &key);
				cbfn(cacheInfo, arg1, arg2, arg3);

				if (cacheInfo)
				{
					UpdateNetoidDatabase(&cacheInfo->data);
				}
			}
		}

_return:
		CacheReleaseLocks(regCache);

		CacheDetach();
	}

    return error;
}

int
InheritIedgeGlobals(NetoidInfoEntry *netInfo)
{
   	int shmId;
	CacheTableInfo *cacheInfo = NULL;
	
	shmId = CacheAttach();

   	if (shmId != -1)
   	{
		/* The iedge has to be added into each cache,
		 * and if there is a duplicate present for
		 * any iedge attribute, it is to be reported
		 */
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		cacheInfo = CacheGet(regidCache, netInfo);

		if (cacheInfo != NULL)
		{
			netInfo->ipaddress.l = cacheInfo->data.ipaddress.l;
			BIT_COPY(netInfo->sflags, ISSET_IPADDRESS, 
				cacheInfo->data.sflags, ISSET_IPADDRESS);
			netInfo->stateFlags |= (cacheInfo->data.stateFlags & CL_STATIC);
			netInfo->callsigport = cacheInfo->data.callsigport;
			netInfo->rasip = cacheInfo->data.rasip;
			netInfo->rasport = cacheInfo->data.rasport;
		}

		CacheReleaseLocks(regCache);

		CacheDetach();
	}

	return 0;
}

// return 0 if no error
int
DbOperate(DB db, char *dinfo, int dlen, char *skey, int skeylen, int op, int verify)
{
	char fn[] = "DbOperate():";
	int rcop = 0, rcverify = 0;
	char *tmp = NULL;

	switch (op)
	{
	case CLIOP_NONE:
		break;
	case CLIOP_ADD:
	case CLIOP_CREAT:
		rcop = DbInsertEntry(db, dinfo, dlen, skey, skeylen);
		break;
	case CLIOP_DELETE:
		rcop = DbDeleteEntry(db, skey, skeylen);
		break;
	case CLIOP_REPLACE:
		rcop = DbStoreEntry(db, dinfo, dlen, skey, skeylen);
		break;
	default:
		NETERROR(MDB, ("%s Invalid args op = %d\n", fn, op));
		break;
	}

	if (rcop < 0)
	{
		return CLIOP_ERR;
	}

	if (verify == CLIOP_VERIFY)
	{
		tmp = DbFindEntry (db, skey, skeylen);
		switch (op)
		{
		case CLIOP_ADD:
		case CLIOP_REPLACE:
			if (tmp != NULL)
			{
				if (memcmp(tmp, dinfo, dlen))
				{
					// Insertion did not happen correctly
					free(tmp);
					return CLIOP_ERRDUP;
				}
				free(tmp);
			}
			else
			{
				// Insertion did not happen correctly
				return CLIOP_ERRVF;
			}
			break;
		case CLIOP_DELETE:
			if (tmp != NULL)
			{
				free(tmp);
				return CLIOP_ERRVF;
			}
			break;
		default:
			if (tmp != NULL) free(tmp);
			break;	
		}
	}

	// SUCCESS
	return 0;
}

void
DbResetStats()
{
	nroutes = 0;
	nbindings = 0; 
	niedges = 0; 
	nplans = 0; 
	nvpns = 0; 
	nvpngs = 0;
	ntriggers = 0;
	nerrors = 0;
	nrealms = 0;
	nigrps = 0;
	nvnets = 0;
}

void
DbPrintStats()
{
	printf("%d endpoints %d plans %d routes %d bindings %d vpns %d vpn groups %d nrealms %d nigrps %d triggers %d nvnets\n%d errors\n",
		niedges, nplans, nroutes, nbindings, nvpns, nvpngs, nrealms, nigrps, ntriggers, nvnets, nerrors);
}

char *
DbOperToStr(int op)
{
	switch (op)
	{
		case CLIOP_ADD:
			return "add";
		case CLIOP_DELETE:
			return "delete";
		case CLIOP_REPLACE:
			return "replace";
		case CLIOP_CREAT:
			return "create";

		case CLIOP_NONE:
		default:
			return "";
	}

	return "";
}

// Return the regid identified by key
// If key contains regid, return it.
// If key contains ip, return regid of entry
// if a regid is not found, MUST return the
// original key passed into the regid
// DB locks must not be acquired around this call
int
CliGetRegid(char *key, char *regid)
{
	unsigned long ipaddr;
	CacheTableInfo *cacheInfo;
	int found = -1;
	char realmName[REALM_NAME_LEN]= {0};
	RealmIP  realmip;

	if (!regid)
	{
		CLIPRINTF((stdout, "error: null regid for storage\n"));
		return -1;
	}

	memset(regid, 0 , REG_ID_LEN);

	if (!strncmp(key, "ip4:", 4))
	{
		realmip.ipaddress = 0;
		realmip.realmId = 0;

		ParseCliGetRegidIPStr(key+4, &realmip.ipaddress, realmName);
		realmip.realmId = realmNameToRealmId(realmName);
        
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		cacheInfo = CacheGet(ipCache, &realmip);
		if (cacheInfo)
		{
			nx_strlcpy(regid, cacheInfo->data.regid, REG_ID_LEN);
			found = 1;
		}

		CacheReleaseLocks(regCache);
	}

	if (found < 0)
	{
		nx_strlcpy(regid, key, REG_ID_LEN);	
	}

	return 1;
}

int
CliSendToRs(Cmd *cmd)
{
	int	sockfd, rc = 1;
	char fn[] = "CliSendToRs";
    struct sockaddr_un  servaddr;

	if ((send2Rs == 0) || !RSDConfig)
	{
		return 0;
	}

	/* Post the message to the Replication Server */

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
	{
		NETDEBUG(MCLI, NETLOG_DEBUG4,
			("%s: socket err - %s", fn, strerror(errno)));
		return(-1);
	}

    bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, RS_STR_FNAME);

	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) 
	{
		NETDEBUG(MCLI, NETLOG_DEBUG4,
			("%s: connect err - %s", fn, strerror(errno)));
		rc = -2;
		goto _error;
	}

	/* Fix the endian-ness of the command header before we post it */
	cmd->cmdlen = htonl(cmd->cmdlen);	
	cmd->cmdseq = htonl(cmd->cmdseq);	
	cmd->cmdtyp = htonl(cmd->cmdtyp);	
	cmd->cmdrval = htonl(cmd->cmdrval);	
	cmd->cmdact = htonl(cmd->cmdact);	
	cmd->cmdpid = htonl(cmd->cmdpid);	
	cmd->cmdtim = htonl(cmd->cmdtim);	

	if (write(sockfd, (void *)cmd, ntohl(cmd->cmdlen)) < 0) 
	{
		NETDEBUG(MCLI, NETLOG_DEBUG4,
			("%s: write err - %s", fn, strerror(errno)));
		rc = -3;
		goto _error;
	}

_error:
	close(sockfd);

	return rc;
}

// post a file copy command to the rs
// see if we need to put a pathname in from of the filename
void
CliPostCpCommand(char *path, int revno, int expectedrc, int action)
{
	char cmd[1024] = { 0 };
	Cmd *cmdp;
	int len = sizeof(Cmd), max = 1020;
	time_t tstamp = time(NULL);
	pid_t pid = getpid();

	if (revno < 0)
	{
		return;
	}

	if (path[0] == '/')
	{
		len += snprintf(cmd+len, max-len, path);
	}
	else
	{
		len += strlen(getcwd(cmd+len, max-len));
		len += snprintf(cmd+len, max-len, "/%s", path);
	}

	/* Fill The Command Header */
	cmdp = (Cmd *)cmd;
	cmdp->cmdtyp = CMD_FCP;
	cmdp->cmdlen = (((len+1)+4)/4)*4;
	cmdp->cmdseq = revno;
	cmdp->cmdrval = expectedrc;
	cmdp->cmdact = action;
	cmdp->cmdpid = pid;
	cmdp->cmdtim = tstamp;

	SaveCmdInHistDB(cmdp);

	if(cli_debug)
	{
		CLIPRINTF((stdout, "posting: [%s] [seq = %d] [rc = %d] [action = %d]\n", 
			cmd+sizeof(Cmd), revno, expectedrc, action));
	}

	if (CliSendToRs(cmdp) > 0)
	{
		CLIPRINTF((stdout, "posted: [%s] [rc = %d] [action = %d]\n", 
			cmd+sizeof(Cmd), expectedrc, action));
	}
}

void
CliPostCmd(int cmdtype, char *cmdprefix, char *cmdrest, long revno, 
		int expectedrc, int action)
{
	char cmd[1024] = { 0 };
	Cmd *cmdp;
	int len = sizeof(Cmd), max = 1020;
	time_t tstamp = time(NULL);
	pid_t pid = getpid();

	if (revno < 0)
	{
		return;
	}

	len += snprintf(cmd+len, max-len, cmdprefix);
	len += snprintf(cmd+len, max-len, cmdrest);

	/* Fill The Command Header */
	cmdp = (Cmd *)cmd;
	cmdp->cmdtyp = cmdtype;
	cmdp->cmdlen = len+1;
	cmdp->cmdlen = (((len+1)+4)/4)*4;
	cmdp->cmdseq = revno;
	cmdp->cmdrval = expectedrc;
	cmdp->cmdact = action;
	cmdp->cmdpid = pid;
	cmdp->cmdtim = tstamp;

	SaveCmdInHistDB(cmdp);

	if(cli_debug)
	{
		CLIPRINTF((stdout, "posting: [%s] [seq = %ld] [rc = %d] [action = %d]\n", 
			cmd+sizeof(Cmd), revno, expectedrc, action));
	}

	if (CliSendToRs(cmdp) > 0)
	{
		CLIPRINTF((stdout, "posted: [%s] [rc = %d] [action = %d]\n", 
			cmd+sizeof(Cmd), expectedrc, action));
	}
}

void
CliPostCmdline(int cmdtype, char *cmdprefix, int argc, char **argv, long revno, 
		int expectedrc, int action)
{
	char cmdrest[1024] = { 0 };
	int len = 0, max = 1020;
	char *p;

	for (argc; argc; argc--)
	{
		p = strdup(argv[0]);
		len += snprintf(cmdrest+len, max-len, " ");
		len += snprintf(cmdrest+len, max-len, "\"%s\"", getfilename(p, "/"));
		free(p);
		argv ++;
	}

	CliPostCmd(cmdtype, cmdprefix, cmdrest, revno, expectedrc, action);
}

/* Check if slave, if in a redundant cluster.*/
int
IsSlave(unsigned int *ipaddr, unsigned int *port)
{

	int flag = 0; 				/* flag = 1 if slave */
	int i;

  	// attach to the cache
 	if (RSDConfig)
	{	
		if (CacheAttach() > 0 )
  		{
    		// lock the mutex
    		if (LockGetLock(&(lsMem->rsdmutex), LOCK_READ, LOCK_BLOCK) == AL_OK) 
			{
				// If Slave, send the command to Master, else process
				// First entry returned, corresponds to self.
				if ((lsMem->rsdInfo->records[0].status == RS_SLAVE) && (send2Rs != 0))
				{
					// IsSlave is True
					// get the ipaddr for the master
					for (i = 1; i < lsMem->rsdInfo->count; i++)
					{
						if (lsMem->rsdInfo->records[i].status == RS_MASTER)
						{
							*ipaddr = lsMem->rsdInfo->records[i].ipaddr;
							*port = lsMem->rsdInfo->records[i].port;
						}	
						flag = 1;
					}
				}
				
				// unlock the mutex
				LockReleaseLock(&(lsMem->rsdmutex));
				
			}
			else
				flag = 0;

   	 		// detach from the cache
   	 		CacheDetach();
  		}
	}
	else
		flag = 0;

	return flag;

}

/* Send the cli command to Master if slave in redundant configuration.*/
int
CliSendToDBMaster(unsigned int ipaddr, unsigned int port, int argc, char **argv, char *msg)
{

	int sockfd;
	struct sockaddr_in priAddr;
	char sendline[RS_LINELEN], recvline[RS_MSGLEN];
	char fn[] = "CliSendToDBMaster";
	int len, n, respStat = 0;
    char numArgs[256];
	char cmdrest[1024] = { 0 }, cmd[1024] = { 0 };
	int max = 1020, status = -xleUndefined;
    char tmpVal[64], *ptr;	
	fd_set rset;
	int maxfdp1;
	struct timeval tVal; 

	//set the timeout for 100 msecs.
	tVal.tv_sec = 0;
	tVal.tv_usec = 100000;
	

	NETDEBUG(MRSD, NETLOG_DEBUG2, ("Process Command called\n"));

	bzero(&priAddr, sizeof(priAddr));
	FD_ZERO(&rset);

	n = sprintf(numArgs, "%d", argc);
	len = 0;

	for (argc; argc; argc--)
	{
		len += snprintf(cmd+len, max-len, " ");
		len += snprintf(cmd+len, max-len, "%s", getfilename(argv[0], "/"));
		argv ++;
	}

	/* If redundant configuration, slave should send cli command to the master.*/
				
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		NETERROR(MCLI,("%s: can not create socket\n",fn));
		return -1;				
	}

	// send the cli command to the TCP Server.
	priAddr.sin_family = AF_INET;
	priAddr.sin_addr.s_addr = htonl(ipaddr);
	priAddr.sin_port = htons(port);

	if ((connect(sockfd, (struct sockaddr *)&priAddr, sizeof(priAddr))) == -1) 
	{
		NETERROR(MCLI,("%s: can not connect to DBMaster.\n", fn));
		close(sockfd);
		return -1;
	}

	len = 0;

	len += snprintf(sendline + len, RS_LINELEN - len, CLI_CMD_FRM_SLAVE);
	len += snprintf(sendline + len, RS_LINELEN - len, " \n");    /* receiver expects newline */

	write(sockfd, sendline, len);
	NETDEBUG(MCLI, NETLOG_DEBUG4, ("Sending Request to DBMaster - %s", sendline));

	len = 0;

	len += snprintf(sendline + len, RS_LINELEN - len, cmd);
	len += snprintf(sendline + len, RS_LINELEN- len, " \n");

	if (write(sockfd, sendline, len) < 0) 
	{
		NETERROR(MCLI,("%s: Write to socket failed.\n", fn));
		close(sockfd);
		return -1;
	}

	n = 0;

	while (1) {
		// Read the status
		FD_SET(sockfd, &rset);
		maxfdp1 = sockfd + 1;
		select(maxfdp1, &rset, NULL, NULL, &tVal);

		if (FD_ISSET(sockfd, &rset))
		{
			/* socket is readable */
			if (!respStat) {
				respStat = 1;
				n = read(sockfd, recvline, RS_LINELEN);   /* Block on Read */
				if (n < 0) {
					NETERROR(MCLI,("%s: Read from socket failed.\n", fn));
					status = -xleUndefined;
					goto _return;
				}
				status = atoi(recvline);
	
				for (len = n, ptr = recvline; (*ptr != '\n') && (len > 0); ptr++, len--) {
				}
	
				if (len > 0) {
					memcpy(msg, ptr + 1, --len); 	/* skip over the \n we just found */
				}
			}
	
			// Read the message
			n = read(sockfd, msg + len, RS_MSGLEN - len); 
			if (n < 0) 
			{
				/* We got a response back */
				/* 	Command processed by master. */
				NETERROR(MCLI,("%s: Read from socket failed.\n", fn));
				status = -xleUndefined;
				break;
			}
			len += n;
		}
		else {
			NETDEBUG(MCLI, NETLOG_DEBUG4, ("%s: timed out waiting for read on socket. \n", fn));
			break;
		}
	}

_return:
	close(sockfd);
	return status;
}

// returns a new revision number
int
CliUpdateRev(void)
{
	if (saveindb == 0)
	{
		return -1;
	}

	return GetCliSeqNum();
}

// Returns -1 if failed to save the cmd in the database
int
SaveCmdInHistDB(Cmd *cmdp)
{
	int 	seqnum_fd;
	long	seqnum;
	HDB		*hdbp;
	CliEntry cli_ent;
	int		rval = 0;

	hdbp = OpenCliHist();

	cli_ent.seqno = cmdp->cmdseq;
	cli_ent.cmdlen = cmdp->cmdlen;
	cli_ent.clicmd = (char *)cmdp;

	rval = StoreCliEntry(hdbp, &cli_ent);

	CloseCliHist(hdbp);

	return(rval);
}

// Utility functions for interactive cli
int
ProcessCommandLine(FILE *in, char *commandbuffer, int buflen)
{
    int rc = 1;
	
    rc = read_command(in, commandbuffer, buflen);

    return rc;
}

int
PrintCommandPrompt(FILE *out, char *prompt)
{
    int error, len = sizeof(int);

    fprintf(out, "%s", prompt);
    fflush(out);
    return(0);
}

int
read_command(FILE *in, char *buffer, int buflen)
{
    char fn[] = "read_command():";
    int error;
    char *retval;

    retval = fgets(buffer, buflen, in);

    if (retval == NULL)
    {
        /* check for errors */
        if (errno != 0)
        {
            fprintf(stderr, "%s encountered error %d\n", fn, errno);
            return 0;
        }
        return 0;
    }
    else
    {
        return 1;
    }
}

int
CliHandleInteractive(char *prompt, char *buffer, int buflen, 
	int *argc, char **argv, int maxargv)
{
	int rc;
	char *tmp;

	if (cli_ix == 0)
	{
		// Not interactive
		return 0;
	}

	PrintCommandPrompt(stdout, prompt);

	rc = ProcessCommandLine(stdin, buffer, buflen);
	tmp = strdup(buffer);

	// extract argc/argv pairs and assign them
    if (rc)
    {
        rc = ExtractArgs(buffer, argc, argv, 64);
    }

	if (*argc > 0)
	{
		// see if its a history command
		if ((*argc == 1) && (argv[0][0] == '!'))
		{
			free(tmp);
			tmp = CliCmdHistoryGet(atoi(&argv[0][1]));

			if (tmp)
			{
				strcpy(buffer, tmp);
        		rc = ExtractArgs(buffer, argc, argv, 64);
			}
			else
			{
				*argc = 0;
			}
		}
		else
		{
			CliCmdHistoryAdd(tmp);
		}
	}

	if ((*argc == 1) && HISTORY_CMD(argv[0]))
	{
		CliCmdHistoryPrint(stdout);
		*argc = 0;
		return 1;
	}

	if ((*argc == 1) && EXIT_CMD(argv[0]))
	{
		return 0;
	}

	return 1;
}

#define MAX_HIST		500
#define MAX_HIST_SHOW	100
static List commands = NULL;
int command_base = 0;

void
CliCmdHistoryInit(void)
{
	commands = listInit();
}

// We will use a static List, as
// this will work only from actuall command
// line, and not a mult thread application
int
CliCmdHistoryAdd(char *cmd)
{
	listAddItem(commands, (cmd));

	if (commands->head->nitems > MAX_HIST)
	{
		listDeleteItem(commands, listGetFirstItem(commands));
		command_base ++;
	}

	return 0;
}

char *
CliCmdHistoryGet(int i)
{
	return ListGetItem(commands, i-command_base);	
}

void
CliCmdHistoryPrint(FILE *out)
{
	void *item;
	int i = 0;
	int n = commands->head->nitems;

	for (item = listGetFirstItem(commands);item;
			item = listGetNextItem(commands, item))
	{
		if ((n-i) > MAX_HIST_SHOW) continue;
		fprintf(out, " %3.3d %s", ++i+command_base, (char *)item);
	}
}

// Mechanism to implement a db server approach
// implemented in server/dbs.c in a single thread environment
// The two techniques will be merged later into one api
// to be able to work in both environments
int
CliDbQueueEntry(CacheEntry **centryListPtr, int type, void *entry, int sz)
{
	CacheEntry *centry = NULL;
	void *tmp = NULL;

	if (entry == NULL)
	{
		// Not allowed
		CLIPRINTF((stdout, "NULL Entry present in save list\n"));
		return -1;
	}

	centry = (CacheEntry *)malloc(sizeof(CacheEntry));
	if (centry == NULL)
	{
		CLIPRINTF((stdout, "malloc failed\n"));
		goto _error;
	}

	// Duplicate the entry
	tmp = malloc(sz);
	if (tmp == NULL)
	{
		CLIPRINTF((stdout, "malloc failed\n"));
		goto _error;
	}

	memcpy(tmp, entry, sz);
	entry = tmp;

	memset(centry, 0, sizeof(CacheEntry));
	centry->type = type;
	centry->entry = entry;

	ListInitElem(centry);

	// Add into the List
	if (*centryListPtr)
	{
		ListInsert(*centryListPtr, centry);	
	}
	else
	{
		*centryListPtr = centry;
	}

	return 0;

_error:
	if (tmp) free(tmp);
	if (centry) free(centry);
	return -1;
}

int
CliDbQueueSave(CacheEntry *centryList)
{
	CacheEntry *centry, *tmp;
	NetoidSNKey *key;
	CacheTableInfo cacheInfoEntry;

	if (centryList == NULL)
	{
		return 0;
	}

	// Walk the list and save the entries
	centry = centryList;	

	// There is at least ONE element in the list
	do
	{
		switch (centry->type)
		{
		case CACHE_INFO_ENTRY:
			if (centry->entry == NULL)
			{
				CLIPRINTF((stdout, "NULL Entry present in save list\n"));
				break;
			}

			key = (NetoidSNKey *)centry->entry;

			// Look up the entry in the cache
			if (CacheFind(regCache, key, &cacheInfoEntry, sizeof(cacheInfoEntry)) >= 0)
			{
				// Update the db
				UpdateNetoidDatabase(&cacheInfoEntry.data);
			}
			
			break;

		default:
			CLIPRINTF((stdout, 
				"undefined entry prsent in save list %d\n", centry->type));
			break;
		}

		// Free contents
		if (centry->entry) free(centry->entry);

		// proceed to next element logic
		// We will delete the current element and proceed
		// on, if there are more

		// Save element to be freed later
		tmp = centry;

		// proceed logic
		if (ListIsSingle(centry))
		{
			// We must stop, this is the last elt
			centry = NULL;
		}
		else
		{
			// We must proceed
			centry = centry->next;
		}

		// Delete+Free tmp, before moving on
		ListDelete(tmp);	
		free(tmp);

	} while (centry);

	return 0;
}

int
CliDeleteTriggerRoutes(char *trname)
{
	char fn[] = "CliDeleteTriggerRoutes():";
    DB db;
	DB_tDb dbstruct;
    VpnRouteEntry *routeEntry,*lastRouteEntry;
    VpnRouteKey *key, *okey;
    int keylen = sizeof(VpnRouteKey), n = 0;

	// Walk the route database, 
	// delete routes which match the trigger

	dbstruct.read_write = GDBM_WRITER;

	if (!(db = DbOpenByID(CALLROUTE_DB_FILE, DB_eCallRoute, &dbstruct)))
	{
    	NETERROR(MDB, ("%s Unable to open database %s\n", 
			fn, CALLROUTE_DB_FILE));
		return -1;
	}

   	for (key = (VpnRouteKey *)DbGetFirstKey(db); key != 0; 
	 		key = (VpnRouteKey *)DbGetNextKey(db, (char *)key, keylen),
	      	free(okey), free(routeEntry))
   	{
		/* Get the info entry */
	 	routeEntry = (VpnRouteEntry *)DbFindEntry(db, (char *)key, keylen);

	 	if (routeEntry == NULL)
	 	{
			NETERROR(MDB, ("%s Db Format Error !!\n", fn));
	 	}

	 	okey = key;
	
		if (routeEntry && !strcmp(routeEntry->trname, trname))
		{
			n ++;

			// Delete entry from database
			if (DbDeleteEntry(db, (char *)key, sizeof(VpnRouteKey)) < 0)
			{
				NETERROR(MDB, ("%s Could not delete route from db: %s\n", 
					fn, key->crname));
			}

			/* lock the cache */
			CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
			CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

			if (DeleteRoute(routeEntry) < 0)
			{
				NETERROR(MDB, ("%s Could not delete route from cache: %s\n", 
					fn, routeEntry->crname));
			}

			CacheReleaseLocks(cpbCache);
			CacheReleaseLocks(cpCache);

		}
   	}

   	DbClose(&dbstruct);

	return n;
}

static int IsAllNumStr(char *str)
{
  if (!str) return 0;

  while(*str)
  {
    if (!isdigit(*str++))
      return 0;
  }

  return 1;
}
