/*
	This test stub can be used to send simple SIP Requests/Responses
	by UDP with retransmission. This program shows a sample timer function
	implementation. The timer uses a linked list in this program and relies
	on the select system call to handle timeout callbacks required by the
	stack. The developer may use other methods to implement the timer
	functionality expected by the stack.

	Usage:
		siptest <my_port> <dest_addredd> [dest_port]

		myport: local port on which the program listens for
				incoming sip messages
		dest_addr: address to which the messages are sent
		dest_port: port to which the messages are sent
				defaults to 5060 if not given.
*/

#ifdef __cplusplus
extern "C" {
#endif

#undef SIP_NO_FILESYS_DEP



#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef SIP_OSE
#include <strings.h>
#endif


#include <sys/types.h>
#include <stdlib.h>
#include "portlayer.h"
#include "sipdecode.h"
#include "sipstruct.h"
#include "sipstring.h"
#include "sipsendmessage.h"
#include "siplist.h"
#include "sipinit.h"
#include "sipfree.h"
#include "general.h"
#include "request.h"
#include "response.h"
#include "entity.h"


#ifdef SIP_CCP
#include "ccp.h"
#endif

#include "rpr.h"
#include "header.h"
#include "sipstatistics.h"
#include "siptimer.h"
#include "siptrace.h"
#include "bcpt.h"

#ifdef SIP_TEL
#include "telstruct.h"
#include "telinit.h"
#include "telfree.h"
#include "telapi.h"
#include "teldecodeintrnl.h"
#endif

#ifdef SIP_IMPP
#include "imppstruct.h"
#include "imppinit.h"
#include "imppfree.h"
#include "impp.h"
#endif

#ifdef SIP_PRES
#include "presstruct.h"
#include "pres.h"
#endif

#ifdef SIP_DCS
#include "dcsfree.h"
#include "dcs.h"
#endif

#ifdef SIP_TXN_LAYER
#include "siptxntimer.h"
#include "siptxntest.h"
#include "siptxnstruct.h"
#include "txndecode.h"
#include "txnfree.h"
#endif

#include <pthread.h>

#ifdef SIP_PINT
SipBool extractSdpPintMsgBody(SipMsgBody *pSipMsgBody,SdpMessage *pSdpMsg) ;
#ifdef SIP_BY_REFERENCE
SipBool processSdpPintBodyInByReference (SipMessage *pSipMsg) ;
#else
SipBool processSdpPintBodyInNonByReference (SipMessage *pSipMsg) ;
#endif
#endif

#define MAXMESG 5000

/* NEXTONE - added SipDebugLoc and NetLogDebugSprintf */
int SipDebugLoc() { return 1; }
int NetLogDebugSprintf(char *fmt, ...) { return 1; }

SIP_U16bit requestFlag = 0; /*Disabled*/
SIP_U16bit responseClassFlag = 0; /*Disabled*/
SIP_U16bit responseCodeFlag = 0; /*Disabled*/
SipList slRequestl;

SIP_S8bit glbtransptype = 0;
#ifdef SIP_TXN_LAYER
int glbtransactiontype =0;
int glbsetTimerValue=0;
SipTimeOutValues dTimeOut;
SIP_U32bit glbTimeTimeoutOption;
#endif

/* Internal Function prototypes */

#ifndef SIP_TXN_LAYER
void timerElementFree(SIP_Pvoid element);
SIP_S32bit getMinimumTimeout(SipList *list);
#else
SIP_U32bit getTimerTimeoutOptions(void);
#endif

/* 
 * Related to Message body processing
 * */
#include "sdp.h"
#include "sipbcptinit.h"
#include "sipbcptfree.h"

#ifdef SIP_SOLARIS
#ifdef SIP_BY_REFERENCE
SipMsgBody * buildSdpBody(SipError *pErr) ;
SipMsgBody * buildIsupBody(SipError *pErr) ;
SipBool buildMultipartMixedBody(SipMessage * pSipMsg, SipError *pErr) ;
SipBool extractSdpMsgBody(SipMsgBody *pSipMsgBody,SdpMessage *pSdpMsg) ;
SipBool extractIsupMsgBody(SipMsgBody *pSipMsgBody,IsupMessage * pIsupMsg);
SipBool processSipMsgBody(SipMessage *pSipMsg) ;
SipMimeHeader * buildMimeHeader(SipError *pErr,SIP_S8bit *pDesc,
               SIP_S8bit *pMedia) ;
SipBool sendInviteRequestWithMsgBody(int bodyFlag,
						SipTranspAddr *pSendAddr,SipError *pErr) ;
void csr_check_require(SipMessage *pSipMsg) ;
#endif
void csr_check_sipurl(SipMessage *pSipMsg) ;
#endif

char sip_getchar(void);

void displayParsedMessage(SipMessage *s);
SipBool extractAuthParamFromChallengeAtIndex(SipGenericChallenge *pChal,
																SIP_U32bit dCount, SipError *pErr) ;


#ifdef SIP_TEL
SipBool do_stuff_tel(SipMessage *s);
SipBool do_stuff_tel_nonbyref(SipMessage *s);
#endif

#ifdef SIP_PRES
SipBool do_stuff_presurl_byref(SipMessage *s);
SipBool do_stuff_presurl_nonbyref(SipMessage *s);
#endif

#ifdef SIP_IMPP
SipBool do_stuff_imurl_byref(SipMessage *s);
SipBool do_stuff_imurl_nonbyref(SipMessage *s);
#endif

SipBool sendRequest(const char *method,SipTranspAddr *sendaddr,SipError *err);
SipBool sendResponse(int code,const char *reason,const char *method,\
	SipTranspAddr *sendaddr,SipError *err);
SIP_S8bit sip_willParseSdpLine (SIP_S8bit *in, SIP_S8bit **out);
SipBool sip_acceptIncorrectSdpLine (SIP_S8bit *in);
SIP_S8bit sip_willParseHeader (en_HeaderType type, SipUnknownHeader *hdr);
SipBool sip_acceptIncorrectHeader (en_HeaderType type, SipUnknownHeader *hdr);
void showErrorInformation(SipMessage *pMessage);

SIP_S8bit* quoteString ( SIP_S8bit *pString) ;

#ifdef SIP_SERIALIZE
#include "serialize.h"
SipBool serialize_sip_message(SipMessage *pSipMsg,SIP_S8bit **pBuffer,
				SIP_U32bit pos,SipError *pErr) ;
SipBool deserialize_sip_message(SipMessage **ppSipMsg,SIP_S8bit *pBuffer, 
		SIP_U32bit pos, SipError *pErr) ;
#endif

#ifdef SIP_SOLARIS
#ifdef SIP_BY_REFERENCE
void csr_check_require(SipMessage *pSipMsg)
{
		SipHeader hdr  ;
		SIP_S8bit *pToken = SIP_NULL ;
		SipError err = E_NO_ERROR ;
		SIP_U32bit dIter=0,dCount=0 ;

		fprintf(stdout,"\n***** In csr_check_require ***** \n") ;
		if (sip_getHeaderCount(pSipMsg,SipHdrTypeRequire,&dCount,&err) ==SipFail)
		{
			printf("Error in 	sip_getHeaderCount : %d \n",dCount) ;
			return ;
		}
		else
		{
			printf("Number of SipHdrTypeRequire Headers = [%d] \n",dCount) ;
		}
		dIter = 0 ;
		while (dIter< dCount )
		{
			if(sip_getHeaderAtIndex(pSipMsg,SipHdrTypeRequire,&hdr,dIter,&err)==SipFail)
			{
			 printf("Error in Accessing require header at %d; Error no: %d\n",dIter,err);
			 return ;
		    }
			else
			{
				if (sip_getTokenFromRequireHdr(&hdr,&pToken,&err) == SipFail )
				{
				 printf("Error in sip_getTokenFromRequireHdr : %d \n",err) ;
				 return ;
				}
				else
				{
				  printf("Token at %d header= [%s]\n",dIter, pToken) ;		
				}
			}
			sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
			dIter++ ;
		}
}

#endif
#endif

#ifdef SIP_SOLARIS
void csr_check_sipurl(SipMessage *pSipMsg)
{
#ifdef SIP_BY_REFERENCE
		SipHeader hdr  ;
		SipError err = E_NO_ERROR ;

		fprintf(stdout,"\n***** In csr_check_sipurl ***** \n") ;
		if(sip_getHeader(pSipMsg,SipHdrTypeFrom,&hdr,&err)==SipFail)
		{
				if (err != E_NO_EXIST)
				{
						printf("Error in Accessing From header; Error no: %d\n", err);
						return ;
				}
	}
	else
	{
    SipAddrSpec *pAddrSpec=SIP_NULL ;
	SipUrl *pSipUrl = SIP_NULL ;
	en_AddrType dAddrType ;
	SIP_S8bit *pHeader = SIP_NULL ;
		
    fprintf(stdout,"Accessed from header \n");
		/*
		 * Extract AddrSpec from From Header 
		 */
		 if (sip_getAddrSpecFromFromHdr(&hdr,&pAddrSpec,&err) == SipFail)
		 {
				 printf("Error in sip_getAddrSpecFromFromHdr = %d\n",err) ;
				 return ;
		 }
		 if (sip_getAddrTypeFromAddrSpec (pAddrSpec,&dAddrType,&err) == SipFail)
		 {
				 printf("Error in sip_getAddrTypeFromAddrSpec = %d\n",err) ;
				 sip_freeSipAddrSpec(pAddrSpec) ;
				 return ;
		 }
		 switch(dAddrType)
		 {
				 case SipAddrSipUri :
				 case SipAddrSipSUri :
						 {
								 printf("AddrType in AddrSpec = %d \n",dAddrType) ;
								if (sip_getUrlFromAddrSpec (pAddrSpec,&pSipUrl,&err) == SipFail)
								{
									printf("Error in sip_getUrlFromAddrSpec : %d\n",err) ;
									sip_freeSipAddrSpec(pAddrSpec) ;
									return ;
								}
								if (sip_getHeaderFromUrl (pSipUrl, &pHeader,&err)==SipFail )
								{
										printf("Error in sip_getHeaderFromUrl = %d \n",err) ;
										sip_freeSipUrl(pSipUrl) ;
										sip_freeSipAddrSpec(pAddrSpec) ;
										return ;
								}
								else
								{
										SIP_S8bit *pTemp = SIP_NULL ,*pValue = SIP_NULL ;
										SIP_U32bit dIndex = 0,dLen=0;
										printf("Header in SipUrl = [%s] \n",pHeader) ;
										/* At this point , headers field is stored in pHeader
										 * and extract whether the header is Replaces or not
										 * Please note that user needs to perform all the check
										 * on its own.
										 * Following code segment illustrates one way to check
										 * for Replaces header presence.
										 */
										dLen = strlen(pHeader) ;
										while ( (dIndex < dLen) && (pHeader[dIndex] != '='))
												dIndex++ ;
										if (dIndex >= dLen ) 
										{
												sip_freeSipUrl(pSipUrl) ;
												sip_freeSipAddrSpec(pAddrSpec) ;
												return ;
										}
										if ( (pTemp = (SIP_S8bit *) malloc(dIndex+1) ) == SIP_NULL)
										{
												printf("Error in allocation \n");
												sip_freeSipUrl(pSipUrl) ;
												sip_freeSipAddrSpec(pAddrSpec) ;
												return ;
										}
										strncpy(pTemp,pHeader,dIndex) ;
										pTemp[dIndex]=0 ;
										printf("pTemp = %s \n",pTemp) ;
                    if ( strcasecmp(pTemp,"Replaces") == 0 )
										{
												pValue = (SIP_S8bit*) malloc (dLen-dIndex) ;
												strncpy(pValue,pHeader+dIndex+1,dLen-dIndex) ;
												pValue[dLen-dIndex-1]=0;
												printf("Value of Replaces header = [%s]\n",
																pValue) ;
												free(pValue) ;
										}
										free(pTemp) ;
								}
								break ;
						 }
				 default :
						 {
								 break ;
						 }
		 }
		 free(pHeader) ;
		 sip_freeSipUrl(pSipUrl) ;
		 sip_freeSipAddrSpec(pAddrSpec) ;
	}
	sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
#else
	{
		SipHeader *pHdr = SIP_NULL ;
		SipError err = E_NO_ERROR ;
		SipAddrSpec *pAddrSpec=SIP_NULL ;
		SipUrl *pSipUrl = SIP_NULL ;
		en_AddrType dAddrType ;
		SIP_S8bit *pHeader = SIP_NULL ;

		fprintf(stdout,"\n***** In csr_check in NON-BY-REFERENCE ***** \n") ;
		if ( sip_initSipHeader(&pHdr,SipHdrTypeFrom,&err) ==SipFail )
		{
			printf("Error in 	sip_initSipHeader = %d \n",err) ;
		}
		if(sip_getHeader(pSipMsg,SipHdrTypeFrom,pHdr,&err)==SipFail)
		{
				if (err != E_NO_EXIST)
				{
						printf("Error in Accessing From header; Error no: %d\n", err);
						return ;
				}
		}
		 if ( sip_initSipAddrSpec(&pAddrSpec,SipAddrReqUri,&err) == SipFail )
		 {
				 printf("Error in sip_initSipAddrSpec = %d \n",err) ;
				 return ;
		 }
		 if (sip_getAddrSpecFromFromHdr(pHdr,pAddrSpec,&err) == SipFail)
		 {
				 printf("Error in sip_getAddrSpecFromFromHdr = %d\n",err) ;
				 return ;
		 }
		 if (sip_getAddrTypeFromAddrSpec (pAddrSpec,&dAddrType,&err) == SipFail)
		 {
				 printf("Error in sip_getAddrTypeFromAddrSpec = %d\n",err) ;
				 return ;
		 }
		 switch(dAddrType)
		 {
				 case SipAddrSipUri :
				 case SipAddrSipSUri :
						 {
								 printf("AddrType in AddrSpec = %d \n",dAddrType) ;
								 if ( sip_initSipUrl(&pSipUrl,&err) == SipFail )
								 {
										 printf("Error in sip_initSipUrl : %d \n",err) ;
										 return ;
								 }
								 if (sip_getUrlFromAddrSpec (pAddrSpec,pSipUrl,&err) == SipFail)
								 {
										 printf("Error in sip_getUrlFromAddrSpec : %d\n",err) ;
										 return ;
								 }
								 if (sip_getHeaderFromUrl (pSipUrl, &pHeader,&err)==SipFail )
								 {
										 printf("Error in sip_getHeaderFromUrl = %d \n",err) ;
										 return ;
								 }
								 else
								 {
										 SIP_S8bit *pTemp = SIP_NULL ,*pValue = SIP_NULL ;
										 SIP_U32bit dIndex = 0,dLen=0;
										 printf("Header in SipUrl = [%s] \n",pHeader) ;
										 /* At this point , headers field is stored in pHeader
										 * and extract whether the header is Replaces or not
										 * Please note that user needs to perform all the check
										 * on its own.
										 * Following code segment illustrates one way to check
										 * for Replaces header presence.
										 */
										dLen = strlen(pHeader) ;
										while ( (dIndex < dLen) && (pHeader[dIndex] != '='))
												dIndex++ ;
										if (dIndex >= dLen ) 
														return ;
										if ( (pTemp = (SIP_S8bit *) malloc(dIndex+1) ) == SIP_NULL)
										{
												printf("Error in allocation \n");
												return ;
										}
										strncpy(pTemp,pHeader,dIndex) ;
										pTemp[dIndex]=0 ;
										printf("pTemp = %s \n",pTemp) ;
                    if ( strcasecmp(pTemp,"Replaces") == 0 )
										{
												pValue = (SIP_S8bit*) malloc (dLen-dIndex) ;
												strncpy(pValue,pHeader+dIndex+1,dLen-dIndex) ;
												pValue[dLen-dIndex-1]=0;
												printf("Value of Replaces header = [%s]\n",
																pValue) ;
												free(pValue) ;
										}
                    free(pHeader) ;
										free(pTemp) ;
								}
								break ;
						 }
				 default :
						 {
								 break ;
						 }
		 }
		 sip_freeSipUrl(pSipUrl) ;
		 sip_freeSipAddrSpec(pAddrSpec) ;
		 sip_freeSipHeader(pHdr);
	}
		 fprintf(stdout,"***** Out csr_check ***** \n") ;
#endif
}
#endif



/* Constants and globals */

const char test_tool_desc[]="\n"
 "|----------------------------------------------------------------------|\n"
 "| SIP Stack Test Tool                                                  |\n"
 "|----------------------------------------------------------------------|\n"
 "| This test program  demonstrates the parsing  capabilities of the SIP |\n"
 "| stack. It also demonstrates the retransmission handling capabilities |\n"
 "| of the stack. This program is not a SIP user agent.                  |\n"
 "|----------------------------------------------------------------------|\n";

const char *menu = "\n\n"
  "|--- Menu ------------------------------------------------------------|\n"
  "|                                                                     |\n"
  "| z: for parser options menu      q: exit test tool                   |\n"
  "|                                                                     |\n"
  "| Options for sending requests:                                       |\n"
  "| i:INVITE    b:BYE     r:REGISTER  o:OPTIONS    k:PRACK     c:CANCEL |\n"
  "| a:ACK       p:PROPOSE f:INFO      e:REFER      v:UNKNOWN            |\n"
#ifdef SIP_IMPP
  "| u:SUBSCRIBE n:NOTIFY  m:MESSAGE                                     |\n"
#endif
  "|                                                                     |\n"
  "| Options for sending 200 OK responses:                               |\n"
  "| I:INVITE    B:BYE     R:REGISTER   O:OPTIONS   C-CANCEL   K-PRACK   |\n"
  "| F:INFO      P:PROPOSE E:REFER                                       |\n"
#ifdef SIP_IMPP
  "| U:SUBSCRIBE N:NOTIFY  M:MESSAGE                                     |\n"
#endif
  "|                                                                     |\n"
  "| Options for sending 100 Trying responses:                           |\n"
  "| Tb:BYE       Tc:CANCEL Tr:REGISTER  To:OPTIONS  Te:REFER  t:INVITE  |\n"
#ifdef SIP_IMPP
  "| Tu:SUBSCRIBE Tn:NOTIFY Tm:MESSAGE                                   |\n"
#endif
#ifdef SIP_MIB
  "|                                                                     |\n"
  "| Statistics Option (Toggle)                                          |\n"
  "| Sr:METHODS Ss: RESPONSE CLASS Sd:RESPONSE CODE                      |\n"
  "| Sc: CONFIGURE STATISTICS                                            |\n"
#endif  
  "|                                                                     |\n"
  "| Options for sending other responses for INVITE:                     |\n"
  "| 3:300  4:401  0-180  1-181  2-182  s-183                            |\n"
  "|                                                                     |\n"
  "|---------------------------------------------------------------------|\n";

int showMessage = 1;
int constructCLen;
int  glbSipOption=0;
FILE *ffp;
FILE *fp;



/*
   Modified implementation of getchar that ensures that the enter key
   pressed in response for a getchar() does not cause the next getchar()
   to return with the enter key character.fflush seems to have inconsistent
   behaviour in LINUX
*/
char sip_getchar(void)
{
	char retVal;
	retVal=getchar();
	return retVal;
}


/*
   Call back implementation
   The EventContext data structure contains data filled by the user.
   The stack does not understand the contents of this structure, but it
   often needs to release the structure. The user must implement this
   function depending on what is being passed in the pData field of
   the event-context structure.
*/
void sip_freeEventContext(SipEventContext *context)
{
	if(context->pData !=NULL)
		free(context->pData);
	if(context->pDirectBuffer !=NULL)
		free(context->pDirectBuffer);
	free(context);
}


/*=======================================================================

Selective parsing Call-backs

The following functions are callback functions that need to be implemented
only if the stack is compiled with the selective parsing option.

In this demonstration, they print out the header/SDP line when invoked.
In real applications they will contain logic to determine whether a header
needs to be parsed or to ignore errors based on the importance of the
header/SDP line in which the error occured.

========================================================================*/

/*
	Callback implementation
	This callback needs to be implemented if the stack is compiled with
	the SIP_SELECTIVE_PARSE option. This function will be called each time
	the stack is about to parse an SDP line. The application has the option
	of deciding on whether to ignore the line or modify it or parse it
	unmodified
*/
#ifdef SIP_SELECTIVE_PARSE
SIP_S8bit sip_willParseSdpLine (SIP_S8bit *in, SIP_S8bit **out)
{
/* if 0, do not parse Sdp line 0- ignore it
	if 1, and *out = SIP_NULL, parse sdp line in "in" variable
	if 1, and *out NOT = SIP_NULL, parse sdp line in "*out" variable */

	printf ("Asking permission to parse SDP line:%s\n",in);
	fflush(stdout);

	*out = SIP_NULL;
	return 1;
}
#endif

/*
	Callback implementation
	This callback needs to be implemented if the stack is compiled with
	the SIP_SELECTIVE_PARSE option. This function will be called each time
	the stack finds an SDP line with syntax errors. The application has
	the option of ignoring the bad line.
	If a bad line is ignored, it is stored in a list (slIncorrectLines)
	inside the SdpMessage structure. If the ignored line is part of a
	media description it is stored in list in the SdpMedia structure.
*/
#ifdef SIP_SELECTIVE_PARSE
SipBool sip_acceptIncorrectSdpLine (SIP_S8bit *in)
{
	/* If SipSuccess, the parser will throw away the SDP line
	If SipFail, parser will cause an error */
	printf ("\n****Asking permission to ignore BAD SDP line:%s\n",in);
	fflush(stdout);
	return SipSuccess ;
}
#endif

/*
	Callback implementation
	This callback needs to be implemented if the stack is compiled with
	the SIP_SELECTIVE_PARSE option.
	This callback is invoked each time the parser is about to parse
	a header. The application has the option of skipping the header,
	treating it as an unknown header or parsing it normally.
	Treating the header as unknown means storing the name and the unparsed
	field value in the message structure in a list of unknown headers.
	For selectively parsing headers please also refer to the new decode
	option - SIP_OPT_SELECTIVE. This allows selection using a preset list
	without the overhead of callback function invocations. Documentation
	for the new selective parsing mechanism can be found in the API reference
	under the node for SipMessage.
*/
#ifdef SIP_SELECTIVE_PARSE
SIP_S8bit sip_willParseHeader (en_HeaderType type, SipUnknownHeader *hdr)
{
	/* Valid return values:
		0 - Do not parse header - skip header entirely
		1 - Parse normally
		2 - Treat as unknown header
	*/
	int dummy;
	dummy= type;
	printf ("Asking permission to parse Header: |%s", hdr->pName);
	fflush(stdout);
	if(hdr->pBody != SIP_NULL)
	{
		printf("%s",hdr->pBody);
		fflush(stdout);
	}
	printf("|\n");
	fflush(stdout);
	return 1;
}
#endif

/*
	Callback implementation
	This callback needs to be implemented if the stack is compiled with
	the SIP_SELECTIVE_PARSE option.
	This callback is invoked each time the parser encounters an error while
	parsing a header. The application has the option of ignoring the error
	after examinig the type of the header.
	Please refer to the SIP_OPT_BADMESSAGE option for another way to ignore
	errors in messages without the use of callbacks. Documentation for this
	decode option can be found in the API reference under the node for
	SipMessage.
*/
#ifdef SIP_SELECTIVE_PARSE
SipBool sip_acceptIncorrectHeader (en_HeaderType type, SipUnknownHeader *hdr)
{
	/* Valid return values:
		SipSuccess - include incorrect header as unknown
		SipFail -  throw an error */
	int dummy;
	dummy= type;
	printf ("Asking permission to include FAILED Header %s\n", hdr->pName);
	fflush(stdout);
	return SipFail;
}
#endif

/*=======================================================================
End of selective parsing callback implementations.
========================================================================*/

/*=======================================================================
New callback introduced in 4.0.1 for notification of retransmission
events.
========================================================================*/
#ifdef SIP_RETRANSCALLBACK
#ifdef ANSI_PROTO
#ifndef SIP_TXN_LAYER
void sip_indicateMessageRetransmission (SipEventContext *pContext,\
	SipTimerKey *pKey, SIP_S8bit *pBuffer,\
	SIP_U32bit dBufferLength, SipTranspAddr *pAddr, SIP_U8bit retransCount,\
	SIP_U32bit duration)
#else
void sip_indicateMessageRetransmission (SipEventContext *pContext,\
	SipTxnKey *pKey, SIP_S8bit *pBuffer,\
	SIP_U32bit dBufferLength, SipTranspAddr *pAddr, SIP_U8bit retransCount,\
	SIP_U32bit duration)
#endif
#else
#ifndef SIP_TXN_LAYER
void sip_indicateMessageRetransmission (pContext, pKey, pBuffer, dBufferLength, pAddr, retransCount, duration)
	SipEventContext *pContext;
	SipTimerKey *pKey;
	SIP_S8bit *pBuffer;
	SIP_U32bit dBufferLength;
	SipTranspAddr *pAddr;
	SIP_U8bit retransCount;
	SIP_U32bit duration;
#else
void sip_indicateMessageRetransmission (pContext, pKey, pBuffer, dBufferLength, pAddr, retransCount, duration)
	SipEventContext *pContext;
	SipTxnKey *pKey;
	SIP_S8bit *pBuffer;
	SIP_U32bit dBufferLength;
	SipTranspAddr *pAddr;
	SIP_U8bit retransCount;
	SIP_U32bit duration;
#endif
#endif
{
	(void) pContext;
	(void) pKey;
	(void) dBufferLength;
	(void) pAddr;
	printf("\n-----Message Retransmission Indication Callback--------------\n");
	fflush(stdout);
	printf("Inside sip_indicateMessageRetransmission demo implementation.\n");
	fflush(stdout);
	printf("Buffer being retransmitted (truncated to 100 bytes):\n");
	fflush(stdout);
	printf("--------------------\n");
	fflush(stdout);
	printf("%.100s",pBuffer);
	fflush(stdout);
	printf("\n------------------");
	fflush(stdout);
	printf("\nRetransmission count: %d\n", retransCount);
	fflush(stdout);
	printf("Retransmission duration: %d\n", duration);
	fflush(stdout);
	printf("-----Message Retransmission Indication Callback Ends---------\n\n");
	fflush(stdout);
}
#endif

/*
 Decrypt buffer call-back implementation.
 This function is called by the stack when it encounters an encryption
 header in the message. In a real implementation, the encryption header will
 be extracted from the message and the required decryption algorithm will be
 applied on encinbuffer. The decrypted result would be given to encoutbuffer.
 If decryption failed for some reason, SipFail would be returned.

 In this dummy implementation, we always return a constant string containing
 two headers and an SDP body. These headers should replace the headers and
 the message body of the original message.
*/
#ifdef ANSI_PROTO
SipBool sip_decryptBuffer(SipMessage *s, SIP_S8bit *encinbuffer,\
	SIP_U32bit clen,SIP_S8bit **encoutbuffer, SIP_U32bit *outlen)
#else
SipBool sip_decryptBuffer(s, encinbuffer, clen, encoutbuffer, outlen)
	SipMessage *s;
	SIP_S8bit *encinbuffer;
	SIP_U32bit clen;
	SIP_S8bit **encoutbuffer;
	SIP_U32bit *outlen;
#endif
{
	const char *replace_string = "content-type: application/sdp\r\n"
		"m: sip:hsssip \r\n\r\n"
		"v=1\no=mephoney 29739 7272939 IN IP4 139.85.229.128\n"
		"c=IN IP4 139.85.229.168\nm=audio 492170 RTP/AVP 0 15\n"
		"m=video 3227 RTP/AVP 33\na=rtpmap:31 LPR\n\n";
	SipMessage *s_dummy;
	SIP_S8bit* encinbuffer_dummy;
	SIP_U32bit clen_dummy;

	s_dummy =s;
	encinbuffer_dummy = encinbuffer;
	clen_dummy = clen;

	*encoutbuffer = (char *) malloc(1000);
	*outlen = 193;

	printf("\n-----Decrypt Buffer Callback---------------------------------\n");
	fflush(stdout);
	printf("Inside sip_decryptBuffer demo implementation.\n");
	fflush(stdout);
	printf("Returning following string as decrypted buffer:\n\n");
	fflush(stdout);
	printf("%s",replace_string);
	fflush(stdout);
	printf("This will replace existing contact and content-type headers\n");
	fflush(stdout);
	printf("and the message body in the original message.\n");
	fflush(stdout);
	strcpy(*encoutbuffer, replace_string);
	printf("-----Decrypt Buffer Callback Ends----------------------------\n\n");
	fflush(stdout);
	return SipSuccess;
}

#ifndef SIP_TXN_LAYER
/* Structure definition for the user's timer implementation */
typedef struct
{
	SIP_S32bit duration;
	SIP_S8bit restart;
	SipBool (*timeoutfunc)(SipTimerKey *key, SIP_Pvoid buf);
	SIP_Pvoid buffer;
	SipTimerKey *key;
} TimerListElement;

SipList timerlist;

void timerElementFree(SIP_Pvoid element)
{
	TimerListElement *el;

	el = (TimerListElement *) element;
	sip_freeSipTimerKey(el->key);
	sip_freeSipTimerBuffer((SipTimerBuffer *)el->buffer);
	/* free(element);*/
}
#endif

/* Implementaion of the fast_startTimer interface reqiuired by the stack
   Application developers may choose to implement this function in any
   manner while preserving the interface
*/
#ifdef ANSI_PROTO
SipBool fast_startTimer( SIP_U32bit duration, SIP_S8bit restart,sip_timeoutFuncPtr timeoutfunc, SIP_Pvoid buffer, SipTimerKey *key, SipError *err)
#else
SipBool fast_startTimer (duration, restart, timeoutfunc, buffer, key,err)
	SIP_U32bit duration;
	SIP_S8bit restart;
	sip_timeoutFuncPtr timeoutfunc;
	SIP_Pvoid buffer;
	SipTimerKey *key;
	 SipError *err;
#endif
{
#ifndef SIP_TXN_LAYER
	SipError *dummy;
	TimerListElement *elem;
	SipError error;
	SIP_U32bit size;

	dummy = err;
	sip_listSizeOf(&timerlist,&size,&error);
	elem = (TimerListElement *) malloc(sizeof(TimerListElement));
	elem->duration = duration;
	elem->restart = restart;
	elem->timeoutfunc = timeoutfunc;
	elem->buffer = buffer;
	elem->key = key;

	sip_listAppend(&timerlist,elem,&error);
#else
	SIP_U32bit dummyDuration;
	SIP_S8bit dummyRestart;
	SIP_Pvoid dummyBuffer;
	sip_timeoutFuncPtr dummyTempfunc;
	SipTimerKey *dummyTimerKey;
	SipError *dummyError;

	dummyDuration = duration;
	dummyRestart  = restart;
	dummyBuffer   = buffer;
	dummyTimerKey = key;
	dummyTempfunc = timeoutfunc;
	dummyError	  = err;
#endif
	return SipSuccess;
}

/* Implementaion of the fast_stopTimer interface reqiuired by the stack
   Application developers may choose to implement this function in any
   manner while preserving the interface
*/

#ifdef ANSI_PROTO
SipBool fast_stopTimer(SipTimerKey *inkey, SipTimerKey **outkey, SIP_Pvoid *buffer,  SipError *err)
#else
SipBool fast_stopTimer(inkey, outkey, buffer, err)
	SipTimerKey *inkey;
	SipTimerKey **outkey;
	SIP_Pvoid *buffer;
	SipError *err;
#endif

{
#ifndef SIP_TXN_LAYER
	TimerListElement *elem;
	SipError error;
	SIP_U32bit i;
	SIP_U32bit size;

	sip_listSizeOf(&timerlist,&size,&error);
	for(i=0; i<size; i++)
	{
		sip_listGetAt(&timerlist, (SIP_U32bit) i, (SIP_Pvoid *) &elem, &error);
		if(sip_compareTimerKeys(inkey,elem->key,err)==SipSuccess)
		{
			*outkey = elem->key;
			*buffer = elem->buffer;
			sip_listDeleteAt(&timerlist, (SIP_U32bit) i , &error);
			return SipSuccess;
		}
	}
#else
	SipTimerKey *dummyInkey, *dummyOutkey;
	SIP_Pvoid *dummyBuffer;
	SipError *dummyErr;

	dummyInkey = inkey;
	dummyOutkey = *outkey;
	dummyBuffer = buffer;
	dummyErr    = err;

#endif

	return SipFail;
}

#ifndef SIP_TXN_LAYER
/* Function to get the minimum timout value from the timer list */
SIP_S32bit getMinimumTimeout(SipList *list)
{
	TimerListElement *elem;
	SipError error;
	SIP_U32bit size,i;
	SIP_S32bit minval;
	SipList *dummy;
	dummy = list;

	minval = 1000;
	sip_listSizeOf(&timerlist,&size,&error);
	for(i=0; i<size; i++)
	{
		sip_listGetAt(&timerlist, (SIP_U32bit) i, (SIP_Pvoid *) &elem, &error);
		if(elem->duration < minval)
			minval = elem->duration;
	}
	return minval;
}
#endif
/* Implementaion of the sendToNetwork interface reqiuired by the stack
   This implementation does not respect the transptype parameter
   The buffer is dispatched via UDP irrespective of the transptype value
   Application developers may choose to implement this function in any
   manner while preserving the interface
*/

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
	int	sockfd;

	int len;

	struct sockaddr_in	cli_addr, serv_addr;
	SIP_S8bit dummy;
	SipError *err_dummy;

	dummy = transptype;
	err_dummy = err;

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family	  = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(addr->dIpv4);
	serv_addr.sin_port	  = htons(addr->dPort);

	if((transptype&SIP_UDP) == SIP_UDP)
	{
		if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			printf("Client : Could not open datagram socket.\n");
			fflush(stdout);
			close(sockfd);
			exit(0);
		}
	}
	else if((transptype&SIP_TCP) == SIP_TCP)
	{
		if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Client : Could not open stream socket.\n");
			fflush(stdout);
			close(sockfd);
			exit(0);
		}
	}
	else
	{
			sockfd=0;
			printf("No Transport Type specified\n");
			fflush(stdout);
			exit(0);
	}

	bzero((char *) &cli_addr, sizeof(cli_addr));	/* zero out */
	cli_addr.sin_family	 = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port	 = htons(0);

	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
	{
		printf("Client : Could not bind to local address.\n");
		fflush(stdout);
		close(sockfd);
		exit(0);
	}
	if((transptype&SIP_UDP) == SIP_UDP)
	{
		len=sendto(sockfd, buffer, buflen, 0, (struct sockaddr*)\
			&serv_addr, sizeof(serv_addr));
		if(len == -1 || (unsigned int)len != buflen)
		{
			printf("Client : send to failed.\n");
			fflush(stdout);
#ifdef SIP_WINDOWS
			closesocket(sockfd);
#else
			close(sockfd);
#endif
			return SipFail;
		}
	}
	else if((transptype&SIP_TCP) == SIP_TCP)
	{
		if (connect(sockfd, (struct sockaddr *)&serv_addr,
			sizeof(serv_addr)) < 0)
		{
			printf("Client : connect failed.\n");
			fflush(stdout);
			close(sockfd);
			return SipFail;
		}
		if(write(sockfd,buffer,buflen) != (SIP_S32bit)buflen)
		{
			printf("Client : send to failed.\n");
			fflush(stdout);
			close(sockfd);
			return SipFail;
		}
	}
		close(sockfd);

	return SipSuccess;
}
#ifdef SIP_TEL
#ifndef SIP_BY_REFERENCE
SipBool do_stuff_tel_nonbyref(SipMessage *s)
{
	SipHeader *hdr;
	SipAddrSpec *addrspec;
	TelUrl *telUrl;
	SipError err;

	/*****handling the from header*****/
	/*initialize the from header*/
	if(sip_initSipHeader(&hdr,SipHdrTypeFrom,&err)==SipFail)
	{
		printf("##TEL:##Error in initialising FROM Header; ERRNO::%d\n",err);
		fflush(stdout);
		return SipFail;
	}
	/*get the FROM header from the message*/
	if(sip_getHeader(s,SipHdrTypeFrom,hdr,&err)==SipFail)
	{
		printf("##TEL:##Error in accessing FROM Header; ERRNO::%d\n",err);
		fflush(stdout);
		return SipFail;
	}

	if(sip_initSipAddrSpec(&addrspec,SipAddrReqUri,&err)==SipFail)
	{
		printf("##TEL:##Error in initialising AddrSpec in From Header; \
			 ERRNO::%d\n",err);
		fflush(stdout);
		sip_freeSipHeader(hdr);
		return SipFail;
	}
	if(sip_getAddrSpecFromFromHdr(hdr,addrspec,&err)==SipFail)
	{
		printf("##TEL:##Error in accessing AddrSpec from FROM Header;NON BY REF \
		 ERRNO::%d\n",err);
		fflush(stdout);
		sip_freeSipHeader(hdr);
		sip_freeSipAddrSpec(addrspec);
		return SipFail;
	}
	if(SipSuccess == sip_isTelUrl(addrspec,&err))
	{
		if((sip_initTelUrl(&telUrl,&err))==SipFail)
		{
				printf("##TEL:##Error in initializing TelUrl from AddrSpec \
					 NON BY REF in FROMHeader; ERRNO::%d\n",err);
				fflush(stdout);
				sip_freeSipAddrSpec(addrspec);
				sip_freeSipHeader(hdr);
				return SipFail;

		}
		if(sip_getTelUrlFromAddrSpec(addrspec,telUrl,&err)==SipFail)
			{
				printf("##TEL:##Error in accessing TelUrl from AddrSpec NON BY REF \
				 in FROMHeader; ERRNO::%d\n",err);
				fflush(stdout);
				sip_freeSipAddrSpec(addrspec);
				sip_freeSipHeader(hdr);
				sip_freeTelUrl(telUrl);
				return SipFail;
			}
		else
			{
				printf("##TEL:##Retrieved TEL URL from AddrSpec in \
				FROM Header---NON BY REF\n");
				fflush(stdout);
				if(sip_setTelUrlInAddrSpec(addrspec,telUrl,&err)==SipFail)
				{
					printf("##TEL:##Error in setting the TEL URL in AddrSpec in\
					 FROM Header---NON BY REF; ERRNO::%d\n",err);
					fflush(stdout);
					sip_freeTelUrl(telUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipHeader(hdr);
					return SipFail;

				}
				else
				{
					printf("##TEL:##Set the TEL URL in AddrSpec in FROM Header\
					---NON BY REF\n");
					fflush(stdout);
					sip_freeTelUrl(telUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipHeader(hdr);
				}
			}
		printf("##TEL:##Parsed Tel Url successfully\n");
		fflush(stdout);
	}
	else
	{
		sip_freeSipAddrSpec(addrspec);
		sip_freeSipHeader(hdr);
	}
 return SipSuccess;
}
#else
/*Fucntion called inside displayParsedMessage () to check the parsing of tel url structure*/

SipBool do_stuff_tel(SipMessage *s)
{
	SipHeader hdr;
	SipAddrSpec *addrspec;
	TelUrl *telUrl;
	SipError err;
/* handling for from header */
	if(sip_getHeader(s,SipHdrTypeFrom,&hdr,&err)==SipFail)
	{
		if (err != E_NO_EXIST)
		{
			printf("##TEL:##Error in Accessing From header; Error no: %d\n", err);
			fflush(stdout);
			return SipFail;
		}
	}
	else
	{
		if(sip_getAddrSpecFromFromHdr(&hdr,&addrspec,&err)==SipFail)
		{
			printf("##TEL:##Error in accessing AddrSpec from FROM Header; ERRNO::%d\n",err);
			fflush(stdout);
			sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
			return SipFail;
		}
		if(SipSuccess == sip_isTelUrl(addrspec,&err))
		{
			if(sip_getTelUrlFromAddrSpec(addrspec,&telUrl,&err)==SipFail)
			{
				printf("##TEL:##Error in accessing TelUrl from AddrSpec in FROMHeader; ERRNO::%d\n",err);
				fflush(stdout);
				sip_freeSipAddrSpec(addrspec);
				sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
				return SipFail;
			}
			else
			{
				printf("##TEL:##Retrieved TEL URL from AddrSpec in FROM Header\n");
				fflush(stdout);
				if(sip_setTelUrlInAddrSpec(addrspec,telUrl,&err)==SipFail)
				{
					printf("##TEL:##Error in setting the TEL URL in AddrSpec in FROM Header; ERRNO::%d\n",err);
					fflush(stdout);
					sip_freeTelUrl(telUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
					return SipFail;

				}
				else
				{
					printf("##TEL:##Set the TEL URL in AddrSpec in FROM Header\n");
					fflush(stdout);
					sip_freeTelUrl(telUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
				}
			}
			printf("##TEL:##Parsed Tel Url successfully\n");
			fflush(stdout);
		}
		else
		{
			sip_freeSipAddrSpec(addrspec);
			sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
		}
	}
	return SipSuccess;
}
#endif
#endif

/* Parsing IM-URL  */

#ifdef SIP_IMPP
#ifndef SIP_BY_REFERENCE
SipBool do_stuff_imurl_nonbyref(SipMessage *s)
{
	SipHeader *hdr;
	SipAddrSpec *addrspec;
	ImUrl *pImUrl;
	SipError err;

	/*****handling the from header*****/
	/*initialize the from header*/
	if(sip_initSipHeader(&hdr,SipHdrTypeFrom,&err)==SipFail)
	{
		printf("##IM-URL:##Error in initialising FROM Header; ERRNO::%d\n",err);
		fflush(stdout);
		return SipFail;
	}
	/*get the FROM header from the message*/
	if(sip_getHeader(s,SipHdrTypeFrom,hdr,&err)==SipFail)
	{
		printf("##IM-URL:##Error in accessing FROM Header; ERRNO::%d\n",err);
		fflush(stdout);
		return SipFail;
	}

	if(sip_initSipAddrSpec(&addrspec,SipAddrReqUri,&err)==SipFail)
	{
		printf("##IM-URL:##Error in initialising AddrSpec in From Header; \
			 ERRNO::%d\n",err);
		fflush(stdout);
		sip_freeSipHeader(hdr);
		return SipFail;
	}
	if(sip_getAddrSpecFromFromHdr(hdr,addrspec,&err)==SipFail)
	{
		printf("##IM-URL:##Error in accessing AddrSpec from FROM Header;NON BY REF \
		 ERRNO::%d\n",err);
		fflush(stdout);
		sip_freeSipHeader(hdr);
		sip_freeSipAddrSpec(addrspec);
		return SipFail;
	}
	if(SipSuccess == sip_isImUrl(addrspec,&err))
	{
		if((sip_initImUrl(&pImUrl,&err))==SipFail)
		{
				printf("##IM-URL:##Error in initializing ImUrl from AddrSpec \
					 NON BY REF in FROMHeader; ERRNO::%d\n",err);
				fflush(stdout);
				sip_freeSipAddrSpec(addrspec);
				sip_freeSipHeader(hdr);
				return SipFail;

		}
		if(sip_getImUrlFromAddrSpec(addrspec,pImUrl,&err)==SipFail)
			{
				printf("##IM-URL:##Error in accessing ImUrl from AddrSpec NON BY REF \
				 in FROMHeader; ERRNO::%d\n",err);
				fflush(stdout);
				sip_freeSipAddrSpec(addrspec);
				sip_freeSipHeader(hdr);
				sip_freeImUrl(pImUrl);
				return SipFail;
			}
		else
			{
				printf("##IM-URL:##Retrieved IM URL from AddrSpec in \
				FROM Header---NON BY REF\n");
				fflush(stdout);
				if(sip_setImUrlInAddrSpec(addrspec,pImUrl,&err)==SipFail)
				{
					printf("##IM-URLL:##Error in setting the IM URL in AddrSpec in\
					 FROM Header---NON BY REF; ERRNO::%d\n",err);
					fflush(stdout);
					sip_freeImUrl(pImUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipHeader(hdr);
					return SipFail;
				}
				else
				{
					printf("##IM-URL:##Set the IM URL in AddrSpec in FROM Header\
					---NON BY REF\n");
					fflush(stdout);
					sip_freeImUrl(pImUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipHeader(hdr);
				}
			}
		printf("##IM-URL:##Parsed IM URL successfully\n");
		fflush(stdout);
	}
	else
	{
		sip_freeSipAddrSpec(addrspec);
		sip_freeSipHeader(hdr);
	}
 return SipSuccess;
}
#else
/*Fucntion called inside displayParsedMessage () to check the parsing of im-url structure BY_REF case */

SipBool do_stuff_imurl_byref(SipMessage *s)
{
	SipHeader hdr;
	SipAddrSpec *addrspec;
	ImUrl *pImUrl;
	SipError err;
/* handling for from header */
	if(sip_getHeader(s,SipHdrTypeFrom,&hdr,&err)==SipFail)
	{
		if (err != E_NO_EXIST)
		{
			printf("##IM-URL:##Error in Accessing From header; Error no: %d\n", err);
			fflush(stdout);
			return SipFail;
		}
	}
	else
	{
		if(sip_getAddrSpecFromFromHdr(&hdr,&addrspec,&err)==SipFail)
		{
			printf("##IM-URL:##Error in accessing AddrSpec from FROM Header; ERRNO::%d\n",err);
			fflush(stdout);
			sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
			return SipFail;
		}
		if(SipSuccess == sip_isImUrl(addrspec,&err))
		{
			if(sip_getImUrlFromAddrSpec(addrspec,&pImUrl,&err)==SipFail)
			{
				printf("##IM-URL:##Error in accessing ImUrl from AddrSpec in FROMHeader; ERRNO::%d\n",err);
				fflush(stdout);
				sip_freeSipAddrSpec(addrspec);
				sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
				return SipFail;
			}
			else
			{
				printf("##IM-URL:##Retrieved IM URL from AddrSpec in FROM Header\n");
				fflush(stdout);
				if(sip_setImUrlInAddrSpec(addrspec,pImUrl,&err)==SipFail)
				{
					printf("##IM-URL:##Error in setting the TIM URL in AddrSpec in FROM Header; ERRNO::%d\n",err);
					fflush(stdout);
					sip_freeImUrl(pImUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
					return SipFail;

				}
				else
				{
					printf("##IM-URL:##Set the IM URL in AddrSpec in FROM Header\n");
					fflush(stdout);
					sip_freeImUrl(pImUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
				}
			}
			printf("##IM-URL:##Parsed IM URL successfully\n");
			fflush(stdout);
		}
		else
		{
			sip_freeSipAddrSpec(addrspec);
			sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
		}
	}
	return SipSuccess;
}
#endif
#endif
/* end of parsing IM-URL */

#ifdef SIP_PRES
#ifdef SIP_BY_REFERENCE
SipBool do_stuff_presurl_byref(SipMessage *s)
{
	SipHeader hdr;
	SipAddrSpec *addrspec;
	PresUrl *pPresUrl = SIP_NULL ;
	SipError err=E_NO_ERROR ;
/* handling for from header */
	if(sip_getHeader(s,SipHdrTypeFrom,&hdr,&err)==SipFail)
	{
		if (err != E_NO_EXIST)
		{
			printf("##PRES-URL:##Error Accessing From header; Error No: %d\n", err);
			fflush(stdout);
			return SipFail;
		}
	}
	else
	{
		if(sip_getAddrSpecFromFromHdr(&hdr,&addrspec,&err)==SipFail)
		{
			printf("##PRES-URL:##Error accessing AddrSpec from FROM Header; Error No:%d\n",err);
			fflush(stdout);
			sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
			return SipFail;
		}
		if(SipSuccess == sip_isPresUrl(addrspec,&err))
		{
			if(sip_getPresUrlFromAddrSpec(addrspec,&pPresUrl,&err)==SipFail)
			{
				printf("##PRES-URL:##Error accessing PresUrl from AddrSpec in FROMHeader; Error no::%d\n",err);
				fflush(stdout);
				sip_freeSipAddrSpec(addrspec);
				sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
				return SipFail;
			}
			else
			{
				printf("##PRES-URL:##Retrieved PRES URL from AddrSpec in FROM Header\n");
				fflush(stdout);
				if(sip_setPresUrlInAddrSpec(addrspec,pPresUrl,&err)==SipFail)
				{
					printf("##PRES-URL:##Error in setting the PRES-URL in AddrSpec in FROM Header; Error no::%d\n",err);
					fflush(stdout);
					sip_freePresUrl(pPresUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
					return SipFail;

				}
				else
				{
					printf("##PRES-URL:##Set the PRES URL in AddrSpec in FROM Header\n");
					fflush(stdout);
					sip_freePresUrl(pPresUrl);
					sip_freeSipAddrSpec(addrspec);
					sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
				}
			}
			printf("##PRES-URL:##Parsed PRES-URL successfully\n");
			fflush(stdout);
		}
		else
		{
			printf("\nNo Pres Url present in from header \n") ;	
			sip_freeSipAddrSpec(addrspec);
			sip_freeSipFromHeader((SipFromHeader *)(hdr.pHeader));
		}
	}
	return SipSuccess;
}
#endif
#endif

/* end of parsing IM-URL */



void headerListFreeFunction(void *pData);
void headerListFreeFunction(void *pData)
{
	sip_freeSipHeader((SipHeader *)pData);
#ifdef SIP_BY_REFERENCE
	fast_memfree(0,pData,SIP_NULL);
#endif
}

#ifdef SIP_BY_REFERENCE
SipBool extractAuthParamFromChallengeAtIndex(SipGenericChallenge *pChal,
								SIP_U32bit dCount, SipError *pErr)
{
    SipParam *pParam = SIP_NULL ;
    SIP_S8bit *pName = SIP_NULL, *pValue = SIP_NULL ;
    SIP_U32bit dIter=0,dValCount=0 ;

	  if ( sip_getAuthorizationParamAtIndexFromChallenge(pChal,&pParam,
															dCount,pErr) == SipFail )
    {
       printf("\nError in sip_getAuthorizationParamAtIndexFromChallenge :%d\n",
                            *pErr) ;
       return SipFail ;
    }
    else 		
    {
       printf("\n -- Extracted Parameter from SipGenericChallenge --\n") ;
       if ( sip_getNameFromSipParam(pParam,&pName,pErr) == SipFail )
       {
           printf("\nError in sip_getNameFromSipParam : %d \n",*pErr) ;
       }   
       else
       {
          printf("Name in SIP-Param : %s \n",pName) ;
       }
       if ( sip_getValueCountFromSipParam(pParam,&dValCount,pErr) == SipFail )
       {
          printf("\nError in sip_getNameFromSipParam : %d \n",*pErr) ;
       }
      else
      {
         printf("Value Count : %d \n",dValCount) ;
         while ( dIter < dValCount )
         {
            pValue = SIP_NULL ;
           if ( sip_getValueAtIndexFromSipParam(pParam,&pValue,dIter,pErr) ==
                          SipFail )
           {
								 printf("\nError in sip_getValueAtIndexFromSipParam : %d \n"
																 ,*pErr) ;
           }                  
           else
           {
							 printf("Value [%d] in SIP-Param : %s \n",dIter,pValue) ;
           }                  
           dIter ++ ; 
         } 
       }
					   
      }
    sip_freeSipParam(pParam) ;
	return SipSuccess ;
}
#endif



/* Function called inside all callbacks to print out
   the message received. The message is available in the decoded form
   to the callbacks. This function converts the structure back into
   a text message and prints it out.
*/
void displayParsedMessage(SipMessage *s)
{
	char *out;
	char fullForm;
	SipError err;
	SipBool res;
	SIP_U32bit length;
	SipOptions options;
#ifdef SIP_TEL
	SipBool restel;
#endif

#ifdef SIP_IMPP
	SipBool resimurl;
#endif

	
	out = (char *) malloc(6000);

	if (out == NULL )
	{
		printf ("Memory allocation error\n");
		exit(0);
	}

	options.dOption =SIP_OPT_SINGLE | SIP_OPT_AUTHCANONICAL;
	if(glbSipOption==1)
	{
		options.dOption = 0;
		fflush(stdin);
		printf("\n Use short form/full form for headers in formed message ?(s/f):[f]");
		fflush(stdout);
		fullForm=sip_getchar();
		if((fullForm=='s') || (fullForm == 'S'))
			options.dOption |=SIP_OPT_SHORTFORM;
		else
			options.dOption |=SIP_OPT_FULLFORM;
		fflush(stdin);
		printf("\n Use comma separated headers ?(y/n): [n] ");
		fflush(stdout);
		fullForm=sip_getchar();
		if((fullForm=='y') || (fullForm == 'Y'))
			options.dOption |=SIP_OPT_COMMASEPARATED;
		else
			options.dOption |=SIP_OPT_SINGLE;
		fflush(stdin);
		printf("\n Enable content-length correction ?(y/n): [y] ");
		fflush(stdout);
		fullForm=sip_getchar();
		if((fullForm=='n') || (fullForm == 'N'))
		{}
		else
			options.dOption |=SIP_OPT_CLEN;
	}
	glbSipOption=0;
#ifdef SIP_TEL
	/*testing of the tel-url structure*/
#ifdef SIP_BY_REFERENCE
	printf("*****Testing The TEL-URL BY REFERNCE MODE*****\n");
	fflush(stdout);
	restel=do_stuff_tel(s);
#else
	printf("*****Testing The TEL-URL NON BY REFERNCE MODE*****\n");
	fflush(stdout);
	restel=do_stuff_tel_nonbyref(s);
#endif
	if(restel!=SipSuccess)
	{
		printf("##TEL:##Error in parsing Tel Url\n");
		fflush(stdout);
		free(out);
		sip_freeSipMessage(s);
		return;
	}
#endif
		

#ifdef SIP_IMPP
	/*testing of the im-url structure*/
#ifdef SIP_BY_REFERENCE
	printf("*****Testing The IM-URL BY REFERNCE MODE*****\n");
	fflush(stdout);
	resimurl=do_stuff_imurl_byref(s);
#else
	printf("*****Testing The IM-URL NON BY REFERNCE MODE*****\n");
	fflush(stdout);
	resimurl=do_stuff_imurl_nonbyref(s);
#endif
	if(resimurl!=SipSuccess)
	{
		printf("##IM-URL:##Error in parsing IM-URL\n");
		fflush(stdout);
		free(out);
		sip_freeSipMessage(s);
		return;
	}
#endif

#ifdef SIP_PRES
	{
		SipBool retVal = SipSuccess ;	
#ifdef SIP_BY_REFERENCE
			printf("*****Testing PRES-URL BY REFERNCE MODE*****\n");
			fflush(stdout);
			retVal=do_stuff_presurl_byref(s);
#else
			printf("*****Testing PRES-URL NON BY REFERNCE MODE*****\n");
			fflush(stdout);
			/*
				 resimurl=do_stuff_presurl_nonbyref(s);
			 */
#endif/*SIP_BY_REFERENCE*/
			if(retVal !=SipSuccess)
			{
					printf("##PRES-URL:##Error in parsing PRES-URL\n");
					fflush(stdout);
					free(out);
					sip_freeSipMessage(s);
					return;
			}
			else
			{
					printf("##PRES-URL:## Successfully parsed PRES-URL\n");
			}
	}
#endif /*#ifdef SIP_PRES*/

#ifdef SIP_SOLARIS
	{
		SipError errAuth = E_NO_ERROR ;
		SIP_U32bit count=0; 
#ifdef SIP_BY_REFERENCE
		SipHeader dHdr ;
		SipGenericChallenge* pChallenge = SIP_NULL ;
#endif

		/*-------------------------------------
       Extract Www-Authenticate Header
     ------------------------------------*/

		if(sip_getHeaderCount(s, SipHdrTypeWwwAuthenticate, &count, &errAuth)==SipFail)
		{
			printf("Error in getting Header Count : %d \n",errAuth) ;
		}

		if ( count > 0 )
		{	
			printf("\n\nCount of www-authenticate header : %d \n",count) ;
        /* 
         * Get WwwAuthenticate Header  
         */
#ifdef SIP_BY_REFERENCE
				if(sip_getHeaderAtIndex(s, SipHdrTypeWwwAuthenticate, &dHdr, 0,\
																&errAuth)==SipFail)
				{
						printf("Error in getting Sip Header : %d \n",errAuth) ;
				}
			  else
				{
					printf("\nGot Www-Authenticate header successfully \n") ;
				}

        /*
         * Extract Challenge from Www-Authenticate Header
         */       
				 if ( sip_getChallengeFromWwwAuthenticateHdr(&dHdr,&pChallenge,&errAuth) 
								== SipFail )
				 {
						printf("\nError in sip_getChallengeFromWwwAuthenticateHdr : %d \n",
														errAuth) ;
				 }
				 else
				 {
							SIP_S8bit *pScheme = SIP_NULL ;
              SIP_U32bit dIter=0,dAuthCount = 0 ;

							if (sip_getSchemeFromChallenge(pChallenge,&pScheme,&errAuth)==SipFail)
							{
                  printf("\nError in sip_getSchemeFromChallenge : %d \n,",errAuth) ;
							}
              else
              {
                  printf("\npScheme in Challenge : [%s]  \n",pScheme) ;
              }     
              if (	sip_getAuthorizationParamCountFromChallenge(pChallenge,
                       &dAuthCount,&errAuth) == SipFail )
              {
                   printf("\nError in "
										"sip_getAuthorizationParamCountFromChallenge: %d \n",errAuth) ;
              } 
              else
              {
                printf("Count of Authorization Parameters = %d \n",dAuthCount) ;
                while (dIter < dAuthCount )
								{
									extractAuthParamFromChallengeAtIndex(pChallenge,dIter,&errAuth);
                  dIter++ ;
								}
              } 
				} /* else */
			  sip_freeSipGenericChallenge(pChallenge) ;
				sip_freeSipHeader(&dHdr);
#endif
		} /* if (count > 0 )*/
	}/*ifdef SOLARIS */
#endif

	res = sip_formMessage( s, &options, out, &length, &err);
	if (res != SipSuccess )
	{
		printf ("\nError in SipFormMessage: %d\n", err);
		free(out);
		sip_freeSipMessage(s);
		return;
	}
	if (showMessage)
		printf ("\n|==Decoded Message====================================="
			"========================|\n\n");




#ifdef SIP_SOLARIS
	out[length] = '\0';
	printf("%s",out);
	fflush(stdout);
#endif


	printf ("\n|==Decoded Message Ends================================"
			"========================|\n");

	/*=========================================================*/
	/* Parsing unrecognized header types using grammars of     */
	/* recognized type                                         */
	/*                                                         */
	/* This example shows Diversion header parsing.            */
	/* The SIP stack does not support the Diversion header     */
	/* defined in draft-levy-sip-diversion-01.                 */
	/* Since the Diversion header grammar is similar to the    */
	/* Route header grammar, the new stack API for parsing     */
	/* unknown headers can be used as shown below to parse     */
	/* and form this header with ease.                         */
	/*=========================================================*/

	do
	{
#ifdef SIP_BY_REFERENCE
		SipHeader dHeader;
#else
		SipHeader *pHeader;
#endif
		SIP_U32bit i,count;

		/* Check if the message has any headers that are not understood */
		if(sip_getHeaderCount(s, SipHdrTypeUnknown, &count, &err)==SipFail)
			break;
		for(i=0;i<count;i++)
		{
			SIP_S8bit *pUnkHdrName;
#ifdef SIP_BY_REFERENCE
			if(sip_getHeaderAtIndex(s, SipHdrTypeUnknown, &dHeader, i,\
				&err)==SipFail)
#else
			if(sip_initSipHeader(&pHeader, SipHdrTypeUnknown, &err)==SipFail)
				break;
			if(sip_getHeaderAtIndex(s, SipHdrTypeUnknown, pHeader, i,\
				&err)==SipFail)
#endif
				continue;
			/* Get the name of the unknown header */
#ifdef SIP_BY_REFERENCE
			if(sip_getNameFromUnknownHdr(&dHeader, &pUnkHdrName, &err)==SipFail)
			{
				sip_freeSipHeader(&dHeader);
				continue;
			}
#else
			if(sip_getNameFromUnknownHdr(pHeader, &pUnkHdrName, &err)==SipFail)
			{
				sip_freeSipHeader(pHeader);
				continue;
			}
#endif

			printf("\nGot unknown header by Name of %s",pUnkHdrName);
			/* Check if it is a Diversion header */
			if(strcasecmp(pUnkHdrName,"Diversion")==0)
			{
				/* Diversion header found in the message */
				SipList parsedHeaderList;

				printf("\nFound Diversion header in the message\n");
				fflush(stdout);
				/* Initialize the list that will contain the parsed headers */
				if(sip_listInit(&parsedHeaderList, headerListFreeFunction,\
					&err)==SipFail)
				{
#ifdef SIP_BY_REFERENCE
					sip_freeSipHeader(&dHeader);
#else
					sip_freeSipHeader(pHeader);
#endif
					continue;
				}
				/* Parse the unknown header using the Route header's parser */
#ifdef SIP_BY_REFERENCE
				if(sip_parseUnknownHeader(&dHeader, SipHdrTypeRoute,\
					&parsedHeaderList, &err)!=SipFail)
#else
				if(sip_parseUnknownHeader(pHeader, SipHdrTypeRoute,\
					&parsedHeaderList, &err)!=SipFail)
#endif
				{
					SIP_U32bit fooIterator, fooCount;
					printf("Parsed Diversion header\n");
					fflush(stdout);
					/* There could have been more than one header in a line
					   separated by commas. The Diversion header grammar
					   supports that.
					*/
					sip_listSizeOf(&parsedHeaderList, &fooCount, &err);
					printf("Found %d Diversion headers in one line\n",fooCount);
					fflush(stdout);
					for(fooIterator=0;fooIterator<fooCount;fooIterator++)
					{
						SipHeader *pDiversionHeader;
						SipAddrSpec *pAddrSpec;
						en_AddrType type;

						sip_listGetAt(&parsedHeaderList, fooIterator,\
							(SIP_Pvoid*)&pDiversionHeader, &err);
						/* The Header structure retrieved will be a route
						   header structure. Use the route header accessor
						   API to get the DIversion header's elements
						*/
#ifndef SIP_BY_REFERENCE
						sip_initSipAddrSpec(&pAddrSpec, SipAddrAny, &err);
#endif
#ifdef SIP_BY_REFERENCE
						sip_getAddrSpecFromRouteHdr(pDiversionHeader,\
							&pAddrSpec, &err);
#else
						sip_getAddrSpecFromRouteHdr(pDiversionHeader,\
							pAddrSpec, &err);
#endif
						sip_getAddrTypeFromAddrSpec(pAddrSpec, &type, &err);
						if(type == SipAddrReqUri)
						{
							SIP_S8bit *pNonSipUri;
							sip_getUriFromAddrSpec(pAddrSpec,&pNonSipUri,&err);
							printf("Diversion header with non-SIP URL.\n");
							fflush(stdout);
							printf("URL retrieved : %s\n", pNonSipUri);
							fflush(stdout);
#ifndef SIP_BY_REFERENCE
							fast_memfree(0,pNonSipUri, SIP_NULL);
#endif
						}
						else if(type == SipAddrSipUri)
						{
							SipUrl *pUrl;
							SIP_S8bit *pHost;

#ifdef SIP_BY_REFERENCE
							sip_getUrlFromAddrSpec(pAddrSpec, &pUrl, &err);
#else
							sip_initSipUrl(&pUrl, &err);
							sip_getUrlFromAddrSpec(pAddrSpec, pUrl, &err);
#endif
							sip_getHostFromUrl(pUrl, &pHost, &err);
							printf("Diversion header with SIP URL.\n");
							fflush(stdout);
							printf("Host retrieved from URL : %s\n", pHost);
							fflush(stdout);
#ifndef SIP_BY_REFERENCE
							fast_memfree(0, pHost, SIP_NULL);
#endif
							sip_freeSipUrl(pUrl);
						}
						sip_freeSipAddrSpec(pAddrSpec);
						/* Other elements of the diversion header like the
						   header parameters can be retrieved using the
						   API used to get parameters from route headers */
					}
					sip_listDeleteAll(&parsedHeaderList,&err);
				}
#ifdef SIP_BY_REFERENCE
				sip_freeSipHeader(&dHeader);
#else
				sip_freeSipHeader(pHeader);
#endif
			}
			else
#ifdef SIP_BY_REFERENCE
				sip_freeSipHeader(&dHeader);
#else
				sip_freeSipHeader(pHeader);
			fast_memfree(0, pUnkHdrName, SIP_NULL);
#endif
		}
	} while(0);
	showErrorInformation(s);
	free (out);
	sip_freeSipMessage(s);
/*	fast_memfree(0,s,SIP_NULL);*/
}

/* These are the callbacks which which the stack calls when
   it successfully decodes a SIP message */
#ifdef SIP_TXN_LAYER
void sip_freeTimerHandle(SIP_Pvoid	pTimerHandle)
{
	/*Making this a dummy function since the actual freeing of the
	 * timer handle is being handled thru the application itself.
	 */
	(void)pTimerHandle;
}

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
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside retransmission timeout callback------\n");
	if(context->pData)
	printf ("%s timed out.\n",(char *)context->pData);
#ifdef SIP_TXN_LAYER
	switch(dTimer)
	{
		case SipTimerA_B:
			printf("***TimerB has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerB:
			printf("***TimerB has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerC:
			printf("***TimerC has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerCr:
			printf("***TimerCr has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerD:
			printf("***TimerD has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerE_F:
			printf("***TimerF has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerF:
			printf("***TimerF has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerG_H:
			printf("***TimerH has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerH:
			printf("***TimerH has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerI:
			printf("***TimerI has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerJ:
			printf("***TimerJ has been TimedOut******\n");
			fflush(stdout);
			break;
		case SipTimerK:
			printf("***TimerK has been TimedOut******\n");
			fflush(stdout);
			break;
		default:
			printf("***Unknown Timer******\n");
			fflush(stdout);
	}
	if (dTimer!=SipTimerC)	
#endif
	sip_freeEventContext(context);
}

#ifndef SIP_NO_CALLBACK
void sip_indicatePublish(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;/* to remove warning */
	printf ("Inside PUBLISH callback------\n");
	dummy = context;
	displayParsedMessage(s);
}
		
void sip_indicateInvite(SipMessage *s, SipEventContext *context)
{
	SipError err;
	SIP_U32bit count;
	SipEventContext *dummy;
	dummy = context;

	printf ("Inside INVITE callback------\n");

	/* Following code demonstrates accessor API usage to access elements
	   from the Expires header if one is found in the INVITE message */
	/* Check if the message has an Expires header */

	if(sip_getHeaderCount(s,SipHdrTypeExpiresAny, &count, &err) == SipSuccess)
	{
#ifdef SIP_BY_REFERENCE
		if(count==1)
		{
			SipHeader *hdr;
			SipDateStruct *datestruct;
			en_ExpiresType etype;
			SIP_U32bit seconds;

			printf("Message contains Expires header.\n");
			fflush(stdout);
			/* SipHeader initialized as type SipHdrTypeAny for references */
			if(sip_initSipHeader(&hdr,SipHdrTypeAny,&err)!=SipSuccess)
			{
				printf("Error in initializing Expires Header\n");
				fflush(stdout);
				return;
			}
			if(sip_getHeader(s,SipHdrTypeExpiresAny,hdr,&err)!=SipSuccess)
			{
				printf("Error in accessing Expires Header\n");
				fflush(stdout);
				sip_freeSipHeader(hdr);
				return;
			}
			if(sip_getTypeFromExpiresHdr(hdr,&etype,&err)!=SipSuccess)
			{
				printf("Error in accessing Type from Expires Header\n");
				fflush(stdout);
				sip_freeSipHeader(hdr);
				return;
			}
			switch(etype)
			{
				case SipExpDate:
					/* No init for structure required in reference mode */
					/* get function takes pointer to pointer */
					if(sip_getDateStructFromExpiresHdr(hdr,&datestruct,&err)\
						==SipSuccess)
					{
						/* Get dateformat and timeformat from the datestruct */
						SipDateFormat *dformat;
						SipTimeFormat *tformat;
						SIP_U16bit year,month;
						SIP_U8bit date;
						SIP_S8bit hour,minute,sec;
						en_Month emonth;

						sip_getDateFormatFromDateStruct(datestruct,&dformat,\
							&err);
						sip_getTimeFormatFromDateStruct(datestruct,&tformat,\
							&err);
						sip_getDayFromDateFormat(dformat,&date,&err);
						sip_getYearFromDateFormat(dformat,&year,&err);
						sip_getMonthFromDateFormat(dformat,&emonth,&err);
						sip_getHourFromTimeFormat(tformat,&hour,&err);
						sip_getMinFromTimeFormat(tformat,&minute,&err);
						sip_getSecFromTimeFormat(tformat,&sec,&err);
						month = (SIP_U16bit) emonth +1;
						printf("INVITE to expire on %d/%d/%d at %d:%d:%d\n",\
							date,month,year,hour,minute,sec);
						fflush(stdout);
						sip_freeSipDateFormat(dformat);
						sip_freeSipTimeFormat(tformat);
					}
					sip_freeSipDateStruct(datestruct);
					break;
				case SipExpSeconds:
					if(sip_getSecondsFromExpiresHdr(hdr,&seconds,&err)\
						==SipSuccess)
					{
						printf("INVITE to expire in %d seconds\n",seconds);
						fflush(stdout);
					}
					break;
				default:
					break;
			} /* switch */
			sip_freeSipHeader(hdr);
			free(hdr);
		} /* if count = 1 */
#else
		if(count==1)
		{
			SipHeader *hdr;
			SipDateStruct *datestruct;
			en_ExpiresType etype;
			SIP_U32bit seconds;

			printf("Message contains Expires header.\n");
			fflush(stdout);
			if(sip_initSipHeader(&hdr,SipHdrTypeExpiresAny,&err)!=SipSuccess)
			{
				printf("Error in initializing Expires Header\n");
				fflush(stdout);
				return;
			}
			if(sip_getHeader(s,SipHdrTypeExpiresAny,hdr,&err)!=SipSuccess)
			{
				printf("Error in accessing Expires Header\n");
				fflush(stdout);
				sip_freeSipHeader(hdr);
				return;
			}
			if(sip_getTypeFromExpiresHdr(hdr,&etype,&err)!=SipSuccess)
			{
				printf("Error in accessing Type from Expires Header\n");
				fflush(stdout);
				sip_freeSipHeader(hdr);
				return;
			}
			switch(etype)
			{
				case SipExpDate:
					if(sip_initSipDateStruct(&datestruct,&err)!=SipSuccess)
					{
						printf("Error in accessing Date from Expires Header\n");
						fflush(stdout);
						sip_freeSipHeader(hdr);
						return;
					}
					if(sip_getDateStructFromExpiresHdr(hdr,datestruct,&err)\
						==SipSuccess)
					{
						/* Get dateformat and timeformat from the datestruct */
						SipDateFormat *dformat;
						SipTimeFormat *tformat;
						SIP_U16bit year,month;
						SIP_U8bit date;
						SIP_S8bit hour,minute,sec;
						en_Month emonth;

						sip_initSipDateFormat(&dformat,&err);
						sip_initSipTimeFormat(&tformat,&err);
						sip_getDateFormatFromDateStruct(datestruct,dformat,\
							&err);
						sip_getTimeFormatFromDateStruct(datestruct,tformat,\
							&err);
						sip_getDayFromDateFormat(dformat,&date,&err);
						sip_getYearFromDateFormat(dformat,&year,&err);
						sip_getMonthFromDateFormat(dformat,&emonth,&err);
						sip_getHourFromTimeFormat(tformat,&hour,&err);
						sip_getMinFromTimeFormat(tformat,&minute,&err);
						sip_getSecFromTimeFormat(tformat,&sec,&err);
						month = (SIP_U16bit) emonth +1;
						printf("INVITE to expire on %d/%d/%d at %d:%d:%d\n",\
							date,month,year,hour,minute,sec);
						fflush(stdout);
						sip_freeSipDateFormat(dformat);
						sip_freeSipTimeFormat(tformat);
					}
					sip_freeSipDateStruct(datestruct);
					break;
				case SipExpSeconds:
					if(sip_getSecondsFromExpiresHdr(hdr,&seconds,&err)==SipSuccess)
					{
						printf("INVITE to expire in %d seconds\n",seconds);
						fflush(stdout);
					}
					break;
				default:
					break;
			} /* switch */
			sip_freeSipHeader(hdr);
		} /* if count = 1 */
#endif
	}
	displayParsedMessage(s);
}

#ifdef SIP_DCS
void sip_indicateComet(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside COMET callback----\n");
	displayParsedMessage(s);
}
#endif

void sip_indicateRegister(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside REGISTER callback----\n");
	displayParsedMessage(s);
}

void sip_indicateCancel(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside CANCEL callback------\n");
	displayParsedMessage(s);
}

void sip_indicateOptions(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside OPTIONS callback-----\n");
	displayParsedMessage(s);
}

void sip_indicateBye(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside BYE callback---------\n");
	displayParsedMessage(s);
}

void sip_indicateRefer(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside REFER callback---------\n");
	displayParsedMessage(s);
}

void sip_indicateAck(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside ACK callback---------\n");
	displayParsedMessage(s);
}

void sip_indicateInfo(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside INFO callback---------\n");
	displayParsedMessage(s);
}

void sip_indicatePropose(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside PROPOSE callback---------\n");
	displayParsedMessage(s);
}

void sip_indicatePrack(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside PRACK callback---------\n");
	displayParsedMessage(s);
}

#ifdef SIP_IMPP
void sip_indicateSubscribe(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside SUBSCRIBE callback---------\n");
	displayParsedMessage(s);
}

void sip_indicateNotify(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside NOTIFY callback---------\n");
	displayParsedMessage(s);
}

void sip_indicateMessage(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside MESSAGE callback---------\n");
	displayParsedMessage(s);
}
#endif

void sip_indicateUpdate(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside UPDATE callback---------\n");
	displayParsedMessage(s);
}

void sip_indicateUnknownRequest(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside UNKNOWN REQUEST callback\n");
	displayParsedMessage(s);
}

void sip_indicateInformational(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside Informational callback\n");
	displayParsedMessage(s);
}

void sip_indicateFinalResponse(SipMessage *s, SipEventContext *context)
{
	SipEventContext *dummy;
	dummy = context;
	printf ("Inside Final callback-------\n");
	displayParsedMessage(s);
}

/* of SIP_NO_CALLBACK check */
#endif

#ifdef SIP_MIB
void sip_indicateResponseHandled(SipMessage *s,en_SipMibCallbackType dEvent,\
		SIP_S32bit	dCode,SipStatParam dParam)
{
	switch(dEvent)
	{
		case SipCallbkForResponseSent:
			{
				printf("\nCallback invoked for response sent <<<<\n");
				printf("\n Response with code %d sent",dCode);
				printf("\n***** %d such responses have been sent\n",\
							dParam.sendInitial+dParam.sendRetry);
				break;
			}
		case SipCallbkForResponseRecvd:
			{
				printf("\nCallback invoked for response recvd <<<<\n");
				printf("\n Response with code %d received",dCode);
				printf("\n**** %d such responses have been recvd\n",\
							dParam.recv);
				break;
			}
	}
	sip_freeSipMessage(s);
}
#endif

/* End of callback implementations */

/* Function used to send simple requests */
SipBool sendResponse(int code,const char *reason,const char *method, SipTranspAddr *sendaddr,SipError *err)
{
	SipMessage *sipmesg;
	SipStatusLine *statline;
	SipHeader *header;
	SipAddrSpec *addrspec;
	SipUrl *sipurl;
	SipBool res;
	SipEventContext *context;
	char contextstr[100];
	SipOptions options;

#ifndef SIP_TXN_LAYER
	int T1,T2;
	char retrans;
#else
	SipTranspAddr	*pTempTranspAddr=SIP_NULL;
	char *pTag;
#endif

	if(sip_initSipMessage(&sipmesg,SipMessageResponse,err)==SipFail)
	{
		printf("Could not form message.\n");
		fflush(stdout);
		exit(0);
	}
	sip_initSipStatusLine(&statline,err);

#ifdef SIP_BY_REFERENCE
	sip_setReasonInStatusLine(statline,strdup(reason),err);
#else
	sip_setReasonInStatusLine(statline,(SIP_S8bit *)reason,err);
#endif
#ifdef SIP_BY_REFERENCE
	sip_setVersionInStatusLine(statline,strdup("SIP/2.0"),err);
#else
	sip_setVersionInStatusLine(statline,(SIP_S8bit *)"SIP/2.0",err);
#endif
	sip_setStatusCodeNumInStatusLine(statline,(SIP_U16bit)code,err);
	sip_setStatusLineInSipRespMsg (sipmesg, statline, err);
	sip_freeSipStatusLine(statline);

	sip_initSipHeader(&header,SipHdrTypeFrom,err);
	sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err);
	sip_initSipUrl(&sipurl,err);
#ifdef SIP_BY_REFERENCE
	sip_setHostInUrl(sipurl,strdup("mydomain.com"),err);
#else
	sip_setHostInUrl(sipurl,(SIP_S8bit *)"mydomain.com",err);
#endif
	sip_setUrlInAddrSpec(addrspec,sipurl,err);
	sip_setAddrSpecInFromHdr(header,addrspec,err);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);

#ifdef SIP_TXN_LAYER
	pTag = strdup("ajkhdd89347");
	sip_insertTagAtIndexInFromHdr(header,pTag,0,err);
#endif
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	sip_initSipHeader(&header,SipHdrTypeTo,err);
	sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err);
	sip_initSipUrl(&sipurl,err);
#ifdef SIP_BY_REFERENCE
	sip_setHostInUrl(sipurl,strdup("yourdomain.com"),err);
#else
	sip_setHostInUrl(sipurl,(SIP_S8bit *)"yourdomain.com",err);
#endif
	sip_setUrlInAddrSpec(addrspec,sipurl,err);
	sip_setAddrSpecInToHdr(header,addrspec,err);

#ifdef SIP_TXN_LAYER
	pTag = strdup("ajkhdd89347");
	sip_insertTagAtIndexInToHdr(header,pTag,0,err);
#endif

	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);

	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	sip_initSipHeader(&header,SipHdrTypeCallId,err);
#ifdef SIP_BY_REFERENCE
	sip_setValueInCallIdHdr(header,strdup("1234324453@mydomain.com"),err);
#else
	sip_setValueInCallIdHdr(header,(SIP_S8bit *)"1234324453@mydomain.com",err);
#endif
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	sip_initSipHeader(&header,SipHdrTypeCseq,err);
	if((strcmp(method,"PRACK")==0)||(strcmp(method,"BYE")==0))
		sip_setSeqNumInCseqHdr(header,2,err);
	else
		sip_setSeqNumInCseqHdr(header,1,err);
#ifdef SIP_BY_REFERENCE
	sip_setMethodInCseqHdr(header,strdup(method),err);
#else
	sip_setMethodInCseqHdr(header,(SIP_S8bit *)method,err);
#endif
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

if((glbtransptype&SIP_UDP) ==SIP_UDP)
{

#ifdef SIP_BY_REFERENCE
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *) "Via: SIP/2.0/UDP 135.180.130.133 (Via header comment); branch=hughes",0,err);
#else
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *) " Via: SIP/2.0/UDP 135.180.130.133 (Via header comment); branch=hughes",0,err);
#endif
}
else
{
#ifdef SIP_BY_REFERENCE
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *) "Via: SIP/2.0/TCP 135.180.130.133 (Via header comment); branch=hughes",0,err);
#else
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *) " Via: SIP/2.0/TCP 135.180.130.133 (Via header comment); branch=hughes",0,err);
#endif
}

	if((strcmp(method,"INVITE")==0)&&( (code<200) && (code != 100) ))
	{
	sip_initSipHeader(&header,SipHdrTypeRSeq,err);
	sip_rpr_setRespNumInRSeqHdr(header,1,err);
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);
#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	}

	context = (SipEventContext* ) malloc(sizeof(SipEventContext));
	sprintf(contextstr,"Response with code %d ",code);
	context->pData = strdup(contextstr);
	context->pDirectBuffer = SIP_NULL;
	options.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | \
		SIP_OPT_RETRANSCALLBACK;
#ifndef SIP_TXN_LAYER
	if(glbSipOption==1)
	{
		printf("\n Do you want customise the retransmission value(y/n):[n]");
		fflush(stdout);
		fflush(stdin);
		retrans=sip_getchar();
		if((retrans=='y')||(retrans=='Y'))
		{
			char value[10];
			options.dOption |= SIP_OPT_PERMSGRETRANS;
			fflush(stdout);
			printf("\n enter value of T1:");
			fflush(stdin);
			fflush(stdout);
			scanf("%s",value);
			T1=atoi(value);
			fflush(stdin);
			fflush(stdout);
			printf("\n enter value of T2:");
			fflush(stdin);
			fflush(stdout);
			scanf("%s",value);
			T2=atoi(value);

			context->dRetransT1 = T1;
			context->dRetransT2 = T2;
		}
	}
	glbSipOption=0;
#endif

/*	Uncomment to test per message retransmission interval setting.
	context->dRetransT1 = 2000;
	context->dRetransT2 = 8000;
	options.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | SIP_OPT_PERMSGRETRANS;
*/


/*	Uncomment to test per message retransmission interval setting with
 	retranscount
	context->dRetransT1 = 2000;
	context->dRetransT2 = 8000;
	context->dMaxRetransCount = 4;
	context->dMaxInviteRetransCount = 2;
	options.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | SIP_OPT_PERMSGRETRANS\
						SIP_OPT_PERMSGRETRANSCOUNT;
*/
/* Uncomment to test for sending a buffer directly into the sendmessage
{
	char* sendbuffer;
	int dLength;
	sendbuffer= (char*) malloc(SIP_MAX_MSG_SIZE);
	if(sendbuffer== SIP_NULL)
			return SipFail;
	if(sip_formMessage(sipmesg, &options, sendbuffer, &dLength, err) == SipFail)
	{
			free(sendbuffer);
			return SipFail;
	}
	options.dOption |= SIP_OPT_DIRECTBUFFER;
	context->pDirectBuffer= (SIP_Pvoid) sendbuffer;
*/

#ifdef SIP_TXN_LAYER
	{
		SipTxnContext *txncontext;
		SipTxnKey *pTxnKey = SIP_NULL;
		en_SipTxnBypass dbypass;
		sip_txn_initSipTxnContext(&txncontext,err);
		txncontext->pEventContext= context;
/*		options.dOption =options.dOption | SIP_OPT_DIRECTBUFFER;*/
		txncontext->txnOption.dOption = options;
		dbypass = SipTxnNoByPass;

		if(!glbtransactiontype)
			txncontext->dTxnType = SipUATypeTxn;
		else
			txncontext->dTxnType = SipProxyTypeTxn;

		txncontext->txnOption.dTimeoutCbkOption=glbTimeTimeoutOption;

		if(glbsetTimerValue)
		{
			txncontext->timeoutValues.dT1 = dTimeOut.dT1;
			txncontext->timeoutValues.dT2		= dTimeOut.dT2;
			txncontext->timeoutValues.dTimerB	= dTimeOut.dTimerB;
			txncontext->timeoutValues.dTimerC	= dTimeOut.dTimerC;
			txncontext->timeoutValues.dTimerD_T3	= dTimeOut.dTimerD_T3;
			txncontext->timeoutValues.dTimerF_T3	= dTimeOut.dTimerF_T3;
			txncontext->timeoutValues.dTimerH	= dTimeOut.dTimerH;
			txncontext->timeoutValues.dTimerI_T4	= dTimeOut.dTimerI_T4;
			txncontext->timeoutValues.dTimerJ_T3	= dTimeOut.dTimerJ_T3;
			txncontext->timeoutValues.dTimerK_T4	= dTimeOut.dTimerK_T4;
			txncontext->txnOption.dTimerOption = SIP_OPT_TIMERALL;
		}
		
/*		{
			sendbuffer= (char*) malloc(SIP_MAX_MSG_SIZE);
			if(sendbuffer== SIP_NULL)
				return SipFail;
			if(sip_formMessage\
					(sipmesg, &options, sendbuffer, &dLength, err) == SipFail)
			{
				free(sendbuffer);
				return SipFail;
			}
			context->pDirectBuffer = (SIP_Pvoid) sendbuffer;
		}	*/
		
		pTempTranspAddr=(SipTranspAddr *)fast_memget\
						(0,sizeof(SipTranspAddr),err);
		*pTempTranspAddr=*sendaddr;
		res = sip_txn_sendMessage(sipmesg,pTempTranspAddr,glbtransptype,\
				txncontext, dbypass,&pTxnKey,err);
		if(res == SipFail)
		{
			sip_freeEventContext(context);
			switch(*err)
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
					break;
			}
			printf("** ERROR : Send Message Failed**\n");
			fflush(stdout);
		}
		else
		{
			sip_txn_freeTxnKey(pTxnKey,err);
		}
		free(txncontext);
	}
#else
	res = sip_sendMessage(sipmesg,&options,sendaddr,glbtransptype,context,err);
#endif
	sip_freeSipMessage(sipmesg);

/* Uncomment to test for sending a buffer directly into the sendmessage/
}
*/
	if(res==SipSuccess)
	{
		printf("Response with code %d sent successfully.\n",code);
		fflush(stdout);
	}
	return SipSuccess;
}
#ifdef SIP_SOLARIS
#ifdef SIP_BY_REFERENCE
/*********************************************************************
** Description :This function takes out Sdp Msg Body frm given
**             : given SipMsgBody.
** Parameters  : pSipMsgBody -> Sip Message Body from where Message
**             :               will be extracted
**             : pSdpMsg -> Sdp Message extracted from pSipMsgBody  
**             :            will be put in this. 
** Please note that this is illustrative function.
** In this follwing fields are extracted from SdpMessage
**       Version,
**       Origin and some of its subfields of Origin,
**       Media and some of its sub-fields
** Rest of the member fields of SdpMessage will map to one of
** above fields.
**********************************************************************/
SipBool extractSdpMsgBody(SipMsgBody *pSipMsgBody,SdpMessage *pSdpMsg)
{
   SIP_S8bit * pVersion = SIP_NULL,*pUser=SIP_NULL,*pInfo=SIP_NULL  ;
   SIP_S8bit * pSession = SIP_NULL,* pBandWidth = SIP_NULL,*pMediaValue=SIP_NULL ;
   SIP_S8bit * pProtocol = SIP_NULL ,* pSessionId = SIP_NULL ;
   SdpOrigin * pOrigin = SIP_NULL ;
   SdpMedia  * pSdpMedia = SIP_NULL ;
   SIP_U32bit index=0,bwIndex=0,count=0,bwCount=0 ;
   SipError err = E_NO_ERROR ;

   printf("\n\nEntering extractSdpMsgBody \n") ;
   /* Extract Sdp Message Body from SipMsgBody */
   if (sip_getSdpFromMsgBody(pSipMsgBody,&pSdpMsg,&err) == SipFail)
   {
	   printf("Error = %d in extracting sdp from msg-body \n",err) ;
	   return SipFail ;
   }
   printf("**Extracted SdpMessage Body **\n") ;

   
   /*
	 Extract Version from SdpMsgBody extracted .
     Version is of type SIP_S8bit * in SdpMessage struct.
   */
   if (sdp_getVersion(pSdpMsg,&pVersion,&err) == SipFail )
   {
	   printf("Error = %d while extracting version from sdp-msg-body\n",
		   err) ;
	   return SipFail ;
   }
	 else
	 {
   printf("Sdp-Version = %s \n",pVersion) ;
	 }

   /*
    Extract Origin from SdpMsgBody extracted
    Origin is of type SdpOrigin in SdpMessage struct.
	SdpOrigin contains User,SessionId,Version,NetType and Address fields
	and all of them are of type SIP_S8bit *
	So we will be working right now only on first two of them.
   */
   if (sdp_getOrigin(pSdpMsg,&pOrigin,&err) == SipFail )
   {
		printf("Error = %d while extracting Origin from sdp-msg-body\n",
		   err) ;
	   return SipFail ;
   }
   printf("Extracted Origin from SdpMessage Body \n") ;


   /*
     Extract user from origin
   */
   if ( sdp_getUserFromOrigin(pOrigin,&pUser,&err) == SipFail )
   {
	   	printf("Error = %d while extracting user from Origin of sdp-msg-body\n",
		   err) ;
   }
   else
	 printf("Origin.user = %s \n",pUser) ;

   /*
     Extract SessionId from Origin
   */
   if ( sdp_getSessionIdFromOrigin(pOrigin,&pSessionId,&err) == SipFail)
   {
   	printf("Error = %d while extracting Session-Id from Origin of sdp-msg-body\n",
		   err) ;
   }
   else
	 printf("Origin.Session = %s \n",pSessionId) ;

   sip_freeSdpOrigin(pOrigin) ;

   
   /*
    Extract Session from SdpMsgBody extracted
	Session is of SIP_S8bit * in SdpMessage struct.
   */
   if (sdp_getSession(pSdpMsg,&pSession,&err) == SipFail )
   {
		printf("Error = %d while extracting Origin from sdp-msg-body\n",
		   err) ;
   }
   else
		printf("Sdp-Session = %s \n",pSession) ;


   /*
    Extract Media from Sdp Message.
    Media is a SipList of SdpMedia struct type
	and SdpMedia contains lots of information namely
	Key,Protocol,Information,List of Connection,List of Bandwidth,
	Port Number,Media Values and virtual CID.
	In following code,we will be working on 
		Info sub-field,
		List of Bandwidth,
		Protocol subfield,
		Media Value subfield.
   Rest of fields will map to one of above.
   */
   if ( sdp_getMediaCount(pSdpMsg,&count,&err) == SipFail )
   {
		printf("Error = %d while extracting Media-Count from sdp-msg-body\n",
		   err) ;
	   return SipFail ;
   }
   index = 0 ;


   while (index < count )
   {
	   /*
	    Extract Media At index from siplist of media stored in sdp-msg
		Media is of type SdpMedia struct.
	   */
	   if ( sdp_getMediaAtIndex(pSdpMsg,&pSdpMedia,index,&err) == SipFail )
	   {
         printf("Error = %d while extracting media at index = %d \n",
			  err,index) ;
	     return SipFail ;
	   }
	   printf("\nExtracted Media at index = %d \n",index) ;
	   /*
	    Extract Info from Media extracted in last step
		Info is of type SIP_S8bit *
		*/
	   if ( sdp_getInfoFromMedia(pSdpMedia,&pInfo,&err) == SipFail )
	   {
         printf("Error = %d while extracting info from media at index = %d \n",
			  err,index) ;
	   }
	   else
		printf("Media[%d].Info = %s \n",index,pInfo) ;

	   /*
	    Extract BandWidth-Count from Media extracted in last step
		Info is of type SipList of type char *
	   */
	   if (sdp_getBandwidthCountFromMedia(pSdpMedia,&bwCount,&err) == SipFail )
	   {
		printf("Error = %d while extracting bandwidth-count from media at index = %d \n",
			  err,index) ;
	   }
	   printf("Bandwidth count = %d \n",bwCount) ;
       for (bwIndex=0;bwIndex < bwCount;bwIndex++)
	   {
	     if ( sdp_getBandwidthAtIndexFromMedia(pSdpMedia,&pBandWidth,bwIndex,&err) == SipFail )
		 {
			 printf("Error = %d while extracting Bandwidth Count at bw-index = %d at media-index = %d \n",
				       err,bwIndex,index);
			 return SipFail ;
		 }
		 printf("Extracted Bandwidth = %d at bw-index = %d at media-index = %d \n",
				       *pBandWidth,bwIndex,index);

	   } /* for */

	   /* Get Media-Value from Media */
	   if (sdp_getMvalueFromMedia(pSdpMedia,&pMediaValue,&err) == SipFail )
	   {
		   printf("Error = %d while extracting media-value at index = %d\n",err,index) ;
	   }
	   else
		   printf("MediaValue = %s at media-index = %d \n",pMediaValue,index) ;

	   /* Get Protocol from Media */
	   if (sdp_getProtoFromMedia(pSdpMedia,&pProtocol,&err) == SipFail )
	   {
		   printf("Error = %d while extracting protocol-value at index = %d\n",err,index) ;
	   }
	   else
		   printf("Protocol = %s at media-index = %d \n",pProtocol,index) ;
	   sip_freeSdpMedia(pSdpMedia) ;
	   index++ ;

   }

 return SipSuccess ;
}


/**********************************************************************
** Description : This function extracts Isup Message from SipMsgBody.
** Parameters  : pSipMsgBody -> Message Body which contains the Isup-Msg
**             : pIsupMsg    -> Extracted IsupMsg will be put in this.
** All Isup related getApis are illustrated in this function.
***********************************************************************/
SipBool extractIsupMsgBody(SipMsgBody *pSipMsgBody,IsupMessage * pIsupMsg)
{
	SIP_U32bit isupLen=0 ;
	SIP_S8bit *pIsupMsgBody = SIP_NULL ;
	SipError err = E_NO_ERROR ;

	printf("\n\nEntering extractIsupMsgBody \n") ;
    /* Extract Isup frm SipMsgBody */
	if (sip_bcpt_getIsupFromMsgBody(pSipMsgBody,&pIsupMsg,&err) == SipFail )
	{
		printf("Error = %d while extracting IsupMessage \n",err) ;
		return SipFail ;
	}
	printf("** Extracted ISUPMsgBody **\n") ;

	/* Extract Length frm IsupMsg extracted */
	if ( sip_bcpt_getLengthFromIsupMessage(pIsupMsg,&isupLen,&err) == SipFail )
	{
		printf("Error = %d while getting length from Isup Message\n",err) ;
		return SipFail ;
	}
	printf("Length of Isup Message = %d \n",isupLen) ;

	/* Extract Body frm IsupMsg extracted */
	if (sip_bcpt_getBodyFromIsupMessage(pIsupMsg,&pIsupMsgBody,&err) == SipFail)
	{
		printf("Error = %d while extracting msg-body from Isup-Msg \n",err) ;
		return SipFail ;
	}
	printf("Isup-Msg-Body = %s \n",pIsupMsgBody) ;

 return SipSuccess ;
}







/*********************************************************************
** Description : This function will process a sip message and work	
**               upon the message body extracted out of that.		
** Parameters  : pSipMsg -> MsgBody of this SipMsg will be worked upon
**             :           in this function
** Returns     : SipFail    -> Failure Completion
**             : SipSuccess -> Successful Completion 
**********************************************************************/
SipBool processSipMsgBody(SipMessage *pSipMsg)
{
   SIP_U32bit index=0,msgBodyCount=0;
   SipError err=E_NO_ERROR;
   en_SipMsgBodyType msgBodyType=SipBodyAny ;
   SipMsgBody *pSipMsgBody = SIP_NULL ;
   SipMimeHeader *pMimeHdr = SIP_NULL ;
   SipHeader *pContentTypeHdr = SIP_NULL ;
   SdpMessage *pSdpMsg = NULL ;
   SIP_S8bit *pMediaType = SIP_NULL ;
   IsupMessage *pIsupMsg = SIP_NULL ;



   printf("Entering processSipMsgBody() \n") ;
   /* 
    Extract count of SipMsgBody from SipMessage 
   */
   if (sip_getMsgBodyCount(pSipMsg,&msgBodyCount,&err) == SipFail)
   {
		printf("Error while extracting sip-mesg-count : %d \n",err) ;
		return SipFail ;
		/*
		 Possible errors
		    -> E_NO_EXIST  (when no message-body exists in sip-messsage)
			-> E_INV_PARAM (when no message exists or count-param
			                  is not pointing to valid memory)
		*/
	}
	printf("No of message-body in sip-message = %d \n",msgBodyCount) ;

	if ( sip_initSipHeader(&pContentTypeHdr,SipHdrTypeContentType,&err) == SipFail)
	{
		printf("Error = %d while initialising the header \n",err ) ;
		return SipFail ;
	}
   /* 
	To get the Type of Message Body , extract contenttype header frm sip-message 
   */
   if ( sip_getHeader(pSipMsg,SipHdrTypeContentType,pContentTypeHdr,&err) == SipFail)
   {
	   printf("Error = %d while extracting content-type frm sip-message \n", err) ;
	   return SipFail ;
   }
 
   /* 
    Get Media Type from Content-Type header 
   */
   if ( sip_getMediaTypeFromContentTypeHdr(pContentTypeHdr,&pMediaType,&err) == SipFail )
   {
	 printf("Error = %d while extracting Content-Type-Hdr \n",err) ;
	 return SipFail ;
   }
   printf("Extracted MediaType = %s \n",pMediaType) ;

   sip_freeSipHeader(pContentTypeHdr) ;
#ifdef SIP_BY_REFERNCE
   free(pContentTypeHdr) ;
#endif

    /* 
     Start working on the list of message bodies one by one 
    */
	while (index < msgBodyCount )
	{
        printf("Extract msg-body at index = %d from sip-msg\n",index) ;
		/* 
		 Get the message body located at "index" 
		*/
		if ( sip_getMsgBodyAtIndex(pSipMsg,&pSipMsgBody,index,&err) == SipFail )
		{
			printf("Error = %d while extracting msg body at index  = %d \n",
			 			err,index) ;
			return SipFail ;

			/* 
			 Possible errors
				-> E_INV_PARAM (when no message exists or count-param
								  is not pointing to valid memory)
				-> E_INV_INDEX (when index from where the message-body is to
								extracted is wrong)
				-> E_NO_EXIST  (when no message-body exists in sip-message)
			*/

		}
		/* 
		  Extract Mime Header from Message Body
		  Mime Header will contain ->
		  		ContentType Header
		  		ContentDisposition Header
		  		ContentId Header
		  		ContentDescription Header
		  		ContentTransEncoding Header
		  		Other Mime Headers
		 */

   	    if ( sip_bcpt_getMimeHeaderFromMsgBody(pSipMsgBody,&pMimeHdr,&err) == SipFail)
		{
			printf("Error = %d while getting mime-hdr-from-msg-body at index = %d \n",
				err,index) ;
			return SipFail ;
		}
		printf("Extracted MimeHeader from Msg Body \n") ;

		if ( sip_initSipHeader(&pContentTypeHdr,SipHdrTypeContentType,&err) == SipFail)
		{
			printf("Error = %d while initialising the header \n",err ) ;
			return SipFail ;
		}

		if ( sip_bcpt_getContentTypeFromMimeHdr(pMimeHdr,&pContentTypeHdr,&err) == SipFail)
		{
			printf("Error = %d while extracting content-type-header \n",err) ;
			return SipFail ;
		}
		printf("Extracted ContentTypeHeader from Mime Header \n");

		if ( sip_getMediaTypeFromContentTypeHdr(pContentTypeHdr,&pMediaType,&err) == SipFail )
		{
			printf("Error = %d while extracting Content-Type-Hdr \n",err) ;
			return SipFail ;
		}
		printf("Extracted MediaType = %s \n",pMediaType) ;
		sip_bcpt_freeSipMimeHeader(pMimeHdr) ;
		sip_freeSipHeader(pContentTypeHdr) ;
		free(pContentTypeHdr) ;


		/* 
		 Get the message body type of the body extracted 
		*/
		if (sip_getTypeFromMsgBody(pSipMsgBody,&msgBodyType,&err) == SipFail )
		{
			printf("Error = %d while extracting message type frm msg-body at index = %d\n",
			                         err,index) ;
			return SipFail ;

			/* 
			  Possible errors
				-> E_INV_PARAM (when no message exists or count-param
								  is not pointing to valid memory)
				-> E_INV_INDEX (when index from where the message-body is to
								extracted is wrong)
				-> E_NO_EXIST  (when no message-body exists in sip-message)
			*/

		 }


		switch (msgBodyType)
		{
		  case  SipSdpBody : /* MsgBody is SDP type (content-type header) */
		   		{
					printf("Message Body Type is Sdp Type \n") ;
					if (sip_initSdpMessage(&pSdpMsg,&err) == SipFail )
					{
					  printf("Error = %d while init-sdp-mesg \n",err);
					  return SipFail ;
					}
					if (extractSdpMsgBody(pSipMsgBody,pSdpMsg) == SipFail )
					{
					  printf("Error = %d while extracting sdp-msg-body\n",err) ;
					  return SipFail ;
					}
					sip_freeSdpMessage(pSdpMsg) ;
					break ;
	   			}
	   	  case 	SipIsupBody:	/* MsgBody is Isup type */
	   	  		 {
					printf("Message Body Type is Isup Type \n") ;
					if (sip_bcpt_initIsupMessage(&pIsupMsg,&err) == SipFail)
					{
						printf("Error = %d while initializing Isup-Msg\n",err) ;
						return SipFail ;
				    }
					if ( extractIsupMsgBody(pSipMsgBody,pIsupMsg) == SipFail )
					{
						printf("Error = %d while extracting isup-msg-body \n",err) ;
						return SipFail ;
					}
					sip_bcpt_freeIsupMessage(pIsupMsg) ;
					break ;
				 }
		  case	SipMultipartMimeBody:	   /* MsgBody is Mime type */
		  	    {
		  		 printf("\n----- Message Body Type is MULTIPARTMime Type ---- \n\n") ;
		  		 break ;
		  	     }
		  case SipAppSipBody:		   /* MsgBody is message/sipfrag  */
		  	    {
		  		 printf("Message Body Type is Isup Type \n") ;
		  		 break ;
		  	    }
		  case SipUnknownBody:		   /* MsgBody is unknown type to stack */
		  	    {
		  		 printf("Message Body Type is Isup Type \n") ;
		  		 break ;
		  	    }

#ifdef	 SIP_MWI

		  case SipMessageSummaryBody:
		  		{
		  		}
#endif
		  case SipQsigBody :		 /* MsgBody is Qsig type */
		  	     {
		  		  printf("Message Body Type is QSig Type \n") ;
		  		  break ;
		  	     }

		  default:
		  	    {
		  		  printf("Message Body Type is Any. Invalid Message \n") ;
				  break ;
			    }


 	    }/* switch */
 	    index ++ ;
 	    msgBodyType=SipBodyAny ; /* initializing the variable back */

		sip_freeSipMsgBody(pSipMsgBody) ; /* to decrement the ref-count */
     }/* while */
	 printf("Exiting processSipMsgBody() \n") ;
	return SipSuccess ;
} /*end of function */


/***********************************************************************************
** Function    :   buildSdpBody
** Description :   This function builds Sdp Body
**             :     pErr -> Variable where the error will be put into
**             :   It returns the sdp-msg-formed.  
***********************************************************************************/

SipMsgBody * buildSdpBody(SipError *pErr)
{
 SipMsgBody * pSipMsgBody = SIP_NULL ;
 SdpMessage * pSdpMsg = SIP_NULL ;
 SIP_S8bit  * pVersion = SIP_NULL ;
 SdpOrigin  * pOrigin = SIP_NULL ;

 printf("\nEntering buildSdpBody() \n") ;

 /* Initialize Sdp Message (allocating space for the sdp-msg-body-pointer 
    (it will init refcount by 1) */
 if ( sip_initSdpMessage(&pSdpMsg,pErr) == SipFail )
 {
	 printf("Error = %d while initialising SdpMessage \n",*pErr) ;
	 return SIP_NULL ;
 }

 
/* Set version in Sdp-Msg (v=0) */
 pVersion  = strdup("0") ;
 if ( sdp_setVersion(pSdpMsg,pVersion,pErr) == SipFail )
 {
	 printf("Error = %d while setting version in SdpMessage \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 return SIP_NULL ;
 }
 printf("set version in sdp-msg \n") ;

 /* Build Origin field for Sdp Message */
 
 /* Initialize Sdp Origin pointer (allocate space) */
 if ( sip_initSdpOrigin(&pOrigin,pErr) == SipFail )
 {
	 printf("Error = %d while init sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 return SIP_NULL ;
 }
 /* Set user field in origin */ 
 if ( sdp_setUserInOrigin(pOrigin,strdup("USER_NAME"),pErr) == SipFail)
 {
	 printf("Error = %d while setting user-part in sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSdpOrigin(pOrigin) ;
	  return SIP_NULL ;
 }

 /* Set session-id field in origin */
 if ( sdp_setSessionIdInOrigin(pOrigin,strdup("221212"),pErr)  == SipFail)
 {
	 printf("Error = %d while setting session-id in sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSdpOrigin(pOrigin) ;
	 return SIP_NULL ;
 }

 /* Set Version Field in Origin */
 if ( sdp_setVersionInOrigin(pOrigin,strdup("3"),pErr) == SipFail)
 {
	 printf("Error = %d while setting session-id in sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSdpOrigin(pOrigin) ;
     return SIP_NULL ;
 }

 /* Set NetType in Origin */
 if ( sdp_setNetTypeInOrigin(pOrigin,strdup("IN"),pErr) == SipFail)
 {
	 printf("Error = %d while setting nettype in sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSdpOrigin(pOrigin) ;
     return SIP_NULL ;
 }

 /* Set Addr Type in Origin */
 if ( sdp_setAddrTypeInOrigin(pOrigin,strdup("IP4"),pErr)  == SipFail )
 {
	 printf("Error = %d while setting addr in sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSdpOrigin(pOrigin) ;
     return SIP_NULL ;
 }
 /* Set Addr in Origin */
 if ( sdp_setAddrInOrigin(pOrigin,strdup("139.85.229.129"),pErr)  == SipFail )
 {
	 printf("Error = %d while setting addr in sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSdpOrigin(pOrigin) ;
     return SIP_NULL ;
 }


 /* Set Origin into Sdp Message */
 if ( sdp_setOrigin(pSdpMsg,pOrigin,pErr) == SipFail )
 {
	 printf("Error = %d while setting session-id in sdp-origin \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSdpOrigin(pOrigin) ;
     return SIP_NULL ;
 }

 /* Free Origin Variable,decrement the ref count as it is no longer required as
   independent entity but only inside SdpMessage */
 sip_freeSdpOrigin(pOrigin) ;
 printf("set origin field in the sdp-msg \n") ;


 /* Initialize Sip Msg Body */
 if ( sip_initSipMsgBody(&pSipMsgBody,SipSdpBody,pErr) == SipFail )
 {
	printf("Error = %d while initializing Isup Message\n",*pErr) ;
	sip_freeSdpMessage(pSdpMsg) ;
	return SIP_NULL  ;
 }

 /* Set Sdp-Msg-Body in Sip-Message-Body . This function will 
    increase the refcount of SdpMsg by 1 (infact,it will be 2 
	after this fn call)*/
 if ( sip_setSdpInMsgBody(pSipMsgBody,pSdpMsg,pErr) == SipFail )
 {
	 printf("Error = %d while setting version in SdpMessage \n",*pErr) ;
	 sip_freeSdpMessage(pSdpMsg) ;
	 sip_freeSipMsgBody(pSipMsgBody) ;
	 return SIP_NULL ;
 }

 /* Free the Sdp Message as this is no longer required as independent
    entity but inside the SipMsgBody . Decrement the refcount by 1.
 */
 sip_freeSdpMessage(pSdpMsg) ;
 
 printf("Successfully Set SdpMessage in SipMsgBody \n\n");
 printf("\nExiting buildSdpBody() successfully \n");
 return pSipMsgBody;
}

/***********************************************************************************
** Function    :   buildIsupBody
** Description :   This function builds Isup Body
**             :     pErr -> Variable where the error will be put into
**             :   It returns the isup-msg-formed.  
***********************************************************************************/
SipMsgBody * buildIsupBody(SipError *pErr)
{
 SipMsgBody * pSipMsgBody = SIP_NULL ;
 IsupMessage *pIsupMsg = SIP_NULL ;
 SIP_S8bit * pBody = strdup("34 56 78 23") ;
 SIP_U32bit len= strlen(pBody) ;

 printf("\nEntering buildIsupBody() \n");

 /* Initialize ISUP Message */
 if (sip_bcpt_initIsupMessage(&pIsupMsg,pErr) == SipFail )
 {
	printf("Error = %d while initializing Isup Message\n",*pErr) ;
	return SIP_NULL  ;
 }

 /* Set Body in Isup Message */
 if (sip_bcpt_setBodyInIsupMessage(pIsupMsg,pBody,len,pErr) == SipFail)
 {
	printf("Error = %d while setting body in isup-msg\n",*pErr) ;
	sip_bcpt_freeIsupMessage(pIsupMsg) ;
	return SIP_NULL  ;
 }

 /* Initialize Sip Message Body where SipIsupMsgBody will be put */
 if ( sip_initSipMsgBody(&pSipMsgBody,SipIsupBody,pErr) == SipFail )
 {
	printf("Error = %d while initializing Isup Message\n",*pErr) ;
	sip_bcpt_freeIsupMessage(pIsupMsg) ;
	return SIP_NULL  ;
 }

 /* Set Isup-Msg-Body in Sip-Message-Body */
 if ( sip_bcpt_setIsupInMsgBody(pSipMsgBody,pIsupMsg,pErr) == SipFail )
 {
	printf("Error = %d while setting isup-msg in sip-msg-body \n",*pErr) ;
	sip_bcpt_freeIsupMessage(pIsupMsg) ;
	sip_freeSipMsgBody(pSipMsgBody) ;
	return SIP_NULL  ;
 }
 
 /* Free IsupMessage as it is only required inside the SipMsgBody */
 sip_bcpt_freeIsupMessage(pIsupMsg) ;

 printf("Successfully Set IsupMessage in SipMsgBody \n\n");
 printf("\nExiting buildIsupBody() successfully \n");
 return pSipMsgBody;
}


/***********************************************************************************
** Function    :   buildMimeHeader
** Description :   This function builds Mime Header.
**             :   	 pContent -> Content Value
**             :     pDesc    -> Description Value
**             :	 pMedia   -> Media Value
***********************************************************************************/
SipMimeHeader * buildMimeHeader(SipError *pErr,SIP_S8bit *pDesc,SIP_S8bit *pMedia)
{
  SipMimeHeader *pMimeHdr = SIP_NULL ;
  SIP_S8bit * pDescription = SIP_NULL ;
  SipHeader *pHeader = SIP_NULL ;

 /* Initialize Mime Header */
 if ( sip_bcpt_initSipMimeHeader(&pMimeHdr,pErr) == SipFail )
 {
	 printf("Error = %d while initializing MimeHeader \n",*pErr) ;
	 return SIP_NULL ;
 }

 pDescription = strdup(pDesc) ;
 printf("MimeHeader === pDescription = %s \n",pDescription) ;

 if ( sip_bcpt_setContentDescInMimeHdr(pMimeHdr,pDescription,pErr) == SipFail )
 {
	 printf("Error = %d while Setting Content-Description in MimeHeader \n",*pErr) ;
	 sip_bcpt_freeSipMimeHeader(pMimeHdr) ;
	 return SIP_NULL ;
 }
 
 
 /* Initialize Sip Header of type SipHdrTypeContentType */
 if ( sip_initSipHeader(&pHeader,SipHdrTypeContentType,pErr) == SipFail )
 {
	 printf("Error = %d while initialising Header \n",*pErr) ;
	 sip_bcpt_freeSipMimeHeader(pMimeHdr) ;
	 return SIP_NULL ;
 }

 /* Set MediaType in content type header*/
 sip_setMediaTypeInContentTypeHdr(pHeader,strdup(pMedia),pErr) ;
 if ( sip_bcpt_setContentTypeInMimeHdr(pMimeHdr,pHeader,pErr) == SipFail )
 {
	 printf("Error = %d while Setting Content-Description in MimeHeader \n",*pErr) ;
	 sip_bcpt_freeSipMimeHeader(pMimeHdr) ;
	 sip_freeSipHeader(pHeader);
#ifdef SIP_BY_REFERENCE
			free(pHeader);
#endif
	 return SIP_NULL ;
 }

 sip_freeSipHeader(pHeader);
#ifdef SIP_BY_REFERENCE
			free(pHeader);
#endif

 return pMimeHdr ;
}

/*************************************************************************************
** Function     :  buildMultipartMixedBody
** Description  :  This function will build multipart/mime body and in case of error
**              :  it will put that in pErr.
**              :  It returns Message Body formed in SipMsgBody variable.
*************************************************************************************/
SipBool buildMultipartMixedBody(SipMessage * pSipMsg, SipError *pErr)
{
  SipMimeHeader *pFirstMimeHdr = SIP_NULL , *pSecondMimeHdr = SIP_NULL  ;
  SipMsgBody * pFirstMsgBody = SIP_NULL , *pSecondMsgBody = SIP_NULL ;


 printf("Entering buildMultipartMixedBody() \n") ;

 /* Building Mime Headers */
 if ( (pFirstMimeHdr  = buildMimeHeader(pErr,(SIP_S8bit*)"Description_Sdp_Message_Body",(SIP_S8bit*)"application/SDP") ) 
		                            == SIP_NULL)
 {
	 return SipFail ;
 }

 if ( (pSecondMimeHdr = buildMimeHeader(pErr,(SIP_S8bit*)"Description_Isup_Message_Body",(SIP_S8bit*)"application/ISUP") ) 
		                     == SIP_NULL )
 {
	 sip_bcpt_freeSipMimeHeader(pSecondMimeHdr) ;
	 return SipFail ;
 }



 /* Now put the message body in the message body. 
    So build the message bodies */

 /* Build First Message Body -> Sdp */
 if ((pFirstMsgBody=buildSdpBody(pErr)) == SIP_NULL)
 {
	 printf("Error = %d while setting first-hdr in msg-body\n",*pErr) ;
	 sip_bcpt_freeSipMimeHeader(pFirstMimeHdr) ;
	 sip_bcpt_freeSipMimeHeader(pSecondMimeHdr) ;
	 return SipFail ;
 }

 /* Set First-Mime Header in First Message Body */
 if (sip_bcpt_setMimeHeaderInMsgBody(pFirstMsgBody,pFirstMimeHdr,pErr) == SipFail )
 {
	 printf("Error = %d in setting MimeHeader in message body \n",*pErr) ;
	 sip_bcpt_freeSipMimeHeader(pFirstMimeHdr) ;
	 sip_bcpt_freeSipMimeHeader(pSecondMimeHdr) ;
	 sip_freeSipMsgBody(pFirstMsgBody) ;
	 return SipFail ;
 }

 /* Build Second Message Body -> Isup */
 if ((pSecondMsgBody=buildIsupBody(pErr)) == SIP_NULL)
 {
	 printf("Error = %d while building second-msg-body\n",*pErr) ;
	 sip_bcpt_freeSipMimeHeader(pSecondMimeHdr) ;
   sip_bcpt_freeSipMimeHeader(pFirstMimeHdr) ;
   sip_freeSipMsgBody(pFirstMsgBody) ;
	 return SipFail ;
 }

 /* Set Sec-Mime Header in Second Message Body */
 if (sip_bcpt_setMimeHeaderInMsgBody(pSecondMsgBody,pSecondMimeHdr,pErr) == SipFail )
 {
	 printf("Error = %d in setting MimeHeader in message body \n",*pErr) ;
   sip_bcpt_freeSipMimeHeader(pFirstMimeHdr) ;
   sip_freeSipMsgBody(pFirstMsgBody) ;
	 sip_bcpt_freeSipMimeHeader(pSecondMimeHdr) ;
	 sip_freeSipMsgBody(pSecondMsgBody) ;
	 return SipFail ;
 }

 /* Insert pFirstMsgBody to slMessageBody of SIpMessage */
 if ( sip_listInsertAt(&(pSipMsg->slMessageBody),0,(void*)pFirstMsgBody,pErr) == SipFail )
 {
	 printf("Error = %d in inserting MsgBody in list of message body in sipmsg\n",*pErr) ;
   sip_bcpt_freeSipMimeHeader(pFirstMimeHdr) ;
   sip_freeSipMsgBody(pFirstMsgBody) ;
	 sip_bcpt_freeSipMimeHeader(pSecondMimeHdr) ;
	 sip_freeSipMsgBody(pSecondMsgBody) ;
	 return SipFail ;	 
 }
 /* Insert pSecondMsgBody to slMessageBody of SIpMessage */
 if ( sip_listInsertAt(&(pSipMsg->slMessageBody),1,(void*)pSecondMsgBody,pErr) == SipFail)
 {
	 printf("Error = %d in inserting MsgBody in list of message body in sipmsg\n",*pErr) ;
   sip_bcpt_freeSipMimeHeader(pFirstMimeHdr) ;
   sip_freeSipMsgBody(pFirstMsgBody) ;
	 sip_bcpt_freeSipMimeHeader(pSecondMimeHdr) ;
	 sip_freeSipMsgBody(pSecondMsgBody) ;
	 return SipFail ;	 
 }

 printf("Exiting  buildMultipartMixedBody() successfully \n") ;
 return SipSuccess ;
}








/************************************************************************************
** Function Name :	sendInviteRequestWithMsgBody								   **
** Description   :  This function will send invite-request with message bodies	   **
** Parameters    :																   **
**                    bodyFlag -> msg-body to be included in message			   **
**                   pSendAddr -> address where the msg will be sent			   **
**                     pErr    -> Error will be returned with this variable		   **
************************************************************************************/

SipBool sendInviteRequestWithMsgBody(int bodyFlag,SipTranspAddr *pSendAddr,SipError *pErr)
{
  SipMessage * pSipMsg = SIP_NULL ;
  SipReqLine * pReqLine = SIP_NULL ;
  SipHeader  * pHeader = SIP_NULL ;
  SipEventContext * pEventContext = SIP_NULL ;
#ifdef SIP_TXN_LAYER
  SipTxnContext * pTxnContext = SIP_NULL ;
  SipTxnKey * pTxnKey =SIP_NULL;
  en_SipTxnBypass dbypass;
  SIP_S8bit *pTag = SIP_NULL ;
#else
	SipOptions options;
#endif
  SipAddrSpec * pAddrSpec = SIP_NULL ;
  SipUrl * pSipUrl = SIP_NULL ;
  SipBool res=SipSuccess ;
  /* SipTranspAddr * pTempTranspAddr = SIP_NULL ; */
  SipMsgBody *pSipMsgBody = SIP_NULL ;
  SipParam * pBoundaryParam = SIP_NULL ;
  int flag = 0 ;

  /*
     First build the mandatory headers and then build the message body
     depending upon the bodyFlag and send the message formed to
     SendAddr.
  */

   /* Build all mandatory headers */
   if(sip_initSipMessage(&pSipMsg,SipMessageRequest,pErr)==SipFail)
	{
		printf("Error = %d while Initialize message.\n",*pErr);
		return SipFail ;
	}
    /* Build ReqLine Header */
	if ( sip_initSipReqLine(&pReqLine,pErr) == SipFail)
	{
		printf("Error = %d while Initialize Reqline of message.\n",*pErr);
		sip_freeSipMessage(pSipMsg) ;
		return SipFail ;
	}

#ifdef SIP_BY_REFERENCE
	/* Set method in reqline header */
	if (sip_setMethodInReqLine(pReqLine,strdup("INVITE"),pErr) == SipFail )
   	{
		printf("Error = %d while Setting Method in ReqLine.\n",*pErr);
		sip_freeSipMessage(pSipMsg) ;
		sip_freeSipReqLine(pReqLine) ;
		return SipFail ;
	}

	/* Set version in reqline header */
	if ( sip_setVersionInReqLine(pReqLine,strdup("SIP/2.0"),pErr) == SipFail )
	{
		printf("Error = %d while Setting Method in ReqLine.\n",*pErr);
		sip_freeSipMessage(pSipMsg) ;
		sip_freeSipReqLine(pReqLine) ;
		return SipFail ;
	}
#endif


	sip_initSipAddrSpec(&pAddrSpec,SipAddrSipUri,pErr);
	sip_initSipUrl(&pSipUrl,pErr);
#ifdef SIP_BY_REFERENCE
	sip_setHostInUrl(pSipUrl,strdup("taqua.com"),pErr);
#endif
	sip_setUrlInAddrSpec(pAddrSpec,pSipUrl,pErr);
	sip_setAddrSpecInReqLine(pReqLine,pAddrSpec, pErr);
	sip_setReqLineInSipReqMsg (pSipMsg, pReqLine, pErr);
	sip_freeSipUrl(pSipUrl) ;
	sip_freeSipAddrSpec(pAddrSpec) ;
	sip_freeSipReqLine(pReqLine);

	/* Building From Header */
	sip_initSipHeader(&pHeader,SipHdrTypeFrom,pErr);
	sip_initSipAddrSpec(&pAddrSpec,SipAddrSipUri,pErr);
	sip_initSipUrl(&pSipUrl,pErr);
#ifdef SIP_BY_REFERENCE
	sip_setHostInUrl(pSipUrl,strdup("taqua.us.com"),pErr);
#endif
	sip_setUrlInAddrSpec(pAddrSpec,pSipUrl,pErr);
	sip_setAddrSpecInFromHdr(pHeader,pAddrSpec,pErr);
	sip_freeSipUrl(pSipUrl) ;
	sip_freeSipAddrSpec(pAddrSpec);
#ifdef SIP_TXN_LAYER
    /* Inserting from-tag in from-header */
	pTag = strdup("ajkhdd89347");
	sip_insertTagAtIndexInFromHdr(pHeader,pTag,0,pErr);
#endif
	/* Inserting From Header into SipMsg */
	sip_setHeader(pSipMsg, pHeader, pErr);
	sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
	free(pHeader);
#endif

	/* Building To Header */
	sip_initSipHeader(&pHeader,SipHdrTypeTo,pErr);
	sip_initSipAddrSpec(&pAddrSpec,SipAddrSipUri,pErr);
	sip_initSipUrl(&pSipUrl,pErr);
#ifdef SIP_BY_REFERENCE
	sip_setHostInUrl(pSipUrl,strdup("yourdomain.com"),pErr);
#endif
	sip_setUrlInAddrSpec(pAddrSpec,pSipUrl,pErr);
	sip_setAddrSpecInToHdr(pHeader,pAddrSpec,pErr);
	sip_freeSipUrl(pSipUrl);
	sip_freeSipAddrSpec(pAddrSpec);
    /* Inserting To Header into SipMsg*/
	sip_setHeader(pSipMsg, pHeader, pErr);
	sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
	free(pHeader);
#endif

   /* Building Call-Id Header */
	sip_initSipHeader(&pHeader,SipHdrTypeCallId,pErr);
#ifdef SIP_BY_REFERENCE
	sip_setValueInCallIdHdr(pHeader,strdup("1234324453@taqua.com"),pErr);
#endif
	/* Inserting Call-Id Header into SipMsg*/
	sip_setHeader(pSipMsg, pHeader, pErr);
	sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
	free(pHeader);
#endif

	/* Building CSeq header */
	sip_initSipHeader(&pHeader,SipHdrTypeCseq,pErr);
	sip_setSeqNumInCseqHdr(pHeader,1,pErr);
#ifdef SIP_BY_REFERENCE
	sip_setMethodInCseqHdr(pHeader,strdup("INVITE"),pErr);
#endif
	/* Inserting CSeq header in SipMsg */
	sip_setHeader(pSipMsg, pHeader, pErr);
	sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
	free(pHeader);
#endif


	/* Building and Inserting Via header into SipMsg*/
	if((glbtransptype&SIP_UDP) ==SIP_UDP)
	{
#ifdef SIP_BY_REFERENCE
	sip_insertHeaderFromStringAtIndex(pSipMsg,SipHdrTypeVia,(SIP_S8bit *)\
		"Via: SIP/2.0/UDP 135.180.130.133 (Via header comment); branch=taqua",\
		0,pErr);
#endif
	}
else
	{
#ifdef SIP_BY_REFERENCE
	sip_insertHeaderFromStringAtIndex(pSipMsg,SipHdrTypeVia,(SIP_S8bit *)\
		"Via: SIP/2.0/TCP 135.180.130.133 (Via header comment); branch=taqua",\
		0,pErr);
#endif
	}

   /* Building and Inserting Contact Header */
	sip_insertHeaderFromStringAtIndex(pSipMsg,SipHdrTypeContactAny,\
		(SIP_S8bit *) "Contact: sip:bfelice@taqua.com",0,pErr);


	/* Building Content-Length-Header  */
	sip_initSipHeader(&pHeader,SipHdrTypeContentLength,pErr);
	sip_setLengthInContentLengthHdr(pHeader,400,pErr);
	/* Inserting Content-Length-Header into pSipMsg */
	sip_setHeader(pSipMsg, pHeader, pErr);
	sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
	free(pHeader);
#endif




    /* Start Building Msg Body
	First build content type header
	Then as per content-type header build message body
	*/
	switch (bodyFlag)
	{
	case '6' : /* Build sdp message body */
		{
				/* Building Content-Type-Header  */
				if ( sip_initSipHeader(&pHeader,SipHdrTypeContentType,pErr) == SipFail)
				{
					printf("Error = %d while initializing header = SipHdrTypeContentType\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					return SipFail ;
				}
				if ( sip_setMediaTypeInContentTypeHdr(pHeader,strdup("application/sdp"),pErr) == SipFail )
				{
					printf("Error = %d while setting header = SipHdrTypeContentType\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					sip_freeSipHeader(pHeader);
#ifdef SIP_BY_REFERENCE
				free(pHeader);
#endif
					return SipFail ;
				}
				/* Inserting Content-Type-Header into pSipMsg */
				if ( sip_setHeader(pSipMsg, pHeader, pErr) == SipFail )
				{
					printf("Error = %d while setting header = SipHdrTypeContentType\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					sip_freeSipHeader(pHeader);
#ifdef SIP_BY_REFERENCE
				free(pHeader);
#endif
					return SipFail ;
				}

				sip_freeSipHeader(pHeader);
#ifdef SIP_BY_REFERENCE
				free(pHeader);
#endif

			   if (  (pSipMsgBody = buildSdpBody(pErr)) == SIP_NULL )
			   {
             	 printf("\nError = %d while building sdp-msg-body \n",*pErr) ;
			     return SipFail ;
			   }

			  break ;
		}
	case '7' : /* Build multipart-mixed body */
		{
				/* Building Content-Type-Header  */
				if ( sip_initSipHeader(&pHeader,SipHdrTypeContentType,pErr) == SipFail )
				{
					printf("Error = %d while initializing header = SipHdrTypeContentType\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					return SipFail ;
				}

				if ( sip_setMediaTypeInContentTypeHdr(pHeader,strdup("multipart/mixed"),pErr) == SipFail )
				{
					printf("Error = %d while setting media-type in header = SipHdrTypeContentType\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					return SipFail ;
				}

				if ( sip_initSipParam(&pBoundaryParam,pErr) == SipFail )
				{
					printf("Error = %d while initializing SipParam (boundary)\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					return SipFail ;
				}

				if ( sip_setNameInSipParam(pBoundaryParam,strdup("boundary"),pErr) == SipFail )
				{
					printf("Error = %d while setting name in SipParam (boundary)\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					sip_freeSipParam(pBoundaryParam) ;
					return SipFail ;
				}

				if ( sip_insertValueAtIndexInSipParam(pBoundaryParam,strdup("uniquetag"),0,pErr) == SipFail )
				{
					printf("Error = %d while inserting value in SipParam (boundary)\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					sip_freeSipParam(pBoundaryParam) ;
					return SipFail ;
				}

				if (  sip_insertParamAtIndexInContentTypeHdr(pHeader,pBoundaryParam,0,pErr) == SipFail )
				{
					printf("Error = %d while inserting param SipParam (boundary)\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					sip_freeSipParam(pBoundaryParam) ;
					return SipFail ;
				}

				/* Inserting Content-Type-Header into pSipMsg */
				if ( sip_setHeader(pSipMsg, pHeader, pErr) == SipFail )
				{	
					printf("Error = %d while setting header in sip-msg\n",*pErr) ;
					sip_freeSipMessage(pSipMsg) ;
					sip_freeSipParam(pBoundaryParam) ;
					return SipFail ;
				}
				
				sip_freeSipParam(pBoundaryParam) ;
				sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
				free(pHeader);
#endif

			if ( buildMultipartMixedBody(pSipMsg,pErr) == SipFail )
			{
				 printf("\nError = %d while building multipart-msg-body \n",*pErr) ;
				 return SipFail ;
			}
			    flag = 1;
			    break ;
		}
    case '8' : /* Build Isup body */
		{
				/* Building Content-Type-Header  */
				sip_initSipHeader(&pHeader,SipHdrTypeContentType,pErr);
				sip_setMediaTypeInContentTypeHdr(pHeader,strdup("application/isup"),pErr) ;
				/* Inserting Content-Type-Header into pSipMsg */
				sip_setHeader(pSipMsg, pHeader, pErr);
				sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
				free(pHeader);
#endif

			    if ( (pSipMsgBody = buildIsupBody(pErr)) == SIP_NULL )
				{
				 printf("\nError = %d while building isup-msg-body \n",*pErr) ;
			     return SipFail ;
				}
			    break ;
		}
	default :
		{
				/* Building Content-Type-Header  */
				sip_initSipHeader(&pHeader,SipHdrTypeContentType,pErr);
				sip_setMediaTypeInContentTypeHdr(pHeader,strdup("application/sdp"),pErr) ;
				/* Inserting Content-Type-Header into pSipMsg */
				sip_setHeader(pSipMsg, pHeader, pErr);
				sip_freeSipHeader(pHeader);

#ifdef SIP_BY_REFERENCE
				free(pHeader);
#endif

			    printf("BodyFlag = %d is wrong.So Isup-Msg is being built by default \n",bodyFlag) ;
			    if ( (pSipMsgBody = buildIsupBody(pErr)) == SIP_NULL )
				{
				 printf("\nError = %d while building isup-msg-body \n",*pErr) ;
			     return SipFail ;
				}
			    break ;
		}

	} /* switch */

	if ( flag != 1 )
    {

	 if( sip_listInit(&(pSipMsg->slMessageBody),__sip_freeSipMsgBody,pErr)==SipFail)
	 {
          sip_freeSipMsgBody(pSipMsgBody) ;
	   	  return SipFail ;
	 }

	 if ( sip_listInsertAt(&(pSipMsg->slMessageBody),0,(void *)pSipMsgBody,pErr) == SipFail )
	 {
         sip_freeSipMsgBody(pSipMsgBody) ;
		 return SipFail ;
	 }
	}
	
	printf("\n\n** Successfully added the built-sdp-msg into SIP-MSG.**\n\n") ;


	/* Fill in Event Context Structure */
	/* Note that there is no sip_stack_init_api for the same */
	if ( (pEventContext = (SipEventContext* ) malloc(sizeof(SipEventContext))) ==  SIP_NULL )
	{
		sip_freeSipMessage(pSipMsg) ;
		return SipFail ;
	}
	pEventContext->pDirectBuffer = SIP_NULL ;
	pEventContext->pData         = SIP_NULL ;

#ifdef SIP_TXN_LAYER
	/* Initialize the txn_context */
	sip_txn_initSipTxnContext(&pTxnContext,pErr);
	pTxnContext->pEventContext= pEventContext;

    pTxnContext->txnOption.dOption.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | SIP_OPT_RETRANSCALLBACK;

	/*  dBypass variable specifies whether the transaction is to be fetched or a new transaction is to
		be created for this message. 
		If this value is set to SipTxnByPass, it will forward the message that is passed to function and 
		will not try to fetch the transaction or to create a new transaction for this message. This 
		functionality is used in case of proxy, where the message needs to be forwarded without fetching 
		the transaction for it.
		If this value is set to SipTxnNoByPass ,sip_txn_sendMessage() tries to create/fetch transaction
		for the message.
	*/
	dbypass = SipTxnNoByPass;

	if(!glbtransactiontype)
		pTxnContext->dTxnType = SipUATypeTxn;
	else
		pTxnContext->dTxnType = SipProxyTypeTxn;

	/*  
	 The application might not need the timeout cbks of all timers. The application can 
	 indicate the timers	it has interest in using the parameter dTimeoutCbkOption.(it is 
	 typical bitmask parameter)
	 By default it is set to all_timers (SIP_ISSUE_CBKFOR_ALLTIMERS)
	 Refer to siptxnstruct.h for more information on this.
	*/
	pTxnContext->txnOption.dTimeoutCbkOption=glbTimeTimeoutOption;

	
	/* These new timer values are applied to all the future timers involved in 
	   the transaction. */
    if(glbsetTimerValue)
	{
		pTxnContext->timeoutValues.dT1          = dTimeOut.dT1;
		pTxnContext->timeoutValues.dT2		    = dTimeOut.dT2;
		pTxnContext->timeoutValues.dTimerB	    = dTimeOut.dTimerB;
		pTxnContext->timeoutValues.dTimerC	    = dTimeOut.dTimerC;
		pTxnContext->timeoutValues.dTimerD_T3	= dTimeOut.dTimerD_T3;
		pTxnContext->timeoutValues.dTimerF_T3	= dTimeOut.dTimerF_T3;
		pTxnContext->timeoutValues.dTimerH	    = dTimeOut.dTimerH;
		pTxnContext->timeoutValues.dTimerI_T4	= dTimeOut.dTimerI_T4;
		pTxnContext->timeoutValues.dTimerJ_T3	= dTimeOut.dTimerJ_T3;
		pTxnContext->timeoutValues.dTimerK_T4	= dTimeOut.dTimerK_T4;
		pTxnContext->txnOption.dTimerOption     = SIP_OPT_TIMERALL;
	}


/*	pTempTranspAddr=(SipTranspAddr *)fast_memget\
						(0,sizeof(SipTranspAddr),pErr);
	*pTempTranspAddr=*pSendAddr;

	pTempTranspAddr=pSendAddr;
*/
	res = sip_txn_sendMessage(pSipMsg,pSendAddr,glbtransptype,\
				pTxnContext, dbypass,&pTxnKey,pErr);
	if(res == SipFail)
	{
		switch(*pErr)
		{
			case E_TXN_NO_EXIST:
				printf("** ERROR : The Txn for this message doesn't exist**\n");
				fflush(stdout);
				break;
			case E_TXN_EXISTS:
				printf("** ERROR : The Txn for this message already exists**\n");
				fflush(stdout);
				break;
			case E_TXN_INV_STATE:
				printf("** ERROR : This message leads to an invalid transaction state**\n");
				fflush(stdout);
				break;
			case E_TXN_INV_MSG:
				printf("** ERROR : This is an invalid message for received for the Txn**\n");
				fflush(stdout);
				break;
			default:
				break;
		}
			sip_freeEventContext(pEventContext);
			printf("** ERROR = %d : Send Message Failed**\n",*pErr);
	}
	else
	{
		/*  
		    Space for txn_key is allocated in sip_txn_sendMessage() 
			so free this once its work is over
			Incase of error-leg (like above),application does not need
			to free this as it is done by sip_txn_sendMessage().
         */
			sip_txn_freeTxnKey(pTxnKey,pErr);
	}
	free(pTxnContext);
#else
    options.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | SIP_OPT_RETRANSCALLBACK;
		res = sip_sendMessage(pSipMsg,&options,pSendAddr,glbtransptype,\
				pEventContext,pErr);
#endif
	/* Free SipMsg , deallocate all the memory/space taken*/
    sip_freeSipMessage(pSipMsg) ;

  return SipSuccess ;
}
#endif/* #ifdef SIP_BY_REFERENCE */
#endif

/* Function used to send a simple response */
SipBool sendRequest(const char *method,SipTranspAddr *sendaddr,SipError *err)
{
	SipMessage *sipmesg;
	SipReqLine *reqline;
	SipHeader *header;
	SipAddrSpec *addrspec;
	SipUrl *sipurl;
	SipBool res;
	SipEventContext *context;
	char contextstr[100];
	SipOptions options;
#ifndef SIP_TXN_LAYER
	int T1,T2;
	char retrans;
#else
	SipTranspAddr	*pTempTranspAddr=SIP_NULL;
	char *pTag;
#endif

	char unknownMethod[100];

	if ( strcasecmp(method,"UNKNOWN") == 0 )
	{
		printf("Method To be Sent:");
		fflush(stdout);
		scanf("%s",unknownMethod);
		fflush(stdin);
	}

	if(sip_initSipMessage(&sipmesg,SipMessageRequest,err)==SipFail)
	{
		printf("Could not form message.\n");
		fflush(stdout);
		exit(0);
	}
	sip_initSipReqLine(&reqline,err);
#ifdef SIP_BY_REFERENCE
	if ( strcasecmp(method,"UNKNOWN") == 0 )
		sip_setMethodInReqLine(reqline,strdup(unknownMethod),err);
	else
		sip_setMethodInReqLine(reqline,strdup(method),err);
#else
	if ( strcasecmp(method,"UNKNOWN") == 0 )
		sip_setMethodInReqLine(reqline,(SIP_S8bit *)unknownMethod,err);
	else
		sip_setMethodInReqLine(reqline,(SIP_S8bit *)method,err);
#endif
#ifdef SIP_BY_REFERENCE
	sip_setVersionInReqLine(reqline,strdup("SIP/2.0"),err);
#else
	sip_setVersionInReqLine(reqline,(SIP_S8bit *)"SIP/2.0",err);
#endif
	sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err);
	sip_initSipUrl(&sipurl,err);
#ifdef SIP_BY_REFERENCE
	sip_setHostInUrl(sipurl,strdup("mydomain.com"),err);
#else
	sip_setHostInUrl(sipurl,(SIP_S8bit *)"mydomain.com",err);
#endif
	sip_setUrlInAddrSpec(addrspec,sipurl,err);
	sip_setAddrSpecInReqLine(reqline,addrspec,err);
	sip_setReqLineInSipReqMsg (sipmesg, reqline, err);
	sip_freeSipReqLine(reqline);

	sip_initSipHeader(&header,SipHdrTypeFrom,err);
	sip_setAddrSpecInFromHdr(header,addrspec,err);

	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
#ifdef SIP_TXN_LAYER
	pTag = strdup("ajkhdd89347");
	sip_insertTagAtIndexInFromHdr(header,pTag,0,err);
#endif
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	sip_initSipHeader(&header,SipHdrTypeTo,err);
	sip_initSipAddrSpec(&addrspec,SipAddrSipUri,err);
	sip_initSipUrl(&sipurl,err);
#ifdef SIP_BY_REFERENCE
	sip_setHostInUrl(sipurl,strdup("yourdomain.com"),err);
#else
	sip_setHostInUrl(sipurl,(SIP_S8bit *)"yourdomain.com",err);
#endif
	sip_setUrlInAddrSpec(addrspec,sipurl,err);
	sip_setAddrSpecInToHdr(header,addrspec,err);
	sip_freeSipUrl(sipurl);
	sip_freeSipAddrSpec(addrspec);
#ifdef SIP_TXN_LAYER
	if((strcmp(method,"ACK")==0) || (strcmp(method,"PRACK")==0))
	{
		pTag = strdup("ajkhdd89347");
		sip_insertTagAtIndexInToHdr(header,pTag,0,err);
	}
#endif

	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	sip_initSipHeader(&header,SipHdrTypeCallId,err);
#ifdef SIP_BY_REFERENCE
	sip_setValueInCallIdHdr(header,strdup("1234324453@mydomain.com"),err);
#else
	sip_setValueInCallIdHdr(header,(SIP_S8bit *)"1234324453@mydomain.com",err);
#endif
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	sip_initSipHeader(&header,SipHdrTypeCseq,err);
	if((strcmp(method,"PRACK")==0)||(strcmp(method,"BYE")==0))
		sip_setSeqNumInCseqHdr(header,2,err);
	else
		sip_setSeqNumInCseqHdr(header,1,err);
#ifdef SIP_BY_REFERENCE
	sip_setMethodInCseqHdr(header,strdup(method),err);
#else
	sip_setMethodInCseqHdr(header,(SIP_S8bit *)method,err);
#endif
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	/* Convenience functions for inserting headers from strings into
	   the message*/
if((glbtransptype&SIP_UDP) ==SIP_UDP)
{
#ifdef SIP_BY_REFERENCE
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *)\
		"Via: SIP/2.0/UDP 135.180.130.133 (Via header comment); branch=hughes",\
		0,err);
#else
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *)\
		"Via: SIP/2.0/UDP 135.180.130.133 (Via header comment); branch=hughes",\
		0,err);
#endif
}
else
{
#ifdef SIP_BY_REFERENCE
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *)\
		"Via: SIP/2.0/TCP 135.180.130.133 (Via header comment); branch=hughes",\
		0,err);
#else
	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeVia,(SIP_S8bit *)\
		"Via: SIP/2.0/TCP 135.180.130.133 (Via header comment); branch=hughes",\
		0,err);
#endif
}

	sip_insertHeaderFromStringAtIndex(sipmesg,SipHdrTypeContactAny,\
		(SIP_S8bit *) "Contact: sip:kbinu@hss.hns.com",0,err);

	/* Header structures may also be formed from strings */
#ifdef SIP_BY_REFERENCE
	sip_initSipHeader(&header, SipHdrTypeAny,err);
#else
	sip_initSipHeader(&header, SipHdrTypeContactAny,err);
#endif
	sip_parseSingleHeader((SIP_S8bit *)\
		"Contact: S Shetti <sip:sshetti@hss.hns.com>",\
		SipHdrTypeContactAny, header, err);
	sip_insertHeaderAtIndex(sipmesg, header, 0, err);
	sip_freeSipHeader(header);
#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	sip_initSipHeader(&header,SipHdrTypeContentLength,err);
	sip_setLengthInContentLengthHdr(header,0,err);
	sip_setHeader(sipmesg, header, err);
	sip_freeSipHeader(header);

#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	if(strcmp(method,"PRACK")==0)
	{
		sip_initSipHeader(&header,SipHdrTypeRAck,err);
		sip_rpr_setRespNumInRAckHdr(header,1,err);
		sip_rpr_setCseqNumInRAckHdr(header,1,err);
#ifdef SIP_BY_REFERENCE
		sip_rpr_setMethodInRAckHdr(header,strdup("INVITE"),err);
#else
		sip_rpr_setMethodInRAckHdr(header,(SIP_S8bit *)"INVITE",err);
#endif
		sip_setHeader(sipmesg, header, err);
		sip_freeSipHeader(header);
#ifdef SIP_BY_REFERENCE
	free(header);
#endif

	}

#ifdef AUTH_CSR
{
    SipHeader *pHeader=SIP_NULL ;
		SipError err = E_NO_ERROR ;
		SipGenericChallenge* pChallenge = SIP_NULL ;
		SipGenericCredential* pCred = SIP_NULL ;
		SIP_S8bit * pScheme = strdup("Digest") ;
		SipParam *pSipParam = SIP_NULL ;
		SIP_S8bit *pName[3] ;
		SIP_S8bit *pValue[3] = { "RegisteredUser", "auth","1291030920071106" } ;
		SIP_U32bit i=0 ;

		pName[0] = strdup("realm") ;
		pName[1] = strdup("qop") ;
		pName[2] = strdup("opaque") ;
		
   /*-------------------------------------------
    * Forming Authorization Header
    * ------------------------------------------*/

   /*
    * Initialize Sip Header
    */ 
    if ( sip_initSipHeader(&pHeader,SipHdrTypeAuthorization,&err) == SipFail )
    {
        printf("\nError in sip_initSipHeader : %d \n",err) ;
    }
		
    /*
     * Initialize Sip Generic Credentials
     */ 
     if ( sip_initSipGenericCredential(&pCred,SipCredAuth,&err) == SipFail )
     {
          printf("\nError in sip_initSipGenericCredential : %d \n",err) ;
     }
    /*
     * Initialize Sip Generic Challenge
     */ 
     if ( sip_initSipGenericChallenge(&pChallenge,&err) == SipFail )
     {
          printf("\nError in sip_initSipGenericChallenge : %d \n",err) ;
     }
     /*
      * Set Scheme in Challenge 
      */
      if ( sip_setSchemeInChallenge(pChallenge,pScheme,&err) == SipFail )
      {
          printf("\nError in sip_setSchemeInChallenge : %d \n",err) ;
      }

    /*
     * Inserting Authorization Parameter in Generic Challenge
     */ 
    for (i=0;i<3;i++)
    {
       SIP_S8bit *pQuotedValue = quoteString(pValue[i]) ;

        sip_initSipParam(&pSipParam,&err) ;
        if ( sip_setNameInSipParam(pSipParam,pName[i],&err) == SipFail )
        {
            printf("\nError in sip_setNameInSipParam : %d \n",err) ;
            sip_freeSipParam(pSipParam) ;
        }
        if ( sip_insertValueAtIndexInSipParam(pSipParam,pQuotedValue,0,&err) == SipFail)
        {
            printf("\nError in sip_insertValueAtIndexInSipParam : %d \n",err) ;
            sip_freeSipParam(pSipParam) ;
        }
        if ( sip_insertAuthorizationParamAtIndexInChallenge(pChallenge,pSipParam,i,&err) == SipFail) 
        {
            printf("\nError in sip_insertAuthorizationParamAtIndexInChallenge : %d \n",err) ;
            sip_freeSipParam(pSipParam) ;
        }
        sip_freeSipParam(pSipParam) ;
    }

    /*
     * Set Challenge in Credential
     */ 
     if ( sip_setChallengeInCredential(pCred,pChallenge,&err) == SipFail)
     {
          printf("\nError in sip_setChallengeInCredential : %d \n",err) ;
     }

     /* 
      * Set Credentials in Authorization header
      */ 
      if ( sip_setCredentialsInAuthorizationHdr (pHeader,pCred,&err) == SipFail )
      {
           printf("\nError in sip_setCredentialsInAuthorizationHdr : %d \n",err) ;
      }
      else
      {
         /* 
          * Insert Authorization header at index  0 in sip-mesg
          */ 
          if ( sip_insertHeaderAtIndex(sipmesg,pHeader,0, &err) == SipFail )
          {
               printf("\nError in sip_setHeader : %d \n",err) ;
          }
          else
          {
               printf("\nSet Authorization header successfully \n") ;
          }
      }

      /*
       * Free Sip Generic Credential
       */ 
      sip_freeSipGenericCredential(pCred) ;
      sip_freeSipGenericChallenge(pChallenge) ;
      sip_freeSipHeader(pHeader);
#ifdef SIP_BY_REFERENCE
      free(pHeader);
#endif
}
#endif

	context = (SipEventContext* ) malloc(sizeof(SipEventContext));
	sprintf(contextstr,"%s request ",method);
	context->pData = strdup(contextstr);
	context->pDirectBuffer = SIP_NULL;
	options.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | \
		SIP_OPT_RETRANSCALLBACK;

#ifndef SIP_TXN_LAYER
	if(glbSipOption==1)
	{
		fflush(stdin);
		fflush(stdout);
		printf("\n Do you want	retransmission (y/n):[y]");
		fflush(stdin);
		fflush(stdout);
		retrans=sip_getchar();
		if((retrans=='n')||(retrans=='N'))
		{
			res = sip_sendMessage(sipmesg,&options,sendaddr,\
				(SIP_S8bit)(glbtransptype|SIP_NORETRANS),context,err);
			if(res==SipSuccess)
			{
				printf("%s sent successfully.\n",method);
				fflush(stdin);
				fflush(stdout);
			}
			sip_freeSipMessage(sipmesg);
			glbSipOption=0;
			return SipSuccess;
		}
		else
		{
			char value[10];
			options.dOption |= SIP_OPT_PERMSGRETRANS;
			fflush(stdin);
			fflush(stdout);
			printf("\n enter value of T1:");
			fflush(stdin);
			fflush(stdout);
			scanf("%s",value);
			T1=atoi(value);
			fflush(stdin);
			fflush(stdout);
			printf("\n enter value of T2:");
			fflush(stdin);
			fflush(stdout);
			scanf("%s",value);
			T2=atoi(value);
			fflush(stdin);
			fflush(stdin);
			printf("\n Maximum Retransmission for INVITE:");
			fflush(stdin);
			fflush(stdout);
			scanf("%s",value);
			SIP_MAXINVRETRANS=atoi(value)-1;
			fflush(stdin);
			fflush(stdout);
			printf("\n Maximum Retransmission for other Request:");
			fflush(stdin);
			fflush(stdout);
			scanf("%s",value);
			SIP_MAXRETRANS=atoi(value)-1;
			context->dRetransT1 = T1;
			context->dRetransT2 = T2;
		}
	}
#endif

/*	Uncomment to test per message retransmission interval setting.
	context->dRetransT1 = 2000;
	context->dRetransT2 = 8000;
	options.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | SIP_OPT_PERMSGRETRANS;
*/


/*	Uncomment to test per message retransmission interval setting with
	retranscount
	context->dRetransT1 = 2000;
	context->dRetransT2 = 8000;
	context->dMaxRetransCount = 4;
	context->dMaxInviteRetransCount = 2;
	options.dOption = SIP_OPT_FULLFORM | SIP_OPT_SINGLE | SIP_OPT_PERMSGRETRANS\
						SIP_OPT_PERMSGRETRANSCOUNT;
*/

/* Uncomment to test for sending direct buffer into sendMessage
{
	char* sendbuffer;
	int dLength;
	sendbuffer= (char*) malloc(SIP_MAX_MSG_SIZE);
	if(sendbuffer== SIP_NULL)
			return SipFail;
	if(sip_formMessage(sipmesg, &options, sendbuffer, &dLength, err) == SipFail)
	{
			free(sendbuffer);
			return SipFail;
	}
	options.dOption |= SIP_OPT_DIRECTBUFFER;
	context->pDirectBuffer = (SIP_Pvoid) sendbuffer;
*/


#ifdef SIP_TXN_LAYER
	{
		SipTxnContext *txncontext;
		SipTxnKey *pTxnKey =SIP_NULL;
		en_SipTxnBypass dbypass;
		sip_txn_initSipTxnContext(&txncontext,err);
		txncontext->pEventContext= context;
/*		options.dOption=options.dOption | SIP_OPT_DIRECTBUFFER;*/
		txncontext->txnOption.dOption = options;

		dbypass = SipTxnNoByPass;

		if(!glbtransactiontype)
			txncontext->dTxnType = SipUATypeTxn;
		else
			txncontext->dTxnType = SipProxyTypeTxn;

		txncontext->txnOption.dTimeoutCbkOption=glbTimeTimeoutOption;

		if(glbsetTimerValue)
		{
			txncontext->timeoutValues.dT1 = dTimeOut.dT1;
			txncontext->timeoutValues.dT2		= dTimeOut.dT2;
			txncontext->timeoutValues.dTimerB	= dTimeOut.dTimerB;
			txncontext->timeoutValues.dTimerC	= dTimeOut.dTimerC;
			txncontext->timeoutValues.dTimerD_T3	= dTimeOut.dTimerD_T3;
			txncontext->timeoutValues.dTimerF_T3	= dTimeOut.dTimerF_T3;
			txncontext->timeoutValues.dTimerH	= dTimeOut.dTimerH;
			txncontext->timeoutValues.dTimerI_T4	= dTimeOut.dTimerI_T4;
			txncontext->timeoutValues.dTimerJ_T3	= dTimeOut.dTimerJ_T3;
			txncontext->timeoutValues.dTimerK_T4	= dTimeOut.dTimerK_T4;
			txncontext->txnOption.dTimerOption = SIP_OPT_TIMERALL;
		}

		
/*		{
			sendbuffer= (char*) malloc(SIP_MAX_MSG_SIZE);
			if(sendbuffer== SIP_NULL)
				return SipFail;
			if(sip_formMessage\
					(sipmesg, &options, sendbuffer, &dLength, err) == SipFail)
			{
				free(sendbuffer);
				return SipFail;
			}
			context->pDirectBuffer = (SIP_Pvoid) sendbuffer;
		}*/
		
		pTempTranspAddr=(SipTranspAddr *)fast_memget\
						(0,sizeof(SipTranspAddr),err);
		*pTempTranspAddr=*sendaddr;
		res = sip_txn_sendMessage(sipmesg,pTempTranspAddr,glbtransptype,\
				txncontext, dbypass,&pTxnKey,err);
		if(res == SipFail)
		{
			sip_freeEventContext(context);
			switch(*err)
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
					break;
			}
			printf("** ERROR : Send Message Failed**\n");
			fflush(stdout);
		}
		else
		{
			sip_txn_freeTxnKey(pTxnKey,err);
		}
		free(txncontext);
	}
#else
	res = sip_sendMessage(sipmesg,&options,sendaddr,glbtransptype,\
		context,err);
#endif
	sip_freeSipMessage(sipmesg);
/* Uncomment to test for sending direct buffer into sendMessage
}
*/
	if(res==SipSuccess)
	{
		printf("%s sent successfully.\n",method);
		fflush(stdin);
		fflush(stdout);
	}
	glbSipOption=0;
	return SipSuccess;
}


void showErrorInformation(SipMessage *pMessage)
{
	SipError err;
	SIP_U32bit count;
#ifdef SIP_BY_REFERENCE
	SIP_U32bit i;
#endif

	printf("\n");
	fflush(stdout);
	printf("-----Error Information in decoded message ----------------\n");
	fflush(stdout);
	if(sip_getIncorrectHeadersCount(pMessage, &count, &err)!=SipFail)
	{
		printf("Errors in parsing headers    : %d\n", count);
		fflush(stdout);
	}
	if(sip_getEntityErrorCount(pMessage, &count, &err)!=SipFail)
	{
		printf("Errors in parsing entity body: %d\n", count);
		fflush(stdout);
	}
#ifdef SIP_BY_REFERENCE
	if(sip_getBadHeaderCount(pMessage, &count, &err)==SipFail)
		return;
	for(i=0;i<count;i++)
	{
		SipBadHeader	*pBadHeader;
		SIP_S8bit		*pStr;
		if(sip_getBadHeaderAtIndex(pMessage, &pBadHeader, i, &err)==SipFail)
			return;
		printf("Bad header in message.\n");
		fflush(stdout);
		if(sip_getNameFromBadHdr(pBadHeader, &pStr, &err)==SipFail)
		{
			sip_freeSipBadHeader(pBadHeader);
			return;
		}
		printf("Name: %s\n", pStr);
		fflush(stdout);
		if(sip_getBodyFromBadHdr(pBadHeader, &pStr, &err)==SipFail)
		{
			sip_freeSipBadHeader(pBadHeader);
			return;
		}
		printf("Body: %s\n", pStr);
		fflush(stdout);
		sip_freeSipBadHeader(pBadHeader);
	}
#endif
	printf("----------------------------------------------------------\n");
	fflush(stdout);
}
#ifdef SIP_TXN_LAYER
SIP_U32bit getTimerTimeoutOptions(void)
{
	char inputtype;
	SIP_U32bit	dRetVal=0;

	fflush(stdout);
	printf("\n<<The Siptest utility allows for the configuration\n of receipt"\
		" of callbacks on expiry of diff timers.");
	printf("\nBy default you receive callbacks for "\
			"the B,C,D,F,H,I,J,K Timers>>");
	printf("\nDo you want to configure the callbacks[y/n]?[n]");
	fflush(stdin);
	fflush(stdout);
	inputtype = sip_getchar();
	fflush(stdin);
	fflush(stdout);
	if(inputtype == 'y' || inputtype =='Y')
	{
		printf\
			("\n For the following interactive session the default option"\
				" is [n]");
		printf\
			("\n============================================================");
		printf\
			("\n Do you want to receive a callback when Timer B expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_B;
		}

		fflush(stdin);
		fflush(stdout);

		printf\
			("\n Do you want to receive a callback when Timer C expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_C;
		}


		fflush(stdin);
		fflush(stdout);
		printf\
			("\n Do you want to receive a callback when Timer D expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_D;
		}

		fflush(stdin);
		fflush(stdout);
		printf\
			("\n Do you want to receive a callback when Timer F expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_F;
		}

		fflush(stdin);
		fflush(stdout);
		printf\
			("\n Do you want to receive a callback when Timer H expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_H;
		}


		fflush(stdin);
		fflush(stdout);
		printf\
			("\n Do you want to receive a callback when Timer I expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_I;
		}

		fflush(stdin);
		fflush(stdout);
		printf\
			("\n Do you want to receive a callback when Timer J expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_J;
		}

		fflush(stdin);
		fflush(stdout);
		printf\
			("\n Do you want to receive a callback when Timer K expires(y/n)");
		inputtype = sip_getchar();
		if(inputtype == 'y' || inputtype =='Y')
		{
			dRetVal |= SIP_ISSUE_CBKFOR_TIMER_K;
		}

		printf("\nTimer Expiry Callbacks are configured");
		printf\
			("\n===========================================================\n");

			return dRetVal;
	}
	else
		return SIP_ISSUE_CBKFOR_ALLTIMERS;
}
#endif

int main(int argc, char * argv[])
{

#ifdef SIP_MIB
SIP_S8bit* pVersion;
SIP_S8bit inKey[23]; /*To get the user Key*/
SipError errorStat;
SipConfigStats *pConfig = SIP_NULL;
SIP_U32bit dT1;
char sendmethod[14];
#ifndef SIP_TXN_LAYER
SIP_U32bit dRetransCount;
#endif
#endif
	int sockfd;
#ifdef SIP_MIB
	SipStatParam statInfo;
#endif
	SIP_U32bit stat;
	int n;
	SIP_U32bit size=0;
	int clilen;
/*
*/

	struct sockaddr_in	serv_addr, cli_addr;
	fd_set readfs;
	struct timeval timeout;
	char *message;
	SIP_S8bit *nextmesg;
	SipError error;
	SipBool r;
	SIP_U32bit mintimeout;
	SipOptions opt;
	SipEventContext context;
	char inputtype;
#ifdef SIP_TXN_LAYER
	en_SipTxnDecodeResult dResult;
	SipTxnContext txncontext;
	SipTxnKey *pTxnKey = SIP_NULL;
	thread_id_t dTimerThread;
#endif

	SipTranspAddr sendaddr;



	if(argc<3)
	{
		printf("Usage:\n");
		fflush(stdout);
		printf("%s my_port dest_address [dest_port]\n",argv[0]);
		fflush(stdout);
		exit(0);
	}

#ifndef SIP_TXN_LAYER
   sip_listInit(&timerlist,free,&error);
#endif

	size=0;

	if (argc==5)
	{
		if(strcmp(argv[4],"clen")==0)
		{
			constructCLen = 1;
			showMessage = 1;
		}
		else
		{
			constructCLen = 0;
			showMessage = 0;
		}
	}
	context.pList=SIP_NULL;
	opt.dOption  = 0;
	context.pData = SIP_NULL;
	context.pDirectBuffer = SIP_NULL;

	bzero(sendaddr.dIpv4,16);
	strncpy(sendaddr.dIpv4,argv[2],15);
	sendaddr.pHost=SIP_NULL;

	if(argc >= 4)
		sendaddr.dPort = atoi(argv[3]);
	else
		sendaddr.dPort = 5060;

	/* Stack and trace initialization */
	sip_initStack();

#ifdef SIP_TXN_LAYER
	sip_initTimerWheel(0,0);

	pthread_create(&dTimerThread,NULL,sip_timerThread,NULL);

	sip_initHashTbl((CompareKeyFuncPtr)sip_txn_compareTxnKeys);
#endif

#ifdef SIP_MIB
	sip_getProtoVersion(&pVersion);
	printf("############## Protocol Version Of the SIP Stack: %s ##############\n",pVersion);
	fflush(stdout);
	free(pVersion);

	sip_listInit(&slRequestl,(sip_listFuncPtr)sip_freeString,&errorStat);

#endif

	printf("\nSip stack product id is:%s\n", (char *)(sip_getPart()) );
	printf("\nSip stack version is:%s\n", (char *)(sip_getVersion()) );
	fflush(stdout);
	printf("%s",test_tool_desc);
	fflush(stdout);

	if (sip_setErrorLevel(SIP_Major|SIP_Minor|SIP_Critical, &error)==SipFail)
		printf ("########## Error Disabled at compile time #####\n");

	if (sip_setTraceLevel(SIP_Brief,&error)== SipFail)
		printf ("########## Trace Disabled at compile time #####\n");

	sip_setTraceType(SIP_All,&error);
	clilen = sizeof(cli_addr);

#ifdef SIP_TXN_LAYER
	printf("\n########## The Stack has been compiled with SIP_TXN_LAYER ######\n ");
	fflush(stdout);
	printf("\n########## Siptest supports the transaction layer as defined in RFC 3261  ( June 2002 ) ######\n ");
	fflush(stdout);
#endif

#ifdef TEST_SIP_CONVERT_TO_QUOTED_STRING
    {
				
						
				{
						SipError err;
						char *p1 = (char *)malloc(1000);
						char *p2 = SIP_NULL;

						memset(p1,'\0',100);
						fflush(stdin) ;
						printf("\n--->String to be Converted \n") ;
/*
			while (1)
						printf("\nEnter the String to be Converted : ") ;
						scanf("%s",p1) ;
						printf("\nString = <%s>\n",p1);
						strcpy(p1,"‡·‚„‰ÊÁËÈÍÎÏÌÓÔ") ;
						strcpy(p1,"¿¡¬ƒƒ≈∆«… ÀÃÕœ–—“”‘’÷◊Ÿ⁄‹ﬁﬂ") ;
						strcpy(p1,"ÒÚÛÙıˆ˜¯˚¸˝") ;
*/

						p2 = (char *)sip_convertIntoQuotedString((SIP_U8bit *)p1,&err);

						if(p2 != SIP_NULL)
						{
								printf("\nResultant string = <%s>", p2);
						}
						else
								printf("p2 is NULL \n") ;
						free (p1);
						free (p2);
				}
		}
#endif


/* For Selecting the transport type  TCP/UDP */
	printf("\n<<The Siptest utility can be configured to run using either TCP/UDP protocol>>\n\n");
	fflush(stdout);
	fflush(stdin);
	printf("1 :\tUDP\n");
	printf("2 :\tTCP\n");
	fflush(stdout);
	printf("Select the type of transport mode(Default:1):");
	fflush(stdout);
	fflush(stdin);
	inputtype = sip_getchar();
	if(inputtype == '2')
	{
		glbtransptype = SIP_TCP;
		printf("TCP Selected.\n");
		fflush(stdout);
	}
	else
	{
		glbtransptype = SIP_UDP;
		printf("UDP Selected.\n");
		fflush(stdout);
	}

#ifdef SIP_TXN_LAYER
/* For Selecting the transsaction type (UA/Proxy)*/
	printf("\n<<The Siptest utility can be configured to run in either the UA mode or the Proxy mode>>\n\n");
	fflush(stdout);
	fflush(stdin);
	printf("1 :\tUA\n");
	printf("2 :\tProxy\n");
	fflush(stdout);
	printf("Select the type of Transaction(Default:1):");
	fflush(stdout);
	fflush(stdin);
	fflush(stdout);
	inputtype = sip_getchar();
	if(inputtype != '2')
	{
			glbtransactiontype = 0;
			printf("UA Selected.\n");
			fflush(stdout);
	}
	else
	{
			glbtransactiontype =1;
			printf("Proxy Selected.\n");
			fflush(stdout);
	}

	/* Filling up the event context*/
	txncontext.pEventContext = &context;
	txncontext.dTxnType = (en_SipTxnType)glbtransactiontype;
	txncontext.txnOption.dOption = opt;
	txncontext.txnOption.dTimerOption= 0;

	/*Query for which timeOut cbks the applicn wants to receive*/
	glbTimeTimeoutOption=getTimerTimeoutOptions();
	txncontext.txnOption.dTimeoutCbkOption=glbTimeTimeoutOption;

/* For Selecting the timer values*/
	fflush(stdout);
	printf("\n<<The Siptest utility allows for the configuration "\
	" of the various timers \n as defined in");
	printf(" bis05 for the transaction layer>>");
	fflush(stdout);
	printf("\nDo you want to configure the Timer Values(y/n):");
	fflush(stdin);
	fflush(stdout);
	inputtype = sip_getchar();
	if(inputtype == 'y' || inputtype =='Y')
	{
		int dTimerValue;
		glbsetTimerValue = 1;
		dTimeOut.dT1		= SIP_DEFAULT_T1;
		dTimeOut.dT2		= SIP_DEFAULT_T2;
		dTimeOut.dTimerB	= SIP_DEFAULT_B;
		dTimeOut.dTimerC	= SIP_DEFAULT_C;
		dTimeOut.dTimerD_T3	= SIP_DEFAULT_D_T3;
		dTimeOut.dTimerF_T3	= SIP_DEFAULT_F_T3;
		dTimeOut.dTimerH	= SIP_DEFAULT_H;
		dTimeOut.dTimerI_T4	= SIP_DEFAULT_I_T4;
		dTimeOut.dTimerJ_T3	= SIP_DEFAULT_J_T3;
		dTimeOut.dTimerK_T4	= SIP_DEFAULT_K_T4;

		fflush(stdin);
		printf("\nDefault Value of Timer T1:%d\n",SIP_DEFAULT_T1);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
			printf("Enter the customized value for T1:");
			fflush(stdout);
			dTimerValue = 0;
			scanf("%d",&dTimerValue);
			if(dTimerValue)
				dTimeOut.dT1 = dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer T2:%d\n",SIP_DEFAULT_T2);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		fflush(stdin);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for T2:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dT2		= dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer B:%d\n",SIP_DEFAULT_B);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for B:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerB		= dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer C:%d\n",SIP_DEFAULT_C);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for C:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerC = dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer D:%d\n",SIP_DEFAULT_D_T3);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for D:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerD_T3		= dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer F:%d\n",SIP_DEFAULT_F_T3);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for F:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerF_T3	= dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer H:%d\n",SIP_DEFAULT_H);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for H:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerH	= dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer I:%d\n",SIP_DEFAULT_I_T4);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for I:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerI_T4	= dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer J:%d\n",SIP_DEFAULT_J_T3);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for J:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerJ_T3	= dTimerValue;
		}
		fflush(stdin);

		printf("\nDefault Value of Timer K:%d\n",SIP_DEFAULT_K_T4);
		fflush(stdout);
		printf("Do You want to change the timer value(y/n):");
		fflush(stdout);
		inputtype = sip_getchar();
		if(inputtype =='y' || inputtype =='Y')
		{
		printf("Enter the customized value for K:");
		fflush(stdout);
		fflush(stdin);
		dTimerValue = 0;
		scanf("%d",&dTimerValue);
		if(dTimerValue)
			dTimeOut.dTimerK_T4	= dTimerValue;
		}
		fflush(stdin);
	}
#endif

	printf("%s", menu);
	fflush(stdout);

	if((glbtransptype&SIP_UDP) == SIP_UDP)
	{
		if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
		printf("Server could not open dgram socket\n");
		fflush(stdout);
		close(sockfd);
		exit(0);
		}
	}
	else
	{
		if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Server could not open stream socket\n");
			fflush(stdout);
			close(sockfd);
			exit(0);
		}
	}

	/* initialize structures for binding to listen port */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family	  = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port	  = htons((SIP_U16bit)atoi(argv[1]));

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Server couldn't bind to local address.\n");
		fflush(stdout);
		close(sockfd);
		exit(0);
	}

	if((glbtransptype&SIP_TCP) == SIP_TCP)
		listen(sockfd,5);

	for(;;)
	{
#ifndef SIP_TXN_LAYER
		TimerListElement *elem;
		SIP_U32bit i;
#endif
		SipError derror;


		struct timeval tv1, tv2;


#ifndef SIP_TXN_LAYER
		/* get minimum timeout from the timer list */
		mintimeout = getMinimumTimeout(&timerlist);
#else
		mintimeout = 1000;
#endif


		timeout.tv_sec = (int)(mintimeout/1000);
		timeout.tv_usec = ((int)mintimeout)%1000;
		FD_ZERO(&readfs);
		FD_SET(sockfd,&readfs);
		FD_SET(0,&readfs);
		/* Enter into select
		   Comes out of select if a key is pressed
		   or a network message arrives
		   or if the minimum timout occurs
		*/

		gettimeofday(&tv1, NULL);
		if (select(sockfd+1, &readfs, NULL, NULL, &timeout) < 0)
		{
			printf("Select system call failed\n\n");
			fflush(stdin);
			fflush(stdout);
			continue;
		}
		gettimeofday(&tv2, NULL);
		mintimeout = (((tv2.tv_sec )*1000+(tv2.tv_usec/1000))\
			-((tv1.tv_sec )*1000+(tv1.tv_usec/1000)));


#ifndef SIP_TXN_LAYER
			   /* update timer lists  and call registered funcs if required */

			sip_listSizeOf(&timerlist,&size,&derror);
			for(i=0; i<size; i++)
			{
				sip_listGetAt(&timerlist, (SIP_U32bit) i, (SIP_Pvoid *) &elem,\
					&derror);
				elem->duration -= mintimeout;
				if(elem->duration<=0)
				{
					SipBool (*tempfunc)(SipTimerKey *, SIP_Pvoid);
					SipTimerKey *tempkey;
					SIP_Pvoid tempbuffer;

					tempfunc = elem->timeoutfunc;
					tempkey = elem->key;
					tempbuffer = elem->buffer;
					sip_listDeleteAt(&timerlist, (SIP_U32bit) i , &derror);
					i--;size--;
					tempfunc(tempkey, tempbuffer);
				}
			}
#endif


		if(FD_ISSET(sockfd,&readfs))
		{
			if((glbtransptype&SIP_UDP) == SIP_UDP)
			{
				message = (char *) malloc(sizeof(char)*MAXMESG);
				n = recvfrom(sockfd, message, MAXMESG, 0,\
					(struct sockaddr *) &cli_addr, &clilen);
			}
			else
			{
				int dacceptfd;
				dacceptfd = accept(sockfd,NULL,NULL);
				if(dacceptfd <0)
				{
					printf("Failed: Accept on a TCP socket failed");
					fflush(stdout);
					close(sockfd);
					exit(0);
				}
				message = (char *) malloc(sizeof(char)*MAXMESG);
				n = recv(dacceptfd,message,MAXMESG,0);
			}


			if (showMessage) printf ("\n\n\n\n");
				fflush(stdout);

			if (n < 0)
			{
				printf("Server : Error in receive.\n");
				fflush(stdout);
				close(sockfd);
				exit(0);
			}
			message[n]='\0';

			do
			{
#ifdef SIP_NO_CALLBACK
				SipMessage *outMsg = SIP_NULL;
#endif
				printf("|---Message Received from the network----------|\n");
				fflush(stdout);
				printf("\n%s\n",message);
				fflush(stdout);
				printf("|---End of message received from the network---|\n");
				fflush(stdout);

				opt.dOption |= SIP_OPT_BADMESSAGE ;
 		
#ifndef SIP_NO_CALLBACK
				r= sip_decodeMessage((SIP_S8bit *)message,&opt,strlen(message),&nextmesg,&context, &derror);
#else
#ifdef SIP_TXN_LAYER
				txncontext.txnOption.dOption = opt;
				r= sip_txn_decodeMessage((SIP_S8bit *)message,&outMsg,\
					&txncontext,n,&nextmesg,&dResult,&pTxnKey,&derror);
#else
				r= sip_decodeMessage((SIP_S8bit *)message,&outMsg,&opt, strlen(message),&nextmesg,&context, &derror);
				{
#ifdef SIP_TEST
#ifdef SIP_BY_REFERENCE
#ifdef SIP_SOLARIS
					SIP_U32bit count = 0 ;
					SipError err = E_NO_ERROR ;
					SIP_S8bit *pIncorrect=(SIP_S8bit *)malloc(50) ;
					SdpMessage *pSdp = SIP_NULL ;
					SipMsgBody *pMsgBody = SIP_NULL ;
					SdpMedia *pSdpMedia = SIP_NULL ;

					sip_getIncorrectHeadersCount(outMsg,&count,&err) ;
					printf("\nINCORRECT Header COUNT = [%d] \n",count) ;
					sip_getEntityErrorCount(outMsg,&count,&err) ;
					printf("\nINCORRECT Entity COUNT = [%d] \n",count) ;
					if ( sip_getMsgBodyAtIndex(outMsg,&pMsgBody,0,&err) == SipSuccess)
					{
							printf("\nExtracted Message Body from SIP-MESSAGE \n") ;
							if ( sip_getSdpFromMsgBody(pMsgBody,&pSdp,&err) == SipSuccess)
							{
									printf("Extracted Sdp Message Body  \n") ;
									if (sdp_getIncorrectLineAtIndex(pSdp,&pIncorrect,0,&err) == SipSuccess )
									{
											printf("\nIncorrect Sdp Line : [%s] \n",pIncorrect) ;
									}
									else
									{
											printf("Error in sdp_getIncorrectLineAtIndex : %d \n",err) ;
									}
									sdp_getMediaAtIndex(pSdp,&pSdpMedia,0,&err) ;
									if (sdp_getIncorrectLineAtIndexFromMedia(pSdpMedia,&pIncorrect,0,&err) == SipSuccess )
									{
											printf("\nIncorrect Sdp Line : [%s] \n",pIncorrect) ;
									}
									else
									{
											printf("Error in sdp_getIncorrectLineAtIndex : %d \n",err) ;
									}
							}
					}
					else
					{
							printf("\nError while trying to access Message Body : %d\n",err) ;
					}
					free(pIncorrect);
#endif
#endif
#endif/* #ifdef SIP_TEST */
				}
#endif
#endif
				if (r==SipFail)
				{
					switch(derror)
					{
						case E_PARSER_ERROR:
							printf("** ERROR : Parsing of the Message Failed**\n");
							fflush(stdout);
							break;
						case E_INCOMPLETE:
							printf("** ERROR : Message Incomplete**\n");
							fflush(stdout);
							break;
#ifdef SIP_TXN_LAYER
						case E_TXN_NO_EXIST:
							printf("** ERROR : The Txn for this message doesn't exist**\n");
							fflush(stdout);
							break;
						case E_TXN_EXISTS:
							printf("** ERROR : The Txn for this message already exists**\n");
							fflush(stdout);
							break;
						case E_TXN_INV_STATE:
							printf("** ERROR : This message leads to an invalid transaction state**\n");
							fflush(stdout);
							break;
						case E_TXN_INV_MSG:
							printf("** ERROR : This is an invalid message received for the Txn**\n");
							fflush(stdout);
							break;
#endif
						default:
							break;
					}
					printf("** ERROR : Decode Message Failed**\n");
					fflush(stdout);

#ifdef SIP_NO_CALLBACK
					if(outMsg != SIP_NULL)
					{
						sip_freeSipMessage(outMsg);
					}
#endif

				}
#ifdef SIP_NO_CALLBACK
				else
				{
#ifdef SIP_TXN_LAYER
					switch(dResult)
					{
					case SipTxnIgnorable:
					printf(" The message can be ignored by the application.\n");
					fflush(stdout);
					break;
					case SipTxnNonIgnorable:
					printf("The message needs to be handled by the application.\n");
					fflush(stdout);
					break;

					case SipTxnStrayPrack:
					printf("A stray prack has been received. Please respond\n");
					fflush(stdout);
					break;
					
					case SipTxnStrayMessage:
					printf("The message is a Stray message, It can be ignored.\n");
					fflush(stdout);
					break;
					case SipTxnQueued:
					printf("The message is a retransmission.It is queued.\n");
					fflush(stdout);
					break;
					case SipTxnConfirmnNeeded:
					printf("The message needs to be handled by the application.\n");
					fflush(stdout);
					break;
					case SipTxnRetrans:
					printf("The message decoded is a retransmission and has" \
							"been received after cancellation of the Txn.\n");
					default:
					printf("The message recieved is not of recognized type.\n");
					fflush(stdout);
					break;
					}
					sip_txn_freeTxnKey(pTxnKey,&derror);
#endif


					/* In no callback mode, the user would have to figure
					 * out which is the decoded method.
					 * See sipstruct.h for more details
					*/
#ifdef SIP_SERIALIZE
				{
						SipError err=E_NO_ERROR ;
						SipMessage *pSipMsg = SIP_NULL ;

						SIP_S8bit *pBuffer = (SIP_S8bit*)malloc(strlen(message)*5) ;

						if ( serialize_sip_message(outMsg,&pBuffer,0,&err) == SipFail )
						{
								printf("\nError while serialization \n") ;
						}
						else
						{
								printf("Serialization done Successfully \n") ;
								if (deserialize_sip_message(&pSipMsg,pBuffer,0,&err) == SipFail)
								{
										printf("\nError while DeSerialization \n") ;
								}
								else
								{
										SipOptions opts  ;
										SIP_U32bit len= 0;
										SIP_S8bit *pTextMsg = (SIP_S8bit*)malloc(6500) ;
										
										opts.dOption=SIP_OPT_FULLFORM ;
										printf("DeSerialization done Successfully \n") ;
										if ( sip_formMessage(pSipMsg, &opts, pTextMsg, &len, &err) == SipFail)
										{
												printf("\n sip_formMessage Error = [%d] \n",err) ;
										}
										else
										{
												printf("After sip_formMessage:\n, Message = [%s] \n",pTextMsg) ;
										}
										sip_freeSipMessage(pSipMsg) ;
										free(pTextMsg) ;
								} /* else */
						}
						free(pBuffer) ;
				}
#endif
#ifdef SIP_TEST
#ifdef SIP_PINT
				{
						printf("-----------------------------------------\n") ;
						printf("Sdp Pint Processing : \n");
#ifdef SIP_BY_REFERENCE
						processSdpPintBodyInByReference(outMsg) ;
#else
						processSdpPintBodyInNonByReference(outMsg) ;
#endif
						
						printf("-----------------------------------------\n") ;
				}
#endif/* #ifdef SIP_PINT */
#endif

					printf ("** Received message without issuing a callback !\n");
					fflush(stdout);
					displayParsedMessage (outMsg);

				}
#endif

				fast_memfree(0,message, SIP_NULL);
				/* See if read buffer had more than one message in it */
				message = nextmesg;
				/* Length of the remaining segment is in event-context */
				n = context.dNextMessageLength;

	if (showMessage)
	{
		if ( sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_COUNT, SIP_STAT_NO_RESET, &stat, &derror)==SipFail)
				{
					printf ("##### Cannot display Statistics - Stats disabled #####\n");
				}
				else
				{
				printf("######## Stack statistics ############\n");
				fflush(stdout);
				sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_REQ_PARSED,\
					SIP_STAT_NO_RESET, &stat, &derror);
				printf("######## Requests parsed	:%d\n",stat);
				fflush(stdout);
				sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_RESP_PARSED,\
					SIP_STAT_NO_RESET, &stat, &derror);
				printf("######## Responses parsed	:%d\n",stat);
				fflush(stdout);
				sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_REQ_SENT, \
					SIP_STAT_NO_RESET, &stat, &derror);
				printf("######## Requests sent		:%d\n",stat);
				fflush(stdout);
				sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_RESP_SENT, \
					SIP_STAT_NO_RESET, &stat, &derror);
				printf("######## Responses sent 	:%d\n",stat);
				fflush(stdout);
				sip_getStatistics(SIP_STAT_TYPE_ERROR, SIP_STAT_ERROR_PROTOCOL,\
					SIP_STAT_NO_RESET, &stat, &derror);
				printf("######## Failed Message 	:%d\n",stat);
				fflush(stdout);

				sip_getStatistics(SIP_STAT_TYPE_API, SIP_STAT_SECOND_API_REQ_PARSED,\
					SIP_STAT_NO_RESET, &stat, &derror);
				printf("######## Second Level Requests parsed	:%d\n",stat);
				fflush(stdout);
				sip_getStatistics(SIP_STAT_TYPE_ERROR, SIP_STAT_SECOND_ERROR_PROTOCOL,\
					SIP_STAT_NO_RESET, &stat, &derror);
				printf("######## Second Level Failed Message	:%d\n",stat);
				fflush(stdout);

#ifdef SIP_MIB
		if ( 1 == requestFlag )
		{
			SIP_S8bit* pType = SIP_NULL;
			printf("\n######## SIP MIB Statistics ########\n");
			fflush(stdout);
			pType = strdup("INVITE");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## INVITE Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## INVITE Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## INVITE retries 	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("ACK");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## ACK Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## ACK Sent			:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## ACK retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("BYE");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## BYE Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## BYE Sent			:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## BYE retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("CANCEL");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## CANCEL Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## CANCEL Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## CANCEL retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("OPTIONS");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## OPTIONS Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## OPTIONS Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## OPTIONS retries	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("REGISTER");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## REGISTER Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## REGISTER Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## REGISTER retries	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("INFO");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## INFO Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## INFO Sent			:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## INFO retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("PRACK");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## PRACK Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## PRACK Sent			:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## PRACK retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("PROPOSE");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## PROPOSE Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## PROPOSE Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## PROPOSE retries	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("REFER");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## REFER Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## REFER Sent			:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## REFER retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("SUBSCRIBE");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## SUBSCRIBE Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## SUBSCRIBE Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## SUBSCRIBE retries	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("NOTIFY");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## NOTIFY Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## NOTIFY Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## NOTIFY retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("MESSAGE");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## MESSAGE Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## MESSAGE Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## MESSAGE retries	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("COMET");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## COMET Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## COMET Sent			:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## COMET retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("UPDATE");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## UPDATE Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## UPDATE Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## UPDATE retries 	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("PUBLISH");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## PUBLISH Received	:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## PUBLISH Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## PUBLISH retries 	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("UNKNOWN");
			sip_mib_getStatistics( SIP_STAT_API_REQUEST,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## UNKNOWN method Received:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## UNKNOWN method Sent	:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## UNKNOWN method retries	:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);
			fflush(stdout);

		/* get the extension methods from the hash */
			if ( sip_listSizeOf(&slRequestl,&size,&errorStat) == SipSuccess )
			{
				SIP_S8bit* pKey = SIP_NULL;
				while ( size > 0 )
               	{
					if ( sip_listGetAt(&slRequestl,size-1,(SIP_Pvoid*)&pKey,\
						&errorStat) != SipSuccess )
					{
						size--;
						continue;
					}
					pType = strdup(pKey);
					sip_mib_getStatistics(SIP_STAT_API_REQUEST,pType,\
						SIP_STAT_NO_RESET, &statInfo, &derror);
					printf("######## %s method Received	:%d\n",pType,statInfo.recv);
					fflush(stdout);
					printf("######## %s method Sent		:%d\n",pType,statInfo.sendInitial);
					fflush(stdout);
					printf("######## %s method retries	:%d\n",pType,statInfo.sendRetry);
					fflush(stdout);
					free(pType);
               	    size--;
               	}
			}
			fflush(stdout);
		}
		if ( 1 == responseClassFlag )
		{
			SIP_S8bit* pType = SIP_NULL;
			pType = strdup("1XX");
			sip_mib_getStatistics( SIP_STAT_API_RESPONSECLASS,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## 1XX Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## 1XX Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## 1XX retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("2XX");
			sip_mib_getStatistics( SIP_STAT_API_RESPONSECLASS,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## 2XX Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## 2XX Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## 2XX retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("3XX");
			sip_mib_getStatistics( SIP_STAT_API_RESPONSECLASS,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## 3XX Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## 3XX Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## 3XX retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("4XX");
			sip_mib_getStatistics( SIP_STAT_API_RESPONSECLASS,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## 4XX Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## 4XX Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## 4XX retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("5XX");
			sip_mib_getStatistics( SIP_STAT_API_RESPONSECLASS,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## 5XX Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## 5XX Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## 5XX retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

			pType = strdup("6XX");
			sip_mib_getStatistics( SIP_STAT_API_RESPONSECLASS,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## 6XX Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## 6XX Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## 6XX retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);

		}
		if ( 1 == responseCodeFlag )
		{
			SIP_S8bit* pType = SIP_NULL;
			pType = strdup("200");
			sip_mib_getStatistics( SIP_STAT_API_RESPONSECODE,pType,\
				SIP_STAT_NO_RESET, &statInfo, &derror);
			printf("######## 200 Received		:%d\n",statInfo.recv);
			fflush(stdout);
			printf("######## 200 Sent		:%d\n",statInfo.sendInitial);
			fflush(stdout);
			printf("######## 200 retries		:%d\n",statInfo.sendRetry);
			fflush(stdout);
			free(pType);
		}
#endif
					printf("######################################\n");
					fflush(stdout);
					}
					printf("%s", menu);
					fflush(stdout);
				}

			} while (message != SIP_NULL);
		}
		else if (FD_ISSET(0,&readfs))

		{
			/* Key pressed */
			char c;
			fflush(stdin);
			c=sip_getchar();
			switch(c)
			{
				case 'i':
					sendRequest("INVITE",&sendaddr,&derror);
					break;
				case 'r':
					sendRequest("REGISTER", &sendaddr, &derror);
					break;
				case 'o':
				    sendRequest("OPTIONS", &sendaddr, &derror);
					break;
				case 'b':
				    sendRequest("BYE", &sendaddr, &derror);
					break;
				case 'c':
					sendRequest("CANCEL", &sendaddr, &derror);
					break;
				case 'a':
					sendRequest("ACK",&sendaddr,&derror);
					break;
				case 'k':
					sendRequest("PRACK",&sendaddr,&derror);
					break;
				case 'p':
					sendRequest("PROPOSE",&sendaddr,&derror);
					break;
#ifdef SIP_IMPP
				case 'u':
					sendRequest("SUBSCRIBE",&sendaddr,&derror);
					break;
				case 'n':
					sendRequest("NOTIFY",&sendaddr,&derror);
					break;
				case 'm':
					sendRequest("MESSAGE",&sendaddr,&derror);
					break;
#endif
				case 'v':
					sendRequest("UNKNOWN",&sendaddr,&derror);
					break;
				case 'e':
					sendRequest("REFER",&sendaddr,&derror);
					break;
				case 'f':
					sendRequest("INFO",&sendaddr,&derror);
					break;
				case '3':
					sendResponse(300,"Multiple Choices","INVITE", &sendaddr,&derror);
					break;
				case '4':
					sendResponse(401,"UnAuthorized","INVITE", &sendaddr,&derror);
					break;
				case 'I':
					sendResponse(200,"OK","INVITE", &sendaddr,&derror);
					break;
				case 'C':
					sendResponse(200,"OK","CANCEL", &sendaddr,&derror);
					break;
				case 'R':
					sendResponse(200,"OK","REGISTER", &sendaddr,&derror);
					break;
				case 'B':
					sendResponse(200,"OK","BYE", &sendaddr,&derror);
					break;
				case 'O':
					sendResponse(200,"OK","OPTIONS",&sendaddr,&derror);
					break;
				case 'K':
					sendResponse(200,"OK","PRACK",&sendaddr,&derror);
					break;
				case 'F':
					sendResponse(200,"OK","INFO",&sendaddr,&derror);
					break;
				case 'P':
					sendResponse(200,"OK","PROPOSE",&sendaddr,&derror);
					break;
				case 'E':
					sendResponse(200,"OK","REFER",&sendaddr,&derror);
					break;
#ifdef SIP_IMPP
				case 'U':
					sendResponse(200,"OK","SUBSCRIBE",&sendaddr,&derror);
					break;
				case 'N':
					sendResponse(200,"OK","NOTIFY",&sendaddr,&derror);
					break;
				case 'M':
					sendResponse(200,"OK","MESSAGE",&sendaddr,&derror);
					break;
#endif

#ifdef SIP_MIB
				case 'S':

					c=sip_getchar();
					switch(c)
					{

						case 'r':
							if ( 1 == requestFlag )
							{
								requestFlag = 0;
								printf("Statistics for Methods Disabled.\n");
								fflush(stdout);
							}
							else
							{
								requestFlag = 1;
								printf("Statistics for Methods Enabled.\n");
								fflush(stdout);
							}
							break;
						case 's':
							if ( 1 == responseClassFlag )
							{
								responseClassFlag = 0;
								printf\
									("Statistics for Resp Class Disabled.\n");
								fflush(stdout);
							}
							else
							{
								responseClassFlag = 1;
								printf("Statistics for Resp Class Enabled.\n");
								fflush(stdout);
							}
							break;
						case 'd':
							if ( 1 == responseCodeFlag )
							{
								responseCodeFlag = 0;
								printf\
									("Stats for Response Code Disabled.\n");
								fflush(stdout);
							}
							else
							{
								responseCodeFlag = 1;
								printf("Stats for Response Code Enabled.\n");
								fflush(stdout);
							}
							break;
						case 'c':
							printf("------------------------\n");
							fflush(stdout);
							printf("M:Methods \n");
							fflush(stdout);
							printf("R:Response\n");
							fflush(stdout);
							printf("S:Send a Request message\n");
							fflush(stdout);
							printf("---------------------------\n");
							fflush(stdout);
							printf("Select the Statistics to be configured:");
							fflush(stdout);
							fflush(stdin);
							c=sip_getchar();
							fflush(stdin);
							switch(c)
							{
								case 'M':
									printf\
										("Give the Method To be Configured:\n");
									fflush(stdout);
									scanf("%s",inKey);
									fflush(stdin);
									printf\
										("Do you want to Configure Timer"\
										" values(y/n): ");
									fflush(stdout);
									c = sip_getchar();
									fflush(stdin);
									pConfig = SIP_NULL;
									if(c == 'y' || c == 'Y')
									{
										pConfig = (SipConfigStats *)\
											malloc(sizeof(SipConfigStats));
										printf("Enter the T1 value:");
										fflush(stdout);
										dT1 =0;
										scanf("%d",&dT1);
										fflush(stdin);
										pConfig->dRetransT1 = dT1;
#ifndef SIP_TXN_LAYER
										printf("Enter the RetransCount:");
										fflush(stdout);
										dRetransCount =0;
										scanf("%d",&dRetransCount);
										fflush(stdin);
										pConfig->dMaxRetransCount= \
											dRetransCount;
#endif
									}
									sip_listAppend(&slRequestl,\
										(SIP_Pvoid)strdup((SIP_S8bit*)inKey),\
										&errorStat);
									if ( sip_mib_configureStatistics\
										(SIP_STAT_API_REQUEST,inKey,pConfig,\
										0,&error) == SipFail )
									{
										printf\
										("Error: in configuring statistics\n");
										fflush(stdout);
										break;
									}
									printf("The Statistics are configured\n");
									fflush(stdout);
									break;
								case 'R':
									printf("\nDo you want to "\
											"enable response-code"\
											" callbacks?");
									c = sip_getchar();
									fflush(stdin);
									if (c == 'y' || c == 'Y')
									{
										SIP_S32bit	dCode;
										SIP_S8bit	sCode[7];
										PrintFunction("\nWhich resp"\
											" code do you want to"\
											" configure callbks for?");
										scanf("%d",&dCode);

										if ((dCode>0)&&
											(dCode<NUM_STAT_RESPONSECODE))
										{
											SipBool dConfResult;
											HSS_SNPRINTF\
												((char *)sCode, 7, "%d",
												dCode);
											dConfResult=\
												sip_mib_configureStatistics\
												(SIP_STAT_API_RESPONSECODE,\
												sCode,SIP_NULL,0,&error);
											if (dConfResult==SipFail)
											{
												printf("Error:in configuring"\
													" statistics\n");
												fflush(stdout);
												break;
											}
											printf("\nCallback configured");
										}
										else
											printf("\nInvalid Value Given");
									}

									fflush(stdin);
									printf\
										("\nDo you want to Configure Timer"\
											" values(y/n): ");
									fflush(stdout);
									c = sip_getchar();
									fflush(stdin);
									pConfig = SIP_NULL;
									if(c == 'y' || c == 'Y')
									{
										pConfig = (SipConfigStats *)malloc\
											(sizeof(SipConfigStats));
										printf("Enter the T1 value:");
										fflush(stdout);
										scanf("%d",&dT1);
										fflush(stdin);
										pConfig->dRetransT1 = dT1;
#ifndef SIP_TXN_LAYER
										printf("Enter the RetransCount:");
										fflush(stdout);
										scanf("%d",&dRetransCount);
										fflush(stdin);
										pConfig->dMaxRetransCount= dRetransCount;
#endif
										if ( sip_mib_configureStatistics\
												(SIP_STAT_API_RESPONSE,SIP_NULL,\
												pConfig,0,&error) == SipFail )
										{
											printf\
												("Error:configuring statistics\n");
											fflush(stdout);
											break;
										}
										printf("The statitics are configured\n");
										fflush(stdout);
									}
									break;
								case 'S':
									fflush(stdin);
									printf\
										(" Enter the req method to be send:");
									fflush(stdout);
									scanf("%s",sendmethod);
									fflush(stdin);
									sendRequest(sendmethod,&sendaddr,&derror);
									printf\
										("The SIP message with method %s"\
											" is sent",sendmethod);
									fflush(stdout);
									break;

							}
							break;
					}
					fflush(stdout);
					break;
#endif

				case 'T':
					c = sip_getchar();
					switch(c)
					{
						case 'c':
						sendResponse(100,"Trying","CANCEL", &sendaddr,&derror);
						break;
						case 'b':
						sendResponse(100,"Trying","BYE", &sendaddr,&derror);
						break;
						case 'o':
						sendResponse(100,"Trying","OPTIONS", &sendaddr,&derror);
						break;
						case 'r':
						sendResponse(100,"Trying","REGISTER", &sendaddr,&derror);
						break;
						case 'e':
						sendResponse(100,"Trying","REFER", &sendaddr,&derror);
						break;
#ifdef SIP_IMPP
						case 'u':
						sendResponse\
							(100,"Trying","SUBSCRIBE", &sendaddr,&derror);
						break;
						case 'n':
						sendResponse(100,"Trying","NOTIFY", &sendaddr,&derror);
						break;
						case 'm':
						sendResponse(100,"Trying","MESSAGE", &sendaddr,&derror);
						break;
						default:
						if (c!=0x0a)
						printf("Unrecognized Key Pressed\n");
						fflush(stdout);
						fflush(stdin);
						break;
#endif
					}
					break;
				case 't':
					sendResponse(100,"Trying","INVITE", &sendaddr,&derror);
					break;
				 case '0':
				    sendResponse(180,"Ringing","INVITE",&sendaddr,&derror);
				    break;
				 case '1':
				    sendResponse(181,"Call Forward","INVITE",&sendaddr,&derror);
				    break;
				 case '2':
				    sendResponse(182,"Queued","INVITE",&sendaddr,&derror);
				    break;
				 case 's':
				     sendResponse\
					 	(183,"Session progress","INVITE",&sendaddr,&derror);
				     break;
				 case 'z':
					{
						char customSelect;
						int j;
						FD_CLR(0,&readfs);
						glbSipOption=1;
						if(context.pList == SIP_NULL)
							context.pList=(SipHdrTypeList *)\
								fast_memget(0,sizeof(SipHdrTypeList),\
								SIP_NULL);
						for(j=0;j<HEADERTYPENUM;j++)
							context.pList->enable[j]=SipSuccess;
						opt.dOption=0;

						printf("\nWith message body parsing disabled, any"\
							" text in the entity-body section of the message"\
							"is admitted by the parser provided the content-"\
							"type header is present. The section is stored as"\
							" an unknown body in the message. When enabled, "\
							"the message body will be parsed depending on the "\
							"content-type specified in the message.\n");
						fflush(stdout);
						printf("\nEnable message body parsing (y/n): [y]\n");
						fflush(stdout);
						fflush(stdin);
						customSelect=sip_getchar();
						if((customSelect=='n')|| (customSelect=='N'))
						{
							opt.dOption |= SIP_OPT_NOPARSEBODY;
#ifdef SIP_TXN_LAYER
							txncontext.txnOption.dOption = opt;
#endif
						}
						else
						{
							printf("\nThis option will enable/disable SDP"
								" parsing."\
								" Other types (multipart-MIME, ISUP, applica"\
								"tion/sip will be parsed.\n");
							fflush(stdout);
							printf("\nEnable SDP parsing (y/n): [y] \n");
							fflush(stdout);
							fflush(stdin);
							customSelect=sip_getchar();
							if((customSelect=='n')||(customSelect=='N'))
							{
								opt.dOption |= SIP_OPT_NOPARSESDP;
#ifdef SIP_TXN_LAYER
								txncontext.txnOption.dOption = opt;
#endif
							}
						}
						printf("With this option enabled, the stack will parse"\
							" messages with errors in SIP headers or in"\
							" the entity body. Messages with errors in the "\
							"request-line or the status line are not parsed."\
							" The decoded message structure "\
							"will contain a count of the number"\
							" of errors encountered. Bad headers are also "\
							"stored in the decoded message structure.\n");
						fflush(stdout);
						printf("\nEnable bad message parsing (y/n): [n]\n");
						fflush(stdout);
						fflush(stdin);
						customSelect=sip_getchar();
						if((customSelect=='y')|| (customSelect=='Y'))
						{
							opt.dOption |= SIP_OPT_BADMESSAGE;
#ifdef SIP_TXN_LAYER
							txncontext.txnOption.dOption = opt;
#endif
						}
						printf("This option enables selective parsing. This "
							"demonstration shows the feature for four header "\
							"types. When parsing for a header type is disabled"\
							", any error in the header will be ignored by the"\
							" parser.\n");
						fflush(stdout);
						printf("\nEnable selective header parsing (y/n): [n]");
						fflush(stdout);
						fflush(stdin);
						customSelect=sip_getchar();
						if((customSelect=='y')|| (customSelect=='Y'))
						{
							char selectHeader;
							opt.dOption |= SIP_OPT_SELECTIVE;
#ifdef SIP_TXN_LAYER
							txncontext.txnOption.dOption = opt;
#endif
							printf("\nDisable Via header parsing(y/n): [n] ");
							fflush(stdout);
							fflush(stdin);

							selectHeader=sip_getchar();
							if((selectHeader=='y') || (selectHeader=='Y'))
								context.pList->enable[SipHdrTypeVia]=SipFail;

							printf\
								("\nDisable Contact header parsing(y/n): [n] ");
							fflush(stdout);
							fflush(stdin);
							selectHeader=sip_getchar();
							if((selectHeader=='y') || (selectHeader=='Y'))
								context.pList->enable[SipHdrTypeContactAny]=\
								SipFail;

							printf("\nDisable Record-Route parsing(y/n): [n] ");
							fflush(stdout);
							fflush(stdin);
							selectHeader=sip_getchar();
							if((selectHeader=='y') || (selectHeader=='Y'))
								context.pList->enable[SipHdrTypeRecordRoute]=\
								SipFail;

							printf("\nDisable Route header parsing(y/n): [n] ");
							fflush(stdout);
							fflush(stdin);
							selectHeader=sip_getchar();
							if((selectHeader=='y') || (selectHeader=='Y'))
								context.pList->enable[SipHdrTypeRoute]=SipFail;
						}

#ifndef SIP_TXN_LAYER
						printf("The following option disables retransmission"\
							" related checks in the decode function. With this"\
							" option, the decode function will not call the "\
							"stop timer callback to stop retransmissions after"\
							" decoding a message. Mandaory header check is "\
							"also disabled with this option.\n");
						fflush(stdout);
						printf("Disable timer checks in decode (y/n): [n]");
						fflush(stdout);
						fflush(stdin);
						customSelect=sip_getchar();
						if((customSelect=='y')|| (customSelect=='Y'))
						{
							opt.dOption |= SIP_OPT_NOTIMER;
						}
#endif
						printf("%s", menu);
						fflush(stdout);

					break;
					}

				case 'q':
#ifndef SIP_TXN_LAYER
					sip_listForEach(&timerlist, &timerElementFree, &derror);
					sip_listDeleteAll(&timerlist, &derror);
#else

					pthread_cancel(dTimerThread);

					__sip_flushTimer();
					__sip_flushHash();
					sip_freeTimerWheel();
					sip_freeHashTable();
#endif
					sip_releaseStack();
					if(context.pList !=SIP_NULL)
					fast_memfree(0,context.pList,SIP_NULL);
#ifdef SIP_OSE
					terminateSipTest();
					kill_proc(current_process());
#endif
					exit(0);
				case ' ':
					printf("%s", menu);
					fflush(stdin);
					fflush(stdout);
					break;
				default:
				    if (c!=0x0a)
					printf("Unrecognized Key Pressed\n");
					fflush(stdin);
					fflush(stdout);
					break;
			}
		}
	}
}




/*******************************************************************************
** FUNCTION:    quoteString
**
** DESCRIPTION: This function takes an unquoted string adds quotes and returns
**        a pointer to the quoted string. The pointer must be freed by
**        the calling function sip_memfree function.
*******************************************************************************/
SIP_S8bit* quoteString ( SIP_S8bit *pString)
{
  /* Allocate memory for the quoted string, add quotes and copy the string. */
  SIP_S8bit *pQuotedString  = SIP_NULL ;
  pQuotedString = (SIP_S8bit *)fast_memget(0, strlen(pString)+3,SIP_NULL);

  strcpy(pQuotedString, "\"");
  strcat(pQuotedString, pString);
  strcat(pQuotedString, "\"\0");

  return pQuotedString;
}


#ifdef SIP_SERIALIZE
/* 
 * ---------------------------------------------------------------------
 * FUNCTION NAME : serialize_sip_message
 *
 * DESCRIPTION   : This function shall serialize a Sip Message 
 * ---------------------------------------------------------------------                 
 */ 
SipBool serialize_sip_message(SipMessage *pSipMsg,SIP_S8bit **pBuffer,
				SIP_U32bit pos,SipError *pErr) 
{
		if (sip_serializeSipMessage(pSipMsg,*pBuffer,pos,pErr) == SipFail)
		{
				printf("\nError in Serialization of Sip Message = [%d] \n",*pErr) ;
				return SipFail ;
		}
	 return SipSuccess ;	
}

/* 
 * ---------------------------------------------------------------------
 * FUNCTION NAME : deserialize_sip_message
 *
 * DESCRIPTION   : This function shall deserialize a serialized buffer
 *                 and extract a Sip Message from serialized buffer
 * ---------------------------------------------------------------------                 
 */ 

SipBool deserialize_sip_message(SipMessage **ppSipMsg,SIP_S8bit *pBuffer,
		SIP_U32bit pos, SipError *pErr)
{
		if ( sip_deserializeSipMessage(ppSipMsg,pBuffer,pos,pErr) == SipFail)
		{
				printf("\nError in DeSerialization of Sip Message = [%d] \n",*pErr) ;
				return SipFail ;
		}
		return SipSuccess ;
}
#endif /* SIP_SERIALIZE */

#ifdef SIP_PINT
/*********************************************************************
** Description : This function takes out Sdp Msg Body frm given
**             : given SipMsgBody and calls accessor functions
*              : on Sdp Pint Message Body.
** Parameters  : pSipMsgBody -> Sip Message Body from where Message
**             :               will be extracted
**             : pSdpMsg -> Sdp Message extracted from pSipMsgBody  
**             :            will be put in this. 
** Please note that this is illustrative function.
** In this follwing fields are extracted from SdpMessage
**       Version,
**       Origin and some of its subfields of Origin,
**       Media and some of its sub-fields
** Rest of the member fields of SdpMessage will map to one of
** above fields.
**********************************************************************/
SipBool extractSdpPintMsgBody(SipMsgBody *pSipMsgBody,SdpMessage *pSdpMsg)
{
   SIP_S8bit * pVersion = SIP_NULL,*pUser=SIP_NULL,*pInfo=SIP_NULL  ;
   SIP_S8bit * pSession = SIP_NULL,* pBandWidth = SIP_NULL,*pMediaValue=SIP_NULL ;
   SIP_S8bit * pProtocol = SIP_NULL ,* pSessionId = SIP_NULL ;
   SdpOrigin * pOrigin = SIP_NULL ;
   SdpMedia  * pSdpMedia = SIP_NULL ;
   SIP_U32bit index=0,bwIndex=0,count=0,bwCount=0 ;
	 SdpConnection *pConnection = SIP_NULL ;
	 SipBool res=SipSuccess ;
   SipError err = E_NO_ERROR ;

   printf("\n\nEntering extractSdpPintMsgBody \n") ;
   /* 
		* Extract Sdp Message Body from SipMsgBody 
		*/
	 printf("SdpBody -> RefCount = %d \n",pSdpMsg->dRefCount) ;
#ifdef SIP_BY_REFERENCE
   res = sip_getSdpFromMsgBody(pSipMsgBody,&pSdpMsg,&err) ;
#else
	 res = sip_initSdpMessage(&(pSdpMsg),&err);
   res = sip_getSdpFromMsgBody(pSipMsgBody,pSdpMsg,&err) ;
#endif
	 printf("SdpBody -> RefCount = %d \n",pSdpMsg->dRefCount) ;

   if ( res == SipFail)
   {
	   printf("Error = %d in extracting sdp from msg-body \n",err) ;
	   return SipFail ;
   }
   printf("**Extracted SdpMessage Body **\n") ;

   
   /*
		* Extract Version from SdpMsgBody extracted .
		* Version is of type SIP_S8bit * in SdpMessage struct.
    */
   if (sdp_getVersion(pSdpMsg,&pVersion,&err) == SipFail )
   {
	   printf("Error = %d while extracting version from sdp-msg-body\n",
		   err) ;
	   return SipFail ;
   }
	 else
	 {
			 printf("Sdp-Version = %s \n",pVersion) ;
#ifndef SIP_BY_REFERENCE
			 free(pVersion) ;
#endif
	 }

   /*
		* Extract Origin from SdpMsgBody extracted
		* Origin is of type SdpOrigin in SdpMessage struct.
		* SdpOrigin contains User,SessionId,Version,NetType and Address fields
		* and all of them are of type SIP_S8bit 
		* 
		* So we will be working right now only on first two of them.
    */
#ifdef SIP_BY_REFERENCE
   res = sdp_getOrigin(pSdpMsg,&pOrigin,&err) ;
#else
	 res = sip_initSdpOrigin(&pOrigin,&err) ;
   res = sdp_getOrigin(pSdpMsg,pOrigin,&err) ;
#endif
	 
	 if ( res == SipFail )
   {
		printf("Error = %d while extracting Origin from sdp-msg-body\n",
		   err) ;
	   return SipFail ;
   }
   printf("Extracted Origin from SdpMessage Body \n") ;


   /*
		* Extract user from origin
    */
   if ( sdp_getUserFromOrigin(pOrigin,&pUser,&err) == SipFail )
   {
	   	printf("Error = %d while extracting user from Origin of sdp-msg-body\n",
							err) ;
   }
   else
	 {
			 printf("Origin.user = %s \n",pUser) ;
#ifndef SIP_BY_REFERENCE
			 free(pUser) ;
#endif
	 }

   /*
		* Extract SessionId from Origin
    */
   if ( sdp_getSessionIdFromOrigin(pOrigin,&pSessionId,&err) == SipFail)
   {
   	printf("Error = %d while extracting Session-Id from Origin of sdp-msg-body\n",
		   err) ;
   }
   else
	 {
			 printf("Origin.Session = %s \n",pSessionId) ;
#ifndef SIP_BY_REFERENCE
			 free(pSessionId) ;
#endif
	 }

   sip_freeSdpOrigin(pOrigin) ;

   
	 /*
		* Extract SdpConnection from Sdp Message Body
		*/
#ifdef SIP_BY_REFERENCE
	res=sdp_getConnection(pSdpMsg,&pConnection,&err) ;
#else
	res=sip_initSdpConnection(&(pConnection),&err);
	if ( res == SipSuccess )
	{
			res=sdp_getConnection(pSdpMsg,pConnection,&err) ;
	}
	else
	{
			printf("Error sip_initSdpConnection  : %d \n",err) ;
	}
#endif
	if ( res == SipSuccess )
	{
			SIP_S8bit *pNType = SIP_NULL , *pAType = SIP_NULL, *pAddr = SIP_NULL ;

			printf("Extracted SdpConnection \n") ;
			if ( sdp_getNetTypeFromConnection(pConnection,&pNType,&err) == SipFail )
			{
					printf("Error : sdp_getNetTypeFromConnection : %d \n",err) ;
			}
			else
			{
					printf("NetType = %s \n",pNType) ;
#ifndef SIP_BY_REFERENCE
					free(pNType)  ;
#endif
			}

			if ( sdp_getAddrTypeFromConnection(pConnection,&pAType,&err) == SipFail )
			{
					printf("Error : sdp_getAddrTypeFromConnection : %d \n",err) ;
			}
			else
			{
					printf("Addr Type = %s \n",pAType) ;
#ifndef SIP_BY_REFERENCE
					free(pAType) ;
#endif
			}

			if ( sdp_getAddrFromConnection(pConnection,&pAddr,&err)==SipFail )
			{
					printf("Error : sdp_getAddrFromConnection : %d \n",err) ;
			}
			else
			{
					printf("Addr = %s \n",pAddr) ;
#ifndef SIP_BY_REFERENCE
					free(pAddr) ;
#endif
			}
	}
	else
	{
			printf("Error in sdp_getConnection : %d \n",err) ;
	}
	sip_freeSdpConnection(pConnection) ;
	

   /*
		* Extract Session from SdpMsgBody extracted
		* Session is of SIP_S8bit * in SdpMessage struct.
    */
   if (sdp_getSession(pSdpMsg,&pSession,&err) == SipFail )
   {
			 printf("Error = %d while extracting Origin from sdp-msg-body\n",
							 err) ;
   }
   else
	 {
			 printf("Sdp-Session = %s \n",pSession) ;
#ifndef SIP_BY_REFERENCE
			 free(pSession) ;
#endif
	 }

   /*
		* Extract Media from Sdp Message.
		* Media is a SipList of SdpMedia struct type
		* and SdpMedia contains lots of information namely
		* Key,Protocol,Information,List of Connection,List of Bandwidth,
		* Port Number,Media Values and virtual CID.
		* In following code,we will be working on 
		* Info sub-field,
		* List of Bandwidth,
		* Protocol subfield,
		* Media Value subfield.
		* Rest of fields will map to one of above.
		*/
   if ( sdp_getMediaCount(pSdpMsg,&count,&err) == SipFail )
   {
		printf("Error = %d while extracting Media-Count from sdp-msg-body\n",
		   err) ;
	   return SipFail ;
   }
   index = 0 ;


   while (index < count )
   {
			 SIP_U32bit portNum = 0 ,dCount=0 ;
			 SIP_S8bit *pFormat = SIP_NULL ;
	   /*
			* Extract Media At index from siplist of media stored in sdp-msg
			* Media is of type SdpMedia struct.
			*/
#ifdef SIP_BY_REFERENCE
	   res = sdp_getMediaAtIndex(pSdpMsg,&pSdpMedia,index,&err) ;
#else
		 res = sip_initSdpMedia(&pSdpMedia,&err) ;
	   res = sdp_getMediaAtIndex(pSdpMsg,pSdpMedia,index,&err) ;
#endif
		 if ( res == SipFail )
	   {
         printf("Error = %d while extracting media at index = %d \n",
								 err,index) ;
				 return SipFail ;
	   }
	   printf("\nExtracted Media at index = %d \n",index) ;
	   /*
			* Extract Info from Media extracted in last step
			* Info is of type SIP_S8bit *
			*/
	   if ( sdp_getInfoFromMedia(pSdpMedia,&pInfo,&err) == SipFail )
	   {
         printf("Error = %d while extracting info from media at index = %d \n",
			  err,index) ;
	   }
	   else
				 printf("Media[%d].Info = %s \n",index,pInfo) ;
#ifndef SIP_BY_REFERENCE
		 free(pInfo) ;
#endif

	   /*
			* Extract BandWidth-Count from Media extracted in last step
			* Info is of type SipList of type char *
			*/
	   if (sdp_getBandwidthCountFromMedia(pSdpMedia,&bwCount,&err) == SipFail )
	   {
				 printf("Error = %d while extracting bandwidth-count from media at "
								 "index = %d \n", err,index) ;
	   }
		 printf("Bandwidth count = %d \n",bwCount) ;
		 for (bwIndex=0;bwIndex < bwCount;bwIndex++)
	   {
	     if ( sdp_getBandwidthAtIndexFromMedia(pSdpMedia,&pBandWidth,bwIndex,&err) 
							 == SipFail )
			 {
					 printf("Error = %d while extracting Bandwidth Count at bw-index = %d "
									 "at media-index = %d \n",err,bwIndex,index);
					 return SipFail ;
			 }
			 printf("Extracted Bandwidth = %d at bw-index = %d at media-index = %d \n",
							 *pBandWidth,bwIndex,index);

		 } /* for */

		 /* Get Media-Value from Media */
		 if (sdp_getMvalueFromMedia(pSdpMedia,&pMediaValue,&err) == SipFail )
				 printf("Error = %d while extracting media-value at index = %d\n",
								 err,index) ;
	   else
		 {
				 printf("MediaValue = %s at media-index = %d \n",pMediaValue,index) ;
#ifndef SIP_BY_REFERENCE
				 free(pMediaValue) ;		 
#endif
		 }

	   /* Get Protocol from Media */
	   if (sdp_getProtoFromMedia(pSdpMedia,&pProtocol,&err) == SipFail )
		 {
				 printf("Error = %d while extracting protocol-value at index = %d\n",
								 err,index) ;
	   }
	   else
		 {
				 printf("Protocol = %s at media-index = %d \n",pProtocol,index) ;
#ifndef SIP_BY_REFERENCE
				 free(pProtocol) ;	 
#endif
		 }

		 if ( sdp_getNumportFromMedia(pSdpMedia,&portNum,&err) == SipFail )
		 {
				 printf("Error = %d while extracting protocol at index = %d\n",
								 err,index) ;
		 }
		 else
		 {
				 printf("Port Number = %d at media-index = %d \n",portNum,index) ;
		 }
		 if ( sdp_getFormatFromMedia(pSdpMedia,&pFormat,&err) == SipSuccess)
		 {
				 printf("Format : %s \n",pFormat) ;
#ifndef SIP_BY_REFERENCE
				 free(pFormat) ;
#endif
		 }
		 else
		 {
				 printf("Error in sdp_getFormatFromMedia : %d \n",err) ;
		 }
		 if ( sdp_getAttrCountFromMedia(pSdpMedia,&dCount,&err) == SipSuccess )
		 {
				 SIP_U32bit dIter =  0 ;
				 SdpAttr * pAttr = SIP_NULL ;
				 SIP_S8bit *pName = SIP_NULL , *pValue = SIP_NULL ;
				 
				 printf("\nNumber of Attributes = %d \n",dCount) ;
				 for (dIter = 0 ;dIter < dCount ; dIter++)
				 {
#ifdef SIP_BY_REFERENCE
						 res = sdp_getAttrAtIndexFromMedia(pSdpMedia,&pAttr,dIter,&err) ;
#else
						 res = sip_initSdpAttr(&pAttr,&err) ;
						 res = sdp_getAttrAtIndexFromMedia(pSdpMedia,pAttr,dIter,&err) ;
#endif
						 if ( res == SipSuccess )
						 {
								 printf("Extracted Attribute at index = %d successfully \n",
												 dIter) ;
								 if ( sdp_getNameFromAttr(pAttr,&pName,&err) == SipSuccess)
								 {
										 printf("Name  = %s",pName) ;
								     if ( strcasecmp (pName,"clir") == 0 )
								     {
										     printf("clir-tag is present in attributes \n") ;
								     }
								     else if ( strcasecmp(pName,"q763-nature") == 0 )
								     {
									    	 printf("Q763-nature is present in attributes \n") ;
								     }
								     else if ( strcasecmp(pName,"q763-plan") == 0 )
								     {
										     printf("Q763-plan is present in attributes \n") ;
								     }
								     else
								     {
										     printf("\nNeither clir nor Q763-nature is present\n") ;
								     }
#ifndef SIP_BY_REFERENCE
						 free(pName) ;
#endif
								      if ( sdp_getValueFromAttr(pAttr,&pValue,&err) == SipSuccess)
								      {
										      printf("\tValue = %s\n",pValue) ;
#ifndef SIP_BY_REFERENCE
						      free(pValue) ;
#endif
                        }
						    }
						    else
						    {
								    printf("Error in sdp_getAttrAtIndexFromMedia : %d \n",err) ;
						    }
						    sip_freeSdpAttr(pAttr) ;
				    }
          }/* for loop */
        } 
		    else
		    {
				      printf("Error : sdp_getAttrCountFromMedia : %d \n",err) ;
		    }
	          sip_freeSdpMedia(pSdpMedia) ;
	          index++ ;
  }/* while */
	 sip_freeSdpMessage(pSdpMsg) ;
   printf("Exiting extractSdpPintMsgBody \n") ;
 return SipSuccess ;
}

#ifdef SIP_BY_REFERENCE
SipBool processSdpPintBodyInByReference (SipMessage *pSipMsg)
{
   SIP_U32bit index=0,msgBodyCount=0;
   SipError err=E_NO_ERROR;
   en_SipMsgBodyType msgBodyType=SipBodyAny ;
   SipMsgBody *pSipMsgBody = SIP_NULL ;
   SipHeader contentTypeHdr ;
   SdpMessage *pSdpMsg = NULL ;
   SIP_S8bit *pMediaType = SIP_NULL ;



   printf("Entering processSdpPintBody () \n") ;
   /* 
    Extract count of SipMsgBody from SipMessage 
   */
	 contentTypeHdr.dType=SipHdrTypeContentType ;
	 contentTypeHdr.pHeader=SIP_NULL ;
	 
   if (sip_getMsgBodyCount(pSipMsg,&msgBodyCount,&err) == SipFail)
   {
		printf("Error while extracting sip-mesg-count : %d \n",err) ;
		return SipFail ;
		/*
		 * Possible errors
		 * -> E_NO_EXIST  (when no message-body exists in sip-messsage)
		 * -> E_INV_PARAM (when no message exists or count-param
		 *         is not pointing to valid memory)
		 */
	}
	printf("No of message-body in sip-message = %d \n",msgBodyCount) ;
   /* 
		* To get the Type of Message Body , extract contenttype header frm sip-msg 
		*/
   if ( sip_getHeader(pSipMsg,SipHdrTypeContentType,&contentTypeHdr,&err) == SipFail)
	 {
			 printf("Error = %d while initialising the header \n",err ) ;
			 return SipFail ;
	 }
   /* 
		* Get Media Type from Content-Type header 
		*/
	 if ( sip_getMediaTypeFromContentTypeHdr(&contentTypeHdr,&pMediaType,&err) 
					 == SipFail )
   {
			 printf("Error = %d while extracting Content-Type-Hdr \n",err) ;
			 sip_freeSipContentTypeHeader((SipContentTypeHeader *)(contentTypeHdr.pHeader));
			 return SipFail ;
   }
	 sip_freeSipContentTypeHeader((SipContentTypeHeader *)(contentTypeHdr.pHeader));
	 printf("Extracted MediaType = %s \n",pMediaType) ;

    /* 
		 * Start working on the list of message bodies one by one 
		 */
	 while (index < msgBodyCount )
	 {
			 printf("Extract msg-body at index = %d from sip-msg\n",index) ;
		/* 
		 * Get the message body located at "index" 
		 */
			 if ( sip_getMsgBodyAtIndex(pSipMsg,&pSipMsgBody,index,&err) == SipFail )
			 {
					 printf("Error = %d while extracting msg body at index  = %d \n",
									 err,index) ;
					 return SipFail ;

			/* 
			 * Possible errors
			 * -> E_INV_PARAM (when no message exists or count-param
			 *                  is not pointing to valid memory)
			 * -> E_INV_INDEX (when index from where the message-body is to
			 *                  extracted is wrong)
			 * -> E_NO_EXIST  (when no message-body exists in sip-message)
			 */

		}

		/* 
		 * Get the message body type of the body extracted 
		 */
			 if (sip_getTypeFromMsgBody(pSipMsgBody,&msgBodyType,&err) == SipFail )
			 {
					 printf("Error = %d while extracting message type frm msg-body "
									 "at index=%d\n",err,index) ;
					 return SipFail ;

					 /* 
						* Possible errors
						* -> E_INV_PARAM (when no message exists or count-param
						*           is not pointing to valid memory)
						*-> E_INV_INDEX (when index from where the message-body is to
						*                extracted is wrong)
						*-> E_NO_EXIST  (when no message-body exists in sip-message)
						*/
			 }


		switch (msgBodyType)
		{
				case  SipSdpBody : /* MsgBody is SDP type (content-type header) */
						{
								printf("Message Body Type is Sdp Type \n") ;
								if (sip_initSdpMessage(&pSdpMsg,&err) == SipFail )
								{
										printf("Error = %d while init-sdp-mesg \n",err);
										return SipFail ;
								}
								printf("SdpBody -> RefCount = %d \n",pSdpMsg->dRefCount) ;
								if (extractSdpPintMsgBody(pSipMsgBody,pSdpMsg) == SipFail )
								{
										printf("Error = %d while extracting sdp-msg-body\n",err) ;
										return SipFail ;
								}
								sip_freeSdpMessage(pSdpMsg) ;
								break ;
						}
				default:
						{
								printf("Message Body Type is Other than Sdp \n") ;
								break ;
						}
		}/* switch */
		index ++ ;
		msgBodyType=SipBodyAny ; /* initializing the variable back */

		sip_freeSipMsgBody(pSipMsgBody) ; /* to decrement the ref-count */
	 }/* while */
	 printf("Exiting processSipMsgBody() \n") ;
	return SipSuccess ; 
}
#endif

#ifndef SIP_BY_REFERENCE
SipBool processSdpPintBodyInNonByReference (SipMessage *pSipMsg)
{
   SIP_U32bit index=0,msgBodyCount=0;
   SipError err=E_NO_ERROR;
   en_SipMsgBodyType msgBodyType=SipBodyAny ;
   SipMsgBody *pSipMsgBody = SIP_NULL ;
   SipHeader *pContentTypeHdr = SIP_NULL ;
   SdpMessage *pSdpMsg = NULL ;
   SIP_S8bit *pMediaType = SIP_NULL ;



   printf("Entering processSdpPintBodyInNonByReference() \n") ;
   /* 
    Extract count of SipMsgBody from SipMessage 
   */
	 sip_initSipHeader(&pContentTypeHdr,SipHdrTypeContentType,&err) ;
	 
   if (sip_getMsgBodyCount(pSipMsg,&msgBodyCount,&err) == SipFail)
   {
		printf("Error while extracting sip-mesg-count : %d \n",err) ;
		return SipFail ;
		/*
		 * Possible errors
		 * -> E_NO_EXIST  (when no message-body exists in sip-messsage)
		 * -> E_INV_PARAM (when no message exists or count-param
		 *         is not pointing to valid memory)
		 */
	}
	printf("No of message-body in sip-message = %d \n",msgBodyCount) ;
   /* 
		* To get the Type of Message Body , extract contenttype header frm sip-msg 
		*/
   if ( sip_getHeader(pSipMsg,SipHdrTypeContentType,pContentTypeHdr,&err) == SipFail)
	 {
			 printf("Error = %d while initialising the header \n",err ) ;
			 free(pContentTypeHdr->pHeader) ;
			 sip_freeSipHeader(pContentTypeHdr);
			 return SipFail ;
	 }
   /* 
		* Get Media Type from Content-Type header 
		*/
	 if ( sip_getMediaTypeFromContentTypeHdr(pContentTypeHdr,&pMediaType,&err) 
					 == SipFail )
	 {
			 printf("Error = %d while extracting Content-Type-Hdr \n",err) ;
			 free(pContentTypeHdr->pHeader) ;
			 sip_freeSipHeader(pContentTypeHdr);
			 return SipFail ;
   }
	 printf("Extracted MediaType = %s \n",pMediaType) ;
	 free(pMediaType) ;
	 sip_freeSipHeader(pContentTypeHdr);

    /* 
		 * Start working on the list of message bodies one by one 
		 */
	 while (index < msgBodyCount )
	 {
			 printf("Extract msg-body at index = %d from sip-msg\n",index) ;
			 sip_initSipMsgBody(&pSipMsgBody,SipSdpBody,&err) ;
		/* 
		 * Get the message body located at "index" 
		 */
			 if ( sip_getMsgBodyAtIndex(pSipMsg,pSipMsgBody,index,&err) == SipFail )
			 {
					 printf("Error = %d while extracting msg body at index  = %d \n",
									 err,index) ;
					 return SipFail ;

			/* 
			 * Possible errors
			 * -> E_INV_PARAM (when no message exists or count-param
			 *                  is not pointing to valid memory)
			 * -> E_INV_INDEX (when index from where the message-body is to
			 *                  extracted is wrong)
			 * -> E_NO_EXIST  (when no message-body exists in sip-message)
			 */

		}

		/* 
		 * Get the message body type of the body extracted 
		 */
			 if (sip_getTypeFromMsgBody(pSipMsgBody,&msgBodyType,&err) == SipFail )
			 {
					 printf("Error = %d while extracting message type frm msg-body "
									 "at index=%d\n",err,index) ;
					 return SipFail ;

					 /* 
						* Possible errors
						* -> E_INV_PARAM (when no message exists or count-param
						*           is not pointing to valid memory)
						*-> E_INV_INDEX (when index from where the message-body is to
						*                extracted is wrong)
						*-> E_NO_EXIST  (when no message-body exists in sip-message)
						*/
			 }


		switch (msgBodyType)
		{
				case  SipSdpBody : /* MsgBody is SDP type (content-type header) */
						{
								printf("Message Body Type is Sdp Type \n") ;
								if (sip_initSdpMessage(&pSdpMsg,&err) == SipFail )
								{
										printf("Error = %d while init-sdp-mesg \n",err);
										return SipFail ;
								}
								printf("SdpBody -> RefCount = %d \n",pSdpMsg->dRefCount) ;
								if (extractSdpPintMsgBody(pSipMsgBody,pSdpMsg) == SipFail )
								{
										printf("Error = %d while extracting sdp-msg-body\n",err) ;
										return SipFail ;
								}
								sip_freeSdpMessage(pSdpMsg) ;
								break ;
						}
				default:
						{
								printf("Message Body Type is Other than Sdp \n") ;
								break ;
						}
		}/* switch */
		index ++ ;
		msgBodyType=SipBodyAny ; /* initializing the variable back */

		sip_freeSipMsgBody(pSipMsgBody) ; /* to decrement the ref-count */
	 }/* while */
	 printf("Exiting processSdpPintBodyInNonByReference() \n") ;
	return SipSuccess ; 
}
#endif
#endif/*SDP_PINT */

#ifdef __cplusplus
}
#endif

