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

SipBool
SipHashViaBranch(SipMessage *m, SIP_S8bit *branchtoken, SipError *err)
{
	char fn[] = "SipHashViaBranch()";
	SipReqLine *reqline = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	SipHeader *header = NULL;
	SIP_S8bit *requri = NULL, *touri = NULL, *fromuri = NULL, *callid = NULL, *seq = NULL;
	SIP_U32bit seqnum;
	SIP_S8bit *hashinput = NULL, *tmpptr;
	header_url *req_uri = NULL, *from = NULL, *to = NULL;
	int len;
	SipBool rc = SipFail;

	if ((SipExtractReqUri(m, &req_uri, err)) == SipFail)
	{
		NETERROR(MSIP, 
			("%s Could not extract req uri\n", fn));
		goto _error;
	}

	len = strlen(SVal(req_uri->name)) + strlen(SVal(req_uri->host)) + 2;

	if((requri = malloc(len)) == NULL)
	{
		NETERROR(MSIP, 
			("%s Could not malloc requri\n", fn));
		goto _error;
	}

	if (req_uri->name)
	{
		snprintf(requri, len, "%s@%s", SVal(req_uri->name), SVal(req_uri->host));
	}
	else
	{
		snprintf(requri, len, "%s", SVal(req_uri->host));
	}

	NETDEBUG(MSIP, NETLOG_DEBUG3,("%s: Request-URI is: %s\n",
		fn,requri));

	if (SipExtractFromUri(m, &from, err) == SipFail)
	{
		NETERROR(MSIP,
			("%s Could not extract from url\n", fn));
		goto _error;
	}
		
	len = strlen(SVal(from->name)) + strlen(SVal(from->host)) + 2;

	if((fromuri = malloc(len)) == NULL)
	{
		NETERROR(MSIP, 
			("%s Could not malloc fromuri\n", fn));
		goto _error;
	}

	if (from->name)
	{
		snprintf(fromuri, len, "%s@%s", SVal(from->name), SVal(from->host));
	}
	else
	{
		snprintf(fromuri, len, "%s", SVal(from->host));
	}

	NETDEBUG(MSIP, NETLOG_DEBUG3,("%s: From-URI is: %s\n",fn,fromuri));

	if (SipExtractToUri(m, &to, err) == SipFail)
	{
		NETERROR(MSIP,
			("%s Could not extract to url\n", fn));
		goto _error;
	}
		
	len = strlen(SVal(to->name)) + strlen(SVal(to->host)) + 2;

	if((touri = malloc(len)) == NULL)
	{
		NETERROR(MSIP, 
			("%s Could not malloc touri\n", fn));
		goto _error;
	}

	if (to->name)
	{
		snprintf(touri, len, "%s@%s", SVal(to->name), SVal(to->host));
	}
	else
	{
		snprintf(touri, len, "%s", SVal(to->host));
	}

	NETDEBUG(MSIP, NETLOG_DEBUG3,("%s: To-URI is: %s\n",fn,touri));

	/***************************** callid header *************************/
	if (sip_initSipHeader (&header, SipHdrTypeAny , err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error initializing header\n", fn));
		goto _error;
	}

	if (( sip_getHeader(m, SipHdrTypeCallId, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("%s:Error getting To from siprequest %d\n",fn,*err));
		goto _error;
	}

	if (( sip_getValueFromCallIdHdr(header, &tmpptr, err))==SipFail)
	{
		NETERROR(MSIP, ("%s:Error getting value from callid hdr %d\n",fn,*err));
		goto _error;
	}

	len = strlen(SVal(tmpptr)) + 1;

	if((callid = malloc(len)) == NULL)
	{
		NETERROR(MSIP, 
			("%s Could not malloc callid\n", fn));
		goto _error;
	}

	snprintf(callid, len, "%s", SVal(tmpptr));

	NETDEBUG(MSIP, NETLOG_DEBUG3,("%s: CallId is: %s\n",fn,callid));

	sip_freeSipHeader(header);

	/***************************** cseq header *************************/
	if (( sip_getHeader(m, SipHdrTypeCseq, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("%s:Error getting To from siprequest %d\n",fn,*err));
		return SipFail;
	}

	if (( sip_getSeqNumFromCseqHdr(header, &seqnum,err))==SipFail)
	{
		NETERROR(MSIP, ("%s:Error getting seq from cseq hdr %d\n",fn,*err));
		goto _error;
	}

	len = 10 + 1;

	if((seq = malloc(len)) == NULL)
	{
		NETERROR(MSIP, 
			("%s Could not malloc seq\n", fn));
		goto _error;
	}

	snprintf(seq, len, "%d", seqnum);

	NETDEBUG(MSIP, NETLOG_DEBUG3,("%s: seq num is: %s\n",fn,seq));

	/************************ Hash func **********************/
	len = strlen(requri) + strlen(fromuri) + strlen(touri) + strlen(callid) + strlen(seq) + 5;

	if((hashinput = malloc(len)) == NULL)
	{
		NETERROR(MSIP, 
			("%s Could not malloc hashinput\n", fn));
		goto _error;
	}

	snprintf(hashinput, len, "%s %s %s %s %s", requri, fromuri, touri, callid, seq);

	HashString(hashinput, branchtoken);

	NETDEBUG(MSIP, NETLOG_DEBUG3,("%s: branch=%s\n",fn,branchtoken));

	rc = SipSuccess;

_error:
	SipCheckFree(hashinput);
	SipCheckFree(requri);
	SipCheckFree(fromuri);
	SipCheckFree(touri);
	SipCheckFree(callid);
	SipCheckFree(seq);

	SipCheckFree(req_uri);
	SipCheckFree(to);
	SipCheckFree(from);

	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipReqLine(reqline);

	return rc;
}
