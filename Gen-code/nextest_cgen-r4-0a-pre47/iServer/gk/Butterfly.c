#include<stdio.h>
#include <signal.h>
#include<errno.h>
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#undef USE_SLOCKS
#include "callsm.h"
#include "uh323.h"
#include "uh323cb.h"
#include "gis.h"
#include <malloc.h>


#ifdef NOT_USED

static int sd;

int CallingPartyRolloverTimer(tid t);
void * spawn_initserver(void *data)
{
	initserver(data);
	return( (void*) NULL );
}

BFStart()
{
	void *p;
	pthread_t thr;
	pthread_create(&thr,NULL,spawn_initserver,NULL);
	errno = 0;
}

int readNumbers(int sockfd, char *caller, char *called)
{
	char	buf[1024] = {0};
	char 	*p = buf;
	int 	len;
	int	nread;
	char	numeric[] = "0123456789";

		while(!strchr(buf,'\n'))
		{
			nread = read(sockfd,p,80);
			if(nread <0)
			{
				break;
			}
			else {
				p += nread;
			}	
		}

		close (sockfd);
		len = strspn(buf,numeric);
		strncpy(caller,buf,len);
		caller[len] = '\0';

		p = buf+len;	
		len = strcspn(p,numeric);
		p += len;
		len = strspn(p,numeric);
		strncpy(called,p,len);
		called[len] = '\0';
		NETDEBUG(MSCC,NETLOG_DEBUG4,("caller = %s called = %s\n",caller,called));

		return(0);
}

int initserver(void *data)
{
	static char fn[] = "initserver():";
	int on =1;
	int flags;
	struct sockaddr_in servaddr,cliaddr;
	struct sockaddr my_addr;
	int size,newsd;
	char	caller[PHONE_NUM_LEN], called[PHONE_NUM_LEN];
	char	*p;

	if((sd = socket(AF_INET,SOCK_STREAM,0)) <0)
	{
		//ERROR(MSCC,("initserver socket %s\n",strerror(errno)));
		return -1;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(12979);

	
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

	if(bind(sd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		//ERROR(MSCC,("%s bind %s\n",fn,strerror(errno)));
                return -1;
	}

	listen(sd,5);
	
	for(;;)
	{
	    size = sizeof(cliaddr);
		newsd = accept(sd,(struct sockaddr *)&cliaddr,&size);
		if(newsd <0 )
		{
			NETERROR(MSCC,("accept Error %s \n", strerror(errno)));
			return -1;
		}
			
		readNumbers(newsd,caller,called);

		NETDEBUG(MSCC,NETLOG_DEBUG4,("caller = %s called = %s\n",caller,called));

		butterflySetup(caller,called);
	}
	
	return sd;
}

int butterflySetup(char *callingPartyNumber,char *calledPartyNumber)
{
	static char fn[] = "butterflySetup() : ";
	CallHandle 			*callHandle,*callHandle2, callHandleBlk1={0},
						tmpCallHandle1 = { 0 }, tmpCallHandle2 = { 0 };
	ConfHandle 			*confHandle;
	UH323CallAppHandle 	*appHandle; // Call Handle for leg2
	int					len;
	int 				rc;
	char				callID1[CALL_ID_LEN],callID2[CALL_ID_LEN] = {0};
	HCALL				hcall;
	static char 		srcAddress[254], dstAddress[254], ipStr[20];
	static char 		userUserStr[128] = "Nextone iServer Proxy";
	static char 		displayStr[128] = "Nextone iServer Proxy";
	char 				callIdStr[CALL_ID_LEN];
	struct 				itimerval rollovertmr;
	char				*callID;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
	
	if(!(appHandle = uh323CallAllocAppHandle()))
	{
		NETERROR(MSCC,("%s Unable to allocate appHandle\n",fn));
		return -1;
	}


	rc = cmCallNew(UH323Globals()->hApp,(HAPPCALL)appHandle,&hcall);
	if(rc <0)
	{
		NETERROR(MSCC,("%s cmCallNew return error = %d\n",fn,rc));
		uh323CallFreeAppHandle(appHandle);
		return -1;
	}


	if( uh323CallInitAppHandle(hcall, appHandle)< 0)
	{
		uh323CallFreeAppHandle(appHandle);
		cmCallDrop(hcall);
		return -1;
	}	

	callHandle = GisAllocCallHandle();
    /* Use the sourceAddress and destinationAddress information
     * to set up the call
     */


    timedef_cur(&callHandle->callStartTime);
    callHandle->callSource = 0;
 	H323controlState(callHandle) = UH323_sControlIdle;
	callHandle->state = SCC_sIdle;

    /* Create the call Handle */
    /* Initialize the main parameter */
    CallSetCallID(callHandle, appHandle->callID);
    memcpy(callHandle->confID, appHandle->confID, CONF_ID_LEN);
    H323hsCall(callHandle) = hcall;

	H323appHandle(callHandle) = appHandle;
	strcpy(callHandle->rfphonode.phone,callingPartyNumber);
	BIT_SET(callHandle->rfphonode.sflags, ISSET_PHONE);

	// Reserve the port and do a remote resolution also
	if((rc = GkResolvePhone2IP(callHandle,1,1))<=0)
	{
		//CHECK Handle the case when iServer has to do ARQ
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s GkResolvePhone2IP Failed for calling %s rc = %d\n",
			fn,callingPartyNumber,rc));
		uh323CallFreeAppHandle(appHandle);
		cmCallDrop(hcall);
		return -1;
	}
#ifdef _no_raphonode_
	//Check if the call can be rolled over...
	if (BIT_TEST(callHandle->raphonode.sflags, ISSET_IPADDRESS))
	{
		canRollover = 1;
	}
#endif

	sprintf(dstAddress, "TA:%s:%d,TEL:%s,%s",
		liIpToString(htonl(*(unsigned long *)callHandle->rfphonode.ipaddress.uc),ipStr),
		H323rfcallsigport(callHandle),
		callHandle->rfphonode.phone,
		callHandle->rfphonode.phone);


	callHandle->leg = SCC_CallLeg2;
	callHandle->networkEventProcessor = SCC_H323Leg2NetworkEventProcessor;
	callHandle->bridgeEventProcessor = SCC_H323Leg2BridgeEventProcessor;
	callHandle->state = SCC_sWaitOnRemote;
	timedef_cur(&callHandle->callStartTime);
	callHandle->callSource = 1;

	memcpy(&callHandle->phonode,&callHandle->rfphonode,sizeof(PhoNode));
	memset(&callHandle->rfphonode,0,sizeof(PhoNode));

	// Resolve the destination phone number also
	callHandle2 = GisAllocCallHandle();
	strcpy(callHandle2->rfphonode.phone,calledPartyNumber);
	BIT_SET(callHandle2->rfphonode.sflags, ISSET_PHONE);

	// Reserve the port and do a remote resolution also
	if(GkResolvePhone2IP(callHandle2,1,1)<=0)
	{
		//CHECK Handle the case when iServer has to do LRQ or ARQ
		uh323CallFreeAppHandle(appHandle);
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s GkResolvePhone2IP Failed for called %s rc = %d\n",
			fn,calledPartyNumber,rc));
		cmCallDrop(hcall);
		return -1;
	}

	// Does not free callhandle if it is CLIENTHANDLE_H323SETUP
	memcpy(&callHandle->rfphonode,&callHandle2->rfphonode,sizeof(PhoNode));
#ifdef _no_raphonode_
	memcpy(&callHandle->raphonode,&callHandle2->raphonode,sizeof(PhoNode));
#endif

	// copy out rfphonde to phonode for call handle2 - and set rfphonode to 0
	// - this is where it it belongs not rfphonode
	memcpy(&callHandle2->phonode,&callHandle2->rfphonode,sizeof(PhoNode));
	memset(&callHandle2->rfphonode,0,sizeof(PhoNode));
	GisFreeCallHandle(callHandle2);


	memcpy(&tmpCallHandle1, callHandle, sizeof(CallHandle));
	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (CacheInsert(callCache, callHandle)<0 )
	{
		GisFreeCallHandle(callHandle);
		CacheReleaseLocks(callCache);
		uh323CallFreeAppHandle(appHandle);
		NETERROR(MSCC,
			("%s Failed to insert new callHandle in callCache\n",fn));
		goto _error;
	}
	CacheReleaseLocks(callCache);
	callHandle = &tmpCallHandle1;

	GisAddCallToConf(callHandle); // Leg2
	
	NETDEBUG(MSCC,NETLOG_DEBUG2,
		("%s - Inserted CallHandle %s",
			fn, (char*) CallID2String(callHandle->callID,callIdStr)));

	rc = cmCallMake(hcall,
			64000,
			0,
			dstAddress,
			srcAddress,
			displayStr,
			userUserStr,
			sizeof(userUserStr));
	if(rc <0)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,("%s cmCallMake return error = %d\n",
			fn, rc));
		cmCallDrop(hcall);
		uh323CallFreeAppHandle(appHandle);
		return -1;
	}

	// if the call can be rolled over then add a timer for rollover.
	if (IsRolledOver(callHandleBlk1.dialledNumber))
	{
		memset(&rollovertmr, 0, sizeof(struct itimerval));
		rollovertmr.it_value.tv_sec = rolloverTime;

		callID = (char *) malloc(CALL_ID_LEN);
		memset(callID, 0, CALL_ID_LEN);
		memcpy(callID, callHandle->callID, CALL_ID_LEN);

		if (CacheFind(callCache, callHandle->callID, &callHandleBlk1, sizeof(CallHandle)) < 0)
		{
			NETERROR(MSCC,
				("%s Failed to get callHandle \n",fn));
			goto _error;
		}
		callHandleBlk1.rolloverTimer = timerAddToList(&localConfig.timerPrivate, &rollovertmr,
							0,PSOS_TIMER_REL, "RolloverTimer", CallingPartyRolloverTimer, callID);
		if (CacheOverwrite(callCache, &callHandleBlk1, sizeof(CallHandle), 
							callHandleBlk1.callID) < 0)
		{
			NETERROR(MSCC,
				("%s Failed to overwrite callHandle in callCache\n",fn));
			goto _error;
		}
	}
	NETDEBUG(MSCC,NETLOG_DEBUG3,
			("%s Leg2 Sent Setup for %s to %s:\n",fn,srcAddress,dstAddress));
	BillCall(callHandle, CDR_CALLSETUP);
	return 0;

_error:
	cmCallDrop(hcall);
	return -1;
}

int CallingPartyRolloverTimer(tid t)
{
	char fn[] = "CallingPartyRolloverTimer:";

	/* TODO Create a new handle and place a call.
	 * We will have to resolve phone once again for the source also
	 * since the source raphonode is not stored nowhere..only dest
	 * raphonode is stored in callhandle->raphonode where callhandle
	 * is the soure callhandle. */

	timerFreeHandle(t);
	return(0);

}
#endif /* NOT_USED */
