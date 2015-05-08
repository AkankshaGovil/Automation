/******************************************************************************
** FUNCTION:
**
**
*******************************************************************************
**
** FILENAME:
** 	sipstring.c
**
** DESCRIPTION:
**
**
** DATE      NAME          REFERENCE      REASON
** ----      ----          ---------      ------
** 13Dec99   S.Luthra	   Creation
**	         B.Borthakur
**	         Preethy
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/

#include "ctype.h"
#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipbcptinit.h"
#ifdef SIP_CCP
#include "ccpinit.h"
#include "ccpfree.h"
#include "ccpinternal.h"
#endif
#ifdef SIP_IMPP
#include "imppinit.h"
#include "imppfree.h"
#include "imppinternal.h"
#endif
#include "rprinit.h"
#include "sipfree.h"
#include "sipbcptfree.h"
#include "rprfree.h"
#include "siplist.h"
#include "sipinternal.h"
#include "rprinternal.h"
#include "sipbcptinternal.h"
#include "sipvalidate.h"
#include "sdp.h"
#include "sipdecode.h"
#include "sipstring.h"
#include "sipdecodeintrnl.h"
#include "sipclone.h"
#include "sipformmessage.h"
#include "sipparserinc.h"
#ifdef SIP_DCS
#include "dcsstring.h"
#endif


/******************************************************************
**
** FUNCTION:  sip_makeUnknownHeader
**
** DESCRIPTION: This function parses a SIP Unknown pHeader represented
**		as a  string and fills up a SIP Unknown pHeader structure
**
******************************************************************/
SipBool sip_makeUnknownHeader
#ifdef ANSI_PROTO
	 ( SIP_S8bit * str,
	   SipUnknownHeader * hdr,
	   SipError *err)
#else
	( str, hdr, err )
	  SIP_S8bit * str;
	  SipUnknownHeader * hdr;
	  SipError *err;
#endif
{
	SIP_S32bit len, count;
	SIP_S8bit * temp_str, *temp_str1;

	SIPDEBUGFN ( "Entering makeUnknownHeader ");

	len = strlen( str ) ;
	if ( len == 0 )
	{
		*err = E_INV_HEADER;
		return SipFail;
	}
	temp_str = ( SIP_S8bit *) STRDUPACCESSOR(str);
	if ( temp_str == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}

	temp_str1 = temp_str;


	for ( count = 0; count < len; count++, temp_str1++)
		if ( *temp_str1 == ':' )
			break;

	if ( (count == 0) || ( count == len ) )
	{
		*err = E_INV_HEADER ;
		sip_freeString(temp_str);
		return SipFail;
	}

	if ( hdr->pName != SIP_NULL )
		sip_freeString( hdr->pName);

	if ( hdr->pBody != SIP_NULL )
		sip_freeString( hdr->pBody);

	hdr->pName = hdr->pBody = SIP_NULL;

	hdr->pName = (SIP_S8bit*)fast_memget(ACCESSOR_MEM_ID, count +2, err);
	if ( hdr->pName == SIP_NULL )
	{
		sip_freeString(temp_str);
		return SipFail;
	}
	strncpy ( hdr->pName, temp_str, count);
	(hdr->pName)[count] ='\0';
	temp_str1++;

	if ( count == len -1 )
		hdr->pBody = SIP_NULL;
	else
	{
		hdr->pBody = (SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID, len - count, err);
		if ( hdr->pBody == SIP_NULL )
		{
			sip_freeString(temp_str);
			return SipFail;
		}
		strcpy ( hdr->pBody, temp_str1);
	}
	sip_freeString(temp_str);
	SIPDEBUGFN ( "Exiting makeUnknownHeader");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  sip_getTypeFromString
**
** DESCRIPTION: This function retrieves the pHeader dType from a pHeader
**		represented as string
**
******************************************************************/
SipBool sip_getTypeFromString
#ifdef ANSI_PROTO
	 ( SIP_S8bit * str1,
	   en_HeaderType *dType,
	   SipError *err)
#else
	( str1, dType, err )
	  SIP_S8bit * str1;
	  en_HeaderType * dType;
	  SipError *err;
#endif
{
	SIP_S8bit * hdr_name, old;
	SIP_S8bit * temp_str, *str;
	SIP_U32bit dLength, i, j;
	SipBool		ret;
	SipError dErr;
        SIP_U32bit string_length;

#ifndef SIP_DCS
	dErr = E_NO_ERROR;
#endif

	dLength = strlen ( str1 );
	if ( dLength == 0 )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	for ( i = 0; i < dLength ; i++ )
		if ( str1[i] == ':' )
			break;

	if ( i >= dLength )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	str = (SIP_S8bit *) STRDUPACCESSOR( str1 );
	if ( str == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
	old = str[i+1];
	str[i+1] = '\0';

	temp_str = ( SIP_S8bit *) STRDUPACCESSOR(str);
	if ( temp_str == SIP_NULL )
	{
		*err = E_NO_MEM;
		return SipFail;
	}

	str[i+1] = old;

	hdr_name = ( SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID,  i + 2, err );
	if ( hdr_name == SIP_NULL)
		return SipFail;
        string_length = strlen(temp_str);
	for(i=0,j=0;i<string_length;i++)
		if((temp_str[i]==' ')||(temp_str[i]=='\t')||(temp_str[i]=='\n')\
			||(temp_str[i]=='\r'))
	{
	}
	else
	{
		hdr_name[j] = temp_str[i];
		j++;
	}
	hdr_name[j]='\0';

	*err = E_NO_ERROR;
	ret = SipFail;

	/* comparision starts here to find dType */
	if(strcasecmp("Accept:", hdr_name)==0)
	{
		*dType =SipHdrTypeAccept;
		ret = SipSuccess;
	}
	else if(strcasecmp("Accept-Encoding:",hdr_name)==0)
	{
		*dType =SipHdrTypeAcceptEncoding;
		ret = SipSuccess;
	}
	else if(strcasecmp("Accept-Language:", hdr_name)==0)
	{
			*dType = SipHdrTypeAcceptLanguage;
			ret = SipSuccess;
	}
	else if((strcasecmp("Call-id:",hdr_name)==0)||\
		(strcasecmp("i:",hdr_name)==0))
	{
		*dType =SipHdrTypeCallId;
		ret = SipSuccess;
	}
	else if(strcasecmp("CSeq:",hdr_name)==0)
	{
		*dType =SipHdrTypeCseq;
		ret = SipSuccess;
	}
	else if(strcasecmp("Content-Language:", hdr_name)==0)
	{
		*dType =SipHdrTypeContentLanguage;
		ret = SipSuccess;
	}
	else if(strcasecmp("Content-Disposition:", hdr_name)==0)
	{
		*dType =SipHdrTypeContentDisposition;
		ret = SipSuccess;
	}
	else if(strcasecmp("Reason:", hdr_name)==0)
	{
		*dType =SipHdrTypeReason;
		ret = SipSuccess;
	}
	else if(strcasecmp("Date:",hdr_name)==0)
	{
		*dType =SipHdrTypeDate;
		ret = SipSuccess;
	}
	else if(strcasecmp("Encryption:",hdr_name)==0)
	{
		*dType =SipHdrTypeEncryption;
		ret = SipSuccess;
	}
	else if(strcasecmp("Reply-To:",hdr_name)==0)
        {
                *dType =SipHdrTypeReplyTo;
                ret = SipSuccess;
        }
	else if(strcasecmp("Replaces:",hdr_name)==0)
        {
                *dType =SipHdrTypeReplaces;
                ret = SipSuccess;
        }
	else if(strcasecmp("Expires:",hdr_name)==0)
	{
		*dType =SipHdrTypeExpiresAny;
		ret = SipSuccess;
	}
#ifdef SIP_SESSIONTIMER
	else if ( strcasecmp("Min-SE:",hdr_name) == 0 )
	{
		*dType =SipHdrTypeMinSE;
		ret = SipSuccess;
	}
	else if((strcasecmp("Session-Expires:",hdr_name)==0) ||
	(strcasecmp("x:",hdr_name)==0))

	{
		*dType =SipHdrTypeSessionExpires;
		ret = SipSuccess;
	}
#endif

	else if((strcasecmp("From:",hdr_name)==0)||(strcasecmp("f:",hdr_name)==0))
	{
		*dType =SipHdrTypeFrom;
		ret = SipSuccess;
	}
	else if(strcasecmp("Record-Route:",hdr_name)==0)
	{
		*dType =SipHdrTypeRecordRoute;
		ret = SipSuccess;
	}
#ifdef SIP_3GPP
	else if(strcasecmp("Path:",hdr_name)==0)
	{
		*dType =SipHdrTypePath;
		ret = SipSuccess;
	}
	else if(strcasecmp("Service-Route:",hdr_name)==0)
	{
		*dType =SipHdrTypeServiceRoute;
		ret = SipSuccess;
	}
	else if(strcasecmp("P-Access-Network-Info:",hdr_name)==0)
	{
		*dType =SipHdrTypePanInfo;
		ret = SipSuccess;
	}
    else if(strcasecmp("P-Charging-Vector:",hdr_name)==0)
	{
		*dType =SipHdrTypePcVector;
		ret = SipSuccess;
	}

#endif
#ifdef SIP_PRIVACY
	else if(strcasecmp("P-Asserted-Identity:",hdr_name)==0)
	{
		*dType =SipHdrTypePAssertId ;
		ret = SipSuccess;
	}
	else if(strcasecmp("P-Preferred-Identity:",hdr_name)==0)
	{
		*dType =SipHdrTypePPreferredId ;
		ret = SipSuccess;
	}
	else if(strcasecmp("Privacy:",hdr_name)==0)
	{
		*dType = SipHdrTypePrivacy;
		ret = SipSuccess;
	}
#endif
	else if(strcasecmp("Timestamp:",hdr_name)==0)
	{
		*dType =SipHdrTypeTimestamp;
		ret = SipSuccess;
	}
	else if((strcasecmp("To:",hdr_name)==0)||(strcasecmp("t:",hdr_name)==0))
	{
		*dType =SipHdrTypeTo;
		ret = SipSuccess;
	}
	else if((strcasecmp("Via:",hdr_name)==0)||(strcasecmp("v:",hdr_name)==0))
	{
		*dType =SipHdrTypeVia;
		ret = SipSuccess;
	}
	else if((strcasecmp("Content-Encoding:",hdr_name)==0)||\
		(strcasecmp("e:",hdr_name)==0))
	{
		*dType =SipHdrTypeContentEncoding;
		ret = SipSuccess;
	}
	/* RPR */
	else if(strcasecmp("Supported:",hdr_name)==0||\
		strcasecmp("k:",hdr_name)==0)
	{
		*dType =SipHdrTypeSupported;
		ret = SipSuccess;
	}
 	else if((strcasecmp("Content-Length:",hdr_name)==0)||\
		(strcasecmp("l:",hdr_name)==0))
	{
		*dType =SipHdrTypeContentLength;
		ret = SipSuccess;
	}
	else if((strcasecmp("Content-Type:",hdr_name)==0)||\
		(strcasecmp("c:",hdr_name)==0))
	{
		*dType =SipHdrTypeContentType;
		ret = SipSuccess;
	}
	/* bcpt ext */
	else if(strcasecmp("Mime-Version:",hdr_name)==0)
	{
		*dType =SipHdrTypeMimeVersion;
		ret = SipSuccess;
	}
	/* bcpt ext ends */
	/* Retrans ext */
	else if(strcasecmp("RSeq:",hdr_name)==0)
	{
		*dType =SipHdrTypeRSeq;
		ret = SipSuccess;
	}
	else if(strcasecmp("RAck:",hdr_name)==0)
	{
		*dType =SipHdrTypeRAck;
		ret = SipSuccess;
	}
	else if(strcasecmp("Authorization:",hdr_name)==0)
	{
		*dType =SipHdrTypeAuthorization;
		ret = SipSuccess;
	}
	else if((strcasecmp("Contact:",hdr_name)==0)||(strcasecmp("m:",hdr_name)\
		==0))
	{
		*dType =SipHdrTypeContactAny;
		ret = SipSuccess;
	}
	else if(strcasecmp("Hide:",hdr_name)==0)
	{
		*dType = SipHdrTypeHide;
		ret = SipSuccess;
	}
	else if(strcasecmp("Max-Forwards:",hdr_name)==0)
	{
		*dType =SipHdrTypeMaxforwards;
		ret = SipSuccess;
	}
	else if(strcasecmp("Organization:",hdr_name)==0)
	{
		*dType = SipHdrTypeOrganization;
		ret = SipSuccess;
	}
	else if(strcasecmp("Priority:",hdr_name)==0)
	{
		*dType =SipHdrTypePriority;
		ret = SipSuccess;
	}
	else if(strcasecmp("Proxy-Authorization:",hdr_name)==0)
	{
		*dType =SipHdrTypeProxyauthorization;
		ret = SipSuccess;
	}
	else if(strcasecmp("Authentication-Info:",hdr_name)==0)
	{
		*dType =SipHdrTypeAuthenticationInfo;
		ret = SipSuccess;
	}
	else if(strcasecmp("Proxy-require:",hdr_name)==0)
	{
		*dType =SipHdrTypeProxyRequire;
		ret = SipSuccess;
	}
	else if(strcasecmp("Route:",hdr_name)==0)
	{
		*dType =SipHdrTypeRoute;
		ret = SipSuccess;
	}
	else if(strcasecmp("Also:",hdr_name)==0)
	{
		*dType =SipHdrTypeAlso;
		ret = SipSuccess;
	}
	else if(strcasecmp("Require:",hdr_name)==0)
	{
		*dType =SipHdrTypeRequire;
		ret = SipSuccess;
	}
	else if(strcasecmp("Response-Key:",hdr_name)==0)
	{
		*dType =SipHdrTypeResponseKey;
		ret = SipSuccess;
	}
	else if((strcasecmp("Refer-To:",hdr_name)==0)||\
		(strcasecmp("r:",hdr_name)==0))
	{
		*dType =SipHdrTypeReferTo;
		ret = SipSuccess;
	}
	else if((strcasecmp("Referred-By:",hdr_name)==0)||\
		(strcasecmp("b:",hdr_name)==0))
	{
		*dType =SipHdrTypeReferredBy;
		ret = SipSuccess;
	}
	else if((strcasecmp("Subject:",hdr_name)==0)||\
		(strcasecmp("s:",hdr_name)==0))
	{
		*dType =SipHdrTypeSubject;
		ret = SipSuccess;
	}
	else if(strcasecmp("User-Agent:",hdr_name)==0)
	{
		*dType =SipHdrTypeUserAgent;
		ret = SipSuccess;
	}
	else if(strcasecmp("Min-Expires:",hdr_name)==0)
	{
		*dType =SipHdrTypeMinExpires;
		ret = SipSuccess;
	}
	else if(strcasecmp("Allow:",hdr_name)==0)
	{
		*dType =SipHdrTypeAllow;
		ret = SipSuccess;
	}
	else if(strcasecmp("Proxy-Authenticate:",hdr_name)==0)
	{
		*dType =SipHdrTypeProxyAuthenticate;
		ret = SipSuccess;
	}
	else if(strcasecmp("Retry-After:",hdr_name)==0)
	{
		*dType =SipHdrTypeRetryAfterAny;
		ret = SipSuccess;
	}
	else if(strcasecmp("Server:",hdr_name)==0)
	{
		*dType =SipHdrTypeServer;
		ret = SipSuccess;
	}
	else if(strcasecmp("Unsupported:",hdr_name)==0)
	{
		*dType =SipHdrTypeUnsupported;
		ret = SipSuccess;
	}
	else if(strcasecmp("Warning:",hdr_name)==0)
	{
		*dType =SipHdrTypeWarning;
		ret = SipSuccess;
	}
	else if(strcasecmp("WWW-Authenticate:",hdr_name)==0)
	{
		*dType =SipHdrTypeWwwAuthenticate;
		ret = SipSuccess;
	}

	else if(strcasecmp("Alert-Info:",hdr_name)==0)
	{
		*dType =SipHdrTypeAlertInfo;
		ret = SipSuccess;
	}
	else if(strcasecmp("In-Reply-To:",hdr_name)==0)
	{
		*dType =SipHdrTypeInReplyTo;
		ret = SipSuccess;
	}
	else if(strcasecmp("Call-Info:",hdr_name)==0)
	{
		*dType =SipHdrTypeCallInfo;
		ret = SipSuccess;
	}
#ifdef SIP_IMPP
	else if(strcasecmp("Event:",hdr_name)==0||\
		strcasecmp("o:",hdr_name)==0)
	{
		*dType =SipHdrTypeEvent;
		ret = SipSuccess;
	}
	else if(strcasecmp("Allow-Events:",hdr_name)==0||\
		strcasecmp("u:",hdr_name)==0)
	{
		*dType =SipHdrTypeAllowEvents;
		ret = SipSuccess;
	}
	else if(strcasecmp("Subscription-State:",hdr_name)==0)
	{
		*dType =SipHdrTypeSubscriptionState;
		ret = SipSuccess;
	}
#endif
#ifdef SIP_CCP
	else if(strcasecmp("Accept-Contact:",hdr_name)==0 ||\
	        strcasecmp("a:",hdr_name)==0)
	{
		*dType =SipHdrTypeAcceptContact;
		ret = SipSuccess;
	}
	else if((strcasecmp("Reject-Contact:",hdr_name)==0)||\
		(strcasecmp("j:",hdr_name)==0))
	{
		*dType =SipHdrTypeRejectContact;
		ret = SipSuccess;
	}
	else if(strcasecmp("Request-Disposition:",hdr_name)==0||\
		strcasecmp("d:",hdr_name)==0)
	{
		*dType =SipHdrTypeRequestDisposition;
		ret = SipSuccess;
	}
#endif
#ifdef SIP_3GPP
	else if(strcasecmp("P-Associated-URI:",hdr_name)==0)
	{
		*dType =SipHdrTypePAssociatedUri;
		ret = SipSuccess;
	}
   	else if(strcasecmp("P-Called-Party-ID:",hdr_name)==0)
	{
		*dType =SipHdrTypePCalledPartyId;
		ret = SipSuccess;
	}
	else if(strcasecmp("P-Visited-Network-ID:",hdr_name)==0)
	{
		*dType =SipHdrTypePVisitedNetworkId;
		ret = SipSuccess;
	}
	else if(strcasecmp("P-Charging-Function-Addresses:",hdr_name)==0)
	{
		*dType =SipHdrTypePcfAddr;
		ret = SipSuccess;
	}

#endif
#ifdef SIP_DCS
	else if (sip_dcs_getDcsTypeFromString(hdr_name, dType, &dErr) == SipSuccess)
		ret = SipSuccess;
#endif
#ifdef SIP_SECURITY
	else if(strcasecmp("Security-Client:",hdr_name)==0)
        {
                *dType =SipHdrTypeSecurityClient;
                ret = SipSuccess;
        }
	else if(strcasecmp("Security-Server:",hdr_name)==0)
        {
                *dType =SipHdrTypeSecurityServer;
                ret = SipSuccess;
        }
	else if(strcasecmp("Security-Verify:",hdr_name)==0)
        {
                *dType =SipHdrTypeSecurityVerify;
                ret = SipSuccess;
        }
#endif
#ifdef SIP_CONF
	else if(strcasecmp("Join:",hdr_name)==0)
        {
                *dType =SipHdrTypeJoin;
                ret = SipSuccess;
        }
#endif
#ifdef SIP_CONGEST
	else if(strcasecmp("Resource-Priority:",hdr_name)==0)
        {
                *dType =SipHdrTypeRsrcPriority;
                ret = SipSuccess;
        }
   	else if(strcasecmp("Accept-Resource-Priority:",hdr_name)==0)
        {
                *dType =SipHdrTypeAcceptRsrcPriority;
                ret = SipSuccess;
        }

#endif
    

	if ( ret == SipFail)
	{
		*dType =  SipHdrTypeUnknown ;
		ret = SipSuccess;
	}

	sip_freeString( str);
	sip_freeString( temp_str);
	sip_freeString( hdr_name );
	return ret;

}


/********************************************************************
**
** FUNCTION:  sip_getReqLineAsString
**
** DESCRIPTION: This function returns the Request Line of the Request
**				message (taken as input) as a string.
**
*********************************************************************/

SipBool sip_getReqLineAsString
#ifdef ANSI_PROTO
(SipMessage *pS, SIP_S8bit **ppReqLine, SipError *pErr)
#else
	( pS,ppReqLine,pErr)
	SipMessage *pS;
	SIP_S8bit **ppReqLine;
	SipError *pErr;
#endif
{
	SIPDEBUG("Entering sip_getReqLineAsString");
	if (pErr == SIP_NULL)
	{
		return SipFail;
	}
	if (pS == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if(pS->dType != SipMessageRequest)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (ppReqLine ==SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	/* VERIFICATION FOR ppReqLine ? */

	*ppReqLine = ( SIP_S8bit * )\
						fast_memget(ACCESSOR_MEM_ID,SIP_MAX_HDR_SIZE,pErr);
	if (*ppReqLine == SIP_NULL )
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	if (pS->u.pRequest->pRequestLine==SIP_NULL)
	{
		fast_memfree(ACCESSOR_MEM_ID,*ppReqLine, pErr) ;
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	STRCPY ((char *)(*ppReqLine),\
							(char *)pS->u.pRequest->pRequestLine->pMethod );
	STRCAT ( (char *)(*ppReqLine)," ");
	sip_formAddrSpec (0,ppReqLine,\
							pS->u.pRequest->pRequestLine->pRequestUri, pErr);
	STRCAT ( (char *)(*ppReqLine)," ");
	STRCAT ( (char *)(*ppReqLine), pS->u.pRequest->pRequestLine->pVersion);

	*pErr = E_NO_ERROR;
	SIPDEBUG("Exiting sip_getReqLineAsString");
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sip_setReqLineFromString
**
** DESCRIPTION: This function sets the Request Line in a SipMessage
**				 of dType SipMessageRequest.
**
*********************************************************************/
SipBool sip_setReqLineFromString
#ifdef ANSI_PROTO
	(SipMessage *pSipMsg, SIP_S8bit *line, SipError *pErr)
#else
	(pSipMsg, line, pErr)
	SipMessage *pSipMsg;
	SIP_S8bit *line;
	SipError *pErr;
#endif
{
	SipBool x,y;

	x = y = SipSuccess;
	SIPDEBUG("Entering sip_setReqLineFromString");

	if (pErr == SIP_NULL)
		return SipFail;

	if ((pSipMsg == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if(line == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if((pSipMsg->u).pRequest ==SIP_NULL)
	{
		if(sip_initSipReqMessage(&(pSipMsg->u).pRequest,pErr) ==SipFail)
		{
			*pErr = E_NO_MEM;
			return SipFail;

		}
		x = SipSuccess;

	}

	if((pSipMsg->u).pRequest->pRequestLine ==SIP_NULL)
	{
		if(sip_initSipReqLine(&(pSipMsg->u).pRequest->pRequestLine,pErr) ==SipFail)
		{
			if(x == SipSuccess)
				sip_freeSipReqMessage((pSipMsg->u).pRequest);

			*pErr = E_NO_MEM;
			return SipFail;

		}
		y = SipSuccess;
	}


	if( sip_makeReqLine((pSipMsg->u).pRequest->pRequestLine,line,pErr) ==SipFail)
	{
		if(x == SipSuccess)
			sip_freeSipReqMessage((pSipMsg->u).pRequest);

		if(y == SipSuccess)
			sip_freeSipReqLine((pSipMsg->u).pRequest->pRequestLine);

		return SipFail;


	}



	SIPDEBUG("Exiting sip_setReqLineFromString");
	*pErr = E_NO_ERROR;

	return SipSuccess;

}


/*********************************************************
**
**FUNCTION:sip_getStatusLineAsString
**
** DESCRIPTION: This function copies the contents of a SIP Message
**		structure into a char buffer
**
**********************************************************/
SipBool sip_getStatusLineAsString
#ifdef ANSI_PROTO
	(
	SipMessage *pSipMsg,
	SIP_S8bit **outStatusLine,
	SipError *err)
#else
	( pSipMsg, outStatusLine, err)
	SipMessage *pSipMsg;
	SIP_S8bit **outStatusLine;
	SipError *err;
#endif
{
	SIP_S8bit dCodeNum[SIP_RESP_CODE_LEN];
	SIPDEBUG("Entering function sip_getStatusLineAsString");

	if (err==SIP_NULL)
		return SipFail;

	if ( (pSipMsg == SIP_NULL)||(outStatusLine == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (pSipMsg->dType != SipMessageResponse)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}

	if (pSipMsg->u.pResponse->pStatusLine==SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	*outStatusLine = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,SIP_MAX_HDR_SIZE,err);

	if((char *)pSipMsg->u.pResponse->pStatusLine->pVersion!= SIP_NULL)
		strcpy ((char *)*outStatusLine, (char *)pSipMsg->u.pResponse->pStatusLine->pVersion );

	strcat ( (char *)*outStatusLine," ");

	HSS_SNPRINTF((char *)dCodeNum, SIP_RESP_CODE_LEN, "%03d",pSipMsg->u.pResponse->\
		pStatusLine->dCodeNum); /* dCodeNum is not accessed */
	dCodeNum[SIP_RESP_CODE_LEN-1]='\0';


	if((char *)dCodeNum!=SIP_NULL)
		strcat ( (char *)*outStatusLine,dCodeNum);

	strcat ( (char *)*outStatusLine," ");

	if(pSipMsg->u.pResponse->pStatusLine->pReason != SIP_NULL)
		strcat ( (char *)*outStatusLine,pSipMsg->u.pResponse->pStatusLine->pReason);

	strcat ( (char *)*outStatusLine,"\r\n");

	*err = E_NO_ERROR;

	SIPDEBUG("Exiting function sip_getStatusLineAsString");

	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_setStatusLineFromString
**
** DESCRIPTION: This function sets the string parameter taken as input
**				as the status line of the SipMessage.
**
*********************************************************************/
SipBool sip_setStatusLineFromString
#ifdef ANSI_PROTO
(SipMessage *pS, SIP_S8bit *pStr, SipError *pErr)
#else
	( pS,pStr,pErr)
	SipMessage *pS;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SipBool x,y;

	x=y=SipFail;

	SIPDEBUG("Entering sip_setStatusLineAsString");
	if (pErr == SIP_NULL)
	{
		return SipFail;
	}
	if (pS == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if(pS->dType != SipMessageResponse)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pStr==SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	/* VALIDATE STATUS LINE ? */
	if (pS->u.pResponse == SIP_NULL)
	{
		if ((sip_initSipRespMessage(&(pS->u.pResponse), pErr))==SipFail)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		else x = SipSuccess;
	}
	if (pS->u.pResponse->pStatusLine == SIP_NULL)
	{
		if ((sip_initSipStatusLine(&(pS->u.pResponse->pStatusLine), pErr))==SipFail)
		{
			if (x==SipSuccess)
				sip_freeSipRespMessage(pS->u.pResponse);
			*pErr = E_NO_MEM;
			return SipFail;
		}
		else y = SipSuccess;
	}

	/* call makeStatusLine */

	if ((sip_makeStatusLine(pS->u.pResponse->pStatusLine, pStr, pErr))==SipFail)
	{
		if (x==SipSuccess)
			sip_freeSipRespMessage(pS->u.pResponse);
		if (y==SipSuccess)
			sip_freeSipStatusLine(((pS->u).pResponse)->pStatusLine);

		return SipFail;
	}

	SIPDEBUG ( "Exiting sip_setStatusLineAsString");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  sip_makeToHeader
**
** DESCRIPTION: This function parses a SIP To pHeader represented as a
**		string and fills up a SIP To pHeader structure
**
******************************************************************/
SipBool sip_makeToHeader
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
( SIP_S8bit * str,
	   SipToHeader ** hdr,
	   SipError *err)
#else
	 ( SIP_S8bit * str,
	   SipToHeader * hdr,
	   SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( str, hdr, err )
	  SIP_S8bit * str;
	  SipToHeader ** hdr;
	  SipError *err;

#else
	( str, hdr, err )
	  SIP_S8bit * str;
	  SipToHeader * hdr;
	  SipError *err;
#endif
#endif
{
	SIP_U32bit parserRetVal;
	SIP_S32bit len;
	SipError temp_err;
	SIP_S8bit *pParserBuffer;
	SIP_S8bit *pParserInput;
	SipHeaderParserParam dHeaderParserParam;

	SIPDEBUGFN ( "Entering makeToHeader ");

	len = strlen(str);
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, err);
	if(pParserBuffer == SIP_NULL)
		return SipFail;
	strcpy(pParserBuffer, str);
	pParserBuffer[len+1]='\0';
	dHeaderParserParam.pError=(SipError*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipError),err);

	*(dHeaderParserParam.pError) = E_NO_ERROR;

	dHeaderParserParam.pSipMessage = ( SipMessage * )\
		fast_memget(ACCESSOR_MEM_ID, sizeof (SipMessage) , err);
	if ( dHeaderParserParam.pSipMessage == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}

	dHeaderParserParam.pSipMessage->dType = SipMessageRequest;
	if ( sip_listInit( &(dHeaderParserParam.pSipMessage->slOrderInfo),\
		__sip_freeSipHeaderOrderInfo, err) == SipFail)
		return SipFail;

	dHeaderParserParam.pSipMessage->pGeneralHdr = ( SipGeneralHeader * )\
		fast_memget(ACCESSOR_MEM_ID, sizeof ( SipGeneralHeader) , err);
	if ( dHeaderParserParam.pSipMessage->pGeneralHdr == SIP_NULL)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo), err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, err);
		*err = E_NO_MEM;
		return SipFail;
	}

	dHeaderParserParam.pSipMessage->pGeneralHdr->pToHdr = SIP_NULL;

	dHeaderParserParam.pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),err);
    if(dHeaderParserParam.pGCList==SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
	if(sip_listInit(dHeaderParserParam.pGCList, sip_freeVoid, err)==SipFail)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo),\
			err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->\
			pGeneralHdr, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, \
			&temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		*err = E_NO_MEM;
		return SipFail;
	}

	/*glbSipParserAddrSpec = SIP_NULL;*/
	if (sip_lex_To_scan_buffer(pParserBuffer, len+2) != 0)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo),\
			err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->\
			pGeneralHdr, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, \
			&temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		sip_listDeleteAll(dHeaderParserParam.pGCList, err);
		*err = E_NO_MEM;
		return SipFail;
	}
	sip_lex_To_reset_state();
	pParserInput = pParserBuffer;
	parserRetVal= glbSipParserToparse((void *)&dHeaderParserParam);
	if(parserRetVal != 0)
	{
		if(*(dHeaderParserParam.pError) ==E_NO_ERROR)
			*(dHeaderParserParam.pError)=E_PARSER_ERROR;
		/*PRINTMOREDETAILS(pParserInput);*/
	}
	sip_lex_To_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);

	sip_listDeleteAll(dHeaderParserParam.pGCList, err);
	fast_memfree(ACCESSOR_MEM_ID,dHeaderParserParam.pGCList,SIP_NULL);
	if ( *(dHeaderParserParam.pError) != E_NO_ERROR)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo), err);
		if (dHeaderParserParam.pSipMessage->pGeneralHdr->pToHdr != SIP_NULL)
			sip_freeSipToHeader( dHeaderParserParam.pSipMessage->\
								pGeneralHdr->pToHdr);
		/*if (glbSipParserAddrSpec != SIP_NULL)
			sip_freeSipAddrSpec(glbSipParserAddrSpec);*/
		/*glbSipParserAddrSpec = SIP_NULL;*/
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->pGeneralHdr, err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,err);

		*err = *(dHeaderParserParam.pError);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, err);
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*hdr = dHeaderParserParam.pSipMessage->pGeneralHdr->pToHdr;
#else
	if ( __sip_cloneSipToHeader(hdr, dHeaderParserParam.pSipMessage->\
		pGeneralHdr->pToHdr, err) == SipFail)
	{
		SipError dTempErr;
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo), err);
		sip_freeSipToHeader( dHeaderParserParam.pSipMessage->pGeneralHdr->\
			pToHdr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->pGeneralHdr,\
			&dTempErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, &dTempErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &dTempErr);
		return SipFail;
	}
#endif
	sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo), err);
#ifndef SIP_BY_REFERENCE
	sip_freeSipToHeader( dHeaderParserParam.pSipMessage->pGeneralHdr->\
		pToHdr);
#endif
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->pGeneralHdr, err);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,err);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, err);

	SIPDEBUGFN ( "Exiting makeToHeader");
	*err = E_NO_ERROR;
	return SipSuccess;
}


#ifdef SIP_BY_REFERENCE
/********************************************************************
**
** FUNCTION:  sip_setAddrSpecFromString
**
** DESCRIPTION: This function sets the string parameter taken as input
**				as a AddrSpec structure.
**
** NOTE: This function in turn calls sip_makeToHeader to parse the
**		 sipurl. Once that is done; the AddrSpec structure is cloned
**		 into the input AddrSpec pointer
*********************************************************************/
SipBool sip_setAddrSpecFromString
#ifdef ANSI_PROTO
(SipAddrSpec **ppAddrSpec, SIP_S8bit *pStr, SipError *pErr)
#else
	( ppAddrSpec,pStr,pErr)
	SipAddrSpec **ppAddrSpec;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIP_U32bit dLength;
	SipToHeader *pToHdr=SIP_NULL;

	SIPDEBUG("Entering sip_setAddrSpecFromString");
	if (pErr == SIP_NULL)
	{
		return SipFail;
	}
	if (ppAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if (pStr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	dLength = strlen((char *)pStr);
	pTemp = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, (sizeof(SIP_S8bit))*(dLength+6), pErr);
	if (pTemp == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	strcpy(pTemp, "To:<");
	strcat(pTemp, pStr);
	strcat(pTemp,">");


	if (sip_makeToHeader(pTemp, &pToHdr, pErr) == SipFail)
	{
		sip_freeSipToHeader (pToHdr);
		fast_memfree(ACCESSOR_MEM_ID, pTemp, SIP_NULL);
		return SipFail;
	}


	HSS_LOCKEDINCREF((pToHdr->pAddrSpec)->dRefCount);
	*ppAddrSpec = pToHdr->pAddrSpec;


	sip_freeSipToHeader(pToHdr);
	fast_memfree(ACCESSOR_MEM_ID, pTemp, SIP_NULL);

	SIPDEBUG("Exiting sip_setAddrSpecFromString");
	return SipSuccess;
}
#endif

/***********************************************************************
**
** Function: sip_formQuotedString
** Description: This API converts a string into a quoted string. 
**              Please note that it returns a new memory area allocated. 
**              It does not modify the original string.
**
** Parameters:
** pOrgString (IN)-  String to be converted
** pErr (OUT)- possible error value (see api ref. doc)
**************************************************************************/

SIP_U8bit * sip_formQuotedString(SIP_U8bit* pOriginalString, SipError *pError)
{
    SIP_U32bit size = 0;
    SIP_U8bit * pTempString;

    SIPDEBUGFN(("Entering sip_formQuotedString "));
    size = strlen((char *)pOriginalString);

    /* the three extra bytes are for the start and ending quotes and the
       terminating NUL */
    pTempString = (SIP_U8bit *)fast_memget(ACCESSOR_MEM_ID, 
                                           (size + SIP_QUOTEQUOTENUL_SIZE), 
                                           pError);
    if( pTempString == SIP_NULL)
    {
        return SIP_NULL;
    }

    /* add the initial quote - zero added for readability
       Any optimizing compiler worth its salt will not add code 
       for it */
    *(pTempString+0) = '\"';
    /* terminate the string for use with strcat */
    *(pTempString+1) = '\0';

    /* add the original string */
    strcat((char *)pTempString,(char *)pOriginalString);
    /* add the end quote - string is terminated by strcat */
    strcat((char *)pTempString,"\"");

    /* the memory has been allocated on the heap so no problems 
       in returning */
    SIPDEBUGFN(("Exiting sip_formQuotedString "));
    return (pTempString);
}

/******************************************************************************
**
** Function: sip_convertIntoQuotedString
** Description: This API converts a string into a quoted string. It checks the 
**              characters in the String :
**              If char belongs to TOKEN-SET then go to next character in String 
**              Else  verify 
**                  If the char belongs to the QUOTED-SET and if yes 
**                       then form a quoted string
**              Else
**                        return Error
**              Please note that it returns a new memory area allocated. 
**              It does not modify the original string.
**
** Parameters:
** pOrgString (IN)-  String to be converted
** pErr (OUT)- possible error value (see api ref. doc)
**
** Return Values:
** SIP_U8bit *  - If successful returns if character from Quoted-Set occured in
**                String else it returns the original string.
** SIP_NULL - If failed with Error Variable Set.
**
 ********************************************************************************/
SIP_U8bit*  sip_convertIntoQuotedString(SIP_U8bit* pOrgStr, SipError *pErr)
{
    SIP_U32bit  dIter=0,dLen = strlen((const char*)pOrgStr) ;
    SIP_U8bit * pResultStr = pOrgStr ;
		SipBool dLoop = SipSuccess ;
    *pErr = E_NO_ERROR ;

    SIPDEBUGFN(("Entering sip_convertIntoQuotedString "));
		
		if (pErr == SIP_NULL)
		{
				return SIP_NULL ;
		}
    while ( (dIter < dLen) && (dLoop == SipSuccess) )
    {
				switch (pOrgStr[dIter])
				{
           case SIP_DEC_VAL_ASCII_asterik      :
           case SIP_DEC_VAL_ASCII_plus         :
           case SIP_DEC_VAL_ASCII_hyphen       :
					 case SIP_DEC_VAL_ASCII_period       : 
					 case SIP_DEC_VAL_ASCII_percent      :
					 case SIP_DEC_VAL_ASCII_exclamation  :
					 case SIP_DEC_VAL_ASCII_closingquote : 
					 case SIP_DEC_VAL_ASCII_openingquote :
					 case SIP_DEC_VAL_ASCII_underscore   :
					 case SIP_DEC_VAL_ASCII_tilde        : 
					    {
                                    dIter ++ ;
																		break ;
						  }	
					 default                             :	
					    {
							                    if ( ((pOrgStr[dIter] >= SIP_DEC_VAL_ASCII_lowercase_a  ) &&
                                         (pOrgStr[dIter] <= SIP_DEC_VAL_ASCII_lowercase_z  )) ||
                                         (( pOrgStr[dIter] >= SIP_DEC_VAL_ASCII_uppercase_a) && 
                                          ( pOrgStr[dIter] <= SIP_DEC_VAL_ASCII_uppercase_z) )  ||
                                          ((pOrgStr[dIter] >= SIP_DEC_VAL_ASCII_zero) && 
                                          (pOrgStr[dIter] <= SIP_DEC_VAL_ASCII_nine) ) )
                                  {
                                          dIter ++ ;
                                  }
                                  else if ( ((pOrgStr[dIter] >= SIP_DEC_VAL_ASCII_numbersign) &&
                                         (pOrgStr[dIter] <= SIP_DEC_VAL_ASCII_openingsquarebracket)) ||
                                       (  (pOrgStr[dIter] >= SIP_DEC_VAL_ASCII_closingsquarebracket) &&
                                          (pOrgStr[dIter] <= SIP_DEC_VAL_ASCII_tilde)) )
                                 {
                                     SIPDEBUG("Going to form quoted string \n") ;
                                     pResultStr = sip_formQuotedString(pOrgStr,pErr) ;
																	   dLoop = SipFail ;
                                 }
								                 else if ( ((pOrgStr[dIter] >= SIP_DEC_VAL_ASCII_unicode_192) &&
												               (pOrgStr[dIter] <= SIP_DEC_VAL_ASCII_unicode_248)) ||
												              ((pOrgStr[dIter] >= SIP_DEC_VAL_ASCII_unicode_251) &&
												              (pOrgStr[dIter] <= SIP_DEC_VAL_ASCII_unicode_253)) )
								                {
                                    SIPDEBUG("Going to form quoted string \n") ;
                                    pResultStr = sip_formQuotedString(pOrgStr,pErr) ;
																	  dLoop = SipFail ;
								                 }
                                else
                                {
                                    pResultStr=NULL ;
                                    *pErr = E_FORMING_QUOTING_STRING ;
																	  dLoop = SipFail ;
                                }
																break ;
					 }/* default */
				}/* switch*/
     } /* while loop */
     SIPDEBUGFN(("Exiting sip_convertIntoQuotedString "));
     return pResultStr ;
}

