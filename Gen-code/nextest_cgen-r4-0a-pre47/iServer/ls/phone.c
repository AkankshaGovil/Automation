#include "ipc.h"
#include "bits.h"
#include "serverdb.h"
#include "mem.h"
#include "entry.h"
#include "srvrlog.h"
#include "serverp.h"
#include "lsprocess.h"
#include "xmltags.h"
#include "gis.h"
#include "ls.h"

int
HandleProxyRegistration(NetoidInfoEntry *infoEntry)
{
	char fn[] = "HandleProxyRegistration():";
	char *sphone = 0;
	unsigned long pkey;
	CacheTableEntry *cacheHandle;
	CacheTableInfo *cacheInfo = 0;
	InfoEntry tmpInfo;

#if 0
	if ( !cacheInfo && strlen(infoEntry->xvpnPhone))
	{
		sphone = infoEntry->xvpnPhone;
		cacheInfo = (CacheTableInfo *)CacheGet(vpnPhoneCache, sphone);
	}

	if ( !cacheInfo && strlen(infoEntry->xphone))
	{
		sphone = infoEntry->xphone;
		cacheInfo = (CacheTableInfo *)CacheGet(phoneCache, sphone);
	}

	if (cacheInfo == NULL)
	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s No cache entry found\n", fn));
		return -1;
	}

	if (cacheInfo)
	{
     		cacheInfo->data.stateFlags |= (CL_PROXYING|CL_PROXY);
	}

	strncpy(cacheInfo->data.xphone, infoEntry->xphone,
		PHONE_NUM_LEN);
	cacheInfo->data.xphone[PHONE_NUM_LEN-1] = '\0';

	strncpy(cacheInfo->data.xvpnPhone, infoEntry->vpnPhone,
		VPN_LEN);	
	cacheInfo->data.xvpnPhone[VPN_LEN-1] = '\0';

	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));

	UpdateNetoidDatabase(&tmpInfo);
#endif
	return(0);
}

int
UnregisterProxy(NetoidInfoEntry *infoEntry)
{
#if 0
	char *sphone = 0;
     	char fn[] = "UnregisterProxy():";
     	CacheTableInfo *cacheInfo = 0;
     	CacheTableEntry *cacheHandle = 0;
	NetoidInfoEntry tmpInfo;

	/* Disable my proxying */
	infoEntry->stateFlags &= ~CL_PROXYING;

	/* Disable proxying in my peer */
	if (!cacheInfo && strlen(infoEntry->xvpnPhone))
	{
		sphone = infoEntry->xvpnPhone;
		cacheInfo = (CacheTableInfo *)CacheGet(vpnPhoneCache, sphone);
	}

	if (!cacheInfo && strlen(infoEntry->xphone))
	{
		sphone = infoEntry->xphone;
		cacheInfo = (CacheTableInfo *)CacheGet(phoneCache, sphone);
	}

	if (cacheInfo == NULL)
	{
		NETERROR(MREGISTER, 
			("UnregisterProxy: Iedge proxied, but no peer phone found\n"));
		return 0;
	}
		
     	cacheInfo->data.stateFlags &= ~CL_PROXYING;
	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));

	UpdateNetoidDatabase(&tmpInfo);
#endif
	return(1);
}

int
ResetIedgeRegistrationState(NetoidInfoEntry *netInfo)
{
	char fn[] = "ResetIedgeRegistrationState():";

	if (netInfo->stateFlags & CL_PROXYING)
	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Disabling previous proxy\n", fn));
		UnregisterProxy(netInfo);
	}
	return(0);
}

int
RedundsUpdateVpnDelete(int fd, VpnEntry *vpnEntry)
{
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("RedundsUpdateVpnDelete: Informing of deleted entry"));

	BITA_SET(tags, TAG_DELETE);

	len += sprintf(buffer, "<CU>");
	len += XMLEncodeVpnEntry(vpnEntry, buffer+len, 4096-len, tags);
	len += sprintf(buffer+len, "</CU>");

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsUpdateVpnGDelete(int fd, VpnGroupEntry *vpnGroupEntry)
{
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("RedundsUpdateVpnGDelete: Informing of deleted entry"));

	BITA_SET(tags, TAG_DELETE);

	len += sprintf(buffer, "<CU>");
	len += XMLEncodeVpnGEntry(vpnGroupEntry, buffer+len, 4096-len, tags);
	len += sprintf(buffer+len, "</CU>");

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsUpdateNetoidDelete(int fd, InfoEntry *entry)
{
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("RedundsUpdateNetoidDelete: Informing of deleted entry"));

	BITA_SET(tags, TAG_REGID);
	BITA_SET(tags, TAG_UPORT);
	BITA_SET(tags, TAG_RTIME);
	BITA_SET(tags, TAG_DELETE);

	len += sprintf(buffer, "<CU>");
	len += XMLEncodeInfoEntry(entry, buffer+len, 4096-len, tags);
	len += sprintf(buffer+len, "</CU>");

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsUpdateCRDelete(int fd, VpnRouteEntry *routeEntry)
{
	 char fn[] = "RedundsUpdateCRDelete():";
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("%s Informing of deleted entry", fn));

	BITA_SET(tags, TAG_CRDEST);
	BITA_SET(tags, TAG_CRPREFIX);
	BITA_SET(tags, TAG_MTIME);
	BITA_SET(tags, TAG_DELETE);

	len += sprintf(buffer, "<CU>");
	len += XMLEncodeCREntry(routeEntry, buffer+len, 4096-len, tags);
	len += sprintf(buffer+len, "</CU>");

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsUpdateCPDelete(int fd, CallPlanEntry *cpEntry)
{
	 char fn[] = "RedundsUpdateCPDelete():";
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("%s Informing of deleted entry", fn));

	BITA_SET(tags, TAG_DELETE);

	len += sprintf(buffer, "<CU>");
	len += XMLEncodeCPEntry(cpEntry, buffer+len, 4096-len, tags);
	len += sprintf(buffer+len, "</CU>");

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsPushVpnEntry(int fd, VpnEntry *vpnEntry)
{
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("RedundsPushVpnEntry: Pushing entry"));

	len = AddEntry(TAG_VPN, buffer, 4096, vpnEntry);

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsPushVpnGEntry(int fd, VpnGroupEntry *vpnGroupEntry)
{
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("RedundsPushVpnGEntry: Pushing entry"));

	len = AddEntry(TAG_VPNG, buffer, 4096, vpnGroupEntry);

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsPushNetoidEntry(int fd, InfoEntry *entry)
{
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("RedundsPushNetoidEntry: Pushing entry"));

	len = AddEntry(TAG_IEDGE, buffer, 4096, entry);

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsPushCREntry(int fd, VpnRouteEntry *routeEntry)
{
	 char fn[] = "RedundsPushCREntry():";
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("%s Pushing entry", fn));

	len = AddEntry(TAG_CR, buffer, 4096, routeEntry);

	SendXML(fd, buffer, len);
	return(0);
}

int
RedundsPushCPEntry(int fd, CallPlanEntry *cpEntry)
{
	 char fn[] = "RedundsPushCPEntry():";
	char buffer[4096];
	int len = 0;
	char tags[TAGH_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG4,
		("%s Pushing entry", fn));

	len = AddEntry(TAG_CP, buffer, 4096, cpEntry);

	SendXML(fd, buffer, len);
	return(0);
}

#if 0
int
ExamineUpdateCache(LsMemStruct *lsMem, int fd)
{
	char fn[] = "ExamineUpdateCache():";
	CacheTableInfo *info = 0;
	CacheVpnEntry *cacheVpnEntry;
	CacheVpnGEntry *cacheVpnGEntry;
	CacheVpnRouteEntry *cacheRouteEntry;
	CacheCPEntry *cacheCPEntry;

	NETDEBUG(MAGE, NETLOG_DEBUG4, ("%s Examining Updates Cache\n", fn));

	/* Look at the update List */
	if (MemGetRwLock(&lsMem->updatemutex, LOCK_READ, LOCK_BLOCK) == AL_OK)
	{
		CacheEntry *ce, *nextce = 0;
		
	      	for (ce = updateList; 
		   		((ce != 0) && (nextce != updateList));
		    		ce = nextce)
		{
			nextce = ce->next;

		  	if (ce->entry == NULL)
			{
				/* Dont process this one */
				continue;
			}

			switch(ce->type)
			{
			case CACHE_INFO_ENTRY:
				info = (CacheTableInfo *)ce->entry;
				if (info->state & CACHE_NEEDS_DELETE)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("%s Found an iedge marked CACHE_NEEDS_DELETE", fn));
					RedundsUpdateNetoidDelete(fd, &info->data);
				}
				if (info->state & CACHE_PUSH_ENTRY)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("%s Found an iedge marked CACHE_PUSH_ENTRY", fn));
					RedundsPushNetoidEntry(fd, &info->data);
				}

				SHM_Free(info);

				break;
			case CACHE_VPN_ENTRY:
				cacheVpnEntry = (CacheVpnEntry *)ce->entry;
				if (cacheVpnEntry->state & CACHE_NEEDS_DELETE)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("%s Found a vpn entry marked CACHE_NEEDS_DELETE", fn));
					RedundsUpdateVpnDelete(fd, &cacheVpnEntry->vpnEntry);
				}
				if (cacheVpnEntry->state & CACHE_PUSH_ENTRY)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("%s Found a vpn entry marked CACHE_PUSH_ENTRY", fn));
					RedundsPushVpnEntry(fd, &cacheVpnEntry->vpnEntry);
				}

				SHM_Free(cacheVpnEntry);	

				break;
			case CACHE_VPNG_ENTRY:
				cacheVpnGEntry = (CacheVpnGEntry *)ce->entry;
				if (cacheVpnGEntry->state & CACHE_NEEDS_DELETE)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("%s Found a vpn group entry marked CACHE_NEEDS_DELETE", fn));
					RedundsUpdateVpnGDelete(fd, &cacheVpnGEntry->vpnGroupEntry);
				}
				if (cacheVpnGEntry->state & CACHE_PUSH_ENTRY)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("%s Found a vpn group entry marked CACHE_PUSH_ENTRY", fn));
					RedundsPushVpnGEntry(fd, &cacheVpnGEntry->vpnGroupEntry);
				}
				SHM_Free(cacheVpnGEntry);	
				break;
			case CACHE_ROUTE_ENTRY:
				cacheRouteEntry = (CacheVpnRouteEntry *)ce->entry;
				if (cacheRouteEntry->state & CACHE_NEEDS_DELETE)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("Found a cp route entry marked CACHE_NEEDS_DELETE"));
					RedundsUpdateCRDelete(lsMem->agefd,
						&cacheRouteEntry->routeEntry);
				}
				if (cacheRouteEntry->state & CACHE_PUSH_ENTRY)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("Found a cp route entry marked CACHE_PUSH_ENTRY"));
					RedundsPushCREntry(lsMem->agefd,
						&cacheRouteEntry->routeEntry);
				}
				SHM_Free(cacheRouteEntry);
				break;
			case CACHE_CP_ENTRY:
			    cacheCPEntry = (CacheCPEntry *)ce->entry;
				if (cacheCPEntry->state & CACHE_NEEDS_DELETE)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("Found a cp entry marked CACHE_NEEDS_DELETE"));
					RedundsUpdateCPDelete(lsMem->agefd,
						&cacheCPEntry->cpEntry);
				}
				if (cacheCPEntry->state & CACHE_PUSH_ENTRY)
				{
					NETDEBUG(MAGE, NETLOG_DEBUG4,
						("Found a cp route entry marked CACHE_PUSH_ENTRY"));
					RedundsPushCPEntry(lsMem->agefd,
						&cacheCPEntry->cpEntry);
				}
				SHM_Free(cacheCPEntry);
				break;			
			default:
				break;
			}

			ListDelete(ce);
			SHM_Free(ce);
		}

		MemReleaseRwLock(&lsMem->updatemutex);
	}
	return(0);
}
#endif

