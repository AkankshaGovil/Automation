/*  ==============================================================
    Filename: parsetime.c
    --------

    Details:
    -------
    This test stub can be used to calculate the time taken by the stack to
    parse a message. The application waits for the receipt of a SIP
    message in the form of a text buffer. On receipt of the same it
    performs sip_decodeMessage() on the message for a finite number of times.
    This takes place within a loop. At the end of every iteration the decoded
    SipMessage is freed to prevent memory build-up.

    There is a call to
    i) clock in case of Solaris
    ii) clock_gettime in case of VxWorks
    iii) get_ticks() in case of OSE
    at the beginning of the loop and at the end which gives us the
    CPU time (or absolute time in case of VxWorks/OSE) taken for those
    many decodes.

    Dividing the time taken by the number of iterations gives the
    decode time for one message.

    Notes:
    -----
    1. The stack should be compiled in SIP_BY_REFERENCE mode to execute
        this program.

    2. For VxWorks please change the define for TARGET_IP at the
        beginning of the file.

    2.  The number of iterations is configurable and can be changed
        by changing the define for NUM_ITERATIONS

    Usage:
    -----
         parsetime <listening_port>

    ==============================================================
                (c) Hughes Software Systems, 2001
    ============================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#define TARGET_IP "139.85.229.176"
				
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <strings.h>

#include <sys/types.h>
#include <stdlib.h>

#include "portlayer.h"
#include "sipdecode.h"
#include "sipstruct.h"
#include "sipsendmessage.h"
#include "siplist.h"
#include "sipinit.h"
#include "sipfree.h"
#include "general.h"
#include "request.h"
#include "response.h"
#include "entity.h"
#include "ccp.h"
#include "rpr.h"
#include "header.h"
#include "sipstatistics.h"
#include "siptimer.h"
#include "siptrace.h"
#ifdef SIP_TXN_LAYER
#include "siptxnstruct.h"
#include "txndecode.h"
#include "siptxntimer.h"
#include "siptxntest.h"
#endif

#define MAXMESG 5000

#ifdef DYNAMIC_MEMORY_CALC
/*
 * Extern definitions to count the memory allocations
 * NUM_ITERATIONS is made 1 because the aim of
 * DYNAMIC MEMORY CALCULATION is to find a pattern
 * of dynamic memory allocation for a given
 * standard message
 */
extern SIP_U32bit glb_memory_alloc_dist[5] ;
extern SIP_U32bit glb_memory_alloc_parser[3] ;
#define NUM_ITERATIONS 1
#define INIT_MEMORY_PATTERN_ARRAYS {\
    int iter=0 ;\
    for (iter=0;iter<6;iter++)\
      glb_memory_alloc_dist[iter]=0 ;\
    for (iter=0;iter<3;iter++)\
      glb_memory_alloc_parser[iter]=0 ;\
}

# define  PRINT_MEMORY_PATTERN_ARRAYS {\
    \
        printf("\n--------------------------------------------------------") ;\
        printf("\n No of Allocs with Size of  0  < size <=  10   = %3d ",glb_memory_alloc_dist[1]) ;\
        printf("\n No of Allocs with size of 10  < size <=  100  = %3d ",glb_memory_alloc_dist[2]) ;\
        printf("\n No of Allocs with size of 100 < size <= 1000  = %3d ",glb_memory_alloc_dist[3]) ;\
        printf("\n No of Allocs with size > 1000                 = %3d ",glb_memory_alloc_dist[4]) ;\
        \
        printf("\n\n No of Bytes Allocated [DECODE]  = %3d ",glb_memory_alloc_parser[0]) ;\
        printf("\n No of Bytes Allocated [BISON]   = %3d ",glb_memory_alloc_parser[1]) ;\
        printf("\n No of Bytes Allocated [FLEX]    = %3d ",glb_memory_alloc_parser[2]) ;\
        printf("\n--------------------------------------------------------\n") ;\
}
#endif

#ifndef NUM_ITERATIONS
#define NUM_ITERATIONS 10000
#endif

int showMessage = 1;
int constructCLen;



void sip_freeEventContext(SipEventContext *context)
{
(void) context;
}

SipBool sip_decryptBuffer(SipMessage *s, SIP_S8bit *encinbuffer, SIP_U32bit clen,SIP_S8bit **encoutbuffer, SIP_U32bit *outlen)
{
	(void) s;
	(void) encinbuffer;
	(void) clen;
	(void) encoutbuffer;
	(void) outlen;
	return SipFail;
}

#ifdef SIP_TXN_LAYER
/*Dummy Callbacks*/
SipBool fast_startTxnTimer
#ifdef ANSI_PROTO
    (
    SIP_U32bit dDuration,
    TimeoutFuncPtr funcptr,
    SIP_Pvoid pData,
    SIP_Pvoid* ppHandle,
    SipError *pErr
    )
#else
    (dDuration,funcptr,pData,ppHandle,pErr)
    SIP_U32bit dDuration;
    TimeoutFuncPtr funcptr;
    SIP_Pvoid pData;
    SIP_Pvoid* ppHandle;
    SipError *pErr;
#endif
{
	(void)dDuration;
	(void)funcptr;
	(void)pData;
	(void)ppHandle;
	(void)pErr;
	return SipSuccess;
}

SipBool fast_stopTxnTimer
#ifdef ANSI_PROTO
    ( SIP_Pvoid pHandle,SIP_Pvoid* ppBuff,SipError* pErr )
#else
    (pHandle,ppBuff,pErr)
    SIP_Pvoid pHandle;
    SIP_Pvoid* ppBuff;
    SipError* pErr;
#endif
{
	(void)pHandle;
	(void)ppBuff;
	(void)pErr;
	return SipSuccess;
}

SipBool sip_cbk_fetchTxn
#ifdef ANSI_PROTO
	(SIP_Pvoid* ppTxnBuffer,SIP_Pvoid pTxnKey,SIP_Pvoid pContext,\
		SIP_S8bit dOpt,SipError *pErr)
#else
	(ppTxnBuffer,pTxnKey,pContext,dOpt,pErr)
	SIP_Pvoid* ppTxnBuffer;
	SIP_Pvoid pTxnKey;
	SIP_Pvoid pContext;
	SIP_S8bit dOpt;
	SipError *pErr;
#endif
{
	(void)ppTxnBuffer;
	(void)pTxnKey;
	(void)pContext;
	(void)dOpt;
	(void)pErr;
	return SipSuccess;
}
SipBool sip_cbk_releaseTxn
#ifdef ANSI_PROTO
	(SIP_Pvoid pTxnKey, SIP_Pvoid* ppTxnKey,SIP_Pvoid *ppTxnBuffer, \
		SIP_Pvoid pContext,SIP_S8bit dOpt, SipError *pErr)
#else
	(pTxnKey, ppTxnKey,ppTxnBuffer,pContext,dOpt,pErr)
	SIP_Pvoid pTxnKey;
	SIP_Pvoid* ppTxnKey;
	SIP_Pvoid *ppTxnBuffer;
	SIP_Pvoid pContext;
	SIP_S8bit dOpt;
	SipError *pErr;
#endif
{
	(void)pTxnKey;
	(void)ppTxnKey;
	(void)ppTxnBuffer;
	(void)pContext;
	(void)dOpt;
	(void)pErr;
	return SipSuccess;
}

/*Dummy Callbacks*/
#endif

SipBool fast_startTimer( SIP_U32bit duration, SIP_S8bit restart,SipBool (*timeoutfunc)(SipTimerKey *key, SIP_Pvoid buf), SIP_Pvoid buffer, SipTimerKey *key, SipError *err)
{
	(void)duration;
	(void)restart;
	(void)timeoutfunc;
	(void)buffer;
	(void)key;
	(void)err;
	return SipSuccess;
}

/* Implementaion of the fast_stopTimer interface reqiuired by the stack
   Application developers may choose to implement this function in any
   manner while preserving the interface
*/
SipBool fast_stopTimer(SipTimerKey *inkey, SipTimerKey **outkey, SIP_Pvoid *buffer,  SipError *err)
{
	(void)inkey;
	(void)outkey;
	(void)buffer;
	(void)err;
	return SipSuccess;
}

SipBool sip_sendToNetwork
#ifdef ANSI_PROTO
( SIP_S8bit *buffer, SIP_U32bit buflen,SipTranspAddr *addr, SIP_S8bit transptype, SipError *err)
#else
	(buffer, buflen, addr, transptype, err)
	SIP_S8bit *buffer;
	SIP_U32bit buflen;
	SipTranspAddr *addr;
	SIP_S8bit transptype;
	SipError *err;
#endif
{
	(void)buffer;
	(void)buflen;
	(void)addr;
	(void)transptype;
	(void)err;
	return SipSuccess;
}

#ifdef SIP_TXN_LAYER
#ifdef ANSI_PROTO
void sip_indicateTimeOut( SipEventContext *context,en_SipTimerType dTimer)
#else
void sip_indicateTimeOut(context,dTimer)
        SipEventContext *context;
        en_SipTimerType dTimer;
#endif
#else
#ifdef ANSI_PROTO
void sip_indicateTimeOut( SipEventContext *context)
#else
void sip_indicateTimeOut(context)
        SipEventContext *context;
#endif
#endif
{
	(void)context;
#ifdef SIP_TXN_LAYER
	(void)dTimer;
#endif
}

#ifdef SIP_SELECTIVE_PARSE
SIP_S8bit sip_willParseSdpLine (SIP_S8bit *in, SIP_S8bit **out)
{
	(void)in;
	(void)out;
	return 1;
}

SipBool sip_acceptIncorrectSdpLine (SIP_S8bit *in)
{
	(void)in;
	return SipFail ;
}


SIP_S8bit sip_willParseHeader (en_HeaderType type, SipUnknownHeader *hdr)
{
	(void)type;
	(void)hdr;
	return 1;
}

SipBool sip_acceptIncorrectHeader (en_HeaderType type, SipUnknownHeader *hdr)
{
	(void)type;
	(void)hdr;
	return SipFail;
}
#endif


#ifdef SIP_RETRANSCALLBACK
#ifdef  SIP_TXN_LAYER
/******************************************************************************
 ** FUNCTION: 		sip_indicateMessageRetransmission
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_RETRANSCALLBACK option is enabled.
 **
 ******************************************************************************/
void sip_indicateMessageRetransmission (SipEventContext *pContext,\
	SipTxnKey *pKey, SIP_S8bit *pBuffer,\
	SIP_U32bit dBufferLength, SipTranspAddr *pAddr, SIP_U8bit dRetransCount,\
	SIP_U32bit dDuration)
{
	(void) pContext;
	(void) pKey;
	(void) dBufferLength;
	(void) pAddr;
	(void ) dRetransCount;
	(void ) pBuffer;
	(void ) dDuration;
	SIPDEBUG("Inside sip_indicateMessageRetransmission.\n");
}
#else
/*******************************************************************************
 ** FUNCTION: 		sip_indicateMessageRetransmission
 **
 ** DESCRIPTION: 	This is a callback function that is invoked by the stack
 **					when the SIP_RETRANSCALLBACK option is enabled.
 **
 ******************************************************************************/
void sip_indicateMessageRetransmission (SipEventContext *pContext,\
	SipTimerKey *pKey, SIP_S8bit *pBuffer,\
	SIP_U32bit dBufferLength, SipTranspAddr *pAddr, SIP_U8bit dRetransCount,\
	SIP_U32bit dDuration)
{
	(void) pContext;
	(void) pKey;
	(void) dBufferLength;
	(void) pAddr;
	(void ) dRetransCount;
	(void ) pBuffer;
	(void ) dDuration;
	SIPDEBUG("Inside sip_indicateMessageRetransmission.\n");
}
#endif
#endif

#ifndef SIP_NO_CALLBACK
void sip_indicateInvite(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateRegister(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateCancel(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateOptions(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateBye(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}


void sip_indicateAck(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateInfo(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateUpdate(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicatePropose(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicatePrack(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateUnknownRequest(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateInformational(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateFinalResponse(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateRefer(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateSubscribe(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateNotify(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

void sip_indicateMessage(SipMessage *s, SipEventContext *context)
{
	(void)context;
	sip_freeSipMessage(s);
}

#ifdef SIP_DCS
void sip_indicateComet(SipMessage *s, SipEventContext *context)
{
	(void)s;
	(void)context;
	sip_freeSipMessage(s);
}
#endif

#endif

#ifdef SIP_MIB
void sip_indicateResponseHandled(SipMessage *s,en_SipMibCallbackType dEvent,\
		SIP_S32bit	dCode,SipStatParam dParam)
{
	(void)dEvent;
	(void)dCode;
	(void)dParam;
	sip_freeSipMessage(s);
}
#endif

/* End of callback implementations */
int main(int argc, char * argv[])
{
	int			sockfd, clilen,n;
	struct sockaddr_in	serv_addr, cli_addr;
	fd_set readfs;
	char *message;
	SIP_S8bit *nextmesg;
	SipError error;
	SipBool r;
	SIP_U32bit stat;
	clock_t pstime1,pstime;



	if(argc<2)
	{
		printf("Usage:\n");
		printf("%s my_port\n",argv[0]);
		exit(0);
	}

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("Server could not open dgram socket\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(atoi(argv[1]));

	bzero((char *) &cli_addr, sizeof(cli_addr));	/* zero out */
	cli_addr.sin_family      = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port        = htons(0);

	clilen=sizeof(cli_addr);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Server couldn't bind to local address.\n");
		exit(0);
	}

	/* Stack and trace initialization */
	sip_initStack();

	printf("\nSip stack product id is:%s\n", (char *)(sip_getPart()) );

	if (sip_setErrorLevel(SIP_Major|SIP_Minor|SIP_Critical, &error)==SipFail)
	{
		printf ("########## Error Disabled at compile time #####\n");
	}


	if (sip_setTraceLevel(SIP_Brief,&error)== SipFail)
	{
		printf ("########## Trace Disabled at compile time #####\n");
	}
	sip_setTraceType(SIP_All,&error);


	for(;;)
	{
		int i;
		struct timeval timeout;
		struct timeval tv1, tv2;
#ifdef SIP_NO_CALLBACK
		SipMessage *pMsg;
#endif

		/* get minimum timeout from the timer list */
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;
		FD_ZERO(&readfs);
		FD_SET(sockfd,&readfs);
		FD_SET(0,&readfs);


		gettimeofday(&tv1, NULL);
		if (select(sockfd+1, &readfs, NULL, NULL, &timeout) < 0)
		{
				printf("select returned < 0\n");
				fflush(stdout);
				exit(0);
		}
		gettimeofday(&tv2, NULL);

		if(FD_ISSET(sockfd,&readfs))
		{
			/* Message on the receive port */
			message = (char *) malloc(sizeof(char)*MAXMESG);
			n = recvfrom(sockfd, message, MAXMESG, 0,(struct sockaddr *) &cli_addr, &clilen);

			if (n < 0)
			{
				printf("Server : Error in receive.\n");
				exit(0);
			}
			message[n]='\0';
			do
			{
				SipOptions opt;

				/* calclulate time to parse one complete message */
				opt.dOption  = SIP_OPT_NOTIMER;
				pstime = clock();
#ifdef DYNAMIC_MEMORY_CALC
       INIT_MEMORY_PATTERN_ARRAYS ;
#endif
 

				for(i=0; i<NUM_ITERATIONS; i++)
				{
#ifndef SIP_NO_CALLBACK
					r= sip_decodeMessage((SIP_S8bit *)message,&opt, \
						strlen(message),&nextmesg,NULL, &error);
#else
					r= sip_decodeMessage((SIP_S8bit *)message,&pMsg,&opt, \
						strlen(message),&nextmesg,NULL, &error);
					sip_freeSipMessage(pMsg);
#endif
				}
				pstime1 = clock();

				printf("\n=========== Results for complete message parsing =========\n\n");
				printf("CPU clock ticks for %d messages - %d\n", NUM_ITERATIONS, pstime1-pstime);
				printf("CPU time for %d messages - %f sec\n", NUM_ITERATIONS, (float)(pstime1-pstime)/CLOCKS_PER_SEC);
				printf("\n=========== Results for complete message parsing =========\n");

				if (r==SipFail)
				{
					printf ("+++++++ BAD MESSAGE %d++++++++++\n",error);
				}

				/* calclulate time to parse one message without parsing message*/
				opt.dOption  = SIP_OPT_NOPARSEBODY;
				pstime = clock();
#ifdef DYNAMIC_MEMORY_CALC
      /*
       * Print the memory global arrays and
       * initialize them
       *
       */
       PRINT_MEMORY_PATTERN_ARRAYS ;
       INIT_MEMORY_PATTERN_ARRAYS ;
#endif
 

				for(i=0; i<NUM_ITERATIONS; i++)
				{	
#ifndef SIP_NO_CALLBACK
				r= sip_decodeMessage((SIP_S8bit *)message,&opt,	strlen(message),&nextmesg,NULL, &error);
#else
				r= sip_decodeMessage((SIP_S8bit *)message,&pMsg,&opt, \
					strlen(message),&nextmesg,NULL, &error);
				sip_freeSipMessage(pMsg);
#endif
				}
				pstime1 = clock();

				printf("\n=========== Results for message parsing (without body) =========\n\n");

				printf("CPU clock ticks for %d messages - %d\n", NUM_ITERATIONS, pstime1-pstime);
				printf("CPU time for %d messages - %f sec\n", NUM_ITERATIONS, (float)(pstime1-pstime)/CLOCKS_PER_SEC);
				printf("\n=========== Results for message parsing (without body) =========\n");

				fast_memfree(0,message, SIP_NULL);
				message = nextmesg;
			} while (message != SIP_NULL);

# ifdef DYNAMIC_MEMORY_CALC
      /*
       * Print the memory global arrays and
       * initialize them
       *
       */
        PRINT_MEMORY_PATTERN_ARRAYS ;
        INIT_MEMORY_PATTERN_ARRAYS ;
#endif

		}
		else if(FD_ISSET(0,&readfs))
		{
			/* Key pressed */
			char c;
			c=getchar();
			if ( sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_COUNT, SIP_STAT_NO_RESET, &stat, &error)==SipFail)
			{
				printf ("##### Cannot display Statistics - Stats disabled #####\n");
			}
			else
			{
			printf("\n######## Stack statistics #######\n");
			sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_REQ_PARSED, SIP_STAT_NO_RESET, &stat, &error);
			printf("######## Requests parsed 	:%d\n",stat);
			}
			if (c == 'q')
			{
				close(sockfd);
				break;
				/*exit(0);*/
			}
		}
	}
	return 1; /* to avoid warning */
}


#ifdef __cplusplus
}
#endif
