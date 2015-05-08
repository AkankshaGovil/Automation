 /******************************************************************************
 ** FUNCTION:
 **	 This file has all the source for init functions for Instant Messaging 
 **	 and Presence Related Headers
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		presinit.c
 **
 ** DESCRIPTION:
 **	  This file lists all the  pres initialization related functions
 **
 ** DATE	     NAME	       REFERENCE	   REASON
 ** -------------------------------------------------------------------------
 ** 03-12-03   Jyoti      Release 5.2 SRDS    Initial Creation
 **
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#include "presstruct.h"
#include "sipfree.h"
#include "sipinit.h"

/****************************************************************************
** FUNCTION:	sip_initPresUrl
**
**
** DESCRIPTION: This function shall allocate memory for PresUrl and
**              initialize all the fields.   
*****************************************************************************/
SipBool sip_initPresUrl
	(PresUrl **ppPres,SipError *pErr)
{
	*ppPres = (PresUrl *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(PresUrl),pErr);
	if (*ppPres==SIP_NULL)
		return SipFail;
	INIT((*ppPres)->pDispName);
	INIT((*ppPres)->pUser);
	INIT((*ppPres)->pHost);
	sip_listInit(& ((*ppPres)->slRoute),__sip_freeString, pErr);
	sip_listInit(& ((*ppPres)->slParams),__sip_freeSipParam,pErr);
	HSS_INITREF((*ppPres)->dRefCount);
	return SipSuccess;
}

