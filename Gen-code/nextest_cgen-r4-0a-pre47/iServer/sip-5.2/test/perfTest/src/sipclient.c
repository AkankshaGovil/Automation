 /**************************************************************************
 ** FUNCTION:
 **			The HSS SIP Stack performance testing application.
 **
 *****************************************************************************
 **
 ** FILENAME:		sipclient.c
 **
 ** DESCRIPTION:	This file contains the perftest application implementation.
 **					This can be configured either as a client or as a server.
 **
 **
 **
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			---------		------
 ** 10-Nov-01		Ramu K							Creation
 ** 30-Mar-02		T.Seshashayi					Added memory pool,
 **					Mohan.R							removed mutexting for
 **													callobject
 *****************************************************************************
 ** 			Copyrights 2002, Hughes Software Systems, Ltd.
 *****************************************************************************/
/*Function Prototypes*/
#ifdef __cplusplus
extern "C" {
#endif

#undef SIP_NO_FILESYS_DEP

#ifdef SIP_SOLARIS
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#endif





#include "sipclient.h"
#include "sipcommon.h"
#include "sipstruct.h"
#include "sipdecode.h"
#include "sipsendmessage.h"
#include "sipinit.h"
#include "sipfree.h"
#include "header.h"
#include "general.h"
#include "request.h"
#include "response.h"
#include "sipthreadmgr.h"
#include "portlayer.h"

#ifdef SIP_TXN_LAYER
#include "txndecodeintrnl.h"
#include "siptxntimer.h"
#include "siptxntest.h"
#include "siptxnstruct.h"
#include "txndecode.h"
#include "txnfree.h"
#else
#include "siptimer.h"
#endif


/* Constants */
const SIP_S8bit test_tool_desc[]="\n"
 "|----------------------------------------------------------------------|\n"
 "| SIP Stack Performance Test Tool                                      |\n"
 "|----------------------------------------------------------------------|\n\n";

#define MAXMESG 1000
#define SIP_MAX_NUMBLK 1000

#ifndef EVENT_Q_SIZE
#define EVENT_Q_SIZE	1024
#endif

SIP_U32bit glbThreadCount;

/* Globals used in this program are declared here*/
SipCall *glbpCallObjectArray;
synch_id_t timer_start_lock;

#ifdef SIP_SOLARIS
thread_cond_t timer_start_cond_var;
SIP_S32bit dServer_fd;
SIP_Pvoid msgProcThread(SIP_Pvoid pArg);
SIP_S8bit sip_getchar(void);
void signal_handle(SIP_S32bit);
void flush(void);
#endif





SipClientParams dClient_params;
SIP_S8bit *pThisHostIP;
SipTestStatistics *glbpTest_statistics, dCumulative_stats;
SIP_S32bit dResponse_delay;
SIP_S8bit dOur_host[]="127.0.0.1";
SIP_S32bit dServer_port;
SIP_S32bit glbClient_mode;
SIP_S32bit nMsgProcThreads;
TThreadMgr msgProcThreadMgr;
SipEvent *pEvent= SIP_NULL;

#ifdef SIP_TXN_LAYER

#ifdef SIP_SOLARIS
	thread_id_t dTimerThread, dDisplayThread;
#endif

#else

#ifdef SIP_SOLARIS
	thread_id_t timerID;
#endif

#endif


#ifdef SIP_TXN_LAYER


#ifdef SIP_SOLARIS
SIP_Pvoid displayThread(SIP_Pvoid);
#endif




#endif



/******************************************************************************
 ** FUNCTION: 		main
 **
 ** DESCRIPTION: 	This is the main function where the application begins
 **
 ******************************************************************************/
int main(void)
{
	SIP_S32bit dOption;
	struct sockaddr_in	serv_addr;



#ifdef SIP_SOLARIS
	thread_attr_t tattr;
	signal(SIGINT,(fnSigType)(signal_handle));
	signal(SIGBUS,(fnSigType)(signal_handle));
#endif




	/* Begin with a description */
	printf("%s", test_tool_desc);
	fflush(stdout);
	/* Initialize the stack */
	sip_initStack();

	printf("Port on which to listen for messages: ");
	fflush(stdout);
	scanf("%d", &dServer_port);
	fflush(stdin);

#ifdef SIP_SOLARIS
	printf("Number of message processing threads: ");
	fflush(stdout);
	scanf("%d", &nMsgProcThreads);
	fflush(stdin);
#endif


	glbThreadCount = nMsgProcThreads;
	printf("thread Count: %d\n",glbThreadCount);
	fflush(stdout);

	printf("\nSelect mode (c=client, s=server, default=c) :");
	fflush(stdout);

	dOption = sip_getchar();

	if(dOption != 's')
	{
		SIP_S8bit client_dest_host[30];
		SIP_S32bit port;
		/* Client parameters */
		glbClient_mode = 1;
		/* Transport parameters */
		printf("\nDestination address (address where another instance"
			" is runnning in the server mode): ");
		fflush(stdout);
		scanf("%s", client_dest_host);
		strncpy(dClient_params.client_dest_addr.dIpv4, client_dest_host, 16);
		/* Fill in the client destination addredd structure */
		printf("\nDestination port (port on which the server mode program"
			" is listening: ");
		fflush(stdout);
		scanf("%d", &port);
		(dClient_params.client_dest_addr.dPort) = port;
		/* Rate related parameters */

#ifdef SIP_SOLARIS
		printf("\nNumber of Active calls: ");
#endif

		fflush(stdout);
		scanf("%d", &(dClient_params.nCalls));


		dClient_params.calls_per_burst = dClient_params.nCalls;
		dClient_params.delay_bet_bursts = 0;
		printf("\nMaximum number of calls in this run (0 for infinite): ");
		fflush(stdout);
		scanf("%d", &(dClient_params.nMaxCalls));
                printf("\n");
		fflush(stdout);
	}
	else
	{
		glbClient_mode = 0;
		printf("\nDelay before responding to a request (microsecs):");
		fflush(stdout);
		scanf("%d", &dResponse_delay);
	}


	/* Spawn Timer thread */
	fast_init_synch(&timer_start_lock);

#ifdef SIP_SOLARIS
	pthread_cond_init(&timer_start_cond_var, NULL);
#endif



	fast_lock_synch(0,&timer_start_lock,0);

#ifdef SIP_SOLARIS
	pthread_attr_init(&tattr);
	if ( pthread_attr_setscope(&tattr,PTHREAD_SCOPE_SYSTEM) != 0 )
	{
		printf("Unable to set the scope of thread to PTHREAD_SCOPE_SYSTEM.\n");
		fflush(stdout);
		exit(1);
	}
#endif

#ifdef SIP_TXN_LAYER
	sip_initTimerWheel(0,0);

#ifdef SIP_SOLARIS
	pthread_create(&dTimerThread,&tattr,(fnPthreadType)sip_timerThread,NULL);
	pthread_create(&dDisplayThread,&tattr,(fnPthreadType)displayThread,NULL);
#endif



#ifdef SIP_TXN_LAYER
	sip_initHashTbl((CompareKeyFuncPtr)sip_txn_compareTxnKeys);
#endif
	__sip_sleep(10000);
#else

#ifdef SIP_SOLARIS
	pthread_create(&timerID, &tattr, timerThread, &dClient_params);
#endif


	/* Wait for timer thread to start operation */
#ifdef SIP_SOLARIS
	pthread_cond_wait(&timer_start_cond_var, &timer_start_lock);
#endif




#endif

#ifdef SIP_SOLARIS
	if (threadMgrInit(&msgProcThreadMgr, nMsgProcThreads, EVENT_Q_SIZE)
	    == SipFail)
	{
		printf("Could not init Message processing thread manager\n");
		fflush(stdout);
		exit(1);
	}

	if (threadMgrSpawnThreads(&msgProcThreadMgr, &tattr, NULL, \
			(SIP_Pvoid (*)(SIP_Pvoid))msgProcThread)
	    == SipFail)
	{
		printf("Could not spawn Message processing threads\n");
		fflush(stdout);
		exit(1);
	}
	pthread_attr_destroy(&tattr);
#endif

	/* Message reception is done in the main thread */

	bzero((SIP_S8bit *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(dServer_port);

	if((dServer_fd = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		printf("Server could not open dgram socket\n");
		fflush(stdout);
		close(dServer_fd);
		exit(0);
	}

	if (bind(dServer_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Server could not bind to listen port\n");
		close(dServer_fd);
		exit(0);
	}
	/* Serve bound to port - now ready to receive messages */

	/* If operating in client mode, spawn thread that sends messages */
	init_statistics();
	if(glbClient_mode)
	{
		/* Initiate calls */
		display_statistics(1);
		SetupCalls(&dClient_params);
		printf("SetUp Call Completed\n");
		fflush(stdout);
	}

	/* Allocate buffer for reading the network message (Only In VxWorks)*/

#ifdef SIP_SOLARIS
	pEvent = (SipEvent*) fast_memget(0,sizeof(SipEvent),SIP_NULL);
	pEvent->message = (SIP_S8bit *) fast_memget(0, \
		sizeof(SIP_S8bit)*MAXMESG,NULL);
#endif

	/* This is our network receive loop */
	while(1)
	{
		SIP_S32bit dClilen;
		SIP_S8bit dThreadNumber;
		SIP_S32bit dBytes_read=0;
		SIP_U32bit dCallId=0;

#ifdef SIP_SOLARIS
		/* Allocate buffer for reading the network message */
		pEvent = (SipEvent*) fast_memget(0,sizeof(SipEvent),SIP_NULL);
		pEvent->message = (SIP_S8bit *) fast_memget(0, \
			sizeof(SIP_S8bit)*MAXMESG,NULL);
#endif

		dClilen = sizeof(pEvent->cli_addr);
		dBytes_read = recvfrom(dServer_fd, pEvent->message, MAXMESG, 0,\
			(struct sockaddr *) &pEvent->cli_addr, &dClilen);
		if(dBytes_read<=0)
		{
			fast_memfree(0,pEvent->message,NULL);
			fast_memfree(0,pEvent,NULL);
			printf((SIP_S8bit*)"Error in receiving "
					"message from UDP socket\n");
			fflush(stdout);
			continue;
		}
		pEvent->message[dBytes_read] = '\0';


#ifdef SIP_SOLARIS
		sip_getCallIdFromBuffer(pEvent->message, &dCallId);
		sip_getThreadIdFromCallId(dCallId, &dThreadNumber);
		threadMgrPostEvent(&msgProcThreadMgr,dThreadNumber, pEvent);
#endif
	}
}

/******************************************************************************
 ** FUNCTION: 		SetupCalls
 **
 ** DESCRIPTION: 	This is function where the initial calls are Setup.
 **
 ******************************************************************************/
void SetupCalls(SipClientParams *pClientParams)
{
	SIP_S32bit dI,dK;
	SIP_S32bit dTemp;
	SipError err;

	/* Allocate space for call state machines */
	glbpCallObjectArray = (SipCall *) fast_memget(0,\
		sizeof(SipCall)*(pClientParams->nCalls),NULL);

	for (dI=0; dI<pClientParams->nCalls; ++dI)
	{
		fast_init_synch(&glbpCallObjectArray[dI].callLock);
		glbpCallObjectArray[dI].seqNum = 0;
		sprintf(glbpCallObjectArray[dI].callId, "%d",dI+1);
#ifdef SIP_TXN_LAYER
		glbpCallObjectArray[dI].pInvTxnKey=NULL;
		glbpCallObjectArray[dI].pByeTxnKey=NULL;
#endif
	}

	for(dI=0;
		dI<(pClientParams->nCalls/pClientParams->calls_per_burst);dI++)
	{
		/* Start the INVITE transactions and update the call states */
		for (dK=0; dK<pClientParams->calls_per_burst;dK++)
		{
			dTemp = dI * pClientParams->calls_per_burst + dK;
 			glbpCallObjectArray[dTemp].state = SipInviteSent;
			glbpTest_statistics[0].calls_attempted++;
#ifndef SIP_TXN_LAYER
			sip_sendRequest("INVITE", &(pClientParams->client_dest_addr),\
				glbpCallObjectArray[dTemp].callId, &err);
#else
			sip_sendRequest("INVITE", &(pClientParams->client_dest_addr),\
				glbpCallObjectArray[dTemp].callId, \
				(SipTxnKey**)&(glbpCallObjectArray[dTemp].pInvTxnKey),&err);
#endif
		}
		sip_sleep(pClientParams->delay_bet_bursts);
	}
}

/******************************************************************************
 ** FUNCTION: 		msgProcThread
 **
 ** DESCRIPTION: 	This function handles all the messages received from the
 **					Network.
 **
 ******************************************************************************/
#ifdef SIP_SOLARIS
SIP_Pvoid msgProcThread(SIP_Pvoid pArg)
#endif
{
	SIP_S8bit *pNext_mesg;
	SipMessage *pSipMessage;
	SIP_U32bit  dBytes_read;
	SipOptions dDecode_options;
	SipError err;

#ifdef SIP_SOLARIS
	SipEvent *pEvent;
#endif

	SIP_U8bit dThreadIndex;

#ifdef SIP_TXN_LAYER
	en_SipTxnDecodeResult dResult;
	SipTxnContext txncontext;
	SIP_S32bit glbtransactiontype =0;
	SipOptions opt;
	SipTxnKey *pTxnKey = SIP_NULL;
	SipEventContext context;
#endif

	dDecode_options.dOption = 0;

#ifdef SIP_SOLARIS
	dThreadIndex = *((SIP_U8bit *)pArg);
	fast_memfree(0,pArg,NULL);
#endif

#ifdef SIP_TXN_LAYER
	context.pList=SIP_NULL;
	opt.dOption  = 0;
	context.pData = SIP_NULL;
	context.pDirectBuffer = SIP_NULL;
	txncontext.pEventContext = &context;
	txncontext.dTxnType = (en_SipTxnType)glbtransactiontype;
	txncontext.txnOption.dOption = opt;
	txncontext.txnOption.dTimerOption= 0;
#endif


#ifdef SIP_SOLARIS
	printf("Thread ID: %d\n",dThreadIndex);
	fflush(stdout);
	while (1)
	{
		if (threadMgrGetEventFromMaster(&msgProcThreadMgr, dThreadIndex, \
								(SIP_Pvoid*) &pEvent)
		    == SipFail)
		{
			printf("Get Event from master failed; Terminating the thread\n");
			fflush(stdout);
			return NULL;
		}
#endif

		dBytes_read = strlen(pEvent->message);
		pSipMessage = SIP_NULL;

		/* Ask stack to decode the received message */
#ifndef SIP_TXN_LAYER
		if(sip_decodeMessage(pEvent->message, &pSipMessage, &dDecode_options,\
			dBytes_read, &pNext_mesg, SIP_NULL, &err)!=SipSuccess)
#else
		if(sip_txn_decodeMessage(pEvent->message, &pSipMessage, &txncontext,\
			dBytes_read, &pNext_mesg,&dResult,&pTxnKey, &err)!=SipSuccess)
#endif
		{
			/* Failed to decode message. Get back to listening for more */
#ifdef SIP_SOLARIS
			fast_memfree(0,pEvent->message,NULL);
			fast_memfree(0,pEvent,NULL);
			continue;
#endif
		}

#ifdef SIP_TXN_LAYER
		if((dResult==SipTxnIgnorable) || (dResult==SipTxnStrayMessage)\
			|| (dResult==SipTxnQueued))
		{
				sip_freeSipMessage(pSipMessage);
#ifdef SIP_SOLARIS
				fast_memfree(0,pEvent->message,SIP_NULL);
				fast_memfree(0,pEvent,SIP_NULL);
				continue;
#endif
		}
#endif
		if(glbClient_mode)
		{
			/*
			 * In the client mode we've got to update the call state
			 * machines and signal the client thread about transaction
			 * completion. Its all done in client_message_handle
			 */
#ifndef SIP_TXN_LAYER
			client_message_handle(pSipMessage, dThreadIndex);
#else
			client_message_handle(pSipMessage,pTxnKey,dThreadIndex);
#endif
		}
		else
		{
			/*
			 * The server's job is simpler. No state maintenance. Just
			 * respond with 200 OK responses to INVITEs and BYEs.
			 */
			SipTranspAddr dClient_addr;
			strcpy(dClient_addr.dIpv4,inet_ntoa(pEvent->cli_addr.sin_addr));
#ifndef SIP_TXN_LAYER
			server_message_handle(pSipMessage, &dClient_addr, dThreadIndex);
#else
			server_message_handle(pSipMessage, &dClient_addr,pTxnKey,\
				dThreadIndex);
#endif
		}
		sip_freeSipMessage(pSipMessage);

#ifdef SIP_SOLARIS
		fast_memfree(0,pEvent->message,NULL);
		fast_memfree(0,pEvent,NULL);
	}
#endif
}

/******************************************************************************
 ** FUNCTION: 		client_message_handle
 **
 ** DESCRIPTION: 	This function handles all the messages received from the
 **					Network in the client mode.
 **
 ******************************************************************************/
#ifndef SIP_TXN_LAYER
void client_message_handle
(SipMessage *pMessage, SIP_U8bit dThreadIndex)
#else
void client_message_handle
(SipMessage *pMessage, SipTxnKey* pTxnKey,SIP_U8bit dThreadIndex)
#endif
{
	SipError err;
	en_SipMessageType dMessage_type;
#ifdef SIP_TXN_LAYER
	SIP_Pvoid pDummy;
	SipTxnContext empty_txnContext ;
	pDummy = pTxnKey;
#endif

	sip_getMessageType(pMessage, &dMessage_type, &err);
	/* chack if response message */
	if(dMessage_type == SipMessageResponse)
	{
		SipHeader header;
		SIP_S8bit *pMethod;
		SIP_S8bit *pCallid;
		SIP_U32bit dCallindex;

		sip_getHeader(pMessage, SipHdrTypeCseq, &header, &err);
		sip_getMethodFromCseqHdr(&header, &pMethod, &err);
		sip_freeSipHeader(&header);

		/* Get call-id to fetch CSM */
		sip_getHeader(pMessage, SipHdrTypeCallId, &header, &err);
		sip_getValueFromCallIdHdr(&header, &pCallid, &err);
		sip_freeSipHeader(&header);
		dCallindex = atoi(pCallid)-1;
		fast_lock_synch(0,&glbpCallObjectArray[dCallindex].callLock,0);


		if(strcmp(pMethod, "INVITE")==0)
		{
			/* If message is an OK to INVITE */
			/* get call-object and and check if call-state is invite-sent */
			/* If yes, change call state to established */
			if(glbpCallObjectArray[dCallindex].state == SipInviteSent)
			{
				glbpCallObjectArray[dCallindex].state = SipInProgress;
				glbpTest_statistics[dThreadIndex].successful_setups++;
			}

			/* Call-state indicates call is established. Probably
			   ACK didint reach. We'll send the ACK again */
			/* ACK to be sent. Need to fill in the destination to which
				requests are sent by the client thread */
#ifndef SIP_TXN_LAYER
			if ( sendResponse(pMessage, 0, "", &(dClient_params.client_dest_addr),\
					&err) != SipSuccess )
#else
			if ( sendResponse(pMessage, 0, "", &(dClient_params.client_dest_addr),\
					NULL,&err) != SipSuccess )
#endif
			{
				return;
			}

			if(glbpCallObjectArray[dCallindex].state == SipInProgress)
			{
				glbpCallObjectArray[dCallindex].state = SipByeSent;
				glbpTest_statistics[dThreadIndex].teardowns_attempted++;
#ifndef SIP_TXN_LAYER
				if ( sip_sendRequest("BYE", &(dClient_params.client_dest_addr),\
                		glbpCallObjectArray[dCallindex].callId, &err) != SipSuccess )
                		return;
#else
			    	memset(&empty_txnContext,0,sizeof(SipTxnContext)) ;
				sip_txn_abortTxn( \
					(SipTxnKey*)(glbpCallObjectArray[dCallindex].pInvTxnKey),\
					&empty_txnContext,&err);
				sip_freeSipTxnKey( \
					(SipTxnKey*)(glbpCallObjectArray[dCallindex].pInvTxnKey));
				glbpCallObjectArray[dCallindex].pInvTxnKey=NULL;
				if ( sip_sendRequest("BYE", &(dClient_params.client_dest_addr),\
                		glbpCallObjectArray[dCallindex].callId, \
                		(SipTxnKey**)&(glbpCallObjectArray[dCallindex].pByeTxnKey),\
	            	   	&err) != SipSuccess )
	            	   	return;
#endif
			}
		}
		else if(strcmp(pMethod, "BYE")==0)
		{
			/* Message is OK to bye */
			if(glbpCallObjectArray[dCallindex].state == SipByeSent)
			{
				/* Call completed; Initiate it again */
				glbpTest_statistics[dThreadIndex].successful_teardowns++;
 				glbpCallObjectArray[dCallindex].state = SipInviteSent;
				glbpTest_statistics[dThreadIndex].calls_attempted++;
#ifndef SIP_TXN_LAYER
				sip_sendRequest("INVITE", &(dClient_params.client_dest_addr),\
							glbpCallObjectArray[dCallindex].callId, &err);
#else
			    memset(&empty_txnContext,0,sizeof(SipTxnContext)) ;
				sip_txn_abortTxn( \
					(SipTxnKey*)(glbpCallObjectArray[dCallindex].pByeTxnKey),\
					&empty_txnContext,&err);
				sip_freeSipTxnKey( \
					(SipTxnKey*)(glbpCallObjectArray[dCallindex].pByeTxnKey));
				glbpCallObjectArray[dCallindex].pByeTxnKey=NULL;
				sip_sendRequest("INVITE", &(dClient_params.client_dest_addr),\
					glbpCallObjectArray[dCallindex].callId, \
					(SipTxnKey**)&(glbpCallObjectArray[dCallindex].pInvTxnKey),\
					&err);
#endif
			}
		}
		fast_unlock_synch(0,&glbpCallObjectArray[dCallindex].callLock);
	}
	return;
}

/******************************************************************************
 ** FUNCTION: 		server_message_handle
 **
 ** DESCRIPTION: 	This function handles all the messages received from the
 **					Network in the server mode.
 **
 ******************************************************************************/
#ifndef SIP_TXN_LAYER
void server_message_handle(SipMessage *pMessage, SipTranspAddr *pClient_addr,
	SIP_U8bit dThreadIndex)
#else
void server_message_handle(SipMessage *pMessage, SipTranspAddr *pClient_addr,
	SipTxnKey* pTxnKey,SIP_U8bit dThreadIndex)
#endif
{
	static SIP_S32bit dFirst_call = 1;
	SipError err;
#ifdef SIP_TXN_LAYER
	SipHeader header;
	SIP_S8bit *pCallid;
	SIP_S32bit dCallindex;
	SipTxnContext empty_txnContext ;
#endif
	/* If INVITE or BYE, form a 200 OK and dispatch to sender */
	en_SipMessageType dMessage_type;

	if (dFirst_call)
	{
		dFirst_call = 0;
		display_statistics(1);
	}

	sip_getMessageType(pMessage, &dMessage_type, &err);

	/* Get call-id to fetch CSM */
#ifdef SIP_TXN_LAYER
	sip_getHeader(pMessage, SipHdrTypeCallId, &header, &err);
	sip_getValueFromCallIdHdr(&header, &pCallid, &err);
	sip_freeSipHeader(&header);
	dCallindex = atoi(pCallid)-1;
#endif

	if(dMessage_type == SipMessageRequest)
	{
		SipReqLine *request_line;
		SIP_S8bit *pMethod;

		sip_getReqLine(pMessage, &request_line, &err);
		sip_getMethodFromReqLine(request_line, &pMethod, &err);
		sip_freeSipReqLine(request_line);

		if (strcmp(pMethod, "INVITE")==0)
		{
			if(dResponse_delay>0)
				sip_sleep(dResponse_delay);
#ifndef SIP_TXN_LAYER
			sendResponse(pMessage, 200, "OK", pClient_addr, &err);
			glbpTest_statistics[dThreadIndex].calls_attempted++;
#else
			if ( sendResponse(pMessage, 200, "OK", pClient_addr,NULL,&err) == \
				SipSuccess )
			{
				glbpTest_statistics[dThreadIndex].calls_attempted++;
			}
			else
			{
				printf("sendResponse: Failed to send 200 OK to Invite\n");
				fflush(stdout);
				return;
			}
		 	memset(&empty_txnContext,0,sizeof(SipTxnContext)) ;
			sip_txn_abortTxn(pTxnKey,&empty_txnContext,&err);
			sip_freeSipTxnKey(pTxnKey);
#endif
		}

		if (strcmp(pMethod, "BYE")==0)
		{
			if(dResponse_delay>0)
				sip_sleep(dResponse_delay);
#ifndef SIP_TXN_LAYER
			sendResponse(pMessage, 200, "OK", pClient_addr, &err);
			glbpTest_statistics[dThreadIndex].teardowns_attempted++;
#else
			if ( sendResponse(pMessage, 200, "OK", pClient_addr,NULL,&err) == \
				SipSuccess )
			{
				glbpTest_statistics[dThreadIndex].teardowns_attempted++;
			}
			else
			{
				printf("sendResponse: Failed to send 200 OK to Bye\n");
				fflush(stdout);
				return;
			}
		 	memset(&empty_txnContext,0,sizeof(SipTxnContext)) ;
			sip_txn_abortTxn(pTxnKey,&empty_txnContext,&err);
			sip_freeSipTxnKey(pTxnKey);
#endif
		}
	}
	return;
}

/******************************************************************************
 ** FUNCTION: 		sip_copyMandatoryHeaders
 **
 ** DESCRIPTION: 	Function to copy the From, To, Call-Id and CSeq headers
 **					from one message into the other.
 **
 **
 ******************************************************************************/
void sip_copyMandatoryHeaders(SipMessage *pOriginal, SipMessage *pResponse)
{
	SipHeader header;
	SipError err;
#ifdef SIP_TXN_LAYER
	SIP_S8bit *pTag;
#endif

	sip_getHeader(pOriginal, SipHdrTypeFrom, &header, &err);
	sip_setHeader(pResponse, &header, &err);
	sip_freeSipHeader(&header);

	sip_getHeader(pOriginal, SipHdrTypeTo, &header, &err);
#ifdef SIP_TXN_LAYER
		pTag = sip_strdup("ajkhdd89347",0);
		sip_insertTagAtIndexInToHdr(&header,pTag,0,&err);
#endif
	sip_setHeader(pResponse, &header, &err);
	sip_freeSipHeader(&header);

	sip_getHeader(pOriginal, SipHdrTypeCallId, &header, &err);
	sip_setHeader(pResponse, &header, &err);
	sip_freeSipHeader(&header);
	sip_getHeader(pOriginal, SipHdrTypeCseq, &header, &err);
	sip_setHeader(pResponse, &header, &err);
	sip_freeSipHeader(&header);
}

/******************************************************************************
 ** FUNCTION: 		sip_copyViaHeaders
 **
 ** DESCRIPTION: 	Function to copy the via header from the original message
 **					into the response.
 **
 **
 ******************************************************************************/
void sip_copyViaHeaders(SipMessage *pOriginal, SipMessage *pResponse)
{
	SipHeader header;
	SipError err;
	SIP_U32bit dI, dVia_count;

	sip_getHeaderCount(pOriginal, SipHdrTypeVia, &dVia_count, &err);
	for(dI=0;dI<dVia_count;dI++)
	{
		sip_getHeaderAtIndex(pOriginal, SipHdrTypeVia, &header, dI, &err);
		sip_insertHeaderAtIndex(pResponse, &header, dI, &err);
		sip_freeSipHeader(&header);
	}
}

/******************************************************************************
 ** FUNCTION: 		sendResponse
 **
 ** DESCRIPTION: 	Function used to send simple responses to requests.
 **					Can also be used to send ACKs to responses
 **
 **
 ******************************************************************************/
#ifndef SIP_TXN_LAYER
SipBool sendResponse(SipMessage *pMessage, SIP_S32bit dCode, \
	const SIP_S8bit *pReason,SipTranspAddr *pSendaddr, SipError *pErr)
#else
SipBool sendResponse(SipMessage *pMessage, SIP_S32bit dCode, \
	const SIP_S8bit *pReason,SipTranspAddr *pSendaddr,\
	SipTxnKey** ppTxnKey,SipError *pErr)
#endif
{
	SipMessage *pSipmesg;
	en_SipMessageType dType;
	SipHeader header;
	SipEventContext *pContext;
	SipCallContext *pCall_context;
	SIP_S8bit contextstr[100];
	SIP_S8bit *pCallid;
	SipOptions dOptions;
	SipBool dRes;
#ifdef SIP_TXN_LAYER
	SipTxnContext *pTxncontext;
	SipTxnKey *pTxnKey;
	en_SipTxnBypass dbypass;
	SipTranspAddr *pTempAddr = SIP_NULL;
#endif

	/* Check message type */
	sip_getMessageType(pMessage, &dType, pErr);
	if(dType == SipMessageRequest)
	{
		SipStatusLine *pStatline;
		SIP_S8bit *pSent_by;
		/* Need to form response to this one */
		sip_initSipMessage(&pSipmesg,SipMessageResponse,pErr);
		/* Set the status line in the response */
		sip_initSipStatusLine(&pStatline,pErr);
		sip_setReasonInStatusLine(pStatline,sip_strdup(pReason,0),pErr);
		sip_setVersionInStatusLine(pStatline,sip_strdup("SIP/2.0",0),pErr);
		sip_setStatusCodeNumInStatusLine(pStatline,(SIP_U16bit)dCode,pErr);
		sip_setStatusLineInSipRespMsg (pSipmesg, pStatline, pErr);
		sip_freeSipStatusLine(pStatline);
		/* Copy From, To, Call-ID and CSeq headers */
		sip_copyMandatoryHeaders(pMessage, pSipmesg);
		/* Copy all Via headers */
		sip_copyViaHeaders(pMessage, pSipmesg);
		/* TranspAddr contains host from which the request came. Need to
		   find port from the Via header */
		sip_getHeaderAtIndex(pMessage, SipHdrTypeVia, &header, 0, pErr);
		sip_getSentByFromViaHdr(&header, &pSent_by, pErr);
		sip_freeSipHeader(&header);
		pSendaddr->dPort = sip_getPortFromSentBy(pSent_by);
		pContext = (SipEventContext* ) fast_memget(0,sizeof(SipEventContext),NULL);
		sprintf(contextstr,"Response : %d ",dCode);
		pCall_context = (SipCallContext *) fast_memget(0,\
			sizeof(SipCallContext),NULL);
		pContext->pData = pCall_context;
		strcpy(pCall_context->contextStr, contextstr);
		sip_getHeader(pSipmesg, SipHdrTypeCallId, &header, pErr);
		sip_getValueFromCallIdHdr(&header, &pCallid, pErr);
		strcpy(pCall_context->callId, pCallid);
		sip_freeSipHeader(&header);
		dOptions.dOption = 0;
	}
	else
	{
		SipReqLine *pReqline;
		SipAddrSpec *pAddr_spec;
		SipHeader *pHeader;
		SIP_S8bit sent_by[100];

		/* ACK to be formed from response message */
		sip_initSipMessage(&pSipmesg,SipMessageRequest,pErr);
		/* Set the status line in the response */
		sip_initSipReqLine(&pReqline,pErr);
		sip_setMethodInReqLine(pReqline, sip_strdup("ACK",0), pErr);
		sip_setVersionInReqLine(pReqline, sip_strdup("SIP/2.0",0), pErr);

		sip_initSipAddrSpec(&pAddr_spec, SipAddrReqUri, pErr);
		sip_setUriInAddrSpec(pAddr_spec, sip_strdup("sip:mydomain.com",0), pErr);
		sip_setAddrSpecInReqLine(pReqline, pAddr_spec, pErr);
		sip_freeSipAddrSpec(pAddr_spec);
		sip_setReqLine(pSipmesg, pReqline, pErr);
		sip_freeSipReqLine(pReqline);
		/* Copy From, To, Call-ID and CSeq headers */
		sip_copyMandatoryHeaders(pMessage, pSipmesg);

		/* Convenience functions for inserting headers from strings into
	   	the message*/
		sip_initSipHeader(&pHeader, SipHdrTypeVia, pErr);
		sip_setSentProtocolInViaHdr(pHeader, sip_strdup("SIP/2.0/UDP",0), pErr);
		strcpy(sent_by, dOur_host);
		strcat(sent_by,":");
		sprintf(sent_by+(sizeof(SIP_S8bit)*strlen(sent_by)), "%d", dServer_port);
		sip_setSentByInViaHdr(pHeader, sip_strdup(sent_by,0), pErr);
		sip_insertHeaderAtIndex(pSipmesg, pHeader, 0, pErr);
		sip_freeSipHeader(pHeader);
		fast_memfree(0,pHeader,SIP_NULL);

		sip_getHeader(pSipmesg, SipHdrTypeCseq, &header, pErr);
		sip_setMethodInCseqHdr(&header, sip_strdup("ACK",0), pErr);
		sip_freeSipHeader(&header);
		pContext = SIP_NULL;
		dOptions.dOption = 0;
	}


#ifdef SIP_TXN_LAYER
	sip_txn_initSipTxnContext(&pTxncontext,pErr);
	dbypass = SipTxnNoByPass;

	pTxncontext->pEventContext= pContext;
	pTxncontext->txnOption.dOption = dOptions;
	pTxncontext->dTxnType = SipUATypeTxn;
	pTxncontext->txnOption.dTimerOption = 0;
	pTxncontext->txnOption.dTimeoutCbkOption = SIP_ISSUE_CBKFOR_ALLTIMERS;
#endif

#ifdef SIP_TXN_LAYER
	pTempAddr = (SipTranspAddr*) malloc(sizeof(SipTranspAddr));
	*pTempAddr = *pSendaddr;
	dRes = sip_txn_sendMessage(pSipmesg,pTempAddr,SIP_UDP,\
		pTxncontext, dbypass,&pTxnKey,pErr);
	if ( ppTxnKey != NULL )
	{
		if ( pTxnKey != NULL )
			*ppTxnKey = pTxnKey;
	}
	fast_memfree(0,pTxncontext,SIP_NULL);
#else
   	dRes = sip_sendMessage(pSipmesg,&dOptions,pSendaddr,SIP_UDP,pContext,pErr);
#endif
	if ( dRes != SipSuccess )
	{
		switch(*pErr)
		{
			case E_TXN_NO_EXIST:
				printf("** ERROR : The Txn for this message" \
					"doesn't exist**\n");
				fflush(stdout);
				break;
			case E_TXN_EXISTS:
				printf("** ERROR : The Txn for this message" \
					"already exists**\n");
				fflush(stdout);
				break;
			case E_TXN_INV_STATE:
				printf("** ERROR : This message leads to"\
					"an invalid transaction state**\n");
				fflush(stdout);
				break;
			case E_TXN_INV_MSG:
				printf("** ERROR : This is an invalid message"\
					"for received for the Txn**\n");
				fflush(stdout);
			break;
			default:
				printf("Error No: %d\n",*pErr);
				fflush(stdout);
				break;
		}
		printf("sendResponse:** ERROR : Send Message Failed**\n");
		fflush(stdout);
		sip_freeSipMessage(pSipmesg);
		return SipFail;
	}

	sip_freeSipMessage(pSipmesg);
	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: 		sip_getPortFromSentBy
 **
 ** DESCRIPTION: 	Function used to get the port number from the
 **					host string
 **
 **
 ******************************************************************************/
SIP_S32bit sip_getPortFromSentBy(SIP_S8bit *pHost)
{
	SIP_S8bit *pPort_index;

	pPort_index = index(pHost, ':');
	if(pPort_index == NULL)
		return 5060;
	else
		return atoi(pPort_index+1);
}

/******************************************************************************
 ** FUNCTION: 		sip_sendRequest
 **
 ** DESCRIPTION: 	Function used to send requests.
 **
 **
 **
 ******************************************************************************/
#ifndef SIP_TXN_LAYER
SipBool sip_sendRequest(const SIP_S8bit *pMethod,SipTranspAddr *pSendaddr,\
	SIP_S8bit *pCallID, SipError *pErr)
#else
SipBool sip_sendRequest(const SIP_S8bit *pMethod,SipTranspAddr *pSendaddr,\
	SIP_S8bit *pCallID,SipTxnKey** ppTxnKey, SipError *pErr)
#endif
{
	SipMessage *pSipmesg;
	SipReqLine *pReqline;
	SipHeader *pHeader;
	SipHeader tmphdr;
	SipAddrSpec *pAddrspec;
	SipUrl *pSipurl;
	SipBool dRes;
	SipEventContext *pContext;
	SipCallContext *pCall_context;
	SIP_S8bit contextstr[100];
	SIP_S8bit sent_by[100];
	SIP_S8bit *pCallid;

#ifdef SIP_TXN_LAYER
	SipTxnContext *pTxncontext;
	SipTxnKey *pTxnKey;
	en_SipTxnBypass dbypass;
	SipTranspAddr *pTempAddr = SIP_NULL;
#endif
	SipOptions dOptions;

	sip_initSipMessage(&pSipmesg,SipMessageRequest,pErr);
	/* Set request line */
	sip_initSipReqLine(&pReqline,pErr);
	sip_setMethodInReqLine(pReqline,sip_strdup(pMethod,0),pErr);
	sip_setVersionInReqLine(pReqline,sip_strdup("SIP/2.0",0),pErr);
	sip_initSipAddrSpec(&pAddrspec,SipAddrSipUri,pErr);
	sip_initSipUrl(&pSipurl,pErr);
	sip_setHostInUrl(pSipurl,sip_strdup("mydomain.com",0),pErr);
	sip_setUrlInAddrSpec(pAddrspec,pSipurl,pErr);
	sip_setAddrSpecInReqLine(pReqline,pAddrspec,pErr);
	sip_freeSipUrl(pSipurl);
	sip_freeSipAddrSpec(pAddrspec);

	sip_setReqLineInSipReqMsg (pSipmesg, pReqline, pErr);
	sip_freeSipReqLine(pReqline);

	/* Set From header */
	sip_initSipHeader(&pHeader,SipHdrTypeFrom,pErr);
	sip_initSipAddrSpec(&pAddrspec,SipAddrSipUri,pErr);
	sip_initSipUrl(&pSipurl,pErr);
	sip_setHostInUrl(pSipurl,sip_strdup("mydomain.com",0),pErr);
	sip_setUrlInAddrSpec(pAddrspec,pSipurl,pErr);
	sip_setAddrSpecInFromHdr(pHeader,pAddrspec,pErr);
	sip_freeSipUrl(pSipurl);
	sip_freeSipAddrSpec(pAddrspec);

	sip_setHeader(pSipmesg, pHeader, pErr);
	sip_freeSipHeader(pHeader);
	fast_memfree(0,pHeader,SIP_NULL);

	/* Set To header */
	sip_initSipHeader(&pHeader,SipHdrTypeTo,pErr);
	sip_initSipAddrSpec(&pAddrspec,SipAddrSipUri,pErr);
	sip_initSipUrl(&pSipurl,pErr);
	sip_setHostInUrl(pSipurl,sip_strdup("yourdomain.com",0),pErr);
	sip_setUrlInAddrSpec(pAddrspec,pSipurl,pErr);
	sip_setAddrSpecInToHdr(pHeader,pAddrspec,pErr);
	sip_freeSipUrl(pSipurl);
	sip_freeSipAddrSpec(pAddrspec);

	sip_setHeader(pSipmesg, pHeader, pErr);
	sip_freeSipHeader(pHeader);
	fast_memfree(0,pHeader,SIP_NULL);

	/* Set Call-ID header */
	sip_initSipHeader(&pHeader,SipHdrTypeCallId,pErr);
	sip_setValueInCallIdHdr(pHeader,sip_strdup(pCallID,0),pErr);
	sip_setHeader(pSipmesg, pHeader, pErr);
	sip_freeSipHeader(pHeader);
	fast_memfree(0,pHeader,SIP_NULL);

	/* Set CSeq header */
	sip_initSipHeader(&pHeader,SipHdrTypeCseq,pErr);
	if(strcmp(pMethod,"BYE")==0)
		sip_setSeqNumInCseqHdr(pHeader,2,pErr);
	else
		sip_setSeqNumInCseqHdr(pHeader,1,pErr);
	sip_setMethodInCseqHdr(pHeader,sip_strdup(pMethod,0),pErr);
	sip_setHeader(pSipmesg, pHeader, pErr);
	sip_freeSipHeader(pHeader);
	fast_memfree(0,pHeader,SIP_NULL);

	/* Convenience functions for inserting headers from strings into
	   the message*/
	sip_initSipHeader(&pHeader, SipHdrTypeVia, pErr);
	sip_setSentProtocolInViaHdr(pHeader, sip_strdup("SIP/2.0/UDP",0), pErr);
	strcpy(sent_by, dOur_host);
	strcat(sent_by,":");
	sprintf(sent_by+(sizeof(SIP_S8bit)*strlen(sent_by)), "%d", dServer_port);
	sip_setSentByInViaHdr(pHeader, sip_strdup(sent_by,0), pErr);
	sip_insertHeaderAtIndex(pSipmesg, pHeader, 0, pErr);
	sip_freeSipHeader(pHeader);
	fast_memfree(0,pHeader,SIP_NULL);

	pContext = (SipEventContext* ) fast_memget(0,sizeof(SipEventContext),NULL);
	sprintf(contextstr,"%s request ",pMethod);
	pCall_context = (SipCallContext *) fast_memget(0,sizeof(SipCallContext),\
		NULL);
	pContext->pData = pCall_context;
	strcpy(pCall_context->contextStr, contextstr);
	sip_getHeader(pSipmesg, SipHdrTypeCallId, &tmphdr, pErr);
	sip_getValueFromCallIdHdr(&tmphdr, &pCallid, pErr);
	strcpy(pCall_context->callId, pCallid);
	sip_freeSipHeader(&tmphdr);
	dOptions.dOption = 0;

#ifdef SIP_TXN_LAYER
	sip_txn_initSipTxnContext(&pTxncontext,pErr);
	dbypass = SipTxnNoByPass;

	pTxncontext->pEventContext= pContext;
	pTxncontext->txnOption.dOption = dOptions;
	pTxncontext->dTxnType = SipUATypeTxn;
	pTxncontext->txnOption.dTimerOption = 0;
	pTxncontext->txnOption.dTimeoutCbkOption = SIP_ISSUE_CBKFOR_ALLTIMERS;
#endif

#ifdef SIP_TXN_LAYER
	pTempAddr = (SipTranspAddr*) malloc(sizeof(SipTranspAddr));
	*pTempAddr = *pSendaddr;
	dRes = sip_txn_sendMessage(pSipmesg,pTempAddr,SIP_UDP,\
		pTxncontext, dbypass,&pTxnKey,pErr);
	if ( ppTxnKey != NULL )
	{
		*ppTxnKey=pTxnKey;
	}
	fast_memfree(0,pTxncontext,SIP_NULL);
#else
	dRes = sip_sendMessage(pSipmesg,&dOptions,pSendaddr,SIP_UDP,\
		pContext,pErr);
#endif
	if ( dRes != SipSuccess )
	{
		switch(*pErr)
		{
			case E_TXN_NO_EXIST:
				printf("** ERROR : The Txn for this message" \
					"doesn't exist**\n");
				fflush(stdout);
				break;
			case E_TXN_EXISTS:
				printf("** ERROR : The Txn for this message" \
					"already exists**\n");
				fflush(stdout);
				break;
			case E_TXN_INV_STATE:
				printf("** ERROR : This message leads to"\
					"an invalid transaction state**\n");
				fflush(stdout);
				break;
			case E_TXN_INV_MSG:
				printf("** ERROR : This is an invalid message"\
					"for received for the Txn**\n");
				fflush(stdout);
			break;
			default:
				printf("Error No: %d\n",*pErr);
				fflush(stdout);
				break;
		}
		printf("sip_sendRequest:** ERROR : Send Message Failed**\n");
		fflush(stdout);
		sip_freeSipMessage(pSipmesg);
		return SipFail;
	}

	sip_freeSipMessage(pSipmesg);
	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: 		init_statistics
 **
 ** DESCRIPTION: 	Function used to initialize the statistics.
 **
 ******************************************************************************/
void init_statistics()
{
	SIP_S32bit i;

	glbpTest_statistics = (SipTestStatistics *) fast_memget(0, \
					sizeof(SipTestStatistics)* (nMsgProcThreads + 1),NULL);
	for (i=0; i<=nMsgProcThreads; ++i)
	{
		glbpTest_statistics[i].calls_attempted = 0;
		glbpTest_statistics[i].successful_setups = 0;
		glbpTest_statistics[i].teardowns_attempted = 0;
		glbpTest_statistics[i].timed_out_calls = 0;
		glbpTest_statistics[i].successful_teardowns = 0;
		glbpTest_statistics[i].message_retransmissions = 0;
	}
	dCumulative_stats.calls_attempted = 0;
	dCumulative_stats.successful_setups = 0;
	dCumulative_stats.teardowns_attempted = 0;
	dCumulative_stats.timed_out_calls = 0;
	dCumulative_stats.successful_teardowns = 0;
	dCumulative_stats.message_retransmissions = 0;

}

/******************************************************************************
 ** FUNCTION: 		display_statistics
 **
 ** DESCRIPTION: 	Function used to display the statistics.
 **
 ******************************************************************************/
void display_statistics(SIP_S32bit dInit_flag)
{
	static SipBool dInitializedFlag = (SipBool)0;
	static struct timeval start_time;
	static struct timeval prev_time;
	struct timeval cur_time;
	SIP_U32bit time_diff; /* in millisecs */
	SipTestStatistics cur_stats, delta_stats;
	SIP_S32bit i;

	if (dInit_flag)
	{
		dInitializedFlag = (SipBool)1;
		gettimeofday(&start_time, NULL);
		prev_time = start_time;
		return;
	}

	if (dInitializedFlag == 0)
		return;

	/* compute cumulative stats */
	cur_stats.calls_attempted = 0;
	cur_stats.successful_setups = 0;
	cur_stats.teardowns_attempted = 0;
	cur_stats.successful_teardowns = 0;
	cur_stats.timed_out_calls = 0;
	cur_stats.message_retransmissions = 0;

	/* compute time between readings */
	gettimeofday(&cur_time, NULL);
	time_diff = (cur_time.tv_sec - prev_time.tv_sec) * 1000 + \
		(((cur_time.tv_usec+1000000) - prev_time.tv_usec)-1000000) / 1000;

	for (i=0; i<=nMsgProcThreads; ++i)
	{
		cur_stats.calls_attempted += \
			glbpTest_statistics[i].calls_attempted;
		cur_stats.successful_setups += \
			glbpTest_statistics[i].successful_setups;
		cur_stats.teardowns_attempted += \
			glbpTest_statistics[i].teardowns_attempted;
		cur_stats.successful_teardowns += \
			glbpTest_statistics[i].successful_teardowns;
		cur_stats.timed_out_calls += \
			glbpTest_statistics[i].timed_out_calls;
		cur_stats.message_retransmissions += \
			glbpTest_statistics[i].message_retransmissions;
	}


	/* Compute the difference from previous reading */
	delta_stats.calls_attempted = cur_stats.calls_attempted - \
								  dCumulative_stats.calls_attempted;
	delta_stats.successful_setups = cur_stats.successful_setups - \
								  dCumulative_stats.successful_setups;
	delta_stats.teardowns_attempted = cur_stats.teardowns_attempted - \
								  dCumulative_stats.teardowns_attempted;
	delta_stats.successful_teardowns = cur_stats.successful_teardowns - \
								  dCumulative_stats.successful_teardowns;
	delta_stats.timed_out_calls = cur_stats.timed_out_calls - \
								  dCumulative_stats.timed_out_calls;
	delta_stats.message_retransmissions = cur_stats.message_retransmissions -\
								  dCumulative_stats.message_retransmissions;

#ifdef SIP_SOLARIS
	printf("\nStatistics for the last %5.2f seconds:\n", time_diff/1000.0);
	fflush(stdout);
#endif

	printf("-------------------------------------------- \n");
	fflush(stdout);
	printf("Calls attempted           = %d\n", delta_stats.calls_attempted);
	fflush(stdout);
	printf("Teardowns attempted       = %d\n",
		   delta_stats.teardowns_attempted);
	fflush(stdout);
	if (glbClient_mode == 1)
	{
		printf("Successful setups         = %d\n",
			delta_stats.successful_setups);
		fflush(stdout);
		printf("Successful teardowns      = %d\n",
		   	delta_stats.successful_teardowns);
		fflush(stdout);
		printf("Timed out calls           = %d\n",
		   	delta_stats.timed_out_calls);
		fflush(stdout);
	}
	printf("Retransmissions           = %d\n",
		   delta_stats.message_retransmissions);
	fflush(stdout);

	/* compute total time elapsed */
	time_diff = (cur_time.tv_sec - start_time.tv_sec) * 1000 + \
			    (((cur_time.tv_usec+1000000) - \
			    start_time.tv_usec)-1000000) / 1000;

#ifdef SIP_SOLARIS
	printf("\nCumulative Statistics : (total time elapsed = %5.2f seconds)",
		   time_diff/1000.0);
#endif

	printf("\n-----------------------------------------------------------\n");
	printf("Calls attempted           = %d\n", cur_stats.calls_attempted);
	printf("Teardowns attempted       = %d\n", cur_stats.teardowns_attempted);
	fflush(stdout);
	if (glbClient_mode == 1)
	{
		printf("Successful setups         = %d\n", cur_stats.successful_setups);
		printf("Successful teardowns      = %d\n", \
			cur_stats.successful_teardowns);
		printf("Timed out calls           = %d\n", cur_stats.timed_out_calls);
	}
	printf("Retransmissions           = %d\n",
		   cur_stats.message_retransmissions);
	fflush(stdout);

	if ((dCumulative_stats.successful_teardowns >= dClient_params.nMaxCalls)
	    && (dClient_params.nMaxCalls != 0))
	{
		sip_releaseStack();
		exit (0);
	}

	dCumulative_stats = cur_stats;
	prev_time = cur_time;
}

/******************************************************************************
 ** FUNCTION: 		hss_getChar
 **
 ** DESCRIPTION: 	Function used to read the character.
 **
 ******************************************************************************/
SIP_S32bit hss_getChar()
{
	return getchar();
}

/******************************************************************************
 ** FUNCTION: 		sip_sleep
 **
 ** DESCRIPTION: 	Function sleeps for the specified duration
 **
 ******************************************************************************/
void sip_sleep(long microsecs)
{
	struct timeval timeout;

	timeout.tv_sec = microsecs /  1000000;
	timeout.tv_usec = microsecs % 1000000;

   if(microsecs != 0)
		select(64,NULL,NULL,NULL, &timeout);
}

/******************************************************************************
 ** FUNCTION: 		sip_getchar
 **
 ** DESCRIPTION: 	A wrapper Function used to read the character.
 **					This function is used to avoid the inconsistency in the
 **					getchar call on LINUX
 **
 ******************************************************************************/
SIP_S8bit sip_getchar(void)
{
	SIP_S8bit dOption;
	dOption = getchar();
	return dOption;
}

#ifdef SIP_TXN_LAYER
/********************************************************************
 ** FUNCTION: 		sip_freeTimerHandle
 **
 ** DESCRIPTION: 	This is a callback used to free the timer handle
*********************************************************************/
void sip_freeTimerHandle(SIP_Pvoid	pTimerHandle)
{
	/*Making this a dummy function since the actual freeing of the
	 * timer handle is being handled thru the application itself.
	 */
	 if(pTimerHandle != SIP_NULL)
		fast_memfree(0,pTimerHandle,SIP_NULL);
}

/******************************************************************************
 ** FUNCTION: 		sip_indicateTimeOut
 **
 ** DESCRIPTION: 	This function is invoked by the stack when a request or a
 **					response times out.
 **
 **
 ******************************************************************************/
#ifdef SIP_TXN_LAYER
#ifdef ANSI_PROTO
void sip_indicateTimeOut( SipEventContext *pContext,en_SipTimerType dTimer)
#else
void sip_indicateTimeOut(pContext,dTimer)
	SipEventContext *pContext;
	en_SipTimerType dTimer;
#endif
#else
#ifdef ANSI_PROTO
void sip_indicateTimeOut( SipEventContext *pContext)
#else
void sip_indicateTimeOut(pContext)
	SipEventContext *pContext;
#endif
#endif
{
	SipCallContext *pCall_context;
	SIP_S32bit dCallindex;
	SipEventContext *dummy;
	SipError err;
#ifdef SIP_TXN_LAYER
	SipTxnContext empty_txnContext ;
#endif
	dummy = pContext;
	printf ("Inside retransmission timeout callback------\n");
	fflush(stdout);
	if(pContext->pData)
		printf ("%s timed out.\n",(SIP_S8bit *)pContext->pData);
	fflush(stdout);
#ifdef SIP_TXN_LAYER
	switch(dTimer)
	{
		case SipTimerA_B:
			printf("***TimerA_B has been TimedOut******\n");
			break;
		case SipTimerB:
			printf("***TimerB has been TimedOut******\n");
			break;
		case SipTimerC:
			printf("***TimerC has been TimedOut******\n");
			break;
		case SipTimerD:
			printf("***TimerD has been TimedOut******\n");
			break;
		case SipTimerE_F:
			printf("***TimerE_F has been TimedOut******\n");
			break;
		case SipTimerF:
			printf("***TimerF has been TimedOut******\n");
			break;
		case SipTimerG_H:
			printf("***TimerG_H has been TimedOut******\n");
			break;
		case SipTimerH:
			printf("***TimerH has been TimedOut******\n");
			break;
		case SipTimerI:
			printf("***TimerI has been TimedOut******\n");
			break;
		case SipTimerJ:
			printf("***TimerJ has been TimedOut******\n");
			break;
		case SipTimerK:
			printf("***TimerK has been TimedOut******\n");
			break;
		default:
			printf("***Unknown Timer******\n");
	}
	fflush(stdout);
	pCall_context = (SipCallContext *) pContext->pData;
	printf ("%s timed out for call id %s.\n",
		pCall_context->contextStr, pCall_context->callId);
	fflush(stdout);
	if (glbClient_mode == 1)
	{
		printf("\tReinitiating call id %s\n", pCall_context->callId);
		dCallindex = atoi(pCall_context->callId)-1;
		glbpTest_statistics[0].timed_out_calls++;
		fast_lock_synch(0,&glbpCallObjectArray[dCallindex].callLock,0);
		glbpCallObjectArray[dCallindex].state = SipInviteSent;
		glbpTest_statistics[0].calls_attempted++;

		memset(&empty_txnContext,0,sizeof(SipTxnContext)) ;
		sip_txn_abortTxn( \
			(SipTxnKey*)(glbpCallObjectArray[dCallindex].pInvTxnKey),\
			&empty_txnContext,&err);
		sip_freeSipTxnKey( \
			(SipTxnKey*)(glbpCallObjectArray[dCallindex].pInvTxnKey));
		glbpCallObjectArray[dCallindex].pInvTxnKey=NULL;
		sip_sendRequest("INVITE", &(dClient_params.client_dest_addr),\
			glbpCallObjectArray[dCallindex].callId, \
			(SipTxnKey**)&(glbpCallObjectArray[dCallindex].pInvTxnKey),&err);
		fast_unlock_synch(0,&glbpCallObjectArray[dCallindex].callLock);
	}
#endif
	sip_freeEventContext(pContext);
}

/******************************************************************************
 ** FUNCTION: 		displayThread
 **
 ** DESCRIPTION: 	This function is responsible for printing out the statistics
 **					at every 5 seconds
 **
 **
 ******************************************************************************/
SIP_Pvoid displayThread(SIP_Pvoid pDummy)
{
	SIP_Pvoid pNDummy;
	pNDummy = pDummy;
	while(1)
	{
		__sip_sleep(5000000);
		printf("---> Calling Display Stat.\n");
		fflush(stdout);
		display_statistics(0);
	}
	return NULL;
}
#endif

/******************************************************************************
 ** FUNCTION: 		signal_handle
 **
 ** DESCRIPTION: 	This function is used for catching event when the user
 **					does a ^C to stop the application.
 **
 **
 ******************************************************************************/
void signal_handle(SIP_S32bit INT_NUM)
{
	SIP_S8bit dOpt='n';
	printf("\n*****Signal %d received*****\n",INT_NUM);
	if ( 2 == INT_NUM ) /* Check for signal SIGINT */
	{
		printf("Do you want to Exit(y/n):");
		fflush(stdout);
		fflush(stdin);
		dOpt=getchar();
		fflush(stdin);
		if ( 'y' == dOpt )
		{
			SIP_Pvoid pVal;
			SIP_S32bit j;
			/* Cancel all the threads */	
#ifndef SIP_TXN_LAYER			

#ifdef SIP_SOLARIS
			pthread_cancel(timerID);
			pthread_join(timerID,&pVal);
#endif
#ifdef SIP_OSE
			pthread_cancel(timerID);
			pthread_join(timerID,&pVal);
#endif
#else

#ifdef SIP_SOLARIS
			pthread_cancel(dTimerThread);
			pthread_join(dTimerThread,&pVal);

			pthread_cancel(dDisplayThread);
			pthread_join(dDisplayThread,&pVal);
#endif
#ifdef SIP_OSE
			pthread_cancel(dTimerThread);
			pthread_join(dTimerThread,&pVal);

			pthread_cancel(dDisplayThread);
			pthread_join(dDisplayThread,&pVal);
#endif
#endif /* #ifndef SIP_TXN_LAYER */
			/* Cancel the message processing threads */
			for(j=1; j< (SIP_U8bit)(nMsgProcThreads+1); j++)
			{
				pVal = SIP_NULL;

#ifdef SIP_SOLARIS
				pthread_cancel(msgProcThreadMgr.workerThreadId[j]);
				pthread_join(msgProcThreadMgr.workerThreadId[j],&pVal);
#endif
#ifdef SIP_OSE
				pthread_cancel(msgProcThreadMgr.workerThreadId[j]);
				pthread_join(msgProcThreadMgr.workerThreadId[j],&pVal);
#endif
			}
			/* Free the memory allocated by thread manager */
			threadMgrFree(&msgProcThreadMgr);

			/* Free the transaction keys */
			if(glbClient_mode)
			{
#ifdef SIP_TXN_LAYER
				SIP_S32bit i;
				for(i=0; i< dClient_params.nCalls; i++)
				{
						if(glbpCallObjectArray[i].pInvTxnKey != SIP_NULL)
							sip_freeSipTxnKey((SipTxnKey*)\
									(glbpCallObjectArray[i].pInvTxnKey));	
						if(glbpCallObjectArray[i].pByeTxnKey != SIP_NULL)
							sip_freeSipTxnKey((SipTxnKey*)\
									(glbpCallObjectArray[i].pByeTxnKey));	
				}
#endif						
				fast_memfree(0,glbpCallObjectArray,SIP_NULL);
			}		
			/* Free the memory allocated for storing statistics */
			fast_memfree(0,glbpTest_statistics, SIP_NULL);

			/* Free the memory allocated for posting of event */
			fast_memfree(0,pEvent->message,NULL);
			fast_memfree(0,pEvent,NULL);

#ifdef SIP_TXN_LAYER
			/* Free the Timer wheel and hash table */
			__sip_flushTimer();
			__sip_flushHash();
			sip_freeTimerWheel();
			sip_freeHashTable();
#else
			/* Free the Hash table */
			flushTimerWheel();
#endif			
			sip_releaseStack();
			exit(1);
		}
	}
	signal(INT_NUM,signal_handle);
	return;
}
/******************************************************************************
 ** FUNCTION: 	 sip_getCallIdFromBuffer
 **
 ** DESCRIPTION: Function to do a first level parse and get the call-id
 **				for the call
 *****************************************************************************/
SipBool sip_getCallIdFromBuffer
#ifdef ANSI_PROTO
	(SIP_S8bit *pString, SIP_U32bit *pCallId)
#else
	(pString, dCallId)
	SIP_S8bit *pString;
	SIP_U32bit *pCallId;
#endif
{
	SIP_S8bit *ptr = SIP_NULL;
	SIP_S8bit pTempArray[200];
	SIP_U32bit dIndex = 0, dOffSet = 0;

	ptr = strstr(pString, "Call-ID:");
	if (ptr == NULL)
	{
		/* Full form header not found. Look for short form. */
		ptr = strstr(pString, "\ni:");
		if (ptr == SIP_NULL)
		{
			printf("************ ERROR IN GETTING CALL-ID **************\n");
			return SipFail;
		}
	}

	dOffSet = ptr - pString + 8;

	while ( (*(pString + dOffSet) != '\r') && (*(pString + dOffSet) != '\n'))
	{
		pTempArray[dIndex] = *(pString+dOffSet);
		dIndex++;
		dOffSet++;
	}

	pTempArray[dIndex] = '\0';
	*pCallId = atoi(pTempArray);

 /*	printf("Returning callid as ----------> %d\n", *pCallId); */
	return SipSuccess;
}


/*****************************************************************************
 ** FUNCTION: 	 sip_getThreadIdFromCallId
 **
 ** DESCRIPTION: Function to return the thread id for the thread handling
 **				 a particular call id.
 **
 *****************************************************************************/
SipBool sip_getThreadIdFromCallId
#ifdef ANSI_PROTO
	(SIP_U32bit dCallId, SIP_S8bit *pThreadId)
#else
	(dCallId, pThreadId)
	SIP_U32bit dCallId;
	SIP_U32bit *pThreadId;
#endif
{
	SIP_S8bit dTemp = 0;

	dTemp = dCallId % msgProcThreadMgr.nWorkerThreads;
	*pThreadId = dTemp;

	return SipSuccess;
}




#ifdef __cplusplus
}
#endif
