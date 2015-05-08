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
#include "uh323.h"
#include "uh323cb.h"
#include "db.h"
#include "firewallcontrol.h"
#include "lsprocess.h"
#include "tags.h"
#include "h323realm.h"
#include "nxosd.h"
#include <malloc.h>

#include "gk.h"
#include "stkutils.h"
#include "dbs.h"
#include "ls.h"
#include "ipstring.h"

int IsIgrpQuotaFull(NetoidInfoEntry *pentry);

int
GkHandleLightRRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall,
		int ttl)
{
	char fn[] = "GkHandleLightRRQ():";
	char endptIdStr[128] = { 0 };
	BYTE endptId[128];
	cmAlias alias;
	cmTransportAddress addr;
	int rc = -1, priority = 0, rcfttl;
	PhoNode phonode = { 0 };
	INT32 tlen;
	CacheTableInfo *cacheInfo = 0, *info = NULL;
	NetoidInfoEntry *netInfo;
	cmAlternateGatekeeper altgk = { 0 };
	struct sockaddr_in *inaddr;
	BYTE gk_id[128];
	char tags[TAGH_LEN] = { 0 };
	RealmIP realmip;
	int i;
	int	sd,realmId;
	cmTransportAddress ouraddr;
	unsigned long lport, hport, uport;

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		  ("%s Entering, hsRas = %p, hsCall = %p, haCall = %p\n",
			fn, hsRas, hsCall, haCall));

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
					  cmRASParamSocketDesc, 0, 
					  (int *)&sd, NULL) < 0)
	{
		NETERROR(MRRQ, ("%s No socket\n", fn));
		rc = -1;
		return rc;
	}
	
	if(getQ931RealmInfo(sd,&ouraddr.ip,&ouraddr.port,&realmId) <0)
	{
		NETERROR(MRRQ, ("%s getQ931RealmInfo failed\n", fn));
		rc = -1;
		return rc;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, realmId, sd, ULIPtostring(ouraddr.ip), ouraddr.port));

	ouraddr.ip = htonl(ouraddr.ip);

	/* Set up alias */
	alias.string = (char *)&endptId[0];
	alias.length = 128;
	tlen = sizeof(cmAlias);

	/* Extract the endpoint Id */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamEndpointID, 0, &tlen,
					  (char *)&alias) < 0)
	{
		 NETERROR(MRRQ,
				  ("%s Could not get endpoint id\n", fn));
		 return rc;
	}

	/* Convert it back to a string */
	utlBmp2Chr(&endptIdStr[0], alias.string, alias.length);

	GisExtractRegIdInfo(endptIdStr, &phonode);
	uport = phonode.uport;
	
	if (phonode.uport > MAX_IEDGE_PORTS)
	{
		lport = 0;
		hport = MAX_IEDGE_PORTS-1;
	}
	else
	{
		lport = hport = phonode.uport;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
			 ("%s Received from endpoint %s/%lu/%s\n",
			  fn, phonode.regid, phonode.uport,
				(char*) ULIPtostring(phonode.ipaddress.l)));

	/* We must referesh the iedge cache entry */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	
	realmip.ipaddress = phonode.ipaddress.l;
	realmip.realmId = phonode.realmId;
	cacheInfo = CacheGet(ipCache, &realmip);

	if (cacheInfo == NULL)
	{
		NETERROR(MRRQ, 
			("%s No IP cache entry found for this LRRQ from %s/%lu/%s/%lu\n", 
			fn, phonode.regid, phonode.uport, 
			ULIPtostring(phonode.ipaddress.l), phonode.realmId));

		rc = -cmRASReasonDiscoveryRequired;

		goto _release_locks;
	}

	BITA_SET(tags, TAG_IPADDRESS);
	BITA_SET(tags, TAG_REGSTATUS);
	BITA_SET(tags, TAG_H323);
	BITA_SET(tags, TAG_H323SIGPT);

	netInfo = &cacheInfo->data;

	for (i=lport; i <= hport; i++)
	{
		if (BITA_TEST(netInfo->ports, i))
		{
			/* Look for this iedge port */
			phonode.uport = i;
	 		BIT_SET(phonode.sflags, ISSET_UPORT);

			info = (CacheTableInfo *)CacheGet(regCache, &phonode);
			
			if (info && !IsSGatekeeper(&info->data))
			{
				NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("%s regid %s/%lu\n", fn, netInfo->regid, 
					netInfo->uport));

				/* We must mark in the db that this
				 * netoid is active now
		 		*/
				time(&info->data.rTime);
				info->data.stateFlags |= CL_REGISTERED;
				info->data.stateFlags |= CL_ACTIVE;

				NETDEBUG(MRRQ, NETLOG_DEBUG4, 
					("%s Entry refreshed at rTime %lu", fn, (info->data.rTime)));

				rc = 1;

				/* Update the database also */
				DbScheduleIedgeUpdate(&info->data);
				GisPostCliIedgeRegCmd(info->data.regid, info->data.uport, tags);
			}
		}
	}

	if ((ttl <=0) || (ttl > cacheTimeout/2))
	{
		rcfttl = cacheTimeout/2;
	}
	else
	{
		rcfttl = ttl;
	}

	/* Set the TTL of the response */
	if (cmRASSetParam(hsRas, cmRASTrStageConfirm, 
					  cmRASParamTimeToLive, 0, 
					  rcfttl, NULL) > 0)
	{
		NETERROR(MRRQ,
			("%s Could not set TTL to %d\n", fn, rcfttl));
	}

	/* We have to assign an endpoint ID to the endpoint */
	/* We will assign him his serial-number/uport combination */
	sprintf(endptIdStr, "%s!%d!%d!%d", 
			cacheInfo->data.regid, htonl(phonode.ipaddress.l), 
			htonl(cacheInfo->data.uport), cacheInfo->data.realmId);

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Assigning id %s\n", fn, endptIdStr));

	alias.type = cmAliasTypeEndpointID;
	alias.length = utlChr2Bmp(&endptIdStr[0], &endptId[0]);
	alias.string = (char *)&endptId[0];

	if (cmRASSetParam(hsRas, cmRASTrStageConfirm, 
					  cmRASParamEndpointID, 0, sizeof(cmAlias),
					  (char *)&alias) < 0)
	{
		 NETERROR(MRRQ,
				  ("%s Could not set endpoint id\n", fn));
	}

	// Add the alt gks
	for (	inaddr = (struct sockaddr_in *)listGetFirstItem(altGkList);
			inaddr; 
			inaddr = (struct sockaddr_in *)listGetNextItem(altGkList, inaddr))
	{
		altgk.rasAddress.ip = inaddr->sin_addr.s_addr;
		altgk.rasAddress.port = inaddr->sin_port;
		altgk.needToRegister = 1;
		altgk.priority = priority ++;
		altgk.gatekeeperIdentifier.type = cmAliasTypeGatekeeperID;
		altgk.gatekeeperIdentifier.length = utlChr2Bmp("nextone", &gk_id[0]);
		altgk.gatekeeperIdentifier.string = (char *)&gk_id[0];

		if (cmRASSetParam(hsRas, cmRASTrStageConfirm,
				cmRASParamAlternateGatekeeper, altgk.priority, sizeof(altgk),
				(char *)&altgk) < 0)
		{
			NETERROR(MRRQ,
				("%s Failed to add altgk %x/%d\n", fn, altgk.rasAddress.ip,
				altgk.rasAddress.port));
		}
	}

	if (rc == 1)
	{
		tlen = sizeof(cmTransportAddress);

		if (cmRASSetParam(hsRas, cmRASTrStageConfirm,
				cmRASParamCallSignalAddress,
				0, tlen, (char*)&ouraddr) < 0)
		{
			NETERROR(MH323, ("%s Unable to set CallSignalAddress\n", fn));
		}
	}

_release_locks:
	CacheReleaseLocks(regCache);

	return rc;
}

int
GkHandleRRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	char fn[] = "GkHandleRRQ():";
	CacheTableInfo *cacheInfo = 0, *ipCacheInfo;
	NetoidInfoEntry tmpInfo;
	char string[80], endptIdStr[128] = { 0 };
	cmAlias number, alias;
	cmTransportAddress addr, rasaddr, oldaddr, ouraddr;
	int index = 0, ttl = -1, keepAlive = 0, rcfttl;
	cmEndpointType eptype;
	INT32  tlen;
	BYTE endptId[128];
	int rc = -1, confirm = 0, priority = 0;
	char tags[TAGH_LEN] = { 0 };
	char h323id[H323ID_LEN] = { 0 };
	cmAlternateGatekeeper altgk = { 0 };
	struct sockaddr_in *inaddr;
	RealmIP realmip;
	BYTE gk_id[128];
	int	sd,realmId, updateIpCache = 0;

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		  ("%s Entering, hsRas = %p, hsCall = %p, haCall = %p\n",
			fn, hsRas, hsCall, haCall));
	
	GkInsertGkID(hsRas, cmRASTrStageConfirm, gkid);

	if (GkCompareGkID(hsRas) < 0)
	{
		 DEBUG(MRRQ, NETLOG_DEBUG4,
				("%s GKID mismatch\n", fn));

		 //rc = -cmRASReasonInvalidPermission;

		 cmRASClose(hsRas);
		 return 0;
	}

	BITA_SET(tags, TAG_RTIME);
	BITA_SET(tags, TAG_ACTIVE);
	BITA_SET(tags, TAG_IPADDRESS);
	BITA_SET(tags, TAG_ENDPTTYPE);

	/* Extract the parameters and see if its anything
	 * in our database.
	 * There may be multiple phones in the RRQ. If we match
	 * any one, we will send an RCF, and if the iedge is not marked
	 * active, we will make it active. Also, mark it as H323-discovered
	 */

	/* Extract the TTL which the client wants */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamTimeToLive, 0, 
					  &ttl, NULL) >= 0)
	{
		 NETDEBUG(MRRQ, NETLOG_DEBUG4,
				  ("%s TTL in reg is %d\n", fn, ttl));
	}	

	/* Check to see if its a keep-alive */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamKeepAlive, 0, 
					  &keepAlive, NULL) < 0)
	{
		 NETDEBUG(MRRQ, NETLOG_DEBUG4, 
				("%s No Keep Alive field: Standard RRQ\n", fn));
		 keepAlive = 0;
	}
	else
	{
		 /* Check to see if its a lightweight RRQ */
		 if (keepAlive)
		 {
			  return GkHandleLightRRQ(hsRas, hsCall, lphaRas, 
									  srcAddress, haCall, ttl);
		 }
	}

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
					  cmRASParamSocketDesc, 0, 
					  (int *)&sd, NULL) < 0)
	{
		NETERROR(MRRQ, ("%s No socket\n", fn));
		rc = -1;
		return rc;
	}
	
	if(getQ931RealmInfo(sd,&ouraddr.ip,&ouraddr.port,&realmId) <0)
	{
		NETERROR(MRRQ, ("%s getQ931RealmInfo failed\n", fn));
		rc = -1;
		return rc;
	}

	tlen = sizeof(cmTransportAddress);

	/* Extract the IP address */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamCallSignalAddress, 0, 
					  &tlen, (char *)&addr) < 0)
	{
		 NETERROR(MRRQ, ("%s No Signalling Address found\n", fn));

		rc = -cmRASReasonInvalidCallSignalAddress;

		 return rc;
	}
	
	addr.ip = ntohl(addr.ip);

	tlen = sizeof(cmTransportAddress);

	/* Extract the RAS IP address */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamRASAddress, 0, 
					  &tlen, (char *)&rasaddr) < 0)
	{
		 NETERROR(MRRQ, ("%s No RAS Address found\n", fn));

		rc = -cmRASReasonInvalidRASAddress;

		 return rc;
	}

	rasaddr.ip = ntohl(rasaddr.ip);

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
					  cmRASParamEndpointType, 0, 
					  (int *)&eptype, NULL) < 0)
	{
		 NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s No eptype\n", fn));
		 eptype = cmEndpointTypeTerminal;
	}
	
	ouraddr.ip = htonl(ouraddr.ip);

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
			 ("%s Sig Addr %s/%d RAS Addr %s/%d sd %d\n",
			  fn, (char*) ULIPtostring(addr.ip), addr.port,
			  (char*) ULIPtostring(rasaddr.ip), rasaddr.port,sd));

	/* Lock the cache */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	realmip.ipaddress = addr.ip;
	realmip.realmId = realmId;

	/* If the ip address is in the cache, get it */
	ipCacheInfo = (CacheTableInfo *)CacheGet(ipCache, &realmip);
	
	tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = string;

	while ( (cmRASGetParam(hsRas, cmRASTrStageRequest, 
							 cmRASParamTerminalAlias, index++, 
							 &tlen, (char *)&number)) >= 0)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s for alias type = %d %s\n", 
			fn, number.type,number.string));

		if (number.type != cmAliasTypeE164 && number.type != cmAliasTypeH323ID)
		{
			goto _continue;
		}

        if (number.type == cmAliasTypeE164)
        {
            cacheInfo = (CacheTableInfo *)CacheGet(phoneCache, number.string);
			NETDEBUG(MRRQ, NETLOG_DEBUG4,
				("%s for alias e164 length = %d number = %s\n", 
				fn,number.length, number.string));
        }
        else if (number.type == cmAliasTypeH323ID)
        {
			utlBmp2Chr(&h323id[0], number.string, number.length);
            cacheInfo = (CacheTableInfo *)CacheGet(h323idCache, h323id);
			NETDEBUG(MRRQ, NETLOG_DEBUG4,
				("%s for alias type H323 %s\n",fn,h323id));
        }   

		if (cacheInfo && (cacheInfo->data.realmId != REALM_ID_UNASSIGNED))
		{
			if (!strcmp(cacheInfo->data.realmName, REALM_ANY))
			{
				// Store the new realm id inside the entry
				cacheInfo->data.realmId = realmId;
			}
			else if (cacheInfo->data.realmId != realmId)
			{
				NETDEBUG(MRRQ, NETLOG_DEBUG4,
					("%s Realm Id %d and %d do not match\n",
					fn, cacheInfo->data.realmId, realmId));
				continue;
			}

			// Must see if the old ip address needs to
			// be deleted from the cache
			DeleteIedgeIpAddr(ipCache, &cacheInfo->data);

			// Set up the ipaddress and realm if needed
			cacheInfo->data.ipaddress.l = addr.ip;
			cacheInfo->data.callsigport = addr.port;
			
			/* We must add an ipCache entry */
			if (AddIedgeIpAddr(ipCache, cacheInfo) >= 0)
			{	
				if (ipCacheInfo == NULL)
				{
					ipCacheInfo = cacheInfo;
				}
				else if (ipCacheInfo != cacheInfo)
				{
					updateIpCache = 1;
				}
			}

            if (number.type == cmAliasTypeH323ID)
            {
            	NETDEBUG(MRRQ, NETLOG_DEBUG4,
                	("%s Activated uport %lu for h323id %s\n",
                    fn, cacheInfo->data.uport, 
                    cacheInfo->data.h323id));
            }
            else
            {
            	NETDEBUG(MRRQ, NETLOG_DEBUG4,
                	("%s Activated uport %lu for phone %s\n",
                    fn, cacheInfo->data.uport, 
                    cacheInfo->data.phone));
            }

			/* We will confirm the registration */
			confirm++;

			/* Set the entry to active, 
			* set the ip address, if its not there,
			* and set it to H323-discovered.
			*/
			cacheInfo->data.stateFlags |= CL_ACTIVE;
			cacheInfo->data.stateFlags |= CL_REGISTERED;

			cacheInfo->data.crId = iservercrId;

			BIT_SET(cacheInfo->data.sflags, ISSET_IPADDRESS);

			time(&cacheInfo->data.rTime);

			cacheInfo->data.rasip = rasaddr.ip;
			cacheInfo->data.rasport = rasaddr.port;

			BIT_SET(cacheInfo->data.cap, CAP_IRQ);
			BIT_SET(cacheInfo->data.cap, CAP_H323);

			NETDEBUG(MRRQ, NETLOG_DEBUG4, 
				("%s Updating database...\n", fn));

			memcpy(&tmpInfo, &cacheInfo->data, 
				 sizeof(NetoidInfoEntry));

		}
	_continue:
		 tlen = sizeof(cmAlias);
		 number.length = 80;
		 number.string = string;
	}

	if (confirm == 0)
	{
		 NETDEBUG(MRRQ, NETLOG_DEBUG4,
				  ("%s No Cache Found\n", fn));

		rc = -cmRASInvalidAlias;

		 goto _return;
	}

	rc = 1;

	if ((ttl <= 0) || (ttl > cacheTimeout/2))
	{
		rcfttl = cacheTimeout/2;
	}
	else
	{
		rcfttl = ttl;
	}

	BITA_SET(tags, TAG_IPADDRESS);
	BITA_SET(tags, TAG_REGSTATUS);
	BITA_SET(tags, TAG_H323);
	BITA_SET(tags, TAG_H323SIGPT);

	/* Set the TTL of the response */
	if (cmRASSetParam(hsRas, cmRASTrStageConfirm, 
					  cmRASParamTimeToLive, 0, 
					  rcfttl, NULL) > 0)
	{
		 NETERROR(MRRQ,
				  ("%s Could not set TTL to %d\n", fn, rcfttl));
	}

	if (updateIpCache && ipCacheInfo)
	{
		 NETDEBUG(MRRQ, NETLOG_DEBUG4,
				  ("%s ip cache regid %s\n", fn, ipCacheInfo->data.regid));

		 /* Set the flags in this one also */
		 time(&ipCacheInfo->data.rTime);

		// If there is no registration yet for this port
		if (ipCacheInfo->data.rasip == 0)
		{
		 	ipCacheInfo->data.rasip = rasaddr.ip;
			ipCacheInfo->data.rasport = rasaddr.port;
		}

		 BIT_SET(ipCacheInfo->data.cap, CAP_IRQ);

		 ipCacheInfo->data.stateFlags |= CL_REGISTERED;
		 ipCacheInfo->data.stateFlags |= CL_ACTIVE;
	}

	if (confirm == 1)
	{
		tlen = sizeof(cmTransportAddress);

		if (cmRASSetParam(hsRas, cmRASTrStageConfirm,
				cmRASParamCallSignalAddress,
				0, tlen, (char*)&ouraddr) < 0)
		{
			NETERROR(MH323, ("%s Unable to set CallSignalAddress\n", fn));
		}
	}

	/* We have to assign an endpoint ID to the endpoint */
	/* We will assign him his serial-number/uport combination */
	sprintf(endptIdStr, "%s!%d!%d!%d", ipCacheInfo->data.regid, htonl(addr.ip), 
		(confirm > 1)?MAX_IEDGE_PORTS+1:htonl(ipCacheInfo->data.uport),
		ipCacheInfo->data.realmId);

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Assigning id %s\n", fn, endptIdStr));

	alias.type = cmAliasTypeEndpointID;
	alias.length = utlChr2Bmp(&endptIdStr[0], &endptId[0]);
	alias.string = (char *)&endptId[0];

	if (cmRASSetParam(hsRas, cmRASTrStageConfirm, 
					  cmRASParamEndpointID, 0, sizeof(cmAlias),
					  (char *)&alias) < 0)
	{
		 NETERROR(MRRQ,
				  ("%s Could not set endpoint id\n", fn));
	}


	// Add the alt gks
	for (	inaddr = (struct sockaddr_in *)listGetFirstItem(altGkList);
			inaddr; 
			inaddr = (struct sockaddr_in *)listGetNextItem(altGkList, inaddr))
	{
		altgk.rasAddress.ip = inaddr->sin_addr.s_addr;
		altgk.rasAddress.port = inaddr->sin_port;
		altgk.needToRegister = 1;
		altgk.priority = priority ++;
		altgk.gatekeeperIdentifier.type = cmAliasTypeGatekeeperID;
		altgk.gatekeeperIdentifier.length = utlChr2Bmp("nextone", &gk_id[0]);
		altgk.gatekeeperIdentifier.string = (char *)&gk_id[0];

		if (cmRASSetParam(hsRas, cmRASTrStageConfirm,
				cmRASParamAlternateGatekeeper, altgk.priority, sizeof(altgk),
				(char *)&altgk) < 0)
		{
			NETERROR(MRRQ,
				("%s Failed to add altgk %x/%d\n", fn, altgk.rasAddress.ip,
				altgk.rasAddress.port));
		}
	}

 _return:	
	/* Release the cache */
	CacheReleaseLocks(regCache);

	if (confirm == 1)
	{
		DbScheduleIedgeUpdate(&tmpInfo);
		GisPostCliIedgeRegCmd(tmpInfo.regid, tmpInfo.uport, tags);

		PropagateIedgeIpAddr(tmpInfo.regid, tmpInfo.ipaddress.l);
	}

	return rc;
}

int
GkHandleBRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	char fn[] = "GkHandleBRQ():";
	int rc = 1, bandwidth;

	if (GkCompareGkID(hsRas) < 0)
	{
		 DEBUG(MRRQ, NETLOG_DEBUG4,
				("%s GKID mismatch\n", fn));

		//	rc = -cmRASReasonInvalidPermission;

		 cmRASClose(hsRas);
		 return 0;
	}

	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamBandwidth, 0, 
					  &bandwidth, NULL) < 0)
	{
		 NETERROR(MRRQ, ("%s No Bandwidth found\n", fn));
		 return rc;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Bandwidth = %d\n", fn, bandwidth));

	rc = 1;

	if (cmRASSetParam(hsRas, cmRASTrStageConfirm, 
					  cmRASParamBandwidth, 0, 
					  bandwidth, NULL) > 0)
	{
		NETERROR(MRRQ,
			("%s Could not set Bandwidth to %d\n", fn, bandwidth));
	}

	return rc;
}

int
GkHandleRAI(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	char fn[] = "GkHandleRAI():";
	CacheTableInfo *cacheInfo = 0, *ipCacheInfo = 0;
	NetoidInfoEntry tmpInfo;
	char string[80], endptIdStr[128] = { 0 };
	cmAlias number, alias;
	cmTransportAddress addr, rasaddr;
	int index = 0, ttl = -1, keepAlive = 0, i;
	INT32  tlen;
	BYTE endptId[128];
	int rc = -1, phonesPresent = 0, eptIdPresent = 0;
	PhoNode phonode;
	unsigned char tags[TAGH_LEN] = { 0 };
	char h323id[H323ID_LEN] = { 0 };
	DB db;
	DB_tDb dbstruct;
	int almostOut = 0;
	RealmIP  realmip;

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
		  ("%s Entering, hsRas = %p, hsCall = %p, haCall = %p\n",
			fn, hsRas, hsCall, haCall));

	/* Set up alias */
	alias.string = (char *)&endptId[0];
	alias.length = 128;
	tlen = sizeof(cmAlias);

	/* Extract the endpoint Id */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamEndpointID, 0, &tlen,
					  (char *)&alias) < 0)
	{
		NETERROR(MRRQ,
				  ("%s Could not get endpoint id\n", fn));
		return rc;
	}
	else
	{
		 /* Convert it back to a string */
		 utlBmp2Chr(&endptIdStr[0], alias.string, alias.length);

		 GisExtractRegIdInfo(endptIdStr, &phonode);
	
		 NETDEBUG(MRRQ, NETLOG_DEBUG4,
				  ("%s Received from endpoint %s/%s\n",
					fn, phonode.regid, (char*) ULIPtostring(phonode.ipaddress.l)));
		 eptIdPresent = 1;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %lu, addr = %s\n",
		fn, phonode.realmId, ULIPtostring(phonode.ipaddress.l)));

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
			 ("%s From endpoint %s/%lu\n",
			  fn, phonode.regid, phonode.uport));

	tlen = sizeof(cmTransportAddress);

	// Extract the RAI boolean
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamAlmostOutOfResources, 0, 
					  &almostOut, NULL) < 0)
	{
		 NETERROR(MRRQ, ("%s No AlmostOutOfResources found\n", fn));
		 return rc;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s AlmostOutOfResources = %d\n", fn, almostOut));

	/* We must referesh the iedge cache entry */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	
	realmip.ipaddress = phonode.ipaddress.l;
	realmip.realmId = phonode.realmId;
	ipCacheInfo = CacheGet(ipCache, &realmip);

	if (ipCacheInfo == NULL)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s No cache entry found for this RAI\n", fn));
		goto _release_locks;
	}

	BITA_SET(tags, TAG_DND);

	// DND should be done for all ports registered
	for (i=0; i<MAX_IEDGE_PORTS; i++)
	{
		if (BITA_TEST(ipCacheInfo->data.ports, i))
		{
			phonode.uport = i;
			cacheInfo = CacheGet(regCache, &phonode);

			if (almostOut)
			{
				cacheInfo->data.stateFlags |= CL_DND;
				time(&cacheInfo->data.dndTime);
			}
			else
			{
				cacheInfo->data.stateFlags &= ~CL_DND;
			}

			GisPostCliIedgeRegCmd(cacheInfo->data.regid, cacheInfo->data.uport, 
				tags);
		}
	}
		
	rc = 1;

_release_locks:
	CacheReleaseLocks(regCache);

	if (rc == 1)
	{
//		GisPostCliIedgeRegCmd(tmpInfo.regid, tmpInfo.uport, tags);
	}

	return 1;
}

/* response to a URQ should be always
 * an OK
 */
int
GkHandleURQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	char fn[] = "GkHandleURQ():";
	CacheTableInfo *cacheInfo = 0, *ipCacheInfo = 0;
	NetoidInfoEntry tmpInfo;
	char string[80], endptIdStr[128] = { 0 };
	cmAlias number, alias;
	cmTransportAddress addr, rasaddr;
	int index = 0, ttl = -1, keepAlive = 0;
	INT32  tlen;
	BYTE endptId[128];
	int rc = -1, phonesPresent = 0, eptIdPresent = 0, sigaddrPresent = 0;
	PhoNode phonode;
	unsigned char tags[TAGH_LEN] = { 0 };
	char h323id[H323ID_LEN] = { 0 };
	RealmIP realmip;
	int	sd,realmId;
	cmTransportAddress ouraddr;

	memset( &phonode, (int) 0, sizeof( PhoNode ) );

	/* If the URQ is from a gatekeeper, then we must invoke
	* the endpoint functionality in the iServer
	*/
	NETDEBUG(MRRQ, NETLOG_DEBUG1,
	  ("%s Entering, hsRas = %p, hsCall = %p, haCall = %p\n",
		fn, hsRas, hsCall, haCall));

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
					  cmRASParamSocketDesc, 0, 
					  (int *)&sd, NULL) < 0)
	{
		NETERROR(MRRQ, ("%s No socket\n", fn));
		rc = -1;
		return rc;
	}
	
	if(getQ931RealmInfo(sd,&ouraddr.ip,&ouraddr.port,&realmId) <0)
	{
		NETERROR(MRRQ, ("%s getQ931RealmInfo failed\n", fn));
		rc = -1;
		return rc;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, realmId, sd, ULIPtostring(ouraddr.ip), ouraddr.port));

	/* Unregistration request. Must delete this ip address
	* from the ipaddress cache.
	*/
	BITA_SET(tags, TAG_ACTIVE);

	/* Set up alias */
	alias.string = (char *)&endptId[0];
	alias.length = 128;
	tlen = sizeof(cmAlias);

	/* Extract the endpoint Id */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamEndpointID, 0, &tlen,
					  (char *)&alias) < 0)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
				  ("%s Could not get endpoint id\n", fn));
	}
	else
	{
		 /* Convert it back to a string */
		 utlBmp2Chr(&endptIdStr[0], alias.string, alias.length);

		 GisExtractRegIdInfo(endptIdStr, &phonode);
	
		 NETDEBUG(MRRQ, NETLOG_DEBUG4,
				  ("%s Received from endpoint %s/%lu/%s on realm %lu\n",
					fn, phonode.regid, phonode.uport,
					(char*) ULIPtostring(phonode.ipaddress.l), phonode.realmId));

		 eptIdPresent = 1;
	}

	tlen = sizeof(cmTransportAddress);

	/* Extract the IP address */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamCallSignalAddress, 0, 
					  &tlen, (char *)&addr) < 0)
	{
		// Signaling address MUST be there, but we dont need it
		NETERROR(MRRQ, ("%s No Signalling Address found\n", fn));

		addr.ip = ntohl(srcAddress->ip);
	}
	else
	{
		sigaddrPresent = 1;

		addr.ip = ntohl(addr.ip);

		/* If there are E164 aliases in this unregistration,
	 	* we delete only those otherwise we delete all ports
	 	* refered by this IP
	 	*/
	}

	/* Lock the cache */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	if (eptIdPresent)
	{
		realmip.ipaddress = addr.ip;
		realmip.realmId = realmId;
	}
	else
	{
		realmip.ipaddress = phonode.ipaddress.l;
		realmip.realmId = phonode.realmId;
	}

	ipCacheInfo = (CacheTableInfo *)CacheGet(ipCache, &realmip);

	if (sigaddrPresent && (ipCacheInfo == NULL))
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
					  ("%s No Ip address found for %s\n",
						fn, (char*) ULIPtostring(addr.ip)));
		
		// This signaling address is not valid
		sigaddrPresent = 0;
	}

	if (eptIdPresent || sigaddrPresent)
	{
		// Endpoint
		if (GkCompareGkID(hsRas) < 0)
		{
			 DEBUG(MRRQ, NETLOG_DEBUG4,
					("%s GKID mismatch\n", fn));

			CacheReleaseLocks(regCache);

			rc = 0;
			cmRASClose(hsRas);
			return rc;
		}
	}
	else 
	{
		// MGK
		CacheReleaseLocks(regCache);
		return GkHandleGkURQ(hsRas, srcAddress);
	}

	// Treat this like a GW unregistration

	/* If the endpoint id was there, that must match
	 * with what we have here
	 */
	if (ipCacheInfo && eptIdPresent)
	{
		/* XXX Needs Change for DMR */
		 if ((phonode.ipaddress.l != addr.ip) ||
			 (phonode.realmId != realmId) ||
			 memcmp(phonode.regid, &ipCacheInfo->data, REG_ID_LEN))
		 {
			  /* We have a mismatch, REJECT */
			  NETDEBUG(MRRQ, NETLOG_DEBUG4,	
						("%s Unregistration: Mismatched endpoint id and ip\n", 
						fn));

			  NETDEBUG(MRRQ, NETLOG_DEBUG4,
						("%s endpoint id %s!%s ip %s regid %s\n",
						fn, phonode.regid, (char*) ULIPtostring(phonode.ipaddress.l), 
                         (char*) ULIPtostring(addr.ip),
                         ipCacheInfo->data.regid));

			  rc = -1;

			  goto _return;
		 }
	}

	tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = string;

	while ( (cmRASGetParam(hsRas, cmRASTrStageRequest, 
							 cmRASParamEndpointAlias, index++, 
							 &tlen, (char *)&number)) >= 0)
	{
		if (number.type != cmAliasTypeE164 && number.type != cmAliasTypeH323ID)
		{
			goto _continue;
		}

		if (number.type == cmAliasTypeE164)
		{
			cacheInfo = (CacheTableInfo *)CacheGet(phoneCache, number.string);
		}
		else if (number.type == cmAliasTypeH323ID)
		{
			utlBmp2Chr(&h323id[0], number.string, number.length);
			cacheInfo = (CacheTableInfo *)CacheGet(h323idCache, h323id);
		}   

		if ((cacheInfo) && !IsSGatekeeper(&cacheInfo->data))
		{
			DeleteIedgeIpAddr(ipCache, &cacheInfo->data);

			/* We will confirm the registration */
			phonesPresent = 1;

			/* Set the entry to active, 
			* set the ip address, if its not there,
			* and set it to H323-discovered.
			*/
			cacheInfo->data.stateFlags &= ~CL_ACTIVE;
			BIT_RESET(cacheInfo->data.cap, CAP_IRQ);
			time(&cacheInfo->data.iaTime);

			/* Update the database also */
			DbScheduleIedgeUpdate(&cacheInfo->data);
			GisPostCliIedgeRegCmd(cacheInfo->data.regid, cacheInfo->data.uport, tags);

			rc = 1;
		}

	_continue:
		tlen = sizeof(cmAlias);
		number.length = 80;
		number.string = string;
	}

	/* If no phones were present, we must delete all entries */
	if (phonesPresent == 0)
	{
		if (ipCacheInfo)
		 GkUnregisterAllIedgePortsByIP(0, &ipCacheInfo->data, 1, 0);
		else
		 GkUnregisterAllIedgePortsByIP(0, (NetoidInfoEntry*)&phonode, 1, 0);
	}

 _return:	
	/* Release the cache */
	CacheReleaseLocks(regCache);

	return 1;
}

int
GkHandleGRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	char fn[] = "GkHandleGRQ():";
	cmTransportAddress rasaddr, addr;
	int rc = 1;
	INT32 tlen;
	int sd,realmId = 0;

	GkInsertGkID(hsRas, cmRASTrStageConfirm, gkid);

	rc = GkCompareGkID(hsRas);

	if (rc < 0 || !iserverPrimary)
	{
		cmRASClose(hsRas);
		return 0;
	}

	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamSocketDesc, 0, 
					  &sd, NULL) < 0)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s SocketDesc not found\n", fn));

		rc = -1;

		return rc;
	}

	// This requires the port number also. If the info is not maintained in
	// RSA then another table is needed 
	if(getRasRealmInfo(sd,&addr.ip,&addr.port,&realmId) <0)
	{
		NETERROR(MRRQ, ("%s getRasRealmInfo failed\n", fn));
		rc = -1;
		return rc;
	}

	addr.ip = htonl(addr.ip);

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, realmId, sd, ULIPtostring(addr.ip), addr.port));

	/* Extract the RAS IP address */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamRASAddress, 0, 
					  &tlen, (char *)&rasaddr) < 0)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s No RAS Address found\n", fn));

		rc = -1;

		return rc;
	}

	rasaddr.ip = ntohl(rasaddr.ip);


	if (cmRASSetParam(hsRas, cmRASTrStageConfirm,
			cmRASParamRASAddress,
			0, tlen, (char*)&addr) < 0)
	{
		NETERROR(MH323, ("%s Unable to set RASAddress\n", fn));
	}

	return rc;
}

/* Functions which deal with endpoint aspect */
int
GkSendGRQ(GkInfo *gkInfo, NetoidInfoEntry *infoEntry)
{
	char fn[] = "GkSendGRQ():";
	cmTransportAddress
		 gkAddr = { 0, 0, 1718, cmRASTransportTypeIP },
			addr = { 0, 0, 1719, cmRASTransportTypeIP };
	HRAS hsRas = NULL;
	cmAlias alias;
	BYTE gk_id[128];
	BYTE h323_id[256];
	int index = 0;
	NetoidSNKey *snkey = (NetoidSNKey *)malloc(sizeof(NetoidSNKey));
	int tlen,sd;

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
			 ("%s Sending GRQ for GK %s/%lu Ip %s\n",
			  fn, infoEntry->regid, infoEntry->uport,
			  (char*) ULIPtostring(infoEntry->ipaddress.l)));
			  
	if (getRasInfoFromRealmId(infoEntry->realmId,
			Ras_eSgk,&sd,&addr.ip,&addr.port) <0)
	{
		NETERROR(MRRQ, 
				  ("%s getRasInfoFromRealmId failed\n", fn));
		goto _error;
	}
	
	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, infoEntry->realmId, sd, ULIPtostring(addr.ip), addr.port));

	/* Fill in the RAS address */
	gkAddr.ip = htonl(infoEntry->ipaddress.l);
	gkAddr.port = infoEntry->rasport;
	
	if (gkAddr.port == 0)
	{
		gkAddr.port = 1718;
	}

	if (cmRASStartTransaction(UH323Globals()->hApp,
			(HAPPRAS)snkey, /* Application Ras Handle */
			&hsRas,
			cmRASGatekeeper,
			&gkAddr,
			NULL) < 0)
	{
		NETERROR(MRRQ, 
				  ("%s cmRASStartTransaction failed\n", fn));
		goto _error;
	}	
 
	addr.ip = htonl(addr.ip);
	/* Add our ras address */
	tlen = sizeof(cmTransportAddress);

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamRASAddress, 0, tlen,
		(char*)&addr) < 0)
	{
		NETERROR(MH323,
			("%s Could not set RASAddress \n", fn));
		goto _error;
	}

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamSocketDesc, 0, sd,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set SocketDesc\n", fn));
		goto _error;
	}

	/* Add the gatekeeper id, and the aliases we wish to add */	
	if (strlen(infoEntry->pgkid))
	{
		alias.type = cmAliasTypeGatekeeperID;
		alias.length = utlChr2Bmp(infoEntry->pgkid,
							&gk_id[0]);
		alias.string = (char *)&gk_id[0];

		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Using GKID %s\n",
			fn, infoEntry->pgkid));

		uh323AddRASAlias(hsRas, &alias, &index, cmRASTrStageRequest,
				cmRASParamGatekeeperID);
	}

	index = 0;
#if 0
	alias.type = cmAliasTypeE164;
	alias.string = infoEntry->phone;
	alias.length = strlen(alias.string);
	
	uh323AddRASAlias(hsRas, &alias, &index, 
				cmRASTrStageRequest, cmRASParamTerminalAlias);
#endif
	alias.type = cmAliasTypeH323ID;
	alias.length = utlChr2Bmp(infoEntry->h323id, &h323_id[0]);
	alias.string = &h323_id[0];
	
	DEBUG(MRRQ, NETLOG_DEBUG1,
		("my Alias - %s\n", infoEntry->h323id));

	uh323AddRASAlias(hsRas, &alias, &index, 
				cmRASTrStageRequest, cmRASParamTerminalAlias);

#if 0 // causes a leak in the stack
	/* Set the terminal type */
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamTerminalType, 0, cmRASEndpointTypeGateway,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Discovery\n", fn));
	}
#endif

	/* Set up the SN key */
	memcpy(snkey, infoEntry, sizeof(NetoidSNKey));

	grqAddAuthentication(hsRas);

	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MRRQ, ("%s cmRASRequest failed\n", fn));
		goto _error;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
		  ("%s Started the RAS Transaction successfully\n", 
			fn));

	return 0;

_error:
	free(snkey);	
	if (hsRas)
	{
		cmRASClose(hsRas);
	}
	return -1;
}

int
GkSendURQ(NetoidInfoEntry *entry)
{
	char fn[] = "GkSendURQ():";
	cmTransportAddress
		  eptAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	HRAS hsRas = NULL;
	int tlen,sd;
	int realmId = 0;

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
			  ("%s Sending URQ for endpt %s Ip %s\n",
			   fn, entry->regid, 
			   ULIPtostring(entry->ipaddress.l)));
			   
	realmId = entry->realmId;

	if(getQ931InfoFromRealmId(realmId,Ras_eArq,&sd,&eptAddr.ip,&eptAddr.port) <0)
	{
		NETERROR(MRRQ, 
				  ("%s getQ931InfoFromRealmId failed\n", fn));
		goto _error;
	}

	/* Fill in the RAS address */
	eptAddr.ip = htonl(entry->rasip);
	eptAddr.port = entry->rasport;

	if (cmRASStartTransaction(UH323Globals()->hApp,
							   (HAPPRAS)NULL, /* Application Ras Handle */
							   &hsRas,
							   cmRASUnregistration,
							   &eptAddr,
							   NULL) < 0)
	{
		  NETERROR(MRRQ, 
				   ("%s cmRASStartTransaction failed\n", fn));
		  return -1;
	}	
 
	// Add the signalling address of the endpoint
	/* Add our signalling address */
	tlen = sizeof(cmTransportAddress);

	eptAddr.ip = htonl(entry->ipaddress.l);
	eptAddr.port = entry->callsigport;

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCallSignalAddress, 0, tlen,
		(char*)&eptAddr) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Signalling Address \n", fn));
	}

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamSocketDesc, 0, sd,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set SocketDesc\n", fn));
	}

	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		  cmRASClose(hsRas);
		  NETERROR(MRRQ, ("%s cmRASRequest failed for registration\n", fn));
		  return -1;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
		   ("%s Started the RAS Transaction successfully, hsRas = %p\n", 
			fn, hsRas));

	return 0;

	_error:
		cmRASClose(hsRas);
		return -1;
}

int
GkSendGkURQ(NetoidInfoEntry *entry)
{
	char fn[] = "GkSendGkURQ():";
	cmTransportAddress
		  eptAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	HRAS hsRas = NULL;
	BYTE gk_id[128];
	cmTransportAddress addr = { 0, 0, 1720, cmRASTransportTypeIP };
	cmAlias alias, endpointID;
	CacheGkInfo *cacheGkInfo = NULL;
	int tlen, index = 0,sd,realmId = 0;

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
			  ("%s Sending URQ for endpt %s Ip %s\n",
			   fn, entry->regid, 
			   ULIPtostring(entry->ipaddress.l)));
			   
	realmId = entry->realmId;

	if(getQ931InfoFromRealmId(realmId,Ras_eSgk,&sd,&addr.ip,&addr.port) <0)
	{
		NETERROR(MRRQ, 
				  ("%s getQ931InfoFromRealmId failed\n", fn));
		goto _error;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, realmId, sd, ULIPtostring(addr.ip), addr.port));

	/* Fill in the RAS address */
	eptAddr.ip = htonl(entry->ipaddress.l);
	eptAddr.port = 1719;

	if (cmRASStartTransaction(UH323Globals()->hApp,
							   (HAPPRAS)NULL, /* Application Ras Handle */
							   &hsRas,
							   cmRASUnregistration,
							   &eptAddr,
							   NULL) < 0)
	{
		  NETERROR(MRRQ, 
				   ("%s cmRASStartTransaction failed\n", fn));
		  return -1;
	}	
 
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	// We must fill in endpoint id, call sig address,
	// gkid, and reason

	cacheGkInfo = CacheGet(gkCache, entry);

	if (cacheGkInfo == NULL)
	{
		NETERROR(MRRQ,
			("%s Could not locate gk cache entry in cache\n",
			fn));
		goto _error;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s URQ to %s/%lu\n", fn, entry->regid, entry->uport));

	if ((cacheGkInfo->endpointIDLen <= 0) ||
		(cacheGkInfo->endpointIDLen > ENDPOINTID_LEN))
	{
		NETERROR(MRRQ, ("%s endpointID from gk %s/%lu is invalid\n",
			fn, entry->regid, entry->uport));
	}

	/* Add the endpoint ID */
	endpointID.type = cmAliasTypeEndpointID;
	endpointID.length = cacheGkInfo->endpointIDLen;
	endpointID.string = cacheGkInfo->endpointIDString;

	if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamEndpointID, 0, sizeof(cmAlias), 
			(char *)&endpointID) < 0)
	{
		NETERROR(MRRQ, 
			("%s cmRASSetParam cmRASParamEndpointID failed\n", 
			fn));
	}

	CacheReleaseLocks(regCache);


	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamSocketDesc, 0, sd,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set SocketDesc\n", fn));
	}


	/* Add our signalling address */
	tlen = sizeof(cmTransportAddress);

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCallSignalAddress, 0, tlen,
		(char*)&addr) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Signalling Address \n", fn));
	}

	/* Add the gatekeeper id, and the aliases we wish to add */	
	if (strlen(entry->pgkid))
	{
		alias.type = cmAliasTypeGatekeeperID;
		alias.length = utlChr2Bmp(entry->pgkid,
							&gk_id[0]);
		alias.string = (char *)&gk_id[0];

		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Using GKID %s\n", fn, entry->pgkid));

		uh323AddRASAlias(hsRas, &alias, &index, cmRASTrStageRequest,
				cmRASParamGatekeeperID);
	}

	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		  cmRASClose(hsRas);
		  NETERROR(MRRQ, ("%s cmRASRequest failed for URQ\n", fn));
		  return -1;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
		   ("%s Started the RAS Transaction successfully, hsRas = %p\n", 
			fn, hsRas));

	return 0;

_error:

	if(hsRas)
		cmRASClose(hsRas);
	CacheReleaseLocks(regCache);

	return -1;
}

int
GkSendRRQ(GkInfo *gkInfo, NetoidInfoEntry *infoEntry, int light)
{
	char fn[] = "GkSendRRQ():";
	cmTransportAddress
		 gkAddr = { 0, 0, 1719, cmRASTransportTypeIP },
			addr = { 0, 0, 1720, cmRASTransportTypeIP };
	HRAS hsRas = NULL;
	cmAlias alias, endpointID;
	BYTE gk_id[128];
	BYTE h323_id[256];
	int index = 0, tlen;
	NetoidSNKey *snkey = (NetoidSNKey *)malloc(sizeof(NetoidSNKey));
	int nodeId;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	int prefixlen;
	int sd,realmId = 0;

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
			 ("%s Sending RRQ for GK %s/%lu Ip %s\n",
			  fn, infoEntry->regid, infoEntry->uport,
			  (char*) ULIPtostring(infoEntry->ipaddress.l)));
			  
	realmId = infoEntry->realmId;

	if(getQ931InfoFromRealmId(realmId,Ras_eSgk,&sd,&addr.ip,&addr.port) <0)
	{
		NETERROR(MRRQ, 
				  ("%s getQ931InfoFromRealmId failed\n", fn));
		goto _error;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, realmId, sd, ULIPtostring(addr.ip), addr.port));

	/* Fill in the RAS address */
	gkAddr.ip = htonl(infoEntry->ipaddress.l);
	if (infoEntry->rasport != 0)
	{
		gkAddr.port = infoEntry->rasport;
	}

	if (cmRASStartTransaction(UH323Globals()->hApp,
			(HAPPRAS)snkey, /* Application Ras Handle */
			&hsRas,
			cmRASRegistration,
			&gkAddr,
			NULL) < 0)
	{
		NETERROR(MRRQ, 
				  ("%s cmRASStartTransaction failed\n", fn));

		goto _error;
	}	
 
	if ((gkInfo->flags & GKCAP_LTWTREG) && light)
	{
		/* Add the keep alive */
		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamKeepAlive, 0, TRUE, NULL) < 0)
		{
			NETERROR(MH323,
				("%s Could not set Keep Alive\n", fn));
		}

		if ((gkInfo->endpointIDLen <= 0) ||
			(gkInfo->endpointIDLen > ENDPOINTID_LEN))
		{
			NETERROR(MRRQ, ("%s endpointID from gk %s/%lu is invalid\n",
				fn, infoEntry->regid, infoEntry->uport));
		}

		/* Add the endpoint ID */
		endpointID.type = cmAliasTypeEndpointID;
		endpointID.length = gkInfo->endpointIDLen;
		endpointID.string = 
			gkInfo->endpointIDString;

		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamEndpointID, 0, sizeof(cmAlias), 
			(char *)&endpointID) < 0)
		{
			NETERROR(MRRQ, 
				("%s cmRASSetParam cmRASParamEndpointID failed\n", 
				fn));
		}
	}

	if (gkInfo->regttl > 0)
	{
		/* Add the ttl */
		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamTimeToLive, 0, gkInfo->regttl,
			NULL) < 0)
		{
			NETERROR(MH323,
				("%s Could not set TTL\n", fn));
		}
		else
		{
			NETDEBUG(MRRQ, NETLOG_DEBUG4,
				("%s Used TTL = %d\n", 
				fn, gkInfo->regttl));
		}
	}

	/* Say Discovery is is Complete */
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamDiscoveryComplete, 0, TRUE,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Discovery\n", fn));
	}

	addr.ip = htonl(addr.ip);
	// addr = UH323Globals()->sigAddr;

	/* Add our signalling address */
	tlen = sizeof(cmTransportAddress);

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCallSignalAddress, 0, tlen,
		(char*)&addr) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Signalling Address \n", fn));
		goto _error;
	}

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamSocketDesc, 0, sd,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set SocketDesc\n", fn));
		goto _error;
	}


#if 0
	/* Set the terminal type */
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamTerminalType, 0, cmRASEndpointTypeGateway,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Discovery\n", fn));
	}
#endif

	/* Add our ras address */
	if(getRasInfoFromRealmId(realmId,Ras_eSgk,&sd,&addr.ip,&addr.port) <0)
	{
		NETERROR(MRRQ, 
				  ("%s getRasInfoFromRealmId failed\n", fn));
		goto _error;
	}

	addr.ip = htonl(addr.ip);
	tlen = sizeof(cmTransportAddress);
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamRASAddress, 0, tlen,
		(char*)&addr) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Signalling Address \n", fn));
	}

	/* Add the gatekeeper id, and the aliases we wish to add */	
	if (strlen(infoEntry->pgkid))
	{
		alias.type = cmAliasTypeGatekeeperID;
		alias.length = utlChr2Bmp(infoEntry->pgkid,
							&gk_id[0]);
		alias.string = (char *)&gk_id[0];

		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Using GKID %s\n",
			fn, infoEntry->pgkid));

		uh323AddRASAlias(hsRas, &alias, &index, cmRASTrStageRequest,
				cmRASParamGatekeeperID);
	}

	index = 0;
#if 0
	alias.type = cmAliasTypeE164;
	alias.string = infoEntry->phone;
	alias.length = strlen(alias.string);
	
	uh323AddRASAlias(hsRas, &alias, &index, 
				cmRASTrStageRequest, cmRASParamTerminalAlias);
#endif

	alias.type = cmAliasTypeH323ID;
	alias.length = utlChr2Bmp(infoEntry->h323id, &h323_id[0]);
	alias.string = &h323_id[0];
	
	DEBUG(MRRQ, NETLOG_DEBUG1,
		("my Alias - %s\n", infoEntry->h323id));

	uh323AddRASAlias(hsRas, &alias, &index, 
				cmRASTrStageRequest, cmRASParamTerminalAlias);

	// Add the tech prefix for registration
	if (prefixlen = strlen(infoEntry->techprefix))
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Using techprefix %s\n", fn, infoEntry->techprefix));

		nodeId = cmGetProperty((HPROTOCOL)hsRas);
		if (pvtBuildByPath(hVal, nodeId,
			"request.registrationRequest.terminalType.gateway.protocol.1.voice.supportedPrefixes.1.prefix.e164",
			prefixlen, infoEntry->techprefix) < 0)
		{
			NETERROR(MH323, ("%s could not build prefix\n", fn));
		}
	}

	/* Set up the SN key */
	memcpy(snkey, infoEntry, sizeof(NetoidSNKey));

	rrqAddAuthentication(hsRas,infoEntry->h323id,infoEntry->passwd);

	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MRRQ, ("%s cmRASRequest failed\n", fn));
		goto _error;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
		  ("%s Started the RAS Transaction successfully\n", 
			fn));

	return 0;

_error:
	if (hsRas)
	{
		cmRASClose(hsRas);
	}
	// If an error happens we treat it like an RRJ
	return GkHandleRRJ((HAPPRAS)snkey, 0, 0);
//	free(snkey);
//	return -1;
}

int
GkSendRAI(GkInfo *gkInfo, NetoidInfoEntry *infoEntry)
{
	char fn[] = "GkSendRAI():";
	cmTransportAddress
		 gkAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	HRAS hsRas = NULL;
	cmAlias alias, endpointID;
	BYTE gk_id[128];
	BYTE h323_id[256];
	int index = 0, tlen;
	NetoidSNKey *snkey = (NetoidSNKey *)malloc(sizeof(NetoidSNKey));
	int avail = 1;
	int sd,realmId = 0;
	cmTransportAddress
			addr = { 0, 0, 1720, cmRASTransportTypeIP };

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
			 ("%s Sending RAI for GK %s/%lu Ip %s\n",
			  fn, infoEntry->regid, infoEntry->uport,
			  (char*) ULIPtostring(infoEntry->ipaddress.l)));
			  
	/* Fill in the RAS address */
	gkAddr.ip = htonl(infoEntry->ipaddress.l);

	realmId = infoEntry->realmId;

	if (cmRASStartTransaction(UH323Globals()->hApp,
			(HAPPRAS)snkey, /* Application Ras Handle */
			&hsRas,
			cmRASResourceAvailability,
			&gkAddr,
			NULL) < 0)
	{
		NETERROR(MRRQ, 
				  ("%s cmRASStartTransaction failed\n", fn));

		goto _error;
	}	
 
	/* Add our ras address */
	if(getRasInfoFromRealmId(realmId,Ras_eSgk,&sd,&addr.ip,&addr.port) <0)
	{
		NETERROR(MRRQ, 
				  ("%s getRasInfoFromRealmId failed\n", fn));
		goto _error;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, realmId, sd, ULIPtostring(addr.ip), addr.port));

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamSocketDesc, 0, sd,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set SocketDesc\n", fn));
		goto _error;
	}


	if ((gkInfo->endpointIDLen <= 0) ||
		(gkInfo->endpointIDLen > ENDPOINTID_LEN))
	{
		NETERROR(MRRQ, ("%s endpointID from gk %s/%lu is invalid\n",
			fn, infoEntry->regid, infoEntry->uport));
	}

	/* Add the endpoint ID */
	endpointID.type = cmAliasTypeEndpointID;
	endpointID.length = gkInfo->endpointIDLen;
	endpointID.string = 
		gkInfo->endpointIDString;

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamEndpointID, 0, sizeof(cmAlias), 
		(char *)&endpointID) < 0)
	{
		NETERROR(MRRQ, 
			("%s cmRASSetParam cmRASParamEndpointID failed\n", 
			fn));
	}

	if ((IedgeXCalls(infoEntry) &&
		((double)IedgeCalls(infoEntry) >= 0.9*((double)IedgeXCalls(infoEntry)))) ||
		(IedgeXInCalls(infoEntry) &&
		((double)IedgeInCalls(infoEntry) >= 0.9*((double)IedgeXInCalls(infoEntry)))) ||
		IsIgrpQuotaFull(infoEntry) ||
		(UH323DetermineBestSigAddr(0) < 0))
	{
		avail = 0;
	}

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamAlmostOutOfResources, 0, 
		avail?FALSE:TRUE, NULL) < 0)
	{
		NETERROR(MRRQ, 
			("%s cmRASSetParam cmRASParamAlmostOutOfResources failed\n", 
			fn));
	}

	/* Set up the SN key */
	memcpy(snkey, infoEntry, sizeof(NetoidSNKey));

	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MRRQ, ("%s cmRASRequest failed\n", fn));
		goto _error;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG1,
		  ("%s Started the RAS Transaction successfully\n", 
			fn));

	return 0;

_error:
	if (hsRas)
	{
		cmRASClose(hsRas);
	}
	free(snkey);
	return -1;
}

int
GkHandleRAIResponse(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		char 				*msg
)
{
	static char fn[] = "GkHandleRAIResponse():";
	NetoidSNKey *snkey = (NetoidSNKey *)haRas;

	if (!snkey)
	{
		NETERROR(MRRQ, ("%s no app data in RAI Response!\n", fn));
		return -1;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Received %s from %s/%d\n",
		fn, msg?msg:"NULL", snkey->regid, snkey->uport));

	free(snkey);

	return 0;
}

int
GkHandleGCF(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	static char fn[] = "GkHandleGCF():";
	NetoidSNKey *snkey = (NetoidSNKey *)haRas;
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry;	
	CacheGkInfo *cacheGkInfo = NULL, cacheGkInfoEntry;

	if (snkey == NULL)
	{
		NETERROR(MRRQ, ("%s snkey is NULL\n", fn));
		return -1;
	}
	
	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Received GCF from %s/%d\n", 
		fn, snkey->regid, snkey->uport));

	cacheInfo = &cacheInfoEntry;
	if (CacheFind(regCache, snkey, cacheInfo,
					sizeof(CacheTableInfo)) < 0)
	{
		NETERROR(MARQ, 
			("%s No entry found in regCache for gk %s/%d\n",
			 fn, snkey->regid, snkey->uport));
		return -1;
	}
	
	// Check the GkID in the response to what we sent
	free(snkey);

#if 0
	if (GkCheckGkID(hsRas, cmRASTrStageConfirm, cacheInfo->data.pgkid) < 0)
	{
		// mismatch
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Gk ID in GCF does not match what we sent %s\n",
			fn, cacheInfo->data.pgkid));
		return 0;
	}
#endif

	/* Overwrite the cache */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheGkInfo = CacheGet(gkCache, &cacheInfo->data);

	if (cacheGkInfo == NULL)
	{
		NETERROR(MARQ, 
			("%s No entry found in gkCache for gk %s/%lu\n",
			 fn, cacheInfo->data.regid, cacheInfo->data.uport));
		return -1;
	}
	/* Set the state */
	cacheGkInfo->regState = GKREG_DISCOVERED;
	memcpy(&cacheGkInfoEntry, cacheGkInfo, 
		sizeof(CacheGkInfo));
	
	CacheReleaseLocks(regCache);

	/* Now send an RRQ to this ep */
	GkSendRRQ(&cacheGkInfoEntry, &cacheInfo->data, 0);

	NETDEBUG(MRRQ, NETLOG_DEBUG4, 
		("%s Successful for  Gk (%s/%lu)\n",
		fn,cacheGkInfo->regid,cacheGkInfo->uport));

	return 0;
}

int
GkHandleRCF(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	char fn[] = "GkHandleRCF():";
	NetoidSNKey *snkey = (NetoidSNKey *)haRas;
	CacheTableInfo *cacheInfo = NULL;
	CacheGkInfo *cacheGkInfo = NULL;
	cmAlias alias, endpointID;
	int len;
	char	epstr[ENDPOINTID_LEN];
	char str[24];
	char tags[TAGH_LEN] = { 0 };

	if (snkey == NULL)
	{
		NETERROR(MRRQ, ("%s snkey is NULL\n", fn));
		return -1;
	}
	
	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Received RCF from %s/%d\n", 
		fn, snkey->regid, snkey->uport));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, snkey);

	if (cacheInfo == NULL)
	{
		NETERROR(MRRQ, 
			("%s Could not locate cache entry for gk %s/%d\n", 
			fn, snkey->regid, snkey->uport));
		goto _error;
	}
	
#if 0
	if (GkCheckGkID(hsRas, cmRASTrStageConfirm, cacheInfo->data.pgkid) < 0)
	{
		// mismatch
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Gk ID in RCF does not match what we sent %s\n",
			fn, cacheInfo->data.pgkid));
		return 0;
	}
#endif

	time(&cacheInfo->data.rTime);
	cacheInfo->data.stateFlags |= CL_ACTIVE;
	cacheInfo->data.stateFlags |= CL_REGISTERED;

	DbScheduleIedgeUpdate(&cacheInfo->data);

	// Also we should make sure that the ip address is in
	if (AddIedgeIpAddr(ipCache, cacheInfo) < 0)
	{
		NETERROR(MRRQ, ("%s Unable to add ip address %s in the cache\n",
			fn, FormatIpAddress(cacheInfo->data.ipaddress.l, str)));
	}

	if (crids)
	{
		// Add into the crId cache
		// Delete any old one first
		CacheDelete(cridCache, &cacheInfo->data.crId);
		if (CacheInsert(cridCache, cacheInfo) < 0)
		{
			NETERROR(MRRQ, 
				("%s Unable to add crid %d in the cache\n",
				fn, cacheInfo->data.crId));
		}
	}

	cacheGkInfo = CacheGet(gkCache, &cacheInfo->data);

	/* Extract the endpoint ID, and the TTL */
	memset(cacheGkInfo->endpointIDString, 0, ENDPOINTID_LEN);
	cacheGkInfo->endpointIDLen = 0;
	
	len = sizeof(cmAlias);

	alias.type = cmAliasTypeEndpointID;
	alias.string = cacheGkInfo->endpointIDString;
	alias.length = ENDPOINTID_LEN;

	if (cmRASGetParam(hsRas, cmRASTrStageConfirm,
		cmRASParamEndpointID,
		0,
		&len,
		(char *)&alias) < 0)
	{
		NETERROR(MRRQ,
			("%s Unable to extract endpoint id\n", fn));
	}
	else
	{
		cacheGkInfo->endpointIDLen = alias.length;
		utlBmp2Chr(epstr, cacheGkInfo->endpointIDString, alias.length);

		NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s endpointId = %s len = %d\n", 
		fn,epstr,cacheGkInfo->endpointIDLen));
	
	}

	if ((cacheGkInfo->endpointIDLen <= 0) ||
		(cacheGkInfo->endpointIDLen > ENDPOINTID_LEN))
	{
		NETERROR(MRRQ, ("%s Bad endpointID rxed from gk %s/%d\n",
			fn, snkey->regid, snkey->uport));
	}

	if (cmRASGetParam(hsRas, cmRASTrStageConfirm,
		cmRASParamTimeToLive, 0, &cacheGkInfo->regttl, 
		NULL) < 0)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Could not extract TTL\n", fn));

		cacheGkInfo->flags &= ~GKCAP_LTWTREG;
		cacheGkInfo->regttl = 0;
	}
	else
	{
		cacheGkInfo->flags |= GKCAP_LTWTREG;
	}

	/* Set state to registered */
	cacheGkInfo->regState = GKREG_REGISTERED;
	
	cacheGkInfo->regttl = GkComputeSchedule(cacheGkInfo);

	NETDEBUG(MRRQ, NETLOG_DEBUG4, 
		("%s Successful for %s/%lu \n",
		fn,cacheGkInfo->regid,cacheGkInfo->uport));

	BITA_SET(tags, TAG_REGSTATUS);
	BITA_SET(tags, TAG_GKREGTTL);
	BITA_SET(tags, TAG_EPID);
	BITA_SET(tags, TAG_GKFLAGS);

	if (SrcARQEnabled())
	{
		nx_strlcpy(sgkSN.regid, cacheGkInfo->regid, REG_ID_LEN);
		sgkSN.uport = cacheGkInfo->uport;
	}
	GisPostCliGkRegCmd(snkey->regid, snkey->uport, tags);

_error:
	free(snkey);

	CacheReleaseLocks(regCache);

	return 0;
}

int
GkHandleGRJ(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	char fn[] = "GkHandleGRJ():";
	NetoidSNKey *snkey = (NetoidSNKey *)haRas;
	CacheTableInfo *cacheInfo = NULL;
	CacheGkInfo *cacheGkInfo = NULL;
	char tags[TAGH_LEN] = { 0 };

	/* Go back to init state */

	if (snkey == NULL)
	{
		NETERROR(MRRQ, ("%s snkey is NULL\n", fn));
		return -1;
	}
	
	if (IServerIsSecondary())
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4, 
			("%s Skipping, we are secondary", fn));
		goto _return;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4, 
		("%s Received for %s/%d\n", fn, snkey->regid, snkey->uport));

	BITA_SET(tags, TAG_REGSTATUS);

	/* Look up the iedge and send an RRQ for it */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, snkey);

	if (cacheInfo == NULL)
	{
		NETERROR(MRRQ, 
			("%s Could not locate cache entry for gk %s/%d\n", 
			fn, snkey->regid, snkey->uport));
		goto _error;
	}
	
#if 0
	if ((reason >= 0) &&
		(GkCheckGkID(hsRas, cmRASTrStageReject, cacheInfo->data.pgkid) < 0))
	{
		// mismatch
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Gk ID in GRJ does not match what we sent %s\n",
			fn, cacheInfo->data.pgkid));
		return 0;
	}
#endif

	cacheInfo->data.stateFlags &= ~CL_ACTIVE;
	time(&cacheInfo->data.iaTime);
	cacheInfo->data.stateFlags &= ~CL_REGISTERED;

	DbScheduleIedgeUpdate(&cacheInfo->data);

	cacheGkInfo = CacheGet(gkCache, snkey);
	if (cacheGkInfo)
	{
		cacheGkInfo->regState = GKREG_TRYALT;
		//cacheGkInfo->regState = GKREG_IDLE;
		cacheGkInfo->nextEvent = time(NULL);

		memset(cacheGkInfo->endpointIDString, 0, ENDPOINTID_LEN);
		cacheGkInfo->endpointIDLen = 0;
	
		GkCheckAltGk(cacheGkInfo, 0);
	}
	else
	{
		NETERROR(MRRQ, ("%s No cache entry found for %s/%d\n",
			fn, snkey->regid, snkey->uport));
	}
	
	if (SrcARQEnabled())
	{
		memset(sgkSN.regid, 0, REG_ID_LEN);
		sgkSN.uport = 0;
	}

	GisPostCliGkRegCmd(snkey->regid, snkey->uport, tags);

_error:
	CacheReleaseLocks(regCache);

_return:
	free(snkey);

	return 0;
}

// hsRas may be NULL, depending on context
int
GkHandleRRJ(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	char fn[] = "GkHandleRRJ():";
	NetoidSNKey *snkey = (NetoidSNKey *)haRas;
	CacheTableInfo *cacheInfo = NULL;
	CacheGkInfo *cacheGkInfo = NULL;
	char tags[TAGH_LEN] = { 0 };

	/* Go back to init state */

	if (snkey == NULL)
	{
		NETERROR(MRRQ, ("%s snkey is NULL\n", fn));
		return -1;
	}
	
	if (IServerIsSecondary())
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4, 
			("%s Skipping, we are secondary", fn));
		goto _return;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4, 
		("%s Received for %s/%d\n", fn, snkey->regid, snkey->uport));

	/* Look up the iedge and send an RRQ for it */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, snkey);

	if (cacheInfo == NULL)
	{
		NETERROR(MRRQ, 
			("%s Could not locate cache entry for gk %s/%d\n", 
			fn, snkey->regid, snkey->uport));
		goto _error;
	}
	
	if ((reason == -1) && (cacheInfo->data.stateFlags & CL_ACTIVE))
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s RRQ Timeout for %s/%lu, sending URQ\n", 
			fn, cacheInfo->data.regid, cacheInfo->data.uport));

		GkSendGkURQ(&cacheInfo->data);
	}

	cacheInfo->data.stateFlags &= ~CL_ACTIVE;
	cacheInfo->data.stateFlags &= ~CL_REGISTERED;
	time(&cacheInfo->data.iaTime);

	DeleteIedgeIpAddr(ipCache, &cacheInfo->data);
	
	if (crids)
	{
		// Delete from crid cache also
		CacheDelete(cridCache, &cacheInfo->data.crId);
	}

	DbScheduleIedgeUpdate(&cacheInfo->data);

	cacheGkInfo = CacheGet(gkCache, snkey);
	if (cacheGkInfo)
	{
		cacheGkInfo->regState = GKREG_TRYALT;
		cacheGkInfo->nextEvent = time(NULL);

		memset(cacheGkInfo->endpointIDString, 0, ENDPOINTID_LEN);
		cacheGkInfo->endpointIDLen = 0;
	
		// Check to see if the next uport in sequence is an
		// alternate gatekeeper
		GkCheckAltGk(cacheGkInfo, 0);
	}
	else
	{
		NETERROR(MRRQ, ("%s No cache entry found for %s/%d\n",
			fn, snkey->regid, snkey->uport));
	}
	
	BITA_SET(tags, TAG_REGSTATUS);

	if (SrcARQEnabled())
	{
		memset(sgkSN.regid, 0, REG_ID_LEN);
		sgkSN.uport = 0;
	}

	GisPostCliGkRegCmd(snkey->regid, snkey->uport, tags);

_error:
	CacheReleaseLocks(regCache);

_return:
	free(snkey);

	return 0;
}

int
GkHandleGkURQ(
		IN	HRAS			hsRas,
		IN	cmTransportAddress	*srcAddress)
{
	char fn[] = "GkHandleGkURQ():";
	CacheTableInfo *cacheInfo = NULL, *info = NULL;
	CacheGkInfo *cacheGkInfo = NULL;
	InfoEntry *infoEntry = NULL;
	PhoNode phonode;
	RealmIP realmip;
	int i;
	char tags[TAGH_LEN] = { 0 };
	int	sd,realmId;
	cmTransportAddress ouraddr;

	/* Based on what the source is, we will have to locate
	* it using ip address in our cache, go to idle state
	* for it.
	*/
	srcAddress->ip = ntohl(srcAddress->ip);

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s URQ from %s/%d\n",
		fn, (char*) ULIPtostring(srcAddress->ip), srcAddress->port));

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
					  cmRASParamSocketDesc, 0, 
					  (int *)&sd, NULL) < 0)
	{
		NETERROR(MRRQ, ("%s No socket\n", fn));
		return -1;
	}
	
	if(getQ931RealmInfo(sd,&ouraddr.ip,&ouraddr.port,&realmId) <0)
	{
		NETERROR(MRRQ, ("%s getQ931RealmInfo failed\n", fn));
		return -1;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Realm = %d, sd = %d, addr = %s/%d\n",
		fn, realmId, sd, ULIPtostring(ouraddr.ip), ouraddr.port));

	/* Always Confirm a URQ ... */
	CacheGetLocks(ipCache, LOCK_WRITE, LOCK_BLOCK);

	realmip.ipaddress = srcAddress->ip;
	realmip.realmId = realmId;
	cacheInfo = CacheGet(ipCache, &realmip);

	if (cacheInfo == NULL)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Could not locate source of URQ in cache\n",
			fn));
		goto _error;
	}

	BITA_SET(tags, TAG_REGSTATUS);

	infoEntry = &cacheInfo->data;

	memcpy(phonode.regid, infoEntry->regid, REG_ID_LEN);
	BIT_SET(phonode.sflags, ISSET_REGID);

	for (i=0; i<MAX_IEDGE_PORTS; i++)
	{
		if (BITA_TEST(infoEntry->ports, i))
		{
			/* Look for this iedge port */
			phonode.uport = i;
	 		BIT_SET(phonode.sflags, ISSET_UPORT);

			info = (CacheTableInfo *)CacheGet(regCache, &phonode);

			if (info && IsSGatekeeper(&info->data))
			{
				NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("%s regid %s/%lu\n", fn, info->data.regid, 
					info->data.uport));

				/* This iedge port is to be aged now */

				/* We must mark in the db that this
				 * netoid is inactive now
				 */
				info->data.stateFlags &= ~(CL_ACTIVE);
				time(&info->data.iaTime);

				DeleteIedgeIpAddr(ipCache, &info->data);

	 			/* Update the database also */
				DbScheduleIedgeUpdate(&info->data);

				cacheGkInfo = CacheGet(gkCache, &info->data);

				if (cacheGkInfo == NULL)
				{
					NETERROR(MRRQ,
						("%s Could not locate gk cache entry in cache\n",
						fn));

					continue;
				}

				cacheGkInfo->regState = GKREG_TRYALT;
				cacheGkInfo->nextEvent = time(NULL);

				memset(cacheGkInfo->endpointIDString, 0, ENDPOINTID_LEN);
				cacheGkInfo->endpointIDLen = 0;
	
				GkCheckAltGk(cacheGkInfo, 0);

				if (crids)
				{
					// Delete from crid cache also
					CacheDelete(cridCache, &info->data.crId);
				}

				GisPostCliGkRegCmd(info->data.regid, info->data.uport, tags);
			}
		}
	}

	CacheReleaseLocks(ipCache);
	return 1;

_error:
	CacheReleaseLocks(ipCache);
	return -1;
}

// If there is an alternate GK which is different from the one
// in gkInfo, schedule it. 
// For restarts, change state of all gks in the list
// For other cases, change the state of only the next alternate gk.
// For this fn to execute, the gkInfo input must be in the TRYINGALT
// state
// looparound will determine if we can actually loop around
// to the first gatekeeper, which may happen to be the same as gkInfo
// looparound should be turned on only at Poll time
int
GkCheckAltGk(CacheGkInfo *gkInfo, int looparound)
{
	char fn[] = "GkCheckAltGk():";
	CacheGkInfo **gkInfoPtr = 0, **gkInfoNextPtr, **gkInfoPrevPtr;
	
	if (gkInfo == NULL)
	{
		NETERROR(MRRQ, ("%s Null entry \n", fn));
		return -1;
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s Looking for alternate GK for %s/%ld\n",fn,
		gkInfo->regid, gkInfo->uport));

	if (gkInfo->regState != GKREG_TRYALT)
	{
		// Failure is due to some other reason.
		return 0;
	}

	// Check to see if there is a next uport in sequence
	gkInfoPtr = (CacheGkInfo **)CacheGetFast(gkCache, gkInfo->regid);
	
	if (gkInfoPtr == NULL)
	{
		// This is a fatal error
		NETERROR(MRRQ, ("%s GetFast failed for %s/%lu\n",
			fn, gkInfo->regid, gkInfo->uport));
		return -1;
	}

	// gkInfo will be REDETERMINED, by the algo below:
	gkInfo = NULL;

	// If there is a next one in the altgk sequence,
	// find it first. This is irrespective of the looparound
	// option
	gkInfoNextPtr = (CacheGkInfo **)CacheGetNextFast(gkCache, (void **)gkInfoPtr);

	if (gkInfoNextPtr &&
		!memcmp((*gkInfoPtr)->regid, (*gkInfoNextPtr)->regid, REG_ID_LEN))
	{
		// This is the next one we should look at.
		gkInfoPtr = gkInfoNextPtr;
		gkInfo = *gkInfoPtr;	
		gkInfo->regState = GKREG_UNKNOWN;
	}
	else if (looparound)
	{
		// execute this scenario only when we are permitted
		// to loop around, we are allowed to loop around
		// to the same entry as input
		// different regid, skip to the first one
	
		while (gkInfoPtr)
		{

			gkInfo = *gkInfoPtr;
			gkInfo->regState = GKREG_UNKNOWN;	

			if (!(gkInfoPrevPtr = 
				(CacheGkInfo **)CacheGetPrevFast(gkCache, (void **)gkInfoPtr)))
			{
				break;
			}

			if (memcmp((*gkInfoPtr)->regid, (*gkInfoPrevPtr)->regid, REG_ID_LEN))
			{
				// this is not the same regid
				break;
			}

			// We can safely assign
			gkInfoPtr = gkInfoPrevPtr;
		}
	}

	if (gkInfo == NULL)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4, 
			("%s Redetermination of gkInfo unsuccesul\n", fn));
		return -1;
	}

	// Re - initiate
	if (GkScheduled(gkInfo) == 0)
	{
		// succesfully scheduled this regid
		// This one was scheduled, so copy the regid
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s succesfully re-started gk sequence on %s/%lu\n",
			fn, gkInfo->regid, gkInfo->uport));
		return 0;
	}

	return -1;
}

int
GkPoll(struct Timer* t)
{
	char fn[] = "GkPoll():";
	CacheGkInfo *gkInfo = 0, *lastgkInfo = 0;
	static time_t presentTime;
	char regid[REG_ID_LEN] = { 0 };
	int success = 0;

	NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Polling Gk Cache\n", fn));

	if (IServerIsSecondary())
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Skipping, we are secondary\n", fn));
		return 0;
	}

	/* Walk the Gk Cache */

	presentTime = time(0);

	if (cmMeiEnter(UH323Globals()->hApp) < 0)
	{
		NETERROR(MH323, ("uh323Stack locks failed!!!\n"));
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (gkInfo = CacheGetFirst(gkCache);
			gkInfo; gkInfo = CacheGetNext(gkCache, gkInfo))
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Found gkEntry (%s/%lu)\n",
			fn,gkInfo->regid,gkInfo->uport));

		if (crids && (gkInfo->crId == iservercrId))
		{
			NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s crId = %d\n",
				fn, gkInfo->crId ));
			continue;
		}

		// If we succesfully schedule an event on a particular
		// regid, then we move on to the nxt regid, w/o going through
		// the ports of this regid anymore.
		if (!memcmp(regid, gkInfo->regid, REG_ID_LEN))
		{
			if (success)
			{
				NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Skipping until new regid\n",
					fn));
				continue;
			}
		}
		else
		{
			// new regid
			if (!success && lastgkInfo)
			{
				GkCheckAltGk(lastgkInfo, 1);
			}

			success = 0;
			memcpy(regid, gkInfo->regid, REG_ID_LEN);
		}

		lastgkInfo = gkInfo;

		if (IsGkScheduled(gkInfo->nextEvent, presentTime))
		{
			NETDEBUG(MRRQ, NETLOG_DEBUG4, 
				("%s Scheduling Gk (%s/%lu) \n", 
				fn, gkInfo->regid, gkInfo->uport));

			if (GkScheduled(gkInfo) == 0)
			{
				// succesfully scheduled this regid
				// This one was scheduled, so copy the regid
				success = 1;
			}

		}
		else
		{
			// Time hasnt arrived ?? means success
			NETDEBUG(MRRQ, NETLOG_DEBUG4,
				("%s Gk (%s/%lu) not ready to be scheduled yet\n",
				fn, gkInfo->regid, gkInfo->uport));

			success = 1;
		}
	}

	// for the last regid in the gk cache
	if (!success && lastgkInfo)
	{
		GkCheckAltGk(lastgkInfo, 1);
	}

	CacheReleaseLocks(regCache);

	cmMeiExit(UH323Globals()->hApp);

	return(0);
}

int
GkScheduled(GkInfo *gkInfo)
{
	char fn[] = "GkScheduled():";
	CacheTableInfo *info = 0, cacheInfoEntry;
	int rc = 0;

	NETDEBUG(MRRQ, NETLOG_DEBUG4,
		("%s (%s/%lu) state = %s\n",
		fn, gkInfo->regid, gkInfo->uport, 
		gkRegState[gkInfo->regState]));

	info = &cacheInfoEntry;
	if (CacheFind(regCache, gkInfo, &cacheInfoEntry, 
								 sizeof(CacheTableInfo)) < 0)
	{
		NETERROR(MH323, ("%s Cannot find entry %s/%lu\n",
			fn, gkInfo->regid, gkInfo->uport));
		return -1;
	}

	// Check to see if we have a valid ip address for this
	if (!BIT_TEST(info->data.sflags, ISSET_IPADDRESS) ||
		(info->data.ipaddress.l == 0) ||
		!RealmIdIsValid(info->data.realmId) || 
		(info->data.ipaddress.l == (unsigned int)-1))
	{
		NETERROR(MH323, ("%s Invalid IP address/Realm for %s/%lu = %s/%d\n",
			fn, gkInfo->regid, gkInfo->uport,
			ULIPtostring(info->data.ipaddress.l), info->data.realmId));
		return -1;
	}

	/* Handle this gk */
	switch (gkInfo->regState)
	{
	case GKREG_REGISTERED:
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Gk is registered sending RRQ/RAI\n", fn));

		/* Send an LRRQ */
		rc = GkSendRRQ(gkInfo, &info->data, 1);
		if (BIT_TEST(info->data.cap, CAP_RAI))
		{
			GkSendRAI(gkInfo, &info->data);
		}
		break;

	case GKREG_IDLE:
	case GKREG_UNKNOWN:
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Gk is not registered sending GRQ/RRQ\n", fn));

		if (BIT_TEST(info->data.cap, CAP_GRQ))
		{
			/* Start discovery */
			rc = GkSendGRQ(gkInfo, &info->data);
		}
		else
		{
			rc = GkSendRRQ(gkInfo, &info->data, 0);
		}
		break;

	case GKREG_REGISTERING:
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Gk config changed re-registering\n", fn));
		rc = GkSendRRQ(gkInfo, &info->data, 0);
		break;

	default:
		NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Gk state is unknown\n", fn));
		/* Do nothing */
		rc = -1;
		break;
	}

	/* Next schedule for this guy depends on his state */
	/* NOTE: right now, this change does not get saved
	* in the cache
	*/
	gkInfo->regttl = GkComputeSchedule(gkInfo);
	
	gkInfo->nextEvent = time(NULL) + 
							gkInfo->regttl/2;

	NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s Scheduling next after %ds\n", fn, gkInfo->regttl/2));

	/* Delete this guy from the cache, and add him back */
	return(rc);
}

int
GkComputeSchedule(GkInfo *gkInfo)
{
	char fn[] = "GkComputeSchedule():";
	
	switch (gkInfo->regState)
	{
	case GKREG_REGISTERED:
		if (gkInfo->regttl > 0)
		{
			return gkInfo->regttl;
		}
		// else fall through
	default:
		return rrqtimer*2;
	}

	return rrqtimer*2;
}

int
GkInsertGkID(IN	HRAS hsRas, cmRASTrStage trstage, char *gkid)
{
	char fn[] = "GkInsertGkID():";
	cmAlias alias;
	BYTE gk_id[128];
	int index = 0;

	if (gkid[0] != '\0')
	{
		/* Add the gatekeeper id, and the aliases we wish to add */	
		alias.type = cmAliasTypeGatekeeperID;
		alias.length = utlChr2Bmp(gkid, &gk_id[0]);
		alias.string = (char *)&gk_id[0];

		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s Inserting GKID %s\n", fn, gkid));

		uh323AddRASAlias(hsRas, &alias, &index, trstage,
				cmRASParamGatekeeperID);
	}

	return(0);
}

int
GkCheckGkID(IN	HRAS hsRas, cmRASTrStage trstage, char *gkid)
{
	char fn[] = "GkCheckGkID():";
	int len;
	char gkidbmpstr[GKID_LEN] = { 0 }, gkidstr[GKID_LEN] = { 0 };
	cmAlias gkidbmp;

	/* Compare he GKID */
	len = sizeof(cmAlias);
	gkidbmp.string = &gkidbmpstr[0];
	gkidbmp.length = GKID_LEN;
	
	if (cmRASGetParam(hsRas, trstage,
						cmRASParamGatekeeperID,
						0,
						&len,
						(char *)&gkidbmp) >= 0)
	{
		 /* We must compare the Gk Id with the gatekeeper
		  * who replied to our request
		  */
		 utlBmp2Chr(&gkidstr[0], gkidbmp.string,
					gkidbmp.length);

		NETDEBUG(MRRQ, NETLOG_DEBUG4, ("%s comparing %s and %s\n",
			fn, gkid, gkidstr));

		 if (strncmp(gkid, gkidstr, GKID_LEN) != 0)
		 {
			  NETDEBUG(MRRQ, NETLOG_DEBUG4,
						("%s GRQ contains gkid - %s, ours %s\n",
						fn, gkidstr, gkid));
			  return -1;
		 }
		 else
		 {
			  /* match */
			  return 1;
		 }
	}
	else
	{
		 return 1;
	}
}

/* return 1 if there is a match, -1 if there is no match */
int
GkCompareGkID(HRAS hsRas)
{
	char fn[] = "GkCompareGkID():";
	int len;
	char gkidbmpstr[GKID_LEN] = { 0 }, gkidstr[GKID_LEN] = { 0 };
	cmAlias gkidbmp;

	return GkCheckGkID(hsRas, cmRASTrStageRequest, gkid);
}

void
GkEnd(void *arg)
{
	int id;
	CacheTableInfo *cacheInfo = 0;

	if (!h323GracefulShutdown)
	{
		// We are not required to send URQs
		return;
	}

	id = *(int *)arg;

	if ((id > 1) || (uh323ready < 0))
	{
		return;
	}

	// Send URQs to endpoints which are registered with us,
	// More importantly, to gatekeepers with whom we are
	// registered

	if (cmMeiEnter(UH323Globals()->hApp) < 0)
	{
		NETERROR(MH323, ("uh323Stack locks failed!!!\n"));
	}

	NETDEBUG(MRRQ, NETLOG_DEBUG4, ("shutdown: sending URQs\n"));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheInfo = CacheGetFirst(regCache);
			cacheInfo;
			cacheInfo = CacheGetNext(regCache, &cacheInfo->data))
	{
		if ((cacheInfo->data.stateFlags & (CL_ACTIVE|CL_REGISTERED)) &&
			BIT_TEST(cacheInfo->data.cap, CAP_H323))
		{
			// See if its an endpoint or a gk
			if ((id == 0) && !IsSGatekeeper(&cacheInfo->data))
			{
				GkSendURQ(&cacheInfo->data);
			}
			else if ((id == 1) || (nh323Instances == 1))
			{
				GkSendGkURQ(&cacheInfo->data);
			}
		}
	}

	CacheReleaseLocks(regCache);

	cmMeiExit(UH323Globals()->hApp);
	
	return;
}

int
IsIgrpQuotaFull(NetoidInfoEntry *pentry)
{
	CacheIgrpInfo	*entry=NULL;
	IgrpInfo		*igrp=NULL ;
	int				haveIgrpLock = 0;

	if (strlen(pentry->igrpName))
	{	
		CacheGetLocks(igrpCache, LOCK_READ, LOCK_BLOCK);
		haveIgrpLock = 1;
		if (entry = CacheGet(igrpCache, pentry->igrpName))
		{
			igrp = &entry->igrp;
		}
	}

	if (igrp && 
		( (IgrpXCallsTotal(igrp) &&
		  ((double)IgrpCallsTotal(igrp) >= 0.9*((double)IgrpXCallsTotal(igrp)))) ||
		  (IgrpXInCalls(igrp) &&
		  ((double)IgrpInCalls(igrp) >= 0.9*((double)IgrpXInCalls(igrp))))) )
	{
		if (haveIgrpLock)
		{
			CacheReleaseLocks(igrpCache);
			haveIgrpLock = 0;
		}
		return 1;
	}

	if (haveIgrpLock)
	{
		CacheReleaseLocks(igrpCache);
		haveIgrpLock = 0;
	}
	return 0;
}
