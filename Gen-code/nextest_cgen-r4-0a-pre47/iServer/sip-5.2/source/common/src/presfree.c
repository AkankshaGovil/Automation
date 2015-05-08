 /******************************************************************************
 ** FUNCTION:
 **	 This file has the source for the free functions for the
 **	 Instant Messaging and Presence related headers
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		presfree.c
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE	NAME	       REFERENCE	   REASON
 ** -------------------------------------------------------------------------
 ** 03-12-03    Jyoti        Release 5.2 SRDS    Initial Creation
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#include "presstruct.h"

/*****************************************************************
** FUNCTION:sip_freePresUrl
**
**
** DESCRIPTION: This function shall free Presurl Structure.
*******************************************************************/
void sip_freePresUrl 
	(PresUrl *pUrl)
{
	SipError dErr;

	if (pUrl == SIP_NULL) 
		return;
	HSS_LOCKREF(pUrl->dRefCount);
	HSS_DECREF(pUrl->dRefCount);
	if(HSS_CHECKREF(pUrl->dRefCount))
	{

		HSS_FREE(pUrl->pDispName);
		HSS_FREE(pUrl->pUser);
		HSS_FREE(pUrl->pHost);
		if (pUrl->slRoute.size != 0)
			if(sip_listDeleteAll ( &(pUrl->slRoute), &dErr)==SipFail)
			{
				return;
			}
		if (pUrl->slParams.size != 0)
			if(sip_listDeleteAll ( &(pUrl->slParams), &dErr)==SipFail)
				return;
		HSS_UNLOCKREF(pUrl->dRefCount);
		HSS_DELETEREF(pUrl->dRefCount);
		HSS_FREE(pUrl);
	}
	else
	{
		HSS_UNLOCKREF(pUrl->dRefCount);
	}
}

