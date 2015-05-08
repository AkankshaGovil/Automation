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
#include "siputils.h"
#include "firewallcontrol.h"
#include "dbs.h"
#include "log.h"

#define MMIN(a,b) ((a)<(b))?(a):(b)

/* As of Rel 1.3, we will support only one
 * contact address in the registration request.
 * Additive registrations may thus cause replacement of
 * the contact address. We will fill the action value
 * as "proxy" always. The callid in the registration
 * is not checked (thus no additions). The expires param
 * is used to do the timeouts. However there is no way
 * we do unregistrations when the timeout expires (The
 * CL_STATIC flag will be helful, if you want to make
 * all registrations static. An expires=0 will do the 
 * unregistration. The From header is not evaluated,
 * so third party registrations of the device are ok.
 * If the req-uri is not our own, we will forward
 * the registration. We will look for the To URL
 * in our cache. If its not found, the req-uri is
 * used to forward the registration.
 */
void 
sip_indicateRegister(
	SipMessage *s, 
	SipEventContext *context
)
{
	char fn[] = "sip_indicateRegister():";
	header_url *to = NULL, *from = NULL, *req_uri = NULL;
	header_url *contact = NULL, *new_contact = NULL;
	char *phone, *domain;
	CacheTableInfo *pxCacheInfo, *srcCacheInfo, *newSrcCacheInfo, srcCacheInfoEntry;
	SipError err;
	SIP_U32bit cseq, count;
	int oldaction;
	char url[SIPURL_LEN] = { 0 };
	int rc = -1, reason = 400;
	char branchtoken[64];
	SipMessage *resp = NULL;
	SIP_S8bit *host=NULL;
	SIP_U16bit hostport;
	struct timeval tv;
	SipHeader *header = NULL;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	char *contactname = NULL, *contacthost = NULL;
	SIP_U16bit contactport;
	int herror;
	int dHops;
	int i;
	int  have_locks = 0;
	CacheTableInfo *tmpCacheInfo;
    CallRealmInfo *realmInfo, *drealmInfo;
	CacheRealmEntry	*realmEntryPtr =NULL;
	char rsadomain[24], destrsadomain[24];
	SipEventContext *dcontext = NULL;
	int drealmId = -1;
	int rxedFromNat = 0;

	unsigned long    mirrorproxy   = 0;
        CacheTableInfo*  info          = NULL;
	char* viahost = NULL;
	int   wild_card_flag = FALSE;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (!context || !context->pData)
	{
		NETERROR(MSIP, ("%s no context is present %p\n", fn, context));
		return;
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

		if (SipFormatResponse(s, context, &resp, 483,
				      "Too Many Hops", &host, &hostport, &err) < 0)
		{
			NETERROR(MSIP, ("%s Error in generating 483\n", fn));
			goto _error;
		}

		/* host will be freed by thread */
		SipSendMsgToHost2(resp,context,
				    SIPMSG_RESPONSE, (host), hostport);
		context = NULL;

		goto _return;
	}

	/* hash via branch param */
	SipHashViaBranch(s,branchtoken,&err); 

	#if 0
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
		context = NULL;
		goto _return;
	}
	#endif

	/* Extract the To URI, this must be of he form
	 * phone@ourdomain
	 */
	if (SipExtractReqUri(s, &req_uri, &err) == SipFail)
	{
		 NETERROR(MSIP,
				  ("%s ExtractReqUriHeaders failed\n", fn));

		 goto _error;
	}
                

	if (SipExtractFromUri(s, &from, &err) == SipFail)
	{
		 NETERROR(MSIP,
				  ("%s ExtractFromHeaders failed\n", fn));

		 goto _error;
	}

	if (SipExtractToUri(s, &to, &err) == SipFail)
	{
		 NETERROR(MSIP,
				  ("%s ExtractToHeaders failed\n", fn));

		 goto _error;
	}


	if(SipIsContactWildCard(s,&err) == SipFail)
	{
		if (SipExtractContact(s, &contact, &err) == SipFail)
		{
			NETERROR(MSIP,("%s ExtractContactHeaders failed\n", fn));
			
			goto _error;
		}		
	}
	else {

		wild_card_flag = TRUE;
	}

	DEBUG(MSIP, NETLOG_DEBUG1, 
		  ("%s From: [%s@%s]\n", fn, SVal(from->name),
			SVal(from->host)));

	DEBUG(MSIP, NETLOG_DEBUG1, 
		  ("%s To: [%s@%s]\n", fn, 
			SVal(to->name), SVal(to->host)));

	DEBUG(MSIP, NETLOG_DEBUG1, 
		  ("%s Request-URI: [%s@%s]\n", 
			fn, SVal(req_uri->name), SVal(req_uri->host)));

	FormatIpAddress(realmInfo->rsa, rsadomain);

	
	CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);					
	CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
	realmEntryPtr = CacheGet(realmCache, &realmInfo->realmId);				
	if (realmEntryPtr)
	{
		// Look up the entry in the cache
		info = CacheGet(regCache, &(realmEntryPtr->realm.mp));
		if(info)
		{
			mirrorproxy = info->data.ipaddress.l;
			NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Mirrorproxy found 0x%lx ", fn, mirrorproxy));
		}                        
	}
	CacheReleaseLocks(realmCache);
	CacheReleaseLocks(regCache);					
	
        // It matches our domain but mirror proxy is configured for this realm
        if(!SipMatchDomains(rsadomain,req_uri->host) && (mirrorproxy != 0 ) )
        {                
                // mirror proxy scenario
                if(SipReplaceReqUri(s,mirrorproxy,0,"REGISTER") == SipSuccess)
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
		// Replace the from header only when to and from headers are same.                
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

	// If the request URI is not ours, then we are potentially sending this
	// request out, unless an error happens later
	if (SipMatchDomains(rsadomain, req_uri->host) )
	{
		// In case everything goes fine, we will end up forwarding this
		// request out. In that case, destination context needs to be
		// constructed

		dcontext = (SipEventContext *) malloc (sizeof(SipEventContext));
		if (dcontext == NULL)
		{
			NETERROR(MSIP, ("%s Malloc for SipEventContext failed!\n", fn));
			goto _error;
		}

		memset(dcontext, 0, sizeof(SipEventContext));

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
	} 
        
	// At this point if drealmId >= 0, then we know it is 
	// an outgoing REGISTER

	/* The To URI must be in our cache */	
	if (to->name)
	{
		sprintf(url, "%s@%s", to->name, to->host);
	}
	else
	{
		sprintf(url, "%s", to->host);
	}

	// Check the To-URI in the REGISTER in our database
	srcCacheInfo = &srcCacheInfoEntry;
	if (CacheFind(uriCache, url, srcCacheInfo, sizeof(CacheTableInfo)) < 0)
	{ 
		/* Not found. Now we look for the name part of the to url,
		 * if the domain matches our default domain
		 */
		NETDEBUG(MSIP, NETLOG_DEBUG3,
			("%s uri %s not in cache\n", fn, url));

		if (SipMatchDomains(rsadomain, to->host))
		{
			/* This one must be forwarded to the requri */
			if (drealmId < 0)
			{
				/* The request URI domain seems to be our domain!!
				 * But to domain is not ours
				 * Must send Error back.
				 */
				NETDEBUG(MSIP, NETLOG_DEBUG4,
					 ("%s: Req matches our domain (%s),but to domain doesn't (%s)\n",
					  fn, req_uri->host, to->host));
				NETDEBUG(MSIP, NETLOG_DEBUG4,("Generating 403 response\n"));
					
				if (SipFormatResponse(s, context, &resp, 403,
						      "Forbidden", &host, &hostport, &err) < 0)
				{
					goto _error;
				}

				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context,
						    SIPMSG_RESPONSE, (host), hostport);
				context = NULL;
				goto _return;
			}

			// At this point, we know that the To URI does not exist on the MSW
			// and the Request is destined to be going out

			// OBP functionality
			NETDEBUG(MSIP, NETLOG_DEBUG3,
				("%s domain %s does not match our domain %s\n",
				fn, to->host, rsadomain));

			goto obp;

		}
		else
		{
			// The To URI matches our domain, is not configured
			// Check if the req uri matched or not
	
			NETDEBUG(MSIP, NETLOG_DEBUG3,
				("%s to domain %s is our domain\n", fn, to->host));

			if (drealmId < 0)
			{
				if (CacheFind(phoneCache, SVal(to->name), srcCacheInfo, 
					sizeof(CacheTableInfo)) >= 0)
				{
					NETDEBUG(MSIP, NETLOG_DEBUG4,
                                                 ("%s Found user in our phone cache as %s\n",
                                                  fn, SVal(to->name)));
                                        
                                        /* We can now register this guy */
                                        SipRegisterContact(s, &context, srcCacheInfo, rxedFromNat, 1);
                                        
                                        goto _return;                                        
				}
				else
				{
					/* The request URI domain seems to be our domain!!
				 	* Must send Error back.
				 	*/
					NETDEBUG(MSIP, NETLOG_DEBUG4,
						 ("%s: Req & to match our domain (%s),but entry not found\n",
						  fn, req_uri->host));
					NETDEBUG(MSIP, NETLOG_DEBUG4,("Generating 403 response\n"));
					
					if (SipFormatResponse(s, context, &resp, 403,
						"Forbidden", &host, &hostport, &err) < 0)
					{
						goto _error;
					}

					/* host will be freed by thread */
					SipSendMsgToHost2(resp, context,
						SIPMSG_RESPONSE, (host), hostport);
					context = NULL;
					goto _return;
				}
			}
			else
			{
				/* Safely forward this one */

				/* Insert our own via header */
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

				host = strdup(req_uri->host);
				SipSendMsgToHost2(s, dcontext, SIPMSG_REQUEST, host, 
					req_uri->port);
				dcontext = NULL;
				goto _request_forwarded;
			}

			NETERROR(MSIP, ("%s should not get here!\n", fn));

			goto _error;
		}
	}
	else if (srcCacheInfo->data.stateFlags & CL_DYNAMIC)
	{
		goto _obp_fwd;
	}
	else
	{
		// To URI exists in our database

		NETDEBUG(MSIP, NETLOG_DEBUG3,
			("%s uri %s found in cache\n", fn, url));

		/* We can now register this guy */
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s registering local user %s\n",
			fn, SVal(to->name)));
		SipRegisterContact(s, &context, srcCacheInfo, rxedFromNat, 1);

		goto _return;
	}

obp:
	if(!obpEnabled)
	{
		if (SipFormatResponse(s, context, &resp, 404, "Not Found", &host, &hostport, &err) < 0)
		{
			goto _error;
		}

		/* host will be freed by thread */
		SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);
		context = NULL;
		goto _return;
	}

	// If FC is enabled and allow dynamic is set, then we must change
	// contact as well as create a new endpt
	if (allowDynamicEndpoints && IsFCEEnabled())
	{
		/* Extract the contact header */
		if ((sip_initSipHeader(&header, SipHdrTypeAny, &err))==SipFail)
		{
			NETERROR(MSIP, ("%s could not initialize sip header\n", fn));
			goto _error;	
		}

		if (sip_getHeaderAtIndex(s, SipHdrTypeContactAny, header, (SIP_U32bit)0, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s could not get any contact header\n", fn));
			goto _error;
		}
		
		if(wild_card_flag)
		{
			if (SipGetSentByHost(s, context, 0, 0, &viahost, &contactport, &err) <= 0)
			{
				goto _error;
			}
			contacthost = viahost;
			if(to->name)
			{
				contactname = (to->name);
			}
		}
		else 
		{
			if (sip_getAddrSpecFromContactHdr(header, &addrspec, &err)==SipFail)
			{
				NETERROR(MSIP, ("%s Could not get addrspec\n", fn));
				goto _error;
			}
			
			if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, &err)==SipFail)
			{
			NETERROR(MSIP, ("Could not get Sip Url from Addr Spec %d\n", err));
			goto _error;
			}
			
			/* get 'to name' and 'to host' from the url */
			if (sip_getUserFromUrl(sipurl, &contactname, &err)==SipFail)
			{
				if (to->name)
				{
					contactname = strdup(to->name);
				}
				else
				{
					NETERROR(MSIP, ("No NAME in TO/CONTACT addr-spec - dropping %d\n", err));
					goto _error;
				}
			}
			
			if ((sip_getHostFromUrl(sipurl, &contacthost, &err))==SipFail)
			{
				NETERROR(MSIP, ("Could not get user name from Sip Url %d\n", err));
				goto _error;
			}
			
			if ((sip_getPortFromUrl(sipurl, &contactport, &err))==SipFail)
			{
				NETDEBUG(MSIP, NETLOG_DEBUG4, ("Could not get port from Sip Url %d\n", err));
				contactport = 5060;
			}
		}
		InitNetoidInfoEntry(&srcCacheInfoEntry);
		
		SetIedgeType(&srcCacheInfo->data, IEDGE_IPPHONE);
		srcCacheInfo->data.realmId = realmInfo->realmId;

		// Set Realm Name
		CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
		realmEntryPtr = CacheGet(realmCache, &realmInfo->realmId);

		if (realmEntryPtr)
		{
			nx_strlcpy(srcCacheInfo->data.realmName, realmEntryPtr->realm.realmName, REALM_NAME_LEN);
		}
		CacheReleaseLocks(realmCache);

		// Cookup a new cache entry
		gettimeofday(&tv, NULL);

		snprintf(srcCacheInfo->data.regid, sizeof(srcCacheInfo->data.regid),
						"dynamic-%lu.%06lu", tv.tv_sec + 2208988800u, tv.tv_usec);
		BIT_SET(srcCacheInfo->data.sflags, ISSET_REGID);

		srcCacheInfo->data.uport = 0;
		BIT_SET(srcCacheInfo->data.sflags, ISSET_UPORT);

		if((srcCacheInfo->data.ipaddress.l = ResolveDNS(contacthost, &herror)) == -1)
		{
			NETERROR(MSIP, ("%s contact %s does not resolve to any ip\n", fn, contacthost));
		}
		BIT_SET(srcCacheInfo->data.sflags, ISSET_IPADDRESS);

		srcCacheInfo->data.iTime = srcCacheInfo->data.mTime = srcCacheInfo->data.rTime = time(0);

		snprintf(srcCacheInfo->data.uri, sizeof(srcCacheInfo->data.uri), "%s@%s", contactname, req_uri->host);

		nx_strlcpy(srcCacheInfo->data.phone, contactname, sizeof(srcCacheInfo->data.phone));
		BIT_SET(srcCacheInfo->data.sflags, ISSET_PHONE);

		nx_strlcpy(srcCacheInfo->data.vpnPhone, contactname, sizeof(srcCacheInfo->data.vpnPhone));
		srcCacheInfo->data.vpnExtLen = strlen(srcCacheInfo->data.vpnPhone);
		BIT_SET(srcCacheInfo->data.sflags, ISSET_VPNPHONE);

		srcCacheInfo->data.stateFlags |= (CL_ACTIVE | CL_DYNAMIC);
		srcCacheInfo->data.ecaps1 |= ECAPS1_SIP_PRIVACY_RFC3325;
		srcCacheInfo->data.ecaps1 |= ECAPS1_SIP_PRIVACY_DRAFT01;
		srcCacheInfo->data.cidblock =  0;

		snprintf(srcCacheInfo->data.contact, SIPURL_LEN, "%s@%s:%d", contactname, contacthost, contactport);

		if(rxedFromNat && enableNatDetection)
		{
			srcCacheInfo->data.ecaps1 |= ECAPS1_NATDETECT;
			srcCacheInfo->data.ipaddress.l = ntohl(inet_addr(context->pTranspAddr->dIpv4));
			BIT_SET(srcCacheInfo->data.sflags, ISSET_IPADDRESS);
		}

		if((newSrcCacheInfo = GetDuplicateIedge(&srcCacheInfo->data)) != NULL)
		{
			if(srcCacheInfo->data.ipaddress.l == newSrcCacheInfo->data.ipaddress.l)
			{
				if(strcmp(srcCacheInfo->data.phone, newSrcCacheInfo->data.phone) == 0 || 
					strcmp(srcCacheInfo->data.vpnPhone, newSrcCacheInfo->data.vpnPhone) == 0 || 
					strcmp(srcCacheInfo->data.uri, newSrcCacheInfo->data.uri) == 0)
				{
					if (SipFormatResponse(s, context, &resp, 409,
						"Conflict", &host, &hostport, &err) < 0)
					{
						goto _error;
					}

					/* host will be freed by thread */
					SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);
					context = NULL;
					goto _return;
				}
				else
				{
					strcpy(srcCacheInfo->data.regid, newSrcCacheInfo->data.regid);

					for(i = 0; i < MAX_IEDGE_PORTS; ++i)
					{
						if(!BITA_TEST(newSrcCacheInfo->data.ports, i))
						{
							srcCacheInfo->data.uport = i;
							break;
						}
					}
				}
			}
			else
			{
				if (SipFormatResponse(s, context, &resp, 409,
					"Conflict", &host, &hostport, &err) < 0)
				{
					goto _error;
				}

				/* host will be freed by thread */
				SipSendMsgToHost2(resp,context, SIPMSG_RESPONSE, host, hostport);
				context = NULL;
				goto _return;
			}
		}

		if((newSrcCacheInfo = CacheDupInfoEntry(&srcCacheInfo->data)) == 0)
		{
			goto _error;
		}

		CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);
		have_locks = 1;

		AddIedge(newSrcCacheInfo);

		srcCacheInfo = newSrcCacheInfo;

	_obp_fwd:
		// We must change the contact now
		/* Register contact but don't send a response */
		if( SipRegisterContact(s, &context, srcCacheInfo, rxedFromNat, 0) < 0 )
		{
			goto _error;
		}

		/* Safely forward this one */

		// Change the contact only if its not a wild card
		if( !wild_card_flag ) {
			/* Change contact address */
			if((new_contact = UrlDup(contact, MEM_LOCAL)) == NULL)
			{
				NETERROR(MSIP,
					 ("%s UrlDup for new contact failed\n", fn));
				goto _error;
			}
			
			SipCheckFree(new_contact->host);
			new_contact->host = strdup(destrsadomain);
			new_contact->port = lSipPort;
			
			if(SipReplaceContactwithRsa(s, new_contact, &err) < 0)
			{
				NETERROR(MSIP,
					 ("%s insert new contact failed\n", fn));
				
				goto _error;
			}
		}
	}

	/* Safely forward this one */

	/* Insert our own via header */
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

	host = strdup(req_uri->host);
	SipSendMsgToHost2(s, dcontext, SIPMSG_REQUEST, host, req_uri->port);
	dcontext = NULL;
	// JUst there so that code is standard
	goto _request_forwarded;

 _request_forwarded:
	if (have_locks)
	{
		CacheReleaseLocks(regCache);
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	SipCheckFree(to);
	SipCheckFree(from);
	SipCheckFree(req_uri);
	SipCheckFree(contact);
	if(new_contact)
	{
		SipCheckFree(new_contact->name); SipCheckFree(new_contact->host); SipCheckFree(new_contact->tag);
		SipCheckFree(new_contact);
	}
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);
	SipFreeContext(context);
	SipCheckFree(viahost);

	return;

 _error:
	SipFreeContext(context);
 _return:
	if (have_locks)
	{
		CacheReleaseLocks(regCache);
	}

	sip_freeSipHeader(header);
	SipCheckFree(header);
	SipCheckFree(to);
	SipCheckFree(from);
	SipCheckFree(req_uri);
	SipCheckFree(contact);
	if(new_contact)
	{
		SipCheckFree(new_contact->name); SipCheckFree(new_contact->host); SipCheckFree(new_contact->tag);
		SipCheckFree(new_contact);
	}
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);
	sip_freeSipMessage(s);
	SipFreeContext(dcontext);
	SipCheckFree(viahost);

	return;
}

/* Add the contact, se the params, send back the OK
 * with the contact params and the action.
 * OK might require a DNS
 */
int
SipRegisterContact(
	SipMessage *s, 
	SipEventContext** in_context,
	CacheTableInfo *cacheInfo,
	int rxedFromNat,
	int generate_response
)
{
	char fn[] = "SipRegisterContact():";
	SipMessage *m = NULL;
	SipHeader *header = NULL;
	SipExpiresHeader *expiresheader=NULL;
	SipError err;
	SipAddrSpec *addrspec = NULL;
	SipUrl *sipurl = NULL;
	char *contactname = NULL, *contacthost = NULL;
	SipExpiresStruct *expires = NULL;
	SipHeader *expireshdr=NULL;
	SIP_U32bit count,k;
	SipContactParam *contactp = NULL;
	en_ContactParamsType ptype;
 	char *action;
	int delta = 3600; /* 1 hour registration */
	SIP_U16bit contactport;
	char *host = NULL;
	unsigned short port;
	int herror;
	SipContactParam *ContactAction=NULL, *ContactExpires=NULL;
	int actionflag=0,expiresflag=0,expireshdrflag=0;
	char tags[TAGH_LEN] = { 0 };
	unsigned long contactip = 0;
	CacheTableInfo *tmpCacheInfo;
	CallRealmInfo *realmInfo;
	unsigned long realmId;
	CacheRealmEntry *realmEntry = NULL;
	SipEventContext* context = NULL;

	if(in_context)
	{
		context = *in_context;
	}

	if (!context || !context->pData) 
	{ 
		NETERROR(MSIP, ("%s no context is present %p\n", fn, context));
		return 0;
	}

	realmInfo = (CallRealmInfo *)context->pData;

	if (!strcmp(cacheInfo->data.realmName, REALM_ANY))
	{
		// Store the new realm id inside the entry
		cacheInfo->data.realmId = realmInfo->realmId;
	}
	else
	{
		// We must match up the realms and deny the registration if necessary	
		if (cacheInfo->data.realmId != realmInfo->realmId)
		{
			NETERROR(MSIP, ("%s %s/%lu realm should be %d instead of %lu\n",
				fn, cacheInfo->data.regid, cacheInfo->data.uport, cacheInfo->data.realmId, realmInfo->realmId));

			NETDEBUG(MSIP, NETLOG_DEBUG4,("Generating 403 response\n"));
			if (SipFormatResponse(s, context, &m, 403,
					      "Forbidden", &host, &port, &err) < 0)
			{
				goto _error;
			}

			/* host will be freed by thread */
			SipSendMsgToHost2(m,context, SIPMSG_RESPONSE, (host), port);
			context = NULL;
			goto _error;
		}
	}

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

	if(sipauth && !obpEnabled && SipChallengeMethod("REGISTER", realmEntry->realm.authFlags) )
	{
		if(SipCheckAuthorization(s,"REGISTER")<0)
		{
			NETDEBUG(MSIP,NETLOG_DEBUG4,
				 ("%s REGISTER authorization failed\n",fn)); 
			if (SipFormatResponse(s, context, &m, 401,"Unauthorized",
					      &host, &port, &err) < 0)
			{
				NETERROR(MSIP,("%s Fail to format response 401.\n",fn));
				return(0);
			}

#if 0
			// This should not be here ???
			/* done with original message */
			sip_freeSipMessage(s);
#endif
			
			if(SipInsertAuthenticateHdr(m,"REGISTER")<0)
			{
				sip_freeSipMessage(m);
				NETERROR(MSIP,("%s Fail to insert AuthenHdr\n",fn));
				return(0);
			}

			/* host will be freed by thread */
			SipSendMsgToHost2(m,context,
					    SIPMSG_RESPONSE, (host), port);
			context = NULL;
			return(0);
		}
		else
		{
			NETDEBUG(MSIP,NETLOG_DEBUG4,
				 ("%s REGISTER authorization passed\n",fn));
		} 
	}
	
	/* Extract the contact header first */
	if ((sip_initSipHeader(&header, SipHdrTypeAny, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip header\n",
			fn));
		return( -1);	
	}

	if (sip_getHeaderAtIndex(s, SipHdrTypeContactAny, header, (SIP_U32bit)0,
			&err) == SipFail)
	{
		NETERROR(MSIP, ("%s could not get any contact header\n", fn));
		goto _error;
	}

	if(sip_getContactParamsCountFromContactHdr(header,&count,&err) == SipFail)
	{
		NETERROR(MSIP,("%s error in getContactParamsCount header %d\n",fn,err));
		goto _error;
	}

	if(count==0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s no contact param\n", fn));
		goto _check_expireshdr;
	}

	for(k=0;k<count;k++)
	{
		if (sip_getContactParamAtIndexFromContactHdr(header,
							     &contactp, k, &err) == SipFail)
		{
			NETERROR(MSIP,("%s error get %d-th ContactParam\n", fn,k));
			goto _error;
		}
		if (sip_getTypeFromContactParam(contactp,&ptype,&err) == SipFail)
		{
			NETERROR(MSIP,("%s error get %d-th ContactParam type\n",fn,k));
			goto _error;
		}
		if(ptype == SipCParamExpires)
		{
			/* Extract the expires parameter */
			if (sip_getExpiresFromContactParam(contactp, &expires, &err) 
			    == SipFail)
			{
				NETERROR(MSIP, ("%s error getExpires %d\n", fn,k));
				goto _error;
			}
			/* Now look into the expires structure */
			/* We just support seconds */
			if (expires->dType != SipExpSeconds)
			{
				delta = 1;
			}
			else if (expires->u.dSec != 0)
			{
				delta = expires->u.dSec;
				/* longest reg. time is CACHE_TIMEOUT_DEFAULT */
				expires->u.dSec=MMIN(delta,cacheTimeout/2);
			}
			else
			{
				delta = 0;
			}
			NETDEBUG(MSIP,NETLOG_DEBUG4,("%s expires param found delta=%d\n",
						     fn,delta));
			expiresflag=1;
		}
#if 0	//12-Apr-04 Action deprecated in RFC3261
		else if(ptype == SipCParamAction)
		{
			if(sip_getActionFromContactParam(contactp,&action,&err) == SipFail)
			{
				NETERROR(MSIP, ("%s error getAction %d\n", fn,k));
				goto _error;
			}
			/* currently we only support action=proxy */
			if((strcmp(action,"proxy") && (sipservertype==SERVER_PROXY)) ||
			/* when we only support action=redirect */
			   (strcmp(action,"redirect") && (sipservertype==SERVER_REDIRECT)))
			{
				NETERROR(MSIP, ("%s action=%s\n",fn,action));
				/* need to generate 403 Forbidden */
				NETDEBUG(MSIP, NETLOG_DEBUG4,("Generating 403 response\n"));
				
				if (SipFormatResponse(s, context, &m, 403,
						      "Forbidden", &host, &port, &err) < 0)
				{
					goto _error;
				}
				
				/* host will be freed by thread */
				SipSendMsgToHost2(m,context,
						    SIPMSG_RESPONSE, (host), port);
				context = NULL;
				goto _error;
			}
			actionflag=1;
		}
#endif	//12-Apr-04 Action deprecated in RFC3261
	}

 _check_expireshdr:
	if(expires==NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s no expires param, checking Expires Hdr\n", fn));
		if (sip_initSipHeader(&expireshdr, SipHdrTypeAny, &err)==SipFail)
		{
			NETERROR(MSIP, ("%s could not initialize expiriesheader\n",fn));
			goto _error;
		}
		/* currently we only support seconds format */
		if (sip_getHeader(s, SipHdrTypeExpiresAny, expireshdr, &err)==SipFail)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s could not get Expires Hdr\n",fn));
			goto _default_reg;
		}
		if (sip_getSecondsFromExpiresHdr(expireshdr,&delta,&err) == SipFail)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4, 
				 ("%s could not get seconds from ExpiresHdr\n",fn));
			goto _default_reg;
		}
		/* longest reg. time is CACHE_TIMEOUT_DEFAULT */
		(((SipExpiresHeader *)(expireshdr->pHeader))->u).dSec 
		  = MMIN(delta,(cacheTimeout/2)); 

		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s found Expires header,delta=%d\n",fn,delta));
		expireshdrflag=1;
	}

_default_reg:
	if (delta == 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Unregistration \n", fn));
		cacheInfo->data.stateFlags &= ~CL_ACTIVE;
		goto _save_db;
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Registration for %d secs\n", fn, delta));
		if ((cacheInfo->data.stateFlags & CL_DYNAMIC) && !(cacheInfo->data.stateFlags & CL_ACTIVE))
			cacheInfo->data.stateFlags |= CL_ACTIVE;
		if (!(cacheInfo->data.stateFlags & CL_DYNAMIC))
			cacheInfo->data.stateFlags |= (CL_ACTIVE|CL_REGISTERED);
		time(&cacheInfo->data.rTime);
	}

	if (sip_getAddrSpecFromContactHdr(header, &addrspec, &err)==SipFail)
	{
		NETERROR(MSIP, ("%s Could not get addrspec\n", fn));
		goto _error;
	}

	if (addrspec->dType != SipAddrSipUri || sip_getUrlFromAddrSpec(addrspec, &sipurl, &err)==SipFail)
	{
		NETERROR(MSIP, ("Could not get Sip Url from Addr Spec %d\n", err));
		goto _error;
	}

	/* get 'to name' and 'to host' from the url */
	if (sip_getUserFromUrl(sipurl, &contactname, &err)==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG3,
			 ("Could not get user name from Sip Url %d\n", err));
	}

	if ((sip_getHostFromUrl(sipurl, &contacthost, &err))==SipFail)
	{
		NETERROR(MSIP, ("Could not get user name from Sip Url %d\n", err));
		goto _error;
	}

	if ((sip_getPortFromUrl(sipurl, &contactport, &err))==SipFail)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("Could not get port from Sip Url %d\n", err));
		contactport = 5060;
	}

	BIT_SET(cacheInfo->data.cap, CAP_SIP);

	if (contactname)
	{
		snprintf(cacheInfo->data.contact, SIPURL_LEN,
			"%s@%s:%d", contactname, contacthost, contactport);
	}
	else
	{
		snprintf(cacheInfo->data.contact, SIPURL_LEN,
			"%s:%d", contacthost, contactport);
	}

	/* Put the contact host into the ip address field after we
	 * do a DNS request on it.
	 */
	if((contactip = ResolveDNS(contacthost, &herror)) == -1)
	{ 
		NETERROR(MSIP, ("%s contact %s does not resolve to any ip\n",
			fn, contacthost));
		contactip = 0;
	}

_save_db:

	BITA_SET(tags, TAG_IPADDRESS);
	BITA_SET(tags, TAG_REGSTATUS);
	BITA_SET(tags, TAG_SIP);
	BITA_SET(tags, TAG_CONTACT);

	/* Update the cache.
	 * We dont know what cache to update at this point.
	 * phone cache or uri cache, as control may have come in
	 * from any place. Thus we will use the reg cache
	 */
	CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);
	if (!(cacheInfo->data.stateFlags & CL_DYNAMIC) || cacheInfo->data.stateFlags & CL_ACTIVE)
	{
		tmpCacheInfo = DeleteIedge(&cacheInfo->data);

		if (tmpCacheInfo && (tmpCacheInfo != cacheInfo))
		{
			memcpy(tmpCacheInfo, cacheInfo, sizeof(CacheTableInfo));
		}
		else if (!tmpCacheInfo)
		{
			// We need to allocate it
			tmpCacheInfo = CacheDupInfoEntry(&cacheInfo->data);
		}

		if(rxedFromNat && (tmpCacheInfo->data.ecaps1 & ECAPS1_NATDETECT))
		{
			tmpCacheInfo->data.natIp = ntohl(inet_addr(context->pTranspAddr->dIpv4));
			BIT_SET(tmpCacheInfo->data.sflags, ISSET_NATIP);

			tmpCacheInfo->data.natPort = context->pTranspAddr->dPort;
			BIT_SET(tmpCacheInfo->data.sflags, ISSET_NATPORT);

			tmpCacheInfo->data.ipaddress.l = tmpCacheInfo->data.natIp;
			BIT_SET(tmpCacheInfo->data.sflags, ISSET_IPADDRESS);
		}
		else if (contactip)
		{
			tmpCacheInfo->data.ipaddress.l = contactip;
			BIT_SET(tmpCacheInfo->data.sflags, ISSET_IPADDRESS);
		}

		// reset the NAT IP and Port if the endpoint is not behind
		// a NAT anymore
		if(!rxedFromNat && (tmpCacheInfo->data.ecaps1 & ECAPS1_NATDETECT))
		{
			tmpCacheInfo->data.natIp = 0;
			BIT_RESET(tmpCacheInfo->data.sflags, ISSET_NATIP);

			tmpCacheInfo->data.natPort = 0;
			BIT_RESET(tmpCacheInfo->data.sflags, ISSET_NATPORT);
		}

		AddIedge(tmpCacheInfo);

		/* Update the database also */
		DbScheduleIedgeUpdate(&tmpCacheInfo->data);

		if (!(cacheInfo->data.stateFlags & CL_DYNAMIC))
		{
			GisPostCliIedgeRegCmd(tmpCacheInfo->data.regid, tmpCacheInfo->data.uport, tags);
		}
	}

	CacheReleaseLocks(regCache);

	if (generate_response) 
	{
		if (SipFormatResponse(s, context, &m, 200, "OK",
			&host, &port, &err) < 0)
		{
			NETERROR(MSIP, ("%s Error in Formatting 200 OK\n", fn));
			goto _error;
		}
	}
	else 
	{
		m = s;
	}

	/* if request does not have action, we specify it to be proxy or redirect */
	/* only if we are not the obp */
	if (delta && (actionflag == 0) && generate_response)
	{
		if(sipservertype==SERVER_PROXY)
			action="proxy";
		else
			action="redirect";
			
#if 0	//12-Apr-04 Action deprecated in RFC3261
		if (sip_initSipContactParam(&ContactAction,SipCParamAction,&err) == SipFail)
		{
			NETERROR(MSIP, ("%s Error in init ContactAction %d\n",fn,err));
			goto _error;
		}
		if(sip_setActionInContactParam(ContactAction,strdup(action),&err) == SipFail)
		{
			NETERROR(MSIP,("%s Error in set action in ContactAction %d\n",fn,err));
			goto _error;
		}
		if(sip_insertContactParamAtIndexInContactHdr(header,ContactAction,0,&err) == SipFail)
		{
			NETERROR(MSIP, ("%s Error insert ContactAction in header %d\n",fn,err));
			goto _error;
		}
#endif	//12-Apr-04 Action deprecated in RFC3261
	}

	/* if request does not have expires, we specify it to be 3600 s */
	if(delta && expireshdrflag)
	{
		/* Expires hdr was present, so add in response */
		if(sip_setHeader(m, expireshdr, &err) == SipFail)
		{
			NETERROR(MSIP,("%s error set Expires hdr \n",fn));
			goto _error;
		}
	}
	else if (delta && (expires==NULL))
	{
		/* no expire param and no Expires hdr, insert default reg. time in expires param */
		if(sip_initSipExpiresStruct(&expires,SipExpSeconds,&err) == SipFail)
		{
			NETERROR(MSIP,("%s error init expirs struct\n",fn));
			goto _error;
		}
		expires->u.dSec=cacheTimeout/2;
		if(sip_initSipContactParam(&ContactExpires,SipCParamExpires,&err) == SipFail)
		{
			NETERROR(MSIP, ("%s Error in init ContactExpires %d\n",fn,err));
			goto _error;
		}
		if(sip_setExpiresInContactParam(ContactExpires,expires,&err)==SipFail)
		{
			NETERROR(MSIP,("%s Error in set expires in ContactAction %d\n",fn,err));
			goto _error;
		}
		if(sip_insertContactParamAtIndexInContactHdr(header,ContactExpires,0,&err) == SipFail)
		{
			NETERROR(MSIP, ("%s Error insert ContactExpires in header %d\n",fn,err));
			goto _error;
		}
	}

	if (delta)
	{
		if (generate_response == 0)
		{
			// if modifying incoming message 
			if (sip_deleteHeaderAtIndex(m, SipHdrTypeContactAny, 0, &err) == SipFail)
			{
				NETERROR(MSIP, ("%s Could not delete contact header\n", fn));
				goto _error;
			}
		}

		/* Add the contact address */
		if (sip_insertHeaderAtIndex(m, header, (SIP_U32bit)0, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Error in copying contact headers\n", fn));
			goto _error;
		}
	}

	/* send message m */
	/* OK must be sent to the address in the Via header,
	 * and not the contact address in the message.
	 * If there is a record-route in registration,
	 * we need to store it, so that, when an invite comes in
	 * we can insert the route headers, and then send the invite out.
	 * host will be freed by the thread
	 */ 
	if (generate_response)
	{
		SipSendMsgToHost2(m, context, SIPMSG_RESPONSE, (host), port);
		context = NULL;
	}

_return:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipHeader(expireshdr);
	SipCheckFree(expireshdr);
	sip_freeSipContactParam(ContactAction);
	sip_freeSipContactParam(ContactExpires);

	sip_freeSipExpiresStruct(expires);
	sip_freeSipContactParam(contactp);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);
	//sip_freeSipMessage(s);
	
	return(0);
_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipHeader(expireshdr);
	SipCheckFree(expireshdr);
	sip_freeSipContactParam(ContactAction);
	sip_freeSipContactParam(ContactExpires);

	sip_freeSipExpiresStruct(expires);
	sip_freeSipContactParam(contactp);
	sip_freeSipAddrSpec(addrspec);
	sip_freeSipUrl(sipurl);
	//sip_freeSipMessage(s);
	
	return(-1);
}


