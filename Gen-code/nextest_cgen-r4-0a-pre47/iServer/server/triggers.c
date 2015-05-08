#include "gis.h"
#include "tags.h"
#include "nxosd.h"
#include "phonode.h"
#include "dbs.h"

extern int dbpoolid, dbclassid;

struct triggerdata
{
	TriggerEntry trigger;

	// valid only for route triggers
	// source regid for call
	char regid[REG_ID_LEN];
	unsigned long uport;

	char number[PHONE_NUM_LEN];
	char srcRouteNumber[PHONE_NUM_LEN];
	char transitRouteNumber[PHONE_NUM_LEN];
	char transitcrname[CALLPLAN_ATTR_LEN];
	char srccrname[CALLPLAN_ATTR_LEN];
	int routeflag;

	// dest regid for call
	char dregid[REG_ID_LEN];
	unsigned long duport;
};

void *
TriggerCmdWorker(void *arg)
{
	char fn[] = "TriggerCmdWorker():";
	struct triggerdata *tdata = (struct triggerdata *)arg;
	CacheRouteEntry *entry = NULL, *template = NULL;
#ifdef _separate_binding_
	CacheCPBEntry *cpbEntry = NULL;
#endif
	CacheTriggerEntry *tgPtr = NULL;
	CacheTableInfo *info = NULL;
	int routeAdded = 0, creatBinding = 0, bindingAdded = 0;
	TriggerEntry tgEntry;
	char crname[CALLPLAN_ATTR_LEN] = { 0 };
	char tags[TAGH_LEN] = { 0 }, s1[25];

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

	// lookup the template route
	template = CacheGet(cpCache, tdata->trigger.actiondata);

	if (!template)
	{
		// no template route ??
		NETERROR(MINIT, 
			("%s no template route found for trigger %s\n",
			fn, tdata->trigger.name));

		goto _return;
	}

	// Check to see if the template is valid
	if (!((template->routeEntry.crflags & CRF_CALLORIGIN) ||
			((template->routeEntry.crflags & (CRF_CALLDEST|CRF_REJECT)) 
				== (CRF_CALLDEST|CRF_REJECT)) ||
			(template->routeEntry.crflags & CRF_TRANSIT)))
	{
		NETERROR(MINIT, 
			("%s invalid template route for trigger %s\n",
			fn, tdata->trigger.name));

		goto _return;
	}

	if (template->routeEntry.crflags & CRF_CALLORIGIN)
	{
		info = CacheGet(regCache, tdata->regid);

		if (!info || info->data.cpname[0] == '\0')
		{
			NETERROR(MINIT,
				("%s No Endpoint found with regid=%s\n", 
				 fn, tdata->regid));
			goto _return;
		}

		creatBinding = 1;
	}

	if (template->routeEntry.crflags & (CRF_CALLDEST|CRF_REJECT))
	{
		info = CacheGet(regCache, tdata->dregid);

		if (!info || info->data.cpname[0] == '\0')
		{
			NETERROR(MINIT,
				("%s No Endpoint found with regid=%s\n", 
				 fn, tdata->dregid));

			goto _return;
		}

		creatBinding = 1;
	}

	entry = CMalloc(cpCache)(sizeof(CacheRouteEntry));	
	
	if (entry == NULL)
	{
		NETERROR(MINIT, ("%s Out of memory!\n", fn));
		goto _return;
	}

	memset(entry, 0, sizeof(CacheRouteEntry)); 

#ifdef _separate_binding_
	if (creatBinding)
	{
		cpbEntry = CMalloc(cpbCache)(sizeof(CacheCPBEntry));
	
		if (cpbEntry == NULL)
		{
			NETERROR(MINIT, ("%s Out of memory!\n", fn));
			goto _return;
		}

		memset(cpbEntry, 0, sizeof(CacheRouteEntry)); 
	}
#endif

	time(&entry->routeEntry.mTime);
	time(&entry->routeEntry.rTime);

	if (template->routeEntry.crflags & CRF_TRANSIT)
	{
		if(tdata->routeflag & TRANSIT_ROUTE_APPLIED)
		{
			if(strcmp(template->routeEntry.crname, tdata->transitcrname) == 0)
			{
				NETERROR(MINIT, ("%s Transit Route %s has already been applied to this call\n", fn, tdata->transitcrname));
				goto _return;
			}
		}

		if(tdata->trigger.actionflags & TRIGGER_FLAG_ROUTE_OVERRIDE)
		{
			if(tdata->routeflag & SRC_ROUTE_APPLIED)
			{
				entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->srcRouteNumber, PHONE_NUM_LEN);
				nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
				nx_strlcat(entry->routeEntry.prefix, tdata->srcRouteNumber, PHONE_NUM_LEN);
			}
			else
			{
				entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->number, PHONE_NUM_LEN);
				nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
				nx_strlcat(entry->routeEntry.prefix, tdata->number, PHONE_NUM_LEN);
			}
		}
		else
		{
			if(tdata->routeflag & TRANSIT_ROUTE_APPLIED)
			{
				if(tdata->routeflag & SRC_ROUTE_APPLIED)
				{
					entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->srcRouteNumber, PHONE_NUM_LEN);
					nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
					nx_strlcat(entry->routeEntry.prefix, tdata->transitRouteNumber, PHONE_NUM_LEN);
				}
				else
				{
					entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->number, PHONE_NUM_LEN);
					nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
					nx_strlcat(entry->routeEntry.prefix, tdata->transitRouteNumber, PHONE_NUM_LEN);
				}
			}
			else
			{
				if(tdata->routeflag & SRC_ROUTE_APPLIED)
				{
					entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->srcRouteNumber, PHONE_NUM_LEN);
					nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
					nx_strlcat(entry->routeEntry.prefix, tdata->srcRouteNumber, PHONE_NUM_LEN);
				}
				else
				{
					entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->number, PHONE_NUM_LEN);
					nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
					nx_strlcat(entry->routeEntry.prefix, tdata->number, PHONE_NUM_LEN);
				}
			}
		}
		// We will be adding transit routes only
		snprintf(entry->routeEntry.crname, CALLPLAN_ATTR_LEN, "%s@%s", 
			tdata->number, tdata->trigger.name);

		entry->routeEntry.crflags |= CRF_TRANSIT;
	}
	else if (template->routeEntry.crflags & CRF_CALLORIGIN)
	{
		if(tdata->routeflag & SRC_ROUTE_APPLIED)
		{
			if(strcmp(template->routeEntry.crname, tdata->srccrname) == 0)
			{
				NETERROR(MINIT, ("%s Src Route %s has already been applied to this call\n", fn, tdata->srccrname));
				goto _return;
			}
		}

		if(!(tdata->trigger.actionflags & TRIGGER_FLAG_ROUTE_OVERRIDE) && (tdata->routeflag & SRC_ROUTE_APPLIED))
		{
			entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->number, PHONE_NUM_LEN);
			nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
			nx_strlcat(entry->routeEntry.prefix, tdata->srcRouteNumber, PHONE_NUM_LEN);
		}
		else
		{
			entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->number, PHONE_NUM_LEN);
			nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
			nx_strlcat(entry->routeEntry.prefix, tdata->number, PHONE_NUM_LEN);
		}

		// We will be adding src routes only
		snprintf(entry->routeEntry.crname, CALLPLAN_ATTR_LEN, "%s@%s", 
			tdata->number, FormatIpAddress(info->data.ipaddress.l, s1));

		entry->routeEntry.crflags |= CRF_CALLORIGIN;

#ifdef _separate_binding_
		nx_strlcpy(cpbEntry->cpbEntry.crname, entry->routeEntry.crname, CALLPLAN_ATTR_LEN);
		nx_strlcpy(cpbEntry->cpbEntry.cpname, info->data.cpname, CALLPLAN_ATTR_LEN);
#else
		nx_strlcpy(entry->routeEntry.cpname, info->data.cpname, CALLPLAN_ATTR_LEN);
#endif
	}
	else if (template->routeEntry.crflags & (CRF_CALLDEST|CRF_REJECT))
	{
		if(tdata->routeflag & TRANSIT_ROUTE_APPLIED)
		{
			entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->transitRouteNumber, PHONE_NUM_LEN);
			nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
			nx_strlcat(entry->routeEntry.prefix, tdata->transitRouteNumber, PHONE_NUM_LEN);
		}
		else if(tdata->routeflag & SRC_ROUTE_APPLIED)
		{
			entry->routeEntry.destlen = nx_strlcpy(entry->routeEntry.dest, tdata->srcRouteNumber, PHONE_NUM_LEN);
			nx_strlcpy(entry->routeEntry.prefix, template->routeEntry.prefix, PHONE_NUM_LEN);
			nx_strlcat(entry->routeEntry.prefix, tdata->srcRouteNumber, PHONE_NUM_LEN);
		}

		// We will be adding src routes only
		snprintf(entry->routeEntry.crname, CALLPLAN_ATTR_LEN, "%s@%s", 
			tdata->number, FormatIpAddress(info->data.ipaddress.l, s1));

		entry->routeEntry.crflags |= (CRF_CALLDEST|CRF_REJECT);

#ifdef _separate_binding_
		nx_strlcpy(cpbEntry->cpbEntry.crname, entry->routeEntry.crname, CALLPLAN_ATTR_LEN);
		nx_strlcpy(cpbEntry->cpbEntry.cpname, info->data.cpname, CALLPLAN_ATTR_LEN);
#else
		nx_strlcpy(entry->routeEntry.cpname, info->data.cpname, CALLPLAN_ATTR_LEN);
#endif
	}
	else
	{
		NETERROR(MINIT, ("%s Invalid template route!\n", fn));
		goto _return;
	}

	nx_strlcpy(crname, entry->routeEntry.crname, CALLPLAN_ATTR_LEN);
	entry->routeEntry.crflags |= CRF_DYNAMICLRU;
	nx_strlcpy(entry->routeEntry.trname, tdata->trigger.name, 
		TRIGGER_ATTR_LEN);

	if (AddRoute(entry) < 0)
	{
		// trigger already exists
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s route %s for Trigger %s already exists\n", 
			fn, entry->routeEntry.crname, tdata->trigger.name));

		routeAdded = 0;
	}
	else
	{

		// Check to see if the route exists
		if (entry == CacheGet(cpCache, crname))
		{
			routeAdded = 1;
	
			// schedule a db update to add this entry
			DbScheduleRouteUpdate(&entry->routeEntry);
			GisPostCliCRAddCmd(entry->routeEntry.crname, tags);
		}
		else
		{
			// If Add route was successful, but entry does not exist
			// then it is an auto lru delete
			entry = NULL;
		}
	}

#ifdef _separate_binding_
	if (AddCPB(cpbEntry) < 0)
	{
		// trigger already exists
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s binding %s/%s for Trigger %s already exists\n", 
			fn, cpbEntry->cpbEntry.cpname, cpbEntry->cpbEntry.crname, 
			tdata->trigger.name));

		bindingAdded = 0;
	}
	else
	{
		bindingAdded = 1;

		// schedule a db update to add this entry
		DbScheduleBindingUpdate(&cpbEntry->cpbEntry);
		GisPostCliCPBAddCmd(&cpbEntry->cpbEntry, tags);
	}
#endif

_return:
	CacheReleaseLocks(cpCache);
	CacheReleaseLocks(regCache);

	if (routeAdded || bindingAdded)
	{
		tgEntry.event = tdata->trigger.event;
		tgEntry.srcvendor = tdata->trigger.srcvendor;
		tgEntry.dstvendor = tdata->trigger.dstvendor;

		CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);
		tgPtr = CacheGet(triggerCache, &tgEntry);	

		if (tgPtr)
		{
			tgPtr->ntriggers += (routeAdded+bindingAdded);
		}
		else
		{
			NETERROR(MFIND, ("%s Could not update trigger count for %d\n",
				fn, tdata->trigger.event));
		}

		CacheReleaseLocks(triggerCache);

		// update local cache to keep track of trigger related routes
		// TBD
	}

	if (!routeAdded)
	{
		if (entry)
		{
			CFree(cpCache)(entry);
		}
	}

#ifdef _separate_binding_
	if (!bindingAdded)
	{
		if (cpbEntry)
		{
			CFree(cpbCache)(cpbEntry);
		}
	}
#endif

	free(tdata);
	return(NULL);
}

// Quickly set up a trigger event to be queued and get out of the call
// path
int
TriggerInstall(CacheTriggerEntry *tgPtr, CallHandle *callHandle1, 
				CallHandle *callHandle2)
{
	char fn[] = "TriggerInstall():";
	struct triggerdata *tdata;

	if ((tgPtr->trigger.actiondata[0] == '\0') ||
		(tgPtr->trigger.action != TRIGGER_ACTION_INSERTROUTE) ||
		(tgPtr->trigger.event != TRIGGER_EVENT_H323REQMODEFAX && 
		tgPtr->trigger.event != TRIGGER_EVENT_H323T38FAX))
	{
		NETERROR(MFIND, ("%s Bad trigger %s\n", fn, tgPtr->trigger.name));
		return 0;
	}

	tdata = (struct triggerdata *)malloc(sizeof(struct triggerdata));
	memset(tdata, 0 , sizeof(struct triggerdata));

	// Only one trigger is supported
	memcpy(&tdata->trigger, &tgPtr->trigger, sizeof(TriggerEntry));

	// This information is valid for route triggers only
	// must be segregated when triggers evolve
	nx_strlcpy(tdata->number, callHandle2->inputNumber, PHONE_NUM_LEN);
	nx_strlcpy(tdata->srcRouteNumber, callHandle2->dialledNumber, PHONE_NUM_LEN);
	nx_strlcpy(tdata->transitRouteNumber, callHandle2->transRouteNumber, PHONE_NUM_LEN);
	nx_strlcpy(tdata->transitcrname, callHandle2->transitcrname, CALLPLAN_ATTR_LEN);
	nx_strlcpy(tdata->srccrname, callHandle2->srccrname, CALLPLAN_ATTR_LEN);
	nx_strlcpy(tdata->regid, callHandle2->phonode.regid, REG_ID_LEN);
	tdata->uport = callHandle2->phonode.uport;
	tdata->routeflag = callHandle2->routeflag;

	nx_strlcpy(tdata->dregid, callHandle2->rfphonode.regid, REG_ID_LEN);
	tdata->duport = callHandle2->rfphonode.uport;

	// Dispatch for insertion
	if (ThreadDispatch(dbpoolid, dbclassid, TriggerCmdWorker,
		tdata, 1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0)
	{
		// Too many iedge updates are queued
		NETERROR(MDB, ("%s Could not post trigger for %s/%d in the db\n",
			fn, tdata->number, tdata->trigger.action));
		free(tdata);
	}

	return 0;
}
