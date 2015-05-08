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
#include <netdb.h>

#include "gis.h"
#include "net.h"

#include "ssip.h"
#include "sipaddr.h"
#include <malloc.h>
#include "siputils.h"
#include "bcpt.h"
#include "sipclone.h"
#include <ctype.h>
#include "siputils.h"
#include "dcs.h"

SipBool 
SipInitRemoteAddr(SipTranspAddr *raddr, char *addr, int port)
{
	 if (strlen(addr) > sizeof(raddr->dIpv4))
	 {
		  NETERROR(MSIP, ("Ip Address %s too large!\n", addr));
		  return SipFail;
	 }

	 strcpy(raddr->dIpv4, addr);
	 raddr->dPort = port;

	 return SipSuccess;
}
			
SipBool 
SipExtractReqUri(
	 SipMessage *m, 
	 header_url **req_uri, 
	 SipError *err
)
{
	char fn[] = "SipExtractReqUri():";
	SipUrl *sipurl= NULL;
	SipAddrSpec *addrspec= NULL;
	SipReqLine *req_line = NULL;
	
	*req_uri = (header_url *) malloc (sizeof(header_url));

	if (*req_uri == (header_url *)NULL)
	{
		NETERROR(MSIP, ("%s Malloc for req_uri header failed!\n", fn));
		return SipFail;
	}

	memset(*req_uri, 0, sizeof(header_url));

	/* get 'req_uri name' and 'req_uri host' from the reqline */

	if (( sip_getReqLineFromSipReqMsg (m, &req_line, err)) == SipFail) 
	{
		NETERROR(MSIP, ("Error getting ReqLine from SipReqMsg %d\n",*err));
		goto returnerr;
	}

	if ((sip_getAddrSpecFromReqLine(req_line, &addrspec, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Addr Spec from ReqLine %d\n", *err));
		goto returnerr;
	}

	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get Sip Url from Addr Spec %d\n", *err));
		goto returnerr;
	}

	/* get 'to name' and 'to host' from the url */
	if ((sip_getUserFromUrl(sipurl, &((*req_uri)->name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get User from SipUrl/AddSpec %d\n", *err));
	}

	if ((sip_getHostFromUrl(sipurl, &((*req_uri)->host), err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Host from SipUrl/AddSpec %d\n", *err));
		goto returnerr;
	}

	if ((sip_getPortFromUrl(sipurl, &((*req_uri)->port), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("Could not get Port from SipUrl/AddSpec %d\n", *err));
		(*req_uri)->port = 5060;
	}

	if (SipExtractMaddrFromUrl(sipurl, &((*req_uri)->maddr), err) < 0)
	{
		NETERROR(MSIP, ("%s fail to get maddr param\n", fn));
		goto returnerr;
	}

	SipExtractUrlParamsFromUrl(sipurl, (SipUrlParameter*)&((*req_uri)->url_parameters));

	/* free up resources */
	sip_freeSipReqLine(req_line) ;
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);

	return SipSuccess;

returnerr:

	SipCheckFree (*req_uri);

	(*req_uri) = NULL;

	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipReqLine(req_line);
	return SipFail;
}

int
SipSetReqUri(SipMessage *msg, header_url * req_uri, 
	char * method, SipError *err)
{
	char fn[] = "SipSetReqUri():";
	SipUrl *sipurl=NULL;
	SipReqLine *reqline = NULL;
	SipAddrSpec *addrspec = NULL;
	char * tmpptr = NULL;
	SipParam *urlParam = NULL;
	int i;

	DEBUG(MSIP, NETLOG_DEBUG4, 
		("%s method = %s name == %s, host == %s\n",
			fn, method, req_uri->name?req_uri->name:"", req_uri->host));
	
	if ((sip_initSipUrl(&sipurl, err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise Sip Url\n", fn));
		goto _error;
	}
	
	if ((sip_setHostInUrl(sipurl, strdup(req_uri->host), err))==SipFail)
	{
		NETERROR(MSIP, ("%s fail to  set hostname in url of from header\n", fn));
		goto _error;
	}
	if (req_uri->name)
	{
		if ((sip_setUserInUrl(sipurl, strdup(req_uri->name), err))==SipFail)
		{
			NETERROR(MSIP, ("%s Could not set user name in url\n", fn));
			goto _error;
		}
	}

	if (req_uri->port != 0)
	{
		if ((sip_setPortInUrl(sipurl,req_uri->port, err))==SipFail) 		 
		{
			NETERROR(MSIP, ("%s Could not set port number in url\n", fn));
			goto _error;
		}
	}
	
	for(i = 0; 
		i < sizeof(req_uri->url_parameters)/sizeof(SipUrlParameter); 
		++i)
	{
		if (sip_initSipParam(&urlParam, err) == SipFail)
		{
			NETERROR(MSIP, ("Could not init url param\n"));
			goto _error;
		}

		if(req_uri->url_parameters[i].name &&
				strlen(req_uri->url_parameters[i].name) > 0)
		{
			if (sip_setNameInSipParam(urlParam, 
				strdup(req_uri->url_parameters[i].name), err) == SipFail)
			{
				NETERROR(MSIP, ("Could not set name of url param\n"));
				goto _error;
			}

			if(req_uri->url_parameters[i].value &&
					strlen(req_uri->url_parameters[i].value) > 0)
			{
				if (sip_insertValueAtIndexInSipParam(urlParam, 
					strdup(req_uri->url_parameters[i].value), 0, err) == 
						SipFail)
				{
					NETERROR(MSIP, ("Could not set value in url param\n"));
					goto _error;
				}
			}

			if (sip_insertUrlParamAtIndexInUrl(sipurl, urlParam, i, err) == 
				SipFail)
			{
				NETERROR(MSIP, ("Could not insert url param in url\n"));
				goto _error;
			}	
		}

		sip_freeSipParam(urlParam);
		urlParam = NULL;
	}

	if ((sip_initSipReqLine(&reqline, err))== SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise Req. Line %d\n", 
			fn, *err));
		goto _error;
	}

	if ((sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err))==SipFail)
	{
		NETERROR(MSIP, 
			("%s Could not initialise Addr Spec for the Req Line %d\n", 
			fn, *err));
		goto _error;
	}

	tmpptr = strdup(method);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for method failed!\n"));
		goto _error;
	}

	if ((sip_setMethodInReqLine(reqline, tmpptr, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not write method in Req. Line %d\n", *err));
		SipCheckFree(tmpptr);
		goto _error;
	}

	tmpptr = strdup("SIP/2.0");

	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for sip version failed!\n"));
		goto _error;
	}

	if ((sip_setVersionInReqLine(reqline,tmpptr,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not write version in Req Line %d\n", *err));
		SipCheckFree(tmpptr);
		goto _error;
	}

	if ((sip_setUrlInAddrSpec(addrspec,sipurl,err))==SipFail)          
	{
		NETERROR(MSIP, ("Could not write URI in Addr Spec %d\n", *err));
		SipCheckFree(tmpptr);
		goto _error;
	}

	if ((sip_setAddrSpecInReqLine(reqline, addrspec ,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not write AddrSpec in Req Line %d\n", *err));
		goto _error;
	}

	if((sip_setReqLineInSipReqMsg(msg, reqline, err))==SipFail)
	{
		NETERROR(MSIP, 
			("Could not set Req Line in request message: %d\n", *err));
		goto _error;
	}

	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec) ;
	sip_freeSipReqLine(reqline);

	return 1;

_error:
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec) ;
	sip_freeSipReqLine(reqline);

	return -1;
}

SipBool 
SipExtractFromUri(
	 SipMessage *m, 
	 header_url **from_uri, 
	 SipError *err
)
{
	char fn[] = "SipExtractFromUri():";
	SipUrl *sipurl= NULL;
	SipAddrSpec *addrspec= NULL;
	SipHeader *header = NULL;
	int count;
	
	*from_uri = (header_url *) malloc (sizeof(header_url));

	if (*from_uri == (header_url *)NULL)
	{
		NETERROR(MSIP, 
			("ExtractFromAndToHeaders: Malloc for from_uri header failed!\n"));
		return SipFail;
	}

	memset(*from_uri, 0, sizeof(header_url));

	if (sip_initSipHeader(&header, SipHdrTypeAny, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialize Header\n", fn));
		goto returnerr;
	}

	if (sip_getHeader(m, SipHdrTypeFrom, header, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not extract Header\n", fn));
		goto returnerr;
	}

	if ((sip_getAddrSpecFromFromHdr(header, &addrspec, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Addr Spec from FromHdr %d\n", *err));
		goto returnerr;
	}

	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get Sip Url from AddrSpec/From %d\n", *err));
		goto returnerr;
	}

	/* get 'display name' from the url */
	if ((sip_getDispNameFromFromHdr(header, &((*from_uri)->display_name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get display name from FromHdr %d\n", *err));
	}

	/* get 'to name' and 'to host' from the url */
	if ((sip_getUserFromUrl(sipurl, &((*from_uri)->name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get User from FromHdr %d\n", *err));
	}

	if ((sip_getHostFromUrl(sipurl, &((*from_uri)->host), err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Host from FromHdr %d\n", *err));
		goto returnerr;
	}

	if ((sip_getPortFromUrl(sipurl, &((*from_uri)->port), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3, ("Could not get Port from FromHdr %d\n", *err));
		(*from_uri)->port = 0;
	}

	/* get tag */
	count=0;
	if( sip_getTagCountFromFromHdr(header, &count, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not get tag count from FromHdr\n", fn));
		goto returnerr;
	}
	if(count != 0)
	{
		if( sip_getTagAtIndexFromFromHdr(header, &((*from_uri)->tag), 0, err) 
		    == SipFail)
		{
			NETERROR(MSIP, ("%s Could not get tag 0 from FromHdr\n", fn));
			goto returnerr;
		}
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s FromHdr tag=%s\n",fn, (*from_uri)->tag));
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no tag in from hdr \n", fn));
	}

	SipExtractUrlParamsFromUrl(sipurl, (SipUrlParameter*)&((*from_uri)->url_parameters));

	/* free up resources */
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);

	return SipSuccess;

returnerr:

	SipCheckFree (*from_uri);
	(*from_uri) = NULL;

	if ( header != NULL )
	{
		sip_freeSipHeader(header) ;
		SipCheckFree(header);
	}

	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);

	return SipFail;
}

int SipSetToFromHdr(
	SipMessage *m, 
	header_url *pheaderurl, 
	int headertype		/* =0 for To hdr, =1 for From hdr */  
)
{
	char fn[] = "SipSetToFromHdr():";
	SipUrl *sipurl=NULL;
	SipAddrSpec *addrspec= NULL;
	SipHeader *header = NULL;
	SipError err;
	en_HeaderType dType;
	char * tmpptr = NULL;
	char *display_name = NULL;
	SipParam *urlParam = NULL;
	int i;

	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("%s setting %s hdr name == %s, host == %s, tag=%s, port=%d\n", 
	      fn, (headertype==0)?"TO":"FROM",
	      pheaderurl->name?pheaderurl->name:"", 
	      pheaderurl->host,
	      pheaderurl->tag?pheaderurl->tag:"", 
	      pheaderurl->port));

	if((display_name  = malloc(82)) == NULL)
	{
		NETERROR(MSIP, 
			("SipSetToFromHdr: Malloc for display_name failed!\n"));
		return SipFail;
	}

	if(headertype == 0)
		dType=SipHdrTypeTo;
	else
		dType=SipHdrTypeFrom;

	if ((sip_initSipHeader(&header, dType, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise 'from' header\n", fn));
		goto _error;
	}
	if ((sip_initSipAddrSpec(&addrspec,SipAddrSipUri,&err))==SipFail)                  
	{
		NETERROR(MSIP, ("%s Could not initialise Addr Spec in 'from' header\n",fn));
		goto _error;
	}
	if ((sip_initSipUrl(&sipurl, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise Sip Url\n", fn));
		goto _error;
	}

	if ((sip_setHostInUrl(sipurl, strdup(pheaderurl->host), &err))==SipFail)
	{
		NETERROR(MSIP, ("%s fail to  set hostname in url of from header\n", fn));
		goto _error;
	}
	if (pheaderurl->name)
	{
		if ((sip_setUserInUrl(sipurl, 
				strdup(pheaderurl->name), &err))==SipFail) 	
		{
			NETERROR(MSIP, ("%s Could not set user name in url\n", fn));
			goto _error;
		}
	}
	if (pheaderurl->port != 0)
	{
		if ((sip_setPortInUrl(sipurl,pheaderurl->port, &err))==SipFail) 		 
		{
			NETERROR(MSIP, ("%s Could not set port number in url\n", fn));
			goto _error;
		}
	}

	for(i = 0; 
		i < sizeof(pheaderurl->url_parameters)/sizeof(SipUrlParameter); 
		++i)
	{
		if (sip_initSipParam(&urlParam, &err) == SipFail)
		{
			NETERROR(MSIP, ("Could not init url param\n"));
			goto _error;
		}

		if(pheaderurl->url_parameters[i].name &&
					strlen(pheaderurl->url_parameters[i].name) > 0)
		{
			if (sip_setNameInSipParam(urlParam, 
				strdup(pheaderurl->url_parameters[i].name), &err) == SipFail)
			{
				NETERROR(MSIP, ("Could not set name of url param\n"));
				goto _error;
			}

			if(pheaderurl->url_parameters[i].value &&
					strlen(pheaderurl->url_parameters[i].value) > 0)
			{
				if (sip_insertValueAtIndexInSipParam(urlParam, 
					strdup(pheaderurl->url_parameters[i].value), 0, &err) == 
						SipFail)
				{
					NETERROR(MSIP, ("Could not set value in url param\n"));
					goto _error;
				}
			}

			if (sip_insertUrlParamAtIndexInUrl(sipurl, urlParam, i, &err) == 
				SipFail)
			{
				NETERROR(MSIP, ("Could not insert url param in url\n"));
				goto _error;
			}	
		}

		sip_freeSipParam(urlParam);
		urlParam = NULL;
	}

	if ((sip_setUrlInAddrSpec(addrspec, sipurl, &err))==SipFail)
	{
		NETERROR(MSIP, 
			("%s Could not set url in Addr Spec of From header\n", fn));
		goto _error;
	}

	if(dType==SipHdrTypeFrom)
	{
		if ((sip_setAddrSpecInFromHdr(header, addrspec, &err))==SipFail)
		{
			NETERROR(MSIP, 
				("%s Could not set Addr Spec in From header\n", fn));
			goto _error;
		}
	
		if(pheaderurl->name == NULL)
		{
			if(m->dType == SipMessageRequest)
			{
				strcpy(display_name, "\"Anonymous\"");

				if((sip_setDispNameInFromHdr(header, display_name, &err))==SipFail)
				{
					NETERROR(MSIP, 
						("%s Could not set Display Name in From header\n", fn));
					goto _error;
				}

				display_name = NULL;
			}
		}
		else if(pheaderurl->display_name)
		{
			if((sip_setDispNameInFromHdr(header, 
				strdup(pheaderurl->display_name), &err))==SipFail)
			{
				NETERROR(MSIP, 
					("%s Could not set Display Name in From header\n", fn));
				goto _error;
			}
		}
		else
		{
			if((sip_setDispNameInFromHdr(header, strdup(""), &err))==SipFail)
			{
				NETERROR(MSIP, 
					("%s Could not set Display Name in From header\n", fn));
				goto _error;
			}
		}

		if ((pheaderurl->tag) && (strlen(pheaderurl->tag) != 0))
		{
			tmpptr = strdup(pheaderurl->tag);
			if ((sip_insertTagAtIndexInFromHdr(header, tmpptr, 
							   (SIP_U32bit) 0, &err))==SipFail)
			{
				NETERROR(MSIP, ("%s fail to set tag in 'from' hdr\n", fn));
				goto _error;
			}
		}
	}
	else
	{
		if ((sip_setAddrSpecInToHdr(header, addrspec, &err))==SipFail)
		{
			NETERROR(MSIP, ("%s Could not set Addr Spec in To header\n", fn));
			goto _error;
		}
	
		if(pheaderurl->display_name)
		{
			if((sip_setDispNameInToHdr(header, 
				strdup(pheaderurl->display_name), &err))==SipFail)
			{
				NETERROR(MSIP, 
					("%s Could not set Display Name in To header\n", fn));
				goto _error;
			}
		}
		else
		{
			if((sip_setDispNameInToHdr(header, strdup(""), &err))==SipFail)
			{
				NETERROR(MSIP, 
					("%s Could not set Display Name in To header\n", fn));
				goto _error;
			}
		}

		if ((pheaderurl->tag) && (strlen(pheaderurl->tag) != 0))
		{
			tmpptr = strdup(pheaderurl->tag);
			if ((sip_insertTagAtIndexInToHdr(header, tmpptr, 
							 (SIP_U32bit) 0, &err)) == SipFail)
			{
				NETERROR(MSIP, ("%s fail to set tag in to hdr\n", fn));
				goto _error;
			}
		}
	}

	if ((sip_setHeader(m, header, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not set From header in message\n", fn));
		goto _error;
	}

	sip_freeSipParam(urlParam);
	SipCheckFree(display_name);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);

	return 0;
 _error:
	sip_freeSipParam(urlParam);
	SipCheckFree(display_name);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);

	return -1;
}

SipBool 
SipExtractToUri(
	 SipMessage *m, 
	 header_url **to, 
	 SipError *err
)
{
	char fn[] = "SipExtractToUri():";
	SipUrl *sipurl= NULL;
	SipAddrSpec *addrspec= NULL;
	SipHeader *header = NULL;
	int count;
	
	*to = (header_url *) malloc (sizeof(header_url));
	if (*to == (header_url *)NULL)
	{
		NETERROR(MSIP, 
			("ExtractFromAndToHeaders: Malloc for from_uri header failed!\n"));
		return SipFail;
	}

	memset(*to, 0, sizeof(header_url));

	if (sip_initSipHeader(&header, SipHdrTypeAny, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialize Header\n", fn));
		goto returnerr;
	}

	if (sip_getHeader(m, SipHdrTypeTo, header, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not extract Header\n", fn));
		goto returnerr;
	}

	if ((sip_getAddrSpecFromToHdr(header, &addrspec, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Addr Spec from ToHdr %d\n", *err));
		goto returnerr;
	}

	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get Sip Url from ToHdr %d\n", *err));
		goto returnerr;
	}

	/* get 'display name' from the url */
	if ((sip_getDispNameFromToHdr(header, &((*to)->display_name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get display name from to header %d\n", *err));
	}

	/* get 'to name' and 'to host' from the url */
	if ((sip_getUserFromUrl(sipurl, &((*to)->name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get User from ToHdr %d\n", *err));
		// The user part can be empty
		//goto returnerr;
	}

	if ((sip_getHostFromUrl(sipurl, &((*to)->host), err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Host from ToHdr %d\n", *err));
		goto returnerr;
	}

	if ((sip_getPortFromUrl(sipurl, &((*to)->port), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3, ("Could not get Port from ToHdr %d\n", *err));
		(*to)->port = 0;
	}

	/* get tag */
	count=0;
	if( sip_getTagCountFromToHdr(header, &count, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not get tag count from toHdr\n", fn));
		goto returnerr;
	}
	if(count != 0)
	{
		if( sip_getTagAtIndexFromToHdr(header, &((*to)->tag), 0, err) 
		    == SipFail)
		{
			NETERROR(MSIP, ("%s Could not get tag 0 from toHdr\n", fn));
			goto returnerr;
		}
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no tag in to hdr \n", fn));
	}

	SipExtractUrlParamsFromUrl(sipurl, (SipUrlParameter*)&((*to)->url_parameters));

	/* free up resources */
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);

	return SipSuccess;

returnerr:
	SipCheckFree (*to);
	(*to) = NULL;
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	return SipFail;
}

SipBool 
SipExtractReferToUri(
	 SipMessage *m, 
	 header_url **referto, 
	 SipError *err
)
{
	char fn[] = "SipExtractRefertoUri():";
	SipUrl *sipurl= NULL;
	SipAddrSpec *addrspec= NULL;
	SipHeader *header = NULL;
	
	*referto = (header_url *) malloc (sizeof(header_url));
	if (*referto == (header_url *)NULL)
	{
		NETERROR(MSIP, 
			("%s Malloc for from_uri header failed!\n", fn));
		return SipFail;
	}

	memset(*referto, 0, sizeof(header_url));

	if (sip_initSipHeader(&header, SipHdrTypeAny, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialize Header\n", fn));
		goto returnerr;
	}

	if (sip_getHeader(m, SipHdrTypeReferTo, header, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not extract Header\n", fn));
		goto returnerr;
	}

	if ((sip_getAddrSpecFromReferToHdr(header, &addrspec, err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not get Addr Spec from ReferTo\n", fn));
		goto returnerr;
	}

	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get Sip Url from ToHdr %d\n", *err));
		goto returnerr;
	}

	/* get 'to name' and 'to host' from the url */
	if ((sip_getUserFromUrl(sipurl, &((*referto)->name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get User from ToHdr %d\n", *err));
		goto returnerr;
	}

	if ((sip_getHostFromUrl(sipurl, &((*referto)->host), err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Host from ToHdr %d\n", *err));
		goto returnerr;
	}

	if((sip_getHeaderFromUrl(sipurl, &((*referto)->header),err)) == SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s No header found in referto",fn));
	}


	if((*referto)->name == NULL || (*referto)->host == NULL)
	{
		NETERROR(MSIP, ("%s referto name|host missing\n", fn));
		goto returnerr;
	}

	/* free up resources */
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);

	return SipSuccess;

returnerr:
	SipCheckFree (*referto);
	(*referto) = NULL;
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	return SipFail;
}

SipBool 
SipExtractReferByUri(
	 SipMessage *m, 
	 header_url **referby, 
	 SipError *err
)
{
	char fn[] = "SipExtractReferbyUri():";
	SipUrl *sipurl= NULL;
	SipAddrSpec *addrspec= NULL;
	SipHeader *header = NULL;
	
	*referby = (header_url *) malloc (sizeof(header_url));
	if (*referby == (header_url *)NULL)
	{
		NETERROR(MSIP, 
			("%s Malloc for from_uri header failed!\n", fn));
		return SipFail;
	}

	memset(*referby, 0, sizeof(header_url));

	if (sip_initSipHeader(&header, SipHdrTypeAny, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialize Header\n", fn));
		goto returnerr;
	}

	if (sip_getHeader(m, SipHdrTypeReferredBy, header, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not extract Header\n", fn));
		goto returnerr;
	}

	if ((sip_getReferrerFromReferredByHdr(header, &addrspec, err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not get Addr Spec from Referby\n", fn));
		goto returnerr;
	}

	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get Sip Url from ToHdr %d\n", *err));
		goto returnerr;
	}

	/* get 'to name' and 'to host' from the url */
	if ((sip_getUserFromUrl(sipurl, &((*referby)->name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get User from ToHdr %d\n", *err));
		goto returnerr;
	}

	if ((sip_getHostFromUrl(sipurl, &((*referby)->host), err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get Host from ToHdr %d\n", *err));
		goto returnerr;
	}

	if((*referby)->name == NULL || (*referby)->host == NULL)
	{
		NETERROR(MSIP, ("%s referby name|host missing\n", fn));
		goto returnerr;
	}

	/* free up resources */
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);

	return SipSuccess;

returnerr:
	SipCheckFree (*referby);
	(*referby) = NULL;
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	return SipFail;
}

int SipSetReferToByHdr(
	SipMessage *m, 
	header_url *referto, 
	header_url *referby 
)
{
	char fn[] = "SipSetReferToByHdr():";
	SipUrl *sipurl=NULL;
	SipAddrSpec *addrspec= NULL;
	SipHeader *header = NULL;
	SipError err;
	en_HeaderType dType;
	header_url *tmpheader = NULL;

	if(referto == NULL || referto->name == NULL || referto->host == NULL)
	{
		NETERROR(MSIP, ("%s referto url error\n", fn));
		return -1;
	}
	// referby is not a mandatory header
	if(referby)
	{
		if(referby->name == NULL || referby->host == NULL) 
		{
			NETERROR(MSIP, ("%s referby url error\n", fn));
			return -1;
		}
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s setting referby hdr name=%s host=%s\n",
					       fn, referby->name, referby->host));
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s setting referto hdr name=%s host=%s\n",
				       fn, referto->name, referto->host));

	dType=SipHdrTypeReferTo;
	tmpheader = referto;

 _again:

	if ((sip_initSipHeader(&header, dType, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise header\n", fn));
		goto _error;
	}

	if(dType==SipHdrTypeReferredBy)
	{
		/* addrspec is set one more time in referredby hdr */
		if(sip_setReferrerInReferredByHdr(header, addrspec, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Could not set referenced \n", fn));
			goto _error;
		} 
	}

	if ((sip_initSipAddrSpec(&addrspec,SipAddrSipUri,&err))==SipFail)                  
	{
		NETERROR(MSIP, ("%s Could not init AddrSpec in header\n",fn));
		goto _error;
	}
	if ((sip_initSipUrl(&sipurl, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise Sip Url\n", fn));
		goto _error;
	}

	if ((sip_setHostInUrl(sipurl, strdup(tmpheader->host), &err))==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set referto hostname in url\n", fn));
		goto _error;
	}
	if ((sip_setUserInUrl(sipurl, strdup(tmpheader->name), &err))==SipFail)	
	{
		NETERROR(MSIP, ("%s Could not set user name in url\n", fn));
		goto _error;
	}

	if(tmpheader->header && strlen(tmpheader->header))
	{
		if( sip_setHeaderInUrl(sipurl,strdup(tmpheader->header), &err) == SipFail )
		{
			NETERROR(MSIP, ("%s could not set header in url",fn));
			goto _error;
		}
	}

	if ((sip_setUrlInAddrSpec(addrspec, sipurl, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not set url in Addr Spec\n", fn));
		goto _error;
	}

	if(dType==SipHdrTypeReferTo)
	{
		if ((sip_setAddrSpecInReferToHdr(header, addrspec, &err))==SipFail)
		{
			NETERROR(MSIP, ("%s Could not set Addr Spec in referto header\n", fn));
			goto _error;
		}
	}
	else
	{
		if ((sip_setReferrerInReferredByHdr(header, addrspec, &err))==SipFail)
		{
			NETERROR(MSIP, ("%s Could not set Addr Spec in referred by header\n", fn));
			goto _error;
		}
	}

	if ((sip_setHeader(m, header, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not set From header in message\n", fn));
		goto _error;
	}

	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	if(dType==SipHdrTypeReferTo)
	{
		dType=SipHdrTypeReferredBy;
		tmpheader = referby;
		goto _again;
	}

	return 0;
 _error:
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);

	return -1;
}

int SipCompareUrlNameandHost(header_url * src, header_url * dst)
{
	/* Routine to compare two URLs. return 0 if they are not, 1 if they are */

	if (((src) && !(dst)) || (!(src) && (dst)))
		return 0;

	if (((src->name) && !(dst->name)) || (!(src->name) && (dst->name)))
		return 0;
	if (((src->host) && !(dst->host)) || (!(src->host) && (dst->host)))
		return 0;
	if ((src->tag) && !(dst->tag))
		return 0;

	if ((src->name) && (strcmp(src->name, dst->name) != 0))
		return 0;
	if ((src->host) && (strcmp(src->host, dst->host) != 0))
		return 0;
	if ((src->tag) && (strcmp(src->tag, dst->tag) != 0))
		return 0;

	return 1;
}

char *
SipGetReason(int status)
{
	 switch (status)
	 {
	 case 100:
		 return "Trying";
	 case 180: 
		 return "Ringing";
	 case 183:
		 return "Session Progress";
	 case 200:
		 return "OK";
	 case 202:
		 return "Accepted";
	 case 300:
		 return "Multiple choices";
	 case 302:
		 return "Moved Temporarily";
	 case 400:
		 return "Bad Request";
	 case 401:
		 return "Unauthorized";
	 case 402:
		 return "Payment Required";
	 case 403:
		 return "Forbidden";
	 case 404:
		 return "Not Found";
	 case 405:
		 return "Method Not Allowed";
	 case 406:
		 return "Not Acceptable";
	 case 407:
		 return "Proxy Authentication Required";
	 case 408:
		 return "Request Timeout";
	 case 409:
		 return "Conflict";
	 case 410:
		 return "Gone";
	 case 413:
		 return "Request Entity Too long";
	 case 414:
		 return "Request Entity Too long";
	 case 415:
		 return "Unsupported Media Type";
	 case 420:
		 return "Bad Extension";
	 case 422:
		 return "Session Timer Too Small";
	 case 480:
		 return "Temporarily Unavailable";
	 case 481:
		 return "Call/Transaction Does Not Exist";
	 case 482:
		 return "Loop Detected";
	 case 483:
		 return "Too Many Hops";
	 case 484:
		 return "Address Incomplete";
	 case 486:
		 return "Busy here";
	 case 487:
		 return "Request Terminated";
	 case 488:
		 return "Not Acceptable Here";
	 case 500:
		 return "Server Internal Error";
	 case 501:
		 return "Not Implemented";
	 case 502:
		 return "Bad Gateway";
	 case 503:
		 return "Service Unavailable";
	 case 504:
		 return "Server Timeout";
	 case 505:
		 return "Version Not Supported";
	 case 513:
		 return "Message Too Large";
	 case 600:
		 return "Busy Everywhere";
	 case 603:
		 return "Decline";
	 case 604:
		 return "Does Not Exist Anywhere";
	 case 606:
		 return "Not Acceptable";
	 default:
		 return "Ambiguous";
	 }
	 
}

SipEventContext *
SipDupContext (SipEventContext *context)
{
	SipEventContext *newcontext;

	newcontext = (SipEventContext *)malloc (sizeof (SipEventContext));
	if (newcontext == NULL)
	{
		return NULL;
	}
	memcpy (newcontext, context, sizeof (SipEventContext));

	if((newcontext->pTranspAddr = (SipTranspAddr *)malloc(sizeof(SipTranspAddr))))
	{
		memcpy(newcontext->pTranspAddr, context->pTranspAddr, sizeof(SipTranspAddr));
	}

	newcontext->pData = RealmInfoDup(context->pData, MEM_LOCAL);

	return newcontext;
}

	
int
SipFreeContext(SipEventContext *context) 
{
	if (context)
	{
		 SipCheckFree(context->pTranspAddr);
		 RealmInfoFree(context->pData, MEM_LOCAL);
		 SipCheckFree(context);
	}
	return(0);
}

void
sip_freeEventContext(SipEventContext *context)
{
	SipFreeContext(context);
}

/* 0 = syslog, 1 = console, -1 = none */
int
SipDebugLoc()
{
	if (netLogStruct.flags & NETLOG_SIPTERMINAL)
	{
		return 1;
	}
	else if (netLogStruct.flags & NETLOG_SIPSYSLOG)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int
SipCheckFree(void *m)
{
	if (m)
	{
		free(m);
	}
	return(0);
}

int
SipCheckFree2(void *m, int freefn)
{
	if (m)
	{
		MFree(freefn, m);
	}
	return(0);
}

/*
** Function :    
**		SipCopyHeaders()
**
** Description :  
**		Copy headers from source to target message
**
**
** Arguments:
**
**		s         source message
**		m         target message
**		context   context of source message
**
** Return Values:
**		 1    on success
**		-1    on failure
*/
int
SipCopyHeaders(
	SipMessage *s, 
	SipMessage *m,
	SipEventContext *context
)
{
	char fn[] = "SipCopyHeaders():";
	SipHeader *header = NULL;
	SipError err;
	SIP_U32bit count, i, inserted_header;
	SipBadHeader *badheader;
	SIP_S8bit *badheader_name, *badheader_body;


	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny ,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init via hdr\n",fn));
		goto _error;
	}

	/* Scan and Copy the Via Headers */

	if (sip_getHeaderCount(s, SipHdrTypeVia, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get via count\n",fn));
		goto _error;
	}

	if(count == 0)
	{
		NETERROR(MSIP, ("%s no via header in message \n",fn));
		goto _error;
	}

	for( i = 0 ; i < count ; i++ )
	{
		if (sip_getHeaderAtIndex(s, SipHdrTypeVia, header, 
			(SIP_U32bit)i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get %d-th via hdr\n",fn,i));
			goto _error;
		}
		
		if (sip_insertHeaderAtIndex(m, header, 
			(SIP_U32bit)i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to insert %d-th via hdr\n",fn,i));
			goto _error;
		}

		sip_freeSipHeader(header);
	}

	SipCheckFree(header);
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init from hdr\n",fn));
		goto _error;
	}

	/* Scan and Copy From: portion of header */

	if ( sip_getHeader(s, SipHdrTypeFrom, header, &err) != SipFail )	
	{
		if (sip_setHeader(m, header, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s fail to set from hdr\n",fn));
			goto _error;
		}
	}
	else
	{
		if(sip_getBadHeaderCount(s, &count, &err) != SipFail)
		{
			for(i = 0, inserted_header = 0; i < count; ++i)
			{
				if(sip_getBadHeaderAtIndex(s, &badheader, i, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s failed to get Bad Header at index %d\n", fn, i ));
					continue;
				}

				if(sip_getNameFromBadHdr(badheader, &badheader_name, &err) != SipFail &&
					sip_getBodyFromBadHdr(badheader, &badheader_body, &err) != SipFail)
				{
					if(strcasecmp(badheader_name, "From") == 0 || strcasecmp(badheader_name, "F") == 0)
					{
						SetSipCustomHeader(m, strdup(badheader_name), strdup(badheader_body), &err);
						inserted_header = 1;
					}
				}
				else
				{
					NETERROR(MSIP, ("%s failed to get Bad Header Body\n", fn));
				}

				sip_freeSipBadHeader(badheader);
			}

			if(!inserted_header)
			{
				NETERROR(MSIP, ("%s fail to get from hdr\n",fn));
				goto _error;
			}
		}
		else
		{
			NETERROR(MSIP, ("%s fail to get from hdr\n",fn));
			goto _error;
		}
	}

	sip_freeSipHeader(header);
	SipCheckFree( header );
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init To hdr\n",fn));
		goto _error;
	}

	/* Scan and Copy To: portion of header */

	if ( sip_getHeader(s, SipHdrTypeTo, header, &err) != SipFail )
	{
		if (sip_setHeader(m, header, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s fail to set to hdr\n",fn));
			goto _error;
		}
	}
	else
	{
		if(sip_getBadHeaderCount(s, &count, &err) != SipFail)
		{
			for(i = 0, inserted_header = 0; i < count; ++i)
			{
				if(sip_getBadHeaderAtIndex(s, &badheader, i, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s failed to get Bad Header at index %d\n", fn, i ));
					continue;
				}

				if(sip_getNameFromBadHdr(badheader, &badheader_name, &err) != SipFail &&
					sip_getBodyFromBadHdr(badheader, &badheader_body, &err) != SipFail)
				{
					if(strcasecmp(badheader_name, "To") == 0 || strcasecmp(badheader_name, "T") == 0)
					{
						SetSipCustomHeader(m, strdup(badheader_name), strdup(badheader_body), &err);
						inserted_header = 1;
					}
				}
				else
				{
					NETERROR(MSIP, ("%s failed to get Bad Header Body\n", fn));
				}

				sip_freeSipBadHeader(badheader);
			}

			if(!inserted_header)
			{
				NETERROR(MSIP, ("%s fail to get to hdr\n",fn));
				goto _error;
			}
		}
		else
		{
			NETERROR(MSIP, ("%s fail to get to hdr\n",fn));
			goto _error;
		}
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init call id hdr\n",fn));
		goto _error;
	}

	if ( sip_getHeader(s, SipHdrTypeCallId, header, &err) == SipFail )
	{
		NETERROR(MSIP, ("%s fail to get callID hdr\n",fn));
		goto _error;
	}

	/* Scan and copy Copy Call Id */

	if (sip_setHeader(m, header, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set callID hdr\n",fn));
		goto _error;
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init Cseq hdr\n",fn));
		goto _error;
	}

	/* Scan and Copy CSeq */	

	if ( sip_getHeader(s, SipHdrTypeCseq, header, &err) == SipFail )
	{
		NETERROR(MSIP, ("%s fail to get Cseq hdr\n",fn));
		goto _error;
	}

	if (sip_setHeader(m, header, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set Cseq hdr\n",fn));
		goto _error;
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	return 1;

 _error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return -1;
}

/* copy via headers from request to response */
int
SipCopyVia(
	SipMessage *req, 
	SipMessage *resp
)
{
	char fn[] = "SipCopyVia():";
	SipHeader *header = NULL;
	SipError err;
	SIP_U32bit count, i;

	/* Copy the Via Headers */
	if (sip_initSipHeader(&header, SipHdrTypeAny ,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init via hdr\n",fn));
		goto _error;
	}

	if (sip_getHeaderCount(req, SipHdrTypeVia, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get via count\n",fn));
		goto _error;
	}
	if(count == 0)
	{
		NETERROR(MSIP, ("%s no via header in message \n",fn));
		goto _error;
	}

	for(i=0 ; i<count ; i++)
	{
		if (sip_getHeaderAtIndex(req, SipHdrTypeVia, header, 
			(SIP_U32bit)i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get %d-th via hdr\n",fn,i));
			goto _error;
		}
		
		if (sip_insertHeaderAtIndex(resp, header, 
			(SIP_U32bit)i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to insert %d-th via hdr\n",fn,i));
			goto _error;
		}

		sip_freeSipHeader(header);
	}
	SipCheckFree(header);
	return 0;

 _error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return -1;
}

/* copy record-route headers from request to response */
int
SipCopyRR(
	SipMessage *req, 
	SipMessage *resp
)
{
	char fn[] = "SipCopyRR():";
	SipHeader *header = NULL;
	SipError err;
	SIP_U32bit count, i;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("entering %s\n", fn));

	if (sip_getHeaderCount(req, SipHdrTypeRecordRoute, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get RR count\n",fn));
		goto _error;
	}
	if(count == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no RR header found \n",fn));
		return 0;;
	}

	/* Copy the RR Headers */
	if (sip_initSipHeader(&header, SipHdrTypeAny ,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init via hdr\n",fn));
		goto _error;
	}

	for(i=0 ; i<count ; i++)
	{
		if (sip_getHeaderAtIndex(req, SipHdrTypeRecordRoute, header, 
			(SIP_U32bit)i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get %d-th via hdr\n",fn,i));
			goto _error;
		}
		
		if (sip_insertHeaderAtIndex(resp, header, 
			(SIP_U32bit)i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to insert %d-th via hdr\n",fn,i));
			goto _error;
		}

		sip_freeSipHeader(header);
	}
	SipCheckFree(header);
	return 0;

 _error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return -1;
}
	
int
SipSetStatusLine(
	SipMessage *s, 
	int status_code,
	char *status_str
)
{
	char fn[] = "SipInitStatusLine():";
	SipStatusLine *status_line;
	SipError err;

	if (sip_initSipStatusLine( &status_line, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to init status line\n",fn));
		goto _error;
	}

	if (sip_setVersionInStatusLine (status_line, strdup("SIP/2.0"), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set version in status line\n",fn));
		goto _error;
	}

	if (sip_setStatusCodeNumInStatusLine (status_line, status_code, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set status code\n",fn));
		goto _error;
	}

	if (sip_setReasonInStatusLine(status_line , strdup(status_str), &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set reason in status line\n",fn));
		goto _error;
	}

	if (sip_setStatusLineInSipRespMsg(s, status_line, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set status line in msg\n",fn));
		goto _error;
	}

	sip_freeSipStatusLine(status_line);

	return 1;
 _error:
	sip_freeSipStatusLine(status_line);
	return -1;
}

/* Must be called inside a thread, as may block.
 * returns 0 if there is a match, -1 if no match, -1 if not sure (due to error)
 */ 
int
SipMatchDomains(char *dom1, char *dom2)
{
	long ipaddr1=0, ipaddr2=0;
	int herror;

	/* Now we must do a DNS lookup */
	ipaddr1 = ResolveDNS(dom1, &herror);
	ipaddr2 = ResolveDNS(dom2, &herror);

	if (ipaddr1 != ipaddr2)
	{
		return -1;
	}

	return 0;
}

int SipIsLetterStr(char *str)
{
	int len,n=0;
	
	len=strlen(str);
	while(n<len)
	{
		if(isalpha(str[n++]))
			return 1;
	}
	
	return 0;
}
			

int SipReplaceContactwithRsa(SipMessage *s,header_url *req_uri, SipError *err)
{
	char fn[]="SipInsertContact():";
	SipHeader *old_header = NULL, *new_header;
	SipAddrSpec *addrspec = NULL;
	SipContactParam  *contactp = NULL;
	SIP_U32bit  count, k;
	char tmparr[500],mytmparr[256];
	char *tmpptr=NULL;
	int i;
	
	/* Extract the contact header in a temporary header */
	if ((sip_initSipHeader(&old_header, SipHdrTypeAny, err))==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip header\n",
			fn));
		return( -1);	
	}

	if ((sip_initSipHeader(&new_header, SipHdrTypeContactNormal, err))==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip header\n",
			fn));
		return( -1);	
	}

	if (sip_getHeaderAtIndex(s, SipHdrTypeContactAny, old_header, (SIP_U32bit)0,
			err) == SipFail)
	{
		NETERROR(MSIP, ("%s could not get any contact header\n", fn));
		goto _error1;
	}

	if(sip_getContactParamsCountFromContactHdr(old_header,&count,err) == SipFail)
	{
		NETERROR(MSIP,("%s error in getContactParamsCount header %d\n",fn,*err));
		goto _error1;
	}

	if(count)
	{
		for(k=0;k<count;k++)
		{
			if (sip_getContactParamAtIndexFromContactHdr(old_header, &contactp,k,err) == SipFail)
			{
				NETERROR(MSIP,("%s error get %d-th ContactParam\n", fn,k));
				goto _error1;
			}

			if (sip_insertContactParamAtIndexInContactHdr(new_header,contactp,k,err) == SipFail)
			{
				NETERROR(MSIP,("%s error set %d-th ContactParam\n", fn,k));
				goto _error1;
			}
		}
	}

	/* delete the old header */
	if (sip_deleteHeaderAtIndex(s, SipHdrTypeContactAny, 0, err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not delete contact header\n", fn));
		goto _error1;
	}

	sip_freeSipHeader(old_header);
	SipCheckFree(old_header);
	old_header = NULL;


	if (req_uri->display_name)
	{
		sprintf(mytmparr, "\"%s\"", req_uri->display_name);
		strcat(tmparr, mytmparr);
	}

	/* First Form the Req Uri */
	strcpy(tmparr,"sip:") ;

	if (req_uri->name)
	{
		 strcat(tmparr, req_uri->name);
		 strcat(tmparr,"@");
	}

	strcat(tmparr, req_uri->host);

	if (req_uri->port != 0)
	{
		strcat(tmparr, ":");
		sprintf(mytmparr, "%d", req_uri->port);
		strcat(tmparr, mytmparr);
	}

	tmpptr = strdup(tmparr);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("%s strdup for sip url failed!\n",fn));
		goto _error;
	}
	DEBUG(MSIP, NETLOG_DEBUG4, ("%s Req Uri == [%s]\n", fn, tmparr));

	/* init structures */
	if ((sip_initSipAddrSpec(&addrspec,SipAddrReqUri,err))==SipFail)
	{
	    NETERROR(MSIP, ("%s Could not initialise addr-spec\n",fn));
	    goto _error;
	}
	
	if ((sip_setUriInAddrSpec(addrspec,tmpptr,err))==SipFail)          
	{
		NETERROR(MSIP, ("%s Could not write URI in Addr Spec %d\n", fn,*err));
		SipCheckFree(tmpptr);
		goto _error;
	}
	
	if((sip_setAddrSpecInContactHdr(new_header,addrspec,err))==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set addrspec in Contact hdr\n",fn));
		goto _error;
	}

	if((sip_insertHeaderAtIndex(s,new_header,0,err)==SipFail))
	{
		NETERROR(MSIP, ("%s fail to insert contact hdr\n",fn));
		goto _error;
	}
	
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(new_header);
	SipCheckFree(new_header);
	new_header = NULL;
	
	return 0;

_error:
	sip_freeSipAddrSpec(addrspec);
_error1:
	sip_freeSipHeader(new_header);
	SipCheckFree(new_header);
	new_header = NULL;
	
	sip_freeSipHeader(old_header);
	SipCheckFree(old_header);
	old_header = NULL;
	return -1;
}

int SipInsertContact(SipMessage *s,header_url *req_uri, SipError *err)
{
	char fn[]="SipInsertContact():";
	SipHeader *header = NULL;
	SipAddrSpec *addrspec = NULL;
	char tmparr[500],mytmparr[256];
	char *tmpptr=NULL;
	int i;
	
	if (req_uri->display_name)
	{
		sprintf(mytmparr, "\"%s\"", req_uri->display_name);
		strcat(tmparr, mytmparr);
	}

	/* First Form the Req Uri */
	strcpy(tmparr,"sip:") ;

	if (req_uri->name)
	{
		 strcat(tmparr, req_uri->name);
		 strcat(tmparr,"@");
	}

	strcat(tmparr, req_uri->host);

	if (req_uri->port != 0)
	{
		strcat(tmparr, ":");
		sprintf(mytmparr, "%d", req_uri->port);
		strcat(tmparr, mytmparr);
	}

	for(i = 0; i < sizeof(req_uri->url_parameters)/sizeof(SipUrlParameter); ++i)
	{
		if(req_uri->url_parameters[i].name && strlen(req_uri->url_parameters[i].name) > 0)
		{
			if (req_uri->url_parameters[i].value &&
					strlen(req_uri->url_parameters[i].value) > 0)
			{
				sprintf(mytmparr, ";%s=%s", req_uri->url_parameters[i].name,
												req_uri->url_parameters[i].value);
				strcat(tmparr, mytmparr);
			}
			else
			{
				sprintf(mytmparr, ";%s", req_uri->url_parameters[i].name);
				strcat(tmparr, mytmparr);
			}
		}
	}

	DEBUG(MSIP, NETLOG_DEBUG4, ("%s Req Uri == [%s]\n", fn, tmparr));

	/* init structures */
	if ((sip_initSipAddrSpec(&addrspec,SipAddrReqUri,err))==SipFail)
	{
	    NETERROR(MSIP, ("%s Could not initialise addr-spec\n",fn));
	    goto _error;
	}
	
	if ((sip_initSipHeader(&header, SipHdrTypeContactNormal, err))==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip header\n",fn));
		goto _error;	
	}
	
	tmpptr = strdup(tmparr);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("%s strdup for sip url failed!\n",fn));
		goto _error;
	}

	if ((sip_setUriInAddrSpec(addrspec,tmpptr,err))==SipFail)          
	{
		NETERROR(MSIP, ("%s Could not write URI in Addr Spec %d\n", fn,*err));
		SipCheckFree(tmpptr);
		goto _error;
	}
	
	if((sip_setAddrSpecInContactHdr(header,addrspec,err))==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set addrspec in Contact hdr\n",fn));
		goto _error;
	}
	if((sip_insertHeaderAtIndex(s,header,0,err)==SipFail))
	{
		NETERROR(MSIP, ("%s fail to insert contact hdr\n",fn));
		goto _error;
	}
	
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return 0;

 _error:
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return -1;
}

SipBool 
SipGetCallID(
	 SipMessage *m, 
	 char **callID,
	 SipError *err
)
{
	char fn[] = "SipExtractCallID():";
	SipHeader *header = NULL;
	char *cid;

	if (sip_initSipHeader (&header, SipHdrTypeAny , err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error initializing header\n", fn));
		goto _error;
	}
	
	if (( sip_getHeader(m, SipHdrTypeCallId, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("%s Error getting callid hdr %d\n",fn,*err));
		goto _error;
	}

	if (( sip_getValueFromCallIdHdr(header, &cid, err))==SipFail)
	{
		NETERROR(MSIP, ("%s:Error getting value from callid hdr %d\n",fn,*err));
		goto _error;
	}

	*callID = strdup(cid);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return SipSuccess;

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return SipFail;
}

SipBool 
SipGetCSeq(
	SipMessage *m, 
	SIP_U32bit *seqnum,
	char **method,
	SipError *err
)
{
	char fn[] = "SipGetCSeq():";
	SipHeader *header = NULL;

	if (sip_initSipHeader (&header, SipHdrTypeAny , err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error initializing header\n", fn));
		goto _error;
	}
	
	if (( sip_getHeader(m, SipHdrTypeCseq, header, err)) == SipFail )
	{
		NETERROR(MSIP, 
			("%s:Error getting To from siprequest %d\n",fn,*err));
		goto _error;
	}

	if (( sip_getSeqNumFromCseqHdr(header, seqnum,err))==SipFail)
	{
		NETERROR(MSIP, ("%s:Error getting seq from cseq hdr %d\n",fn,*err));
		goto _error;
	}

	if (method)
	{
		if (sip_getMethodFromCseqHdr(header, method, err) == SipFail)
		{
			NETERROR(MSIP, 
				("%s:Error getting method from cseq hdr %d\n",fn,*err));
			goto _error;
		}
		*method = strdup(*method);
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	return SipSuccess;

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return SipFail;
}

header_url *
UrlAlloc(int mallocfn)
{
	header_url *url;

	url = MMalloc(mallocfn, sizeof(header_url));
	memset(url, 0, sizeof(header_url));

	return url;
}

int
UrlFree(header_url *url, int freefn)
{
	int i;

	if (url == NULL)
	{
		return 0;
	}

	SipCheckFree2(url->name, freefn);
	SipCheckFree2(url->host, freefn);
	SipCheckFree2(url->tag, freefn);
	SipCheckFree2(url->display_name, freefn);
	SipCheckFree2(url->maddr, freefn);

	for(i = 0; i < sizeof(url->url_parameters)/sizeof(SipUrlParameter); ++i)
	{
		SipCheckFree2(url->url_parameters[i].name, freefn);

		SipCheckFree2(url->url_parameters[i].value, freefn);
	}

	for(i = 0; i < sizeof(url->priv_params)/sizeof(SipPrivacyParams); ++i)
	{
		SipCheckFree2(url->priv_params[i].name, freefn);
		SipCheckFree2(url->priv_params[i].value, freefn);
	}

	SipCheckFree2(url->header,freefn);

	SipCheckFree2(url, freefn);

	return 1;
}


	
header_url *
UrlDup(header_url *url, int mallocfn)
{
	char fn[] = "UrlDup():";
	header_url *urldup = NULL;
	int i;

	if (url == NULL)
	{
		return NULL;
	}

	urldup = (header_url *) MMalloc(mallocfn, sizeof(header_url));
	if (urldup == (header_url *)NULL)
	{
		NETERROR(MSIP, ("%s Malloc for urldup failed!\n", fn));
		return NULL;
	}

	memset(urldup, 0, sizeof(header_url));
	
	if (url->display_name)
	{
		urldup->display_name = MMalloc(mallocfn, strlen(url->display_name) + 1);
		strcpy(urldup->display_name, url->display_name);
	}

	if (url->name)
	{
		urldup->name = MMalloc(mallocfn, strlen(url->name) + 1);
		strcpy(urldup->name, url->name);
	}

	if (url->host)
	{
		urldup->host = MMalloc(mallocfn, strlen(url->host) + 1);
		strcpy(urldup->host, url->host);
	}

	if (url->tag)
	{
		urldup->tag = MMalloc(mallocfn, strlen(url->tag) + 1);
		strcpy(urldup->tag, url->tag);
	}

	for(i = 0; i < sizeof(url->url_parameters)/sizeof(SipUrlParameter); ++i)
	{
		if (url->url_parameters[i].name)
		{
			urldup->url_parameters[i].name = 
				MMalloc(mallocfn, strlen(url->url_parameters[i].name) + 1);
			strcpy(urldup->url_parameters[i].name, url->url_parameters[i].name);

			if (url->url_parameters[i].value)
			{
				urldup->url_parameters[i].value = 
						MMalloc(mallocfn, strlen(url->url_parameters[i].value) + 1);
				strcpy(urldup->url_parameters[i].value, url->url_parameters[i].value);
			}
		}
	}

	for(i = 0; i < sizeof(url->priv_params)/sizeof(SipPrivacyParams); ++i)
	{
		if (url->priv_params[i].name)
		{
			urldup->priv_params[i].name = 
				MMalloc(mallocfn, strlen(url->priv_params[i].name) + 1);
			strcpy(urldup->priv_params[i].name, url->priv_params[i].name);

			if (url->priv_params[i].value)
			{
				urldup->priv_params[i].value = 
						MMalloc(mallocfn, strlen(url->priv_params[i].value) + 1);
				strcpy(urldup->priv_params[i].value, url->priv_params[i].value);
			}
		}
	}

	urldup->port = url->port;
	urldup->type = url->type;
	urldup->realmId = url->realmId;

	if( url->header )
	{
		urldup->header = MMalloc(mallocfn, strlen(url->header)+1);
		strcpy(urldup->header, url->header );
	}

	return urldup;
}

int
SipValidateFrom(header_url *url)
{
	if (url == NULL)
	{
		return -1;
	}

	if ((url->host == NULL) || !strlen(url->host))
	{
		return -1;
	}

	return 1;
}

int
SipValidateTo(header_url *url)
{
	if (url == NULL)
	{
		return -1;
	}

#if 0
	if ((url->name == NULL) || !strlen(url->name))
	{
		return -1;
	}
#endif

	if ((url->host == NULL) || !strlen(url->host))
	{
		return -1;
	}

	return 1;
}

SipBool 
SipExtractContact(
	 SipMessage *m, 
	 header_url **contact_url, 
	 SipError *err
)
{
	char fn[] = "SipExtractContact";
	SipHeader *header = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	int count=0;

	if( sip_getHeaderCount(m, SipHdrTypeContactAny, &count, err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to geet contact hdr count\n",fn));
		return SipFail;
	}
	if(count == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s found no contact hdr\n",fn));
		return SipSuccess;
	}

	/* allocate memory */
	*contact_url = (header_url *) malloc (sizeof(header_url));
	if (*contact_url == (header_url *)NULL)
	{
		NETERROR(MSIP, ("%s fail to malloc contact url\n", fn));
		goto _error;
	}
	memset(*contact_url, 0, sizeof(header_url));

	/* Extract the contact header first */
	if( sip_initSipHeader(&header, SipHdrTypeAny, err)==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip contact header\n",fn));
		goto _error;
	}

	if( sip_getHeaderAtIndex(m, SipHdrTypeContactAny, header, (SIP_U32bit)0, err)
	    == SipFail)
	{
		NETERROR(MSIP, ("%s could not get any contact header\n", fn));
		goto _error;
	}
	
	if (sip_getAddrSpecFromContactHdr(header, &addrspec, err)==SipFail)
	{
		NETERROR(MSIP, ("%s Could not get addrspec\n", fn));
		goto _error;
	}

	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail)
	{
		NETERROR(MSIP, ("%s Could not get Sip Url from Addr Spec\n", fn));
		goto _error;
	}

	/* get 'display name' from the url */
	if ((sip_getDispNameFromContactHdr(header, &((*contact_url)->display_name), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
				 ("Could not get display name from contact header %d\n", *err));
	}

	/* get 'to name' and 'to host' from the url */
	if (sip_getUserFromUrl(sipurl, &((*contact_url)->name), err)==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			 ("%s no user in contact hdr\n", fn));
		(*contact_url)->name = NULL;
	}

	if ((sip_getHostFromUrl(sipurl, &((*contact_url)->host), err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not get host from Sip Url\n", fn));
		goto _error;
	}

	if ((sip_getPortFromUrl(sipurl, &((*contact_url)->port), err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s no port in contact hdr\n", fn));
		(*contact_url)->port = 0;
	}

	SipExtractUrlParamsFromUrl(sipurl, (SipUrlParameter*)&((*contact_url)->url_parameters));

	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);
	
	return SipSuccess;
 _error:
	SipCheckFree(*contact_url);
	*contact_url = NULL;
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header) ;
	SipCheckFree(header);

	return SipFail;
}

SipBool 
SipExtractContactList(
	 SipMessage *m, 
	 header_url_list **contact_url_list, 
	int realmId,
	 SipError *err
)
{
	char fn[] = "SipExtractContactList";
	SipHeader *header = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	header_url *contact_url = NULL;
	header_url_list *list_entry = NULL;
	int i, count = 0;

	if( sip_getHeaderCount(m, SipHdrTypeContactAny, &count, err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to geet contact hdr count\n",fn));
		return SipFail;
	}
	if(count == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s found no contact hdr\n",fn));
		return SipSuccess;
	}

	for(i = 0; i < count; ++i)
	{
		/* allocate memory */
		list_entry = malloc (sizeof(header_url_list));
		if (list_entry == NULL)
		{
			NETERROR(MSIP, ("%s fail to malloc contact list entry\n", fn));
			goto _error;
		}

		contact_url = (header_url *) malloc (sizeof(header_url));
		if (contact_url == (header_url *)NULL)
		{
			NETERROR(MSIP, ("%s fail to malloc contact url\n", fn));
			goto _error;
		}
		memset(contact_url, 0, sizeof(header_url));

		/* Extract the contact header first */
		if( sip_initSipHeader(&header, SipHdrTypeAny, err)==SipFail)
		{
			NETERROR(MSIP, ("%s could not initialize sip contact header\n",fn));
			goto _error;
		}

		if( sip_getHeaderAtIndex(m, SipHdrTypeContactAny, header, i, err) == SipFail)
		{
			NETERROR(MSIP, ("%s could not get any contact header\n", fn));
			goto _error;
		}	

		if (sip_getAddrSpecFromContactHdr(header, &addrspec, err)==SipFail)
		{
			NETERROR(MSIP, ("%s Could not get addrspec\n", fn));
			goto _error;
		}

		if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail)
		{
			NETERROR(MSIP, ("%s Could not get Sip Url from Addr Spec\n", fn));
			goto _error;
		}

		/* get 'display name' from the url */
		if ((sip_getDispNameFromContactHdr(header, &(contact_url->display_name), err))==SipFail)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG3,
					 ("Could not get display name from contact header %d\n", *err));
		}

		/* get 'to name' and 'to host' from the url */
		if (sip_getUserFromUrl(sipurl, &(contact_url->name), err)==SipFail)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				 ("%s no user in contact hdr\n", fn));
			contact_url->name = NULL;
		}

		if ((sip_getHostFromUrl(sipurl, &(contact_url->host), err))==SipFail)
		{
			NETERROR(MSIP, ("%s Could not get host from Sip Url\n", fn));
			goto _error;
		}

		if ((sip_getPortFromUrl(sipurl, &(contact_url->port), err))==SipFail)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s no port in contact hdr\n", fn));
			contact_url->port = 0;
		}

		SipExtractUrlParamsFromUrl(sipurl,(SipUrlParameter*)&(contact_url->url_parameters));

		contact_url->realmId = realmId;

		list_entry->url = UrlDup(contact_url, MEM_LOCAL);

		if (*contact_url_list)
		{
			ListInsert((*contact_url_list)->prev, list_entry);
		}	
		else
		{
			ListInitElem(list_entry);
			*contact_url_list = list_entry;
		}

		sip_freeSipHeader(header) ;
		SipCheckFree(header);
		SipCheckFree(contact_url);
		sip_freeSipAddrSpec(addrspec);
		sip_freeSipUrl(sipurl);
	}
	
	return SipSuccess;
 _error:
	SipCheckFree(contact_url);
	SipCheckFree(list_entry);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipHeader(header) ;
	SipCheckFree(header);

	return SipFail;
}

header_url *
SipPopUrlFromContactList(
	 header_url_list **contact_url_list,
	 int freefn
)
{
	header_url *contact_url = NULL;
	header_url_list *list_entry = NULL;

	if(*contact_url_list)
	{
		if((list_entry = *contact_url_list))
		{
			contact_url = list_entry->url;

			if(ListIsSingle(list_entry))
			{
				*contact_url_list = NULL;
			}
			else
			{
				*contact_url_list = list_entry->next;
			}

			ListDelete(list_entry);

			SipCheckFree2(list_entry, freefn);
		}
	}

	return contact_url;
}

/* return 0 if no error, -1 on error */
int SipFormatAck(SipMessage **ack, SipMessage *invite, SipMessage *resp)
{
	char fn[]="SipFormatAck";
	SipMessage *ackptr = NULL;
	SipHeader *header = NULL, *headernew = NULL;
	SipReqLine *pReqLine = NULL, *reqlinenew=NULL;
	SipError err;
	SIP_U32bit count, i, j;
	SIP_U8bit **addrs = NULL;
	char *route;
	header_url *contact = NULL;
	SipCallLegKey	callLeg;
	CallHandle *callHandle;
	int rc = -1;

	bzero(&callLeg, sizeof(SipCallLegKey));

	if( sip_initSipMessage(ack, SipMessageRequest, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error in init msg\n", fn));
		goto _error;
	}

	ackptr = *ack;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init from hdr\n",fn));
		goto _error;
	}

	if (sip_getHeader(invite, SipHdrTypeMaxforwards, header, &err) != SipFail)
	{
		if ( sip_setHeader(ackptr, header, &err) == SipFail )	
		{
			NETERROR(MSIP, ("%s fail to get from hdr\n",fn));
		}
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init from hdr\n",fn));
		goto _error;
	}

	/* Scan and copy From hdr from resp */

	if ( sip_getHeader(resp, SipHdrTypeFrom, header, &err) == SipFail )	
	{
		NETERROR(MSIP, ("%s fail to get from hdr\n",fn));
		goto _error;
	}

	if (sip_setHeader(ackptr, header, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set from hdr\n",fn));
		goto _error;
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init To hdr\n",fn));
		goto _error;
	}

	/* Scan and copy To hdr from resp */

	if ( sip_getHeader(resp, SipHdrTypeTo, header, &err) == SipFail )
	{
		NETERROR(MSIP, ("%s fail to get to hdr\n",fn));
		goto _error;
	}

	if (sip_setHeader(ackptr, header, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set to hdr\n",fn));
		goto _error;
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init call id hdr\n",fn));
		goto _error;
	}

	/* Scan and copy Call Id from resp */

	if ( sip_getHeader(resp, SipHdrTypeCallId, header, &err) == SipFail )
	{
		NETERROR(MSIP, ("%s fail to get callID hdr\n",fn));
		goto _error;
	}

	if (sip_setHeader(ackptr, header, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set callID hdr\n",fn));
		goto _error;
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	/* Allocate and Initialize header for scanning */

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init Cseq hdr\n",fn));
		goto _error;
	}

	/* clone and modify CSeq from resp */	

	if ( sip_getHeader(resp, SipHdrTypeCseq, header, &err) == SipFail )
	{
		NETERROR(MSIP, ("%s fail to get Cseq hdr\n",fn));

		sip_freeSipHeader(header);
		SipCheckFree(header);
		header = NULL;

		goto _error;
	}

	/* Allocate and Initialize headernew for scanning */

	if (sip_initSipHeader(&headernew, SipHdrTypeCseq, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init Cseq hdr\n",fn));

		sip_freeSipHeader(header);
		SipCheckFree(header);
		header = NULL;

		goto _error;
	}

	/* clone header */

	if( sip_cloneSipHeader(headernew, header, &err) == SipFail )
	{
		NETERROR(MSIP, ("%s fail to clone Cseq hdr\n",fn));

		sip_freeSipHeader(header);
		SipCheckFree(header);
		header = NULL;

		sip_freeSipHeader(headernew);
		SipCheckFree(headernew);
		headernew = NULL;

		goto _error;
	}

	if( sip_setMethodInCseqHdr(headernew, strdup("ACK"), &err) == SipFail )
	{
		NETERROR(MSIP, ("%s fail to set ack in cseq hdr\n",fn));


		sip_freeSipHeader(header);
		SipCheckFree(header);
		header = NULL;

		sip_freeSipHeader(headernew);
		SipCheckFree(headernew);
		headernew = NULL;

		goto _error;
	}

	if (sip_setHeader(ackptr, headernew, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to set Cseq hdr\n",fn));

		sip_freeSipHeader(header);
		SipCheckFree(header);
		header = NULL;

		sip_freeSipHeader(headernew);
		SipCheckFree(headernew);
		headernew = NULL;

		goto _error;
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	header = NULL;

	sip_freeSipHeader(headernew);
	SipCheckFree(headernew);
	headernew = NULL;

	/* clone and modify request-uri from invite */

	if( sip_getReqLine(invite, &pReqLine, &err) == SipFail) 
	{
		NETERROR(MSIP, ("%s Error getting ReqLine from SipReqMsg\n", fn));
		goto _error;;
	}

	if( sip_initSipReqLine(&reqlinenew, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error initializing reqlinenew\n", fn));
		goto _error;
	}

	if( sip_cloneSipReqLine(reqlinenew, pReqLine, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not clone ReqLine\n", fn));
		goto _error;
	}

	if( sip_setMethodInReqLine(reqlinenew, strdup("ACK"), &err) == SipFail) 
	{
		NETERROR(MSIP, ("%s Error setting method in ReqLine\n", fn));
		goto _error;;
	}

	if( sip_setReqLine(ackptr, reqlinenew, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not set ReqLine in msg\n", fn));
		goto _error;
	}

	sip_freeSipReqLine(pReqLine);
	sip_freeSipReqLine(reqlinenew);

	/* via */
	/* Allocate and Initialize header for scanning */
	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to init HdrTypeVia hdr\n",fn));
		goto _error; 
	}

	if (sip_getHeader(invite, SipHdrTypeVia, header, &err) != SipFail)
	{
        if ((sip_insertHeaderAtIndex(ackptr, header, 0, &err))==SipFail)        
        {       
            NETERROR(MSIP, ("%s fail to get from hdr\n",fn));
        }       
    }

    sip_freeSipHeader(header);
    SipCheckFree(header);
    header = NULL; 

	/* check for record route */
	if (sip_getHeaderCount(resp, SipHdrTypeRecordRoute, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get RR count\n",fn));
		goto _error;
	}

	if(count != 0)
	{
		addrs = calloc(count + 1, sizeof(*addrs));

		for(i = 0; i < count; ++i)
		{
			if( (SipExtractHdrAsString(resp, SipHdrTypeRecordRoute, 
						   &route, i)) <= 0 )
			{
				NETERROR(MSIP, ("%s fail to get RR hdr from index %d\n", 
						fn, i));
				goto _error;
			}

			addrs[count-1-i] = strdup(route + strlen("Record-"));

			SipCheckFree(route);
		}

		if (SipExtractContact(resp, &contact, &err) == SipFail)
		{
			 NETERROR(MSIP,
					  ("%s ExtractContactHeaders failed\n", fn));

			 goto _error;
		}

		SipSetRoute(ackptr, count - 1, (char**)addrs + 1, contact);
	}
	else
	{
		if (SipGetCallID(resp, &callLeg.callid, &err) == SipFail)
		{
			 NETERROR(MSIP,
					  ("%s ExtractContactHeaders failed\n", fn));

			 goto _error;
		}

		if (SipExtractFromUri(resp, &callLeg.local, &err) == SipFail)
		{
			 NETERROR(MSIP,
					  ("%s ExtractContactHeaders failed\n", fn));

			 goto _error;
		}

		if (SipExtractToUri(resp, &callLeg.remote, &err) == SipFail)
		{
			 NETERROR(MSIP,
					  ("%s ExtractContactHeaders failed\n", fn));

			 goto _error;
		}
	
		CacheGetLocks(sipCallCache,LOCK_READ,LOCK_BLOCK);

		callHandle = CacheGet(sipCallCache, &callLeg);

		if(callHandle && callHandle->handle.sipCallHandle.routes.routes_len > 0)
		{
			SipSetRoute(ackptr, callHandle->handle.sipCallHandle.routes.routes_len - 1, callHandle->handle.sipCallHandle.routes.routes_val + 1, callHandle->handle.sipCallHandle.remoteContact);

		}

		CacheReleaseLocks(sipCallCache);
	}

	rc = 0;

 _error:
	if(addrs)
	{
		for(i = 0; addrs[i]; ++i) SipCheckFree(addrs[i]);

		SipCheckFree(addrs);
	}

	SipCheckFree(contact);
	SipCheckFree(callLeg.callid);
	SipCheckFree(callLeg.local);
	SipCheckFree(callLeg.remote);

	return rc;
} 

int
SipDumpMessage(SipMessage *s)
{
	char fn[] = "SipDumpMessage():";
	char msg[SIP_MAX_MSG_SIZE];
	SIP_U32bit dLength;
	SipError err;
	extern SipOptions sip_options;

	if (sip_formMessage( s, &sip_options, msg, &dLength, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error\n", fn));
	}
	else
	{
		// Print out the while message
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("message- \n"));
		NETDEBUG(MSIP, NETLOG_DEBUG4, (msg));
	}
	return(0);
}

/* remove the leading "+" sign for username and phone-context and stick
 * phone-context before username
 */
int SipPhoneContextPlus(SIP_S8bit **username)
{
	SIP_S8bit *copyusername, *phonecontext, *p;
	char fn[]="SipPhoneContextPlus";
	
	if(*username == NULL)
		return 0;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s entering user name=%s\n", fn, *username));

	p = copyusername = strdup(*username);

	while((*p++ = tolower(*p)));

	if( (phonecontext = strstr(copyusername, ";phone-context=")) != NULL)
	{
		*phonecontext = '\0';
		phonecontext += strlen(";phone-context=");
		if(*phonecontext == '+')
		{
			++phonecontext;
		}

		strcpy(*username, phonecontext);

		if ((p = strchr(*username, ';')) != 0)
		{
			*p = '\0';
		}

		strcat(*username, *copyusername == '+' ? (copyusername + 1) : copyusername);

		if ((p = strchr(phonecontext, ';')) != 0)
		{
			strcat(*username, p);
		}
	}
	else if(*copyusername == '+')
	{
		strcpy(*username, (copyusername + 1));
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s: new user name=%s \n", fn, *username));

	SipCheckFree(copyusername);

	return 0;
}


/* extract i-th header of type dType as string and store in strptr
 * memory of strptr must be allocated by calling fn
 * return 1 on success, 0 on hdr not found, -1 on error 
 */
int SipExtractHdrAsString(SipMessage *m, en_HeaderType dType, char **str, int i)
{
	char fn[]="SipExtractHdrAsString";
	char *strptr = NULL;
	int j;
	SipError err;

	if( (sip_getHeaderAsStringAtIndex(m, dType, str, i, &err)) == SipFail )
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			 ("%s no %d hdr found\n", fn, dType));
		return 0;
	}
	else
	{
		/* remove trailing '\r' and '\n' */
		strptr = *str;
		for(j=0;j<strlen(strptr);j++)
		{
			if(strptr[j] == '\r' || strptr[j] == '\n')
			{
				strptr[j] = '\0';
				break;
			}
		}
		return 1;
	}
}

SipBool 
SetSipCustomHeader(SipMessage *s, SIP_S8bit *name, SIP_S8bit *body, SipError *err) 
{
	SipHeader *header;

	if (( sip_initSipHeader (&header, SipHdrTypeUnknown, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error initializing SipHeader for custom header %d\n",*err));
		return SipFail;
	}

	if (( sip_setNameInUnknownHdr (header, name, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error initializing name in SipHeader for custom header %d\n",*err));
		sip_freeSipHeader(header) ;  
		SipCheckFree(header);
		return SipFail;
	}

	if (( sip_setBodyInUnknownHdr (header, body, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error initializing body in SipHeader for custom header %d\n",*err));
		sip_freeSipHeader(header) ;  
		SipCheckFree(header);
		return SipFail;
	}

	if ((sip_insertHeaderAtIndex(s, header, 0, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set custom header in message %d\n", *err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}

	sip_freeSipHeader(header) ;  
	SipCheckFree(header);
	return SipSuccess;
}

SipBool 
GetSipCustomHeader(SipMessage *s, SIP_S8bit *name, SIP_S8bit **body, SipError *err) 
{
	SipHeader *header;
	SIP_U32bit count, index;

	*body = NULL;

	if ((sip_getHeaderCount(s, SipHdrTypeUnknown, &count, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get count of unknown header %d. May not be present\n", *err));
		return SipSuccess;
	}

	if (( sip_initSipHeader (&header, SipHdrTypeAny, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error initializing SipHeader for custom header %d\n",*err));
		return SipFail;
	}

	for(index = 0; index < count; ++index)
	{
		if ((sip_getHeaderAtIndex(s, SipHdrTypeUnknown, header, index, err)) == SipFail)
		{
			NETERROR(MSIP, ("Error getting Header of type unknown %d\n",*err));
			sip_freeSipHeader(header) ;  
			SipCheckFree(header);
			return SipFail;
		}

		if(strcmp(((SipUnknownHeader*)(header->pHeader))->pName, name) == 0)
		{
			*body = strdup(((SipUnknownHeader*)(header->pHeader))->pBody);
			break;
		}
	}

	sip_freeSipHeader(header) ;  
	SipCheckFree(header);
	return SipSuccess;
}

// Will return the string format for codec,
// if there is one needed. 
// Note: NULL is the most common return value !!
char *
SipMediaTypeAsStr(int mediaType)
{
	switch (mediaType)
	{
	case MediaAudio:
		return "audio";
	case MediaImage:
		return "image";
	case MediaVideo:
		return "video";
	}

	return "audio";
}

char *
SipTransportAsStr(int transport)
{
	switch (transport)
	{
	case TransportRTPAVP:
		return "RTP/AVP";
	case TransportUDPTL:
		return "udptl";
	}

	return "RTP/AVP";
}

int
SipMediaFmtToCodec(char *fmt)
{
	if (!strcmp(fmt, "t38"))
	{
		return T38Fax;
	}

	return atoi(fmt);
}

MediaType
SipMediaType(char *media)
{
	MediaType type = MediaAudio;

	if(media)
	{
		if(strcasecmp(media, "audio") == 0)
		{
			type = MediaAudio;
		}
		else if(strcasecmp(media, "image") == 0)
		{
			type = MediaImage;
		}
		else if(strcasecmp(media, "video") == 0)
		{
			type = MediaVideo;
		}
	}

	return type;
}

int
SipExtractUrlParamsFromUrl(SipUrl *sipurl, SipUrlParameter *url_parameters)
{
	char fn[]="SipExtractUrlParamsFromUrl";
	int count, i, j;
	SipParam *urlParam = NULL;
	SipError err;
	char *name, *value;

	/* Go over the url parameters one by one */
	if (sip_getUrlParamCountFromUrl(sipurl, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get urlparam count\n", fn));
		return 0;
	}

	if (count == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s No url parameters found\n", fn));
		return 0;
	}

	for (i = 0, j = 0; (i < count)&&(j < SIP_MAXURLPARMS); i++)
	{
		if (sip_getUrlParamAtIndexFromUrl(sipurl, &urlParam, i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get urlparam at %d\n", fn, i));
			continue;
		}

		/* extract name */
		if (sip_getNameFromSipParam(urlParam, &name, &err) != SipFail)
		{
			(url_parameters[j]).name = name;

			/* extract value */
			if (sip_getValueAtIndexFromSipParam(urlParam, &value, 0, &err) != SipFail)
			{
				(url_parameters[j]).value = value;
			}
			else
			{
				(url_parameters[j]).value = NULL;
			}
		}
		else
		{
			NETERROR(MSIP, ("%s fail to get name from urlparam\n", fn));
		}

		++j;

		sip_freeSipParam(urlParam);
		urlParam = NULL;
	}

	sip_freeSipParam(urlParam);
	return 0;
}

int
SipSetMinSE (SipMessage *s, int minSE)
{
	char fn[] = "SipSetMinSE()";
	SipHeader *header = NULL;
	SipError err;

	if (minSE == 0)
	{
		NETERROR (MSIP, ("%s bad minSE  value  (0)", fn));
		return -1;
	}
	if ((sip_initSipHeader (&header, SipHdrTypeMinSE, &err)) == SipFail)
	{
		NETERROR (MSIP, ("%s could not initialize 'minSE' header\n", fn));
		goto _error;
	}
	if (sip_setSecondsInMinSEHdr (header, minSE, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s Could not set 'minSE' %d ", fn, err));
		goto _error;
	}
	if  (sip_insertHeaderAtPosition  (s,header, 0,0,0, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to insert 'minSE' header", fn));
		goto _error;
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s successfully set minSe header to %d", fn, minSE));
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return 1;
 _error:
	sip_freeSipHeader (header);
	SipCheckFree(header);
	return -1;
}

int
SipSetSupported (SipMessage *s, char *options)
{
	char fn[] = "SipSetSupported ()";
	SipHeader *header = NULL;
	SipError err;

	if ((sip_initSipHeader (&header, SipHdrTypeSupported, &err)) == SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize 'supported' header\n", fn));
		goto _error;
	}
	if (sip_setOptionInSupportedHdr  (header, options, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s could not set option in supported header", fn));
		goto _error;
	}
	if (sip_insertHeaderAtPosition (s, header, 0, 0, 0, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to insert supported header", fn));
		goto _error;
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s successfuly inserted supported:%s header\n", fn, options));
	sip_freeSipHeader (header);
	SipCheckFree(header);
	return 1;
 _error:
	sip_freeSipHeader (header);
	SipCheckFree(header);
	return -1;
}

int
SipSetRequire (SipMessage *s, char *option)
{
	char fn[] = "SipSetRequire()";
	SipHeader *header =NULL;
	SipError err;

	if ((sip_initSipHeader ( &header, SipHdrTypeRequire, &err)) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to init header", fn));
		return -1;
	}
	if (sip_setTokenInRequireHdr  (header, option, &err) == SipFail) 
	{
		NETERROR (MSIP, ("%s failed to set token inrequire header", fn));
		goto _error;
	}
	if (sip_insertHeaderAtPosition( s, header, 0, 0, 0, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to insert require header", fn));
		goto _error;
	}
	sip_freeSipHeader (header);
	SipCheckFree(header);
	return 1;
 _error:
	sip_freeSipHeader  (header);
	SipCheckFree(header);
	return -1;
}

int 
SipSetSessionExpires (SipMessage *s, int sessionExpires,
		      int refresher)
{
	char fn[] = "SipSetSessionExpires()";
	SipHeader *header = NULL;
	SipError err;
	char *srefresher;

	if (refresher == SESSION_REFRESHER_UAC)
		srefresher = "uac";
	else
		srefresher = "uas";

	if ((sip_initSipHeader (&header, SipHdrTypeSessionExpires, &err)) == SipFail)
	{
		NETERROR (MSIP, ("%s could not initialize 'sessionExpires' header\n", fn));
		goto _error;
	}
	if (sip_setSecondsInSessionExpiresHdr (header, sessionExpires, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s Could not set 'sessionExpires second' %d ", fn, err));
		goto _error;
	}
	if (refresher != SESSION_REFRESHER_NONE) 
	{
		if (sip_setRefresherInSessionExpiresHdr (header, strdup(srefresher), &err) == SipFail)
		{
			NETERROR  (MSIP, ("%s could not set 'sessionExpires refresher' %d", fn, err));
			goto _error;
		}
	}
	if  (sip_insertHeaderAtPosition (s, header, 0, 0, 0, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s failed to insert 'sessionExpires' header", fn));
		goto _error;
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s successfully set sessionExpires header to %d", fn, sessionExpires));
	sip_freeSipHeader(header);
	SipCheckFree(header);

	return 1;
 _error:
	sip_freeSipHeader (header);
	SipCheckFree(header);

	return -1;
	
}


int
SipSetMaxForwards(
SipMessage *s, 
int max_forwards
)
{
	char fn[] = "SipSetMaxForwards():";
	SipHeader *header = NULL;
	SipError err;

	if ((sip_initSipHeader(&header, SipHdrTypeMaxforwards, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise 'max forwards' header\n", fn));
		goto _error;
	}

	if (sip_setHopsInMaxForwardsHdr (header, max_forwards, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to set max forwards %d\n",fn, err));
		goto _error;
	}

	if (sip_insertHeaderAtPosition(s, header, 0, 0, 0, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s fail to insert max forwards hdr %d\n",fn, err));
		goto _error;
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);

	return 1;

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);

	return -1;
}


int
SipGetMinSE (SipMessage *s, int *minSE)
{
	char fn[] = "SipGetMinSE";
	SipHeader *header = NULL;
	SipError err;
	int count;

	if ((sip_getHeaderCount (s, SipHdrTypeMinSE, &count, &err)) == SipFail) 
	{
		NETERROR (MSIP, ("%s unable to get minSe hdrcount %d\n", fn, err));
		return -1;
	}
	if (count == 0) 
	{
		NETDEBUG( MSIP, NETLOG_DEBUG4 , ("%s minSE hdr count zero\n", fn));
		return -1;
	}
	if (sip_initSipHeader (&header, SipHdrTypeAny, &err ) == SipFail)
	{
		NETERROR (MSIP, ("%s count not initialize minSe Hdr\n", fn));
		return -1;
	}
	if (sip_getHeader (s, SipHdrTypeMinSE, header, &err) == SipFail)
	{
		NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s could not get minSE hdr\n", fn));
		goto _error;
	}
	if (sip_getSecondsFromMinSEHdr (header, minSE, &err) == SipFail)
	{
		NETERROR  (MSIP, ("%s could not get seconds from hdr\n", fn));
		goto _error;
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s found minSE = %d\n", fn, *minSE));
	sip_freeSipHeader (header);
	SipCheckFree (header);
	return 0;
 _error:
	sip_freeSipHeader (header);
	SipCheckFree (header);
	return -1;
}


int
SipGetSessionExpires (SipMessage  *s, int *sessionExpires,
		      int *refresher)
{
	char fn[] = "SipGetSessionExpires()";
	SipHeader  *header = NULL;
	SipError err;
	char *srefresher;
	int count;

	if ((sip_getHeaderCount (s, SipHdrTypeSessionExpires, &count, &err)) == SipFail)
	{
		NETERROR (MSIP, ("%s unable to get SessionExpires hdr count", fn));
		return -1;
	}
	if (count == 0) 
	{
		NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s session expires hdr = 0\n", fn));
		return -1;
	}
	if (sip_initSipHeader (&header, SipHdrTypeAny, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s could not init session expires hdr", fn));
		return -1;
	}
	if (sip_getHeader (s, SipHdrTypeSessionExpires, header, &err) == SipFail) 
	{
		NETERROR (MSIP, ("%s could not get hdr for session expire", fn));
		goto _error;
	}
	if (sip_getSecondsFromSessionExpiresHdr (header, sessionExpires, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s could not get session expires hdr", fn));
		goto _error;
	}
	if (sip_getRefresherFromSessionExpiresHdr(header, &srefresher, &err) == SipFail)
	{
		NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s could not get session refresher", fn));
		*refresher = SESSION_REFRESHER_NONE;
	} else 
	{
		if (strcasecmp (srefresher, "uac") == 0)
		{
			*refresher = SESSION_REFRESHER_UAC;
		} 
		else if (strcasecmp (srefresher, "uas") == 0) 
		{
			*refresher = SESSION_REFRESHER_UAS;
		}
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s session expires : %d refresher %d", 
					fn, *sessionExpires, *refresher));
	sip_freeSipHeader (header);
	SipCheckFree (header);
	return 1;
 _error:
	sip_freeSipHeader (header);
	SipCheckFree (header);
	return -1;
}

int
SipGetSessionTimerRequire(SipMessage *s,
			  int *timerSupport)
{
	char fn[] = "SipGetSessionTimerRequire";
	SipHeader *header = NULL;
	SipError err;
	char *token;
	int count;

	if ((sip_getHeaderCount (s, SipHdrTypeRequire, &count, &err)) == SipFail)
	{
		NETERROR (MSIP,("%s unable to get require header count.\n", fn));
		return -1;
	}
	if (count == 0)
	{
		return -1;
	}
	if (sip_initSipHeader (&header, SipHdrTypeAny, &err) == SipFail)
	{
		NETERROR (MSIP, ("%s could not init hdr\n", fn));
		return -1;
	}
	if (sip_getHeader (s, SipHdrTypeRequire, header, &err) == SipFail)
	{
		NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s could not get require header", fn));
		goto _error;
	}
	if (sip_getTokenFromRequireHdr(header, &token, &err) == SipFail)
	{
		NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s could not get require header token", fn));
		goto _error;
	}
	if (strcasecmp (token, "timer") == 0)
	{
		*timerSupport = 1;
	} else
	{
		*timerSupport = 0;
	}
	sip_freeSipHeader  (header);
	SipCheckFree (header);
	return 0;
 _error:
	sip_freeSipHeader (header);
	SipCheckFree (header);
	return -1;
}


int 
SipGetSessionTimerSupport (SipMessage *s,
			   int *timerSupport)
{
	char fn[] = "SipGetSessionTimerSupport()";
	SipHeader *header = NULL;
	SipError err;
	int count, ndx;
	char *option;

	if ((sip_getHeaderCount (s, SipHdrTypeSupported, &count, &err)) == SipFail)
	{
		NETERROR ( MSIP , ("%s unable to get supported header count\n", fn));
		return -1;
	}
	if (count == 0) 
	{
		return -1;
	}

	for (ndx=0, *timerSupport=0; (ndx < count) && (*timerSupport == 0); ndx++)
	{
		if (sip_initSipHeader (&header, SipHdrTypeAny, &err) == SipFail)
		{
			NETERROR  (MSIP, ("%s could not init hdr\n", fn));
			return -1;
		}

		if (sip_getHeaderAtIndex (s, SipHdrTypeSupported, header, ndx, &err) == SipFail)
		{
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s could not get supported header.\n", fn));
			goto _error;
		}
		if (sip_getOptionFromSupportedHdr (header, &option, &err) == SipFail)
		{
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s could not get options from supported header", fn));
			goto _error;
		}

		if (strcasecmp (option, "timer") == 0)
		{
			*timerSupport = 1;
		} else 
		{
			*timerSupport = 0;
		}
		sip_freeSipHeader (header);
		SipCheckFree ( header);
	}
	return 0;
 _error:
	sip_freeSipHeader ( header);
	SipCheckFree (header);
	return -1;
}

	
int
SipGetMaxForwards(
SipMessage *s, 
int *max_forwards
)
{
	char fn[]="SipGetMaxForwards:";
	SipHeader *header=NULL;
	SIP_U32bit count;
	SipError err;

	if ((sip_getHeaderCount(s, SipHdrTypeMaxforwards, &count, &err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Unable to get Max-Forward hdr count %d\n", fn, err));
		return -1;
	}

	if (count == 0)
	{
		return -1;
	}

	if (sip_initSipHeader(&header, SipHdrTypeAny, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize Max-Forward Hdr\n",fn));
		return -1;
	}

	if (sip_getHeader(s, SipHdrTypeMaxforwards, header, &err)==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s could not get Max-Forward Hdr\n",fn));
		goto _error;
	}

	if (sip_getHopsFromMaxForwardsHdr(header, max_forwards, &err)==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Fail to get hops from  Max-F Hdr\n",fn));
		goto _error;
	}

	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Found Max-Forwards hdr=%d\n",fn,*max_forwards));

	sip_freeSipHeader(header);
	SipCheckFree(header);
	return 0;

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return -1;
}

char *
SipExtractParmVal(SipHeaderUrl *url, char *parm, int *exists)
{
	int i, n;

	if (url != NULL)
	{
		n = sizeof(url->url_parameters)/sizeof(SipUrlParameter);
		for (i = 0; i < n; i++)
		{
			if (url->url_parameters[i].name &&
				!strcmp(url->url_parameters[i].name, parm))
			{
				if (exists)
				{
					*exists = 1;
				}

				return url->url_parameters[i].value;
			}
		}
	}

	if (exists)
	{
		*exists = 0;
	}

	return NULL;
}

// Add a parm if one does not exist and replace one if
// if it is already there
int
SipAddParm(SipHeaderUrl *url, char *parm, char *val, int memfn)
{
	int i = 0;

	while (i < sizeof(url->url_parameters)/sizeof(SipUrlParameter))
	{
		if (url->url_parameters[i].name)
		{
			if (!strcmp(url->url_parameters[i].name, parm))
			{
				// Exists. We will have to free the value and
				// dup the new one
				SipCheckFree2(url->url_parameters[i].value, memfn);
				url->url_parameters[i].value = MStrdup(memfn, val);
				return 1;
			}
		}
		else
		{
			break;
		}

		i++;
	}

	// Not found, add one
	if (i < sizeof(url->url_parameters)/sizeof(SipUrlParameter))
	{
		url->url_parameters[i].name = MStrdup(memfn, parm);
		url->url_parameters[i].value = MStrdup(memfn, val);

		return 1;
	}

	// Was not able to add the parameter
	return -1;
}


void SipFreeRemotecontactList(header_url_list *remotecontact_list)
{
	header_url_list *elem, *list;

	if(remotecontact_list)
	{
		list = remotecontact_list;
		do
		{
			elem = list;
			list = elem->next;
			UrlFree(elem->url, sipCallCache->free);
			SipCheckFree2(elem, sipCallCache->free);
		}
		while(list != remotecontact_list);
	}
}

/*

  Sip Privacy Header related helper functions

*/


SipBool SipExtractFromPAssertedID( SipMessage *m, header_url **pAssertedID, int hdrCount, SipError *err) {

  SipUrl*      sipurl      = NULL;
  SipHeader*   header      = NULL;
  SipAddrSpec* addrspec    = NULL;
  int          i;	   
  int          sipurifound = 0;

  if ( hdrCount <= 0 ) {
    NETDEBUG(MSIP,NETLOG_DEBUG4,("SipExtractFromAssetedID():Invalid HdrCount "));
    return SipFail;
  }


  for( i=0; i < hdrCount; i++ ){

    if( sip_initSipHeader(&header, SipHdrTypeAny, err) == SipFail) {
      
      NETERROR(MSIP,("SipPAssertedID(): Could not initialize header"));      
      break;
    }    
    
    if( sip_getHeaderAtIndex(m, SipHdrTypePAssertId, header, i, err) == SipFail) {
      
      NETERROR(MSIP,("SipPAssertedID(): Could not extract header"));
      
      sip_freeSipHeader(header) ;
      SipCheckFree(header);
      continue;
    }

    if( sip_getAddrSpecFromPAssertIdHdr(header, &addrspec, err) == SipFail ){
    
      NETERROR(MSIP,("Could not get Addr Spec from PAssertedID Hdr"));
    
      sip_freeSipHeader(header) ;
      SipCheckFree(header);
      sip_freeSipAddrSpec(addrspec);
      continue;
    }

    if (addrspec->dType != SipAddrSipUri || 
	sip_getUrlFromAddrSpec(addrspec, &sipurl, err)==SipFail) {

      NETERROR(MSIP, ("Could not get Sip Url from AddrSpec/From %d\n", *err));

      sip_freeSipHeader(header) ;
      SipCheckFree(header);
      sip_freeSipAddrSpec(addrspec);
      sip_freeSipUrl(sipurl);
      continue;
    }
    
    else {
      sipurifound = 1;
      break;
    }
  }
 
  if( sipurifound ) {

    *pAssertedID = (header_url *) malloc (sizeof(header_url));
    if (*pAssertedID == (header_url *)NULL) {
      NETERROR(MSIP, ("SipExtractFromPAssertedID(): fail to malloc contact url\n"));
      goto _error;
    } 
    memset(*pAssertedID, 0 , sizeof(header_url));

    /* get 'display name' from the url */
    if ((sip_getDispNameFromFromHdr(header, &((*pAssertedID)->display_name), err))==SipFail) {
      NETDEBUG(MSIP, NETLOG_DEBUG3,("Could not get display name from FromHdr %d\n", *err));
    }
  
    /* get ' name' and ' host' from the url */
    if ((sip_getUserFromUrl(sipurl, &((*pAssertedID)->name), err))==SipFail) {
      NETDEBUG(MSIP, NETLOG_DEBUG3,("Could not get User from AssetedHdr %d\n", *err));
    }

    if ((sip_getPortFromUrl(sipurl, &((*pAssertedID)->port), err))==SipFail) {
      NETDEBUG(MSIP, NETLOG_DEBUG3, ("Could not get Port from FromHdr %d\n", *err));
      (*pAssertedID)->port = 0;
    }
    
    SipExtractUrlParamsFromUrl(sipurl, (SipUrlParameter*)&((*pAssertedID)->url_parameters));   
    
    if ((sip_getHostFromUrl(sipurl, &((*pAssertedID)->host), err))==SipFail) {
      
      NETERROR(MSIP, ("Could not get Host from FromHdr %d\n", *err));
      goto _error;
    }    
  }

  sip_freeSipHeader(header);
  SipCheckFree(header);
  sip_freeSipAddrSpec(addrspec);
  sip_freeSipUrl(sipurl);

  if(sipurifound ) {
    return SipSuccess;
  }
  else {
    return SipFail;
  }

 _error:
  SipCheckFree(*pAssertedID);
  *pAssertedID = NULL;
  if(header != NULL) {
    sip_freeSipHeader(header);
    SipCheckFree(header);
  }
  sip_freeSipAddrSpec(addrspec);
  sip_freeSipUrl(sipurl);
  return SipFail;
}

SipBool SipStorePAssertedIDTel( SipMessage* m, char** pAssertedIDTel, int hdrCount, SipError* err) {

  SipHeader*   header      = NULL;
  SipAddrSpec* addrspec    = NULL;
  int          i;	   
  int          telfound    = 0;
  char*        fn          = "SipStorePAssertedIDTel";
           

  if ( hdrCount <= 0 ) {
    NETDEBUG(MSIP,NETLOG_DEBUG4,("SipExtractFromAssetedID():Invalid HdrCount "));
    goto _error;    
  }


  for( i=0; i < hdrCount; i++ ){

    if( sip_initSipHeader(&header, SipHdrTypeAny, err) == SipFail) {
      
      NETERROR(MSIP,("%s:Could not initialize header",fn));      
      goto _error;
    }    
    
    if( sip_getHeaderAtIndex(m, SipHdrTypePAssertId, header, i, err) == SipFail) {
      
      NETERROR(MSIP,("SipPAssertedID(): Could not extract header"));      
      goto _error;
    }

    if( sip_getAddrSpecFromPAssertIdHdr(header, &addrspec, err) == SipFail ){
    
      NETERROR(MSIP,("Could not get Addr Spec from PAssertedID Hdr"));    
      goto _error;
    }
    
    if (addrspec->dType != SipAddrSipUri ) {
      NETDEBUG(MSIP, NETLOG_DEBUG4, ("We received PAI Tel URL"));
      
      if( (sip_getHeaderAsStringAtIndex(m,SipHdrTypePAssertId,pAssertedIDTel,i,err)) == SipFail ) {
              NETERROR(MSIP,("Failed to Retrieve Hdr as string - hdr type %d",SipHdrTypePAssertId));
              goto _error;              
      }    
      telfound = 1;      
    }
            
    sip_freeSipHeader(header);
    SipCheckFree(header);
    sip_freeSipAddrSpec(addrspec);    

    if(telfound){
            return SipSuccess;
    }    
  }

  return SipFail;  

 _error:
  if(header) {
          sip_freeSipHeader(header);
          SipCheckFree(header);          
  }
  header = NULL;
  sip_freeSipAddrSpec(addrspec);
  return SipFail;
}


SipBool SipExtractFromPrivacy( SipMessage *m, char **priv_value, SipError *err) {

  
  SipHeader*   		header     = NULL;
  SipNameValuePair*     pNameValue = NULL;
  SIP_U32bit   		privCount, i;
  
  if( sip_initSipHeader(&header, SipHdrTypeAny, err) == SipFail) {
    
    NETERROR(MSIP,("SipExtractFromPrivacy(): Could not initialize header"));
    goto _error;
  }

  if( sip_getHeader(m, SipHdrTypePrivacy, header, err) == SipFail) {

    NETERROR(MSIP,("SipExtractFromPrivacy(): Could not extract header"));    
    goto _error;
  }

  if( sip_getNameValuePairCountFromPrivacyHdr(header,&privCount,err) == SipFail) {

    NETERROR(MSIP,("SipExtractFromPrivacy(): failed to retrieve name value pair count"));
    goto _error;
  }

  for( i = 0; i < privCount; i++) {
    
    pNameValue = NULL;
    if(sip_getNameValuePairAtIndexFromPrivacyHdr(header,&pNameValue,i,err) == SipSuccess) {
      // for now , we will only process privacy: id only
      if(pNameValue->pName && strcmp(pNameValue->pName,"id")==0){
	*priv_value = strdup(pNameValue->pName);
      }      
    }
    sip_freeSipNameValuePair(pNameValue);
  }

  sip_freeSipHeader(header) ;
  SipCheckFree(header);
  return SipSuccess;

 _error:
  sip_freeSipHeader(header) ;
  SipCheckFree(header);
  return SipFail;
}


SipBool SipIsHdrTypePresent(SipMessage* msg, int hdrType, int* count, SipError* err) {

  if( sip_getHeaderCount(msg,hdrType,count,err) == SipFail) {
    
    return SipFail;
  }

  if(*count > 0) {    
    return SipSuccess;
  }
  return SipFail;
}

SipBool SipIsPAssertedIDPresent(SipMessage* msg,int* count, SipError* err) {

  return SipIsHdrTypePresent(msg, SipHdrTypePAssertId, count, err);
}


SipBool SipIsPrivacyPresent(SipMessage* msg,int* count, SipError* err) {

  return SipIsHdrTypePresent(msg, SipHdrTypePrivacy, count, err);
}


SipBool SipIsRemotePartyIdPresent(SipMessage* msg,int* count, SipError* err) {

  return SipIsHdrTypePresent(msg, SipHdrTypeDcsRemotePartyId, count, err);
}

SipBool SipIsProxyRequirePresent(SipMessage* msg,int* count, SipError* err) {

  return SipIsHdrTypePresent(msg, SipHdrTypeProxyRequire, count, err);
}

SipBool SipConvertRFC3325ToDraft01Hdrs(SipMessage *s, header_url* pAssertedID_Sip, 
                                       SipPrivacyLevel privacy_level, SipError* err) {

        return CopyHdrToRPID(s,pAssertedID_Sip,privacy_level,err);        
}

SipBool CopyHdrToRPID(SipMessage* s, header_url* hdr, SipPrivacyLevel privacy_level, SipError* err) 
{        
  char      	*fn       = "SipConvertRFC3325ToDraft01Hdrs";
  SipHeader 	*header   = NULL;
  SipUrl        *sipurl   = NULL;
  SipAddrSpec   *addrspec = NULL;
  SipParam      *rparam   = NULL;
  SipError       erlocal;
  char*         ptr       = NULL;


  if(!hdr) {
    NETERROR(MSIP,("%s(): Null P-Asserted-ID passed",fn));
    return SipFail;
  }

  if (sip_initSipUrl(&sipurl, err) == SipFail) {
    NETERROR(MSIP, ("%s Could not initialise Sip Url\n", fn));
    goto _error;
  }

  if(hdr->host) {
    if ((sip_setHostInUrl(sipurl, strdup(hdr->host), err))==SipFail) {
      NETERROR(MSIP, ("%s fail to  set hostname in url of from header\n", fn));
    goto _error;
    }
  }
  if (hdr->name) {
    if ((sip_setUserInUrl(sipurl, strdup(hdr->name), err))==SipFail) {
      NETERROR(MSIP, ("%s Could not set user name in url\n", fn));
    goto _error;
    }
  }  
  if (hdr->port != 0)  {
    if ((sip_setPortInUrl(sipurl,hdr->port, err))==SipFail) {
	NETERROR(MSIP, ("%s Could not set port number in url\n", fn));
    goto _error;
    }
  }

  // Have to set url parameters
  if( (sip_initSipHeader(&header, SipHdrTypeDcsRemotePartyId, err) == SipFail)) {
    NETERROR(MSIP,("SipConvertRFC3325ToDraft01Hdrs():Unable to init rpid hdr type"));
    goto _error;
  }
  
  if ((sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err))==SipFail)  {
    NETERROR(MSIP,("%s Could not initialise Addr Spec for the Req Line %d\n",fn, *err));
    goto _error;
  }
  
  if ((sip_setUrlInAddrSpec(addrspec,sipurl,err))==SipFail)     {
    NETERROR(MSIP, ("Could not write URI in Addr Spec %d\n", *err));
    goto _error;
  }
  
  if( sip_dcs_setAddrSpecInDcsRemotePartyIdHdr(header,addrspec,err) == SipFail) {

    NETERROR(MSIP, ("%s Could not set Addr Spec in RPID %d\n", fn, *err));
    sip_freeSipHeader(header);
    sip_freeSipUrl(sipurl);
    sip_freeSipAddrSpec(addrspec) ;
    return SipFail;    
  }
  
  // Set Display Name if available
  if( hdr->display_name != NULL ) {    
    sip_dcs_setDispNameInDcsRemotePartyIdHdr(header, strdup(hdr->display_name), err );
  }

  /* set privacy param */
  if (sip_initSipParam(&rparam, &erlocal) == SipFail){
    NETERROR(MSIP, ("%s Could not init url param\n",fn));
    goto _error;
  }
  
  if (sip_setNameInSipParam(rparam, strdup("privacy"), &erlocal) == SipFail)  {
    NETERROR(MSIP, ("%s Could not privacy param\n",fn));
    goto _error;
  }

  if(privacy_level == privacyLevelId)
  {
          ptr = strdup("full");
  }
  else if(privacy_level == privacyLevelNone) 
  {
          ptr = strdup("off");
  }
  if(ptr)
  {        
          if (sip_insertValueAtIndexInSipParam(rparam, ptr, 0, &erlocal) == SipFail)	{
                  NETERROR(MSIP, ("Could not set value url param\n"));
                  goto _error; 
          }
  }

  if (sip_dcs_insertParamAtIndexInDcsRemotePartyIdHdr(header,rparam,0, &erlocal) == SipFail) {
    NETERROR(MSIP, ("Could not param to rpid header\n"));
    goto _error;   
  }

  if(sip_insertHeaderAtIndex(s,header,0,&erlocal) == SipFail) {
    NETERROR (MSIP, ("%s failed to insert rpid header", fn));
    goto _error;
  }

  sip_freeSipHeader(header);
  SipCheckFree(header);
  sip_freeSipUrl(sipurl);
  sip_freeSipAddrSpec(addrspec) ;
  sip_freeSipParam(rparam);
  return SipSuccess;

 _error:
  sip_freeSipHeader(header);
  SipCheckFree(header);
  sip_freeSipUrl(sipurl);
  sip_freeSipAddrSpec(addrspec) ;
  sip_freeSipParam(rparam);
  return SipFail;
}

SipBool SipSetProxyRequire(SipMessage* s, SipError* err) {

  char        *fn     = "SipSetProxyRequire";
  SipHeader   *header = NULL;


  if( (sip_initSipHeader(&header, SipHdrTypeProxyRequire, err) == SipFail)) {
    NETERROR(MSIP,("%s:Unable to init proxy-require hdr type",fn));
    goto _error;
  }

  if( sip_setTokenInProxyRequireHdr(header,strdup("privacy"),err) == SipFail) {
    NETERROR(MSIP,("%s:Unable to init proxy-require hdr type",fn));
    goto _error;
  }

#if 0
    if (sip_insertHeaderAtPosition(s, header,0,0,0,err) == SipFail)	{
#endif
  if ( sip_insertHeaderAtIndex(s,header,0,err) == SipFail ){

    NETERROR (MSIP, ("%s failed to insert proxy-require header", fn));
    goto _error;
  }

  sip_freeSipHeader(header);
  SipCheckFree(header);
  return SipSuccess;

 _error:
  sip_freeSipHeader(header);
  SipCheckFree(header);
  return SipFail;
}


void SipPrivacyModifyMsgHandle(SipMessage* s,SipAppMsgHandle* appMsgHandle,header_url* from) {

  // Changes from in hte msg handle if required

  
  header_url* ptr = NULL;

  if(appMsgHandle->generate_cid == cid_block)
  {
          SipFormFromHdr(appMsgHandle,from);
  } 
  else {
          
  switch(appMsgHandle->incomingPrivType) {

  case privacyTypeRFC3325:
    {

      switch(appMsgHandle->privTranslate) {
      case privTranslateRFC3325ToDraft01:
	{
	  if( appMsgHandle->privLevel == privacyLevelId ) {

	    // fix display name
	    if(from->display_name != NULL) {
	      if( strlen(from->display_name) && strcasecmp(from->display_name,"anonymous")!=0){
		free(from->display_name);
	      }
	      from->display_name  = strdup("Anonymous");
	    }
	    if(from->name != NULL ) {
	      if(strlen(from->name) && strcasecmp(from->name,"anonymous")!=0 ) {		
		free(from->name);
		from->name = strdup("anonymous");
	      }
	    }
	    else {
	      from->name = strdup("anonymous");
	    }

	    if(from->host != NULL ) {
	      if(strlen(from->host) && strcasecmp(from->host,"localhost") != 0 ) {
		free(from->host);
		from->host = strdup("localhost");
	      }
	    }
	  }
	  else if( appMsgHandle->privLevel == privacyLevelNone ) {

	     // fix display name
	    if(from->display_name != NULL && strlen(from->display_name)) {
	      free(from->display_name);
	    }
	    from->display_name = NULL;
	    if(appMsgHandle->pAssertedID_Sip->display_name && 
	       strlen(appMsgHandle->pAssertedID_Sip->display_name)) {
	      from->display_name = strdup(appMsgHandle->pAssertedID_Sip->display_name);
	    }

	    if(from->name != NULL && strlen(from->name) ) {
		free(from->name);
	    }
	    from->name = NULL;
	    if(appMsgHandle->pAssertedID_Sip->name && strlen(appMsgHandle->pAssertedID_Sip->name)) {
	      from->name = strdup(appMsgHandle->pAssertedID_Sip->name);
	    }
            /*	    
	    if(from->host != NULL ) {
	      free(from->host);
	    }
	    from->host = NULL;
	    if(appMsgHandle->pAssertedID_Sip->host && strlen(appMsgHandle->pAssertedID_Sip->host)) {
	      from->host = strdup(appMsgHandle->pAssertedID_Sip->host);
	    }
            */
	  }
	}
	break;
      case privTranslateNone:
	// We should not enter here.
	break;
      case privTranslateDraft01ToRFC3325:
	// Unsupported at this time
	break;
      }
    }
    break;

  case privacyTypeDraft01:
    {
      // We are going to let it go through.
      // Simply copy from header to the destination
      // NO Msg Handle manipulation is required
    }
    break;

  case privacyTypeNone:
    {
    }
    break;
  }
  }
  

  return;
}



void PrivacyTranslateHdrs(SipMessage* m, SipAppMsgHandle* AppMsgHandle) {
  
  SipError err;

  
  switch(AppMsgHandle->incomingPrivType) 
    {

    case privacyTypeRFC3325:
      {
	// Incoming Message Has privacy headers in it
	  
	switch (AppMsgHandle->privTranslate) 
	  {
	    
	  case privTranslateRFC3325ToDraft01: 
	    {	                           

                    if(SipConvertRFC3325ToDraft01Hdrs(m,AppMsgHandle->pAssertedID_Sip,
                                                      AppMsgHandle->privLevel,&err ) == SipFail) {
                            NETERROR(MSIP,("privacy conversion failed"));
                    }
                    if(SipSetProxyRequire(m,&err) ==SipFail) {
                            NETERROR(MSIP,("Proxy Require header not inserted"));
                    }

	    }
	    break;
	  
	  case privTranslateNone            :   
	    {
	      // No Translation Required copy the incomig headers to outgoing side
	      if(SipCopyRFC3325HdrsToDestination(m,AppMsgHandle,&err) == SipFail) {
	      }
	    }
	    break;
	  case privTranslateDraft01ToRFC3325: 
	    {
	      NETERROR(MSIP,("Incompatible privacy conversion requested"));
	    }
	    break;
	  }	
      }
      break;
	
    case privacyTypeDraft01: 
      {
	switch(AppMsgHandle->privTranslate) 
	  {
	  case privTranslateRFC3325ToDraft01:
	    NETERROR(MSIP,("Incompatible privacy conversion requested"));
	    break;
	  case privTranslateNone:
	    if(SipCopyDraft01HdrToDestination(m,AppMsgHandle,&err) == SipFail){
	      NETERROR(MSIP,("Error copying draft01 hdrs to destination"));
	    }
	    break;
	  case privTranslateDraft01ToRFC3325:
	    // Not supported 
	    break;
	  }
      }
      break;

    case privacyTypeNone   :
      // No privacy headers present in the incoming message
      // Nothing to do	
      break;
    default                :
      break;
    }

  return;

}

SipBool SipCopyRFC3325HdrsToDestination(SipMessage* m, SipAppMsgHandle* appMsgHandle, 
					SipError* err) {
  SipBool ret;
  char*   temp = NULL;


  ret = SipCopyRFC3325PAIHdrToDestination(m,appMsgHandle,err);
  if(ret == SipFail) {
    return ret;
  }

  if(appMsgHandle->pAssertedID_Tel && strlen(appMsgHandle->pAssertedID_Tel)) {
          
          if(sip_insertHeaderFromStringAtIndex(m,SipHdrTypePAssertId, appMsgHandle->pAssertedID_Tel, 
                                               0,err) == SipFail) {
                  NETERROR(MSIP,("Failed to insert rpid hdr"));
          }
  }

  if(appMsgHandle->priv_value)
  {
	  ret = SipCopyRFC3325PrivacyHdrToDestination( m, appMsgHandle->priv_value );
          
  }
  return ret;
}
SipBool SipCopyRFC3325PAIHdrToDestination (SipMessage* m, SipAppMsgHandle* appMsgHandle, 
					SipError* err) {

        return CopyHdrToPAI(m,appMsgHandle->pAssertedID_Sip,SipHdrTypePAssertId);        
        
}

SipBool CopyHdrToPAI(SipMessage* m, header_url* hdr, int hdrtype)   {
                 
  char          *fn       = "SipCopyRFC3325PAIHdrToDestination";
  SipHeader     *header   = NULL;
  SipAddrSpec   *addrspec = NULL;
  SipUrl        *sipurl   = NULL;
  SipError      errlocal;
  SipParam      *urlParam = NULL;
  int           i;

  if(hdr == NULL){
    NETERROR(MSIP,("%s,Null asserted header",fn));
    return SipFail;
  }
 
  if (sip_initSipUrl(&sipurl, &errlocal) == SipFail) {
    NETERROR(MSIP, ("%s Could not initialise Sip Url\n", fn));
    goto _error;
  }
  if(hdr->host) {
    if ((sip_setHostInUrl(sipurl, strdup(hdr->host), &errlocal))==SipFail) {
      NETERROR(MSIP, ("%s fail to  set hostname in url of from header\n", fn));
      goto _error;
    }
  }
  if (hdr->name) {
    if ((sip_setUserInUrl(sipurl, strdup(hdr->name), &errlocal))==SipFail) {
      NETERROR(MSIP, ("%s Could not set user name in url\n", fn));
      goto _error;
    }
  }
  
  if (hdr->port != 0)  {
    if ((sip_setPortInUrl(sipurl,hdr->port, &errlocal))==SipFail) {
	NETERROR(MSIP, ("%s Could not set port number in url\n", fn));
	goto _error;
    }
  }

  for(i = 0; i < sizeof(hdr->url_parameters)/sizeof(SipUrlParameter); 
      ++i)
  {
	  if (sip_initSipParam(&urlParam, &errlocal) == SipFail)
	  {
		  NETERROR(MSIP, ("Could not init url param\n"));
		  goto _error;
	  }
	  
	  if(hdr->url_parameters[i].name &&
	     strlen(hdr->url_parameters[i].name) > 0)
	  {
		  if (sip_setNameInSipParam(urlParam, 
					    strdup(hdr->url_parameters[i].name), &errlocal) == SipFail)
		  {
			  NETERROR(MSIP, ("Could not set name of url param\n"));
				goto _error;
		  }
		  
		  if(hdr->url_parameters[i].value &&
		     strlen(hdr->url_parameters[i].value) > 0)
		  {
			  if (sip_insertValueAtIndexInSipParam(urlParam, 
							       strdup(hdr->url_parameters[i].value), 0, &errlocal) == 
			      SipFail)
				{
					NETERROR(MSIP, ("Could not set value in url param\n"));
					goto _error;
				}
		  }
		  
		  if (sip_insertUrlParamAtIndexInUrl(sipurl, urlParam, i, &errlocal) == 
				SipFail)
		  {
			  NETERROR(MSIP, ("Could not insert url param in url\n"));
			  goto _error;
		  }	
	  }
	  
	  sip_freeSipParam(urlParam);
	  urlParam = NULL;
  }


  if( (sip_initSipHeader(&header, hdrtype, &errlocal) == SipFail)) {
    NETERROR(MSIP,("%s():Unable to init PAI hdr type",fn));
    goto _error;
  }

  if ((sip_initSipAddrSpec(&addrspec,SipAddrSipUri,&errlocal))==SipFail)  {
    NETERROR(MSIP,("%s Could not initialise Addr Spec for the Req Line %d\n",fn, errlocal));
    goto _error;
  }

  if ((sip_setUrlInAddrSpec(addrspec,sipurl,&errlocal))==SipFail)     {
    NETERROR(MSIP, ("Could not write URI in Addr Spec %d\n", errlocal));
    goto _error;
  }
  
  if( hdr->display_name != NULL ) {    
    sip_setDisplayNameInPAssertIdHdr(header, strdup(hdr->display_name), 
				     &errlocal );
  }
  
  if(sip_setAddrSpecInPAssertIdHdr(header,addrspec,&errlocal) == SipFail) {

    NETERROR (MSIP, ("%s failed to insert addrspec in  header", fn));
    goto _error;
  }

  if(hdrtype == SipHdrTypeDcsRemotePartyId)
  {
	  for(i = 0; i < sizeof(hdr->priv_params)/sizeof(SipPrivacyParams); 
	      ++i)
	  {
		  if (sip_initSipParam(&urlParam, &errlocal) == SipFail)
		  {
			  NETERROR(MSIP, ("Could not init url param\n"));
			  goto _error;
		  }
	  
		  if(hdr->priv_params[i].name &&
		     strlen(hdr->priv_params[i].name) > 0)
		  {
			  if (sip_setNameInSipParam(urlParam, 
						    strdup(hdr->priv_params[i].name), &errlocal) == SipFail)
			  {
				  NETERROR(MSIP, ("Could not set name of url param\n"));
				  goto _error;
			  }
		  
			  if(hdr->priv_params[i].value &&
			     strlen(hdr->priv_params[i].value) > 0)
			  {
				  if (sip_insertValueAtIndexInSipParam(urlParam, 
								       strdup(hdr->priv_params[i].value), 0, &errlocal) == 
				      SipFail)
				  {
					  NETERROR(MSIP, ("Could not set value in url param\n"));
					  goto _error;
				  }
			  }
		  
			  if (sip_dcs_insertParamAtIndexInDcsRemotePartyIdHdr(header, urlParam, i, &errlocal) == 
			      SipFail)
			  {
				  NETERROR(MSIP, ("Could not insert url param in url\n"));
				  goto _error;
			  }	
		  }
	  
		  sip_freeSipParam(urlParam);
		  urlParam = NULL;
	  }
  }


  if( sip_insertHeaderAtIndex(m,header,0,&errlocal) == SipFail ) {
    NETERROR (MSIP, ("%s failed to insert PAI header", fn));
    goto _error;
  }
  
  sip_freeSipHeader(header);
  SipCheckFree(header);
  sip_freeSipUrl(sipurl);
  sip_freeSipAddrSpec(addrspec) ;
  sip_freeSipParam(urlParam);

  return SipSuccess;

 _error:
  sip_freeSipHeader(header);
  SipCheckFree(header);
  sip_freeSipUrl(sipurl);
  sip_freeSipAddrSpec(addrspec) ;
  sip_freeSipParam(urlParam);

  return SipFail;
}

SipBool SipCopyRFC3325PrivacyHdrToDestination (SipMessage* m, char* priv_value) {

  SipHeader         *header     = NULL;
  SipNameValuePair  *pNameValue = NULL;
  SipError          errlocal;

  if(priv_value == NULL){
    NETERROR(MSIP,("SipCopyRFC3325PrivacyHdrToDestination(): no privacy header to insert"));
    return SipFail;
  }
  
  if( (sip_initSipHeader(&header, SipHdrTypePrivacy, &errlocal) == SipFail)) {
    NETERROR(MSIP,("SipConvertRFC3325ToDraft01Hdrs():Unable to init rpid hdr type"));
    goto _error;
  }

  if( sip_initSipNameValuePair(&pNameValue,&errlocal) == SipFail) {
    NETERROR(MSIP,("SipCopyRFC3325PrivacyHdrToDestination():Unable to init name value pair"));
    goto _error;
  }
  
  if(sip_setNameInNameValuePair(pNameValue,strdup(priv_value),
				&errlocal) == SipFail) {
    NETERROR(MSIP,("SipCopyRFC3325PrivacyHdrToDestination():Unable to insert name in value pair"));
    goto _error;
  }
  
  if( sip_insertNameValuePairAtIndexInPrivacyHdr(header,pNameValue,0,&errlocal) == SipFail) {
    NETERROR(MSIP,("SipCopyRFC3325PrivacyHdrToDestination():\
                    Unable to insert name value pair in privacy header"));
    goto _error;
  }
  
  if(sip_insertHeaderAtIndex(m,header,0,&errlocal) == SipFail ) {

    NETERROR (MSIP, ("SipCopyRFC3325PrivacyHdrToDestination() failed to insert rpid header"));
    goto _error;
  }

  sip_freeSipNameValuePair(pNameValue);
  sip_freeSipHeader(header);
  SipCheckFree(header);

  return SipSuccess;

 _error:
  sip_freeSipNameValuePair(pNameValue);
  sip_freeSipHeader(header);
  SipCheckFree(header);
  return SipFail;
}

SipBool SipCopyDraft01HdrToDestination(SipMessage* m, SipAppMsgHandle*appMsgHandle, 
				       SipError* err) {
	
	if(appMsgHandle->rpid_hdr)
	{
		if( appMsgHandle->callingpn && appMsgHandle->callingpn->name )
		{
			SipCheckFree(appMsgHandle->rpid_hdr->name);
			appMsgHandle->rpid_hdr->name = NULL;
			appMsgHandle->rpid_hdr->name = strdup(appMsgHandle->callingpn->name);
		}
		
		if( appMsgHandle->callingpn && appMsgHandle->callingpn->host )
		{
			SipCheckFree(appMsgHandle->rpid_hdr->host);
			appMsgHandle->rpid_hdr->host = NULL;
			appMsgHandle->rpid_hdr->host = strdup(appMsgHandle->callingpn->host);
		}

		CopyHdrToPAI(m,appMsgHandle->rpid_hdr,SipHdrTypeDcsRemotePartyId);
	}

  if(appMsgHandle->proxy_req_hdr)
  {
	  if( sip_insertHeaderFromStringAtIndex(m,SipHdrTypeProxyRequire,appMsgHandle->proxy_req_hdr,
						0,err) == SipFail )
	  {
		  NETERROR(MSIP,("SipCopyDraft01HdrToDestination():Unable to insert proxy-require hdr "));
	  }
	  
  }

  return SipSuccess;
}

SipBool SipExtractFromRemotePartyHdr(SipMessage* m, header_url** rpid_hdr, SipError* err) {
  
  char           *fn       = "SipExtractFromRemotePartyHdr";
  SipHeader      *header   = NULL;
  SipAddrSpec    *addrspec = NULL;
  SipUrl         *sipurl   = NULL;
  SipError       erlocal;

  *rpid_hdr = (header_url*) malloc(sizeof(header_url));
  if( *rpid_hdr == NULL ) {
    NETERROR(MSIP,("Unable to malloc rpid hdr"));
    return SipFail;
  }
  memset(*rpid_hdr, 0, sizeof(header_url));

  if(sip_initSipHeader(&header,SipHdrTypeAny,&erlocal) == SipFail) {
    NETERROR(MSIP,("%s():error init header type",fn));
    goto _error;
  }

  if(sip_getHeader(m,SipHdrTypeDcsRemotePartyId,header,&erlocal) == SipFail) {

    NETERROR(MSIP,("%s():error getting  header type",fn));
    goto _error;
  }
  if( sip_dcs_getAddrSpecFromDcsRemotePartyIdHdr(header, &addrspec, &erlocal) == SipFail ){
    
    NETERROR(MSIP,("Could not get Addr Spec from remotepartyid Hdr"));
    goto _error;
  }
  
  if (addrspec->dType != SipAddrSipUri || 
      sip_getUrlFromAddrSpec(addrspec, &sipurl, &erlocal)==SipFail) {
    
    NETERROR(MSIP, ("Could not get Sip Url from AddrSpec/From %d\n", erlocal));
    goto _error;
  }

  /* get 'display name' from the url */
  if ((sip_getDispNameFromFromHdr(header, &((*rpid_hdr)->display_name), err))==SipFail) {
	  NETDEBUG(MSIP, NETLOG_DEBUG3,("Could not get display name from FromHdr %d\n", *err));
  }

  /* get ' name' and ' host' from the url */
  if ((sip_getUserFromUrl(sipurl, &((*rpid_hdr)->name), err))==SipFail) {
	  NETDEBUG(MSIP, NETLOG_DEBUG3,("Could not get User from AssetedHdr %d\n", *err));
  }

  if ((sip_getPortFromUrl(sipurl, &((*rpid_hdr)->port), err))==SipFail) {
	  NETDEBUG(MSIP, NETLOG_DEBUG3, ("Could not get Port from FromHdr %d\n", *err));
	  (*rpid_hdr)->port = 0;
  }
 
  if ((sip_getHostFromUrl(sipurl, &((*rpid_hdr)->host), err))==SipFail) {
	  
	  NETERROR(MSIP, ("Could not get Host from FromHdr %d\n", *err));
	  goto _error;
  }    

  SipExtractUrlParamsFromUrl(sipurl, (SipUrlParameter*)&((*rpid_hdr)->url_parameters));   
  SipExtractPrivacyParamsFromUrl(header, (SipUrlParameter*)&((*rpid_hdr)->priv_params));   

  sip_freeSipHeader(header) ;
  SipCheckFree(header);
  sip_freeSipAddrSpec(addrspec);
  sip_freeSipUrl(sipurl);


  return SipSuccess;

 _error:
  SipCheckFree (*rpid_hdr);           
  *rpid_hdr = NULL;
  if(header != NULL) {
    sip_freeSipHeader(header) ;
    SipCheckFree(header);
  }
  sip_freeSipAddrSpec(addrspec);
  sip_freeSipUrl(sipurl);
  return SipFail;
}

static void SipCreateAttrib(SDPAttr *attrib, char type, int mLineNo, char *name, char *value)
{
	attrib->type = type;
	attrib->name = strdup(name);
	// Handle bug in HSS stack where attribs can not start with a ":"
	attrib->value = value ? value[0] == ':' ? strdup(value+1) : strdup(value) : NULL;
	attrib->mLineNo = mLineNo;
}

void SipCreateEncryptAttrib(SDPAttr *attrib, char *value)
{
	SipCreateAttrib(attrib, 'K', 0, "key", value);
}

void SipCreateBandwidthAttrib(SDPAttr *attrib, char *value)
{
	SipCreateAttrib(attrib, 'B', 0, "bandwith", value);
}

void SipCreateMediaAttrib(SDPAttr *attrib, int mLineNo, char *name, char *value)
{
	SipCreateAttrib(attrib, 'a', mLineNo, name, value);
}

void SipCreateMediaEncryptAttrib(SDPAttr *attrib, int mLineNo, char *value)
{
	SipCreateAttrib(attrib, 'k', mLineNo, "key", value);
}

void SipCreateMediaBandwidthAttrib(SDPAttr *attrib, int mLineNo, char *value)
{
	SipCreateAttrib(attrib, 'b', mLineNo, "bandwidth", value);
}

void SipDupAttrib(SDPAttr *attrib1, const SDPAttr *attrib2)
{
	attrib1->type = attrib2->type;
	attrib1->name = attrib2->name ? strdup(attrib2->name) : NULL;
	attrib1->value = attrib2->value ? strdup(attrib2->value) : NULL;
	attrib1->mLineNo = attrib2->mLineNo;
}

void SipCDupAttrib(cache_t cache, SDPAttr *attrib1, const SDPAttr *attrib2)
{
	attrib1->type = attrib2->type;
	attrib1->name = attrib2->name ? CStrdup(cache, attrib2->name) : NULL;
	attrib1->value = attrib2->value ? CStrdup(cache, attrib2->value) : NULL;
	attrib1->mLineNo = attrib2->mLineNo;
}


int SipReplaceToOrFrom(SipMessage *s, unsigned long mirrorproxy, int header_type)
{
	char*         fn       = "SipReplaceToOrFromwithRsa()";
        header_url*   hdr      = NULL;
	header_url*   tmpptr   = NULL;
        SipError      erlocal;
        char*         str      = NULL;
        
        struct  in_addr in;
        
        in.s_addr = htonl(mirrorproxy);                      
        str       = inet_ntoa(in);                                
                                        
        switch(header_type) 
        {
                
            case SipHdrTypeFrom:
                {                       
                        if( SipExtractFromUri(s,&hdr,&erlocal) == SipFail ) 
                        {
                                NETERROR(MSIP, ("%s Could not retrieve From Header",fn));                
                                goto _error;
                        }
			if( !hdr ) 
			{
				goto _error;
			}
			tmpptr = UrlDup(hdr,MEM_LOCAL);
			
                        if(tmpptr && tmpptr->host)
                        {       
                                if(str && strlen(str)) 
                                {
                                        free(tmpptr->host);                                        
                                        tmpptr->host = strdup(str);                                        
                                }                                
                        }
			
			/*
			  Note: Since tmpptr is being duped and used again to set header in the 
			   message, it would appear to cause memory problems. But a careful examination
			   would reveal that stack frees before inserting the new value
			*/


                        if( SipSetToFromHdr(s,tmpptr,1) < 0 )
                        {
                                NETERROR(MSIP,("%s Failed to Set From Header",fn));
                                goto _error;
                        }                                                
                }                
                break;
                        
            case SipHdrTypeTo:
                {
                        if (SipExtractToUri(s,&hdr,&erlocal) == SipFail )
                        {
                                NETERROR(MSIP,("%s Could not retrieve To Header",fn));
                                goto _error;                                
                        }
			
                        if( !hdr ) 
			{
				goto _error;
			}
			tmpptr = UrlDup(hdr,MEM_LOCAL);
			
                        if(tmpptr && tmpptr->host)
                        {       
                                if(str && strlen(str)) 
                                {
                                        free(tmpptr->host);                                        
                                        tmpptr->host = strdup(str);                                        
                                }                                
                        }
                        if( SipSetToFromHdr(s,tmpptr,0) < 0 )
                        {
                                NETERROR(MSIP,("%s Failed to Set To Header",fn));
                                goto _error;
                        }                                                                        
                }
                break;
        }
                        
 _return:
        UrlFree(tmpptr,MEM_LOCAL);
	SipCheckFree(hdr);
        return SipSuccess;
        
 _error:
        UrlFree(tmpptr,MEM_LOCAL);
	SipCheckFree(hdr);
        return SipFail;        
}

int SipReplaceReqUri(SipMessage *s, unsigned long mirrorproxy, int port, char* type)
{
        
        char*         fn       = "SipReplaceToOrFromwithRsa()";
        header_url*   hdr      = NULL;
	header_url*   tmpptr   = NULL;
        SipError      erlocal;
        char*         str      = NULL;
        
        struct  in_addr in;
        
	if( type == NULL  || strlen(type) <= 0 )
	{
		goto _error;
	}

        in.s_addr = htonl(mirrorproxy);                      
        str       = inet_ntoa(in);                                
        
        if( SipExtractReqUri(s,&hdr,&erlocal) == SipFail ) 
        {
                NETERROR(MSIP, ("%s Could not retrieve ReqUri",fn));                
                goto _error;
        }

	if(!hdr)
	{
		goto _error;
	}


	tmpptr = UrlDup(hdr,MEM_LOCAL);

        if(tmpptr && tmpptr->host)
        {       
                if(str && strlen(str)) 
                {
                        free(tmpptr->host);                                        
                        tmpptr->host = strdup(str);                                        
                }                                
        }
	
	if(port != 0 )
	{
		tmpptr->port = port;
	}

        if( SipSetReqUri(s,tmpptr,type,&erlocal) < 0 )
        {
                NETERROR(MSIP,("%s Failed to Req URI",fn));
                goto _error;
        }                            
           
 _return:
	UrlFree(tmpptr,MEM_LOCAL);
	SipCheckFree(hdr);
        return SipSuccess;
        
 _error:
	UrlFree(tmpptr,MEM_LOCAL);
	SipCheckFree(hdr);
        return SipFail;        
}
                

void SipFormFromHdr(SipAppMsgHandle* appMsgHandle,header_url* from)
{
        if(from->display_name != NULL) {
                if( strlen(from->display_name) && strcasecmp(from->display_name,"anonymous")!=0){
                        free(from->display_name);
                }
                from->display_name  = strdup("Anonymous");
        }

        if(from->name != NULL && strlen(from->name) ) {
                free(from->name);
        }
        from->name = strdup("anonymous");

        if(from->host && strlen(from->host) ) {
                free(from->host);
        }
        
        if(appMsgHandle->dest_priv_type == privacyTypeRFC3325)
        {
                from->host = strdup("anonymous.invalid");
        } 
        else if (appMsgHandle->dest_priv_type == privacyTypeDraft01)
        {
                from->host = strdup("localhost");
        }                
}


void SipGeneratePrivacyHdr(SipMessage* m,SipAppMsgHandle* appMsgHandle) 
{
        
        SipError err;
        
        if(appMsgHandle->dest_priv_type == privacyTypeRFC3325)
        {
                CopyHdrToPAI(m,appMsgHandle->callingpn,SipHdrTypePAssertId);                
                if(appMsgHandle->privLevel == privacyLevelId)
                {
                        SipCopyRFC3325PrivacyHdrToDestination( m, "id" );                
                }
                else if(appMsgHandle->privLevel == privacyLevelNone)
                {
                        SipCopyRFC3325PrivacyHdrToDestination( m, "none" );                
                }
        } 
        else if (appMsgHandle->dest_priv_type == privacyTypeDraft01)
        {
                CopyHdrToRPID(m,appMsgHandle->callingpn, appMsgHandle->privLevel, &err);                   
        }
        
}

SipBool 
SipIsContactWildCard(
	 SipMessage *m, 
	 SipError *err
)
{
	char fn[] = "SipIsContactWildCard";
	SipHeader *header = NULL;
	int count=0;
	SipBool flag = SipFail;

	if( sip_getHeaderCount(m, SipHdrTypeContactAny, &count, err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to geet contact hdr count\n",fn));
		return SipFail;
	}
	if(count == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s found no contact hdr\n",fn));
		return SipSuccess;
	}

	/* Extract the contact header first */
	if( sip_initSipHeader(&header, SipHdrTypeAny, err)==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip contact header\n",fn));
		goto _error;
	}

	if( sip_getHeaderAtIndex(m, SipHdrTypeContactAny, header, (SIP_U32bit)0, err)
	    == SipFail)
	{
		NETERROR(MSIP, ("%s could not get any contact header\n", fn));
		goto _error;
	}

	if(header->dType == SipHdrTypeContactWildCard)
	{
		flag = SipSuccess;
	}

	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	return flag;

 _error:
	sip_freeSipHeader(header) ;
	SipCheckFree(header);

	return SipFail;
}

void SipExtractPrivacyLevelFromRPID(SipMessage* m, char** hdr, SipError* err) 
{
	char           *fn       = "SipExtractPrivacyLevelFromRPID";
	SipHeader      *header   = NULL;
	SipError       erlocal;
	SIP_U32bit     count;
	int            i;
	SipParam*      param     = NULL;
	char*          name;
	char*          value;

	if(sip_initSipHeader(&header,SipHdrTypeAny,&erlocal) == SipFail) {
		NETERROR(MSIP,("%s():error init header type",fn));
		goto _error;
	}

	if(sip_getHeader(m,SipHdrTypeDcsRemotePartyId,header,&erlocal) == SipFail) {

		NETERROR(MSIP,("%s():error getting  header type",fn));
		goto _error;
	}

	if(sip_dcs_getParamCountFromDcsRemotePartyIdHdr(header,&count,&erlocal) == SipFail) 
	{
		NETERROR(MSIP,("%s(): error getting param count from rpid hdr ",fn));
		goto _error;
	}

	for(i=0;i<count;i++) 
	{

		if(sip_dcs_getParamAtIndexFromDcsRemotePartyIdHdr(header,&param,i,&erlocal) == SipFail)
		{
			NETERROR(MSIP,("%s(): Error getting parameter from rpid hdr ", fn));
			goto _error;
		}

		if (sip_getNameFromSipParam(param, &name, &erlocal) != SipFail)
		{
			if( name && strcmp(name,"privacy") == 0 )
			{
				/* extract value */
				if (sip_getValueAtIndexFromSipParam(param, &value, 0, &erlocal) != SipFail)
				{
					*hdr = strdup(value);
				}
			}
			
		}
		sip_freeSipParam(param);
		param = NULL;
	}	
	
	sip_freeSipHeader(header);
	SipCheckFree(header);

	return;

 _error:
	sip_freeSipHeader(header) ;
	SipCheckFree(header);
	sip_freeSipParam(param);
	return ;
}

int
SipExtractPrivacyParamsFromUrl(SipHeader *siphdr, SipPrivacyParams *priv_params)
{
	char fn[]="SipExtractUrlParamsFromUrl";
	int count, i, j;
	SipParam *privParam = NULL;
	SipError err;
	char *name, *value;

	/* Go over the url parameters one by one */
	if (sip_dcs_getParamCountFromDcsRemotePartyIdHdr(siphdr, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get urlparam count\n", fn));
		return 0;
	}

	if (count == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s No privacy parameters found\n", fn));
		return 0;
	}

	for (i = 0, j = 0; (i < count)&&(j < SIP_MAXURLPARMS); i++)
	{
		if (sip_dcs_getParamAtIndexFromDcsRemotePartyIdHdr(siphdr, &privParam, i, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get urlparam at %d\n", fn, i));
			continue;
		}

		if (sip_getNameFromSipParam(privParam, &name, &err) != SipFail)
		{
			(priv_params[j]).name = (name);


			if (sip_getValueAtIndexFromSipParam(privParam, &value, 0, &err) != SipFail)
			{
				(priv_params[j]).value = (value);
			}
			else
			{
				(priv_params[j]).value = NULL;
			}
		}
		else
		{
			NETERROR(MSIP, ("%s fail to get name from urlparam\n", fn));
		}

		++j;

		sip_freeSipParam(privParam);
		privParam = NULL;
        }
	
	sip_freeSipParam(privParam);
	return 0;
}

/*
 * GetSipUnknownHeaders - get the unknown (msw does not process) headers from the message
 * and store them in the msgHandle
 * NOTE: implements "Diversion" header only for now
 */
SipError
GetSipUnknownHeaders(SipMessage *s, SipAppMsgHandle *msghandle)
{
  	char *fn = "GetSipUnknownHeaders";
	SipHeader 	*header;
	SIP_U32bit 	count, index;
	SipError 	err;
	SIP_S8bit 	*name, *names[] = {"Diversion"};
	int i, numhdr = sizeof(names)/sizeof(name);

	msghandle->nunkhdr = 0;
	if ((msghandle->unkhdrs = calloc(numhdr, sizeof(nvpair))) == NULL) {
	  	NETERROR(MSIP, ("%s: malloc failed.\n", fn));
		return SipFail;
	}

	if ((sip_getHeaderCount(s, SipHdrTypeUnknown, &count, &err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get count of unknown header %d. May not be present\n", err));
		return SipSuccess;
	}

	for(index = 0; index < count; ++index)
	{
		if (( sip_initSipHeader (&header, SipHdrTypeAny, &err)) == SipFail)
		{
			NETERROR(MSIP, ("Error initializing SipHeader for custom header %d\n",err));
			return SipFail;
		}

		if ((sip_getHeaderAtIndex(s, SipHdrTypeUnknown, header, index, &err)) == SipFail)
		{
			NETERROR(MSIP, ("Error getting Header of type unknown %d\n",err));
			sip_freeSipHeader(header) ;  
			SipCheckFree(header);
			return SipFail;
		}

		for (i = 0; i < numhdr; i++) { 
		  	name = names[i];
			if(strcmp(((SipUnknownHeader*)(header->pHeader))->pName, name) == 0)
			{
				msghandle->unkhdrs[msghandle->nunkhdr].name = strdup(name);
				msghandle->unkhdrs[msghandle->nunkhdr].val = strdup(((SipUnknownHeader*)(header->pHeader))->pBody);
			  	msghandle->nunkhdr++;

				// check to see if more space needs to be allocated	
				if (msghandle->nunkhdr == numhdr) {
				  	numhdr += numhdr;
					if ((msghandle->unkhdrs = realloc(msghandle->unkhdrs, numhdr*sizeof(nvpair))) == NULL) {
	  					NETERROR(MSIP, ("%s: realloc failed.\n", fn));
						return SipSuccess;
					}
				}

				break;
			}
		}

		sip_freeSipHeader(header) ;  
		SipCheckFree(header);
	}

	return SipSuccess;
}

/*
 * SetSipUnknownHeaders - set the unknown headers (which we don't process) to the message
 * from the msgHandle
 */
SipError
SetSipUnknownHeaders(SipMessage *s, SipAppMsgHandle *msghandle)
{
  	char *fn = "SetSipUnknownHeaders";
	SipHeader *header;
	int i;
	nvpair *hdr;
	SipError err;

	for (i = 0; i < msghandle->nunkhdr; i++) {
	  	hdr = msghandle->unkhdrs + i;

		if (SetSipCustomHeader(s, strdup(hdr->name), strdup(hdr->val), &err) == SipFail) {
		  	return SipFail;
		}
	}

	return SipSuccess;
}

/****************************************/
// Added for Gens only.
// Inserts an Expires Header in the Registration message.

int
SipSetExpiresHdr
	(
        SipMessage *m,
        int regValue       /* expires value*/ 
)
{
        SipError err;
	SipHeader *expireshdr=NULL;
	char fn[] = "SipSetExpiresHdr():";

	if (sip_initSipHeader(&expireshdr, SipHdrTypeExpiresSec, &err)==SipFail)
        {
		NETERROR (MSIP, ("%s Could not initialize expiries header\n", fn));
		goto _error;
        }
        if (sip_setSecondsInExpiresHdr(expireshdr,regValue,&err) == SipFail)
        {
		NETERROR (MSIP, ("%s Could not set seconds in Expires Hdr\n", fn));
		goto _error;
        }

	if  (sip_insertHeaderAtPosition(m,expireshdr, 0,0,0, &err) == SipFail)
        {
		NETERROR (MSIP, ("%s Could not insert expires header.\n", fn));
		goto _error;
        }
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("successfully set expires header to %d",regValue));
        sip_freeSipHeader(expireshdr);
        SipCheckFree(expireshdr);
        return 1;
 _error:
        sip_freeSipHeader (expireshdr);
        SipCheckFree(expireshdr);
        return -1;

}
/*****************************************/
