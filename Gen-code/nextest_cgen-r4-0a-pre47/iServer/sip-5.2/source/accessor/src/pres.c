 /******************************************************************************
 ** FUNCTION:
 **	 	This file has all accessor API source of Instant Messaging and
 **     Presence Related Structures (Pres Url)
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		pres.c
 **
 ** DESCRIPTION:
 **      This file contains all the pres-url related accessor functions.
 **
 ** DATE	     NAME	       REFERENCE	         REASON
 ** ----------------------------------------------------------------------
 ** 03-12-03    Jyoti     Release 5.2 SRDS    Initial Creation
 **
 **	Copyright 2001, Hughes Software Systems, Ltd.
 ******************************************************************************/

#include "presstruct.h"
#include "pres.h"
#include "sipcommon.h"
#include "sipinternal.h"
#include "request.h"
#include "sipstatistics.h"
#include "imurldecodeintrnl.h"
#include "sipformmessage.h"
#include "sipclone.h"




/* PRES-URL Related APIs */

/***********************************************************************
** Function: sip_isPresUrl
** Description: Checks if the Addrspec has a pres-url
** Parameters:
**		pAddrSpec (IN)- SipAddrSpec
**		pErr (OUT)    - Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_isPresUrl
	(SipAddrSpec *pAddrSpec, SipError *pErr)
{
	en_AddrType dAddrtype;
	SIP_S8bit *pTempUri=SIP_NULL;
	SIP_U32bit i=0;


	SIPDEBUGFN("Entering sip_isPresUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( sip_getAddrTypeFromAddrSpec(pAddrSpec,&dAddrtype,pErr) != \
		SipSuccess )
	{
		SIPDEBUGFN("Exiting sip_isPresUrl\n");
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if ( dAddrtype == SipAddrReqUri )
	{

		pTempUri = pAddrSpec->u.pUri;

		i=0;
		if (( pTempUri[i] == 'P' || pTempUri[i] == 'p') &&
		    ( pTempUri[i+1] == 'R' || pTempUri[i+1] == 'r') && 
		    ( pTempUri[i+2] == 'E' || pTempUri[i+2] == 'e') && 
		    ( pTempUri[i+3] == 'S' || pTempUri[i+3] == 's') )
		{
		 for (i=4;((pTempUri[i] == ' ') || (pTempUri[i] == '\t'))&& \
				(pTempUri[i] != '\0');i++);

		 if (pTempUri[i] == ':')
		 {
			*pErr = E_NO_ERROR;
			SIPDEBUGFN("Exiting sip_isPresUrl\n");
			return SipSuccess;
		 }
		}/* if */
	}
	SIPDEBUGFN("Exiting sip_isPresUrl \n");
	*pErr = E_NO_EXIST;
	return SipFail;
}

/***********************************************************************
** Function   : sip_getPresUrlFromAddrSpec
** Description: This function gets the PresUrl from the 
**              SipAddrSpec structure
** Parameters :
**		pAddrSpec (IN)	- SipAddrSpec
**		pPresUrl (OUT)	- Retrieved PresUrl
**		pErr (OUT)      - Possible Error value (see API ref doc)
************************************************************************/
#ifdef SIP_BY_REFERENCE
SipBool sip_getPresUrlFromAddrSpec (
	SipAddrSpec *pAddrSpec, PresUrl **ppPresUrl, SipError *pErr)
#else
SipBool sip_getPresUrlFromAddrSpec (
	SipAddrSpec *pAddrSpec, PresUrl *pPresUrl, SipError *pErr)
#endif
{
	SIP_S8bit *pParserBuffer=SIP_NULL;
	SIP_S32bit dLen=0;
	SipImParserParam *pImParserParam=SIP_NULL;

	INC_API_COUNT

	SIPDEBUGFN ("Entering sip_getPresUrlFromAddrSpec\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
	{
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pPresUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}

#else
	if (ppPresUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
#endif

	if (pAddrSpec->u.pUri == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
	if ( sip_isPresUrl(pAddrSpec, pErr) == SipFail)
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
#endif
	dLen = strlen(pAddrSpec->u.pUri);
	pImParserParam = (SipImParserParam*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipImParserParam), pErr);
	if(pImParserParam==SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
	pImParserParam->pError = (SipError*)fast_memget( \
		ACCESSOR_MEM_ID,sizeof(SipError),pErr);
	if(pImParserParam->pError==SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
                fast_memfree (ACCESSOR_MEM_ID, (pImParserParam), SIP_NULL);
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
	pParserBuffer =(SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, dLen+2, pErr);
	if(pParserBuffer == SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
                fast_memfree (ACCESSOR_MEM_ID, (pImParserParam->pError), SIP_NULL);
                fast_memfree (ACCESSOR_MEM_ID, (pImParserParam), SIP_NULL);
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}
	pImParserParam->pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),pErr);
       if(pImParserParam->pGCList==SIP_NULL)
       {
               *pErr = E_NO_MEM;
               fast_memfree (ACCESSOR_MEM_ID, (pImParserParam->pError), SIP_NULL);
               fast_memfree (ACCESSOR_MEM_ID, (pImParserParam), SIP_NULL);
	       fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
	       SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
	       return SipFail;
       }
       sip_listInit((pImParserParam->pGCList),&sip_freeVoid,pErr);
       if(*pErr !=E_NO_ERROR)
       {
	     *pErr = E_NO_MEM;
	     fast_memfree(ACCESSOR_MEM_ID,pImParserParam->pGCList,SIP_NULL);
             fast_memfree (ACCESSOR_MEM_ID, (pImParserParam->pError), SIP_NULL);
             fast_memfree (ACCESSOR_MEM_ID, (pImParserParam), SIP_NULL);
	     fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
	     SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
	     return SipFail;
       }

       strcpy(pParserBuffer,pAddrSpec->u.pUri );
       pParserBuffer[dLen+1]='\0';
       *(pImParserParam->pError) = E_NO_ERROR;
       if (sip_lex_Im_scan_buffer(pParserBuffer, dLen+2) != 0)
       {
	 INC_ERROR_MEM
         fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
	 if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
	 {
		 SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		 return SipFail;
	 }
	 fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pGCList),SIP_NULL);
	 fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
	 fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
	 *pErr = E_NO_MEM;
	 SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
	 return SipFail;
       }
       sip_lex_Im_reset_state();
       pImParserParam->pImUrl = SIP_NULL;
       glbSipParserImparse((void *)pImParserParam);
       sip_lex_Im_release_buffer();
       fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);

       if (*(pImParserParam->pError) != E_NO_ERROR)
       {
#ifdef SIP_STATISTICS
	    INC_SECOND_ERROR_PROTOCOL
#endif

            *pErr = E_PARSER_ERROR;
	    fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
	    if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
	    {
		    SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		    return SipFail;
	    }
	    fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pGCList),SIP_NULL);
	    fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
	    SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
	    return SipFail;
       }
#ifdef SIP_STATISTICS
       INC_SECOND_API_REQ_PARSED
#endif

#ifdef SIP_BY_REFERENCE
        *ppPresUrl = (PresUrl *) (pImParserParam->pImUrl) ;
#else
       if (sip_clonePresUrl(pPresUrl, (PresUrl *)pImParserParam->pImUrl,\
			       pErr) == SipFail)
	{
	    sip_freePresUrl((PresUrl *)(pImParserParam->pImUrl));
	    fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
	    if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
	    {
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	    }
	    fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pGCList),SIP_NULL);
	    fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
	    SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
	    return SipFail;
	}
	sip_freePresUrl((PresUrl *)(pImParserParam->pImUrl));
#endif

	fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pError),SIP_NULL);
	if(sip_listDeleteAll((pImParserParam->pGCList),pErr)==SipFail)
	{
	        fast_memfree(ACCESSOR_MEM_ID,(pImParserParam->pGCList),SIP_NULL);
	        fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
		SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
		return SipFail;
	}

	fast_memfree(ACCESSOR_MEM_ID,pImParserParam->pGCList,SIP_NULL);
	fast_memfree(ACCESSOR_MEM_ID,(pImParserParam),SIP_NULL);
	SIPDEBUGFN ("Exiting sip_getPresUrlFromAddrSpec\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setPresUrlInAddrSpec
** Description: sets the PresUrl in the SipAddrSpec structure
** Parameters:
**	pAddrSpec (IN)	- SipAddrSpec
**	pPresUrl (OUT)	- PresUrl to be set
**	pErr (OUT)	- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setPresUrlInAddrSpec
	(SipAddrSpec *pAddrSpec, PresUrl *pPresUrl, SipError *pErr)
{
	SIP_S8bit *pOut=SIP_NULL;
	SIP_U32bit dListSize=0;

	SIPDEBUGFN("Entering sip_setPresUrlInAddrSpec\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting sip_setPresUrlInAddrSpec\n");
		return SipFail;
	}
	if( pAddrSpec->dType != SipAddrReqUri )
	{
		*pErr = E_INV_TYPE;
		SIPDEBUGFN("Exiting sip_setPresUrlInAddrSpec\n");
		return SipFail;
	}
#endif
	if ( pAddrSpec->u.pUri != SIP_NULL)
	{
	    if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pAddrSpec->u.pUri),\
			pErr) ==SipFail)
	    {
		 SIPDEBUGFN("Exiting sip_setPresUrlInAddrSpec\n");
		 return SipFail;
	    }
	}

	if (pPresUrl==SIP_NULL)
	{
	    pAddrSpec->u.pUri=SIP_NULL;
	    *pErr=E_NO_ERROR;
	}
	else
	{
	   pOut=(SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID,MAX_IM_URL_SIZE, pErr);
	   if (pOut == SIP_NULL)
	   {
		   *pErr = E_NO_MEM;
		   SIPDEBUGFN("Exiting sip_setPresUrlInAddrSpec\n");
		   return SipFail;
	   }
	   strcpy(pOut,"pres:");

	   if(pPresUrl->pDispName != SIP_NULL)
	   {
		   STRCAT(pOut,pPresUrl->pDispName);
		   STRCAT(pOut,"%3c");
	   }

	   sip_formSipList(pOut, &(pPresUrl->slRoute), (SIP_S8bit *) ",",1,\
			   pErr);

	   if(pPresUrl->pUser != SIP_NULL)
	   {
		   STRCAT(pOut,pPresUrl->pUser);
	   }

	   if(pPresUrl->pHost != SIP_NULL)
	   {
		   STRCAT(pOut,"@");
		   STRCAT(pOut,pPresUrl->pHost);
	   }

	   if(pPresUrl->pDispName != SIP_NULL)
	   {
		   STRCAT(pOut,"%3e");
	   }

	   /* Now Form Header */
	   sip_listSizeOf(&(pPresUrl->slParams), &dListSize, pErr);
	   if(dListSize!=0)
	   {
		   STRCAT(pOut,"?");
	   }

	   /* Called with leadingsep value of 2 to avoid putting leading 
	    * space or leading separator */
	   sip_formSipParamList(0,&pOut, &(pPresUrl->slParams), \
			   (SIP_S8bit *) "&", 2, pErr);

	   pAddrSpec->u.pUri=pOut;
	}

	SIPDEBUGFN("Exiting sip_setPresUrlInAddrSpec\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getDispNameFromPresUrl
** Description: gets the Display Name field from the PresUrl structure
** Parameters:
**		pUrl (IN)	- PresUrl
**		ppDispName (OUT)- retrieved Display Name
**		pErr (OUT	- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getDispNameFromPresUrl
	(PresUrl *pUrl,
	SIP_S8bit **ppDispName,
	SipError *pErr)
{
	SIPDEBUGFN("Entering sip_getDispNameFromPresUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (( pUrl == SIP_NULL) || (ppDispName == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting sip_getDispNameFromPresUrl\n");
		return SipFail;
	}
#endif

	if (pUrl->pDispName == SIP_NULL)
	{
		*pErr=E_NO_EXIST;
		SIPDEBUGFN("Exiting sip_getDispNameFromPresUrl\n");
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
 	*ppDispName = (SIP_S8bit *) STRDUPACCESSOR (pUrl->pDispName);
	if (*ppDispName == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		SIPDEBUGFN("Exiting sip_getDispNameFromPresUrl\n");
		return SipFail;
	}
#else
	*ppDispName = pUrl->pDispName;
#endif

	SIPDEBUGFN("Exiting sip_getDispNameFromPresUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setDispNameInPresUrl
** Description: sets the Display Name in the PresUrl structure
** Parameters:
**		pUrl (IN/OUT)	- PresUrl
**		pDispName (OUT)	- Display Name to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setDispNameInPresUrl
	(PresUrl *pUrl,
	SIP_S8bit *pDispName,
	SipError *pErr)
{
	SIP_S8bit *pTempDispName=SIP_NULL;
	SIP_S8bit *pName=SIP_NULL;

	SIPDEBUGFN("Entering sip_setDispNameInPresUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
	{
		SIPDEBUGFN("Exiting sip_setDispNameInPresUrl\n");
		return SipFail;
	}
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting sip_setDispNameInPresUrl\n");
		return SipFail;
	}
#endif

	if( pDispName == SIP_NULL)
		pTempDispName = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
	    pTempDispName = (SIP_S8bit *)STRDUPACCESSOR(pDispName);
	    if (pTempDispName == SIP_NULL)
	    {
		    *pErr = E_NO_MEM;
		    SIPDEBUGFN("Exiting sip_setDispNameInPresUrl\n");
		    return SipFail;
	    }
#else
	    pTempDispName = pDispName;
#endif
	}

	pName = pUrl->pDispName;
	if (pName != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, \
			(SIP_Pvoid *)(&pName), pErr) ==  SipFail)
		{
#ifndef SIP_BY_REFERENCE
			sip_freeString(pTempDispName);
#endif
			SIPDEBUGFN("Exiting sip_setDispNameInPresUrl\n");
			return SipFail;
		}
	}

	pUrl->pDispName = pTempDispName;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_setDispNameInPresUrl\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getUserFromPresUrl
** Description: gets the User Name field from the PresUrl structure
** Parameters:
**		pUrl (IN)   - PresUrl
**		ppUser (OUT)- retrieved User
**		pErr (OUT)  - Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getUserFromPresUrl
	(PresUrl *pUrl,
	SIP_S8bit **ppUser,
	SipError *pErr)
{
	SIPDEBUGFN("Entering sip_getUserFromPresUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (( pUrl == SIP_NULL) || (ppUser  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting sip_getUserFromPresUrl\n");
		return SipFail;
	}
#endif

	if (pUrl->pUser == SIP_NULL)
	{
		*pErr=E_NO_EXIST;
		SIPDEBUGFN("Exiting sip_getUserFromPresUrl\n");
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
 	*ppUser = (SIP_S8bit *) STRDUPACCESSOR (pUrl->pUser);
	if (*ppUser == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		SIPDEBUGFN("Exiting sip_getUserFromPresUrl\n");
		return SipFail;
	}
#else
	*ppUser = pUrl->pUser;
#endif

	SIPDEBUGFN("Exiting sip_getUserFromPresUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setUserInPresUrl
** Description: sets the User in the PresUrl structure
** Parameters:
**		pUrl (IN/OUT)	- PresUrl
**		pUser (OUT)		- User to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setUserInPresUrl
	(PresUrl *pUrl,
	SIP_S8bit *pUser,
	SipError *pErr)
{
	SIP_S8bit *pTempUserName=SIP_NULL;
	SIP_S8bit *pUsertemp=SIP_NULL;

	SIPDEBUGFN("Entering sip_setUserInPresUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
	{
	    SIPDEBUGFN("Exiting sip_setUserInPresUrl\n");
	    return SipFail;
	}
	if ( pUrl == SIP_NULL)
	{
	    *pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting sip_setUserInPresUrl\n");
	    return SipFail;
	}
#endif

	if( pUser == SIP_NULL)
	    pTempUserName = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
	    pTempUserName = (SIP_S8bit *)STRDUPACCESSOR(pUser);
	    if (pTempUserName == SIP_NULL)
	    {
		    *pErr = E_NO_MEM;
		    SIPDEBUGFN("Exiting sip_setUserInPresUrl\n");
		    return SipFail;
	    }
#else
	    pTempUserName = pUser;
#endif
	}

	pUsertemp = pUrl->pUser;
	if (pUsertemp != SIP_NULL)
	{
	   if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&pUsertemp)\
				   , pErr) ==  SipFail)
	   {
#ifndef SIP_BY_REFERENCE
		   sip_freeString(pTempUserName);
#endif
		   SIPDEBUGFN("Exiting sip_setUserInPresUrl\n");
		   return SipFail;
	   }
	}

	pUrl->pUser = pTempUserName;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_setUserInPresUrl\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getHostFromPresUrl
** Description: gets the Host Name field from the PresUrl structure
** Parameters:
**		pUrl (IN)   - PresUrl
**		ppHost (OUT)- Retrieved Host
**		pErr (OUT)  - Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getHostFromPresUrl
	(PresUrl *pUrl,
	SIP_S8bit **ppHost,
	SipError *pErr)
{
	SIPDEBUGFN("Entering sip_getHostFromPresUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
	{
	     SIPDEBUGFN("Exiting sip_getHostFromPresUrl\n");
	     return SipFail;
	}
	if (( pUrl == SIP_NULL) || (ppHost  == SIP_NULL))
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN("Exiting sip_getHostFromPresUrl\n");
	     return SipFail;
	}
#endif

	if (pUrl->pHost == SIP_NULL)
	{
	    *pErr=E_NO_EXIST;
	    SIPDEBUGFN("Exiting sip_getHostFromPresUrl\n");
	    return SipFail;
	}

#ifndef SIP_BY_REFERENCE
 	*ppHost = (SIP_S8bit *) STRDUPACCESSOR (pUrl->pHost);
	if (*ppHost == SIP_NULL)
	{
	     *pErr = E_NO_MEM;
	     SIPDEBUGFN("Exiting sip_getHostFromPresUrl\n");
	     return SipFail;
	}
#else
	*ppHost = pUrl->pHost;
#endif

	SIPDEBUGFN("Exiting sip_getHostFromPresUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setHostInPresUrl
** Description: sets the Host in the PresUrl structure
** Parameters:
**		pUrl (IN/OUT)	- PresUrl
**		pHost (OUT)		- Host to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setHostInPresUrl
	(PresUrl *pUrl,
	SIP_S8bit *pHost,
	SipError *pErr)
{
	SIP_S8bit *pTempHostName=SIP_NULL;
	SIP_S8bit *pHosttemp=SIP_NULL;

	SIPDEBUGFN("Entering sip_setHostInPresUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
	{
	     SIPDEBUGFN("Exiting sip_setHostInPresUrl\n");
	     return SipFail;
	}
	if ( pUrl == SIP_NULL)
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN("Exiting sip_setHostInPresUrl\n");
	     return SipFail;
	}
#endif

	if( pHost == SIP_NULL)
	     pTempHostName = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
	     pTempHostName = (SIP_S8bit *)STRDUPACCESSOR(pHost);
	     if (pTempHostName == SIP_NULL)
	     {
		     *pErr = E_NO_MEM;
		     SIPDEBUGFN("Exiting sip_setHostInPresUrl\n");
		     return SipFail;
	     }
#else
	     pTempHostName = pHost;
#endif
	}

	pHosttemp = pUrl->pHost;
	if (pHosttemp != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID,\
			 (SIP_Pvoid *)(&pHosttemp), pErr) ==  SipFail)
		{
#ifndef SIP_BY_REFERENCE
			sip_freeString(pTempHostName);
#endif
			SIPDEBUGFN("Exiting sip_setHostInPresUrl\n");
			return SipFail;
		}
	}

	pUrl->pHost = pTempHostName;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_setHostInPresUrl\n");
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getParamCountFromPresUrl
** Description: gets the number of parameters in PresUrl
** Parameters:
**		pPresUrl (IN)- PresUrl
**		pCount (OUT) - Number of parameters
**		pErr (OUT)   - Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getParamCountFromPresUrl
	(PresUrl *pPresUrl,SIP_U32bit *pCount, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_getParamCountFromPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
	    SIPDEBUGFN("Exiting function sip_getParamCountFromPresUrl");
	    return SipFail;
	}

	if ( pPresUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getParamCountFromPresUrl");
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN("Exiting function sip_getParamCountFromPresUrl");
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(pPresUrl)->slParams, pCount , pErr) == SipFail )
	{
		SIPDEBUGFN("Exiting function sip_getParamCountFromPresUrl");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamCountFromPresUrl");
	return SipSuccess;
}

/*****************************************************************************
** Function: sip_getParamAtIndexFromPresUrl
** Description: gets the Param at the specified index in PresUrl
** Parameters:
**	pPresUrl (IN)- PresUrl
**	ppParam(OUT) - Retreived Parameter
**	dIndex (IN)  - Index at which param is to be retieved
**	pErr (OUT)   - Possible Error value (see API ref doc)
******************************************************************************/

SipBool sip_getParamAtIndexFromPresUrl
#ifdef SIP_BY_REFERENCE
	(PresUrl *pPresUrl,SipParam **ppParam, SIP_U32bit dIndex,\
		SipError *pErr)
#else
	(PresUrl *pPresUrl,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#endif
{
	SIP_Pvoid pElementfromList=SIP_NULL ;
	SipParam *pTempParam=SIP_NULL ;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
	   SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPresUrl");
	   return SipFail;
	}

	if ( pPresUrl == SIP_NULL )
	{
	    *pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPresUrl");
	    return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
    	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPresUrl");
	     return SipFail;
	}

#endif

	if (sip_listGetAt( &(pPresUrl->slParams), dIndex,  \
		&pElementfromList, pErr) == SipFail)
	{
	     SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPresUrl");
	     return SipFail;
	}

	if (pElementfromList == SIP_NULL)
	{
	      *pErr = E_NO_EXIST;
	      SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPresUrl");
	      return SipFail;
	}

	pTempParam = (SipParam *)pElementfromList;

#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, pTempParam, pErr) == SipFail)
	{
	      SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPresUrl");
	      return SipFail;
	}
#else
	HSS_LOCKEDINCREF(pTempParam->dRefCount);
	*ppParam = pTempParam;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_getParamAtIndexFromPresUrl");
	return SipSuccess;
}

/*****************************************************************************
** Function    : sip_setParamAtIndexInPresUrl
** Description : This function sets the Param at specified index in PresUrl
** Parameters  :
**	pPresUrl (IN/OUT)- PresUrl
**	pParam(IN)	 - Param to be set
**	dIndex (IN)	 - Index at which param is set in PresUrl
**	pErr (OUT)       - Possible Error value (see API ref doc)
******************************************************************************/

SipBool sip_setParamAtIndexInPresUrl
	(PresUrl *pPresUrl,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
{
	SipParam *pTempParam=SIP_NULL;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
	   SIPDEBUGFN("Exiting function sip_setParamAtIndexInPresUrl");
	   return SipFail;
	}

	if ( pPresUrl == SIP_NULL )
	{
	    *pErr = E_INV_PARAM;
	    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPresUrl");
	    return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&pTempParam, pErr) == SipFail)
		{
		   SIPDEBUGFN("Exiting function sip_setParamAtIndexInPresUrl");
		   return SipFail;
		}
		if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
		{
		    sip_freeSipParam (pTempParam);
		    SIPDEBUGFN("Exiting function sip_setParamAtIndexInPresUrl");
		    return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		pTempParam = pParam;
#endif
	}

	if( sip_listSetAt( &(pPresUrl->slParams),  \
		dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		SIPDEBUGFN("Exiting function sip_setParamAtIndexInPresUrl");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_setParamAtIndexInPresUrl");
	return SipSuccess;
}
/******************************************************************************
** Function    : sip_insertParamAtIndexInPresUrl
** Description : This function inserts the Param at the 
**               specified index in PresUrl
** Parameters :
**	pPresUrl (IN/OUT)- PresUrl
**	pParam(IN)	 - Param to be inserted
**	dIndex (IN)	 - Index at which param is inserted in PresUrl
**	pErr (OUT)	 - Possible Error value (see API ref doc)
******************************************************************************/

SipBool sip_insertParamAtIndexInPresUrl
	(PresUrl *pPresUrl, SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
{
	SipParam *pTempParam=SIP_NULL;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
	    SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPresUrl");
	    return SipFail;
	}

	if ( pPresUrl == SIP_NULL )
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPresUrl");
	     return SipFail;
	}

#endif
	if ( pParam == SIP_NULL )
		pTempParam = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
	     if (sip_initSipParam(&pTempParam, pErr) == SipFail)
		     return SipFail;
	     if (__sip_cloneSipParam(pTempParam, pParam, pErr) == SipFail)
	     {
		 sip_freeSipParam (pTempParam);
		 SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPresUrl");
		 return SipFail;
	     }
#else
	     HSS_LOCKEDINCREF(pParam->dRefCount);
	     pTempParam = pParam;
#endif
	}

	if( sip_listInsertAt( &(pPresUrl->slParams),  \
		dIndex, (SIP_Pvoid)(pTempParam), pErr) == SipFail)
	{
		if (pTempParam != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (pTempParam);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPresUrl");
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_insertParamAtIndexInPresUrl");
	return SipSuccess;
}

/***********************************************************************
** Function   : sip_deleteParamAtIndexInPresUrl
** Description: This function deletes the param at the 
**              specified index in PresUrl
** Parameters:
**	pPresUrl (IN)		- PresUrl
**	dIndex (IN)		- index at which param is deleted in PresUrl
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_deleteParamAtIndexInPresUrl
(PresUrl *pPresUrl, SIP_U32bit dIndex, SipError *pErr)
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
	    SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPresUrl");
	    return SipFail;
	}

	if ( pPresUrl == SIP_NULL )
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPresUrl");
	     return SipFail;
	}

#endif
	if( sip_listDeleteAt( &(pPresUrl->slParams), dIndex, pErr) == SipFail)
	{
	     SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPresUrl");
	     return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_deleteParamAtIndexInPresUrl");
	return SipSuccess;
}


/***************************************************************
** Function: sip_getRouteCountFromPresUrl
** Description: gets the Route count from the PresUrl Structure
** Parameters:
**	pPresUrl (IN)- PresUrl
**	pCount (OUT) - Number of PresUrl Routes
**	pErr (OUT)   - Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getRouteCountFromPresUrl
	(PresUrl *pPresUrl,
	SIP_U32bit *pCount,
	SipError *pErr)
{
	SIPDEBUGFN ( "Entering getRouteCountFromPresUrl");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
	{
	     SIPDEBUGFN ( "Exiting getRouteCountFromPresUrl");
	     return SipFail;
	}

	if ( (pPresUrl == SIP_NULL) || ( pCount == SIP_NULL ))
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN ( "Exiting getRouteCountFromPresUrl");
	     return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pPresUrl->slRoute), pCount , pErr) == SipFail )
	{
	     SIPDEBUGFN ( "Exiting getRouteCountFromPresUrl");
	     return SipFail;
	}

	SIPDEBUGFN ( "Exiting getRouteCountFromPresUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_getRouteAtIndexFromPresUrl
** Description: gets the Route field at the index from PresUrl
** Parameters:
**	pPresUrl (IN)- PresUrl
**	ppRoute(OUT) - Retreived Route
**	dIndex (IN)  - Index at which Route is to be retrieved
**	pErr (OUT)   - Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getRouteAtIndexFromPresUrl
#ifdef SIP_BY_REFERENCE
	(PresUrl *pPresUrl,
	 SIP_S8bit **ppRoute,
	 SIP_U32bit dIndex,
	 SipError 	*pErr )
#else
	(PresUrl *pPresUrl,
	 SIP_S8bit *pRoute,
	 SIP_U32bit dIndex,
	 SipError 	*pErr )
#endif
{
	SIP_Pvoid pElementFromList=SIP_NULL;
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dSize=0;
#endif

	SIPDEBUGFN ( "Entering sip_getRouteAtIndexFromPresUrl ");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromPresUrl");
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	if ( (pPresUrl == SIP_NULL) || (ppRoute == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromPresUrl");
		return SipFail;
	}
#else
	if ( (pPresUrl == SIP_NULL) || (pRoute == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromPresUrl");
		return SipFail;
	}
#endif
#endif
	if( sip_listGetAt(&(pPresUrl->slRoute), dIndex, &pElementFromList, \
		pErr) == SipFail)
	{
		SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromPresUrl");
		return SipFail;
	}

	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromPresUrl");
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppRoute = (SIP_S8bit*)pElementFromList;
#else
	dSize = strlen( (SIP_S8bit * )pElementFromList);
	pRoute = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, dSize +1, \
		pErr);
	if(pRoute == SIP_NULL)
	{
		SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromPresUrl");
		return SipFail;
	}

	strcpy(pRoute, (SIP_S8bit*)pElementFromList);
#endif
	SIPDEBUGFN ( "Exiting sip_getRouteAtIndexFromPresUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setRouteAtIndexInPresUrl
** Description: sets the Route field at the index in PresUrl
** Parameters:
**	pPresUrl (IN/OUT)- PresUrl
**	pRoute(IN)	- Route to be set
**	dIndex (IN)	- Index at which Route is to be set
**	pErr (OUT)	- Possible Error value (see API ref doc)
************************************************************************/

SipBool  sip_setRouteAtIndexInPresUrl
	(PresUrl *pPresUrl,
	 SIP_S8bit *pRoute,
	 SIP_U32bit dIndex,
	 SipError 	*pErr )
{

	SIP_S8bit * pElementFromList=SIP_NULL;

	SIPDEBUGFN ( "Entering sip_setRouteAtIndexInPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
	     SIPDEBUGFN ( "Exiting sip_setRouteAtIndexInPresUrl");
	     return SipFail;
	}

	if (pPresUrl == SIP_NULL)
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN ( "Exiting sip_setRouteAtIndexInPresUrl");
	     return SipFail;
	}
#endif
	if (pRoute  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pRoute;
#else

		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pRoute) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pRoute );
#endif
	}

	if( sip_listSetAt(&(pPresUrl->slRoute), dIndex, pElementFromList, \
		pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( pElementFromList != SIP_NULL )
		{
		    sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)\
				    (&(pElementFromList)),  pErr);
		}
#endif
		SIPDEBUGFN ( "Exiting sip_setRouteAtIndexInPresUrl");
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting sip_setRouteAtIndexInPresUrl");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_insertRouteAtIndexInPresUrl
** Description: inserts the Route field at the index in PresUrl
** Parameters:
**	pPresUrl (IN/OUT)- PresUrl
**	pRoute(IN)	 - Route to be inserted
**	dIndex (IN)	 - index at which Route is to be inserted
**	pErr (OUT)	 - Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_insertRouteAtIndexInPresUrl
	(PresUrl *pPresUrl,
	 SIP_S8bit *pRoute,
	 SIP_U32bit	dIndex,
	 SipError 	*pErr )
{

	SIP_S8bit * pElementFromList=SIP_NULL;

	SIPDEBUGFN ( "Entering sip_insertRouteAtIndexInPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
		SIPDEBUGFN ( "Exiting sip_insertRouteAtIndexInPresUrl");
		return SipFail;
	}

	if ( pPresUrl == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		SIPDEBUGFN ( "Exiting sip_insertRouteAtIndexInPresUrl");
		return SipFail;
	}
#endif
	/* copying the Value Headers structure/char*  */
	if (pRoute  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pRoute	;
#else
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pRoute) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pRoute );
#endif
	}

	if( sip_listInsertAt(&(pPresUrl->slRoute), dIndex, pElementFromList,\
		pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
		{
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)\
					(&(pElementFromList)),pErr);
		}
		SIPDEBUGFN ( "Exiting sip_insertRouteAtIndexInPresUrl");
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting sip_insertRouteAtIndexInPresUrl");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_deleteRouteAtIndexInPresUrl
** Description: deletes the Route field at the dIndex in PresUrl
** Parameters:
**	pPresUrl (IN/OUT)		- PresUrl
**	pRoute(IN)			- Route to be deleted
**	dIndex (IN)			- index at which Route is to be deleted
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_deleteRouteAtIndexInPresUrl
	(PresUrl *pPresUrl,
	  SIP_U32bit dIndex,
	  SipError 	*pErr )
{

	SIPDEBUGFN ( "Entering sip_deleteRouteAtIndexInPresUrl");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
	{
	    SIPDEBUGFN ( "Exiting sip_deleteRouteAtIndexInPresUrl");
	    return SipFail;
	}

	if ( pPresUrl  == SIP_NULL )
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN ( "Exiting sip_deleteRouteAtIndexInPresUrl");
	     return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pPresUrl->slRoute), dIndex, pErr) == SipFail)
	{
	      SIPDEBUGFN ( "Exiting sip_deleteRouteAtIndexInPresUrl");
	      return SipFail;
	}

	SIPDEBUGFN ( "Exiting sip_deleteRouteAtIndexInPresUrl");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_clonePresUrl
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the  PresUrl structures "from" to "to".
** Parameters:
**	pTo (OUT) - PresUrl
**	pFrom (IN)- PresUrl which has to be cloned
**	pErr (OUT)- Possible Error value (see API ref doc)
**********************************************************/

SipBool sip_clonePresUrl
	(PresUrl *pTo,
	PresUrl *pFrom,
	SipError *pErr)
{
	SIP_U32bit dLength=0;

	SIPDEBUGFN("Entering function sip_clonePresUrl");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
	{
	     SIPDEBUGFN("Exiting function sip_clonePresUrl");
	     return SipFail;
	}
	if ((pTo == SIP_NULL)||(pFrom  == SIP_NULL))
	{
	     *pErr = E_INV_PARAM;
	     SIPDEBUGFN("Exiting function sip_clonePresUrl");
	     return SipFail;
	}
#endif
	if(pTo->pDispName  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID,\
			(SIP_Pvoid*)(&(pTo->pDispName)), pErr) == SipFail)
		{
			SIPDEBUGFN("Exiting function sip_clonePresUrl");
			return SipFail;
		}
	}
	if(pTo->pUser  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(pTo->pUser))\
					, pErr)  == SipFail)
		{
			SIPDEBUGFN("Exiting function sip_clonePresUrl");
			return SipFail;
		}
	}
	if(pTo->pHost  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(pTo->pHost))\
					, pErr)  == SipFail)
		{
			SIPDEBUGFN("Exiting function sip_clonePresUrl");
			return SipFail;
		}
	}
	if (sip_listDeleteAll(&(pTo->slParams), pErr) == SipFail)
	{
		SIPDEBUGFN("Exiting function sip_clonePresUrl");
		return SipFail;
	}

	if (sip_listDeleteAll(&(pTo->slRoute), pErr) == SipFail)
	{
		SIPDEBUGFN("Exiting function sip_clonePresUrl");
		return SipFail;
	}

	/* cleaning of to over */

	if(pFrom->pDispName== SIP_NULL)
	{
		pTo->pDispName=SIP_NULL;
	}
	else
	{
		dLength = strlen(pFrom->pDispName );

		pTo->pDispName=(SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
				dLength+1,pErr);
		if ( pTo->pDispName == SIP_NULL )
		{
		     *pErr = E_NO_MEM;
		     SIPDEBUGFN("Exiting function sip_clonePresUrl");
		     return SipFail;
		}
		strcpy( pTo->pDispName, pFrom->pDispName );
	}
	if(pFrom->pUser== SIP_NULL)
	{
		pTo->pUser=SIP_NULL;
	}
	else
	{
		dLength = strlen(pFrom->pUser );

		pTo->pUser=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
				dLength+1,pErr);
		if ( pTo->pUser == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			SIPDEBUGFN("Exiting function sip_clonePresUrl");
			return SipFail;
		}
		strcpy( pTo->pUser, pFrom->pUser );
	}
	if(pFrom->pHost== SIP_NULL)
	{
		pTo->pHost=SIP_NULL;
	}
	else
	{
		dLength = strlen(pFrom->pHost );

		pTo->pHost=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
				dLength+1,pErr);
		if ( pTo->pHost == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			SIPDEBUGFN("Exiting function sip_clonePresUrl");
			return SipFail;
		}
		strcpy( pTo->pHost, pFrom->pHost);
	}
	if (__sip_cloneSipStringList(&(pTo->slRoute), \
		&(pFrom->slRoute), pErr) == SipFail)
	{
			SIPDEBUGFN("Exiting function sip_clonePresUrl");
			return SipFail;
	}

	if (__sip_cloneSipParamList (&(pTo->slParams), &(pFrom->slParams), \
		pErr) == SipFail)
	{
			SIPDEBUGFN("Exiting function sip_clonePresUrl");
			return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_clonePresUrl");
	return SipSuccess;
}


