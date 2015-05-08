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
#include "ssip.h"
#include "siputils.h"
#include "bcpt.h"

/* return 0 if no loop was found */
int
SipCheckLoopUsingBranch(SipMessage *s,SIP_S8bit *branchtoken, SipError *err)
{
	char fn[]= "SipCheckLoopUsingBranch";
 	SIP_S8bit *viastring;
	SIP_U32bit count;
	int i;

	/* See if there is a Via Header in the packet */
	if ((sip_getHeaderCount(s, SipHdrTypeVia, &count, err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Unable to get via header list %d\n", 
			fn, *err));
		goto _error;
	}

	DEBUG(MSIP, NETLOG_DEBUG4, ("%s: no. of via headers == %d\n", fn,count));

	if (count == 0)
	{
		/* Weird... */
		NETERROR(MSIP, ("%s Via Header count is 0 !!\n", fn));
		goto _error;
	}

	for(i=1;i<count;i++)
	{
		if ((sip_getHeaderAsStringAtIndex(s, SipHdrTypeVia, &viastring, (SIP_U32bit)i, err))== SipFail)
		{
			NETERROR(MSIP, ("%s Unable to get via header from index %d\n", 
					fn, *err));
			goto _error;
		}

		DEBUG(MSIP, NETLOG_DEBUG4, ("%d-th via: %s\n",i,viastring));

		/* if we are in via  AND the same via has a branch that is the
		   same as the current branch, then
		   declare loop detected */
		if(strstr(viastring,branchtoken)!=NULL)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s detected a loop\n", fn));
			goto _error;
		}
		SipCheckFree(viastring);
	}
	
	return 0;
	
_error:
	SipCheckFree(viastring);
	return -1;
}

/* return 1 if our domain appears in topmost via
 * return 0 if not
 */
int
SipCheckTopVia(
	SipMessage *s, 
	SipError *err,
	SipEventContext *context
)
{
	char fn[] = "SipCheckTopVia():";
	char *host = NULL;
	unsigned short port;
	char s1[24];
	in_addr_t hostip;
	CallRealmInfo *ri;
	int rc = 1;
	int herror;
	unsigned long ipaddr;

	if (context == NULL) return 0;

	if (SipGetSentByHost(s, context, 0, 0, &host, &port, err) <= 0)
	{
		NETERROR(MSIP, ("%s SipGetSentByHost failed\n", fn));
		goto _error;
	}

	hostip = inet_addr(host);
	ri = (CallRealmInfo *) context->pData;
#if 0 // FIX THIS VIA STUFF ...
	if (hostip != htonl (ri->rsa) )
	{
		goto _error;
	}
#endif
#if 0
	if((ipaddr = ResolveDNS(sipdomain, &herror)) != -1)
	{
		FormatIpAddress(ipaddr, s1);
		if (strcmp(host, s1))

		{
			FormatIpAddress(ntohl(FCEGetSigPrivateAddress()), s1);
			if(strcmp(host, s1))
			{
				NETERROR(MSIP, ("%s strcmp failed for %s, %s\n", fn, host, s1));
				goto _error;
			}
		}
	}
	else
	{
		NETERROR(MSIP, ("%s ResolveDNS failed for sipdomain %s\n", 
					 fn, sipdomain));
		goto _error;
	}

#endif
	SipCheckFree(host);

	return 1;

_error:
	SipCheckFree(host);

	return 0;
}

/* return -1 on error, >0 on success, memory allocated in host by this function,
 * it returns == 0 if there was no via header found
 */ 
int
SipGetSentByHost(
	SipMessage *s, 
	SipEventContext *context,
	int position,	/* Via Header position */
	int check_receive_parameter,
	char **host,
	unsigned short *port,
	SipError *err
)
{
	char fn[] = "SipGetSentByHost():";
 	SipHeader *header=NULL ;
	SIP_U32bit count, i;
	SIP_S8bit *sent_by=NULL;
	char *tmp=NULL, *portStr=NULL;
	char *name=NULL;
	SipParam *sipParam = NULL;

#if 1
	/* See if there is a Via Header in the packet */
	count=0;
	if ((sip_getHeaderCount(s, SipHdrTypeVia, &count, err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Unable to get via header count %d\n", 
			fn, *err));
		return -1;
	}
	if(count==0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no via header found\n",fn));
		goto _no_via;
	}
#endif
	/* See if there is a Via Header in the packet */
	if ((sip_initSipHeader(&header, SipHdrTypeAny, err))==SipFail)
	{
		NETERROR(MSIP, ("%s Could not initialise 'via' header %d\n", 
			fn, *err));
		return -1;
	}

	if ((sip_getHeaderAtIndex(s, SipHdrTypeVia, header, (SIP_U32bit)position, 
		err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Unable to get via header from index %d \n", 
				fn, position));
		goto _error;
	}

	/* Check the sent by parameter and the branch */
	if ((sip_getSentByFromViaHdr(header, &sent_by, err))==SipFail)
	{
		NETERROR(MSIP, 
			("%s Could not get 'Sent By' in Via header %d\n", fn, *err));
		goto _error;
	}

	/* sent-by is a string, of the form host[:port] always */
	if (sent_by)
	{
		*host = sent_by;
	}

	if (!(*host))
	{
		NETERROR(MSIP, ("%s host is NULL\n", fn));
		goto _error;
	}
	else
	{
		/* duplicate the memory first, and we will return it.
		 */
		*host = strdup(*host);
		*host = strtok_r(*host, ":", &portStr);

		if (portStr)
		{
			portStr = strtok_r(portStr, ":", &tmp);
		}

		if (portStr)
		{
			*port = atoi(portStr);
		}
		else
		{
			*port = 5060;
		}
	}
	
	/* Look for other Via parameters - maddr and received */
	if (sip_getViaParamCountFromViaHdr(header, &count, err) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get via param count\n", fn));
		goto _error;
	}

	if (count == 0 || !check_receive_parameter)
	{
		goto _return;
	}

	tmp = NULL;

	for (i = 0; i < count; i++)
	{
		if (sip_getViaParamAtIndexFromViaHdr(header, 
			&sipParam, i, err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get %d-th via param\n", fn, i));
			goto _error;
		}

		if (sip_getNameFromSipParam(sipParam, &name, err) == SipFail)
		{
			NETERROR(MSIP, ("%s fail to get name from %d-th via param\n",
					fn, i));
			goto _error;
		}

		if (!strcasecmp(name, "maddr"))
		{
			/* extract value */
			if (sip_getValueAtIndexFromSipParam(sipParam, &tmp, 0, err) 
				== SipFail)
			{
				NETERROR(MSIP, ("%s fail to get maddr value from %d-th via param\n", fn, i));
				goto _error;
			}

			break;
		}

		if (!strcasecmp(name, "received"))
		{
			/* extract value */
			if (sip_getValueAtIndexFromSipParam(sipParam, &tmp, 0, err) 
				== SipFail)
			{
				NETERROR(MSIP, ("%s fail to get recvd value from %d-th via param\n", fn, i));
				goto _error;
			}

			if(context && context->pTranspAddr)
			{
				*port = context->pTranspAddr->dPort;
			}

			/* keep looking to see if we find the maddr param */
		}
		
		sip_freeSipParam(sipParam);
		sipParam = NULL;
	}

	if (tmp)
	{
		/* must free previously allocated memory */
		SipCheckFree(*host);
		/* host will be freed by calling function */
		*host = strdup(tmp);
	}

_return:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipParam(sipParam);

	return 1;

_no_via:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipParam(sipParam);

	return 0;
_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipParam(sipParam);

	return -1;
}

/* return 0 if Max-Forwards not present, and max(-1,(max-forwards)--) if present */
int SipCheckMaxForwards(SipMessage *s)
{
	char fn[]="SipCheckMaxForwards:";
	SIP_U32bit count;
	SipError err;
	SipHeader *header=NULL;

	if ((sip_getHeaderCount(s, SipHdrTypeMaxforwards, &count, &err)) == SipFail)
	{
		NETERROR(MSIP, ("%s Unable to get Max-Forward hdr count %d\n", fn, err));
		return -1;
	}
	if (count == 0)
	{
		return 0;
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

	if (sip_getHopsFromMaxForwardsHdr(header,&count,&err)==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Fail to get hops from  Max-F Hdr\n",fn));
		goto _error;
	}
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Found Max-Forwards hdr=%d\n",fn,count));

	if(count>0)
	{
		count--;
		( (SipMaxForwardsHeader *) (header->pHeader) )->dHops = count;
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return count;
	}
	else
	{
		goto _error;
	}

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	return -1;
}
