#include "gis.h"
#include "uh323inc.h"
#include "calldefs.h"
#include "sipcall.h"
#include <malloc.h>
#include <ctype.h>


void H323FreeEvData(H323EventData *ptr)
{
	header_url_list *elem, *list;

	if(ptr == NULL)
	{	
		return;
	}

	if(ptr->localSet)
	{
		free(ptr->localSet);
	}

	if(ptr->requri)
	{
		UrlFree(ptr->requri, MEM_LOCAL);
	}

	if(ptr->remotecontact_list)
	{
		list = ptr->remotecontact_list;
		do
		{
			elem = list;
			list = elem->next;
			UrlFree(elem->url, MEM_LOCAL);
			SipCheckFree(elem);
		}
		while(list != ptr->remotecontact_list);
	}
		
	// peer for all instances will be the same,
	// and since this function can be called from
	// almost anywhere, we can just use instance 0
	// specifically. UH323Globals() may not return valid
	// data.
	if (ptr->nodeId > 0)
	{
			freeNodeTree(uh323Globals[0].peerhApp, ptr->nodeId, 0);
	}

	if (ptr->q931NodeId > 0)
	{
			freeNodeTree(uh323Globals[0].peerhApp, ptr->q931NodeId, 0);
	}

	if (ptr->confID)
	{
		free(ptr->confID);
	}

	if (ptr->display)
	{
		free(ptr->display);
	}

	if(ptr->dtmf)
	{
		free(ptr->dtmf);
	}

	free (ptr);
}


void H323FreeEvent(SCC_EventBlock *evtPtr)
{
	if(evtPtr == NULL)
	{	
		return;
	}

	if(evtPtr->data)
	{
		H323FreeEvData(evtPtr->data);
	}

	free (evtPtr);
}


int h323HuntError(int callError,int cause)
{
    // Hunt based on ISDN cause code
    if (VALID_ISDNCODE(cause))
    {
		return codemap[cause].hunt;
    }
 
    // Hunt based on call error
   	return HuntError(callError);
}

int ise164(char phone[PHONE_NUM_LEN])
{
	char *p;
	for(p=phone;*p;++p)
	{
		if(isdigit(*p)|| *p== '#' || *p == '*' ||*p==',')
		{
			continue;	
		}
		return FALSE;
	}
	return TRUE;
}
