/*
 * pktsend.c
 *
 *	Copyright 1998, Netoids Inc.
 */

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

#include "ipc.h"
#include "phonode.h"


/*
 * Routine to transmit a packet defined by (Pkt *) pkt
 * to the socket descriptor 'sockfd'.
 */
int
PktSend (int sockfd, Pkt * pkt)
{
	int	retval = 0;

	if (pkt == NULL)
		return -1;

	/* Make sure the len is set correctly */
	if (pkt->totalLen == 0)
	{
		pkt->totalLen = htonl(sizeof(Pkt));
	}
	
	retval = write (sockfd, pkt, sizeof(*pkt)); 

	if (retval != sizeof(*pkt))
	{
		perror ("Error in writing");
		return (retval);
	}

	return (retval);
}

int
htonPkt(int type, Pkt *pktP)
{
	pktP->type = htonl(pktP->type);
	pktP->totalLen = htonl(pktP->totalLen);

	if (type != PKT_XML)
	{
		/* Change the credo */
		htonPhonode(&pktP->credo);
	}

	switch (type)
	{
	case PKT_REGISTER:
	case PKT_PROXYREGISTER:
	case PKT_REGISTER_SUCCESS:
	case PKT_REGISTER_FAILURE:
	case PKT_UNREGISTER:
		htonPhonode(&pktP->data.reginfo.node);
		pktP->data.reginfo.reason = htonl(pktP->data.reginfo.reason);

		break;
	case PKT_FIND:
	case PKT_FOUND:
	case PKT_NOTFOUND:
		htonPhonode(&pktP->data.find.node);
		htonPhonode(&pktP->data.find.fnode);
		htonPhonode(&pktP->data.find.anode);

		break;
	case PKT_WAKEUP:
	case PKT_CONNECT:
	case PKT_ALERTED:
	case PKT_BUSY:
	case PKT_HANGUP:
		htonPhonode(&pktP->data.wakeup.snode);
		htonPhonode(&pktP->data.wakeup.dnode);
		htons(pktP->data.wakeup.snetport);
		htons(pktP->data.wakeup.dnetport);

		break;
	case PKT_DND:
		htonPhonode(&pktP->data.dnd.node);
		pktP->data.dnd.enableCall = htonl(pktP->data.dnd.enableCall);

		break;
	case PKT_REDIRECT:
	case PKT_PROXY:
		htonPhonode(&pktP->data.redirect.onode);
		htonPhonode(&pktP->data.redirect.nnode);
		pktP->data.redirect.protocol = htonl(pktP->data.redirect.protocol);
		pktP->data.redirect.valid = htonl(pktP->data.redirect.valid);

		break;
	case PKT_CONTROL:
		htonPhonode(&pktP->data.control.snode);
		htonPhonode(&pktP->data.control.dnode);
		pktP->data.control.hold = htonl(pktP->data.control.hold);
		pktP->data.control.mute = htonl(pktP->data.control.mute);

		break;
	case PKT_TRANSFER:
	case PKT_TRANSFERRED:
		htonPhonode(&pktP->data.transfer.transferer);
		htonPhonode(&pktP->data.transfer.transferee);
		htonPhonode(&pktP->data.transfer.dest);
		pktP->data.transfer.transfererTransfereeNetport = 
			htons(pktP->data.transfer.transfererTransfereeNetport);
		pktP->data.transfer.transfererDestNetport = 
			htons(pktP->data.transfer.transfererDestNetport);	
		pktP->data.transfer.transfereeNetport = 
			htons(pktP->data.transfer.transfereeNetport);
		pktP->data.transfer.destNetport = 
			htons(pktP->data.transfer.destNetport);
		pktP->data.transfer.role = 
			htons(pktP->data.transfer.role);
		pktP->data.transfer.localState = 
			htons(pktP->data.transfer.localState);	
		break;

	case PKT_SERVERMSG:
		break;
	case PKT_PROFILE:
	case PKT_HEARTBEAT:
		htonPhonode(&pktP->data.profile.local);
		htonPhonode(&pktP->data.profile.remote);
		pktP->data.profile.aloidIpAddress = 
			htonl(pktP->data.profile.aloidIpAddress);
		pktP->data.profile.vpnsIpAddress = 
			htonl(pktP->data.profile.vpnsIpAddress);
		pktP->data.profile.ftime = 
			htonl(pktP->data.profile.ftime);
		pktP->data.profile.ctime = 
			htonl(pktP->data.profile.ctime);
		pktP->data.profile.sltime = 
			htonl(pktP->data.profile.sltime);
		pktP->data.profile.rltime = 
			htonl(pktP->data.profile.rltime);
		pktP->data.profile.lastSeqNo = 
			htons(pktP->data.profile.lastSeqNo);
		break;
	case PKT_ERROR:
		switch (pktP->data.errormsg.node.type)
		{
		case NODE_PHONODE:
			htonPhonode(&pktP->data.errormsg.node.unode.phonode);
			break;
		case NODE_SRVR:
			pktP->data.errormsg.node.unode.srvr.ipaddress = 
				htonl(pktP->data.errormsg.node.unode.srvr.ipaddress);
			pktP->data.errormsg.node.unode.srvr.cport = 
				htons(pktP->data.errormsg.node.unode.srvr.cport);
			break;
		}

		pktP->data.errormsg.node.type = 
			htonl(pktP->data.errormsg.node.type);

		switch (pktP->data.errormsg.snode.type)
		{
		case NODE_PHONODE:
			htonPhonode(&pktP->data.errormsg.snode.unode.phonode);
			break;
		case NODE_SRVR:
			pktP->data.errormsg.snode.unode.srvr.ipaddress = 
				htonl(pktP->data.errormsg.snode.unode.srvr.ipaddress);
			pktP->data.errormsg.snode.unode.srvr.cport = 
				htons(pktP->data.errormsg.snode.unode.srvr.cport);
			break;
		}

		pktP->data.errormsg.snode.type = 
			htonl(pktP->data.errormsg.snode.type);
		pktP->data.errormsg.errortype = 
			htonl(pktP->data.errormsg.errortype);
		pktP->data.errormsg.errornumber = 
			htonl(pktP->data.errormsg.errornumber);
		
		break;
	case PKT_NEXTIME:
		pktP->data.nextime.referenceTimestamp[0] = 
			htonl(pktP->data.nextime.referenceTimestamp[0]);
		pktP->data.nextime.originateTimestamp[0] = 
			htonl(pktP->data.nextime.originateTimestamp[0]);
		pktP->data.nextime.receiveTimestamp[0] = 
			htonl(pktP->data.nextime.receiveTimestamp[0]);
		pktP->data.nextime.transmitTimestamp[0] = 
			htonl(pktP->data.nextime.transmitTimestamp[0]);
		pktP->data.nextime.clientReference = 
			htonl(pktP->data.nextime.clientReference);
		
		break;
	default:
		break;
	}
	return 0;
}

int
ntohPkt(int type, Pkt *pktP)
{
	pktP->type = ntohl(pktP->type);
	pktP->totalLen = ntohl(pktP->totalLen);

	if (type != PKT_XML)
	{
		/* Change the credo */
		ntohPhonode(&pktP->credo);
	}

	switch (type)
	{
	case PKT_REGISTER:
	case PKT_PROXYREGISTER:
	case PKT_REGISTER_SUCCESS:
	case PKT_REGISTER_FAILURE:
	case PKT_UNREGISTER:
		ntohPhonode(&pktP->data.reginfo.node);
		pktP->data.reginfo.reason = ntohl(pktP->data.reginfo.reason);

		break;
	case PKT_FIND:
	case PKT_FOUND:
	case PKT_NOTFOUND:
		ntohPhonode(&pktP->data.find.node);
		ntohPhonode(&pktP->data.find.fnode);
		ntohPhonode(&pktP->data.find.anode);

		break;
	case PKT_WAKEUP:
	case PKT_CONNECT:
	case PKT_ALERTED:
	case PKT_BUSY:
	case PKT_HANGUP:
		ntohPhonode(&pktP->data.wakeup.snode);
		ntohPhonode(&pktP->data.wakeup.dnode);
		ntohs(pktP->data.wakeup.snetport);
		ntohs(pktP->data.wakeup.dnetport);

		break;
	case PKT_DND:
		ntohPhonode(&pktP->data.dnd.node);
		pktP->data.dnd.enableCall = ntohl(pktP->data.dnd.enableCall);

		break;
	case PKT_REDIRECT:
	case PKT_PROXY:
		ntohPhonode(&pktP->data.redirect.onode);
		ntohPhonode(&pktP->data.redirect.nnode);
		pktP->data.redirect.protocol = ntohl(pktP->data.redirect.protocol);
		pktP->data.redirect.valid = ntohl(pktP->data.redirect.valid);

		break;
	case PKT_CONTROL:
		ntohPhonode(&pktP->data.control.snode);
		ntohPhonode(&pktP->data.control.dnode);
		pktP->data.control.hold = ntohl(pktP->data.control.hold);
		pktP->data.control.mute = ntohl(pktP->data.control.mute);

		break;
	case PKT_TRANSFER:
	case PKT_TRANSFERRED:
		ntohPhonode(&pktP->data.transfer.transferer);
		ntohPhonode(&pktP->data.transfer.transferee);
		ntohPhonode(&pktP->data.transfer.dest);
		pktP->data.transfer.transfererTransfereeNetport = 
			ntohs(pktP->data.transfer.transfererTransfereeNetport);
		pktP->data.transfer.transfererDestNetport = 
			ntohs(pktP->data.transfer.transfererDestNetport);	
		pktP->data.transfer.transfereeNetport = 
			ntohs(pktP->data.transfer.transfereeNetport);
		pktP->data.transfer.destNetport = 
			ntohs(pktP->data.transfer.destNetport);
		pktP->data.transfer.role = 
			ntohs(pktP->data.transfer.role);
		pktP->data.transfer.localState = 
			ntohs(pktP->data.transfer.localState);	
		break;

	case PKT_SERVERMSG:
		break;
	case PKT_PROFILE:
	case PKT_HEARTBEAT:
		ntohPhonode(&pktP->data.profile.local);
		ntohPhonode(&pktP->data.profile.remote);
		pktP->data.profile.aloidIpAddress = 
			ntohl(pktP->data.profile.aloidIpAddress);
		pktP->data.profile.vpnsIpAddress = 
			ntohl(pktP->data.profile.vpnsIpAddress);
		pktP->data.profile.ftime = 
			ntohl(pktP->data.profile.ftime);
		pktP->data.profile.ctime = 
			ntohl(pktP->data.profile.ctime);
		pktP->data.profile.sltime = 
			ntohl(pktP->data.profile.sltime);
		pktP->data.profile.rltime = 
			ntohl(pktP->data.profile.rltime);
		pktP->data.profile.lastSeqNo = 
			ntohs(pktP->data.profile.lastSeqNo);
		break;
	case PKT_XML:
		break;
	case PKT_ERROR:
		pktP->data.errormsg.snode.type = 
			ntohl(pktP->data.errormsg.snode.type);
		switch (pktP->data.errormsg.node.type)
		{
		case NODE_PHONODE:
			ntohPhonode(&pktP->data.errormsg.node.unode.phonode);
			break;
		case NODE_SRVR:
			pktP->data.errormsg.node.unode.srvr.ipaddress = 
				ntohl(pktP->data.errormsg.node.unode.srvr.ipaddress);
			pktP->data.errormsg.node.unode.srvr.cport = 
				ntohs(pktP->data.errormsg.node.unode.srvr.cport);
			break;
		}

		switch (pktP->data.errormsg.snode.type)
		{
		case NODE_PHONODE:
			ntohPhonode(&pktP->data.errormsg.snode.unode.phonode);
			break;
		case NODE_SRVR:
			pktP->data.errormsg.snode.unode.srvr.ipaddress = 
				ntohl(pktP->data.errormsg.snode.unode.srvr.ipaddress);
			pktP->data.errormsg.snode.unode.srvr.cport = 
				ntohs(pktP->data.errormsg.snode.unode.srvr.cport);
			break;
		}

		pktP->data.errormsg.errortype = 
			ntohl(pktP->data.errormsg.errortype);
		pktP->data.errormsg.errornumber = 
			ntohl(pktP->data.errormsg.errornumber);
		
		break;
	case PKT_NEXTIME:
		pktP->data.nextime.referenceTimestamp[0] = 
			ntohl(pktP->data.nextime.referenceTimestamp[0]);
		pktP->data.nextime.originateTimestamp[0] = 
			ntohl(pktP->data.nextime.originateTimestamp[0]);
		pktP->data.nextime.receiveTimestamp[0] = 
			ntohl(pktP->data.nextime.receiveTimestamp[0]);
		pktP->data.nextime.transmitTimestamp[0] = 
			ntohl(pktP->data.nextime.transmitTimestamp[0]);
		pktP->data.nextime.clientReference = 
			ntohl(pktP->data.nextime.clientReference);
		
		break;
	default:
		break;
	}

	return 0;
}

int
ntohRegisterIndex(RegisterIndex *registerIndex)
{
	registerIndex->ipaddress = ntohl(registerIndex->ipaddress);

	return 0;
}

int
ExecuteScript(char *cmd, char *o, int n)
{
	FILE *out;
	
	out = popen(cmd, "r");

	if (out == NULL)
	{
		return -1;
	}
	
	if (n)
	{
		fread(o, 1, n, out);
	}

	pclose(out);

	return 0;
}

int
ExecuteScript2(char *cmd)
{
	return ExecuteScript(cmd, 0, 0);
}
