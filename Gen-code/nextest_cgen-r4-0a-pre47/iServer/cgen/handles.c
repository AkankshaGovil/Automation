#include "gis.h"
#include <malloc.h>

void
GisFreeCallHandle(void *ptr)
{
	CallHandle *p = ptr;

	switch(p->handleType)
	{
	case SCC_eH323CallHandle:
		if (H323localSet(p)!= NULL)
		{
	 		MFree(callCache->free, H323localSet(p));
		}
		if (H323remoteSet(p)!= NULL)
		{
	 		MFree(callCache->free, H323remoteSet(p));
		}
		break;
	case SCC_eSipCallHandle:
		SipFreeSipCallHandle(SipCallHandle(p), sipCallCache->free);
		break;
	default:
		break;
	}

	MFree(callCache->free, p->conf_id);
	MFree(callCache->free, p->incoming_conf_id);

	GwFreeRejectList(p->destRejectList, CFree(callCache));

	MFree(callCache->free, p);
}

ConfHandle *
GisAllocConfHandle(void)
{
         ConfHandle *c = (ConfHandle *)(CMalloc(confCache))(sizeof(ConfHandle));

         memset(c, 0, sizeof(ConfHandle));

#if 0
         time(&c->iTime);
         time(&c->rTime);
#endif

         return c;
}


int
GisAddCallToConf(CallHandle *callHandle)
{
	return 0;

}

CallHandle *
GisAllocCallHandle(void)
{
	 CallHandle *c = (CallHandle *)MMalloc(callCache->malloc,
									sizeof(CallHandle));

	 memset(c, 0, sizeof(CallHandle));

	 H323controlState(c) = -1;

	 c->handleType = SCC_eH323CallHandle;

	 return c;
}

int
GisDeleteCallFromConf(char *callID, char *confID)
{
}
setRadiusAccountingSessionId () {}

CallRealmInfo *
RealmInfoDup (CallRealmInfo *ri, int mallocfn)
{
	char fn[] = "RealmInfoDup()";
	CallRealmInfo *realmInfo = NULL;

	if (ri == NULL)
	{
		return NULL;
	}
	realmInfo = (CallRealmInfo *)malloc ( sizeof (CallRealmInfo));
	if (realmInfo == NULL) 
	{
		NETERROR (MSIP, ("%s malloc failed for realminfo", fn));
		return NULL;
	}

	memcpy (realmInfo, ri, sizeof (CallRealmInfo));
	return realmInfo;
}

void
RealmInfoFree(CallRealmInfo *ri, int freefn)
{
        if (!ri)
        {
                return;
        }

        if (ri->sipdomain)
        {
                MFree(freefn, ri->sipdomain);
        }

        MFree(freefn, ri);
}

RealmPopulateCgen (int rsa)
{
	RealmEntry *defaultEntry;

	defaultEntry = (RealmEntry *) malloc (sizeof (RealmEntry));
	defaultEntry->rsa = htonl (rsa);

	defaultEntry->realmId = 0;
	CacheGetLocks (realmCache, LOCK_WRITE, LOCK_BLOCK);
	CacheInsert (realmCache, defaultEntry);
	CacheInsert (rsaCache, defaultEntry);
	CacheReleaseLocks (realmCache);
}

