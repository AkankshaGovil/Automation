#include <stdio.h>
#include <stdlib.h>

#include "bits.h"
#include "ipc.h"
#include "serverdb.h"
#include "key.h"
#include "alerror.h"
#include "db.h"
#include "profile.h"
#include "mem.h"
#include "entry.h"
#include "phone.h"
#include "srvrlog.h"
#include "gw.h"
#include "xmltags.h"
#include "lsconfig.h"

#if 0
/*
 * Cross check the netoid specified in registration/
 * unregistration packet, with the netInfo specified.
 * If possible return a sphone, which may be used to
 * as a key for looking up, inserting in the cache,
 * which the caller may use, and a set of flags, set
 * by this routine to tell what is present in netInfo,
 * but not in the packet.
 */
int
DiffNetoid(
	NetoidInfoEntry *netInfo, 	/* Database entry found for the
					 * registration id of the netoid
					 * referred in the data_pkt
					 */
	PhoNode 	*phonodep,
	unsigned short 	*rflags,	/* Tells the difference
					 * netInfo - phonodep,
					 * as a set of flags.
					 */
	unsigned short	*xflags		/* places where netInfo doesnt
					 * match phonodep
					 */
)			
{
	char fn[] = "DiffNetoid():";

     	if ((netInfo == NULL) || (phonodep == NULL))
     	{
		goto _error;
     	}

     	/* Compare the registration ids and uport */
     	if (memcmp(netInfo->regid, phonodep->regid, REG_ID_LEN) != 0)
     	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s Registration ID %s don't match with what we have %s\n",
				fn,
				phonodep->regid,
				netInfo->regid));
		BIT_SET(*xflags, ISSET_REGID);
     	}

     	if (netInfo->uport != phonodep->uport)
     	{
	  	NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s Uport %d does not match with what we have %d\n",
			fn,
			phonodep->uport,
			netInfo->uport));
		BIT_SET(*xflags, ISSET_UPORT);
     	}

     	/* We know that the thing exists in the database, Now check if the phone
      	* number matches...
      	*/
     	if (BIT_TEST(netInfo->sflags, ISSET_PHONE))
     	{
	  	if (BIT_TEST(phonodep->sflags, ISSET_PHONE))
	  	{
	       		if (strcmp(phonodep->phone, netInfo->phone) != 0)
	       		{
		    		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s Phone number %s does not match what we have %s\n", 
				fn,
				phonodep->phone,
				netInfo->phone
				));
				BIT_SET(*xflags, ISSET_PHONE);
	       		}
	  	}
	  	else
	  	{
			BIT_SET(*rflags, ISSET_PHONE);
	  	}
     	}

     	if (BIT_TEST(netInfo->sflags, ISSET_VPNPHONE))
     	{
	  	if (BIT_TEST(phonodep->sflags, ISSET_VPNPHONE))
	  	{
	       		if (strcmp(phonodep->vpnPhone, netInfo->vpnPhone) != 0)
	       		{
		    		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s Phone number %s does not match what we have %s\n", 
				fn,
				phonodep->vpnPhone,
				netInfo->vpnPhone
				));
				BIT_SET(*xflags, ISSET_VPNPHONE);
	       		}
	  	}
	  	else
	  	{
			BIT_SET(*rflags, ISSET_VPNPHONE);
	  	}
     	}
     
     	return 1;

_error:
	return -1;
}

/* Find the master entry for this iedge who is coming in
 * as a proxy client
 */
CacheTableInfo *
FindProxyMasterEntry(PhoNode *phonodep)
{
	CacheTableInfo *cacheInfo = 0;

	if (!cacheInfo && BIT_TEST(phonodep->sflags, ISSET_PHONE))
	{
		cacheInfo = (CacheTableInfo *)CacheGet(phoneCache, phonodep->phone);
	}

	if (!cacheInfo && BIT_TEST(phonodep->sflags, ISSET_VPNPHONE))
	{
		cacheInfo = (CacheTableInfo *)CacheGet(vpnPhoneCache, phonodep->vpnPhone);
	}

	return cacheInfo;
}

CacheTableInfo *
ConstructProxyInfo(InfoEntry *entry, PhoNode *phonodep)
{
	CacheTableInfo *cacheInfo = 0;

	/* Before we can do this proxy registration, we must
 	* apply the calling plans
 	*/
	
	if (!cacheInfo && BIT_TEST(phonodep->sflags, ISSET_PHONE))
	{
#if 0
		ApplyNetworkPolicy(entry,
			phonodep->phone,
			phonodep->phone,
			CRF_CALLORIGIN|CRF_CALLDEST|CRF_POSITIVE|CRF_VPNINT|CRF_VPNEXT,
			APPLY_DEST,
			NULL,
			NULL,
			NULL);
#endif
		cacheInfo = (CacheTableInfo *)CacheGet(phoneCache, 
						phonodep->phone);
	}

	if (!cacheInfo && BIT_TEST(phonodep->sflags, ISSET_VPNPHONE))
	{
#if 0
		ApplyNetworkPolicy(entry,
			phonodep->vpnPhone,
			phonodep->phone,
			CRF_CALLORIGIN|CRF_CALLDEST|CRF_POSITIVE|CRF_VPNINT|CRF_VPNEXT,
			APPLY_DEST,
			NULL,
			NULL,
			NULL);
#endif

		BIT_SET(phonodep->sflags, ISSET_PHONE);
		BIT_RESET(phonodep->sflags, ISSET_VPNPHONE);

		cacheInfo = (CacheTableInfo *)CacheGet(vpnPhoneCache, 
						phonodep->vpnPhone);
	}

	return cacheInfo;
}

#if 0
int
ProcessProxyRegistration(
	InfoEntry 	*masterInfo,
	InfoEntry 	*infoEntry,
	PhoNode 	*phonodep	
)
{
	char fn[] = "ProcessProxyRegistration():";
	InfoEntry tmpInfo;
	unsigned long pkey;
	CacheTableEntry *cacheHandle;

	if (!masterInfo || !infoEntry || !phonodep)
	{
		return -1;
	}
	
	if (masterInfo->stateFlags & CL_PROXYING)
	{
		/* Check to see if it is the same guy coming in again */
		if ((strncmp(masterInfo->xphone, infoEntry->phone,
				PHONE_NUM_LEN) != 0) ||
			(strncmp(masterInfo->xvpnPhone, infoEntry->vpnPhone,
				VPN_LEN) != 0))
		{
			NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s Server is already proxied by someone else. Sorry.\n", fn));
			return -1;
		}
	}

	if (masterInfo->stateFlags & CL_PROXY)
	{
		/* Now mark the master to be proxied for... */
		/* This fact not to be reflected in the database */
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s Marking the master entry as being proxied...\n",
				fn));
		masterInfo->stateFlags |= CL_PROXYING;

		/* Copy the relevent information from the data_pkt,
		 * onto the PROXYS cache entry
		 */ 
		memset(masterInfo->xphone, 0, PHONE_NUM_LEN);
		strncpy(masterInfo->xphone, infoEntry->phone, PHONE_NUM_LEN);
		memset(masterInfo->xvpnPhone, 0, VPN_LEN);
		strncpy(masterInfo->xvpnPhone, infoEntry->vpnPhone, VPN_LEN);
		
		/* Also copy the cacheEntry's information onto the infoEntry */
		memset(infoEntry->xphone, 0, PHONE_NUM_LEN);
		strncpy(infoEntry->xphone, masterInfo->phone, PHONE_NUM_LEN);
		memset(infoEntry->xvpnPhone, 0, VPN_LEN);
		strncpy(infoEntry->xvpnPhone, masterInfo->vpnPhone, VPN_LEN);
		infoEntry->stateFlags |= CL_PROXYING;
	
		/* Write the original cache info into the database */
		//UpdateNetoidDatabase(masterInfo);
		//UpdateNetoidDatabase(infoEntry);
		DbScheduleIedgeUpdate(masterInfo);
		DbScheduleIedgeUpdate(infoEntry);
	}
	else
	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s Master has not allowed proxying by someone\n", 
			fn));

		return -1;
	}
	
	return 1;
}
#endif

/* For a normal reg:
 * Everything must match.
 * For a proxy reg:
 * ser and uport match with db, and phones
 * match with master.
 */
int
ApplyAdmissionPolicy(
	PhoNode 	*phonodep, 
	NetoidInfoEntry	*info,
	int		isProxyReg,
	unsigned short 	xflags,		/* Places where iedge differs from
					 * its db entry
					 */
	unsigned short 	pflags,		/* Places where iedge differs from
					 * its master entry
					 */
	int		*reason
)
{
	char	fn[] = "ApplyAdmissionPolicy():";
	int 	eptype;

	if (nopolicy)
	{
	  	NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s No Policy Settings\n", fn));
		return 1;
	}

	eptype = IedgeTypeFromCap(phonodep);

	if (BIT_TEST(xflags, ISSET_REGID) ||
		BIT_TEST(xflags, ISSET_UPORT))
	{
	  	NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s  ser no or uport is not set\n",
			fn));
     		*reason = nextoneResourceUnavailable;
		return -1;
	}
	
	if (isProxyReg == 0)
	{
		if (BIT_TEST(xflags, ISSET_PHONE) ||
			BIT_TEST(xflags, ISSET_VPNPHONE))
		{
	  		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s  phone/vpn-phone num doesnt match\n",
				fn));
     			*reason = nextoneMismatchedAlias;
			return -1;
		}
	}
	else
	{
		if (BIT_TEST(pflags, ISSET_PHONE) ||
			BIT_TEST(pflags, ISSET_VPNPHONE))
		{
	  		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s  phone/vpn-phone num doesnt match (proxy-reg)\n",
				fn));
     			*reason = nextoneMismatchedAlias;
			return -1;
		}
	}

	/* Check the types */
	if (eptype != info->ispcorgw)
	{
	  	NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s  types dont match we have %d, incoming is %d\n",
			fn, eptype, info->ispcorgw));
     		*reason = nextoneMismatchedAlias;
		return -1;
	}

#if 0
	if (ispc || isgw)
	{
		if ((ispc && (info->ispcorgw != 1)) ||
			(isgw && (info->ispcorgw != 2)))
		{
	  		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s  types dont match\n",
				fn));
     			*reason = nextoneMismatchedAlias;
			return -1;
		}
	} 
	else if (info->ispcorgw)
	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s  types dont match\n",
			fn));
		*reason = nextoneMismatchedAlias;
		return -1;
	}
#endif

	return 1;
}

/* Add an entry or update an entry in the cache and database.
 * We make the following checks:
 * Registration ID is Valid
 * Phone # matches that in database (if one is there)
 * Same phone number does not already exist
 */
int
ProcessIedgeRegistration(Pkt * data_pkt)
{
	 char 			fn[] = "ProcessIedgeRegistration():";
	 NetoidInfoEntry 	*netInfo, tmpInfo;
	 PhoNode 		*phonodep;
	 CacheTableInfo 		*cacheInfo = 0, *gwCacheInfo = 0, 
		  *masterCacheInfo = 0, *ipCacheInfo = 0;
	 int 			eptype, wasGateway = 0, isProxyReg = 0;
	 unsigned short 		rflags = 0, xflags = 0, pflags = 0, tmpflags = 0;
	 int 			type = data_pkt->type;
	 char 			tags[TAGH_LEN] = { 0 };

	 isProxyReg = (type == PKT_PROXYREGISTER);

	 data_pkt->data.reginfo.reason = nextoneResourceUnavailable;

     /* First obtain entry from database and cache */
	 NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Processing Starts for\n", fn));

	 DEBUG_PrintPhoNode(MREGISTER, NETLOG_DEBUG4, 
						&data_pkt->data.reginfo.node);

	 if (!BIT_TEST(data_pkt->data.reginfo.node.sflags, ISSET_REGID) ||
		 !BIT_TEST(data_pkt->data.reginfo.node.sflags, ISSET_UPORT))
	 {
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				   ("%s Registration ID missing... \n", fn));
		  return -1;
	 }

	 phonodep = &data_pkt->data.reginfo.node;

	 return ProcessPhonodeRegistration(phonodep, isProxyReg,
				&data_pkt->data.reginfo.reason);
}

int
ProcessPhonodeRegistration(PhoNode *phonodep, int isProxyReg, int *reason)
{
	 char 			fn[] = "ProcessPhonodeRegistration():";
	 NetoidInfoEntry 	*netInfo, tmpInfo;
	 CacheTableInfo 	*cacheInfo = 0, *gwCacheInfo = 0, 
		  		*masterCacheInfo = 0, *ipCacheInfo = 0;
	 VpnEntry 		vpn;
	 int 			eptype, wasGateway = 0;
	 unsigned short 	rflags = 0, xflags = 0, pflags = 0, 
				tmpflags = 0;
	 char 			tags[TAGH_LEN] = { 0 };

	 *reason = nextoneResourceUnavailable;

     /* First obtain entry from database and cache */
	 NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Processing Starts for\n", fn));

	 DEBUG_PrintPhoNode(MREGISTER, NETLOG_DEBUG4, 
						phonodep);

	 if (!BIT_TEST(phonodep->sflags, ISSET_REGID) ||
		 !BIT_TEST(phonodep->sflags, ISSET_UPORT))
	 {
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				   ("%s Registration ID missing... \n", fn));
		  return -1;
	 }

	 eptype = IedgeTypeFromCap(phonodep);

	 NETDEBUG(MREGISTER, NETLOG_DEBUG4, 
		("%s Incoming Registration for %s\n", 
		 fn, IedgeName(eptype)));

	 NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Looking up Cache... \n", fn));

	/* Lock the cache */
	 CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	 /* Check to see if there is already a different entry with the same
	  * ip address in the cache
	  */
	 cacheInfo = (CacheTableInfo *)CacheGet(regCache, phonodep);

	 if (cacheInfo == NULL)
	 {
		  /* This means that the registration ID is not in our db */
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				   ("%s Entry for %s %d does not exist in cache\n", 
					fn, phonodep->regid, phonodep->uport));
		  *reason = nextoneInvalidEndpointID;
		  goto _error;
	 }

	 NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("Entry for %s exists in cache\n",
										 phonodep->regid));

	 /* Do an ipaddress check */
	 if (BIT_TEST(phonodep->sflags, ISSET_IPADDRESS))
	 {
		  if (ipCacheInfo = CheckDuplicateIedgeIpAddr(ipCache, 
													  &cacheInfo->data, 
													  phonodep->ipaddress.l))
		  {
			/* There is a duplicate iedge in the cache */
			NETDEBUG(MREGISTER, NETLOG_DEBUG1,
				("%s Found an iedge with the same IP address %s/%d\n",
			 	fn, ipCacheInfo->data.regid, 
				ipCacheInfo->data.uport));
			   goto _error;
		  }
	 }

	 /* Find the difference between the iedge and its database entry */
	 DiffNetoid(&cacheInfo->data, phonodep, &rflags, &xflags);

	 if (isProxyReg == 1)
	 {
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s Proxy Registration Request\n", fn));

		masterCacheInfo = ConstructProxyInfo(&cacheInfo->data, 
					phonodep);
	 }

	 if (masterCacheInfo)
	 {
		  /* Find the difference between the iedge and its master entry */
		  DiffNetoid(&masterCacheInfo->data, phonodep, 
				&tmpflags, &pflags);
	 }
	 else if (isProxyReg == 1)
	 {
		  /* This means that the registration ID is not in our db */
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("%s Master Entry for %s does not exist in cache\n", 
			fn, phonodep->regid));
		  *reason = nextoneInvalidEndpointID;
		  goto _error;
	 }

	 if (ApplyAdmissionPolicy(phonodep, &cacheInfo->data,
							  isProxyReg, xflags, pflags,
							  (int *)reason) < 0)
	 {
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("Entry does not match database information\n"));
		  goto _error;
	 }

	 /* Reset any previous states, which may need to be set */
	 ResetIedgeRegistrationState(&cacheInfo->data);

#if 0
	 if (isProxyReg)
	 {
		  if (ProcessProxyRegistration(&masterCacheInfo->data, &cacheInfo->data, phonodep) > 0)
		  {
			   /* Do nothing */
			   BITA_SET(tags, TAG_PROXYS);
			   BITA_SET(tags, TAG_PROXYC);

			   NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s Proxy Registration Successful\n", fn));
		  }
		  else
		  {
			   /* Aha... error */
			   /* Free the locks 
				* Locks will be freed by HandleProxyRegistration.
				*/
			   NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s Proxy Registration Failed\n", fn));
			   goto _error;
		  }
	 }
#endif
	
	 netInfo = &cacheInfo->data;

	/* Grab data from packet and update
	 * cache entry, as needed
	 */
	 netInfo->stateFlags |= CL_ACTIVE;

	 netInfo->rTime = (time(0));

	 BITA_SET(tags, TAG_RTIME);
	 BITA_SET(tags, TAG_ACTIVE);

	 if ( (netInfo->stateFlags & CL_FORWARD) &&
		  ((netInfo->protocol) == NEXTONE_REDIRECT_ROLLOVER) &&
		!(netInfo->stateFlags & CL_FORWARDSTATIC))
	 {
		  BITA_SET(tags, TAG_FWDINFO);

		  netInfo->stateFlags &= ~CL_FORWARD;
		  netInfo->protocol = 0;
	 }

	 if (BIT_TEST(phonodep->sflags, 
				  ISSET_IPADDRESS))
	 {
		  BITA_SET(tags, TAG_IPADDRESS);

		  netInfo->ipaddress.l = 
			   phonodep->ipaddress.l;
		  netInfo->callsigport = 1720;

		  BIT_SET(netInfo->sflags, ISSET_IPADDRESS);
		  netInfo->rasip = phonodep->ipaddress.l;
		  netInfo->rasport = 1719;
	 }

	 if (IsGateway(netInfo))
	 {
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				   ("Device previously was a gateway"));
		  wasGateway = 1;
	 }

	 netInfo->ispcorgw = eptype;
	 netInfo->cap = phonodep->cap;

	 BIT_SET(netInfo->cap, CAP_UCC);
	 BIT_SET(netInfo->cap, CAP_H323);

	 if (eptype == IEDGE_1000)
	 {
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			("iedge 1000 marked SIP Enabled\n"));
		BIT_SET(netInfo->cap, CAP_SIP);
	 }

	 /* If this device is a gateway, see if we can add a gateway */
	 if (BIT_TEST(phonodep->cap, CAP_IGATEWAY))
	 {
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				   ("%s Device is a Gateway\n", fn));

		/* Copy the DND state in the packet */
		if (phonodep->clientState & CL_DND)
		{
		  	netInfo->stateFlags |= CL_DND;
		}
		else
		{
		  	netInfo->stateFlags &= ~CL_DND;
		}

		BITA_SET(tags, TAG_ENDPTTYPE);
		BITA_SET(tags, TAG_GISPREFIX);
		netInfo->xcalls = 1;
	 }
	 else
	 {
		BITA_SET(tags, TAG_ENDPTTYPE);
		netInfo->xcalls = 2;
	 }

	 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
			  ("%s Entry updated in cache\n", fn));

	 netInfo->stateFlags |= CL_REGISTERED;

	 memcpy(&tmpInfo, netInfo, sizeof(NetoidInfoEntry));
     
	 /* See if we have to add a gateway entry into our gw cache.
     	 * Use tmpInfo for this purpose.
      	*/ 
	 if (IsGateway(&tmpInfo))
	 {
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				   ("%s Registration is for a gateway\n", fn));

		  CacheInsert(gwCache, cacheInfo);
	 }
	 else if ((wasGateway == 1) && !(tmpInfo.stateFlags & CL_STATIC))
	 {
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				   ("%s Entry was previously a gateway\n", fn));
		  CacheDelete(gwCache, &tmpInfo);
	 }

	 /* Make sure the ip address is in our cache */
	 AddIedgeIpAddr(ipCache, cacheInfo);

	 /* Release the cache at this point */
	 CacheReleaseLocks(regCache);

	 NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Updating database...\n", fn));

	 //UpdateNetoidDatabase(&tmpInfo);
	 DbScheduleIedgeUpdate(&tmpInfo);

	 PropagateIedgeIpAddr(tmpInfo.regid, tmpInfo.ipaddress.l);

	 /* Before we return, we must set the remaining fields back in the
      	* data packet - those which are set in us, but not in the packet...
      	*/
	 if (BIT_TEST(rflags, ISSET_IPADDRESS))
	 {
		  phonodep->ipaddress.l = tmpInfo.ipaddress.l;
		  BIT_SET(phonodep->sflags, ISSET_IPADDRESS);
	 }

	 if (BIT_TEST(rflags, ISSET_PHONE))
	 {
		  strcpy(phonodep->phone, tmpInfo.phone);
		  BIT_SET(phonodep->sflags, ISSET_PHONE);
	 }
     
	 if ((FindIedgeVpn(netInfo->vpnName, &vpn) >= 0) &&
		(vpn.vpnExtLen + strlen(vpn.vpnId) < PHONE_NUM_LEN))
	 {
		  strcpy(phonodep->vpnPhone, vpn.vpnId);
		  phonodep->vpnExtLen = vpn.vpnExtLen;
		  strcat(phonodep->vpnPhone, tmpInfo.vpnPhone);
		  BIT_SET(phonodep->sflags, ISSET_VPNPHONE);
		  NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s Assigned vpn phone %s\n",
				fn, phonodep->vpnPhone));
	 }

	 /* The following will tell the iedge whether forwarding
      	* was enabled by the user. The iedge will not know to what
      	* number it had turned forwarding on, and that information
      	* resides on the server
      	*/

	 phonodep->clientState = tmpInfo.stateFlags;

	 NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Registration done\n", fn));

	 return 1;

 _error:
	 CacheReleaseLocks(regCache);

	 return -1;
}    

#endif
