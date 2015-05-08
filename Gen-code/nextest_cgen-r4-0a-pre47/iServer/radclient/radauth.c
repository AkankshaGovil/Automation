#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <time.h>
#include <libradius.h>
#include <conf.h>
#include <radpaths.h>
#include "cdr.h"
#include "radclient.h"
#include "srvrlog.h"
#include "licenseIf.h"
#include "lsconfig.h"
#include "log.h"
#include "bridge.h"

#define Val(_x_)       ((_x_)?(_x_):"")


#ifndef INADDR_NONE
#define INADDR_NONE     ((in_addr_t) 0xffffffff)
#endif 

int radius_initialised;

static int radauth_port;

static int *radauthinPool;
static int *radauthinClass;

static int totalapp = 0;
static int totalchal = 0;
static int totaldeny = 0;


extern int bridgeSipEventProcessorWorker(SCC_EventBlock *evtPtr);

int initRadiusAuth()
{
	char fn[] = "initRadiusAuth";
	int i;

	if((radauth_port = rad_getport("radauth")) == 0)
	{
		radauth_port = PW_AUTH_UDP_PORT;
	}

	radauthinPool = (int *)malloc(nRadiusThreads*sizeof(int));
	radauthinClass = (int *)malloc(nRadiusThreads*sizeof(int));

	for(i = 0; i < nRadiusThreads; ++i)
	{
		radauthinPool[i] = ThreadPoolInit("radauth", 1, PTHREAD_SCOPE_PROCESS, 0, 1);

		radauthinClass[i] = ThreadAddPoolClass("radauth-class", radauthinPool[i], 0, 100000000);

		ThreadPoolStart(radauthinPool[i]);
	}

	return 0;
}


void stopRadiusAuth()
{
	int i;

	for(i = 0; i < nRadiusThreads; ++i)
	{
		ThreadPoolEnd(radauthinPool[i]);
	}
}


int getChallenge(header_url* from, char* callid, Challenge* challenge)
{
	char fn[] = "getChallenge";
	RADIUS_PACKET *req;
	RADIUS_PACKET *rep = NULL;
	DICT_ATTR  *da;
	VALUE_PAIR *vp;
	char *realm = "", *nonce= "", *algorithm = "";
	char *username;
	int len = strlen("sip:") + strlen(from->name) + strlen("@") + strlen(from->host) + 1;
	int ret = -1;
	struct sockaddr_in cliaddr;

	if(!radius_initialised) return -1;

	if((username = malloc(len)) == NULL)
	{
		NETERROR(MRADC, ("%s Failed to alloc space for username \n", fn));

		goto _error;
	}

	strcpy(username, "sip:");
	strcat(username, from->name);
	strcat(username, "@");
	strcat(username, from->host);

	if((req = rad_alloc(1)) == NULL)
	{
		NETERROR(MRADC, ("%s Failed to alloc request \n", fn));

		goto _error;
	}

	req->code = PW_AUTHENTICATION_REQUEST;

	if((req->dst_ipaddr = ip_getaddr(rad_server_addr[0])) == INADDR_NONE)
	{
		NETERROR(MRADC, ("%s Unable to determine address for Radius Server \n", fn));

		goto _error;
	}

	req->dst_port = radauth_port;

	vp = pairmake("User-Name", username, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"User-Name\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("CHAP-Password", "challenge", T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"CHAP-Password\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("SIP-Callid", callid, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"SIP-Callid\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	if((req->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		NETERROR(MRADC, ("%s Unable to create socket \n", fn));

		goto _error;
	}

	// if management interface is configured, bind to it
	if( strlen(mgmtInterfaceIp) )
	{
		bzero(&cliaddr,sizeof(cliaddr));
		cliaddr.sin_family = AF_INET;
		cliaddr.sin_addr.s_addr = inet_addr(mgmtInterfaceIp);
		cliaddr.sin_port = htons(0);

		if(bind(req->sockfd,(struct sockaddr *)&cliaddr,sizeof(cliaddr)) < 0)
		{
			NETERROR(MRADC, ("%s Unable to bind socket \n", fn));
			goto _error;
		}
	}

	librad_md5_calc(req->vector, req->vector, sizeof(req->vector));

	if(rad_send_packet(req, &rep, secret[0]) < 0)
	{
		NETERROR(MRADC, ("%s Unable to send packet to %s\n", fn, rad_server_addr[0]));

		goto _error;
	}

	if(rep->code == PW_AUTHENTICATION_ACK)
	{
		ret = 0;
	}
	else if(rep->code == PW_ACCESS_CHALLENGE)
	{
		if((da = dict_attrbyname("Digest-Realm")) == 0)
		{
			NETERROR(MRADC, ("%s failed to find attribute \"Digest-Realm\" \n", fn));

			goto _error;
		}

		vp = pairfind(rep->vps, da->attr);
		if(vp == NULL || strcmp(vp->strvalue, "") == 0)
		{
			NETERROR(MRADC, ("%s attribute \"Digest-Realm\" must have value for authentication \n", fn));

			goto _error;
		}

		realm = vp->strvalue;

		if((da = dict_attrbyname("Nonce")) == 0)
		{
			NETERROR(MRADC, ("%s failed to find attribute \"Nonce\" \n", fn));

			goto _error;
		}

		vp = pairfind(rep->vps, da->attr);
		if(vp == NULL || strcmp(vp->strvalue, "") == 0)
		{
			NETERROR(MRADC, ("%s attribute \"Nonce\" must have value for authentication \n", fn));

			goto _error;
		}

		nonce = vp->strvalue;

		if((da = dict_attrbyname("Algorithm")) == 0)
		{
			NETERROR(MRADC, ("%s failed to find attribute \"Algorithm\" \n", fn));

			goto _error;
		}

		vp = pairfind(rep->vps, da->attr);
		if(vp == NULL || strcmp(vp->strvalue, "") == 0)
		{
			NETERROR(MRADC, ("%s attribute \"Algorithm\" must have value for authentication \n", fn));

			goto _error;
		}

		algorithm = vp->strvalue;

		ret = 1;
	}

_error:

	challenge->realm = strdup(realm);
	challenge->nonce = strdup(nonce);
	challenge->algorithm = strdup(algorithm);

	free(username);

	if(req)
	{
		if(req->sockfd)
		{
			close(req->sockfd);
		}

		rad_free(&req);
	}

	if(rep)
	{
		rad_free(&rep);
	}

	return(ret);
}

/* This routine fixes up Digest-Attributes issues
 * Ported from radclient.c
 */
VALUE_PAIR	*pairmakeDigest(const char *attribute, const char *value, int operator)
{
	char fn[] = "pairmakeDigest";
	VALUE_PAIR *vp;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	vp = pairmake(attribute, value, operator);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to make attribue %s \n", fn, attribute));

		goto _error;
	}

	/* overlapping! */
	memmove(&vp->strvalue[2], &vp->strvalue[0], vp->length);
	vp->strvalue[0] = vp->attribute - PW_DIGEST_REALM + 1;
	vp->length += 2;
	vp->strvalue[1] = vp->length;
	vp->attribute = PW_DIGEST_ATTRIBUTES;

_error:

    return (vp);

} /* pairmakeDigest */


int sendCredentials(header_url* from, char* callid, Credentials* credentials)
{
	char fn[] = "sendCredentials";
	RADIUS_PACKET *req = NULL;
	RADIUS_PACKET *rep = NULL;
	VALUE_PAIR *vp;
	char *username;
	int len = strlen(credentials->authentication_name) + strlen("@") + strlen(from->host) + 1;
	int ret = -1;
	struct sockaddr_in cliaddr;

	if(!radius_initialised) return -1;

	if((username = malloc(len)) == NULL)
	{
		NETERROR(MRADC, ("%s Failed to alloc space for username \n", fn));

		goto _error;
	}

	strcpy(username, credentials->authentication_name);

	if((req = rad_alloc(1)) == NULL)
	{
		NETERROR(MRADC, ("%s Failed to alloc request \n", fn));

		goto _error;
	}

	req->code = PW_AUTHENTICATION_REQUEST;

	if((req->dst_ipaddr = ip_getaddr(rad_server_addr[0])) == INADDR_NONE)
	{
		NETERROR(MRADC, ("%s Unable to determine address for Radius Server \n", fn));

		goto _error;
	}

	req->dst_port = radauth_port;
	vp = pairmake("User-Name", username, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to make attribue \"User-Name\" \n", fn));
		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmakeDigest("Digest-User-Name", username, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to make attribue \"User-Name\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmakeDigest("Digest-Method", credentials->method, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to make attribue \"Method\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmakeDigest("Digest-Realm", credentials->realm, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to make attribue \"Digest-Realm\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmakeDigest("Digest-URI", credentials->uri, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to make attribue \"Digest-URI\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("Digest-Response", credentials->digest_response, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to make attribue \"Digest-Response\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmakeDigest("Digest-Nonce", credentials->nonce, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"Nonce\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	vp = pairmakeDigest("Digest-Algorithm", credentials->algorithm, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"Algorithm\" \n", fn));

		goto _error;
	}
	pairadd(&req->vps, vp);

	if((req->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		NETERROR(MRADC, ("%s Unable to create socket \n", fn));

		goto _error;
	}

	// if management interface is configured, bind to it
	if( strlen(mgmtInterfaceIp) )
	{
		bzero(&cliaddr,sizeof(cliaddr));
		cliaddr.sin_family = AF_INET;
		cliaddr.sin_addr.s_addr = inet_addr(mgmtInterfaceIp);
		cliaddr.sin_port = htons(0);

		if(bind(req->sockfd,(struct sockaddr *)&cliaddr,sizeof(cliaddr)) < 0)
		{
			NETERROR(MRADC, ("%s Unable to bind socket \n", fn));
			goto _error;
		}
	}

	librad_md5_calc(req->vector, req->vector, sizeof(req->vector));

	if(rad_send_packet(req, &rep, secret[0]) < 0)
	{
		NETERROR(MRADC, ("%s Unable to send packet to %s\n", fn, rad_server_addr[0]));

		goto _error;
	}

	if(rep->code == PW_AUTHENTICATION_ACK)
	{
		ret = 0;
	}
	else if(rep->code == PW_ACCESS_CHALLENGE)
	{
		ret = 1;
	}

_error:

	if(req)
	{
		if(req->sockfd)
		{
			close(req->sockfd);
		}

		rad_free(&req);
	}
			
	if(rep)
	{
		rad_free(&rep);
	}

	return(ret);
}


static int sendCiscoRadiusAuthenticate(CallHandle *callHandle)
{
	char fn[] = "sendCiscoRadiusAuthenticate";
	RADIUS_PACKET *req = NULL;
	RADIUS_PACKET *rep = NULL;
	DICT_ATTR  *da;
	DICT_VALUE *v;
	VALUE_PAIR *vp;
	PhoNode *local, *remote;
	char buf[256];
	int ret = -1;
	struct sockaddr_in cliaddr;

	if(callHandle->callSource == 0)
	{
		local = &callHandle->phonode;
		remote = &callHandle->rfphonode;
	}
	else
	{
		local = &callHandle->rfphonode;
		remote = &callHandle->phonode;
	}

	if((req = rad_alloc(1)) == NULL)
	{
		NETERROR(MRADC, ("%s Failed to alloc request \n", fn));

		goto _error;
	}

	req->code = PW_AUTHENTICATION_REQUEST;

	if((req->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		NETERROR(MRADC, ("%s Unable to create socket \n", fn));

		goto _error;
	}

	// if management interface is configured, bind to it
	if( strlen(mgmtInterfaceIp) )
	{
		bzero(&cliaddr,sizeof(cliaddr));
		cliaddr.sin_family = AF_INET;
		cliaddr.sin_addr.s_addr = inet_addr(mgmtInterfaceIp);
		cliaddr.sin_port = htons(0);

		if(bind(req->sockfd,(struct sockaddr *)&cliaddr,sizeof(cliaddr)) < 0)
		{
			NETERROR(MRADC, ("%s Unable to bind socket \n", fn));
			goto _error;
		}
	}

	if((req->dst_ipaddr = ip_getaddr(rad_server_addr[0])) == INADDR_NONE)
	{
		NETERROR(MRADC, ("%s Unable to determine address for Radius Server \n", fn));

		goto _error;
	}

	req->dst_port = radauth_port;

	vp = pairmake("NAS-IP-Address", nas_ipaddr, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"NAS-IP-Address\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("NAS-Port-Type", "Async", T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"NAS-Port\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("User-Name", first_auth_username, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"User-Name\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	snprintf(buf, sizeof(buf), "h323-conf-id=%s", callHandle->conf_id);

	vp = pairmake("h323-conf-id", buf, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"h323-conf-id\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	FormatIpAddress(callHandle->peerIp, buf);

	vp = pairmake("Calling-Station-Id", buf, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"Calling-Station-Id\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("User-Password", first_auth_password, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"User-Password\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	snprintf(buf, sizeof(buf), "remote-media-address=");
	FormatIpAddress(local->ipaddress.l, &buf[strlen(buf)]);

	vp = pairmake("remote-media-address", buf, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"remote-media-address\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	librad_md5_calc(req->vector, req->vector, sizeof(req->vector));

	vp = pairfind(req->vps, PW_PASSWORD);
	if(vp)
	{
		rad_pwencode((char*)vp->strvalue, &(vp->length), secret[0], (char*)req->vector);
	}

	if(rad_send_packet(req, &rep, secret[0]) < 0)
	{
		NETERROR(MRADC, ("%s Unable to send packet to %s\n", fn, rad_server_addr[0]));

		goto _error;
	}

	if(rep->code == PW_AUTHENTICATION_ACK)
	{
		ret = 0;
	}

_error:

	if(req)
	{
		if(req->sockfd)
		{
			close(req->sockfd);
		}

		rad_free(&req);
	}
			
	if(rep)
	{
		rad_free(&rep);
	}

	return(ret);
}


int SipCiscoRadiusAuthenticateWorker(SCC_EventBlock *evtPtr)
{
	char fn[] = "SipCiscoRadiusAuthenticateWorker";
	CallHandle *callHandle = NULL;
	SipAppCallHandle *pSipData;
	char srcCallID[CALL_ID_LEN];
	char callIDStr[CALL_ID_LEN];
	int sendError = 0;

	if(!evtPtr)
	{
		NETERROR(MRADC,("%s Null EventPtr \n",fn));
		return -1;
	}

	pSipData = (SipAppCallHandle *) evtPtr->data;
	memcpy(srcCallID, evtPtr->callID, CALL_ID_LEN);

	if(!(callHandle = CacheGet(callCache, evtPtr->callID)))
	{
		NETERROR(MRADC,("%s Failed to find CallId %s\n",
				fn, (char*) CallID2String(evtPtr->callID, callIDStr)));
		evtPtr->callDetails.callError = SCC_errorResourceUnavailable;
		pSipData->responseCode = 481;
		goto _error;
	}

	if(sendCiscoRadiusAuthenticate(callHandle) == 0)
	{
		evtPtr->evtProcessor = bridgeSipEventProcessorWorker;

		return bridgeQueueEvent(evtPtr);
	}
	else
	{
		evtPtr->callDetails.callError = SCC_errorResourceUnavailable;
		pSipData->responseCode = 403;
		goto _error;
	}

_error:
	if(callHandle)
	{
		memcpy(pSipData->callID, callHandle->callID, CALL_ID_LEN);
		memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

		evtPtr->event = Sip_eBridgeFinalResponse;
		evtPtr->evtProcessor = NULL;

		sendError = 1;
	}

	GisDeleteCallFromConf(srcCallID, evtPtr->confID);

	if(sendError && (sipBridgeEventProcessor(evtPtr) <0) )
	{
		NETDEBUG(MRADC, NETLOG_DEBUG2,
			("%s bridgeEventProcessor returned error\n",fn));
	}

	return -1;
}


int SipCiscoRadiusAuthenticate(SipEventHandle *evb)
{
	char fn[] = "SipCiscoRadiusAuthenticate";
	CallHandle 		*callHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	SCC_EventBlock *evPtr;
	unsigned short index = 0;

	if(!radius_initialised) return -1;

	memcpy(&index, callHandle->confID+14, 2);
	index %= nRadiusThreads;

	appMsgHandle = SipEventAppHandle(evb);
	SipEventAppHandle(evb) = NULL;

	/* Allocate an SCC Event Block and fill it with data */
	evPtr = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(evPtr, 0, sizeof(SCC_EventBlock));

	evPtr->event = evb->event;
	evPtr->data = appMsgHandle;

	SipFreeEventHandle(evb);

	memcpy(evPtr->callID, appMsgHandle->callID, CALL_ID_LEN);
	memcpy(evPtr->confID, appMsgHandle->confID, CONF_ID_LEN);

	evPtr->evtProcessor = bridgeSipEventProcessorWorker;

	if(ThreadDispatch(radauthinPool[index], radauthinClass[index],
			(void *(*)(void*))SipCiscoRadiusAuthenticateWorker, evPtr,
			1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		NETERROR(MSIP, ("%s SipCiscoRadiusAuthenticate dispatch error\n", fn));

		// Free the call handle and return an error for now
		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

		callHandle = CacheGet(callCache, evPtr->callID);

		if(callHandle == NULL)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Call Handle already deleted?\n", fn));
		
			goto _release_locks;
		}

		CacheDelete(callCache, callHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);

		/* Delete the call Handle */
		GisFreeCallHandle(callHandle);

	_release_locks:
		CacheReleaseLocks(callCache);

		return -1;
	}

	return 0;
}


static int sendCiscoRadiusAuthourize(CallHandle *callHandle)
{
	char fn[] = "sendCiscoRadiusAuthourize";
	RADIUS_PACKET *req = NULL;
	RADIUS_PACKET *rep = NULL;
	DICT_ATTR  *da;
	DICT_VALUE *v;
	VALUE_PAIR *vp;
	PhoNode *local, *remote;
	char buf[256];
	int ret = -1;
	struct sockaddr_in cliaddr;

	if(callHandle->callSource == 0)
	{
		local = &callHandle->phonode;
		remote = &callHandle->rfphonode;
	}
	else
	{
		local = &callHandle->rfphonode;
		remote = &callHandle->phonode;
	}

	if((req = rad_alloc(1)) == NULL)
	{
		NETERROR(MRADC, ("%s Failed to alloc request \n", fn));

		goto _error;
	}

	req->code = PW_AUTHENTICATION_REQUEST;

	if((req->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		NETERROR(MRADC, ("%s Unable to create socket \n", fn));

		goto _error;
	}

	// if management interface is configured, bind to it
	if( strlen(mgmtInterfaceIp) )
	{
		bzero(&cliaddr,sizeof(cliaddr));
		cliaddr.sin_family = AF_INET;
		cliaddr.sin_addr.s_addr = inet_addr(mgmtInterfaceIp);
		cliaddr.sin_port = htons(0);

		if(bind(req->sockfd,(struct sockaddr *)&cliaddr,sizeof(cliaddr)) < 0)
		{
			NETERROR(MRADC, ("%s Unable to bind socket \n", fn));
			goto _error;
		}
	}

	if((req->dst_ipaddr = ip_getaddr(rad_server_addr[0])) == INADDR_NONE)
	{
		NETERROR(MRADC, ("%s Unable to determine address for Radius Server \n", fn));

		goto _error;
	}

	req->dst_port = radauth_port;

	vp = pairmake("NAS-IP-Address", nas_ipaddr, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"NAS-IP-Address\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("NAS-Port-Type", "Async", T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"NAS-Port\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("User-Name", second_auth_username, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"User-Name\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	snprintf(buf, sizeof(buf), "h323-conf-id=%s", callHandle->conf_id);

	vp = pairmake("h323-conf-id", buf, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"h323-conf-id\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("Called-Station-Id", remote->phone, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"Called-Station-Id\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("Calling-Station-Id", local->phone, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"Calling-Station-Id\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	vp = pairmake("User-Password", second_auth_password, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"User-Password\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	snprintf(buf, sizeof(buf), "remote-media-address=");
	FormatIpAddress(local->ipaddress.l, &buf[strlen(buf)]);

	vp = pairmake("remote-media-address", buf, T_OP_EQ);
	if(vp == NULL)
	{
		NETERROR(MRADC, ("%s Failed to find attribue \"remote-media-address\" \n", fn));

		return -1;
	}
	pairadd(&req->vps, vp);

	librad_md5_calc(req->vector, req->vector, sizeof(req->vector));

	vp = pairfind(req->vps, PW_PASSWORD);
	if(vp)
	{
		rad_pwencode((char*)vp->strvalue, &(vp->length), secret[0], (char*)req->vector);
	}

	if(rad_send_packet(req, &rep, secret[0]) < 0)
	{
		NETERROR(MRADC, ("%s Unable to send packet to %s\n", fn, rad_server_addr[0]));

		goto _error;
	}

	if(rep->code == PW_AUTHENTICATION_ACK)
	{
		ret = 0;
	}

_error:

	if(req)
	{
		if(req->sockfd)
		{
			close(req->sockfd);
		}

		rad_free(&req);
	}
			
	if(rep)
	{
		rad_free(&rep);
	}

	return(ret);
}


int SipCiscoRadiusAuthourizeWorker(SCC_EventBlock *evtPtr)
{
	char fn[] = "SipCiscoRadiusAuthourizeWorker";
	CallHandle *callHandle = NULL;
	SipAppCallHandle *pSipData;
	char srcCallID[CALL_ID_LEN];
	char callIDStr[CALL_ID_LEN];
	int sendError = 0;

	if(!evtPtr)
	{
		NETERROR(MRADC,("%s Null EventPtr \n",fn));
		return -1;
	}

	pSipData = (SipAppCallHandle *) evtPtr->data;
	memcpy(srcCallID, evtPtr->callID, CALL_ID_LEN);

	if(!(callHandle = CacheGet(callCache, evtPtr->callID)))
	{
		NETERROR(MRADC,("%s Failed to find CallId %s\n",
				fn, (char*) CallID2String(evtPtr->callID, callIDStr)));
		evtPtr->callDetails.callError = SCC_errorResourceUnavailable;
		pSipData->responseCode = 481;
		goto _error;
	}

	if(sendCiscoRadiusAuthourize(callHandle) == 0)
	{
		evtPtr->evtProcessor = bridgeSipEventProcessorWorker;

		return bridgeQueueEvent(evtPtr);
	}
	else
	{
		evtPtr->callDetails.callError = SCC_errorResourceUnavailable;
		pSipData->responseCode = 403;
		goto _error;
	}

_error:
	if(callHandle)
	{
		memcpy(pSipData->callID, callHandle->callID, CALL_ID_LEN);
		memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

		evtPtr->event = Sip_eBridgeFinalResponse;
		evtPtr->evtProcessor = NULL;

		sendError = 1;
	}

	GisDeleteCallFromConf(srcCallID, evtPtr->confID);

	if(sendError && (sipBridgeEventProcessor(evtPtr) <0) )
	{
		NETDEBUG(MRADC, NETLOG_DEBUG2,
			("%s bridgeEventProcessor returned error\n",fn));
	}

	return -1;
}


int SipCiscoRadiusAuthourize(SCC_EventBlock *evtPtr)
{
	char fn[] = "SipCiscoRadiusAuthourize";
	CallHandle *callHandle = NULL;
	unsigned short index = 0;

	if(!radius_initialised) return -1;

	memcpy(&index, callHandle->confID+14, 2);
	index %= nRadiusThreads;

	evtPtr->evtProcessor = bridgeSipEventProcessorWorker;

	if(ThreadDispatch(radauthinPool[index], radauthinClass[index],
			(void *(*)(void*))SipCiscoRadiusAuthourizeWorker, evtPtr,
			1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		NETERROR(MSIP, ("%s SipCiscoRadiusAuthourize dispatch error\n", fn));

		// Free the call handle and return an error for now
		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

		callHandle = CacheGet(callCache, evtPtr->callID);

		if(callHandle == NULL)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Call Handle already deleted?\n", fn));
		
			goto _release_locks;
		}

		CacheDelete(callCache, callHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);

		/* Delete the call Handle */
		GisFreeCallHandle(callHandle);

	_release_locks:
		CacheReleaseLocks(callCache);

		return -1;
	}

	return 0;
}
