#include "uh323.h"
#include "uh323cb.h"
#include "gis.h"
#include "arq.h"
#include "firewallcontrol.h"
#include "nxosd.h"
#include "gk.h"
#include "uh323proto.h"

#include "ipstring.h"

extern int reInviteClc;

/*
 * Hook callbacks
 */
BOOL CALLCONV cmHookRecv(
	 IN      HPROTCONN           hConn,
	 IN      int                 nodeId,
	 IN      BOOL   	error);

BOOL CALLCONV cmHookSend(IN HPROTCONN hConn, IN int nodeId, IN BOOL error);

BOOL CALLCONV cmHookSendTo(IN HPROTCONN hConn,
						   IN int nodeId, IN int addrTo, IN BOOL error);

BOOL CALLCONV cmHookRecvFrom(IN HPROTCONN hConn,
							 IN int nodeId,
							 IN int addrFrom, IN BOOL multicast, IN BOOL error);

BOOL CALLCONV cmHookListen(		/*Before listen */
							  IN HPROTCONN hConn, IN int addr);


int CALLCONV cmHookListening(	/*After listen */
								IN HPROTCONN hConn, IN int addr, IN BOOL error);

int CALLCONV cmHookConnecting(	/*Before connect */
								 IN HPROTCONN hConn, IN int addr);

int CALLCONV cmHookInConn(		/*Incomming connect */
							 IN HPROTCONN hConn, IN int addrFrom, IN int addrTo);

int CALLCONV cmHookOutConn(		/*Outgoing connect */
							  IN HPROTCONN hConn,
							  IN int addrFrom, IN int addrTo, IN BOOL error);

void CALLCONV cmHookClose(IN HPROTCONN hConn);

SCMPROTOCOLEVENT cmProtocolEvent = {NULL, NULL, NULL, cmHookInConn, NULL, cmHookSend, cmHookRecv, cmHookSendTo, NULL, NULL};

int extractH245MessagesFromFacility(HPVT			hVal,
                         int            messageBodyNode,
                         int            *currH245nodeId, /* for looping */
						 char*			msgString);

 
static void InsertTunnelH245Messages(HPVT			hVal,
						 int				rootH245node,
						 int				peernodeId,
						 char				encodeFlag);

#if 0 // just for reference
SCMPROTOCOLEVENT cmProtocolEvent =
{	cmHookListen,
	cmHookListening,
	cmHookConnecting,
	cmHookInConn,
	cmHookOutConn,
	cmHookSend,
	cmHookRecv,
	cmHookSendTo,
	cmHookRecvFrom,
	cmHookClose
};
#endif

void CALLCONV
cmHookClose(IN HPROTCONN hConn)
{
	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmHookClose -- \n"));

	return;
}

BOOL CALLCONV cmHookRecv(
	 IN      HPROTCONN           hConn,
	 IN      int                 nodeId,
	 IN      BOOL                error)
{
	char fn[] = "cmHookRecv():";
	char				msgname[80];
	int msgNodeId, paramNodeId, newNodeId,uuNodeId;
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	int 				h245nodeId;
	INT32 				ip, port;
	cmTransportAddress 	cmTransAddr;
	BOOL 				string;
	int					retval = -1;
    UH323CallAppHandle 	*appHandle;
	CallHandle			*callHandle;
	int					rv;
	int					drop = 0;
	int 				tunnodeId=-1;
	int 				tmpnodeId;
	int 				peernodeId=0;
	int 				h245rootnode=0;
	pthread_t			myid;
	char 				*msgptr;
	char				ipstr[32];
	MFCP_Request		*mfcpReq;
	unsigned int 		sPoolId;
	pMFCP_Session		callFceSession;
	unsigned int   		remoteBundleId;

	if(error)
	{
		NETERROR(MH323,(" -- cmHookRecv message decoding error %d -- \n",error));
		_handleHookError(hConn);
		return 1;
	}

	cmMeiEnter(UH323Globals()->hApp);

	if(!(msgptr = cmGetProtocolMessageName(UH323Globals()->hApp, nodeId)))
	{
		NETERROR(MH323,
			(" -- cmHookRecv unable to get message name-- \n"));
		cmMeiExit(UH323Globals()->hApp);
		return 0;
	}
	nx_strlcpy(msgname,msgptr,80);

	myid = pthread_self();

	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmHookRecv %s thread %lu -- \n", msgname, ULONG_FMT(myid)));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	/* FIX for H.245 not present in version 1 messages */
	// Remove H.245 addresses from incoming setup/alerting/proc
	if (!strcmp(msgname, "setup"))
	{
		if ((uuNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.setup.userUser.h323-UserInformation.h323-uu-pdu")) >= 0)
		{
			// remove h.245 address from setup if its there
			if ((paramNodeId = pvtGetNodeIdByPath(hVal, uuNodeId, 
				"h323-message-body.setup.h245Address")) > 0)
			{
				// remove this node id
				pvtDelete(hVal, paramNodeId);
			}

			if ((paramNodeId = pvtGetNodeIdByPath(hVal, uuNodeId, 
				"h323-message-body.setup.fastStart")) < 0)
			{
				if (paramNodeId <= 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG1,	
						("Disabling Tunneling field\n"));
					if(pvtBuildByPath( hVal, uuNodeId, "h245Tunneling", 0, NULL) <0)
					{
						NETERROR(MH323, ("Failed to Disable h245Tunneling\n"));
					}
				}
			}
		}
	}
	else if(!strcmp(msgname,"alerting"))
	{
		if ((paramNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.alerting.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.alerting.h245Address")) > 0)
		{
			// remove this node id
			pvtDelete(hVal, paramNodeId);
		}
	}
	else if(!strcmp(msgname,"proceeding"))
	{
		if ((paramNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.proceeding.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.proceeding.h245Address")) > 0)
		{
			// remove this node id
			pvtDelete(hVal, paramNodeId);
		}
	}
	else if(!strcmp(msgname,"progress"))
	{
		if ((paramNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.progress.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.progress.h245Address")) > 0)
		{
			// remove this node id
			pvtDelete(hVal, paramNodeId);
		}
	}
	else if(!strcmp(msgname,"releaseComplete"))
	{
    	rv = cmGetTpktChanApplHandle(hConn,(HAPPCONN*)&appHandle);

		if(!appHandle)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
			("%s %s Failed to retrieve appHandle\n",fn,msgname));
			goto _return;
		}
		
		appHandle->flags |= FL_CALL_RXRELCOMP;
	}
	else 
loop_over_tunnelled_h245_olcs:
		if(!strcmp(msgname,"openLogicalChannel") || 
			(!strcmp(msgname, "facility") && 
			((tunnodeId = pvtGetNodeIdByPath(hVal, nodeId,
			"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h245Control"))>=0) &&
			 (h245rootnode = extractH245MessagesFromFacility(hVal, tunnodeId, &peernodeId, "openLogicalChannel")) ))
    {
        int fieldId = 0,sid = 0,flag = 0,synNodeId;

		if (tunnodeId < 0)
		{ 
			tmpnodeId = nodeId ;
		}
		else
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, (" -- openLogicalChannel tunneled inside %s-- \n", msgname));
			tmpnodeId = h245rootnode;
		}

        //pvtPrintStd(hVal,nodeId,72);
        //pvtPrintStd(hVal,nodeId,51);
       if ((paramNodeId = pvtGetByPath(hVal, tmpnodeId,
            "request.openLogicalChannel.forwardLogicalChannelParameters.dataType.data.application.t38fax",&fieldId,&sid,&flag)) > 0)
        {
            NETDEBUG(MH323, NETLOG_DEBUG1,("%s received olc for fax\n", fn));
			if ((paramNodeId = pvtGetByPath(hVal, tmpnodeId,
				"request.openLogicalChannel.forwardLogicalChannelParameters.multiplexParameters.h2250LogicalChannelParameters.sessionID",&fieldId,&sid,&flag)) > 0)
			{
				if(sid != 3)
				{
					if(pvtSet(hVal,paramNodeId,-1,3,NULL) <0)
					{
						NETERROR(MH323, 
							("%s failed to set fax sessionID to 3\n", fn));
					}
				}
			}	
		}


		if (tunnodeId >= 0)
		{
			int		bronodeId;
			bronodeId = pvtBrother(hVal, peernodeId);
			InsertTunnelH245Messages(hVal, h245rootnode, peernodeId, TRUE);
			/* anymore tunnlled messages left ? */
			tunnodeId = -1;
			if (bronodeId > 0)
			{
				goto loop_over_tunnelled_h245_olcs;
			}
			peernodeId = 0;
		}
	}
	else 
loop_over_tunnelled_h245_reqmodes:
		if(!strcmp(msgname,"requestMode") ||
			(!strcmp(msgname, "facility") && 
			((tunnodeId = pvtGetNodeIdByPath(hVal, nodeId,
			"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h245Control"))>=0) &&
			(h245rootnode = extractH245MessagesFromFacility(hVal, tunnodeId, &peernodeId, "requestMode")) ))
	{
		/* Radvision stack has trouble handling 2 simultaneous reqmode */
		/* So has clarent. Only Vocaltec does this stupid thing */
		rv = cmGetTpktChanApplHandle(hConn,(HAPPCONN*)&appHandle);

		if(!appHandle)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
			("%s %s Failed to retrieve appHandle\n",fn,msgname));
			goto _return;
		}

		if(!(callHandle = CacheGet(callCache,appHandle->callID)))
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
				("%s Failed to find callHandle\n",fn));
			goto _return;
		}
		else if(!H323reqMode(callHandle))
			{
				H323reqMode(callHandle) = 1;
			}
			else {
				drop = 1;
			}

		if (tunnodeId >= 0)
		{
			int		bronodeId;
			bronodeId = pvtBrother(hVal, peernodeId);
			InsertTunnelH245Messages(hVal, h245rootnode, peernodeId, FALSE);
			/* anymore tunnlled messages left ? */
			tunnodeId = -1;
			if (bronodeId > 0)
			{
				goto loop_over_tunnelled_h245_reqmodes;
			}
			peernodeId = 0;
		}
	}
	else if(!strcmp(msgname,"connect"))
	{
		if ((paramNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.connect.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.connect.h245Address")) < 0)
		{
			// nothing leave it
			goto _return;
		}

    	rv = cmGetTpktChanApplHandle(hConn,(HAPPCONN*)&appHandle);

		if(!appHandle)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
			("%s %s Failed to retrieve appHandle\n",fn,msgname));
			goto _return;
		}

		if (appHandle->flags & FL_CALL_H245CTRLCONN)
		{
			// We have the connection setup already, there is no need
			// to open any new holes.
			NETDEBUG(MH323, NETLOG_DEBUG4,	
			("%s %s flags = %x deleting h245 address\n",fn,msgname,appHandle->flags));
			pvtDelete(hVal, paramNodeId);

			// nothing leave it
			goto _return;
		}
		else
		{
			// BUG 3906: MSW should not open outgoing holes. 
			// These may lead to leaks
			NETDEBUG(MH323, NETLOG_DEBUG4,	
				("%s %s No need to open hole for outgoing H.245 connection\n",
				fn,msgname));

			goto _return;
		}

#if 0
		// This is neccesary for v4stack to keep on doing
		// H.245 when fst fails.
		// remove this node id
		pvtDelete(hVal, paramNodeId);

		// nothing leave it
		goto _return;
#endif

		if ( (h245nodeId = pvtGetNodeIdByPath(hVal, paramNodeId,
			"ipAddress.ip"))< 0)
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("%s H.245 Ip not present\n",fn));
			goto _return;
		}

		if (pvtGetString(hVal, h245nodeId, 4, 
			(char *)&ip) < 0)
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("%s H.245 Ip not present\n",fn));
			goto _return;
		}

		if (pvtGetByPath(hVal, paramNodeId,
			"ipAddress.port", NULL,
				&port, &string) < 0)
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("%s H.245 port not present\n",fn));
			goto _return;
		}

		DEBUG(MH323, NETLOG_DEBUG1,
			("%s IP: %s, port %d\n",fn,FormatIpAddress(ip,ipstr),port));

		cmTransAddr.ip = ip;
		cmTransAddr.port = port;

		NETDEBUG(MFCE, NETLOG_DEBUG4, ("Opening H.245 pinhole upon CONNECT from 0x%08x:%d\n", ip, port));
		if(!(callHandle = CacheGet(callCache,appHandle->callID)))
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
				("%s Failed to find callHandle\n",fn));
			goto _return;
		}

		if (callHandle->realmInfo->sPoolId)
		{
                	if (CallFceSession(callHandle))
				H323remoteBundleId(callHandle) = FCEGetNextBundleId(CallFceSession(callHandle));
                	else
				CallFceSession(callHandle) = FCEAllocateBundleId(&H323remoteBundleId(callHandle));
                	if (CallFceSession(callHandle) == NULL)
			{
				NETERROR(MFCE, ("%s: Error opening H.245 pinhole upon incoming CONNECT\n", fn));
			}
			else
			{

				sPoolId = callHandle->realmInfo->sPoolId;
				callFceSession = CallFceSession(callHandle);
				remoteBundleId = H323remoteBundleId(callHandle);

				/* holding locks results in deadlock in mfcp processing */

				CacheReleaseLocks(callCache);

				mfcpReq = FCEOpenResourceSync(callFceSession,
								remoteBundleId,
								0,
								FCE_ANY_IP,
								FCE_ANY_PORT,
								ntohl(ip),
								port,
								0,
								0,
								0,
								0,
								sPoolId,
								sPoolId,
								"tcp",
								0,
								0,
								0);

				CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
				if(!(callHandle = CacheGet(callCache,appHandle->callID)))
				{
					NETDEBUG(MH323, NETLOG_DEBUG4,	
					  ("%s Failed to find callHandle after FCEOpenResourceSync\n",fn));
				  goto _return;
				}
				if (mfcpReq == NULL)
				{
					NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Error opening H.245 pinhole upon incoming CONNECT\n", fn));
					H323remoteBundleId(callHandle) = 0;
				}
				else
				{
					cmTransAddr.ip = htonl(mfcp_get_nat_dest_addr(mfcpReq));
					cmTransAddr.port = htons(mfcp_get_nat_dest_port(mfcpReq));
					mfcp_req_free(mfcpReq);
				}

				// set new address
				if (pvtBuildByPath(hVal, paramNodeId,
					"ipAddress.ip", 4, (char *)&cmTransAddr.ip) < 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG1,	
						("%s Failed to set h245ip address\n", fn));
					goto _return;
				}

				if (pvtBuildByPath(hVal, paramNodeId,
					"ipAddress.port",cmTransAddr.port,NULL) < 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG1,	
						("%s Failed to set h245 port\n", fn));
				}
			}
		}
	}

_return:
	cmMeiExit(UH323Globals()->hApp);
	CacheReleaseLocks(callCache);
	return drop;

}

BOOL CALLCONV 
cmHookSend(IN HPROTCONN hConn, IN int nodeId, IN BOOL error)
{
	static char			fn[] = "cmHookSend";
    UH323CallAppHandle 	*appHandle = NULL;
	cmCallParam			param = -1;
	char				msgname[80];
	int					addrlen;
	cmTransportAddress 	cmTransAddr = { 0 };
	CallHandle			*callHandle;
	int					rv;
	unsigned int		rla = 0;
	unsigned short		rlp = 0;
	int					retval = -1, throwMsg = 0;
	INT32 				ip, port;
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	int 				facnodeId = -1;
	int 				h245nodeId;
	int 				tunnodeId=-1;
	int 				tmpnodeId;
	int 				peernodeId=0;
	int 				h245rootnode=0;
	BOOL 				string;
	int					facility = 0;
	HCALL				hsCall;
	char 				*msgptr;
	int					layer1 = 0;
	HPROTCONN			h245conn;
	char				ipstr[32];
	MFCP_Request			*mfcpReq;
	int					cdpntype;
	unsigned int 		sPoolId;
	pMFCP_Session 		callFceSession;
	unsigned int   		localBundleId;

	if(error)
	{
		NETERROR(MH323,(" -- cmHookSend message decoding error %d -- \n",error));
		_handleHookError(hConn);
		return 1;
	}

	cmMeiEnter(UH323Globals()->hApp);
	if(!(msgptr = cmGetProtocolMessageName(UH323Globals()->hApp, nodeId)))
	{
		NETERROR(MH323,
			(" -- cmHookSend unable to get message name-- \n"));
		cmMeiExit(UH323Globals()->hApp);
		return 0;
	}
	nx_strlcpy(msgname,msgptr,80);

	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmHookSend for %s-- \n", msgname));
	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

//	TimeStamp(MINIT, NETLOG_DEBUG1, msgname);

	rv = cmGetTpktChanApplHandle(hConn,(HAPPCONN*)&appHandle);

	if(!appHandle)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,	
			("%s %s Failed to retrieve appHandle\n",fn,msgname));
		goto _return;
	}

	// appHandle must exist now
	if(!(callHandle = CacheGet(callCache,appHandle->callID)))
	{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
				("%s Failed to find callHandle\n",fn));
			goto _return;
	}

	// callHandle must exist now

	hsCall = H323hsCall(callHandle);

	if (!strcmp(msgname, "setup"))
	{
		int cgpnNodeId;
		// we will remove h.245 address from setup always
		// param = cmParamSetupH245Address;
#if 0
		if ((cgpnNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.setup.callingPartyNumber")) > 0)
		{
			NETDEBUG(MH323,NETLOG_DEBUG4, ("found cgpnNode id\n"));
			if(pvtBuildByPath( hVal, cgpnNodeId,
					"octet3.screeningIndicator",
					3, NULL) <0 )
			{
				NETERROR(MH323, ("%s could not set screeing Indicator in setup\n",fn));
			}
		}
#endif

	}
	else if (!strcmp(msgname, "connect"))
	{
    	rv = cmGetTpktChanApplHandle(hConn,(HAPPCONN*)&appHandle);

		if(!appHandle)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
			("%s %s Failed to retrieve appHandle\n",fn,msgname));
			goto _return;
		}

		if(!(callHandle = CacheGet(callCache,appHandle->callID)))
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
				("%s Failed to find callHandle\n",fn));
			goto _return;
		}

		// Delete the h.245 address if the endpt is marked to do that
		// AND fast start was successful.
		if ((callHandle->ecaps1 & ECAPS1_NOCONNH245) &&
			callHandle->fastStartStatus)
		{
			// remove h.245 address from connect if its there
			if ((h245nodeId = pvtGetNodeIdByPath(hVal, nodeId, 
				"message.connect.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.connect.h245Address")) > 0)
			{
				// remove this node id
				pvtDelete(hVal, h245nodeId);
			}
		}
		if (callHandle->fastStartStatus &&
    			(appHandle->flags & FL_CALL_H245CTRLCONN) && !fsInConnect)
		{
			int					nodeId = -1;
   			if( cmCallGetParam(H323hsCall(callHandle),
	        		cmParamConnectFastStart,
	        		0,
	       	 		&nodeId,
	        		NULL) >=0)
			{
   		 		NETDEBUG(MH323,NETLOG_DEBUG4,
	       		    ("%s cmCallGetParam nodeid = %d. Connect\n",
       	 			   fn,nodeId));	
				pvtDelete(hVal,nodeId);
				NETDEBUG(MH323,NETLOG_DEBUG4, ("%s Deleting Connect fastStart Node as H.245 established%d\n", fn, nodeId));
			}
		}
	}

#ifdef H323v30
	if(!reInviteClc)
// For v4 - there is a problem in outgoing olcack - it has incorrect address
#endif
	{
		tunnodeId = -1;
		/* If we are not closing logicalChannel on reInvite
		*  then rtp/rtcp mismatch may be there 
		*/

loop_process_tunnelled_h245_olcacks:  /* processes more than one OLCAck in a facility */
		if (!strcmp(msgname, "openLogicalChannelAck") ||
			(!strcmp(msgname, "facility") && 
			((tunnodeId = pvtGetNodeIdByPath(hVal, nodeId,
			"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h245Control"))>=0) &&
			 (h245rootnode = extractH245MessagesFromFacility(hVal, tunnodeId, &peernodeId, "openLogicalChannelAck")) ))
		{
			int rtpport = 0, rtcpport = 0, rtpaddr = 0;

			if (tunnodeId < 0)
			{ 
				tmpnodeId = nodeId ;
			}
			else
			{
				NETDEBUG(MH323, NETLOG_DEBUG1, (" -- openLogicalChannelAck tunneled inside %s-- \n", msgname));
				tmpnodeId = h245rootnode;
			}
			
			// make sure that the rtp and the rtcp addresses
			// are appropriate
			if ((facnodeId = pvtGetByPath(hVal, tmpnodeId, 
				"response.openLogicalChannelAck.forwardMultiplexAckParameters.h2250LogicalChannelAckParameters.mediaChannel.unicastAddress.iPAddress.tsapIdentifier", NULL, &rtpport, &string)) < 0)
			{
				NETERROR(MH323, ("invalid node id\n"));
			}
			if ((facnodeId = pvtGetNodeIdByPath(hVal, tmpnodeId, 
				"response.openLogicalChannelAck.forwardMultiplexAckParameters.h2250LogicalChannelAckParameters.mediaChannel.unicastAddress.iPAddress.network")) < 0)
			{
				NETERROR(MH323, ("invalid node id\n"));
			}
			if (pvtGetString(hVal, facnodeId, 4, (char *)&rtpaddr) < 0)
			{
				NETERROR(MH323, ("invalid node id\n"));
			}

			if ((facnodeId = pvtGetByPath(hVal, tmpnodeId, 
				"response.openLogicalChannelAck.forwardMultiplexAckParameters.h2250LogicalChannelAckParameters.mediaControlChannel.unicastAddress.iPAddress.tsapIdentifier", NULL, &rtcpport, &string)) < 0)
			{
				// we must add the rtcp address into it
				rtcpport = -10;
			}

			if (rtcpport != (rtpport +1))
			{
				if (pvtBuildByPath(hVal, tmpnodeId,
					"response.openLogicalChannelAck.forwardMultiplexAckParameters.h2250LogicalChannelAckParameters.mediaControlChannel.unicastAddress.iPAddress.tsapIdentifier",rtpport+1,NULL) < 0)
				{
					NETERROR(MH323, ("invalid node id\n"));
				}

				if (pvtBuildByPath(hVal, tmpnodeId,
					"response.openLogicalChannelAck.forwardMultiplexAckParameters.h2250LogicalChannelAckParameters.mediaControlChannel.unicastAddress.iPAddress.network",4,(char *)&rtpaddr) < 0)
				{
					NETERROR(MH323, ("invalid node id\n"));
				}
			}

			if (tunnodeId >= 0)
			{
				int		bronodeId;
				bronodeId = pvtBrother(hVal, peernodeId);
				InsertTunnelH245Messages(hVal, h245rootnode, peernodeId, TRUE);
				/* anymore tunnlled messages left ? */
				tunnodeId = -1;
				if (bronodeId > 0)
				{
					goto loop_process_tunnelled_h245_olcacks;
				}
				peernodeId = 0;
			}
			goto _return;
		}
	}

	// Set up the H.245 app handle. We need to do this only if
	// we haven't done this yet
	if (!(appHandle->flags & FL_CALL_H245APPINIT))
	{
		if ((h245conn = cmGetTpktChanHandle(hsCall,cmH245TpktChannel)))
		{
			if(cmSetTpktChanApplHandle(h245conn,(HAPPCONN)appHandle) < 0)
			{
				NETERROR(MH323,("%s : cmGetTpktChanApplHandle Failed\n",
					fn));
			}
			else
			{
				appHandle->flags |= FL_CALL_H245APPINIT;
				appHandle->h245Conn = h245conn;

				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s : H245 Tpkt app handle initialized in %s\n", 
					fn, msgname));
			}
		}
		else
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s : H245 Tpkt channel doesn't exist yet for %s\n", 
				fn, msgname));
		}
	}

	if(!strcmp(msgname,"requestModeAck") ||
		(!strcmp(msgname, "facility") && 
		((tunnodeId = pvtGetNodeIdByPath(hVal, nodeId,
		"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h245Control"))>=0) &&
		 (h245rootnode = extractH245MessagesFromFacility(hVal, tunnodeId, &peernodeId, "requestModeAck")) ))
	{
		H323reqMode(callHandle) = 0;
		if (tunnodeId >= 0)
		{
			/* TRUE then peernodeId (h245Control.0 pr h245Control.1 ...) that was decode
			 * and processed will be deleted. So do a pvtBrother before deleting it
			 */
			InsertTunnelH245Messages(hVal, h245rootnode, peernodeId, FALSE);
			tunnodeId = -1;
		}
		goto _return;
	}
	// Facility message
	else if(!strcmp(msgname,"facility"))
	{
		facility = 1;	
		if ((facnodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.facility")) < 0)
		{
			goto _return;
		}

		if ((pvtGetNodeIdByPath(hVal, facnodeId, 
				"reason.starth245") >= 0) || 
			(pvtGetNodeIdByPath(hVal, facnodeId,
				"reason.startH245") >= 0) )
		{
			DEBUG(MH323, NETLOG_DEBUG1, ("facility reason = starth245\n"));

			if(routeH245)
			{
				if(appHandle->flags & FL_CALL_H245CTRLCONN)
				{
					// We must discard this facility message
					throwMsg = 1;
					NETDEBUG(MH323, NETLOG_DEBUG1, 
						("Control Already setup, facility msg not needed\n"));
					goto _return;
				}

					if ( (h245nodeId = pvtGetNodeIdByPath(hVal, facnodeId,
						"h245Address.ipAddress.ip"))< 0)
					{
						DEBUG(MH323, NETLOG_DEBUG1,
							("%s H.245 Ip not present\n",fn));
						goto _return;
					}

					if (pvtGetString(hVal, h245nodeId,
						4, 
						(char *)&ip) < 0)
					{
						DEBUG(MH323, NETLOG_DEBUG1,
							("%s H.245 Ip not present\n",fn));
						goto _return;
					}

					if (pvtGetByPath(hVal, facnodeId,
						"h245Address.ipAddress.port", NULL,
						&port, &string) < 0)
					{
						DEBUG(MH323, NETLOG_DEBUG1,
							("%s H.245 port not present\n",fn));
						goto _return;
					}

					DEBUG(MH323, NETLOG_DEBUG1,
						("%s IP: %s, port %d\n",fn,FormatIpAddress(ip,ipstr),port));

					cmTransAddr.ip = ip;
					cmTransAddr.port = port;

			}			
			else
			{
				goto _return;
			}
		}
		else
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("MSW originated non H.245 fac msg\n"));
			goto _return;
		}
	} 
	else if(!strcmp(msgname,"alerting"))
	{
		param = cmParamAlertingH245Address;
		facility=0;
		if ((facnodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.alerting.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.alerting.h245Address")) < 0)
		{
			goto _return;
		}

		if(appHandle->flags & FL_CALL_H245CTRLCONN)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("control is already connected, deleting h.245 address from %s\n", 
				msgname));
			pvtDelete(hVal, facnodeId);
			goto _return;
		}
		if (callHandle->fastStartStatus &&
    			(appHandle->flags & FL_CALL_H245CTRLCONN) && !fsInConnect)
		{
			int					nodeId = -1;
   			if( cmCallGetParam(H323hsCall(callHandle),
	        		cmParamAlertingFastStart,
	        		0,
	       	 		&nodeId,
	        		NULL) >=0)
			{
   		 		NETDEBUG(MH323,NETLOG_DEBUG4,
	       		    ("%s cmCallGetParam nodeid = %d. Alerting\n",
       	 			   fn,nodeId));	
				pvtDelete(hVal,nodeId);
				NETDEBUG(MH323,NETLOG_DEBUG4, ("%s Deleting Alerting fastStart Node as H.245 established%d\n", fn, nodeId));
			}
		}
	}
	else if(!strcmp(msgname,"proceeding"))
	{
		param = cmParamCallProcH245Address;
		facility=0;
		if ((facnodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.proceeding.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.proceeding.h245Address")) < 0)
		{
			goto _return;
		}

		if(appHandle->flags & FL_CALL_H245CTRLCONN)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("control is already connected, deleting h.245 address from %s\n", 
				msgname));
			pvtDelete(hVal, facnodeId);
			goto _return;
	 	}
		if (callHandle->fastStartStatus &&
    			(appHandle->flags & FL_CALL_H245CTRLCONN) && !fsInConnect)
		{
			int					nodeId = -1;
   			if( cmCallGetParam(H323hsCall(callHandle),
	        		cmParamCallProcFastStart,
	        		0,
           	 		&nodeId,
	        		NULL) >=0)
			{
    	 		NETDEBUG(MH323,NETLOG_DEBUG4,
	       		    ("%s cmCallGetParam nodeid = %d. Proceeding\n",
       	 			   fn,nodeId));	
				pvtDelete(hVal,nodeId);
				NETDEBUG(MH323,NETLOG_DEBUG4, ("%s Deleting Proceeding fastStart Node as H.245 established%d\n", fn, nodeId));
			}
		}
	}
	else if(!strcmp(msgname,"connect"))
	{
		param = cmParamH245Address;
		facility=0;
		if ((facnodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.connect.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.connect.h245Address")) < 0)
		{
			goto _return;
		}

		if(appHandle->flags & FL_CALL_H245CTRLCONN)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("control is already connected, deleting h.245 address from %s\n", 
				msgname));
			pvtDelete(hVal, facnodeId);
			goto _return;
		}
	}
	else 
	{
		goto _return;
	}

	if(!facility)
	{
		if(cmCallGetParam(hsCall,
					param, //cmParamH245Address,
					0,
					&addrlen,
					(char *)&cmTransAddr) <0)
		{
				NETDEBUG(MH323, NETLOG_DEBUG4,	
					("%s Failed to find H245 address\n",fn));
				goto _return;	
		}
	}

	if ((cmTransAddr.ip == 0) || (cmTransAddr.port == 0))
	{
		// ip address should have been initialized
		NETERROR(MH323, 
			("%s failed to initialize cmTransAddr.ip/port %s/%d\n",
			fn, FormatIpAddress(cmTransAddr.ip,ipstr),cmTransAddr.port));
		goto _return;
	}

	/* We don't have peer h245 address or control session */
	if((H323controlState(callHandle) == UH323_sControlError) ||
		(H323controlState(callHandle) == UH323_sControlIdle) )
	{
		if (callHandle->realmInfo->sPoolId)
		{
			/* we are sending setup/alerting/connect/facility, open hole for H.245 */
			if (H323localBundleId(callHandle) == 0)
			{
				/* opening hole for the first time */
				NETDEBUG(MFCE, NETLOG_DEBUG4, ("Attempting to open H.245 hole for outgoing %s to %08x:%d\n", msgname, ntohl(cmTransAddr.ip), cmTransAddr.port));
                		if (CallFceSession(callHandle))
					H323localBundleId(callHandle) = FCEGetNextBundleId(CallFceSession(callHandle));
                		else
					CallFceSession(callHandle) = FCEAllocateBundleId(&H323localBundleId(callHandle));
                		if (CallFceSession(callHandle) == NULL)
				{
					NETERROR(MFCE, ("%s: Error opening H.245 pinhole for outgoing %s\n", fn, msgname));
					rla = cmTransAddr.ip;
				       	rlp = cmTransAddr.port;
				}
				else
				{
					sPoolId= callHandle->realmInfo->sPoolId;
					callFceSession = CallFceSession(callHandle);
					localBundleId = H323localBundleId(callHandle);

					  /* holding locks results in deadlock in mfcp processing */

					CacheReleaseLocks(callCache);
					mfcpReq = FCEOpenResourceSync(callFceSession,
												localBundleId,
												0,
												FCE_ANY_IP,
												FCE_ANY_PORT,
												ntohl(cmTransAddr.ip),
												cmTransAddr.port,
												0,
												0,
												0,
												0,
												sPoolId,
												sPoolId,
												"tcp",
												0,
												0,
												0);

					CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
					if(!(callHandle = CacheGet(callCache,appHandle->callID)))
					{
						NETDEBUG(MH323, NETLOG_DEBUG4,	
						  ("%s Failed to find callHandle after FCEOpenResourceSync\n",fn));
					  goto _return;
					}
					if (mfcpReq == NULL)
					{
						H323localBundleId(callHandle) = 0;
						rla = cmTransAddr.ip;
						rlp = cmTransAddr.port;
					}
					else
					{
						rla = H323localTranslatedIp(callHandle) = htonl(mfcp_get_nat_dest_addr(mfcpReq));;
						rlp = H323localTranslatedPort(callHandle) = (mfcp_get_nat_dest_port(mfcpReq));
						mfcp_req_free(mfcpReq);
					}
				}
			}
			else
			{
				/* hole already opened, just resend the translated ip/port */
				NETDEBUG(MFCE, NETLOG_DEBUG4, ("Using previously opened H.245 hole (%08lx:%d, session id %d) for outgoing %s\n", H323localTranslatedIp(callHandle), H323localTranslatedPort(callHandle), H323localBundleId(callHandle), msgname));
				rla = H323localTranslatedIp(callHandle);
				rlp = H323localTranslatedPort(callHandle);
			}
	
	
			cmTransAddr.ip = rla;
			cmTransAddr.port = rlp;
			addrlen = sizeof(cmTransportAddress);
			NETDEBUG(MH323, NETLOG_DEBUG4,	
				("%s %s setting h245 address to %s/%d \n",
				fn,msgname,(char*) ULIPtostring(cmTransAddr.ip),cmTransAddr.port));
			if(facility)
			{
				if (!rla ||
					(pvtBuildByPath(hVal, facnodeId,
					"h245Address.ipAddress.ip", 4, (char *)&cmTransAddr.ip) < 0))
				{
					NETDEBUG(MH323, NETLOG_DEBUG1,	
						("%s Failed to set h245ip address\n", fn));
					goto _return;
				}

				if (!rlp ||
					(pvtBuildByPath(hVal, facnodeId,
					"h245Address.ipAddress.port",cmTransAddr.port,NULL) < 0))
				{
					NETDEBUG(MH323, NETLOG_DEBUG1,	
						("%s Failed to set h245 port\n", fn));
				}

			} 
			else if (param >= 0)
			{
				if(cmCallSetParam(hsCall,
							param,
						0,
						addrlen,
						(char *)&cmTransAddr) <0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG4, ("Error setting H245 address\n"));
				}
			}
			else
			{
				NETERROR(MH323, ("%s param not defined\n", fn));
			}
		}
		else
		{
			if (facility)
			// We are sending a facility message to private side
			// Need to put private address
			{
				cmTransAddr.ip = htonl(callHandle->realmInfo->rsa);
				if (pvtBuildByPath(hVal, facnodeId,
					"h245Address.ipAddress.ip", 4, (char *)&cmTransAddr.ip) < 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG1,	
						("%s Failed to set private h245ip address\n", fn));
					goto _return;
				}
			}
			else
			{
				cmTransAddr.ip = htonl(callHandle->realmInfo->rsa);
				if(cmCallSetParam(hsCall,
						param,
						0,
						addrlen,
						(char *)&cmTransAddr) <0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG4, ("Error setting H245 address\n"));
				}
			}
		}
	}
	else 
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,	("%s Have Control Session. \n",fn));
	}

_return:
	cmMeiExit(UH323Globals()->hApp);
	CacheReleaseLocks(callCache);
	return throwMsg;
}

//Incoming connect

int CALLCONV
cmHookInConn(	IN HPROTCONN hConn,
				IN int addrFrom,
				IN int addrTo)
{
	char fn[] = "cmHookInConn():";
   	UH323CallAppHandle 	*appHandle = NULL;
	CallHandle *callHandle = NULL;

	cmGetTpktChanApplHandle(hConn, (HAPPCONN*)&appHandle);

	// Make sure we have the right protocol channel

	// If there is a remote hole, close it
	if(appHandle && (appHandle->h245Conn == hConn))
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s detected H.245 connection coming in\n", fn));

		// Mark the connection as incoming
		appHandle->flags |= FL_CALL_H245CTRLIN;

#ifdef _moved_to_ctrlconnected
		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

		if (callHandle = CacheGet(callCache, appHandle->callID));
		{
			if (H323remoteBundleId(callHandle) != 0)
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s Closing remote H.245 hole, local hole used\n",
					fn));
				FCECloseBundle(CallFceSession(callHandle), H323remoteBundleId(callHandle));
				H323remoteBundleId(callHandle) = 0;
			}
		}

		CacheReleaseLocks(callCache);
#endif
	}

	return 0;
}

//Outgoing connect
// not used anymore for FCE
int CALLCONV
cmHookOutConn(	IN HPROTCONN hConn,
				IN int addrFrom,
				IN int addrTo,
				IN BOOL error)
{
	char fn[] = "cmHookOutConn():";
    	UH323CallAppHandle 	*appHandle = NULL;
	CallHandle *callHandle = NULL;

	cmGetTpktChanApplHandle(hConn, (HAPPCONN*)&appHandle);

	// If there is a remote hole, close it
	if(appHandle)
	{
		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

		if ((callHandle = CacheGet(callCache, appHandle->callID)))
		{
			if (H323localBundleId(callHandle) != 0)
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s Closing local H.245 hole, remote hole used\n",
					fn));
				FCECloseBundle(CallFceSession(callHandle), H323localBundleId(callHandle));
				H323localBundleId(callHandle) = 0;
			}
		}

		CacheReleaseLocks(callCache);
	}

	return 0;
}

BOOL CALLCONV
cmHookSendTo(IN HPROTCONN hConn, IN int nodeId, IN int addrTo, IN BOOL error)
{
	char msgname[256];
//	int len = strlen(prefix);
	int paramNodeId;
	struct timeval tp;
	char 				*msgptr;

	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmHookSendTo -- \n"));

	cmMeiEnter(UH323Globals()->hApp);

	if(!(msgptr = cmGetProtocolMessageName(UH323Globals()->hApp, nodeId)))
	{
		NETERROR(MH323,
			(" -- cmHookSendTo unable to get message name-- \n"));
		cmMeiExit(UH323Globals()->hApp);
		return 0;
	}
	nx_strlcpy(msgname,msgptr,256);

#if 0
	char md5[] =  "iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) md5(5)"; 
	int	oidlen = 128;
	char oidbuff[128]  = {0};
	char	hash[16] = {0};

	oidlen = oidEncodeOID(oidlen,oidbuff,md5);
	if(!strcmp(msgname,"gatekeeperConfirm"))
	{
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"gatekeeperConfirm.authenticationMode.pwdHash",
			0,NULL) < 0)
		{
			NETERROR(MH323, ("GCF could not add authenticationMode\n"));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"gatekeeperConfirm.algorithmOID",
			oidlen,oidbuff) < 0)
		{
			NETERROR(MH323, ("GCF could not add OID %d %s\n",oidlen,oidbuff));
		}
	}
#endif
	
#if 0
	if( len && !strcmp(msgname,"registrationRequest"))
	{
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"registrationRequest.terminalType.gateway.protocol.1.voice.supportedPrefixes.1.prefix.e164",
			len, prefix) < 0)
		{
			NETERROR(MH323, ("HookSendTo could not build prefix\n"));
		}
	}
#endif

#if 1
	if(!strcmp(msgname,"locationRequest"))
	{
		if ((paramNodeId = pvtGetNodeIdByPath(cmGetValTree(UH323Globals()->hApp), 
			nodeId, 
			"locationRequest.endpointIdentifier")) > 0)
		{
			// remove this node id
			pvtDelete(cmGetValTree(UH323Globals()->hApp), paramNodeId);
		}
	}
#endif

	cmMeiExit(UH323Globals()->hApp);

	return(0);
}

/* NOT being used */
BOOL CALLCONV
cmHookRecvFrom(IN HPROTCONN hConn,
			   IN int nodeId, IN int addrFrom, IN BOOL multicast, IN BOOL error)
{
	static char fn[] = "cmHookReceiveFrom";
	char msgname[256];
	HPVT 		hVal = cmGetValTree(UH323Globals()->hApp);
	static HPST	hPst = NULL; 
	int			newNodeId,setupNodeId;
	pthread_t			myid;
	char 		*msgptr;

	
	myid = pthread_self();

	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmHookReceiveFrom %s %lu -- \n",
		msgname, ULONG_FMT(myid)));

#if 0
	cmMeiEnter(UH323Globals()->hApp);

	if(!(msgptr = cmGetProtocolMessageName(UH323Globals()->hApp, nodeId)))
	{
		NETERROR(MH323,
			(" -- cmHookRecvFrom  unable to get message name-- \n"));
		cmMeiExit(UH323Globals()->hApp);
		return 0;
	}
	nx_strlcpy(msgname,msgptr,256);

	if(hPst== NULL)
	{
		hPst = pstConstruct(cmEmGetQ931Syntax(),"Setup-UUIE");
		if(hPst == NULL)
		{
			NETERROR(MH323,
				("cmHookSendTo pstConstruct PwdCertToken Error\n"));
		}
	}

	if(!strcmp(msgname,"admissionConfirm") )
	{
		pvtPrintStd(hVal,acfTokenNodeId,62);
		if ((acfTokenNodeId = pvtGetNodeIdByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"admissionConfirm.tokens")) < 0)
		{
			NETERROR(MH323, ("%s could not get token from ACF\n",fn));
		}
		newNodeId = pvtAddRoot(hVal,
				hPst,
				0,
				NULL);

		if (newNodeId < 0)
		{
			NETERROR(MH323,
			("%s pvtAddRoot  returns error\n",fn));
		}
		pvtPrintStd(hVal,newNodeId,83);

 		if(pvtAddTree(hVal,newNodeId,hVal,acfTokenNodeId) < 0)
        {
            NETERROR(MH323, ("%s could not set token in setup\n",fn));
        }

		if ((acfTokenNodeId = pvtGetNodeIdByPath(cmGetValTree(UH323Globals()->hApp),
			newNodeId,
			".tokens")) < 0)
		{
            NETERROR(MH323, ("%s could not get acfNodeId\n",fn));
		}

	}

	cmMeiExit(UH323Globals()->hApp);

#endif
	return(0);
}


int _handleHookError(HPROTCONN  hConn)
{
	static char			fn[] = "handleHookError";
    UH323CallAppHandle 	*appHandle;
	int					rv;
	CallHandle 			*callHandle;

	rv = cmGetTpktChanApplHandle(hConn,(HAPPCONN*)&appHandle);

	if(!appHandle)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,	
		("%s Failed to retrieve appHandle\n",fn));
		return 0;
	}

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if(!(callHandle = CacheGet(callCache,appHandle->callID)))
	{
			NETDEBUG(MH323, NETLOG_DEBUG4,	
				("%s Failed to find callHandle\n",fn));
	}
	else {
			cmCallDrop(H323hsCall(callHandle));
	}
	CacheReleaseLocks(callCache);
	return 0;
}

#if 0
/* take this function out for v4 stack */

unsigned char * stSyntax();

unsigned char * cmEmGetQ931Syntax(void)
{

	HPST        hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"q931");

	return stSyntax(hSyn);
}
#endif

/**************************************************************************************
 * extractH245MessagesFromFacility
 *
 * putrpose: extracts, decodes and lookup the tunneled H.245 messages from the given message.
 *
 * Input: hVal				- 
 *        messageBodyNode   - the h245Control part of the message.
 *        msgString			- The string name H.245 message which we are looking for.
 * Output:
 ***************************************************************************************/
int extractH245MessagesFromFacility(HPVT			hVal,
                         int            messageBodyNode,
                         int            *lastH245NodeId, /* for looping */
						 char*			msgString)
{
    int     encodedOctedStrings;
    int     nodeId;
    int     msgLen;
    int     pvtNode;
    int     lastNodeId = *lastH245NodeId;
	BYTE	buffer[MAX_ENCODEDECODE_BUFFRSIZE]; /* maxMessageSize+MSG_HEADER_SIZE+TPKT_HEADER_SIZE */
	HPST	synTreeH245= cmGetSynTreeByRootName(UH323Globals()->hApp, "h245");

	/* user has supplied a h245 message node */
	if (lastNodeId)
	{
        /* go to the next encoded tunneled message */
        nodeId = pvtBrother(hVal, lastNodeId);
	}
	else
	{
		/* begin with nod for "h245Control" */
		nodeId = pvtChild(hVal, messageBodyNode);
	}

	/* loop over all the tunneled messages */
    while (nodeId > 0)
    {
        /* read the encoded message from the pvt into the buffer */
        msgLen = pvtGetString(hVal, nodeId, MAX_ENCODEDECODE_BUFFRSIZE, (char *)buffer);
        if (msgLen > 0)
        {
            int decoded_length = 0;

            /* build an H.245 message tree and decode the message into it */
            pvtNode = pvtAddRoot(hVal, synTreeH245, 0, NULL);

            decoded_length = cmEmDecode(hVal, pvtNode, buffer, msgLen, &decoded_length);

            if (decoded_length > 0)
            {
                INTPTR fieldId;
                int tmpnodeId;
                int ret;
				char buff[64];

                tmpnodeId = pvtChild(hVal, pvtNode);
                tmpnodeId = pvtChild(hVal, tmpnodeId);

                pvtGet(hVal, tmpnodeId, &fieldId, NULL, NULL, NULL);
                ret = pstGetFieldName(synTreeH245, fieldId, sizeof(buff), buff);
				
				if ((ret != RVERROR) && !strcmp(buff, msgString))
				{
					*lastH245NodeId = nodeId;
					return pvtNode;
				}
				else
				{
					pvtDelete(hVal, pvtNode);
				}
            }
            else
			{
				pvtDelete(hVal, pvtNode);
			}
        }

        /* go to the next encoded tunneled message */
        nodeId = pvtBrother(hVal, nodeId);
    }
	return 0;
}

/* if encodeflag = TRUE: replace the existing nodeId "peernodeId" 
 * 						with encoded tree using rom PVT rootH245node. 
 * 						Free up PVT rootH245node tree
 *    encodeflag = FALSE : Free up the PVT rootH245node tree
 */
void InsertTunnelH245Messages(HPVT			hVal,
						 int				rootH245node,
						 int				peernodeId,
						 char				encodeFlag)
{
	int		iBufLen;
	BYTE	encodeBuffer[MAX_ENCODEDECODE_BUFFRSIZE];
	HPST	synTreeq931= cmGetSynTreeByRootName(UH323Globals()->hApp, "q931");

	if (encodeFlag && (!(cmEmEncode(hVal, rootH245node, encodeBuffer, MAX_ENCODEDECODE_BUFFRSIZE, &iBufLen)<0)))
	{
		int tmpnodeId;
		/* add the modified value tree */
		tmpnodeId = pstGetFieldId(synTreeq931, "h245Control");
		pvtSet(hVal, peernodeId, tmpnodeId,iBufLen, (char *)encodeBuffer);
	}
	pvtDelete(hVal,rootH245node);
}
