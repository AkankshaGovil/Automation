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
#include "net.h"
#include <netdb.h>
#include <malloc.h>
#include "nxosd.h"
#include "ssip.h"
#include "sipstring.h"
#include "siputils.h"
#include "firewallcontrol.h"
#include "tsm.h"
#include "dbs.h"

/* This routine checks if we need to challenge a method
 * return 1 if true
 * return 0 if false
 */
int
SipChallengeMethod (char *method, unsigned int authFlags)
{
	char fn[] = "SipChallengeMethod():";
	int  rc = 0;

	DEBUG(MSIP, NETLOG_DEBUG3,
		("%s Received an incoming request: %s\n", fn,method));

	/* currently, we only challenge register and invite
	 * register is handled in register.c
	 */
	if( strcasecmp(method, "INVITE") == 0 && (authFlags & REALM_SIP_AUTH_INVITE) )
	{
		rc = 1;
	}
	else if( strcasecmp(method, "REGISTER") == 0 && (authFlags & REALM_SIP_AUTH_REGISTER) )
	{
		rc = 1;
	}
	else if( strcasecmp(method, "BYE") == 0 && (authFlags & REALM_SIP_AUTH_BYE) )
	{
		rc = 1;
	}
	return rc;

} /* SipChallengeMethod */


/* If the incoming request uri is in our cache,
 * We will derive the phone number from the cache
 * entry we find, and pass that phone number to resolve.
 * If the uri is not in our cache, and the domain of the uri
 * is not our domain, then we just forward the uri, by doing
 * a dns query. If the domain is the iServer domain, then we 
 * do the Resolution, assuming that the user part of the uri
 * is a phone number.
 * In the resulting (after resolution), if there is a contact 
 * address specified, then that is used, otherwise the contact
 * is the phonenumber or sipuser in the entry @ ip address in the
 * cache entry, if its there. If the ip address is not there,
 * a 404 will be returned.
 */
void 
SipIncomingRequest ( 
	SipMessage *s, 
	char *method,
	SipEventContext *context 
)
{
	char fn[] = "SipIncomingRequest():";
	SIP_S8bit *callId;
	SIP_S8bit *host,branchtoken[64];
	SIP_U16bit hostport;
	SipError err = 0;
	SIP_U32bit cseq,count;
	header_url *req_uri = NULL, *newreq_uri = NULL;
	int rc = -1, reason = 400;
	char requri[SIPURL_LEN], *tmpstr;
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry;
	SipMessage *resp = NULL;
	int dHops;
	en_HeaderType dType;
	SipBadHeader *badheader;
	SIP_S8bit *reqline, *badheader_name, *badheader_body;
	SIP_S8bit *p;
	int i;
	CallRealmInfo *realmInfo;
	char rsadomain[24];
	int rxedFromNat = 0;
	unsigned long realmId = 0;
	CacheRealmEntry *realmEntry = NULL;

	DEBUG(MSIP, NETLOG_DEBUG3, 
		  ("%s Received an incoming request: %s\n", fn,method));

	if (!context || !context->pData)
	{
		NETERROR (MSIP, ("%s: Null context/data %p!", fn, context));
		return ;
	}

	realmInfo = (CallRealmInfo *)context->pData;

	FormatIpAddress(realmInfo->rsa, rsadomain);

	realmId = realmInfo->realmId;
	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);
	realmEntry = CacheGet (realmCache, &realmId);
	if (realmEntry == NULL) 
	{
		NETERROR  (MSIP, ("%s: Failed to get realm entry for id %lu", fn, realmId));
		CacheReleaseLocks (realmCache);
		goto _error;
	}
	CacheReleaseLocks (realmCache);

	/* We are just going to forward this one, no new request
	 * should be originated
	 */

	if(SipCheckSourceAddressWithVia(s, context, 0, &rxedFromNat, &err) < 0)
	{
		if(err != -1 || strcasecmp(method, "ACK") == 0)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Error in SipCheckSourceAddressWithVia\n", fn));
		}
		else
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s DNS error\n", fn));
			if (SipFormatResponse(s, context, &resp, 503,
				  "Service Unavailable", &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 503\n", fn));
				goto _error;
			}

			/* host will be freed by thread */
			SipSendMsgToHost2(resp,context,
						SIPMSG_RESPONSE, (host), hostport);

			goto _return;
		}
	}

	if(sip_getBadHeaderCount(s, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error failed to get number of bad headers\n", fn));
		goto _error;
	}

	if(count > 0)
	{
		NETERROR(MSIP, ("%s Bad Message...\n", fn));

		if (sip_getReqLineAsString(s, &reqline, &err) != SipFail)
		{
			NETERROR(MSIP, ("\t%s\n", reqline));

			SipCheckFree(reqline);
		}
		else
		{
			NETERROR(MSIP, ("%s failed to get Request Line\n", fn));
		}

		for(i = 0; i < count; ++i)
		{
			if(sip_getBadHeaderAtIndex(s, &badheader, i, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s failed to get Bad Header at index %d\n", fn, i ));
				continue;
			}

			if(sip_getNameFromBadHdr(badheader, &badheader_name, &err) != SipFail &&
				sip_getBodyFromBadHdr(badheader, &badheader_body, &err) != SipFail)
			{
				NETERROR(MSIP, ("->\t%s: %s\n", badheader_name, badheader_body));
			}
			else
			{
				NETERROR(MSIP, ("%s failed to get Bad Header Body\n", fn));
			}

			sip_freeSipBadHeader(badheader);
		}

		if(strcmp(method, "ACK") != 0)
		{
			NETERROR(MSIP, ("Sending 400 Bad Message\n"));

			if (SipFormatResponse(s, context, &resp, 400,
						  "Bad Request", &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 400\n", fn));
				goto _error;
			}

			/* host will be freed by thread */
			SipSendMsgToHost2(resp,context,
				  		  SIPMSG_RESPONSE, (host), hostport);
		}
		else
		{
			goto _error;
		}

		goto _return;
	}

	/* Send a 400 if an Invite has no Contact header */
	if(strcmp(method, "INVITE") == 0)
	{
		if (sip_getHeaderCount(s, SipHdrTypeContactAny, &count, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Failed to get number of Contact headers\n", fn));
			goto _error;
		}

		if (count <= 0)
		{
			NETERROR(MSIP, ("%s Invite has no Contact header\n", fn));
			NETERROR(MSIP, ("%s Sending 400 Bad Message\n", fn));

			if (SipFormatResponse(s, context, &resp, 400,
						  "Bad Request", &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 400\n", fn));
				goto _error;
			}

			SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);
			goto _return;
		}
	}

	if((dHops=SipCheckMaxForwards(s))<0)
	{
		if(strcmp(method, "ACK") != 0)
		{
			NETERROR(MSIP, ("%s MaxForwards Err (%d)... Sending 483\n", fn, dHops));

			if (SipFormatResponse(s, context, &resp, 483,
						  "Too Many Hops", &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP, ("%s Error in generating 483\n", fn));
				goto _error;
			}

			/* host will be freed by thread */
			SipSendMsgToHost2(resp,context,
						SIPMSG_RESPONSE, (host), hostport);
		}
		else
		{
			goto _error;
		}

		goto _return;
	}

	if(sipauth && !obpEnabled && SipChallengeMethod(method, realmEntry->realm.authFlags) )
	{
		if(SipCheckAuthorization(s, method)<0)
		{
			NETDEBUG(MSIP,NETLOG_DEBUG4,
				 ("%s authorization failed\n",fn)); 
			if (SipFormatResponse(s, context, &resp, 407,
					      "Proxy Authentication Required",
					      &host, &hostport, &err) < 0)
			{
				NETERROR(MSIP,("%s Fail to format response 401.\n",fn));
				goto _error;
			}

			if(SipInsertAuthenticateHdr(resp, method)<0)
			{
				sip_freeSipMessage(resp);
				NETERROR(MSIP,("%s Fail to insert AuthenHdr\n",fn));
				goto _error;
			}

			/* host will be freed by thread */
			SipSendMsgToHost2(resp,context,
					    SIPMSG_RESPONSE, host, hostport);
			
			goto _return;
		}
		else
		{
			NETDEBUG(MSIP,NETLOG_DEBUG4,
				 ("%s authorization passed\n",fn));
			/* delete ProxyAuthorization Hdr */
			if(strcmp(method,"REGISTER"))
			{
				/* non-register msg, check Proxyauthorization */
				dType=SipHdrTypeProxyauthorization;
			}
			else
			{
				/* register msg, check Authorization */
				dType=SipHdrTypeAuthorization;
			}
			if( sip_deleteHeaderAtIndex(s, dType, 0, &err) == SipFail)
			{
				NETDEBUG(MSIP, NETLOG_DEBUG2, ("%s did not delete proxyauth hdr\n", fn));
			}

		} 
	}

	if( strcasecmp(method, "ACK") != 0)
	{
		/* hash via branch param */
		SipHashViaBranch(s,branchtoken,&err); 

		if (SipCheckLoopUsingBranch(s, branchtoken, &err)) 
		{
			NETERROR(MSIP, ("%s Detected a loop... Sending 482\n", fn));
			if (SipFormatResponse(s, context, &resp, 482,
					      "Loop Detected", &host, &hostport, &err) < 0)
			{
				goto _error;
			}
			/* host will be freed by thread */
			SipSendMsgToHost2(resp,context,
					    SIPMSG_RESPONSE, (host), hostport);
			goto _return;
		}
	}


	if ((sipservertype==SERVER_PROXYSTATEFULL) ||
		SipTransGetTransOrCallForIncomingMsg(s))
	{
		/* There is a transaction corresponding to this message.
		 * we must pass this message to the TSM.
		 */
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Message associated w/ a transaction already\n", fn));
		if( SipTransIncomingMsg(s, method, context) < 0)
		{
			goto _error;
		}
		
		goto _request_forwarded;
	}

	if (sipservertype != SERVER_PROXYSTATEFULL && recordroute)
	{
		/* Insert our own record-route header */
		SipInsertRecordRoute(s,&err);
	}

	if ((SipExtractReqUri( s, &req_uri, &err)) == SipFail)
	{
		 NETERROR(MSIP, 
				  ("%s ExtractFromAndToHeaders returned Error [%d]\n", 
					fn, err));
		 goto _error;
	}

	/* Set the URLs */
	sprintf(requri, "%s@%s",
			SVal(req_uri->name), SVal(req_uri->host));

	if ((sip_getHeaderCount(s, SipHdrTypeRoute, &count, &err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get 'route' header count from msg %d\n",err));
		goto _error;
    }

	if( (sipservertype != SERVER_PROXYSTATEFULL) && 
	    count != 0 && (SipMatchDomains(sipdomain, req_uri->host) == 0))
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
			 ("Route header exists and Request-URI matches our domain\n"));

		/* Insert our own via header, before proxy the message */
		SipInsertRealmVia(s, SIPPROTO_UDP, branchtoken, rsadomain, &err);

		/* Need to pop up next Route to Request-URI and send it there */
		if((SipPopRoute(s, &host, &hostport, &err))==SipFail)
		{
			NETERROR(MSIP, ("Could not set route to Request Line %d\n",err));
			goto _error;
		}

		NETDEBUG(MSIP, NETLOG_DEBUG3,
			 ("First entry in route header: Sending message to %s\n", host));

		SipSendMsgToHost2(s, context, SIPMSG_REQUEST, host, hostport);

		goto _request_forwarded;
	}

	/* Look up the cache */
	cacheInfo = &cacheInfoEntry;
	if (CacheFind(uriCache, requri, cacheInfo, sizeof(CacheTableInfo)) < 0)
	{ 
		/* Not found. If the domain is not one of our sip domains,
		 * we do a DNS lookup on it.
		 */
		NETDEBUG(MSIP, NETLOG_DEBUG3,
			("%s uri %s not in cache\n", fn, requri));

		if (SipMatchDomains(sipdomain, req_uri->host))
		{
			NETDEBUG(MSIP, NETLOG_DEBUG3,
				("%s domain %s does not match our domain %s\n",
				fn, req_uri->host, sipdomain));

			if(sipservertype==SERVER_REDIRECT)
			{
				if (SipFormatResponse(s, context, &resp, 302,
						      "Moved Temporarily", &host, &hostport, &err) < 0)
				{
					goto _error;
				}
				if(SipInsertContact(resp,req_uri,&err)<0)
				{
					goto _error;
				}
				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context,
						    SIPMSG_RESPONSE, (host), hostport);
				goto _return;
			}

			/* Insert our own via header, before proxy the message */
			SipInsertRealmVia(s, SIPPROTO_UDP, branchtoken, rsadomain, &err);

			host = strdup(req_uri->host);

			SipSendMsgToHost2(s, context, SIPMSG_REQUEST, host, 
				req_uri->port);

			goto _request_forwarded;
		}
		else
		{
			NETDEBUG(MSIP, NETLOG_DEBUG3,
				("%s domain %s is our domain\n", fn, req_uri->host));

			/* In our domain, then we should also try looking up
			 * the phone number itself. Here we will use the old resolve
			 * function. If the Resolve returns nothing, we will
			 * return a 404. If success is returned, then we will use the
			 * URI constructed by the final IP address and phone
			 * as the new URI. If the destination entry has a contact
			 * configured, we will use that instead.
			 */
			if (SipResolveCache(s, context, req_uri->name, cacheInfo) >= 0)
			{
				if (!BIT_TEST(cacheInfo->data.cap, CAP_SIP) ||
					(sipservertype==SERVER_PROXYSTATEFULL))
				{
					NETDEBUG(MSIP, NETLOG_DEBUG4,
						("%s Message destined for SIPUA\n", fn));

					if( SipTransIncomingMsgForUADest(s, method, context,
									 cacheInfo) < 0)
					{
						goto _error;
					}

					goto _request_forwarded;
				}

				/* Success. We have a cache entry. We can now use
				 * the contact info here to forward the request.
				 */
				if (SipReqUriFromIedgeEntry(&cacheInfo->data, 
						&newreq_uri) < 0)
				{
					goto _error;
				}

				if(sipservertype==SERVER_REDIRECT)
				{
					if (SipFormatResponse(s, context, &resp, 302,
					     "Moved Temporarily", &host, &hostport, &err) < 0)
					{
						goto _error;
					}
					if(SipInsertContact(resp,newreq_uri,&err)<0)
					{
						goto _error;
					}
					/* host will be freed by thread */
					SipSendMsgToHost2(resp,context,
					     SIPMSG_RESPONSE, (host), hostport);
					goto _return;
				}


				NETDEBUG(MSIP, NETLOG_DEBUG3,
					("%s setting new req uri to %s\n", fn, newreq_uri->host));

				if (SipSetReqUri(s, newreq_uri, method, &err) < 0)
				{
					NETERROR(MSIP, 
						("%s Unable to set the requri to %s\n",
						fn, cacheInfo->data.contact));
					SipCheckFree(newreq_uri->host);
					SipCheckFree(newreq_uri->name);
					goto _error;
				}

				/* Insert our own via header, before proxy the message */
				SipInsertRealmVia(s, SIPPROTO_UDP, branchtoken, rsadomain, &err);

				host = strdup(newreq_uri->host);
				SipSendMsgToHost2(s, context, SIPMSG_REQUEST, 
					host, newreq_uri->port);

				SipCheckFree(newreq_uri->host);
				SipCheckFree(newreq_uri->name);
				goto _request_forwarded;
			}
			else
			{
				/* No entry was found for the phone. We must send a 404
				   but if it is ACK, we silently discard it 
				 */
				if(strcmp(method,"ACK"))
				{
					NETDEBUG(MSIP, NETLOG_DEBUG4, ("Generating 404\n"));

					if (SipFormatResponse(s, context, &resp, 404,
						"Not Found", &host, &hostport, &err) < 0)
					{
						goto _error;
					}

					/* host will be freed by thread */
					SipSendMsgToHost2(resp,context,
						SIPMSG_RESPONSE, (host), hostport);

					goto _return;
				}
				else
				{
					/* It is an ack. If its worth to anybody, it would
					 * be to the UA 
					 */
					NETDEBUG(MSIP, NETLOG_DEBUG4,
						("%s ACK destined for SIPUA\n", fn));

					if( SipTransIncomingMsg(s, method, context) < 0)
					{
						goto _error;
					}

					goto _request_forwarded;
				}
			}
	
#ifdef _test_via
			/* default action.. send the request back to ourselves,
			 * we will drop it, on detecting a via loop. This way we can
			 * test our via headers. Note: Same can be used for route
			 * header testing.
			 */
			
			SipSendMsgToHost2(s, context, strdup(req_uri->host));
			goto _request_forwarded;
#endif

			goto _error;
		}
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
			("%s uri %s found in cache\n", fn, requri));

		/* Use the phone in the entry to traverse cache */
		if (SipResolveCache(s, context, 
				cacheInfo->data.phone, cacheInfo) < 0)
		{
			/* This is probably an error */
			NETDEBUG(MSIP, NETLOG_DEBUG3,
				("%s Could not locate entry in cache\n", fn));
			goto _error;
		}

		if (!BIT_TEST(cacheInfo->data.cap, CAP_SIP) ||
			(sipservertype==SERVER_PROXYSTATEFULL))
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Message destined for SIPUA\n", fn));

			if( SipTransIncomingMsgForUADest(s, method, context, cacheInfo) < 0)
			{
				goto _error;
			}
			
			goto _request_forwarded;
		}

		/* Success. We have a cache entry. It may be
		 * a new entry or the one which we already have.
		 */
		if (SipReqUriFromIedgeEntry(&cacheInfo->data, 
				&newreq_uri) < 0)
		{
			goto _error;
		}

		if(sipservertype==SERVER_REDIRECT)
		{
			if (SipFormatResponse(s, context, &resp, 302,
					      "Moved Temporarily", &host, &hostport, &err) < 0)
			{
				goto _error;
			}
			if(SipInsertContact(resp,newreq_uri,&err)<0)
			{
				goto _error;
			}
			/* host will be freed by thread */
			SipSendMsgToHost2(resp,context,
					    SIPMSG_RESPONSE, (host), hostport);
			goto _return;
		}

		NETDEBUG(MSIP, NETLOG_DEBUG3,
			("%s setting new req uri to %s\n", fn, newreq_uri->host));

		if (SipSetReqUri(s, newreq_uri, method, &err) < 0)
		{
			NETERROR(MSIP, 
				("%s Unable to set the requri to %s\n",
				fn, cacheInfo->data.contact));
			SipCheckFree(newreq_uri->host);
			SipCheckFree(newreq_uri->name);
			goto _error;
		}

		/* Insert our own via header, before proxy the message */
		SipInsertRealmVia(s, SIPPROTO_UDP, branchtoken, rsadomain, &err);

		host = strdup(newreq_uri->host);
		SipSendMsgToHost2(s, context, SIPMSG_REQUEST, host, 
			newreq_uri->port);

		SipCheckFree(newreq_uri->host);
		SipCheckFree(newreq_uri->name);
		goto _request_forwarded;
	}

 _request_forwarded:
	SipCheckFree(newreq_uri);
	SipCheckFree(req_uri);

	return;

 _error:
	SipFreeContext(context);
 _return:
	SipCheckFree(newreq_uri);
	SipCheckFree(req_uri);
	sip_freeSipMessage(s);

	return;
}

void 
sip_indicateInvite ( 
	SipMessage *s, 
	SipEventContext *context 
)
{
	extern int shutdown_inprogress;

	char fn[] = "sip_indicateInvite():";

	if(shutdown_inprogress)
	{
		sip_freeSipMessage(s);
		SipFreeContext(context);
		return;
	}
	
	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Request Received, Body part count = %d, %p, %p\n", fn, s->slMessageBody.size, s->slMessageBody.head, s->slMessageBody.tail));

	return SipIncomingRequest(s, "INVITE", context);
}

int
SipResolveCache(SipMessage *s, 
	SipEventContext *context,
	char *fphone, 
	CacheTableInfo *cacheInfo)
{
	char fn[] = "SipResolveCache():";
	PhoNode phonode = { 0 }, fphonode = { 0 }, 
		 aphonode = { 0 }, *phonodep, *rphonodep, *rfphonodep, *raphonodep;
	CacheTableInfo srcCacheInfoEntry = { 0 }, *srcCacheInfo, tempCacheInfo;
	ResolveHandle *rhandle = NULL;
	header_url *from_uri = NULL;
	char fromuri[SIPURL_LEN], *tmpstr;
	int herror, srcfound = 0;
	char *host = NULL;
	unsigned short port;
	SipError err;
	char s1[25];
	char fphone2[256], *fphoneptr=NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Starting Resolution in cache\n", fn));

	/* First step is to identify the source. This can be done
	 * on the basis of (1) From address:name,
	 * (2) From address:address,
	 * (3) Via address (implemented later... )
	 */
	
	if(fphone)
	{
		fphoneptr = &fphone2[0];
		strncpy(fphoneptr, fphone, 256);
		fphoneptr[255] = '\0';
		SipPhoneContextPlus(&fphoneptr);
		SipStripUriParams(fphoneptr);
		fphone = fphoneptr;
	}
	phonodep = &phonode;

	if (SipExtractFromUri(s, &from_uri, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error in extracting From\n", fn));
		goto _error;
	}

	/* Set the URLs */
	sprintf(fromuri, "%s@%s",
			SVal(from_uri->name), SVal(from_uri->host));

	srcCacheInfo = &srcCacheInfoEntry;

	/* Look up the uri cache */
	if (CacheFind(uriCache, fromuri, srcCacheInfo, sizeof(CacheTableInfo)) < 0)
	{ 
		/* We must try to look up a sub domain vpn which contains this
		 * sip uri within it.
		 */
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s No src uri %s in cache\n", fn, fromuri));

		/* Use the uri as a phone */
		if (strlen(SVal(from_uri->name)))
		{
			strncpy(phonodep->phone, from_uri->name, PHONE_NUM_LEN);
			BIT_SET(phonodep->sflags, ISSET_PHONE);
		}

		/* Need to find out the from ip entry in the cache */
		if (strlen(SVal(from_uri->host)))
		{
			if((phonodep->ipaddress.l = ResolveDNS(from_uri->host, &herror)) != -1)
			{
				BIT_SET(phonodep->sflags, ISSET_IPADDRESS);
			}
		}

		if (FillSourceCacheForCallerId(phonodep, "", "", "", "", srcCacheInfo) < 0)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Could not identify from in our database\n", fn));
		}
		else
		{
			srcfound = 1;
		}

		/* If this fails then we should try out the via ip address
		 */
		BIT_RESET(phonodep->sflags, ISSET_IPADDRESS);
		BIT_RESET(phonodep->sflags, ISSET_PHONE);

		if (SipGetSentByHost(s, context, 0, 1, &host, &port, &err) > 0)
		{
			if((phonodep->ipaddress.l = ResolveDNS(host, &herror)) != -1)
			{
				BIT_SET(phonodep->sflags, ISSET_IPADDRESS);
				if (FillSourceCacheForCallerId(phonodep, "", "", "", "", srcCacheInfo) < 0)
				{
					NETDEBUG(MSIP, NETLOG_DEBUG4,
						("%s Could not identify via in our database\n", fn));
				}
				else
				{
					srcfound = 1;
				}
			}
		}
		else
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				 ("%s No Via or Unable to decipher Via\n", fn));
		}
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Found src uri %s in cache\n", fn, fromuri));
		srcfound = 1;
	}

	if (!srcfound && !allowSrcAll)
	{
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Source: %s/%lu %s %s\n", fn, srcCacheInfo->data.regid,
		srcCacheInfo->data.uport, srcCacheInfo->data.phone,
		FormatIpAddress(srcCacheInfo->data.ipaddress.l, s1)));

	/* Proceed still to the next phase */
	memcpy(fphonode.phone, fphone, PHONE_NUM_LEN-1);
	BIT_SET(fphonode.sflags, ISSET_PHONE);

	rhandle 			= GisAllocRHandle();
	rhandle->phonodep 	= &phonode;
	rhandle->rfphonodep = &fphonode;
	rhandle->checkVpnGroup = 1;
	rhandle->checkZone = 1;
	rhandle->scpname = srcCacheInfo->data.cpname;

	/* Determine the vpn group of this entry */
	FindIedgeVpn(srcCacheInfo->data.vpnName, &rhandle->sVpn);

	/* Zones are valid only for LUS */
	nx_strlcpy(rhandle->sZone, srcCacheInfo->data.zone, ZONE_LEN);

	ResolvePhone(rhandle);

	switch (rhandle->result)
	{
	case CACHE_FOUND:
		memcpy(cacheInfo, RH_FOUND_CACHE(rhandle, 0), 
			sizeof(CacheTableInfo));
	  break;
	case CACHE_NOTFOUND:
		goto _error;
	  break;
	case CACHE_INPROG:
		goto _error;
	default:
	  break;
	}

	/* Free the remote handle here */
	GisFreeRHandle(rhandle);
	SipCheckFree(from_uri);
	return 1;
	
_error:
	/* Free the remote handle here */
	GisFreeRHandle(rhandle);
	SipCheckFree(from_uri);
	return -1;
}

/* The newreq_uri MUST be freed (and the fields)
 * by the CALLER. Only a copy of the entry should
 * be passed as we are doing strtok on it.
 */
int
SipReqUriFromIedgeEntry(
	InfoEntry *entry,
	header_url **newreq_uri_in)
{
	char fn[] = "SipReqUriFromIedgeEntry():";
	char requri[SIPURL_LEN], *tmpstr;
	header_url *newreq_uri;
	char *nameStr = NULL, *hostStr = NULL, *portStr = NULL,
		*contactStr = NULL;

	/* The contact may require a DNS query.
	 * We update the requri with the contact address.
	 */ 
	newreq_uri = *newreq_uri_in = 
		(header_url *) malloc (sizeof(header_url));
	memset(newreq_uri, 0, sizeof(header_url));

	newreq_uri->port = 0;

	if (strlen(entry->contact))
	{
		/* get rid of the sip: if there is one */
		if (strstr(entry->contact, "sip://"))
		{
			contactStr = entry->contact + strlen("sip://");
		}
		else
		{
			contactStr = entry->contact;
		}

		/* If the contact does not have the "@" address, we
		 * must derive the name. 
		 */
		nameStr = strtok_r(contactStr, "@", &hostStr);
		if (!nameStr)
		{
			NETERROR(MSIP, ("No name found in contact\n"));
			return -1;
		}

		if (!hostStr)
		{
			hostStr = nameStr;
			nameStr = NULL;
		}

		hostStr = strtok_r(hostStr, ":", &portStr);

		/* Now assign these to the requri */
		if (nameStr)
		{
			newreq_uri->name = strdup(nameStr);
		}
		else
		{
			/* We must use the ip address of the iedge entry here */
			newreq_uri->name = strdup(entry->phone);
		}

		newreq_uri->host = strdup(hostStr);
		if (portStr)
		{
			newreq_uri->port = atoi(portStr);
		}
	
	}
	else if (BIT_TEST(entry->sflags, ISSET_IPADDRESS))
	{
		/* We must use the ip address of the iedge entry here */
		newreq_uri->name = strdup(entry->phone);

		newreq_uri->host = (char *)malloc(24);
		FormatIpAddress(entry->ipaddress.l, newreq_uri->host);
	}

	return 1;
}

int
SipReqUriFromPhonode(
	PhoNode *phonode,
	char *contact,
	header_url **newreq_uri_in)
{
	char fn[] = "SipReqUriFromPhonode():";
	char requri[SIPURL_LEN], *tmpstr;
	header_url *newreq_uri;
	char *nameStr = NULL, *hostStr = NULL, *portStr = NULL,
		*contactStr = NULL;
	char *uri_params_in = NULL;

	/* The contact may require a DNS query.
	 * We update the requri with the contact address.
	 */ 
	newreq_uri = (header_url *) malloc (sizeof(header_url));
	memset(newreq_uri, 0, sizeof(header_url));

	newreq_uri->port = 0;
	newreq_uri->realmId = phonode->realmId;

	// Check if there are any parameters in the requri
	if(*newreq_uri_in && (*newreq_uri_in)->name)
	{
		strtok_r((*newreq_uri_in)->name, ";", &uri_params_in);
	}

	if (strlen(contact))
	{
		/* get rid of the sip: if there is one */
		if (strstr(contact, "sip://"))
		{
			contactStr = contact + strlen("sip://");
		}
		else
		{
			contactStr = contact;
		}

		/* If the contact does not have the "@" address, we
		 * must derive the name. 
		 */
		nameStr = strtok_r(contactStr, "@", &hostStr);
		if (!nameStr)
		{
			NETERROR(MSIP, ("No name found in contact\n"));
			return -1;
		}

		if (!hostStr)
		{
			hostStr = nameStr;
			nameStr = NULL;
		}

		hostStr = strtok_r(hostStr, ":", &portStr);

		/* Now assign these to the requri */
		if (nameStr)
		{
			if(uri_params_in)
			{
				newreq_uri->name = malloc(strlen(nameStr) + strlen(uri_params_in) + 2);
				sprintf(newreq_uri->name, "%s;%s", nameStr, uri_params_in);
			}
			else
			{
				newreq_uri->name = strdup(nameStr);
			}
		}
		else
		{
			if(uri_params_in)
			{
				newreq_uri->name = malloc(strlen(phonode->phone) + strlen(uri_params_in) + 2);
				sprintf(newreq_uri->name, "%s;%s", phonode->phone, uri_params_in);
			}
			else
			{
				newreq_uri->name = strdup(phonode->phone);
			}
		}

		newreq_uri->host = strdup(hostStr);
		if (portStr)
		{
			newreq_uri->port = atoi(portStr);
		}
			
	}
	else if (BIT_TEST(phonode->sflags, ISSET_IPADDRESS))
	{
		if(uri_params_in)
		{
			newreq_uri->name = malloc(strlen(phonode->phone) + strlen(uri_params_in) + 2);
			sprintf(newreq_uri->name, "%s;%s", phonode->phone, uri_params_in);
		}
		else
		{
			newreq_uri->name = strdup(phonode->phone);
		}

		newreq_uri->host = (char *)malloc(24);
		FormatIpAddress(phonode->ipaddress.l, newreq_uri->host);
	}

	UrlFree(*newreq_uri_in, MEM_LOCAL);
	*newreq_uri_in = newreq_uri;

	return 1;
}

void 
SipIncomingResponse(SipMessage *s, SipEventContext *context) 
{
	char fn[] = "SipIncomingResponse():";
//	SipStatusLine *status_line ;
	SIP_S8bit* status_line;
	SIP_U16bit code_num ;
	SIP_S8bit *callId = NULL;
	header_url *to = NULL, *from = NULL, *contact = NULL;
	char url[SIPURL_LEN] = { 0 };
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry = {0};
	SipHeader *header = NULL, *new_header = NULL;
	SipAddrSpec *new_contact_addrspec = NULL;
	SipContactParam *contactp = NULL;
	SIP_U32bit count,k;
	SipError err;
	SIP_U32bit cseq;
	SIP_U32bit status_code;
	char *method = NULL;
	char *host = NULL;
	unsigned short port = 0;
	char *contacttmpstr = NULL;
	int rcSentBy = 0;
	CallRealmInfo *drealmInfo;
	SipBadHeader *badheader;
	SIP_S8bit *badheader_name, *badheader_body;
	int i;
	char s1[16];
	int forUA = 0;
	CacheRealmEntry* realmEntryPtr = NULL; 
	unsigned long    mirrorproxy   = 0;
        CacheTableInfo*  info          = NULL;
	SipTransKey siptrankey;
	int notify_tran_exists = FALSE;
	SipTrans* siptranptr=NULL;
	char contacthostname[255];

	DEBUG(MSIP, NETLOG_DEBUG3, ("%s Received a response\n", fn));

	if (sipservertype != SERVER_PROXYSTATEFULL)
	{
		/* Check if we are at the top of the Via Header */
		if (SipCheckTopVia(s, &err, context) == 0)
		{
			/* We are not at the top of the Via Lost ...*/
			NETERROR(MSIP, ("%s We are not at the top of the Via list\n",
				fn));
			goto _error;
		} 
	}
	
	/* Remove ourselves from the Via header */
	if (sip_deleteHeaderAtIndex(s, SipHdrTypeVia, 0, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Could not delete our via header\n", fn));
		goto _error;
	}

	if(sip_getBadHeaderCount(s, &count, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s Error failed to get number of bad headers\n", fn));
		goto _error;
	}

	if(count > 0)
	{
		NETERROR(MSIP, ("%s Received Bad Message...\n", fn));

		if (sip_getStatusLineAsString(s, &status_line, &err) != SipFail)
		{
			NETERROR(MSIP, ("\t%s\n", status_line));

			SipCheckFree(status_line);
		}
		else
		{
			NETERROR(MSIP, ("%s failed to get Status Line\n", fn));
		}

		for(i = 0; i < count; ++i)
		{
			if(sip_getBadHeaderAtIndex(s, &badheader, i, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s failed to get Bad Header at index %d\n", fn, i ));
				continue;
			}

			if(sip_getNameFromBadHdr(badheader, &badheader_name, &err) != SipFail &&
				sip_getBodyFromBadHdr(badheader, &badheader_body, &err) != SipFail)
			{
				NETERROR(MSIP, ("->\t%s: %s\n", badheader_name, badheader_body));
			}
			else
			{
				NETERROR(MSIP, ("%s failed to get Bad Header Body\n", fn));
			}

			sip_freeSipBadHeader(badheader);
		}

		goto _error;
	}

	if (sipservertype==SERVER_PROXYSTATEFULL)
	{
		/* Extract the Cseq */
		if (SipGetCSeq(s, &cseq, &method, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Could not extract cseq\n", fn));
			goto _error;
		}

		
		if( strcasecmp(method,"NOTIFY") == 0 )
		{
			if( SipTranKeyFromIncomingMsg(s, &siptrankey, sizeof(siptrankey)) < 0)
			{
				NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s failed to get transaction for incoming message.\n", fn));
				
				/* error */
				goto _error;
			}

			CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);
			if( (siptranptr = CacheGet(transCache, &siptrankey)) != NULL  )
			{
				notify_tran_exists = TRUE;
			}
			CacheReleaseLocks(transCache);
			SipFreeTranKeyMember(&siptrankey);
		}

		if ( (strcasecmp(method, "REGISTER") != 0  && strcasecmp(method, "NOTIFY") != 0) || notify_tran_exists )
			//if ( strcasecmp(method, "REGISTER") != 0 )
		{
			SipCheckFree(method);

			/* This message may be meant for the UA itself */
			SipTransIncomingMsg(s, "RESPONSE", context);

			return;
		}
		else
		{
			SipCheckFree(method);

			if (SipExtractToUri(s, &to, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s Error in extracting From\n", fn));
				goto _error;
			}

			/* The To URI must be in our cache */	
			if (to->name)
			{
				sprintf(url, "%s@%s", to->name, to->host);
			}
			else
			{
				sprintf(url, "%s", to->host);
			}

			SipCheckFree(to);
			
			cacheInfo = &cacheInfoEntry;
			if( CacheFind(uriCache, url, cacheInfo, sizeof(CacheTableInfo)) > 0)
			{
				SipGetStatusCode(s, &status_code);

				if (cacheInfo->data.realmId > 0)
				{
					drealmInfo = getRealmInfo(cacheInfo->data.realmId, MEM_LOCAL);

					if(drealmInfo == NULL) 
					{
						NETERROR(MSIP, ("%s src realm unknown for %s\n",
							fn, url));
						goto _error;
					}
				}
				else
				{
					NETERROR(MSIP, ("%s src realm unknown for %s\n",
						fn, url));
				}	

				if (context->pData)
				{
					RealmInfoFree(context->pData, MEM_LOCAL);
				}

				context->pData = (SIP_Pvoid)drealmInfo;
                                
				CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);
				CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
				realmEntryPtr = CacheGet(realmCache, &drealmInfo->realmId);
				if (realmEntryPtr)
				{
					// Look up the entry in the cache
					info = CacheGet(regCache, &(realmEntryPtr->realm.mp));
					if(info)
					{
						mirrorproxy = info->data.ipaddress.l;
					}                        
				}
				CacheReleaseLocks(realmCache);
				CacheReleaseLocks(regCache);					

                                // For all REGISTER 200 OKs going back on this realm replace TO and FROM to realm rsa
                                if( mirrorproxy != 0 )
                                {
					//  From and To header in a REGISTER need not be same always.
					//  Replace rsa only when from and to header are same.
					if(SipExtractFromUri(s, &from, &err) == SipFail)
					{
						NETERROR(MSIP,("%s Unable to extract from header", fn));
						goto _error;
					}
					if(SipExtractToUri(s, &to, &err) == SipFail)
					{
						NETERROR(MSIP,("%s Unable to extract to header", fn));
						goto _error;
					}
					if( strcmp(to->host,from->host)==0 )
					{
						SipReplaceToOrFrom(s,(unsigned long)drealmInfo->rsa,SipHdrTypeFrom);
					}
                                        SipReplaceToOrFrom(s,(unsigned long)drealmInfo->rsa,SipHdrTypeTo);

					SipCheckFree(to);
					SipCheckFree(from);
                                }            
                        
				/* Get the contact header count */
				if (sip_getHeaderCount(s, SipHdrTypeContactAny, &count, &err) == SipFail)
				{
					NETERROR(MSIP, ("%s could not get count of contact header\n", fn));
					goto _del_entry;
				}

				if( BIT_TEST(cacheInfo->data.sflags, ISSET_NATIP) && 
					BIT_TEST(cacheInfo->data.sflags, ISSET_NATPORT) )
				{
					FormatIpAddress(cacheInfo->data.natIp, s1);
					host = strdup(s1);
					port = cacheInfo->data.natPort;
				}				
				if(host == NULL && (cacheInfo->data.stateFlags & CL_DYNAMIC))
				{
					ExtractContactHost(cacheInfo->data.contact,contacthostname, &port);
					host = strdup(contacthostname);
				}

				if( (cacheInfo->data.stateFlags & CL_UAREG) )
				{
					forUA = 1;
				}

				if(!(cacheInfo->data.stateFlags & CL_UAREG) && count > 0 &&
					IsFCEEnabled() && status_code == 200)
				{
					CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);

					cacheInfo->data.stateFlags |= CL_REGISTERED;
					time(&cacheInfo->data.rTime);

					DbScheduleIedgeUpdate(&cacheInfo->data);

					CacheReleaseLocks(regCache);

					contacttmpstr = malloc(strlen(cacheInfo->data.contact)+ 4 + 1);
					strcpy(contacttmpstr, "sip:");
					strcat(contacttmpstr, cacheInfo->data.contact);

					if((sip_setAddrSpecFromString(&new_contact_addrspec, contacttmpstr, &err)) == SipFail)
					{
						NETERROR(MSIP, ("%s could not initialize contact header\n",fn));
						free(contacttmpstr);
						goto _error;	
					}
					free(contacttmpstr);

					if ((sip_initSipHeader(&new_header, SipHdrTypeContactNormal, &err))==SipFail)
					{
						NETERROR(MSIP, ("%s could not initialize sip header\n",fn));
						goto _error;	
					}
					
					if((sip_setAddrSpecInContactHdr(new_header, new_contact_addrspec, &err))==SipFail)
					{
						NETERROR(MSIP, ("%s could not initialize sip header\n",fn));
						goto _error;	
					}

					/* Extract the contact header */
					if ((sip_initSipHeader(&header, SipHdrTypeAny, &err))==SipFail)
					{
						NETERROR(MSIP, ("%s could not initialize sip header\n",
							fn));
						goto _error;
					}

					if (sip_getHeaderAtIndex(s, SipHdrTypeContactAny, header, (SIP_U32bit)0,
							&err) == SipFail)
					{
						NETERROR(MSIP, ("%s could not get any contact header\n", fn));
						goto _del_entry;
					}

					if(sip_getContactParamsCountFromContactHdr(header,&count,&err) == SipFail)
					{
						NETERROR(MSIP,("%s error in getContactParamsCount header %d\n",fn,err));
						goto _error;
					}

					for(k=0;k<count;k++)
					{
						if (sip_getContactParamAtIndexFromContactHdr(header,
												 &contactp, k, &err) == SipFail)
						{
							NETERROR(MSIP,("%s error get %d-th ContactParam\n", fn,k));
							goto _error;
						}

						if (sip_insertContactParamAtIndexInContactHdr(new_header,
												 contactp, k, &err) == SipFail)
						{
							NETERROR(MSIP,("%s error set %d-th ContactParam\n", fn,k));
							goto _error;
						}
					}

					/* Remove current contact header */
					if (sip_deleteHeaderAtIndex(s, SipHdrTypeContactAny, 0, &err) == SipFail)
					{
						NETERROR(MSIP, ("%s Could not delete contact header\n", fn));
						goto _error;
					}

					/* Insert new contact header */
					if((sip_insertHeaderAtIndex(s, new_header, 0, &err)==SipFail))
					{
						 NETERROR(MSIP, ("%s insert new contact failed\n", fn));

						 goto _error;
					}
				}
				else
				{
_del_entry:
					// delete the endpoint only if it's a dynamic endpoint and the response is
					// not 200 OK or an unregister
					if( (cacheInfo->data.stateFlags & CL_DYNAMIC) && (status_code != 200 || count == 0) )
					{
						CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);
						cacheInfo = DeleteIedge(&cacheInfo->data);
						DbScheduleIedgeDelete(&cacheInfo->data);
						CFree(regCache)(cacheInfo);
						CacheReleaseLocks(regCache);
					}
				}
			}
		}
	}

	if(forUA)
	{
		/* This message may be meant for the UA itself */
		SipTransIncomingMsg(s, "RESPONSE", context);

		return;
	}

	/* We are just going to forward this one, no new request
	 * should be originated
	 */
	if(host == NULL)
	{
		rcSentBy = SipGetSentByHost(s, context, 0, 1, &host, &port, &err);

		if (rcSentBy < 0)
		{
			NETERROR(MSIP, 
				("%s SipGetSentByHost returned error\n", fn));
			goto _error;
		}
		else if (rcSentBy == 0)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s SipGetSentByHost returned no via header\n", fn));
			goto _error;
		}
	}

	/* Now forward the response to the new host */
	/* host will be freed by the thread */
	SipSendMsgToHost2(s, context, SIPMSG_RESPONSE, host, port);
	return;
	
_error:
	SipCheckFree(to);
	SipCheckFree(host);
	SipFreeContext(context);
	sip_freeSipMessage(s);
	SipCheckFree(from);

	return;
}

int 
SipHandleIncomingNotifyMessage( SipMessage* s, char* method, SipEventContext* context ) 
{
	char*          		fn         		= "SipHandleIncomingNotifyMessage():";
	CallRealmInfo* 		realmInfo  		= NULL;
	CallRealmInfo* 		drealmInfo 		= NULL;
	int            		rxedFromNat      	= 0;
	SipError       		err;		 	
	int            		dHops            	= 0;
	SipMessage*    		resp             	= NULL;
	SIP_S8bit*     		host             	= NULL;
	SIP_U16bit     		hostport;	 	
	SIP_U32bit     		count;	 	
	char           		branchtoken[64]; 	
	header_url*    		to               	= NULL;
	header_url*    		from             	= NULL;
	header_url*    		req_uri          	= NULL;
	header_url*    		contact          	= NULL;
	char           		rsadomain[24];	 	
	char           		destrsadomain[24]	;
	unsigned long  		mirrorproxy      	= 0;
	SipEventContext*        dcontext         	= NULL;
	CacheTableInfo*         pxCacheInfo      	= NULL;
	CacheTableInfo*         srcCacheInfo     	= NULL; 
	CacheTableInfo	        srcCacheInfoEntry	;
        CacheTableInfo*         info             	= NULL;
	CacheRealmEntry*        realmEntryPtr    	= NULL;
	char                    url[SIPURL_LEN]         = { 0 };
	int                     drealmId         	= -1;
	int                     port                    = 0;
	char                    temp[24];

	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Entering",fn));

	if( !context || !context->pData) 
	{
		NETERROR(MSIP,("%s no context is present %p", fn, context));
		return -1;
	}

	realmInfo = (CallRealmInfo *)context->pData;

	if( SipCheckSourceAddressWithVia(s, context, 0, &rxedFromNat, &err) < 0)
	{
		NETERROR(MSIP, ("%s Error in SipCheckSourceAddressWithVia\n", fn));
		goto _error;
	}

	if( (dHops = SipCheckMaxForwards(s)) < 0)
	{
		NETERROR(MSIP, ("%s MaxForwards Err (%d)... Sending 483\n", fn, dHops));

		if (SipFormatResponse(s, context, &resp, 483,"Too Many Hops", &host, &hostport, &err) < 0)
		{
			NETERROR(MSIP, ("%s Error in generating 483\n", fn));
			goto _error;
		}

		/* host will be freed by thread */
		SipSendMsgToHost2(resp,context,SIPMSG_RESPONSE, (host), hostport);
		context = NULL;
		goto _return;
	}

	/* hash via branch param */
	SipHashViaBranch(s,branchtoken,&err); 

	/* Extract the To URI, this must be of he form
	 * phone@ourdomain
	 */
	if (SipExtractReqUri(s, &req_uri, &err) == SipFail)
	{
		 NETERROR(MSIP,("%s ExtractReqUriHeaders failed\n", fn));
		 goto _error;
	}
                

	if (SipExtractFromUri(s, &from, &err) == SipFail)
	{
		 NETERROR(MSIP,("%s ExtractFromHeaders failed\n", fn));
		 goto _error;
	}

	if (SipExtractToUri(s, &to, &err) == SipFail)
	{
		 NETERROR(MSIP,("%s ExtractToHeaders failed\n", fn));
		 goto _error;
	}


	if (SipExtractContact(s, &contact, &err) == SipFail)
	{
		 NETERROR(MSIP,("%s ExtractContactHeaders failed\n", fn));
		 goto _error;
	}

	DEBUG(MSIP, NETLOG_DEBUG1,("%s From: [%s@%s]\n", fn, SVal(from->name),SVal(from->host)));
	DEBUG(MSIP, NETLOG_DEBUG1,("%s To: [%s@%s]\n", fn,SVal(to->name), SVal(to->host)));
	DEBUG(MSIP, NETLOG_DEBUG1,("%s Request-URI: [%s@%s]\n",fn, SVal(req_uri->name), SVal(req_uri->host)));

	FormatIpAddress(realmInfo->rsa, rsadomain);

	CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
	realmEntryPtr = CacheGet(realmCache, &realmInfo->realmId);				

	if (realmEntryPtr)
	{
		CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);
					
		// Look up the entry in the cache
		info = CacheGet(regCache, &(realmEntryPtr->realm.mp));
		if(info)
		{
			mirrorproxy = info->data.ipaddress.l;
			NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Mirrorproxy found 0x%lx ", fn, mirrorproxy));
		}                        
		CacheReleaseLocks(regCache);					
	}
	CacheReleaseLocks(realmCache);

        // It matches our domain but mirror proxy is configured for this realm
        if(!SipMatchDomains(rsadomain,req_uri->host) && (mirrorproxy != 0 ) )        {                
                // mirror proxy scenario
                if(SipReplaceReqUri(s,mirrorproxy, 0, "NOTIFY" ) == SipSuccess)
                {
                        SipCheckFree(req_uri);
                        req_uri = NULL; 
                        // Get modified req_uri for further checks
                        if (SipExtractReqUri(s, &req_uri, &err) == SipFail)
                        {
                                NETERROR(MSIP,("%s ExtractReqUriHeaders failed\n", fn));                                        
                                goto _error;
                        }
                }          
		// From and To headers in REGISTER need not always be the same
		// Replace the from header only when to and from headers are same.          2      
		// From and To has already been extracted
		if( strcmp(from->host,to->host) == 0 )
		{
			SipReplaceToOrFrom(s,mirrorproxy,SipHdrTypeFrom);
		}
                if(SipReplaceToOrFrom(s,mirrorproxy,SipHdrTypeTo) == SipSuccess) 
                {
                        SipCheckFree(to);
                        to = NULL;
                        if (SipExtractToUri(s, &to, &err) == SipFail)
                        {
                                NETERROR(MSIP,("%s ExtractToHeaders failed\n", fn));                                
                                goto _error;
                        }                        
                }
        }

	dcontext = (SipEventContext *) malloc (sizeof(SipEventContext));
	if (dcontext == NULL)
	{
		NETERROR(MSIP, ("%s Malloc for SipEventContext failed!\n", fn));
		goto _error;
	}
	
	memset(dcontext, 0, sizeof(SipEventContext));

	if( SipMatchDomains(rsadomain, req_uri->host) )
	{		
		// Check to see if the destination sip domain is defined in our database
		CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);
		
		sprintf(url, "%s", req_uri->host);
		if (pxCacheInfo = CacheGet(uriCache, url))
		{
			drealmId = pxCacheInfo->data.realmId;
		}
		CacheReleaseLocks(regCache);

		if (drealmId < 0)
		{
			NETERROR(MSIP, ("%s Destination proxy realm unknown for %s\n",
					fn, req_uri->host));
			goto _error;
		}
		drealmInfo = getRealmInfo(drealmId, MEM_LOCAL);

		if(drealmInfo == NULL) 
		{
			NETERROR(MSIP, ("%s Destination proxy realm unknown for %s\n",
					fn, req_uri->host));
			goto _error;
		}

		dcontext->pData = (SIP_Pvoid)drealmInfo;
		FormatIpAddress(drealmInfo->rsa, destrsadomain);
		
		SipInsertRealmVia(s, SIPPROTO_UDP, branchtoken, destrsadomain, &err);
		/* Insert a Max Forwards header if necessary */
		if ((sip_getHeaderCount(s, SipHdrTypeMaxforwards, &count, &err)) == SipFail)
		{
			NETERROR(MSIP, ("%s Unable to get Max-Forward hdr count %d\n", fn, err));
			goto _error;
		}
		
		if (count == 0)
		{
			SipSetMaxForwards(s, sipmaxforwards);
		}		

		if ( (pxCacheInfo->data.ecaps1 & ECAPS1_NATDETECT) && 
		     BIT_TEST(pxCacheInfo->data.sflags, ISSET_NATIP) )
		{
			FormatIpAddress(pxCacheInfo->data.natIp,temp);
			host = strdup(temp);
			port = pxCacheInfo->data.natPort;
		}
		else
		{
			host = strdup(req_uri->host);			
			port = req_uri->port;
		}
		SipSendMsgToHost2(s, dcontext, SIPMSG_REQUEST, host, port);
		dcontext = NULL;
		goto _return;
	}		

	/* The To URI must be in our cache */	
	if (to->name)
	{
		sprintf(url, "%s@%s", to->name, drealmInfo?destrsadomain:to->host);
	}
	else 
	{
		sprintf(url, "%s", drealmInfo?destrsadomain:to->host);
	}

	// Check the To-URI in the REGISTER in our database
	srcCacheInfo = &srcCacheInfoEntry;
	if (CacheFind(uriCache, url, srcCacheInfo, sizeof(CacheTableInfo)) < 0)
	{ 
		if (CacheFind(phoneCache, SVal(to->name), srcCacheInfo,sizeof(CacheTableInfo)) >= 0)
		{
			int port;
			char contacthostname[255];
			NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Found user in our phone cache as %s\n",
						      fn, SVal(to->name)));
			// FWD Notify
			drealmId = srcCacheInfo->data.realmId;			
			drealmInfo = getRealmInfo(drealmId, MEM_LOCAL);		
			if(drealmInfo == NULL) 
			{
				NETERROR(MSIP, ("%s Destination proxy realm unknown for %s\n",fn, req_uri->host));
				goto _error;
			}		
			dcontext->pData = (SIP_Pvoid)drealmInfo;
			FormatIpAddress(drealmInfo->rsa, destrsadomain);
			
			ExtractContactHost( srcCacheInfo->data.contact,contacthostname,&port);		
			if( SipReplaceReqUri(s,htonl(inet_addr(contacthostname)),port, "NOTIFY" ) == SipSuccess)
			{
				SipCheckFree(req_uri);
				req_uri = NULL; 
				// Get modified req_uri for further checks
				if (SipExtractReqUri(s, &req_uri, &err) == SipFail)
				{
					NETERROR(MSIP,("%s ExtractReqUriHeaders failed\n", fn));
					goto _error;
				}
				dcontext->pTranspAddr = (SipTranspAddr *) malloc (sizeof (SipTranspAddr));
				if (dcontext->pTranspAddr == NULL)
				{
					NETERROR (MSIP, ("%s malloc failure", fn));
					goto _error;
				}
				memset(dcontext->pTranspAddr, 0, sizeof (SipTranspAddr));
				FormatIpAddress(htonl(inet_addr(contacthostname)), context->pTranspAddr->dIpv4);
				context->pTranspAddr->dPort = port;							

				SipInsertRealmVia(s, SIPPROTO_UDP, branchtoken, destrsadomain, &err);

				SipReplaceToOrFrom(s,drealmInfo->rsa,SipHdrTypeFrom);
				SipReplaceToOrFrom(s,drealmInfo->rsa,SipHdrTypeTo);

				if ( (srcCacheInfo->data.ecaps1 & ECAPS1_NATDETECT) && 
				     BIT_TEST(srcCacheInfo->data.sflags, ISSET_NATIP) )
				{
					FormatIpAddress(srcCacheInfo->data.natIp,temp);
					host = strdup(temp);
					port = srcCacheInfo->data.natPort;
				}
				else
				{
					host = strdup(req_uri->host);			
					port = req_uri->port;
				}
				SipSendMsgToHost2(s, dcontext, SIPMSG_REQUEST, host, port);
				dcontext = NULL;
			}
			goto _return;                                        
		}
		else
		{
			/* The request URI domain seems to be our domain!!
			 * Must send Error back.
			 */
			NETDEBUG(MSIP, NETLOG_DEBUG4,("%s: Req & to match our domain (%s),but entry not found\n",
						      fn, req_uri->host));
			NETDEBUG(MSIP, NETLOG_DEBUG4,("Generating 404 response\n"));				
			if (SipFormatResponse(s, context, &resp, 404,"Not Found", &host, &hostport, &err) < 0)
			{
				goto _error;
			}
			
			/* host will be freed by thread */
			SipSendMsgToHost2(resp, context,SIPMSG_RESPONSE, (host), hostport);
			context = NULL;
			goto _return;
		}
	}
	
	else
	{
		// To URI exists in our database		
		NETDEBUG(MSIP, NETLOG_DEBUG3,("%s uri %s found in cache\n", fn, url));

		// FWD NOTIFY here		
		drealmId = srcCacheInfo->data.realmId;

		drealmInfo = getRealmInfo(drealmId, MEM_LOCAL);		
		if(drealmInfo == NULL) 
		{
			NETERROR(MSIP, ("%s Destination proxy realm unknown for %s\n",fn, req_uri->host));
			goto _error;
		}		
		dcontext->pData = (SIP_Pvoid)drealmInfo;
		FormatIpAddress(drealmInfo->rsa, destrsadomain);

		if ( (srcCacheInfo->data.ecaps1 & ECAPS1_NATDETECT) && 
		     BIT_TEST(srcCacheInfo->data.sflags, ISSET_NATIP) )
		{
			FormatIpAddress(srcCacheInfo->data.natIp,temp);
			host = strdup(temp);
			port = srcCacheInfo->data.natPort;
		}
		else
		{
			host = strdup(req_uri->host);			
			port = req_uri->port;
		}
		SipSendMsgToHost2(s, dcontext, SIPMSG_REQUEST, host, port);
		dcontext = NULL;
		goto _return;
	}

 _return:

	SipCheckFree(to);
	SipCheckFree(from);
	SipCheckFree(req_uri);
	SipCheckFree(contact); 
	SipFreeContext(dcontext); 
	return 1;
 _error:
	SipCheckFree(to);
	SipCheckFree(from);
	SipCheckFree(req_uri);
	SipCheckFree(contact);
	SipFreeContext(dcontext);
	return -1;

}

