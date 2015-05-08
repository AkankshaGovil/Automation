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

#include "ssip.h"
#include "sipaddr.h"
#include <malloc.h>
#include "bcpt.h"


/******** function to set Call Id in the request message taking string inputs ************/

SipBool sip_form_callid_inmsg(SipMessage *msg, SIP_S8bit  *callid, SipError *err)
{
	SipHeader *header;
	char * tmpptr;
	if ((sip_initSipHeader(&header, SipHdrTypeCallId, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise 'Call Id' header %d\n", *err));
		return(SipFail);
	}

	tmpptr = strdup(callid);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for sip callid failed!\n"));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	if ((sip_setValueInCallIdHdr(header, tmpptr ,err))==SipFail)						
	{
		NETERROR(MSIP, ("Could not set value of Call Id in 'Call Id' header %d\n", *err));
		SipCheckFree(tmpptr);
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	if ((sip_setHeader(msg, header, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set 'Call Id' header in the message %d\n", *err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return SipSuccess;
}

SipBool sip_form_cseqnum_inmsg(SipMessage *msg, int seqnum, char * method, SipError *err)
{
	SipHeader *header;
	char * tmpptr;

	if ((sip_initSipHeader(&header, SipHdrTypeCseq, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise 'CSeq' header %d\n", *err));
		return(SipFail);
	}
	if ((sip_setSeqNumInCseqHdr(header, seqnum, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set Cseq number in 'CSeq' header %d\n", *err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	tmpptr = strdup(method);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for sip seqnum method failed!\n"));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	if ((sip_setMethodInCseqHdr(header, tmpptr, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set method in 'CSeq' header %d\n", *err));
		SipCheckFree(tmpptr);
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	if ((sip_setHeader(msg, header, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set 'CSeq' header in the INVITE message %d\n", *err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return SipSuccess;
}

SipBool sip_form_contacthdr_inmsg(SipMessage *msg, header_url *contact, SipError *err)
{
	SipHeader *header = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	char * tmpptr = NULL;
	int port;
	SipParam *urlParam = NULL;
	int i;

	port = contact->port;
	port = (port==0)?lSipPort:port;
	if ((sip_initSipHeader(&header,SipHdrTypeContactNormal, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise 'Contact' header %d\n", *err));
		return(SipFail);
	}
	if ((sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err))==SipFail)                  
	{
		NETERROR(MSIP, ("Could not initialise Addr Spec in 'Contact' header %d\n", *err));
		goto _error;
	}
	if ((sip_initSipUrl(&sipurl, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise Sip Url %d\n", *err));
		goto _error;
	}
	tmpptr = strdup(contact->host);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for sip contact->host failed!\n"));
		goto _error;
	}
	if ((sip_setHostInUrl(sipurl, tmpptr , err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set hostname in url of 'Contact' header %d\n", *err));
		goto _error;
	}
	if (contact->name)
	{
		tmpptr = strdup(contact->name);
		if (tmpptr == NULL)
		{
			NETERROR(MSIP, ("strdup for sip contact->name failed!\n"));
			goto _error;
		}
		if ((sip_setUserInUrl(sipurl, tmpptr, err))==SipFail)   				 
		{
			NETERROR(MSIP, ("Could not set user name in url of 'Contact' header %d\n", *err));
			goto _error;
		}
	}
	if ((sip_setPortInUrl(sipurl, (SIP_U16bit) port, err))==SipFail)   				 
	{
		NETERROR(MSIP, ("Could not set port number in url of 'Contact' header %d\n", *err));
		goto _error;
	}

	for(i = 0; 
		i < sizeof(contact->url_parameters)/sizeof(SipUrlParameter); 
		++i)
	{
		if (sip_initSipParam(&urlParam, err) == SipFail)
		{
			NETERROR(MSIP, ("Could not init url param\n"));
			goto _error;
		}

		if(contact->url_parameters[i].name && strlen(contact->url_parameters[i].name) > 0)
		{
			if (sip_setNameInSipParam(urlParam, 
				strdup(contact->url_parameters[i].name), err) == SipFail)
			{
				NETERROR(MSIP, ("Could not set name of url param\n"));
				goto _error;
			}

			if(contact->url_parameters[i].value && strlen(contact->url_parameters[i].value) > 0)
			{
				if (sip_insertValueAtIndexInSipParam(urlParam, 
					strdup(contact->url_parameters[i].value), 0, err) == 
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

	if ((sip_setUrlInAddrSpec(addrspec, sipurl, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set url in Addr Spec of 'Contact' header %d\n", *err));
		goto _error;
	}
	if ((sip_setAddrSpecInContactHdr(header, addrspec, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set Addr Spec in 'Contact' header %d\n", *err));
		goto _error;
	}
	if ((sip_insertHeaderAtIndex(msg, header, (SIP_U32bit)0, err))==SipFail)
	{
		goto _error;
	}

_return:
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
