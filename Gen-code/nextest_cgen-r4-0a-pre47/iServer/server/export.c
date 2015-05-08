#include <stdio.h>
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

#ifdef ALLOW_ISERVER_SIP
#include "sipaddr.h"
#endif
#include "callsm.h"
#include "export.h"

/********************************************************************
 * Db Export utilities, called from CLI
 ********************************************************************/
int
ExportInfoEntry(FILE *exportf, InfoEntry *infoEntry, ClientAttribs *clAttribs)
{
	struct in_addr in;
        char buf[INET_ADDRSTRLEN];

	fprintf(exportf, "<E>\n");
  	if (BIT_TEST(infoEntry->sflags, ISSET_REGID))
   	{
	  	fprintf(exportf, 
			"<SRNO><![CDATA[ \"%s\" ]]></SRNO>\n", infoEntry->regid);
   	}

   	if (BIT_TEST(infoEntry->sflags, ISSET_UPORT))
   	{
		fprintf(exportf, "<UPORT><![CDATA[ \"%lu\" ]]></UPORT>\n", 
			infoEntry->uport);
   	}


   	if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
   	{
		fprintf(exportf, "<PHONE><![CDATA[ \"%s\" ]]></PHONE>\n",
			  infoEntry->phone);
   	}
     
	in.s_addr = htonl(infoEntry->ipaddress.l);
   	if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_IPADDRESS))
   	{
		fprintf(exportf, "<IPADDR><![CDATA[ \"%s\" ]]></IPADDR>\n",
			  inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
    }
     
	in.s_addr = htonl(infoEntry->subnetip);
   	if (infoEntry->subnetip)
   	{
		fprintf(exportf, "<SUBNETIP><![CDATA[ \"%s\" ]]></SUBNETIP>\n",
			  inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
    }
     
	in.s_addr = htonl(infoEntry->subnetmask);
   	if (infoEntry->subnetmask)
   	{
		fprintf(exportf, "<SUBNETMASK><![CDATA[ \"%s\" ]]></SUBNETMASK>\n",
			  inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
    }
     
    if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_VPNPHONE))
    {
		fprintf(exportf, "<VPN_PHONE><![CDATA[ \"%s\" ]]></VPN_PHONE>\n",
			  infoEntry->vpnPhone); 
		fprintf(exportf, "<VPN_EXTLEN><![CDATA[ \"%lu\" ]]></VPN_EXTLEN>\n",
			infoEntry->vpnExtLen);
    }

	if (infoEntry->vpnName[0])
	{
		fprintf(exportf, "<VPN_NAME><![CDATA[ \"%s\" ]]></VPN_NAME>\n",
			infoEntry->vpnName);
	}

	if (infoEntry->h323id[0])
	{
		fprintf(exportf, "<H323ID><![CDATA[ \"%s\" ]]></H323ID>\n",
			infoEntry->h323id);
	}

	if (infoEntry->pgkid[0])
	{
		fprintf(exportf, "<PGKID><![CDATA[ \"%s\" ]]></PGKID>\n",
			infoEntry->pgkid);
	}

	if (infoEntry->techprefix[0])
	{
		fprintf(exportf, "<TECHP><![CDATA[ \"%s\" ]]></TECHP>\n",
			infoEntry->techprefix);
	}

	if (infoEntry->uri[0])
	{
		fprintf(exportf, "<URI><![CDATA[ \"%s\" ]]></URI>\n",
			infoEntry->uri);
	}

	/* new CAP tag is in network order, the old tag used to be in host order */
	fprintf(exportf, "<CAP2><![CDATA[ \"%u\" ]]></CAP2>\n",
			htons(infoEntry->cap));

	fprintf(exportf, "<VENDOR><![CDATA[ \"%u\" ]]></VENDOR>\n",
			infoEntry->vendor);

	if (infoEntry->passwd[0])
	{
		fprintf(exportf, "<PASSWORD> <![CDATA[\"%s\"]]></PASSWORD>\n",
				infoEntry->passwd);
	}

	if (infoEntry->contact[0])
	{
		fprintf(exportf, "<CONTACT><![CDATA[ \"%s\"]]> </CONTACT>\n",
			infoEntry->contact);
	}

	in.s_addr = htonl(infoEntry->rasip);
	fprintf(exportf, "<RASIP><![CDATA[ \"%s\" ]]></RASIP>\n",
		  inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));

    fprintf(exportf, "<RASPORT><![CDATA[ \"%u\" ]]></RASPORT>\n",
		infoEntry->rasport); 

    fprintf(exportf, "<SIGPORT><![CDATA[ \"%u\" ]]></SIGPORT>\n",
		infoEntry->callsigport); 

	fprintf(exportf, "<ITIME><![CDATA[ \"%lu\" ]]></ITIME>\n",
		infoEntry->iTime);

	fprintf(exportf, "<MTIME><![CDATA[ \"%lu\" ]]></MTIME>\n",
		infoEntry->mTime);

	fprintf(exportf, "<RTIME><![CDATA[ \"%lu\" ]]></RTIME>\n",
		infoEntry->rTime);

	if (infoEntry->stateFlags & CL_FORWARD)
	{
		fprintf(exportf, "<FW> </FW>\n");
		if (infoEntry->protocol == NEXTONE_REDIRECT_ROLLOVER)
		{
			fprintf(exportf, "<FP><![CDATA[ \"ROLLOVER\" ]]></FP>\n");
		}
	}

	if (infoEntry->stateFlags & CL_PROXY)
	{
		fprintf(exportf, "<PXS> </PXS>\n");
	}

	if (infoEntry->stateFlags & CL_PROXYING)
	{
		fprintf(exportf, "<PXC> </PXC>\n");
	}

#if 0
	if ((infoEntry->stateFlags & CL_PROXY) ||
		(infoEntry->stateFlags & CL_PROXYING))
	{
		if (strlen(infoEntry->xphone))
		{
			fprintf(exportf, "<XPHONE><![CDATA[ \"%s\" ]]></XPHONE>\n",
				infoEntry->xphone);	
		}

		if (strlen(infoEntry->xvpnPhone))
		{
			fprintf(exportf, "<XVPN_PHONE><![CDATA[ \"%s\" ]]></XVPN_PHONE>\n",
				infoEntry->xvpnPhone);
		}
	}
#endif

    if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE))
    {
		fprintf(exportf, "<NPHONE><![CDATA[ \"%s\" ]]></NPHONE>\n",
			  infoEntry->nphone);
    }
     
    if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE))
    {
		  fprintf(exportf, "<NVPN_PHONE><![CDATA[ \"%s\" ]]></NVPN_PHONE>\n",
			  infoEntry->nvpnPhone); 
		  fprintf(exportf, "<NVPN_EXTLEN><![CDATA[ \"%lu\" ]]></NVPN_EXTLEN>\n",
			infoEntry->nvpnExtLen);
	}

    if (infoEntry->stateFlags & CL_DND)
	{
		fprintf(exportf, "<DND></DND>\n");
	}

	if (infoEntry->stateFlags & CL_STATIC)
	{
		 fprintf(exportf, "<STATIC></STATIC>\n");
	}

	if (infoEntry->stateFlags & CL_DYNAMIC)
	{
		 fprintf(exportf, "<DYNOBP></DYNOBP>\n");
	}

	fprintf(exportf, "<SFLAGS><![CDATA[ \"%lu\" ]]></SFLAGS>\n",
		infoEntry->stateFlags);

	fprintf(exportf, "<ET><![CDATA[ \"%s\" ]]></ET>\n",
		IedgeName(infoEntry->ispcorgw));

    if (infoEntry->zone[0])
    {
		fprintf(exportf, "<ZONE><![CDATA[ \"%s\" ]]></ZONE>\n", infoEntry->zone);
    }

    if (infoEntry->email[0])
    {
		fprintf(exportf, "<EMAIL><![CDATA[ \"%s\" ]]></EMAIL>\n", infoEntry->email);
    }

	if (infoEntry->cpname[0])
	{
		fprintf(exportf, "<CP_NAME><![CDATA[ \"%s\" ]]></CP_NAME>\n", 
			infoEntry->cpname);
	}

	if (IedgeXCalls(infoEntry))
	{
		fprintf(exportf, "<XCALLS><![CDATA[ \"%u\" ]]></XCALLS>\n", 
			IedgeXCalls(infoEntry));
	}
	fprintf(exportf, "<INFOTRANSCAP><![CDATA[ \"%u\" ]]></INFOTRANSCAP>\n", 
			IedgeInfoTransCap(infoEntry));

	if (IedgeXInCalls(infoEntry))
	{
		fprintf(exportf, "<XINCALLS><![CDATA[ \"%u\" ]]></XINCALLS>\n", 
			IedgeXInCalls(infoEntry));
	}

	if (IedgeXOutCalls(infoEntry))
	{
		fprintf(exportf, "<XOUTCALLS><![CDATA[ \"%u\" ]]></XOUTCALLS>\n", 
			IedgeXOutCalls(infoEntry));
	}

	if (infoEntry->maxHunts)
	{
		fprintf(exportf, "<MAXHUNTS><![CDATA[ \"%u\" ]]></MAXHUNTS>\n", 
			infoEntry->maxHunts);
	}

	if (infoEntry->priority)
	{
		fprintf(exportf, "<PRIO><![CDATA[ \"%u\" ]]></PRIO>\n", infoEntry->priority);
	}

	if(BIT_TEST(infoEntry->sflags, ISSET_NATIP))
	{
		in.s_addr = htonl(infoEntry->natIp);
		fprintf(exportf, "<NATIP><![CDATA[ \"%s\" ]]></NATIP>\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
	}

	if(BIT_TEST(infoEntry->sflags, ISSET_NATPORT))
	{
		fprintf(exportf, "<NATPORT><![CDATA[ \"%u\" ]]></NATPORT>\n", infoEntry->natPort);
	}

	if (clAttribs)
	{
		fprintf(exportf, "<FNAME><![CDATA[ \"%s\" ]]></FNAME>\n", clAttribs->clFname);
		fprintf(exportf, "<LNAME><![CDATA[ \"%s\" ]]></LNAME>\n", clAttribs->clLname);
		fprintf(exportf, "<LOCATION><![CDATA[ \"%s\" ]]></LOCATION>\n", clAttribs->clLoc);
		fprintf(exportf, "<COUNTRY><![CDATA[ \"%s\" ]]></COUNTRY>\n", clAttribs->clCountry);
		fprintf(exportf, "<COMMENTS><![CDATA[ \"%s\" ]]></COMMENTS>\n", clAttribs->clComments);
	}
	fprintf(exportf, "<ECAPS1><![CDATA[ \"%u\" ]]></ECAPS1>\n", infoEntry->ecaps1);
	fprintf(exportf, "<CRID><![CDATA[ \"%d\" ]]></CRID>\n", 
		infoEntry->crId);

	if (infoEntry->custID[0])
	{
		fprintf(exportf, "<CUSTID><![CDATA[ \"%s\" ]]></CUSTID>\n", 
			infoEntry->custID);
	}
	
	if (infoEntry->tg[0])
	{
		fprintf(exportf, "<TG><![CDATA[ \"%s\" ]]></TG>\n", 
			infoEntry->tg);
	}

	if (infoEntry->srcIngressTG[0])
	{
		fprintf(exportf, "<SRCITG><![CDATA[ \"%s\" ]]></SRCITG>\n", 
			infoEntry->srcIngressTG);
	}

	if (infoEntry->dtg[0])
	{
		fprintf(exportf, "<DTG><![CDATA[ \"%s\" ]]></DTG>\n", 
			infoEntry->dtg);
	}

	if (infoEntry->srcEgressTG[0])
	{
		fprintf(exportf, "<SRCETG><![CDATA[ \"%s\" ]]></SRCETG>\n", 
			infoEntry->srcEgressTG);
	}

	fprintf(exportf, "<CDPNTYPE><![CDATA[ \"%u\" ]]></CDPNTYPE>\n",
			infoEntry->q931IE[Q931IE_CDPN]);
	fprintf(exportf, "<CGPNTYPE><![CDATA[ \"%u\" ]]></CGPNTYPE>\n",
			infoEntry->q931IE[Q931IE_CGPN]);
	fprintf(exportf, "<BCAP><![CDATA[ \"%u\" ]]></BCAP>\n",
			*(unsigned int *)infoEntry->bcap);

	if (infoEntry->ogprefix[0])
	{
		fprintf(exportf, "<OGP><![CDATA[ \"%s\" ]]></OGP>\n", 
			infoEntry->ogprefix);
	}

	if (infoEntry->realmName[0])
	{
		fprintf(exportf, "<REALM><![CDATA[ \"%s\" ]]></REALM>\n", 
			infoEntry->realmName);
		fprintf(exportf, "<REALMID><![CDATA[ \"%d\" ]]></REALMID>\n", 
			infoEntry->realmId);
	}

	if (infoEntry->igrpName[0])
	{
		fprintf(exportf, "<IGROUP><![CDATA[ \"%s\" ]]></IGROUP>\n", 
			infoEntry->igrpName);
	}

	fprintf(exportf, "<CIDBLK><![CDATA[ \"%d\" ]]></CIDBLK>\n", 
		infoEntry->cidblock);
		

	fprintf(exportf, "</E>\n\n");
	return(0);
}

int
ExportVpnEntry(FILE *exportf, VpnEntry *vpnEntry)
{
	if (vpnEntry->vpnId[0] == '\0')
	{
		return 0;
	}
	
	fprintf(exportf, "<V>\n");

	fprintf(exportf, "<VPN_NAME><![CDATA[ \"%s\" ]]></VPN_NAME>\n",
		vpnEntry->vpnName);
		
	fprintf(exportf, "<VPN_ID><![CDATA[ \"%s\" ]]></VPN_ID>\n",
		vpnEntry->vpnId);

	fprintf(exportf, "<VPN_EXTLEN><![CDATA[ \"%u\" ]]></VPN_EXTLEN>\n",
		vpnEntry->vpnExtLen);
		
	fprintf(exportf, "<VPN_GROUP><![CDATA[ \"%s\" ]]></VPN_GROUP>\n",
		vpnEntry->vpnGroup);
		
	fprintf(exportf, "<VPN_PREFIX><![CDATA[ \"%s\" ]]></VPN_PREFIX>\n",
		vpnEntry->prefix);

	fprintf(exportf, "<VPN_SIPDOMAIN><![CDATA[ \"%s\" ]]></VPN_SIPDOMAIN>\n",
		vpnEntry->sipdomain);

	fprintf(exportf, "<VPN_LOC><![CDATA[ \"%s\" ]]></VPN_LOC>\n",
		vpnEntry->vpnLoc);
		
	fprintf(exportf, "<VPN_CONTACT><![CDATA[ \"%s\" ]]></VPN_CONTACT>\n",
		vpnEntry->vpnContact);
		
	fprintf(exportf, "<MTIME><![CDATA[ \"%lu\" ]]></MTIME>\n",
		vpnEntry->mTime);

	fprintf(exportf, "</V>\n\n");
	return(1);
}

int
ExportVpnGEntry(FILE *exportf, VpnGroupEntry *vpnGroupEntry)
{
	if (vpnGroupEntry->vpnGroup[0] == '\0')
	{
		return 0;
	}
	
	fprintf(exportf, "<VG>\n");

	fprintf(exportf, "<VPN_GROUP><![CDATA[ \"%s\" ]]></VPN_GROUP>\n",
		vpnGroupEntry->vpnGroup);

	fprintf(exportf, "<MTIME><![CDATA[ \"%lu\" ]]></MTIME>\n",
		vpnGroupEntry->mTime);

	fprintf(exportf, "</VG>\n\n");
	return(1);
}

int
ExportCPEntry(FILE *stream, CallPlanEntry *entry)
{
	fprintf(stream, "<CP>\n");
	fprintf(stream, "<CP_NAME><![CDATA[ \"%s\" ]]></CP_NAME>\n", entry->cpname);
	fprintf(stream, "<VPN_GROUP><![CDATA[ \"%s\" ]]></VPN_GROUP>\n", entry->vpnGroup);

    fprintf(stream, "<MTIME><![CDATA[ \"%lu\" ]]></MTIME>\n", entry->mTime);

	fprintf(stream, "</CP>\n\n");
	return(0);
}

int
ExportCREntry(FILE *stream, VpnRouteEntry *entry)
{
	if(entry->trname[0] != '\0')
	{
		// if it's a trigger based route, use a different tag
		fprintf(stream, "<DCR>\n");
	}
	else
	{
		fprintf(stream, "<CR>\n");
	}

	fprintf(stream, "<CR_NAME><![CDATA[ \"%s\" ]]></CR_NAME>\n", entry->crname);
	fprintf(stream, "<CR_SRC><![CDATA[ \"%s\" ]]></CR_SRC>\n", entry->src);
	fprintf(stream, "<CR_SRCLEN><![CDATA[ \"%d\" ]]></CR_SRCLEN>\n", entry->srclen);
	fprintf(stream, "<CR_SRCPREFIX><![CDATA[ \"%s\" ]]></CR_SRCPREFIX>\n", entry->srcprefix);
	fprintf(stream, "<CR_DEST><![CDATA[ \"%s\" ]]></CR_DEST>\n", entry->dest);
	fprintf(stream, "<CR_DESTLEN><![CDATA[ \"%d\" ]]></CR_DESTLEN>\n", entry->destlen);
	fprintf(stream, "<CR_PREFIX><![CDATA[ \"%s\" ]]></CR_PREFIX>\n", entry->prefix);
	fprintf(stream, "<CR_FLAGS><![CDATA[ \"%d\" ]]></CR_FLAGS>\n", entry->crflags);
	fprintf(stream, "<CR_CPNAME><![CDATA[ \"%s\" ]]></CR_CPNAME>\n", entry->cpname);
	fprintf(stream, "<CR_TRNAME><![CDATA[ \"%s\" ]]></CR_TRNAME>\n", entry->trname);

   	fprintf(stream, "<MTIME><![CDATA[ \"%lu\" ]]></MTIME>\n", entry->mTime);

	if(entry->trname[0] != '\0')
	{
		fprintf(stream, "</DCR>\n\n");
	}
	else
	{
		fprintf(stream, "</CR>\n\n");
	}
	return(0);
}

int
ExportRealmEntry(FILE *stream, RealmEntry *entry)
{
	struct in_addr in;
        char buf[INET_ADDRSTRLEN];

	fprintf(stream, "<RM>\n");

   	fprintf(stream, "<RM_ID><![CDATA[ \"%lu\" ]]></RM_ID>\n", entry->realmId);
	fprintf(stream, "<RM_NAME><![CDATA[ \"%s\" ]]></RM_NAME>\n", entry->realmName);

	in.s_addr = htonl(entry->rsa);
	fprintf(stream, "<RM_RSA><![CDATA[ \"%s\" ]]></RM_RSA>\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));

	in.s_addr = htonl(entry->mask);
	fprintf(stream, "<RM_MASK><![CDATA[ \"%s\" ]]></RM_MASK>\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));

	fprintf(stream, "<RM_SIGPOOL><![CDATA[ \"%u\" ]]></RM_SIGPOOL>\n", entry->sigPoolId);
	fprintf(stream, "<RM_MEDPOOL><![CDATA[ \"%u\" ]]></RM_MEDPOOL>\n", entry->medPoolId);
	//fprintf(stream, "<RM_IFNAME><![CDATA[ \"%s\" ]]></RM_IFNAME>\n", entry->ifName);
	fprintf(stream, "<RM_ASTATUS><![CDATA[ \"%u\" ]]></RM_ASTATUS>\n", entry->adminStatus);
	fprintf(stream, "<RM_OSTATUS><![CDATA[ \"%u\" ]]></RM_OSTATUS>\n", entry->operStatus);
	fprintf(stream, "<RM_ADDR><![CDATA[ \"%u\" ]]></RM_ADDR>\n", entry->addrType);
	fprintf(stream, "<RM_IMR><![CDATA[ \"%u\" ]]></RM_IMR>\n", entry->interRealm_mr);
	fprintf(stream, "<RM_EMR><![CDATA[ \"%u\" ]]></RM_EMR>\n", entry->intraRealm_mr);
	fprintf(stream, "<RM_AUTHFLAGS><![CDATA[ \"%u\" ]]></RM_AUTHFLAGS>\n", entry->authFlags);
	fprintf(stream, "<RM_CIDBLK_PREFIX><![CDATA[ \"%s\"]]></RM_CIDBLK_PREFIX>\n",entry->cidblk);
	fprintf(stream, "<RM_CIDUNBLK_PREFIX><![CDATA[ \"%s\"]]></RM_CIDUNBLK_PREFIX>\n",entry->cidunblk);
	fprintf(stream, "<RM_PROXY_REGID><![CDATA[\"%s\"]]></RM_PROXY_REGID>\n",entry->mp.regid);
	fprintf(stream, "<RM_PROXY_UPORT><![CDATA[\"%lu\"]]></RM_PROXY_UPORT>\n",entry->mp.uport);
	fprintf(stream, "<RM_VNETNAME><![CDATA[ \"%s\" ]]></RM_VNETNAME>\n", entry->vnetName);

	fprintf(stream, "</RM>\n\n");
	return(0);
}

int
ExportIgrpEntry(FILE *stream, IgrpInfo *entry)
{
	fprintf(stream, "<IGRP>\n");

	fprintf(stream, "<IGRP_NAME><![CDATA[ \"%s\" ]]></IGRP_NAME>\n", entry->igrpName);
	fprintf(stream, "<IGRP_MAXIN><![CDATA[ \"%u\" ]]></IGRP_MAXIN>\n", IgrpXInCalls(entry));
	fprintf(stream, "<IGRP_MAXOUT><![CDATA[ \"%u\" ]]></IGRP_MAXOUT>\n", IgrpXOutCalls(entry));
	fprintf(stream, "<IGRP_MAXTOTAL><![CDATA[ \"%u\" ]]></IGRP_MAXTOTAL>\n", IgrpXCallsTotal(entry));

	fprintf(stream, "<IGRP_IN><![CDATA[ \"%u\" ]]></IGRP_IN>\n", 0);
	fprintf(stream, "<IGRP_OUT><![CDATA[ \"%u\" ]]></IGRP_OUT>\n", 0);
	fprintf(stream, "<IGRP_TOTAL><![CDATA[ \"%u\" ]]></IGRP_TOTAL>\n", 0);

	fprintf(stream, "<IGRP_TIME><![CDATA[ \"%lu\" ]]></IGRP_TIME>\n", entry->dndTime);
	fprintf(stream, "</IGRP>\n\n");
	return(0);
}

int
ExportVnetEntry(FILE *stream, VnetEntry *entry)
{
	struct in_addr in;
        char buf[INET_ADDRSTRLEN];

	fprintf(stream, "<VNET>\n");

	fprintf(stream, "<VNET_VNETNAME><![CDATA[ \"%s\" ]]></VNET_VNETNAME>\n", entry->vnetName);
	fprintf(stream, "<VNET_IFNAME><![CDATA[ \"%s\" ]]></VNET_IFNAME>\n", entry->ifName);
	fprintf(stream, "<VNET_VLANID><![CDATA[ \"%u\" ]]></VNET_VLANID>\n", entry->vlanid);
	fprintf(stream, "<VNET_RTGTBLID><![CDATA[ \"%u\" ]]></VNET_RTGTBLID>\n", entry->rtgTblId);

	in.s_addr = htonl(entry->gateway);
	fprintf(stream, "<VNET_GATEWAY><![CDATA[ \"%s\" ]]></VNET_GATEWAY>\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));

	fprintf(stream, "</VNET>\n\n");
	return(0);
}

int
ExportCPBEntry(FILE *stream, CallPlanBindEntry *entry)
{
	fprintf(stream, "<CPB>\n");

	fprintf(stream, "<CP_NAME><![CDATA[ \"%s\" ]]></CP_NAME>\n", entry->cpname);
	fprintf(stream, "<CR_NAME><![CDATA[ \"%s\" ]]></CR_NAME>\n", entry->crname);
	fprintf(stream, "<CR_FLAGS><![CDATA[ \"%d\" ]]></CR_FLAGS>\n", entry->crflags);

	fprintf(stream, "<CR_STIME><![CDATA[ \"%d/%d/%d/%d/%d/%d/%d/%d\" ]]></CR_STIME>\n",
			entry->sTime.tm_year,
			entry->sTime.tm_yday,
			entry->sTime.tm_wday,
            entry->sTime.tm_mon,
            entry->sTime.tm_mday,
            entry->sTime.tm_hour,
            entry->sTime.tm_min,
            entry->sTime.tm_sec);

	fprintf(stream, "<CR_FTIME><![CDATA[ \"%d/%d/%d/%d/%d/%d/%d/%d\" ]]></CR_FTIME>\n",
			entry->fTime.tm_year,
			entry->fTime.tm_yday,
			entry->fTime.tm_wday,
            entry->fTime.tm_mon,
            entry->fTime.tm_mday,
            entry->fTime.tm_hour,
            entry->fTime.tm_min,
            entry->fTime.tm_sec);

	fprintf(stream, "<PRIO><![CDATA[ \"%u\" ]]></PRIO>\n", entry->priority);
   	fprintf(stream, "<MTIME><![CDATA[ \"%lu\" ]]></MTIME>\n", entry->mTime);

	fprintf(stream, "</CPB>\n\n");
	return(0);
}

int
ExportTriggerEntry(FILE *stream, TriggerEntry *entry)
{
	fprintf(stream, "<TRG>\n");

	fprintf(stream, "<TRG_NAME><![CDATA[ \"%s\" ]]></TRG_NAME>\n", entry->name);
	fprintf(stream, "<TRG_EVT><![CDATA[ \"%d\" ]]></TRG_EVT>\n", entry->event);
	fprintf(stream, "<TRG_SVEND><![CDATA[ \"%d\" ]]></TRG_SVEND>\n", entry->srcvendor);
	fprintf(stream, "<TRG_DVEND><![CDATA[ \"%d\" ]]></TRG_DVEND>\n", entry->dstvendor);
	fprintf(stream, "<TRG_ACT><![CDATA[ \"%d\" ]]></TRG_ACT>\n", entry->action);
	fprintf(stream, "<TRG_ACTD><![CDATA[ \"%s\" ]]></TRG_ACTD>\n", entry->actiondata);
	fprintf(stream, "<TRG_ACTFLAGS><![CDATA[ \"%d\" ]]></TRG_ACTFLAGS>\n", entry->actionflags);

	fprintf(stream, "</TRG>\n\n");
	return(0);
}
