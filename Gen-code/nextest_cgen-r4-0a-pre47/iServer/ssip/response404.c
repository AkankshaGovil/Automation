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
SipResponse404(SipMessage *m, SipMessage *resp, SIP_S8bit *hostname, SIP_U16bit *port, SipError *err)
{
	SipStatusLine *status_line;
	SIP_U16bit status_code=404; 
	SIP_S8bit *reason="Not Found";
	SipHeader *header ;
	SIP_U32bit count;
	SipTranspAddr transpadr;
	SIP_U32bit i;
	char * tmpptr;

	/* Status line */
	if ((sip_initSipStatusLine(&status_line, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error initializing initSipstatusline %d\n",*err));
		return SipFail;
	}

	tmpptr = strdup("SIP/2.0");
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for version in status line failed!\n"));
		sip_freeSipStatusLine(status_line);
		return(SipFail);
	}
	if (( sip_setVersionInStatusLine (status_line, tmpptr, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error setting version in statusLine %d\n",*err));
		SipCheckFree(tmpptr);
		sip_freeSipStatusLine(status_line);
		return SipFail;
	}

	if (( sip_setStatusCodeNumInStatusLine (status_line , status_code, err))==SipFail )
	{
		NETERROR(MSIP, ("Error setting status code number in status line %d\n",
				*err));
		sip_freeSipStatusLine(status_line);
		return SipFail;
	}
	tmpptr = strdup(reason);
	if (tmpptr == NULL)
	{
		NETERROR(MSIP, ("strdup for reason in response failed!\n"));
		sip_freeSipStatusLine(status_line);
		return(SipFail);
	}
	if (( sip_setReasonInStatusLine(status_line , tmpptr, err)) == SipFail)
	{
		NETERROR(MSIP, ("Error setting reason in status line %d\n",*err));
		SipCheckFree(tmpptr);
		sip_freeSipStatusLine(status_line);
		return SipFail;
	}

	if (( sip_setStatusLineInSipRespMsg(resp, status_line, err)) == SipFail)
	{
		NETERROR(MSIP, ( "Error setting status line in sipresp message %d\n", *err));
		sip_freeSipStatusLine(status_line);
		return SipFail;
	}

	sip_freeSipStatusLine( status_line) ;

	/* setting up headers */
	if (( sip_initSipHeader (&header, SipHdrTypeAny , err)) == SipFail )
	{
		NETERROR(MSIP, ("Error initializing SipHeader of type Cseq %d\n", *err));
		return SipFail;
	}

	/* Via header */
	if ((sip_getHeaderCount(m, SipHdrTypeVia, &count, err)) == SipFail)
	{
		NETERROR(MSIP, ("response404: Unable to get via header list %d\n", *err));
		SipCheckFree(header);
		return SipFail;
	}
	DEBUG(MSIP, NETLOG_DEBUG4, ("response404: no. of via headers == %d\n", count));

	/* skip first one, which is put in by us */
	for(i=1 ; i<count ; i++)
	{
		if ((sip_getHeaderAtIndex(m, SipHdrTypeVia, header,(SIP_U32bit)i, err))
		    == SipFail)
		{
			NETERROR(MSIP, ("response404: Unable to get via header from index %d\n", *err));
			sip_freeSipHeader(header);
			SipCheckFree(header);
			return SipFail;
		}
		if ((sip_insertHeaderAtIndex(resp, header, (SIP_U32bit)(i-1), err)) 
		    == SipFail)
		{
			NETERROR(MSIP, ("response404: Unable to set via header from index %d\n", *err));
			sip_freeSipHeader(header);
			SipCheckFree(header);
			return SipFail;
		}
		sip_freeSipHeader(header);
	}

	/************************************************************
 				 From  being set in the response message
	*************************************************************/
	if (( sip_getHeader(m, SipHdrTypeFrom, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("Error getting From from siprequest %d\n",*err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return SipFail;
	}

	if ((sip_setHeader(resp, header, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set From header in the response message %d\n", *err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	sip_freeSipHeader(header);

	/*************************************************************
 			 To  being set in the response message
	*************************************************************/
	if (( sip_getHeader(m, SipHdrTypeTo, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("Error getting To from siprequest %d\n",*err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return SipFail;
	}

	if ((sip_setHeader(resp, header, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set From header in the response message %d\n", *err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}
	sip_freeSipHeader(header);

	/************************************************************
      Cseq  being displayed and set in the response message 
	************************************************************/

	if (( sip_getHeader(m, SipHdrTypeCseq, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("Error getting cseq no from sipHeader %d\n",*err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return SipFail;
	}

	if (( sip_setHeader(resp, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("Error setting cseq in sipHeader %d\n",*err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return SipFail;
	}
	sip_freeSipHeader(header) ;

	/*************************************************************
 			 CallID  being set in the response message
	*************************************************************/
	if (( sip_getHeader(m, SipHdrTypeCallId, header, err)) == SipFail )
	{
		NETERROR(MSIP, ("Error getting To from siprequest %d\n",*err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return SipFail;
	}

	if ((sip_setHeader(resp, header, err))==SipFail)
	{
		NETERROR(MSIP, ("Could not set From header in the response message %d\n", *err));
		sip_freeSipHeader(header);
		SipCheckFree(header);
		return(SipFail);
	}

	sip_freeSipHeader(header);

	/* figure out where to send message */
	if((SipExtractRemoteAddrFromVia(resp, &transpadr,err))==SipFail)
	{
		NETERROR(MSIP, ("response404: Could not extract remote addr%d\n", *err));
		SipCheckFree(header);
		return(SipFail);
	}

	strcpy(hostname, transpadr.dIpv4);
	*port=transpadr.dPort;

	NETDEBUG(MSIP, NETLOG_DEBUG3,("response404: Sent to host=%s port=%d\n",
				      hostname,*port));

	/* done with header forever */
	SipCheckFree(header);

	return(SipSuccess);
}

