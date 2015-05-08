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
#include <ctype.h>
#include "nxosd.h"
#include "net.h"
#include "ssip.h"
#include "bcpt.h"

SipBool
SipInsertRealmVia (SipMessage *m, int protocol, SIP_S8bit *branchtoken, char *rsadomain, SipError *err)
{
	SipHeader *header = NULL;
	SIP_S8bit sent_by[256];
	char port[10];
	char * tmpptr = NULL;
	SipParam *viaparam = NULL;

	if ((sip_initSipHeader(&header, SipHdrTypeVia ,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise 'via' header %d\n", *err));
		goto _error;
	}

	nx_strlcpy(sent_by, rsadomain, 256);
	nx_strlcat(sent_by, ":", 256);
	sprintf(port, "%d", lSipPort);
	nx_strlcat(sent_by, port, 256);

	tmpptr = strdup(sent_by);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for sip via sentby failed!\n"));
		goto _error;

	}
	if ((sip_setSentByInViaHdr(header,tmpptr,err))==SipFail)							
	{
		NETERROR(MSIP, ("Could not set 'Sent By' in Via header %d\n", *err));
		goto _error;
	}

	if (protocol == SIPPROTO_TCP)
	{
		tmpptr = strdup("SIP/2.0/TCP");
		if (tmpptr == NULL)
		{
			NETERROR(MSIP, ("strdup for sip via sent protocol failed!\n"));
			goto _error;
		}
	}
	else
	{
		tmpptr = strdup("SIP/2.0/UDP");
		if (tmpptr == NULL)
		{
			NETERROR(MSIP, ("strdup for sip via sent protocol failed!\n"));
			goto _error;
		}
	}

	if ((sip_setSentProtocolInViaHdr(header, tmpptr,err))==SipFail) 					
	{
		NETERROR(MSIP, ("Could not set 'Sent Protocol' in Via header %d\n", *err));
		goto _error;
	}

	/**************** set via params ******************/
	if ((sip_initSipParam(&viaparam, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not init SipParam %d\n", *err));
		goto _error;
	}

	if(branchtoken==NULL)
		goto _skipViaParam;

	if ((sip_setNameInSipParam(viaparam,strdup("branch"),err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set name in SipParam %d\n", *err));
		goto _error;
	}
	if((sip_insertValueAtIndexInSipParam(viaparam,strdup(branchtoken),
					     0,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set value in SipParam %d\n", *err));
		goto _error;
	}
	if((sip_insertViaParamAtIndexInViaHdr(header,viaparam,0,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not insert SipParam in Via hdr %d\n", *err));
		goto _error;
	}

 _skipViaParam:
	if ((sip_insertHeaderAtIndex(m, header, 0,  err))==SipFail)                            
	{
		NETERROR(MSIP, ("Could not set Via header in the message %d\n", *err));
		goto _error;
	}

	/* DOnt free tmpptr here */

	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipParam(viaparam);
	return SipSuccess;

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipParam(viaparam);
	return SipFail;
}

SipBool 
SipInsertVia(SipMessage *m, int protocol, SIP_S8bit *host, SIP_S8bit *branchtoken, SipError *err)
{
	SipHeader *header = NULL;
	SIP_S8bit sent_by[256];
	char port[10];
	char * tmpptr = NULL;
	SipParam *viaparam = NULL;

	if ((sip_initSipHeader(&header, SipHdrTypeVia ,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not initialise 'via' header %d\n", *err));
		goto _error;
	}

	strcpy(sent_by, host);
	strcat(sent_by, ":");
	sprintf(port, "%d", lSipPort);
	strcat(sent_by, port);

	tmpptr = strdup(sent_by);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for sip via sentby failed!\n"));
		goto _error;

	}
	if ((sip_setSentByInViaHdr(header,tmpptr,err))==SipFail)							
	{
		NETERROR(MSIP, ("Could not set 'Sent By' in Via header %d\n", *err));
		goto _error;
	}

	if (protocol == SIPPROTO_TCP)
	{
		tmpptr = strdup("SIP/2.0/TCP");
		if (tmpptr == NULL)
		{
			NETERROR(MSIP, ("strdup for sip via sent protocol failed!\n"));
			goto _error;
		}
	}
	else
	{
		tmpptr = strdup("SIP/2.0/UDP");
		if (tmpptr == NULL)
		{
			NETERROR(MSIP, ("strdup for sip via sent protocol failed!\n"));
			goto _error;
		}
	}

	if ((sip_setSentProtocolInViaHdr(header, tmpptr,err))==SipFail) 					
	{
		NETERROR(MSIP, ("Could not set 'Sent Protocol' in Via header %d\n", *err));
		goto _error;
	}

	/**************** set via params ******************/
	if ((sip_initSipParam(&viaparam, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not init SipParam %d\n", *err));
		goto _error;
	}

	if(branchtoken==NULL)
		goto _skipViaParam;

	if ((sip_setNameInSipParam(viaparam,strdup("branch"),err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set name in SipParam %d\n", *err));
		goto _error;
	}
	if((sip_insertValueAtIndexInSipParam(viaparam,strdup(branchtoken),
					     0,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set value in SipParam %d\n", *err));
		goto _error;
	}
	if((sip_insertViaParamAtIndexInViaHdr(header,viaparam,0,err))==SipFail)
	{
		NETERROR(MSIP, ("Could not insert SipParam in Via hdr %d\n", *err));
		goto _error;
	}

 _skipViaParam:
	if ((sip_insertHeaderAtIndex(m, header, 0,  err))==SipFail)                            
	{
		NETERROR(MSIP, ("Could not set Via header in the message %d\n", *err));
		goto _error;
	}

	/* DOnt free tmpptr here */

	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipParam(viaparam);
	return SipSuccess;

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipParam(viaparam);
	return SipFail;
}

/* This function will do a DNS lookup... */
int
SipCheckSourceAddressWithVia(
	SipMessage *s, 
	SipEventContext *context,
	int viaPosition,
	int *rxedFromNat,
	SipError *err
)
{
	char fn[] = "SipCheckSourceAddressWithVia():";
	char *host = NULL;
	unsigned short port;
	int herror;
	long ipaddr,ipaddrtrue;
	SipParam *pSipParam = NULL;
	char ipaddrstr[24];
 	SipHeader *header=NULL ;
	char tmpmsg[1024];
	int tmplen;

	if (SipGetSentByHost(s, context, viaPosition, 0, &host, &port, err) <= 0)
	{
		goto _error;
	}

	if((ipaddr = ResolveDNS(host, &herror)) == -1)
	{
		if(herror == TRY_AGAIN || herror == NO_RECOVERY)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s DNS failed so assuming there is no need to insert received parameter\n", fn));
			goto _return;
		}
		else
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Bad ip address for %s\n", fn, host));
			*err = -1;
			goto _error;
		}
	}
	
	ipaddrtrue=ntohl(inet_addr(context->pTranspAddr->dIpv4));
	if (ipaddrtrue == ipaddr)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s No need to insert received parameter\n", fn));
		goto _return;
	}
		
	FormatIpAddress(ipaddrtrue, ipaddrstr);

	/* There is a mismatch.
	 * We must insert a received attribute in Via header
	 */

	if(rxedFromNat) *rxedFromNat = 1;
	
	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Detected mismatch in Via with source ip\n", fn));

	if (sip_initSipParam(&pSipParam, err) == SipFail)
	{
		NETERROR(MSIP, ("Could not init sip param\n"));
		goto _error;
	}

	if (sip_setNameInSipParam(pSipParam, strdup("received"), err) == SipFail)
	{
		NETERROR(MSIP, ("Could not set name of sip param\n"));
		goto _error;
	}

	if (sip_insertValueAtIndexInSipParam(pSipParam, strdup(ipaddrstr), 
		0, err) == SipFail)
	{
		NETERROR(MSIP, ("Could not set maddr in url param\n"));
		goto _error;
	}

	if ((sip_initSipHeader(&header, SipHdrTypeAny ,err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise 'via' header %d\n", 
			fn, *err));
		return -1;
	}

	if ((sip_getHeaderAtIndex(s, SipHdrTypeVia, header, (SIP_U32bit)0, 
		err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Unable to get via header from index %d\n", 
			fn, *err));
		goto _error;
	}

	if (sip_insertViaParamAtIndexInViaHdr(header , pSipParam, 0, err) == SipFail)
	{
		NETERROR(MSIP, ("%s failed to insert via param in Via hdr\n",fn));
		goto _error;
	}

_return:
	if ( header != NULL )
	{
		sip_freeSipHeader(header);
		SipCheckFree(header);
	}

	sip_freeSipParam(pSipParam);
	SipCheckFree(host);

	return 0;

_error:
	if ( header != NULL )
	{
		sip_freeSipHeader(header);
		SipCheckFree(header);
	}

	sip_freeSipParam(pSipParam);
	SipCheckFree(host);

	return -1;
}

