#include "gis.h"
#include <malloc.h>
#include "ua.h"

//Go over the endpoints in the cache and check to see if any SIP
// registrations need to be sent
void*
SipRegStart(void* dummy)
{
	struct timespec ts;
	int nregs, newItn = 1, rc;
	CacheTableInfo *info, cacheInfoEntry, nextCacheInfoEntry;
	time_t now;	

	// Every 5 seconds, scan the list of endpoints in the cache
	// Do no acquire locks
	// Check to see if registration state machine has been enabled
	// for each endpoint which needs to be registered

	info = &cacheInfoEntry;

	while (1)
	{
		time (&now);

		nregs = 0;
		
		// start at most 5 new registrations at a time
		while (nregs < 5)
		{
			if (newItn)
			{
				rc = CacheFindFirst(regCache, info, sizeof(CacheTableInfo));
			}
			else
			{
				rc = CacheFindNext(regCache, &info->data, info, sizeof(CacheTableInfo));
			}

			if (rc < 0)
			{

				newItn = 1;
				break;
			}

			newItn = 0;

			if ((info->data.stateFlags & CL_UAREG) &&	// Configuration
				(!(info->data.stateFlags & CL_UAREGSM) &&	// State machine is not running
					(!(info->data.stateFlags & CL_REGISTERED) ||
					(now - info->data.rTime) > 120)))	// If confirmed, refresh time is not recent enough
			{
				// Found a new entry
				nregs ++;
				SipRegStartRegSM(&info->data, NULL);
			}
		}

		// sleep for 5s
		ts.tv_sec = 5;
		ts.tv_nsec = 0; 
		nanosleep (&ts, NULL);
	}
	return(dummy);
}

int
SipStringToURI(char *istr, header_url **puri)
{
	header_url *uri;
	char *tmp, *nameStr = NULL, *hostStr = NULL, *portStr = NULL;
	char *str;

	if (!puri || (*istr == '\0'))
	{
		return -1;
	}

	str = strdup(istr);

	uri = *puri = (header_url *) malloc (sizeof(header_url));
	memset(uri, 0, sizeof(header_url));

	/* get rid of the sip: if there is one */
	if (strstr(str, "sip://"))
	{
		tmp = str + strlen("sip://");
	}
	else
	{
		tmp = str;
	}

	nameStr = strtok_r(tmp, "@", &hostStr);
	if (!nameStr || !hostStr)
	{
		NETERROR(MSIP, ("invalid uri %s\n", str));
		free(str);
		return -1;
	}

	hostStr = strtok_r(hostStr, ":", &portStr);

	/* Now assign these to the requri */
	uri->name = strdup(nameStr);
	uri->host = strdup(hostStr);
	if (portStr)
	{
		uri->port = atoi(portStr);
	}

	free(str);

	return 0;
}

void
SipRegStartRegSM(NetoidInfoEntry *infoEntry, char *auth)
{
	char fn[] = "SipRegStartRegSM():";
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	CacheTableInfo *info;
	header_url *uri = NULL;
	CacheRealmEntry *realmEntry;
	CallRealmInfo *realmInfo;
	int realmId;
	char rsadomain[24];

	// Setup the appMsgHandle
	if (SipStringToURI(infoEntry->uri, &uri) < 0)
	{
		NETERROR(MSIP, ("%s Cannot start reg on %s/%lu\n",
			fn, infoEntry->regid, infoEntry->uport));
		
		return;	
	}

	// Set the flag for the s/m
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	
	info = CacheGet(regCache, infoEntry);

	if (info)
	{
		info->data.stateFlags |= CL_UAREGSM;
	}
	else
	{
		NETERROR(MSIP, ("%s Cannot find %s/%lu in cache\n",
			fn, infoEntry->regid, infoEntry->uport));

		infoEntry = NULL;
	}

	realmId = info->data.realmId;

	CacheReleaseLocks(regCache);

	if (infoEntry == NULL)
	{
		return;
	}

	// send an event in for registration to start
	// Construct an app msg handle

	appMsgHandle = SipAllocAppMsgHandle();
	evHandle = SipAllocEventHandle();

	evHandle->type = Sip_eBridgeEvent;
	evHandle->event = SipReg_eBridgeRegister;
	evHandle->handle = appMsgHandle;

	appMsgHandle->calledpn = uri;

	appMsgHandle->requri = UrlDup(uri, MEM_LOCAL);
	SipCheckFree(appMsgHandle->requri->name);
	appMsgHandle->requri->name = NULL;

	appMsgHandle->callingpn = UrlDup(uri, MEM_LOCAL);
	//SipCheckFree(appMsgHandle->callingpn->host);

	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);
	realmEntry = CacheGet (realmCache, &realmId);
	if (realmEntry == NULL) 
	{
		NETERROR  (MBRIDGE, ("%s: Failed to get realm entry for id %d", fn, realmId));
		CacheReleaseLocks (realmCache);
		return;
	}
	FormatIpAddress(realmEntry->realm.rsa, rsadomain);

	realmInfo = (CallRealmInfo *) malloc (sizeof (CallRealmInfo));
	memset(realmInfo, 0, sizeof(CallRealmInfo));
	if (realmInfo == NULL) 
	{
		NETERROR (MSIP, ("%s: Malloc failure for RealmInfo!", fn));
		return;
	}
	realmInfo->realmId = realmEntry->realm.realmId;
	realmInfo->rsa = realmEntry->realm.rsa;
	realmInfo->sPoolId = realmEntry->realm.sigPoolId;
	realmInfo->mPoolId = realmEntry->realm.medPoolId;
	realmInfo->addrType = realmEntry->realm.addrType;
	realmInfo->interRealm_mr = realmEntry->realm.interRealm_mr;
	realmInfo->intraRealm_mr = realmEntry->realm.intraRealm_mr;
	realmInfo->sipdomain = strdup(rsadomain);

	CacheReleaseLocks (realmCache);

	//appMsgHandle->callingpn->host = strdup(rsadomain);

	appMsgHandle->localContact = UrlDup(uri, MEM_LOCAL);
	SipCheckFree(appMsgHandle->localContact->host);
	appMsgHandle->localContact->host = strdup(rsadomain);
	if (infoEntry->qval[0])
	{
		// add the q-value parameter as a contact param
		appMsgHandle->localContact->url_parameters[0].name = strdup("q");
		appMsgHandle->localContact->url_parameters[0].value = strdup(infoEntry->qval);
	}

	appMsgHandle->hdrAuthorization = auth;

	appMsgHandle->maxForwards = sipmaxforwards;

	appMsgHandle->realmInfo = realmInfo;

	SipRegProcessEvent(evHandle);
}
