/***********************************************************************
 ** FUNCTION:
 **             Forms a Text SIP Message from SIP Structure
 *********************************************************************
 **
 ** FILENAME:
 ** sipformmessage.c
 **
 ** DESCRIPTION:
 ** This file contains code to convert from structures to SIP Text
 ** Entry function is : sip_formMessage
 **
 ** DATE        NAME          REFERENCE               REASON
 ** ----        ----          ---------              --------
 ** 6/12/99    Arjun RC      	            	   Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


#include "sipstruct.h"
#ifdef SIP_CCP
#include "ccpstruct.h"
#endif

#include "sipstatistics.h"
#include "portlayer.h"
#include "sipformmessage.h"
#include "sipinit.h"
#include "sipdecode.h"

#ifdef SIP_DCS
#include "dcsformmessage.h"
#endif

#ifdef STRCAT
#undef STRCAT
#endif

#ifndef SIP_MSGBUFFER_CHECK
#define STRCAT(e,a,b) __sip_check(&a,(char*)b)
#else
#define STRCAT(e,a,b) \
do \
{ \
	if(__sip_check(e,&a,(char*)b) == SipFail)\
	{\
		*err = E_BUF_OVERFLOW;\
		return SipFail;\
	}\
}\
while(0)
#endif


#define CRLF "\r\n"


/******************************************************************
**
** FUNCTION:  sip_verifyTypeAny
**
** DESCRIPTION: Internal functionto verify if the Header has an ANY
**		Type
**
******************************************************************/
SipBool __sip_verifyTypeAny
#ifdef ANSI_PROTO
        (
          en_HeaderType         dType,
          SipError              *err )
#else
        ( dType, err )
          en_HeaderType         dType;
          SipError              *err;
#endif
{
	switch(dType)
	{
		case SipHdrTypeExpiresDate:
		case SipHdrTypeExpiresSec:
		case SipHdrTypeContactNormal:
		case SipHdrTypeContactWildCard:
		case SipHdrTypeRetryAfterDate:
		case SipHdrTypeRetryAfterSec:
				*err = E_INV_TYPE;
				return SipFail;
		default :
				*err = E_NO_ERROR;
				return SipSuccess;
	}

}


/******************************************************************
**
** FUNCTION:  __sip_setHeaderCountInHeaderLine
**
** DESCRIPTION: Internal function to set the Header count in a
**		particular Header line of a SIP message
**
******************************************************************/
SipBool __sip_setHeaderCountInHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage            *msg,
          SIP_U32bit            line,
          SIP_U32bit	        count,
          SipError              *err )
#else
        ( msg, line, count, err )
          SipMessage            *msg;
          SIP_U32bit            line;
          SIP_U32bit	        count;
          SipError              *err;
#endif
{
	SIP_Pvoid       temp;

	SIPDEBUGFN("Entering function sip_setHeaderCountAtHeaderLine");
	if (count>0)
	{
		if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
			return SipFail;

		((SipHeaderOrderInfo *)temp)->dNum = count;
		*err = E_NO_ERROR;
		SIPDEBUGFN("Exitting function sip_setHeaderCountAtHeaderLine");
		return SipSuccess;
	}
	else
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
}


/********************************************************************
**
** FUNCTION:  __sip_getHeaderPositionFromIndex
**
** DESCRIPTION: This function retrieves the absolute line and the
**  position in that line of an en_HeaderType "Type" given the
**  index among all the headers of that type.
**
*********************************************************************/
SipBool __sip_getHeaderPositionFromIndex
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SIP_U32bit list_index,\
	 SIP_U32bit *abs_line, SIP_U32bit *position, SipError *err)
#else
	(msg, dType, list_index, abs_line, position, err)
	SipMessage *msg;
	en_HeaderType dType;
	SIP_U32bit list_index;
	SIP_U32bit *abs_line;
	SIP_U32bit *position;
	SipError *err;
#endif
{
	SIP_Pvoid header_line;
	SipHeaderOrderInfo *hdr_info;
	SIP_U32bit temp_count, count, prev_count, iter;
	SipBool result = SipFail;
	SIPDEBUGFN("Entering sip_getHeaderPositionFromIndex");
	count=prev_count=0;

	/* checking for the validity of input parameters. */
	if (err == SIP_NULL)
		return SipFail;
	if ( (msg==SIP_NULL)||((abs_line==SIP_NULL)&&(position==SIP_NULL)) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if (__sip_verifyTypeAny(dType, err) == SipFail)
		return SipFail;

	if (sip_listSizeOf(&(msg->slOrderInfo), &temp_count, err) == SipFail)
		return SipFail;
	for (iter=0; iter<temp_count; iter++)
	{
		if (sip_listGetAt(&(msg->slOrderInfo), iter, &header_line, err)\
			== SipFail)
			return SipFail;
		hdr_info = (SipHeaderOrderInfo *)header_line;
		if (hdr_info->dType == dType)
			count+=hdr_info->dNum;
		if (count>list_index)
		{
			if (abs_line != SIP_NULL)
				*abs_line = iter;
			if (position != SIP_NULL)
				*position = list_index-prev_count;
			result = SipSuccess;
			break;
		}
		prev_count=count;
	}
	if (result == SipFail)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting sip_getHeaderPositionFromIndex");
	return SipSuccess;
}



/******************************************************************
**
** FUNCTION:  __sip_getHeaderCountFromHeaderLine
**
** DESCRIPTION: Internal function to retrieve the number of headers
**		in a particular header line in a SIP Message
**
******************************************************************/
SipBool __sip_getHeaderCountFromHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage            *msg,
          SIP_U32bit            line,
          SIP_U32bit		*count,
	  SipError              *err )
#else
        ( msg, line, count, err )
          SipMessage            *msg;
          SIP_U32bit            line;
          SIP_U32bit            *count;
          SipError              *err;
#endif
{
	SIP_Pvoid       temp;

	SIPDEBUGFN("Entering function sip_getHeaderCountFromHeaderLine");


	if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
		return SipFail;

	*count = ((SipHeaderOrderInfo *)temp)->dNum;
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getHeaderCountFromHeaderLine");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  __sip_getHeaderFormFromHeaderLine
**
** DESCRIPTION: This function gets the form of the header (short/full)
**		at a particular header line
**
******************************************************************/
SipBool __sip_getHeaderFormFromHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage            *msg,
          SIP_U32bit            line,
          en_HeaderForm		*dTextType,
          SipError              *err )
#else
        ( msg, line, dTextType, err )
          SipMessage            *msg;
          SIP_U32bit            line;
          en_HeaderForm		*dTextType;
          SipError              *err;
#endif
{
	SIP_Pvoid       temp;

	SIPDEBUGFN("Entering function sip_getHeaderFormFromHeaderLine");
	if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
		return SipFail;

	*dTextType = ((SipHeaderOrderInfo *)temp)->dTextType;
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getHeaderFormFromHeaderLine");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  __sip_setHeaderFormAtHeaderLine
**
** DESCRIPTION: Internal function to set the pHeader form at a
**		particular pHeader line in a SIP message
**
******************************************************************/
SipBool __sip_setHeaderFormAtHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage            *msg,
          SIP_U32bit            line,
          en_HeaderForm         dTextType,
          SipError              *err )
#else
        ( msg, line, dTextType, err )
          SipMessage            *msg;
          SIP_U32bit            line;
          en_HeaderForm         dTextType;
          SipError              *err;
#endif
{
	SIP_Pvoid       temp;

	SIPDEBUGFN("Entering function sip_setHeaderFormAtHeaderLine");
	if (dTextType == SipFormFull || dTextType == SipFormShort)
	{
		if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
			return SipFail;

		((SipHeaderOrderInfo *)temp)->dTextType = dTextType;
		*err = E_NO_ERROR;
		SIPDEBUGFN("Exitting function sip_setHeaderFormAtHeaderLine");
		return SipSuccess;
	}
	else
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
}



/******************************************************************
**
** FUNCTION:  __sip_getHeaderTypeAtHeaderLine
**
** DESCRIPTION: Internal function to retrieves the Header Type at a
**		given absolute line in message
**
******************************************************************/
SipBool __sip_getHeaderTypeAtHeaderLine
#ifdef ANSI_PROTO
        ( SipMessage		*msg,
	  SIP_U32bit		line,
	  en_HeaderType		*dType,
	  SipError              *err )
#else
        ( msg, line, dType, err )
	  SipMessage            *msg;
	  SIP_U32bit            line;
	  en_HeaderType         *dType;
	  SipError              *err;
#endif
{
	SIP_Pvoid	temp;

	SIPDEBUGFN("Entering function sip_getHeaderTypeAtHeaderLine");
	if (sip_listGetAt (&(msg->slOrderInfo), line, &temp, err) == SipFail)
		return SipFail;
	*dType = ((SipHeaderOrderInfo *)temp)->dType;
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getHeaderTypeAtHeaderLine");
	return SipSuccess;
}
/*****************************************************************
** FUNCTION: sip_areThereParamsinAddrSpec
**
**
** DESCRIPTION: Checks for presence of params,headers within a AddrSpec
*****************************************************************/
SipBool sip_areThereParamsinAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec 	*aspec,
	SipError	*err)
#else
	(aspec,err)
	SipAddrSpec *aspec;
	SipError *err;
#endif
{

		SIP_U32bit listSize;
	    SipList		*list;


		switch(aspec->dType)
		{
			case SipAddrSipUri:
			case SipAddrSipSUri:
			            /*Check for presence of params or
							any header within the SipUrl*/
						list=&(aspec->u.pSipUrl->slParam);
						sip_listSizeOf(list, &listSize, err);
						if ((listSize>0)||\
							(aspec->u.pSipUrl->pHeader!=SIP_NULL))
							return SipSuccess;
						break;
			default:
			           /*Check for presence of a ; , ? within the uri*/
					   if (strstr(aspec->u.pUri,";")||\
						   strstr(aspec->u.pUri,",")||\
						   strstr(aspec->u.pUri,"?"))
						   	return SipSuccess;
		}

		return SipFail;
}

/*****************************************************************
** FUNCTION: sip_formSipParamList
**
**
** DESCRIPTION: Converts a list of SipParams to text
*****************************************************************/

SipBool sip_formSipParamList
#ifdef ANSI_PROTO
	(
	SIP_S8bit	*pEndBuff,
	SIP_S8bit 	**ppOut,
	SipList 	*list,
	SIP_S8bit 	*separator,
	SIP_U8bit	leadingsep,
	SipError 	*err)
#else
	(pEndBuff,ppOut, list, separator, leadingsep, err)
	SIP_S8bit	*pEndBuff;
	SIP_S8bit **ppOut;
	SipList *list;
	SIP_S8bit *separator;
	SIP_U8bit leadingsep;
	SipError *err;
#endif
{
	SIP_U32bit listSize,listIter;
	SIP_S8bit* out;
	out = *ppOut;


	SIPDEBUGFN("Entering into sip_formSipParamList");

	sip_listSizeOf( list, &listSize, err);
	/* put space only if params present */
	if(( listSize!=0)&&(leadingsep==0) )
	{
		STRCAT ( pEndBuff,out," ");
	}
	listIter = 0;
	while (listIter < listSize)
	{
		SipParam *pParam;
		SIP_U32bit valueSize;

		sip_listGetAt (list, listIter, (SIP_Pvoid *) &pParam, err);
		if((listIter!=0)||(leadingsep!=0))
		{
			if ((listIter!=0)||(leadingsep!=2))
				STRCAT ( pEndBuff,out, separator);
		}
		STRCAT(pEndBuff,out,pParam->pName);
		sip_listSizeOf( &(pParam->slValue), &valueSize, err);
		if ( valueSize>=1)
		{
			SIP_S8bit *value;
			SIP_U32bit valueIter=0;
			STRCAT(pEndBuff,out,"=");
			while(valueIter < valueSize)
			{
				if(valueIter>0)
					STRCAT(pEndBuff,out,",");
				sip_listGetAt (&(pParam->slValue), valueIter, \
					(SIP_Pvoid *) &value, err);
				STRCAT(pEndBuff,out,value);
				valueIter++;
			}
		}
		listIter++;
	} /* while */
	if(pEndBuff)*ppOut = out;
	SIPDEBUGFN("Exiting from sip_formSipParamList");
	return SipSuccess;
}



/*****************************************************************
** FUNCTION: sip_formAddrSpec
**
**
** DESCRIPTION: Converts a SipAddrSpec to Text
*****************************************************************/

SipBool sip_formAddrSpec
#ifdef ANSI_PROTO
	(
	SIP_S8bit	*pEndBuff,
	SIP_S8bit 	**ppOut,
	SipAddrSpec 	*aspec,
	SipError 	*err)
#else
	(pEndBuff,ppOut, aspec, err)
	SIP_S8bit	*pEndBuff;
	SIP_S8bit **ppOut;
	SipAddrSpec *aspec;
	SipError *err;
#endif
{
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into sip_formAddrSpec");

	out  = *ppOut;
	if ((aspec->dType == SipAddrSipUri) || (aspec->dType == SipAddrSipSUri))
	/* Address is a SIP Uri, so more dissection */
	{
		SipUrl *su;
		su = aspec->u.pSipUrl;
		if(aspec->dType == SipAddrSipUri)
			STRCAT ( pEndBuff,out,"sip:");
		else
			STRCAT (pEndBuff,out,"sips:");
		if (su->pUser)
			STRCAT ( pEndBuff,out, su->pUser);
		if (su->pPassword !=SIP_NULL)
		{
			STRCAT ( pEndBuff,out, ":");
			STRCAT ( pEndBuff,out, su->pPassword);
		}
		/* if pUser was there, add an @ after pUser:[passwd] */
		if (su->pUser !=SIP_NULL) STRCAT ( pEndBuff,out,"@");
		if (su->pHost !=SIP_NULL ) STRCAT ( pEndBuff,out, su->pHost);
		/* see if its pHost or pHost:dPort */
		if (su->dPort != SIP_NULL)
		{
			SIP_S8bit porttext[SIP_MAX_PORT_SIZE]; 
			STRCAT ( pEndBuff,out, ":");
			HSS_SNPRINTF((char *)porttext, SIP_MAX_PORT_SIZE, "%u", *(su->dPort) );
			porttext[SIP_MAX_PORT_SIZE-1]='\0';
			STRCAT ( pEndBuff,out, porttext);
		}

		if (SipFail==sip_formSipParamList\
				(pEndBuff,&out, &(su->slParam), (SIP_S8bit *) ";",1, err))
		{
			return SipFail;
		}

		/* Now parse Header */
		if (su->pHeader !=SIP_NULL)
		{
			STRCAT ( pEndBuff,out, "?");
			STRCAT ( pEndBuff,out, su->pHeader);
		}
	} /* End of if (ch->pAddrSpec->dType == SipAddrSipUri) */
	else
	/* Address is  a pUri */
		STRCAT ( pEndBuff,out, aspec->u.pUri);
	if(pEndBuff)*ppOut = out;
	SIPDEBUGFN("Exiting from  sip_formAddrSpec");
	return SipSuccess;
}

/*****************************************************************
** FUNCTION: sip_formDateStruct
**
**
** DESCRIPTION: Converts a SipDateStruct to Text
*****************************************************************/


SipBool sip_formDateStruct
#ifdef ANSI_PROTO
	(
	SIP_S8bit	*pEndBuff,
	SIP_S8bit **ppOut,
	SipDateStruct *dt,
	SipError *err)
#else
	(pEndBuff,ppOut, dt, err)
	SIP_S8bit	*pEndBuff;
	SIP_S8bit **ppOut;
	SipDateStruct *dt;
	SipError *err;
#endif

{
	SIP_S8bit days[SIP_NO_DAYS][SIP_MAX_DAY_LEN]={"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	SIP_S8bit dMonth[SIP_NO_MONTHS][SIP_MAX_MONTH_LEN]={"Jan", "Feb", "Mar", "Apr", "May", \
							"Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	SIP_S8bit temp[SIP_TEMP_BUF_SIZE];
	SipDateFormat *df;
	SipTimeFormat *tf;
	SipError *dummy;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into  sip_formDateStruct");

	out = *ppOut;
	dummy = err;
	if (dt==SIP_NULL) return SipSuccess;
	if (dt->dDow != SipDayNone)
	{
		STRCAT ( pEndBuff,out, days[(int)(dt->dDow)]);
		STRCAT ( pEndBuff,out,", ");
	}
	df = dt->pDate;
	HSS_SNPRINTF((char *)temp,SIP_TEMP_BUF_SIZE,"%02d",df->dDay);
	temp[SIP_TEMP_BUF_SIZE-1]='\0';
	STRCAT ( pEndBuff,out, temp);
	STRCAT ( pEndBuff,out, " ");
	STRCAT ( pEndBuff,out, dMonth[(int)(df->dMonth)]);
	STRCAT ( pEndBuff,out," ");
	HSS_SNPRINTF((char *)temp, SIP_TEMP_BUF_SIZE, "%04d", df->dYear);
	temp[ SIP_TEMP_BUF_SIZE-1]='\0';
	STRCAT ( pEndBuff,out,temp);
	STRCAT ( pEndBuff,out, " ");
	/* Now get slTime */
	tf = dt->pTime;
	HSS_SNPRINTF((char *)temp, SIP_TEMP_BUF_SIZE, "%02d:%02d:%02d ",\
										tf->dHour, tf->dMin, tf->dSec );
	temp[ SIP_TEMP_BUF_SIZE-1]='\0';
	STRCAT ( pEndBuff,out, temp);
	STRCAT ( pEndBuff,out,"GMT");
	if(pEndBuff)*ppOut = out;

	SIPDEBUGFN("Exiting from sip_formDateStruct");

	return SipSuccess;
}

#ifdef SIP_MWI
/*****************************************************************
** FUNCTION: sip_formMesgSummaryBody
**
**
** DESCRIPTION: Converts a Message Waiting Summary body to Text
*****************************************************************/
SipBool sip_formMesgSummaryBody
#ifdef ANSI_PROTO
	( SIP_S8bit	*pEndBuff,
	MesgSummaryMessage *m,
	SIP_S8bit **ppOut,
	SipError *err)
#else
	(pEndBuff,m, ppOut, err)
	SIP_S8bit	*pEndBuff;
	MesgSummaryMessage *m;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SIP_U32bit index, size;
	SummaryLine *summary;
	SipNameValuePair *namevalue;
	SIP_S8bit digits[SIP_TEMP_BUF_SIZE];
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into sip_formMesgSummaryBody");

	out = *ppOut;

	/* Get Status */

	STRCAT(pEndBuff,out, "Messages-Waiting: ");
	if( m->dStatus == SipMsgWaitingNo )
		STRCAT(pEndBuff,out, "no");
	else
		STRCAT(pEndBuff,out, "yes");
 	STRCAT(pEndBuff,out,CRLF); 

    /*Form the Message-Account header */
	if (m->pAddrSpec != SIP_NULL)
	{
		SIP_S8bit addBracket=0;
	    
        STRCAT(pEndBuff,out, "Messages-Account: ");
	
        if( (sip_areThereParamsinAddrSpec(m->pAddrSpec,err) ) == \
                                                       SipSuccess)
        {
			addBracket=1;
        }

		if (addBracket)
        {
            STRCAT ( pEndBuff,out,"<");
        }
		
        if (SipFail==sip_formAddrSpec\
					(pEndBuff,&out, m->pAddrSpec, err))
		{
			return SipFail;
		}

		if (addBracket) 
        {
            STRCAT ( pEndBuff,out,">");
        }
	
    } /*end if (m->pAddrSpec != SIP_NULL)  */

	/* Get Summary Line */
	sip_listSizeOf( &(m->slSummaryLine), &size, err);
	index = 0;
	while (index < size)
	{
		sip_listGetAt (&(m->slSummaryLine), index, (SIP_Pvoid *) &summary, err);
		STRCAT(pEndBuff,out,summary->pMedia);
		STRCAT(pEndBuff,out,": ");
		HSS_SNPRINTF ( (char *)digits, SIP_TEMP_BUF_SIZE, "%u", \
				summary->newMessages);
		digits[SIP_TEMP_BUF_SIZE-1] = '\0';
		STRCAT(pEndBuff,out,digits);
		STRCAT(pEndBuff,out,"/");
		HSS_SNPRINTF ( (char *)digits, SIP_TEMP_BUF_SIZE, "%u", \
			summary->oldMessages);
		digits[SIP_TEMP_BUF_SIZE-1] = '\0';
		STRCAT(pEndBuff,out,digits);
		if((summary->newUrgentMessages)||(summary->oldUrgentMessages))
			/* if there are urgent messages */
		{
			STRCAT(pEndBuff,out,"(");
			HSS_SNPRINTF\
			( (char *)digits, SIP_TEMP_BUF_SIZE, "%u", \
					summary->newUrgentMessages);
			digits[SIP_TEMP_BUF_SIZE-1] = '\0';
			STRCAT(pEndBuff,out,digits);
			STRCAT(pEndBuff,out,"/");
			HSS_SNPRINTF \
			( (char *)digits, SIP_TEMP_BUF_SIZE, "%u",\
					 summary->oldUrgentMessages);
			digits[SIP_TEMP_BUF_SIZE-1] = '\0';
			STRCAT(pEndBuff,out,digits);
			STRCAT(pEndBuff,out,")");
		}
		STRCAT(pEndBuff,out,CRLF);
		index++;
	}

	/* Get HName and HValue */
	sip_listSizeOf( &(m->slNameValue), &size, err);
	index = 0;
	while (index < size)
	{
		sip_listGetAt (&(m->slNameValue), index, (SIP_Pvoid *) &namevalue, err);
		STRCAT(pEndBuff,out,CRLF);
		STRCAT(pEndBuff,out,namevalue->pName);
		STRCAT(pEndBuff,out,": ");
		STRCAT(pEndBuff,out,namevalue->pValue);
		index++;
	}


	if(pEndBuff)*ppOut = out;

	SIPDEBUGFN("Exiting from  sip_formMesgSummaryBody");
    return SipSuccess;
}

#endif /*#ifdef SIP_MWI */

/*****************************************************************
** FUNCTION: sip_formSdpBody
**
**
** DESCRIPTION: Converts a SdpMessage to Text
*****************************************************************/
SipBool sip_formSdpBody
#ifdef ANSI_PROTO
	( SIP_S8bit	*pEndBuff,
	SdpMessage *s,
	 SIP_S8bit **ppOut,
	  SipError *err)
#else
	(pEndBuff,s, ppOut, err)
	SIP_S8bit	*pEndBuff;
	SdpMessage *s;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SIP_U32bit index, size;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into sip_formSdpBody");

	out = *ppOut;
/* Get pVersion */
	if (s->pVersion !=SIP_NULL)
	{
		STRCAT(pEndBuff,out, "v=");
		STRCAT (pEndBuff,out, s->pVersion);
		STRCAT (pEndBuff,out,CRLF);
	}
/* Get SdpOrigin */
	if (s->pOrigin !=SIP_NULL)
	{
		STRCAT(pEndBuff,out,"o=");
		STRCAT (pEndBuff,out,s->pOrigin->pUser);
		STRCAT (pEndBuff,out," ");
		STRCAT (pEndBuff,out,s->pOrigin->pSessionid);
		STRCAT (pEndBuff,out," ");
		STRCAT (pEndBuff,out,s->pOrigin->pVersion);
		STRCAT (pEndBuff,out," ");
		STRCAT (pEndBuff,out,s->pOrigin->pNetType);
		STRCAT (pEndBuff,out," ");
		STRCAT (pEndBuff,out,s->pOrigin->pAddrType);
		STRCAT (pEndBuff,out," ");
		STRCAT (pEndBuff,out,s->pOrigin->dAddr);
		STRCAT (pEndBuff,out,CRLF);
	}

/* get Session */
	if (s->pSession !=SIP_NULL)
	{
		STRCAT(pEndBuff,out,"s=");
		STRCAT (pEndBuff,out, s->pSession);
		STRCAT (pEndBuff,out, CRLF);
	}

/* get Information */
	if (s->pInformation !=SIP_NULL)
	{
		STRCAT(pEndBuff,out,"i=");
		STRCAT (pEndBuff,out, s->pInformation);
		STRCAT (pEndBuff,out, CRLF);
	}

/* get Uri */
	if (s->pUri !=SIP_NULL)
	{
		STRCAT(pEndBuff,out,"u=");
		STRCAT (pEndBuff,out, s->pUri);
		STRCAT (pEndBuff,out, CRLF);
	}

/* get Email */
	sip_listSizeOf( &(s->slEmail), &size, err);
	index = 0;
	while (index < size)
	{
		SIP_S8bit *e;
		sip_listGetAt (&(s->slEmail), index, (SIP_Pvoid *) &e, err);
		STRCAT(pEndBuff,out,"e=");
		STRCAT (pEndBuff,out, e);
		STRCAT (pEndBuff,out, CRLF);
		index++;
	}

/* get Phone */
	sip_listSizeOf( &(s->slPhone), &size, err);
	index = 0;
	while (index < size)
	{
		SIP_S8bit *p;
		sip_listGetAt (&(s->slPhone), index, (SIP_Pvoid *) &p, err);
		STRCAT(pEndBuff,out,"p=");
		STRCAT (pEndBuff,out, p);
		STRCAT (pEndBuff,out, CRLF);
		index++;
	}

/* Get SdpConnection */
	if (s->slConnection != SIP_NULL)
	{
		STRCAT (pEndBuff,out, "c=");
		STRCAT (pEndBuff,out, s->slConnection->pNetType);
		STRCAT (pEndBuff,out," ");
		STRCAT (pEndBuff,out, s->slConnection->pAddrType);
		STRCAT (pEndBuff,out," ");
		STRCAT (pEndBuff,out, s->slConnection->dAddr);
		STRCAT (pEndBuff,out, CRLF);
	}

/* get Bandwidth */
	sip_listSizeOf( &(s->pBandwidth), &size, err);
	index = 0;
	while (index < size)
	{
		SIP_S8bit *b;
		sip_listGetAt (&(s->pBandwidth), index, (SIP_Pvoid *) &b, err);
		STRCAT(pEndBuff,out,"b=");
		STRCAT (pEndBuff,out, b);
		STRCAT (pEndBuff,out, CRLF);
		index++;
	}

/* Get SdpTime */
	sip_listSizeOf( &(s->slTime), &size, err);
	index = 0;
	while (index < size)
	{
		SdpTime *st;
		SIP_U32bit tsize, tindex;
		sip_listGetAt (&(s->slTime), index, (SIP_Pvoid *) &st, err);
		STRCAT(pEndBuff,out,"t=");
		STRCAT(pEndBuff,out, st->pStart);
		STRCAT(pEndBuff,out, " ");
		STRCAT(pEndBuff,out, st->pStop);

		/* Now add Repeat r= */

		sip_listSizeOf( &(st->slRepeat), &tsize, err);
		tindex = 0;
		while (tindex < tsize)
		{
			SIP_S8bit* r;
			STRCAT (pEndBuff,out, CRLF);
			sip_listGetAt ( &(st->slRepeat), tindex, (SIP_Pvoid *)&r, err);
			STRCAT (pEndBuff,out, "r=");
			STRCAT (pEndBuff,out, r);
			tindex++;
		}
		if (st->pZone !=SIP_NULL)
		{
			STRCAT (pEndBuff,out, CRLF);
			STRCAT (pEndBuff,out, "z=");
			STRCAT (pEndBuff,out, st->pZone);
		}
		STRCAT (pEndBuff,out, CRLF);
		index++;
	}


/* get Key */
	if (s->pKey != SIP_NULL)
	{
		STRCAT(pEndBuff,out,"k=");
		STRCAT (pEndBuff,out, s->pKey);
		STRCAT (pEndBuff,out, CRLF);
	}


/* Get SdpAttr */
	sip_listSizeOf( &(s->slAttr), &size, err);
	index = 0;
	while (index < size)
	{
		SdpAttr *a;
		sip_listGetAt (&(s->slAttr), index, (SIP_Pvoid *) &a, err);
		STRCAT(pEndBuff,out,"a=");
		STRCAT(pEndBuff,out, a->pName);
		if(a->pValue!=SIP_NULL)
		{
			STRCAT(pEndBuff,out, ":");
			STRCAT(pEndBuff,out, a->pValue);
		}
		STRCAT (pEndBuff,out, CRLF);
		index++;
	}

/* Get SdpMedia */
	sip_listSizeOf( &(s->slMedia), &size, err);
	index = 0;
	while (index < size)
	{
		/* pMediaValue, dPort, pPortNum, pProtocol, fmt are all
			part of m= line */
		SdpMedia *m;
		SIP_S8bit dPort[SIP_TEMP_BUF_SIZE];
		SIP_U32bit tsize, tindex;
		sip_listGetAt (&(s->slMedia), index, (SIP_Pvoid *) &m, err);
		if (m->pMediaValue != SIP_NULL) /* is there an m= line ? */
		{
			STRCAT(pEndBuff,out,"m=");
			STRCAT (pEndBuff,out, m->pMediaValue);
			STRCAT(pEndBuff,out, " ");
#ifdef SIP_ATM
			if ( m->pVirtualCID != SIP_NULL )
			{
				STRCAT(pEndBuff,out,m->pVirtualCID);
			}
			else
			{
				HSS_SNPRINTF ( (char *)dPort, SIP_TEMP_BUF_SIZE, \
								"%u", m->dPort);
				dPort[SIP_TEMP_BUF_SIZE-1]='\0';
				STRCAT (pEndBuff,out, dPort);
			}
#else
			HSS_SNPRINTF ( (char *)dPort, SIP_TEMP_BUF_SIZE, "%u", m->dPort);
			dPort[ SIP_TEMP_BUF_SIZE-1]='\0';
			STRCAT (pEndBuff,out, dPort);
#endif
			if (m->pPortNum !=SIP_NULL)
			{
				STRCAT (pEndBuff,out,"/");
				HSS_SNPRINTF ((char *)dPort, SIP_TEMP_BUF_SIZE, \
						"%u", *(m->pPortNum));
				dPort[ SIP_TEMP_BUF_SIZE-1]='\0';
				STRCAT(pEndBuff,out, dPort);
			}
			if ( m->pProtocol != SIP_NULL )
			{
				STRCAT (pEndBuff,out," ");
				STRCAT (pEndBuff,out, m->pProtocol);
			}
			if ( m->pFormat != SIP_NULL )
			{
				STRCAT (pEndBuff,out," ");
				STRCAT (pEndBuff,out, m->pFormat);
			}
#ifdef SIP_ATM
			{
				sip_listSizeOf( &(m->slProtofmt), &tsize, err);
				tindex = 0;
				while (tindex < tsize)
				{
					SipNameValuePair *c;
					sip_listGetAt ( &(m->slProtofmt), tindex, \
										(SIP_Pvoid *)&c, err);
					STRCAT (pEndBuff,out," ");
					STRCAT (pEndBuff,out, c->pName);
					STRCAT (pEndBuff,out," ");
					STRCAT (pEndBuff,out, c->pValue);
					tindex++;
				}
			}
#endif
			STRCAT (pEndBuff,out, CRLF);
		}
		if (m->pInformation != SIP_NULL) /* is there an i= ? */
		{
			STRCAT(pEndBuff,out,"i=");
			STRCAT(pEndBuff,out, m->pInformation);
			STRCAT (pEndBuff,out, CRLF);
		}
		/* Are there slConnection fields ? */
		sip_listSizeOf( &(m->slConnection), &tsize, err);
		tindex = 0;
		while (tindex < tsize)
		{
			SdpConnection *c;
			sip_listGetAt ( &(m->slConnection), tindex, (SIP_Pvoid *)&c, err);
			STRCAT (pEndBuff,out, "c=");
			STRCAT (pEndBuff,out, c->pNetType);
			STRCAT (pEndBuff,out," ");
			STRCAT (pEndBuff,out, c->pAddrType);
			STRCAT (pEndBuff,out," ");
			STRCAT (pEndBuff,out, c->dAddr);
			STRCAT (pEndBuff,out, CRLF);
			tindex++;
		}

		sip_listSizeOf( &(m->slBandwidth), &tsize, err);
		tindex = 0;
		while (tindex < tsize)
		{
			SIP_S8bit *b;
			sip_listGetAt (&(m->slBandwidth), tindex, (SIP_Pvoid *) &b, err);
			STRCAT(pEndBuff,out,"b=");
			STRCAT (pEndBuff,out, b);
			STRCAT (pEndBuff,out, CRLF);
			tindex++;
		}

		if (m->pKey != SIP_NULL) /* is there a k= ? */
		{
			STRCAT (pEndBuff,out,"k=");
			STRCAT (pEndBuff,out, m->pKey);
			STRCAT (pEndBuff,out, CRLF);
		}

		/* Are there Attr fields ? */
		sip_listSizeOf( &(m->slAttr), &tsize, err);
		tindex = 0;
		while (tindex < tsize)
		{
			SdpAttr *a;
			sip_listGetAt ( &(m->slAttr), tindex, (SIP_Pvoid *)&a, err);
			STRCAT (pEndBuff,out, "a=");
			STRCAT (pEndBuff,out, a->pName);
			if (a->pValue != NULL)
			/* since attribute can be <attribute:value> or <attribute> itself */
			{
				STRCAT (pEndBuff,out,":");
				STRCAT (pEndBuff,out, a->pValue);
			}
			STRCAT (pEndBuff,out, CRLF);
			tindex++;
		} /* while Attr */
	index++;
	}
	if(pEndBuff)*ppOut = out;
	SIPDEBUGFN("Exiting from  sip_formSdpBody");
	return SipSuccess;
} /* of formSdpBody */

/*****************************************************************
** FUNCTION: __checkHeaderTypeHop
**
**
** DESCRIPTION: Internal function to check if Header is of Type Hop-by-Hop
*****************************************************************/

SipBool __checkHeaderTypeHop
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	switch(dType)
	{
	case	SipHdrTypeHide  		:
	case	SipHdrTypeOrganization 		:
	case	SipHdrTypeProxyAuthenticate 	:
	case	SipHdrTypeProxyauthorization	:
	case	SipHdrTypeProxyRequire		:
	case	SipHdrTypeRecordRoute		:
	case	SipHdrTypeMaxforwards		:
	case	SipHdrTypeVia		:
#ifdef SIP_3GPP
	case	SipHdrTypePath:
	case	SipHdrTypeServiceRoute:
#endif
	case	SipHdrTypeRoute			:
						 return SipSuccess;
	default					:
						 return SipFail;
	}

}

/*****************************************************************
** FUNCTION: sip_formMessage
**
**
** DESCRIPTION: This forms the Text message with the order as given
** 				in Order Table.
**				s - SipMessage structure to be converted
**				options - structure containing messages formation
**					options like - short/full form for header names
**					comma separated/single line headers etc.
**				out - pre-allocated buffer into which the formed
**					text messsage is written into
**				dLength - length of the formed message returned in this
**
*****************************************************************/

/* New formMessage from order table */
SipBool sip_formMessage
#ifdef ANSI_PROTO
	(
	SipMessage *s,
	SipOptions *options,
	SIP_S8bit *out,
	SIP_U32bit *dLength,
	SipError *err)
#else
	( s, options,out, dLength, err)
	SipMessage *s;
	SipOptions *options;
	SIP_S8bit *out;
	SIP_U32bit *dLength;
	SipError *err;
#endif
{
	SipBool retval=SipFail;
	SIP_U32bit sipheaderlength;
	SIP_U32bit cpsize,cpiter,i,index;
	SIP_S32bit siplistmap[HEADERTYPENUM];
	SIP_S8bit dCodeNum[SIP_RESP_CODE_LEN];
	SIP_U32bit hdrcount,addcount,count;
	en_HeaderType hdrtype;
	en_HeaderForm form;
	SipHeaderOrderInfo *order,*ordertemp;
	SipContentTypeHeader *ctypehdr;
	SIP_U32bit bodylength;
	SIP_S8bit* pStartBuff;
	SIP_S8bit* pEndBuff;
	SIP_U32bit	dTempLength=0;

	SIPDEBUGFN("Entering into sip_formMessage");

	pStartBuff = out;

	/*Get the address of the last byte that can be populated*/
	if(options->dOption&SIP_OPT_MAXBUFSIZE)
	{
		if(*dLength == 0)
		{
			*err = E_BUF_OVERFLOW;
			return SipFail;
		}
		pEndBuff = out + *dLength;
	}
	else
	{
		pEndBuff = out + SIP_MAX_MSG_SIZE;
	}

	/* initialize the buffer */
	*out = 0;

	/* Check if options has both FULL and SHORT Header options
	   and return error */
	if((options->dOption&(SIP_OPT_FULLFORM|SIP_OPT_SHORTFORM))==\
		(SIP_OPT_FULLFORM|SIP_OPT_SHORTFORM))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	/* Check if options has both COMMA SEPARATED and SINGLE Header options
	   and return error */

	if((options->dOption&(SIP_OPT_COMMASEPARATED|SIP_OPT_SINGLE))==\
		(SIP_OPT_COMMASEPARATED|SIP_OPT_SINGLE))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	/* Convert all headers to FULL form till Authorization Header */

	if((options->dOption&(SIP_OPT_FULLFORM))==(SIP_OPT_FULLFORM))
	{
		sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);

		for (cpiter = 0;cpiter < cpsize;cpiter++)
		{
			__sip_setHeaderFormAtHeaderLine(s,cpiter,SipFormFull,err);
		}
	}

	/* Convert all headers to SHORT form till Authorization Header*/
	if((options->dOption&(SIP_OPT_SHORTFORM))==(SIP_OPT_SHORTFORM))
	{
		sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);

		for (cpiter = 0;cpiter < cpsize;cpiter++)
		{
			__sip_setHeaderFormAtHeaderLine(s,cpiter,SipFormShort,err);
		}
	}
	/* Convert all headers into SINGLE form till Authorization Header */
	if((options->dOption&(SIP_OPT_SINGLE))==(SIP_OPT_SINGLE))
	{
		sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);
		cpiter = 0;
		while (cpiter < cpsize)
		{
			__sip_getHeaderTypeAtHeaderLine(s,cpiter,&hdrtype,err);

			__sip_getHeaderCountFromHeaderLine(s,cpiter,&hdrcount,err);
			if(hdrcount>1)
			{
				__sip_getHeaderFormFromHeaderLine(s,cpiter,&form,err);
				for(i=cpiter;i<(cpiter+hdrcount);i++)
				{
					if(sip_initSipHeaderOrderInfo(&order,err)==SipFail)
						return SipFail;
					order->dType=hdrtype;
					order->dTextType=form;
					order->dNum=1;

					if(i==cpiter)
						sip_listSetAt(&(s->slOrderInfo),i,(SIP_Pvoid*)order,\
							err);
					else
						if(sip_listInsertAt(&(s->slOrderInfo),i,\
							(SIP_Pvoid*)order,err) == SipFail)
							return SipFail;
				}
				cpiter+=hdrcount;
				sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);
			}
			else
				cpiter++;
		}
	}
	/* Convert all headers to COMMA SEPARATED headers till Auth Header */
	if((options->dOption&(SIP_OPT_COMMASEPARATED))==(SIP_OPT_COMMASEPARATED))
	{
		/* siplistmap contains an entry for each kind of header.
		   Initialize this to -1 */
		for(i=0;i<HEADERTYPENUM;i++)
			siplistmap[i]=-1;
		sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);
		cpiter=0;
		while(cpiter<cpsize)
		{
			__sip_getHeaderTypeAtHeaderLine(s,cpiter,&hdrtype,err);

			if(siplistmap[hdrtype]!=-1)
			{
				/* Secong occurance of this kind of header */
				/* Get the line where it was last found */
				index=siplistmap[hdrtype];
				/* Get the count number of headers in this line and the last
				   line where it was found */
				__sip_getHeaderCountFromHeaderLine(s,cpiter,&addcount,err);
				__sip_getHeaderCountFromHeaderLine(s,index,&count,err);
				/* Move all headers in this line to the first line where the
				   the header was found */
				__sip_setHeaderCountInHeaderLine(s,index,(count+addcount),err);
				/* Delete this entry */
				sip_listDeleteAt(&(s->slOrderInfo), cpiter, err);
				/* Update the order-info count */
				sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);
			}
			else
			{
				/* This type of header is encountered for the first time */
				/* Store the line in which this header was found */
				siplistmap[hdrtype]=cpiter;
				cpiter++;
			}
		}
	}

	/* Reorder the order table to place Hop by Hop headers at the end */
	if((options->dOption&(SIP_OPT_REORDERHOP))==(SIP_OPT_REORDERHOP))
	{
		sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);
		cpiter=0;index=0;
		while(cpiter<cpsize)
		{
			__sip_getHeaderTypeAtHeaderLine(s,cpiter,&hdrtype,err);
			if(__checkHeaderTypeHop(hdrtype)==SipSuccess)
			{
				sip_listGetAt (&(s->slOrderInfo), cpiter, (SIP_Pvoid *)&order,\
					err);
				if(sip_initSipHeaderOrderInfo(&ordertemp,err)==SipFail)
					return SipFail;
				*ordertemp=*order;
				if(sip_listInsertAt(&(s->slOrderInfo),index,\
					(SIP_Pvoid*)ordertemp,err) == SipFail)
					return SipFail;
				index++;
				cpiter++;
				sip_listDeleteAt(&(s->slOrderInfo), cpiter, err);
				sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);
			}
			else
				cpiter++;
		}
	}

	INC_API_COUNT
	if (err==SIP_NULL) return SipFail;
	*err = E_INV_PARAM;
	if (s==SIP_NULL) return SipFail;
	*err =  E_NO_ERROR;

	if(s->dType == SipMessageRequest)
	{
		/* Get Request Line */
		if (s->u.pRequest->pRequestLine!=SIP_NULL)
		{
			STRCAT ( pEndBuff,out,(char *)s->u.pRequest->pRequestLine->pMethod);
			STRCAT ( pEndBuff,out," ");
			if (SipFail==sip_formAddrSpec \
				(pEndBuff,&out, s->u.pRequest->pRequestLine->pRequestUri, err))
			{
				return SipFail;
			}

			STRCAT ( pEndBuff,out," ");
			STRCAT ( pEndBuff,out, s->u.pRequest->pRequestLine->pVersion);
			STRCAT ( pEndBuff,out,CRLF);
		}
		else
		{
			if((options->dOption&(SIP_OPT_NOSTARTLINE))!= (SIP_OPT_NOSTARTLINE))
			{
				*err = E_INV_PARAM;
				return SipFail;
			}
		}
	}
	else if(s->dType == SipMessageResponse)
	{
		/* Get Stat Line */
		if (s->u.pResponse->pStatusLine!=SIP_NULL)
		{
			STRCAT(pEndBuff,out,(char *)s->u.pResponse->pStatusLine->pVersion);
			STRCAT ( pEndBuff,out," ");
			HSS_SNPRINTF((char *)dCodeNum, 4, "%03d",\
				s->u.pResponse->pStatusLine->dCodeNum);
			dCodeNum[ 4-1]='\0';
			STRCAT ( pEndBuff,out,dCodeNum);
			STRCAT ( pEndBuff,out," ");

			/*Reason phrase can be null*/
			if (s->u.pResponse->pStatusLine->pReason!=SIP_NULL)
				STRCAT ( pEndBuff,out,s->u.pResponse->pStatusLine->pReason);

			STRCAT ( pEndBuff,out,CRLF);
		}
		else
		{
			if((options->dOption&(SIP_OPT_NOSTARTLINE))!= (SIP_OPT_NOSTARTLINE))
			{
				*err = E_INV_PARAM;
				return SipFail;
			}
		}
	}

	/* Request/Status Line formed. Now deal with the headers */
	/* Get the number of order-info entries in the message */
	/* Each line in the message has one order-entry indicating the
	   the header in that line, whether it is in full/short form and
	   the number of headers in that line */

	sip_listSizeOf (&(s->slOrderInfo),&cpsize,err);
	cpiter = 0;

	/* The siplistmap array contains one entry for each kind of header */
	/* This will track the number headers of that have been formed for a
	   particular type of header */
	for(i=0;i<HEADERTYPENUM;i++)
		siplistmap[i]=0;

	/* Iterate through the header order list and for each header line */
	if((options->dOption&(SIP_OPT_NOSTARTLINE))==(SIP_OPT_NOSTARTLINE))
		retval=SipSuccess;

	while (cpiter < cpsize)
	{
		retval=sip_formEachHeader\
						(pEndBuff,cpiter,s,&out,(SIP_U32bit *) siplistmap,err);
		if (retval!=SipSuccess)
		{
			return SipFail;
		}
		cpiter++;
	}

	/* Through with the SIP headers. Now form Body */
	sipheaderlength = out - pStartBuff;
	bodylength = 0;
	ctypehdr=s->pGeneralHdr->pContentTypeHdr;

	if(sip_formMimeBody(pEndBuff,s->slMessageBody,ctypehdr,&out,\
		&bodylength,err)==SipFail)
	{
		return SipFail;
	}

	/* Now reduce bodylen by 2 since top CRLF has been counted*/
	if (bodylength != 0) bodylength -=2;
	if((options->dOption&(SIP_OPT_CLEN))==(SIP_OPT_CLEN))
	{
		/* Content length if present has to be updated */
		/* If not, a new header is to be added at the end */
		SIP_S8bit constructedclen[SIP_CLEN_SIZE];
		SIP_U32bit ctypeoffset,crlfoffset,ctypehdrlen;

		out=pStartBuff;

		/* Check if message has a content-length header */
		/* Check for short form first */
		if(glbSipParserMemStrStr(out,sipheaderlength,\
			(SIP_S8bit *) "\r\nl:",&ctypeoffset)\
			==SipFail)
		{
			/* short form not present. check for long form */
			if(glbSipParserMemStrStr(out,sipheaderlength,\
				(SIP_S8bit *) "\r\nContent-Length:",&ctypeoffset)==SipFail)
			{
				/* Message does not have a content-length header */
				ctypeoffset = sipheaderlength-2;
				/* ctypeoffset points to CRLF after last header */
				if((options->dOption&(SIP_OPT_SHORTFORM))==(SIP_OPT_SHORTFORM))
					HSS_SPRINTF(constructedclen,"l: %d\r\n",\
						bodylength);

				else
					HSS_SPRINTF(constructedclen,"Content-Length: %d\r\n",\
						bodylength);
			}
			else
			{
				/* Long form found */
				/* ctypeoffset points to the CRLF before the clen header */
				HSS_SPRINTF(constructedclen,"Content-Length: %d\r\n",\
					bodylength);
			}
		}
		else
		{
			/* Short form found - so form a short form header */
			/* ctypeoffset points to the CRLF before the clen header */
			HSS_SPRINTF(constructedclen,"l: %d\r\n",bodylength);
		}
		/* ctypehdrlen is the length of the constructed clen header */
		/* This includes the CRLF at the end of the header but not the '\0' */
		ctypehdrlen = strlen(constructedclen);
		/* Now check for CRLF after the clen header found in the message */
		/* crlfoffset will contain the offset of the CRLF the
		   content-type header in the message - so length of the
		   existing clen header excluding the CRLF 
			2 is CRLF size*/
		if(glbSipParserMemStrStr(&(out[ctypeoffset+2]),\
			sipheaderlength-ctypeoffset-2,(SIP_S8bit *) "\r\n",\
			&crlfoffset)==SipFail)
		{
			/* No CRLF found - message did not have a content length header */
			crlfoffset = 0;
			if(bodylength!=0)
			{
				/*Check for OVERFLOW*/
				dTempLength=sipheaderlength+bodylength+\
											4-(ctypeoffset+2+crlfoffset)+1;
				if (out+ctypeoffset+ctypehdrlen+dTempLength >=pEndBuff)
				{
					*err = E_BUF_OVERFLOW;
					return SipFail;
				}

				memmove(&(out[ctypeoffset+ctypehdrlen]),\
					&(out[ctypeoffset+crlfoffset]),\
					dTempLength);
			}
			else
			{
				/*Check For OVERFLOW*/
				dTempLength=sipheaderlength+2-(ctypeoffset+2+crlfoffset)+1;
				if (out+ctypeoffset+ctypehdrlen+dTempLength >= pEndBuff)
				{
					*err = E_BUF_OVERFLOW;
					return SipFail;
				}

				memmove(&(out[ctypeoffset+ctypehdrlen]),\
					&(out[ctypeoffset+crlfoffset]),\
					dTempLength);
			}
			*dLength = ctypehdrlen+sipheaderlength+bodylength-crlfoffset+2;
		}
		/* Now move stuff after the existing clen header to the position
		   where the clen we will insert is going to end */
		/* source - ctypeoffset+crlfoffset+2 =
		   (CRLF before existing clen)+(length of existing clen excluding CRLF)
		   +(2 to account for ctypeoffset poiting to CRLF)
		   = CRLF before header following clen header.
		   if no clen - source is '\0' after the last CRLF
		   destination - ctypeoffset+ctypeheaderlen =
		   (CRLF before existing clen)+(length of the constructed clen)
		*/
		else
		{
			dTempLength=sipheaderlength+bodylength+2-\
												(ctypeoffset+2+crlfoffset)+1;
			if (out+ctypeoffset+ctypehdrlen+dTempLength>pEndBuff)
			{
				return SipFail;
			}

			memmove(&(out[ctypeoffset+ctypehdrlen]),\
				&(out[ctypeoffset+crlfoffset+2]),\
				dTempLength);
			*dLength = ctypehdrlen+sipheaderlength+bodylength-crlfoffset;
		}

		/* Move the clen header we constructed to fit the gap created above */
		/*Check For OVERFLOW*/
		if (out+ctypeoffset+2+ctypehdrlen>=pEndBuff)
		{
			*err = E_BUF_OVERFLOW;
			return SipFail;
		}
		memmove(&(out[ctypeoffset+2]),constructedclen,ctypehdrlen);
	}
	else
		*dLength = sipheaderlength + bodylength + 2;
	if(bodylength==0)
	{
		STRCAT(pEndBuff,out,CRLF);
	}

	SIPDEBUGFN("Exiting from  sip_formMessage");
	return retval;
}
/*****************************************************************
** FUNCTION: sip_formMimeBody
**
**
** DESCRIPTION: This forms the Body part of the SIP Message
*****************************************************************/

SipBool sip_formMimeBody
#ifdef ANSI_PROTO
(
	SIP_S8bit	*pEndBuff,
	SipList mbodyList,
	SipContentTypeHeader *ctypehdr,
	SIP_S8bit **ppOut,
	SIP_U32bit *dLength,
	SipError *err
)
#else
( pEndBuff,mbodyList, ctypehdr,ppOut, dLength, err)
	SIP_S8bit	*pEndBuff;
	SipList mbodyList;
	SipContentTypeHeader *ctypehdr;
	SIP_S8bit **ppOut;
	SIP_U32bit *dLength;
	SipError *err;
#endif
{
	SIP_U32bit count,i,index1,size1,size2,iterator,boundarylen;
	SipMsgBody *msgbody;
	SIP_S8bit *boundary,*separator;
	SIP_S8bit *tempout;

	SIPDEBUGFN("Entering into sip_formMimeBody");

	tempout = *ppOut;
	if(sip_listSizeOf(&(mbodyList),&count,err)!=SipSuccess)
		return SipFail;
	/* Validity check  for parameters */
	if((ctypehdr == SIP_NULL)||(count==0))
	{
		*dLength = 0;
	}

	if(ctypehdr!=SIP_NULL)
	{
		if ( ctypehdr->pMediaType != SIP_NULL )
		{
			/* Check if multipart and find separator if it is multipart */
			if (strcasecmp(ctypehdr->pMediaType,"multipart/mixed")==0)
			{
				if(sip_listSizeOf(&(ctypehdr->slParams),&count,err)==SipFail)
					return SipFail;
				iterator = 0;
				boundary = SIP_NULL;
				while(iterator<count)
				{
					SipParam *param;
					if(sip_listGetAt(&(ctypehdr->slParams),iterator,\
						(SIP_Pvoid *)&(param),err)==SipFail)
						return SipFail;
					if(strcmp(param->pName,"boundary")==0)
					{
						if(sip_listGetAt(&(param->slValue),0,(SIP_Pvoid*)&boundary,\
							err)==SipFail)
								return SipFail;
						break;
					}
					iterator++;
				}
				if(boundary==SIP_NULL)
					return SipFail;
				/* Split message Body into constituent elements */
				separator = (SIP_S8bit *) fast_memget(FORM_MEM_ID,\
					sizeof(SIP_S8bit)*strlen(boundary)+5,err);
				if(separator==SIP_NULL)
						return SipFail;
				strcpy(separator,"--");
				if(boundary[0]=='\"')
				{
					strcat(separator,&(boundary[1]));
					separator[strlen(boundary)] = '\0';
					boundarylen = strlen(boundary)-2;
				}
				else
				{
					strcat(separator,boundary);
					boundarylen = strlen(boundary);
				}
				/* Check if the multipart message has been read into unknown body */
				/* There will be one MsgBody in the list and its type will be */
				/* SipUnknownBody. The body will begin with the boundary */
				if(sip_listSizeOf(&(mbodyList),&count,err)!=SipSuccess)
					return SipFail;
				if(count==1)
				{
					SipMsgBody *tempmsgbody;
					if(sip_listGetAt(&(mbodyList),0,(SIP_Pvoid *)&tempmsgbody,err)\
						==SipFail)
						return SipFail;
					if(tempmsgbody->dType==SipUnknownBody)
					{
						if ( tempmsgbody->u.pUnknownMessage != SIP_NULL )
						{
							if(strncmp(tempmsgbody->u.pUnknownMessage->pBuffer,\
								separator, strlen(separator))==0)
							{/* separator shouldn't be printed out. */
								fast_memfree(FORM_MEM_ID,separator,err);
								separator=SIP_NULL;
							}
						}
					}
				}
			}
			else
				separator=SIP_NULL;
		}
		else
			separator=SIP_NULL;
	}
	else
		separator=SIP_NULL;
	count=0;
	/* Iterate through the list of message bodies */
	if(sip_listSizeOf(&(mbodyList),&count,err)==SipSuccess)
	{
		if(count >0)
			STRCAT ( pEndBuff, tempout,CRLF);
		for(i=0;i<count;i++)
		{
			if(separator!=SIP_NULL)
			{
				/*if(i!=0)
					STRCAT ( pEndBuff, tempout,CRLF);*/
				STRCAT ( pEndBuff, tempout,separator);
				STRCAT ( pEndBuff, tempout,CRLF);
			}

			if(sip_listGetAt(&(mbodyList),i,(SIP_Pvoid *)&msgbody,err)==SipFail)
			{
				sip_freeString(separator);
				return SipFail;
			}
			/* get Content-Type */
			if(msgbody->pMimeHeader !=SIP_NULL)
			{
				if (msgbody->pMimeHeader->pContentType !=SIP_NULL)
				{
					STRCAT(pEndBuff,tempout,"Content-Type: ");
					if (msgbody->pMimeHeader->pContentType->pMediaType\
						!=SIP_NULL)
					{
						STRCAT (pEndBuff,tempout,\
							msgbody->pMimeHeader->pContentType->pMediaType);
					}
					sip_listSizeOf(&(msgbody->pMimeHeader->pContentType->\
						slParams), &size1, err);
					index1 = 0;
					while (index1 < size1)
					{
						SipParam *sParam;
						sip_listGetAt (&(msgbody->pMimeHeader->pContentType->\
							slParams), index1, (SIP_Pvoid *) &sParam, err);
						STRCAT (pEndBuff,tempout, ";");
						STRCAT(pEndBuff,tempout,sParam->pName);
						STRCAT(pEndBuff,tempout,"=");
						sip_listSizeOf( &(sParam->slValue), &size2, err);
						if ( size2>=1)
						{
							SIP_S8bit *e;
							sip_listGetAt (&(sParam->slValue), 0,\
								(SIP_Pvoid *) &e, err);
							STRCAT(pEndBuff,tempout,e);
						}
						index1++;
					}
				}
				/* get Content-ID */
				if (msgbody->pMimeHeader->pContentId !=SIP_NULL)
				{
					STRCAT (pEndBuff,tempout, CRLF);
					STRCAT (pEndBuff,tempout,"Content-ID: ");
					STRCAT (pEndBuff,tempout, msgbody->pMimeHeader->pContentId);
				}

				/* get Content-Description */
				if (msgbody->pMimeHeader->pContentDescription !=SIP_NULL)
				{
					STRCAT (pEndBuff,tempout, CRLF);
					STRCAT (pEndBuff,tempout,"Content-Description: ");
					STRCAT (pEndBuff,tempout, \
									msgbody->pMimeHeader->pContentDescription);
				}

				/* get Content Disposition */
				if (msgbody->pMimeHeader->pContentDisposition !=SIP_NULL)
				{
					STRCAT ( pEndBuff, tempout,CRLF);
					STRCAT ( pEndBuff, tempout, "Content-Disposition: ");
					STRCAT ( pEndBuff, tempout, msgbody->pMimeHeader->\
								pContentDisposition->pDispType);
                   	if (SipFail==sip_formSipParamList(pEndBuff,&tempout,\
						&(msgbody->pMimeHeader->pContentDisposition->slParam),\
								 (SIP_S8bit *) ";", 1, err))
					{
						return SipFail;
					}
				}

				/* get ContentTransEncoding */
				if (msgbody->pMimeHeader->pContentTransEncoding !=SIP_NULL)
				{
					STRCAT (pEndBuff,tempout, CRLF);
					STRCAT (pEndBuff,tempout,"Content-Transfer-Encoding: ");
					STRCAT (pEndBuff,tempout,\
						msgbody->pMimeHeader->pContentTransEncoding);
				}

				/* Unknown Mime headers */
				sip_listSizeOf(&(msgbody->pMimeHeader->slAdditionalMimeHeaders)\
					,&size1,err);
				index1=0;
				while(index1<size1)
				{
					SIP_S8bit *unknownHeader;
					sip_listGetAt(&(msgbody->pMimeHeader->\
						slAdditionalMimeHeaders),index1, (SIP_Pvoid *)\
						&unknownHeader, err);
					STRCAT (pEndBuff,tempout, CRLF);
					STRCAT (pEndBuff,tempout,unknownHeader);
					index1++;
				}
			}
			else if(separator!=SIP_NULL)
			{
				STRCAT (pEndBuff,tempout, CRLF);
			}

			if(msgbody->dType == SipSdpBody)
			{
				/* Form SDP Body*/
				if(msgbody->pMimeHeader !=SIP_NULL)
				{
					STRCAT(pEndBuff,tempout,CRLF);
				}
				if ( msgbody->u.pSdpMessage != SIP_NULL )
				{
					if ( msgbody->pMimeHeader != SIP_NULL )
						STRCAT(pEndBuff,tempout,CRLF);
					if (SipFail==sip_formSdpBody\
							(pEndBuff,msgbody->u.pSdpMessage, &tempout, err))
					{
						return SipFail;
					}
				}
			}
#ifdef SIP_MWI
		 	else if(msgbody->dType == SipMessageSummaryBody)
            {
				/* Form SDP Body*/
				if(msgbody->pMimeHeader !=SIP_NULL)
				{
					STRCAT(pEndBuff,tempout,CRLF);
				}
				if ( msgbody->u.pSummaryMessage != SIP_NULL )
				{
					if ( msgbody->pMimeHeader != SIP_NULL )
						STRCAT(pEndBuff,tempout,CRLF);
					if (SipFail==sip_formMesgSummaryBody\
						(pEndBuff,msgbody->u.pSummaryMessage, &tempout , err))
					{
						return SipFail;
					}
					if ( separator != SIP_NULL )
						STRCAT(pEndBuff,tempout,CRLF);
				}
            }
#endif
			else if(msgbody->dType == SipUnknownBody)
			{
				/* Form Unknown Body */
				if(msgbody->pMimeHeader !=SIP_NULL)
				{
					STRCAT(pEndBuff,tempout,CRLF);
					*dLength+=2;
				}
				/*Check for BUFFER_OVERFLOW*/
				if ( msgbody->u.pUnknownMessage != SIP_NULL )
				{
					if ( msgbody->pMimeHeader != SIP_NULL )
					{
						STRCAT(pEndBuff,tempout,CRLF);
						*dLength+=2;
					}
					if (tempout+msgbody->u.pUnknownMessage->dLength >= pEndBuff)
					{
						*err=E_BUF_OVERFLOW;
						return SipFail;
					}
					memcpy(tempout,msgbody->u.pUnknownMessage->pBuffer,\
							msgbody->u.pUnknownMessage->dLength);

					/* -1 removed */
					tempout += msgbody->u.pUnknownMessage->dLength;
					*dLength += msgbody->u.pUnknownMessage->dLength;
					*tempout = '\0';
					STRCAT(pEndBuff,tempout,CRLF);
				}
			}
			else if(msgbody->dType == SipIsupBody)
			{
				/* Form ISUP Message */
				if(msgbody->pMimeHeader !=SIP_NULL)
				{
					STRCAT(pEndBuff,tempout,CRLF);
					*dLength+=2;
				}

				/*Check for BUFFER_OVERFLOW*/
				if (msgbody->u.pIsupMessage != SIP_NULL)
				{
					if ( msgbody->pMimeHeader != SIP_NULL )
					{
						STRCAT(pEndBuff,tempout,CRLF);
						*dLength+=2;
					}
					if (tempout+msgbody->u.pIsupMessage->dLength >= pEndBuff)
					{
						*err=E_BUF_OVERFLOW;
						return SipFail;
					}
					memcpy(tempout,msgbody->u.pIsupMessage->pBody,\
						msgbody->u.pIsupMessage->dLength);
					/* -1 removed */
					tempout += msgbody->u.pIsupMessage->dLength;
					*dLength += msgbody->u.pIsupMessage->dLength;
					*tempout = '\0';
					if ( separator != SIP_NULL )
						STRCAT(pEndBuff,tempout,CRLF);
				}
			}
			else if(msgbody->dType == SipQsigBody)
			{
				/* Form QSIG Message */
				if(msgbody->pMimeHeader !=SIP_NULL)
				{
					STRCAT(pEndBuff,tempout,CRLF);
					*dLength+=2;
				}

				/*Check for BUFFER_OVERFLOW*/
				if ( msgbody->u.pQsigMessage != SIP_NULL )
				{
					if ( msgbody->pMimeHeader != SIP_NULL )
					{
						STRCAT(pEndBuff,tempout,CRLF);
						*dLength+=2;
					}
					if (tempout+msgbody->u.pQsigMessage->dLength >= pEndBuff)
					{
						*err=E_BUF_OVERFLOW;
						return SipFail;
					}
					/* CSR  1-1427584 ** requested by NexTone** */ 
                    			memcpy(tempout,msgbody->u.pQsigMessage->pBody,\
					msgbody->u.pQsigMessage->dLength);

					/* -1 removed */
					tempout += msgbody->u.pQsigMessage->dLength;
					*dLength += msgbody->u.pQsigMessage->dLength;
					*tempout = '\0';
					if ( separator != SIP_NULL )
						STRCAT(pEndBuff,tempout,CRLF);
				}
			}
			else if(msgbody->dType == SipMultipartMimeBody)
			{
				/* Form a Multipart message */
				SIP_U32bit lengthchange;
				lengthchange = *dLength;
				/* Call sip_formMimeBody recursively in case of multipart */
				if(msgbody->u.pMimeMessage!=SIP_NULL)
				{
					if (SipFail==sip_formMimeBody(pEndBuff,\
						msgbody->u.pMimeMessage->slRecmimeBody,\
						msgbody->pMimeHeader->pContentType,\
						&tempout,dLength,err))
					{
						return SipFail;
					}
				}
				lengthchange = (*dLength)-lengthchange;
			}
			else if(msgbody->dType == SipAppSipBody)
			{
				/* Form SIP message in body */
				SIP_U32bit lengthchange;
				SipOptions formOption;

				if(msgbody->pMimeHeader !=SIP_NULL)
				{
					STRCAT(pEndBuff,tempout,CRLF);
					STRCAT(pEndBuff,tempout,CRLF);
					*dLength+=4;
				}
				formOption.dOption = SIP_OPT_NOSTARTLINE|SIP_OPT_FULLFORM\
					| SIP_OPT_MAXBUFSIZE;
				/*Calculate the remaining Length */
				lengthchange = pEndBuff - tempout;
				if(sip_formMessage(msgbody->u.pAppSipMessage, &formOption,
					tempout, &lengthchange, err)==SipFail)
				{
					return SipFail;
				}
				*dLength = *dLength + lengthchange;
				tempout+=lengthchange;
			}
		}
		if(separator!=SIP_NULL)
		{
			strcat(separator,"--");
			STRCAT(pEndBuff,tempout,separator);
			STRCAT(pEndBuff,tempout,CRLF);
		}
	}
	else
	{
		sip_freeString(separator);
		return SipFail;
	}
	*dLength = tempout - *ppOut;
	sip_freeString(separator);

	if(pEndBuff) *ppOut=tempout;
	SIPDEBUGFN("Exiting from sip_formMimeBody");

	return SipSuccess;
}

/*****************************************************************
** FUNCTION: sip_formEachHeader
**
**
** DESCRIPTION: This function forms each Header in the SIP Message
*****************************************************************/

SipBool sip_formEachHeader
#ifdef ANSI_PROTO
	(
	SIP_S8bit	*pEndBuff,
	SIP_U32bit cpiter,
	SipMessage *s,
	 SIP_S8bit **ppOut,
	SIP_U32bit siplistmap[],
	 SipError *err
	 )
#else
	(pEndBuff, cpiter,s, ppOut, siplistmap,err)
	SIP_S8bit	*pEndBuff;
	SIP_U32bit cpiter;
	SipMessage *s;
	SIP_S8bit **ppOut;
	SIP_U32bit siplistmap[];
	SipError *err;
#endif
{
	SIP_U32bit hdrcount,i,initial;
	en_HeaderType hdrtype;
	SIP_S8bit* out;


	SIPDEBUGFN("Entering into sip_formEachHeader");

	out = *ppOut;

	/* Get the type of the header to be formed in the current line
	   and the number of such headers that need to be formed */
	__sip_getHeaderCountFromHeaderLine(s,cpiter,&hdrcount,err);
	__sip_getHeaderTypeAtHeaderLine(s,cpiter,&hdrtype,err);
	/* Get the number of headers of this type that have already been formed
	   This gives the index of the header from which we need to start.
	   Eg., If we have already formed till

	   INVITE sip:.....
	   Via: SIP/2.0/UDP addr1, SIP/2.0/TCP addr2
	   Route: myroute

	   and cpiter is 2 with type indicating	Via, the siplistmap entry
	   for the Via header will indicate 2.
	*/
	initial=siplistmap[hdrtype];
	/* Update the siplistmap entry with the number of headers that will
	   be formed now */
	siplistmap[hdrtype]+=hdrcount;

	/* Form the first header in the line with the name of the header */
	if (SipFail == sip_formSingleHeader\
			(pEndBuff,hdrtype,initial,SipModeNew,s,&out,err))
	{
		return SipFail;
	}

	/* Form the rest of the headers in the join mode (ie. no header name,
	   just a comma preceding the header. */
	for(i=initial+1;i<(initial+hdrcount);i++)
	{
		if(hdrtype == SipHdrTypeAuthorization || \
			hdrtype == SipHdrTypeProxyAuthenticate || \
			hdrtype == SipHdrTypeProxyauthorization || \
			hdrtype == SipHdrTypeWwwAuthenticate )
		{
			if (SipFail==sip_formSingleHeader\
						(pEndBuff,hdrtype,i,SipModeNew,s,&out,err))
			{
				return SipFail;
			}
		}
		else
		{
			if(hdrtype != SipHdrTypeUnknown)
			{
			/* Move the string pointer back by two so that the CRLF after the
			   first header is ignored */
				out-=2;
				if (SipFail==sip_formSingleHeader\
							(pEndBuff,hdrtype,i,SipModeJoin,s,&out,err))
				{
					return SipFail;
				}
			}
			else
			{
				/* Unknown headers should not be joined even if the order
				   entry indicates that. This is because unknown headers may
				   have different names though their type is different.
				*/
				if (SipFail==sip_formSingleHeader\
							(pEndBuff,hdrtype,i,SipModeNew,s,&out,err))
				{
					return SipFail;
				}
			}
		}
	}
	if(pEndBuff)*ppOut = out;

	SIPDEBUGFN("Exiting from sip_formEachHeader");

	return SipSuccess;
}
/*============= These APIs are included for converting Headers to Strings===*/

/*****************************************************************
** FUNCTION: sip_formSingleGeneralHeader
**
**
** DESCRIPTION: Converts a SipGeneralHeader to Text
*****************************************************************/


SipBool sip_formSingleGeneralHeader
#ifdef ANSI_PROTO
	(
	SIP_S8bit	*pEndBuff,
	en_HeaderType dType,
	 SIP_U32bit ndx,
	 en_AdditionMode mode,
	 en_HeaderForm form,
	 SipGeneralHeader *g,
	 SIP_S8bit **ppOut,
	 SipError *err)
#else
	(pEndBuff,dType, ndx, mode, form,g,ppOut,err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	 en_HeaderForm form;
	SipGeneralHeader *g;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SipBool res;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into sip_formSingleGeneralHeader");

	out = *ppOut;
/* Now convert requested Header */
	switch (dType)
	{
			/*=============================================================*/
			case SipHdrTypeAccept:
			{
				SipAcceptHeader *a;
				res = sip_listGetAt (&(g->slAcceptHdr), ndx, \
					(SIP_Pvoid *) &a, err);
				if ( res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out, "Accept: ");
				else
					STRCAT ( pEndBuff,out, ",");

					STRCAT ( pEndBuff,out, a->pMediaRange);
					STRCAT ( pEndBuff,out, a->pAcceptRange);
					STRCAT ( pEndBuff,out, CRLF);
					break;
			}

			/*=============================================================*/
				case SipHdrTypeRetryAfterDate:
				case SipHdrTypeRetryAfterSec:
				case SipHdrTypeRetryAfterAny:
				{

					if (g->pRetryAfterHdr == SIP_NULL)
					{
						*err = E_INV_PARAM;
						return SipFail;
					}
					if (ndx !=0)
					{
						*err = E_INV_INDEX;
						return SipFail;
					}
					
                                        /* Get Retry After - single instance header */
					if(mode==SipModeNew)
                                            STRCAT ( pEndBuff,out,"Retry-After: ");

					if (g->pRetryAfterHdr->dType == SipExpSeconds)
					{
						SIP_S8bit dSec[20];
						/* its seconds */
						HSS_SNPRINTF\
							((char *)dSec, 20, "%u", g->pRetryAfterHdr->u.dSec);
						dSec[ 20-1]='\0';
						STRCAT ( pEndBuff,out,dSec);
					}
					else
					if (SipFail==sip_formDateStruct\
						(pEndBuff,&out, g->pRetryAfterHdr->u.pDate, err))
					{
						return SipFail;
					}

					/* get Comment */
					if (g->pRetryAfterHdr->pComment !=SIP_NULL)
					{
						STRCAT ( pEndBuff,out," ");
						STRCAT ( pEndBuff,out, g->pRetryAfterHdr->pComment);
					}


					if (SipFail==sip_formSipParamList\
							(pEndBuff,&out, &(g->pRetryAfterHdr->slParams),\
							 (SIP_S8bit *) ";", 1, err))
					{
						return SipFail;
					}
					STRCAT ( pEndBuff,out,CRLF);
					break;
			} /* retry After */


			/*=============================================================*/
			case SipHdrTypeAcceptEncoding:
				/* process  accept Encoding hdrs */
				{
					SipAcceptEncodingHeader *ae;
					SIP_U32bit	tsize=0;
					SIP_U32bit	tindex=0;
					SipBool	dResult;

					res = sip_listGetAt \
						(&(g->slAcceptEncoding), ndx, (SIP_Pvoid *) &ae, err);
					if ( res == SipFail)
					{
						return SipFail;
					}
					if(mode==SipModeNone) {}
					else if(mode==SipModeNew)
						STRCAT ( pEndBuff,out, "Accept-Encoding: ");
					else
						STRCAT ( pEndBuff,out, ",");

					STRCAT ( pEndBuff,out, ae->pCoding);
					if(ae->pQValue != SIP_NULL)
					{
						STRCAT(pEndBuff,out,";q=");
						STRCAT ( pEndBuff,out, ae->pQValue);
					}

					dResult=sip_listSizeOf( &(ae->slParam), &tsize, err);
					if (dResult==SipSuccess)
					{
						tindex = 0;
						while (tindex < tsize)
						{
							SipNameValuePair *c;
							dResult= sip_listGetAt ( &(ae->slParam), tindex, \
												(SIP_Pvoid *)&c, err);
							if (dResult==SipSuccess)
							{
								STRCAT (pEndBuff,out,";");
								STRCAT (pEndBuff,out, c->pName);
								if (c->pValue!=SIP_NULL)
								{
									STRCAT (pEndBuff,out,"=");
									STRCAT (pEndBuff,out, c->pValue);
								}
								tindex++;
							}
						}
					}

					STRCAT ( pEndBuff,out, CRLF);
					break;
				}

			/*=============================================================*/
			case SipHdrTypeAcceptLanguage:
				/* process  accept Lang hdrs */
				{
					SipAcceptLangHeader *al;
					SIP_U32bit	tsize=0,tindex=0;
					SipBool	dResult=SipFail;

					res = sip_listGetAt \
							(&(g->slAcceptLang), ndx, (SIP_Pvoid *) &al, err);
					if ( res == SipFail)
					{
						return SipFail;
					}
					if(mode==SipModeNone) {}
					else if(mode==SipModeNew)
							STRCAT ( pEndBuff,out, "Accept-Language: ");
						else
							STRCAT ( pEndBuff,out, ",");

					STRCAT ( pEndBuff,out, al->pLangRange);
					if (al->pQValue != SIP_NULL)
					{
						STRCAT(pEndBuff,out, ";q=");
						STRCAT ( pEndBuff,out, al->pQValue);
					}

					dResult=sip_listSizeOf( &(al->slParam), &tsize, err);
					if (dResult==SipSuccess)
					{
						tindex = 0;
						while (tindex < tsize)
						{
							SipNameValuePair *c;
							dResult= sip_listGetAt ( &(al->slParam), tindex, \
												(SIP_Pvoid *)&c, err);
							if (dResult==SipSuccess)
							{
								STRCAT (pEndBuff,out,";");
								STRCAT (pEndBuff,out, c->pName);
								if (c->pValue!=SIP_NULL)
								{
									STRCAT (pEndBuff,out,"=");
									STRCAT (pEndBuff,out, c->pValue);
								}
								tindex++;
							}
						}
					}
					STRCAT ( pEndBuff,out, CRLF);
					break;
				}

			/*=============================================================*/
			case SipHdrTypeCallId:
			/* process  Call ID  hdrs */
			{

				if (g->pCallidHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}

				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if(mode==SipModeNew)
				{
					if (form==SipFormShort)
						STRCAT ( pEndBuff,out, "i: ");
					else
						STRCAT ( pEndBuff,out, "Call-ID: ");
				} /* Its single Instance Header so JoinMode is wrong */

				STRCAT ( pEndBuff,out, g->pCallidHdr->pValue);
				STRCAT ( pEndBuff,out, CRLF);
				break;
			} /* of call id */


			/*=============================================================*/
			case SipHdrTypeContactNormal:
			case SipHdrTypeContactWildCard:
			case SipHdrTypeContactAny:
			{
				/* process  contact hdrs */
				SipContactParam *cp;
				SipExpiresStruct *pExpire;
				SIP_U32bit cpsize,cpiter;
				SIP_S8bit dSec[11];
				SipContactHeader *ch;
				SIP_S8bit addBracket=0;

				res = sip_listGetAt\
						(&(g->slContactHdr), ndx, (SIP_Pvoid *) &ch, err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
				{
					if (form==SipFormShort)
						STRCAT ( pEndBuff,out, "m: ");
					else
						STRCAT ( pEndBuff,out, "Contact: ");
				}
				else
					STRCAT ( pEndBuff,out, ",");

				/* if its a *, then all others need not be accessed */
				if (ch->dType == SipContactWildCard )
				{
					STRCAT ( pEndBuff,out, "*");
					STRCAT ( pEndBuff,out,CRLF);
					break;
				}
				/* we come here if this contact Header is not a "*" */
				if (ch->pDispName !=SIP_NULL)
				{
					if (strcmp(ch->pDispName,"")!=0)
					{
							if (ch->pDispName[0]!=' ')
								STRCAT ( pEndBuff,out, ch->pDispName);
							else
								STRCAT ( pEndBuff,out, &(ch->pDispName[1]));
					}
					addBracket = 1;
				}
				if (ch->pAddrSpec !=SIP_NULL)
				{
					SipBool paramPresent=SipFail;

					if (!addBracket)
					{
						paramPresent=sip_areThereParamsinAddrSpec\
															(ch->pAddrSpec,err);
					    if (paramPresent==SipSuccess) addBracket=1;
					}

					if (addBracket)
					{
						if (ch->pDispName!=SIP_NULL)
						{
							if(ch->pDispName[0]=='\0')
								STRCAT ( pEndBuff,out,"<");
							else
								STRCAT ( pEndBuff,out," <");
						}
						else
							STRCAT ( pEndBuff,out,"<");
					}

					/* if dispname was there add <>s around */
					if (SipFail==sip_formAddrSpec\
								(pEndBuff,&out, ch->pAddrSpec, err ))
					{
						return SipFail;
					}

					if (addBracket) STRCAT ( pEndBuff,out,">");
					/* if dispname was there add <>s around */
				} /* of if addrspec */

				/* Now fetch ContactParam */
				sip_listSizeOf( &(ch->slContactParam), &cpsize, err);
				cpiter = 0;
				while (cpiter < cpsize)
				{
					sip_listGetAt (&(ch->slContactParam), cpiter, \
								(SIP_Pvoid *) &cp, err);
					/* Now find out Type of contact Param */
					switch (cp->dType)
					{
						case SipCParamQvalue:
							STRCAT ( pEndBuff,out, ";");
							STRCAT ( pEndBuff,out, "q=");
							STRCAT ( pEndBuff,out, cp->u.pQValue);
							break;
						case SipCParamExpires:
							STRCAT ( pEndBuff,out, ";");
							STRCAT ( pEndBuff,out, "expires=");
							pExpire = cp->u.pExpire;
							if (pExpire->dType== SipExpSeconds)
							{
								HSS_SNPRINTF\
									((char*)dSec,11, "%u", pExpire->u.dSec);
								dSec[11-1]='\0';
								STRCAT(pEndBuff,out, dSec);
								break;
							}
							else /* Expiry is DateStructFormat */
							{
								STRCAT ( pEndBuff,out, "\"");
								if (SipFail==sip_formDateStruct\
										(pEndBuff,&out, pExpire->u.pDate, err))
								{
									return SipFail;
								}
								STRCAT ( pEndBuff,out, "\"");
								break;
							}
						case SipCParamExtension:
							STRCAT ( pEndBuff,out, ";");
							STRCAT ( pEndBuff,out,cp->u.pExtensionAttr);
							break;
						case SipCParamFeatureParam:	
						 {
							SIP_U32bit dCount=0 ,dIter=0 ;
							SIP_S8bit *pValue = SIP_NULL ;

							STRCAT ( pEndBuff,out, ";");
							STRCAT ( pEndBuff,out,cp->u.pParam->pName);
							STRCAT ( pEndBuff,out,"=<");
							sip_listSizeOf( &(cp->u.pParam->slValue),\
											&dCount, err);
						/* iterate through SipList of Param and form */
						/* comma separated entries */
							if(dCount>1)
							{
									dIter = 0;
									while (dIter < dCount)
									{
											sip_listGetAt (&(cp->u.pParam->slValue),\
															dIter,(SIP_Pvoid *) &pValue, err);
											STRCAT ( pEndBuff,out,pValue);
											if(dIter<dCount-1)
													STRCAT ( pEndBuff,out,",");
											dIter++;
									}
							}
							else if (dCount==1)
							{
									sip_listGetAt (&(cp->u.pParam->slValue), 0,
													(SIP_Pvoid *) &pValue, err);
									STRCAT ( pEndBuff,out,pValue);
							}
							STRCAT ( pEndBuff,out,">");
							break;
						}
						case SipCParamAny:
							break;
					} /* of switch */
						cpiter++;
				} /* while (cpiter < cpsize)*/


				STRCAT ( pEndBuff,out,CRLF);
				break;
			} /* of if contact hdr present */

			/*=============================================================*/
			case SipHdrTypeCseq:
			{
			/* process CSeq Header */
				SIP_S8bit cs[11];
				if (g->pCseqHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				HSS_SNPRINTF((char*)cs, 11, "%u",g->pCseqHdr->dSeqNum);
				cs[ 11-1]='\0';
				if(mode==SipModeNew)
						STRCAT ( pEndBuff,out, "CSeq: ");

				STRCAT ( pEndBuff,out,cs);
				STRCAT ( pEndBuff,out," ");
				STRCAT(pEndBuff,out,g->pCseqHdr->pMethod);
				STRCAT ( pEndBuff,out,CRLF);
				break;
			}

			/*=============================================================*/
			case SipHdrTypeDate:
			{
				/* process SipDate Header */
				if (g->pDateHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if(mode==SipModeNew)
						STRCAT ( pEndBuff,out,"Date: ");
				/* Because Date is Single Instance Header
					 so ModeJoin and ModeNone are not valid */	
				if (SipFail==sip_formDateStruct \
						(pEndBuff,&out, (SipDateStruct *)(g->pDateHdr),err))
				{
					return SipFail;
				}
				STRCAT ( pEndBuff,out,CRLF);
				break;
			}

			/*=============================================================*/
			case SipHdrTypeEncryption:
			{
				/* process Encryption Header */
				if (g->pEncryptionHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}

				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}

				if(mode==SipModeNew)
						STRCAT ( pEndBuff,out,"Encryption: ");
				/* Since it is Single Instance Header so
				   SipModeJoin and SipModeNone are not valid 
					 and nothing has to be done for those cases */		

				STRCAT ( pEndBuff,out, g->pEncryptionHdr->pScheme);
				/* commented for Canonical form */
				/* STRCAT ( pEndBuff,out, " "); */
				if (SipFail==sip_formSipParamList\
						(pEndBuff,&out,&(g->pEncryptionHdr->slParam),\
						(SIP_S8bit *) ",",0,err))
				{
					return SipFail;
				}
				STRCAT ( pEndBuff,out, CRLF);
				break;
			}

			/*=============================================================*/
			case SipHdrTypeExpiresDate:
			case SipHdrTypeExpiresSec:
			case SipHdrTypeExpiresAny:
			{
				if (g->pExpiresHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}

				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}		/* Process Expires Header */
				if(mode==SipModeNew)
					STRCAT ( pEndBuff,out,"Expires: ");

		   /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  

				if (g->pExpiresHdr->dType == SipExpSeconds)
				{
					SIP_S8bit dSec[20];
					HSS_SNPRINTF((char*)dSec,20,"%u",g->pExpiresHdr->u.dSec);
					dSec[20-1]='\0';
					STRCAT( pEndBuff,out,dSec);
				}
				else
				if (SipFail==sip_formDateStruct\
							(pEndBuff,&out, g->pExpiresHdr->u.pDate,err))
				{
					return SipFail;
				}
				STRCAT ( pEndBuff,out,CRLF);
				break;
			}
#ifdef SIP_SESSIONTIMER
     case SipHdrTypeMinSE:
      {
				SIP_S8bit dSec[20];
				SIP_U32bit dSize=0,dIndex=0;
				if (g->pMinSEHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}

				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
					/* because this is single instance header */	
				if (mode==SipModeNew) 
					STRCAT(pEndBuff,out,"Min-SE: ");
					  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  

				HSS_SNPRINTF((SIP_S8bit*)dSec,20,"%u",g->pMinSEHdr->dSec);
				dSec[20-1]='\0';
				STRCAT( pEndBuff,out,dSec);
				{
					sip_listSizeOf( &(g->pMinSEHdr->slNameValue), &dSize, err);
					while (dIndex < dSize)
					{
						SipNameValuePair *c;
						sip_listGetAt ( &(g->pMinSEHdr->slNameValue), dIndex, \
							(SIP_Pvoid *)&c, err);

						STRCAT (pEndBuff,out, ";");
						STRCAT (pEndBuff,out, c->pName);
						if ( c->pValue != SIP_NULL )
						{
							STRCAT (pEndBuff,out,"=");
							STRCAT (pEndBuff,out, c->pValue);
						}
						dIndex++;
					}
					STRCAT (pEndBuff,out,CRLF);
				}
				break;
             }


			case SipHdrTypeSessionExpires:
			{
				SIP_S8bit dSec[20];
				SIP_U32bit dSize=0,dIndex=0;

				if (g->pSessionExpiresHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}

				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}		/* Process SessionExpires Header */
				if(mode==SipModeNew)
				{
					if (form==SipFormShort)
						STRCAT ( pEndBuff,out,"x: ");
					else
						STRCAT ( pEndBuff,out,"Session-Expires: ");
				}
				  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  

				HSS_SNPRINTF((char*)dSec,20,"%u",\
					g->pSessionExpiresHdr->dSec);
				dSec[20-1]='\0';
				STRCAT( pEndBuff,out,dSec);
				{
					sip_listSizeOf( &(g->pSessionExpiresHdr->slNameValue),\
						&dSize, err);
					while (dIndex < dSize)
					{
						SipNameValuePair *c;
						sip_listGetAt ( &(g->pSessionExpiresHdr->slNameValue), \
							dIndex,(SIP_Pvoid *)&c, err);

						STRCAT (pEndBuff,out, ";");
						STRCAT (pEndBuff,out, c->pName);
						if ( c->pValue != SIP_NULL )
						{
							STRCAT (pEndBuff,out,"=");
							STRCAT (pEndBuff,out, c->pValue);
						}
						dIndex++;
					}
					STRCAT (pEndBuff,out,CRLF);
				}
				break;
			}
#endif
			/*=============================================================*/
			case SipHdrTypeReplyTo:
			{
			/* Process From Header */
				if (g->pReplyToHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if (g->pReplyToHdr !=SIP_NULL)
				{
					SipReplyToHeader *sf;
					SIP_S8bit addBracket=0;
					sf=g->pReplyToHdr;
					if(mode==SipModeNew) 
						STRCAT ( pEndBuff,out,"Reply-To: ");
					 /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
					 

					if (sf->pDispName !=SIP_NULL)
					{
						STRCAT ( pEndBuff,out, sf->pDispName);
						if (strcmp(sf->pDispName,""))
							STRCAT ( pEndBuff,out," ");
						addBracket=1;
					}

					if (sf->pAddrSpec != SIP_NULL)
					{
						SipBool paramPresent=SipFail;

						if (!addBracket)
						{
							paramPresent=sip_areThereParamsinAddrSpec\
															(sf->pAddrSpec,err);
						    if (paramPresent==SipSuccess)
							{
								addBracket=1;
							}
                        }

						if (addBracket) STRCAT ( pEndBuff,out,"<");
						if (SipFail==sip_formAddrSpec\
									(pEndBuff,&out, sf->pAddrSpec, err))
						{
							return SipFail;
						}

						if (addBracket) STRCAT ( pEndBuff,out,">");
					}
					/* Now get generic Param */
					{
						SIP_U32bit dSize=0,dIndex=0;
						sip_listSizeOf( &(sf->slParams), &dSize, err);
						while (dIndex < dSize)
						{
							SipNameValuePair *c;
							sip_listGetAt ( &(sf->slParams), dIndex, \
									(SIP_Pvoid *)&c, err);
							STRCAT (pEndBuff,out,";");
							STRCAT (pEndBuff,out, c->pName);
							if(c->pValue)
							{
								STRCAT (pEndBuff,out,"=");
								STRCAT (pEndBuff,out, c->pValue);
							}
							dIndex++;
						}
					}
					STRCAT ( pEndBuff,out,CRLF);
				} /* of ReplyTo Header */
				break;
			}

			/*=============================================================*/
			case SipHdrTypeFrom:
			{
			/* Process From Header */
				if (g->pFromHeader == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if (g->pFromHeader !=SIP_NULL)
				{
					SipFromHeader *sf;
					SIP_U32bit size,iter;
					SIP_S8bit addBracket=0;
					sf=g->pFromHeader;
					if(mode==SipModeNew)
					{
						if (form==SipFormShort)
							STRCAT ( pEndBuff,out,"f: ");
						else
							STRCAT ( pEndBuff,out,"From: ");
					}

					if (sf->pDispName !=SIP_NULL)
					{
						STRCAT ( pEndBuff,out, sf->pDispName);
						if (strcmp(sf->pDispName,""))
							STRCAT ( pEndBuff,out," ");
						addBracket=1;
					}

					if (sf->pAddrSpec != SIP_NULL)
					{
						SipBool paramPresent=SipFail;

						if (!addBracket)
						{
							paramPresent=sip_areThereParamsinAddrSpec\
															(sf->pAddrSpec,err);
						    if (paramPresent==SipSuccess)
							{
								addBracket=1;
							}
                        }

						if (addBracket) STRCAT ( pEndBuff,out,"<");
						if (SipFail==sip_formAddrSpec\
									(pEndBuff,&out, sf->pAddrSpec, err))
						{
							return SipFail;
						}

						if (addBracket) STRCAT ( pEndBuff,out,">");
					}
					/* Now get Addr Param */
					sip_listSizeOf( &(sf->slTag), &size, err);
					iter=0;
					while  (iter < size)
					{
						SIP_S8bit *ap;
						sip_listGetAt \
							(&(sf->slTag), iter, (SIP_Pvoid *) &ap, err);
						STRCAT ( pEndBuff,out,";tag=");
						STRCAT ( pEndBuff,out, ap);
						iter++;
					}

					/* Now get extension Param */
					if (SipFail==sip_formSipParamList\
						(pEndBuff,&out,&(sf->slParam),(SIP_S8bit *) ";",1,err))
					{
						return SipFail;
					}
					STRCAT ( pEndBuff,out,CRLF);
				} /* of From Header */
				break;
			}

			/*=============================================================*/
			case SipHdrTypeTo:
			{
			/* Process To Header */
				if (g->pToHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if (g->pToHdr !=SIP_NULL)
				{
					SipToHeader *st;
					SIP_S8bit addBracket=0;
					SIP_U32bit size,iter;
					st=g->pToHdr;
					if(mode==SipModeNew)
					{
						if (form==SipFormShort)
							STRCAT ( pEndBuff,out,"t: ");
						else
							STRCAT ( pEndBuff,out,"To: ");
					}
					  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
					 

					if (st->pDispName !=SIP_NULL)
					{
						STRCAT ( pEndBuff,out, st->pDispName);
						if (strcmp(st->pDispName,""))
							STRCAT ( pEndBuff,out," ");
						addBracket=1;
					}
					if (st->pAddrSpec !=SIP_NULL)
					{
						SipBool paramPresent=SipFail;

						if (!addBracket)
						{
							paramPresent=sip_areThereParamsinAddrSpec\
															(st->pAddrSpec,err);
						    if (paramPresent==SipSuccess) addBracket=1;
                        }

						if (addBracket) STRCAT ( pEndBuff,out,"<");
						if (SipFail==sip_formAddrSpec\
										(pEndBuff,&out, st->pAddrSpec, err))
						{
							return SipFail;
						}

						if (addBracket) STRCAT ( pEndBuff,out,">");
					}
					/* Now get Addr Param */
					sip_listSizeOf( &(st->slTag), &size, err);
					iter=0;
					while  (iter < size)
					{
						SIP_S8bit *ap;
						sip_listGetAt (&(st->slTag), iter,\
													(SIP_Pvoid *) &ap, err);
						STRCAT ( pEndBuff,out,";tag=");
						STRCAT ( pEndBuff,out, ap);
						iter++;
					}

					/* Now get extension Param */
					if (SipFail==sip_formSipParamList\
						(pEndBuff,&out,&(st->slParam),(SIP_S8bit *) ";",1,err))
					{
						return SipFail;
					}
					STRCAT ( pEndBuff,out,CRLF);
				} /* of To Header */
				break;
			}

			/*=============================================================*/
			case SipHdrTypeRecordRoute:
			{
				SipRecordRouteHeader *rr;
				/* Process Record Route Header */
				/* Record route is a sipList - but defn is 1#Name-Addr
				- should have only been Record Route */

				res = sip_listGetAt\
					(&(g->slRecordRouteHdr), ndx, (SIP_Pvoid *) &rr, err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out,"Record-Route: ");
				else
					STRCAT ( pEndBuff,out, ",");

				if (rr->pDispName !=SIP_NULL)
				{
					STRCAT ( pEndBuff,out,rr->pDispName);
					if (strcmp(rr->pDispName,""))
						STRCAT ( pEndBuff,out," ");
				}

				STRCAT ( pEndBuff,out,"<");

				if (SipFail==sip_formAddrSpec\
						(pEndBuff,&out, rr->pAddrSpec, err))
				{
					return SipFail;
				}

				STRCAT ( pEndBuff,out,">");
				if (SipFail==sip_formSipParamList(pEndBuff,&out,\
						&(rr->slParams), (SIP_S8bit *) ";", 1, err))
				{
					return SipFail;
				}
				STRCAT ( pEndBuff,out,CRLF);
				break;

			} /* of record route */

			/*=============================================================*/
#ifdef SIP_3GPP
			case SipHdrTypeServiceRoute :
			 {
					 SipServiceRouteHeader *pService = SIP_NULL;

					 res = sip_listGetAt\
							 (&(g->slServiceRouteHdr), ndx, (SIP_Pvoid *) &pService, err);
					 if (res == SipFail)
					 {
							 return SipFail;
					 }
					 if(mode==SipModeNone) {}
					 else if(mode==SipModeNew)
							 STRCAT ( pEndBuff,out,"Service-Route: ");
					 else
							 STRCAT ( pEndBuff,out, ",");

					 if (pService->pDispName !=SIP_NULL)
					 {
							 STRCAT ( pEndBuff,out,pService->pDispName);
							 if (strcmp(pService->pDispName,""))
									 STRCAT ( pEndBuff,out," ");
					 }

					 STRCAT ( pEndBuff,out,"<");

					 if (SipFail==sip_formAddrSpec\
									 (pEndBuff,&out, pService->pAddrSpec, err))
					 {
							 return SipFail;
					 }

					 STRCAT ( pEndBuff,out,">");
					 if (SipFail==sip_formSipParamList(pEndBuff,&out,\
											 &(pService->slParams), (SIP_S8bit *) ";", 1, err))
					 {
							 return SipFail;
					 }
					 STRCAT ( pEndBuff,out,CRLF);
					 break;
			 } /* Service-Route */

			case SipHdrTypePath:
			{
				SipPathHeader *pPath;
				res = sip_listGetAt\
					(&(g->slPathHdr), ndx, (SIP_Pvoid *) &pPath, err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out,"Path: ");
				else
					STRCAT ( pEndBuff,out, ",");

				if (pPath->pDispName !=SIP_NULL)
				{
					STRCAT ( pEndBuff,out,pPath->pDispName);
					if (strcmp(pPath->pDispName,""))
						STRCAT ( pEndBuff,out," ");
				}

				STRCAT ( pEndBuff,out,"<");

				if (SipFail==sip_formAddrSpec\
						(pEndBuff,&out, pPath->pAddrSpec, err))
				{
					return SipFail;
				}

				STRCAT ( pEndBuff,out,">");
				if (SipFail==sip_formSipParamList(pEndBuff,&out,\
						&(pPath->slParams), (SIP_S8bit *) ";", 1, err))
				{
					return SipFail;
				}
				STRCAT ( pEndBuff,out,CRLF);
				break;

			} /* Of Path */

           
           /*=============================================================*/
            case SipHdrTypePanInfo:
			{
				SipPanInfoHeader *pf;
				SIP_U32bit dSize=0,dIndex=0;
				if (g->pPanInfoHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				pf = g->pPanInfoHdr;
				if (ndx!=0)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if(mode==SipModeNew)
					STRCAT( pEndBuff,out,"P-Access-Network-Info: ");
				if(pf->pAccessType)
				{
					STRCAT( pEndBuff,out,pf->pAccessType);
				}
				sip_listSizeOf( &(pf->slParams),&dSize,err);
				while (dIndex < dSize)
				{
					SipNameValuePair *c;
					sip_listGetAt ( &(pf->slParams), dIndex, \
					(SIP_Pvoid *)&c, err);
					STRCAT(pEndBuff,out,";");
					STRCAT(pEndBuff,out,c->pName);
						if(c->pValue)
						{
							STRCAT(pEndBuff,out,"=");
							STRCAT(pEndBuff,out,c->pValue);
					    }
					   dIndex++;
				 }
			STRCAT ( pEndBuff,out, CRLF);
			break;

		}	/* of P-Access-Network-Info*/
				
          /*==========================================================*/
			
			case SipHdrTypePcVector:
			{ 
                SIP_U32bit dSize=0,dIndex=0;   
				if (g->pPcVectorHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if(mode==SipModeNew)
					STRCAT( pEndBuff,out,"P-Charging-Vector: ");
				if(g->pPcVectorHdr->pAccessType)
				{
					STRCAT( pEndBuff,out,g->pPcVectorHdr->pAccessType);
				}
				if(g->pPcVectorHdr->pAccessValue)
				{
					STRCAT( pEndBuff,out,"=");
					STRCAT( pEndBuff,out,g->pPcVectorHdr->pAccessValue);
				}
               		sip_listSizeOf( &(g->pPcVectorHdr->slParams),&dSize,err);
				while (dIndex < dSize)
				{
					SipNameValuePair *c;
					sip_listGetAt ( &(g->pPcVectorHdr->slParams), dIndex, \
					(SIP_Pvoid *)&c, err);
					STRCAT(pEndBuff,out,";");
					STRCAT (pEndBuff,out,c->pName);
					if(c->pValue)
					{
						STRCAT (pEndBuff,out,"=");
						STRCAT (pEndBuff,out, c->pValue);
					}
					dIndex++;
				}
			
			STRCAT ( pEndBuff,out, CRLF);
			break;
			
			
		}			/*of P-Charging-Vector*/
   
            
			
#endif

			/*=============================================================*/

			case SipHdrTypeTimestamp:
			/* Process Time Stamp Header */
			{
				if (g->pTimeStampHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if(mode==SipModeNew)
							STRCAT ( pEndBuff,out, "Timestamp: ");
				  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
		
				if (g->pTimeStampHdr->pTime != SIP_NULL)
				{
					STRCAT ( pEndBuff,out,g->pTimeStampHdr->pTime);
				}
				if (g->pTimeStampHdr->delay !=SIP_NULL)
				{
					STRCAT ( pEndBuff,out," ");
					STRCAT ( pEndBuff,out,g->pTimeStampHdr->delay);
				}
				STRCAT ( pEndBuff,out,CRLF);
				break;
			} /* of Time Stamp */


			/*=============================================================*/
			case SipHdrTypeVia:
			/* Process Via Header */
			{
				SipViaHeader *vh;

				res = sip_listGetAt\
							(&(g->slViaHdr), ndx, (SIP_Pvoid *) &vh, err);
				if (res == SipFail )
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
				{
					if (form==SipFormShort)
						STRCAT ( pEndBuff,out,"v: ");
					else
						STRCAT ( pEndBuff,out,"Via: ");
				}
				else
					STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, vh->pSentProtocol);
				STRCAT ( pEndBuff,out, " ");
				STRCAT ( pEndBuff,out, vh->pSentBy);
				/* Now traverse list of Via Params */
				if (SipFail==sip_formSipParamList\
						(pEndBuff,&out,&(vh->slParam),(SIP_S8bit *) ";",1,err))
				{
					return SipFail;
				}

				/* Now get Comment */
				if (vh->pComment !=SIP_NULL)
				{
					STRCAT ( pEndBuff,out, " ");
					STRCAT ( pEndBuff,out, vh->pComment);
				}
				STRCAT ( pEndBuff,out,CRLF);
				break;
			} /* of Via Header */


#ifdef SIP_PRIVACY
			case SipHdrTypePAssertId:
			/* Process PAssertId Header */
			{
				SipPAssertIdHeader *pPAssertId =SIP_NULL ;

				res = sip_listGetAt (&(g->slPAssertIdHdr), ndx, \
							(SIP_Pvoid *) &pPAssertId, err);
				if (res == SipFail )
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
				{
					if (form==SipFormFull)
						STRCAT ( pEndBuff,out,"P-Asserted-Identity:");
				}
				else
					STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, pPAssertId->pDispName);
				STRCAT ( pEndBuff,out,"<");

				if (SipFail==sip_formAddrSpec\
						(pEndBuff,&out, pPAssertId->pAddrSpec, err))
				{
					return SipFail;
				}

				STRCAT ( pEndBuff,out,">");
				STRCAT ( pEndBuff,out,CRLF);
				break;
            }
			case SipHdrTypePPreferredId:
			/* Process P Header */
			{
				SipPPreferredIdHeader *pPPreferredId =SIP_NULL ;

				res = sip_listGetAt (&(g->slPPreferredIdHdr), ndx, \
							(SIP_Pvoid *) &pPPreferredId, err);
				if (res == SipFail )
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
				{
					if (form==SipFormFull)
						STRCAT ( pEndBuff,out,"P-Preferred-Identity:");
				}
				else
					STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, pPPreferredId->pDispName);
				STRCAT ( pEndBuff,out,"<");

				if (SipFail==sip_formAddrSpec\
						(pEndBuff,&out, pPPreferredId->pAddrSpec, err))
				{
					return SipFail;
				}

				STRCAT ( pEndBuff,out,">");
				STRCAT ( pEndBuff,out,CRLF);
				break;
            }
#endif /* # ifdef SIP_PRIVACY */

			case SipHdrTypeContentLength:
			{
			/* Process Content-Length Header */
				if (g->pContentLengthHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if (g->pContentLengthHdr !=SIP_NULL)
				{
					SIP_S8bit clen[20];
					if (mode==SipModeNew)
					{
						if (form==SipFormShort)
								STRCAT ( pEndBuff,out, "l: ");
							else
								STRCAT ( pEndBuff,out, "Content-Length: ");
					}
					  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  

					HSS_SNPRINTF((char *)clen, 20, "%u",\
											g->pContentLengthHdr->dLength);
					clen[ 20-1]='\0';
					STRCAT ( pEndBuff,out,clen);
					STRCAT ( pEndBuff,out,CRLF);
				}
				break;
			}
			/*=============================================================*/
			case  SipHdrTypeContentType:
			{
				SIP_U32bit count,iterator;
				SipParam *param;
				SIP_S8bit *pValue;
				/* Process Content-Type Header */
				if (g->pContentTypeHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}

				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if(mode==SipModeNew)
				{
					if (form==SipFormShort)
						STRCAT ( pEndBuff,out, "c: ");
					else
						STRCAT ( pEndBuff,out, "Content-Type: ");
				}
				  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  

				if (g->pContentTypeHdr->pMediaType !=SIP_NULL)
				{
					STRCAT ( pEndBuff,out, g->pContentTypeHdr->pMediaType);
				}

				if(sip_listSizeOf(&(g->pContentTypeHdr->slParams),&count,err)\
														==SipFail)
					return SipFail;
				iterator = 0;
				while(iterator<count)
				{
					SIP_U32bit valCount, i=0;
					if(sip_listGetAt(&(g->pContentTypeHdr->slParams),\
								iterator,(SIP_Pvoid *) &param,err)==SipFail)
					{
						return SipFail;
					}
					STRCAT ( pEndBuff,out,";");
					STRCAT ( pEndBuff,out,param->pName);
					STRCAT ( pEndBuff,out,"=");
					if (sip_listSizeOf(&(param->slValue),&valCount,err)\
																	== SipFail)
					{
						return SipFail;
					}
					while(i<valCount)
					{
						if(sip_listGetAt(&(param->slValue),i,\
										(SIP_Pvoid *) &pValue,err)==SipFail)
						{
							return SipFail;
						}
						STRCAT ( pEndBuff,out,pValue);
						if (i < (valCount-1))
							STRCAT ( pEndBuff,out,",");
						i++;
					}
					iterator++;
				}
				STRCAT ( pEndBuff,out,CRLF);
				break;
			}

			/*=============================================================*/
			case SipHdrTypeContentEncoding:
			{
			/* Process Content-Encoding Header */
				SipContentEncodingHeader *ce;

					res = sip_listGetAt (&(g->slContentEncodingHdr), ndx,\
									(SIP_Pvoid *) &ce, err);
					if (res == SipFail)
					{
						return SipFail;
					}
					if(mode==SipModeNone) {}
					else if(mode==SipModeNew)
					{
						if (form==SipFormShort)
							STRCAT ( pEndBuff,out, "e: ");
						else
							STRCAT ( pEndBuff,out, "Content-Encoding: ");
					}
					else
						STRCAT ( pEndBuff,out, ",");

					STRCAT ( pEndBuff,out, ce->pEncoding);
					STRCAT ( pEndBuff,out,CRLF);
				break;
			} /* of content Encoding */
			/*=============================================================*/

			case SipHdrTypeSupported:
			{
				SipSupportedHeader *rr;
				/* Process Supported Header */
				res = sip_listGetAt (&(g->slSupportedHdr), ndx,\
													(SIP_Pvoid *) &rr, err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
				{
					if (form==SipFormShort)
							STRCAT ( pEndBuff,out, "k: ");
						else
					STRCAT ( pEndBuff,out,"Supported: ");
				}
				else
					STRCAT ( pEndBuff,out, ",");

				if (rr->pOption !=SIP_NULL) STRCAT ( pEndBuff,out,rr->pOption);
				STRCAT ( pEndBuff,out,CRLF);
				break;
			} /* of supported */
			/*=============================================================*/
			case SipHdrTypeMimeVersion:
			{
			/* Process Mime-Version Header */
				if (g->pMimeVersionHdr == SIP_NULL)
				{
					*err = E_INV_PARAM;
					return SipFail;
				}
				if (ndx !=0)
				{
					*err = E_INV_INDEX;
					return SipFail;
				}
				if (g->pMimeVersionHdr !=SIP_NULL)
				{
					if(mode==SipModeNew)
						STRCAT ( pEndBuff,out, "Mime-Version: ");
					   /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
		
					if(g->pMimeVersionHdr->pVersion != SIP_NULL)
						STRCAT ( pEndBuff,out,g->pMimeVersionHdr->pVersion);
					STRCAT ( pEndBuff,out,CRLF);
				}
				break;
			}

	/*=============================================================*/
			case SipHdrTypeAllow:
			/* Get Allow Header */
			{
				SipAllowHeader *ah;
				res = sip_listGetAt\
							( &(g->slAllowHdr), ndx, (SIP_Pvoid *) &ah, err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out, "Allow: ");
				else
					STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, ah->pMethod);
				STRCAT ( pEndBuff,out,CRLF);
				break;
			}

			/*=============================================================*/
			case SipHdrTypeUnknown:
			{
				/* Process Unknown Header - Just for convention,
					all Unknown headers are put into general
				Headers as they are accessible from both Request and Response */
				SipUnknownHeader *u;
				res = sip_listGetAt(&(g->slUnknownHdr), ndx, 
									(SIP_Pvoid *) &u, err);
				if (res == SipFail)
				{
					return SipFail;
				}

				if(mode==SipModeNone) 
				{
				}
				else if(mode==SipModeNew)
				{
					STRCAT ( pEndBuff,out, u->pName);
					STRCAT ( pEndBuff,out, ":");
				}
				else
				{
					STRCAT ( pEndBuff,out, ",");
				}

				STRCAT ( pEndBuff,out, u->pBody);
				STRCAT ( pEndBuff,out, CRLF);
				break;
			} /* if unknown pHeader exists */

			/*=============================================================*/

			case SipHdrTypeCallInfo:
			{
				/* Get CallInfo Header */
				SipCallInfoHeader  *cih;

				res = sip_listGetAt\
						(&(g->slCallInfoHdr), ndx, (SIP_Pvoid *) &cih,err);
				if (res == SipFail)
				{
					return SipFail;
				}

				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out, "Call-Info: ");
				else
					STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, cih->pUri);
				if (SipFail==sip_formSipParamList\
					(pEndBuff,&out, &(cih->slParam), (SIP_S8bit *) ";", 1, err))
				{
					return SipFail;
				}

				STRCAT ( pEndBuff,out,CRLF);
				break;
			}
			case SipHdrTypeContentDisposition:
			{
				/* Get ContentDisposition Header */
				SipContentDispositionHeader  *cdh;

				res = sip_listGetAt(&(g->slContentDispositionHdr), \
											ndx, (SIP_Pvoid *) &cdh,err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out, "Content-Disposition: ");
				else
					STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, cdh->pDispType);
				if (SipFail==sip_formSipParamList\
					(pEndBuff,&out, &(cdh->slParam), (SIP_S8bit *) ";", 1, err))
				{
					return SipFail;
				}

				STRCAT ( pEndBuff,out,CRLF);
				break;
			}

			case SipHdrTypeReason:
			{
				/* Get Reason Header */
				SipReasonHeader  *pReason;

				res = sip_listGetAt(&(g->slReasonHdr), \
						ndx, (SIP_Pvoid *) &pReason,err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out, "Reason: ");
				else
					STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, pReason->pDispType);
				if (SipFail==sip_formSipParamList(pEndBuff,&out, \
					&(pReason->slParam), (SIP_S8bit *) ";", 1, err))
				{
					return SipFail;
				}

				STRCAT ( pEndBuff,out,CRLF);
				break;
			}

			case SipHdrTypeContentLanguage:
			{
				/* Get ContentLanguage Header */
				SipContentLanguageHeader  *clh;

				res = sip_listGetAt\
					(&(g->slContentLanguageHdr), ndx, (SIP_Pvoid *) &clh,err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Content-Language: ");
				else
				STRCAT ( pEndBuff,out, ",");

				STRCAT ( pEndBuff,out, clh->pLangTag);
				STRCAT ( pEndBuff,out,CRLF);
				break;
			}
			case SipHdrTypeRequire:
			{
				/* Get Require Header */
				SipRequireHeader *sr;
				res = sip_listGetAt\
							(&(g->slRequireHdr), ndx, (SIP_Pvoid *) &sr, err);
				if (res == SipFail)
				{
					return SipFail;
				}

				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out,"Require: ");
				else
					STRCAT ( pEndBuff,out, ",");
				STRCAT ( pEndBuff,out, sr->pToken);
				STRCAT ( pEndBuff,out, CRLF);
				break;
			}

			case SipHdrTypeOrganization:
			{
				/*We do not need to check for an empty
				organization hdr since an empty Organization hdr
				is allowed as per bis09*/
				if(mode==SipModeNew)
					STRCAT ( pEndBuff,out, "Organization: ");
				  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
			
				if (g->pOrganizationHdr != SIP_NULL)
				{
					STRCAT ( pEndBuff,out, g->pOrganizationHdr->pOrganization);
				}

				STRCAT ( pEndBuff,out,CRLF);
				break;
			}

			case SipHdrTypeUserAgent:
			{
				/* Get User Agent Header */

				if (g->pUserAgentHdr == SIP_NULL)
				{
					*err =  E_INV_PARAM;
					return SipFail;
				}
				if(mode==SipModeNew)
					STRCAT ( pEndBuff,out,"User-Agent: ");
				  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
					
				STRCAT ( pEndBuff,out, g->pUserAgentHdr->pValue);
				STRCAT ( pEndBuff,out, CRLF);
				break;
			}
#ifdef SIP_IMPP
			case SipHdrTypeAllowEvents:
			/* Get Allow-Events Header */
			{
				SipAllowEventsHeader *pAllowEventsHdr;
				res = sip_listGetAt(&(g->slAllowEventsHdr), ndx, \
									(SIP_Pvoid *) &pAllowEventsHdr, err);
				if (res == SipFail)
				{
					return SipFail;
				}
				if(mode==SipModeNone) {}
				else if(mode==SipModeNew)
        {
          	if (form==SipFormShort)
							STRCAT ( pEndBuff,out, "u: ");
						else
              STRCAT ( pEndBuff,out,"Allow-Events: ");
        }
				else
					STRCAT ( pEndBuff,out, ",");

				if ( pAllowEventsHdr->pEventType != SIP_NULL )
					STRCAT ( pEndBuff,out, pAllowEventsHdr->pEventType);
				STRCAT ( pEndBuff,out,CRLF);
				break;
			}
#endif
#ifdef SIP_PRIVACY
	case SipHdrTypePrivacy:
		{
			SIP_U32bit dSize=0,dIndex=0,dFirst=0;
			if (mode==SipModeNew)
				STRCAT(pEndBuff,out,"Privacy: ");
				  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
			{
				sip_listSizeOf( &(g->pPrivacyHdr->slPrivacyValue), &dSize, err);
				while (dIndex < dSize)
				{
					SipParam *c;
					sip_listGetAt ( &(g->pPrivacyHdr->slPrivacyValue), dIndex, \
										(SIP_Pvoid *)&c, err);
					if ( (dFirst != 0))
					{
						STRCAT (pEndBuff,out, ";");
					}
					if(strcmp(c->pName,";") != 0)
					STRCAT (pEndBuff,out, c->pName);
					dIndex++;
					dFirst=1;
				}
			 }

			STRCAT(pEndBuff,out,CRLF);
			
		}
		break;
#endif
#ifdef SIP_3GPP
	case SipHdrTypePcfAddr:
		{
			SIP_U32bit dSize=0,dIndex=0,dFirst=0,valueSize=0;
            SipPcfAddrHeader *cfAddrh;
			
            if (mode==SipModeNew)
				STRCAT(pEndBuff,out,"P-Charging-Function-Addresses: ");
				  /* Since it is Single Instance Header so
           SipModeJoin and SipModeNone are not valid
           and nothing has to be done for those cases */  
            
            cfAddrh = g->pPcfAddrHdr;
            
            if(cfAddrh == SIP_NULL)
                return SipFail;

			{
				sip_listSizeOf( &(cfAddrh->slParams), &dSize, err);
				while (dIndex < dSize)
				{
					SipParam *c;
					sip_listGetAt ( &(cfAddrh->slParams), dIndex, \
										(SIP_Pvoid *)&c, err);
					if ( (dFirst != 0))
					{
						STRCAT (pEndBuff,out, ";");
					}
					if(strcmp(c->pName,";") != 0)
                      STRCAT (pEndBuff,out, c->pName);
                    
               		sip_listSizeOf( &(c->slValue), &valueSize, err);
		            if ( valueSize>=1)
            		{
			            SIP_S8bit *value;
            			SIP_U32bit valueIter=0;
            			STRCAT(pEndBuff,out,"=");
			            while(valueIter < valueSize)
            			{
			            	if(valueIter>0)
        					STRCAT(pEndBuff,out,",");
		            		sip_listGetAt (&(c->slValue), valueIter, \
					            (SIP_Pvoid *) &value, err);
            				STRCAT(pEndBuff,out,value);
			            	valueIter++;
            			}
            		}

					dIndex++;
					dFirst=1;
				}
			 }
            
			STRCAT(pEndBuff,out,CRLF);
			
		}
		break;
#endif
			
			default:
#ifdef SIP_DCS
				if(sip_dcs_formSingleGeneralHeader\
						(pEndBuff,dType, ndx,mode, form,g,&out,err)!=SipSuccess)
				{
						return SipFail;
				}
#endif
				break;
	} /* switch */

	if(pEndBuff)*ppOut = out;

	SIPDEBUGFN("Exiting from sip_formSingleGeneralHeader");
	return SipSuccess;
} /* of sip_formSingleGeneralHeader */


/*****************************************************************
** FUNCTION: sip_formSingleResponseHeader
**
**
** DESCRIPTION: Converts a SipRespHeader to Text
*****************************************************************/
SipBool sip_formSingleResponseHeader
#ifdef ANSI_PROTO
	(
	SIP_S8bit	*pEndBuff,
	 en_HeaderType dType,
	 SIP_U32bit ndx,
 	en_AdditionMode mode,
	 en_HeaderForm form,
	 SipRespHeader *s,
	 SIP_S8bit **ppOut,
	 SipError *err)
#else
	(pEndBuff,dType, ndx, mode, form,s, ppOut, err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	 en_HeaderForm form;
	SipRespHeader *s;
	 SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SipBool res;
	en_HeaderForm dummy;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into sip_formSingleResponseHeader");
	out = *ppOut;

	 dummy = form;

	switch (dType)
	{

	/*=============================================================*/
		case SipHdrTypeProxyAuthenticate:
		/* Get Proxy Authenticate Header */
		{
			SipGenericChallenge *gch;
			SipProxyAuthenticateHeader *pah;
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
					STRCAT ( pEndBuff,out, "Proxy-Authenticate: ");
				else
					STRCAT ( pEndBuff,out, ",");
			res = sip_listGetAt ( &(s->slProxyAuthenticateHdr),ndx,\
									(SIP_Pvoid *) &pah, err);
			if (res == SipFail)
			{
				return SipFail;
			}

			gch = pah->pChallenge;
			STRCAT ( pEndBuff,out, gch->pScheme);
			/* Now get slAuthorizationParam which is a sipList */
			if (SipFail==sip_formSipParamList(pEndBuff,&out,&(gch->slParam),\
				(SIP_S8bit *) ",",0,err))
			{
				return SipFail;
			}

			STRCAT ( pEndBuff,out,CRLF);
			break;
		} /* of Proxy Auth */


	/*=============================================================*/
		case SipHdrTypeServer:
		{
		/* Get Server Header */

			if (s->pServerHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Server: ");
			/* Single Instance Header */	
			STRCAT ( pEndBuff,out, s->pServerHdr->pValue);
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}


	/*=============================================================*/
		case SipHdrTypeUnsupported:
		{
		/* Get Unsupp Header */
			SipUnsupportedHeader *us;
			res = sip_listGetAt (&(s->slUnsupportedHdr), ndx,\
						(SIP_Pvoid *) &us, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Unsupported: ");
			else
				STRCAT ( pEndBuff,out, ",");
			STRCAT ( pEndBuff,out, us->pOption);
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

	/*=============================================================*/
	/* get Warning Header */
		case SipHdrTypeWarning:
		{
			SIP_S8bit dCodeNum[20];
			SipWarningHeader *wh;

			res = sip_listGetAt (&(s->slWarningHeader), ndx,\
								(SIP_Pvoid *) &wh, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
			STRCAT ( pEndBuff,out, "Warning: ");
			else
			STRCAT ( pEndBuff,out, ",");
			HSS_SNPRINTF((char *)dCodeNum,20, "%03d", wh->dCodeNum);
			dCodeNum[20-1]='\0';
			STRCAT ( pEndBuff,out, dCodeNum);
			STRCAT ( pEndBuff,out," ");
			STRCAT ( pEndBuff,out, wh->pAgent);
			STRCAT ( pEndBuff,out," ");
			STRCAT ( pEndBuff,out, wh->pText);
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}
	/*=============================================================*/
	/* Get Authorization Header */
		case SipHdrTypeAuthorization:
		{
			SipGenericCredential *gc;
			SipAuthorizationHeader *auth;

			res = sip_listGetAt (&(s->slAuthorizationHdr), ndx,\
			(SIP_Pvoid *) &auth, err);
			if (res == SipFail)
			{
				return SipFail;
			}

			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Authorization: ");
			else
				STRCAT ( pEndBuff,out, ",");
			gc = auth->pCredential;

			/* Now get Credentials */
			if (gc->dType == SipCredBasic)
			{
				STRCAT ( pEndBuff,out, "Basic ");
				STRCAT ( pEndBuff,out, gc->u.pBasic);
			}
			else /* its a Challenge so get SipGenericChallenge */
			{
				SipGenericChallenge *gch;

				gch = gc->u.pChallenge;
				STRCAT ( pEndBuff,out, gch->pScheme);
				/* Now get slAuthorizationParam which is a sipList */
				if (SipFail==sip_formSipParamList\
						(pEndBuff,&out,&(gch->slParam),(SIP_S8bit *) ",",0,err))
				{
					return SipFail;
				}
			} /* of SipGenericChallenge */
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

	/*=============================================================*/
		case SipHdrTypeWwwAuthenticate:
		{
			/* Get WWW Auth Header */
			SipWwwAuthenticateHeader *wah;
			SipGenericChallenge *gch;

			res  = sip_listGetAt (&(s->slWwwAuthenticateHdr), ndx,\
					(SIP_Pvoid *) &wah, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "WWW-Authenticate: ");
			else
				STRCAT ( pEndBuff,out, ",");

			gch = wah->pChallenge;
			STRCAT ( pEndBuff,out, gch->pScheme);

			/* Now get slAuthorizationParam which is a sipList */
			if (SipFail==sip_formSipParamList(pEndBuff,&out,&(gch->slParam),\
							(SIP_S8bit *) ",",0,err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out,CRLF);
			break;
		} /*  www auth */
	/*=============================================================*/
		case SipHdrTypeAuthenticationInfo:
		{
			/* Get Auth Info Header */
			SipAuthenticationInfoHeader *pAuthinfo;
			SIP_U32bit dSize=0,dIndex=0,dFirst=0;

			res  = sip_listGetAt (&(s->slAuthenticationInfoHdr), ndx, \
					(SIP_Pvoid *) &pAuthinfo, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if (mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Authentication-Info: ");
			else if (mode == SipModeJoin)
				STRCAT ( pEndBuff,out, ",");
				
			{
				sip_listSizeOf( &(pAuthinfo->slNameValue), &dSize, err);
				while (dIndex < dSize)
				{
					SipNameValuePair *c;
					sip_listGetAt ( &(pAuthinfo->slNameValue), dIndex, \
										(SIP_Pvoid *)&c, err);
					if ( dFirst != 0 )
					{
						STRCAT (pEndBuff,out, ",");
					}
					STRCAT (pEndBuff,out, c->pName);
					STRCAT (pEndBuff,out,"=");
					STRCAT (pEndBuff,out, c->pValue);
					dIndex++;
					dFirst=1;
				}
				STRCAT (pEndBuff,out,CRLF);
			}
			break;

		} /*  auth info */
 	/*=============================================================*/
		case SipHdrTypeRSeq:
		{
			SIP_S8bit fwd[20];
			if (s->pRSeqHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"RSeq: ");
			HSS_SNPRINTF((char *)fwd, 20,"%u",s->pRSeqHdr->dRespNum);
			fwd[ 20-1]='\0';
			STRCAT ( pEndBuff,out, fwd);
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}
		case SipHdrTypeErrorInfo:
		{
			/* Get ErrorInfo Header */
			SipErrorInfoHeader  *eih;

			res = sip_listGetAt\
							(&(s->slErrorInfoHdr), ndx, (SIP_Pvoid *) &eih,err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Error-Info: ");
			else
				STRCAT ( pEndBuff,out, ",");

			STRCAT ( pEndBuff,out, eih->pUri);
			if (SipFail==sip_formSipParamList(pEndBuff,&out, &(eih->slParam),\
					(SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}

			STRCAT ( pEndBuff,out,CRLF);
			break;
		}
		case SipHdrTypeMinExpires:
	    {
			SIP_S8bit dSec[20];
			if (s->pMinExpiresHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}

			if (ndx !=0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			if (mode==SipModeNew)
				STRCAT(pEndBuff,out,"Min-Expires: ");
			/* It is single instance header */

			HSS_SNPRINTF((SIP_S8bit*)dSec,20,"%u",s->pMinExpiresHdr->dSec);
			dSec[20-1]='\0';
			STRCAT( pEndBuff,out,dSec);
			STRCAT ( pEndBuff,out,CRLF);
			break;
	     }
#ifdef SIP_3GPP
        case SipHdrTypePAssociatedUri:
			{
			/* Get PAssociatedUri Header */
			SipPAssociatedUriHeader *r;

			res = sip_listGetAt (&(s->slPAssociatedUriHdr), ndx, (SIP_Pvoid *) &r, err);
			if (res == SipFail)
			{
				return SipFail;
			} /* switch */
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"P-Associated-URI: ");
				else
				STRCAT ( pEndBuff,out, ",");

            if (r->pDispName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out,r->pDispName);
				if (strcmp(r->pDispName,""))
					STRCAT ( pEndBuff,out," ");
			    STRCAT ( pEndBuff,out,"<");
			}


			if (SipFail==sip_formAddrSpec (pEndBuff,&out, r->pAddrSpec, err))
			{
				return SipFail;
			}
			if (r->pDispName !=SIP_NULL)
            {
                STRCAT ( pEndBuff,out,">");
            }
			if (SipFail==sip_formSipParamList(pEndBuff,&out, &(r->slParams),\
					(SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}
#endif
#ifdef SIP_CONGEST
		case  SipHdrTypeAcceptRsrcPriority:
        {
			/* Get Resource-Priority Header */
			SipRsrcPriorityHeader *rph;
			res = sip_listGetAt(&(s->slAcceptRsrcPriorityHdr), ndx,\
					(SIP_Pvoid *) &rph,err);
			if (res == SipFail)
			{
				return SipFail;
			}

			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
            {
                STRCAT ( pEndBuff,out, "Accept-Resource-Priority: ");
            }
			else
            {
                STRCAT ( pEndBuff,out, ",");
            }

			STRCAT ( pEndBuff,out, rph->pNamespace);
            STRCAT ( pEndBuff,out, ".");
			STRCAT ( pEndBuff,out, rph->pPriority);
			STRCAT ( pEndBuff,out,CRLF);

            break;
        }
#endif

#ifdef SIP_SECURITY
                case SipHdrTypeSecurityServer:
                {
                        /* Get Security-Server Header */
                        SipSecurityServerHeader *sser;
                       	res = sip_listGetAt (&(s->slSecurityServerHdr), ndx, (SIP_Pvoid *) \
    						&sser, err);
                        if (res == SipFail)
                        {
                                return SipFail;
                        }
                        if(mode==SipModeNone) {}
                        else if(mode==SipModeNew)
                                STRCAT ( pEndBuff,out,"Security-Server: ");
                                else
                                STRCAT ( pEndBuff,out, ",");

                        if (sser->pMechanismName !=SIP_NULL)
                        {
                                STRCAT ( pEndBuff,out,sser->pMechanismName);
                        }

                        if (SipFail==sip_formSipParamList(pEndBuff,&out, &(sser->slParams),\
                                        (SIP_S8bit *) ";", 1, err))
                        {
                                return SipFail;
                        }
                        STRCAT ( pEndBuff,out, CRLF);
                        break;
                }
#endif

		default:
#ifdef SIP_DCS
			if(sip_dcs_formSingleResponseHeader(pEndBuff,dType, ndx, mode,\
					form,s, &out, err)!=SipSuccess)
				return SipFail;
#endif
			break;

 	 } /* switch */

	if(pEndBuff)*ppOut = out;

	SIPDEBUGFN("Exiting from  sip_formSingleResponseHeader");
	return SipSuccess;
} /* sip_formSingleResponseHeader */


/*****************************************************************
** FUNCTION: sip_formSingleRequestHeader
**
**
** DESCRIPTION: Converts a SipReqHeader to pText
*****************************************************************/
SipBool sip_formSingleRequestHeader
#ifdef ANSI_PROTO
(
	SIP_S8bit	*pEndBuff,
	en_HeaderType dType,
	SIP_U32bit ndx,
	en_AdditionMode mode,
	en_HeaderForm form,
	SipReqHeader *s,
	SIP_S8bit **ppOut,
	SipError *err
)
#else
	(pEndBuff,dType, ndx, mode, form,s,ppOut,err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	en_HeaderForm form;
	SipReqHeader *s;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SipBool res;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into sip_formSingleRequestHeader");

	out = *ppOut;

	switch (dType)
	{
	/*=============================================================*/
		case SipHdrTypeAuthorization:
		/* Get Authorization Header */
		{
			SipGenericCredential *gc;
			SipAuthorizationHeader *auth;

			res = sip_listGetAt (&(s->slAuthorizationHdr), ndx,\
					(SIP_Pvoid *) &auth, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Authorization: ");
			else
				STRCAT ( pEndBuff,out, ",");

			gc = auth->pCredential;
			/* Now get Credentials */
			if (gc->dType == SipCredBasic)
			{
				STRCAT ( pEndBuff,out, "Basic ");
				STRCAT ( pEndBuff,out, gc->u.pBasic);
			}
			else /* its a Challenge so get SipGenericChallenge */
			{
				SipGenericChallenge *gch;
				gch = gc->u.pChallenge;
				STRCAT ( pEndBuff,out, gch->pScheme);
				/* Now get slAuthorizationParam which is a sipList */
				if (SipFail==sip_formSipParamList(pEndBuff,&out,\
							&(gch->slParam),(SIP_S8bit *) ",",0,err))
				{
					return SipFail;
				}
			} /* of SipGenericChallenge */
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

		/*=============================================================*/
		case SipHdrTypeReplaces:
		/* process  Replaces  hdrs */
		{

			if (s->pReplacesHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}

			if (ndx !=0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Replaces: ");
			/* Single Instance header */

			STRCAT ( pEndBuff,out, s->pReplacesHdr->pCallid);
			if(s->pReplacesHdr->pFromTag)
			{
				STRCAT ( pEndBuff,out, ";from-tag=");
				STRCAT ( pEndBuff,out, s->pReplacesHdr->pFromTag);
			}
			if(s->pReplacesHdr->pToTag)
			{
				STRCAT ( pEndBuff,out, ";to-tag=");
				STRCAT ( pEndBuff,out, s->pReplacesHdr->pToTag);
			}
			/* form the generic-param */
			{
			SIP_U32bit dSize=0,dIndex=0;
			sip_listSizeOf( &(s->pReplacesHdr->slParams), &dSize, err);
				while (dIndex < dSize)
				{
					SipNameValuePair *c;
					sip_listGetAt ( &(s->pReplacesHdr->slParams), dIndex, \
										(SIP_Pvoid *)&c, err);
					STRCAT (pEndBuff,out,";");
					STRCAT (pEndBuff,out, c->pName);
					if(c->pValue)
					{
						STRCAT (pEndBuff,out,"=");
						STRCAT (pEndBuff,out, c->pValue);
					}
					dIndex++;
				}
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		} /* of Replaces Header */


	/*=============================================================*/
		case SipHdrTypeHide:
		{
			/* Get Hide Header */
			if (s->pHideHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Hide: ");
			STRCAT ( pEndBuff,out, s->pHideHdr->pType);
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

	/*=============================================================*/
		case SipHdrTypeMaxforwards:
		{
			/* Get Max Forwards Header */
			SIP_S8bit fwd[20];
			if (s->pMaxForwardsHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Max-Forwards: ");
			HSS_SNPRINTF((char *)fwd, 20,"%u", s->pMaxForwardsHdr->dHops);
			fwd[ 20-1]='\0';
			STRCAT ( pEndBuff,out, fwd);
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}


	/*=============================================================*/
		case SipHdrTypePriority:
		{
	/* Get Priority Header */
			if (s->pPriorityHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Priority: ");
			STRCAT ( pEndBuff,out, s->pPriorityHdr->pPriority);
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

	/*=============================================================*/
		case SipHdrTypeProxyauthorization:
		{
			SipProxyAuthorizationHeader *pauth;
			/* Get Proxy Authorization Header */
			SipGenericCredential *gc;

			res = sip_listGetAt (&(s->slProxyAuthorizationHdr), ndx,\
				(SIP_Pvoid *) &pauth, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Proxy-Authorization: ");
			else
				STRCAT ( pEndBuff,out, ",");

			gc = pauth->pCredentials;
			/* Now get Credentials */
			if (gc->dType == SipCredBasic)
			{
				STRCAT ( pEndBuff,out, "Basic ");
				STRCAT ( pEndBuff,out, gc->u.pBasic);
			}
			else /* its a Challenge so get SipGenericChallenge */
			{
				SipGenericChallenge *gch;
				gch = gc->u.pChallenge;
				STRCAT ( pEndBuff,out, gch->pScheme);
				/* Now get slAuthorizationParam which is a sipList */
				if (SipFail==sip_formSipParamList(pEndBuff,&out,\
							&(gch->slParam),(SIP_S8bit *) ",",0,err))
				{
					return SipFail;
				}
			} /* of SipGenericChallenge */
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

		/*=============================================================*/
		case SipHdrTypeProxyRequire:
		{
			SipProxyRequireHeader *spr;
			/* Get Proxy-Require Header */

			res = sip_listGetAt (&(s->slProxyRequireHdr), ndx, \
										(SIP_Pvoid *) &spr, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Proxy-Require: ");
			else
				STRCAT ( pEndBuff,out, ",");
			STRCAT ( pEndBuff,out, spr->pToken);
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		/*=============================================================*/
		case SipHdrTypeRoute:
		{
			/* Get route Header */
			SipRouteHeader *r;

			res = sip_listGetAt (&(s->slRouteHdr), ndx, (SIP_Pvoid *) &r, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Route: ");
				else
				STRCAT ( pEndBuff,out, ",");

			if (r->pDispName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out,r->pDispName);
				if (strcmp(r->pDispName,""))
					STRCAT ( pEndBuff,out," ");
			}

			STRCAT ( pEndBuff,out,"<");
			if (SipFail==sip_formAddrSpec (pEndBuff,&out, r->pAddrSpec, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out,">");
			if (SipFail==sip_formSipParamList(pEndBuff,&out, &(r->slParams),\
					(SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

   		 /*=============================================================*/
		case SipHdrTypeReferTo:
		{
			SIP_S8bit addBracket=0;
			/* Get refer-to Header */
			if (s->pReferToHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}

			if(mode==SipModeNew)
			{
				if (form==SipFormShort)
					STRCAT ( pEndBuff,out, "r: ");
				else
					STRCAT ( pEndBuff,out, "Refer-To: ");
			}

			if (s->pReferToHdr->pDispName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out, s->pReferToHdr->pDispName);
				if (strcmp(s->pReferToHdr->pDispName,""))
					STRCAT ( pEndBuff,out," ");
				addBracket=1;
			}
			if(s->pReferToHdr->pAddrSpec !=SIP_NULL )
			{
				SipBool paramPresent=SipFail;

				if (!addBracket)
				{
					paramPresent=sip_areThereParamsinAddrSpec\
												(s->pReferToHdr->pAddrSpec,err);
				    if (paramPresent==SipSuccess) addBracket=1;
				}

				if (addBracket) STRCAT ( pEndBuff,out,"<");
				if (SipFail==sip_formAddrSpec\
							(pEndBuff,&out, s->pReferToHdr->pAddrSpec, err))
				{
					return SipFail;
				}
				if (addBracket) STRCAT ( pEndBuff,out,">");
			}

			if (SipFail==sip_formSipParamList(pEndBuff,&out,\
							&(s->pReferToHdr->slParams),(SIP_S8bit *) ";",1,err))
			{
				return SipFail;
			}

			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		 /*=============================================================*/
		case SipHdrTypeReferredBy:
		{
			/* Get refer-to Header */
			SIP_S8bit addBracket=0;
			SipReferredByHeader *sf;

			sf=s->pReferredByHdr;

			if (sf == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}

			if(mode==SipModeNew)
			{
				if (form==SipFormShort)
					STRCAT ( pEndBuff,out, "b: ");
				else
					STRCAT ( pEndBuff,out, "Referred-By: ");
			}

			if (sf->pDispName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out, sf->pDispName);
				if (strcmp(sf->pDispName,""))
					STRCAT ( pEndBuff,out," ");
				addBracket=1;
			}
			if (sf->pAddrSpecReferrer != SIP_NULL)
			{
				SipBool paramPresent=SipFail;

				if (!addBracket)
				{
					paramPresent=sip_areThereParamsinAddrSpec\
												(sf->pAddrSpecReferrer,err);
				    if (paramPresent==SipSuccess) addBracket=1;
				}

				if (addBracket) STRCAT ( pEndBuff,out,"<");
				if (SipFail==sip_formAddrSpec\
							(pEndBuff,&out, sf->pAddrSpecReferrer, err))
				{
					return SipFail;
				}
				if (addBracket) STRCAT ( pEndBuff,out,">");
			}
			
			if (sf->pMsgId != SIP_NULL)
			{
				STRCAT(pEndBuff,out,";cid=");
				STRCAT(pEndBuff,out,sf->pMsgId);
			}
			
			if (sf->pAddrSpecReferenced != SIP_NULL)
			{
				STRCAT(pEndBuff,out,";ref=<");
				if (SipFail==sip_formAddrSpec\
						(pEndBuff,&out, sf->pAddrSpecReferenced, err))
				{
					return SipFail;
				}
				STRCAT ( pEndBuff,out,">");
			}

			if (SipFail==sip_formSipParamList(pEndBuff,&out,\
							&(sf->slParams),(SIP_S8bit *) ";",1,err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, CRLF);

			break;
		}

		/*=============================================================*/
		/* Get Response Key Header */
		case SipHdrTypeResponseKey:
		{
			if (s->pRespKeyHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}

			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Response-Key: ");
			if (s->pRespKeyHdr->pKeyScheme != SIP_NULL)
			{
				STRCAT ( pEndBuff,out, s->pRespKeyHdr->pKeyScheme);
			}
			if (SipFail==sip_formSipParamList(pEndBuff,&out,\
						&(s->pRespKeyHdr->slParam),(SIP_S8bit *) ",",0,err))
			{
				return SipFail;
			}

			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		/*=============================================================*/
		/* Get Subject Header */
		case SipHdrTypeSubject:
		{
			if (s->pSubjectHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew)
			{

				if (form==SipFormShort)
					STRCAT ( pEndBuff,out,"s: ");
				else
					STRCAT ( pEndBuff,out, "Subject: ");
			}
			if (s->pSubjectHdr->pSubject!=SIP_NULL)
			{
				if (s->pSubjectHdr->pSubject[0]==' ')
					STRCAT ( pEndBuff,out, &(s->pSubjectHdr->pSubject[1]));
				else
					STRCAT ( pEndBuff,out, s->pSubjectHdr->pSubject);
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		/*=============================================================*/
		case SipHdrTypeWwwAuthenticate:
		{
		/* Get WWW Authenticate Header */
			SipWwwAuthenticateHeader *wah;
			SipGenericChallenge *gch;

			res = sip_listGetAt (&(s->slWwwAuthenticateHdr), ndx,\
					(SIP_Pvoid *) &wah, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "WWW-Authenticate: ");
				else
				STRCAT ( pEndBuff,out, ",");
			gch = wah->pChallenge;
			STRCAT ( pEndBuff,out, gch->pScheme);
			/* Now get slAuthorizationParam which is a sipList */
			if (SipFail==sip_formSipParamList\
					(pEndBuff,&out,&(gch->slParam),(SIP_S8bit *) ",",0,err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out,CRLF);
			break;
		} /*  www auth */
		/*=============================================================*/
		case SipHdrTypeRAck:
		{
			SIP_S8bit fwd[20];
			if (s->pRackHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"RAck: ");
			HSS_SNPRINTF((char *)fwd, 20,"%u",s->pRackHdr->dRespNum);
			fwd[ 20-1]='\0';
			STRCAT ( pEndBuff,out, fwd);
			STRCAT ( pEndBuff,out, " ");
			HSS_SNPRINTF((char *)fwd, 20,"%u",s->pRackHdr->dCseqNum);
			fwd[ 20-1]='\0';
			STRCAT ( pEndBuff,out, fwd);
			STRCAT ( pEndBuff,out, " ");
			STRCAT ( pEndBuff,out, s->pRackHdr->pMethod);
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

		/*=============================================================*/

#ifdef SIP_CCP
		case SipHdrTypeRequestDisposition:
		{
			/* Get RequestDisposition Header */
			SipRequestDispositionHeader  *rdh;

			res = sip_listGetAt (&(s->slRequestDispositionHdr), ndx,\
										(SIP_Pvoid *) &rdh, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
			{
				if (form==SipFormShort)
					STRCAT ( pEndBuff,out,"d: ");
				else
					STRCAT ( pEndBuff,out, "Request-Disposition: ");
			}
			else
			STRCAT ( pEndBuff,out, ",");

			STRCAT ( pEndBuff,out, rdh->pFeature);
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

		/*=============================================================*/

		case SipHdrTypeAcceptContact:
		{
#ifdef SIP_CCP_VERSION10
		 {	
			/* Get AcceptContact Header */
			SipAcceptContactHeader  *pAch=SIP_NULL;
			SipAcceptContactParam   *pAcp=SIP_NULL;
			SIP_S8bit *pValue=SIP_NULL;
			SIP_U32bit dCpSize=0,dCpIter=0,dCount=0,dIter=0;

			res = sip_listGetAt (&(s->slAcceptContactHdr), ndx,\
						(SIP_Pvoid *) &pAch, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
			{
				if (form==SipFormShort)
                       	STRCAT ( pEndBuff,out, "a: ");
                        else
						STRCAT ( pEndBuff,out, "Accept-Contact: ");
			}
			else
				STRCAT ( pEndBuff,out, ",");
			STRCAT ( pEndBuff,out, "*");
			/* Now fetch slContactParam */
			sip_listSizeOf( &(pAch->slAcceptContactParams), &dCpSize, err);
			dCpIter = 0;
			while (dCpIter < dCpSize)
			{
				sip_listGetAt (&(pAch->slAcceptContactParams), dCpIter,\
					(SIP_Pvoid *) &pAcp, err);
				STRCAT ( pEndBuff,out, ";");
				/* Now find out Type of contact Param */
				switch (pAcp->dType)
				{
					case SipAccContactTypeGeneric:
					     {
						  STRCAT ( pEndBuff,out,pAcp->u.pParam->pName);
						  STRCAT ( pEndBuff,out,"=");
						  sip_listSizeOf( &(pAcp->u.pParam->slValue),\
										&dCount, err);
						  /* iterate through SipList of Param and form */
						  /* comma separated entries */
						  if(dCount>1)
					  	  {
						  	dIter = 0;
							STRCAT ( pEndBuff,out,"\"");
							  while (dIter < dCount)
						  	  {
								  sip_listGetAt (&(pAcp->u.pParam->slValue),\
								  	dIter,(SIP_Pvoid *) &pValue, err);
								  STRCAT ( pEndBuff,out,pValue);
								  if(dIter<dCount-1)
									  STRCAT ( pEndBuff,out,",");
								  dIter++;
							  }
							STRCAT ( pEndBuff,out,"\"");
						  }
						  else if (dCount==1)
					  	  {
							  sip_listGetAt (&(pAcp->u.pParam->slValue), 0,
								  (SIP_Pvoid *) &pValue, err);
							  STRCAT ( pEndBuff,out,pValue);
						  }
					     break;
					    }
					case SipAccContactTypeFeature:
					   {
						STRCAT ( pEndBuff,out,pAcp->u.pParam->pName);
						STRCAT ( pEndBuff,out,"=<");
						sip_listSizeOf( &(pAcp->u.pParam->slValue),\
										&dCount, err);
						/* iterate through SipList of Param and form */
						/* comma separated entries */
						if(dCount>1)
						{
							dIter = 0;
							while (dIter < dCount)
							{
								sip_listGetAt (&(pAcp->u.pParam->slValue),\
									dIter,(SIP_Pvoid *) &pValue, err);
								STRCAT ( pEndBuff,out,pValue);
								if(dIter<dCount-1)
									STRCAT ( pEndBuff,out,",");
								dIter++;
							}
						}
						else if (dCount==1)
						{
							sip_listGetAt (&(pAcp->u.pParam->slValue), 0,
								(SIP_Pvoid *) &pValue, err);
							STRCAT ( pEndBuff,out,pValue);
						}
						STRCAT ( pEndBuff,out,">");
					   break;
					  }
				  case SipAccContactTypeOther:
						STRCAT ( pEndBuff,out,pAcp->u.pToken);
						break ;
					case SipAccContactTypeAny:
						break;
				} /* of switch */
				dCpIter++;
			} /* while (dCpIter < dCpSize)*/
			STRCAT ( pEndBuff,out,CRLF);
			break;
    }
#else
			/* Get AcceptContact Header */
			SipAcceptContactHeader  *ach;
			SipAcceptContactParam *acp;
			SIP_S8bit *pValue;
			SIP_U32bit cpsize, cpiter,count,iter;
			SIP_S8bit addBracket=0;

			res = sip_listGetAt (&(s->slAcceptContactHdr), ndx,\
						(SIP_Pvoid *) &ach, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
			{
				if (form==SipFormShort)
                       	STRCAT ( pEndBuff,out, "a: ");
                        else
						STRCAT ( pEndBuff,out, "Accept-Contact: ");
			}
			else
				STRCAT ( pEndBuff,out, ",");
			if (ach->pDispName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out, ach->pDispName);
				if (strcmp(ach->pDispName,""))
					STRCAT ( pEndBuff,out," ");
				addBracket = 1;
			}
			if (ach->pAddrSpec !=SIP_NULL)
			{
				SipBool paramPresent=SipFail;

				if (!addBracket)
				{
					paramPresent=sip_areThereParamsinAddrSpec\
							(ach->pAddrSpec,err);
				    if (paramPresent==SipSuccess) addBracket=1;
				}
				if (addBracket) STRCAT ( pEndBuff,out,"<");
				/* if dispname was there add <>s around */
				if (SipFail==sip_formAddrSpec \
						(pEndBuff,&out, ach->pAddrSpec, err ))
				{
					return SipFail;
				}

				if (addBracket) STRCAT ( pEndBuff,out,">");
				/* if dispname was there add <>s around */
			} /* of if addrspec */
			else
			{
				STRCAT ( pEndBuff,out, "*");
			}
			/* Now fetch slContactParam */
			sip_listSizeOf( &(ach->slAcceptContactParams), &cpsize, err);
			cpiter = 0;
			while (cpiter < cpsize)
			{
				sip_listGetAt (&(ach->slAcceptContactParams), cpiter,\
					(SIP_Pvoid *) &acp, err);
				STRCAT ( pEndBuff,out, ";");
				/* Now find out Type of contact Param */
				switch (acp->dType)
				{
					case SipAccContactTypeQvalue:
						STRCAT ( pEndBuff,out, "q=");
						STRCAT ( pEndBuff,out, acp->u.pQvalue);
						break;
					case SipAccContactTypeExt:
						STRCAT ( pEndBuff,out,acp->u.pExtParam->pName);
						STRCAT ( pEndBuff,out,"=");
						sip_listSizeOf( &(acp->u.pExtParam->slValue),\
										&count, err);
						/* iterate through SipList of Param and form */
						/* comma separated entries */
						if(count>1)
						{
							iter = 0;
							STRCAT ( pEndBuff,out,"\"");
							while (iter < count)
							{
								sip_listGetAt (&(acp->u.pExtParam->slValue),\
									iter,(SIP_Pvoid *) &pValue, err);
								STRCAT ( pEndBuff,out,pValue);
								if(iter<count-1)
									STRCAT ( pEndBuff,out,",");
								iter++;
							}
							STRCAT ( pEndBuff,out,"\"");
						}
						else if (count==1)
						{
							sip_listGetAt (&(acp->u.pExtParam->slValue), 0,
								(SIP_Pvoid *) &pValue, err);
							STRCAT ( pEndBuff,out,pValue);
						}
						break;
					case SipAccContactTypeAny:
						break;
				} /* of switch */
				cpiter++;
			} /* while (cpiter < cpsize)*/
			STRCAT ( pEndBuff,out,CRLF);
			break;
#endif
		}
		case SipHdrTypeRejectContact:
		{
#ifdef SIP_CCP_VERSION10
			{
			/* Get RejectContact Header */
			SipRejectContactHeader  *pRch=SIP_NULL;
			SipRejectContactParam *pRcp=SIP_NULL;
			SIP_S8bit *pValue=SIP_NULL;
			SIP_U32bit  dCpSize=0, dCpIter=0,dCount=0,dIter=0;

			res = sip_listGetAt (&(s->slRejectContactHdr), ndx,\
						(SIP_Pvoid *) &pRch, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
			{
				if (form==SipFormShort)
                       	STRCAT ( pEndBuff,out, "j: ");
                        else
						STRCAT ( pEndBuff,out, "Reject-Contact: ");
			}
			else
				STRCAT ( pEndBuff,out, ",");
			STRCAT ( pEndBuff,out, "*");
			/* Now fetch slContactParam */
			sip_listSizeOf( &(pRch->slRejectContactParams), &dCpSize, err);
			dCpIter = 0;
			while (dCpIter < dCpSize)
			{
				sip_listGetAt (&(pRch->slRejectContactParams), dCpIter,\
					(SIP_Pvoid *) &pRcp, err);
				STRCAT ( pEndBuff,out, ";");
				/* Now find out Type of contact Param */
				switch (pRcp->dType)
				{
					case SipRejContactTypeGeneric:
					   {
						STRCAT ( pEndBuff,out,pRcp->u.pParam->pName);
						STRCAT ( pEndBuff,out,"=");
						sip_listSizeOf( &(pRcp->u.pParam->slValue),\
										&dCount, err);
						/* iterate through SipList of Param and form */
						/* comma separated entries */
						if(dCount>1)
						{
							dIter = 0;
							STRCAT ( pEndBuff,out,"\"");
							while (dIter < dCount)
							{
								sip_listGetAt (&(pRcp->u.pParam->slValue),\
									dIter,(SIP_Pvoid *) &pValue, err);
								STRCAT ( pEndBuff,out,pValue);
								if(dIter<dCount-1)
									STRCAT ( pEndBuff,out,",");
								dIter++;
							}
							STRCAT ( pEndBuff,out,"\"");
						}
						else if (dCount==1)
						{
							sip_listGetAt (&(pRcp->u.pParam->slValue), 0,
								(SIP_Pvoid *) &pValue, err);
							STRCAT ( pEndBuff,out,pValue);
						}
						break;
					   }
					case SipRejContactTypeFeature:
					   {
						STRCAT ( pEndBuff,out,pRcp->u.pParam->pName);
						STRCAT ( pEndBuff,out,"=<");
						sip_listSizeOf( &(pRcp->u.pParam->slValue),\
										&dCount, err);
						/* iterate through SipList of Param and form */
						/* comma separated entries */
						if(dCount>1)
						{
							dIter = 0;
							while (dIter < dCount)
							{
								sip_listGetAt (&(pRcp->u.pParam->slValue),\
									dIter,(SIP_Pvoid *) &pValue, err);
								STRCAT ( pEndBuff,out,pValue);
								if(dIter<dCount-1)
									STRCAT ( pEndBuff,out,",");
								dIter++;
							}
						}
						else if (dCount==1)
						{
							sip_listGetAt (&(pRcp->u.pParam->slValue), 0,
								(SIP_Pvoid *) &pValue, err);
							STRCAT ( pEndBuff,out,pValue);
						}
						STRCAT ( pEndBuff,out,">");
						break;
					   }
				  case SipRejContactTypeOther:
						STRCAT ( pEndBuff,out,pRcp->u.pToken);
						break ;
					case SipRejContactTypeAny:
						break;
				} /* of switch */
				dCpIter++;
			} /* while (dCpIter < dCpSize)*/
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}
#else
			/* Get RejectContact Header */
			SipRejectContactHeader  *rch;
			SipRejectContactParam *rcp;
			SIP_S8bit *pValue;
			SIP_U32bit cpsize, cpiter,count,iter;
			SIP_U32bit addBracket=0;

			res = sip_listGetAt (&(s->slRejectContactHdr), ndx, \
					(SIP_Pvoid *) &rch, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
			{
				if (form==SipFormShort)
                   	STRCAT ( pEndBuff,out, "j: ");
				else
					STRCAT ( pEndBuff,out, "Reject-Contact: ");
			}
			else
				STRCAT ( pEndBuff,out, ",");
			if (rch->pDispName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out, rch->pDispName);
				if (strcmp(rch->pDispName,""))
					STRCAT ( pEndBuff,out," ");
				addBracket = 1;
			}
			if (rch->pAddrSpec !=SIP_NULL)
			{
				SipBool paramPresent=SipFail;

				if (!addBracket)
				{
					paramPresent=sip_areThereParamsinAddrSpec\
								(rch->pAddrSpec,err);
				    if (paramPresent==SipSuccess) addBracket=1;
				}

				if (addBracket) STRCAT ( pEndBuff,out,"<");
				/* if dispname was there add <>s around */
				if (SipFail==sip_formAddrSpec\
							(pEndBuff,&out, rch->pAddrSpec, err ))
				{
					return SipFail;
				}
				if (addBracket) STRCAT ( pEndBuff,out,">");
				/* if dispname was there add <>s around */
			} /* of if addrspec */
			else
			{
				STRCAT ( pEndBuff,out, "*");
			}
			/* Now fetch slContactParam */
			sip_listSizeOf( &(rch->slRejectContactParams), &cpsize, err);
			cpiter = 0;
			while (cpiter < cpsize)
			{
				sip_listGetAt (&(rch->slRejectContactParams), cpiter,
					(SIP_Pvoid *) &rcp, err);
				STRCAT ( pEndBuff,out, ";");
				/* Now find out Type of contact pParam */
				switch (rcp->dType)
				{
					case SipAccContactTypeQvalue:
						STRCAT ( pEndBuff,out, "q=");
						STRCAT ( pEndBuff,out, rcp->u.pQvalue);
						break;
				     case SipAccContactTypeExt:
						STRCAT ( pEndBuff,out,rcp->u.pExtParam->pName);
						STRCAT ( pEndBuff,out,"=");
						/* iterate through SipList of Param and form */
						/* comma separated entries */
						sip_listSizeOf\
								( &(rcp->u.pExtParam->slValue), &count, err);
						if(count>1)
						{
							iter = 0;
							STRCAT ( pEndBuff,out,"\"");
							while (iter < count)
							{
								sip_listGetAt(&(rcp->u.pExtParam->slValue),\
									iter,(SIP_Pvoid *) &pValue, err);
								STRCAT ( pEndBuff,out,pValue);
								if(iter<count-1)
									STRCAT ( pEndBuff,out,",");
								iter++;
							}
							STRCAT ( pEndBuff,out,"\"");
						}
						else if (count==1)
						{
							sip_listGetAt (&(rcp->u.pExtParam->slValue), 0,
								(SIP_Pvoid *) &pValue, err);
							STRCAT ( pEndBuff,out,pValue);
						}
						break;
					case SipAccContactTypeAny:
						break;
				} /* of switch */
				cpiter++;
			} /* while (cpiter < cpsize)*/
			STRCAT ( pEndBuff,out,CRLF);
			break;
#endif
		}
#endif
		case SipHdrTypeAlertInfo:
		{
			/* Get AlertInfo Header */
			SipAlertInfoHeader  *aih;

			res = sip_listGetAt(&(s->slAlertInfoHdr), ndx,\
					(SIP_Pvoid *) &aih,err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Alert-Info: ");
			else
				STRCAT ( pEndBuff,out, ",");

			STRCAT ( pEndBuff,out, aih->pUri);
			if (SipFail==sip_formSipParamList\
				(pEndBuff,&out, &(aih->slParam), (SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

		case SipHdrTypeInReplyTo:
		{
			/* Get InReplyTo Header */
			SipInReplyToHeader *rth;
			res = sip_listGetAt(&(s->slInReplyToHdr), ndx,\
					(SIP_Pvoid *) &rth,err);
			if (res == SipFail)
			{
				return SipFail;
			}

			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "In-Reply-To: ");
			else
				STRCAT ( pEndBuff,out, ",");

			STRCAT ( pEndBuff,out, rth->pCallId);
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}
		case SipHdrTypeAlso:
		{
			/* Get Also header */
			SipAlsoHeader *pAh;
			SIP_S8bit addBracket=0;

			res = sip_listGetAt(&(s->slAlsoHdr), ndx, (SIP_Pvoid*) &pAh, err);
			if(res == SipFail)
				return SipFail;
			if(mode==SipModeNone) {}
			else if(mode == SipModeNew)
				STRCAT(pEndBuff,out, "Also: ");
				else
				STRCAT(pEndBuff,out, ",");
			if(pAh->pDispName != SIP_NULL)
			{
				STRCAT(pEndBuff,out, pAh->pDispName);
				addBracket=1;
			}

			if (pAh->pAddrSpec != SIP_NULL)
			{
				SipBool paramPresent=SipFail;

				if (!addBracket)
				{
					paramPresent=sip_areThereParamsinAddrSpec\
							(pAh->pAddrSpec,err);
				    if (paramPresent==SipSuccess) addBracket=1;
				}

				if (addBracket) STRCAT ( pEndBuff,out,"<");
				if(SipFail==sip_formAddrSpec\
						(pEndBuff,&out, pAh->pAddrSpec, err))
				{
					return SipFail;
				}
				if (addBracket) STRCAT ( pEndBuff,out,">");
			}

			STRCAT ( pEndBuff,out,CRLF);
			break;
		}
#ifdef SIP_IMPP
		case  SipHdrTypeEvent:
		{
			/* Process Event Header */
			if (s->pEventHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}

			if (ndx !=0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
		  if(mode==SipModeNew)
      {
        	if (form==SipFormShort)
							STRCAT ( pEndBuff,out, "o: ");
					else
							STRCAT ( pEndBuff,out,"Event: ");
      }

			if (s->pEventHdr->pEventType !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out, s->pEventHdr->pEventType);
			}

			if (SipFail==sip_formSipParamList(pEndBuff,&out,\
					&(s->pEventHdr->slParams),(SIP_S8bit *) ";",1,err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}

		case SipHdrTypeSubscriptionState:
		{
			if (s->pSubscriptionStateHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}

			if (ndx !=0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}		/* Process Subscription State Header */
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Subscription-State: ");

			STRCAT( pEndBuff,out, s->pSubscriptionStateHdr->pSubState);
			if (SipFail==sip_formSipParamList (pEndBuff,&out,\
				&(s->pSubscriptionStateHdr->slParams),(SIP_S8bit *) ";",1,err))
			{
				return SipFail;
			}

			STRCAT ( pEndBuff,out,CRLF);
			break;
		}
#endif

#ifdef SIP_CONGEST
		case  SipHdrTypeRsrcPriority:
        {
			/* Get Resource-Priority Header */
			SipRsrcPriorityHeader *rph;
			res = sip_listGetAt(&(s->slRsrcPriorityHdr), ndx,\
					(SIP_Pvoid *) &rph,err);
			if (res == SipFail)
			{
				return SipFail;
			}

			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
            {
                STRCAT ( pEndBuff,out, "Resource-Priority: ");
            }
			else
            {
                STRCAT ( pEndBuff,out, ",");
            }

			STRCAT ( pEndBuff,out, rph->pNamespace);
            STRCAT ( pEndBuff,out, ".");
			STRCAT ( pEndBuff,out, rph->pPriority);
			STRCAT ( pEndBuff,out,CRLF);
            
            break;
        }
#endif

#ifdef SIP_CONF
		case SipHdrTypeJoin:
		/* process  Join  hdr */
		{

			if (s->pJoinHdr == SIP_NULL)
			{
				*err = E_INV_PARAM;
				return SipFail;
			}

			if (ndx !=0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Join: ");
			/* Single Instance header */

			STRCAT ( pEndBuff,out, s->pJoinHdr->pCallid);
			if(s->pJoinHdr->pFromTag)
			{
				STRCAT ( pEndBuff,out, ";from-tag=");
				STRCAT ( pEndBuff,out, s->pJoinHdr->pFromTag);
			}
			if(s->pJoinHdr->pToTag)
			{
				STRCAT ( pEndBuff,out, ";to-tag=");
				STRCAT ( pEndBuff,out, s->pJoinHdr->pToTag);
			}
			/* form the generic-param */
			{
			SIP_U32bit dSize=0,dIndex=0;
			sip_listSizeOf( &(s->pJoinHdr->slParams), &dSize, err);
				while (dIndex < dSize)
				{
					SipNameValuePair *c;
					sip_listGetAt ( &(s->pJoinHdr->slParams), dIndex, \
										(SIP_Pvoid *)&c, err);
					STRCAT (pEndBuff,out,";");
					STRCAT (pEndBuff,out, c->pName);
					if(c->pValue)
					{
						STRCAT (pEndBuff,out,"=");
						STRCAT (pEndBuff,out, c->pValue);
					}
					dIndex++;
				}
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		} /* of Join Header */
#endif
/*=============================================================*/       
#ifdef SIP_SECURITY		
		case SipHdrTypeSecurityClient:
		{
			/* Get Security-Client Header */
			SipSecurityClientHeader *scli;
			res = sip_listGetAt (&(s->slSecurityClientHdr), ndx, (SIP_Pvoid *) \
						&scli, err);
			if (res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"Security-Client: ");
				else
				STRCAT ( pEndBuff,out, ",");

			if (scli->pMechanismName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out,scli->pMechanismName);
			}

			if (SipFail==sip_formSipParamList(pEndBuff,&out, &(scli->slParams),\
					(SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}
		case SipHdrTypeSecurityVerify:
                {
                        /* Get Security-Verify Header */
                        SipSecurityVerifyHeader *sver;
                        res = sip_listGetAt (&(s->slSecurityVerifyHdr), ndx, (SIP_Pvoid *) \
                                                &sver, err);
                        if (res == SipFail)
                        {
                                return SipFail;
                        }
                        if(mode==SipModeNone) {}
                        else if(mode==SipModeNew)
                                STRCAT ( pEndBuff,out,"Security-Verify: ");
                                else
                                STRCAT ( pEndBuff,out, ",");

                        if (sver->pMechanismName !=SIP_NULL)
                        {
                                STRCAT ( pEndBuff,out,sver->pMechanismName);
                        }

                        if (SipFail==sip_formSipParamList(pEndBuff,&out, &(sver->slParams),\
                                        (SIP_S8bit *) ";", 1, err))
                        {
                                return SipFail;
                        }
                        STRCAT ( pEndBuff,out, CRLF);
                        break;
                }
#endif
 
#ifdef SIP_3GPP
        case SipHdrTypePCalledPartyId:
		{
			/* Get PCalledPartyId Header */
			SipPCalledPartyIdHeader *r;

            r = s->pPCalledPartyIdHdr;
            if (r == SIP_NULL)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out,"P-Called-Party-ID: ");
				else
				STRCAT ( pEndBuff,out, ",");

			if (r->pDispName !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out,r->pDispName);
				if (strcmp(r->pDispName,""))
					STRCAT ( pEndBuff,out," ");
			}

			STRCAT ( pEndBuff,out,"<");
			if (SipFail==sip_formAddrSpec (pEndBuff,&out, r->pAddrSpec, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out,">");
			if (SipFail==sip_formSipParamList(pEndBuff,&out, &(r->slParams),\
					(SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}
        
		/* PVisitedNetworkId Header */
        case SipHdrTypePVisitedNetworkId:
		{
            SipPVisitedNetworkIdHeader *vnh;
			res = sip_listGetAt(&(s->slPVisitedNetworkIdHdr), ndx,\
					(SIP_Pvoid *) &vnh,err);
			if (res == SipFail)
			{
				return SipFail;
			}

			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
            {
                STRCAT ( pEndBuff,out, "P-Visited-Network-ID:");
            }
			else
            {
                STRCAT ( pEndBuff,out, ",");
            }
			STRCAT ( pEndBuff,out, vnh->pVNetworkSpec);
			if (SipFail==sip_formSipParamList(pEndBuff,&out, &(vnh->slParams),\
					(SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
        }
#endif
        
		default:
#ifdef SIP_DCS
			if(sip_dcs_formSingleRequestHeader(pEndBuff,dType, ndx, mode, form,\
			s,&out,err)!=SipSuccess)
				return SipFail;
#endif
			break;
	} /* switch */
	if(pEndBuff)*ppOut = out;

	SIPDEBUGFN("Exiting from sip_formSingleRequestHeader");
	return SipSuccess;
} /* sip_formSingleRequestHeader */

/*****************************************************************
** FUNCTION: sip_formSingleRequest
**
**
** DESCRIPTION: Forms a single request Header
*****************************************************************/

SipBool sip_formSingleRequest
#ifdef ANSI_PROTO
	(
	SIP_S8bit	*pEndBuff,
	en_HeaderType dType,
	 SIP_U32bit ndx,
	 en_AdditionMode mode,
	en_HeaderForm form,
	 SipMessage *s,
	 SIP_S8bit **ppOut,
	 SipError *err)
#else
	(pEndBuff,dType, ndx,mode,form, s, ppOut, err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	en_HeaderForm form;
	SipMessage *s;
	 SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SipBool res;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into sip_formSingleRequest");

	out = *ppOut;
	strcpy (out, "");
	switch(dType)
	{
			/* These are general Headers */
			case SipHdrTypeAccept:
			case SipHdrTypeAcceptEncoding:
			case SipHdrTypeAcceptLanguage:
			case SipHdrTypeCallId:
			case SipHdrTypeCseq:
			case SipHdrTypeDate:
			case SipHdrTypeEncryption:
			case SipHdrTypeReplyTo:
			case SipHdrTypeExpiresDate:
			case SipHdrTypeExpiresSec:
			case SipHdrTypeExpiresAny:
			case SipHdrTypeFrom:
			case SipHdrTypeRetryAfterDate:
			case SipHdrTypeRetryAfterSec:
			case SipHdrTypeRetryAfterAny:
			case SipHdrTypeRecordRoute:
#ifdef SIP_3GPP
			case SipHdrTypeServiceRoute:
			case SipHdrTypePath:
			case SipHdrTypePanInfo:
			case SipHdrTypePcVector:
#endif
			case SipHdrTypeTimestamp:
			case SipHdrTypeTo:
			case SipHdrTypeVia:
			case SipHdrTypeContentEncoding:
			case SipHdrTypeContentLength:
			case SipHdrTypeContentType:
			case SipHdrTypeContactNormal:
			case SipHdrTypeContactWildCard:
			case SipHdrTypeContactAny:
			case SipHdrTypeMimeVersion:
			case SipHdrTypeSupported:
			case SipHdrTypeCallInfo:
			case SipHdrTypeContentDisposition:
			case SipHdrTypeReason:
			case SipHdrTypeContentLanguage:
			case SipHdrTypeAllow:
			case SipHdrTypeRequire:
			case SipHdrTypeOrganization:
			case SipHdrTypeUserAgent:
#ifdef SIP_PRIVACY
			case SipHdrTypePrivacy:
#endif
#ifdef SIP_SESSIONTIMER
			case SipHdrTypeMinSE:
			case SipHdrTypeSessionExpires:
#endif
#ifdef SIP_IMPP
			case SipHdrTypeAllowEvents:
#endif
#ifdef SIP_DCS
			case SipHdrTypeDcsMediaAuthorization:
			case SipHdrTypeDcsBillingId:
			case SipHdrTypeDcsBillingInfo:
			case SipHdrTypeDcsLaes:
			case SipHdrTypeDcsGate:
			case SipHdrTypeDcsRemotePartyId:
			case SipHdrTypeDcsRpidPrivacy:
			case SipHdrTypeDcsAnonymity:
			case SipHdrTypeDcsState:
#endif
#ifdef SIP_PRIVACY
			case SipHdrTypePAssertId:
			case SipHdrTypePPreferredId:
#endif /* # ifdef SIP_PRIVACY */
#ifdef SIP_3GPP
			case SipHdrTypePcfAddr:
#endif

			case SipHdrTypeUnknown:
			 	res = sip_formSingleGeneralHeader (pEndBuff,dType,\
							ndx,mode,form,(s->pGeneralHdr), &out, err);
				break;

			/* These are request Headers */
			case SipHdrTypeWwwAuthenticate:
			case SipHdrTypeSubject:
			case SipHdrTypeResponseKey:
			case SipHdrTypeRoute:
			case SipHdrTypeProxyRequire:
			case SipHdrTypeProxyauthorization:
			case SipHdrTypePriority:
			case SipHdrTypeMaxforwards:
			case SipHdrTypeHide:
			case SipHdrTypeReplaces:
			case SipHdrTypeAuthorization:
			case SipHdrTypeRAck:
#ifdef SIP_IMPP
			case SipHdrTypeEvent:
			case SipHdrTypeSubscriptionState:
#endif
#ifdef SIP_CCP
			case SipHdrTypeRejectContact:
			case SipHdrTypeAcceptContact:
			case SipHdrTypeRequestDisposition:
#endif
			case SipHdrTypeAlertInfo:
			case SipHdrTypeInReplyTo:
			case SipHdrTypeAlso:
			case SipHdrTypeReferTo:
			case SipHdrTypeReferredBy:
#ifdef SIP_DCS
			case SipHdrTypeDcsOsps:
			case SipHdrTypeDcsTracePartyId:
			case SipHdrTypeDcsRedirect:
#endif
#ifdef SIP_CONGEST
			case SipHdrTypeRsrcPriority:
#endif
#ifdef SIP_CONF                
			case SipHdrTypeJoin:
#endif
#ifdef SIP_SECURITY
			case SipHdrTypeSecurityClient:
			case SipHdrTypeSecurityVerify:
#endif
#ifdef SIP_3GPP
			case SipHdrTypePCalledPartyId:
			case SipHdrTypePVisitedNetworkId:
#endif

			 	res = sip_formSingleRequestHeader (pEndBuff,dType, ndx, mode,\
							form,s->u.pRequest->pRequestHdr, &out, err);
				break;

			default:
					res = SipFail;
					*err = E_INV_PARAM;
		} /* switch */

		SIPDEBUGFN("Exiting from sip_formSingleRequest");
		if(pEndBuff)*ppOut = out;
		return res;
} /* sip_formSingleRequest */


/*****************************************************************
** FUNCTION: sip_formSingleResponse
**
**
** DESCRIPTION: Forms a single response Header
*****************************************************************/
SipBool sip_formSingleResponse
#ifdef ANSI_PROTO
(
	SIP_S8bit	*pEndBuff,
	en_HeaderType dType,
	SIP_U32bit ndx,
	en_AdditionMode mode,
	 en_HeaderForm form,
	 SipMessage *s,
	 SIP_S8bit **ppOut,
	 SipError *err)
#else
	(pEndBuff,dType, ndx, mode, form,s, ppOut, err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	en_HeaderForm form;
	SipMessage *s;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SipBool res;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering into  sip_formSingleResponse");
	out = *ppOut;
	strcpy (out, "");

	/* Now depnding on the Header Type, we call the correct formSingle
		function */
	*err = E_INV_TYPE;
	res = SipFail;
	switch(dType)
	{
			/* These are general Headers */
			case SipHdrTypeAccept:
			case SipHdrTypeAcceptEncoding:
			case SipHdrTypeAcceptLanguage:
			case SipHdrTypeCallId:
			case SipHdrTypeCseq:
			case SipHdrTypeDate:
			case SipHdrTypeEncryption:
			case SipHdrTypeExpiresDate:
			case SipHdrTypeReplyTo:
			case SipHdrTypeExpiresSec:
			case SipHdrTypeExpiresAny:
			case SipHdrTypeRetryAfterDate:
			case SipHdrTypeRetryAfterSec:
			case SipHdrTypeRetryAfterAny:
			case SipHdrTypeFrom:
			case SipHdrTypeRecordRoute:
#ifdef SIP_3GPP
			case SipHdrTypePath:
			case SipHdrTypeServiceRoute :
			case SipHdrTypePanInfo:
			case SipHdrTypePcVector:
#endif
			case SipHdrTypeTimestamp:
			case SipHdrTypeTo:
			case SipHdrTypeVia:
			case SipHdrTypeContentEncoding:
			case SipHdrTypeContentLength:
			case SipHdrTypeContentType:
			case SipHdrTypeContentLanguage:
			case SipHdrTypeContactNormal:
			case SipHdrTypeContactWildCard:
			case SipHdrTypeContactAny:
			case SipHdrTypeMimeVersion:
			case SipHdrTypeContentDisposition:
			case SipHdrTypeReason:
			case SipHdrTypeCallInfo:
			case SipHdrTypeRequire:
			case SipHdrTypeOrganization:
			case SipHdrTypeUserAgent:
			case SipHdrTypeSupported:
			case SipHdrTypeAllow:
#ifdef SIP_PRIVACY
			case SipHdrTypePrivacy: /*Privacy header */
#endif
#ifdef SIP_SESSIONTIMER
			case SipHdrTypeMinSE:
			case SipHdrTypeSessionExpires:
#endif

#ifdef SIP_IMPP
			case SipHdrTypeEvent:
			case SipHdrTypeAllowEvents:
#endif
#ifdef SIP_DCS
			case SipHdrTypeDcsMediaAuthorization:
			case SipHdrTypeDcsBillingId:
			case SipHdrTypeDcsBillingInfo:
			case SipHdrTypeDcsAnonymity:
			case SipHdrTypeDcsState:
			case SipHdrTypeDcsLaes:
			case SipHdrTypeDcsGate:
			case SipHdrTypeDcsRemotePartyId:
			case SipHdrTypeDcsRpidPrivacy:

#endif
#ifdef SIP_PRIVACY
			case SipHdrTypePAssertId:
			case SipHdrTypePPreferredId:
#endif
#ifdef SIP_3GPP
			case SipHdrTypePcfAddr:
#endif
			case SipHdrTypeUnknown:
		 		res = sip_formSingleGeneralHeader\
					(pEndBuff,dType, ndx, mode,form,s->pGeneralHdr, &out, err);
				break;

			/* These are response Headers */
			case SipHdrTypeProxyAuthenticate:
			case SipHdrTypeServer:
			case SipHdrTypeUnsupported:
			case SipHdrTypeWarning:
			case SipHdrTypeWwwAuthenticate:
			case SipHdrTypeAuthenticationInfo:
			case SipHdrTypeAuthorization:
			case SipHdrTypeErrorInfo:
			case SipHdrTypeMinExpires:
			case SipHdrTypeRSeq:
#ifdef SIP_DCS
			case SipHdrTypeSession:
#endif
#ifdef SIP_3GPP
			case SipHdrTypePAssociatedUri:
#endif
#ifdef SIP_CONGEST
			case SipHdrTypeAcceptRsrcPriority:
#endif
#ifdef SIP_SECURITY
                        case SipHdrTypeSecurityServer:
#endif
			 	res = sip_formSingleResponseHeader (pEndBuff,dType, ndx,\
					mode,form,s->u.pResponse->pResponseHdr, &out, err);
				break;
			default:
				break;
	}

	if(pEndBuff)*ppOut = out;

	SIPDEBUGFN("Exiting from  sip_formSingleResponse");
	return res;
} /* sip_formSingleResponse */


/*****************************************************************
** FUNCTION: sip_formSingleHeader
**
**
** DESCRIPTION: Constructs the specified Header from SipStructure
*****************************************************************/

SipBool sip_formSingleHeader
#ifdef ANSI_PROTO
(
	SIP_S8bit	*pEndBuff,
	en_HeaderType hdrtype,
	SIP_U32bit ndx,
	en_AdditionMode mode,
	SipMessage *s,
	 SIP_S8bit **ppOut,
	SipError *err)
#else
	( pEndBuff,hdrtype, ndx, mode,s, ppOut, err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType hdrtype;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	SipMessage *s;
	 SIP_S8bit **ppOut;
	SipError *err;
#endif
{

	en_HeaderForm form;
	SIP_U32bit abs_line;
	SipBool retval;
	SIP_S8bit* out;

	SIPDEBUGFN("Entering  into  sip_formSingleHeader");

	out = *ppOut;
	if (err==SIP_NULL) return SipFail;
	*err = E_INV_PARAM;
	if (s==SIP_NULL) return SipFail;
	*err =  E_NO_ERROR;

	if(__sip_getHeaderPositionFromIndex(s,hdrtype,ndx,&abs_line,SIP_NULL,err)\
		==SipFail)
		return SipFail;
	if( __sip_getHeaderFormFromHeaderLine(s,abs_line,&form,err) == SipFail)
		return SipFail;

	/* Check if it is a Request or Response Type and call appropriate fn */
	if ((s->dType==SipMessageRequest))
	{
		retval=sip_formSingleRequest\
					(pEndBuff,hdrtype,ndx,mode,form, s, &out, err);
	}
	else
	if ((s->dType==SipMessageResponse))
	{
    		retval=sip_formSingleResponse\
					(pEndBuff,hdrtype,ndx,mode,form, s, &out,err);
	}
	else
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(pEndBuff)*ppOut = out;
	SIPDEBUGFN("Exiting  from  sip_formSingleHeader");
	return retval;
}

/*****************************************************************
** FUNCTION: __sip_check
**
**
** DESCRIPTION: This function checks for buffer overflow  and strcat's
**				the buffer only when the compile time option
**				SIP_MSGBUFFER_CHECK is enabled. Otherwise it just
**				strcat's the buffer.
*****************************************************************/
#ifndef SIP_MSGBUFFER_CHECK
SipBool __sip_check(char **pInput,char *pOutput)
#else
SipBool __sip_check(char *e,char **pInput,char *pOutput)
#endif
{

	if (pOutput!=SIP_NULL)
	{
		SIP_U32bit srcLength;
#ifndef SIP_MSGBUFFER_CHECK
		if(*pInput) (*pInput)+=strlen((*pInput));
		srcLength = strlen(pOutput);
		memcpy ((*pInput),pOutput,srcLength);
		(*pInput) +=srcLength;
#else
		SIP_S8bit* pTemp;
		if(*pInput) (*pInput)+=strlen((*pInput));
		srcLength = strlen(pOutput);
		pTemp = (*pInput) + srcLength;
		if((!e)||(pTemp < e))
		memcpy ((*pInput),(pOutput),srcLength);
		else
		{
			return SipFail;
		}
		(*pInput) = pTemp;
#endif
		(**pInput) = 0;
	}
	return SipSuccess;
}
