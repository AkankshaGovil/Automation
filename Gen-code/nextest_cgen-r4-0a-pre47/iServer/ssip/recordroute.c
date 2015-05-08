#include "bits.h"
#include "ipc.h"

#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"
#include "age.h"
#include "xmltags.h"

#include "gis.h"
#include <netdb.h>
#include <malloc.h>
#include "siputils.h"
#include "bcpt.h"
#include "sipclone.h"

SipBool 
SipInsertRecordRoute(SipMessage *m, SipError *err)
{
	char fn[] = "SipInsertRecordRoute():";
	SipHeader *header = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	header_url *req_uri = NULL;
	SIP_S8bit host[256];
	char port[10];
	char * tmpptr = NULL;
	SipParam *urlParam = NULL;

	if ((SipExtractReqUri(m, &req_uri, err)) == SipFail)
	{
		NETERROR(MSIP,("%s SipExtractReqUri returned Error [%d]\n", 
			       fn, *err));
		goto _error;
	}

	if ((sip_initSipHeader(&header, SipHdrTypeRecordRoute ,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise 'record-route' header %d\n",
				*err));
		goto _error;
	}

    if ((sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err))==SipFail)
    {
        NETERROR(MSIP, ("Could not initialise addr-spec in 'record-route' header %d\n", *err));
	goto _error;
    }

    if ((sip_initSipUrl(&sipurl, err))==SipFail)
    {
        NETERROR(MSIP, ("Could not initialise Sip Url %d\n", *err));
	goto _error;
    }

	strcpy(host, sipdomain);
	strcat(host, ":");
	sprintf(port, "%d", lSipPort);
	strcat(host, port);

	NETDEBUG(MSIP, NETLOG_DEBUG1, ("r-r header: host=%s\n",host));
	NETDEBUG(MSIP, NETLOG_DEBUG1, ("r-r header: user=%s\n",req_uri->name));

	tmpptr = strdup(req_uri->name);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup name for sip record-route host failed!\n"));
		goto _error;
	}

    if ((sip_setUserInUrl(sipurl, tmpptr, err))==SipFail)
    {
        NETERROR(MSIP, ("Could not set host in url of r-r header %d\n", 
				*err));
	goto _error;
    }

	tmpptr = strdup(host);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup host for sip record-route host failed!\n"));
		goto _error;
	}

    if ((sip_setHostInUrl(sipurl, tmpptr, err))==SipFail)
    {
        NETERROR(MSIP, ("Could not set host in url of r-r header %d\n", 
				*err));
	goto _error;
    }

	/* set the maddr param */
	if (sip_initSipParam(&urlParam, err) == SipFail)
	{
		NETERROR(MSIP, ("Could not init url param\n"));
		goto _error;
	}

	if (sip_setNameInSipParam(urlParam, strdup("maddr"), err) == SipFail)
	{
		NETERROR(MSIP, ("Could not set name of url param\n"));
		goto _error;
	}

	if (sip_insertValueAtIndexInSipParam(urlParam, strdup(sipdomain), 0, err) == SipFail)
	{
		NETERROR(MSIP, ("Could not set maddr in url param\n"));
		goto _error;
	}

	if (sip_insertUrlParamAtIndexInUrl(sipurl, urlParam, 0, err) == SipFail)
	{
		NETERROR(MSIP, ("Could not insert url param in url\n"));
		goto _error;
	}	

    if (sip_setUrlInAddrSpec(addrspec, sipurl, err)==SipFail)
    {
         NETERROR(MSIP, ("Could not set url in addr-spec of r-r header %d\n", *err));
	goto _error;
    }

    if (sip_setAddrSpecInRecordRouteHdr(header, addrspec, err)==SipFail)
    {
        NETERROR(MSIP, ("Could not set addr-spec in r-r header %d\n", *err));
		goto _error;
	}

	if ((sip_insertHeaderAtIndex(m, header, 0,  err))==SipFail)
	{
		NETERROR(MSIP, 
			("Could not set r-r header in the message %d\n", *err));
		goto _error;
	}

	sip_freeSipParam(urlParam);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return SipSuccess;

_error:
	sip_freeSipParam(urlParam);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return(SipFail);
}

SipBool 
SipPopRoute(SipMessage *m, SIP_S8bit **hostname, unsigned short *port,
	SipError *err)
{
	char fn[] = "SipPopRoute():";
	SipHeader *header = NULL;
    	SipReqLine *reqline = NULL, *reqlinenew = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;

	/* get AddrSpec from Route header */
	if ((sip_initSipHeader(&header, SipHdrTypeAny, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise 'route' header %d\n", *err));
		goto _error;
	}

	if ((sip_getHeaderAtIndex(m, SipHdrTypeRoute, header, 0, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get 'route' header %d\n", *err));
		goto _error;
	}

	if ((sip_getAddrSpecFromRouteHdr(header, &addrspec, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get addrspec from route header %d\n", *err));
		goto _error;
	}

	/* retrieve Request Line from message, clone it, set AddrSpec in clone, */
	/* and set clone in msg */
	if ((sip_getReqLine(m, &reqline, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get ReqLine from msg %d\n", *err));
		goto _error;
	}		

	/* and set clone in msg */
	if ((sip_initSipReqLine( &reqlinenew, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error initializing reqlinenew %d\n",*err));
		goto _error;
	}		

	if ((sip_cloneSipReqLine(reqlinenew,reqline,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not clone ReqLine %d\n", *err));
		goto _error;
	}

		
	if ((sip_setAddrSpecInReqLine(reqlinenew,addrspec,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set addrspec in reqlinenew %d\n", *err));
		goto _error;
	}
	if ((sip_setReqLine(m,reqlinenew,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set ReqLinenew in msg %d\n", *err));
		goto _error;
	}

	/* retrieve new host in request line and remove 1st route header */
	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec,&sipurl,err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get url from AddrSpec %d\n", *err));
		goto _error;
	}

	/* We must use the maddr parameter in the route header,
	 * if its there.
	 * and the port specified in the request uri
	 */
	if (SipExtractMaddrFromUrl(sipurl, hostname, err) > 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("maddr parameter present in url\n"));
	}
	else if (sip_getHostFromUrl(sipurl,hostname, err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get hostname from Url %d\n", *err));
		goto _error;
	}

	/* duplicate hostname before we return it */
	*hostname=strdup(*hostname);

	if (sip_getPortFromUrl(sipurl, port, err) == SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("No port found in url\n"));
		*port = 5060;
	}

	if ((sip_deleteHeaderAtIndex(m, SipHdrTypeRoute, 0, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not delete 'route' header %d\n", *err));
		goto _error;
	}

	sip_freeSipReqLine(reqline);
	sip_freeSipHeader(header);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipReqLine(reqlinenew);
	sip_freeSipUrl(sipurl);
	SipCheckFree(header);

	return SipSuccess;

_error:
	sip_freeSipReqLine(reqline);
	sip_freeSipHeader(header);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipReqLine(reqlinenew);
	sip_freeSipUrl(sipurl);
	SipCheckFree(header);

	return(SipFail);
}

/* returns 1 if maddr present;
   returns 0 if maddr NOT present;
   returns -1 if error 
*/
int
SipExtractMaddrFromUrl(SipUrl *sipurl, char **tmpptr, SipError *err)
{
	char fn[]="SipExtractMaddrFromUrl";
	int count, i;
	SipParam *urlParam = NULL;
	char *name;

	/* Go over the url parameters one by one */
	if (sip_getUrlParamCountFromUrl(sipurl, &count, err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get urlparam count\n", fn));
		goto _error;
	}

	if (count == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s No maddr found\n", fn));
		return 0;
	}

	for (i = 0; i < count; i++)
	{
		if (sip_getUrlParamAtIndexFromUrl(sipurl, &urlParam, i, err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get urlparam at %d\n", fn, i));
			goto _error;
		}

		/* see if its maddr */
		if (sip_getNameFromSipParam(urlParam, &name, err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get name from urlparam\n", fn));
			goto _error;
		}

		if (!strcmp(name, "maddr"))
		{
			/* extract value */
			if (sip_getValueAtIndexFromSipParam(urlParam, tmpptr, 0, err) 
				== SipFail)
			{
				NETERROR(MSIP, ("%s fail to get maddr value\n", fn));
				goto _error;
			}
			sip_freeSipParam(urlParam);
			return 1;
		}
		
		sip_freeSipParam(urlParam);
		urlParam = NULL;
	}

	return 0;
_error:

	sip_freeSipParam(urlParam);
	return -1;
}

 /*
 install route hdr in message, and append remotecontact to the end of route hdr
 * the order of routes is the same as they should be in Route hdr
 */
int SipSetRoute(SipMessage *m, int nroutes, char **routes, header_url *rcontact)
{
	char fn[]="SipSetRoute";
	SipError err;
	SipHeader *header = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	SipParam *urlParam = NULL;
	int i, n, paramAllowed;

	if(nroutes == 0 && rcontact == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no need to insert route hdr", 
					       fn));
		return 0;
	}

	/* first take care of remote contact, since it is NOT included routes */
	if(rcontact != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s inserting remote contact\n", fn));

		if( sip_initSipHeader(&header, SipHdrTypeRoute, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s Could not init route hdr\n", fn));
			goto _error;
		}

		if( sip_initSipAddrSpec(&addrspec, SipAddrSipUri, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s Could not init  addr-spec", fn));
			goto _error;
		}
		if( sip_initSipUrl(&sipurl, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s Could not initialise Sip Url\n", fn));
			goto _error;
		}
		if(rcontact->name)
		{
			if( sip_setUserInUrl(sipurl, strdup(rcontact->name), &err)
			    ==SipFail)
			{
				NETERROR(MSIP, ("%s Could not set user\n", fn)); 
				goto _error;
			}
		}
		if(rcontact->host)
		{
			if( sip_setHostInUrl(sipurl, strdup(rcontact->host), &err)
			    ==SipFail)
			{
				NETERROR(MSIP, ("%s Could not set host\n", fn)); 
				goto _error;
			}
		}
		if(rcontact->port)
		{
			if( sip_setPortInUrl(sipurl, rcontact->port, &err)
			    ==SipFail)
			{
				NETERROR(MSIP, ("%s Could not set port\n", fn)); 
				goto _error;
			}
		}

		// There can be other URL parameters we may be interested in
		n = 0;
		for (i=0; i < sizeof(rcontact->url_parameters)/sizeof(SipUrlParameter);
				i++)
		{
			paramAllowed = 0;

			if (rcontact->url_parameters[i].name)
			{
				if (!strcmp(rcontact->url_parameters[i].name, "transport"))
				{
					paramAllowed = 1;
				}

				if (paramAllowed)
				{
					if (sip_initSipParam(&urlParam, &err) == SipFail)
					{
						NETERROR(MSIP, ("Could not init url param\n"));
						goto _error;
					}

					if (sip_setNameInSipParam(urlParam,
						strdup(rcontact->url_parameters[i].name), &err) == SipFail)
					{
						NETERROR(MSIP, ("Could not set name of url param\n"));
						goto _error;
					}

					if(rcontact->url_parameters[i].value)
					{
						if (sip_insertValueAtIndexInSipParam(urlParam, 
							strdup(rcontact->url_parameters[i].value), 0, &err) == 
								SipFail)
						{
							NETERROR(MSIP, ("Could not set value in url param\n"));
							goto _error;
						}
					}

					if (sip_insertUrlParamAtIndexInUrl(sipurl, urlParam, n, &err) == 
						SipFail)
					{
						NETERROR(MSIP, ("Could not insert url param in url\n"));
						goto _error;
					}	

					n++;
					sip_freeSipParam(urlParam);
					urlParam = NULL;
				}
			}
		}

		if(rcontact->maddr)
		{
			/* set the maddr param */
			if( sip_initSipParam(&urlParam, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s Could not init url param\n", 
						fn));
				goto _error;
			}
			if( sip_setNameInSipParam(urlParam, strdup("maddr"), &err) 
			    == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set name of url param\n",
						fn));
				goto _error;
			}
			
			if( sip_insertValueAtIndexInSipParam(urlParam, strdup(rcontact->maddr), 0, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s Could not set maddr in url param\n", fn));
				goto _error;
			}

			if( sip_insertUrlParamAtIndexInUrl(sipurl, urlParam, n, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s Could not insert url param in url\n", fn));
				goto _error;
			}	

			sip_freeSipParam(urlParam);
			urlParam = NULL;
		}

		if( sip_setUrlInAddrSpec(addrspec, sipurl, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s Could not set url in addr-spec\n", fn));
			goto _error;
		}
		if( sip_setAddrSpecInRouteHdr(header, addrspec, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s Could not set addrspec in Route hdr\n",
					fn));
			goto _error;
		}
		if( sip_insertHeaderAtIndex(m, header, 0, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s fail to set route hdr\n", fn));
			goto _error;
		}
	}

	/* now insert routes, note order */
	for(i=0;i<nroutes;i++)
	{
		if(routes[nroutes-i-1] == NULL)
		{
			NETERROR(MSIP, ("%s routes %d: NULL\n", fn, nroutes-i-1));
			goto _error;
		}
		
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s inserting route %s\n", fn,
					       routes[nroutes-i-1]));

		if( sip_insertHeaderFromStringAtIndex(m, SipHdrTypeRoute, 
						      routes[nroutes-i-1], 
						      0, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to set %d route\n", fn, nroutes-i-1));
			goto _error;
		}
	}

	sip_freeSipParam(urlParam);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return 0;
 _error:
	sip_freeSipParam(urlParam);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return -1;

}
